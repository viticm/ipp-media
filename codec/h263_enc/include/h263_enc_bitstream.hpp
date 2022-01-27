/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2006 Intel Corporation. All Rights Reserved.
//
//  Description:    class H263BitStream
//
*/

#include "ippdefs.h"
#include <string.h>

#pragma warning(disable : 4514)     // unreferenced inline function has been removed

// class for BitStream
class H263BitStream
{
public :
    Ipp8u*  mPtr;
    Ipp8u*  mBuffer;
    Ipp32s  mBitOff;
    Ipp32s  mBuffSize;
    void    Init(Ipp8u *ptr, Ipp32s size) { mBuffer = mPtr = ptr; mBitOff = 0; mBuffSize = size; };
    Ipp8u*  GetPtr() { return mBuffer; };
    Ipp32s  GetFullness() { return (Ipp32s)(mPtr - mBuffer); };
    Ipp8u*  GetBitPtr() { return mPtr; };
    Ipp32s  GetBitOff() { return mBitOff; };
    void    GetPos(Ipp8u **ptr, Ipp32s *bitOff) { *ptr = mPtr; *bitOff = mBitOff; };
    void    SetPos(Ipp8u *ptr, Ipp32s bitOff = 0) { mPtr = ptr; mBitOff = bitOff; };
    void    Reset() { mPtr = mBuffer; mBitOff = 0; };
    void    PutBits(Ipp32u val, Ipp32s n);
    void    PutZeroBit() { PutBits(0, 1); };
    void    PutMarkerBit() { PutBits(1, 1); };
    H263BitStream() { mBuffer = mPtr = 0; mBitOff = mBuffSize = 0; };
};
