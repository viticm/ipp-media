/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2006-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "vm_debug.h"
#include "umc_muxer.h"

using namespace UMC;

// this line to turn off excessive output, comment it to get full debug output
//#define VM_DEBUG_PROGRESS 0

/******************************************************************************/

MuxerParams::MuxerParams()
{
  m_SystemType = UNDEF_STREAM;
  m_lFlags = 0;
  m_nNumberOfTracks = 0;
  pTrackParams = NULL;
  m_lpDataWriter = NULL;
  m_bAllocated = false;
}

MuxerParams::~MuxerParams()
{
  MuxerParams::Close();
}

Status MuxerParams::Close()
{
  Ipp32s i;
  if (!m_bAllocated) return UMC_OK;
  for (i = 0; i < m_nNumberOfTracks; i++) {
    if (pTrackParams[i].type == VIDEO_TRACK) {
      UMC_DELETE(pTrackParams[i].info.video);
    } else if (pTrackParams[i].type == AUDIO_TRACK) {
      UMC_DELETE(pTrackParams[i].info.audio);
    }
  }
  UMC_DELETE_ARR(pTrackParams);
  m_bAllocated = false;
  return UMC_OK;
}

Status MuxerParams::operator=(MuxerParams &p)
{
  Ipp32s i;

  MuxerParams::Close();

  m_SystemType = p.m_SystemType;
  m_lFlags = p.m_lFlags;
  m_nNumberOfTracks = p.m_nNumberOfTracks;
  m_lpDataWriter = p.m_lpDataWriter;

  pTrackParams = NULL;
  UMC_CHECK_PTR(p.pTrackParams);
  UMC_CHECK(m_nNumberOfTracks > 0, UMC_OK);

  UMC_NEW_ARR(pTrackParams, TrackParams, m_nNumberOfTracks);
  for (i = 0; i < m_nNumberOfTracks; i++) {
    pTrackParams[i] = p.pTrackParams[i];
    pTrackParams[i].info.undef = NULL;
    if (pTrackParams[i].type == VIDEO_TRACK) {
      UMC_NEW(pTrackParams[i].info.video, VideoStreamInfo);
      *pTrackParams[i].info.video = *p.pTrackParams[i].info.video;
    }
    else if (pTrackParams[i].type == AUDIO_TRACK) {
      UMC_NEW(pTrackParams[i].info.audio, AudioStreamInfo);
      *pTrackParams[i].info.audio = *p.pTrackParams[i].info.audio;
    }
  }
  m_bAllocated = true;
  return UMC_OK;
}

/******************************************************************************/

Muxer::Muxer()
{
  m_pParams = NULL;
  m_ppBuffers = NULL;
  m_uiTotalNumStreams = 0;
  m_pTrackParams = NULL;
}

Muxer::~Muxer()
{
  Muxer::Close();
}

Status Muxer::CopyMuxerParams(MuxerParams *lpInit)
{
  Ipp32u i;

  UMC_CHECK_PTR(lpInit);
  if (!m_pParams) {
    UMC_NEW(m_pParams, MuxerParams);
  }

  m_uiTotalNumStreams = lpInit->m_nNumberOfTracks;
  UMC_CHECK(m_uiTotalNumStreams >= 0, UMC_ERR_INVALID_PARAMS);

  *m_pParams = *lpInit; // via operator= !!!
  m_pTrackParams = m_pParams->pTrackParams; // copy of pointer, don't delete!

  // check MediaBufferParams
  for (i = 0; i < m_uiTotalNumStreams; i++) {
    UMC_CHECK_PTR(lpInit->pTrackParams[i].info.undef);
    if (!m_pTrackParams[i].bufferParams.m_prefInputBufferSize) {
      Ipp32s bitrate = 4000000;
      if (m_pTrackParams[i].type == VIDEO_TRACK) {
        bitrate = m_pTrackParams[i].info.video->bitrate;
        if (0 == bitrate) {
          bitrate = 4000000;
        }
      } else if (m_pTrackParams[i].type == AUDIO_TRACK) {
        bitrate = m_pTrackParams[i].info.audio->bitrate;
        if (0 == bitrate) {
          bitrate = 100000;
        }
      } else {
        return UMC_ERR_INVALID_PARAMS;
      }
      m_pTrackParams[i].bufferParams.m_prefInputBufferSize = bitrate >> 3; /* 1 sec in bytes */
    }
    if (!m_pTrackParams[i].bufferParams.m_prefOutputBufferSize) {
      m_pTrackParams[i].bufferParams.m_prefOutputBufferSize = m_pTrackParams[i].bufferParams.m_prefInputBufferSize;
    }
    if (!m_pTrackParams[i].bufferParams.m_numberOfFrames) {
      m_pTrackParams[i].bufferParams.m_numberOfFrames = 5;
    }
  }

  // Alloc pointers to MediaBuffer (and set to NULL)
  UMC_ALLOC_ZERO_ARR(m_ppBuffers, MediaBuffer*, m_uiTotalNumStreams);

  return UMC_OK;
}

Status Muxer::Close()
{
  if (m_ppBuffers) {
    Ipp32u i;
    for (i = 0; i < m_uiTotalNumStreams; i++) {
      UMC_DELETE(m_ppBuffers[i]);
    }
    UMC_FREE(m_ppBuffers);
  }

  UMC_DELETE(m_pParams);

  return UMC_OK;
}

Ipp32s Muxer::GetTrackIndex(MuxerTrackType type, Ipp32s index)
{
  Ipp32u i;
  for (i = 0; i < m_uiTotalNumStreams; i++) {
    if (m_pTrackParams[i].type == type) {
      if (index <= 0) {
        vm_debug_trace2(VM_DEBUG_PROGRESS, VM_STRING("GetTrackIndex: type = %d, index = %d\n"), (Ipp32s)type, i);
        return (Ipp32s)i;
      }
      index--;
    }
  }
  return -1;
}

Status Muxer::LockBuffer(MediaData *lpData, Ipp32s iTrack)
{
  UMC_CHECK_PTR(lpData);
  UMC_CHECK(iTrack >= 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK((Ipp32u)iTrack < m_uiTotalNumStreams, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(m_ppBuffers, UMC_ERR_NOT_INITIALIZED);

  UMC_CALL(m_ppBuffers[iTrack]->LockInputBuffer(lpData));
  return UMC_OK;
} //Status Muxer::LockBuffer(MediaData *lpData, Ipp32u iTrack)

Status Muxer::UnlockBuffer(MediaData *lpData, Ipp32s iTrack)
{
  UMC_CHECK_PTR(lpData);
  UMC_CHECK(iTrack >= 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK((Ipp32u)iTrack < m_uiTotalNumStreams, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(m_ppBuffers, UMC_ERR_NOT_INITIALIZED);

#ifdef VM_DEBUG
  Ipp64f pts, dts;
  lpData->GetTime(pts, dts);
  vm_debug_trace1(VM_DEBUG_PROGRESS, VM_STRING("Try to unlock buffer #%d with:"), iTrack);
  vm_debug_trace1(VM_DEBUG_PROGRESS, VM_STRING("    data_size = %d"), lpData->GetDataSize());
  vm_debug_trace2(VM_DEBUG_PROGRESS, VM_STRING("    data_time = (%.3f; %.3f)"), pts, dts);
#endif //VM_DEBUG

  UMC_CALL(m_ppBuffers[iTrack]->UnLockInputBuffer(lpData));
  return UMC_OK;
} //Status Muxer::UnlockBuffer(MediaData *lpData, Ipp32u iTrack)

Status Muxer::PutData(MediaData *lpData, Ipp32s iTrack)
{
  MediaData data;

  UMC_CALL(LockBuffer(&data, iTrack));

  // copy data
  UMC_CHECK(lpData->GetDataSize() <= data.GetBufferSize(), UMC_ERR_NOT_ENOUGH_BUFFER);
  ippsCopy_8u((Ipp8u*)lpData->GetDataPointer(), (Ipp8u*)data.GetDataPointer(), (Ipp32s)lpData->GetDataSize());

  // copy time & frame type
  Ipp64f dPTS, dDTS;
  lpData->GetTime(dPTS, dDTS);
  data.SetTime(dPTS, dDTS);
  data.SetDataSize(lpData->GetDataSize());
  data.SetFrameType(lpData->GetFrameType());

  UMC_CALL(UnlockBuffer(&data, iTrack));
  return UMC_OK;
} //Status Muxer::PutData(MediaData *lpData, Ipp32u iTrack)

Status Muxer::PutEndOfStream(Ipp32s iTrack)
{
  UMC_CHECK(m_ppBuffers, UMC_ERR_NOT_INITIALIZED);
  UMC_CHECK(iTrack >= 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK((Ipp32u)iTrack < m_uiTotalNumStreams, UMC_ERR_INVALID_PARAMS);

  UMC_CALL(m_ppBuffers[iTrack]->UnLockInputBuffer(NULL, UMC_ERR_END_OF_STREAM));
  return UMC_OK;
} //Status Muxer::PutEndOfStream(Ipp32u iTrack)

Status Muxer::PutVideoData(MediaData *lpData, Ipp32s index)
{
  UMC_CALL(PutData(lpData, GetTrackIndex(VIDEO_TRACK, index)));
  return UMC_OK;
} //Status Muxer::PutVideoData(MediaData *lpData)

Status Muxer::PutAudioData(MediaData *lpData, Ipp32s index)
{
  UMC_CALL(PutData(lpData, GetTrackIndex(AUDIO_TRACK, index)));
  return UMC_OK;
} //Status Muxer::PutAudioData(MediaData *lpData)

Status Muxer::GetStreamToWrite(Ipp32s &rStreamNumber, bool bFlushMode)
{
  static const Ipp64f MAXIMUM_DOUBLE = 1.7E+308;

  Status umcRes;
  Ipp32u streamNum, minNum = 0;
  Ipp64f streamTime, minTime = MAXIMUM_DOUBLE;

  for (streamNum = 0; streamNum < (Ipp32u)m_uiTotalNumStreams; streamNum++)
  {
    umcRes = GetOutputTime(streamNum, streamTime);
    if (UMC_ERR_NOT_ENOUGH_DATA == umcRes && !bFlushMode)
    {
      vm_debug_trace1(VM_DEBUG_PROGRESS, VM_STRING("Ordering of elementary streams... stream #%d is empty"), streamNum);
      return umcRes;
    }

    if (UMC_OK == umcRes)
    {
      if (streamTime < minTime)
      {
        minNum = streamNum;
        minTime = streamTime;
      }
    }
  }

  // no more data in buffers
  if (minTime >= MAXIMUM_DOUBLE)
  {
    vm_debug_trace(VM_DEBUG_INFO, VM_STRING("END_OF_STREAM. All data was written"));
    return UMC_ERR_END_OF_STREAM;
  }

  rStreamNumber = minNum;
  vm_debug_trace2(VM_DEBUG_PROGRESS, VM_STRING("Ordering of elementary streams... stream #%d has min time (%.4f sec)"), minNum, minTime);
  return UMC_OK;
} //Status Muxer::GetStreamToWrite(Ipp32s &rStreamNumber, bool bFlushMode)
