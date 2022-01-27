/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH263 (encode Pics)
//  Contents:
//                  EncodeFrame
//
//  References:
//    Fast ME algorithm
//        IEEE Transactions on image processing, vol. 9, N. 2, Feb 2000
//        "A new Diamond Search Algorithm for Fast Block-Matching Motion Estimation"
//        Shan Zhu and Kai-Kuang Ma
//    Fast half-pel algorithm
//        V. Bhaskaran, K. Konstantinides
//        "Image And Video Compression Standarts"
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)
#include <stdlib.h>
#ifdef PRINT_INFO
#include <math.h>
#endif
#include "h263_enc.hpp"

#pragma warning(disable : 981)      // operands are evaluated in unspecified order
#pragma warning(disable : 279)      // controlling expression is constant

//--------------------- defines to control ME process -------------------------

//#define ME_FULLSEARCH_RECT        // using rectangle instead of involute search
//#define ME_USE_THRESHOLD          // using threshold in ME (faster)

const Ipp32s SAD_FAVOR_ZERO   = 129 /4;
const Ipp32s SAD_FAVOR_DIRECT = 129 /2;
const Ipp32s SAD_FAVOR_INTER  = 512 /2;   // 500 H.263 App III
const Ipp32s SAD_FAVOR_16x16  = 200; //129;   // 200 H.263 App III
const Ipp32s SAD_FAVOR_PRED   = - SAD_FAVOR_ZERO / 2;
const Ipp32s rangeME_8x8      =   1;   // radius for search 8x8 MVs from 16x16 approximation
const Ipp32s SAD_NOTCODED_THR_LUMA = 10;
const Ipp32s SAD_NOTCODED_THR_CHROMA = 20;
const Ipp32s DEV_FAVOR_INTRA = 100;
const Ipp32s MAX_SAD = 16 * 16 * 256;

//---------------------- aux defines and functions ----------------------------

#define H263_MV_OFF_HP(dx, dy, step) \
    (((dx) >> 1) + (step) * ((dy) >> 1))

#define H263_MV_ACC_HP(dx, dy) \
    ((((dy) & 1) << 1) + ((dx) & 1))

#define H263_MV_OFF_QP(dx, dy, step) \
    (((dx) >> 2) + (step) * ((dy) >> 2))

#define H263_MV_ACC_QP(dx, dy) \
    ((((dy) & 3) << 2) + ((dx) & 3))

#define h263e_Copy8x4HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x4HP_8u_C1R(pSrc + H263_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy8x8HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x8HP_8u_C1R(pSrc + H263_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy16x8HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x8HP_8u_C1R(pSrc + H263_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy16x16HP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x16HP_8u_C1R(pSrc + H263_MV_OFF_HP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_HP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy8x8QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy8x8QP_H263_8u_C1R(pSrc + H263_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy16x8QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x8QP_H263_8u_C1R(pSrc + H263_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define h263e_Copy16x16QP_8u(pSrc, srcStep, pDst, dstStep, mv, rc) \
    ippiCopy16x16QP_H263_8u_C1R(pSrc + H263_MV_OFF_QP((mv)->dx, (mv)->dy, srcStep), srcStep, pDst, dstStep, H263_MV_ACC_QP((mv)->dx, (mv)->dy), rc)

#define h263e_Add8x8_16s8u(pSrcDst, pResid, srcDstStep) \
    ippiAdd8x8_16s8u_C1IRS(pResid, 16, pSrcDst, srcDstStep)

inline void h263e_ComputeChromaMV(const IppMotionVector *mvLuma, IppMotionVector *mvChroma)
{
    mvChroma->dx = (Ipp16s)h263e_Div2Round(mvLuma->dx);
    mvChroma->dy = (Ipp16s)h263e_Div2Round(mvLuma->dy);
}


static void h263e_ComputeChroma4MV(const IppMotionVector mvLuma[4], IppMotionVector *mvChroma)
{
    Ipp32s  dx, dy, cdx, cdy, adx, ady;

    dx = mvLuma[0].dx + mvLuma[1].dx + mvLuma[2].dx + mvLuma[3].dx;
    dy = mvLuma[0].dy + mvLuma[1].dy + mvLuma[2].dy + mvLuma[3].dy;
    adx = h263e_Abs(dx);
    ady = h263e_Abs(dy);
    cdx = h263e_cCbCrMvRound16_[adx & 15] + (adx >> 4) * 2;
    cdy = h263e_cCbCrMvRound16_[ady & 15] + (ady >> 4) * 2;
    mvChroma->dx = (Ipp16s)((dx >= 0) ? cdx : -cdx);
    mvChroma->dy = (Ipp16s)((dy >= 0) ? cdy : -cdy);
}

static void h263e_Set8x8_8u(Ipp8u *p, Ipp32s step, Ipp8u level)
{
    Ipp32u  val;

    val = level + (level <<  8);
    val += val << 16;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val; p += step;
    ((Ipp32u*)p)[0] = val; ((Ipp32u*)p)[1] = val;
}

inline Ipp32s h263e_CalcMSE_16x16(Ipp8u *pSrc1, Ipp32s stepSrc1, Ipp8u *pSrc2, Ipp32s stepSrc2)
{
    Ipp32s  e;
    ippiSqrDiff16x16_8u32s(pSrc1, stepSrc1, pSrc2, stepSrc2, IPPVC_MC_APX_FF, &e);
    return e;
}

inline Ipp32s h263e_CalcMSE_8x8(Ipp8u *pSrc1, Ipp32s stepSrc1, Ipp8u *pSrc2, Ipp32s stepSrc2)
{
    Ipp32s  e;
    ippiSSD8x8_8u32s_C1R(pSrc1, stepSrc1, pSrc2, stepSrc2, &e, IPPVC_MC_APX_FF);
    return e;
}

inline Ipp16s h263e_Median(Ipp16s a, Ipp16s b, Ipp16s c)
{
    if (a > b) {
        Ipp16s  t = a; a = b; b = t;
    }
    return (b <= c) ? b : (c >= a) ? c : a;
}

inline void h263e_MV_CheckRange(IppMotionVector *mvD, Ipp32s umv)
{
  Ipp32s fMin, fMax;
  if (!umv) {
    fMin = -32;
    fMax = 31;
  } else {
    fMin = -63;
    fMax = 63;
  }

  if (mvD->dx < fMin)
    mvD->dx = (Ipp16s)(mvD->dx + 64);
  else if (mvD->dx > fMax)
    mvD->dx = (Ipp16s)(mvD->dx - 64);
  if (mvD->dy < fMin)
    mvD->dy = (Ipp16s)(mvD->dy + 64);
  else if (mvD->dy > fMax)
    mvD->dy = (Ipp16s)(mvD->dy - 64);
}

#define h263e_SetPatternInter(pattern, nzCount) \
    pattern = 0; \
    pattern |= (nzCount[0] > 0) ? 32 : 0; \
    pattern |= (nzCount[1] > 0) ? 16 : 0; \
    pattern |= (nzCount[2] > 0) ?  8 : 0; \
    pattern |= (nzCount[3] > 0) ?  4 : 0; \
    pattern |= (nzCount[4] > 0) ?  2 : 0; \
    pattern |= (nzCount[5] > 0) ?  1 : 0

#define h263e_SetPatternIntra(pattern, nzCount, thresh) \
{ \
  pattern = 0; \
  pattern |= (nzCount[0] > thresh) ? 32 : 0; \
  pattern |= (nzCount[1] > thresh) ? 16 : 0; \
  pattern |= (nzCount[2] > thresh) ?  8 : 0; \
  pattern |= (nzCount[3] > thresh) ?  4 : 0; \
  pattern |= (nzCount[4] > thresh) ?  2 : 0; \
  pattern |= (nzCount[5] > thresh) ?  1 : 0; \
}

inline void h263e_NonZeroCount(Ipp16s *coeff, Ipp32s *nzCount)
{
    Ipp32s i;
    Ipp32u c;
    for (i = 0; i < 6; i ++) {
        ippiCountZeros8x8_16s_C1(coeff+i*64, &c);
        nzCount[i] = 64 - c;
    }
}


Ipp32s ippVideoEncoderH263::EncodePic(Ipp32s picType, Ipp32s nt)
{
  Ipp32s sts = H263_STS_NOERR;

  mFrameSkipped = 0;

//  mQuantAvg = 0;
  mPSNR_Y = mPSNR_U = mPSNR_V = 0;
  mNumIntraMB = 0;
  mVideoPicture.picture_coding_type = picType;

  if (mSkipFrame) {
    mFrameSkipped = 1;
    sts = H263_STS_SKIPPED_FRAME;
  } else {
    if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_I) {
      mVideoPicture.pic_quant = mQuantIPic;
      EncodePicture_Header();
      EncodeIPic();
    } else if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_P) {
      mVideoPicture.pic_quant = mQuantPPic;
      EncodePicture_Header();
      EncodePPic();
    }
//  else
//    EncodeBPic();
  }

  if (mNumIntraMB > mSceneChangeThreshold)
    return H263_STS_NOERR;

  mBitsEncodedFrame = cBS.GetFullness() << 3;
  mBitsEncodedTotal += mBitsEncodedFrame;


//  mQuantAvg = mVideoPicture.pic_quant;

  if (mRateControl)
    PostFrameRC();
/*
  if (mVideoPicture.plusptype && mVideoPicture.picture_coding_type != H263e_PIC_TYPE_B) {
    if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_I) {
      mVideoPicture.pic_rounding_type = 0; // reset rounding_type for next P-mVideoPicture
    } else
      mVideoPicture.pic_rounding_type ^= 1; // switch rounding_type
  }
*/
  return sts;
}

Ipp32s ippVideoEncoderH263::EncodeFrame(Ipp32s noMoreData)
{
  Ipp32s isIPic, isPPic, isBPic, nt, sts;
  Ipp32s minBufSize;
  Ipp32s bufTooSmall;

  if (!mIsInit)
    return H263_STS_ERR_NOTINIT;

  /* ??? !!!
  needs to be modified when adding custom format support:
  the buffer allocated in the app for small pictures can be smaller than mBPPmax which is >= 8 kB
  according to the Standard
  */
  if (mRateControl)
    minBufSize = IPP_MIN(mBitsDesiredFrame * 2, mBPPmax);
  else
    minBufSize = mBPPmax;

  minBufSize >>= 3; // bits -> bytes
  bufTooSmall = (cBS.mBuffSize - cBS.GetFullness() < minBufSize);

  // without B-frames
  if (mBPicdist == 0) {
    if (noMoreData)
      return H263_STS_NODATA;

    if (bufTooSmall)
      return H263_STS_ERR_BUFOVER;

    //isIPic = !(mFrameCount % mIPicdist);
    isIPic = (mFrameCount - mLastIPic >= mIPicdist);
    if (isIPic) {
      sts = EncodePic(H263e_PIC_TYPE_I, 0);
      if (sts != H263_STS_SKIPPED_FRAME)
        mLastIPic = mFrameCount;
    } else {
      sts = EncodePic(H263e_PIC_TYPE_P, 0);
      if (mNumIntraMB > mSceneChangeThreshold/* || sts == H263_STS_ERR_BUFOVER*/) {
        mVideoPicture.picture_coding_type = H263e_PIC_TYPE_I;
        mQuantIPic = mQuantPPic;
        cBS.Reset();
        sts = EncodePic(H263e_PIC_TYPE_I, 0);
        mLastIPic = mFrameCount;
      }
    }
    if (sts != H263_STS_SKIPPED_FRAME)
      if (mIPicdist != 1) {
        ExpandFrame(mFrameC->pY, mFrameC->pU, mFrameC->pV);
        h263e_Swap(mFrameF, mFrameC);
      }

#ifdef PRINT_INFO
    if (mCalcPSNR)
      printf("%f \n", 10.0 * log10(255.0 * 255.0 * mSourceWidth * mSourceHeight) - 10.0 * log10((double)mPSNR_Y));
#endif

    mVideoPicture.temporal_reference += mVideoPicture.temporal_reference_increment;
    mPictime += mVideoSequence.fixed_pic_time_increment;
    mFrameCount++;
    return sts;
  }

  // with B-frames
  if (noMoreData) {
    if (mNumOfFrames == -1)
        mNumOfFrames = mFrameCount;
    if (mFrameCount >= mNumOfFrames + mBPicdist)
        return H263_STS_NODATA;
    if (bufTooSmall)
      return H263_STS_ERR_BUFOVER;

    // last not closed B-frames are coded as P
    if (mFrameCount >= mNumOfFrames + mBPicdist - (mNumOfFrames - 1) % mPPicdist) {
      Ipp32s  bIndx;

      bIndx = mIndxBPic + 1;
      if (bIndx > 2 + mBPicdist)
        bIndx = 2;
      mFrameC = &mFrame[bIndx];
      // set mVideoPicture time
      nt = (Ipp32s)(mPictime - mBPicdist * mVideoSequence.fixed_pic_time_increment - mSyncTime);
      EncodePic(H263e_PIC_TYPE_P, nt);
      mSyncTimeB = nt;
      // reset Sync Time
      if (mVideoPicture.modulo_time_base != 0)
        mSyncTime = mPictime - mVideoPicture.pic_time_increment;
      mIndxBPic ++;
      if (mIndxBPic > 2 + mBPicdist)
        mIndxBPic = 2;
      ExpandFrame(mFrameC->pY, mFrameC->pU, mFrameC->pV);
      h263e_Swap(mFrameF, mFrameC);
      mPictime += mVideoSequence.fixed_pic_time_increment;
      mFrameCount ++;
        return H263_STS_NOERR;
    }
  }
  if (bufTooSmall)
    return H263_STS_ERR_BUFOVER;
  isIPic = !(mFrameCount % mIPicdist);
  isPPic = !(mFrameCount % mPPicdist);
  isBPic = !(isIPic || isPPic);
  if (!isBPic) {
    mFrameC = mFrameB;
    // set mVideoPicture time
    nt = (Ipp32s)(mPictime - mSyncTime);
    if (isIPic)
      EncodePic(H263e_PIC_TYPE_I, nt);
    else
      EncodePic(H263e_PIC_TYPE_P, nt);
    mSyncTimeB = nt;
    // reset Sync Time
    if (mVideoPicture.modulo_time_base != 0)
      mSyncTime = mPictime - mVideoPicture.pic_time_increment;
    if (mIPicdist != 1)
      ExpandFrame(mFrameC->pY, mFrameC->pU, mFrameC->pV);
  } else if (mFrameCount > mBPicdist) {
    Ipp32s  bIndx;

    bIndx = mIndxBPic + 1;
    if (bIndx > 2 + mBPicdist)
      bIndx = 2;
    mFrameC = &mFrame[bIndx];
    // set mVideoPicture time
    nt = mSyncTimeB - (mBPicdist - mNumBPic) * mVideoSequence.fixed_pic_time_increment;
    mTRD = mBPicdist + 1;
    mTRB = mNumBPic + 1;
    EncodePic(H263e_PIC_TYPE_B, nt);
  }
  // setup next frame
  if (isBPic) {
    mIndxBPic++;
    if (mIndxBPic > 2 + mBPicdist)
      mIndxBPic = 2;
    mNumBPic++;
    if (mNumBPic == mBPicdist) {
      // next frame is not B
      mNumBPic = 0;
      h263e_Swap(mFrameF, mFrameB);
      mFrameC = mFrameB;
    } else {
      // next frame is B
      mFrameC = &mFrame[mIndxBPic];
    }
  } else {
      // next frame is B
    mFrameC = &mFrame[mIndxBPic];
  }

  mPictime += mVideoSequence.fixed_pic_time_increment;
  mFrameCount ++;
  if (mFrameCount > mBPicdist + 1)
    return H263_STS_NOERR;
  else
    return isBPic ? H263_STS_BUFFERED : H263_STS_NOERR;
}

void ippVideoEncoderH263::PredictMV(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j, Ipp32s adv, IppMotionVector *mvPred)
{
  IppMotionVector *mvLeft, *mvTop, *mvRight;
  Ipp32s i1, i2;

  if (adv) {
    i1 = 1;
    i2 = 2;
  } else {
    i1 = i2 = 0;
  }

  mvLeft  = MBcurr[-1].mv;
  mvTop   = MBcurr[-mNumMacroBlockPerRow].mv;
  mvRight = MBcurr[-mNumMacroBlockPerRow+1].mv;

  if (i == frGOB && j == 0) {
    mvPred[0].dx = mvPred[0].dy = 0;
  } else if (j == 0) {
    mvPred[0].dx = h263e_Median(0, mvTop[i2].dx, mvRight[i2].dx);
    mvPred[0].dy = h263e_Median(0, mvTop[i2].dy, mvRight[i2].dy);
  } else if (i == frGOB) {
    mvPred[0] = mvLeft[i1];
  } else if (j == mNumMacroBlockPerRow - 1) {
    mvPred[0].dx = h263e_Median(0, mvLeft[i1].dx, mvTop[i2].dx);
    mvPred[0].dy = h263e_Median(0, mvLeft[i1].dy, mvTop[i2].dy);
  } else {
    mvPred[0].dx = h263e_Median(mvLeft[i1].dx, mvTop[i2].dx, mvRight[i2].dx);
    mvPred[0].dy = h263e_Median(mvLeft[i1].dy, mvTop[i2].dy, mvRight[i2].dy);
  }
}

void ippVideoEncoderH263::me4MV_Neighbours(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j, IppMotionVector *mvPred)
{
  IppMotionVector *mvLeft, *mvTop, *mvRight;
  mvLeft  = MBcurr[-1].mv;
  mvTop   = MBcurr[-mNumMacroBlockPerRow].mv;
  mvRight = MBcurr[-mNumMacroBlockPerRow+1].mv;

  if (i == frGOB) {
    mvPred[1].dx = mvPred[1].dy = 32767;  // top
    mvPred[2].dx = mvPred[2].dy = -32767; // top right
  } else if (j == mNumMacroBlockPerRow - 1) {
    mvPred[1] = mvTop[3];                // top
    mvPred[2].dx = mvPred[2].dy = 0;     // top right
  } else {
    mvPred[1] = mvTop[3];                // top
    mvPred[2] = mvRight[2];     // top right
  }
  if (j == 0)
    mvPred[3].dx = mvPred[3].dy = 0; // left
  else
    mvPred[3] = mvLeft[3];
}

void ippVideoEncoderH263::Predict3MV(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j,
                                     IppMotionVector *mvPred, IppMotionVector *mvCurr)
{
  IppMotionVector *mvLeft, *mvTop, *mvRight;

  mvLeft  = MBcurr[-1].mv;
  mvTop   = MBcurr[-mNumMacroBlockPerRow].mv;
  mvRight = MBcurr[-mNumMacroBlockPerRow+1].mv;
  // block 1
  if (i == frGOB) {
    mvPred[1] = mvCurr[0];
  } else if (j == mNumMacroBlockPerRow - 1) {
    mvPred[1].dx = h263e_Median(mvCurr[0].dx, mvTop[3].dx, 0);
    mvPred[1].dy = h263e_Median(mvCurr[0].dy, mvTop[3].dy, 0);
  } else {
    mvPred[1].dx = h263e_Median(mvCurr[0].dx, mvTop[3].dx, mvRight[2].dx);
    mvPred[1].dy = h263e_Median(mvCurr[0].dy, mvTop[3].dy, mvRight[2].dy);
  }
  // block 2
  if (j == 0) {
    mvPred[2].dx = h263e_Median(0, mvCurr[0].dx, mvCurr[1].dx);
    mvPred[2].dy = h263e_Median(0, mvCurr[0].dy, mvCurr[1].dy);
  } else {
    mvPred[2].dx = h263e_Median(mvLeft[3].dx, mvCurr[0].dx, mvCurr[1].dx);
    mvPred[2].dy = h263e_Median(mvLeft[3].dy, mvCurr[0].dy, mvCurr[1].dy);
  }
  // block 3
  mvPred[3].dx = h263e_Median(mvCurr[2].dx, mvCurr[0].dx, mvCurr[1].dx);
  mvPred[3].dy = h263e_Median(mvCurr[2].dy, mvCurr[0].dy, mvCurr[1].dy);
}

void ippVideoEncoderH263::EncodeMV(IppMotionVector *mv, Ipp32s mbType)
{
  Ipp32s i, nMV = (mbType == IPPVC_MBTYPE_INTER4V) ? 4 : 1;

  for (i = 0; i < nMV; i++) {
    if (mVideoPicture.UMV <= 1) {
      cBS.PutBits(mVLC_MVD_TB12[mv[i].dx+32].code, mVLC_MVD_TB12[mv[i].dx+32].len);
      cBS.PutBits(mVLC_MVD_TB12[mv[i].dy+32].code, mVLC_MVD_TB12[mv[i].dy+32].len);
    } else {
      Ipp32s mvabs[2], sign[2], j, val, s;
      Ipp32s m, nbits;
      Ipp32u code;

      sign[0] = (mv[i].dx >= 0 ? 0 : 1);
      mvabs[0] = (mv[i].dx >= 0 ? mv[i].dx : -mv[i].dx);
      sign[1] = (mv[i].dy >= 0 ? 0 : 1);
      mvabs[1] = (mv[i].dy >= 0 ? mv[i].dy : -mv[i].dy);

      for (j = 0; j < 2; j++) {
        val = mvabs[j];
        s = sign[j];

        if (val == 0)
          cBS.PutBits(1, 1);
        else if (val == 1)
          cBS.PutBits(s << 1, 3);
        else {
          if (val < 32) {
            m = 16;
            nbits = 11;
          } else {
            m = 2048;
            nbits = 25;
          }
          while (1) {
            if (val & m)
              break;
            nbits -= 2;
            m >>= 1;
          }
          val &= m - 1;
          m >>= 1;
          code = 0;
          while (m) {
            code |= ((val & m) << 1) | m;
            code <<= 1;
            m >>= 1;
          }
          code <<= 1;
          code |= s << 1;
          cBS.PutBits(code, nbits);
        }
      }
      if (mv[i].dx == 1 && mv[i].dy == 1)
        cBS.PutBits(1, 1);
    }
  }
}

inline void ippVideoEncoderH263::EncodeMCBPC_I(Ipp32s mbtype, Ipp32s mcbpc)
{
  if (mbtype == IPPVC_MBTYPE_INTRA) {
    if (mcbpc == 0)
      cBS.PutBits(1, 1);
    else
      cBS.PutBits(mcbpc, 3);
  } else {
    if (mcbpc == 0)
      cBS.PutBits(1, 4);
    else
      cBS.PutBits(mcbpc, 7);
  }
}

inline void ippVideoEncoderH263::EncodeAdvIntraPredMode(Ipp32s scan)
{
  if (scan == IPPVC_SCAN_ZIGZAG)
    cBS.PutBits(0, 1);
  else if (scan == IPPVC_SCAN_VERTICAL)
    cBS.PutBits(3, 2);
  else
    cBS.PutBits(2, 2);
}

inline void ippVideoEncoderH263::EncodeCBPY_I(Ipp32s pat)
{
    cBS.PutBits(mVLC_CBPY_TB8[pat].code, mVLC_CBPY_TB8[pat].len);
}

inline void ippVideoEncoderH263::EncodeMCBPC_P(Ipp32s mbtype, Ipp32s pat)
{
    cBS.PutBits(mVLC_MCBPC_TB7[mbtype*4+pat].code, mVLC_MCBPC_TB7[mbtype*4+pat].len);
}

inline void ippVideoEncoderH263::EncodeCBPY_P(Ipp32s mbtype, Ipp32s pat)
{
    if (mbtype <= IPPVC_MBTYPE_INTER4V)
        pat = 15 - pat;
    cBS.PutBits(mVLC_CBPY_TB8[pat].code, mVLC_CBPY_TB8[pat].len);
}

void ippVideoEncoderH263::ChooseAdvIntraPred(h263e_MacroBlock *MBcurr, Ipp16s *coeffs, Ipp32s *predDir)
{
  h263e_Block *bCurr;
  Ipp16s      *predAcA, *predAcC;
  Ipp32s      sadDC, sadH, sadV, absCoefV, absCoefH, s, dc;
  Ipp16s      *coef;
  Ipp32s      i, v;

  sadDC = sadH = sadV = 0;

  for (i = 0; i < 4; i++) {
    bCurr = &MBcurr->block[i];
    coef = coeffs + i*64;

    absCoefH = absCoefV = 0;

    bCurr->dctOr_acA[0] = bCurr->dctOr_acC[0] = coef[0];
    for (v = 1; v < 8; v++) {
      bCurr->dctOr_acA[v] = coef[v*8];
      absCoefV += h263e_Abs(coef[v*8]);
      bCurr->dctOr_acC[v] = coef[v];
      absCoefH += h263e_Abs(coef[v]);
    }

    predAcA = bCurr->predA ? (bCurr->predA->validPredIntra ? bCurr->predA->dctOr_acA : NULL) : NULL;
    predAcC = bCurr->predC ? (bCurr->predC->validPredIntra ? bCurr->predC->dctOr_acC : NULL) : NULL;

    if (predAcA) {
      if (predAcC)
        dc = h263e_Abs(coef[0] - ((predAcA[0] + predAcC[0] + 1) >> 1));
      else
        dc = h263e_Abs(coef[0] - predAcA[0]);
    } else if (predAcC)
      dc = h263e_Abs(coef[0] - predAcC[0]);
    else
      dc = h263e_Abs(coef[0] - 1024);

    sadDC += dc + ((absCoefV + absCoefH) << 5);

    // block to the left
    s = absCoefH;
    if (predAcA) {
      dc = h263e_Abs(coef[0] - predAcA[0]);
      for (v = 1; v < 8; v++)
        s += h263e_Abs(coef[v*8] - predAcA[v]);
    } else {
      dc = h263e_Abs(coef[0] - 1024);
      s += absCoefV;
    }
    sadV += dc + (s << 5);

    // block above
    s = absCoefV;
    if (predAcC) {
      dc = h263e_Abs(coef[0] - predAcC[0]);
      for (v = 1; v < 8; v++)
        s += h263e_Abs(coef[v] - predAcC[v]);
    } else {
      dc = h263e_Abs(coef[0] - 1024);
      s += absCoefH;
    }
    sadH += dc + (s << 5);
  }

  *predDir = IPPVC_SCAN_ZIGZAG;

  if (sadV < sadDC) {
    *predDir = IPPVC_SCAN_VERTICAL;
    sadDC = sadV;
  }

  if (sadH < sadDC)
    *predDir = IPPVC_SCAN_HORIZONTAL;
}

Ipp32s ippVideoEncoderH263::PredictReconstructAdvIntra(Ipp8u *pF[6], h263e_MacroBlock *MBcurr, Ipp16s *coeffs,
                                                       Ipp32s *nzCount, Ipp32s quant, Ipp32s scan)
{
  __ALIGN16(Ipp16s, rcoef, 64);
  __ALIGN16(Ipp8u, pDec, 64*6);
  h263e_Block *bCurr;
  Ipp16s    *predAcA, *predAcC;
  Ipp16s    *coef;
  Ipp32s    i, v;
  Ipp16s    pred;
  Ipp32s    step;
  Ipp32s    pattern = 0;

  if (mCalcPSNR) {
    ippiCopy16x16_8u_C1R(pF[0], mStepLuma, pDec, 16);
    ippiCopy8x8_8u_C1R(pF[4], mStepChroma, pDec+256, 8);
    ippiCopy8x8_8u_C1R(pF[5], mStepChroma, pDec+320, 8);
  }

  for (i = 0; i < 6; i++) {
    bCurr = &MBcurr->block[i];
    coef = coeffs + i*64;
    step = (i < 4 ? mStepLuma : mStepChroma);

    predAcA = bCurr->predA ? (bCurr->predA->validPredIntra ? bCurr->predA->dct_acA : NULL) : NULL;
    predAcC = bCurr->predC ? (bCurr->predC->validPredIntra ? bCurr->predC->dct_acC : NULL) : NULL;

    if (scan == IPPVC_SCAN_ZIGZAG) {
      if (predAcA)
        if (predAcC)
          pred = (predAcA[0] + predAcC[0] + 1) >> 1;
        else
          pred = predAcA[0];
      else
        pred = predAcC ? predAcC[0] : 1024;
      coef[0] = coef[0] - pred;
    } else if (scan == IPPVC_SCAN_VERTICAL) {
      if (predAcA) {
        for (v = 0; v < 8; v++)
          coef[v*8] = coef[v*8] - predAcA[v];
      } else
        coef[0] = coef[0] - 1024;
    } else { // IPPVC_SCAN_HORIZONTAL
      if (predAcC) {
        for (v = 0; v < 8; v++)
          coef[v] = coef[v] - predAcC[v];
      } else
        coef[0] = coef[0] - 1024;
    }

    // No need to clip to [-2048,2047] as Intra AC DCT coeffs are in the range [-1020, 1020], DC -  [0, 2040]
    ippiQuantIntra_H263_16s_C1I(coef, quant, &nzCount[i], 1, mVideoPicture.modQuant);

    if (nzCount[i])
      pattern |= 32 >> i;

    ippsCopy_16s(coef, rcoef, 64); // have only inplace Quant/InvQuant

    ReconBlockIntra_AdvI_H263(pF[i], step, rcoef, bCurr, quant, nzCount[i], scan);
  }

  if (mCalcPSNR) {
    mPSNR_Y += h263e_CalcMSE_16x16(pF[0], mStepLuma, pDec, 16);
    mPSNR_U += h263e_CalcMSE_8x8(pF[4], mStepChroma, pDec+256, 8);
    mPSNR_V += h263e_CalcMSE_8x8(pF[5], mStepChroma, pDec+320, 8);
  }

  return pattern;
}

void ippVideoEncoderH263::DCT8x8MacroBlock_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB)
{
  ippiDCT8x8Fwd_8u16s_C1R(pY, mStepLuma, coeffMB+0*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8, mStepLuma, coeffMB+1*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma, mStepLuma, coeffMB+2*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma+8, mStepLuma, coeffMB+3*64);
  ippiDCT8x8Fwd_8u16s_C1R(pU, mStepChroma, coeffMB+4*64);
  ippiDCT8x8Fwd_8u16s_C1R(pV, mStepChroma, coeffMB+5*64);
}

// called only if (!mVideoPicture.advIntra)
Ipp32s ippVideoEncoderH263::QuantMacroBlockIntra_H263(Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s quant)
{
  Ipp32s pattern;
  ippiQuantIntra_H263_16s_C1I(coeffMB+0*64, quant, &nzCount[0], 0, mVideoPicture.modQuant);
  ippiQuantIntra_H263_16s_C1I(coeffMB+1*64, quant, &nzCount[1], 0, mVideoPicture.modQuant);
  ippiQuantIntra_H263_16s_C1I(coeffMB+2*64, quant, &nzCount[2], 0, mVideoPicture.modQuant);
  ippiQuantIntra_H263_16s_C1I(coeffMB+3*64, quant, &nzCount[3], 0, mVideoPicture.modQuant);
  ippiQuantIntra_H263_16s_C1I(coeffMB+4*64, quant, &nzCount[4], 0, mVideoPicture.modQuant);
  ippiQuantIntra_H263_16s_C1I(coeffMB+5*64, quant, &nzCount[5], 0, mVideoPicture.modQuant);
  h263e_SetPatternIntra(pattern, nzCount, 1);
  return pattern;
}

Ipp32s ippVideoEncoderH263::TransMacroBlockInter_H263(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp16s *coeffMB,
                                                      Ipp32s *nzCount, Ipp32s quant, Ipp8u *mcPred, Ipp32s lumaErr)
{
    Ipp32s pattern, sU, sV, sL0, sL1, sL2, sL3, lim;

    lim = quant * 16;
    if (lumaErr < quant * 20) {
        nzCount[0] = nzCount[1] = nzCount[2] = nzCount[3] = 0;
        coeffMB[0*64] = coeffMB[1*64] = coeffMB[2*64] = coeffMB[3*64] = 0;
    } else {
        ippiSubSAD8x8_8u16s_C1R(pYc, mStepLuma, mcPred, 16, coeffMB+0*64, 16, &sL0);
        ippiSubSAD8x8_8u16s_C1R(pYc+8, mStepLuma, mcPred+8, 16, coeffMB+1*64, 16, &sL1);
        ippiSubSAD8x8_8u16s_C1R(pYc+8*mStepLuma, mStepLuma, mcPred+128, 16, coeffMB+2*64, 16, &sL2);
        ippiSubSAD8x8_8u16s_C1R(pYc+8*mStepLuma+8, mStepLuma, mcPred+136, 16, coeffMB+3*64, 16, &sL3);
        if (sL0 < lim) {
            nzCount[0] = 0;
            coeffMB[0*64] = 0;
        } else {
            ippiDCT8x8Fwd_16s_C1I(coeffMB+0*64);
            ippiQuantInter_H263_16s_C1I(coeffMB+0*64, quant, &nzCount[0], mVideoPicture.modQuant);
        }
        if (sL1 < lim) {
            nzCount[1] = 0;
            coeffMB[1*64] = 0;
        } else {
            ippiDCT8x8Fwd_16s_C1I(coeffMB+1*64);
            ippiQuantInter_H263_16s_C1I(coeffMB+1*64, quant, &nzCount[1], mVideoPicture.modQuant);
        }
        if (sL2 < lim) {
            nzCount[2] = 0;
            coeffMB[2*64] = 0;
        } else {
            ippiDCT8x8Fwd_16s_C1I(coeffMB+2*64);
            ippiQuantInter_H263_16s_C1I(coeffMB+2*64, quant, &nzCount[2], mVideoPicture.modQuant);
        }
        if (sL3 < lim) {
            nzCount[3] = 0;
            coeffMB[3*64] = 0;
        } else {
            ippiDCT8x8Fwd_16s_C1I(coeffMB+3*64);
            ippiQuantInter_H263_16s_C1I(coeffMB+3*64, quant, &nzCount[3], mVideoPicture.modQuant);
        }
    }
    ippiSubSAD8x8_8u16s_C1R(pUc, mStepChroma, mcPred+64*4, 8, coeffMB+4*64, 16, &sU);
    ippiSubSAD8x8_8u16s_C1R(pVc, mStepChroma, mcPred+64*5, 8, coeffMB+5*64, 16, &sV);
    if (sU < lim) {
        nzCount[4] = 0;
        coeffMB[4*64] = 0;
    } else {
        ippiDCT8x8Fwd_16s_C1I(coeffMB+4*64);
        ippiQuantInter_H263_16s_C1I(coeffMB+4*64, quant, &nzCount[4], mVideoPicture.modQuant);
    }
    if (sV < lim) {
        nzCount[5] = 0;
        coeffMB[5*64] = 0;
    } else {
        ippiDCT8x8Fwd_16s_C1I(coeffMB+5*64);
        ippiQuantInter_H263_16s_C1I(coeffMB+5*64, quant, &nzCount[5], mVideoPicture.modQuant);
    }
    h263e_SetPatternInter(pattern, nzCount);
    return pattern;
}

inline void ippVideoEncoderH263::EncodeMacroBlockIntra_H263(Ipp16s *coeffMB, Ipp32s pattern, Ipp32s *nzCount, Ipp32s scan)
{
  Ipp32s i, pm = 32;

  if (mVideoPicture.advIntra) {
    for (i = 0; i < 6; i ++) {
      if (pattern & pm)
        ippiEncodeCoeffsIntra_H263_16s1u(coeffMB+i*64, &cBS.mPtr, &cBS.mBitOff, nzCount[i], 1, mVideoPicture.modQuant, scan);
      pm >>= 1;
    }
  } else {
    for (i = 0; i < 6; i ++) {
      ippiEncodeDCIntra_H263_16s1u(coeffMB[i*64], &cBS.mPtr, &cBS.mBitOff);
      if (pattern & pm)
        ippiEncodeCoeffsIntra_H263_16s1u(coeffMB+i*64, &cBS.mPtr, &cBS.mBitOff, nzCount[i] - 1, 0, mVideoPicture.modQuant, IPPVC_SCAN_ZIGZAG);
      pm >>= 1;
    }
  }
}

inline void ippVideoEncoderH263::EncodeMacroBlockInter_H263(Ipp16s *coeffMB, Ipp32s pattern, Ipp32s *nzCount)
{
  Ipp32s  i, pm = 32;

  for (i = 0; i < 6; i ++) {
    if (pattern & pm)
      ippiEncodeCoeffsInter_H263_16s1u(coeffMB+i*64, &cBS.mPtr, &cBS.mBitOff, nzCount[i], mVideoPicture.modQuant, IPPVC_SCAN_ZIGZAG);
    pm >>= 1;
  }
}

inline void ippVideoEncoderH263::ReconMacroBlockNotCoded(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred)
{
  if (mCalcPSNR) {
    mPSNR_Y += h263e_CalcMSE_16x16(mcPred, 16, pYc, mStepLuma);
    mPSNR_U += h263e_CalcMSE_8x8(mcPred+64*4, 8, pUc, mStepChroma);
    mPSNR_V += h263e_CalcMSE_8x8(mcPred+64*5, 8, pVc, mStepChroma);
  }
  ippiCopy16x16_8u_C1R(mcPred, 16, pYc, mStepLuma);
  ippiCopy8x8_8u_C1R(mcPred+64*4, 8, pUc, mStepChroma);
  ippiCopy8x8_8u_C1R(mcPred+64*5, 8, pVc, mStepChroma);
}

// called only if (!mVideoPicture.advIntra)
void ippVideoEncoderH263::ReconMacroBlockIntra_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s quant, Ipp32s pattern)
{
  __ALIGN16(Ipp8u, pDec, 64*6);

  if (mCalcPSNR) {
    ippiCopy16x16_8u_C1R(pY, mStepLuma, pDec, 16);
    ippiCopy8x8_8u_C1R(pU, mStepChroma, pDec+256, 8);
    ippiCopy8x8_8u_C1R(pV, mStepChroma, pDec+320, 8);
  }
  if (pattern & 32) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+0*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+0*64, pY, mStepLuma);
  } else {
    h263e_Set8x8_8u(pY, mStepLuma, (Ipp8u)coeffMB[0*64]);
  }
  if (pattern & 16) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+1*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+1*64, pY+8, mStepLuma);
  } else {
    h263e_Set8x8_8u(pY+8, mStepLuma, (Ipp8u)coeffMB[1*64]);
  }
  if (pattern & 8) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+2*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+2*64, pY+8*mStepLuma, mStepLuma);
  } else {
    h263e_Set8x8_8u(pY+8*mStepLuma, mStepLuma, (Ipp8u)coeffMB[2*64]);
  }
  if (pattern & 4) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+3*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+3*64, pY+8*mStepLuma+8, mStepLuma);
  } else {
    h263e_Set8x8_8u(pY+8*mStepLuma+8, mStepLuma, (Ipp8u)coeffMB[3*64]);
  }
  if (pattern & 2) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+4*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+4*64, pU, mStepChroma);
  } else {
    h263e_Set8x8_8u(pU, mStepChroma, (Ipp8u)coeffMB[4*64]);
  }
  if (pattern & 1) {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+5*64, 63, quant, 0, mVideoPicture.modQuant);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB+5*64, pV, mStepChroma);
  } else {
    h263e_Set8x8_8u(pV, mStepChroma, (Ipp8u)coeffMB[5*64]);
  }
  if (mCalcPSNR) {
      mPSNR_Y += h263e_CalcMSE_16x16(pY, mStepLuma, pDec, 16);
      mPSNR_U += h263e_CalcMSE_8x8(pU, mStepChroma, pDec+256, 8);
      mPSNR_V += h263e_CalcMSE_8x8(pV, mStepChroma, pDec+320, 8);
  }
}

void ippVideoEncoderH263::ReconBlockIntra_AdvI_H263(Ipp8u *p, Ipp32s step, Ipp16s *coef, h263e_Block *bCurr,
                                                    Ipp32s quant, Ipp32s pattern, Ipp32s scan)
{
  Ipp32s v;
  Ipp16s *predAcA, *predAcC, pred;
  Ipp32s lnz;

  predAcA = bCurr->predA ? (bCurr->predA->validPredIntra ? bCurr->predA->dct_acA : NULL) : NULL;
  predAcC = bCurr->predC ? (bCurr->predC->validPredIntra ? bCurr->predC->dct_acC : NULL) : NULL;

  if (pattern) {
    ippiQuantInvIntra_H263_16s_C1I(coef, 63, quant, 1, mVideoPicture.modQuant);
    lnz = 63;
  } else {
    if (scan != IPPVC_SCAN_ZIGZAG) {
      h263e_Zero64_16s(coef);
    } else
      coef[0] = 0;
    lnz = 0;
  }

  if (scan == IPPVC_SCAN_ZIGZAG) {
    if (predAcA)
      if (predAcC)
        pred = (predAcA[0] + predAcC[0] + 1) >> 1;
      else
        pred = predAcA[0];
    else
      pred = predAcC ? predAcC[0] : 1024;
    pred += coef[0];
    pred |= 1;
    h263e_Clip(pred, 0, 2047);
    coef[0] = pred;
  } else if (scan == IPPVC_SCAN_VERTICAL) {
    if (predAcA) {
      pred = coef[0] + predAcA[0];
      pred |= 1;
      h263e_Clip(pred, 0, 2047);
      coef[0] = pred;
      for (v = 1; v < 8; v++) {
        pred = coef[v*8] + predAcA[v];
        h263e_Clip(pred, -2048, 2047);
        coef[v*8] = pred;
        lnz |= pred;
      }
    } else {
      pred = coef[0] + 1024;
      pred |= 1;
      h263e_Clip(pred, 0, 2047);
      coef[0] = pred;
    }
  } else {
    if (predAcC) {
      pred = coef[0] + predAcC[0];
      pred |= 1;
      h263e_Clip(pred, 0, 2047);
      coef[0] = pred;
      for (v = 1; v < 8; v++) {
        pred = coef[v] + predAcC[v];
        h263e_Clip(pred, -2048, 2047);
        coef[v] = pred;
        lnz |= pred;
      }
    } else {
      pred = coef[0] + 1024;
      pred |= 1;
      h263e_Clip(pred, 0, 2047);
      coef[0] = pred;
    }
  }

  if (lnz) {
    ippiDCT8x8Inv_16s8u_C1R(coef, p, step);
    /* copy predicted coeffs for future Prediction */
    for (v = 0; v < 8; v ++) {
      bCurr->dct_acC[v] = coef[v];
      bCurr->dct_acA[v] = coef[v*8];
    }
  } else {
    h263e_Zero8_16s(bCurr->dct_acA);
    h263e_Zero8_16s(bCurr->dct_acC);
    bCurr->dct_acA[0] = bCurr->dct_acC[0] = coef[0];
    pred = (coef[0] + 4) >> 3;
    h263e_ClipR(pred, 255);
    h263e_Set8x8_8u(p, step, (Ipp8u)pred);
  }
}

void ippVideoEncoderH263::ReconMacroBlockInter_H263(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred,
                                                    Ipp16s *coeffMB, Ipp32s quant, Ipp32s pattern)
{
    if (pattern & 32) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+0*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+0*64);
        h263e_Add8x8_16s8u(mcPred, coeffMB+0*64, 16);
    }
    if (pattern & 16) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+1*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+1*64);
        h263e_Add8x8_16s8u(mcPred+8, coeffMB+1*64, 16);
    }
    if (pattern & 8) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+2*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+2*64);
        h263e_Add8x8_16s8u(mcPred+16*8, coeffMB+2*64, 16);
    }
    if (pattern & 4) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+3*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+3*64);
        h263e_Add8x8_16s8u(mcPred+16*8+8, coeffMB+3*64, 16);
    }
    if (pattern & 2) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+4*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+4*64);
        h263e_Add8x8_16s8u(mcPred+64*4, coeffMB+4*64, 8);
    }
    if (pattern & 1) {
        ippiQuantInvInter_H263_16s_C1I(coeffMB+5*64, 63, quant, mVideoPicture.modQuant);
        ippiDCT8x8Inv_16s_C1I(coeffMB+5*64);
        h263e_Add8x8_16s8u(mcPred+64*5, coeffMB+5*64, 8);
    }
    if (mCalcPSNR) {
        mPSNR_Y += h263e_CalcMSE_16x16(pYc, mStepLuma, mcPred, 16);
        mPSNR_U += h263e_CalcMSE_8x8(pUc, mStepChroma, mcPred+64*4, 8);
        mPSNR_V += h263e_CalcMSE_8x8(pVc, mStepChroma, mcPred+64*5, 8);
    }
    ippiCopy16x16_8u_C1R(mcPred, 16, pYc, mStepLuma);
    ippiCopy8x8_8u_C1R(mcPred+64*4, 8, pUc, mStepChroma);
    ippiCopy8x8_8u_C1R(mcPred+64*5, 8, pVc, mStepChroma);
}

void ippVideoEncoderH263::EncodeIPic()
{
  __ALIGN16(Ipp16s, coeffMB, 64*6);
  Ipp8u  *pY, *pU, *pV, *pF[6];
  Ipp32s i, j, quant, pattern;
  Ipp32s nzCount[6];
  h263e_MacroBlock *MBcurr = mMBinfo;
  Ipp32s scan;
  Ipp32s nmbf = 0;
  Ipp32s gn, gRow, frGOB = 0;

  mVideoPicture.gob_number = 1;
  quant = mVideoPicture.pic_quant;

  for (gn = 0; gn < mVideoPicture.num_gobs_in_pic; gn++) {
    mRTPdata.GOBstartPos[gn] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;

    if (mGOBheaders && gn) {
      EncodeGOB_Header(gn);
      frGOB = gn*mVideoPicture.num_MBrows_in_gob;
    }

    for (gRow = 0; gRow < mVideoPicture.num_MBrows_in_gob; gRow++) {
      Ipp32s boundOnTop;
      i = gn*mVideoPicture.num_MBrows_in_gob + gRow;

      boundOnTop = (i == frGOB && i);

      pY = mFrameC->pY + i * 16 * mStepLuma;
      pU = mFrameC->pU + i * 8 * mStepChroma;
      pV = mFrameC->pV + i * 8 * mStepChroma;
      for (j = 0; j < mNumMacroBlockPerRow; j++) {
        MBcurr->mv[0].dx = MBcurr->mv[0].dy = 0;
        MBcurr->block[0].validPredIntra = MBcurr->block[1].validPredIntra = MBcurr->block[2].validPredIntra = MBcurr->block[3].validPredIntra = MBcurr->block[4].validPredIntra = MBcurr->block[5].validPredIntra = 1;

        // for RTP support
        mRTPdata.MBpos[nmbf] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
        mRTPdata.MBquant[nmbf] = (Ipp8u)quant;

        DCT8x8MacroBlock_H263(pY, pU, pV, coeffMB);

        if (mVideoPicture.advIntra) {
          pF[0] = pY; pF[1] = pY + 8; pF[2] = pY + 8*mStepLuma; pF[3] = pY + 8*mStepLuma + 8;
          pF[4] = pU; pF[5] = pV;

          if (boundOnTop) { // mark top row blocks as invalid
            MBcurr->block[0].predC->validPredIntra = 0;
            MBcurr->block[1].predC->validPredIntra = 0;
            MBcurr->block[4].predC->validPredIntra = 0;
            MBcurr->block[5].predC->validPredIntra = 0;
          }

          ChooseAdvIntraPred(MBcurr, coeffMB, &scan);
          pattern = PredictReconstructAdvIntra(pF, MBcurr, coeffMB, nzCount, quant, scan);
        } else {
          scan = IPPVC_SCAN_ZIGZAG;
          pattern = QuantMacroBlockIntra_H263(coeffMB, nzCount, quant);
        }

        EncodeMCBPC_I(IPPVC_MBTYPE_INTRA, pattern & 3);

        if (mVideoPicture.advIntra)
          EncodeAdvIntraPredMode(scan);

        EncodeCBPY_I(pattern >> 2);

        EncodeMacroBlockIntra_H263(coeffMB, pattern, nzCount, scan);

        if (!mVideoPicture.advIntra && (mIPicdist != 1 || mCalcPSNR))
          ReconMacroBlockIntra_H263(pY, pU, pV, coeffMB, quant, pattern);

        pY += 16; pU += 8; pV += 8;
        MBcurr ++;
        nmbf ++;
      }
    }
  }
  EncodeZeroBitsAlign();
}

inline Ipp32s h263e_MC_type(IppMotionVector *mv)
{
    return (((mv->dy & 1) << 2) + ((mv->dx & 1) << 3));
}

inline Ipp32s h263e_MC_offs(IppMotionVector *mv, Ipp32s step)
{
    return ((mv->dy >> 1) * step + (mv->dx >> 1));
}

static void h263e_ME_FullSearch_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                          Ipp32s yT, Ipp32s yB, Ipp32s *xPos, Ipp32s *yPos, Ipp32s *bestSAD, Ipp32s thrSAD_16x16)
{
    Ipp32s  cSAD, bSAD, i, j, jPos, iPos;

    jPos = *xPos;
    iPos = *yPos;
    bSAD = *bestSAD;
#ifdef ME_FULLSEARCH_RECT
    pRef += yT * step;
    for (i = yT; i <= yB; i ++) {
        for (j = xL; j <= xR; j ++) {
            ippiSAD16x16_8u32s(pCur, step, pRef+j, step, &cSAD, IPPVC_MC_APX_FF);
            if (cSAD < bSAD) {
                bSAD = cSAD;
                iPos = i;
                jPos = j;
#ifdef ME_USE_THRESHOLD
                if (cSAD <= thrSAD_16x16) goto L_01;
#endif
            }
        }
        pRef += step;
    }
#else   // involute search
    Ipp32s  nLoop, cLoop, lPos;
    i = iPos;
    j = jPos;
    pRef += iPos * step + jPos;
    nLoop = IPP_MAX(IPP_MAX(jPos - xL, xR - jPos), IPP_MAX(iPos - yT, yB - iPos));
    for (cLoop = 1; cLoop <= nLoop; cLoop ++) {
        i --;
        j --;
        pRef -= step + 1;
        for (lPos = 0; lPos < (cLoop << 3); lPos ++) {
            if (j >= xL && j <= xR && i >= yT && i <= yB) {
                ippiSAD16x16_8u32s(pCur, step, pRef, step, &cSAD, IPPVC_MC_APX_FF);
                if (cSAD < bSAD) {
                    bSAD = cSAD;
                    iPos = i;
                    jPos = j;
#ifdef ME_USE_THRESHOLD
                    if (cSAD <= thrSAD_16x16) goto L_01;
#endif
                }
            }
            if (lPos < (cLoop * 2)) {
                j ++;  pRef ++;
            } else if (lPos < (cLoop * 4)) {
                i ++;  pRef += step;
            } else if (lPos < (cLoop * 6)) {
                j --;  pRef --;
            } else {
                i --;  pRef -= step;
            }
        }
    }
#endif
#ifdef ME_USE_THRESHOLD
L_01:
#endif
    *bestSAD = bSAD;
    *xPos = jPos;
    *yPos = iPos;
}

static const Ipp32s bdJ[9] = {0, -2, -1, 0, 1, 2, 1, 0, -1},
                    bdI[9] = {0, 0, -1, -2, -1, 0, 1, 2, 1},
                    sdJ[5] = {0, -1, 0, 1, 0},
                    sdI[5] = {0, 0, -1, 0, 1};

static void h263e_ME_DiamondSearch_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                             Ipp32s yT, Ipp32s yB, Ipp32s *xPos, Ipp32s *yPos, Ipp32s *bestSAD,
                                             Ipp32s *sadBuf, Ipp32s thrSAD_16x16)
{
    Ipp32s  cSAD, bSAD, i, j, k, l, iPos, jPos, swWidth, swHeight;

    jPos = *xPos;
    iPos = *yPos;
    bSAD = *bestSAD;
    swWidth = (xR - xL + 1);
    swHeight = (yB - yT + 1);
    ippsSet_8u((Ipp8u)-1, (Ipp8u*)sadBuf, swWidth * swHeight * 4);
    sadBuf[(iPos-yT)*swWidth+jPos-xL] = bSAD;
    for (;;) {
        // find SAD at big diamond
        l = 0;
        for (k = 1; k < 9; k ++) {
            j = jPos + bdJ[k];
            i = iPos + bdI[k];
            if (j >= xL && j <= xR && i >= yT && i <= yB) {
                if (sadBuf[(i-yT)*swWidth+j-xL] == -1) {
                    ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &cSAD, IPPVC_MC_APX_FF);
#ifdef ME_USE_THRESHOLD
                    if (cSAD <= thrSAD_16x16) {
                        *xPos = j;
                        *yPos = i;
                        *bestSAD = cSAD;
                        return;
                    }
#endif
                    sadBuf[(i-yT)*swWidth+j-xL] = cSAD;
                    if (cSAD < bSAD) {
                        l = k;
                        bSAD = cSAD;
                    }
                }
            }
        }
        if (l == 0) {
            // find SAD at small diamond
            for (k = 1; k <= 4; k ++) {
                j = jPos + sdJ[k];
                i = iPos + sdI[k];
                if (j >= xL && j <= xR && i >= yT && i <= yB) {
                    if (sadBuf[(i-yT)*swWidth+j-xL] == -1) {
                        ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &cSAD, IPPVC_MC_APX_FF);
#ifdef ME_USE_THRESHOLD
                        if (cSAD <= thrSAD_16x16) {
                            *xPos = j;
                            *yPos = i;
                            *bestSAD = cSAD;
                            return;
                        }
#endif
                        sadBuf[(i-yT)*swWidth+j-xL]= cSAD;
                        if (cSAD < bSAD) {
                            l = k;
                            bSAD = cSAD;
                        }
                    }
                }
            }
            *xPos = jPos + sdJ[l];
            *yPos = iPos + sdI[l];
            *bestSAD = bSAD;
            return;
        }
        iPos += bdI[l];
        jPos += bdJ[l];
    }
}

static void h263e_ME_HalfPel_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                       Ipp32s yT, Ipp32s yB, Ipp32s xPos, Ipp32s yPos, Ipp32s *bestSAD,
                                       IppMotionVector *mv, Ipp32s thrSAD_16x16)
{
    Ipp32s  cSAD, bSAD, i, j;

    i = 0;  j = 0;  bSAD = *bestSAD;
    if (xPos > xL && yPos > yT) {
        ippiSAD16x16_8u32s(pCur, step, pRef+(yPos-1)*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i = -1;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (yPos > yT) {
        ippiSAD16x16_8u32s(pCur, step, pRef+(yPos-1)*step+xPos, step, &cSAD, IPPVC_MC_APX_FH);
        if (cSAD < bSAD) {
            i = -1;  j =  0;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (xPos < xR && yPos > yT) {
        ippiSAD16x16_8u32s(pCur, step, pRef+(yPos-1)*step+xPos, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i = -1;  j =  1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (xPos > xL) {
        ippiSAD16x16_8u32s(pCur, step, pRef+yPos*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HF);
        if (cSAD < bSAD) {
            i =  0;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (xPos < xR) {
        ippiSAD16x16_8u32s(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_HF);
        if (cSAD < bSAD) {
            i =  0;  j =  1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (xPos > xL && yPos < yB) {
        ippiSAD16x16_8u32s(pCur, step, pRef+yPos*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i =  1;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (yPos < yB) {
        ippiSAD16x16_8u32s(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_FH);
        if (cSAD < bSAD) {
            i =  1;  j =  0;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_16x16) goto L_01;
#endif
        }
    }
    if (xPos < xR && yPos < yB) {
        ippiSAD16x16_8u32s(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i =  1;  j =  1;  bSAD = cSAD;
        }
    }
#ifdef ME_USE_THRESHOLD
L_01:
#endif
    mv->dx = (Ipp16s)((xPos << 1) + j);
    mv->dy = (Ipp16s)((yPos << 1) + i);
    *bestSAD = bSAD;
}

static void h263e_ME_HalfPel_SAD_Fast_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                            Ipp32s yT, Ipp32s yB, Ipp32s xPos, Ipp32s yPos, Ipp32s *bestSAD,
                                            IppMotionVector *mv, Ipp32s *sadBuff)
{
    Ipp32s  i, j, swWidth, swHeight;
    Ipp32s  m0, m1, m2, m3, m4;

    m0 = *bestSAD;
    swWidth = (xR - xL + 1);
    swHeight = (yB - yT + 1);
    j = xPos;
    i = yPos - 1;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        m1 = sadBuff[(i-yT)*swWidth+j-xL];
        if (m1 == -1)
            ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &m1, IPPVC_MC_APX_FF);
    } else
        m1 = 1000000;
    j = xPos;
    i = yPos + 1;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        m2 = sadBuff[(i-yT)*swWidth+j-xL];
        if (m2 == -1)
            ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &m2, IPPVC_MC_APX_FF);
    } else
        m2 = 1000000;
    j = xPos - 1;
    i = yPos;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        m3 = sadBuff[(i-yT)*swWidth+j-xL];
        if (m3 == -1)
            ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &m3, IPPVC_MC_APX_FF);
    } else
        m3 = 1000000;
    j = xPos + 1;
    i = yPos;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        m4 = sadBuff[(i-yT)*swWidth+j-xL];
        if (m4 == -1)
            ippiSAD16x16_8u32s(pCur, step, pRef+i*step+j, step, &m4, IPPVC_MC_APX_FF);
    } else
        m4 = 1000000;
    if (2*(m3-m0) < (m4-m0)) {
        j = -1;
    } else if ((m3-m0) > 2*(m4-m0)) {
        j = +1;
    } else
        j = 0;
    if (2*(m1-m0) < (m2-m0)) {
        i = -1;
    } else if ((m1-m0) > 2*(m2-m0)) {
        i = +1;
    } else
        i = 0;
    mv->dx = (Ipp16s)((xPos << 1) + j);
    mv->dy = (Ipp16s)((yPos << 1) + i);
    if (j != 0 || i != 0) {
        ippiSAD16x16_8u32s(pCur, step, pRef+h263e_MC_offs(mv, step), step, bestSAD, h263e_MC_type(mv));
        if (*bestSAD > m0) {
            // false choosing
            mv->dx = (Ipp16s)(xPos << 1);
            mv->dy = (Ipp16s)(yPos << 1);
            *bestSAD = m0;
        }
    }
}

void ippVideoEncoderH263::ME_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                       Ipp32s yT, Ipp32s yB, Ipp32s *bestSAD, IppMotionVector *mv)
{
    Ipp32s  xPos, yPos, sadL;

    // mv - Predicted MV
    // bestSAD is the SAD at (0,0)
#ifdef ME_USE_THRESHOLD
    if (*bestSAD <= mMEthrSAD16x16) {
        mv->dx = mv->dy = 0;
        return;
    }
#endif
    xPos = h263e_Div2(mv->dx);
    yPos = h263e_Div2(mv->dy);
    // check init mv is in rect
    if (xPos < xL || xPos > xR || yPos < yT || yPos > yB) {
        // reset to (0,0)
        xPos = yPos = 0;
    } else {
        // calc SAD at Predict (intpel acc)
        if (xPos != 0 || yPos != 0) {
            ippiSAD16x16_8u32s(pCur, mStepLuma, pRef+yPos*mStepLuma+xPos, mStepLuma, &sadL, IPPVC_MC_APX_FF);
            if (sadL - SAD_FAVOR_PRED < *bestSAD) {
                // choose Predict as origin for ME
                *bestSAD = sadL;
            } else {
                // choose (0,0) as origin for ME
                xPos = yPos = 0;
            }
        }
    }

    xL = IPP_MAX(xL, xPos - mPPicsearchHor);
    xR = IPP_MIN(xR, xPos + mPPicsearchHor);
    yT = IPP_MAX(yT, yPos - mPPicsearchVer);
    yB = IPP_MIN(yB, yPos + mPPicsearchVer);

    *bestSAD -= SAD_FAVOR_ZERO;
    sadL = *bestSAD;
#ifdef ME_USE_THRESHOLD
    if (*bestSAD > mMEthrSAD16x16) {
#endif
        if (mMEalgorithm == 0)
            h263e_ME_FullSearch_SAD_16x16(pCur, pRef, step, xL, xR, yT, yB, &xPos, &yPos, bestSAD, mMEthrSAD16x16);
        else
            h263e_ME_DiamondSearch_SAD_16x16(pCur, pRef, step, xL, xR, yT, yB, &xPos, &yPos, bestSAD, mMEfastSAD, mMEthrSAD16x16);
#ifdef ME_USE_THRESHOLD
    }
    if (mMEaccuracy == 1 || *bestSAD <= mMEthrSAD16x16) {
        mv->dx = (Ipp16s)(xPos << 1);
        mv->dy = (Ipp16s)(yPos << 1);
        if (mMEaccuracy > 2) {
            // quarter pel
            mv->dx = (Ipp16s)(mv->dx << 1);
            mv->dy = (Ipp16s)(mv->dy << 1);
        }
#else
    if (mMEaccuracy == 1) {
        mv->dx = (Ipp16s)(xPos << 1);
        mv->dy = (Ipp16s)(yPos << 1);
#endif
    } else {
        if (mMEalgorithm == 1 && mMEfastHP)
            h263e_ME_HalfPel_SAD_Fast_16x16(pCur, pRef, step, xL, xR, yT, yB, xPos, yPos, bestSAD, mv, mMEfastSAD);
        else
            h263e_ME_HalfPel_SAD_16x16(pCur, pRef, step, xL, xR, yT, yB, xPos, yPos, bestSAD, mv, mMEthrSAD16x16);
    }
    if (*bestSAD == sadL)
        *bestSAD += SAD_FAVOR_ZERO;
}

static void h263e_ME_HalfPel_SAD_8x8(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                     Ipp32s yT, Ipp32s yB, Ipp32s xPos, Ipp32s yPos, Ipp32s *bestSAD,
                                     IppMotionVector *mv, Ipp32s thrSAD_8x8)
{
    Ipp32s  cSAD, bSAD, i, j;

    i = 0;  j = 0;  bSAD = *bestSAD;
    if (xPos > xL && yPos > yT) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+(yPos-1)*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i = -1;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (yPos > yT) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+(yPos-1)*step+xPos, step, &cSAD, IPPVC_MC_APX_FH);
        if (cSAD < bSAD) {
            i = -1;  j =  0;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (xPos < xR && yPos > yT) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+(yPos-1)*step+xPos, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i = -1;  j =  1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (xPos > xL) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+yPos*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HF);
        if (cSAD < bSAD) {
            i =  0;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (xPos < xR) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_HF);
        if (cSAD < bSAD) {
            i =  0;  j =  1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (xPos > xL && yPos < yB) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+yPos*step+xPos-1, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i =  1;  j = -1;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (yPos < yB) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_FH);
        if (cSAD < bSAD) {
            i =  1;  j =  0;  bSAD = cSAD;
#ifdef ME_USE_THRESHOLD
            if (bSAD <= thrSAD_8x8) goto L_01;
#endif
        }
    }
    if (xPos < xR && yPos < yB) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+yPos*step+xPos, step, &cSAD, IPPVC_MC_APX_HH);
        if (cSAD < bSAD) {
            i =  1;  j =  1;  bSAD = cSAD;
        }
    }
#ifdef ME_USE_THRESHOLD
L_01:
#endif
    mv->dx = (Ipp16s)((xPos << 1) + j);
    mv->dy = (Ipp16s)((yPos << 1) + i);
    *bestSAD = bSAD;
}

static void h263e_ME_HalfPel_SAD_Fast_8x8(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR,
                                          Ipp32s yT, Ipp32s yB, Ipp32s xPos, Ipp32s yPos, Ipp32s *bSad,
                                          IppMotionVector *mv, Ipp32s *sad, Ipp32s xP, Ipp32s yP)
{
    Ipp32s  i, j;
    Ipp32s  m0, m1, m2, m3, m4;

    m0 = *bSad;
    j = xPos;
    i = yPos - 1;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        if (j >= xP - rangeME_8x8 && j <= xP + rangeME_8x8 && i >= yP - rangeME_8x8 && i <= yP + rangeME_8x8)
            m1 = sad[(i - yP + rangeME_8x8) * (rangeME_8x8*2+1) + j - xP + rangeME_8x8];
        else
            ippiSAD8x8_8u32s_C1R(pCur, step, pRef+i*step+j, step, &m1, IPPVC_MC_APX_FF);
    } else
        m1 = 1000000;
    j = xPos;
    i = yPos + 1;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        if (j >= xP - rangeME_8x8 && j <= xP + rangeME_8x8 && i >= yP - rangeME_8x8 && i <= yP + rangeME_8x8)
            m2 = sad[(i - yP + rangeME_8x8) * (rangeME_8x8*2+1) + j - xP + rangeME_8x8];
        else
            ippiSAD8x8_8u32s_C1R(pCur, step, pRef+i*step+j, step, &m2, IPPVC_MC_APX_FF);
    } else
        m2 = 1000000;
    j = xPos - 1;
    i = yPos;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        if (j >= xP - rangeME_8x8 && j <= xP + rangeME_8x8 && i >= yP - rangeME_8x8 && i <= yP + rangeME_8x8)
            m3 = sad[(i - yP + rangeME_8x8) * (rangeME_8x8*2+1) + j - xP + rangeME_8x8];
        else
            ippiSAD8x8_8u32s_C1R(pCur, step, pRef+i*step+j, step, &m3, IPPVC_MC_APX_FF);
    } else
        m3 = 1000000;
    j = xPos + 1;
    i = yPos;
    if (j >= xL && j <= xR && i >= yT && i <= yB) {
        if (j >= xP - rangeME_8x8 && j <= xP + rangeME_8x8 && i >= yP - rangeME_8x8 && i <= yP + rangeME_8x8)
            m4 = sad[(i - yP + rangeME_8x8) * (rangeME_8x8*2+1) + j - xP + rangeME_8x8];
        else
            ippiSAD8x8_8u32s_C1R(pCur, step, pRef+i*step+j, step, &m4, IPPVC_MC_APX_FF);
    } else
        m4 = 1000000;
    if (2*(m3-m0) < (m4-m0)) {
        j = -1;
    } else if ((m3-m0) > 2*(m4-m0)) {
        j = +1;
    } else
        j = 0;
    if (2*(m1-m0) < (m2-m0)) {
        i = -1;
    } else if ((m1-m0) > 2*(m2-m0)) {
        i = +1;
    } else
        i = 0;
    mv->dx = (Ipp16s)((xPos << 1) + j);
    mv->dy = (Ipp16s)((yPos << 1) + i);
    if (j != 0 || i != 0) {
        ippiSAD8x8_8u32s_C1R(pCur, step, pRef+h263e_MC_offs(mv, step), step, bSad, h263e_MC_type(mv));
        if (*bSad > m0) {
            // false choosing
            mv->dx = (Ipp16s)(xPos << 1);
            mv->dy = (Ipp16s)(yPos << 1);
            *bSad = m0;
        }
    }
}

#define FIND_MV_LIMITS_UMV1(mvPred, x, y) \
{ \
  if (mvPred.dx <= -32) { \
    xL = -IPP_MIN(x * 16 + 16, 31); \
    xR = 0; \
  } else if (mvPred.dx <= 32) { \
    xL = -IPP_MIN(x * 16 + 16, (32 - mvPred.dx) >> 1); \
    xR = IPP_MIN((mNumMacroBlockPerRow - x - 1) * 16 + 15, (mvPred.dx + 31) >> 1); \
  } else { \
    xL = 0; \
    xR = IPP_MIN((mNumMacroBlockPerRow - x - 1) * 16 + 15, 31); \
  } \
  if (mvPred.dy <= -32) { \
    yT = -IPP_MIN(y * 16 + 16, 31); \
    yB = 0; \
  } else if (mvPred.dy <= 32) { \
    yT = -IPP_MIN(y * 16 + 16, (32 - mvPred.dy) >> 1); \
    yB = IPP_MIN((mNumMacroBlockPerCol - y - 1) * 16 + 15, (mvPred.dy + 31) >> 1); \
  } else { \
    yT = 0; \
    yB = IPP_MIN((mNumMacroBlockPerCol - y - 1) * 16 + 15, 31); \
  } \
}

void ippVideoEncoderH263::ME_SAD_8x8(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL0, Ipp32s xR0,
                                     Ipp32s yT0, Ipp32s yB0, Ipp32s *bestSAD, IppMotionVector *mv,
                                     IppMotionVector *mv4, Ipp32s x, Ipp32s y, IppMotionVector *mvNeighb)
{
    Ipp32s xPos[4], yPos[4], bSad[4], sad[4][(rangeME_8x8*2+1)*(rangeME_8x8*2+1)], pOff[4], xP[4], yP[4];
    IppMotionVector mvPred;
    Ipp32s i, j, k, n, iPos, jPos;
    Ipp32s xL, xR, yT, yB;
#ifdef ME_USE_THRESHOLD
    Ipp32s  sadThresh[4] = {0, 0, 0, 0};
#endif

    xP[0] = h263e_Div2(mv->dx);
    yP[0] = h263e_Div2(mv->dy);
    xL = xL0; xR = xR0; yT = yT0; yB = yB0;

    bSad[0] = bSad[1] = bSad[2] = bSad[3] = MAX_SAD;
    pOff[0] = 0;
    pOff[1] = 8;
    pOff[2] = step * 8;
    pOff[3] = step * 8 + 8;
    if (mMEfastHP)
        memset(sad, -1, 4*(rangeME_8x8*2+1)*(rangeME_8x8*2+1)*sizeof(Ipp32s));
    for (n = 0; n < 4; n ++) {
        k = 0;

        if (n) {
          if (n == 1) {
            mvPred.dx = h263e_Median(xPos[0] << 1, mvNeighb[1].dx, mvNeighb[2].dx);
            mvPred.dy = h263e_Median(yPos[0] << 1, mvNeighb[1].dy, mvNeighb[2].dy);
          } else if (n == 2) {
            mvPred.dx = h263e_Median(xPos[0] << 1, xPos[1] << 1, mvNeighb[3].dx);
            mvPred.dy = h263e_Median(yPos[0] << 1, yPos[1] << 1, mvNeighb[3].dy);
          } else {
            mvPred.dx = h263e_Median(xPos[0] << 1, xPos[1] << 1, xPos[2] << 1);
            mvPred.dy = h263e_Median(yPos[0] << 1, yPos[1] << 1, yPos[2] << 1);
          }
          xP[n] = h263e_Div2(mvPred.dx);
          yP[n] = h263e_Div2(mvPred.dy);
          if (mVideoPicture.UMV == 1)
            FIND_MV_LIMITS_UMV1(mvPred, x, y);
        }

        for (iPos = -rangeME_8x8; iPos <= rangeME_8x8; iPos ++) {
            i = yP[n] + iPos;
            if (i >= yT && i <= yB) {
                for (jPos = -rangeME_8x8; jPos <= rangeME_8x8; jPos ++) {
                    j = xP[n] + jPos;
                    if (j >= xL && j <= xR) {
                        ippiSAD8x8_8u32s_C1R(pCur+pOff[n], step, pRef+pOff[n]+i*step+j, step, &sad[n][k], IPPVC_MC_APX_FF);
                        if (bSad[n] > sad[n][k]) {
                            bSad[n] = sad[n][k];
                            xPos[n] = j;
                            yPos[n] = i;
#ifdef ME_USE_THRESHOLD
                            if (sad[n][k] <= mMEthrSAD8x8) {
                                sadThresh[n] = 1;
                                goto L_1;
                            }
#endif
                        }
                    }
                    k ++;
                }
            } else
                k += (rangeME_8x8*2+1);
        }
#ifdef ME_USE_THRESHOLD
L_1:;
#endif
    }
    if (mMEaccuracy == 1) {
        for (n = 0; n < 4; n ++) {
            mv4[n].dx = (Ipp16s)(xPos[n] << 1);
            mv4[n].dy = (Ipp16s)(yPos[n] << 1);
        }
    } else {

        xL = xL0; xR = xR0; yT = yT0; yB = yB0;

        if (mMEalgorithm == 1 && mMEfastHP) {
            for (n = 0; n < 4; n ++) {
              if (mVideoPicture.UMV == 1 && n) {
                if (n == 1) {
                  mvPred.dx = h263e_Median(mv4[0].dx, mvNeighb[1].dx, mvNeighb[2].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mvNeighb[1].dy, mvNeighb[2].dy);
                } else if (n == 2) {
                  mvPred.dx = h263e_Median(mv4[0].dx, mv4[1].dx, mvNeighb[3].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mv4[1].dy, mvNeighb[3].dy);
                } else {
                  mvPred.dx = h263e_Median(mv4[0].dx, mv4[1].dx, mv4[2].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mv4[1].dy, mv4[2].dy);
                }
                FIND_MV_LIMITS_UMV1(mvPred, x, y);
              }

              h263e_ME_HalfPel_SAD_Fast_8x8(pCur+pOff[n], pRef+pOff[n], step, xL, xR, yT, yB,
                                            xPos[n], yPos[n], &bSad[n], &mv4[n], sad[n], xP[n], yP[n]);
            }
        } else
            for (n = 0; n < 4; n ++) {

              if (mVideoPicture.UMV == 1 && n) {
                if (n == 1) {
                  mvPred.dx = h263e_Median(mv4[0].dx, mvNeighb[1].dx, mvNeighb[2].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mvNeighb[1].dy, mvNeighb[2].dy);
                } else if (n == 2) {
                  mvPred.dx = h263e_Median(mv4[0].dx, mv4[1].dx, mvNeighb[3].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mv4[1].dy, mvNeighb[3].dy);
                } else {
                  mvPred.dx = h263e_Median(mv4[0].dx, mv4[1].dx, mv4[2].dx);
                  mvPred.dy = h263e_Median(mv4[0].dy, mv4[1].dy, mv4[2].dy);
                }
                FIND_MV_LIMITS_UMV1(mvPred, x, y);
              }

#ifdef ME_USE_THRESHOLD
                if (!sadThresh[n])
                    h263e_ME_HalfPel_SAD_8x8(pCur+pOff[n], pRef+pOff[n], step, xL, xR, yT, yB, xPos[n], yPos[n], &bSad[n], &mv4[n], mMEthrSAD8x8);
                else {
                    mv4[n].dx = (Ipp16s)(xPos[n] << 1);
                    mv4[n].dy = (Ipp16s)(yPos[n] << 1);
                }
#else
                h263e_ME_HalfPel_SAD_8x8(pCur+pOff[n], pRef+pOff[n], step, xL, xR, yT, yB, xPos[n], yPos[n], &bSad[n], &mv4[n], mMEthrSAD8x8);
#endif
            }
    }
    *bestSAD = bSad[0] + bSad[1] + bSad[2] + bSad[3];
}

#ifdef ME_USE_THRESHOLD
#define SAD_ZERO mMEthrSAD16x16
#else
#define SAD_ZERO 0
#endif

/*
void ippVideoEncoderH263::Encode_RepeatedPPic()
{
  for (gn = 0; gn < mVideoPicture.num_gobs_in_pic; gn++) {
    mRTPdata.GOBstartPos[gn] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;

    if (mGOBheaders && gn) {
      EncodeGOB_Header(gn);
      frGOB = gn*mVideoPicture.num_MBrows_in_gob;
    }

    for (gRow = 0; gRow < mVideoPicture.num_MBrows_in_gob; gRow++) {
      Ipp32s boundOnTop;
      i = gn*mVideoPicture.num_MBrows_in_gob + gRow;

      boundOnTop = (i == frGOB && i);

      pYc = mFrameC->pY + i * 16 * mStepLuma;  pYf = mFrameF->pY + i * 16 * mStepLuma;
      pUc = mFrameC->pU + i * 8 * mStepChroma;  pUf = mFrameF->pU + i * 8 * mStepChroma;
      pVc = mFrameC->pV + i * 8 * mStepChroma;  pVf = mFrameF->pV + i * 8 * mStepChroma;
      for (j = 0; j < mNumMacroBlockPerRow; j ++) {

}
*/

void ippVideoEncoderH263::EncodePPic()
{
  __ALIGN16(Ipp16s, coeffMB, 64*10);
  __ALIGN16(Ipp8u, mcPred, 64*6);
  Ipp8u  *pYc, *pUc, *pVc, *pYf, *pUf, *pVf, *pF[6];
  Ipp32s i, j, quant, pattern, xL, xR, yT, yB;
  Ipp32s nzCount[6], dev, bestSAD, sadU, sadV, sadL0, sadL1, sadL2, sadL3, ncThrL, ncThrC, sadFavorInter;
  Ipp32s bestSAD16x16, bestSAD8x8 = MAX_SAD;
  h263e_MacroBlock *MBcurr = mMBinfo;
  Ipp32s scan;
  Ipp32s maxMVx, maxMVy;
  IppMotionVector mvLuma4[4], mvChroma, mvPred[4];
  Ipp32s frGOB = 0;
  Ipp32s rt = mVideoPicture.pic_rounding_type;
  IppiRect limitRectL, limitRectC;
  IppMotionVector mvLuma;
  Ipp32s nmbf;
  Ipp32s gn, gRow;

  mME4mv = mVideoPicture.advPred | mVideoPicture.deblockFilt;

#ifdef ME_USE_THRESHOLD
  // setup thresholds for ME
  mMEthrSAD16x16 = mVideoPicture.pic_quant >= 6 ? 256 : (4 << mVideoPicture.pic_quant);
  mMEthrSAD8x8 = mMEthrSAD16x16 >> 2;
  mMEthrSAD16x8 = mMEthrSAD16x16 >> 1;
#endif
// encode short_video_header P-mVideoPicture

  limitRectL.x = -16;
  limitRectL.y = -16;
  limitRectL.width = mNumMacroBlockPerRow * 16 + 32;
  limitRectL.height = mNumMacroBlockPerCol * 16 + 32;
  limitRectC.x = -8;
  limitRectC.y = -8;
  limitRectC.width = mNumMacroBlockPerRow * 8 + 16;
  limitRectC.height = mNumMacroBlockPerCol * 8 + 16;

  if (mVideoPicture.UMV == 2) {
    if (mVideoPicture.pic_width <= 352)
      maxMVx = 16;
    else if (mVideoPicture.pic_width <= 704)
      maxMVx = 32;
    else if (mVideoPicture.pic_width <= 1408)
      maxMVx = 64;
    else
      maxMVx = 128;

    if (mVideoPicture.pic_height <= 288)
      maxMVy = 16;
    else if (mVideoPicture.pic_width <= 576)
      maxMVy = 32;
    else
      maxMVy = 64;
  }

  quant = mVideoPicture.pic_quant;
  MBcurr = mMBinfo;

  for (gn = 0; gn < mVideoPicture.num_gobs_in_pic; gn++) {

    if (mGOBheaders)
      frGOB = gn*mVideoPicture.num_MBrows_in_gob;

    for (gRow = 0; gRow < mVideoPicture.num_MBrows_in_gob; gRow++) {
      i = gn*mVideoPicture.num_MBrows_in_gob + gRow;


//  for (i = 0; i < mNumMacroBlockPerCol; i ++) {
      pYc = mFrameC->pY + i * 16 * mStepLuma;  pYf = mFrameF->pY + i * 16 * mStepLuma;
      pUc = mFrameC->pU + i * 8 * mStepChroma;  pUf = mFrameF->pU + i * 8 * mStepChroma;
      pVc = mFrameC->pV + i * 8 * mStepChroma;  pVf = mFrameF->pV + i * 8 * mStepChroma;
      if (!mVideoPicture.UMV) {
        yT = (i | mVideoPicture.advPred) ? -16 : 0;
        yB = ((mNumMacroBlockPerCol - i - 1) | mVideoPicture.advPred) ? 15 : 0;
      } else if (mVideoPicture.UMV == 2) {
        yT = -IPP_MIN(i * 16 + 15, maxMVy);
        yB = IPP_MIN((mNumMacroBlockPerCol - i - 1)*16 + 15, maxMVy - 1);
      } else if (mVideoPicture.UMV == 3) {
        yT = -(i * 16 + 15);
        yB = (mNumMacroBlockPerCol - i - 1)*16 + 15;
      }
      for (j = 0; j < mNumMacroBlockPerRow; j ++) {
          // calc SAD for Y, U,V blocks at (0,0)
          ippiSAD8x8_8u32s_C1R(pYc, mStepLuma, pYf, mStepLuma, &sadL0, IPPVC_MC_APX_FF);
          ippiSAD8x8_8u32s_C1R(pYc+8, mStepLuma, pYf+8, mStepLuma, &sadL1, IPPVC_MC_APX_FF);
          ippiSAD8x8_8u32s_C1R(pYc+8*mStepLuma, mStepLuma, pYf+8*mStepLuma, mStepLuma, &sadL2, IPPVC_MC_APX_FF);
          ippiSAD8x8_8u32s_C1R(pYc+8*mStepLuma+8, mStepLuma, pYf+8*mStepLuma+8, mStepLuma, &sadL3, IPPVC_MC_APX_FF);
          ippiSAD8x8_8u32s_C1R(pUc, mStepChroma, pUf, mStepChroma, &sadU, IPPVC_MC_APX_FF);
          ippiSAD8x8_8u32s_C1R(pVc, mStepChroma, pVf, mStepChroma, &sadV, IPPVC_MC_APX_FF);
          // not_coded decision
          ncThrL = quant * SAD_NOTCODED_THR_LUMA;
          ncThrC = quant * SAD_NOTCODED_THR_CHROMA;
          if ((sadL0 < ncThrL) && (sadL1 < ncThrL) && (sadL2 < ncThrL) && (sadL3 < ncThrL) && (sadU < ncThrC) && (sadV < ncThrC)) {
              MBcurr->type = IPPVC_MBTYPE_INTER; // ??? IPPVC_MB_STUFFING
              MBcurr->not_coded = 1;
              MBcurr->mv[0].dx = MBcurr->mv[0].dy = 0;
              if (mME4mv)
                MBcurr->mv[1].dx = MBcurr->mv[1].dy = MBcurr->mv[2].dx = MBcurr->mv[2].dy = MBcurr->mv[3].dx = MBcurr->mv[3].dy = 0;
          } else {
              MBcurr->not_coded = 0;
              // SAD at (0,0)
              bestSAD = sadL0 + sadL1 + sadL2 + sadL3;

              PredictMV(MBcurr, frGOB, i, j, mME4mv, &mvLuma);

              if (!mVideoPicture.UMV) {
                xL = (j | mVideoPicture.advPred) ? -16 : 0;
                xR = ((mNumMacroBlockPerRow - j - 1) | mVideoPicture.advPred) ? 15 : 0;
              } else if (mVideoPicture.UMV == 1) {
                // ??? To pad with 32 for UMV w/out PLUSPTYPE: can save some bits on MV coding at borders when predictors are large ???

                if (mvLuma.dx <= -32) {
                  xL = -IPP_MIN(j * 16 + 16, 31);
                  xR = 0;
                } else if (mvLuma.dx <= 32) {
                  xL = -IPP_MIN(j * 16 + 16, (32 - mvLuma.dx) >> 1);
                  xR = IPP_MIN((mNumMacroBlockPerRow - j - 1) * 16 + 15, (mvLuma.dx + 31) >> 1);
                } else {
                  xL = 0;
                  xR = IPP_MIN((mNumMacroBlockPerRow - j - 1) * 16 + 15, 31);
                }
                if (mvLuma.dy <= -32) {
                  yT = -IPP_MIN(i * 16 + 16, 31);
                  yB = 0;
                } else if (mvLuma.dy <= 32) {
                  yT = -IPP_MIN(i * 16 + 16, (32 - mvLuma.dy) >> 1);
                  yB = IPP_MIN((mNumMacroBlockPerCol - i - 1) * 16 + 15, (mvLuma.dy + 31) >> 1);
                } else {
                  yT = 0;
                  yB = IPP_MIN((mNumMacroBlockPerCol - i - 1) * 16 + 15, 31);
                }
              } else if (mVideoPicture.UMV == 2) {
                xL = -IPP_MIN(j * 16 + 15, maxMVx);
                xR = IPP_MIN((mNumMacroBlockPerRow - j - 1)*16 + 15, maxMVx - 1);
              } else { // unlimited MV, UUI == 01
                xL = -(j * 16 + 15);
                xR = (mNumMacroBlockPerRow - j - 1)*16 + 15;
              }

              mvPred[0] = mvLuma;
              ME_SAD_16x16(pYc, pYf, mStepLuma, xL, xR, yT, yB, &bestSAD, &mvLuma);
              MBcurr->type = IPPVC_MBTYPE_INTER;

              if (mME4mv) {
                bestSAD16x16 = bestSAD - SAD_FAVOR_16x16;
                if (bestSAD16x16 > SAD_ZERO) {

//                    if (mVideoPicture.UMV == 1)
                  me4MV_Neighbours(MBcurr, frGOB, i, j, mvPred);
                  ME_SAD_8x8(pYc, pYf, mStepLuma, xL, xR, yT, yB, &bestSAD8x8, &mvLuma, MBcurr->mv, j, i, mvPred);
                }
                if (bestSAD16x16 < bestSAD8x8) {
                  MBcurr->mv[0] = MBcurr->mv[1] = MBcurr->mv[2] = MBcurr->mv[3] = mvLuma;
                } else {
                  bestSAD = bestSAD8x8;
                  MBcurr->type = IPPVC_MBTYPE_INTER4V;
                }
              } else
                MBcurr->mv[0] = mvLuma;

//                ME_SAD_16x16(pYc, pYf, mStepLuma, xL, xR, yT, yB, &bestSAD, &mvLuma);

              //sadFavorInter = SAD_FAVOR_INTER;
              sadFavorInter = quant << 5; // ??? SAD_FAVOR_INTER ?
              dev = MAX_SAD;
              if (bestSAD > sadFavorInter)
                ippiMeanAbsDev16x16_8u32s_C1R(pYc, mStepLuma, &dev);

              // ??? not present in H.263 App.III

              if (j > 0)
                if (MBcurr[-1].type == IPPVC_MBTYPE_INTRA)
                    dev -= DEV_FAVOR_INTRA;
              if (i > 0)
                if (MBcurr[-mNumMacroBlockPerRow].type == IPPVC_MBTYPE_INTRA)
                    dev -= DEV_FAVOR_INTRA;

              // ??? done before 8x8 ME in H.263 App.III
              if (dev < bestSAD - sadFavorInter) {
                // intra coded
                MBcurr->type = IPPVC_MBTYPE_INTRA;
                MBcurr->mv[0].dx = MBcurr->mv[0].dy = 0;
                if (mME4mv) // ???
                  MBcurr->mv[1].dx = MBcurr->mv[1].dy = MBcurr->mv[2].dx = MBcurr->mv[2].dy = MBcurr->mv[3].dx = MBcurr->mv[3].dy = 0;
                mNumIntraMB ++;
                if (mNumIntraMB > mSceneChangeThreshold)
                  return;
              } else
                MBcurr->lumaErr = bestSAD;
          }
          MBcurr ++;
          pYc += 16; pUc += 8; pVc += 8;
          pYf += 16; pUf += 8; pVf += 8;
      }
    }
  }
  quant = mVideoPicture.pic_quant;
  mVideoPicture.gob_number = 1;
  nmbf = 0;
  MBcurr = mMBinfo;
  frGOB = 0;

  for (gn = 0; gn < mVideoPicture.num_gobs_in_pic; gn++) {
    mRTPdata.GOBstartPos[gn] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;

    if (mGOBheaders && gn) {
      EncodeGOB_Header(gn);
      frGOB = gn*mVideoPicture.num_MBrows_in_gob;
    }

    for (gRow = 0; gRow < mVideoPicture.num_MBrows_in_gob; gRow++) {
      Ipp32s boundOnTop;
      i = gn*mVideoPicture.num_MBrows_in_gob + gRow;

      boundOnTop = (i == frGOB && i);

      pYc = mFrameC->pY + i * 16 * mStepLuma;  pYf = mFrameF->pY + i * 16 * mStepLuma;
      pUc = mFrameC->pU + i * 8 * mStepChroma;  pUf = mFrameF->pU + i * 8 * mStepChroma;
      pVc = mFrameC->pV + i * 8 * mStepChroma;  pVf = mFrameF->pV + i * 8 * mStepChroma;
      for (j = 0; j < mNumMacroBlockPerRow; j ++) {
        // for RTP support
        mRTPdata.MBpos[nmbf] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
        mRTPdata.MBquant[nmbf] = (Ipp8u)quant;
        if (MBcurr->not_coded) {
          MBcurr->block[0].validPredIntra = MBcurr->block[1].validPredIntra = MBcurr->block[2].validPredIntra =
          MBcurr->block[3].validPredIntra = MBcurr->block[4].validPredIntra = MBcurr->block[5].validPredIntra = 0;
          // encode not_coded
          cBS.PutBits(1, 1);
          if (!mVideoPicture.advPred) {
            if (mCalcPSNR) {
              mPSNR_Y += h263e_CalcMSE_16x16(pYf, mStepLuma, pYc, mStepLuma);
              mPSNR_U += h263e_CalcMSE_8x8(pUf, mStepChroma, pUc, mStepChroma);
              mPSNR_V += h263e_CalcMSE_8x8(pVf, mStepChroma, pVc, mStepChroma);
            }
            ippiCopy16x16_8u_C1R(pYf, mStepLuma, pYc, mStepLuma);
          } else {
            if (mCalcPSNR) {
              ippiCopy16x16_8u_C1R(pYc, mStepLuma, mcPred, 16);
              OBMC_Macroblock(MBcurr, MBcurr->mv, j, i, limitRectL, pYf, mStepLuma, pYc, mStepLuma);
              mPSNR_Y += h263e_CalcMSE_16x16(mcPred, 16, pYc, mStepLuma);
              mPSNR_U += h263e_CalcMSE_8x8(pUf, mStepChroma, pUc, mStepChroma);
              mPSNR_V += h263e_CalcMSE_8x8(pVf, mStepChroma, pVc, mStepChroma);
            } else
              OBMC_Macroblock(MBcurr, MBcurr->mv, j, i, limitRectL, pYf, mStepLuma, pYc, mStepLuma);
          }
          ippiCopy8x8_8u_C1R(pUf, mStepChroma, pUc, mStepChroma);
          ippiCopy8x8_8u_C1R(pVf, mStepChroma, pVc, mStepChroma);
        } else {
          if (MBcurr->type == IPPVC_MBTYPE_INTRA) {
            MBcurr->block[0].validPredIntra = MBcurr->block[1].validPredIntra = MBcurr->block[2].validPredIntra =
            MBcurr->block[3].validPredIntra = MBcurr->block[4].validPredIntra = MBcurr->block[5].validPredIntra = 1;
            // encode not_coded
            cBS.PutBits(0, 1);

            DCT8x8MacroBlock_H263(pYc, pUc, pVc, coeffMB);

            if (mVideoPicture.advIntra) {
              pF[0] = pYc; pF[1] = pYc + 8; pF[2] = pYc + 8*mStepLuma; pF[3] = pYc + 8*mStepLuma + 8;
              pF[4] = pUc; pF[5] = pVc;

              if (boundOnTop) { // mark top row blocks as invalid
                MBcurr->block[0].predC->validPredIntra = 0;
                MBcurr->block[1].predC->validPredIntra = 0;
                MBcurr->block[4].predC->validPredIntra = 0;
                MBcurr->block[5].predC->validPredIntra = 0;
              }

              ChooseAdvIntraPred(MBcurr, coeffMB, &scan);
              pattern = PredictReconstructAdvIntra(pF, MBcurr, coeffMB, nzCount, quant, scan);
            } else {
              scan = IPPVC_SCAN_ZIGZAG;
              pattern = QuantMacroBlockIntra_H263(coeffMB, nzCount, quant);
            }

            EncodeMCBPC_P(IPPVC_MBTYPE_INTRA, pattern & 3);

            if (mVideoPicture.advIntra)
              EncodeAdvIntraPredMode(scan);

            EncodeCBPY_P(IPPVC_MBTYPE_INTRA, pattern >> 2);
            EncodeMacroBlockIntra_H263(coeffMB, pattern, nzCount, scan);

            if (!mVideoPicture.advIntra)
              ReconMacroBlockIntra_H263(pYc, pUc, pVc, coeffMB, quant, pattern);
          } else {
            MBcurr->block[0].validPredIntra = MBcurr->block[1].validPredIntra = MBcurr->block[2].validPredIntra =
            MBcurr->block[3].validPredIntra = MBcurr->block[4].validPredIntra = MBcurr->block[5].validPredIntra = 0;
            mvLuma4[0] = MBcurr->mv[0];
            if (mME4mv) {
              mvLuma4[1] = MBcurr->mv[1];  mvLuma4[2] = MBcurr->mv[2];  mvLuma4[3] = MBcurr->mv[3];
            }
            // find Luma Pred
            if (!mVideoPicture.advPred) {
              if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
                h263e_Copy8x8HP_8u(pYf, mStepLuma, mcPred, 16, &mvLuma4[0], rt);
                h263e_Copy8x8HP_8u(pYf+8, mStepLuma, mcPred+8, 16, &mvLuma4[1], rt);
                h263e_Copy8x8HP_8u(pYf+8*mStepLuma, mStepLuma, mcPred+16*8, 16, &mvLuma4[2], rt);
                h263e_Copy8x8HP_8u(pYf+8*mStepLuma+8, mStepLuma, mcPred+16*8+8, 16, &mvLuma4[3], rt);
              } else
                h263e_Copy16x16HP_8u(pYf, mStepLuma, mcPred, 16, &mvLuma4[0], rt);
            } else {

              OBMC_Macroblock(MBcurr, mvLuma4, j, i, limitRectL, pYf, mStepLuma, mcPred, 16);
/*
              IppMotionVector *mvRight, *mvLeft, *mvUpper;
              mvRight = (j == mNumMacroBlockPerRow - 1) ? &mvLuma4[1] : (MBcurr[1].type == IPPVC_MBTYPE_INTRA || MBcurr[1].type == IPPVC_MBTYPE_INTRA_Q) ? &mvLuma4[1] : MBcurr[1].mv;
              mvLeft = (j == 0) ? mvLuma4 - 1 : (MBcurr[-1].type == IPPVC_MBTYPE_INTRA || MBcurr[-1].type == IPPVC_MBTYPE_INTRA_Q) ? mvLeft = mvLuma4 - 1 : MBcurr[-1].mv;
              mvUpper = (i == 0) ? mvLuma4 - 2 : (MBcurr[-mNumMacroBlockPerRow].type == IPPVC_MBTYPE_INTRA || MBcurr[-mNumMacroBlockPerRow].type == IPPVC_MBTYPE_INTRA_Q) ? mvLuma4 - 2 : MBcurr[-mNumMacroBlockPerRow].mv;
              ippiOBMC8x8HP_MPEG4_8u_C1R(pYf, mStepLuma, mcPred, 16, &mvLuma4[0], &mvLeft[1], &mvLuma4[1], &mvUpper[2], &mvLuma4[2], rt);
              ippiOBMC8x8HP_MPEG4_8u_C1R(pYf+8, mStepLuma, mcPred+8, 16, &mvLuma4[1], &mvLuma4[0], &mvRight[0], &mvUpper[3], &mvLuma4[3], rt);
              ippiOBMC8x8HP_MPEG4_8u_C1R(pYf+mStepLuma*8, mStepLuma, mcPred+16*8, 16, &mvLuma4[2], &mvLeft[3], &mvLuma4[3], &mvLuma4[0], &mvLuma4[2], rt);
              ippiOBMC8x8HP_MPEG4_8u_C1R(pYf+mStepLuma*8+8, mStepLuma, mcPred+16*8+8, 16, &mvLuma4[3], &mvLuma4[2], &mvRight[2], &mvLuma4[1], &mvLuma4[3], rt);
*/
            }
            // calculate Chroma MV
            if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
              h263e_ComputeChroma4MV(mvLuma4, &mvChroma);
              h263e_LimitMV(&mvChroma, &mvChroma, &limitRectC, j*8, i*8, 8); // ???
            }
            else
              h263e_ComputeChromaMV(mvLuma4, &mvChroma);
            // find Chroma Pred
            h263e_Copy8x8HP_8u(pUf, mStepChroma, mcPred+64*4, 8, &mvChroma, rt);
            h263e_Copy8x8HP_8u(pVf, mStepChroma, mcPred+64*5, 8, &mvChroma, rt);

            pattern = TransMacroBlockInter_H263(pYc, pUc, pVc, coeffMB, nzCount, quant, mcPred, MBcurr->lumaErr);
/*
            h263e_ComputeChromaMV(&mvLuma, &mvChroma);
            h263e_Copy16x16HP_8u(pYf, mStepLuma, mcPred, 16, &mvLuma, 0);
            h263e_Copy8x8HP_8u(pUf, mStepChroma, mcPred+64*4, 8, &mvChroma, 0);
            h263e_Copy8x8HP_8u(pVf, mStepChroma, mcPred+64*5, 8, &mvChroma, 0);
            pattern = TransMacroBlockInter_H263(pYc, pUc, pVc, coeffMB, nzCount, quant, mcPred, MBcurr->lumaErr);
            if (pattern == 0 && mvLuma.dx == 0 && mvLuma.dy == 0) {
*/
            if (pattern == 0 && mvLuma4[0].dx == 0 && mvLuma4[0].dy == 0 &&
              (!mME4mv || (mvLuma4[1].dx == 0 && mvLuma4[1].dy == 0 && mvLuma4[2].dx == 0 && mvLuma4[2].dy == 0 && mvLuma4[3].dx == 0 && mvLuma4[3].dy == 0))) {
              MBcurr->not_coded = 1;
              // encode not_coded
              cBS.PutBits(1, 1);
              ReconMacroBlockNotCoded(pYc, pUc, pVc, mcPred);
            } else {
              // ESC code restriction [-127;127]
              if (coeffMB[  0] < -127 || coeffMB[  0] > 127 ||
                coeffMB[ 64] < -127 || coeffMB[ 64] > 127 ||
                coeffMB[128] < -127 || coeffMB[128] > 127 ||
                coeffMB[192] < -127 || coeffMB[192] > 127 ||
                coeffMB[256] < -127 || coeffMB[256] > 127 ||
                coeffMB[320] < -127 || coeffMB[320] > 127) {
                // intra coded
                MBcurr->block[0].validPredIntra = MBcurr->block[1].validPredIntra = MBcurr->block[2].validPredIntra = MBcurr->block[3].validPredIntra = MBcurr->block[4].validPredIntra = MBcurr->block[5].validPredIntra = 1;
                MBcurr->type = IPPVC_MBTYPE_INTRA;
                MBcurr->mv[0].dx = MBcurr->mv[0].dy = 0;
                if (mME4mv)
                  MBcurr->mv[1].dx = MBcurr->mv[1].dy = MBcurr->mv[2].dx = MBcurr->mv[2].dy =
                  MBcurr->mv[3].dx = MBcurr->mv[3].dy =0;
                // encode not_coded
                cBS.PutBits(0, 1);

                DCT8x8MacroBlock_H263(pYc, pUc, pVc, coeffMB);

                if (mVideoPicture.advIntra) {
                  pF[0] = pYc; pF[1] = pYc + 8; pF[2] = pYc + 8*mStepLuma; pF[3] = pYc + 8*mStepLuma + 8;
                  pF[4] = pUc; pF[5] = pVc;

                  if (boundOnTop) {
                    MBcurr->block[0].predC->validPredIntra = 0;
                    MBcurr->block[1].predC->validPredIntra = 0;
                    MBcurr->block[4].predC->validPredIntra = 0;
                    MBcurr->block[5].predC->validPredIntra = 0;
                  }

                  ChooseAdvIntraPred(MBcurr, coeffMB, &scan);
                  pattern = PredictReconstructAdvIntra(pF, MBcurr, coeffMB, nzCount, quant, scan);
                } else {
                  scan = IPPVC_SCAN_ZIGZAG;
                  pattern = QuantMacroBlockIntra_H263(coeffMB, nzCount, quant);
                }

                EncodeMCBPC_P(IPPVC_MBTYPE_INTRA, pattern & 3);

                if (mVideoPicture.advIntra)
                  EncodeAdvIntraPredMode(scan);

                EncodeCBPY_P(IPPVC_MBTYPE_INTRA, pattern >> 2);
                EncodeMacroBlockIntra_H263(coeffMB, pattern, nzCount, scan);

                if (!mVideoPicture.advIntra)
                  ReconMacroBlockIntra_H263(pYc, pUc, pVc, coeffMB, quant, pattern);
              } else {
                PredictMV(MBcurr, frGOB, i, j, mME4mv, &mvPred[0]);
                // for RTP support
                mRTPdata.MBpredMV[nmbf] = mvPred[0];

                mvPred[0].dx = (Ipp16s)(mvLuma4[0].dx - mvPred[0].dx);
                mvPred[0].dy = (Ipp16s)(mvLuma4[0].dy - mvPred[0].dy);
                if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
                  Predict3MV(MBcurr, frGOB, i, j, mvPred, mvLuma4);
                  if (mVideoPicture.advPred)
                    mRTPdata.MBpredMV1[nmbf] = mvPred[2];
                  mvPred[1].dx = (Ipp16s)(mvLuma4[1].dx - mvPred[1].dx);
                  mvPred[1].dy = (Ipp16s)(mvLuma4[1].dy - mvPred[1].dy);
                  mvPred[2].dx = (Ipp16s)(mvLuma4[2].dx - mvPred[2].dx);
                  mvPred[2].dy = (Ipp16s)(mvLuma4[2].dy - mvPred[2].dy);
                  mvPred[3].dx = (Ipp16s)(mvLuma4[3].dx - mvPred[3].dx);
                  mvPred[3].dy = (Ipp16s)(mvLuma4[3].dy - mvPred[3].dy);

                }
                if (mVideoPicture.UMV < 2) {
                  h263e_MV_CheckRange(&mvPred[0], 0 /*mVideoPicture.UMV*/);
                  if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
                    h263e_MV_CheckRange(&mvPred[1], 0);
                    h263e_MV_CheckRange(&mvPred[2], 0);
                    h263e_MV_CheckRange(&mvPred[3], 0);
                  }
                }

                // encode not_coded
                cBS.PutBits(0, 1);
                EncodeMCBPC_P(MBcurr->type, pattern & 3);
                EncodeCBPY_P(MBcurr->type, pattern >> 2);
                EncodeMV(mvPred, MBcurr->type);
                EncodeMacroBlockInter_H263(coeffMB, pattern, nzCount);
                if (pattern)
                  ReconMacroBlockInter_H263(pYc, pUc, pVc, mcPred, coeffMB, quant, pattern);
                else
                  ReconMacroBlockNotCoded(pYc, pUc, pVc, mcPred);
              }
            }
          }
        }
        nmbf++;
        MBcurr++;
        pYc += 16; pUc += 8; pVc += 8;
        pYf += 16; pUf += 8; pVf += 8;
      }
    }
  }
  EncodeZeroBitsAlign();
}

#if 0
void ippVideoEncoderH263::EncodeBPic()
{
  __ALIGN16(Ipp16s, coeffMB, 64*10);
  __ALIGN16(Ipp8u, mcPredD, 64*6);
  __ALIGN16(Ipp8u, mcPredI, 64*6);
  __ALIGN16(Ipp8u, mcTmp, 64*6);
  Ipp8u          *pYc, *pUc, *pVc, *pYf, *pUf, *pVf, *pYb, *pUb, *pVb, *mcPred = 0;
  Ipp32s          i, j, xL, xR, yT, yB, pattern = 0;
  Ipp32s          nzCount[6], bestSADDirect, bestSAD16x16Inter, bestSAD16x16Forw, bestSAD16x16Back, bestSAD;
  IppMotionVector mvLumaForw, mvPredForw, mvForw, mvLumaBack, mvPredBack, mvBack, mvChromaF, mvChromaB, mvLumaForwDirect[4], mvDelta, mvLumaBackDirect[4];
  Ipp32s          modb = 0, mb_type, dbquant, quant, dct_type;
  Ipp32s          nBits, sBitoff = cBS.mBitOff, mbn = 0;
  Ipp8u          *sPtr = cBS.mPtr;
  h263e_MacroBlock *MBcurr = mMBinfo;
  Ipp32s          fRangeMinForw = -(16 << mVideoPicture.pic_fcode_forward), fRangeMaxForw = (16 << mVideoPicture.pic_fcode_forward) - 1, fRangeForw = fRangeMaxForw - fRangeMinForw + 1;
  Ipp32s          fRangeMinBack = -(16 << mVideoPicture.pic_fcode_forward), fRangeMaxBack = (16 << mVideoPicture.pic_fcode_forward) - 1, fRangeBack = fRangeMaxBack - fRangeMinBack + 1;

#ifdef ME_USE_THRESHOLD
  // setup thresholds for ME
  mMEthrSAD16x16 = mVideoPicture.pic_quant >= 6 ? 256 : (4 << mVideoPicture.pic_quant);
  mMEthrSAD8x8 = mMEthrSAD16x16 >> 2;
#endif
  dct_type = 0;
  quant = mVideoPicture.pic_quant;
  dbquant = 0;
  mvDelta.dx = mvDelta.dy = 0;
  for (i = 0; i < mNumMacroBlockPerCol; i++) {
    pYc = mFrameC->pY + i * 16 * mStepLuma;
    pUc = mFrameC->pU + i * 8 * mStepChroma;
    pVc = mFrameC->pV + i * 8 * mStepChroma;
    pYf = mFrameF->pY + i * 16 * mStepLuma;
    pUf = mFrameF->pU + i * 8 * mStepChroma;
    pVf = mFrameF->pV + i * 8 * mStepChroma;
    pYb = mFrameB->pY + i * 16 * mStepLuma;
    pUb = mFrameB->pU + i * 8 * mStepChroma;
    pVb = mFrameB->pV + i * 8 * mStepChroma;
    mvPredForw.dx = mvPredForw.dy = mvPredBack.dx = mvPredBack.dy = 0;
    for (j = 0; j < mNumMacroBlockPerRow; j++) {
      mb_type = -1;
      if (!MBcurr->not_coded) {
        // avoid warning
        mvLumaForw.dx = mvLumaForw.dy = mvLumaBack.dx = mvLumaBack.dy = 0;
        // Direct mode with MVd = 0
        if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
          for (Ipp32s nB = 0; nB < 4; nB ++) {
            mvLumaForwDirect[nB].dx = (Ipp16s)(mTRB * MBcurr->mv[nB].dx / mTRD);
            mvLumaForwDirect[nB].dy = (Ipp16s)(mTRB * MBcurr->mv[nB].dy / mTRD);
            mvLumaBackDirect[nB].dx = (Ipp16s)((mTRB - mTRD) * MBcurr->mv[nB].dx / mTRD);
            mvLumaBackDirect[nB].dy = (Ipp16s)((mTRB - mTRD) * MBcurr->mv[nB].dy / mTRD);
          }
          h263e_Copy8x8HP_8u(pYf, mStepLuma, mcPredD, 16, &mvLumaForwDirect[0], 0);
          h263e_Copy8x8HP_8u(pYf+8, mStepLuma, mcPredD+8, 16, &mvLumaForwDirect[1], 0);
          h263e_Copy8x8HP_8u(pYf+8*mStepLuma, mStepLuma, mcPredD+8*16, 16, &mvLumaForwDirect[2], 0);
          h263e_Copy8x8HP_8u(pYf+8*mStepLuma+8, mStepLuma, mcPredD+8*16+8, 16, &mvLumaForwDirect[3], 0);
          h263e_Copy8x8HP_8u(pYb, mStepLuma, mcTmp, 16, &mvLumaBackDirect[0], 0);
          h263e_Copy8x8HP_8u(pYb+8, mStepLuma, mcTmp+8, 16, &mvLumaBackDirect[1], 0);
          h263e_Copy8x8HP_8u(pYb+8*mStepLuma, mStepLuma, mcTmp+8*16, 16, &mvLumaBackDirect[2], 0);
          h263e_Copy8x8HP_8u(pYb+8*mStepLuma+8, mStepLuma, mcTmp+8*16+8, 16, &mvLumaBackDirect[3], 0);
          ippiAverage8x8_8u_C1IR(mcTmp, 16, mcPredD, 16);
          ippiAverage8x8_8u_C1IR(mcTmp+8, 16, mcPredD+8, 16);
          ippiAverage8x8_8u_C1IR(mcTmp+8*16, 16, mcPredD+8*16, 16);
          ippiAverage8x8_8u_C1IR(mcTmp+8*16+8, 16, mcPredD+8*16+8, 16);
        } else {
          mvLumaForwDirect[0].dx = (Ipp16s)(mTRB * MBcurr->mv[0].dx / mTRD);
          mvLumaForwDirect[0].dy = (Ipp16s)(mTRB * MBcurr->mv[0].dy / mTRD);
          mvLumaBackDirect[0].dx = (Ipp16s)((mTRB - mTRD) * MBcurr->mv[0].dx / mTRD);
          mvLumaBackDirect[0].dy = (Ipp16s)((mTRB - mTRD) * MBcurr->mv[0].dy / mTRD);
          h263e_Copy16x16HP_8u(pYf, mStepLuma, mcPredD, 16, &mvLumaForwDirect[0], 0);
          h263e_Copy16x16HP_8u(pYb, mStepLuma, mcTmp, 16, &mvLumaBackDirect[0], 0);
          ippiAverage16x16_8u_C1IR(mcTmp, 16, mcPredD, 16);
        }
        ippiSAD16x16_8u32s(pYc, mStepLuma, mcPredD, 16, &bestSADDirect, IPPVC_MC_APX_FF);
        bestSADDirect -= SAD_FAVOR_DIRECT;
        if (bestSADDirect <= SAD_ZERO) {
          mb_type = IPPVC_MBTYPE_DIRECT;
        } else {
          // Forward pred
          mvLumaForw = mvPredForw;
          // SAD at (0,0)
          ippiSAD16x16_8u32s(pYc, mStepLuma, pYf, mStepLuma, &bestSAD16x16Forw, IPPVC_MC_APX_FF);
          xL = -IPP_MIN(j * 16 + 16, mBPicsearchHorForw);
          yT = -IPP_MIN(i * 16 + 16, mBPicsearchVerForw);
          xR = (j == mNumMacroBlockPerRow - 1) ? IPP_MIN(16, mBPicsearchHorForw) : IPP_MIN(mVideoSequence.video_object_layer_width + 16 - (j + 1) * 16, mBPicsearchHorForw);
          yB = (i == mNumMacroBlockPerCol - 1) ? IPP_MIN(16, mBPicsearchVerForw) : IPP_MIN(mVideoSequence.video_object_layer_height + 16 - (i + 1) * 16, mBPicsearchVerForw);
          ME_SAD_16x16(pYc, pYf, mStepLuma, xL, xR, yT, yB, &bestSAD16x16Forw, &mvLumaForw);
          mvForw.dx = (Ipp16s)(mvLumaForw.dx - mvPredForw.dx);
          mvForw.dy = (Ipp16s)(mvLumaForw.dy - mvPredForw.dy);
          if (mVideoPicture.UMV < 2)
            h263e_MV_CheckRange(&mvForw, mVideoPicture.UMV);
          if (bestSAD16x16Forw <= SAD_ZERO) {
            mb_type = IPPVC_MBTYPE_FORWARD;
          } else {
            // Backward pred
            mvLumaBack = mvPredBack;
            // SAD at (0,0)
            ippiSAD16x16_8u32s(pYc, mStepLuma, pYb, mStepLuma, &bestSAD16x16Back, IPPVC_MC_APX_FF);
            xL = -IPP_MIN(j * 16 + 16, mBPicsearchHorBack);
            yT = -IPP_MIN(i * 16 + 16, mBPicsearchVerBack);
            xR = (j == mNumMacroBlockPerRow - 1) ? IPP_MIN(16, mBPicsearchHorBack) : IPP_MIN(mVideoSequence.video_object_layer_width + 16 - (j + 1) * 16, mBPicsearchHorBack);
            yB = (i == mNumMacroBlockPerCol - 1) ? IPP_MIN(16, mBPicsearchVerBack) : IPP_MIN(mVideoSequence.video_object_layer_height + 16 - (i + 1) * 16, mBPicsearchVerBack);
            ME_SAD_16x16(pYc, pYb, mStepLuma, xL, xR, yT, yB, &bestSAD16x16Back, &mvLumaBack);
            mvBack.dx = (Ipp16s)(mvLumaBack.dx - mvPredBack.dx);
            mvBack.dy = (Ipp16s)(mvLumaBack.dy - mvPredBack.dy);
            if (mVideoPicture.UMV < 2)
              h263e_MV_CheckRange(&mvBack, mVideoPicture.UMV);
            if (bestSAD16x16Back <= SAD_ZERO) {
                mb_type = IPPVC_MBTYPE_BACKWARD;
            } else {
              // Inter pred
              h263e_Copy16x16HP_8u(pYf, mStepLuma, mcPredI, 16, &mvLumaForw, 0);
              h263e_Copy16x16HP_8u(pYb, mStepLuma, mcTmp, 16, &mvLumaBack, 0);
              ippiAverage16x16_8u_C1IR(mcTmp, 16, mcPredI, 16);
              ippiSAD16x16_8u32s(pYc, mStepLuma, mcPredI, 16, &bestSAD16x16Inter, IPPVC_MC_APX_FF);
              bestSAD = IPP_MIN(IPP_MIN(bestSAD16x16Forw, bestSAD16x16Back), IPP_MIN(bestSADDirect, bestSAD16x16Inter));
              if (bestSADDirect == bestSAD)
                mb_type = IPPVC_MBTYPE_DIRECT;
              else if (bestSAD16x16Back == bestSAD)
                mb_type = IPPVC_MBTYPE_BACKWARD;
              else if (bestSAD16x16Forw == bestSAD)
                mb_type = IPPVC_MBTYPE_FORWARD;
              else
                mb_type = IPPVC_MBTYPE_INTERPOLATE;
            }
          }
        }
        if (mb_type == IPPVC_MBTYPE_FORWARD) {
          h263e_Copy16x16HP_8u(pYf, mStepLuma, mcTmp, 16, &mvLumaForw, 0);
          h263e_ComputeChromaMV(&mvLumaForw, &mvChromaF);
          h263e_Copy8x8HP_8u(pUf, mStepChroma, mcTmp+64*4, 8, &mvChromaF, 0);
          h263e_Copy8x8HP_8u(pVf, mStepChroma, mcTmp+64*5, 8, &mvChromaF, 0);
          mcPred = mcTmp;
          mvPredForw = mvLumaForw;
        } else if (mb_type == IPPVC_MBTYPE_BACKWARD) {
          h263e_Copy16x16HP_8u(pYb, mStepLuma, mcTmp, 16, &mvLumaBack, 0);
          h263e_ComputeChromaMV(&mvLumaBack, &mvChromaB);
          h263e_Copy8x8HP_8u(pUb, mStepChroma, mcTmp+64*4, 8, &mvChromaB, 0);
          h263e_Copy8x8HP_8u(pVb, mStepChroma, mcTmp+64*5, 8, &mvChromaB, 0);
          mcPred = mcTmp;
          mvPredBack = mvLumaBack;
        } else if (mb_type == IPPVC_MBTYPE_INTERPOLATE) {
          h263e_ComputeChromaMV(&mvLumaForw, &mvChromaF);
          h263e_ComputeChromaMV(&mvLumaBack, &mvChromaB);
          h263e_Copy8x8HP_8u(pUf, mStepChroma, mcPredI+64*4, 8, &mvChromaF, 0);
          h263e_Copy8x8HP_8u(pUb, mStepChroma, mcTmp+64*4, 8, &mvChromaB, 0);
          h263e_Copy8x8HP_8u(pVf, mStepChroma, mcPredI+64*5, 8, &mvChromaF, 0);
          h263e_Copy8x8HP_8u(pVb, mStepChroma, mcTmp+64*5, 8, &mvChromaB, 0);
          ippiAverage8x8_8u_C1IR(mcTmp+64*4, 8, mcPredI+64*4, 8);
          ippiAverage8x8_8u_C1IR(mcTmp+64*5, 8, mcPredI+64*5, 8);
          mcPred = mcPredI;
          mvPredBack = mvLumaBack;
          mvPredForw = mvLumaForw;
        } else {
          if (MBcurr->type == IPPVC_MBTYPE_INTER4V) {
              h263e_ComputeChroma4MV(mvLumaForwDirect, &mvChromaF);
              h263e_ComputeChroma4MV(mvLumaBackDirect, &mvChromaB);
          } else {
              h263e_ComputeChromaMV(&mvLumaForwDirect[0], &mvChromaF);
              h263e_ComputeChromaMV(&mvLumaBackDirect[0], &mvChromaB);
          }
          h263e_Copy8x8HP_8u(pUf, mStepChroma, mcPredD+64*4, 8, &mvChromaF, 0);
          h263e_Copy8x8HP_8u(pUb, mStepChroma, mcTmp+64*4, 8, &mvChromaB, 0);
          h263e_Copy8x8HP_8u(pVf, mStepChroma, mcPredD+64*5, 8, &mvChromaF, 0);
          h263e_Copy8x8HP_8u(pVb, mStepChroma, mcTmp+64*5, 8, &mvChromaB, 0);
          ippiAverage8x8_8u_C1IR(mcTmp+64*4, 8, mcPredD+64*4, 8);
          ippiAverage8x8_8u_C1IR(mcTmp+64*5, 8, mcPredD+64*5, 8);
          mcPred = mcPredD;
        }
//                pattern = TransMacroBlockInter_H263(pYc, pUc, pVc, coeffMB, nzCount, quant, mcPred, i, j, &dct_type);
        pattern = TransMacroBlockInter_H263(pYc, pUc, pVc, coeffMB, nzCount, quant, mcPred, MBcurr->lumaErr);
        if (mb_type == IPPVC_MBTYPE_DIRECT && pattern == 0) {
            modb = 2;
            // encode modb
            cBS.PutBits(1, 1);
        } else {
          modb = pattern == 0 ? 1 : 0;
          // encode modb
          cBS.PutBits(modb, 2);
          // encode mb_type
          cBS.PutBits(1, 1 + mb_type - IPPVC_MBTYPE_DIRECT);
          // encode cbpb
          if (modb == 0)
              cBS.PutBits(pattern, 6);
          // code dbquant
          if (mb_type != IPPVC_MBTYPE_DIRECT && pattern != 0)
            if (dbquant == 0)
              cBS.PutBits(0, 1);
            else
              cBS.PutBits(dbquant < 0 ? 2 : 3, 2);
          // encode delta mv
          if (mb_type == IPPVC_MBTYPE_DIRECT)
            EncodeMV(&mvDelta, IPPVC_MBTYPE_INTER);
          // encode forward mv
          if (mb_type == IPPVC_MBTYPE_FORWARD || mb_type == IPPVC_MBTYPE_INTERPOLATE)
            EncodeMV(&mvForw, IPPVC_MBTYPE_INTER);
          // encode backward mv
          if (mb_type == IPPVC_MBTYPE_BACKWARD || mb_type == IPPVC_MBTYPE_INTERPOLATE)
            EncodeMV(&mvBack, IPPVC_MBTYPE_INTER);
          // encode blocks
          if (pattern)
            EncodeMacroBlockInter_H263(coeffMB, pattern, nzCount);
        }
      }
      // restore mVideoPicture
      if (mCalcPSNR) {
        if (MBcurr->not_coded) {
          mPSNR_Y += h263e_CalcMSE_16x16(pYf, mStepLuma, pYc, mStepLuma);
          mPSNR_U += h263e_CalcMSE_8x8(pUf, mStepChroma, pUc, mStepChroma);
          mPSNR_V += h263e_CalcMSE_8x8(pVf, mStepChroma, pVc, mStepChroma);
        } else {
          if (modb != 0) {  // pattern == 0
            mPSNR_Y += h263e_CalcMSE_16x16(mcPred, 16, pYc, mStepLuma);
            mPSNR_U += h263e_CalcMSE_8x8(mcPred+64*4, 8, pUc, mStepChroma);
            mPSNR_V += h263e_CalcMSE_8x8(mcPred+64*5, 8, pVc, mStepChroma);
          } else
            ReconMacroBlockInter_H263(pYc, pUc, pVc, mcPred, coeffMB, quant, pattern);
//              ReconMacroBlockInter_H263(pYc, pUc, pVc, mcPred, coeffMB, quant, pattern, dct_type);
        }
      }
      MBcurr++;
      pYc += 16; pUc += 8; pVc += 8;
      pYf += 16; pUf += 8; pVf += 8;
      pYb += 16; pUb += 8; pVb += 8;
    }
  }
  EncodeStuffingBitsAlign();
}
#endif

#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
