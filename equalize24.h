// equalize24.h
#ifndef EQUALIZE24_H
#define EQUALIZE24_H

#include "bmp24.h"

// Perform histogram equalization on the luminance (Y) channel of a 24-bit image.
// This will boost contrast while preserving color.
void bmp24_equalize(t_bmp24 *img);

#endif // EQUALIZE24_H

