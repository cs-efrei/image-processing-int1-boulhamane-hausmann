#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bmp8.h"
#include "bmp24.h"
#include "equalize8.h"
#include "equalize24.h"

// This serves as detection for the bit depth of the images
int detectBitDepth(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    fseek(f, 28, SEEK_SET);
    uint16_t bits;
    fread(&bits, sizeof(uint16_t), 1, f);
    printf("bits = %d\n", bits);
    fclose(f);
    return (bits == 8 || bits == 24) ? bits : -1;
}


// This serves as the Filter menu for every 8 bit related images
void applyFilters8(t_bmp8 *img) {
    int choice;
    while (1) {
        printf("\nChoose a filter to apply:\n");
        printf("1. Negative\n");
        printf("2. Brightness\n");
        printf("3. Black and white (threshold)\n");
        printf("4. Box Blur\n");
        printf("5. Gaussian Blur\n");
        printf("6. Outline\n");
        printf("7. Emboss\n");
        printf("8. Sharpen\n");
        printf("9. Histogram Equalization\n");
        printf("10. Return to main menu\n");
        printf(">>> Your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                bmp8_negative(img);
                printf("Negative applied.\n");
                break;
            case 2: {
                int value;
                printf("Brightness value (-255 to 255): ");
                scanf("%d", &value);
                getchar();
                bmp8_brightness(img, value);
                printf("Brightness adjusted.\n");
                break;
            }
            case 3: {
                int threshold;
                printf("Threshold value (0 to 255): ");
                scanf("%d", &threshold);
                getchar();
                bmp8_threshold(img, threshold);
                printf("Black and white applied.\n");
                break;
            }
            case 4: {
                float kernel[3][3] = {
                    {1.0f/9, 1.0f/9, 1.0f/9},
                    {1.0f/9, 1.0f/9, 1.0f/9},
                    {1.0f/9, 1.0f/9, 1.0f/9}
                };
                bmp8_applyFilter(img, (float **)kernel, 3);
                printf("Box Blur applied.\n");
                break;
            }
            case 5: {
                float kernel[3][3] = {
                    {1.0f/16, 2.0f/16, 1.0f/16},
                    {2.0f/16, 4.0f/16, 2.0f/16},
                    {1.0f/16, 2.0f/16, 1.0f/16}
                };
                bmp8_applyFilter(img, (float **)kernel, 3);
                printf("Gaussian Blur applied.\n");
                break;
            }
            case 6: {
                float kernel[3][3] = {
                    {-1, -1, -1},
                    {-1,  8, -1},
                    {-1, -1, -1}
                };
                bmp8_applyFilter(img, (float **)kernel, 3);
                printf("Outline filter applied.\n");
                break;
            }
            case 7: {
                float kernel[3][3] = {
                    {-2, -1, 0},
                    {-1,  1, 1},
                    { 0,  1, 2}
                };
                bmp8_applyFilter(img, (float **)kernel, 3);
                printf("Emboss filter applied.\n");
                break;
            }
            case 8: {
                float kernel[3][3] = {
                    { 0, -1,  0},
                    {-1,  5, -1},
                    { 0, -1,  0}
                };
                bmp8_applyFilter(img, (float **)kernel, 3);
                printf("Sharpen filter applied.\n");
                break;
            }
            case 9:
                bmp8_equalize(img);
                printf("Histogram Equalization applied.\n");
                break;
            case 10:
                return;
            default:
                printf("Invalid option.\n");
        }
    }
}




// This serves as the Filter menu for every 24 bit related images
void applyFilters24(t_bmp24 *img) {
    int choice;
    while (1) {
        printf("\nChoose a filter to apply:\n");
        printf("1. Negative\n");
        printf("2. Grayscale\n");
        printf("3. Brightness\n");
        printf("4. Box Blur\n");
        printf("5. Gaussian Blur\n");
        printf("6. Outline\n");
        printf("7. Emboss\n");
        printf("8. Sharpen\n");
        printf("9. Histogram Equalization\n");
        printf("10. Return to main menu\n");
        printf(">>> Your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                bmp24_negative(img);
            printf("Negative applied.\n");
            break;
            case 2:
                bmp24_grayscale(img);
            printf("Grayscale applied.\n");
            break;
            case 3: {
                int value;
                printf("Brightness value (-255 to 255): ");
                scanf("%d", &value);
                getchar();
                bmp24_brightness(img, value);
                printf("Brightness adjusted.\n");
                break;
            }
            case 4:
                bmp24_boxBlur(img);
            printf("Box Blur applied.\n");
            break;
            case 5:
                bmp24_gaussianBlur(img);
            printf("Gaussian Blur applied.\n");
            break;
            case 6:
                bmp24_outline(img);
            printf("Outline filter applied.\n");
            break;
            case 7:
                bmp24_emboss(img);
            printf("Emboss filter applied.\n");
            break;
            case 8:
                bmp24_sharpen(img);
            printf("Sharpen filter applied.\n");
            break;
            case 9:
                bmp24_equalize(img);
            printf("Histogram Equalization applied.\n");
            break;
            case 10:
                return;
            default:
                printf("Invalid option.\n");
        }

    }
}




// This serves as the main function for out program
int main(void) {
    char filepath[256];
    int choice;
    int bits = -1;

    t_bmp8 *img8 = NULL;
    t_bmp24 *img24 = NULL;

    while (1) {
        printf("\nPlease choose an option:\n");
        printf("1. Open an image\n");
        printf("2. Save image\n");
        printf("3. Apply a filter\n");
        printf("4. Display image information\n");
        printf("5. Quit\n");
        printf(">>> Your choice: ");

        char input[10];
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid option. Please choose 1 to 5.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                printf("Enter file path: ");
                scanf("%255s", filepath);
                getchar();

                bits = detectBitDepth(filepath);

                if (img8) {
                    bmp8_free(img8);
                    img8 = NULL;
                }
                if (img24) {
                    bmp24_free(img24);
                    img24 = NULL;
                }

                if (bits == 8) {
                    img8 = bmp8_loadImage(filepath);
                    if (!img8) {
                        printf("Failed to load 8-bit image.\n");
                        bits = -1;
                    } else {
                        printf("8-bit image loaded successfully.\n");
                    }
                } else if (bits == 24) {
                    img24 = bmp24_loadImage(filepath);
                    if (!img24) {
                        printf("Failed to load 24-bit image.\n");
                        bits = -1;
                    } else {
                        printf("24-bit image loaded successfully.\n");
                    }
                } else {
                    printf("Unsupported format. Only 8-bit and 24-bit BMP files are supported.\n");
                }
                break;
            }

            case 2: {
                if (bits == 8 && img8) {
                    printf("Enter output file name: ");
                    scanf("%255s", filepath);
                    getchar();
                    bmp8_saveImage(filepath, img8);
                } else if (bits == 24 && img24) {
                    printf("Enter output file name: ");
                    scanf("%255s", filepath);
                    getchar();
                    bmp24_saveImage(filepath, img24);
                } else {
                    printf("Please load an image first.\n");
                }
                break;
            }

            case 3:
                if (bits == 8 && img8) {
                    applyFilters8(img8);
                } else if (bits == 24 && img24) {
                    applyFilters24(img24);
                } else {
                    printf("Please load an image first.\n");
                }
                break;

            case 4:
                if (bits == 8 && img8) {
                    bmp8_printInfo(img8);
                } else if (bits == 24 && img24) {
                    printf("Image information:\n");
                    printf("Width       : %d px\n", img24->width);
                    printf("Height      : %d px\n", img24->height);
                    printf("Color depth : %d bits\n", img24->colorDepth);
                } else {
                    printf("Please load an image first.\n");
                }
                break;

            case 5:
                if (img8) {
                    bmp8_free(img8);
                }
                if (img24) {
                    bmp24_free(img24);
                }
                printf("Program exited. Goodbye!\n");
                return 0;

            default:
                printf("Invalid option. Please choose 1 to 5.\n");
        }
    }
}
