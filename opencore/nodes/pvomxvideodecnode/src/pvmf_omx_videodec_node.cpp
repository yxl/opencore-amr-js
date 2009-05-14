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
#include "pvmf_omx_videodec_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_omx_basedec_port.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmi_kvp_util.h"
// needed for capability and config
#include "pv_omx_config_parser.h"


#include "OMX_Core.h"
#include "pvmf_omx_basedec_callbacks.h"     //used for thin AO in Decoder's callbacks
#include "pv_omxcore.h"
#include "OMX_Video.h"

#define CONFIG_SIZE_AND_VERSION(param) \
	    param.nSize=sizeof(param); \
	    param.nVersion.s.nVersionMajor = SPECVERSIONMAJOR; \
	    param.nVersion.s.nVersionMinor = SPECVERSIONMINOR; \
	    param.nVersion.s.nRevision = SPECREVISION; \
	    param.nVersion.s.nStep = SPECSTEP;


#define PVOMXVIDEODEC_EXTRA_YUVBUFFER_POOLNUM 3
#define PVOMXVIDEODEC_MEDIADATA_POOLNUM (PVOMXVIDEODECMAXNUMDPBFRAMESPLUS1 + PVOMXVIDEODEC_EXTRA_YUVBUFFER_POOLNUM)


// Node default settings
#define PVOMXVIDEODECNODE_CONFIG_POSTPROCENABLE_DEF false
#define PVOMXVIDEODECNODE_CONFIG_POSTPROCTYPE_DEF 0  // 0 (nopostproc),1(deblock),3(deblock&&dering)
#define PVOMXVIDEODECNODE_CONFIG_DROPFRAMEENABLE_DEF false
// H263 default settings
#define PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_DEF 40000
#define PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MIN 20000
#define PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MAX 120000
#define PVOMXVIDEODECNODE_CONFIG_H263MAXWIDTH_DEF 352
#define PVOMXVIDEODECNODE_CONFIG_H263MAXHEIGHT_DEF 288
#define PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MIN 4
#define PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MAX 352
// M4v default settings
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_DEF 40000
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MIN 20000
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MAX 120000
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXWIDTH_DEF 352
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXHEIGHT_DEF 288
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MIN 4
#define PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MAX 352

// AVC default settings
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXBITSTREAMFRAMESIZE_DEF 20000
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXBITSTREAMFRAMESIZE_MIN 20000
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXBITSTREAMFRAMESIZE_MAX 120000
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXWIDTH_DEF 352
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXHEIGHT_DEF 288
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXDIMENSION_MIN 4
#define PVOMXVIDEODECNODE_CONFIG_AVCMAXDIMENSION_MAX 352

/* WMV default settings */
#define PVOMXVIDEODECNODE_CONFIG_WMVMAXWIDTH_DEF 352
#define PVOMXVIDEODECNODE_CONFIG_WMVMAXHEIGHT_DEF 288


#define PVMF_OMXVIDEODEC_NUM_METADATA_VALUES 6

// Constant character strings for metadata keys
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY[] = "codec-info/video/format";
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY[] = "codec-info/video/width";
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY[] = "codec-info/video/height";
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY[] = "codec-info/video/profile";
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY[] = "codec-info/video/level";
static const char PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY[] = "codec-info/video/avgbitrate";//(bits per sec)

static const char PVOMXVIDEODECMETADATA_SEMICOLON[] = ";";



/////////////////////////////////////////////////////////////////////////////
// Class Destructor
/////////////////////////////////////////////////////////////////////////////
PVMFOMXVideoDecNode::~PVMFOMXVideoDecNode()
{
    ReleaseAllPorts();
}

/////////////////////////////////////////////////////////////////////////////
// Add AO to the scheduler
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXVideoDecNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXVideoDecNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
            {
                AddToScheduler();
                iIsAdded = true;
            }
            iLogger = PVLogger::GetLoggerObject("PVMFOMXVideoDecNode");
            iRunlLogger = PVLogger::GetLoggerObject("Run.PVMFOMXVideoDecNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath");
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.decnode.OMXVideoDecnode");

            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}

/////////////////////
// Private Section //
/////////////////////

/////////////////////////////////////////////////////////////////////////////
// Class Constructor
/////////////////////////////////////////////////////////////////////////////
PVMFOMXVideoDecNode::PVMFOMXVideoDecNode(int32 aPriority) :
        PVMFOMXBaseDecNode(aPriority, "PVMFOMXVideoDecNode"),
        iH263MaxBitstreamFrameSize(PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_DEF),
        iH263MaxWidth(PVOMXVIDEODECNODE_CONFIG_H263MAXWIDTH_DEF),
        iH263MaxHeight(PVOMXVIDEODECNODE_CONFIG_H263MAXHEIGHT_DEF),
        iM4VMaxBitstreamFrameSize(PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_DEF),
        iM4VMaxWidth(PVOMXVIDEODECNODE_CONFIG_M4VMAXWIDTH_DEF),
        iM4VMaxHeight(PVOMXVIDEODECNODE_CONFIG_M4VMAXHEIGHT_DEF),
        iNewWidth(0),
        iNewHeight(0)
{
    iInterfaceState = EPVMFNodeCreated;

    iNodeConfig.iPostProcessingEnable = PVOMXVIDEODECNODE_CONFIG_POSTPROCENABLE_DEF;
    iNodeConfig.iPostProcessingMode = PVOMXVIDEODECNODE_CONFIG_POSTPROCTYPE_DEF;
    iNodeConfig.iDropFrame = PVOMXVIDEODECNODE_CONFIG_DROPFRAMEENABLE_DEF;
    iNodeConfig.iMimeType = PVMF_MIME_FORMAT_UNKNOWN;


    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_OMXBASEDEC_NODE_COMMAND_ID_START, PVMF_OMXBASEDEC_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = true;
             iCapability.iMaxNumberOfPorts = 2;
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_MP4);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_RAW);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_H2631998);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_WMV);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_YUV420);

             iAvailableMetadataKeys.reserve(PVMF_OMXVIDEODEC_NUM_METADATA_VALUES);
             iAvailableMetadataKeys.clear();
            );

    // need to init this allocator since verifyParameterSync (using the buffers) may be called through
    // port interface before anything else happens.
    OSCL_TRY(err, iFsiFragmentAlloc.size(PVOMXVIDEODEC_MEDIADATA_POOLNUM, sizeof(PVMFYuvFormatSpecificInfo0)));

    OSCL_TRY(err, iPrivateDataFsiFragmentAlloc.size(PVOMXVIDEODEC_MEDIADATA_POOLNUM, sizeof(OsclAny *)));

    iLastYUVWidth = 0;
    iLastYUVHeight = 0;
}

/////////////////////////////////////////////////////////////////////////////
// This routine will handle the PortReEnable state
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXVideoDecNode::HandlePortReEnable()
{
    // set the port index so that we get parameters for the proper port
    iParamPort.nPortIndex = iPortIndexForDynamicReconfig;

    CONFIG_SIZE_AND_VERSION(iParamPort);

    // get new parameters of the port
    OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

    // send command for port re-enabling (for this to happen, we must first recreate the buffers)
    OMX_SendCommand(iOMXDecoder, OMX_CommandPortEnable, iPortIndexForDynamicReconfig, NULL);

    // is this output port?
    if (iPortIndexForDynamicReconfig == iOutputPortIndex)
    {
        iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & (~15)) * ((iParamPort.format.video.nFrameHeight + 15) & (~15)) * 3 / 2;

        // check the new buffer size
        if (iInPort)
        {
            if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO ||
                    ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                    ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                    ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                    ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 ||
                    ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
            {
                iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & (~15)) * ((iParamPort.format.video.nFrameHeight + 15) & (~15)) * 3 / 2;
            }
            else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV) // This is a requirement for the WMV decoder that we have currently
            {
                iOMXComponentOutputBufferSize = ((iParamPort.format.video.nFrameWidth + 3) & (~3)) * (iParamPort.format.video.nFrameHeight) * 3 / 2;
            }
            else
            {
                OSCL_ASSERT(false);
            }
        }
        // set the new width / height
        iYUVWidth =  iParamPort.format.video.nFrameWidth;
        iYUVHeight = iParamPort.format.video.nFrameHeight;

        if (iOMXComponentOutputBufferSize < iParamPort.nBufferSize)
            iOMXComponentOutputBufferSize = iParamPort.nBufferSize;

        // do we need to increase the number of buffers?
        if (iNumOutputBuffers < iParamPort.nBufferCountMin)
            iNumOutputBuffers = iParamPort.nBufferCountMin;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::HandlePortReEnable() new output buffers %d, size %d", iNumOutputBuffers, iOMXComponentOutputBufferSize));



        // Before allocating new set of output buffers, re-send Video FSI to
        // media output node in case of dynamic port reconfiguration

        sendFsi = true;
        iCompactFSISettingSucceeded = false;

        iLastYUVWidth = iYUVWidth ;
        iLastYUVHeight = iYUVHeight;

        // Check if Fsi configuration need to be sent
        if (sendFsi)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Re-sending YUV FSI after Dynamic port reconfiguration"));

            int fsiErrorCode = 0;
            OsclRefCounterMemFrag yuvFsiMemfrag;

            OSCL_TRY(fsiErrorCode, yuvFsiMemfrag = iFsiFragmentAlloc.get(););

            OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                 (0, "PVMFOMXVideoDecNode::HandlePortReEnable() Failed to allocate memory for FSI")));

            if (fsiErrorCode == 0)
            {
                PVMFYuvFormatSpecificInfo0* fsiInfo = OSCL_PLACEMENT_NEW(yuvFsiMemfrag.getMemFragPtr(), PVMFYuvFormatSpecificInfo0());
                if (fsiInfo != NULL)
                {
                    fsiInfo->uid = PVMFYuvFormatSpecificInfo0_UID;
                    fsiInfo->video_format = iYUVFormat;
                    fsiInfo->display_width = iYUVWidth;
                    fsiInfo->display_height = iYUVHeight;
                    fsiInfo->num_buffers = iNumOutputBuffers;
                    fsiInfo->buffer_size = iOMXComponentOutputBufferSize;

                    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                    {
                        fsiInfo->width = (iYUVWidth + 15) & (~15);
                        fsiInfo->height = (iYUVHeight + 15) & (~15);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                    {
                        fsiInfo->width = (iYUVWidth + 3) & -4;
                        fsiInfo->height = iYUVHeight;
                    }
                    else
                    {
                        fsiInfo->width = iYUVWidth;
                        fsiInfo->height = iYUVHeight;
                    }

                    OsclMemAllocator alloc;
                    int32 KeyLength = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY_YUV) + 1;
                    PvmiKeyType KvpKey = (PvmiKeyType)alloc.ALLOCATE(KeyLength);

                    if (NULL == KvpKey)
                    {
                        return false;
                    }

                    oscl_strncpy(KvpKey, PVMF_FORMAT_SPECIFIC_INFO_KEY_YUV, KeyLength);
                    int32 err;

                    OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(yuvFsiMemfrag, KvpKey););
                    if (err != OsclErrNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Problem to set FSI"));

                    }
                    else
                    {
                        sendFsi = false;
                        iCompactFSISettingSucceeded = true;
                    }



                    alloc.deallocate((OsclAny*)(KvpKey));
                    fsiInfo->video_format.~PVMFFormatType();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Problem allocating Output FSI"));
                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return false; // this is going to make everything go out of scope
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Problem allocating Output FSI"));
                return false; // this is going to make everything go out of scope
            }


        }

        //Buffer allocation has to be done again in case we landed to port reconfiguration
        PvmiKvp* kvp = NULL;
        int numKvp = 0;
        PvmiKeyType aIdentifier = (PvmiKeyType)PVMF_BUFFER_ALLOCATOR_KEY;
        int32 err, err1;
        ipExternalOutputBufferAllocatorInterface = NULL;

        OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiGetBufferAllocatorSpecificInfoSync(aIdentifier, kvp, numKvp););

        if ((err == OsclErrNone) && (NULL != kvp))
        {
            ipExternalOutputBufferAllocatorInterface = (PVInterface*) kvp->value.key_specific_value;

            if (ipExternalOutputBufferAllocatorInterface)
            {
                PVInterface* pTempPVInterfacePtr = NULL;

                OSCL_TRY(err, ipExternalOutputBufferAllocatorInterface->queryInterface(PVMFFixedSizeBufferAllocUUID, pTempPVInterfacePtr););

                OSCL_TRY(err1, ((PVMFOMXDecPort*)iOutPort)->releaseParametersSync(kvp, numKvp););

                if (err1 != OsclErrNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Unable to Release Parameters"));
                }

                if ((err == OsclErrNone) && (NULL != pTempPVInterfacePtr))
                {
                    ipFixedSizeBufferAlloc = OSCL_STATIC_CAST(PVMFFixedSizeBufferAlloc*, pTempPVInterfacePtr);

                    uint32 iNumBuffers, iBufferSize;

                    iNumBuffers = ipFixedSizeBufferAlloc->getNumBuffers();
                    iBufferSize = ipFixedSizeBufferAlloc->getBufferSize();

                    if ((iNumBuffers < iParamPort.nBufferCountMin) || (iBufferSize < iOMXComponentOutputBufferSize))
                    {
                        ipExternalOutputBufferAllocatorInterface->removeRef();
                        ipExternalOutputBufferAllocatorInterface = NULL;
                    }
                    else
                    {
                        iNumOutputBuffers = iNumBuffers;
                        iOMXComponentOutputBufferSize = iBufferSize;
                    }
                }
                else
                {
                    ipExternalOutputBufferAllocatorInterface->removeRef();
                    ipExternalOutputBufferAllocatorInterface = NULL;
                }
            }
        }


        /* Allocate output buffers */
        if (!CreateOutMemPool(iNumOutputBuffers))
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXVideoDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot allocate output buffers "));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return PVMFErrNoMemory;
        }

        if (out_ctrl_struct_ptr == NULL)
        {

            out_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::HandlePortReEnable() out_ctrl_struct_ptr == NULL"));

                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return PVMFErrNoMemory;
            }
        }

        if (out_buff_hdr_ptr == NULL)
        {

            out_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumOutputBuffers * sizeof(OsclAny *));

            if (out_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::HandlePortReEnable()  out_buff_hdr_ptr == NULL"));

                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return PVMFErrNoMemory;
            }
        }


        if (!ProvideBuffersToComponent(iOutBufMemoryPool, // allocator
                                       iOutputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                       iNumOutputBuffers, // number of buffers
                                       iOMXComponentOutputBufferSize, // actual buffer size
                                       iOutputPortIndex, // port idx
                                       iOMXComponentSupportsExternalOutputBufferAlloc, // can component use OMX_UseBuffer
                                       false // this is not input
                                      ))
        {


            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXVideoDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot provide output buffers to component"));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return PVMFErrNoMemory;

        }

        // do not drop output any more, i.e. enable output to be sent downstream
        iDoNotSendOutputBuffersDownstreamFlag = false;


    }
    else
    {
        // this is input port

        iOMXComponentInputBufferSize = iParamPort.nBufferSize;
        // do we need to increase the number of buffers?
        if (iNumInputBuffers < iParamPort.nBufferCountMin)
            iNumInputBuffers = iParamPort.nBufferCountMin;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::HandlePortReEnable() new buffers %d, size %d", iNumInputBuffers, iOMXComponentInputBufferSize));

        /* Allocate input buffers */
        if (!CreateInputMemPool(iNumInputBuffers))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXVideoDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot allocate new input buffers to component"));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return PVMFErrNoMemory;
        }

        if (in_ctrl_struct_ptr == NULL)
        {

            in_ctrl_struct_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_ctrl_struct_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::HandlePortReEnable() in_ctrl_struct_ptr == NULL"));

                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return PVMFErrNoMemory;
            }
        }

        if (in_buff_hdr_ptr == NULL)
        {

            in_buff_hdr_ptr = (OsclAny **) oscl_malloc(iNumInputBuffers * sizeof(OsclAny *));

            if (in_buff_hdr_ptr == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::HandlePortReEnable()  in_buff_hdr_ptr == NULL"));

                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return PVMFErrNoMemory;
            }
        }

        if (!ProvideBuffersToComponent(iInBufMemoryPool, // allocator
                                       iInputAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                       iNumInputBuffers, // number of buffers
                                       iOMXComponentInputBufferSize, // actual buffer size
                                       iInputPortIndex, // port idx
                                       iOMXComponentSupportsExternalInputBufferAlloc, // can component use OMX_UseBuffer
                                       true // this is input
                                      ))
        {


            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXVideoDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot provide new input buffers to component"));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrNoMemory);
            return PVMFErrNoMemory;

        }
        // do not drop partially consumed input
        iDoNotSaveInputBuffersFlag = false;


    }

    // if the callback that the port was re-enabled has not arrived yet, wait for it
    // if it has arrived, it will set the state to either PortReconfig or to ReadyToDecode
    if (iProcessingState != EPVMFOMXBaseDecNodeProcessingState_PortReconfig &&
            iProcessingState != EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode)
        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_WaitForPortEnable;

    return PVMFSuccess; // allow rescheduling of the node
}
////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXVideoDecNode::NegotiateComponentParameters(OMX_PTR aOutputParameters)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() In"));

    OMX_ERRORTYPE Err;
    // first get the number of ports and port indices
    OMX_PORT_PARAM_TYPE VideoPortParameters;
    uint32 NumPorts;
    uint32 ii;

    //set version and size;
    CONFIG_SIZE_AND_VERSION(VideoPortParameters);
    // get starting number
    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamVideoInit, &VideoPortParameters);
    NumPorts = VideoPortParameters.nPorts; // must be at least 2 of them (in&out)

    if (Err != OMX_ErrorNone || NumPorts < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() There is insuffucient (%d) ports", NumPorts));
        return false;
    }


    // loop through video ports starting from the starting index to find index of the first input port
    for (ii = VideoPortParameters.nStartPortNumber ;ii < VideoPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has input direction is picked

        CONFIG_SIZE_AND_VERSION(iParamPort);

        //port
        iParamPort.nPortIndex = ii; // iInputPortIndex; //OMF_MC_H264D_PORT_INDEX_OF_STREAM;
        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem negotiating with port %d ", ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Found Input port index %d ", ii));

            iInputPortIndex = ii;
            break;
        }
    }
    if (ii == VideoPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Cannot find any input port "));
        return false;
    }


    // loop through video ports starting from the starting index to find index of the first output port
    for (ii = VideoPortParameters.nStartPortNumber ;ii < VideoPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has output direction is picked

        CONFIG_SIZE_AND_VERSION(iParamPort);

        //port
        iParamPort.nPortIndex = ii;
        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem negotiating with port %d ", ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirOutput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Found Output port index %d ", ii));

            iOutputPortIndex = ii;
            break;
        }
    }
    if (ii == VideoPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Cannot find any output port "));
        return false;
    }



    // now get input parameters
    CONFIG_SIZE_AND_VERSION(iParamPort);

    //Input port
    iParamPort.nPortIndex = iInputPortIndex;
    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem negotiating with input port %d ", iInputPortIndex));
        return false;
    }

    // preset the number of input buffers

    //iNumInputBuffers = NUMBER_INPUT_BUFFER;
    iNumInputBuffers = iParamPort.nBufferCountActual;  // use the value provided by component

    // do we need to increase the number of buffers?
    if (iNumInputBuffers < iParamPort.nBufferCountMin)
        iNumInputBuffers = iParamPort.nBufferCountMin;
    iOMXComponentInputBufferSize = iParamPort.nBufferSize;

    iParamPort.nBufferCountActual = iNumInputBuffers;

    // set the number of actual input buffers
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Inport buffers %d,size %d", iNumInputBuffers, iOMXComponentInputBufferSize));


    VideoOMXConfigParserOutputs *pOutputParameters;

    pOutputParameters = (VideoOMXConfigParserOutputs *)aOutputParameters;


    // set the width/height on INPUT port parameters (this may change during port reconfig)
    if ((pOutputParameters->width != 0) && (pOutputParameters->height != 0))
    {
        iParamPort.format.video.nFrameWidth = pOutputParameters->width;
        iParamPort.format.video.nFrameHeight = pOutputParameters->height;
    }

    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting parameters in input port %d ", iInputPortIndex));
        return false;
    }

    //Port 1 for output port
    iParamPort.nPortIndex = iOutputPortIndex;
    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem negotiating with output port %d ", iOutputPortIndex));
        return false;
    }

    // check if params are OK. In case of H263, width/height cannot be obtained until
    // 1st frame is decoded, so read them from the output port.
    // otherwise, used Width/Height from the config parser utility
    // set the width/height based on port parameters (this may change during port reconfig)
    if ((pOutputParameters->width != 0) && (pOutputParameters->height != 0) && iInPort && (((PVMFOMXDecPort*)iInPort)->iFormat != PVMF_MIME_H2631998 || ((PVMFOMXDecPort*)iInPort)->iFormat != PVMF_MIME_H2632000))
    {
        iYUVWidth  = pOutputParameters->width;
        iYUVHeight = pOutputParameters->height;
    }
    else
    {
        iYUVWidth =  iParamPort.format.video.nFrameWidth;
        iYUVHeight = iParamPort.format.video.nFrameHeight;
    }

    //iNumOutputBuffers = NUMBER_OUTPUT_BUFFER;
    iNumOutputBuffers = iParamPort.nBufferCountActual;
    if (iNumOutputBuffers > NUMBER_OUTPUT_BUFFER)
        iNumOutputBuffers = NUMBER_OUTPUT_BUFFER; // make sure number of output buffers is not larger than port queue size
    iOMXComponentOutputBufferSize = iParamPort.nBufferSize;
    if (iNumOutputBuffers < iParamPort.nBufferCountMin)
        iNumOutputBuffers = iParamPort.nBufferCountMin;

    //Send the FSI information to media output node here, before setting output
    //port parameters to the omx component

    if (iLastYUVWidth != iYUVWidth || iYUVHeight != iLastYUVHeight)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters - Sending YUV FSI"));

        // set a flag to send Fsi configuration
        sendFsi = true;
        iCompactFSISettingSucceeded = false;
        //store new values for reference
        iLastYUVWidth = iYUVWidth ;
        iLastYUVHeight = iYUVHeight;
    }

    // Check if Fsi configuration need to be sent
    if (sendFsi)
    {
        int fsiErrorCode = 0;
        OsclRefCounterMemFrag yuvFsiMemfrag;

        OSCL_TRY(fsiErrorCode, yuvFsiMemfrag = iFsiFragmentAlloc.get(););

        OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                             (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Failed to allocate memory for FSI")));

        if (fsiErrorCode == 0)
        {
            PVMFYuvFormatSpecificInfo0* fsiInfo = OSCL_PLACEMENT_NEW(yuvFsiMemfrag.getMemFragPtr(), PVMFYuvFormatSpecificInfo0());
            if (fsiInfo != NULL)
            {
                fsiInfo->uid = PVMFYuvFormatSpecificInfo0_UID;
                fsiInfo->video_format = iYUVFormat;
                fsiInfo->display_width = iYUVWidth;
                fsiInfo->display_height = iYUVHeight;
                fsiInfo->num_buffers = iNumOutputBuffers;
                fsiInfo->buffer_size = iOMXComponentOutputBufferSize;

                if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO ||
                        ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                        ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                        ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                        ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 ||
                        ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                {
                    fsiInfo->width = (iYUVWidth + 15) & (~15);
                    fsiInfo->height = (iYUVHeight + 15) & (~15);
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                {
                    fsiInfo->width = (iYUVWidth + 3) & -4;
                    fsiInfo->height = iYUVHeight;
                }
                else
                {
                    fsiInfo->width = iYUVWidth;
                    fsiInfo->height = iYUVHeight;
                }

                OsclMemAllocator alloc;
                int32 KeyLength = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY_YUV) + 1;
                PvmiKeyType KvpKey = (PvmiKeyType)alloc.ALLOCATE(KeyLength);

                if (NULL == KvpKey)
                {
                    return false;
                }

                oscl_strncpy(KvpKey, PVMF_FORMAT_SPECIFIC_INFO_KEY_YUV, KeyLength);
                int32 err;

                OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(yuvFsiMemfrag, KvpKey););

                if (err != OsclErrNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters - Problem to set FSI"));


                }
                else
                {
                    iCompactFSISettingSucceeded = true;
                    sendFsi = false;
                }


                alloc.deallocate((OsclAny*)(KvpKey));
                fsiInfo->video_format.~PVMFFormatType();
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters - Problem allocating Output FSI"));
                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return false; // this is going to make everything go out of scope
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters - Problem allocating Output FSI"));
            return false; // this is going to make everything go out of scope
        }


    }

    //Try querying the buffer allocator KVP for output buffer allocation outside the node

    PvmiKvp* kvp = NULL;
    int numKvp = 0;
    PvmiKeyType aIdentifier = (PvmiKeyType)PVMF_BUFFER_ALLOCATOR_KEY;
    int32 err, err1;
    ipExternalOutputBufferAllocatorInterface = NULL;

    OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiGetBufferAllocatorSpecificInfoSync(aIdentifier, kvp, numKvp););

    if ((err == OsclErrNone) && (NULL != kvp))
    {
        ipExternalOutputBufferAllocatorInterface = (PVInterface*) kvp->value.key_specific_value;

        if (ipExternalOutputBufferAllocatorInterface)
        {
            PVInterface* pTempPVInterfacePtr = NULL;

            OSCL_TRY(err, ipExternalOutputBufferAllocatorInterface->queryInterface(PVMFFixedSizeBufferAllocUUID, pTempPVInterfacePtr););

            OSCL_TRY(err1, ((PVMFOMXDecPort*)iOutPort)->releaseParametersSync(kvp, numKvp););

            if (err1 != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters - Unable to Release Parameters"));
            }


            if ((err == OsclErrNone) && (NULL != pTempPVInterfacePtr))
            {
                ipFixedSizeBufferAlloc = OSCL_STATIC_CAST(PVMFFixedSizeBufferAlloc*, pTempPVInterfacePtr);

                uint32 iNumBuffers, iBufferSize;

                iNumBuffers = ipFixedSizeBufferAlloc->getNumBuffers();
                iBufferSize = ipFixedSizeBufferAlloc->getBufferSize();

                if ((iNumBuffers < iParamPort.nBufferCountMin) || (iBufferSize < iOMXComponentOutputBufferSize))
                {
                    ipExternalOutputBufferAllocatorInterface->removeRef();
                    ipExternalOutputBufferAllocatorInterface = NULL;
                }
                else
                {
                    iNumOutputBuffers = iNumBuffers;
                    iOMXComponentOutputBufferSize = iBufferSize;
                }
            }
            else
            {
                ipExternalOutputBufferAllocatorInterface->removeRef();
                ipExternalOutputBufferAllocatorInterface = NULL;

            }
        }
    }


    iParamPort.nBufferCountActual = iNumOutputBuffers;
    CONFIG_SIZE_AND_VERSION(iParamPort);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Outport buffers %d,size %d", iNumOutputBuffers, iOMXComponentOutputBufferSize));

    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting parameters in output port %d ", iOutputPortIndex));
        return false;
    }



    // Get video color format
    OMX_VIDEO_PARAM_PORTFORMATTYPE VideoPortFormat;
    // init to unknown
    iOMXVideoColorFormat = OMX_COLOR_FormatUnused;
    CONFIG_SIZE_AND_VERSION(VideoPortFormat);
    VideoPortFormat.nPortIndex = iOutputPortIndex;

    VideoPortFormat.nIndex = 0; // read the preferred format - first

// doing this in a while loop while incrementing nIndex will get all supported formats
// until component says OMX_ErrorNoMore
// For now, we just use the preferred one (with nIndex=0) assuming it is supported at MIO

    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamVideoPortFormat, &VideoPortFormat);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem getting video port format"));
        return false;
    }
    // check if color format is valid
    if (VideoPortFormat.eCompressionFormat == OMX_VIDEO_CodingUnused)
    {
        // color format is valid, so read it
        iOMXVideoColorFormat = VideoPortFormat.eColorFormat;


        // Now set the format to confirm parameters
        CONFIG_SIZE_AND_VERSION(VideoPortFormat);

        Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamVideoPortFormat, &VideoPortFormat);
        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting video port format"));
            return false;
        }
    }

    // now that we have the color format, interpret it
    if (iOMXVideoColorFormat == OMX_COLOR_Format8bitRGB332)
    {
        iYUVFormat = PVMF_MIME_RGB8;
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_Format12bitRGB444)
    {
        iYUVFormat = PVMF_MIME_RGB12;
    }
    else if (iOMXVideoColorFormat >= OMX_COLOR_Format16bitARGB4444 && iOMXVideoColorFormat <= OMX_COLOR_Format16bitBGR565)
    {
        iYUVFormat = PVMF_MIME_RGB16;
    }
    else if (iOMXVideoColorFormat >= OMX_COLOR_Format24bitRGB888 && iOMXVideoColorFormat <= OMX_COLOR_Format24bitARGB1887)
    {
        iYUVFormat = PVMF_MIME_RGB24;
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV420Planar)
    {
        iYUVFormat = PVMF_MIME_YUV420_PLANAR; // Y, U, V are separate - entire planes
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV420PackedPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV420_PACKEDPLANAR; // each slice contains Y,U,V separate
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV420SemiPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV420_SEMIPLANAR; // Y and UV interleaved - entire planes
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV420_PACKEDSEMIPLANAR; // Y and UV interleaved - sliced
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV422Planar)
    {
        iYUVFormat = PVMF_MIME_YUV422_PLANAR; // Y, U, V are separate - entire planes
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV422PackedPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV422_PACKEDPLANAR; // each slice contains Y,U,V separate
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV422SemiPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV422_SEMIPLANAR; // Y and UV interleaved - entire planes
    }
    else if (iOMXVideoColorFormat == OMX_COLOR_FormatYUV422PackedSemiPlanar)
    {
        iYUVFormat = PVMF_MIME_YUV422_PACKEDSEMIPLANAR; // Y and UV interleaved - sliced
    }
    else if (iOMXVideoColorFormat == 0x7FA30C00) // SPECIAL VALUE
    {
        iYUVFormat = PVMF_MIME_YUV420_SEMIPLANAR_YVU; // semiplanar with Y and VU interleaved
    }
    else
    {
        iYUVFormat = PVMF_MIME_FORMAT_UNKNOWN;
        return false;
    }

    //Set input video format
    //This is need it since a single component could handle differents roles

    // Init to desire format
    PVMFFormatType Format = PVMF_MIME_FORMAT_UNKNOWN;
    if (iInPort != NULL)
    {
        Format = ((PVMFOMXDecPort*)iInPort)->iFormat;
    }
    if (Format == PVMF_MIME_H264_VIDEO ||
            Format == PVMF_MIME_H264_VIDEO_MP4 ||
            Format == PVMF_MIME_H264_VIDEO_RAW)
    {
        iOMXVideoCompressionFormat = OMX_VIDEO_CodingAVC;
    }
    else if (Format == PVMF_MIME_M4V)
    {
        iOMXVideoCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (Format == PVMF_MIME_H2631998 ||
             Format == PVMF_MIME_H2632000)
    {
        iOMXVideoCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (Format == PVMF_MIME_WMV)
    {
        iOMXVideoCompressionFormat = OMX_VIDEO_CodingWMV;
    }
    else
    {
        // Illegal codec specified.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting video compression format"));
        return false;
    }


    CONFIG_SIZE_AND_VERSION(VideoPortFormat);
    VideoPortFormat.nPortIndex = iInputPortIndex;

    // Search the proper format index and set it.
    // Since we already know that the component has the role we need, search until finding the proper nIndex
    // if component does not find the format will return OMX_ErrorNoMore

    for (ii = 0;; ii++)
    {
        VideoPortFormat.nIndex = ii;
        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamVideoPortFormat, &VideoPortFormat);
        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting video compression format"));
            return false;
        }
        if (iOMXVideoCompressionFormat == VideoPortFormat.eCompressionFormat)
        {
            break;
        }
    }
    // Now set the format to confirm parameters
    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamVideoPortFormat, &VideoPortFormat);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXVideoDecNode::NegotiateComponentParameters() Problem setting video compression format"));
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXVideoDecNode::InitDecoder(PVMFSharedMediaDataPtr& DataIn)
{
    OSCL_UNUSED_ARG(DataIn);

    uint16 length = 0, size = 0;
    uint8 *tmp_ptr;
    PVMFFormatType Format = PVMF_MIME_FORMAT_UNKNOWN;

    OsclRefCounterMemFrag DataFrag;
    OsclRefCounterMemFrag refCtrMemFragOut;



    // NOTE: the component may not start decoding without providing the Output buffer to it,
    //		here, we're sending input/config buffers.
    //		Then, we'll go to ReadyToDecode state and send output as well

    if (iInPort != NULL)
    {
        Format = ((PVMFOMXDecPort*)iInPort)->iFormat;
    }
    if (Format == PVMF_MIME_H264_VIDEO ||
            Format == PVMF_MIME_H264_VIDEO_MP4)
    {
        uint8* initbuffer = ((PVMFOMXDecPort*)iInPort)->getTrackConfig();
        int32 initbufsize = (int32)((PVMFOMXDecPort*)iInPort)->getTrackConfigSize();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::InitDecoder() for H264 Decoder. Initialization data Size %d.", initbufsize));

        if (initbufsize > 0)
        {

            // there may be more than 1 NAL in config info in format specific data memfragment (SPS, PPS)
            tmp_ptr = initbuffer;
            do
            {
                length = (uint16)(tmp_ptr[1] << 8) | tmp_ptr[0];
                size += (length + 2);
                if (size > initbufsize)
                    break;
                tmp_ptr += 2;


                if (!SendConfigBufferToOMXComponent(tmp_ptr, length))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXVideoDecNode::InitDecoder() Error in processing config buffer"));
                    return false;

                }

                tmp_ptr += length;

            }
            while (size < initbufsize);
        }
    }
    else if (Format == PVMF_MIME_M4V ||
             Format == PVMF_MIME_H2631998 ||
             Format == PVMF_MIME_H2632000)
    {
        uint8* initbuffer = ((PVMFOMXDecPort*)iInPort)->getTrackConfig();
        int32 initbufsize = (int32)((PVMFOMXDecPort*)iInPort)->getTrackConfigSize();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::InitDecoder() for H263 Decoder. Initialization data Size %d.", initbufsize));

        // for H263, the initbufsize is 0, and initbuf= NULL. Config is done after 1st frame of data
        if (initbufsize > 0)
        {

            if (!SendConfigBufferToOMXComponent(initbuffer, initbufsize))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::InitDecoder() Error in processing config buffer"));
                return false;
            }
        }
    }
    else if (Format == PVMF_MIME_WMV)
    {
        uint8* initbuffer = ((PVMFOMXDecPort*)iInPort)->getTrackConfig();
        int32 initbufsize = (int32)((PVMFOMXDecPort*)iInPort)->getTrackConfigSize();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::InitDecoder() for WMV Decoder. Initialization data Size %d.", initbufsize));

        if (initbufsize > 0)
        {

            if (!SendConfigBufferToOMXComponent(initbuffer, initbufsize))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::InitDecoder() Error in processing config buffer"));
                return false;
            }
        }
    }
    else
    {
        // Unknown codec type
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXVideoDecNode::InitDecoder() Unknown codec type"));
        return false;
    }

    //Varibles initialization
    //sendFsi = true;

    return true;
}



/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR EVENT HANDLER
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXVideoDecNode::EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    OSCL_UNUSED_ARG(aEventData);

    switch (aEvent)
    {
        case OMX_EventCmdComplete:
        {

            switch (aData1)
            {
                case OMX_CommandStateSet:
                {
                    HandleComponentStateChange(aData2);
                    break;
                }
                case OMX_CommandFlush:
                {

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_CommandFlush - completed on port %d", aData2));

                    if (iIsRepositioningRequestSentToComponent)
                    {
                        if (aData2 == iOutputPortIndex)
                        {
                            iIsOutputPortFlushed = true;
                        }
                        else if (aData2 == iInputPortIndex)
                        {
                            iIsInputPortFlushed = true;
                        }

                        if (iIsOutputPortFlushed && iIsInputPortFlushed)
                        {
                            iIsRepositionDoneReceivedFromComponent = true;
                        }
                    }

                    if (IsAdded())
                        RunIfNotReady();

                }
                break;

                case OMX_CommandPortDisable:
                {
                    // if port disable command is done, we can re-allocate the buffers and re-enable the port

                    iProcessingState = EPVMFOMXBaseDecNodeProcessingState_PortReEnable;
                    iPortIndexForDynamicReconfig =  aData2;

                    RunIfNotReady();
                    break;
                }
                case OMX_CommandPortEnable:
                    // port enable command is done. Check if the other port also reported change.
                    // If not, we can start data flow. Otherwise, must start dynamic reconfig procedure for
                    // the other port as well.
                {
                    if (iSecondPortReportedChange)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, dynamic reconfiguration needed on port %d", aData2, iSecondPortToReconfig));

                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_PortReconfig;
                        iPortIndexForDynamicReconfig = iSecondPortToReconfig;
                        iSecondPortReportedChange = false;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, resuming normal data flow", aData2));
                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode;
                        iDynamicReconfigInProgress = false;
                        // in case pause or stop command was sent to component
                        // change processing state (because the node might otherwise
                        // start sending buffers to component before pause/stop is processed)
                        if (iPauseCommandWasSentToComponent)
                        {
                            iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Pausing;
                        }
                        if (iStopCommandWasSentToComponent)
                        {
                            iProcessingState = EPVMFOMXBaseDecNodeProcessingState_Stopping;
                        }
                    }
                    RunIfNotReady();
                    break;
                }

                case OMX_CommandMarkBuffer:
                    // nothing to do here yet;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_CommandMarkBuffer - completed - no action taken"));

                    break;

                default:
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: Unsupported event"));
                    break;
                }
            }//end of switch (aData1)

            break;
        }//end of case OMX_EventCmdComplete

        case OMX_EventError:
        {

            if (aData1 == (OMX_U32) OMX_ErrorStreamCorrupt)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventError - Bitstream corrupt error"));
                // Errors from corrupt bitstream are reported as info events
                ReportInfoEvent(PVMFInfoProcessingFailure, NULL);

            }
            else
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventError"));
                // for now, any error from the component will be reported as error
                ReportErrorEvent(PVMFErrorEvent, NULL, NULL);
                SetState(EPVMFNodeError);
            }
            break;

        }

        case OMX_EventBufferFlag:
        {
            // the component is reporting it encountered end of stream flag
            // we'll send eos when we get the actual last buffer with marked eos

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventBufferFlag (EOS) flag returned from OMX component"));

            RunIfNotReady();
            break;
        }//end of case OMX_EventBufferFlag

        case OMX_EventMark:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventMark returned from OMX component - no action taken"));

            RunIfNotReady();
            break;
        }//end of case OMX_EventMark

        case OMX_EventPortSettingsChanged:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned from OMX component"));

            // first check if dynamic reconfiguration is already in progress,
            // if so, wait until this is completed, and then initiate the 2nd reconfiguration
            if (iDynamicReconfigInProgress)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d, dynamic reconfig already in progress", aData1));

                iSecondPortToReconfig = aData1;
                iSecondPortReportedChange = true;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d", aData1));

                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_PortReconfig;
                iPortIndexForDynamicReconfig = aData1;
                iDynamicReconfigInProgress = true;
            }

            RunIfNotReady();
            break;
        }//end of case OMX_PortSettingsChanged

        case OMX_EventResourcesAcquired:        //not supported
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::EventHandlerProcessing: OMX_EventResourcesAcquired returned from OMX component - no action taken"));

            RunIfNotReady();

            break;
        }

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXVideoDecNode::EventHandlerProcessing:  Unknown Event returned from OMX component - no action taken"));

            break;
        }

    }//end of switch (eEvent)



    return OMX_ErrorNone;
}




////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Put output buffer in outgoing queue //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXVideoDecNode::QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen)
{

    bool status = true;
    PVMFSharedMediaDataPtr mediaDataOut;
    int32 leavecode = OsclErrNone;

    // NOTE: ASSUMPTION IS THAT OUTGOING QUEUE IS BIG ENOUGH TO QUEUE ALL THE OUTPUT BUFFERS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::QueueOutputFrame: In"));

    // First check if we can put outgoing msg. into the queue
    if (iOutPort->IsOutgoingQueueBusy())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFOMXVideoDecNode::QueueOutputFrame() OutgoingQueue is busy"));
        return false;
    }

    OSCL_TRY(leavecode,
             mediaDataOut = PVMFMediaData::createMediaData(mediadataimplout, iMediaDataMemPool););
    if (OsclErrNone == leavecode)
    {

        // Update the filled length of the fragment
        mediaDataOut->setMediaFragFilledLen(0, aDataLen);

        // Set timestamp
        mediaDataOut->setTimestamp(iOutTimeStamp);

        // Set Streamid
        mediaDataOut->setStreamID(iStreamID);

        // Set sequence number
        mediaDataOut->setSeqNum(iSeqNum++);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iDataPathLogger, PVLOGMSG_INFO, (0, ":PVMFOMXVideoDecNode::QueueOutputFrame(): - SeqNum=%d, TS=%d", iSeqNum, iOutTimeStamp));

        int fsiErrorCode = 0;


        // Check if Fsi configuration need to be sent
        if (sendFsi && !iCompactFSISettingSucceeded)
        {
            OsclRefCounterMemFrag yuvFsiMemfrag;

            OSCL_TRY(fsiErrorCode, yuvFsiMemfrag = iFsiFragmentAlloc.get(););

            OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                 (0, "PVMFOMXVideoDecNode::RemoveOutputFrame() Failed to allocate memory for  FSI")));

            if (fsiErrorCode == 0)
            {
                PVMFYuvFormatSpecificInfo0* fsiInfo = OSCL_PLACEMENT_NEW(yuvFsiMemfrag.getMemFragPtr(), PVMFYuvFormatSpecificInfo0());
                if (fsiInfo != NULL)
                {
                    fsiInfo->uid = PVMFYuvFormatSpecificInfo0_UID;
                    fsiInfo->video_format = iYUVFormat;
                    fsiInfo->display_width = iYUVWidth;
                    fsiInfo->display_height = iYUVHeight;

                    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 ||
                            ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                    {
                        fsiInfo->width = (iYUVWidth + 15) & (~15);
                        fsiInfo->height = (iYUVHeight + 15) & (~15);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                    {
                        fsiInfo->width = (iYUVWidth + 3) & -4;
                        fsiInfo->height = iYUVHeight;
                    }
                    else
                    {
                        fsiInfo->width = iYUVWidth;
                        fsiInfo->height = iYUVHeight;
                    }

                    OsclMemAllocator alloc;
                    int32 KeyLength = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY) + 1;
                    PvmiKeyType KvpKey = (PvmiKeyType)alloc.ALLOCATE(KeyLength);

                    if (NULL == KvpKey)
                    {
                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrNoMemory);
                        return false;
                    }

                    oscl_strncpy(KvpKey, PVMF_FORMAT_SPECIFIC_INFO_KEY, KeyLength);
                    int32 err;

                    OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(yuvFsiMemfrag, KvpKey););
                    if (err != OsclErrNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXVideoDecNode::HandlePortReEnable - Problem to set FSI"));

                    }


                    alloc.deallocate((OsclAny*)(KvpKey));
                    fsiInfo->video_format.~PVMFFormatType();


                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXVideoDecNode::QueueOutputFrame - Problem allocating Output FSI"));
                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return false; // this is going to make everything go out of scope
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXVideoDecNode::QueueOutputFrame - Problem allocating Output FSI"));
                return false; // this is going to make everything go out of scope
            }


            // Reset the flag
            sendFsi = false;
        }



        // in case of special YVU format, attach fsi to every outgoing message containing ptr to private data
        if (iYUVFormat == PVMF_MIME_YUV420_SEMIPLANAR_YVU)
        {
            OsclRefCounterMemFrag privatedataFsiMemFrag;

            OSCL_TRY(fsiErrorCode, privatedataFsiMemFrag = iPrivateDataFsiFragmentAlloc.get(););

            OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                 (0, "PVMFOMXVideoDecNode::RemoveOutputFrame() Failed to allocate memory for  FSI for private data")));


            if (fsiErrorCode == 0)
            {
                uint8 *fsiptr = (uint8*) privatedataFsiMemFrag.getMemFragPtr();
                privatedataFsiMemFrag.getMemFrag().len = sizeof(OsclAny*);
                oscl_memcpy(fsiptr, &ipPrivateData, sizeof(OsclAny *)); // store ptr data into fsi
                mediaDataOut->setFormatSpecificInfo(privatedataFsiMemFrag);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXVideoDecNode::QueueOutputFrame - Problem allocating Output FSI for private data"));
                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return false; // this is going to make everything go out of scope
            }
        }


        if (fsiErrorCode == 0)
        {
            // Send frame to downstream node
            PVMFSharedMediaMsgPtr mediaMsgOut;
            convertToPVMFMediaMsg(mediaMsgOut, mediaDataOut);

            if (iOutPort && (iOutPort->QueueOutgoingMsg(mediaMsgOut) == PVMFSuccess))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "PVMFOMXVideoDecNode::QueueOutputFrame(): Queued frame OK "));

            }
            else
            {
                // we should not get here because we always check for whether queue is busy or not
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::QueueOutputFrame(): Send frame failed"));
                return false;
            }

        }


    }//end of if (leavecode==0)
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXVideoDecNode::QueueOutputFrame() call PVMFMediaData::createMediaData is failed"));
        return false;
    }

    return status;

}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXVideoDecNode::DoRequestPort(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::DoRequestPort() In"));
    //This node supports port request from any state

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;

    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(tag, portconfig);

    PVMFPortInterface* port = NULL;
    int32 leavecode = OsclErrNone;
    //validate the tag...
    switch (tag)
    {
        case PVMF_OMX_DEC_NODE_PORT_TYPE_INPUT:
            if (iInPort)
            {
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                break;
            }
            OSCL_TRY(leavecode, iInPort = OSCL_NEW(PVMFOMXDecPort, ((int32)tag, this, PVMF_OMX_VIDEO_DEC_INPUT_PORT_NAME)););
            if (leavecode || iInPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::DoRequestPort: Error - Input port instantiation failed"));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }
            port = iInPort;
            break;

        case PVMF_OMX_DEC_NODE_PORT_TYPE_OUTPUT:
            if (iOutPort)
            {
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                break;
            }
            OSCL_TRY(leavecode, iOutPort = OSCL_NEW(PVMFOMXDecPort, ((int32)tag, this, PVMF_OMX_VIDEO_DEC_OUTPUT_PORT_NAME)));
            if (leavecode || iOutPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXVideoDecNode::DoRequestPort: Error - Output port instantiation failed"));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }
            port = iOutPort;
            break;

        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXVideoDecNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
            return;
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXVideoDecNode::DoReleasePort(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVMFPortInterface* p = NULL;
    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(p);
    PVMFOMXDecPort* port = (PVMFOMXDecPort*)p;

    if (port != NULL && (port == iInPort || port == iOutPort))
    {
        if (port == iInPort)
        {
            OSCL_DELETE(((PVMFOMXDecPort*)iInPort));
            iInPort = NULL;
        }
        else
        {
            OSCL_DELETE(((PVMFOMXDecPort*)iOutPort));
            iOutPort = NULL;
        }
        //delete the port.
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        //port not found.
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXVideoDecNode::DoGetNodeMetadataKey(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXVideoDecNode::DoGetNodeMetadataKey() In"));

    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key;

    aCmd.PVMFOMXBaseDecNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);

    // Check parameters
    if (keylistptr == NULL)
    {
        // The list pointer is invalid
        return PVMFErrArgument;
    }

    // Update the available metadata keys
    iAvailableMetadataKeys.clear();
    int32 leavecode = OsclErrNone;
    leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY, iAvailableMetadataKeys);
    if (OsclErrNone != leavecode)
    {
        return PVMFErrNoMemory;
    }

    if (iYUVWidth > 0 && iYUVHeight > 0)
    {
        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return PVMFErrNoMemory;
        }

        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return PVMFErrNoMemory;
        }
    }
    // add the profile, level and avgbitrate
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
    {
        // For H263 this metadata will be available only after first frame decoding
        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY, iAvailableMetadataKeys);
        if (leavecode != OsclErrNone)
        {
            return PVMFErrNoMemory;
        }

        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return PVMFErrNoMemory;
        }
    }
    leavecode = OsclErrNone;
    leavecode = PushKVPKey(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY, iAvailableMetadataKeys);
    if (OsclErrNone != leavecode)
    {
        return PVMFErrNoMemory;
    }

    if ((starting_index > (iAvailableMetadataKeys.size() - 1)) || max_entries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    for (uint32 lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > starting_index)
            {
                // Past the starting index so copy the key
                leavecode = OsclErrNone;
                leavecode = PushKVPKey(iAvailableMetadataKeys[lcv] , keylistptr);
                if (OsclErrNone != leavecode)
                {
                    return PVMFErrNoMemory;
                }
                num_added++;
            }
        }
        else
        {
            // Check if the key matche the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                // This key is counted
                ++num_entries;
                if (num_entries > starting_index)
                {
                    // Past the starting index so copy the key
                    leavecode = OsclErrNone;
                    leavecode = PushKVPKey(iAvailableMetadataKeys[lcv] , keylistptr);
                    if (OsclErrNone != leavecode)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetNodeMetadataKey() Memory allocation failure when copying metadata key"));
                        return PVMFErrNoMemory;
                    }
                    num_added++;
                }
            }
        }

        // Check if max number of entries have been copied
        if (max_entries > 0 && num_added >= max_entries)
        {
            break;
        }
    }

    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXVideoDecNode::DoGetNodeMetadataValue(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetNodeMetadataValue() In"));

    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    aCmd.PVMFOMXBaseDecNodeCommand::Parse(keylistptr, valuelistptr, starting_index, max_entries);

    // Check the parameters
    if (keylistptr == NULL || valuelistptr == NULL)
    {
        return PVMFErrArgument;
    }

    uint32 numkeys = keylistptr->size();

    if (starting_index > (numkeys - 1) || numkeys <= 0 || max_entries == 0)
    {
        // Don't do anything
        return PVMFErrArgument;
    }

    uint32 numvalentries = 0;
    int32 numentriesadded = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        int32 leavecode = OsclErrNone;
        int32 leavecode1 = OsclErrNone;
        PvmiKvp KeyVal;
        KeyVal.key = NULL;
        uint32 KeyLen = 0;

        if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY) == 0) &&
                iYUVWidth > 0)
        {
            // Video width
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY) + 1; // for "codec-info/video/width;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                if (OsclErrNone == leavecode)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iYUVWidth;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) == 0) &&
                 iYUVHeight > 0)
        {
            // Video height
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) + 1; // for "codec-info/video/height;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                if (OsclErrNone == leavecode)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iYUVHeight;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY) == 0))
        {
            // Video profile
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY) + 1; // for "codec-info/video/profile;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                if (OsclErrNone == leavecode)
                {
                    PVMF_MPEGVideoProfileType aProfile;
                    PVMF_MPEGVideoLevelType aLevel;
                    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = (uint32)aProfile; // This is to be decided, who will interpret these value
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY) == 0))
        {
            // Video level
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY) + 1; // for "codec-info/video/level;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                if (OsclErrNone == leavecode)
                {
                    PVMF_MPEGVideoProfileType aProfile;
                    PVMF_MPEGVideoLevelType aLevel;
                    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = (uint32)aLevel; // This is to be decided, who will interpret these value
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) == 0) &&
                 (iAvgBitrateValue > 0))
        {
            // Video average bitrate
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) + 1; // for "codec-info/video/avgbitrate;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                if (OsclErrNone == leavecode)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iAvgBitrateValue;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;

                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY) == 0) &&
                 (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000 ||
                  ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V ||
                  ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                  ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW  || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV))
        {
            // Format
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY) + 1; // for "codec-info/video/format;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*" and NULL terminator

                uint32 valuelen = 0;

                if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H264_VIDEO_RAW)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_M4V)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H2631998)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_H2632000)) + 1; // Value string plus one for NULL terminator
                }
                else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                {
                    valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_WMV)) + 1; // Value string plus one for NULL terminator
                }
                else
                {
                    // Should not enter here
                    OSCL_ASSERT(false);
                    valuelen = 1;
                }

                // Allocate memory for the strings
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                if (OsclErrNone == leavecode)
                {
                    KeyVal.value.pChar_value = (char*) AllocateKVPKeyArray(leavecode1, PVMI_KVPVALTYPE_CHARPTR, valuelen);
                }

                if (OsclErrNone == leavecode && OsclErrNone == leavecode1)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY, oscl_strlen(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXVIDEODECMETADATA_SEMICOLON, oscl_strlen(PVOMXVIDEODECMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_RAW), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_M4V), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H2631998), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_H2632000), valuelen);
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
                    {
                        oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_WMV), valuelen);
                    }
                    else
                    {
                        // Should not enter here
                        OSCL_ASSERT(false);
                    }
                    KeyVal.value.pChar_value[valuelen-1] = NULL_TERM_CHAR;
                    // Set the length and capacity
                    KeyVal.length = valuelen;
                    KeyVal.capacity = valuelen;
                }
                else
                {
                    // Memory allocation failed so clean up
                    if (KeyVal.key)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.key);
                        KeyVal.key = NULL;
                    }
                    if (KeyVal.value.pChar_value)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                    }
                    break;
                }
            }
        }

        if (KeyVal.key != NULL)
        {
            leavecode = OsclErrNone;
            leavecode = PushKVP(KeyVal, *valuelistptr);
            if (OsclErrNone != leavecode)
            {
                switch (GetValTypeFromKeyString(KeyVal.key))
                {
                    case PVMI_KVPVALTYPE_CHARPTR:
                        if (KeyVal.value.pChar_value != NULL)
                        {
                            OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                            KeyVal.value.pChar_value = NULL;
                        }
                        break;

                    default:
                        // Add more case statements if other value types are returned
                        break;
                }

                OSCL_ARRAY_DELETE(KeyVal.key);
                KeyVal.key = NULL;
            }
            else
            {
                // Increment the counter for number of value entries added to the list
                ++numentriesadded;
            }

            // Check if the max number of value entries were added
            if (max_entries > 0 && numentriesadded >= max_entries)
            {
                break;
            }
        }
    }

    return PVMFSuccess;
}

/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXVideoDecNode::ReleaseAllPorts()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::ReleaseAllPorts() In"));

    if (iInPort)
    {
        iInPort->ClearMsgQueues();
        iInPort->Disconnect();
        OSCL_DELETE(((PVMFOMXDecPort*)iInPort));
        iInPort = NULL;
    }

    if (iOutPort)
    {
        iOutPort->ClearMsgQueues();
        iOutPort->Disconnect();
        OSCL_DELETE(((PVMFOMXDecPort*)iOutPort));
        iOutPort = NULL;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXVideoDecNode::DoQueryUuid(PVMFOMXBaseDecNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node

    //Match against custom interface1...
    if (*mimetype == PVMF_OMX_BASE_DEC_NODE_CUSTOM1_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_OMX_VIDEO_DEC_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {

        PVUuid uuid(PVMF_OMX_BASE_DEC_NODE_CUSTOM1_UUID);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXVideoDecNode::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::GetNumMetadataKeys() called"));

    // Update the available metadata keys
    iAvailableMetadataKeys.clear();
    int32 errcode = OsclErrNone;
    OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY));

    if (iYUVWidth > 0 && iYUVHeight > 0)
    {
        errcode = OsclErrNone;
        OSCL_TRY(errcode,
                 iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY);
                 iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY));
    }
    // add the profile, level and avgbitrate
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    if (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess)
    {
        // For H263 this metadata will be available only after first frame decoding
        errcode = OsclErrNone;
        OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY));
        errcode = OsclErrNone;
        OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY));
    }
    errcode = OsclErrNone;
    OSCL_TRY(errcode, iAvailableMetadataKeys.push_back(PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY));



    uint32 num_entries = 0;

    if (aQueryKeyString == NULL)
    {
        num_entries = iAvailableMetadataKeys.size();
    }
    else
    {
        for (uint32 i = 0; i < iAvailableMetadataKeys.size(); i++)
        {
            if (pv_mime_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) >= 0)
            {
                num_entries++;
            }
        }
    }
    return num_entries; // Number of elements
}

/////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXVideoDecNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::GetNumMetadataValues() called"));

    uint32 numkeys = aKeyList.size();

    if (numkeys <= 0)
    {
        // Don't do anything
        return 0;
    }

    // Count the number of value entries for the provided key list
    uint32 numvalentries = 0;
    PVMF_MPEGVideoProfileType aProfile;
    PVMF_MPEGVideoLevelType aLevel;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_WIDTH_KEY) == 0) &&
                iYUVWidth > 0)
        {
            // Video width
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_HEIGHT_KEY) == 0) &&
                 iYUVHeight > 0)
        {
            // Video height
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_PROFILE_KEY) == 0) &&
                 (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess))

        {
            // Video profile
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_LEVEL_KEY) == 0) &&
                 (GetProfileAndLevel(aProfile, aLevel) == PVMFSuccess))
        {
            // Video level
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_AVGBITRATE_KEY) == 0) &&
                 (iAvgBitrateValue > 0))

        {
            // Video average bitrate
            if (iAvgBitrateValue > 0)
                ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXVIDEODECMETADATA_CODECINFO_VIDEO_FORMAT_KEY) == 0) &&
                 (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_M4V || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2631998 || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H2632000 || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_MP4 || ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_H264_VIDEO_RAW))
        {
            // Format
            ++numvalentries;
        }
    }

    return numvalentries;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// CAPABILITY CONFIG PRIVATE
PVMFStatus PVMFOMXVideoDecNode::DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() In"));
    OSCL_UNUSED_ARG(aContext);

    // Initialize the output parameters
    aNumParamElements = 0;
    aParameters = NULL;

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aIdentifier);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/decoder")) < 0) || compcount < 3)
    {
        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/render")) < 0) || compcount != 3)
        {
            // First 3 component should be "x-pvmf/video/decoder" and there must
            // be at least three components
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() Unsupported key"));
            return PVMFErrArgument;
        }
    }

    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/render")) >= 0)
    {
        aParameters = (PvmiKvp*)oscl_malloc(PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS * sizeof(PvmiKvp));
        if (aParameters == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
            return PVMFErrNoMemory;
        }
        oscl_memset(aParameters, 0, PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS*sizeof(PvmiKvp));
        // Allocate memory for the key strings in each KVP
        PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
        if (memblock == NULL)
        {
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
            return PVMFErrNoMemory;
        }
        oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS*PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
        // Assign the key string buffer to each KVP
        int32 j;
        for (j = 0; j < PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS; ++j)
        {
            aParameters[j].key = memblock + (j * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE);
        }
        // Copy the requested info
        for (j = 0; j < PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS; ++j)
        {
            oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/video/render/"), 20);
            oscl_strncat(aParameters[j].key, PVOMXVideoDecNodeConfigRenderKeys[j].iString, oscl_strlen(PVOMXVideoDecNodeConfigRenderKeys[j].iString));
            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";valtype=uint32_value"), 21);
            aParameters[j].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;

            // Copy the requested info
            switch (j)
            {
                case 0:	// "width"
                    // Return current value
                    aParameters[j].value.uint32_value = iNewWidth;
                    break;
                case 1: // "height"
                    aParameters[j].value.uint32_value = iNewHeight;
                    break;
                default:
                    break;
            }
        }

        aNumParamElements = PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS;
    }
    else if (compcount == 3)
    {
        // Since key is "x-pvmf/video/decoder" return all
        // nodes available at this level. Ignore attribute
        // since capability is only allowed

        // Allocate memory for the KVP list
        aParameters = (PvmiKvp*)oscl_malloc(PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS * sizeof(PvmiKvp));
        if (aParameters == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
            return PVMFErrNoMemory;
        }
        oscl_memset(aParameters, 0, PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS*sizeof(PvmiKvp));
        // Allocate memory for the key strings in each KVP
        PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
        if (memblock == NULL)
        {
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
            return PVMFErrNoMemory;
        }
        oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS*PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
        // Assign the key string buffer to each KVP
        int32 j;
        for (j = 0; j < PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS; ++j)
        {
            aParameters[j].key = memblock + (j * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE);
        }
        // Copy the requested info
        for (j = 0; j < PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS; ++j)
        {
            oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/video/decoder/"), 21);
            oscl_strncat(aParameters[j].key, PVOMXVideoDecNodeConfigBaseKeys[j].iString, oscl_strlen(PVOMXVideoDecNodeConfigBaseKeys[j].iString));
            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type="), 6);
            switch (PVOMXVideoDecNodeConfigBaseKeys[j].iType)
            {
                case PVMI_KVPTYPE_AGGREGATE:
                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_AGGREGATE_STRING), oscl_strlen(PVMI_KVPTYPE_AGGREGATE_STRING));
                    break;

                case PVMI_KVPTYPE_POINTER:
                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_POINTER_STRING), oscl_strlen(PVMI_KVPTYPE_POINTER_STRING));
                    break;

                case PVMI_KVPTYPE_VALUE:
                default:
                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_VALUE_STRING), oscl_strlen(PVMI_KVPTYPE_VALUE_STRING));
                    // Now append the valtype param
                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";valtype="), 9);
                    switch (PVOMXVideoDecNodeConfigBaseKeys[j].iValueType)
                    {
                        case PVMI_KVPVALTYPE_BITARRAY32:
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BITARRAY32_STRING), oscl_strlen(PVMI_KVPVALTYPE_BITARRAY32_STRING));
                            break;

                        case PVMI_KVPVALTYPE_UINT32:
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                            break;

                        case PVMI_KVPVALTYPE_BOOL:
                        default:
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                            break;
                    }
                    break;
            }

            aParameters[j].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;
        }

        aNumParamElements = PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS;
    }
    else
    {
        // Retrieve the fourth component from the key string
        pv_mime_string_extract_type(3, aIdentifier, compstr);

        for (int32 vdeccomp4ind = 0; vdeccomp4ind < PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS; ++vdeccomp4ind)
        {
            // Go through each video dec component string at 4th level
            if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigBaseKeys[vdeccomp4ind].iString)) >= 0)
            {
                if (vdeccomp4ind == 3)
                {
                    // "x-pvmf/video/decoder/h263"
                    if (compcount == 4)
                    {
                        // Return list of H263 settings. Ignore the
                        // attribute since capability is only allowed

                        // Allocate memory for the KVP list
                        aParameters = (PvmiKvp*)oscl_malloc(PVOMXVIDEODECNODECONFIG_H263_NUMKEYS * sizeof(PvmiKvp));
                        if (aParameters == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                            return PVMFErrNoMemory;
                        }
                        oscl_memset(aParameters, 0, PVOMXVIDEODECNODECONFIG_H263_NUMKEYS*sizeof(PvmiKvp));
                        // Allocate memory for the key strings in each KVP
                        PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_H263_NUMKEYS * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
                        if (memblock == NULL)
                        {
                            oscl_free(aParameters);
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                            return PVMFErrNoMemory;
                        }
                        oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_H263_NUMKEYS*PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
                        // Assign the key string buffer to each KVP
                        int32 j;
                        for (j = 0; j < PVOMXVIDEODECNODECONFIG_H263_NUMKEYS; ++j)
                        {
                            aParameters[j].key = memblock + (j * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE);
                        }
                        // Copy the requested info
                        for (j = 0; j < PVOMXVIDEODECNODECONFIG_H263_NUMKEYS; ++j)
                        {
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/video/decoder/h263/"), 26);
                            oscl_strncat(aParameters[j].key, PVOMXVideoDecNodeConfigH263Keys[j].iString, oscl_strlen(PVOMXVideoDecNodeConfigH263Keys[j].iString));
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type=value;valtype="), 20);
                            switch (PVOMXVideoDecNodeConfigH263Keys[j].iValueType)
                            {
                                case PVMI_KVPVALTYPE_RANGE_UINT32:
                                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
                                    break;

                                case PVMI_KVPVALTYPE_UINT32:
                                default:
                                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                                    break;
                            }

                            aParameters[j].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;
                        }

                        aNumParamElements = PVOMXVIDEODECNODECONFIG_H263_NUMKEYS;
                    }
                    else if (compcount > 4)
                    {
                        // Retrieve the fifth component from the key string
                        pv_mime_string_extract_type(4, aIdentifier, compstr);

                        for (int32 vdeccomp5ind = 0; vdeccomp5ind < PVOMXVIDEODECNODECONFIG_H263_NUMKEYS; ++vdeccomp5ind)
                        {
                            if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigH263Keys[vdeccomp5ind].iString)) >= 0)
                            {
                                // Determine what is requested
                                PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                                if (reqattr == PVMI_KVPATTR_UNKNOWN)
                                {
                                    // Default is current setting
                                    reqattr = PVMI_KVPATTR_CUR;
                                }

                                // Return the requested info
                                PVMFStatus retval = DoGetH263DecoderParameter(aParameters, aNumParamElements, vdeccomp5ind, reqattr);
                                if (retval != PVMFSuccess)
                                {
                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Retrieving H.263 parameter failed"));
                                    return retval;
                                }

                                // Break out of the for(vdeccomp5ind) loop
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Right now video dec node doesn't support more than 5 components
                        // for the key sub-string so error out
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Unsupported key"));
                        return PVMFErrArgument;
                    }
                }
                else if (vdeccomp4ind == 4)
                {
                    // "x-pvmf/video/decoder/m4v"
                    if (compcount == 4)
                    {
                        // Return list of M4v settings. Ignore the
                        // attribute since capability is only allowed

                        // Allocate memory for the KVP list
                        aParameters = (PvmiKvp*)oscl_malloc(PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS * sizeof(PvmiKvp));
                        if (aParameters == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for KVP failed"));
                            return PVMFErrNoMemory;
                        }
                        oscl_memset(aParameters, 0, PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS*sizeof(PvmiKvp));
                        // Allocate memory for the key strings in each KVP
                        PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
                        if (memblock == NULL)
                        {
                            oscl_free(aParameters);
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Memory allocation for key string failed"));
                            return PVMFErrNoMemory;
                        }
                        oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS*PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
                        // Assign the key string buffer to each KVP
                        int32 j;
                        for (j = 0; j < PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS; ++j)
                        {
                            aParameters[j].key = memblock + (j * PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE);
                        }
                        // Copy the requested info
                        for (j = 0; j < PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS; ++j)
                        {
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/video/decoder/m4v/"), 25);
                            oscl_strncat(aParameters[j].key, PVOMXVideoDecNodeConfigM4VKeys[j].iString, oscl_strlen(PVOMXVideoDecNodeConfigM4VKeys[j].iString));
                            oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type=value;valtype="), 20);
                            switch (PVOMXVideoDecNodeConfigM4VKeys[j].iValueType)
                            {
                                case PVMI_KVPVALTYPE_RANGE_UINT32:
                                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
                                    break;

                                case PVMI_KVPVALTYPE_UINT32:
                                default:
                                    oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                                    break;
                            }

                            aParameters[j].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;
                        }

                        aNumParamElements = PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS;
                    }
                    else if (compcount > 4)
                    {
                        // Retrieve the fifth component from the key string
                        pv_mime_string_extract_type(4, aIdentifier, compstr);

                        for (int32 vdeccomp5ind = 0; vdeccomp5ind < PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS; ++vdeccomp5ind)
                        {
                            if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigM4VKeys[vdeccomp5ind].iString)) >= 0)
                            {
                                // Determine what is requested
                                PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                                if (reqattr == PVMI_KVPATTR_UNKNOWN)
                                {
                                    // Default is current setting
                                    reqattr = PVMI_KVPATTR_CUR;
                                }

                                // Return the requested info
                                PVMFStatus retval = DoGetM4VDecoderParameter(aParameters, aNumParamElements, vdeccomp5ind, reqattr);
                                if (retval != PVMFSuccess)
                                {
                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Retrieving M4v parameter failed"));
                                    return retval;
                                }

                                // Break out of the for(vdeccomp5ind) loop
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Right now video dec node doesn't support more than 5 components
                        // for the key sub-string so error out
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Unsupported key"));
                        return PVMFErrArgument;
                    }
                }
                else if ((vdeccomp4ind == 0) || // "postproc_enable",
                         (vdeccomp4ind == 1) ||	// "postproc_type"
                         (vdeccomp4ind == 2) ||	// "dropframe_enable"
                         (vdeccomp4ind == 5)	// "format_type"
                        )
                {
                    if (compcount == 4)
                    {
                        // Determine what is requested
                        PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                        if (reqattr == PVMI_KVPATTR_UNKNOWN)
                        {
                            reqattr = PVMI_KVPATTR_CUR;
                        }

                        // Return the requested info
                        PVMFStatus retval = DoGetVideoDecNodeParameter(aParameters, aNumParamElements, vdeccomp4ind, reqattr);
                        if (retval != PVMFSuccess)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Retrieving video dec node parameter failed"));
                            return retval;
                        }
                    }
                    else
                    {
                        // Right now videodec node doesn't support more than 4 components
                        // for this sub-key string so error out
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Unsupported key"));
                        return PVMFErrArgument;
                    }
                }

                // Breakout of the for(vdeccomp4ind) loop
                break;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Out"));
    if (aNumParamElements == 0)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigGetParametersSync() Unsupported key"));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}


PVMFStatus PVMFOMXVideoDecNode::DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() In"));

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/decoder")) < 0) || compcount < 3)
    {
        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/render")) < 0) || compcount != 4)
        {
            // First 3 component should be "x-pvmf/video/decoder" and there must
            // be at least three components
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() Unsupported key"));
            return PVMFErrArgument;
        }
    }

    if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/decoder")) >= 0)
    {
        // Retrieve the third component from the key string
        pv_mime_string_extract_type(2, aParameters[0].key, compstr);

        // Go through each KVP and release memory for value if allocated from heap
        for (int32 i = 0; i < aNumElements; ++i)
        {
            // Next check if it is a value type that allocated memory
            PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[i].key);
            if (kvptype == PVMI_KVPTYPE_VALUE || kvptype == PVMI_KVPTYPE_UNKNOWN)
            {
                PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[i].key);
                if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() Valtype not specified in key string"));
                    return PVMFErrArgument;
                }

                if (keyvaltype == PVMI_KVPVALTYPE_CHARPTR && aParameters[i].value.pChar_value != NULL)
                {
                    oscl_free(aParameters[i].value.pChar_value);
                    aParameters[i].value.pChar_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_KSV && aParameters[i].value.key_specific_value != NULL)
                {
                    oscl_free(aParameters[i].value.key_specific_value);
                    aParameters[i].value.key_specific_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_UINT32 && aParameters[i].value.key_specific_value != NULL)
                {
                    range_uint32* rui32 = (range_uint32*)aParameters[i].value.key_specific_value;
                    aParameters[i].value.key_specific_value = NULL;
                    oscl_free(rui32);
                }

            }
        }
    }
    // Video dec node allocated its key strings in one chunk so just free the first key string ptr
    oscl_free(aParameters[0].key);

    // Free memory for the parameter list
    oscl_free(aParameters);
    aParameters = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigReleaseParameters() Out"));
    return PVMFSuccess;
}


void PVMFOMXVideoDecNode::DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() In"));

    if (aParameters == NULL || aNumElements < 1)
    {
        if (aParameters)
        {
            aRetKVP = aParameters;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Passed in parameter invalid"));
        return;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/decoder")) < 0) || compcount < 4)
        {
            // First 3 components should be "x-pvmf/video/decoder" and there must
            // be at least four components
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Unsupported key"));
            return;
        }

        if (compcount == 4)
        {
            // Verify and set the passed-in video dec node setting
            PVMFStatus retval = DoVerifyAndSetVideoDecNodeParameter(aParameters[paramind], true);
            if (retval != PVMFSuccess)
            {
                aRetKVP = &aParameters[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                return;
            }
        }
        else if (compcount == 5)
        {
            // Determine the 4th level component
            pv_mime_string_extract_type(3, aParameters[paramind].key, compstr);
            if (pv_mime_strcmp(compstr, _STRLIT_CHAR("h263")) >= 0)
            {
                // Verify and set the passed-in H.263 decoder setting
                PVMFStatus retval = DoVerifyAndSetH263DecoderParameter(aParameters[paramind], true);
                if (retval != PVMFSuccess)
                {
                    aRetKVP = &aParameters[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return;
                }
            }
            else if (pv_mime_strcmp(compstr, _STRLIT_CHAR("m4v")) >= 0)
            {
                // Verify and set the passed-in M4v decoder setting
                PVMFStatus retval = DoVerifyAndSetM4VDecoderParameter(aParameters[paramind], true);
                if (retval != PVMFSuccess)
                {
                    aRetKVP = &aParameters[paramind];
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Setting parameter %d failed", paramind));
                    return;
                }
            }
            else
            {
                // Unknown key sub-string
                aRetKVP = &aParameters[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Unsupported key"));
                return;
            }
        }
        else
        {
            // Do not support more than 5 components right now
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Unsupported key"));
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigSetParameters() Out"));
}

/* This function finds a nal from the SC's, moves the bitstream pointer to the beginning of the NAL unit, returns the
	size of the NAL, and at the same time, updates the remaining size in the bitstream buffer that is passed in */
int32 PVMFOMXVideoDecNode::GetNAL_OMXNode(uint8** bitstream, int* size)
{
    int i = 0;
    int j;
    uint8* nal_unit = *bitstream;
    int count = 0;

    /* find SC at the beginning of the NAL */
    while (nal_unit[i++] == 0 && i < *size)
    {
    }

    if (nal_unit[i-1] == 1)
    {
        *bitstream = nal_unit + i;
    }
    else
    {
        j = *size;
        *size = 0;
        return j;  // no SC at the beginning, not supposed to happen
    }

    j = i;

    /* found the SC at the beginning of the NAL, now find the SC at the beginning of the next NAL */
    while (i < *size)
    {
        if (count == 2 && nal_unit[i] == 0x01)
        {
            i -= 2;
            break;
        }

        if (nal_unit[i])
            count = 0;
        else
            count++;
        i++;
    }

    *size -= i;
    return (i -j);
}


PVMFStatus PVMFOMXVideoDecNode::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() In"));

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/video/decoder")) < 0) || compcount < 4)
        {
            if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/media/format_specific_info")) < 0) || compcount < 3)
            {
                // First 3 components should be "x-pvmf/media/format_specific_info" and there must
                // be at least three components
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Unsupported key"));
                return PVMFErrArgument;
            }
            else
            {
                pvVideoConfigParserInputs aInputs;
                OMXConfigParserInputs aInputParameters;
                VideoOMXConfigParserOutputs aOutputParameters;

                aInputs.inPtr = (uint8*)(aParameters->value.key_specific_value);
                aInputs.inBytes = (int32)aParameters->capacity;
                aInputs.iMimeType = iNodeConfig.iMimeType;
                aInputParameters.inBytes = aInputs.inBytes;
                aInputParameters.inPtr = aInputs.inPtr;

                if (aInputs.iMimeType ==  PVMF_MIME_H264_VIDEO ||
                        aInputs.iMimeType == PVMF_MIME_H264_VIDEO_MP4 ||
                        aInputs.iMimeType == PVMF_MIME_H264_VIDEO_RAW)
                {
                    aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.avc";
                }
                else if (aInputs.iMimeType ==  PVMF_MIME_M4V)
                {
                    aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.mpeg4";
                }
                else if (aInputs.iMimeType ==  PVMF_MIME_H2631998 ||
                         aInputs.iMimeType == PVMF_MIME_H2632000)
                {
                    aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.h263";
                }
                else if (aInputs.iMimeType ==  PVMF_MIME_WMV)
                {
                    aInputParameters.cComponentRole = (OMX_STRING)"video_decoder.wmv";
                }
                else
                {
                    // Illegal codec specified.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Input port format other then codec type", iName.Str()));

                }


                OMX_BOOL status = OMX_FALSE;
                OMX_U32 num_comps = 0;
                OMX_STRING *CompOfRole;
                OMX_U32 ii;

                // call once to find out the number of components that can fit the role

                OMX_GetComponentsOfRole(aInputParameters.cComponentRole, &num_comps, NULL);

                if (num_comps > 0)
                {
                    CompOfRole = (OMX_STRING *)oscl_malloc(num_comps * sizeof(OMX_STRING));
                    for (ii = 0; ii < num_comps; ii++)
                        CompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

                    // call 2nd time to get the component names
                    OMX_GetComponentsOfRole(aInputParameters.cComponentRole, &num_comps, (OMX_U8 **)CompOfRole);
                    for (ii = 0; ii < num_comps; ii++)
                    {
                        aInputParameters.cComponentName = CompOfRole[ii];
                        status = OMXConfigParser(&aInputParameters, &aOutputParameters);
                        if (status == OMX_TRUE)
                        {
                            break;
                        }
                        else
                        {
                            status = OMX_FALSE;
                        }
                    }

                    // whether successful or not, need to free CompOfRoles
                    for (ii = 0; ii < num_comps; ii++)
                    {
                        oscl_free(CompOfRole[ii]);
                        CompOfRole[ii] = NULL;
                    }
                    oscl_free(CompOfRole);
                }
                else
                {
                    // if no component supports the role, nothing else to do
                    return PVMFErrNotSupported;
                }

                if (status == OMX_FALSE)
                {
                    return PVMFErrNotSupported;
                }

                iNewWidth = aOutputParameters.width;
                iNewHeight = aOutputParameters.height;

                return PVMFSuccess;
            }
        }
        else
        {
            if (compcount == 4)
            {
                // Verify and set the passed-in video dec node setting
                PVMFStatus retval = DoVerifyAndSetVideoDecNodeParameter(aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Setting parameter %d failed", paramind));
                    return retval;
                }
            }
            else if (compcount == 5)
            {
                // Determine the 4th level component
                pv_mime_string_extract_type(3, aParameters[paramind].key, compstr);
                if (pv_mime_strcmp(compstr, _STRLIT_CHAR("h263")) >= 0)
                {
                    // Verify and set the passed-in H.263 decoder setting
                    PVMFStatus retval = DoVerifyAndSetH263DecoderParameter(aParameters[paramind], false);
                    if (retval != PVMFSuccess)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Setting parameter %d failed", paramind));
                        return retval;
                    }
                }
                else if (pv_mime_strcmp(compstr, _STRLIT_CHAR("m4v")) >= 0)
                {
                    // Verify and set the passed-in M4v decoder setting
                    PVMFStatus retval = DoVerifyAndSetM4VDecoderParameter(aParameters[paramind], false);
                    if (retval != PVMFSuccess)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Setting parameter %d failed", paramind));
                        return retval;
                    }
                }
                else
                {
                    // Unknown key sub-string
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Unsupported key"));
                    return PVMFErrArgument;
                }
            }
            else
            {
                // Do not support more than 5 components right now
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Unsupported key"));
                return PVMFErrArgument;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoCapConfigVerifyParameters() Out"));
    return PVMFSuccess;
}



PVMFStatus PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/video/decoder/"), 21);
    oscl_strncat(aParameters[0].key, PVOMXVideoDecNodeConfigBaseKeys[aIndex].iString, oscl_strlen(PVOMXVideoDecNodeConfigBaseKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (PVOMXVideoDecNodeConfigBaseKeys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_BITARRAY32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BITARRAY32_STRING), oscl_strlen(PVMI_KVPVALTYPE_BITARRAY32_STRING));
            break;

        case PVMI_KVPVALTYPE_KSV:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
            break;

        case PVMI_KVPVALTYPE_BOOL:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
            break;

        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (reqattr == PVMI_KVPATTR_CAP)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case 0:	// "postproc_enable"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.bool_value = iNodeConfig.iPostProcessingEnable;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.bool_value = PVOMXVIDEODECNODE_CONFIG_POSTPROCENABLE_DEF;
            }

            break;

        case 1:	// "postproc_type"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iNodeConfig.iPostProcessingMode;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVOMXVIDEODECNODE_CONFIG_POSTPROCTYPE_DEF;
            }

            break;

        case 2:	// "dropframe_enable"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.bool_value = iNodeConfig.iDropFrame;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.bool_value = PVOMXVIDEODECNODE_CONFIG_DROPFRAMEENABLE_DEF;
            }

            break;

        case 5: //"format-type"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.pChar_value = (char*)iNodeConfig.iMimeType.getMIMEStrPtr();
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.pChar_value = (char*)PVMF_MIME_FORMAT_UNKNOWN;
            }

            break;

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter() Invalid index to video dec node parameter"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetVideoDecNodeParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMFOMXVideoDecNode::DoGetH263DecoderParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/video/decoder/h263/"), 26);
    oscl_strncat(aParameters[0].key, PVOMXVideoDecNodeConfigH263Keys[aIndex].iString, oscl_strlen(PVOMXVideoDecNodeConfigH263Keys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (PVOMXVideoDecNodeConfigH263Keys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_RANGE_UINT32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            break;

        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (reqattr == PVMI_KVPATTR_CAP)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case 0:	// "maxbitstreamframesize"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iH263MaxBitstreamFrameSize;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MIN;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case 1:	// "maxdimension"
        {
            range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
            if (rui32 == NULL)
            {
                oscl_free(aParameters[0].key);
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Memory allocation for range uint32 failed"));
                return PVMFErrNoMemory;
            }

            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                rui32->min = iH263MaxWidth;
                rui32->max = iH263MaxHeight;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                rui32->min = PVOMXVIDEODECNODE_CONFIG_H263MAXWIDTH_DEF;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_H263MAXHEIGHT_DEF;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            else
            {
                // Return capability
                rui32->min = PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MIN;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
        }
        break;

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Invalid index to H.263 decoder parameter"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetH263DecoderParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMFOMXVideoDecNode::DoGetM4VDecoderParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE*sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/video/decoder/m4v/"), 25);
    oscl_strncat(aParameters[0].key, PVOMXVideoDecNodeConfigM4VKeys[aIndex].iString, oscl_strlen(PVOMXVideoDecNodeConfigM4VKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (PVOMXVideoDecNodeConfigM4VKeys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_RANGE_UINT32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            break;

        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (reqattr == PVMI_KVPATTR_CAP)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case 0:	// "maxbitstreamframesize"
            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                aParameters[0].value.uint32_value = iM4VMaxBitstreamFrameSize;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                aParameters[0].value.uint32_value = PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_DEF;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (rui32 == NULL)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MIN;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;

        case 1:	// "maxdimension"
        {
            range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
            if (rui32 == NULL)
            {
                oscl_free(aParameters[0].key);
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Memory allocation for range uint32 failed"));
                return PVMFErrNoMemory;
            }

            if (reqattr == PVMI_KVPATTR_CUR)
            {
                // Return current value
                rui32->min = iM4VMaxWidth;
                rui32->max = iM4VMaxHeight;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            else if (reqattr == PVMI_KVPATTR_DEF)
            {
                // Return default
                rui32->min = PVOMXVIDEODECNODE_CONFIG_M4VMAXWIDTH_DEF;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_M4VMAXHEIGHT_DEF;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            else
            {
                // Return capability
                rui32->min = PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MIN;
                rui32->max = PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MAX;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
        }
        break;

        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Invalid index to H.263 decoder parameter"));
            return PVMFErrArgument;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoGetM4VDecoderParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Valtype in key string unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the fourth component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(3, aParameter.key, compstr);

    int32 vdeccomp4ind = 0;
    for (vdeccomp4ind = 0; vdeccomp4ind < PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS; ++vdeccomp4ind)
    {
        // Go through each component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigBaseKeys[vdeccomp4ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (vdeccomp4ind == PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS || vdeccomp4ind == 3 || vdeccomp4ind == 4)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Unsupported key or non-leaf node"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVOMXVideoDecNodeConfigBaseKeys[vdeccomp4ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (vdeccomp4ind)
    {
        case 0: // "postproc_enable"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                iNodeConfig.iPostProcessingEnable = aParameter.value.bool_value;
            }
            break;

        case 1: // "postproc_type"
            // Nothing to validate since it is bitarray32
            // Change the config if to set
            if (aSetParam)
            {
                iNodeConfig.iPostProcessingMode = aParameter.value.uint32_value;
                if (iNodeConfig.iPostProcessingEnable && iOMXDecoder)
                {
                    // Don't do anything yet: Need to communicate post-processing to decoder
                    //iVideoDecoder->SetPostProcType(iNodeConfig.iPostProcessingMode);
                }
            }
            break;

        case 2: // "dropframe_enable"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iNodeConfig.iDropFrame = aParameter.value.bool_value;
            }
            break;

        case 5: // "format-type"
            // Nothing to validate since it is boolean
            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iNodeConfig.iMimeType = aParameter.value.pChar_value;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Invalid index for video dec node parameter"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Valtype in key string unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the fifth component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(4, aParameter.key, compstr);

    int32 vdeccomp5ind = 0;
    for (vdeccomp5ind = 0; vdeccomp5ind < PVOMXVIDEODECNODECONFIG_H263_NUMKEYS; ++vdeccomp5ind)
    {
        // Go through each component string at 5th level
        if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigH263Keys[vdeccomp5ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (vdeccomp5ind == PVOMXVIDEODECNODECONFIG_H263_NUMKEYS)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Unsupported key or non-leaf node"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVOMXVideoDecNodeConfigH263Keys[vdeccomp5ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (vdeccomp5ind)
    {
        case 0: // "maxbitstreamframesize"
            // Check if within range
            if (aParameter.value.uint32_value < PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MIN ||
                    aParameter.value.uint32_value > PVOMXVIDEODECNODE_CONFIG_H263MAXBITSTREAMFRAMESIZE_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Invalid value for maxbitstreamframesize"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iH263MaxBitstreamFrameSize = aParameter.value.uint32_value;
            }
            break;

        case 1: // "maxdimension"
        {
            range_uint32* rui32 = (range_uint32*)aParameter.value.key_specific_value;
            if (rui32 == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() ksv for maxdimension is NULL"));
                return PVMFErrArgument;
            }

            // Check if within range
            if (rui32->min < PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MIN ||
                    rui32->min > PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MAX ||
                    rui32->max < PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MIN ||
                    rui32->max > PVOMXVIDEODECNODE_CONFIG_H263MAXDIMENSION_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Invalid range for maxdimension"));
                return PVMFErrArgument;
            }

            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iH263MaxWidth = rui32->min;
                iH263MaxHeight = rui32->max;
            }
        }
        break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Invalid index for H.263 decoder parameter"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetH263DecoderParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Valtype in key string unknown"));
        return PVMFErrArgument;
    }
    // Retrieve the fifth component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(4, aParameter.key, compstr);

    int32 vdeccomp5ind = 0;
    for (vdeccomp5ind = 0; vdeccomp5ind < PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS; ++vdeccomp5ind)
    {
        // Go through each component string at 5th level
        if (pv_mime_strcmp(compstr, (char*)(PVOMXVideoDecNodeConfigM4VKeys[vdeccomp5ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (vdeccomp5ind == PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Unsupported key or non-leaf node"));
        return PVMFErrArgument;
    }

    // Verify the valtype
    if (keyvaltype != PVOMXVideoDecNodeConfigM4VKeys[vdeccomp5ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Valtype does not match for key"));
        return PVMFErrArgument;
    }

    switch (vdeccomp5ind)
    {
        case 0: // "maxbitstreamframesize"
            // Check if within range
            if (aParameter.value.uint32_value < PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MIN ||
                    aParameter.value.uint32_value > PVOMXVIDEODECNODE_CONFIG_M4VMAXBITSTREAMFRAMESIZE_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Invalid value for maxbitstreamframesize"));
                return PVMFErrArgument;
            }
            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iM4VMaxBitstreamFrameSize = aParameter.value.uint32_value;
            }
            break;

        case 1: // "maxdimension"
        {
            range_uint32* rui32 = (range_uint32*)aParameter.value.key_specific_value;
            if (rui32 == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() ksv for maxdimension is NULL"));
                return PVMFErrArgument;
            }

            // Check if within range
            if (rui32->min < PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MIN ||
                    rui32->min > PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MAX ||
                    rui32->max < PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MIN ||
                    rui32->max > PVOMXVIDEODECNODE_CONFIG_M4VMAXDIMENSION_MAX)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Invalid range for maxdimension"));
                return PVMFErrArgument;
            }

            // Change the config if to set
            if (aSetParam)
            {
                if (iInterfaceState == EPVMFNodeStarted || iInterfaceState == EPVMFNodePaused)
                {
                    // This setting cannot be changed when decoder has been initialized
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetVideoDecNodeParameter() Setting cannot be changed while started or paused"));
                    return PVMFErrInvalidState;
                }

                iM4VMaxWidth = rui32->min;
                iM4VMaxHeight = rui32->max;
            }
        }
        break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Invalid index for M4v decoder parameter"));
            return PVMFErrArgument;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::DoVerifyAndSetM4VDecoderParameter() Out"));
    return PVMFSuccess;
}



PVMFStatus PVMFOMXVideoDecNode::GetProfileAndLevel(PVMF_MPEGVideoProfileType& aProfile, PVMF_MPEGVideoLevelType& aLevel)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXVideoDecNode::GetProfileAndLevel() In"));

    if (NULL == iOMXDecoder)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::GetProfileAndLevel() iVideoDecoder is Null"));
        aProfile = PV_MPEG_VIDEO_RESERVED_PROFILE;
        aLevel	= PV_MPEG_VIDEO_LEVEL_UNKNOWN;
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::GetProfileAndLevel() iVideoDecoder is Null"));
    aProfile = PV_MPEG_VIDEO_RESERVED_PROFILE;
    aLevel	= PV_MPEG_VIDEO_LEVEL_UNKNOWN;
    // FOR NOW, JUST RETURN FAILURE, WE DON'T SUPPORT THIS FEATURE YET
    return PVMFFailure;


}


// DEFINITIONS for parsing the config information & sequence header for WMV

#define GetUnalignedDword( pb, dw ) \
            (dw) = ((uint32) *(pb + 3) << 24) + \
                   ((uint32) *(pb + 2) << 16) + \
                   ((uint16) *(pb + 1) << 8) + *pb;

#define GetUnalignedDwordEx( pb, dw )   GetUnalignedDword( pb, dw ); (pb) += sizeof(uint32);
#define LoadDWORD( dw, p )  GetUnalignedDwordEx( p, dw )
#ifndef MAKEFOURCC_WMC
#define MAKEFOURCC_WMC(ch0, ch1, ch2, ch3) \
        ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
        ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define mmioFOURCC_WMC(ch0, ch1, ch2, ch3)  MAKEFOURCC_WMC(ch0, ch1, ch2, ch3)
#endif

#define FOURCC_WMV3     mmioFOURCC_WMC('W','M','V','3')
#define FOURCC_WMV2     mmioFOURCC_WMC('W','M','V','2')
#define FOURCC_WMVA		mmioFOURCC_WMC('W','M','V','A')

//For WMV3
enum { NOT_WMV3 = -1, WMV3_SIMPLE_PROFILE, WMV3_MAIN_PROFILE, WMV3_PC_PROFILE, WMV3_SCREEN };

//For WMVA
#define ASFBINDING_SIZE                   1   // size of ASFBINDING is 1 byte
#define SC_SEQ          0x0F
#define SC_ENTRY        0x0E


bool PVMFOMXVideoDecNode::VerifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    // unused parameters
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);

    // call this in case of WMV format
    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMV)
    {

        //verify bitrate
        if (pv_mime_strcmp(aParameters->key, PVMF_BITRATE_VALUE_KEY) == 0)
        {
            if (((PVMFOMXDecPort*)iOutPort)->verifyConnectedPortParametersSync(PVMF_BITRATE_VALUE_KEY, &(aParameters->value.uint32_value)) != PVMFSuccess)
            {
                return false;
            }
            return true;
        }
        else if (pv_mime_strcmp(aParameters->key, PVMF_FRAMERATE_VALUE_KEY) == 0)
        {
            if (((PVMFOMXDecPort*)iOutPort)->verifyConnectedPortParametersSync(PVMF_FRAMERATE_VALUE_KEY, &(aParameters->value.uint32_value)) != PVMFSuccess)
            {
                return false;
            }
            return true;
        }
        else if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) < 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::VerifyParametersSync() - Unsupported Key"));
            return true;
        }

        // pConfig points to format specific info and sequence header.
        uint8 *pConfig = (uint8*)(aParameters->value.key_specific_value);
        uint8 *pData;
        uint32 dwdat;
        uint32 NewCompression;
        uint32 NewSeqHeader;
        uint32 NewProfile, NewFrameRate, NewBitRate;

        // We are interested in the following (and will extract it)
        //	1. Version (WMV9 or WMV8 etc.) (from format specific info)
        //  2. picture dimensions // from format specific info
        //  3. interlaced YUV411 /sprite content is not supported (from sequence header)
        //  4. framerate / bitrate information (from sequence header)

        pData = pConfig + 15; // position ptr to Width & Height

        LoadDWORD(dwdat, pData);
        iNewWidth = dwdat;
        LoadDWORD(dwdat, pData);
        iNewHeight = dwdat;

        if (((iNewWidth != (uint32)iYUVWidth) || (iNewHeight != (uint32)iYUVHeight)) && iOutPort != NULL)
        {
            // see if downstream node can handle the re-sizing
            int32 errcode = OsclErrNone;
            OsclRefCounterMemFrag yuvFsiMemfrag;
            OSCL_TRY(errcode, yuvFsiMemfrag = iFsiFragmentAlloc.get());


            OSCL_FIRST_CATCH_ANY(errcode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXVideoDecNode::VerifyParametersSync() Failed to allocate memory for verifyParametersSync FSI")));
            if (OsclErrNone == errcode)
            {
                PVMFYuvFormatSpecificInfo0* fsiInfo = OSCL_PLACEMENT_NEW(yuvFsiMemfrag.getMemFragPtr(), PVMFYuvFormatSpecificInfo0());
                if (fsiInfo != NULL)
                {
                    fsiInfo->video_format = PVMF_MIME_YUV420;
                    fsiInfo->uid = PVMFYuvFormatSpecificInfo0_UID;
                    fsiInfo->display_width = iNewWidth;
                    fsiInfo->display_height = iNewHeight;
                    fsiInfo->width = (iNewWidth + 3) & -4;
                    fsiInfo->height = iNewHeight;

                    if (((PVMFOMXDecPort*)iOutPort)->verifyConnectedPortParametersSync(PVMF_FORMAT_SPECIFIC_INFO_KEY, &yuvFsiMemfrag) != PVMFSuccess)
                    {
                        fsiInfo->video_format.~PVMFFormatType();
                        return false;
                    }
                    fsiInfo->video_format.~PVMFFormatType();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        pData += 4; //position ptr to Compression type

        LoadDWORD(dwdat, pData);
        NewCompression = dwdat;

        if (NewCompression != FOURCC_WMV2 &&
                NewCompression != FOURCC_WMV3 &&
                NewCompression != FOURCC_WMVA)
            return false;


        // Check sequence header
        switch (NewCompression)
        {
            case FOURCC_WMV3:
            {
                pData = pConfig + 11 + 40; //sizeof(BITMAPINFOHEADER); // position to sequence header

                LoadDWORD(dwdat, pData);
                NewSeqHeader = dwdat; // this is little endian read sequence header

                uint32 YUV411flag, Spriteflag;

                // For FOURCC_WMV3
                uint32 YUV411;
                uint32 SpriteMode;
                uint32 LoopFilter;
                uint32 Xintra8Switch;
                uint32 MultiresEnabled;
                uint32 X16bitXform;
                uint32 UVHpelBilinear;
                uint32 ExtendedMvMode;
                uint32 DQuantCodingOn;
                uint32 XformSwitch;
                uint32 DCTTable_MB_ENABLED;
                uint32 SequenceOverlap;
                uint32 StartCode;
                uint32 PreProcRange;
                uint32 NumBFrames;
                uint32 ExplicitSeqQuantizer;
                uint32 Use3QPDZQuantizer = 0;
                uint32 ExplicitFrameQuantizer = 0;


                bool bValidProfile = true;

                NewProfile = (NewSeqHeader & 0xC0) >> 6; // 0 - simple , 1- main, 3 - complex, 2-forbidden

                if (NewProfile == WMV3_PC_PROFILE)
                    return false;

                YUV411flag = (NewSeqHeader & 0x20) >> 5;
                Spriteflag = (NewSeqHeader & 0x10) >> 4;
                if ((YUV411flag != 0) || (Spriteflag != 0))
                    return false;

                YUV411				= (uint32)YUV411flag;
                SpriteMode			= (uint32)Spriteflag;
                LoopFilter			= (NewSeqHeader & 0x800) >> 11;
                Xintra8Switch		= (NewSeqHeader & 0x400) >> 10;
                MultiresEnabled		= (NewSeqHeader & 0x200) >> 9;
                X16bitXform			= (NewSeqHeader & 0x100) >> 8;
                UVHpelBilinear		= (NewSeqHeader & 0x800000) >> 23;
                ExtendedMvMode		= (NewSeqHeader & 0x400000) >> 22;
                DQuantCodingOn		= (NewSeqHeader & 0x300000) >> 20;
                XformSwitch			= (NewSeqHeader & 0x80000) >> 19;
                DCTTable_MB_ENABLED	= (NewSeqHeader & 0x40000) >> 18;
                SequenceOverlap		= (NewSeqHeader & 0x20000) >> 17;
                StartCode			= (NewSeqHeader & 0x10000) >> 16;
                PreProcRange			= (NewSeqHeader & 0x80000000) >> 31;
                NumBFrames			= (NewSeqHeader & 0x70000000) >> 28;
                ExplicitSeqQuantizer	= (NewSeqHeader & 0x8000000) >> 27;
                if (ExplicitSeqQuantizer)
                    Use3QPDZQuantizer = (NewSeqHeader & 0x4000000) >> 26;
                else
                    ExplicitFrameQuantizer = (NewSeqHeader & 0x4000000) >> 26;

                NewFrameRate = (NewSeqHeader & 0x0E) >> 1 ; // from 2 to 30 fps (in steps of 4)
                NewFrameRate = 4 * NewFrameRate + 2; // (in fps)

                NewBitRate = (((NewSeqHeader & 0xF000) >> 24) | ((NewSeqHeader & 0x01) << 8));  // from 32 to 2016 kbps in steps of 64kbps
                NewBitRate = 64 * NewBitRate + 32; // (in kbps)

                // Verify Profile
                if (!SpriteMode)
                {
                    if (NewProfile == WMV3_SIMPLE_PROFILE)
                    {
                        bValidProfile = (Xintra8Switch == 0) &&
                                        (X16bitXform == 1) &&
                                        (UVHpelBilinear == 1) &&
                                        (StartCode == 0) &&
                                        (LoopFilter == 0) &&
                                        (YUV411 == 0) &&
                                        (MultiresEnabled == 0) &&
                                        (DQuantCodingOn == 0) &&
                                        (NumBFrames == 0) &&
                                        (PreProcRange == 0);

                    }
                    else if (NewProfile == WMV3_MAIN_PROFILE)
                    {
                        bValidProfile = (Xintra8Switch == 0) &&
                                        (X16bitXform == 1);
                    }
                    else if (NewProfile == WMV3_PC_PROFILE)
                    {
                        // no feature restrictions for complex profile.
                    }

                    if (!bValidProfile)
                    {
                        return false;
                    }
                }
                else
                {
                    if (!Xintra8Switch   &&
                            !DCTTable_MB_ENABLED  &&
                            !YUV411 &&
                            !LoopFilter &&
                            !ExtendedMvMode &&
                            !MultiresEnabled &&
                            !UVHpelBilinear &&
                            !DQuantCodingOn &&
                            !XformSwitch &&
                            !StartCode &&
                            !PreProcRange &&
                            !ExplicitSeqQuantizer &&
                            !Use3QPDZQuantizer &&
                            !ExplicitFrameQuantizer)
                        return true;
                    else
                        return false;
                }
            }
            break;
            case FOURCC_WMVA:
            {
                pData = pConfig + 11 + 40 + ASFBINDING_SIZE; //sizeof(BITMAPINFOHEADER); // position to sequence header

                LoadDWORD(dwdat, pData);
                NewSeqHeader = dwdat; // this is little endian read sequence header

                int32 iPrefix;
                //ignore start code prefix
                iPrefix = NewSeqHeader & 0xFF;
                if (iPrefix != 0) return false;
                iPrefix = (NewSeqHeader & 0xFF00) >> 8;
                if (iPrefix != 0) return false;
                iPrefix = (NewSeqHeader & 0xFF0000) >> 16;
                if (iPrefix != 1) return false;
                iPrefix = (NewSeqHeader & 0xFF000000) >> 24;
                if (iPrefix != SC_SEQ) return false;

                LoadDWORD(dwdat, pData);
                NewSeqHeader = dwdat;

                NewProfile = (NewSeqHeader & 0xC0) >> 6;
                if (NewProfile != 3)
                    return false;
                pData += 3;
                LoadDWORD(dwdat, pData);
                NewSeqHeader = dwdat;
                //ignore start code prefix
                iPrefix = NewSeqHeader & 0xFF;
                if (iPrefix != 0) return false;
                iPrefix = (NewSeqHeader & 0xFF00) >> 8;
                if (iPrefix != 0) return false;
                iPrefix = (NewSeqHeader & 0xFF0000) >> 16;
                if (iPrefix != 1) return false;
                iPrefix = (NewSeqHeader & 0xFF000000) >> 24;
                if (iPrefix != SC_ENTRY) return false;
            }
            break;

            case FOURCC_WMV2:
                break;

            default:
                return false;
        }

    } // end of if(format == PVMF_MIME_WMV)
    return true;
}


