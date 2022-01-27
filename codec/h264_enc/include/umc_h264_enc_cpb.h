//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef __UMC_H264_ENC_CPB_H__
#define __UMC_H264_ENC_CPB_H__

#include "umc_memory_allocator.h"
#include "umc_h264_defs.h"
#include "vm_debug.h"

namespace UMC_H264_ENCODER
{

#define PIXBITS 8
#include "umc_h264_enc_cpb_tmpl.h"
#undef PIXBITS

#if defined (BITDEPTH_9_12)

#define PIXBITS 16
#include "umc_h264_enc_cpb_tmpl.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER
#endif // __UMC_H264_ENC_CPB_H__

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
