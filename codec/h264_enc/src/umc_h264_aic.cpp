//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include <string.h>
#include <math.h>
#include "vm_debug.h"
#include "umc_h264_video_encoder.h"
#include "umc_h264_tables.h"
#include "umc_h264_bme.h"

////////////////////////////////////////////////////////////////////////////////
// Uncomment the following #define to remove the left MB dependency in the
// code to select the 4x4 Intra mode for a given block.

// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..L, and from above left M, as follows:
//
//  M A B C D E F G H
//  I a b c d
//  J e f g h
//  K i j k l
//  L m n o p

// Predictor array index definitions
#define P_Z PredPel[0]
#define P_A PredPel[1]
#define P_B PredPel[2]
#define P_C PredPel[3]
#define P_D PredPel[4]
#define P_E PredPel[5]
#define P_F PredPel[6]
#define P_G PredPel[7]
#define P_H PredPel[8]
#define P_I PredPel[9]
#define P_J PredPel[10]
#define P_K PredPel[11]
#define P_L PredPel[12]
#define P_M PredPel[13]
#define P_N PredPel[14]
#define P_O PredPel[15]
#define P_P PredPel[16]
#define P_Q PredPel[17]
#define P_R PredPel[18]
#define P_S PredPel[19]
#define P_T PredPel[20]
#define P_U PredPel[21]
#define P_V PredPel[22]
#define P_W PredPel[23]
#define P_X PredPel[24]

// Predicted pixel array offset macros
#define P_a pPredBuf[0]
#define P_b pPredBuf[1]
#define P_c pPredBuf[2]
#define P_d pPredBuf[3]
#define P_e pPredBuf[0+1*16]
#define P_f pPredBuf[1+1*16]
#define P_g pPredBuf[2+1*16]
#define P_h pPredBuf[3+1*16]
#define P_i pPredBuf[0+2*16]
#define P_j pPredBuf[1+2*16]
#define P_k pPredBuf[2+2*16]
#define P_l pPredBuf[3+2*16]
#define P_m pPredBuf[0+3*16]
#define P_n pPredBuf[1+3*16]
#define P_o pPredBuf[2+3*16]
#define P_p pPredBuf[3+3*16]

namespace UMC_H264_ENCODER
{

#define NUM_AI_MODES 9

Ipp32s intra_modes[4][8] = {
    { 0, 1, 3, 7, 8,-1,-1,-1},
    { 0, 1, 3, 4, 5, 6, 7, 8},
    { 0, 3, 7,-1,-1,-1,-1,-1},
    { 1, 8,-1,-1,-1,-1,-1,-1}
};

static const Ipp8u uBlockURPredOK[] = {
    // luma
    0xff, 0xff, 0xff, 0,
    0xff, 0xff, 0xff, 0,
    0xff, 0xff, 0xff, 0,
    0xff, 0,    0xff, 0
};

typedef enum {
    PRED16x16_VERT   = 0,
    PRED16x16_HORZ   = 1,
    PRED16x16_DC     = 2,
    PRED16x16_PLANAR = 3
} Enum16x16PredType;

typedef enum {
    PRED8x8_DC     = 0,
    PRED8x8_HORZ   = 1,
    PRED8x8_VERT   = 2,
    PRED8x8_PLANAR = 3
} Enum8x8PredType;

typedef enum {
    LEFT_AVAILABLE = 1, // Pixels to the left from the MB are available.
    TOP_AVAILABLE  = 2  // Pixels above the MB are available.
} PixelsAvailabilityType;

inline void MemorySet(Ipp8u *dst, Ipp32s val, Ipp32s length)
{
    memset(dst, val, length);
}

inline void MemorySet(Ipp16u *dst, Ipp32s val, Ipp32s length)
{
    ippsSet_16s((Ipp16s)val, (Ipp16s*)dst, length);
}

#define PIXBITS 8
#include "umc_h264_aic_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_aic_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER

