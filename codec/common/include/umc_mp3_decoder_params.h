/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2007 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#if defined (UMC_ENABLE_MP3_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_DECODER)

#ifndef __UMC_MP3_DECODER_PARAMS_H__
#define __UMC_MP3_DECODER_PARAMS_H__

#include "umc_audio_codec.h"

namespace UMC
{

class MP3DecoderParams:public AudioCodecParams {
  DYNAMIC_CAST_DECL(MP3DecoderParams, AudioCodecParams)
public:
  MP3DecoderParams(void) {
    mc_lfe_filter_off = 0;
    synchro_mode = 0;
  };
  Ipp32s mc_lfe_filter_off;
  Ipp32s synchro_mode;
};

}// namespace UMC

#endif /* __UMC_MP3_DECODER_PARAMS_H__ */

#endif //UMC_ENABLE_XXX
