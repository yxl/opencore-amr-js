/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
/**
 *  @file pvmi_kvp.h
 *  @brief This files defines a key-value pair type for PVMI component capability exchange
 */

#ifndef PVMI_KVP_H_INCLUDED
#define PVMI_KVP_H_INCLUDED

typedef void* PvmiMIOSession;
typedef char* PvmiKeyType; //Mime String

typedef enum
{
    PVMI_KVPTYPE_UNKNOWN,
    PVMI_KVPTYPE_VALUE,
    PVMI_KVPTYPE_POINTER,
    PVMI_KVPTYPE_AGGREGATE
} PvmiKvpType;

#define PVMI_KVPTYPE_STRING "type="
static const char PVMI_KVPTYPE_STRING_CONSTCHAR[] = PVMI_KVPTYPE_STRING;

#define PVMI_KVPTYPE_VALUE_STRING "value"
static const char PVMI_KVPTYPE_VALUE_STRING_CONSTCHAR[] = PVMI_KVPTYPE_VALUE_STRING;

#define PVMI_KVPTYPE_POINTER_STRING "pointer"
static const char PVMI_KVPTYPE_POINTER_STRING_CONSTCHAR[] = PVMI_KVPTYPE_POINTER_STRING;

#define PVMI_KVPTYPE_AGGREGATE_STRING "aggregate"
static const char PVMI_KVPTYPE_AGGREGATE_STRING_CONSTCHAR[] = PVMI_KVPTYPE_AGGREGATE_STRING;

#include "oscl_string_containers.h"

typedef enum
{
    PVMI_KVPATTR_UNKNOWN,
    PVMI_KVPATTR_CAP,
    PVMI_KVPATTR_DEF,
    PVMI_KVPATTR_CUR
} PvmiKvpAttr;

#define PVMI_KVPATTR_STRING "attr="
static const char PVMI_KVPATTR_STRING_CONSTCHAR[] = PVMI_KVPATTR_STRING;

#define PVMI_KVPATTR_CAP_STRING "cap"
static const char PVMI_KVPATTR_CAP_STRING_CONSTCHAR[] = PVMI_KVPATTR_CAP_STRING;

#define PVMI_KVPATTR_DEF_STRING "def"
static const char PVMI_KVPATTR_DEF_STRING_CONSTCHAR[] = PVMI_KVPATTR_DEF_STRING;

#define PVMI_KVPATTR_CUR_STRING "cur"
static const char PVMI_KVPATTR_CUR_STRING_CONSTCHAR[] = PVMI_KVPATTR_CUR_STRING;

typedef enum
{
    PVMI_KVPVALTYPE_UNKNOWN,
    PVMI_KVPVALTYPE_BOOL,
    PVMI_KVPVALTYPE_FLOAT,
    PVMI_KVPVALTYPE_DOUBLE,
    PVMI_KVPVALTYPE_UINT8,
    PVMI_KVPVALTYPE_INT32,
    PVMI_KVPVALTYPE_UINT32,
    PVMI_KVPVALTYPE_INT64,
    PVMI_KVPVALTYPE_UINT64,
    PVMI_KVPVALTYPE_WCHARPTR,
    PVMI_KVPVALTYPE_CHARPTR,
    PVMI_KVPVALTYPE_UINT8PTR,
    PVMI_KVPVALTYPE_INT32PTR,
    PVMI_KVPVALTYPE_UINT32PTR,
    PVMI_KVPVALTYPE_INT64PTR,
    PVMI_KVPVALTYPE_UINT64PTR,
    PVMI_KVPVALTYPE_FLOATPTR,
    PVMI_KVPVALTYPE_DOUBLEPTR,
    PVMI_KVPVALTYPE_KSV,
    PVMI_KVPVALTYPE_PKVP,
    PVMI_KVPVALTYPE_PPKVP,
    PVMI_KVPVALTYPE_RANGE_FLOAT,
    PVMI_KVPVALTYPE_RANGE_DOUBLE,
    PVMI_KVPVALTYPE_RANGE_UINT8,
    PVMI_KVPVALTYPE_RANGE_INT32,
    PVMI_KVPVALTYPE_RANGE_UINT32,
    PVMI_KVPVALTYPE_RANGE_INT64,
    PVMI_KVPVALTYPE_RANGE_UINT64,
    PVMI_KVPVALTYPE_BITARRAY32,
    PVMI_KVPVALTYPE_BITARRAY64
} PvmiKvpValueType;


#define PVMI_KVP_SEMICOLON_STRING ";"
static const char PVMI_KVP_SEMICOLON_STRING_CONSTCHAR[] = ";";

#define PVMI_KVPVALTYPE_STRING "valtype="
static const char PVMI_KVPVALTYPE_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_STRING;

#define PVMI_KVPVALTYPE_BOOL_STRING "bool"
static const char PVMI_KVPVALTYPE_BOOL_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_BOOL_STRING;

#define PVMI_KVPVALTYPE_FLOAT_STRING "float"
static const char PVMI_KVPVALTYPE_FLOAT_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_FLOAT_STRING;

#define PVMI_KVPVALTYPE_DOUBLE_STRING "double"
static const char PVMI_KVPVALTYPE_DOUBLE_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_DOUBLE_STRING;

#define PVMI_KVPVALTYPE_UINT8_STRING "uint8"
static const char PVMI_KVPVALTYPE_UINT8_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT8_STRING;

#define PVMI_KVPVALTYPE_INT32_STRING "int32"
static const char PVMI_KVPVALTYPE_INT32_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_INT32_STRING;

#define PVMI_KVPVALTYPE_UINT32_STRING "uint32"
static const char PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT32_STRING;

#define PVMI_KVPVALTYPE_INT64_STRING "int64"
static const char PVMI_KVPVALTYPE_INT64_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_INT64_STRING;

#define PVMI_KVPVALTYPE_UINT64_STRING "uint64"
static const char PVMI_KVPVALTYPE_UINT64_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT64_STRING;

#define PVMI_KVPVALTYPE_WCHARPTR_STRING "wchar*"
static const char PVMI_KVPVALTYPE_WCHARPTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_WCHARPTR_STRING;

#define PVMI_KVPVALTYPE_CHARPTR_STRING "char*"
static const char PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_CHARPTR_STRING;

#define PVMI_KVPVALTYPE_UINT8PTR_STRING "uint8*"
static const char PVMI_KVPVALTYPE_UINT8PTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT8PTR_STRING;

#define PVMI_KVPVALTYPE_INT32PTR_STRING "int32*"
static const char PVMI_KVPVALTYPE_INT32PTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_INT32PTR_STRING;

#define PVMI_KVPVALTYPE_UINT32PTR_STRING "uint32*"
static const char PVMI_KVPVALTYPE_UINT32PTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT32PTR_STRING;

#define PVMI_KVPVALTYPE_INT64PTR_STRING "int64*"
static const char PVMI_KVPVALTYPE_INT64PTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_INT64PTR_STRING;

#define PVMI_KVPVALTYPE_UINT64PTR_STRING "uint64*"
static const char PVMI_KVPVALTYPE_UINT64PTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_UINT64PTR_STRING;

#define PVMI_KVPVALTYPE_FLOATPTR_STRING "float*"
static const char PVMI_KVPVALTYPE_FLOATPTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_FLOATPTR_STRING;

#define PVMI_KVPVALTYPE_DOUBLEPTR_STRING "double*"
static const char PVMI_KVPVALTYPE_DOUBLEPTR_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_DOUBLEPTR_STRING;

#define PVMI_KVPVALTYPE_KSV_STRING "ksv"
static const char PVMI_KVPVALTYPE_KSV_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_KSV_STRING;

#define PVMI_KVPVALTYPE_PKVP_STRING "pKvp"
static const char PVMI_KVPVALTYPE_PKVP_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_PKVP_STRING;

#define PVMI_KVPVALTYPE_PPKVP_STRING "ppKvp"
static const char PVMI_KVPVALTYPE_PPKVP_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_PPKVP_STRING;

#define PVMI_KVPVALTYPE_RANGE_FLOAT_STRING "range_float"
static const char PVMI_KVPVALTYPE_RANGE_FLOAT_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_FLOAT_STRING;

#define PVMI_KVPVALTYPE_RANGE_DOUBLE_STRING "range_double"
static const char PVMI_KVPVALTYPE_RANGE_DOUBLE_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_DOUBLE_STRING;

#define PVMI_KVPVALTYPE_RANGE_UINT8_STRING "range_uint8"
static const char PVMI_KVPVALTYPE_RANGE_UINT8_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_UINT8_STRING;

#define PVMI_KVPVALTYPE_RANGE_INT32_STRING "range_int32"
static const char PVMI_KVPVALTYPE_RANGE_INT32_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_INT32_STRING;

#define PVMI_KVPVALTYPE_RANGE_UINT32_STRING "range_uint32"
static const char PVMI_KVPVALTYPE_RANGE_UINT32_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_UINT32_STRING;

#define PVMI_KVPVALTYPE_RANGE_INT64_STRING "range_int64"
static const char PVMI_KVPVALTYPE_RANGE_INT64_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_INT64_STRING;

#define PVMI_KVPVALTYPE_RANGE_UINT64_STRING "range_uint64"
static const char PVMI_KVPVALTYPE_RANGE_UINT64_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_RANGE_UINT64_STRING;

#define PVMI_KVPVALTYPE_BITARRAY32_STRING "bitarray32"
static const char PVMI_KVPVALTYPE_BITARRAY32_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_BITARRAY32_STRING;

#define PVMI_KVPVALTYPE_BITARRAY64_STRING "bitarray64"
static const char PVMI_KVPVALTYPE_BITARRAY64_STRING_CONSTCHAR[] = PVMI_KVPVALTYPE_BITARRAY64_STRING;

typedef struct PvmiKvpRangeInt32
{
    int32 min;
    int32 max;
} range_int32;

typedef struct PvmiKvpRangeUint32
{
    uint32 min;
    uint32 max;
} range_uint32;

typedef union
{
#ifdef __cplusplus
    bool  bool_value;
#else
    c_bool bool_value;
#endif
    float float_value;
    double double_value;
    uint8 uint8_value;
    int32 int32_value;
    uint32 uint32_value;
    oscl_wchar* pWChar_value;
    char* pChar_value;
    uint8* pUint8_value;
    int32* pInt32_value;
    uint32* pUint32_value;
    int64* pInt64_value;
    uint64*  pUint64_value;
    float* pFloat_value;
    double* pDouble_value;
    void *key_specific_value;
    /* pointer to another key/value pair */
    struct __PvmiKvp*  pKvp;
    /* pointer to an array of key/value pairs */
    struct __PvmiKvp**  ppKvp;
} PvmiKvpUnion;

typedef struct __PvmiKvp
{
    PvmiKeyType key;
    int32 length;
    int32 capacity;
    PvmiKvpUnion value;
} PvmiKvp;

#ifdef __cplusplus
class PvmfApicStruct
{
    public:
        PvmfApicStruct()
        {
            iGraphicType = 0;
            iGraphicMimeType = NULL;
            iGraphicDescription = NULL;
            iGraphicData = NULL;
            iGraphicDataLen = 0;
        };

        virtual ~PvmfApicStruct()
        {
            iGraphicType = 0;
            iGraphicMimeType = NULL;
            iGraphicDescription = NULL;
            iGraphicData = NULL;
            iGraphicDataLen = 0;
        };

        uint8        iGraphicType;
        // var length null-terminated unicode string for the picture mime type
        oscl_wchar* iGraphicMimeType;
        // var length null-terminated unicode string for picture description
        oscl_wchar* iGraphicDescription;
        // picture data
        uint8*       iGraphicData;
        // picture data length
        uint32      iGraphicDataLen;
};

class PvmfLyricsCommStruct
{
    public:
        PvmfLyricsCommStruct()
        {
            iDescription = NULL;
            iData = NULL;
        };
        virtual ~PvmfLyricsCommStruct()
        {
            iDescription = NULL;
            iData = NULL;

        }

        oscl_wchar *iDescription;
        oscl_wchar *iData;
};


class PvmfAssetInfo3GPPLocationStruct
{
    public:
        PvmfAssetInfo3GPPLocationStruct()
        {
            _location_name = NULL;
            _astronomical_body = NULL;
            _additional_notes = NULL;
            _role = 0;
            _longitude = 0;
            _latitude = 0;
            _altitude = 0;
            _langCode = 0;
            Lang_code = NULL;
        }
        virtual ~PvmfAssetInfo3GPPLocationStruct()
        {
            _location_name = NULL;
            _astronomical_body = NULL;
            _additional_notes = NULL;
            _role = 0;
            _longitude = 0;
            _latitude = 0;
            _altitude = 0;
            _langCode = 0;
            Lang_code = NULL;
        }

        oscl_wchar* _location_name;
        oscl_wchar* _astronomical_body;
        oscl_wchar* _additional_notes;
        OSCL_HeapString<OsclMemAllocator> Lang_code;
        uint8	_role;

        uint32	_longitude;//fixed-point 16.16 number indicating the longitude in degrees.
        //Negative values represent western longitude

        uint32	_latitude;//fixed-point 16.16 number indicating the latitude in degrees.
        //Negative values represent southern latitude

        uint32	_altitude;//fixed-point 16.16 number indicating the altitude in meters.
        //The reference altitude, indicated by zero, is set to the sea level.
        uint16 _langCode;
};

#endif // __cplusplus

// String used with the termination key value
#define PVMI_KVP_END "kvp_end"

// Capability mime strings
#define INPUT_FORMATS_CAP_QUERY ".../input_formats;attr=cap"
#define INPUT_FORMATS_CUR_QUERY ".../input_formats;attr=cur"
#define INPUT_FORMATS_VALTYPE ".../input_formats;valtype=char*"
#define OUTPUT_FORMATS_CAP_QUERY ".../output_formats;attr=cap"
#define OUTPUT_FORMATS_CUR_QUERY ".../output_formats;attr=cur"
#define OUTPUT_FORMATS_VALTYPE ".../output_formats;valtype=char*"

#define VIDEO_INPUT_WIDTH_CAP_QUERY ".../input/width;attr=cap"
#define VIDEO_INPUT_WIDTH_CAP_RANGE ".../input/width;valtype=range_uint32"
#define VIDEO_INPUT_WIDTH_CUR_QUERY ".../input/width;attr=cur"
#define VIDEO_INPUT_WIDTH_CUR_VALUE ".../input/width;valtype=uint32"

#define VIDEO_INPUT_HEIGHT_CAP_QUERY ".../input/height;attr=cap"
#define VIDEO_INPUT_HEIGHT_CAP_RANGE ".../input/height;valtype=range_uint32"
#define VIDEO_INPUT_HEIGHT_CUR_QUERY ".../input/height;attr=cur"
#define VIDEO_INPUT_HEIGHT_CUR_VALUE ".../input/height;valtype=uint32"

#define VIDEO_INPUT_FRAME_RATE_CUR_QUERY ".../input/frame_rate;attr=cur"
#define VIDEO_INPUT_FRAME_RATE_CUR_VALUE ".../input/frame_rate;valtype=uint32"

#define VIDEO_INPUT_BITRATE_CUR_QUERY ".../input/bitrate;attr=cur"
#define VIDEO_INPUT_BITRATE_CUR_VALUE ".../input/bitrate;valtype=uint32"

#define VIDEO_OUTPUT_WIDTH_CAP_QUERY ".../output/width;attr=cap"
#define VIDEO_OUTPUT_WIDTH_CAP_RANGE ".../output/width;valtype=range_uint32"
#define VIDEO_OUTPUT_WIDTH_CUR_QUERY ".../output/width;attr=cur"
#define VIDEO_OUTPUT_WIDTH_CUR_VALUE ".../output/width;valtype=uint32"

#define VIDEO_FRAME_ORIENTATION_CUR_QUERY ".../output/frame_orientation;attr=cur"
#define VIDEO_FRAME_ORIENTATION_CUR_VALUE ".../output/frame_orientation;valtype=uint8"
#define VIDEO_OUTPUT_HEIGHT_CAP_QUERY ".../output/height;attr=cap"
#define VIDEO_OUTPUT_HEIGHT_CAP_RANGE ".../output/height;valtype=range_uint32"
#define VIDEO_OUTPUT_HEIGHT_CUR_QUERY ".../output/height;attr=cur"
#define VIDEO_OUTPUT_HEIGHT_CUR_VALUE ".../output/height;valtype=uint32"

#define VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY ".../output/frame_rate;attr=cur"
#define VIDEO_OUTPUT_FRAME_RATE_CUR_VALUE ".../output/frame_rate;valtype=uint32"

#define VIDEO_OUTPUT_IFRAME_INTERVAL_CUR_QUERY ".../output/iframe_interval;attr=cur"
#define VIDEO_OUTPUT_IFRAME_INTERVAL_CUR_VALUE ".../output/iframe_interval;valtype=uint32"

#define TEXT_OUTPUT_WIDTH_CUR_QUERY ".../output/width;attr=cur"
#define TEXT_OUTPUT_WIDTH_CUR_VALUE ".../output/width;valtype=uint32"
#define TEXT_OUTPUT_HEIGHT_CUR_QUERY ".../output/height;attr=cur"
#define TEXT_OUTPUT_HEIGHT_CUR_VALUE ".../output/height;valtype=uint32"


#define OUTPUT_BITRATE_CUR_QUERY ".../output/bitrate;attr=cur"
#define OUTPUT_BITRATE_CUR_VALUE ".../output/bitrate;valtype=uint32"

#define OUTPUT_TIMESCALE_CUR_QUERY ".../output/timescale;attr=cur"
#define OUTPUT_TIMESCALE_CUR_VALUE ".../output/timescale;valtype=uint32"

#define AUDIO_INPUT_SAMPLING_RATE_CUR_QUERY ".../input/sampling_rate;attr=cur"
#define AUDIO_INPUT_SAMPLING_RATE_CUR_VALUE ".../input/sampling_rate;valtype=uint32"

#define AUDIO_OUTPUT_SAMPLING_RATE_CUR_QUERY ".../output/sampling_rate;attr=cur"
#define AUDIO_OUTPUT_SAMPLING_RATE_CUR_VALUE ".../output/sampling_rate;valtype=uint32"

#define AUDIO_OUTPUT_NUM_CHANNELS_CUR_QUERY ".../num_channels;attr=cur"
#define AUDIO_OUTPUT_NUM_CHANNELS_CUR_VALUE ".../num_channels;valtype=uint32"

#define VIDEO_AVC_OUTPUT_SPS_CUR_QUERY ".../output/sps;attr=cur"
#define VIDEO_AVC_OUTPUT_SPS_CUR_VALUE ".../output/sps;valtype=key_specific_value"

#define VIDEO_AVC_OUTPUT_PPS_CUR_QUERY ".../output/pps;attr=cur"
#define VIDEO_AVC_OUTPUT_PPS_CUR_VALUE ".../output/pps;valtype=key_specific_value"
#define TIMED_TEXT_OUTPUT_CONFIG_INFO_CUR_QUERY ".../output/timedtext;attr=cur"
#define TIMED_TEXT_OUTPUT_CONFIG_INFO_CUR_VALUE ".../output/timedtext;valtype=key_specific_value"

// key for format type for any type of media
#define PVMF_FORMAT_TYPE_VALUE_KEY "x-pvmf/media/format-type;valtype=char*"

// key for audio decoder format
#define PVMF_AUDIO_DEC_FORMAT_TYPE_VALUE_KEY "x-pvmf/audio/decoder/format-type;valtype=char*"

// key for video decoder format
#define PVMF_VIDEO_DEC_FORMAT_TYPE_VALUE_KEY "x-pvmf/video/decoder/format-type;valtype=char*"

// Keys for format specific info for any type of media
#define PVMF_FORMAT_SPECIFIC_INFO_KEY "x-pvmf/media/format_specific_info;valtype=key_specific_value"

// Keys for format specific info for video media type
#define PVMF_FORMAT_SPECIFIC_INFO_KEY_YUV "x-pvmf/media/format_specific_info_yuv;valtype=key_specific_value"

// Keys for format specific info for audio media type
#define PVMF_FORMAT_SPECIFIC_INFO_KEY_PCM "x-pvmf/media/format_specific_info_pcm;valtype=key_specific_value"

// Keys for buffer allocator
#define PVMF_BUFFER_ALLOCATOR_KEY "x-pvmf/media/buffer_allocator;valtype=key_specific_value"

// Keys for format specific info plus first media sample for any type of media
#define PVMF_FORMAT_SPECIFIC_INFO_PLUS_FIRST_SAMPLE_KEY "x-pvmf/media/format_specific_info_plus_first_sample;valtype=uint8*"

#define PVMF_CLIENT_PLAYBACK_CLOCK_KEY "x-pvmf/player-clock-pointer;valtype=key_specific_value"

// Keys for format bitrate info for any type of media
#define PVMF_BITRATE_VALUE_KEY "x-pvmf/media/bit-rate;valtype=uint32"

// Keys for format framerate info for any type of media
#define PVMF_FRAMERATE_VALUE_KEY "x-pvmf/media/frame-rate;valtype=uint32"

// Key for signalling max number of outstanding media msgs
#define PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY "x-pvmf/datapath/port/max-num-media-msgs;valtype=uint32"


// Keys for media output components -- Audio
//
//audio format strings as defined in pvmf_format_type.h
#define MOUT_AUDIO_FORMAT_KEY "x-pvmf/audio/render/media_format;valtype=char*"
//audio sampling rate in Hz
#define MOUT_AUDIO_SAMPLING_RATE_KEY "x-pvmf/audio/render/sampling_rate;valtype=uint32"
//audio number of channels (1 for mono, 2 for stereo)
#define MOUT_AUDIO_NUM_CHANNELS_KEY "x-pvmf/audio/render/channels;valtype=uint32"

// Key to configure the Number of frames required in a Media msg
#define PVMF_NUM_DECODED_FRAMES_CONFIG_KEY "x-pvmf/audio/render/num_decoded_frames;valtype=uint32"

// Keys for media output components -- Video
//
//video format strings as defined in pvmf_format_type.h
#define MOUT_VIDEO_FORMAT_KEY "x-pvmf/video/render/media_format;valtype=char*"
// video sub-format (e.g. YUV420-planar etc.)
#define MOUT_VIDEO_SUBFORMAT_KEY "x-pvmf/video/render/media_subformat;valtype=char*"

//video dimensions in pixels
#define MOUT_VIDEO_WIDTH_KEY "x-pvmf/video/render/width;valtype=uint32"
#define MOUT_VIDEO_HEIGHT_KEY "x-pvmf/video/render/height;valtype=uint32"
//video display dimensions in pixels.
#define MOUT_VIDEO_DISPLAY_WIDTH_KEY "x-pvmf/video/render/display_width;valtype=uint32"
#define MOUT_VIDEO_DISPLAY_HEIGHT_KEY "x-pvmf/video/render/display_height;valtype=uint32"
#define MOUT_VIDEO_OUTPUT_WIDTH_KEY "x-pvmf/video/render/output_width;valtype=uint32"
#define MOUT_VIDEO_OUTPUT_HEIGHT_KEY "x-pvmf/video/render/output_height;valtype=uint32"
#define MOUT_VIDEO_DISPLAY_ROTATION_KEY "x-pvmf/video/render/display_rotation;valtype=uint32"
#define MOUT_VIDEO_DISPLAY_BITMAP_KEY "x-pvmf/video/render/display_bitmap;valtype=uint32"
//video vol header info
#define MOUT_VIDEO_VOL_HEADER_KEY "x-pvmf/video/vol_header;valtype=uint8*"


// Keys for media output components -- Text
//
//text format strings as defined in pvmf_format_type.h
#define MOUT_TEXT_FORMAT_KEY "x-pvmf/text/render/media_format;valtype=char*"

// Key for media output rate
#define MOUT_MEDIAXFER_OUTPUT_RATE "x-pvmf/mediaxfer/output/rate;type=rel;valtype=int32"


// Keys for media input components -- Audio
//
//audio sampling rate in Hz
#define MIN_AUDIO_SAMPLING_RATE_KEY "x-pvmf/encoder/audio/sampling_rate;valtype=uint32"
//audio number of channels (1 for mono, 2 for stereo)
#define MIN_AUDIO_NUM_CHANNELS_KEY "x-pvmf/encoder/audio/channels;valtype=uint32"

// Keys for media input components -- Video
//
// Keys for media output components -- Text
//
#define TEXT_INPUT_WIDTH_CUR_QUERY ".../input/width;attr=cur"
#define TEXT_INPUT_WIDTH_CUR_VALUE ".../input/width;valtype=uint32"
#define TEXT_INPUT_HEIGHT_CUR_QUERY ".../input/height;attr=cur"
#define TEXT_INPUT_HEIGHT_CUR_VALUE ".../input/height;valtype=uint32"

//AVC Encoder
#define PVMF_AVC_ENCODER_ENCODINGMODE_KEY	"x-pvmf/encoder/video/encoding_mode;valtype=uint32"

#define INPUT_TIMESCALE_CUR_QUERY ".../input/timescale;attr=cur"
#define INPUT_TIMESCALE_CUR_VALUE ".../input/timescale;valtype=uint32"



#endif // PVMI_KVP_H_INCLUDED




