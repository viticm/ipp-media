/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_VIDEO_DECODER_H__
#define __UMC_VIDEO_DECODER_H__

#include "umc_structures.h"
#include "umc_video_data.h"
#include "umc_base_codec.h"
#include "umc_base_color_space_converter.h"

namespace UMC
{

class VideoAccelerator;

class VideoDecoderParams : public BaseCodecParams
{
    DYNAMIC_CAST_DECL(VideoDecoderParams, BaseCodecParams)

public:
    // Default constructor
    VideoDecoderParams();
    // Destructor
    virtual ~VideoDecoderParams();

    VideoStreamInfo         info;                           // (VideoStreamInfo) compressed video info
    Ipp32u                  lFlags;                         // (Ipp32u) decoding flag(s)
    Ipp32u                  lTrickModesFlag;                // (Ipp32u) trick modes

    Ipp64f                  dPlaybackRate;

    BaseCodec               *pPostProcessing;               // (BaseCodec*) pointer to post processing

    VideoAccelerator        *pVideoAccelerator;             // pointer to video accelerator
};

/******************************************************************************/

class VideoDecoder : public BaseCodec
{
    DYNAMIC_CAST_DECL(VideoDecoder, BaseCodec)

public:
    VideoDecoder(void) :
        m_PostProcessing(NULL),
        m_allocatedPostProcessing(NULL)
    {}

    // Destructor
    virtual ~VideoDecoder(void);

    // BaseCodec methods
    // Get codec working (initialization) parameter(s)
    virtual Status GetInfo(BaseCodecParams *info);
    // Set new working parameter(s)
    virtual Status SetParams(BaseCodecParams *params);

    // Additional methods
    // Reset skip frame counter
    virtual Status ResetSkipCount() = 0;
    // Increment skip frame counter
    virtual Status SkipVideoFrame(Ipp32s) = 0;
    // Get skip frame counter statistic
    virtual Ipp32u GetNumOfSkippedFrames() = 0;
    // Preview last decoded frame
    virtual Status PreviewLastFrame(VideoData *out, BaseCodec *pPostProcessing = NULL);

    // returns closed capture data
    virtual Status GetUserData(MediaData* /*pCC*/)
    {
        return UMC_ERR_NOT_IMPLEMENTED;
    }

protected:

    VideoStreamInfo         m_ClipInfo;                         // (VideoStreamInfo) clip info
    VideoData               m_LastDecodedFrame;                 // (VideoData) last decoded frame
    BaseCodec               *m_PostProcessing;                  // (BaseCodec*) pointer to post processing
    BaseCodec               *m_allocatedPostProcessing;         // (BaseCodec*) pointer to default post processing allocated by decoder
};

} // end namespace UMC

#endif // __UMC_VIDEO_DECODER_H__
