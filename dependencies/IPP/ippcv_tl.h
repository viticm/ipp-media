/*
// Copyright 2020-2021 Intel Corporation All Rights Reserved.
//
// The source code, information and material ("Material") contained herein is
// owned by Intel Corporation or its suppliers or licensors, and title
// to such Material remains with Intel Corporation or its suppliers or
// licensors. The Material contains proprietary information of Intel
// or its suppliers and licensors. The Material is protected by worldwide
// copyright laws and treaty provisions. No part of the Material may be used,
// copied, reproduced, modified, published, uploaded, posted, transmitted,
// distributed or disclosed in any way without Intel's prior express written
// permission. No license under any patent, copyright or other intellectual
// property rights in the Material is granted to or conferred upon you,
// either expressly, by implication, inducement, estoppel or otherwise.
// Any license under such intellectual property rights must be express and
// approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing,
// you may not remove or alter this notice or any other notice embedded in
// Materials by Intel or Intel's suppliers or licensors in any way.
//
*/

/*
//              Intel(R) Integrated Performance Primitives (Intel(R) IPP)
//              Computer Vision (ippCV_TL)
//
//
*/


#if !defined( IPPCV_TL_H__ )
#define IPPCV_TL_H__

#ifndef IPPDEFS_L_H__
#include "ippdefs_l.h"
#endif

#include "ippcv_l.h"

#if defined( IPP_ENABLED_THREADING_LAYER_REDEFINITIONS )
#include "ippcv_tl_redefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*                                   Separable Filters                                    *
\****************************************************************************************/

/*///////////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiFilterSeparableGetBufferSize_LT, ippiFilterSeparableGetSpecSize_T
//             ippiFilterSeparableGetSpecSize_LT, ippiFilterSeparableGetSpecSize_T
//             ippiFilterSeparableInit_16s_LT, ippiFilterSeparableInit_16s_T
//             ippiFilterSeparableInit_32f_LT, ippiFilterSeparableInit_32f_T
//             ippiFilterSeparable_8u_C1R_LT,      ippiFilterSeparable_8u_C3R_LT,      ippiFilterSeparable_8u_C4R_LT
//             ippiFilterSeparable_8u16s_C1R_LT,   ippiFilterSeparable_8u16s_C3R_LT,   ippiFilterSeparable_8u16s_C4R_LT
//             ippiFilterSeparable_16s_C1R_LT,     ippiFilterSeparable_16s_C3R_LT,     ippiFilterSeparable_16s_C4R_LT
//             ippiFilterSeparable_16u_C1R_LT,     ippiFilterSeparable_16u_C3R_LT,     ippiFilterSeparable_16u_C4R_LT
//             ippiFilterSeparable_32f_C1R_LT,     ippiFilterSeparable_32f_C3R_LT,     ippiFilterSeparable_32f_C4R_LT
//             ippiFilterSeparable_8u_C1R_T,      ippiFilterSeparable_8u_C3R_T,      ippiFilterSeparable_8u_C4R_T
//             ippiFilterSeparable_8u16s_C1R_T,   ippiFilterSeparable_8u16s_C3R_T,   ippiFilterSeparable_8u16s_C4R_T
//             ippiFilterSeparable_16s_C1R_T,     ippiFilterSeparable_16s_C3R_T,     ippiFilterSeparable_16s_C4R_T
//             ippiFilterSeparable_16u_C1R_T,     ippiFilterSeparable_16u_C3R_T,     ippiFilterSeparable_16u_C4R_T
//             ippiFilterSeparable_32f_C1R_T,     ippiFilterSeparable_32f_C3R_T,     ippiFilterSeparable_32f_C4R_T
//  Purpose:   Convolves source image rows and columns with the row and column kernels
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Zero divisor
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    pDst                     The pointer to the destination image
//    srcStep                  The step in the source image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    borderType               Type of the border
//    borderValue              Pointer to the constant value(s) if border type equals ippBorderConstant
//    pSpec                    Pointer to the allocated and initialized context structure
//    pBuffer                  The pointer to the working buffer
//    kernelSize               Sizes of row and column kernels
//    dataType                 Data type of source image {ipp8u|ipp16s|ipp16u|ipp32f}
//    kernelType               Kernel type {ipp16s|ipp32f}
//    numChannels              Number of channels, possible values are 1, 3 or 4
//    pBufferSize              Pointer to the size (in bytes) of the external buffer
//    pSpecSize                Pointer to the size (in bytes) of the spec structure
//    pRowKernel               Pointer to row kernel
//    pColumnKernel            Pointer to column kernel
//    divisor                  The integer value by which the computed result is divided
//    scaleFactor              The integer value by which the computed result is scaled
*/

IPPAPI(IppStatus, ippiFilterSeparableGetBufferSize_LT, (IppiSizeL roiSize, IppiSize kernelSize, IppDataType dataType, IppDataType kernelType, int numChannels, IppSizeL* pBufferSize))
IPPAPI(IppStatus, ippiFilterSeparableGetSpecSize_LT, (IppiSize kernelSize, IppDataType dataType, int numChannels, int* pSpecSize))

IPPAPI(IppStatus, ippiFilterSeparableGetBufferSize_T, (IppiSize roiSize, IppiSize kernelSize, IppDataType dataType, IppDataType kernelType, int numChannels, int* pBufferSize))
IPPAPI(IppStatus, ippiFilterSeparableGetSpecSize_T, (IppiSize kernelSize, IppDataType dataType, int numChannels, int* pSpecSize))

IPPAPI(IppStatus, ippiFilterSeparableInit_16s_LT, (const Ipp16s* pRowKernel, const Ipp16s* pColumnKernel, IppiSize kernelSize, int divisor, int scaleFactor, IppDataType dataType,
    int numChannels, IppiFilterSeparableSpec_LT* pSpec))
IPPAPI(IppStatus, ippiFilterSeparableInit_32f_LT, (const Ipp32f* pRowKernel, const Ipp32f* pColumnKernel, IppiSize kernelSize, IppDataType dataType,
    int numChannels, IppiFilterSeparableSpec_LT* pSpec))

IPPAPI(IppStatus, ippiFilterSeparableInit_16s_T, (const Ipp16s* pRowKernel, const Ipp16s* pColumnKernel, IppiSize kernelSize, int divisor, int scaleFactor, IppDataType dataType,
    int numChannels, IppiFilterSeparableSpec_T* pSpec))
IPPAPI(IppStatus, ippiFilterSeparableInit_32f_T, (const Ipp32f* pRowKernel, const Ipp32f* pColumnKernel, IppiSize kernelSize, IppDataType dataType,
    int numChannels, IppiFilterSeparableSpec_T* pSpec))

IPPAPI(IppStatus, ippiFilterSeparable_8u_C1R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C3R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C4R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_8u_C1R_T, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C3R_T, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C4R_T, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C1R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C3R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C4R_LT, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C1R_T, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C3R_T, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C4R_T, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_16s_C1R_LT, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C3R_LT, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16s* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C4R_LT, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16s* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_16s_C1R_T, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C3R_T, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16s* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C4R_T, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16s* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_16u_C1R_LT, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C3R_LT, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C4R_LT, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp16u* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_16u_C1R_T, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C3R_T, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C4R_T, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp16u* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_32f_C1R_LT, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C3R_LT, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp32f* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C4R_LT, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize,
    IppiBorderType borderType, Ipp32f* borderValue, const IppiFilterSeparableSpec_LT* pSpec, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSeparable_32f_C1R_T, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C3R_T, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp32f* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C4R_T, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
    IppiBorderType borderType, Ipp32f* borderValue, const IppiFilterSeparableSpec_T* pSpec, Ipp8u* pBuffer))

#if defined __cplusplus
}
#endif

#endif /* IPPCV_TL_H__ */
