#include "equalize8.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Step 1: This will help us compute the histogram of grayscale immages
unsigned int *bmp8_computeHistogram(t_bmp8 *img) {
    if (!img || !img->data) return NULL;

    unsigned int *hist = calloc(256, sizeof(unsigned int));
    if (!hist) return NULL;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        hist[img->data[i]]++;
    }

    return hist;
}

// Step 2: This will help us compute the CDF
unsigned int *bmp8_computeCDF(unsigned int *hist, unsigned int total_pixels) {
    if (!hist || total_pixels == 0) return NULL;

    unsigned int *cdf = calloc(256, sizeof(unsigned int));
    if (!cdf) return NULL;

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // Finding the non 0 minimum value in the CDF
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    // Keep the CDF in the interval [0 , 255]
    for (int i = 0; i < 256; i++) {
        cdf[i] = round(((float)(cdf[i] - cdf_min) / (total_pixels - cdf_min)) * 255);
    }

    return cdf;
}

// Step 3: Applying the equalization to the chosen image
void bmp8_equalize(t_bmp8 *img) {
    if (!img || !img->data) return;

    unsigned int *hist = bmp8_computeHistogram(img);
    if (!hist) return;

    unsigned int *cdf = bmp8_computeCDF(hist, img->width * img->height);
    if (!cdf) {
        free(hist);
        return;
    }

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (unsigned char)cdf[img->data[i]];
    }

    free(hist);
    free(cdf);
}
