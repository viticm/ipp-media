////               INTEL CORPORATION PROPRIETARY INFORMATION
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

#define H264CoreEncoderType H264ENC_MAKE_NAME(H264CoreEncoder)
#define H264SliceType H264ENC_MAKE_NAME(H264Slice)
#define H264CurrentMacroblockDescriptorType H264ENC_MAKE_NAME(H264CurrentMacroblockDescriptor)
#define T_Block_CABAC_DataType H264ENC_MAKE_NAME(T_Block_CABAC_Data)

#if (defined(TRACE_INTRA) || defined(TRACE_INTER) || defined(TRACE_INTRA_16x16))
#define printMatr(a, pitchA, m, n, rshift, name) { \
    fprintf(stderr,"Matrix \"%s\" {\n", name); \
    for(Ipp32s i = 0; i < (m); i++) { \
        fprintf(stderr,"   "); \
        for(Ipp32s j = 0; j < (n); j++) \
            fprintf(stderr," %3d,", (a)[i*(pitchA) + j] >> (rshift)); \
        fprintf(stderr,"\n"); \
    } \
    fprintf(stderr,"}\n"); \
}
#endif

////////////////////////////////////////////////////////////////////////////////
// CEncAndRec4x4IntraBlock
//
// Encode and Reconstruct one blocks in an Intra macroblock with 4x4 prediction
//
////////////////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME(H264CoreEncoder_Encode4x4IntraBlock)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s block)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s      iNumCoeffs=0;
    Ipp32s      iLastCoeff=0;
    __ALIGN16 Ipp16s pDiffBuf[16];
    COEFFSTYPE*  pTransformResult;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32u uMBQP       = cur_mb.lumaQP;
    Ipp32s pitchPixels = cur_mb.mbPitchPixels;
    Ipp32u uCBPLuma     = cur_mb.m_uIntraCBP4x4;
    PIXTYPE* pBlockData = cur_mb.mbPtr + xoff[block] + yoff[block]*pitchPixels;
    PIXTYPE* pPredBuf = cur_mb.mb4x4.prediction + xoff[block] + yoff[block]*16;
    PIXTYPE* pReconBuf = cur_mb.mb4x4.reconstruct + xoff[block] + yoff[block]*16;
    __ALIGN16 COEFFSTYPE pTransRes[16];

    pTransformResult = &cur_mb.mb4x4.transform[block*16];
    Diff4x4(pPredBuf, pBlockData, pitchPixels, pDiffBuf);
    if (!core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag || uMBQP != 0) {
        H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
            pDiffBuf,
            pTransformResult,
            uMBQP,
            &iNumCoeffs,
            1,
            enc_single_scan[curr_slice->m_is_cur_mb_field],
            &iLastCoeff,
            NULL,
            NULL,
            0,
            NULL); //Always use f for INTRA
        if (!iNumCoeffs) {
            Copy4x4(pPredBuf, 16, pReconBuf, 16);
            uCBPLuma &= ~CBP4x4Mask[block];
        } else {
            memcpy( pTransRes, pTransformResult, 16*sizeof( COEFFSTYPE ));
            H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                pPredBuf,
                pTransRes,
                NULL,
                pReconBuf,
                16,
                16,
                uMBQP,
                ((iNumCoeffs < -1) || (iNumCoeffs > 0)),
                core_enc->m_PicParamSet.bit_depth_luma,
                NULL);
        }
    } else {
        // Transform bypass => lossless.
        Copy4x4(pBlockData, pitchPixels, pReconBuf, 16);
        for( Ipp32s i = 0; i < 16; i++) pTransformResult[i] = pDiffBuf[i];
        ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan[curr_slice->m_is_cur_mb_field], &iLastCoeff, 16);
        if (iNumCoeffs == 0) {
            uCBPLuma &= ~CBP4x4Mask[block];
            Copy4x4(pBlockData, pitchPixels, pPredBuf, 16);
        }
    }
    cur_mb.m_iNumCoeffs4x4[ block ] = iNumCoeffs;
    cur_mb.m_iLastCoeff4x4[ block ] = iLastCoeff;
    cur_mb.m_uIntraCBP4x4 = uCBPLuma;
}

void H264ENC_MAKE_NAME(H264CoreEncoder_Encode8x8IntraBlock)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s block)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32s     iNumCoeffs;
    Ipp32s     iLastCoeff;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32u uMBQP       = cur_mb.lumaQP;
    Ipp32s pitchPixels = cur_mb.mbPitchPixels;

    PIXTYPE* pBlockData = cur_mb.mbPtr + xoff[4*block] + yoff[4*block]*pitchPixels;
    // loop over all 8x8 blocks in Y plane for the MB
    PIXTYPE* pPredBuf = cur_mb.mb8x8.prediction + xoff[block<<2] + yoff[block<<2]*16;
    PIXTYPE* pReconBuf = cur_mb.mb8x8.reconstruct + xoff[block<<2] + yoff[block<<2]*16;

    Ipp32u uCBPLuma     = cur_mb.m_uIntraCBP8x8;
    COEFFSTYPE* pTransformResult = &cur_mb.mb8x8.transform[block*64];
    __ALIGN16 Ipp16s pDiffBuf[64];
    __ALIGN16 COEFFSTYPE pTransRes[64];

    Diff8x8(pPredBuf, pBlockData, pitchPixels, pDiffBuf);
    if (!core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag || uMBQP != 0) {
        H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
            pTransformResult,
            pTransformResult,
            QP_DIV_6[uMBQP],
            1,
            enc_single_scan_8x8[curr_slice->m_is_cur_mb_field],
            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[0][QP_MOD_6[uMBQP]],
            &iNumCoeffs,
            &iLastCoeff,
            NULL,
            NULL,
            NULL);

        if (!iNumCoeffs) {
            Copy8x8(pPredBuf, 16, pReconBuf, 16);
            uCBPLuma &= ~CBP8x8Mask[block];
        } else {
            memcpy( pTransRes, pTransformResult, 64*sizeof( COEFFSTYPE ));
            H264ENC_MAKE_NAME(ippiQuantLuma8x8Inv_H264)(pTransRes, QP_DIV_6[uMBQP], core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[0][QP_MOD_6[uMBQP]]);
            H264ENC_MAKE_NAME(ippiTransformLuma8x8InvAddPred_H264)(pPredBuf, 16, pTransRes, pReconBuf, 16, core_enc->m_PicParamSet.bit_depth_luma);
        }
    } else {
        // Transform bypass => lossless.
        Copy8x8(pBlockData, pitchPixels, pReconBuf, 16);
        for (Ipp32s i = 0; i < 64; i++)
            pTransformResult[i] = pDiffBuf[i];
        ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan_8x8[curr_slice->m_is_cur_mb_field], &iLastCoeff, 64);
        if (iNumCoeffs == 0) {
            uCBPLuma &= ~CBP8x8Mask[block];
            Copy8x8(pBlockData, pitchPixels, pPredBuf, 16);
        }
    }
    cur_mb.m_iNumCoeffs8x8[ block ] = iNumCoeffs;
    cur_mb.m_iLastCoeff8x8[ block ] = iLastCoeff;
    cur_mb.m_uIntraCBP8x8 = uCBPLuma;
}

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantIntra16x16_RD)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks
    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s* pDiffBuf;       // difference block pointer
    Ipp16s* pTempDiffBuf;       // difference block pointer
    COEFFSTYPE *pTransformResult; // for transform results.
    Ipp16s* pMassDiffBuf;   // difference block pointer
    COEFFSTYPE* pQBuf;          // quantized block pointer
    Ipp32s  pitchPixels;     // buffer pitch in pixels
    Ipp8u   bCoded; // coded block flag
    Ipp32s  iNumCoeffs; // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s  iLastCoeff; // Number of nonzero coeffs after quant (negative if DC is nonzero)
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;

    pitchPixels = cur_mb.mbPitchPixels;
    uCBPLuma    = cur_mb.LocalMacroblockInfo->cbp_luma;
    uMBQP       = cur_mb.lumaQP;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pTransformResult = (COEFFSTYPE*)(pDiffBuf + 16);
    pQBuf       = (COEFFSTYPE*) (pTransformResult + 16);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf = (Ipp16s*) (pDCBuf + 16);
    uMB = cur_mb.uMB;

    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    // initialize pointers and offset
    pPredBuf    = cur_mb.mb16x16.prediction; // 16-byte aligned work buffer
    pReconBuf    = cur_mb.mb16x16.reconstruct; // 16-byte aligned work buffer
    Ipp32s pitchPix = 16;

    cur_mb.MacroblockCoeffsInfo->lumaAC = 0;
    H264ENC_MAKE_NAME(ippiSumsDiff16x16Blocks4x4)(cur_mb.mbPtr, pitchPixels, pPredBuf, 16, pDCBuf, pMassDiffBuf); // compute the 4x4 luma DC transform coeffs

    // apply second transform on the luma DC transform coeffs
    H264ENC_MAKE_NAME(ippiTransformQuantLumaDC_H264)(
        pDCBuf,
        pQBuf,
        uMBQP,
        &iNumCoeffs,
        1,
        enc_single_scan[is_cur_mb_field],&iLastCoeff,
        NULL);

    if (core_enc->m_PicParamSet.entropy_coding_mode){
        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[Y_DC_RLE];
        bCoded = c_data->uNumSigCoeffs /* = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]*/ = ABS(iNumCoeffs);
        c_data->uLastSignificant = iLastCoeff;
        c_data->CtxBlockCat = BLOCK_LUMA_DC_LEVELS;
        c_data->uFirstCoeff = 0;
        c_data->uLastCoeff = 15;
        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pDCBuf,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[Y_DC_RLE]);
    }else{
        H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(pDCBuf,0, dec_single_scan[is_cur_mb_field],iLastCoeff,
                                   &curr_slice->Block_RLE[Y_DC_RLE].uTrailing_Ones,
                                   &curr_slice->Block_RLE[Y_DC_RLE].uTrailing_One_Signs,
                                   &curr_slice->Block_RLE[Y_DC_RLE].uNumCoeffs,
                                   &curr_slice->Block_RLE[Y_DC_RLE].uTotalZeros,
                                   curr_slice->Block_RLE[Y_DC_RLE].iLevels,
                                   curr_slice->Block_RLE[Y_DC_RLE].uRuns);
        bCoded = curr_slice->Block_RLE[Y_DC_RLE].uNumCoeffs;
    }

    H264ENC_MAKE_NAME(ippiTransformDequantLumaDC_H264)(
        pDCBuf,
        uMBQP,
        NULL);

    // loop over all 4x4 blocks in Y plane for the MB
    for (uBlock = 0; uBlock < 16; uBlock++ ){
        pPredBuf = cur_mb.mb16x16.prediction + xoff[uBlock] + yoff[uBlock]*16;
        pReconBuf = cur_mb.mb16x16.reconstruct + xoff[uBlock] + yoff[uBlock]*16;

        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // This will be updated if the block is coded
        if (core_enc->m_PicParamSet.entropy_coding_mode) {
            curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        } else {
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
        }

        bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0)); // check if block is coded

        if (!bCoded){
            Copy4x4(pPredBuf, 16, pReconBuf, pitchPix); // update reconstruct frame for the empty block
        }else{   // block not declared empty, encode
            pTempDiffBuf = pMassDiffBuf+ xoff[uBlock]*4 + yoff[uBlock]*16;
            H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                pTempDiffBuf,
                pTransformResult,
                uMBQP,
                &iNumCoeffs,
                1,
                enc_single_scan[is_cur_mb_field],
                &iLastCoeff,
                NULL,
                NULL,
                0,
                NULL); //Always use f for INTRA

            cur_mb.MacroblockCoeffsInfo->lumaAC |= ((iNumCoeffs < -1) || (iNumCoeffs > 0));

            if (!iNumCoeffs){
                bCoded = 0;
            } else {
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    c_data->uLastSignificant = iLastCoeff;
                    c_data->CtxBlockCat = BLOCK_LUMA_AC_LEVELS;
                    c_data->uFirstCoeff = 1;
                    c_data->uLastCoeff = 15;
                    H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult, dec_single_scan[is_cur_mb_field], &curr_slice->Block_CABAC[uBlock]);
                    bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                } else {
                    H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(pTransformResult, 1, dec_single_scan[is_cur_mb_field], iLastCoeff,
                                               &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                                               &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                                               &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                                               &curr_slice->Block_RLE[uBlock].uTotalZeros,
                                               curr_slice->Block_RLE[uBlock].iLevels,
                                               curr_slice->Block_RLE[uBlock].uRuns);
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = bCoded = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                }
            }

            if (!bCoded) uCBPLuma &= ~CBP4x4Mask[uBlock];

            // If the block wasn't coded and the DC coefficient is zero
            if (!bCoded && !pDCBuf[block_subblock_mapping[uBlock]]){
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
            } else {
                H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                    pPredBuf,
                    pTransformResult,
                    &pDCBuf[block_subblock_mapping[uBlock]],
                    pReconBuf,
                    16,
                    pitchPix,
                    uMBQP,
                    ((iNumCoeffs < -1) || (iNumCoeffs > 0)),
                    core_enc->m_PicParamSet.bit_depth_luma,
                    NULL);
            }
        }
    }

    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;
    if (cur_mb.MacroblockCoeffsInfo->lumaAC > 1)
        cur_mb.MacroblockCoeffsInfo->lumaAC = 1;

}

void H264ENC_MAKE_NAME(H264CoreEncoder_EncodeChroma)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;         // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    PIXTYPE*  pSrcPlane;    // start of plane to encode
    Ipp32s    pitchPixels;  // buffer pitch
    COEFFSTYPE *pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;     // prediction block pointer
    PIXTYPE*  pReconBuf;     // prediction block pointer
    PIXTYPE*  pPredBuf_copy;     // prediction block pointer
    PIXTYPE*  pReconBuf_copy;     // prediction block pointer
    COEFFSTYPE* pQBuf;      // quantized block pointer
    Ipp16s* pMassDiffBuf;   // difference block pointer
    Ipp32u   uCBPChroma;    // coded flags for all chroma blocks
    Ipp32s   bCoded;        // coded block flag
    Ipp32s   iNumCoeffs = 0;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   iLastCoeff;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   RLE_Offset;    // Index into BlockRLE array

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    COEFFSTYPE *pTransformResult;
    COEFFSTYPE *pTransform;
    Ipp32s QPy = cur_mb.lumaQP;
//    __ALIGN16 CabacStates cbSt;

    pitchPixels = cur_mb.mbPitchPixels;
    uMBQP       = cur_mb.chromaQP;
    pTransform = (COEFFSTYPE*)curr_slice->m_pMBEncodeBuffer;
    pQBuf       = (COEFFSTYPE*) (pTransform + 64*2);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    Ipp16s*  pTempDiffBuf;
    Ipp32u  uMB = cur_mb.uMB;
    // initialize pointers and offset
    uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    bool transform_bypass = core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag && QPy == 0;
    uCBPChroma  = cur_mb.LocalMacroblockInfo->cbp_chroma;
    bool intra = (cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTRA) || (cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTRA_16x16);
    if (intra) {
        pPredBuf = cur_mb.mbChromaIntra.prediction;
        pReconBuf = cur_mb.mbChromaIntra.reconstruct;
        if(!((core_enc->m_Analyse & ANALYSE_RD_OPT) || (core_enc->m_Analyse & ANALYSE_RD_MODE))){
            cur_mb.MacroblockCoeffsInfo->chromaNC = 0;
            H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectChromaMBs_8x8)(
                state,
                curr_slice,
                core_enc->m_pCurrentFrame->m_pUPlane + uOffset,
                core_enc->m_pReconstructFrame->m_pUPlane + uOffset,
                core_enc->m_pCurrentFrame->m_pVPlane + uOffset,
                core_enc->m_pReconstructFrame->m_pVPlane + uOffset,
                pitchPixels,
                &cur_mb.LocalMacroblockInfo->intra_chroma_mode,
                pPredBuf,
                pPredBuf+8);  //up to 422 only
        }
    } else {
        cur_mb.MacroblockCoeffsInfo->chromaNC = 0;
        pPredBuf = cur_mb.mbChromaInter.prediction;
        pReconBuf = cur_mb.mbChromaInter.reconstruct;
        H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBChroma)(state, curr_slice, pPredBuf);
    }
    // initialize pointers for the U plane blocks
    Ipp32s num_blocks = 2 << core_enc->m_PicParamSet.chroma_format_idc;
    Ipp32s startBlock;
    startBlock = uBlock = 16;
    Ipp32u uLastBlock = uBlock+num_blocks;
    Ipp32u uFinalBlock = uBlock+2*num_blocks;
    do
    {
        if (uBlock == uLastBlock) {
            startBlock = uBlock;
            uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
            pSrcPlane = core_enc->m_pCurrentFrame->m_pVPlane;
            pPredBuf -= 60;
            pReconBuf = core_enc->m_pReconstructFrame->m_pVPlane+uOffset;
            RLE_Offset = V_DC_RLE;
            uLastBlock += num_blocks;
        } else {
            RLE_Offset = U_DC_RLE;
            pSrcPlane = core_enc->m_pCurrentFrame->m_pUPlane;
            pReconBuf = core_enc->m_pReconstructFrame->m_pUPlane+uOffset;
        }
        //TODO add for 422
        H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(pSrcPlane + uOffset, pitchPixels, pPredBuf, 16, pDCBuf, pMassDiffBuf);
        if (core_enc->m_PicParamSet.chroma_format_idc == 2)
             H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(pSrcPlane + uOffset+8*pitchPixels, pitchPixels, pPredBuf+8*16, 16, pDCBuf+4, pMassDiffBuf+64);
        // Code chromaDC
        if (!transform_bypass)  {
            switch (core_enc->m_PicParamSet.chroma_format_idc) {
                case 1:
                    H264ENC_MAKE_NAME(ippiTransformQuantChromaDC_H264)(pDCBuf, pQBuf, uMBQP, &iNumCoeffs, (slice_type == INTRASLICE), 1, NULL);
                    break;
                case 2:
                    H264ENC_MAKE_NAME(ippiTransformQuantChroma422DC_H264)(pDCBuf, pQBuf, uMBQP, &iNumCoeffs, (slice_type == INTRASLICE), 1, NULL);
                    break;
                default:
                    break;
            }
        } else {
            Ipp32s i,j;
            Ipp32s num_rows, num_cols;
            Ipp32s bPitch;
            num_cols = ((core_enc->m_PicParamSet.chroma_format_idc - 1) & 0x2) ? 4 : 2;
            num_rows = (core_enc->m_PicParamSet.chroma_format_idc & 0x2) ? 4 : 2;
            bPitch = num_cols * 16;
            for(i = 0; i < num_rows; i++) {
                for(j = 0; j < num_cols; j++) {
                    pDCBuf[i*num_cols+j] = pMassDiffBuf[i*bPitch + j*16];
                }
            }
            ippiCalcNonZero(pDCBuf, num_blocks, &iNumCoeffs);
        }
        // DC values in this block if iNonEmpty is 1.
        cur_mb.MacroblockCoeffsInfo->chromaNC |= (iNumCoeffs != 0);
        // record RLE info
        if (core_enc->m_PicParamSet.entropy_coding_mode){
            Ipp32s ctxIdxBlockCat = BLOCK_CHROMA_DC_LEVELS;
            switch (core_enc->m_PicParamSet.chroma_format_idc) {
                case 1:
                    H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,4,dec_single_scan_p,&curr_slice->Block_CABAC[RLE_Offset]);
                    break;
                case 2:
                    H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,8,dec_single_scan_p422,&curr_slice->Block_CABAC[RLE_Offset]);
                    break;
                default:
                    break;
            }
       }else{
            switch( core_enc->m_PicParamSet.chroma_format_idc ){
                case 1:
                   H264ENC_MAKE_NAME(ippiEncodeChromaDcCoeffsCAVLC_H264)(
                       pDCBuf,
                       &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                       &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                       &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                       &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                       curr_slice->Block_RLE[RLE_Offset].iLevels,
                       curr_slice->Block_RLE[RLE_Offset].uRuns);
                    break;
                case 2:
                    H264ENC_MAKE_NAME(ippiEncodeChroma422DC_CoeffsCAVLC_H264)(
                        pDCBuf,
                        &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                        &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                        &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                        &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                        curr_slice->Block_RLE[RLE_Offset].iLevels,
                        curr_slice->Block_RLE[RLE_Offset].uRuns);
                    break;
            }
        }
        // Inverse transform and dequantize for chroma DC
        if (!transform_bypass ){
            switch( core_enc->m_PicParamSet.chroma_format_idc ){
             case 1:
                 H264ENC_MAKE_NAME(ippiTransformDequantChromaDC_H264)(pDCBuf, uMBQP, NULL);
                 break;
             case 2:
                 H264ENC_MAKE_NAME(ippiTransformDequantChromaDC422_H264)(pDCBuf, uMBQP, NULL);
                 break;
            default:
                break;
            }
        }
//Encode croma AC
        Ipp32s coeffsCost = 0;
        pPredBuf_copy = pPredBuf;
        pReconBuf_copy = pReconBuf;
        for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;     // This will be updated if the block is coded
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
            } else {
                curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
            }
            // check if block is coded
            bCoded = ((uCBPChroma & CBP4x4Mask[uBlock-16])?(1):(0));
            if (!bCoded){ // update reconstruct frame for the empty block
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPixels);
            } else {   // block not declared empty, encode
                pTempDiffBuf = pMassDiffBuf + (uBlock-startBlock)*16;
                pTransformResult = pTransform + (uBlock-startBlock)*16;
                if(!transform_bypass) {
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                         pTempDiffBuf,
                         pTransformResult,
                         uMBQP,
                         &iNumCoeffs,
                         0,
                         enc_single_scan[is_cur_mb_field],
                         &iLastCoeff,
                         NULL,
                         NULL,
                         9,
                         NULL);//,NULL, curr_slice, 1, &cbSt);
                    coeffsCost += H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 15, &dec_single_scan[is_cur_mb_field][1]);
                 }else {
                    for(Ipp32s i = 0; i < 16; i++) {
                        pTransformResult[i] = pTempDiffBuf[i];
                    }
                    ippiCountCoeffs(pTempDiffBuf, &iNumCoeffs, enc_single_scan[is_cur_mb_field], &iLastCoeff, 16);
                }
                // if everything quantized to zero, skip RLE
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                if (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]){  // the block is empty so it is not coded
                   if (core_enc->m_PicParamSet.entropy_coding_mode){
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock];
                        c_data->CtxBlockCat = BLOCK_CHROMA_AC_LEVELS;
                        c_data->uFirstCoeff = 1;
                        c_data->uLastCoeff = 15;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                    } else {
                        H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                            pTransformResult,// pDiffBuf,
                            1,
                            dec_single_scan[is_cur_mb_field],
                            iLastCoeff,
                            &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                            &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                            &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                            &curr_slice->Block_RLE[uBlock].uTotalZeros,
                            curr_slice->Block_RLE[uBlock].iLevels,
                            curr_slice->Block_RLE[uBlock].uRuns);

                        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                    }
                }
            }
            pPredBuf += chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock]; //!!!
            pReconBuf += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
        }

        if(!transform_bypass && coeffsCost <= (CHROMA_COEFF_MAX_COST<<(core_enc->m_PicParamSet.chroma_format_idc-1)) ){ //Reset all ac coeffs
//           if(cur_mb.MacroblockCoeffsInfo->chromaNC&1) //if we have DC coeffs
//           memset( pTransform, 0, (64*sizeof(COEFFSTYPE))<<(core_enc->m_PicParamSet.chroma_format_idc-1));
           for(uBlock = startBlock; uBlock < uLastBlock; uBlock++){
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                   curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
                } else {
                    curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                    curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
                }
            }
        }

        pPredBuf = pPredBuf_copy;
        pReconBuf = pReconBuf_copy;
        for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
            cur_mb.MacroblockCoeffsInfo->chromaNC |= 2*(cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0);
            if (!cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] && !pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ]){
                uCBPChroma &= ~CBP4x4Mask[uBlock-16];
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPixels);
            }else if(!transform_bypass){
                    H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                        pPredBuf,
                        pTransform + (uBlock-startBlock)*16,
                        &pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ],
                        pReconBuf,
                        16,
                        pitchPixels,
                        uMBQP,
                        (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0),
                        core_enc->m_SeqParamSet.bit_depth_chroma,
                        NULL);
            }
            pPredBuf += chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock]; //!!!
            pReconBuf += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
        }   // for uBlock in chroma plane
    } while (uBlock < uFinalBlock);
    //Reset other chroma
    uCBPChroma &= ~(0xffffffff<<(uBlock-16));
    cur_mb.LocalMacroblockInfo->cbp_chroma = uCBPChroma;
    if (cur_mb.MacroblockCoeffsInfo->chromaNC == 3)
        cur_mb.MacroblockCoeffsInfo->chromaNC = 2;
}

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec4x4IntraMB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMBType;        // current MB type
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks
    Ipp32u  uIntraSAD;      // intra MB SAD
    Ipp16s* pMassDiffBuf;   // difference block pointer

    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s*   pDiffBuf;       // difference block pointer
    COEFFSTYPE* pTransformResult; // Result of the transformation.
    COEFFSTYPE* pQBuf;          // quantized block pointer
    PIXTYPE*  pSrcPlane;      // start of plane to encode
    Ipp32s    pitchPixels;     // buffer pitch
    Ipp32s    iMBCost;        // recode MB cost counter
    Ipp32s    iBlkCost[2];    // coef removal counter for left/right 8x8 luma blocks
    Ipp8u     bCoded; // coded block flag
    Ipp32s    iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s    iLastCoeff;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32u    uTotalCoeffs = 0;    // Used to detect single expensive coeffs.

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    __ALIGN16 CabacStates cbSt;

    uMB = cur_mb.uMB;

    pitchPixels = cur_mb.mbPitchPixels;
    uCBPLuma     = cur_mb.LocalMacroblockInfo->cbp_luma;
    uMBQP       = cur_mb.lumaQP;
    uMBType     = cur_mb.GlobalMacroblockInfo->mbtype;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pTransformResult = (COEFFSTYPE*)(pDiffBuf + 64);
    pQBuf       = (COEFFSTYPE*) (pTransformResult + 64);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
//  uIntraSAD   = rd_quant_intra[uMBQP] * 24;   // TODO ADB 'handicap' using reconstructed data
    uIntraSAD   = 0;
    iMBCost     = 0;
    iBlkCost[0] = 0;
    iBlkCost[1] = 0;

    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    // initialize pointers and offset
    pSrcPlane = core_enc->m_pCurrentFrame->m_pYPlane;
    uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    bool transform_bypass = core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag && uMBQP == 0;

    Ipp32s pitchPix;
    pitchPix = pitchPixels;

#if defined (TRACE_INTRA)
    if(uMB == TRACE_INTRA) {
        fprintf(stderr,"ermb intra: uMB = %d, tr_bypass = %d, uCBPLuma = 0x%x\n", uMB, transform_bypass, uCBPLuma);
        printMatr(pSrcPlane + uOffset, pitchPixels, 16, 16, 0, "SrcPlane");
        if(pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo))
            printMatr(cur_mb.mb8x8.prediction, 16, 16, 16, 0, "PredBuf")
        else
            printMatr(cur_mb.mb4x4.prediction, 16, 16, 16, 0, "PredBuf");
    }
#endif // TRACE_INTRA
    if(pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo)) {

    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA8x8 ){
        uCBPLuma = 0xffff;
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[402], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[417], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[402];
            cbSt.last = &curr_slice->m_pbitstream->context_array[417];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[436], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[451], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[436];
            cbSt.last = &curr_slice->m_pbitstream->context_array[451];
        }
    }
        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 1);
#ifdef ALT_RC
        cur_mb.LocalMacroblockInfo->sad = SAD16x16( pSrcPlane + uOffset, pitchPixels*sizeof(PIXTYPE), cur_mb.mb8x8.prediction, 16*sizeof(PIXTYPE));
        curr_slice->m_Sad_Sum += cur_mb.LocalMacroblockInfo->sad;
#endif
        //loop over all 8x8 blocks in Y plane for the MB
        for (uBlock = 0; uBlock < 4; uBlock ++){
            Ipp32s idxb, idx, idxe;

            idxb = uBlock<<2;
            idxe = idxb+4;
            pPredBuf = cur_mb.mb8x8.prediction + xoff[4*uBlock] + yoff[4*uBlock]*16;
            pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
                curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
            } else {
                for( idx = idxb; idx<idxe; idx++ ){
                    curr_slice->Block_RLE[idx].uNumCoeffs = 0;
                    curr_slice->Block_RLE[idx].uTrailing_Ones = 0;
                    curr_slice->Block_RLE[idx].uTrailing_One_Signs = 0;
                    curr_slice->Block_RLE[idx].uTotalZeros = 16;
                    cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = 0;
               }
            }

            if (!curr_slice->m_use_transform_for_intra_decision){
                uIntraSAD += H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneMB_8x8)(
                    state,
                    curr_slice,
                    pSrcPlane + uOffset,
                    pReconBuf,
                    uBlock,
                    cur_mb.intra_types,
                    pPredBuf);
            }

            // check if block is coded
            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));

            if (!bCoded){  // update reconstruct frame for the empty block
                Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
            } else {   // block not declared empty, encode
                // compute difference of predictor and source pels
                // note: asm version does not use pDiffBuf
                //       output is being passed in the mmx registers
            if (!curr_slice->m_use_transform_for_intra_decision ||  core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA8x8){
                Diff8x8(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                if(!transform_bypass) {
                    if(  core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA8x8  ){
                        __ALIGN16 PIXTYPE pred[64];
                        PIXTYPE pred_pels[25]; //Sources for prediction
                        Ipp32u pred_pels_mask = 0;
                        Ipp32s i;
                        bool top_avbl;
                        bool left_avbl;
                        bool left_above_avbl = curr_slice->m_cur_mb.BlockNeighbours.mb_above_left.mb_num >= 0;
                        bool right_above_avbl = curr_slice->m_cur_mb.BlockNeighbours.mb_above_right.mb_num >= 0;

                        if( uBlock & 0x2 ){
                            top_avbl = true;
                        }else{
                            top_avbl = curr_slice->m_cur_mb.BlockNeighbours.mb_above.mb_num >= 0;
                        }

                        if( uBlock & 0x1 ){
                            left_avbl = true;
                        }else{
                            left_avbl = curr_slice->m_cur_mb.BlockNeighbours.mbs_left[0].mb_num >= 0;
                        }

                        //Copy pels
                        //TOP
                        if( top_avbl ){
                            for( i=0; i<8; i++ )
                                pred_pels[1+i] = *(pReconBuf-pitchPixels+i);
                            pred_pels_mask |= 0x000001fe;
                        }

                        //LEFT
                        if( left_avbl ){
                            for( i=0; i<8; i++ )
                                pred_pels[17+i] = *(pReconBuf+i*pitchPixels-1);
                            pred_pels_mask |= 0x1fe0000;
                        }

                        //LEFT_ABOVE
                        if( (uBlock == 0 && left_above_avbl) || uBlock == 3 ||
                            (uBlock == 1 && top_avbl) || ( uBlock == 2 && left_avbl)){
                                pred_pels[0] = *(pReconBuf-pitchPixels-1);
                            pred_pels_mask |= 0x01;
                        }

                        //RIGHT_ABOVE
                        if( (uBlock == 2) || (uBlock == 0 && top_avbl) ||
                            (uBlock == 1 && right_above_avbl) ){
                            for( i=0; i<8; i++ )
                                pred_pels[9+i] = *(pReconBuf-pitchPixels+i+8);
                            pred_pels_mask |= 0x0001fe00;
                        }

                        if( !((pred_pels_mask & 0x0001FE00)==0x0001FE00) && (pred_pels_mask & 0x00000100) ){
                            pred_pels_mask |= 0x0001FE00;
                            for( i=0; i<8; i++ ) pred_pels[9+i] = pred_pels[1+7];
                        }

                        H264ENC_MAKE_NAME(H264CoreEncoder_Filter8x8Pels)(pred_pels, pred_pels_mask);
                        H264ENC_MAKE_NAME(H264CoreEncoder_GetPrediction8x8)(state, cur_mb.intra_types[uBlock<<2], pred_pels, pred_pels_mask, pred );
                        PIXTYPE* p = pPredBuf;
                        for( i=0; i<8; i++){
                            memcpy(p, &pred[i*8], 8*sizeof(PIXTYPE));
                            p += 16; //pitch = 16
                        }
                        Diff8x8(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                        H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                            pTransformResult,
                            pTransformResult,
                            uMBQP,
                            1,
                            enc_single_scan_8x8[is_cur_mb_field],
                            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[0][QP_MOD_6[uMBQP]], //Use scaling matrix for INTRA
                            &iNumCoeffs,
                            &iLastCoeff,
                            curr_slice,
                            &cbSt,
                            core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[0][QP_MOD_6[uMBQP]]);
                    }else{
                    // forward transform and quantization, in place in pDiffBuf
                        H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                            pTransformResult,
                            pTransformResult,
                            QP_DIV_6[uMBQP],
                            1,
                            enc_single_scan_8x8[is_cur_mb_field],
                            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[0][QP_MOD_6[uMBQP]], //Use scaling matrix for INTRA
                            &iNumCoeffs,
                            &iLastCoeff,
                            NULL,
                            NULL,
                            NULL);
                    }
                }
                else {
                    for(Ipp32s i = 0; i < 64; i++) {
                        pTransformResult[i] = pDiffBuf[i];
                    }
                    ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan_8x8[is_cur_mb_field], &iLastCoeff, 64);
                }
            }else{
                    iNumCoeffs = cur_mb.m_iNumCoeffs8x8[ uBlock ];
                    iLastCoeff = cur_mb.m_iLastCoeff8x8[ uBlock ];
                    pTransformResult = &cur_mb.mb8x8.transform[ uBlock*64 ];
            }

                // if everything quantized to zero, skip RLE
                if (!iNumCoeffs ){ // the block is empty so it is not coded
                    bCoded = 0;
                } else {
                    uTotalCoeffs += ((iNumCoeffs < 0) ? -(iNumCoeffs*2) : iNumCoeffs);

                    // record RLE info
                    if (core_enc->m_PicParamSet.entropy_coding_mode){
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_64_LEVELS;
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 63;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan_8x8[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                        bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                    }else{
                        COEFFSTYPE buf4x4[4][16];
                        Ipp32s i4x4;

                        //Reorder 8x8 block for coding with CAVLC
                        for(i4x4=0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            for(i = 0; i<16; i++ )
                                buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] =
                                    pTransformResult[dec_single_scan_8x8[is_cur_mb_field][4*i+i4x4]];
                        }

                        bCoded = 0;
                        //Encode each block with CAVLC 4x4
                        for(i4x4 = 0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            iLastCoeff = 0;
                            idx = idxb + i4x4;

                            //Check for last coeff
                            for(i = 0; i<16; i++ ) if( buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] != 0 ) iLastCoeff=i;

                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                                buf4x4[i4x4],
                                0, //Luma
                                dec_single_scan[is_cur_mb_field],
                                iLastCoeff,
                                &curr_slice->Block_RLE[idx].uTrailing_Ones,
                                &curr_slice->Block_RLE[idx].uTrailing_One_Signs,
                                &curr_slice->Block_RLE[idx].uNumCoeffs,
                                &curr_slice->Block_RLE[idx].uTotalZeros,
                                curr_slice->Block_RLE[idx].iLevels,
                                curr_slice->Block_RLE[idx].uRuns);

                            bCoded += curr_slice->Block_RLE[idx].uNumCoeffs;
                            cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = curr_slice->Block_RLE[idx].uNumCoeffs;
                         }
                    }
                }

            // update flags if block quantized to empty
            if (curr_slice->m_use_transform_for_intra_decision && core_enc->m_info.quant_opt_level < OPT_QUANT_INTRA8x8 + 1 ){
                if (!bCoded){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                    //Copy  prediction
                    Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                }else //Copy reconstruct
                    Copy8x8(pPredBuf + 256, 16, pReconBuf, pitchPix);
            }else{
                // update flags if block quantized to empty
                if (!bCoded){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                    // update reconstruct frame with prediction
                    Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                }else if(!transform_bypass){
                    // inverse transform for reconstruct AND...
                    // add inverse transformed coefficients to original predictor
                    // to obtain reconstructed block, store in reconstruct frame
                    // buffer
                    if(iNumCoeffs != 0) {
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8Inv_H264)(pTransformResult, QP_DIV_6[uMBQP], core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[0][QP_MOD_6[uMBQP]]);
                        H264ENC_MAKE_NAME(ippiTransformLuma8x8InvAddPred_H264)(pPredBuf, 16, pTransformResult, pReconBuf, pitchPix, core_enc->m_PicParamSet.bit_depth_luma);
                    }
                } else {
                    // Transform bypass => lossless
                    // RecPlane == SrcPlane => there is no need to copy.
                }
            }   // block not declared empty
            } //curr_slice->m_use_transform_for_intra_decision
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }  // for uBlock in luma plane
    }else{
#ifdef ALT_RC
    cur_mb.LocalMacroblockInfo->sad = SAD16x16( pSrcPlane + uOffset, pitchPixels*sizeof(PIXTYPE), cur_mb.mb4x4.prediction, 16*sizeof(PIXTYPE));
    curr_slice->m_Sad_Sum += cur_mb.LocalMacroblockInfo->sad;
#endif
    // loop over all 4x4 blocks in Y plane for the MB
    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA4x4 ){
        uCBPLuma = 0xffff;
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+20], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+29];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+172+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+172+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+172+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+172+29];
        }
     }

    for (uBlock = 0; uBlock < 16; uBlock++ ){
        pPredBuf = cur_mb.mb4x4.prediction + xoff[uBlock] + yoff[uBlock]*16;
        pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0; // These will be updated if the block is coded
        if (core_enc->m_PicParamSet.entropy_coding_mode){
            curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        }else{
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 16;
        }

        // find advanced intra prediction block, store in PredBuf
        // Select best AI mode for the block, using reconstructed
        // predictor pels. This function also stores the block
        // predictor pels at pPredBuf.
        if (!curr_slice->m_use_transform_for_intra_decision){
            uIntraSAD += H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneBlock)(
                state,
                curr_slice,
                pSrcPlane + uOffset,
                pReconBuf,
                uBlock,
                cur_mb.intra_types,
                pPredBuf);
        }

        // check if block is coded
        bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));

        if (!bCoded){
            // update reconstruct frame for the empty block
            Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
        } else {   // block not declared empty, encode
            // compute difference of predictor and source pels
            // note: asm version does not use pDiffBuf
            //       output is being passed in the mmx registers
          if (!curr_slice->m_use_transform_for_intra_decision || core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA4x4 ){
            Diff4x4(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
#if defined (TRACE_INTRA)
            if(uMB == TRACE_INTRA) {
                printf("uBlock = %d\n", uBlock);
                printMatr(pSrcPlane + uOffset, pitchPixels, 4, 4, 0, "Src B");
                printMatr(pPredBuf, 16, 4, 4, 0, "PredBuf B");
                printMatr(pDiffBuf, 4, 4, 4, 0, "pDiffBuf");
            }
#endif // TRACE_INTRA

            if(!transform_bypass) {
                if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA4x4 ){
                    PIXTYPE PredPel[13];
                    //We need to calculate new prediction
                    H264ENC_MAKE_NAME(H264CoreEncoder_GetBlockPredPels)(state, curr_slice, pReconBuf, pitchPixels, pReconBuf, pitchPixels, pReconBuf, pitchPixels, uBlock, PredPel);
                    H264ENC_MAKE_NAME(H264CoreEncoder_GetPredBlock)(cur_mb.intra_types[uBlock], pPredBuf, PredPel);
                    Diff4x4(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);

                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs,
                        1, //Always use f for INTRA
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        curr_slice,
                        0,
                        &cbSt);
                }else{
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs,
                        1, //Always use f for INTRA
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        NULL,
                        0,
                        NULL);
                }
            }else{
                for(Ipp32s i = 0; i < 16; i++) {
                    pTransformResult[i] = pDiffBuf[i];
                }
                ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan[is_cur_mb_field],&iLastCoeff, 16);
            }
          }else{
              iNumCoeffs = cur_mb.m_iNumCoeffs4x4[ uBlock ];
              iLastCoeff = cur_mb.m_iLastCoeff4x4[ uBlock ];
              pTransformResult = &cur_mb.mb4x4.transform[ uBlock*16 ];
          }
#if defined (TRACE_INTRA)
            if(uMB == TRACE_INTRA) {
                fprintf(stderr,"iNumCoeffs = %d, iLastCoeff = %d\n", iNumCoeffs, iLastCoeff);
                printMatr(pTransformResult, 16, 1, 16, 0, "pTransformResult");
            }
#endif // TRACE_INTRA
            // if everything quantized to zero, skip RLE
            if (!iNumCoeffs){
                // the block is empty so it is not coded
                bCoded = 0;
            } else {
                // Preserve the absolute number of coeffs.
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
//                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                    c_data->uLastSignificant = iLastCoeff;
                    c_data->CtxBlockCat = BLOCK_LUMA_LEVELS;
                    c_data->uFirstCoeff = 0;
                    c_data->uLastCoeff = 15;
                    H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                } else {
                // record RLE info
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                    H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                        pTransformResult,
                        0,
                        dec_single_scan[is_cur_mb_field],
                        iLastCoeff,
                        &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                        &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                        &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                        &curr_slice->Block_RLE[uBlock].uTotalZeros,
                        curr_slice->Block_RLE[uBlock].iLevels,
                        curr_slice->Block_RLE[uBlock].uRuns);
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = bCoded = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                }
            }

            // update flags if block quantized to empty
            if (curr_slice->m_use_transform_for_intra_decision && core_enc->m_info.quant_opt_level < OPT_QUANT_INTRA4x4+1 ) {
                if (!bCoded) {
                    uCBPLuma &= ~CBP4x4Mask[uBlock]; //Copy predition
                    Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                }else //Copy reconstruct
                    Copy4x4(pPredBuf + 256, 16, pReconBuf, pitchPix);
            } else {
                if (!bCoded){
                    uCBPLuma &= ~CBP4x4Mask[uBlock];

                    // update reconstruct frame for the empty block
                    Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                } else if(!transform_bypass) {
                    // inverse transform for reconstruct AND...
                    // add inverse transformed coefficients to original predictor
                    // to obtain reconstructed block, store in reconstruct frame
                    // buffer
                    H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                        pPredBuf,
                        pTransformResult,
                        NULL,
                        pReconBuf,
                        16,
                        pitchPix,
                        uMBQP,
                        ((iNumCoeffs < -1) || (iNumCoeffs > 0)),
                        core_enc->m_PicParamSet.bit_depth_luma,
                        NULL);

                }
            }
        }   // block not declared empty

        // proceed to the next block
        uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
    }  // for uBlock in luma plane
    }
#if defined (TRACE_INTRA)
    if(uMB == TRACE_INTRA) {
        printMatr(core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field], pitchPixels, 16, 16, 0, "RecPlane");
    }
#endif // TRACE_INTRA

    // update coded block flags
    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;

    // for each block of the MB initialize the AI mode (for INTER MB)
    // or motion vectors (for INTRA MB) to values which will be
    // correct predictors of that type. MV and AI mode prediction
    // depend upon this instead of checking MB type.

    return 1;
}   // CEncAndRec4x4IntraMB

////////////////////////////////////////////////////////////////////////////////
// CEncAndRec16x16IntraMB
//
// Encode and Reconstruct all blocks in one 16x16 Intra macroblock
//
////////////////////////////////////////////////////////////////////////////////

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec16x16IntraMB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks
    Ipp32u  uCBPChroma;        // coded flags for all chroma blocks
    Ipp32u  uIntraSAD;      // intra MB SAD
    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s* pDiffBuf;       // difference block pointer
    Ipp16s* pTempDiffBuf;       // difference block pointer
    COEFFSTYPE *pTransformResult; // for transform results.
    Ipp16s* pMassDiffBuf;   // difference block pointer
    COEFFSTYPE* pQBuf;          // quantized block pointer
    PIXTYPE*  pSrcPlane;      // start of plane to encode
    Ipp32s  pitchPixels;     // buffer pitch in pixels
    Ipp8u   bCoded; // coded block flag
    Ipp32s  iNumCoeffs; // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s  iLastCoeff; // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32u  RLE_Offset;    // Index into BlockRLE array
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;

    pitchPixels = cur_mb.mbPitchPixels;
    uCBPLuma    = cur_mb.LocalMacroblockInfo->cbp_luma;
    uCBPChroma  = cur_mb.LocalMacroblockInfo->cbp_chroma;
    uMBQP       = cur_mb.lumaQP;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pTransformResult = (COEFFSTYPE*)(pDiffBuf + 16);
    pQBuf       = (COEFFSTYPE*) (pTransformResult + 16);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf = (Ipp16s*) (pDCBuf + 16);
//  uIntraSAD   = rd_quant_intra[uMBQP] * 24;   // 'handicap' using reconstructed data
    uIntraSAD   = 0;
    uMB = cur_mb.uMB;

    bool transform_bypass = core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag && uMBQP == 0;
    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    // initialize pointers and offset
    pSrcPlane = core_enc->m_pCurrentFrame->m_pYPlane;
    uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    RLE_Offset = Y_DC_RLE;  // Used in 16x16 Intra mode only
    pPredBuf    = cur_mb.mb16x16.prediction; // 16-byte aligned work buffer
    Ipp32s pitchPix;
    pReconBuf    = core_enc->m_pReconstructFrame->m_pYPlane; // 16-byte aligned work buffer
    pitchPix = pitchPixels;

    // for INTRA 16x16 MBs computation of luma prediction was done as
    // a byproduct of sad calculation prior to this function being
    // called; the predictor blocks are already at pPredBuf.

    // Initialize the AC coeff flag value
    cur_mb.MacroblockCoeffsInfo->lumaAC = 0;
#ifdef ALT_RC
    cur_mb.LocalMacroblockInfo->sad = SAD16x16( pSrcPlane + uOffset, pitchPixels*sizeof(PIXTYPE), pPredBuf, 16*sizeof(PIXTYPE));
    curr_slice->m_Sad_Sum += cur_mb.LocalMacroblockInfo->sad;
#endif
    // compute the 4x4 luma DC transform coeffs
    H264ENC_MAKE_NAME(ippiSumsDiff16x16Blocks4x4)(pSrcPlane + uOffset, pitchPixels, pPredBuf, 16, pDCBuf, pMassDiffBuf);

#if defined (TRACE_INTRA_16X16)
    if(uMB == TRACE_INTRA_16X16) {
        printf("ermb 16x16 intra: uMB = %d, tr_bypass = %d, uCBPLuma = %d\n", uMB, transform_bypass, uCBPLuma);
        printMatr(pSrcPlane + uOffset, pitchPixels, 16, 16, 0, "SrcPlane");
        printMatr(pPredBuf, 16, 16, 16, 0, "PredBuf");
        printMatr(pDCBuf, 16, 1, 16, 0, "DC");
        printMatr(pMassDiffBuf, 4, 64, 4, 0, "MassDiff");
    }
#endif // TRACE_INTRA_16X16

    if(!transform_bypass) {
        // apply second transform on the luma DC transform coeffs
        // special case for baseline + CAVLC
        if( core_enc->m_info.profile_idc == 66 && !core_enc->m_info.entropy_coding_mode ){
            bool CAVLC_overflow;
            Ipp32s i;
            do{
                H264ENC_MAKE_NAME(ippiTransformQuantLumaDC_H264)(
                    pDCBuf,
                    pQBuf,
                    uMBQP,
                    &iNumCoeffs,
                    1,
                    enc_single_scan[is_cur_mb_field],
                    &iLastCoeff,
                    NULL);
                CAVLC_overflow = false;
                for(i=0; i<16; i++)
                    if( pDCBuf[i] > MAX_CAVLC_LEVEL ){ CAVLC_overflow = true; break; }
                if( CAVLC_overflow ){
                    cur_mb.LocalMacroblockInfo->QP++;
                    uMBQP = cur_mb.lumaQP = getLumaQP(cur_mb.LocalMacroblockInfo->QP, core_enc->m_PicParamSet.bit_depth_luma);
                }
            }while(CAVLC_overflow);
        }else
            H264ENC_MAKE_NAME(ippiTransformQuantLumaDC_H264)(
                pDCBuf,
                pQBuf,
                uMBQP,
                &iNumCoeffs,
                1,
                enc_single_scan[is_cur_mb_field],
                &iLastCoeff,
                NULL);

    }else {
       for(Ipp32s i = 0; i < 4; i++) {
            for(Ipp32s j = 0; j < 4; j++) {
                Ipp32s x, y;
                x = j*16;
                y = i*64;
                pDCBuf[i*4 + j] = pMassDiffBuf[x+y];
            }
        }
        ippiCountCoeffs(pDCBuf, &iNumCoeffs, enc_single_scan[is_cur_mb_field], &iLastCoeff, 16);
    }
#if defined (TRACE_INTRA_16X16)
    if(uMB == TRACE_INTRA_16X16) {
        printMatr(pDCBuf, pitchPixels, 1, 16, 0, "DC modified");
    }
#endif // TRACE_INTRA_16X16

    // insert the quantized luma Ipp64f transform DC coeffs into
    // RLE buffer

    // record RLE info
    if (core_enc->m_PicParamSet.entropy_coding_mode){
        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[Y_DC_RLE];
        bCoded = c_data->uNumSigCoeffs /* = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]*/ = ABS(iNumCoeffs);
        c_data->uLastSignificant = iLastCoeff;
        c_data->CtxBlockCat = BLOCK_LUMA_DC_LEVELS;
        c_data->uFirstCoeff = 0;
        c_data->uLastCoeff = 15;
        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pDCBuf,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[Y_DC_RLE]);
    }else{
        H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
            pDCBuf,
            0,
            dec_single_scan[is_cur_mb_field],
            iLastCoeff,
            &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
            &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
            &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
            &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
            curr_slice->Block_RLE[RLE_Offset].iLevels,
            curr_slice->Block_RLE[RLE_Offset].uRuns);

        bCoded = curr_slice->Block_RLE[RLE_Offset].uNumCoeffs;
    }

    if(!transform_bypass) {
        H264ENC_MAKE_NAME(ippiTransformDequantLumaDC_H264)(pDCBuf, uMBQP, NULL);
    }

    CabacStates cbSt;
    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA16x16 ){
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+10], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig+1, &curr_slice->m_pbitstream->context_array[105+15], 14*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last+1, &curr_slice->m_pbitstream->context_array[166+15], 14*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+14];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+14];
        }else{
            //memcpy( cbSt.sig+1, &curr_slice->m_pbitstream->context_array[105+172+15], 14*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last+1, &curr_slice->m_pbitstream->context_array[166+172+15], 14*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+172+14];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+172+14];
        }
    }

    // loop over all 4x4 blocks in Y plane for the MB
    for (uBlock = 0; uBlock < 16; uBlock++ ){
        pPredBuf = cur_mb.mb16x16.prediction + xoff[uBlock] + yoff[uBlock]*16;
        pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane+uOffset;
        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // This will be updated if the block is coded
        if (core_enc->m_PicParamSet.entropy_coding_mode) {
            curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        } else {
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
        }

        // check if block is coded
        bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));

        if (!bCoded){
            // update reconstruct frame for the empty block
            Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
        }else{   // block not declared empty, encode

            pTempDiffBuf = pMassDiffBuf+ xoff[uBlock]*4 + yoff[uBlock]*16;
            if(!transform_bypass) {
                if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTRA16x16 ){
                H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                    pTempDiffBuf,
                    pTransformResult,
                    uMBQP,
                    &iNumCoeffs,
                    1, //Always use f for INTRA
                    enc_single_scan[is_cur_mb_field],
                    &iLastCoeff,
                    NULL,
                    curr_slice,
                    1,
                    &cbSt);
                }else{
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pTempDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs,
                        1, //Always use f for INTRA
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        NULL,
                        0,
                        NULL);
                }
            }else{
                for(Ipp32s i = 0; i < 16; i++){
                    pTransformResult[i] = pTempDiffBuf[i];
                }
                ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan[is_cur_mb_field], &iLastCoeff, 16);
            }

            if( ((iNumCoeffs < -1) || (iNumCoeffs > 0)) ){
                cur_mb.MacroblockCoeffsInfo->lumaAC |= 1;
                // Preserve the absolute number of coeffs.
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    c_data->uLastSignificant = iLastCoeff;
                    c_data->CtxBlockCat = BLOCK_LUMA_AC_LEVELS;
                    c_data->uFirstCoeff = 1;
                    c_data->uLastCoeff = 15;
                    H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                } else {
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                        pTransformResult,//pDiffBuf,
                        1,
                        dec_single_scan[is_cur_mb_field],
                        iLastCoeff,
                        &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                        &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                        &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                        &curr_slice->Block_RLE[uBlock].uTotalZeros,
                        curr_slice->Block_RLE[uBlock].iLevels,
                        curr_slice->Block_RLE[uBlock].uRuns);

                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = bCoded = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                }
            }else{
               bCoded = 0;
               uCBPLuma &= ~CBP4x4Mask[uBlock];
            }

            // If the block wasn't coded and the DC coefficient is zero
            if (!bCoded && !pDCBuf[block_subblock_mapping[uBlock]]){
                // update reconstruct frame for the empty block
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
            } else if(!transform_bypass) {
                // inverse transform for reconstruct AND...
                // add inverse transformed coefficients to original predictor
                // to obtain reconstructed block, store in reconstruct frame
                // buffer
                H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264) (
                    pPredBuf,
                    pTransformResult,
                    &pDCBuf[block_subblock_mapping[uBlock]],
                    pReconBuf,
                    16,
                    pitchPix,
                    uMBQP,
                    ((iNumCoeffs < -1) || (iNumCoeffs > 0)),
                    core_enc->m_PicParamSet.bit_depth_luma,
                    NULL);
            }
        }   // block not declared empty

        // proceed to the next block
        uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
    }  // for uBlock in luma plane
#if defined (TRACE_INTRA_16X16)
    if(uMB == TRACE_INTRA_16X16) {
        printMatr(pReconBuf + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field],
            pitchPixels, 16, 16, 0, "RecPlane");
    }
#endif // TRACE_INTRA_16X16

    // In JVT, Chroma is Intra if any part of luma is intra.
    // update coded block flags
    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;

    // Correct the value of nc if both chroma DC and AC coeffs will be coded.
    if (cur_mb.MacroblockCoeffsInfo->lumaAC > 1)
        cur_mb.MacroblockCoeffsInfo->lumaAC = 1;

    // for each block of the MB initialize the AI mode (for INTER MB)
    // or motion vectors (for INTRA MB) to values which will be
    // correct predictors of that type. MV and AI mode prediction
    // depend upon this instead of checking MB type.

    cur_mb.intra_types[0] = cur_mb.LocalMacroblockInfo->intra_16x16_mode;
    for (Ipp32s i=1; i<16; i++)
    {
        cur_mb.intra_types[i] = 2;
    }

    return 1;

}   // CEncAndRec16x16IntraMB

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecInterMB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMBType;        // current MB type
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks

    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s* pDiffBuf;       // difference block pointer
    COEFFSTYPE *pTransform; // result of the transform.
    COEFFSTYPE *pTransformResult; // result of the transform.
    COEFFSTYPE* pQBuf;          // quantized block pointer
    Ipp16s* pMassDiffBuf;   // difference block pointer
    PIXTYPE*  pSrcPlane;      // start of plane to encode
    Ipp32s    pitchPixels;     // buffer pitch in pixels
    Ipp8u     bCoded;        // coded block flag
    Ipp32s    iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s    iLastCoeff;  // Number of nonzero coeffs after quant (negative if DC is nonzero)
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;

    uMBQP       = cur_mb.lumaQP;
    bool transform_bypass = core_enc->m_SeqParamSet.qpprime_y_zero_transform_bypass_flag && uMBQP == 0;
    __ALIGN16 CabacStates cbSt;

    uCBPLuma    = cur_mb.LocalMacroblockInfo->cbp_luma;
    pitchPixels = core_enc->m_pCurrentFrame->m_pitchPixels<<is_cur_mb_field;
    uMBType     = cur_mb.GlobalMacroblockInfo->mbtype;
    pTransform  = (COEFFSTYPE*)curr_slice->m_pMBEncodeBuffer;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pQBuf       = (COEFFSTYPE*) (pDiffBuf+64);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    uMB=cur_mb.uMB;

    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    Ipp32s pitchPix;
    pitchPix = pitchPixels;

    // initialize pointers and offset
    pSrcPlane = core_enc->m_pCurrentFrame->m_pYPlane;
    uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    pPredBuf = cur_mb.mbInter.prediction;

    // Motion Compensate this MB
    H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBLuma)(state, curr_slice, cur_mb.MVs[LIST_0]->MotionVectors, cur_mb.MVs[LIST_1]->MotionVectors, pPredBuf);
#ifdef ALT_RC
    cur_mb.LocalMacroblockInfo->sad = SAD16x16( pSrcPlane + uOffset, pitchPixels*sizeof(PIXTYPE), pPredBuf, 16*sizeof(PIXTYPE));
    curr_slice->m_Sad_Sum += cur_mb.LocalMacroblockInfo->sad;
#endif

#if defined (TRACE_INTER)
    if(uMB == TRACE_INTER) {
        fprintf(stderr,"ermbInter: uMB = %d, uCBPLuma = %d MBType=%d\n", uMB, uCBPLuma, cur_mb.GlobalMacroblockInfo->mbtype);
        printMatr(pSrcPlane + uOffset, pitchPixels, 16, 16, 0, "SrcPlane");
        printMatr(pPredBuf, 16, 16, 16, 0, "PredBuf");
    }
#endif // TRACE_INTER

    if (core_enc->m_PicParamSet.entropy_coding_mode){
        for( uBlock = 0; uBlock<16; uBlock++ ){
              cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
              curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        }
    } else {
        for( uBlock = 0; uBlock<16; uBlock++ ){
            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 16;
        }
    }

    if(pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo)) {
        Ipp32s mbCost=0;

    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER ){
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[402], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[417], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[402];
            cbSt.last = &curr_slice->m_pbitstream->context_array[417];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[436], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[451], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[436];
            cbSt.last = &curr_slice->m_pbitstream->context_array[451];
        }
    }
        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 1);
        //loop over all 8x8 blocks in Y plane for the MB
        Ipp32s coeffCost = 0;
        for (uBlock = 0; uBlock < 4; uBlock++){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock*4] + yoff[uBlock*4]*16;
            // check if block is coded
            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));

            if (bCoded){
                Diff8x8(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                pTransformResult = pTransform + uBlock*64;
                if(!transform_bypass) {
                    // forward transform and quantization, in place in pDiffBuf
                    H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
                    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER ){
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(pTransformResult, pTransformResult, uMBQP, 0,
                            enc_single_scan_8x8[is_cur_mb_field], core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[1][QP_MOD_6[uMBQP]], // INTER scaling matrix
                            &iNumCoeffs, &iLastCoeff,curr_slice,&cbSt,core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[1][QP_MOD_6[uMBQP]]);
                    }else{
                    H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                        pTransformResult,
                        pTransformResult,
                        QP_DIV_6[uMBQP],
                        0,
                        enc_single_scan_8x8[is_cur_mb_field],
                        core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[1][QP_MOD_6[uMBQP]], // INTER scaling matrix
                        &iNumCoeffs,
                        &iLastCoeff,
                        NULL,
                        NULL,
                        NULL);
                    }
                    coeffCost = H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 64, dec_single_scan_8x8[is_cur_mb_field]);
                    mbCost += coeffCost;
                } else {
                    for(Ipp32s i = 0; i < 64; i++) {
                        pTransformResult[i] = pDiffBuf[i];
                    }
                    ippiCountCoeffs(pTransformResult, &iNumCoeffs, enc_single_scan_8x8[is_cur_mb_field], &iLastCoeff, 64);
                }

#if defined (TRACE_INTER)
            if(uMB == TRACE_INTER) {
                fprintf(stderr,"iNumCoeffs = %d, iLastCoeff = %d\n", iNumCoeffs, iLastCoeff);
                printMatr(pTransformResult, 16, 1, 64, 0, "pTransformResult");
            }
#endif // TRACE_INTER

                // if everything quantized to zero, skip RLE
            if (!iNumCoeffs || (!transform_bypass && coeffCost < LUMA_COEFF_8X8_MAX_COST && core_enc->m_info.quant_opt_level < OPT_QUANT_INTER+1 ) ){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                } else {
                    // record RLE info
                    if (core_enc->m_PicParamSet.entropy_coding_mode){
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_64_LEVELS;
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 63;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan_8x8[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    }else{
                        COEFFSTYPE buf4x4[4][16];
                        Ipp8u iLastCoeff;
                        Ipp32s i4x4;

                        //Reorder 8x8 block for coding with CAVLC
                        for(i4x4=0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            for(i = 0; i<16; i++ )
                                buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] =
                                    pTransformResult[dec_single_scan_8x8[is_cur_mb_field][4*i+i4x4]];
                        }

                        Ipp32s idx = uBlock*4;
                        //Encode each block with CAVLC 4x4
                        for(i4x4 = 0; i4x4<4; i4x4++, idx++ ) {
                            Ipp32s i;
                            iLastCoeff = 0;

                            //Check for last coeff
                            for(i = 0; i<16; i++ ) if( buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] != 0 ) iLastCoeff=i;

                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                                buf4x4[i4x4],
                                0, //Luma
                                dec_single_scan[is_cur_mb_field],
                                iLastCoeff,
                                &curr_slice->Block_RLE[idx].uTrailing_Ones,
                                &curr_slice->Block_RLE[idx].uTrailing_One_Signs,
                                &curr_slice->Block_RLE[idx].uNumCoeffs,
                                &curr_slice->Block_RLE[idx].uTotalZeros,
                                curr_slice->Block_RLE[idx].iLevels,
                                curr_slice->Block_RLE[idx].uRuns);

                            cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = curr_slice->Block_RLE[idx].uNumCoeffs;
                         }
                    }
                }
            }
                uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }

        if( !transform_bypass && mbCost < LUMA_COEFF_MB_8X8_MAX_COST ){
                uCBPLuma = 0;
        }

       uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
       for (uBlock = 0; uBlock < 4; uBlock++){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock*4] + yoff[uBlock*4]*16;
            pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;
            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));
            if (!bCoded){
                Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                if (core_enc->m_PicParamSet.entropy_coding_mode)
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
                else
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+0] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+1] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+2] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+3] = 0;
            } else if(!transform_bypass){
                    H264ENC_MAKE_NAME(ippiQuantLuma8x8Inv_H264)(pTransform + uBlock*64, QP_DIV_6[uMBQP], core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[1][QP_MOD_6[uMBQP]]); //scaling matrix for INTER slice
                    H264ENC_MAKE_NAME(ippiTransformLuma8x8InvAddPred_H264)(pPredBuf, 16, pTransform + uBlock*64, pReconBuf, pitchPix, core_enc->m_PicParamSet.bit_depth_luma);
            }
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }
    } else {
        //loop over all 4x4 blocks in Y plane for the MB
        //first make transform for all blocks
      if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER ){
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+20], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+29];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+172+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+172+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+172+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+172+29];
        }
      }

        Ipp32s iNumCoeffs[16], CoeffsCost[16] = {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9};
        for (uBlock = 0; uBlock < 16; uBlock++ ){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock] + yoff[uBlock]*16;

            // check if block is coded
            bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));

            if( bCoded ){   // block not declared empty, encode
                Diff4x4(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                pTransformResult = pTransform + uBlock*16;
                if(!transform_bypass) {
                    // forward transform and quantization, in place in pDiffBuf
                    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER ){
                        H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                            pDiffBuf,
                            pTransformResult,
                            uMBQP,
                            &iNumCoeffs[uBlock],
                            0,
                            enc_single_scan[is_cur_mb_field],
                            &iLastCoeff,
                            NULL,
                            curr_slice,
                            0,
                            &cbSt);
                    }else{
                        H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                            pDiffBuf,
                            pTransformResult,
                            uMBQP,
                            &iNumCoeffs[uBlock],
                            0,
                            enc_single_scan[is_cur_mb_field],
                            &iLastCoeff,
                            NULL,
                            NULL,
                            0,
                            NULL);
                    }
                    CoeffsCost[uBlock] = H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 16, dec_single_scan[is_cur_mb_field]);
                } else {
                    for(Ipp32s i = 0; i < 16; i++) {
                        pTransformResult[i] = pDiffBuf[i];
                    }
                    ippiCountCoeffs(pTransformResult, &iNumCoeffs[uBlock], enc_single_scan[is_cur_mb_field], &iLastCoeff, 16);
                }

#if defined (TRACE_INTER)
            if(uMB == TRACE_INTER) {
                fprintf(stderr,"iNumCoeffs = %d, iLastCoeff = %d\n", iNumCoeffs[uBlock], iLastCoeff);
                printMatr(pTransformResult, 16, 1, 16, 0, "pTransformResult");
            }
#endif // TRACE_INTER

                if (!iNumCoeffs[uBlock]) { // if everything quantized to zero, skip RLE
                    uCBPLuma &= ~CBP4x4Mask[uBlock];
                }else{
                    // Preserve the absolute number of coeffs.
                    if (core_enc->m_PicParamSet.entropy_coding_mode) {
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs[uBlock] < 0) ? -(iNumCoeffs[uBlock]+1) : iNumCoeffs[uBlock]);
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs[uBlock]);
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_LEVELS;
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 15;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    } else {
                        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs[uBlock]);
                        H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                            pTransformResult,
                            0,
                            dec_single_scan[is_cur_mb_field],
                            iLastCoeff,
                            &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                            &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                            &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                            &curr_slice->Block_RLE[uBlock].uTotalZeros,
                            curr_slice->Block_RLE[uBlock].iLevels,
                            curr_slice->Block_RLE[uBlock].uRuns);
                    }
                }
            }

            // proceed to the next block
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
        }  // for 4x4 uBlock in luma plane

        //Skip subblock 8x8 if it cost is < 4 or skip MB if it's cost is < 5
        if( !transform_bypass ){
            Ipp32s mbCost=0;
            for( uBlock = 0; uBlock < 4; uBlock++ ){
                Ipp32s sb = uBlock*4;
                Ipp32s block8x8cost = CoeffsCost[sb] + CoeffsCost[sb+1] + CoeffsCost[sb+2] + CoeffsCost[sb+3];

                mbCost += block8x8cost;
                if( block8x8cost <= LUMA_8X8_MAX_COST && core_enc->m_info.quant_opt_level < OPT_QUANT_INTER+1 )
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
            }
                if( mbCost <= LUMA_MB_MAX_COST )
                    uCBPLuma = 0;
        }

        //Make inverse quantization and transform for non zero blocks
        uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
        for( uBlock=0; uBlock < 16; uBlock++ ){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock] + yoff[uBlock]*16;
            pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

            bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));
            if (!bCoded) {
                // update reconstruct frame for the empty block
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
            } else if(!transform_bypass) {
                 H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264) (
                     pPredBuf,
                     pTransform + uBlock*16,
                     NULL,
                     pReconBuf,
                     16,
                     pitchPix,
                     uMBQP,
                     ((iNumCoeffs[uBlock] < -1) || (iNumCoeffs[uBlock] > 0)),
                     core_enc->m_PicParamSet.bit_depth_luma,
                     NULL);
              }
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
           }
   }
#if defined (TRACE_INTER)
    if(uMB == TRACE_INTER) {
        printMatr(core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field],
            pitchPixels, 16, 16, 0, "RecPlane");
    }
#endif // TRACE_INTER
    //--------------------------------------------------------------------------
    // encode U plane blocks then V plane blocks
    //--------------------------------------------------------------------------

    // update coded block flags
    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// CEncAndRecMB
//
// Main function to drive encode and reconstruct for all blocks
// of one macroblock.
////////////////////////////////////////////////////////////////////////////////
void H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecMB)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    curr_slice->m_cur_mb.MacroblockCoeffsInfo->chromaNC = 0;
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    Ipp32s pitchPix;
    PIXTYPE *pDst, *pSrc;

    switch (curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype)
    {
        case MBTYPE_INTRA:
            H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec4x4IntraMB)(state, curr_slice);
            if( core_enc->m_PicParamSet.chroma_format_idc != 0 ) H264ENC_MAKE_NAME(H264CoreEncoder_EncodeChroma)(state, curr_slice);
            break;
        case MBTYPE_INTER:
        case MBTYPE_INTER_8x8:
        case MBTYPE_INTER_8x8_REF0:
        case MBTYPE_INTER_8x16:
        case MBTYPE_INTER_16x8:
        case MBTYPE_FORWARD:
        case MBTYPE_BACKWARD:
        case MBTYPE_FWD_FWD_16x8:
        case MBTYPE_FWD_BWD_16x8:
        case MBTYPE_BWD_FWD_16x8:
        case MBTYPE_BWD_BWD_16x8:
        case MBTYPE_FWD_FWD_8x16:
        case MBTYPE_FWD_BWD_8x16:
        case MBTYPE_BWD_FWD_8x16:
        case MBTYPE_BWD_BWD_8x16:
        case MBTYPE_BIDIR_FWD_16x8:
        case MBTYPE_FWD_BIDIR_16x8:
        case MBTYPE_BIDIR_BWD_16x8:
        case MBTYPE_BWD_BIDIR_16x8:
        case MBTYPE_BIDIR_BIDIR_16x8:
        case MBTYPE_BIDIR_FWD_8x16:
        case MBTYPE_FWD_BIDIR_8x16:
        case MBTYPE_BIDIR_BWD_8x16:
        case MBTYPE_BWD_BIDIR_8x16:
        case MBTYPE_BIDIR_BIDIR_8x16:
        case MBTYPE_B_8x8:
        case MBTYPE_DIRECT:
        case MBTYPE_BIDIR:
            H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecInterMB)(state, curr_slice);
            if( core_enc->m_PicParamSet.chroma_format_idc != 0 )
                H264ENC_MAKE_NAME(H264CoreEncoder_EncodeChroma)(state, curr_slice);
            //Check for possible skips after cbp reset for MBTYPE_DIRECT & MBTYPE_INTER
            {
                if( cur_mb.LocalMacroblockInfo->cbp_luma != 0 || cur_mb.LocalMacroblockInfo->cbp_chroma != 0 ) break;
                if( cur_mb.GlobalMacroblockInfo->mbtype == MBTYPE_INTER && cur_mb.RefIdxs[LIST_0]->RefIdxs[0] == 0 ){
                    H264MotionVector skip_vec;
                    H264ENC_MAKE_NAME(H264CoreEncoder_Skip_MV_Predicted)(state, curr_slice, NULL, &skip_vec);
                    if( cur_mb.MVs[LIST_0]->MotionVectors[0] != skip_vec ) break;
                }else if( cur_mb.GlobalMacroblockInfo->mbtype != MBTYPE_DIRECT ) break;
            }

        case MBTYPE_SKIPPED: //copy prediction to recostruct
            {
                Ipp32s i;
//                for( i = 0; i<4; i++ )  cur_mb.GlobalMacroblockInfo->sbtype[i] = (MBTypeValue)NUMBER_OF_MBTYPES;
                pitchPix = cur_mb.mbPitchPixels;
                pDst = core_enc->m_pReconstructFrame->m_pYPlane + core_enc->m_pMBOffsets[cur_mb.uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
                pSrc = cur_mb.mbInter.prediction;
                if( curr_slice->m_slice_type == BPREDSLICE ){
                    curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_DIRECT; //for correct MC
                }
                H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBLuma)(state, curr_slice, cur_mb.MVs[LIST_0]->MotionVectors, cur_mb.MVs[LIST_1]->MotionVectors, pSrc);
                curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_SKIPPED;
                for( i = 0; i<16; i++ ){
                    memcpy( pDst, pSrc, 16*sizeof(PIXTYPE));
                    pDst += pitchPix;
                    pSrc += 16;
                }
                memset( cur_mb.MacroblockCoeffsInfo->numCoeff, 0, 16 );  //Reset this number for skips
                for (i=0; i<16; i++) cur_mb.intra_types[i] = 2;

                if( core_enc->m_PicParamSet.chroma_format_idc != 0 ){
                    pDst = core_enc->m_pReconstructFrame->m_pUPlane + core_enc->m_pMBOffsets[cur_mb.uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
                    pSrc = cur_mb.mbChromaInter.prediction;
                    if( curr_slice->m_slice_type == BPREDSLICE )
                         curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_DIRECT; //for correct MC
                    H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBChroma)(state, curr_slice, pSrc);
                    curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype = MBTYPE_SKIPPED;
                    Ipp32s vsize = 8;
                    if( core_enc->m_info.chroma_format_idc == 2 ) vsize = 16;
                    for( i = 0; i < vsize; i++ ){
                        memcpy( pDst, pSrc, 8*sizeof(PIXTYPE));
                        pDst += pitchPix;
                        pSrc += 16;
                    }
                    pDst = core_enc->m_pReconstructFrame->m_pVPlane + core_enc->m_pMBOffsets[cur_mb.uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
                    pSrc = cur_mb.mbChromaInter.prediction+8;
                    for( i = 0; i < vsize; i++ ){
                         memcpy( pDst, pSrc, 8*sizeof(PIXTYPE));
                         pDst += pitchPix;
                         pSrc += 16;
                    }
                    memset( &cur_mb.MacroblockCoeffsInfo->numCoeff[16], 0, 32 );  //Reset this number for skips
                    cur_mb.LocalMacroblockInfo->cbp_chroma = 0;
                }
            }
            break;

        case MBTYPE_INTRA_16x16:
            H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec16x16IntraMB)(state, curr_slice);
            if( core_enc->m_PicParamSet.chroma_format_idc != 0 )
                H264ENC_MAKE_NAME(H264CoreEncoder_EncodeChroma)(state, curr_slice);
            break;

        default:
            break;
    }

   if( IS_INTRA_MBTYPE( curr_slice->m_cur_mb.GlobalMacroblockInfo->mbtype ) ){
        Ipp32s k;
        for (k = 0; k < 16; k ++){
            curr_slice->m_cur_mb.MVs[LIST_0]->MotionVectors[k] = null_mv;
            curr_slice->m_cur_mb.MVs[LIST_1]->MotionVectors[k] = null_mv;
            curr_slice->m_cur_mb.MVs[LIST_0 + 2]->MotionVectors[k] = null_mv;
            curr_slice->m_cur_mb.MVs[LIST_1 + 2]->MotionVectors[k] = null_mv;
            curr_slice->m_cur_mb.RefIdxs[LIST_0]->RefIdxs[k] = -1;
            curr_slice->m_cur_mb.RefIdxs[LIST_1]->RefIdxs[k] = -1;
         }
   }else{
      Ipp32s k;
      for (k = 0; k < 16; k++)
           curr_slice->m_cur_mb.intra_types[k] = (T_AIMode) 2;
   }

    if (curr_slice->m_cur_mb.LocalMacroblockInfo->cbp_chroma == 0)
        curr_slice->m_cur_mb.MacroblockCoeffsInfo->chromaNC = 0;
}

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantIntra_RD)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMBType;        // current MB type
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks
    Ipp32u  uIntraSAD;      // intra MB SAD
    Ipp16s* pMassDiffBuf;   // difference block pointer

    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s*   pDiffBuf;       // difference block pointer
    COEFFSTYPE* pTransformResult; // Result of the transformation.
    COEFFSTYPE* pQBuf;          // quantized block pointer
    PIXTYPE*  pSrcPlane;      // start of plane to encode
    Ipp32s    pitchPixels;     // buffer pitch
    Ipp32s    iMBCost;        // recode MB cost counter
    Ipp32s    iBlkCost[2];    // coef removal counter for left/right 8x8 luma blocks
    Ipp8u     bCoded; // coded block flag
    Ipp32s    iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s    iLastCoeff;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32u    uTotalCoeffs = 0;    // Used to detect single expensive coeffs.

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    uMB = cur_mb.uMB;

    pitchPixels = cur_mb.mbPitchPixels;
    uCBPLuma     = cur_mb.LocalMacroblockInfo->cbp_luma;
    uMBQP       = cur_mb.lumaQP;
    uMBType     = cur_mb.GlobalMacroblockInfo->mbtype;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pTransformResult = (COEFFSTYPE*)(pDiffBuf + 64);
    pQBuf       = (COEFFSTYPE*) (pTransformResult + 64);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    uIntraSAD   = 0;
    iMBCost     = 0;
    iBlkCost[0] = 0;
    iBlkCost[1] = 0;

    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    // initialize pointers and offset
    pSrcPlane = core_enc->m_pCurrentFrame->m_pYPlane;
    uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    CabacStates cbSt;

    Ipp32s pitchPix = 16;
//    pitchPix = pitchPixels;

    if(pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo)) {

      if( core_enc->m_info.quant_opt_level > 1){
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[402], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[417], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[402];
            cbSt.last = &curr_slice->m_pbitstream->context_array[417];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[436], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[451], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[436];
            cbSt.last = &curr_slice->m_pbitstream->context_array[451];
        }
     }
        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 1);

        for (uBlock = 0; uBlock < 4; uBlock ++){
            Ipp32s idxb, idx, idxe;

            idxb = uBlock<<2;
            idxe = idxb+4;
            pPredBuf = cur_mb.mb8x8.prediction + xoff[4*uBlock] + yoff[4*uBlock]*16;
            pReconBuf = cur_mb.mb8x8.reconstruct + xoff[4*uBlock] + yoff[4*uBlock]*16;
            //pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

            if (core_enc->m_PicParamSet.entropy_coding_mode)
            {
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
                curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
            } else {
                for( idx = idxb; idx<idxe; idx++ ){
                    curr_slice->Block_RLE[idx].uNumCoeffs = 0;
                    curr_slice->Block_RLE[idx].uTrailing_Ones = 0;
                    curr_slice->Block_RLE[idx].uTrailing_One_Signs = 0;
                    curr_slice->Block_RLE[idx].uTotalZeros = 16;
                    cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = 0;
               }
            }

            if (!curr_slice->m_use_transform_for_intra_decision){
                uIntraSAD += H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneMB_8x8)(
                    state,
                    curr_slice,
                    pSrcPlane + uOffset,
                    pReconBuf,
                    uBlock,
                    cur_mb.intra_types,
                    pPredBuf);
            }

            // check if block is coded
            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));

            if (!bCoded){  // update reconstruct frame for the empty block
                Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
            } else {   // block not declared empty, encode
                // compute difference of predictor and source pels
                // note: asm version does not use pDiffBuf
                //       output is being passed in the mmx registers
            if (!curr_slice->m_use_transform_for_intra_decision /*|| core_enc->m_info.quant_opt_level > 1*/){
                Diff8x8(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                    // forward transform and quantization, in place in pDiffBuf
                    H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
                    if( core_enc->m_info.quant_opt_level > 1 ){
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(pTransformResult,pTransformResult,uMBQP, 1,
                            enc_single_scan_8x8[is_cur_mb_field], core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[0][QP_MOD_6[uMBQP]], //Use scaling matrix for INTRA
                            &iNumCoeffs, &iLastCoeff,curr_slice,&cbSt,core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[0][QP_MOD_6[uMBQP]]);
                    }else{
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                            pTransformResult,
                            pTransformResult,
                            QP_DIV_6[uMBQP],
                            1,
                            enc_single_scan_8x8[is_cur_mb_field],
                            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[0][QP_MOD_6[uMBQP]], //Use scaling matrix for INTRA
                            &iNumCoeffs,
                            &iLastCoeff,
                            NULL,
                            NULL,
                            NULL);
                    }
            }else{
                    iNumCoeffs = cur_mb.m_iNumCoeffs8x8[ uBlock ];
                    iLastCoeff = cur_mb.m_iLastCoeff8x8[ uBlock ];
                    pTransformResult = &cur_mb.mb8x8.transform[ uBlock*64 ];
            }

                // if everything quantized to zero, skip RLE
                if (!iNumCoeffs ){ // the block is empty so it is not coded
                    bCoded = 0;
                } else {
                    uTotalCoeffs += ((iNumCoeffs < 0) ? -(iNumCoeffs*2) : iNumCoeffs);

                    // record RLE info
                    if (core_enc->m_PicParamSet.entropy_coding_mode){
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_64_LEVELS;
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 63;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan_8x8[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                        bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                    }else{
                        COEFFSTYPE buf4x4[4][16];
                        Ipp32s i4x4;

                        //Reorder 8x8 block for coding with CAVLC
                        for(i4x4=0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            for(i = 0; i<16; i++ )
                                buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] =
                                    pTransformResult[dec_single_scan_8x8[is_cur_mb_field][4*i+i4x4]];
                        }

                        bCoded = 0;
                        //Encode each block with CAVLC 4x4
                        for(i4x4 = 0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            iLastCoeff = 0;
                            idx = idxb + i4x4;

                            //Check for last coeff
                            for(i = 0; i<16; i++ ) if( buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] != 0 ) iLastCoeff=i;

                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                                buf4x4[i4x4],
                                0, //Luma
                                dec_single_scan[is_cur_mb_field],
                                iLastCoeff,
                                &curr_slice->Block_RLE[idx].uTrailing_Ones,
                                &curr_slice->Block_RLE[idx].uTrailing_One_Signs,
                                &curr_slice->Block_RLE[idx].uNumCoeffs,
                                &curr_slice->Block_RLE[idx].uTotalZeros,
                                curr_slice->Block_RLE[idx].iLevels,
                                curr_slice->Block_RLE[idx].uRuns);

                            bCoded += curr_slice->Block_RLE[idx].uNumCoeffs;
                            cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = curr_slice->Block_RLE[idx].uNumCoeffs;
                         }
                    }
                }

            // update flags if block quantized to empty
            if (curr_slice->m_use_transform_for_intra_decision){
                if (!bCoded){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                    //Copy  prediction
                    Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                }else //Copy reconstruct
                    Copy8x8(pPredBuf + 256, 16, pReconBuf, pitchPix);
            }else{
                // update flags if block quantized to empty
                if (!bCoded){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                    // update reconstruct frame with prediction
                    Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                }else {
                    // inverse transform for reconstruct AND...
                    // add inverse transformed coefficients to original predictor
                    // to obtain reconstructed block, store in reconstruct frame
                    // buffer
                    if(iNumCoeffs != 0) {
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8Inv_H264)(pTransformResult, QP_DIV_6[uMBQP], core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[0][QP_MOD_6[uMBQP]]);
                        H264ENC_MAKE_NAME(ippiTransformLuma8x8InvAddPred_H264)(pPredBuf, 16, pTransformResult, pReconBuf, pitchPix, core_enc->m_PicParamSet.bit_depth_luma);
                    }
                }
            }   // block not declared empty
            } //curr_slice->m_use_transform_for_intra_decision
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }  // for uBlock in luma plane
    }else{
      if( core_enc->m_info.quant_opt_level > 1 ){
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+20], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+29];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+172+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+172+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+172+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+172+29];
        }
      }

    for (uBlock = 0; uBlock < 16; uBlock++ ){
        pPredBuf = cur_mb.mb4x4.prediction + xoff[uBlock] + yoff[uBlock]*16;
        pReconBuf = cur_mb.mb4x4.reconstruct + xoff[uBlock] + yoff[uBlock]*16;
        //pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0; // These will be updated if the block is coded
        if (core_enc->m_PicParamSet.entropy_coding_mode){
            curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        }else{
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 16;
        }

        // find advanced intra prediction block, store in PredBuf
        // Select best AI mode for the block, using reconstructed
        // predictor pels. This function also stores the block
        // predictor pels at pPredBuf.
        if (!curr_slice->m_use_transform_for_intra_decision){
            uIntraSAD += H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneBlock)(
                state,
                curr_slice,
                pSrcPlane + uOffset,
                pReconBuf,
                uBlock,
                cur_mb.intra_types,
                pPredBuf);
        }

        // check if block is coded
        bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));

        if (!bCoded){
            // update reconstruct frame for the empty block
            Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
        } else {   // block not declared empty, encode
            // compute difference of predictor and source pels
            // note: asm version does not use pDiffBuf
            //       output is being passed in the mmx registers
          if (!curr_slice->m_use_transform_for_intra_decision /*|| core_enc->m_info.quant_opt_level > 1*/){
            Diff4x4(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                if( core_enc->m_info.quant_opt_level > 1 ){
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs,
                        1, //Always use f for INTRA
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        curr_slice,
                        0,
                        &cbSt);
                }else{
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs,
                        1, //Always use f for INTRA
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        NULL,
                        0,
                        NULL);
                 }
          }else{
              iNumCoeffs = cur_mb.m_iNumCoeffs4x4[ uBlock ];
              iLastCoeff = cur_mb.m_iLastCoeff4x4[ uBlock ];
              pTransformResult = &cur_mb.mb4x4.transform[ uBlock*16 ];
          }
            // if everything quantized to zero, skip RLE
            if (!iNumCoeffs){
                // the block is empty so it is not coded
                bCoded = 0;
            } else {
                // Preserve the absolute number of coeffs.
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
//                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                    c_data->uLastSignificant = iLastCoeff;
                    c_data->CtxBlockCat = BLOCK_LUMA_LEVELS;
                    c_data->uFirstCoeff = 0;
                    c_data->uLastCoeff = 15;
                    H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    bCoded = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                } else {
                // record RLE info
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
                    H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                        pTransformResult,
                        0,
                        dec_single_scan[is_cur_mb_field],
                        iLastCoeff,
                        &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                        &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                        &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                        &curr_slice->Block_RLE[uBlock].uTotalZeros,
                        curr_slice->Block_RLE[uBlock].iLevels,
                        curr_slice->Block_RLE[uBlock].uRuns);
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = bCoded = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                }
            }

            // update flags if block quantized to empty
            if (curr_slice->m_use_transform_for_intra_decision) {
                if (!bCoded) {
                    uCBPLuma &= ~CBP4x4Mask[uBlock]; //Copy predition
                    Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                }else //Copy reconstruct
                    Copy4x4(pPredBuf + 256, 16, pReconBuf, pitchPix);
            } else {
                if (!bCoded){
                    uCBPLuma &= ~CBP4x4Mask[uBlock];
                    Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                } else {
                    H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                        pPredBuf,
                        pTransformResult,
                        NULL,
                        pReconBuf,
                        16,
                        pitchPix,
                        uMBQP,
                        ((iNumCoeffs < -1) || (iNumCoeffs > 0)),
                        core_enc->m_PicParamSet.bit_depth_luma,
                        NULL);
                }
            }
        }   // block not declared empty

        // proceed to the next block
        uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
    }  // for uBlock in luma plane
    }

    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;

    return 1;
}

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantInter_RD)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;     // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMBType;        // current MB type
    Ipp32u  uMB;
    Ipp32u  uCBPLuma;        // coded flags for all 4x4 blocks

    COEFFSTYPE* pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;       // prediction block pointer
    PIXTYPE*  pReconBuf;       // prediction block pointer
    Ipp16s* pDiffBuf;       // difference block pointer
    COEFFSTYPE *pTransform; // result of the transform.
    COEFFSTYPE *pTransformResult; // result of the transform.
    COEFFSTYPE* pQBuf;          // quantized block pointer
    Ipp16s* pMassDiffBuf;   // difference block pointer
    PIXTYPE*  pSrcPlane;      // start of plane to encode
    Ipp32s    pitchPixels;     // buffer pitch in pixels
    Ipp8u     bCoded;        // coded block flag
    Ipp32s    iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s    iLastCoeff;  // Number of nonzero coeffs after quant (negative if DC is nonzero)
    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;

    uMBQP       = cur_mb.lumaQP;
    CabacStates cbSt;

    uCBPLuma    = cur_mb.LocalMacroblockInfo->cbp_luma;
    pitchPixels = core_enc->m_pCurrentFrame->m_pitchPixels << is_cur_mb_field;
    uMBType     = cur_mb.GlobalMacroblockInfo->mbtype;
    pTransform  = (COEFFSTYPE*)curr_slice->m_pMBEncodeBuffer;
    pDiffBuf    = (Ipp16s*) (curr_slice->m_pMBEncodeBuffer + 512);
    pQBuf       = (COEFFSTYPE*) (pDiffBuf+64);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    uMB=cur_mb.uMB;

    //--------------------------------------------------------------------------
    // encode Y plane blocks (0-15)
    //--------------------------------------------------------------------------

    Ipp32s pitchPix = 16;
//    pitchPix = pitchPixels;

    // initialize pointers and offset
    pSrcPlane = core_enc->m_pCurrentFrame->m_pYPlane;
    uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    pPredBuf = cur_mb.mbInter.prediction;

    // Motion Compensate this MB
    H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBLuma)(state, curr_slice, cur_mb.MVs[LIST_0]->MotionVectors, cur_mb.MVs[LIST_1]->MotionVectors, pPredBuf);

    if (core_enc->m_PicParamSet.entropy_coding_mode){
        for( uBlock = 0; uBlock<16; uBlock++ ){
              cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
              curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
        }
    } else {
        for( uBlock = 0; uBlock<16; uBlock++ ){
            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;        // These will be updated if the block is coded
            curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
            curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
            curr_slice->Block_RLE[uBlock].uTotalZeros = 16;
        }
    }

    if(pGetMB8x8TSPackFlag(cur_mb.GlobalMacroblockInfo)) {
        Ipp32s mbCost=0;

    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER_RD ){
//Save current cabac state
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[426], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[402], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[417], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[402];
            cbSt.last = &curr_slice->m_pbitstream->context_array[417];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[436], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[451], 9*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[436];
            cbSt.last = &curr_slice->m_pbitstream->context_array[451];
        }
     }
        pSetMB8x8TSFlag(cur_mb.GlobalMacroblockInfo, 1);
        //loop over all 8x8 blocks in Y plane for the MB
        Ipp32s coeffCost;
        for (uBlock = 0; uBlock < 4; uBlock++){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock*4] + yoff[uBlock*4]*16;
            // check if block is coded
            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));

            if (bCoded){
                Diff8x8(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                pTransformResult = pTransform + uBlock*64;
                    // forward transform and quantization, in place in pDiffBuf
                    H264ENC_MAKE_NAME(ippiTransformLuma8x8Fwd_H264)(pDiffBuf, pTransformResult);
                    if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER_RD ){
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                            pTransformResult,
                            pTransformResult,
                            uMBQP,
                            0,
                            enc_single_scan_8x8[is_cur_mb_field],
                            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[1][QP_MOD_6[uMBQP]], // INTER scaling matrix
                            &iNumCoeffs,
                            &iLastCoeff,
                            curr_slice,
                            &cbSt,
                            core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[1][QP_MOD_6[uMBQP]]);
                    }else{
                        H264ENC_MAKE_NAME(ippiQuantLuma8x8_H264)(
                            pTransformResult,
                            pTransformResult,
                            QP_DIV_6[uMBQP],
                            0,
                            enc_single_scan_8x8[is_cur_mb_field],
                            core_enc->m_SeqParamSet.seq_scaling_matrix_8x8[1][QP_MOD_6[uMBQP]], // INTER scaling matrix
                            &iNumCoeffs,
                            &iLastCoeff,
                            NULL,
                            NULL,
                            NULL);
                    }
                    coeffCost = H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 64, dec_single_scan_8x8[is_cur_mb_field]);
                    mbCost += coeffCost;

                // if everything quantized to zero, skip RLE
                if (!iNumCoeffs || (coeffCost < LUMA_COEFF_8X8_MAX_COST && core_enc->m_info.quant_opt_level < OPT_QUANT_INTER_RD+1)){
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
                } else {
                    // record RLE info
                    if (core_enc->m_PicParamSet.entropy_coding_mode){
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_64_LEVELS;
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs);
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 63;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan_8x8[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    }else{
                        COEFFSTYPE buf4x4[4][16];
                        Ipp8u iLastCoeff;
                        Ipp32s i4x4;

                        //Reorder 8x8 block for coding with CAVLC
                        for(i4x4=0; i4x4<4; i4x4++ ) {
                            Ipp32s i;
                            for(i = 0; i<16; i++ )
                                buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] =
                                    pTransformResult[dec_single_scan_8x8[is_cur_mb_field][4*i+i4x4]];
                        }

                        Ipp32s idx = uBlock*4;
                        //Encode each block with CAVLC 4x4
                        for(i4x4 = 0; i4x4<4; i4x4++, idx++ ) {
                            Ipp32s i;
                            iLastCoeff = 0;

                            //Check for last coeff
                            for(i = 0; i<16; i++ ) if( buf4x4[i4x4][dec_single_scan[is_cur_mb_field][i]] != 0 ) iLastCoeff=i;

                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                                buf4x4[i4x4],
                                0, //Luma
                                dec_single_scan[is_cur_mb_field],
                                iLastCoeff,
                                &curr_slice->Block_RLE[idx].uTrailing_Ones,
                                &curr_slice->Block_RLE[idx].uTrailing_One_Signs,
                                &curr_slice->Block_RLE[idx].uNumCoeffs,
                                &curr_slice->Block_RLE[idx].uTotalZeros,
                                curr_slice->Block_RLE[idx].iLevels,
                                curr_slice->Block_RLE[idx].uRuns);

                            cur_mb.MacroblockCoeffsInfo->numCoeff[idx] = curr_slice->Block_RLE[idx].uNumCoeffs;
                         }
                    }
                }
            }
                uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }

        if( mbCost < LUMA_COEFF_MB_8X8_MAX_COST ){
                uCBPLuma = 0;
        }

       uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
       for (uBlock = 0; uBlock < 4; uBlock++){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock*4] + yoff[uBlock*4]*16;
            pReconBuf = cur_mb.mbInter.reconstruct + xoff[uBlock*4] + yoff[uBlock*4]*16;
            //pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

            bCoded = ((uCBPLuma & CBP8x8Mask[uBlock])?(1):(0));
            if (!bCoded){
                Copy8x8(pPredBuf, 16, pReconBuf, pitchPix);
                if (core_enc->m_PicParamSet.entropy_coding_mode)
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
                else
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+0] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+1] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+2] =
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock*4+3] = 0;
            } else {
                    H264ENC_MAKE_NAME(ippiQuantLuma8x8Inv_H264)(pTransform + uBlock*64, QP_DIV_6[uMBQP], core_enc->m_SeqParamSet.seq_scaling_inv_matrix_8x8[1][QP_MOD_6[uMBQP]]); //scaling matrix for INTER slice
                    H264ENC_MAKE_NAME(ippiTransformLuma8x8InvAddPred_H264)(pPredBuf, 16, pTransform + uBlock*64, pReconBuf, pitchPix, core_enc->m_PicParamSet.bit_depth_luma);
            }
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock] * 2;
        }
    } else {
      if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER_RD ){
//Save current cabac state
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+20], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+29];
        }else{
            //memcpy( cbSt.sig, &curr_slice->m_pbitstream->context_array[105+172+29], 15*sizeof(CABAC_CONTEXT));
            //memcpy( cbSt.last, &curr_slice->m_pbitstream->context_array[166+172+29], 15*sizeof(CABAC_CONTEXT));
            cbSt.sig = &curr_slice->m_pbitstream->context_array[105+172+29];
            cbSt.last = &curr_slice->m_pbitstream->context_array[166+172+29];
        }
     }

        Ipp32s iNumCoeffs[16], CoeffsCost[16] = {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9};
        for (uBlock = 0; uBlock < 16; uBlock++ ){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock] + yoff[uBlock]*16;

            // check if block is coded
            bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));

            if( bCoded ){   // block not declared empty, encode
                Diff4x4(pPredBuf, pSrcPlane + uOffset, pitchPixels, pDiffBuf);
                pTransformResult = pTransform + uBlock*16;
                if( core_enc->m_info.quant_opt_level > OPT_QUANT_INTER_RD ){
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs[uBlock],
                        0,
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        curr_slice,
                        0,
                        &cbSt);
                }else{
                    H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                        pDiffBuf,
                        pTransformResult,
                        uMBQP,
                        &iNumCoeffs[uBlock],
                        0,
                        enc_single_scan[is_cur_mb_field],
                        &iLastCoeff,
                        NULL,
                        NULL,
                        0,
                        NULL);
                }
                CoeffsCost[uBlock] = H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 16, dec_single_scan[is_cur_mb_field]);

                if (!iNumCoeffs[uBlock]) { // if everything quantized to zero, skip RLE
                    uCBPLuma &= ~CBP4x4Mask[uBlock];
                }else{
                    // Preserve the absolute number of coeffs.
                    if (core_enc->m_PicParamSet.entropy_coding_mode) {
                        T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
//                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs[uBlock] < 0) ? -(iNumCoeffs[uBlock]+1) : iNumCoeffs[uBlock]);
                        c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs[uBlock]);
                        c_data->uLastSignificant = iLastCoeff;
                        c_data->CtxBlockCat = BLOCK_LUMA_LEVELS;
                        c_data->uFirstCoeff = 0;
                        c_data->uLastCoeff = 15;
                        H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                    } else {
                        cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)ABS(iNumCoeffs[uBlock]);
                        H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264)(
                            pTransformResult,
                            0,
                            dec_single_scan[is_cur_mb_field],
                            iLastCoeff,
                            &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                            &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                            &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                            &curr_slice->Block_RLE[uBlock].uTotalZeros,
                            curr_slice->Block_RLE[uBlock].iLevels,
                            curr_slice->Block_RLE[uBlock].uRuns);
                    }
                }
            }

            // proceed to the next block
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
        }  // for 4x4 uBlock in luma plane

        //Skip subblock 8x8 if it cost is < 4 or skip MB if it's cost is < 5
            Ipp32s mbCost=0;
            for( uBlock = 0; uBlock < 4; uBlock++ ){
                Ipp32s sb = uBlock*4;
                Ipp32s block8x8cost = CoeffsCost[sb] + CoeffsCost[sb+1] + CoeffsCost[sb+2] + CoeffsCost[sb+3];

                mbCost += block8x8cost;
                if( block8x8cost <= LUMA_8X8_MAX_COST && core_enc->m_info.quant_opt_level < OPT_QUANT_INTER_RD+1)
                    uCBPLuma &= ~CBP8x8Mask[uBlock];
            }
                if( mbCost <= LUMA_MB_MAX_COST )
                    uCBPLuma = 0;

        //Make inverse quantization and transform for non zero blocks
        uOffset = core_enc->m_pMBOffsets[uMB].uLumaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
        for( uBlock=0; uBlock < 16; uBlock++ ){
            pPredBuf = cur_mb.mbInter.prediction + xoff[uBlock] + yoff[uBlock]*16;
            pReconBuf = cur_mb.mbInter.reconstruct + xoff[uBlock] + yoff[uBlock]*16;
            //pReconBuf = core_enc->m_pReconstructFrame->m_pYPlane + uOffset;

            bCoded = ((uCBPLuma & CBP4x4Mask[uBlock])?(1):(0));
            if (!bCoded) {
                // update reconstruct frame for the empty block
                Copy4x4(pPredBuf, 16, pReconBuf, pitchPix);
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
            } else {
                 H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264) (
                     pPredBuf,
                     pTransform + uBlock*16,
                     NULL,
                     pReconBuf,
                     16,
                     pitchPix,
                     uMBQP,
                     ((iNumCoeffs[uBlock] < -1) || (iNumCoeffs[uBlock] > 0)),
                     core_enc->m_PicParamSet.bit_depth_luma,
                     NULL);
              }
            uOffset += core_enc->m_EncBlockOffsetInc[is_cur_mb_field][uBlock];
           }
   }
    cur_mb.LocalMacroblockInfo->cbp_luma = uCBPLuma;
    return 1;
}

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantChromaIntra_RD)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;         // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMB;
    PIXTYPE*  pSrcPlane;    // start of plane to encode
    Ipp32s    pitchPixels;  // buffer pitch
    COEFFSTYPE *pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;     // prediction block pointer
    PIXTYPE*  pReconBuf;     // prediction block pointer
    PIXTYPE*  pPredBuf_copy;     // prediction block pointer
    COEFFSTYPE* pQBuf;      // quantized block pointer
    Ipp16s* pMassDiffBuf;   // difference block pointer

    Ipp32u   uCBPChroma;    // coded flags for all chroma blocks
    Ipp32s   iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   iLastCoeff;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   RLE_Offset;    // Index into BlockRLE array

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    COEFFSTYPE *pTransformResult;
    COEFFSTYPE *pTransform;

    pitchPixels = cur_mb.mbPitchPixels;
    uMBQP       = cur_mb.chromaQP;
    pTransform = (COEFFSTYPE*)curr_slice->m_pMBEncodeBuffer;
    pQBuf       = (COEFFSTYPE*) (pTransform + 64*2);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    Ipp16s*  pTempDiffBuf;
    uMB = cur_mb.uMB;

    // initialize pointers and offset
    uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
//    uCBPChroma  = cur_mb.LocalMacroblockInfo->cbp_chroma;
    uCBPChroma  = cur_mb.LocalMacroblockInfo->cbp_chroma = 0xffffffff;
    cur_mb.MacroblockCoeffsInfo->chromaNC = 0;

    pPredBuf = cur_mb.mbChromaIntra.prediction;
        // initialize pointers for the U plane blocks
        Ipp32s num_blocks = 2<<core_enc->m_PicParamSet.chroma_format_idc;
        Ipp32s startBlock;
        startBlock = uBlock = 16;
        Ipp32u uLastBlock = uBlock+num_blocks;
        Ipp32u uFinalBlock = uBlock+2*num_blocks;

        // encode first chroma U plane then V plane
        do{
            // Adjust the pPredBuf to point at the V plane predictor when appropriate:
            // (blocks and an INTRA Y plane mode...)
            if (uBlock == uLastBlock) {
                startBlock = uBlock;
                uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
                pSrcPlane = core_enc->m_pCurrentFrame->m_pVPlane;
                pPredBuf = cur_mb.mbChromaIntra.prediction+8;
                pReconBuf = cur_mb.mbChromaIntra.reconstruct+8;
                RLE_Offset = V_DC_RLE;
                uLastBlock += num_blocks;
            } else {
                RLE_Offset = U_DC_RLE;
                pSrcPlane = core_enc->m_pCurrentFrame->m_pUPlane;
                pPredBuf = cur_mb.mbChromaIntra.prediction;
                pReconBuf = cur_mb.mbChromaIntra.reconstruct;
            }

            if( core_enc->m_PicParamSet.chroma_format_idc == 2 ){
                H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                    pSrcPlane + uOffset,    // source pels
                    pitchPixels,                 // source pitch
                    pPredBuf,               // predictor pels
                    16,
                    pDCBuf,                 // result buffer
                    pMassDiffBuf);
                // Process second part of 2x4 block for DC coeffs
                H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                    pSrcPlane + uOffset+8*pitchPixels,    // source pels
                    pitchPixels,                 // source pitch
                    pPredBuf+8*16,               // predictor pels
                    16,
                    pDCBuf+4,                 // result buffer
                    pMassDiffBuf+64);   //+Offset for second path
                H264ENC_MAKE_NAME(ippiTransformQuantChroma422DC_H264)(
                    pDCBuf,
                    pQBuf,
                    uMBQP,
                    &iNumCoeffs,
                    (slice_type == INTRASLICE),
                    1,
                    NULL);
                 // DC values in this block if iNonEmpty is 1.
                cur_mb.MacroblockCoeffsInfo->chromaNC |= (iNumCoeffs != 0);
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    Ipp32s ctxIdxBlockCat = BLOCK_CHROMA_DC_LEVELS;
                    H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,8,dec_single_scan_p422,&curr_slice->Block_CABAC[RLE_Offset]);
                }else{
                        H264ENC_MAKE_NAME(ippiEncodeChroma422DC_CoeffsCAVLC_H264)(
                            pDCBuf,
                            &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                            &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                            &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                            &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                            curr_slice->Block_RLE[RLE_Offset].iLevels,
                            curr_slice->Block_RLE[RLE_Offset].uRuns);
                }
                H264ENC_MAKE_NAME(ippiTransformDequantChromaDC422_H264)(pDCBuf, uMBQP, NULL);
           }else{
                H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                    pSrcPlane + uOffset,    // source pels
                    pitchPixels,                 // source pitch
                    pPredBuf,               // predictor pels
                    16,
                    pDCBuf,                 // result buffer
                    pMassDiffBuf);
                H264ENC_MAKE_NAME(ippiTransformQuantChromaDC_H264)(
                    pDCBuf,
                    pQBuf,
                    uMBQP,
                    &iNumCoeffs,
                    (slice_type == INTRASLICE),
                    1,
                    NULL);
                // DC values in this block if iNonEmpty is 1.
                cur_mb.MacroblockCoeffsInfo->chromaNC |= (iNumCoeffs != 0);
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                    Ipp32s ctxIdxBlockCat = BLOCK_CHROMA_DC_LEVELS;
                        H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,4,dec_single_scan_p,&curr_slice->Block_CABAC[RLE_Offset]);
                }else{
                       H264ENC_MAKE_NAME(ippiEncodeChromaDcCoeffsCAVLC_H264)(
                           pDCBuf,
                           &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                           &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                           &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                           &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                           curr_slice->Block_RLE[RLE_Offset].iLevels,
                           curr_slice->Block_RLE[RLE_Offset].uRuns);
                }
                H264ENC_MAKE_NAME(ippiTransformDequantChromaDC_H264)(pDCBuf, uMBQP, NULL);
           }

//Encode croma AC
       Ipp32s coeffsCost = 0;
       pPredBuf_copy = pPredBuf;
       for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;     // This will be updated if the block is coded
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
            } else {
                curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
            }
                 pTempDiffBuf = pMassDiffBuf + (uBlock-startBlock)*16;
                 pTransformResult = pTransform + (uBlock-startBlock)*16;
                 H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                     pTempDiffBuf,
                     pTransformResult,
                     uMBQP,
                     &iNumCoeffs,
                     0,
                     enc_single_scan[is_cur_mb_field],
                     &iLastCoeff,
                     NULL,
                     NULL,
                     0,
                     NULL);
                 coeffsCost += H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 15, &dec_single_scan[is_cur_mb_field][1]);

                    // if everything quantized to zero, skip RLE
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    if (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]){  // the block is empty so it is not coded
                       if (core_enc->m_PicParamSet.entropy_coding_mode){
                            T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                            c_data->uLastSignificant = iLastCoeff;
                            c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock];
                            c_data->CtxBlockCat = BLOCK_CHROMA_AC_LEVELS;
                            c_data->uFirstCoeff = 1;
                            c_data->uLastCoeff = 15;
                            H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                        } else {
                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264) (pTransformResult,// pDiffBuf,
                                                        1,
                                                        dec_single_scan[is_cur_mb_field],
                                                        iLastCoeff,
                                                        &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                                                        &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                                                        &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                                                        &curr_slice->Block_RLE[uBlock].uTotalZeros,
                                                        curr_slice->Block_RLE[uBlock].iLevels,
                                                        curr_slice->Block_RLE[uBlock].uRuns);

                            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                        }
                    }
                pPredBuf += chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock]; //!!!
       }

       if(coeffsCost <= (CHROMA_COEFF_MAX_COST<<(core_enc->m_PicParamSet.chroma_format_idc-1)) ){ //Reset all ac coeffs
//           memset( pTransform, 0, (64*sizeof(COEFFSTYPE))<<(core_enc->m_PicParamSet.chroma_format_idc-1));
           for(uBlock = startBlock; uBlock < uLastBlock; uBlock++){
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                   curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
                } else {
                    curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                    curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
                }
           }
       }


       pPredBuf = pPredBuf_copy;
       for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
                   cur_mb.MacroblockCoeffsInfo->chromaNC |= 2*(cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0);
                    if (!cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] && !pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ]){
                        uCBPChroma &= ~CBP4x4Mask[uBlock-16];
                        Copy4x4(pPredBuf, 16, pReconBuf, 16);
                    }else {
                            H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                                pPredBuf,
                                pTransform + (uBlock-startBlock)*16,
                                &pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ],
                                pReconBuf,
                                16,
                                16,
                                uMBQP,
                                (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0),
                                core_enc->m_SeqParamSet.bit_depth_chroma,
                                NULL);
                    }
                Ipp32s inc = chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock];
                pPredBuf += inc; //!!!
                pReconBuf += inc;
            }   // for uBlock in chroma plane
    } while (uBlock < uFinalBlock);
    uCBPChroma &= ~(0xffffffff<<(uBlock-16));

    cur_mb.LocalMacroblockInfo->cbp_chroma = uCBPChroma;

    if (cur_mb.MacroblockCoeffsInfo->chromaNC == 3)
        cur_mb.MacroblockCoeffsInfo->chromaNC = 2;

    if ((cur_mb.GlobalMacroblockInfo->mbtype != MBTYPE_INTRA_16x16) && (cur_mb.GlobalMacroblockInfo->mbtype!= MBTYPE_PCM)){
        cur_mb.LocalMacroblockInfo->cbp = (cur_mb.MacroblockCoeffsInfo->chromaNC << 4);
    } else  {
        cur_mb.LocalMacroblockInfo->cbp = 0;
    }
}

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantChromaInter_RD)(
    void* state,
    H264SliceType *curr_slice)
{
    H264CoreEncoderType* core_enc = (H264CoreEncoderType *)state;
    Ipp32u  uBlock;         // block number, 0 to 23
    Ipp32u  uOffset;        // to upper left corner of block from start of plane
    Ipp32u  uMBQP;          // QP of current MB
    Ipp32u  uMB;
    PIXTYPE*  pSrcPlane;    // start of plane to encode
    Ipp32s    pitchPixels;  // buffer pitch
    COEFFSTYPE *pDCBuf;     // chroma & luma dc coeffs pointer
    PIXTYPE*  pPredBuf;     // prediction block pointer
    PIXTYPE*  pReconBuf;     // prediction block pointer
    PIXTYPE*  pPredBuf_copy;     // prediction block pointer
    COEFFSTYPE* pQBuf;      // quantized block pointer
    Ipp16s* pMassDiffBuf;   // difference block pointer

    Ipp32u   uCBPChroma;    // coded flags for all chroma blocks
    Ipp32s   iNumCoeffs;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   iLastCoeff;    // Number of nonzero coeffs after quant (negative if DC is nonzero)
    Ipp32s   RLE_Offset;    // Index into BlockRLE array

    H264CurrentMacroblockDescriptorType &cur_mb = curr_slice->m_cur_mb;
    Ipp32s is_cur_mb_field = curr_slice->m_is_cur_mb_field;
    EnumSliceType slice_type = curr_slice->m_slice_type;
    COEFFSTYPE *pTransformResult;
    COEFFSTYPE *pTransform;
    bool  VPlane;

    pitchPixels = cur_mb.mbPitchPixels;
    uMBQP       = cur_mb.chromaQP;
    pTransform = (COEFFSTYPE*)curr_slice->m_pMBEncodeBuffer;
    pQBuf       = (COEFFSTYPE*) (pTransform + 64*2);
    pDCBuf      = (COEFFSTYPE*) (pQBuf + 16);   // Used for both luma and chroma DC blocks
    pMassDiffBuf= (Ipp16s*) (pDCBuf+ 16);
    Ipp16s*  pTempDiffBuf;
    uMB = cur_mb.uMB;

    // initialize pointers and offset
    uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
    uCBPChroma  = cur_mb.LocalMacroblockInfo->cbp_chroma;

    cur_mb.MacroblockCoeffsInfo->chromaNC = 0;

    // initialize pointers for the U plane blocks
    Ipp32s num_blocks = 2<<core_enc->m_PicParamSet.chroma_format_idc;
    Ipp32s startBlock;
    startBlock = uBlock = 16;
    Ipp32u uLastBlock = uBlock+num_blocks;
    Ipp32u uFinalBlock = uBlock+2*num_blocks;

    pPredBuf = cur_mb.mbChromaInter.prediction;
    H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBChroma)(state, curr_slice, pPredBuf);
    // encode first chroma U plane then V plane
    do
    {
        // Adjust the pPredBuf to point at the V plane predictor when appropriate:
        // (blocks and an INTRA Y plane mode...)
        if (uBlock == uLastBlock) {
            startBlock = uBlock;
            uOffset = core_enc->m_pMBOffsets[uMB].uChromaOffset[core_enc->m_is_cur_pic_afrm][is_cur_mb_field];
            pSrcPlane = core_enc->m_pCurrentFrame->m_pVPlane;
            pPredBuf = cur_mb.mbChromaInter.prediction+8;
            pReconBuf = cur_mb.mbChromaInter.reconstruct+8;
            RLE_Offset = V_DC_RLE;
            // initialize pointers for the V plane blocks
            uLastBlock += num_blocks;
            VPlane = true;
        } else {
            RLE_Offset = U_DC_RLE;
            pSrcPlane = core_enc->m_pCurrentFrame->m_pUPlane;
            pPredBuf = cur_mb.mbChromaInter.prediction;
            pReconBuf = cur_mb.mbChromaInter.reconstruct;
            VPlane = false;
        }
        if( core_enc->m_PicParamSet.chroma_format_idc == 2 ){
            H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                pSrcPlane + uOffset,    // source pels
                pitchPixels,                 // source pitch
                pPredBuf,               // predictor pels
                16,
                pDCBuf,                 // result buffer
                pMassDiffBuf);
            // Process second part of 2x4 block for DC coeffs
            H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                pSrcPlane + uOffset+8*pitchPixels,    // source pels
                pitchPixels,                 // source pitch
                pPredBuf+8*16,               // predictor pels
                16,
                pDCBuf+4,                 // result buffer
                pMassDiffBuf+64);   //+Offset for second path
            H264ENC_MAKE_NAME(ippiTransformQuantChroma422DC_H264)(
                pDCBuf,
                pQBuf,
                uMBQP,
                &iNumCoeffs,
                (slice_type == INTRASLICE),
                1,
                NULL);
             // DC values in this block if iNonEmpty is 1.
             cur_mb.MacroblockCoeffsInfo->chromaNC |= (iNumCoeffs != 0);
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                Ipp32s ctxIdxBlockCat = BLOCK_CHROMA_DC_LEVELS;
                H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,8,dec_single_scan_p422,&curr_slice->Block_CABAC[RLE_Offset]);
            }else{
                    H264ENC_MAKE_NAME(ippiEncodeChroma422DC_CoeffsCAVLC_H264)(
                        pDCBuf,
                        &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                        &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                        &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                        &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                        curr_slice->Block_RLE[RLE_Offset].iLevels,
                        curr_slice->Block_RLE[RLE_Offset].uRuns);
            }
            H264ENC_MAKE_NAME(ippiTransformDequantChromaDC422_H264)(pDCBuf, uMBQP, NULL);
       }else{
            H264ENC_MAKE_NAME(ippiSumsDiff8x8Blocks4x4)(
                pSrcPlane + uOffset,    // source pels
                pitchPixels,                 // source pitch
                pPredBuf,               // predictor pels
                16,
                pDCBuf,                 // result buffer
                pMassDiffBuf);
            H264ENC_MAKE_NAME(ippiTransformQuantChromaDC_H264)(
                pDCBuf,
                pQBuf,
                uMBQP,
                &iNumCoeffs,
                (slice_type == INTRASLICE),
                1,
                NULL);
            // DC values in this block if iNonEmpty is 1.
            cur_mb.MacroblockCoeffsInfo->chromaNC |= (iNumCoeffs != 0);
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                Ipp32s ctxIdxBlockCat = BLOCK_CHROMA_DC_LEVELS;
                    H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)(pDCBuf,ctxIdxBlockCat,4,dec_single_scan_p,&curr_slice->Block_CABAC[RLE_Offset]);
            }else{
                   H264ENC_MAKE_NAME(ippiEncodeChromaDcCoeffsCAVLC_H264)(
                       pDCBuf,
                       &curr_slice->Block_RLE[RLE_Offset].uTrailing_Ones,
                       &curr_slice->Block_RLE[RLE_Offset].uTrailing_One_Signs,
                       &curr_slice->Block_RLE[RLE_Offset].uNumCoeffs,
                       &curr_slice->Block_RLE[RLE_Offset].uTotalZeros,
                       curr_slice->Block_RLE[RLE_Offset].iLevels,
                       curr_slice->Block_RLE[RLE_Offset].uRuns);
            }
            H264ENC_MAKE_NAME(ippiTransformDequantChromaDC_H264)(pDCBuf, uMBQP, NULL);
       }

//Encode croma AC
#ifdef H264_RD_TRELLIS
//Save current cabac state
/*      CabacStates cbSt;
        memcpy( cbSt.absLevelM1, &curr_slice->m_pbitstream->context_array[227+39], 10*sizeof(CABAC_CONTEXT));
        if( !is_cur_mb_field ){
            memcpy( cbSt.sig+1, &curr_slice->m_pbitstream->context_array[105+47], 14*sizeof(CABAC_CONTEXT));
            memcpy( cbSt.last+1, &curr_slice->m_pbitstream->context_array[166+47], 14*sizeof(CABAC_CONTEXT));
        }else{
            memcpy( cbSt.sig+1, &curr_slice->m_pbitstream->context_array[105+172+47], 14*sizeof(CABAC_CONTEXT));
            memcpy( cbSt.last+1, &curr_slice->m_pbitstream->context_array[166+172+47], 14*sizeof(CABAC_CONTEXT));
        }
*/
#endif
       Ipp32s coeffsCost = 0;
       pPredBuf_copy = pPredBuf;
       for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;     // This will be updated if the block is coded
            if (core_enc->m_PicParamSet.entropy_coding_mode){
                curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
            } else {
                curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
            }

            // check if block is coded
                 pTempDiffBuf = pMassDiffBuf + (uBlock-startBlock)*16;
                 pTransformResult = pTransform + (uBlock-startBlock)*16;
                 H264ENC_MAKE_NAME(ippiTransformQuantResidual_H264)(
                     pTempDiffBuf,
                     pTransformResult,
                     uMBQP,
                     &iNumCoeffs,
                     0,
                     enc_single_scan[is_cur_mb_field],
                     &iLastCoeff,
                     NULL,
                     NULL,
                     0,
                     NULL);
                 coeffsCost += H264ENC_MAKE_NAME(CalculateCoeffsCost)(pTransformResult, 15, &dec_single_scan[is_cur_mb_field][1]);

                    // if everything quantized to zero, skip RLE
                    cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = (T_NumCoeffs)((iNumCoeffs < 0) ? -(iNumCoeffs+1) : iNumCoeffs);
                    if (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]){  // the block is empty so it is not coded
                       if (core_enc->m_PicParamSet.entropy_coding_mode){
                            T_Block_CABAC_DataType* c_data = &curr_slice->Block_CABAC[uBlock];
                            c_data->uLastSignificant = iLastCoeff;
                            c_data->uNumSigCoeffs = cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock];
                            c_data->CtxBlockCat = BLOCK_CHROMA_AC_LEVELS;
                            c_data->uFirstCoeff = 1;
                            c_data->uLastCoeff = 15;
                            H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)(pTransformResult,dec_single_scan[is_cur_mb_field],&curr_slice->Block_CABAC[uBlock]);
                            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_CABAC[uBlock].uNumSigCoeffs;
                        } else {
                            H264ENC_MAKE_NAME(ippiEncodeCoeffsCAVLC_H264) (pTransformResult,// pDiffBuf,
                                                        1,
                                                        dec_single_scan[is_cur_mb_field],
                                                        iLastCoeff,
                                                        &curr_slice->Block_RLE[uBlock].uTrailing_Ones,
                                                        &curr_slice->Block_RLE[uBlock].uTrailing_One_Signs,
                                                        &curr_slice->Block_RLE[uBlock].uNumCoeffs,
                                                        &curr_slice->Block_RLE[uBlock].uTotalZeros,
                                                        curr_slice->Block_RLE[uBlock].iLevels,
                                                        curr_slice->Block_RLE[uBlock].uRuns);

                            cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = curr_slice->Block_RLE[uBlock].uNumCoeffs;
                        }
                    }
                pPredBuf += chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock]; //!!!
       }

       if(coeffsCost <= (CHROMA_COEFF_MAX_COST<<(core_enc->m_PicParamSet.chroma_format_idc-1)) ){ //Reset all ac coeffs
//           if(cur_mb.MacroblockCoeffsInfo->chromaNC&1) //if we have DC coeffs
//           memset( pTransform, 0, (64*sizeof(COEFFSTYPE))<<(core_enc->m_PicParamSet.chroma_format_idc-1));
           for(uBlock = startBlock; uBlock < uLastBlock; uBlock++){
                cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] = 0;
                if (core_enc->m_PicParamSet.entropy_coding_mode){
                   curr_slice->Block_CABAC[uBlock].uNumSigCoeffs = 0;
                } else {
                    curr_slice->Block_RLE[uBlock].uNumCoeffs = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_Ones = 0;
                    curr_slice->Block_RLE[uBlock].uTrailing_One_Signs = 0;
                    curr_slice->Block_RLE[uBlock].uTotalZeros = 15;
                }
           }
       }
//#endif

       pPredBuf = pPredBuf_copy;
       for (uBlock = startBlock; uBlock < uLastBlock; uBlock ++) {
                    cur_mb.MacroblockCoeffsInfo->chromaNC |= 2*(cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0);

                    if (!cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock] && !pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ]){
                        uCBPChroma &= ~CBP4x4Mask[uBlock-16];
                        Copy4x4(pPredBuf, 16, pReconBuf, 16);
                    }else{
                            H264ENC_MAKE_NAME(ippiDequantTransformResidualAndAdd_H264)(
                                pPredBuf,
                                pTransform + (uBlock-startBlock)*16,
                                &pDCBuf[ chromaDCOffset[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock] ],
                                pReconBuf,
                                16,
                                16,
                                uMBQP,
                                (cur_mb.MacroblockCoeffsInfo->numCoeff[uBlock]!=0),
                                core_enc->m_SeqParamSet.bit_depth_chroma,
                                NULL);
                    }
                Ipp32s inc = chromaPredInc[core_enc->m_PicParamSet.chroma_format_idc-1][uBlock-startBlock];
                pPredBuf += inc; //!!!
                pReconBuf += inc; //!!!
            }   // for uBlock in chroma plane
    } while (uBlock < uFinalBlock);

    //Reset other chroma
    uCBPChroma &= ~(0xffffffff<<(uBlock-16));

    cur_mb.LocalMacroblockInfo->cbp_chroma = uCBPChroma;

    if (cur_mb.MacroblockCoeffsInfo->chromaNC == 3)
        cur_mb.MacroblockCoeffsInfo->chromaNC = 2;
}


#undef T_Block_CABAC_DataType
#undef H264CurrentMacroblockDescriptorType
#undef H264SliceType
#undef H264CoreEncoderType
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
