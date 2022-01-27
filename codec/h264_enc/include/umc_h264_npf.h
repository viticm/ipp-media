//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2004 - 2008 Intel Corporation. All Rights Reserved.
//

// This file defines the CNoiseReductionFilter class that encapsulates a
// noise reduction spatio-temporal filter.

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#ifndef UMC_H264_NPF_H__
#define UMC_H264_NPF_H__

#include "umc_h264_defs.h"

namespace UMC_H264_ENCODER
{

// Bit masks indicating the operation modes of the noise reduction
// filter.  There are currently two modes available: impulse noise
// reduction (INR), noisy edge removal (NER), and temporal noise
// reduction (TNR).
const Ipp32s H264_Noise_Reduction_INR_Mode = 0x01;
const Ipp32s H264_Noise_Reduction_NER_Mode = 0x02;
const Ipp32s H264_Noise_Reduction_TNR_Mode = 0x04;

// Upper and lower limits on threshold settings
#define H264_Noise_Reduction_Min_Thres      0
#define H264_Noise_Reduction_Max_Thres    256
#define H264_Noise_Reduction_Max_Thres_TF 128

// Structure that contains the noise reduction filter options.
// Note that these options are not currently exposed to end users.
// They are intended for developers, for purposes of tuning the filter.
// Thus we have the freedom to change this interface as needed.
typedef struct
{
    bool    y;              // Filter y plane?
    bool    u;              // Filter u plane?
    bool    v;              // Filter v plane?
    bool    mf_simple;      // Apply simplified median filter?
    Ipp32u  mf_thres_s;     // Median filter spatial diff threshold
    Ipp32u  mf_thres_t;     // Median filter temporal diff threshold
    Ipp32u  tf_thres1;      // Temporal filter threshold 1
    Ipp32u  tf_thres2;      // Temporal filter threshold 2
    Ipp32u  tf_thres3;      // Temporal filter threshold 3
} H264_Noise_Reduction_Options;
// The H264_I_Noise_Reduction interface defines the interface that
// controls the underlying prefilter.  Only this interface, not the
// filter implementation, are exposed to the controlling entity.

template <class PixType>
class H264_I_Noise_Reduction
{
public:

    // Enable or disable the noise reduction filter
    // Set enable to true to enable, false to disable.
    // Return true if successful, else false.
    virtual bool EnableNoiseReduction(const bool enable) = 0;

    // Determine whether noise reduction is on, or on by default.
    virtual bool IsNoiseReductionEnabled() = 0;
    virtual bool IsNoiseReductionEnabledByDefault() = 0;

    // Set the noise reduction filter operation mode by ORing the
    // flags specified above.  For example, to select both the impulse
    // noise reduction and the noisy edge removal, pass in the parameter
    // as (H264_Noise_Reduction_INR_Mode | H264_Noise_Reduction_INR_Mode).
    // Return true if successful, else false.
    virtual bool PutNoiseReductionMode(const Ipp32s mode) = 0;

    // Get the current/default noise reduction filter operation mode.
    virtual Ipp32u GetNoiseReductionMode() = 0;
    virtual Ipp32u GetDefaultNoiseReductionMode() = 0;

    // Replace the current noise reduction options with what are given
    // in the parameter opt.
    virtual void PutNoiseReductionOptions(const H264_Noise_Reduction_Options& opt) = 0;

    // Get the current/default noise reduction options into opt.
    virtual void GetNoiseReductionOptions(H264_Noise_Reduction_Options& opt) = 0;
    virtual void GetDefaultNoiseReductionOptions(H264_Noise_Reduction_Options& opt) = 0;

    // Perform noise reduction filtering
    // Return true if succeeded, false if failed
    // x: pointer to the input buffer (IYUV/I420 format)
    // y: pointer to the output buffer (IYUV/I420 format)
    // w: picture width
    // h: picture height
    virtual bool DoFiltering(PixType* y, PixType* u, PixType * v,
                             Ipp32s y_pitch, Ipp32s uv_pitch,
                             Ipp32s width, Ipp32s height) = 0;

    // Start_Sequence informs the filter that a new video sequence is
    // about to be filtered.  It thus forgets about any state it may
    // have maintained.
    virtual bool Start_Sequence(const Ipp32s width, const Ipp32s height) = 0;
};

#define H264_DECLARE_H264_I_Noise_Reduction \
    virtual bool   EnableNoiseReduction(const bool); \
    virtual bool   IsNoiseReductionEnabled(); \
    virtual bool   IsNoiseReductionEnabledByDefault(); \
    virtual bool   PutNoiseReductionMode(const Ipp32s mode); \
    virtual Ipp32u GetNoiseReductionMode(); \
    virtual Ipp32u GetDefaultNoiseReductionMode(); \
    virtual void   GetNoiseReductionOptions(H264_Noise_Reduction_Options&); \
    virtual void   PutNoiseReductionOptions(const H264_Noise_Reduction_Options&); \
    virtual void   GetDefaultNoiseReductionOptions(H264_Noise_Reduction_Options&); \
    virtual bool   DoFiltering(PixType* y, PixType* u, PixType* v, Ipp32s y_pitch, Ipp32s uv_pitch, Ipp32s width, Ipp32s height); \
    virtual bool   Start_Sequence(const Ipp32s, const Ipp32s)

template <class PixType>
// Noise reduction filter class
class CNoiseReductionFilter : public H264_I_Noise_Reduction<PixType>
{

public:

    // Constructor
    CNoiseReductionFilter(const bool bIsMMX,
                          const bool bIsKNI,
                          const bool bIs7BitPel);

    // Destructor
    virtual ~CNoiseReductionFilter();

    // Support H264_I_Noise_Reduction interface
    H264_DECLARE_H264_I_Noise_Reduction;

private:

    // Update data used by the MMX routine with the current options
    void UpdateAndScaleThresholds();

    // Check local buffer
    bool CheckBuffer(const Ipp32s y_pitch_pixels,
                     const Ipp32s uv_pitch_pixels,
                     const Ipp32s width,
                     const Ipp32s height);

    // Calls setup to filter a single plane
    void FilterPlane(PixType*     x,
                     PixType*     xp,
                     const Ipp32s pitchPixels,
                     const Ipp32s width,
                     const Ipp32s height);

    // Perform spatio-temporal filtering
    void SpatioTemporalFilter(PixType*     y,
                              PixType*     yp,
                              const Ipp32s pitchPixels,
                              const Ipp32u width,
                              const Ipp32u height);

    // Perform median filtering
    PixType MedianFilter(PixType*     y,
                         const Ipp32s pitchPixels,
                         const Ipp32s k);

    // Sort 5 points
    static void Sort5(PixType* v);

    // General sorting
    static void Sort(PixType*  v, const Ipp32u n);

    // Perform weighted average temporal filtering
    Ipp32u WeightedAvg(register Ipp32u x,
                       register Ipp32u y);

    // Detect and replace noisy edges along the borders
    void RemoveNoisyEdges(PixType*     y,
                          PixType*     u,
                          PixType*     v,
                          const Ipp32s y_pitch_pixels,
                          const Ipp32s uv_pitch_pixels,
                          const Ipp32u width,
                          const Ipp32u height);

    // Perform temporal filtering
    void TemporalFilter(PixType*     y,
                        PixType*     yp,
                        const Ipp32s pitchPixels,
                        const Ipp32s width,
                        const Ipp32s height);

    // Variables
    H264_Noise_Reduction_Options m_options;  // Filter options

    bool     m_bEnabled;         // Filter enabled?
    bool     m_b7BitPel;         // 7-bit pel flag
    bool     m_bFirstFrame;      // First frame flag
    bool     m_bFirstFrame_MF;   // Median filter first frame flag
    bool     m_bIsMMX;           // MMX flag

    Ipp32s   m_uMode;            // Filter mode (bitmask)
    Ipp32s   m_uBadEdge_Top;     // Number of bad rows on top
    Ipp32s   m_uBadEdge_Left;    // Number of bad cols to the left
    Ipp32s   m_uBadEdge_Bottom;  // Number of bad rows at the bottom

    PixType* m_pLocalBuffer;     // Pointer to pre-aligned buffer
    Ipp32u   m_uLocalBufferSize; // Buffer size

    Ipp8s*   m_pTFtaps;          // Pointer to temporal filter taps
    Ipp8s*   m_pTFtapsBuffer;    // Pointer to pre-aligned filter taps

    PixType* m_pLastFrame;       // Pointer to previous frame buffer
    PixType* m_pLastFrameY;      // Pointer to previous frame Y buffer
    PixType* m_pLastFrameU;      // Pointer to previous frame U buffer
    PixType* m_pLastFrameV;      // Pointer to previous frame V buffer

    // Scaled thresholds
    Ipp32u   m_mf_thres_s;       // Median flt spatial diff threshold
    Ipp32u   m_mf_thres_t;       // Median flt temporal diff threshold
    Ipp32u   m_tf_thres1;        // Temporal flt threshold 1
    Ipp32u   m_tf_thres2;        // Temporal flt threshold 2
    Ipp32u   m_tf_thres3;        // Temporal flt threshold 3

}; // CNoiseReductionFilter

} //namespace UMC_H264_ENCODER

#endif // UMC_H264_NPF_H__

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
