/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)

#ifndef __UMC_MPEG4_VIDEO_ENCODER_H__
#define __UMC_MPEG4_VIDEO_ENCODER_H__

#include "mp4_enc.hpp"
#include "umc_video_encoder.h"
#include "umc_dynamic_cast.h"
#include "vm_strings.h"

namespace UMC
{

#define STR_LEN 511

class MPEG4EncoderParams : public VideoEncoderParams
{
    DYNAMIC_CAST_DECL(MPEG4EncoderParams, VideoEncoderParams)
public:
    MPEG4_ENC::mp4_Param  m_Param;

    MPEG4EncoderParams();

    virtual Status ReadParamFile(const vm_char *FileName);
};

class MPEG4VideoEncoder: public VideoEncoder
{
public:
    MPEG4_ENC::ippVideoEncoderMPEG4 *mp4enc;
    bool        m_IsInit;
    Ipp32s      m_FrameCount;
    MPEG4VideoEncoder();
    ~MPEG4VideoEncoder();
    virtual Status Init(BaseCodecParams *init);
    virtual Status GetFrame(MediaData *in, MediaData *out);
    virtual Status GetInfo(BaseCodecParams *info);
    virtual Status Close();
    virtual Status Reset();
    virtual Status SetParams(BaseCodecParams* params);
    Ipp32s GetFrameMacroBlockPerRow() { return mp4enc->GetFrameMacroBlockPerRow(); }
    Ipp32s GetFrameMacroBlockPerCol() { return mp4enc->GetFrameMacroBlockPerCol(); }
    Ipp8u* GetFrameQuant() { return mp4enc->GetFrameQuant(); };
    IppMotionVector* GetFrameMVpred() { return mp4enc->GetFrameMVpred(); };
    Ipp32u* GetFrameMBpos() { return mp4enc->GetFrameMBpos(); };
protected:
    MPEG4EncoderParams  m_Param;
    Ipp64f             *bTime, gTime, iTime;
    Ipp32s              bTimePos;
    // allocate memory for internal buffers
    Status AllocateBuffers();
    // free memory for internal buffers
    Status FreeBuffers();
    // lock memory for internal buffers
    void LockBuffers();
    // unlock memory for internal buffers
    Status UnlockBuffers();
};

} //namespace UMC

#endif /* __UMC_MPEG4_VIDEO_ENCODER_H__ */

#endif //defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)
