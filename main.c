#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"

int main() {
    const char * inputFilename = "lena_gray_8bit.bmp";
    t_bmp8 * image = bmp8_loadImage(inputFilename);
    if (image == NULL) return 1;

    bmp8_printInfo(image);

    bmp8_negative(image);
    bmp8_saveImage("image_negative.bmp", image);

    bmp8_changeBrightness(image, 50);
    bmp8_saveImage("image_brightness.bmp", image);

    bmp8_threshold(image, 128);
    bmp8_saveImage("image_threshold.bmp", image);

    int kernelSize = 3;
    float **boxBlur = malloc(kernelSize * sizeof(float *));
    for (int i = 0; i < kernelSize; i++) {
        boxBlur[i] = malloc(kernelSize * sizeof(float));
        for (int j = 0; j < kernelSize; j++) {
            boxBlur[i][j] = 1.0f / 9.0f;
        }
    }

    bmp8_applyFilter(image, boxBlur, kernelSize);
    bmp8_saveImage("image_filtered.bmp", image);

    for (int i = 0; i < kernelSize; i++) {
        free(boxBlur[i]);
    }
    free(boxBlur);

    bmp8_free(image);
    printf("Traitement terminé !\n");

    return 0;
}
