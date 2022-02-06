/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH263
//  Contents:
//                  ~ippVideoEncoderH263
//                  ippVideoEncoderH263(h263e_Param)
//                  Close
//                  Init
//                  ExpandFrame
//
*/
#include "umc_config.h"
#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)
#include <math.h>
#include "h263_enc.hpp"

ippVideoEncoderH263::ippVideoEncoderH263(h263e_Param *par)
{
    Init(par);
}

ippVideoEncoderH263::~ippVideoEncoderH263()
{
    Close();
}

void ippVideoEncoderH263::Close()
{
  if (mIsInit) {
    // free
    if (mMBinfo)
      delete [] mMBinfo;
    if (mbsAlloc && cBS.mBuffer)
      ippsFree(cBS.mBuffer);
    if (mBuffer_1)
      ippsFree(mBuffer_1);
    if (mBuffer_2)
      ippsFree(mBuffer_2);
    if (mMEfastSAD)
      ippsFree(mMEfastSAD);
    if (mRTPdata.GOBstartPos)
      ippsFree(mRTPdata.GOBstartPos);
    if (mRTPdata.MBpos)
      ippsFree(mRTPdata.MBpos);
    if (mRTPdata.MBquant)
      ippsFree(mRTPdata.MBquant);
    if (mRTPdata.MBpredMV)
      ippsFree(mRTPdata.MBpredMV);
    if (mRTPdata.MBpredMV1)
      ippsFree(mRTPdata.MBpredMV1);
    if (mFrame)
      delete [] mFrame;
  }
  mIsInit = false;
}

Ipp32s ippVideoEncoderH263::Init(h263e_Param *par)
{
    Ipp32s  i, j;
    IppStatus stsAlloc = ippStsNoErr;

    // check parameters correctness
    if (par->Width < 1 || par->Width > 8191) {
        ErrorMessage(VM_STRING("Width must be between 1 and 8191"));
        return H263_STS_ERR_PARAM;
    }
    if (par->Width & 1) {
        ErrorMessage(VM_STRING("Width must be a even"));
        return H263_STS_ERR_PARAM;
    }
    if (par->Height < 1 || par->Height > 8191) {
        ErrorMessage(VM_STRING("Height must be between 1 and 8191"));
        return H263_STS_ERR_PARAM;
    }
    if (par->Height & 1) {
        ErrorMessage(VM_STRING("Height must be a even"));
        return H263_STS_ERR_PARAM;
    }
    if (!par->RateControl && (par->quantIPic < 1 || par->quantIPic > 31)) {
        ErrorMessage(VM_STRING("quantIPic must be between 1 and 31"));
        return H263_STS_ERR_PARAM;
    }
    if (!par->RateControl && (par->quantPPic < 1 || par->quantPPic > 31)) {
        ErrorMessage(VM_STRING("quantPPic must be between 1 and 31"));
        return H263_STS_ERR_PARAM;
    }
/*
    if (!par->RateControl && (par->quantBPic < 1 || par->quantBPic > 31)) {
        ErrorMessage(VM_STRING("quantBPic must be between 1 and 31"));
        return H263_STS_ERR_PARAM;
    }
*/
    if (par->IPicdist < 1) {
        ErrorMessage(VM_STRING("IPicdist must be positive"));
        return H263_STS_ERR_PARAM;
    }
    if (par->RateControl && par->BitRate <= 0) {
        ErrorMessage(VM_STRING("BitRate must be positive"));
        return H263_STS_ERR_PARAM;
    }
    if (par->SceneChangeThreshold < 0 || par->SceneChangeThreshold > 100) {
        ErrorMessage(VM_STRING("SceneChangeThreshold must be between 0 and 100"));
        return H263_STS_ERR_PARAM;
    }
    if (0) { // ??? tmp
        if (par->TimeResolution != 30000) {
            ErrorMessage(VM_STRING("TimeResolution must equal 30000 in the current implementation")); // ???
            return H263_STS_ERR_PARAM;
        }
        if ((par->TimeIncrement % 1001) || par->TimeIncrement < 1001 || par->TimeIncrement > 256 * 1001) {
            ErrorMessage(VM_STRING("TimeIncrement must equal i*1001 (i=1,256) in the current implementation")); // ???
            return H263_STS_ERR_PARAM;
        }
    } else {
        if (par->TimeResolution < 1) {
            ErrorMessage(VM_STRING("TimeResolution must be positive"));
            return H263_STS_ERR_PARAM;
        }
        if (par->TimeIncrement < 1) {
            ErrorMessage(VM_STRING("TimeIncrement must be positive"));
            return H263_STS_ERR_PARAM;
        }
        if (par->PPicdist < 1) {
            ErrorMessage(VM_STRING("PPicdist must be positive"));
            return H263_STS_ERR_PARAM;
        }
        if (par->IPicdist % par->PPicdist != 0) {
            ErrorMessage(VM_STRING("IPicdist must be an integer multiple of PPicdist"));
            return H263_STS_ERR_PARAM;
        }
        if (par->PPicsearchWidth < 1 || par->PPicsearchWidth > IPP_MIN(1023, par->Width)) {
            ErrorMessage(VM_STRING("PPicsearchWidth must be between 1 and MIN(1023, Width)"));
            return H263_STS_ERR_PARAM;
        }
        if (par->PPicsearchHeight < 1 || par->PPicsearchHeight > IPP_MIN(1023, par->Height)) {
            ErrorMessage(VM_STRING("PPicsearchWidth must be between 1 and MIN(1023, Height)"));
            return H263_STS_ERR_PARAM;
        }
/*
        if (par->BPicsearchWidthForw < 1 || par->BPicsearchWidthForw > IPP_MIN(1023, par->Width)) {
            ErrorMessage(VM_STRING("BPicsearchWidthForw must be between 1 and MIN(1023, Width)"));
            return H263_STS_ERR_PARAM;
        }
        if (par->BPicsearchHeightForw < 1 || par->BPicsearchHeightForw > IPP_MIN(1023, par->Height)) {
            ErrorMessage(VM_STRING("BPicsearchWidthForw must be between 1 and MIN(1023, Height)"));
            return H263_STS_ERR_PARAM;
        }
        if (par->BPicsearchWidthBack < 1 || par->BPicsearchWidthBack > IPP_MIN(1023, par->Width)) {
            ErrorMessage(VM_STRING("BPicsearchWidthBack must be between 1 and MIN(1023, Width)"));
            return H263_STS_ERR_PARAM;
        }
        if (par->BPicsearchHeightBack < 1 || par->BPicsearchHeightBack > IPP_MIN(1023, par->Height)) {
            ErrorMessage(VM_STRING("BPicsearchWidthBack must be between 1 and MIN(1023, Height)"));
            return H263_STS_ERR_PARAM;
        }
*/
    }
    Close();

    memset(&mVideoSequence, 0, sizeof(mVideoSequence));
    memset(&mVideoPicture, 0, sizeof(mVideoPicture));
    mVideoSequence.pic_time_increment_resolution = par->TimeResolution;
    mVideoSequence.fixed_pic_time_increment = par->TimeIncrement;

    {
      Ipp64f finc = mVideoSequence.fixed_pic_time_increment * 0.001;
      if (finc == (Ipp32s)finc || mVideoSequence.pic_time_increment_resolution != 30000) {
        mVideoPicture.PCF = 1;
        mVideoPicture.temporal_reference_increment = 1;
        if (finc == (Ipp32s)finc) {
          mVideoPicture.clock_conversion_code = 1000;
          mVideoPicture.clock_divisor = (Ipp32s)finc;
        } else {
          mVideoPicture.clock_conversion_code = 1001;
          mVideoPicture.clock_divisor = mVideoSequence.fixed_pic_time_increment / mVideoPicture.clock_conversion_code;
        }
        if (mVideoPicture.clock_divisor > 127) {
          mVideoPicture.temporal_reference_increment = mVideoPicture.clock_divisor;
          mVideoPicture.clock_divisor = 1;
          if (mVideoPicture.temporal_reference_increment > 1023) {
            Ipp64f fcld = sqrt((Ipp64f)mVideoPicture.temporal_reference_increment);
            Ipp32s tinc, cld, prod, bestdiff;
            finc = fcld;
            if (fcld > 127.0) {
              finc = finc * fcld / 127.0;
              fcld = 127.0;
            }
            tinc = (Ipp32s)finc + 1;
            if (tinc > 1023)
              tinc = 1023;
            cld = (Ipp32s)fcld;
            prod = tinc * cld;
            bestdiff = h263e_Abs(mVideoPicture.temporal_reference_increment - prod);
            if (cld < 127 && bestdiff > h263e_Abs(mVideoPicture.temporal_reference_increment - prod - tinc))
              cld++;
            else if (bestdiff > h263e_Abs(mVideoPicture.temporal_reference_increment - prod + cld))
              tinc--;

            mVideoPicture.temporal_reference_increment = tinc;
            mVideoPicture.clock_divisor = cld;
            mVideoSequence.fixed_pic_time_increment = par->TimeIncrement = tinc * cld * mVideoPicture.clock_conversion_code;
          }
        }
      } else {
        mVideoPicture.PCF = 0;
        mVideoPicture.temporal_reference_increment = mVideoSequence.fixed_pic_time_increment / 1001;
//        if (mVideoPicture.temporal_reference_increment > 255) // checked earlier
//          mVideoPicture.temporal_reference_increment = 255;
      }
    }

    {
      Ipp32s picSize = par->Width * par->Height;
      // H.263 Spec, Table 1
      if (picSize < 25360)
        mBPPmax = 64 << 10;
      else if (picSize < 101392)
        mBPPmax = 256 << 10;
      else if (picSize < 405520)
        mBPPmax = 512 << 10;
      else
        mBPPmax = 1024 << 10;
    }

    //mNumOfFrames = par->NumOfFrames;
//    mNumOfFrames = -1; ???
    mSkipFrame = 0;
    mRateControl = par->RateControl;
    mFrameSkipEnabled = par->FrameSkip;

    mBitsEncodedTotal = 0;
    if (mRateControl) {
      mBitRate = par->BitRate;
      mBitsDesiredFrame = (Ipp32s)((Ipp64s)mBitRate * par->TimeIncrement / par->TimeResolution);
      if (mBitsDesiredFrame > mBPPmax) // ??? mBPPmax can be larger if negotiated by external means
        mBitsDesiredFrame = mBPPmax;
      mBitsDesiredTotal = 0;
      mQuantIPic = mQuantPPic = (par->Width * par->Height  / mBitsDesiredFrame) + 1;
      h263e_Clip(mQuantIPic, 2, 31); // 3,31 ???
      h263e_Clip(mQuantPPic, 2, 31); // 3,31 ???
      mQuantBPic = (mQuantPPic * 5 >> 2) + 1;
      h263e_Clip(mQuantBPic, 2, 31);

      mRCfa = mBitsDesiredFrame;
      mRCfap = 10;
//      mRCfap = par->TimeResolution / par->TimeIncrement;
      mRCqap = 100;
      mRCbap = 10;
      mRCq = mQuantIPic;
      mRCqa = 1. / (Ipp64f)mRCq;
    } else {
        mQuantIPic = par->quantIPic;
        mQuantPPic = par->quantPPic;
//        mQuantBPic = par->quantBPic;
    }
    mIPicdist = par->IPicdist;
    mPPicdist = par->PPicdist;
    mBPicdist = mPPicdist - 1;
    mMEalgorithm = par->MEalgorithm;
    mMEaccuracy = par->MEaccuracy;
    mMEfastHP = 0;  // using fast algorithm for halfpel MVs (faster but with lower PSNR and compression)

    mGOBheaders = par->GOBheaders;
    mGSTUF = par->GOBheaders >= 2;

    mCalcPSNR = par->calcPSNR;
    mFrameCount = 0;
    mLastIPic = -mIPicdist;

    mVideoSequence.aspect_ratio_info = H263e_ASPECT_RATIO_1_1;

    mVideoPicture.pic_width = par->Width;
    mVideoPicture.pic_height = par->Height;

    mVideoPicture.advIntra = par->advIntra ? 1 : 0;
    mVideoPicture.UMV = par->UMV;
    mVideoPicture.advPred = par->advPred ? 1 : 0;
    mVideoPicture.modQuant = par->modQuant ? 1 : 0;
//    mVideoPicture.deblockFilt = par->deblockFilt;
    mVideoPicture.deblockFilt = 0;

//    mVideoSequence.newpred_enable = 0;
    //f mVideoSequence.requested_upstream_message_type;
    //f mVideoSequence.newpred_segment_type;
//    mVideoSequence.reduced_resolution_pic_enable = 0;
//    mVideoSequence.scalability = 0;
    //f mVideoSequence.ScalabilityParameters;
    mVideoPicture.pic_time_increment = 0;
    mVideoPicture.pic_rounding_type = 0;
    mPPicsearchHor = par->PPicsearchWidth;
    mPPicsearchVer = par->PPicsearchHeight;
    // calc pic_fcode_forward for PPics
    i = IPP_MAX(mPPicsearchHor, mPPicsearchVer);
//    j = i << 1;
//    mPPicfcodeForw = 1;
//    while (j > ((16 << mPPicfcodeForw) - 1))
//        mPPicfcodeForw ++;
    {
        mPPicdist = 1;
        mBPicdist = 0;
        mVideoSequence.data_partitioned = 0;
//        mVideoSequence.scalability = 0;
        mVideoPicture.pic_rounding_type = 0;
        mVideoPicture.split_screen_indicator = 0;
        mVideoPicture.document_camera_indicator = 0;
        mVideoPicture.full_picture_freeze_release = 0;
        if (par->Width == 128 && par->Height == 96) {
            mVideoPicture.source_format = 1;
            mVideoPicture.num_gobs_in_pic = 6;
            mVideoPicture.num_macroblocks_in_gob = 8;
            mVideoPicture.num_MBrows_in_gob = 1;
        } else if (par->Width == 176 && par->Height == 144) {
            mVideoPicture.source_format = 2;
            mVideoPicture.num_gobs_in_pic = 9;
            mVideoPicture.num_macroblocks_in_gob = 11;
            mVideoPicture.num_MBrows_in_gob = 1;
        } else if (par->Width == 352 && par->Height == 288) {
            mVideoPicture.source_format = 3;
            mVideoPicture.num_gobs_in_pic = 18;
            mVideoPicture.num_macroblocks_in_gob = 22;
            mVideoPicture.num_MBrows_in_gob = 1;
        } else if (par->Width == 704 && par->Height == 576) {
            mVideoPicture.source_format = 4;
            mVideoPicture.num_gobs_in_pic = 18;
            mVideoPicture.num_macroblocks_in_gob = 88;
            mVideoPicture.num_MBrows_in_gob = 2;
        } else if (par->Width == 1408 && par->Height == 1152) {
            mVideoPicture.source_format = 5;
            mVideoPicture.num_gobs_in_pic = 18;
            mVideoPicture.num_macroblocks_in_gob = 352;
            mVideoPicture.num_MBrows_in_gob = 4;
        } else {
          if ((par->Width | par->Height) & 3) {
            ErrorMessage(VM_STRING("Picture height and width must be divisible by 4"));
            return H263_STS_ERR_PARAM;
          }
          mVideoPicture.source_format = 7;
          mVideoPicture.num_MBrows_in_gob = par->Height <= 400 ? 1 : (par->Height <= 800 ? 2 : 4);
          mVideoPicture.num_gobs_in_pic = (par->Height + 15) >> (4 + (mVideoPicture.num_MBrows_in_gob >> 1));
          mVideoPicture.num_macroblocks_in_gob = ((par->Width + 15) >> 4) << (mVideoPicture.num_MBrows_in_gob >> 1);
        }

        mVideoPicture.PAR_width = par->PAR_width;
        mVideoPicture.PAR_height = par->PAR_height;

        switch (mVideoPicture.PAR_height) {
        case 11:
          switch (mVideoPicture.PAR_width) {
          case 12:
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_12_11;
            break;
          case 10:
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_10_11;
            break;
          case 16:
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_16_11;
            break;
          default:
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_EXTPAR;
            break;
          }
          break;
        case 1:
          if (mVideoPicture.PAR_width == 1)
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_1_1;
          else
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_EXTPAR;
          break;
        case 33:
          if (mVideoPicture.PAR_width == 40)
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_40_33;
          else
            mVideoPicture.PAR_code = H263e_ASPECT_RATIO_EXTPAR;
          break;
        default:
          mVideoPicture.PAR_code = H263e_ASPECT_RATIO_EXTPAR;
          break;
        }

        if (mVideoPicture.PAR_code != H263e_ASPECT_RATIO_12_11) //custom picture format
          mVideoPicture.source_format = 7;


//        mVideoPicture.temporal_reference_increment = mVideoSequence.fixed_pic_time_increment / 1001;


        if (mMEaccuracy > 2)
            mMEaccuracy = 2;
//        mBPicsearchHorForw = mBPicsearchVerForw = mBPicsearchHorBack = mBPicsearchVerBack = 0;
        mME4mv = 0;
    }
#if 0
    else {
        mBPicsearchHorForw = par->BPicsearchWidthForw;
        mBPicsearchVerForw = par->BPicsearchHeightForw;
        mBPicsearchHorBack = par->BPicsearchWidthBack;
        mBPicsearchVerBack = par->BPicsearchHeightBack;
        // calc pic_fcode_forward for BPics
        i = IPP_MAX(mBPicsearchHorForw, mBPicsearchVerForw);
        j = i << 1;
        mBPicfcodeForw = 1;
        while (j > ((16 << mBPicfcodeForw) - 1))
            mBPicfcodeForw ++;
        // calc pic_fcode_backward for BPics
        i = IPP_MAX(mBPicsearchHorBack, mBPicsearchHorBack);
        j = i << 1;
        mBPicfcodeBack = 1;
        while (j > ((16 << mBPicfcodeBack) - 1))
            mBPicfcodeBack ++;
    }
#endif
    mPlanes = (mIPicdist == 1) ? 1 : (mBPicdist == 0) ? 2 : 2 + mBPicdist + 1;
    mSourceWidth = par->Width;
    mSourceHeight = par->Height;
    mSourceFormat = YUV_CHROMA_420;
    mExpandSize = 16;
    mExpandSizeA = (mExpandSize + 15) & (~15);
    mNumMacroBlockPerRow = (mSourceWidth + 15) / 16;
    mNumMacroBlockPerCol = (mSourceHeight + 15) / 16;
    mNumMacroBlockPerPic = mNumMacroBlockPerRow * mNumMacroBlockPerCol;
    mStepLuma = mExpandSizeA * 2 + mNumMacroBlockPerRow * 16;
    mLumaPlaneSize = mStepLuma * (mExpandSizeA * 2 + mNumMacroBlockPerCol * 16);
    mStepChroma = (mExpandSizeA / 2) * 2 + mNumMacroBlockPerRow * 8;
    mChromaPlaneSize = mStepChroma * ((mExpandSizeA / 2) * 2 + mNumMacroBlockPerCol * 8);
    mSceneChangeThreshold = (mNumMacroBlockPerPic * par->SceneChangeThreshold + 50) / 100;
    mIsInit = true;
    // buffers allocation
    mFrame = new h263e_Frame[mPlanes];
    if (!mFrame)
      stsAlloc = ippStsMemAllocErr;
    mRTPdata.MBpos = ippsMalloc_32u(mNumMacroBlockPerPic);
    mRTPdata.MBquant = ippsMalloc_8u(mNumMacroBlockPerPic);
    mRTPdata.MBpredMV = (IppMotionVector*)ippsMalloc_8u(mNumMacroBlockPerPic * sizeof(IppMotionVector));
    if (mVideoPicture.advPred) {
      mRTPdata.MBpredMV1 = (IppMotionVector*)ippsMalloc_8u(mNumMacroBlockPerPic * sizeof(IppMotionVector));
      if (!mRTPdata.MBpredMV1)
        stsAlloc = ippStsMemAllocErr;
    } else
      mRTPdata.MBpredMV1 = NULL;
    mRTPdata.GOBstartPos =  ippsMalloc_32u(mVideoPicture.num_gobs_in_pic);
    if (!mRTPdata.MBpos || !mRTPdata.MBquant || !mRTPdata.MBpredMV || !mRTPdata.GOBstartPos)
      stsAlloc = ippStsMemAllocErr;
    mRTPdata.codingModes = 0; // forbidden src format 000 to be returned if plusptype is present
    // init bitstream buffer
    if (par->bsBuffer && (par->bsBuffSize > 0)) {
        cBS.mBuffer = par->bsBuffer;
        cBS.mBuffSize = par->bsBuffSize;
        mbsAlloc = false;
    } else {
        cBS.mBuffSize = mSourceWidth * mSourceHeight;
        cBS.mBuffer = ippsMalloc_8u(cBS.mBuffSize);
        if (!cBS.mBuffer)
          stsAlloc = ippStsMemAllocErr;
        mbsAlloc = true;
    }
    cBS.mPtr = cBS.mBuffer;
    if (mVideoSequence.data_partitioned) {
        mBuffer_1 = ippsMalloc_8u(mSourceWidth * mSourceHeight >> 1);
        mBuffer_2 = ippsMalloc_8u(mSourceWidth * mSourceHeight >> 1);
        if (!mBuffer_1 || !mBuffer_2)
          stsAlloc = ippStsMemAllocErr;
    } else {
        mBuffer_1 = mBuffer_2 = NULL;
    }
    mMBinfo = new h263e_MacroBlock[mNumMacroBlockPerPic];
//    mMEfastSADsize = (IPP_MAX(IPP_MAX(mPPicsearchHor, mBPicsearchHorForw), mBPicsearchHorBack) * 2 + 1) * (IPP_MAX(IPP_MAX(mPPicsearchVer, mBPicsearchVerForw), mBPicsearchVerBack) * 2 + 1);
    mMEfastSADsize = (mPPicsearchHor * 2 + 1) * (mPPicsearchVer * 2 + 1);
    mMEfastSAD = ippsMalloc_32s(mMEfastSADsize);
    if (!mMBinfo || !mMEfastSAD)
      stsAlloc = ippStsMemAllocErr;

    if (stsAlloc != ippStsNoErr) {
      Close();
      ErrorMessage(VM_STRING("Not enough memory"));
      return H263_STS_ERR_NOMEM;
    }

    {
      h263e_MacroBlock *mbCurr = mMBinfo;
      for (i = 0; i < mNumMacroBlockPerCol; i ++) {
          for (j = 0; j < mNumMacroBlockPerRow; j ++) {
            h263e_MacroBlock *mbA = mbCurr - 1;
            h263e_MacroBlock *mbC = mbCurr - mNumMacroBlockPerRow;
            mbCurr->block[0].predA = &mbA->block[1];
            mbCurr->block[0].predC = &mbC->block[2];
            mbCurr->block[1].predA = &mbCurr->block[0];
            mbCurr->block[1].predC = &mbC->block[3];
            mbCurr->block[2].predA = &mbA->block[3];
            mbCurr->block[2].predC = &mbCurr->block[0];
            mbCurr->block[3].predA = &mbCurr->block[2];
            mbCurr->block[3].predC = &mbCurr->block[1];
            mbCurr->block[4].predA = &mbA->block[4];
            mbCurr->block[4].predC = &mbC->block[4];
            mbCurr->block[5].predA = &mbA->block[5];
            mbCurr->block[5].predC = &mbC->block[5];
            mbCurr++;
          }
      }

      for (j = 0; j < mNumMacroBlockPerRow; j ++) {
        h263e_MacroBlock *mbCurr = &mMBinfo[j];
        mbCurr->block[0].predC = NULL;
        mbCurr->block[1].predC = NULL;
        mbCurr->block[4].predC = NULL;
        mbCurr->block[5].predC = NULL;
      }

      for (i = 0; i < mNumMacroBlockPerCol; i ++) {
        h263e_MacroBlock *mbCurr = &mMBinfo[i*mNumMacroBlockPerRow];
        mbCurr->block[0].predA = NULL;
        mbCurr->block[2].predA = NULL;
        mbCurr->block[4].predA = NULL;
        mbCurr->block[5].predA = NULL;
      }
    }
    // setup frames
    if (mBPicdist == 0) {
      mFrameC = &mFrame[0];
      if (mIPicdist > 1)
        mFrameF = &mFrame[1];
    } else {
      mFrameC = &mFrame[0];
      mFrameF = &mFrame[1];
      mFrameB = &mFrame[0];
      mIndxBPic = 2;
      mNumBPic = 0;
    }
    mPictime = mSyncTime = mSyncTimeB = 0;
    return H263_STS_NOERR;
}


//-----------------------------------------------------------------------------

void ippVideoEncoderH263::PostFrameRC()
{
  Ipp64f  bo, qs, dq;
  int     quant, coding_type;

  mSkipFrame = 0;
  mBitsDesiredTotal += mBitsDesiredFrame;
  coding_type = mVideoPicture.picture_coding_type;

  quant = (coding_type == H263e_PIC_TYPE_I) ? mQuantIPic : (coding_type == H263e_PIC_TYPE_B) ? mQuantBPic : mQuantPPic;

#ifdef PRINT_INFO
  printf("%d  %d %d %d %f \n", mFrameCount, coding_type, quant, mBitsEncodedFrame, mBitsEncodedTotal / (Ipp64f)mBitsDesiredTotal);
#endif

  if (!mFrameSkipped) {
    mRCqa += (1. / quant - mRCqa) / mRCqap;
    h263e_Clip(mRCqa, 1./31. , 1./1.);
    if (coding_type != H263e_PIC_TYPE_I || mIPicdist < 5) // ???
      mRCfa += (mBitsEncodedFrame - mRCfa) / mRCfap;
    if (coding_type == H263e_PIC_TYPE_B) {
      quant = (mQuantPPic * 5 >> 2) + 1;
      h263e_Clip(quant, 2, 31);
      mQuantBPic = quant;
      return;
    }
  }
  qs = pow(mBitsDesiredFrame / mRCfa, 1.5);
  dq = mRCqa * qs;
  bo = (Ipp64f)((Ipp64s)mBitsEncodedTotal - (Ipp64s)mBitsDesiredTotal) / mRCbap / mBitsDesiredFrame;

  if (mFrameSkipEnabled && bo > 1.05 && quant == 31) {
    mSkipFrame = 1;
    return;
  }

  h263e_Clip(bo, -1.0, 1.0);
  //dq = dq * (1.0 - bo);
  dq = dq + (1./31 - dq) * bo;

  h263e_ClipL(dq, 1./31.);
  if (bo > -0.05) {
    h263e_ClipR(dq, 1./2.);
  } else {
    h263e_ClipR(dq, 1./1.);
  }
  quant = (int) (1. / dq + 0.5);
  //h263e_Clip(quant, 2, 31);
  if (quant >= mRCq + 3)
    quant = mRCq + 2;
  else if (quant > mRCq + 1)
    quant = mRCq + 1;
  else if (quant <= mRCq - 3)
    quant = mRCq - 2;
  else if (quant < mRCq - 1)
    quant = mRCq - 1;
  mRCq = mQuantIPic = mQuantPPic = quant;
}

void ippVideoEncoderH263::ErrorMessage(const vm_char *msg)
{
    printf("%s\n", msg);
    /*
    vm_debug_trace(VM_DEBUG_INFO, __VM_STRING("H.263 encoder error: "));
    vm_debug_trace(VM_DEBUG_INFO, msg);
    vm_debug_trace(VM_DEBUG_INFO, __VM_STRING("\n"));
    */
}

static void h263e_ExpandFrameReplicate(Ipp8u *pSrcDstPlane, Ipp32s frameWidth, Ipp32s frameHeight, Ipp32s expandPels, Ipp32s step)
{
    Ipp8u   *pDst1, *pDst2, *pSrc1, *pSrc2;
    Ipp32s  i, j;
    Ipp32u  t1, t2;

    pDst1 = pSrcDstPlane + step * expandPels;
    pDst2 = pDst1 + frameWidth + expandPels;
    if (expandPels == 8) {
        for (i = 0; i < frameHeight; i ++) {
            t1 = pDst1[8] + (pDst1[8] << 8);
            t2 = pDst2[-1] + (pDst2[-1] << 8);
            t1 = (t1 << 16) + t1;
            t2 = (t2 << 16) + t2;
            ((Ipp32u*)pDst1)[0] = t1;
            ((Ipp32u*)pDst1)[1] = t1;
            ((Ipp32u*)pDst2)[0] = t2;
            ((Ipp32u*)pDst2)[1] = t2;
            pDst1 += step;
            pDst2 += step;
        }
    } else if (expandPels == 16) {
        for (i = 0; i < frameHeight; i ++) {
            t1 = pDst1[16] + (pDst1[16] << 8);
            t2 = pDst2[-1] + (pDst2[-1] << 8);
            t1 = (t1 << 16) + t1;
            t2 = (t2 << 16) + t2;
            ((Ipp32u*)pDst1)[0] = t1;
            ((Ipp32u*)pDst1)[1] = t1;
            ((Ipp32u*)pDst1)[2] = t1;
            ((Ipp32u*)pDst1)[3] = t1;
            ((Ipp32u*)pDst2)[0] = t2;
            ((Ipp32u*)pDst2)[1] = t2;
            ((Ipp32u*)pDst2)[2] = t2;
            ((Ipp32u*)pDst2)[3] = t2;
            pDst1 += step;
            pDst2 += step;
        }
    } else {
        for (i = 0; i < frameHeight; i ++) {
            ippsSet_8u(pDst1[expandPels], pDst1, expandPels);
            ippsSet_8u(pDst2[-1], pDst2, expandPels);
            pDst1 += step;
            pDst2 += step;
        }
    }
    pDst1 = pSrcDstPlane;
    pSrc1 = pSrcDstPlane + expandPels * step;
    pDst2 = pSrc1 + frameHeight * step;
    pSrc2 = pDst2 - step;
    j = frameWidth + 2 * expandPels;
    for (i = 0; i < expandPels; i ++) {
        ippsCopy_8u(pSrc1, pDst1, j);
        ippsCopy_8u(pSrc2, pDst2, j);
        pDst1 += step;
        pDst2 += step;
    }
}

/*
//  padding frame: replication
*/
void ippVideoEncoderH263::ExpandFrame(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV)
{
  if (mExpandSize) {
    Ipp32s  wL, hL, wC, hC, es = (mExpandSize + 1) >> 1;

    wL = mNumMacroBlockPerRow * 16;
    hL = mNumMacroBlockPerCol * 16;
    wC = mSourceWidth >> 1;
    hC = mSourceHeight >> 1;

    h263e_ExpandFrameReplicate(pY-mExpandSize*mStepLuma-mExpandSize, wL, hL, mExpandSize, mStepLuma);
    h263e_ExpandFrameReplicate(pU-es*mStepChroma-es, wC, hC, es, mStepChroma);
    h263e_ExpandFrameReplicate(pV-es*mStepChroma-es, wC, hC, es, mStepChroma);
  }
}

Ipp32s ippVideoEncoderH263::GetCurrentFrameInfo(Ipp8u **pY, Ipp8u **pU, Ipp8u **pV, Ipp32s *stepLuma, Ipp32s *stepChroma)
{
  if (!mIsInit)
    return H263_STS_ERR_NOTINIT;
  *pY = mFrameC->pY;
  *pU = mFrameC->pU;
  *pV = mFrameC->pV;
  *stepLuma = mStepLuma;
  *stepChroma = mStepChroma;
  return H263_STS_NOERR;
}

void ippVideoEncoderH263::OBMC_Macroblock(h263e_MacroBlock *pMBinfo, IppMotionVector *mvCur,
                                          Ipp32s colNum, Ipp32s rowNum, IppiRect limitRectL,
                                          Ipp8u *pYr, Ipp32s stepYr, Ipp8u *pYc, Ipp32s stepYc)
{
  IppMotionVector mvOBMCL, mvOBMCU, mvOBMCR, mvOBMCB, *mvLeft, *mvUpper, *mvRight;
  Ipp32s mbPerRow = mNumMacroBlockPerRow, dx, dy, rt;

  /* get Right MV */
  if (colNum == mNumMacroBlockPerRow - 1)
    mvRight = &mvCur[1];
//  else if ((pMBinfo[1].type & 0xC0) == 0x80) /* INTRA(_Q), no vector for B-part */
  else if (pMBinfo[1].type == IPPVC_MBTYPE_INTRA || pMBinfo[1].type == IPPVC_MBTYPE_INTRA_Q)
    mvRight = &mvCur[1];
  else
    mvRight = pMBinfo[1].mv;
  /* get Left MV */
  if (colNum == 0)
    mvLeft = mvCur - 1;
//  else if ((pMBinfo[-1].type & 0xC0) == 0x80)
  else if (pMBinfo[-1].type == IPPVC_MBTYPE_INTRA || pMBinfo[1].type == IPPVC_MBTYPE_INTRA_Q)
    mvLeft = mvCur - 1;
  else
    mvLeft = pMBinfo[-1].mv;
  /* get Upper MV */
  if (rowNum == 0)
    mvUpper = mvCur - 2;
//  else if ((pMBinfo[-mNumMacroBlockPerRow].type & 0xC0) == 0x80)
  else if (pMBinfo[-mNumMacroBlockPerRow].type == IPPVC_MBTYPE_INTRA || pMBinfo[-mNumMacroBlockPerRow].type == IPPVC_MBTYPE_INTRA_Q)
    mvUpper = mvCur - 2;
  else
    mvUpper = pMBinfo[-mNumMacroBlockPerRow].mv;

  dx = colNum * 16;
  dy = rowNum * 16;
  rt = mVideoPicture.pic_rounding_type;

  h263e_LimitMV(&mvLeft[1], &mvOBMCL, &limitRectL, dx, dy, 8);
  h263e_LimitMV(&mvUpper[2], &mvOBMCU, &limitRectL, dx, dy, 8);
  h263e_LimitMV(&mvCur[1], &mvOBMCR, &limitRectL, dx, dy, 8);
  h263e_LimitMV(&mvCur[2], &mvOBMCB, &limitRectL, dx, dy, 8);
  ippiOBMC8x8HP_MPEG4_8u_C1R(pYr, stepYr, pYc, stepYc, &mvCur[0], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
  h263e_LimitMV(&mvCur[0], &mvOBMCL, &limitRectL, dx+8, dy, 8);
  h263e_LimitMV(&mvUpper[3], &mvOBMCU, &limitRectL, dx+8, dy, 8);
  h263e_LimitMV(&mvRight[0], &mvOBMCR, &limitRectL, dx+8, dy, 8);
  h263e_LimitMV(&mvCur[3], &mvOBMCB, &limitRectL, dx+8, dy, 8);
  ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+8, stepYr, pYc+8, stepYc, &mvCur[1], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvOBMCB, rt);
  h263e_LimitMV(&mvLeft[3], &mvOBMCL, &limitRectL, dx, dy+8, 8);
  h263e_LimitMV(&mvCur[0], &mvOBMCU, &limitRectL, dx, dy+8, 8);
  h263e_LimitMV(&mvCur[3], &mvOBMCR, &limitRectL, dx, dy+8, 8);
  ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+stepYr*8, stepYr, pYc+stepYc*8, stepYc, &mvCur[2], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[2], rt);
  h263e_LimitMV(&mvCur[2], &mvOBMCL, &limitRectL, dx+8, dy+8, 8);
  h263e_LimitMV(&mvCur[1], &mvOBMCU, &limitRectL, dx+8, dy+8, 8);
  h263e_LimitMV(&mvRight[2], &mvOBMCR, &limitRectL, dx+8, dy+8, 8);
  ippiOBMC8x8HP_MPEG4_8u_C1R(pYr+8+stepYr*8, stepYr, pYc+8+stepYc*8, stepYc, &mvCur[3], &mvOBMCL, &mvOBMCR, &mvOBMCU, &mvCur[3], rt);
}
#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
