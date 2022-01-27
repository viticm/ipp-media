//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef UMC_BASE_BITSTREAM_H__
#define UMC_BASE_BITSTREAM_H__

#include "umc_h264_defs.h"
#include "umc_h264_tables.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#if (_MSC_FULL_VER >= 140050110)
//#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif
#endif

//Fast CABAC implementation
//#define CABAC_FAST
#undef CABAC_FAST

// ---------------------------------------------------------------------------
//  CBaseBitstream - bitstream base class
// ---------------------------------------------------------------------------
namespace UMC_H264_ENCODER
{
  extern const Ipp16u transIdxMPS[64];
  extern const Ipp16u transIdxLPS[64];
  extern const Ipp8u  rangeTabLPS[128][4];
  extern const Ipp8u renormTAB[32];

// This macro is used to convert a Ipp32s VLC code into an
// Ipp32u VLC code, ready to pipe into PutVLCCode below.
// This saves having another function for that purpose and
// should be faster.
#define SIGNED_VLC_CODE(code) (2*ABS(code) - (code > 0))

typedef Ipp8u CABAC_CONTEXT;

const Ipp32s MB_TRANSFORM_SIZE_8X8_FLAG = 399; // ctxIdxOffset for transform_size_8x8_flag

enum // Syntax element type
{
    MB_SKIP_FLAG_P_SP               = 0,
    MB_SKIP_FLAG_B                  = 1,
    MB_FIELD_DECODING_FLAG          = 2,
    MB_TYPE_SI                      = 3,
    MB_TYPE_I                       = 4,
    MB_TYPE_P_SP                    = 5,
    MB_TYPE_B                       = 6,
    CODED_BLOCK_PATTERN_LUMA        = 7,
    CODED_BLOCK_PATTERN_CHROMA      = 8,
    MB_QP_DELTA                     = 9,
    PREV_INTRA4X4_PRED_MODE_FLAG    = 10,
    REM_INTRA4X4_PRED_MODE          = 11,
    INTRA_CHROMA_PRED_MODE          = 12,
    REF_IDX_L0                      = 13,
    REF_IDX_L1                      = 14,
    MVD_L0_0                        = 15,
    MVD_L1_0                        = 16,
    MVD_L0_1                        = 17,
    MVD_L1_1                        = 18,
    SUB_MB_TYPE_P_SP                = 19,
    SUB_MB_TYPE_B                   = 20,

    MAIN_SYNTAX_ELEMENT_NUMBER
};

// See table 9-30 of H.264 standard
enum // Syntax element type
{
    CODED_BLOCK_FLAG                = 0,
    SIGNIFICANT_COEFF_FLAG          = 1,
    LAST_SIGNIFICANT_COEFF_FLAG     = 2,
    COEFF_ABS_LEVEL_MINUS1          = 3,

    SYNTAX_ELEMENT_NUMBER
};

// See table 9-32 of H.264 standard
enum // Context block category
{
    BLOCK_LUMA_DC_LEVELS            = 0,
    BLOCK_LUMA_AC_LEVELS            = 1,
    BLOCK_LUMA_LEVELS               = 2,
    BLOCK_CHROMA_DC_LEVELS          = 3,
    BLOCK_CHROMA_AC_LEVELS          = 4,
    BLOCK_LUMA_64_LEVELS            = 5,

    BLOCK_CATEGORY_NUMBER
};
// See table 9-11 of H.264 standard
const Ipp32s ctxIdxOffset[MAIN_SYNTAX_ELEMENT_NUMBER] =
{
    11,
    24,
    70,
    0,
    3,
    14,
    27,
    73,
    77,
    60,
    68,
    69,
    64,
    54,
    54,
    40,
    40,
    47,
    47,
    21,
    36,
};

// See table 9-24 of H.264 standard
const Ipp32s ctxIdxOffsetFrameCoded[SYNTAX_ELEMENT_NUMBER] =
{
    85,
    105,
    166,
    227,

};

// See table 9-24 of H.264 standard
const Ipp32s ctxIdxOffsetFieldCoded[SYNTAX_ELEMENT_NUMBER] =
{
    85,
    277,
    338,
    227,
};

// See table 9-24 of H.264 standard
const Ipp32s ctxIdxOffsetFrameCoded_BlockCat_5[SYNTAX_ELEMENT_NUMBER] =
{
    0xffffffff, // na
    402,
    417,
    426
};

// See table 9-24 of H.264 standard
const Ipp32s ctxIdxOffsetFieldCoded_BlockCat_5[SYNTAX_ELEMENT_NUMBER] =
{
    0xffffffff, // na
    436,
    451,
    426
};

// See table 9-30 of H.264 standard
const Ipp32s ctxIdxBlockCatOffset[SYNTAX_ELEMENT_NUMBER][BLOCK_CATEGORY_NUMBER] =
{
    {0,  4,  8, 12, 16, -1},
    {0, 15, 29, 44, 47,  0},
    {0, 15, 29, 44, 47,  0},
    {0, 10, 20, 30, 39,  0}
};

// See table 9-34 of H.264 standard
extern const Ipp8s Table_9_34[3][64];
extern const Ipp32s p_bits[128];
extern const Ipp32u bitcount_EG0[268];
extern const Ipp32u bitcount_EG3[263];
extern const Ipp32s pref_bits[128][16];
extern __ALIGN8 const Ipp8u pref_state[128][16];

// LossRecovery return status
typedef enum
{
    BSLR_STATUS_ERROR,  // Loss recovery did not detect lost packet, or couldn't recover.
    BSLR_STATUS_OK,     // Loss recovery detected lost packet and recovered
    BSLR_STATUS_EOS     // Loss recovery detected lost packet but end of
                        // stream encountered (i.e. no more packet avail.)
} BSLR_Status;

typedef struct sH264BaseBs
{
//protected:
    Ipp8u* m_pbs;  // m_pbs points to the current position of the buffer.
    Ipp8u* m_pbsBase; // m_pbsBase points to the first byte of the buffer.
    Ipp32u m_bitOffset; // Indicates the bit position (0 to 7) in the byte pointed by m_pbs.
    Ipp32u m_maxBsSize; // Maximum buffer size in bytes.};

//private:
    Ipp8u* m_pbsRBSPBase;  // Points to beginning of previous "Raw Byte Sequence Payload"
    Ipp32s num8x8Cshift2;

//public:
    CABAC_CONTEXT context_array[460];                       // (CABAC_CONTEXT []) array of cabac context(s)
    Ipp32u m_lcodIRange;                                    // arithmetic encoding engine variable
    Ipp32u m_lcodIOffset;                                   // arithmetic encoding engine variable
    Ipp32u m_nRegister;
#ifdef CABAC_FAST
    Ipp32s m_nReadyBits;
    Ipp32s m_nReadyBytes;
    Ipp32u m_nOutstandingChunks;
#else
    Ipp32u m_nReadyBits;
    Ipp32u m_nOutstandingBits;
#endif

} H264BsBase;

// Returns the bit position of the buffer pointer relative to the beginning of the buffer.
inline
Ipp32u H264BsBase_GetBsOffset(
    H264BsBase* state)
{
    if (state->m_pbsBase == NULL)
        return (state->m_bitOffset + 128) >> 8;
    else
        return(Ipp32u(state->m_pbs - state->m_pbsBase) * 8 + state->m_bitOffset);
}

// Returns the size of the bitstream data in bytes based on the current position of the buffer pointer, m_pbs.
inline
Ipp32u H264BsBase_GetBsSize(
    H264BsBase* state)
{
    //Ipp32u size;
    //size = Ipp32u(state->m_pbs - state->m_pbsBase) + 1;
    //return(!state->m_bitOffset ? (size - 1) : size);
    return (H264BsBase_GetBsOffset(state) + 7) >> 3;
}

// Returns the base pointer to the beginning of the bitstream.
inline
Ipp8u* H264BsBase_GetBsBase(
    H264BsBase* state)
{
    return state->m_pbsBase;
}

// Returns the maximum bitstream size.
inline
Ipp32u H264BsBase_GetMaxBsSize(
    H264BsBase* state)
{
    return state->m_maxBsSize;
}

// Checks if read/write passed the maximum buffer size.
inline
bool H264BsBase_CheckBsLimit(
    H264BsBase* state)
{
    Ipp32u size;
    size = H264BsBase_GetBsSize(state);
    if (size > state->m_maxBsSize)
        return false;
    return true;
}

// Assigns new position to the buffer pointer.
inline
void H264BsBase_SetState(
    H264BsBase* state,
    Ipp8u* const pbs,
    const Ipp32u bitOffset)
{
    state->m_pbs       = pbs;
    state->m_bitOffset = bitOffset;
}

// Obtains current position of the buffer pointer.
inline
void H264BsBase_GetState(
    H264BsBase* state,
    Ipp8u** pbs,
    Ipp32u* bitOffset)
{
    *pbs       = state->m_pbs;
    *bitOffset = state->m_bitOffset;
}

// Advances buffer pointer with given number of bits.
inline
void H264BsBase_UpdateState(
    H264BsBase* state,
    const Ipp32u nbits)
{
    state->m_pbs      += (nbits + state->m_bitOffset) >> 3;
    state->m_bitOffset = (nbits + state->m_bitOffset) & 0x7;
}

// Clears the bitstream buffer.
inline
void H264BsBase_ClearBs(
    H264BsBase* state)
{
    memset(state->m_pbsBase, 0, state->m_maxBsSize);
}

// Resets pointer to the beginning and clears the bitstream buffer.
inline
void H264BsBase_Reset(
    H264BsBase* state)
{
    state->m_pbs = state->m_pbsBase;
    state->m_bitOffset = 0;
    H264BsBase_ClearBs(state);
}

// Write RBSP Trailing Bits to Byte Align
inline
void H264BsBase_WriteTrailingBits(
    H264BsBase* state)
{
    // Write Stop Bit
    state->m_pbs[0] = (Ipp8u)(state->m_pbs[0] | (Ipp8u)(0x01 << (7 - state->m_bitOffset)));

    state->m_pbs++;
    state->m_pbs[0] = 0;
    state->m_bitOffset = 0;
}

// Add zero bits to byte-align the buffer.
inline
void H264BsBase_ByteAlignWithZeros(
    H264BsBase* state)
{
    // No action needed if already byte aligned, i.e. !m_bitOffset
    if (state->m_bitOffset){ // note that prior write operation automatically clears the unused bits in the current byte*/
        state->m_pbs++;
        state->m_bitOffset = 0;
    }
}

// Add one bits to byte-align the buffer.
inline
void H264BsBase_ByteAlignWithOnes(
    H264BsBase* state)
{
    if (state->m_bitOffset){
        // No action needed if already byte aligned, i.e. !m_bitOffset
        state->m_pbs[0] = (Ipp8u)(state->m_pbs[0] | (Ipp8u)(0xff >> state->m_bitOffset));
        state->m_pbs++;
        state->m_pbs[0] = 0;
        state->m_bitOffset = 0;
    }
}

inline
Ipp32u H264BsBase_GetBsOffset_CABAC(
    H264BsBase* state)
{
#ifdef CABAC_FAST
    return Ipp32u(state->m_pbs - state->m_pbsBase) * 8 +
        state->m_bitOffset + state->m_nReadyBytes * 8 +
        state->m_nOutstandingChunks * 16 + 16 - state->m_nReadyBits;
#else
    return H264BsBase_GetBsOffset(state) + 32 - state->m_nReadyBits + state->m_nOutstandingBits;
#endif
}

void H264BsBase_CopyContextCABAC_I4x4(
    H264BsBase* state,
    H264BsBase* bstrm,
    Ipp32s isFrame);

void H264BsBase_CopyContextCABAC_I8x8(
    H264BsBase* state,
    H264BsBase* bstrm,
    Ipp32s isFrame);

void H264BsBase_CopyContextCABAC_I16x16(
    H264BsBase* state,
    H264BsBase* bstrm,
    Ipp32s isFrame);

void H264BsBase_CopyContextCABAC_InterP(
    H264BsBase* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8);

void H264BsBase_CopyContextCABAC_InterB(
    H264BsBase* state,
    H264BsBase* bstrm,
    Ipp32s isFrame,
    Ipp32s is8x8);

void H264BsBase_CopyContextCABAC_Chroma(
    H264BsBase* state,
    H264BsBase* bstrm,
    int isFrame);

#define PIXBITS 8
#include "umc_base_bitstream_tmpl.h"
#undef PIXBITS

#if defined (BITDEPTH_9_12)

#define PIXBITS 16
#include "umc_base_bitstream_tmpl.h"
#undef PIXBITS

#endif // BITDEPTH_9_12


} //namespace UMC_H264_ENCODER

#endif // UMC_BASE_BITSTREAM_H__
#endif //UMC_ENABLE_H264_VIDEO_ENCODER
