/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2007 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_FIO_READER_H__
#define __UMC_FIO_READER_H__

#include "umc_defs.h"

#if defined (UMC_ENABLE_FIO_READER)

#include "umc_file_reader.h"
#include "vm_file.h"
#include <stdio.h>

namespace UMC
{
    class FIOReader : public DataReader
    {
        DYNAMIC_CAST_DECL(FIOReader, DataReader);
    public:
        /**
        Create and map first portion from file
        \param InitParams Pointer to the init (for filereader it's vm_char file_name[255] and portion_size
        \retval UMC_OK
        \retval UMC_ERR_OPEN_FAILED       file was not open
        \retval UMC_ERR_ALLOC   mapping was not create
        \note default value of portion_size is 0. After open portion = 1
        */
        virtual Status Init(DataReaderParams *InitParams);

        /**
        Close file
        \retval UMC_OK
        */
        virtual Status Close();

        /**
        Reset all internal parameters
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED             object was not initialize
        \retval UMC_ERR_ALLOC   mapping was not create
        */
        virtual Status Reset();

        /**
        Read nsize bytes and copy to data (return number bytes which was copy)
        Cache data in case of small nsize
        \param data          pointer to the data where copy nsize byte from stream
        \param nsize         integer - number bytes
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED object was not initialize
        \retval UMC_ERR_END_OF_STREAM   end of stream
        */
        virtual Status ReadData(void *data, Ipp32u *nsize);

        /**
        Move position on npos bytes
        \param npos          integer (+/-) bytes
        \retval UMC_OK
        \retval UMC_ERR_NOT_INITIALIZED object was not initialize
        \retval UMC_ERR_END_OF_STREAM   end of stream
        */
        virtual Status MovePosition(Ipp64u npos);

        virtual Status CacheData(void *data, Ipp32u *nsize, Ipp32s how_far);

        /**
        Set position
        \param pos Ipp64f (0:1.0)
        \retval OK
        \retval ERR_MAP    Error map next portion
        \note set position in the stream (file size * pos)
        */
        virtual Status SetPosition(Ipp64f pos);

        /// return position in the stream
        virtual Ipp64u GetPosition();

        /// return file_size
        virtual Ipp64u GetSize();

        FIOReader();
        virtual ~FIOReader();

    protected:
        vm_file*     m_pFile;

        Ipp64u    m_stFileSize;               // file size
        Ipp64u    m_stPos;                    // position in current portion of file
    };

}//namespace UMC

#endif //(UMC_ENABLE_FILE_READER)

#endif /* __UMC_FIO_READER_H__ */
