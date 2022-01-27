/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_FILE_WRITER_H__
#define __UMC_FILE_WRITER_H__

#include "umc_defs.h"

#if defined (UMC_ENABLE_FILE_WRITER)

#include <stdio.h>
#include "ippdefs.h"
#include "umc_data_writer.h"
#include "vm_file.h"

//#define USE_FILE_MAPPING

namespace UMC
{
    class FileWriterParams : public DataWriterParams
    {
        DYNAMIC_CAST_DECL(FileWriterParams, DataWriterParams)
    public:
        vm_char m_file_name[MAXIMUM_PATH];
        Ipp64u  m_portion_size;

        FileWriterParams();
    };

    class FileWriter : public DataWriter
    {
    public:
        //////////////////////////////////////////////////////////////////////////////
        //  Name:           Init
        //
        //  Purpose:        Create and map first portion from file
        //
        //  Parameters:
        //    InitParams    Pointer to the init (for filereader it's
        //                  vm_char m_file_name[] and m_portion_size
        //
        //
        //  Return:
        //    OK
        //    ERR_FILECREATE_IS_INVALID     file not ctrate
        //    ERR_NULL_PTR              pointer to the buffer is NULL
        //
        //
        //  Notes:      default value of portion_size is 0. After open portion = 1
        //
        ////////////////////////////////////////////////////////////////////////////////
        Status Init(DataWriterParams *InitParams);

        //////////////////////////////////////////////////////////////////////////////
        //  Name:           Close
        //
        //  Purpose:        Close file
        //
        //  Parameters:
        //
        //  Return:
        //    OK        file was close
        //
        //  Notes:
        //
        ////////////////////////////////////////////////////////////////////////////////
        Status Close();

        //////////////////////////////////////////////////////////////////////////////
        //  Name:           Reset
        //
        //  Purpose:        Reset all internal parameters
        //
        //  Parameters:
        //
        //  Return:
        //    OK
        //
        //  Notes:
        //
        ////////////////////////////////////////////////////////////////////////////////
        Status Reset();

        //////////////////////////////////////////////////////////////////////////////
        //  Name:           GetData
        //
        //  Purpose:        Get nsize bytes and copy to data (return number bytes which was copy)
        //
        //  Parameters:
        //    data          pointer to the data where copy nsize byte from stream
        //    nsize         integer - number bytes
        //
        //  Return:
        //    return number bytes which was copy
        //
        //  Notes:
        //
        ////////////////////////////////////////////////////////////////////////////////
        Status PutData(const void *data, Ipp32s &nsize);

#ifndef USE_FILE_MAPPING
        //////////////////////////////////////////////////////////////////////////////
        //  Name:           SetPosition
        //
        //  Purpose:        Set current position in destination media
        //
        //  Parameters:
        //    nPosLow       Lower 32 bit(s) of position
        //    lpnPosHigh    Pointer to upper 32 bit(s) of position (can be NULL)
        //    nMethod       Initial position. Use C standard defines:
        //                  SEEK_CUR - Current position of pointer
        //                  SEEK_END - End of media
        //                  SEEK_SET - Beginning of media
        //
        //  Return:
        //    return UMC_OK if position is changed, otherwise code of error
        //
        //  Notes:
        //    Not all media can change current position
        ////////////////////////////////////////////////////////////////////////////////
        virtual Status SetPosition(Ipp32u nPosLow, Ipp32u *lpnPosHigh, Ipp32u nMethod);

        //////////////////////////////////////////////////////////////////////////////
        //  Name:           GetPosition
        //
        //  Purpose:        Get current position in destiantion media
        //
        //  Parameters:
        //    lpnPosLow     Pointer to variable to lower 32 bit(s) of position
        //    lpnPosHigh    Pointer to variable to upper 32 bit(s) of position
        //
        //  Return:
        //    return UMC_OK if position is returned, otherwise code of error
        //
        //  Notes:
        //    Not all media can return current position
        ////////////////////////////////////////////////////////////////////////////////
        virtual Status GetPosition(Ipp32u *lpnPosLow, Ipp32u *lpnPosHigh);
#endif //USE_FILE_MAPPING

        FileWriter();
        virtual ~FileWriter();

    protected:

#ifndef USE_FILE_MAPPING
        vm_file * m_file;
#else
        Status FileWriter::MapCSize();
        vm_mmap      m_mmap;

        Ipp8u       *m_pbBuffer;
        Ipp32u       m_uiFileSize;
        Ipp32u       m_uiPortionSize;
        Ipp64u       m_stDoneSize;               // accumulative size of processed portions
        Ipp64u       m_stPos;                    // position in current portion of file
        Ipp32u       m_uiPageSize;
        bool         m_bBufferInit;
#endif
    };

}//namespace UMC

#endif //(UMC_ENABLE_FILE_READER)

#endif /* __UMC_FILE_WRITER_H__ */
