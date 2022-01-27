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
#if defined (UMC_ENABLE_MP3_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_AUDIO_ENCODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_ENCODER)

#ifndef __MP3_ALLOC_TAB_H__
#define __MP3_ALLOC_TAB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
Table 3-B.2a Possible quantization per subband

Fs = 48 kHz Bit rates per channel = 56, 64, 80, 96, 112, 128, 160, 192 kbits/s,
and free format
Fs = 44.1 kHz Bit rates per channel = 56, 64, 80 kbits/s
Fs = 32 kHz Bit rates per channel = 56, 64, 80 kbits/s
*/
extern Ipp8u  mp3_alloc_table1[];

extern Ipp32s mp3_nbal_alloc_table1[32];

/*
Table 3-B.2b. Possible quantization per subband

Fs = 48 kHz -------------- not relevant --------------
Fs = 44.1 kHz Bitrates per channel = 96, 112, 128, 160, 192 kbits/s
and free format
Fs = 32 kHz Bitrates per channel = 96, 112, 128, 160, 192 kbits/s
and free format
*/

extern Ipp8u  mp3_alloc_table2[];

extern Ipp32s mp3_nbal_alloc_table2[32];

/*
Table 3-B.2c. Possible quantization per subband


Fs = 48 kHz Bitrates per channel = 32, 48 kbits/s
Fs = 44.1 kHz Bitrates per channel = 32, 48 kbits/s
Fs = 32 kHz -------- not relevant --------
*/
extern Ipp8u  mp3_alloc_table3[];

extern Ipp32s mp3_nbal_alloc_table3[32];

/*
Table 3-B.2d. Possible quantization per subband


Fs = 48 kHz  ------- not relevant -------
Fs = 44.1kHz  ------- not relevant -------
Fs = 32 kHz  Bitrates per channel = 32, 48 kbits/s
*/

extern Ipp8u  mp3_alloc_table4[];

extern Ipp32s mp3_nbal_alloc_table4[32];

/*
Table B-1 (MPEG 2). Possible quantization per subband

Fs = 16, 22.05, 24 kHz
*/

extern Ipp8u  mp3_alloc_table5[];
extern Ipp32s mp3_nbal_alloc_table5[32];
extern Ipp32s mp3_cls_quant[17];
extern Ipp32s mp3_numbits[17];
extern Ipp32s mp3_sblimit_table[];
extern Ipp16s *mp3_degroup[];

#ifdef __cplusplus
}
#endif

#endif //__MP3_ALLOC_TAB_H__

#endif //UMC_ENABLE_XXX
