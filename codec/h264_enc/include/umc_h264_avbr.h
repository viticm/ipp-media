/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//      Copyright (c) 2006-2008 Intel Corporation. All Rights Reserved.
//
//  Purpose
//    AVBR frame adaptive bitrate control
*/

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include "ippdefs.h"
#include "umc_defs.h"
#include "umc_structures.h"
#include "umc_h264_defs.h"


namespace UMC_H264_ENCODER {


struct H264_AVBR
{
//protected :
    Ipp32s  mBitRate, mBitsDesiredFrame;
    Ipp64s  mBitsEncodedTotal, mBitsDesiredTotal;
    Ipp8u   *mRCqb;
    Ipp32s  mRCqh, mRCqs, mRCbf, mRClen;
    bool    mIsInit;
    Ipp32s  mQuantI, mQuantP, mQuantB, mQuantMax, mQuantPrev, mQuantOffset;
    Ipp32s  mRCfap, mRCqap, mRCbap, mRCq;
    Ipp64f  mRCqa, mRCfa;

#ifdef ALT_RC
    bool    mIsGopSet;
    Ipp64s bframeRate, MaxBuf, TotalFrameNum, Texture, BitsGopRemaining, TBL, LastPNum, CurSumPQP, BufFullLastGOP, SumPQP, BitsLastGopRemaining, CurPNum, GOPLen, PLeft, L, BLeft, BNum, PredTexture, QP1, QP2, RealGOPLen, RealPLeft, RealBLeft;
    Ipp64f mFrameRate, X1, X2, MADC1, MADC2, BufFull, Mx, My, Mxx, Mxy, par1, par2, par3, par4, par5, Const, WeightP, WeightB, g, b, f1, f2, PredMAD, PrevMAD, CurMAD;
    Ipp64f BMADC1, BMADC2, BMx, BMy, BMxx, BMxy, BPrevMAD, MAD1, MAD2;
    Ipp64f AvgMAD, AvgQP, MADRatio;
    Ipp64s *QPs;
    EnumPicCodType* GopTypes;

    Ipp32s BufferLoBound, BufferHiBound, BufferSize; //in frames
    Ipp64s BufferFull; //in bits
    Ipp64f HiZ; // used for buffer control
    Ipp32s iF;
    char fname[1024];
    FILE* filelog;
#endif
};

Status H264_AVBR_Create(
    H264_AVBR* state);

void H264_AVBR_Destroy(
    H264_AVBR* state);

UMC::Status H264_AVBR_Init(
    H264_AVBR* state,
    Ipp32s qas,
    Ipp32s fas,
    Ipp32s bas,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha);

void H264_AVBR_Close(
    H264_AVBR* state);

void H264_AVBR_PostFrame(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s bEncoded);

Ipp32s H264_AVBR_GetQP(
    H264_AVBR* state,
    EnumPicCodType frameType);

void H264_AVBR_SetQP(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s qp);

Ipp32s H264_AVBR_GetInitQP(
    H264_AVBR* state,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha);

#ifdef ALT_RC
void H264_AVBR_PostFrame_AltRC(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s bEncoded,
    Ipp64f mMAD,
    Ipp64s textureBits);

UMC::Status H264_AVBR_Init_AltRC(
    H264_AVBR* state,
    Ipp32s MaxFrame,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha,
    Ipp64s gopLen,
    EnumPicCodType* gopTypes,
    Ipp32s BFrameRate);

void H264_AVBR_PreFrame(
    H264_AVBR* state,
    EnumPicCodType FrameType);

void H264_AVBR_SetGopLen(
    H264_AVBR* state,
    Ipp32s len);
#endif

#define h264_Clip(a, l, r) if (a < (l)) a = l; else if (a > (r)) a = r
#define h264_ClipL(a, l) if (a < (l)) a = l
#define h264_ClipR(a, r) if (a > (r)) a = r
#define h264_Abs(a) ((a) >= 0 ? (a) : -(a))
}

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
