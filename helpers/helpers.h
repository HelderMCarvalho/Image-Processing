//
// Created by Helder Carvalho, Leandro Faria and João Castro on 13/04/2021.
//

#ifndef TP1_HELPERS_H
#define TP1_HELPERS_H

#include "../vc_lib/vc.h"

int vc_gray_extract_binary(IVC *src, IVC *mask, IVC *dst);

int vc_rgb_extract_binary(IVC *src, IVC *mask, IVC *dst);

int vc_rgb_draw_center_of_mass(IVC *src_dst, OVC *blob);

int vc_rgb_draw_bounding_box(IVC *src_dst, OVC *blob);

#endif //TP1_HELPERS_H
