#include <stdio.h>
#include <stdlib.h>
#include "bmp8.c"
#include "bmp24.c"
#include "histogram_equalization.c"
#include <math.h>

t_bmp24* create_test_image(int width, int height);

t_bmp24* create_test_image(int width, int height) {
    t_bmp24 *img = bmp24_allocate(width, height, 24);
    if (img == NULL) {
        return NULL;
    }
    
    img->header.type = BMP_TYPE; 
    img->header.size = HEADER_SIZE + INFO_SIZE + (width * height * 3); 
    img->header.reserved1 = 0;
    img->header.reserved2 = 0;
    img->header.offset = HEADER_SIZE + INFO_SIZE; 

    img->header_info.size = INFO_SIZE;
    img->header_info.width = width;
    img->header_info.height = height;
    img->header_info.planes = 1;
    img->header_info.bits = 24;
    img->header_info.compression = 0;  
    img->header_info.imagesize = width * height * 3;
    img->header_info.xresolution = 2835; // 72 DPI
    img->header_info.yresolution = 2835; // 72 DPI
    img->header_info.ncolors = 0;
    img->header_info.importantcolors = 0;
    

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img->data[y][x].red = (uint8_t)(((float)x / (float)width) * 150.0f + 50);
            img->data[y][x].green = (uint8_t)(((float)y / (float)height) * 150.0f + 50);
            img->data[y][x].blue = 200;
        }
    }
    

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
    

    int circle_radius = width / 8;
    int circle_x = width / 4;
    int circle_y = height / 4;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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
    
    circle_x = 3 * width / 4;
    circle_y = 3 * height / 4;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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
    printf("What ");


    const char *inputFilename8 = "barbara_gray.bmp";
    printf("===== Testing 8-bit BMP processing (with the barbara image) =====\n");
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

    const char *inputFilename24 = "flowers_color.bmp";
    printf("Attempting to load image from: %s\n", inputFilename24);
    
    FILE *check = fopen(inputFilename24, "rb");
    t_bmp24 *originalImage = NULL;  
    
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
    
    t_bmp24 *image24 = bmp24_copy(originalImage);
    if (image24 == NULL) {
        printf("Failed to copy image. Exiting.\n");
        bmp24_free(originalImage);
        return 1;
    }

    printf("24-bit Image Info:\n");
    printf("    Width: %d\n", image24->width);
    printf("    Height: %d\n", image24->height);
    printf("    Color Depth: %d\n", image24->colorDepth);
    
    bmp24_saveImage(image24, "test_original.bmp");
    printf("Created original image: test_original.bmp\n");

    for (int y = 0; y < 50 && y < image24->height; y++) {
        for (int x = 0; x < 50 && x < image24->width; x++) {
            image24->data[y][x].red = 255;
            image24->data[y][x].green = 0;
            image24->data[y][x].blue = 0;
        }
    }
    bmp24_saveImage(image24, "test_red_corner.bmp");

    for (int y = 0; y < image24->height; y++) {
        for (int x = 0; x < image24->width; x++) {
            uint8_t temp = image24->data[y][x].red;
            image24->data[y][x].red = image24->data[y][x].blue;
            image24->data[y][x].blue = temp;
        }
    }
    bmp24_saveImage(image24, "test_rb_swapped.bmp");


    bmp24_free(image24);
    printf("24-bit image processing completed!\n");
    
    printf("\n===== Testing 24-bit Image Processing Functions =====\n");
    
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
    
    printf("Converting to grayscale...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_grayscale(testImg);
        bmp24_saveImage(testImg, "test_grayscale.bmp");
        bmp24_free(testImg);
    }
    
    printf("Increasing brightness...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_brightness(testImg, 50);
        bmp24_saveImage(testImg, "test_brighter.bmp");
        
        printf("Decreasing brightness...\n");
        bmp24_brightness(testImg, -75);  
        bmp24_saveImage(testImg, "test_darker.bmp");
        bmp24_free(testImg);
    }
    

    printf("\n===== Testing Convolution Filters =====\n");
    

    printf("Applying box blur filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        t_bmp24 *beforeBlur = bmp24_copy(testImg);
        bmp24_boxBlur(testImg);
        bmp24_saveImage(testImg, "test_box_blur.bmp");
        
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
    
    printf("Applying gaussian blur filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_gaussianBlur(testImg);
        bmp24_saveImage(testImg, "test_gaussian_blur.bmp");
        bmp24_free(testImg);
    }

    printf("Applying outline filter...\n");
    testImg = bmp24_copy(originalImage);
    if (testImg != NULL) {
        bmp24_outline(testImg);
        bmp24_saveImage(testImg, "test_outline.bmp");
        bmp24_free(testImg);
    }
    

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
    
    // Test histogram equalization functions
    printf("\n===== Testing Histogram Equalization =====\n");
    // Load the 8-bit image again for histogram processing
    t_bmp8 *histImg = bmp8_loadImage(inputFilename8);
    if (histImg == NULL) {
        printf("Failed to load 8-bit image for histogram equalization: %s\n", inputFilename8);
        // Create a synthetic 8-bit image if necessary
        // For this example, we'll just skip if loading fails
    } else {
        printf("Processing histogram equalization on %s\n", inputFilename8);
        
        // Copy the original image to preserve it
        t_bmp8 *origCopy = bmp8_copy(histImg);
        if (origCopy != NULL) {
            bmp8_saveImage("hist_original.bmp", origCopy);
            
            // Step 1: Compute the histogram
            printf("Computing histogram...\n");
            unsigned int *histogram = bmp8_computeHistogram(origCopy);
            if (histogram != NULL) {
                printf("Histogram computed successfully\n");
                
                // Print some histogram values
                printf("Sample histogram values:\n");
                for (int i = 0; i < 256; i += 25) {
                    printf("  Value %d: %u pixels\n", i, histogram[i]);
                }
                
                // Step 2: Compute CDF
                printf("Computing CDF...\n");
                unsigned int *cdf = bmp8_computeCDF(histogram);
                if (cdf != NULL) {
                    // Step 3: Compute equalization mapping
                    int totalPixels = origCopy->width * origCopy->height;
                    printf("Computing equalization mapping for %d pixels...\n", totalPixels);
                    unsigned int *eq_mapping = computeEqualizedMapping(cdf, totalPixels);
                    
                    if (eq_mapping != NULL) {
                        // Step 4: Apply equalization
                        printf("Applying histogram equalization...\n");
                        bmp8_equalize(origCopy, eq_mapping);
                        
                        // Save the equalized image
                        bmp8_saveImage("hist_equalized.bmp", origCopy);
                        printf("Saved equalized image to hist_equalized.bmp\n");
                        
                        free(eq_mapping);
                    } else {
                        printf("Failed to compute equalization mapping\n");
                    }
                    free(cdf);
                } else {
                    printf("Failed to compute CDF\n");
                }
                free(histogram);
            } else {
                printf("Failed to compute histogram\n");
            }
            bmp8_free(origCopy);
        }
        bmp8_free(histImg);
    }
    
    printf("\n===== Testing Histogram Equalization on flowers_colors.bmp =====\n");
    const char *flowerFilename = "flowers_color.bmp";
    printf("Attempting to load image from: %s\n", flowerFilename);
    FILE *flowerCheck = fopen(flowerFilename, "rb");
    if (flowerCheck == NULL) {
        printf("File not found: %s\n", flowerFilename);
        printf("Skipping flowers image equalization...\n");
    } else {
        fclose(flowerCheck);
        
        t_bmp24 *flowerColor = bmp24_loadImage(flowerFilename);
        if (flowerColor == NULL) {
            printf("Failed to load color image: %s\n", flowerFilename);
        } else {
            printf("Successfully loaded %s\n", flowerFilename);
            
            bmp24_saveImage(flowerColor, "flower_original.bmp");
            

            t_bmp24 *flowerGrayCopy = bmp24_copy(flowerColor);
            if (flowerGrayCopy != NULL) {
                bmp24_grayscale(flowerGrayCopy);
                bmp24_saveImage(flowerGrayCopy, "flower_gray.bmp");
                
    
                
                bmp24_free(flowerGrayCopy);
            }
            
            printf("Applying YUV color histogram equalization...\n");
            t_bmp24 *flowerYuvCopy = bmp24_copy(flowerColor);
            if (flowerYuvCopy != NULL) {
                bmp24_equalize(flowerYuvCopy);
                bmp24_saveImage(flowerYuvCopy, "flower_yuv_equalized.bmp");
                printf("Saved YUV-equalized flower image to flower_yuv_equalized.bmp\n");
                bmp24_free(flowerYuvCopy);
            }
            
            bmp24_free(flowerColor);
        }
    }

    printf("\n===== Testing Color Histogram Equalization on flowers_color.bmp =====\n");
    
    const char *flowerFilenames[] = {"flowers_color.bmp", "flowers.bmp", "flower.bmp", "flower_color.bmp"};
    t_bmp24 *flowerColor = NULL;
    
    for (int i = 0; i < 4 && flowerColor == NULL; i++) {
        printf("Attempting to load image from: %s\n", flowerFilenames[i]);
        FILE *flowerCheck = fopen(flowerFilenames[i], "rb");
        if (flowerCheck != NULL) {
            fclose(flowerCheck);
            flowerColor = bmp24_loadImage(flowerFilenames[i]);
            if (flowerColor != NULL) {
                printf("Successfully loaded %s\n", flowerFilenames[i]);
                break;
            }
        }
    }
    
    if (flowerColor == NULL) {
        printf("Could not find any flower image. Creating a test image instead.\n");
        flowerColor = create_test_image(400, 300);
        if (flowerColor == NULL) {
            printf("Failed to create test image for flower equalization. Skipping.\n");
        }
    }
    
    if (flowerColor != NULL) {
        bmp24_saveImage(flowerColor, "flower_original.bmp");
        printf("Saved original flower image to flower_original.bmp\n");
        
        printf("Applying YUV color histogram equalization to flower image...\n");
        t_bmp24 *flowerYuvCopy = bmp24_copy(flowerColor);
        if (flowerYuvCopy != NULL) {
            bmp24_equalize(flowerYuvCopy);

            bmp24_saveImage(flowerYuvCopy, "flower_yuv_equalized.bmp");
            printf("Saved YUV-equalized flower image to flower_yuv_equalized.bmp\n");
            
            int sampleX = flowerColor->width / 2;
            int sampleY = flowerColor->height / 2;
            printf("Sample comparison before/after YUV equalization:\n");
            printf("Before - Pixel at (%d,%d): R=%d, G=%d, B=%d\n", 
                   sampleX, sampleY,
                   flowerColor->data[sampleY][sampleX].red,
                   flowerColor->data[sampleY][sampleX].green,
                   flowerColor->data[sampleY][sampleX].blue);
            printf("After  - Pixel at (%d,%d): R=%d, G=%d, B=%d\n", 
                   sampleX, sampleY,
                   flowerYuvCopy->data[sampleY][sampleX].red,
                   flowerYuvCopy->data[sampleY][sampleX].green,
                   flowerYuvCopy->data[sampleY][sampleX].blue);
                   
            bmp24_free(flowerYuvCopy);
        } else {
            printf("Failed to create copy of flower image for YUV equalization\n");
        }
        
        bmp24_free(flowerColor);
        printf("Flower image processing completed!\n");
    }

    bmp24_free(originalImage);
    
    printf("\nAll image processing tests completed!\n");
    
    printf("\nAll processing complete!\n");
    
    return 0;
}
