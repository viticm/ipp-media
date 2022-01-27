//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#if PIXBITS == 8

#define PIXTYPE Ipp8u
#define COEFFSTYPE Ipp16s
#define H264ENC_MAKE_NAME(NAME) NAME##_8u16s

#elif PIXBITS == 16

#define PIXTYPE Ipp16u
#define COEFFSTYPE Ipp32s
#define H264ENC_MAKE_NAME(NAME) NAME##_16u32s

#elif //PIXBITS

void H264EncoderFakeFunction() { UNSUPPORTED_PIXBITS; }

#endif //PIXBITS

#define H264BsBaseType H264ENC_MAKE_NAME(H264BsBase)
#define H264BsRealType H264ENC_MAKE_NAME(H264BsReal)
#define H264BsFakeType H264ENC_MAKE_NAME(H264BsFake)

/*Status H264ENC_MAKE_NAME(H264BsReal_Create)(
    H264BsRealType* state)
{
    return H264ENC_MAKE_NAME(H264BsReal_Create)(state, NULL, 0);
}

Status H264ENC_MAKE_NAME(H264BsFake_Create)(
    H264BsFakeType* state);
{
    return H264ENC_MAKE_NAME(H264BsFake_Create)(state, NULL, 0);
}*/

// ---------------------------------------------------------------------------
//  CBaseBitstream::CBaseBitstream()
//      Constructs a new object.  Sets base pointer to point to the given
//      bitstream buffer.  Neither the encoder or the decoder allocates
//      memory for the bitstream buffer.
//      pb      : pointer to input bitstream
//      maxsize : size of bitstream
// ---------------------------------------------------------------------------

Status H264ENC_MAKE_NAME(H264BsReal_Create)(
    H264BsRealType* state,
    Ipp8u* const pb,
    const Ipp32u maxsize,
    Ipp32s chroma_format_idc,
    Status &plr)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    bs->m_base.m_pbsBase   = pb;
    bs->m_base.m_pbs       = pb;
    bs->m_base.m_bitOffset = 0;
    bs->m_base.m_maxBsSize = maxsize;

    bs->m_pbsRBSPBase = pb;
    plr = UMC_OK;

    if (chroma_format_idc)
        bs->num8x8Cshift2 = chroma_format_idc - 1;
    else
        bs->num8x8Cshift2 = 0;

    return UMC_OK;
}

Status H264ENC_MAKE_NAME(H264BsFake_Create)(
    H264BsFakeType* state,
    Ipp8u* const pb,
    const Ipp32u maxsize,
    Ipp32s chroma_format_idc,
    Status &plr)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    bs->m_base.m_pbsBase   = pb;
    bs->m_base.m_pbs       = pb;
    bs->m_base.m_bitOffset = 0;
    bs->m_base.m_maxBsSize = maxsize;

    bs->m_pbsRBSPBase = pb;
    plr = UMC_OK;

    if (chroma_format_idc)
        bs->num8x8Cshift2 = chroma_format_idc - 1;
    else
        bs->num8x8Cshift2 = 0;

    return UMC_OK;
}

// ---------------------------------------------------------------------------
//  [ENC] CBaseBitstream::PutBits()
//      This is a special debug version that Detects Start Code Emulations.
//      Appends bits into the bitstream buffer.  Supports only up to 24 bits.
//
//      code        : code to be inserted into the bitstream
//      code_length : length of the given code in number of bits
// ---------------------------------------------------------------------------

void H264ENC_MAKE_NAME(H264BsReal_PutBit)(
    void* state,
    Ipp32u code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    if (code & 1)
        bs->m_base.m_pbs[0] =
            (Ipp8u)(bs->m_base.m_pbs[0] | (Ipp8u)(0x01 << (7 - bs->m_base.m_bitOffset)));
    //else
    //    bs->m_base.m_pbs[0] =
    //        (Ipp8u)(bs->m_base.m_pbs[0] & (Ipp8u)(0xff << (8 - bs->m_base.m_bitOffset)));

    bs->m_base.m_bitOffset++;
    if (bs->m_base.m_bitOffset == 8)
    {
        bs->m_base.m_pbs++;
        bs->m_base.m_pbs[0] = 0;
        bs->m_base.m_bitOffset = 0;
    }
}

void H264ENC_MAKE_NAME(H264BsReal_PutBits)(
    void* state,
    Ipp32u code,
    Ipp32u length)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    // make sure that the number of bits given is <= 24
    // clear any nonzero bits in upper part of code
    VM_ASSERT(length <= 24);
    code <<= (32 - length);

    // shift field so that the given code begins at the current bit
    // offset in the most significant byte of the 32-bit word
    length += bs->m_base.m_bitOffset;
    code >>= bs->m_base.m_bitOffset;

    // write bytes back into memory, big-endian
    bs->m_base.m_pbs[0] = (Ipp8u)((code >> 24) | bs->m_base.m_pbs[0]);
    bs->m_base.m_pbs[1] = (Ipp8u)(code >> 16);
//f
    bs->m_base.m_pbs[2] = 0;
    if (length > 16)
    {
        bs->m_base.m_pbs[2] = (Ipp8u)(code >> 8);
        bs->m_base.m_pbs[3] = (Ipp8u)(code);
    }

    // update bitstream pointer and bit offset
    bs->m_base.m_pbs += (length >> 3);
    bs->m_base.m_bitOffset = (length & 7);
}

// ---------------------------------------------------------------------------
//  [ENC] CBaseBitstream::PutVLCCode()
//      Writes one Exp-Golomb code to the bitstream.  Automatically calculates
//      the required code length.  Use only when this can not be implicitly
//      known to the calling code, requiring length calculation anyway.
//
//      code        : code to be inserted into the bitstream
// ---------------------------------------------------------------------------

Ipp32u H264ENC_MAKE_NAME(H264BsReal_PutVLCCode)(
    void* state,
    const Ipp32u code)
{
    Ipp32s i, NN;
    Ipp32u code_length;
    NN = code + 1;

#if defined(__i386__) && defined(__GNUC__) && (__GNUC__ > 3) && !defined(__INTEL_COMPILER)
    i = 31 - __builtin_clz(NN);
#elif defined(__INTEL_COMPILER) && (defined(__i386__) || defined(WIN32)) && !defined(WIN64)
    i = _bit_scan_reverse( NN );
#elif defined(_MSC_VER) && (_MSC_FULL_VER >= 140050110) && !defined(__INTEL_COMPILER)
    unsigned long idx;
    _BitScanReverse(&idx, (unsigned long)NN);
    i = (Ipp32s)idx;
#else
    i = -1;
    while (NN) {
        NN >>= 1;
        i++;
    }
#endif

    code_length = 1 + (i << 1);
//f    PutVLCBits(code,code_length);
    if (code_length == 1)
        H264ENC_MAKE_NAME(H264BsReal_PutBit)(state, 1);
    else {
        Ipp32s info_length = (code_length - 1) >> 1;
        Ipp32s bits = code + 1 - (1 << info_length);
        H264ENC_MAKE_NAME(H264BsReal_PutBits)(state, 1, info_length + 1);
        H264ENC_MAKE_NAME(H264BsReal_PutBits)(state, bits, info_length);
    }

    return code_length;
}

// ---------------------------------------------------------------------------
//  [ENC] CBaseBitstream::PutVLCBits()
//      Writes one Exp-Golomb code to the bitstream.
//      code        : code to be inserted into the bitstream
//      code_length : length of the given code in number of bits
// ---------------------------------------------------------------------------

void H264ENC_MAKE_NAME(H264BsReal_PutVLCBits)(
    H264BsRealType* state,
    const Ipp32u code,
    const Ipp32u code_length)
{
    Ipp32s info_length, bits;

    if (code_length == 1){
        H264ENC_MAKE_NAME(H264BsReal_PutBit)(state, 1);
        return;
    }

    info_length = (code_length-1) >> 1;
    bits = code+1-(1<<info_length);

    H264ENC_MAKE_NAME(H264BsReal_PutBits)(state, 0, info_length);
    H264ENC_MAKE_NAME(H264BsReal_PutBit)(state, 1);
    H264ENC_MAKE_NAME(H264BsReal_PutBits)(state, bits, info_length);
}

void H264ENC_MAKE_NAME(H264BsReal_SaveCABACState)(
    void* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    memcpy(&bs->context_array_copy[0], &bs->m_base.context_array[0], 460 * sizeof(CABAC_CONTEXT));
    bs->m_lcodIRange_copy = bs->m_base.m_lcodIRange;
    bs->m_lcodIOffset_copy = bs->m_base.m_lcodIOffset;
    bs->m_nRegister_copy = bs->m_base.m_nRegister;
    bs->m_nReadyBits_copy = bs->m_base.m_nReadyBits;
    bs->m_pbs_copy = bs->m_base.m_pbs;
    bs->m_bitOffset_copy = bs->m_base.m_bitOffset;

#ifdef CABAC_FAST
    bs->m_nReadyBytes_copy = bs->m_base.m_nReadyBytes;
    bs->m_nOutstandingChunks_copy = bs->m_base.m_nOutstandingChunks;
#else
    bs->m_nOutstandingBits_copy = bs->m_base.m_nOutstandingBits;
#endif
}

void H264ENC_MAKE_NAME(H264BsReal_RestoreCABACState)(
    void* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    memcpy(&bs->m_base.context_array[0], &bs->context_array_copy[0], 460*sizeof(CABAC_CONTEXT));
    bs->m_base.m_lcodIRange = bs->m_lcodIRange_copy;
    bs->m_base.m_lcodIOffset = bs->m_lcodIOffset_copy;
    bs->m_base.m_nRegister = bs->m_nRegister_copy;
    bs->m_base.m_nReadyBits = bs->m_nReadyBits_copy;
    bs->m_base.m_pbs = bs->m_pbs_copy;
    bs->m_base.m_bitOffset =  bs->m_bitOffset_copy;
#ifdef CABAC_FAST
    bs->m_base.m_nReadyBytes = bs->m_nReadyBytes_copy;
    bs->m_base.m_nOutstandingChunks = bs->m_nOutstandingChunks_copy;
#else
    bs->m_base.m_nOutstandingBits = bs->m_nOutstandingBits_copy;
#endif
}

void H264ENC_MAKE_NAME(H264BsReal_CopyContext_CABAC)(
    void* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    memcpy(&bs->m_base.context_array[0],   &bstrm->context_array[0],  105*sizeof(CABAC_CONTEXT));
    memcpy(&bs->m_base.context_array[227], &bstrm->context_array[227], 49*sizeof(CABAC_CONTEXT));
    if (isFrame)
        memcpy(&bs->m_base.context_array[105], &bstrm->context_array[105], 122*sizeof(CABAC_CONTEXT));
    else
        memcpy(&bs->m_base.context_array[277], &bstrm->context_array[277], 122*sizeof(CABAC_CONTEXT));

    if (is8x8)
    {
        memcpy(&bs->m_base.context_array[426], &bstrm->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if (isFrame)
            memcpy(&bs->m_base.context_array[402], &bstrm->context_array[402], 24*sizeof(CABAC_CONTEXT));
        else
            memcpy(&bs->m_base.context_array[436], &bstrm->context_array[436], 24*sizeof(CABAC_CONTEXT));
    }

    bs->m_base.context_array[276] = bstrm->context_array[276]; //end of slice flag
    bs->m_base.context_array[399] = bstrm->context_array[399]; //transform 8x8 flag
    bs->m_base.context_array[400] = bstrm->context_array[400]; //transform 8x8 flag
    bs->m_base.context_array[401] = bstrm->context_array[401]; //transform 8x8 flag
/*
    bs->m_base.m_lcodIRange = bstrm->m_lcodIRange;
    bs->m_base.m_lcodIOffset = bstrm->m_lcodIOffset;
    bs->m_base.m_nRegister = bstrm->m_nRegister;
    bs->m_base.m_nReadyBits = bstrm->m_nReadyBits;
    bs->m_base.m_nOutstandingBits = bstrm->m_nOutstandingBits;
*/
}

void H264ENC_MAKE_NAME(H264BsReal_ResetBitStream_CABAC)(
    void* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    bs->m_base.m_lcodIOffset = 0;
    bs->m_base.m_nRegister = 0;
#ifdef CABAC_FAST
    bs->m_base.m_lcodIRange = ENC_M_HALF - 2;
    bs->m_base.m_nReadyBits = ENC_M_BITS + 1;
    bs->m_base.m_nReadyBytes = 0;
    bs->m_base.m_nOutstandingChunks = 0;
#else
    bs->m_base.m_lcodIRange=ENC_HALF_RANGE-2;
    bs->m_base.m_nReadyBits = 33;
//  bs->m_base.m_nReadyBits = 9;
    bs->m_base.m_nOutstandingBits = 0;
#endif
    H264BsBase_ByteAlignWithOnes(&bs->m_base);
}

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(
    void* state,
    Ipp32u b)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    if (bs->m_base.m_nReadyBytes == 4)
    {
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister);
        bs->m_base.m_nRegister = bs->m_base.m_nReadyBytes = 0;
    }
    else if (bs->m_base.m_nReadyBytes == 3)
    {
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister);
        bs->m_base.m_nReadyBytes = bs->m_base.m_nRegister = 0;
    }
    bs->m_base.m_nRegister <<= 16;
    bs->m_base.m_nRegister |= b;
    bs->m_base.m_nReadyBytes++;
    bs->m_base.m_nReadyBytes++;
    return;
}

void H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(
    void* state,
    Ipp32u b)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    if (bs->m_base.m_nReadyBytes > 0)
    {
        switch (bs->m_base.m_nReadyBytes)
        {
            case 4:
                *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
                *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
                *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
                *(bs->m_base.m_pbs+3) = (Ipp8u)(bs->m_base.m_nRegister);
                bs->m_base.m_pbs += 4;
                break;
            case 3:
                *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>16);
                *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>8);
                *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister);
                bs->m_base.m_pbs += 3;
                break;
            case 2:
                *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>8);
                *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister);
                bs->m_base.m_pbs += 2;
                break;
            case 1:
                *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister);
                bs->m_base.m_pbs++;
                break;
        }
        bs->m_base.m_nReadyBytes =  bs->m_base.m_nRegister = 0;
    }
    *(bs->m_base.m_pbs) = (Ipp8u)(b);
    bs->m_base.m_pbs++;
}
#else

void H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(
    H264BsRealType* state,
    bool code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    bs->m_base.m_nRegister <<= 1;
    bs->m_base.m_nRegister |= (Ipp32s)code;
    bs->m_base.m_nReadyBits --;
    if (0 == bs->m_base.m_nReadyBits)
    {
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
        //m_nReadyBits = 8;
    }
    return;
}

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(
    H264BsRealType* state,
    bool code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(bs, code);
    while (bs->m_base.m_nOutstandingBits > 0)
    {
        bs->m_base.m_nOutstandingBits--;
        H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(bs, !code);
    }
}

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(
    H264BsRealType* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    bs->m_base.m_nRegister <<= 1;
    //bs->m_base.m_nRegister |= 0;
    bs->m_base.m_nReadyBits --;
    if (0 == bs->m_base.m_nReadyBits)
    {
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
    }

    if (bs->m_base.m_nReadyBits < bs->m_base.m_nOutstandingBits)
    {
        bs->m_base.m_nRegister <<= bs->m_base.m_nReadyBits;
        bs->m_base.m_nRegister |= (Ipp32u)0xffffffff >> (32-bs->m_base.m_nReadyBits);
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nOutstandingBits -= bs->m_base.m_nReadyBits;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
        while (bs->m_base.m_nOutstandingBits >= 32)
        {
            *(bs->m_base.m_pbs) = 0xff;
            *(bs->m_base.m_pbs+1) = 0xff;
            *(bs->m_base.m_pbs+2) = 0xff;
            *(bs->m_base.m_pbs+3) = 0xff;
            bs->m_base.m_pbs += 4;
            bs->m_base.m_nOutstandingBits -= 32;
        }
    }

    if (bs->m_base.m_nOutstandingBits == 0)
        return;

    bs->m_base.m_nRegister <<= bs->m_base.m_nOutstandingBits;
    bs->m_base.m_nRegister |= (Ipp32u)0xffffffff >> (32-bs->m_base.m_nOutstandingBits);
    bs->m_base.m_nReadyBits -= bs->m_base.m_nOutstandingBits;
    if (0 == bs->m_base.m_nReadyBits)
    {
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
    }
    bs->m_base.m_nOutstandingBits = 0;
}

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(
    H264BsRealType* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    bs->m_base.m_nRegister <<= 1;
    bs->m_base.m_nRegister |= 1;
    bs->m_base.m_nReadyBits --;
    if (0 == bs->m_base.m_nReadyBits)
    {
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
    }

    if (bs->m_base.m_nReadyBits < bs->m_base.m_nOutstandingBits)
    {
        bs->m_base.m_nRegister <<= bs->m_base.m_nReadyBits;
        bs->m_base.m_nRegister &= (Ipp32u)0xffffffff << bs->m_base.m_nReadyBits;
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nOutstandingBits -= bs->m_base.m_nReadyBits;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
        while( bs->m_base.m_nOutstandingBits >= 32 ){
            *(bs->m_base.m_pbs) = 0;
            *(bs->m_base.m_pbs+1) = 0;
            *(bs->m_base.m_pbs+2) = 0;
            *(bs->m_base.m_pbs+3) = 0;
            bs->m_base.m_pbs += 4;
            bs->m_base.m_nOutstandingBits -= 32;
        }
    }

    if (bs->m_base.m_nOutstandingBits == 0)
        return;
    bs->m_base.m_nRegister <<= bs->m_base.m_nOutstandingBits;
    bs->m_base.m_nRegister &= (Ipp32u)0xffffffff << bs->m_base.m_nOutstandingBits;
    bs->m_base.m_nReadyBits -= bs->m_base.m_nOutstandingBits;
    if (0 == bs->m_base.m_nReadyBits)
    {
        *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>24);
        *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
        *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
        *(bs->m_base.m_pbs+3) = (Ipp8u)bs->m_base.m_nRegister;
        bs->m_base.m_pbs += 4;
        bs->m_base.m_nRegister = 0;
        bs->m_base.m_nReadyBits = 32;
    }
    bs->m_base.m_nOutstandingBits = 0;
}
#endif

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(
    void* state_,
    Ipp8u* ctx,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state_;
    Ipp8u pStateIdx = *ctx;
    Ipp8u state;
    register Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    register Ipp32u codIRange = bs->m_base.m_lcodIRange;
    Ipp32u codIRangeLPS = rangeTabLPS[pStateIdx][((codIRange >> 6) & 0x03)];

    codIRange -= codIRangeLPS;

    state = pStateIdx>>6;
    pStateIdx = transTbl[code][pStateIdx];
    if (code != state ){
        Ipp8u Renorm;
        codIOffset += (codIRange<<bs->m_base.m_nReadyBits);
        codIRange = codIRangeLPS;

        Renorm = renormTAB[(codIRangeLPS>>3)&0x1f];
        bs->m_base.m_nReadyBits -= Renorm;
        codIRange <<= Renorm;
        if( codIOffset >= ENC_M_FULL ){ //carry
            codIOffset -= ENC_M_FULL;
            bs->m_base.m_nRegister++;
            while( bs->m_base.m_nOutstandingChunks > 0 ){
                H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(bs, 0);
                bs->m_base.m_nOutstandingChunks--;
            }
        }
        if( bs->m_base.m_nReadyBits > 0 ){
            *ctx = pStateIdx;
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;  //no output
        }
    }else{
        if( codIRange >= ENC_M_QUARTER ){
            *ctx = pStateIdx;
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;
        }
        codIRange <<= 1;
        bs->m_base.m_nReadyBits--;
        if( bs->m_base.m_nReadyBits > 0 ){
            *ctx = pStateIdx;
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;
        }
    }

    Ipp32s L = (codIOffset>>ENC_B_BITS)& ((1<<ENC_M_BITS)-1);
    codIOffset = (codIOffset<<ENC_M_BITS)&(ENC_M_FULL-1);
    if( L < ((1<<ENC_M_BITS)-1) ){
        while( bs->m_base.m_nOutstandingChunks > 0 ){
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state_, 0xffff);
            bs->m_base.m_nOutstandingChunks--;
        }
        H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state_, L);
    }else{
        bs->m_base.m_nOutstandingChunks++;
    }
    bs->m_base.m_nReadyBits += ENC_M_BITS;

    *ctx = pStateIdx;
    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
}

#else

void H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp8u  pStateIdx = *ctx;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    Ipp32u codIRange = bs->m_base.m_lcodIRange;
    Ipp32u codIRangeLPS = rangeTabLPS[pStateIdx][((codIRange >> 6) & 0x03)];

    codIRange -= codIRangeLPS;

    if (code != (pStateIdx >> 6))
    {
        codIOffset += codIRange;
        codIRange = codIRangeLPS;
    }
    pStateIdx = transTbl[code][pStateIdx];

    /* renormalisation */
    while (codIRange < ENC_QUARTER_RANGE)
    {
        if (codIOffset >= ENC_HALF_RANGE)
        {
            //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 1);
            H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(bs);
            codIOffset -= ENC_HALF_RANGE;
        }
        else if (codIOffset < ENC_QUARTER_RANGE)
        {
            //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 0);
            H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(bs);
        }
        else
        {
            bs->m_base.m_nOutstandingBits++;
            codIOffset -= ENC_QUARTER_RANGE;
        }

        codIOffset <<= 1;
        codIRange <<= 1;
    }

    *ctx = pStateIdx;
    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
}
#endif

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_EncodeBins_CABAC)(
    void* state_,
    Ipp8u* ctx,
    Ipp32u code,
    Ipp32s len)
{
    H264BsRealType* bs = (H264BsRealType *)state_;
    Ipp8u pStateIdx = *ctx;
    Ipp8u state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    register Ipp32u codIRange = bs->m_base.m_lcodIRange;
    Ipp32u codIRangeLPS;

    while (len > 0)
    {
        codIRangeLPS = rangeTabLPS[pStateIdx][((codIRange >> 6) & 0x03)];
        codIRange -= codIRangeLPS;

        len--;
        Ipp8u c = (Ipp8u)((code >> len) & 1);
        state = pStateIdx>>6;
        pStateIdx = transTbl[c][pStateIdx];
        if (c != state)
        {
            Ipp8u Renorm;
            codIOffset += (codIRange << bs->m_base.m_nReadyBits);
            codIRange = codIRangeLPS;

            Renorm = renormTAB[(codIRangeLPS>>3)&0x1f];
            bs->m_base.m_nReadyBits -= Renorm;
            codIRange <<= Renorm;
            if (codIOffset >= ENC_M_FULL)
            { //carry
                codIOffset -= ENC_M_FULL;
                bs->m_base.m_nRegister++;
                while (bs->m_base.m_nOutstandingChunks > 0)
                {
                    H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state_, 0);
                    bs->m_base.m_nOutstandingChunks--;
                }
            }
            if (bs->m_base.m_nReadyBits > 0)
                continue;
        }
        else
        {
            if (codIRange >= ENC_M_QUARTER)
                continue;
            codIRange <<= 1;
            bs->m_base.m_nReadyBits--;
            if (bs->m_base.m_nReadyBits > 0)
                continue;
        }

        Ipp32s L = (codIOffset >> ENC_B_BITS) & ((1 << ENC_M_BITS) - 1);
        codIOffset = (codIOffset << ENC_M_BITS) & (ENC_M_FULL - 1);
        if (L < ((1 << ENC_M_BITS) - 1))
        {
            while (bs->m_base.m_nOutstandingChunks > 0)
            {
                H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state_, 0xffff);
                bs->m_base.m_nOutstandingChunks--;
            }
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state_, L);
        }
        else
        {
            bs->m_base.m_nOutstandingChunks++;
        }
        bs->m_base.m_nReadyBits += ENC_M_BITS;
    }

    bs->m_base.m_lcodIRange = codIRange;
    *ctx = pStateIdx;
    bs->m_base.m_lcodIOffset = codIOffset;
}

#else
void H264ENC_MAKE_NAME(H264BsReal_EncodeBins_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32u code,
    Ipp32s len)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u pStateIdx = *ctx;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    register Ipp32u codIRange = bs->m_base.m_lcodIRange;
    Ipp32u codIRangeLPS;

    while (len > 0)
    {
        codIRangeLPS = rangeTabLPS[pStateIdx][((codIRange >> 6) & 0x03)];
        codIRange -= codIRangeLPS;

        len--;
        Ipp32u c = (code>>len)&1;
        if(c != (pStateIdx>>6) ){
            codIOffset += codIRange;
            codIRange = codIRangeLPS;
        }
        pStateIdx = transTbl[c][pStateIdx];

        /* renormalisation */
        while (codIRange  < ENC_QUARTER_RANGE)
        {
            if (codIOffset >= ENC_HALF_RANGE)
            {
                H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(bs);
                codIOffset  -= ENC_HALF_RANGE;
            }
            else if (codIOffset < ENC_QUARTER_RANGE)
            {
                H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(bs);
            }
            else
            {
                bs->m_base.m_nOutstandingBits++;
                codIOffset  -= ENC_QUARTER_RANGE;
            }

            codIOffset<<= 1;
            codIRange <<= 1;
        }
    }

    bs->m_base.m_lcodIRange = codIRange;
    *ctx = (Ipp8u)pStateIdx;
    bs->m_base.m_lcodIOffset = codIOffset;
}
#endif

#if 1
void H264ENC_MAKE_NAME(H264BsFake_EncodeBins_CABAC)(
    H264BsFakeType* state,
    Ipp8u* ctx,
    Ipp32u code,
    Ipp32s len)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    register Ipp8u pStateIdx = *ctx;
    register Ipp32s bits=0;

    while (len>0)
    {
        len--;
        Ipp32u c = (code>>len)&1;
        bits += ( c ? p_bits[pStateIdx^64] : p_bits[pStateIdx] );
        pStateIdx = transTbl[c][pStateIdx];
    }

    *ctx = pStateIdx;
    bs->m_base.m_bitOffset += bits;
#if 0
    register Ipp8u pStateIdx = bs->m_base.context_array[ctxIdx].pStateIdx;
    register Ipp32u codIRange = bs->m_base.m_lcodIRange;

    while(len>0){
        len--;
        register Ipp8u idx = ((codIRange >> 6) & 0x03);
        Ipp8u c = (code>>len)&1;
        if (c != (pStateIdx>>6) ){
            bs->m_base.m_bitOffset += bitcountLPS[pStateIdx][idx];
            codIRange = rangeLPS[pStateIdx][idx];
        }else{
            codIRange -= rangeTabLPS[pStateIdx][idx];  //MPS range
            register Ipp32s renorm = (codIRange>>8)^1;
            bs->m_base.m_bitOffset +=  renorm;
            codIRange <<= renorm;
        }
        pStateIdx = transTbl[c][pStateIdx];
    }

    bs->m_base.m_lcodIRange = codIRange;
    bs->m_base.context_array[ctxIdx].pStateIdx = pStateIdx;
#endif
}
#endif

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(
    void* state,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    Ipp32u codIRange = bs->m_base.m_lcodIRange;
    codIRange -=2;
    if (code){ //LPS
        codIOffset += (codIRange<<bs->m_base.m_nReadyBits);
        codIRange  = 2;
        if( codIOffset >= ENC_M_FULL ){
            codIOffset -= ENC_M_FULL;
            bs->m_base.m_nRegister++;
            while( bs->m_base.m_nOutstandingChunks > 0 ){
                H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0);
                bs->m_base.m_nOutstandingChunks--;
            }
        }
        bs->m_base.m_nReadyBits -= 7;
        codIRange <<= 7;
        if( bs->m_base.m_nReadyBits > 0 ){
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;
        }
    }else{
        if( codIRange >= ENC_M_QUARTER ){
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;
        }
        codIRange <<= 1;
        bs->m_base.m_nReadyBits--;
        if( bs->m_base.m_nReadyBits > 0 ){
            bs->m_base.m_lcodIOffset = codIOffset;
            bs->m_base.m_lcodIRange = codIRange;
            return;
        }
    }

    Ipp32s L = (codIOffset>>ENC_B_BITS)& ((1<<ENC_M_BITS)-1);
    codIOffset = (codIOffset<<ENC_M_BITS)&(ENC_M_FULL-1);
    if (L < ((1 << ENC_M_BITS) - 1))
    {
        while (bs->m_base.m_nOutstandingChunks > 0)
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0xffff);
            bs->m_base.m_nOutstandingChunks--;
        }
        H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, L);
    }
    else
    {
        bs->m_base.m_nOutstandingChunks++;
    }

    bs->m_base.m_nReadyBits += ENC_M_BITS;
    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
    return;
}

#else

void H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(
    void* state,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    Ipp32u codIRange = bs->m_base.m_lcodIRange;
    codIRange -=2;
    if (code)
    {
        codIOffset += codIRange ;
        codIRange  = 2;
    }

  /* renormalisation */
    while (codIRange  < ENC_QUARTER_RANGE)
    {
        if (codIOffset >= ENC_HALF_RANGE)
        {
            //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 1);
            H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(bs);
            codIOffset -= ENC_HALF_RANGE;
        }
        else if (codIOffset < ENC_QUARTER_RANGE)
        {
            //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 0);
            H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(bs);
        }
        else
        {
            bs->m_base.m_nOutstandingBits++;
            codIOffset -= ENC_QUARTER_RANGE;
        }

        codIOffset <<= 1;
        codIRange <<= 1;
    }

    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
    return;
}
#endif

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(
    void* state,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    Ipp32u codIRange = bs->m_base.m_lcodIRange;
    bs->m_base.m_nReadyBits--;

    if (code)
    { //LPS
        codIOffset  += (codIRange<<bs->m_base.m_nReadyBits);
        if (codIOffset >= ENC_M_FULL)
        {
            codIOffset -= ENC_M_FULL;
            bs->m_base.m_nRegister++;
            while (bs->m_base.m_nOutstandingChunks > 0)
            {
                H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0);
                bs->m_base.m_nOutstandingChunks--;
            }
        }
    }

    if (bs->m_base.m_nReadyBits == 0)
    {
        Ipp32s L = (codIOffset>>ENC_B_BITS)& ((1<<ENC_M_BITS)-1);
        codIOffset = (codIOffset<<ENC_M_BITS)&(ENC_M_FULL-1);
        if (L < ((1 << ENC_M_BITS) - 1))
        {
            while (bs->m_base.m_nOutstandingChunks > 0)
            {
                H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0xffff);
                bs->m_base.m_nOutstandingChunks--;
            }
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, L);
        }
        else
        {
            bs->m_base.m_nOutstandingChunks++;
        }

        bs->m_base.m_nReadyBits += ENC_M_BITS;
    }

    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
    return;
}

#else

void H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(
    void* state,
    Ipp32s code)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    Ipp32u codIRange = bs->m_base.m_lcodIRange;
    codIOffset *= 2;

    if (code)
        codIOffset += codIRange;

    if (codIOffset >= ENC_FULL_RANGE)
    {
        //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 1);
        H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(bs);
        codIOffset -= ENC_FULL_RANGE;
    }
    else if (codIOffset < ENC_HALF_RANGE)
    {
        H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(bs);
        //H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, 0);
    }
    else
    {
        bs->m_base.m_nOutstandingBits++;
        codIOffset -= ENC_HALF_RANGE;
    }

    bs->m_base.m_lcodIOffset = codIOffset;
    bs->m_base.m_lcodIRange = codIRange;
    return;
}
#endif

#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_TerminateEncode_CABAC)(
    void* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset =  bs->m_base.m_lcodIOffset;
    Ipp32s remBits = ENC_M_BITS -  bs->m_base.m_nReadyBits;
    Ipp8u  mask;

    if (remBits <= 5)
    {
        mask = (Ipp8u)(255 - ((1 << (6 - remBits)) - 1));
        codIOffset = (codIOffset >> (ENC_B_BITS + 8)) & mask;
        codIOffset += (1 << (5 - remBits));
        while (bs->m_base.m_nOutstandingChunks > 0)
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0xffff);
            bs->m_base.m_nOutstandingChunks--;
        }

        H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, codIOffset);
        //put buffer
    }
    else if (remBits <= 13)
    {
        Ipp32s L = (codIOffset >> (ENC_B_BITS + 8) & 0xff);
        while (bs->m_base.m_nOutstandingChunks > 0)
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0xffff);
            bs->m_base.m_nOutstandingChunks--;
        }

        H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, L);
        //put buffer
        if (remBits > 6)
        {
            mask = (Ipp8u)(255 - ((1 << (14 - remBits)) - 1));
            codIOffset = (codIOffset >> ENC_B_BITS) & mask;
            codIOffset += (1 << (13 - remBits));
            H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, codIOffset);
        }
        else
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, 128);
        }
    }
    else
    {
        Ipp32s L = (codIOffset >> (ENC_B_BITS) & ((1 << ENC_M_BITS) - 1));
        while (bs->m_base.m_nOutstandingChunks > 0)
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, 0xffff);
            bs->m_base.m_nOutstandingChunks--;
        }

        H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(state, L);
        //put buffer
        if (remBits > 14)
        {
            mask = (Ipp8u)(255 - ((1<<(22-remBits))-1));
            codIOffset = (codIOffset>>(ENC_B_BITS - 8)) & mask;
            codIOffset += (1<<(21-remBits));
            H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, codIOffset);
        }
        else
        {
            H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(state, 128);
        }
    }

    bs->m_base.m_nReadyBits = 8;
    bs->m_base.m_lcodIOffset = codIOffset;
}

#else

void H264ENC_MAKE_NAME(H264BsReal_TerminateEncode_CABAC)(
    void* state)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32u codIOffset = bs->m_base.m_lcodIOffset;
    H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(bs, (codIOffset >> (ENC_B_BITS-1)) & 1);
    H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(bs, (codIOffset >> (ENC_B_BITS - 2)) & 1);
    H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(bs, 1);
    //FlushBitStream_CABAC;
    while (bs->m_base.m_nReadyBits & 7)
    {
        bs->m_base.m_nRegister <<= 1;
        bs->m_base.m_nRegister |= (Ipp32s)0;
        bs->m_base.m_nReadyBits --;
    }

    switch (bs->m_base.m_nReadyBits >> 3)
    {
        case 0:
            *(bs->m_base.m_pbs) = (Ipp8u)((Ipp32u)bs->m_base.m_nRegister>>24);
            *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>16);
            *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister>>8);
            *(bs->m_base.m_pbs+3) = (Ipp8u)(bs->m_base.m_nRegister);
            bs->m_base.m_pbs += 4;
            break;
        case 1:
            *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>16);
            *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister>>8);
            *(bs->m_base.m_pbs+2) = (Ipp8u)(bs->m_base.m_nRegister);
            bs->m_base.m_pbs += 3;
            break;
        case 2:
            *(bs->m_base.m_pbs) = (Ipp8u)(bs->m_base.m_nRegister>>8);
            *(bs->m_base.m_pbs+1) = (Ipp8u)(bs->m_base.m_nRegister);
            bs->m_base.m_pbs += 2;
            break;
        case 3:
            *(bs->m_base.m_pbs++) = (Ipp8u)(bs->m_base.m_nRegister);
            break;
    }
    bs->m_base.m_nRegister = 0;
    bs->m_base.m_nReadyBits = 32;
    //H264ENC_MAKE_NAME(H264BsReal_ByteAlignWithZeros)(state);
}
#endif

void H264ENC_MAKE_NAME(H264BsFake_TerminateEncode_CABAC)(
    void* state)
{
    H264ENC_UNREFERENCED_PARAMETER(state);
}

void H264ENC_MAKE_NAME(H264BsFake_CopyContext_CABAC)(
    void* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    memcpy(&bs->m_base.context_array[0], &bstrm->context_array[0], 105*sizeof(CABAC_CONTEXT));
    memcpy(&bs->m_base.context_array[227], &bstrm->context_array[227], 49*sizeof(CABAC_CONTEXT));
    if (isFrame)
        memcpy(&bs->m_base.context_array[105], &bstrm->context_array[105], 122*sizeof(CABAC_CONTEXT));
    else
        memcpy(&bs->m_base.context_array[277], &bstrm->context_array[277], 122*sizeof(CABAC_CONTEXT));

    if (is8x8)
    {
        memcpy(&bs->m_base.context_array[426], &bstrm->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if (isFrame)
            memcpy(&bs->m_base.context_array[402], &bstrm->context_array[402], 24*sizeof(CABAC_CONTEXT));
        else
            memcpy(&bs->m_base.context_array[436], &bstrm->context_array[436], 24*sizeof(CABAC_CONTEXT));
    }

    bs->m_base.context_array[276] = bstrm->context_array[276]; //end of slice flag
    bs->m_base.context_array[399] = bstrm->context_array[399]; //transform 8x8 flag
    bs->m_base.context_array[400] = bstrm->context_array[400]; //transform 8x8 flag
    bs->m_base.context_array[401] = bstrm->context_array[401]; //transform 8x8 flag
    bs->m_base.m_lcodIRange = bstrm->m_lcodIRange;
}

void H264ENC_MAKE_NAME(H264BsReal_EncodeUnaryRepresentedSymbol_CABAC)(
    H264BsRealType* state,
    Ipp8u* ctx,
    Ipp32s ctxIdx,
    Ipp32s code,
    Ipp32s suppremum)
{
    if (code == 0)
    {
        H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx, 0);
    }
    else
    {
        H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx, 1);
        Ipp32s temp=code;
        while ((--temp) > 0)
            H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx + ctxIdx, 1);
        if (code < suppremum)
            H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx + ctxIdx, 0);
    }
}

void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedLevels_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    /* 2^(i+1)-2 except last=2^(i+1)-1 */
    static Ipp32u c[14] = {0,2,6,14,30,62,126,254,510,1022,2046,4094,8190,16383};

    if (code == 0)
    {
        H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx, 0);
    }
    else
    {
        if (code < 13)
        {
            H264ENC_MAKE_NAME(H264BsReal_EncodeBins_CABAC)(state, ctx, c[code], code + 1);
        }
        else
        {
            H264ENC_MAKE_NAME(H264BsReal_EncodeBins_CABAC)(state, ctx, c[13], 13);
            H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedSymbol_CABAC)(state, code - 13, 0);
        }
    }
}

void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedMVS_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    Ipp32s tempval, tempindex;
    Ipp32s bin = 1;
    Ipp32s inc;

    if (code == 0)
    {
        H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx, 0);
        return;
    }
    else
    {
        H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx, 1);
        tempval = code;
        tempindex = 1;
        inc = 1;
        while (((--tempval) > 0) && (++tempindex <= 8))
        {
            H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx + inc, 1);
            if ((++bin) == 2)
                inc++;
            if (bin == 3)
                inc++;
        }

        if (code < 8)
            H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(state, ctx + inc, 0);
        else
            H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedSymbol_CABAC)(state, code - 8, 3);
    }
}

void H264ENC_MAKE_NAME(H264BsFake_EncodeExGRepresentedMVS_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    if (code == 0)
    {
        H264ENC_MAKE_NAME(H264BsFake_EncodeSingleBin_CABAC)(state, ctx, 0);
        return;
    }
    else
    {
        Ipp32s tempval, tempindex;
        //Ipp32s bin=1;
        Ipp32s inc;
        H264ENC_MAKE_NAME(H264BsFake_EncodeSingleBin_CABAC)(state, ctx, 1);
        tempval = code;
        tempindex = 1;
        inc = 1;
        while (((--tempval) > 0) && (++tempindex <= 8))
        {
            H264ENC_MAKE_NAME(H264BsFake_EncodeSingleBin_CABAC)(state, ctx + inc, 1);
            if (inc < 3)
                inc++;
            //if ((++bin) == 2)
                //inc++;
            //if (bin == 3)
                //inc++;
        }

        if (code < 8)
        {
            H264ENC_MAKE_NAME(H264BsFake_EncodeSingleBin_CABAC)(state, ctx + inc, 0);
        }
        else
        {
            //H264ENC_MAKE_NAME(H264BsFake_EncodeExGRepresentedSymbol_CABAC)(state, code - 8, 3);
            if (code >= 65536 - 1 + 8)
            {
                bs->m_base.m_bitOffset += 256 * 32;
                code >>= 16;
            }

            if (code >= 256 - 1 + 8)
            {
                bs->m_base.m_bitOffset += 256 * 16;
                code >>= 8;
            }

            bs->m_base.m_bitOffset += bitcount_EG3[code];
        }
    }
}

void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedSymbol_CABAC)(
    void* state,
    Ipp32s code,
    Ipp32s log2ex)
{
    for (;;)
    {
        if (code>= (1<<log2ex))
        {
            H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(state, 1);
            code -= (1<<log2ex);
            log2ex++;
        }
        else
        {
            H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(state, 0);
            while (log2ex--)
                H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(state, (code >> log2ex) & 1);
            return;
        }
    }
}

void H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesIntra_CABAC)(
    void* state,
    Ipp32s SliceQPy)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32s l;

    // See subclause 9.3.1.1 of H.264 standard

    SliceQPy = Clip3(0, 51, SliceQPy);

    // Initialize context(s) for mb_type (SI & I slices)
    for (l = 0;l <= 10;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_0_10[l - 0].m,
                          M_and_N_for_ctxIdx_0_10[l - 0].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                        );
    };

    // Initialize context(s) for mb_qp_delta &
    // intra_chroma_pred_mode & prev_intra4x4_pred_mode_flag &
    // rem_intra4x4_pred_mode
    for (l = 60;l <= 69;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_60_69[l - 60].m,
                          M_and_N_for_ctxIdx_60_69[l - 60].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                        );
    };

    // Initialize context(s) for mb_field_decoding_flag &
    // coded_block_pattern(luma) & coded_block_pattern(chroma) &
    // coded_block_flag (SI & I slices)
    for (l = 70;l <= 104;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_70_104_intra[l - 70].m,
                          M_and_N_for_ctxIdx_70_104_intra[l - 70].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for significant_coeff_flag[] (frame coded)
    for (l = 105;l <= 165;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_105_165_intra[l - 105].m,
                          M_and_N_for_ctxIdx_105_165_intra[l - 105].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for last_significant_coeff_flag[] (frame coded)
    for (l = 166;l <= 226;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_166_226_intra[l - 166].m,
                          M_and_N_for_ctxIdx_166_226_intra[l - 166].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for coeff_abs_level_minus1[]
    for (l = 227;l <= 275;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_227_275_intra[l - 227].m,
                          M_and_N_for_ctxIdx_227_275_intra[l - 227].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // ctxIdx equal to 276 is associated the end_of_slice_flag
    // Initial values associated with ctxIdx equal to 276
    // are specified to be pStateIdx = 63 and valMPS = 0
    bs->m_base.context_array[276] = 63;
    //context_array[276].valMPS = 0;

    // Initalize context(s) for significant_coeff_flag[] (field coded)
    for (l = 277;l <= 337;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_277_337_intra[l - 277].m,
                          M_and_N_for_ctxIdx_277_337_intra[l - 277].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for last_significant_coeff_flag[] (field coded)
    for (l = 338;l <= 398;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_338_398_intra[l - 338].m,
                          M_and_N_for_ctxIdx_338_398_intra[l - 338].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    for (l = 399;l <= 401;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_399_401_intra[l - 399].m,
                          M_and_N_for_ctxIdx_399_401_intra[l - 399].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };
    for (l = 402;l <= 459;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_402_459_intra[l - 402].m,
                          M_and_N_for_ctxIdx_402_459_intra[l - 402].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    H264ENC_MAKE_NAME(H264BsReal_ResetBitStream_CABAC)(state);
} //void H264Bitstream::InitializeContextVariablesIntra_CABAC(Ipp32s SliceQPy)

// ---------------------------------------------------------------------------

void H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesInter_CABAC)(
    void* state,
    Ipp32s SliceQPy,
    Ipp32s cabac_init_idc)
{
    H264BsRealType* bs = (H264BsRealType *)state;
    Ipp32s l;

    // See subclause 9.3.1.1 of H.264 standard

    SliceQPy = Clip3(0, 51, SliceQPy);
    // Initialize context(s) for mb_skip_flag & mb_type (P & SP slices)
    // & sub_mb_pred (P & SP slices)
    for (l = 11;l <= 23;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_11_23[l - 11][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_11_23[l - 11][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initialize context(s) for mb_skip_flag & mb_type (B slices)
    // & sub_mb_pred (B slices)
    for (l = 24;l <= 39;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_24_39[l - 24][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_24_39[l - 24][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initialize context(s) for mvd_10 & mvd_11
    for (l = 40;l <= 53;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_40_53[l - 40][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_40_53[l - 40][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initialize context(s) for ref_idx_10 & ref_idx_11
    for (l = 54;l <= 59;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_54_59[l - 54][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_54_59[l - 54][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initialize context(s) for mb_qp_delta &
    // intra_chroma_pred_mode & prev_intra4x4_pred_mode_flag &
    // rem_intra4x4_pred_mode
    for (l = 60;l <= 69;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_60_69[l - 60].m,
                          M_and_N_for_ctxIdx_60_69[l - 60].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for mb_field_decoding_flag &
    // coded_block_pattern(luma) & coded_block_pattern(chroma) &
    // coded_block_flag (P, SP & B slices)
    for (l = 70;l <= 104;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_70_104_inter[l - 70][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_70_104_inter[l - 70][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for significant_coeff_flag[] (frame coded)
    for (l = 105;l <= 165;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_105_165_inter[l - 105][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_105_165_inter[l - 105][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    }

    // Initalize context(s) for last_significant_coeff_flag[] (frame coded)
    for (l = 166;l <= 226;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_166_226_inter[l - 166][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_166_226_inter[l - 166][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for coeff_abs_level_minus1[]
    for (l = 227;l <= 275;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_227_275_inter[l - 227][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_227_275_inter[l - 227][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for significant_coeff_flag[] (field coded)
    for (l = 277;l <= 337;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_277_337_inter[l - 277][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_277_337_inter[l - 277][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    // Initalize context(s) for last_significant_coeff_flag[] (field coded)
    for (l = 338;l <= 398;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_338_398_inter[l - 338][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_338_398_inter[l - 338][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    for (l = 399;l <= 401;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_399_401_inter[l - 399][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_399_401_inter[l - 399][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };
    for (l = 402;l <= 459;l += 1)
    {
        InitializeContext(&(bs->m_base.context_array[l]),
                          M_and_N_for_ctxIdx_402_459_inter[l - 402][cabac_init_idc].m,
                          M_and_N_for_ctxIdx_402_459_inter[l - 402][cabac_init_idc].n,
                          SliceQPy
#ifdef STORE_CABAC_BITS
#ifdef CABAC_CONTEXTS_COMP
                         ,l
#endif
#endif
                         );
    };

    H264ENC_MAKE_NAME(H264BsReal_ResetBitStream_CABAC)(state);
} //void H264Bitstream::InitializeContextVariablesInter_CABAC(Ipp32s SliceQPy, Ipp32s cabac_init_idc)

// ---------------------------------------------------------------------------


#undef H264BsFakeType
#undef H264BsBaseType
#undef H264BsType
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
