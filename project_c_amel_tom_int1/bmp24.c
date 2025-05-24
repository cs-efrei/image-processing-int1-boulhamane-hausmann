#include <stdio.h>
#include <stdlib.h>
#include "bmp24.h"
#include <math.h>


t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = (t_pixel **)malloc(height * sizeof(t_pixel *));
    if (!pixels) {
        fprintf(stderr, "Memory allocation failed for pixel rows.\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            fprintf(stderr, "Memory allocation failed for pixel row %d.\n", i);
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    if (!pixels) return;
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (!img) {
        fprintf(stderr, "Memory allocation failed for image.\n");
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    img->data = bmp24_allocateDataPixels(width, height);

    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (!img) return;
    bmp24_freeDataPixels(img->data, img->height);
    free(img);
}



t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (!img) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed for image.\n");
        return NULL;
    }

    // Read the BMP file header
    fread(&img->header.type, sizeof(uint16_t), 1, file);
    fread(&img->header.size, sizeof(uint32_t), 1, file);
    fread(&img->header.reserved1, sizeof(uint16_t), 1, file);
    fread(&img->header.reserved2, sizeof(uint16_t), 1, file);
    fread(&img->header.offset, sizeof(uint32_t), 1, file);

    // Read the BMP info header
    fread(&img->header_info.size, sizeof(uint32_t), 1, file);
    fread(&img->header_info.width, sizeof(int32_t), 1, file);
    fread(&img->header_info.height, sizeof(int32_t), 1, file);
    fread(&img->header_info.planes, sizeof(uint16_t), 1, file);
    fread(&img->header_info.bits, sizeof(uint16_t), 1, file);
    fread(&img->header_info.compression, sizeof(uint32_t), 1, file);
    fread(&img->header_info.imagesize, sizeof(uint32_t), 1, file);
    fread(&img->header_info.xresolution, sizeof(int32_t), 1, file);
    fread(&img->header_info.yresolution, sizeof(int32_t), 1, file);
    fread(&img->header_info.ncolors, sizeof(uint32_t), 1, file);
    fread(&img->header_info.importantcolors, sizeof(uint32_t), 1, file);

    // Debug: Print header information
    printf("File header type: 0x%X\n", img->header.type);
    printf("File header size: %u\n", img->header.size);
    printf("File header offset: %u\n", img->header.offset);
    printf("Image width: %d\n", img->header_info.width);
    printf("Image height: %d\n", img->header_info.height);
    printf("Bits per pixel: %u\n", img->header_info.bits);

    // Check if the file is a valid 24-bit BMP file
    if (img->header.type != 0x4D42 || img->header_info.bits != 24) {
        fprintf(stderr, "Error: Not a 24-bit BMP file.\n");
        free(img);
        fclose(file);
        return NULL;
    }

    img->width = img->header_info.width;
    img->height = abs(img->header_info.height); // Handle negative heights
    img->colorDepth = img->header_info.bits;
    img->data = bmp24_allocateDataPixels(img->width, img->height);

    if (!img->data) {
        free(img);
        fclose(file);
        return NULL;
    }

    // Move file pointer to the start of pixel data
    fseek(file, img->header.offset, SEEK_SET);

    // Read pixel data
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t bgr[3];
            if (fread(bgr, sizeof(uint8_t), 3, file) != 3) {
                fprintf(stderr, "Error reading pixel data.\n");
                bmp24_free(img);
                fclose(file);
                return NULL;
            }
            img->data[y][x].blue = bgr[0];
            img->data[y][x].green = bgr[1];
            img->data[y][x].red = bgr[2];
        }
    }

    fclose(file);
    return img;
}

void bmp24_applyFilter(t_bmp24 *img, float *kernel, int kernelSize) {
    if (!img || !img->data || !kernel) return;

    t_pixel **newData = malloc(img->height * sizeof(t_pixel *));
    for (int y = 0; y < img->height; y++) {
        newData[y] = malloc(img->width * sizeof(t_pixel));
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            newData[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);
        }
    }

    // Copy back
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x] = newData[y][x];
        }
        free(newData[y]);
    }
    free(newData);
}


int bmp24_saveImage(const char *filename, t_bmp24 *img){
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Error opening file: %s\n", filename);
        return 1;  // ERREUR
    }

    // Write BMP header (14 bytes)
    uint16_t type = 0x4D42;
    uint32_t offset = 54;
    uint32_t size = offset + (img->width * 3 + (4 - (img->width * 3 % 4)) % 4) * img->height;
    uint16_t reserved = 0;

    fwrite(&type, sizeof(uint16_t), 1, f);
    fwrite(&size, sizeof(uint32_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&offset, sizeof(uint32_t), 1, f);

    // Info header (40 bytes)
    uint32_t headerSize = 40;
    uint16_t planes = 1;
    uint16_t bits = 24;
    uint32_t compression = 0;
    uint32_t imageSize = size - offset;
    int32_t resolution = 2835;

    fwrite(&headerSize, sizeof(uint32_t), 1, f);
    fwrite(&img->width, sizeof(int32_t), 1, f);
    fwrite(&img->height, sizeof(int32_t), 1, f);
    fwrite(&planes, sizeof(uint16_t), 1, f);
    fwrite(&bits, sizeof(uint16_t), 1, f);
    fwrite(&compression, sizeof(uint32_t), 1, f);
    fwrite(&imageSize, sizeof(uint32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    fwrite(&compression, sizeof(uint32_t), 1, f);
    fwrite(&compression, sizeof(uint32_t), 1, f);

    // Write pixels
    int padding = (4 - (img->width * 3) % 4) % 4;
    unsigned char pad[3] = {0, 0, 0};

    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            unsigned char bgr[3] = {
                img->data[y][x].blue,
                img->data[y][x].green,
                img->data[y][x].red
            };
            fwrite(bgr, 1, 3, f);
        }
        fwrite(pad, 1, padding, f);
    }

    fclose(f);
    printf("Saved in %s\n", filename);
    return 0;  // SUCCÈS
}




void bmp24_negative(t_bmp24 *img) {
    if (!img) return;

    // Print original pixel values
    printf("Original pixel (0,0): R=%d, G=%d, B=%d\n",
           img->data[0][0].red,
           img->data[0][0].green,
           img->data[0][0].blue);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    if (!img) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue = gray;
        }
    }
    // Print modified pixel values
    printf("Negative pixel (0,0): R=%d, G=%d, B=%d\n",
           img->data[0][0].red,
           img->data[0][0].green,
           img->data[0][0].blue);
}

void bmp24_brightness(t_bmp24 *img, int value) {
    if (!img) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;

            // Clamp values to [0, 255]
            img->data[y][x].red = (r > 255) ? 255 : (r < 0 ? 0 : r);
            img->data[y][x].green = (g > 255) ? 255 : (g < 0 ? 0 : g);
            img->data[y][x].blue = (b > 255) ? 255 : (b < 0 ? 0 : b);
        }
    }
}

void bmp24_boxBlur(t_bmp24 *img) {
    if (!img) return;

    // Define the 2D kernel
    float kernel2D[3][3] = {
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
        {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0}
    };

    // Flatten the 2D kernel into a 1D array
    float kernel[9];
    int index = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[index++] = kernel2D[i][j];
        }
    }

    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);
    if (!copy) return;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            copy->data[y][x] = bmp24_convolution(img, x, y, kernel, 3);
        }
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}



void bmp24_gaussianBlur(t_bmp24 *img) {
    if (!img) return;

    float kernel[3][3] = {
        {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
        {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
        {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}
    };

    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);
    if (!copy) return;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            copy->data[y][x] = bmp24_convolution(img, x, y, (float *)kernel, 3);
        }
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}

void bmp24_outline(t_bmp24 *img) {
    if (!img) return;

    float kernel[3][3] = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}
    };

    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);
    if (!copy) return;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            copy->data[y][x] = bmp24_convolution(img, x, y, (float *)kernel, 3);
        }
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}

void bmp24_emboss(t_bmp24 *img) {
    if (!img) return;

    float kernel[3][3] = {
        {-2, -1, 0},
        {-1, 1, 1},
        {0, 1, 2}
    };

    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);
    if (!copy) return;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            copy->data[y][x] = bmp24_convolution(img, x, y, (float *)kernel, 3);
        }
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}

void bmp24_sharpen(t_bmp24 *img) {
    if (!img) return;

    float kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };

    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);
    if (!copy) return;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            copy->data[y][x] = bmp24_convolution(img, x, y, (float *)kernel, 3);
        }
    }

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}

t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float *kernel, int kernelSize) {
    int n = kernelSize / 2;
    float r = 0, g = 0, b = 0;

    for (int i = -n; i <= n; i++) {
        for (int j = -n; j <= n; j++) {
            int xi = x + i;
            int yj = y + j;

            if (xi >= 0 && xi < img->width && yj >= 0 && yj < img->height) {
                t_pixel p = img->data[yj][xi];
                float k = kernel[(i + n) * kernelSize + (j + n)];
                r += p.red * k;
                g += p.green * k;
                b += p.blue * k;
            }
        }
    }

    t_pixel result;
    result.red = (uint8_t)fmin(fmax(round(r), 0), 255);
    result.green = (uint8_t)fmin(fmax(round(g), 0), 255);
    result.blue = (uint8_t)fmin(fmax(round(b), 0), 255);

    return result;
}

