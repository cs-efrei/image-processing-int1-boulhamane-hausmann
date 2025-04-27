//
// Created by Boulhamane Amel on 23/04/2025.
//

#ifndef BMP8_H
#define BMP8_H

#define BITMAP_MAGIC 0x00 // offset 0
#define BITMAP_SIZE 0x02 // offset 2
#define BITMAP_OFFSET 0x0A // offset 10

typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char *data;
    unsigned char **pixels;
    unsigned int width;
    unsigned int height;
    unsigned short colorDepth;
    unsigned int dataSize;
} t_bmp8;


t_bmp8 * bmp8_loadImage(const char * filename);
void bmp8_saveImage(const char * filename, t_bmp8 * image);
void bmp8_free(t_bmp8 * image);
void bmp8_printInfo(t_bmp8 * image);

void bmp8_negative(t_bmp8 * image);
void bmp8_changeBrightness(t_bmp8 * image, int brightnessChange);
void bmp8_threshold(t_bmp8 * image, int threshold);
void bmp8_applyFilter(t_bmp8 * image, float **kernel, int kernelSize);

#endif //BMP8_H
