//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef __UMC_H264_CONFIG__
#define __UMC_H264_CONFIG__

//#define H264_LOG
#undef H264_LOG

#define BITDEPTH_9_12
//#undef BITDEPTH_9_12

#define ALPHA_BLENDING_H264
//#undef ALPHA_BLENDING_H264

//#define H264_STAT
#undef H264_STAT

//#define H264_COMMON_ME
#undef H264_COMMON_ME

//#define FRAME_QP_FROM_FILE "fqp.txt"
#undef FRAME_QP_FROM_FILE

#define TABLE_FUNC
//#undef TABLE_FUNC

#define FRAME_INTERPOLATION
//#undef FRAME_INTERPOLATION

//#define SLICE_CHECK_LIMIT
#undef SLICE_CHECK_LIMIT

//#define ALT_RC
#undef ALT_RC

//#define FRAME_TYPE_DETECT_DS
#undef FRAME_TYPE_DETECT_DS

#undef INTRINSIC_OPT
#if !defined(WIN64)
#if defined (WIN32) || defined (_WIN32) || defined (WIN32E) || defined (_WIN32E) || defined(__i386__) || defined(__x86_64__)
    #if defined(__INTEL_COMPILER) || (_MSC_VER >= 1300) || (defined(__GNUC__) && defined(__SSE2__) && (__GNUC__ > 3))
        #define INTRINSIC_OPT
        #include "emmintrin.h"
    #endif
#endif
#endif // !defined(WIN64)

//#define PRESET_SEI
#undef PRESET_SEI

#endif /* __UMC_H264_CONFIG__ */

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
