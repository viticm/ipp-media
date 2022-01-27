/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#include "aaccmn_const.h"
#include "aaccmn_cce.h"
#include "aaccmn_huff.h"
#include "bstream.h"
#include "ippac.h"
#include "ipps.h"

//Ipp32u decode_sf(Ipp8u** pp_bs, Ipp32s* p_offset);

Ipp32s
unpack_coupling_channel_element(sCoupling_channel_element * p_data, Ipp8u** pp_bs,
                                Ipp32s* p_offset, Ipp32s audio_object_type, Ipp32s sampling_frequency_index)
{
  Ipp32s c;
  Ipp32s cge;
  Ipp32s t;
  Ipp32s g;
  Ipp32s sfb;
  Ipp32s res;
  IppAACMainHeader aac_main_header;
  Ipp16s p_sf[60];
  Ipp8u  p_sf_cb[60];
  Ipp32s p_spectrum[1024];
  Ipp8s  p_tns_coef[60];
  IppAACChanInfo ch_info;
  IppAACIcsInfo  ics_info;

  ippsZero_8u((Ipp8u*)&ics_info,sizeof(ics_info));
  ippsZero_8u((Ipp8u*)&ch_info,sizeof(ch_info));

  ch_info.samplingRateIndex = sampling_frequency_index;

  ch_info.pIcsInfo = &ics_info;

  p_data->element_instance_tag = get_bits(pp_bs, p_offset, 4);

  p_data->ind_sw_cce_flag = get_bits(pp_bs, p_offset, 1);
  p_data->num_coupled_elements = get_bits(pp_bs, p_offset, 3);

  p_data->num_gain_element_lists = 0;
  for (c = 0; c < p_data->num_coupled_elements + 1; c ++)
  {
    p_data->num_gain_element_lists++;
    p_data->cc_target_is_cpe[c]    = get_bits(pp_bs, p_offset, 1);
    p_data->cc_target_tag_select[c]= get_bits(pp_bs, p_offset, 4);
    if ( p_data->cc_target_is_cpe[c] )
    {
      p_data->cc_l[c] = get_bits(pp_bs, p_offset, 1);
      p_data->cc_r[c] = get_bits(pp_bs, p_offset, 1);
      if (p_data->cc_l[c] && p_data->cc_r[c] )
      {
        p_data->num_gain_element_lists++;
      }
    }
  }

  p_data->cc_domain = get_bits(pp_bs, p_offset, 1);
  p_data->gain_element_sign = get_bits(pp_bs, p_offset,1);
  p_data->gain_element_scale = get_bits(pp_bs, p_offset,2);

  res = ippsNoiselessDecode_AAC(pp_bs, p_offset, &aac_main_header,
                                p_sf, p_spectrum, p_sf_cb, p_tns_coef, &ch_info,
                                0/*windows sequence*/,0/*max_sfb*/,0/*common_win*/,
                                0,audio_object_type);

//  dec_individual_channel_stream(&p_data->stream,pBS,0,0,aydioObjectType);

  for (c = 1; c < p_data->num_gain_element_lists; c ++)
  {
    if ( p_data->ind_sw_cce_flag )
    {
      cge = 1;
    }
    else
    {
      p_data->common_gain_element_present[c] = get_bits(pp_bs, p_offset, 1);
      cge = p_data->common_gain_element_present[c];
    }
    if (cge)
    {
      t = aac_huff_decode_sf(pp_bs, p_offset);
            p_data->common_gain_element[c] = t;
    }
    else
    {
      t = 0;
            /// The branch is not tested due to the streams are absent.
            for (g = 0; g < ics_info.numWinGrp; g ++)
            {
                for (sfb = 0; sfb < ics_info.maxSfb; sfb ++)
                {
                    if (p_sf_cb[sfb] != ZERO_HCB)
                    {
                        t = aac_huff_decode_sf(pp_bs,p_offset);
//                        p_data->common_gain_element[c] = t;
                    }
                }
            }
    }
  }

  return 0;
}
