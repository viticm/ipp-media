/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

//#include <string.h>
#include "umc_video_encoder.h"

namespace UMC
{

// reads parameters from ParamList to VideoEncoderParams
Status ReadParamList(VideoEncoderParams* par, ParamList* lst)
{
  Status sts; // checked rarely - dst is not modified if failed

  if(par == 0 || lst == 0)
    return UMC_ERR_NULL_PTR;

  // BaseCodecParams:
  sts = lst->getValue(VM_STRING("threads"),&par->numThreads);
  sts = lst->getValue(VM_STRING("profile"),&par->profile);
  sts = lst->getValue(VM_STRING("level"),  &par->level);

  // VideoEncoderParams:
  sts = lst->getValue(VM_STRING("width"),  &par->info.clip_info.width);
  sts = lst->getValue(VM_STRING("height"), &par->info.clip_info.height);
  sts = lst->getValue(VM_STRING("bitrate"), (Ipp32s*)&par->info.bitrate);
  sts = lst->getValue(VM_STRING("framerate"), &par->info.framerate);
  sts = lst->getValue(VM_STRING("aspect"), &par->info.aspect_ratio_width, 0);
  sts = lst->getValue(VM_STRING("aspect"), &par->info.aspect_ratio_height, 1);

  // Color format is string name
  const vm_char* ptmp;
  ColorFormat cf;
  sts = lst->getValue(VM_STRING("cformat"), &ptmp);
  if (sts >= UMC_OK && UMC_OK == GetFormatType(ptmp, &cf))
    par->info.color_format = cf;

  VideoStreamType vt;
  sts = lst->getValue(VM_STRING("codec"), &ptmp);
  if (sts >= UMC_OK && UMC_OK == GetVideoType(ptmp, &vt))
    par->info.stream_type = vt;

  // interlace type is a number for now, can be changed to some string names
  Ipp32s itmp = -1;
  sts = lst->getValue(VM_STRING("interlace"), &itmp);
  if(sts >= UMC_OK) {
    if(itmp == PROGRESSIVE)
      par->info.interlace_type = PROGRESSIVE;
    else if (itmp == INTERLEAVED_TOP_FIELD_FIRST)
      par->info.interlace_type = INTERLEAVED_TOP_FIELD_FIRST;
    else if (itmp == INTERLEAVED_BOTTOM_FIELD_FIRST)
      par->info.interlace_type = INTERLEAVED_BOTTOM_FIELD_FIRST;
  }

  return UMC_OK;
}

// information about parameters for VideoEncoderParams
const ParamList::OptionInfo VideoEncoderOptions[] = {
  {VM_STRING("threads"), 0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0 128"), VM_STRING("maximum number of threads, 0 for best")},
  {VM_STRING("t"), VM_STRING("threads"), },
  {VM_STRING("profile"),   0, 1, ParamList::argOther, ParamList::checkNone, 0, VM_STRING("encoder profile")},
  {VM_STRING("level"),     0, 1, ParamList::argOther, ParamList::checkNone, 0, VM_STRING("encoder level")},
  {VM_STRING("width"),     0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1;4096"), VM_STRING("frame width in pixels")},
  {VM_STRING("w"), VM_STRING("width"), },
  {VM_STRING("height"),    0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 4096"), VM_STRING("frame height in pixels")},
  {VM_STRING("h"), VM_STRING("height"), },
  {VM_STRING("bitrate"),   0, 1, ParamList::argFlt, ParamList::checkMinMax, VM_STRING("0,1.e9"), VM_STRING("bitrate in bps")},
  {VM_STRING("b"), VM_STRING("bitrate"), },
  {VM_STRING("framerate"), 0, 1, ParamList::argFlt, ParamList::checkNone, 0, VM_STRING("frame rate in fps")},
  {VM_STRING("f"), VM_STRING("framerate"), },
  {VM_STRING("aspect"),    0, 2, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1;4096"), VM_STRING("pixel aspect ratio x:y")},
  {VM_STRING("cformat"),   0, 1, ParamList::argStr, ParamList::checkNone, 0, VM_STRING("color format name")},
  {VM_STRING("interlace"), 0, 1, ParamList::argInt, ParamList::checkSet, VM_STRING("0 2 3"), VM_STRING("interlace type: 0 - prog, 2 - top first, 3 - bottom first")},
  {VM_STRING("codec"),     0, 1, ParamList::argStr, ParamList::checkSet, VM_STRING("mpeg2,m2;mpeg4,m4;h264 h263 h261;vc1_video ;dv_SD dv_50 dv_HD, mjpeg"), VM_STRING("codec name")},
  {0,} // list terminator
};

}
