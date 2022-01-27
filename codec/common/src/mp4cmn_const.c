/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"

#ifdef UMC_MP4_CONST
#undef UMC_MP4_CONST
#endif

#if defined (UMC_ENABLE_AAC_AUDIO_DECODER)
#define UMC_MP4_CONST
#elif defined UMC_ENABLE_AAC_INT_AUDIO_DECODER
#define UMC_MP4_CONST
//#elif defined UMC_ENABLE_AAC_AUDIO_ENCODER
//#define UMC_MP4_CONST
//#elif defined UMC_ENABLE_AAC_INT_AUDIO_ENCODER
//#define UMC_MP4_CONST
#elif defined UMC_ENABLE_MP4_SPLITTER
#define UMC_MP4_CONST
#endif

#ifdef UMC_MP4_CONST

#include "mp4cmn_const.h"

static Ipp32s g_sampling_frequency_table[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0
};

Ipp32s
get_sampling_frequency_by_index(Ipp32s index)
{
    return g_sampling_frequency_table[index];
}

#endif //UMC_ENABLE_XXX
