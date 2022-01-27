/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH261 (encode Frames)
//  Contents:
//                  EncodeFrame
//
//  References:
//    Fast ME algorithm
//        IEEE Transactions on image processing, vol. 9, ¹. 2, Feb 2000
//        "A new Diamond Search Algorithm for Fast Block-Matching Motion Estimation"
//        Shan Zhu and Kai-Kuang Ma
*/
#include "umc_defs.h"

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)
#include "h261_enc.hpp"

//--------------------- defines to control ME process -------------------------

//#define ME_FULLSEARCH_RECT      // using rectangle instead of involute search
//#define ME_USE_THRESHOLD        // using threshold in ME (faster) ???
//#define ME_USE_PREDICTEDMV_INIT   // using predicted MV as init point of ME otherwise zero MV is used

const Ipp32s SAD_FAVOR_ZERO  = 129 /4;
const Ipp32s SAD_FAVOR_ZERO_MVD  = 129 /4;
const Ipp32s SAD_FAVOR_ZERO_MV  = 129 /4;
const Ipp32s SAD_FAVOR_INTER  = 512 /8;   // 500 H.263 App III
const Ipp32s SAD_FAVOR_FILTER = 16;

//-----------------------------------------------------------------------------

#if 0
void ippVideoEncoderH261::PostFrameRC(Ipp32s bpfEncoded)
{
  Ipp32s bpfExpected, delay, quant, coding_type;
  coding_type = mVideoPicture.picture_coding_type;

  mBitsDesiredRC += mBitsPerFrameRC;
  delay = (Ipp32s)(mBitsDesiredRC - mBitsEncoded);
  if (coding_type == H261_PIC_TYPE_I) {
    bpfExpected = (mBitsPerFrameRC << 1) + (delay >> 1);
    h261e_ClipL(bpfExpected, mBitsPerFrameRC * 7 >> 3);
  } else { // P-Picture
    bpfExpected = mBitsPerFrameRC + (delay >> 2);
    h261e_Clip(bpfExpected, mBitsPerFrameRC >> 1, mBitsPerFrameRC * 3 >> 1);
  }
  quant = (coding_type == H261_PIC_TYPE_I) ? mQuantIFrame : mQuantPFrame;
  if (abs(bpfEncoded - bpfExpected) > (bpfExpected >> 4)) {
    if (bpfEncoded > bpfExpected) {
      quant++;
      if (bpfEncoded > 2 * bpfExpected)
        quant++;
      h261e_ClipR(quant, 31);
    } else if (bpfEncoded < bpfExpected) {
      quant--;
      if (bpfEncoded * 2 < bpfExpected)
        quant--;
      h261e_ClipL(quant, 1);
    }
  }
  if (coding_type == H261_PIC_TYPE_I)
    mQuantIFrame = quant;
  else
    mQuantPFrame = quant;
}
#endif

Ipp32s ippVideoEncoderH261::EncodeFrame()
{
    Ipp32s isIFrame;
    Ipp32s sts;
    Ipp32s minBufSize;
//    Ipp32s bpfEncoded;
    mFrameSkipped = 0;

    if (!mIsInit)
      return H261_STS_ERR_NOTINIT;

    if (mRateControl)
      minBufSize = IPP_MIN(mBitsDesiredFrame * 2, mBPPmax);
    else
      minBufSize = mBPPmax;
    minBufSize >>= 3; // bits -> bytes

    if (cBS.mBuffSize - cBS.GetFullness() < minBufSize)
      return H261_STS_ERR_BUFOVER;

    if (mSkipFrame) {
      mFrameSkipped = 1;
      sts = H261_STS_SKIPPED_FRAME;
    } else {
      isIFrame = (mFrameCount - mLastIFrame >= mIFramedist);
      if (isIFrame) {
        mVideoPicture.frame_quant = mQuantIFrame;
        mVideoPicture.picture_coding_type = H261_PIC_TYPE_I;
        EncodePicture_Header();
        EncodeIFrame();
        mLastIFrame = mFrameCount;
      } else {
        mVideoPicture.frame_quant = mQuantPFrame;
        mVideoPicture.picture_coding_type = H261_PIC_TYPE_P;
        EncodePicture_Header();
        EncodePFrame();
      }
      sts = H261_STS_NOERR;
    }
    mVideoPicture.temporal_reference += mFrameInterval;

#if 0
    bpfEncoded = cBS.GetFullness();
    bpfEncoded <<= 3;
    mBitsEncoded += bpfEncoded;
    if (mRateControl) {
      PostFrameRC(bpfEncoded);
    }
#endif

    mBitsEncodedFrame = cBS.GetFullness() << 3;
    mBitsEncodedTotal += mBitsEncodedFrame;

    if (mRateControl)
      PostFrameRC();

    if (sts != H261_STS_SKIPPED_FRAME && mIFramedist != 1) {
      h261e_Swap(mFrameC, mFrameF);
    }
    mFrameCount++;
    return sts;
}

#define h261_SetPattern(pattern, nzCount, f) \
{ \
    pattern = 0; \
    if (nzCount[0] >= f) \
        pattern = 32; \
    if (nzCount[1] >= f) \
        pattern |= 16; \
    if (nzCount[2] >= f) \
        pattern |= 8; \
    if (nzCount[3] >= f) \
        pattern |= 4; \
    if (nzCount[4] >= f) \
        pattern |= 2; \
    if (nzCount[5] >= f) \
        pattern |= 1; \
}

void ippVideoEncoderH261::TransMacroBlockIntra_H261(
  Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s quant)
{
  ippiDCT8x8Fwd_8u16s_C1R(pY, mStepLuma, coeffMB+0*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8, mStepLuma, coeffMB+1*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma, mStepLuma, coeffMB+2*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma+8, mStepLuma, coeffMB+3*64);
  ippiDCT8x8Fwd_8u16s_C1R(pU, mStepChroma, coeffMB+4*64);
  ippiDCT8x8Fwd_8u16s_C1R(pV, mStepChroma, coeffMB+5*64);
  ippiQuantIntra_H263_16s_C1I(coeffMB+0*64, quant, &nzCount[0], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+1*64, quant, &nzCount[1], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+2*64, quant, &nzCount[2], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+3*64, quant, &nzCount[3], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+4*64, quant, &nzCount[4], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+5*64, quant, &nzCount[5], 0, 0);
}

void ippVideoEncoderH261::TransMacroBlockIntra_H261_qc(
  Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s *pQuant)
{
  Ipp16s dc[6];
  Ipp32s i;
  Ipp32s quant = *pQuant;
  IppiSize roi = {6*64, 1};
  Ipp16s gmin, gmax;
  Ipp16s minLevel, maxLevel;

  ippiDCT8x8Fwd_8u16s_C1R(pY, mStepLuma, coeffMB+0*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8, mStepLuma, coeffMB+1*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma, mStepLuma, coeffMB+2*64);
  ippiDCT8x8Fwd_8u16s_C1R(pY+8*mStepLuma+8, mStepLuma, coeffMB+3*64);
  ippiDCT8x8Fwd_8u16s_C1R(pU, mStepChroma, coeffMB+4*64);
  ippiDCT8x8Fwd_8u16s_C1R(pV, mStepChroma, coeffMB+5*64);

  for (i = 0; i < 6; i++) {
    dc[i] = coeffMB[i*64];
    coeffMB[i*64] = 0;
  }
  ippiMinMax_16s_C1R(coeffMB, roi.width*2, roi, &gmin, &gmax);
  for (i = 0; i < 6; i++)
    coeffMB[i*64] = dc[i];

  minLevel = -255 * quant + 1 - (quant & 1);
  maxLevel = 255 * quant + (quant & 1) - 1;
  while (gmax > maxLevel || gmin < minLevel) {
    maxLevel += 256 - (quant & 1) * 2;
    minLevel -= 256 - (quant & 1) * 2;
    quant++;
  }
  ippiQuantIntra_H263_16s_C1I(coeffMB+0*64, quant, &nzCount[0], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+1*64, quant, &nzCount[1], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+2*64, quant, &nzCount[2], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+3*64, quant, &nzCount[3], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+4*64, quant, &nzCount[4], 0, 0);
  ippiQuantIntra_H263_16s_C1I(coeffMB+5*64, quant, &nzCount[5], 0, 0);
  *pQuant = quant;
}



Ipp32s ippVideoEncoderH261::TransMacroBlockInter_H261_qc(
  Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp16s *coeffMB,
  Ipp32s *nzCount, Ipp32s *pQuant, Ipp8u *mcPred, Ipp32s lumaErr, Ipp32s sU, Ipp32s sV)
{
  IppiSize roi = {64, 1};
  Ipp16s max, min, gmax = 0, gmin = 0;
  Ipp16s minLevel, maxLevel;
  Ipp32s pattern, lim, i;
  Ipp32s sL[4];
  Ipp32s quant = *pQuant;

  lim = quant * 16;
  if (lumaErr < quant * 20) {
    sL[0] = sL[1] = sL[2] = sL[3] = 0;
  } else {
    for (i = 0; i < 4; i++) {
      ippiSubSAD8x8_8u16s_C1R(pYc + (i & 1)*8 + (i & 2)*4*mStepLuma, mStepLuma,
        mcPred + (i & 2)*64 + (i & 1)*8, 16,
        coeffMB + i*64, 16, &(sL[i]));
      if (sL[i] >= lim) {
        ippiDCT8x8Fwd_16s_C1I(coeffMB + i*64);
        ippiMinMax_16s_C1R(coeffMB + i*64, roi.width*2, roi, &min, &max);
        if (max > gmax) gmax = max;
        if (min < gmin) gmin = min;
      }
    }
  }
  if (sU >= lim) {
    ippiSub8x8_8u16s_C1R(pUc, mStepChroma, mcPred+64*4, 8, coeffMB+4*64, 16);
    ippiDCT8x8Fwd_16s_C1I(coeffMB+4*64);
    ippiMinMax_16s_C1R(coeffMB + 4*64, roi.width*2, roi, &min, &max);
    if (max > gmax) gmax = max;
    if (min < gmin) gmin = min;
  }
  if (sV >= lim) {
    ippiSub8x8_8u16s_C1R(pVc, mStepChroma, mcPred+64*5, 8, coeffMB+5*64, 16);
    ippiDCT8x8Fwd_16s_C1I(coeffMB+5*64);
    ippiMinMax_16s_C1R(coeffMB + 5*64, roi.width*2, roi, &min, &max);
    if (max > gmax) gmax = max;
    if (min < gmin) gmin = min;
  }

  minLevel = -255 * quant + 1 - (quant & 1);
  maxLevel = 255 * quant + (quant & 1) - 1;
  while (gmax > maxLevel || gmin < minLevel) {
    maxLevel += 256 - (quant & 1) * 2;
    minLevel -= 256 - (quant & 1) * 2;
    quant++;
  }

  lim = quant * 16;
  for (i = 0; i < 4; i++) {
    if (sL[i] < lim)
      nzCount[i] = 0;
    else
      ippiQuantInter_H263_16s_C1I(coeffMB + i*64, quant, &(nzCount[i]), 0);
  }
  if (sU < lim)
    nzCount[4] = 0;
  else
    ippiQuantInter_H263_16s_C1I(coeffMB+4*64, quant, &(nzCount[4]), 0);
  if (sV < lim)
    nzCount[5] = 0;
  else
    ippiQuantInter_H263_16s_C1I(coeffMB+5*64, quant, &(nzCount[5]), 0);
  *pQuant = quant;
  h261_SetPattern(pattern, nzCount, 1);
  return pattern;
}

Ipp32s ippVideoEncoderH261::TransMacroBlockInter_H261(
  Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp16s *coeffMB,
  Ipp32s *nzCount, Ipp32s quant, Ipp8u *mcPred, Ipp32s lumaErr, Ipp32s sU, Ipp32s sV)
{
  Ipp32s pattern, lim, i;
  Ipp32s sL[4];

  lim = quant * 16;
  if (lumaErr < quant * 20) {
    nzCount[0] = nzCount[1] = nzCount[2] = nzCount[3] = 0;
  } else {
    for (i = 0; i < 4; i++) {
      ippiSubSAD8x8_8u16s_C1R(pYc + (i & 1)*8 + (i & 2)*4*mStepLuma, mStepLuma,
                              mcPred + (i & 2)*64 + (i & 1)*8, 16,
                              coeffMB + i*64, 16, &(sL[i]));

      if (sL[i] < lim) {
        nzCount[i] = 0;
      } else {
        ippiDCT8x8Fwd_16s_C1I(coeffMB + i*64);
        ippiQuantInter_H263_16s_C1I(coeffMB + i*64, quant, &(nzCount[i]), 0);
      }
    }
  }

  if (sU < lim) {
    nzCount[4] = 0;
  } else {
    ippiSub8x8_8u16s_C1R(pUc, mStepChroma, mcPred+64*4, 8, coeffMB+4*64, 16);
    ippiDCT8x8Fwd_16s_C1I(coeffMB+4*64);
    ippiQuantInter_H263_16s_C1I(coeffMB+4*64, quant, &(nzCount[4]), 0);
  }
  if (sV < lim) {
    nzCount[5] = 0;
  } else {
    ippiSub8x8_8u16s_C1R(pVc, mStepChroma, mcPred+64*5, 8, coeffMB+5*64, 16);
    ippiDCT8x8Fwd_16s_C1I(coeffMB+5*64);
    ippiQuantInter_H263_16s_C1I(coeffMB+5*64, quant, &(nzCount[5]), 0);
  }
  h261_SetPattern(pattern, nzCount, 1);
  return pattern;
}


void ippVideoEncoderH261::EncodeMacroBlockIntra_H261(Ipp16s *coeffMB, Ipp32s *nzCount)
{
  Ipp32s i;
  for (i = 0; i < 6; i++) {
#if 0
    Ipp16s coeffZ[64];
    ippiScanFwd_16s_C1(coeffMB + i*64, coeffZ, nzCount[i], IPPVC_SCAN_ZIGZAG);
    ippiEncodeCoeffsIntra_H261_16s1u(coeffZ, &cBS.mPtr, &cBS.mBitOff, nzCount[i], -1);
#else
    ippiEncodeCoeffsIntra_H261_16s1u(coeffMB + i*64, &cBS.mPtr, &cBS.mBitOff, nzCount[i], IPPVC_SCAN_ZIGZAG);
#endif
  }
}

void ippVideoEncoderH261::EncodeMacroBlockInter_H261(Ipp16s *coeffMB, Ipp32s *nzCount)
{
  Ipp32s i;
  for (i = 0; i < 6; i++) {
    if (nzCount[i]) {
#if 0
      Ipp16s coeffZ[64];
      ippiScanFwd_16s_C1(coeffMB + i*64, coeffZ, nzCount[i], IPPVC_SCAN_ZIGZAG);
      ippiEncodeCoeffsInter_H261_16s1u(coeffZ, &cBS.mPtr, &cBS.mBitOff, nzCount[i], -1);
#else
      ippiEncodeCoeffsInter_H261_16s1u(coeffMB + i*64, &cBS.mPtr, &cBS.mBitOff, nzCount[i], IPPVC_SCAN_ZIGZAG);
#endif
    }
  }
}


// for iDCT with DC only
static void h261_Set8x8(Ipp8u *p, Ipp32s step, Ipp32s level)
{
    Ipp32s i, j;
    if (level < 0) level = 0; if (level > 255) level = 255;
    for (i = 0; i < 8; i++) {
       for (j = 0; j < 8; j++)
           p[j] = (Ipp8u)level;
       p += step;
    }
}


inline Ipp32s CalcMSE_16x16(Ipp8u *pSrc1, Ipp32s stepSrc1, Ipp8u *pSrc2, Ipp32s stepSrc2)
{
    Ipp32s e;
    ippiSqrDiff16x16_8u32s(pSrc1, stepSrc1, pSrc2, stepSrc2, IPPVC_MC_APX_FF, &e);
    return e;
}


inline Ipp32s CalcMSE_8x8(Ipp8u *pSrc1, Ipp32s stepSrc1, Ipp8u *pSrc2, Ipp32s stepSrc2)
{
    Ipp32s e;
    ippiSSD8x8_8u32s_C1R(pSrc1, stepSrc1, pSrc2, stepSrc2, &e, IPPVC_MC_APX_FF);
    return e;
}


inline void ippVideoEncoderH261::ReconMacroBlockNotCoded(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred)
{
  if (mCalcPSNR) {
      mPSNR_Y += CalcMSE_16x16(mcPred, 16, pYc, mStepLuma);
      mPSNR_U += CalcMSE_8x8(mcPred+64*4, 8, pUc, mStepChroma);
      mPSNR_V += CalcMSE_8x8(mcPred+64*5, 8, pVc, mStepChroma);
  }
  ippiCopy16x16_8u_C1R(mcPred, 16, pYc, mStepLuma);
  ippiCopy8x8_8u_C1R(mcPred+64*4, 8, pUc, mStepChroma);
  ippiCopy8x8_8u_C1R(mcPred+64*5, 8, pVc, mStepChroma);
}


void ippVideoEncoderH261::ReconMacroBlockIntra_H261(Ipp8u *pY,
                                                    Ipp8u *pU,
                                                    Ipp8u *pV,
                                                    Ipp16s *coeffMB,
                                                    Ipp32s quant,
                                                    Ipp32s *nzCount)
{
  __ALIGN16(Ipp8u, pDec, 64*6);
  Ipp32s i;

  if (mCalcPSNR) {
    ippiCopy16x16_8u_C1R(pY, mStepLuma, pDec, 16);
    ippiCopy8x8_8u_C1R(pU, mStepChroma, pDec+256, 8);
    ippiCopy8x8_8u_C1R(pV, mStepChroma, pDec+320, 8);
  }

  for (i = 0; i < 4; i++) {
    if (nzCount[i] == 1) { /* DC is always non-zero */
      h261_Set8x8(pY + ((i & 1) << 3) + ((i & 2) << 2)*mStepLuma, mStepLuma, coeffMB[i*64]);
    } else {
      ippiQuantInvIntra_H263_16s_C1I(coeffMB+i*64, 63, quant, 0, 0);
      ippiDCT8x8Inv_16s8u_C1R(coeffMB + i*64, pY + ((i & 1) << 3) + ((i & 2) << 2)*mStepLuma, mStepLuma);
    }
  }
  if (nzCount[4] == 1) /* DC is always non-zero */
    h261_Set8x8(pU, mStepChroma, coeffMB[4*64]);
  else {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+4*64, 63, quant, 0, 0);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB + 4*64, pU, mStepChroma);
  }
  if (nzCount[5] == 1) /* DC is always non-zero */
    h261_Set8x8(pV, mStepChroma, coeffMB[5*64]);
  else {
    ippiQuantInvIntra_H263_16s_C1I(coeffMB+5*64, 63, quant, 0, 0);
    ippiDCT8x8Inv_16s8u_C1R(coeffMB + 5*64, pV, mStepChroma);
  }

  if (mCalcPSNR) {
      mPSNR_Y += CalcMSE_16x16(pY, mStepLuma, pDec, 16);
      mPSNR_U += CalcMSE_8x8(pU, mStepChroma, pDec+256, 8);
      mPSNR_V += CalcMSE_8x8(pV, mStepChroma, pDec+320, 8);
  }
}


void ippVideoEncoderH261::ReconMacroBlockInter_H261(Ipp8u  *pYc,
                                                    Ipp8u  *pUc,
                                                    Ipp8u  *pVc,
                                                    Ipp8u  *mcPred,
                                                    Ipp16s *coeffMB,
                                                    Ipp32s quant,
                                                    Ipp32s *nzCount)
{
  Ipp32s i;
  for (i = 0; i < 4; i++) {
    if (nzCount[i]) {
      ippiQuantInvInter_H263_16s_C1I(coeffMB+i*64, 63, quant, 0);
      ippiDCT8x8Inv_16s_C1I(coeffMB+i*64);
      ippiAdd8x8_16s8u_C1IRS(coeffMB+i*64, 16, mcPred + (i & 1)*8 + (i & 2)*64, 16);
    }
  }
  if (nzCount[4]) {
    ippiQuantInvInter_H263_16s_C1I(coeffMB+4*64, 63, quant, 0);
    ippiDCT8x8Inv_16s_C1I(coeffMB+4*64);
    ippiAdd8x8_16s8u_C1IRS(coeffMB+4*64, 16, mcPred + 4*64, 8);
  }
  if (nzCount[5]) {
    ippiQuantInvInter_H263_16s_C1I(coeffMB+5*64, 63, quant, 0);
    ippiDCT8x8Inv_16s_C1I(coeffMB+5*64);
    ippiAdd8x8_16s8u_C1IRS(coeffMB+5*64, 16, mcPred + 5*64, 8);
  }
  if (mCalcPSNR) {
    mPSNR_Y += CalcMSE_16x16(pYc, mStepLuma, mcPred, 16);
    mPSNR_U += CalcMSE_8x8(pUc, mStepChroma, mcPred+64*4, 8);
    mPSNR_V += CalcMSE_8x8(pVc, mStepChroma, mcPred+64*5, 8);
  }
  ippiCopy16x16_8u_C1R(mcPred, 16, pYc, mStepLuma);
  ippiCopy8x8_8u_C1R(mcPred+64*4, 8, pUc, mStepChroma);
  ippiCopy8x8_8u_C1R(mcPred+64*5, 8, pVc, mStepChroma);
}

static void h261_ME_FullSearch_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step,
                                         Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB,
                                         Ipp32s *xPos, Ipp32s *yPos, Ipp32s *bestSAD,
                                         Ipp32s thrSAD_16x16)
{
    Ipp32s cSAD, bSAD, i, j, jPos, iPos;

    jPos = *xPos;
    iPos = *yPos;
    // find SAD at init mv in full pixel
    ippiSAD16x16_8u32s(pCur, step, pRef+iPos*step+jPos, step, &bSAD, IPPVC_MC_APX_FF);

    bSAD -= SAD_FAVOR_ZERO_MVD;
#ifdef ME_USE_THRESHOLD
    if (bSAD <= thrSAD_16x16) {
        *bestSAD = bSAD;
        return;
    }
#endif
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

static void h261_ME_DiamondSearch_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step,
                                            Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB,
                                            Ipp32s *xPos, Ipp32s *yPos, Ipp32s *bestSAD,
                                            Ipp32s *sadBuf, Ipp32s thrSAD_16x16)
{
    Ipp32s cSAD, zSAD, bSAD, i, j, k, l, iPos, jPos, swWidth, swHeight;

    jPos = *xPos;
    iPos = *yPos;
    ippiSAD16x16_8u32s(pCur, step, pRef+iPos*step+jPos, step, &bSAD, IPPVC_MC_APX_FF);
    bSAD -= SAD_FAVOR_ZERO_MVD;
    zSAD = 0; // to avoid warnings
#ifdef ME_USE_THRESHOLD
    if (bSAD <= thrSAD_16x16) {
        *bestSAD = bSAD;
        return;
    }
#endif

  if (iPos | jPos) {
    // find SAD at zero mv
    ippiSAD16x16_8u32s(pCur, step, pRef, step, &zSAD, IPPVC_MC_APX_FF);
    zSAD -= SAD_FAVOR_ZERO_MV;
    if (zSAD < bSAD) {
      bSAD = zSAD;
      iPos = jPos = 0;
    }
#ifdef ME_USE_THRESHOLD
    if (bSAD <= thrSAD_16x16) {
        *bestSAD = bSAD;
        *xPos = *yPos = 0;
        return;
    }
#endif
  }

    swWidth = (xR - xL + 1);
    swHeight = (yB - yT + 1);
    ippsSet_8u((Ipp8u)-1, (Ipp8u*)sadBuf, swWidth * swHeight * 4);
    sadBuf[(iPos-yT)*swWidth+jPos-xL] = bSAD;
    if (iPos | jPos)
      sadBuf[(0-yT)*swWidth+0-xL] = zSAD;
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

void ippVideoEncoderH261::ME_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step,
                                       Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB,
                                       Ipp32s *bestSAD, IppMotionVector *mv)
{
  Ipp32s xPos, yPos;

#ifdef ME_USE_PREDICTEDMV_INIT
  xPos = mv->dx;
  yPos = mv->dy;
  // check init mv is in rect
  if (xPos < xL || xPos > xR || yPos < yT || yPos > yB)
#endif
    xPos = yPos = 0;

  if (mMEalgorithm == 0)
    h261_ME_FullSearch_SAD_16x16(pCur, pRef, step, xL, xR, yT, yB, &xPos, &yPos, bestSAD, mMEthrSAD16x16);
  else
    h261_ME_DiamondSearch_SAD_16x16(pCur, pRef, step, xL, xR, yT, yB, &xPos, &yPos, bestSAD, mMEfastSAD, mMEthrSAD16x16);

//printf("MV %d %d \n", xPos, yPos);

#ifdef ME_USE_THRESHOLD
  if (*bestSAD <= mMEthrSAD16x16) {
#endif
    mv->dx = (Ipp16s)xPos;
    mv->dy = (Ipp16s)yPos;
#ifdef ME_USE_THRESHOLD
  }
#endif
}

#ifdef ME_USE_THRESHOLD
#define SAD_ZERO mMEthrSAD16x16
#else
#define SAD_ZERO 0
#endif


/*
???
Motion estimation is performed on the previous reconstructed frame, not
previous true raw frame
Skipping the copying of  MB's, which are not coded in two subsequent frames
doesn't work in the current configuration (with 2 buffers)

*/
void ippVideoEncoderH261::EncodePFrame()
{
  Ipp8u  *pYc, *pUc, *pVc, *pYf, *pUf, *pVf;
  Ipp8u  *pgobYc, *pgobUc, *pgobVc, *pgobYf, *pgobUf, *pgobVf;
  Ipp32s i, quant, pattern, xL, xR, yT, yB;
  Ipp8u  mba;
  Ipp32s prev_mba;
  Ipp8u  mtype = 0;
  Ipp32s dev, bestSAD, nzCount[6];
  h261_MacroBlock *MBcurr;
  __ALIGN16(Ipp16s, coeffMB, 64*6);
  __ALIGN16(Ipp8u, mcPred, 64*6);

  IppMotionVector mvLuma, mvChroma, mvPred;
  Ipp32s gobmask = mVideoPicture.source_format == 1 ? 1 : 0;
  Ipp32s filter;
  Ipp32s gobx = 0, goby = 0;
  Ipp32s nmb = 0;

  mvPred.dx = mvPred.dy = 0;

  for (i = 1; i <= mVideoPicture.num_gobs_in_frame; i++) {

    mVideoPicture.gob_number = i;
    quant = mVideoPicture.frame_quant = mQuantPFrame;

    // RTP support
    mRTPdata.GOBstartPos[i - 1] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
    EncodeGOB_Header();

    pgobYc = mFrameC->pY + goby + gobx;
    pgobUc = mFrameC->pU + (goby >> 2) + (gobx >> 1);
    pgobVc = mFrameC->pV + (goby >> 2) + (gobx >> 1);
    pgobYf = mFrameF->pY + goby + gobx;
    pgobUf = mFrameF->pU + (goby >> 2) + (gobx >> 1);
    pgobVf = mFrameF->pV + (goby >> 2) + (gobx >> 1);

    prev_mba = -1;

    for (mba = 0; mba < 33; mba++) {
      Ipp32s mby = (mba / 11) * 16 * mStepLuma;
      Ipp32s mbx = (mba % 11) * 16;

      quant = mQuantPFrame;

      MBcurr = MBinfo + (i - 1)*33 + mba;

      pYc = pgobYc + mby + mbx;
      pUc = pgobUc + (mby >> 2) + (mbx >> 1);
      pVc = pgobVc + (mby >> 2) + (mbx >> 1);

      pYf = pgobYf + mby + mbx;
      pUf = pgobUf + (mby >> 2) + (mbx >> 1);
      pVf = pgobVf + (mby >> 2) + (mbx >> 1);

      yT = (goby | mby) ? -mPFramesearchVer : 0;
      yB = (goby + mby == (mSourceHeight - 16) * mStepLuma) ? 0 : mPFramesearchVer;
      xL = (gobx | mbx) ? -mPFramesearchHor : 0;
      xR = (gobx + mbx == mSourceWidth - 16) ? 0 : mPFramesearchHor;

      if (mbx == 0)
        mvPred.dx = mvPred.dy = 0;

#ifdef ME_USE_PREDICTEDMV_INIT
      mvLuma = mvPred;
#else
      mvLuma.dx = mvLuma.dy = 0;
#endif

      ME_SAD_16x16(pYc, pYf, mStepLuma, xL, xR, yT, yB, &bestSAD, &mvLuma);

      ippiMeanAbsDev16x16_8u32s_C1R(pYc, mStepLuma, &dev);

      // choose mbtype
      if (dev < bestSAD - SAD_FAVOR_INTER) {
        // intra coded
        MBcurr->type = IPPVC_MBTYPE_INTRA;
        MBcurr->mv.dx = MBcurr->mv.dy = 0;

        if (quant < 8)
          TransMacroBlockIntra_H261_qc(pYc, pUc, pVc, coeffMB, nzCount, &quant);
        else
          TransMacroBlockIntra_H261(pYc, pUc, pVc, coeffMB, nzCount, quant);


        // RTP support
        mRTPdata.GOBN_MBAP[nmb] = (i << 8) | (prev_mba & 0xFF);
        mRTPdata.MBpos[nmb] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
        mRTPdata.MBquant[nmb] = (Ipp8u)quant;

        EncodeMBA(mba - prev_mba);
        prev_mba = mba;

        mtype = MTYPE_INTRA;
        if (quant != mVideoPicture.frame_quant) {
          mtype |= MTYPE_MQUANT;
          mVideoPicture.frame_quant = quant;
        }
        EncodeMType(mtype);
        if (mtype & MTYPE_MQUANT)
          cBS.PutBits(quant, 5);

        EncodeMacroBlockIntra_H261(coeffMB, nzCount);

        ReconMacroBlockIntra_H261(pYc, pUc, pVc, coeffMB, quant, nzCount);

        mvPred.dx = mvPred.dy = 0;
        MBcurr->not_coded = 0;
      } else {
        // inter coded
        Ipp8u *pRefY, *pRefU, *pRefV;
        IppMotionVector mvd;
        Ipp32s fSAD, fSADU, fSADV, bSADU, bSADV;
        mvChroma.dx = mvLuma.dx / 2;
        mvChroma.dy = mvLuma.dy / 2;

        pRefY = pYf + mvLuma.dy*mStepLuma + mvLuma.dx;
        pRefU = pUf + mvChroma.dy*mStepChroma + mvChroma.dx;
        pRefV = pVf + mvChroma.dy*mStepChroma + mvChroma.dx;

        ippiFilter8x8_H261_8u_C1R(pRefY, mStepLuma, mcPred, 16);
        ippiFilter8x8_H261_8u_C1R(pRefY + 8, mStepLuma, mcPred + 8, 16);
        ippiFilter8x8_H261_8u_C1R(pRefY + 8*mStepLuma, mStepLuma, mcPred + 8*16, 16);
        ippiFilter8x8_H261_8u_C1R(pRefY + 8 + 8*mStepLuma, mStepLuma, mcPred + 8 + 8*16, 16);
        ippiFilter8x8_H261_8u_C1R(pRefU, mStepChroma, mcPred + 64*4, 8);
        ippiFilter8x8_H261_8u_C1R(pRefV, mStepChroma, mcPred + 64*5, 8);

        ippiSAD16x16_8u32s(pYc, mStepLuma, mcPred, 16, &fSAD, IPPVC_MC_APX_FF);
        ippiSAD8x8_8u32s_C1R(pUc, mStepChroma, mcPred + 64*4, 8, &fSADU, IPPVC_MC_APX_FF);
        ippiSAD8x8_8u32s_C1R(pVc, mStepChroma, mcPred + 64*5, 8, &fSADV, IPPVC_MC_APX_FF);

        ippiSAD8x8_8u32s_C1R(pUc, mStepChroma, pRefU, mStepChroma, &bSADU, IPPVC_MC_APX_FF);
        ippiSAD8x8_8u32s_C1R(pVc, mStepChroma, pRefV, mStepChroma, &bSADV, IPPVC_MC_APX_FF);

        if (fSAD + fSADU + fSADV <= bestSAD + bSADU + bSADV + SAD_FAVOR_FILTER) {
          filter = 1;
          bestSAD = fSAD;
          bSADU = fSADU;
          bSADV = fSADV;
        } else {
          filter = 0;
          ippiCopy16x16_8u_C1R(pRefY, mStepLuma, mcPred, 16);
          ippiCopy8x8_8u_C1R(pRefU, mStepChroma, mcPred + 64*4, 8);
          ippiCopy8x8_8u_C1R(pRefV, mStepChroma, mcPred + 64*5, 8);
        }

        if (quant < 8)
          pattern = TransMacroBlockInter_H261_qc(pYc, pUc, pVc, coeffMB, nzCount, &quant, mcPred, bestSAD, bSADU, bSADV);
        else
          pattern = TransMacroBlockInter_H261(pYc, pUc, pVc, coeffMB, nzCount, quant, mcPred, bestSAD, bSADU, bSADV);
        if (pattern == 0)
          quant = mVideoPicture.frame_quant;
        if (pattern == 0 && mvLuma.dx == 0 && mvLuma.dy == 0) {
          // MB not coded
          MBcurr->mv.dx = MBcurr->mv.dy = 0;
          mvPred.dx = mvPred.dy = 0;

          /* Can't be applied in the current configuration - with only 2 frame buffers !!! */
/*        if (MBcurr->not_coded == 0)  */
          { /* the macroblock was coded in the previous frame */
            MBcurr->not_coded = 1;
            ReconMacroBlockNotCoded(pYc, pUc, pVc, mcPred);
          }
          continue;
        }

        MBcurr->type = IPPVC_MBTYPE_INTER;
        MBcurr->mv = mvLuma;

        // RTP support
        mRTPdata.GOBN_MBAP[nmb] = (i << 8) | (prev_mba & 0xFF);
        mRTPdata.MBpos[nmb] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
        mRTPdata.MBquant[nmb] = (Ipp8u)quant;
        mRTPdata.MBpredMV[nmb] = mvPred;

        EncodeMBA(mba - prev_mba);
        prev_mba = mba;

        mtype = 0;
        if (pattern)
          mtype |= MTYPE_TCOEFF;
        if (mvLuma.dx | mvLuma.dy)
          mtype |= MTYPE_MVD;
        if (quant != mVideoPicture.frame_quant) {
          mtype |= MTYPE_MQUANT;
          mVideoPicture.frame_quant = quant;
        }
        if (filter)
          mtype |= (MTYPE_FIL | MTYPE_MVD);
        EncodeMType(mtype);

        if (mtype & MTYPE_MQUANT)
          cBS.PutBits(quant, 5);

        if (mtype & MTYPE_MVD) {
          mvd.dx = mvLuma.dx - mvPred.dx;
          mvd.dy = mvLuma.dy - mvPred.dy;
          EncodeMVD(mvd);
        }
        mvPred = mvLuma;

        if (pattern)
          EncodeCBP(pattern);

        EncodeMacroBlockInter_H261(coeffMB, nzCount);

        if (pattern)
          ReconMacroBlockInter_H261(pYc, pUc, pVc, mcPred, coeffMB, quant, nzCount);
        else
          ReconMacroBlockNotCoded(pYc, pUc, pVc, mcPred);
      }
      nmb++;
    }
    gobx = (i & gobmask) * 11 * 16;
    goby = ((i >> 1) + 1 - gobmask) * 3 * 16 * mStepLuma;
  }
  mRTPdata.num_coded_MB = nmb;
  EncodeZeroBitsAlign();
}

void ippVideoEncoderH261::EncodeIFrame()
{
  Ipp8u  *pYc, *pUc, *pVc;
  Ipp8u  *pgobYc, *pgobUc, *pgobVc;
  Ipp32s i, quant;
  Ipp8u  mba;
  Ipp32s prev_mba;
  Ipp8u  mtype = 0;
  Ipp32s nzCount[6];
  h261_MacroBlock *MBcurr;
  __ALIGN16(Ipp16s, coeffMB, 64*6);
  Ipp32s gobmask = mVideoPicture.source_format == 1 ? 1 : 0;
  Ipp32s gobx = 0, goby = 0;
  Ipp32s nmb = 0;

  for (i = 1; i <= mVideoPicture.num_gobs_in_frame; i++) {

    mVideoPicture.gob_number = i;
    quant = mVideoPicture.frame_quant = mQuantIFrame;

    // RTP support
    mRTPdata.GOBstartPos[i - 1] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
    EncodeGOB_Header();

    pgobYc = mFrameC->pY + goby + gobx;
    pgobUc = mFrameC->pU + (goby >> 2) + (gobx >> 1);
    pgobVc = mFrameC->pV + (goby >> 2) + (gobx >> 1);

    prev_mba = -1;

    for (mba = 0; mba < 33; mba++) {
      Ipp32s mby = (mba / 11) * 16 * mStepLuma;
      Ipp32s mbx = (mba % 11) * 16;

      quant = mQuantIFrame;

      MBcurr = MBinfo + (i - 1)*33 + mba;

      pYc = pgobYc + mby + mbx;
      pUc = pgobUc + (mby >> 2) + (mbx >> 1);
      pVc = pgobVc + (mby >> 2) + (mbx >> 1);

      MBcurr->type = IPPVC_MBTYPE_INTRA;
      MBcurr->mv.dx = MBcurr->mv.dy = 0;

      if (quant < 8)
        TransMacroBlockIntra_H261_qc(pYc, pUc, pVc, coeffMB, nzCount, &quant);
      else
        TransMacroBlockIntra_H261(pYc, pUc, pVc, coeffMB, nzCount, quant);

      // RTP support
      mRTPdata.GOBN_MBAP[nmb] = (i << 8) | (prev_mba & 0xFF);
      mRTPdata.MBpos[nmb] = 8 * (cBS.mPtr - cBS.mBuffer) + cBS.mBitOff;
      mRTPdata.MBquant[nmb] = (Ipp8u)quant;

      EncodeMBA(mba - prev_mba);

      prev_mba = mba;

      mtype = MTYPE_INTRA;
      if (quant != mVideoPicture.frame_quant) {
        mtype |= MTYPE_MQUANT;
        mVideoPicture.frame_quant = quant;
      }
      EncodeMType(mtype);
      if (mtype & MTYPE_MQUANT)
        cBS.PutBits(quant, 5);

      EncodeMacroBlockIntra_H261(coeffMB, nzCount);

      ReconMacroBlockIntra_H261(pYc, pUc, pVc, coeffMB, quant, nzCount);

      MBcurr->not_coded = 0;
      nmb++;
    }
    gobx = (i & gobmask) * 11 * 16;
    goby = ((i >> 1) + 1 - gobmask) * 3 * 16 * mStepLuma;
  }
  mRTPdata.num_coded_MB = nmb; // = mNumMacroBlockPerFrame
  EncodeZeroBitsAlign();
}
#endif // defined (UMC_ENABLE_H261_VIDEO_ENCODER)
