//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"
#include "performanceMeasure.c"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels) {
    IVC *image = (IVC *) malloc(sizeof(IVC));

    if (image == NULL) return NULL;
    if ((levels <= 0) || (levels > 255)) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL) {
        return vc_image_free(image);
    }

    return image;
}

// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image) {
    if (image != NULL) {
        if (image->data != NULL) {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len) {
    char *t;
    int c;

    for (;;) {
        while (isspace(c = getc(file)));
        if (c != '#') break;
        do c = getc(file);
        while ((c != '\n') && (c != EOF));
        if (c == EOF) break;
    }

    t = tok;

    if (c != EOF) {
        do {
            *t++ = c;
            c = getc(file);
        } while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if (c == '#') ungetc(c, file);
    }

    *t = 0;

    return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height) {
    int x, y;
    int countbits;
    long int pos, counttotalbytes;
    unsigned char *p = databit;

    *p = 0;
    countbits = 1;
    counttotalbytes = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //*p |= (datauchar[pos] != 0) << (8 - countbits);

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                *p |= (datauchar[pos] == 0) << (8 - countbits);

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }

    return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height) {
    int x, y;
    int countbits;
    long int pos;
    unsigned char *p = databit;

    countbits = 1;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1)) {
                p++;
                countbits = 1;
            }
        }
    }
}

IVC *vc_read_image(char *filename) {
    FILE *file = NULL;
    IVC *image = NULL;
    unsigned char *tmp;
    char tok[20];
    long int size, sizeofbinarydata;
    int width, height, channels;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if ((file = fopen(filename, "rb")) != NULL) {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0) {
            channels = 1;
            levels = 1;
        }    // Se PBM (Binary [0,1])
        else if (strcmp(tok, "P5") == 0) channels = 1;                // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0) channels = 3;                // Se PPM (RGB [0,MAX(level,255)])
        else {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if (levels == 1) // PBM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca memória para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);

            free(tmp);
        } else // PGM ou PPM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca memória para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }

        fclose(file);
    } else {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }

    return image;
}

int vc_write_image(char *filename, IVC *image) {
    FILE *file = NULL;
    unsigned char *tmp;
    long int totalbytes, sizeofbinarydata;

    if (image == NULL) return 0;

    if ((file = fopen(filename, "wb")) != NULL) {
        if (image->levels == 1) {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                free(tmp);
                return 0;
            }

            free(tmp);
        } else {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                return 0;
            }
        }

        fclose(file);

        return 1;
    }

    return 0;
}

//
// Created by Helder Carvalho on 07/03/2021.
//

/**
 * Convert a Gray image to a Negative
 * @param src_dst -> Image In and Out
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_negative(IVC *src_dst) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL)) return 0;
    if (src_dst->channels != 1) return 0;

    // Generate image
    int size = src_dst->width * src_dst->height * src_dst->channels;
    for (int pos = 0; pos < size; pos += src_dst->channels) {
        src_dst->data[pos] = (unsigned char) (255 - src_dst->data[pos]);
    }
    return 1;
}

/**
 * Convert a Gray image to an RGB image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_scale_gray_to_rgb(IVC *src, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 1) || (dst->channels != 3)) return 0;

    // Generate image
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            long int pos_src = y * src->bytesperline + x * src->channels,
                    pos_dst = y * dst->bytesperline + x * dst->channels;
            int v = (int) src->data[pos_src];
            if (v < 64) {
                dst->data[pos_dst] = (unsigned char) 0;
                dst->data[pos_dst + 1] = (unsigned char) (v * 4);
                dst->data[pos_dst + 2] = (unsigned char) 255;
            } else if (v < 128) {
                dst->data[pos_dst] = (unsigned char) 0;
                dst->data[pos_dst + 1] = (unsigned char) 255;
                dst->data[pos_dst + 2] = (unsigned char) 255 - (v - 64) * 4;
            } else if (v < 192) {
                dst->data[pos_dst] = (unsigned char) (v - 128) * 4;
                dst->data[pos_dst + 1] = (unsigned char) 255;
                dst->data[pos_dst + 2] = (unsigned char) 0;
            } else {
                dst->data[pos_dst] = (unsigned char) 255;
                dst->data[pos_dst + 1] = (unsigned char) 255 - (v - 192) * 4;
                dst->data[pos_dst + 2] = (unsigned char) 0;
            }
        }
    }
    return 1;
}

/**
 * Segment a Gray image using threshold to a Gray image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @param threshold -> Threshold to become white pixel color
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    int size = src->width * src->height * src->channels;
    for (int pos = 0; pos < size; pos += src->channels) {
        int v = (int) src->data[pos];
        if (v > threshold)
            dst->data[pos] = (unsigned char) 255;
        else
            dst->data[pos] = (unsigned char) 0;
    }
    return 1;
}

/**
 * Segment a Gray image using Mean threshold to a Gray image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_to_binary_global_mean(IVC *src, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    int threshold = 0;
    int size = src->width * src->height * src->channels;
    for (int pos = 0; pos < size; pos += src->channels) {
        threshold += (int) src->data[pos];
    }
    threshold = threshold / (src->width * src->height);
    vc_gray_to_binary(src, dst, threshold);
    return 1;
}

/**
 * Segment a Gray image using Neighborhood Midpoint threshold to a Gray image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @param kernel -> Neighborhood size (in pixels)
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_to_binary_neighborhood_midpoint(IVC *src, IVC *dst, int kernel) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    int k_movement = (kernel - 1) / 2;
    for (int x = 0; x < src->width; ++x) {
        for (int y = 0; y < src->height; ++y) {
            int pos = y * src->bytesperline + x * src->channels;
            int v_min = (int) src->data[pos], v_max = (int) src->data[pos];
            for (int kx = (x - k_movement); kx <= (x + k_movement); ++kx) {
                if ((kx < 0) || (kx >= src->width)) continue; // out of bounds, jump to next iteration (next "kx")
                for (int ky = (y - k_movement); ky <= (y + k_movement); ++ky) {
                    if ((ky < 0) || (ky >= src->height)) continue; // out of bounds, jump to next iteration (next "ky")
                    int pos_k = ky * src->bytesperline + kx * src->channels;
                    v_min = min(v_min, (int) src->data[pos_k]);
                    v_max = max(v_max, (int) src->data[pos_k]);
                }
            }
            //                                   (     Threshold     )
            if (src->data[pos] > (unsigned char) ((v_min + v_max) / 2))
                dst->data[pos] = (unsigned char) 255;
            else
                dst->data[pos] = (unsigned char) 0;
        }
    }
    return 1;
}

/**
 * Creates a Histogram of a Gray image
 * @param src -> Image to create the Histogram for
 * @param dst -> Histogram in image format
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_gray_histogram_show(IVC *src, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((dst->width != 256) || (dst->height != 256)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Get the total quantity of pixels (with 1 channel image it's the same as the src_size of the image)
    int src_size = src->width * src->height * src->channels;

    // Count the quantity of pixels for each brightness level
    int bright_count[256] = {0};
    for (int pos = 0; pos < src_size; pos += src->channels) {
        // The position in bright_count is the value of the pixel brightness
        bright_count[src->data[pos]]++;
    }

    // Calculate the Probability Density Function for each brightness level
    float pdf[256] = {0.0f}, pdf_max = 0.0f;
    for (int i = 0; i <= 255; i++) {
        pdf[i] = (float) bright_count[i] / (float) src_size;
        pdf_max = max(pdf_max, pdf[i]);
    }

    // Calculate the Normalized value of each element of the Probability Density Function, between 0 and 1
    float pdf_normalized[256] = {0.0f};
    for (int i = 0; i <= 255; i++) {
        pdf_normalized[i] = pdf[i] / pdf_max;
    }

    // Set all the pixels of the dst image to black
    int dst_size = dst->width * dst->height * dst->channels;
    for (int pos = 0; pos < dst_size; pos += dst->channels)
        dst->data[pos] = 0;

    // Create the histogram
    for (int x = 0; x < dst->width; x++)
        for (int y = dst->height - 1; (float) y > (float) (dst->height - 1) - pdf_normalized[x] * 255; y--)
            dst->data[y * 256 + x] = 255;

    return 1;
}

/**
 * Convert an RGB image to a Negative
 * @param src_dst -> Image In and Out
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_negative(IVC *src_dst) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL)) return 0;
    if (src_dst->channels != 3) return 0;

    // Generate image
    int size = src_dst->width * src_dst->height * src_dst->channels;
    for (int pos = 0; pos < size; pos += src_dst->channels) {
        src_dst->data[pos] = (unsigned char) (255 - src_dst->data[pos]);
        src_dst->data[pos + 1] = (unsigned char) (255 - src_dst->data[pos + 1]);
        src_dst->data[pos + 2] = (unsigned char) (255 - src_dst->data[pos + 2]);
    }
    return 1;
}

/**
 * Convert Red channel from RGB image to a Gray image
 * @param src_dst -> Image In and Out
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_get_red_gray(IVC *src_dst) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL)) return 0;
    if (src_dst->channels != 3) return 0;

    // Generate image
    int size = src_dst->width * src_dst->height * src_dst->channels;
    for (int pos = 0; pos < size; pos += src_dst->channels) {
        unsigned char r = (unsigned char) src_dst->data[pos];
        src_dst->data[pos + 1] = r;
        src_dst->data[pos + 2] = r;
    }
    return 1;
}

/**
 * Convert Green channel from RGB image to a Gray image
 * @param src_dst -> Image In and Out
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_get_green_gray(IVC *src_dst) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL)) return 0;
    if (src_dst->channels != 3) return 0;

    // Generate image
    int size = src_dst->width * src_dst->height * src_dst->channels;
    for (int pos = 0; pos < size; pos += src_dst->channels) {
        unsigned char g = (unsigned char) src_dst->data[pos + 1];
        src_dst->data[pos] = g;
        src_dst->data[pos + 2] = g;
    }
    return 1;
}

/**
 * Convert Blue channel from RGB image to a Gray image
 * @param src_dst -> Image In and Out
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_get_blue_gray(IVC *src_dst) {
    // Error check
    if ((src_dst->width <= 0) || (src_dst->height <= 0) || (src_dst->data == NULL)) return 0;
    if (src_dst->channels != 3) return 0;

    // Generate image
    int size = src_dst->width * src_dst->height * src_dst->channels;
    for (int pos = 0; pos < size; pos += src_dst->channels) {
        unsigned char b = (unsigned char) src_dst->data[pos + 2];
        src_dst->data[pos] = b;
        src_dst->data[pos + 1] = b;
    }
    return 1;
}

/**
 * Convert an RGB image to a Gray image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_to_gray(IVC *src, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 3) || (dst->channels != 1)) return 0;

    // Generate image
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            long int pos_src = y * src->bytesperline + x * src->channels,
                    pos_dst = y * dst->bytesperline + x * dst->channels;
            dst->data[pos_dst] = (unsigned char) (((float) src->data[pos_src] * 0.299) +
                                                  ((float) src->data[pos_src + 1] * 0.587) +
                                                  ((float) src->data[pos_src + 3] * 0.114));
        }
    }
    return 1;
}

/**
 * Convert an RGB image to an HSV image
 * @param src -> Image to convert
 * @param dst -> Converted image
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_rgb_to_hsv(IVC *src, IVC *dst) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 3) || (dst->channels != 3)) return 0;

    // Generate image
    int size = src->width * src->height * src->channels;
    for (int pos = 0; pos < size; pos += src->channels) {
        float r = (float) src->data[pos], g = (float) src->data[pos + 1], b = (float) src->data[pos + 2],
                max = fmaxf(r, fmaxf(g, b)), min = fminf(r, fminf(g, b)), sat, hue = 0;
        if (max == 0.0f)
            sat = hue = 0.0f;
        else {
            sat = (max - min) / max;
            if (sat == 0.0f)
                hue = 0.0f;
            else {
                if ((max == r) && (g >= b))
                    hue = 60.0f * (g - b) / (max - min);
                else if ((max == r) && (b > g))
                    hue = 360.0f + 60.0f * (g - b) / (max - min);
                else if (max == g)
                    hue = 120.0f + 60.0f * (b - r) / (max - min);
                else if (max == b)
                    hue = 240.0f + 60.0f * (r - g) / (max - min);
            }
        }
        dst->data[pos] = (unsigned char) (hue / 360.0f * 255.0f);
        dst->data[pos + 1] = (unsigned char) (sat * 255.0f);
        dst->data[pos + 2] = (unsigned char) max;
    }
    return 1;
}

/**
 * Segment an HSV image
 * @param src -> Image to segment
 * @param dst -> Segmented image
 * @param hmin -> Minimum Hue
 * @param hmax -> Maximum Hue
 * @param smin -> Minimum Saturation
 * @param smax -> Maximum Saturation
 * @param vmin -> Minimum Value
 * @param vmax -> Maximum Value
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 3) || (dst->channels != 1)) return 0;

    // Generate image
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            long int pos_src = y * src->bytesperline + x * src->channels,
                    pos_dst = y * dst->bytesperline + x * dst->channels;
            int h = (int) ((float) (src->data[pos_src]) / 255.0f * 360.0f),
                    s = (int) ((float) (src->data[pos_src + 1]) / 255.0f * 100.0f),
                    v = (int) ((float) (src->data[pos_src + 2]) / 255.0f * 100.0f);
            if ((h >= hmin && h <= hmax) && (s >= smin && s <= smax) && (v >= vmin && v <= vmax))
                dst->data[pos_dst] = (unsigned char) 255;
            else
                dst->data[pos_dst] = (unsigned char) 0;
        }
    }
    return 1;
}

/**
 * Dilate an Binary image (background has to be black and objects white)
 * @param src -> Image to dilate
 * @param dst -> Dilated image
 * @param kernel -> Neighborhood size (in pixels)
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_binary_dilate(IVC *src, IVC *dst, int kernel) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    int k_movement = (kernel - 1) / 2;
    for (int x = 0; x < src->width; ++x) {
        for (int y = 0; y < src->height; ++y) {
            int pos = y * src->bytesperline + x * src->channels, level = 0;
            for (int kx = (x - k_movement); kx <= (x + k_movement); ++kx) {
                if ((kx >= 0) && (kx < src->width)) {
                    for (int ky = (y - k_movement); ky <= (y + k_movement); ++ky) {
                        if ((ky >= 0) && (ky < src->height)) {
                            int pos_k = ky * src->bytesperline + kx * src->channels;
                            if (src->data[pos_k] == (unsigned char) 255) {
                                level = 1;
                                break;
                            }
                        }
                        if (level)
                            break;
                    }
                }
            }
            if (level)
                dst->data[pos] = (unsigned char) 255;
            else
                dst->data[pos] = (unsigned char) 0;
        }
    }
    return 1;
}

/**
 * Erode an Binary image (background has to be Black and objects White)
 * @param src -> Image to erode
 * @param dst -> Eroded image
 * @param kernel -> Neighborhood size (in pixels)
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_binary_erode(IVC *src, IVC *dst, int kernel) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    int k_movement = (kernel - 1) / 2;
    for (int x = 0; x < src->width; ++x) {
        for (int y = 0; y < src->height; ++y) {
            int pos = y * src->bytesperline + x * src->channels, level = 0;
            for (int kx = (x - k_movement); kx <= (x + k_movement); ++kx) {
                if ((kx >= 0) && (kx < src->width)) {
                    for (int ky = (y - k_movement); ky <= (y + k_movement); ++ky) {
                        if ((ky >= 0) && (ky < src->height)) {
                            int pos_k = ky * src->bytesperline + kx * src->channels;
                            if (src->data[pos_k] == (unsigned char) 0) {
                                level = 1;
                                break;
                            }
                        }
                        if (level)
                            break;
                    }
                }
            }
            if (level)
                dst->data[pos] = (unsigned char) 0;
            else
                dst->data[pos] = (unsigned char) 255;
        }
    }
    return 1;
}

/**
 * Opens (morphological operator) a Binary image
 * @param src -> Image to open
 * @param dst -> Opened image
 * @param kernel_erode -> Neighborhood size for Erode (in pixels)
 * @param kernel_dilate -> Neighborhood size for Dilate (in pixels)
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_binary_open(IVC *src, IVC *dst, int kernel_erode, int kernel_dilate) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    IVC *aux = vc_image_new(src->width, src->height, src->channels, src->levels);
    if (!vc_binary_erode(src, aux, kernel_erode)) {
        vc_image_free(aux);
        return 0;
    }
    if (!vc_binary_dilate(aux, dst, kernel_dilate)) {
        vc_image_free(aux);
        return 0;
    }
    vc_image_free(aux);

    return 1;
}

/**
 * Closes (morphological operator) a Binary image
 * @param src -> Image to close
 * @param dst -> Closed image
 * @param kernel_erode -> Neighborhood size for Erode (in pixels)
 * @param kernel_dilate -> Neighborhood size for Dilate (in pixels)
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_binary_close(IVC *src, IVC *dst, int kernel_erode, int kernel_dilate) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Generate image
    IVC *aux = vc_image_new(src->width, src->height, src->channels, src->levels);
    if (!vc_binary_dilate(src, aux, kernel_dilate)) {
        vc_image_free(aux);
        return 0;
    }
    if (!vc_binary_erode(aux, dst, kernel_erode)) {
        vc_image_free(aux);
        return 0;
    }
    vc_image_free(aux);

    return 1;
}

/**
 * Counts the quantity of objects (number of labels)
 * <p>This algorithm is limited to 255 labels</p>
 * @param src -> Image to label
 * @param dst -> Labeled image (usually all black)
 * @param n_labels -> Output of the quantity of labels
 * @return blobs object with information about the labels
 */
OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *n_labels) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

    // Copy data from src to dst.
    memcpy(dst->data, src->data, src->bytesperline * src->height);

    // All background pixels need to have value 0.
    // All foreground pixels need to have value 255.
    // Labels will be attributed with values [1,254].
    for (long int i = 0, size = dst->width * dst->height * dst->channels; i < size; i += dst->channels) {
        if (dst->data[i] != 0)
            dst->data[i] = 255;
    }

    int label = 1, label_table[256] = {0};
    for (int y = 0; y < dst->height; y++) {
        for (int x = 0; x < dst->width; x++) {
            // These ifs will see if the positions to check are valid and if they are they will put the coordinate in
            // the "pos" variable otherwise they will put "-1" in that same variable so that way it is ignored.
            long int posX = y * dst->bytesperline + x * dst->channels,
                    posA = ((y - 1 >= 0) && (x - 1 >= 0)) ? (y - 1) * dst->bytesperline + (x - 1) * dst->channels : -1,
                    posB = (y - 1 >= 0) ? (y - 1) * dst->bytesperline + x * dst->channels : -1,
                    posC = ((y - 1 >= 0) && (x + 1 < dst->width)) ? (y - 1) * dst->bytesperline +
                                                                    (x + 1) * dst->channels : -1,
                    posD = (x - 1 >= 0) ? y * dst->bytesperline + (x - 1) * dst->channels : -1;
            // If pixel isn't black.
            if (dst->data[posX] != (unsigned char) 0) {
                // Check if neighborhood pixels exist or if they are black.
                if ((posA == -1 || (dst->data[posA] == (unsigned char) 0)) &&
                    (posB == -1 || (dst->data[posB] == (unsigned char) 0)) &&
                    (posC == -1 || (dst->data[posC] == (unsigned char) 0)) &&
                    (posD == -1 || (dst->data[posD] == (unsigned char) 0))) {
                    // All pixels around dont exist or are black (or some dont exit and some are black), then its a new
                    // label.
                    dst->data[posX] = (unsigned char) label;
                    label_table[label] = label;
                    label++;

                } else {
                    // Its not a new label, the pixel belongs to an existing label.
                    // "Num" will always have the lower label.
                    int num = 255;
                    // If A is labeled.
                    if (dst->data[posA] != 0)
                        num = label_table[dst->data[posA]];
                    // If B is labeled and his label is lower than "num".
                    if (num > label_table[dst->data[posB]] && dst->data[posB] != 0)
                        num = label_table[dst->data[posB]];
                    // If C is labeled and his label is lower than "num".
                    if (num > label_table[dst->data[posC]] && dst->data[posC] != 0)
                        num = label_table[dst->data[posC]];
                    // If D is labeled and his label is lower than "num".
                    if (num > label_table[dst->data[posD]] && dst->data[posD] != 0)
                        num = label_table[dst->data[posD]];

                    // Sets the pixel label.
                    dst->data[posX] = num;
                    label_table[num] = num;

                    // Updates the label table.
                    if (dst->data[posA] != 0)
                        if (label_table[dst->data[posA]] != num)
                            for (int tmpLabel = label_table[dst->data[posA]], a = 1; a < label; a++)
                                if (label_table[a] == tmpLabel)
                                    label_table[a] = num;

                    if (dst->data[posB] != 0)
                        if (label_table[dst->data[posB]] != num)
                            for (int tmpLabel = label_table[dst->data[posB]], a = 1; a < label; a++)
                                if (label_table[a] == tmpLabel)
                                    label_table[a] = num;

                    if (dst->data[posC] != 0)
                        if (label_table[dst->data[posC]] != num)
                            for (int tmpLabel = label_table[dst->data[posC]], a = 1; a < label; a++)
                                if (label_table[a] == tmpLabel)
                                    label_table[a] = num;

                    if (dst->data[posD] != 0)
                        if (label_table[dst->data[posD]] != num)
                            for (int tmpLabel = label_table[dst->data[posC]], a = 1; a < label; a++)
                                if (label_table[a] == tmpLabel)
                                    label_table[a] = num;

                }
            }
        }
    }

    // Labels the image again.
    for (long int i = 0, size = dst->width * dst->height * dst->channels; i < size; i += dst->channels) {
        if (dst->data[i] != 0)
            dst->data[i] = label_table[dst->data[i]];
    }

    // Count of labels
    // Step 1: Delete duplicated labels from the table.
    for (int i = 1; i < label - 1; i++) {
        for (int j = i + 1; j < label; j++) {
            if (label_table[i] == label_table[j])
                label_table[j] = 0;
        }
    }
    // Step 2: Count labels and organize the table, so that way there is no "zeros" between labels.
    *n_labels = 0;
    for (int j = 1; j < label; j++) {
        if (label_table[j] != 0) {
            label_table[*n_labels] = label_table[j];
            (*n_labels)++;
        }
    }

    // If there are no labels.
    if (*n_labels == 0)
        return NULL;

    // Create list of blobs (objects) and fill the label field.
    OVC *blobs = (OVC *) calloc((*n_labels), sizeof(OVC));
    if (blobs != NULL)
        for (int a = 0; a < (*n_labels); a++)
            blobs[a].label = label_table[a];
    else
        return NULL;

    return blobs;
}

/**
 * Calculates all the information about every blob (label)
 * @param src -> Labeled image
 * @param blobs -> Output of all the information about every blob (label)
 * @param n_blobs -> Quantity of blobs (labels) to analyse
 * @return -> 0 (Error) ou 1 (Successes)
 */
int vc_binary_blob_info(IVC *src, OVC *blobs, int n_blobs) {
    // Error check
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->channels != 1)) return 0;

    // Calculates every blob
    for (int i = 0; i < n_blobs; i++) {
        int x_min = src->width - 1, y_min = src->height - 1, x_max = 0, y_max = 0;
        long int sum_x = 0, sum_y = 0;
        blobs[i].area = 0;

        // Colocar a verificar pixeis das bordas (neste momento ignora a 1ª e ultima linha e coluna)
        for (int y = 1; y < src->height - 1; y++) {
            for (int x = 1; x < src->width - 1; x++) {
                long int pos = y * src->bytesperline + x * src->channels;
                if (src->data[pos] == blobs[i].label) {
                    // Area
                    blobs[i].area++;

                    // Center of Gravity
                    sum_x += x;
                    sum_y += y;

                    // Bounding Box
                    x_min = min(x_min, x);
                    y_min = min(y_min, y);
                    x_max = max(x_max, x);
                    y_max = max(y_max, y);

                    // Perimeter
                    // If at least one of the four neighbours doesn't belong to the same label, then its a contour pixel.
                    if ((src->data[pos - 1] != blobs[i].label) || (src->data[pos + 1] != blobs[i].label) ||
                        (src->data[pos - src->bytesperline] != blobs[i].label) ||
                        (src->data[pos + src->bytesperline] != blobs[i].label)) {
                        blobs[i].perimeter++;
                    }
                }
            }
        }

        // Center of Gravity
        blobs[i].xc = (int) round((double) sum_x / max(blobs[i].area, 1));
        blobs[i].yc = (int) round((double) sum_y / max(blobs[i].area, 1));

        // Bounding Box
        blobs[i].x = x_min;
        blobs[i].y = y_min;
        blobs[i].width = (x_max - x_min) + 1;
        blobs[i].height = (y_max - y_min) + 1;
    }

    return 1;
}
