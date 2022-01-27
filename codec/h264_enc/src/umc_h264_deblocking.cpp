/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2003-2008 Intel Corporation. All Rights Reserved.
//
//
*/

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include "umc_h264_video_encoder.h"
#include "umc_h264_deblocking.h"
#include "umc_h264_deblocking_tools.h"
#include "vm_types.h"
#include "vm_debug.h"
#include "umc_h264_to_ipp.h"
#include "umc_h264_bme.h"

namespace UMC_H264_ENCODER
{

// alpha table
Ipp8u ENCODER_ALPHA_TABLE_8u[52] =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    4,  4,  5,  6,  7,  8,  9,  10,
    12, 13, 15, 17, 20, 22, 25, 28,
    32, 36, 40, 45, 50, 56, 63, 71,
    80, 90, 101,113,127,144,162,182,
    203,226,255,255
};

// beta table
Ipp8u ENCODER_BETA_TABLE_8u[52] =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    2,  2,  2,  3,  3,  3,  3,  4,
    4,  4,  6,  6,  7,  7,  8,  8,
    9,  9,  10, 10, 11, 11, 12, 12,
    13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18
};

// clipping table
Ipp8u ENCODER_CLIP_TAB_8u[52][5] =
{
    { 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 1, 1, 1, 1},
    { 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 2, 3, 3},
    { 0, 1, 2, 3, 3},{ 0, 2, 2, 3, 3},{ 0, 2, 2, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 3, 3, 5, 5},{ 0, 3, 4, 6, 6},{ 0, 3, 4, 6, 6},
    { 0, 4, 5, 7, 7},{ 0, 4, 5, 8, 8},{ 0, 4, 6, 9, 9},{ 0, 5, 7,10,10},{ 0, 6, 8,11,11},{ 0, 6, 8,13,13},{ 0, 7,10,14,14},{ 0, 8,11,16,16},
    { 0, 9,12,18,18},{ 0,10,13,20,20},{ 0,11,15,23,23},{ 0,13,17,25,25}
};

// chroma scaling QP table
Ipp8u ENCODER_QP_SCALE_CR[52] =
{
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 29, 30,
    31, 32, 32, 33, 34, 34, 35, 35,
    36, 36, 37, 37, 37, 38, 38, 38,
    39, 39, 39, 39
};

// masks for external blocks pair "coded bits"
Ipp32u ENCODER_EXTERNAL_BLOCK_MASK[NUMBER_OF_DIRECTION][2][4] =
{
    // block mask for vertical deblocking
    {
        {2 << 0, 2 << 2, 2 << 8, 2 << 10},
        {2 << 5, 2 << 7, 2 << 13,2 << 15}
    },

    // block mask for horizontal deblocking
    {
        {2 << 0, 2 << 1, 2 << 4, 2 << 5},
        {2 << 10,2 << 11,2 << 14,2 << 15}
    }
};

#define DECL(prev, cur) (2 << (prev) | 2 << (cur))
// masks for internal blocks pair "coded bits"
Ipp32u ENCODER_INTERNAL_BLOCKS_MASK[NUMBER_OF_DIRECTION][12] =
{
    // blocks pair-mask for vertical deblocking
    {
        DECL(0, 1),  DECL(2, 3),  DECL(8, 9),  DECL(10, 11),
        DECL(1, 4),  DECL(3, 6),  DECL(9, 12), DECL(11, 14),
        DECL(4, 5),  DECL(6, 7),  DECL(12, 13),DECL(14, 15)
    },

    // blocks pair-mask for horizontal deblocking
    {
        DECL(0, 2),  DECL(1, 3),  DECL(4, 6),  DECL(5, 7),
        DECL(2, 8),  DECL(3, 9),  DECL(6, 12), DECL(7, 13),
        DECL(8, 10), DECL(9, 11), DECL(12, 14),DECL(13, 15)
    }
};
#undef DECL

#define PIXBITS 8
#include "umc_h264_deblocking_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_deblocking_tmpl.cpp.h"
#undef PIXBITS

#endif //BITDEPTH_9_12

} // end namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER

