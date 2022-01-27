/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#include "umc_file_reader.h"

#if defined (UMC_ENABLE_FILE_READER) || defined (UMC_ENABLE_FIO_READER)

#include <string.h>
#include "vm_debug.h"
#include "umc_dynamic_cast.h"

using namespace UMC;

FileReader::FileReader()
: m_pBuffer(0)
, m_iPageSize(vm_mmap_get_alloc_granularity())
, m_iFileSize(0)
, m_iOff(0)
, m_iPortion(0)
{
    vm_mmap_set_invalid(&m_mmap);
}

FileReader::~FileReader()
{
    Close();
}

Status FileReader::Init(DataReaderParams *pInit)
{
    FileReaderParams* pParams = DynamicCast<FileReaderParams, DataReaderParams>(pInit);

    UMC_CHECK(pParams, UMC_ERR_INIT)
    UMC_CHECK((m_iPageSize > 0),  UMC_ERR_INIT)
    m_iPortion = m_iPageSize * ((pParams->m_portion_size <= 0) ? 188 : ((m_iPortion + m_iPageSize - 1) / m_iPageSize));
    UMC_CHECK((m_iPortion > 0),  UMC_ERR_INIT)

    if (vm_mmap_is_valid(&m_mmap))
        Close();

    m_iFileSize = vm_mmap_create(&m_mmap, pParams->m_file_name, FLAG_ATTRIBUTE_READ);

    if (0 == m_iFileSize) {
        vm_debug_trace1(VM_DEBUG_INFO, VM_STRING("File %s open error\n"), pParams->m_file_name);
        vm_mmap_set_invalid(&m_mmap);
        return UMC_ERR_OPEN_FAILED;
    }

    return UMC_OK;
}

Status FileReader::Close()
{
    vm_mmap_close(&m_mmap);
    vm_mmap_set_invalid(&m_mmap);
    m_pDataPointer = 0;
    m_pEODPointer  = 0;
    m_iFileSize    = 0;
    m_iOff         = 0;
    m_iPortion     = 0;
    return UMC_OK;
}

Status FileReader::Reset()
{
    UMC_CHECK(vm_mmap_is_valid(&m_mmap), UMC_ERR_NOT_INITIALIZED)
    vm_mmap_unmap(&m_mmap);
    m_pDataPointer = 0; // set to zero pointers
    m_pBuffer      = 0;
    m_pEODPointer  = 0;
    m_iOff         = 0;
    return UMC_OK;
}

// Requirements:
// 0 <= m_iFileSize
// 0 <= m_iPos <= m_iFileSize
// 0 < m_iPageSize
// 0 < m_iPortion
// m_mmap is valid
Status FileReader::OpenView(Ipp64s iSize)
{
    UMC_CHECK((iSize > (m_pEODPointer - m_pDataPointer)), UMC_OK); // already ok
    Ipp64s iPos     = m_iOff + m_pDataPointer - m_pBuffer;
    Ipp64s iStart   = iPos - (iPos % m_iPageSize);     // align to page
    Ipp64s iMax     = m_iFileSize - iStart;            // maximum depends on file size
    Ipp64u iiStart  = iStart, iiSize;                  // for vm_mmap_set_view

    iSize += iPos - iStart;                            // increase size due align
    iSize  = IPP_MAX(iSize, m_iPortion);               // would like to open more if iSize small
    iiSize = iSize = IPP_MIN(iSize, iMax);             // cut off end of file
    UMC_CHECK(((iSize - (iPos - iStart)) > (m_pEODPointer - m_pDataPointer)), UMC_OK); // ok after end of file cut

    m_pBuffer      = m_pEODPointer = m_pDataPointer =  (Ipp8u *)vm_mmap_set_view(&m_mmap, &iiStart, &iiSize);
    m_iOff         = iPos; // nullify all if bad return
    UMC_CHECK(((m_pDataPointer) && (iStart == (Ipp64s)iiStart) && (iSize == (Ipp64s)iiSize)), UMC_ERR_FAILED); // sanity check
    m_iOff          = iStart; // right setup if ok
    m_pDataPointer += (iPos - iStart);
    m_pEODPointer  += iSize;

    return UMC_OK;
}

Status FileReader::ReadData(void *data, Ipp32u *nsize)
{
    if (((size_t)(m_pEODPointer - m_pDataPointer)) >= *nsize)
    {
        Ipp32u uiSize = *nsize;

        for(Ipp32u i = 0; i < uiSize; i++)
            ((Ipp8u*)data)[i] = m_pDataPointer[i]; //memcpy(data, m_pDataPointer, data_sz);
        m_pDataPointer += *nsize;
        return UMC_OK;
    }

    Status umcRes = CacheData(data, nsize, 0);
    MovePosition(*nsize); // lazy to check all twice

    return umcRes;
}

Status FileReader::CacheData(void *data, Ipp32u *nsize, Ipp32s how_far)
{
    if (((size_t)(m_pEODPointer - m_pDataPointer)) >= (*nsize + how_far))
    {
        Ipp32u uiSize = *nsize;
        for(Ipp32u i = 0; i < uiSize; i++)
            ((Ipp8u*)data)[i] = m_pDataPointer[i + how_far];
        return UMC_OK;
    }
    Ipp64s iSize = *nsize, iMax;
    Status umcRes;

    *nsize = 0; // return zero if nothing read
    UMC_CHECK(vm_mmap_is_valid(&m_mmap), UMC_ERR_NOT_INITIALIZED)
        umcRes = OpenView(iSize + how_far);
    if (UMC_OK == umcRes)
    {
        umcRes = ((iSize += how_far) <= (iMax = (m_pEODPointer - m_pDataPointer))) ? UMC_OK : UMC_ERR_END_OF_STREAM;
        iSize  = IPP_MIN(iSize, iMax);
        iSize -= how_far;
        *nsize = (Ipp32u)IPP_MAX(iSize, 0);
        if (iSize)
            memcpy(data, m_pDataPointer + how_far, *nsize);
    }

    return umcRes;
}

Status FileReader::MovePosition(Ipp64u npos)
{
    UMC_CHECK(vm_mmap_is_valid(&m_mmap), UMC_ERR_NOT_INITIALIZED)
        Ipp64s iOldPos = m_iOff + m_pDataPointer - m_pBuffer;
    Ipp64s iPos = iOldPos + npos;

    iPos = IPP_MIN(m_iFileSize, iPos); // cut max
    iPos = IPP_MAX(0, iPos);           // cut min
    if ((iPos < m_iOff) || (iPos > (m_iOff + (m_pEODPointer - m_pBuffer)))) // out of view
    {
        vm_mmap_unmap(&m_mmap);
        m_pBuffer = m_pDataPointer = m_pEODPointer = 0;
        m_iOff = iPos;
    } else
        m_pDataPointer += (iPos - iOldPos); // move inside view

    return UMC_OK;
}

Ipp64u FileReader::GetPosition()
{
    return m_iOff + m_pDataPointer - m_pBuffer;
}

Status FileReader::SetPosition(Ipp64f position)
{
    UMC_CHECK(vm_mmap_is_valid(&m_mmap), UMC_ERR_NOT_INITIALIZED)
        Ipp64s iPos = (Ipp64s)(position * (m_iFileSize + 0.5));
    Ipp64s iOldPos = m_iOff + m_pDataPointer - m_pBuffer;

    return MovePosition(iPos - iOldPos);
}

Ipp64u FileReader::GetSize()
{
    return m_iFileSize;
}

#endif // (UMC_ENABLE_FILE_READER)

