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

#define sH264SliceType H264ENC_MAKE_NAME(sH264Slice)
#define H264SliceType H264ENC_MAKE_NAME(H264Slice)
#define sH264CoreEncoderType H264ENC_MAKE_NAME(sH264CoreEncoder)
#define H264CoreEncoderType H264ENC_MAKE_NAME(H264CoreEncoder)
#define DeblockingParametersType H264ENC_MAKE_NAME(DeblockingParameters)
#define DeblockingParametersMBAFFType H264ENC_MAKE_NAME(DeblockingParametersMBAFF)
#define H264CurrentMacroblockDescriptorType H264ENC_MAKE_NAME(H264CurrentMacroblockDescriptor)
#define T_RLE_DataType H264ENC_MAKE_NAME(T_RLE_Data)
#define T_Block_CABAC_DataType H264ENC_MAKE_NAME(T_Block_CABAC_Data)
#define ME_InfType H264ENC_MAKE_NAME(ME_Inf)
#define H264EncoderFrameType H264ENC_MAKE_NAME(H264EncoderFrame)
#define H264EncoderFrameListType H264ENC_MAKE_NAME(H264EncoderFrameList)
#define EncoderRefPicListType H264ENC_MAKE_NAME(EncoderRefPicList)
#define EncoderRefPicListStructType H264ENC_MAKE_NAME(EncoderRefPicListStruct)
#define H264BsRealType H264ENC_MAKE_NAME(H264BsReal)
#define H264BsFakeType H264ENC_MAKE_NAME(H264BsFake)

typedef struct H264ENC_MAKE_NAME(sDeblockingParameters) DeblockingParametersType;
typedef struct H264ENC_MAKE_NAME(sDeblockingParametersMBAFF) DeblockingParametersMBAFFType;

//public:
Status H264ENC_MAKE_NAME(H264Slice_Create)(
    void* state);

Status H264ENC_MAKE_NAME(H264Slice_Init)(
    void* state,
    H264EncoderParams& info); // Must be called once as parameters are available.

void H264ENC_MAKE_NAME(H264Slice_Destroy)(
    void* state);

typedef struct sH264SliceType
{
//public:
    EnumSliceType m_slice_type; // Type of the current slice.
    Ipp32s        m_slice_number; // Number of the current slice.
    Ipp32s        status;     //Return value from Compress_Slice function
    Ipp8s         m_iLastXmittedQP;
    Ipp32u      m_MB_Counter;
    Ipp32u      m_Intra_MB_Counter;
    Ipp32u      m_uSkipRun;
    Ipp32s      m_prev_dquant;

#ifdef ALT_RC
    //For Rate Control
    Ipp64f      m_Sad_Sum;
    Ipp64s      m_Texture_Sum;
#endif

    Ipp32s      m_is_cur_mb_field;
    bool        m_is_cur_mb_bottom_field;
    Ipp32s      m_first_mb_in_slice;
    Ipp8s       m_slice_qp_delta;            // delta between slice QP and picture QP
    Ipp8u       m_cabac_init_idc;            // CABAC initialization table index (0..2)
    bool        m_use_transform_for_intra_decision;

    H264CurrentMacroblockDescriptorType m_cur_mb;

    H264BsBase*     m_pbitstream; // Where this slice is encoded to.
    H264BsFakeType* fakeBitstream;
    H264BsFakeType* fBitstreams[9]; //For INTRA mode selection

    Ipp8u       m_disable_deblocking_filter_idc; // deblock filter control, 0=filter all edges
    Ipp8s       m_slice_alpha_c0_offset;         // deblock filter c0, alpha table offset
    Ipp8s       m_slice_beta_offset;             // deblock filter beta table offset
    Ipp32s     *m_InitialOffset;
    Ipp32s      m_NumRefsInL0List;
    Ipp32s      m_NumRefsInL1List;
    Ipp32s      m_NumRefsInLTList;
    Ipp8u       num_ref_idx_active_override_flag;   // nonzero: use ref_idx_active from slice header
    Ipp32s      num_ref_idx_l0_active;              // num of ref pics in list 0 used to decode the slice,
    Ipp32s      num_ref_idx_l1_active;              // num of ref pics in list 1 used to decode the slice

    // MB work buffer, allocated buffer pointer for freeing
    Ipp8u*      m_pAllocatedMBEncodeBuffer;
    // m_pAllocatedMBEncodeBuffer is mapped onto the following pointers.
    PIXTYPE*    m_pPred4DirectB;      // the 16x16 MB prediction for direct B mode
    PIXTYPE*    m_pPred4BiPred;       // the 16x16 MB prediction for BiPredicted B Mode
    PIXTYPE*    m_pTempBuff4DirectB;  // 16x16 working buffer for direct B
    PIXTYPE*    m_pTempBuff4BiPred;  // 16x16 working buffer for BiPred B
    PIXTYPE*    m_pTempChromaPred;  // 16x16 working buffer for chroma pred B
    PIXTYPE*    m_pMBEncodeBuffer;    // temp work buffer

    // Buffers for CABAC.
    T_RLE_DataType Block_RLE[51];       // [0-15] Luma, [16-31] Chroma U/Luma1, [32-47] Chroma V/Luma2, [48] Chroma U DC/Luma1 DC, [49] Chroma V DC/Luma2 DC, [50] Luma DC
    T_Block_CABAC_DataType Block_CABAC[51];  // [0-15] Luma, [16-31] Chroma U/Luma1, [32-47] Chroma V/Luma2, [48] Chroma U DC/Luma1 DC, [49] Chroma V DC/Luma2 DC, [50] Luma DC

    EncoderRefPicListType m_TempRefPicList[2][2];

    Ipp32s      MapColMBToList0[MAX_NUM_REF_FRAMES][2];
    Ipp32s      DistScaleFactor[MAX_NUM_REF_FRAMES][MAX_NUM_REF_FRAMES];
    Ipp32s      DistScaleFactorMV[MAX_NUM_REF_FRAMES][MAX_NUM_REF_FRAMES];
    Ipp32s      DistScaleFactorAFF[2][2][2][MAX_NUM_REF_FRAMES]; // [curmb field],[ref1field],[ref0field]
    Ipp32s      DistScaleFactorMVAFF[2][2][2][MAX_NUM_REF_FRAMES]; // [curmb field],[ref1field],[ref0field]

} H264SliceType;

inline EncoderRefPicListStructType* H264ENC_MAKE_NAME(GetRefPicList)(
    H264SliceType* curr_slice,
    Ipp32u List,
    Ipp32s mb_cod_type,
    Ipp32s is_bottom_mb)
{
    EncoderRefPicListStructType *pList;
    if (List == LIST_0)
        pList = &curr_slice->m_TempRefPicList[mb_cod_type][is_bottom_mb].m_RefPicListL0;
    else
        pList = &curr_slice->m_TempRefPicList[mb_cod_type][is_bottom_mb].m_RefPicListL1;
    return pList;
}

/* public: */
Status H264ENC_MAKE_NAME(H264CoreEncoder_Create)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_Destroy)(
    void* state);

// Initialize codec with specified parameter(s)
Status H264ENC_MAKE_NAME(H264CoreEncoder_Init)(
    void* state,
    BaseCodecParams *init,
    MemoryAllocator *pMemAlloc);

// Compress (decompress) next frame
Status H264ENC_MAKE_NAME(H264CoreEncoder_GetFrame)(
    void* state,
    MediaData *in,
    MediaData *out);

// Get codec working (initialization) parameter(s)
Status H264ENC_MAKE_NAME(H264CoreEncoder_GetInfo)(
    void* state,
    BaseCodecParams *info);

const H264PicParamSet* H264ENC_MAKE_NAME(H264CoreEncoder_GetPicParamSet)(void* state);
const H264SeqParamSet* H264ENC_MAKE_NAME(H264CoreEncoder_GetSeqParamSet)(void* state);

// Close all codec resources
Status H264ENC_MAKE_NAME(H264CoreEncoder_Close)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Reset)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_SetParams)(
    void* state,
    BaseCodecParams* params);

VideoData* H264ENC_MAKE_NAME(H264CoreEncoder_GetReconstructedFrame)(
    void* state);

/* protected: */
Status H264ENC_MAKE_NAME(H264CoreEncoder_CheckEncoderParameters)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetLeftLocationForCurrentMBLumaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetLeftLocationForCurrentMBChromaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLocationForCurrentMBLumaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLocationForCurrentMBChromaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLeftLocationForCurrentMBLumaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopRightLocationForCurrentMBLumaNonMBAFF)( //stateless
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetLeftLocationForCurrentMBLumaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block,
    Ipp32s AdditionalDecrement/* = 0*/);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetLeftLocationForCurrentMBChromaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLocationForCurrentMBLumaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block,
    bool is_deblock_calls);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLocationForCurrentMBChromaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopLeftLocationForCurrentMBLumaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetTopRightLocationForCurrentMBLumaMBAFF)(
    void* state,
    H264CurrentMacroblockDescriptorType& cur_mb,
    H264BlockLocation *Block);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_GetColocatedLocation)(
    void* state,
    H264SliceType *curr_slice,
    H264EncoderFrameType *pRefFrame,
    Ipp8u Field,
    Ipp8s& block,
    Ipp8s *scale/* = 0*/);

void H264ENC_MAKE_NAME(H264CoreEncoder_UpdateCurrentMBInfo)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_MBFrameFieldSelect)(
    void* state,
    H264SliceType *curr_slice);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_ComputeMBFrameFieldCost)(
    void* state,
    H264SliceType *curr_slice,
    bool is_frame);

// A former part of UpdateRefPicList() which does not depend on slice type.
void H264ENC_MAKE_NAME(H264CoreEncoder_UpdateRefPicListCommon)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_UpdateRefPicList)(
    void* state,
    H264SliceType *curr_slice,
    EncoderRefPicListType *ref_pic_list,
    H264SliceHeader &SHdr,
    RefPicListReorderInfo *pReorderInfo_L0,
    RefPicListReorderInfo *pReorderInfo_L1);

void H264ENC_MAKE_NAME(H264CoreEncoder_InitPSliceRefPicList)(
    void* state,
    H264SliceType *curr_slice,
    bool bIsFieldSlice,
    H264EncoderFrameType **pRefPicList);    // pointer to start of list 0

void H264ENC_MAKE_NAME(H264CoreEncoder_InitBSliceRefPicLists)(
    void* state,
    H264SliceType *curr_slice,
    bool bIsFieldSlice,
    H264EncoderFrameType **pRefPicList0,    // pointer to start of list 0
    H264EncoderFrameType **pRefPicList1);   // pointer to start of list 1

void H264ENC_MAKE_NAME(H264CoreEncoder_InitDistScaleFactor)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s NumL0RefActive,
    Ipp32s NumL1RefActive,
    H264EncoderFrameType **pRefPicList0,
    H264EncoderFrameType **pRefPicList1,
    Ipp8s *pFields0,
    Ipp8s *pFields1);

void H264ENC_MAKE_NAME(H264CoreEncoder_InitMapColMBToList0)( //stateless
    H264SliceType *curr_slice,
    Ipp32s NumL0RefActive,
    H264EncoderFrameType **pRefPicList0,
    H264EncoderFrameType **pRefPicList1);

void H264ENC_MAKE_NAME(H264CoreEncoder_AdjustRefPicListForFields)(
    void* state,
    H264EncoderFrameType **pRefPicList,
    Ipp8s *pFields);

void H264ENC_MAKE_NAME(H264CoreEncoder_ReOrderRefPicList)(
    void* state,
    bool bIsFieldSlice,
    H264EncoderFrameType **pRefPicList,
    Ipp8s *pFields,
    RefPicListReorderInfo *pReorderInfo,
    Ipp32s MaxPicNum,
    Ipp32s NumRefActive);

Status H264ENC_MAKE_NAME(H264CoreEncoder_UpdateRefPicMarking)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_CompressFrame)(
    void* state,
    EnumPicCodType&,
    EnumPicClass&,
    MediaData* outBitstream);

// Encodes blank frame when overflow
Status H264ENC_MAKE_NAME(H264CoreEncoder_EncodeDummyFrame)(
    void* state,
    MediaData* dst);

EnumPicCodType H264ENC_MAKE_NAME(H264CoreEncoder_DetermineFrameType)(
    void* state,
    Ipp32s);

//Do we need to start new picture?
Status H264ENC_MAKE_NAME(H264CoreEncoder_encodeFrameHeader)(
    void* state,
    H264BsRealType*,
    MediaData* dst,
    bool bIDR_Pic,
    bool& startPicture);

void H264ENC_MAKE_NAME(H264CoreEncoder_SetSequenceParameters)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_SetPictureParameters)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_SetDPBSize)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_SetSliceHeaderCommon)(
    void* state,
    H264EncoderFrameType*);

void H264ENC_MAKE_NAME(H264CoreEncoder_InferFDFForSkippedMBs)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_MoveFromCPBToDPB)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_CleanDPB)(
    void* state);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Encode)(
    void* state,
    VideoData* src,
    MediaData* dst,
    const H264_Encoder_Compression_Flags,
    H264_Encoder_Compression_Notes&);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Start_Picture)(
    void* state,
    const EnumPicClass* pic_class,
    EnumPicCodType pic_type);

// Compress picture slice
Status H264ENC_MAKE_NAME(H264CoreEncoder_Compress_Slice)(
    void* state,
    H264SliceType *curr_slice,
    bool is_first_mb);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_MB_Decision)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s uMB);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Compress_Slice_MBAFF)(
    void* state,
    H264SliceType *curr_slice);

// any processing needed after each picture
void H264ENC_MAKE_NAME(H264CoreEncoder_End_Picture)(
    void* state);

/* private: */
void H264ENC_MAKE_NAME(H264CoreEncoder_InitializeMBData)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_Make_MBSlices)(
    void* state);

void H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBestSAD,        //Best previous SAD
    Ipp32u *puAIMBSAD);     // return total MB SAD here

void H264ENC_MAKE_NAME(H264CoreEncoder_Encode4x4IntraBlock)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s block);

void H264ENC_MAKE_NAME(H264CoreEncoder_Encode8x8IntraBlock)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32s block);

void H264ENC_MAKE_NAME(H264CoreEncoder_EncodeChroma)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneBlock)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pSrcBlock,     // pointer to upper left pel of source block
    PIXTYPE* pReconBlock,   // pointer to same block in reconstructed picture
    Ipp32u uBlock,          // which 4x4 of the MB (0..15)
    T_AIMode *intra_types,  // selected mode goes here
    PIXTYPE *pPred);        // predictor pels for selected mode goes here

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneMB_16x16)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pSrc,          // pointer to upper left pel of source MB
    PIXTYPE* pRef,          // pointer to same MB in reference picture
    Ipp32s   pitchPixels,   // of source and ref data in pixels
    T_AIMode *pMode,        // selected mode goes here
    PIXTYPE *pPredBuf);     // predictor pels for selected mode goes here

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectChromaMBs_8x8)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pUSrc,         // pointer to upper left pel of U source MB
    PIXTYPE* pURef,         // pointer to same MB in U reference picture
    PIXTYPE* pVSrc,         // pointer to upper left pel of V source MB
    PIXTYPE* pVRef,         // pointer to same MB in V reference picture
    Ipp32u uPitch,          // of source and ref data
    Ipp8u *pMode,           // selected mode goes here
    PIXTYPE *pUPredBuf,     // U predictor pels for selected mode go here
    PIXTYPE *pVPredBuf);    // V predictor pels for selected mode go here

void H264ENC_MAKE_NAME(H264CoreEncoder_GetPredBlock)( //stateless
    Ipp32u uMode,           // advanced intra mode of the block
    PIXTYPE *pPredBuf,
    PIXTYPE* PredPel);      // predictor pels

void H264ENC_MAKE_NAME(H264CoreEncoder_GetBlockPredPels)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pLeftRefBlock,       // pointer to block in reference picture
    Ipp32u uLeftPitch,              // of source data. Pitch in pixels.
    PIXTYPE* pAboveRefBlock,      // pointer to block in reference picture
    Ipp32u uAbovePitch,             // of source data. Pitch in pixels.
    PIXTYPE* pAboveLeftRefBlock,  // pointer to block in reference picture
    Ipp32u uAboveLeftPitch,         // of source data. Pitch in pixels.
    Ipp32u uBlock,                  // 0..15 for luma blocks only
    PIXTYPE* PredPel);              // result here

void H264ENC_MAKE_NAME(H264CoreEncoder_AdvancedIntraModeSelectOneMacroblock8x8)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBestSAD,    // Best previous SAD
    Ipp32u *puAIMBSAD); // return total MB SAD here

void H264ENC_MAKE_NAME(H264CoreEncoder_Filter8x8Pels)( //stateless
    PIXTYPE* pred_pels,
    Ipp32u pred_pels_mask);

void H264ENC_MAKE_NAME(H264CoreEncoder_GetPrediction8x8)(
    void* state,
    T_AIMode mode,
    PIXTYPE* pred_pels,
    Ipp32u pred_pels_mask,
    PIXTYPE* pels);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_AIModeSelectOneMB_8x8)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pSrc,            // pointer to upper left pel of source MB
    PIXTYPE* pRef,            // pointer to same MB in reference picture
    Ipp32s uBlock,
    T_AIMode* pMode,            // selected mode goes here
    PIXTYPE* pPredBuf);       // predictor pels for selected mode goes here

void H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectAndPredict)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u *puAIMBSAD,      // return total MB SAD here
    PIXTYPE *pPredBuf);   // return predictor pels here

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_Intra4x4SelectRD)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pSrcBlock,   // pointer to upper left pel of source block
    PIXTYPE* pReconBlock, // pointer to same block in reconstructed picture
    Ipp32u     uBlock,      // which 4x4 of the MB (0..15)
    T_AIMode*  intra_types, // selected mode goes here
    PIXTYPE* pPred);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_Intra8x8SelectRD)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pSrc,        // pointer to upper left pel of source MB
    PIXTYPE* pRef,        // pointer to same MB in reference picture
    Ipp32s     uBlock,      // 8x8 block number
    T_AIMode*  pMode,       // selected mode goes here
    PIXTYPE* pPredBuf);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_Intra16x16SelectRD)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE  *pSrc,           // pointer to upper left pel of source MB
    PIXTYPE  *pRef,           // pointer to same MB in reference picture
    Ipp32s      pitchPixels,    // of source and ref data
    T_AIMode   *pMode,          // selected mode goes here
    PIXTYPE  *pPredBuf);      // predictor pels for selected mode goes here

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_IntraSelectChromaRD)(
    void* state,
    H264SliceType *curr_slice,
    PIXTYPE* pUSrc,           // pointer to upper left pel of U source MB
    PIXTYPE* pURef,           // pointer to same MB in U reference picture
    PIXTYPE* pVSrc,           // pointer to upper left pel of V source MB
    PIXTYPE* pVRef,           // pointer to same MB in V reference picture
    Ipp32u   uPitch,            // of source and ref data
    Ipp8u*   pMode,             // selected mode goes here
    PIXTYPE *pUPredBuf,       // U predictor pels for selected mode go here
    PIXTYPE *pVPredBuf);

void H264ENC_MAKE_NAME(H264CoreEncoder_ReconstuctCBP)( //stateless
    H264CurrentMacroblockDescriptorType *cur_mb);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MB_Real)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MB_Fake)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBHeader_Real)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBHeader_Fake)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBLuma_Real)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBLuma_Fake)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBChroma_Real)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_Put_MBChroma_Fake)(
    void* state,
    H264SliceType *curr_slice);

Status H264ENC_MAKE_NAME(H264CoreEncoder_PackSubBlockLuma_Real)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBlock);

Status H264ENC_MAKE_NAME(H264CoreEncoder_PackSubBlockLuma_Fake)(
    void* state,
    H264SliceType *curr_slice,
    Ipp32u uBlock);

void H264ENC_MAKE_NAME(H264CoreEncoder_Encode_CBP_Real)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_Encode_CBP_Fake)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_ScanSignificant_CABAC)( //stateless
    COEFFSTYPE coeff[],
    Ipp32s ctxBlockCat,
    Ipp32s numcoeff,
    const Ipp32s* dec_single_scan,
    T_Block_CABAC_DataType* c_data);

void H264ENC_MAKE_NAME(H264CoreEncoder_MakeSignificantLists_CABAC)( //stateless
    COEFFSTYPE* coeff,
    const Ipp32s* dec_single_scan,
    T_Block_CABAC_DataType* c_data);

// Encode and reconstruct macroblock
void H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecMB)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec16x16IntraMB)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRec4x4IntraMB)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_CEncAndRecInterMB)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantIntra16x16_RD)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantIntra_RD)(
    void* state,
    H264SliceType *curr_slice);

Ipp32u H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantInter_RD)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantChromaIntra_RD)(
    void* state,
    H264SliceType *curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_TransQuantChromaInter_RD)(
    void* state,
    H264SliceType *curr_slice);

// luma MB motion comp
void H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBLuma)(
    void* state,
    H264SliceType *curr_slice,
    const H264MotionVector* pMVFwd,   // motion vectors in subpel units
    const H264MotionVector* pMVBwd,   // motion vectors in subpel units
    PIXTYPE* pDst);                   // put the resulting block here

// chroma MB motion comp
void H264ENC_MAKE_NAME(H264CoreEncoder_MCOneMBChroma)(
    void* state,
    H264SliceType* curr_slice,
    PIXTYPE* pDst);

void H264ENC_MAKE_NAME(H264CoreEncoder_CalcMVPredictor)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32u block_idx,
    Ipp32u uList,
    Ipp32u uBlocksWide,
    Ipp32u uBlocksHigh,
    H264MotionVector *pMVPred);

void H264ENC_MAKE_NAME(H264CoreEncoder_ME_CandList16x16)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32s list_id,
    ME_InfType* meInfo,
    Ipp32s refIdx);

bool H264ENC_MAKE_NAME(H264CoreEncoder_CheckSkip)(
    void* state,
    H264SliceType *curr_slice,
    H264MotionVector &skip_vec);

bool H264ENC_MAKE_NAME(H264CoreEncoder_CheckSkipB)(
    void* state,
    H264SliceType* curr_slice);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_ME_P)(
    void* state,
    H264SliceType* curr_slice);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_ME_B)(
    void* state,
    H264SliceType* curr_slice);

void H264ENC_MAKE_NAME(H264CoreEncoder_FrameTypeDetect)(
    void* state);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_MB_P_RDCost)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32s is8x8,
    Ipp32s bestCost);

Ipp32s H264ENC_MAKE_NAME(H264CoreEncoder_MB_B_RDCost)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32s is8x8);

void H264ENC_MAKE_NAME(H264CoreEncoder_ME_CheckCandidate)( //stateless
    ME_InfType* meInfo,
    H264MotionVector& mv);

void H264ENC_MAKE_NAME(H264CoreEncoder_Calc_One_MV_Predictor)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32u uBlock,              // which 4x4 Block (UL Corner, Raster Order)
    Ipp32u uList,               // 0 or 1 for L0 or L1
    Ipp32u uBlocksWide,         // 1, 2, or 4
    Ipp32u uBlocksHigh,         // 1, 2, or 4 (4x16 and 16x4 not permitted)
    H264MotionVector* pMVPred,  // resulting MV predictor
    H264MotionVector* pMVDelta, // resulting MV delta
    bool updateDMV/* = true*/);

void H264ENC_MAKE_NAME(H264CoreEncoder_Skip_MV_Predicted)(
    void* state,
    H264SliceType* curr_slice,
    H264MotionVector* pMVPredicted,
    H264MotionVector* pMVOut);  // Returns Skip MV if not NULL

void H264ENC_MAKE_NAME(H264CoreEncoder_ComputeDirectSpatialRefIdx)(
    void* state,
    H264SliceType* curr_slice,
    T_RefIdx& pRefIndexL0,
    T_RefIdx& pRefIndexL1);

bool H264ENC_MAKE_NAME(H264CoreEncoder_ComputeDirectSpatialMV)(
    void* state,
    H264SliceType* curr_slice,
    H264MacroblockRefIdxs  ref_direct[2],
    H264MacroblockMVs      mvs_direct[2]);     // MVs used returned here.

bool H264ENC_MAKE_NAME(H264CoreEncoder_ComputeDirectTemporalMV)(
    void* state,
    H264SliceType* curr_slice,
    H264MacroblockRefIdxs ref_direct[2],
    H264MacroblockMVs     mvs_direct[2]);     // MVs used returned here.

// pointer to future frame buffer
void H264ENC_MAKE_NAME(H264CoreEncoder_CDirectBOneMB_Interp)(
    void* state,
    H264SliceType* curr_slice,
    const Ipp32u          uBlkIndex,      // first block index
    H264MacroblockRefIdxs ref_direct[2],
    H264MacroblockMVs     mvs_direct[2]); // MVs used returned here.

void H264ENC_MAKE_NAME(H264CoreEncoder_CDirectBOneMB_Interp_Cr)(
    void* state,
    H264SliceType* curr_slice,
    const H264MotionVector* pMVL0,// Fwd motion vectors in subpel units
    const H264MotionVector* pMVL1,// Bwd motion vectors in subpel units
    Ipp8s* pFields0,              //
    Ipp8s* pFields1,              //
    PIXTYPE* pDst,                // put the resulting block here with pitch of 16
    Ipp32s offset,
    IppiSize size);

// Perform deblocking on single slice
void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockSlice)(
    void* state,
    H264SliceType* curr_slice,
    Ipp32u uFirstMB,
    Ipp32u uNumMBs,
    bool bLastSlice/* = false*/);

// Reset deblocking variables
void H264ENC_MAKE_NAME(H264CoreEncoder_ResetDeblockingVariables)(
    void* state,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_ResetDeblockingVariablesMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

// Function to do luma deblocking
void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockLuma)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockLumaVerticalMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockLumaHorizontalMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

// Function to do chroma deblocking
void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockChroma)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockChromaVerticalMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockChromaHorizontalMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockISlice)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersISlice)(
    void* state,
    DeblockingParametersType *pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockISliceMBAFF)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersISliceMBAFF)(
    void* state,
    DeblockingParametersMBAFFType *pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockPSlice)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockPSliceMBAFF)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice)(
    void* state,
    DeblockingParametersType *pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSliceMBAFF)(
    void* state,
    DeblockingParametersMBAFFType *pParams);

// Prepare deblocking parameters for macroblocks from P slice
// MbPart is 16, MbPart of opposite direction is 16
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice16)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from P slice
// MbPart is 8, MbPart of opposite direction is 16
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice8x16)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from P slice
// MbPart is 16, MbPart of opposite direction is 8
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice16x8)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from P slice
// MbParts of both directions are 4
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice4)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice4MBAFFField)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblock from P slice,
// which coded in frame mode, but above macroblock is coded in field mode
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice4MBAFFMixedExternalEdge)(
    void* state,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblock from P slice,
// which coded in frame mode, but left macroblock is coded in field mode
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice4MBAFFComplexFrameExternalEdge)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

// Prepare deblocking parameters for macroblock from P slice,
// which coded in field mode, but left macroblock is coded in frame mode
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersPSlice4MBAFFComplexFieldExternalEdge)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockBSlice)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_DeblockMacroblockBSliceMBAFF)(
    void* state,
    Ipp32u MBAddr);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice)(
    void* state,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSliceMBAFF)(
    void* state,
    DeblockingParametersMBAFFType* pParams);

// Prepare deblocking parameters for macroblocks from B slice
// MbPart is 16, MbPart of opposite direction is 16
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice16)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from B slice
// MbPart is 8, MbPart of opposite direction is 16
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice8x16)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from B slice
// MbPart is 16, MbPart of opposite direction is 8
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice16x8)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

// Prepare deblocking parameters for macroblocks from B slice
// MbParts of both directions are 4
void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice4)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

void H264ENC_MAKE_NAME(H264CoreEncoder_PrepareDeblockingParametersBSlice4MBAFFField)(
    void* state,
    Ipp32u dir,
    DeblockingParametersType* pParams);

typedef struct sH264CoreEncoderType
{
//public:

#ifdef H264_COMMON_ME
    //====================== common ME ===========================================
    MeBase * m_pME;
    MeParams MEParams_r;
    MeParams * MEParams;
    MeFrame ** MeFrameRefList;
    H264EncoderFrameType **ppRefPicList;
    Ipp8s *pFields;
    //============================================================================
#endif

//protected:
    //f
    Ipp32s                        m_Analyse, m_SubME_Algo;
    Ipp32s                        profile_frequency;
    H264LocalMacroblockDescriptor m_mbinfo;

    Ipp32s                        m_HeightInMBs;
    Ipp32s                        m_WidthInMBs;

    MemoryAllocator*              memAlloc;
    Ipp8u                         *m_pParsedDataNew;
    T_EncodeMBOffsets             *m_pMBOffsets;
    EnumPicCodType                *eFrameType;
    H264EncoderFrameType      **eFrameSeq;
    H264BsRealType* m_bs1; // Pointer to the main bitstream.
    IppiSize                        m_PaddedSize;

//public:
    H264EncoderParams               m_info;
    H264_Encoder_Compression_Flags  cflags;
    H264_Encoder_Compression_Notes  cnotes;
    H264EncoderFrameListType        m_cpb;
    H264EncoderFrameListType        m_dpb;
    Ipp32s                          m_dpbSize;
    H264EncoderFrameType*      m_pCurrentFrame;
    H264EncoderFrameType*      m_pLastFrame;     // ptr to last frame
    H264EncoderFrameType*      m_pReconstructFrame;

    // Table to obtain value to advance the 4x4 block offset for the next block.
    Ipp32s                          m_EncBlockOffsetInc[2][48];
    Ipp32s                          m_is_cur_pic_afrm;
    bool                            m_is_mb_data_initialized;

    bool                            m_NeedToCheckMBSliceEdges;
    Ipp32s                          m_field_index;
    Ipp32s                          m_NumShortEntriesInList;
    Ipp32s                          m_NumLongEntriesInList;
    AdaptiveMarkingInfo             m_AdaptiveMarkingInfo;
    RefPicListReorderInfo           m_ReorderInfoL0;
    RefPicListReorderInfo           m_ReorderInfoL1;

    Ipp32s                          m_InitialOffsets[2][2];

    Ipp32s                          m_MaxLongTermFrameIdx;

//protected:
    Ipp8u*                          m_pAllocEncoderInst;

    // flags read by DetermineFrameType while sequencing the profile:
    bool                            m_bMakeNextFrameKey;
    bool                            m_bMakeNextFrameIDR;
    Ipp32s                          m_uIntraFrameInterval;
    Ipp32s                          m_uIDRFrameInterval;
    Ipp32s                          m_l1_cnt_to_start_B;

    H264SliceType *m_Slices; // thread independent slice information.
    Ipp32s m_iProfileIndex;
    Ipp8u* m_pBitStream;   // Pointer to bitstream buffer.
    Ipp32u m_uTRWrapAround;  // Wrap around for TR
    Ipp32u m_uFrames_Num;  // Decode order frame number.
    Ipp32u m_uFrameCounter;
    CNoiseReductionFilter<PIXTYPE>* m_noisepf;
    Ipp32s m_Pitch;
    Ipp64s m_total_bits_encoded; //Bit counter for all stream

//private:
    ///////////////////////////////////////////////////////////////////////
    // Data
    ///////////////////////////////////////////////////////////////////////

    H264BsRealType**    m_pbitstreams; // Array of bitstreams for threads.

    // Which CPU-specific flavor of algorithm to use.
    EnumPicClass            m_PicClass;
    H264SliceHeader         m_SliceHeader;
    H264SeqParamSet         m_SeqParamSet;
    H264PicParamSet         m_PicParamSet;
    EnumPicCodType          m_PicType;
    Ipp32u                  m_FrameNum;
    Ipp32s                  m_FrameNumGap;
    Ipp32s                  m_PicOrderCnt_Accu; // Accumulator to compensate POC resets on IDR frames.
    Ipp32u                  m_PicOrderCnt;
    Ipp32u                  m_PicOrderCntMsb;
    Ipp32u                  m_PicOrderCntLsb;
    Ipp32s                  m_FrameNumOffset;
    Ipp32u                  m_TopFieldPOC;
    Ipp32u                  m_BottomFieldPOC;

    bool use_implicit_weighted_bipred;

    Ipp16u uNumSlices;                  // Number of Slices in the Frame

    Ipp32u m_slice_length;                // Number of Macroblocks in each slice
    // Except the last slice which may have
    // more macroblocks.

    Ipp32u m_uSliceRemainder;         // Number of extra macroblocks in last slice.

    Ipp32u* m_EmptyThreshold; // empty block threshold table.
    Ipp32u* m_DirectBSkipMEThres;
    Ipp32u* m_PSkipMEThres;
    Ipp32s* m_BestOf5EarlyExitThres;

    // ***************************************************************************
    // Rate control related fields.
    // ***************************************************************************

    Ipp64f  FrameRate;
    Ipp32s  BitRate;
    Ipp32s  qscale[3];             // qscale codes for 3 frame types (Ipp32s!)
    Ipp32s  m_DirectTypeStat[2];
    //f
    H264_AVBR avbr;
#ifdef SLICE_CHECK_LIMIT
    Ipp32u  m_MaxSliceSize;
#endif
#ifdef H264_STAT
    H264FramesStat hstats;
#endif

#ifdef FRAME_QP_FROM_FILE
    std::list<char> frame_type;
    std::list<int> frame_qp;
#endif

} H264CoreEncoderType;

void H264ENC_MAKE_NAME(ExpandPlane)(
    PIXTYPE* StartPtr,
    Ipp32s   frameWidth,
    Ipp32s   frameHeight,
    Ipp32s   pitchPixels,
    Ipp32s   pels);

void H264ENC_MAKE_NAME(PlanarPredictLuma)(
    PIXTYPE* pBlock,
    Ipp32u uPitch,
    PIXTYPE* pPredBuf,
    Ipp32s bitDepth);

void H264ENC_MAKE_NAME(PlanarPredictChroma)(
    PIXTYPE* pBlock,
    Ipp32u uPitch,
    PIXTYPE* pPredBuf,
    Ipp32s bitDepth,
    Ipp32s idc);

//-------- H264CoreEncoder<class PixType, class CoeffsType> -----------// end

#undef sH264CoreEncoderType
#undef H264CoreEncoderType
#undef DeblockingParametersType
#undef DeblockingParametersMBAFFType
#undef H264BsRealType
#undef H264BsFakeType
#undef sH264SliceType
#undef H264SliceType
#undef H264CurrentMacroblockDescriptorType
#undef T_RLE_DataType
#undef T_Block_CABAC_DataType
#undef ME_InfType
#undef H264EncoderFrameType
#undef H264EncoderFrameListType
#undef EncoderRefPicListType
#undef EncoderRefPicListStructType
#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
