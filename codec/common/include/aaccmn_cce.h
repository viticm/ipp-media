/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __AACCMN_CCE_H
#define __AACCMN_CCE_H

#include "ippdefs.h"

typedef struct
{
  Ipp32s  element_instance_tag;

  Ipp32s ind_sw_cce_flag;         /// 1 bit
  Ipp32s num_coupled_elements;    /// 3 bits
  Ipp32s cc_target_is_cpe[9];     /// 1 bit
  Ipp32s cc_target_tag_select[9]; /// 4 bits
  Ipp32s cc_l[9];                 /// 1 bit   if (cc_target_is_cpe[9];)
  Ipp32s cc_r[9];                 /// 1 bit   if (cc_target_is_cpe[9];)
  Ipp32s cc_domain;               /// 1 bit
  Ipp32s gain_element_sign;       /// 1 bit
  Ipp32s gain_element_scale;      /// 2 bit

  Ipp32s common_gain_element_present[10];///
  Ipp32s common_gain_element[10];

//  s_SE_Individual_channel_stream  stream;

  Ipp32s num_gain_element_lists; ///

} sCoupling_channel_element;

#ifdef  __cplusplus
extern "C" {
#endif

Ipp32s unpack_coupling_channel_element(sCoupling_channel_element* p_data,Ipp8u** pp_bs,
                                       Ipp32s* p_offset,Ipp32s audio_object_type,
                                       Ipp32s sampling_frequency_index);

#ifdef  __cplusplus
}
#endif

#endif//__AACCMN_CCE_H
