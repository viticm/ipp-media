//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef UMC_H264_CORE_ENC_H__
#define UMC_H264_CORE_ENC_H__

#include "umc_memory_allocator.h"
#include "umc_h264_defs.h"
#include "umc_h264_bs.h"
#include "umc_h264_enc_cpb.h"

#ifdef H264_COMMON_ME
//================== common ME ===============================================
#include "umc_me.h"
#include "umc_vme.h"
//============================================================================
#endif

//f
#include "umc_h264_avbr.h"

#define VM_DEBUG 7
#include "vm_debug.h"

#ifdef H264_STAT
#include <vector>
#endif

#ifdef FRAME_QP_FROM_FILE
#include <list>
#endif

// These are used to index into Block_RLE below.
#define U_DC_RLE 48     // Used in Intra Prediciton Modes
#define V_DC_RLE 49     // Used in Intra Prediciton Modes
#define Y_DC_RLE 50     // Used in Intra 16x16 Prediciton Mode

#define ANALYSE_I_4x4               (1 << 0)
#define ANALYSE_I_8x8               (1 << 1)
#define ANALYSE_P_4x4               (1 << 2)
#define ANALYSE_P_8x8               (1 << 3)
#define ANALYSE_B_4x4               (1 << 4)
#define ANALYSE_B_8x8               (1 << 5)
#define ANALYSE_SAD                 (1 << 6)
#define ANALYSE_ME_EARLY_EXIT       (1 << 7)
#define ANALYSE_ME_ALL_REF          (1 << 8)
#define ANALYSE_ME_CHROMA           (1 << 9)
#define ANALYSE_ME_SUBPEL           (1 << 10)
#define ANALYSE_CBP_EMPTY           (1 << 11)
#define ANALYSE_RECODE_FRAME        (1 << 12)
#define ANALYSE_ME_AUTO_DIRECT      (1 << 13)
#define ANALYSE_FRAME_TYPE          (1 << 14)
#define ANALYSE_FLATNESS            (1 << 15)
#define ANALYSE_RD_MODE             (1 << 16)
#define ANALYSE_RD_OPT              (1 << 17)
#define ANALYSE_B_RD_OPT            (1 << 18)
#define ANALYSE_CHECK_SKIP_PREDICT  (1 << 19)
#define ANALYSE_CHECK_SKIP_INTPEL   (1 << 20)
#define ANALYSE_CHECK_SKIP_BESTCAND (1 << 21)
#define ANALYSE_CHECK_SKIP_SUBPEL   (1 << 22)
#define ANALYSE_SPLIT_SMALL_RANGE   (1 << 23)
#define ANALYSE_ME_EXT_CANDIDATES   (1 << 24)
#define ANALYSE_ME_SUBPEL_SAD       (1 << 25)
#define ANALYSE_INTRA_IN_ME         (1 << 26)
#define ANALYSE_ME_FAST_MULTIREF    (1 << 27)
#define ANALYSE_FAST_INTRA          (1 << 28)
#define ANALYSE_ME_PRESEARCH        (1 << 29)
#define ANALYSE_ME_CONTINUED_SEARCH (1 << 30)

//Optimal quantization levels
#define OPT_QUANT_INTER_RD 2
#define OPT_QUANT_INTER 0
#define OPT_QUANT_INTRA16x16 0
#define OPT_QUANT_INTRA4x4 1
#define OPT_QUANT_INTRA8x8 1


#define CALC_16x16_INTRA_MB_TYPE(slice, mode, nc, ac) (1+IntraMBTypeOffset[slice]+mode+4*nc+12*ac)
#define CALC_4x4_INTRA_MB_TYPE(slice) (IntraMBTypeOffset[slice])
#define CALC_PCM_MB_TYPE(slice) (IntraMBTypeOffset[slice]+25);


namespace UMC_H264_ENCODER
{

template <class PixType> class CNoiseReductionFilter;

#ifdef H264_STAT
    class H264FramesStat{
        public:
            int numIf, sizeI;
            int numPf, sizeP;
            int numBf, sizeB;
            int numI[32][2];
            int numP[32][2];
            int numB[32][2];
            int numIsize[32][2];
            int numPsize[32][2];
            int numBsize[32][2];
            int numImb, numPmb, numBmb;
            int numPnsb;
            int numPsb[4];
            int numRefsP[32];
            int numRefsB[2][32];
            std::vector<int> slice_sizes;
            std::vector<int> frame_sizes;
            H264FramesStat();
            void addMB(Ipp32s sliceType, Ipp32s mbType, Ipp8u sbType[4], bool t8x8, Ipp32s mbBits);
            void addMBRefs(Ipp32s sliceType, Ipp32s mbType, H264MacroblockRefIdxs* refs[2]);
            void addFrame( Ipp32s frameType, Ipp32s size );
            void printStat();
    };
#endif

    /* Common for 8u and 16u */

    inline Ipp8s GetReferenceField(
        Ipp8s *pFields,
        Ipp32s RefIndex)
    {
        if (RefIndex < 0)
            return -1;
        else {
            VM_ASSERT(pFields[RefIndex] >= 0);
            return pFields[RefIndex];
        }
    }

    // forward declaration of internal types
    typedef void (*H264CoreEncoder_DeblockingFunction)(void* state, Ipp32u nMBAddr);

    /* End of: Common for 8u and 16u */

#define PIXBITS 8
#include "umc_h264_core_enc_tmpl.h"
#undef PIXBITS

#if defined (BITDEPTH_9_12)

#define PIXBITS 16
#include "umc_h264_core_enc_tmpl.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

} // namespace UMC_H264_ENCODER

#endif // UMC_H264_CORE_ENC_H__

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
