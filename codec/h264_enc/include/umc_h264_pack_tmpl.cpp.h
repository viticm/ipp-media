//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//


/* CONTENTS:

// Bitstream-independent functions
H264CoreEncoder_Derive_ctxIdxInc_CABAC
H264CoreEncoder_GetBlocksLumaContext
H264CoreEncoder_GetBlocksChromaContext

// Static functions
H264CoreEncoder_Encode_transform_size_8x8_flag
H264CoreEncoder_Encode_Inter_Type
H264CoreEncoder_Encode_BiPred_Type
H264CoreEncoder_Encode_ref_idx
H264CoreEncoder_Encode_MB_Vectors
H264CoreEncoder_PackSubBlockChroma
H264CoreEncoder_PackDC
H264CoreEncoder_Encode_AIC_Type
H264CoreEncoder_Encode_AIC_Type_16x16
H264CoreEncoder_Encode_PCM_MB

// Extern functions
H264CoreEncoder_Put_MB
H264CoreEncoder_Put_MBLuma
H264CoreEncoder_Put_MBChroma
H264CoreEncoder_Put_MBHeader
H264CoreEncoder_Encode_CBP
H264CoreEncoder_PackSubBlockLuma
*/

#if PIXBITS == 8

#define PIXTYPE Ipp8u
#define COEFFSTYPE Ipp16s
#define H264ENC_MAKE_NAME(NAME) NAME##_8u16s

#ifdef FAKE_BITSTREAM
#define H264ENC_MAKE_NAME_BS(NAME) H264BsFake_##NAME##_8u16s
#define H264ENC_MAKE_NAME_BS_DEP(NAME) NAME##_Fake_8u16s
#define H264BsType H264BsFake_8u16s
#else // real bitstream
#define H264ENC_MAKE_NAME_BS(NAME) H264BsReal_##NAME##_8u16s
#define H264ENC_MAKE_NAME_BS_DEP(NAME) NAME##_Real_8u16s
#define H264BsType H264BsReal_8u16s
#endif

#elif PIXBITS == 16

#define PIXTYPE Ipp16u
#define COEFFSTYPE Ipp32s
#define H264ENC_MAKE_NAME(NAME) NAME##_16u32s

#ifdef FAKE_BITSTREAM
#define H264ENC_MAKE_NAME_BS(NAME) H264BsFake_##NAME##_16u32s
#define H264ENC_MAKE_NAME_BS_DEP(NAME) NAME##_Fake_16u32s
#define H264BsType H264BsFake_16u32s
#else // real bitstream
#define H264ENC_MAKE_NAME_BS(NAME) H264BsReal_##NAME##_16u32s
#define H264ENC_MAKE_NAME_BS_DEP(NAME) NAME##_Real_16u32s
#define H264BsType H264BsReal_16u32s
#endif

#elif //PIXBITS

void H264EncoderFakeFunction() { UNSUPPORTED_PIXBITS; }

#endif //PIXBITS

#define H264CoreEncoderType H264ENC_MAKE_NAME(H264CoreEncoder)
#define H264SliceType H264ENC_MAKE_NAME(H264Slice)
#define H264CurrentMacroblockDescriptorType H264ENC_MAKE_NAME(H264CurrentMacroblockDescriptor)

#ifndef FAKE_BITSTREAM

/////////////////////////////////////
// Bitstream-independent functions
/////////////////////////////////////

static Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_Derive_ctxIdxInc_CABAC)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s list_num,
    Ipp32s block_idx)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s ref_idx_ctxIdxInc;
    H264BlockLocation block_a, block_b;

    if (BLOCK_IS_ON_LEFT_EDGE(block_idx))
    {
        block_a = cur_mb.BlockNeighbours.mbs_left[block_idx / 4];
    } else {
        block_a.mb_num = cur_mb.uMB;
        block_a.block_num = block_idx - 1;
    }

    if (BLOCK_IS_ON_TOP_EDGE(block_idx))
    {
        block_b = cur_mb.BlockNeighbours.mb_above;
        if (block_b.mb_num >= 0)
        {
            block_b.block_num += block_idx;
        }
    } else {
        block_b.mb_num = cur_mb.uMB;
        block_b.block_num = block_idx - 4;
    }

    Ipp32s condTermFlagA = 0;
    if (block_a.mb_num >= 0)
    {
        Ipp8u lval = (Ipp8u) (pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo) <
                            GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_a.mb_num]));

        MBTypeValue mb_type = core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_a.mb_num].mbtype;
        Ipp32s ref_idx = core_enc->m_pCurrentFrame->m_mbinfo.RefIdxs[list_num][block_a.mb_num].RefIdxs[block_a.block_num];
        Ipp32s refIdxZeroFlag = ref_idx > lval ? 0 : 1; // predModeEqualFlag here

        Ipp32s block_8x8 = block_subblock_mapping[block_a.block_num] / 4;
        SBTypeValue sbtype = core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_a.mb_num].sbtype[block_8x8];

        condTermFlagA = (IS_SKIP_MBTYPE(mb_type)
                    || IS_INTRA_MBTYPE(mb_type)
                    || (sbtype == SBTYPE_DIRECT)
                    || refIdxZeroFlag) ? 0 : 1;
    }

    Ipp32s condTermFlagB = 0;
    if (block_b.mb_num >= 0)
    {
        Ipp8u tval = (Ipp8u) (pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo) <
                            GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_b.mb_num]));

        MBTypeValue mb_type = core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_b.mb_num].mbtype;
        Ipp32s ref_idx = core_enc->m_pCurrentFrame->m_mbinfo.RefIdxs[list_num][block_b.mb_num].RefIdxs[block_b.block_num];
        Ipp32s refIdxZeroFlag = ref_idx > tval ? 0 : 1; // predModeEqualFlag here

        Ipp32s block_8x8 = block_subblock_mapping[block_b.block_num] / 4;
        SBTypeValue sbtype = core_enc->m_pCurrentFrame->m_mbinfo.mbs[block_b.mb_num].sbtype[block_8x8];
        condTermFlagB = (IS_SKIP_MBTYPE(mb_type)
                    || IS_INTRA_MBTYPE(mb_type)
                    || (sbtype == SBTYPE_DIRECT)
                    || refIdxZeroFlag) ? 0 : 1;
    }

    ref_idx_ctxIdxInc = condTermFlagA + 2*condTermFlagB;
    return ref_idx_ctxIdxInc;
}

////////////////////////////////////////////////////////////////////////////////
// calculate "N", which is used to select the appropriate VLC Table.
static Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_GetBlocksLumaContext)(
    void* state,
    H264CurrentMacroblockDescriptorType &cur_mb,
    Ipp32s x,
    Ipp32s y)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264MBAddr curMBAddr = cur_mb.uMB;
    bool use_above = y || cur_mb.BlockNeighbours.mb_above.mb_num>=0;
    bool use_left = x || cur_mb.BlockNeighbours.mbs_left[y].mb_num>=0;
    Ipp8u above_coeffs=0;
    if (use_above) above_coeffs= y==0?
        core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mb_above.mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mb_above.block_num+x]]:
        core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[block_subblock_mapping[y*4+x-4]];
    Ipp8u left_coeffs=0;
    if (use_left)left_coeffs = x==0?
        core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mbs_left[y].mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mbs_left[y].block_num]]:
        core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[block_subblock_mapping[y*4+x-1]];

    if(use_above && use_left) return (above_coeffs+left_coeffs+1)/2;
    else if (use_above ) return above_coeffs;
    else if (use_left) return left_coeffs;
    else return 0;
}

////////////////////////////////////////////////////////////////////////////////
// calculate "N", which is used to select the appropriate VLC Table.
static Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_GetBlocksChromaContext)(
    void* state,
    H264CurrentMacroblockDescriptorType &cur_mb,
    Ipp32s uBlockLeft,
    Ipp32s uBlockTop,
    bool component)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264MBAddr curMBAddr = cur_mb.uMB;
    Ipp8u above_coeffs=0;
    Ipp8u left_coeffs=0;

    bool use_above, use_left;
    if (component)
    {
        use_above = (uBlockTop>0) || cur_mb.BlockNeighbours.mb_above_chroma[1].mb_num>=0;
        use_left = (uBlockLeft>0) || cur_mb.BlockNeighbours.mbs_left_chroma[1][0].mb_num>=0; //TODO FPV
        if (use_above)
            above_coeffs= uBlockTop<0?
                core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mb_above_chroma[1].mb_num].numCoeff[-uBlockTop]:
                core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[uBlockTop];
        if (use_left)
            left_coeffs= uBlockLeft<0?
                core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mbs_left_chroma[1][0].mb_num].numCoeff[-uBlockLeft]: //TODO FPV
                core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[uBlockLeft];
    } else {
        use_above = (uBlockTop>0) || cur_mb.BlockNeighbours.mb_above_chroma[0].mb_num>=0;
        use_left = (uBlockLeft>0) || cur_mb.BlockNeighbours.mbs_left_chroma[0][0].mb_num>=0; //TODO FPV
        if (use_above)
            above_coeffs= uBlockTop<0?
                core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mb_above_chroma[0].mb_num].numCoeff[-uBlockTop]:
                core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[uBlockTop];
        if (use_left)
            left_coeffs= uBlockLeft<0?
                core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mbs_left_chroma[0][0].mb_num].numCoeff[-uBlockLeft]:  //TODO FPV
                core_enc->m_mbinfo.MacroblockCoeffsInfo[curMBAddr].numCoeff[uBlockLeft];
    }

    if(use_above && use_left) return (above_coeffs+left_coeffs+1)/2;
    else if (use_above ) return above_coeffs;
    else if (use_left) return left_coeffs;
    else return 0;
}

#endif //!FAKE_BITSTREAM

/////////////////////
// Static functions
/////////////////////

static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_transform_size_8x8_flag)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    if(core_enc->m_PicParamSet.entropy_coding_mode) {
        bool is_left_avail = cur_mb.BlockNeighbours.mbs_left[0].mb_num >= 0;
        bool is_top_avail = cur_mb.BlockNeighbours.mb_above.mb_num >= 0;
        Ipp32s  left_c = 0;
        Ipp32s  top_c = 0;

        if(is_left_avail && pGetMB8x8TSFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs + cur_mb.BlockNeighbours.mbs_left[0].mb_num) != 0
            && core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype != MBTYPE_SKIPPED)
            left_c = 1;
        if(is_top_avail &&  pGetMB8x8TSFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs + cur_mb.BlockNeighbours.mb_above.mb_num) != 0
            && core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype != MBTYPE_SKIPPED)
            top_c = 1;

        H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
            curr_slice->m_pbitstream,
            curr_slice->m_pbitstream->context_array + left_c + top_c + MB_TRANSFORM_SIZE_8X8_FLAG,
            pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo) != 0);
    } else {
        H264ENC_MAKE_NAME_BS(PutBit)(
            curr_slice->m_pbitstream,
            pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo));
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Encode_Inter_Type
//
////////////////////////////////////////////////////////////////////////////////
static bool H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_Inter_Type)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s N, length;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    MBTypeValue mb_type = cur_mb.GlobalMacroblockInfo->mbtype;
    bool noSubMbPartSizeLessThan8x8Flag = 1;
    switch (mb_type)
    {
    case MBTYPE_INTER:
    case MBTYPE_SKIPPED:
        N = 0;
        break;

    case MBTYPE_INTER_16x8:
        N = 1;
        break;

    case MBTYPE_INTER_8x16:
        N = 2;
        break;

    case MBTYPE_INTER_8x8:
        N = 3;
        break;
    case MBTYPE_INTER_8x8_REF0:
        N = 4;
        // P_8x8ref0 is not allowed for CABAC coding => for CAVLC we
        // do code P_8x8ref0 and for CABAC we switch to P_8x8.
        if(core_enc->m_PicParamSet.entropy_coding_mode) {
            mb_type = cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTER_8x8;
            N -= 1;//for cabac
        }
        break;
    default:
        N = -1;
        break;
    }

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        Ipp32s uLeftMBType = cur_mb.MacroblockNeighbours.mb_A < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_A].mbtype;
        Ipp32s uTopMBType = cur_mb.MacroblockNeighbours.mb_B < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_B].mbtype;
        H264ENC_MAKE_NAME_BS(MBTypeInfo_CABAC)(
            pBitstream,
            slice_type,
            N,
            (MB_Type)mb_type,
            (MB_Type)uLeftMBType,
            (MB_Type)uTopMBType);
    } else
        length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);

    if (mb_type == MBTYPE_INTER_8x8 || mb_type == MBTYPE_INTER_8x8_REF0) {
        // for each of the 8x8 blocks
        for (Ipp32u block=0; block<4; block++) {
            switch (cur_mb.GlobalMacroblockInfo->sbtype[block]) {
                case SBTYPE_8x8:
                    N = 0;
                    break;
                case SBTYPE_8x4:
                    N = 1;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;
                case SBTYPE_4x8:
                    N = 2;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;
                case SBTYPE_4x4:
                    N = 3;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;
                default:
                    break;
            }

            // Sub MB Type
            if (core_enc->m_PicParamSet.entropy_coding_mode)
                H264ENC_MAKE_NAME_BS(SubTypeInfo_CABAC)(pBitstream, slice_type, N);
            else
                length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);
        }
    }

    return noSubMbPartSizeLessThan8x8Flag;
}   // Encode_Inter_Type

////////////////////////////////////////////////////////////////////////////////
//
// Encode_BiPred_Type
//
////////////////////////////////////////////////////////////////////////////////
static bool H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_BiPred_Type)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s N;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    MBTypeValue mb_type = cur_mb.GlobalMacroblockInfo->mbtype;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    bool noSubMbPartSizeLessThan8x8Flag = 1;

    switch (mb_type) {

    case MBTYPE_DIRECT:
        N = 0;
        break;

    case MBTYPE_FORWARD:
        N = 1;
        break;

    case MBTYPE_BACKWARD:
        N = 2;
        break;

    case MBTYPE_BIDIR:
        N = 3;
        break;

    case MBTYPE_FWD_FWD_16x8:
        N = 4;
        break;

    case MBTYPE_FWD_FWD_8x16:
        N = 5;
        break;

    case MBTYPE_BWD_BWD_16x8:
        N = 6;
        break;

    case MBTYPE_BWD_BWD_8x16:
        N = 7;
        break;

    case MBTYPE_FWD_BWD_16x8:
        N = 8;
        break;

    case MBTYPE_FWD_BWD_8x16:
        N = 9;
        break;

    case MBTYPE_BWD_FWD_16x8:
        N = 10;
        break;

    case MBTYPE_BWD_FWD_8x16:
        N = 11;
        break;

    case MBTYPE_FWD_BIDIR_16x8:
        N = 12;
        break;

    case MBTYPE_FWD_BIDIR_8x16:
        N = 13;
        break;

    case MBTYPE_BWD_BIDIR_16x8:
        N = 14;
        break;

    case MBTYPE_BWD_BIDIR_8x16:
        N = 15;
        break;

    case MBTYPE_BIDIR_FWD_16x8:
        N = 16;
        break;

    case MBTYPE_BIDIR_FWD_8x16:
        N = 17;
        break;

    case MBTYPE_BIDIR_BWD_16x8:
        N = 18;
        break;

    case MBTYPE_BIDIR_BWD_8x16:
        N = 19;
        break;

    case MBTYPE_BIDIR_BIDIR_16x8:
        N = 20;
        break;

    case MBTYPE_BIDIR_BIDIR_8x16:
        N = 21;
        break;

    case MBTYPE_B_8x8:
        N = 22;
        break;

    default:
        N = -1;
        break;
    }

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        MBTypeValue mb_type = cur_mb.GlobalMacroblockInfo->mbtype;

        Ipp32s uLeftMBType = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype;
        Ipp32s uTopMBType = cur_mb.BlockNeighbours.mb_above.mb_num < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype;

        H264ENC_MAKE_NAME_BS(MBTypeInfo_CABAC)(
            pBitstream,
            slice_type,
            N,
            (MB_Type)mb_type,
            (MB_Type)uLeftMBType,
            (MB_Type)uTopMBType);
    }
    else {
        H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);
    }
    if (mb_type == MBTYPE_B_8x8) {
        // for each of the 8x8 blocks
        for (Ipp32u block=0; block<4; block++)
        {
            switch (cur_mb.GlobalMacroblockInfo->sbtype[block])
            {
                case SBTYPE_DIRECT:
                    N = 0;
                    if( !core_enc->m_SeqParamSet.direct_8x8_inference_flag )
                       noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_FORWARD_8x8:
                    N = 1;
                    break;

                case SBTYPE_BACKWARD_8x8:
                    N = 2;
                    break;

                case SBTYPE_BIDIR_8x8:
                    N = 3;
                    break;

                case SBTYPE_FORWARD_8x4:
                    N = 4;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_FORWARD_4x8:
                    N = 5;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BACKWARD_8x4:
                    N = 6;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BACKWARD_4x8:
                    N = 7;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BIDIR_8x4:
                    N = 8;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BIDIR_4x8:
                    N = 9;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_FORWARD_4x4:
                    N = 10;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BACKWARD_4x4:
                    N = 11;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                case SBTYPE_BIDIR_4x4:
                    N = 12;
                    noSubMbPartSizeLessThan8x8Flag = 0;
                    break;

                default:    // Unknown SBTYPE!
                    break;
            }

            // Sub MB Type
            if (core_enc->m_PicParamSet.entropy_coding_mode)
                H264ENC_MAKE_NAME_BS(SubTypeInfo_CABAC)(pBitstream, slice_type,N);
            else
                H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);
        }
    }

    return noSubMbPartSizeLessThan8x8Flag;
}   // Encode_BiPred_Type

// Encodes ref_idx_lX of a list num.
static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_ref_idx)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s block_idx,
    Ipp32s listNum)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    Ipp32s ref_idx_val = curr_slice->m_cur_mb.RefIdxs[listNum]->RefIdxs[block_idx];

    if(core_enc->m_PicParamSet.entropy_coding_mode){ // CABAC
        Ipp32s list_cabac = (listNum == LIST_0) ?  REF_IDX_L0 : REF_IDX_L1;
        Ipp32s ref_idx_ctxIdxInc = H264ENC_MAKE_NAME(H264CoreEncoder_Derive_ctxIdxInc_CABAC)(state, curr_slice, listNum, block_idx);

        bool curr_bit = (ref_idx_val != 0);
        Ipp8u* ctx = pBitstream->context_array + ctxIdxOffset[list_cabac];
        H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(pBitstream, ctx + ref_idx_ctxIdxInc,  curr_bit);
        ref_idx_ctxIdxInc = 3;
        while(curr_bit)
        {
            ref_idx_val--;
            ref_idx_ctxIdxInc = (ref_idx_ctxIdxInc < 5)? ref_idx_ctxIdxInc + 1: ref_idx_ctxIdxInc;
            curr_bit = (ref_idx_val != 0);
            H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(pBitstream, ctx + ref_idx_ctxIdxInc, curr_bit);
        }
    } else { // CAVLC
        Ipp32s num_ref_idx_active = (listNum == LIST_0)?
                                 curr_slice->num_ref_idx_l0_active<<pGetMBFieldDecodingFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo)
                               : curr_slice->num_ref_idx_l1_active<<pGetMBFieldDecodingFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo);
        if(num_ref_idx_active > 2) {
            H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, ref_idx_val);
        } else {
            // range == [0; 1]
            H264ENC_MAKE_NAME_BS(PutBit)(pBitstream, 1 - ref_idx_val);
        }
    }
    return;
} // Encode_ref_idx

////////////////////////////////////////////////////////////////////////////////
//
// Encode_MB_Vectors
//
////////////////////////////////////////////////////////////////////////////////
static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_MB_Vectors)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s num_vectors, ref0_vectors = 0;
    H264MotionVector    mvd[32];  // for the up to 16 deltas for the MB in each direction
    Ipp32s              block_index[32];    // for the up to 16 indexes for the blocks in each direction

    BlocksInfo mv_info_buf[32];
    BlocksInfo ref_info_buf[8];
    MV_Ref_Info info;
    info.mv_blocks = mv_info_buf;
    info.ref_blocks = ref_info_buf;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;

    get_blocks_info(cur_mb.GlobalMacroblockInfo->mbtype, cur_mb.GlobalMacroblockInfo->sbtype, info);

    num_vectors = 0;
    BlocksInfo * mv_blocks_info = info.mv_blocks;
    for(Ipp32s i = 0; i < info.size_mv; i++, mv_blocks_info++)
    {
        H264MotionVector mv;

        block_index[i] = mv_blocks_info->block_idx;

        H264ENC_MAKE_NAME(H264CoreEncoder_Calc_One_MV_Predictor)(
            state,
            curr_slice,
            mv_blocks_info->block_idx,
            mv_blocks_info->list_num,
            mv_blocks_info->block_w,
            mv_blocks_info->block_h,
            &mv,
            &mvd[i],
            true);

        StoreDMVs((&cur_mb.MVs[mv_blocks_info->list_num + 2]->MotionVectors[mv_blocks_info->block_idx]),
            0, mv_blocks_info->block_w, 1,
            0, mv_blocks_info->block_h, 1, mvd[i]);
    }

    ref0_vectors = info.mvs_l0_cnt;
    num_vectors = info.size_mv;

    // Note, right now, ref_idx_l0 and ref_idx_l1 values assumed to be zero are
    // written because we don't implement multiple reference frames.
    if ((curr_slice->num_ref_idx_l0_active<<pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo)) - 1)
    {
        BlocksInfo * ref_blocks_info = info.ref_blocks;
        for(Ipp32s i = 0; i < info.ref_l0_cnt; i++, ref_blocks_info++)
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_ref_idx)(state, curr_slice, ref_blocks_info->block_idx, ref_blocks_info->list_num);
    }

    if ((curr_slice->num_ref_idx_l1_active<<pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo)) - 1)
    {
        BlocksInfo * ref_blocks_info = info.ref_blocks + info.ref_l0_cnt;
        for(Ipp32s i = info.ref_l0_cnt; i < info.size_ref; i++, ref_blocks_info++)
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_ref_idx)(state, curr_slice, ref_blocks_info->block_idx, ref_blocks_info->list_num);
    }

    // Put vectors into bitstream.
    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        for (Ipp32s i = 0; i < num_vectors; i++)
        {
            H264MotionVector top_mv, left_mv;

            Ipp32s list_num = i < ref0_vectors ? LIST_0 : LIST_1;

            Ipp32s block_num = block_index[i];

            if (BLOCK_IS_ON_TOP_EDGE(block_num))
            {
                H264BlockLocation block = cur_mb.BlockNeighbours.mb_above;
                if (block.mb_num >= 0)
                {
                    block.block_num += block_num;
                    top_mv = core_enc->m_mbinfo.MVDeltas[list_num][block.mb_num].MotionVectors[block.block_num];
                    top_mv.mvy = (Ipp16s)labs(top_mv.mvy);

                    if (GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block.mb_num]) >
                        pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo))
                        top_mv.mvy <<= 1;
                    else if (GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block.mb_num]) <
                            pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo))
                        top_mv.mvy >>= 1;
                } else {
                    top_mv = null_mv;
                }
            }else{
                top_mv = cur_mb.MVs[list_num + 2]->MotionVectors[block_num - 4];
            }

            if (BLOCK_IS_ON_LEFT_EDGE(block_num))
            {
                H264BlockLocation block = cur_mb.BlockNeighbours.mbs_left[block_num / 4];
                if (block.mb_num >= 0)
                {
                    left_mv = core_enc->m_mbinfo.MVDeltas[list_num][block.mb_num].MotionVectors[block.block_num];

                    left_mv.mvy = (Ipp16s)labs(left_mv.mvy);

                    if (GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block.mb_num]) >
                        pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo))
                        left_mv.mvy <<= 1;
                    else if (GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[block.mb_num]) <
                            pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo))
                        left_mv.mvy >>= 1;
                } else {
                    left_mv = null_mv;
                }
            }else{
                left_mv = cur_mb.MVs[list_num + 2]->MotionVectors[block_num - 1];
            }

            H264ENC_MAKE_NAME_BS(MVD_CABAC)(
                pBitstream,
                mvd[i].mvx,
                left_mv.mvx,
                top_mv.mvx,
                i < ref0_vectors ? MVD_L0_0 : MVD_L1_0);

            H264ENC_MAKE_NAME_BS(MVD_CABAC)(
                pBitstream,
                mvd[i].mvy,
                left_mv.mvy,
                top_mv.mvy,
                i < ref0_vectors ? MVD_L0_1 : MVD_L1_1);

        }   // for i
    } else {
        for (Ipp32s i = 0; i < num_vectors; i++)
        {
            Ipp32s vec;

            // Horizontal component
            vec = mvd[i].mvx;
            H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, SIGNED_VLC_CODE(vec));
            // Vertical component
            vec = mvd[i].mvy;

            H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, SIGNED_VLC_CODE(vec));

        }   // for i
    }
}   // Encode_MB_Vectors

////////////////////////////////////////////////////////////////////////////////
//
// PackSubBlockChroma
//
////////////////////////////////////////////////////////////////////////////////
static Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackSubBlockChroma)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBlock)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    Ipp32u blocks_break = 16+(2<<cur_mb.chroma_format_idc);

    // Write coeff_token and trailing ones signs
    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {

        Ipp32s uvsel = uBlock >= blocks_break;

        bool is_intra = IS_TRUEINTRA_MBTYPE(cur_mb.GlobalMacroblockInfo->mbtype);
        Ipp32s upper_bit   = is_intra;
        Ipp32s left_bit    = is_intra;

        Ipp32s bit = uBlock-16; // -Luma
        Ipp32u block_pitch = 2 + ( (cur_mb.chroma_format_idc-1) & 0x2 );
        Ipp32s blocks = 2<<cur_mb.chroma_format_idc;
        Ipp32s up_block = uvsel ? (uBlock - blocks_break) < block_pitch ? 1: 0:
            (uBlock-16) < block_pitch ? 1 : 0;
        Ipp32s left_block = !((uvsel ? uBlock-blocks_break: uBlock-16 ) & (block_pitch-1));

        if (curr_slice->Block_CABAC[uBlock].uNumSigCoeffs != 0)
        {
            cur_mb.LocalMacroblockInfo->cbp_bits_chroma |= (1<<bit);
        }

        //--- get bits from neighbouring blocks ---
        if ( up_block )
        {
            H264BlockLocation upper = cur_mb.BlockNeighbours.mb_above_chroma[uvsel];

            if (upper.mb_num>=0)
            {
                upper_bit = BIT_SET(core_enc->m_mbinfo.mbs[upper.mb_num].cbp_bits_chroma, uBlock - 16 + blocks - block_pitch );
            }
        } else {
            upper_bit = BIT_SET(cur_mb.LocalMacroblockInfo->cbp_bits_chroma, bit - block_pitch);
        }

        if ( left_block )
        {
            H264BlockLocation left = cur_mb.BlockNeighbours.mbs_left_chroma[uvsel][0];
            if (left.mb_num>=0)
            {
                left_bit = BIT_SET(core_enc->m_mbinfo.mbs[left.mb_num].cbp_bits_chroma, bit+block_pitch-1);
            }
        } else {
            left_bit = BIT_SET(cur_mb.LocalMacroblockInfo->cbp_bits_chroma, bit-1);
        }

        Ipp32s CtxInc = 2*upper_bit+left_bit;
        H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
            pBitstream,
            pBitstream->context_array +
                ctxIdxOffsetFrameCoded[CODED_BLOCK_FLAG] +
                ctxIdxBlockCatOffset[CODED_BLOCK_FLAG][BLOCK_CHROMA_AC_LEVELS] +
                CtxInc,
            curr_slice->Block_CABAC[uBlock].uNumSigCoeffs != 0);

        if (curr_slice->Block_CABAC[uBlock].uNumSigCoeffs)
            H264ENC_MAKE_NAME_BS(ResidualBlock_CABAC)(
                pBitstream,
                &curr_slice->Block_CABAC[uBlock],
                !curr_slice->m_is_cur_mb_field);
    } else {
        Ipp32u uCoeffstoWrite;

        // Put coeff_token and trailing ones signs
        Ipp32s uBlockLeft = chroma_left[cur_mb.chroma_format_idc-1][uBlock-16];
        Ipp32s uBlockTop = chroma_top[cur_mb.chroma_format_idc-1][uBlock-16];

        // First calculate "N", which is used to select the appropriate VLC Table.
        Ipp32u N = H264ENC_MAKE_NAME(H264CoreEncoder_GetBlocksChromaContext)(state, cur_mb, uBlockLeft, uBlockTop, uBlock >= blocks_break);

        H264ENC_MAKE_NAME_BS(PutNumCoeffAndTrailingOnes)(
            pBitstream,
            N,
            0,  // Chroma DC?
            curr_slice->Block_RLE[uBlock].uNumCoeffs,
            curr_slice->Block_RLE[uBlock].uTrailing_Ones,
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs);

        if (curr_slice->Block_RLE[uBlock].uNumCoeffs != 0) {

            uCoeffstoWrite = curr_slice->Block_RLE[uBlock].uNumCoeffs -
                curr_slice->Block_RLE[uBlock].uTrailing_Ones;

            if (uCoeffstoWrite) {
                H264ENC_MAKE_NAME_BS(PutLevels)(
                    pBitstream,
                    curr_slice->Block_RLE[uBlock].iLevels,
                    uCoeffstoWrite,
                    curr_slice->Block_RLE[uBlock].uTrailing_Ones);
            }

            if (curr_slice->Block_RLE[uBlock].uNumCoeffs != 15) {
                H264ENC_MAKE_NAME_BS(PutTotalZeros)(
                    pBitstream,
                    curr_slice->Block_RLE[uBlock].uTotalZeros,
                    curr_slice->Block_RLE[uBlock].uNumCoeffs,
                    0); // Chroma DC?

                if ((curr_slice->Block_RLE[uBlock].uTotalZeros) &&
                    (curr_slice->Block_RLE[uBlock].uNumCoeffs > 1))
                    H264ENC_MAKE_NAME_BS(PutRuns)(
                        pBitstream,
                        curr_slice->Block_RLE[uBlock].uRuns,
                        curr_slice->Block_RLE[uBlock].uTotalZeros,
                        curr_slice->Block_RLE[uBlock].uNumCoeffs);
            }
        }
    }

    return UMC_OK;
} // PackSubBlockChroma


////////////////////////////////////////////////////////////////////////////////
//
// PackDC
//
////////////////////////////////////////////////////////////////////////////////
static Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackDC)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uPlane)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s N=0, iCoeffstoWrite, iMaxCoeffs;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;

    if (uPlane == Y_DC_RLE) {  // Y Plane 4x4 DC
        iMaxCoeffs = 16;
        // Put coeff_token and trailing ones signs
        if (core_enc->m_PicParamSet.entropy_coding_mode) {
            bool is_intra = IS_TRUEINTRA_MBTYPE(cur_mb.GlobalMacroblockInfo->mbtype);
            if (curr_slice->Block_CABAC[uPlane].uNumSigCoeffs != 0)
                cur_mb.LocalMacroblockInfo->cbp_bits |= 1;

            Ipp32s upper_bit = cur_mb.BlockNeighbours.mb_above.mb_num>=0 ?
                BIT_SET(core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].cbp_bits, 0)
                : is_intra;

            Ipp32s left_bit = cur_mb.BlockNeighbours.mbs_left[0].mb_num>=0 ?
                BIT_SET(core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].cbp_bits, 0)
                : is_intra;

            Ipp32s CtxInc = 2*upper_bit + left_bit;

            H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                pBitstream,
                pBitstream->context_array +
                ctxIdxOffsetFrameCoded[CODED_BLOCK_FLAG] +
                ctxIdxBlockCatOffset[CODED_BLOCK_FLAG][BLOCK_LUMA_DC_LEVELS] +
                CtxInc,
                curr_slice->Block_CABAC[uPlane].uNumSigCoeffs != 0);
        } else {
            // First calculate "N", which is used to select the appropriate VLC Table.
            bool bMBIsOnTopEdge = cur_mb.BlockNeighbours.mb_above.mb_num < 0;
            bool bMBIsOnLeftEdge = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0;

            if (!(bMBIsOnTopEdge || bMBIsOnLeftEdge)) { // Not on any edge
                // N is average of blocks above and to the left
                N = (core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mb_above.mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mb_above.block_num]] +
                     core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mbs_left[0].mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mbs_left[0].block_num]] + 1) >> 1;
            } else if (bMBIsOnTopEdge && !bMBIsOnLeftEdge) {
                // N is block to the left
                N =  core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mbs_left[0].mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mbs_left[0].block_num]];
            } else if (bMBIsOnLeftEdge && !bMBIsOnTopEdge) {
                // N is block above
                N = core_enc->m_mbinfo.MacroblockCoeffsInfo[cur_mb.BlockNeighbours.mb_above.mb_num].numCoeff[block_subblock_mapping[cur_mb.BlockNeighbours.mb_above.block_num]];
            } else {    // Upper left Corner
                N = 0;
            }
        }
    } else { // ! Y_DC_RLE

        iMaxCoeffs = 2<<cur_mb.chroma_format_idc;
        if (core_enc->m_PicParamSet.entropy_coding_mode)
        {
            bool is_intra = IS_TRUEINTRA_MBTYPE(cur_mb.GlobalMacroblockInfo->mbtype);

            if (curr_slice->Block_CABAC[uPlane].uNumSigCoeffs != 0)
            {
                cur_mb.LocalMacroblockInfo->cbp_bits |= (1<<(17+uPlane-U_DC_RLE));
            }

            Ipp32s upper_bit  = cur_mb.BlockNeighbours.mb_above_chroma[uPlane==V_DC_RLE].mb_num>=0 ?
                BIT_SET(core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above_chroma[uPlane==V_DC_RLE].mb_num].cbp_bits, 17+uPlane-U_DC_RLE)
                : is_intra;

            Ipp32s left_bit  = cur_mb.BlockNeighbours.mbs_left_chroma[uPlane==V_DC_RLE][0].mb_num>=0?
                BIT_SET(core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left_chroma[uPlane==V_DC_RLE][0].mb_num].cbp_bits, 17+uPlane-U_DC_RLE)
                : is_intra;

            Ipp32s CtxInc = 2*upper_bit+left_bit;

            H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                pBitstream,
                pBitstream->context_array +
                ctxIdxOffsetFrameCoded[CODED_BLOCK_FLAG] +
                ctxIdxBlockCatOffset[CODED_BLOCK_FLAG][BLOCK_CHROMA_DC_LEVELS] +
                CtxInc,
                curr_slice->Block_CABAC[uPlane].uNumSigCoeffs != 0);
        }
    }

    // Write coeff_token and trailing ones signs

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        H264ENC_MAKE_NAME_BS(ResidualBlock_CABAC)(
            pBitstream,
            &curr_slice->Block_CABAC[uPlane],
            !curr_slice->m_is_cur_mb_field);
    }
    else
    {
        H264ENC_MAKE_NAME_BS(PutNumCoeffAndTrailingOnes)(
            pBitstream,
            N,
            (uPlane != Y_DC_RLE)? cur_mb.chroma_format_idc : 0,   // Chroma DC? and value
            curr_slice->Block_RLE[uPlane].uNumCoeffs,
            curr_slice->Block_RLE[uPlane].uTrailing_Ones,
            curr_slice->Block_RLE[uPlane].uTrailing_One_Signs);

        if (curr_slice->Block_RLE[uPlane].uNumCoeffs != 0)
        {
            iCoeffstoWrite = curr_slice->Block_RLE[uPlane].uNumCoeffs -
                curr_slice->Block_RLE[uPlane].uTrailing_Ones;

            if (iCoeffstoWrite) {
                H264ENC_MAKE_NAME_BS(PutLevels)(
                    pBitstream,
                    curr_slice->Block_RLE[uPlane].iLevels,
                    iCoeffstoWrite,
                    curr_slice->Block_RLE[uPlane].uTrailing_Ones);
            }

            if (curr_slice->Block_RLE[uPlane].uNumCoeffs != iMaxCoeffs)
            {
                H264ENC_MAKE_NAME_BS(PutTotalZeros)(
                    pBitstream,
                    curr_slice->Block_RLE[uPlane].uTotalZeros,
                    curr_slice->Block_RLE[uPlane].uNumCoeffs,
                    (uPlane != Y_DC_RLE)? cur_mb.chroma_format_idc : 0);  // Chroma DC?

                if ((curr_slice->Block_RLE[uPlane].uTotalZeros) &&
                    (curr_slice->Block_RLE[uPlane].uNumCoeffs > 1))
                    H264ENC_MAKE_NAME_BS(PutRuns)(
                        pBitstream,
                        curr_slice->Block_RLE[uPlane].uRuns,
                        curr_slice->Block_RLE[uPlane].uTotalZeros,
                        curr_slice->Block_RLE[uPlane].uNumCoeffs);
            }
        }
    }

    return UMC_OK;
} // PackDC

////////////////////////////////////////////////////////////////////////////////
// Encode MB_Type and Advanced Intra Prediction mode for each luma 4x4 block.
// Also Encode the Chroma 8x8 Intra Prediction mode.
////////////////////////////////////////////////////////////////////////////////
static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s prob0;
    T_AIMode iLeftMode;
    T_AIMode iAboveMode;
    T_AIMode iThisMode;
    T_AIMode iMostProbMode;
    Ipp32u uBlock, blocks, block;
    Ipp32u N, length;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    EnumSliceType slice_type = curr_slice->m_slice_type;

    // Encode MB_Type
    N = CALC_4x4_INTRA_MB_TYPE(slice_type);

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        Ipp32s left_n = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype;
        Ipp32s top_n = cur_mb.BlockNeighbours.mb_above.mb_num < 0 ? NUMBER_OF_MBTYPES :
                core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype;

        H264ENC_MAKE_NAME_BS(MBTypeInfo_CABAC)(
            pBitstream,
            slice_type,
            N,
            MBTYPE_INTRA,
            (MB_Type)left_n,
            (MB_Type)top_n);
    }
    else
        length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);

    if(core_enc->m_PicParamSet.transform_8x8_mode_flag) {
        H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_transform_size_8x8_flag)(state, curr_slice);
    }

    if( pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo) ) blocks=4;
    else blocks=16;

    // Loop over 16 blocks. One block is coded in each iteration
    for (block = 0; block < blocks; block++)
    {
        if( blocks == 4 ) uBlock=block<<2;
            //uBlock=(block&0x2)*4 + (block&0x1)*2;
        else uBlock = block;

        //Here is uBlock in raster order.

        // Use the prediction mode of the block above, the block to
        // the left, and current block to code type, combining in a pair
        // of blocks. Prediction mode of above/left blocks is 2 if they
        // are not present (edge) or are not INTRA.

        // Block one, block above type
        iAboveMode = -1;
        if (BLOCK_IS_ON_TOP_EDGE(block_subblock_mapping[uBlock]))
        {
            H264BlockLocation block = cur_mb.BlockNeighbours.mb_above;
            if (block.mb_num >= 0)
            {
                block.block_num += block_subblock_mapping[uBlock];
                iAboveMode = core_enc->m_mbinfo.intra_types[block.mb_num].intra_types[block_subblock_mapping[block.block_num]];
            }
        }else {
            Ipp32s block_num = block_subblock_mapping[uBlock] - 4; // neighbour in non-raster order
            block_num = block_subblock_mapping[block_num];
            iAboveMode = cur_mb.intra_types[block_num];
        }

        // Block one, block left type
        iLeftMode = -1;
        if (BLOCK_IS_ON_LEFT_EDGE(block_subblock_mapping[uBlock]))
        {
            H264BlockLocation block = cur_mb.BlockNeighbours.mbs_left[block_subblock_mapping[uBlock] / 4];
            if (block.mb_num >= 0)
            {
                iLeftMode = core_enc->m_mbinfo.intra_types[block.mb_num].intra_types[block_subblock_mapping[block.block_num]];
            }
        } else {
            Ipp32s block_num = block_subblock_mapping[uBlock] - 1; // neighbour in non-raster order
            block_num = block_subblock_mapping[block_num];
            iLeftMode = cur_mb.intra_types[block_num];
        }

        // Block one AI mode
        iThisMode = cur_mb.intra_types[uBlock];

        // The most probable mode is the Minimum of the two predictors
        iMostProbMode = MIN(iAboveMode, iLeftMode);
        if (iMostProbMode == -1) {  // Unless one or both of the predictors is "outside"
            iMostProbMode = 2;      // In this case it defaults to mode 2 (DC Prediction).
        }
        // The probability of the current mode is 0 if it equals the Most Probable Mode
        if (iMostProbMode == iThisMode) {
            prob0 = -1;
        } else if (iThisMode < iMostProbMode ) {    // Otherwise, the mode probability increases
            prob0 = iThisMode;                      // (the opposite of intuitive notion of probability)
        } else {                                    // with the order of the remaining modes.
            prob0 = iThisMode-1;
        }

        if (core_enc->m_PicParamSet.entropy_coding_mode)
        {
            H264ENC_MAKE_NAME_BS(IntraPredMode_CABAC)(pBitstream, prob0);
        } else {
            if (prob0 >= 0) {
                H264ENC_MAKE_NAME_BS(PutBits)(pBitstream, prob0, 4);    // Not the most probable type, 1 bit(0) +3 bits to signal actual mode
            } else {
                H264ENC_MAKE_NAME_BS(PutBit)(pBitstream, 1);  // most probable type in 1 bit
            }
        }
    }

    if(cur_mb.chroma_format_idc != 0) {
        // Encode Chroma Prediction Mode
        if (core_enc->m_PicParamSet.entropy_coding_mode)
        {
            Ipp32s left_p = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0 ? 0 :
                core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].intra_chroma_mode;
            Ipp32s top_p = cur_mb.BlockNeighbours.mb_above.mb_num < 0 ? 0 :
                core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].intra_chroma_mode;

            H264ENC_MAKE_NAME_BS(ChromaIntraPredMode_CABAC)(pBitstream, cur_mb.LocalMacroblockInfo->intra_chroma_mode,left_p!=0,top_p!=0);
        } else {
            length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, cur_mb.LocalMacroblockInfo->intra_chroma_mode);
        }
    }
}   // Encode_AIC_Type

////////////////////////////////////////////////////////////////////////////////
// Encode MB_Type and Luma/Chroma Prediction modes for Intra 16x16 MB
////////////////////////////////////////////////////////////////////////////////
static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type_16x16)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u length;
    Ipp32s N;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;

    // Encode MB_Type, Luma Prediction Mode, Luma AC coeff flag, and Chroma DC/AC (NC) flag

    N = CALC_16x16_INTRA_MB_TYPE(curr_slice->m_slice_type,
                                 cur_mb.LocalMacroblockInfo->intra_16x16_mode,//m_cur_mb.intra_types[0],
                                 cur_mb.MacroblockCoeffsInfo->chromaNC,
                                 cur_mb.MacroblockCoeffsInfo->lumaAC);

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        Ipp32s left_n = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0 ? NUMBER_OF_MBTYPES :
            core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype;
        Ipp32s top_n = cur_mb.BlockNeighbours.mb_above.mb_num < 0 ? NUMBER_OF_MBTYPES :
            core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype;

        H264ENC_MAKE_NAME_BS(MBTypeInfo_CABAC)(
            pBitstream,
            curr_slice->m_slice_type,
            N,
            MBTYPE_INTRA_16x16,
            (MB_Type)left_n,
            (MB_Type)top_n);

        if(cur_mb.chroma_format_idc != 0) {
            Ipp32s left_p = cur_mb.BlockNeighbours.mbs_left[0].mb_num < 0 ? 0 :
                core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].intra_chroma_mode;
            Ipp32s top_p = cur_mb.BlockNeighbours.mb_above.mb_num < 0 ? 0 :
                core_enc->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].intra_chroma_mode;

            H264ENC_MAKE_NAME_BS(ChromaIntraPredMode_CABAC)(pBitstream, cur_mb.LocalMacroblockInfo->intra_chroma_mode,left_p!=0,top_p!=0);
        }
    }
    else
    {
        length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);

        if(cur_mb.chroma_format_idc != 0) {
            // Encode Chroma Prediction Mode
            length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, cur_mb.LocalMacroblockInfo->intra_chroma_mode);
        }
    }
}   // Encode_AIC_Type_16x16

////////////////////////////////////////////////////////////////////////////////
// Encode MB_Type and alignment bits for PCM 16x16 MB followed by the
// 384 pcm_bytes for the MB
////////////////////////////////////////////////////////////////////////////////
static void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_PCM_MB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u length, row, col;
    Ipp32s N;
    PIXTYPE*  pSrcPlane;     // start of plane to encode
    PIXTYPE*  pRecPlane;     // start of reconstructed plane
    Ipp32s    pitchPixels;   // buffer pitch in pixels.
    Ipp32s    offset;        // to upper left corner of block from start of plane (in pixels)
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;

    // Encode MB_Type
    N = CALC_PCM_MB_TYPE(curr_slice->m_slice_type);

    length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, N);

    // Write the pcm_alignment bit(s) if necessary.
    H264BsBase_ByteAlignWithOnes(pBitstream);

    // Now, write the pcm_bytes and update the reconstructed buffer...

    offset = core_enc->m_pMBOffsets[curr_slice->m_cur_mb.uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
    pSrcPlane = &((PIXTYPE*)core_enc->m_pCurrentFrame->m_pYPlane)[offset];
    pRecPlane = &((PIXTYPE*)core_enc->m_pReconstructFrame->m_pYPlane)[offset];
    pitchPixels = core_enc->m_pCurrentFrame->m_pitchPixels<<curr_slice->m_is_cur_mb_field;

    // Y plane first

    for (row=0; row<16; row++) {
        for (col=0; col<16; col++) {

            if (!pSrcPlane[col])    // Replace forbidden zero samples with 1.
                pSrcPlane[col] = 1;
            // Write sample to BS
            H264ENC_MAKE_NAME_BS(PutBits)(pBitstream, pSrcPlane[col], 8);
        }
        memcpy(pRecPlane, pSrcPlane, 16*sizeof(PIXTYPE));   // Copy row to reconstructed buffer
        pSrcPlane += pitchPixels;
        pRecPlane += pitchPixels;
    }

    if(curr_slice->m_cur_mb.chroma_format_idc == 0) {
        // Monochrome mode.
        return;
    }

    offset = core_enc->m_pMBOffsets[curr_slice->m_cur_mb.uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
    pSrcPlane = &((PIXTYPE*)core_enc->m_pCurrentFrame->m_pUPlane)[offset];
    pRecPlane = &((PIXTYPE*)core_enc->m_pReconstructFrame->m_pUPlane)[offset];

    // U plane next

    for (row=0; row<8; row++) {
        for (col=0; col<8; col++) {

            if (!pSrcPlane[col])    // Replace forbidden zero samples with 1.
                pSrcPlane[col] = 1;
            // Write sample to BS
            H264ENC_MAKE_NAME_BS(PutBits)(pBitstream, pSrcPlane[col], 8);
        }
        memcpy(pRecPlane, pSrcPlane, 8*sizeof(PIXTYPE));    // Copy row to reconstructed buffer
        pSrcPlane += pitchPixels;
        pRecPlane += pitchPixels;
    }

    pSrcPlane = &((PIXTYPE*)core_enc->m_pCurrentFrame->m_pVPlane)[offset];
    pRecPlane = &((PIXTYPE*)core_enc->m_pReconstructFrame->m_pVPlane)[offset];

    // V plane last

    for (row=0; row<8; row++) {
        for (col=0; col<8; col++) {

            if (!pSrcPlane[col])    // Replace forbidden zero samples with 1.
                pSrcPlane[col] = 1;
            // Write sample to BS
            H264ENC_MAKE_NAME_BS(PutBits)(pBitstream, pSrcPlane[col], 8);
        }
        memcpy(pRecPlane, pSrcPlane, 8*sizeof(PIXTYPE));    // Copy row to reconstructed buffer
        pSrcPlane += pitchPixels;
        pRecPlane += pitchPixels;
    }

}   // Encode_PCM_MB

//////////////////////
// Extern functions
//////////////////////

Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s b1,b2,b3;
    Status status = UMC_OK;

    b1 = H264BsBase_GetBsOffset_CABAC(curr_slice->m_pbitstream);
    status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBHeader)(state, curr_slice);
    if (status != UMC_OK)
        return status;

    b2 = H264BsBase_GetBsOffset_CABAC(curr_slice->m_pbitstream);
    if( cur_mb.GlobalMacroblockInfo->mbtype != MBTYPE_SKIPPED ){
        status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBLuma)(state, curr_slice);

        if( status == UMC_OK && cur_mb.chroma_format_idc != 0)
            status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBChroma)(state, curr_slice);
    }
    b3 = H264BsBase_GetBsOffset_CABAC(curr_slice->m_pbitstream);


    //f cur_mb.LocalMacroblockInfo->header_bits = b2 - b1;
    //f cur_mb.LocalMacroblockInfo->texture_bits = b3 - b2;
#ifdef ALT_RC
    curr_slice->m_Texture_Sum += b3 - b2;
#endif
    return status;
}

Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBLuma)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Status status = UMC_OK;

    // if 16x16 intra MB, code the double transform DC coeffs
    if (cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTRA_16x16){
        status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackDC)(state, curr_slice, Y_DC_RLE);
        if (status != UMC_OK) return status;
    }

    for (Ipp32s i8x8 = 0; i8x8 < 4; i8x8++) {
        if(!pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo)
            || !core_enc->m_PicParamSet.entropy_coding_mode)
        {
            for(Ipp32s i4x4 = 0; i4x4 < 4; i4x4++) // Loop over the all of the luma 4x4 blocks
            {
                Ipp32s i = i8x8*4 + i4x4;
                // skip if no coeff in MB
                if (((cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTRA_16x16) &&
                        (cur_mb.MacroblockCoeffsInfo->lumaAC)) ||
                    (cur_mb.LocalMacroblockInfo->cbp & (1<<i8x8)))
                {
                    status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackSubBlockLuma)(state, curr_slice, i);
                    if (status != UMC_OK)
                    {
                        return status;
                    }
                }
            }
        }
        else if(cur_mb.LocalMacroblockInfo->cbp & (1<<i8x8)) {
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackSubBlockLuma)(state, curr_slice, i8x8);
        }
    }

    return status;
}

Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBChroma)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Status status = UMC_OK;

    if (cur_mb.MacroblockCoeffsInfo->chromaNC){   // DC Residuals?
         status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackDC)(state, curr_slice, U_DC_RLE);
         if (status != UMC_OK) return status;

         status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackDC)(state, curr_slice, V_DC_RLE);
         if (status != UMC_OK) return status;
    }

    if (cur_mb.MacroblockCoeffsInfo->chromaNC == 2){    // AC Residuals?
        for (Ipp32s i = 16; i < 16+(4<<cur_mb.chroma_format_idc); i++){
          status = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackSubBlockChroma)(state, curr_slice, i);
          if (status != UMC_OK) return status;
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
//
// Put_MBHeader
//
// Writes one MB header to the bitstream.
//
////////////////////////////////////////////////////////////////////////////////

Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Put_MBHeader)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Status ps = UMC_OK;
    bool bIntra;
    MBTypeValue mb_type;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp8u uCBP = cur_mb.LocalMacroblockInfo->cbp;
    Ipp32u uCBPLuma;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    Ipp8s &iLastXmittedQP = curr_slice->m_iLastXmittedQP;
    Ipp32s &prev_dquant = curr_slice->m_prev_dquant;

    mb_type = cur_mb.GlobalMacroblockInfo->mbtype;

    //f StoreDMVs(&cur_mb.MVs[LIST_0 + 2]->MotionVectors[0],0,4,1,0,4,1, null_mv);
    //f StoreDMVs(&cur_mb.MVs[LIST_1 + 2]->MotionVectors[0],0,4,1,0,4,1, null_mv);
    for (Ipp32s i = 0; i < 16; i ++) {
        cur_mb.MVs[LIST_0+2]->MotionVectors[i] = null_mv;
        cur_mb.MVs[LIST_1+2]->MotionVectors[i] = null_mv;
    }

    // set CBP for the MB based upon coded block bits.
    uCBPLuma = cur_mb.LocalMacroblockInfo->cbp_luma;

    if ((mb_type != MBTYPE_INTRA_16x16) && (mb_type != MBTYPE_PCM))
    {
        uCBP = (Ipp8u)
            ((((uCBPLuma >> 0) | (uCBPLuma >> 1) | (uCBPLuma >> 2) | (uCBPLuma >> 3)) & 1) |    // 8x8 - 0
            (((uCBPLuma >> 3) | (uCBPLuma >> 4) | (uCBPLuma >> 5) | (uCBPLuma >> 6)) & 2) |     // 8x8 - 1
            (((uCBPLuma >> 6) | (uCBPLuma >> 7) | (uCBPLuma >> 8) | (uCBPLuma >> 9)) & 4) |     // 8x8 - 2
            (((uCBPLuma >> 9) | (uCBPLuma >> 10) | (uCBPLuma >> 11) | (uCBPLuma >> 12)) & 8));  // 8x8 - 3

        uCBP += (cur_mb.MacroblockCoeffsInfo->chromaNC << 4);

        cur_mb.LocalMacroblockInfo->cbp = uCBP;
    } else  {
        cur_mb.LocalMacroblockInfo->cbp = 0;
    }

    // Shift the CBP to match the decoder in the deblocking filter...
    cur_mb.LocalMacroblockInfo->cbp_luma = ((uCBPLuma & 0xffff)<<1);

    bIntra = IS_INTRA_MBTYPE(mb_type);

    if (slice_type == INTRASLICE)
    {
        // INTRA slice
        if (core_enc->m_SliceHeader.MbaffFrameFlag && (cur_mb.uMB & 1) == 0)
        {
            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                Ipp32s left_mb_field = cur_mb.MacroblockNeighbours.mb_A >=0?
                    GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_A]) : 0;
                Ipp32s top_mb_field = cur_mb.MacroblockNeighbours.mb_B >=0 ?
                    GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_B]) : 0;

                H264ENC_MAKE_NAME_BS(MBFieldModeInfo_CABAC)(
                    pBitstream,
                    pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo),
                    left_mb_field,
                    top_mb_field);
            }
            else
                H264ENC_MAKE_NAME_BS(PutBit)(
                    pBitstream,
                    pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo));

        }
        // Encode Advanced Intra Coding type
        if (mb_type == MBTYPE_INTRA_16x16)
        {
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type_16x16)(state, curr_slice);
            // Always Send Delta_QP for Intra 16x16 mode (needed for DC coeffs)
            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                Ipp32s prevMB = (cur_mb.uMB > 0)? prev_dquant != 0 : 0;
                H264ENC_MAKE_NAME_BS(DQuant_CABAC)(
                    pBitstream,
                    cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP,
                    prevMB);
            }
            else
                H264ENC_MAKE_NAME_BS(PutDQUANT)(
                    pBitstream,
                    cur_mb.LocalMacroblockInfo->QP,
                    iLastXmittedQP);

            prev_dquant = cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP;
            iLastXmittedQP = cur_mb.LocalMacroblockInfo->QP;
        }
        else if (mb_type == MBTYPE_INTRA)
        {
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type)(state, curr_slice);
            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_CBP)(state, curr_slice);

            if (uCBP > 0) { // Only Send Delta_QP if there are residuals to follow.
                if (core_enc->m_PicParamSet.entropy_coding_mode)
                {
                    Ipp32s prevMB = (cur_mb.uMB > 0)? curr_slice->m_prev_dquant != 0 : 0;
                    H264ENC_MAKE_NAME_BS(DQuant_CABAC)(
                        pBitstream,
                        cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP,
                        prevMB);
                }
                else
                    H264ENC_MAKE_NAME_BS(PutDQUANT)(
                        pBitstream,
                        cur_mb.LocalMacroblockInfo->QP,
                        iLastXmittedQP);

                prev_dquant = cur_mb.LocalMacroblockInfo->QP-iLastXmittedQP;
                iLastXmittedQP = cur_mb.LocalMacroblockInfo->QP;
            } else {
                // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
                // core_enc->m_iLastXmittedQP.
                cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;
                prev_dquant = 0;
            }
        }
        else if (mb_type == MBTYPE_PCM) {

            H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_PCM_MB)(state, curr_slice);

            // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
            // core_enc->m_iLastXmittedQP.
            cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;
            prev_dquant = 0;
        } else {
        }
    } else {
        //if (!IS_INTRA_MBTYPE(core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB].mbtype))
        //  cur_mb.LocalMacroblockInfo->intra_chroma_mode = 0;
        // Non-INTRA
        // COD
        // check for skipped MB

//        bool can_skip = (((uCBP == 0)) && ((MBTYPE_DIRECT == mb_type) || (MBTYPE_SKIPPED == mb_type)));
//        bool can_skip = ((MBTYPE_DIRECT == mb_type) || (MBTYPE_SKIPPED == mb_type));
        bool can_skip = (MBTYPE_SKIPPED == mb_type);
//            ((MBTYPE_INTER <= mb_type) && Skip_MV_Predicted(curr_slice, uMB, NULL))));

        if (can_skip && core_enc->m_SliceHeader.MbaffFrameFlag)
        {
            // TODO we could skip top MB if know that bottom MB will not be skipped.
            if (!((cur_mb.uMB & 1) == 1 && cur_mb.GlobalMacroblockPairInfo->mbtype != MBTYPE_SKIPPED))
            {
                Ipp32s mb_field_decoding_flag = 0;
                if (cur_mb.MacroblockNeighbours.mb_A < 0)
                {
                    if (cur_mb.MacroblockNeighbours.mb_B >= 0)
                    {
                        mb_field_decoding_flag = GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_B]);
                    }
                } else {
                    mb_field_decoding_flag = GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_A]);
                }

                can_skip = (mb_field_decoding_flag == pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo));
            }
        }

        if (can_skip)
        {
            // Skipped
            curr_slice->m_uSkipRun++;

            // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
            // core_enc->m_iLastXmittedQP.
            prev_dquant = 0;
            cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;
            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                Ipp32s left_c=0, top_c = 0;

                bool is_left_avail = cur_mb.BlockNeighbours.mbs_left[0].mb_num>=0;
                bool is_top_avail = cur_mb.BlockNeighbours.mb_above.mb_num>=0;

                if (IS_B_SLICE(slice_type))
                {
                    if(is_left_avail &&
                        (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype != MBTYPE_SKIPPED))
                        left_c = 1;

                    if(is_top_avail &&
                        (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype != MBTYPE_SKIPPED))
                        top_c = 1;
                    H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                        pBitstream,
                        pBitstream->context_array + left_c + top_c + ctxIdxOffset[MB_SKIP_FLAG_B],
                        1);
                } else {
                    if(is_left_avail && (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype != MBTYPE_SKIPPED))
                        left_c = 1;

                    if(is_top_avail &&  (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype != MBTYPE_SKIPPED))
                        top_c = 1;
                    H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                        pBitstream,
                        pBitstream->context_array + left_c + top_c + ctxIdxOffset[MB_SKIP_FLAG_P_SP],
                        1);
                }

                /*m_cur_mb.LocalMacroblockInfo->cbp = core_enc->m_pCurrentFrame->pMBData[uMB].cbp_bits =
                core_enc->m_pCurrentFrame->pMBData[uMB].uChromaNC =
                core_enc->m_pCurrentFrame->pMBData[uMB].uLumaAC = */
                prev_dquant = 0;
            }
            cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_SKIPPED;
/*            uCBP = 0;
            cur_mb.LocalMacroblockInfo->cbp_luma = 0;
            cur_mb.LocalMacroblockInfo->cbp_chroma = 0;
            cur_mb.LocalMacroblockInfo->cbp = 0;
*/        } else {
            // Code the number of skipped MBs (mb_skip_run) and reset the counter
            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                Ipp32s left_c=0, top_c = 0;

                bool is_left_avail = cur_mb.BlockNeighbours.mbs_left[0].mb_num>=0;
                bool is_top_avail = cur_mb.BlockNeighbours.mb_above.mb_num>=0;

                if (IS_B_SLICE(slice_type))
                {
                    if(is_left_avail &&
                        (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype != MBTYPE_SKIPPED))
                        left_c = 1;

                    if(is_top_avail &&
                        (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype != MBTYPE_SKIPPED))
                        top_c = 1;
                    H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                        pBitstream,
                        pBitstream->context_array + left_c + top_c + ctxIdxOffset[MB_SKIP_FLAG_B],
                        0);
                } else {
                    if(is_left_avail && (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mbs_left[0].mb_num].mbtype != MBTYPE_SKIPPED))
                        left_c = 1;

                    if(is_top_avail && (core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.BlockNeighbours.mb_above.mb_num].mbtype != MBTYPE_SKIPPED))
                        top_c = 1;
                    H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                        pBitstream,
                        pBitstream->context_array + left_c + top_c + ctxIdxOffset[MB_SKIP_FLAG_P_SP],
                        0);
                }

                if (core_enc->m_SliceHeader.MbaffFrameFlag && ((cur_mb.uMB & 1) == 0 &&
                    cur_mb.GlobalMacroblockInfo->mbtype!=MBTYPE_SKIPPED ||
                    ((cur_mb.uMB & 1) == 1 && cur_mb.GlobalMacroblockInfo->mbtype!=MBTYPE_SKIPPED &&
                    cur_mb.GlobalMacroblockPairInfo->mbtype==MBTYPE_SKIPPED)))
                {
                        Ipp32s left_mb_field = cur_mb.MacroblockNeighbours.mb_A >= 0?
                            GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_A]):0;
                        Ipp32s top_mb_field= cur_mb.MacroblockNeighbours.mb_B >= 0?
                            GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_B]):0;

                        H264ENC_MAKE_NAME_BS(MBFieldModeInfo_CABAC)(
                            pBitstream,
                            pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo),
                            left_mb_field,
                            top_mb_field);
                }
            } else {
                H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, curr_slice->m_uSkipRun);
                if (core_enc->m_SliceHeader.MbaffFrameFlag && ((cur_mb.uMB & 1) == 0 &&
                    cur_mb.GlobalMacroblockInfo->mbtype!=MBTYPE_SKIPPED ||
                    ((cur_mb.uMB & 1) == 1 && cur_mb.GlobalMacroblockInfo->mbtype!=MBTYPE_SKIPPED &&
                    cur_mb.GlobalMacroblockPairInfo->mbtype==MBTYPE_SKIPPED)))
                {
                    H264ENC_MAKE_NAME_BS(PutBit)(
                        pBitstream,
                        pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo));
                }
            }

            curr_slice->m_uSkipRun = 0;

            if (bIntra)
            {
                // Encode Advanced Intra Coding type
                if (mb_type == MBTYPE_INTRA_16x16)
                {
                    H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type_16x16)(state, curr_slice);
                    // Always Send Delta_QP for Intra 16x16 mode (needed for DC coeffs)
                    if (core_enc->m_PicParamSet.entropy_coding_mode)
                    {
                        Ipp32s prevMB = (cur_mb.uMB > 0)? curr_slice->m_prev_dquant != 0 : 0;
                        H264ENC_MAKE_NAME_BS(DQuant_CABAC)(
                            pBitstream,
                            cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP,
                            prevMB);
                    }
                    else
                        H264ENC_MAKE_NAME_BS(PutDQUANT)(
                            pBitstream,
                            cur_mb.LocalMacroblockInfo->QP,
                            iLastXmittedQP);
                    prev_dquant = cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP;
                    iLastXmittedQP = cur_mb.LocalMacroblockInfo->QP;
                }
                else if (mb_type == MBTYPE_INTRA)
                {
                    H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_AIC_Type)(state, curr_slice);
                    H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_CBP)(state, curr_slice);

                    if (uCBP > 0) { // Only Send Delta_QP if there are residuals to follow.
                        if (core_enc->m_PicParamSet.entropy_coding_mode)
                        {
                            Ipp32s prevMB = (cur_mb.uMB > 0)? curr_slice->m_prev_dquant !=0: 0;
                            H264ENC_MAKE_NAME_BS(DQuant_CABAC)(
                                pBitstream,
                                cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP,
                                prevMB);
                        }
                        else
                            H264ENC_MAKE_NAME_BS(PutDQUANT)(
                                pBitstream,
                                cur_mb.LocalMacroblockInfo->QP,
                                iLastXmittedQP);
                        prev_dquant = cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP;
                        iLastXmittedQP = cur_mb.LocalMacroblockInfo->QP;
                    } else {
                        // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
                        // core_enc->m_iLastXmittedQP.
                        cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;
                        prev_dquant = 0;
                    }
                }
                else if (mb_type == MBTYPE_PCM) {

                    H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_PCM_MB)(state, curr_slice);

                    // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
                    // core_enc->m_iLastXmittedQP.
                    prev_dquant = 0;
                    cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;

                } else {
                }
            } else { // INTRA
                bool noSubMbPartSizeLessThan8x8Flag = true;
                if ((mb_type == MBTYPE_INTER) || (mb_type == MBTYPE_SKIPPED) || (mb_type == MBTYPE_INTER_8x8) ||
                    (mb_type == MBTYPE_INTER_8x8_REF0) ||
                    (mb_type == MBTYPE_INTER_16x8) || (mb_type == MBTYPE_INTER_8x16)) {

                    // Encode MB type
                    noSubMbPartSizeLessThan8x8Flag = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_Inter_Type)(state, curr_slice);

                } else {    // B Slice
                    noSubMbPartSizeLessThan8x8Flag = H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_BiPred_Type)(state, curr_slice);
                }

                // Encode MB Motion Vectors
                H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_MB_Vectors)(state, curr_slice);

                // Write CBP
                H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_CBP)(state, curr_slice);

                if (uCBP > 0) { // Only Send Delta_QP if there are residuals to follow.
                    if(   core_enc->m_PicParamSet.transform_8x8_mode_flag
//                       && pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo)
                       && (cur_mb.LocalMacroblockInfo->cbp & 0xf)
                       && noSubMbPartSizeLessThan8x8Flag
                       && (mb_type != MBTYPE_DIRECT || core_enc->m_SeqParamSet.direct_8x8_inference_flag)
                       ) {
                        H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_transform_size_8x8_flag)(state, curr_slice);
                    } else {
                        // For correct packing of the successor MB's
                        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 0);
                    }

                    if (core_enc->m_PicParamSet.entropy_coding_mode)
                    {
                        Ipp32s prevMB = (cur_mb.uMB > 0)? curr_slice->m_prev_dquant !=0: 0;
                        H264ENC_MAKE_NAME_BS(DQuant_CABAC)(
                            pBitstream,
                            cur_mb.LocalMacroblockInfo->QP - iLastXmittedQP,
                            prevMB);
                    }
                    else
                        H264ENC_MAKE_NAME_BS(PutDQUANT)(
                            pBitstream,
                            cur_mb.LocalMacroblockInfo->QP,
                            iLastXmittedQP);
                    prev_dquant = cur_mb.LocalMacroblockInfo->QP-iLastXmittedQP;
                    iLastXmittedQP = cur_mb.LocalMacroblockInfo->QP;
                } else {
                    // Set QP correctly for Loop filter, since it is not transmitted, decoder will use
                    // core_enc->m_iLastXmittedQP.
                    prev_dquant = 0;
                    cur_mb.LocalMacroblockInfo->QP = iLastXmittedQP;
                    // For correct packing of the successor MB's
                    pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 0);
                }

            }   // INTER
        }   // not skipped
    }   // not INTRA slice
    return ps;
}   // Put_MBHeader

////////////////////////////////////////////////////////////////////////////////
//
// Encode_CBP
//
////////////////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_Encode_CBP)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;

    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        Ipp32s nBNum;
        Ipp32s nTop, nLeft;
        Ipp32u left_c, top_c;
        Ipp32u mask;

        Ipp32s cbp = cur_mb.LocalMacroblockInfo->cbp;

        for(Ipp32s i = 0; i < 2; i++)
        {
            for(Ipp32s j = 0; j < 2; j++)
            {
                if (!i)
                {
                    // get number of above macroblock
                    nTop = cur_mb.BlockNeighbours.mb_above.mb_num;

                    if (0 <= nTop)
                    {
                        nBNum = cur_mb.BlockNeighbours.mb_above.block_num;

                        top_c = (core_enc->m_mbinfo.mbs[nTop].cbp &
                            (1 << (subblock_block_ss[nBNum + j * 2])) || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nTop].mbtype == MBTYPE_PCM) ? (0) : (1);
                    }
                    else
                        top_c = 0;
                }
                else
                    top_c = (cbp & (1 << j)) ? (0) : (1);

                if (!j)
                {
                    // get number of left macroblock
                    nLeft = cur_mb.BlockNeighbours.mbs_left[i * 2].mb_num;

                    if (0 <= nLeft)
                    {
                        nBNum = cur_mb.BlockNeighbours.mbs_left[i * 2].block_num;
                        left_c = (core_enc->m_mbinfo.mbs[nLeft].cbp &
                                (1 << (subblock_block_ss[nBNum])) || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nLeft].mbtype == MBTYPE_PCM) ? (0) : (1);
                    }
                    else
                        left_c = 0;
                }
                else
                    left_c = (cbp & (1 << (2 * i))) ? (0) : (1);

                mask = (1 << (2 * i + j));

                Ipp32s ctxIdxInc = left_c + 2 * top_c;
                H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                    pBitstream,
                    pBitstream->context_array + ctxIdxOffset[CODED_BLOCK_PATTERN_LUMA] + ctxIdxInc,
                    (cbp&mask) != 0);
            }
        }

        if(cur_mb.chroma_format_idc) {
            // CHROMA part
            nTop = cur_mb.BlockNeighbours.mb_above.mb_num;

            if (0 <= nTop)
            {
                top_c = (core_enc->m_mbinfo.mbs[nTop].cbp > 15 || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nTop].mbtype==MBTYPE_PCM) ? (1) : (0);
            }
            else
                top_c = 0;
            // obtain number of left macroblock
            nLeft = cur_mb.BlockNeighbours.mbs_left[0].mb_num;

            if (0 <= nLeft)
            {
                left_c = (core_enc->m_mbinfo.mbs[nLeft].cbp > 15 || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nLeft].mbtype==MBTYPE_PCM) ? (1) : (0);
            }
            else
                left_c = 0;

            Ipp32s ctxIdxInc = left_c + 2 * top_c;
            H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                pBitstream,
                pBitstream->context_array + ctxIdxOffset[CODED_BLOCK_PATTERN_CHROMA] + ctxIdxInc,
                cbp > 15);

            if (cbp>15)
            {
                if (top_c)
                    top_c = ((core_enc->m_mbinfo.mbs[nTop].cbp >> 4) == 2 || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nTop].mbtype==MBTYPE_PCM) ? (1) : (0);

                if (left_c)
                    left_c = ((core_enc->m_mbinfo.mbs[nLeft].cbp >> 4) == 2 || core_enc->m_pCurrentFrame->m_mbinfo.mbs[nLeft].mbtype==MBTYPE_PCM) ? (1) : (0);

                ctxIdxInc = left_c + 2 * top_c;
                H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                    pBitstream,
                    pBitstream->context_array + ctxIdxOffset[CODED_BLOCK_PATTERN_CHROMA] + ctxIdxInc + 4,
                    ((cbp>>4) == 2));
            }
        }
    } else {
        Ipp32s length;
        MBTypeValue mb_type = cur_mb.GlobalMacroblockInfo->mbtype;
        Ipp32s cbp = cur_mb.LocalMacroblockInfo->cbp;
        if (mb_type == MBTYPE_INTRA)
            cbp = (cur_mb.chroma_format_idc)? enc_cbp_intra[cbp]
                                                   : enc_cbp_intra_monochrome[cbp];
        else
            cbp = (cur_mb.chroma_format_idc)? enc_cbp_inter[cbp]
                                                   : enc_cbp_inter_monochrome[cbp];

        length = H264ENC_MAKE_NAME_BS(PutVLCCode)(pBitstream, cbp);
    }
}   // Encode_CBP


////////////////////////////////////////////////////////////////////////////////
Status H264ENC_MAKE_NAME_BS_DEP(H264CoreEncoder_PackSubBlockLuma)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBlock)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsBase  *pBitstream = curr_slice->m_pbitstream;
    Ipp32u uMaxCoeffs;

    // Figure out the maximum number of coeffs for this block type
    if (cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTRA_16x16)
    {
        uMaxCoeffs = 15;
    } else if(pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo) && core_enc->m_PicParamSet.entropy_coding_mode) {
        uMaxCoeffs = 64; // 8x8 transform
    } else {
        uMaxCoeffs = 16;
    }

    // Write coeff_token and trailing ones signs
    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        if(!pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo)) {
            // coded_block_flag is not coded for 8x8 transform.
            Ipp32s sbx = sb_x[uBlock];
            Ipp32s sby = sb_y[uBlock];
            Ipp32s bit         = 1 + sbx + (sby<<2);

            bool is_intra = IS_TRUEINTRA_MBTYPE(cur_mb.GlobalMacroblockInfo->mbtype);
            Ipp32s upper_bit   = is_intra;
            Ipp32s left_bit    = is_intra;

            if (curr_slice->Block_CABAC[uBlock].uNumSigCoeffs != 0){
                cur_mb.LocalMacroblockInfo->cbp_bits |= (1<<bit);
            }

            //--- get bits from neighbouring blocks ---
            if (!sby){
                if (cur_mb.BlockNeighbours.mb_above.mb_num>=0){
                    Ipp32s iMBAbove = cur_mb.BlockNeighbours.mb_above.mb_num;

                    if (0 <= iMBAbove){
                        upper_bit = BIT_SET(core_enc->m_mbinfo.mbs[iMBAbove].cbp_bits,
                            cur_mb.BlockNeighbours.mb_above.block_num + sbx + 1);
                        // +1 since 0=DC
                    }

                }
            } else {
                upper_bit = BIT_SET(cur_mb.LocalMacroblockInfo->cbp_bits, bit - 4);
            }

            if (!sbx){
                if (cur_mb.BlockNeighbours.mbs_left[block_subblock_mapping[uBlock]/4].mb_num>=0)                {
                    Ipp32s iMBLeft = cur_mb.BlockNeighbours.mbs_left[sby].mb_num;

                    if (0 <= iMBLeft){
                        left_bit = BIT_SET(core_enc->m_mbinfo.mbs[iMBLeft].cbp_bits,
                            cur_mb.BlockNeighbours.mbs_left[sby].block_num + 1);
                        // +1 since 0=DC
                    }
                }
            } else {
                left_bit = BIT_SET(cur_mb.LocalMacroblockInfo->cbp_bits, bit-1);
            }

            Ipp32s CtxInc = 2*upper_bit+left_bit;

            if (uMaxCoeffs > 15)
                H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                    pBitstream,
                    pBitstream->context_array +
                        ctxIdxOffsetFrameCoded[CODED_BLOCK_FLAG] +
                        ctxIdxBlockCatOffset[CODED_BLOCK_FLAG][BLOCK_LUMA_LEVELS] +
                        CtxInc,
                    curr_slice->Block_CABAC[uBlock].uNumSigCoeffs != 0);
            else
                H264ENC_MAKE_NAME_BS(EncodeSingleBin_CABAC)(
                    pBitstream,
                    pBitstream->context_array +
                        ctxIdxOffsetFrameCoded[CODED_BLOCK_FLAG] +
                        ctxIdxBlockCatOffset[CODED_BLOCK_FLAG][BLOCK_LUMA_AC_LEVELS] +
                        CtxInc,
                    curr_slice->Block_CABAC[uBlock].uNumSigCoeffs != 0);
        }else {
            // Set coded_block_flag to 1 for all 4x4 blocks of the 8x8 block.
            Ipp32s bit = 1 + sb_x[uBlock*4] + 4*sb_y[uBlock*4];
            cur_mb.LocalMacroblockInfo->cbp_bits |= (0x33<<bit);
        }

        if (curr_slice->Block_CABAC[uBlock].uNumSigCoeffs) {
            H264ENC_MAKE_NAME_BS(ResidualBlock_CABAC)(
                pBitstream,
                &curr_slice->Block_CABAC[uBlock],
                !curr_slice->m_is_cur_mb_field);
        }
    } else  {
        Ipp32u uCoeffstoWrite;

        // Put coeff_token and trailing ones signs
        // First calculate "N", which is used to select the appropriate VLC Table.
        Ipp32u x = sb_x[uBlock];
        Ipp32u y = sb_y[uBlock];

        Ipp32u N = H264ENC_MAKE_NAME(H264CoreEncoder_GetBlocksLumaContext)(state, cur_mb, x, y);
//#define TRACE_CAVLC 185
        H264ENC_MAKE_NAME_BS(PutNumCoeffAndTrailingOnes)(
            pBitstream,
            N,
            0,  // Chroma DC?
            curr_slice->Block_RLE[uBlock].uNumCoeffs,
            curr_slice->Block_RLE[uBlock].uTrailing_Ones,
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs);
#if defined (TRACE_CAVLC)
        if(uMB == TRACE_CAVLC) {
            printf("uMB = %d, uBlock = %d, NumCoeffs = %d, TOnes = %d, TOSignes = %d\n",
                uMB, uBlock, curr_slice->Block_RLE[uBlock].uNumCoeffs,
                curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                curr_slice->Block_RLE[uBlock].uTrailing_One_Signs);
        }
#endif // TRACE_CAVLC
        if (curr_slice->Block_RLE[uBlock].uNumCoeffs != 0)
        {
            uCoeffstoWrite = curr_slice->Block_RLE[uBlock].uNumCoeffs -
                curr_slice->Block_RLE[uBlock].uTrailing_Ones;

            if (uCoeffstoWrite) {
                H264ENC_MAKE_NAME_BS(PutLevels)(
                    pBitstream,
                    curr_slice->Block_RLE[uBlock].iLevels,
                    uCoeffstoWrite,
                    curr_slice->Block_RLE[uBlock].uTrailing_Ones);
            }
            if (curr_slice->Block_RLE[uBlock].uNumCoeffs != uMaxCoeffs)
            {
                H264ENC_MAKE_NAME_BS(PutTotalZeros)(
                    pBitstream,
                    curr_slice->Block_RLE[uBlock].uTotalZeros,
                    curr_slice->Block_RLE[uBlock].uNumCoeffs,
                    0); // Chroma DC?

                if ((curr_slice->Block_RLE[uBlock].uTotalZeros) &&
                    (curr_slice->Block_RLE[uBlock].uNumCoeffs > 1))
                    H264ENC_MAKE_NAME_BS(PutRuns)(
                        pBitstream,
                        curr_slice->Block_RLE[uBlock].uRuns,
                        curr_slice->Block_RLE[uBlock].uTotalZeros,
                        curr_slice->Block_RLE[uBlock].uNumCoeffs);
            }
        }
    }

    return UMC_OK;

} // PackSubBlockLuma

#undef H264CurrentMacroblockDescriptorType
#undef H264SliceType
#undef H264CoreEncoderType
#undef H264BsType
#undef H264ENC_MAKE_NAME_BS_DEP
#undef H264ENC_MAKE_NAME_BS
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
