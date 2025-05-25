#ifndef BMP24_H
#define BMP24_H

#include <stdint.h>

// Structure for BMP header
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} t_bmp_header;

// Structure for BMP info header
typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t imagesize;
    int32_t xresolution;
    int32_t yresolution;
    uint32_t ncolors;
    uint32_t importantcolors;
} t_bmp_info;

// Structure for a pixel
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} t_pixel;


typedef struct {
    t_bmp_header header;
    t_bmp_info header_info;
    int width;
    int height;
    int colorDepth;
    t_pixel **data;
} t_bmp24;

// Function declarations
t_pixel **bmp24_allocateDataPixels(int width, int height);
void bmp24_freeDataPixels(t_pixel **pixels, int height);
t_bmp24 *bmp24_allocate(int width, int height, int colorDepth);
void bmp24_free(t_bmp24 *img);
t_bmp24 *bmp24_loadImage(const char *filename);
int bmp24_saveImage(const char *filename, t_bmp24 *img);
void bmp24_negative(t_bmp24 *img);
void bmp24_grayscale(t_bmp24 *img);
void bmp24_brightness(t_bmp24 *img, int value);
void bmp24_boxBlur(t_bmp24 *img);
void bmp24_gaussianBlur(t_bmp24 *img);
void bmp24_outline(t_bmp24 *img);
void bmp24_emboss(t_bmp24 *img);
void bmp24_sharpen(t_bmp24 *img);
t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float *kernel, int kernelSize);
void bmp24_applyFilter(t_bmp24 *img, float *kernel, int kernelSize);

#endif // BMP24_H

