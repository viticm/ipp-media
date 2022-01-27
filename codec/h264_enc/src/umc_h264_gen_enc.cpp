//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#if defined _OPENMP
#include "omp.h"
#endif // _OPENMP
#if defined _WIN32
#include <windows.h>
#undef true
#undef false
#endif

#include <math.h>
#include <string.h>
#include "vm_strings.h"
#include "vm_thread.h"
#include "umc_h264_video_encoder.h"
#include "umc_h264_core_enc.h"
#include "umc_h264_npf.h"
#include "umc_video_data.h"
#include "umc_video_processing.h"
#include "umc_h264_to_ipp.h"

#include "umc_h264_tables.h"
#include "umc_h264_wrappers.h"

namespace UMC_H264_ENCODER
{

#define PIXBITS 8
#include "umc_h264_gen_enc_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_gen_enc_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER


