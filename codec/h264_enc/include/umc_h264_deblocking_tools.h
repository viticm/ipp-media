/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2003-2008 Intel Corporation. All Rights Reserved.
//
//
*/

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef __UMC_H264_DEBLOCKING_TOOLS_H
#define __UMC_H264_DEBLOCKING_TOOLS_H

#include <ippi.h>
#include "vm_event.h"
#include "vm_thread.h"
#include "umc_structures.h"
#include "umc_h264_video_encoder.h"
#include "umc_h264_core_enc.h"

using namespace UMC;
namespace UMC_H264_ENCODER
{

#define PIXBITS 8
#include "umc_h264_deblocking_tools_tmpl.h"
#undef PIXBITS

#if defined (BITDEPTH_9_12)

#define PIXBITS 16
#include "umc_h264_deblocking_tools_tmpl.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} // namespace UMC_H264_ENCODER

#endif // UMC_H264_DEBLOCKING_TOOLS_H

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
