/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2007 Intel Corporation. All Rights Reserved.
//
*/

#include "umc_defs.h"
#if defined (UMC_ENABLE_MPEG2_VIDEO_ENCODER)

#include "umc_mpeg2_enc_defs.h"
#include "vm_strings.h"
#include <ipps.h>

using namespace UMC;

///* identifies valid profile/level combinations */
//static Ipp8u Profile_Level_Defined[5][4] =
//{
///* HL   H-14 ML   LL  */
//  {1,   1,   1,   0},  /* HP   */
//  {0,   1,   0,   0},  /* Spat */
//  {0,   0,   1,   1},  /* SNR  */
//  {1,   1,   1,   1},  /* MP   */
//  {0,   0,   1,   0}   /* SP   */
//};

static struct LevelLimits
{
  Ipp32s f_code[2];
  Ipp32s hor_size;
  Ipp32s vert_size;
  Ipp32s sample_rate;
  Ipp32s bit_rate;         /* Mbit/s */
  Ipp32s vbv_buffer_size;  /* 16384 bit steps */
} MaxValTbl[5] =
{
  {{9, 5}, 1920, 1152, 62668800, 80, 597}, /* HL */
  {{9, 5}, 1440, 1152, 47001600, 60, 448}, /* H-14 */
  {{8, 5},  720,  576, 10368000, 15, 112}, /* ML */
  {{7, 4},  352,  288,  3041280,  4,  29}, /* LL */
  {{9, 9}, 8192, 8192, 0x7fffffff, 1000,  10000}  /* some limitations for unlimited */
};

#define SP   5
#define MP   4
#define SNR  3
#define SPAT 2
#define HP   1

#define LL  10
#define ML   8
#define H14  6
#define HL   4

/* chroma_format for MPEG2EncoderParams::chroma_format*/
#define CHROMA420 1
#define CHROMA422 2
#define CHROMA444 3

#define error(Message) \
{ \
  vm_debug_trace(VM_DEBUG_ERROR, VM_STRING(Message)); \
  return UMC_ERR_FAILED; \
}

/* ISO/IEC 13818-2, 6.3.11 */
/*  this pretty table was designed to
    avoid coincidences with any GPL code */
static VM_ALIGN16_DECL(Ipp16s) DefaultIntraQuantMatrix[64] =
{
     8,
     16,

     19,        22, 26, 27,             29, 34, 16,
     16,        22,         24,         27,         29,
     34,        37,             19,     22,             26,
     27,        29,             34,     34,             38,
     22,        22,             26,     27,             29,
     34,        37,             40,     22,             26,
     27,        29,         32,         35,         40,
     48,        26, 27, 29,             32, 35, 40,
     48,        58,                     26,
     27,        29,                     34,
     38,        46,                     56,
     69,        27,                     29,
     35,        38,                     46,
     56,        69,                     83
};

static const Ipp64f ratetab[8]=
    {24000.0/1001.0,24.0,25.0,30000.0/1001.0,30.0,50.0,60000.0/1001.0,60.0};
static const Ipp32s aspecttab[3][2] =
    {{4,3},{16,9},{221,100}};

MPEG2EncoderParams::MPEG2EncoderParams()
{
  Ipp32s i;

  *IntraQMatrix = 0;
  *NonIntraQMatrix = 0;

  lFlags = FLAG_VENC_REORDER;
  info.bitrate = 5000000;
  info.framerate = 30;
  numEncodedFrames = 0;
  qualityMeasure = -1;

  info.interlace_type = PROGRESSIVE;          // progressive sequence
  //progressive_frame = 1; // progressive frame
  CustomIntraQMatrix = 0;
  CustomNonIntraQMatrix = 0;
  for(i=0; i<64; i++) {   // for reconstruction
    IntraQMatrix[i] = DefaultIntraQuantMatrix[i];
    NonIntraQMatrix[i] = 16;
  }
  IPDistance = 3;         // distance between key-frames
  gopSize = 15;           // size of GOP
  info.framerate = 30;
  info.aspect_ratio_width = 4;
  info.aspect_ratio_height = 3;
  profile = MP;
  level = ML;
  info.color_format = YUV420;
  //repeat_first_field = 0;
  //top_field_first = 0;    // display top field first
  //intra_dc_precision = 0; // 8 bit
  FieldPicture = 0;       // field or frame picture (if progframe=> frame)
  VBV_BufferSize = 112;
  low_delay = 0;
  frame_pred_frame_dct[0] = 1;
  frame_pred_frame_dct[1] = 1;
  frame_pred_frame_dct[2] = 1;
  intraVLCFormat[0] = 1;
  intraVLCFormat[1] = 1;
  intraVLCFormat[2] = 1;
  altscan_tab[0] = 0;
  altscan_tab[1] = 0;
  altscan_tab[2] = 0;
  mpeg1 = 0;               // 1 - mpeg1 (unsupported), 0 - mpeg2
  *idStr = 0;              // user data to put to each sequence
  UserData = idStr;
  UserDataLen = 0;
  numThreads = 1;
  performance = 0;
  encode_time = 0;
  motion_estimation_perf = 0;
  me_alg_num = 3;
  me_auto_range = 1;
  allow_prediction16x8 = 0;
  rc_mode = RC_CBR;
  quant_vbr[0] = 0;
  quant_vbr[1] = 0;
  quant_vbr[2] = 0;

  rangeP[0] = 8*IPDistance;
  rangeP[1] = 4*IPDistance;
  rangeB[0][0] = rangeB[1][0] = rangeP[0] >> 1;
  rangeB[0][1] = rangeB[1][1] = rangeP[1] >> 1;

}

MPEG2EncoderParams::~MPEG2EncoderParams()
{
}

Status MPEG2EncoderParams::ReadQMatrices(vm_char* IntraQMatrixFName, vm_char* NonIntraQMatrixFName)
{
  Ipp32s i, temp;
  vm_file *InputFile;

  if( IntraQMatrixFName==0 || IntraQMatrixFName[0] == 0 || IntraQMatrixFName[0] == '-' )
  {
    // use default intra matrix
    CustomIntraQMatrix = 0;
    for(i=0; i<64; i++)
      IntraQMatrix[i] = DefaultIntraQuantMatrix[i];
  }
  else
  {
    // load custom intra matrix
    CustomIntraQMatrix = 1;
    if( 0 == (InputFile = vm_file_open(IntraQMatrixFName,VM_STRING("rt"))) )
    {
      vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("Can't open quant matrix file %s\n"), IntraQMatrixFName);
      return UMC_ERR_OPEN_FAILED;
    }

    for(i=0; i<64; i++)
    {
      vm_file_fscanf( InputFile, VM_STRING("%d"), &temp );
      if( temp < 1 || temp > 255 )
        error("invalid value in quant matrix\n");
      IntraQMatrix[i] = (Ipp16s)temp;
    }

    vm_file_fclose( InputFile );
  }

  if (NonIntraQMatrixFName == 0 || NonIntraQMatrixFName[0] == 0 || NonIntraQMatrixFName[0] == '-')
  {
    // use default non-intra matrix
    CustomNonIntraQMatrix = 0;
    for(i=0; i<64; i++)
      NonIntraQMatrix[i] = 16;
  }
  else
  {
    // load custom non-intra matrix
    CustomNonIntraQMatrix = 1;
    if( 0 == (InputFile = vm_file_open(NonIntraQMatrixFName,VM_STRING("rt"))) )
    {
      vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("Couldn't open quant matrix file %s\n"), NonIntraQMatrixFName);
      return UMC_ERR_OPEN_FAILED;
    }

    for(i=0; i<64; i++)
    {
      vm_file_fscanf( InputFile, VM_STRING("%d"), &temp );
      if( temp < 1 || temp > 255 )
        error("invalid value in quant matrix\n");
      NonIntraQMatrix[i] = (Ipp16s)temp;
    }

    vm_file_fclose( InputFile );
  }

  return UMC_OK;
}

Status MPEG2EncoderParams::Profile_and_Level_Checks()
{
  Ipp32s i,j,k;
  struct LevelLimits *MaxVal;
  Ipp32s newLevel = level;
  Ipp32s newProfile = profile;
  Status ret = UMC_OK;

  if( profile == SNR || profile == SPAT )
    error("The encoder doesn't support scalable profiles\n");

  // check level, select appropriate when it is wrong
  if( newLevel < HL ||
    newLevel > LL ||
    newLevel & 1 )
    newLevel = LL;
  if(info.color_format != YUV420 && newLevel == LL )
    newLevel = ML;
  if( info.framerate > ratetab[5 - 1] && newLevel >= ML )
    newLevel = H14;

  MaxVal = &MaxValTbl[(newLevel - 4) >> 1];
  while( info.clip_info.width > MaxVal->hor_size ||
         info.clip_info.height > MaxVal->vert_size ||
         info.clip_info.width * info.clip_info.height * info.framerate > MaxVal->sample_rate ||
         info.bitrate > 1000000 * (Ipp32u)MaxVal->bit_rate )
  {
    if(newLevel > HL) {
      newLevel -= 2;
      MaxVal --;
    } else {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("encoding parameters exceed highest level limitations\n"));
      MaxVal = &MaxValTbl[4];
      break;
    }
  }

  // check profile values
  if( newProfile < HP || newProfile > SP ) {
    newProfile = MP;
  }
  if(info.color_format != YUV420) {
    newProfile = HP;
    if(info.color_format == YUV444) {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("CHROMA444 has no appropriate prifile\n"));
    }
  }

  // check profile constraints
  if( newProfile == SP && IPDistance != 1 ) {
    newProfile = MP;
  }

  // check profile - level combination
  if(newProfile == SP && newLevel != ML)
    newProfile = MP;
  if(newProfile == HP && newLevel == LL) {
    newLevel = ML;
    MaxVal = &MaxValTbl[(newLevel - 4) >> 1];
  }

  //if( newProfile != HP && intra_dc_precision == 3 )
  //  newProfile = HP;

  //// SP, MP: constrained repeat_first_field
  //if( newProfile >= MP && repeat_first_field &&
  //    ( info.framerate <= ratetab[2 - 1] ||
  //      info.framerate <= ratetab[6 - 1] && info.interlace_type == PROGRESSIVE ) ) {
  //  vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("repeat_first_field set to zero\n"));
  //  repeat_first_field = 0;
  //}

  if(IPDistance < 1) IPDistance = 1;
  if(gopSize < 1)    gopSize = 1;
  if(gopSize > 132)  gopSize = 132;
  if(IPDistance > gopSize) IPDistance = gopSize;

  // compute f_codes from ranges, check limits, extend to the f_code limit
  for(i = 0; i < IPP_MIN(IPDistance,2); i++) // P, B
    for(k=0; k<2; k++) {                     // FW, BW
      Ipp32s* prange;
      if (i == 0) {
        if( k==1)
          continue; // no backward for P-frame
        prange = rangeP;
      } else {
        prange = rangeB[k];
      }
      for(j=0; j<2; j++) {                   // x, y
        Ipp32s req_f_code;
        RANGE_TO_F_CODE(prange[j], req_f_code);
        if( req_f_code > MaxVal->f_code[j] ) {
          vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("search range is greater than permitted in specified Level\n"));
          req_f_code = MaxVal->f_code[j];
          prange[j] = 4<<req_f_code;
        }
        if (i==0) // extend range only for P
          prange[j] = 4<<req_f_code;
      }
    }

  // Table 8-13
  if( VBV_BufferSize > MaxVal->vbv_buffer_size )
    VBV_BufferSize = MaxVal->vbv_buffer_size;

  if(newLevel != level) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("Level changed\n"));
    level = newLevel;
  }
  if(newProfile != profile) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("Profile changed\n"));
    profile = newProfile;
  }

  return ret;
}

Status MPEG2EncoderParams::RelationChecks()
{
  Status ret = UMC_OK;

  if( mpeg1 ) {
    if(info.interlace_type != PROGRESSIVE)
    {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting progressive_sequence = 1\n"));
      info.interlace_type = PROGRESSIVE;
    }
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("MPEG1 is not implemented. Setting to MPEG2\n"));
    mpeg1 = 0;
  }

  //if(aspectRatio < 1 || aspectRatio > 15) {
  //  vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting aspect ratio to 1\n"));
  //  aspectRatio = 1;
  //}

  //progressive_frame = (progressive_frame != 0) ? 1 : 0;
  //if( info.interlace_type == PROGRESSIVE && !progressive_frame )
  //{
  //  vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting progressive_frame = 1\n"));
  //  progressive_frame = 1;
  //}

  //repeat_first_field = (repeat_first_field != 0) ? 1 : 0;
  ////if( !progressive_frame && repeat_first_field )
  //if( info.interlace_type != PROGRESSIVE && repeat_first_field )
  //{
  //  vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting repeat_first_field = 0\n"));
  //  repeat_first_field = 0;
  //}

  frame_pred_frame_dct[0] = (frame_pred_frame_dct[0] != 0) ? 1 : 0;
  frame_pred_frame_dct[1] = (frame_pred_frame_dct[1] != 0) ? 1 : 0;
  frame_pred_frame_dct[2] = (frame_pred_frame_dct[2] != 0) ? 1 : 0;

  //if( progressive_frame )
  if( info.interlace_type == PROGRESSIVE )
  {
    if( FieldPicture )
    {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting FieldPicture = 0\n"));
      FieldPicture = 0;
    }
    if( !frame_pred_frame_dct[0] )
    {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting frame_pred_frame_dct[I_PICTURE] = 1\n"));
      frame_pred_frame_dct[0] = 1;
    }
    if( !frame_pred_frame_dct[1] )
    {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting frame_pred_frame_dct[P_PICTURE] = 1\n"));
      frame_pred_frame_dct[1] = 1;
    }
    if( !frame_pred_frame_dct[2] )
    {
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting frame_pred_frame_dct[B_PICTURE] = 1\n"));
      frame_pred_frame_dct[2] = 1;
    }
  }

  //top_field_first = (top_field_first != 0) ? 1 : 0;
  //if (info.interlace_type == PROGRESSIVE && !repeat_first_field && top_field_first)
  //{
  //  vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting top_field_first = 1\n"));
  //  top_field_first = 0;
  //}

  if(info.color_format < YUV420 || info.color_format > YUV444) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("color_format fixed to YUV420\n"));
    info.color_format = YUV420;
  }

  return ret;
}

Status MPEG2EncoderParams::ReadOldParamFile(const vm_char *ParFileName)
{
  Ipp32s i, j;
  vm_file *InputFile;
  vm_char line[PAR_STRLEN];
  vm_char IntraQMatrixFName[PAR_STRLEN];
  vm_char NonIntraQMatrixFName[PAR_STRLEN];
  vm_char TemplateLogFile[PAR_STRLEN];
  Ipp32s LogMask;
  Status ret, ret2;
  //Ipp32s dst_width, dst_height; // not used
  Ipp32s frame_rate_code, aspect_code;
  Ipp32s prog_seq;

  info.color_format = YUV420;
  profile = MP;
  level   = ML;
  //repeat_first_field = 0;

  if( 0 == (InputFile = vm_file_open(ParFileName,VM_STRING("rt"))) )
  {
    vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("Couldn't open parameter file %s\n"), ParFileName);
    return UMC_ERR_OPEN_FAILED;
  }

  vm_file_fgets( idStr, PAR_STRLEN, InputFile );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); //vm_string_sscanf(line, VM_STRING("%255s"), SrcFileName );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%255s"), IntraQMatrixFName );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%255s"), NonIntraQMatrixFName );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); //vm_string_sscanf(line, VM_STRING("%d"), &numFramesToEncode );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &gopSize );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &IPDistance);
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &info.clip_info.width );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &info.clip_info.height );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &aspect_code );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &frame_rate_code );

  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &info.bitrate );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d %d %d"), &quant_vbr[0],
    &quant_vbr[1],
    &quant_vbr[2] );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); //vm_string_sscanf(line, VM_STRING("%d"), &dst_width );
  vm_file_fgets( line, PAR_STRLEN, InputFile ); //vm_string_sscanf(line, VM_STRING("%d"), &dst_height );

  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d %d %d"), &frame_pred_frame_dct[0],
    &frame_pred_frame_dct[1],
    &frame_pred_frame_dct[2] );

  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d"), &prog_seq );
  info.interlace_type = prog_seq ? PROGRESSIVE : INTERLEAVED_TOP_FIELD_FIRST;

  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d %d %d"),    &intraVLCFormat[0],
    &intraVLCFormat[1],
    &intraVLCFormat[2] );
  line[0]=0;
  vm_file_fgets( line, PAR_STRLEN, InputFile ); vm_string_sscanf(line, VM_STRING("%d %255s"), &LogMask,TemplateLogFile);

  if( gopSize < 1 )
    error("N must be positive\n");
  if( IPDistance < 1 )
    error("M must be positive\n");
  if( gopSize%IPDistance != 0 )
    error("N must be an integer multiple of M\n");
  if( IPDistance > 128 )
    error("M should not be more than 128\n");

  for (i = 0; i < IPDistance; i++) {
    Ipp32s f0, f1;
    for (j=0; j<2; j++) { // fwd/bwd
      if (i==0) {
        if (j!=0) continue;
        vm_file_fgets( line, PAR_STRLEN, InputFile );
        vm_string_sscanf(line, VM_STRING("%d %d %d %d"), &f0, &f1,
          &rangeP[0], &rangeP[1] );
      } else {
        vm_file_fgets( line, PAR_STRLEN, InputFile );
        if (i < 3)
          vm_string_sscanf(line, VM_STRING("%d %d %d %d"), &f0, &f1,
            &rangeB[j][0], &rangeB[j][1] );
      }
    }
  }

  vm_file_fclose( InputFile );

  if(frame_rate_code < 1 || frame_rate_code > 8) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting frame rate to 30 fps\n"));
    frame_rate_code = 5;
  }
  info.framerate = ratetab[frame_rate_code - 1];

  if(aspect_code < 1 || aspect_code > 4) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting aspect ratio to 4:3\n"));
    aspect_code = 2;
  }
  if(aspect_code == 1) {
    info.aspect_ratio_width = info.clip_info.width;
    info.aspect_ratio_height = info.clip_info.height;
  } else {
    info.aspect_ratio_width = aspecttab[aspect_code-2][0];
    info.aspect_ratio_height = aspecttab[aspect_code-2][1];
  }

  // check parameters correctness
  ret = Profile_and_Level_Checks();
  if (ret < 0) return ret;
  ret2 = RelationChecks();
  if (ret2 < 0) return ret2;
  if (ret2 != UMC_OK) ret = ret2;
  ret2 = ReadQMatrices(IntraQMatrixFName, NonIntraQMatrixFName);
  if (ret2 < 0) return ret2;
  if (ret2 != UMC_OK) ret = ret2;

  return ret;
}

Status MPEG2EncoderParams::ReadParamFile(const vm_char *ParFileName)
{
  Ipp32s i, j;
  Ipp32s h, m, s, f;
  vm_file *InputFile;
  vm_char line[PAR_STRLEN];
  vm_char IntraQMatrixFName[PAR_STRLEN];
  vm_char NonIntraQMatrixFName[PAR_STRLEN];
  vm_char TemplateLogFile[PAR_STRLEN];
  //Ipp32s P;
  //Ipp32s r,Xi,Xb,Xp,d0i,d0p,d0b; /* rate control */
  //Ipp64f avg_act; /* rate control */
  //vm_char tplref[PAR_STRLEN]; // pattern for reconstructed frames. unused
  Ipp64f tmp;
  Status ret, ret2;
  //Ipp32s numStartFrame, dst_width, dst_height; // not used
  Ipp32s frame_rate_code, chroma_format_code, aspect_code;
  Ipp32s prog_seq, top_field_first;

  if (0 == (InputFile = vm_file_open(ParFileName, VM_STRING("rt"))))
  {
    vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("Can't open parameter file %s\n"), ParFileName);
    return UMC_ERR_OPEN_FAILED;
  }

  vm_file_fgets(idStr, PAR_STRLEN,InputFile);
  if( vm_string_strstr(idStr, VM_STRING("Intel IPP")) != NULL ||
      vm_string_strstr(idStr, VM_STRING("UMC")) != NULL ) {
    vm_file_fclose(InputFile);
    return ReadOldParamFile(ParFileName);
  }
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%255s"),SrcFileName);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%255s"),tplref);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%255s"),IntraQMatrixFName);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%255s"),NonIntraQMatrixFName);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%255s"),TemplateLogFile);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&inputtype);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&numFramesToEncode);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&numStartFrame);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d:%d:%d:%d"),&h,&m,&s,&f);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&gopSize);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&IPDistance);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&mpeg1);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&FieldPicture);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&info.clip_info.width);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&info.clip_info.height);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&aspect_code);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&frame_rate_code);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%lf"),&tmp); info.bitrate = (Ipp32s)tmp;

  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&VBV_BufferSize);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&low_delay);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&constrparms);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&profile);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&level);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&prog_seq);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&chroma_format_code);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&video_format);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&color_primaries);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&transfer_characteristics);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&matrix_coefficients);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&dst_width);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&dst_height);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&intra_dc_precision);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d"),&top_field_first);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d %d %d"),
    frame_pred_frame_dct,frame_pred_frame_dct+1,frame_pred_frame_dct+2);

  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d %d %d"),
    conceal_tab,conceal_tab+1,conceal_tab+2);

  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d %d %d"),
    //nonLinearQScale,nonLinearQScale+1,nonLinearQScale+2);

  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d %d %d"),
    intraVLCFormat,intraVLCFormat+1,intraVLCFormat+2);
  vm_file_fgets(line,PAR_STRLEN,InputFile); vm_string_sscanf(line, VM_STRING("%d %d %d"),
    altscan_tab,altscan_tab+1,altscan_tab+2);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&repeat_first_field);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&progressive_frame);

  /* intra slice interval refresh period */
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&P);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&r);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%lf"),&avg_act);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&Xi);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&Xp);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&Xb);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&d0i);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&d0p);
  vm_file_fgets(line,PAR_STRLEN,InputFile); //vm_string_sscanf(line, VM_STRING("%d"),&d0b);

  if( gopSize < 1 )
    error("N must be positive\n");
  if( IPDistance < 1 )
    error("M must be positive\n");
  if( gopSize%IPDistance != 0 )
    error("N must be an integer multiple of M\n");
  if( IPDistance > 128 )
    error("M should not be more than 128\n");

  switch(chroma_format_code) {
    case CHROMA420: info.color_format = YUV420; break;
    case CHROMA422: info.color_format = YUV422; break;
    case CHROMA444: info.color_format = YUV444; break;
    default:
      vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("chroma_format fixed to CHROMA420\n"));
      chroma_format_code = CHROMA420;
      info.color_format = YUV420;
  }

  for (i = 0; i < IPDistance; i++) {
    Ipp32s f0, f1;
    for (j=0; j<2; j++) { // fwd/bwd
      if (i==0) {
        if (j!=0) continue;
        vm_file_fgets( line, PAR_STRLEN, InputFile );
        vm_string_sscanf(line, VM_STRING("%d %d %d %d"), &f0, &f1,
          &rangeP[0], &rangeP[1] );
      } else {
        vm_file_fgets( line, PAR_STRLEN, InputFile );
        if (i < 3)
          vm_string_sscanf(line, VM_STRING("%d %d %d %d"), &f0, &f1,
          &rangeB[j][0], &rangeB[j][1] );
      }
    }
  }

  vm_file_fclose( InputFile );

  for (i=0; i<3; i++)
  {
    frame_pred_frame_dct[i] = !!frame_pred_frame_dct[i];
    conceal_tab[i] = !!conceal_tab[i];
    intraVLCFormat[i] = !!intraVLCFormat[i];
    altscan_tab[i] = !!altscan_tab[i];
  }

  if(aspect_code < 1 || aspect_code > 4) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting aspect ratio to 4:3\n"));
    aspect_code = 2;
  }
  if(aspect_code == 1) {
    info.aspect_ratio_width = info.clip_info.width;
    info.aspect_ratio_height = info.clip_info.height;
  } else {
    info.aspect_ratio_width = aspecttab[aspect_code-2][0];
    info.aspect_ratio_height = aspecttab[aspect_code-2][1];
  }

  if(frame_rate_code < 1 || frame_rate_code > 8) {
    vm_debug_trace(VM_DEBUG_WARNING, VM_STRING("setting frame rate to 30 fps\n"));
    frame_rate_code = 5;
  }
  info.framerate = ratetab[frame_rate_code - 1];

  info.interlace_type =
    prog_seq ? PROGRESSIVE :
    top_field_first ? INTERLEAVED_TOP_FIELD_FIRST : INTERLEAVED_BOTTOM_FIELD_FIRST;

  // check parameters correctness
  ret = Profile_and_Level_Checks();
  if (ret < 0) return ret;
  ret2 = RelationChecks();
  if (ret2 < 0) return ret2;
  if (ret2 != UMC_OK) ret = ret2;
  ret2 = ReadQMatrices(IntraQMatrixFName, NonIntraQMatrixFName);
  if (ret2 < 0) return ret2;
  if (ret2 != UMC_OK) ret = ret2;

  return ret;
}

Status UMC::ReadParamList(MPEG2EncoderParams* par, ParamList* lst)
{
  Status sts; // checked rarely - dst is not modified if failed
  Ipp32s i;

  if(par == 0 || lst == 0)
    return UMC_ERR_NULL_PTR;

  // VideoEncoderParams:
  ReadParamList((VideoEncoderParams*)par, lst);

  // MPEG2EncoderParams:
  sts = lst->getValue(VM_STRING("GOPsize"),&par->gopSize);
  sts = lst->getValue(VM_STRING("IPdistance"),&par->IPDistance);
  if (UMC_OK <= lst->getValue(VM_STRING("field"))) par->FieldPicture = 1;
  for(i=0; i<3; i++) {
    sts = lst->getValue(VM_STRING("framemode"), &par->frame_pred_frame_dct[i], i);
    sts = lst->getValue(VM_STRING("intraVLC"), &par->intraVLCFormat[i], i);
    sts = lst->getValue(VM_STRING("scan"), &par->altscan_tab[i], i);
    sts = lst->getValue(VM_STRING("quant"), &par->quant_vbr[i], i);
  }
  sts = lst->getValue(VM_STRING("ME"),&par->me_alg_num);
  if (UMC_OK <= lst->getValue(VM_STRING("fixrange"))) par->me_auto_range = 1;
  if (UMC_OK <= lst->getValue(VM_STRING("use16x8"))) par->allow_prediction16x8 = 1;

  const vm_char* ptmp;
  sts = lst->getValue(VM_STRING("rc"), &ptmp);
  if (sts >= UMC_OK) {
    if(0 == vm_string_stricmp(ptmp, VM_STRING("CBR")))
      par->rc_mode = RC_CBR;
    else if(0 == vm_string_stricmp(ptmp, VM_STRING("VBR")))
      par->rc_mode = RC_VBR;
    else if(0 == vm_string_stricmp(ptmp, VM_STRING("UVBR")))
      par->rc_mode = RC_UVBR;
  }

  sts = lst->getValue(VM_STRING("rP"),&par->rangeP[0], 0);
  sts = lst->getValue(VM_STRING("rP"),&par->rangeP[0], 1);
  par->rangeB[0][0] = par->rangeB[1][0] = par->rangeP[0]>>1;
  par->rangeB[0][1] = par->rangeB[1][1] = par->rangeP[1]>>1;
  sts = lst->getValue(VM_STRING("rB"),&par->rangeB[0][0], 0);
  sts = lst->getValue(VM_STRING("rB"),&par->rangeB[0][1], 1);
  sts = lst->getValue(VM_STRING("rB"),&par->rangeB[1][0], 2);
  sts = lst->getValue(VM_STRING("rB"),&par->rangeB[1][1], 3);

  return UMC_OK;
}

const ParamList::OptionInfo UMC::MPEG2EncoderOptions[] = {
  {VM_STRING("GOPsize"), 0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 132"), VM_STRING("group of pictures size")},
  {VM_STRING("IPdistance"), 0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 132"), VM_STRING("distance between I or P frames")},
  {VM_STRING("field"),     0, 1, ParamList::argOpt, ParamList::checkNone, 0, VM_STRING("field coding if interlaced")},
  {VM_STRING("framemode"), 0, 3, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0 1"), VM_STRING("frame prediction and DCT for IPB")},
  {VM_STRING("intraVLC"),  0, 3, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0 1"), VM_STRING("intra VLC format for IPB")},
  {VM_STRING("scan"),      0, 3, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0 1"), VM_STRING("scan matrix for IPB")},
  {VM_STRING("ME"),        0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 19"), VM_STRING("ME algorithm 1-local,2-log,3-both,9-full search,+10-fullpix")},
  {VM_STRING("fixrange"),  0, 1, ParamList::argOpt, ParamList::checkNone, 0, VM_STRING("don\'t adjust search range")},
  {VM_STRING("use16x8"),   0, 1, ParamList::argOpt, ParamList::checkNone, 0, VM_STRING("enable 16x8 mode in field pictures")},
  {VM_STRING("rc"),        0, 1, ParamList::argStr, ParamList::checkSet, VM_STRING("CBR VBR UVBR"), VM_STRING("rate control mode: constant, restricted, unrestricted")},
  {VM_STRING("quant"),     0, 3, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 112"), VM_STRING("IPB constant quantizers for UVBR or maximum for VBR")},
  {VM_STRING("rP"),        0, 2, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0,512"), VM_STRING("search range for P frames x,y")},
  {VM_STRING("rB"),        0, 4, ParamList::argInt, ParamList::checkMinMax, VM_STRING("0,512"), VM_STRING("search range for B frames Fx,Fy,Bx,By")},
  {0,} // list terminator
};

#endif // UMC_ENABLE_MPEG2_VIDEO_ENCODER
