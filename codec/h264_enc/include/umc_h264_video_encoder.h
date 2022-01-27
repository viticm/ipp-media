//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef __UMC_H264_VIDEO_ENCODER_H__
#define __UMC_H264_VIDEO_ENCODER_H__

#include "ippdefs.h"
#include "umc_video_data.h"
#include "umc_video_encoder.h"
#include "umc_h264_config.h"

namespace UMC
{
// Slice Group definitions
#define MAX_NUM_SLICE_GROUPS 8
#define MAX_SLICE_GROUP_MAP_TYPE 6

    typedef enum {
        H264_BASE_PROFILE     = 66,
        H264_MAIN_PROFILE     = 77,
        H264_EXTENDED_PROFILE = 88,
        H264_HIGH_PROFILE     = 100,
        H264_HIGH10_PROFILE   = 110,
        H264_HIGH422_PROFILE  = 122,
        H264_HIGH444_PROFILE  = 144
    } H264_PROFILE_IDC;

    typedef Ipp32s H264_Key_Frame_Control_Method;

    const H264_Key_Frame_Control_Method      H264_KFCM_AUTO     = 0;
    // Let the encoder decide when to generate key frames.
    // This method typically causes the least number of key frames to
    // be generated.

    const H264_Key_Frame_Control_Method      H264_KFCM_INTERVAL = 1;
    // Generate key frames at a regular interval

    typedef enum {
        H264_RCM_QUANT     = 0,
        H264_RCM_CBR       = 1,
        H264_RCM_VBR       = 2,
        H264_RCM_DEBUG     = 3, // Fix quantizer values, no actual rate control.
        H264_RCM_CBR_SLICE  = 4,
        H264_RCM_VBR_SLICE  = 5
    } H264_Rate_Control_Method;

    typedef struct {
        H264_Rate_Control_Method   method;
        Ipp8s                      quantI;
        Ipp8s                      quantP;
        Ipp8s                      quantB;
    } H264_Rate_Controls;

    typedef struct {
        H264_Key_Frame_Control_Method method;
        Ipp32s                        interval;
        Ipp32s                        idr_interval;
        // 'interval' is meaningful only when method == H264_KFCM_INTERVAL.
        // It specifies the frequency of key frames.  A value of 1000,
        // for example, means that a key frame should be generated
        // approximately every 1000 frames.  A value of 1 means that
        // every frame should be a key frame.
        // The interval must always be >= 1.

    } H264_Key_Frame_Controls;
};

namespace UMC
{

struct SliceGroupInfoStruct
{
    Ipp8u slice_group_map_type;               // 0..6
    // The additional slice group data depends upon map type
    union {
        // type 0
        Ipp32u run_length[MAX_NUM_SLICE_GROUPS];
        // type 2
        struct {
            Ipp32u top_left[MAX_NUM_SLICE_GROUPS-1];
            Ipp32u bottom_right[MAX_NUM_SLICE_GROUPS-1];
        } t1;
        // types 3-5
        struct {
            Ipp8u  slice_group_change_direction_flag;
            Ipp32u slice_group_change_rate;
        } t2;
        // type 6
        struct {
            Ipp32u pic_size_in_map_units;       // number of macroblocks if no field coding
            Ipp8u *pSliceGroupIDMap;            // Id for each slice group map unit
        } t3;
    };
};  // SliceGroupInfoStruct

struct H264VUIParams {
    Ipp8u   aspect_ratio_info_present_flag;
    Ipp8u   aspect_ratio_idc;
    Ipp16u   sar_width;
    Ipp16u   sar_height;

    Ipp8u   overscan_info_present_flag;
    Ipp8u   overscan_appropriate_flag;

    Ipp8u  video_signal_type_present_flag;
    Ipp8u  video_format;
    Ipp8u  video_full_range_flag;
    Ipp8u  colour_description_present_flag;
    Ipp8u  colour_primaries;
    Ipp8u  transfer_characteristics;
    Ipp8u  matrix_coefficients;

    Ipp8u  chroma_loc_info_present_flag;
    Ipp8u  chroma_sample_loc_type_top_field;
    Ipp8u  chroma_sample_loc_type_bottom_field;

    Ipp8u  timing_info_present_flag;
    Ipp32u num_units_in_tick;
    Ipp32u time_scale;
    Ipp8u  fixed_frame_rate_flag;

    Ipp8u  nal_hrd_parameters_present_flag;
    Ipp8u  vcl_hrd_parameters_present_flag;
    Ipp8u  low_delay_hrd_flag;

    Ipp8u  pic_struct_present_flag;
    Ipp8u  bitstream_restriction_flag;
    Ipp8u  motion_vectors_over_pic_boundaries_flag;
    Ipp8u  max_bytes_per_pic_denom;
    Ipp8u  max_bits_per_mb_denom;
    Ipp8u  log2_max_mv_length_horizontal;
    Ipp8u  log2_max_mv_length_vertical;
    Ipp8u  num_reorder_frames;
    Ipp8u  max_dec_frame_buffering;
};

// Sequence parameter set structure, corresponding to the H.264 bitstream definition.
struct H264SeqParamSet
{
    H264_PROFILE_IDC profile_idc;                   // baseline, main, etc.

    Ipp8s       level_idc;
    Ipp8s       constraint_set0_flag;               // nonzero: bitstream obeys all set 0 constraints
    Ipp8s       constraint_set1_flag;               // nonzero: bitstream obeys all set 1 constraints
    Ipp8s       constraint_set2_flag;               // nonzero: bitstream obeys all set 2 constraints
    Ipp8s       constraint_set3_flag;               // nonzero: bitstream obeys all set 3 constraints
    Ipp8s       chroma_format_idc;

    Ipp8s       seq_parameter_set_id;               // id of this sequence parameter set
    Ipp8s       log2_max_frame_num;                 // Number of bits to hold the frame_num
    Ipp8s       pic_order_cnt_type;                 // Picture order counting method

    Ipp8s       delta_pic_order_always_zero_flag;   // If zero, delta_pic_order_cnt fields are
    // present in slice header.
    Ipp8s       frame_mbs_only_flag;                // Nonzero indicates all pictures in sequence
    // are coded as frames (not fields).
    Ipp8s       gaps_in_frame_num_value_allowed_flag;

    Ipp8s       mb_adaptive_frame_field_flag;       // Nonzero indicates frame/field switch
    // at macroblock level
    Ipp8s       direct_8x8_inference_flag;          // Direct motion vector derivation method
    Ipp8s       vui_parameters_present_flag;        // Zero indicates default VUI parameters
    H264VUIParams vui_parameters;                   // VUI parameters if it is going to be used
    Ipp8s       frame_cropping_flag;                // Nonzero indicates frame crop offsets are present.
    Ipp32s      frame_crop_left_offset;
    Ipp32s      frame_crop_right_offset;
    Ipp32s      frame_crop_top_offset;
    Ipp32s      frame_crop_bottom_offset;
    Ipp32s      log2_max_pic_order_cnt_lsb;         // Value of MaxPicOrderCntLsb.
    Ipp32s      offset_for_non_ref_pic;

    Ipp32s      offset_for_top_to_bottom_field;     // Expected pic order count difference from
    // top field to bottom field.

    Ipp32s      num_ref_frames_in_pic_order_cnt_cycle;
    Ipp32s      *poffset_for_ref_frame;             // pointer to array of stored frame offsets,
    // length num_stored_frames_in_pic_order_cnt_cycle,
    // for pic order cnt type 1
    Ipp32s      num_ref_frames;                     // total number of pics in decoded pic buffer
    Ipp32s      frame_width_in_mbs;
    Ipp32s      frame_height_in_mbs;

    // These fields are calculated from values above.  They are not written to the bitstream
    Ipp32s      MaxMbAddress;
    Ipp32s      MaxPicOrderCntLsb;
    Ipp32s      aux_format_idc;                     // See H.264 standard for details.
    Ipp32s      bit_depth_aux;
    Ipp32s      bit_depth_luma;
    Ipp32s      bit_depth_chroma;
    Ipp32s      alpha_incr_flag;
    Ipp32s      alpha_opaque_value;
    Ipp32s      alpha_transparent_value;

    bool        seq_scaling_matrix_present_flag;
    bool        seq_scaling_list_present_flag[8];
    Ipp8u       seq_scaling_list_4x4[6][16];
    Ipp8u       seq_scaling_list_8x8[2][64];

    Ipp16s      seq_scaling_matrix_4x4[6][6][16];
    Ipp16s      seq_scaling_matrix_8x8[2][6][64];
    Ipp16s      seq_scaling_inv_matrix_4x4[6][6][16];
    Ipp16s      seq_scaling_inv_matrix_8x8[2][6][64];

    bool        pack_sequence_extension;
    Ipp32s        qpprime_y_zero_transform_bypass_flag;
    bool        residual_colour_transform_flag;
    Ipp32s      additional_extension_flag;
};  // H264SeqParamSet

// Picture parameter set structure, corresponding to the H.264 bitstream definition.
struct H264PicParamSet
{
    Ipp8s       pic_parameter_set_id;           // of this picture parameter set
    Ipp8s       seq_parameter_set_id;           // of seq param set used for this pic param set
    Ipp8s       entropy_coding_mode;            // zero: CAVLC, else CABAC

    Ipp8s       pic_order_present_flag;         // Zero indicates only delta_pic_order_cnt[0] is
    // present in slice header; nonzero indicates
    // delta_pic_order_cnt[1] is also present.

    Ipp8s       weighted_pred_flag;             // Nonzero indicates weighted prediction applied to
    // P and SP slices
    Ipp8s       weighted_bipred_idc;            // 0: no weighted prediction in B slices
    // 1: explicit weighted prediction
    // 2: implicit weighted prediction
    Ipp8s       pic_init_qp;                    // default QP for I,P,B slices
    Ipp8s       pic_init_qs;                    // default QP for SP, SI slices

    Ipp8s       chroma_qp_index_offset;         // offset to add to QP for chroma

    Ipp8s       deblocking_filter_variables_present_flag; // If nonzero, deblock filter params are
    // present in the slice header.
    Ipp8s       constrained_intra_pred_flag;    // Nonzero indicates constrained intra mode

    Ipp8s       redundant_pic_cnt_present_flag; // Nonzero indicates presence of redundant_pic_cnt
    // in slice header
    Ipp8s       num_slice_groups;               // Usually 1

    Ipp8s       second_chroma_qp_index_offset;

    SliceGroupInfoStruct SliceGroupInfo;        // Used only when num_slice_groups > 1
    Ipp32s      num_ref_idx_l0_active;          // num of ref pics in list 0 used to decode the picture
    Ipp32s      num_ref_idx_l1_active;          // num of ref pics in list 1 used to decode the picture
    bool        transform_8x8_mode_flag;
    bool        pic_scaling_matrix_present_flag; // Only "false" is supported.
    bool        pack_sequence_extension;
    Ipp32s      chroma_format_idc;              // needed for aux/primary picture switch.
    Ipp32s      bit_depth_luma;                 // needed for aux/primary picture switch.
};  // H264PicParamSet

class H264EncoderParams: public VideoEncoderParams
{
    DYNAMIC_CAST_DECL(H264EncoderParams, VideoEncoderParams)

public:
    H264EncoderParams();
    virtual Status ReadParamFile(const vm_char *ParFileName);

    Ipp32s          chroma_format_idc;
    Ipp32s          coding_type; // 0 - only FRM, 1 - only FLD , 2 - only AFRM, 3  - pure PicAFF(no MBAFF) 4 PicAFF + MBAFF
    Ipp32s          B_frame_rate;
    Ipp32s          treat_B_as_reference;
    Ipp32s          num_ref_frames;
    Ipp32s          num_ref_to_start_code_B_slice;
    Ipp8s           level_idc;
    H264_Rate_Controls  rate_controls;
    Ipp16s          num_slices; // Number of slices
    Ipp8s           m_do_weak_forced_key_frames;
    Ipp8s           deblocking_filter_idc;
    Ipp32s          deblocking_filter_alpha;
    Ipp32s          deblocking_filter_beta;
    Ipp32s          mv_search_method;
    Ipp32s          me_split_mode; // 0 - 16x16 only; 1 - 16x16, 16x8, 8x16, 8x8; 2 - could split 8x8.
    Ipp32s          me_search_x;
    Ipp32s          me_search_y;
    Ipp32s          use_weighted_pred;
    Ipp32s          use_weighted_bipred;
    Ipp32s          use_implicit_weighted_bipred;
    Ipp8s           direct_pred_mode; // 1 - spatial, 0 - temporal
    Ipp32s          use_direct_inference;
    Ipp8s           entropy_coding_mode; // 0 - CAVLC, 1 - CABAC
    Ipp8s           cabac_init_idc; // [0..2] used for CABAC
    Ipp8s           write_access_unit_delimiters; // 1 - write, 0 - do not
    H264_Key_Frame_Controls    key_frame_controls;
    bool            use_transform_for_intra_decision;

public:
    H264_PROFILE_IDC profile_idc; // profile_idc
    bool transform_8x8_mode_flag;
    Ipp32s  qpprime_y_zero_transform_bypass_flag;
    Ipp32s  use_default_scaling_matrix;

    Ipp32s  aux_format_idc;
    bool alpha_incr_flag;
    Ipp32s  alpha_opaque_value;
    Ipp32s  alpha_transparent_value;
    Ipp32s  bit_depth_aux;
    Ipp32s  bit_depth_luma;
    Ipp32s  bit_depth_chroma;

    Ipp32s numFramesToEncode;
    Ipp32s m_QualitySpeed;
    Ipp32u quant_opt_level;
    //Additional parameters
#if defined (H264_LOG)
    bool m_log;
    vm_char m_log_file[254];
#endif
};

} // namespace UMC

namespace UMC_H264_ENCODER {
    struct sH264CoreEncoder_8u16s;
    struct sH264CoreEncoder_16u32s;
    typedef struct sH264CoreEncoder_8u16s H264CoreEncoder_8u16s;
    typedef struct sH264CoreEncoder_16u32s H264CoreEncoder_16u32s;
}

using namespace UMC_H264_ENCODER;

namespace UMC {
    typedef enum {
        H264_VIDEO_ENCODER_8U_16S  = 0,
        H264_VIDEO_ENCODER_16U_32S = 1,
        H264_VIDEO_ENCODER_NONE    = 2
    } EncoderType;

    class H264VideoEncoder : public VideoEncoder
    {
    public:
        H264VideoEncoder();
        ~H264VideoEncoder();

    public:
        // Initialize codec with specified parameter(s)
        virtual Status Init(BaseCodecParams *init);
        // Compress (decompress) next frame
        virtual Status GetFrame(MediaData *in, MediaData *out);
        // Get codec working (initialization) parameter(s)
        virtual Status GetInfo(BaseCodecParams *info);

        const H264PicParamSet* GetPicParamSet();
        const H264SeqParamSet* GetSeqParamSet();
        
        // Close all codec resources
        virtual Status Close();

        virtual Status Reset();
        virtual Status SetParams(BaseCodecParams* params);

        VideoData* GetReconstructedFrame();

    protected:
        EncoderType                     m_CurrEncoderType;  // Type of the encoder applicable now.
        void* m_pEncoder_8u_16s;  // For 8 bit video.

#if defined (BITDEPTH_9_12)
        void* m_pEncoder_16u_32s; // For 9-12 bit video.
#endif // BITDEPTH_9_12
    };

} // namespace UMC

#endif // __UMC_H264_VIDEO_ENCODER_H__

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
