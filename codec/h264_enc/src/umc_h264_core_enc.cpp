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
#include <stddef.h>
#include <math.h>
#include "ippdefs.h"
#include "umc_h264_video_encoder.h"
#include "umc_h264_core_enc.h"
#include "umc_h264_tables.h"
#include "umc_h264_to_ipp.h"
#include "umc_h264_bme.h"
#include "umc_h264_defs.h"
#include "umc_h264_wrappers.h"

// Table to obtain edge info for a 4x4 block of a MB. The table entry when
// OR'd with the edge info for the MB, results in edge info for the block.
//
//  H264 4x4 Block ordering in a 16x16 Macroblock and edge assignments
//
//  ULC = Upper Left Corner, U = Upper Edge
//  L = Left Edge, R = Right Edge
//
//               luma (Y)                chroma (U)          chroma (V)
//
//        +-U--+-U--+-U--+-U--+         +-U--+-U--+         +-U--+-U--+
//        |    |    |    |    |         |    |    |         |    |    |
// ULC--> L 0  | 1  | 4  | 5  R  ULC--> L 16 | 17 R  ULC--> L 20 | 21 R
//        |    |    |    |    |         |    |    |         |    |    |
//        +----+----+----+----+         +----+----+         +----+----+
//        |    |    |    |    |         |    |    |         |    |    |
//        L 2  | 3  | 6  | 7  R         L 18 | 19 R         L 22 | 23 R
//        |    |    |    |    |         |    |    |         |    |    |
//        +----+----+----+----+         +----+----+         +----+----+
//        |    |    |    |    |
//        L 8  | 9  | 12 | 13 R
//        |    |    |    |    |
//        +----+----+----+----+
//        |    |    |    |    |
//        L 10 | 11 | 14 | 15 R
//        |    |    |    |    |
//        +----+----+----+----+
//
//  This table provides easy look-up by block number to determine
//  which edges is does NOT border on.


namespace UMC_H264_ENCODER
{

const H264MotionVector null_mv = {0};

inline Ipp32s convertLumaQP(Ipp32s QP, Ipp32s bit_depth_src, Ipp32s bit_depth_dst)
{
    Ipp32s qpy = QP + 6*(bit_depth_src - 8);
    Ipp32s qpDstOffset = 6*(bit_depth_dst - 8);
    Ipp32s minQpDst = -qpDstOffset;
    Ipp32s maxQpDst = 51;
    qpy = qpy - qpDstOffset;
    qpy = (qpy < minQpDst)? minQpDst : (qpy > maxQpDst)? maxQpDst : qpy;
    return(qpy);
}

#define PIXBITS 8
#include "umc_h264_core_enc_tmpl.cpp.h"
#undef PIXBITS

#if defined BITDEPTH_9_12

#define PIXBITS 16
#include "umc_h264_core_enc_tmpl.cpp.h"
#undef PIXBITS

#endif // BITDEPTH_9_12

#ifdef H264_STAT
    H264FramesStat::H264FramesStat(){
        numIf = sizeI = numPf = sizeP = numBf = sizeB = 0;
        numImb=numPmb=numBmb=0;
        numPnsb = 0;
        for( int i = 0; i<32; i++ ){
            numI[i][0] = numI[i][1] = 0;
            numP[i][0] = numP[i][1] = 0;
            numB[i][0] = numB[i][1] = 0;
            numIsize[i][0] = numIsize[i][1] = 0;
            numPsize[i][0] = numPsize[i][1] = 0;
            numBsize[i][0] = numBsize[i][1] = 0;
            numRefsP[i]=0;
            numRefsB[0][i]=0;
            numRefsB[1][i]=0;
        }
        for( int i = 0; i<4; i++ ){
            numPsb[i] = 0;
        }
    }

    void H264FramesStat::addMB(Ipp32s sliceType, Ipp32s mbType, Ipp8u sbType[4], bool t8x8, Ipp32s mbBits){
        switch( sliceType ){
                    case INTRASLICE:
                        numI[mbType][t8x8]++;
                        numIsize[mbType][t8x8]+=mbBits;
                        numImb++;
                        break;
                    case PREDSLICE:
                        numPmb++;
                        numP[mbType][t8x8]++;
                        numPsize[mbType][t8x8]+=mbBits;
                        if( mbType == MBTYPE_INTER_8x8 || mbType == MBTYPE_INTER_8x8_REF0 ){
                            numPnsb += 4;
                            for( Ipp32s i = 0; i < 4; i++ ){
                                numPsb[sbType[i]]++;
                            }
                        }
                        break;
                    case BPREDSLICE:
                        numBmb++;
                        numB[mbType][t8x8]++;
                        numBsize[mbType][t8x8]+=mbBits;
                        break;
        }
    }

    void H264FramesStat::addMBRefs( Ipp32s sliceType, Ipp32s mbType, H264MacroblockRefIdxs* refs[2] )
    {
        Ipp32s i;
        const int boff[4]={0,8,10,12};

        switch( sliceType ){
                    case PREDSLICE:
                        for(i=0;i<4; i++ ){
                            T_RefIdx ref = refs[0]->RefIdxs[boff[i]];
                            if( ref >= 0 ) numRefsP[ref]++;
                        }
                        break;
                    case BPREDSLICE:
                        for(i=0;i<4; i++ ){
                            T_RefIdx ref0 = refs[0]->RefIdxs[boff[i]];
                            T_RefIdx ref1 = refs[1]->RefIdxs[boff[i]];
                            if( ref0 >= 0 ) numRefsB[0][ref0]++;
                            if( ref1 >= 0 ) numRefsB[1][ref0]++;
                        }
                        break;
        }
    }

    void H264FramesStat::addFrame( Ipp32s frameType, Ipp32s size ){
        switch( frameType ){
                    case INTRAPIC:
                        numIf++;
                        sizeI += size;
                        break;
                    case PREDPIC:
                        numPf++;
                        sizeP += size;
                        break;
                    case BPREDPIC:
                        numBf++;
                        sizeB += size;
                        break;
        }
    }

    void H264FramesStat::printStat(){
        static const char *slice_stat = "slice_sizes.txt";
        FILE* fslice;

        //Average frame size
        printf("Number of frames: %d\n",numIf+numPf+numBf);
        if( numIf ){
            printf(" I: %d  Avg size: %d\n",numIf,sizeI/numIf);
        }else{
            printf(" I: none Avg size: 0\n");
        }
        if( numPf ){
            printf(" P: %d  Avg size: %d\n",numPf,sizeP/numPf);
        }else{
            printf(" P: none Avg size: 0\n");
        }
        if( numBf ){
            printf(" B: %d  Avg size: %d\n",numBf,sizeB/numBf);
        }else{
            printf(" B: none Avg size: 0\n");
        }

        if( numIf )
            printf(" mb I I16..4: %.2f%%(%d) %.2f%%(%d) %.2f%%(%d) \n",
            100.0*numI[MBTYPE_INTRA_16x16][0]/numImb,
            numIsize[MBTYPE_INTRA_16x16][0]/numImb,
            100.0*numI[MBTYPE_INTRA][1]/numImb,
            numIsize[MBTYPE_INTRA][1]/numImb,
            100.0*numI[MBTYPE_INTRA][0]/numImb,
            numIsize[MBTYPE_INTRA][0]/numImb);
        if( numPf )
            printf(" mb P I16..4: %.2f%%(%d) %.2f%%(%d) %.2f%%(%d) P16..4: %.2f%%(%d) %.2f%%(%d) %.2f%%(%d) | 88: %.2f%%   84: %.2f%%   48: %.2f%%   44: %.2f%% | skip: %.2f%%\n",
            100.0*numP[MBTYPE_INTRA_16x16][0]/numPmb,
            numPsize[MBTYPE_INTRA_16x16][0]/numPmb,
            100.0*numP[MBTYPE_INTRA][1]/numPmb,
            numPsize[MBTYPE_INTRA][1]/numPmb,
            100.0*numP[MBTYPE_INTRA][0]/numPmb,
            numPsize[MBTYPE_INTRA][0]/numPmb,
            100.0*(numP[MBTYPE_INTER][0]+numP[MBTYPE_INTER][1])/numPmb,
            (numPsize[MBTYPE_INTER][0]+numPsize[MBTYPE_INTER][1])/numPmb,
            100.0*(numP[MBTYPE_INTER_8x16][0]+numP[MBTYPE_INTER_8x16][1]+numP[MBTYPE_INTER_16x8][0]+numP[MBTYPE_INTER_16x8][1])/numPmb,
            (numPsize[MBTYPE_INTER_8x16][0]+numPsize[MBTYPE_INTER_8x16][1]+numPsize[MBTYPE_INTER_16x8][0]+numPsize[MBTYPE_INTER_16x8][1])/numPmb,
            100.0*(numP[MBTYPE_INTER_8x8][0]+numP[MBTYPE_INTER_8x8][1]+numP[MBTYPE_INTER_8x8_REF0][0]+numP[MBTYPE_INTER_8x8_REF0][1])/numPmb,
            (numPsize[MBTYPE_INTER_8x8][0]+numPsize[MBTYPE_INTER_8x8][1]+numPsize[MBTYPE_INTER_8x8_REF0][0]+numPsize[MBTYPE_INTER_8x8_REF0][1])/numPmb,
            100.0*numPsb[SBTYPE_8x8]/numPnsb,
            100.0*numPsb[SBTYPE_8x4]/numPnsb,
            100.0*numPsb[SBTYPE_4x8]/numPnsb,
            100.0*numPsb[SBTYPE_4x4]/numPnsb,
            100.0*(numP[MBTYPE_SKIPPED][0]+numP[MBTYPE_SKIPPED][1])/numPmb);
        if( numBf )
            printf(" mb B I16..4: %.2f%% %.2f%% %.2f%% P16..8: %.2f%% %.2f%% %.2f%% direct: %.2f%% skip: %.2f%%\n",
            100.0*numB[MBTYPE_INTRA_16x16][0]/numBmb,
            100.0*numB[MBTYPE_INTRA][1]/numBmb,
            100.0*numB[MBTYPE_INTRA][0]/numBmb,
            100.0*(numB[MBTYPE_FORWARD][0]+numB[MBTYPE_FORWARD][1]+numB[MBTYPE_BACKWARD][0]+numB[MBTYPE_BACKWARD][1]+numB[MBTYPE_BIDIR][0]+numB[MBTYPE_BIDIR][1])/numBmb,
            100.0*(numB[MBTYPE_FWD_FWD_16x8][0]+numB[MBTYPE_FWD_FWD_16x8][1]+
            numB[MBTYPE_FWD_FWD_8x16][0]+numB[MBTYPE_FWD_FWD_8x16][1]+
            numB[MBTYPE_BWD_BWD_16x8][0]+numB[MBTYPE_BWD_BWD_16x8][1]+
            numB[MBTYPE_BWD_BWD_8x16][0]+numB[MBTYPE_BWD_BWD_8x16][1]+
            numB[MBTYPE_FWD_BWD_16x8][0]+numB[MBTYPE_FWD_BWD_16x8][1]+
            numB[MBTYPE_FWD_BWD_8x16][0]+numB[MBTYPE_FWD_BWD_8x16][1]+
            numB[MBTYPE_BWD_FWD_16x8][0]+numB[MBTYPE_BWD_FWD_16x8][1]+
            numB[MBTYPE_BWD_FWD_8x16][0]+numB[MBTYPE_BWD_FWD_8x16][1]+
            numB[MBTYPE_BIDIR_FWD_16x8][0]+numB[MBTYPE_BIDIR_FWD_16x8][1]+
            numB[MBTYPE_BIDIR_FWD_8x16][0]+numB[MBTYPE_BIDIR_FWD_8x16][1]+
            numB[MBTYPE_BIDIR_BWD_16x8][0]+numB[MBTYPE_BIDIR_BWD_16x8][1]+
            numB[MBTYPE_BIDIR_BWD_8x16][0]+numB[MBTYPE_BIDIR_BWD_8x16][1]+
            numB[MBTYPE_FWD_BIDIR_16x8][0]+numB[MBTYPE_FWD_BIDIR_16x8][1]+
            numB[MBTYPE_FWD_BIDIR_8x16][0]+numB[MBTYPE_FWD_BIDIR_8x16][1]+
            numB[MBTYPE_BWD_BIDIR_16x8][0]+numB[MBTYPE_BWD_BIDIR_16x8][1]+
            numB[MBTYPE_BWD_BIDIR_8x16][0]+numB[MBTYPE_BWD_BIDIR_8x16][1]+
            numB[MBTYPE_BIDIR_BIDIR_16x8][0]+numB[MBTYPE_BIDIR_BIDIR_16x8][1]+
            numB[MBTYPE_BIDIR_BIDIR_8x16][0]+numB[MBTYPE_BIDIR_BIDIR_8x16][1]
        )/numBmb,
            100.0*(numB[MBTYPE_B_8x8][0]+numB[MBTYPE_B_8x8][1])/numBmb,
            100.0*(numB[MBTYPE_DIRECT][0]+numB[MBTYPE_DIRECT][1])/numBmb,
            100.0*(numB[MBTYPE_SKIPPED][0]+numB[MBTYPE_SKIPPED][1])/numBmb
            );

        if( (fslice = fopen(slice_stat,"w")) != NULL){
            std::vector<int>::iterator i = slice_sizes.begin();
            for(int m=0;i != slice_sizes.end(); i++,m++){
                fprintf(fslice,"%d %d\n",m,(*i));
            }
            fclose( fslice );
        }
        //Print refs
        int all = 0;
        int last = 0;
        if(numPf){
            for( int i = 0; i<32; i++ ) { all += numRefsP[i]; if( numRefsP[i] != 0 ) last = i; }
            printf("ref P: ");
            for( int i = 0; i<=last; i++ ) printf("%.1f%% ",(float)100.0*numRefsP[i]/all);
            printf("\n");
        }
        all = 0;
        last = 0;
        if(numBf){
            for( int i = 0; i<32; i++ ) { all += numRefsB[0][i]; if( numRefsB[0][i] != 0 ) last = i; }
            printf("ref B L0: ");
            for( int i = 0; i<=last; i++ ) printf("%.1f%% ",(float)100.0*numRefsB[0][i]/all);
            printf("\nref B L1: ");
            all = 0; last=0;
            for( int i = 0; i<32; i++ ) { all += numRefsB[1][i]; if( numRefsB[1][i] != 0 ) last = i; }
            for( int i = 0; i<=last; i++ ) printf("%.1f%% ",(float)100.0*numRefsB[1][i]/all);
            printf("\n");

        }
    }
#endif // H264_STAT

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER

