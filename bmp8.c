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
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }

    t_bmp8 * image = (t_bmp8 *) malloc(sizeof(t_bmp8));
    if (image == NULL) {
        perror("Erreur d'allocation mémoire pour l'image");
        fclose(file);
        return NULL;
    }

    if (fread(image->header, sizeof(unsigned char), 54, file) != 54) {
        fprintf(stderr, "Erreur lors de la lecture du header\n");
        free(image);
        fclose(file);
        return NULL;
    }

    image->width       = *(unsigned int *)&image->header[18];
    image->height      = *(unsigned int *)&image->header[22];
    image->colorDepth  = *(unsigned short *)&image->header[28];
    image->dataSize    = *(unsigned int *)&image->header[34];

    if (image->colorDepth != 8) {
        fprintf(stderr, "Erreur : l'image n'est pas en 8 bits (colorDepth = %u)\n", image->colorDepth);
        free(image);
        fclose(file);
        return NULL;
    }

    if (fread(image->colorTable, sizeof(unsigned char), 1024, file) != 1024) {
        fprintf(stderr, "Erreur lors de la lecture de la table de couleurs\n");
        free(image);
        fclose(file);
        return NULL;
    }

    image->data = (unsigned char *) malloc(image->dataSize);
    if (image->data == NULL) {
        perror("Erreur d'allocation mémoire pour les données");
        free(image);
        fclose(file);
        return NULL;
    }

    if (fread(image->data, sizeof(unsigned char), image->dataSize, file) != image->dataSize) {
        fprintf(stderr, "Erreur lors de la lecture des données de l'image\n");
        free(image->data);
        free(image);
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Création de la matrice pixels
    int rowSize = ((image->width + 3) / 4) * 4;
    image->pixels = malloc(image->height * sizeof(unsigned char *));
    for (unsigned int i = 0; i < image->height; i++) {
        image->pixels[i] = malloc(image->width * sizeof(unsigned char));
        for (unsigned int j = 0; j < image->width; j++) {
            image->pixels[i][j] = image->data[i * rowSize + j];
        }
    }

    return image;
}

void bmp8_saveImage(const char * filename, t_bmp8 * image) {
    FILE * file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier en écriture");
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
        printf("Aucune image à afficher.\n");
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
        image->data[i] = (image->data[i] >= threshold) ? 255 : 0;
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

    // Copier les pixels vers data (ligne par ligne)
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
