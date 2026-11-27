#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>

typedef struct {
    int width;
    int height;
    int *pixels;
} ImageData;

bool image_load_grayscale(const char *file_path, ImageData *image);

void image_free(ImageData *image);

bool image_calculate_histogram(
    const ImageData *image,
    int histogram[256]
);

#endif