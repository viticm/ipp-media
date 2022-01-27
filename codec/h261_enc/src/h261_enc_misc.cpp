/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH261 (read param, init, constructor, destructor)
//  Contents:
//                  ~ippVideoEncoderH261
//                  ippVideoEncoderH261(h261_Param)
//                  Close
//                  ReadParamFile
//                  Init
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)

#include <math.h>
#include "h261_enc.hpp"


ippVideoEncoderH261::ippVideoEncoderH261(h261_Param *par)
{
  Init(par);
}

ippVideoEncoderH261::~ippVideoEncoderH261()
{
  Close();
}

void ippVideoEncoderH261::Close()
{
  if (mIsInit) {
      // free
    if (mRTPdata.GOBN_MBAP)
      ippsFree(mRTPdata.GOBN_MBAP);
    if (mRTPdata.MBpos)
      ippsFree(mRTPdata.MBpos);
    if (mRTPdata.MBquant)
      ippsFree(mRTPdata.MBquant);
    if (mRTPdata.MBpredMV)
      ippsFree(mRTPdata.MBpredMV);
    if (MBinfo)
      delete [] MBinfo;
    if (mbsAlloc && cBS.mBuffer)
      delete [] cBS.mBuffer;
    if (mMEfastSAD)
      ippsFree(mMEfastSAD);
  }
  mIsInit = false;
}


Ipp32s ippVideoEncoderH261::Init(h261_Param *par)
{
  // check parameters correctness
  if (!((par->Width == 176 && par->Height == 144) || (par->Width == 352 && par->Height == 288))) {
      ErrorMessage(__VM_STRING("Size of picture is incorrect"));
      return H261_STS_ERR_PARAM;
  }
  if (!par->RateControl && (par->quantIFrame < 1 || par->quantIFrame > 31)) {
      ErrorMessage(__VM_STRING("quantIFrame must be between 1 and 31"));
      return H261_STS_ERR_PARAM;
  }
  if (!par->RateControl && (par->quantPFrame < 1 || par->quantPFrame > 31)) {
      ErrorMessage(__VM_STRING("quantPFrame must be between 1 and 31"));
      return H261_STS_ERR_PARAM;
  }
  if (par->IFramedist < 1) {
      ErrorMessage(__VM_STRING("IFramedist must be positive"));
      return H261_STS_ERR_PARAM;
  }
  if (par->PFramesearchWidth < 1 || par->PFramesearchWidth > 15) {
      ErrorMessage(__VM_STRING("PFramesearchWidth must be between 1 and 15"));
      return H261_STS_ERR_PARAM;
  }
  if (par->PFramesearchHeight < 1 || par->PFramesearchHeight > 15) {
      ErrorMessage(__VM_STRING("PFramesearchHeight must be between 1 and 15"));
      return H261_STS_ERR_PARAM;
  }
  Close();
  mIFramedist = par->IFramedist;
  mMEalgorithm = par->MEalgorithm;
  mCalcPSNR = par->calcPSNR;
  mPSNR_Y = mPSNR_U = mPSNR_V = 0;
  mFrameCount = 0;
  mLastIFrame = -mIFramedist;

  mPlanes = mIFramedist > 1 ? 2 : 1;

  // minimal allowed maximum frame size (in bytes)
  mBPPmax = H261_MAX_FRAME_SIZE;
  if ((par->bPP * 1024 >> 3) > mBPPmax)
    mBPPmax = par->bPP * 1024 >> 3;
  if (par->Width < 352)
    mBPPmax >>= 2;

  mRateControl = par->RateControl;
  mBitsEncodedTotal = 0;
  mFrameSkipEnabled = par->FrameSkip;
  mSkipFrame = 0;

  if (mRateControl) {
#if 0
    Ipp64f ppb = (Ipp64f)par->Width * par->Height * 30000 / ((Ipp64f)1001 * par->frameInterval * par->BitRate);
    mBitRate = par->BitRate;
    mQuantIFrame = (Ipp32s)(1.2 * ppb);
    mQuantPFrame = (Ipp32s)(1.5 * ppb);
    h261e_Clip(mQuantIFrame, 1, 31);
    h261e_Clip(mQuantPFrame, 1, 31);
    mBitsPerFrameRC = (Ipp32s)((Ipp64f)mBitRate * 1001 * par->frameInterval / (Ipp64f)30000);
    if (mBitsPerFrameRC > mBPPmax << 3)
      mBitsPerFrameRC = mBPPmax << 3;
    mBitsDesiredRC = 0;
    mBitsEncoded = 0;
#endif
    mBitRate = par->BitRate;
    mBitsDesiredFrame = (Ipp32s)((Ipp64s)mBitRate * 1001 * par->frameInterval / (Ipp64f)30000);
    if (mBitsDesiredFrame > mBPPmax << 3) // ??? mBPPmax can be larger if negotiated by external means
      mBitsDesiredFrame = mBPPmax << 3;
    mBitsDesiredTotal = 0;
    mQuantIFrame = mQuantPFrame = (par->Width * par->Height  / mBitsDesiredFrame) + 1;
    h261e_Clip(mQuantIFrame, 2, 31); // 3,31 ???
    h261e_Clip(mQuantPFrame, 2, 31); // 3,31 ???

    mRCfa = mBitsDesiredFrame;
    mRCfap = 10;
    //      mRCfap = par->TimeResolution / par->TimeIncrement;
    mRCqap = 100;
    mRCbap = 10;
    mRCq = mQuantIFrame;
    mRCqa = 1. / (Ipp64f)mRCq;
  } else {
    mQuantIFrame = par->quantIFrame;
    mQuantPFrame = par->quantPFrame;
  }

  // setup H.261 headers variables
  memset(&mVideoPicture, 0, sizeof(mVideoPicture));
  mPFramesearchHor = par->PFramesearchWidth;
  mPFramesearchVer = par->PFramesearchHeight;

//    mVideoPicture.temporal_reference = 0;  // done by memset above
//    mVideoPicture.split_screen_indicator = 0;
//    mVideoPicture.document_camera_indicator = 0;
//    mVideoPicture.freeze_picture_release = 0;
  mVideoPicture.still_image_mode = 1;

  if (par->Width == 176 && par->Height == 144) {
    mVideoPicture.source_format = 0;
    mVideoPicture.num_gobs_in_frame = 3;
  } else {
    mVideoPicture.source_format = 1;
    mVideoPicture.num_gobs_in_frame = 12;
  }

  mSourceWidth = par->Width;
  mSourceHeight = par->Height;
  mFrameInterval = par->frameInterval;

  mNumMacroBlockPerFrame = mVideoPicture.num_gobs_in_frame * 33;

  mStepLuma = ((mSourceWidth + 15) >> 4) * 16;
  mLumaPlaneSize = mStepLuma * ((mSourceHeight + 15) >> 4) * 16;
  mStepChroma = ((mSourceWidth + 15) >> 4) * 8;
  mChromaPlaneSize = mStepChroma * ((mSourceHeight + 15) >> 4) * 8;
  mIsInit = true;

  mFrameC = &mFrame[0];
  mFrameF = &mFrame[1];

  mRTPdata.GOBN_MBAP = ippsMalloc_16u(mNumMacroBlockPerFrame);
  mRTPdata.MBpos = ippsMalloc_32u(mNumMacroBlockPerFrame);
  mRTPdata.MBquant = ippsMalloc_8u(mNumMacroBlockPerFrame);
  mRTPdata.MBpredMV = (IppMotionVector*)ippsMalloc_8u(mNumMacroBlockPerFrame * sizeof(IppMotionVector));

  // init bitstream buffer
  if (par->bsBuffer && (par->bsBuffSize > 0)) {
    cBS.mBuffer = par->bsBuffer;
    cBS.mBuffSize = par->bsBuffSize;
    mbsAlloc = false;
  } else {
    cBS.mBuffSize = par->Width * par->Height >> 1;
    cBS.mBuffer = new Ipp8u[cBS.mBuffSize];
    mbsAlloc = true;
  }
  cBS.mPtr = cBS.mBuffer;
  MBinfo = new h261_MacroBlock[mNumMacroBlockPerFrame];
  mMEfastSADsize = (mPFramesearchHor * 2 + 1) * (mPFramesearchVer * 2 + 1);
  mMEfastSAD = ippsMalloc_32s(mMEfastSADsize);

  if (!cBS.mBuffer || !MBinfo || !mMEfastSAD ||
    (!mRTPdata.GOBN_MBAP || !mRTPdata.MBpredMV || !mRTPdata.MBquant || !mRTPdata.MBpos)) {
    Close();
    ErrorMessage(__VM_STRING("Not enough memory"));
    return  H261_STS_ERR_NOMEM;
  }
  memset(mRTPdata.GOBN_MBAP, -1, mNumMacroBlockPerFrame * sizeof(Ipp16u));

  // setup thresholds for ME
  mMEthrSAD16x16 = par->quantPFrame >= 6 ? 256 : (4 << par->quantPFrame);
  mMEthrSAD8x8 = mMEthrSAD16x16 >> 2;
  return H261_STS_NOERR;
}


void ippVideoEncoderH261::PostFrameRC()
{
  Ipp64f  bo, qs, dq;
  int     quant, coding_type;

  mSkipFrame = 0;
  mBitsDesiredTotal += mBitsDesiredFrame;
  coding_type = mVideoPicture.picture_coding_type;

  quant = (coding_type == H261_PIC_TYPE_I) ? mQuantIFrame :  mQuantPFrame;

#ifdef PRINT_INFO
  printf("%d  %d %d %d %f \n", mFrameCount, coding_type, quant, mBitsEncodedFrame, mBitsEncodedTotal / (Ipp64f)mBitsDesiredTotal);
#endif

  if (!mFrameSkipped) {
    mRCqa += (1. / quant - mRCqa) / mRCqap;
    h261e_Clip(mRCqa, 1./31. , 1./1.);
    if (coding_type != H261_PIC_TYPE_I || mIFramedist < 5) // ???
      mRCfa += (mBitsEncodedFrame - mRCfa) / mRCfap;
  }
  qs = pow(mBitsDesiredFrame / mRCfa, 1.5);
  dq = mRCqa * qs;
  bo = (Ipp64f)((Ipp64s)mBitsEncodedTotal - (Ipp64s)mBitsDesiredTotal) / mRCbap / mBitsDesiredFrame;

  if (mFrameSkipEnabled && bo > 1.05 && quant == 31) {
    mSkipFrame = 1;
    return;
  }

  h261e_Clip(bo, -1.0, 1.0);
  //dq = dq * (1.0 - bo);
  dq = dq + (1./31 - dq) * bo;

  h261e_ClipL(dq, 1./31.);
  if (bo > -0.05) {
    h261e_ClipR(dq, 1./2.);
  } else {
    h261e_ClipR(dq, 1./1.);
  }
  quant = (int) (1. / dq + 0.5);
  //h261e_Clip(quant, 2, 31);
  h261e_ClipR(quant, 31);
  if (quant >= mRCq + 3)
    quant = mRCq + 2;
  else if (quant > mRCq + 1)
    quant = mRCq + 1;
  else if (quant <= mRCq - 3)
    quant = mRCq - 2;
  else if (quant < mRCq - 1)
    quant = mRCq - 1;
  mRCq = mQuantIFrame = mQuantPFrame = quant;
}

Ipp32s ippVideoEncoderH261::GetCurrentFrameInfo(Ipp8u **pY, Ipp8u **pU, Ipp8u **pV, Ipp32s *stepLuma, Ipp32s *stepChroma)
{
  if (!mIsInit)
    return H261_STS_ERR_NOTINIT;
  *pY = mFrameC->pY;
  *pU = mFrameC->pU;
  *pV = mFrameC->pV;
  *stepLuma = mStepLuma;
  *stepChroma = mStepChroma;
  return H261_STS_NOERR;
}

void ippVideoEncoderH261::ErrorMessage(vm_char *msg)
{
  vm_debug_trace(VM_DEBUG_INFO, __VM_STRING("H.261 encoder error: "));
  vm_debug_trace(VM_DEBUG_INFO, msg);
  vm_debug_trace(VM_DEBUG_INFO, __VM_STRING("\n"));
}
#endif  //defined (UMC_ENABLE_H261_VIDEO_ENCODER)

