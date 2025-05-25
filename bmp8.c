#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>


// -------------------- HEADER ---------------------------
//  Name : bmp8.c
//  Goal : handle all functions related to 8-bit images (loading, debugging, filters, informations...)
//  Authors : Amel Boulhamane and Tom Hausmann
// 
/// NOTE : Throughout the files, we use the @brief, @param and @return structure for more consistency in the comments 
//
// --------------------------------------------------------



/// @brief This function must load the image with a pointer to the filename, while checking if it is valid (depth, headers, etc...)
/// @param filename 
/// @return An error if the file can't be open, else, the dynamically attribute memory for the image data.


t_bmp8 *bmp8_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    t_bmp8 *img = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!img) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read BMP header
    if (fread(img->header, 1, 54, file) != 54) {
        fprintf(stderr, "Error reading BMP header\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Validate BMP signature
    if (img->header[0] != 'B' || img->header[1] != 'M') {
        fprintf(stderr, "Not a valid BMP file (missing 'BM' signature).\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Extract key info
    img->width = *(uint32_t *)&img->header[18];
    img->height = *(uint32_t *)&img->header[22];
    img->colorDepth = *(uint16_t *)&img->header[28];
    img->dataSize = *(uint32_t *)&img->header[34];

    // Fallback data size (some BMPs set it to 0)
    if (img->dataSize == 0) {
        img->dataSize = img->width * img->height;
    }

    // Debug info
    printf("DEBUG: Width=%u, Height=%u, ColorDepth=%u, DataSize=%u\n",
        img->width, img->height, img->colorDepth, img->dataSize);

    // Reject unsupported formats
    if (img->colorDepth != 8) {
        fprintf(stderr, "Unsupported BMP format: Only 8-bit grayscale BMPs are supported.\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read color table (1024 bytes = 256 entries × 4 bytes)
    if (fread(img->colorTable, 1, 1024, file) != 1024) {
        fprintf(stderr, "Error reading color table.\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Allocate memory for image data
    img->data = (unsigned char *)malloc(img->dataSize);
    if (!img->data) {
        perror("Error allocating memory for image data");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read pixel data
    if (fread(img->data, 1, img->dataSize, file) != img->dataSize) {
        fprintf(stderr, "Error reading pixel data.\n");
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}



/// @brief This function allow the user to save the image, with error handling 
/// @param filename @param img
/// @return An error if the file can't be open, else it writes the file directly.

int bmp8_saveImage(const char *filename, t_bmp8 *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }


    if (fwrite(img->header, 1, 54, file) != 54) {
        perror("Error writing header");
        fclose(file);
        return -1;
    }


    if (fwrite(img->colorTable, 1, 1024, file) != 1024) {
        perror("Error writing color table");
        fclose(file);
        return -1;
    }


    if (fwrite(img->data, 1, img->dataSize, file) != img->dataSize) {
        perror("Error writing image data");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}


/// @brief This function free the allocated memory for an image
/// @param img 
/// @return VOID

void bmp8_free(t_bmp8 *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}


/// @brief This function prints out all the informations of a previously loaded image, using the structure
/// @param img 
/// @return VOID


void bmp8_printInfo(t_bmp8 *img) {
    if (img) {
        printf("Image Info:\n");
        printf("    Width: %u\n", img->width);
        printf("    Height: %u\n", img->height);
        printf("    Color Depth: %u\n", img->colorDepth);
        printf("    Data Size: %u\n", img->dataSize);
    }
}


/// @brief This function apply the negative filter, by applying 255 - current value to each pixel
/// @param img 
/// @return VOID
void bmp8_negative(t_bmp8 *img) {
    if (img && img->data) {
        for (unsigned int i = 0; i < img->dataSize; i++) {
            img->data[i] = 255 - img->data[i];
        }
    }
}



/// @brief Adjusts the brightness of an 8-bit BMP image. It decrease it or increases it by adding the value given to each pixel.
/// @param img, @param value 
/// @return VOID


void bmp8_brightness(t_bmp8 *img, int value) {
    if (img && img->data) {
        for (unsigned int i = 0; i < img->dataSize; i++) {
            int newValue = img->data[i] + value;
            if (newValue > 255) {
                newValue = 255;
            } else if (newValue < 0) {
                newValue = 0;
            }
            img->data[i] = (unsigned char)newValue;
        }
    }
}
/// @brief This converts a grayscale image to a binary (black and white) image based on the given threshold. If the treshold is 100 and a pixel has value = 80, it become 0.
/// If it has 120 as value, it becomes 255
/// @param img, @param treshold 
/// @return VOID




void bmp8_threshold(t_bmp8 *img, int threshold) {
    if (img && img->data) {
        for (unsigned int i = 0; i < img->dataSize; i++) {
            img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
        }
    }
}




///@brief This function apply the specified filter to the input data.

///@param input Pointer to the input data to be filtered.
///@param filter_params Structure containing filter configuration parameters.
///@return The result of applying the filter to the input data.


void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    if (!img || !img->data || !kernel) {
        return;
    }

    int n = kernelSize / 2;
    unsigned char *newData = (unsigned char *)malloc(img->dataSize);
    if (!newData) {
        perror("Error allocating memory for new image data");
        return;
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            float sum = 0.0f;
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    int pixelValue = img->data[(y + i) * img->width + (x + j)];
                    sum += pixelValue * kernel[i + n][j + n];
                }
            }
            newData[y * img->width + x] = (unsigned char)sum;
        }
    }


    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y * img->width + x] = newData[y * img->width + x];
        }
    }

    free(newData);
}

