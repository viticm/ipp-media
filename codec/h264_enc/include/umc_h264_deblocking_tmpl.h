/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2003-2008 Intel Corporation. All Rights Reserved.
//
//
*/

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

#define DeblockingParametersType H264ENC_MAKE_NAME(DeblockingParameters)
#define DeblockingParametersMBAFFType H264ENC_MAKE_NAME(DeblockingParametersMBAFF)

#pragma pack(16)

typedef struct H264ENC_MAKE_NAME(sDeblockingParameters)
{
    Ipp8u    Strength[NUMBER_OF_DIRECTION][16];                   // (PixType [][]) arrays of deblocking sthrengths
    Ipp32u   DeblockingFlag[NUMBER_OF_DIRECTION];                 // (Ipp32s []) flags to do deblocking
    Ipp32u   ExternalEdgeFlag[NUMBER_OF_DIRECTION];               // (Ipp32s []) flags to do deblocking on external edges
    Ipp32s   nMBAddr;                                             // (Ipp32s) macroblock number
    Ipp32s   nMaxMVector;                                         // (Ipp32s) maximum vertical motion vector
    Ipp32s   nNeighbour[NUMBER_OF_DIRECTION];                     // (Ipp32s) neighbour macroblock addres
    Ipp32s   MBFieldCoded;                                        // (Ipp32s) flag means macroblock is field coded (picture may not)
    Ipp32s   nAlphaC0Offset;                                      // (Ipp32s) alpha c0 offset
    Ipp32s   nBetaOffset;                                         // (Ipp32s) beta offset
    PIXTYPE *pY;                                                  // (PixType *) pointer to Y data
    PIXTYPE *pU;                                                  // (PixType *) pointer to U data
    PIXTYPE *pV;                                                  // (PixType *) pointer to V data
    Ipp32s   pitchPixels;                                         // (Ipp32s) working pitch in pixels
} DeblockingParametersType;

typedef struct H264ENC_MAKE_NAME(sDeblockingParametersMBAFF)
{
    DeblockingParametersType m_base;
    Ipp8u  StrengthComplex[16];                                 // (Ipp8u) arrays of deblocking sthrengths
    Ipp8u  StrengthExtra[16];                                   // (Ipp8u) arrays of deblocking sthrengths
    Ipp32s UseComplexVerticalDeblocking;                        // (Ipp32u) flag to do complex deblocking on external vertical edge
    Ipp32s ExtraHorizontalEdge;                                 // (Ipp32u) flag to do deblocking on extra horizontal edge
    Ipp32s nLeft[2];                                            // (Ipp32u []) left couple macroblock numbers
} DeblockingParametersMBAFFType;

inline
Ipp8u H264ENC_MAKE_NAME(getEncoderBethaTable)(
    Ipp32s index)
{
    return(ENCODER_BETA_TABLE_8u[index]);
}

inline
Ipp8u H264ENC_MAKE_NAME(getEncoderAlphaTable)(
    Ipp32s index)
{
    return(ENCODER_ALPHA_TABLE_8u[index]);
}

inline
Ipp8u* H264ENC_MAKE_NAME(getEncoderClipTab)(
    Ipp32s index)
{
    return(ENCODER_CLIP_TAB_8u[index]);
}

#pragma pack()

#undef DeblockingParametersType
#undef DeblockingParametersMBAFFType
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
