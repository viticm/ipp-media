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
#include "umc_h264_bme.h"

// pitch of the prediction buffer which is the destination buffer for the
// functions in this file

#define DEST_PITCH 16

namespace UMC_H264_ENCODER
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
typedef struct{
    Ipp32s block_index;
    Ipp32s block_w;
    Ipp32s block_h;
    Direction_t direction;
} BlocksBidirInfo;

BlocksBidirInfo bidir_blocks_infos[] =
{
    {0, 4, 4, D_DIR_FWD}, // 0 - MBTYPE_INTER

    {0, 4, 4, D_DIR_BWD}, // 1 - MBTYPE_BACKWARD

    {0, 4, 2, D_DIR_FWD}, // 2 - MBTYPE_FWD_FWD_16x8, MBTYPE_INTER_16x8
    {8, 4, 2, D_DIR_FWD}, // 3 - MBTYPE_FWD_FWD_16x8, MBTYPE_INTER_16x8

    {0, 2, 4, D_DIR_FWD}, // 4 - MBTYPE_FWD_FWD_8x16, MBTYPE_INTER_8x16
    {2, 2, 4, D_DIR_FWD}, // 5 - MBTYPE_FWD_FWD_8x16, MBTYPE_INTER_8x16

    {0, 4, 2, D_DIR_BWD}, // 6 - MBTYPE_BWD_BWD_16x8
    {8, 4, 2, D_DIR_BWD}, // 7 - MBTYPE_BWD_BWD_16x8

    {0, 2, 4, D_DIR_BWD}, // 8 - MBTYPE_BWD_BWD_8x16
    {2, 2, 4, D_DIR_BWD}, // 9 - MBTYPE_BWD_BWD_8x16

    {0, 4, 2, D_DIR_FWD}, // 10 - MBTYPE_FWD_BWD_16x8
    {8, 4, 2, D_DIR_BWD}, // 11 - MBTYPE_FWD_BWD_16x8

    {0, 2, 4, D_DIR_FWD}, // 12 - MBTYPE_FWD_BWD_8x16
    {2, 2, 4, D_DIR_BWD}, // 13 - MBTYPE_FWD_BWD_8x16

    {8, 4, 2, D_DIR_FWD}, // 14 - MBTYPE_BWD_FWD_16x8
    {0, 4, 2, D_DIR_BWD}, // 15 - MBTYPE_BWD_FWD_16x8

    {2, 2, 4, D_DIR_FWD}, // 16 - MBTYPE_BWD_FWD_8x16
    {0, 2, 4, D_DIR_BWD}, // 17 - MBTYPE_BWD_FWD_8x16

    {0, 4, 2, D_DIR_BIDIR}, // 18 - MBTYPE_BIDIR_FWD_16x8
    {8, 4, 2, D_DIR_FWD},   // 19 - MBTYPE_BIDIR_FWD_16x8

    {0, 4, 2, D_DIR_FWD},   // 20 - MBTYPE_FWD_BIDIR_16x8
    {8, 4, 2, D_DIR_BIDIR}, // 21 - MBTYPE_FWD_BIDIR_16x8

    {0, 4, 2, D_DIR_BIDIR}, // 22 - MBTYPE_BIDIR_BWD_16x8
    {8, 4, 2, D_DIR_BWD},   // 23 - MBTYPE_BIDIR_BWD_16x8

    {0, 4, 2, D_DIR_BWD},   // 24 - MBTYPE_BWD_BIDIR_16x8
    {8, 4, 2, D_DIR_BIDIR}, // 25 - MBTYPE_BWD_BIDIR_16x8

    {0, 4, 2, D_DIR_BIDIR}, // 26 - MBTYPE_BIDIR_BIDIR_16x8
    {8, 4, 2, D_DIR_BIDIR}, // 27 - MBTYPE_BIDIR_BIDIR_16x8

    {0, 2, 4, D_DIR_BIDIR}, // 28 - MBTYPE_BIDIR_FWD_8x16
    {2, 2, 4, D_DIR_FWD},   // 29 - MBTYPE_BIDIR_FWD_8x16

    {0, 2, 4, D_DIR_FWD},   // 30 - MBTYPE_FWD_BIDIR_8x16
    {2, 2, 4, D_DIR_BIDIR}, // 31 - MBTYPE_FWD_BIDIR_8x16

    {0, 2, 4, D_DIR_BIDIR}, // 32 - MBTYPE_BIDIR_BWD_8x16
    {2, 2, 4, D_DIR_BWD},   // 33 - MBTYPE_BIDIR_BWD_8x16

    {0, 2, 4, D_DIR_BWD},   // 34 - MBTYPE_BWD_BIDIR_8x16
    {2, 2, 4, D_DIR_BIDIR}, // 35 - MBTYPE_BWD_BIDIR_8x16

    {0, 2, 4, D_DIR_BIDIR}, // 36 - MBTYPE_BIDIR_BIDIR_8x16
    {2, 2, 4, D_DIR_BIDIR}, // 37 - MBTYPE_BIDIR_BIDIR_8x16

    {0, 4, 4, D_DIR_BIDIR}, // 38 - MBTYPE_BIDIR

    {0, 2, 2, D_DIR_FWD}, // 39 - MBTYPE_B_8x8 / SBTYPE_FORWARD_8x8
    {0, 2, 1, D_DIR_FWD}, // 40 - MBTYPE_B_8x8 / SBTYPE_FORWARD_8x4
    {0, 1, 2, D_DIR_FWD}, // 41 - MBTYPE_B_8x8 / SBTYPE_FORWARD_4x8
    {0, 1, 1, D_DIR_FWD}, // 42 - MBTYPE_B_8x8 / SBTYPE_FORWARD_4x4

    {0, 2, 2, D_DIR_BWD}, // 43 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_8x8
    {0, 2, 1, D_DIR_BWD}, // 44 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_8x4
    {0, 1, 2, D_DIR_BWD}, // 45 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_4x8
    {0, 1, 1, D_DIR_BWD}, // 46 - MBTYPE_B_8x8 / SBTYPE_BACKWARD_4x4

    {0, 2, 2, D_DIR_BIDIR}, // 47 - MBTYPE_B_8x8 / SBTYPE_BIDIR_8x8
    {0, 2, 1, D_DIR_BIDIR}, // 48 - MBTYPE_B_8x8 / SBTYPE_BIDIR_8x4
    {0, 1, 2, D_DIR_BIDIR}, // 49 - MBTYPE_B_8x8 / SBTYPE_BIDIR_4x8
    {0, 1, 1, D_DIR_BIDIR}, // 50 - MBTYPE_B_8x8 / SBTYPE_BIDIR_4x4

    {-1, 4, 4, D_DIR_DIRECT}, // 51 - MBTYPE_DIRECT
    {0, 2, 2, D_DIR_DIRECT},  // 52 - MBTYPE_DIRECT_8x8
};

typedef struct{
    Ipp32s size;
    BlocksBidirInfo *blocks;
} Bidir_Blocks_Info;

void get_bidir_info(MBTypeValue mb_type, SBTypeValue *sub_type, Bidir_Blocks_Info & bidir_info)
{
    const Ipp32s blockpos[4] = {0, 2, 8, 10};

    switch (mb_type)
    {
    case MBTYPE_DIRECT:
        bidir_info.blocks = &bidir_blocks_infos[51];
        bidir_info.size = 1;
        break;

    case MBTYPE_INTER:
    case MBTYPE_FORWARD:
    case MBTYPE_SKIPPED:  //only for P frames
        bidir_info.blocks = &bidir_blocks_infos[0];
        bidir_info.size = 1;
        break;

    case MBTYPE_BACKWARD:
        bidir_info.blocks = &bidir_blocks_infos[1];
        bidir_info.size = 1;
        break;

    case MBTYPE_FWD_FWD_16x8:
    case MBTYPE_INTER_16x8:
        bidir_info.blocks = &bidir_blocks_infos[2];
        bidir_info.size = 2;
        break;

    case MBTYPE_FWD_FWD_8x16:
    case MBTYPE_INTER_8x16:
        bidir_info.blocks = &bidir_blocks_infos[4];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_BWD_16x8:
        bidir_info.blocks = &bidir_blocks_infos[6];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_BWD_8x16:
        bidir_info.blocks = &bidir_blocks_infos[8];
        bidir_info.size = 2;
        break;

    case MBTYPE_FWD_BWD_16x8:
        bidir_info.blocks = &bidir_blocks_infos[10];
        bidir_info.size = 2;
        break;

    case MBTYPE_FWD_BWD_8x16:
        bidir_info.blocks = &bidir_blocks_infos[12];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_FWD_16x8:
        bidir_info.blocks = &bidir_blocks_infos[14];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_FWD_8x16:
        bidir_info.blocks = &bidir_blocks_infos[16];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_FWD_16x8:
        bidir_info.blocks = &bidir_blocks_infos[18];
        bidir_info.size = 2;
        break;

    case MBTYPE_FWD_BIDIR_16x8:
        bidir_info.blocks = &bidir_blocks_infos[20];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_BWD_16x8:
        bidir_info.blocks = &bidir_blocks_infos[22];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_BIDIR_16x8:
        bidir_info.blocks = &bidir_blocks_infos[24];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_BIDIR_16x8:
        bidir_info.blocks = &bidir_blocks_infos[26];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_FWD_8x16:
        bidir_info.blocks = &bidir_blocks_infos[28];
        bidir_info.size = 2;
        break;

    case MBTYPE_FWD_BIDIR_8x16:
        bidir_info.blocks = &bidir_blocks_infos[30];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_BWD_8x16:
        bidir_info.blocks = &bidir_blocks_infos[32];
        bidir_info.size = 2;
        break;

    case MBTYPE_BWD_BIDIR_8x16:
        bidir_info.blocks = &bidir_blocks_infos[34];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR_BIDIR_8x16:
        bidir_info.blocks = &bidir_blocks_infos[36];
        bidir_info.size = 2;
        break;

    case MBTYPE_BIDIR:
        bidir_info.blocks = &bidir_blocks_infos[38];
        bidir_info.size = 1;
        break;

    case MBTYPE_INTER_8x8_REF0:
    case MBTYPE_INTER_8x8:
        {
        BlocksBidirInfo *inf = bidir_info.blocks;
        bidir_info.size = 0;
        for (Ipp32s block = 0; block < 4; block++)
        {
            switch (*sub_type)
            {
                case SBTYPE_8x8:
                    *inf = bidir_blocks_infos[39];
                    inf->block_index = blockpos[block];
                    inf++;
                    bidir_info.size++;
                    break;

                case SBTYPE_8x4:
                    *inf = bidir_blocks_infos[40];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[40];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_4x8:
                    *inf = bidir_blocks_infos[41];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[41];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_4x4:
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 5;
                    inf++;
                    bidir_info.size += 4;
                    break;

                default:
                    VM_ASSERT(false);
                    break;
            }

            sub_type++;
        };
        }
        break;

    case MBTYPE_B_8x8:
        {
        BlocksBidirInfo *inf = bidir_info.blocks;
        bidir_info.size = 0;

        SBTypeValue *stype = sub_type;
        for (Ipp32s block = 0; block < 4; block++)
        {
            switch (*stype)
            {
                case SBTYPE_DIRECT:
                    *inf = bidir_blocks_infos[52];
                    // inf->block_index = blockpos[block];
                    inf->block_index = block; // Special case !
                    inf++;
                    bidir_info.size++;
                    break;

                case SBTYPE_FORWARD_8x8:
                    *inf = bidir_blocks_infos[39];
                    inf->block_index = blockpos[block];
                    inf++;
                    bidir_info.size++;
                    break;

                case SBTYPE_BIDIR_8x8:
                    *inf = bidir_blocks_infos[47];
                    inf->block_index = blockpos[block];
                    inf++;
                    bidir_info.size++;
                    break;

                case SBTYPE_BACKWARD_8x8:
                    *inf = bidir_blocks_infos[43];
                    inf->block_index = blockpos[block];
                    inf++;
                    bidir_info.size++;
                    break;

                case SBTYPE_FORWARD_8x4:
                    *inf = bidir_blocks_infos[40];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[40];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_BACKWARD_8x4:
                    *inf = bidir_blocks_infos[44];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[44];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_BIDIR_8x4:
                    *inf = bidir_blocks_infos[48];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[48];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_FORWARD_4x8:
                    *inf = bidir_blocks_infos[41];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[41];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_BACKWARD_4x8:
                    *inf = bidir_blocks_infos[45];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[45];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_BIDIR_4x8:
                    *inf = bidir_blocks_infos[49];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[49];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    bidir_info.size += 2;
                    break;

                case SBTYPE_FORWARD_4x4:
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    *inf = bidir_blocks_infos[42];
                    inf->block_index = blockpos[block] + 5;
                    inf++;
                    bidir_info.size += 4;
                    break;

                case SBTYPE_BIDIR_4x4:
                    *inf = bidir_blocks_infos[50];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[50];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    *inf = bidir_blocks_infos[50];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    *inf = bidir_blocks_infos[50];
                    inf->block_index = blockpos[block] + 5;
                    inf++;
                    bidir_info.size += 4;
                    break;

                case SBTYPE_BACKWARD_4x4:
                    *inf = bidir_blocks_infos[46];
                    inf->block_index = blockpos[block];
                    inf++;
                    *inf = bidir_blocks_infos[46];
                    inf->block_index = blockpos[block] + 1;
                    inf++;
                    *inf = bidir_blocks_infos[46];
                    inf->block_index = blockpos[block] + 4;
                    inf++;
                    *inf = bidir_blocks_infos[46];
                    inf->block_index = blockpos[block] + 5;
                    inf++;
                    bidir_info.size += 4;
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
        bidir_info.size = 0;
        break;
    }
}

#define TRUNCATE_LO(val, lim)      \
    { Ipp32s (tmp) = (lim);                      \
    if ((tmp) < (val))                  \
        (val) = (Ipp16s) (tmp);}

#define TRUNCATE_HI(val, lim)      \
    {Ipp32s (tmp) = (lim);                      \
    if ((tmp) > (val))                  \
        (val) = (Ipp16s) (tmp);}

#define PIXBITS 8
#include "umc_h264_mc_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_mc_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER



