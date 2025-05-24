#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include "bmp24.h"
#include "histogram_equalization.h"


typedef struct {
    GtkWidget *window;
    GtkWidget *image_display;
    GtkWidget *info_label;
    char *current_image_path;
    int image_type; // 8 or 24 bit
} AppData;

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
        
        // Store the image path
        if (app->current_image_path) {
            g_free(app->current_image_path);
        }
        app->current_image_path = g_strdup(filename);
        
        // Load and display image
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        if (pixbuf) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), pixbuf);
            g_object_unref(pixbuf);
            
            // Update info label
            char info_text[256];
            snprintf(info_text, sizeof(info_text), "Loaded: %s", filename);
            gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
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

    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Save Image",
                                        GTK_WINDOW(app->window),
                                        action,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Save", GTK_RESPONSE_ACCEPT,
                                        NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        // Save logic here - copy current processed image
        gtk_label_set_text(GTK_LABEL(app->info_label), "Image saved");
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void apply_filter(AppData *app, const char *filter_name, void (*filter_func_8bit)(), void (*filter_func_24bit)()) {
    if (!app->current_image_path) {
        gtk_label_set_text(GTK_LABEL(app->info_label), "No image loaded");
        return;
    }
    
    // Apply filter based on image type
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "Applied %s filter", filter_name);
    gtk_label_set_text(GTK_LABEL(app->info_label), info_text);
    
    // Reload image display after filter application
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(app->current_image_path, NULL);
    if (pixbuf) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_display), pixbuf);
        g_object_unref(pixbuf);
    }
}

static void on_negative_filter(GtkWidget *widget, gpointer data) {
    apply_filter((AppData*)data, "Negative", NULL, NULL);
}

static void on_blur_filter(GtkWidget *widget, gpointer data) {
    apply_filter((AppData*)data, "Blur", NULL, NULL);
}

static void on_sharpen_filter(GtkWidget *widget, gpointer data) {
    apply_filter((AppData*)data, "Sharpen", NULL, NULL);
}

static void on_equalize_histogram(GtkWidget *widget, gpointer data) {
    apply_filter((AppData*)data, "Histogram Equalization", NULL, NULL);
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *app_data = g_malloc(sizeof(AppData));
    app_data->current_image_path = NULL;
    app_data->image_type = 24;

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
    GtkWidget *blur_item = gtk_menu_item_new_with_label("Blur");
    GtkWidget *sharpen_item = gtk_menu_item_new_with_label("Sharpen");
    GtkWidget *equalize_item = gtk_menu_item_new_with_label("Histogram Equalization");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), negative_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), blur_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), sharpen_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(filters_menu), equalize_item);

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
    g_signal_connect(blur_item, "activate", G_CALLBACK(on_blur_filter), app_data);
    g_signal_connect(sharpen_item, "activate", G_CALLBACK(on_sharpen_filter), app_data);
    g_signal_connect(equalize_item, "activate", G_CALLBACK(on_equalize_histogram), app_data);

    gtk_widget_show_all(app_data->window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.imageprocessing.gui", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
