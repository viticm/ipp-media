/* 
// Copyright 2015 Intel Corporation All Rights Reserved.
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

/* /////////////////////////////////////////////////////////////////////////////
//
//         Intel(R) Integrated Performance Primitives
//             Color Conversion Library (ippCC)
//                     Legacy Library
//
*/

#if !defined( __IPPCC_90_LEGACY_H__ )
#define __IPPCC_90_LEGACY_H__

#include "ippdefs90legacy.h"
#include "ippcc90legacy_redef.h"

#ifdef __cplusplus
extern "C" {
#endif


/*//////////////////////////////////////////////////////////////////////////////
//  Core functionality for legacy libraries
//////////////////////////////////////////////////////////////////////////////*/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippInit
//  Purpose:    Automatic switching to best for current cpu library code using.
//  Returns:
//   ippStsNoErr
//
//  Parameter:  nothing
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
LEGACY90IPPAPI( IppStatus, legacy90ippccInit, ( void ))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippSetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNoOperation        For static library internal threading is not supported
//    ippStsSizeErr            Desired number of threads less or equal zero
//
//  Arguments:
//    numThr                   Desired number of threads
*/
LEGACY90IPPAPI( IppStatus, legacy90ippccSetNumThreads, ( int numThr ) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Pointer to numThr is Null
//    ippStsNoOperation        For static library internal threading is not supported
//                             and return value is always == 1
//
//  Arguments:
//    pNumThr                  Pointer to memory location where to store current numThr
*/
LEGACY90IPPAPI( IppStatus, legacy90ippccGetNumThreads, (int* pNumThr) )

/*////////////////////////////////////////////////////////////////////////////*/


/* ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippccGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippCC library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
LEGACY90IPPAPI( const IppLibraryVersion*, legacy90ippccGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//                   Color Space  Conversion Functions
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//  ippiBGRToCbYCr422_8u_AC4C2R,       ippiCbYCr422ToBGR_8u_C2C4R,
//  ippiYCbCr411ToBGR_8u_P3C3R
//  ippiYCbCr411ToBGR_8u_P3C4R
//
//  ippiRGBToCbYCr422_8u_C3C2R,        ippiCbYCr422ToRGB_8u_C2C3R,
//  ippiRGBToCbYCr422Gamma_8u_C3C2R,
//  ippiYCbCr422ToRGB_8u_P3C3R
//
//  ippiRGBToYCbCr422_8u_C3C2R,        ippiYCbCr422ToRGB_8u_C2C3R,
//                                     ippiYCbCr422ToRGB_8u_C2C4R,
//  ippiRGBToYCbCr422_8u_P3C2R,        ippiYCbCr422ToRGB_8u_C2P3R,
//
//  ippiRGBToYCbCr420_8u_C3P3R,        ippiYCbCr420ToRGB_8u_P3C3R,
//  ippiYCbCr420ToBGR_8u_P3C3R,
//
//  ippiYCbCr422ToRGB565_8u16u_C2C3R,  ippiYCbCr422ToBGR565_8u16u_C2C3R,
//  ippiYCbCr422ToRGB555_8u16u_C2C3R,  ippiYCbCr422ToBGR555_8u16u_C2C3R,
//  ippiYCbCr422ToRGB444_8u16u_C2C3R,  ippiYCbCr422ToBGR444_8u16u_C2C3R,
//
//  ippiYCbCrToRGB565_8u16u_P3C3R,     ippiYCbCrToBGR565_8u16u_P3C3R,
//  ippiYCbCrToRGB444_8u16u_P3C3R,     ippiYCbCrToBGR444_8u16u_P3C3R,
//  ippiYCbCrToRGB555_8u16u_P3C3R,     ippiYCbCrToBGR555_8u16u_P3C3R,
//
//  ippiYCbCr420ToRGB565_8u16u_P3C3R,  ippiYCbCr420ToBGR565_8u16u_P3C3R
//  ippiYCbCr420ToRGB555_8u16u_P3C3R,  ippiYCbCr420ToBGR555_8u16u_P3C3R,
//  ippiYCbCr420ToRGB444_8u16u_P3C3R,  ippiYCbCr420ToBGR444_8u16u_P3C3R,
//
//  ippiYCbCr422ToRGB565_8u16u_P3C3R,  ippiYCbCr422ToBGR565_8u16u_P3C3R,
//  ippiYCbCr422ToRGB555_8u16u_P3C3R,  ippiYCbCr422ToBGR555_8u16u_P3C3R,
//  ippiYCbCr422ToRGB444_8u16u_P3C3R,  ippiYCbCr422ToBGR444_8u16u_P3C3R,
//
//  ippiRGBToYCrCb422_8u_C3C2R,        ippiYCrCb422ToRGB_8u_C2C3R,
//                                     ippiYCrCb422ToBGR_8u_C2C3R,
//                                     ippiYCrCb422ToRGB_8u_C2C4R,
//                                     ippiYCrCb422ToBGR_8u_C2C4R,
//  ippiRGBToYCrCb422_8u_P3C2R,        ippiYCrCb422ToRGB_8u_C2P3R,
//
//
//  Purpose:    Converts an RGB/BGR image to the YCbCr/CbYCr/YCrCb image and vice versa.
//  Parameters:
//     pSrc     Pointer to the source image (for pixel-order data).An array of
//              pointers to separate source color planes (for plane-order data)
//     pDst     Pointer to the destination image (for pixel-order data).An array
                of pointers to separate destination color planes (for plane-order data)
//     roiSize  Size of source and destination ROI in pixels
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//     aval     Constant value to create the fourth channel.
//  Returns:
//     ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//     ippStsSizeErr     roiSize has field with zero or negative value
//     ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//
//  The YCbCr color space was developed as part of Recommendation ITU-R BT.601
//  (formerly CCI 601). Y is defined to have a nominal range of 16 to 235;
//  Cb and Cr are defined to have a range of 16 to 240, with 128 equal to zero.
//  The function ippiRGBToYCbCr422_8u_P3C2R uses 4:2:2 sampling format. For every
//  two  horizontal Y samples, there is one Cb and Cr sample.
//  Each pixel in the input RGB image is of 24 bit depth. Each pixel in the
//  output YCbCr image is of 16 bit depth.
//  Sequence of samples in the YCbCr422 image is
//             Y0Cb0Y1Cr0,Y2Cb1Y3Cr1,...
//  Sequence of samples in the CbYCr422 image is:
//             Cb0Y0CrY1,Cb1Y2Cr1Y3,...
//  All functions operate on the gamma-corrected RGB (R'G'B') images
//  (except ippiRGBToCbYCrGamma_8u_C3C2R, see below) with pixel values
//  in the range 0 .. 255, as is commonly found in computer system.
//  Conversion is performed according to the following equations:
//
//       Y  =  0.257*R' + 0.504*G' + 0.098*B' + 16
//       Cb = -0.148*R' - 0.291*G' + 0.439*B' + 128
//       Cr =  0.439*R' - 0.368*G' - 0.071*B' + 128
//
//       R' = 1.164*(Y - 16) + 1.596*(Cr - 128 )
//       G' = 1.164*(Y - 16) - 0.813*(Cr - 128 )- 0.392*( Cb - 128 )
//       B' = 1.164*(Y - 16) + 2.017*(Cb - 128 )
//
//   Note that for the YCbCr-to-RGB equations, the RGB values must be saturated
//   at the 0 and 255 levels due to occasional excursions outside the nominal
//   YCbCr ranges.
//   Note that two-planar YCbCr420 image is also known as NV12 format and YCrCb420 as NV21.
//
//   ippiRGBToCbYCr422Gamma_8u_C3C2R function additionally performs gamma-correction, there is
//   sample down filter(1/4,1/2,1/4).
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB565_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR565_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB555_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR555_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))


LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB444_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR444_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////////
//  Name: ippiYCbCrToBGR(RGB)565(555,444)Dither_8u16u_C3R
//        ippiYCbCrToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYCbCr422ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYCbCr420ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYUV420ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//  Purpose:
//      Converts a YCbCr(YUV) image to the 16-bit per pixel BGR(RGB) image with dithering.
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to separate source color planes (for plane-order data)
//     pDst   Pointer to the destination image (for pixel-order data).An array of pointers
//            to separate destination color planes (for plane-order data)
//     roiSize  Size of the source and destination ROI in pixels.
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  After color conversion bit reduction is performed using Bayer's dithering algorithm
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR565Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB565Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR555Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB555Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR444Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB444Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr420ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB555Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR555Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB565Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR565Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToRGB444Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr422ToBGR444Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGR565ToYUV420_16u8u_C3P3R/ippiBGR555ToYUV420_16u8u_C3P3R
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 0
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYUV420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYUV420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))


/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToBGR565_8u16u_P3C3R / ippiYCbCr411ToBGR555_8u16u_P3C3R
//  Purpose:    Converts a P411 image to the RGB565 / RGB555 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 or if roiSize.height < 1
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     pointer to the destination image
//    dstStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr411ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst,int dstStep,IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCr411ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst,int dstStep,IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr411_8u_C3P3R/ippiBGRToYCbCr411_8u_AC4P3R/ippiBGR565ToYCbCr411_16u8u_C3P3R/ippiBGR555ToYCbCr411_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the P411 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 4 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYCbCr411_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYCbCr411_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr422_8u_C3P3R/ippiBGRToYCbCr422_8u_AC4P3R/ippiBGR565ToYCbCr422_16u8u_C3P3R/ippiBGR555ToYCbCr422_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the P422 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYCbCr422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYCbCr422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr420_8u_C3P3R/ippiBGRToYCbCr420_8u_AC4P3R/ippiBGR565ToYCbCr420_16u8u_C3P3R/ippiBGR555ToYCbCr420_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the IYUV image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 0
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYCbCr420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYCbCr420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr422_8u_C3C2R/ippiBGRToYCbCr422_8u_AC4C2R/ippiBGR555ToYCbCr422_16u8u_C3C2R/ippiBGR565ToYCbCr422_16u8u_C3C2R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the YUY2 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYCbCr422_16u8u_C3C2R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYCbCr422_16u8u_C3C2R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCrCb420_8u_C3P3R/ippiBGRToYCrCb420_8u_AC4P3R/ippiBGR555ToYCrCb420_16u8u_C3P3R/ippiBGR565ToYCrCb420_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the YV12 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 2
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR555ToYCrCb420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToYCrCb420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))



/* /////////////////////////////////////////////////////////////////////////////
//  Name: ippiRGBToYCbCr_8u_C3R,       ippiYCbCrToRGB_8u_C3R.
//        ippiRGBToYCbCr_8u_AC4R,      ippiYCbCrToRGB_8u_AC4R.
//        ippiRGBToYCbCr_8u_P3R,       ippiYCbCrToRGB_8u_P3R.
//        ippiYCbCrToRGB_8u_P3C3R
//        ippiYCbCrToBGR444_8u16u_C3R, ippiYCbCrToRGB444_8u16u_C3R,
//        ippiYCbCrToBGR555_8u16u_C3R, ippiYCbCrToRGB555_8u16u_C3R,
//        ippiYCbCrToBGR565_8u16u_C3R, ippiYCbCrToRGB565_8u16u_C3R,

//  Purpose:    Convert an RGB(BGR) image to and from YCbCr color model
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to separate source color planes (in case of plane-order data)
//     pDst   Pointer to the resultant image (for pixel-order data).An array of pointers
//            to separate source color planes (in case of plane-order data)
//     roiSize Size of the ROI in pixels.
//     srcStep Step in bytes through the source image to jump on the next line
//     dstStep Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  src == NULL or dst == NULL
//           ippStsStepErr,    srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//
//  The YCbCr color space was developed as part of Recommendation ITU-R BT.601
//  (formerly CCI 601). Y is defined to have a nominal range of 16 to 235;
//  Cb and Cr are defined to have a range of 16 to 240, with 128 equal to zero.
//  If the gamma-corrected RGB(R'G'B') image has a range 0 .. 255, as is commonly
//  found in computer system (and in our library), the following equations may be
//  used:
//
//       Y  =  0.257*R' + 0.504*G' + 0.098*B' + 16
//       Cb = -0.148*R' - 0.291*G' + 0.439*B' + 128
//       Cr =  0.439*R' - 0.368*G' - 0.071*B' + 128
//
//       R' = 1.164*(Y - 16) + 1.596*(Cr - 128 )
//       G' = 1.164*(Y - 16) - 0.813*(Cr - 128 )- 0.392*( Cb - 128 )
//       B' = 1.164*(Y - 16) + 2.017*(Cb - 128 )
//
//   Note that for the YCbCr-to-RGB equations, the RGB values must be saturated
//   at the 0 and 255 levels due to occasional excursions outside the nominal
//   YCbCr ranges.
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR444_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB444_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR555_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB555_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToBGR565_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYCbCrToRGB565_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGB565ToYUV420_16u8u_C3P3R/ippiRGB565ToYUV422_16u8u_C3P3R
//  PVCS ID 8910
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//   pSrc        Pointer to the source image ROI for pixel-order image.
//   srcStep     Distance in bytes between starts of consecutive lines in the source image.
//   pDst        An array of pointers to ROI in the separate destination color and alpha planes for planar images.
//   dstStep     Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize     Size of the source and destination ROI in pixels.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiRGB565ToYUV420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiRGB565ToYUV422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToYUV422_8u_C3P3R,  ippiYUV422ToRGB_8u_P3C3R.
//              ippiRGBToYUV422_8u_P3R,    ippiYUV422ToRGB_8u_P3R.
//              ippiRGBToYUV420_8u_C3P3R,  ippiYUV420ToRGB_8u_P3C3R.
//              ippiRGBToYUV422_8u_C3C2R,  ippiYUV422ToRGB_8u_C2C3R.
//         ippiYUV420ToBGR565_8u16u_P3C3R,
//         ippiYUV420ToBGR555_8u16u_P3C3R,
//         ippiYUV420ToBGR444_8u16u_P3C3R,
//         ippiYUV420ToRGB565_8u16u_P3C3R,
//         ippiYUV420ToRGB555_8u16u_P3C3R,
//         ippiYUV420ToRGB444_8u16u_P3C3R.

//  Purpose:    Converts an RGB (BGR) image to the YUV color model with 4:2:2 or
//              4:2:0 sampling and vice versa.
//  Parameters:
//     pSrc  Pointer to the source image (for pixel-order data).An array of pointers
//           to separate source color planes (for plane-order data)
//     pDst  Pointer to the destination image (for pixel-order data).An array of pointers
//           to separate destination color planes (for plane-order data)
//     roiSize   Size of the ROI in pixels.
//     srcStep   Step in bytes through the source image to jump on the next line(for pixel-order data).
//               An array of step values for the separate source color planes (for plane-order data).
//     dstStep   Step in bytes through destination image to jump on the next line(for pixel-order data).
//               An array of step values for the separate destination color planes (for plane-order data).
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(40-41)
//
//     The YUV color space is the basic color space used by the PAL , NTSC , and
//  SECAM composite color video standards.
//
//  The functions operate with 4:2:2 and 4:2:0 sampling formats.
//    4:2:2 uses the horizontal-only 2:1 reduction of U and V,
//    4:2:0 uses 2:1 reduction of U and V in both the vertical and
//    horizontal directions.
//
//  These functions operate with gamma-corrected images.
//  The basic equations for conversion between gamma-corrected RGB(R'G'B')and YUV are:
//
//       Y' =  0.299*R' + 0.587*G' + 0.114*B'
//       U  = -0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B' - Y' )
//       V  =  0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R' - Y' )
//
//       R' = Y' + 1.140 * V
//       G' = Y' - 0.394 * U - 0.581 * V
//       B' = Y' + 2.032 * U
//
//     For digital RGB values with the range [0 .. 255], Y has a range [0..255],
//   U a range [-112 .. +112], and V a range [-157..+157].
//

//   These equations are usually scaled to simplify the implementation in an actual
//   NTSC or PAL digital encoder or decoder.
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiYUV420ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiColorTwist
//
//  Purpose:    Applies a color-twist matrix to an image.
//              |R|   | t11 t12 t13 t14 |   |r|
//              |G| = | t21 t22 t23 t24 | * |g|
//              |B|   | t31 t32 t33 t34 |   |b|
//
//               R = t11*r + t12*g + t13*b + t14
//               G = t21*r + t22*g + t23*b + t24
//               B = t31*r + t32*g + t33*b + t34
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is zero or negative
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc            Pointer to the source image
//    srcStep         Step through the source image
//    pDst            Pointer to the  destination image
//    dstStep         Step through the destination image
//    pSrcDst         Pointer to the source/destination image (in-place flavors)
//    srcDstStep      Step through the source/destination image (in-place flavors)
//    roiSize         Size of the ROI
//    twist           An array of color-twist matrix elements
*/
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_C3R, ( const Ipp8s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_C3IR, ( Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_AC4R, ( const Ipp8s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_AC4IR, ( Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_P3R, ( const Ipp8s* pSrc[3], int srcStep,
                    Ipp8s* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
LEGACY90IPPAPI ( IppStatus, legacy90ippiColorTwist32f_8s_IP3R, ( Ipp8s* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Names:        ippiRGBToRGB565_8u16u_C3R
//                ippiBGRToBGR565_8u16u_C3R
//
//  Purpose:      Converts RGB(BGR) image to RGB565(BGR565) image and vice versa
//
//  Returns:
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       The roiSize has a field with negative or zero value
//    ippStsStepErr       One of steps is less than or equal to zero
//
//  Parameters:
//    pSrc                Pointers to the source images
//    srcStep             Steps through the source image
//    pDst                Pointer to the destination image
//    dstStep             Step through the destination image
//    roiSize             Size of the image ROI
//
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiRGBToRGB565_8u16u_C3R,(const Ipp8u*  pSrc, int srcStep, Ipp16u*  pDst, int dstStep, IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGRToBGR565_8u16u_C3R,(const Ipp8u*  pSrc, int srcStep, Ipp16u*  pDst, int dstStep, IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiRGB565ToRGB_16u8u_C3R,(const Ipp16u* pSrc, int srcStep,  Ipp8u*  pDst, int dstStep, IppiSize roiSize ))
LEGACY90IPPAPI(IppStatus, legacy90ippiBGR565ToBGR_16u8u_C3R,(const Ipp16u* pSrc, int srcStep,  Ipp8u*  pDst, int dstStep, IppiSize roiSize ))


#ifdef __cplusplus
}
#endif

#endif /* __IPPCC_90_LEGACY_H__ */
