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
#if defined (UMC_ENABLE_MP3_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_AUDIO_ENCODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_ENCODER)

#ifndef __MP3_OWN_H__
#define __MP3_OWN_H__

#include "ippac.h"
#include "ippdc.h"
#include "ipps.h"
#include "mp3_alloc_tab.h"
#include "vm_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MP3_UPDATE_PTR(type, ptr, inc)    \
  if (ptr) {                              \
    ptr = (type *)((Ipp8u *)(ptr) + inc); \
  }

typedef struct {
    Ipp32s ext_bit_stream_present;
    Ipp32s n_ad_bytes;
    Ipp32s center;
    Ipp32s surround;
    Ipp32s lfe;
    Ipp32s audio_mix;
    Ipp32s dematrix_procedure;
    Ipp32s no_of_multi_lingual_ch;
    Ipp32s multi_lingual_fs;
    Ipp32s multi_lingual_layer;
    Ipp32s copyright_identification_bit;
    Ipp32s copyright_identification_start;
} mp3_mc_header;

extern Ipp32s mp3_bitrate[2][3][16];
extern Ipp32s mp3_frequency[3][4];
extern const Ipp32s mp3_mc_pred_coef_table[6][16];
extern const Ipp8u mp3_mc_sb_group[32];

extern const Ipp32f mp3_lfe_filter[480];


Ipp32s mp3_SetAllocTable(Ipp32s header_id, Ipp32s mpg25, Ipp32s header_layer,
                         Ipp32s header_bitRate, Ipp32s header_samplingFreq,
                         Ipp32s stereo,
                         Ipp32s **nbal_alloc_table,
                         Ipp8u **alloc_table,
                         Ipp32s *sblimit);

#ifdef __cplusplus
}
#endif

#endif

#endif //UMC_ENABLE_XXX
