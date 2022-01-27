/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#if defined (UMC_ENABLE_MP3_AUDIO_ENCODER) || defined (UMC_ENABLE_MP3_INT_AUDIO_ENCODER)

#ifndef __MP3ENC_OWN_H__
#define __MP3ENC_OWN_H__

#include "ippac.h"
#include "ippdc.h"
#include "ipps.h"

#include "mp3_own.h"
#include "mp3enc.h"
#include "mp3enc_tables.h"
#include "mp3enc_hufftables.h"
#include "mp3enc_psychoacoustic.h"
#include "align.h"
#include "bstream.h"

#include "vm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SI_MAX 10

#define NUM_CHANNELS 5
#define LFE_CHANNEL 1

#define MAX_GBUF_COEF 16

typedef struct {
    void    *phuftable;
    Ipp16s  mav_value;
    Ipp16s  linbits;
} MP3Enc_HuffmanTable;

typedef struct {
//  Quantization
    VM_ALIGN16_DECL(Ipp16s) quant_ix[2][2][576];

    VM_ALIGN16_DECL(Ipp16u) scalefac_l[2][2][32];
    VM_ALIGN16_DECL(Ipp16u) scalefac_s[2][2][12][3];
    Ipp32s max_bits[2][2];   // max bit for encode granule

//  huffman tables
    MP3Enc_HuffmanTable htables[32];

    IppMP3FrameHeader header;

// wav parameters
    Ipp32s stereo;
    Ipp32s frameSize;
    Ipp32s grnum;
    Ipp32s jschannel;
    Ipp32s br_mode;
    Ipp32s frameBits;

    /* multichannel */
    Ipp32s mc_channel_mask, mc_channel_conf;
    Ipp32s mc_sblimit, mc_lfe_filter_off;
    mp3_mc_header mc_header;

    Ipp32s mc_tc_sbgr_select;
    Ipp32s mc_dyn_cross_on;
    Ipp32s mc_dyn_cross_LR;
    Ipp32s mc_prediction_on;
    Ipp32s mc_channel;
    Ipp32s mc_alloc_bits;
    Ipp32s mc_dyn_cross_bits;
    Ipp32s mc_pred_mode;
    Ipp32s mc_tc_allocation;
    Ipp32s mc_tc_alloc[12];
    Ipp32s mc_dyn_cross_mode[12];
    Ipp32s mc_dyn_second_stereo[12];
    Ipp32s mc_prediction[8];
    Ipp32s mc_predsi[8][6];
    Ipp32s mc_pred_coeff[8][6][3];
    Ipp32s mc_delay_comp[8][6];
    Ipp32s mc_lfe_alloc;
    Ipp32s mc_lfe_scf;
    Ipp32s mc_lfe_spl[12];

// SIDE INFO
    Ipp32u si_main_data_begin;
    Ipp32u si_private_bits;
    Ipp32u si_part23Len[2][2];
    Ipp32u si_bigVals[2][2];
    Ipp32u si_count1[2][2];

    Ipp16s si_globGain[2][2];
    Ipp32u si_sfCompress[2][2];
    Ipp32u si_winSwitch[2][4];
    Ipp32u si_blockType[2][4];
    Ipp32u si_mixedBlock[2][4];
    Ipp32u si_pTableSelect[2][2][3];
    Ipp16s si_pSubBlkGain[2][2][3];
    Ipp32u si_address[2][2][3];
    Ipp32u si_reg0Cnt[2][2];
    Ipp32u si_reg1Cnt[2][2];
    Ipp32u si_preFlag[2][2];
    Ipp32u si_sfScale[2][2];
    Ipp32u si_cnt1TabSel[2][2];
    Ipp32u si_scfsi[2][4];
    Ipp32u si_part2Len[2][2];

    Ipp32u sfb_lmax, sfb_smax;
    Ipp32s slen[2][4];
    Ipp32s *sfb_part_table[2];

    Ipp32s sfb_l_max, sfb_s_max;

// END OF SIDE INFO

    sBitsreamBuffer mainStream;
    sBitsreamBuffer sideStream;

    Ipp32u buffer_main_data[1024];
    Ipp32u buffer_side_info[256];

/* l1 l2 start */
    VM_ALIGN16_DECL(Ipp16s)    allocation[NUM_CHANNELS][32];
    VM_ALIGN16_DECL(Ipp16s)    scalefactor_l1[2][32];
    VM_ALIGN16_DECL(Ipp32s)    sample[NUM_CHANNELS][32][36];
    VM_ALIGN16_DECL(Ipp16s)    scalefactor[NUM_CHANNELS][3][32];
    VM_ALIGN16_DECL(Ipp16u)    scfsi[NUM_CHANNELS][32];
    Ipp32s *nbal_alloc_table;
    Ipp8u  *alloc_table;
    Ipp32s jsbound;
    Ipp32s sblimit;
    Ipp32s sblimit_real;
/* ************ */

    Ipp32s slot_sizes[16];
    Ipp32s slot_size;
    Ipp32s main_data_ptr;
    Ipp32s resr_bytes;
    Ipp32s resr_mod_slot;
    Ipp32s bytes_in_gbuf;

    Ipp32s framesNum;

    Ipp32s ns_mode;
    Ipp32s stereo_mode;
    Ipp32s stereo_mode_param;
    Ipp32s stereo_mode_ext;

    Ipp32s quant_mode_fast;

    Ipp32s lowpass_maxline;
    Ipp32s lowpass_maxline_allowholes;

    Ipp8u  si_buf[SI_MAX][40];
    Ipp32s si_beg, si_new, si_num;

    Ipp32s upsample;
    Ipp32s si_bits;
} MP3Enc_com;

Ipp32s mp3enc_formatBitstream_l12(MP3Enc_com *state, Ipp8u *pOutputData);
Ipp32s mp3enc_formatBitstream_l3(MP3Enc_com *state, Ipp32s (*mdct_out)[2][576],
                                 Ipp8u *pOutputData);

Ipp32s mp3enc_writeLastFrames(MP3Enc_com *state,
                              Ipp8u *pOutputData);

Ipp32s mp3enc_huffInit(MP3Enc_HuffmanTable *htables, Ipp8u *mem, Ipp32s *size);

Ipp32s mp3enc_quantChooseTableLong(MP3Enc_com *state, Ipp32s gr, Ipp32s ch, Ipp16s *pInput, Ipp32s length,
                                   Ipp32s table);
Ipp32s mp3enc_quantCalcBitsLong(MP3Enc_com *state, Ipp16s *pInput, Ipp32s gr, Ipp32s ch);
Ipp32s mp3enc_quantCalcBits(MP3Enc_com *state, Ipp32s gr, Ipp32s ch);

void   mp3enc_quantIterReset(MP3Enc_com *state, Ipp32s gr, Ipp32s ch);
Ipp32s mp3enc_quantCalcPart2Len(MP3Enc_com *state, Ipp32s gr, Ipp32s ch);
Ipp32s mp3enc_quantScaleBitCount(MP3Enc_com *state, Ipp32s gr, Ipp32s ch);
Ipp32s mp3enc_quantcalcPart2Len(MP3Enc_com *state, Ipp32s gr, Ipp32s ch);

Ipp32s mp3enc_mc_trans_channel(MP3Enc_com *state, Ipp32s sbgr, Ipp32s ch);

Ipp32s mp3encLEBitrate(MP3Enc_com *state, Ipp32s slot_size);
Ipp32s mp3encGEBitrate(MP3Enc_com *state, Ipp32s slot_size);

#ifdef __cplusplus
}
#endif

#endif

#endif //UMC_ENABLE_XXX
