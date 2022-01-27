/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_AAC_DECODER_PARAMS_H__
#define __UMC_AAC_DECODER_PARAMS_H__

#include "umc_audio_codec.h"
#include "aaccmn_const.h"
#include "sbr_dec_struct.h"
#include "ps_dec_settings.h"

namespace UMC
{
  class   AACDecoderParams:public AudioCodecParams {
  public:
    AACDecoderParams(void) {
      ModeDecodeHEAACprofile = HEAAC_HQ_MODE;
      ModeDwnsmplHEAACprofile= HEAAC_DWNSMPL_ON;
      flag_SBR_support_lev   = SBR_ENABLE;
      flag_PS_support_lev    = PS_DISABLE;
      layer = -1;
    };
    Ipp32s ModeDecodeHEAACprofile;
    Ipp32s ModeDwnsmplHEAACprofile;
    eSBR_SUPPORT flag_SBR_support_lev;
    Ipp32s       flag_PS_support_lev;
    Ipp32s layer;
  };

}// namespace UMC

#endif /* __UMC_AAC_DECODER_PARAMS_H__ */
