/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2008 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"

#ifdef UMC_SBR_HUFF_TAB
#undef UMC_SBR_HUFF_TAB
#endif

#if defined (UMC_ENABLE_AAC_AUDIO_DECODER)
#define UMC_SBR_HUFF_TAB
#elif defined UMC_ENABLE_AAC_INT_AUDIO_DECODER
#define UMC_SBR_HUFF_TAB
#elif defined UMC_ENABLE_AAC_AUDIO_ENCODER
#define UMC_SBR_HUFF_TAB
#elif defined UMC_ENABLE_AAC_INT_AUDIO_ENCODER
#define UMC_SBR_HUFF_TAB
#endif

#ifdef UMC_SBR_HUFF_TAB

#include "ippdefs.h"
#include "ippdc.h"
#include "ipps.h"
#include "sbr_settings.h"
#include "sbr_huff_tabs.h"

/********************************************************************/

Ipp32s vlcSbrTableSizes[] = {
  121, 121, 49, 49, 63, 63, 25, 25, 63, 25
};
Ipp32s vlcSbrNumSubTables[] = {
  3, 4, 3, 3, 3, 4, 2, 2, 2, 2
};

/********************************************************************/

static Ipp32s vlcSbrSubTablesSizes0[]  = {6, 6, 7};
static Ipp32s vlcSbrSubTablesSizes1[]  = {5, 5, 5, 5};
static Ipp32s vlcSbrSubTablesSizes2[]  = {6, 6, 5};
static Ipp32s vlcSbrSubTablesSizes3[]  = {6, 6, 7};
static Ipp32s vlcSbrSubTablesSizes4[]  = {6, 6, 7};
static Ipp32s vlcSbrSubTablesSizes5[]  = {5, 5, 5, 5};
static Ipp32s vlcSbrSubTablesSizes6[]  = {7, 7};
static Ipp32s vlcSbrSubTablesSizes7[]  = {7, 7};
static Ipp32s vlcSbrSubTablesSizes8[]  = {7, 7};
static Ipp32s vlcSbrSubTablesSizes9[]  = {4, 4};

/********************************************************************/

Ipp32s *vlcSbrSubTablesSizes[] = {
  vlcSbrSubTablesSizes0, vlcSbrSubTablesSizes1, vlcSbrSubTablesSizes2,
  vlcSbrSubTablesSizes3, vlcSbrSubTablesSizes4, vlcSbrSubTablesSizes5,
  vlcSbrSubTablesSizes6, vlcSbrSubTablesSizes7, vlcSbrSubTablesSizes8,
  vlcSbrSubTablesSizes9
};

/********************************************************************/

// 0
static IppsVLCTable_32s ipp_t_huffman_env_1_5dB[] = {
  {   59, 0x00000001,  2},  {   60, 0x00000000,  2},
  {   58, 0x00000005,  3},  {   61, 0x00000004,  3},
  {   57, 0x0000000d,  4},  {   62, 0x0000000c,  4},
  {   56, 0x0000001d,  5},  {   63, 0x0000001c,  5},
  {   55, 0x0000003d,  6},  {   64, 0x0000003c,  6},
  {   54, 0x0000007d,  7},  {   65, 0x0000007c,  7},
  {   53, 0x000000fd,  8},  {   66, 0x000000fc,  8},
  {   52, 0x000001fd,  9},  {   67, 0x000001fc,  9},
  {   51, 0x000003fc, 10},  {   68, 0x000003fd, 10},
  {   50, 0x000007fc, 11},  {   49, 0x00000ffb, 12},
  {   69, 0x00000ffa, 12},  {   47, 0x00001ffa, 13},
  {   48, 0x00001ff9, 13},  {   70, 0x00001ff8, 13},
  {   45, 0x00003ff9, 14},  {   46, 0x00003ff7, 14},
  {   71, 0x00003ff6, 14},  {   72, 0x00003ff8, 14},
  {   44, 0x00007ff4, 15},  {   73, 0x00007ff5, 15},
  {   36, 0x0000fff0, 16},  {   40, 0x0000fff1, 16},
  {   41, 0x0000ffec, 16},  {   42, 0x0000ffed, 16},
  {   43, 0x0000ffee, 16},  {   74, 0x0000ffef, 16},
  {   76, 0x0000fff2, 16},  {   34, 0x0001ffe6, 17},
  {   37, 0x0001ffe9, 17},  {   39, 0x0001ffe7, 17},
  {   75, 0x0001ffe8, 17},  {    0, 0x0003ffd6, 18},
  {    1, 0x0003ffd7, 18},  {    2, 0x0003ffd8, 18},
  {    3, 0x0003ffd9, 18},  {    4, 0x0003ffda, 18},
  {    5, 0x0003ffdb, 18},  {   35, 0x0003ffd4, 18},
  {   38, 0x0003ffd5, 18},  {    6, 0x0007ffb8, 19},
  {    7, 0x0007ffb9, 19},  {    8, 0x0007ffba, 19},
  {    9, 0x0007ffbb, 19},  {   10, 0x0007ffbc, 19},
  {   11, 0x0007ffbd, 19},  {   12, 0x0007ffbe, 19},
  {   13, 0x0007ffbf, 19},  {   14, 0x0007ffc0, 19},
  {   15, 0x0007ffc1, 19},  {   16, 0x0007ffc2, 19},
  {   17, 0x0007ffc3, 19},  {   18, 0x0007ffc4, 19},
  {   19, 0x0007ffc5, 19},  {   20, 0x0007ffc6, 19},
  {   21, 0x0007ffc7, 19},  {   22, 0x0007ffc8, 19},
  {   23, 0x0007ffc9, 19},  {   24, 0x0007ffca, 19},
  {   25, 0x0007ffcb, 19},  {   26, 0x0007ffcc, 19},
  {   27, 0x0007ffcd, 19},  {   28, 0x0007ffce, 19},
  {   29, 0x0007ffcf, 19},  {   30, 0x0007ffd0, 19},
  {   31, 0x0007ffd1, 19},  {   32, 0x0007ffd2, 19},
  {   33, 0x0007ffd3, 19},  {   77, 0x0007ffd4, 19},
  {   78, 0x0007ffd5, 19},  {   79, 0x0007ffd6, 19},
  {   80, 0x0007ffd7, 19},  {   81, 0x0007ffd8, 19},
  {   82, 0x0007ffd9, 19},  {   83, 0x0007ffda, 19},
  {   84, 0x0007ffdb, 19},  {   85, 0x0007ffdc, 19},
  {   86, 0x0007ffdd, 19},  {   87, 0x0007ffde, 19},
  {   88, 0x0007ffdf, 19},  {   89, 0x0007ffe0, 19},
  {   90, 0x0007ffe1, 19},  {   91, 0x0007ffe2, 19},
  {   92, 0x0007ffe3, 19},  {   93, 0x0007ffe4, 19},
  {   94, 0x0007ffe5, 19},  {   95, 0x0007ffe6, 19},
  {   96, 0x0007ffe7, 19},  {   97, 0x0007ffe8, 19},
  {   98, 0x0007ffe9, 19},  {   99, 0x0007ffea, 19},
  {  100, 0x0007ffeb, 19},  {  101, 0x0007ffec, 19},
  {  102, 0x0007ffed, 19},  {  103, 0x0007ffee, 19},
  {  104, 0x0007ffef, 19},  {  105, 0x0007fff0, 19},
  {  106, 0x0007fff1, 19},  {  107, 0x0007fff2, 19},
  {  108, 0x0007fff3, 19},  {  109, 0x0007fff4, 19},
  {  110, 0x0007fff5, 19},  {  111, 0x0007fff6, 19},
  {  112, 0x0007fff7, 19},  {  113, 0x0007fff8, 19},
  {  114, 0x0007fff9, 19},  {  115, 0x0007fffa, 19},
  {  116, 0x0007fffb, 19},  {  117, 0x0007fffc, 19},
  {  118, 0x0007fffd, 19},  {  119, 0x0007fffe, 19},
  {  120, 0x0007ffff, 19},
};

/********************************************************************/
// 1
static IppsVLCTable_32s ipp_f_huffman_env_1_5dB[] = {
  {   59, 0x00000001,  2},  {   60, 0x00000000,  2},
  {   58, 0x00000005,  3},  {   61, 0x00000004,  3},
  {   57, 0x0000000c,  4},  {   62, 0x0000000d,  4},
  {   56, 0x0000001c,  5},  {   63, 0x0000001d,  5},
  {   55, 0x0000003c,  6},  {   64, 0x0000003d,  6},
  {   54, 0x0000007c,  7},  {   53, 0x000000fb,  8},
  {   65, 0x000000fa,  8},  {   66, 0x000000fc,  8},
  {   51, 0x000001fc,  9},  {   52, 0x000001fa,  9},
  {   67, 0x000001fb,  9},  {   50, 0x000003fb, 10},
  {   68, 0x000003fa, 10},  {   49, 0x000007f9, 11},
  {   69, 0x000007f8, 11},  {   70, 0x000007fa, 11},
  {   71, 0x000007fb, 11},  {   47, 0x00000ffa, 12},
  {   48, 0x00000ff8, 12},  {   72, 0x00000ff9, 12},
  {   73, 0x00000ffb, 12},  {   45, 0x00001ffa, 13},
  {   46, 0x00001ff9, 13},  {   74, 0x00001ff8, 13},
  {   75, 0x00001ffb, 13},  {   44, 0x00003ffa, 14},
  {   76, 0x00003ff8, 14},  {   77, 0x00003ff9, 14},
  {   42, 0x00007ff7, 15},  {   43, 0x00007ff6, 15},
  {   39, 0x0000fff4, 16},  {   40, 0x0000fff3, 16},
  {   41, 0x0000fff0, 16},  {   78, 0x0000fff1, 16},
  {   79, 0x0000fff2, 16},  {   34, 0x0001ffef, 17},
  {   36, 0x0001ffec, 17},  {   37, 0x0001ffed, 17},
  {   38, 0x0001ffee, 17},  {   80, 0x0001ffea, 17},
  {   81, 0x0001ffeb, 17},  {   19, 0x0003ffe4, 18},
  {   30, 0x0003ffe8, 18},  {   32, 0x0003ffe0, 18},
  {   33, 0x0003ffe9, 18},  {   35, 0x0003ffe5, 18},
  {   82, 0x0003ffe1, 18},  {   83, 0x0003ffe2, 18},
  {   84, 0x0003ffea, 18},  {   85, 0x0003ffe3, 18},
  {   86, 0x0003ffe6, 18},  {   87, 0x0003ffe7, 18},
  {   88, 0x0003ffeb, 18},  {  104, 0x0003ffec, 18},
  {    0, 0x0007ffe7, 19},  {    1, 0x0007ffe8, 19},
  {    9, 0x0007ffda, 19},  {   14, 0x0007ffdb, 19},
  {   16, 0x0007ffdc, 19},  {   17, 0x0007ffdd, 19},
  {   23, 0x0007ffde, 19},  {   27, 0x0007ffdf, 19},
  {   29, 0x0007ffe0, 19},  {   31, 0x0007ffe1, 19},
  {   90, 0x0007ffe2, 19},  {   97, 0x0007ffe3, 19},
  {  102, 0x0007ffe4, 19},  {  107, 0x0007ffe5, 19},
  {  108, 0x0007ffe6, 19},  {    2, 0x000fffd2, 20},
  {    3, 0x000fffd3, 20},  {    4, 0x000fffd4, 20},
  {    5, 0x000fffd5, 20},  {    6, 0x000fffd6, 20},
  {    7, 0x000fffd7, 20},  {    8, 0x000fffd8, 20},
  {   10, 0x000fffd9, 20},  {   11, 0x000fffda, 20},
  {   12, 0x000fffdb, 20},  {   13, 0x000fffdc, 20},
  {   15, 0x000fffdd, 20},  {   18, 0x000fffde, 20},
  {   20, 0x000fffdf, 20},  {   21, 0x000fffe0, 20},
  {   22, 0x000fffe1, 20},  {   24, 0x000fffe2, 20},
  {   25, 0x000fffe3, 20},  {   26, 0x000fffe4, 20},
  {   28, 0x000fffe5, 20},  {   89, 0x000fffe6, 20},
  {   91, 0x000fffe7, 20},  {   92, 0x000fffe8, 20},
  {   93, 0x000fffe9, 20},  {   94, 0x000fffea, 20},
  {   95, 0x000fffeb, 20},  {   96, 0x000fffec, 20},
  {   98, 0x000fffed, 20},  {   99, 0x000fffee, 20},
  {  100, 0x000fffef, 20},  {  101, 0x000ffff0, 20},
  {  103, 0x000ffff1, 20},  {  105, 0x000ffff2, 20},
  {  106, 0x000ffff3, 20},  {  109, 0x000ffff4, 20},
  {  110, 0x000ffff5, 20},  {  111, 0x000ffff6, 20},
  {  112, 0x000ffff7, 20},  {  113, 0x000ffff8, 20},
  {  114, 0x000ffff9, 20},  {  115, 0x000ffffa, 20},
  {  116, 0x000ffffb, 20},  {  117, 0x000ffffc, 20},
  {  118, 0x000ffffd, 20},  {  119, 0x000ffffe, 20},
  {  120, 0x000fffff, 20},
};

/********************************************************************/
// 2
static IppsVLCTable_32s ipp_t_huffman_env_bal_1_5dB[] = {
  {   24, 0x00000000,  1},  {   25, 0x00000002,  2},
  {   23, 0x00000006,  3},  {   26, 0x0000000e,  4},
  {   22, 0x0000001e,  5},  {   27, 0x0000003e,  6},
  {   21, 0x0000007e,  7},  {   28, 0x000000fe,  8},
  {   20, 0x000001fe,  9},  {   19, 0x000007fc, 11},
  {   29, 0x000007fd, 11},  {   18, 0x00000ffc, 12},
  {   30, 0x00000ffd, 12},  {   31, 0x00007ff0, 15},
  {    0, 0x0000ffe4, 16},  {    1, 0x0000ffe5, 16},
  {    2, 0x0000ffe6, 16},  {    3, 0x0000ffe7, 16},
  {    4, 0x0000ffe8, 16},  {    5, 0x0000ffe9, 16},
  {    6, 0x0000ffea, 16},  {    7, 0x0000ffeb, 16},
  {    8, 0x0000ffec, 16},  {    9, 0x0000ffed, 16},
  {   10, 0x0000ffee, 16},  {   11, 0x0000ffef, 16},
  {   12, 0x0000fff0, 16},  {   13, 0x0000fff1, 16},
  {   14, 0x0000fff2, 16},  {   15, 0x0000fff3, 16},
  {   16, 0x0000fff4, 16},  {   17, 0x0000ffe2, 16},
  {   32, 0x0000ffe3, 16},  {   33, 0x0000fff5, 16},
  {   34, 0x0000fff6, 16},  {   35, 0x0000fff7, 16},
  {   36, 0x0000fff8, 16},  {   37, 0x0000fff9, 16},
  {   38, 0x0000fffa, 16},  {   39, 0x0001fff6, 17},
  {   40, 0x0001fff7, 17},  {   41, 0x0001fff8, 17},
  {   42, 0x0001fff9, 17},  {   43, 0x0001fffa, 17},
  {   44, 0x0001fffb, 17},  {   45, 0x0001fffc, 17},
  {   46, 0x0001fffd, 17},  {   47, 0x0001fffe, 17},
  {   48, 0x0001ffff, 17},
};

/********************************************************************/
// 3
static IppsVLCTable_32s ipp_f_huffman_env_bal_1_5dB[] = {
  {   24, 0x00000000,  1},  {   23, 0x00000002,  2},
  {   25, 0x00000006,  3},  {   22, 0x0000000e,  4},
  {   26, 0x0000001e,  5},  {   27, 0x0000003e,  6},
  {   21, 0x0000007e,  7},  {   20, 0x000000fe,  8},
  {   28, 0x000001fe,  9},  {   18, 0x000007fe, 11},
  {   19, 0x000007fc, 11},  {   29, 0x000007fd, 11},
  {   30, 0x00000ffe, 12},  {   17, 0x00003ffc, 14},
  {   31, 0x00007ffa, 15},  {   15, 0x0000fff7, 16},
  {   32, 0x0000fff6, 16},  {   16, 0x0001fff0, 17},
  {    0, 0x0003ffe2, 18},  {    1, 0x0003ffe3, 18},
  {    2, 0x0003ffe4, 18},  {    3, 0x0003ffe5, 18},
  {    4, 0x0003ffe6, 18},  {    5, 0x0003ffe7, 18},
  {    6, 0x0003ffe8, 18},  {    7, 0x0003ffe9, 18},
  {    8, 0x0003ffea, 18},  {    9, 0x0003ffeb, 18},
  {   10, 0x0003ffec, 18},  {   11, 0x0003ffed, 18},
  {   12, 0x0003ffee, 18},  {   13, 0x0003ffef, 18},
  {   14, 0x0003fff0, 18},  {   33, 0x0003fff1, 18},
  {   34, 0x0003fff2, 18},  {   35, 0x0003fff3, 18},
  {   36, 0x0003fff4, 18},  {   37, 0x0003fff5, 18},
  {   38, 0x0003fff6, 18},  {   39, 0x0003fff7, 18},
  {   40, 0x0003fff8, 18},  {   41, 0x0003fff9, 18},
  {   42, 0x0003fffa, 18},  {   43, 0x0003fffb, 18},
  {   44, 0x0003fffc, 18},  {   45, 0x0003fffd, 18},
  {   46, 0x0003fffe, 18},  {   47, 0x0007fffe, 19},
  {   48, 0x0007ffff, 19},
};

/********************************************************************/
// 4
static IppsVLCTable_32s ipp_t_huffman_env_3_0dB[] = {
  {   31, 0x00000000,  1},  {   30, 0x00000002,  2},
  {   32, 0x00000006,  3},  {   29, 0x0000000e,  4},
  {   33, 0x0000001e,  5},  {   28, 0x0000003e,  6},
  {   34, 0x0000007e,  7},  {   27, 0x000000fe,  8},
  {   35, 0x000001fe,  9},  {   26, 0x000007fc, 11},
  {   36, 0x000007fd, 11},  {   25, 0x00000ffc, 12},
  {   24, 0x00001ffa, 13},  {   37, 0x00001ffb, 13},
  {   21, 0x00003ffb, 14},  {   22, 0x00003ffa, 14},
  {   23, 0x00003ff8, 14},  {   38, 0x00003ff9, 14},
  {   39, 0x00003ffc, 14},  {   40, 0x00007ffa, 15},
  {   18, 0x0000fff7, 16},  {   19, 0x0000fff9, 16},
  {   20, 0x0000fff8, 16},  {   41, 0x0000fff6, 16},
  {   17, 0x0001fff4, 17},  {   42, 0x0001fff5, 17},
  {    0, 0x0003ffed, 18},  {    1, 0x0003ffee, 18},
  {   43, 0x0003ffec, 18},  {    2, 0x0007ffde, 19},
  {    3, 0x0007ffdf, 19},  {    4, 0x0007ffe0, 19},
  {    5, 0x0007ffe1, 19},  {    6, 0x0007ffe2, 19},
  {    7, 0x0007ffe3, 19},  {    8, 0x0007ffe4, 19},
  {    9, 0x0007ffe5, 19},  {   10, 0x0007ffe6, 19},
  {   11, 0x0007ffe7, 19},  {   12, 0x0007ffe8, 19},
  {   13, 0x0007ffe9, 19},  {   14, 0x0007ffea, 19},
  {   15, 0x0007ffeb, 19},  {   16, 0x0007ffec, 19},
  {   44, 0x0007ffed, 19},  {   45, 0x0007ffee, 19},
  {   46, 0x0007ffef, 19},  {   47, 0x0007fff0, 19},
  {   48, 0x0007fff1, 19},  {   49, 0x0007fff2, 19},
  {   50, 0x0007fff3, 19},  {   51, 0x0007fff4, 19},
  {   52, 0x0007fff5, 19},  {   53, 0x0007fff6, 19},
  {   54, 0x0007fff7, 19},  {   55, 0x0007fff8, 19},
  {   56, 0x0007fff9, 19},  {   57, 0x0007fffa, 19},
  {   58, 0x0007fffb, 19},  {   59, 0x0007fffc, 19},
  {   60, 0x0007fffd, 19},  {   61, 0x0007fffe, 19},
  {   62, 0x0007ffff, 19},
};

/********************************************************************/
// 5
static IppsVLCTable_32s ipp_f_huffman_env_3_0dB[] = {
  {   31, 0x00000000,  1},  {   30, 0x00000002,  2},
  {   32, 0x00000006,  3},  {   29, 0x0000000e,  4},
  {   33, 0x0000001e,  5},  {   28, 0x0000003e,  6},
  {   27, 0x000000fd,  8},  {   34, 0x000000fc,  8},
  {   26, 0x000001fd,  9},  {   35, 0x000001fc,  9},
  {   25, 0x000003fd, 10},  {   36, 0x000003fc, 10},
  {   24, 0x000007fd, 11},  {   37, 0x000007fc, 11},
  {   23, 0x00000ffd, 12},  {   38, 0x00000ffc, 12},
  {   39, 0x00001ffc, 13},  {   22, 0x00003ffb, 14},
  {   40, 0x00003ffa, 14},  {   21, 0x00007ff8, 15},
  {   41, 0x00007ff9, 15},  {   42, 0x00007ffa, 15},
  {   19, 0x0000fff7, 16},  {   20, 0x0000fff6, 16},
  {   43, 0x0000fff8, 16},  {   44, 0x0000fff9, 16},
  {   16, 0x0001fff5, 17},  {   18, 0x0001fff4, 17},
  {   45, 0x0001fff6, 17},  {   46, 0x0001fff7, 17},
  {    7, 0x0003fff3, 18},  {   12, 0x0003fff4, 18},
  {   13, 0x0003fff2, 18},  {   17, 0x0003fff0, 18},
  {   47, 0x0003fff5, 18},  {   48, 0x0003fff6, 18},
  {   49, 0x0003fff1, 18},  {    8, 0x0007fff5, 19},
  {    9, 0x0007ffee, 19},  {   10, 0x0007ffef, 19},
  {   11, 0x0007fff6, 19},  {   15, 0x0007fff0, 19},
  {   51, 0x0007fff1, 19},  {   52, 0x0007fff2, 19},
  {   53, 0x0007fff3, 19},  {   55, 0x0007fff7, 19},
  {   56, 0x0007fff4, 19},  {    0, 0x000ffff0, 20},
  {    1, 0x000ffff1, 20},  {    2, 0x000ffff2, 20},
  {    3, 0x000ffff3, 20},  {    4, 0x000ffff4, 20},
  {    5, 0x000ffff5, 20},  {    6, 0x000ffff6, 20},
  {   14, 0x000ffff7, 20},  {   50, 0x000ffff8, 20},
  {   54, 0x000ffff9, 20},  {   57, 0x000ffffa, 20},
  {   58, 0x000ffffb, 20},  {   59, 0x000ffffc, 20},
  {   60, 0x000ffffd, 20},  {   61, 0x000ffffe, 20},
  {   62, 0x000fffff, 20},
};

/********************************************************************/
// 6
static IppsVLCTable_32s ipp_t_huffman_env_bal_3_0dB[] = {
  {   12, 0x00000000,  1},  {   13, 0x00000002,  2},
  {   11, 0x00000006,  3},  {   10, 0x0000000e,  4},
  {   14, 0x0000001e,  5},  {   15, 0x0000003e,  6},
  {    9, 0x0000007e,  7},  {    8, 0x000000fe,  8},
  {   16, 0x000001fe,  9},  {    7, 0x00000ff8, 12},
  {    0, 0x00001ff2, 13},  {    1, 0x00001ff3, 13},
  {    2, 0x00001ff4, 13},  {    3, 0x00001ff5, 13},
  {    4, 0x00001ff6, 13},  {    5, 0x00001ff7, 13},
  {    6, 0x00001ff8, 13},  {   17, 0x00001ff9, 13},
  {   18, 0x00001ffa, 13},  {   19, 0x00001ffb, 13},
  {   20, 0x00001ffc, 13},  {   21, 0x00001ffd, 13},
  {   22, 0x00001ffe, 13},  {   23, 0x00003ffe, 14},
  {   24, 0x00003fff, 14},
};

/********************************************************************/
// 7
static IppsVLCTable_32s ipp_f_huffman_env_bal_3_0dB[] = {
  {   12, 0x00000000,  1},  {   11, 0x00000002,  2},
  {   13, 0x00000006,  3},  {   10, 0x0000000e,  4},
  {   14, 0x0000001e,  5},  {   15, 0x0000003e,  6},
  {    9, 0x0000007e,  7},  {    8, 0x000000fe,  8},
  {   16, 0x000001fe,  9},  {    7, 0x000007fc, 11},
  {   17, 0x00000ffa, 12},  {    0, 0x00001ff7, 13},
  {    1, 0x00001ff8, 13},  {    2, 0x00001ff9, 13},
  {    3, 0x00001ffa, 13},  {    4, 0x00001ffb, 13},
  {   18, 0x00001ff6, 13},  {    5, 0x00003ff8, 14},
  {    6, 0x00003ff9, 14},  {   19, 0x00003ffa, 14},
  {   20, 0x00003ffb, 14},  {   21, 0x00003ffc, 14},
  {   22, 0x00003ffd, 14},  {   23, 0x00003ffe, 14},
  {   24, 0x00003fff, 14},
};

/********************************************************************/
// 8
static IppsVLCTable_32s ipp_t_huffman_noise_3_0dB[] = {
  {   31, 0x00000000,  1},  {   32, 0x00000002,  2},
  {   30, 0x00000006,  3},  {   29, 0x0000000e,  4},
  {   33, 0x0000001e,  5},  {   28, 0x0000003e,  6},
  {   27, 0x000000fd,  8},  {   34, 0x000000fc,  8},
  {   35, 0x000003f8, 10},  {   26, 0x000007f2, 11},
  {    0, 0x00001fce, 13},  {    1, 0x00001fcf, 13},
  {    2, 0x00001fd0, 13},  {    3, 0x00001fd1, 13},
  {    4, 0x00001fd2, 13},  {    5, 0x00001fd3, 13},
  {    6, 0x00001fd4, 13},  {    7, 0x00001fd5, 13},
  {    8, 0x00001fd6, 13},  {    9, 0x00001fd7, 13},
  {   10, 0x00001fd8, 13},  {   11, 0x00001fd9, 13},
  {   12, 0x00001fda, 13},  {   13, 0x00001fdb, 13},
  {   14, 0x00001fdc, 13},  {   15, 0x00001fdd, 13},
  {   16, 0x00001fde, 13},  {   17, 0x00001fdf, 13},
  {   18, 0x00001fe0, 13},  {   19, 0x00001fe1, 13},
  {   20, 0x00001fe2, 13},  {   21, 0x00001fe3, 13},
  {   22, 0x00001fe4, 13},  {   23, 0x00001fe5, 13},
  {   24, 0x00001fe6, 13},  {   25, 0x00001fe7, 13},
  {   36, 0x00001fcc, 13},  {   37, 0x00001fe8, 13},
  {   38, 0x00001fe9, 13},  {   39, 0x00001fea, 13},
  {   40, 0x00001feb, 13},  {   41, 0x00001fec, 13},
  {   42, 0x00001fcd, 13},  {   43, 0x00001fed, 13},
  {   44, 0x00001fee, 13},  {   45, 0x00001fef, 13},
  {   46, 0x00001ff0, 13},  {   47, 0x00001ff1, 13},
  {   48, 0x00001ff2, 13},  {   49, 0x00001ff3, 13},
  {   50, 0x00001ff4, 13},  {   51, 0x00001ff5, 13},
  {   52, 0x00001ff6, 13},  {   53, 0x00001ff7, 13},
  {   54, 0x00001ff8, 13},  {   55, 0x00001ff9, 13},
  {   56, 0x00001ffa, 13},  {   57, 0x00001ffb, 13},
  {   58, 0x00001ffc, 13},  {   59, 0x00001ffd, 13},
  {   60, 0x00001ffe, 13},  {   61, 0x00003ffe, 14},
  {   62, 0x00003fff, 14},
};

/********************************************************************/
// 9
static IppsVLCTable_32s ipp_t_huffman_noise_bal_3_0dB[] = {
  {   12, 0x00000000,  1},  {   11, 0x00000002,  2},
  {   13, 0x00000006,  3},  {   10, 0x0000001c,  5},
  {   14, 0x0000003a,  6},  {    0, 0x000000ec,  8},
  {    1, 0x000000ed,  8},  {    2, 0x000000ee,  8},
  {    3, 0x000000ef,  8},  {    4, 0x000000f0,  8},
  {    5, 0x000000f1,  8},  {    6, 0x000000f2,  8},
  {    7, 0x000000f3,  8},  {    8, 0x000000f4,  8},
  {    9, 0x000000f5,  8},  {   15, 0x000000f6,  8},
  {   16, 0x000000f7,  8},  {   17, 0x000000f8,  8},
  {   18, 0x000000f9,  8},  {   19, 0x000000fa,  8},
  {   20, 0x000000fb,  8},  {   21, 0x000000fc,  8},
  {   22, 0x000000fd,  8},  {   23, 0x000000fe,  8},
  {   24, 0x000000ff,  8},
};

/********************************************************************/

IppsVLCTable_32s* vlcSbrBooks[] = {
  ipp_t_huffman_env_1_5dB,//0
  ipp_f_huffman_env_1_5dB,//1
  ipp_t_huffman_env_bal_1_5dB,//2
  ipp_f_huffman_env_bal_1_5dB,//3
  ipp_t_huffman_env_3_0dB,//4
  ipp_f_huffman_env_3_0dB,//5
  ipp_t_huffman_env_bal_3_0dB,//6
  ipp_f_huffman_env_bal_3_0dB,//7
  ipp_t_huffman_noise_3_0dB,//8
  ipp_t_huffman_noise_bal_3_0dB//9
};

/********************************************************************/

IppStatus ownInitSBREncHuffTabs(IppsVLCEncodeSpec_32s** ppSpec,
                                Ipp32s *sizeAll)
{
  IppStatus status = ippStsNoErr;
  Ipp32s i, mSize;
  Ipp32s size[NUM_SBR_HUFF_TABS];

  mSize = 0;
  for (i = 0; i < NUM_SBR_HUFF_TABS; i++) {
    status = ippsVLCEncodeGetSize_32s(vlcSbrBooks[i],
                                      vlcSbrTableSizes[i],
                                      &size[i]);
    if (status != ippStsNoErr) return status;
    mSize += size[i];
  }

  *sizeAll = mSize;

  if (ppSpec) {
    for (i = 1; i < NUM_SBR_HUFF_TABS; i++) {
      ppSpec[i] = (IppsVLCEncodeSpec_32s*)((Ipp8u*)ppSpec[i-1] + size[i-1]);
    }

    for (i = 0; i < NUM_SBR_HUFF_TABS; i++) {
      status = ippsVLCEncodeInit_32s(vlcSbrBooks[i],
                                     vlcSbrTableSizes[i],
                                     ppSpec[i]);
      if (status != ippStsNoErr) return status;
    }
  }

  return status;
}

/****************************************************************************/

Ipp32s ownVLCCountBits_16s32s(Ipp16s delta, IppsVLCEncodeSpec_32s* pHuffTab)
{
  Ipp32s num_bits = 0;

  ippsVLCCountBits_16s32s (&delta, 1, &num_bits, pHuffTab);

  return num_bits;
}

/*
#define HUFF_NOISE_COMPRESS 0

#define HUFF_ENV_COMPRESS   1
*/

/*******************************************************************************/

Ipp32s sbrencSetEnvHuffTabs(Ipp32s  bs_amp_res,
                            Ipp16s* LAV,
                            Ipp32s* bs_env_start_bits,

                            IppsVLCEncodeSpec_32s** pTimeHuffTab,
                            IppsVLCEncodeSpec_32s** pFreqHuffTab,
                            IppsVLCEncodeSpec_32s*  sbrHuffTabs[NUM_SBR_HUFF_TABS],

                            Ipp32s  typeCompress) /* [1] - envelope, [0] - noise */
{
  Ipp32s criterion_1_5dB = !bs_amp_res; //flag || !bs_amp_res;

  if( HUFF_ENV_COMPRESS == typeCompress ){ /* envelope compress */
    if( criterion_1_5dB ){

      *pTimeHuffTab = sbrHuffTabs[0]; /* = t_huffman_env_1_5dB */
      *pFreqHuffTab = sbrHuffTabs[1]; /* = f_huffman_env_1_5dB */

      *LAV = 60;
      *bs_env_start_bits = 7;
    }else{

      *pTimeHuffTab = sbrHuffTabs[4]; /* = t_huffman_env_3_0dB */
      *pFreqHuffTab = sbrHuffTabs[5]; /* = f_huffman_env_3_0dB */

      *LAV = 31;
      *bs_env_start_bits = 6;
    }

  } else { /* noise compress */
    //------------------------------------------------
    *pTimeHuffTab = sbrHuffTabs[8]; /* = t_huffman_noise_3_0dB */
    *pFreqHuffTab = sbrHuffTabs[5]; /* = f_huffman_env_3_0dB = f_huffman_noise_3_0dB */

    *LAV = 31;
    *bs_env_start_bits = 5;
    //------------------------------------------------
  }

  return 0;//OK
}

#endif //UMC_ENABLE_XXX

