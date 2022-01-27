//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef UMC_H264_BS_H__
#define UMC_H264_BS_H__

#include "umc_h264_defs.h"
#include "umc_base_bitstream.h"

namespace UMC_H264_ENCODER
{

#define PIXBITS 8
#include "umc_h264_bs_tmpl.h"
#define FAKE_BITSTREAM
    #include "umc_h264_bs_tmpl.h"
#undef FAKE_BITSTREAM
#undef PIXBITS

#ifdef BITDEPTH_9_12

    #define PIXBITS 16
    #include "umc_h264_bs_tmpl.h"
    #define FAKE_BITSTREAM
        #include "umc_h264_bs_tmpl.h"
    #undef FAKE_BITSTREAM
    #undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif // UMC_H264_BS_H__

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
