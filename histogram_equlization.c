#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Structure simplifiée de l'image 8 bits en niveaux de gris
typedef struct {
    int width;
    int height;
    unsigned char *data; // tableau de taille width * height
} t_bmp8;

// 1. Calcul de l'histogramme
typedef unsigned int uint;

uint *bmp8_computeHistogram(t_bmp8 *img) {
    if (!img || !img->data) return NULL;

    uint *hist = calloc(256, sizeof(uint));
    if (!hist) return NULL;

    for (int i = 0; i < img->width * img->height; i++) {
        hist[img->data[i]]++;
    }

    return hist;
}

// 2. Calcul de la CDF
uint *bmp8_computeCDF(uint *hist) {
    if (!hist) return NULL;

    uint *cdf = calloc(256, sizeof(uint));
    if (!cdf) return NULL;

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    return cdf;
}

// 3. Calcul du mapping égalisé
uint *computeEqualizedMapping(uint *cdf, int total_pixels) {
    if (!cdf) return NULL;

    uint *hist_eq = malloc(256 * sizeof(uint));
    if (!hist_eq) return NULL;

    // Trouver CDF_min non nul
    uint cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    for (int i = 0; i < 256; i++) {
        hist_eq[i] = round(((double)(cdf[i] - cdf_min) / (total_pixels - cdf_min)) * 255);
    }

    return hist_eq;
}

// 4. Application de l'égalisation
void bmp8_equalize(t_bmp8 *img, uint *hist_eq) {
    if (!img || !img->data || !hist_eq) return;

    for (int i = 0; i < img->width * img->height; i++) {
        img->data[i] = (unsigned char)hist_eq[img->data[i]];
    }
}

// Exemple d'utilisation (sans chargement réel d'image)
int main() {
    // Création d'une image de test (10x10 pixels)
    t_bmp8 img;
    img.width = 10;
    img.height = 10;
    img.data = malloc(100 * sizeof(unsigned char));

    // Remplissage avec des niveaux de gris entre 0 et 127
    for (int i = 0; i < 100; i++) {
        img.data[i] = i % 128;
    }

    // Traitement
    uint *hist = bmp8_computeHistogram(&img);
    uint *cdf = bmp8_computeCDF(hist);
    uint *hist_eq = computeEqualizedMapping(cdf, img.width * img.height);
    bmp8_equalize(&img, hist_eq);

    // Libération
    free(hist);
    free(cdf);
    free(hist_eq);
    free(img.data);

    return 0;
}

//
// Created by Boulhamane Amel on 02/05/2025.
//
