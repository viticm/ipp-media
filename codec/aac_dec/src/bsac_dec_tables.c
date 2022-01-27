/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2007 Intel Corporation. All Rights Reserved.
//
//     Intel(R) Integrated Performance Primitives AAC Decode Sample for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel(R) Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP
//  product installation for more information.
//
//  MPEG-4 and AAC are international standards promoted by ISO, IEC, ITU, ETSI
//  and other organizations. Implementations of these standards, or the standard
//  enabled platforms may require licenses from various entities, including
//  Intel Corporation.
//
*/

#include "umc_defs.h"

#if defined (UMC_ENABLE_AAC_AUDIO_DECODER)

/********************************************************************/

#include "ipp.h"

Ipp32s bsacHalf[] = {
  0x20000000, 0x10000000, 0x08000000, 0x04000000,
  0x02000000, 0x01000000, 0x00800000, 0x00400000,
  0x00200000, 0x00100000, 0x00080000, 0x00040000,
  0x00020000, 0x00010000, 0x00008000, 0x00004000
};

Ipp8u bsacMaxCbandSiLenTbl[] = {
  6,  5,  6,  5,  6,  6,  5,  6,  5,  6,  5,  6,  8,  6,  5,  6,
  8,  9,  6,  5,  6,  8, 10,  8, 10,  9, 10, 10, 12, 12, 12, 12
};

Ipp8u bsacLargestCband0SiTbl[] = {
  6,  6,  8,  8,  8, 10, 10, 10, 10, 12, 12, 12, 12, 14, 14, 14,
 14, 14, 15, 15, 15, 15, 15, 16, 16, 17, 17, 18, 19, 20, 21, 22
};

Ipp8u bsacLargestCbandSiTbl[] = {
  4,  6,  4,  6,  8,  4,  6,  8, 10,  4,  6,  8, 12,  4,  6,  8,
 12, 14,  4,  6,  8, 12, 15, 12, 16, 14, 17, 18, 19, 20, 21, 22
};

Ipp16s bsacCbandModel0[] = {
  0x3ef6, 0x3b59, 0x1b12, 0x12a3, 0x0000
};

Ipp16s bsacCbandModel1[] = {
  0x3d51, 0x33ae, 0x1cff, 0x0fb7, 0x07e4, 0x022b, 0x0000
};

Ipp16s bsacCbandModel2[] = {
  0x3a47, 0x2aec, 0x1e05, 0x1336, 0x0e7d, 0x0860, 0x05e0, 0x044a,
  0x0000
};

Ipp16s bsacCbandModel3[] = {
  0x36be, 0x27ae, 0x20f4, 0x1749, 0x14d5, 0x0d46, 0x0ad3, 0x0888,
  0x0519, 0x020b, 0x0000
};

Ipp16s bsacCbandModel4[] = {
  0x3983, 0x2e77, 0x2b03, 0x1ee8, 0x1df9, 0x1307, 0x11e4, 0x0b4d,
  0x094c, 0x0497, 0x0445, 0x0040, 0x0000
};

Ipp16s bsacCbandModel5[] = {
  0x306f, 0x249e, 0x1f56, 0x1843, 0x161a, 0x102d, 0x0f6c, 0x0c81,
  0x0af2, 0x07a8, 0x071a, 0x0454, 0x0413, 0x0016, 0x0000
};

Ipp16s bsacCbandModel6[] = {
  0x31af, 0x2001, 0x162d, 0x127e, 0x0f05, 0x0c34, 0x0b8f, 0x0a61,
  0x0955, 0x0825, 0x07dd, 0x06a9, 0x0688, 0x055b, 0x054b, 0x02f7,
  0x0198, 0x0077, 0x0010, 0x000c, 0x0008, 0x0004, 0x0000
};

Ipp16s bsacCbandModel7[] = {
  0x3ff8, 0x3ff0, 0x3fe8, 0x3fe0, 0x3fd7, 0x3f31, 0x3cd7, 0x3bc9,
  0x3074, 0x2bcf, 0x231b, 0x13db, 0x0d51, 0x0603, 0x044c, 0x0080,
  0x0030, 0x0028, 0x0020, 0x0018, 0x0010, 0x0008, 0x0000
};

Ipp16s *bsacCbandModelTab[] = {
  bsacCbandModel0, bsacCbandModel1, bsacCbandModel0, bsacCbandModel1,
  bsacCbandModel2, bsacCbandModel0, bsacCbandModel1, bsacCbandModel2,
  bsacCbandModel3, bsacCbandModel0, bsacCbandModel1, bsacCbandModel2,
  bsacCbandModel4, bsacCbandModel0, bsacCbandModel1, bsacCbandModel2,
  bsacCbandModel4, bsacCbandModel5, bsacCbandModel0, bsacCbandModel1,
  bsacCbandModel2, bsacCbandModel4, bsacCbandModel6, bsacCbandModel4,
  bsacCbandModel6, bsacCbandModel5, bsacCbandModel6, bsacCbandModel6,
  bsacCbandModel6, bsacCbandModel6, bsacCbandModel6, bsacCbandModel6
};

Ipp16s bsacMsUsedModelTab[] = {
  0x2ccd, 0x0000
};

Ipp16s bsacStereoInfoModelTab[] = {
  0x3666, 0x1000, 0x0666, 0x0000
};

Ipp16s bsacNoiseFlagModelTab[] = {
  0x2000, 0x0000
};

Ipp16s bsacNoiseModeModelTab[] = {
  0x3000, 0x2000, 0x1000, 0x0000
};

Ipp16s bsacNoiseEnergyModelTab[] = {
  0x3fe0, 0x3fc0, 0x3fa0, 0x3f80, 0x3f60, 0x3f40, 0x3f20, 0x3f00,
  0x3ee0, 0x3ec0, 0x3ea0, 0x3e80, 0x3e60, 0x3e40, 0x3e20, 0x3e00,
  0x3de0, 0x3dc0, 0x3da0, 0x3d80, 0x3d60, 0x3d40, 0x3d20, 0x3d00,
  0x3ce0, 0x3cc0, 0x3ca0, 0x3c80, 0x3c60, 0x3c40, 0x3c20, 0x3c00,
  0x3be0, 0x3bc0, 0x3ba0, 0x3b80, 0x3b60, 0x3b40, 0x3b20, 0x3b00,
  0x3ae0, 0x3ac0, 0x3aa0, 0x3a80, 0x3a60, 0x3a40, 0x3a20, 0x3a00,
  0x39e0, 0x39c0, 0x39a0, 0x3980, 0x3960, 0x3940, 0x3920, 0x3900,
  0x38e0, 0x38c0, 0x38a0, 0x3880, 0x3860, 0x3840, 0x3820, 0x3800,
  0x37e0, 0x37c0, 0x37a0, 0x3780, 0x3760, 0x3740, 0x3720, 0x3700,
  0x36e0, 0x36c0, 0x36a0, 0x3680, 0x3660, 0x3640, 0x3620, 0x3600,
  0x35e0, 0x35c0, 0x35a0, 0x3580, 0x3560, 0x3540, 0x3520, 0x3500,
  0x34e0, 0x34c0, 0x34a0, 0x3480, 0x3460, 0x3440, 0x3420, 0x3400,
  0x33e0, 0x33c0, 0x33a0, 0x3380, 0x3360, 0x3340, 0x3320, 0x3300,
  0x32e0, 0x32c0, 0x32a0, 0x3280, 0x3260, 0x3240, 0x3220, 0x3200,
  0x31e0, 0x31c0, 0x31a0, 0x3180, 0x3160, 0x3140, 0x3120, 0x3100,
  0x30e0, 0x30c0, 0x30a0, 0x3080, 0x3060, 0x3040, 0x3020, 0x3000,
  0x2fe0, 0x2fc0, 0x2fa0, 0x2f80, 0x2f60, 0x2f40, 0x2f20, 0x2f00,
  0x2ee0, 0x2ec0, 0x2ea0, 0x2e80, 0x2e60, 0x2e40, 0x2e20, 0x2e00,
  0x2de0, 0x2dc0, 0x2da0, 0x2d80, 0x2d60, 0x2d40, 0x2d20, 0x2d00,
  0x2ce0, 0x2cc0, 0x2ca0, 0x2c80, 0x2c60, 0x2c40, 0x2c20, 0x2c00,
  0x2be0, 0x2bc0, 0x2ba0, 0x2b80, 0x2b60, 0x2b40, 0x2b20, 0x2b00,
  0x2ae0, 0x2ac0, 0x2aa0, 0x2a80, 0x2a60, 0x2a40, 0x2a20, 0x2a00,
  0x29e0, 0x29c0, 0x29a0, 0x2980, 0x2960, 0x2940, 0x2920, 0x2900,
  0x28e0, 0x28c0, 0x28a0, 0x2880, 0x2860, 0x2840, 0x2820, 0x2800,
  0x27e0, 0x27c0, 0x27a0, 0x2780, 0x2760, 0x2740, 0x2720, 0x2700,
  0x26e0, 0x26c0, 0x26a0, 0x2680, 0x2660, 0x2640, 0x2620, 0x2600,
  0x25e0, 0x25c0, 0x25a0, 0x2580, 0x2560, 0x2540, 0x2520, 0x2500,
  0x24e0, 0x24c0, 0x24a0, 0x2480, 0x2460, 0x2440, 0x2420, 0x2400,
  0x23e0, 0x23c0, 0x23a0, 0x2380, 0x2360, 0x2340, 0x2320, 0x2300,
  0x22e0, 0x22c0, 0x22a0, 0x2280, 0x2260, 0x2240, 0x2220, 0x2200,
  0x21e0, 0x21c0, 0x21a0, 0x2180, 0x2160, 0x2140, 0x2120, 0x2100,
  0x20e0, 0x20c0, 0x20a0, 0x2080, 0x2060, 0x2040, 0x2020, 0x2000,
  0x1fe0, 0x1fc0, 0x1fa0, 0x1f80, 0x1f60, 0x1f40, 0x1f20, 0x1f00,
  0x1ee0, 0x1ec0, 0x1ea0, 0x1e80, 0x1e60, 0x1e40, 0x1e20, 0x1e00,
  0x1de0, 0x1dc0, 0x1da0, 0x1d80, 0x1d60, 0x1d40, 0x1d20, 0x1d00,
  0x1ce0, 0x1cc0, 0x1ca0, 0x1c80, 0x1c60, 0x1c40, 0x1c20, 0x1c00,
  0x1be0, 0x1bc0, 0x1ba0, 0x1b80, 0x1b60, 0x1b40, 0x1b20, 0x1b00,
  0x1ae0, 0x1ac0, 0x1aa0, 0x1a80, 0x1a60, 0x1a40, 0x1a20, 0x1a00,
  0x19e0, 0x19c0, 0x19a0, 0x1980, 0x1960, 0x1940, 0x1920, 0x1900,
  0x18e0, 0x18c0, 0x18a0, 0x1880, 0x1860, 0x1840, 0x1820, 0x1800,
  0x17e0, 0x17c0, 0x17a0, 0x1780, 0x1760, 0x1740, 0x1720, 0x1700,
  0x16e0, 0x16c0, 0x16a0, 0x1680, 0x1660, 0x1640, 0x1620, 0x1600,
  0x15e0, 0x15c0, 0x15a0, 0x1580, 0x1560, 0x1540, 0x1520, 0x1500,
  0x14e0, 0x14c0, 0x14a0, 0x1480, 0x1460, 0x1440, 0x1420, 0x1400,
  0x13e0, 0x13c0, 0x13a0, 0x1380, 0x1360, 0x1340, 0x1320, 0x1300,
  0x12e0, 0x12c0, 0x12a0, 0x1280, 0x1260, 0x1240, 0x1220, 0x1200,
  0x11e0, 0x11c0, 0x11a0, 0x1180, 0x1160, 0x1140, 0x1120, 0x1100,
  0x10e0, 0x10c0, 0x10a0, 0x1080, 0x1060, 0x1040, 0x1020, 0x1000,
  0x0fe0, 0x0fc0, 0x0fa0, 0x0f80, 0x0f60, 0x0f40, 0x0f20, 0x0f00,
  0x0ee0, 0x0ec0, 0x0ea0, 0x0e80, 0x0e60, 0x0e40, 0x0e20, 0x0e00,
  0x0de0, 0x0dc0, 0x0da0, 0x0d80, 0x0d60, 0x0d40, 0x0d20, 0x0d00,
  0x0ce0, 0x0cc0, 0x0ca0, 0x0c80, 0x0c60, 0x0c40, 0x0c20, 0x0c00,
  0x0be0, 0x0bc0, 0x0ba0, 0x0b80, 0x0b60, 0x0b40, 0x0b20, 0x0b00,
  0x0ae0, 0x0ac0, 0x0aa0, 0x0a80, 0x0a60, 0x0a40, 0x0a20, 0x0a00,
  0x09e0, 0x09c0, 0x09a0, 0x0980, 0x0960, 0x0940, 0x0920, 0x0900,
  0x08e0, 0x08c0, 0x08a0, 0x0880, 0x0860, 0x0840, 0x0820, 0x0800,
  0x07e0, 0x07c0, 0x07a0, 0x0780, 0x0760, 0x0740, 0x0720, 0x0700,
  0x06e0, 0x06c0, 0x06a0, 0x0680, 0x0660, 0x0640, 0x0620, 0x0600,
  0x05e0, 0x05c0, 0x05a0, 0x0580, 0x0560, 0x0540, 0x0520, 0x0500,
  0x04e0, 0x04c0, 0x04a0, 0x0480, 0x0460, 0x0440, 0x0420, 0x0400,
  0x03e0, 0x03c0, 0x03a0, 0x0380, 0x0360, 0x0340, 0x0320, 0x0300,
  0x02e0, 0x02c0, 0x02a0, 0x0280, 0x0260, 0x0240, 0x0220, 0x0200,
  0x01e0, 0x01c0, 0x01a0, 0x0180, 0x0160, 0x0140, 0x0120, 0x0100,
  0x00e0, 0x00c0, 0x00a0, 0x0080, 0x0060, 0x0040, 0x0020, 0x0000
};

Ipp16s bsacScfModel0[] = {
  0x0001, 0x0000
};

Ipp16s bsacScfModel1[] = {
  0x0752, 0x03cd, 0x014d, 0x0000
};

Ipp16s bsacScfModel2[] = {
  0x112f, 0x0de7, 0x0a8b, 0x07c1, 0x047a, 0x023a, 0x00d4, 0x0000
};

Ipp16s bsacScfModel3[] = {
  0x1f67, 0x1c5f, 0x18d8, 0x1555, 0x1215, 0x0eb4, 0x0adc, 0x0742,
  0x0408, 0x01e6, 0x00df, 0x0052, 0x0032, 0x0023, 0x000c
};

Ipp16s bsacScfModel4[] = {
  0x250f, 0x22b8, 0x2053, 0x1deb, 0x1b05, 0x186d, 0x15df, 0x12d9,
  0x0f77, 0x0c01, 0x0833, 0x050d, 0x0245, 0x008c, 0x0033, 0x0000
};

Ipp16s bsacScfModel5[] = {
  0x08a8, 0x074e, 0x0639, 0x0588, 0x048c, 0x03cf, 0x032e, 0x0272,
  0x01bc, 0x013e, 0x00e4, 0x0097, 0x0069, 0x0043, 0x002f, 0x0029,
  0x0020, 0x001b, 0x0018, 0x0015, 0x0012, 0x000f, 0x000d, 0x000c,
  0x000a, 0x0009, 0x0007, 0x0006, 0x0004, 0x0003, 0x0001, 0x0000
};

Ipp16s bsacScfModel6[] = {
  0x0c2a, 0x099f, 0x0809, 0x06ec, 0x0603, 0x053d, 0x0491, 0x040e,
  0x0394, 0x030a, 0x02a5, 0x0259, 0x0202, 0x01bc, 0x0170, 0x0133,
  0x0102, 0x00c9, 0x0097, 0x0073, 0x004f, 0x0037, 0x0022, 0x0016,
  0x000f, 0x000b, 0x0009, 0x0007, 0x0005, 0x0003, 0x0001, 0x0000,
};

Ipp16s bsacScfModel7[] = {
  0x3b5e, 0x3a90, 0x39d3, 0x387c, 0x3702, 0x3566, 0x33a7, 0x321c,
  0x2f90, 0x2cf2, 0x29fe, 0x26fa, 0x23e4, 0x20df, 0x1e0d, 0x1ac4,
  0x1804, 0x159a, 0x131e, 0x10e7, 0x0e5b, 0x0c9c, 0x0b78, 0x0a21,
  0x08fd, 0x07b7, 0x06b5, 0x062c, 0x055d, 0x04f6, 0x04d4, 0x044b,
  0x038e, 0x02e2, 0x029d, 0x0236, 0x0225, 0x01f2, 0x01cf, 0x01ad,
  0x019c, 0x0179, 0x0168, 0x0157, 0x0146, 0x0135, 0x0123, 0x0112,
  0x0101, 0x00f0, 0x00df, 0x00ce, 0x00bc, 0x00ab, 0x009a, 0x0089,
  0x0078, 0x0067, 0x0055, 0x0044, 0x0033, 0x0022, 0x0011, 0x0000,
};

Ipp16s *bsacScfModelTab[] = {
  bsacScfModel0, bsacScfModel1,bsacScfModel2, bsacScfModel3,
  bsacScfModel4, bsacScfModel5,bsacScfModel6, bsacScfModel7
};

Ipp16s bsacSpectralProbabilityTab[] = {
  0x3900, 0x3a00, 0x2f00, 0x3b00, 0x2f00, 0x3700, 0x2c00, 0x3b00,
  0x3000, 0x3600, 0x2d00, 0x3900, 0x2f00, 0x3700, 0x2c00, 0x2800,
  0x2800, 0x2500, 0x2900, 0x2600, 0x2700, 0x2300, 0x2a00, 0x2700,
  0x2800, 0x2400, 0x2800, 0x2500, 0x2600, 0x2200, 0x3d00, 0x3d00,
  0x3300, 0x3d00, 0x3300, 0x3b00, 0x3300, 0x3d00, 0x3200, 0x3b00,
  0x3100, 0x3e00, 0x3700, 0x3c00, 0x3300, 0x3700, 0x3a00, 0x2800,
  0x3b00, 0x2600, 0x2c00, 0x2400, 0x3a00, 0x2500, 0x2b00, 0x2400,
  0x3100, 0x2300, 0x2900, 0x2300, 0x3000, 0x2c00, 0x1d00, 0x2200,
  0x1a00, 0x1c00, 0x1600, 0x2700, 0x2200, 0x1a00, 0x1d00, 0x1900,
  0x1c00, 0x1e00, 0x2c00, 0x2400, 0x1900, 0x1e00, 0x1f00, 0x1c00,
  0x2b00, 0x2400, 0x2900, 0x2700, 0x2400, 0x1300, 0x1a00, 0x2000,
  0x1800, 0x2300, 0x2500, 0x1f00, 0x2c00, 0x2300, 0x3600, 0x2800,
  0x3100, 0x2500, 0x1400, 0x1200, 0x1800, 0x1400, 0x2100, 0x2200,
  0x1000, 0x1e00, 0x3000, 0x2600, 0x1200, 0x2200, 0x3100, 0x3900,
  0x3a00, 0x2e00, 0x3a00, 0x2f00, 0x3400, 0x2a00, 0x3a00, 0x3000,
  0x3500, 0x2c00, 0x3600, 0x2b00, 0x3100, 0x2500, 0x1e00, 0x1d00,
  0x1c00, 0x1d00, 0x1c00, 0x1d00, 0x1b00, 0x1d00, 0x1e00, 0x1e00,
  0x1a00, 0x1e00, 0x1c00, 0x1d00, 0x1b00, 0x1a00, 0x1a00, 0x1800,
  0x1800, 0x1800, 0x1700, 0x1700, 0x1800, 0x1a00, 0x1700, 0x1700,
  0x1900, 0x1800, 0x1600, 0x1700, 0x1600, 0x1500, 0x1700, 0x1800,
  0x1600, 0x1c00, 0x1700, 0x1900, 0x1700, 0x1500, 0x1c00, 0x1500,
  0x1600, 0x0f00, 0x1800, 0x1400, 0x1700, 0x1a00, 0x1a00, 0x1e00,
  0x1800, 0x1c00, 0x1b00, 0x1500, 0x1300, 0x1500, 0x1400, 0x1600,
  0x1500, 0x1700, 0x1600, 0x1b00, 0x1800, 0x1400, 0x1400, 0x3600,
  0x3d00, 0x3d00, 0x3200, 0x3d00, 0x3300, 0x3d00, 0x3600, 0x3d00,
  0x3500, 0x3c00, 0x3500, 0x3f00, 0x3b00, 0x3f00, 0x3d00, 0x3c00,
  0x3d00, 0x2b00, 0x3d00, 0x2900, 0x3500, 0x2c00, 0x3d00, 0x2b00,
  0x3400, 0x2b00, 0x3800, 0x2b00, 0x3700, 0x2a00, 0x3900, 0x3400,
  0x2400, 0x2a00, 0x1c00, 0x1f00, 0x1600, 0x3500, 0x2500, 0x1a00,
  0x2a00, 0x2200, 0x2b00, 0x2a00, 0x3500, 0x2600, 0x1a00, 0x2600,
  0x2500, 0x2700, 0x3500, 0x2d00, 0x3800, 0x3200, 0x2e00, 0x1800,
  0x1600, 0x2900, 0x2500, 0x3100, 0x2c00, 0x2300, 0x3600, 0x3000,
  0x3c00, 0x3300, 0x3b00, 0x3400, 0x1700, 0x1a00, 0x1c00, 0x1900,
  0x2900, 0x2a00, 0x2400, 0x2700, 0x3c00, 0x3600, 0x1d00, 0x3100,
  0x3400, 0x3800, 0x2700, 0x3900, 0x2700, 0x2f00, 0x2200, 0x3800,
  0x2500, 0x2d00, 0x2000, 0x3300, 0x2000, 0x2900, 0x1e00, 0x2b00,
  0x2300, 0x1a00, 0x1a00, 0x1b00, 0x1800, 0x1700, 0x1e00, 0x1c00,
  0x1b00, 0x1c00, 0x1b00, 0x1a00, 0x1800, 0x1d00, 0x1b00, 0x1800,
  0x1900, 0x1b00, 0x1a00, 0x1d00, 0x1e00, 0x1f00, 0x1b00, 0x1e00,
  0x1200, 0x1400, 0x1a00, 0x1300, 0x1c00, 0x1b00, 0x1900, 0x2000,
  0x1e00, 0x3000, 0x2900, 0x2d00, 0x2500, 0x1300, 0x1700, 0x1400,
  0x1300, 0x1e00, 0x1f00, 0x1100, 0x1900, 0x2100, 0x1e00, 0x1500,
  0x1a00, 0x3100, 0x2a00, 0x2b00, 0x2800, 0x3800, 0x3a00, 0x2d00,
  0x3a00, 0x2d00, 0x3600, 0x2d00, 0x3a00, 0x2d00, 0x3600, 0x2b00,
  0x3a00, 0x2800, 0x3600, 0x2700, 0x2b00, 0x3000, 0x2500, 0x2f00,
  0x2600, 0x2d00, 0x2400, 0x3000, 0x2500, 0x2b00, 0x2400, 0x2d00,
  0x2500, 0x2800, 0x2500, 0x2a00, 0x2900, 0x2300, 0x2200, 0x1e00,
  0x1b00, 0x1900, 0x2600, 0x2300, 0x1f00, 0x1d00, 0x2200, 0x1b00,
  0x1800, 0x2100, 0x2100, 0x1d00, 0x1d00, 0x1f00, 0x1f00, 0x2900,
  0x2600, 0x2a00, 0x2100, 0x2300, 0x1800, 0x1a00, 0x1d00, 0x2000,
  0x1c00, 0x1a00, 0x1e00, 0x2900, 0x2800, 0x2f00, 0x2300, 0x2f00,
  0x2600, 0x1d00, 0x1700, 0x1d00, 0x1c00, 0x1e00, 0x2100, 0x1700,
  0x2200, 0x2300, 0x2300, 0x1400, 0x1a00, 0x1900, 0x1900, 0x1900,
  0x1b00, 0x1700, 0x1b00, 0x1a00, 0x1000, 0x1900, 0x1600, 0x1800,
  0x1e00, 0x1900, 0x1a00, 0x1700, 0x1b00, 0x1700, 0x1500, 0x1500,
  0x1500, 0x1700, 0x1400, 0x1900, 0x1700, 0x1600, 0x1600, 0x1200,
  0x1300, 0x1200, 0x1600, 0x1500, 0x1500, 0x1300, 0x1600, 0x1600,
  0x1c00, 0x1400, 0x1700, 0x1600, 0x1400, 0x1400, 0x1400, 0x1500,
  0x1400, 0x1300, 0x1300, 0x1500, 0x1800, 0x1600, 0x1f00, 0x1a00,
  0x1e00, 0x1800, 0x1700, 0x1600, 0x1600, 0x1300, 0x1400, 0x1300,
  0x1100, 0x1500, 0x1600, 0x1500, 0x1200, 0x1300, 0x3000, 0x2b00,
  0x2800, 0x2700, 0x3d00, 0x3d00, 0x3500, 0x3e00, 0x3500, 0x3f00,
  0x3b00, 0x3e00, 0x3200, 0x3f00, 0x3a00, 0x3f00, 0x3d00, 0x3f00,
  0x3b00, 0x3f00, 0x3f00, 0x3200, 0x3f00, 0x3500, 0x3e00, 0x3700,
  0x3f00, 0x2d00, 0x3c00, 0x3000, 0x3f00, 0x3700, 0x3e00, 0x3400,
  0x3f00, 0x3900, 0x2600, 0x2f00, 0x1e00, 0x2400, 0x1500, 0x3700,
  0x3100, 0x1b00, 0x2600, 0x2300, 0x3a00, 0x3900, 0x3e00, 0x2b00,
  0x2200, 0x2800, 0x2f00, 0x2500, 0x3e00, 0x3700, 0x3e00, 0x3d00,
  0x3900, 0x1a00, 0x3300, 0x2500, 0x2800, 0x3c00, 0x3800, 0x2c00,
  0x3d00, 0x3800, 0x3f00, 0x3b00, 0x3f00, 0x3a00, 0x1e00, 0x1b00,
  0x1800, 0x1800, 0x3b00, 0x3a00, 0x1200, 0x2f00, 0x3f00, 0x3b00,
  0x1b00, 0x3500, 0x3c00, 0x3e00, 0x3000, 0x3e00, 0x3100, 0x3a00,
  0x3100, 0x3d00, 0x2c00, 0x3900, 0x2e00, 0x3c00, 0x2d00, 0x3c00,
  0x3100, 0x3d00, 0x3100, 0x2100, 0x2c00, 0x2600, 0x2800, 0x1d00,
  0x2b00, 0x2800, 0x2800, 0x2400, 0x2200, 0x2100, 0x2300, 0x2d00,
  0x2500, 0x1f00, 0x2100, 0x2b00, 0x2700, 0x3200, 0x2d00, 0x3400,
  0x2a00, 0x3500, 0x1800, 0x1800, 0x1f00, 0x1e00, 0x2e00, 0x2a00,
  0x2400, 0x3000, 0x2b00, 0x3e00, 0x3d00, 0x3d00, 0x3a00, 0x1e00,
  0x2b00, 0x2600, 0x1900, 0x3400, 0x3500, 0x1c00, 0x2600, 0x3300,
  0x2a00, 0x1c00, 0x2b00, 0x3500, 0x3b00, 0x2900, 0x3b00, 0x2a00,
  0x3100, 0x2700, 0x3b00, 0x2600, 0x2f00, 0x2400, 0x3400, 0x2300,
  0x2d00, 0x2000, 0x3300, 0x2700, 0x1c00, 0x2400, 0x1c00, 0x1c00,
  0x1900, 0x2700, 0x2800, 0x1b00, 0x1d00, 0x2000, 0x1b00, 0x1a00,
  0x2300, 0x1d00, 0x1700, 0x1e00, 0x2400, 0x2100, 0x2b00, 0x2100,
  0x2800, 0x2000, 0x2300, 0x1b00, 0x1500, 0x1b00, 0x1400, 0x1a00,
  0x1a00, 0x2000, 0x2a00, 0x2200, 0x3700, 0x2f00, 0x3200, 0x2a00,
  0x1700, 0x1700, 0x1600, 0x1900, 0x2500, 0x2300, 0x1500, 0x1900,
  0x2500, 0x2200, 0x1400, 0x1b00, 0x2f00, 0x2800, 0x2900, 0x2400,
  0x2d00, 0x2500, 0x2300, 0x2500, 0x2500, 0x2600, 0x2400, 0x3b00,
  0x3c00, 0x3400, 0x3c00, 0x3400, 0x3a00, 0x3000, 0x3c00, 0x3200,
  0x3a00, 0x3100, 0x3c00, 0x3000, 0x3900, 0x2f00, 0x3500, 0x3800,
  0x2c00, 0x3900, 0x2c00, 0x3400, 0x2b00, 0x3800, 0x2e00, 0x3400,
  0x2d00, 0x3600, 0x2a00, 0x3300, 0x2800, 0x3100, 0x3100, 0x2600,
  0x2900, 0x2000, 0x2300, 0x1f00, 0x2d00, 0x2600, 0x2000, 0x2600,
  0x2300, 0x2500, 0x2100, 0x2c00, 0x2400, 0x1d00, 0x2500, 0x2400,
  0x2400, 0x3000, 0x2800, 0x3000, 0x2900, 0x2200, 0x1e00, 0x1c00,
  0x2500, 0x1d00, 0x2300, 0x2300, 0x2500, 0x3300, 0x2c00, 0x3700,
  0x2b00, 0x3400, 0x2c00, 0x1e00, 0x1c00, 0x2100, 0x1b00, 0x2900,
  0x2a00, 0x1d00, 0x2600, 0x3200, 0x2a00, 0x2000, 0x2400, 0x2900,
  0x2e00, 0x2600, 0x2f00, 0x2600, 0x2d00, 0x2600, 0x2e00, 0x2500,
  0x2b00, 0x2600, 0x2f00, 0x2300, 0x2a00, 0x2300, 0x2800, 0x2800,
  0x2100, 0x2400, 0x2000, 0x2000, 0x1b00, 0x2400, 0x1f00, 0x1c00,
  0x2100, 0x2200, 0x1d00, 0x1c00, 0x1f00, 0x1c00, 0x1900, 0x1e00,
  0x2100, 0x2100, 0x2900, 0x2200, 0x2300, 0x2100, 0x1c00, 0x1a00,
  0x1a00, 0x2100, 0x2100, 0x1c00, 0x1c00, 0x1f00, 0x2700, 0x2500,
  0x2d00, 0x2700, 0x2a00, 0x2300, 0x1c00, 0x1d00, 0x1a00, 0x1a00,
  0x1b00, 0x1d00, 0x1800, 0x2000, 0x2300, 0x1f00, 0x1900, 0x1c00,
  0x1c00, 0x1e00, 0x1b00, 0x1e00, 0x1c00, 0x1e00, 0x1900, 0x1a00,
  0x1f00, 0x1f00, 0x1900, 0x2000, 0x1a00, 0x1f00, 0x1700, 0x1b00,
  0x1a00, 0x1900, 0x1800, 0x1900, 0x1800, 0x1600, 0x1900, 0x1a00,
  0x1900, 0x1700, 0x1800, 0x1700, 0x1800, 0x1600, 0x1700, 0x1400,
  0x1600, 0x1800, 0x1a00, 0x1c00, 0x1c00, 0x1c00, 0x1700, 0x1700,
  0x1500, 0x1500, 0x1600, 0x1600, 0x1500, 0x1400, 0x1700, 0x1b00,
  0x1a00, 0x2300, 0x1c00, 0x1d00, 0x1a00, 0x1600, 0x1600, 0x1500,
  0x1400, 0x1800, 0x1500, 0x1300, 0x1700, 0x1900, 0x1600, 0x1400,
  0x1400, 0x3200, 0x2b00, 0x2900, 0x2800, 0x2800, 0x2500, 0x2500,
  0x2700, 0x2500, 0x2600, 0x2500, 0x3d00, 0x3e00, 0x3300, 0x3e00,
  0x3500, 0x3e00, 0x3700, 0x3e00, 0x3400, 0x3e00, 0x3500, 0x3f00,
  0x3d00, 0x3f00, 0x3c00, 0x2e00, 0x2900, 0x2a00, 0x2700, 0x2d00,
  0x2500, 0x2400, 0x2500, 0x2400, 0x2500, 0x2300, 0x2800, 0x2500,
  0x2300, 0x2300, 0x2200, 0x2200, 0x2200, 0x2200, 0x2200, 0x2200,
  0x2200, 0x2100, 0x2000, 0x2200, 0x2100, 0x2000, 0x3b00, 0x3c00,
  0x3400, 0x3c00, 0x3200, 0x3900, 0x2e00, 0x3d00, 0x3400, 0x3900,
  0x2f00, 0x3c00, 0x2d00, 0x3700, 0x2d00, 0x3100, 0x2b00, 0x2a00,
  0x2900, 0x2700, 0x2600, 0x2500, 0x2500, 0x2500, 0x2200, 0x2200,
  0x2200, 0x2300, 0x2300, 0x2300, 0x2200, 0x2300, 0x2200, 0x2300,
  0x2200, 0x2200, 0x2200, 0x2200, 0x2200, 0x2000, 0x2100, 0x2200,
};

Ipp16s bsacNoisePowerModelTab[] = {
  0x3fe0, 0x3fc0, 0x3fa0, 0x3f80, 0x3f60, 0x3f40, 0x3f20, 0x3f00,
  0x3ee0, 0x3ec0, 0x3ea0, 0x3e80, 0x3e60, 0x3e40, 0x3e20, 0x3e00,
  0x3de0, 0x3dc0, 0x3da0, 0x3d80, 0x3d60, 0x3d40, 0x3d20, 0x3d00,
  0x3ce0, 0x3cc0, 0x3ca0, 0x3c80, 0x3c60, 0x3c40, 0x3c20, 0x3c00,
  0x3be0, 0x3bc0, 0x3ba0, 0x3b80, 0x3b60, 0x3b40, 0x3b20, 0x3b00,
  0x3ae0, 0x3ac0, 0x3aa0, 0x3a80, 0x3a60, 0x3a40, 0x3a20, 0x3a00,
  0x39e0, 0x39c0, 0x39a0, 0x3980, 0x3960, 0x3940, 0x3920, 0x3900,
  0x38e0, 0x38c0, 0x38a0, 0x3880, 0x3860, 0x3840, 0x3820, 0x3800,
  0x37e0, 0x37c0, 0x37a0, 0x3780, 0x3760, 0x3740, 0x3720, 0x3700,
  0x36e0, 0x36c0, 0x36a0, 0x3680, 0x3660, 0x3640, 0x3620, 0x3600,
  0x35e0, 0x35c0, 0x35a0, 0x3580, 0x3560, 0x3540, 0x3520, 0x3500,
  0x34e0, 0x34c0, 0x34a0, 0x3480, 0x3460, 0x3440, 0x3420, 0x3400,
  0x33e0, 0x33c0, 0x33a0, 0x3380, 0x3360, 0x3340, 0x3320, 0x3300,
  0x32e0, 0x32c0, 0x32a0, 0x3280, 0x3260, 0x3240, 0x3220, 0x3200,
  0x31e0, 0x31c0, 0x31a0, 0x3180, 0x3160, 0x3140, 0x3120, 0x3100,
  0x30e0, 0x30c0, 0x30a0, 0x3080, 0x3060, 0x3040, 0x3020, 0x3000,
  0x2fe0, 0x2fc0, 0x2fa0, 0x2f80, 0x2f60, 0x2f40, 0x2f20, 0x2f00,
  0x2ee0, 0x2ec0, 0x2ea0, 0x2e80, 0x2e60, 0x2e40, 0x2e20, 0x2e00,
  0x2de0, 0x2dc0, 0x2da0, 0x2d80, 0x2d60, 0x2d40, 0x2d20, 0x2d00,
  0x2ce0, 0x2cc0, 0x2ca0, 0x2c80, 0x2c60, 0x2c40, 0x2c20, 0x2c00,
  0x2be0, 0x2bc0, 0x2ba0, 0x2b80, 0x2b60, 0x2b40, 0x2b20, 0x2b00,
  0x2ae0, 0x2ac0, 0x2aa0, 0x2a80, 0x2a60, 0x2a40, 0x2a20, 0x2a00,
  0x29e0, 0x29c0, 0x29a0, 0x2980, 0x2960, 0x2940, 0x2920, 0x2900,
  0x28e0, 0x28c0, 0x28a0, 0x2880, 0x2860, 0x2840, 0x2820, 0x2800,
  0x27e0, 0x27c0, 0x27a0, 0x2780, 0x2760, 0x2740, 0x2720, 0x2700,
  0x26e0, 0x26c0, 0x26a0, 0x2680, 0x2660, 0x2640, 0x2620, 0x2600,
  0x25e0, 0x25c0, 0x25a0, 0x2580, 0x2560, 0x2540, 0x2520, 0x2500,
  0x24e0, 0x24c0, 0x24a0, 0x2480, 0x2460, 0x2440, 0x2420, 0x2400,
  0x23e0, 0x23c0, 0x23a0, 0x2380, 0x2360, 0x2340, 0x2320, 0x2300,
  0x22e0, 0x22c0, 0x22a0, 0x2280, 0x2260, 0x2240, 0x2220, 0x2200,
  0x21e0, 0x21c0, 0x21a0, 0x2180, 0x2160, 0x2140, 0x2120, 0x2100,
  0x20e0, 0x20c0, 0x20a0, 0x2080, 0x2060, 0x2040, 0x2020, 0x2000,
  0x1fe0, 0x1fc0, 0x1fa0, 0x1f80, 0x1f60, 0x1f40, 0x1f20, 0x1f00,
  0x1ee0, 0x1ec0, 0x1ea0, 0x1e80, 0x1e60, 0x1e40, 0x1e20, 0x1e00,
  0x1de0, 0x1dc0, 0x1da0, 0x1d80, 0x1d60, 0x1d40, 0x1d20, 0x1d00,
  0x1ce0, 0x1cc0, 0x1ca0, 0x1c80, 0x1c60, 0x1c40, 0x1c20, 0x1c00,
  0x1be0, 0x1bc0, 0x1ba0, 0x1b80, 0x1b60, 0x1b40, 0x1b20, 0x1b00,
  0x1ae0, 0x1ac0, 0x1aa0, 0x1a80, 0x1a60, 0x1a40, 0x1a20, 0x1a00,
  0x19e0, 0x19c0, 0x19a0, 0x1980, 0x1960, 0x1940, 0x1920, 0x1900,
  0x18e0, 0x18c0, 0x18a0, 0x1880, 0x1860, 0x1840, 0x1820, 0x1800,
  0x17e0, 0x17c0, 0x17a0, 0x1780, 0x1760, 0x1740, 0x1720, 0x1700,
  0x16e0, 0x16c0, 0x16a0, 0x1680, 0x1660, 0x1640, 0x1620, 0x1600,
  0x15e0, 0x15c0, 0x15a0, 0x1580, 0x1560, 0x1540, 0x1520, 0x1500,
  0x14e0, 0x14c0, 0x14a0, 0x1480, 0x1460, 0x1440, 0x1420, 0x1400,
  0x13e0, 0x13c0, 0x13a0, 0x1380, 0x1360, 0x1340, 0x1320, 0x1300,
  0x12e0, 0x12c0, 0x12a0, 0x1280, 0x1260, 0x1240, 0x1220, 0x1200,
  0x11e0, 0x11c0, 0x11a0, 0x1180, 0x1160, 0x1140, 0x1120, 0x1100,
  0x10e0, 0x10c0, 0x10a0, 0x1080, 0x1060, 0x1040, 0x1020, 0x1000,
  0x0fe0, 0x0fc0, 0x0fa0, 0x0f80, 0x0f60, 0x0f40, 0x0f20, 0x0f00,
  0x0ee0, 0x0ec0, 0x0ea0, 0x0e80, 0x0e60, 0x0e40, 0x0e20, 0x0e00,
  0x0de0, 0x0dc0, 0x0da0, 0x0d80, 0x0d60, 0x0d40, 0x0d20, 0x0d00,
  0x0ce0, 0x0cc0, 0x0ca0, 0x0c80, 0x0c60, 0x0c40, 0x0c20, 0x0c00,
  0x0be0, 0x0bc0, 0x0ba0, 0x0b80, 0x0b60, 0x0b40, 0x0b20, 0x0b00,
  0x0ae0, 0x0ac0, 0x0aa0, 0x0a80, 0x0a60, 0x0a40, 0x0a20, 0x0a00,
  0x09e0, 0x09c0, 0x09a0, 0x0980, 0x0960, 0x0940, 0x0920, 0x0900,
  0x08e0, 0x08c0, 0x08a0, 0x0880, 0x0860, 0x0840, 0x0820, 0x0800,
  0x07e0, 0x07c0, 0x07a0, 0x0780, 0x0760, 0x0740, 0x0720, 0x0700,
  0x06e0, 0x06c0, 0x06a0, 0x0680, 0x0660, 0x0640, 0x0620, 0x0600,
  0x05e0, 0x05c0, 0x05a0, 0x0580, 0x0560, 0x0540, 0x0520, 0x0500,
  0x04e0, 0x04c0, 0x04a0, 0x0480, 0x0460, 0x0440, 0x0420, 0x0400,
  0x03e0, 0x03c0, 0x03a0, 0x0380, 0x0360, 0x0340, 0x0320, 0x0300,
  0x02e0, 0x02c0, 0x02a0, 0x0280, 0x0260, 0x0240, 0x0220, 0x0200,
  0x01e0, 0x01c0, 0x01a0, 0x0180, 0x0160, 0x0140, 0x0120, 0x0100,
  0x00e0, 0x00c0, 0x00a0, 0x0080, 0x0060, 0x0040, 0x0020, 0x0000
};

Ipp16s bsacOffsetsZeroTab1[]  = {0};
Ipp16s bsacOffsetsZeroTab2[]  = {15};
Ipp16s bsacOffsetsZeroTab3[]  = {30,   45};
Ipp16s bsacOffsetsZeroTab4[]  = {111, 126};
Ipp16s bsacOffsetsZeroTab5[]  = {192, 207, 272};
Ipp16s bsacOffsetsZeroTab6[]  = {341, 356, 421};
Ipp16s bsacOffsetsZeroTab7[]  = {490, 505, 570, 635};
Ipp16s bsacOffsetsZeroTab8[]  = {711, 726, 791, 856};
Ipp16s bsacOffsetsZeroTab9[]  = {932, 505, 570, 635};
Ipp16s bsacOffsetsZeroTab10[] = {974, 726, 791, 856};

Ipp16s *bsacOffsetsZero[] = {
  bsacOffsetsZeroTab1,  bsacOffsetsZeroTab2,
  bsacOffsetsZeroTab3,  bsacOffsetsZeroTab4,
  bsacOffsetsZeroTab5,  bsacOffsetsZeroTab6,
  bsacOffsetsZeroTab7,  bsacOffsetsZeroTab8,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab10,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab10,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab10,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9,
  bsacOffsetsZeroTab9,  bsacOffsetsZeroTab9
};

Ipp16s bsacAddOffsetZero[] = {
   0, -1, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1, -1, -1, -1, -1,
  22, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, -1, -1, -1, -1,
  32, -1, -1, -1, -1, -1, -1, -1, 39, -1, -1, -1, -1, -1, -1, -1,
  42, -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   1,  2, -1, -1, -1, -1, -1, -1, 16, 17, -1, -1, -1, -1, -1, -1,
  23, 24, -1, -1, -1, -1, -1, -1, 30, 31, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  46, 46, -1, -1, -1, -1, -1, -1, 53, 53, -1, -1, -1, -1, -1, -1,
  56, 56, -1, -1, -1, -1, -1, -1, 59, 59, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   3,  4,  5,  6, -1, -1, -1, -1, 18, 19, 20, 21, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  33, 33, 34, 34, -1, -1, -1, -1, 40, 40, 41, 41, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  47, 48, 47, 48, -1, -1, -1, -1, 54, 55, 54, 55, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  60, 60, 60, 60, -1, -1, -1, -1, 63, 63, 63, 63, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1, -1, -1, -1, -1, -1,
  25, 25, 26, 26, 27, 27, 28, 28, -1, -1, -1, -1, -1, -1, -1, -1,
  35, 36, 35, 36, 37, 38, 37, 38, -1, -1, -1, -1, -1, -1, -1, -1,
  43, 43, 43, 43, 44, 44, 44, 44, -1, -1, -1, -1, -1, -1, -1, -1,
  49, 50, 51, 52, 49, 50, 51, 52, -1, -1, -1, -1, -1, -1, -1, -1,
  57, 57, 58, 58, 57, 57, 58, 58, -1, -1, -1, -1, -1, -1, -1, -1,
  61, 62, 61, 62, 61, 62, 61, 62, -1, -1, -1, -1, -1, -1, -1, -1,
  64, 64, 64, 64, 64, 64, 64, 64, -1, -1, -1, -1, -1, -1, -1, -1,
};

Ipp16s bsacOffsetsNonZeroTab3[]  = {110};
Ipp16s bsacOffsetsNonZeroTab4[]  = {191};
Ipp16s bsacOffsetsNonZeroTab5[]  = {337, 338};
Ipp16s bsacOffsetsNonZeroTab6[]  = {486, 487};
Ipp16s bsacOffsetsNonZeroTab7[]  = {700, 701, 704};
Ipp16s bsacOffsetsNonZeroTab8[]  = {921, 922, 925};
Ipp16s bsacOffsetsNonZeroTab9[]  = {947, 948, 951, 958};
Ipp16s bsacOffsetsNonZeroTab10[] = {989, 990, 993, 1000};

Ipp16s *bsacOffsetsNonZero[] = {
  bsacOffsetsNonZeroTab3,  bsacOffsetsNonZeroTab4,
  bsacOffsetsNonZeroTab5,  bsacOffsetsNonZeroTab6,
  bsacOffsetsNonZeroTab7,  bsacOffsetsNonZeroTab8,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab10,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab10,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab10,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9,
  bsacOffsetsNonZeroTab9,  bsacOffsetsNonZeroTab9
};

Ipp16s bsacMinP0[] = {
  0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
  0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002
};

Ipp16s bsacMaxP0[] = {
  0x0000, 0x2000, 0x3000, 0x3800, 0x3c00, 0x3e00, 0x3f00,
  0x3f80, 0x3fc0, 0x3fe0, 0x3ff0, 0x3ff8, 0x3ffc, 0x3ffe
};

/********************************************************************/

#endif //UMC_ENABLE_AAC_AUDIO_DECODER


