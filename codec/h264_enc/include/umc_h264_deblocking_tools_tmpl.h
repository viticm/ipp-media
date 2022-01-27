/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2003-2008 Intel Corporation. All Rights Reserved.
//
//
*/

#if PIXBITS == 8

#define PIXTYPE Ipp8u
#define COEFFSTYPE Ipp16s
#define H264ENC_MAKE_NAME(NAME) NAME##_8u16s

#elif PIXBITS == 16

#define PIXTYPE Ipp16u
#define COEFFSTYPE Ipp32s
#define H264ENC_MAKE_NAME(NAME) NAME##_16u32s

#elif //PIXBITS

void H264EncoderFakeFunction() { UNSUPPORTED_PIXBITS; }

#endif //PIXBITS

//public:

// Default constructor
Status H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_Create)(
    void* state);

// Destructor
void H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_Destroy)(
    void* state);

// Initialize object
Status H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_Initialize)(
    void* state,
    H264ENC_MAKE_NAME(H264CoreEncoder) *pEncoder);

// Deblock slice by two thread
void H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_DeblockSliceTwoThreaded)(
    Ipp32u uFirstMB,
    Ipp32u unumMBs,
    H264CoreEncoder_DeblockingFunction pDeblocking);

// Deblock slice asynchronous
void H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_WaitEndOfSlice)(
    void* state);

void H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_DeblockSliceAsync)(
    void* state,
    Ipp32u uFirstMB,
    Ipp32u unumMBs,
    H264CoreEncoder_DeblockingFunction pDeblocking);

// Release object
void H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_Release)(
    void* state);

// Additional deblocking thread
Ipp32u VM_THREAD_CALLCONVENTION
H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_DeblockSliceSecondThread)(
    void *p);

Ipp32u VM_THREAD_CALLCONVENTION
H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools_DeblockSliceAsyncSecondThread)(
    void *p);


typedef struct H264ENC_MAKE_NAME(sH264EncoderThreadedDeblockingTools)
{
//public:
    Ipp32u m_nMBAFF;                                            // (Ipp32u) MBAFF flag

    H264ENC_MAKE_NAME(H264CoreEncoder)* m_pDecoder;             // (H264CoreEncoder *) pointer to decoder-owner

    vm_thread m_hDeblockingSliceSecondThread;                   // (vm_thread) handle to deblocking slice second thread
    vm_thread m_hDeblockingSliceAsyncSecondThread;              // (vm_thread) handle to deblocking slice second thread

    vm_event m_hBeginFrame;                                     // (vm_event) event to begin of deblocking frame
    vm_event m_hBeginSlice;                                     // (vm_event) event to begin of deblocking slice
    vm_event m_hBeginRow;                                       // (vm_event) event to begin of deblocking row
    vm_event m_hDoneBorder;                                     // (vm_event) event of border macroblock is complete
    vm_event m_hDoneRow;                                        // (vm_event) event of row is complete
    vm_event m_hDoneSlice;                                      // (vm_event) event of slice is complete

    bool m_bQuit;                                               // (bool) quit flag
    Ipp32u m_nFirstMB;                                          // (Ipp32u) first macroblock to deblock
    Ipp32u m_nNumMB;                                            // (Ipp32u) number of macroblock to deblock
    H264CoreEncoder_DeblockingFunction m_pDeblocking;           // (H264CoreEncoder::DeblockingFunction) pointer to current deblocking function
} H264ENC_MAKE_NAME(H264EncoderThreadedDeblockingTools);

#undef H264ENC_MAKE_NAME
#undef COEFFSTYPE
#undef PIXTYPE
