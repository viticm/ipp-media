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
#if defined (UMC_ENABLE_MP3_AUDIO_ENCODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_ENCODER)

#ifndef __UMC_MP3_ENCODER_PARAMS_H__
#define __UMC_MP3_ENCODER_PARAMS_H__

#include "umc_audio_codec.h"

enum UMC_MP3StereoMode {
  UMC_MPA_MONO,
  UMC_MPA_LR_STEREO,
  UMC_MPA_MS_STEREO,
  UMC_MPA_JOINT_STEREO
};

#define UMC_MPAENC_CBR 0
#define UMC_MPAENC_ABR 1

namespace UMC
{

class MP3EncoderParams:public AudioCodecParams {
  DYNAMIC_CAST_DECL(MP3EncoderParams, AudioCodecParams)
public:
  MP3EncoderParams(void) {
    stereo_mode = UMC_MPA_LR_STEREO;
    layer = 3;
    mode = UMC_MPAENC_CBR;
    ns_mode = 0;
    force_mpeg1 = 0;
    mc_matrix_procedure = 0;
    mc_lfe_filter_off = 0;
  };
  enum UMC_MP3StereoMode stereo_mode;
  Ipp32s layer;
  Ipp32s mode;
  Ipp32s ns_mode;
  Ipp32s force_mpeg1;
  Ipp32s mc_matrix_procedure;
  Ipp32s mc_lfe_filter_off;
};

}// namespace UMC

#endif /* __UMC_MP3_ENCODER_PARAMS_H__ */

#endif //UMC_ENABLE_XXX
