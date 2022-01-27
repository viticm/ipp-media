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

#if !defined( __IPP_IWPP_SIGNAL_TRANSFORM__ )
#define __IPP_IWPP_SIGNAL_TRANSFORM__

#include "iw/iw_signal_transform.h"
#include "iw++/iw_signal.hpp"

namespace ipp
{

/**/////////////////////////////////////////////////////////////////////////////
//                   iwsDCT
///////////////////////////////////////////////////////////////////////////// */

// Auxiliary parameters class
// C API descriptions has more details
class IwsDCTParams: public ::IwsDCTParams
{
public:
    IW_BASE_PARAMS_CONSTRUCTORS(IwsDCTParams, iwsDCT_SetDefaultParams)
    IwsDCTParams(IppHintAlgorithm _algoMode = ippAlgHintNone)
    {
        this->algoMode = _algoMode;
    }
};

// Applies DCT to the source vector
// C API descriptions has more details
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwsDCT(
    const IwsVector    &srcVector, // [in]     Reference to the source vector
    IwsVector          &dstVector, // [in,out] Reference to the destination vector
    IwTransDirection    direction, // [in]     Transform direction
    const IwsDCTParams &auxParams  // [in]     Reference to the auxiliary parameters structure
)
{
    IppStatus ippStatus = ::iwsDCT(&srcVector, &dstVector, direction, &auxParams);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

}

#endif
