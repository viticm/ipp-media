/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2008 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoEncoderH261 (tables)
//  Contents:
//                  mVLC_MBA
//                  mLen_MType
//                  mVLC_MVD
//                  mVLC_CBP
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_H261_VIDEO_ENCODER)
#include "h261_enc.hpp"

const VLCcode ippVideoEncoderH261::mVLC_MBA[34] = {
  {1,1}, {3,3}, {2,3}, {3,4}, {2,4}, {3,5}, {2,5}, {7,7}, {6,7},
  {11,8}, {10,8}, {9,8}, {8,8}, {7,8}, {6,8}, {23,10}, {22,10}, {21,10},
  {20,10}, {19,10}, {18,10}, {35,11}, {34,11}, {33,11}, {32,11}, {31,11}, {30,11},
  {29,11}, {28,11}, {27,11}, {26,11}, {25,11}, {24,11}, {15,11}
};

const Ipp8u ippVideoEncoderH261::mLen_MType[16] = {
  0, 0, 9, 0, 1, 5, 8, 10, 0, 0, 3, 0, 0, 0, 2, 6
};

const VLCcode ippVideoEncoderH261::mVLC_MVD[32] = { // only 17 are used
  /* 0, 1, 2, ... 16 */
  {1,1}, {2,3}, {2,4}, {2,5}, {6,7}, {10,8}, {8,8}, {6,8}, {22,10}, {20,10},
  {18,10}, {34,11}, {32,11}, {30,11}, {28,11}, {26,11}, {25,11}
};

const VLCcode ippVideoEncoderH261::mVLC_CBP[63] = {
  {11,5}, {9,5}, {13,6}, {13,4}, {23,7}, {19,7}, {31,8}, {12,4}, {22,7},
  {18,7}, {30,8}, {19,5}, {27,8}, {23,8}, {19,8}, {11,4}, {21,7}, {17,7},
  {29,8}, {17,5}, {25,8}, {21,8}, {17,8}, {15,6}, {15,8}, {13,8}, {3,9},
  {15,5}, {11,8}, {7,8}, {7,9}, {10,4}, {20,7}, {16,7}, {28,8}, {14,6},
  {14,8}, {12,8}, {2,9}, {16,5}, {24,8}, {20,8}, {16,8}, {14,5}, {10,8},
  {6,8}, {6,9}, {18,5}, {26,8}, {22,8}, {18,8}, {13,5}, {9,8}, {5,8},
  {5,9}, {12,5}, {8,8}, {4,8}, {4,9}, {7,3}, {10,5}, {8,5}, {12,6}
};
#endif // defined (UMC_ENABLE_H261_VIDEO_ENCODER)
