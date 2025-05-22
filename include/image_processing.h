#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

// Structure to represent an image
typedef struct {
    unsigned char *data;  // Pointer to the image data
    int width;           // Width of the image
    int height;          // Height of the image
} Image;

// Function to load an image from a file
Image *load_image(const char *filename);

// Function to save an image to a file
void save_image(const char *filename, const Image *img);

// Function to apply Gaussian blur to an image
void apply_gaussian_blur(Image *img);

// Function to copy an image
Image *copy_image(const Image *src);

#endif // IMAGE_PROCESSING_H