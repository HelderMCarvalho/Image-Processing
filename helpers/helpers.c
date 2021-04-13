//
// Created by Helder Carvalho, Leandro Faria and João Castro on 13/04/2021.
//

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "helpers.h"

/**
 * Extract the original pixels from the source image according to a mask image
 * @param src -> Original Image
 * @param mask -> Mask image (pixels to extract in white)
 * @param dst -> Extracted Image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_extract_binary(IVC *src, IVC *mask, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (mask->data == NULL) || (dst->data == NULL) ||
        (src->channels != 1))
        return 0;
    if ((src->width != mask->width || src->width != dst->width) ||
        (src->height != mask->height || src->height != dst->height) ||
        (src->channels != mask->channels || src->channels != dst->channels))
        return 0;

    // Calculate the size of the images (they are all the same size)
    int size = src->width * src->height * src->channels;

    // Loop the mask image
    for (int pos = 0; pos < size; pos += src->channels) {
        // If the pixel of the mask is white, then that pixel will be copied from source to destination, else the
        // destination pixel will be black.
        if (mask->data[pos] == 255)
            dst->data[pos] = src->data[pos];
        else
            dst->data[pos] = 0;
    }

    return 1;
}
