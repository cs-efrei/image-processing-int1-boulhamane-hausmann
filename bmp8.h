#ifndef BMP8_H
#define BMP8_H

// -------------------- HEADER ---------------------------
//  Name : bmp8.c
//  Goal : handle all prototypes and structures related to 8-bit images (loading, debugging, filters, informations...)
//  Authors : Amel Boulhamane and Tom Hausmann
// 
/// NOTE : Throughout the files, we use the @brief, @param and @return structure for more consistency in the comments 
//
// --------------------------------------------------------



#include <stdint.h>

// Define the t_bmp8 structure
typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    unsigned int colorDepth;
    unsigned int dataSize;
} t_bmp8;


t_bmp8 *bmp8_loadImage(const char *filename);
int bmp8_saveImage(const char *filename, t_bmp8 *img);
void bmp8_free(t_bmp8 *img);
void bmp8_printInfo(t_bmp8 *img);
void bmp8_negative(t_bmp8 *img);
void bmp8_brightness(t_bmp8 *img, int value);
void bmp8_threshold(t_bmp8 *img, int threshold);
void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize);
void applyFilters8(t_bmp8 *img);

#endif

