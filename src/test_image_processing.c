#include "image_processing.h"
#include "bmp_handler.h"
#include <stdio.h>
#include <stdlib.h> // Required for exit()
#include <string.h> // Required for strcmp()

// Function to test image processing functions
void test_image_processing(const char *input_filename) {
    // Array of filter types and corresponding output filenames
    const char *filter_names[] = {
        "negative", "grayscale", "box_blur", "gaussian_blur", "emboss", "brighter", "darker"
    };
    const char *output_filenames[] = {
        "test_negative.bmp", "test_grayscale.bmp", "test_box_blur.bmp",
        "test_gaussian_blur.bmp", "test_emboss.bmp", "test_brighter.bmp", "test_darker.bmp"
    };
    int num_filters = sizeof(filter_names) / sizeof(filter_names[0]);

    // Load the base input image
    Image *base_image = read_bmp(input_filename);
    if (!base_image) {
        fprintf(stderr, "Error: Failed to load base image '%s'\n", input_filename);
        return; // Or exit(1);
    }
     printf("Base image '%s' loaded successfully.\n", input_filename);

    for (int i = 0; i < num_filters; ++i) {
        printf("Applying filter: %s\n", filter_names[i]);

        // Create a copy of the base image for processing
        Image *img_copy = copy_image(base_image);
        if (!img_copy) {
             fprintf(stderr, "Error: Failed to copy base image for filter '%s'\n", filter_names[i]);
             continue; // Skip this filter if copy fails
        }

        // Apply the filter based on the name
        if (strcmp(filter_names[i], "negative") == 0) {
            apply_negative(img_copy);
        } else if (strcmp(filter_names[i], "grayscale") == 0) {
            apply_grayscale(img_copy);
        } else if (strcmp(filter_names[i], "box_blur") == 0) {
            apply_box_blur(img_copy);
        } else if (strcmp(filter_names[i], "gaussian_blur") == 0) {
             apply_gaussian_blur(img_copy);
        } else if (strcmp(filter_names[i], "emboss") == 0) {
             apply_emboss(img_copy);
        } else if (strcmp(filter_names[i], "brighter") == 0) {
             adjust_brightness(img_copy, 50); // Example brightness increase
        } else if (strcmp(filter_names[i], "darker") == 0) {
             adjust_brightness(img_copy, -50); // Example brightness decrease
        }
        // Add other filters here if needed

        // Save the processed image
        if (write_bmp(output_filenames[i], img_copy)) {
            printf("Saved processed image to %s\n", output_filenames[i]);
        } else {
            fprintf(stderr, "Error: Failed to save image %s\n", output_filenames[i]);
        }

        // Free the copied image
        free_image(img_copy);
    }

    // Free the base image
    free_image(base_image);
    printf("Finished testing all filters.\n");
}

// Main function to run the tests
int main() {
    // Ensure the input image exists in a known relative path or provide absolute path
    const char *input_image_path = "lena_color.bmp"; // Adjust path if needed
    test_image_processing(input_image_path);
    return 0;
}
