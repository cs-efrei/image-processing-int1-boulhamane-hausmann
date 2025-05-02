


#ifndef BMP24_H
#define BMP24_H

#include "bmp8.h" // Include the header file that defines t_bmp8

unsigned int * bmp8_computeHistogram(t_bmp8 * img);
unsigned int * bmp8_computeCDF(unsigned int * hist);
void bmp8_equalize(t_bmp8 * img, unsigned int * hist_eq);


#endif