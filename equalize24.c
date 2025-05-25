// equalize24.c
#include "equalize24.h"
#include <stdlib.h>
#include <math.h>

// -------------------- HEADER ---------------------------
//  Name : equalize24.c
//  Goal : handle histogram equalization functions for 24-bit color images using YUV color space
//  Authors : Amel Boulhamane and Tom Hausmann
// 
/// NOTE : Throughout the files, we use the @brief, @param and @return structure for more consistency in the comments 
//
// --------------------------------------------------------

// Converter from RGB to YUV
/// @brief Converts RGB color values to YUV color space.
/// @param R Red component (0-255).
/// @param G Green component (0-255).
/// @param B Blue component (0-255).
/// @param Y Pointer to store luminance component.
/// @param U Pointer to store U  component.
/// @param V Pointer to store V  component.
static void rgb2yuv(uint8_t R, uint8_t G, uint8_t B,
                    float *Y, float *U, float *V)
{
    *Y =  0.299f * R + 0.587f * G + 0.114f * B;
    *U = -0.14713f * R - 0.28886f * G + 0.436f * B;
    *V =  0.615f * R - 0.51499f * G - 0.10001f * B;
}

// Inverse conversion
/// @brief Converts YUV color values back to RGB color space.
/// @param Y Luminance component.
/// @param U U  component.
/// @param V V  component.
/// @param R Pointer to store red component (0-255).
/// @param G Pointer to store green component (0-255).
/// @param B Pointer to store blue component (0-255).
static void yuv2rgb(float Y, float U, float V,
                    uint8_t *R, uint8_t *G, uint8_t *B)
{
    int r = (int)roundf(Y + 1.13983f * V);
    int g = (int)roundf(Y - 0.39465f * U - 0.58060f * V);
    int b = (int)roundf(Y + 2.03211f * U);

    *R = (uint8_t)(r < 0 ? 0 : (r > 255 ? 255 : r));
    *G = (uint8_t)(g < 0 ? 0 : (g > 255 ? 255 : g));
    *B = (uint8_t)(b < 0 ? 0 : (b > 255 ? 255 : b));
}

/// @brief Applies histogram equalization to 24-bit color image using YUV color space.
/// @param img Pointer to 24-bit BMP image to equalize.
void bmp24_equalize(t_bmp24 *img) {
    if (!img || !img->data) return;

    int w = img->width, h = img->height;
    int N = w * h;

    // 1) Helps build histogram for the Y channel
    unsigned int *hist = calloc(256, sizeof(unsigned int));
    if (!hist) return;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            t_pixel p = img->data[y][x];
            float Yf, Uf, Vf;
            rgb2yuv(p.red, p.green, p.blue, &Yf, &Uf, &Vf);
            int Yi = (int)roundf(Yf);
            if (Yi < 0) Yi = 0; else if (Yi > 255) Yi = 255;
            hist[Yi]++;
        }
    }

    // 2) Computation of the CDF
    unsigned int *cdf = calloc(256, sizeof(unsigned int));
    if (!cdf) { free(hist); return; }
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }
    // Finding the non 0 minimum
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) { cdf_min = cdf[i]; break; }
    }

    // 3) Build lookup table
    uint8_t map[256];
    for (int i = 0; i < 256; i++) {
        map[i] = (uint8_t)roundf(((float)(cdf[i] - cdf_min) / (float)(N - cdf_min)) * 255.0f);
    }

    // 4) Mapping back to the image
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            t_pixel p = img->data[y][x];
            float Yf, Uf, Vf;
            rgb2yuv(p.red, p.green, p.blue, &Yf, &Uf, &Vf);
            int Yi = (int)roundf(Yf);
            if (Yi < 0) Yi = 0; else if (Yi > 255) Yi = 255;

            // While keeping the U/V replacing the Y with the equalized Y
            yuv2rgb(map[Yi], Uf, Vf,
                     &img->data[y][x].red,
                     &img->data[y][x].green,
                     &img->data[y][x].blue);
        }
    }

    free(hist);
    free(cdf);
}
