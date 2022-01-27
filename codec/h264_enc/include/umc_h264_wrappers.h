//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_video_data.h"
#include "umc_video_processing.h"

#define UMC_CWRAPPER_FUNC(RES, TYPE, FUNC, PROTO, PTR, ARGS) \
    inline RES TYPE##_##FUNC PROTO { return ((UMC::TYPE *)PTR)->FUNC ARGS; } \

#define UMC_CWRAPPER_CTOR(TYPE) \
    inline void TYPE##_Create(void* p) { new (p) UMC::TYPE(); } \

#define UMC_CWRAPPER_DTOR(TYPE) \
    inline void TYPE##_Destroy(void* p) { ((UMC::TYPE *)p)->~TYPE(); } \

UMC_CWRAPPER_CTOR(VideoData);
UMC_CWRAPPER_DTOR(VideoData);

UMC_CWRAPPER_CTOR(H264EncoderParams);
UMC_CWRAPPER_DTOR(H264EncoderParams);

UMC_CWRAPPER_FUNC(UMC::Status, VideoData, Init,
         (void* p, Ipp32s iWidth, Ipp32s iHeight, ColorFormat cFormat, Ipp32s iBitDepth),
         p, (iWidth, iHeight, cFormat, iBitDepth));

UMC_CWRAPPER_FUNC(UMC::ColorFormat, VideoData, GetColorFormat,
         (void* p),
         p, ());

UMC_CWRAPPER_FUNC(UMC::FrameType, VideoData, GetFrameType,
         (void* p),
         p, ());

UMC_CWRAPPER_FUNC(UMC::Status, VideoData, SetFrameType,
         (void* p, UMC::FrameType ft),
         p, (ft));

UMC_CWRAPPER_FUNC(Ipp32s, VideoData, GetNumPlanes,
         (void* p),
         p, ());

UMC_CWRAPPER_FUNC(void*, VideoData, GetPlanePointer,
         (void* p, Ipp32s iPlaneNumber),
         p, (iPlaneNumber));

UMC_CWRAPPER_FUNC(UMC::Status, VideoData, SetPlanePointer,
         (void* p, void *pDest, Ipp32s iPlaneNumber),
         p, (pDest, iPlaneNumber));

UMC_CWRAPPER_FUNC(Ipp32s, VideoData, GetPlaneBitDepth,
         (void* p, Ipp32s iPlaneNumber),
         p, (iPlaneNumber));

UMC_CWRAPPER_FUNC(UMC::Status, VideoData, SetPlaneBitDepth,
         (void* p, Ipp32s iBitDepth, Ipp32s iPlaneNumber),
         p, (iBitDepth, iPlaneNumber));

UMC_CWRAPPER_FUNC(UMC::Status, VideoData, SetPlanePitch,
         (void* p, size_t nPitch, Ipp32s iPlaneNumber),
         p, (nPitch, iPlaneNumber));

UMC_CWRAPPER_FUNC(UMC::Status, VideoProcessing, GetFrame,
         (void* p, MediaData *input, MediaData *output),
         p, (input, output));

#undef UMC_CWRAPPER_FUNC
#undef UMC_CWRAPPER_CTOR
#undef UMC_CWRAPPER_DTOR
