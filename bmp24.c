#include <stdio.h>
#include <stdlib.h>
#include "bmp24.h"

// Memory allocation and deallocation functions
t_pixel **bmp24_allocateDataPixels(int width, int height) {
    // Allocate array of row pointers
    t_pixel **pixels = (t_pixel **)malloc(height * sizeof(t_pixel *));
    if (pixels == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for pixel matrix\n");
        return NULL;
    }

    // Allocate each row
    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
        if (pixels[i] == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for pixel row %d\n", i);
            // Free already allocated rows
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
    
    // Free each row
    for (int i = 0; i < height; i++) {
        if (pixels[i] != NULL) {
            free(pixels[i]);
        }
    }
    
    // Free the array of pointers
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    // Allocate the t_bmp24 structure
    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (img == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for BMP24 image\n");
        return NULL;
    }

    // Allocate the pixel data
    img->data = bmp24_allocateDataPixels(width, height);
    if (img->data == NULL) {
        fprintf(stderr, "Error: Failed to allocate pixel data\n");
        free(img);
        return NULL;
    }

    // Initialize fields
    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img == NULL) return;
    
    // Free pixel data
    if (img->data != NULL) {
        bmp24_freeDataPixels(img->data, img->height);
    }
    
    // Free the image structure itself
    free(img);
}

// Helper functions for raw file operations
void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

// Calculate row padding for BMP files (each row must be a multiple of 4 bytes)
int calculateRowPadding(int width) {
    return (4 - ((width * 3) % 4)) % 4;
}

void bmp24_writePixelData(t_bmp24 *image, FILE *file) {
    // Calculate row padding
    int padding = calculateRowPadding(image->width);
    uint8_t padValue = 0;
    
    // Write pixel data row by row
    for (int y = 0; y < image->height; y++) {
        // In BMP format, image is stored bottom-to-top
        int bmpY = image->height - 1 - y;
        
        for (int x = 0; x < image->width; x++) {
            // Write in BGR format (BMP standard)
            uint8_t bgr[3];
            bgr[0] = image->data[y][x].blue;   // B
            bgr[1] = image->data[y][x].green;  // G
            bgr[2] = image->data[y][x].red;    // R
            
            fwrite(bgr, 3, 1, file);
        }
        
        // Write padding bytes
        for (int p = 0; p < padding; p++) {
            fwrite(&padValue, 1, 1, file);
        }
    }
}

// Loading and Saving functions
t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File opening error");
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return NULL;
    }
    
    // Read BMP header
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
    
    // Debug header information
    printf("Debug - BMP Header: Type=%x, Size=%u, Offset=%u\n", 
           header.type, header.size, header.offset);
    
    // Verify this is a BMP file
    if (header.type != BMP_TYPE) {
        fprintf(stderr, "Error: Not a valid BMP file (magic number mismatch: %x)\n", header.type);
        fclose(file);
        return NULL;
    }
    
    // Read info header
    t_bmp_info info;
    fread(&info, sizeof(t_bmp_info), 1, file);
    
    // Verify this is a 24-bit image
    if (info.bits != 24) {
        fprintf(stderr, "Error: Not a 24-bit BMP image (found %d-bit)\n", info.bits);
        fclose(file);
        return NULL;
    }
    
    // Allocate image structure
    t_bmp24 *image = bmp24_allocate(info.width, info.height, info.bits);
    if (image == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for image\n");
        fclose(file);
        return NULL;
    }
    
    // Copy header information
    image->header = header;
    image->header_info = info;
    
    // Calculate row padding (each row must be aligned to 4-byte boundary)
    int padding = (4 - ((info.width * 3) % 4)) % 4;
    
    // Seek to the beginning of pixel data
    fseek(file, header.offset, SEEK_SET);
    
    // Read pixel data (BMP stores data bottom-to-top)
    for (int y = info.height - 1; y >= 0; y--) {
        for (int x = 0; x < info.width; x++) {
            uint8_t bgr[3];
            fread(bgr, 1, 3, file);
            
            // Convert BGR to RGB for our internal structure
            image->data[y][x].blue = bgr[0];
            image->data[y][x].green = bgr[1];
            image->data[y][x].red = bgr[2];
        }
        
        // Skip padding bytes
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
    
    // Calculate row padding and file sizes
    int padding = (4 - ((img->width * 3) % 4)) % 4;
    uint32_t dataSize = (img->width * 3 + padding) * img->height;
    uint32_t fileSize = HEADER_SIZE + INFO_SIZE + dataSize;
    
    // Update header fields
    img->header.type = BMP_TYPE;
    img->header.size = fileSize;
    img->header.reserved1 = 0;
    img->header.reserved2 = 0;
    img->header.offset = HEADER_SIZE + INFO_SIZE;
    
    // Update info header fields
    img->header_info.size = INFO_SIZE;
    img->header_info.width = img->width;
    img->header_info.height = img->height;
    img->header_info.planes = 1;
    img->header_info.bits = 24;
    img->header_info.compression = 0;
    img->header_info.imagesize = dataSize;
    img->header_info.xresolution = 2835; // ~72 DPI
    img->header_info.yresolution = 2835; // ~72 DPI
    img->header_info.ncolors = 0;
    img->header_info.importantcolors = 0;
    
    // Write BMP file header
    fwrite(&img->header.type, sizeof(uint16_t), 1, file);
    fwrite(&img->header.size, sizeof(uint32_t), 1, file);
    fwrite(&img->header.reserved1, sizeof(uint16_t), 1, file);
    fwrite(&img->header.reserved2, sizeof(uint16_t), 1, file);
    fwrite(&img->header.offset, sizeof(uint32_t), 1, file);
    
    // Write BMP info header
    fwrite(&img->header_info, sizeof(t_bmp_info), 1, file);
    
    // Write pixel data
    uint8_t padValue = 0;
    
    // BMP stores pixel data bottom-to-top
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            // Write BGR (not RGB)
            fputc(img->data[y][x].blue, file);
            fputc(img->data[y][x].green, file);
            fputc(img->data[y][x].red, file);
        }
        
        // Write padding
        for (int p = 0; p < padding; p++) {
            fputc(padValue, file);
        }
    }
    
    fclose(file);
    printf("Image saved to: %s\n", filename);
}

// Basic Image Processing Functions
void bmp24_negative(t_bmp24 *img) {
    if (img == NULL || img->data == NULL) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            // Invert each color channel (255 - value)
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
            // Calculate average of RGB values
            uint8_t avg = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            
            // Set all channels to the average value
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
            // Adjust brightness by adding value, capped at 255
            int red = img->data[y][x].red + value;
            int green = img->data[y][x].green + value;
            int blue = img->data[y][x].blue + value;
            
            // Ensure values are within 0-255 range
            img->data[y][x].red = (red > 255) ? 255 : (red < 0) ? 0 : red;
            img->data[y][x].green = (green > 255) ? 255 : (green < 0) ? 0 : green;
            img->data[y][x].blue = (blue > 255) ? 255 : (blue < 0) ? 0 : blue;
        }
    }
}

// Convolution filter functions - FIXED IMPLEMENTATION

// FIXED: Completely rewritten convolution filter application
void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    if (img == NULL || img->data == NULL || kernel == NULL) return;
    
    int width = img->width;
    int height = img->height;
    int n = kernelSize / 2;
    
    // Create a temporary copy of the image data
    t_pixel **tempData = bmp24_allocateDataPixels(width, height);
    if (tempData == NULL) {
        fprintf(stderr, "Failed to allocate memory for image processing\n");
        return;
    }
    
    // Copy original data to temp buffer
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tempData[y][x] = img->data[y][x];
        }
    }
    
    // Apply convolution to each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float red_sum = 0.0f;
            float green_sum = 0.0f;
            float blue_sum = 0.0f;
            
            // Apply convolution kernel
            for (int ky = -n; ky <= n; ky++) {
                for (int kx = -n; kx <= n; kx++) {
                    // Calculate source pixel coordinates with boundary checking
                    int sy = y + ky;
                    int sx = x + kx;
                    
                    // Handle boundary conditions - clamp to edge
                    if (sy < 0) sy = 0;
                    if (sx < 0) sx = 0;
                    if (sy >= height) sy = height - 1;
                    if (sx >= width) sx = width - 1;
                    
                    // Get kernel weight
                    float weight = kernel[ky + n][kx + n];
                    
                    // Apply kernel weight to each color channel
                    red_sum += tempData[sy][sx].red * weight;
                    green_sum += tempData[sy][sx].green * weight;
                    blue_sum += tempData[sy][sx].blue * weight;
                }
            }
            
            // Clamp results to 0-255 range
            img->data[y][x].red   = (red_sum > 255.0f)   ? 255 : (red_sum < 0.0f)   ? 0 : (uint8_t)red_sum;
            img->data[y][x].green = (green_sum > 255.0f) ? 255 : (green_sum < 0.0f) ? 0 : (uint8_t)green_sum;
            img->data[y][x].blue  = (blue_sum > 255.0f)  ? 255 : (blue_sum < 0.0f)  ? 0 : (uint8_t)blue_sum;
        }
    }
    
    // Free temporary data
    bmp24_freeDataPixels(tempData, height);
}

// Specific filter implementations - update box blur for better results
void bmp24_boxBlur(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = (float *)malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] = 1.0f / 9.0f;  // Box blur: uniform 1/9 weight
        }
    }
    
    bmp24_applyFilter(img, kernel, kernelSize);
    
    // Free kernel memory
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    int kernelSize = 5;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    // Initialize 5x5 Gaussian kernel
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
    
    // Free kernel memory
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_outline(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    // Initialize outline/edge detection kernel
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
    
    // Free kernel memory
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_emboss(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    // Initialize emboss kernel
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
    
    // Free kernel memory
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_sharpen(t_bmp24 *img) {
    int kernelSize = 3;
    float **kernel = (float **)malloc(kernelSize * sizeof(float *));
    
    // Initialize sharpen kernel
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
    
    // Free kernel memory
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

// Update the copy function implementation
t_bmp24 * bmp24_copy(t_bmp24 * src) {
    if (src == NULL) return NULL;
    
    // Allocate a new image with the same dimensions
    t_bmp24 *dst = bmp24_allocate(src->width, src->height, src->colorDepth);
    if (dst == NULL) return NULL;
    
    // Copy header information
    dst->header = src->header;
    dst->header_info = src->header_info;
    
    // Copy pixel data
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            dst->data[y][x] = src->data[y][x];
        }
    }
    
    return dst;
}