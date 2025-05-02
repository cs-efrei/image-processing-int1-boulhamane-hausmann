//
// Created by Boulhamane Amel on 02/05/2025.
//
#ifndef BMP8_EQUALIZATION_H
#define BMP8_EQUALIZATION_H

#include <stdlib.h>

// Structure de l'image BMP en niveaux de gris 8 bits
typedef struct {
    int width;
    int height;
    unsigned char *data;
} t_bmp8;

// Fonctions de traitement
unsigned int * bmp8_computeHistogram(t_bmp8 * img);
unsigned int * bmp8_computeCDF(unsigned int * hist);
unsigned int * computeEqualizedMapping(unsigned int *cdf, int total_pixels);
void bmp8_equalize(t_bmp8 * img, unsigned int * hist_eq);

#endif // BMP8_EQUALIZATION_H

#ifndef HISTOGRAM_EQUALIZATION_H
#define HISTOGRAM_EQUALIZATION_H

#endif //HISTOGRAM_EQUALIZATION_H
