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

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)

#ifndef _ENCODER_H261_H_
#define _ENCODER_H261_H_

#include "umc_video_encoder.h"
#include "umc_dynamic_cast.h"
#include "vm_strings.h"
#include "h261_enc.hpp"

namespace UMC
{

#define STR_LEN 511

class H261EncoderParams : public VideoEncoderParams
{
    DYNAMIC_CAST_DECL(H261EncoderParams, VideoEncoderParams)
public:
    h261_Param  m_Param;
    H261EncoderParams();
    virtual Status ReadParamFile(const vm_char *ParFileName);
};

class H261VideoEncoder: public VideoEncoder
{
protected:
  ippVideoEncoderH261 h261enc;
public:
  H261VideoEncoder();
  ~H261VideoEncoder();
  virtual Status Init(BaseCodecParams *init);
  virtual Status GetFrame(MediaData *in, MediaData *out);
  virtual Status GetInfo(BaseCodecParams *info);
  virtual Status Close();
  virtual Status Reset();
  virtual Status SetParams(BaseCodecParams* params);
  // RTP (RFC 2032) support
  Ipp32u* GetFrameGOBstartPos() { return h261enc.GetFrameGOBstartPos(); };
  Ipp16u* GetFrameGOBN_MBAP() { return h261enc.GetFrameGOBN_MBAP(); }
  Ipp8u*  GetFrameQuant() { return h261enc.GetFrameQuant(); };
  IppMotionVector*  GetFrameMVpred() { return h261enc.GetFrameMVpred(); };
  Ipp32u* GetFrameMBpos() { return h261enc.GetFrameMBpos(); };
  Ipp32s  GetFrameNumCodedMB() { return h261enc.GetFrameNumCodedMB(); };
protected:
  bool        m_IsInit;
  H261EncoderParams  m_Param;
  Status AllocateBuffers();
  // free memory for internal buffers
  Status FreeBuffers();
  // lock memory for internal buffers
  void LockBuffers();
  // unlock memory for internal buffers
  Status UnlockBuffers();
};

VideoEncoder* createH261VideoEncoder();

} //namespace UMC

#endif /*_ENCODER_H261_H_*/

#endif //defined (UMC_ENABLE_H261_VIDEO_ENCODER)
