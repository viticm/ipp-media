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

#if defined (UMC_ENABLE_FIO_READER)

#include <string.h>
#include "umc_fio_reader.h"

UMC::FIOReader::FIOReader():
    m_pFile(NULL),
    m_stFileSize(0),
    m_stPos(0)
{}

UMC::FIOReader::~FIOReader()
{
    Close();
}

UMC::Status UMC::FIOReader::Init(DataReaderParams *pInit)
{
    UMC::Status umcRes = UMC_OK;

    FileReaderParams* pParams = DynamicCast<FileReaderParams, DataReaderParams>(pInit);

    if (NULL == pParams)
    {   umcRes = UMC_ERR_NULL_PTR;  }

    if (UMC_OK == umcRes)
    {
        m_pFile = vm_file_open(pParams->m_file_name, VM_STRING("rb"));
        if (NULL == m_pFile)
        {   umcRes = UMC_ERR_FAILED;  }
    }

    if (UMC_OK == umcRes)
    {
        if (0 != vm_file_fseek(m_pFile, 0, VM_FILE_SEEK_END))
        {   umcRes = UMC_ERR_FAILED;  }
    }

    if (UMC_OK == umcRes)
    {
        Ipp64u lRes = vm_file_ftell(m_pFile);
        if (-1 == lRes)
        {   umcRes = UMC_ERR_FAILED;  }
        else
        {   m_stFileSize = (Ipp64u)lRes;    }
    }

    if (UMC_OK == umcRes)
    {
        if (0 != vm_file_fseek(m_pFile, 0, VM_FILE_SEEK_SET))
        {   umcRes = UMC_ERR_FAILED;  }
    }
    return umcRes;
}

UMC::Status UMC::FIOReader::Close()
{
    if (NULL != m_pFile)
    {   vm_file_fclose(m_pFile);  m_pFile = NULL; }
    m_stFileSize = NULL;
    m_stFileSize = 0;
    m_stPos = 0;

    return UMC_OK;
}

UMC::Status UMC::FIOReader::Reset()
{
    UMC::Status umcRes = UMC_OK;

    if (UMC_OK == umcRes && NULL == m_pFile)
    {   umcRes = UMC_ERR_NOT_INITIALIZED;   }

    if (UMC_OK == umcRes)
    {
        if (0 != vm_file_fseek(m_pFile, 0, VM_FILE_SEEK_SET))
        {   umcRes = UMC_ERR_FAILED;  }
        else
        {   m_stPos = 0;    }
    }
    return umcRes;
}

UMC::Status UMC::FIOReader::ReadData(void *data, Ipp32u *nsize)
{
    Status  umcRes = UMC_OK;

    if (UMC_OK == umcRes && NULL == m_pFile)
    {   umcRes = UMC_ERR_NOT_INITIALIZED;   }

    if (UMC_OK == umcRes)
    {
        size_t stRes = vm_file_fread(data, 1, *nsize, m_pFile);
        if (stRes < *nsize)
        {
            if (vm_file_feof(m_pFile))
            {
               *nsize = (Ipp32u) stRes;
               umcRes = UMC_ERR_END_OF_STREAM;
            }
            else
            {
                *nsize = 0;
                umcRes = UMC_ERR_FAILED;
            }
        }
    }

    if (UMC_OK == umcRes)
    {   m_stPos = vm_file_ftell(m_pFile);   }

    return umcRes;
}

UMC::Status UMC::FIOReader::CacheData(void *data, Ipp32u *nsize, Ipp32s how_far)
{
    Status umcRes = UMC_OK;
    Ipp64u stInitialPos = m_stPos;

    if (UMC_OK == umcRes && NULL == m_pFile)
    {   return UMC_ERR_NOT_INITIALIZED;   }

    if (UMC_OK == umcRes)
    {
        if (0 != vm_file_fseek(m_pFile, how_far, VM_FILE_SEEK_CUR))
        {   umcRes = UMC_ERR_FAILED;  }
    }

    if (UMC_OK == umcRes)
    {   umcRes = GetData(data, nsize);  }

    if (UMC_OK == umcRes || UMC_ERR_END_OF_STREAM == umcRes)
    {
        if (0 != vm_file_fseek(m_pFile, static_cast<long>(stInitialPos), VM_FILE_SEEK_SET))
        {   umcRes = UMC_ERR_FAILED; }
        else
        {   m_stPos = stInitialPos; }
    }
    return umcRes;
}

UMC::Status UMC::FIOReader::MovePosition(Ipp64u npos)
{
    UMC::Status  umcRes = UMC_OK;

    if (UMC_OK == umcRes && NULL == m_pFile)
    {   umcRes = UMC_ERR_NOT_INITIALIZED;   }

    if (UMC_OK == umcRes && 0 != npos)
    {
        if (0 != vm_file_fseek(m_pFile, (long)npos, VM_FILE_SEEK_CUR))
        {   umcRes = UMC_ERR_FAILED;  }

        if (UMC_OK == umcRes)
        {   m_stPos = vm_file_ftell(m_pFile);   }
    }
    return umcRes;
}

Ipp64u UMC::FIOReader::GetPosition()
{
    return m_stPos;
}

UMC::Status UMC::FIOReader::SetPosition(Ipp64f position)
{
    UMC::Status umcRes = UMC_OK;

    if (UMC_OK == umcRes && NULL == m_pFile)
    {   return UMC_ERR_NOT_INITIALIZED;   }

    if (UMC_OK == umcRes)
    {
        long lOffset = static_cast<long>(static_cast<Ipp64s>(m_stFileSize) * position);
        if (0 != vm_file_fseek(m_pFile, lOffset, VM_FILE_SEEK_SET))
        {   umcRes = UMC_ERR_FAILED;  }
    }

    if (UMC_OK == umcRes)
    {   m_stPos = vm_file_ftell(m_pFile);   }

    return umcRes;
}

Ipp64u UMC::FIOReader::GetSize()
{
    return m_stFileSize;
}

#endif /* UMC_ENABLE_FIO_READER */
