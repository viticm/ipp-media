/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH263
//
*/

#include "h263_enc_bitstream.hpp"
#include "ippdefs.h"
#include "ipps.h"
#include "ippi.h"
#include "ippvc.h"
#include "vm_debug.h"

#pragma warning(disable : 4514)     // unreferenced inline function has been removed

/* H.263++ code values */
/* ITU-T Recommendation H.263, Table 5 */
enum {
  H263e_ASPECT_RATIO_FORBIDDEN  = 0,
  H263e_ASPECT_RATIO_1_1        = 1,
  H263e_ASPECT_RATIO_12_11      = 2,
  H263e_ASPECT_RATIO_10_11      = 3,
  H263e_ASPECT_RATIO_16_11      = 4,
  H263e_ASPECT_RATIO_40_33      = 5,
  H263e_ASPECT_RATIO_EXTPAR     = 15
};

enum {
  H263e_PIC_TYPE_I    = 0,
  H263e_PIC_TYPE_P    = 1,
  H263e_PIC_TYPE_iPB  = 2,
  H263e_PIC_TYPE_B    = 3,
  H263e_PIC_TYPE_EI   = 4,
  H263e_PIC_TYPE_EP   = 5,
  H263e_PIC_TYPE_PB   = 6
};

/* Block Info */
struct h263e_Block {
  h263e_Block *predA;
  h263e_Block *predC;
  Ipp16s      dct_acA[8];
  Ipp16s      dct_acC[8];
  Ipp8u       quant;
  Ipp8u       validPredIntra;

  // original (not reconstructed) DCT coeffs, used to decide on Intra prediction mode
  Ipp16s      dctOr_acA[8];
  Ipp16s      dctOr_acC[8];
};


/* MacroBlock Info */
struct h263e_MacroBlock {
  IppMotionVector mv[4];
  Ipp32s          lumaErr;
  h263e_Block     block[6];
  Ipp8u           type;
  Ipp8u           not_coded;
  Ipp8u           validPredInter;
};


/* Video Object Plane Info */
struct h263e_VideoPicture {
  Ipp32s      modulo_time_base;
  Ipp32s      pic_time_increment;
//    Ipp32s      pic_id;                             // verid != 1 (newpred)
//    Ipp32s      pic_id_for_prediction_indication;   // verid != 1 (newpred)
//    Ipp32s      pic_id_for_prediction;          // verid != 1 (newpred)
  Ipp32s      pic_rounding_type;
  Ipp32s      pic_reduced_resolution;             // verid != 1
  Ipp32s      pic_width;
  Ipp32s      pic_height;

  Ipp32s      pic_quant;

  Ipp32s      temporal_reference;
  Ipp32s      temporal_reference_increment;
  Ipp32s      split_screen_indicator;
  Ipp32s      document_camera_indicator;
  Ipp32s      full_picture_freeze_release;
  Ipp32s      source_format;
  Ipp32s      picture_coding_type;
  Ipp32s      gob_number;
  Ipp32s      num_gobs_in_pic;
  Ipp32s      num_macroblocks_in_gob;
  Ipp32s      num_MBrows_in_gob;
  Ipp32s      gob_header_empty;
  Ipp32s      gob_frame_id;
  Ipp32s      quant_scale;

  Ipp32s      UMV;
  Ipp32s      SAC;
  Ipp32s      advPred;
  Ipp32s      PBframes;
  Ipp32s      temporal_reference_B;
  Ipp32s      dbquant;

  /* plusptype fields  */

  Ipp32s      plusptype;
  Ipp32s      ufep;
  Ipp32s      advIntra;
  Ipp32s      deblockFilt;
  Ipp32s      sliceStruct;
  Ipp32s      RPS;
  Ipp32s      ISD;
  Ipp32s      altInterVLC;
  Ipp32s      modQuant;
  Ipp32s      resample;
  Ipp32s      redResUp;

  Ipp32s      PCF;
  Ipp32s      CPM;
  Ipp32s      PSBI;
  Ipp32s      PAR_code;
  Ipp32s      PAR_width;
  Ipp32s      PAR_height;
  Ipp32s      clock_conversion_code;
  Ipp32s      clock_divisor;
  Ipp32s      unlimited_UMV;
  Ipp32s      sliceSubmodes; /* bit 1 - Rectangular slices; bit 2 - Arbitrary slice ordering */
  Ipp32s      enh_layer_num;
  Ipp32s      ref_layer_num;
  Ipp32s      RPSflags;
  Ipp32s      pred_temp_ref;
  Ipp32s      wda;
  IppMotionVector warpParams[4];
  Ipp32s      fillMode;
  Ipp32s      fillColor[3];
  Ipp32s      scalability;
};

/* Video Object Info */
struct h263e_VideoSequence {
  Ipp32s      aspect_ratio_info;
  Ipp32s      chroma_format;
  Ipp32s      pic_time_increment_resolution;
  Ipp32s      fixed_pic_time_increment;
  Ipp32s      data_partitioned;
/*
  Ipp32s      newpred_enable;                 // verid != 1
  Ipp32s      requested_upstream_message_type;// verid != 1
  Ipp32s      newpred_segment_type;           // verid != 1
  Ipp32s      reduced_resolution_pic_enable;  // verid != 1
  Ipp32s      scalability;
*/
};

struct h263e_Param
{
  Ipp32s      Width;
  Ipp32s      Height;
  //Ipp32s      NumOfFrames;
  Ipp32s      TimeResolution;
  Ipp32s      TimeIncrement;
/*
  Ipp32s      load_intra_quant_mat;
  Ipp32s      load_intra_quant_mat_len;
  Ipp8u       intra_quant_mat[64];
  Ipp32s      load_nonintra_quant_mat;
  Ipp32s      load_nonintra_quant_mat_len;
  Ipp8u       nonintra_quant_mat[64];
*/
  Ipp32s      quantIPic, quantPPic, quantBPic;
  Ipp32s      IPicdist, PPicdist;
  Ipp32s      PPicsearchWidth, PPicsearchHeight;
//    Ipp32s      BPicsearchWidthForw, BPicsearchHeightForw;
//    Ipp32s      BPicsearchWidthBack, BPicsearchHeightBack;
  Ipp32s      MEalgorithm;
  Ipp32s      MEaccuracy;

  Ipp32s      advPred;
  Ipp32s      deblockFilt;

  Ipp32s      calcPSNR;
//    Ipp32s      VideoPacketLength;
  Ipp32s      advIntra;
  Ipp32s      UMV;
  Ipp32s      modQuant;
//    Ipp32s      data_partitioned;
  Ipp8u       *bsBuffer;
  Ipp32s      bsBuffSize;
  Ipp32s      RateControl;
  Ipp32s      BitRate;
  Ipp32s      SceneChangeThreshold;
  Ipp32s      FrameSkip;
  Ipp32s      GOBheaders;
  Ipp32s      PAR_width;
  Ipp32s      PAR_height;
};

struct h263e_Frame {
  Ipp8u*      ap;
  Ipp8u*      pY;
  Ipp8u*      pU;
  Ipp8u*      pV;
  Ipp32u      mid;
//  Ipp32s      type;
//  Ipp64s      time;
};

struct h263e_VLC {
    Ipp32s  code;
    Ipp32s  len;
};

struct h263e_RTPdata
{
  Ipp32u                      *GOBstartPos;
  Ipp8u                       *MBquant;
  IppMotionVector             *MBpredMV;
  IppMotionVector             *MBpredMV1;
  Ipp32u                      *MBpos;
  Ipp8u                       codingModes;
};


#define YUV_CHROMA_420    0
#define YUV_CHROMA_422    1
#define YUV_CHROMA_444    2

#define H263_STS_SKIPPED_FRAME      -3
#define H263_STS_NODATA      -2
#define H263_STS_BUFFERED    -1
#define H263_STS_NOERR        0
#define H263_STS_ERR_NOTINIT  1
#define H263_STS_ERR_PARAM    2
#define H263_STS_ERR_NOMEM    3
#define H263_STS_ERR_BUFOVER  4


class ippVideoEncoderH263
{
public :
  Ipp32s      mSourceFormat;          // 4:2:0 only supported
  Ipp32s      mSourceWidth;           // width
  Ipp32s      mSourceHeight;          // height
  Ipp32s      mStepLuma;
  Ipp32s      mStepChroma;
  Ipp32s      mLumaPlaneSize;
  Ipp32s      mChromaPlaneSize;
  Ipp32s      mPlanes;
  h263e_Frame *mFrame;
  Ipp32s      mExpandSizeA;
  Ipp32s      mNumOfFrames;
  Ipp32s      mFrameCount;
//  Ipp64s      mQuantAvg;
  Ipp64s      mPSNR_Y, mPSNR_U, mPSNR_V;
  Ipp32s      mIPicdist, mPPicdist, mBPicdist, mLastIPic;
  H263BitStream cBS;
protected :
  bool        mIsInit;
  Ipp32s      mbsAlloc;
  Ipp32s      mExpandSize;
  Ipp32s      mNumMacroBlockPerRow;
  Ipp32s      mNumMacroBlockPerCol;
  Ipp32s      mNumMacroBlockPerPic;
  h263e_Frame  *mFrameC, *mFrameF, *mFrameB;
  Ipp32s      mQuantIPic, mQuantPPic, mQuantBPic;
  Ipp32s      mPPicsearchHor, mPPicsearchVer;
//    Ipp32s      mBPicsearchHorForw, mBPicsearchVerForw;
//    Ipp32s      mBPicsearchHorBack, mBPicsearchVerBack;
//    Ipp32s      mPPicfcodeForw;
//    Ipp32s      mBPicfcodeForw;
//    Ipp32s      mBPicfcodeBack;
  Ipp32s      mMEaccuracy;
  Ipp32s      mMEalgorithm;
  Ipp32s      mMEfastHP;
  Ipp32s      *mMEfastSAD;
  Ipp32s      mMEfastSADsize;
  Ipp32s      mMEthrSAD16x16, mMEthrSAD8x8, mMEthrSAD16x8;
  Ipp32s      mME4mv;
  Ipp32s      mCalcPSNR;
//    Ipp32s      mVideoPacketLength;
  Ipp64s      mPictime, mSyncTime;
  Ipp32s      mNumBPic, mIndxBPic, mTRB, mTRD, mSyncTimeB;
  Ipp32s      mRateControl;
  Ipp32s      mBitRate;
  Ipp32s      mBPPmax;

  Ipp32s      mBitsEncodedFrame, mBitsDesiredFrame;
  Ipp64s      mBitsEncodedTotal, mBitsDesiredTotal;
  Ipp32s      mRCfap, mRCqap, mRCbap, mRCq;
  Ipp64f      mRCqa, mRCfa;

  Ipp32s      mFrameSkipEnabled;
  Ipp32s      mSkipFrame;
  Ipp32s      mFrameSkipped;

  Ipp32s      mSceneChangeThreshold, mNumIntraMB;
  Ipp32s      mGOBheaders;
  Ipp32s      mGSTUF;
  h263e_VideoSequence          mVideoSequence;
  h263e_VideoPicture           mVideoPicture;
  static const h263e_VLC       mVLC_CBPY_TB8[16];
  static const h263e_VLC       mVLC_MCBPC_TB7[20];
  static const h263e_VLC       mVLC_MVD_TB12[65];
  // info for each macroblock
  h263e_MacroBlock            *mMBinfo;
  // additional buffers for data_partitioned
  Ipp8u                       *mBuffer_1;
  Ipp8u                       *mBuffer_2;
  // data for RTP support
  h263e_RTPdata                mRTPdata;
public :
          ippVideoEncoderH263() { mIsInit = false; };
          ippVideoEncoderH263(h263e_Param *par);
          ~ippVideoEncoderH263();
  Ipp32s  Init(h263e_Param *par);
  void    Close();
//  Ipp32s  GetAverageQuant() { return (Ipp32s)mQuantAvg; };
  Ipp32s  GetNumMacroBlockPerRow() { return mNumMacroBlockPerRow; };
  Ipp32s  GetNumMacroBlockPerCol() { return mNumMacroBlockPerCol; };
  Ipp32s  GetFrameType() { return mVideoPicture.picture_coding_type; };
  Ipp64s  GetBitsEncoded() { return mBitsEncodedTotal; };
  void    GetMSE(Ipp64s *y, Ipp64s *u, Ipp64s *v) { *y = mPSNR_Y; *u = mPSNR_U; *v = mPSNR_V; };
  Ipp32s  GetCurrentFrameInfo(Ipp8u **pY, Ipp8u **pU, Ipp8u **pV, Ipp32s *stepLuma, Ipp32s *stepChroma);
  void    InitBuffer(Ipp8u *ptr, Ipp32s size) { cBS.Init(ptr, size); };
  Ipp8u*  GetBufferPtr() { return cBS.GetPtr(); };
  Ipp32s  GetBufferFullness() { return cBS.GetFullness(); };
  void    GetBufferPos(Ipp8u **ptr, Ipp32s *bitOff) { cBS.GetPos(ptr, bitOff); };
  void    ResetBuffer() { cBS.Reset(); };
  Ipp32s  EncodeFrame(Ipp32s noMoreData);
  // for RTP support
  Ipp32u* GetFrameGOBstartPos() { return mRTPdata.GOBstartPos; };
  Ipp8u*  GetFrameQuant() { return mRTPdata.MBquant; };
  IppMotionVector*  GetFrameMVpred() { return mRTPdata.MBpredMV; };
  IppMotionVector*  GetFrameMVpred1() { return mRTPdata.MBpredMV1; };
  Ipp32u* GetFrameMBpos() { return mRTPdata.MBpos; };
  Ipp8u   GetFrameCodingModes() { return mRTPdata.codingModes; };
  Ipp16u  GetFrameDBQ_TRB_TR() {
    return (Ipp16u)(mVideoPicture.PBframes ? ((mVideoPicture.dbquant << 11) | (mVideoPicture.temporal_reference_B << 8) | mVideoPicture.temporal_reference) : 0);
  };
protected :
  Ipp32s  EncodePic(Ipp32s picType, Ipp32s nt);
  void    ErrorMessage(const vm_char *msg);
  void    EncodeZeroBitsAlign();
  void    EncodeStuffingBitsAlign();
  void    EncodePicture_Header();
  void    EncodeGOB_Header(Ipp32s gob_num);
  void    EncodeIPic();
  void    EncodePPic();
//    void    EncodeBPic();
  void    EncodeMCBPC_I(Ipp32s mbtype, Ipp32s mcbpc);
  void    EncodeAdvIntraPredMode(Ipp32s scan);
  void    EncodeCBPY_I(Ipp32s pat);
  void    EncodeMCBPC_P(Ipp32s mbtype, Ipp32s mcbpc);
  void    EncodeCBPY_P(Ipp32s mbtype, Ipp32s pat);
  void    EncodeMacroBlockIntra_H263(Ipp16s *coeffs, Ipp32s pattern, Ipp32s *nzCount, Ipp32s scan);
  void    EncodeMacroBlockInter_H263(Ipp16s *coeffs, Ipp32s pattern, Ipp32s *nzCount);
  void    EncodeMV(IppMotionVector *mv, Ipp32s mbType);
  void    ExpandFrame(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV);
  void    PredictMV(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j, Ipp32s adv, IppMotionVector *mvPred);
  void    Predict3MV(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j, IppMotionVector *mvPred, IppMotionVector *mvCurr);
  void    me4MV_Neighbours(h263e_MacroBlock *MBcurr, Ipp32s frGOB, Ipp32s i, Ipp32s j, IppMotionVector *mvPred);

  void    ChooseAdvIntraPred(h263e_MacroBlock *MBcurr, Ipp16s *coeffs, Ipp32s *predDir);

  void    PredictAdvIntra(h263e_MacroBlock *MBcurr, Ipp16s *coeffs, Ipp32s scan);
  Ipp32s  PredictReconstructAdvIntra(Ipp8u *pF[6], h263e_MacroBlock *MBcurr, Ipp16s *coeffs, Ipp32s *nzCount, Ipp32s quant, Ipp32s scan);

  void    DCT8x8MacroBlock_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB);
  Ipp32s  QuantMacroBlockIntra_H263(Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s quant);
  Ipp32s  TransMacroBlockInter_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s *nzCount, Ipp32s quant, Ipp8u *mcPred, Ipp32s lumaErr);
  void    ReconMacroBlockNotCoded(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred);

  void    OBMC_Macroblock(h263e_MacroBlock *pMBinfo, IppMotionVector *mvCur, Ipp32s colNum, Ipp32s rowNum, IppiRect limitRectL, Ipp8u *pYr, Ipp32s stepYr, Ipp8u *pYc, Ipp32s stepYc);

  void    ReconMacroBlockIntra_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s quant, Ipp32s pattern);
  void    ReconBlockIntra_AdvI_H263(Ipp8u *p, Ipp32s step, Ipp16s *coef, h263e_Block *bCurr, Ipp32s quant, Ipp32s pattern, Ipp32s scan);

  void    ReconMacroBlockInter_H263(Ipp8u *pYc, Ipp8u *pUc, Ipp8u *pVc, Ipp8u *mcPred, Ipp16s *coeffMB, Ipp32s quant, Ipp32s pattern);
  void    ReconMacroBlockIntra_H263(Ipp8u *pY, Ipp8u *pU, Ipp8u *pV, Ipp16s *coeffMB, Ipp32s quant, h263e_MacroBlock *MBcurr, Ipp32s pattern, Ipp32s dct_type);
  void    ME_SAD_16x16(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB, Ipp32s *bestSAD, IppMotionVector *mv);
  void    ME_SAD_8x8(Ipp8u *pCur, Ipp8u *pRef, Ipp32s step, Ipp32s xL, Ipp32s xR, Ipp32s yT, Ipp32s yB, Ipp32s *bestSAD, IppMotionVector *mv, IppMotionVector *mv4, Ipp32s x, Ipp32s y, IppMotionVector *mvNeighb);
  void    PostFrameRC();
};

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


#define h263e_Div2(a) ((a) >= 0 ? ((a) >> 1) : (((a)+1) >> 1))
#define h263e_Div2Round(a) (((a) >> 1) | ((a) & 1))
#define h263e_DivRoundInf(a, b) ((a) < 0 ? (((a) - ((b) >> 1)) / (b)) : (((a) + ((b) >> 1)) / (b)))
#define h263e_Clip(a, l, r) if (a < (l)) a = l; else if (a > (r)) a = r;
#define h263e_ClipL(a, l) if (a < (l)) a = l;
#define h263e_ClipR(a, r) if (a > (r)) a = r;
#define h263e_Abs(a) ((a) >= 0 ? (a) : -(a))

#define h263e_Zero8_16s(pDst) \
  ((Ipp32u*)pDst)[0] = ((Ipp32u*)pDst)[1] = ((Ipp32u*)pDst)[2] = ((Ipp32u*)pDst)[3] = 0;

#ifdef USE_INTRINSIC

#define h263e_Zero4MV(mv) \
  memset(mv, 0, 4 * sizeof(IppMotionVector));

#define h263e_Zero64_16s(pDst) \
{ \
  __m128i _p_val = _mm_setzero_si128(); \
  ((__m128i*)(pDst))[0] = _p_val; \
  ((__m128i*)(pDst))[1] = _p_val; \
  ((__m128i*)(pDst))[2] = _p_val; \
  ((__m128i*)(pDst))[3] = _p_val; \
  ((__m128i*)(pDst))[4] = _p_val; \
  ((__m128i*)(pDst))[5] = _p_val; \
  ((__m128i*)(pDst))[6] = _p_val; \
  ((__m128i*)(pDst))[7] = _p_val; \
}

#define h263e_Set64_16s(val, pDst) \
{ \
  __m128i _p_val = _mm_set1_epi16((Ipp16s)(val)); \
  ((__m128i*)(pDst))[0] = _p_val; \
  ((__m128i*)(pDst))[1] = _p_val; \
  ((__m128i*)(pDst))[2] = _p_val; \
  ((__m128i*)(pDst))[3] = _p_val; \
  ((__m128i*)(pDst))[4] = _p_val; \
  ((__m128i*)(pDst))[5] = _p_val; \
  ((__m128i*)(pDst))[6] = _p_val; \
  ((__m128i*)(pDst))[7] = _p_val; \
}

#else

#define h263e_Zero4MV(mv) \
    *((Ipp32u*)&mv[0]) = *((Ipp32u*)&mv[1]) = *((Ipp32u*)&mv[2]) = *((Ipp32u*)&mv[3]) = 0

#define h263e_Zero64_16s(pDst) \
{ \
  Ipp32s  i; \
  for (i = 0; i < 16; i += 4) {\
    ((Ipp64u*)(pDst))[i] = 0; \
    ((Ipp64u*)(pDst))[i+1] = 0; \
    ((Ipp64u*)(pDst))[i+2] = 0; \
    ((Ipp64u*)(pDst))[i+3] = 0; \
  } \
}

#define h263e_Set64_16s(val, pDst) \
{ \
  Ipp32s  i; \
  Ipp32u  v; \
  v = ((val) << 16) + (Ipp16u)(val); \
  for (i = 0; i < 32; i += 8) { \
    ((Ipp32u*)(pDst))[i] = v; \
    ((Ipp32u*)(pDst))[i+1] = v; \
    ((Ipp32u*)(pDst))[i+2] = v; \
    ((Ipp32u*)(pDst))[i+3] = v; \
    ((Ipp32u*)(pDst))[i+4] = v; \
    ((Ipp32u*)(pDst))[i+5] = v; \
    ((Ipp32u*)(pDst))[i+6] = v; \
    ((Ipp32u*)(pDst))[i+7] = v; \
  } \
}

#endif

template <class T> inline void h263e_Swap(T &a, T &b)
{
  T  t;
  t = a; a = b; b = t;
}

#define limitMV(dx, xmin, xmax, mvd) \
{                                    \
  if ((dx) < (xmin))                 \
    mvd = (Ipp16s)(xmin);          \
  else if ((dx) >= (xmax))           \
    mvd = (Ipp16s)(xmax);          \
  else                               \
    mvd = (Ipp16s)(dx);            \
}

inline void h263e_LimitMV(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, Ipp32s x, Ipp32s y, Ipp32s size)
{
  limitMV(pSrcMV->dx, (limitRect->x - x) << 1, (limitRect->x - x + limitRect->width  - size) << 1, pDstMV->dx);
  limitMV(pSrcMV->dy, (limitRect->y - y) << 1, (limitRect->y - y + limitRect->height - size) << 1, pDstMV->dy);
}

inline void h263e_Limit4MV(const IppMotionVector *pSrcMV, IppMotionVector *pDstMV, const IppiRect *limitRect, Ipp32s x, Ipp32s y, Ipp32s size)
{
  h263e_LimitMV(&pSrcMV[0], &pDstMV[0], limitRect, x       , y,        size);
  h263e_LimitMV(&pSrcMV[1], &pDstMV[1], limitRect, x + size, y,        size);
  h263e_LimitMV(&pSrcMV[2], &pDstMV[2], limitRect, x       , y + size, size);
  h263e_LimitMV(&pSrcMV[3], &pDstMV[3], limitRect, x + size, y + size, size);
}

extern const Ipp8u h263e_cCbCrMvRound16_[];
extern const Ipp8u h263e_DCScalerLuma_[];
extern const Ipp8u h263e_DCScalerChroma_[];
