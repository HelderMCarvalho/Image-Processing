#include <stdio.h>
#include <stdlib.h>
#include "./vc_lib/vc.h"
#include "./helpers/helpers.h"

int main() {
    // P1

    IVC *image[6];
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


    if (!vc_gray_to_binary(image[0], image[1], 74)) {
        printf("Gray to Binary == NULL\n");
        return 0;
    }
    if (!vc_binary_erode(image[1], image[2], 11)) {
        printf("Binary Erode == NULL\n");
        return 0;
    }
    if (!vc_binary_dilate(image[2], image[3], 11)) {
        printf("Binary Dilate == NULL\n");
        return 0;
    }
    if (!vc_binary_close(image[3], image[4], 3, 7)) {
        printf("Binary Close == NULL\n");
        return 0;
    }
    if (!vc_gray_extract_binary(image[0], image[4], image[5])) {
        printf("Gray Extract Binary == NULL\n");
        return 0;
    }


    vc_write_image("../P1/Output/P11.pgm", image[1]);
    vc_write_image("../P1/Output/P12.pgm", image[2]);
    vc_write_image("../P1/Output/P13.pgm", image[3]);
    vc_write_image("../P1/Output/P14.pgm", image[4]);
    vc_write_image("../P1/Output/P15.pgm", image[5]);
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    vc_image_free(image[2]);
    vc_image_free(image[3]);
    vc_image_free(image[4]);
    vc_image_free(image[5]);
    system("cmd /c start ..\\FilterGear.exe ../P1/img1.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P11.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P12.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P13.pgm");
//    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P14.pgm");
    system("cmd /c start ..\\FilterGear.exe ../P1/Output/P15.pgm");


    return 0;
}
