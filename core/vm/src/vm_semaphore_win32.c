/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2006 Intel Corporation. All Rights Reserved.
//
*/

#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WCE)

#include "vm_semaphore.h"
#include "limits.h"

/* Invalidate a semaphore */
void vm_semaphore_set_invalid(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return;

    sem->handle = NULL;

} /* void vm_semaphore_set_invalid(vm_semaphore *sem) */

/* Verify if a semaphore is valid */
Ipp32s vm_semaphore_is_valid(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return 0;

    return (Ipp32s)(0 != sem->handle);

} /* Ipp32s vm_semaphore_is_valid(vm_semaphore *sem) */

/* Init a semaphore with value */
vm_status vm_semaphore_init(vm_semaphore *sem, Ipp32s count)
{
    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    sem->handle = CreateSemaphore(NULL, count, LONG_MAX, 0);
    return (0 != sem->handle) ? VM_OK : VM_OPERATION_FAILED;

} /* vm_status vm_semaphore_init(vm_semaphore *sem, Ipp32s count) */

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_timedwait(vm_semaphore *sem, Ipp32u msec)
{
    vm_status umcRes = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (sem->handle)
    {
        Ipp32u dwRes = WaitForSingleObject(sem->handle, msec);
        umcRes = VM_OK;

        if (WAIT_TIMEOUT == dwRes)
            umcRes = VM_TIMEOUT;
        else if (WAIT_OBJECT_0 != dwRes)
            umcRes = VM_OPERATION_FAILED;
    }
    return umcRes;

} /* vm_status vm_semaphore_timedwait(vm_semaphore *sem, Ipp32u msec) */

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_wait(vm_semaphore *sem)
{
    vm_status umcRes = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (sem->handle)
    {
        umcRes = VM_OK;
        if (WAIT_OBJECT_0 != WaitForSingleObject(sem->handle, INFINITE))
            umcRes = VM_OPERATION_FAILED;
    }
    return umcRes;

} /* vm_status vm_semaphore_wait(vm_semaphore *sem) */

/* Decrease the semaphore value without blocking, return 1 if success */
vm_status vm_semaphore_try_wait(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    return vm_semaphore_timedwait(sem, 0);

} /* vm_status vm_semaphore_try_wait(vm_semaphore *sem) */

/* Increase the semaphore value */
vm_status vm_semaphore_post(vm_semaphore *sem)
{
    vm_status umcRes = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (sem->handle)
    {
        if (ReleaseSemaphore(sem->handle, 1, NULL))
            umcRes = VM_OK;
        else
            umcRes = VM_OPERATION_FAILED;
    }
    return umcRes;

} /* vm_status vm_semaphore_post(vm_semaphore *sem) */

/* Destroy a semaphore */
void vm_semaphore_destroy(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return;

    if (sem->handle)
    {
        CloseHandle(sem->handle);
        sem->handle = NULL;
    }
} /* void vm_semaphore_destroy(vm_semaphore *sem) */

#endif /* defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WCE) */
