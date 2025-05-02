#include "histo.h"
#include <stdlib.h>


unsigned int * bmp8_computeHistogram(t_bmp8 * img) {
    if (img == NULL || img->data == NULL) {
        return NULL;
    }

    unsigned int *histogram = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (histogram == NULL) {
        return NULL;
    }

    for (unsigned int i = 0; i < img->dataSize; i++) {
        histogram[img->data[i]]++;
    }

    return histogram;
}