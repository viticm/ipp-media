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

#include "umc_color_space_conversion.h"
#include "umc_video_data.h"
#include "ippi.h"
#include "ippcc.h"
#include "ippvc.h"

using namespace UMC;

namespace UMC
{
    BaseCodec *CreateColorSpaceConversion() { return (new ColorSpaceConversion); }
}

template <class T> inline
void SwapValues(T &one, T& two)
{
  T tmp;
  tmp = one;
  one = two;
  two = tmp;
}

static void ConvertImage_16s8u_C1R(const Ipp16s *pSrc,
                                  Ipp32s   iSrcStride,
                                  Ipp32s   iSrcBitsPerSample,
                                  Ipp8u    *pDst,
                                  Ipp32s   iDstStride,
                                  IppiSize size);
static IppStatus cc_BGRAToBGR(const Ipp8u *pSrc,
                              Ipp32s   iSrcStride,
                              Ipp8u    *pDst,
                              Ipp32s   iDstStride,
                              IppiSize srcSize);
static IppStatus cc_BGRToBGRA(const Ipp8u *pSrc,
                              Ipp32s   iSrcStride,
                              Ipp8u    *pDst,
                              Ipp32s   iDstStride,
                              IppiSize srcSize);
static IppStatus cc_BGR555ToBGR(const Ipp16u *pSrc,
                                Ipp32s   iSrcStride,
                                Ipp8u    *pDst,
                                Ipp32s   iDstStride,
                                IppiSize srcSize);
static IppStatus cc_BGR565ToBGR(const Ipp16u *pSrc,
                                Ipp32s   iSrcStride,
                                Ipp8u    *pDst,
                                Ipp32s   iDstStride,
                                IppiSize srcSize);
static IppStatus cc_Y41P_to_I420(const Ipp8u *pSrc,
                                 Ipp32s   iSrcStride,
                                 Ipp8u    **pDst,
                                 Ipp32s   *iDstStride,
                                 IppiSize srcSize);
static IppStatus cc_I420_to_Y41P(const Ipp8u **pSrc,
                                 Ipp32s   *iSrcStride,
                                 Ipp8u    *pDst,
                                 Ipp32s   iDstStride,
                                 IppiSize srcSize);

static IppStatus cc_YUV411_to_YUV420(const Ipp8u *pSrc[3],
                                     Ipp32s   iSrcStride[3],
                                     Ipp8u    *pDst[3],
                                     Ipp32s   iDstStride[3],
                                     IppiSize srcSize);

static Status CopyImage(VideoData *pSrc, VideoData *pDst, int flag, int bSwapUV)
{
  VideoData::PlaneInfo src;
  VideoData::PlaneInfo dst;
  IppiSize size;
  int cPlanes;
  int iDstPlane;
  IppStatus sts = ippStsNoErr;

  cPlanes = pSrc->GetNumPlanes();
  if (cPlanes > pDst->GetNumPlanes()) cPlanes = pDst->GetNumPlanes();

  for (iDstPlane = 0; iDstPlane < cPlanes; iDstPlane++) {
    int iSrcPlane = iDstPlane;
    if (bSwapUV) {
      if (iDstPlane == 1) iSrcPlane = 2; else
      if (iDstPlane == 2) iSrcPlane = 1;
    }

    pSrc->GetPlaneInfo(&src, iSrcPlane);
    pDst->GetPlaneInfo(&dst, iDstPlane);

    size.width = src.m_ippSize.width * src.m_iSamples;
    size.height = src.m_ippSize.height;

    if (src.m_iSampleSize == dst.m_iSampleSize) {
      size.width *= src.m_iSampleSize;
      if (flag == 2 && src.m_iBitDepth >= 0) { // case VC1->YUV420
        sts = ippiRangeMapping_VC1_8u_C1R(src.m_pPlane, src.m_nPitch, dst.m_pPlane, dst.m_nPitch, size, src.m_iBitDepth);
      } else {
        sts = ippiCopy_8u_C1R(src.m_pPlane, src.m_nPitch, dst.m_pPlane, dst.m_nPitch, size);
      }
    } else if (src.m_iSampleSize == 2 && dst.m_iSampleSize == 1) {
      ConvertImage_16s8u_C1R((const Ipp16s*)src.m_pPlane, src.m_nPitch, src.m_iBitDepth, dst.m_pPlane, dst.m_nPitch, size);
    } else {
      return UMC_ERR_UNSUPPORTED;
    }
  }

  return (ippStsNoErr == sts) ? UMC_OK : UMC_ERR_FAILED;
}

Status ColorSpaceConversion::GetFrame(MediaData *input, MediaData *output)
{
  VideoData *in = DynamicCast<VideoData>(input);
  VideoData *out = DynamicCast<VideoData>(output);

  UMC_CHECK(in, UMC_ERR_NULL_PTR);
  UMC_CHECK(out, UMC_ERR_NULL_PTR);

  IppiSize srcSize = {in->GetWidth(), in->GetHeight()};
  IppiSize dstSize = {out->GetWidth(), out->GetHeight()};
  UMC_CHECK(srcSize.width == dstSize.width, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(srcSize.height == dstSize.height, UMC_ERR_INVALID_PARAMS);


  ColorFormat srcFormat = in->GetColorFormat();
  ColorFormat dstFormat = out->GetColorFormat();
  int bSrcSwapUV = 0;
  int bDstSwapUV = 0;

  if (srcFormat == YV12) // process YV12 as YUV420
  {
    bSrcSwapUV = 1;
    srcFormat = YUV420;
  }
  if (dstFormat == YV12) // process YV12 as YUV420
  {
    bDstSwapUV = 1;
    dstFormat = YUV420;
  }

  int flag_OnlyCopy = 0;
  if (srcFormat == dstFormat) flag_OnlyCopy = 1;
  if (YUV_VC1 == srcFormat && YUV420 == dstFormat) flag_OnlyCopy = 2;
  if (YUV420A == srcFormat && YUV420 == dstFormat) flag_OnlyCopy = 3;
  if (GRAYA == srcFormat && GRAY == dstFormat) flag_OnlyCopy = 4;
  if (flag_OnlyCopy) {
      return CopyImage(in, out, flag_OnlyCopy, bSrcSwapUV ^ bDstSwapUV);
  }

  const Ipp8u *(pSrc[3]) = {(Ipp8u*)in->GetPlanePointer(0),
                            (Ipp8u*)in->GetPlanePointer(1),
                            (Ipp8u*)in->GetPlanePointer(2)};
  Ipp32s pSrcStep[3] = {in->GetPlanePitch(0),
                        in->GetPlanePitch(1),
                        in->GetPlanePitch(2)};
  Ipp8u *(pDst[3]) = {(Ipp8u*)out->GetPlanePointer(0),
                      (Ipp8u*)out->GetPlanePointer(1),
                      (Ipp8u*)out->GetPlanePointer(2)};
  Ipp32s pDstStep[3] = {out->GetPlanePitch(0),
                        out->GetPlanePitch(1),
                        out->GetPlanePitch(2)};
  if (bSrcSwapUV) {
    SwapValues(pSrc[1], pSrc[2]);
    SwapValues(pSrcStep[1], pSrcStep[2]);
  }
  if (bDstSwapUV) {
    SwapValues(pDst[1], pDst[2]);
    SwapValues(pDstStep[1], pDstStep[2]);
  }
  if (srcFormat == YUV422 && dstFormat != YUV420) { // 422->X as 420->X
    pSrcStep[1] *= 2;
    pSrcStep[2] *= 2;
    srcFormat = YUV420;
  }
  const Ipp8u *(pYVU[3]) = {pSrc[0], pSrc[2], pSrc[1]};
  Ipp32s pYVUStep[3] = {pSrcStep[0], pSrcStep[2], pSrcStep[1]};
  Ipp8u *(pDstYVU[3]) = {pDst[0], pDst[2], pDst[1]};
  Ipp32s pDstStepYVU[3] = {pDstStep[0], pDstStep[2], pDstStep[1]};
  IppStatus status;

  switch (srcFormat) {
  case YUV411:
    switch (dstFormat) {
    case YUV420:
      //status = ippiYCbCr411ToYCbCr420_8u_P3R(pSrc, pSrcStep, pDst, pDstStep, srcSize);
      status = cc_YUV411_to_YUV420(pSrc, pSrcStep, pDst, pDstStep, srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case YUV420:
    switch (dstFormat) {
    case YUV411:
      status = ippiYCbCr420To411_8u_P3R(pSrc, pSrcStep, pDst, pDstStep, srcSize);
      break;
    case YUV422:
      status = ippiYCbCr420ToYCbCr422_8u_P3R(pSrc, pSrcStep, pDst, pDstStep, srcSize);
      break;
    case Y41P:
      status = cc_I420_to_Y41P(pYVU, pYVUStep, pDst[0], pDstStep[0], srcSize);
      break;
    case NV12:
      status = ippiYCrCb420ToYCbCr420_8u_P3P2R(pYVU, pYVUStep, pDst[0], pDstStep[0], pDst[1], pDstStep[1], srcSize);
      break;
    case YUY2:
      status = ippiYCrCb420ToYCbCr422_8u_P3C2R(pYVU, pYVUStep, pDst[0], pDstStep[0], srcSize);
      break;
    case UYVY:
      status = ippiYCrCb420ToCbYCr422_8u_P3C2R(pYVU, pYVUStep, pDst[0], pDstStep[0], srcSize);
      break;
    case RGB24:
      status = ippiYCbCr420ToBGR_8u_P3C3R(pSrc, pSrcStep, pDst[0], pDstStep[0], srcSize);
      break;
    case RGB32:
      status = ippiYCrCb420ToBGR_Filter_8u_P3C4R(pYVU, pYVUStep, pDst[0], pDstStep[0], srcSize, 0);
      break;
    case RGB565:
      status = ippiYCbCr420ToBGR565_8u16u_P3C3R(pSrc, pSrcStep, (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    case RGB555:
      status = ippiYCbCr420ToBGR555_8u16u_P3C3R(pSrc, pSrcStep, (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    case RGB444:
      status = ippiYCbCr420ToBGR444_8u16u_P3C3R(pSrc, pSrcStep, (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case YUV422:
  case YUV422A:
    switch (dstFormat) {
    case YUV420:
      status = ippiYCbCr422ToYCbCr420_8u_P3R(pSrc, pSrcStep, pDst, pDstStep, srcSize);
      break;
    case YUY2:
      status = ippiYCbCr422_8u_P3C2R(pSrc, pSrcStep, pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case YUY2:
    switch (dstFormat) {
    case YUV420:
      status = ippiYCbCr422ToYCrCb420_8u_C2P3R(pSrc[0], pSrcStep[0], pDst, pDstStep, srcSize);
      break;
    case NV12:
      status = ippiYCbCr422ToYCbCr420_8u_C2P2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], pDst[1], pDstStep[1], srcSize);
      break;
    case UYVY:
      status = ippiYCbCr422ToCbYCr422_8u_C2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case YUV422:
      status = ippiYCrCb422ToYCbCr422_8u_C2P3R( pSrc[0], pSrcStep[0], pDst, pDstStep, srcSize);
      break;
    case RGB24:
      status = ippiYCbCr422ToBGR_8u_C2C3R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB32:
      status = ippiYCbCr422ToBGR_8u_C2C4R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize, 0);
      break;
    case RGB565:
      status = ippiYCbCr422ToBGR565_8u16u_C2C3R(pSrc[0], pSrcStep[0], (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    case RGB555:
      status = ippiYCbCr422ToBGR555_8u16u_C2C3R(pSrc[0], pSrcStep[0], (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    case RGB444:
      status = ippiYCbCr422ToBGR444_8u16u_C2C3R(pSrc[0], pSrcStep[0], (Ipp16u*)pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case UYVY:
    switch (dstFormat) {
    case YUV420:
      status = ippiCbYCr422ToYCrCb420_8u_C2P3R(pSrc[0], pSrcStep[0], pDstYVU, pDstStepYVU, srcSize);
      break;
    case NV12:
      status = ippiCbYCr422ToYCbCr420_8u_C2P2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], pDst[1], pDstStep[1], srcSize);
      break;
    case YUY2:
      status = ippiCbYCr422ToYCbCr422_8u_C2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB32:
      status = ippiCbYCr422ToBGR_8u_C2C4R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize, 0);
      break;
    case YUV422:
      status = ippiCbYCr422ToYCbCr422_8u_C2P3R( pSrc[0], pSrcStep[0], pDst, pDstStep, srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case NV12:
    switch (dstFormat) {
    case YUV420:
      status = ippiYCbCr420_8u_P2P3R(pSrc[0], pSrcStep[0], pSrc[1], pSrcStep[1], pDst, pDstStep, srcSize);
      break;
    case UYVY:
      status = ippiYCbCr420ToCbYCr422_8u_P2C2R(pSrc[0], pSrcStep[0], pSrc[1], pSrcStep[1], pDst[0], pDstStep[0], srcSize);
      break;
    case YUY2:
      status = ippiYCbCr420ToYCbCr422_8u_P2C2R(pSrc[0], pSrcStep[0], pSrc[1], pSrcStep[1], pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case RGB24:
    switch (dstFormat) {
    case YUV420:
      status = ippiBGRToYCrCb420_8u_C3P3R(pSrc[0], pSrcStep[0], pDstYVU, pDstStepYVU, srcSize);
      break;
    case YUY2:
      status = ippiBGRToYCbCr422_8u_C3C2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB32:
      status = cc_BGRToBGRA(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case RGB32:
    switch (dstFormat) {
    case YUV420:
      status = ippiBGRToYCrCb420_8u_AC4P3R(pSrc[0], pSrcStep[0], pDstYVU, pDstStepYVU, srcSize);
      break;
    case YUY2:
      status = ippiBGRToYCbCr422_8u_AC4C2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case UYVY:
      status = ippiBGRToCbYCr422_8u_AC4C2R(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB24:
      status = cc_BGRAToBGR(pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case RGB555:
    switch (dstFormat) {
    case YUV420:
      status = ippiBGR555ToYCrCb420_16u8u_C3P3R((const Ipp16u*)pSrc[0], pSrcStep[0], pDstYVU, pDstStepYVU, srcSize);
      break;
    case YUY2:
      status = ippiBGR555ToYCbCr422_16u8u_C3C2R((const Ipp16u*)pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB24:
      status = cc_BGR555ToBGR((const Ipp16u*)pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case RGB565:
    switch (dstFormat) {
    case YUV420:
      status = ippiBGR565ToYCrCb420_16u8u_C3P3R((const Ipp16u*)pSrc[0], pSrcStep[0], pDstYVU, pDstStepYVU, srcSize);
      break;
    case YUY2:
      status = ippiBGR565ToYCbCr422_16u8u_C3C2R((const Ipp16u*)pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    case RGB24:
      status = cc_BGR565ToBGR((const Ipp16u*)pSrc[0], pSrcStep[0], pDst[0], pDstStep[0], srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  case Y41P:
    switch (dstFormat) {
    case YUV420:
      status = cc_Y41P_to_I420(pSrc[0], pSrcStep[0], pDst, pDstStep, srcSize);
      break;
    default:
      return UMC_ERR_NOT_IMPLEMENTED;
    }
    break;
  default:
    return UMC_ERR_NOT_IMPLEMENTED;
  }

  return (status == ippStsNoErr) ? UMC_OK : UMC_ERR_INVALID_PARAMS;
}

////////////////////////////////////////////////////////////////////////

static void ConvertImage_16s8u_C1R(const Ipp16s *pSrc,
                                  Ipp32s   iSrcStride,
                                  Ipp32s   iSrcBitsPerSample,
                                  Ipp8u    *pDst,
                                  Ipp32s   iDstStride,
                                  IppiSize size)
{
  int iWidth = size.width;
  int iHeight = size.height;
  int rnd = (1 << (iSrcBitsPerSample - 8 - 1));
  Ipp32s x, y;

  for (y = 0; y < iHeight; y += 1) {
    for (x = 0; x < iWidth; x += 1) {
      pDst[x] = (Ipp8u) ((pSrc[x] + rnd) >> (iSrcBitsPerSample - 8));
    }
    pSrc = (Ipp16s *) ((Ipp8u *) pSrc + iSrcStride);
    pDst = pDst + iDstStride;
  }
}

IppStatus cc_BGRAToBGR(const Ipp8u *pSrc,
                       Ipp32s   iSrcStride,
                       Ipp8u    *pDst,
                       Ipp32s   iDstStride,
                       IppiSize srcSize)
{
  int i, j ;

  for (i = 0; i < srcSize.height; i++) {
    for (j = 0; j < srcSize.width; j++) {
      pDst[3*j + 0] = pSrc[4*j + 0];
      pDst[3*j + 1] = pSrc[4*j + 1];
      pDst[3*j + 2] = pSrc[4*j + 2];
    }
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
  return ippStsNoErr;
}

IppStatus cc_BGRToBGRA(const Ipp8u *pSrc,
                       Ipp32s   iSrcStride,
                       Ipp8u    *pDst,
                       Ipp32s   iDstStride,
                       IppiSize srcSize)
{
  int i, j ;

  for (i = 0; i < srcSize.height; i++) {
    for (j = 0; j < srcSize.width; j++) {
      pDst[4*j + 0] = pSrc[3*j + 0];
      pDst[4*j + 1] = pSrc[3*j + 1];
      pDst[4*j + 2] = pSrc[3*j + 2];
      pDst[4*j + 3] = 0;
    }
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
  return ippStsNoErr;
}

IppStatus cc_BGR555ToBGR(const Ipp16u *pSrc,
                         Ipp32s   iSrcStride,
                         Ipp8u    *pDst,
                         Ipp32s   iDstStride,
                         IppiSize srcSize)
{
  int i, j;
  for (i = 0; i < srcSize.height; i++) {
    for (j = 0; j < srcSize.width; j++) {
      Ipp16u pix = pSrc[j];
      //pDst[3*j + 0] = (Ipp8u)((pix >> 10) & 0x1f);
      //pDst[3*j + 1] = (Ipp8u)((pix >> 5) & 0x1f);
      //pDst[3*j + 2] = (Ipp8u)((pix & 0x1f) << 3);
      pDst[3*j + 2] = (Ipp8u)((pix & 0x7c00) >> 7);
      pDst[3*j + 1] = (Ipp8u)((pix & 0x03e0) >> 2);
      pDst[3*j + 0] = (Ipp8u)((pix & 0x001f) << 3);
    }
    pSrc = (Ipp16u *) ((Ipp8u *) pSrc + iSrcStride);
    pDst += iDstStride;
  }
  return ippStsNoErr;
}

IppStatus cc_BGR565ToBGR(const Ipp16u *pSrc,
                         Ipp32s   iSrcStride,
                         Ipp8u    *pDst,
                         Ipp32s   iDstStride,
                         IppiSize srcSize)
{
  int i, j;
  for (i = 0; i < srcSize.height; i++) {
    for (j = 0; j < srcSize.width; j++) {
      Ipp16u pix = pSrc[j];
      pDst[3*j + 2] = (Ipp8u)((pix & 0xf800) >> 8);
      //pDst[3*j + 0] = (Ipp8u)((pix & 0x001f) << 3);
      pDst[3*j + 1] = (Ipp8u)((pix & 0x07e0) >> 3);
      pDst[3*j + 0] = (Ipp8u)((pix & 0x001f) << 3);
      //pDst[3*j + 2] = (Ipp8u)((pix & 0xf800) >> 8);
    }
    pSrc = (Ipp16u *) ((Ipp8u *) pSrc + iSrcStride);
    pDst += iDstStride;
  }
  return ippStsNoErr;
}

IppStatus cc_Y41P_to_I420(const Ipp8u *pSrc,
                          Ipp32s   iSrcStride,
                          Ipp8u    **pDst,
                          Ipp32s   *iDstStride,
                          IppiSize srcSize)
{
  Ipp8u *Y = pDst[0];
  Ipp8u *U = pDst[1];
  Ipp8u *V = pDst[2];
  int Y_stride = iDstStride[0];
  int U_stride = iDstStride[1];
  int V_stride = iDstStride[2];
  int i, j;

  for (j = 0; j < srcSize.height; j++) {
    for (i = 0; i < srcSize.width/8; i++) {
      int U0 = pSrc[12*i + 0];
      int Y0 = pSrc[12*i + 1];
      int V0 = pSrc[12*i + 2];
      int Y1 = pSrc[12*i + 3];
      int U4 = pSrc[12*i + 4];
      int Y2 = pSrc[12*i + 5];
      int V4 = pSrc[12*i + 6];
      int Y3 = pSrc[12*i + 7];
      int Y4 = pSrc[12*i + 8];
      int Y5 = pSrc[12*i + 9];
      int Y6 = pSrc[12*i + 10];
      int Y7 = pSrc[12*i + 11];
      Y[8*i + 0] = (Ipp8u)Y0;
      Y[8*i + 1] = (Ipp8u)Y1;
      Y[8*i + 2] = (Ipp8u)Y2;
      Y[8*i + 3] = (Ipp8u)Y3;
      Y[8*i + 4] = (Ipp8u)Y4;
      Y[8*i + 5] = (Ipp8u)Y5;
      Y[8*i + 6] = (Ipp8u)Y6;
      Y[8*i + 7] = (Ipp8u)Y7;
      if (!(j & 1)) {
        U[4*i + 0] = (Ipp8u)U0;
        U[4*i + 1] = (Ipp8u)U0;
        U[4*i + 2] = (Ipp8u)U4;
        U[4*i + 3] = (Ipp8u)U4;
        V[4*i + 0] = (Ipp8u)V0;
        V[4*i + 1] = (Ipp8u)V0;
        V[4*i + 2] = (Ipp8u)V4;
        V[4*i + 3] = (Ipp8u)V4;
      }
    }
    pSrc += iSrcStride;
    Y += Y_stride;
    if (j & 1) {
      U += U_stride;
      V += V_stride;
    }
  }
  return (IppStatus)0;
}

IppStatus cc_I420_to_Y41P(const Ipp8u **pSrc,
                          Ipp32s   *iSrcStride,
                          Ipp8u    *pDst,
                          Ipp32s   iDstStride,
                          IppiSize srcSize)
{
  const Ipp8u *Y = pSrc[0];
  const Ipp8u *U = pSrc[1];
  const Ipp8u *V = pSrc[2];
  int Y_stride = iSrcStride[0];
  int U_stride = iSrcStride[1];
  int V_stride = iSrcStride[2];
  int i, j;

  for (j = 0; j < srcSize.height; j++) {
    for (i = 0; i < srcSize.width/8; i++) {
      int Y0 = Y[8*i + 0];
      int Y1 = Y[8*i + 1];
      int Y2 = Y[8*i + 2];
      int Y3 = Y[8*i + 3];
      int Y4 = Y[8*i + 4];
      int Y5 = Y[8*i + 5];
      int Y6 = Y[8*i + 6];
      int Y7 = Y[8*i + 7];
      int U0 = U[4*i + 0];
      int U4 = U[4*i + 2];
      int V0 = V[4*i + 0];
      int V4 = V[4*i + 2];
      pDst[12*i + 0] = (Ipp8u)U0;
      pDst[12*i + 1] = (Ipp8u)Y0;
      pDst[12*i + 2] = (Ipp8u)V0;
      pDst[12*i + 3] = (Ipp8u)Y1;
      pDst[12*i + 4] = (Ipp8u)U4;
      pDst[12*i + 5] = (Ipp8u)Y2;
      pDst[12*i + 6] = (Ipp8u)V4;
      pDst[12*i + 7] = (Ipp8u)Y3;
      pDst[12*i + 8] = (Ipp8u)Y4;
      pDst[12*i + 9] = (Ipp8u)Y5;
      pDst[12*i + 10] = (Ipp8u)Y6;
      pDst[12*i + 11] = (Ipp8u)Y7;
    }
    pDst += iDstStride;
    Y += Y_stride;
    if (j & 1) {
      U += U_stride;
      V += V_stride;
    }
  }
  return (IppStatus)0;
}

IppStatus cc_YUV411_to_YUV420(const Ipp8u *pSrc[3],
                          Ipp32s   pSrcStep[3],
                          Ipp8u    *pDst[3],
                          Ipp32s   pDstStep[3],
                          IppiSize srcSize)
{
    Ipp32s h,w;
    Ipp32s srcStepU , srcStepV ;
    Ipp32s dstStepU , dstStepV ;
    int  width ;
    int  height ;
    const Ipp8u* srcu;
    const Ipp8u* srcv;
    IppStatus sts = ippStsNoErr;

    Ipp8u* dstu;
    Ipp8u* dstv;

    srcu = pSrc[1];
    srcv = pSrc[2];
    dstu = pDst[1];
    dstv = pDst[2];
    srcStepU = pSrcStep[1];
    srcStepV = pSrcStep[2];
    dstStepU = pDstStep[1];
    dstStepV = pDstStep[2];
    width  = srcSize.width ;
    height = srcSize.height;

    /* Y plane */
    sts = ippiCopy_8u_C1R( pSrc[0], pSrcStep[0], pDst[0], pDstStep[0],  srcSize );
    if( ippStsNoErr != sts ) return sts;

    for( h = 0; h < height ; h +=2)
    {
        for( w = 0; w < (width/4 -1) ;w ++ )
        {
            dstu[w*2] = srcu[w];
            dstu[w*2+1] = (srcu[w] + srcu[w+1]) / 2;

            dstv[w*2] = srcv[w];
            dstv[w*2+1] = (srcv[w] + srcv[w+1]) / 2;
        }
        dstu[w*2] = dstu[w*2 + 1] = srcu[w];
        dstv[w*2] = dstv[w*2 + 1] = srcv[w];

        srcu += 2*srcStepU;
        dstu += dstStepU;
        srcv += 2*srcStepV;
        dstv += dstStepV;
    }

    return (IppStatus)0;
}

namespace UMC
{

Status FillBlockWithColor(VideoData *pData, int y, int u, int v)
{
    IppiSize ySize = {16, 16};
    IppiSize lSize = {8, 8};
    Ipp8u yuyv[4] = {(Ipp8u)y, (Ipp8u)u, (Ipp8u)y, (Ipp8u)v};
    Ipp16s uv = (Ipp16s)(u | (v << 8));

    switch (pData->GetColorFormat())
    {
    case YV12:
    case YUV420:
        ippiSet_8u_C1R((Ipp8u)y, (Ipp8u*)pData->GetPlanePointer(0), pData->GetPlanePitch(0), ySize);
        ippiSet_8u_C1R((Ipp8u)u, (Ipp8u*)pData->GetPlanePointer(1), pData->GetPlanePitch(1), lSize);
        ippiSet_8u_C1R((Ipp8u)v, (Ipp8u*)pData->GetPlanePointer(2), pData->GetPlanePitch(2), lSize);
        return UMC_OK;
    case YUY2:
    case UYVY:
        ippiSet_8u_C4R(yuyv, (Ipp8u*)pData->GetPlanePointer(0), pData->GetPlanePitch(0), ySize);
        return UMC_OK;
    case NV12:
        ippiSet_8u_C1R((Ipp8u)y,   (Ipp8u*)pData->GetPlanePointer(0), pData->GetPlanePitch(0), ySize);
        ippiSet_16s_C1R(uv, (Ipp16s*)pData->GetPlanePointer(1), pData->GetPlanePitch(1), lSize);
        return UMC_OK;
    default:
        return UMC_ERR_NOT_IMPLEMENTED;
    }
}

} // namespace UMC
