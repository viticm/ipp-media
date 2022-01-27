//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

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

#define H264BsBaseType H264ENC_MAKE_NAME(H264BsBase)
#define H264BsRealType H264ENC_MAKE_NAME(H264BsReal)
#define H264BsFakeType H264ENC_MAKE_NAME(H264BsFake)
#define H264CoreEncoderType H264ENC_MAKE_NAME(H264CoreEncoder)
#define H264SliceType H264ENC_MAKE_NAME(H264Slice)
#define H264CurrentMacroblockDescriptorType H264ENC_MAKE_NAME(H264CurrentMacroblockDescriptor)
#define H264EncoderFrameType H264ENC_MAKE_NAME(H264EncoderFrame)
#define T_Block_CABAC_DataType H264ENC_MAKE_NAME(T_Block_CABAC_Data)

//--------------------------------------------------------------------------;
//
//  The algorithm fills in (1) the bottom (not including corners),
//  then (2) the sides (including the bottom corners, but not the
//  top corners), then (3) the top (including the top
//  corners) as shown below, replicating the outermost bytes
//  of the original frame outward:
//
//               ----------------------------
//              |                            |
//              |            (3)             |
//              |                            |
//              |----------------------------|
//              |     |                |     |
//              |     |                |     |
//              |     |                |     |
//              |     |    original    |     |
//              |     |     frame      |     |
//              |     |                |     |
//              | (2) |                | (2) |
//              |     |                |     |
//              |     |                |     |
//              |     |----------------|     |
//              |     |                |     |
//              |     |      (1)       |     |
//              |     |                |     |
//               ----------------------------
//
//--------------------------------------------------------------------------;
void H264ENC_MAKE_NAME(ExpandPlane)(
    PIXTYPE* StartPtr,
    Ipp32s   frameWidth,
    Ipp32s   frameHeight,
    Ipp32s   pitchPixels,
    Ipp32s   pels)
{
    Ipp32s   row, col;
    PIXTYPE  uLeftFillVal;
    PIXTYPE  uRightFillVal;
    PIXTYPE* pByteSrc;

    PIXTYPE *pSrc = StartPtr + (frameHeight - 1)*pitchPixels;
    PIXTYPE *pDst = pSrc + pitchPixels;
    // section 1 at bottom
    // obtain pointer to start of bottom row of original frame
    for (row=0; row < pels; row++, pDst += pitchPixels) {
        memcpy(pDst, pSrc, frameWidth*sizeof(PIXTYPE));
    }

    // section 2 on left and right
    // obtain pointer to start of first row of original frame
    pByteSrc = StartPtr;
    for (row=0; row<(frameHeight + pels); row++, pByteSrc += pitchPixels)
    {
        // get fill values from left and right columns of original frame
        uLeftFillVal = *pByteSrc;
        uRightFillVal = *(pByteSrc + frameWidth - 1);

        // fill all bytes on both edges
        for (col=0; col<pels; col++)
        {
            *(pByteSrc - pels + col) = uLeftFillVal;
            *(pByteSrc + frameWidth + col) = uRightFillVal;
        }
    }

    // section 3 at top
    // obtain pointer to top row of original frame, less expand pels
    pSrc = StartPtr - pels;
    pDst = pSrc - pitchPixels;
    for (row=0; row<pels; row++, pDst -= pitchPixels) {
       memcpy(pDst, pSrc, sizeof(PIXTYPE)*(frameWidth + pels + pels));
    }
}   // end ExpandPlane

void H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u *puAIMBSAD,
    PIXTYPE *pPredBuf)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    T_AIMode BestMode;
    H264CurrentMacroblockDescriptorType& cur_mb = curr_slice->m_cur_mb;
    Ipp32u uMB = cur_mb.uMB;

    cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTRA_16x16;
    if( core_enc->m_Analyse & ANALYSE_RD_OPT ){
        *puAIMBSAD = H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectRD)(
            state,
            curr_slice,
            cur_mb.mbPtr,
            core_enc->m_pReconstructFrame->m_pYPlane +
                core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field],
            cur_mb.mbPitchPixels,
            &BestMode,
            pPredBuf);
    }else{
        *puAIMBSAD = H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneMB_16x16)(
            state,
            curr_slice,
            cur_mb.mbPtr,
            core_enc->m_pReconstructFrame->m_pYPlane +
                core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field],
            cur_mb.mbPitchPixels,
            &BestMode,
            pPredBuf);
    }
    curr_slice->m_cur_mb.LocalMacroblockInfo->intra_16x16_mode = BestMode;   // Selected mode is stored in block 0
//    if ((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)) {
    if (!(core_enc->m_Analyse & ANALYSE_RD_OPT) && (core_enc->m_Analyse & ANALYSE_RD_MODE)) {
        H264BsBase *pBitstream = curr_slice->m_pbitstream;
        H264ENC_MAKE_NAME(H264BsFake_Reset)(curr_slice->fakeBitstream);
        //H264ENC_MAKE_NAME(H264BsFake_CopyContext_CABAC)(curr_slice->fakeBitstream, pBitstream, !curr_slice->m_is_cur_mb_field, 0);
        H264BsBase_CopyContextCABAC_I16x16(&curr_slice->fakeBitstream->m_base, pBitstream, !curr_slice->m_is_cur_mb_field);
        curr_slice->m_pbitstream = (H264BsBase *)curr_slice->fakeBitstream;
        //H264MacroblockLocalInfo sLocalMBinfo = *cur_mb.LocalMacroblockInfo;
        //H264MacroblockGlobalInfo sGlobalMBinfo = *cur_mb.GlobalMacroblockInfo;
        pSetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo, false);
        cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
        cur_mb.LocalMacroblockInfo->cbp = cur_mb.LocalMacroblockInfo->cbp_bits = 0;
        H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantIntra16x16_RD)(state, curr_slice);
        H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBHeader_Fake)(state, curr_slice);
        H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBLuma_Fake)(state, curr_slice);
        Ipp32s bs = H264ENC_MAKE_NAME(H264BsFake_GetBsOffset)(curr_slice->fakeBitstream);
        //Ipp32s d = SSD16x16(cur_mb.mbPtr, cur_mb.mbPitchPixels, core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field], cur_mb.mbPitchPixels)<<5;
        Ipp32s d = SSD16x16(cur_mb.mbPtr, cur_mb.mbPitchPixels, cur_mb.mb16x16.reconstruct, 16)<<5;
        *puAIMBSAD = d + cur_mb.lambda * bs;
        curr_slice->m_pbitstream = pBitstream;
        //*cur_mb.LocalMacroblockInfo = sLocalMBinfo;
        //*cur_mb.GlobalMacroblockInfo = sGlobalMBinfo;
    }
}

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_MB_Decision)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s uMB)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u best_sad, uAIMBSAD_16x16, uAIMBSAD_4x4, uAIMBSAD_8x8, uBestIntraSAD;
    MB_Type uBestIntraMBType;
    bool bIntra8x8 = false;
    T_AIMode intra_types_save[16];
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s MBHasEdges;
    Ipp8u  uMBQP = getLumaQP51(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.bit_depth_luma);
    Ipp32u uBestInterSAD;

    best_sad = uAIMBSAD_4x4 = uAIMBSAD_8x8 = uBestIntraSAD = MAX_SAD>>1;
    cur_mb.LocalMacroblockInfo->cbp_bits =
    cur_mb.LocalMacroblockInfo->cbp_bits_chroma =
    cur_mb.LocalMacroblockInfo->cbp =
    cur_mb.LocalMacroblockInfo->intra_chroma_mode = 0;
    cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
    cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
    cur_mb.LocalMacroblockInfo->cost = MAX_SAD;
    cur_mb.m_uIntraCBP4x4 = 0xffff;
    cur_mb.m_uIntraCBP8x8 = 0xffff;

    if (curr_slice->m_slice_type != INTRASLICE)  {
#ifndef H264_COMMON_ME
        if (core_enc->m_Analyse & ANALYSE_INTRA_IN_ME) {
            best_sad = (BPREDSLICE == curr_slice->m_slice_type) ? H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(state, curr_slice) : H264ENC_MAKE_NAME(H264CoreEncoder_ME_P)(state, curr_slice);
            if (core_enc->m_PicParamSet.chroma_format_idc == 0)
                cur_mb.LocalMacroblockInfo->cbp_chroma = 0;
            cur_mb.LocalMacroblockInfo->cost = best_sad;
            return best_sad;
        } else
            uBestInterSAD = (BPREDSLICE == curr_slice->m_slice_type) ? H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(state, curr_slice) : H264ENC_MAKE_NAME(H264CoreEncoder_ME_P)(state, curr_slice);

#else //H264_COMMON_ME
        if (BPREDSLICE == curr_slice->m_slice_type)
            uBestInterSAD = H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(state, curr_slice);
        else if (PREDSLICE == curr_slice->m_slice_type)
        {
//================== common ME (EstimateFrame() for 1 MB) =====================
            Status err;
            Ipp32s ref_idx;
            Ipp32s numRef = curr_slice->m_NumRefsInL0List;

            for (ref_idx = 0; ref_idx < numRef; ref_idx ++)
            {
                assert(core_enc->m_pME!=NULL);

                if(core_enc->m_pME == NULL)
                    return UMC::UMC_ERR_NULL_PTR;

                Ipp32s QP = cur_mb.LocalMacroblockInfo->QP;
                double Qstep;
                double mult;
                switch (QP%6)
                {
                case 0:
                    mult = 0.625;
                    break;
                case 1:
                    mult = 0.6875;
                    break;
                case 2:
                    mult = 0.8125;
                    break;
                case 3:
                    mult = 0.875;
                    break;
                case 4:
                    mult = 1;
                    break;
                case 5:
                    mult = 1.125;
                    break;
                }
                Qstep = mult * (1 << QP/6);
                core_enc->MEParams->Quant = Qstep;

                core_enc->MEParams->pRefF[0] = core_enc->MeFrameRefList[ref_idx];
                core_enc->MEParams->pRefF[0]->ptr[0] = core_enc->ppRefPicList[ref_idx]->m_pYPlane;
                core_enc->MEParams->pRefF[0]->step[0] = core_enc->m_pCurrentFrame->m_pitchPixels;

                core_enc->MEParams->FirstMB = cur_mb.uMB;
                core_enc->MEParams->LastMB = cur_mb.uMB;

                core_enc->MEParams->Interpolation = ME_H264_Luma;

                if(!(core_enc->m_pME->EstimateFrame(core_enc->MEParams)))
                        return UMC::UMC_ERR_INVALID_PARAMS;
                for (Ipp32s i = 0; i< 16; i++)
                    curr_slice->m_cur_mb.RefIdxs[LIST_0]->RefIdxs[i] = 0; //estimation only for 1 ref frame as temporal measure

                if (core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MbPart == ME_Mb16x16)
                {
                    if (core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MbType == ME_MbFrwSkipped)
                    {
                        /*H264MotionVector skip_vec;
                        Skip_MV_Predicted(curr_slice, cur_mb.uMB, &skip_vec);
                        for (Ipp32s i = 0; i < 16; i ++)
                        {
                            cur_mb.MVs[LIST_0]->MotionVectors[i] = skip_vec;
                            cur_mb.MVs[LIST_1]->MotionVectors[i] = null_mv;
                            cur_mb.RefIdxs[LIST_0]->RefIdxs[i] = (T_RefIdx)0;
                            cur_mb.RefIdxs[LIST_1]->RefIdxs[i] = (T_RefIdx)-1;
                        }
                        core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][0].x = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][1].x = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][2].x = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][3].x = skip_vec.mvx;
                        core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][0].y = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][1].y = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][2].y = core_enc->m_pME->m_ResMB[cur_mb.uMB].MV[0][3].y = skip_vec.mvy;*/
                        for (Ipp32s i = 0; i< 16; i++)
                        {
                            (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[i].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][0].x;
                            (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[i].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][0].y;
                            cur_mb.RefIdxs[LIST_0]->RefIdxs[i] = (T_RefIdx)0;
                        }
                        curr_slice->m_cur_mb.LocalMacroblockInfo->cbp_luma = 0x000000;
                        curr_slice->m_cur_mb.LocalMacroblockInfo->cbp_chroma = 0;
                        curr_slice->m_cur_mb.LocalMacroblockInfo->cbp = 0;
                        curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_SKIPPED;
                        best_sad = 0;
                    }
                    else if (core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MbType == ME_MbFrw)
                    {
                        curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTER;
                        uBestInterSAD = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MbCosts[0];
                        best_sad = uBestInterSAD;
                        for (Ipp32s i = 0; i< 16; i++)
                        {
                            (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[i].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][0].x;
                            (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[i].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][0].y;
                        }
                    }
                    else
                    {
                        H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, uMB, &uAIMBSAD_16x16, cur_mb.mb16x16.prediction);
                        uBestIntraSAD = uAIMBSAD_16x16;
                        uBestIntraMBType = MBTYPE_INTRA_16x16;
                        // FAST intra decision
                        if ((core_enc->m_Analyse & ANALYSE_I_4x4) || (core_enc->m_Analyse & ANALYSE_I_8x8)) {
                            // Use edge detection to determine if the MB is a flat region
                            H264ENC_MAKE_NAME(ippiEdgesDetect16x16)(cur_mb.mbPtr, cur_mb.mbPitchPixels, EdgePelDiffTable[uMBQP], EdgePelCountTable[uMBQP], &MBHasEdges);
                            if (MBHasEdges) {
                                if (core_enc->m_Analyse & ANALYSE_I_8x8) {
                                    pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, true);
                                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock8x8)(
                                        state,
                                        curr_slice,
                                        uMB,
                                        uBestIntraSAD,
                                        &uAIMBSAD_8x8);
                                    //Save intra_types
                                    memcpy(intra_types_save, curr_slice->m_cur_mb.intra_types, 16*sizeof(T_AIMode));
                                    if (uAIMBSAD_8x8 < uBestIntraSAD) {
                                        uBestIntraSAD = uAIMBSAD_8x8;
                                        uBestIntraMBType = MBTYPE_INTRA;
                                        bIntra8x8 = true;
                                    }
                                    pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, false);
                                }
                                if (core_enc->m_Analyse & ANALYSE_I_4x4) {
                                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uMB, uBestIntraSAD, &uAIMBSAD_4x4);
                                    if (uAIMBSAD_4x4 < uBestIntraSAD) {
                                        uBestIntraSAD = uAIMBSAD_4x4;
                                        uBestIntraMBType = MBTYPE_INTRA;
                                        bIntra8x8 = false;
                                    }
                                }
                            }
                        }
                        if (!((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)) && (core_enc->m_Analyse & ANALYSE_ME_CHROMA) && core_enc->m_PicParamSet.chroma_format_idc != 0) {
                            Ipp8u mode;
                            Ipp32s  uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
                            PIXTYPE* pPredBuf = cur_mb.mbChromaIntra.prediction;
                            //Get intra chroma prediction
                            uBestIntraSAD += H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectChromaMBs_8x8)(
                                state,
                                curr_slice,
                                core_enc->m_pCurrentFrame->m_pUPlane + uOffset,
                                core_enc->m_pReconstructFrame->m_pUPlane + uOffset,
                                core_enc->m_pCurrentFrame->m_pVPlane + uOffset,
                                core_enc->m_pReconstructFrame->m_pVPlane + uOffset,
                                cur_mb.mbPitchPixels,
                                &mode,
                                pPredBuf,
                                pPredBuf + 8);
                        }
                        cur_mb.GlobalMacroblockInfo->mbtype = static_cast<MBTypeValue>(uBestIntraMBType);
                        pSetMB8x8TSPackFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, bIntra8x8);
                        if (uBestIntraMBType == MBTYPE_INTRA && !bIntra8x8)
                            cur_mb.LocalMacroblockInfo->cbp_luma = curr_slice->m_uIntraCBP4x4;
                        else if (uBestIntraMBType == MBTYPE_INTRA && bIntra8x8) {
                            cur_mb.LocalMacroblockInfo->cbp_luma = curr_slice->m_uIntraCBP8x8;
                            //Restore intra_types
                            memcpy( curr_slice->m_cur_mb.intra_types, intra_types_save, 16*sizeof(T_AIMode));
                        } else
                            cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
                        cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
                        curr_slice->m_Intra_MB_Counter++;
                        best_sad = uBestIntraSAD;
                    }
                }
                else // 8x8 mode TODO: write correct data taking
                {
                    Ipp8u Mapping[4] = {0,2,8,10};
                    curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTER_8x8;
                    uBestInterSAD = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MbCosts[0];
                    best_sad = uBestInterSAD;
                    for (Ipp32s i = 0; i < 4; i++)
                    {
                        Ipp32s offset = Mapping[i];
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].x;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].y;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 1].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].x;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 1].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].y;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 4].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].x;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 4].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].y;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 5].mvx = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].x;
                        (curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors)[offset + 5].mvy = core_enc->MEParams->pSrc->MBs[cur_mb.uMB].MV[0][i].y;

                    }
                }
            }
        }


//============================================================================

#endif // H264_COMMON_ME

#if !defined(H264_COMMON_ME)
        if (!(core_enc->m_Analyse & ANALYSE_INTRA_IN_ME)) {
            // MB type classification. Only need to change anything if INTRA is chosen, as the ME function filled in MB info for best INTER mode and cbp's.
            // save INTER results for possible later switch back to INTER
            MBTypeValue MBTypeInter = cur_mb.GlobalMacroblockInfo->mbtype;
            Ipp32u uInterCBP4x4 = cur_mb.LocalMacroblockInfo->cbp_luma;
            Ipp32s InterMB8x8PackFlag = pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo);

            Ipp32s mb_type = cur_mb.GlobalMacroblockInfo->mbtype;
            if (mb_type != MBTYPE_SKIPPED)
            if ((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE) || !(uBestInterSAD < (Ipp32u)EmptyThreshold[uMBQP])) {
          //if (((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)) || (!((mb_type == MBTYPE_DIRECT) || (mb_type == MBTYPE_SKIPPED) || (uBestInterSAD < EmptyThreshold[uMBQP])))) {
                Ipp32s cost_chroma=0;
                Ipp32u chroma_cbp = cur_mb.LocalMacroblockInfo->cbp_chroma;
                if ((core_enc->m_Analyse & ANALYSE_ME_CHROMA) && core_enc->m_PicParamSet.chroma_format_idc != 0) {
                    Ipp32s uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
                    if((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)){
                        pSetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo, false);
                        cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTRA; //make INTRA prediction - the same for I16x16, I8x8, I4x4
                        cost_chroma = H264ENC_MAKE_NAME(H264CoreEncoder_IntraSelectChromaRD)(
                            state,
                            curr_slice,
                            core_enc->m_pCurrentFrame->m_pUPlane + uOffset,
                            core_enc->m_pReconstructFrame->m_pUPlane + uOffset,
                            core_enc->m_pCurrentFrame->m_pVPlane + uOffset,
                            core_enc->m_pReconstructFrame->m_pVPlane + uOffset,
                            cur_mb.mbPitchPixels,
                            &cur_mb.LocalMacroblockInfo->intra_chroma_mode,
                            cur_mb.mbChromaIntra.prediction,
                            cur_mb.mbChromaIntra.prediction+8);
                    }else{
                        Ipp8u mode;
                        PIXTYPE* pPredBuf = cur_mb.mbChromaIntra.prediction;
                        //Get intra chroma prediction
                        cost_chroma = H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectChromaMBs_8x8)(
                            state,
                            curr_slice,
                            core_enc->m_pCurrentFrame->m_pUPlane + uOffset,
                            core_enc->m_pReconstructFrame->m_pUPlane + uOffset,
                            core_enc->m_pCurrentFrame->m_pVPlane + uOffset,
                            core_enc->m_pReconstructFrame->m_pVPlane + uOffset,
                            cur_mb.mbPitchPixels,
                            &mode,
                            pPredBuf,
                            pPredBuf + 8);
                    }
                }
                H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, cur_mb.mb16x16.prediction);
                uBestIntraSAD = uAIMBSAD_16x16;
                uBestIntraMBType = MBTYPE_INTRA_16x16;
                // FAST intra decision
                //f if (uAIMBSAD_16x16+cost_chroma <= 2 * uBestInterSAD && ((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE) || (curr_slice->m_uMBInterSAD >= rd_quant_intra_min[uMBQP]))) {
                if (uAIMBSAD_16x16+cost_chroma <= (uBestInterSAD * 5 >> 2)) {
                    if ((core_enc->m_Analyse & ANALYSE_I_4x4) || (core_enc->m_Analyse & ANALYSE_I_8x8)) {
                        // Use edge detection to determine if the MB is a flat region
                        H264ENC_MAKE_NAME(ippiEdgesDetect16x16)(cur_mb.mbPtr, cur_mb.mbPitchPixels, EdgePelDiffTable[uMBQP], EdgePelCountTable[uMBQP], &MBHasEdges);
                        if (MBHasEdges) {
                            
                            if (core_enc->m_Analyse & ANALYSE_I_8x8) {
                                pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, true);
                                H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock8x8)(state, curr_slice, uBestIntraSAD, &uAIMBSAD_8x8);
                                //Save intra_types
                                memcpy(intra_types_save, curr_slice->m_cur_mb.intra_types, 16 * sizeof(T_AIMode));
                                if (uAIMBSAD_8x8 < uBestIntraSAD) {
                                    uBestIntraSAD = uAIMBSAD_8x8;
                                    uBestIntraMBType = MBTYPE_INTRA;
                                    bIntra8x8 = true;
                                }
                                pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, false);
                                if ((core_enc->m_Analyse & ANALYSE_I_4x4) && (uAIMBSAD_8x8 <= (uAIMBSAD_16x16 * 5 >> 2))) {
                                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uBestIntraSAD, &uAIMBSAD_4x4);
                                    if (uAIMBSAD_4x4 < uBestIntraSAD) {
                                        uBestIntraSAD = uAIMBSAD_4x4;
                                        uBestIntraMBType = MBTYPE_INTRA;
                                        bIntra8x8 = false;
                                    }
                                }
                            } else if (core_enc->m_Analyse & ANALYSE_I_4x4) {
                                H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uBestIntraSAD, &uAIMBSAD_4x4);
                                if (uAIMBSAD_4x4 < uBestIntraSAD) {
                                    uBestIntraSAD = uAIMBSAD_4x4;
                                    uBestIntraMBType = MBTYPE_INTRA;
                                    bIntra8x8 = false;
                                }
                            }
                            /*
                            if (core_enc->m_Analyse & ANALYSE_I_4x4) {
                                H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uMB, uBestIntraSAD, &uAIMBSAD_4x4);
                                if (uAIMBSAD_4x4 < uBestIntraSAD) {
                                    uBestIntraSAD = uAIMBSAD_4x4;
                                    uBestIntraMBType = MBTYPE_INTRA;
                                    bIntra8x8 = false;
                                }
                            }
                            //Save intra_types
                            memcpy(intra_types_save, curr_slice->m_cur_mb.intra_types, 16 * sizeof(T_AIMode));
                            if ((core_enc->m_Analyse & ANALYSE_I_8x8) && (uAIMBSAD_4x4 <= (uAIMBSAD_16x16 * 9 >> 3))) {
                                pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, true);
                                H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock8x8)(state, curr_slice, uMB, uBestIntraSAD, &uAIMBSAD_8x8);
                                if (uAIMBSAD_8x8 < uBestIntraSAD) {
                                    uBestIntraSAD = uAIMBSAD_8x8;
                                    uBestIntraMBType = MBTYPE_INTRA;
                                    bIntra8x8 = true;
                                }
                                pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, false);
                            }
                            */
                        }
                    }
                }
                cur_mb.LocalMacroblockInfo->cbp_chroma = chroma_cbp;
                uBestIntraSAD += cost_chroma;
                if (!((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)) && (curr_slice->m_slice_type == BPREDSLICE))
                    uBestIntraSAD += BITS_COST(9, glob_RDQM[uMBQP]);
                if (uBestIntraSAD < uBestInterSAD) {
                    cur_mb.GlobalMacroblockInfo->mbtype = static_cast<MBTypeValue>(uBestIntraMBType);
                    pSetMB8x8TSPackFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, bIntra8x8);
                    if (uBestIntraMBType == MBTYPE_INTRA && !bIntra8x8) {
                        cur_mb.LocalMacroblockInfo->cbp_luma = cur_mb.m_uIntraCBP4x4;
                        //Restore intra_types
//                        memcpy(curr_slice->m_cur_mb.intra_types, intra_types_save, 16*sizeof(T_AIMode));
                    } else if (uBestIntraMBType == MBTYPE_INTRA && bIntra8x8) {
                        cur_mb.LocalMacroblockInfo->cbp_luma = cur_mb.m_uIntraCBP8x8;
                        //Restore intra_types
                        memcpy(curr_slice->m_cur_mb.intra_types, intra_types_save, 16*sizeof(T_AIMode));
                    } else
                        cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
                    cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
                    curr_slice->m_Intra_MB_Counter++;
                    best_sad = uBestIntraSAD;
                } else {
                    cur_mb.LocalMacroblockInfo->intra_chroma_mode = 0; //Needed for packing
                    cur_mb.GlobalMacroblockInfo->mbtype = MBTypeInter;
                    cur_mb.LocalMacroblockInfo->cbp_luma = uInterCBP4x4;
                    best_sad = uBestInterSAD;
                    pSetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo, InterMB8x8PackFlag);
                }
            } else {
                best_sad = uBestInterSAD;
            }
        }
#endif //!defined(H264_COMMON_ME)
    } else { // intra slice
        //Disable chroma coding in RD, no side effect in non RD mode
        Ipp32s tmp_chroma_format_idc = cur_mb.chroma_format_idc;
        if (((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE)) && core_enc->m_PicParamSet.chroma_format_idc != 0) {
            cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_INTRA; //make INTRA prediction - the same for I16x16, I8x8, I4x4
            Ipp32s uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
            H264ENC_MAKE_NAME(H264CoreEncoder_IntraSelectChromaRD)(
                state,
                curr_slice,
                core_enc->m_pCurrentFrame->m_pUPlane + uOffset,
                core_enc->m_pReconstructFrame->m_pUPlane + uOffset,
                core_enc->m_pCurrentFrame->m_pVPlane + uOffset,
                core_enc->m_pReconstructFrame->m_pVPlane + uOffset,
                cur_mb.mbPitchPixels,
                &cur_mb.LocalMacroblockInfo->intra_chroma_mode,
                cur_mb.mbChromaIntra.prediction,
                cur_mb.mbChromaIntra.prediction + 8);
            cur_mb.chroma_format_idc = 0;
        }
        H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, cur_mb.mb16x16.prediction);
        uBestIntraMBType = MBTYPE_INTRA_16x16;
        best_sad = uAIMBSAD_16x16;
        if ((core_enc->m_Analyse & ANALYSE_I_4x4) || (core_enc->m_Analyse & ANALYSE_I_8x8)) {
            // Use edge detection to determine if the MB is a flat region
            H264ENC_MAKE_NAME(ippiEdgesDetect16x16)(cur_mb.mbPtr, cur_mb.mbPitchPixels, EdgePelDiffTable[uMBQP], EdgePelCountTable[uMBQP], &MBHasEdges);
            if (MBHasEdges) {
                if (core_enc->m_Analyse & ANALYSE_I_8x8) {
                    pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, true);
                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock8x8)(state, curr_slice, best_sad, &uAIMBSAD_8x8);
                    //Save intra_types
                    memcpy( intra_types_save, curr_slice->m_cur_mb.intra_types, 16*sizeof(T_AIMode));
                    if( uAIMBSAD_8x8 < best_sad ){
                        best_sad = uAIMBSAD_8x8;
                        uBestIntraMBType = MBTYPE_INTRA;
                        bIntra8x8 = true;
                    }
                    pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, false);
                }
                if (core_enc->m_Analyse & ANALYSE_I_4x4) {
                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, best_sad, &uAIMBSAD_4x4);
                    if (uAIMBSAD_4x4 < best_sad) {
                        uBestIntraMBType = MBTYPE_INTRA;
                        best_sad = uAIMBSAD_4x4;
                        bIntra8x8 = false;
                    }
                }
            }
        }

        cur_mb.chroma_format_idc = tmp_chroma_format_idc;
        cur_mb.GlobalMacroblockInfo->mbtype = static_cast<MBTypeValue>(uBestIntraMBType);
        pSetMB8x8TSPackFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, bIntra8x8);
        if (uBestIntraMBType == MBTYPE_INTRA && !bIntra8x8)
            cur_mb.LocalMacroblockInfo->cbp_luma = cur_mb.m_uIntraCBP4x4;
        else if (uBestIntraMBType == MBTYPE_INTRA && bIntra8x8) {
            cur_mb.LocalMacroblockInfo->cbp_luma = cur_mb.m_uIntraCBP8x8;
            //Restore intra_types
            memcpy(curr_slice->m_cur_mb.intra_types, intra_types_save, 16*sizeof(T_AIMode));
        } else
            cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
        cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
    }
    if (core_enc->m_PicParamSet.chroma_format_idc == 0) {
        // Monochrome
        cur_mb.LocalMacroblockInfo->cbp_chroma = 0; // Clear chroma blocks.
    }

    cur_mb.LocalMacroblockInfo->cost =  best_sad;

    return best_sad;
}

void H264ENC_MAKE_NAME(H264CoreEncoder_ReconstuctCBP)(
    H264CurrentMacroblockDescriptorType *cur_mb)
{
    const Ipp8u ICBPTAB[6] = {0,16,32,15,31,47};
    if (cur_mb->GlobalMacroblockInfo->mbtype == MBTYPE_INTRA_16x16){
        Ipp32s N = CALC_16x16_INTRA_MB_TYPE(INTRASLICE,
                                cur_mb->LocalMacroblockInfo->intra_16x16_mode,
                                cur_mb->MacroblockCoeffsInfo->chromaNC,
                                cur_mb->MacroblockCoeffsInfo->lumaAC)-1;
        cur_mb->LocalMacroblockInfo->cbp = ICBPTAB[N>>2];
    }
}

//////////////////////////////////
// Compress one slice
Status H264ENC_MAKE_NAME(H264CoreEncoder_Compress_Slice)(
    void* state,
    H264SliceType *curr_slice,
    bool is_first_mb)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    H264BsRealType* pBitstream = (H264BsRealType *)curr_slice->m_pbitstream;
    Ipp32s slice_num = curr_slice->m_slice_number;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    Ipp8u uUsePCM = 0;

    Ipp8u *pStartBits;
    Ipp32u uStartBitOffset;

    Ipp32u uRecompressMB;
    Ipp8u  iLastQP;
    Ipp32u uSaved_Skip_Run;

    Ipp8u bSeenFirstMB = false;

    Status status = UMC_OK;

    Ipp32u uNumMBs = core_enc->m_HeightInMBs * core_enc->m_WidthInMBs;
    Ipp32u uFirstMB = core_enc->m_field_index * uNumMBs;

    Ipp32s MBYAdjust = 0;
    if (core_enc->m_field_index)
    {
        MBYAdjust  = core_enc->m_HeightInMBs;
    }

    curr_slice->m_InitialOffset = core_enc->m_InitialOffsets[core_enc->m_pCurrentFrame->m_bottom_field_flag[core_enc->m_field_index]];
    curr_slice->m_is_cur_mb_field = core_enc->m_pCurrentFrame->m_PictureStructureForDec < FRM_STRUCTURE;
    curr_slice->m_is_cur_mb_bottom_field = core_enc->m_pCurrentFrame->m_bottom_field_flag[core_enc->m_field_index] == 1;

    //    curr_slice->m_use_transform_for_intra_decision = core_enc->m_info.use_transform_for_intra_decision ?
    //            (curr_slice->m_slice_type == INTRASLICE) : false;
    curr_slice->m_use_transform_for_intra_decision = 1;

#ifdef H264_COMMON_ME
    //================ common ME (Initialization of vars for frame estimation) ===
    if (curr_slice->m_slice_type == PREDSLICE)
    {
        Ipp32s ref_idx;
        Ipp32s numRef = curr_slice->m_NumRefsInL0List;
        core_enc->MeFrameRefList = new MeFrame*[numRef];
        core_enc->MEParams->pRefF/* = new MeFrame**/; // only MEParams->pRefF[0] is used in EstimateFrame() function


        core_enc->ppRefPicList = (H264EncoderFrameType **)(H264ENC_MAKE_NAME(GetRefPicList)(
            curr_slice,
            LIST_0,
            curr_slice->m_is_cur_mb_field,
            cur_mb.uMB & 1)->m_RefPicList);

        core_enc->pFields = H264ENC_MAKE_NAME(GetRefPicList)(
            curr_slice,
            LIST_0,
            curr_slice->m_is_cur_mb_field,
            cur_mb.uMB & 1)->m_Prediction;

        core_enc->MEParams->pSrc = core_enc->m_pME->LockFrame();
        core_enc->MEParams->pSrc->ptr[0] = (Ipp8u*)core_enc->m_pCurrentFrame->m_pYPlane;
        core_enc->MEParams->pSrc->step[0] = core_enc->m_pCurrentFrame->m_pitchPixels;
        core_enc->MEParams->pSrc->type = core_enc->m_pCurrentFrame->m_PicCodType == PREDPIC ? ME_FrmFrw : core_enc->m_pCurrentFrame->m_PicCodType == BPREDPIC ? ME_FrmBidir : ME_FrmIntra; //TODO: change choice

        core_enc->MEParams->SearchDirection    =  ME_ForwardSearch;
        core_enc->MEParams->MbPart             =  ME_Mb16x16; // | ME_Mb8x8;

        //------------------- set search speed ---------------------------------------
        core_enc->MEParams->UseMvPrediction = 1;
        core_enc->MEParams->UseLowThresholdForMvPrediction = 0;
        core_enc->MEParams->UseDownSampledImageForSearch = 0;
        core_enc->MEParams->UseFastIntraInterDecision = 0;
        //----------------------------------------------------------------------------

        core_enc->MEParams->PicRange.top_left.x       = 0;
        core_enc->MEParams->PicRange.top_left.y       = 0;
        core_enc->MEParams->PicRange.bottom_right.x   = core_enc->m_info.info.clip_info.width;
        core_enc->MEParams->PicRange.bottom_right.y   = core_enc->m_info.info.clip_info.height;
        core_enc->MEParams->SearchRange.x             = core_enc->m_info.me_search_x;
        core_enc->MEParams->SearchRange.y             = core_enc->m_info.me_search_y;
        core_enc->MEParams->SkippedMetrics  = ME_Sad; // was ME_MAXAD
        core_enc->MEParams->ProcessSkipped = true;
        core_enc->MEParams->ProcessDirect = false;

        core_enc->MEParams->PredictionType     = ME_H264;

        for (ref_idx = 0; ref_idx < numRef; ref_idx ++)
        {
            core_enc->MeFrameRefList[ref_idx] = core_enc->m_pME->LockFrame();
        }
    }
    //============================================================================
#endif


    // loop over all MBs in the picture
    for (Ipp32u uMB = uFirstMB; uMB < uFirstMB + uNumMBs; uMB++)
    {
        // Is this MB in the current slice?  If not, move on...
        if (core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB].slice_id != slice_num) {
            continue;
        } else if (!bSeenFirstMB) {
            // Reset xpos and ypos in framedata struct
            // This is necessary because the same slice may be recoded multiple times.

            // reset intra MB counter per slice
            curr_slice->m_Intra_MB_Counter = 0;
            curr_slice->m_MB_Counter = 0;

            // Fill in the first mb in slice field in the slice header.
            curr_slice->m_first_mb_in_slice = is_first_mb ? 0 : uMB - uFirstMB;

            // Fill in the current deblocking filter parameters.
            curr_slice->m_slice_alpha_c0_offset = (Ipp8s)core_enc->m_info.deblocking_filter_alpha;
            curr_slice->m_slice_beta_offset = (Ipp8s)core_enc->m_info.deblocking_filter_beta;
            curr_slice->m_disable_deblocking_filter_idc =  core_enc->m_info.deblocking_filter_idc;
            curr_slice->m_cabac_init_idc = core_enc->m_info.cabac_init_idc;

            // Write a slice header
            H264ENC_MAKE_NAME(H264BsReal_PutSliceHeader)(
                pBitstream,
                core_enc->m_SliceHeader,
                core_enc->m_PicParamSet,
                core_enc->m_SeqParamSet,
                core_enc->m_PicClass,
                curr_slice);
            bSeenFirstMB = true;

            // Fill in the correct value for m_iLastXmittedQP, used to correctly code
            // the per MB QP Delta
            curr_slice->m_iLastXmittedQP = core_enc->m_PicParamSet.pic_init_qp + curr_slice->m_slice_qp_delta;
            Ipp32s SliceQPy = curr_slice->m_iLastXmittedQP;

            if (core_enc->m_info.entropy_coding_mode)
            {
                if (slice_type==INTRASLICE)
                    H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesIntra_CABAC)(
                        pBitstream,
                        SliceQPy);
                else
                    H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesInter_CABAC)(
                        pBitstream,
                        SliceQPy,
                        curr_slice->m_cabac_init_idc);
            }

            // Initialize the MB skip run counter
            curr_slice->m_uSkipRun = 0;
        }

        //        cur_mb.lambda = int( (pow(2.0,MAX(0,(curr_slice->m_iLastXmittedQP-12))/3.0)*0.85) + 0.5);
        cur_mb.lambda = lambda_sq[curr_slice->m_iLastXmittedQP];
        cur_mb.uMB = uMB;
        cur_mb.chroma_format_idc = core_enc->m_PicParamSet.chroma_format_idc;
        cur_mb.mbPtr = core_enc->m_pCurrentFrame->m_pYPlane + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field];
        cur_mb.mbPitchPixels =  core_enc->m_pCurrentFrame->m_pitchPixels << curr_slice->m_is_cur_mb_field;
        cur_mb.uMBx = uMB % core_enc->m_WidthInMBs;
        cur_mb.uMBy = uMB / core_enc->m_WidthInMBs - MBYAdjust;
        H264ENC_MAKE_NAME(H264CoreEncoder_UpdateCurrentMBInfo)(state, curr_slice);
        cur_mb.lumaQP = getLumaQP(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.bit_depth_luma);
        cur_mb.lumaQP51 = getLumaQP51(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.bit_depth_luma);
        cur_mb.chromaQP = getChromaQP(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.chroma_qp_index_offset, core_enc->m_SeqParamSet.bit_depth_chroma);
        pSetMB8x8TSFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo, 0);
        curr_slice->m_MB_Counter++;
        H264BsBase_GetState(&pBitstream->m_base, &pStartBits, &uStartBitOffset);
        iLastQP = curr_slice->m_iLastXmittedQP;
        uSaved_Skip_Run = curr_slice->m_uSkipRun;   // To restore it if we recompress
        uUsePCM = 0;    // Don't use the PCM mode initially.
        //cur_mb.LocalMacroblockInfo->QP = curr_slice->m_iLastXmittedQP;
        do {    // this is to recompress MBs that are too big.
            H264ENC_MAKE_NAME(H264CoreEncoder_MB_Decision)(state, curr_slice, uMB);
            Ipp32s mb_bits;
            Ipp32s bit_offset;
            if (core_enc->m_PicParamSet.entropy_coding_mode) {
                bit_offset = pBitstream->m_base.m_nReadyBits;
                if (pBitstream->m_base.m_nReadyBits == 9) bit_offset = 8;
            }
            // Code the macroblock, all planes
            cur_mb.LocalMacroblockInfo->cbp_bits = 0;
            cur_mb.LocalMacroblockInfo->cbp_bits_chroma = 0;
            uSaved_Skip_Run = curr_slice->m_uSkipRun;
            H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecMB)(state, curr_slice);

            mb_bits = 0;
            status = H264ENC_MAKE_NAME(H264CoreEncoder_Put_MB_Real)(state, curr_slice);
            if (status != UMC_OK) goto done;

//            if (core_enc->m_PicParamSet.entropy_coding_mode){
//                pBitstream->EncodeFinalSingleBin_CABAC( (uMB==uFirstMB + uNumMBs - 1) || (core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB + 1].slice_id != slice_num) );
//                mb_bits = bit_offset - pBitstream->m_nReadyBits;
//            }

            Ipp8u *pEndBits;
            Ipp32u uEndBitOffset;
            H264BsBase_GetState(&pBitstream->m_base, &pEndBits, &uEndBitOffset);

            mb_bits += (Ipp32u) (pEndBits - pStartBits)*8;
            if (uEndBitOffset >= uStartBitOffset)
                mb_bits += uEndBitOffset - uStartBitOffset;
            else
                mb_bits -= uStartBitOffset - uEndBitOffset;

            //            mbtype_stat[cur_mb.GlobalMacroblockInfo->mbtype]++;
#ifdef H264_STAT
            core_enc->hstats.addMB( slice_type, curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype, curr_slice->m_cur_mb.GlobalMacroblockInfo->sbtype, pGetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo), mb_bits);
            core_enc->hstats.addMBRefs( slice_type, curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype, curr_slice->m_cur_mb.RefIdxs);
#endif

            // Should not recompress for CABAC
            if (!core_enc->m_PicParamSet.entropy_coding_mode && (mb_bits > MB_RECODE_THRESH) && core_enc->m_info.rate_controls.method == H264_RCM_QUANT)
            {
                // OK, this is bad, it's not compressing very much!!!
                // TBD: Tune this decision to QP...  Higher QPs will progressively trash PSNR,
                // so if they are still using a lot of bits, then PCM coding is extra attractive.

                // We're going to be recoding this MB, so reset some stuff.
                H264BsBase_SetState(&pBitstream->m_base, pStartBits, uStartBitOffset);
                // Zero out unused bits in buffer before OR in next op
                // This removes dependency on buffer being zeroed out.
                *pStartBits = (Ipp8u)((*pStartBits >> (8-uStartBitOffset)) << (8-uStartBitOffset));

                curr_slice->m_iLastXmittedQP = iLastQP; // Restore the last xmitted QP
                curr_slice->m_uSkipRun = uSaved_Skip_Run;   // Restore the skip run

                // If the QP has only been adjusted up 0 or 1 times, and QP != 51
                if (((cur_mb.LocalMacroblockInfo->QP -
                    core_enc->m_PicParamSet.pic_init_qp + curr_slice->m_slice_qp_delta) < 2) &&
                    (cur_mb.LocalMacroblockInfo->QP != 51))
                {
                    // Quantize more and try again!
                    cur_mb.LocalMacroblockInfo->QP++;
                    uRecompressMB = 1;
                } else {
                    // Code this block as a PCM MB next time around.
                    uUsePCM = 1;
                    uRecompressMB = 0;
                    // Reset the MB QP value to the "last transmitted QP"
                    // Since no DeltaQP will be transmitted for a PCM block
                    // This is important, since the Loop Filter will use the
                    // this value in filtering this MB
                    cur_mb.LocalMacroblockInfo->QP = curr_slice->m_iLastXmittedQP;
                }

            } else
                uRecompressMB = 0;
        } while (uRecompressMB);        // End of the MB recompression loop.

        // If the above MB encoding failed to efficiently predict the MB, then
        // code it as raw pixels using the mb_type = PCM
        if (uUsePCM)
        {
            cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_PCM;
            cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;

            memset(cur_mb.MacroblockCoeffsInfo->numCoeff, 16, 24);

            Ipp32s  k;     // block number, 0 to 15
            for (k = 0; k < 16; k++) {
                cur_mb.intra_types[k] = 2;
                cur_mb.MVs[LIST_0]->MotionVectors[k] = null_mv;
                cur_mb.MVs[LIST_1]->MotionVectors[k] = null_mv;
                cur_mb.RefIdxs[LIST_0]->RefIdxs[k] = -1;
                cur_mb.RefIdxs[LIST_1]->RefIdxs[k] = -1;
            }

            H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBHeader_Real)(state, curr_slice);   // PCM values are written in the MB Header.
        }

#ifdef SLICE_CHECK_LIMIT
        Ipp32s size = (H264BsBase_GetBsOffset(&pBitstream->m_base) >> 3) + 3/*Start code*/ + 5;
        if (core_enc->m_MaxSliceSize && core_enc->m_MaxSliceSize < size){
            //Increase slice_id
            for (Ipp32u mb = uMB; mb < uFirstMB + uNumMBs; mb++) core_enc->m_pCurrentFrame->m_mbinfo.mbs[mb].slice_id++;
            curr_slice->m_MB_Counter--;
            //Restore bitstream
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                H264ENC_MAKE_NAME(H264BsReal_RestoreCABACState)(pBitstream);
                H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(pBitstream, 1);
            }else{
                H264BsBase_SetState(&pBitstream->m_base, pStartBits, uStartBitOffset);
                //Reset unused bits
                *pStartBits = *pStartBits & (0xff<<(8- uStartBitOffset));
            }
        }
#endif
        if (core_enc->m_PicParamSet.entropy_coding_mode){
#ifdef SLICE_CHECK_LIMIT
            if (core_enc->m_MaxSliceSize)
                H264ENC_MAKE_NAME(H264BsReal_SaveCABACState)(pBitstream);
#endif
            H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(
                pBitstream,
                (uMB == uFirstMB + uNumMBs - 1) ||
                (core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB + 1].slice_id != slice_num));
            H264ENC_MAKE_NAME(H264CoreEncoder_ReconstuctCBP)(&cur_mb);
        }
    }   // loop over MBs

#ifndef NO_FINAL_SKIP_RUN
    // Check if the last N MBs were skip blocks.  If so, write a final skip run
    // NOTE!  This is _optional_.  The encoder is not required to do this, and
    // decoders need to be able to handle it either way.

    // Even though skip runs are not written for I Slices, m_uSkipRun can only be
    // non-zero for non-I slices, so the following test is OK.
    if (curr_slice->m_uSkipRun !=0 && core_enc->m_info.entropy_coding_mode==0) {
        H264ENC_MAKE_NAME(H264BsReal_PutVLCCode)(pBitstream, curr_slice->m_uSkipRun);
    }

#endif // NO_FINAL_SKIP_RUN

    // save the frame class

done:
    if (core_enc->m_PicParamSet.entropy_coding_mode) {
        H264ENC_MAKE_NAME(H264BsReal_TerminateEncode_CABAC)(pBitstream);
    }
    else {
        H264BsBase_WriteTrailingBits(&pBitstream->m_base);
    }

#ifdef H264_COMMON_ME
    //============== common ME (delete and free vars for estimating frame) =======
    if (curr_slice->m_slice_type == PREDSLICE)
    {
        Ipp32s ref_idx;
        Ipp32s numRef = curr_slice->m_NumRefsInL0List;
        core_enc->m_pME->UnLockFrame(core_enc->MEParams->pSrc);
        for (ref_idx = 0; ref_idx < numRef; ref_idx ++)
            core_enc->m_pME->UnLockFrame(core_enc->MeFrameRefList[ref_idx]);
        delete core_enc->MeFrameRefList;
        memset(core_enc->MEParams, 0, sizeof(core_enc->MEParams));
    }
    //============================================================================
#endif

    return status;

}

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_ComputeMBFrameFieldCost)(
    void* state,
    H264SliceType *curr_slice,
    bool is_frame)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s curMBAddr = curr_slice->m_cur_mb.uMB;
    curr_slice->m_cur_mb.uMBx = ((curMBAddr >> 1) % core_enc->m_WidthInMBs);
    if ((curMBAddr & 1) == 0) {
        curr_slice->m_cur_mb.uMBy = ((curMBAddr >> 1) / core_enc->m_WidthInMBs)*2;
    } else {
        curr_slice->m_cur_mb.uMBy = ((curMBAddr >> 1) / core_enc->m_WidthInMBs)*2;
        if (!is_frame)
            curr_slice->m_cur_mb.uMBy ++;
    }
    H264ENC_MAKE_NAME(H264CoreEncoder_UpdateCurrentMBInfo)(state, curr_slice);
    SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr], is_frame ? 0 : 1);
    SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr+1], is_frame ? 0 : 1);
    curr_slice->m_is_cur_mb_field = pGetMBFieldDecodingFlag(curr_slice->m_cur_mb.GlobalMacroblockInfo);
    return H264ENC_MAKE_NAME(H264CoreEncoder_MB_Decision)(state, curr_slice, curMBAddr);
}

void H264ENC_MAKE_NAME(H264CoreEncoder_MBFrameFieldSelect)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u &curMBAddr = curr_slice->m_cur_mb.uMB;
#if 1
    Ipp32u uOffset = core_enc->m_pMBOffsets[curMBAddr].uLumaOffset[true][false];
    PIXTYPE* curr = core_enc->m_pCurrentFrame->m_pYPlane + uOffset;
    Ipp32s pitchPixels = core_enc->m_pCurrentFrame->m_pitchPixels;
    //Ipp32s pitchBytes = core_enc->m_pCurrentFrame->pitchBytes();
    Ipp32s frame_sad = SAD16x16(curr, pitchPixels, curr + pitchPixels, pitchPixels) + SAD16x16(curr + 16*pitchPixels, pitchPixels, curr + 17*pitchPixels, pitchPixels);
    Ipp32s field_sad = SAD16x16(curr, pitchPixels << 2, curr + (pitchPixels << 2), pitchPixels << 2) + SAD16x16(curr + pitchPixels, pitchPixels << 2, curr + (pitchPixels << 2) + pitchPixels, pitchPixels << 2);

    if ((frame_sad - field_sad) > 0)
    {
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr],   1);
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr+1], 1);
    } else {
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr],   0);
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr+1], 0);
    }

#elif 1
    Ipp32u field_sad = ComputeMBFrameFieldCost(false);
    curMBAddr++;
    field_sad += ComputeMBFrameFieldCost(false);
    curMBAddr--;

    Ipp32u frame_sad = ComputeMBFrameFieldCost(true);
    curMBAddr++;
    frame_sad += ComputeMBFrameFieldCost(true);
    curMBAddr--;

    if (field_sad < frame_sad)
    {
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr],1);
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr+1],1);
        curr_slice->m_is_cur_mb_field = pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo);
    }

#else
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr],0);
        SetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[curMBAddr+1],0);
#endif
}

Status H264ENC_MAKE_NAME(H264CoreEncoder_Compress_Slice_MBAFF)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s slice_num = curr_slice->m_slice_number;
    Ipp32u uAIMBSAD;            // MB prediction SAD for INTRA 4x4 mode
    Ipp32u uAIMBSAD_16x16;      // MB prediction SAD for INTRA 16x16 mode
    Ipp8u  uMBQP;
    MB_Type uBestIntraMBType;
    Ipp8u   uUsePCM = 0;

    Ipp8u *pStartBits;
    Ipp32u uStartBitOffset;

    Ipp32u uRecompressMB;
    Ipp8s  iLastQP;
    Ipp32u uSaved_Skip_Run;

    Ipp8u bSeenFirstMB = false;

    Status status = UMC_OK;

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32u &curMBAddr = cur_mb.uMB;
    H264BsRealType* pBitstream = (H264BsRealType *)curr_slice->m_pbitstream;
    Ipp32u uNumMBs = core_enc->m_HeightInMBs * core_enc->m_WidthInMBs;
    Ipp32u uFirstMB = core_enc->m_field_index*uNumMBs;

    curr_slice->m_use_transform_for_intra_decision = core_enc->m_info.use_transform_for_intra_decision ?
                    (curr_slice->m_slice_type == INTRASLICE) : false;

    // loop over all MBs in the picture
    for (Ipp32u uMB = uFirstMB; uMB < uFirstMB + uNumMBs; uMB++)
    {
        // Is this MB in the current slice?  If not, move on...
        if (core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB].slice_id != slice_num) {
            continue;
        } else if (!bSeenFirstMB)
        {
            // Reset xpos and ypos in framedata struct
            // This is necessary because the same slice may be recoded multiple times.

            // reset intra MB counter per slice
            curr_slice->m_Intra_MB_Counter = 0;
            curr_slice->m_MB_Counter = 0;
            // Fill in the first mb in slice field in the slice header.
            curr_slice->m_first_mb_in_slice = uMB >> 1;

            // Fill in the current deblocking filter parameters.
            curr_slice->m_slice_alpha_c0_offset = (Ipp8s)core_enc->m_info.deblocking_filter_alpha;
            curr_slice->m_slice_beta_offset = (Ipp8s)core_enc->m_info.deblocking_filter_beta;
            curr_slice->m_disable_deblocking_filter_idc = core_enc->m_info.deblocking_filter_idc;
            curr_slice->m_cabac_init_idc = core_enc->m_info.cabac_init_idc;

            // Write a slice header
            H264ENC_MAKE_NAME(H264BsReal_PutSliceHeader)(
                pBitstream,
                core_enc->m_SliceHeader,
                core_enc->m_PicParamSet,
                core_enc->m_SeqParamSet,
                core_enc->m_PicClass,
                curr_slice);
            bSeenFirstMB = true;

            // Fill in the correct value for m_iLastXmittedQP, used to correctly code
            // the per MB QP Delta
            curr_slice->m_iLastXmittedQP = core_enc->m_PicParamSet.pic_init_qp + curr_slice->m_slice_qp_delta;
            Ipp32s SliceQPy = curr_slice->m_iLastXmittedQP;

            if (core_enc->m_info.entropy_coding_mode)
            {
                if (curr_slice->m_slice_type==INTRASLICE)
                    H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesIntra_CABAC)(
                        pBitstream,
                        SliceQPy);
                else
                    H264ENC_MAKE_NAME(H264BsReal_InitializeContextVariablesInter_CABAC)(
                        pBitstream,
                        SliceQPy,
                        curr_slice->m_cabac_init_idc);
            }

            // Initialize the MB skip run counter
            curr_slice->m_uSkipRun = 0;
        }

        curMBAddr = uMB;
        if ((uMB&1)==0) H264ENC_MAKE_NAME(H264CoreEncoder_MBFrameFieldSelect)(state, curr_slice);
        cur_mb.uMBx = ((uMB >> 1) % core_enc->m_WidthInMBs);
        if ((uMB & 1)==0) {
            cur_mb.uMBy = ((uMB >> 1) / core_enc->m_WidthInMBs) * 2;
        } else {
            if (!GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB]))
                cur_mb.uMBy ++;
        }
        H264ENC_MAKE_NAME(H264CoreEncoder_UpdateCurrentMBInfo)(state, curr_slice);

        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 0);

        curr_slice->m_MB_Counter++;
        H264BsBase_GetState(curr_slice->m_pbitstream, &pStartBits, &uStartBitOffset);
        iLastQP = curr_slice->m_iLastXmittedQP;
        uSaved_Skip_Run = curr_slice->m_uSkipRun;   // To restore it if we recompress
        uUsePCM = 0;    // Don't use the PCM mode initially.
        curr_slice->m_is_cur_mb_field = pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo);
        curr_slice->m_InitialOffset = core_enc->m_InitialOffsets[curr_slice->m_is_cur_mb_field & curMBAddr];
        curr_slice->m_is_cur_mb_bottom_field = (curr_slice->m_is_cur_mb_field & curMBAddr) == 1;

        do {    // this is to recompress MBs that are too big.
            uMBQP = getLumaQP51(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.bit_depth_luma);
            cur_mb.LocalMacroblockInfo->cbp_chroma =
            cur_mb.LocalMacroblockInfo->cbp_luma =
            cur_mb.LocalMacroblockInfo->cbp_bits =
            cur_mb.LocalMacroblockInfo->cbp_bits_chroma =
            cur_mb.LocalMacroblockInfo->cbp =
            cur_mb.LocalMacroblockInfo->intra_chroma_mode = 0;
            Ipp32s MBHasEdges;
            // Use edge detection to determine if the MB is a flat region
            H264ENC_MAKE_NAME(ippiEdgesDetect16x16)(core_enc->m_pCurrentFrame->m_pYPlane + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][curr_slice->m_is_cur_mb_field],
                core_enc->m_pCurrentFrame->m_pitchPixels<<curr_slice->m_is_cur_mb_field,uMBQP/2,EdgePelCountTable[uMBQP],&MBHasEdges);

            if (curr_slice->m_slice_type != INTRASLICE) {
                if (core_enc->m_Analyse & ANALYSE_INTRA_IN_ME) {
                    cur_mb.LocalMacroblockInfo->cost = (BPREDSLICE == curr_slice->m_slice_type) ? H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(state, curr_slice) : H264ENC_MAKE_NAME(H264CoreEncoder_ME_P)(state, curr_slice);
                    if (core_enc->m_PicParamSet.chroma_format_idc == 0)
                        cur_mb.LocalMacroblockInfo->cbp_chroma = 0;
                } else {
                    Ipp32u uMinIntraSAD, uBestIntraSAD, uBestInterSAD;
                    uBestInterSAD = (BPREDSLICE == curr_slice->m_slice_type) ? H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(state, curr_slice) : H264ENC_MAKE_NAME(H264CoreEncoder_ME_P)(state, curr_slice);
                    // save INTER results for possible later switch back to INTER
                    Ipp32u uInterCBP4x4 = cur_mb.LocalMacroblockInfo->cbp_luma;
                    MBTypeValue MBTypeInter = cur_mb.GlobalMacroblockInfo->mbtype;
                    Ipp32s InterMB8x8PackFlag = pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo);
                    uMinIntraSAD = rd_quant_intra_min[uMBQP];

                    if (MBHasEdges && (uBestInterSAD >= uMinIntraSAD)) {
                        H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, curr_slice->m_pMBEncodeBuffer);
                        uBestIntraSAD = uAIMBSAD_16x16;
                        uBestIntraMBType = MBTYPE_INTRA_16x16;
                        H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uBestIntraSAD, &uAIMBSAD);
                        //FPV TODO ADD 8x8?
                        if (uAIMBSAD < uBestIntraSAD) {
                            uBestIntraSAD = uAIMBSAD;
                            uBestIntraMBType = MBTYPE_INTRA;
                        }
                    } else {
                        H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, curr_slice->m_pMBEncodeBuffer);
                        uBestIntraSAD = uAIMBSAD_16x16;
                        uBestIntraMBType = MBTYPE_INTRA_16x16;
                    }
                    if (uBestIntraSAD < uBestInterSAD) {
                        cur_mb.GlobalMacroblockInfo->mbtype = static_cast<MBTypeValue>(uBestIntraMBType);
                        cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
                        cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
                        curr_slice->m_Intra_MB_Counter++;
                    }
                    else {
                        cur_mb.GlobalMacroblockInfo->mbtype = MBTypeInter;
                        cur_mb.LocalMacroblockInfo->cbp_luma = uInterCBP4x4;
                        pSetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo, InterMB8x8PackFlag);
                    }
                }
            } else { // intra slice
                if (MBHasEdges) {
                    H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, curr_slice->m_pMBEncodeBuffer);
                    uBestIntraMBType = MBTYPE_INTRA_16x16;
                    H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(state, curr_slice, uAIMBSAD, &uAIMBSAD);
                    if (uAIMBSAD_16x16 < uAIMBSAD)
                        uBestIntraMBType = MBTYPE_INTRA;
                } else {
                    H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(state, curr_slice, &uAIMBSAD_16x16, curr_slice->m_pMBEncodeBuffer);
                    uBestIntraMBType = MBTYPE_INTRA_16x16;
                }
                cur_mb.GlobalMacroblockInfo->mbtype = static_cast<MBTypeValue>(uBestIntraMBType);
                cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;
                cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
                curr_slice->m_Intra_MB_Counter++;
            }
            cur_mb.LocalMacroblockInfo->cbp_bits = 0;
            cur_mb.LocalMacroblockInfo->cbp_bits_chroma = 0;
            cur_mb.uMB = uMB;
            H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecMB)(state, curr_slice);
            status = H264ENC_MAKE_NAME(H264CoreEncoder_Put_MB_Real)(state, curr_slice);
            if (status != UMC_OK)
                goto done;

            Ipp8u *pEndBits;
            Ipp32u uEndBitOffset;

            H264BsBase_GetState(curr_slice->m_pbitstream, &pEndBits, &uEndBitOffset);

            Ipp32u mb_bits = (Ipp32u) (pEndBits - pStartBits)*8;
            if (uEndBitOffset >= uStartBitOffset)
                mb_bits += uEndBitOffset - uStartBitOffset;
            else
                mb_bits -= uStartBitOffset - uEndBitOffset;

            if (!core_enc->m_PicParamSet.entropy_coding_mode && (mb_bits > MB_RECODE_THRESH) && core_enc->m_info.rate_controls.method == H264_RCM_QUANT)
            {
                // OK, this is bad, it's not compressing very much!!!
                // TBD: Tune this decision to QP...  Higher QPs will progressively trash PSNR,
                // so if they are still using a lot of bits, then PCM coding is extra attractive.

                // We're going to be recoding this MB, so reset some stuff.
                H264BsBase_SetState(curr_slice->m_pbitstream, pStartBits, uStartBitOffset);    // Reset the BS
                // Zero out unused bits in buffer before OR in next op
                // This removes dependency on buffer being zeroed out.
                *pStartBits = (Ipp8u)((*pStartBits >> (8-uStartBitOffset)) << (8-uStartBitOffset));

                curr_slice->m_iLastXmittedQP = iLastQP; // Restore the last xmitted QP
                curr_slice->m_uSkipRun = uSaved_Skip_Run;   // Restore the skip run

                // If the QP has only been adjusted up 0 or 1 times, and QP != 51
                if (((cur_mb.LocalMacroblockInfo->QP -
                    core_enc->m_PicParamSet.pic_init_qp + curr_slice->m_slice_qp_delta) < 2) &&
                    (cur_mb.LocalMacroblockInfo->QP != 51))
                {
                    // Quantize more and try again!
                    cur_mb.LocalMacroblockInfo->QP++;
                    uRecompressMB = 1;
                } else {
                    // Code this block as a PCM MB next time around.
                    uUsePCM = 1;
                    uRecompressMB = 0;
                    // Reset the MB QP value to the "last transmitted QP"
                    // Since no DeltaQP will be transmitted for a PCM block
                    // This is important, since the Loop Filter will use the
                    // this value in filtering this MB
                    cur_mb.LocalMacroblockInfo->QP = curr_slice->m_iLastXmittedQP;
                }

            } else
                uRecompressMB = 0;

        } while (uRecompressMB);        // End of the MB recompression loop.

        // If the above MB encoding failed to efficiently predict the MB, then
        // code it as raw pixels using the mb_type = PCM
        if (uUsePCM)
        {
            cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_PCM;
            cur_mb.LocalMacroblockInfo->cbp_luma = 0xffff;

            memset(cur_mb.MacroblockCoeffsInfo->numCoeff, 16, 24);

            Ipp32s k;     // block number, 0 to 15
            for (k = 0; k < 16; k++) {
                cur_mb.intra_types[k] = 2;
                cur_mb.MVs[LIST_0]->MotionVectors[k] = null_mv;
                cur_mb.MVs[LIST_1]->MotionVectors[k] = null_mv;
                cur_mb.RefIdxs[LIST_0]->RefIdxs[k] = -1;
                cur_mb.RefIdxs[LIST_1]->RefIdxs[k] = -1;
            }

            H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBHeader_Real)(state, curr_slice);   // PCM values are written in the MB Header.
        }

        /*
        if (!m_pbitstream->CheckBsLimit())
        {
        // If the buffer is filled, break out of encoding loop
        // Encoder::CompressFrame will write a blank frame to the bitstream.
        goto done;
        }*/

        if (core_enc->m_PicParamSet.entropy_coding_mode)
        {
            if ((uMB & 1) != 0)
            {
                H264ENC_MAKE_NAME(H264BsReal_EncodeFinalSingleBin_CABAC)(
                    pBitstream,
                    (uMB==(uFirstMB + uNumMBs - 1)) ||
                        (core_enc->m_pCurrentFrame->m_mbinfo.mbs[uMB + 1].slice_id != slice_num));
            }
            H264ENC_MAKE_NAME(H264CoreEncoder_ReconstuctCBP)(&cur_mb);
        }
    }   // loop over MBs

#ifndef NO_FINAL_SKIP_RUN
    // Check if the last N MBs were skip blocks.  If so, write a final skip run
    // NOTE!  This is _optional_.  The encoder is not required to do this, and
    // decoders need to be able to handle it either way.

    // Even though skip runs are not written for I Slices, m_uSkipRun can only be
    // non-zero for non-I slices, so the following test is OK.
    if (curr_slice->m_uSkipRun !=0 && core_enc->m_info.entropy_coding_mode==0) {
        H264ENC_MAKE_NAME(H264BsReal_PutVLCCode)(pBitstream, curr_slice->m_uSkipRun);
    }

#endif // NO_FINAL_SKIP_RUN

done:
    if (core_enc->m_PicParamSet.entropy_coding_mode)
    {
        H264ENC_MAKE_NAME(H264BsReal_TerminateEncode_CABAC)(pBitstream);
    }
    // use core timing infrastructure to measure/report deblock filter time
    // by measuring the time to get from here to the start of EndPicture.
    return status;

}

Status H264ENC_MAKE_NAME(H264CoreEncoder_Start_Picture)(
    void* state,
    const EnumPicClass* pic_class,
    EnumPicCodType pic_type)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Status ps = UMC_OK;

    core_enc->m_PicClass = *pic_class;
    core_enc->use_implicit_weighted_bipred = (core_enc->m_PicParamSet.weighted_bipred_idc == 2);
    if (!core_enc->m_is_mb_data_initialized){
        Ipp32s uNumMBs = core_enc->m_HeightInMBs*core_enc->m_WidthInMBs>>(Ipp8u)(core_enc->m_pCurrentFrame->m_PictureStructureForDec<FRM_STRUCTURE);
        core_enc->uNumSlices = core_enc->m_info.num_slices;
        core_enc->m_slice_length = uNumMBs / core_enc->uNumSlices;
        core_enc->m_uSliceRemainder = uNumMBs % core_enc->uNumSlices;

        if (core_enc->m_SliceHeader.MbaffFrameFlag && (core_enc->m_slice_length&1)){
            core_enc->m_slice_length--;
            core_enc->m_uSliceRemainder += core_enc->uNumSlices;
        }

        H264ENC_MAKE_NAME(H264CoreEncoder_InitializeMBData)(state);
        core_enc->m_is_mb_data_initialized = 1;
    }

    if (core_enc->m_field_index == 0)  H264ENC_MAKE_NAME(H264CoreEncoder_Make_MBSlices)(state);

    if (core_enc->m_pCurrentFrame->m_PictureStructureForDec<FRM_STRUCTURE){
        core_enc->m_HeightInMBs/=2;
    }

    if (core_enc->m_info.rate_controls.method == H264_RCM_VBR || core_enc->m_info.rate_controls.method == H264_RCM_CBR){
#ifdef FRAME_QP_FROM_FILE
        Ipp32s slice_qp = core_enc->m_pCurrentFrame->frame_qp;
#else
        Ipp32s slice_qp = H264_AVBR_GetQP(&core_enc->avbr, pic_type);
#endif
        core_enc->m_Slices[0].m_slice_qp_delta = (Ipp8s)(slice_qp - core_enc->m_PicParamSet.pic_init_qp);
    }else if(core_enc->m_info.rate_controls.method == H264_RCM_DEBUG){
            switch (pic_type) {
                case PREDPIC:
                    core_enc->m_Slices[0].m_slice_qp_delta = core_enc->m_info.rate_controls.quantP - core_enc->m_PicParamSet.pic_init_qp;
                    break;
                case BPREDPIC:
                    core_enc->m_Slices[0].m_slice_qp_delta = core_enc->m_info.rate_controls.quantB - core_enc->m_PicParamSet.pic_init_qp;
                    break;
                case INTRAPIC:
                    core_enc->m_Slices[0].m_slice_qp_delta = core_enc->m_info.rate_controls.quantI - core_enc->m_PicParamSet.pic_init_qp;
                    break;
            }
    }

#if defined (ALPHA_BLENDING_H264)
    if(core_enc->m_pCurrentFrame->m_isAuxiliary) {
        Ipp32s slice_qp = core_enc->m_Slices[0].m_slice_qp_delta + core_enc->m_PicParamSet.pic_init_qp;
        slice_qp = convertLumaQP(slice_qp, core_enc->m_SeqParamSet.bit_depth_luma, core_enc->m_PicParamSet.bit_depth_luma);
        core_enc->m_Slices[0].m_slice_qp_delta = slice_qp - core_enc->m_PicParamSet.pic_init_qp;
    }
#endif // ALPHA_BLENDING_H264
    for(Ipp32s sliceId = 0; sliceId < core_enc->m_info.num_slices*((core_enc->m_info.coding_type == 1) + 1); sliceId++) {
        core_enc->m_Slices[sliceId].m_slice_qp_delta = core_enc->m_Slices[0].m_slice_qp_delta;
        core_enc->m_Slices[sliceId].m_iLastXmittedQP = core_enc->m_PicParamSet.pic_init_qp + core_enc->m_Slices[sliceId].m_slice_qp_delta;

        // reset intra MB counter
        core_enc->m_Slices[sliceId].m_Intra_MB_Counter = 0;
        core_enc->m_Slices[sliceId].m_MB_Counter = 0;
        core_enc->m_Slices[sliceId].m_prev_dquant = 0;
    }

    Ipp32s slice_qp = core_enc->m_PicParamSet.pic_init_qp + core_enc->m_Slices[0].m_slice_qp_delta;
    for (Ipp32s uMB = core_enc->m_field_index*core_enc->m_WidthInMBs * core_enc->m_HeightInMBs; uMB < (core_enc->m_field_index+1)* core_enc->m_WidthInMBs * core_enc->m_HeightInMBs; uMB++){
        // initialize QP settings and MB type
        core_enc->m_mbinfo.mbs[uMB].QP = slice_qp;
    }

    return ps;
}


#ifdef FRAME_INTERPOLATION
void H264ENC_MAKE_NAME(InterpolateHP)(
    PIXTYPE *pY,
    Ipp32s mbw,
    Ipp32s mbh,
    Ipp32s step,
    Ipp32s planeSize,
    Ipp32s bitDepth)
{
    Ipp32s   i, j;
    PIXTYPE  *pB, *pH, *pJ;
    IppiSize sz = {16, 16};

    pB = pY + planeSize;
    pH = pB + planeSize;
    pJ = pH + planeSize;
    for (i = -1; i <= mbh; i ++) {
        for (j = -1; j <= mbw; j ++) {
            H264ENC_MAKE_NAME(ippiInterpolateLuma_H264)(pY + i * 16 * step + j * 16, step, pB + i * 16 * step + j * 16, step, 2, 0, sz, bitDepth);
            H264ENC_MAKE_NAME(ippiInterpolateLuma_H264)(pY + i * 16 * step + j * 16, step, pH + i * 16 * step + j * 16, step, 0, 2, sz, bitDepth);
            H264ENC_MAKE_NAME(ippiInterpolateLuma_H264)(pY + i * 16 * step + j * 16, step, pJ + i * 16 * step + j * 16, step, 2, 2, sz, bitDepth);
        }
    }
    H264ENC_MAKE_NAME(ExpandPlane)(pB - 16 * step - 16, (mbw + 2)* 16, (mbh + 2)* 16, step, LUMA_PADDING - 16);
    H264ENC_MAKE_NAME(ExpandPlane)(pH - 16 * step - 16, (mbw + 2)* 16, (mbh + 2)* 16, step, LUMA_PADDING - 16);
    H264ENC_MAKE_NAME(ExpandPlane)(pJ - 16 * step - 16, (mbw + 2)* 16, (mbh + 2)* 16, step, LUMA_PADDING - 16);
}
#endif


///////////////////////////////////////////////////////////////////////////
//
//  End_Picture
//
// Any processing needed after each picture
//
///////////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME(H264CoreEncoder_End_Picture)(
    void* state)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    // deblock filter just completed
    if (core_enc->m_PicClass != DISPOSABLE_PIC){
        Ipp32s shift_x = 0;
        Ipp32s shift_y = 0;

        switch( core_enc->m_PicParamSet.chroma_format_idc ){
            case 1:
                shift_x=shift_y=1;
                break;
            case 2:
                shift_x=1;
                shift_y=0;
                break;
            case 3:
                shift_x=shift_y=0;
                break;
        }
        Ipp32s chromaPadding = LUMA_PADDING >> 1;
        if (VideoData_GetColorFormat(&core_enc->m_pCurrentFrame->m_data) == YUV422
#ifdef ALPHA_BLENDING_H264
            || VideoData_GetColorFormat(&core_enc->m_pCurrentFrame->m_data) == YUV422A
#endif
            )
            chromaPadding = LUMA_PADDING;
        switch (core_enc->m_pCurrentFrame->m_PictureStructureForDec){
        case FRM_STRUCTURE:
            H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pYPlane, core_enc->m_WidthInMBs*16, core_enc->m_HeightInMBs*16, core_enc->m_pReconstructFrame->m_pitchPixels, LUMA_PADDING);
#ifdef FRAME_INTERPOLATION
            H264ENC_MAKE_NAME(InterpolateHP)(core_enc->m_pReconstructFrame->m_pYPlane, core_enc->m_WidthInMBs, core_enc->m_HeightInMBs, core_enc->m_pReconstructFrame->m_pitchPixels, core_enc->m_pReconstructFrame->m_PlaneSize, core_enc->m_PicParamSet.bit_depth_luma);
#endif
            if(core_enc->m_PicParamSet.chroma_format_idc){
                H264ENC_MAKE_NAME(ExpandPlane)(
                    core_enc->m_pReconstructFrame->m_pUPlane,
                    core_enc->m_WidthInMBs * 16 >> shift_x,
                    core_enc->m_HeightInMBs * 16 >> shift_y,
                    core_enc->m_pReconstructFrame->m_pitchPixels,
                    chromaPadding);
                H264ENC_MAKE_NAME(ExpandPlane)(
                    core_enc->m_pReconstructFrame->m_pVPlane,
                    core_enc->m_WidthInMBs * 16 >> shift_x,
                    core_enc->m_HeightInMBs * 16 >> shift_y,
                    core_enc->m_pReconstructFrame->m_pitchPixels,
                    chromaPadding);
            }
            break;
        case AFRM_STRUCTURE:
            if(!shift_y) shift_y=1;
            else shift_y <<= 1;
            H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pYPlane, core_enc->m_WidthInMBs*16, core_enc->m_HeightInMBs*16>>1, core_enc->m_pReconstructFrame->m_pitchPixels*2, LUMA_PADDING);
            if(core_enc->m_PicParamSet.chroma_format_idc){
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pUPlane, core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pReconstructFrame->m_pitchPixels*2, chromaPadding);
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pVPlane, core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pReconstructFrame->m_pitchPixels*2, chromaPadding);
            }
            H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pReconstructFrame->m_pitchPixels, core_enc->m_WidthInMBs*16, core_enc->m_HeightInMBs*16>>1, core_enc->m_pReconstructFrame->m_pitchPixels*2, LUMA_PADDING);
            if(core_enc->m_PicParamSet.chroma_format_idc){
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pUPlane + core_enc->m_pReconstructFrame->m_pitchPixels, core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pCurrentFrame->m_pitchPixels*2, chromaPadding);
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pVPlane + core_enc->m_pReconstructFrame->m_pitchPixels, core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pReconstructFrame->m_pitchPixels*2, chromaPadding);
            }
            break;
        case FLD_STRUCTURE:
            H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pReconstructFrame->m_pitchPixels*core_enc->m_pReconstructFrame->m_bottom_field_flag[core_enc->m_field_index],
                        core_enc->m_WidthInMBs*16, core_enc->m_HeightInMBs*16, core_enc->m_pReconstructFrame->m_pitchPixels*2, LUMA_PADDING);
#ifdef FRAME_INTERPOLATION
            H264ENC_MAKE_NAME(InterpolateHP)(core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pReconstructFrame->m_pitchPixels*core_enc->m_pReconstructFrame->m_bottom_field_flag[core_enc->m_field_index], core_enc->m_WidthInMBs, core_enc->m_HeightInMBs, core_enc->m_pReconstructFrame->m_pitchPixels * 2, core_enc->m_pReconstructFrame->m_PlaneSize, core_enc->m_PicParamSet.bit_depth_luma);
#endif
            if(core_enc->m_PicParamSet.chroma_format_idc) {
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pUPlane + core_enc->m_pReconstructFrame->m_pitchPixels*core_enc->m_pReconstructFrame->m_bottom_field_flag[core_enc->m_field_index],
                            core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pReconstructFrame->m_pitchPixels*2, chromaPadding);
                H264ENC_MAKE_NAME(ExpandPlane)(core_enc->m_pReconstructFrame->m_pVPlane + core_enc->m_pReconstructFrame->m_pitchPixels*core_enc->m_pReconstructFrame->m_bottom_field_flag[core_enc->m_field_index],
                            core_enc->m_WidthInMBs*16>>shift_x, core_enc->m_HeightInMBs*16>>shift_y, core_enc->m_pReconstructFrame->m_pitchPixels*2, chromaPadding);
            }
            break;
        }
    }
}   // End_Picture

////////////////////////////////////////////////////////////////////////////////
// InitializeMBData
//
// One-time (after allocation) initialization of the per MB data,
// specifically edge flags which are set to match picture edges,
// the MB offsets, and block index.
//
////////////////////////////////////////////////////////////////////////////////

void H264ENC_MAKE_NAME(H264CoreEncoder_InitializeMBData)(
    void* state)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u uMB = 0;
    Ipp32u uLumaOffsetFRM = 0;
    Ipp32u uChromaOffsetFRM = 0;
    Ipp32u uLumaOffsetFLD = core_enc->m_pCurrentFrame->m_bottom_field_flag[0]*core_enc->m_Pitch;
    Ipp32u uChromaOffsetFLD = core_enc->m_pCurrentFrame->m_bottom_field_flag[0]*core_enc->m_Pitch;
    Ipp32u uLumaOffsetAFRMFRM = 0;
    Ipp32u uChromaOffsetAFRMFRM = 0;
    Ipp32u uLumaOffsetAFRMFLD = 0;
    Ipp32u uChromaOffsetAFRMFLD = 0;
    Ipp32s chroma_width = 0, chroma_height = 0;

    switch( core_enc->m_PicParamSet.chroma_format_idc ){
        case 0:
        case 1:
            chroma_width=chroma_height=8;
            break;
        case 2:
            chroma_width=8;
            chroma_height=16;
            break;
        case 3:
            chroma_width=16;
            chroma_height=16;
            break;
    }

    // Initialize the MB slices first, so that the edge calculations
    // below come out correctly.
    for (Ipp32s uMBRow = 0; uMBRow < core_enc->m_HeightInMBs; uMBRow++)
    {
        for (Ipp32s uMBCol = 0; uMBCol < core_enc->m_WidthInMBs; uMBCol++, uMB++)
        {
            core_enc->m_pMBOffsets[uMB].uLumaOffset[0][0] = uLumaOffsetFRM;
            core_enc->m_pMBOffsets[uMB].uChromaOffset[0][0] = uChromaOffsetFRM;
            core_enc->m_pMBOffsets[uMB].uLumaOffset[0][1] = uLumaOffsetFLD;
            core_enc->m_pMBOffsets[uMB].uChromaOffset[0][1] = uChromaOffsetFLD;

            core_enc->m_pMBOffsets[uMB].uLumaOffset[1][0] = uLumaOffsetAFRMFRM;
            core_enc->m_pMBOffsets[uMB].uChromaOffset[1][0] = uChromaOffsetAFRMFRM;
            core_enc->m_pMBOffsets[uMB].uLumaOffset[1][1] = uLumaOffsetAFRMFLD;
            core_enc->m_pMBOffsets[uMB].uChromaOffset[1][1] = uChromaOffsetAFRMFLD;

            uLumaOffsetFRM   += 16;
            uChromaOffsetFRM += chroma_width;
            uLumaOffsetFLD   += 16;
            uChromaOffsetFLD += chroma_width;
            if ((uMB&1)==0)
            {
                uLumaOffsetAFRMFRM   += core_enc->m_Pitch*16;
                uChromaOffsetAFRMFRM += core_enc->m_Pitch*chroma_height; //TODO???
                uLumaOffsetAFRMFLD   += core_enc->m_Pitch;
                uChromaOffsetAFRMFLD += core_enc->m_Pitch;
            }
            else
            {
                uLumaOffsetAFRMFRM   += 16 - core_enc->m_Pitch*16;
                uChromaOffsetAFRMFRM += chroma_width - core_enc->m_Pitch*chroma_height;  //TODO???
                uLumaOffsetAFRMFLD   += 16 - core_enc->m_Pitch;
                uChromaOffsetAFRMFLD += chroma_width - core_enc->m_Pitch;   //TODO ???

            }
        }

        uLumaOffsetFRM += (core_enc->m_Pitch*16) - core_enc->m_WidthInMBs*16;
        uChromaOffsetFRM += (core_enc->m_Pitch*chroma_height) - core_enc->m_WidthInMBs*chroma_width;
        uLumaOffsetFLD += (core_enc->m_Pitch*32) - core_enc->m_WidthInMBs*16;
        uChromaOffsetFLD += (core_enc->m_Pitch*(chroma_height<<1)) - core_enc->m_WidthInMBs*chroma_width;
        if (uMBRow == core_enc->m_HeightInMBs/2-1)
        {
            uLumaOffsetFLD = core_enc->m_pCurrentFrame->m_bottom_field_flag[1]*core_enc->m_Pitch;
            uChromaOffsetFLD = core_enc->m_pCurrentFrame->m_bottom_field_flag[1]*core_enc->m_Pitch;
        }
        if (uMBRow&1)
        {
            uLumaOffsetAFRMFRM   += core_enc->m_Pitch*32 - core_enc->m_WidthInMBs*16;
            uChromaOffsetAFRMFRM += core_enc->m_Pitch*16 - core_enc->m_WidthInMBs*8;  //TODO???
            uLumaOffsetAFRMFLD   += core_enc->m_Pitch*32 - core_enc->m_WidthInMBs*16;
            uChromaOffsetAFRMFLD += core_enc->m_Pitch*16 - core_enc->m_WidthInMBs*8;  //TODO???
        }
    }
    T_EncodeMBOffsets *pMBOffsets = core_enc->m_pMBOffsets;
#define BORDER_SIZE 15
    if (core_enc->m_pCurrentFrame->m_PictureStructureForDec == FRM_STRUCTURE) {
        Ipp32s row, col; //, height_in_mbs = core_enc->m_HeightInMBs >> (core_enc->m_pCurrentFrame->m_PictureStructureForDec < FRM_STRUCTURE ? 1 : 0);
    //    for (row = core_enc->m_field_index * height_in_mbs; row < ((core_enc->m_field_index + 1) * height_in_mbs) << 4; row += 16) {
        for (row = 0; row < core_enc->m_HeightInMBs << 4; row += 16) {
            for (col = 0; col < core_enc->m_WidthInMBs << 4; col += 16) {
                pMBOffsets->uMVLimits_L = -MIN(MAX_MV_INT, col + BORDER_SIZE);
                pMBOffsets->uMVLimits_R =  MIN(MAX_MV_INT, ((core_enc->m_WidthInMBs - 1) << 4) - col + BORDER_SIZE);
                pMBOffsets->uMVLimits_U = -MIN(MAX_MV_INT,  row + BORDER_SIZE);
               // pMBOffsets->uMVLimits_D =  MIN(MAX_MV_INT, ((height_in_mbs - 1) << 4) - row + BORDER_SIZE);
                pMBOffsets->uMVLimits_D =  MIN(MAX_MV_INT, ((core_enc->m_HeightInMBs - 1) << 4) - row + BORDER_SIZE);
                pMBOffsets ++;
            }
        }
    } else {
        Ipp32s row, col;
        for (row = 0; row < core_enc->m_HeightInMBs << 3; row += 16) {
            for (col = 0; col < core_enc->m_WidthInMBs << 4; col += 16) {
                pMBOffsets->uMVLimits_L = -MIN(MAX_MV_INT, col + BORDER_SIZE);
                pMBOffsets->uMVLimits_R =  MIN(MAX_MV_INT, ((core_enc->m_WidthInMBs - 1) << 4) - col + BORDER_SIZE);
                pMBOffsets->uMVLimits_U = -MIN(MAX_MV_INT,  row + BORDER_SIZE);
                pMBOffsets->uMVLimits_D =  MIN(MAX_MV_INT, (((core_enc->m_HeightInMBs >> 1) - 1) << 4) - row + BORDER_SIZE);
                pMBOffsets ++;
            }
        }
        for (row = 0; row < core_enc->m_HeightInMBs << 3; row += 16) {
            for (col = 0; col < core_enc->m_WidthInMBs << 4; col += 16) {
                pMBOffsets->uMVLimits_L = -MIN(MAX_MV_INT, col + BORDER_SIZE);
                pMBOffsets->uMVLimits_R =  MIN(MAX_MV_INT, ((core_enc->m_WidthInMBs - 1) << 4) - col + BORDER_SIZE);
                pMBOffsets->uMVLimits_U = -MIN(MAX_MV_INT,  row + BORDER_SIZE);
                pMBOffsets->uMVLimits_D =  MIN(MAX_MV_INT, (((core_enc->m_HeightInMBs >> 1) - 1) << 4) - row + BORDER_SIZE);
                pMBOffsets ++;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
//  Make_MBSlices - "Slices" the frame by writing a Slice
//  number into the uSlice member of the per MB data structure.
//  Right now, this method implements a simple scheme where a number
//  of fixed length Slices are present in a single JVT Slice Group,
//  and the last slice contains any "remainder" MBs (and thus may be larger).
//
//  Other slicing schemes can be implemented by parameterizing this
//  method to produce different values of uSlice for each MB.
//  If doing something other than simple raster ordered slices, then
//  don't forget to change the Picture Header to properly encode the
//  chosen slicing method as appropriate "Slice Groups".
//  Right now, only a single Slice Group is used, so there is no
//  mb_allocation_map_type coded.
//
///////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME(H264CoreEncoder_Make_MBSlices)(
    void* state)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s mb = 0;
    Ipp16u uSlice = 0;
    Ipp32u uSliceMBCnt = 0;

    Ipp32s mb_nums = (core_enc->m_HeightInMBs*core_enc->m_WidthInMBs) >> (Ipp8u)(core_enc->m_pCurrentFrame->m_PictureStructureForDec<FRM_STRUCTURE);

    for (Ipp32s i = 0; i < (core_enc->m_pCurrentFrame->m_PictureStructureForDec < FRM_STRUCTURE) + 1; i++)
    {
        Ipp32s num_slices = core_enc->uNumSlices << i*(core_enc->m_pCurrentFrame->m_PictureStructureForDec<FRM_STRUCTURE);

        for (mb = i*mb_nums; mb < mb_nums*(i+1); mb++)
        {
            core_enc->m_pCurrentFrame->m_mbinfo.mbs[mb].slice_id = uSlice;

            uSliceMBCnt++;
            if ((uSliceMBCnt == core_enc->m_slice_length) &&
                (uSlice < num_slices - 1)) {
                uSlice++;
                uSliceMBCnt = 0;
            }
        }

        uSlice++;
        uSliceMBCnt = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Set_MVLimits
//
//  Initialize the MVLimits field of the MB offsets struct for all MBs. The field
//  contains the maximum (picture edge limited) search range in each
//  direction for each MB.
//
////////////////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(
    COEFFSTYPE* coeff,
    const Ipp32s* dec_single_scan,
    T_Block_CABAC_DataType* c_data)
{
    /* This should be set outside
        c_data->uLastSignificant - last coeffs in decode scan
        c_data->uNumSigCoeffs - number of non-zero coeffs
        c_data->CtxBlockCat - ctxBlockCat - block context
        c_data->uLastCoeff - last possible coeff
        c_data->uFirstCoeff - first possible coeff
    */

    Ipp8u j=0;
    Ipp32s i;

    for (i=c_data->uFirstCoeff;i<=c_data->uLastSignificant;i++){
        Ipp32s coef=coeff[dec_single_scan[i]];
        if (coef)
        {
            Ipp32s sign = coef < 0;
            coef = (sign)? -coef : coef;
            c_data->uSignificantMap[j] = (Ipp8u)i;
            c_data->uSignificantLevels[j] = coef - 1;
            c_data->uSignificantSigns[j] = (Ipp8u)sign;
            j++;
        }
    }
    c_data->uFirstSignificant = c_data->uSignificantMap[0];
}

void H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(
    COEFFSTYPE coeff[],
    Ipp32s ctxBlockCat,
    Ipp32s numcoeff,
    const Ipp32s* dec_single_scan,
    T_Block_CABAC_DataType* c_data)
{
    Ipp8u start_scan, end_scan, j;
    Ipp32s i;

    switch(numcoeff)
    {
        case 64:start_scan = 0;end_scan=63;break;
        case 16:start_scan = 0;end_scan=15;break;
        case 15:start_scan = 1;end_scan=15;break;
        case  8:start_scan = 0;end_scan= 7;break;
        case  4:start_scan = 0;end_scan= 3;break;
        default:
            VM_ASSERT(false);
            start_scan = 0;
            end_scan = 0;
            break;
    }

    c_data->uLastCoeff = end_scan; //dec_single_scan[end_scan]; By Burakov seems to be an error.
    c_data->uFirstCoeff = start_scan;//dec_single_scan[start_scan]; By Burakov seems to be an error.
    for (i=end_scan;i>=start_scan;i--)
    {
        Ipp32s coef=coeff[dec_single_scan[i]];
        if (coef)
        {
            end_scan = (Ipp8u)i;
            break;
        }
    }
    c_data->uLastSignificant = end_scan;
    for (i=start_scan;i<=end_scan;i++)
    {
        Ipp32s coef=coeff[dec_single_scan[i]];
        if (coef)
        {
            start_scan = (Ipp8u)i;
            break;
        }
    }
    c_data->uFirstSignificant = start_scan;

    for (i=c_data->uFirstSignificant,j=0;i<=c_data->uLastSignificant;i++)
    {
        Ipp32s coef=coeff[dec_single_scan[i]];
        if (coef)
        {
            Ipp32s sign = coef < 0;
            coef = (sign)? -coef : coef;
            c_data->uSignificantMap[j] = (Ipp8u)i;
            c_data->uSignificantLevels[j] = coef - 1;
            c_data->uSignificantSigns[j] = (Ipp8u)sign;
            j++;
        }
    }

    c_data->uNumSigCoeffs = j;
    c_data->CtxBlockCat = ctxBlockCat;
}


void H264ENC_MAKE_NAME(H264CoreEncoder_InferFDFForSkippedMBs)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;

    //inherit field decoding flag
    if (cur_mb.MacroblockNeighbours.mb_A>=0)
    {
        pSetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo,GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_A]));
    }
    else if (cur_mb.MacroblockNeighbours.mb_B>=0)
    {
        pSetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo,GetMBFieldDecodingFlag(core_enc->m_pCurrentFrame->m_mbinfo.mbs[cur_mb.MacroblockNeighbours.mb_B]));
    }
    else
    {
        pSetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo,0);
    }
    pSetMBFieldDecodingFlag(cur_mb.GlobalMacroblockPairInfo,pGetMBFieldDecodingFlag(cur_mb.GlobalMacroblockInfo));
}


#undef H264EncoderFrameType
#undef T_Block_CABAC_DataType
#undef H264CurrentMacroblockDescriptorType
#undef H264SliceType
#undef H264CoreEncoderType
#undef H264BsBaseType
#undef H264BsRealType
#undef H264BsFakeType
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
