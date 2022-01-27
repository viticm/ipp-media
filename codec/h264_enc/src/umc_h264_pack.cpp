//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include "umc_h264_video_encoder.h"
#include "umc_h264_core_enc.h"
#include "umc_h264_tables.h"
#include "vm_debug.h"

namespace UMC_H264_ENCODER
{

// Encoder CBP tables, created from decoder CBP tables

static const Ipp8u enc_cbp_intra[64] = {
     3, 29, 30, 17, 31, 18, 37,  8, 32, 38,
    19,  9, 20, 10, 11,  2, 16, 33, 34, 21,
    35, 22, 39, 04, 36, 40, 23,  5, 24,  6,
    07, 01, 41, 42, 43, 25, 44, 26, 46, 12,
    45, 47, 27, 13, 28, 14, 15, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00
};

static const Ipp8u enc_cbp_inter[64] = {
     0,  2,  3,  7,  4,  8, 17, 13,  5, 18,
     9, 14, 10, 15, 16, 11,  1, 32, 33, 36,
    34, 37, 44, 40, 35, 45, 38, 41, 39, 42,
    43, 19, 06, 24, 25, 20, 26, 21, 46, 28,
    27, 47, 22, 29, 23, 30, 31, 12, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00
};

static const Ipp8u enc_cbp_intra_monochrome[16] = {
     1, 10, 11, 6, 12, 7, 14, 2, 13, 15, 8, 3, 9, 4, 5, 0
};

static const Ipp8u enc_cbp_inter_monochrome[16] = {
    0, 1, 2, 5, 3, 6, 14, 10, 4, 15, 7, 11, 8, 12, 13, 9
};

#define BIT_SET(x,n)  ((Ipp32s)(((x)&(1<<(n)))>>(n)))

#define StoreDMVs(m_mv_start,min_x,max_x,step_x,min_y,max_y,step_y,mv)      \
if(core_enc->m_PicParamSet.entropy_coding_mode)                             \
{                                                                           \
    H264MotionVector   *tmv=m_mv_start;                                     \
    for (Ipp32s ii_=min_y;ii_<max_y;ii_+=step_y)                            \
        for (Ipp32s j_=min_x;j_<max_x;j_+=step_x)                           \
        {                                                                   \
            Ipp32s index=ii_*4 + j_;                                        \
            tmv[index] = mv;                                                \
        }                                                                   \
}

typedef struct{
    Ipp32s list_num;
    Ipp32s block_idx;
    Ipp32s block_w;
    Ipp32s block_h;
} BlocksInfo;

static BlocksInfo blocks_infos[] =
{
    {0, 0, 4, 4}, // 0 - MBTYPE_INTER

    {1, 0, 4, 4}, // 1 - MBTYPE_BACKWARD

    {0, 0, 4, 2}, // 2 - MBTYPE_FWD_FWD_16x8, MBTYPE_INTER_16x8
    {0, 8, 4, 2}, // 3 - MBTYPE_FWD_FWD_16x8, MBTYPE_INTER_16x8

    {0, 0, 2, 4}, // 4 - MBTYPE_FWD_FWD_8x16, MBTYPE_INTER_8x16
    {0, 2, 2, 4}, // 5 - MBTYPE_FWD_FWD_8x16, MBTYPE_INTER_8x16

    {1, 0, 4, 2}, // 6 - MBTYPE_BWD_BWD_16x8
    {1, 8, 4, 2}, // 7 - MBTYPE_BWD_BWD_16x8

    {1, 0, 2, 4}, // 8 - MBTYPE_BWD_BWD_8x16
    {1, 2, 2, 4}, // 9 - MBTYPE_BWD_BWD_8x16


    {0, 0, 4, 2}, // 10 - MBTYPE_FWD_BWD_16x8
    {1, 8, 4, 2}, // 11 - MBTYPE_FWD_BWD_16x8

    {0, 0, 2, 4}, // 12 - MBTYPE_FWD_BWD_8x16
    {1, 2, 2, 4}, // 13 - MBTYPE_FWD_BWD_8x16

    {0, 8, 4, 2}, // 14 - MBTYPE_BWD_FWD_16x8
    {1, 0, 4, 2}, // 15 - MBTYPE_BWD_FWD_16x8

    {0, 2, 2, 4}, // 16 - MBTYPE_BWD_FWD_8x16
    {1, 0, 2, 4}, // 17 - MBTYPE_BWD_FWD_8x16

    {0, 0, 4, 2}, // 18 - MBTYPE_BIDIR_FWD_16x8
    {0, 8, 4, 2}, // 19 - MBTYPE_BIDIR_FWD_16x8
    {1, 0, 4, 2}, // 20 - MBTYPE_BIDIR_FWD_16x8

    {0, 0, 4, 2}, // 21 - MBTYPE_FWD_BIDIR_16x8
    {0, 8, 4, 2}, // 22 - MBTYPE_FWD_BIDIR_16x8
    {1, 8, 4, 2}, // 23 - MBTYPE_FWD_BIDIR_16x8

    {0, 0, 4, 2}, // 24 - MBTYPE_BIDIR_BWD_16x8
    {1, 0, 4, 2}, // 25 - MBTYPE_BIDIR_BWD_16x8
    {1, 8, 4, 2}, // 26 - MBTYPE_BIDIR_BWD_16x8

    {0, 8, 4, 2}, // 27 - MBTYPE_BWD_BIDIR_16x8
    {1, 0, 4, 2}, // 28 - MBTYPE_BWD_BIDIR_16x8
    {1, 8, 4, 2}, // 29 - MBTYPE_BWD_BIDIR_16x8

    {0, 0, 4, 2}, // 30 - MBTYPE_BIDIR_BIDIR_16x8
    {0, 8, 4, 2}, // 31 - MBTYPE_BIDIR_BIDIR_16x8
    {1, 0, 4, 2}, // 32 - MBTYPE_BIDIR_BIDIR_16x8
    {1, 8, 4, 2}, // 33 - MBTYPE_BIDIR_BIDIR_16x8

    {0, 0, 2, 4}, // 34 - MBTYPE_BIDIR_FWD_8x16
    {0, 2, 2, 4}, // 35 - MBTYPE_BIDIR_FWD_8x16
    {1, 0, 2, 4}, // 36 - MBTYPE_BIDIR_FWD_8x16

    {0, 0, 2, 4}, // 37 - MBTYPE_FWD_BIDIR_8x16
    {0, 2, 2, 4}, // 38 - MBTYPE_FWD_BIDIR_8x16
    {1, 2, 2, 4}, // 39 - MBTYPE_FWD_BIDIR_8x16

    {0, 0, 2, 4}, // 40 - MBTYPE_BIDIR_BWD_8x16
    {1, 0, 2, 4}, // 41 - MBTYPE_BIDIR_BWD_8x16
    {1, 2, 2, 4}, // 42 - MBTYPE_BIDIR_BWD_8x16

    {0, 2, 2, 4}, // 43 - MBTYPE_BWD_BIDIR_8x16
    {1, 0, 2, 4}, // 44 - MBTYPE_BWD_BIDIR_8x16
    {1, 2, 2, 4}, // 45 - MBTYPE_BWD_BIDIR_8x16

    {0, 0, 2, 4}, // 46 - MBTYPE_BIDIR_BIDIR_8x16
    {0, 2, 2, 4}, // 47 - MBTYPE_BIDIR_BIDIR_8x16
    {1, 0, 2, 4}, // 48 - MBTYPE_BIDIR_BIDIR_8x16
    {1, 2, 2, 4}, // 49 - MBTYPE_BIDIR_BIDIR_8x16

    {0, 0, 4, 4}, // 50 - MBTYPE_BIDIR
    {1, 0, 4, 4}, // 51 - MBTYPE_BIDIR

    {0, 0, 2, 2}, // 52 - MBTYPE_INTER_8x8 / SBTYPE_8x8
    {0, 0, 2, 1}, // 53 - MBTYPE_INTER_8x8 / SBTYPE_8x4
    {0, 0, 1, 2}, // 54 - MBTYPE_INTER_8x8 / SBTYPE_4x8
    {0, 0, 1, 1}, // 55 - MBTYPE_INTER_8x8 / SBTYPE_4x4


    {0, 0, 2, 2}, // 56 - MBTYPE_B_8x8 / SBTYPE_FORWARD_8x8
    {0, 0, 2, 1}, // 57 - MBTYPE_B_8x8 / SBTYPE_FORWARD_8x4
    {0, 0, 1, 2}, // 58 - MBTYPE_B_8x8 / SBTYPE_FORWARD_4x8
    {0, 0, 1, 1}, // 59 - MBTYPE_B_8x8 / SBTYPE_FORWARD_4x4
    {1, 0, 2, 2}, // 60 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_8x8
    {1, 0, 2, 1}, // 61 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_8x4
    {1, 0, 1, 2}, // 62 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_4x8
    {1, 0, 1, 1}, // 63 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_4x4
};

typedef struct {
    BlocksInfo *mv_blocks;
    Ipp32s size_mv;
    Ipp32s mvs_l0_cnt;

    BlocksInfo *ref_blocks;
    Ipp32s size_ref;
    Ipp32s ref_l0_cnt;
} MV_Ref_Info;

void get_blocks_info(MBTypeValue mb_type, SBTypeValue *sub_type, MV_Ref_Info & mv_ref_info)
{
    const Ipp32s blockpos[4] = {0, 2, 8, 10};

    mv_ref_info.mvs_l0_cnt = 0;
    mv_ref_info.ref_l0_cnt = 0;

    switch (mb_type)
    {
    case MBTYPE_DIRECT:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 0;
        break;

    case MBTYPE_INTER:
    case MBTYPE_SKIPPED:
    case MBTYPE_FORWARD:
        mv_ref_info.size_mv = mv_ref_info.mvs_l0_cnt =
        mv_ref_info.size_ref = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = &blocks_infos[0];
        mv_ref_info.ref_blocks = &blocks_infos[0];
        break;

    case MBTYPE_BACKWARD:
        mv_ref_info.size_mv =
        mv_ref_info.size_ref = 1;
        mv_ref_info.mv_blocks = &blocks_infos[1];
        mv_ref_info.ref_blocks = &blocks_infos[1];
        break;

    case MBTYPE_FWD_FWD_16x8:
    case MBTYPE_INTER_16x8:
        mv_ref_info.size_mv = mv_ref_info.mvs_l0_cnt =
        mv_ref_info.size_ref = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = &blocks_infos[2];
        mv_ref_info.ref_blocks = &blocks_infos[2];
        break;

    case MBTYPE_FWD_FWD_8x16:
    case MBTYPE_INTER_8x16:
        mv_ref_info.size_mv = mv_ref_info.mvs_l0_cnt =
        mv_ref_info.size_ref = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[4];
        break;

    case MBTYPE_BWD_BWD_16x8:
        mv_ref_info.size_mv =
        mv_ref_info.size_ref = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[6];
        break;

    case MBTYPE_BWD_BWD_8x16:
        mv_ref_info.size_mv =
        mv_ref_info.size_ref = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[8];
        break;

    case MBTYPE_FWD_BWD_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 2;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[10];
        break;

    case MBTYPE_FWD_BWD_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 2;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[12];
        break;

    case MBTYPE_BWD_FWD_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 2;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[14];
        break;

    case MBTYPE_BWD_FWD_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 2;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[16];
        break;

    case MBTYPE_BIDIR_FWD_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[18];
        break;

    case MBTYPE_FWD_BIDIR_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[21];
        break;

    case MBTYPE_BIDIR_BWD_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[24];
        break;

    case MBTYPE_BWD_BIDIR_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[27];
        break;

    case MBTYPE_BIDIR_BIDIR_16x8:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 4;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[30];
        break;

    case MBTYPE_BIDIR_FWD_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[34];
        break;

    case MBTYPE_FWD_BIDIR_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[37];
        break;

    case MBTYPE_BIDIR_BWD_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[40];
        break;

    case MBTYPE_BWD_BIDIR_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 3;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[43];
        break;

    case MBTYPE_BIDIR_BIDIR_8x16:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 4;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 2;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[46];
        break;

    case MBTYPE_BIDIR:
        mv_ref_info.size_mv = mv_ref_info.size_ref = 2;
        mv_ref_info.mvs_l0_cnt = mv_ref_info.ref_l0_cnt = 1;
        mv_ref_info.mv_blocks = mv_ref_info.ref_blocks = &blocks_infos[50];
        break;

    case MBTYPE_INTER_8x8_REF0:
    case MBTYPE_INTER_8x8:
        {
        BlocksInfo *mv_inf = mv_ref_info.mv_blocks;
        BlocksInfo *ref_inf = mv_ref_info.ref_blocks;
        mv_ref_info.size_mv = mv_ref_info.size_ref = 0;
        for (Ipp32s block = 0; block < 4; block++)
        {
            switch (*sub_type)
            {
                case SBTYPE_8x8:
                    mv_ref_info.mvs_l0_cnt++;
                    mv_ref_info.ref_l0_cnt++;
                    *mv_inf = blocks_infos[52];
                    mv_inf->block_idx = blockpos[block];
                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_mv++;
                    mv_ref_info.size_ref++;
                    break;

                case SBTYPE_8x4:
                    *mv_inf = blocks_infos[53];
                    mv_inf->block_idx = blockpos[block];
                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.ref_l0_cnt++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[53];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;
                    mv_ref_info.mvs_l0_cnt += 2;
                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_4x8:
                    *mv_inf = blocks_infos[54];
                    mv_inf->block_idx = blockpos[block];
                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.ref_l0_cnt++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[54];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;
                    mv_ref_info.mvs_l0_cnt += 2;
                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_4x4:
                    *mv_inf = blocks_infos[55];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.ref_l0_cnt++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[55];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;
                    *mv_inf = blocks_infos[55];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;
                    *mv_inf = blocks_infos[55];
                    mv_inf->block_idx = blockpos[block] + 5;
                    mv_inf++;

                    mv_ref_info.mvs_l0_cnt += 4;
                    mv_ref_info.size_mv += 4;
                    break;

                default:
                    VM_ASSERT(false);
                    break;
            }

            sub_type++;
        };

        if (mb_type == MBTYPE_INTER_8x8_REF0)
        {
            mv_ref_info.ref_l0_cnt = mv_ref_info.size_ref = 0;
        }
        }
        break;

    case MBTYPE_B_8x8:
        {
        BlocksInfo *mv_inf = mv_ref_info.mv_blocks;
        BlocksInfo *ref_inf = mv_ref_info.ref_blocks;
        mv_ref_info.size_mv = mv_ref_info.size_ref = 0;
        SBTypeValue *stype = sub_type;
        Ipp32s block;
        for (block = 0; block < 4; block++)
        {
            switch (*stype)
            {
                case SBTYPE_DIRECT:
                    // nothing to do
                    break;

                case SBTYPE_FORWARD_8x8:
                case SBTYPE_BIDIR_8x8:
                    mv_ref_info.mvs_l0_cnt++;
                    mv_ref_info.ref_l0_cnt++;
                    *mv_inf = blocks_infos[56];
                    mv_inf->block_idx = blockpos[block];
                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_mv++;
                    mv_ref_info.size_ref++;
                    break;

                case SBTYPE_FORWARD_8x4:
                case SBTYPE_BIDIR_8x4:
                    *mv_inf = blocks_infos[57];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    ref_inf++;
                    mv_inf++;
                    mv_ref_info.ref_l0_cnt++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[57];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;
                    mv_ref_info.mvs_l0_cnt += 2;
                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_FORWARD_4x8:
                case SBTYPE_BIDIR_4x8:
                    *mv_inf = blocks_infos[58];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.ref_l0_cnt++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[58];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;
                    mv_ref_info.mvs_l0_cnt += 2;
                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_FORWARD_4x4:
                case SBTYPE_BIDIR_4x4:
                    *mv_inf = blocks_infos[59];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_ref++;
                    mv_ref_info.ref_l0_cnt++;

                    *mv_inf = blocks_infos[59];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;
                    *mv_inf = blocks_infos[59];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;
                    *mv_inf = blocks_infos[59];
                    mv_inf->block_idx = blockpos[block] + 5;
                    mv_inf++;

                    mv_ref_info.mvs_l0_cnt += 4;
                    mv_ref_info.size_mv += 4;
                    break;

                default:
                    break;
            }

            stype++;
        }

        stype = sub_type;
        for (block = 0; block < 4; block++)
        {
            switch (*stype)
            {
                case SBTYPE_DIRECT:
                    // nothing to do
                    break;

                case SBTYPE_BIDIR_8x8:
                case SBTYPE_BACKWARD_8x8:
                    *mv_inf = blocks_infos[60];
                    mv_inf->block_idx = blockpos[block];
                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_mv++;
                    mv_ref_info.size_ref++;
                    break;

                case SBTYPE_BACKWARD_8x4:
                case SBTYPE_BIDIR_8x4:
                    *mv_inf = blocks_infos[61];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[61];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;

                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_BACKWARD_4x8:
                case SBTYPE_BIDIR_4x8:
                    *mv_inf = blocks_infos[62];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[62];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;

                    mv_ref_info.size_mv += 2;
                    break;

                case SBTYPE_BACKWARD_4x4:
                case SBTYPE_BIDIR_4x4:
                    *mv_inf = blocks_infos[63];
                    mv_inf->block_idx = blockpos[block];

                    *ref_inf = *mv_inf;
                    mv_inf++;
                    ref_inf++;
                    mv_ref_info.size_ref++;

                    *mv_inf = blocks_infos[63];
                    mv_inf->block_idx = blockpos[block] + 1;
                    mv_inf++;
                    *mv_inf = blocks_infos[63];
                    mv_inf->block_idx = blockpos[block] + 4;
                    mv_inf++;
                    *mv_inf = blocks_infos[63];
                    mv_inf->block_idx = blockpos[block] + 5;
                    mv_inf++;

                    mv_ref_info.size_mv += 4;
                    break;

                default:
                    break;
            }

            stype++;
        }

        }
        break;


    default:
        VM_ASSERT(false);
        mv_ref_info.size_mv =
        mv_ref_info.size_ref = 0;
        break;
    }
}

// table used to obtain block mask bit from subblock index
static const Ipp8u subblock2block[24] =
{
    0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02,
    0x04, 0x04, 0x08, 0x08, 0x04, 0x04, 0x08, 0x08,
    0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20
};

////////////////////////////////////////////////////////////////////////////////
//
//
//
static Ipp32s sb_x[24] = {0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3, 0,1,0,1,0,1,0,1};
static Ipp32s sb_y[24] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3, 0,0,1,1,0,0,1,1};
static Ipp32s chroma_left[3][32] = {
    {-17,16,-19,18,-21,20,-23,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {-17,16,-19,18,-21,20,-23,22,-25,24,-27,26,-29,28,-31,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {-21,16,-23,18,17,20,19,22,-29,24,-31,26,25,28,27,30, -37,32,-39,34,33,36,35,38, -45,40,-47,42,41,44,43,46}
};
static Ipp32s chroma_top[3][32] = {
    {-18,-19,16,17,-22,-23,20,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {-22,-23,16,17, 18, 19,20,21,-30,-31,24,25,26,27,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {-26,-27,16,17,-30,-31,20,21, 18, 19,24,25,22,23,28,29,-42,-43,32,33,-46,-47,36,37,34,35,40,41,38,39,44,45}
};

#define PIXBITS 8
#include "umc_h264_pack_tmpl.cpp.h"
#define FAKE_BITSTREAM
#include "umc_h264_pack_tmpl.cpp.h"
#undef FAKE_BITSTREAM
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_pack_tmpl.cpp.h"
#define FAKE_BITSTREAM
#include "umc_h264_pack_tmpl.cpp.h"
#undef FAKE_BITSTREAM
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER



