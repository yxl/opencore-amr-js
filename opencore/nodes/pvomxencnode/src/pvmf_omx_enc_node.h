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
#ifndef PVMF_OMX_ENC_NODE_H_INCLUDED
#define PVMF_OMX_ENC_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVMF_VIDEO_H_INCLUDED
#include "pvmf_video.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_OMX_ENC_PORT_H_INCLUDED
#include "pvmf_omx_enc_port.h"
#endif

#ifndef PVMF_OMX_ENC_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvmf_omx_enc_node_extension_interface.h"
#endif

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#ifndef PVMF_AUDIO_ENCNODE_EXTENSION_H_INCLUDED
#include "pvmf_audio_encnode_extension.h"
#endif

// DV: NOTE - this needs to be fixed
//#ifndef __PVM4VENCODER_H
//#include "pvm4vencoder.h"
//#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PV_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef PVMF_OMX_ENC_CALLBACKS_H_INCLUDED
#include "pvmf_omx_enc_callbacks.h"
#endif

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#if (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_REL)
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#endif

#define PVMFOMXENCNODE_NUM_CMD_IN_POOL 8
#define PVOMXENCMAXNUMDPBFRAMESPLUS1 17


typedef struct OutputBufCtrlStruct
{
    OMX_BUFFERHEADERTYPE *pBufHdr;
}OutputBufCtrlStruct;

typedef struct InputBufCtrlStruct
{
    OMX_BUFFERHEADERTYPE *pBufHdr;
    PVMFSharedMediaDataPtr pMediaData;
} InputBufCtrlStruct;


// fwd class declaration
class PVLogger;
//memory allocator type for this node.
typedef OsclMemAllocator PVMFOMXEncNodeAllocator;
typedef OsclMemAllocDestructDealloc<uint8> PVMFOMXEncNodeAllocDestructDealloc;


// CALLBACK PROTOTYPES
OMX_ERRORTYPE CallbackEventHandlerEnc(OMX_OUT OMX_HANDLETYPE aComponent,
                                      OMX_OUT OMX_PTR aAppData,
                                      OMX_OUT OMX_EVENTTYPE aEvent,
                                      OMX_OUT OMX_U32 aData1,
                                      OMX_OUT OMX_U32 aData2,
                                      OMX_OUT OMX_PTR aEventData);

OMX_ERRORTYPE CallbackEmptyBufferDoneEnc(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

OMX_ERRORTYPE CallbackFillBufferDoneEnc(OMX_OUT OMX_HANDLETYPE aComponent,
                                        OMX_OUT OMX_PTR aAppData,
                                        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);



//Default values for number of Input/Output buffers. If the component needs more than this, it will be
// negotiated. If the component does not need more than this number, the default is used
#define NUMBER_INPUT_BUFFER 5
#define NUMBER_OUTPUT_BUFFER 9

// for AMR encoder
#define MAX_NUM_AMR_FRAMES_PER_BUFFER 25
#define PVMF_AMRENC_DEFAULT_SAMPLING_RATE 8000
#define PVMF_AMRENC_DEFAULT_NUM_CHANNELS 1
#define PVMF_AMRENC_DEFAULT_BITSPERSAMPLE 16
// in milliseconds
#define PVMF_AMRENC_DEFAULT_FRAME_DURATION 20

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////
/////////////////////////
/////////////////////////
// CUSTOM DEALLOCATOR FOR MEDIA DATA SHARED PTR WRAPPER:
//						1) Deallocates the underlying output buffer
//						2) Deallocates the pvci buffer wrapper and the rest of accompanying structures
//					  Deallocator is created as part of the wrapper, and travels with the buffer wrapper

class PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA : public OsclDestructDealloc
{
    public:
        PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA(Oscl_DefAlloc* allocator, void *pMempoolData) :
                buf_alloc(allocator), ptr_to_data_to_dealloc(pMempoolData) {};
        virtual ~PVOMXEncBufferSharedPtrWrapperCombinedCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // call buffer deallocator
            if (buf_alloc != NULL)
            {
                buf_alloc->deallocate(ptr_to_data_to_dealloc);
            }

            // this is needed to completely free PVMFMediaDataImpl, since it allocates memory for the frag list
            ((PVMFMediaDataImpl*)ptr)->~PVMFMediaDataImpl();
            oscl_free(ptr);
        }

    private:
        Oscl_DefAlloc* buf_alloc;
        void *ptr_to_data_to_dealloc;
};



#define PVMFOMXEncNodeCommandBase PVMFGenericNodeCommand<PVMFOMXEncNodeAllocator>

class PVMFOMXEncNodeCommand: public PVMFOMXEncNodeCommandBase
{
    public:
        //constructor for Custom2 command
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFOMXEncNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }

        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList* aKeyList, uint32 aStartIndex, int32 aMaxEntries, char* aQueryKey, const OsclAny* aContext)
        {
            PVMFOMXEncNodeCommandBase::Construct(s, cmd, aContext);
            iStartIndex = aStartIndex;
            iMaxEntries = aMaxEntries;

            if (aQueryKey == NULL)
            {
                query_key[0] = 0;
            }
            else
            {
                if (aQueryKey != NULL)
                    oscl_strncpy(query_key, aQueryKey, oscl_strlen(aQueryKey) + 1);
            }

            iParam1 = (OsclAny*)aKeyList;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        void Parse(PVMFMetadataList*& MetaDataListPtr, uint32 &aStartingIndex, int32 &aMaxEntries, char*&aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = iStartIndex;
            aMaxEntries = iMaxEntries;
            if (query_key[0] == 0)
            {
                aQueryKey = NULL;
            }
            else
            {
                aQueryKey = query_key;
            }
        }

        // Constructor and parser for GetNodeMetadataValue
        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList* aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* aValueList, uint32 aStartIndex, int32 aMaxEntries, const OsclAny* aContext)
        {
            PVMFOMXEncNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aKeyList;
            iParam2 = (OsclAny*)aValueList;

            iStartIndex = aStartIndex;
            iMaxEntries = aMaxEntries;

            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFMetadataList* &aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList, uint32 &aStartingIndex, int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = iStartIndex;
            aMaxEntries = iMaxEntries;
        }


        virtual bool hipri()
        {
            //this routine identifies commands that need to
            //go at the front of the queue.  derived command
            //classes can override it if needed.
            return (iCmd == PVOMXENC_NODE_CMD_CANCELALL
                    || iCmd == PVOMXENC_NODE_CMD_CANCELCMD);
        }

        enum PVOMXEncNodeCmdType
        {
            PVOMXENC_NODE_CMD_QUERYUUID,
            PVOMXENC_NODE_CMD_QUERYINTERFACE,
            PVOMXENC_NODE_CMD_REQUESTPORT,
            PVOMXENC_NODE_CMD_RELEASEPORT,
            PVOMXENC_NODE_CMD_INIT,
            PVOMXENC_NODE_CMD_PREPARE,
            PVOMXENC_NODE_CMD_START,
            PVOMXENC_NODE_CMD_STOP,
            PVOMXENC_NODE_CMD_FLUSH,
            PVOMXENC_NODE_CMD_PAUSE,
            PVOMXENC_NODE_CMD_RESET,
            PVOMXENC_NODE_CMD_CANCELALL,
            PVOMXENC_NODE_CMD_CANCELCMD,
            PVOMXENC_NODE_CMD_GETNODEMETADATAKEY,
            PVOMXENC_NODE_CMD_GETNODEMETADATAVALUE,
            PVOMXENC_NODE_CMD_INVALID
        };




    private:
        uint32 iStartIndex;
        uint32 iMaxEntries;
        char query_key[256];

};

//Default vector reserve size
#define PVMF_OMXENC_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_OMXENC_NODE_COMMAND_ID_START 6000

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// STRUCTURES FOR ENCODING PARAMETERS ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_LAYER 1

/** Contains supported input format */
enum EncVideoInputFormat
{
    EI_RGB24,
    EI_RGB12,
    EI_YUV420,
    EI_UYVY
};

/** Type of contents for optimal encoding mode. */
enum EncContentType
{
    /** Content is to be streamed in real-time. */
    EI_M4V_STREAMING,

    /** Content is to be downloaded and playbacked later.*/
    EI_M4V_DOWNLOAD,

    /** Content is to be 3gpp baseline compliant. */
    EI_H263,

    EI_H264
};

/** Rate control type. */
enum EncRateControlType
{
    /** Constant quality, variable bit rate, fixed quantization level. */
    ECONSTANT_Q,

    /** Short-term constant bit rate control. */
    ECBR_1,

    /** Long-term constant bit rate control. */
    EVBR_1
};

/** Targeted profile and level to encode. */
enum EncM4VProfileLevel
{
    /* Non-scalable profile */
    EI_SIMPLE_LEVEL0 = 0,
    EI_SIMPLE_LEVEL1,
    EI_SIMPLE_LEVEL2,
    EI_SIMPLE_LEVEL3,
    EI_CORE_LEVEL1,
    EI_CORE_LEVEL2,

    /* Scalable profile */
    EI_SIMPLE_SCALABLE_LEVEL0 = 6,
    EI_SIMPLE_SCALABLE_LEVEL1,
    EI_SIMPLE_SCALABLE_LEVEL2,

    EI_CORE_SCALABLE_LEVEL1 = 10,
    EI_CORE_SCALABLE_LEVEL2,
    EI_CORE_SCALABLE_LEVEL3
};

/** Targeted profile to encode. */
enum EncAVCProfile
{
    /* Non-scalable profile */
    EI_PROFILE_DEFAULT,
    EI_PROFILE_BASELINE,
    EI_PROFILE_MAIN,
    EI_PROFILE_EXTENDED,
    EI_PROFILE_HIGH,
    EI_PROFILE_HIGH10,
    EI_PROFILE_HIGH422,
    EI_PROFILE_HIGH444
};

/** Targeted level to encode. */
enum EncAVCLevel
{
    EI_LEVEL_AUTODETECT,
    EI_LEVEL_1,
    EI_LEVEL_1B,
    EI_LEVEL_11,
    EI_LEVEL_12,
    EI_LEVEL_13,
    EI_LEVEL_2,
    EI_LEVEL_21,
    EI_LEVEL_22,
    EI_LEVEL_3,
    EI_LEVEL_31,
    EI_LEVEL_32,
    EI_LEVEL_4,
    EI_LEVEL_41,
    EI_LEVEL_42,
    EI_LEVEL_5,
    EI_LEVEL_51,
};

/** Output format */
enum TAVCEIOutputFormat
{
    /** output in byte stream format according to Annex B */
    EAVCEI_OUTPUT_ANNEXB,

    /** output for MP4 file format */
    EAVCEI_OUTPUT_MP4,

    /** output in RTP format according to RFC 3984 */
    EAVCEI_OUTPUT_RTP
};



/** Type of contents for optimal encoding mode. */
enum EncEncodingMode
{
    /** Content is encoded as fast as possible with error protection */
    EI_ENCMODE_TWOWAY,

    /** Content is encoded as fast as possible without error protection */
    EI_ENCMODE_RECORDER,

    /** Content is encoded with better quality (slow) with error protection */
    EI_ENCMODE_STREAMING,

    /** Content is encoded with better quality (slow) without error protection */
    EI_ENCMODE_DOWNLOAD
};


/** This structure contains encoder settings. */
typedef struct PV_VideoEncodeParam
{

////////////////////////////////////////////////////////////////////////////////////
////////////////////////// COMMON PARAMETERS ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

    /** Specifies an  ID that will be used to specify this encoder while returning
    the bitstream in asynchronous mode. */
    uint32				iEncodeID;


    /** Specifies whether base only (iNumLayer = 1) or base + enhancement layer
    (iNumLayer =2 ) is to be used. */
    int32				iNumLayer;

    /** Specifies the width in pixels of the encoded frames. IFrameWidth[0] is for
    base layer and iFrameWidth[1] is for enhanced layer. */
    int 				iFrameWidth[MAX_LAYER];

    /** Specifies the height in pixels of the encoded frames. IFrameHeight[0] is for
    base layer and iFrameHeight[1] is for enhanced layer. */
    int					iFrameHeight[MAX_LAYER];

    /** Specifies the cumulative bit rate in bit per second. IBitRate[0] is for base
    layer and iBitRate[1] is for base+enhanced layer.*/
    int 				iBitRate[MAX_LAYER];

    /** Specifies the cumulative frame rate in frame per second. IFrameRate[0] is for
    base layer and iFrameRate[1] is for base+enhanced layer. */
    float 				iFrameRate[MAX_LAYER];

    /** Specifies the rate control algorithm among one of the following constant Q,
    CBR and VBR.  The structure EncRateControlType is defined above.*/
    EncRateControlType iRateControlType;

    /** Specifies the initial quantization parameter for the first I-frame. If constant Q
    rate control is used, this QP will be used for all the I-frames. This number must be
    set between 1 and 31, otherwise, Initialize() will fail. */
    int					iIquant[2];

    /** Specifies the initial quantization parameter for the first P-frame. If constant Q
    rate control is used, this QP will be used for all the P-frames. This number must be
    set between 1 and 31, otherwise, Initialize() will fail. */
    int					iPquant[2];

    /** Specifies the initial quantization parameter for the first B-frame. If constant Q
    rate control is used, this QP will be used for all the B-frames. This number must be
    set between 1 and 31, otherwise, Initialize() will fail. */
    int					iBquant[2];


    /** Specifies the maximum number of P-frames between 2 INTRA frames. An INTRA mode is
    forced to a frame once this interval is reached. When there is only one I-frame is present
    at the beginning of the clip, iIFrameInterval should be set to -1. */
    int32				iIFrameInterval;

    /** According to iIFrameInterval setting, the minimum number of intra MB per frame is
    optimally calculated for error resiliency. However, when iIFrameInterval is set to -1,
    iNumIntraMBRefresh must be specified to guarantee the minimum number of intra
    macroblocks per frame.*/
    uint32				iNumIntraMBRefresh;

    /** Specifies automatic scene detection where I-frame will be used the the first frame
    in a new scene. */
    bool				iSceneDetection;

    /** Specifies the packet size in bytes which represents the number of bytes between two resync markers.
    For EI_M4V_DOWNLOAD and EI_H263, if iPacketSize is set to 0, there will be no resync markers in the bitstream.
    For EI_M4V_STREAMING is parameter must be set to a value greater than 0. For AVC, this value specifies
    the packet size in bytes which represents the desired number of bytes per NAL.
    If this number is set to 0, the AVC encoder will encode the entire slice group as one NAL. */
    uint32				iPacketSize;

    /** Specifies the VBV buffer size which determines the end-to-end delay between the
    encoder and the decoder.  The size is in unit of seconds. For download application,
    the buffer size can be larger than the streaming application. For 2-way application,
    this buffer shall be kept minimal. For a special case, in VBR mode, iBufferDelay will
    be set to -1 to allow buffer underflow. */
    float				iBufferDelay;


    /** Specifies the duration of the clip in millisecond.*/
    int32				iClipDuration;


////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// H263/M4V specific parameters ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

    /** Specifies the profile and level used to encode the bitstream. When present,
    other settings will be checked against the range allowable by this target profile
    and level. Fail may be returned from the Initialize call. */
    EncM4VProfileLevel	iProfileLevel;

    /** Specifies the picture quality factor on the scale of 1 to 10. It trades off
    the picture quality with the frame rate. Higher frame quality means lower frame rate.
    Lower frame quality for higher frame rate.*/
    int32				iFrameQuality;


    /** Enable the use of iFrameQuality to determine the frame rate. If it is false,
    the encoder will try to meet the specified frame rate regardless of the frame quality.*/
    bool				iEnableFrameQuality;


    /** Specifies the type of the access whether it is streaming, EI_H263, EI_M4V_STREAMING
    (data partitioning mode) or download, EI_M4V_DOWNLOAD (combined mode).*/
    EncContentType		iContentType;


    /** Specifies high quality but also high complexity mode for rate control. */
    bool				iRDOptimal;


    /** Specifies the search range in pixel unit for motion vector. The range of the
    motion vector will be of dimension [-iSearchRange.5, +iSearchRange.0]. */
    int32				iSearchRange;

    /** Specifies the use of 8x8 motion vectors. */
    bool				iMV8x8;

    /** Specifies the use of half-pel motion vectors. */
    bool				iMVHalfPel;

    /** Specifies whether the current frame skipping decision is allowed after encoding
    the current frame. If there is no memory of what has been coded for the current frame,
    iNoCurrentSkip has to be on. */
    bool				iNoCurrentSkip;

    /** Specifies that no frame skipping is allowed. Frame skipping is a tool used to
    control the average number of bits spent to meet the target bit rate. */
    bool				iNoFrameSkip;

    /** Specifies whether RVLC (reversible VLC) is to be used or not.
    */
    bool				iRVLCEnable;

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// H264 specific parameters ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

    /** Specifies the targeted profile, and will also specifies available tools for iEncMode.
    If default is used, encoder will choose its own preferred profile. If autodetect is used, encoder
    will check other settings and choose the right profile that doesn't have any conflicts. */
    EncAVCProfile		iAVCProfile;

    /** Specifies the target level  When present,
    other settings will be checked against the range allowable by this target level.
    Fail will returned upon Initialize call. If not known, users must set it to autodetect. Encoder will
    calculate the right level that doesn't conflict with other settings. */
    EncAVCLevel			iAVCLevel;

    /** Specifies the encoding mode. This translates to the complexity of encoding modes and
    error resilient tools. This is for future use - enc mode will be used to set other parameters to appropriate
    values.
    */

    EncEncodingMode		iEncMode;

    /** Specifies that SPS and PPS are retrieved first and sent out-of-band */
    //bool				iOutOfBandParamSet;


} PV_VideoEncodeParam;



/** Structure for input format information */
typedef struct PV_VideoInputFormat
{
    /** Contains the width in pixels of the input frame. */
    int32 			iFrameWidth;

    /** Contains the height in pixels of the input frame. */
    int32			iFrameHeight;

    /** Contains the input frame rate in the unit of frame per second. */
    float			iFrameRate;

    /** Contains Frame Orientation. Used for RGB input. 1 means Bottom_UP RGB, 0 means Top_Down RGB, -1 for video formats other than RGB*/
    int				iFrameOrientation;

    /** Contains the format of the input video, e.g., YUV 4:2:0, UYVY, RGB24, etc. */
    EncVideoInputFormat 	iVideoFormat;
} PV_VideoInputFormat;


/** Structure for audio encode information */
typedef struct PV_AudioEncodeParam
{

    // DESCRIPTION: Final sampling frequency used when encoding in Hz.
    //              For example, "44100" Hz.
    // USAGE:       If the input sampling rate is not appropriate (e.g.,
    //              the codec requires a different sampling frequency),
    //              the CAEI will fill this in with the final sampling
    //              rate. The CAEI will perform resampling if the
    //              input sampling frequency is not the same as the output
    //              sampling frequency.
    uint32 iOutputSamplingRate;

    // DESCRIPTION: Number of output channels:1=Mono,2=Stereo. (Mono uses 1
    //              channel; Stereo uses 2 channels).
    // USAGE:       The CAEI will fill this in if it needs to convert
    //              the input samples to what is required by the codec.
    uint8 iOutputNumChannels;

    // DESCRIPTION: Currently, output bitrate is expressed as GSM-AMR type rate
    PVMF_GSMAMR_Rate iAMRBitrate;

    // DESCRIPTION: output bitrate in bits per second for non-AMR codecs
    uint32  iOutputBitrate;

    //
    uint32	iMaxNumOutputFramesPerBuffer;


} PV_AudioEncodeParam;

enum EInterleaveMode
{
    EINTERLEAVE_LR,        // interleaved left-right
    EGROUPED_LR,           // non-interleaved left-right
    ENUM_INTERLEAVE_MODES  // number of modes supported
};

/** Structure for input format information */
typedef struct PV_AudioInputFormat
{


    // DESCRIPTION: Number of bits per sample. For example, set it to "16"
    //              bits for PCM.
    // USAGE:       The authoring application is required to fill this in.
    //
    int32 iInputBitsPerSample;

    // DESCRIPTION: Sampling rate of the input samples in Hz.
    //              For example, set it to "22050" Hz. AMRNB is always 8khz
    // USAGE:       The authoring application is required to fill this in.
    //              If sampling rate is not known until CAEI is initialized,
    //              use '0'.
    uint32 iInputSamplingRate;


    // DESCRIPTION: Number of input channels:1=Mono,2=Stereo.(Mono uses 1 channel;
    //              Stereo uses 2 channels).
    // USAGE:       The authoring application is required to fill this in.
    //
    uint8 iInputNumChannels;

    // DESCRIPTION: Whether to interleave or not the multi-channel input samples:
    //              EINTERLEAVE_LR  = LRLRLRLRLR    (interleaved left-right)
    //              EGROUPED_LR = LLLLLL...RRRRRR   (non-interleaved left-right)
    // USAGE:       The authoring application is required to fill this in.
    //
    EInterleaveMode iInputInterleaveMode;



} PV_AudioInputFormat;








///////////////////////////////////////////////////////////////////////////////////////////////////////
//CAPABILITY AND CONFIG

// Structure to hold the key string info for
// encnode's capability-and-config
struct PVOMXEncNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

// The number of characters to allocate for the key string
#define PVOMXENCNODECONFIG_KEYSTRING_SIZE 128


// Key string info at the base level ("x-pvmf/encoder/video")
//								  or ("x-pvmf/encoder/audio")

#ifdef _TEST_AE_ERROR_HANDLING
#define PVOMXENCNODECONFIG_BASE_NUMKEYS 8
#else
#define PVOMXENCNODECONFIG_BASE_NUMKEYS 3
#endif

const PVOMXEncNodeKeyStringData PVOMXEncNodeConfigBaseKeys[PVOMXENCNODECONFIG_BASE_NUMKEYS] =
{
    {"sampling_rate", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"channels", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"encoding_mode", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#ifdef _TEST_AE_ERROR_HANDLING
    , {"error_start_init", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-encode", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-node-cmd", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-config-header", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"data-path-stall", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#endif
};

enum BaseKeys_IndexMapType
{
    SAMPLING_RATE = 0, // audio
    CHANNELS,			// audio
    ENCODING_MODE		// avc
#ifdef _TEST_AE_ERROR_HANDLING
    , ERROR_START_INIT
    , ERROR_ENCODE
    , ERROR_NODE_CMD
    , ERROR_CONFIG_HEADER
    , ERROR_DATAPATH_STALL
#endif
};

// default encode parameters
#define DEFAULT_BITRATE 32000
#define DEFAULT_FRAME_WIDTH 176
#define DEFAULT_FRAME_HEIGHT 144
#define DEFAULT_FRAME_RATE 5
#define DEFAULT_OMX_MP4ENC_QPI 15
#define DEFAULT_OMX_MP4ENC_QPP 12
#define DEFAULT_OMX_MP4ENC_QPB 12

#define DEFAULT_OMX_AVCENC_QPI 25
#define DEFAULT_OMX_AVCENC_QPP 30
#define DEFAULT_OMX_AVCENC_QPB 33

#define PVMF_AVCENC_NODE_SPS_VECTOR_RESERVE 16  // max allowed by standard
#define PVMF_AVCENC_NODE_PPS_VECTOR_RESERVE 256 // max allowed by standard
#define DEFAULT_PARAMS_SET_LENGTH 256

typedef enum
{
    PV_MPEG_VIDEO_SIMPLE_PROFILE = 0,
    PV_MPEG_VIDEO_SIMPLE_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_CORE_PROFILE,
    PV_MPEG_VIDEO_MAIN_PROFILE,
    PV_MPEG_VIDEO_N_BIT_PROFILE,
    PV_MPEG_VIDEO_SCALABLE_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_FACE_ANIMATION_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_FBA_PROFILE,
    PV_MPEG_VIDEO_BASIC_ANIMATED_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_HYBRID_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_REAL_TIME_SIMPLE_PROFILE,
    PV_MPEG_VIDEO_CORE_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_CODING_EFFICIENCY_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_CORE_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_SCALABLE_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_STUDIO_PROFILE,
    PV_MPEG_VIDEO_CORE_STUDIO_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_SIMPLE_PROFILE,
    PV_MPEG_VIDEO_FINE_GRANUALITY_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_RESERVED_PROFILE
}PVMF_MPEGVideoProfileType;

typedef enum
{
    PV_MPEG_VIDEO_LEVEL0 = 0,
    PV_MPEG_VIDEO_LEVEL1,
    PV_MPEG_VIDEO_LEVEL2,
    PV_MPEG_VIDEO_LEVEL3,
    PV_MPEG_VIDEO_LEVEL4,
    PV_MPEG_VIDEO_LEVEL5,
    PV_MPEG_VIDEO_LEVEL_UNKNOWN

}PVMF_MPEGVideoLevelType;

/////////////////////////////////////////////////////////////////////////////////////////





//Mimetypes for the custom interface
#define PVMF_OMX_ENC_NODE_MIMETYPE "pvxxx/OMXEncNode"
#define PVMF_BASEMIMETYPE "pvxxx"

//Command queue type
typedef PVMFNodeCommandQueue<PVMFOMXEncNodeCommand, PVMFOMXEncNodeAllocator> PVMFOMXEncNodeCmdQ;


class PVMFOMXEncNode
            : public OsclActiveObject
            , public PVMFNodeInterface
            , public OsclMemPoolFixedChunkAllocatorObserver
            , public PVMFOMXEncNodeExtensionInterface
            , public PVMFMetadataExtensionInterface
            , public PvmiCapabilityAndConfig
            , public PVMp4H263EncExtensionInterface
            , public PVAudioEncExtensionInterface
{
    public:
        PVMFOMXEncNode(int32 aPriority);
        ~PVMFOMXEncNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, PVMFOMXEncNodeAllocator>& aUuids,
                                bool aExactUuidsOnly = false,
                                const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr,
                                     const OsclAny* aContext = NULL);
        PVMFCommandId RequestPort(PVMFSessionId
                                  , int32 aPortTag, const PvmfMimeString* aPortConfig = NULL, const OsclAny* aContext = NULL);
        PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContextData = NULL);
        PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);

        // From PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // From PVInterface
        virtual void addRef();
        virtual void removeRef();
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);


        //**********begin PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* query_key = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries,
                                          char* query_key = NULL, const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                            Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 starting_index, uint32 end_index);
        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, uint32 end_index);
        //**********End PVMFMetadataExtensionInterface

        bool VerifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        //********** CB Functions to serve OpenMAX Encoder

        //Process callback functions. They will be executed in testapp thread context
        //	These callbacks are used only in the Multithreaded component case
        OsclReturnCode ProcessCallbackEventHandler_MultiThreaded(OsclAny* P);
        OsclReturnCode ProcessCallbackEmptyBufferDone_MultiThreaded(OsclAny* P);
        OsclReturnCode ProcessCallbackFillBufferDone_MultiThreaded(OsclAny* P);

        //Callback objects - again, these are used only in the case of Multithreaded component
        EventHandlerThreadSafeCallbackAOEnc*	 iThreadSafeHandlerEventHandler;
        EmptyBufferDoneThreadSafeCallbackAOEnc* iThreadSafeHandlerEmptyBufferDone;
        FillBufferDoneThreadSafeCallbackAOEnc*  iThreadSafeHandlerFillBufferDone;

        OMX_CALLBACKTYPE       iCallbacks; // structure that contains callback ptrs.
        // OMX CALLBACKS
        // 1) AO OMX component running in the same thread as the OMX node
        //	In this case, the callbacks can be called directly from the component
        //	The callback: OMX Component->CallbackEventHandler->EventHandlerProcessing
        //	The callback can perform do RunIfNotReady

        // 2) Multithreaded component
        //	In this case, the callback is made using the threadsafe callback (TSCB) AO
        //	Component thread : OMX Component->CallbackEventHandler->TSCB(ReceiveEvent)
        //  Node thread		 : TSCB(ProcessEvent)->ProcessCallbackEventHandler_MultiThreaded->EventHandlerProcessing


        //==============================================================================

        OMX_ERRORTYPE EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                                             OMX_OUT OMX_PTR aAppData,
                                             OMX_OUT OMX_EVENTTYPE aEvent,
                                             OMX_OUT OMX_U32 aData1,
                                             OMX_OUT OMX_U32 aData2,
                                             OMX_OUT OMX_PTR aEventData);

        OMX_ERRORTYPE EmptyBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                                                OMX_OUT OMX_PTR aAppData,
                                                OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        OMX_ERRORTYPE FillBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                                               OMX_OUT OMX_PTR aAppData,
                                               OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);



        bool IsComponentMultiThreaded()
        {
            return iIsOMXComponentMultiThreaded;
        };


        // From PvmiCapabilityAndConfig
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements);
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext = NULL);
        uint32 getCapabilityMetric(PvmiMIOSession aSession);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);

        // Virtual functions of PVMp4H263EncExtensionInterface
//	OSCL_IMPORT_REF void addRef();
//	OSCL_IMPORT_REF void removeRef();
//	OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF bool SetNumLayers(uint32 aNumLayers);
        OSCL_IMPORT_REF bool SetOutputBitRate(uint32 aLayer, uint32 aBitRate);
        OSCL_IMPORT_REF bool SetOutputFrameSize(uint32 aLayer, uint32 aWidth, uint32 aHeight);
        OSCL_IMPORT_REF bool SetOutputFrameRate(uint32 aLayer, OsclFloat aFrameRate);
        OSCL_IMPORT_REF bool SetSegmentTargetSize(uint32 aLayer, uint32 aSizeBytes);
        OSCL_IMPORT_REF bool SetRateControlType(uint32 aLayer, PVMFVENRateControlType aRateControl);
        OSCL_IMPORT_REF bool SetDataPartitioning(bool aDataPartitioning);
        OSCL_IMPORT_REF bool SetRVLC(bool aRVLC);
        OSCL_IMPORT_REF bool SetIFrameInterval(uint32 aIFrameInterval);
        OSCL_IMPORT_REF bool GetVolHeader(OsclRefCounterMemFrag& aVolHeader);
        OSCL_IMPORT_REF bool RequestIFrame();
        OSCL_IMPORT_REF bool SetSceneDetection(bool aSCD);
        OSCL_IMPORT_REF bool SetCodec(PVMFFormatType aCodec);



        PVMFStatus SetCodecType(PVMFFormatType aCodec);

        OSCL_IMPORT_REF virtual bool SetFSIParam(uint8* aFSIBuff, int aFSIBuffLength);

        // from AudioEncExtensionInterface
        OSCL_IMPORT_REF PVMFStatus SetOutputBitRate(PVMF_GSMAMR_Rate aBitRate);
        OSCL_IMPORT_REF PVMFStatus SetMaxNumOutputFramesPerBuffer(uint32 aNumOutputFrames);
        OSCL_IMPORT_REF PVMFStatus SetOutputBitRate(uint32 aBitRate);
        OSCL_IMPORT_REF PVMFStatus SetOutputNumChannel(uint32 aNumChannel);
        OSCL_IMPORT_REF PVMFStatus SetOutputSamplingRate(uint32 aSamplingRate);


        PVMFStatus SetInputSamplingRate(uint32 aSamplingRate);
        PVMFStatus SetInputBitsPerSample(uint32 aBitsPerSample);
        PVMFStatus SetInputNumChannels(uint32 aNumChannels);
        uint32 GetOutputBitRate(); // for audio - void
        // encoder input format setting
        /////////////////////////////////////////////////////
        //      Encoder settings routine
        /////////////////////////////////////////////////////
        PVMFStatus SetInputFormat(PVMFFormatType aFormat);
        PVMFStatus SetInputFrameSize(uint32 aWidth, uint32 aHeight, uint8 aFrmOrient = 0);
        PVMFStatus SetInputFrameRate(OsclFloat aFrameRate);


        PVMFFormatType GetCodecType();
        uint32 GetOutputBitRate(uint32 aLayer);
        OsclFloat GetOutputFrameRate(uint32 aLayer);
        PVMFStatus GetOutputFrameSize(uint32 aLayer, uint32& aWidth, uint32& aHeight);
        uint32 GetIFrameInterval();
        uint32 GetOutputSamplingRate();
        uint32 GetOutputNumChannels();

    private:
        void CommandComplete(PVMFOMXEncNodeCmdQ& aCmdQ, PVMFOMXEncNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData = NULL);

        void DoQueryUuid(PVMFOMXEncNodeCommand&);
        void DoQueryInterface(PVMFOMXEncNodeCommand&);
        void DoRequestPort(PVMFOMXEncNodeCommand&);
        void DoReleasePort(PVMFOMXEncNodeCommand&);
        void DoInit(PVMFOMXEncNodeCommand&);
        void DoPrepare(PVMFOMXEncNodeCommand&);
        void DoStart(PVMFOMXEncNodeCommand&);
        void DoStop(PVMFOMXEncNodeCommand&);
        void DoPause(PVMFOMXEncNodeCommand&);
        void DoReset(PVMFOMXEncNodeCommand&);
        void DoFlush(PVMFOMXEncNodeCommand&);
        PVMFStatus DoGetNodeMetadataKey(PVMFOMXEncNodeCommand&);
        PVMFStatus DoGetNodeMetadataValue(PVMFOMXEncNodeCommand&);
        void DoCancelAllCommands(PVMFOMXEncNodeCommand&);
        void DoCancelCommand(PVMFOMXEncNodeCommand&);

        void Run();
        bool ProcessCommand(PVMFOMXEncNodeCommand& aCmd);
        bool ProcessIncomingMsg(PVMFPortInterface* aPort);
        bool ProcessOutgoingMsg(PVMFPortInterface* aPort);
        PVMFStatus HandleProcessingState();




        // DV:
        bool NegotiateVideoComponentParameters();
        bool SetMP4EncoderParameters();
        bool SetH263EncoderParameters();
        bool SetH264EncoderParameters();

        bool NegotiateAudioComponentParameters();
        bool SetAMREncoderParameters();
        bool SetAACEncoderParameters();

        bool SetDefaultCapabilityFlags();
        bool CreateOutMemPool(uint32 num);
        bool CreateInputMemPool(uint32 num);
        bool ProvideBuffersToComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
                                       uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                       uint32 aNumBuffers,    // number of buffers
                                       uint32 aActualBufferSize, // aactual buffer size
                                       uint32 aPortIndex,      // port idx
                                       bool aUseBufferOK,	// can component use OMX_UseBuffer?
                                       bool	aIsThisInputBuffer // is this input or output
                                      );

        bool FreeBuffersFromComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
                                      uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                      uint32 aNumBuffers,    // number of buffers
                                      uint32 aPortIndex,      // port idx
                                      bool	aIsThisInputBuffer		// is this input or output
                                     );

        OsclSharedPtr<class PVMFMediaDataImpl> WrapOutputBuffer(uint8 *pData, uint32 aDataLen, OsclAny *pContext);
        bool QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen);

        bool SendOutputBufferToOMXComponent();
        bool SendInputBufferToOMXComponent();


        bool SendEOSBufferToOMXComponent();

        bool SendBeginOfMediaStreamCommand(void);
        bool SendEndOfTrackCommand(void);



        bool ReleaseAllPorts();
        bool DeleteOMXEncoder();

        void ChangeNodeState(TPVMFNodeInterfaceState aNewState);

        void HandleComponentStateChange(OMX_U32 encoder_state);

        // Capability And Config Helper Methods
        PVMFStatus DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        PVMFStatus DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements);
        void DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        PVMFStatus DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements);

        PVMFStatus GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr aReqattr);
        PVMFStatus VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam);

        PVMFStatus GetProfileAndLevel(PVMF_MPEGVideoProfileType& aProfile, PVMF_MPEGVideoLevelType& aLevel);



        void freechunkavailable(OsclAny*);

        PVMFCommandId QueueCommandL(PVMFOMXEncNodeCommand& aCmd);


        int32 PushBackKeyVal(Oscl_Vector<PvmiKvp, OsclMemAllocator>*&, PvmiKvp&);
        int32 Push_Back_MetadataKeys(const char*);
        int32 Push_Back_MetadataKeys(PVMFMetadataList*&, uint32);
        int32 CreateNewArray(char*&, int32);
        int32 MemAllocate(OsclAny*& , OsclMemPoolFixedChunkAllocator*, uint32);

        bool ParseFullAVCFramesIntoNALs(OMX_BUFFERHEADERTYPE* aOutputBuffer);
        bool AVCAnnexBGetNALUnit(uint8 *bitstream, uint8 **nal_unit, int32 *size, bool getPtrOnly);

        friend class PVMFOMXEncPort;

        // Ports pointers
        PVMFPortInterface* iInPort;
        PVMFPortInterface* iOutPort;

        // Commands
        PVMFOMXEncNodeCmdQ iInputCommands;
        PVMFOMXEncNodeCmdQ iCurrentCommand;

        // Shared pointer for Media Msg.Input buffer
        PVMFSharedMediaDataPtr iDataIn;


        // OUTPUT BUFFER RELATED MEMBERS


        // Output buffer memory pool
        OsclMemPoolFixedChunkAllocator *iOutBufMemoryPool;

        // Memory pool for simple media data
        OsclMemPoolFixedChunkAllocator *iMediaDataMemPool;

        // Size of output buffer (negotiated with component)
        uint32 iOMXComponentOutputBufferSize;

        // size of output to allocate (OMX_ALLOCATE_BUFFER =  size of buf header )
        // (OMX_USE_BUFFER = size of buf header + iOMXCoponentOutputBufferSize)
        uint32 iOutputAllocSize;

        // Number of output buffers (negotiated with component)
        uint32 iNumOutputBuffers;

        // Number of output buffers in possession of the component or downstream,
        // namely, number of unavailable buffers
        uint32 iNumOutstandingOutputBuffers;

        // flag to prevent sending output buffers downstream during flushing etc.
        bool iDoNotSendOutputBuffersDownstreamFlag;

        // flag to prevent freeing the buffers twice
        bool iOutputBuffersFreed;


        // INPUT BUFFER RELATED MEMBERS
        OsclMemPoolFixedChunkAllocator *iInBufMemoryPool;
        uint32 iOMXComponentInputBufferSize; // size of input buffer that the component sees (negotiated with the component)
        uint32 iInputAllocSize; 	// size of input buffer to allocate (OMX_ALLOCATE_BUFFER =  size of buf header )
        // (OMX_USE_BUFFER = size of buf header + iOMXCoponentInputBufferSize)
        uint32 iNumInputBuffers; // total num of input buffers (negotiated with component)

        uint32 iNumOutstandingInputBuffers; // number of input buffers in use (i.e. unavailable)

        bool iDoNotSaveInputBuffersFlag;

        // flag to prevent freeing buffers twice
        bool iInputBuffersFreed;

        // input buffer fragmentation etc.
        uint32 iCopyPosition;				// for copying memfrag data into a buffer
        uint32 iFragmentSizeRemainingToCopy;
        bool	iIsNewDataFragment;

        // input data info
        uint32 iCurrFragNum;
        uint32 iCodecSeqNum;	// sequence number tracking
        uint32 iInPacketSeqNum;

        uint32 iInTimestamp;
        uint32 iInDuration;
        uint32 iInNumFrags;
        uint32 iCurrentMsgMarkerBit;

        // DYNAMIC PORT RE-CONFIGURATION
        uint32 iInputPortIndex;
        uint32 iOutputPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE iParamPort;
        uint32 iPortIndexForDynamicReconfig;
        bool iSecondPortReportedChange;
        bool iDynamicReconfigInProgress;
        uint32 iSecondPortToReconfig;



        OMX_BUFFERHEADERTYPE *iInputBufferToResendToComponent; // ptr to input buffer that is not empty, but that the OMX component returned
        // we need to resend this same buffer back to the component


        ////////////////// OMX COMPONENT CAPABILITY RELATED MEMBERS
        bool iOMXComponentSupportsExternalOutputBufferAlloc;
        bool iOMXComponentSupportsExternalInputBufferAlloc;
        bool iOMXComponentSupportsMovableInputBuffers;
        bool iOMXComponentUsesNALStartCodes;
        bool iOMXComponentSupportsPartialFrames;
        bool iOMXComponentCanHandleIncompleteFrames;
        bool iOMXComponentUsesFullAVCFrames;

        bool iSetMarkerBitForEveryFrag;
        bool iIsOMXComponentMultiThreaded;

        // State definitions for HandleProcessingState() state machine
        typedef enum
        {
            EPVMFOMXEncNodeProcessingState_Idle,                  //default state after constraction/reset
            EPVMFOMXEncNodeProcessingState_ReadyToEncode,         //nornal operation state of the encoder
            EPVMFOMXEncNodeProcessingState_WaitForOutputBuffer,   //wait state for avalible media output buffer
            EPVMFOMXEncNodeProcessingState_WaitForOutputPort,     //wait state, output port is busy
            EPVMFOMXEncNodeProcessingState_WaitForOutgoingQueue,   //wait state, outgoing queue
            EPVMFOMXEncNodeProcessingState_PortReconfig,			// Dynamic Port Reconfiguration - step 1
            EPVMFOMXEncNodeProcessingState_WaitForBufferReturn,		//	step 2
            EPVMFOMXEncNodeProcessingState_WaitForPortDisable,		// Dynamic Port Reconfiguration - step 3
            EPVMFOMXEncNodeProcessingState_PortReEnable,			// Dynamic Port Reconfiguration - step 4
            EPVMFOMXEncNodeProcessingState_WaitForPortEnable,		// step 5
            EPVMFOMXEncNodeProcessingState_Stopping				// when STOP command is issued, the node has to wait for component to transition into
            // idle state. The buffers keep coming back , the node is rescheduled
            // to run. Prevent the node from sending buffers back
        } PVMFOMXEncNode_ProcessingState;

        // State of HandleProcessingState() state machine
        PVMFOMXEncNode_ProcessingState iProcessingState;

        // Handle of OMX Component
        OMX_HANDLETYPE iOMXEncoder;

        // Current State of the component
        OMX_STATETYPE iCurrentEncoderState;

        // BOS
        bool iSendBOS;
        uint32 iStreamID;
        uint32 iBOSTimestamp;

        //EOS control flags
        bool iIsEOSSentToComponent;
        bool iIsEOSReceivedFromComponent;

        // Send Fsi configuration flag
        bool	sendYuvFsi;

        // Width of encoded frame
        int32 iYUVWidth;
        // Height of encoded frame
        int32 iYUVHeight;



        // Pointer to input data fragment
        uint8* iBitstreamBuffer;
        // Size of input data fragment
        int32 iBitstreamSize;

        // Output frame sequence counter
        uint32 iSeqNum;

        // Input frame sequence counter
        uint32 iSeqNum_In;

        // Added to Scheduler Flag
        bool iIsAdded;

        // Log related
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;
        PVLogger *iRunlLogger;
        const char *iNodeTypeId;

        // Counter of fragment read from current Media Msg.Input buffer
        uint fragnum;
        // Number of fragments in the Media Msg.Input buffer
        uint numfrags;


        // Node configuration update
        //PVMFOMXEncNodeConfig iNodeConfig;

        // Capability exchange
        PVMFNodeCapability iCapability;

        // Reference counter for extension
        uint32 iExtensionRefCount;

        // Vector for KVP
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;

        // EOS flag
        bool iEndOfDataReached;
        // Time stame upon EOS
        PVMFTimestamp iEndOfDataTimestamp;

        /* Diagnostic log related */
        PVLogger* iDiagnosticsLogger;
        bool iDiagnosticsLogged;
        void LogDiagnostics();

        uint32 iFrameCounter;



        uint32 iAvgBitrateValue;
        bool iResetInProgress;
        bool iResetMsgSent;
        bool iStopInResetMsgSent;

        PV_VideoEncodeParam iVideoEncodeParam;
        PV_VideoInputFormat iVideoInputFormat;
        PV_AudioEncodeParam iAudioEncodeParam;
        PV_AudioInputFormat iAudioInputFormat;

        PVMFFormatType iInFormat;
        PVMFFormatType iOutFormat;

        PVMFOMXEncNodeAllocDestructDealloc iAlloc;
        OsclRefCounterMemFrag iVolHeader; /** Vol header */
        OsclRefCounterMemFrag iConfigHeader;

        OsclRefCounterMemFrag iParamSet; /* place holder for sps,pps values */
        OsclMemoryFragment iSPSs[PVMF_AVCENC_NODE_SPS_VECTOR_RESERVE]; // just ptrs
        OsclMemoryFragment iPPSs[PVMF_AVCENC_NODE_PPS_VECTOR_RESERVE]; // just ptrs
        int   iNumSPSs;
        int	  iNumPPSs;
        bool iSpsPpsSequenceOver;
        OsclSharedPtr<PVMFMediaDataImpl> iPreviousMediaData;
        bool iFirstNAL;
        uint32* iNALSizeArray;
        uint8** iNALPtrArray;
        uint32 iNALSizeArrayMaxElems;
        uint32 iNumNALs;
        uint32 iFirstNALStartCodeSize;

        uint32 iEndOfFrameFlagPrevious;
        uint32 iKeyFrameFlagPrevious;
        uint32 iEndOfNALFlagPrevious;
        uint32 iTimeStampPrevious;
        uint32 iBufferLenPrevious;

        uint32 iEndOfFrameFlagOut;
        uint32 iKeyFrameFlagOut;
        uint32 iEndOfNALFlagOut;
        uint32 iTimeStampOut;
        uint32 iBufferLenOut;
        OsclAny **out_ctrl_struct_ptr ;
        OsclAny **out_buff_hdr_ptr ;
        OsclAny **in_ctrl_struct_ptr ;
        OsclAny **in_buff_hdr_ptr ;



#ifdef _TEST_AE_ERROR_HANDLING
        bool   iErrorHandlingInit;
        uint32 iErrorHandlingEncodeCount;
        int    iCountFrames;
        uint32 iErrorNodeCmd;
        bool iErrorConfigHeader;
        uint32 iErrorEncodeFlag;
        uint32 iErrorDataPathStall;
#endif
};


#endif // PVMF_OMXENC_NODE_H_INCLUDED

