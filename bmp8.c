//
// Created by Boulhamane Amel on 23/04/2025.
//

#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_bmp8 * bmp8_loadImage(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (file == NULL) {
        printf("You have an error opening the file. %s\n", filename);
        return NULL;
    }

    t_bmp8 * image = (t_bmp8 *) malloc(sizeof(t_bmp8));
    if (image == NULL) {
        printf("Memory allocation error for image.\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < 54; i++) {
        int byte = fgetc(file);
        if (byte == EOF) {
            printf("Error reading header\n");
            free(image);
            fclose(file);
            return NULL;
        }
        image->header[i] = (unsigned char) byte;
    }

    image->width       = *(unsigned int *)&image->header[18];
    image->height      = *(unsigned int *)&image->header[22];
    image->colorDepth  = *(unsigned short *)&image->header[28];
    image->dataSize    = *(unsigned int *)&image->header[34];

    if (image->colorDepth != 8) {
        printf("There's an Error because the image is not 8-bit (colorDepth = %u)\n", image->colorDepth);
        free(image);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < 1024; i++) {
        int byte = fgetc(file);
        if (byte == EOF) {
            printf("Error when reading color table\n");
            free(image);
            fclose(file);
            return NULL;
        }
        image->colorTable[i] = (unsigned char) byte;
    }

    image->data = (unsigned char *) malloc(image->dataSize);
    if (image->data == NULL) {
        printf("Memory allocation error for data\n");
        free(image);
        fclose(file);
        return NULL;
    }

    for (size_t i = 0; i < image->dataSize; i++) {
        int byte = fgetc(file);
        if (byte == EOF) {
            printf("Error reading image data\n");
            free(image->data);
            free(image);
            fclose(file);
            return NULL;
        }
        image->data[i] = (unsigned char) byte;
    }
    fclose(file);

    int rowSize = ((image->width + 3) / 4) * 4;
    image->pixels = malloc(image->height * sizeof(unsigned char *));

    if (image->pixels == NULL) {
        printf("Memory allocation error for pixels\n");
        free(image->data);
        free(image);
        return NULL;
    }

    for (unsigned int i = 0; i < image->height; i++) {
        image->pixels[i] = malloc(image->width * sizeof(unsigned char));
        if (image->pixels[i] == NULL) {
            printf("Memory allocation error for line %u\n", i);
            free(image->pixels);
            free(image->data);
            free(image);
            return NULL;
        }

        for (unsigned int j = 0; j < image->width; j++) {
            image->pixels[i][j] = image->data[i * rowSize + j];
        }
    }
    return image;
}

void bmp8_saveImage(const char * filename, t_bmp8 * image) {
    FILE * file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file for writing\n");
        return;
    }

    

    fwrite(image->header, sizeof(unsigned char), 54, file);
    fwrite(image->colorTable, sizeof(unsigned char), 1024, file);
    fwrite(image->data, sizeof(unsigned char), image->dataSize, file);

    fclose(file);
}


void bmp8_free(t_bmp8 * image) {
    if (image != NULL) {
        if (image->data != NULL) free(image->data);
        if (image->pixels != NULL) {
            for (unsigned int i = 0; i < image->height; i++) {
                free(image->pixels[i]);
            }
            free(image->pixels);
        }
        free(image);
    }
}

void bmp8_printInfo(t_bmp8 * image) {
    if (image == NULL) {
        printf("No images to display.\n");
        return;
    }

    printf("Image Info:\n");
    printf("    Width: %u\n", image->width);
    printf("    Height: %u\n", image->height);
    printf("    Color Depth: %u\n", image->colorDepth);
    printf("    Data Size: %u\n", image->dataSize);
}

void bmp8_negative(t_bmp8 * image) {
    if (image == NULL || image->data == NULL) return;

    for (unsigned int i = 0; i < image->dataSize; ++i) {
        image->data[i] = 255 - image->data[i];
    }
}

void bmp8_changeBrightness(t_bmp8 * image, int brightnessChange) {
    if (image == NULL || image->data == NULL) return;

    for (unsigned int i = 0; i < image->dataSize; ++i) {
        int newValue = image->data[i] + brightnessChange;
        if (newValue > 255) newValue = 255;
        else if (newValue < 0) newValue = 0;
        image->data[i] = (unsigned char)newValue;
    }
}

void bmp8_threshold(t_bmp8 * image, int threshold) {
    if (image == NULL || image->data == NULL) return;

    for (unsigned int i = 0; i < image->dataSize; ++i) {
        if (image->data[i] >= threshold) {
            image->data[i] = 255;
        } else {
            image->data[i] = 0;
        }
    }
}

void bmp8_applyFilter(t_bmp8 * img, float **kernel, int kernelSize) {
    int width = img->width;
    int height = img->height;
    int n = kernelSize / 2;

    unsigned char **tempPixels = malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++) {
        tempPixels[i] = malloc(width * sizeof(unsigned char));
        for (int j = 0; j < width; j++) {
            tempPixels[i][j] = img->pixels[i][j];
        }
    }

    for (int y = n; y < height - n; y++) {
        for (int x = n; x < width - n; x++) {
            float sum = 0.0;
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    int pixel = tempPixels[y + i][x + j];
                    float weight = kernel[i + n][j + n];
                    sum += pixel * weight;
                }
            }
            if (sum > 255.0f) sum = 255.0f;
            if (sum < 0.0f) sum = 0.0f;
            img->pixels[y][x] = (unsigned char)(sum);
        }
    }
    int rowSize = ((width + 3) / 4) * 4;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img->data[y * rowSize + x] = img->pixels[y][x];
        }
    }

    for (int i = 0; i < height; i++) {
        free(tempPixels[i]);
    }
    free(tempPixels);
}

t_bmp8 * bmp8_copy(t_bmp8 * src) {
    if (src == NULL) return NULL;

    t_bmp8 * dest = (t_bmp8 *) malloc(sizeof(t_bmp8));
    if (dest == NULL) {
        return NULL;
    }

    memcpy(dest->header, src->header, 54);
    memcpy(dest->colorTable, src->colorTable, 1024);
    
    dest->width = src->width;
    dest->height = src->height;
    dest->colorDepth = src->colorDepth;
    dest->dataSize = src->dataSize;

    dest->data = (unsigned char *) malloc(src->dataSize);
    if (dest->data == NULL) {
        free(dest);
        return NULL;
    }
    memcpy(dest->data, src->data, src->dataSize);

    int rowSize = ((src->width + 3) / 4) * 4;
    dest->pixels = malloc(src->height * sizeof(unsigned char *));
    if (dest->pixels == NULL) {
        free(dest->data);
        free(dest);
        return NULL;
    }

    for (unsigned int i = 0; i < src->height; i++) {
        dest->pixels[i] = malloc(src->width * sizeof(unsigned char));
        if (dest->pixels[i] == NULL) {
            for (unsigned int j = 0; j < i; j++) {
                free(dest->pixels[j]);
            }
            free(dest->pixels);
            free(dest->data);
            free(dest);
            return NULL;
        }
        for (unsigned int j = 0; j < src->width; j++) {
            dest->pixels[i][j] = src->pixels[i][j];
        }
    }

    return dest;
}
