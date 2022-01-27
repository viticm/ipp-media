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
#include "umc_h264_video_encoder.h"
#include "umc_h264_tables.h"
#include "umc_h264_to_ipp.h"
#include "umc_h264_bme.h"
#include "ippvc.h"

namespace UMC_H264_ENCODER
{

Ipp32u SAT8x8D(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
    Ipp32u satd = 0;
    __ALIGN16 Ipp16s diff[8][8];

#ifndef INTRINSIC_OPT
    Ipp32s i;

    ippiSub8x8_8u16s_C1R(pSrc1, src1Step, pSrc2, src2Step, &diff[0][0], 16);
    for (i = 0; i < 8; i++) {
        Ipp32s t0 = diff[i][0] + diff[i][4];
        Ipp32s t4 = diff[i][0] - diff[i][4];
        Ipp32s t1 = diff[i][1] + diff[i][5];
        Ipp32s t5 = diff[i][1] - diff[i][5];
        Ipp32s t2 = diff[i][2] + diff[i][6];
        Ipp32s t6 = diff[i][2] - diff[i][6];
        Ipp32s t3 = diff[i][3] + diff[i][7];
        Ipp32s t7 = diff[i][3] - diff[i][7];
        Ipp32s s0 = t0 + t2;
        Ipp32s s2 = t0 - t2;
        Ipp32s s1 = t1 + t3;
        Ipp32s s3 = t1 - t3;
        Ipp32s s4 = t4 + t6;
        Ipp32s s6 = t4 - t6;
        Ipp32s s5 = t5 + t7;
        Ipp32s s7 = t5 - t7;
        diff[i][0] = s0 + s1;
        diff[i][1] = s0 - s1;
        diff[i][2] = s2 + s3;
        diff[i][3] = s2 - s3;
        diff[i][4] = s4 + s5;
        diff[i][5] = s4 - s5;
        diff[i][6] = s6 + s7;
        diff[i][7] = s6 - s7;
    }
    for (i = 0; i < 8; i++) {
        Ipp32s t0 = diff[0][i] + diff[4][i];
        Ipp32s t4 = diff[0][i] - diff[4][i];
        Ipp32s t1 = diff[1][i] + diff[5][i];
        Ipp32s t5 = diff[1][i] - diff[5][i];
        Ipp32s t2 = diff[2][i] + diff[6][i];
        Ipp32s t6 = diff[2][i] - diff[6][i];
        Ipp32s t3 = diff[3][i] + diff[7][i];
        Ipp32s t7 = diff[3][i] - diff[7][i];
        Ipp32s s0 = t0 + t2;
        Ipp32s s2 = t0 - t2;
        Ipp32s s1 = t1 + t3;
        Ipp32s s3 = t1 - t3;
        Ipp32s s4 = t4 + t6;
        Ipp32s s6 = t4 - t6;
        Ipp32s s5 = t5 + t7;
        Ipp32s s7 = t5 - t7;
        satd += ABS(s0 + s1);
        satd += ABS(s0 - s1);
        satd += ABS(s2 + s3);
        satd += ABS(s2 - s3);
        satd += ABS(s4 + s5);
        satd += ABS(s4 - s5);
        satd += ABS(s6 + s7);
        satd += ABS(s6 - s7);
    }
#else
#if 0
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_6, _p_7, _b_2, _b_3, _b_6, _b_7, _p_t, _p_s;
    Ipp32s  s;
/*
    for (i = 0; i < 8; i++) {
        _p_0 = _mm_loadl_epi64((__m128i*)(&diff[i][0]));
        _p_2 = _mm_loadl_epi64((__m128i*)(&diff[i][4]));
        _p_1 = _mm_sub_epi16(_p_0, _p_2);   //  0,  0,  0,  0, a7, a6, a5, a4
        _p_0 = _mm_add_epi16(_p_0, _p_2);   //  0,  0,  0,  0, a3, a2, a1, a0
        _p_5 = _mm_srli_si128(_p_1, 4);     //  0,  0,  0,  0,  0,  0, a7, a6
        _p_4 = _mm_srli_si128(_p_0, 4);     //  0,  0,  0,  0,  0,  0, a3, a2
        _p_3 = _mm_sub_epi16(_p_1, _p_5);   //  0,  0,  0,  0, xx, xx, b7, b6
        _p_1 = _mm_add_epi16(_p_1, _p_5);   //  0,  0,  0,  0, xx, xx, b5, b4
        _p_2 = _mm_sub_epi16(_p_0, _p_4);   //  0,  0,  0,  0, xx, xx, b3, b2
        _p_0 = _mm_add_epi16(_p_0, _p_4);   //  0,  0,  0,  0, xx, xx, b1, b0
        _p_0 = _mm_unpacklo_epi16(_p_0, _p_2);   // xx, xx, xx, xx, b3, b1, b2, b0
        _p_1 = _mm_unpacklo_epi16(_p_1, _p_3);   // xx, xx, xx, xx, b7, b5, b6, b4
        _p_0 = _mm_unpacklo_epi32(_p_0, _p_1);   // b7, b5, b3, b1, b6, b4, b2, b0
        _p_2 = _mm_srli_si128(_p_0, 8);          //  0,  0,  0,  0, b7, b5, b3, b1
        _p_1 = _mm_sub_epi16(_p_0, _p_2);
        _p_0 = _mm_add_epi16(_p_0, _p_2);
        _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
        _mm_store_si128((__m128i*)diff[i], _p_0);
    }
*/
    for (i = 0; i < 8; i++) {
        Ipp32s t0 = diff[i][0] + diff[i][4];
        Ipp32s t4 = diff[i][0] - diff[i][4];
        Ipp32s t1 = diff[i][1] + diff[i][5];
        Ipp32s t5 = diff[i][1] - diff[i][5];
        Ipp32s t2 = diff[i][2] + diff[i][6];
        Ipp32s t6 = diff[i][2] - diff[i][6];
        Ipp32s t3 = diff[i][3] + diff[i][7];
        Ipp32s t7 = diff[i][3] - diff[i][7];
        Ipp32s s0 = t0 + t2;
        Ipp32s s2 = t0 - t2;
        Ipp32s s1 = t1 + t3;
        Ipp32s s3 = t1 - t3;
        Ipp32s s4 = t4 + t6;
        Ipp32s s6 = t4 - t6;
        Ipp32s s5 = t5 + t7;
        Ipp32s s7 = t5 - t7;
        diff[i][0] = s0 + s1;
        diff[i][1] = s0 - s1;
        diff[i][2] = s2 + s3;
        diff[i][3] = s2 - s3;
        diff[i][4] = s4 + s5;
        diff[i][5] = s4 - s5;
        diff[i][6] = s6 + s7;
        diff[i][7] = s6 - s7;
    }
    _p_0 = _mm_load_si128((__m128i*)(diff[0]));
    _p_4 = _mm_sub_epi16(_p_0, *(__m128i*)(diff[4]));
    _p_0 = _mm_add_epi16(_p_0, *(__m128i*)(diff[4]));
    _p_1 = _mm_load_si128((__m128i*)(diff[1]));
    _p_5 = _mm_sub_epi16(_p_1, *(__m128i*)(diff[5]));
    _p_1 = _mm_add_epi16(_p_1, *(__m128i*)(diff[5]));
    _p_2 = _mm_load_si128((__m128i*)(diff[2]));
    _p_6 = _mm_sub_epi16(_p_2, *(__m128i*)(diff[6]));
    _p_2 = _mm_add_epi16(_p_2, *(__m128i*)(diff[6]));
    _p_3 = _mm_load_si128((__m128i*)(diff[3]));
    _p_7 = _mm_sub_epi16(_p_3, *(__m128i*)(diff[7]));
    _p_3 = _mm_add_epi16(_p_3, *(__m128i*)(diff[7]));

    _b_2 = _mm_sub_epi16(_p_0, _p_2);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _b_3 = _mm_sub_epi16(_p_1, _p_3);
    _p_1 = _mm_add_epi16(_p_1, _p_3);
    _b_6 = _mm_sub_epi16(_p_4, _p_6);
    _p_4 = _mm_add_epi16(_p_4, _p_6);
    _b_7 = _mm_sub_epi16(_p_5, _p_7);
    _p_5 = _mm_add_epi16(_p_5, _p_7);

    _p_s = _mm_sub_epi16(_p_0, _p_1);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_1);
    _p_t = _mm_srai_epi16(_p_0, 15);
    _p_0 = _mm_xor_si128(_p_0, _p_t);
    _p_0 = _mm_sub_epi16(_p_0, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);

    _p_s = _mm_sub_epi16(_b_2, _b_3);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _b_2 = _mm_add_epi16(_b_2, _b_3);
    _p_t = _mm_srai_epi16(_b_2, 15);
    _b_2 = _mm_xor_si128(_b_2, _p_t);
    _b_2 = _mm_sub_epi16(_b_2, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _b_2);

    _p_s = _mm_sub_epi16(_p_4, _p_5);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _p_4 = _mm_add_epi16(_p_4, _p_5);
    _p_t = _mm_srai_epi16(_p_4, 15);
    _p_4 = _mm_xor_si128(_p_4, _p_t);
    _p_4 = _mm_sub_epi16(_p_4, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_4);

    _p_s = _mm_sub_epi16(_b_6, _b_7);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _b_6 = _mm_add_epi16(_b_6, _b_7);
    _p_t = _mm_srai_epi16(_b_6, 15);
    _b_6 = _mm_xor_si128(_b_6, _p_t);
    _b_6 = _mm_sub_epi16(_b_6, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _b_6);

    _p_t = _mm_srli_si128(_p_0, 8);
    _p_0 = _mm_add_epi16(_p_0, _p_t);
    _p_t = _mm_srli_si128(_p_0, 4);
    _p_0 = _mm_add_epi16(_p_0, _p_t);
    s = _mm_cvtsi128_si32(_p_0);
    satd += (s >> 16) + (Ipp16s)s;
#endif
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_6, _p_7, _b_2, _b_3, _b_6, _b_7, _p_t, _p_s, _p_zero;
    Ipp32s  s;
    __ALIGN16 Ipp16s tmp[8][8];

    _p_zero = _mm_setzero_si128();
    _p_0 = _mm_loadl_epi64((__m128i*)(pSrc1));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2));
    _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_0 = _mm_sub_epi16(_p_0, _p_t);
    _p_s = _mm_loadl_epi64((__m128i*)(pSrc1+4*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+4*src2Step));
    _p_s = _mm_unpacklo_epi8(_p_s, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_4 = _mm_sub_epi16(_p_0, _p_s);
    _p_0 = _mm_add_epi16(_p_0, _p_s);

    _p_1 = _mm_loadl_epi64((__m128i*)(pSrc1+1*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+1*src2Step));
    _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_1 = _mm_sub_epi16(_p_1, _p_t);
    _p_s = _mm_loadl_epi64((__m128i*)(pSrc1+5*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+5*src2Step));
    _p_s = _mm_unpacklo_epi8(_p_s, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_5 = _mm_sub_epi16(_p_1, _p_s);
    _p_1 = _mm_add_epi16(_p_1, _p_s);

    _p_2 = _mm_loadl_epi64((__m128i*)(pSrc1+2*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+2*src2Step));
    _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_2 = _mm_sub_epi16(_p_2, _p_t);
    _p_s = _mm_loadl_epi64((__m128i*)(pSrc1+6*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+6*src2Step));
    _p_s = _mm_unpacklo_epi8(_p_s, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_6 = _mm_sub_epi16(_p_2, _p_s);
    _p_2 = _mm_add_epi16(_p_2, _p_s);

    _p_3 = _mm_loadl_epi64((__m128i*)(pSrc1+3*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+3*src2Step));
    _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_3 = _mm_sub_epi16(_p_3, _p_t);
    _p_s = _mm_loadl_epi64((__m128i*)(pSrc1+7*src1Step));
    _p_t = _mm_loadl_epi64((__m128i*)(pSrc2+7*src2Step));
    _p_s = _mm_unpacklo_epi8(_p_s, _p_zero);
    _p_t = _mm_unpacklo_epi8(_p_t, _p_zero);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_7 = _mm_sub_epi16(_p_3, _p_s);
    _p_3 = _mm_add_epi16(_p_3, _p_s);

/*
    ippiSub8x8_8u16s_C1R(pSrc1, src1Step, pSrc2, src2Step, &diff[0][0], 16);
    _p_0 = _mm_load_si128((__m128i*)(diff[0]));
    _p_4 = _mm_sub_epi16(_p_0, *(__m128i*)(diff[4]));
    _p_0 = _mm_add_epi16(_p_0, *(__m128i*)(diff[4]));
    _p_1 = _mm_load_si128((__m128i*)(diff[1]));
    _p_5 = _mm_sub_epi16(_p_1, *(__m128i*)(diff[5]));
    _p_1 = _mm_add_epi16(_p_1, *(__m128i*)(diff[5]));
    _p_2 = _mm_load_si128((__m128i*)(diff[2]));
    _p_6 = _mm_sub_epi16(_p_2, *(__m128i*)(diff[6]));
    _p_2 = _mm_add_epi16(_p_2, *(__m128i*)(diff[6]));
    _p_3 = _mm_load_si128((__m128i*)(diff[3]));
    _p_7 = _mm_sub_epi16(_p_3, *(__m128i*)(diff[7]));
    _p_3 = _mm_add_epi16(_p_3, *(__m128i*)(diff[7]));
*/
    _b_2 = _mm_sub_epi16(_p_0, _p_2);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _b_3 = _mm_sub_epi16(_p_1, _p_3);
    _p_1 = _mm_add_epi16(_p_1, _p_3);
    _b_6 = _mm_sub_epi16(_p_4, _p_6);
    _p_4 = _mm_add_epi16(_p_4, _p_6);
    _b_7 = _mm_sub_epi16(_p_5, _p_7);
    _p_5 = _mm_add_epi16(_p_5, _p_7);

    _p_s = _mm_sub_epi16(_p_0, _p_1);
    _p_0 = _mm_add_epi16(_p_0, _p_1);
    _mm_store_si128((__m128i*)diff[1], _p_s);
    _mm_store_si128((__m128i*)diff[0], _p_0);
    _p_s = _mm_sub_epi16(_b_2, _b_3);
    _b_2 = _mm_add_epi16(_b_2, _b_3);
    _mm_store_si128((__m128i*)diff[3], _p_s);
    _mm_store_si128((__m128i*)diff[2], _b_2);
    _p_s = _mm_sub_epi16(_p_4, _p_5);
    _p_4 = _mm_add_epi16(_p_4, _p_5);
    _mm_store_si128((__m128i*)diff[5], _p_s);
    _mm_store_si128((__m128i*)diff[4], _p_4);
    _p_s = _mm_sub_epi16(_b_6, _b_7);
    _b_6 = _mm_add_epi16(_b_6, _b_7);
    _mm_store_si128((__m128i*)diff[7], _p_s);
    _mm_store_si128((__m128i*)diff[6], _b_6);

    _p_0 = _mm_loadl_epi64((__m128i*)(&diff[0][0]));
    _p_1 = _mm_loadl_epi64((__m128i*)(&diff[1][0]));
    _p_2 = _mm_loadl_epi64((__m128i*)(&diff[2][0]));
    _p_3 = _mm_loadl_epi64((__m128i*)(&diff[3][0]));
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_2 = _mm_unpacklo_epi16(_p_2, _p_3);
    _p_1 = _mm_unpackhi_epi32(_p_0, _p_2);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_2);
    _mm_storel_epi64((__m128i*)&tmp[0][0], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[2][0], _p_1);
    _p_0 = _mm_srli_si128(_p_0, 8);
    _p_1 = _mm_srli_si128(_p_1, 8);
    _mm_storel_epi64((__m128i*)&tmp[1][0], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[3][0], _p_1);

    _p_0 = _mm_loadl_epi64((__m128i*)(&diff[0][4]));
    _p_1 = _mm_loadl_epi64((__m128i*)(&diff[1][4]));
    _p_2 = _mm_loadl_epi64((__m128i*)(&diff[2][4]));
    _p_3 = _mm_loadl_epi64((__m128i*)(&diff[3][4]));
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_2 = _mm_unpacklo_epi16(_p_2, _p_3);
    _p_1 = _mm_unpackhi_epi32(_p_0, _p_2);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_2);
    _mm_storel_epi64((__m128i*)&tmp[4][0], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[6][0], _p_1);
    _p_0 = _mm_srli_si128(_p_0, 8);
    _p_1 = _mm_srli_si128(_p_1, 8);
    _mm_storel_epi64((__m128i*)&tmp[5][0], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[7][0], _p_1);

    _p_0 = _mm_loadl_epi64((__m128i*)(&diff[4][0]));
    _p_1 = _mm_loadl_epi64((__m128i*)(&diff[5][0]));
    _p_2 = _mm_loadl_epi64((__m128i*)(&diff[6][0]));
    _p_3 = _mm_loadl_epi64((__m128i*)(&diff[7][0]));
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_2 = _mm_unpacklo_epi16(_p_2, _p_3);
    _p_1 = _mm_unpackhi_epi32(_p_0, _p_2);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_2);
    _mm_storel_epi64((__m128i*)&tmp[0][4], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[2][4], _p_1);
    _p_0 = _mm_srli_si128(_p_0, 8);
    _p_1 = _mm_srli_si128(_p_1, 8);
    _mm_storel_epi64((__m128i*)&tmp[1][4], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[3][4], _p_1);

    _p_0 = _mm_loadl_epi64((__m128i*)(&diff[4][4]));
    _p_1 = _mm_loadl_epi64((__m128i*)(&diff[5][4]));
    _p_2 = _mm_loadl_epi64((__m128i*)(&diff[6][4]));
    _p_3 = _mm_loadl_epi64((__m128i*)(&diff[7][4]));
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_2 = _mm_unpacklo_epi16(_p_2, _p_3);
    _p_1 = _mm_unpackhi_epi32(_p_0, _p_2);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_2);
    _mm_storel_epi64((__m128i*)&tmp[4][4], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[6][4], _p_1);
    _p_0 = _mm_srli_si128(_p_0, 8);
    _p_1 = _mm_srli_si128(_p_1, 8);
    _mm_storel_epi64((__m128i*)&tmp[5][4], _p_0);
    _mm_storel_epi64((__m128i*)&tmp[7][4], _p_1);

    _p_0 = _mm_load_si128((__m128i*)(tmp[0]));
    _p_4 = _mm_sub_epi16(_p_0, *(__m128i*)(tmp[4]));
    _p_0 = _mm_add_epi16(_p_0, *(__m128i*)(tmp[4]));
    _p_1 = _mm_load_si128((__m128i*)(tmp[1]));
    _p_5 = _mm_sub_epi16(_p_1, *(__m128i*)(tmp[5]));
    _p_1 = _mm_add_epi16(_p_1, *(__m128i*)(tmp[5]));
    _p_2 = _mm_load_si128((__m128i*)(tmp[2]));
    _p_6 = _mm_sub_epi16(_p_2, *(__m128i*)(tmp[6]));
    _p_2 = _mm_add_epi16(_p_2, *(__m128i*)(tmp[6]));
    _p_3 = _mm_load_si128((__m128i*)(tmp[3]));
    _p_7 = _mm_sub_epi16(_p_3, *(__m128i*)(tmp[7]));
    _p_3 = _mm_add_epi16(_p_3, *(__m128i*)(tmp[7]));

    _b_2 = _mm_sub_epi16(_p_0, _p_2);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _b_3 = _mm_sub_epi16(_p_1, _p_3);
    _p_1 = _mm_add_epi16(_p_1, _p_3);
    _b_6 = _mm_sub_epi16(_p_4, _p_6);
    _p_4 = _mm_add_epi16(_p_4, _p_6);
    _b_7 = _mm_sub_epi16(_p_5, _p_7);
    _p_5 = _mm_add_epi16(_p_5, _p_7);

    _p_s = _mm_sub_epi16(_p_0, _p_1);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_1);
    _p_t = _mm_srai_epi16(_p_0, 15);
    _p_0 = _mm_xor_si128(_p_0, _p_t);
    _p_0 = _mm_sub_epi16(_p_0, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);

    _p_s = _mm_sub_epi16(_b_2, _b_3);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _b_2 = _mm_add_epi16(_b_2, _b_3);
    _p_t = _mm_srai_epi16(_b_2, 15);
    _b_2 = _mm_xor_si128(_b_2, _p_t);
    _b_2 = _mm_sub_epi16(_b_2, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _b_2);

    _p_s = _mm_sub_epi16(_p_4, _p_5);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _p_4 = _mm_add_epi16(_p_4, _p_5);
    _p_t = _mm_srai_epi16(_p_4, 15);
    _p_4 = _mm_xor_si128(_p_4, _p_t);
    _p_4 = _mm_sub_epi16(_p_4, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_4);

    _p_s = _mm_sub_epi16(_b_6, _b_7);
    _p_t = _mm_srai_epi16(_p_s, 15);
    _p_s = _mm_xor_si128(_p_s, _p_t);
    _p_s = _mm_sub_epi16(_p_s, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _p_s);
    _b_6 = _mm_add_epi16(_b_6, _b_7);
    _p_t = _mm_srai_epi16(_b_6, 15);
    _b_6 = _mm_xor_si128(_b_6, _p_t);
    _b_6 = _mm_sub_epi16(_b_6, _p_t);
    _p_0 = _mm_add_epi16(_p_0, _b_6);

    _p_t = _mm_srli_si128(_p_0, 8);
    _p_0 = _mm_add_epi16(_p_0, _p_t);
    _p_t = _mm_srli_si128(_p_0, 4);
    _p_0 = _mm_add_epi16(_p_0, _p_t);
    s = _mm_cvtsi128_si32(_p_0);
    satd += (s >> 16) + (Ipp16s)s;
#endif
    return satd >> 2;
}


Ipp32u SAT8x8D(const Ipp16u *pSrc1, Ipp32s src1Step, const Ipp16u *pSrc2, Ipp32s src2Step)
{
    __ALIGN16 Ipp32s tmp[8][8];
    __ALIGN16 Ipp16s diff[8][8];
    Ipp32s i;
    Ipp32u satd = 0;

    ippiSub8x8_16u16s_C1R(pSrc1, src1Step*sizeof(Ipp16u), pSrc2, src2Step*sizeof(Ipp16u), &diff[0][0], 16);
    for (i = 0; i < 8; i++) {
        Ipp32s t0 = diff[i][0] + diff[i][4];
        Ipp32s t4 = diff[i][0] - diff[i][4];
        Ipp32s t1 = diff[i][1] + diff[i][5];
        Ipp32s t5 = diff[i][1] - diff[i][5];
        Ipp32s t2 = diff[i][2] + diff[i][6];
        Ipp32s t6 = diff[i][2] - diff[i][6];
        Ipp32s t3 = diff[i][3] + diff[i][7];
        Ipp32s t7 = diff[i][3] - diff[i][7];
        Ipp32s s0 = t0 + t2;
        Ipp32s s2 = t0 - t2;
        Ipp32s s1 = t1 + t3;
        Ipp32s s3 = t1 - t3;
        Ipp32s s4 = t4 + t6;
        Ipp32s s6 = t4 - t6;
        Ipp32s s5 = t5 + t7;
        Ipp32s s7 = t5 - t7;
        tmp[i][0] = s0 + s1;
        tmp[i][1] = s0 - s1;
        tmp[i][2] = s2 + s3;
        tmp[i][3] = s2 - s3;
        tmp[i][4] = s4 + s5;
        tmp[i][5] = s4 - s5;
        tmp[i][6] = s6 + s7;
        tmp[i][7] = s6 - s7;
    }
    for (i = 0; i < 8; i++) {
        Ipp32s t0 = tmp[0][i] + tmp[4][i];
        Ipp32s t4 = tmp[0][i] - tmp[4][i];
        Ipp32s t1 = tmp[1][i] + tmp[5][i];
        Ipp32s t5 = tmp[1][i] - tmp[5][i];
        Ipp32s t2 = tmp[2][i] + tmp[6][i];
        Ipp32s t6 = tmp[2][i] - tmp[6][i];
        Ipp32s t3 = tmp[3][i] + tmp[7][i];
        Ipp32s t7 = tmp[3][i] - tmp[7][i];
        Ipp32s s0 = t0 + t2;
        Ipp32s s2 = t0 - t2;
        Ipp32s s1 = t1 + t3;
        Ipp32s s3 = t1 - t3;
        Ipp32s s4 = t4 + t6;
        Ipp32s s6 = t4 - t6;
        Ipp32s s5 = t5 + t7;
        Ipp32s s7 = t5 - t7;
        satd += ABS(s0 + s1);
        satd += ABS(s0 - s1);
        satd += ABS(s2 + s3);
        satd += ABS(s2 - s3);
        satd += ABS(s4 + s5);
        satd += ABS(s4 - s5);
        satd += ABS(s6 + s7);
        satd += ABS(s6 - s7);
    }
    return satd >> 2;
}


Ipp32u SATD_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step, Ipp32s width, Ipp32s height)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
#endif
    Ipp32s x, y;
    Ipp32u satd = 0;

    for( y = 0; y < height; y += 4 ) {
        for( x = 0; x < width; x += 4 )  {
#ifndef INTRINSIC_OPT
            Ipp32s b;
            ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
#else
            __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
            const Ipp8u *pS1, *pS2;
            Ipp32s  s;

            pS1 = pSrc1 + x;
            pS2 = pSrc2 + x;
            _p_zero = _mm_setzero_si128();
            _p_0 = _mm_cvtsi32_si128(*(int*)(pS1));
            _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
            _p_1 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
            _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
            _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_0 = _mm_sub_epi16(_p_0, _p_4);
            _p_1 = _mm_sub_epi16(_p_1, _p_5);
            pS1 += 2 * src1Step;
            pS2 += 2 * src2Step;
            _p_2 = _mm_cvtsi32_si128(*(int*)(pS1));
            _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
            _p_3 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
            _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
            _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_2 = _mm_sub_epi16(_p_2, _p_4);
            _p_3 = _mm_sub_epi16(_p_3, _p_5);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_7 = _mm_subs_epi16(_p_2, _p_3);
            _p_2 = _mm_adds_epi16(_p_2, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_2);
            _p_0 = _mm_adds_epi16(_p_0, _p_2);
            _p_3 = _mm_adds_epi16(_p_5, _p_7);
            _p_5 = _mm_subs_epi16(_p_5, _p_7);
            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_1 = _mm_srli_si128(_p_0, 8);
            _p_3 = _mm_srli_si128(_p_7, 8);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_2 = _mm_subs_epi16(_p_7, _p_3);
            _p_7 = _mm_adds_epi16(_p_7, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_7);
            _p_0 = _mm_adds_epi16(_p_0, _p_7);
            _p_3 = _mm_adds_epi16(_p_5, _p_2);
            _p_5 = _mm_subs_epi16(_p_5, _p_2);
            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_3 = _mm_srai_epi16(_p_2, 15);
            _p_1 = _mm_srai_epi16(_p_0, 15);
            _p_2 = _mm_xor_si128(_p_2, _p_3);
            _p_0 = _mm_xor_si128(_p_0, _p_1);
            _p_2 = _mm_sub_epi16(_p_2, _p_3);
            _p_0 = _mm_sub_epi16(_p_0, _p_1);
            _p_0 = _mm_add_epi16(_p_0, _p_2);
            _p_2 = _mm_srli_si128(_p_0, 8);
            _p_0 = _mm_add_epi16(_p_0, _p_2);
            _p_2 = _mm_srli_si128(_p_0, 4);
            _p_0 = _mm_add_epi16(_p_0, _p_2);
            s = _mm_cvtsi128_si32(_p_0);
            satd += (s >> 16) + (Ipp16s)s;
#endif
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
    return satd >> 1;
}

Ipp32u SATD4x4_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32u satd = 0;
    Ipp32s b;

    ippiSub4x4_8u16s_C1R(pSrc1, src1Step, pSrc2, src2Step, &diffBuff[0][0], 8);
    for (b = 0; b < 4; b ++) {
        Ipp32s a01, a23, b01, b23;

        a01 = diffBuff[b][0] + diffBuff[b][1];
        a23 = diffBuff[b][2] + diffBuff[b][3];
        b01 = diffBuff[b][0] - diffBuff[b][1];
        b23 = diffBuff[b][2] - diffBuff[b][3];
        tmpBuff[b][0] = a01 + a23;
        tmpBuff[b][1] = a01 - a23;
        tmpBuff[b][2] = b01 - b23;
        tmpBuff[b][3] = b01 + b23;
    }
    for (b = 0; b < 4; b ++) {
        Ipp32s a01, a23, b01, b23;

        a01 = tmpBuff[0][b] + tmpBuff[1][b];
        a23 = tmpBuff[2][b] + tmpBuff[3][b];
        b01 = tmpBuff[0][b] - tmpBuff[1][b];
        b23 = tmpBuff[2][b] - tmpBuff[3][b];
        satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
    }
#else
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    Ipp32s s;
    Ipp32u satd;

    pS1 = pSrc1;
    pS2 = pSrc2;
    _p_zero = _mm_setzero_si128();
    _p_0 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_1 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_0 = _mm_sub_epi16(_p_0, _p_4);
    _p_1 = _mm_sub_epi16(_p_1, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_2 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_3 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_2 = _mm_sub_epi16(_p_2, _p_4);
    _p_3 = _mm_sub_epi16(_p_3, _p_5);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_7 = _mm_subs_epi16(_p_2, _p_3);
    _p_2 = _mm_adds_epi16(_p_2, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_2);
    _p_0 = _mm_adds_epi16(_p_0, _p_2);
    _p_3 = _mm_adds_epi16(_p_5, _p_7);
    _p_5 = _mm_subs_epi16(_p_5, _p_7);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_1 = _mm_srli_si128(_p_0, 8);
    _p_3 = _mm_srli_si128(_p_7, 8);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_2 = _mm_subs_epi16(_p_7, _p_3);
    _p_7 = _mm_adds_epi16(_p_7, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_7);
    _p_0 = _mm_adds_epi16(_p_0, _p_7);
    _p_3 = _mm_adds_epi16(_p_5, _p_2);
    _p_5 = _mm_subs_epi16(_p_5, _p_2);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_3 = _mm_srai_epi16(_p_2, 15);
    _p_1 = _mm_srai_epi16(_p_0, 15);
    _p_2 = _mm_xor_si128(_p_2, _p_3);
    _p_0 = _mm_xor_si128(_p_0, _p_1);
    _p_2 = _mm_sub_epi16(_p_2, _p_3);
    _p_0 = _mm_sub_epi16(_p_0, _p_1);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _p_2 = _mm_srli_si128(_p_0, 8);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _p_2 = _mm_srli_si128(_p_0, 4);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    s = _mm_cvtsi128_si32(_p_0);
    satd = (s >> 16) + (Ipp16s)s;
#endif
    return satd >> 1;
}

Ipp32u SATD4x8_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s y;
    Ipp32u satd = 0;

    for (y = 0; y < 8; y += 4) {
        Ipp32s b;
        ippiSub4x4_8u16s_C1R(pSrc1, src1Step, pSrc2, src2Step, &diffBuff[0][0], 8);
        for (b = 0; b < 4; b ++) {
            Ipp32s a01, a23, b01, b23;

            a01 = diffBuff[b][0] + diffBuff[b][1];
            a23 = diffBuff[b][2] + diffBuff[b][3];
            b01 = diffBuff[b][0] - diffBuff[b][1];
            b23 = diffBuff[b][2] - diffBuff[b][3];
            tmpBuff[b][0] = a01 + a23;
            tmpBuff[b][1] = a01 - a23;
            tmpBuff[b][2] = b01 - b23;
            tmpBuff[b][3] = b01 + b23;
        }
        for (b = 0; b < 4; b ++) {
            Ipp32s a01, a23, b01, b23;

            a01 = tmpBuff[0][b] + tmpBuff[1][b];
            a23 = tmpBuff[2][b] + tmpBuff[3][b];
            b01 = tmpBuff[0][b] - tmpBuff[1][b];
            b23 = tmpBuff[2][b] - tmpBuff[3][b];
            satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
#else
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero, _p_satd;
    const Ipp8u *pS1, *pS2;
    Ipp32s  s;
    Ipp32u satd;

    pS1 = pSrc1;
    pS2 = pSrc2;
    _p_zero = _mm_setzero_si128();
    _p_0 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_1 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_0 = _mm_sub_epi16(_p_0, _p_4);
    _p_1 = _mm_sub_epi16(_p_1, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_2 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_3 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_2 = _mm_sub_epi16(_p_2, _p_4);
    _p_3 = _mm_sub_epi16(_p_3, _p_5);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_7 = _mm_subs_epi16(_p_2, _p_3);
    _p_2 = _mm_adds_epi16(_p_2, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_2);
    _p_0 = _mm_adds_epi16(_p_0, _p_2);
    _p_3 = _mm_adds_epi16(_p_5, _p_7);
    _p_5 = _mm_subs_epi16(_p_5, _p_7);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_1 = _mm_srli_si128(_p_0, 8);
    _p_3 = _mm_srli_si128(_p_7, 8);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_2 = _mm_subs_epi16(_p_7, _p_3);
    _p_7 = _mm_adds_epi16(_p_7, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_7);
    _p_0 = _mm_adds_epi16(_p_0, _p_7);
    _p_3 = _mm_adds_epi16(_p_5, _p_2);
    _p_5 = _mm_subs_epi16(_p_5, _p_2);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_3 = _mm_srai_epi16(_p_2, 15);
    _p_1 = _mm_srai_epi16(_p_0, 15);
    _p_2 = _mm_xor_si128(_p_2, _p_3);
    _p_0 = _mm_xor_si128(_p_0, _p_1);
    _p_2 = _mm_sub_epi16(_p_2, _p_3);
    _p_0 = _mm_sub_epi16(_p_0, _p_1);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _p_satd = _p_0;
    _p_0 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_1 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_0 = _mm_sub_epi16(_p_0, _p_4);
    _p_1 = _mm_sub_epi16(_p_1, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_2 = _mm_cvtsi32_si128(*(int*)(pS1));
    _p_4 = _mm_cvtsi32_si128(*(int*)(pS2));
    _p_3 = _mm_cvtsi32_si128(*(int*)(pS1+src1Step));
    _p_5 = _mm_cvtsi32_si128(*(int*)(pS2+src2Step));
    _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_2 = _mm_sub_epi16(_p_2, _p_4);
    _p_3 = _mm_sub_epi16(_p_3, _p_5);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_7 = _mm_subs_epi16(_p_2, _p_3);
    _p_2 = _mm_adds_epi16(_p_2, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_2);
    _p_0 = _mm_adds_epi16(_p_0, _p_2);
    _p_3 = _mm_adds_epi16(_p_5, _p_7);
    _p_5 = _mm_subs_epi16(_p_5, _p_7);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_1 = _mm_srli_si128(_p_0, 8);
    _p_3 = _mm_srli_si128(_p_7, 8);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_2 = _mm_subs_epi16(_p_7, _p_3);
    _p_7 = _mm_adds_epi16(_p_7, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_7);
    _p_0 = _mm_adds_epi16(_p_0, _p_7);
    _p_3 = _mm_adds_epi16(_p_5, _p_2);
    _p_5 = _mm_subs_epi16(_p_5, _p_2);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_3 = _mm_srai_epi16(_p_2, 15);
    _p_1 = _mm_srai_epi16(_p_0, 15);
    _p_2 = _mm_xor_si128(_p_2, _p_3);
    _p_0 = _mm_xor_si128(_p_0, _p_1);
    _p_2 = _mm_sub_epi16(_p_2, _p_3);
    _p_0 = _mm_sub_epi16(_p_0, _p_1);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _p_0 = _mm_add_epi16(_p_0, _p_satd);
    _p_2 = _mm_srli_si128(_p_0, 8);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    _p_2 = _mm_srli_si128(_p_0, 4);
    _p_0 = _mm_add_epi16(_p_0, _p_2);
    s = _mm_cvtsi128_si32(_p_0);
    satd = (s >> 16) + (Ipp16s)s;
#endif
    return satd >> 1;
}

Ipp32u SATD8x4_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x;
    Ipp32u satd = 0;

    for( x = 0; x < 8; x += 4 )  {
        Ipp32s b;
        ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
        for (b = 0; b < 4; b ++) {
            Ipp32s a01, a23, b01, b23;

            a01 = diffBuff[b][0] + diffBuff[b][1];
            a23 = diffBuff[b][2] + diffBuff[b][3];
            b01 = diffBuff[b][0] - diffBuff[b][1];
            b23 = diffBuff[b][2] - diffBuff[b][3];
            tmpBuff[b][0] = a01 + a23;
            tmpBuff[b][1] = a01 - a23;
            tmpBuff[b][2] = b01 - b23;
            tmpBuff[b][3] = b01 + b23;
        }
        for (b = 0; b < 4; b ++) {
            Ipp32s a01, a23, b01, b23;

            a01 = tmpBuff[0][b] + tmpBuff[1][b];
            a23 = tmpBuff[2][b] + tmpBuff[3][b];
            b01 = tmpBuff[0][b] - tmpBuff[1][b];
            b23 = tmpBuff[2][b] - tmpBuff[3][b];
            satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
        }
    }
#else
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    Ipp32s  s;
    Ipp32u  satd;

    pS1 = pSrc1;
    pS2 = pSrc2;
    _p_zero = _mm_setzero_si128();
    _p_0 = _mm_loadl_epi64((__m128i*)(pS1));
    _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
    _p_1 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
    _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
    _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_0 = _mm_sub_epi16(_p_0, _p_4);
    _p_1 = _mm_sub_epi16(_p_1, _p_5);
    pS1 += 2 * src1Step;
    pS2 += 2 * src2Step;
    _p_2 = _mm_loadl_epi64((__m128i*)(pS1));
    _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
    _p_3 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
    _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
    _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
    _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
    _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
    _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
    _p_2 = _mm_sub_epi16(_p_2, _p_4);
    _p_3 = _mm_sub_epi16(_p_3, _p_5);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_7 = _mm_subs_epi16(_p_2, _p_3);
    _p_2 = _mm_adds_epi16(_p_2, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_2);
    _p_0 = _mm_adds_epi16(_p_0, _p_2);
    _p_3 = _mm_adds_epi16(_p_5, _p_7);
    _p_5 = _mm_subs_epi16(_p_5, _p_7);

    __ALIGN16 __m128i  _r_0, _r_1, _r_2, _r_3, _r_5, _r_7;

    _r_0 = _mm_unpackhi_epi16(_p_0, _p_1);
    _r_5 = _mm_unpackhi_epi16(_p_5, _p_3);

    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_1 = _mm_srli_si128(_p_0, 8);
    _p_3 = _mm_srli_si128(_p_7, 8);
    _p_5 = _mm_subs_epi16(_p_0, _p_1);
    _p_0 = _mm_adds_epi16(_p_0, _p_1);
    _p_2 = _mm_subs_epi16(_p_7, _p_3);
    _p_7 = _mm_adds_epi16(_p_7, _p_3);
    _p_1 = _mm_subs_epi16(_p_0, _p_7);
    _p_0 = _mm_adds_epi16(_p_0, _p_7);
    _p_3 = _mm_adds_epi16(_p_5, _p_2);
    _p_5 = _mm_subs_epi16(_p_5, _p_2);
    _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
    _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
    _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
    _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
    _p_3 = _mm_srai_epi16(_p_2, 15);
    _p_1 = _mm_srai_epi16(_p_0, 15);
    _p_2 = _mm_xor_si128(_p_2, _p_3);
    _p_0 = _mm_xor_si128(_p_0, _p_1);
    _p_2 = _mm_sub_epi16(_p_2, _p_3);
    _p_0 = _mm_sub_epi16(_p_0, _p_1);
    _p_0 = _mm_add_epi16(_p_0, _p_2);

    _r_7 = _mm_unpackhi_epi32(_r_0, _r_5);
    _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
    _r_1 = _mm_srli_si128(_r_0, 8);
    _r_3 = _mm_srli_si128(_r_7, 8);
    _r_5 = _mm_subs_epi16(_r_0, _r_1);
    _r_0 = _mm_adds_epi16(_r_0, _r_1);
    _r_2 = _mm_subs_epi16(_r_7, _r_3);
    _r_7 = _mm_adds_epi16(_r_7, _r_3);
    _r_1 = _mm_subs_epi16(_r_0, _r_7);
    _r_0 = _mm_adds_epi16(_r_0, _r_7);
    _r_3 = _mm_adds_epi16(_r_5, _r_2);
    _r_5 = _mm_subs_epi16(_r_5, _r_2);
    _r_0 = _mm_unpacklo_epi16(_r_0, _r_1);
    _r_5 = _mm_unpacklo_epi16(_r_5, _r_3);
    _r_2 = _mm_unpackhi_epi32(_r_0, _r_5);
    _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
    _r_3 = _mm_srai_epi16(_r_2, 15);
    _r_1 = _mm_srai_epi16(_r_0, 15);
    _r_2 = _mm_xor_si128(_r_2, _r_3);
    _r_0 = _mm_xor_si128(_r_0, _r_1);
    _r_2 = _mm_sub_epi16(_r_2, _r_3);
    _r_0 = _mm_sub_epi16(_r_0, _r_1);
    _r_0 = _mm_add_epi16(_r_0, _r_2);
    _r_0 = _mm_add_epi16(_r_0, _p_0);
    _r_2 = _mm_srli_si128(_r_0, 8);
    _r_0 = _mm_add_epi16(_r_0, _r_2);
    _r_2 = _mm_srli_si128(_r_0, 4);
    _r_0 = _mm_add_epi16(_r_0, _r_2);
    s = _mm_cvtsi128_si32(_r_0);
    satd = (s >> 16) + (Ipp16s)s;
#endif
    return satd >> 1;
}

Ipp32u SATD8x8_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x, y;
    Ipp32u satd = 0;

    for (y = 0; y < 8; y += 4) {
        for (x = 0; x < 8; x += 4)  {
            Ipp32s b;
            ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
#else
    Ipp32s s, y;
    Ipp32u satd;
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    __ALIGN16 __m128i  _r_0, _r_1, _r_2, _r_3, _r_5, _r_7, _p_satd;

    _p_zero = _mm_setzero_si128();
    _p_satd = _mm_setzero_si128();
    pS1 = pSrc1;
    pS2 = pSrc2;
    for( y = 0; y < 8; y += 4 ) {
        _p_0 = _mm_loadl_epi64((__m128i*)(pS1));
        _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
        _p_1 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
        _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
        _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
        _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
        _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
        _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
        _p_0 = _mm_sub_epi16(_p_0, _p_4);
        _p_1 = _mm_sub_epi16(_p_1, _p_5);
        pS1 += 2 * src1Step;
        pS2 += 2 * src2Step;
        _p_2 = _mm_loadl_epi64((__m128i*)(pS1));
        _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
        _p_3 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
        _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
        _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
        _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
        _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
        _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
        _p_2 = _mm_sub_epi16(_p_2, _p_4);
        _p_3 = _mm_sub_epi16(_p_3, _p_5);
        _p_5 = _mm_subs_epi16(_p_0, _p_1);
        _p_0 = _mm_adds_epi16(_p_0, _p_1);
        _p_7 = _mm_subs_epi16(_p_2, _p_3);
        _p_2 = _mm_adds_epi16(_p_2, _p_3);
        _p_1 = _mm_subs_epi16(_p_0, _p_2);
        _p_0 = _mm_adds_epi16(_p_0, _p_2);
        _p_3 = _mm_adds_epi16(_p_5, _p_7);
        _p_5 = _mm_subs_epi16(_p_5, _p_7);

        _r_0 = _mm_unpackhi_epi16(_p_0, _p_1);
        _r_5 = _mm_unpackhi_epi16(_p_5, _p_3);
        pS1 += 2 * src1Step;
        pS2 += 2 * src2Step;

        _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
        _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
        _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
        _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
        _p_1 = _mm_srli_si128(_p_0, 8);
        _p_3 = _mm_srli_si128(_p_7, 8);
        _p_5 = _mm_subs_epi16(_p_0, _p_1);
        _p_0 = _mm_adds_epi16(_p_0, _p_1);
        _p_2 = _mm_subs_epi16(_p_7, _p_3);
        _p_7 = _mm_adds_epi16(_p_7, _p_3);
        _p_1 = _mm_subs_epi16(_p_0, _p_7);
        _p_0 = _mm_adds_epi16(_p_0, _p_7);
        _p_3 = _mm_adds_epi16(_p_5, _p_2);
        _p_5 = _mm_subs_epi16(_p_5, _p_2);
        _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
        _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
        _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
        _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
        _p_3 = _mm_srai_epi16(_p_2, 15);
        _p_1 = _mm_srai_epi16(_p_0, 15);
        _p_2 = _mm_xor_si128(_p_2, _p_3);
        _p_0 = _mm_xor_si128(_p_0, _p_1);
        _p_2 = _mm_sub_epi16(_p_2, _p_3);
        _p_0 = _mm_sub_epi16(_p_0, _p_1);
        _p_0 = _mm_add_epi16(_p_0, _p_2);
        _p_satd = _mm_add_epi16(_p_satd, _p_0);

        _r_7 = _mm_unpackhi_epi32(_r_0, _r_5);
        _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
        _r_1 = _mm_srli_si128(_r_0, 8);
        _r_3 = _mm_srli_si128(_r_7, 8);
        _r_5 = _mm_subs_epi16(_r_0, _r_1);
        _r_0 = _mm_adds_epi16(_r_0, _r_1);
        _r_2 = _mm_subs_epi16(_r_7, _r_3);
        _r_7 = _mm_adds_epi16(_r_7, _r_3);
        _r_1 = _mm_subs_epi16(_r_0, _r_7);
        _r_0 = _mm_adds_epi16(_r_0, _r_7);
        _r_3 = _mm_adds_epi16(_r_5, _r_2);
        _r_5 = _mm_subs_epi16(_r_5, _r_2);
        _r_0 = _mm_unpacklo_epi16(_r_0, _r_1);
        _r_5 = _mm_unpacklo_epi16(_r_5, _r_3);
        _r_2 = _mm_unpackhi_epi32(_r_0, _r_5);
        _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
        _r_3 = _mm_srai_epi16(_r_2, 15);
        _r_1 = _mm_srai_epi16(_r_0, 15);
        _r_2 = _mm_xor_si128(_r_2, _r_3);
        _r_0 = _mm_xor_si128(_r_0, _r_1);
        _r_2 = _mm_sub_epi16(_r_2, _r_3);
        _r_0 = _mm_sub_epi16(_r_0, _r_1);
        _r_0 = _mm_add_epi16(_r_0, _r_2);
        _p_satd = _mm_add_epi16(_p_satd, _r_0);
    }
    _p_2 = _mm_srli_si128(_p_satd, 8);
    _p_satd = _mm_add_epi16(_p_satd, _p_2);
    _p_2 = _mm_srli_si128(_p_satd, 4);
    _p_satd = _mm_add_epi16(_p_satd, _p_2);
    s = _mm_cvtsi128_si32(_p_satd);
    satd = (s >> 16) + (Ipp16s)s;
#endif
    return satd >> 1;
}

Ipp32u SATD8x16_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x, y;
    Ipp32u satd = 0;

    for (y = 0; y < 16; y += 4) {
        for (x = 0; x < 8; x += 4)  {
            Ipp32s b;
            ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
#else
    Ipp32s y;
    Ipp32u satd;
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    __ALIGN16 __m128i  _r_0, _r_1, _r_2, _r_3, _r_5, _r_7, _p_satd;
    static __ALIGN16 Ipp16s _p_one[8] = {1, 1, 1, 1, 1, 1, 1, 1};

    _p_zero = _mm_setzero_si128();
    _p_satd = _mm_setzero_si128();
    pS1 = pSrc1;
    pS2 = pSrc2;
    for( y = 0; y < 16; y += 4 ) {
        _p_0 = _mm_loadl_epi64((__m128i*)(pS1));
        _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
        _p_1 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
        _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
        _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
        _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
        _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
        _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
        _p_0 = _mm_sub_epi16(_p_0, _p_4);
        _p_1 = _mm_sub_epi16(_p_1, _p_5);
        pS1 += 2 * src1Step;
        pS2 += 2 * src2Step;
        _p_2 = _mm_loadl_epi64((__m128i*)(pS1));
        _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
        _p_3 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
        _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
        _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
        _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
        _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
        _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
        _p_2 = _mm_sub_epi16(_p_2, _p_4);
        _p_3 = _mm_sub_epi16(_p_3, _p_5);
        _p_5 = _mm_subs_epi16(_p_0, _p_1);
        _p_0 = _mm_adds_epi16(_p_0, _p_1);
        _p_7 = _mm_subs_epi16(_p_2, _p_3);
        _p_2 = _mm_adds_epi16(_p_2, _p_3);
        _p_1 = _mm_subs_epi16(_p_0, _p_2);
        _p_0 = _mm_adds_epi16(_p_0, _p_2);
        _p_3 = _mm_adds_epi16(_p_5, _p_7);
        _p_5 = _mm_subs_epi16(_p_5, _p_7);

        _r_0 = _mm_unpackhi_epi16(_p_0, _p_1);
        _r_5 = _mm_unpackhi_epi16(_p_5, _p_3);
        pS1 += 2 * src1Step;
        pS2 += 2 * src2Step;

        _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
        _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
        _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
        _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
        _p_1 = _mm_srli_si128(_p_0, 8);
        _p_3 = _mm_srli_si128(_p_7, 8);
        _p_5 = _mm_subs_epi16(_p_0, _p_1);
        _p_0 = _mm_adds_epi16(_p_0, _p_1);
        _p_2 = _mm_subs_epi16(_p_7, _p_3);
        _p_7 = _mm_adds_epi16(_p_7, _p_3);
        _p_1 = _mm_subs_epi16(_p_0, _p_7);
        _p_0 = _mm_adds_epi16(_p_0, _p_7);
        _p_3 = _mm_adds_epi16(_p_5, _p_2);
        _p_5 = _mm_subs_epi16(_p_5, _p_2);
        _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
        _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
        _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
        _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
        _p_3 = _mm_srai_epi16(_p_2, 15);
        _p_1 = _mm_srai_epi16(_p_0, 15);
        _p_2 = _mm_xor_si128(_p_2, _p_3);
        _p_0 = _mm_xor_si128(_p_0, _p_1);
        _p_2 = _mm_sub_epi16(_p_2, _p_3);
        _p_0 = _mm_sub_epi16(_p_0, _p_1);
        _p_0 = _mm_add_epi16(_p_0, _p_2);
        _p_0 = _mm_madd_epi16(_p_0, *(__m128i*)_p_one);
        _p_satd = _mm_add_epi32(_p_satd, _p_0);

        _r_7 = _mm_unpackhi_epi32(_r_0, _r_5);
        _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
        _r_1 = _mm_srli_si128(_r_0, 8);
        _r_3 = _mm_srli_si128(_r_7, 8);
        _r_5 = _mm_subs_epi16(_r_0, _r_1);
        _r_0 = _mm_adds_epi16(_r_0, _r_1);
        _r_2 = _mm_subs_epi16(_r_7, _r_3);
        _r_7 = _mm_adds_epi16(_r_7, _r_3);
        _r_1 = _mm_subs_epi16(_r_0, _r_7);
        _r_0 = _mm_adds_epi16(_r_0, _r_7);
        _r_3 = _mm_adds_epi16(_r_5, _r_2);
        _r_5 = _mm_subs_epi16(_r_5, _r_2);
        _r_0 = _mm_unpacklo_epi16(_r_0, _r_1);
        _r_5 = _mm_unpacklo_epi16(_r_5, _r_3);
        _r_2 = _mm_unpackhi_epi32(_r_0, _r_5);
        _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
        _r_3 = _mm_srai_epi16(_r_2, 15);
        _r_1 = _mm_srai_epi16(_r_0, 15);
        _r_2 = _mm_xor_si128(_r_2, _r_3);
        _r_0 = _mm_xor_si128(_r_0, _r_1);
        _r_2 = _mm_sub_epi16(_r_2, _r_3);
        _r_0 = _mm_sub_epi16(_r_0, _r_1);
        _r_0 = _mm_add_epi16(_r_0, _r_2);
        _r_0 = _mm_madd_epi16(_r_0, *(__m128i*)_p_one);
        _p_satd = _mm_add_epi32(_p_satd, _r_0);
    }
    _p_1 = _mm_srli_si128(_p_satd, 8);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    _p_1 = _mm_srli_si128(_p_satd, 4);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    satd = _mm_cvtsi128_si32(_p_satd);
#endif
    return satd >> 1;
}

Ipp32u SATD16x8_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x, y;
    Ipp32u satd = 0;

    for (y = 0; y < 8; y += 4) {
        for (x = 0; x < 16; x += 4)  {
            Ipp32s b;
            ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
#else
    Ipp32s x, y;
    Ipp32u satd;
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    __ALIGN16 __m128i  _r_0, _r_1, _r_2, _r_3, _r_5, _r_7, _p_satd;
    static __ALIGN16 Ipp16s _p_one[8] = {1, 1, 1, 1, 1, 1, 1, 1};

    _p_zero = _mm_setzero_si128();
    _p_satd = _mm_setzero_si128();
    for (y = 0; y < 8; y += 4) {
        for (x = 0; x < 16; x += 8) {
            pS1 = pSrc1 + x;
            pS2 = pSrc2 + x;
            _p_0 = _mm_loadl_epi64((__m128i*)(pS1));
            _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
            _p_1 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
            _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
            _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_0 = _mm_sub_epi16(_p_0, _p_4);
            _p_1 = _mm_sub_epi16(_p_1, _p_5);
            pS1 += 2 * src1Step;
            pS2 += 2 * src2Step;
            _p_2 = _mm_loadl_epi64((__m128i*)(pS1));
            _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
            _p_3 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
            _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
            _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_2 = _mm_sub_epi16(_p_2, _p_4);
            _p_3 = _mm_sub_epi16(_p_3, _p_5);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_7 = _mm_subs_epi16(_p_2, _p_3);
            _p_2 = _mm_adds_epi16(_p_2, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_2);
            _p_0 = _mm_adds_epi16(_p_0, _p_2);
            _p_3 = _mm_adds_epi16(_p_5, _p_7);
            _p_5 = _mm_subs_epi16(_p_5, _p_7);

            _r_0 = _mm_unpackhi_epi16(_p_0, _p_1);
            _r_5 = _mm_unpackhi_epi16(_p_5, _p_3);

            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_1 = _mm_srli_si128(_p_0, 8);
            _p_3 = _mm_srli_si128(_p_7, 8);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_2 = _mm_subs_epi16(_p_7, _p_3);
            _p_7 = _mm_adds_epi16(_p_7, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_7);
            _p_0 = _mm_adds_epi16(_p_0, _p_7);
            _p_3 = _mm_adds_epi16(_p_5, _p_2);
            _p_5 = _mm_subs_epi16(_p_5, _p_2);
            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_3 = _mm_srai_epi16(_p_2, 15);
            _p_1 = _mm_srai_epi16(_p_0, 15);
            _p_2 = _mm_xor_si128(_p_2, _p_3);
            _p_0 = _mm_xor_si128(_p_0, _p_1);
            _p_2 = _mm_sub_epi16(_p_2, _p_3);
            _p_0 = _mm_sub_epi16(_p_0, _p_1);
            _p_0 = _mm_add_epi16(_p_0, _p_2);
            _p_0 = _mm_madd_epi16(_p_0, *(__m128i*)_p_one);
            _p_satd = _mm_add_epi32(_p_satd, _p_0);

            _r_7 = _mm_unpackhi_epi32(_r_0, _r_5);
            _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
            _r_1 = _mm_srli_si128(_r_0, 8);
            _r_3 = _mm_srli_si128(_r_7, 8);
            _r_5 = _mm_subs_epi16(_r_0, _r_1);
            _r_0 = _mm_adds_epi16(_r_0, _r_1);
            _r_2 = _mm_subs_epi16(_r_7, _r_3);
            _r_7 = _mm_adds_epi16(_r_7, _r_3);
            _r_1 = _mm_subs_epi16(_r_0, _r_7);
            _r_0 = _mm_adds_epi16(_r_0, _r_7);
            _r_3 = _mm_adds_epi16(_r_5, _r_2);
            _r_5 = _mm_subs_epi16(_r_5, _r_2);
            _r_0 = _mm_unpacklo_epi16(_r_0, _r_1);
            _r_5 = _mm_unpacklo_epi16(_r_5, _r_3);
            _r_2 = _mm_unpackhi_epi32(_r_0, _r_5);
            _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
            _r_3 = _mm_srai_epi16(_r_2, 15);
            _r_1 = _mm_srai_epi16(_r_0, 15);
            _r_2 = _mm_xor_si128(_r_2, _r_3);
            _r_0 = _mm_xor_si128(_r_0, _r_1);
            _r_2 = _mm_sub_epi16(_r_2, _r_3);
            _r_0 = _mm_sub_epi16(_r_0, _r_1);
            _r_0 = _mm_add_epi16(_r_0, _r_2);
            _r_0 = _mm_madd_epi16(_r_0, *(__m128i*)_p_one);
            _p_satd = _mm_add_epi32(_p_satd, _r_0);
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
    _p_1 = _mm_srli_si128(_p_satd, 8);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    _p_1 = _mm_srli_si128(_p_satd, 4);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    satd = _mm_cvtsi128_si32(_p_satd);
#endif
    return satd >> 1;
}

Ipp32u SATD16x16_8u_C1R(const Ipp8u *pSrc1, Ipp32s src1Step, const Ipp8u *pSrc2, Ipp32s src2Step)
{
#ifndef INTRINSIC_OPT
    __ALIGN16 Ipp16s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x, y;
    Ipp32u satd = 0;

    for (y = 0; y < 16; y += 4) {
        for (x = 0; x < 16; x += 4)  {
            Ipp32s b;
            ippiSub4x4_8u16s_C1R(pSrc1 + x, src1Step, pSrc2 + x, src2Step, &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
#else
    Ipp32s x, y;
    Ipp32u satd;
    __ALIGN16 __m128i  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_7, _p_zero;
    const Ipp8u *pS1, *pS2;
    __ALIGN16 __m128i  _r_0, _r_1, _r_2, _r_3, _r_5, _r_7, _p_satd;
    static __ALIGN16 Ipp16s _p_one[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    _p_zero = _mm_setzero_si128();
    _p_satd = _mm_setzero_si128();
    for (y = 0; y < 16; y += 4) {
        for (x = 0; x < 16; x += 8) {
            pS1 = pSrc1 + x;
            pS2 = pSrc2 + x;
            _p_0 = _mm_loadl_epi64((__m128i*)(pS1));
            _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
            _p_1 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
            _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
            _p_0 = _mm_unpacklo_epi8(_p_0, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_1 = _mm_unpacklo_epi8(_p_1, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_0 = _mm_sub_epi16(_p_0, _p_4);
            _p_1 = _mm_sub_epi16(_p_1, _p_5);
            pS1 += 2 * src1Step;
            pS2 += 2 * src2Step;
            _p_2 = _mm_loadl_epi64((__m128i*)(pS1));
            _p_4 = _mm_loadl_epi64((__m128i*)(pS2));
            _p_3 = _mm_loadl_epi64((__m128i*)(pS1+src1Step));
            _p_5 = _mm_loadl_epi64((__m128i*)(pS2+src2Step));
            _p_2 = _mm_unpacklo_epi8(_p_2, _p_zero);
            _p_4 = _mm_unpacklo_epi8(_p_4, _p_zero);
            _p_3 = _mm_unpacklo_epi8(_p_3, _p_zero);
            _p_5 = _mm_unpacklo_epi8(_p_5, _p_zero);
            _p_2 = _mm_sub_epi16(_p_2, _p_4);
            _p_3 = _mm_sub_epi16(_p_3, _p_5);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_7 = _mm_subs_epi16(_p_2, _p_3);
            _p_2 = _mm_adds_epi16(_p_2, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_2);
            _p_0 = _mm_adds_epi16(_p_0, _p_2);
            _p_3 = _mm_adds_epi16(_p_5, _p_7);
            _p_5 = _mm_subs_epi16(_p_5, _p_7);

            _r_0 = _mm_unpackhi_epi16(_p_0, _p_1);
            _r_5 = _mm_unpackhi_epi16(_p_5, _p_3);

            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_7 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_1 = _mm_srli_si128(_p_0, 8);
            _p_3 = _mm_srli_si128(_p_7, 8);
            _p_5 = _mm_subs_epi16(_p_0, _p_1);
            _p_0 = _mm_adds_epi16(_p_0, _p_1);
            _p_2 = _mm_subs_epi16(_p_7, _p_3);
            _p_7 = _mm_adds_epi16(_p_7, _p_3);
            _p_1 = _mm_subs_epi16(_p_0, _p_7);
            _p_0 = _mm_adds_epi16(_p_0, _p_7);
            _p_3 = _mm_adds_epi16(_p_5, _p_2);
            _p_5 = _mm_subs_epi16(_p_5, _p_2);
            _p_0 = _mm_unpacklo_epi16(_p_0, _p_1);
            _p_5 = _mm_unpacklo_epi16(_p_5, _p_3);
            _p_2 = _mm_unpackhi_epi32(_p_0, _p_5);
            _p_0 = _mm_unpacklo_epi32(_p_0, _p_5);
            _p_3 = _mm_srai_epi16(_p_2, 15);
            _p_1 = _mm_srai_epi16(_p_0, 15);
            _p_2 = _mm_xor_si128(_p_2, _p_3);
            _p_0 = _mm_xor_si128(_p_0, _p_1);
            _p_2 = _mm_sub_epi16(_p_2, _p_3);
            _p_0 = _mm_sub_epi16(_p_0, _p_1);
            _p_0 = _mm_add_epi16(_p_0, _p_2);
            _p_0 = _mm_madd_epi16(_p_0, *(__m128i*)_p_one);
            _p_satd = _mm_add_epi32(_p_satd, _p_0);

            _r_7 = _mm_unpackhi_epi32(_r_0, _r_5);
            _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
            _r_1 = _mm_srli_si128(_r_0, 8);
            _r_3 = _mm_srli_si128(_r_7, 8);
            _r_5 = _mm_subs_epi16(_r_0, _r_1);
            _r_0 = _mm_adds_epi16(_r_0, _r_1);
            _r_2 = _mm_subs_epi16(_r_7, _r_3);
            _r_7 = _mm_adds_epi16(_r_7, _r_3);
            _r_1 = _mm_subs_epi16(_r_0, _r_7);
            _r_0 = _mm_adds_epi16(_r_0, _r_7);
            _r_3 = _mm_adds_epi16(_r_5, _r_2);
            _r_5 = _mm_subs_epi16(_r_5, _r_2);
            _r_0 = _mm_unpacklo_epi16(_r_0, _r_1);
            _r_5 = _mm_unpacklo_epi16(_r_5, _r_3);
            _r_2 = _mm_unpackhi_epi32(_r_0, _r_5);
            _r_0 = _mm_unpacklo_epi32(_r_0, _r_5);
            _r_3 = _mm_srai_epi16(_r_2, 15);
            _r_1 = _mm_srai_epi16(_r_0, 15);
            _r_2 = _mm_xor_si128(_r_2, _r_3);
            _r_0 = _mm_xor_si128(_r_0, _r_1);
            _r_2 = _mm_sub_epi16(_r_2, _r_3);
            _r_0 = _mm_sub_epi16(_r_0, _r_1);
            _r_0 = _mm_add_epi16(_r_0, _r_2);
            _r_0 = _mm_madd_epi16(_r_0, *(__m128i*)_p_one);
            _p_satd = _mm_add_epi32(_p_satd, _r_0);
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
    _p_1 = _mm_srli_si128(_p_satd, 8);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    _p_1 = _mm_srli_si128(_p_satd, 4);
    _p_satd = _mm_add_epi32(_p_satd, _p_1);
    satd = _mm_cvtsi128_si32(_p_satd);
#endif
    return satd >> 1;
}

Ipp32u SATD_16u_C1R(const Ipp16u *pSrc1, Ipp32s src1Step, const Ipp16u *pSrc2, Ipp32s src2Step, Ipp32s width, Ipp32s height)
{
    __ALIGN16 Ipp32s tmpBuff[4][4];
    __ALIGN16 Ipp16s diffBuff[4][4];
    Ipp32s x, y;
    Ipp32u satd = 0;

    for( y = 0; y < height; y += 4 ) {
        for( x = 0; x < width; x += 4 )  {
            Ipp32s b;
            ippiSub4x4_16u16s_C1R(pSrc1 + x, src1Step*sizeof(Ipp16u), pSrc2 + x, src2Step*sizeof(Ipp16u), &diffBuff[0][0], 8);
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = diffBuff[b][0] + diffBuff[b][1];
                a23 = diffBuff[b][2] + diffBuff[b][3];
                b01 = diffBuff[b][0] - diffBuff[b][1];
                b23 = diffBuff[b][2] - diffBuff[b][3];
                tmpBuff[b][0] = a01 + a23;
                tmpBuff[b][1] = a01 - a23;
                tmpBuff[b][2] = b01 - b23;
                tmpBuff[b][3] = b01 + b23;
            }
            for (b = 0; b < 4; b ++) {
                Ipp32s a01, a23, b01, b23;

                a01 = tmpBuff[0][b] + tmpBuff[1][b];
                a23 = tmpBuff[2][b] + tmpBuff[3][b];
                b01 = tmpBuff[0][b] - tmpBuff[1][b];
                b23 = tmpBuff[2][b] - tmpBuff[3][b];
                satd += ABS(a01 + a23) + ABS(a01 - a23) + ABS(b01 - b23) + ABS(b01 + b23);
            }
        }
        pSrc1 += 4 * src1Step;
        pSrc2 += 4 * src2Step;
    }
    return satd >> 1;
}


void __STDCALL SAD8x4_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
#ifndef INTRINSIC_OPT
    int     i, j, r;

    r = 0;
    for (i = 0; i < 4; i ++, pSrcRef += srcRefStep, pSrcCur += srcCurStep)
        for (j = 0; j < 8; j ++)
            r += abs(pSrcCur[j] - pSrcRef[j]);
    *pDst = r;
#else
    __ALIGN8  __m64  _p_0, _p_1, _p_2, _p_3;

    _p_0 = *((__m64*)(pSrcRef));
    _p_1 = *((__m64*)(pSrcRef+srcRefStep));
    _p_2 = *((__m64*)(pSrcRef+2*srcRefStep));
    _p_3 = *((__m64*)(pSrcRef+srcRefStep + 2 * srcRefStep));
    _p_0 = _mm_sad_pu8(_p_0, *((__m64*)(pSrcCur)));
    _p_1 = _mm_sad_pu8(_p_1, *((__m64*)(pSrcCur+srcCurStep)));
    _p_2 = _mm_sad_pu8(_p_2, *((__m64*)(pSrcCur+2*srcCurStep)));
    _p_3 = _mm_sad_pu8(_p_3, *((__m64*)(pSrcCur+srcCurStep + 2 * srcCurStep)));
    _p_0 = _mm_add_pi16(_p_0, _p_1);
    _p_2 = _mm_add_pi16(_p_2, _p_3);
    _p_0 = _mm_add_pi16(_p_0, _p_2);
    *pDst = _mm_cvtsi64_si32(_p_0);
    _m_empty();
#endif
}


void __STDCALL SAD4x8_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
#ifndef INTRINSIC_OPT
    int     i, j, r;

    r = 0;
    for (i = 0; i < 8; i ++, pSrcRef += srcRefStep, pSrcCur += srcCurStep)
        for (j = 0; j < 4; j ++)
            r += abs(pSrcCur[j] - pSrcRef[j]);
    *pDst = r;
#else
    int  s0, s1;
    __ALIGN8  __m64  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_6, _p_7, _p_8, _p_9;

    _p_0 = _mm_cvtsi32_si64(*(int*)(pSrcRef));
    _p_1 = _mm_cvtsi32_si64(*(int*)(pSrcRef+srcRefStep));
    _p_8 = _mm_cvtsi32_si64(*(int*)(pSrcCur));
    _p_9 = _mm_cvtsi32_si64(*(int*)(pSrcCur+srcCurStep));
    _p_0 = _mm_sad_pu8(_p_0, _p_8);
    _p_1 = _mm_sad_pu8(_p_1, _p_9);
    pSrcRef += 2 * srcRefStep;
    pSrcCur += 2 * srcCurStep;
    _p_2 = _mm_cvtsi32_si64(*(int*)(pSrcRef));
    _p_3 = _mm_cvtsi32_si64(*(int*)(pSrcRef+srcRefStep));
    _p_8 = _mm_cvtsi32_si64(*(int*)(pSrcCur));
    _p_9 = _mm_cvtsi32_si64(*(int*)(pSrcCur+srcCurStep));
    _p_2 = _mm_sad_pu8(_p_2, _p_8);
    _p_3 = _mm_sad_pu8(_p_3, _p_9);
    pSrcRef += 2 * srcRefStep;
    pSrcCur += 2 * srcCurStep;
    _p_4 = _mm_cvtsi32_si64(*(int*)(pSrcRef));
    _p_5 = _mm_cvtsi32_si64(*(int*)(pSrcRef+srcRefStep));
    _p_8 = _mm_cvtsi32_si64(*(int*)(pSrcCur));
    _p_9 = _mm_cvtsi32_si64(*(int*)(pSrcCur+srcCurStep));
    _p_4 = _mm_sad_pu8(_p_4, _p_8);
    _p_5 = _mm_sad_pu8(_p_5, _p_9);
    pSrcRef += 2 * srcRefStep;
    pSrcCur += 2 * srcCurStep;
    _p_6 = _mm_cvtsi32_si64(*(int*)(pSrcRef));
    _p_7 = _mm_cvtsi32_si64(*(int*)(pSrcRef+srcRefStep));
    _p_8 = _mm_cvtsi32_si64(*(int*)(pSrcCur));
    _p_9 = _mm_cvtsi32_si64(*(int*)(pSrcCur+srcCurStep));
    _p_6 = _mm_sad_pu8(_p_6, _p_8);
    _p_7 = _mm_sad_pu8(_p_7, _p_9);
    _p_0 = _mm_add_pi16(_p_0, _p_1);
    _p_2 = _mm_add_pi16(_p_2, _p_3);
    _p_4 = _mm_add_pi16(_p_4, _p_5);
    _p_6 = _mm_add_pi16(_p_6, _p_7);
    _p_0 = _mm_add_pi16(_p_0, _p_2);
    _p_4 = _mm_add_pi16(_p_4, _p_6);
    s0 = _mm_cvtsi64_si32(_p_0);
    s1 = _mm_cvtsi64_si32(_p_4);
    _m_empty();
    *pDst = s0 + s1;
#endif
}


void __STDCALL SAD8x16_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
#ifndef INTRINSIC_OPT
    int     i, j, r;

    r = 0;
    for (i = 0; i < 16; i ++, pSrcRef += srcRefStep, pSrcCur += srcCurStep)
        for (j = 0; j < 8; j ++)
            r += abs(pSrcCur[j] - pSrcRef[j]);
    *pDst = r;
#else
    int  srcCurStep3, srcRefStep3, s0, s1;
    __ALIGN8  __m64  _p_0, _p_1, _p_2, _p_3, _p_4, _p_5, _p_6, _p_7;

    srcCurStep3 = srcCurStep + 2 * srcCurStep;
    srcRefStep3 = srcRefStep + 2 * srcRefStep;
    _p_0 = *((__m64*)(pSrcRef));
    _p_1 = *((__m64*)(pSrcRef+srcRefStep));
    _p_2 = *((__m64*)(pSrcRef+2*srcRefStep));
    _p_3 = *((__m64*)(pSrcRef+srcRefStep3));
    _p_0 = _mm_sad_pu8(_p_0, *((__m64*)(pSrcCur)));
    _p_1 = _mm_sad_pu8(_p_1, *((__m64*)(pSrcCur+srcCurStep)));
    _p_2 = _mm_sad_pu8(_p_2, *((__m64*)(pSrcCur+2*srcCurStep)));
    _p_3 = _mm_sad_pu8(_p_3, *((__m64*)(pSrcCur+srcCurStep3)));
    pSrcRef += 4 * srcRefStep;
    pSrcCur += 4 * srcCurStep;
    _p_4 = *((__m64*)(pSrcRef));
    _p_5 = *((__m64*)(pSrcRef+srcRefStep));
    _p_6 = *((__m64*)(pSrcRef+2*srcRefStep));
    _p_7 = *((__m64*)(pSrcRef+srcRefStep3));
    _p_4 = _mm_sad_pu8(_p_4, *((__m64*)(pSrcCur)));
    _p_5 = _mm_sad_pu8(_p_5, *((__m64*)(pSrcCur+srcCurStep)));
    _p_6 = _mm_sad_pu8(_p_6, *((__m64*)(pSrcCur+2*srcCurStep)));
    _p_7 = _mm_sad_pu8(_p_7, *((__m64*)(pSrcCur+srcCurStep3)));
    _p_0 = _mm_add_pi16(_p_0, _p_1);
    _p_2 = _mm_add_pi16(_p_2, _p_3);
    _p_4 = _mm_add_pi16(_p_4, _p_5);
    _p_6 = _mm_add_pi16(_p_6, _p_7);
    pSrcRef += 4 * srcRefStep;
    pSrcCur += 4 * srcCurStep;
    _p_1 = *((__m64*)(pSrcRef));
    _p_3 = *((__m64*)(pSrcRef+srcRefStep));
    _p_5 = *((__m64*)(pSrcRef+2*srcRefStep));
    _p_7 = *((__m64*)(pSrcRef+srcRefStep3));
    _p_1 = _mm_sad_pu8(_p_1, *((__m64*)(pSrcCur)));
    _p_3 = _mm_sad_pu8(_p_3, *((__m64*)(pSrcCur+srcCurStep)));
    _p_5 = _mm_sad_pu8(_p_5, *((__m64*)(pSrcCur+2*srcCurStep)));
    _p_7 = _mm_sad_pu8(_p_7, *((__m64*)(pSrcCur+srcCurStep3)));
    _p_0 = _mm_add_pi16(_p_0, _p_1);
    _p_2 = _mm_add_pi16(_p_2, _p_3);
    _p_4 = _mm_add_pi16(_p_4, _p_5);
    _p_6 = _mm_add_pi16(_p_6, _p_7);
    pSrcRef += 4 * srcRefStep;
    pSrcCur += 4 * srcCurStep;
    _p_1 = *((__m64*)(pSrcRef));
    _p_3 = *((__m64*)(pSrcRef+srcRefStep));
    _p_5 = *((__m64*)(pSrcRef+2*srcRefStep));
    _p_7 = *((__m64*)(pSrcRef+srcRefStep3));
    _p_1 = _mm_sad_pu8(_p_1, *((__m64*)(pSrcCur)));
    _p_3 = _mm_sad_pu8(_p_3, *((__m64*)(pSrcCur+srcCurStep)));
    _p_5 = _mm_sad_pu8(_p_5, *((__m64*)(pSrcCur+2*srcCurStep)));
    _p_7 = _mm_sad_pu8(_p_7, *((__m64*)(pSrcCur+srcCurStep3)));
    _p_0 = _mm_add_pi16(_p_0, _p_1);
    _p_2 = _mm_add_pi16(_p_2, _p_3);
    _p_4 = _mm_add_pi16(_p_4, _p_5);
    _p_6 = _mm_add_pi16(_p_6, _p_7);

    _p_0 = _mm_add_pi16(_p_0, _p_2);
    _p_4 = _mm_add_pi16(_p_4, _p_6);
    s0 = _mm_cvtsi64_si32(_p_0);
    s1 = _mm_cvtsi64_si32(_p_4);
    _m_empty();
    *pDst = s0 + s1;
#endif
}


#ifdef TABLE_FUNC

void __STDCALL SAD4x2_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
    Ipp32s d0 = pSrcCur[0] - pSrcRef[0];
    Ipp32s d1 = pSrcCur[1] - pSrcRef[1];
    Ipp32s d2 = pSrcCur[srcCurStep+0] - pSrcRef[srcRefStep+0];
    Ipp32s d3 = pSrcCur[srcCurStep+1] - pSrcRef[srcRefStep+1];
    Ipp32s d4 = pSrcCur[2] - pSrcRef[2];
    Ipp32s d5 = pSrcCur[3] - pSrcRef[3];
    Ipp32s d6 = pSrcCur[srcCurStep+2] - pSrcRef[srcRefStep+2];
    Ipp32s d7 = pSrcCur[srcCurStep+3] - pSrcRef[srcRefStep+3];
    *pDst = ABS(d0) + ABS(d1) + ABS(d2) + ABS(d3) + ABS(d4) + ABS(d5) + ABS(d6) + ABS(d7);
}


void __STDCALL SAD2x4_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
    Ipp32s d0 = pSrcCur[0] - pSrcRef[0];
    Ipp32s d1 = pSrcCur[1] - pSrcRef[1];
    Ipp32s d2 = pSrcCur[srcCurStep+0] - pSrcRef[srcRefStep+0];
    Ipp32s d3 = pSrcCur[srcCurStep+1] - pSrcRef[srcRefStep+1];
    pSrcCur += srcCurStep * 2;
    pSrcRef += srcRefStep * 2;
    Ipp32s d4 = pSrcCur[0] - pSrcRef[0];
    Ipp32s d5 = pSrcCur[1] - pSrcRef[1];
    Ipp32s d6 = pSrcCur[srcCurStep+0] - pSrcRef[srcRefStep+0];
    Ipp32s d7 = pSrcCur[srcCurStep+1] - pSrcRef[srcRefStep+1];
    *pDst = ABS(d0) + ABS(d1) + ABS(d2) + ABS(d3) + ABS(d4) + ABS(d5) + ABS(d6) + ABS(d7);
}


void __STDCALL SAD2x2_8u32s_C1R(const Ipp8u* pSrcCur, int srcCurStep, const Ipp8u* pSrcRef, int srcRefStep, Ipp32s* pDst, Ipp32s mc)
{
    H264ENC_UNREFERENCED_PARAMETER(mc);
    Ipp32s d0 = pSrcCur[0] - pSrcRef[0];
    Ipp32s d1 = pSrcCur[1] - pSrcRef[1];
    Ipp32s d2 = pSrcCur[srcCurStep+0] - pSrcRef[srcRefStep+0];
    Ipp32s d3 = pSrcCur[srcCurStep+1] - pSrcRef[srcRefStep+1];
    *pDst = ABS(d0) + ABS(d1) + ABS(d2) + ABS(d3);
}


Ipp32u SATD4x2_8u_C1R(const Ipp8u *pCur, Ipp32s pitchPixelsCur, const Ipp8u *pRef, Ipp32s pitchPixelsRef)
{
    Ipp32s d0 = pCur[0] - pRef[0];
    Ipp32s d1 = pCur[1] - pRef[1];
    Ipp32s d2 = pCur[pitchPixelsCur+0] - pRef[pitchPixelsRef+0];
    Ipp32s d3 = pCur[pitchPixelsCur+1] - pRef[pitchPixelsRef+1];
    Ipp32s a0 = d0 + d2;
    Ipp32s a1 = d1 + d3;
    Ipp32s a2 = d0 - d2;
    Ipp32s a3 = d1 - d3;
    Ipp32u sad = ABS(a0 + a1) + ABS(a0 - a1) + ABS(a2 + a3) + ABS(a2 - a3);
    d0 = pCur[2] - pRef[2];
    d1 = pCur[3] - pRef[3];
    d2 = pCur[pitchPixelsCur+2] - pRef[pitchPixelsRef+2];
    d3 = pCur[pitchPixelsCur+3] - pRef[pitchPixelsRef+3];
    a0 = d0 + d2;
    a1 = d1 + d3;
    a2 = d0 - d2;
    a3 = d1 - d3;
    sad += ABS(a0 + a1) + ABS(a0 - a1) + ABS(a2 + a3) + ABS(a2 - a3);
    return sad;
}


Ipp32u SATD2x4_8u_C1R(const Ipp8u *pCur, Ipp32s pitchPixelsCur, const Ipp8u *pRef, Ipp32s pitchPixelsRef)
{
    Ipp32s d0 = pCur[0] - pRef[0];
    Ipp32s d1 = pCur[1] - pRef[1];
    Ipp32s d2 = pCur[pitchPixelsCur+0] - pRef[pitchPixelsRef+0];
    Ipp32s d3 = pCur[pitchPixelsCur+1] - pRef[pitchPixelsRef+1];
    Ipp32s a0 = d0 + d2;
    Ipp32s a1 = d1 + d3;
    Ipp32s a2 = d0 - d2;
    Ipp32s a3 = d1 - d3;
    Ipp32u sad = ABS(a0 + a1) + ABS(a0 - a1) + ABS(a2 + a3) + ABS(a2 - a3);
    pCur += pitchPixelsCur * 2;
    pRef += pitchPixelsRef * 2;
    d0 = pCur[0] - pRef[0];
    d1 = pCur[1] - pRef[1];
    d2 = pCur[pitchPixelsCur+0] - pRef[pitchPixelsRef+0];
    d3 = pCur[pitchPixelsCur+1] - pRef[pitchPixelsRef+1];
    a0 = d0 + d2;
    a1 = d1 + d3;
    a2 = d0 - d2;
    a3 = d1 - d3;
    sad += ABS(a0 + a1) + ABS(a0 - a1) + ABS(a2 + a3) + ABS(a2 - a3);
    return sad;
}


Ipp32u SATD2x2_8u_C1R(const Ipp8u *pCur, Ipp32s pitchPixelsCur, const Ipp8u *pRef, Ipp32s pitchPixelsRef)
{
    Ipp32s d0 = pCur[0] - pRef[0];
    Ipp32s d1 = pCur[1] - pRef[1];
    Ipp32s d2 = pCur[pitchPixelsCur+0] - pRef[pitchPixelsRef+0];
    Ipp32s d3 = pCur[pitchPixelsCur+1] - pRef[pitchPixelsRef+1];
    Ipp32s a0 = d0 + d2;
    Ipp32s a1 = d1 + d3;
    Ipp32s a2 = d0 - d2;
    Ipp32s a3 = d1 - d3;
    Ipp32u sad = ABS(a0 + a1) + ABS(a0 - a1) + ABS(a2 + a3) + ABS(a2 - a3);
    return sad;
}


SADfunc8u SAD_8u[21] = {NULL, NULL, (SADfunc8u)SAD2x2_8u32s_C1R, (SADfunc8u)SAD2x4_8u32s_C1R, (SADfunc8u)SAD4x2_8u32s_C1R, (SADfunc8u)ippiSAD4x4_8u32s, (SADfunc8u)SAD4x8_8u32s_C1R, NULL, NULL, (SADfunc8u)SAD8x4_8u32s_C1R, (SADfunc8u)ippiSAD8x8_8u32s_C1R, NULL, (SADfunc8u)SAD8x16_8u32s_C1R, NULL, NULL, NULL, NULL, NULL, (SADfunc8u)ippiSAD16x8_8u32s_C1R, NULL, (SADfunc8u)ippiSAD16x16_8u32s};
SATDfunc8u SATD_8u[21] = {NULL, NULL, SATD2x2_8u_C1R, SATD2x4_8u_C1R, SATD4x2_8u_C1R, SATD4x4_8u_C1R, SATD4x8_8u_C1R, NULL, NULL, SATD8x4_8u_C1R, SATD8x8_8u_C1R, NULL, SATD8x16_8u_C1R, NULL, NULL, NULL, NULL, NULL, SATD16x8_8u_C1R, NULL, SATD16x16_8u_C1R};

#endif // TABLE_FUNC


} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER



