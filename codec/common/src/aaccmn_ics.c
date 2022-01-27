/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#if 0

#include "aaccmn_ics.h"
#include "aaccmn_const.h"
#include "bstream.h"
#include "ippac.h"


#define SF_MID          60
#define SF_OFFSET       100

Ipp32s dec_ics_info(sIndividual_channel_stream * p_data,sBitsreamBuffer * p_bs, Ipp32s common_window, Ipp32s audioObjectType);


Ipp32s
dec_individual_channel_stream(sIndividual_channel_stream * p_data,sBitsreamBuffer * p_bs, Ipp32s common_window, Ipp32s scal_flag,Ipp32s aydioObjectType)
{
  Ipp32s i;
  p_data->global_gain = Getbits(p_bs,8);

  if (!common_window && !scal_flag)
  {
    dec_ics_info(p_data,p_bs,common_window,aydioObjectType);
  }
  dec_ics_section_data(p_data, p_bs);
  dec_ics_scale_factor_data(p_data,p_bs);

  if (!scal_flag)
  {
    p_data->pulse_data_present = Getbits(p_bs,1);
    if(p_data->pulse_data_present)
    {
      dec_ics_pulse_data(p_data,p_bs);
    }

    p_data->tns_data_present = Getbits(p_bs,1);
    if (p_data->tns_data_present)
    {
      dec_ics_tns_data(p_data,p_bs);
    }
    p_data->gain_control_data_present = Getbits(p_bs,1);
        if (p_data->gain_control_data_present)
        {
            dec_ics_gain_control_data(p_data,p_bs);
        }
  }
  for (i = 0; i < p_data->num_windows; i++ )
  {
    p_data->p_spectrum[i] = &p_data->spectrum_data[i*128];
  }

  dec_ics_spectral_data(p_data,p_bs);
  return 0;
}

Ipp32s
dec_ics_info(sIndividual_channel_stream * p_data,sBitsreamBuffer * p_bs, Ipp32s common_window,Ipp32s audioObjectType)
{
  Ipp32s i;
  Ipp32s pred_min_sfb;

  p_data->ics_reserved_bit = Getbits(p_bs,1);
  p_data->window_sequence  = Getbits(p_bs,2);
  p_data->window_shape     = Getbits(p_bs,1);

  p_data->num_window_groups = 1;
  p_data->len_window_group[0] = 1;

  if (p_data->window_sequence == EIGHT_SHORT_SEQUENCE)
  {
    p_data->num_windows = 8;
    p_data->max_sfb = Getbits(p_bs,4);
    for (i = 0; i < 7 ; i++)
    {
      p_data->scale_factor_grouping[i] = Getbits(p_bs,1);

      if (p_data->scale_factor_grouping[i] == 0)
      {
        p_data->len_window_group[p_data->num_window_groups] = 1;
        p_data->num_window_groups ++;
      }
      else
      {
        p_data->len_window_group[p_data->num_window_groups - 1] ++;

      }
    }

  }
  else
  {
    p_data->num_windows = 1;
    p_data->max_sfb = Getbits(p_bs,6);
    p_data->predictor_data_present = Getbits(p_bs,1);

    if (p_data->predictor_data_present)
    {
            if (audioObjectType == AOT_AAC_MAIN)
            {
                p_data->predictor_reset = Getbits(p_bs,1);
                if (p_data->predictor_reset)
                {
                    p_data->predictor_reset_group_number = Getbits(p_bs,5);
                }
                pred_min_sfb = p_data->max_sfb < p_data->pred_max_sfb ? p_data->max_sfb : p_data->pred_max_sfb;
                for (i = 0; i < pred_min_sfb; i ++)
                {
                    p_data->prediction_used[i] = Getbits(p_bs,1);
                }
            }
            else
            {
                p_data->ltp_data_present = Getbits(p_bs,1);
                if (p_data->ltp_data_present)
                {
                    dec_ics_ltp_data(p_data,p_bs,audioObjectType);
                }
            }
    }
  }


  return 0;
}


Ipp32s
dec_ltp_data(sIndividual_channel_stream* p_data, sBitsreamBuffer* p_bs,Ipp32s audioObjectType)
{
    Ipp32s i;
    Ipp32s w;
    Ipp32s pred_max_sfb;

    pred_max_sfb = p_data->pred_max_sfb < p_data->max_sfb ? p_data->pred_max_sfb : p_data->max_sfb;
    if (audioObjectType == 19)
    {
        p_data->ltp_lag_update = Getbits(p_bs,1);
        if ( p_data->ltp_lag_update )
        {
            p_data->ltp_lag = Getbits(p_bs,10);
        }
        else
        {
            //ltp_lag = ltp_prev_lag;
        }
        p_data->ltp_coef = Getbits(p_bs,3);
//        for (i = 0; i < p_data->max_sfb; i++)
        for (i = 0; i < pred_max_sfb; i++)
        {
            p_data->ltp_long_used[i] = Getbits(p_bs,1);
        }
    }
    else
    {
        p_data->ltp_lag = Getbits(p_bs,11);
        p_data->ltp_coef = Getbits(p_bs,3);

        if (p_data->window_sequence == EIGHT_SHORT_SEQUENCE)
        {
            for (w = 0; w < p_data->num_windows; w ++)
            {
                p_data->ltp_short_used[w] = Getbits(p_bs,1);
                if (p_data->ltp_short_used[w])
                {
                    p_data->ltp_short_lag_present[w] = Getbits(p_bs,1);
                    if (p_data->ltp_short_lag_present[w])
                    {
                        p_data->ltp_short_lag[w] = Getbits(p_bs,4);
                    }
                }
            }

        }
        else
        {
            for (i = 0; i < pred_max_sfb; i++)
            {
                p_data->ltp_long_used[i] = Getbits(p_bs,1);
            }
        }
    }
    return 0;
}

Ipp32s
dec_ics_section_data(sIndividual_channel_stream * p_data,sBitsreamBuffer *p_bs)
{
  Ipp32s sfb;
  Ipp32s k;
  Ipp32s g;
  Ipp32s sect_esc_val;
  Ipp32s sect_len_incr;
  Ipp32s esc_code_len;
  Ipp32s sect_cb;
  Ipp32s sect_len;

  if(p_data->window_sequence == EIGHT_SHORT_SEQUENCE)
  {
    sect_esc_val = (1<<3)-1;
    esc_code_len = 3;
  }
  else
  {
    sect_esc_val = (1<<5)-1;
    esc_code_len = 5;
  }
  for(g = 0; g < p_data->num_window_groups; g++)
  {
    k = 0;
    while(k < p_data->max_sfb)
    {
      sect_cb = Getbits(p_bs,4);
      sect_len = 0;
      while((sect_len_incr = Getbits(p_bs,esc_code_len)) == sect_esc_val)
      {
        sect_len += sect_esc_val;
      }
      sect_len += sect_len_incr;

      for(sfb = k; sfb < k+sect_len; sfb++)
      {
        p_data->sfb_cb[g][sfb] = sect_cb;
      }
      k += sect_len;

    }
    for(; k < 51; k++)
    {
      p_data->sfb_cb[g][k] = 0;
    }
  }

  return 0;
}

Ipp32s
dec_scale_factor_data(sIndividual_channel_stream* p_data,sBitsreamBuffer * p_bs)
{
  Ipp32s g;
  Ipp32s sfb;
  Ipp32s t;
  Ipp32s is_pos;
   Ipp32s scale_factor;
    Ipp32s noise_pcm_flag;
    Ipp32s noise_nrg;

  is_pos = 0;
    noise_pcm_flag = 1;
    noise_nrg = p_data->global_gain;
    noise_nrg -= 90;

  scale_factor = p_data->global_gain;

  for(g = 0; g < p_data->num_window_groups; g++)
  {
    for (sfb = 0; sfb < p_data->max_sfb; sfb ++)
    {
      switch(p_data->sfb_cb[g][sfb])
      {
      case ZERO_HCB:
        p_data->sf[g][sfb] = 0;
        break;
      case INTENSITY_HCB:
      case INTENSITY_HCB2:
//                dbg_trace("INTENSITY_HCB2\n");
        ippsDecodeVLC_32s( &p_bs->pCurrent_dword, &p_bs->nBit_offset, p_data->p_huffman_tables[0], &t);
        is_pos += t - SF_MID; ///
        p_data->sf[g][sfb] = is_pos;
        break;
            case NOISE_HCB:
//                dbg_trace("!!! NOISE_HCB\n");

                if (noise_pcm_flag)
                {
                    noise_pcm_flag = 0;
                    t = Getbits(p_bs,9);
                }
                else
                {
                    ippsDecodeVLC_32s( &p_bs->pCurrent_dword, &p_bs->nBit_offset, p_data->p_huffman_tables[0], &t);
                    t -= SF_MID;
                }
                noise_nrg += t;
                p_data->sf[g][sfb] = noise_nrg;
                break;
      default:
        ippsDecodeVLC_32s( &p_bs->pCurrent_dword, &p_bs->nBit_offset, p_data->p_huffman_tables[0], &t);
        scale_factor += t - SF_MID; ///
        p_data->sf[g][sfb] = scale_factor;
        break;
      }
    }
  }

  return 0;
}

#endif
