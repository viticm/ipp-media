/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//    Copyright (c) 2003-2007 Intel Corporation. All Rights Reserved.
//
//
*/

#include "umc_video_resizing.h"
#include "umc_video_data.h"
#include "ippi.h"

using namespace UMC;

namespace UMC
{
    BaseCodec *CreateVideoResizing() { return (new VideoResizing); }
}

VideoResizing::VideoResizing()
{
  mInterpolation = IPPI_INTER_NN;
}

Status VideoResizing::SetMethod(int lInterpolation)
{
  mInterpolation = lInterpolation;
  return UMC_OK;
}

Status VideoResizing::GetFrame(MediaData *input, MediaData *output)
{
  VideoData *in = DynamicCast<VideoData>(input);
  VideoData *out = DynamicCast<VideoData>(output);
  VideoData::PlaneInfo srcPlane;
  VideoData::PlaneInfo dstPlane;
  int k;

  if (NULL == in || NULL == out) {
    return UMC_ERR_NULL_PTR;
  }

  ColorFormat cFormat = in->GetColorFormat();
  if (out->GetColorFormat() != cFormat) {
    return UMC_ERR_INVALID_PARAMS;
  }
  Ipp32s in_Width = in->GetWidth();
  Ipp32s in_Height = in->GetHeight();
  Ipp32s out_Width = out->GetWidth();
  Ipp32s out_Height = out->GetHeight();

  Ipp64f xRatio = static_cast<Ipp64f> (out_Width)  / static_cast<Ipp64f> (in_Width);
  Ipp64f yRatio = static_cast<Ipp64f> (out_Height) / static_cast<Ipp64f> (in_Height);

  for (k = 0; k < in->GetNumPlanes(); k++) {
    in->GetPlaneInfo(&srcPlane, k);
    out->GetPlaneInfo(&dstPlane, k);

    UMC_CHECK(srcPlane.m_iSampleSize == dstPlane.m_iSampleSize, UMC_ERR_INVALID_PARAMS);
    UMC_CHECK(srcPlane.m_iSamples == dstPlane.m_iSamples, UMC_ERR_INVALID_PARAMS);

    IppiRect RectSrc = {0, 0, srcPlane.m_ippSize.width, srcPlane.m_ippSize.height};

    if (cFormat == YUY2) {
      // YUY2 format defined in VideoData with WidthDiv = 2
      srcPlane.m_ippSize.width *= 2;
      dstPlane.m_ippSize.width *= 2;
      RectSrc.width *= 2;
      ippiResizeYUV422_8u_C2R((const Ipp8u *)srcPlane.m_pPlane,
        srcPlane.m_ippSize,
        srcPlane.m_nPitch,
        RectSrc,
        (Ipp8u *)dstPlane.m_pPlane,
        dstPlane.m_nPitch,
        dstPlane.m_ippSize,
        xRatio,
        yRatio,
        mInterpolation);
      return UMC_OK;
    }

    if (srcPlane.m_iSampleSize == sizeof(Ipp8u)) {
      switch (srcPlane.m_iSamples) {
      case 1:
        ippiResize_8u_C1R((const Ipp8u *)srcPlane.m_pPlane,
                          srcPlane.m_ippSize,
                          srcPlane.m_nPitch,
                          RectSrc,
                          (Ipp8u *)dstPlane.m_pPlane,
                          dstPlane.m_nPitch,
                          dstPlane.m_ippSize,
                          xRatio,
                          yRatio,
                          mInterpolation);
        break;
      case 3:
        ippiResize_8u_C3R((const Ipp8u *)srcPlane.m_pPlane,
                          srcPlane.m_ippSize,
                          srcPlane.m_nPitch,
                          RectSrc,
                          (Ipp8u *)dstPlane.m_pPlane,
                          dstPlane.m_nPitch,
                          dstPlane.m_ippSize,
                          xRatio,
                          yRatio,
                          mInterpolation);
        break;
      case 4:
        ippiResize_8u_C4R((const Ipp8u *)srcPlane.m_pPlane,
                          srcPlane.m_ippSize,
                          srcPlane.m_nPitch,
                          RectSrc,
                          (Ipp8u *)dstPlane.m_pPlane,
                          dstPlane.m_nPitch,
                          dstPlane.m_ippSize,
                          xRatio,
                          yRatio,
                          mInterpolation);
        break;
      default:
        return UMC_ERR_UNSUPPORTED;
      }
    } else if (srcPlane.m_iSampleSize == sizeof(Ipp16u)) {
      switch (srcPlane.m_iSamples) {
      case 1:
        ippiResize_16u_C1R((const Ipp16u *)srcPlane.m_pPlane,
                           srcPlane.m_ippSize,
                           srcPlane.m_nPitch,
                           RectSrc,
                           (Ipp16u *)dstPlane.m_pPlane,
                           dstPlane.m_nPitch,
                           dstPlane.m_ippSize,
                           xRatio,
                           yRatio,
                           mInterpolation);
        break;
      case 3:
        ippiResize_16u_C3R((const Ipp16u *)srcPlane.m_pPlane,
                           srcPlane.m_ippSize,
                           srcPlane.m_nPitch,
                           RectSrc,
                           (Ipp16u *)dstPlane.m_pPlane,
                           dstPlane.m_nPitch,
                           dstPlane.m_ippSize,
                           xRatio,
                           yRatio,
                           mInterpolation);
        break;
      case 4:
        ippiResize_16u_C4R((const Ipp16u *)srcPlane.m_pPlane,
                           srcPlane.m_ippSize,
                           srcPlane.m_nPitch,
                           RectSrc,
                           (Ipp16u *)dstPlane.m_pPlane,
                           dstPlane.m_nPitch,
                           dstPlane.m_ippSize,
                           xRatio,
                           yRatio,
                           mInterpolation);
        break;
      default:
        return UMC_ERR_UNSUPPORTED;
      }
    } else {
      return UMC_ERR_UNSUPPORTED;
    }
  }
  return UMC_OK;
}
