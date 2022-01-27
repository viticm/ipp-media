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
#if defined (UMC_ENABLE_MP3_AUDIO_DECODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_DECODER)

#ifndef __MP3DEC_HUFTABS_H__
#define __MP3DEC_HUFTABS_H__

#include "ippdc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Ipp32s mp3dec_VLCShifts[];
extern Ipp32s mp3dec_VLCOffsets[];
extern Ipp32s mp3dec_VLCTypes[];
extern Ipp32s mp3dec_VLCTableSizes[];
extern Ipp32s mp3dec_VLCNumSubTables[];
extern Ipp32s *mp3dec_VLCSubTablesSizes[];
extern IppsVLCTable_32s *mp3dec_VLCBooks[];

extern Ipp32s mp3idec_VLCShifts[];
extern Ipp32s mp3idec_VLCOffsets[];
extern Ipp32s mp3idec_VLCTypes[];
extern Ipp32s mp3idec_VLCTableSizes[];
extern Ipp32s mp3idec_VLCNumSubTables[];
extern Ipp32s *mp3idec_VLCSubTablesSizes[];
extern IppsVLCTable_32s *mp3idec_VLCBooks[];

#ifdef __cplusplus
}
#endif

#endif //__HUFTABS_H__

#endif //UMC_ENABLE_XXX
