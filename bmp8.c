#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>

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


void bmp8_free(t_bmp8 *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}


void bmp8_printInfo(t_bmp8 *img) {
    if (img) {
        printf("Image Info:\n");
        printf("    Width: %u\n", img->width);
        printf("    Height: %u\n", img->height);
        printf("    Color Depth: %u\n", img->colorDepth);
        printf("    Data Size: %u\n", img->dataSize);
    }
}


void bmp8_negative(t_bmp8 *img) {
    if (img && img->data) {
        for (unsigned int i = 0; i < img->dataSize; i++) {
            img->data[i] = 255 - img->data[i];
        }
    }
}


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


void bmp8_threshold(t_bmp8 *img, int threshold) {
    if (img && img->data) {
        for (unsigned int i = 0; i < img->dataSize; i++) {
            img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
        }
    }
}


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

