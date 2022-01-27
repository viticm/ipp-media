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

#if defined (UMC_ENABLE_FILE_WRITER)

#include <string.h>
#include "umc_file_writer.h"
#include "vm_mmap.h"
#include "vm_debug.h"

#define UMC_BUF_SIZE 1024


UMC::FileWriterParams::FileWriterParams()
{
    m_portion_size = 0;
    memset(m_file_name, 0, sizeof(m_file_name));
} // ctor


UMC::FileWriter::FileWriter()
#ifdef USE_FILE_MAPPING
        :m_pbBuffer(NULL),
        m_uiPortionSize(0),
        m_stDoneSize(0),
        m_stPos(0),
        m_uiPageSize(0),
        m_bBufferInit(false)
#endif
{
#ifndef USE_FILE_MAPPING
    m_file = 0;
#else
    vm_mmap_set_invalid(&m_mmap);
#endif
} // ctor


UMC::FileWriter::~FileWriter()
{
    Close();
} // dtor


UMC::Status UMC::FileWriter::Init(DataWriterParams *pInit)
{
    Status umcRes = UMC_OK;
    FileWriterParams* initFileExt =
        DynamicCast<FileWriterParams, DataWriterParams>(pInit);
    if(!initFileExt)
        umcRes = UMC_ERR_NULL_PTR;

#ifndef USE_FILE_MAPPING
    if (UMC_OK == umcRes)
    {
        m_file = vm_file_open(initFileExt->m_file_name,__VM_STRING("wb"));
        if (NULL == m_file)
        {
          umcRes = UMC_ERR_FAILED;
        }
    }
#else
    m_uiPageSize = vm_mmap_get_alloc_granularity() * UMC_BUF_SIZE;

    if (UMC_OK == umcRes)
    {
        m_uiFileSize = vm_mmap_create(&m_mmap, initFileExt->m_file_name, FLAG_ATTRIBUTE_WRITE);
    }

    if (UMC_OK == umcRes)
    {
        m_uiPortionSize = m_uiPageSize;

        Ipp64u csize  = m_uiPortionSize;
        Ipp64u offset = 0;
        m_bBufferInit = false;
        m_pbBuffer = (Ipp8u*)vm_mmap_set_view(&m_mmap, &offset, &csize);

        if (!m_pbBuffer)
        {
            umcRes = UMC_ERR_ALLOC;
            vm_debug_trace1(VM_DEBUG_INFO, __VM_STRING("File %s map error\n"),
                            initFileExt->m_file_name);
        }
    }

    if (UMC_OK == umcRes)
    {
        m_bBufferInit = true;
        m_stPos       = 0;
        m_stDoneSize  = 0;
    }
    else
    {
        Close();
    }
#endif

    return umcRes;
} // FileWriter::Init()


UMC::Status UMC::FileWriter::Close()
{
#ifndef USE_FILE_MAPPING
    if (m_file)
        vm_file_fclose(m_file);

    m_file = 0;
#else
    m_bBufferInit = false;
    vm_mmap_close(&m_mmap);
    m_pbBuffer          = 0;
    m_stPos             = 0;
    m_stDoneSize        = 0;
    m_uiPortionSize     = 0;
#endif

    return UMC_OK;
} // FileWriter::Close()


UMC::Status UMC::FileWriter::Reset()
{
    return UMC_OK;
} // FileWriter::Reset()


#ifdef USE_FILE_MAPPING
UMC::Status UMC::FileWriter::MapCSize()
{
    Ipp64u tmp_size;

    if (!vm_mmap_is_valid(&m_mmap)) { return UMC_ERR_NOT_INITIALIZED; }

    m_stDoneSize += m_uiPageSize;
    m_stPos = 0;

    m_bBufferInit = false;
    tmp_size      = m_uiPageSize;

    if(m_uiFileSize < m_stDoneSize)
        return UMC_ERR_ALLOC;

    m_pbBuffer = (Ipp8u*)vm_mmap_set_view(&m_mmap, &m_stDoneSize, &tmp_size);
    if (!m_pbBuffer)
    {
        return UMC_ERR_ALLOC;
    }

    m_bBufferInit = true;
    return UMC_OK;
} // FileWriter::MapCSize()
#endif // USE_FILE_MAPPING


UMC::Status UMC::FileWriter::PutData(const void *data, Ipp32s &nsize)
{
#ifndef USE_FILE_MAPPING
    if(m_file && data && (nsize > 0))
        nsize = static_cast<Ipp32s> (vm_file_fwrite((void *) data, 1, nsize, m_file));

    return UMC_OK;
#else
    Status  ret     = UMC_OK;
    Ipp32s  bufsize = 0;
    Ipp32s  tmp_size = nsize;

    if (!vm_mmap_is_valid(&m_mmap))
        return UMC_ERR_NOT_INITIALIZED;

    while(tmp_size > 0 || ret != UMC_OK)
    {
        if ((m_stPos + tmp_size > m_uiPageSize) || !m_bBufferInit)
        {
            bufsize     = m_uiPageSize - m_stPos;
            memcpy(m_pbBuffer + m_stPos, data, bufsize);
            tmp_size -= bufsize;
            ret = MapCSize();
        }

        if((UMC_OK == ret) && (tmp_size <= m_uiPageSize))
        {
            memcpy(m_pbBuffer + m_stPos, (Ipp8u*)data + bufsize, tmp_size);
            m_stPos += tmp_size;
            break;
        }
    }
    return ret;
#endif
}

#ifndef USE_FILE_MAPPING

UMC::Status UMC::FileWriter::SetPosition(Ipp32u nPosLow, Ipp32u *lpnPosHight, Ipp32u nMethod)
{
    // check error(s)
    if (NULL == m_file)
        return UMC::UMC_ERR_NOT_INITIALIZED;

    // current version isn't support large file(s)
    if ((NULL != lpnPosHight) &&
            (0 != *lpnPosHight))
        return UMC::UMC_ERR_UNSUPPORTED;

    // set new position
    if (0 != vm_file_fseek(m_file, nPosLow, (VM_FILE_SEEK_MODE)nMethod))
        return UMC::UMC_ERR_FAILED;

    return UMC::UMC_OK;
} // FileWriter::SetPosition()


UMC::Status UMC::FileWriter::GetPosition(Ipp32u *lpnPosLow, Ipp32u *lpnPosHigh)
{
    // check error(s)
    if (NULL == m_file)
        return UMC::UMC_ERR_NOT_INITIALIZED;
    if (NULL == lpnPosLow)
        return UMC::UMC_ERR_NULL_PTR;

    // return value(s)
    if (lpnPosHigh)
        *lpnPosHigh = 0;

    *lpnPosLow = (Ipp32u)vm_file_ftell(m_file);

    return UMC::UMC_OK;
} // FileWriter::GetPosition()

#endif // USE_FILE_MAPPING

#endif // UMC_ENABLE_FILE_WRITER
