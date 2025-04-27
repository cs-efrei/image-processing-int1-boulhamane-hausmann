#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include "bmp24.h"

// Function to create a synthetic test image
t_bmp24* create_test_image(int width, int height) {
    t_bmp24 *img = bmp24_allocate(width, height, 24);
    if (img == NULL) {
        return NULL;
    }
    
    // Initialize BMP header with standard values
    img->header.type = BMP_TYPE;  // 'BM'
    img->header.size = HEADER_SIZE + INFO_SIZE + (width * height * 3); // Total file size
    img->header.reserved1 = 0;
    img->header.reserved2 = 0;
    img->header.offset = HEADER_SIZE + INFO_SIZE; // Offset to pixel data
    
    // Initialize info header
    img->header_info.size = INFO_SIZE;  // Size of info header
    img->header_info.width = width;
    img->header_info.height = height;
    img->header_info.planes = 1;
    img->header_info.bits = 24;
    img->header_info.compression = 0;  // No compression
    img->header_info.imagesize = width * height * 3;
    img->header_info.xresolution = 2835; // 72 DPI
    img->header_info.yresolution = 2835; // 72 DPI
    img->header_info.ncolors = 0;
    img->header_info.importantcolors = 0;
    
    // Create a gradient pattern
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create gradient pattern with proper floating-point division
            img->data[y][x].red = (uint8_t)(((float)x / (float)width) * 255.0f);
            img->data[y][x].green = (uint8_t)(((float)y / (float)height) * 255.0f);
            img->data[y][x].blue = (uint8_t)(((float)(x + y) / (float)(width + height)) * 255.0f);
        }
    }
    
    return img;
}

int main() {
    // Part 1: Test BMP8 functionality (existing code)
    const char *inputFilename8 = "lena_gray_8bit.bmp";
    printf("===== Testing 8-bit BMP processing =====\n");
    t_bmp8 *image8 = bmp8_loadImage(inputFilename8);
    if (image8 == NULL) {
        printf("Failed to load 8-bit image: %s\n", inputFilename8);
        printf("Skipping 8-bit image tests...\n\n");
    } else {
        bmp8_printInfo(image8);

        bmp8_negative(image8);
        bmp8_saveImage("image_negative.bmp", image8);

        bmp8_changeBrightness(image8, 50);
        bmp8_saveImage("image_brightness.bmp", image8);

        bmp8_threshold(image8, 128);
        bmp8_saveImage("image_threshold.bmp", image8);

        int kernelSize = 3;
        float **boxBlur = malloc(kernelSize * sizeof(float *));
        for (int i = 0; i < kernelSize; i++) {
            boxBlur[i] = malloc(kernelSize * sizeof(float));
            for (int j = 0; j < kernelSize; j++) {
                boxBlur[i][j] = 1.0f / 9.0f;
            }
        }

        bmp8_applyFilter(image8, boxBlur, kernelSize);
        bmp8_saveImage("image_filtered.bmp", image8);

        for (int i = 0; i < kernelSize; i++) {
            free(boxBlur[i]);
        }
        free(boxBlur);

        bmp8_free(image8);
        printf("8-bit image processing completed!\n\n");
    }

    // Part 2: Test BMP24 functionality
    printf("===== Testing 24-bit BMP processing =====\n");
    const char *inputFilename24 = "lena_color.bmp";
    printf("Attempting to load image from: %s\n", inputFilename24);
    
    // Check if file exists before trying to load it
    FILE *check = fopen(inputFilename24, "rb");
    if (check == NULL) {
        printf("File not found: %s\n", inputFilename24);
        printf("Please make sure the image file is in the current working directory.\n");
        // You might want to try with an absolute path if needed
        // inputFilename24 = "/full/path/to/lena_color.bmp";
    } else {
        fclose(check);
    }
    
    t_bmp24 *image24 = bmp24_loadImage(inputFilename24);
    
    if (image24 == NULL) {
        printf("Failed to load 24-bit image: %s\n", inputFilename24);
        printf("Creating a synthetic test image instead...\n");
        
        // Create a synthetic image if we can't load a file
        image24 = create_test_image(400, 300);
        if (image24 == NULL) {
            printf("Failed to create synthetic image. Exiting.\n");
            return 1;
        }
        printf("Created a synthetic 400x300 gradient image.\n");
    }

    // Print basic image information
    printf("24-bit Image Info:\n");
    printf("    Width: %d\n", image24->width);
    printf("    Height: %d\n", image24->height);
    printf("    Color Depth: %d\n", image24->colorDepth);
    
    // Save a copy of the original image
    bmp24_saveImage(image24, "test_original.bmp");
    printf("Created original image: test_original.bmp\n");
    
    // Example pixel manipulation: Set top-left corner (50x50) to red
    printf("Setting top-left corner (50x50) to red...\n");
    for (int y = 0; y < 50 && y < image24->height; y++) {
        for (int x = 0; x < 50 && x < image24->width; x++) {
            image24->data[y][x].red = 255;
            image24->data[y][x].green = 0;
            image24->data[y][x].blue = 0;
        }
    }
    bmp24_saveImage(image24, "test_red_corner.bmp");
    
    // Example pixel manipulation: Swap red and blue channels
    printf("Creating a version with red and blue channels swapped...\n");
    for (int y = 0; y < image24->height; y++) {
        for (int x = 0; x < image24->width; x++) {
            uint8_t temp = image24->data[y][x].red;
            image24->data[y][x].red = image24->data[y][x].blue;
            image24->data[y][x].blue = temp;
        }
    }
    bmp24_saveImage(image24, "test_rb_swapped.bmp");
    
    // Debug: Print some pixel values to verify they're not all black
    printf("\nDebug - Sample pixel values:\n");
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            printf("Pixel at (%d,%d): R=%d, G=%d, B=%d\n", 
                   x, y,
                   image24->data[y][x].red,
                   image24->data[y][x].green,
                   image24->data[y][x].blue);
        }
    }

    // Free the 24-bit image
    bmp24_free(image24);
    printf("24-bit image processing completed!\n");
    
    // Test basic image processing functions
    printf("\n===== Testing 24-bit Image Processing Functions =====\n");
    
    // Create a fresh test image
    t_bmp24 *testImg = create_test_image(400, 300);
    
    // Test negative
    printf("Applying negative filter...\n");
    bmp24_negative(testImg);
    bmp24_saveImage(testImg, "test_negative.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Test grayscale
    printf("Converting to grayscale...\n");
    bmp24_grayscale(testImg);
    bmp24_saveImage(testImg, "test_grayscale.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Test brightness
    printf("Increasing brightness...\n");
    bmp24_brightness(testImg, 50);
    bmp24_saveImage(testImg, "test_brighter.bmp");
    
    printf("Decreasing brightness...\n");
    bmp24_brightness(testImg, -75);  // Darker from the brightened image
    bmp24_saveImage(testImg, "test_darker.bmp");
    
    // Reset image for filter tests
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Test convolution filters
    printf("\n===== Testing Convolution Filters =====\n");
    
    // Box blur
    printf("Applying box blur filter...\n");
    // Print a sample before filter
    printf("Sample before box blur - Pixel at (100,100): R=%d, G=%d, B=%d\n", 
           testImg->data[100][100].red,
           testImg->data[100][100].green,
           testImg->data[100][100].blue);
    
    bmp24_boxBlur(testImg);
    
    // Print a sample after filter
    printf("Sample after box blur - Pixel at (100,100): R=%d, G=%d, B=%d\n", 
           testImg->data[100][100].red,
           testImg->data[100][100].green,
           testImg->data[100][100].blue);
    
    bmp24_saveImage(testImg, "test_box_blur.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Continue with other tests with debugging
    // Gaussian blur
    printf("Applying gaussian blur filter...\n");
    bmp24_gaussianBlur(testImg);
    bmp24_saveImage(testImg, "test_gaussian_blur.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Outline
    printf("Applying outline filter...\n");
    bmp24_outline(testImg);
    bmp24_saveImage(testImg, "test_outline.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Emboss
    printf("Applying emboss filter...\n");
    bmp24_emboss(testImg);
    bmp24_saveImage(testImg, "test_emboss.bmp");
    
    // Reset image
    bmp24_free(testImg);
    testImg = create_test_image(400, 300);
    
    // Sharpen
    printf("Applying sharpen filter...\n");
    bmp24_sharpen(testImg);
    bmp24_saveImage(testImg, "test_sharpen.bmp");
    
    // Free memory
    bmp24_free(testImg);
    
    printf("\nAll image processing tests completed!\n");
    
    printf("\nAll processing complete!\n");
    return 0;
}
