//
// Created by Helder Carvalho, Leandro Faria and João Castro on 13/04/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include "./vc_lib/vc.h"
#include "./helpers/helpers.h"

int main() {
    // P1
    IVC *image[7];
    image[0] = vc_read_image("../P1/img1.pgm");
    if (image[0] == NULL) {
        printf("IMAGE 0 == NULL\n");
        return 0;
    }
    image[1] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[1] == NULL) {
        printf("IMAGE 1 == NULL\n");
        return 0;
    }
    image[2] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[2] == NULL) {
        printf("IMAGE 2 == NULL\n");
        return 0;
    }
    image[3] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[3] == NULL) {
        printf("IMAGE 3 == NULL\n");
        return 0;
    }
    image[4] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[4] == NULL) {
        printf("IMAGE 4 == NULL\n");
        return 0;
    }
    image[5] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[5] == NULL) {
        printf("IMAGE 5 == NULL\n");
        return 0;
    }
    image[6] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[6] == NULL) {
        printf("IMAGE 6 == NULL\n");
        return 0;
    }

    // Converts the original gray image[0] (Head Magnetic resonance) to black and white image[1]
    if (!vc_gray_to_binary(image[0], image[1], 74)) {
        printf("Gray to Binary == NULL\n");
        return 0;
    }

    // Removes some white parts of the black and white image[1] (removes the skull) to a new image[2]
    if (!vc_binary_erode(image[1], image[2], 11)) {
        printf("Binary Erode == NULL\n");
        return 0;
    }

    // Adds some white parts to the image[2] (compensates for the loss of brain size in the previous procedure) to a new
    // image[3]
    if (!vc_binary_dilate(image[2], image[3], 11)) {
        printf("Binary Dilate == NULL\n");
        return 0;
    }

    // Adds some white parts to the image[3] to close the gaps inside the brain to a new image[4]
    // This image[4] is the final black and white image without the skull
    if (!vc_binary_close(image[3], image[4], 3, 7)) {
        printf("Binary Close == NULL\n");
        return 0;
    }

    // Extracts the brain from the original image[0] using the closed image[4] as a mask to a new image[5]
    // This image[5] is the final image
    if (!vc_gray_extract_binary(image[0], image[4], image[5])) {
        printf("Gray Extract Binary == NULL\n");
        return 0;
    }

    // Counts the objects of the image[4] to a new labeled image[6]
    int n_labels;
    OVC *blobs = vc_binary_blob_labelling(image[4], image[6], &n_labels);
    if (blobs == NULL || blobs == 0) {
        printf("Binary Blob Labelling == NULL || 0\n");
        return 0;
    } else {
        // Calculates the information of the objects (labels) using the labeled image[6]
        if (!vc_binary_blob_info(image[6], blobs, n_labels)) {
            printf("Binary Blob Info == NULL\n");
            return 0;
        } else {
            // Prints the information about the brain (its the only existing object (label) in the labeled image[6])
            printf("BRAIN:\n");
            printf("\tArea: %d\n", blobs->area);
            printf("\tPerimeter: %d\n", blobs->perimeter);
            printf("\tCenter of mass: x = %d | y = %d\n", blobs->xc, blobs->yc);
        }
    }

//    vc_write_image("../P1/Output/P11.pgm", image[1]);
//    vc_write_image("../P1/Output/P12.pgm", image[2]);
//    vc_write_image("../P1/Output/P13.pgm", image[3]);
//    vc_write_image("../P1/Output/P14.pgm", image[4]);
    vc_write_image("../P1/Output/P1.pgm", image[5]);
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    vc_image_free(image[2]);
    vc_image_free(image[3]);
    vc_image_free(image[4]);
    vc_image_free(image[5]);
    vc_image_free(image[6]);
    system("cmd /c start ..\\FilterGear.exe ../P1/img1.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P11.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P12.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P13.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P14.pgm");
    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P1.pgm");


    return 0;
}
