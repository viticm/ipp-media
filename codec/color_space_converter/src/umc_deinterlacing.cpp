/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//    Copyright (c) 2003-2008 Intel Corporation. All Rights Reserved.
//
//
*/

#include "umc_deinterlacing.h"
#include "umc_video_data.h"
#include "ippi.h"
#include "ippvc90legacy.h" //#include "ippvc.h"

using namespace UMC;

namespace UMC
{
    BaseCodec *CreateDeinterlacing() { return (new Deinterlacing); }
}

Deinterlacing::Deinterlacing()
{
  mMethod = DEINTERLACING_DUPLICATE;
}

Status Deinterlacing::SetMethod(DeinterlacingMethod method)
{
  mMethod = method;
  if (mMethod == DEINTERLACING_MEDIAN_THRESHOLD)
    mThreshold = 6;
  if (mMethod == DEINTERLACING_CAVT)
    mThreshold = 17;
  return UMC_OK;
}

Status Deinterlacing::SetThreshold(Ipp32s threshold)
{
  mThreshold = threshold;
  return UMC_OK;
}

Status Deinterlacing::GetFrame(MediaData *input, MediaData *output)
{
  VideoData *in = DynamicCast<VideoData>(input);
  VideoData *out = DynamicCast<VideoData>(output);
  DeinterlacingMethod method = mMethod;
  int k;
  int field = (in->GetPictureStructure() & PS_TOP_FIELD_FIRST) ? 1 : 0;

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
  if ( (in_Width != out_Width) || (in_Height != out_Height) ) {
    return UMC_ERR_INVALID_PARAMS;
  }

  for (k = 0; k < in->GetNumPlanes(); k++) {
    VideoData::PlaneInfo srcPlane;
    const Ipp8u *pSrc0; //, *pSrc1;
    Ipp8u *pDst0, *pDst1;
    int srcPitch, dstPitch;
    IppiSize size;

    in->GetPlaneInfo(&srcPlane, k);
    pSrc0 = (const Ipp8u*)in->GetPlanePointer(k);
    srcPitch = in->GetPlanePitch(k);
    pDst0 = (Ipp8u*)out->GetPlanePointer(k);
    dstPitch = out->GetPlanePitch(k);
    size.width = srcPlane.m_ippSize.width * srcPlane.m_iSamples * srcPlane.m_iSampleSize;
    size.height = srcPlane.m_ippSize.height;

    if (method == DEINTERLACING_BLEND) {
      if (srcPlane.m_iSampleSize != 1) {
        //return UMC_ERR_UNSUPPORTED;
        method = DEINTERLACING_DUPLICATE;
      }
      ippiDeinterlaceFilterTriangle_8u_C1R(pSrc0, srcPitch,
                                           pDst0, dstPitch,
                                           size,
                                           128,
                                           IPP_LOWER | IPP_UPPER | IPP_CENTER);
      continue;
    }

    if (method == DEINTERLACING_EDGE_DETECT) {
      if (srcPlane.m_iSampleSize != 1 || srcPlane.m_iSamples != 1) {
        //return UMC_ERR_UNSUPPORTED;
        method = DEINTERLACING_DUPLICATE;
      }

      ippiDeinterlaceEdgeDetect_8u_C1R(pSrc0, srcPitch,
                                       pDst0, dstPitch,
                                       size,
                                       field,
                                       1);
      continue;
    }


    if (method == DEINTERLACING_MEDIAN_THRESHOLD) {
      if (srcPlane.m_iSampleSize != 1 || srcPlane.m_iSamples != 1) {
        //return UMC_ERR_UNSUPPORTED;
        method = DEINTERLACING_DUPLICATE;
      }

      ippiDeinterlaceMedianThreshold_8u_C1R(pSrc0, srcPitch,
                                            pDst0, dstPitch,
                                            size,
                                            mThreshold,
                                            field,
                                            1);
      continue;
    }

    if (method == DEINTERLACING_CAVT) {
      int srcStep, dstStep;
      Ipp8u *pS, *pD;
      if (srcPlane.m_iSampleSize != 1 || srcPlane.m_iSamples != 1) {
        //return UMC_ERR_UNSUPPORTED;
        method = DEINTERLACING_DUPLICATE;
      }

      if (field) {
        pS = (Ipp8u *)pSrc0;
        pD = pDst0;
        srcStep = srcPitch;
        dstStep = dstPitch;
      } else {
        pS = (Ipp8u *)pSrc0 + (size.height - 1)*srcPitch;
        pD = pDst0 + (size.height - 1)*dstPitch;
        srcStep = -srcPitch;
        dstStep = -dstPitch;
      }

      ippiDeinterlaceFilterCAVT_8u_C1R(pS, srcStep,
                                       pD, dstStep,
                                       (Ipp16u)mThreshold,
                                       size);
      continue;
    }


    if (method == DEINTERLACING_MEDIAN) {
      IppiSize roi;
      const Ipp8u *pS[3];
      Ipp8u *pD;
      if (srcPlane.m_iSampleSize != 1 || srcPlane.m_iSamples != 1) {
        //return UMC_ERR_UNSUPPORTED;
        method = DEINTERLACING_DUPLICATE;
      }
      roi.height = size.height >> 1;
      roi.width = size.width;

      if (field == 1) { // top field first, bottom field to generate, top - to copy
        ippiCopy_8u_C1R(pSrc0, 2 * srcPitch, pDst0, 2 * dstPitch, roi);
        ippsCopy_8u(pSrc0 + (size.height - 1) * srcPitch, pDst0 + (size.height - 1) * dstPitch, size.width);
        pS[0] = pSrc0;
        pS[1] = pSrc0 + srcPitch;
        pS[2] = pSrc0 + 2 * srcPitch;
        pD = pDst0 + dstPitch;
      } else {
        ippsCopy_8u(pSrc0, pDst0, size.width);
        ippiCopy_8u_C1R(pSrc0 + srcPitch, 2 * srcPitch, pDst0 + dstPitch, 2 * dstPitch, roi);
        pS[0] = pSrc0 + srcPitch;
        pS[1] = pSrc0 + 2 * srcPitch;
        pS[2] = pSrc0 + 3 * srcPitch;
        pD = pDst0 + 2 * dstPitch;
      }
      roi.height--;

      ippiMedian_8u_P3C1R(pS,
                          2 * srcPitch,
                          pD,
                          2 * dstPitch,
                          roi);

      continue;
    }



    // DEINTERLACING_DUPLICATE
    //pSrc1 = pSrc0 += srcPitch;
    pSrc0 += (field ? 0 : srcPitch);
    srcPitch *= 2;
    pDst1 = pDst0 + dstPitch;
    dstPitch *= 2;
    size.height /= 2;
    ippiCopy_8u_C1R(pSrc0, srcPitch, pDst0, dstPitch, size);
    ippiCopy_8u_C1R(pSrc0, srcPitch, pDst1, dstPitch, size);
  }

  return UMC_OK;
}
