/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef ___UMC_VIDEO_RENDER_H___
#define ___UMC_VIDEO_RENDER_H___

#include <memory.h>
#include "vm_debug.h"
#include "umc_media_receiver.h"
#include "vm_semaphore.h"
#include "umc_structures.h"
#include "umc_dynamic_cast.h"
#include "umc_module_context.h"
#include "umc_video_data.h"

namespace UMC
{

#if defined(_WIN32) || defined(_WIN32_WCE)

void UMCRect2Rect(UMC::RECT& umcRect, ::RECT& rect);
void Rect2UMCRect(::RECT& rect, UMC::RECT& umcRect);

#endif // defined(_WIN32) || defined(_WIN32_WCE)

class VideoRenderParams: public MediaReceiverParams
{
    DYNAMIC_CAST_DECL(VideoRenderParams, MediaReceiverParams)

public:
    // Default constructor
    VideoRenderParams();

    ColorFormat color_format;       // (UMC::ColorFormat) working color format
    ClipInfo    info;               // (UMC::ClipInfo) video size

    // NOTE: next parameter should replace in future 'color_format' and 'clip_info'
    VideoData   out_data_template;  // (UMC::VideoData) image information: color format, video size, etc

    UMC::RECT   disp;               // (UMC::RECT) display position
    UMC::RECT   range;              // (UMC::RECT) range of source video to display
    Ipp32u      lFlags;             // (Ipp32u) video render flag(s)
};

class VideoRender: public MediaReceiver
{
    DYNAMIC_CAST_DECL(VideoRender, MediaReceiver)

public:

    // Destructor
    virtual ~VideoRender()
    {
        Close();
    }

    struct sCaps
    {
        Ipp64f min_stretch;
        Ipp64f max_stretch;
        bool   overlay_support;
        bool   colorkey_support;
    };

    // Initialize the render
    virtual Status Init(MediaReceiverParams *) {return UMC_OK;};

    // VideoRender interface extension above MediaReceiver

    // Peek presentation of next frame, return presentation time
    virtual Status GetRenderFrame(Ipp64f *time) = 0;

    // Rendering the current frame
    virtual Status RenderFrame() = 0;

    // Show the last rendered frame
    virtual Status ShowLastFrame() = 0;

    // Set/Reset full screen playback
    virtual Status SetFullScreen(ModuleContext& rContext, bool full) = 0;

    // Resize the display rectangular
    virtual Status ResizeDisplay(UMC::RECT &disp, UMC::RECT &range) = 0;

    // Show/Hide Surface
    virtual void ShowSurface() = 0;

    virtual void HideSurface() = 0;

    virtual Status PrepareForRePosition()
    {return UMC_OK;};

protected:
    virtual Ipp32s LockSurface(Ipp8u **vidmem) = 0;
    virtual Ipp32s UnlockSurface(Ipp8u**vidmem) = 0;

    VideoData  m_OutDataTemplate;
};

} // namespace UMC

#endif // ___UMC_VIDEO_RENDER_H___
