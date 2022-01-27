/*******************************************************************************
* Copyright 2016-2021 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/

#if !defined( __IPP_IW_SIGNAL_TRANSFORM__ )
#define __IPP_IW_SIGNAL_TRANSFORM__

#include "iw/iw_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////////////////////////
//                   iwsDCT
///////////////////////////////////////////////////////////////////////////// */

// Auxiliary parameters structure
typedef struct _IwsDCTParams
{
    IppHintAlgorithm algoMode;   // Accuracy mode
} IwsDCTParams;

// Sets auxiliary parameters to default values
static IW_INLINE void iwsDCT_SetDefaultParams(
    IwsDCTParams *pParams      // [in,out] Pointer to the auxiliary parameters structure
)
{
    if(pParams)
    {
        pParams->algoMode = ippAlgHintNone;
    }
}

// Applies Discrete Cosine Transform (DCT) to the source vector
// Features support:
//      Inplace mode:            yes
//      64-bit sizes:            no
//      Internal threading:      no
//      Manual tiling:           yes
//      IwsTile simple tiling:   no
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwsDCT(
    const IwsVector    *pSrcVector, // [in]     Pointer to the source vector
    IwsVector          *pDstVector, // [in,out] Pointer to the destination vector
    IwTransDirection    direction,  // [in]     Transform direction
    const IwsDCTParams *pAuxParams  // [in]     Pointer to the auxiliary parameters structure. If NULL - default parameters will be used
);

#ifdef __cplusplus
}
#endif

#endif
