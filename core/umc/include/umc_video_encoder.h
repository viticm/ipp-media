/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_VIDEO_ENCODER_H__
#define __UMC_VIDEO_ENCODER_H__

#include "umc_base_codec.h"
#include "umc_par_reader.h"

namespace UMC
{

class VideoEncoderParams : public BaseCodecParams
{
    DYNAMIC_CAST_DECL(VideoEncoderParams, BaseCodecParams)
public:
    // Constructor
    VideoEncoderParams() :
        numEncodedFrames(0),
        qualityMeasure(51)
    {
      info.clip_info.width     = 0;
      info.clip_info.height    = 0;
      info.color_format        = YUV420;
      info.bitrate             = 0;
      info.aspect_ratio_width  = 1;
      info.aspect_ratio_height = 1;
      info.framerate           = 30;
      info.duration            = 0;
      info.interlace_type      = PROGRESSIVE;
      info.stream_type         = UNDEF_VIDEO;
      info.stream_subtype      = UNDEF_VIDEO_SUBTYPE;
      info.streamPID           = 0;

    }
    // Destructor
    virtual ~VideoEncoderParams(void){}
    // Read parameter from file
    virtual Status ReadParamFile(const vm_char * /*ParFileName*/)
    {
      return UMC_ERR_NOT_IMPLEMENTED;
    }

    VideoStreamInfo info;               // (VideoStreamInfo) compressed video info
    Ipp32s          numEncodedFrames;   // (Ipp32s) number of encoded frames

    // additional controls
    Ipp32s qualityMeasure;      // per cent, represent quantization precision
};

/******************************************************************************/

class VideoEncoder : public BaseCodec
{
    DYNAMIC_CAST_DECL(VideoEncoder, BaseCodec)
public:
    // Destructor
    virtual ~VideoEncoder() {};
};

/******************************************************************************/

// reads parameters from ParamList to VideoEncoderParams
Status ReadParamList(VideoEncoderParams* par, ParamList* lst);

// information about parameters for VideoEncoderParams
extern const ParamList::OptionInfo VideoEncoderOptions[];

} // end namespace UMC

#endif /* __UMC_VIDEO_ENCODER_H__ */
