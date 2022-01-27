/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_MMAP_H__
#define __UMC_MMAP_H__

#include "ippdefs.h"
#include "vm_debug.h"
#include "vm_mmap.h"
#include "umc_structures.h"

namespace UMC
{

class MMap
{
public:
    // Default constructor
    MMap(void);
    // Destructor
    virtual ~MMap(void);

    // Initialize object
    Status Init(vm_char *sz_file);
    // Map memory
    Status Map(Ipp64u st_offset, Ipp64u st_sizet);
    // Get addres of mapping
    void *GetAddr(void);
    // Get offset of mapping
    Ipp64u GetOffset(void);
    // Get size of mapping
    Ipp64u GetSize(void);
    // Get size of mapped file
    Ipp64u GetFileSize(void);

protected:
    vm_mmap m_handle;                                         // (vm_mmap) handle to system mmap object
    void *m_address;                                          // (void *) addres of mapped window
    Ipp64u m_file_size;                                       // (Ipp64u) file size
    Ipp64u m_offset;                                          // (Ipp64u) offset of mapping
    Ipp64u m_sizet;                                           // (Ipp64u) size of window
};

inline
void *MMap::GetAddr(void)
{
    return m_address;

} // void *MMap::GetAddr(void)

inline
Ipp64u MMap::GetOffset(void)
{
    return m_offset;

} // Ipp64u MMap::GetOffset(void)

inline
Ipp64u MMap::GetSize(void)
{
    return m_sizet;

} // Ipp64u MMap::GetSize(void)

inline
Ipp64u MMap::GetFileSize(void)
{
    return m_file_size;

} // Ipp64u MMap::GetFileSize(void)

} // namespace UMC

#endif // __UMC_MMAP_H__
