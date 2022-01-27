/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//        Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_COLOR_SPACE_CONVERSION_H__
#define __UMC_COLOR_SPACE_CONVERSION_H__

#include "umc_base_codec.h"

namespace UMC
{

class ColorSpaceConversion : public BaseCodec
{
  DYNAMIC_CAST_DECL(ColorSpaceConversion, BaseCodec)
public:
  // Initialize codec with specified parameter(s)
  virtual Status Init(BaseCodecParams *) { return UMC_OK; };

  // Convert next frame
  virtual Status GetFrame(MediaData *in, MediaData *out);

  // Get codec working (initialization) parameter(s)
  virtual Status GetInfo(BaseCodecParams *) { return UMC_OK; };

  // Close all codec resources
  virtual Status Close(void) { return UMC_OK; };

  // Set codec to initial state
  virtual Status Reset(void) { return UMC_OK; };
};

} // namespace UMC

#endif /* __UMC_COLOR_SPACE_CONVERSION_H__ */
