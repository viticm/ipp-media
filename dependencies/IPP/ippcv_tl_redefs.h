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


#if !defined( IPPCV_TL_REDEFS_H__ )
#define IPPCV_TL_REDEFS_H__


#define ippiFilterSeparableGetBufferSize ippiFilterSeparableGetBufferSize_T
#define ippiFilterSeparableGetSpecSize ippiFilterSeparableGetSpecSize_T
#define ippiFilterSeparableGetBufferSize_L ippiFilterSeparableGetBufferSize_LT
#define ippiFilterSeparableGetSpecSize_L ippiFilterSeparableGetSpecSize_LT

#define ippiFilterSeparableInit_16s ippiFilterSeparableInit_16s_T_Proxy
#define ippiFilterSeparableInit_32f ippiFilterSeparableInit_32f_T_Proxy
#define ippiFilterSeparableInit_16s_L ippiFilterSeparableInit_16s_LT_Proxy
#define ippiFilterSeparableInit_32f_L ippiFilterSeparableInit_32f_LT_Proxy

#define ippiFilterSeparable_8u_C1R ippiFilterSeparable_8u_C1R_T_Proxy
#define ippiFilterSeparable_8u_C3R ippiFilterSeparable_8u_C3R_T_Proxy
#define ippiFilterSeparable_8u_C4R ippiFilterSeparable_8u_C4R_T_Proxy
#define ippiFilterSeparable_16s_C1R ippiFilterSeparable_16s_C1R_T_Proxy
#define ippiFilterSeparable_16s_C3R ippiFilterSeparable_16s_C3R_T_Proxy
#define ippiFilterSeparable_16s_C4R ippiFilterSeparable_16s_C4R_T_Proxy
#define ippiFilterSeparable_16u_C1R ippiFilterSeparable_16u_C1R_T_Proxy
#define ippiFilterSeparable_16u_C3R ippiFilterSeparable_16u_C3R_T_Proxy
#define ippiFilterSeparable_16u_C4R ippiFilterSeparable_16u_C4R_T_Proxy
#define ippiFilterSeparable_32f_C1R ippiFilterSeparable_32f_C1R_T_Proxy
#define ippiFilterSeparable_32f_C3R ippiFilterSeparable_32f_C3R_T_Proxy
#define ippiFilterSeparable_32f_C4R ippiFilterSeparable_32f_C4R_T_Proxy
#define ippiFilterSeparable_8u16s_C1R ippiFilterSeparable_8u16s_C1R_T_Proxy
#define ippiFilterSeparable_8u16s_C3R ippiFilterSeparable_8u16s_C3R_T_Proxy
#define ippiFilterSeparable_8u16s_C4R ippiFilterSeparable_8u16s_C4R_T_Proxy
#define ippiFilterSeparable_8u_C1R_L ippiFilterSeparable_8u_C1R_LT_Proxy
#define ippiFilterSeparable_8u_C3R_L ippiFilterSeparable_8u_C3R_LT_Proxy
#define ippiFilterSeparable_8u_C4R_L ippiFilterSeparable_8u_C4R_LT_Proxy
#define ippiFilterSeparable_16u_C1R_L ippiFilterSeparable_16u_C1R_LT_Proxy
#define ippiFilterSeparable_16u_C3R_L ippiFilterSeparable_16u_C3R_LT_Proxy
#define ippiFilterSeparable_16u_C4R_L ippiFilterSeparable_16u_C4R_LT_Proxy
#define ippiFilterSeparable_16s_C1R_L ippiFilterSeparable_16s_C1R_LT_Proxy
#define ippiFilterSeparable_16s_C3R_L ippiFilterSeparable_16s_C3R_LT_Proxy
#define ippiFilterSeparable_16s_C4R_L ippiFilterSeparable_16s_C4R_LT_Proxy
#define ippiFilterSeparable_32f_C1R_L ippiFilterSeparable_32f_C1R_LT_Proxy
#define ippiFilterSeparable_32f_C3R_L ippiFilterSeparable_32f_C3R_LT_Proxy
#define ippiFilterSeparable_32f_C4R_L ippiFilterSeparable_32f_C4R_LT_Proxy
#define ippiFilterSeparable_8u16s_C1R_L ippiFilterSeparable_8u16s_C1R_LT_Proxy
#define ippiFilterSeparable_8u16s_C3R_L ippiFilterSeparable_8u16s_C3R_LT_Proxy
#define ippiFilterSeparable_8u16s_C4R_L ippiFilterSeparable_8u16s_C4R_LT_Proxy


#ifdef __cplusplus
extern "C" {
#endif

IPPAPI(IppStatus, ippiFilterSeparableInit_16s_T_Proxy, (const Ipp16s* pRowKernel, const Ipp16s* pColumnKernel, IppiSize kernelSize, int divisor, int scaleFactor, IppDataType dataType, int numChannels, IppiFilterSeparableSpec* pSpec))
IPPAPI(IppStatus, ippiFilterSeparableInit_32f_T_Proxy, (const Ipp32f* pRowKernel, const Ipp32f* pColumnKernel, IppiSize kernelSize, IppDataType dataType, int numChannels, IppiFilterSeparableSpec* pSpec))
IPPAPI(IppStatus, ippiFilterSeparableInit_16s_LT_Proxy, (const Ipp16s* pRowKernel, const Ipp16s* pColumnKernel, IppiSize kernelSize, int divisor, int scaleFactor, IppDataType dataType, int numChannels, IppiFilterSeparableSpec* pSpec))
IPPAPI(IppStatus, ippiFilterSeparableInit_32f_LT_Proxy, (const Ipp32f* pRowKernel, const Ipp32f* pColumnKernel, IppiSize kernelSize, IppDataType dataType, int numChannels, IppiFilterSeparableSpec* pSpec))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C1R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C3R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C4R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C1R_T_Proxy, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C3R_T_Proxy, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C4R_T_Proxy, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C1R_T_Proxy, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C3R_T_Proxy, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C4R_T_Proxy, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C1R_T_Proxy, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C3R_T_Proxy, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C4R_T_Proxy, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C1R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C3R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C4R_T_Proxy, (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C1R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C3R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u_C4R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp8u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C1R_LT_Proxy, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C3R_LT_Proxy, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16s_C4R_LT_Proxy, (const Ipp16s* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16s borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C1R_LT_Proxy, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C3R_LT_Proxy, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_16u_C4R_LT_Proxy, (const Ipp16u* pSrc, IppSizeL srcStep, Ipp16u* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp16u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C1R_LT_Proxy, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C3R_LT_Proxy, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_32f_C4R_LT_Proxy, (const Ipp32f* pSrc, IppSizeL srcStep, Ipp32f* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp32f borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C1R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C3R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiFilterSeparable_8u16s_C4R_LT_Proxy, (const Ipp8u* pSrc, IppSizeL srcStep, Ipp16s* pDst, IppSizeL dstStep, IppiSizeL roiSize, IppiBorderType borderType, Ipp8u* borderValue, const IppiFilterSeparableSpec* pSpec, Ipp8u* pBuffer))

#ifdef __cplusplus
}
#endif

#endif /* IPPCV_TL_REDEFS_H__ */
