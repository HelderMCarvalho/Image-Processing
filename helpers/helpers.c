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
 * Extract the original pixels from the source Gray image according to a mask image
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

/**
 * Extract the original pixels from the source RGB image according to a mask image
 * @param src -> Original Image
 * @param mask -> Mask image (pixels to extract in white)
 * @param dst -> Extracted Image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_extract_binary(IVC *src, IVC *mask, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (mask->data == NULL) || (dst->data == NULL) ||
        (src->channels != 3))
        return 0;
    if ((src->width != mask->width || src->width != dst->width) ||
        (src->height != mask->height || src->height != dst->height) ||
        (src->channels != dst->channels))
        return 0;
    if (mask->channels != 1) return 0;

    // Loop the images
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            long int pos_src_dst = y * src->bytesperline + x * src->channels,
                    pos_mask = y * mask->bytesperline + x * mask->channels;
            // If the pixel of the mask is white, then that pixel will be copied from source to destination, else the
            // destination pixel will be black.
            if (mask->data[pos_mask] == 255) {
                dst->data[pos_src_dst] = src->data[pos_src_dst];
                dst->data[pos_src_dst + 1] = src->data[pos_src_dst + 1];
                dst->data[pos_src_dst + 2] = src->data[pos_src_dst + 2];
            } else
                dst->data[pos_src_dst] = 0;
        }
    }

    return 1;
}

/**
 * Draw a white Plus "+" symbol at the center of mass in an RGB image
 * @param src_dst -> Image In and Out
 * @param blob -> Blob (label) to draw the center of mass
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_draw_center_of_mass(IVC *src_dst, OVC *blob) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL) || (src_dst->channels != 3))
        return 0;

    // Calculate the positions of a plus "+" symbol at the center of mass
    // Dont draw on image edge pixels
    long int pos_c = blob->yc * src_dst->bytesperline + blob->xc * src_dst->channels,
            pos_u = (blob->yc - 1) * src_dst->bytesperline + blob->xc * src_dst->channels,
            pos_uu = (blob->yc - 2) * src_dst->bytesperline + blob->xc * src_dst->channels,
            pos_d = (blob->yc + 1) * src_dst->bytesperline + blob->xc * src_dst->channels,
            pos_dd = (blob->yc + 2) * src_dst->bytesperline + blob->xc * src_dst->channels,
            pos_l = blob->yc * src_dst->bytesperline + (blob->xc - 1) * src_dst->channels,
            pos_ll = blob->yc * src_dst->bytesperline + (blob->xc - 2) * src_dst->channels,
            pos_r = blob->yc * src_dst->bytesperline + (blob->xc + 1) * src_dst->channels,
            pos_rr = blob->yc * src_dst->bytesperline + (blob->xc + 2) * src_dst->channels;

    // Draw a white Plus "+" symbol at the center of mass (the three channels are white)
    src_dst->data[pos_c] = src_dst->data[pos_c + 1] = src_dst->data[pos_c + 2]
            = src_dst->data[pos_u] = src_dst->data[pos_u + 1] = src_dst->data[pos_u + 2]
            = src_dst->data[pos_uu] = src_dst->data[pos_uu + 1] = src_dst->data[pos_uu + 2]
            = src_dst->data[pos_d] = src_dst->data[pos_d + 1] = src_dst->data[pos_d + 2]
            = src_dst->data[pos_dd] = src_dst->data[pos_dd + 1] = src_dst->data[pos_dd + 2]
            = src_dst->data[pos_l] = src_dst->data[pos_l + 1] = src_dst->data[pos_l + 2]
            = src_dst->data[pos_ll] = src_dst->data[pos_ll + 1] = src_dst->data[pos_ll + 2]
            = src_dst->data[pos_r] = src_dst->data[pos_r + 1] = src_dst->data[pos_r + 2]
            = src_dst->data[pos_rr] = src_dst->data[pos_rr + 1] = src_dst->data[pos_rr + 2] = 255;

    return 1;
}

/**
 * Draw a white Box in an RGB image with information of the Bounding Box of a Blob (Label)
 * @param src_dst -> Image In and Out
 * @param blob -> Blob (label) to draw the center of mass
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_draw_bounding_box(IVC *src_dst, OVC *blob) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL) || (src_dst->channels != 3))
        return 0;

    // Calculate the bottom and right borders of the bounding box so that they get drawn inside the image (only for the
    // bottom and right borders (top and left will be always drawn inside the image so not needed calculation for these
    // two))

    // If the bottom border of the bounding box will be drawn outside the image, calculate the lowest possible Y for
    // the bottom bounding box border so that it fits inside the image.
    int bottom_bounding_y = blob->y + blob->height, tmp_y = 0;
    while (bottom_bounding_y >= src_dst->height) {
        bottom_bounding_y = blob->y + (blob->height - (++tmp_y));
    }

    // If the right border of the bounding box will be drawn outside the image, calculate the rightest possible X for
    // the right bounding box border so that it fits inside the image.
    int right_bounding_x = blob->x + blob->width, tmp_x = 0;
    while (right_bounding_x >= src_dst->width) {
        right_bounding_x = blob->x + (blob->width - (++tmp_x));
    }

    for (int x = blob->x; x < blob->x + blob->width; x++) {
        // Draw the upper line of the box (paint all X's from the upper Y coord)
        long int pos = blob->y * src_dst->bytesperline + x * src_dst->channels;
        src_dst->data[pos] = src_dst->data[pos + 1] = src_dst->data[pos + 2] = 255;

        // Draw the bottom line of the box (paint all X's from the bottom Y coord)
        pos = bottom_bounding_y * src_dst->bytesperline + x * src_dst->channels;
        src_dst->data[pos] = src_dst->data[pos + 1] = src_dst->data[pos + 2] = 255;
    }

    for (int y = blob->y; y < blob->y + blob->height; y++) {
        // Draw the left line of the box (paint all Y' from the left X coord)
        long int pos = y * src_dst->bytesperline + blob->x * src_dst->channels;
        src_dst->data[pos] = src_dst->data[pos + 1] = src_dst->data[pos + 2] = 255;

        // Draw the right line of the box (paint all Y' from the right X coord)
        pos = y * src_dst->bytesperline + right_bounding_x * src_dst->channels;
        src_dst->data[pos] = src_dst->data[pos + 1] = src_dst->data[pos + 2] = 255;
    }

    return 1;
}
