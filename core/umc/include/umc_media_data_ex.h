/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright(c) 2003 - 2006 Intel Corporation. All Rights Reserved.
//
//
*/

#ifndef __UMC_MEDIA_DATA_EX_H__
#define __UMC_MEDIA_DATA_EX_H__

#include "umc_media_data.h"

namespace UMC
{

class MediaDataEx : public MediaData
{
    DYNAMIC_CAST_DECL(MediaDataEx, MediaData)

public:
    class _MediaDataEx{
        DYNAMIC_CAST_DECL_BASE(_MediaDataEx)
        public:
        Ipp32u count;
        Ipp32u index;
        Ipp64u bstrm_pos;
        Ipp32u *offsets;
        Ipp32u *values;
        Ipp32u limit;

        _MediaDataEx()
        {
            count = 0;
            index = 0;
            bstrm_pos = 0;
            limit   = 2000;
            offsets = (Ipp32u*)malloc(sizeof(Ipp32u)*limit);
            values  = (Ipp32u*)malloc(sizeof(Ipp32u)*limit);
        }

        ~_MediaDataEx()
        {
            if(offsets)
            {
                free(offsets);
                offsets = 0;
            }
            if(values)
            {
                free(values);
                values = 0;
            }
            limit   = 0;
        }
    };

    // Default constructor
    MediaDataEx()
    {
        m_exData = NULL;
    };

    // Destructor
    virtual ~MediaDataEx(){};

    _MediaDataEx* GetExData()
    {
        return m_exData;
    };

    void SetExData(_MediaDataEx* pDataEx)
    {
        m_exData = pDataEx;
    };

protected:
    _MediaDataEx *m_exData;
};

}

#endif //__UMC_MEDIA_DATA_EX_H__

