//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//


#include <math.h>
#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include "umc_h264_video_encoder.h"
#include "umc_h264_tables.h"
#include "umc_h264_to_ipp.h"
#include "umc_h264_bme.h"
#include "ippdefs.h"
#include "ippvc.h"

//#define TRACE_INTRA 50
//#define TRACE_INTER 5
//#define TRACE_INTRA_16X16 185

#define MAX_CAVLC_LEVEL 2063

#define LUMA_MB_MAX_COST 6
#define CHROMA_COEFF_MAX_COST 7
#define LUMA_8X8_MAX_COST 4

#define LUMA_COEFF_8X8_MAX_COST 4
#define LUMA_COEFF_MB_8X8_MAX_COST 6

    static const __ALIGN16 Ipp16s FwdQuantTable_16s[6][16] = {
        {13107, 8066, 13107, 8066, 8066, 5243,  8066, 5243, 13107, 8066, 13107, 8066, 8066, 5243,  8066, 5243},
        {11916, 7490, 11916, 7490, 7490, 4660,  7490, 4660, 11916, 7490, 11916, 7490, 7490, 4660,  7490, 4660},
        {10082, 6554, 10082, 6554, 6554, 4194,  6554, 4194, 10082, 6554, 10082, 6554, 6554, 4194,  6554, 4194},
        {9362, 5825, 9362, 5825, 5825, 3647, 5825, 3647, 9362, 5825, 9362, 5825, 5825, 3647, 5825, 3647},
        {8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355, 8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355},
        {7282, 4559, 7282, 4559, 4559, 2893, 4559, 2893, 7282, 4559, 7282, 4559, 4559, 2893, 4559, 2893}
    };

    static const __ALIGN16 Ipp16s InvLevelScale_4x4_default[6][16] = {
        {160, 208, 160, 208, 208, 256, 208, 256, 160, 208, 160, 208, 208, 256, 208, 256},
        {176, 224, 176, 224, 224, 288, 224, 288, 176, 224, 176, 224, 224, 288, 224, 288},
        {208, 256, 208, 256, 256, 320, 256, 320, 208, 256, 208, 256, 256, 320, 256, 320},
        {224, 288, 224, 288, 288, 368, 288, 368, 224, 288, 224, 288, 288, 368, 288, 368},
        {256, 320, 256, 320, 320, 400, 320, 400, 256, 320, 256, 320, 320, 400, 320, 400},
        {288, 368, 288, 368, 368, 464, 368, 464, 288, 368, 288, 368, 368, 464, 368, 464}
    };

    static const __ALIGN16 Ipp16s InvScale4x4[6][16] = {
        { 2560, 4160, 2560, 4160, 4160, 6400, 4160, 6400, 2560, 4160, 2560, 4160, 4160, 6400, 4160, 6400, },
        { 2816, 4480, 2816, 4480, 4480, 7200, 4480, 7200, 2816, 4480, 2816, 4480, 4480, 7200, 4480, 7200, },
        { 3328, 5120, 3328, 5120, 5120, 8000, 5120, 8000, 3328, 5120, 3328, 5120, 5120, 8000, 5120, 8000, },
        { 3584, 5760, 3584, 5760, 5760, 9200, 5760, 9200, 3584, 5760, 3584, 5760, 5760, 9200, 5760, 9200, },
        { 4096, 6400, 4096, 6400, 6400, 10000, 6400, 10000, 4096, 6400, 4096, 6400, 6400, 10000, 6400, 10000, },
        { 4608, 7360, 4608, 7360, 7360, 11600, 7360, 11600, 4608, 7360, 4608, 7360, 7360, 11600, 7360, 11600 }
    };

    static const __ALIGN16 Ipp8u h264_qp_rem[90]= {
        0,   1,   2,   3,   4,   5,   0,   1,   2,   3,   4,   5,   0,   1,   2,  3,   4,   5,
        0,   1,   2,   3,   4,   5,   0,   1,   2,   3,   4,   5,   0,   1,   2,  3,   4,   5,
        0,   1,   2,   3,   4,   5,   0,   1,   2,   3,   4,   5,   0,   1,   2,  3,   4,   5,
        0,   1,   2,   3,   4,   5,   0,   1,   2,   3,   4,   5,   0,   1,   2,  3,   4,   5,
        0,   1,   2,   3,   4,   5,   0,   1,   2,   3,   4,   5,   0,   1,   2,  3,   4,   5
    };

    static const __ALIGN16 Ipp8u h264_qp6[90] = {
        0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,  2,   2,   2,
        3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   5,   5,   5,  5,   5,   5,
        6,   6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   8,   8,   8,  8,   8,   8,
        9,   9,   9,   9,   9,   9,  10,  10,  10,  10,  10,  10,  11,  11,  11, 11,  11,  11,
       12,  12,  12,  12,  12,  12,  13,  13,  13,  13,  13,  13,  14,  14,  14, 14,  14,  14
    };

    static const __ALIGN16 Ipp8u CorrMatrix[16] = {
        16, 20, 16, 20,
        20, 25, 20, 25,
        16, 20, 16, 20,
        20, 25, 20, 25
    };

static const Ipp32s ctx_neq1p1[8] = { 1,2,3,4,0,0,0,0};
static const Ipp32s ctx_ngt1[8] =   { 5,5,5,5,6,7,8,9};
static const Ipp32s ctx_id_trans0[8] = {1,2,3,3,4,5,6,7};
static const Ipp32s ctx_id_trans1[8] = {4,4,4,4,5,6,7,7};
static const Ipp32s ctx_id_trans13[7] = {4,4,4,4,5,6,6};

void transform4x4(Ipp16s* pSrc, Ipp16s* pDst)
{
    const Ipp16s *psrcrow[4];
    Ipp16s *pdstrow[4];
    Ipp16s a[4];
    Ipp16s i;

    psrcrow[0] = pSrc;
    psrcrow[1] = pSrc + 4;
    psrcrow[2] = pSrc + 8;
    psrcrow[3] = pSrc + 12;
    pdstrow[0] = pDst;
    pdstrow[1] = pDst + 4;
    pdstrow[2] = pDst + 8;
    pdstrow[3] = pDst + 12;

    for (i = 0; i < 4; i ++)
    {
        a[0] = (Ipp16s) (psrcrow[i][0] + psrcrow[i][3]);
        a[3] = (Ipp16s) (psrcrow[i][0] - psrcrow[i][3]);
        a[1] = (Ipp16s) (psrcrow[i][1] + psrcrow[i][2]);
        a[2] = (Ipp16s) (psrcrow[i][1] - psrcrow[i][2]);

        pdstrow[i][0] = (Ipp16s) (a[0] + a[1]);
        pdstrow[i][1] = (Ipp16s) (2 * a[3] + a[2]);
        pdstrow[i][2] = (Ipp16s) (a[0] - a[1]);
        pdstrow[i][3] = (Ipp16s) (a[3] - 2 * a[2]);
    }

    /* vertical */
    for (i = 0; i < 4; i ++)
    {
        a[0] = (Ipp16s) (pdstrow[0][i] + pdstrow[3][i]);
        a[3] = (Ipp16s) (pdstrow[0][i] - pdstrow[3][i]);
        a[1] = (Ipp16s) (pdstrow[1][i] + pdstrow[2][i]);
        a[2] = (Ipp16s) (pdstrow[1][i] - pdstrow[2][i]);

        pdstrow[0][i] = (Ipp16s) (a[0] + a[1]);
        pdstrow[1][i] = (Ipp16s) (2 * a[3] + a[2]);
        pdstrow[2][i] = (Ipp16s) (a[0] - a[1]);
        pdstrow[3][i] = (Ipp16s) (a[3] - 2 * a[2]);
    }
}

Ipp16s quant(Ipp16s coeff, Ipp32s pos, Ipp32s QP, Ipp32s intra)
{
    H264ENC_UNREFERENCED_PARAMETER(intra);
    Ipp32s  qp_rem, qp6;
    const   Ipp16s*  quantTbl;
    Ipp16s sign;
    Ipp32s scale;
    Ipp32s scaleOffset;

    qp_rem = h264_qp_rem[QP];
    qp6 = h264_qp6[QP];

    quantTbl = FwdQuantTable_16s[qp_rem];
    scale = 15+qp6;
//    scaleOffset = intra ? (1<<scale)/3 : (1<<scale)/6;
//    scaleOffset = (1<<(scale-1)); //intra ? (1<<scale)/3 : (1<<scale)/6;
    scaleOffset = 1<<scale; //intra ? (1<<scale)/3 : (1<<scale)/6;

    sign = (Ipp16s)(coeff < 0 ? -1:1);
    return (Ipp16s)(sign*((sign*coeff*quantTbl[pos]+scaleOffset)>>scale));
}

//pos - non zig-zag scan pos (normal pos)
Ipp16s dequant(Ipp16s coeff,Ipp32s pos, Ipp32s QP)
{
    Ipp32s  qp_rem, qp6;
    const   Ipp16s*  invQuantTbl;

    qp_rem = h264_qp_rem[QP];
    qp6 = h264_qp6[QP];

    //invQuantTbl = (pScaleLevelsInv == NULL)? InvLevelScale_4x4_default[qp_rem]: pScaleLevelsInv;
    invQuantTbl = InvLevelScale_4x4_default[qp_rem];

    if( QP >= 24 ){
        Ipp32s shift = qp6 - 4;
        return  (coeff*invQuantTbl[pos])<<shift;
    }else{
        Ipp32s shift = 4 - qp6;
        Ipp32s offset = 1<<(shift - 1);
        return (coeff*invQuantTbl[pos]+offset)>>shift;
    }
}

IppStatus TransformQuantOptFwd4x4_H264_16s32s_C1(
    Ipp16s* pSrc,
    Ipp32s* pDst,
    Ipp32s  Qp,
    Ipp32s* pNumLevels,
    Ipp32s  intra,
    const Ipp16s* pScanMatrix,
    Ipp32s* pLastCoeff,
    const Ipp16s* pLevelScales,
    const H264Slice_16u32s* curr_slice,
    Ipp32s uBlock,
    CabacStates* cbSt)
{
    H264ENC_UNREFERENCED_PARAMETER(pSrc);
    H264ENC_UNREFERENCED_PARAMETER(pDst);
    H264ENC_UNREFERENCED_PARAMETER(Qp);
    H264ENC_UNREFERENCED_PARAMETER(pNumLevels);
    H264ENC_UNREFERENCED_PARAMETER(intra);
    H264ENC_UNREFERENCED_PARAMETER(pScanMatrix);
    H264ENC_UNREFERENCED_PARAMETER(pLastCoeff);
    H264ENC_UNREFERENCED_PARAMETER(pLevelScales);
    H264ENC_UNREFERENCED_PARAMETER(curr_slice);
    H264ENC_UNREFERENCED_PARAMETER(uBlock);
    H264ENC_UNREFERENCED_PARAMETER(cbSt);
    return ippStsNoErr;
}

static const Ipp8u bitcountLPS[128][4] = {
{ 1, 1, 1, 1,  },
{ 1, 1, 1, 1,  },
{ 1, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 2,  },
{ 2, 2, 2, 2,  },
{ 2, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 4, 3,  },
{ 4, 4, 4, 3,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 6, 5, 5,  },
{ 6, 6, 5, 5,  },
{ 6, 6, 6, 6,  },
{ 1, 1, 1, 1,  },
{ 1, 1, 1, 1,  },
{ 1, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 1, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 1, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 1,  },
{ 2, 2, 2, 2,  },
{ 2, 2, 2, 2,  },
{ 2, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 2, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 2, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 2,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 3, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 3, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 3, 3,  },
{ 4, 4, 4, 3,  },
{ 4, 4, 4, 3,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 4, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 4, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 4, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 4,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 5, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 5, 5, 5,  },
{ 6, 6, 5, 5,  },
{ 6, 6, 5, 5,  },
{ 6, 6, 6, 6,  },
};

static const Ipp16u rangeLPS[128][4] = {
{ 256, 352, 416, 480,  },
{ 256, 334, 394, 454,  },
{ 256, 316, 374, 432,  },
{ 492, 300, 356, 410,  },
{ 464, 284, 338, 390,  },
{ 444, 270, 320, 370,  },
{ 420, 256, 304, 350,  },
{ 400, 488, 288, 332,  },
{ 380, 464, 274, 316,  },
{ 360, 440, 260, 300,  },
{ 340, 416, 492, 284,  },
{ 324, 396, 468, 270,  },
{ 308, 376, 444, 256,  },
{ 292, 356, 420, 488,  },
{ 276, 340, 400, 464,  },
{ 264, 320, 380, 440,  },
{ 496, 304, 360, 416,  },
{ 472, 288, 344, 396,  },
{ 448, 276, 324, 376,  },
{ 424, 260, 308, 356,  },
{ 408, 496, 292, 340,  },
{ 384, 472, 276, 320,  },
{ 368, 448, 264, 304,  },
{ 344, 424, 504, 288,  },
{ 328, 400, 472, 276,  },
{ 312, 384, 448, 260,  },
{ 296, 360, 432, 496,  },
{ 280, 344, 408, 472,  },
{ 264, 328, 384, 448,  },
{ 256, 312, 368, 424,  },
{ 480, 296, 344, 400,  },
{ 464, 280, 328, 384,  },
{ 432, 264, 312, 360,  },
{ 416, 496, 296, 344,  },
{ 384, 480, 280, 328,  },
{ 368, 448, 264, 312,  },
{ 352, 432, 256, 296,  },
{ 336, 416, 480, 280,  },
{ 320, 384, 464, 264,  },
{ 304, 368, 432, 496,  },
{ 288, 352, 416, 480,  },
{ 272, 336, 400, 448,  },
{ 256, 320, 368, 432,  },
{ 480, 304, 352, 400,  },
{ 448, 288, 336, 384,  },
{ 448, 272, 320, 368,  },
{ 416, 256, 304, 352,  },
{ 384, 480, 288, 336,  },
{ 384, 448, 272, 320,  },
{ 352, 448, 256, 304,  },
{ 352, 416, 480, 288,  },
{ 320, 384, 480, 272,  },
{ 320, 384, 448, 256,  },
{ 288, 352, 416, 480,  },
{ 288, 352, 384, 448,  },
{ 256, 320, 384, 448,  },
{ 256, 288, 352, 416,  },
{ 448, 288, 352, 384,  },
{ 448, 288, 320, 384,  },
{ 448, 256, 320, 352,  },
{ 384, 256, 288, 352,  },
{ 384, 448, 288, 320,  },
{ 384, 448, 256, 288,  },
{ 128, 128, 128, 128,  },

{ 256, 352, 416, 480,  },
{ 256, 334, 394, 454,  },
{ 256, 316, 374, 432,  },
{ 492, 300, 356, 410,  },
{ 464, 284, 338, 390,  },
{ 444, 270, 320, 370,  },
{ 420, 256, 304, 350,  },
{ 400, 488, 288, 332,  },
{ 380, 464, 274, 316,  },
{ 360, 440, 260, 300,  },
{ 340, 416, 492, 284,  },
{ 324, 396, 468, 270,  },
{ 308, 376, 444, 256,  },
{ 292, 356, 420, 488,  },
{ 276, 340, 400, 464,  },
{ 264, 320, 380, 440,  },
{ 496, 304, 360, 416,  },
{ 472, 288, 344, 396,  },
{ 448, 276, 324, 376,  },
{ 424, 260, 308, 356,  },
{ 408, 496, 292, 340,  },
{ 384, 472, 276, 320,  },
{ 368, 448, 264, 304,  },
{ 344, 424, 504, 288,  },
{ 328, 400, 472, 276,  },
{ 312, 384, 448, 260,  },
{ 296, 360, 432, 496,  },
{ 280, 344, 408, 472,  },
{ 264, 328, 384, 448,  },
{ 256, 312, 368, 424,  },
{ 480, 296, 344, 400,  },
{ 464, 280, 328, 384,  },
{ 432, 264, 312, 360,  },
{ 416, 496, 296, 344,  },
{ 384, 480, 280, 328,  },
{ 368, 448, 264, 312,  },
{ 352, 432, 256, 296,  },
{ 336, 416, 480, 280,  },
{ 320, 384, 464, 264,  },
{ 304, 368, 432, 496,  },
{ 288, 352, 416, 480,  },
{ 272, 336, 400, 448,  },
{ 256, 320, 368, 432,  },
{ 480, 304, 352, 400,  },
{ 448, 288, 336, 384,  },
{ 448, 272, 320, 368,  },
{ 416, 256, 304, 352,  },
{ 384, 480, 288, 336,  },
{ 384, 448, 272, 320,  },
{ 352, 448, 256, 304,  },
{ 352, 416, 480, 288,  },
{ 320, 384, 480, 272,  },
{ 320, 384, 448, 256,  },
{ 288, 352, 416, 480,  },
{ 288, 352, 384, 448,  },
{ 256, 320, 384, 448,  },
{ 256, 288, 352, 416,  },
{ 448, 288, 352, 384,  },
{ 448, 288, 320, 384,  },
{ 448, 256, 320, 352,  },
{ 384, 256, 288, 352,  },
{ 384, 448, 288, 320,  },
{ 384, 448, 256, 288,  },
{ 128, 128, 128, 128,  },
};

Ipp32s w[16] = {
    25,10,25,10,
    10, 4,10, 4,
    25,10,25,10,
    10, 4,10, 4
};

#if 0
/* int(-log2(p)*256 + 0.5)
   Based on original CABAC probabilities from paper
   H.Schwarz, T.Wiegand "Context-Based Adaptive Binary Arithmetic Coding in the H.264/AVC Video Compression Standard" */
const Ipp32s p_bits_lps[64] = {
 256,  275,  294,  314,
 333,  352,  371,  391,
 410,  429,  448,  468,
 487,  506,  525,  545,
 564,  583,  602,  622,
 641,  660,  679,  699,
 718,  737,  756,  776,
 795,  814,  833,  853,
 872,  891,  910,  930,
 949,  968,  987, 1007,
1026, 1045, 1064, 1084,
1103, 1122, 1141, 1161,
1180, 1199, 1218, 1238,
1257, 1276, 1295, 1315,
1334, 1353, 1372, 1392,
1411, 1430, 1449, 1469
};

const Ipp32s p_bits_mps[64]={
 256,  238,  221,  206,
 192,  180,  168,  157,
 148,  139,  130,  122,
 115,  108,  102,   96,
  90,   85,   80,   76,
  72,   68,   64,   60,
  57,   54,   51,   48,
  46,   43,   41,   39,
  37,   35,   33,   31,
  29,   28,   26,   25,
  24,   22,   21,   20,
  19,   18,   17,   16,
  15,   15,   14,   13,
  12,   12,   11,   11,
  10,   10,    9,    9,
   8,    8,    7,    7,
};
#endif

/* Average from H.264 standard range table */
//extern const Ipp32s p_bits[128];
#if 0
static const Ipp32s p_bits[128]={
//MPS bits
 246,  232,  220,  206,
 192,  179,  167,  157,
 148,  138,  129,  122,
 115,  108,  102,   96,
  90,   85,   80,   75,
  72,   67,   64,   60,
  57,   54,   51,   48,
  45,   43,   41,   39,
  36,   35,   33,   31,
  30,   28,   26,   25,
  24,   23,   21,   20,
  19,   18,   17,   16,
  15,   15,   14,   13,
  13,   12,   11,   11,
  10,    9,    9,    9,
   8,    8,    7,    2,
//LPS bits
 266,  282,  296,  314,
 334,  353,  373,  391,
 410,  430,  450,  468,
 488,  507,  526,  545,
 566,  584,  603,  624,
 641,  662,  679,  700,
 720,  738,  757,  776,
 796,  813,  835,  852,
 874,  893,  912,  933,
 948,  967,  987, 1008,
1024, 1044, 1065, 1087,
1106, 1120, 1141, 1164,
1180, 1198, 1216, 1237,
1249, 1280, 1294, 1313,
1338, 1358, 1366, 1385,
1409, 1430, 1451, 1928
};
#endif
#if 0
/* Average overall input range */
const Ipp32s p_bits_mps[64]={
 247,  233,  221,  207,
 193,  180,  168,  158,
 148,  139,  130,  123,
 115,  108,  102,   96,
  90,   85,   81,   76,
  72,   68,   64,   60,
  57,   54,   51,   48,
  46,   44,   41,   39,
  37,   35,   33,   31,
  30,   28,   27,   25,
  24,   23,   21,   20,
  19,   18,   17,   16,
  16,   15,   14,   13,
  13,   12,   11,   11,
  10,   10,    9,    9,
   8,    8,    7,    2
};
#endif

#define MAX_COST ((((Ipp64u)1)<<63)-1)

typedef struct _node{
     Ipp64u cost;
     Ipp16s coeff_id;
     Ipp8u cabac_states_alevelm1[10];
} node;

typedef struct _node8x8{
     Ipp64u cost;
     Ipp16s coeff_id;
     Ipp8u cabac_states_alevelm1[10];
} node8x8;

typedef struct _coeff_n{
    Ipp16s coeff;
    Ipp16s prev_coeff;
} coeff_n;

#define P_BITS

/* int( (pow(2.0,MAX(0,(iQP-12))/3.0)*0.85) + 0.5) */
static const Ipp32u lambda_sq[87] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 3, 3, 4, 5, 7, 9, 11, 14, 17, 22, 27,
    34, 43, 54, 69, 86, 109, 137, 173, 218, 274,
    345, 435, 548, 691, 870, 1097, 1382, 1741,
    2193, 2763, 3482, 4387, 5527, 6963, 8773,
    11053, 13926, 17546, 22107, 27853, 35092,
    44214, 55706, 70185, 88427, 111411, 140369,
    176854, 222822, 280739, 353709, 445645, 561477,
    707417, 891290, 1122955, 1414834, 1782579,
    2245909, 2829668, 3565158, 4491818, 5659336,
    7130317, 8983636, 11318672, 14260634, 17967272,
    22637345,
};
/* Inter: int(pow(2.0,(double)qp/3)*0.84*400/16+0.5)
   Intra: int(pow(2.0,(double)qp/3)*0.84*0.9*400/16+0.5)
*/
/*const Ipp32s lambda_t[2][52] = {
{ //Inter
   21,    26,    33,    42,    53,    67,    84,   106,
  133,   168,   212,   267,   336,   423,   533,   672,
  847,  1067,  1344,  1693,  2133,  2688,  3387,  4267,
 5376,  6773,  8534, 10752, 13547, 17068, 21504, 27093,
34135, 43008, 54187, 68271, 86016, 108373, 136542, 172032,
216747, 273084, 344064, 433493, 546168, 688128, 866987, 1092335,
1376256, 1733974, 2184670, 2752512,
},
{ //Intra
   19,    24,    30,    38,    48,    60,    76,    95,
  120,   151,   191,   240,   302,   381,   480,   605,
  762,   960,  1210,  1524,  1920,  2419,  3048,  3840,
 4838,  6096,  7680,  9677, 12192, 15361, 19354, 24384,
30722, 38707, 48768, 61444, 77414, 97536, 122888, 154829,
195072, 245775, 309658, 390144, 491551, 619315, 780288, 983102,
1238630, 1560577, 1966203, 2477261,
}
};
*/
#if 1
const Ipp32s lambda_t[2][52] = {
{
   19,    24,    30,    38,    48,    60,    76,    96,
  121,   152,   192,   242,   306,   386,   486,   614,
  774,   976,  1232,  1554,  1962,  2475,  3124,  3943,
 4977,  6283,  7932, 10015, 12645, 15968, 20165, 25466,
32164, 40627, 51320, 64831, 81906, 103485, 130757, 165229,
208802, 263884, 333519, 421557, 532869, 673615, 851590, 1076656,
1361289, 1721276, 2176591, 2752512,
},
{
/*   11,    14,    18,    22,    28,    35,    45,    56,
   71,    89,   113,   142,   180,   227,   286,   361,
  455,   574,   725,   914,  1154,  1456,  1838,  2319,
 2928,  3696,  4666,  5891,  7438,  9393, 11862, 14980,
18920, 23898, 30188, 38136, 48180, 60874, 76916, 97194,
122825, 155226, 196188, 247975, 313452, 396244, 500935, 633327,
800758, 1012515, 1280348, 1619125,
*/

   12,    16,    19,    24,    31,    39,    49,    62,
   78,    99,   124,   157,   198,   250,   315,   398,
  502,   634,   801,  1011,  1277,  1613,  2037,  2574,
 3252,  4109,  5193,  6563,  8296, 10487, 13259, 16764,
21199, 26809, 33908, 42890, 54258, 68646, 86856, 109908,
139091, 176039, 222823, 282064, 357088, 452105, 572454, 724900,
918019, 1162679, 1472662, 1865436,

/*   13,    17,    21,    26,    33,    42,    53,    67,
   84,   106,   134,   169,   213,   269,   339,   428,
  541,   683,   862,  1089,  1375,  1737,  2194,  2772,
 3502,  4425,  5592,  7068,  8934, 11294, 14278, 18053,
22829, 28871, 36516, 46189, 58432, 73926, 93537, 118362,
149791, 189581, 239963, 303762, 384556, 486882, 616488, 780662,
988635, 1252116, 1585944, 2008931,
*/
/*   17,    22,    27,    34,    43,    55,    69,    87,
  109,   138,   174,   220,   277,   350,   441,   557,
  703,   888,  1121,  1416,  1788,  2258,  2852,  3603,
 4553,  5753,  7270,  9188, 11614, 14682, 18562, 23469,
29678, 37532, 47471, 60046, 75961, 96104, 121598, 153871,
194728, 246455, 311952, 394890, 499923, 632947, 801435, 1014860,
1285226, 1627751, 2061727, 2611610,
*/
/*   15,    20,    25,    31,    39,    50,    63,    79,
   99,   125,   158,   200,   252,   318,   401,   506,
  639,   807,  1019,  1287,  1625,  2053,  2593,  3275,
 4139,  5230,  6609,  8353, 10558, 13347, 16875, 21335,
26980, 34120, 43155, 54587, 69055, 87367, 110544, 139883,
177025, 224050, 283593, 358991, 454475, 575406, 728577, 922600,
1168387, 1479774, 1874297, 2374191,
*/
}
};
#endif


#undef TRANS4x4_OPT
#ifdef INTRINSIC_OPT
    #define TRANS4x4_OPT
#endif


static __ALIGN16 const Ipp8u enc_scan[2][16] = {
    {0,1,5,6,2,4,7,12,3,8,11,13,9,10,14,15},
    {0,2,8,12,1,5,9,13,3,6,10,14,4,7,11,15}
};

static __ALIGN16 const Ipp16s offset_dequant[4][16]={
    { 128, 160, 128, 160, 160, 200, 160, 200, 128, 160, 128, 160, 160, 200, 160, 200, },
    { 64, 80, 64, 80, 80, 100, 80, 100, 64, 80, 64, 80, 80, 100, 80, 100, },
    { 32, 40, 32, 40, 40, 50, 40, 50, 32, 40, 32, 40, 40, 50, 40, 50, },
    { 16, 20, 16, 20, 20, 25, 20, 25, 16, 20, 16, 20, 20, 25, 20, 25, }
};

IppStatus TransformQuantOptFwd4x4_H264_16s_C1(
    Ipp16s* pSrc,
    Ipp16s* pDst,
    Ipp32s  Qp,
    Ipp32s* pNumLevels,
    Ipp32s  Intra,
    Ipp16s* pScanMatrix,
    Ipp32s* pLastCoeff,
    Ipp16s* pScaleLevels,
    const H264Slice_8u16s* curr_slice,
    Ipp32s sCoeff,
    CabacStates* cbSt)
{
    H264ENC_UNREFERENCED_PARAMETER(pScaleLevels);
    H264ENC_UNREFERENCED_PARAMETER(pScanMatrix);
    Ipp32s i;
    __ALIGN16 Ipp16s t[16];
    const Ipp32s isField = curr_slice->m_is_cur_mb_field;
    const Ipp32u lambda = lambda_t[Intra][Qp];
//    if( Intra ) lambda = lambda_t[Intra][Qp]*1.2;
//    else lambda = lambda_t[Intra][Qp];

    //Ipp64u lambda = 400*curr_slice->m_cur_mb.lambda;
    const Ipp32s* dec_scan = dec_single_scan[isField];
    const Ipp8u*  escan = enc_scan[isField];
    const Ipp32s qp_rem = h264_qp_rem[Qp];
    const Ipp32s qp6 = h264_qp6[Qp];
    const Ipp16s* quantTbl = FwdQuantTable_16s[qp_rem];
    const Ipp16s* invQuantTbl = InvScale4x4[qp_rem];
    const Ipp32s scale = 15+qp6;
    const Ipp32s scaleOffset = 1<<scale;
    Ipp32s shift_dq;
    Ipp32s offset_d = 0;
    __ALIGN16 Ipp16s sign[16];

    if( Qp >= 24 ){
        shift_dq = qp6 - 4;
    }else{
        shift_dq = 4 - qp6;
        offset_d = (1<<(shift_dq-1));
    }

#ifdef TRANS4x4_OPT
#if 0
    __ALIGN16 __m128i _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_6, _p_7;

    _p_0 = _mm_loadl_epi64((__m128i*)(&pSrc[0]));
    _p_1 = _mm_loadl_epi64((__m128i*)(&pSrc[4]));
    _p_2 = _mm_loadl_epi64((__m128i*)(&pSrc[8]));
    _p_3 = _mm_loadl_epi64((__m128i*)(&pSrc[12]));
//Vertical
    _p_0 = _mm_unpacklo_epi16( _p_0, _p_1 );
    _p_2 = _mm_unpacklo_epi16( _p_2, _p_3 );
    _p_1 = _p_0;
    _p_0 = _mm_unpacklo_epi32( _p_0, _p_2 );
    _p_1 = _mm_unpackhi_epi32( _p_1, _p_2 );

    _p_1 = _mm_shuffle_epi32( _p_1, 0x4e );
    _p_5 = _p_0;
    _p_0 = _mm_adds_epi16( _p_0, _p_1 );
    _p_5 = _mm_subs_epi16( _p_5, _p_1 );
    _p_6 = _p_0;
    _p_1 = _p_0;
    _p_0 = _mm_srli_si128( _p_0, 8);
    _p_1 = _mm_subs_epi16( _p_1, _p_0 );
    _p_0 = _mm_adds_epi16( _p_0, _p_6 );
    _p_2 = _p_5;
    _p_2 = _mm_slli_epi16( _p_2, 1 );
    _p_2 = _mm_shuffle_epi32( _p_2, 0x4e );
    _p_3 = _p_2;
    _p_2 = _mm_adds_epi16( _p_2, _p_5 );
    _p_2 = _mm_srli_si128( _p_2, 8);
    _p_5 = _mm_subs_epi16( _p_5, _p_3 );
//Horizontal
    _p_0 = _mm_unpacklo_epi16( _p_0, _p_2 );
    _p_1 = _mm_unpacklo_epi16( _p_1, _p_5 );
    _p_2 = _p_0;
    _p_0 = _mm_unpacklo_epi32( _p_0, _p_1 );
    _p_2 = _mm_unpackhi_epi32( _p_2, _p_1 );

    _p_2 = _mm_shuffle_epi32( _p_2, 0x4e );
    _p_5 = _p_0;
    _p_0 = _mm_adds_epi16( _p_0, _p_2 );
    _p_5 = _mm_subs_epi16( _p_5, _p_2 );
    _p_6 = _p_0;
    _p_1 = _p_0;
    _p_0 = _mm_srli_si128( _p_0, 8);
    _p_1 = _mm_subs_epi16( _p_1, _p_0 ); //result 2 _p_1
    _p_0 = _mm_adds_epi16( _p_0, _p_6 ); //result 0 _p_0
    _p_2 = _p_5;
    _p_2 = _mm_slli_epi16( _p_2, 1 );
    _p_2 = _mm_shuffle_epi32( _p_2, 0x4e );
    _p_3 = _p_2;
    _p_2 = _mm_adds_epi16( _p_2, _p_5 );
    _p_2 = _mm_srli_si128( _p_2, 8);      //result 1 _p_2
    _p_5 = _mm_subs_epi16( _p_5, _p_3 );     //result 3 _p_5
    _p_0 = _mm_unpacklo_epi64(_p_0,_p_2);
    _p_1 = _mm_unpacklo_epi64(_p_1,_p_5);
    _mm_store_si128((__m128i*)&t[0], _p_0 );
    _mm_store_si128((__m128i*)&t[8], _p_1 );
#else
//Alternative transform
//Matrix pre mult
    __ALIGN16 __m128i p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = _mm_load_si128((__m128i*)(&pSrc[0]));
    p1 = _mm_load_si128((__m128i*)(&pSrc[8]));
    p1 = _mm_shuffle_epi32( p1, 0x4e );
    p2 = p0;
    p0 = _mm_adds_epi16( p0, p1 );
    p2 = _mm_subs_epi16( p2, p1 );
    p1 = _mm_shuffle_epi32( p0, 0x4e );
    p3 = _mm_shuffle_epi32( p2, 0x4e );

    p4 = _mm_adds_epi16( p0, p1 );
    p5 = _mm_adds_epi16( p3, p2 );
    p6 = _mm_subs_epi16( p0, p1 );
    p7 = _mm_subs_epi16( p2, p3 );
    p5 = _mm_adds_epi16( p5, p2 );
    p7 = _mm_subs_epi16( p7, p3 );
//Transpose matrix
    p4 = _mm_unpacklo_epi16( p4, p5 );
    p6 = _mm_unpacklo_epi16( p6, p7 );
    p0 = p4;
    p4 = _mm_unpacklo_epi32( p4, p6 );
    p0 = _mm_unpackhi_epi32( p0, p6 );
//Matrix post mult
    p0 = _mm_shuffle_epi32( p0, 0x4e );
    p2 = p4;
    p4 = _mm_adds_epi16( p4, p0 );
    p2 = _mm_subs_epi16( p2, p0 );
    p0 = _mm_shuffle_epi32( p4, 0x4e );
    p3 = _mm_shuffle_epi32( p2, 0x4e );

    p6 = _mm_subs_epi16( p4, p0 );
    p5 = _mm_adds_epi16( p3, p2 );
    p4 = _mm_adds_epi16( p4, p0 );
    p7 = _mm_subs_epi16( p2, p3 );
    p5 = _mm_adds_epi16( p5, p2 );
    p7 = _mm_subs_epi16( p7, p3 );
//Transpose matrix
    p4 = _mm_unpacklo_epi16( p4, p5 );
    p6 = _mm_unpacklo_epi16( p6, p7 );
    p0 = p4;
    p4 = _mm_unpacklo_epi32( p4, p6 );
    p0 = _mm_unpackhi_epi32( p0, p6 );

//    _mm_store_si128((__m128i*)&t[0], p4 );
//    _mm_store_si128((__m128i*)&t[8], p0 );
/*
//Count number non-zeros
    p3 = _mm_xor_si128(p3,p3);
    p6 = _mm_xor_si128(p6,p6);
    p5 = p4;
    p5 = _mm_packs_epi16(p5,p0);
    p3 = _mm_cmpeq_epi8(p3,p5);
    p6 = _mm_sad_epu8( p6, p3 );
    p5 = _mm_shuffle_epi32( p6, 0x0e );
    p6 = _mm_add_epi32( p6,p5 );
*/

//Store signs
    p1 = _mm_xor_si128(p1,p1);
    p2 = _mm_xor_si128(p2,p2);
    p1 = _mm_cmpgt_epi16(p1,p4);
    p2 = _mm_cmpgt_epi16(p2,p0);
    _mm_store_si128((__m128i*)&sign[0], p1 );
    _mm_store_si128((__m128i*)&sign[8], p2 );
//Make abs tranform
    p4 = _mm_adds_epi16(p4,p1);
    p4 = _mm_xor_si128(p4,p1);
    p0 = _mm_adds_epi16(p0,p2);
    p0 = _mm_xor_si128(p0,p2);
    _mm_store_si128((__m128i*)&t[0], p4 );
    _mm_store_si128((__m128i*)&t[8], p0 );
//Quant
    p6 = _mm_load_si128((__m128i*)&quantTbl[0]);
    p7 = _mm_load_si128((__m128i*)&quantTbl[8]);
    p3 = p4;
    p5 = p0;

    p3 = _mm_mullo_epi16(p3,p6);
    p5 = _mm_mullo_epi16(p5,p7);
    p4 = _mm_mulhi_epu16(p4,p6);
    p0 = _mm_mulhi_epu16(p0,p7);

    p6 = _mm_cvtsi32_si128(scaleOffset);
    p7 = _mm_cvtsi32_si128(scale);
    p2 = p3;
    p1 = p5;
    p2 = _mm_unpacklo_epi16( p2, p4 );
    p3 = _mm_unpackhi_epi16( p3, p4 );
    p1 = _mm_unpacklo_epi16( p1, p0 );
    p5 = _mm_unpackhi_epi16( p5, p0 );
//Add scale
    p6 = _mm_shuffle_epi32(p6, 0);
    p2 = _mm_add_epi32(p2,p6);
    p3 = _mm_add_epi32(p3,p6);
    p1 = _mm_add_epi32(p1,p6);
    p5 = _mm_add_epi32(p5,p6);

    p2 = _mm_sra_epi32(p2,p7);
    p3 = _mm_sra_epi32(p3,p7);
    p1 = _mm_sra_epi32(p1,p7);
    p5 = _mm_sra_epi32(p5,p7);

    p2 = _mm_packs_epi32(p2,p3);
    p1 = _mm_packs_epi32(p1,p5);

    _mm_store_si128((__m128i*)&pDst[0], p2 );
    _mm_store_si128((__m128i*)&pDst[8], p1 );
//Get last
    p3 = _mm_xor_si128(p3,p3);
    p5 = p2;
    p5 = _mm_packs_epi16(p5,p1);
    p3 = _mm_cmpeq_epi8(p3,p5);
    //load scan matrix
    p5 = _mm_load_si128((__m128i*)escan);
    p3 = _mm_andnot_si128(p3,p5);
    p5 = _mm_shuffle_epi32( p3, 0x4e ); //could be shift
    p3 = _mm_max_epu8(p3,p5);
    p5 = _mm_shufflelo_epi16( p3, 0x4e ); //could be shift
    p3 = _mm_max_epu8(p3,p5);
    p5 = p3;
    p5 = _mm_srli_epi32( p5, 16);
    p3 = _mm_max_epu8(p3,p5);
    p5 = p3;
    p5 = _mm_srli_epi16( p5, 8);
    p3 = _mm_max_epu8(p3,p5);
    *pLastCoeff =  _mm_cvtsi128_si32(p3) & 0xff;
//Dequant
/*
            Ipp32s d = (tcoeff<<6) + 32; //*64
            if( Qp >= 24 )
                d = (d - (c*invQuantTbl[pos]<<shift_dq)*CorrMatrix[pos])>>6;
            else
                d = (d - ((c*invQuantTbl[pos]+offset_dq)>>shift_dq)*CorrMatrix[pos])>>6;
*/
/*
    if( Qp >= 24 ){
         p6 = _mm_load_si128((__m128i*)&quantTbl[0]);
         p7 = _mm_load_si128((__m128i*)&quantTbl[8]);
    p3 = p4;
    p5 = p0;

    p3 = _mm_mullo_epi16(p3,p6);
    p5 = _mm_mullo_epi16(p5,p7);
    p4 = _mm_mulhi_epu16(p4,p6);
    p0 = _mm_mulhi_epu16(p0,p7);

    p6 = _mm_cvtsi32_si128(scaleOffset);
    p7 = _mm_cvtsi32_si128(scale);
    p2 = p3;
    p1 = p5;
    p2 = _mm_unpacklo_epi16( p2, p4 );
    p3 = _mm_unpackhi_epi16( p3, p4 );
    p1 = _mm_unpacklo_epi16( p1, p0 );
    p5 = _mm_unpackhi_epi16( p5, p0 );
//Add scale
    p6 = _mm_shuffle_epi32(p6, 0);
    p2 = _mm_add_epi32(p2,p6);
    p3 = _mm_add_epi32(p3,p6);
    p1 = _mm_add_epi32(p1,p6);
    p5 = _mm_add_epi32(p5,p6);

    p2 = _mm_sra_epi32(p2,p7);
    p3 = _mm_sra_epi32(p3,p7);
    p1 = _mm_sra_epi32(p1,p7);
    p5 = _mm_sra_epi32(p5,p7);

    p2 = _mm_packs_epi32(p2,p3);
    p1 = _mm_packs_epi32(p1,p5);

    _mm_store_si128((__m128i*)&pDst[0], p2 );
    _mm_store_si128((__m128i*)&pDst[8], p1 );

    }else{
    }
*/
//    *pLastCoeff = -1;
//    for( i = sCoeff; i<16; i++ ){
//        if(pDst[i] != 0 && *pLastCoeff < pScanMatrix[i] ) *pLastCoeff = pScanMatrix[i];
//    }
#endif
#else
    transform4x4(pSrc, t);
    *pLastCoeff = -1;
    for( i = sCoeff; i<16; i++ ){
        sign[i] = t[i] < 0 ? -1:1;
        pDst[i] = (Ipp16s)((sign[i]*t[i]*quantTbl[i]+scaleOffset)>>scale);
        if(pDst[i] != 0 && *pLastCoeff < pScanMatrix[i] ) *pLastCoeff = pScanMatrix[i];
    }
#endif
    if( *pLastCoeff < sCoeff ){
        *pNumLevels = *pLastCoeff = 0;
        return ippStsNoErr;
    }

    Ipp32s ctx_id,c;

    __ALIGN16 node  layer[2][8];
    node* layer_next = &layer[0][0];
    node* layer_cur = &layer[1][0];

    coeff_n coeffs[16*8*2];
    Ipp32s  last_coeff=0;

    //Init first layer
    layer_next[0].cost = 0;
    layer_next[0].coeff_id = 0;
    coeffs[0].coeff = 0;
    coeffs[0].prev_coeff = 0; //loop to itself
    last_coeff = 1;
    for(ctx_id=1;ctx_id<8; ctx_id++) layer_next[ctx_id].cost = MAX_COST;

    /* Copy CABAC contexts to 0 ctx_id (start ctx) */
    Ipp8u* cabac_sig, *cabac_last;
    //I4x4 contexts
    memcpy( layer_next[0].cabac_states_alevelm1, cbSt->absLevelM1, 10*sizeof(CABAC_CONTEXT));

//Precalculate cost of last and significant
    cabac_sig = cbSt->sig;
    cabac_last = cbSt->last;

    for (i=*pLastCoeff; i>=sCoeff; i--){
        Ipp32s pos = dec_scan[i];
        Ipp32s coeff = pDst[pos];
        Ipp32s state_sig = cabac_sig[i];
        Ipp32s state_last = cabac_last[i];

        if( coeff == 0 ){  //Take into account only sig_flag
            Ipp64u cost = (lambda*p_bits[state_sig]+128)>>8;
            for( ctx_id = 1; ctx_id<8; ctx_id++ ){
                register node* n = &layer_next[ctx_id];
                if( n->cost != MAX_COST ){
                    n->cost += cost;
                    coeffs[last_coeff].coeff = 0;
                    coeffs[last_coeff].prev_coeff = n->coeff_id;
                    n->coeff_id = last_coeff;
                    last_coeff++;
                }
            }
            continue;
        }

#ifdef TRANS4x4_OPT
        Ipp32u tcoeff = t[pos];
#else
        Ipp32u tcoeff = t[pos];
        if( sign[pos] < 0 ) tcoeff = -tcoeff;
#endif

        //Exchange layers
        node* layer_tmp = layer_cur;
        layer_cur = layer_next;
        layer_next = layer_tmp;
        //Set cost to MAX on next layer
        //for(ctx_id = 0; ctx_id<8; ctx_id++) layer_next[ctx_id].cost = MAX_COST;
        for(; layer_tmp<layer_next+8; layer_tmp++) layer_tmp->cost = MAX_COST;

        for( c = coeff; c >= coeff-1; c-- ){
            Ipp64s d = (tcoeff<<6) + 32; //*64
            if( Qp >= 24 )
                d = (d - (c*invQuantTbl[pos]<<shift_dq))>>6;
            else
                d = (d - ((c*invQuantTbl[pos]+offset_d)>>shift_dq))>>6;
            Ipp64u ssd = d*d*w[pos];

            if( c == 0 ){
              Ipp64u b=0;
              if( i != 15 ) b = ((lambda*p_bits[state_sig]+128)>>8);
              for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                Ipp64u cost = layer_cur[ctx_id].cost+ssd;
                if( ctx_id != 0) cost += b;
                if( cost < layer_next[ctx_id].cost ){
                    Ipp32s id = layer_next[ctx_id].coeff_id;
                    if( layer_next[ctx_id].cost == MAX_COST ){
                        id = last_coeff;
                        last_coeff++;
                    }
                    layer_next[ctx_id] = layer_cur[ctx_id]; //copy states
                    layer_next[ctx_id].cost = cost;
                    layer_next[ctx_id].coeff_id = id;
                    coeffs[id].coeff = c;
                    coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                }
              }
            }else if( c == 1){
                Ipp32u b[2]={0,0},bb=0;
                if( i != 15 ){
                    bb   =  p_bits[state_sig^64];
                    b[0] =  p_bits[state_last];
                    b[1] =  p_bits[state_last^64];
                }
                for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                    if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                    Ipp32s next_ctx_id = ctx_id_trans0[ctx_id];
                    Ipp32s ctx = ctx_neq1p1[ctx_id];
                    Ipp32u bits = bb+b[ctx_id==0];
                    Ipp8u ctx_val = *(layer_cur[ctx_id].cabac_states_alevelm1+ctx);
                    bits += 256 + p_bits[ctx_val]; //+1 bit for sign
                    ctx_val = transTbl[0][ctx_val];
                    Ipp64u cost = layer_cur[ctx_id].cost + ssd + ((lambda*bits+128)>>8);
                    if( cost < layer_next[next_ctx_id].cost ){
                       Ipp32s id = layer_next[next_ctx_id].coeff_id;
                       if( layer_next[next_ctx_id].cost == MAX_COST ){
                            id = last_coeff;
                            last_coeff++;
                        }
                        layer_next[next_ctx_id] = layer_cur[ctx_id]; //copy states
                        layer_next[next_ctx_id].cost = cost;
                        layer_next[next_ctx_id].coeff_id = id;
                        coeffs[id].coeff = c;
                        coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + ctx) = ctx_val;
                    }
                }
            }else{
                Ipp32u b[2]={0,0},bb=0;
                if( i != 15 ){
                    bb   =  p_bits[state_sig^64];
                    b[0] =  p_bits[state_last];
                    b[1] =  p_bits[state_last^64];
                }
                for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                    if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                    Ipp32s next_ctx_id = ctx_id_trans1[ctx_id];
                    Ipp8u st[4];
                    st[0] = ctx_neq1p1[ctx_id];
                    Ipp32u bits = bb+b[ctx_id==0];
                    st[1] = *(layer_cur[ctx_id].cabac_states_alevelm1+st[0]);
                    bits += 256 + p_bits[(st[1])^64]; //+1 bit for sign
                    st[1] = transTbl[1][st[1]];
                    st[2] = ctx_ngt1[ctx_id];
                    st[3] = *(layer_cur[ctx_id].cabac_states_alevelm1+st[2]);
                    Ipp32s code = c - 2;
                    if( code < 13 ){
                         bits +=  pref_bits[st[3]][code];
                         st[3] = pref_state[st[3]][code];
                    }else{
                         bits += pref_bits[st[3]][13];
                         st[3] = pref_state[st[3]][13];
                         if( code >= 65536-1+13 ){ bits += 32<<8; code >>= 16; }
                         if( code >= 256-1+13 ){ bits += 16<<8; code >>= 8; }
                         bits += bitcount_EG0[code];
                    }
                    Ipp64u cost = layer_cur[ctx_id].cost + ssd + ((lambda*bits+128)>>8);
                    if( cost < layer_next[next_ctx_id].cost ){
                       Ipp32s id = layer_next[next_ctx_id].coeff_id;
                       if( layer_next[next_ctx_id].cost == MAX_COST ){
                            id = last_coeff;
                            last_coeff++;
                        }
                        layer_next[next_ctx_id] = layer_cur[ctx_id]; //copy states
                        layer_next[next_ctx_id].cost = cost;
                        layer_next[next_ctx_id].coeff_id = id;
                        coeffs[id].coeff = c;
                        coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + st[0]) = st[1];
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + st[2]) = st[3];
                    }
                }
            }

        }
    }

    //Store new coeffs
    node *best=layer_next;
    node *cur;
    for( cur = layer_next+1; cur<layer_next+8; cur++ ){
        if( cur->cost < best->cost ) best=cur;
    }
    c = best->coeff_id;
    memcpy( cbSt->absLevelM1, best->cabac_states_alevelm1, 10*sizeof(CABAC_CONTEXT));
/*
    Ipp32s best_ctx = 0;
    Ipp64u best_cost = layer_next[0].cost;
    for( ctx_id = 1; ctx_id<8 ; ctx_id++ ){
        if( layer_next[ctx_id].cost < best_cost ){
            best_ctx = ctx_id;
            best_cost = layer_next[ctx_id].cost;
        }
    }
    c = layer_next[best_ctx].coeff_id;
    memcpy( cbSt->absLevelM1, layer_next[best_ctx].cabac_states_alevelm1, 10*sizeof(CABAC_CONTEXT));
*/
    Ipp32s last = *pLastCoeff;
    *pNumLevels = *pLastCoeff = 0;
    for (i=sCoeff; i<=last; i++){
        Ipp16s cf = coeffs[c].coeff;
        Ipp32s pos = dec_scan[i];
        if( cf != 0 ){
#ifndef TRANS4x4_OPT
            pDst[pos] = cf * sign[pos];
#else
            pDst[pos] = cf;
#endif
            (*pNumLevels)++;
            *pLastCoeff = i;
        }else{
            pDst[pos] = 0;
        }
        c = coeffs[c].prev_coeff;
    }

#ifdef TRANS4x4_OPT
//restore sign
    p0 = _mm_load_si128((__m128i*)&pDst[0]);
    p1 = _mm_load_si128((__m128i*)&pDst[8]);
    p2 = _mm_load_si128((__m128i*)&sign[0]);
    p3 = _mm_load_si128((__m128i*)&sign[8]);
    p0 = _mm_adds_epi16(p0,p2); //sign
    p0 = _mm_xor_si128(p0,p2);
    p1 = _mm_adds_epi16(p1,p3); //sign
    p1 = _mm_xor_si128(p1,p3);
    _mm_store_si128((__m128i*)&pDst[0], p0 );
    _mm_store_si128((__m128i*)&pDst[8], p1 );
#endif
#if 0
    for( i=sCoeff; i<=*pLastCoeff && i<15; i++ ){
            if( pDst[dec_scan[i]] != 0 ){
                cabac_sig[i] = transTbl[1][cabac_sig[i]];
                cabac_last[i] = transTbl[ i == *pLastCoeff ][cabac_last[i]];
            }else{
                cabac_sig[i] = transTbl[0][cabac_sig[i]];
            }
    }
#endif

    if (pDst[0] != 0){
        if( sCoeff ) (*pNumLevels)++;
        *pNumLevels = -*pNumLevels;
     }

    return (IppStatus)0;
}

Ipp16s quant8x8(Ipp16s coeff,Ipp32s pos, Ipp32s Qp6, Ipp32s intra, const Ipp16s* pScaleLevels)
{
    H264ENC_UNREFERENCED_PARAMETER(intra);
    Ipp16s sign;
    Ipp32s scale;
    Ipp32s scaleOffset;

    scale = 12+Qp6;
//    scaleOffset = intra ? (682<<(1 + Qp6)) : (342<<(1 + Qp6));
//    scaleOffset = (1<<(scale-1)); //intra ? (1<<scale)/3 : (1<<scale)/6;
    scaleOffset = 1<<scale; //intra ? (1<<scale)/3 : (1<<scale)/6;

    sign = (Ipp16s)(coeff < 0 ? -1:1);
    return (Ipp16s)(sign*((sign*coeff*pScaleLevels[pos]+scaleOffset)>>scale));
}

//pos - non zig-zag scan pos (normal pos)
Ipp16s dequant8x8(Ipp16s coeff,Ipp32s pos, Ipp32s Qp6, const Ipp16s* pInvLevelScale)
{
    if( Qp6 >= 6 ){
        Ipp32s shift = Qp6 - 6;
        return  (coeff*pInvLevelScale[pos])<<shift;
    }else{
        Ipp32s shift = 6 - Qp6;
        Ipp32s offset = 1<<(shift - 1);
        return (coeff*pInvLevelScale[pos]+offset)>>shift;
    }
}

const Ipp32s CorrMatrix8x8[64] = {
       65536, 73984, 40960, 73984, 65536, 73984, 40960, 73984,
       73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521,
       40960, 46240, 25600, 46240, 40960, 46240, 25600, 46240,
       73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521,
       65536, 73984, 40960, 73984, 65536, 73984, 40960, 73984,
       73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521,
       40960, 46240, 25600, 46240, 40960, 46240, 25600, 46240,
       73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521
};

const Ipp32s w8x8[64] = {
    400, 354, 640, 354, 400, 354, 640, 354,
    354, 314, 567, 314, 354, 314, 567, 314,
    640, 567,1024, 567, 640, 567,1024, 567,
    354, 314, 567, 314, 354, 314, 567, 314,
    400, 354, 640, 354, 400, 354, 640, 354,
    354, 314, 567, 314, 354, 314, 567, 314,
    640, 567,1024, 567, 640, 567,1024, 567,
    354, 314, 567, 314, 354, 314, 567, 314
};

const Ipp32u lambda_t8x8[2][52] = {
{
   19,    24,    30,    38,    48,    61,    76,    96,
  121,   153,   193,   243,   306,   386,   486,   612,
  771,   971,  1224,  1542,  1943,  2448,  3084,  3982,
 5068,  6154,  7964,  9774, 12308, 15566, 19548, 24978,
31132, 39458, 49594, 62626, 78916, 99188, 124890, 157470,
198376, 250142, 314940, 397114, 500284, 630242, 793866, 1000206,
1260484, 1588094, 2000774, 2520606,
},{
   14,    17,    21,    27,    34,    43,    54,    68,
   86,   108,   136,   171,   216,   271,   342,   431,
  543,   684,   862,  1086,  1369,  1724,  2172,  2805,
 3570,  4335,  5610,  6885,  8670, 10965, 13770, 17595,
21930, 27795, 34935, 44115, 55590, 69870, 87975, 110925,
139740, 176205, 221850, 279735, 352410, 443955, 559215, 704565,
887910, 1118685, 1409385, 1775565,

/*   15,    18,    23,    29,    37,    46,    58,    73,
   92,   116,   146,   184,   232,   292,   368,   464,
  585,   737,   928,  1170,  1474,  1857,  2339,  3021,
 3845,  4668,  6042,  7415,  9337, 11808, 14829, 18948,
23617, 29933, 37622, 47508, 59866, 75245, 94742, 119458,
150489, 189759, 238915, 301253, 379518, 478105, 602232, 758762,
956211, 1204738, 1517799, 1912147,
*/
/*
   19,    24,    30,    38,    48,    60,    75,    95,
  120,   151,   190,   239,   302,   380,   479,   603,
  760,   958,  1207,  1521,  1916,  2414,  3041,  3927,
 4998,  6069,  7854,  9639, 12138, 15351, 19278, 24633,
30702, 38913, 48909, 61761, 77826, 97818, 123165, 155295,
195636, 246687, 310590, 391629, 493374, 621537, 782901, 986391,
1243074, 1566159, 1973139, 2485791,
*/
}
};

static __ALIGN16 const Ipp32s InvScale8x8[6][64] = {
{ 20971520, 22491136, 16384000, 22491136, 20971520, 22491136, 16384000, 22491136, 22491136, 24054048, 17756160, 24054048, 22491136, 24054048, 17756160, 24054048, 16384000, 17756160, 13107200, 17756160, 16384000, 17756160, 13107200, 17756160, 22491136, 24054048, 17756160, 24054048, 22491136, 24054048, 17756160, 24054048, 20971520, 22491136, 16384000, 22491136, 20971520, 22491136, 16384000, 22491136, 22491136, 24054048, 17756160, 24054048, 22491136, 24054048, 17756160, 24054048, 16384000, 17756160, 13107200, 17756160, 16384000, 17756160, 13107200, 17756160, 22491136, 24054048, 17756160, 24054048, 22491136, 24054048, 17756160, 24054048, },
{ 23068672, 24858624, 18350080, 24858624, 23068672, 24858624, 18350080, 24858624, 24858624, 25390384, 19235840, 25390384, 24858624, 25390384, 19235840, 25390384, 18350080, 19235840, 14336000, 19235840, 18350080, 19235840, 14336000, 19235840, 24858624, 25390384, 19235840, 25390384, 24858624, 25390384, 19235840, 25390384, 23068672, 24858624, 18350080, 24858624, 23068672, 24858624, 18350080, 24858624, 24858624, 25390384, 19235840, 25390384, 24858624, 25390384, 19235840, 25390384, 18350080, 19235840, 14336000, 19235840, 18350080, 19235840, 14336000, 19235840, 24858624, 25390384, 19235840, 25390384, 24858624, 25390384, 19235840, 25390384, },
{ 27262976, 28409856, 21626880, 28409856, 27262976, 28409856, 21626880, 28409856, 28409856, 30735728, 22935040, 30735728, 28409856, 30735728, 22935040, 30735728, 21626880, 22935040, 17203200, 22935040, 21626880, 22935040, 17203200, 22935040, 28409856, 30735728, 22935040, 30735728, 28409856, 30735728, 22935040, 30735728, 27262976, 28409856, 21626880, 28409856, 27262976, 28409856, 21626880, 28409856, 28409856, 30735728, 22935040, 30735728, 28409856, 30735728, 22935040, 30735728, 21626880, 22935040, 17203200, 22935040, 21626880, 22935040, 17203200, 22935040, 28409856, 30735728, 22935040, 30735728, 28409856, 30735728, 22935040, 30735728, },
{ 29360128, 30777344, 22937600, 30777344, 29360128, 30777344, 22937600, 30777344, 30777344, 33408400, 24414720, 33408400, 30777344, 33408400, 24414720, 33408400, 22937600, 24414720, 18432000, 24414720, 22937600, 24414720, 18432000, 24414720, 30777344, 33408400, 24414720, 33408400, 30777344, 33408400, 24414720, 33408400, 29360128, 30777344, 22937600, 30777344, 29360128, 30777344, 22937600, 30777344, 30777344, 33408400, 24414720, 33408400, 30777344, 33408400, 24414720, 33408400, 22937600, 24414720, 18432000, 24414720, 22937600, 24414720, 18432000, 24414720, 30777344, 33408400, 24414720, 33408400, 30777344, 33408400, 24414720, 33408400, },
{ 33554432, 35512320, 26214400, 35512320, 33554432, 35512320, 26214400, 35512320, 35512320, 37417408, 28113920, 37417408, 35512320, 37417408, 28113920, 37417408, 26214400, 28113920, 20889600, 28113920, 26214400, 28113920, 20889600, 28113920, 35512320, 37417408, 28113920, 37417408, 35512320, 37417408, 28113920, 37417408, 33554432, 35512320, 26214400, 35512320, 33554432, 35512320, 26214400, 35512320, 35512320, 37417408, 28113920, 37417408, 35512320, 37417408, 28113920, 37417408, 26214400, 28113920, 20889600, 28113920, 26214400, 28113920, 20889600, 28113920, 35512320, 37417408, 28113920, 37417408, 35512320, 37417408, 28113920, 37417408, },
{ 37748736, 40247296, 30146560, 40247296, 37748736, 40247296, 30146560, 40247296, 40247296, 42762752, 31813120, 42762752, 40247296, 42762752, 31813120, 42762752, 30146560, 31813120, 23756800, 31813120, 30146560, 31813120, 23756800, 31813120, 40247296, 42762752, 31813120, 42762752, 40247296, 42762752, 31813120, 42762752, 37748736, 40247296, 30146560, 40247296, 37748736, 40247296, 30146560, 40247296, 40247296, 42762752, 31813120, 42762752, 40247296, 42762752, 31813120, 42762752, 30146560, 31813120, 23756800, 31813120, 30146560, 31813120, 23756800, 31813120, 40247296, 42762752, 31813120, 42762752, 40247296, 42762752, 31813120, 42762752, },
};

static __ALIGN16 const Ipp32s offset_dequant8x8[6][64]={
{ 2097152, 2367488, 1310720, 2367488, 2097152, 2367488, 1310720, 2367488, 2367488, 2672672, 1479680, 2672672, 2367488, 2672672, 1479680, 2672672, 1310720, 1479680, 819200, 1479680, 1310720, 1479680, 819200, 1479680, 2367488, 2672672, 1479680, 2672672, 2367488, 2672672, 1479680, 2672672, 2097152, 2367488, 1310720, 2367488, 2097152, 2367488, 1310720, 2367488, 2367488, 2672672, 1479680, 2672672, 2367488, 2672672, 1479680, 2672672, 1310720, 1479680, 819200, 1479680, 1310720, 1479680, 819200, 1479680, 2367488, 2672672, 1479680, 2672672, 2367488, 2672672, 1479680, 2672672, },
{ 1048576, 1183744, 655360, 1183744, 1048576, 1183744, 655360, 1183744, 1183744, 1336336, 739840, 1336336, 1183744, 1336336, 739840, 1336336, 655360, 739840, 409600, 739840, 655360, 739840, 409600, 739840, 1183744, 1336336, 739840, 1336336, 1183744, 1336336, 739840, 1336336, 1048576, 1183744, 655360, 1183744, 1048576, 1183744, 655360, 1183744, 1183744, 1336336, 739840, 1336336, 1183744, 1336336, 739840, 1336336, 655360, 739840, 409600, 739840, 655360, 739840, 409600, 739840, 1183744, 1336336, 739840, 1336336, 1183744, 1336336, 739840, 1336336, },
{ 524288, 591872, 327680, 591872, 524288, 591872, 327680, 591872, 591872, 668168, 369920, 668168, 591872, 668168, 369920, 668168, 327680, 369920, 204800, 369920, 327680, 369920, 204800, 369920, 591872, 668168, 369920, 668168, 591872, 668168, 369920, 668168, 524288, 591872, 327680, 591872, 524288, 591872, 327680, 591872, 591872, 668168, 369920, 668168, 591872, 668168, 369920, 668168, 327680, 369920, 204800, 369920, 327680, 369920, 204800, 369920, 591872, 668168, 369920, 668168, 591872, 668168, 369920, 668168, },
{ 262144, 295936, 163840, 295936, 262144, 295936, 163840, 295936, 295936, 334084, 184960, 334084, 295936, 334084, 184960, 334084, 163840, 184960, 102400, 184960, 163840, 184960, 102400, 184960, 295936, 334084, 184960, 334084, 295936, 334084, 184960, 334084, 262144, 295936, 163840, 295936, 262144, 295936, 163840, 295936, 295936, 334084, 184960, 334084, 295936, 334084, 184960, 334084, 163840, 184960, 102400, 184960, 163840, 184960, 102400, 184960, 295936, 334084, 184960, 334084, 295936, 334084, 184960, 334084, },
{ 131072, 147968, 81920, 147968, 131072, 147968, 81920, 147968, 147968, 167042, 92480, 167042, 147968, 167042, 92480, 167042, 81920, 92480, 51200, 92480, 81920, 92480, 51200, 92480, 147968, 167042, 92480, 167042, 147968, 167042, 92480, 167042, 131072, 147968, 81920, 147968, 131072, 147968, 81920, 147968, 147968, 167042, 92480, 167042, 147968, 167042, 92480, 167042, 81920, 92480, 51200, 92480, 81920, 92480, 51200, 92480, 147968, 167042, 92480, 167042, 147968, 167042, 92480, 167042, },
{ 65536, 73984, 40960, 73984, 65536, 73984, 40960, 73984, 73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521, 40960, 46240, 25600, 46240, 40960, 46240, 25600, 46240, 73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521, 65536, 73984, 40960, 73984, 65536, 73984, 40960, 73984, 73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521, 40960, 46240, 25600, 46240, 40960, 46240, 25600, 46240, 73984, 83521, 46240, 83521, 73984, 83521, 46240, 83521, },
};

void QuantOptLuma8x8_H264_16s_C1_8u16s(
    const Ipp16s* pSrc,
    Ipp16s* pDst,
    Ipp32s  Qp,
    Ipp32s  Intra,
    const Ipp16s* pScanMatrix,
    const Ipp16s* pScaleLevels,
    Ipp32s* pNumLevels,
    Ipp32s* pLastCoeff,
    const H264Slice_8u16s* curr_slice,
    CabacStates* cbSt,
    const Ipp16s* pInvLevelScale)
{
    H264ENC_UNREFERENCED_PARAMETER(pInvLevelScale);
    __ALIGN16 Ipp16s qcoeff[64];
    Ipp32s i;
    Ipp32s isField = curr_slice->m_is_cur_mb_field;

    const Ipp32s Qp6 = h264_qp6[Qp];
//    Ipp64u lambda = 362*curr_slice->m_cur_mb.lambda;
    Ipp32u lambda = lambda_t8x8[Intra][Qp];
//    if(Intra) lambda = lambda_t8x8[0][Qp]/3;
//    else lambda = lambda_t8x8[Intra][Qp];
    const Ipp32s* dec_scan = dec_single_scan_8x8[isField];
    const Ipp32s scale = 12+Qp6;
    const Ipp32s scaleOffset = 1<<scale;
    Ipp16s sign[64];
    Ipp32s shift_dq;
    Ipp32s offset_d = 0;
    const Ipp32s* invQuantTbl = InvScale8x8[ h264_qp_rem[Qp] ];

    if( Qp6 >= 6 ){
        shift_dq = Qp6 - 6;
    }else{
        shift_dq = 6 - Qp6;
        offset_d = 1<<(shift_dq - 1);
     }

    *pLastCoeff = -1;
    //Save original data
    for( i = 0; i<64; i++ ){
        sign[i] = pSrc[i] < 0 ? -1:1;
        qcoeff[i] = (Ipp16s)((sign[i]*pSrc[i]*pScaleLevels[i]+scaleOffset)>>scale);
        if(qcoeff[i] != 0 && (*pLastCoeff < pScanMatrix[i]) ) *pLastCoeff = pScanMatrix[i];
    }

    if( *pLastCoeff < 0 ){
        *pNumLevels = *pLastCoeff = 0;
        memset( pDst, 0, 64*sizeof(Ipp16s) );
        return ;
    }

    Ipp32s ctx_id,c;

    node8x8 layer[2][8];
    node8x8* layer_next = &layer[0][0];
    node8x8* layer_cur = &layer[1][0];

    coeff_n coeffs[64*8*2];
    Ipp32s  last_coeff=0;

    //Init first layer
    layer_next[0].cost = 0;
    layer_next[0].coeff_id = 0;
    coeffs[0].coeff = 0;
    coeffs[0].prev_coeff = 0; //loop to itself
    last_coeff = 1;
    for(ctx_id=1;ctx_id<8; ctx_id++) layer_next[ctx_id].cost = MAX_COST;

    /* Copy CABAC contexts to 0 ctx_id (start ctx) */
    //I8x8 contexts
    memcpy( layer_next[0].cabac_states_alevelm1, cbSt->absLevelM1, 10*sizeof(CABAC_CONTEXT));
    //Copy contexts for significant and last

    Ipp8u cabac_sig[64],cabac_last[64];
    for( i = 0; i<=*pLastCoeff; i++ ){
        cabac_sig[i] = cbSt->sig[Table_9_34[isField][i]];
        cabac_last[i] = cbSt->last[Table_9_34[2][i]];
    }

    for (i=*pLastCoeff; i>=0 ; i--){
        Ipp32s pos = dec_scan[i];
        Ipp32s coeff = qcoeff[pos];
        Ipp32s state_sig = cabac_sig[i];
        Ipp32s state_last = cabac_last[i];

        if( coeff == 0 ){  //Take into account only sig_flag
            Ipp64u cost = (lambda*p_bits[state_sig]+128)>>8;
            for( ctx_id = 1; ctx_id<8; ctx_id++ ){
                register node8x8* n = &layer_next[ctx_id];
                if( n->cost != MAX_COST ){
                    n->cost += cost;
                    coeffs[last_coeff].coeff = 0;
                    coeffs[last_coeff].prev_coeff = n->coeff_id;
                    n->coeff_id = last_coeff;
                    last_coeff++;
                }
            }
            continue;
        }

        Ipp32u tcoeff;
        if( sign[pos] < 0 )
            tcoeff = -pSrc[pos];
        else
            tcoeff = pSrc[pos];

        //Exchange layers
        node8x8* layer_tmp = layer_cur;
        layer_cur = layer_next;
        layer_next = layer_tmp;
        //Set cost to MAX on next layer
        for(; layer_tmp<layer_next+8; layer_tmp++) layer_tmp->cost = MAX_COST;

        for( c = coeff; c >= coeff-1; c-- ){
            Ipp64s d = (tcoeff<<6) + 32;
            if( Qp6 >= 6 )
                d =  ( d - ((((c*invQuantTbl[pos])<<shift_dq) + 512)>>10) )>>6;
            else
                d =  ( d - ((((c*invQuantTbl[pos]+offset_d)>>shift_dq) + 512)>>10) )>>6;

            Ipp64u ssd = (d*d*w8x8[pos] + 32)>>6;

            if( c == 0 ){
              Ipp64u b=0;
              if( i != 63 ) b = ((lambda*p_bits[state_sig]+128)>>8);
              for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                Ipp64u cost = layer_cur[ctx_id].cost+ssd;
                if( ctx_id != 0) cost += b;
                if( cost < layer_next[ctx_id].cost ){
                    Ipp32s id = layer_next[ctx_id].coeff_id;
                    if( layer_next[ctx_id].cost == MAX_COST ){
                        id = last_coeff;
                        last_coeff++;
                    }
                    layer_next[ctx_id] = layer_cur[ctx_id]; //copy states
                    layer_next[ctx_id].cost = cost;
                    layer_next[ctx_id].coeff_id = id;
                    coeffs[id].coeff = c;
                    coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                }
              }
            }else if( c == 1){
                Ipp32u b[2]={0,0},bb=0;
                if( i != 63 ){
                    bb   =  p_bits[state_sig^64];
                    b[0] =  p_bits[state_last];
                    b[1] =  p_bits[state_last^64];
                }
                for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                    if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                    Ipp32s next_ctx_id = ctx_id_trans0[ctx_id];
                    Ipp32u bits = bb+b[ctx_id==0];

                    Ipp32s ctx = ctx_neq1p1[ctx_id];
                    Ipp8u ctx_val = *(layer_cur[ctx_id].cabac_states_alevelm1+ctx);
                    bits += 256 + p_bits[ctx_val]; //+1 bit for sign
                    ctx_val = transTbl[0][ctx_val];
                    Ipp64u cost = layer_cur[ctx_id].cost + ssd + ((lambda*bits+128)>>8);
                    if( cost < layer_next[next_ctx_id].cost ){
                       Ipp32s id = layer_next[next_ctx_id].coeff_id;
                       if( layer_next[next_ctx_id].cost == MAX_COST ){
                            id = last_coeff;
                            last_coeff++;
                        }
                        layer_next[next_ctx_id] = layer_cur[ctx_id]; //copy states
                        layer_next[next_ctx_id].cost = cost;
                        layer_next[next_ctx_id].coeff_id = id;
                        coeffs[id].coeff = c;
                        coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + ctx) = ctx_val;
                    }
                }
            }else{
                Ipp32u b[2]={0,0},bb=0;
                if( i != 63 ){
                    bb   =  p_bits[state_sig^64];
                    b[0] =  p_bits[state_last];
                    b[1] =  p_bits[state_last^64];
                }
                for( ctx_id = 0; ctx_id<8; ctx_id++ ){
                    if( layer_cur[ctx_id].cost == MAX_COST ) continue;
                    Ipp32s next_ctx_id = ctx_id_trans1[ctx_id];
                    Ipp32u bits = bb+b[ctx_id==0];
                    Ipp8u st[4];

                    st[0] = ctx_neq1p1[ctx_id];
                    st[1] = *(layer_cur[ctx_id].cabac_states_alevelm1+st[0]);
                    bits += 256 + p_bits[(st[1])^64]; //+1 bit for sign
                    st[1] = transTbl[1][st[1]];
                    st[2] = ctx_ngt1[ctx_id];
                    st[3] = *(layer_cur[ctx_id].cabac_states_alevelm1+st[2]);
                    Ipp32s code = c - 2;
                    if( code < 13 ){
                         bits +=  pref_bits[st[3]][code];
                         st[3] = pref_state[st[3]][code];
                    }else{
                         bits += pref_bits[st[3]][13];
                         st[3] = pref_state[st[3]][13];
                         if( code >= 65536-1+13 ){ bits += 32<<8; code >>= 16; }
                         if( code >= 256-1+13 ){ bits += 16<<8; code >>= 8; }
                         bits += bitcount_EG0[code];
                    }
                    Ipp64u cost = layer_cur[ctx_id].cost + ssd + ((lambda*bits+128)>>8);
                    if( cost < layer_next[next_ctx_id].cost ){
                       Ipp32s id = layer_next[next_ctx_id].coeff_id;
                       if( layer_next[next_ctx_id].cost == MAX_COST ){
                            id = last_coeff;
                            last_coeff++;
                        }
                        layer_next[next_ctx_id] = layer_cur[ctx_id]; //copy states
                        layer_next[next_ctx_id].cost = cost;
                        layer_next[next_ctx_id].coeff_id = id;
                        coeffs[id].coeff = c;
                        coeffs[id].prev_coeff = layer_cur[ctx_id].coeff_id;
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + st[0]) = st[1];
                        *(layer_next[next_ctx_id].cabac_states_alevelm1 + st[2]) = st[3];
                    }
                }
            }

        }
    }

    //Store new coeffs
    node8x8 *best=layer_next;
    node8x8 *cur;
    for( cur = layer_next+1; cur<layer_next+8; cur++ ){
        if( cur->cost < best->cost ) best=cur;
    }
    c = best->coeff_id;

#if 0
    //cbSt->codRange = layer_next[best_ctx].codRange;
    memcpy( cbSt->absLevelM1, layer_next[best_ctx].cabac_states_alevelm1, 10*sizeof(CABAC_CONTEXT));
    memcpy( cbSt->sig, cabac_sig, 15*sizeof(CABAC_CONTEXT));
    memcpy( cbSt->last, cabac_last, 15*sizeof(CABAC_CONTEXT));
#endif

    Ipp32s last = *pLastCoeff;
    *pNumLevels = *pLastCoeff = 0;
    for (i=0; i<=last; i++){
        Ipp16s cf = coeffs[c].coeff;
        Ipp32s pos = dec_scan[i];
        if( cf != 0 ){
            pDst[pos] = cf * sign[pos];
//            pDst[pos] = cf;
            (*pNumLevels)++;
            *pLastCoeff = i;
        }else{
            pDst[pos] = 0;
        }
        c = coeffs[c].prev_coeff;
    }

    if (pDst[0] != 0)
        *pNumLevels = -*pNumLevels;
}

namespace UMC_H264_ENCODER
{

#define PRINT 0

static Ipp32s chromaPredInc[3][16] = {
     { 4, 60, 4,   0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, //pitch 16
     { 4, 60, 4,  60,  4, 60, 4,  0,  0,  0,  0,  0,  0,  0,  0,  0  }, //pitch 16
     { 4, 60, 4, -60,  4, 60, 4, 52,  4, 60,  4,-60,  4, 60,  4,  0  } //pitch 16
};

static Ipp32s chromaDCOffset[3][16] = {
    { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0 , 0,  0,  0,  0,  0 },
    { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15}
};

inline void ippiCalcNonZero(const Ipp16s *coeffs, Ipp32s  m, Ipp32s *numCoeffs)
{
    Ipp32s n = 0;
    for (Ipp32s i = 0; i < m; i ++) {
        if (coeffs[i] != 0)
            n ++;
    }
    *numCoeffs = (coeffs[0] != 0)? -n: n;
}

inline void ippiCalcNonZero(const Ipp32s *coeffs, Ipp32s  m, Ipp32s *numCoeffs)
{
    Ipp32s n = 0;
    for (Ipp32s i = 0; i < m; i ++) {
        if (coeffs[i] != 0)
            n ++;
    }
    *numCoeffs = (coeffs[0] != 0)? -n: n;
}

inline void ippiCountCoeffs(const Ipp32s *coeffs, Ipp32s *numCoeffs, const Ipp16s *enc_scan, Ipp32s *lastCoeff, Ipp32s  m)
{
    // m can take the following values: 16, 64.
    Ipp32s l = 0;
    Ipp32s n = 0;
    for(Ipp32s i = 0; i < m; i++) {
        if (coeffs[i] != 0) {
            n ++;
            if (l < enc_scan[i])
                l = enc_scan[i];
        }
    }

    *numCoeffs = (coeffs[0] != 0)? -n: n;
    *lastCoeff = l;
}

inline void ippiCountCoeffs(const Ipp16s *coeffs, Ipp32s *numCoeffs, const Ipp16s *enc_scan, Ipp32s *lastCoeff, Ipp32s  m)
{
    // m can take the following values: 16, 64.
    Ipp32s l = 0;
    Ipp32s n = 0;
    for(Ipp32s i = 0; i < m; i++) {
        if (coeffs[i] != 0) {
            n ++;
            if (l < enc_scan[i])
                l = enc_scan[i];
        }
    }
    *numCoeffs = (coeffs[0] != 0)? -n: n;
    *lastCoeff = l;
}

#define PIXBITS 8
#include "umc_h264_ermb_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_ermb_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER


