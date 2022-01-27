/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __AACCMN_ICS_H
#define __AACCMN_ICS_H

enum eICS
{
  ICS_MAX_SFB           = 51,
  ICS_MAX_NUM_WINDOWS   = 8,
  ICS_MAX_GROUP_NUMBER  = 8,
  ICS_MAX_NUMBER_PULSE  = ((1<<2)+1),
  ICS_MAX_FILT          = 4,
  ICS_TNS_MAX_ORDER     = 32,
};

typedef struct
{
  Ipp32s global_gain;

  /// ics_info
  Ipp32s ics_reserved_bit;
  Ipp32s window_sequence;
  Ipp32s window_shape;

  Ipp32s max_sfb;
  Ipp32s num_swb_long;
  Ipp32s num_swb_short;
  Ipp32s scale_factor_grouping[7];

  Ipp32s num_window_groups;
  Ipp32s len_window_group[8];
  Ipp32s num_windows;

  Ipp32s predictor_data_present;  ///
  Ipp32s predictor_reset;
  Ipp32s predictor_reset_group_number;
  Ipp32s pred_max_sfb;
  Ipp32s prediction_used[41];

  Ipp32s pulse_data_present;
  Ipp32s tns_data_present;
  Ipp32s gain_control_data_present;

  Ipp32s sfb_cb[ICS_MAX_GROUP_NUMBER][ICS_MAX_SFB];
  Ipp16s sf[ICS_MAX_GROUP_NUMBER][ICS_MAX_SFB];

  Ipp16s spectrum_data[1024];
  Ipp16s* p_spectrum[8];

  Ipp32s* sfb_offset_long_window;
  Ipp32s* sfb_offset_short_window;

  void** p_huffman_tables;

  Ipp32s number_pulse;
  Ipp32s pulse_start_sfb;
  Ipp32s pulse_offset[ICS_MAX_NUMBER_PULSE];
  Ipp32s pulse_amp[ICS_MAX_NUMBER_PULSE];

    ///    Tns data
  Ipp32s n_filt[ICS_MAX_NUM_WINDOWS];
  Ipp32s coef_res[ICS_MAX_NUM_WINDOWS];
  Ipp32s length[ICS_MAX_NUM_WINDOWS][ICS_MAX_FILT];
  Ipp32s order[ICS_MAX_NUM_WINDOWS][ICS_MAX_FILT];
  Ipp32s direction[ICS_MAX_NUM_WINDOWS][ICS_MAX_FILT];
  Ipp32s coef_compress[ICS_MAX_NUM_WINDOWS][ICS_MAX_FILT];
  Ipp32s coef[ICS_MAX_NUM_WINDOWS][ICS_MAX_FILT][ICS_TNS_MAX_ORDER];
  Ipp32s tns_max_bands_short;
  Ipp32s tns_max_bands_long;

  /// Gain control data
  Ipp32s max_band;
  Ipp32s adjust_num[4][8];
  Ipp32s alevcode[4][8][8];
  Ipp32s aloccode[4][8][8];

  /// LTP data
  Ipp32s ltp_data_present; ///1 bit
  Ipp32s ltp_lag_update; /// 1 bit
  Ipp32s ltp_lag;  /// 10-11 bits
  Ipp32s ltp_coef; /// 3 bits
  Ipp32s ltp_long_used[ICS_MAX_SFB];
  Ipp32s ltp_short_used[ICS_MAX_NUM_WINDOWS];
  Ipp32s ltp_short_lag_present[ICS_MAX_NUM_WINDOWS];
  Ipp32s ltp_short_lag[ICS_MAX_NUM_WINDOWS];

} sIndividual_channel_stream;

#endif//__AACCMN_ICS_H
