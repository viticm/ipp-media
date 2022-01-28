/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2012 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

#include <stdio.h>
#include "vm_time.h"
#include "vm_types.h"

namespace UMC
{

typedef struct sSystemInfo
{
    Ipp32u num_proc;                                           // number of processor(s)
    Ipp32u cpu_freq;                                           // CPU frequency
    vm_char os_name[_MAX_LEN];                                 // OS name
    vm_char proc_name[_MAX_LEN];                               // processor's name
    vm_char computer_name[_MAX_LEN];                           // computer's name
    vm_char user_name[_MAX_LEN];                               // user's name
    vm_char video_card[_MAX_LEN];                              // video adapter's name
    vm_char program_path[_MAX_LEN];                            // program path
    vm_char program_name[_MAX_LEN];                            // program name
    vm_char description[_MAX_LEN];
    Ipp32u phys_mem;
} sSystemInfo;

#if defined WINDOWS && !(defined _WIN32_WCE)
/* obtain  */
typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID
{
    Ipp32u        UniqueProcess;
    Ipp32u        UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING
{
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR         Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS
{
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG         PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG         WaitTime;
    PVOID         StartAddress;
    CLIENT_ID     ClientId;
    KPRIORITY     Priority;
    KPRIORITY     BasePriority;
    ULONG         ContextSwitchCount;
    LONG          State;
    LONG          WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES
{
    ULONG             NextEntryDelta;
    ULONG             ThreadCount;
    ULONG             Reserved1[6];
    LARGE_INTEGER     CreateTime;
    LARGE_INTEGER     UserTime;
    LARGE_INTEGER     KernelTime;
    UNICODE_STRING    ProcessName;
    KPRIORITY         BasePriority;
    ULONG             ProcessId;
    ULONG             InheritedFromProcessId;
    ULONG             HandleCount;
    ULONG             Reserved2[2];
    VM_COUNTERS       VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS       IoCounters;
#endif
    SYSTEM_THREADS    Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;
#endif

class SysInfo
{
public:
    SysInfo(vm_char *pProcessName = NULL);
    virtual ~SysInfo(void);

    // Get system information
    sSystemInfo *GetSysInfo(void);
    Ipp64f GetCpuUsage(void);
    Ipp64f GetAvgCpuUsage(void) {return avg_cpuusage;};
    Ipp64f GetMaxCpuUsage(void) {return max_cpuusage;};
    void CpuUsageRelease(void);

protected:
    // Get OS version information
    void GetCpuUseTime(vm_char*, vm_tick* process_use, vm_tick* total_use);
    void GetOSVersion(vm_char *);
    vm_tick user_time;
    vm_tick total_time;
    vm_tick user_time_start;
    vm_tick total_time_start;
    Ipp64f last_cpuusage;
    Ipp64f max_cpuusage;
    Ipp64f avg_cpuusage;

    sSystemInfo m_sSystemInfo;  // system info struct
};

}
#endif
