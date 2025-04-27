#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include "bmp24.h"
#include <math.h>

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
    
    // First, fill the whole image with a gradient background
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create a subtle gradient for background
            img->data[y][x].red = (uint8_t)(((float)x / (float)width) * 150.0f + 50);
            img->data[y][x].green = (uint8_t)(((float)y / (float)height) * 150.0f + 50);
            img->data[y][x].blue = 200;
        }
    }
    
    // Draw some geometric shapes for better filter visibility
    
    // 1. Draw a white square in the center
    int square_size = width / 4;
    int square_x = width / 2 - square_size / 2;
    int square_y = height / 2 - square_size / 2;
    
    for (int y = square_y; y < square_y + square_size; y++) {
        for (int x = square_x; x < square_x + square_size; x++) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                img->data[y][x].red = 255;
                img->data[y][x].green = 255;
                img->data[y][x].blue = 255;
            }
        }
    }
    
    // 2. Draw a red circle in the top left
    int circle_radius = width / 8;
    int circle_x = width / 4;
    int circle_y = height / 4;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate distance from center of circle
            float dx = x - circle_x;
            float dy = y - circle_y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < circle_radius) {
                img->data[y][x].red = 255;
                img->data[y][x].green = 0;
                img->data[y][x].blue = 0;
            }
        }
    }
    
    // 3. Draw a green circle in the bottom right
    circle_x = 3 * width / 4;
    circle_y = 3 * height / 4;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate distance from center of circle
            float dx = x - circle_x;
            float dy = y - circle_y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < circle_radius) {
                img->data[y][x].red = 0;
                img->data[y][x].green = 255;
                img->data[y][x].blue = 0;
            }
        }
    }
    
    // 4. Draw some lines for edge detection tests
    for (int i = 0; i < width; i++) {
        int y = height / 3;
        if (i >= 0 && i < width && y >= 0 && y < height) {
            img->data[y][i].red = 0;
            img->data[y][i].green = 0;
            img->data[y][i].blue = 0;
        }
    }
    
    for (int i = 0; i < height; i++) {
        int x = 2 * width / 3;
        if (x >= 0 && x < width && i >= 0 && i < height) {
            img->data[i][x].red = 0;
            img->data[i][x].green = 0;
            img->data[i][x].blue = 0;
        }
    }
    
    return img;
}

int main() {
    // Part 1: Test BMP8 functionality - keep this unchanged
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
    t_bmp24 *originalImage = NULL;  // This will store our original image for later use
    
    if (check == NULL) {
        printf("File not found: %s\n", inputFilename24);
        printf("Creating a synthetic test image instead...\n");
        originalImage = create_test_image(400, 300);
        if (originalImage == NULL) {
            printf("Failed to create synthetic image. Exiting.\n");
            return 1;
        }
        printf("Created a synthetic 400x300 gradient image.\n");
    } else {
        fclose(check);
        originalImage = bmp24_loadImage(inputFilename24);
        if (originalImage == NULL) {
            printf("Failed to load 24-bit image: %s\n", inputFilename24);
            printf("Creating a synthetic test image instead...\n");
            originalImage = create_test_image(400, 300);
            if (originalImage == NULL) {
                printf("Failed to create synthetic image. Exiting.\n");
                return 1;
            }
        } else {
            printf("Successfully loaded %s\n", inputFilename24);
        }
    }
    
    // Make a working copy of the original image for the first set of tests
    t_bmp24 *image24 = bmp24_copy(originalImage);
    if (image24 == NULL) {
        printf("Failed to copy image. Exiting.\n");
        bmp24_free(originalImage);
        return 1;
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

    // Free the 24-bit image working copy
    bmp24_free(image24);
    printf("24-bit image processing completed!\n");
    
    // Test basic image processing functions
    printf("\n===== Testing 24-bit Image Processing Functions =====\n");
    
    // Test negative - use a copy of the original image
    printf("Applying negative filter...\n");
    t_bmp24 *testImg = bmp24_copy(originalImage);
    if (testImg == NULL) {
        printf("Failed to copy image. Exiting.\n");
        bmp24_free(originalImage);
        return 1;
    }
    bmp24_negative(testImg);
    bmp24_saveImage(testImg, "test_negative.bmp");
    bmp24_free(testImg);
    
    // Test grayscale - use a copy of the original image
    printf("Converting to grayscale...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_grayscale(testImg);
        bmp24_saveImage(testImg, "test_grayscale.bmp");
        bmp24_free(testImg);
    }
    
    // Test brightness - use a copy of the original image
    printf("Increasing brightness...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_brightness(testImg, 50);
        bmp24_saveImage(testImg, "test_brighter.bmp");
        
        printf("Decreasing brightness...\n");
        bmp24_brightness(testImg, -75);  // Darker from the brightened image
        bmp24_saveImage(testImg, "test_darker.bmp");
        bmp24_free(testImg);
    }
    
    // Test convolution filters
    printf("\n===== Testing Convolution Filters =====\n");
    
    // Box blur
    printf("Applying box blur filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        t_bmp24 *beforeBlur = bmp24_copy(testImg);
        bmp24_boxBlur(testImg);
        bmp24_saveImage(testImg, "test_box_blur.bmp");
        
        // Compare before/after pixel values for debugging
        if (beforeBlur != NULL) {
            printf("Sample comparison before/after blur:\n");
            int sampleX = testImg->width / 2;
            int sampleY = testImg->height / 2;
            printf("Before - Pixel at (%d,%d): R=%d, G=%d, B=%d\n", 
                   sampleX, sampleY,
                   beforeBlur->data[sampleY][sampleX].red,
                   beforeBlur->data[sampleY][sampleX].green,
                   beforeBlur->data[sampleY][sampleX].blue);
            printf("After  - Pixel at (%d,%d): R=%d, G=%d, B=%d\n", 
                   sampleX, sampleY,
                   testImg->data[sampleY][sampleX].red,
                   testImg->data[sampleY][sampleX].green,
                   testImg->data[sampleY][sampleX].blue);
            bmp24_free(beforeBlur);
        }
        bmp24_free(testImg);
    }
    
    // Gaussian blur
    printf("Applying gaussian blur filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_gaussianBlur(testImg);
        bmp24_saveImage(testImg, "test_gaussian_blur.bmp");
        bmp24_free(testImg);
    }
    
    // Outline
    printf("Applying outline filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_outline(testImg);
        bmp24_saveImage(testImg, "test_outline.bmp");
        bmp24_free(testImg);
    }
    
    // Emboss
    printf("Applying emboss filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_emboss(testImg);
        bmp24_saveImage(testImg, "test_emboss.bmp");
        bmp24_free(testImg);
    }
    
    // Sharpen
    printf("Applying sharpen filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_sharpen(testImg);
        bmp24_saveImage(testImg, "test_sharpen.bmp");
        bmp24_free(testImg);
    }
    
    // Free the original image
    bmp24_free(originalImage);
    
    printf("\nAll image processing tests completed!\n");
    
    printf("\nAll processing complete!\n");
    return 0;
}
