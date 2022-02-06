/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_config.h"
#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)
#include "vm_debug.h"
#include "vm_time.h"
#include "umc_h263_video_encoder.h"
#include "umc_video_data.h"

namespace UMC
{

H263VideoEncoder::H263VideoEncoder()
{
    m_IsInit = false;
}

H263VideoEncoder::~H263VideoEncoder()
{
    Close();
}

Status H263VideoEncoder::AllocateBuffers()
{
  Status status = UMC_OK;
  int    i;

  // allocate only frame memory, may be extended for whole buffers
  for (i = 0; i < h263enc.mPlanes; i ++)
    h263enc.mFrame[i].mid = 0;
  for (i = 0; i < h263enc.mPlanes; i ++) {
    status = m_pMemoryAllocator->Alloc(&h263enc.mFrame[i].mid, h263enc.mLumaPlaneSize + h263enc.mChromaPlaneSize + h263enc.mChromaPlaneSize, UMC_ALLOC_PERSISTENT);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

Status H263VideoEncoder::FreeBuffers()
{
  Status status = UMC_OK;
  int    i;

  for (i = 0; i < h263enc.mPlanes; i ++) {
    if (h263enc.mFrame[i].mid)
      status = m_pMemoryAllocator->Free(h263enc.mFrame[i].mid);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

void H263VideoEncoder::LockBuffers()
{
  int    i;

  for (i = 0; i < h263enc.mPlanes; i ++) {
    h263enc.mFrame[i].ap = (Ipp8u*)m_pMemoryAllocator->Lock(h263enc.mFrame[i].mid);
    h263enc.mFrame[i].pY = h263enc.mFrame[i].ap + h263enc.mExpandSizeA + h263enc.mExpandSizeA * h263enc.mStepLuma;
    h263enc.mFrame[i].pU = h263enc.mFrame[i].ap + h263enc.mLumaPlaneSize + (h263enc.mExpandSizeA >> 1) + (h263enc.mExpandSizeA >> 1) * h263enc.mStepChroma;
    h263enc.mFrame[i].pV = h263enc.mFrame[i].ap + h263enc.mLumaPlaneSize + h263enc.mChromaPlaneSize + (h263enc.mExpandSizeA >> 1) + (h263enc.mExpandSizeA >> 1) * h263enc.mStepChroma;
  }
}

Status H263VideoEncoder::UnlockBuffers()
{
  Status status = UMC_OK;
  int    i;

  for (i = 0; i < h263enc.mPlanes; i ++) {
    status = m_pMemoryAllocator->Unlock(h263enc.mFrame[i].mid);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

Status H263VideoEncoder::Reset()
{
  h263enc.mLastIPic = -h263enc.mIPicdist;
  return UMC_OK;

  //  return UMC_ERR_NOT_IMPLEMENTED;
}

Status H263VideoEncoder::SetParams(BaseCodecParams* params)
{
    return UMC_ERR_NOT_IMPLEMENTED;
}

H263EncoderParams::H263EncoderParams()
{
// default values - required when run w/out par file
  m_Param.advIntra = m_Param.advPred = m_Param.deblockFilt = 0;
  m_Param.modQuant = 0;
  m_Param.UMV = 0;
  m_Param.calcPSNR = 0;
  m_Param.MEaccuracy = 2;
  m_Param.MEalgorithm = 1;
  m_Param.IPicdist = 10;
  m_Param.PPicdist = 1;
  m_Param.PPicsearchHeight = 15;
  m_Param.PPicsearchWidth = 15;
  m_Param.RateControl = 0;
  m_Param.SceneChangeThreshold = 50;
  m_Param.BitRate = 400000;
  m_Param.FrameSkip = 1;
  m_Param.quantPPic = m_Param.quantIPic = 7;
//  m_Param.quantBPic = 9;
  m_Param.TimeIncrement = 1001;
  m_Param.TimeResolution = 30000;
  m_Param.bsBuffer = (Ipp8u*)1;
  m_Param.bsBuffSize = 1; // encoder will not allocate buffer
  m_Param.GOBheaders = 0;
  m_Param.PAR_width = 12;
  m_Param.PAR_height = 11;

  // just in case
  m_Param.Height = 288;
  m_Param.Width = 352;
}

Status H263VideoEncoder::Init(BaseCodecParams* init)
{
  VideoEncoderParams *VideoParams = DynamicCast<VideoEncoderParams>(init);
  H263EncoderParams *pParam = DynamicCast<H263EncoderParams>(init);
  h263e_Param *h263Params;

  if (pParam) {
    m_Param = *pParam;
  }

  if (VideoParams) {
    h263Params = &m_Param.m_Param;
    h263Params->Width = VideoParams->m_info.videoInfo.m_iWidth;
    h263Params->Height = VideoParams->m_info.videoInfo.m_iHeight;
    if (VideoParams->m_info.iBitrate > 0) {
      h263Params->RateControl = 1;
      h263Params->BitRate = VideoParams->m_info.iBitrate;
    } else
      h263Params->BitRate = 0;

    {
      Ipp64f finc;
      Ipp32s fr_delta = (Ipp32s)(VideoParams->m_info.fFramerate * h263Params->TimeIncrement) - h263Params->TimeResolution;
      Ipp32s tinc, tres, div0, div1, rem0, rem1, q;
      fr_delta = h263e_Abs(fr_delta);
      if (fr_delta > 500) {
        tres = 1800000;
        finc = (Ipp64f)tres / VideoParams->m_info.fFramerate;
        tinc = (Ipp32s)finc;
        div1 = (Ipp32s)(finc * (1.0 / 1001) + 0.5);
        div0 = (Ipp32s)(finc * 0.001 + 0.5);
        rem0 = div0 * 1000 - tinc;
        rem1 = div1 * 1001 - tinc;
        if (h263e_Abs(rem0) < h263e_Abs(rem1))
          tinc = div0 * 1000;
        else {
          q = div1 / 60;
          if (q * 60 == div1 && q <= 255) {
            div1 = q;
            tres = 30000;
          }
          tinc = div1 * 1001;
        }
        h263Params->TimeResolution = tres;
        h263Params->TimeIncrement = tinc;
      }
    }

//    while ((Ipp32s)((Ipp64f)h263Params->TimeResolution / VideoParams->m_info.framerate) > h263Params->TimeIncrement + 1001/2) {
//      h263Params->TimeIncrement += 1001;
//    }
    //h263Params->NumOfFrames = VideoParams->numFramesToEncode;

    m_Param.m_info.videoInfo.m_iWidth = h263Params->Width;
    m_Param.m_info.videoInfo.m_iHeight = h263Params->Height;
    m_Param.m_info.iBitrate = h263Params->BitRate;
    m_Param.m_info.fFramerate = (Ipp64f)h263Params->TimeResolution / h263Params->TimeIncrement;

    //    h263Params->numThreads     = VideoParams->numThreads;
  } else
    return UMC_ERR_NULL_PTR;

  if (m_IsInit)
    Close();

  Ipp32s  h263status = h263enc.Init(h263Params);
  if (h263status == H263_STS_ERR_PARAM)
    return UMC_ERR_INVALID_PARAMS;
  if (h263status == H263_STS_ERR_NOMEM)
    return UMC_ERR_ALLOC;
  m_FrameCount = 0;

  if (VideoParams->m_info.videoInfo.m_colorFormat != YUV420) {
    //vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("Invalid color format: only YUV420 supported\n"));
    return UMC_ERR_INVALID_PARAMS;
  }
  // create default memory allocator if not exist
  Status status = BaseCodec::Init(init);
  if (status != UMC_OK)
    return status;
  status = AllocateBuffers();
  if (status != UMC_OK)
    return UMC_ERR_ALLOC;

  m_IsInit = true;
  return UMC_OK;
}

Status H263VideoEncoder::GetInfo(BaseCodecParams* info)
{
  H263EncoderParams* pParam = DynamicCast<H263EncoderParams>(info);
  VideoEncoderParams *VideoParams = DynamicCast<VideoEncoderParams>(info);

  if (!m_IsInit)
      return UMC_ERR_NOT_INITIALIZED;
  if (pParam) {
    *pParam = m_Param;
    VideoParams = pParam;
  } else if (!VideoParams)
    return UMC_ERR_NULL_PTR;

  /*
  VideoParams->m_info.clip_info.width = m_Param.m_Param.Width;
  VideoParams->m_info.clip_info.height = m_Param.m_Param.Height;
  VideoParams->m_info.framerate = (Ipp64f)m_Param.m_Param.TimeResolution / m_Param.m_Param.TimeIncrement;

  if (m_Param.m_Param.RateControl)
    VideoParams->m_info.iBitrate = m_Param.m_Param.iBitrate;
  else
    VideoParams->m_info.iBitrate = 0;
  //VideoParams->numFramesToEncode = m_Param.m_Param.NumOfFrames;
  VideoParams->numEncodedFrames = m_FrameCount;

  // only 4:3 display aspect ratio is currently supported
  VideoParams->m_info.aspect_ratio_width = 4;
  VideoParams->m_info.aspect_ratio_height = 3;
  VideoParams->m_info.color_format = YUV420;
  VideoParams->m_info.stream_type = H263_VIDEO;
  */

  VideoParams->m_info.videoInfo.m_iWidth = m_Param.m_Param.Width;
  VideoParams->m_info.videoInfo.m_iHeight = m_Param.m_Param.Height;
  VideoParams->m_info.fFramerate = (Ipp64f)m_Param.m_Param.TimeResolution / m_Param.m_Param.TimeIncrement;
  if (m_Param.m_Param.RateControl)
    VideoParams->m_info.iBitrate = m_Param.m_Param.BitRate;
  else
    VideoParams->m_info.iBitrate = 0;
  //VideoParams->numFramesToEncode = m_Param.m_Param.NumOfFrames;
  //VideoParams->numEncodedFrames = h261enc.mFrameCount;
  VideoParams->m_iFramesCounter = h263enc.mFrameCount;


  VideoParams->m_info.videoInfo.m_iWidth = 4;
  VideoParams->m_info.videoInfo.m_iWidth = 3;
  VideoParams->m_info.videoInfo.m_colorFormat = YUV420;
  VideoParams->m_info.streamType = H263_VIDEO;

  return UMC_OK;
}

Status H263VideoEncoder::Close()
{
    if (!m_IsInit)
      return UMC_ERR_NOT_INITIALIZED;
    FreeBuffers();
    // close default memory allocator if exist
    BaseCodec::Close();
    h263enc.Close();
    m_IsInit = false;
    return UMC_OK;
}

Status H263VideoEncoder::GetFrame(MediaData* pIn, MediaData* pOut)
{
  if (!m_IsInit)
    return UMC_ERR_NOT_INITIALIZED;
  VideoData* pVideoDataIn = DynamicCast<VideoData> (pIn);
  if (!pOut)
    return UMC_ERR_NULL_PTR;
  h263enc.InitBuffer((Ipp8u*)pOut->GetDataPointer() + pOut->GetDataSize(),
                      (Ipp8u*)pOut->GetBufferPointer() - (Ipp8u*)pOut->GetDataPointer() + pOut->GetBufferSize() - pOut->GetDataSize());

  LockBuffers();
  if (pVideoDataIn) {
    // copy YUV to internal frame
    IppiSize  roi;
    Ipp8u    *pY, *pU, *pV;
    Ipp32s    stepL, stepC;

    h263enc.GetCurrentFrameInfo(&pY, &pU, &pV, &stepL, &stepC);
    roi.width = m_Param.m_info.videoInfo.m_iWidth;
    roi.height = m_Param.m_info.videoInfo.m_iHeight;
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlaneDataPtr(0), pVideoDataIn->GetPlanePitch(0), pY, stepL, roi);
    roi.width >>= 1;
    roi.height >>= 1;
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlaneDataPtr(1), pVideoDataIn->GetPlanePitch(1), pU, stepC, roi);
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlaneDataPtr(2), pVideoDataIn->GetPlanePitch(2), pV, stepC, roi);
  }
  Ipp32s  sts = h263enc.EncodeFrame(pIn == NULL);

  if (sts == H263_STS_ERR_BUFOVER) {
    UnlockBuffers();
    return UMC_ERR_NOT_ENOUGH_BUFFER;
  }
  pOut->SetDataSize(h263enc.GetBufferFullness() + pOut->GetDataSize());
  if (pIn) {
    pIn->SetDataSize(0);
    //pOut->SetTime(pIn->GetTime());
  }
  if (sts != H263_STS_NODATA)
    m_FrameCount++;
  UnlockBuffers();
  //return (sts == H263_STS_BUFFERED) ? UMC_ERR_NOT_ENOUGH_DATA : (sts == H263_STS_NODATA) ? UMC_ERR_END_OF_STREAM : UMC_OK;
  return (sts == H263_STS_SKIPPED_FRAME || sts == H263_STS_NODATA) ? UMC_ERR_NOT_ENOUGH_DATA : UMC_OK;
}

VideoEncoder* createH263VideoEncoder()
{
    H263VideoEncoder* ptr = new H263VideoEncoder;
    return ptr;
}

Status H263EncoderParams::ReadParamFile(const vm_char *FileName)
{
    vm_file *InputFile;
    vm_char str[STR_LEN+1];
    Ipp32s rem0, rem1, quot1;
    Ipp64f fdiv1 = 1.0/1001, fquot1;

    InputFile = vm_file_open(FileName, VM_STRING("rt"));
    if (!InputFile) {
        //vm_debug_trace1(VM_DEBUG_INFO,VM_STRING("Error: Couldn't open file '%s'\n"), FileName);
        return UMC_ERR_FAILED;
    }
    vm_file_gets(str, STR_LEN, InputFile);
    vm_file_gets(str, STR_LEN, InputFile); //if (SrcFileName) vm_string_sscanf(str, VM_STRING("%s"), SrcFileName);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Width);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Height);
    vm_file_gets(str, STR_LEN, InputFile); //vm_string_sscanf(str, VM_STRING("%d"), &m_Param.NumOfFrames);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.TimeResolution);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.TimeIncrement);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantIPic);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantPPic);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantBPic);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.IPicdist);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PPicdist);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PPicsearchWidth);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PPicsearchHeight);
/*
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BPicsearchWidthForw);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BPicsearchHeightForw);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BPicsearchWidthBack);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BPicsearchHeightBack);
*/
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.MEalgorithm);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.MEaccuracy);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.calcPSNR);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.RateControl);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BitRate);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.SceneChangeThreshold);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.UMV);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.advPred);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.advIntra);
    if (vm_file_gets(str, STR_LEN, InputFile))
      vm_string_sscanf(str, VM_STRING("%d"), &m_Param.GOBheaders);
    if (vm_file_gets(str, STR_LEN, InputFile))
      vm_string_sscanf(str, VM_STRING("%d"), &m_Param.modQuant);
    if (vm_file_gets(str, STR_LEN, InputFile))
      vm_string_sscanf(str, VM_STRING("%d"), &m_Param.FrameSkip);
    if (vm_file_gets(str, STR_LEN, InputFile))
      vm_string_sscanf(str, VM_STRING("%d:%d"), &m_Param.PAR_width, &m_Param.PAR_height);

//    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.deblockFilt);
//    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.data_partitioned);
    vm_file_fclose(InputFile);

    m_Param.bsBuffer = (Ipp8u*)1;
    m_Param.bsBuffSize = 1; // encoder will not allocate buffer

    m_info.videoInfo.m_iWidth = m_Param.Width;
    m_info.videoInfo.m_iHeight = m_Param.Height;

    fquot1 = m_Param.TimeIncrement * fdiv1;
    quot1 = (Ipp32s)(fquot1 + 0.5);
    rem1 = m_Param.TimeIncrement - quot1 * 1001;

    if ((rem1 || quot1 > 255) && m_Param.TimeResolution == 30000) {
      m_Param.TimeResolution = 1800000;
      m_Param.TimeIncrement *= 60;
    }
    if (m_Param.TimeResolution != 30000 && m_Param.TimeResolution != 1800000) {
      m_Param.TimeIncrement = (Ipp32s)((Ipp64f)m_Param.TimeIncrement * 1800000 / m_Param.TimeResolution + 0.5);
      rem1 = m_Param.TimeIncrement - (Ipp32s)(m_Param.TimeIncrement * fdiv1 + 0.5) * 1001;
      m_Param.TimeResolution = 1800000;
    }

    rem0 = m_Param.TimeIncrement % 1000;
    if (rem1 && rem0) {
      if (rem1 > 500)
        rem1 = rem1 - 1001;
      if (rem0 > 500)
        rem0 = rem0 - 1000;

      if (h263e_Abs(rem0) < h263e_Abs(rem1)) {
        m_Param.TimeIncrement = m_Param.TimeIncrement - rem0;
      } else {
        m_Param.TimeIncrement = m_Param.TimeIncrement - rem1;
        rem1 = 0;
      }
    } else if (!rem1 && m_Param.TimeResolution == 1800000) {
      Ipp32s q = m_Param.TimeIncrement / 60;
      if (q * 60 == m_Param.TimeIncrement && q <= 255) {
        m_Param.TimeIncrement = q;
        m_Param.TimeResolution = 30000;
      }
    }

    quot1 = (rem1 ? 1000 : 1001);
    if (m_Param.TimeIncrement > 1023 * 127 * quot1)
      m_Param.TimeIncrement = 1023 * 127 * quot1;
    m_info.fFramerate = (Ipp64f)m_Param.TimeResolution / m_Param.TimeIncrement;
    if (m_Param.RateControl)
      m_info.iBitrate = m_Param.BitRate;
    else
      m_info.iBitrate = 0;
    //numFramesToEncode = m_Param.NumOfFrames;

    return UMC_OK;
}

}; //namespace UMC
#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
