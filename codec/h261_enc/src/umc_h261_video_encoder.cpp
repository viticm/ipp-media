/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)

#include "vm_debug.h"
#include "vm_time.h"
#include "umc_h261_video_encoder.h"
#include "umc_video_data.h"

namespace UMC {

H261VideoEncoder::H261VideoEncoder()
{
    m_IsInit = false;
}


H261VideoEncoder::~H261VideoEncoder()
{
    Close();
}

Status H261VideoEncoder::AllocateBuffers()
{
  Status status = UMC_OK;
  Ipp32s i;
  // allocate only frame memory, may be extended for whole buffers

  for (i = 0; i < h261enc.mPlanes; i++) {
    status = m_pMemoryAllocator->Alloc(&h261enc.mFrame[i].mid, h261enc.mLumaPlaneSize + h261enc.mChromaPlaneSize + h261enc.mChromaPlaneSize, UMC_ALLOC_PERSISTENT);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

Status H261VideoEncoder::FreeBuffers()
{
  Status status = UMC_OK;
  Ipp32s i;

  for (i = 0; i < h261enc.mPlanes; i ++) {
    if (h261enc.mFrame[i].mid)
      status = m_pMemoryAllocator->Free(h261enc.mFrame[i].mid);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

void H261VideoEncoder::LockBuffers()
{
  Ipp32s i;

  for (i = 0; i < h261enc.mPlanes; i ++) {
    h261enc.mFrame[i].pY = (Ipp8u*)m_pMemoryAllocator->Lock(h261enc.mFrame[i].mid);
    h261enc.mFrame[i].pU = h261enc.mFrame[i].pY + h261enc.mLumaPlaneSize;
    h261enc.mFrame[i].pV = h261enc.mFrame[i].pY + h261enc.mLumaPlaneSize + h261enc.mChromaPlaneSize;
  }
}

Status H261VideoEncoder::UnlockBuffers()
{
  Status status = UMC_OK;
  int    i;

  for (i = 0; i < h261enc.mPlanes; i ++) {
    status = m_pMemoryAllocator->Unlock(h261enc.mFrame[i].mid);
    if (status != UMC_OK)
      return status;
  }
  return status;
}

Status H261VideoEncoder::Reset()
{
    return UMC_ERR_NOT_IMPLEMENTED;
}

Status H261VideoEncoder::SetParams(BaseCodecParams* params)
{
    return UMC_ERR_NOT_IMPLEMENTED;
}

H261EncoderParams::H261EncoderParams()
{
  m_Param.calcPSNR = 0;
  m_Param.MEalgorithm = 1;
  m_Param.IFramedist = 50;
  m_Param.PFramesearchHeight = 15;
  m_Param.PFramesearchWidth = 15;
  m_Param.quantIFrame = 7;
  m_Param.quantPFrame = 7;
  m_Param.frameInterval = 1;
  m_Param.RateControl = 0;
  m_Param.BitRate = 0;
  m_Param.bPP = 0;       // default H261_MAX_FRAME_SIZE (/4 for QCIF)
  m_Param.FrameSkip = 1; // allowed by default

  m_Param.bsBuffer = (Ipp8u*)1;
  m_Param.bsBuffSize = 1; // encoder will not allocate buffer

  // just in case
  m_Param.Height = 288;
  m_Param.Width = 352;
}

Status H261VideoEncoder::Init(BaseCodecParams* init)
{
  VideoEncoderParams *VideoParams = DynamicCast<VideoEncoderParams>(init);
  H261EncoderParams *pParam = DynamicCast<H261EncoderParams>(init);
  h261_Param *h261Params;

  if (pParam) {
    m_Param = *pParam;
  }
  if (VideoParams) {
    h261Params = &m_Param.m_Param;
    h261Params->Width = VideoParams->info.clip_info.width;
    h261Params->Height = VideoParams->info.clip_info.height;
    if (VideoParams->info.bitrate > 0) {
      h261Params->RateControl = 1;
      h261Params->BitRate = VideoParams->info.bitrate;
    }

    while ((Ipp32s)((Ipp64f)30000 / VideoParams->info.framerate) > h261Params->frameInterval*1001 + 1001/2) {
      h261Params->frameInterval++;
    }
    //h261Params->NumOfFrames = VideoParams->numFramesToEncode;

    m_Param.info.bitrate = h261Params->BitRate;
    m_Param.info.framerate = ((Ipp64f)30000 / 1001) / h261Params->frameInterval;
    m_Param.info.clip_info.width = h261Params->Width;
    m_Param.info.clip_info.height = h261Params->Height;
    //    h261Params->numThreads     = VideoParams->numThreads;
  } else
    return UMC_ERR_NULL_PTR;

  if (m_IsInit)
    Close();

  Ipp32s h261status = h261enc.Init(h261Params);
  if (h261status == H261_STS_ERR_PARAM)
    return UMC_ERR_INVALID_PARAMS;
  if (h261status == H261_STS_ERR_NOMEM)
    return UMC_ERR_ALLOC;
//  m_FrameCount = 0;

  if (VideoParams->info.color_format != YUV420) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("Invalid color format: only YUV420 supported\n"));
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

Status H261VideoEncoder::GetInfo(BaseCodecParams* info)
{
  H261EncoderParams* pParam = DynamicCast<H261EncoderParams>(info);
  VideoEncoderParams *VideoParams = DynamicCast<VideoEncoderParams>(info);

  if (!m_IsInit)
      return UMC_ERR_NOT_INITIALIZED;
  if (pParam) {
    *pParam = m_Param;
    VideoParams = pParam;
  } else if (!VideoParams)
    return UMC_ERR_NULL_PTR;

  VideoParams->info.clip_info.width = m_Param.m_Param.Width;
  VideoParams->info.clip_info.height = m_Param.m_Param.Height;
  VideoParams->info.framerate = ((Ipp64f)30000 / 1001) /  m_Param.m_Param.frameInterval;
  if (m_Param.m_Param.RateControl)
    VideoParams->info.bitrate = m_Param.m_Param.BitRate;
  else
    VideoParams->info.bitrate = 0;
  //VideoParams->numFramesToEncode = m_Param.m_Param.NumOfFrames;
  VideoParams->numEncodedFrames = h261enc.mFrameCount;

  VideoParams->info.aspect_ratio_width = 4;
  VideoParams->info.aspect_ratio_height = 3;
  VideoParams->info.color_format = YUV420;
  VideoParams->info.stream_type = H261_VIDEO;

  return UMC_OK;
}

Status H261VideoEncoder::Close()
{
  if (!m_IsInit)
    return UMC_ERR_NOT_INITIALIZED;
  FreeBuffers();
  // close default memory allocator if exist
  BaseCodec::Close();
  h261enc.Close();
  m_IsInit = false;
  return UMC_OK;
}

Status H261VideoEncoder::GetFrame(MediaData* pIn, MediaData* pOut)
{
  Ipp32s sts = H261_STS_NOERR;
  if (!m_IsInit)
    return UMC_ERR_NOT_INITIALIZED;
  VideoData* pVideoDataIn = DynamicCast<VideoData> (pIn);
  if (!pOut)
    return UMC_ERR_NULL_PTR;
  if (pIn) {
    Ipp8u    *pY, *pU, *pV;
    Ipp32s    stepL, stepC;
    IppiSize roi;
    LockBuffers();
    // init bitstream buffer
    h261enc.cBS.mBitOff = 0;
    h261enc.cBS.mBuffer = (Ipp8u*)pOut->GetDataPointer() + pOut->GetDataSize();
    h261enc.cBS.mBuffSize = (Ipp8u*)pOut->GetBufferPointer() - (Ipp8u*)pOut->GetDataPointer() +
                              pOut->GetBufferSize() - pOut->GetDataSize();
    // max bitrate = 30*64 kbps, framerate = 29.97
    h261enc.cBS.mPtr = h261enc.cBS.mBuffer;
    // copy YUV to internal frame
    h261enc.GetCurrentFrameInfo(&pY, &pU, &pV, &stepL, &stepC);
    roi.width = h261enc.mSourceWidth;
    roi.height = h261enc.mSourceHeight;
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(0), pVideoDataIn->GetPlanePitch(0), pY, stepL, roi);
    roi.width >>= 1;
    roi.height >>= 1;
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(1), pVideoDataIn->GetPlanePitch(1), pU, stepC, roi);
    ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(2), pVideoDataIn->GetPlanePitch(2), pV, stepC, roi);
    sts = h261enc.EncodeFrame();
    if (sts == H261_STS_ERR_BUFOVER) {
      UnlockBuffers();
      return UMC_ERR_NOT_ENOUGH_BUFFER;
    }
    pOut->SetDataSize(h261enc.cBS.mPtr - h261enc.cBS.mBuffer + pOut->GetDataSize());
    pOut->SetTime(pIn->GetTime());
    pIn->SetDataSize(0);
    UnlockBuffers();
  } else {
    //pOut->SetDataSize(0);
    //pOut->SetTime(-1.0);
    return UMC_ERR_NOT_ENOUGH_DATA;
  }
  return (sts == H261_STS_SKIPPED_FRAME ? UMC_ERR_NOT_ENOUGH_DATA : UMC_OK);
}

VideoEncoder* createH261VideoEncoder()
{
  H261VideoEncoder* ptr = new H261VideoEncoder;
  return ptr;
}

Status H261EncoderParams::ReadParamFile(const vm_char *FileName)
{
  vm_file *InputFile;
  vm_char str[STR_LEN+1];

  InputFile = vm_file_open(FileName, VM_STRING("rt"));
  if (!InputFile) {
    vm_debug_trace1(VM_DEBUG_INFO,__VM_STRING("Error: Couldn't open file '%s'\n"), FileName);
    return UMC_ERR_FAILED;
  }
  vm_file_gets(str, STR_LEN, InputFile);
  vm_file_gets(str, STR_LEN, InputFile); //if (SrcFileName) vm_string_sscanf(str, VM_STRING("%s"), SrcFileName);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Width);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Height);
  vm_file_gets(str, STR_LEN, InputFile); //vm_string_sscanf(str, VM_STRING("%d"), &m_Param.NumOfFrames);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantIFrame);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantPFrame);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.IFramedist);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PFramesearchWidth);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PFramesearchHeight);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.MEalgorithm);
  vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.calcPSNR);
  if (vm_file_gets(str, STR_LEN, InputFile))
    vm_string_sscanf(str, VM_STRING("%d"), &m_Param.frameInterval);
  else
    m_Param.frameInterval = 1;
  if (vm_file_gets(str, STR_LEN, InputFile))
    vm_string_sscanf(str, VM_STRING("%d"), &m_Param.RateControl);
  else
    m_Param.RateControl = 0;
  if (vm_file_gets(str, STR_LEN, InputFile))
    vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BitRate);
  else
    m_Param.BitRate = 0;
  if (vm_file_gets(str, STR_LEN, InputFile))
    vm_string_sscanf(str, VM_STRING("%d"), &m_Param.FrameSkip);
  else
    m_Param.FrameSkip = 1;
  if (vm_file_gets(str, STR_LEN, InputFile))
    vm_string_sscanf(str, VM_STRING("%d"), &m_Param.bPP);
  else
    m_Param.bPP = 0;

  vm_file_fclose(InputFile);
  m_Param.bsBuffer = (Ipp8u*)1;
  m_Param.bsBuffSize = 1; // encoder will not allocate buffer

  info.clip_info.width = m_Param.Width;
  info.clip_info.height = m_Param.Height;
  info.framerate = (((Ipp64f)30000)/1001)/m_Param.frameInterval;
  if (m_Param.RateControl)
    info.bitrate = m_Param.BitRate;
  else
    info.bitrate = 0;
  //numFramesToEncode = m_Param.NumOfFrames;

  return UMC_OK;
}

}; //namespace UMC

#endif // defined (UMC_ENABLE_H261_VIDEO_ENCODER)
