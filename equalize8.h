#ifndef EQUALIZE8_H
#define EQUALIZE8_H

#include "bmp8.h"

// Computes the histogram for 8 bit grayscale images
unsigned int *bmp8_computeHistogram(t_bmp8 *img);

// Computes CDF from a given histogram
unsigned int *bmp8_computeCDF(unsigned int *hist, unsigned int total_pixels);

// Applies histogram equalization to the grayscale image
void bmp8_equalize(t_bmp8 *img);

#endif // EQUALIZE8_H



