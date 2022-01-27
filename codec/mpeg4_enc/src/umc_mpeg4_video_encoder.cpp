/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2003-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)

#include "umc_mpeg4_video_encoder.h"
#include "umc_video_data.h"
#include "vm_debug.h"
#include "vm_time.h"

namespace UMC
{

MPEG4EncoderParams::MPEG4EncoderParams() {
    ippsZero_8u((Ipp8u*)(&m_Param), sizeof(m_Param));
    m_Param.quantIVOP = 4;
    m_Param.quantPVOP = 4;
    m_Param.quantBVOP = 6;
    m_Param.IVOPdist = 300;
    m_Param.PVOPsearchWidth = -15;
    m_Param.PVOPsearchHeight = 15;
    m_Param.BVOPsearchWidthForw = 15;
    m_Param.BVOPsearchHeightForw = 15;
    m_Param.BVOPsearchWidthBack = 15;
    m_Param.BVOPsearchHeightBack = 15;
    m_Param.MEalgorithm = 1;
    m_Param.MEaccuracy = 2;
    m_Param.obmc_disable = 1;
    m_Param.RoundingControl = 1;
    m_Param.SceneChangeThreshold = 45;
    m_Param.bsBuffer = (Ipp8u*)1;
    m_Param.bsBuffSize = 1; // encoder will not allocate buffer
    m_Param.padType = 0; // set 1 for QuickTime(tm) and 2 for DivX (tm) v. >= 5
    m_Param.TimeResolution = 30;
    m_Param.TimeIncrement = 1;
}

MPEG4VideoEncoder::MPEG4VideoEncoder()
{
    m_IsInit = false;
}

MPEG4VideoEncoder::~MPEG4VideoEncoder()
{
    Close();
}

Status MPEG4VideoEncoder::AllocateBuffers()
{
    Status status = UMC_OK;
    int    i;

    // allocate only frame memory, may be extended for whole buffers
    for (i = 0; i < mp4enc->mPlanes; i ++)
        mp4enc->mFrame[i].mid = 0;
    for (i = 0; i < mp4enc->mPlanes; i ++) {
        status = m_pMemoryAllocator->Alloc(&mp4enc->mFrame[i].mid, mp4enc->mLumaPlaneSize + mp4enc->mChromaPlaneSize + mp4enc->mChromaPlaneSize, UMC_ALLOC_PERSISTENT);
        if (status != UMC_OK)
            return status;
    }
    return status;
}

Status MPEG4VideoEncoder::FreeBuffers()
{
    Status status = UMC_OK;
    int    i;

    for (i = 0; i < mp4enc->mPlanes; i ++) {
        if (mp4enc->mFrame[i].mid)
            status = m_pMemoryAllocator->Free(mp4enc->mFrame[i].mid);
        if (status != UMC_OK)
            return status;
    }
    return status;
}

void MPEG4VideoEncoder::LockBuffers()
{
    int    i;

    for (i = 0; i < mp4enc->mPlanes; i ++) {
        mp4enc->mFrame[i].ap = (Ipp8u*)m_pMemoryAllocator->Lock(mp4enc->mFrame[i].mid);
        mp4enc->mFrame[i].pY = mp4enc->mFrame[i].ap + mp4enc->mExpandSizeA + mp4enc->mExpandSizeA * mp4enc->mStepLuma;
        mp4enc->mFrame[i].pU = mp4enc->mFrame[i].ap + mp4enc->mLumaPlaneSize + (mp4enc->mExpandSizeA >> 1) + (mp4enc->mExpandSizeA >> 1) * mp4enc->mStepChroma;
        mp4enc->mFrame[i].pV = mp4enc->mFrame[i].ap + mp4enc->mLumaPlaneSize + mp4enc->mChromaPlaneSize + (mp4enc->mExpandSizeA >> 1) + (mp4enc->mExpandSizeA >> 1) * mp4enc->mStepChroma;
    }
}

Status MPEG4VideoEncoder::UnlockBuffers()
{
    Status status = UMC_OK;
    int    i;

    for (i = 0; i < mp4enc->mPlanes; i ++) {
        status = m_pMemoryAllocator->Unlock(mp4enc->mFrame[i].mid);
        if (status != UMC_OK)
            return status;
    }
    return status;
}

Status MPEG4VideoEncoder::Reset()
{
    return Init(&m_Param);
    //return UMC_ERR_NOT_IMPLEMENTED;
}

Status MPEG4VideoEncoder::SetParams(BaseCodecParams* baseParams)
{
    VideoEncoderParams *bParam = DynamicCast<VideoEncoderParams, BaseCodecParams>(baseParams);

    if (bParam == NULL)
        return UMC_ERR_UNSUPPORTED;
    // only BitRate and FrameRate could be changed
    if (bParam->info.bitrate == m_Param.info.bitrate && bParam->info.framerate == m_Param.info.framerate)
        return UMC_ERR_UNSUPPORTED;
    m_Param.info.bitrate = bParam->info.bitrate;
    m_Param.info.framerate = bParam->info.framerate;
    mp4enc->ResetRC(m_Param.info.bitrate, m_Param.info.framerate);
    return UMC_OK;
    //return UMC_ERR_NOT_IMPLEMENTED;
}

Status MPEG4VideoEncoder::Init(BaseCodecParams* init)
{
    MPEG4EncoderParams *pParam = DynamicCast<MPEG4EncoderParams>(init);
    VideoEncoderParams *vParam = DynamicCast<VideoEncoderParams>(init);

    if (vParam == NULL && pParam == NULL)
        return UMC_ERR_NULL_PTR;
    if (m_IsInit)
        Close();
    mp4enc = new MPEG4_ENC::ippVideoEncoderMPEG4;
    if (!mp4enc)
        return UMC_ERR_ALLOC;
    if (pParam == NULL) {
        // default params are in constructor
        m_Param.m_Param.Width = vParam->info.clip_info.width;
        m_Param.m_Param.Height = vParam->info.clip_info.height;
        if (vParam->info.bitrate <= 0) {
            m_Param.m_Param.RateControl = 0;
            m_Param.m_Param.BitRate = 0;
        } else {
            m_Param.m_Param.RateControl = 1;
            m_Param.m_Param.BitRate = vParam->info.bitrate;
        }
        m_Param.info.framerate = vParam->info.framerate;
        if (vParam->info.framerate > 0 && (vParam->info.framerate == (Ipp32s)vParam->info.framerate)) {
            m_Param.m_Param.TimeResolution = (Ipp32s)vParam->info.framerate;
            m_Param.m_Param.TimeIncrement = 1;
        } else {
            if (vParam->info.framerate >= 23.976 && vParam->info.framerate < 24) {
                m_Param.m_Param.TimeResolution = 24000;
                m_Param.m_Param.TimeIncrement = 1001;
            } else if (vParam->info.framerate >= 29.97 && vParam->info.framerate < 30) {
                m_Param.m_Param.TimeResolution = 30000;
                m_Param.m_Param.TimeIncrement = 1001;
            } else {
                m_Param.m_Param.TimeResolution = 30;
                m_Param.m_Param.TimeIncrement = 1;
                m_Param.info.framerate = 30;
            }
        }
        m_Param.info.clip_info.width = vParam->info.clip_info.width;
        m_Param.info.clip_info.height = vParam->info.clip_info.height;
        //m_Param.numFramesToEncode = vParam->numFramesToEncode;
        m_Param.info.bitrate = vParam->info.bitrate;
    } else {
        m_Param = *pParam;
        // override MPEG-4 params if base params are valid
        if (m_Param.info.clip_info.width && m_Param.info.clip_info.height) {
            m_Param.m_Param.Width = m_Param.info.clip_info.width;
            m_Param.m_Param.Height = m_Param.info.clip_info.height;
        }
        if (m_Param.info.framerate > 0) {
            if (m_Param.info.framerate == (Ipp32s)m_Param.info.framerate) {
                m_Param.m_Param.TimeResolution = (Ipp32s)m_Param.info.framerate;
                m_Param.m_Param.TimeIncrement = 1;
            } else {
                if (m_Param.info.framerate >= 23.976 && m_Param.info.framerate < 24) {
                    m_Param.m_Param.TimeResolution = 24000;
                    m_Param.m_Param.TimeIncrement = 1001;
                } else if (m_Param.info.framerate >= 29.97 && m_Param.info.framerate < 30) {
                    m_Param.m_Param.TimeResolution = 30000;
                    m_Param.m_Param.TimeIncrement = 1001;
                }
            }
        } else
            m_Param.info.framerate = m_Param.m_Param.TimeResolution / m_Param.m_Param.TimeIncrement;
        if (m_Param.info.bitrate > 0) {
            if (m_Param.m_Param.RateControl == 0)
                m_Param.m_Param.RateControl = 1;
            m_Param.m_Param.BitRate = m_Param.info.bitrate;
        }
    }
    m_Param.m_Param.numThreads = vParam->numThreads;
    m_Param.m_Param.profile_and_level = (Ipp8u)((vParam->profile << 4) + (vParam->level & 15));
    m_Param.m_Param.aspect_ratio_width = (Ipp8u)m_Param.info.aspect_ratio_width;
    m_Param.m_Param.aspect_ratio_height = (Ipp8u)m_Param.info.aspect_ratio_height;
    Ipp32s  mp4status = mp4enc->Init(&m_Param.m_Param);
    if (mp4status == MPEG4_ENC::MP4_STS_ERR_PARAM)
        return UMC_ERR_INIT;
    if (mp4status == MPEG4_ENC::MP4_STS_ERR_NOMEM)
        return UMC_ERR_ALLOC;
    m_FrameCount = 0;
    if (m_Param.m_Param.BVOPdist) {
        bTime = new Ipp64f [m_Param.m_Param.BVOPdist];
        if (!bTime)
            return UMC_ERR_ALLOC;
    }
    bTimePos = 0;
    gTime = 0.0;
    iTime = 1.0 / vParam->info.framerate;
    // create default memory allocator if not exist
    Status status = BaseCodec::Init(init);
    if (status != UMC_OK)
        return status;
    status = AllocateBuffers();
    if (status != UMC_OK)
        return status;
    m_Param.m_SuggestedOutputSize = m_Param.info.clip_info.width * m_Param.info.clip_info.height;
    m_IsInit = true;
    return UMC_OK;
}

Status MPEG4VideoEncoder::GetInfo(BaseCodecParams* baseParams)
{
    MPEG4EncoderParams* mp4Params = DynamicCast<MPEG4EncoderParams>(baseParams);
    VideoEncoderParams *encParams = DynamicCast<VideoEncoderParams>(baseParams);

    if (!m_IsInit)
        return UMC_ERR_NOT_INITIALIZED;
    m_Param.info.stream_type = MPEG4_VIDEO;
    m_Param.qualityMeasure = 100 - m_Param.m_Param.quantPVOP * 100 / 33;
    if (mp4Params) {
        *mp4Params = m_Param;
    } else if (encParams) {
        *encParams = m_Param;
    } else if (baseParams) {
        *baseParams = m_Param;
    } else
        return UMC_ERR_NULL_PTR;
    return UMC_OK;
}

Status MPEG4VideoEncoder::Close()
{
    if (!m_IsInit)
        return UMC_ERR_NOT_INITIALIZED;
    FreeBuffers();
    // close default memory allocator if exist
    BaseCodec::Close();
    if (m_Param.m_Param.BVOPdist)
        delete [] bTime;
    mp4enc->Close();
    delete mp4enc;
    m_IsInit = false;
    return UMC_OK;
}

Status MPEG4VideoEncoder::GetFrame(MediaData* pIn, MediaData* pOut)
{
    if (!m_IsInit)
        return UMC_ERR_NOT_INITIALIZED;
    VideoData* pVideoDataIn = DynamicCast<VideoData> (pIn);
    if (!pOut)
        return UMC_ERR_NULL_PTR;
    mp4enc->InitBuffer((Ipp8u*)pOut->GetDataPointer() + pOut->GetDataSize(), (Ipp8u*)pOut->GetBufferPointer() - (Ipp8u*)pOut->GetDataPointer() + pOut->GetBufferSize() - pOut->GetDataSize());
    if (m_FrameCount == 0)
        mp4enc->EncodeHeader();
    LockBuffers();
    if (pIn && ((m_Param.m_Param.sprite_enable != MP4_SPRITE_STATIC) || (m_FrameCount == 0))) {
        // copy YUV to internal frame
        IppiSize  roi;
        Ipp8u    *pY, *pU, *pV;
        Ipp32s    stepL, stepC;

        mp4enc->GetCurrentFrameInfo(&pY, &pU, &pV, &stepL, &stepC);
        roi.width = m_Param.info.clip_info.width;
        roi.height = m_Param.info.clip_info.height;
        ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(0), pVideoDataIn->GetPlanePitch(0), pY, stepL, roi);
        roi.width >>= 1;
        roi.height >>= 1;
        ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(1), pVideoDataIn->GetPlanePitch(1), pU, stepC, roi);
        ippiCopy_8u_C1R((Ipp8u*)pVideoDataIn->GetPlanePointer(2), pVideoDataIn->GetPlanePitch(2), pV, stepC, roi);
    }
    Ipp32s  sts = mp4enc->EncodeFrame(pIn == NULL);
    Ipp64f  pts = gTime;
    if (pIn) {
        pIn->SetDataSize(0);
        if (pIn->GetTime() >= 0)
            pts = pIn->GetTime();
    }
    if (sts == MPEG4_ENC::MP4_STS_BUFFERED) {
        //pOut->SetDataSize(0);
        pOut->SetTime(-1.0);
        if (pIn) {
            bTime[bTimePos] = pts;
            bTimePos ++;
            if (bTimePos >= m_Param.m_Param.BVOPdist)
                bTimePos = 0;
        }
    } else {
        if (sts != MPEG4_ENC::MP4_STS_NODATA)
            m_FrameCount ++;
        pOut->SetDataSize(mp4enc->GetBufferFullness() + pOut->GetDataSize());
        if (mp4enc->GetFrameType() != MPEG4_ENC::MP4_VOP_TYPE_B) {
            pOut->SetTime(pts);
        } else {
            pOut->SetTime(bTime[bTimePos]);
            bTime[bTimePos] = pts;
            bTimePos ++;
            if (bTimePos >= m_Param.m_Param.BVOPdist)
                bTimePos = 0;
        }
        switch (mp4enc->GetFrameType()) {
          case MPEG4_ENC::MP4_VOP_TYPE_I:
            pOut->SetFrameType(I_PICTURE);
            break;
          case MPEG4_ENC::MP4_VOP_TYPE_P:
            pOut->SetFrameType(P_PICTURE);
            break;
          case MPEG4_ENC::MP4_VOP_TYPE_B:
            pOut->SetFrameType(B_PICTURE);
            break;
          default:
            pOut->SetFrameType(NONE_PICTURE);
            break;
        }
    }
    gTime += iTime;
    UnlockBuffers();
    //return (sts == MPEG4_ENC::MP4_STS_BUFFERED) ? UMC_ERR_NOT_ENOUGH_DATA : (sts == MPEG4_ENC::MP4_STS_NODATA) ? UMC_ERR_END_OF_STREAM : UMC_OK;
    return (sts == MPEG4_ENC::MP4_STS_NODATA) ? UMC_ERR_END_OF_STREAM : UMC_OK;
    //return UMC_OK;
}

VideoEncoder* CreateMPEG4Encoder()
{
    MPEG4VideoEncoder* ptr = new MPEG4VideoEncoder;
    return ptr;
}

Status MPEG4EncoderParams::ReadParamFile(const vm_char *FileName)
{
    vm_file *InputFile;
    vm_char str[STR_LEN+1], IntraQMatrixFileName[STR_LEN+1], NonIntraQMatrixFileName[STR_LEN+1];
    Ipp32s   i, j, k, numFramesToEncode;

    InputFile = vm_file_open(FileName, VM_STRING("rt"));
    if (!InputFile) {
        vm_debug_trace1(VM_DEBUG_INFO,__VM_STRING("Error: Couldn't open file '%s'\n"), FileName);
        return UMC_ERR_FAILED;
    }
    vm_file_gets(str, STR_LEN, InputFile);
    vm_file_gets(str, STR_LEN, InputFile); //vm_string_sscanf(str, VM_STRING("%s"), SrcFileName);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Width);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.Height);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &numFramesToEncode);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.TimeResolution);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.TimeIncrement);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quant_type);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantIVOP);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantPVOP);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.quantBVOP);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%s"), IntraQMatrixFileName);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%s"), NonIntraQMatrixFileName);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.short_video_header);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.IVOPdist);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BVOPdist);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PVOPsearchWidth);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.PVOPsearchHeight);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BVOPsearchWidthForw);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BVOPsearchHeightForw);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BVOPsearchWidthBack);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BVOPsearchHeightBack);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.MEalgorithm);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.MEaccuracy);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.ME4mv);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.obmc_disable);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.RoundingControl);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.calcPSNR);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.RateControl);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.BitRate);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.SceneChangeThreshold);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.insertGOV);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.repeatHeaders);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.resync);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.VideoPacketLenght);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.data_partitioned);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.reversible_vlc);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.interlaced);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.top_field_first);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.alternate_vertical_scan_flag);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.interlacedME);
    vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.sprite_enable);
    if (m_Param.sprite_enable) {
        vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.no_of_sprite_warping_points);
        vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.sprite_warping_accuracy);
    }
    if (m_Param.sprite_enable == IPPVC_SPRITE_STATIC) {
        vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d"), &m_Param.sprite_brightness_change);
        vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d %d"), &m_Param.sprite_left_coordinate, &m_Param.sprite_top_coordinate);
        vm_file_gets(str, STR_LEN, InputFile); vm_string_sscanf(str, VM_STRING("%d %d"), &m_Param.sprite_width, &m_Param.sprite_height);
        if (numFramesToEncode < 1)
            numFramesToEncode = 1;
        if (m_Param.no_of_sprite_warping_points < 0 || m_Param.no_of_sprite_warping_points > 3)
            m_Param.no_of_sprite_warping_points = 0;
        if (m_Param.no_of_sprite_warping_points > 0) {
            m_Param.warping_mv_code_du = new Ipp32s [m_Param.no_of_sprite_warping_points * numFramesToEncode];
            m_Param.warping_mv_code_dv = new Ipp32s [m_Param.no_of_sprite_warping_points * numFramesToEncode];
            m_Param.brightness_change_factor = new Ipp32s [numFramesToEncode];
            for (i = 0; i < (Ipp32s)numFramesToEncode; i ++) {
                for (j = 0; j < m_Param.no_of_sprite_warping_points; j ++) {
                    vm_file_fscanf(InputFile, VM_STRING("%d"), &m_Param.warping_mv_code_du[i*m_Param.no_of_sprite_warping_points+j]);
                    vm_file_fscanf(InputFile, VM_STRING("%d"), &m_Param.warping_mv_code_dv[i*m_Param.no_of_sprite_warping_points+j]);
                }
                if (m_Param.sprite_brightness_change)
                    vm_file_fscanf(InputFile, VM_STRING("%d"), &m_Param.brightness_change_factor[i]);
            }
        }
    }
    vm_file_fclose(InputFile);
    // read quant matrix
    m_Param.load_intra_quant_mat = 0;
    m_Param.load_intra_quant_mat_len = 0;
    if (IntraQMatrixFileName[0] != '-' ) {
        InputFile = vm_file_open(IntraQMatrixFileName, VM_STRING("rt"));
        if (!InputFile) {
            vm_debug_trace1(VM_DEBUG_INFO,__VM_STRING("Error: Couldn't open quant matrix file '%s'\n"), IntraQMatrixFileName);
            return UMC_ERR_FAILED;
        } else {
            m_Param.load_intra_quant_mat = 1;
            for (i = 0; i < 64; i++) {
                k = vm_file_fscanf(InputFile, VM_STRING("%d"), &j);
                if (k <= 0 || j < 1 || j > 255 )
                    break;
                m_Param.intra_quant_mat[i] = (Ipp8u)j;
            }
            m_Param.load_intra_quant_mat_len = i;
            if (m_Param.load_intra_quant_mat_len < 2) {
                m_Param.load_intra_quant_mat = 0;
                m_Param.load_intra_quant_mat_len = 0;
            }
        }
        vm_file_fclose(InputFile);
        m_Param.quant_type = 1;
    }
    m_Param.load_nonintra_quant_mat = 0;
    m_Param.load_nonintra_quant_mat_len = 0;
    if (NonIntraQMatrixFileName[0] != '-' ) {
        InputFile = vm_file_open(NonIntraQMatrixFileName, VM_STRING("rt"));
        if (!InputFile) {
            vm_debug_trace1(VM_DEBUG_INFO,__VM_STRING("Error: Couldn't open quant matrix file '%s'\n"), NonIntraQMatrixFileName);
            return UMC_ERR_FAILED;
        } else {
            m_Param.load_nonintra_quant_mat = 1;
            for (i = 0; i < 64; i++) {
                k = vm_file_fscanf(InputFile, VM_STRING("%d"), &j);
                if (k <= 0 || j < 1 || j > 255 )
                    break;
                m_Param.nonintra_quant_mat[i] = (Ipp8u)j;
            }
            m_Param.load_nonintra_quant_mat_len = i;
            if (m_Param.load_nonintra_quant_mat_len < 2) {
                m_Param.load_nonintra_quant_mat = 0;
                m_Param.load_nonintra_quant_mat_len = 0;
            }
        }
        vm_file_fclose(InputFile);
        m_Param.quant_type = 1;
    }
    m_Param.bsBuffer = (Ipp8u*)1;
    m_Param.bsBuffSize = 1; // encoder will not allocate buffer
    m_Param.padType = 0; // set 1 for QuickTime(tm) and 2 for DivX (tm) v. >= 5
    info.clip_info.width = m_Param.Width;
    info.clip_info.height = m_Param.Height;
    if (m_Param.sprite_enable == IPPVC_SPRITE_STATIC) {
        info.clip_info.width = m_Param.sprite_width;
        info.clip_info.height = m_Param.sprite_height;
    }
    info.framerate = (Ipp64f)m_Param.TimeResolution / m_Param.TimeIncrement;
    info.bitrate = (m_Param.RateControl > 0) ? m_Param.BitRate : 0;
    return UMC_OK;
}

} // namespace UMC

#endif //defined (UMC_ENABLE_MPEG4_VIDEO_ENCODER)
