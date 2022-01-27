/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_FILE_READER_H__
#define __UMC_FILE_READER_H__

#include "umc_defs.h"

#if defined (UMC_ENABLE_FILE_READER) || defined(UMC_ENABLE_FIO_READER)

#include "vm_mmap.h"
#include "vm_types.h"
#include "ippdefs.h"
#include "umc_data_reader.h"

namespace UMC
{
    class FileReaderParams : public DataReaderParams
    {
        DYNAMIC_CAST_DECL(FileReaderParams, DataReaderParams)

    public:
        /** Default constructor
        Sets m_file_name to empty string and m_portion_size to 0
        */
        FileReaderParams():m_portion_size(0)
        {   memset(m_file_name, 0, sizeof(m_file_name));   }

        vm_char m_file_name[MAXIMUM_PATH]; ///< file name
        Ipp32u m_portion_size; ///< portion size
    };

    class FileReader : public DataReader
    {
        DYNAMIC_CAST_DECL(FileReader, DataReader)
    public:
        /**
        Create and map first portion from file
        \param InitParams Pointer to the init (for filereader it's vm_char file_name[255] and portion_size
        \retval UMC_OK
        \retval UMC_ERR_OPEN_FAILED       file was not open
        \retval UMC_ERR_INIT              wrong page size
        */
        Status      Init(DataReaderParams *InitParams);

        /**
        Close file
        \retval UMC_OK
        */
        Status      Close();

        /**
        Reset all internal parameters
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED             object was not initialize
        */
        Status      Reset();

        /**
        Read nsize bytes and copy to data (return number bytes which was copy)
        Cache data in case of small nsize
        \param data          pointer to the data where copy nsize byte from stream
        \param nsize         integer - number bytes
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED object was not initialize
        \retval UMC_ERR_END_OF_STREAM   end of stream
        \retval UMC_ERR_FAILED          can't map
        */
        Status      ReadData(void *data, Ipp32u *nsize);

        /**
        Move position on npos bytes
        \param npos          integer (+/-) bytes
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED object was not initialize
        */
        Status      MovePosition(Ipp64u npos);

        Status      CacheData(void *data, Ipp32u *nsize, Ipp32s how_far);

        /**
        Set position
        \param pos Ipp64f (0:1.0)
        \retval OK
        \note set position in the stream (file size * pos)
        */
        Status      SetPosition(Ipp64f pos);

        /// return position in the stream
        Ipp64u      GetPosition();

        /// return file_size
        Ipp64u      GetSize();

        FileReader();
        virtual ~FileReader();

    protected:
        vm_mmap             m_mmap;
        Ipp8u               *m_pBuffer;
        Ipp32s              m_iPageSize; // granularity size
        Ipp64s              m_iFileSize; // file size
        Ipp64s              m_iOff;      // offset of m_pBuffer
        Ipp32s              m_iPortion;  // desired view size

        virtual Status  OpenView(Ipp64s iSize);
    };

} // namespace UMC

#endif //(UMC_ENABLE_FILE_READER) || defined(UMC_ENABLE_FIO_READER)

#endif /* __UMC_FILE_READER_H__ */

