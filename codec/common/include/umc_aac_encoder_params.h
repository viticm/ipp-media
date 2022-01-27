/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_AAC_ENCODER_PARAMS_H__
#define __UMC_AAC_ENCODER_PARAMS_H__

#include "umc_audio_codec.h"
#include "ippdefs.h"

enum UMC_AACStereoMode {
  UMC_AAC_MONO,
  UMC_AAC_LR_STEREO,
  UMC_AAC_MS_STEREO,
  UMC_AAC_JOINT_STEREO
};

namespace UMC
{
#include "aaccmn_const.h"

class AACEncoderParams:public AudioCodecParams {
  DYNAMIC_CAST_DECL(AACEncoderParams, AudioCodecParams)
public:
  AACEncoderParams(void) {
    audioObjectType = AOT_AAC_LC;
    auxAudioObjectType = AOT_UNDEF;

    stereo_mode = UMC_AAC_LR_STEREO;
    outputFormat = UMC_AAC_ADTS;
    ns_mode = 0;
  };
  enum AudioObjectType audioObjectType;
  enum AudioObjectType auxAudioObjectType;

  enum UMC_AACStereoMode  stereo_mode;
  enum UMC_AACOuputFormat outputFormat;
  Ipp32s ns_mode;
};

}// namespace UMC

#endif /* __UMC_AAC_ENCODER_PARAMS_H__ */

