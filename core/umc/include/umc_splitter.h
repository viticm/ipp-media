/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//        Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_SPLITTER_H__
#define __UMC_SPLITTER_H__

#include "umc_structures.h"
#include "umc_data_reader.h"
#include "umc_dynamic_cast.h"
#include "umc_memory_allocator.h"
#include "umc_media_data.h"

namespace UMC
{

class SplitterParams
{
    DYNAMIC_CAST_DECL_BASE(SplitterParams)

public:
    // Default constructor
    SplitterParams();
    // Destructor
    virtual ~SplitterParams();

    Ipp32u m_lFlags;                                       // (Ipp32u) splitter's flags
    DataReader *m_pDataReader;                             // (DataReader *) pointer to data reader
    Ipp32u m_uiSelectedVideoPID;                           // ID for video stream chosen by user
    Ipp32u m_uiSelectedAudioPID;                           // ID for audio stream chosen by user
    MemoryAllocator *m_pMemoryAllocator;                   // (MemoryAllocator *) pointer to memory allocator object
};

enum TrackType
{
    /* video types 0x0000XXXX */
    TRACK_MPEG1V                = 0x00000001,
    TRACK_MPEG2V                = 0x00000002,
    TRACK_MPEG4V                = 0x00000004,
    TRACK_H261                  = 0x00000008,
    TRACK_H263                  = 0x00000010,
    TRACK_H264                  = 0x00000020,
    TRACK_DVSD                  = 0x00000040,
    TRACK_DV50                  = 0x00000080,
    TRACK_DVHD                  = 0x00000100,
    TRACK_DVSL                  = 0x00000200,
    TRACK_VC1                   = 0x00000400,
    TRACK_WMV                   = 0x00000800,
    TRACK_MJPEG                 = 0x00001000,
    TRACK_YUV                   = 0x00002000,
    TRACK_AVS                   = 0x00004000,
    TRACK_ANY_VIDEO             = 0x0000FFFF,

    /* audio typTRACK 0x0XXX0000 */
    TRACK_PCM                   = 0x00010000,
    TRACK_LPCM                  = 0x00020000,
    TRACK_AC3                   = 0x00040000,
    TRACK_AAC                   = 0x00080000,
    TRACK_MPEGA                 = 0x00100000,
    TRACK_TWINVQ                = 0x00200000,
    TRACK_DTS                   = 0x00400000,
    TRACK_VORBIS                = 0x00800000,
    TRACK_AMR                   = 0x01000000,
    TRACK_ANY_AUDIO             = 0x0FFF0000,

    TRACK_SUB_PIC               = 0x10000000,
    TRACK_DVD_NAV               = 0x20000000,
    TRACK_ANY_DVD               = 0x30000000,

    TRACK_VBI_TXT               = 0x40000000,
    TRACK_VBI_SPEC              = 0x80000000,
    TRACK_ANY_VBI               = 0xC0000000,

    TRACK_ANY_SPECIAL           = 0xF0000000,

    TRACK_UNKNOWN               = 0x00000000
};

struct TrackInfo
{
    DYNAMIC_CAST_DECL_BASE(TrackInfo)

    TrackInfo()
    {
        m_Type = TRACK_UNKNOWN;
        m_PID = 0;
        m_isSelected = 0;
        m_pDecSpecInfo = NULL;
        m_pStreamInfo = NULL;
    }

    TrackType     m_Type;                 // common type (all audio/video/other in one enum)
    Ipp32u        m_PID;                  //
    Ipp32s        m_isSelected;           // if Track is on or off
    MediaData    *m_pDecSpecInfo;         // Keeps DecSpecInfo and its length
    StreamInfo   *m_pStreamInfo;          // Base for AudioStreamInfo, VideoStreamInfo, etc
};

class SplitterInfo
{
    DYNAMIC_CAST_DECL_BASE(SplitterInfo)

public:
    // Default constructor
    SplitterInfo();
    // Destructor
    virtual ~SplitterInfo();

    // common fields
    Ipp32u              m_splitter_flags;
    SystemStreamType    m_SystemType;       // system type (MPEG4, MPEG2, AVI, pure)
    Ipp32u              m_nOfTracks;        // number of tracks detected
    Ipp64f              m_dRate;            // current playback rate
    Ipp64f              m_dDuration;        // duration of stream
    TrackInfo         **m_ppTrackInfo;      // array of pointers to TrackInfo(s)
};

/*
//  Class:       Splitter
//
//  Notes:       Base abstract class of splitter. Class describes
//               the high level interface of abstract splitter of media stream.
//               All specific ( avi, mpeg2, mpeg4 etc ) must be implemented in
//               derevied classes.
//               Splitter uses this class to obtain data
//
*/
class Splitter
{
    DYNAMIC_CAST_DECL_BASE(Splitter)

public:
    // constructor
    Splitter();

    // decstructor
    virtual ~Splitter() {}

    // Get media data type
    static SystemStreamType GetStreamType(DataReader *dr);

    // Initialize splitter
    virtual Status Init(SplitterParams& rInit) = 0;

    // Close splitter and free all resources
    virtual Status Close() = 0;

    // Get next data, unlocks previously returned
    virtual Status GetNextData(MediaData* /*data*/, Ipp32u /*nTrack*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Get next data without moving DataReader
    virtual Status CheckNextData(MediaData* /*data*/, Ipp32u /*nTrack*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Set time position
    virtual Status SetTimePosition(Ipp64f /*timePos*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Get time position
    virtual Status GetTimePosition(Ipp64f& timePos)
        {timePos = 0; return UMC_ERR_NOT_IMPLEMENTED;}

    // Get splitter info
    virtual Status GetInfo(SplitterInfo** /*ppInfo*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Set playback rate
    virtual Status SetRate(Ipp64f /*rate*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // changes state of track
    // iState = 0 means disable, iState = 1 means enable
    virtual Status EnableTrack(Ipp32u /*nTrack*/, Ipp32s /*iState*/)
    {
        return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Runs reading threads
    virtual Status Run()
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    // Stops reading threads
    virtual Status Stop() = 0;

protected:

    DataReader  *m_pDataReader;  // (DataReader *) pointer to data reader
    SplitterInfo m_info;      // (SplitterInfo *) splitter info
};

} // namespace UMC

#endif /* __UMC_SPLITTER_H__ */
