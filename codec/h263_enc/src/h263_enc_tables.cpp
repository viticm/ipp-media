/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH263 (tables)
//  Contents:
//                  mVLC_MCBPC_TB7
//                  mVLC_CBPY_TB8
//                  mVLC_MVD_TB12
//
*/
#include "umc_config.h"
#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)
#include "h263_enc.hpp"

#pragma warning(disable : 279)      // controlling expression is constant

// table B-7
const h263e_VLC ippVideoEncoderH263::mVLC_MCBPC_TB7[20] = {
    {1, 1}, {3, 4}, {2, 4}, {5, 6}, {3, 3}, {7, 7}, {6, 7}, {5, 9},
    {2, 3}, {5, 7}, {4, 7}, {5, 8}, {3, 5}, {4, 8}, {3, 8}, {3, 7},
    {4, 6}, {4, 9}, {3, 9}, {2, 9}
};

// table B-8
const h263e_VLC ippVideoEncoderH263::mVLC_CBPY_TB8[16] = {
    {0x3, 4}, {5, 5}, {4, 5}, {9, 4}, {3, 5}, {7, 4}, {2, 6}, {11, 4},
    {0x2, 5}, {3, 6}, {5, 4}, {10, 4}, {4, 4}, {8, 4}, {6, 4}, {3, 2}
};

// table B-12
const h263e_VLC ippVideoEncoderH263::mVLC_MVD_TB12[65] = {
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

const Ipp8u h263e_DCScalerLuma_[32] = {
    DCSL( 0), DCSL( 1), DCSL( 2), DCSL( 3), DCSL( 4), DCSL( 5), DCSL( 6), DCSL( 7),
    DCSL( 8), DCSL( 9), DCSL(10), DCSL(11), DCSL(12), DCSL(13), DCSL(14), DCSL(15),
    DCSL(16), DCSL(17), DCSL(18), DCSL(19), DCSL(20), DCSL(21), DCSL(22), DCSL(23),
    DCSL(24), DCSL(25), DCSL(26), DCSL(27), DCSL(28), DCSL(29), DCSL(30), DCSL(31)
};

#define DCSC(q) (q <= 4) ? 8 : (q <= 24) ? ((q + 13) >> 1) : (q - 6)

const Ipp8u h263e_DCScalerChroma_[32] = {
    DCSC( 0), DCSC( 1), DCSC( 2), DCSC( 3), DCSC( 4), DCSC( 5), DCSC( 6), DCSC( 7),
    DCSC( 8), DCSC( 9), DCSC(10), DCSC(11), DCSC(12), DCSC(13), DCSC(14), DCSC(15),
    DCSC(16), DCSC(17), DCSC(18), DCSC(19), DCSC(20), DCSC(21), DCSC(22), DCSC(23),
    DCSC(24), DCSC(25), DCSC(26), DCSC(27), DCSC(28), DCSC(29), DCSC(30), DCSC(31)
};

const Ipp8u h263e_cCbCrMvRound16_[16] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};
#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
