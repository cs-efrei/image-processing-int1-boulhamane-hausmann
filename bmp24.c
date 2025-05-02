#include <stdio.h>
#include <stdlib.h>
#include "bmp24.h"
#include  <math.h>

typedef struct {
    float y;
    float u;
    float v;
} t_yuv;

t_yuv rgb_to_yuv(uint8_t r, uint8_t g, uint8_t b) {
    t_yuv yuv;
    yuv.y = 0.299f * r + 0.587f * g + 0.114f * b;
    yuv.u = -0.14713f * r - 0.28886f * g + 0.436f * b;
    yuv.v = 0.615f * r - 0.51499f * g - 0.10001f * b;
    return yuv;
}

// Convert YUV to RGB
t_pixel yuv_to_rgb(t_yuv yuv) {
    t_pixel rgb;
    float r = yuv.y + 1.13983f * yuv.v;
    float g = yuv.y - 0.39465f * yuv.u - 0.58060f * yuv.v;
    float b = yuv.y + 2.03211f * yuv.u;
    
    rgb.red = (uint8_t)(r < 0 ? 0 : (r > 255 ? 255 : round(r)));
    rgb.green = (uint8_t)(g < 0 ? 0 : (g > 255 ? 255 : round(g)));
    rgb.blue = (uint8_t)(b < 0 ? 0 : (b > 255 ? 255 : round(b)));
    
    return rgb;
}

// Histogram equalization for color images in YUV space
void bmp24_equalize(t_bmp24 *img) {
    if (img == NULL || img->data == NULL) return;
    
    int width = img->width;
    int height = img->height;
    int total_pixels = width * height;
    
    t_yuv **yuv_data = (t_yuv **)malloc(height * sizeof(t_yuv *));
    for (int i = 0; i < height; i++) {
        yuv_data[i] = (t_yuv *)malloc(width * sizeof(t_yuv));
        for (int j = 0; j < width; j++) {
            yuv_data[i][j] = rgb_to_yuv(
                img->data[i][j].red,
                img->data[i][j].green,
                img->data[i][j].blue
            );
        }
    }
    
    unsigned int *hist = (unsigned int *)calloc(256, sizeof(unsigned int));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int y_val = (int)round(yuv_data[i][j].y);
            y_val = (y_val < 0) ? 0 : (y_val > 255) ? 255 : y_val;
            hist[y_val]++;
        }
    }
    
    unsigned int *cdf = (unsigned int *)malloc(256 * sizeof(unsigned int));
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }
    
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            cdf_min = cdf[i];
            break;
        }
    }
    
    unsigned int *eq_mapping = (unsigned int *)malloc(256 * sizeof(unsigned int));
    for (int i = 0; i < 256; i++) {
        eq_mapping[i] = round(((float)(cdf[i] - cdf_min) / (total_pixels - cdf_min)) * 255.0f);
    }
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int y_val = (int)round(yuv_data[i][j].y);
            y_val = (y_val < 0) ? 0 : (y_val > 255) ? 255 : y_val;
            yuv_data[i][j].y = (float)eq_mapping[y_val];
        }
    }
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            img->data[i][j] = yuv_to_rgb(yuv_data[i][j]);
        }
    }
    
    for (int i = 0; i < height; i++) {
        free(yuv_data[i]);
    }
    free(yuv_data);
    free(hist);
    free(cdf);
    free(eq_mapping);
}

t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = (t_pixel **)malloc(height * sizeof(t_pixel *));
    if (pixels == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for pixel matrix\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
        if (pixels[i] == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for pixel row %d\n", i);
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
    if (pixels == NULL) return;
    
    for (int i = 0; i < height; i++) {
        if (pixels[i] != NULL) {
            free(pixels[i]);
        }
    }
    
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (img == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for BMP24 image\n");
        return NULL;
    }

    img->data = bmp24_allocateDataPixels(width, height);
    if (img->data == NULL) {
        fprintf(stderr, "Error: Failed to allocate pixel data\n");
        free(img);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img == NULL) return;
    
    if (img->data != NULL) {
        bmp24_freeDataPixels(img->data, img->height);
    }
    
    free(img);
}

void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

int calculateRowPadding(int width) {
    return (4 - ((width * 3) % 4)) % 4;
}

void bmp24_writePixelData(t_bmp24 *image, FILE *file) {
    int padding = calculateRowPadding(image->width);
    uint8_t padValue = 0;
    
    for (int y = 0; y < image->height; y++) {
        int bmpY = image->height - 1 - y;
        
        for (int x = 0; x < image->width; x++) {
            uint8_t bgr[3];
            bgr[0] = image->data[y][x].blue;
            bgr[1] = image->data[y][x].green;
            bgr[2] = image->data[y][x].red;
            
            fwrite(bgr, 3, 1, file);
        }
        
        for (int p = 0; p < padding; p++) {
            fwrite(&padValue, 1, 1, file);
        }
    }
}

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File opening error");
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return NULL;
    }
    
    t_bmp_header header;
    if (fread(&header.type, sizeof(uint16_t), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read file header\n");
        fclose(file);
        return NULL;
    }
    fread(&header.size, sizeof(uint32_t), 1, file);
    fread(&header.reserved1, sizeof(uint16_t), 1, file);
    fread(&header.reserved2, sizeof(uint16_t), 1, file);
    fread(&header.offset, sizeof(uint32_t), 1, file);
    
    printf("Debug - BMP Header: Type=%x, Size=%u, Offset=%u\n", 
           header.type, header.size, header.offset);
    
    if (header.type != BMP_TYPE) {
        fprintf(stderr, "Error: Not a valid BMP file (magic number mismatch: %x)\n", header.type);
        fclose(file);
        return NULL;
    }
    
    t_bmp_info info;
    fread(&info, sizeof(t_bmp_info), 1, file);
    
    if (info.bits != 24) {
        fprintf(stderr, "Error: Not a 24-bit BMP image (found %d-bit)\n", info.bits);
        fclose(file);
        return NULL;
    }
    
    t_bmp24 *image = bmp24_allocate(info.width, info.height, info.bits);
    if (image == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for image\n");
        fclose(file);
        return NULL;
    }
    
    image->header = header;
    image->header_info = info;
    
    int padding = (4 - ((info.width * 3) % 4)) % 4;
    
    fseek(file, header.offset, SEEK_SET);
    
    for (int y = info.height - 1; y >= 0; y--) {
        for (int x = 0; x < info.width; x++) {
            uint8_t bgr[3];
            fread(bgr, 1, 3, file);
            
            image->data[y][x].blue = bgr[0];
            image->data[y][x].green = bgr[1];
            image->data[y][x].red = bgr[2];
        }
        
        fseek(file, padding, SEEK_CUR);
    }
    
    fclose(file);
    return image;
}

void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    if (img == NULL) {
        fprintf(stderr, "Error: NULL image pointer\n");
        return;
    }
    
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file %s for writing\n", filename);
        return;
    }
    
    int padding = (4 - ((img->width * 3) % 4)) % 4;
    uint32_t dataSize = (img->width * 3 + padding) * img->height;
    uint32_t fileSize = HEADER_SIZE + INFO_SIZE + dataSize;
    
    img->header.type = BMP_TYPE;
    img->header.size = fileSize;
    img->header.reserved1 = 0;
    img->header.reserved2 = 0;
    img->header.offset = HEADER_SIZE + INFO_SIZE;
    
    img->header_info.size = INFO_SIZE;
    img->header_info.width = img->width;
    img->header_info.height = img->height;
    img->header_info.planes = 1;
    img->header_info.bits = 24;
    img->header_info.compression = 0;
    img->header_info.imagesize = dataSize;
    img->header_info.xresolution = 2835;
    img->header_info.yresolution = 2835;
    img->header_info.ncolors = 0;
    img->header_info.importantcolors = 0;
    
    fwrite(&img->header.type, sizeof(uint16_t), 1, file);
    fwrite(&img->header.size, sizeof(uint32_t), 1, file);
    fwrite(&img->header.reserved1, sizeof(uint16_t), 1, file);
    fwrite(&img->header.reserved2, sizeof(uint16_t), 1, file);
    fwrite(&img->header.offset, sizeof(uint32_t), 1, file);
    
    fwrite(&img->header_info, sizeof(t_bmp_info), 1, file);
    
    uint8_t padValue = 0;
    
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            fputc(img->data[y][x].blue, file);
            fputc(img->data[y][x].green, file);
            fputc(img->data[y][x].red, file);
        }
        
        for (int p = 0; p < padding; p++) {
            fputc(padValue, file);
        }
    }
    
    fclose(file);
    printf("Image saved to: %s\n", filename);
}

void bmp24_negative(t_bmp24 *img) {
    if (img == NULL || img->data == NULL) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    if (img == NULL || img->data == NULL) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t avg = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            
            img->data[y][x].red = avg;
            img->data[y][x].green = avg;
            img->data[y][x].blue = avg;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    if (img == NULL || img->data == NULL) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int red = img->data[y][x].red + value;
            int green = img->data[y][x].green + value;
            int blue = img->data[y][x].blue + value;
            
            img->data[y][x].red = (red > 255) ? 255 : (red < 0) ? 0 : red;
            img->data[y][x].green = (green > 255) ? 255 : (green < 0) ? 0 : green;
            img->data[y][x].blue = (blue > 255) ? 255 : (blue < 0) ? 0 : blue;
        }
    }
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    if (img == NULL || img->data == NULL || kernel == NULL) return;
    
    int width = img->width;
    int height = img->height;
    int n = kernelSize / 2;
    
    t_pixel **tempData = bmp24_allocateDataPixels(width, height);
    if (tempData == NULL) {
        fprintf(stderr, "Failed to allocate memory for image processing\n");
        return;
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tempData[y][x] = img->data[y][x];
        }
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float red_sum = 0.0f;
            float green_sum = 0.0f;
            float blue_sum = 0.0f;
            
            for (int ky = -n; ky <= n; ky++) {
                for (int kx = -n; kx <= n; kx++) {
                    int sy = y + ky;
                    int sx = x + kx;
                    
                    if (sy < 0) sy = 0;
                    if (sx < 0) sx = 0;
                    if (sy >= height) sy = height - 1;
                    if (sx >= width) sx = width - 1;
                    
                    float weight = kernel[ky + n][kx + n];
                    
                    red_sum += tempData[sy][sx].red * weight;
                    green_sum += tempData[sy][sx].green * weight;
                    blue_sum += tempData[sy][sx].blue * weight;
                }
            }
            
            img->data[y][x].red   = (red_sum > 255.0f)   ? 255 : (red_sum < 0.0f)   ? 0 : (uint8_t)red_sum;
            img->data[y][x].green = (green_sum > 255.0f) ? 255 : (green_sum < 0.0f) ? 0 : (uint8_t)green_sum;
            img->data[y][x].blue  = (blue_sum > 255.0f)  ? 255 : (blue_sum < 0.0f)  ? 0 : (uint8_t)blue_sum;
        }
    }
    
    bmp24_freeDataPixels(tempData, height);
}

void bmp24_boxBlur(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = 1.0f / 9.0f;
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    int kernelSize = 5;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    float gaussianKernel[5][5] = {
        {1.0f/256, 4.0f/256,  6.0f/256,  4.0f/256,  1.0f/256},
        {4.0f/256, 16.0f/256, 24.0f/256, 16.0f/256, 4.0f/256},
        {6.0f/256, 24.0f/256, 36.0f/256, 24.0f/256, 6.0f/256},
        {4.0f/256, 16.0f/256, 24.0f/256, 16.0f/256, 4.0f/256},
        {1.0f/256, 4.0f/256,  6.0f/256,  4.0f/256,  1.0f/256}
    };
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = gaussianKernel[i][j];
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_outline(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    float outlineKernel[3][3] = {
        {-1.0f, -1.0f, -1.0f},
        {-1.0f,  8.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f}
    };
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = outlineKernel[i][j];
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_emboss(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    float embossKernel[3][3] = {
        {-2.0f, -1.0f,  0.0f},
        {-1.0f,  1.0f,  1.0f},
        { 0.0f,  1.0f,  2.0f}
    };
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = embossKernel[i][j];
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_sharpen(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    float sharpenKernel[3][3] = {
        { 0.0f, -1.0f,  0.0f},
        {-1.0f,  5.0f, -1.0f},
        { 0.0f, -1.0f,  0.0f}
    };
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = sharpenKernel[i][j];
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

t_bmp24 * bmp24_copy(t_bmp24 * src) {
    if (src == NULL) return NULL;
    
    t_bmp24 *dst = bmp24_allocate(src->width, src->height, src->colorDepth);
    if (dst == NULL) return NULL;
    
    dst->header = src->header;
    dst->header_info = src->header_info;
    
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            dst->data[y][x] = src->data[y][x];
        }
    }
    
    return dst;
}