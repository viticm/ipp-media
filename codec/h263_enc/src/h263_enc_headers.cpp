/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH263 (put headers to bitstream)
//  Contents:
//                  EncodePicture_Header
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_H263_VIDEO_ENCODER)
#include "h263_enc.hpp"

void ippVideoEncoderH263::EncodeZeroBitsAlign()
{
    if (cBS.mBitOff != 0)
        cBS.PutBits(0, 8 - cBS.mBitOff);
}

void ippVideoEncoderH263::EncodeStuffingBitsAlign()
{
    cBS.PutBits(0xFF >> (cBS.mBitOff + 1), 8 - cBS.mBitOff);
}

void ippVideoEncoderH263::EncodePicture_Header()
{
  cBS.PutBits(32, 22);
  cBS.PutBits(mVideoPicture.temporal_reference, 8);
  cBS.PutMarkerBit();
  cBS.PutZeroBit();
  cBS.PutBits(mVideoPicture.split_screen_indicator, 1);
  cBS.PutBits(mVideoPicture.document_camera_indicator, 1);
  cBS.PutBits(mVideoPicture.full_picture_freeze_release, 1);

  mVideoPicture.plusptype = mVideoPicture.advIntra || mVideoPicture.modQuant || (mVideoPicture.UMV > 1) || mVideoPicture.PCF
    || mVideoPicture.source_format > 5 || mVideoPicture.PAR_code != H263e_ASPECT_RATIO_12_11;
  // TODO: update when adding modes
  if (mVideoPicture.plusptype) {
    if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_I)
      mVideoPicture.ufep = 1; // ???
    else
      mVideoPicture.ufep = 0;
    if (mVideoPicture.UMV == 1) // switch to PLUSPTYPE UMV mode
      mVideoPicture.UMV = 2;
  }

  if (!mVideoPicture.plusptype) {
    mRTPdata.codingModes = (mVideoPicture.source_format << 5) | (mVideoPicture.picture_coding_type << 4) |
      (mVideoPicture.UMV << 3) | (mVideoPicture.SAC << 2) | (mVideoPicture.advPred << 1) | mVideoPicture.PBframes;
    cBS.PutBits(mRTPdata.codingModes, 8);
/*
    cBS.PutBits(mVideoPicture.source_format, 3);
    cBS.PutBits(mVideoPicture.picture_coding_type, 1);
    cBS.PutBits(mVideoPicture.UMV, 1);
    cBS.PutBits(mVideoPicture.SAC, 1);
    cBS.PutBits(mVideoPicture.advPred, 1);
    cBS.PutBits(mVideoPicture.PBframes, 1);
*/
    cBS.PutBits(mVideoPicture.pic_quant, 5);
    cBS.PutBits(mVideoPicture.CPM, 1);
    if (mVideoPicture.CPM)
      cBS.PutBits(mVideoPicture.PSBI, 1);
    mVideoPicture.pic_rounding_type = 0;
  } else { // PLUSPTYPE

    if (mVideoPicture.source_format == 7)
      mVideoPicture.source_format = 6;

    cBS.PutBits(7, 3);
    cBS.PutBits(mVideoPicture.ufep, 3);
    if (mVideoPicture.ufep == 1) { // OPPTYPE
      cBS.PutBits(mVideoPicture.source_format, 3);
      cBS.PutBits(mVideoPicture.PCF, 1);
      cBS.PutBits(mVideoPicture.UMV ? 1 : 0, 1);
      cBS.PutBits(mVideoPicture.SAC, 1);
      cBS.PutBits(mVideoPicture.advPred, 1);

      cBS.PutBits(mVideoPicture.advIntra, 1);
      cBS.PutBits(mVideoPicture.deblockFilt, 1);
      cBS.PutBits(mVideoPicture.sliceStruct, 1);

      cBS.PutBits(mVideoPicture.RPS, 1);
      cBS.PutBits(mVideoPicture.ISD, 1);
      cBS.PutBits(mVideoPicture.altInterVLC, 1);
      cBS.PutBits(mVideoPicture.modQuant, 1);

      cBS.PutBits(8, 4); // anti-emulation + 3 reserved
    }

    // MPPTYPE
    cBS.PutBits(mVideoPicture.picture_coding_type, 3);
    cBS.PutBits(mVideoPicture.resample, 1);
    cBS.PutBits(mVideoPicture.redResUp, 1);

    if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_P || mVideoPicture.picture_coding_type == H263e_PIC_TYPE_iPB)
      mVideoPicture.pic_rounding_type ^= 1;

    cBS.PutBits(mVideoPicture.pic_rounding_type, 1);
    cBS.PutBits(1, 3); // 3 reserved + anti-emulation

    cBS.PutBits(mVideoPicture.CPM, 1);
    if (mVideoPicture.CPM)
      cBS.PutBits(mVideoPicture.PSBI, 1);

    if (mVideoPicture.ufep == 1) {
      if (mVideoPicture.source_format == 6) {
        Ipp32s pwhi;
        cBS.PutBits(mVideoPicture.PAR_code, 4);
        pwhi = (mVideoPicture.pic_width >> 2) - 1;
        cBS.PutBits(pwhi, 9);
        cBS.PutBits(1, 1); // anti-emulation
        pwhi = mVideoPicture.pic_height >> 2;
        cBS.PutBits(pwhi, 9);
        if (mVideoPicture.PAR_code == H263e_ASPECT_RATIO_EXTPAR) { //
          cBS.PutBits(mVideoPicture.PAR_width, 8);
          cBS.PutBits(mVideoPicture.PAR_height, 8);
        }
      }
      if (mVideoPicture.PCF) {
        cBS.PutBits(mVideoPicture.clock_conversion_code, 1);
        cBS.PutBits(mVideoPicture.clock_divisor, 7);
      }
    }

    if (mVideoPicture.PCF)
      cBS.PutBits(mVideoPicture.temporal_reference >> 8, 2);

    if (mVideoPicture.ufep == 1) {
      if (mVideoPicture.UMV)
        cBS.PutBits(1, mVideoPicture.UMV - 1); // mVideoPicture.UMV == 2 or 3
      if (mVideoPicture.sliceStruct)
        cBS.PutBits(mVideoPicture.sliceSubmodes, 2);
    }

    if (mVideoPicture.picture_coding_type == H263e_PIC_TYPE_B || mVideoPicture.picture_coding_type == H263e_PIC_TYPE_EP || mVideoPicture.picture_coding_type == H263e_PIC_TYPE_EI) {
      mVideoPicture.scalability = 1;
      cBS.PutBits(mVideoPicture.enh_layer_num, 4);

      if (mVideoPicture.ufep == 1) {
        cBS.PutBits(mVideoPicture.ref_layer_num, 4);
      }
    }

#if 0
    // Reference Picture Selection mode
    if (mVideoPicture.RPS) {
      if (mVideoPicture.ufep == 1)
        cBS.PutBits(mVideoPicture.RPSflags, 3);

      cBS.PutBits(mVideoPicture.TRPI, 1);
      if (mVideoPicture.TRPI)
        cBS.PutBits(mVideoPicture.pred_temp_ref, 10);

      if (mVideoPicture.BCI) {
        cBS.PutBits(1, 1);
        // EncodeBCM -  not implemented yet. TODO ???
      } else
        cBS.PutBits(1, 2);
    }
#endif

    if (mVideoPicture.resample) {
      cBS.PutBits((mVideoPicture.wda | 2), 2);
      // EncodeResampleParams - not implemented yet. TODO
    }

    cBS.PutBits(mVideoPicture.pic_quant, 5);
  }

  if (mVideoPicture.PBframes) {
    cBS.PutBits(mVideoPicture.temporal_reference_B, 3);
    cBS.PutBits(mVideoPicture.dbquant, 2);
  }
  cBS.PutZeroBit(); // pei
}

void ippVideoEncoderH263::EncodeGOB_Header(Ipp32s gob_num)
{
  if (mGSTUF) {
//    cBS.PutBits(0, (8 - cBS.mBitOff) & 7);
    cBS.mPtr += (cBS.mBitOff + 7) >> 3;
    cBS.mBitOff = 0;
  }

  cBS.PutBits(1, 17);
  cBS.PutBits(gob_num, 5);
//  if (mVideoPicture.CPM)  // Annex C, not implemented
//       cBS.PutBits(mVideoPicture.GSBI, 2);
  cBS.PutBits(0, 2); // GFID - not used
  cBS.PutBits(mVideoPicture.pic_quant, 5); // GQUANT
}
#endif // defined (UMC_ENABLE_H263_VIDEO_ENCODER)
