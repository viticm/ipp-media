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

#ifndef __UMC_H264_DEBLOCKING_H
#define __UMC_H264_DEBLOCKING_H

#include "ippdefs.h"
#include "umc_h264_video_encoder.h"

namespace UMC_H264_ENCODER
{

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#define __align(value) __declspec(align(value))
#else // !defined(_MSC_VER)
#define __align(value)
#endif // defined(_MSC_VER)

#define IClip(Min, Max, Val) (((Val) < (Min)) ? (Min) : (((Val) > (Max)) ? (Max) : (Val)))
#define SetEdgeStrength(edge, strength) \
    *((Ipp32u *) (edge)) = (((((strength) * 256) + strength) * 256 + strength) * 256 + strength)
#define CopyEdgeStrength(dst_edge, src_edge) \
    *((Ipp32u *) (dst_edge)) = (*((Ipp32u *) (src_edge)))
#define CompareEdgeStrength(strength, edge) \
    ((((((strength) * 256) + strength) * 256 + strength) * 256 + strength) == *((Ipp32u *) (edge)))

// declare used types and constants
enum
{
    VERTICAL_DEBLOCKING     = 0,
    HORIZONTAL_DEBLOCKING   = 1,
    NUMBER_OF_DIRECTION     = 2
};

enum
{
    CURRENT_BLOCK           = 0,
    NEIGHBOUR_BLOCK         = 1
};

// alpha table
extern
Ipp8u ENCODER_ALPHA_TABLE_8u[52];

// beta table
extern
Ipp8u ENCODER_BETA_TABLE_8u[52];

// clipping table
extern
Ipp8u ENCODER_CLIP_TAB_8u[52][5];

// chroma scaling QP table
extern
Ipp8u ENCODER_QP_SCALE_CR[52];

// masks for external blocks pair "coded bits"
extern
Ipp32u ENCODER_EXTERNAL_BLOCK_MASK[NUMBER_OF_DIRECTION][2][4];

// masks for internal blocks pair "coded bits"
extern
Ipp32u ENCODER_INTERNAL_BLOCKS_MASK[NUMBER_OF_DIRECTION][12];

#define CONVERT_TO_16U(size_alpha, size_clipping)   \
    Ipp32s i;\
    Ipp32s bitDepthScale = 1 << (bit_depth - 8);\
    \
    Ipp16u pAlpha_16u[size_alpha];\
    Ipp16u pBeta_16u[size_alpha];\
    Ipp16u pThresholds_16u[size_clipping];\
    IppiFilterDeblock_16u info;\
    info.pSrcDstPlane = pSrcDst;\
    info.srcDstStep = srcdstStep;\
    info.pAlpha = pAlpha_16u;\
    info.pBeta = pBeta_16u;\
    info.pThresholds = pThresholds_16u;\
    info.pBs = pBS;\
    info.bitDepth = bit_depth;\
    \
    for (i = 0; i < sizeof(pAlpha_16u)/sizeof(pAlpha_16u[0]); i++)\
{\
    pAlpha_16u[i]   = (Ipp16u)(pAlpha[i]*bitDepthScale);\
    pBeta_16u[i]    = (Ipp16u)(pBeta[i]*bitDepthScale);\
}\
    \
    for (i = 0; i < sizeof(pThresholds_16u)/sizeof(pThresholds_16u[0]); i++)\
{\
    pThresholds_16u[i] = (Ipp16u)(pThresholds[i]*bitDepthScale);\
}

#define CONVERT_TO_8U(size_alpha, size_clipping)   \
    IppiFilterDeblock_8u info;\
    info.pSrcDstPlane = pSrcDst;\
    info.srcDstStep = srcdstStep;\
    info.pAlpha = pAlpha;\
    info.pBeta = pBeta;\
    info.pThresholds = pThresholds;\
    info.pBs = pBS;


#define PIXBITS 8
#include "umc_h264_deblocking_tmpl.h"
#undef PIXBITS

#if defined (BITDEPTH_9_12)

#define PIXBITS 16
#include "umc_h264_deblocking_tmpl.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} // namespace UMC_H264_ENCODER

#endif // __UMC_H264_DEBLOCKING_H

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
