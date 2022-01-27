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

#define H264BsRealType H264ENC_MAKE_NAME(H264BsReal)
#define sH264BsRealType H264ENC_MAKE_NAME(sH264BsReal)
#define H264BsFakeType H264ENC_MAKE_NAME(H264BsFake)
#define sH264BsFakeType H264ENC_MAKE_NAME(sH264BsFake)
#define H264SliceType H264ENC_MAKE_NAME(H264Slice)
#define sH264SliceType H264ENC_MAKE_NAME(sH264Slice)

typedef struct sH264SliceType H264SliceType;

typedef struct sH264BsRealType
{
    H264BsBase m_base;

//protected:
    CABAC_CONTEXT context_array_copy[460];                       // (CABAC_CONTEXT []) array of cabac context(s)
    Ipp8u* m_pbs_copy;
    Ipp32u m_bitOffset_copy;

    Ipp32u m_lcodIRange_copy;                                    // arithmetic encoding engine variable
    Ipp32u m_lcodIOffset_copy;                                   // arithmetic encoding engine variable
    Ipp32u m_nRegister_copy;
#ifdef CABAC_FAST
    Ipp32s m_nReadyBits_copy;
    Ipp32s m_nReadyBytes_copy;
    Ipp32u m_nOutstandingChunks_copy;
#else
    Ipp32u m_nReadyBits_copy;
    Ipp32u m_nOutstandingBits_copy;
#endif

//private:
    Ipp8u* m_pbsRBSPBase;  // Points to beginning of previous "Raw Byte Sequence Payload"
    Ipp32s num8x8Cshift2;

} H264BsRealType;

Status H264ENC_MAKE_NAME(H264BsReal_Create)(
    H264BsRealType* state);

Status H264ENC_MAKE_NAME(H264BsReal_Create)(
    H264BsRealType* state,
    Ipp8u* const pb,
    const Ipp32u maxsize,
    Ipp32s chroma_format_idc,
    Status &plr);

void H264ENC_MAKE_NAME(H264BsReal_Destroy)(
    H264BsRealType* state);

void H264ENC_MAKE_NAME(H264BsReal_SaveCABACState)(
    void* state);

void H264ENC_MAKE_NAME(H264BsReal_RestoreCABACState)(
    void* state);

void H264ENC_MAKE_NAME(H264BsReal_CopyContext_CABAC)(
    void* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8);

void H264ENC_MAKE_NAME(H264BsReal_ResetBitStream_CABAC)(
    void* state);

void H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesIntra_CABAC)(
    void* state,
    Ipp32s SliceQPy);

void H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesInter_CABAC)(
    void* state,
    Ipp32s SliceQPy,
    Ipp32s cabac_init_idc);

// Appends bits into the bitstream buffer.
#ifdef CABAC_FAST
void H264ENC_MAKE_NAME(H264BsReal_WriteOneByte_CABAC)(
    void* state,
    Ipp32u b);

void H264ENC_MAKE_NAME(H264BsReal_WriteTwoBytes_CABAC)(
    void* state,
    Ipp32u b);

#else

void H264ENC_MAKE_NAME(H264BsReal_WriteBit_CABAC)(
    H264BsRealType* state,
    bool code);

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingZeroBit_CABAC)(
    H264BsRealType* state);

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingOneBit_CABAC)(
    H264BsRealType* state);

void H264ENC_MAKE_NAME(H264BsReal_WriteOutstandingBit_CABAC)(
    H264BsRealType* state,
    bool code);

#endif

void H264ENC_MAKE_NAME(H264BsReal_TerminateEncode_CABAC)(
    void* state);

// Encode single bin from stream
void H264ENC_MAKE_NAME(H264BsReal_EncodeSingleBin_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code);

void H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(
    void* state,
    Ipp32s code);

void H264ENC_MAKE_NAME(H264BsReal_EncodeBins_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32u code,
    Ipp32s len);

void H264ENC_MAKE_NAME(H264BsReal_EncodeBypass_CABAC)(
    void* state,
    Ipp32s code);

// Appends bits into the bitstream buffer.
void H264ENC_MAKE_NAME(H264BsReal_PutBit)(
    void* state,
    Ipp32u code);

void H264ENC_MAKE_NAME(H264BsReal_PutBits)(
    void* state,
    Ipp32u code,
    Ipp32u length);

void H264ENC_MAKE_NAME(H264BsReal_PutVLCBits)(
    H264BsRealType* state,
    const Ipp32u val,
    const Ipp32u len); // Writes one general VLC code to the bitstream

Ipp32u H264ENC_MAKE_NAME(H264BsReal_PutVLCCode)(
    void* state,
    const Ipp32u code);// Writes one general VLC code to the bitstream without knowing the code length... Returns the length of the code written.

//unary binarization
void H264ENC_MAKE_NAME(H264BsReal_EncodeUnaryRepresentedSymbol_CABAC)(
    H264BsRealType* state,
    Ipp8u* ctx,
    Ipp32s ctxIdx,
    Ipp32s code,
    Ipp32s suppremum/* = 0x7fffffff*/);

//Exp Golomb binarization
void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedLevels_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code);

void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedMVS_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code);

void H264ENC_MAKE_NAME(H264BsReal_Reset)(
    void* state);

void H264ENC_MAKE_NAME(H264BsReal_EncodeExGRepresentedSymbol_CABAC)(
    void* state,
    Ipp32s code,
    Ipp32s log2ex);


typedef struct sH264BsFakeType
{
    H264BsBase m_base;

//private:
    Ipp8u* m_pbsRBSPBase;  // Points to beginning of previous "Raw Byte Sequence Payload"
    Ipp32s num8x8Cshift2;

} H264BsFakeType;

Status H264ENC_MAKE_NAME(H264BsFake_Create)(
    H264BsFakeType* state);

void H264ENC_MAKE_NAME(H264BsFake_Destroy)(
    H264BsFakeType* state);

Status H264ENC_MAKE_NAME(H264BsFake_Create)(
    H264BsFakeType* state,
    Ipp8u* const pb,
    const Ipp32u maxsize,
    Ipp32s chroma_format_idc,
    Status &plr);

void H264ENC_MAKE_NAME(H264BsFake_TerminateEncode_CABAC)(
    void* state);

void H264ENC_MAKE_NAME(H264BsFake_CopyContext_CABAC)(
    void* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8);

inline
void H264ENC_MAKE_NAME(H264BsFake_SaveCABACState)(
    void* /*state*/)
{
}

inline
void H264ENC_MAKE_NAME(H264BsFake_RestoreCABACState)(
    void* /*state*/)
{
}

inline
void H264ENC_MAKE_NAME(H264BsFake_PutBit)(
    void* state,
    Ipp32u code)
{
    H264ENC_UNREFERENCED_PARAMETER(code);
    H264BsFakeType* bs = (H264BsFakeType *)state;
    bs->m_base.m_bitOffset += 256;
}

inline
void H264ENC_MAKE_NAME(H264BsFake_PutBits)(
    void* state,
    Ipp32u code,
    Ipp32u length)
{
    H264ENC_UNREFERENCED_PARAMETER(code);
    H264BsFakeType* bs = (H264BsFakeType *)state;
    bs->m_base.m_bitOffset += length << 8;
}

inline
void H264ENC_MAKE_NAME(H264BsFake_PutVLCBits)(
    H264BsFakeType* state,
    const Ipp32u val,
    const Ipp32u len)
{
    H264ENC_UNREFERENCED_PARAMETER(val);
    H264BsFakeType* bs = (H264BsFakeType *)state;
    bs->m_base.m_bitOffset += ((((len - 1) & (~1)) + 1) << 8);
}

inline
Ipp32u H264ENC_MAKE_NAME(H264BsFake_PutVLCCode)(
    void* state,
    const Ipp32u val)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    VM_ASSERT(val != 0);
    register Ipp32s NN = val + 1;
    register Ipp32s i = 0;
#if defined(__i386__) && defined(__GNUC__) && (__GNUC__ > 3) && !defined(__INTEL_COMPILER)
    i = 31 - __builtin_clz(NN);
#elif defined(__INTEL_COMPILER) && (defined(__i386__) || defined(WIN32)) && !defined(WIN64)
    i = _bit_scan_reverse(NN);
#elif defined(_MSC_VER) && (_MSC_FULL_VER >= 140050110) && !defined(__INTEL_COMPILER)
    unsigned long idx;
    _BitScanReverse(&idx, (unsigned long)NN);
    i = (Ipp32s)idx;
#else
    if (NN & 0xffff0000) { i += 16; NN >>= 16; }
    if (NN & 0xff00) { i += 8; NN >>= 8; }
    if (NN & 0xf0) { i += 4; NN >>= 4; }
    if (NN & 0xc) { i += 2; NN >>= 2; }
    if (NN & 0x2) { i += 1; NN >>= 1; }
#endif

    register Ipp32u code_length;
    //code_length = (1 + (i << 1)) & (~1) + 1;
    code_length = 1 + (i << 1);
    bs->m_base.m_bitOffset += (code_length << 8);
    return code_length;
}

inline
void H264ENC_MAKE_NAME(H264BsFake_EncodeBypass_CABAC)(
    void* state,
    Ipp32s code)
{
    H264ENC_UNREFERENCED_PARAMETER(code);
    H264BsFakeType* bs = (H264BsFakeType *)state;
    bs->m_base.m_bitOffset += 256;
}

inline
void H264ENC_MAKE_NAME(H264BsFake_EncodeSingleBin_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    register Ipp8u pStateIdx = *ctx;
    //bs->m_base.m_bitOffset += ( code ? p_bits[pStateIdx^64] : p_bits[pStateIdx] );
    bs->m_base.m_bitOffset += p_bits[pStateIdx ^ (code << 6)];
    *ctx = transTbl[code][pStateIdx];
}

void H264ENC_MAKE_NAME(H264BsFake_EncodeBins_CABAC)(
    H264BsFakeType* state,
    Ipp8u* ctx,
    Ipp32u code,
    Ipp32s len);

inline
void H264ENC_MAKE_NAME(H264BsFake_EncodeFinalSingleBin_CABAC)(
    void* state,
    Ipp32s code)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    if (code)
        bs->m_base.m_bitOffset += 7 << 8;
    else
        bs->m_base.m_bitOffset += p_bits[64];
}

//Exp Golomb binarization
inline
void H264ENC_MAKE_NAME(H264BsFake_EncodeExGRepresentedLevels_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    if (code < 13)
    {
        bs->m_base.m_bitOffset += pref_bits[*ctx][code];
        *ctx = pref_state[*ctx][code];
    }
    else
    {
        bs->m_base.m_bitOffset  += pref_bits[*ctx][13];
        *ctx = pref_state[*ctx][13];
        if (code >= 65536 - 1 + 13)
        {
            bs->m_base.m_bitOffset += 256 * 32;
            code >>= 16;
        }

        if (code >= 256 - 1 + 13)
        {
            bs->m_base.m_bitOffset += 256 * 16;
            code >>= 8;
        }

        bs->m_base.m_bitOffset += bitcount_EG0[code];
    }
}

void H264ENC_MAKE_NAME(H264BsFake_EncodeExGRepresentedMVS_CABAC)(
    void* state,
    Ipp8u* ctx,
    Ipp32s code);

inline
Ipp32u H264ENC_MAKE_NAME(H264BsFake_GetBsOffset)(
    H264BsFakeType* state)
{
    H264BsFakeType* bs = (H264BsFakeType *)state;
    return (bs->m_base.m_bitOffset + 128) >> 8;
}

Ipp32u H264ENC_MAKE_NAME(H264BsCommon_EndOfNAL)(
    void* state,
    Ipp8u* const pout,
    Ipp8u const uIDC,
    NAL_Unit_Type const uUnitType,
    bool& startPicture);

#undef sH264SliceType
#undef H264SliceType
#undef sH264BsFakeType
#undef H264BsFakeType
#undef sH264BsRealType
#undef H264BsRealType

#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
