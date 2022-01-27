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
#include "umc_h264_bs.h"
#include "umc_h264_video_encoder.h"
#include "umc_h264_core_enc.h"
#include "umc_h264_tables.h"

// Bit stream field sizes
namespace UMC_H264_ENCODER
{

typedef struct {
    Ipp8u code;
    Ipp8u len;
} struct_CodeEntry;

static const struct_CodeEntry EncTotalCoeff[5][17][4] =
{
    {   // Num-VLC0
        //   0       1       2       3  <-- Trailing Ones
        { { 1, 1},{ 0, 0},{ 0, 0},{ 0, 0} }, // 0 Total_Coeffs
        { { 5, 6},{ 1, 2},{ 0, 0},{ 0, 0} }, // 1 Total_Coeffs
        { { 7, 8},{ 4, 6},{ 1, 3},{ 0, 0} }, // 2 Total_Coeffs
        { { 7, 9},{ 6, 8},{ 5, 7},{ 3, 5} }, // 3 Total_Coeffs
        { { 7,10},{ 6, 9},{ 5, 8},{ 3, 6} }, // 4 Total_Coeffs
        { { 7,11},{ 6,10},{ 5, 9},{ 4, 7} }, // 5 Total_Coeffs
        { {15,13},{ 6,11},{ 5,10},{ 4, 8} }, // 6 Total_Coeffs
        { {11,13},{14,13},{ 5,11},{ 4, 9} }, // 7 Total_Coeffs
        { { 8,13},{10,13},{13,13},{ 4,10} }, // 8 Total_Coeffs
        { {15,14},{14,14},{ 9,13},{ 4,11} }, // 9 Total_Coeffs
        { {11,14},{10,14},{13,14},{12,13} }, // 10 Total_Coeffs
        { {15,15},{14,15},{ 9,14},{12,14} }, // 11 Total_Coeffs
        { {11,15},{10,15},{13,15},{ 8,14} }, // 12 Total_Coeffs
        { {15,16},{ 1,15},{ 9,15},{12,15} }, // 13 Total_Coeffs
        { {11,16},{14,16},{13,16},{ 8,15} }, // 14 Total_Coeffs
        { { 7,16},{10,16},{ 9,16},{12,16} }, // 15 Total_Coeffs
        { { 4,16},{ 6,16},{ 5,16},{ 8,16} }  // 16 Total_Coeffs

    },
    {   // Num-VLC1
        //   0       1       2       3  <-- Trailing Ones
        { { 3, 2},{ 0, 0},{ 0, 0},{ 0, 0} }, // 0 Total_Coeffs
        { {11, 6},{ 2, 2},{ 0, 0},{ 0, 0} }, // 1 Total_Coeffs
        { { 7, 6},{ 7, 5},{ 3, 3},{ 0, 0} }, // 2 Total_Coeffs
        { { 7, 7},{10, 6},{ 9, 6},{ 5, 4} }, // 3 Total_Coeffs
        { { 7, 8},{ 6, 6},{ 5, 6},{ 4, 4} }, // 4 Total_Coeffs
        { { 4, 8},{ 6, 7},{ 5, 7},{ 6, 5} }, // 5 Total_Coeffs
        { { 7, 9},{ 6, 8},{ 5, 8},{ 8, 6} }, // 6 Total_Coeffs
        { {15,11},{ 6, 9},{ 5, 9},{ 4, 6} }, // 7 Total_Coeffs
        { {11,11},{14,11},{13,11},{ 4, 7} }, // 8 Total_Coeffs
        { {15,12},{10,11},{ 9,11},{ 4, 9} }, // 9 Total_Coeffs
        { {11,12},{14,12},{13,12},{12,11} }, // 10 Total_Coeffs
        { { 8,12},{10,12},{ 9,12},{ 8,11} }, // 11 Total_Coeffs
        { {15,13},{14,13},{13,13},{12,12} }, // 12 Total_Coeffs
        { {11,13},{10,13},{ 9,13},{12,13} }, // 13 Total_Coeffs
        { { 7,13},{11,14},{ 6,13},{ 8,13} }, // 14 Total_Coeffs
        { { 9,14},{ 8,14},{10,14},{ 1,13} }, // 15 Total_Coeffs
        { { 7,14},{ 6,14},{ 5,14},{ 4,14} }  // 16 Total_Coeffs
    },
    {   // Num-VLC2
        //   0       1       2       3  <-- Trailing Ones
        { {15, 4},{ 0, 0},{ 0, 0},{ 0, 0} }, // 0 Total_Coeffs
        { {15, 6},{14, 4},{ 0, 0},{ 0, 0} }, // 1 Total_Coeffs
        { {11, 6},{15, 5},{13, 4},{ 0, 0} }, // 2 Total_Coeffs
        { { 8, 6},{12, 5},{14, 5},{12, 4} }, // 3 Total_Coeffs
        { {15, 7},{10, 5},{11, 5},{11, 4} }, // 4 Total_Coeffs
        { {11, 7},{ 8, 5},{ 9, 5},{10, 4} }, // 5 Total_Coeffs
        { { 9, 7},{14, 6},{13, 6},{ 9, 4} }, // 6 Total_Coeffs
        { { 8, 7},{10, 6},{ 9, 6},{ 8, 4} }, // 7 Total_Coeffs
        { {15, 8},{14, 7},{13, 7},{13, 5} }, // 8 Total_Coeffs
        { {11, 8},{14, 8},{10, 7},{12, 6} }, // 9 Total_Coeffs
        { {15, 9},{10, 8},{13, 8},{12, 7} }, // 10 Total_Coeffs
        { {11, 9},{14, 9},{ 9, 8},{12, 8} }, // 11 Total_Coeffs
        { { 8, 9},{10, 9},{13, 9},{ 8, 8} }, // 12 Total_Coeffs
        { {13,10},{ 7, 9},{ 9, 9},{12, 9} }, // 13 Total_Coeffs
        { { 9,10},{12,10},{11,10},{10,10} }, // 14 Total_Coeffs
        { { 5,10},{ 8,10},{ 7,10},{ 6,10} }, // 15 Total_Coeffs
        { { 1,10},{ 4,10},{ 3,10},{ 2,10} }  // 16 Total_Coeffs
    },
    {   // Num-VLC_ChromaDC 420
        //   0       1       2       3  <-- Trailing Ones
        { { 1, 2},{ 0, 0},{ 0, 0},{ 0, 0} }, // 0 Total_Coeffs
        { { 7, 6},{ 1, 1},{ 0, 0},{ 0, 0} }, // 1 Total_Coeffs
        { { 4, 6},{ 6, 6},{ 1, 3},{ 0, 0} }, // 2 Total_Coeffs
        { { 3, 6},{ 3, 7},{ 2, 7},{ 5, 6} }, // 3 Total_Coeffs
        { { 2, 6},{ 3, 8},{ 2, 8},{ 0, 7} }, // 4 Total_Coeffs
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} }, // Not used...
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} }
    },
    {   // Num-VLC_ChromaDC 422
        //   0       1       2       3  <-- Trailing Ones
        { { 1,  1},{ 0,  0},{ 0,  0},{ 0, 0} }, // 0 Total_Coeffs
        { { 15, 7},{ 1,  2},{ 0,  0},{ 0, 0} }, // 1 Total_Coeffs
        { { 14, 7},{ 13, 7},{ 1,  3},{ 0, 0} }, // 2 Total_Coeffs
        { {  7, 9},{ 12, 7},{ 11, 7},{ 1, 5} }, // 3 Total_Coeffs
        { {  6, 9},{ 5,  9},{ 10, 7},{ 1, 6}  }, // 4 Total_Coeffs
        { { 7, 10},{ 6, 10},{ 4,  9},{ 9, 7} }, // 5 Total_Coeffs
        { { 7, 11},{ 6, 11},{ 5, 10},{ 8, 7} }, // 6 Total_Coeffs
        { { 7, 12},{ 6, 12},{ 5, 11},{ 4, 10} }, // 7 Total_Coeffs
        { { 7, 13},{ 5, 12},{ 4, 12},{ 4, 11} }, // 8 Total_Coeffs
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} }, // Not used...
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
        { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} }
    }
};

static const struct_CodeEntry EncTotalZerosChroma[3][16][16] = {
    {
    //  0     1     2    <- (TotalCoeff-1)
    { {1,1},{1,1},{1,1} },  // 0 TotalZeros
    { {1,2},{1,2},{0,1} },  // 1 TotalZeros
    { {1,3},{0,2},{0,0} },  // 2 TotalZeros
    { {0,3},{0,0},{0,0} }   // 3 TotalZeros
    },{
    //  0     1     2     3     4     5     6 <- (TotalCoeff-1)
    { {1,1},{0,3},{0,3},{6,3},{0,2},{0,2},{0,1} },   // 0 TotalZeros
    { {2,3},{1,2},{1,3},{0,2},{1,2},{1,2},{1,1} },   // 1 TotalZeros
    { {3,3},{1,3},{1,2},{1,2},{2,2},{1,1},{0,0} },   // 2 TotalZeros
    { {2,4},{4,3},{2,2},{2,2},{3,2},{0,0},{0,0} },   // 3 TotalZeros
    { {3,4},{5,3},{6,3},{7,3},{0,0},{0,0},{0,0} },   // 4 TotalZeros
    { {1,4},{6,3},{7,3},{0,0},{0,0},{0,0},{0,0} },   // 5 TotalZeros
    { {1,5},{7,3},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 6 TotalZeros
    { {0,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 7 TotalZeros
    },{
    //  0     1     2     3     4     5     6     7     8     9     10    11    12    13    14 <- (TotalCoeff-1)
    { {1,1},{7,3},{5,4},{3,5},{5,4},{1,6},{1,6},{1,6},{1,6},{1,5},{0,4},{0,4},{0,3},{0,2},{0,1} },   // 0 TotalZeros
    { {3,3},{6,3},{7,3},{7,3},{4,4},{1,5},{1,5},{1,4},{0,6},{0,5},{1,4},{1,4},{1,3},{1,2},{1,1} },   // 1 TotalZeros
    { {2,3},{5,3},{6,3},{5,4},{3,4},{7,3},{5,3},{1,5},{1,4},{1,3},{1,3},{1,2},{1,1},{1,1},{0,0} },   // 2 TotalZeros
    { {3,4},{4,3},{5,3},{4,4},{7,3},{6,3},{4,3},{3,3},{3,2},{3,2},{2,3},{1,1},{1,2},{0,0},{0,0} },   // 3 TotalZeros
    { {2,4},{3,3},{4,4},{6,3},{6,3},{5,3},{3,3},{3,2},{2,2},{2,2},{1,1},{1,3},{0,0},{0,0},{0,0} },   // 4 TotalZeros
    { {3,5},{5,4},{3,4},{5,3},{5,3},{4,3},{3,2},{2,2},{1,3},{1,2},{3,3},{0,0},{0,0},{0,0},{0,0} },   // 5 TotalZeros
    { {2,5},{4,4},{4,3},{4,3},{4,3},{3,3},{2,3},{2,3},{1,2},{1,4},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 6 TotalZeros
    { {3,6},{3,4},{3,3},{3,4},{3,3},{2,3},{1,4},{1,3},{1,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 7 TotalZeros
    { {2,6},{2,4},{2,4},{3,3},{2,4},{1,4},{1,3},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 8 TotalZeros
    { {3,7},{3,5},{3,5},{2,4},{1,5},{1,3},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 9 TotalZeros
    { {2,7},{2,5},{2,5},{2,5},{1,4},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 10 TotalZeros
    { {3,8},{3,6},{1,6},{1,5},{0,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 11 TotalZeros
    { {2,8},{2,6},{1,5},{0,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 12 TotalZeros
    { {3,9},{1,6},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 13 TotalZeros
    { {2,9},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 14 TotalZeros
    { {1,9},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },   // 15 TotalZeros
    }
};

static const struct_CodeEntry EncTotalZeros4x4[16][16] = {
    //   0/7   1/8   2/9  3/10  4/11  5/12  6/13   14  <- (TotalCoeff-1)
    {   {1,1},{7,3},{5,4},{3,5},{5,4},{1,6},{1,6},
        {1,6},{1,6},{1,5},{0,4},{0,4},{0,3},{0,2},{0,1}, {0,0} },  // 0 TotalZeros
    {   {3,3},{6,3},{7,3},{7,3},{4,4},{1,5},{1,5},
        {1,4},{0,6},{0,5},{1,4},{1,4},{1,3},{1,2},{1,1}, {0,0} },  // 1 TotalZeros
    {   {2,3},{5,3},{6,3},{5,4},{3,4},{7,3},{5,3},
        {1,5},{1,4},{1,3},{1,3},{1,2},{1,1},{1,1},{0,0}, {0,0} },  // 2 TotalZeros
    {   {3,4},{4,3},{5,3},{4,4},{7,3},{6,3},{4,3},
        {3,3},{3,2},{3,2},{2,3},{1,1},{1,2},{0,0},{0,0}, {0,0} },  // 3 TotalZeros
    {   {2,4},{3,3},{4,4},{6,3},{6,3},{5,3},{3,3},
        {3,2},{2,2},{2,2},{1,1},{1,3},{0,0},{0,0},{0,0}, {0,0} },  // 4 TotalZeros
    {   {3,5},{5,4},{3,4},{5,3},{5,3},{4,3},{3,2},
        {2,2},{1,3},{1,2},{3,3},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 5 TotalZeros
    {   {2,5},{4,4},{4,3},{4,3},{4,3},{3,3},{2,3},
        {2,3},{1,2},{1,4},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 6 TotalZeros
    {   {3,6},{3,4},{3,3},{3,4},{3,3},{2,3},{1,4},
        {1,3},{1,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 7 TotalZeros
    {   {2,6},{2,4},{2,4},{3,3},{2,4},{1,4},{1,3},
        {0,6},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 8 TotalZeros
    {   {3,7},{3,5},{3,5},{2,4},{1,5},{1,3},{0,6},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 9 TotalZeros
    {   {2,7},{2,5},{2,5},{2,5},{1,4},{0,6},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 10 TotalZeros
    {   {3,8},{3,6},{1,6},{1,5},{0,5},{0,0},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 11 TotalZeros
    {   {2,8},{2,6},{1,5},{0,5},{0,0},{0,0},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 12 TotalZeros
    {   {3,9},{1,6},{0,6},{0,0},{0,0},{0,0},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 13 TotalZeros
    {   {2,9},{0,6},{0,0},{0,0},{0,0},{0,0},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} },  // 14 TotalZeros
    {   {1,9},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
        {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}, {0,0} }   // 15 TotalZeros
};

static const struct_CodeEntry EncRuns[15][8] = {
    //    0     1     2     3     4     5     6+     <- (ZerosLeft-1)
    {   {1,1},{1,1},{3,2},{3,2},{3,2},{3,2},{7,3}, {0,0} }, // 0 RunBefore
    {   {0,1},{1,2},{2,2},{2,2},{2,2},{0,3},{6,3}, {0,0} }, // 1 RunBefore
    {   {0,0},{0,2},{1,2},{1,2},{3,3},{1,3},{5,3}, {0,0} }, // 2 RunBefore
    {   {0,0},{0,0},{0,2},{1,3},{2,3},{3,3},{4,3}, {0,0} }, // 3 RunBefore
    {   {0,0},{0,0},{0,0},{0,3},{1,3},{2,3},{3,3}, {0,0} }, // 4 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,3},{5,3},{2,3}, {0,0} }, // 5 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{4,3},{1,3}, {0,0} }, // 6 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,4}, {0,0} }, // 7 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,5}, {0,0} }, // 8 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,6}, {0,0} }, // 9 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,7}, {0,0} }, // 10 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,8}, {0,0} }, // 11 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,9}, {0,0} }, // 12 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,10}, {0,0}}, // 13 RunBefore
    {   {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,11}, {0,0}}  // 14 RunBefore
};

static const Ipp32s ctx_neq1p1[8] = { 1,2,3,4,0,0,0,0};
static const Ipp32s ctx_ngt1[8] =   { 5,5,5,5,6,7,8,9};
static const Ipp32s ctx_id_trans0[8] = {1,2,3,3,4,5,6,7};
static const Ipp32s ctx_id_trans1[8] = {4,4,4,4,5,6,7,7};
static const Ipp32s ctx_id_trans13[7] = {4,4,4,4,5,6,6};

#define PIXBITS 8
#include "umc_h264_bs_tmpl.cpp.h"
#define FAKE_BITSTREAM
#include "umc_h264_bs_tmpl.cpp.h"
#undef FAKE_BITSTREAM
#undef PIXBITS

#ifdef BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_bs_tmpl.cpp.h"
#define FAKE_BITSTREAM
#include "umc_h264_bs_tmpl.cpp.h"
#undef FAKE_BITSTREAM
#undef PIXBITS

#endif // BITDEPTH_9_12


} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER

