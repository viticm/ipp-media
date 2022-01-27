//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

// This file implements the core noise reduction filter class,
// CNoiseReductionFilter.  It contains the constructor, destructor, and
// mainly all the custom interfaces exposed.  Other methods that involve
// the actual spatio-temporal filtering operations are contained in the
// files npf_c.cpp and npf_mmx.cpp.

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include <memory.h>
#include <stdlib.h>

#include "umc_h264_npf.h"
#include "umc_h264_defs.h"
#include "umc_h264_config.h"

// following timing code are for development purposes

//----------------------------------------------------------------------------
// Constructor
namespace UMC_H264_ENCODER
{
template <class PixType>
CNoiseReductionFilter<PixType>::CNoiseReductionFilter(
    const bool , // bIsMMX,
    const bool , // bIsKNI,
    const bool bIs7BitPel)
{
    m_bFirstFrame       = true;
    m_bFirstFrame_MF    = true;
    m_b7BitPel          = bIs7BitPel;
    m_pLocalBuffer      = NULL;
    m_pLastFrame        = NULL;
    m_pLastFrameY       = NULL;
    m_pLastFrameU       = NULL;
    m_pLastFrameV       = NULL;
    m_uLocalBufferSize  = 0;
    m_pTFtapsBuffer     = NULL;
    m_pTFtaps           = NULL;
    m_uBadEdge_Top      = 2;
    m_uBadEdge_Left     = 2;
    m_uBadEdge_Bottom   = 2;
    m_uMode             = GetDefaultNoiseReductionMode();
    m_bEnabled          = IsNoiseReductionEnabledByDefault();

    // Allocate buffer for temporal filter taps
    if ((m_pTFtapsBuffer = new Ipp8s[512 + 8]) != NULL)
        m_pTFtaps = (Ipp8s*)align_pointer<Ipp8s*>(m_pTFtapsBuffer, 8);

    // Set the filter options to their default values
    GetDefaultNoiseReductionOptions(m_options);
    UpdateAndScaleThresholds();

    // No MMX optimization
    m_bIsMMX = false;

#if defined(GET_NPF_TIMING)
    timing_sum = timing_cnt = timing_pels = 0;
#endif

} // (Constructor)


//----------------------------------------------------------------------------
// Destructor
template <class PixType>
CNoiseReductionFilter<PixType>::~CNoiseReductionFilter()
{
    if (m_pLocalBuffer)
    {
        delete [] m_pLocalBuffer;
        m_pLocalBuffer = NULL;
        m_pLastFrame = NULL;
        m_pLastFrameY = NULL;
        m_pLastFrameU = NULL;
        m_pLastFrameV = NULL;
    }
    delete [] m_pTFtapsBuffer;

#if defined(GET_NPF_TIMING)
    Ipp32u timing_avg = timing_cnt ? (timing_sum / timing_cnt) : 0;
    printf("\nNoise Prefilter Avg = %d per frm, %d per pel\n",
        timing_avg, timing_pels ? (timing_avg / timing_pels) : 0);
#endif

} // (Destructor)


//----------------------------------------------------------------------------
template <class PixType>
bool CNoiseReductionFilter<PixType>::Start_Sequence(
    const Ipp32s width,
    const Ipp32s height)
{
    // Forget about any past memory
    m_bFirstFrame       = true;
    m_bFirstFrame_MF    = true;
    m_uBadEdge_Top      = 2;
    m_uBadEdge_Left     = 2;
    m_uBadEdge_Bottom   = 2;

    // Width and height must be 4x and >= 8
    if ((width & 3) || (height & 3) || (width < 8) || (height < 8))
    {
        return false;
    }

    // Pitch not known at this point
    // Local buffer to be allocated later in DoFiltering
    return true;

} // Start_Sequence


//----------------------------------------------------------------------------
// Enable or disable filter

template <class PixType>
bool CNoiseReductionFilter<PixType>::EnableNoiseReduction(const bool enable)
{
    // If being turned off, reset our state for the next time we are enabled
    if (!enable)
    {
        m_bFirstFrame = true;
        m_bFirstFrame_MF = true;
    }

    m_bEnabled = enable;

    return true;

} // EnableNoiseReduction


//----------------------------------------------------------------------------
template <class PixType>
bool CNoiseReductionFilter<PixType>::IsNoiseReductionEnabled()
{
    return m_bEnabled;
}


//----------------------------------------------------------------------------
const bool IS_NOISEPF_ENABLED_BY_DEFAULT = false;

template <class PixType>
bool CNoiseReductionFilter<PixType>::IsNoiseReductionEnabledByDefault()
{
    return IS_NOISEPF_ENABLED_BY_DEFAULT;
}


//----------------------------------------------------------------------------
template <class PixType>
bool CNoiseReductionFilter<PixType>::PutNoiseReductionMode(const Ipp32s mode)
{
    if (!mode || (mode & ~(H264_Noise_Reduction_INR_Mode |
                           H264_Noise_Reduction_NER_Mode |
                           H264_Noise_Reduction_TNR_Mode)))
    {
        return false;
    }

    if (!(m_uMode & H264_Noise_Reduction_INR_Mode) &&
            (mode & H264_Noise_Reduction_INR_Mode))
        m_bFirstFrame_MF = true;

    if (!(m_uMode & H264_Noise_Reduction_TNR_Mode) &&
            (mode & H264_Noise_Reduction_TNR_Mode) && m_bFirstFrame_MF)
        m_bFirstFrame = true;

    m_uMode = mode;
    return true;
}


//----------------------------------------------------------------------------
template <class PixType>
Ipp32u CNoiseReductionFilter<PixType>::GetNoiseReductionMode()
{
    return m_uMode;
}


//----------------------------------------------------------------------------
const Ipp32u DEFAULT_NOISEPF_MODE =
    (H264_Noise_Reduction_INR_Mode | H264_Noise_Reduction_NER_Mode);

template <class PixType>
Ipp32u CNoiseReductionFilter<PixType>::GetDefaultNoiseReductionMode()
{
    return DEFAULT_NOISEPF_MODE;
}


//----------------------------------------------------------------------------
// Get default filter options and store in opt

template <class PixType>
void CNoiseReductionFilter<PixType>::GetDefaultNoiseReductionOptions(
    H264_Noise_Reduction_Options& opt)
{
    opt.y           = true;
    opt.u           = true;
    opt.v           = true;
    opt.tf_thres1   = 6;
    opt.tf_thres2   = 12;
    opt.tf_thres3   = 20;
    opt.mf_simple   = true;
    opt.mf_thres_s  = 15;
    opt.mf_thres_t  = 6;

} // GetDefaultNoiseReductionOptions


//----------------------------------------------------------------------------
// Update data used by the MMX / intrinsic routine with the current filter
// options in m_options
// Note: opt.mf_simple is not used, always apply simplified sort

template <class PixType>
void CNoiseReductionFilter<PixType>::UpdateAndScaleThresholds()
{
    if (m_b7BitPel)
    {
        m_tf_thres1     = m_options.tf_thres1 >> 1;
        m_tf_thres2     = m_options.tf_thres2 >> 1;
        m_tf_thres3     = m_options.tf_thres3 >> 1;
        m_mf_thres_s    = m_options.mf_thres_s >> 1;
        m_mf_thres_t    = m_options.mf_thres_t >> 1;
    }
    else
    {
        m_tf_thres1     = m_options.tf_thres1;
        m_tf_thres2     = m_options.tf_thres2;
        m_tf_thres3     = m_options.tf_thres3;
        m_mf_thres_s    = m_options.mf_thres_s;
        m_mf_thres_t    = m_options.mf_thres_t;
    }

    // Initialize temporal filter taps
    if (m_pTFtapsBuffer != NULL)
    {
        for (Ipp32s i = 0; i < 512; i ++)
        {
            Ipp32s j = i - 255; // ? 16 bits.
            Ipp32u d = labs(j);

            // note: All thresholds are < H264_Noise_Reduction_Max_Thres_TF.
            //       This ensures the difference to fit within 8 bits.
            if (d < m_tf_thres1)
                m_pTFtaps[i] = Ipp8s(((j << 2) + j + 3) >> 3);  // * 0.625
            else
            if (d < m_tf_thres2)
                m_pTFtaps[i] = Ipp8s(j >> 1);   // * 0.5
            else
            if (d < m_tf_thres3)
                m_pTFtaps[i] = Ipp8s(((j << 1) + j + 3) >> 3);  // * 0.375
            else
                m_pTFtaps[i] = 0;
        }
    }

} // UpdateAndScaleThresholds


//----------------------------------------------------------------------------
// Get filter options with given values

template <class PixType>
void CNoiseReductionFilter<PixType>::GetNoiseReductionOptions(
    H264_Noise_Reduction_Options& opt)
{
    opt = m_options;

} // GetNoiseReductionOptions


//----------------------------------------------------------------------------
// Check to see if we need to allocate a local buffer for the filter and
// verify the buffer size

template <class PixType>
bool CNoiseReductionFilter<PixType>::CheckBuffer(
    const Ipp32s y_pitch_pixels,
    const Ipp32s uv_pitch_pixels,
    const Ipp32s width,
    const Ipp32s height)
{
    // First check the filter taps buffer
    if (m_pTFtapsBuffer == NULL)
        return false;

    Ipp32u size;

    if (y_pitch_pixels == uv_pitch_pixels)
        size = y_pitch_pixels * (height + (height >> 1));
    else
        size = (y_pitch_pixels + uv_pitch_pixels) * height;

    // Reallocate last frame buffer if necessary
    if (size > m_uLocalBufferSize)
    {
        delete [] m_pLocalBuffer;
        m_pLocalBuffer = NULL;
        m_pLastFrame = NULL;
        m_pLastFrameY = NULL;
        m_pLastFrameU = NULL;
        m_pLastFrameV = NULL;
    }

    if (!m_pLocalBuffer)
    {
        // Extra 32 bytes for prefetching in the MMX filter
        if ((m_pLocalBuffer = new PixType[size + 32 + 8]) == NULL)
        {
            return false;
        }

        // 8-byte aligned the buffer
        m_pLastFrame = (PixType*)align_pointer<PixType*>(m_pLocalBuffer, 8);
        m_uLocalBufferSize = size;

        // Set up YUV buffer pointers
        m_pLastFrameY = m_pLastFrame;
        m_pLastFrameU = m_pLastFrameY + y_pitch_pixels * height;

        if (y_pitch_pixels == uv_pitch_pixels)
            m_pLastFrameV = m_pLastFrameU + (((width >> 1) + 7) & ~7);
        else
            m_pLastFrameV = m_pLastFrameU + uv_pitch_pixels * (height >> 1);
    }

    // Clear the last frame buffer if this is the first frame to be filtered
    if (m_bFirstFrame)
        memset((void *) m_pLastFrame, 0, m_uLocalBufferSize*sizeof(PixType));

    return true;

} // CheckBuffer


//----------------------------------------------------------------------------
// Set filter options with given values

#define LIMIT_THRESHOLD_RANGE(x) \
    ((x) > H264_Noise_Reduction_Max_Thres ? \
            H264_Noise_Reduction_Max_Thres : (x))

#define LIMIT_THRESHOLD_RANGE_TF(x) \
    ((x) > H264_Noise_Reduction_Max_Thres_TF ? \
            H264_Noise_Reduction_Max_Thres_TF : (x))

template <class PixType>
void CNoiseReductionFilter<PixType>::PutNoiseReductionOptions(
    const H264_Noise_Reduction_Options& opt)
{
    m_options.y             = opt.y;
    m_options.u             = opt.u;
    m_options.v             = opt.v;
    m_options.tf_thres1     = LIMIT_THRESHOLD_RANGE_TF(opt.tf_thres1);
    m_options.tf_thres2     = LIMIT_THRESHOLD_RANGE_TF(opt.tf_thres2);
    m_options.tf_thres3     = LIMIT_THRESHOLD_RANGE_TF(opt.tf_thres3);
    m_options.mf_simple     = opt.mf_simple;
    m_options.mf_thres_s    = LIMIT_THRESHOLD_RANGE(opt.mf_thres_s);
    m_options.mf_thres_t    = LIMIT_THRESHOLD_RANGE(opt.mf_thres_t);

    UpdateAndScaleThresholds();

} // PutNoiseReductionOptions


//----------------------------------------------------------------------------
// Set up calls to filter a single plane
template <class PixType>
#if defined(_MSC_VER)
inline
#endif
void CNoiseReductionFilter<PixType>::FilterPlane(
    PixType* x,
    PixType* xp,
    Ipp32s   pitchPixels,
    Ipp32s   width,
    Ipp32s   height)
{
    if (m_uMode & H264_Noise_Reduction_INR_Mode)
    {
            SpatioTemporalFilter(x, xp, pitchPixels, width, height);
    }
    if (m_uMode & H264_Noise_Reduction_TNR_Mode)
        TemporalFilter(x, xp, pitchPixels, width, height);

} // FilterPlane


//----------------------------------------------------------------------------
// Perform noise reduction filtering on the Y, U, and V planes
//
// Note : Due to limitations in the mmx noise reduction filter, the pitch of
//        the buffer must be 8x.  The width and height of the picture must be
//        4x and no smaller than 8.

template <class PixType>
bool CNoiseReductionFilter<PixType>::DoFiltering(
    PixType* y,
    PixType* u,
    PixType* v,
    Ipp32s   y_pitch_pixels,
    Ipp32s   uv_pitch_pixels,
    Ipp32s   width,
    Ipp32s   height)
{
    // Skip all if the filter has been disabled
    if (!m_bEnabled)
        return true;

    // Make sure that the picture dimensions are supported
    if ((y_pitch_pixels & 3) || (uv_pitch_pixels & 1) || (width & 3) || (height & 3) ||
        (width < 8) || (height < 8))
    {
        return false;
    }

    // Proceed with noisy edge removal if enabled
    if (m_uMode & H264_Noise_Reduction_NER_Mode)
        RemoveNoisyEdges(y, u, v, y_pitch_pixels, uv_pitch_pixels, width, height);

    // Done if none of the other noise reduction modes is enabled
    if (!(m_uMode & H264_Noise_Reduction_INR_Mode) &&
        !(m_uMode & H264_Noise_Reduction_TNR_Mode))
        return true;

    // Make sure that the local buffer size is correct
    if (!CheckBuffer(y_pitch_pixels, uv_pitch_pixels, width, height))
        return false;

    // m_bFirstFrame must be false if m_bFirstFrame_MF is false
    // Filter Y plane if m_options.y is set
    if (m_options.y)
        FilterPlane(y, m_pLastFrameY, y_pitch_pixels, width, height);

    // Update Y plane in last frame buffer
    Ipp32u i, j;
    for (i = 0, j = (height >> 1) * y_pitch_pixels; i < j; i += y_pitch_pixels)
    {
        memcpy(m_pLastFrameY + i, y + i, sizeof(PixType)*width);
        memcpy(m_pLastFrameY + i + j, y + i + j, sizeof(PixType)*width);
    }

    // Adjust dimensions for U and V planes
    width >>= 1;
    height >>= 1;

    // Filter U plane if m_options.u is set
    if (m_options.u)
        FilterPlane(u, m_pLastFrameU, uv_pitch_pixels, width, height);

    // Filter V plane if m_options.v is set
    if (m_options.v)
        FilterPlane(v, m_pLastFrameV, uv_pitch_pixels, width, height);

    // Update U and V planes in last frame buffer
    for (i = 0, j = (height >> 1) * uv_pitch_pixels; i < j; i += uv_pitch_pixels)
    {
        memcpy(m_pLastFrameU + i, u + i, sizeof(PixType)*width);
        memcpy(m_pLastFrameU + i + j, u + i + j, sizeof(PixType)*width);
        memcpy(m_pLastFrameV + i, v + i, sizeof(PixType)*width);
        memcpy(m_pLastFrameV + i + j, v + i + j, sizeof(PixType)*width);
    }

    // Last frame buffer has been filled, clear flag
    m_bFirstFrame = false;

    // Done filtering
    return true;

} // DoFiltering
//----------------------------------------------------------------------------
// Sort given array of 5 into ascending order

template <class PixType>
#if defined(_MSC_VER)
// Some compilation systems don't like "inline" here, given that "inline"
// is not specified at the declaration in "umc_h264_npf.h", and thus not seen
// when Sort5 is called within "npf_c.cpp".
// But, MSVC doesn't seem to mind.
inline
#endif
void CNoiseReductionFilter<PixType>::Sort5(PixType* v)
{
    PixType u;

    if (v[1] > v[0])
    {
        u = v[1];
        v[1] = v[0];
        v[0] = u;
    }

    if (v[2] > v[1])
    {
        u = v[2];
        v[2] = v[1];
        if (u > v[0])
        {
            v[1] = v[0];
            v[0] = u;
        }
        else
            v[1] = u;
    }

    if (v[3] > v[2])
    {
        u = v[3];
        v[3] = v[2];
        if (u > v[1])
        {
            v[2] = v[1];
            if (u > v[0])
            {
                v[1] = v[0];
                v[0] = u;
            }
            else
                v[1] = u;
        }
        else
            v[2] = u;
    }

    if (v[4] > v[3])
    {
        u = v[4];
        v[4] = v[3];
        if (u > v[2])
        {
            v[3] = v[2];
            if (u > v[1])
            {
                v[2] = v[1];
                if (u > v[0])
                {
                    v[1] = v[0];
                    v[0] = u;
                }
                else
                    v[1] = u;
            }
            else
                v[2] = u;
        }
        else
            v[3] = u;
    }

} // Sort5


//----------------------------------------------------------------------------
// Sort given array v of n elements into ascending order
template <class PixType>
#if defined(_MSC_VER)
// Some compilation systems don't like "inline" here, given that "inline"
// is not specified at the declaration in "umc_h264_npf.h", and thus not seen
// when Sort is called within "npf_c.cpp".
// But, MSVC doesn't seem to mind.
inline
#endif
void CNoiseReductionFilter<PixType>::Sort(PixType* v, const Ipp32u n)
{
    Ipp32u i;
    Ipp32s j;
    PixType u;

    for (i = 1; i < n; i ++)
    {
        u = v[i];
        for (j = i; j > 0 && u > v[j - 1]; j --)
            v[j] = v[j - 1];
        v[j] = u;
    }

} // Sort


//----------------------------------------------------------------------------
// Compute a weighted average on given inputs
//     0.625 * y + 0.375 * x    if |x - y| < thres1
// z = 0.5   * y + 0.5   * x    else if |x - y| < thres2
//     0.375 * y + 0.625 * x    else if |x - y| < thres3
//     x                        otherwise

template <class PixType>
#if defined(_MSC_VER)
// Some compilation systems don't like "inline" here, given that "inline"
// is not specified at the declaration in "umc_h264_npf.h", and thus not seen
// when WeightedAvg is called within "npf_c.cpp".
// But, MSVC doesn't seem to mind.
inline
#endif
Ipp32u CNoiseReductionFilter<PixType>::WeightedAvg(register Ipp32u x, register Ipp32u y)
{
    return Ipp32u(x + m_pTFtaps[255 + y - x]); // 16 bits????

} // WeightedAvg


//----------------------------------------------------------------------------
// Perform spatio-temporal filtering on a given plane
// x        : pointer to input buffer
// y        : pointer to output buffer
// yp       : pointer to previous output buffer
// width    : width of color plane
// height   : height of color plane
//
// Notes    : pitch >= width
//            width must be 2x and >= 4
//            height must be 2x and >= 4

#define FILTERING(offs)                                 \
    if (Ipp32u(labs(y[offs] - yp[offs])) >= m_mf_thres_t)   \
        y[offs] = MedianFilter(y, pitchPixels, offs);

template <class PixType>
void CNoiseReductionFilter<PixType>::SpatioTemporalFilter(
    PixType*   y,
    PixType*   yp,
    const Ipp32s  pitchPixels,
    const Ipp32u  width,
    const Ipp32u  height)
{

    // Spatial median filtering
    // Filter second row thru second last row, two rows at a time
    Ipp32u i, j, k;
    Ipp32u dpitch = (pitchPixels - width) + pitchPixels;
    for (k = pitchPixels, i = 1; i < height - 1; i += 2, k += dpitch)
    {
        // Col 1 thru 2: no median filtering on column 0
        FILTERING(k+1)
        FILTERING(k+1+pitchPixels)
        k += 2;

        // Col 3 thru Col width-2
        for (j = k + width - 4; k < j; k += 2)
        {
            FILTERING(k)
            FILTERING(k+1)
            FILTERING(k+pitchPixels)
            FILTERING(k+1+pitchPixels)
        }

        // Last two columns: no median filtering on last column
        FILTERING(k)
        FILTERING(k+pitchPixels)
        k += 2;
    }

    // If this is the first frame, clear flag and skip temporal filtering
    if (m_bFirstFrame_MF)
    {
        m_bFirstFrame_MF = false;
        return;
    }

    // Temporal filtering
    for (i = 0; i < height; i ++, y += pitchPixels, yp += pitchPixels)
    {
        for (j = 0; j < width; j += 4)
        {
            y[j + 0] = PixType(WeightedAvg(y[j + 0], yp[j + 0]));
            y[j + 1] = PixType(WeightedAvg(y[j + 1], yp[j + 1]));
            y[j + 2] = PixType(WeightedAvg(y[j + 2], yp[j + 2]));
            y[j + 3] = PixType(WeightedAvg(y[j + 3], yp[j + 3]));
        }
    }

} // SpatioTemporalFilter


//----------------------------------------------------------------------------
// Find the median of a given point from it neighbors
// x        : pointer to input buffer
// y        : pointer to output buffer
// width    : width of color plane
// height   : height of color plane
// k        : index to point of interest

template <class PixType>
PixType CNoiseReductionFilter<PixType>::MedianFilter(
    PixType*     y,
    const Ipp32s pitchPixels,
    const Ipp32s k)
{
    Ipp32s m, n;
    PixType s[9];

    // Initialize the sort buffer with given input, x[k], and 0's
    s[0] = y[k];
    s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = s[7] = s[8] = 0;

    m = 1;

    // Check to include neighbors x[k + 1], x[k + width]
    // Neighbors x[k - 1] and x[k - width] are substituted by the output
    // y[k - 1] and y[k - width]
    // Also, outliers that differ from x[k] by more than the threshold
    // m_mfo.thres_s are excluded
    if ((Ipp32u) labs(y[k] - y[k - 1]) < m_mf_thres_s)
        s[m ++] = y[k - 1];
    if ((Ipp32u) labs(y[k] - y[k + 1]) < m_mf_thres_s)
        s[m ++] = y[k + 1];
    if ((Ipp32u) labs(y[k] - y[k - pitchPixels]) < m_mf_thres_s)
        s[m ++] = y[k - pitchPixels];
    if ((Ipp32u) labs(y[k] - y[k + pitchPixels]) < m_mf_thres_s)
        s[m ++] = y[k + pitchPixels];

    // Check to include diagonal neighbors x[k + width - 1], x[k + width + 1]
    // Neighbors x[k - width - 1] and x[k - width + 1] are substituted by the
    // output y[k - width - 1] and y[k - width + 1]
    // Again excluded outliers
    if (!m_options.mf_simple)
    {
        if ((Ipp32u) labs(y[k] - y[k - pitchPixels - 1]) < m_mf_thres_s)
            s[m ++] = y[k - pitchPixels - 1];
        if ((Ipp32u) labs(y[k] - y[k - pitchPixels + 1]) < m_mf_thres_s)
            s[m ++] = y[k - pitchPixels + 1];
        if ((Ipp32u) labs(y[k] - y[k + pitchPixels - 1]) < m_mf_thres_s)
            s[m ++] = y[k + pitchPixels - 1];
        if ((Ipp32u) labs(y[k] - y[k + pitchPixels + 1]) < m_mf_thres_s)
            s[m ++] = y[k + pitchPixels + 1];
    }

    // Now sort
    if (m_options.mf_simple)
        Sort5(s);
    else
        Sort(s, m);

    // Get the median and return
    n = m >> 1;
    return((m & 1) ? s[n] : ((s[n] + s[n - 1]) >> 1));

} // MedianFilter


//----------------------------------------------------------------------------
// Detect and remove noise along the edges.  Apply only to the top and
// left edges.  A maximum of two lines can be replaced.  Bad lines
// detected are simply replaced with the next valid neighbor.
//
// Notes: pitch >= width
//        width must be 2x
//        height must be 2x

#define REASONABLE_LUMA 75
#define AMOUNT_DARKER_THRESHOLD 5

template <class PixType>
void CNoiseReductionFilter<PixType>::RemoveNoisyEdges(
    PixType*          y,
    PixType*          u,
    PixType*          v,
    const Ipp32s y_pitch_pixels,
    const Ipp32s uv_pitch_pixels,
    const Ipp32u width,
    const Ipp32u height)
{

    PixType* y0, *y1, *y2, *y3;
    Ipp32u d1, d2, da, db, dc;
    Ipp32u v1, v2;
    Ipp32u i, j, k;
    Ipp32u thres1, thres2;

    Ipp32u halfw = (width >> 1);        // 0.5 * width
    Ipp32u halfh = (height >> 1);       // 0.5 * height

    Ipp32u thres_luma = REASONABLE_LUMA;
    Ipp32u thres_dark = AMOUNT_DARKER_THRESHOLD;

    if (!m_b7BitPel)
    {
        thres_luma <<= 1;
        thres_dark <<= 1;
    }

    // TOP: Detect and replace junk on the first/second rows
    // Note: Junk at the top is often closed-captioning info that requires some
    //       different detection code than that to find junk on other sides

    if (m_uBadEdge_Top > 0)
    {
        thres1 = width * 5;
        thres2 = width * 2;

        if (!m_b7BitPel)
        {
            thres1 <<= 1;
            thres2 <<= 1;
        }

        y1 = y  + y_pitch_pixels;
        y2 = y1 + y_pitch_pixels;
        y3 = y2 + y_pitch_pixels;

        for (i = 0, da = db = dc = v1 = v2 = 0; i < width; i += 2)
        {
            da += labs(y[i]  - y1[i]) + labs(y[i+1]  - y1[i+1]);
            db += labs(y2[i] - y1[i]) + labs(y2[i+1] - y1[i+1]);
            dc += labs(y3[i] - y2[i]) + labs(y3[i+1] - y2[i+1]);
            v1 += y[i]  + y[i+1];
            v2 += y2[i] + y2[i+1];
        }
        d1 = labs(da - db);
        d2 = labs(dc - db);

        // If...
        //  (1) difference between the gradient across all lines is small
        //      (smooth at the top) and...
        //  (2) the very top line is brighter than the third line and...
        //  (3) the third line is reasonable bright then
        // ... then we can be sure that there is no junk at the top border.
        if ((d1 < thres2) && (d2 < thres2) && (v1 > v2) &&
            (v2 > (width * thres_luma)))
        {
            m_uBadEdge_Top = 0;
        }
        // If there is a sufficient gradient to think we need 2 lines
        // replicated then...
        else
        if ((m_uBadEdge_Top == 2) && (d2 >= thres2))
        {
            // ...replace 1st and 2nd rows with the 3rd row
            memcpy(y,  y2, sizeof(PixType)*width);
            memcpy(y1, y2, sizeof(PixType)*width);
            memcpy(u, u + uv_pitch_pixels, sizeof(PixType)*halfw);
            memcpy(v, v + uv_pitch_pixels, sizeof(PixType)*halfw);
            // m_uBadEdge_Top = 2;
        }
        else
        {
            // ... else replace 1st row with the 2nd row
            memcpy(y, y1, sizeof(PixType)*width);
            memcpy(u, u + uv_pitch_pixels, sizeof(PixType)*halfw);
            memcpy(v, v + uv_pitch_pixels, sizeof(PixType)*halfw);
            m_uBadEdge_Top = 1;
        }
    }

    // LEFT: Detect and replace junk on the first/second columns

    if (m_uBadEdge_Left > 0)
    {
        thres1 = height * 10;
        thres2 = height * 2;

        if (!m_b7BitPel)
        {
            thres1 <<= 1;
            thres2 <<= 1;
        }

        k = halfh * y_pitch_pixels;    // 0.5 * height * pitch

        for (i = 0, da = db = dc = v1 = v2 = 0; i < k; i += y_pitch_pixels)
        {
            da += labs(y[i]   - y[i+1]) + labs(y[k+i]   - y[k+i+1]);
            db += labs(y[i+1] - y[i+2]) + labs(y[k+i+1] - y[k+i+2]);
            dc += labs(y[i+3] - y[i+2]) + labs(y[k+i+3] - y[k+i+2]);
            v2 += y[i+1] + y[k+i+1];
            // If the pels are all reasonably bright and there is not a
            // significantly darker pel along the border, this indicates
            // that the is no junk here.
            if ((y[i] > thres_luma) && ((y[i]+thres_dark) > y[i+2]))
                v1 ++;
            if ((y[k+i] > thres_luma) && ((y[k+i]+thres_dark) > y[k+i+2]))
                v1 ++;
        }

        // if the number of pels indicating that there is no black line is
        // more than 1/8 the border size, we can be sure there is no junk here
        if (v1 > (height >> 3))
        {
            m_uBadEdge_Left = 0;
        }
        else
        {
            d1 = labs(da - db);
            d2 = labs(dc - db);

            // If there is sufficient gradient to think we need 2 lines
            // replicated then...
            if ((m_uBadEdge_Left == 2) && (d2 >= thres2) && (d1 < thres1))
            {
                // ...replace 1st and 2nd columns with the 3rd column
                for (i = j = 0; i < k; i += y_pitch_pixels, j += uv_pitch_pixels)
                {
                    y[i]        = y[i+2];
                    y[i+1]      = y[i+2];
                    y[k+i]      = y[k+i+2];
                    y[k+i+1]    = y[k+i+2];
                    u[j]        = u[j+1];
                    v[j]        = v[j+1];
                }
                // m_uBadEdge_Left = 2;
            }
            else
            {
                // ...else replace 1st column with the 2nd column
                for (i = j = 0; i < k; i += y_pitch_pixels, j += uv_pitch_pixels)
                {
                    y[i]    = y[i+1];
                    y[k+i]  = y[k+i+1];
                    u[j]    = u[j+1];
                    v[j]    = v[j+1];
                }
                if (v2 > (height * thres_luma))
                    m_uBadEdge_Left = 1;
            }
        }
    }

    // BOTTOM: Detect and replace junk on the bottom two rows

    if (m_uBadEdge_Bottom > 0)
    {
        thres1 = width * 10;
        thres2 = width * 2;

        if (!m_b7BitPel)
        {
            thres1 <<= 1;
            thres2 <<= 1;
        }

        y0 = y + y_pitch_pixels * (height - 1);
        y1 = y0 - y_pitch_pixels;
        y2 = y1 - y_pitch_pixels;
        y3 = y2 - y_pitch_pixels;

        for (i = 0, da = db = dc = v1 = v2 = 0; i < width; i += 2)
        {
            da += labs(y0[i] - y1[i]) + labs(y0[i+1] - y1[i+1]);
            db += labs(y1[i] - y2[i]) + labs(y1[i+1] - y2[i+1]);
            dc += labs(y3[i] - y2[i]) + labs(y3[i+1] - y2[i+1]);
            v2 += y1[i] + y1[i+1];
            // If the pels are all reasonably bright and there is not a
            // significantly darker pel along the border, this indicates
            // that the is no junk here.
            if ((y0[i] > thres_luma) && ((y0[i]+thres_dark) > y2[i]))
                v1 ++;
            if ((y0[i+1] > thres_luma) && ((y0[i+1]+thres_dark) > y2[i+1]))
                v1 ++;
        }

        if (v1 > (width >> 3))
        {
            m_uBadEdge_Bottom = 0;
        }
        else
        {
            d1 = labs(da - db);
            d2 = labs(dc - db);
            k  = uv_pitch_pixels * (halfh - 1);

            // If there is sufficient gradient to think we need 2 lines
            // replicated then...
            if ((m_uBadEdge_Bottom == 2) && (d2 >= thres2) && (d1 < thres1))
            {
                // ...replace 1st and 2nd rows with the 3rd row
                memcpy(y0, y2, sizeof(PixType)*width);
                memcpy(y1, y2, sizeof(PixType)*width);
                memcpy(u + k, u + k - uv_pitch_pixels, sizeof(PixType)*halfw);
                memcpy(v + k, v + k - uv_pitch_pixels, sizeof(PixType)*halfw);
                // m_uBadEdge_Bottom = 2;
            }
            else
            {
                // ...else replace 1st row with the 2nd row
                memcpy(y0, y1, sizeof(PixType)*width);
                memcpy(u + k, u + k - uv_pitch_pixels, sizeof(PixType)*halfw);
                memcpy(v + k, v + k - uv_pitch_pixels, sizeof(PixType)*halfw);
                if (v2 > (width * thres_luma))
                    m_uBadEdge_Bottom = 1;
            }
        }
    }

} // RemoveNoisyEdges


//----------------------------------------------------------------------------
// Temporal Filter (RN)

#define NL_CONSTANT 200

static Ipp8u FiltTab[512] = {
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,
101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,
121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,
141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
161,162,163,164,165,166,167,168,169,171,172,173,174,175,176,177,178,179,180,181,
182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,
201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,
220,221,222,223,224,225,226,227,228,229,230,231,233,234,235,236,238,239,241,
242,244,245,246,248,249,250,252,253,254,254,255,255,255,255,255,255,255,
0,
1,1,1,1,1,1,1,2,2,3,4,6,7,8,10,11,12,14,15,17,18,20,21,22,23,25,26,27,28,29,30,
31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
81,82,83,84,85,87,88,89,90,91,92,93,94,95,96,97,98,99,100,
101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,
121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,
141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,
181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,
201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,0

};

template <class PixType>
void CNoiseReductionFilter<PixType>::TemporalFilter(
    PixType*          y,
    PixType*          yp,
    const Ipp32s pitchPixels,
    const Ipp32s width,
    const Ipp32s height)
{
    Ipp32s line,pad;
    register Ipp32s pel;
    register PixType *s = yp;
    register PixType *d = y;

    if (m_bFirstFrame)
        return;

    pad = pitchPixels-width;
    line = height;
    do {
        pel = width/4;
        do {
            register Ipp32u a,b;
            a = *(s + 0);
            b = *(d + 0);
            b -= a;
            *(d + 0) = (PixType) (a + *(FiltTab + 255 + b));

            a = *(s + 1);
            b = *(d + 1);
            b -= a;
            *(d + 1) = (PixType) (a + *(FiltTab + 255 + b));

            a = *(s + 2);
            b = *(d + 2);
            b -= a;
            *(d + 2) = (PixType) (a + *(FiltTab + 255 + b));

            a = *(s + 3);
            b = *(d + 3);
            b -= a;
            *(d + 3) = (PixType) (a + *(FiltTab + 255 + b));

            s += 4;
            d += 4;
        } while (--pel);
        d += pad;
    } while (--line);

} // TemporalFilter

template class CNoiseReductionFilter<Ipp8u>;
#if defined BITDEPTH_9_12
template class CNoiseReductionFilter<Ipp16u>;
#endif // BITDEPTH_9_12

} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER

