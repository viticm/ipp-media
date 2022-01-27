/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH261 (based on class ippVideoEncoder)
//
*/

#include "h261_enc_bitstream.hpp"
#include "ippdefs.h"
#include "ipps.h"
#include "ippi.h"
#include "ippvc.h"
#include "vm_debug.h"

#pragma warning(disable : 4514)     // unreferenced inline function has been removed

#define H261_MAX_FRAME_SIZE (256*1024 >> 3) // ???

#define MTYPE_MQUANT  1
#define MTYPE_MVD     2
#define MTYPE_TCOEFF  4
#define MTYPE_FIL     8
#define MTYPE_INTRA   16

enum {
  H261_PIC_TYPE_I  = 0,
  H261_PIC_TYPE_P  = 1
};

/* MacroBlock Info */
struct h261_MacroBlock {
  Ipp8u           type;
  Ipp8u           not_coded;
  IppMotionVector mv;
};

typedef struct {
  Ipp8u val;
  Ipp8u len;
} VLCcode;

/* Video Picture Info */
struct h261_VideoPicture {
  Ipp32s      frame_quant;
  Ipp32s      temporal_reference;
  Ipp32s      split_screen_indicator;
  Ipp32s      document_camera_indicator;
  Ipp32s      freeze_picture_release;
  Ipp32s      source_format;
  Ipp32s      still_image_mode;
  Ipp32s      picture_coding_type;
  Ipp32s      gob_number;
  Ipp32s      num_gobs_in_frame;
//  Ipp32s      gob_frame_id;
//  Ipp32s      prev_mba;
};

struct h261_Param
{
  Ipp32s      Width;
  Ipp32s      Height;
  //Ipp32s      NumOfFrames;
  Ipp32s      quantIFrame, quantPFrame;
  Ipp32s      IFramedist;
  Ipp32s      PFramesearchWidth, PFramesearchHeight;
  Ipp32s      MEalgorithm;
  Ipp32s      calcPSNR;
  Ipp32s      frameInterval;
  Ipp8u       *bsBuffer;
  Ipp32s      bsBuffSize;
  Ipp32s      RateControl;
  Ipp32s      BitRate;
  Ipp32s      FrameSkip;
  Ipp32s      bPP;
};


struct h261e_Frame {
  Ipp8u*      pY;
  Ipp8u*      pU;
  Ipp8u*      pV;
  Ipp32u      mid;
//  Ipp32s      type;
//  Ipp64s      time;
};

struct h261_RTPdata
{
  Ipp32u                      GOBstartPos[12];
  Ipp16u                      *GOBN_MBAP;
  Ipp8u                       *MBquant;
  IppMotionVector             *MBpredMV;
  Ipp32u                      *MBpos;
  Ipp32s                      num_coded_MB;
};

#define H261_STS_SKIPPED_FRAME   -1
#define H261_STS_NOERR           0
#define H261_STS_ERR_NOTINIT     1
#define H261_STS_ERR_PARAM       2
#define H261_STS_ERR_NOMEM       3
#define H261_STS_ERR_BUFOVER     4

class ippVideoEncoderH261
{
public :
  h261e_Frame mFrame[2];
  Ipp32s      mSourceWidth;           // width
  Ipp32s      mSourceHeight;          // height
  Ipp32s      mStepLuma;
  Ipp32s      mStepChroma;
  Ipp32s      mPlanes;
  Ipp32s      mLumaPlaneSize;
  Ipp32s      mChromaPlaneSize;
  Ipp32s      mFrameCount;
  Ipp64s      mPSNR_Y, mPSNR_U, mPSNR_V;
  ippBitStreamH cBS;
protected :
  bool        mIsInit;
  Ipp32s      mbsAlloc;
  Ipp32s      mNumMacroBlockPerFrame;
  h261e_Frame  *mFrameC, *mFrameF;
  Ipp32s      mQuantIFrame, mQuantPFrame;
  Ipp32s      mIFramedist, mLastIFrame;
  Ipp32s      mPFramesearchHor, mPFramesearchVer;
  Ipp32s      mMEalgorithm;
  Ipp32s      *mMEfastSAD;
  Ipp32s      mMEfastSADsize;
  Ipp32s      mMEthrSAD16x16, mMEthrSAD8x8;
  Ipp32s      mCalcPSNR;
  Ipp32s      mFrameInterval; // in CIF picture clock periods (1001/30000 s)
  Ipp32s      mBPPmax;

  Ipp32s      mRateControl;
  Ipp32s      mBitRate;
#if 0
  Ipp64s      mBitsEncoded;
  Ipp32s      mBitsPerFrameRC;
  Ipp64s      mBitsDesiredRC;
#endif
  Ipp32s      mBitsEncodedFrame, mBitsDesiredFrame;
  Ipp64s      mBitsEncodedTotal, mBitsDesiredTotal;
  Ipp32s      mRCfap, mRCqap, mRCbap, mRCq;
  Ipp64f      mRCqa, mRCfa;

  Ipp32s      mFrameSkipEnabled;
  Ipp32s      mSkipFrame;
  Ipp32s      mFrameSkipped;

  h261_VideoPicture            mVideoPicture;
  h261_MacroBlock              *MBinfo;
  static const VLCcode         mVLC_MBA[34];
  static const Ipp8u           mLen_MType[16];
  static const VLCcode         mVLC_MVD[32]; // only 17 are used
  static const VLCcode         mVLC_CBP[63];
  // data for RTP support
  h261_RTPdata                mRTPdata;
public :
          ippVideoEncoderH261() { mIsInit = false; };
          ippVideoEncoderH261(h261_Param *par);
          ~ippVideoEncoderH261();
  Ipp32s  Init(h261_Param *par);
  void    Close();
  Ipp32s  EncodeFrame();
  void    ErrorMessage(vm_char *msg);
  // for RTP support
  Ipp32u* GetFrameGOBstartPos() { return mRTPdata.GOBstartPos; };
  Ipp16u* GetFrameGOBN_MBAP() { return mRTPdata.GOBN_MBAP; }
  Ipp8u*  GetFrameQuant() { return mRTPdata.MBquant; };
  IppMotionVector*  GetFrameMVpred() { return mRTPdata.MBpredMV; };
  Ipp32u* GetFrameMBpos() { return mRTPdata.MBpos; };
  Ipp32s  GetFrameNumCodedMB() { return mRTPdata.num_coded_MB; };
  Ipp32s  GetCurrentFrameInfo(Ipp8u **pY, Ipp8u **pU, Ipp8u **pV, Ipp32s *stepLuma, Ipp32s *stepChroma);
protected :
  void    EncodeStartCode(Ipp8u sc);
  void    EncodeZeroBitsAlign();
  void    EncodeStuffingBitsAlign();
  void    EncodeVideoPacketHeader(Ipp32s mbn);
  void    EncodePicture_Header();
  void    EncodeGOB_Header();
  void    EncodeMBA(Ipp8u mba);
  void    EncodeMType(Ipp8u mtype);
  void    EncodeMVD(IppMotionVector mvd);
  void    EncodeCBP(Ipp8u cbp);
  void    EncodeIFrame();
  void    EncodePFrame();
  void    EncodeMacroBlockIntra_H261(Ipp16s *coeffs, Ipp32s *nzCount);
  void    EncodeMacroBlockInter_H261(Ipp16s *coeffs, Ipp32s *nzCount);
  void    TransMacroBlockIntra_H261(Ipp8u *py, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s quant);
  void    TransMacroBlockIntra_H261_qc(Ipp8u *py, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s *pQuant);
  Ipp32s  TransMacroBlockInter_H261(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp16s *coeffMB,
                                    Ipp32s *nzCount, Ipp32s quant, Ipp8u *mcPred, Ipp32s lumaErr, Ipp32s sU, Ipp32s sV);
  Ipp32s  TransMacroBlockInter_H261_qc(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp16s *coeffMB,
                                       Ipp32s *nzCount, Ipp32s *pQuant, Ipp8u *mcPred, Ipp32s lumaErr, Ipp32s sU, Ipp32s sV);
  void    ReconMacroBlockNotCoded(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred);
  void    ReconMacroBlockIntra_H261(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s quant, Ipp32s *nzCount);
  void    ReconMacroBlockInter_H261(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred, Ipp16s *coeffMB, Ipp32s quant, Ipp32s *nzCount);
  void    ME_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB, Ipp32s *bestSAD, IppMotionVector *mv);
#if 0
  void    PostFrameRC(Ipp32s bpfEncoded);
#endif
  void    PostFrameRC();
};

#define h261e_Clip(a, l, r) if (a < (l)) a = l; else if (a > (r)) a = r;
#define h261e_ClipL(a, l) if (a < (l)) a = l;
#define h261e_ClipR(a, r) if (a > (r)) a = r;

template <class T> inline void h261e_Swap(T &a, T &b)
{
  T  t;
  t = a; a = b; b = t;
}

#if defined(__INTEL_COMPILER) && !defined(_WIN32_WCE)
    #define __ALIGN16(type, name, size) \
        __declspec (align(16)) type name[size]
#else
    #if defined(_WIN64) || defined(WIN64) || defined(LINUX64)
        #define __ALIGN16(type, name, size) \
            Ipp8u _a16_##name[(size)*sizeof(type)+15]; type *name = (type*)(((Ipp64s)(_a16_##name) + 15) & ~15)
    #else
        #define __ALIGN16(type, name, size) \
            Ipp8u _a16_##name[(size)*sizeof(type)+15]; type *name = (type*)(((Ipp32s)(_a16_##name) + 15) & ~15)
    #endif
#endif
