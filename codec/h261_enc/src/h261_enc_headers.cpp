/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH261 (put headers to bitstream)
//  Contents:
//                  EncodeFrame
//
*/

#include "umc_config.h"
#include "umc_defs.h"

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)
#include "h261_enc.hpp"


inline void ippVideoEncoderH261::EncodeStartCode(Ipp8u gn)
{
  /* if gn==0 - picture start code */
  cBS.PutBits(16 + gn, 20);
}


void ippVideoEncoderH261::EncodeZeroBitsAlign()
{
  if (cBS.mBitOff != 0)
    cBS.PutBits(0, 8 - cBS.mBitOff);
}


void ippVideoEncoderH261::EncodeStuffingBitsAlign()
{
  cBS.PutBits(0xFF >> (cBS.mBitOff + 1), 8 - cBS.mBitOff);
}

void ippVideoEncoderH261::EncodePicture_Header()
{
  cBS.PutBits(16, 20);
  cBS.PutBits(mVideoPicture.temporal_reference, 5);
  cBS.PutBits(mVideoPicture.split_screen_indicator, 1);
  cBS.PutBits(mVideoPicture.document_camera_indicator, 1);
  cBS.PutBits(mVideoPicture.freeze_picture_release, 1);
  cBS.PutBits(mVideoPicture.source_format, 1);
  cBS.PutBits(mVideoPicture.still_image_mode, 1);
  cBS.PutZeroBit();
  cBS.PutZeroBit();  // pei
}


void ippVideoEncoderH261::EncodeGOB_Header()
{
  Ipp32u gobn_code;
  if (mVideoPicture.source_format == 0)
    gobn_code = 2*mVideoPicture.gob_number - 1;
  else
    gobn_code = mVideoPicture.gob_number;
  cBS.PutBits(1, 16);
  cBS.PutBits(gobn_code, 4);
  cBS.PutBits(mVideoPicture.frame_quant, 5);
  cBS.PutZeroBit();  // gei
}


void ippVideoEncoderH261::EncodeMBA(Ipp8u mba)
{
  if (mba == IPPVC_MB_STUFFING)
    mba = 34;

  cBS.PutBits(mVLC_MBA[mba-1].val, mVLC_MBA[mba-1].len);
}

void ippVideoEncoderH261::EncodeMType(Ipp8u mtype)
{
  Ipp32s len;
  if (mtype & MTYPE_INTRA)
    len = (mtype & MTYPE_MQUANT) ? 7 : 4;
  else
    len = mLen_MType[mtype];
  cBS.PutBits(1, len);
}

void ippVideoEncoderH261::EncodeMVD(IppMotionVector mvd)
{
  Ipp32s x, y, sx = 0, sy = 0;
  VLCcode vlc;

  x = mvd.dx;
  if (x & 16) {
    x = 32 - x;
    sx = 1;
  }
  x &= 0x1F;

  y = mvd.dy;
  if (y & 16) {
    y = 32 - y;
    sy = 1;
  }
  y &= 0x1F;

  vlc = mVLC_MVD[x];
  cBS.PutBits((vlc.val | sx), vlc.len);
  vlc = mVLC_MVD[y];
  cBS.PutBits((vlc.val | sy), vlc.len);
}

void ippVideoEncoderH261::EncodeCBP(Ipp8u cbp)
{
  VLCcode vlc;
  vlc = mVLC_CBP[cbp-1];
  cBS.PutBits(vlc.val, vlc.len);
}
#endif //defined (UMC_ENABLE_H261_VIDEO_ENCODER)

