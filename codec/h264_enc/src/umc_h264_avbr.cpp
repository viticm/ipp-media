/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//      Copyright (c) 2006-2008 Intel Corporation. All Rights Reserved.
//
//  Purpose
//    AVBR frame adaptive bitrate control
*/

#include "umc_defs.h"
#if defined(UMC_ENABLE_H264_VIDEO_ENCODER)

#include <math.h>
#include "umc_h264_avbr.h"
#include "umc_h264_config.h"

namespace UMC_H264_ENCODER {


Status H264_AVBR_Create(
    H264_AVBR* state)
{
    memset(state, 0, sizeof(H264_AVBR));
    return UMC_OK;
}


void H264_AVBR_Destroy(
    H264_AVBR* state)
{
    H264_AVBR_Close(state);
}


void H264_AVBR_Close(
    H264_AVBR* state)
{
    if (state->mIsInit)
    {
#ifdef ALT_RC
        if (state->QPs)
            H264_Free(state->QPs);
        state->QPs = NULL;
#endif //ALT_RC
    }
    state->mIsInit = false;
}


Ipp32s H264_AVBR_GetInitQP(
    H264_AVBR* state,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha)
{
    const Ipp64f x0 = 0, y0 = 1.19, x1 = 1.75, y1 = 1.75;
    Ipp32s fs, fsLuma;

    fsLuma = fPixels;
    fs = fsLuma;
    if (alpha)
        fs += fsLuma;
    if (chromaSampling == 1)
        fs += fsLuma / 2;
    else if (chromaSampling == 2)
        fs += fsLuma;
    else if (chromaSampling == 3)
        fs += fsLuma * 2;
    fs = fs * bitDepth / 8;
    Ipp32s q = (Ipp32s)(1. / 1.2 * pow(10.0, (log10(fs * 2. / 3. * frameRate / bitRate) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);
    h264_Clip(q, 1, state->mQuantMax);
    return q;
}


UMC::Status H264_AVBR_Init(
    H264_AVBR* state,
    Ipp32s qas,
    Ipp32s fas,
    Ipp32s bas,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha)
{
    if (state->mIsInit)
        H264_AVBR_Close(state);
    state->mQuantOffset = 6 * (bitDepth - 8);
    state->mQuantMax = 51 + state->mQuantOffset;
    if (qas <= 0)
        qas = 100;
    if (fas <= 0)
        fas = 30;
    if (bas <= 0)
        bas = 100;
    state->mBitRate = bitRate;
    state->mBitsDesiredTotal = 0;
    state->mBitsDesiredFrame = (Ipp32s)((Ipp64f)state->mBitRate / frameRate);
    Ipp32s q = H264_AVBR_GetInitQP(state, bitRate, frameRate, fPixels, bitDepth, chromaSampling, alpha);
    state->mQuantPrev = state->mQuantI = state->mQuantP = state->mQuantB = q;
    state->mRCfap = fas;
    state->mRCqap = qas;
    state->mRCbap = bas;
    state->mRCq = q;
    state->mRCqa = 1. / (Ipp64f)state->mRCq;
    state->mRCfa = state->mBitsDesiredFrame;
    state->mIsInit = true;
    return UMC::UMC_OK;
}


void H264_AVBR_PostFrame(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s bpfEncoded)
{
    Ipp64f  bo, qs, dq;
    Ipp32s  quant;

    state->mBitsEncodedTotal += bpfEncoded;
    state->mBitsDesiredTotal += state->mBitsDesiredFrame;
    quant = (frameType == INTRAPIC) ? state->mQuantI : (frameType == BPREDPIC) ? state->mQuantB : state->mQuantP;
    //if (frameType == BPREDPIC)
    //    state->mRCqa += (1. / state->mQuantP - state->mRCqa) / state->mRCqap;
    //else
        state->mRCqa += (1. / quant - state->mRCqa) / state->mRCqap;
    h264_Clip(state->mRCqa, 1./state->mQuantMax , 1./1.);
    //if (frameType == INTRAPIC)
    //    bpfEncoded >>= 1;
    if (frameType != INTRAPIC || state->mRCfap < 30)
        state->mRCfa += (bpfEncoded - state->mRCfa) / state->mRCfap;
    state->mQuantB = ((state->mQuantP + state->mQuantPrev) * 563 >> 10) + 1;
    h264_Clip(state->mQuantB, 1, state->mQuantMax);
    //if (frameType == BPREDPIC) return;
    qs = pow(state->mBitsDesiredFrame / state->mRCfa, 2.0);
    dq = state->mRCqa * qs;
    bo = (Ipp64f)((Ipp64s)state->mBitsEncodedTotal - (Ipp64s)state->mBitsDesiredTotal) / state->mRCbap / state->mBitsDesiredFrame;
    h264_Clip(bo, -1.0, 1.0);
    //dq = dq * (1.0 - bo);
    dq = dq + (1./state->mQuantMax - dq) * bo;
    h264_Clip(dq, 1./state->mQuantMax, 1./1.);
    quant = (int) (1. / dq + 0.5);
    //h264_Clip(quant, 1, state->mQuantMax);
    if (quant >= state->mQuantP + 5)
        quant = state->mQuantP + 3;
    else if (quant >= state->mQuantP + 3)
        quant = state->mQuantP + 2;
    else if (quant > state->mQuantP + 1)
        quant = state->mQuantP + 1;
    else if (quant <= state->mQuantP - 5)
        quant = state->mQuantP - 3;
    else if (quant <= state->mQuantP - 3)
        quant = state->mQuantP - 2;
    else if (quant < state->mQuantP - 1)
        quant = state->mQuantP - 1;
    if (frameType != BPREDPIC)
        state->mQuantPrev = state->mQuantP;
    state->mRCq = state->mQuantI = state->mQuantP = quant;
//    state->mQuantI --;
//    h264_ClipL(state->mQuantI, 1);
}

#ifdef ALT_RC
UMC::Status H264_AVBR_Init_AltRC(
    H264_AVBR* state,
    Ipp32s MaxFrame,
    Ipp32s bitRate,
    Ipp64f frameRate,
    Ipp32s fPixels,
    Ipp32s bitDepth,
    Ipp32s chromaSampling,
    Ipp32s alpha,
    Ipp64s gopLen,
    EnumPicCodType* gopTypes,
    Ipp32s BFrameRate)
{

    if (state->mIsInit) {
        H264_AVBR_Close(state);
    }
    state->mQuantOffset = 6 * (bitDepth - 8);
    state->mQuantMax = 51 + state->mQuantOffset;

    state->L = state->bframeRate = BFrameRate;
    state->mBitRate = bitRate;
    state->mFrameRate = frameRate;
    state->mBitsDesiredTotal = 0;
    state->mBitsDesiredFrame = (Ipp32s)((Ipp64f)state->mBitRate / frameRate);
    Ipp32s q = H264_AVBR_GetInitQP(state, bitRate, frameRate, fPixels, bitDepth, chromaSampling, alpha);
    q -= (Ipp32s)(gopLen/15 - 7);
    state->mQuantPrev = state->mQuantI = state->mQuantP = q;
    SetQuantB();

    state->mIsInit = true;
    state->mIsGopSet = false;

    state->BufferSize = 8;
    state->BufferLoBound = 0;
    state->BufferHiBound = 4;

    state->iF = 0;

    sprintf(state->fname, "NNforeman%dyes.txt", state->mBitRate);

    state->RealGOPLen = state->RealPLeft = state->RealBLeft = 0;
    state->MaxBuf = 8*state->mBitsDesiredFrame;
    state->X1 = state->mBitRate;
    state->X2 = 0;
    state->QP1 = state->QP2 = 0;
    state->MADC1 = 1;
    state->MADC2 = 0;
    state->AvgQP = 0;
    state->BufFull = state->MaxBuf / 2.;
    state->TotalFrameNum = 0;
    state->Mx = state->My = state->Mxx = state->Mxy = 0;
    state->par1 = state->par2 = state->par3 = state->par4 = state->par5 = 0;
    state->GOPLen = gopLen;
    state->GopTypes = gopTypes;
    state->QPs = H264_Malloc(MaxFrame*sizeof(Ipp64s));

    return UMC::UMC_OK;
}

void H264_AVBR_PostFrame_AltRC(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s bEncoded,
    Ipp64f mMAD,
    Ipp64s textureBits)
{
    Ipp32s  bpfExpected, quant;
    Ipp64s  bpfEncoded;

    quant = state->mQuantPrev;
    if (state->TotalFrameNum <= 20)
    {
        state->AvgQP = (state->AvgQP*(state->TotalFrameNum-1) + quant)/state->TotalFrameNum;
    }
    else
    {
        state->AvgQP = (state->AvgQP*20 - state->QPs[state->TotalFrameNum-20-1] + quant)/20;
    }
    bpfEncoded = bEncoded - state->mBitsEncodedTotal;
    //printf("%d %d\n", state->iF, bpfEncoded);
    state->mBitsEncodedTotal = bEncoded;
    state->mBitsDesiredTotal += state->mBitsDesiredFrame;
    state->CurMAD = mMAD;
    state->AvgMAD = (state->AvgMAD*(state->TotalFrameNum-1) + state->CurMAD)/state->TotalFrameNum;
    state->Texture = textureBits;
    //update MAD prediction and quadratic model parameters

    if (frameType == PREDPIC)
    {
        if (state->CurMAD == 0)
        {
            state->CurMAD = 0.0001;
        }
        state->Mx += state->PrevMAD;
        state->My += state->CurMAD;
        state->Mxx += state->PrevMAD*state->PrevMAD;
        state->Mxy += state->PrevMAD*state->CurMAD;
        if (state->TotalFrameNum*state->Mxx - state->Mx*state->Mx == 0)
        {
            state->MADC1 = 1;
            state->MADC2 = 0;
        }
        else
        {
            state->MADC1 = (state->TotalFrameNum*state->Mxy - state->Mx*state->My)/(state->TotalFrameNum*state->Mxx - state->Mx*state->Mx);
            state->MADC2 = (state->My - state->MADC1*state->Mx)/(double)state->TotalFrameNum;
        }

        state->par1 += quant*state->Texture/(double)state->CurMAD;
        state->par2 += state->Texture/(double)state->CurMAD;
        state->par3 += 1/(double)quant;
        state->par4 += 1/(double)quant/(double)quant;
        if ((frameType != INTRAPIC))
        {
            state->X2 = (state->TotalFrameNum*state->par2 - state->par3*state->par1)/((double)state->TotalFrameNum*state->par4 - state->par3*state->par3);
            state->X1 = (state->par1 - state->X2/(double)quant)/(double)state->TotalFrameNum;
        }
    }
    //state->BufFull = min(max(0,state->BufFull + bpfEncoded - state->mBitRate/state->mFrameRate),state->MaxBuf);
    state->BufFull = state->BufFull + bpfEncoded - state->mBitRate/state->mFrameRate;

    if (bpfEncoded >= state->mBitsDesiredFrame)
        state->HiZ += (double)bpfEncoded / state->mBitsDesiredFrame;
    else
        state->HiZ -= (double)state->mBitsDesiredFrame / bpfEncoded;

    state->BitsGopRemaining = state->BitsGopRemaining - bpfEncoded;
    state->PrevMAD = state->CurMAD;

    if ((frameType == PREDPIC) && (state->CurPNum == 1))
    {
        state->TBL = (Ipp64s)state->BufFull;
        state->Const = (state->TBL - (double)state->MaxBuf/2)/(double)(state->LastPNum - 1);
    }
    if (frameType == PREDPIC)
    {
            state->WeightP = (double)bpfEncoded*quant/8 + 7.0*state->WeightP/8;
            state->CurSumPQP += quant;
    }
    if (frameType == BPREDPIC)
            state->WeightB = (double)bpfEncoded*quant/1.3636/8 + 7.0*state->WeightB/8;

    state->iF++;

    state->filelog = fopen(state->fname, "a");
    printf("%d %d\n", state->iF, quant);
    fprintf(state->filelog, "%d %lf\n", state->iF, state->BufFull/state->mBitsDesiredFrame);
    fclose(state->filelog);

}

void H264_AVBR_PreFrame(
    H264_AVBR* state,
    EnumPicCodType FrameType)
{
    Ipp32s  quant;
    Ipp32f  alpha, beta;

    quant = state->mQuantPrev;
    state->TotalFrameNum++;

    if (FrameType == INTRAPIC)//GOP begins
    {
        if (state->TotalFrameNum == 1)
        {
            state->PLeft = (state->GOPLen-1)/(state->L+1) + ((state->GOPLen-1)%(state->L+1)>0);
            state->BLeft = state->GOPLen - state->PLeft;
            state->RealGOPLen = 1;
        }
        else
        {
            state->GOPLen = state->RealGOPLen;
            state->PLeft = state->RealPLeft;
            state->BLeft = state->RealBLeft;
        }

        //Old GOP has ended
        if (state->TotalFrameNum != 1)
            state->BufFullLastGOP = (Ipp64s)state->BufFull;
        else
        {
            state->BufFullLastGOP = 0;
            state->CurPNum = state->PLeft;
        }
        state->SumPQP = state->CurSumPQP;
        state->BitsLastGopRemaining = state->BitsGopRemaining;
        state->LastPNum = state->CurPNum;
        //New GOP has begun

        state->CurSumPQP = 0;
        state->CurPNum = 0;
        state->BNum = 0;
        state->WeightP = state->WeightB = 0;
        state->BitsGopRemaining = (Ipp64s)(state->mBitRate * state->GOPLen / state->mFrameRate - (double)state->MaxBuf/2 + state->BufFullLastGOP);
        if (state->L > 0)
        {
            state->g = 0.25;
            state->b = 0.9;
        }
        else
        {
            state->g = 0.75;
            state->b = 0.6;
        }
        if (state->TotalFrameNum != 1)
        {
            quant = (Ipp32s)(state->SumPQP/(double)state->LastPNum - 1 - 8.0*state->BitsLastGopRemaining/state->BitsGopRemaining - (double)state->GOPLen/15);
        }
        else
        {
            quant = state->mQuantP;
            state->QP2 = state->QP1 = quant;
        }
    }
    if (FrameType == PREDPIC)
    {
        if (state->GOPLen != state->RealGOPLen)
        {
            state->RealGOPLen++;
            state->RealPLeft++;
        }
        state->CurPNum++;
        state->BNum = 0;
        state->PLeft--;
        if (state->TotalFrameNum != 2)
        {
            if (state->CurPNum == 1)
            {
                quant = (Ipp32s)(state->SumPQP/(double)state->LastPNum - 1 - 8*state->BitsLastGopRemaining/(double)state->BitsGopRemaining - (double)state->GOPLen/15);
            }
            else
            {
                state->TBL = (Ipp64s)(state->TBL - state->Const + (state->WeightP*(state->L+1)*state->mBitRate)/(state->mFrameRate*(state->WeightP + state->L*state->WeightB)) - state->mBitRate/state->mFrameRate);
                state->f1 = state->mBitRate/state->mFrameRate + state->g*(state->TBL-state->BufFull);
                state->PredMAD = state->MADC1*state->PrevMAD + state->MADC2;
                state->MADRatio = state->PredMAD/state->AvgMAD;
                if (state->MADRatio <= 0.9)
                    beta = (Ipp32f)0.2;
                else if (state->MADRatio <= 1.05)
                    beta = (Ipp32f)(0.7*state->MADRatio);
                else if (state->MADRatio <= 1.3)
                    beta = (Ipp32f)(0.85*state->MADRatio);
                else if (state->MADRatio <= 1.8)
                    beta = (Ipp32f)(0.95*state->MADRatio);
                else
                    beta = (Ipp32f)(1.5);
                //beta=1;
                state->f2 = beta*state->WeightP*state->BitsGopRemaining/(state->WeightP*state->PLeft + state->WeightB*state->BLeft);
                state->PredTexture = (Ipp64s)((1-state->b)*state->f1 + state->b*state->f2);

                quant = (Ipp32s)((state->X1*state->PredMAD + sqrt(state->X1*state->PredMAD*state->X1*state->PredMAD + 4*state->X2*state->PredMAD*state->PredTexture))/2/state->PredTexture);
                //quant = (state->X1*state->MADRatio + sqrt(state->X1*state->MADRatio*state->X1*state->MADRatio + 4*state->X2*state->MADRatio*state->PredTexture))/2/state->PredTexture;

            }
        }
    }
    if (FrameType == BPREDPIC)
    {
        if (state->GOPLen != state->RealGOPLen)
        {
            state->RealGOPLen++;
            state->RealBLeft++;
        }

        state->BLeft--;
        if (state->L == 1)
        {
            if (state->QP1 != state->QP2)
                quant = (Ipp32s)((state->QP1 + state->QP2 + 2)/2);
            else
                quant = (Ipp32s)(state->QP1 + 2);
        }
        else
        {
            if (state->QP2-state->QP1 <= -2*state->L-3)
                alpha = -3;
            else if (state->QP2-state->QP1 == -2*state->L-2)
                alpha = -2;
            else if (state->QP2-state->QP1 == -2*state->L-1)
                alpha = -1;
            else if (state->QP2-state->QP1 == -2*state->L)
                alpha = -0;
            else if (state->QP2-state->QP1 == -2*state->L+1)
                alpha = 1;
            else
                alpha = 2;
            quant = (Ipp32s)(state->QP1 + alpha + max(min((state->QP2-state->QP1)/(state->L-1), 2*(state->BNum-1)), -2*(state->BNum-1)));
        }
        state->BNum++;

        if (quant - state->mQuantPrev >= 2)
            quant = state->mQuantPrev + 2;
        if (quant - state->mQuantPrev <= -2)
            quant = state->mQuantPrev - 2;

        if (state->TotalFrameNum >= 30)
        {
            if (quant - state->AvgQP >= 5)
                quant = (Ipp32s)(state->AvgQP + 5);
            if (quant - state->AvgQP <= -5)
                quant = (Ipp32s)(state->AvgQP - 5);
        }

        if ((state->BufFull > state->BufferHiBound*state->mBitsDesiredFrame) && (state->HiZ > 5))
        {
            quant+=1;
        }
        if ((state->BufFull < state->BufferLoBound*state->mBitsDesiredFrame) && (state->HiZ < -2))
        {
            quant-=3;
        }

        quant = min(max(quant,1),51);
        state->QPs[state->TotalFrameNum-1] = state->mQuantPrev = state->mQuantB = quant;

        //quant = 1;
        //printf ("B QP %d  ", quant);
        return;
    }

    if (quant - state->mQuantPrev >= 2)
        quant = state->mQuantPrev + 2;
    if (quant - state->mQuantPrev <= -2)
        quant = state->mQuantPrev - 2;
    if (state->TotalFrameNum >= 30)
    {
        if (quant - state->AvgQP >= 5)
            quant = (Ipp32s)(state->AvgQP + 5);
        if (quant - state->AvgQP <= -5)
            quant = (Ipp32s)(state->AvgQP - 5);
    }
    if ((state->BufFull > (state->BufferHiBound-1.5)*state->mBitsDesiredFrame) && (state->HiZ > 3))
    {
        quant+=2;
    }
    if ((state->BufFull < (state->BufferLoBound+1.5)*state->mBitsDesiredFrame) && (state->HiZ < -2))
    {
        quant-=3;
    }
    quant = min(max(quant,1),51);

    //quant = 1;

    state->QPs[state->TotalFrameNum-1] = state->mQuantPrev = state->mRCq = state->mQuantI = state->mQuantP = quant;
    state->QP1 = state->QP2;
    state->QP2 = quant;
    //if (FrameType == INTRAPIC)
        //printf ("I QP %d  ", quant);
    //else
        //printf ("P QP %d  ", quant);

}

void H264_AVBR_SetGopLen(
    H264_AVBR* state,
    Ipp32s len)
{
    if(!state->mIsGopSet)
        state->GOPLen = len;
    state->mIsGopSet = true;
}
#endif

Ipp32s H264_AVBR_GetQP(
    H264_AVBR* state,
    EnumPicCodType frameType)
{
    return ((frameType == INTRAPIC) ? state->mQuantI : (frameType == BPREDPIC) ? state->mQuantB : state->mQuantP) - state->mQuantOffset;
}

void H264_AVBR_SetQP(
    H264_AVBR* state,
    EnumPicCodType frameType,
    Ipp32s qp)
{
    if(frameType == BPREDPIC) {
        state->mQuantB = qp + state->mQuantOffset;
        h264_Clip(state->mQuantB, 1, state->mQuantMax);
    } else {
        state->mRCq = qp + state->mQuantOffset;
        h264_Clip(state->mRCq, 1, state->mQuantMax);
        state->mQuantI = state->mQuantP = state->mRCq;
    }
}


} //namespace UMC_H264_ENCODER

#endif //UMC_ENABLE_H264_VIDEO_ENCODER
