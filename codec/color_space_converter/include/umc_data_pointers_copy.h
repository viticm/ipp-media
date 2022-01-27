/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//        Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_DATA_POINTERS_COPY_H__
#define __UMC_DATA_POINTERS_COPY_H__

#include "vm_types.h"
#include "ippdefs.h"
#include "umc_video_data.h"
#include "umc_base_codec.h"

namespace UMC
{

// This class is designed as special kind of VideoProcessing which just
// copies pointers and therefore gives access to internal buffers of decoders.

class DataPointersCopy : public BaseCodec
{
  DYNAMIC_CAST_DECL(DataPointersCopy, BaseCodec)
public:
  virtual Status Init(BaseCodecParams* /*init*/)
  {
    return UMC_OK;
  }

  virtual Status GetFrame(MediaData *input, MediaData *output)
  {
    VideoData *video_input = DynamicCast<VideoData>(input);
    VideoData *video_output = DynamicCast<VideoData>(output);

    if (video_input && video_output) {
      *video_output = *video_input;
    } else {
      *output = *input;
    }

    return UMC_OK;
  }

  virtual Status GetInfo(BaseCodecParams* /*info*/)
  {
    return UMC_OK;
  }

  virtual Status Close(void)
  {
    return UMC_OK;
  }

  virtual Status Reset(void)
  {
    return UMC_OK;
  }
};

} // namespace UMC

#endif /* __UMC_DATA_POINTERS_COPY_H__ */
