#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Use the project_c_amel_tom_int1 headers for the working version
#include "project_c_amel_tom_int1/bmp8.h"
#include "project_c_amel_tom_int1/bmp24.h"
#include "project_c_amel_tom_int1/equalize8.h"
#include "project_c_amel_tom_int1/equalize24.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *image_display;
    GtkWidget *info_label;
    char *current_image_path;
    char *current_processed_path;
    int image_type; // 8 or 24 bit
    t_bmp8 *img8;
    t_bmp24 *img24;
} AppData;

// Function to detect bit depth
int detectBitDepth(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    fseek(f, 28, SEEK_SET);
    uint16_t bits;
    fread(&bits, sizeof(uint16_t), 1, f);
    fclose(f);
    return (bits == 8 || bits == 24) ? bits : -1;
}

static void on_open_image(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open Image",
                                        GTK_WINDOW(app->window),
                                        action,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Open", GTK_RESPONSE_ACCEPT,
                                        NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "BMP Images");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        // Clean up previous images
        if (app->img8) {
            bmp8_free(app->img8);
            app->img8 = NULL;
        }
        if (app->img24) {
            bmp24_free(app->img24);
            app->img24 = NULL;
        }
        
        // Store the image path
        if (app->current_image_path) {
            g_free(app->current_image_path);
        }
        app->current_image_path = g_strdup(filename);
        
        // Detect bit depth and load appropriate image
        app->image_type = detectBitDepth(filename);
        
        if (app->image_type == 8) {
            app->img8 = bmp8_loadImage(filename);
            if (app->img8) {
                char info_text[512];
                snprintf(info_text, sizeof(info_text), 
                        "Loaded 8-bit image: %s (%dx%d)", 
                        filename, app->img8->width, app->img8->height);
                gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
            } else {
                gtk_label_set_text(GTK_LABEL(app->info_label), "Failed to load 8-bit image");
            }
        } else if (app->image_type == 24) {
            app->img24 = bmp24_loadImage(filename);
            if (app->img24) {
                char info_text[512];
                snprintf(info_text, sizeof(info_text), 
                        "Loaded 24-bit image: %s (%dx%d)", 
                        filename, app->img24->width, app->img24->height);
                gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
            } else {
                gtk_label_set_text(GTK_LABEL(app->info_label), "Failed to load 24-bit image");
            }
        } else {
            gtk_label_set_text(GTK_LABEL(app->info_label), "Unsupported image format");
            g_free(filename);
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Load and display image in GUI
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        if (pixbuf) {
            // Scale image to fit in display
            int width = gdk_pixbuf_get_width(pixbuf);
            int height = gdk_pixbuf_get_height(pixbuf);
            int max_size = 400;
            
            if (width > max_size || height > max_size) {
                double scale = (double)max_size / (width > height ? width : height);
                int new_width = (int)(width * scale);
                int new_height = (int)(height * scale);
                
                GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
                gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), scaled);
                g_object_unref(scaled);
            } else {
                gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), pixbuf);
            }
            g_object_unref(pixbuf);
        }
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_save_image(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    if (!app->current_image_path) {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No image loaded");
        return;
    }

    if (!app->img8 && !app->img24) {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No processed image to save");
        return;
    }

    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Save Image",
                                        GTK_WINDOW(app->window),
                                        action,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Save", GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Set default filename
    char default_name[512];
    snprintf(default_name, sizeof(default_name), "processed_%s", 
             strrchr(app->current_image_path, '/') ? strrchr(app->current_image_path, '/') + 1 : app->current_image_path);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), default_name);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        // Save the appropriate image type
        int success = 0;
        if (app->image_type == 8 && app->img8) {
            success = (bmp8_saveImage(filename, app->img8) == 0);
        } else if (app->image_type == 24 && app->img24) {
            success = (bmp24_saveImage(filename, app->img24) == 0);
        }
        
        if (success) {
            char info_text[512];
            snprintf(info_text, sizeof(info_text), "Image saved to: %s", filename);
            gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
        } else {
            gtk_label_set_text(GTK_LABEL(app->info_label), "Failed to save image");
        }
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void apply_filter_and_update_display(AppData *app, const char *filter_name) {
    if (!app->current_image_path) {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No image loaded");
        return;
    }
    
    if (!app->img8 && !app->img24) {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No image data loaded");
        return;
    }
    
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "Applying %s filter...", filter_name);
    gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
    
    // Process events to update the UI
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    // Create temporary filename for processed image
    char temp_filename[512];
    snprintf(temp_filename, sizeof(temp_filename), "/tmp/gui_processed_%d.bmp", (int)time(NULL));
    
    // Apply filter and save temporary file
    if (app->image_type == 8 && app->img8) {
        if (bmp8_saveImage(temp_filename, app->img8) == 0) {
            // Update display with processed image
            GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(temp_filename, NULL);
            if (pixbuf) {
                // Scale image to fit in display
                int width = gdk_pixbuf_get_width(pixbuf);
                int height = gdk_pixbuf_get_height(pixbuf);
                int max_size = 400;
                
                if (width > max_size || height > max_size) {
                    double scale = (double)max_size / (width > height ? width : height);
                    int new_width = (int)(width * scale);
                    int new_height = (int)(height * scale);
                    
                    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
                    gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), scaled);
                    g_object_unref(scaled);
                } else {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), pixbuf);
                }
                g_object_unref(pixbuf);
            }
            // Clean up temporary file
            unlink(temp_filename);
        }
    } else if (app->image_type == 24 && app->img24) {
        if (bmp24_saveImage(temp_filename, app->img24) == 0) {
            // Update display with processed image
            GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(temp_filename, NULL);
            if (pixbuf) {
                // Scale image to fit in display
                int width = gdk_pixbuf_get_width(pixbuf);
                int height = gdk_pixbuf_get_height(pixbuf);
                int max_size = 400;
                
                if (width > max_size || height > max_size) {
                    double scale = (double)max_size / (width > height ? width : height);
                    int new_width = (int)(width * scale);
                    int new_height = (int)(height * scale);
                    
                    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
                    gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), scaled);
                    g_object_unref(scaled);
                } else {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), pixbuf);
                }
                g_object_unref(pixbuf);
            }
            // Clean up temporary file
            unlink(temp_filename);
        }
    }
    
    snprintf(info_text, sizeof(info_text), "Applied %s filter", filter_name);
    gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
}

static void on_negative_filter(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 8 && app->img8) {
        bmp8_negative(app->img8);
    } else if (app->image_type == 24 && app->img24) {
        bmp24_negative(app->img24);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No compatible image loaded");
        return;
    }
    
    apply_filter_and_update_display(app, "Negative");
}

static void on_grayscale_filter(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 24 && app->img24) {
        bmp24_grayscale(app->img24);
        apply_filter_and_update_display(app, "Grayscale");
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "Grayscale only available for 24-bit images");
    }
}

static void on_blur_filter(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 24 && app->img24) {
        bmp24_boxBlur(app->img24);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "Box blur only available for 24-bit images");
        return;
    }
    
    apply_filter_and_update_display(app, "Box Blur");
}

static void on_sharpen_filter(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 24 && app->img24) {
        bmp24_sharpen(app->img24);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "Sharpen only available for 24-bit images");
        return;
    }
    
    apply_filter_and_update_display(app, "Sharpen");
}

static void on_equalize_histogram(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 8 && app->img8) {
        bmp8_equalize(app->img8);
    } else if (app->image_type == 24 && app->img24) {
        bmp24_equalize(app->img24);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No compatible image loaded");
        return;
    }
    
    apply_filter_and_update_display(app, "Histogram Equalization");
}

static void on_brightness_up(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 8 && app->img8) {
        bmp8_brightness(app->img8, 30);
    } else if (app->image_type == 24 && app->img24) {
        bmp24_brightness(app->img24, 30);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No compatible image loaded");
        return;
    }
    
    apply_filter_and_update_display(app, "Brightness +30");
}

static void on_brightness_down(GtkWidget *widget, gpointer data) {
    AppData *app = (AppData*)data;
    
    if (app->image_type == 8 && app->img8) {
        bmp8_brightness(app->img8, -30);
    } else if (app->image_type == 24 && app->img24) {
        bmp24_brightness(app->img24, -30);
    } else {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No compatible image loaded");
        return;
    }
    
    apply_filter_and_update_display(app, "Brightness -30");
}

static void cleanup_app_data(AppData *app_data) {
    if (app_data->img8) {
        bmp8_free(app_data->img8);
        app_data->img8 = NULL;
    }
    if (app_data->img24) {
        bmp24_free(app_data->img24);
        app_data->img24 = NULL;
    }
    if (app_data->current_image_path) {
        g_free(app_data->current_image_path);
        app_data->current_image_path = NULL;
    }
    if (app_data->current_processed_path) {
        g_free(app_data->current_processed_path);
        app_data->current_processed_path = NULL;
    }
}

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData*)data;
    cleanup_app_data(app_data);
    g_free(app_data);
    gtk_main_quit();
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *app_data = g_malloc(sizeof(AppData));
    app_data->current_image_path = NULL;
    app_data->current_processed_path = NULL;
    app_data->image_type = 24;
    app_data->img8 = NULL;
    app_data->img24 = NULL;

    // Create main window
    app_data->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data->window), "Image Processing Tool");
    gtk_window_set_default_size(GTK_WINDOW(app_data->window), 800, 600);

    // Create main layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app_data->window), vbox);

    // Create menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);

    GtkWidget *open_item = gtk_menu_item_new_with_label("Open Image");
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save Image");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);

    // Filters menu
    GtkWidget *filters_menu = gtk_menu_new();
    GtkWidget *filters_item = gtk_menu_item_new_with_label("Filters");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(filters_item), filters_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), filters_item);

    GtkWidget *negative_item = gtk_menu_item_new_with_label("Negative");
    GtkWidget *grayscale_item = gtk_menu_item_new_with_label("Grayscale");
    GtkWidget *blur_item = gtk_menu_item_new_with_label("Blur");
    GtkWidget *sharpen_item = gtk_menu_item_new_with_label("Sharpen");
    GtkWidget *equalize_item = gtk_menu_item_new_with_label("Histogram Equalization");
    GtkWidget *brightness_up_item = gtk_menu_item_new_with_label("Brightness +30");
    GtkWidget *brightness_down_item = gtk_menu_item_new_with_label("Brightness -30");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), negative_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), grayscale_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), blur_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), sharpen_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), equalize_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), brightness_up_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), brightness_down_item);

    // Image display area
    app_data->image_display = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(vbox), app_data->image_display, TRUE, TRUE, 0);

    // Info label
    app_data->info_label = gtk_label_new("Ready - Load an image to begin");
    gtk_box_pack_start(GTK_BOX(vbox), app_data->info_label, FALSE, FALSE, 0);

    // Connect signals
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_image), app_data);
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save_image), app_data);
    g_signal_connect(negative_item, "activate", G_CALLBACK(on_negative_filter), app_data);
    g_signal_connect(grayscale_item, "activate", G_CALLBACK(on_grayscale_filter), app_data);
    g_signal_connect(blur_item, "activate", G_CALLBACK(on_blur_filter), app_data);
    g_signal_connect(sharpen_item, "activate", G_CALLBACK(on_sharpen_filter), app_data);
    g_signal_connect(equalize_item, "activate", G_CALLBACK(on_equalize_histogram), app_data);
    g_signal_connect(brightness_up_item, "activate", G_CALLBACK(on_brightness_up), app_data);
    g_signal_connect(brightness_down_item, "activate", G_CALLBACK(on_brightness_down), app_data);
    g_signal_connect(app_data->window, "destroy", G_CALLBACK(on_window_destroy), app_data);

    gtk_widget_show_all(app_data->window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.imageprocessing.gui", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
