/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_AAC_DECODER_H__
#define __UMC_AAC_DECODER_H__

#include "ippdefs.h"
#include "umc_audio_codec.h"
#include "umc_memory_allocator.h"
#include "umc_dynamic_cast.h"

#include "aaccmn_const.h"
#include "mp4cmn_config.h"
#include "audio_codec_params.h"
#include "aac_status.h"

struct _AACDec;

namespace UMC {

  // profile and level definitions for aac
  // values for profile and level fields in BaseCodecParams
#define AAC_PROFILE_MAIN  1
#define AAC_PROFILE_LC    2
#define AAC_PROFILE_SSR   3
#define AAC_PROFILE_LTP   4
#define AAC_PROFILE_SBR   0x100

#define AAC_LEVEL_MAIN    0

  class   AACDecoder:public AudioCodec {
    DYNAMIC_CAST_DECL(AACDecoder, AudioCodec);

  public:
    enum DecodeMode {
      DM_UNDEF_STREAM = 0,
      DM_RAW_STREAM,
      DM_ADTS_STREAM
    };

    AACDecoder();
    ~AACDecoder(void);

    virtual Status  Init(BaseCodecParams * init);
    virtual Status  GetFrame(MediaData * in, MediaData * out);
    virtual Status  Close();
    virtual Status  Reset();

    virtual Status  GetInfo(BaseCodecParams * info);
    virtual Status  SetParams(BaseCodecParams * params);
    virtual Status  GetDuration(Ipp32f *p_duration);

  protected:
    Status StatusAAC_2_UMC(AACStatus st);
    Status SetObjectType(AudioObjectType mType,
                         sALS_specific_config *inState);
    Status MemLock();
    Status MemUnlock();

    struct _AACDec *state;
    cAudioCodecParams params;

    Ipp64f m_pts_prev;
    Ipp32s initSubtype;
    Ipp32u adts_sw;
    MemID  stateMemId;
    MemID  objMemId;
    Ipp8u *pObjMem;

    struct {
      AudioStreamType    m_init_stream_type;
      size_t             m_init_config_data_size;
      DecodeMode         m_decode_mode;
      Ipp32s             m_sampling_frequency_index;

      Ipp32u             m_channel_config;
      Ipp32s             m_frame_number;
      AudioObjectType    m_audio_object_type;
      AudioStreamSubType m_stream_subtype;
    } m_info;
  };
};      // namespace UMC

#endif // __UMC_AAC_DECODER_H__
