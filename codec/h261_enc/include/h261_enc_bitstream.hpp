/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippBitStreamH
//
*/

#include "ippdefs.h"
#include <string.h>

#pragma warning(disable : 4514)     // unreferenced inline function has been removed

// class for BitStream
class ippBitStreamH
{
public :
  Ipp8u*  mPtr;
  Ipp8u*  mBuffer;
  Ipp32s  mBitOff;
  Ipp32s  mBuffSize;
  void    PutBits(Ipp32u val, Ipp32s n);
  void    PutZeroBit() { PutBits(0, 1); };
  void    PutMarkerBit() { PutBits(1, 1); };
  Ipp32s  GetFullness() { return (Ipp32s)(mPtr - mBuffer); };
  ippBitStreamH() { mBuffer = mPtr = 0; mBitOff = mBuffSize = 0; };
};
