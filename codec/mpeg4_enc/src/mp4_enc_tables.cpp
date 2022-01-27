/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2003-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderMPEG4 (tables)
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)

#include "mp4_enc.hpp"

#pragma warning(disable : 279)      // controlling expression is constant

namespace MPEG4_ENC
{

const Ipp8u mp4_DefaultIntraQuantMatrix[64] = {
     8, 17, 18, 19, 21, 23, 25, 27, 17, 18, 19, 21, 23, 25, 27, 28,
    20, 21, 22, 23, 24, 26, 28, 30, 21, 22, 23, 24, 26, 28, 30, 32,
    22, 23, 24, 26, 28, 30, 32, 35, 23, 24, 26, 28, 30, 32, 35, 38,
    25, 26, 28, 30, 32, 35, 38, 41, 27, 28, 30, 32, 35, 38, 41, 45
};

const Ipp8u mp4_DefaultNonIntraQuantMatrix[64] = {
    16, 17, 18, 19, 20, 21, 22, 23, 17, 18, 19, 20, 21, 22, 23, 24,
    18, 19, 20, 21, 22, 23, 24, 25, 19, 20, 21, 22, 23, 24, 26, 27,
    20, 21, 22, 23, 25, 26, 27, 28, 21, 22, 23, 24, 26, 27, 28, 30,
    22, 23, 24, 26, 27, 28, 30, 31, 23, 24, 25, 27, 28, 30, 31, 33
};

const Ipp8u mp4_ZigZagScan[64] = {
     0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

const Ipp8u mp4_AltVertScan[64] = {
     0,  8, 16, 24,  1,  9,  2, 10, 17, 25, 32, 40, 48, 56, 57, 49,
    41, 33, 26, 18,  3, 11,  4, 12, 19, 27, 34, 42, 50, 58, 35, 43,
    51, 59, 20, 28,  5, 13,  6, 14, 21, 29, 36, 44, 52, 60, 37, 45,
    53, 61, 22, 30,  7, 15, 23, 31, 38, 46, 54, 62, 39, 47, 55, 63
};

const Ipp8u mp4_HorScan[64] = {
     0,  1,  2,  3,  8,  9, 16, 17, 10, 11,  4,  5,  6,  7, 15, 14,
    13, 12, 19, 18, 24, 25, 32, 33, 26, 27, 20, 21, 22, 23, 28, 29,
    30, 31, 34, 35, 40, 41, 48, 49, 42, 43, 36, 37, 38, 39, 44, 45,
    46, 47, 50, 51, 56, 57, 58, 59, 52, 53, 54, 55, 60, 61, 62, 63
};


const int mp4_DC_VLC_Threshold[8] = {512, 13, 15, 17, 19, 21, 23, 0};

// table B-7
const mp4_VLC mp4_VLC_MCBPC_TB7[20] = {
    {1, 1}, {3, 4}, {2, 4}, {5, 6}, {3, 3}, {7, 7}, {6, 7}, {5, 9},
    {2, 3}, {5, 7}, {4, 7}, {5, 8}, {3, 5}, {4, 8}, {3, 8}, {3, 7},
    {4, 6}, {4, 9}, {3, 9}, {2, 9}
};

// table B-8
const mp4_VLC mp4_VLC_CBPY_TB8[16] = {
    {0x3, 4}, {5, 5}, {4, 5}, {9, 4}, {3, 5}, {7, 4}, {2, 6}, {11, 4},
    {0x2, 5}, {3, 6}, {5, 4}, {10, 4}, {4, 4}, {8, 4}, {6, 4}, {3, 2}
};

// table B-12
const mp4_VLC mp4_VLC_MVD_TB12[65] = {
    {0x05, 13}, {0x07, 13}, {0x05, 12}, {0x07, 12}, {0x09, 12}, {0x0b, 12}, {0x0d, 12}, {0x0f, 12},
    {0x09, 11}, {0x0b, 11}, {0x0d, 11}, {0x0f, 11}, {0x11, 11}, {0x13, 11}, {0x15, 11}, {0x17, 11},
    {0x19, 11}, {0x1b, 11}, {0x1d, 11}, {0x1f, 11}, {0x21, 11}, {0x23, 11}, {0x13, 10}, {0x15, 10},
    {0x17, 10}, {0x07,  8}, {0x09,  8}, {0x0b,  8}, {0x07,  7}, {0x03,  5}, {0x03,  4}, {0x03,  3},
    {0x01,  1}, {0x02,  3}, {0x02,  4}, {0x02,  5}, {0x06,  7}, {0x0a,  8}, {0x08,  8}, {0x06,  8},
    {0x16, 10}, {0x14, 10}, {0x12, 10}, {0x22, 11}, {0x20, 11}, {0x1e, 11}, {0x1c, 11}, {0x1a, 11},
    {0x18, 11}, {0x16, 11}, {0x14, 11}, {0x12, 11}, {0x10, 11}, {0x0e, 11}, {0x0c, 11}, {0x0a, 11},
    {0x08, 11}, {0x0e, 12}, {0x0c, 12}, {0x0a, 12}, {0x08, 12}, {0x06, 12}, {0x04, 12}, {0x06, 13},
    {0x04, 13}
};

#define DCSL(q) (q <= 4) ? 8 : (q <= 8) ? (q << 1) : (q <= 24) ? (q + 8) : ((q << 1) - 16)

const Ipp8u mp4_DCScalerLuma[32] = {
    DCSL( 0), DCSL( 1), DCSL( 2), DCSL( 3), DCSL( 4), DCSL( 5), DCSL( 6), DCSL( 7),
    DCSL( 8), DCSL( 9), DCSL(10), DCSL(11), DCSL(12), DCSL(13), DCSL(14), DCSL(15),
    DCSL(16), DCSL(17), DCSL(18), DCSL(19), DCSL(20), DCSL(21), DCSL(22), DCSL(23),
    DCSL(24), DCSL(25), DCSL(26), DCSL(27), DCSL(28), DCSL(29), DCSL(30), DCSL(31)
};

#undef DCSC
#define DCSC(q) (q <= 4) ? 8 : (q <= 24) ? ((q + 13) >> 1) : (q - 6)

const Ipp8u mp4_DCScalerChroma[32] = {
    DCSC( 0), DCSC( 1), DCSC( 2), DCSC( 3), DCSC( 4), DCSC( 5), DCSC( 6), DCSC( 7),
    DCSC( 8), DCSC( 9), DCSC(10), DCSC(11), DCSC(12), DCSC(13), DCSC(14), DCSC(15),
    DCSC(16), DCSC(17), DCSC(18), DCSC(19), DCSC(20), DCSC(21), DCSC(22), DCSC(23),
    DCSC(24), DCSC(25), DCSC(26), DCSC(27), DCSC(28), DCSC(29), DCSC(30), DCSC(31)
};

#undef DCSC

const Ipp8u mp4_cCbCrMvRound16[16] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};

#define DI(b) ((1 << 18) + b) / b

const Ipp32s mp4_DivIntraDivisor[47] = {
    DI( 1), DI( 1), DI( 2), DI( 3), DI( 4), DI( 5), DI( 6), DI( 7), DI( 8), DI( 9),
    DI(10), DI(11), DI(12), DI(13), DI(14), DI(15), DI(16), DI(17), DI(18), DI(19),
    DI(20), DI(21), DI(22), DI(23), DI(24), DI(25), DI(26), DI(27), DI(28), DI(29),
    DI(30), DI(31), DI(32), DI(33), DI(34), DI(35), DI(36), DI(37), DI(38), DI(39),
    DI(40), DI(41), DI(42), DI(43), DI(44), DI(45), DI(46)
};

#undef DI

#define SF  20. / 6.
const Ipp8u mp4_MV_Weigth[32] = {
    0, (Ipp8u)( 1.115*SF+0.5), (Ipp8u)( 1.243*SF+0.5), (Ipp8u)( 1.386*SF+0.5),
       (Ipp8u)( 1.546*SF+0.5), (Ipp8u)( 1.723*SF+0.5), (Ipp8u)( 1.922*SF+0.5), (Ipp8u)( 2.143*SF+0.5),
       (Ipp8u)( 2.389*SF+0.5), (Ipp8u)( 2.664*SF+0.5), (Ipp8u)( 2.970*SF+0.5), (Ipp8u)( 3.311*SF+0.5),
       (Ipp8u)( 3.692*SF+0.5), (Ipp8u)( 4.117*SF+0.5), (Ipp8u)( 4.590*SF+0.5), (Ipp8u)( 5.118*SF+0.5),
       (Ipp8u)( 5.707*SF+0.5), (Ipp8u)( 6.363*SF+0.5), (Ipp8u)( 7.095*SF+0.5), (Ipp8u)( 7.911*SF+0.5),
       (Ipp8u)( 8.821*SF+0.5), (Ipp8u)( 9.835*SF+0.5), (Ipp8u)(10.966*SF+0.5), (Ipp8u)(12.227*SF+0.5),
       (Ipp8u)(13.633*SF+0.5), (Ipp8u)(15.201*SF+0.5), (Ipp8u)(16.949*SF+0.5), (Ipp8u)(18.898*SF+0.5),
       (Ipp8u)(21.072*SF+0.5), (Ipp8u)(23.495*SF+0.5), (Ipp8u)(26.197*SF+0.5), (Ipp8u)(29.209*SF+0.5)
};

#undef SF

const Ipp32s mp4_Inter_Favor[32] = {
    0, 10, 29, 54, 85, 121, 160, 204, 251, 301, 354, 411, 470, 532, 597, 665, 735, 807,
    882, 959, 1038, 1120, 1204, 1290, 1378, 1468, 1560, 1654, 1750, 1848, 1947, 2049
};

// table B-13
const Ipp8u mp4_VLC_DCSIZE_TB13_len[13] = {
    3, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

// table B-14
const Ipp8u mp4_VLC_DCSIZE_TB14_len[13] = {
    2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

// table B-16
static const int l0_offs_TB16[15] = {
    0, 27, 37, 42, 46, 49, 52, 55, 58, 60, 62, 63, 64, 65, 66
};
static const int l0_lmax_TB16[15] = {
    27, 10, 5, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1
};
static const int l1_offs_TB16[21] = {
    67, 75, 78, 80, 82, 84, 86, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101
};
static const int l1_lmax_TB16[21] = {
    8, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const mp4_VLC vlc_TB16[102] = {
    {0x04,  3}, {0x0c,  4}, {0x1e,  5}, {0x1a,  6}, {0x18,  6}, {0x2a,  7}, {0x26,  7}, {0x24,  7},
    {0x2e,  8}, {0x3e,  9}, {0x3c,  9}, {0x3a,  9}, {0x4a, 10}, {0x48, 10}, {0x46, 10}, {0x42, 10},
    {0x42, 11}, {0x40, 11}, {0x1e, 11}, {0x1c, 11}, {0x0e, 12}, {0x0c, 12}, {0x40, 12}, {0x42, 12},
    {0xa0, 13}, {0xa2, 13}, {0xa4, 13}, {0x1c,  5}, {0x28,  7}, {0x2c,  8}, {0x38,  9}, {0x40, 10},
    {0x3e, 10}, {0x1a, 11}, {0x44, 12}, {0xa6, 13}, {0xaa, 13}, {0x16,  6}, {0x2a,  8}, {0x3c, 10},
    {0x18, 11}, {0xac, 13}, {0x22,  7}, {0x36,  9}, {0x3a, 10}, {0x16, 11}, {0x20,  7}, {0x44, 10},
    {0x14, 11}, {0x1a,  7}, {0x38, 10}, {0x10, 11}, {0x24,  8}, {0x36, 10}, {0xa8, 13}, {0x28,  8},
    {0x34, 10}, {0xae, 13}, {0x32,  9}, {0x12, 11}, {0x30,  9}, {0x46, 12}, {0x2e,  9}, {0x32, 10},
    {0x30, 10}, {0x0e, 11}, {0xb0, 13}, {0x0e,  5}, {0x18,  7}, {0x2c,  9}, {0x2e, 10}, {0x0c, 11},
    {0x0a, 12}, {0x08, 12}, {0xb2, 13}, {0x1e,  7}, {0x2c, 10}, {0x0a, 11}, {0x1c,  7}, {0x08, 11},
    {0x22,  8}, {0x48, 12}, {0x20,  8}, {0x4a, 12}, {0x26,  8}, {0xb4, 13}, {0x2a,  9}, {0xb6, 13},
    {0x28,  9}, {0x26,  9}, {0x34,  9}, {0x2a, 10}, {0x28, 10}, {0x26, 10}, {0x24, 10}, {0x22, 10},
    {0x4c, 12}, {0x4e, 12}, {0xb8, 13}, {0xba, 13}, {0xbc, 13}, {0xbe, 13}
};

const mp4_VLC_TCOEF mp4_VLC_TB16 = {
    {14, 20},
    {l0_offs_TB16, l1_offs_TB16},
    {l0_lmax_TB16, l1_lmax_TB16},
    vlc_TB16
};

// table B-17
static const int l0_offs_TB17[27] = {
    0, 12, 18, 22, 25, 28, 31, 34, 36, 38, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57
};
static const int l0_lmax_TB17[27] = {
    12, 6, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const int l1_offs_TB17[41] = {
    58, 61, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101
};
static const int l1_lmax_TB17[41] = {
    3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const mp4_VLC vlc_TB17[102] = {
    {0x04,  3}, {0x1e,  5}, {0x2a,  7}, {0x2e,  8}, {0x3e,  9}, {0x4a, 10}, {0x48, 10}, {0x42, 11},
    {0x40, 11}, {0x0e, 12}, {0x0c, 12}, {0x40, 12}, {0x0c,  4}, {0x28,  7}, {0x3c,  9}, {0x1e, 11},
    {0x42, 12}, {0xa0, 13}, {0x1c,  5}, {0x3a,  9}, {0x1c, 11}, {0xa2, 13}, {0x1a,  6}, {0x46, 10},
    {0x1a, 11}, {0x18,  6}, {0x44, 10}, {0xa4, 13}, {0x16,  6}, {0x18, 11}, {0xa6, 13}, {0x26,  7},
    {0x16, 11}, {0xa8, 13}, {0x24,  7}, {0x14, 11}, {0x22,  7}, {0x12, 11}, {0x20,  7}, {0x10, 11},
    {0x2c,  8}, {0xaa, 13}, {0x2a,  8}, {0x28,  8}, {0x38,  9}, {0x36,  9}, {0x42, 10}, {0x40, 10},
    {0x3e, 10}, {0x3c, 10}, {0x3a, 10}, {0x38, 10}, {0x36, 10}, {0x34, 10}, {0x44, 12}, {0x46, 12},
    {0xac, 13}, {0xae, 13}, {0x0e,  5}, {0x32, 10}, {0x0a, 12}, {0x1e,  7}, {0x08, 12}, {0x1c,  7},
    {0x1a,  7}, {0x18,  7}, {0x26,  8}, {0x24,  8}, {0x22,  8}, {0x20,  8}, {0x34,  9}, {0x32,  9},
    {0x30,  9}, {0x2e,  9}, {0x2c,  9}, {0x2a,  9}, {0x28,  9}, {0x26,  9}, {0x30, 10}, {0x2e, 10},
    {0x2c, 10}, {0x2a, 10}, {0x28, 10}, {0x26, 10}, {0x24, 10}, {0x22, 10}, {0x0e, 11}, {0x0c, 11},
    {0x0a, 11}, {0x08, 11}, {0x48, 12}, {0x4a, 12}, {0x4c, 12}, {0x4e, 12}, {0xb0, 13}, {0xb2, 13},
    {0xb4, 13}, {0xb6, 13}, {0xb8, 13}, {0xba, 13}, {0xbc, 13}, {0xbe, 13}
};

const mp4_VLC_TCOEF mp4_VLC_TB17 = {
    {26, 40},
    {l0_offs_TB17, l1_offs_TB17},
    {l0_lmax_TB17, l1_lmax_TB17},
    vlc_TB17
};

// table B-21
const mp4_VLC mp4_VLC_RMAX_TB21[2][7] = {
    {{1, 14}, {2, 9}, {3, 7}, {4, 3}, {5, 2}, {10, 1}, {27, 0}},
    {{1, 20}, {2, 6}, {3, 1}, {8, 0}}
};

// table B-22
const mp4_VLC mp4_VLC_RMAX_TB22[2][6] = {
    {{1, 26}, {2, 10}, {3, 6}, {4, 2}, {6, 1}, {12, 0}},
    {{1, 40}, {2, 1}, {3, 0}}
};

// table B-23 intra
static const int l0_offs_TB23a[20] = {
    0, 27, 40, 51, 60, 66, 72, 77, 82, 86, 90, 92, 94, 96, 97, 98, 99, 100, 101, 102
};
static const int l0_lmax_TB23a[20] = {
    27, 13, 11, 9, 6, 6, 5, 5, 4, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1
};
static const int l1_offs_TB23a[45] = {
    103, 108, 113, 116, 118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 139, 140, 141, 142, 143, 144, 145, 146,
    147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168
};
static const int l1_lmax_TB23a[45] = {
    5, 5, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const mp4_VLC vlc_TB23a[169] = {
    {0x000c,  4}, {0x000e,  4}, {0x0014,  5}, {0x0012,  6}, {0x0028,  7}, {0x002a,  7}, {0x0068,  8}, {0x00e8,  9},
    {0x00ea,  9}, {0x01ba, 10}, {0x01d8, 10}, {0x03d8, 11}, {0x03da, 11}, {0x03e8, 11}, {0x07d8, 12}, {0x07da, 12},
    {0x07e8, 12}, {0x0efa, 13}, {0x0f78, 13}, {0x1f7a, 14}, {0x1fb8, 14}, {0x0f7a, 13}, {0x1fba, 14}, {0x3f7a, 15},
    {0x3fb8, 15}, {0x3fba, 15}, {0x3ff8, 16}, {0x0002,  5}, {0x0010,  6}, {0x005a,  8}, {0x00d8,  9}, {0x00da,  9},
    {0x01b8, 10}, {0x03ba, 11}, {0x07b8, 12}, {0x07ba, 12}, {0x0ef8, 13}, {0x1f78, 14}, {0x3efa, 15}, {0x3f78, 15},
    {0x0008,  6}, {0x0058,  8}, {0x0178, 10}, {0x03b8, 11}, {0x0778, 12}, {0x077a, 12}, {0x1dfa, 14}, {0x1ef8, 14},
    {0x1efa, 14}, {0x3dfa, 15}, {0x3ef8, 15}, {0x000a,  6}, {0x00b8,  9}, {0x017a, 10}, {0x06fa, 12}, {0x0df8, 13},
    {0x1df8, 14}, {0x3bfa, 15}, {0x3df8, 15}, {0x3ffa, 16}, {0x0018,  7}, {0x00ba,  9}, {0x037a, 11}, {0x07fa, 13},
    {0x0dfa, 13}, {0x37fa, 15}, {0x001a,  7}, {0x00fa, 10}, {0x05f8, 12}, {0x0bf8, 13}, {0x37f8, 15}, {0x3bf8, 15},
    {0x0038,  8}, {0x02f8, 11}, {0x05fa, 12}, {0x0bfa, 13}, {0x5ff8, 16}, {0x003a,  8}, {0x02fa, 11}, {0x06f8, 12},
    {0x1bfa, 14}, {0x5ffa, 16}, {0x0078,  9}, {0x0378, 11}, {0x17fa, 14}, {0x2ffa, 15}, {0x007a,  9}, {0x03fa, 12},
    {0x1bf8, 14}, {0x6ff8, 16}, {0x00f8, 10}, {0x07f8, 13}, {0x01f8, 11}, {0x17f8, 14}, {0x01fa, 11}, {0x6ffa, 16},
    {0x03f8, 12}, {0x0ff8, 14}, {0x0ffa, 14}, {0x1ff8, 15}, {0x1ffa, 15}, {0x2ff8, 15}, {0x77f8, 16}, {0x0016,  5},
    {0x00f0,  9}, {0x07ea, 12}, {0x1fd8, 14}, {0x3fd8, 15}, {0x0024,  6}, {0x01da, 10}, {0x0fb8, 13}, {0x3fda, 15},
    {0x77fa, 16}, {0x0026,  6}, {0x07f0, 12}, {0x7bf8, 16}, {0x0030,  7}, {0x0fba, 13}, {0x0032,  7}, {0x0fd8, 13},
    {0x0044,  7}, {0x1fda, 14}, {0x0046,  7}, {0x1fe8, 14}, {0x006a,  8}, {0x1fea, 14}, {0x0070,  8}, {0x1ff0, 14},
    {0x0072,  8}, {0x1ff2, 14}, {0x0084,  8}, {0x3fe8, 15}, {0x0086,  8}, {0x3fea, 15}, {0x00f2,  9}, {0x3ff0, 15},
    {0x0104,  9}, {0x7bfa, 16}, {0x0106,  9}, {0x01e8, 10}, {0x01ea, 10}, {0x01f0, 10}, {0x01f2, 10}, {0x0204, 10},
    {0x0206, 10}, {0x03ea, 11}, {0x03f0, 11}, {0x03f2, 11}, {0x0404, 11}, {0x0406, 11}, {0x07f2, 12}, {0x0804, 12},
    {0x0806, 12}, {0x0fda, 13}, {0x0fe8, 13}, {0x0fea, 13}, {0x0ff0, 13}, {0x0ff2, 13}, {0x1004, 13}, {0x1006, 13},
    {0x2004, 14}, {0x2006, 14}, {0x3ff2, 15}, {0x4004, 15}, {0x4006, 15}, {0x7df8, 16}, {0x7dfa, 16}, {0x7ef8, 16},
    {0x7efa, 16}
};

const mp4_VLC_TCOEF mp4_VLC_TB23a = {
    {19, 44},
    {l0_offs_TB23a, l1_offs_TB23a},
    {l0_lmax_TB23a, l1_lmax_TB23a},
    vlc_TB23a
};

// table B-23 inter
static const int l0_offs_TB23b[39] = {
    0, 19, 29, 36, 43, 48, 52, 56, 60, 63, 66, 68, 70, 72, 74, 76, 78, 80, 82, 83, 84, 85,
    86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102
};
static const int l0_lmax_TB23b[39] = {
    19, 10, 7, 7, 5, 4, 4, 4, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const int l1_offs_TB23b[45] = {
    103, 108, 113, 116, 118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 139, 140, 141,
    142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168
};
static const int l1_lmax_TB23b[45] = {
    5, 5, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const mp4_VLC vlc_TB23b[169] = {
    {0x000c,  4}, {0x0002,  5}, {0x0008,  6}, {0x0038,  8}, {0x0078,  9}, {0x007a,  9}, {0x00f8, 10}, {0x01f8, 11},
    {0x01fa, 11}, {0x03f8, 12}, {0x03fa, 12}, {0x07f8, 13}, {0x0ff8, 14}, {0x0ffa, 14}, {0x17f8, 14}, {0x17fa, 14},
    {0x1ff8, 15}, {0x1ffa, 15}, {0x3ff8, 16}, {0x000e,  4}, {0x0018,  7}, {0x00b8,  9}, {0x00fa, 10}, {0x02f8, 11},
    {0x05f8, 12}, {0x07fa, 13}, {0x1bf8, 14}, {0x2ff8, 15}, {0x2ffa, 15}, {0x0014,  5}, {0x003a,  8}, {0x0178, 10},
    {0x05fa, 12}, {0x0bf8, 13}, {0x37f8, 15}, {0x37fa, 15}, {0x000a,  6}, {0x00ba,  9}, {0x02fa, 11}, {0x0bfa, 13},
    {0x1bfa, 14}, {0x3bf8, 15}, {0x3ffa, 16}, {0x0010,  6}, {0x00d8,  9}, {0x06f8, 12}, {0x1df8, 14}, {0x5ff8, 16},
    {0x0012,  6}, {0x017a, 10}, {0x06fa, 12}, {0x1dfa, 14}, {0x001a,  7}, {0x0378, 11}, {0x0df8, 13}, {0x3bfa, 15},
    {0x0028,  7}, {0x037a, 11}, {0x0dfa, 13}, {0x5ffa, 16}, {0x002a,  7}, {0x03b8, 11}, {0x1ef8, 14}, {0x0058,  8},
    {0x03ba, 11}, {0x3df8, 15}, {0x005a,  8}, {0x0778, 12}, {0x0068,  8}, {0x0ef8, 13}, {0x00da,  9}, {0x1efa, 14},
    {0x00e8,  9}, {0x3dfa, 15}, {0x00ea,  9}, {0x3ef8, 15}, {0x01b8, 10}, {0x3efa, 15}, {0x01ba, 10}, {0x3f78, 15},
    {0x01d8, 10}, {0x6ff8, 16}, {0x03d8, 11}, {0x03da, 11}, {0x03e8, 11}, {0x077a, 12}, {0x07b8, 12}, {0x07ba, 12},
    {0x07d8, 12}, {0x07da, 12}, {0x07e8, 12}, {0x0efa, 13}, {0x0f78, 13}, {0x0f7a, 13}, {0x1f78, 14}, {0x1f7a, 14},
    {0x1fb8, 14}, {0x1fba, 14}, {0x3f7a, 15}, {0x3fb8, 15}, {0x3fba, 15}, {0x6ffa, 16}, {0x77f8, 16}, {0x0016,  5},
    {0x00f0,  9}, {0x07ea, 12}, {0x1fd8, 14}, {0x3fd8, 15}, {0x0024,  6}, {0x01da, 10}, {0x0fb8, 13}, {0x3fda, 15},
    {0x77fa, 16}, {0x0026,  6}, {0x07f0, 12}, {0x7bf8, 16}, {0x0030,  7}, {0x0fba, 13}, {0x0032,  7}, {0x0fd8, 13},
    {0x0044,  7}, {0x1fda, 14}, {0x0046,  7}, {0x1fe8, 14}, {0x006a,  8}, {0x1fea, 14}, {0x0070,  8}, {0x1ff0, 14},
    {0x0072,  8}, {0x1ff2, 14}, {0x0084,  8}, {0x3fe8, 15}, {0x0086,  8}, {0x3fea, 15}, {0x00f2,  9}, {0x3ff0, 15},
    {0x0104,  9}, {0x7bfa, 16}, {0x0106,  9}, {0x01e8, 10}, {0x01ea, 10}, {0x01f0, 10}, {0x01f2, 10}, {0x0204, 10},
    {0x0206, 10}, {0x03ea, 11}, {0x03f0, 11}, {0x03f2, 11}, {0x0404, 11}, {0x0406, 11}, {0x07f2, 12}, {0x0804, 12},
    {0x0806, 12}, {0x0fda, 13}, {0x0fe8, 13}, {0x0fea, 13}, {0x0ff0, 13}, {0x0ff2, 13}, {0x1004, 13}, {0x1006, 13},
    {0x2004, 14}, {0x2006, 14}, {0x3ff2, 15}, {0x4004, 15}, {0x4006, 15}, {0x7df8, 16}, {0x7dfa, 16}, {0x7ef8, 16},
    {0x7efa, 16}
};

const mp4_VLC_TCOEF mp4_VLC_TB23b = {
    {38, 44},
    {l0_offs_TB23b, l1_offs_TB23b},
    {l0_lmax_TB23b, l1_lmax_TB23b},
    vlc_TB23b
};

} // namespace MPEG4_ENC

#endif //defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)
