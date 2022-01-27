////////////////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that aFgreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include <string.h>
#include <limits.h>
#include <math.h>

#include "umc_h264_tables.h"
#include "umc_h264_to_ipp.h"
#include "umc_h264_bme.h"
#include "umc_h264_video_encoder.h"


namespace UMC_H264_ENCODER
{

#define BS_16x16  20
#define BS_16x8   18
#define BS_8x16   12
#define BS_8x8    10
#define BS_8x4     9
#define BS_4x8     6
#define BS_4x4     5
#define BS_4x2     4
#define BS_2x4     3
#define BS_2x2     2

#define CHECK_CBP_EMPTY_THRESH(sad, mask, factor) \
    if ((sad) < (core_enc->m_EmptyThreshold[iQP] >> (factor))) \
        cur_mb.LocalMacroblockInfo->cbp_luma &= (mask)

#define VertInWind(V) (V.mvx >= xL && V.mvx <= xR && V.mvy >= yT && V.mvy <= yB)
#define VertOutWind(V) (V.mvx < xL || V.mvx > xR || V.mvy < yT || V.mvy > yB)
#define SAD_M(V) H264ENC_MAKE_NAME(SAD)(pCur, pitchPixels, MVADJUST(pRef, pitchPixels, V.mvx, V.mvy), pitchPixels, blockSize) + MVConstraint((V.mvx << SUB_PEL_SHIFT) - predictedMV.mvx, (V.mvy << SUB_PEL_SHIFT) - predictedMV.mvy, pRDQM)

#define TR_SHIFT 8

#define MV_SEARCH_TYPE_FULL             0
#define MV_SEARCH_TYPE_CLASSIC_LOG      1
#define MV_SEARCH_TYPE_LOG              2
#define MV_SEARCH_TYPE_EPZS             3
#define MV_SEARCH_TYPE_FULL_ORTHOGONAL  4
#define MV_SEARCH_TYPE_LOG_ORTHOGONAL   5
#define MV_SEARCH_TYPE_TTS              6
#define MV_SEARCH_TYPE_NEW_EPZS         7
#define MV_SEARCH_TYPE_UMH              8
#define MV_SEARCH_TYPE_SQUARE           9
#define MV_SEARCH_TYPE_FTS             10
#define MV_SEARCH_TYPE_SMALL_DIAMOND   11

#define MV_SEARCH_TYPE_SUBPEL_FULL      0
#define MV_SEARCH_TYPE_SUBPEL_HALF      1
#define MV_SEARCH_TYPE_SUBPEL_SQUARE    2
#define MV_SEARCH_TYPE_SUBPEL_HQ        3
#define MV_SEARCH_TYPE_SUBPEL_DIAMOND   4

#define SB_THRESH_RD  269 >> 8

enum PredType
{
    MVPRED_MEDIAN,
    MVPRED_A,
    MVPRED_B,
    MVPRED_C
};

#define TR_RND (1 << (TR_SHIFT - 1))

#define TRUNCATE_LO(val, lim) \
{ \
    Ipp32s (tmp) = (lim); \
    if ((tmp) < (val)) \
        (val) = (Ipp16s) (tmp); \
}

#define TRUNCATE_HI(val, lim) \
{ \
    Ipp32s (tmp) = (lim); \
    if ((tmp) > (val)) \
        (val) = (Ipp16s) (tmp); \
}

void AdjustIndex(
                 Ipp8u cur_mb_is_bottom,
                 Ipp8u cur_mb_is_field,
                 Ipp8u ref_mb_is_bottom,
                 Ipp8u ref_mb_is_field,
                 Ipp8s& RefIdx)
{
    if (RefIdx<0)
    {
        RefIdx=0;
    }
    if (ref_mb_is_field) //both are AFRM
    {
        if (cur_mb_is_field)
        {
            bool same_parity = (((RefIdx&1) ^ ref_mb_is_bottom) == cur_mb_is_bottom);
            if (same_parity)
                RefIdx&=-2;
            else
                RefIdx|=1;
        }
    }
}

#define PIXBITS 8
#include "umc_h264_me_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_me_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER



