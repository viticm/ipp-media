/* 
// Copyright 2014-2021 Intel Corporation All Rights Reserved.
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
//              Embedded functionality (ippE)
// 
// 
*/


#if !defined( IPPE_H__ ) || defined( _OWN_BLDPCS )
#define IPPE_H__

#ifndef IPPDEFS_H__
#include "ippdefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#if !defined( IPP_NO_DEFAULT_LIB )
  #if defined( _IPP_SEQUENTIAL_DYNAMIC )
    #pragma comment( lib, "ippe" )
    #pragma comment( lib, "ippcore" )
  #elif defined( _IPP_SEQUENTIAL_STATIC )
    #pragma comment( lib, "ippemt" )
    #pragma comment( lib, "ippsmt" )
    #pragma comment( lib, "ippvmmt" )
    #pragma comment( lib, "ippcoremt" )
  #elif defined( _IPP_PARALLEL_DYNAMIC )
    #pragma comment( lib, "threaded/ippe" )
    #pragma comment( lib, "threaded/ippcore" )
  #elif defined( _IPP_PARALLEL_STATIC )
    #pragma comment( lib, "threaded/ippemt" )
    #pragma comment( lib, "threaded/ippsmt" )
    #pragma comment( lib, "threaded/ippvmmt" )
    #pragma comment( lib, "threaded/ippcoremt" )
  #endif
#endif

#if !defined( _OWN_BLDPCS )

#endif  /* _OWN_BLDPCS */



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippeGetLibVersion
//  Purpose:    get the library version
//  Parameters:
//  Returns:    pointer to structure describing version of the ippE library
//
//  Notes:      don't free the pointer
*/
IPPAPI( const IppLibraryVersion*, ippeGetLibVersion, (void) )

/*LTE MIMO*/
#if !defined( _OWN_BLDPCS )
typedef struct {
     Ipp16sc symb[4];
} IppFourSymb;
#endif
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for opening a ticket and providing feedback: https://supporttickets.intel.com/ if you have concerns/")\
IPPAPI(IppStatus, ippsMimoMMSE_1X2_16sc,(Ipp16sc* pSrcH[2],int srcHStride2, int srcHStride1, int srcHStride0, Ipp16sc* pSrcY[4][12], int Sigma2,IppFourSymb* pDstX, int dstXStride1, int dstXStride0,int numSymb, int numSC, int SINRIdx,Ipp32f* pDstSINR, int scaleFactor))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for opening a ticket and providing feedback: https://supporttickets.intel.com/ if you have concerns/")\
IPPAPI(IppStatus, ippsMimoMMSE_2X2_16sc,(Ipp16sc* pSrcH[2],int srcHStride2, int srcHStride1, int srcHStride0, Ipp16sc* pSrcY[4][12], int Sigma2,IppFourSymb* pDstX, int dstXStride1, int dstXStride0,int numSymb, int numSC, int SINRIdx,Ipp32f* pDstSINR, int scaleFactor))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for opening a ticket and providing feedback: https://supporttickets.intel.com/ if you have concerns/")\
IPPAPI(IppStatus, ippsMimoMMSE_1X4_16sc,(Ipp16sc* pSrcH[2],int srcHStride2, int srcHStride1, int srcHStride0, Ipp16sc* pSrcY[4][12], int Sigma2,IppFourSymb* pDstX, int dstXStride1, int dstXStride0,int numSymb, int numSC, int SINRIdx,Ipp32f* pDstSINR, int scaleFactor))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for opening a ticket and providing feedback: https://supporttickets.intel.com/ if you have concerns/")\
IPPAPI(IppStatus, ippsMimoMMSE_2X4_16sc,(Ipp16sc* pSrcH[2],int srcHStride2, int srcHStride1, int srcHStride0, Ipp16sc* pSrcY[4][12], int Sigma2,IppFourSymb* pDstX, int dstXStride1, int dstXStride0,int numSymb, int numSC, int SINRIdx,Ipp32f* pDstSINR, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCRC24
//  Purpose:    computes check-sum for given data vector
//  Parameters:
//   pSrc               pointer to the source vector
//   len                length of the vector, number of items
//   pCRC24             pointer to the checksum value
//   Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to source vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsCRC24a_8u, (Ipp8u* pSrc, int len, Ipp32u* pCRC24))
IPPAPI(IppStatus, ippsCRC24b_8u, (Ipp8u* pSrc, int len, Ipp32u* pCRC24))
IPPAPI(IppStatus, ippsCRC24c_8u, (Ipp8u* pSrc, int len, Ipp32u* pCRC24))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCRC16
//  Purpose:    computes check-sum for given data vector
//  Parameters:
//   pSrc               pointer to the source vector
//   len                length of the vector, number of items
//   pCRC16             pointer to the checksum value
//   Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to source vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsCRC16_8u, (Ipp8u* pSrc, int len, Ipp32u* pCRC16))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCRC24_1u
//  Purpose:    computes check-sum for given data vector
//  Parameters:
//   pSrc               pointer to the source vector
//   srcBitOffset       offset in bits from pSrc
//   pDst               pointer to CRC destination vector
//   dstBitOffset       offset in bits from pDst
//   bitLen             length of input vector in bits
//   Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to source vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsCRC24a_1u, (Ipp8u* pSrc, int srcBitOffset, Ipp8u* pDst, int dstBitOffset, int bitLen))
IPPAPI(IppStatus, ippsCRC24b_1u, (Ipp8u* pSrc, int srcBitOffset, Ipp8u* pDst, int dstBitOffset, int bitLen))
IPPAPI(IppStatus, ippsCRC24c_1u, (Ipp8u* pSrc, int srcBitOffset, Ipp8u* pDst, int dstBitOffset, int bitLen))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCRC16_1u
//  Purpose:    computes check-sum for given data vector
//  Parameters:
//   pSrc               pointer to the source vector
//   srcBitOffset       offset in bits from pSrc
//   pDst               pointer to CRC destination vector
//   dstBitOffset       offset in bits from pDst
//   bitLen             length of input vector in bits
//   Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to source vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsCRC16_1u, (Ipp8u* pSrc, int srcBitOffset, Ipp8u* pDst, int dstBitOffset, int bitLen))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCRC_8u
//                
//  Purpose:    computes check-sum for given data vector
//  Parameters:
//   pSrc               pointer to the source vector
//   len                length of the vector, number of items
//   poly               CRC polynomial with explicit leading 1(indicates CRC length 8/16/24/32 bits)
//   optPoly            the data table initialized by ippsGenCRCOptPoly_8u(NULL by default) 
//   init               initial value of register
//   pCRC               pointer to CRC value

//   Return:
//   ippStsNoErr                  Ok
//   ippStsNullPtrErr             pointer to source vector is NULL
//   ippStsSizeErr                length of a vector is less or equal 0
//   ippStsAlgTypeErr             leading 1 in polynomial incorrect
//   ippStsBadArgErr              optPoly is not correct 
//   ippStsNonOptimalPathSelected initialize optPoly by ippsGenCRCOptPoly_8u
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsCRC_8u, (const Ipp8u* pSrc, int len, Ipp64u poly, const Ipp8u optPoly[128], Ipp32u init, Ipp32u* pCRC))
/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsGenCRCOptPoly_8u
//                
//  Purpose:    computes optimization table for ippsCRC_8u
//              ippsCRC_8u has optimization for some fixed polynomials only
//              and returns 'ippStsNonOptimalPathSelected' warning in other case.
//              ippsGenCRCOptPoly_8u generates table for arbitrary polynomial
//                   
//  Parameters:
//   poly               CRC polynomial with explicit leading 1(indicates CRC length 8/16/24/32 bits)
//   optPoly            the data table initialized by ippsGenCRCOptPoly_8u(NULL by default) 
//                      1. ippsCRC_8u(..,poly, NULL,..) - 'ippStsNoErr' this 'poly' already has table 
//                                                        'ippStsNonOptimalPathSelected' function works slow. see 2.   
//                      2. ippsGenCRCOptPoly_8u(poly, optPoly) - calculates table for 'poly' 
//                         ippsCRC_8u(..,poly, optPoly,..) - function works fast.    
//
//   Return:
//   ippStsNoErr        Ok;
//   ippStsNullPtrErr   pointer to source vector is NULL
//   ippStsAlgTypeErr   leading 1 in polynomial incorrect
//   ippStsNoErr        otherwise
*/

IPPAPI(IppStatus, ippsGenCRCOptPoly_8u, (Ipp64u poly, Ipp8u optPoly[128]))

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifdef __cplusplus
}
#endif

#endif /* IPPE_H__ */
