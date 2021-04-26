//
// Created by Helder Carvalho, Leandro Faria and João Castro on 13/04/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include "./vc_lib/vc.h"
#include "./helpers/helpers.h"

int P1();

int P2();

int main() {
    int op;
    do {
        system("@cls||clear");
        printf("===================== VC TP1 ====================\n");
        printf("= Helder Carvalho | Leandro Faria | Joao Castro =\n");
        printf("\nChoose an option:\n");
        printf("1 - P1\n");
        printf("2 - P2\n");
        printf("3 - Exit\n");
        printf("Option: ");
        scanf("%d", &op);
        printf("\n\n");
        switch (op) {
            default: {
                printf("Invalid Option!\n\n");
                system("pause");
                break;
            }
            case 1: {
                if (!P1())
                    printf("P1 Error!\n\n");
                system("pause");
                break;
            }
            case 2: {
                if (!P2())
                    printf("P2 Error!\n\n");
                system("pause");
                break;
            }
            case 3:
                break;
        }
    } while (op != 3);

    return 0;
}

int P1() {
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
            printf("\nBRAIN:\n");
            printf("\tArea: %d\n", blobs->area);
            printf("\tPerimeter: %d\n", blobs->perimeter);
            printf("\tCenter of mass: x = %d | y = %d\n\n", blobs->xc, blobs->yc);
        }
    }

//    vc_write_image("../P1/Output/P11.pgm", image[1]);
//    vc_write_image("../P1/Output/P12.pgm", image[2]);
//    vc_write_image("../P1/Output/P13.pgm", image[3]);
//    vc_write_image("../P1/Output/P14.pgm", image[4]);
    vc_write_image("../P1/Output/P1.pgm", image[5]);
    for (int i = 0; i <= 6; i++)
        vc_image_free(image[i]);
    system("cmd /c start ..\\FilterGear.exe ../P1/img1.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P11.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P12.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P13.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P14.pgm");
    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P1.pgm");
    return 1;
}

int P2() {
    IVC *image[9];
    image[0] = vc_read_image("../P2/img2.ppm");
    if (image[0] == NULL) {
        printf("IMAGE 0 == NULL\n");
        return 0;
    }
    image[1] = vc_read_image("../P2/img2.ppm");
    if (image[1] == NULL) {
        printf("IMAGE 1 == NULL\n");
        return 0;
    }
    image[2] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[2] == NULL) {
        printf("IMAGE 2 == NULL\n");
        return 0;
    }
    image[3] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[3] == NULL) {
        printf("IMAGE 3 == NULL\n");
        return 0;
    }
    image[4] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[4] == NULL) {
        printf("IMAGE 4 == NULL\n");
        return 0;
    }
    image[5] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[5] == NULL) {
        printf("IMAGE 5 == NULL\n");
        return 0;
    }
    image[6] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[6] == NULL) {
        printf("IMAGE 6 == NULL\n");
        return 0;
    }
    image[7] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
    if (image[7] == NULL) {
        printf("IMAGE 7 == NULL\n");
        return 0;
    }
    image[8] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    if (image[8] == NULL) {
        printf("IMAGE 8 == NULL\n");
        return 0;
    }

    // With the original RGB image (Cells) extracts the Blue Channel to gray image[1]
    if (!vc_rgb_get_blue_gray(image[1])) {
        printf("RGB Get Blue Gray == NULL\n");
        return 0;
    }

    // Converts the gray image[1] to black and white image[2]
    if (!vc_rgb_to_gray(image[1], image[2])) {
        printf("RGB to Gray == NULL\n");
        return 0;
    }

    // Converts the gray image[2] to black and white image[3]
    if (!vc_gray_to_binary(image[2], image[3], 83)) {
        printf("Gray to Binary == NULL\n");
        return 0;
    }

    // Adds some white parts to the image[3] to close the gaps inside the Cells and make the round to a new image[4]
    if (!vc_binary_close(image[3], image[4], 13, 13)) {
        printf("Binary Close == NULL\n");
        return 0;
    }

    // Removes some white parts of the black and white image[4] (removes the incomplete cells in the border) to a new
    // image[5]
    if (!vc_binary_erode(image[4], image[5], 11)) {
        printf("Binary Erode == NULL\n");
        return 0;
    }

    // Adds some white parts to the image[5] (compensates for the loss of cell size in the previous procedure) to a new
    // image[6]
    // This image[6] is the final black and white image of the cells core
    if (!vc_binary_dilate(image[5], image[6], 11)) {
        printf("Binary Dilate == NULL\n");
        return 0;
    }

    // Extracts the cells core from the original image[0] using the dilated image[6] as a mask to a new image[8]
    // This image[5] is the final image
    if (!vc_rgb_extract_binary(image[0], image[6], image[8])) {
        printf("RGB Extract Binary == NULL\n");
        return 0;
    }


    // Counts the objects of the image[6] to a new labeled image[7]
    int n_labels;
    OVC *blobs = vc_binary_blob_labelling(image[6], image[7], &n_labels);
    if (blobs == NULL || blobs == 0) {
        printf("Binary Blob Labelling == NULL || 0\n");
        return 0;
    } else {
        // Calculates the information of the objects (labels) using the labeled image[6~5]
        if (!vc_binary_blob_info(image[7], blobs, n_labels)) {
            printf("Binary Blob Info == NULL\n");
            return 0;
        } else {
            // Prints the information about the Cells found in the labeled image[6])
            printf("CELLS\n");
            for (int i = 0; i < n_labels; i++) {
                printf("\tCell: %d", i);
                printf(" -> Area: %d\n", blobs[i].area);
                if (!vc_rgb_draw_center_of_mass(image[8], &blobs[i]))
                    printf("RGB Draw Center of Mass == NULL on Blob %d\n", i);
                if (!vc_rgb_draw_bounding_box(image[8], &blobs[i]))
                    printf("RGB Draw Bounding Box == NULL on Blob %d\n", i);
            }
        }
    }

//    vc_write_image("../P2/Output/P21.pgm", image[1]);
//    vc_write_image("../P2/Output/P22.pgm", image[2]);
//    vc_write_image("../P2/Output/P23.pgm", image[3]);
//    vc_write_image("../P2/Output/P24.pgm", image[4]);
//    vc_write_image("../P2/Output/P25.pgm", image[5]);
//    vc_write_image("../P2/Output/P26.pgm", image[6]);
    vc_write_image("../P2/Output/P2.ppm", image[8]);
    for (int i = 0; i <= 8; i++)
        vc_image_free(image[i]);
    system("cmd /c start ..\\FilterGear.exe ../P2/img2.ppm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P21.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P22.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P23.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P24.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P25.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P26.pgm");
    system("cmd /c start ..\\FilterGear.exe ../P2/Output/P2.ppm");

    return 1;
}
