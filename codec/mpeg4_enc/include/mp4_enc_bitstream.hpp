/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2003-2006 Intel Corporation. All Rights Reserved.
//
//  Description:    class ippVideoBitStream
//
*/

#include "ippdefs.h"
#include <string.h>

#pragma warning(disable : 4514)     // unreferenced inline function has been removed

// class for BitStream
class ippBitStream
{
public :
    Ipp8u*  mPtr;
    Ipp8u*  mBuffer;
    Ipp32s  mBitOff;
    Ipp32s  mBuffSize;
    void    Init(Ipp8u *ptr, Ipp32s size) { mBuffer = mPtr = ptr; mBitOff = 0; mBuffSize = size; };
    void    Reset() { mPtr = mBuffer; mBitOff = 0; };
    Ipp8u*  GetPtr() { return mBuffer; };
    Ipp32s  GetFullness() { return (Ipp32s)(mPtr - mBuffer); };
    Ipp8u*  GetBitPtr() { return mPtr; };
    Ipp32s  GetBitOff() { return mBitOff; };
    void    GetPos(Ipp8u **ptr, Ipp32s *bitOff) { *ptr = mPtr; *bitOff = mBitOff; };
    void    SetPos(Ipp8u *ptr, Ipp32s bitOff = 0) { mPtr = ptr; mBitOff = bitOff; };
    Ipp32s  GetNumBits() { return ((Ipp32s)((mPtr - mBuffer) << 3) + mBitOff); };
    Ipp32s  GetNumBits(Ipp8u *ptr, Ipp32s off) { return ((Ipp32s)((mPtr - ptr) << 3) + mBitOff - off); };
    void    MovePtr(Ipp32s nBits) { mPtr += (mBitOff + nBits) >> 3; mBitOff = (mBitOff + nBits) & 7; };
    void    PutBits(Ipp32u val, Ipp32s n);
    void    PutBit(Ipp32u val);
    void    PutZeroBit() { PutBit(0); };
    void    PutMarkerBit() { PutBit(1); };
    inline void PutStr(const Ipp8s* str);
    ippBitStream() { mBuffer = mPtr = 0; mBitOff = mBuffSize = 0; };
};

inline void ippBitStream::PutStr(const Ipp8s *str)
{
    if (mBitOff > 0) {
        mBitOff = 0;
        mPtr ++;
    }
    strcpy((char *)mPtr, (char *)str);
    mPtr += strlen((char *)str);
}
