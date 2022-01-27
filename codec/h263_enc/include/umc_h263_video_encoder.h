/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2007 Intel Corporation. All Rights Reserved.
//
*/
#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)

#ifndef _ENCODER_H263_H_
#define _ENCODER_H263_H_

#include "umc_video_encoder.h"
#include "umc_dynamic_cast.h"
#include "vm_strings.h"
#include "h263_enc.hpp"

namespace UMC
{

#define STR_LEN 511

class H263EncoderParams : public VideoEncoderParams
{
    DYNAMIC_CAST_DECL(H263EncoderParams, VideoEncoderParams)
public:
    h263e_Param  m_Param;
    H263EncoderParams();
    virtual Status ReadParamFile(const vm_char *ParFileName);
};

class H263VideoEncoder: public VideoEncoder
{
protected:
  ippVideoEncoderH263 h263enc;
public:
  H263VideoEncoder();
  ~H263VideoEncoder();
  virtual Status Init(BaseCodecParams *init);
  virtual Status GetFrame(MediaData *in, MediaData *out);
  virtual Status GetInfo(BaseCodecParams *info);
  virtual Status Close();
  virtual Status Reset();
  virtual Status SetParams(BaseCodecParams* params);
  Ipp32s GetNumMacroBlockPerRow() { return h263enc.GetNumMacroBlockPerRow(); }
  Ipp32s GetNumMacroBlockPerCol() { return h263enc.GetNumMacroBlockPerCol(); }
  // RTP (RFC 2190) support
  Ipp32u* GetFrameGOBstartPos() { return h263enc.GetFrameGOBstartPos(); };
  Ipp8u* GetFrameQuant() { return h263enc.GetFrameQuant(); };
  IppMotionVector* GetFrameMVpred() { return h263enc.GetFrameMVpred(); };
  IppMotionVector* GetFrameMVpred1() { return h263enc.GetFrameMVpred1(); };
  Ipp32u* GetFrameMBpos() { return h263enc.GetFrameMBpos(); };
  Ipp8u   GetFrameCodingModes() { return h263enc.GetFrameCodingModes(); };
  Ipp16u  GetFrameDBQ_TRB_TR() {return h263enc.GetFrameDBQ_TRB_TR(); };
protected:
  bool           m_IsInit;
  Ipp32s         m_FrameCount;
  H263EncoderParams  m_Param;
  // allocate memory for internal buffers
  Status AllocateBuffers();
  // free memory for internal buffers
  Status FreeBuffers();
  // lock memory for internal buffers
  void LockBuffers();
  // unlock memory for internal buffers
  Status UnlockBuffers();
};

VideoEncoder* createH263VideoEncoder();

} //namespace UMC

#endif /*_ENCODER_H263_H_*/
#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
