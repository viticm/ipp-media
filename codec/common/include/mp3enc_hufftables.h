/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2007 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#if defined (UMC_ENABLE_MP3_AUDIO_ENCODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_ENCODER)

#ifndef __MP3ENC_HUFFTABLES_H__
#define __MP3ENC_HUFFTABLES_H__

#include "ippdc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Ipp32s mp3enc_VLCShifts[];
extern Ipp32s mp3enc_VLCOffsets[];
extern Ipp32s mp3enc_VLCTypes[];
extern Ipp32s mp3enc_VLCTableSizes[];
extern Ipp32s mp3enc_VLCNumSubTables[];
extern Ipp32s *mp3enc_VLCSubTablesSizes[];
extern IppsVLCTable_32s *mp3enc_VLCBooks[];

extern Ipp16s mp3enc_table32[];
extern Ipp16s mp3enc_table33[];

#ifdef __cplusplus
}
#endif

#endif  //      __MP3ENC_HUFFTABLES_H__

#endif //UMC_ENABLE_XXX
