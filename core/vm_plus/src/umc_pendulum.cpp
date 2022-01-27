/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//    Copyright (c) 2003-2006 Intel Corporation. All Rights Reserved.
//
//
*/

#include "umc_pendulum.h"

namespace UMC
{

Pendulum::Pendulum(void)
{
    vm_event_set_invalid(&m_hHigh);
    vm_event_set_invalid(&m_hLow);

} // Pendulum::Pendulum(void)

Pendulum::~Pendulum(void)
{
    Release();

} // Pendulum::~Pendulum(void)

void Pendulum::Release(void)
{
    if (vm_event_is_valid(&m_hHigh))
        vm_event_destroy(&m_hHigh);
    if (vm_event_is_valid(&m_hLow))
        vm_event_destroy(&m_hLow);

    vm_event_set_invalid(&m_hHigh);
    vm_event_set_invalid(&m_hLow);

} // void Pendulum::Release(void)

Status Pendulum::Init(bool bSignaled)
{
    vm_status vmRes;

    // Release object before initialization
    Release();

    vmRes = vm_event_init(&m_hHigh, 0, (bSignaled) ? (1) : (0));
    if (VM_OK != vmRes)
        return UMC_ERR_INIT;

    vmRes = vm_event_init(&m_hLow, 0, (bSignaled) ? (0) : (1));
    if (VM_OK != vmRes)
        return UMC_ERR_INIT;

    return UMC_OK;

} // Status Pendulum::Init(bool bSignaled)

Status Pendulum::Reset(bool bSignaled)
{
    if (vm_event_is_valid(&m_hHigh))
    {
        if (bSignaled)
            vm_event_signal(&m_hHigh);
        else
            vm_event_reset(&m_hHigh);
    }

    if (vm_event_is_valid(&m_hLow))
    {
        if (bSignaled)
            vm_event_reset(&m_hLow);
        else
            vm_event_signal(&m_hLow);
    }

    return UMC_OK;

} // Status Pendulum::Reset(bool bSignaled)

Status Pendulum::Set(void)
{
    // wait pendulum is non-signaled
    vm_event_wait(&m_hLow);

    // set pendulum to signaled state
    vm_event_signal(&m_hHigh);

    return UMC_OK;

} // Status Pendulum::Set(void)

Status Pendulum::Wait(void)
{
    // wait pendulum is signaled
    vm_event_wait(&m_hHigh);

    // set pendulum to signaled state
    vm_event_signal(&m_hLow);

    return UMC_OK;

} // Status Pendulum::Wait(void)

} // namespace UMC
