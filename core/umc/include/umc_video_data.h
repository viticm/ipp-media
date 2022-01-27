/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __UMC_VIDEO_DATA_H__
#define __UMC_VIDEO_DATA_H__

#include "ippdefs.h"
#include "umc_structures.h"
#include "umc_media_data.h"

/*
    USAGE MODEL:
    I. Initialization of VideoData parameters. It has to be done after construction
      or after Close.
      A. Simplest case. No additional planes, planes have equal bitdepth.
        1. Set required alignment for data with SetAlignment. Default is the sample size.
        2. Init(w,h,ColorFormat[,bitdepth]). Default bitdepth is derived from format.
      B. Advanced case
        1. Init(w,h,nplanes[,bitdepth]) and SetAlignment if required before or after.
        2. Modify bitdepth or sample size for planes where necessary
        3. Call SetColorFormat. It is the only moment to call this method.
      This stage fill all internal information about planes, including pitches.
      After this no more changes to parameters shall be introduced.
      Function GetMappingSize can be called now to determine required quantity of memory
      for all planes taking into account current alignment. All other Get methods except
      of GetPlanePointer are possible to use.

    II. Link to memory. These operations assign all plane pointers. After that
      MediaData::GetDataPointer will return aligned beginning of the first plane,
      MediaData::GetDataSize will return value equal to MappingSize,
      MediaData::GetBufferPointer can differ from DataPointer due to aligning
      Two ways:
      A. Allocation using Alloc. BufferSize will be MappingSize + alignment.
      B. Call SetBufferPointer(bufsize). After that BufferSize will be bufsize.
      Method ReleaseImage cancels this operations (unlink from memory).
      These methods only work with continuously located planes.

    III. Operations which don't change plane parameters, like SetFrameType, can be used at
      any moment. Operations SetPlanePointer and SetPlanePitch allow working with separate
      planes or without specified ColorFormat but have to be used with care. Functions like
      GetMappingSize and GetPlaneInfo can provide incorrect results.

    Note:
    parent class methods GetDataPointer, MoveDataPointer operator= shouldn't be used.
*/

namespace UMC
{

enum PictureStructure
{
    PS_TOP_FIELD                = 1,
    PS_BOTTOM_FIELD             = 2,
    PS_FRAME                    = PS_TOP_FIELD | PS_BOTTOM_FIELD,
    PS_TOP_FIELD_FIRST          = PS_FRAME | 4,
    PS_BOTTOM_FIELD_FIRST       = PS_FRAME | 8
};

struct LVASurface
{
    Ipp32s index;
    void*  data;
};

// converts display aspect ratio to pixel AR
// or vise versa with exchanged width and height
Status DARtoPAR(Ipp32s width, Ipp32s height, Ipp32s dar_h, Ipp32s dar_v,
                Ipp32s *par_h, Ipp32s *par_v);

class VideoData : public MediaData
{
  DYNAMIC_CAST_DECL(VideoData, MediaData)

public:
    struct PlaneInfo
    {
        Ipp8u*   m_pPlane;         // pointer to plane data
        IppiSize m_ippSize;        // width and height of the plane
        Ipp32s   m_iSampleSize;    // sample size (in bytes)
        Ipp32s   m_iSamples;       // number of samples per plane element
        Ipp32s   m_iBitDepth;      // number of significant bits per sample (should be <= 8*m_iSampleSize)
        size_t   m_nPitch;         // plane pitch (should be >= width*m_iSamples*m_iSampleSize)
        size_t   m_nOffset;        // Offset from the beginning of aligned memory block
        size_t   m_nMemSize;       // size of occupied memory (pitch*height)
        Ipp32s   m_iWidthDiv;      // Horizontal downsampling factor
        Ipp32s   m_iHeightDiv;     // Vertical downsampling factor
    };

    // Default constructor
    VideoData(void);
    // Destructor
    virtual
    ~VideoData(void);

    // operator=
    void operator=(VideoData &par);

    // Initialize. Only remembers image characteristics for future.
    virtual
    Status Init(Ipp32s iWidth,
                Ipp32s iHeight,
                ColorFormat cFormat,
                Ipp32s iBitDepth = 0);

    // Initialize. Only remembers image characteristics for future.
    // Should be followed by SetColorFormat
    virtual
    Status Init(Ipp32s iWidth,
                Ipp32s iHeight,
                Ipp32s iPlanes,
                Ipp32s iBitDepth = 8);

    // Allocate buffer for video data and initialize it.
    virtual
    Status Alloc();

    // Reset all plane pointers, release memory if allocated by Alloc
    virtual
    Status ReleaseImage(void);

    // Release video data and all internal memory. Inherited.
    virtual
    Status Close(void);

    // Set buffer pointer, assign all pointers. Inherited.
    // VideoData parameters must have been prepared
    virtual
    Status SetBufferPointer(Ipp8u *pbBuffer, size_t nSize);

    // Set common Alignment
    Status SetAlignment(Ipp32s iAlignment);
    // Get Alignment
    inline
    Ipp32s GetAlignment(void);

    // Set plane destination pointer
    Status SetPlanePointer(void *pDest, Ipp32s iPlaneNumber);
    // Get plane destination pointer
    inline
      void *GetPlanePointer(Ipp32s iPlaneNumber);

    // Set plane pitch
    Status SetPlanePitch(size_t nPitch, Ipp32s iPlaneNumber);
    // Get plane pitch
    inline
    size_t GetPlanePitch(Ipp32s iPlaneNumber);

    // Set plane bitdepth
    Status SetPlaneBitDepth(Ipp32s iBitDepth, Ipp32s iPlaneNumber);
    // Get plane bitdepth
    inline
    Ipp32s GetPlaneBitDepth(Ipp32s iPlaneNumber);

    // Set plane sample size
    Status SetPlaneSampleSize(Ipp32s iSampleSize, Ipp32s iPlaneNumber);
    // Get plane sample size
    inline
    Ipp32s GetPlaneSampleSize(Ipp32s iPlaneNumber);

    // Set color format and planes' information
    Status SetColorFormat(ColorFormat cFormat);
    // Get color format
    inline
    ColorFormat GetColorFormat(void);

    // Set aspect Ratio
    inline
    Status SetAspectRatio(Ipp32s iHorzAspect, Ipp32s iVertAspect);
    // Get aspect Ratio
    inline
    Status GetAspectRatio(Ipp32s *piHorzAspect, Ipp32s *piVertAspect);

    // Set picture structure
    inline
    Status SetPictureStructure(PictureStructure picStructure);
    // Get picture structure
    inline
    PictureStructure GetPictureStructure(void);
    // Convert to other picture structure
    Status ConvertPictureStructure(PictureStructure newPicStructure);

    inline
    Ipp32s GetNumPlanes(void);
    inline
    Ipp32s GetWidth(void);
    inline
    Ipp32s GetHeight(void);

    // fills PlaneInfo structure
    Status GetPlaneInfo(PlaneInfo* pInfo, Ipp32s iPlaneNumber);

    // Returns the needed size of a buffer for mapping.
    virtual
    size_t GetMappingSize();

    // links plane pointers to surface using provided pitch
    // all pitches and plane info are updated according to current
    // color format.
    // Works only with FourCC formats, which define planes location,
    // like YV12 or NV12.
    virtual
    Status SetSurface(void* ptr, size_t nPitch);

    // Calculate pitch from mapping size
    virtual
    size_t GetPitchFromMappingSize(size_t mappingSize);

    // Crop
    virtual Status Crop(UMC::RECT CropArea);

protected:

    PlaneInfo*       m_pPlaneData;    // pointer to allocated planes info

    Ipp32s           m_iPlanes;       // number of planes

    IppiSize         m_ippSize;       // dimension of the image

    ColorFormat      m_ColorFormat;   // color format of image
    PictureStructure m_picStructure;  // variants: progressive frame, top first, bottom first, only top, only bottom

    Ipp32s           m_iHorzAspect;   // aspect ratio: pixel width/height proportion
    Ipp32s           m_iVertAspect;   // default 1,1 - square pixels

    Ipp32s           m_iAlignment;    // default 1
    Ipp8u*           m_pbAllocated;   // pointer to allocated image buffer

};

// Get Alignment
inline
Ipp32s VideoData::GetAlignment(void)
{
  return m_iAlignment;
} // Ipp32s VideoData::GetAlignment(void)

inline
void* VideoData::GetPlanePointer(Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return NULL;

    return m_pPlaneData[iPlaneNumber].m_pPlane;

} // void *VideoData::GetPlanePointer(Ipp32s iPlaneNumber)

inline
Ipp32s VideoData::GetPlaneBitDepth(Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return 0;

  return m_pPlaneData[iPlaneNumber].m_iBitDepth;

} // Ipp32s VideoData::GetPlaneBitDepth(Ipp32s iPlaneNumber)

inline
Ipp32s VideoData::GetPlaneSampleSize(Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return 0;

  return m_pPlaneData[iPlaneNumber].m_iSampleSize;

} // Ipp32s VideoData::GetPlaneSampleSize(Ipp32s iPlaneNumber)

inline
size_t VideoData::GetPlanePitch(Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return 0;

  return m_pPlaneData[iPlaneNumber].m_nPitch;

} // size_t VideoData::GetPlanePitch(Ipp32s iPlaneNumber)

inline
ColorFormat VideoData::GetColorFormat(void)
{
    return m_ColorFormat;

} // ColorFormat VideoData::GetColorFormat(void)

inline
Status VideoData::SetAspectRatio(Ipp32s iHorzAspect, Ipp32s iVertAspect)
{
    if ((1 > iHorzAspect) || (1 > iVertAspect))
        return UMC_ERR_INVALID_STREAM;

    m_iHorzAspect = iHorzAspect;
    m_iVertAspect = iVertAspect;

    return UMC_OK;

} // Status VideoData::SetAspectRatio(Ipp32s iHorzAspect, Ipp32s iVertAspect)

inline
Status VideoData::GetAspectRatio(Ipp32s *piHorzAspect, Ipp32s *piVertAspect)
{
    if ((NULL == piHorzAspect) ||
        (NULL == piVertAspect))
        return UMC_ERR_NULL_PTR;

    *piHorzAspect = m_iHorzAspect;
    *piVertAspect = m_iVertAspect;

    return UMC_OK;

} // Status VideoData::GetAspectRatio(Ipp32s *piHorzAspect, Ipp32s *piVertAspect)

inline
Status VideoData::SetPictureStructure(PictureStructure picStructure)
{
    if ((PS_TOP_FIELD != picStructure) &&
        (PS_BOTTOM_FIELD != picStructure) &&
        (PS_FRAME != picStructure) &&
        (PS_TOP_FIELD_FIRST != picStructure) &&
        (PS_BOTTOM_FIELD_FIRST != picStructure))
        return UMC_ERR_INVALID_STREAM;

    m_picStructure = picStructure;

    return UMC_OK;

} // Status VideoData::SetPictureStructure(PictureStructure picStructure)

inline
PictureStructure VideoData::GetPictureStructure(void)
{
    return m_picStructure;

} // PictureStructure VideoData::GetPictureStructure(void)

inline
Ipp32s VideoData::GetNumPlanes(void)
{
    return m_iPlanes;

} // Ipp32s VideoData::GetNumPlanes(void)

inline
Ipp32s VideoData::GetWidth(void)
{
    return m_ippSize.width;

} // Ipp32s VideoData::GetWidth(void)

inline
Ipp32s VideoData::GetHeight(void)
{
    return m_ippSize.height;

} // Ipp32s VideoData::GetHeight(void)

} // namespace UMC

#endif // __UMC_VIDEO_DATA_H__
