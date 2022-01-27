/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#include "umc_video_processing.h"

#include "umc_video_resizing.h"
#include "umc_deinterlacing.h"
#include "umc_d3d_video_processing.h"

using namespace UMC;

namespace UMC
{
  BaseCodec *createVideoProcessing()
  {
    return (new VideoProcessing);
  }

  BaseCodec *CreateVideoProcessing()
  {
    return (new VideoProcessing);
  }
}

VideoProcessing::VideoProcessing()
{
  memset(pFilter, 0, sizeof(pFilter));
  memset(bFiltering, 0, sizeof(bFiltering));
  bSrcCropArea = false;

  // order of filters
  numFilters = 0;
  iDeinterlacing = numFilters++;
  iColorConv0 = numFilters++;
  iColorConv = numFilters++;
  iResizing = numFilters++;
  // create filters
  pFilter[iDeinterlacing] = CreateDeinterlacing();
  pFilter[iColorConv0] = CreateColorSpaceConversion(); // first color conversion
  pFilter[iColorConv] = pFilter[iColorConv0]; // second color conversion
  pFilter[iResizing] = CreateVideoResizing();
}

Status VideoProcessing::Close()
{
    int i;

    pFilter[iColorConv0] = NULL; // duplication of iColorConv!!!

    for (i = 0; i < UMC_ARRAY_SIZE(pFilter); i++)
    {
        UMC_DELETE(pFilter[i]);
    }

    return UMC_OK;
}

Status VideoProcessing::AddFilter(BaseCodec *filter, int atEnd)
{
  int k;
  if (atEnd) {
    bFiltering[numFilters] = true;
    pFilter[numFilters++] = filter;
  } else {
    for (k = numFilters; k > 0; k--) {
      pFilter[k] = pFilter[k - 1];
      bFiltering[k] = bFiltering[k - 1];
    }
    bFiltering[0] = true;
    pFilter[0] = filter;
    iDeinterlacing++;
    iColorConv0++;
    iColorConv++;
    iResizing++;
    numFilters++;
  }
  return UMC_OK;
}

Status VideoProcessing::Init(BaseCodecParams *init)
{
  return SetParams(init);
}

static Status InitVideoData(VideoData *tmpData, int w, int h, ColorFormat color)
{
  if (tmpData->GetWidth() != w ||
      tmpData->GetHeight() != h ||
      tmpData->GetColorFormat() != color)
  {
    tmpData->Init(w, h, color);
    return tmpData->Alloc();
  }
  return UMC_OK;
}

static ColorFormat GetIntermediatedColor(ColorFormat src_color)
{
  switch (src_color) {
    case YV12: return YUV420;
    case NV12: return YUV420;
    case YUY2: return YUV420;
    case UYVY: return YUV420;
    case YUV411: return YUV422;
    case YUV420: return YV12;
    case YUV422: return YUY2;
    case YUV444: return RGB24;
    case YUV_VC1: return YUV420;
    case Y411: return YUV411;
    case Y41P: return YUV411;
    case RGB32: return RGB24;
    case RGB24: return RGB32;
    case RGB565: return RGB24;
    case RGB555: return RGB24;
    case RGB444: return RGB24;
    case GRAY: return YUV420;
    case YUV420A: return YUV420;
    case YUV422A: return YUV422;
    case YUV444A: return YUV444;
    case YVU9: return YUV420;
  }
  return YUV420;
}

Status VideoProcessing::GetFrame(MediaData *input, MediaData *output)
{
  VideoData *in = DynamicCast<VideoData>(input);
  VideoData *out = DynamicCast<VideoData>(output);
  VideoData *src;
  VideoData *dst;
  Status res;
  Ipp64f timeStart, timeEnd;
  int k;

  UMC_CHECK(in, UMC_ERR_NULL_PTR);
  UMC_CHECK(out, UMC_ERR_NULL_PTR);

  // copy FrameType and Time
  out->SetFrameType(in->GetFrameType());
  in->GetTime(timeStart, timeEnd);
  out->SetTime(timeStart, timeEnd);

  // Crop
  if (bSrcCropArea) {
    *(VideoData*)tmp_in = *in;
    in = tmp_in;
    tmp_in->Crop(Param.SrcCropArea);
  }

  int src_w = in->GetWidth();
  int src_h = in->GetHeight();
  int dst_w = out->GetWidth();
  int dst_h = out->GetHeight();
  ColorFormat src_c = in->GetColorFormat();
  ColorFormat dst_c = out->GetColorFormat();

  memset(bFiltering, 0, sizeof(bFiltering));
  {
      bFiltering[iDeinterlacing] =
          (Param.m_DeinterlacingMethod != NO_DEINTERLACING) &&
          (in->GetPictureStructure() != PS_FRAME);
      bFiltering[iColorConv] = (src_c != dst_c);
      bFiltering[iResizing] = (src_w != dst_w || src_h != dst_h);
      UMC_CHECK(in->GetPlanePointer(0), UMC_ERR_NOT_ENOUGH_DATA);
  }

  // Get frame size
  Ipp32s lFrameSize = out->GetMappingSize();
  if (lFrameSize <= 0) lFrameSize = in->GetMappingSize();
  if (lFrameSize <= 0) lFrameSize = 1;

  // Set PictureStructure
  if (!bFiltering[iDeinterlacing]) {
    out->SetPictureStructure(in->GetPictureStructure());
  } else {
    out->SetPictureStructure(PS_FRAME);
  }

  // skip color conversion between YUV420 and YV12
  int skip_flag = 0;
  if (src_c == YUV420 && dst_c == YV12) skip_flag = 1; // YUV420->YV12
  if (src_c == YV12 && dst_c == YUV420) skip_flag = 2; // YV12->YUV420
  if (skip_flag) {
    for (k = 0; k < numFilters; k++) {
      if (bFiltering[k] && k != iColorConv) {
        if (skip_flag == 1) {
          *(VideoData*)tmp_out = *out;
          out = tmp_out;
          tmp_out->Convert_YV12_To_YUV420();
          dst_c = YUV420;
          bFiltering[iColorConv] = false;
        } else { // skip_flag == 2
          if (in != tmp_in) {
            *(VideoData*)tmp_in = *in;
            in = tmp_in;
          }
          tmp_in->Convert_YV12_To_YUV420();
          src_c = YUV420;
          bFiltering[iColorConv] = false;
        }
        break;
      }
    }
  }

  // Get last filter in chain
  int iLastFilter = numFilters - 1;
  while (iLastFilter >= 0 && bFiltering[iLastFilter] == false) iLastFilter--;
  if (iLastFilter < 0) { // if no filters
    bFiltering[iColorConv] = true;
    iLastFilter = iColorConv;
  }

  src = in;
  for (k = 0; k <= iLastFilter; k++) {
    if (!bFiltering[k] || pFilter[k] == NULL) continue;
    if (k == iLastFilter) {
      dst = out;
    } else {
      dst = &tmpData[k];
      if (k == iD3DProcessing || k == iColorConv) {
        src_c = dst_c;
      } else if (k == iResizing) {
        src_w = dst_w;
        src_h = dst_h;
      }
      InitVideoData(dst, src_w, src_h, src_c);
    }
    res = pFilter[k]->GetFrame(src, dst);
    if (res != UMC_OK) {
      if (k == iColorConv && res == UMC_ERR_NOT_IMPLEMENTED && !bFiltering[iColorConv0]) {
        bFiltering[iColorConv0] = true; // try double color conversion
        src_c = GetIntermediatedColor(src->GetColorFormat()); // intermediated color
        k = iColorConv0 - 1; // back to first color conversion
        continue;
      }
      return res;
    }
    src = dst;
  }

  input->SetDataSize(0);
  output->SetDataSize(lFrameSize);

  return UMC_OK;
}

Status VideoProcessing::SetParams(BaseCodecParams *params)
{
  VideoProcessingParams *p_params = DynamicCast<VideoProcessingParams>(params);
  UMC_CHECK(p_params, UMC_ERR_NULL_PTR);

  Param = *p_params;
  if (pFilter[iDeinterlacing]) {
    ((Deinterlacing*)pFilter[iDeinterlacing])->SetMethod(Param.m_DeinterlacingMethod);
  }
  if (pFilter[iResizing]) {
    ((VideoResizing*)pFilter[iResizing])->SetMethod(Param.InterpolationMethod);
  }
  bSrcCropArea = Param.SrcCropArea.left || Param.SrcCropArea.right ||
     Param.SrcCropArea.top || Param.SrcCropArea.bottom;

  return UMC_OK;
}

Status VideoProcessing::GetInfo(BaseCodecParams *info)
{
  VideoProcessingParams *p_params = DynamicCast<VideoProcessingParams>(info);
  if (!p_params) {
    return UMC_ERR_NULL_PTR;
  }
  *p_params = Param;
  return UMC_OK;
}

Status VideoProcessing::Reset()
{
  return UMC_OK;
}

VideoProcessing::~VideoProcessing()
{
  Close();
}

//////////////////////////////////////////////////////////////////////

Status VideoDataExt::Convert_YV12_To_YUV420()
{
  if (m_ColorFormat == YV12) {
    PlaneInfo tmp_plane = m_pPlaneData[1];
    m_pPlaneData[1] = m_pPlaneData[2];
    m_pPlaneData[2] = tmp_plane;
    m_ColorFormat = YUV420;
  }
  return UMC_OK;
}
