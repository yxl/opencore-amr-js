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
#include "pvmf_omx_audiodec_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_omx_basedec_port.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmi_kvp_util.h"
#include "latmpayloadparser.h"


#include "OMX_Core.h"
#include "pvmf_omx_basedec_callbacks.h"     //used for thin AO in Decoder's callbacks
#include "pv_omxcore.h"

// needed for capability and config
#include "pv_omx_config_parser.h"


#define CONFIG_SIZE_AND_VERSION(param) \
	    param.nSize=sizeof(param); \
	    param.nVersion.s.nVersionMajor = SPECVERSIONMAJOR; \
	    param.nVersion.s.nVersionMinor = SPECVERSIONMINOR; \
	    param.nVersion.s.nRevision = SPECREVISION; \
	    param.nVersion.s.nStep = SPECSTEP;



#define PVOMXAUDIODEC_MEDIADATA_POOLNUM 2*NUMBER_OUTPUT_BUFFER
#define PVOMXAUDIODEC_MEDIADATA_CHUNKSIZE 128


// Node default settings

#define PVOMXAUDIODECNODE_CONFIG_MIMETYPE_DEF 0

#define PVMF_OMXAUDIODEC_NUM_METADATA_VALUES 6

// Constant character strings for metadata keys
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY[] = "codec-info/audio/format";
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY[] = "codec-info/audio/channels";
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY[] = "codec-info/audio/sample-rate";
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_AVGBITRATE_KEY[] = "codec-info/audio/avgbitrate";
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_AACOBJECTTYPE_KEY[] = "codec-info/audio/aac-objecttype";
static const char PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_AACSTREAMTYPE_KEY[] = "codec-info/audio/aac-streamtype";


static const char PVOMXAUDIODECMETADATA_SEMICOLON[] = ";";

/////////////////////////////////////////////////////////////////////////////
// Class Destructor
/////////////////////////////////////////////////////////////////////////////
PVMFOMXAudioDecNode::~PVMFOMXAudioDecNode()
{
    DeleteLATMParser();
    ReleaseAllPorts();
}

/////////////////////////////////////////////////////////////////////////////
// Add AO to the scheduler
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXAudioDecNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXAudioDecNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
            {
                AddToScheduler();
                iIsAdded = true;
            }
            iLogger = PVLogger::GetLoggerObject("PVMFOMXAudioDecNode");
            iRunlLogger = PVLogger::GetLoggerObject("Run.PVMFOMXAudioDecNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath");
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.decnode.OMXAudioDecnode");

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
PVMFOMXAudioDecNode::PVMFOMXAudioDecNode(int32 aPriority) :
        PVMFOMXBaseDecNode(aPriority, "PVMFOMXAudioDecNode")
{
    iInterfaceState = EPVMFNodeCreated;

    iNodeConfig.iMimeType = PVOMXAUDIODECNODE_CONFIG_MIMETYPE_DEF;


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
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_MPEG4_AUDIO);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_3640);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_ADIF);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_LATM);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_ASF_MPEG4_AUDIO);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AAC_SIZEHDR);

             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR_IF2);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR_IETF);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMRWB_IETF);
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMRWB);

             iCapability.iInputFormatCapability.push_back(PVMF_MIME_MP3);

             iCapability.iInputFormatCapability.push_back(PVMF_MIME_WMA);

             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_PCM16);

             iAvailableMetadataKeys.reserve(PVMF_OMXAUDIODEC_NUM_METADATA_VALUES);
             iAvailableMetadataKeys.clear();
            );
    // LATM init
    iLATMParser = NULL;
    iLATMConfigBuffer = NULL;
    iLATMConfigBufferSize = 0;

    //Try Allocate FSI buffer

    // Do This first in case of Query
    OSCL_TRY(err, iFsiFragmentAlloc.size(PVOMXAUDIODEC_MEDIADATA_POOLNUM, sizeof(channelSampleInfo)));


    OSCL_TRY(err, iPrivateDataFsiFragmentAlloc.size(PVOMXAUDIODEC_MEDIADATA_POOLNUM, sizeof(OsclAny *)));
}

/////////////////////////////////////////////////////////////////////////////
// This routine will process incomming message from the port
/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXAudioDecNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one buffer off the port's
    //incoming data queue.  This routine will dequeue and
    //dispatch the data.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFOMXAudioDecNode::ProcessIncomingMsg: aPort=0x%x", this, aPort));

    PVMFStatus status = PVMFFailure;
#ifdef SIMULATE_DROP_MSGS
    if ((((PVMFOMXDecPort*)aPort)->iNumFramesConsumed % 300 == 299))  // && (((PVMFOMXDecPort*)aPort)->iNumFramesConsumed < 30) )
    {

        // just dequeue
        PVMFSharedMediaMsgPtr msg;

        status = aPort->DequeueIncomingMsg(msg);
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        status = aPort->DequeueIncomingMsg(msg);
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        status = aPort->DequeueIncomingMsg(msg);
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;

#ifdef _DEBUG
        printf("PVMFOMXAudioDecNode::ProcessIncomingMsg() SIMULATED DROP 3 MSGS\n");
#endif


    }
#endif

#ifdef SIMULATE_BOS

    if ((((PVMFOMXDecPort*)aPort)->iNumFramesConsumed == 6))
    {

        PVMFSharedMediaCmdPtr BOSCmdPtr = PVMFMediaCmd::createMediaCmd();

        // Set the format ID to BOS
        BOSCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);

        // Set the timestamp
        BOSCmdPtr->setTimestamp(201);
        BOSCmdPtr->setStreamID(0);

        // Convert to media message and send it out
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, BOSCmdPtr);

        //store the stream id and time stamp of bos message
        iStreamID = mediaMsgOut->getStreamID();
        iBOSTimestamp = mediaMsgOut->getTimestamp();
        iSendBOS = true;

#ifdef _DEBUG
        printf("PVMFOMXAudioDecNode::ProcessIncomingMsg() SIMULATED BOS\n");
#endif
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true;

    }
#endif

#ifdef SIMULATE_PREMATURE_EOS
    if (((PVMFOMXDecPort*)aPort)->iNumFramesConsumed == 5)
    {
        PVMFSharedMediaCmdPtr EOSCmdPtr = PVMFMediaCmd::createMediaCmd();

        // Set the format ID to EOS
        EOSCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

        // Set the timestamp
        EOSCmdPtr->setTimestamp(200);

        // Convert to media message and send it out
        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, EOSCmdPtr);

        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: SIMULATED EOS"));
#ifdef _DEBUG
        printf("PVMFOMXAudioDecNode::ProcessIncomingMsg() SIMULATED EOS\n");
#endif
        // Set EOS flag
        iEndOfDataReached = true;
        // Save the timestamp for the EOS cmd
        iEndOfDataTimestamp = mediaMsgOut->getTimestamp();

        return true;
    }

#endif



    PVMFSharedMediaMsgPtr msg;

    status = aPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFOMXAudioDecNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this));
        return false;
    }

    if (msg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
    {
        //store the stream id and time stamp of bos message
        iStreamID = msg->getStreamID();
        iBOSTimestamp = msg->getTimestamp();
        iSendBOS = true;

        // if new BOS arrives, and
        //if we're in the middle of a partial frame assembly
        // abandon it and start fresh
        if (iObtainNewInputBuffer == false)
        {
            if (iInputBufferUnderConstruction != NULL)
            {
                if (iInBufMemoryPool != NULL)
                {
                    iInBufMemoryPool->deallocate((OsclAny *)iInputBufferUnderConstruction);
                }
                iInputBufferUnderConstruction = NULL;
            }
            iObtainNewInputBuffer = true;

        }

        // needed to init the sequence numbers and timestamp for partial frame assembly
        iFirstDataMsgAfterBOS = true;
        iKeepDroppingMsgsUntilMarkerBit = false;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: Received BOS stream %d, timestamp %d", iStreamID, iBOSTimestamp));
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true;
    }
    else if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        // Set EOS flag
        iEndOfDataReached = true;
        // Save the timestamp for the EOS cmd
        iEndOfDataTimestamp = msg->getTimestamp();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: Received EOS"));

        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
        return true; // do not do conversion into media data, just set the flag and leave
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // For LATM data, need to convert to raw bitstream
    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM)
    {
        // Keep looping and parsing LATM data until frame complete or data queue runs out
        uint8 retval; //=FRAME_INCOMPLETE;
        // if LATM parser does not exist (very first frame), create it:
        if (iLATMParser == NULL)
        {
            // Create and configure the LATM parser based on the stream MUX config
            // which should be sent as the format specific config in the first media data
            if (CreateLATMParser() != PVMFSuccess)
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::Process Incoming Msg - LATM parser cannot be created"));
                OSCL_ASSERT(false);
                ReportErrorEvent(PVMFErrResourceConfiguration);
                ChangeNodeState(EPVMFNodeError);
                return true;
            }

            // get FSI
            OsclRefCounterMemFrag DataFrag;
            msg->getFormatSpecificInfo(DataFrag);

            //get pointer to the data fragment
            uint8* initbuffer = (uint8 *) DataFrag.getMemFragPtr();
            uint32 initbufsize = (int32) DataFrag.getMemFragSize();

            iLATMConfigBufferSize = initbufsize;
            iLATMConfigBuffer = iLATMParser->ParseStreamMuxConfig(initbuffer, (int32 *) & iLATMConfigBufferSize);
            if (iLATMConfigBuffer == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg() LATM Stream MUX config parsing failed"));
                OSCL_ASSERT(false);
                ReportErrorEvent(PVMFErrResourceConfiguration);
                ChangeNodeState(EPVMFNodeError);
                return true;
            }

        }

        do
        {
            if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
            {
                // Set EOS flag
                iEndOfDataReached = true;
                // Save the timestamp for the EOS cmd
                iEndOfDataTimestamp = msg->getTimestamp();

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: Received EOS"));

                ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
                return true; // do not do conversion into media data, just set the flag and leave

            }
            // Convert the next input media msg to media data
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, msg);


            ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDataPathLogger, PVLOGMSG_INFO,
                            (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: TS=%d, SEQNUM= %d", msg->getTimestamp(), msg->getSeqNum()));


            // Convert the LATM data to raw bitstream
            retval = iLATMParser->compose(mediaData);

            // if frame is complete, break out of the loop
            if (retval != FRAME_INCOMPLETE && retval != FRAME_ERROR)
                break;

            // frame is not complete, keep looping
            if (aPort->IncomingMsgQueueSize() == 0)
            {
                // no more data in the input port queue, unbind current msg, and return
                msg.Unbind();
                // enable reading more data from port
                break;
            }
            else
            {
                msg.Unbind();
                aPort->DequeueIncomingMsg(msg); // dequeue the message directly from input port

            }

            // Log parser error
            if (retval == FRAME_ERROR)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFAACDecNode::GetInputMediaData() LATM parser error"));
            }
        }
        while ((retval == FRAME_INCOMPLETE || retval == FRAME_ERROR));

        if (retval == FRAME_COMPLETE)
        {
            // Save the media data containing the parser data as the input media data
            iDataIn = iLATMParser->GetOutputBuffer();
            // set the MARKER bit on the data msg, since this is a complete frame produced by LATM parser
            iDataIn->setMarkerInfo(PVMF_MEDIA_DATA_MARKER_INFO_M_BIT);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: - LATM frame assembled"));

        }
        else if ((retval == FRAME_INCOMPLETE) || (retval == FRAME_ERROR))
        {
            // Do nothing and wait for more data to come in
            PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: - incomplete LATM"));
            // return immediately (i.e. don't assign anything to iDataIn, which will prevent
            // processing
            return true;
        }
        else if (retval == FRAME_OUTPUTNOTAVAILABLE)
        {
            // This should not happen since this node processes one parsed media data at a time
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: LATM parser OUTPUT NOT AVAILABLE"));

            msg.Unbind();

            OSCL_ASSERT(false);
            ReportErrorEvent(PVMFErrResourceConfiguration);
            ChangeNodeState(EPVMFNodeError);

            return true;
        }
    }
/////////////////////////////////////////////////////////
    //////////////////////////
    else
    {
        // regular (i.e. Non-LATM case)
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDataPathLogger, PVLOGMSG_INFO,
                        (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg: TS=%d, SEQNUM= %d", msg->getTimestamp(), msg->getSeqNum()));

        convertToPVMFMediaData(iDataIn, msg);
        ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed++;
    }

    iCurrFragNum = 0; // for new message, reset the fragment counter
    iIsNewDataFragment = true;


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXAudioDecNode::ProcessIncomingMsg() Received %d frames", ((PVMFOMXDecPort*)aPort)->iNumFramesConsumed));

    //return true if we processed an activity...
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// This routine will handle the PortReEnable state
/////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXAudioDecNode::HandlePortReEnable()
{
    // set the port index so that we get parameters for the proper port
    iParamPort.nPortIndex = iPortIndexForDynamicReconfig;

    CONFIG_SIZE_AND_VERSION(iParamPort);

    // get new parameters of the port
    OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

    // send command for port re-enabling (for this to happen, we must first recreate the buffers)
    OMX_SendCommand(iOMXDecoder, OMX_CommandPortEnable, iPortIndexForDynamicReconfig, NULL);


    // get also input info (for frame duration if necessary)
    OMX_ERRORTYPE Err;
    OMX_PTR CodecProfilePtr;
    OMX_INDEXTYPE CodecProfileIndx;
    OMX_AUDIO_PARAM_AACPROFILETYPE Audio_Aac_Param;

    // determine the proper index and structure (based on codec type)
    if (iInPort)
    {
        // AAC
        if (((PVMFOMXDecPort*)iInPort)->iFormat ==  PVMF_MIME_MPEG4_AUDIO ||
                ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640 ||
                ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM ||
                ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF ||
                ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ASF_MPEG4_AUDIO ||
                ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AAC_SIZEHDR) // for testing
        {
            CodecProfilePtr = (OMX_PTR) & Audio_Aac_Param;
            CodecProfileIndx = OMX_IndexParamAudioAac;
            Audio_Aac_Param.nPortIndex = iInputPortIndex;

            CONFIG_SIZE_AND_VERSION(Audio_Aac_Param);


            // get parameters:
            Err = OMX_GetParameter(iOMXDecoder, CodecProfileIndx, CodecProfilePtr);
            if (Err != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Input port parameters problem"));

                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrResource);
                return PVMFErrResource;
            }
        }
        // for AMR, frame sizes are known, no need to get the parameters
        // for MP3, frame sizes cannot be obtained through OMX params
        // for WMA, frame sizes cannot be obtained through OMX params
    }



    PVMFFormatType Format = PVMF_MIME_FORMAT_UNKNOWN;
    if (iInPort != NULL)
    {
        Format = ((PVMFOMXDecPort*)iInPort)->iFormat;
    }
    if (Format ==  PVMF_MIME_MPEG4_AUDIO ||
            Format == PVMF_MIME_3640 ||
            Format == PVMF_MIME_LATM ||
            Format == PVMF_MIME_ADIF ||
            Format == PVMF_MIME_ASF_MPEG4_AUDIO ||
            Format == PVMF_MIME_AAC_SIZEHDR) // for testing
    {
        iSamplesPerFrame = Audio_Aac_Param.nFrameLength;
    }
    // AMR
    else if (Format == PVMF_MIME_AMR_IF2 ||
             Format == PVMF_MIME_AMR_IETF ||
             Format == PVMF_MIME_AMR)
    {
        // AMR NB has fs=8khz Mono and the frame is 20ms long, i.e. there is 160 samples per frame
        iSamplesPerFrame = PVOMXAUDIODEC_AMRNB_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_AMRWB_IETF ||
             Format == PVMF_MIME_AMRWB)
    {
        // AMR WB has fs=16khz Mono and the frame is 20ms long, i.e. there is 320 samples per frame
        iSamplesPerFrame = PVOMXAUDIODEC_AMRWB_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_MP3)
    {
        // frame size is either 576 or 1152 samples per frame. However, this information cannot be
        // obtained through OMX MP3 Params. Assume that it's 1152
        iSamplesPerFrame = PVOMXAUDIODEC_MP3_DEFAULT_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_WMA)
    {
        // output frame size is unknown in WMA. However, the PV-WMA decoder can control the number
        // of samples it places in an output buffer, so we can create an output buffer of arbitrary size
        // and let the decoder control how it is filled
        iSamplesPerFrame = 0; // unknown
    }

    // is this output port?
    if (iPortIndexForDynamicReconfig == iOutputPortIndex)
    {

        // GET the output buffer params and sizes
        OMX_AUDIO_PARAM_PCMMODETYPE Audio_Pcm_Param;
        Audio_Pcm_Param.nPortIndex = iOutputPortIndex; // we're looking for output port params

        CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);



        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot get component output parameters"));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrResource);
            return PVMFErrResource;
        }

        iPCMSamplingRate = Audio_Pcm_Param.nSamplingRate; // can be set to 0 (if unknown)

        if (iPCMSamplingRate == 0) // use default sampling rate (i.e. 48000)
            iPCMSamplingRate = PVOMXAUDIODEC_DEFAULT_SAMPLINGRATE;

        iNumberOfAudioChannels = Audio_Pcm_Param.nChannels;		// should be 1 or 2
        if (iNumberOfAudioChannels != 1 && iNumberOfAudioChannels != 2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Output parameters num channels = %d", iNumberOfAudioChannels));

            SetState(EPVMFNodeError);
            ReportErrorEvent(PVMFErrResource);
            return PVMFErrResource;
        }

        if ((iSamplesPerFrame != 0) && ((iSamplesPerFrame * 1000) > iPCMSamplingRate))
            // if this iSamplesPerFrame is known and is large enough to ensure that the iMilliSecPerFrame calculation
            // below won't be set to 0.
        {
            // CALCULATE NumBytes per frame, Msec per frame, etc.
            iNumBytesPerFrame = 2 * iSamplesPerFrame * iNumberOfAudioChannels;
            iMilliSecPerFrame = (iSamplesPerFrame * 1000) / iPCMSamplingRate;
            // Determine the size of each PCM output buffer. Size would be big enough to hold certain time amount of PCM data
            uint32 numframes = PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME / iMilliSecPerFrame;

            if (PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME % iMilliSecPerFrame)
            {
                // If there is a remainder, include one more frame
                ++numframes;
            }

            // set the output buffer size accordingly:
            iOMXComponentOutputBufferSize = numframes * iNumBytesPerFrame;
        }
        else
            iOMXComponentOutputBufferSize = (2 * iNumberOfAudioChannels * PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME * iPCMSamplingRate) / 1000; // assuming 16 bits per sample

        if (iOMXComponentOutputBufferSize < iParamPort.nBufferSize)
        {
            // the OMX spec says that nBuffersize is a read only field, but the client is allowed to allocate
            // a buffer size larger than nBufferSize.
            iOMXComponentOutputBufferSize = iParamPort.nBufferSize;
        }


        // do we need to increase the number of buffers?
        if (iNumOutputBuffers < iParamPort.nBufferCountMin)
            iNumOutputBuffers = iParamPort.nBufferCountMin;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::HandlePortReEnable() new output buffers %d, size %d", iNumOutputBuffers, iOMXComponentOutputBufferSize));

        //Send the FSI information to media output node here, before setting output
        //port parameters to the omx component
        // Check if Fsi configuration need to be sent
        sendFsi = true;
        iCompactFSISettingSucceeded = false;

        if (sendFsi)
        {
            int fsiErrorCode = 0;

            OsclRefCounterMemFrag FsiMemfrag;

            OSCL_TRY(fsiErrorCode, FsiMemfrag = iFsiFragmentAlloc.get(););

            OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                 (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Failed to allocate memory for  FSI")));

            if (fsiErrorCode == 0)
            {
                channelSampleInfo* pcminfo = (channelSampleInfo*) FsiMemfrag.getMemFragPtr();

                if (pcminfo != NULL)
                {
                    OSCL_ASSERT(pcminfo != NULL);

                    pcminfo->samplingRate    = iPCMSamplingRate;
                    pcminfo->desiredChannels = iNumberOfAudioChannels;
                    pcminfo->bitsPerSample = 16;
                    pcminfo->num_buffers = iNumOutputBuffers;
                    pcminfo->buffer_size = iOMXComponentOutputBufferSize;

                    OsclMemAllocator alloc;
                    int32 KeyLength = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY_PCM) + 1;
                    PvmiKeyType KvpKey = (PvmiKeyType)alloc.ALLOCATE(KeyLength);

                    if (NULL == KvpKey)
                    {
                        return false;
                    }

                    oscl_strncpy(KvpKey, PVMF_FORMAT_SPECIFIC_INFO_KEY_PCM, KeyLength);
                    int32 err;

                    OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(FsiMemfrag, KvpKey););
                    if (err != OsclErrNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXAudioDecNode::HandlePortReEnable - Problem to set FSI"));
                    }
                    else
                    {
                        sendFsi = false;
                        iCompactFSISettingSucceeded = true;
                    }


                    alloc.deallocate((OsclAny*)(KvpKey));
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXAudioDecNode::HandlePortReEnable - Problem allocating Output FSI"));
                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return false; // this is going to make everything go out of scope
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable - Problem allocating Output FSI"));
                return false; // this is going to make everything go out of scope
            }


        }


        //Buffer allocator kvp query and allocation has to be done again if we landed into handle port reconfiguration

        PvmiKvp* kvp = NULL;
        int numKvp = 0;
        PvmiKeyType aIdentifier = (PvmiKeyType)PVMF_BUFFER_ALLOCATOR_KEY;
        int32 err, err1;
        ipExternalOutputBufferAllocatorInterface = NULL;

        OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiGetBufferAllocatorSpecificInfoSync(aIdentifier, kvp, numKvp););

        if ((err == OsclErrNone) && (NULL != kvp))
        {
            ipExternalOutputBufferAllocatorInterface = (PVInterface *)kvp->value.key_specific_value;

            if (ipExternalOutputBufferAllocatorInterface)
            {
                PVInterface* pTempPVInterfacePtr = NULL;
                OSCL_TRY(err, ipExternalOutputBufferAllocatorInterface->queryInterface(PVMFFixedSizeBufferAllocUUID, pTempPVInterfacePtr););

                OSCL_TRY(err1, ((PVMFOMXDecPort*)iOutPort)->releaseParametersSync(kvp, numKvp););

                if (err1 != OsclErrNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXAudioDecNode::HandlePortReEnable - Unable to Release Parameters"));
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
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot allocate output buffers "));

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
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable() out_ctrl_struct_ptr == NULL"));

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
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable()  out_buff_hdr_ptr == NULL"));

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
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot provide output buffers to component"));

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
                        (0, "PVMFOMXAudioDecNode::HandlePortReEnable() new buffers %d, size %d", iNumInputBuffers, iOMXComponentInputBufferSize));

        /* Allocate input buffers */
        if (!CreateInputMemPool(iNumInputBuffers))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot allocate new input buffers to component"));

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
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable() in_ctrl_struct_ptr == NULL"));

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
                                (0, "PVMFOMXAudioDecNode::HandlePortReEnable()  in_buff_hdr_ptr == NULL"));

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
                            (0, "PVMFOMXAudioDecNode::HandlePortReEnable() Port Reconfiguration -> Cannot provide new input buffers to component"));

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
bool PVMFOMXAudioDecNode::NegotiateComponentParameters(OMX_PTR aOutputParameters)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() In"));

    OMX_ERRORTYPE Err;
    // first get the number of ports and port indices
    OMX_PORT_PARAM_TYPE AudioPortParameters;
    uint32 NumPorts;
    uint32 ii;


    pvAudioConfigParserInputs aInputs;
    AudioOMXConfigParserOutputs *pOutputParameters;

    aInputs.inPtr = (uint8*)((PVMFOMXDecPort*)iInPort)->iTrackConfig;
    aInputs.inBytes = (int32)((PVMFOMXDecPort*)iInPort)->iTrackConfigSize;
    aInputs.iMimeType = ((PVMFOMXDecPort*)iInPort)->iFormat;
    pOutputParameters = (AudioOMXConfigParserOutputs *)aOutputParameters;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Calling audio config parser - TrackConfig = %p, TrackConfigSize = %d, mimetype = %s", aInputs.inPtr, aInputs.inBytes, aInputs.iMimeType.getMIMEStrPtr()));



    if (aInputs.iMimeType == PVMF_MIME_WMA)
    {
        iNumberOfAudioChannels = pOutputParameters->Channels;
        iPCMSamplingRate = pOutputParameters->SamplesPerSec;
    }
    else if (aInputs.iMimeType == PVMF_MIME_MPEG4_AUDIO ||
             aInputs.iMimeType == PVMF_MIME_3640 ||
             aInputs.iMimeType == PVMF_MIME_LATM ||
             aInputs.iMimeType == PVMF_MIME_ADIF ||
             aInputs.iMimeType == PVMF_MIME_ASF_MPEG4_AUDIO ||
             aInputs.iMimeType == PVMF_MIME_AAC_SIZEHDR)

    {
        iNumberOfAudioChannels = pOutputParameters->Channels;
    }

    CONFIG_SIZE_AND_VERSION(AudioPortParameters);
    // get starting number
    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioInit, &AudioPortParameters);
    NumPorts = AudioPortParameters.nPorts; // must be at least 2 of them (in&out)

    if (Err != OMX_ErrorNone || NumPorts < 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() There is insuffucient (%d) ports", NumPorts));
        return false;
    }


    // loop through ports starting from the starting index to find index of the first input port
    for (ii = AudioPortParameters.nStartPortNumber ;ii < AudioPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has input direction is picked

        iParamPort.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);

        //port
        iParamPort.nPortIndex = ii;

        CONFIG_SIZE_AND_VERSION(iParamPort);

        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating with port %d ", ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Found Input port index %d ", ii));

            iInputPortIndex = ii;
            break;
        }
    }
    if (ii == AudioPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Cannot find any input port "));
        return false;
    }


    // loop through ports starting from the starting index to find index of the first output port
    for (ii = AudioPortParameters.nStartPortNumber ;ii < AudioPortParameters.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has output direction is picked


        //port
        iParamPort.nPortIndex = ii;

        CONFIG_SIZE_AND_VERSION(iParamPort);

        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating with port %d ", ii));

            return false;
        }

        if (iParamPort.eDir == OMX_DirOutput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Found Output port index %d ", ii));

            iOutputPortIndex = ii;
            break;
        }
    }
    if (ii == AudioPortParameters.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Cannot find any output port "));
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
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating with input port %d ", iInputPortIndex));
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
                    (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Inport buffers %d,size %d", iNumInputBuffers, iOMXComponentInputBufferSize));




    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iInputPortIndex;
    // finalize setting input port parameters
    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting parameters in input port %d ", iInputPortIndex));
        return false;
    }



    // Codec specific info set/get: SamplingRate, formats etc.
    // NOTE: iParamPort is modified in the routine below - it is loaded from the component output port values
    // Based on sampling rate - we also determine the desired output buffer size
    if (!GetSetCodecSpecificInfo())
        return false;



    //Port 1 for output port
    iParamPort.nPortIndex = iOutputPortIndex;

    CONFIG_SIZE_AND_VERSION(iParamPort);

    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating with output port %d ", iOutputPortIndex));
        return false;
    }

    // set number of output buffers and the size
    iNumOutputBuffers = iParamPort.nBufferCountActual;

    if (iNumOutputBuffers > NUMBER_OUTPUT_BUFFER)
        iNumOutputBuffers = NUMBER_OUTPUT_BUFFER; // make sure to limit this number to what the port can hold


    if (iNumOutputBuffers < iParamPort.nBufferCountMin)
        iNumOutputBuffers = iParamPort.nBufferCountMin;


    //Send the FSI information to media output node here, before setting output
    //port parameters to the omx component

    // Check if Fsi configuration need to be sent
    sendFsi = true;
    iCompactFSISettingSucceeded = false;
    if (sendFsi)
    {
        int fsiErrorCode = 0;

        OsclRefCounterMemFrag FsiMemfrag;

        OSCL_TRY(fsiErrorCode, FsiMemfrag = iFsiFragmentAlloc.get(););

        OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                             (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Failed to allocate memory for  FSI")));

        if (fsiErrorCode == 0)
        {
            channelSampleInfo* pcminfo = (channelSampleInfo*) FsiMemfrag.getMemFragPtr();
            if (pcminfo != NULL)
            {
                OSCL_ASSERT(pcminfo != NULL);

                pcminfo->samplingRate    = iPCMSamplingRate;
                pcminfo->desiredChannels = iNumberOfAudioChannels;
                pcminfo->bitsPerSample = 16;
                pcminfo->num_buffers = iNumOutputBuffers;
                pcminfo->buffer_size = iOMXComponentOutputBufferSize;

                OsclMemAllocator alloc;
                int32 KeyLength = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY_PCM) + 1;
                PvmiKeyType KvpKey = (PvmiKeyType)alloc.ALLOCATE(KeyLength);

                if (NULL == KvpKey)
                {
                    return false;
                }

                oscl_strncpy(KvpKey, PVMF_FORMAT_SPECIFIC_INFO_KEY_PCM, KeyLength);
                int32 err;

                OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(FsiMemfrag, KvpKey););
                if (err != OsclErrNone)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters - Problem to set FSI"));
                }
                else
                {
                    sendFsi = false;
                    iCompactFSISettingSucceeded = true;
                }

                alloc.deallocate((OsclAny*)(KvpKey));


            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters - Problem allocating Output FSI"));
                SetState(EPVMFNodeError);
                ReportErrorEvent(PVMFErrNoMemory);
                return false; // this is going to make everything go out of scope
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters - Problem allocating Output FSI"));
            return false; // this is going to make everything go out of scope
        }


    }

    //Try querying the buffer allocator KVP for output buffer allocation outside of the node
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
                                (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters - Unable to Release Parameters"));
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

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Outport buffers %d,size %d", iNumOutputBuffers, iOMXComponentOutputBufferSize));

    CONFIG_SIZE_AND_VERSION(iParamPort);
    iParamPort.nPortIndex = iOutputPortIndex;
    // finalize setting output port parameters
    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting parameters in output port %d ", iOutputPortIndex));
        return false;
    }

    //Set input audio format
    //This is need it since a single component could handle differents roles

    // Init to desire format
    PVMFFormatType Format = PVMF_MIME_FORMAT_UNKNOWN;
    if (iInPort != NULL)
    {
        Format = ((PVMFOMXDecPort*)iInPort)->iFormat;
    }
    if (Format == PVMF_MIME_MPEG4_AUDIO ||
            Format == PVMF_MIME_3640 ||
            Format == PVMF_MIME_LATM ||
            Format == PVMF_MIME_ADIF ||
            Format == PVMF_MIME_ASF_MPEG4_AUDIO ||
            Format == PVMF_MIME_AAC_SIZEHDR)
    {
        iOMXAudioCompressionFormat = OMX_AUDIO_CodingAAC;
    }
    else if (Format == PVMF_MIME_AMR_IF2 ||
             Format == PVMF_MIME_AMR_IETF ||
             Format == PVMF_MIME_AMR ||
             Format == PVMF_MIME_AMRWB_IETF ||
             Format == PVMF_MIME_AMRWB)
    {
        iOMXAudioCompressionFormat = OMX_AUDIO_CodingAMR;
    }
    else if (Format == PVMF_MIME_MP3)
    {
        iOMXAudioCompressionFormat = OMX_AUDIO_CodingMP3;
    }
    else if (Format == PVMF_MIME_WMA)
    {
        iOMXAudioCompressionFormat = OMX_AUDIO_CodingWMA;
    }
    else
    {
        // Illegal codec specified.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting audio compression format"));
        return false;
    }


    OMX_AUDIO_PARAM_PORTFORMATTYPE AudioPortFormat;
    CONFIG_SIZE_AND_VERSION(AudioPortFormat);
    AudioPortFormat.nPortIndex = iInputPortIndex;

    // Search the proper format index and set it.
    // Since we already know that the component has the role we need, search until finding the proper nIndex
    // if component does not find the format will return OMX_ErrorNoMore

    for (ii = 0;; ii++)
    {
        AudioPortFormat.nIndex = ii;
        Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioPortFormat, &AudioPortFormat);
        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting audio compression format"));
            return false;
        }
        if (iOMXAudioCompressionFormat == AudioPortFormat.eEncoding)
        {
            break;
        }
    }
    // Now set the format to confirm parameters
    Err = OMX_SetParameter(iOMXDecoder, OMX_IndexParamAudioPortFormat, &AudioPortFormat);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting audio compression format"));
        return false;
    }


    return true;
}

bool PVMFOMXAudioDecNode::GetSetCodecSpecificInfo()
{

    // for AAC, need to let the decoder know about the type of AAC format. Need to get the frame length
    // need to get the parameters
    OMX_PTR CodecProfilePtr = NULL;
    OMX_INDEXTYPE CodecProfileIndx = OMX_IndexAudioStartUnused;
    OMX_AUDIO_PARAM_AACPROFILETYPE Audio_Aac_Param;
    OMX_AUDIO_PARAM_AMRTYPE Audio_Amr_Param;
    OMX_AUDIO_PARAM_MP3TYPE Audio_Mp3_Param;
    OMX_AUDIO_PARAM_WMATYPE Audio_Wma_Param;
    OMX_ERRORTYPE Err = OMX_ErrorNone;
    PVMFFormatType Format = PVMF_MIME_FORMAT_UNKNOWN;

    // determine the proper index and structure (based on codec type)

    if (iInPort != NULL)
    {
        Format = ((PVMFOMXDecPort*)iInPort)->iFormat;
    }
    if (Format ==  PVMF_MIME_MPEG4_AUDIO ||
            Format == PVMF_MIME_3640 ||
            Format == PVMF_MIME_LATM ||
            Format == PVMF_MIME_ADIF ||
            Format == PVMF_MIME_ASF_MPEG4_AUDIO ||
            Format == PVMF_MIME_AAC_SIZEHDR) // for testing
    {
        // AAC

        CodecProfilePtr = (OMX_PTR) & Audio_Aac_Param;
        CodecProfileIndx = OMX_IndexParamAudioAac;
        Audio_Aac_Param.nPortIndex = iInputPortIndex;

        CONFIG_SIZE_AND_VERSION(Audio_Aac_Param);
    }
    // AMR
    else if (Format ==  PVMF_MIME_AMR_IF2 ||
             Format == PVMF_MIME_AMR_IETF ||
             Format == PVMF_MIME_AMR ||
             Format == PVMF_MIME_AMRWB_IETF ||
             Format == PVMF_MIME_AMRWB)
    {
        CodecProfilePtr = (OMX_PTR) & Audio_Amr_Param;
        CodecProfileIndx = OMX_IndexParamAudioAmr;
        Audio_Amr_Param.nPortIndex = iInputPortIndex;

        CONFIG_SIZE_AND_VERSION(Audio_Amr_Param);
    }
    else if (Format == PVMF_MIME_MP3)
    {
        CodecProfilePtr = (OMX_PTR) & Audio_Mp3_Param;
        CodecProfileIndx = OMX_IndexParamAudioMp3;
        Audio_Mp3_Param.nPortIndex = iInputPortIndex;

        CONFIG_SIZE_AND_VERSION(Audio_Mp3_Param);
    }
    else if (Format == PVMF_MIME_WMA)
    {
        CodecProfilePtr = (OMX_PTR) & Audio_Wma_Param;
        CodecProfileIndx = OMX_IndexParamAudioWma;
        Audio_Wma_Param.nPortIndex = iInputPortIndex;

        CONFIG_SIZE_AND_VERSION(Audio_Wma_Param);
    }

    // first get parameters:
    Err = OMX_GetParameter(iOMXDecoder, CodecProfileIndx, CodecProfilePtr);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem getting codec profile parameter on input port %d ", iInputPortIndex));
        return false;
    }
    // Set the stream format


    // AAC FORMATS:
    if (Format ==  PVMF_MIME_MPEG4_AUDIO)
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    }
    else if (Format ==  PVMF_MIME_3640)
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    }
    else if (Format == PVMF_MIME_LATM)
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4LATM;
    }
    else if (Format == PVMF_MIME_ADIF)
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatADIF;
    }
    else if (Format == PVMF_MIME_ASF_MPEG4_AUDIO)
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    }
    else if (Format == PVMF_MIME_AAC_SIZEHDR) // for testing
    {
        Audio_Aac_Param.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    }
    // AMR FORMATS
    else if (Format == PVMF_MIME_AMR_IF2)
    {
        Audio_Amr_Param.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatIF2;
        Audio_Amr_Param.eAMRBandMode = OMX_AUDIO_AMRBandModeNB0; // we don't know the bitrate yet, but for init
        // purposes, we'll set this to any NarrowBand bitrate
        // to indicate NB vs WB
    }
    // File format
    // NB
    else if (Format == PVMF_MIME_AMR_IETF)
    {
        Audio_Amr_Param.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;
        Audio_Amr_Param.eAMRBandMode = OMX_AUDIO_AMRBandModeNB0; // we don't know the bitrate yet, but for init
        // purposes, we'll set this to any NarrowBand bitrate
        // to indicate NB vs WB
    }
    // WB
    else if (Format == PVMF_MIME_AMRWB_IETF)
    {
        Audio_Amr_Param.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;
        Audio_Amr_Param.eAMRBandMode = OMX_AUDIO_AMRBandModeWB0; // we don't know the bitrate yet, but for init
        // purposes, we'll set this to any WideBand bitrate
        // to indicate NB vs WB
    }
    // streaming with Table of Contents
    else if (Format == PVMF_MIME_AMR)
    {
        Audio_Amr_Param.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatRTPPayload;
        Audio_Amr_Param.eAMRBandMode = OMX_AUDIO_AMRBandModeNB0; // we don't know the bitrate yet, but for init
        // purposes, we'll set this to any WideBand bitrate
        // to indicate NB vs WB
    }
    else if (Format == PVMF_MIME_AMRWB)
    {
        Audio_Amr_Param.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatRTPPayload;
        Audio_Amr_Param.eAMRBandMode = OMX_AUDIO_AMRBandModeWB0; // we don't know the bitrate yet, but for init
        // purposes, we'll set this to any WideBand bitrate
        // to indicate NB vs WB
    }
    else if (Format == PVMF_MIME_MP3)
    {
        // nothing to do here
    }
    else if (Format == PVMF_MIME_WMA)
    {
        Audio_Wma_Param.eFormat = OMX_AUDIO_WMAFormatUnused; // set this initially
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Unknown format in input port negotiation "));
        return false;
    }

    // set parameters to inform teh component of the stream type
    Err = OMX_SetParameter(iOMXDecoder, CodecProfileIndx, CodecProfilePtr);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem setting codec profile parameter on input port %d ", iInputPortIndex));
        return false;
    }


    // read the output frame size
    // AAC
    if (Format ==  PVMF_MIME_MPEG4_AUDIO ||
            Format == PVMF_MIME_3640 ||
            Format == PVMF_MIME_LATM ||
            Format == PVMF_MIME_ADIF ||
            Format == PVMF_MIME_ASF_MPEG4_AUDIO ||
            Format == PVMF_MIME_AAC_SIZEHDR) // for testing
    {
        // AAC frame size is 1024 samples or 2048 samples for AAC-HE
        iSamplesPerFrame = Audio_Aac_Param.nFrameLength;
    }
    // AMR
    else if (Format == PVMF_MIME_AMR_IF2 ||
             Format == PVMF_MIME_AMR_IETF ||
             Format == PVMF_MIME_AMR)
    {
        // AMR NB has fs=8khz Mono and the frame is 20ms long, i.e. there is 160 samples per frame
        iSamplesPerFrame = PVOMXAUDIODEC_AMRNB_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_AMRWB_IETF ||
             Format == PVMF_MIME_AMRWB)
    {
        // AMR WB has fs=16khz Mono and the frame is 20ms long, i.e. there is 320 samples per frame
        iSamplesPerFrame = PVOMXAUDIODEC_AMRWB_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_MP3)
    {
        // frame size is either 576 or 1152 samples per frame. However, this information cannot be
        // obtained through OMX MP3 Params. Assume that it's 1152
        iSamplesPerFrame = PVOMXAUDIODEC_MP3_DEFAULT_SAMPLES_PER_FRAME;
    }
    else if (Format == PVMF_MIME_WMA)
    {
        // output frame size is unknown in WMA. However, the PV-WMA decoder can control the number
        // of samples it places in an output buffer, so we can create an output buffer of arbitrary size
        // and let the decoder control how it is filled
        iSamplesPerFrame = 0; // unknown
    }

    // iSamplesPerFrame depends on the codec.
    // for AAC: iSamplesPerFrame = 1024
    // for AAC+: iSamplesPerFrame = 2048
    // for AMRNB: iSamplesPerFrame = 160
    // for AMRWB: iSamplesPerFrame = 320
    // for MP3:	  iSamplesPerFrame = unknown, but either 1152 or 576 (we pick 1152 as default)
    // for WMA:	   unknown (iSamplesPerFrame is set to 0)

    // GET the output buffer params and sizes
    OMX_AUDIO_PARAM_PCMMODETYPE Audio_Pcm_Param;
    Audio_Pcm_Param.nPortIndex = iOutputPortIndex; // we're looking for output port params

    CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);


    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating PCM parameters with output port %d ", iOutputPortIndex));
        return false;
    }


    // these are some initial default values that may change
    iPCMSamplingRate = Audio_Pcm_Param.nSamplingRate; // can be set to 0 (if unknown)

    if (iPCMSamplingRate == 0) // use default sampling rate (i.e. 48000)
        iPCMSamplingRate = PVOMXAUDIODEC_DEFAULT_SAMPLINGRATE;

    iNumberOfAudioChannels = Audio_Pcm_Param.nChannels;		// should be 1 or 2
    if (iNumberOfAudioChannels != 1 && iNumberOfAudioChannels != 2)
        return false;


    if ((iSamplesPerFrame != 0) && ((iSamplesPerFrame * 1000) > iPCMSamplingRate))
        // if this iSamplesPerFrame is known and is large enough to ensure that the iMilliSecPerFrame calculation
        // below won't be set to 0.
    {
        // CALCULATE NumBytes per frame, Msec per frame, etc.

        iNumBytesPerFrame = 2 * iSamplesPerFrame * iNumberOfAudioChannels;
        iMilliSecPerFrame = (iSamplesPerFrame * 1000) / iPCMSamplingRate;
        // Determine the size of each PCM output buffer. Size would be big enough to hold certain time amount of PCM data
        uint32 numframes = PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME / iMilliSecPerFrame;

        if (PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME % iMilliSecPerFrame)
        {
            // If there is a remainder, include one more frame
            ++numframes;
        }
        // set the output buffer size accordingly:
        iOMXComponentOutputBufferSize = numframes * iNumBytesPerFrame;
    }
    else
        iOMXComponentOutputBufferSize = (2 * iNumberOfAudioChannels * PVOMXAUDIODEC_DEFAULT_OUTPUTPCM_TIME * iPCMSamplingRate) / 1000; // assuming 16 bits per sample

    //Port 1 for output port
    iParamPort.nPortIndex = iOutputPortIndex;
    CONFIG_SIZE_AND_VERSION(iParamPort);
    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamPortDefinition, &iParamPort);
    if (Err != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters() Problem negotiating with output port %d ", iOutputPortIndex));
        return false;
    }

    if (iOMXComponentOutputBufferSize < iParamPort.nBufferSize)
    {
        // the OMX spec says that nBuffersize is a read only field, but the client is allowed to allocate
        // a buffer size larger than nBufferSize.
        iOMXComponentOutputBufferSize = iParamPort.nBufferSize;
    }

    return true;

}

/////////////////////////////////////////////////////////////////////////////
bool PVMFOMXAudioDecNode::InitDecoder(PVMFSharedMediaDataPtr& DataIn)
{

    OsclRefCounterMemFrag DataFrag;
    OsclRefCounterMemFrag refCtrMemFragOut;
    uint8* initbuffer = NULL;
    uint32 initbufsize = 0;


    // NOTE: the component may not start decoding without providing the Output buffer to it,
    //		here, we're sending input/config buffers.
    //		Then, we'll go to ReadyToDecode state and send output as well

    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM)
    {
        // must have the LATM config buffer and size already present
        if (iLATMConfigBuffer != NULL)
        {
            initbuffer = iLATMConfigBuffer;
            initbufsize = iLATMConfigBufferSize;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::InitDecoder() Error - LATM config buffer not present"));
            return false;
        }
    }
    else if (((PVMFOMXDecPort*)iInPort)->iFormat ==  PVMF_MIME_MPEG4_AUDIO ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640 ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ASF_MPEG4_AUDIO ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AAC_SIZEHDR) // for testing
    {
        // get format specific info and send it as config data:
        DataIn->getFormatSpecificInfo(DataFrag);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::InitDecoder() VOL header (Size=%d)", DataFrag.getMemFragSize()));

        //get pointer to the data fragment
        initbuffer = (uint8 *) DataFrag.getMemFragPtr();
        initbufsize = (int32) DataFrag.getMemFragSize();

    }			// in some cases, initbufsize may be 0, and initbuf= NULL. Config is done after 1st frame of data
    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IF2 ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB ||
             ((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MP3)
    {
        initbuffer = NULL; // no special config header. Need to decode 1 frame
        initbufsize = 0;
    }

    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMA)
    {
        // in case of WMA, get config parameters from the port
        initbuffer = ((PVMFOMXDecPort*)iInPort)->getTrackConfig();
        initbufsize = (int32)((PVMFOMXDecPort*)iInPort)->getTrackConfigSize();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::InitDecoder() for WMA Decoder. Initialization data Size %d.", initbufsize));
    }


    if (initbufsize > 0)
    {


        if (!SendConfigBufferToOMXComponent(initbuffer, initbufsize))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::InitDecoder() Error in processing config buffer"));
            return false;
        }
    }




    return true;
}



/////////////////////////////////////////////////////////////////////////////
////////////////////// CALLBACK PROCESSING FOR EVENT HANDLER
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PVMFOMXAudioDecNode::EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
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
                    // flush can be sent as part of repositioning or as part of port reconfig

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_CommandFlush - completed on port %d", aData2));

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
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, dynamic reconfiguration needed on port %d", aData2, iSecondPortToReconfig));

                        iProcessingState = EPVMFOMXBaseDecNodeProcessingState_PortReconfig;
                        iPortIndexForDynamicReconfig = iSecondPortToReconfig;
                        iSecondPortReportedChange = false;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_CommandPortEnable - completed on port %d, resuming normal data flow", aData2));
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
                                    (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_CommandMarkBuffer - completed - no action taken"));

                    break;

                default:
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: Unsupported event"));
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
                                (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventError - Bitstream corrupt error"));
                // Errors from corrupt bitstream are reported as info events
                ReportInfoEvent(PVMFInfoProcessingFailure, NULL);

            }
            else
            {

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventError"));
                // for now, any error from the component will be reported as error
                ReportErrorEvent(PVMFErrProcessing, NULL, NULL);
                SetState(EPVMFNodeError);
            }
            break;



        }

        case OMX_EventBufferFlag:
        {
            // the component is reporting it encountered end of stream flag
            // we'll send EOS when we receive the last buffer marked with eos


            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventBufferFlag (EOS) flag returned from OMX component"));

            RunIfNotReady();
            break;
        }//end of case OMX_EventBufferFlag

        case OMX_EventMark:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventMark returned from OMX component - no action taken"));

            RunIfNotReady();
            break;
        }//end of case OMX_EventMark

        case OMX_EventPortSettingsChanged:
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned from OMX component"));

            // first check if dynamic reconfiguration is already in progress,
            // if so, wait until this is completed, and then initiate the 2nd reconfiguration
            if (iDynamicReconfigInProgress)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d, dynamic reconfig already in progress", aData1));

                iSecondPortToReconfig = aData1;
                iSecondPortReportedChange = true;

                // check the audio sampling rate and fs right away in case of output port
                // is this output port?
                if (iSecondPortToReconfig == iOutputPortIndex)
                {

                    OMX_ERRORTYPE Err;
                    // GET the output buffer params and sizes
                    OMX_AUDIO_PARAM_PCMMODETYPE Audio_Pcm_Param;
                    Audio_Pcm_Param.nPortIndex = iOutputPortIndex; // we're looking for output port params
                    CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);



                    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
                    if (Err != OMX_ErrorNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing() PortSettingsChanged -> Cannot get component output parameters"));

                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrResource);
                    }

                    iPCMSamplingRate = Audio_Pcm_Param.nSamplingRate; // can be set to 0 (if unknown)

                    if (iPCMSamplingRate == 0) // use default sampling rate (i.e. 48000)
                        iPCMSamplingRate = PVOMXAUDIODEC_DEFAULT_SAMPLINGRATE;

                    iNumberOfAudioChannels = Audio_Pcm_Param.nChannels;		// should be 1 or 2
                    if (iNumberOfAudioChannels != 1 && iNumberOfAudioChannels != 2)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing() PortSettingsChanged -> Output parameters num channels = %d", iNumberOfAudioChannels));

                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrResource);

                    }
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventPortSettingsChanged returned for port %d", aData1));

                iProcessingState = EPVMFOMXBaseDecNodeProcessingState_PortReconfig;
                iPortIndexForDynamicReconfig = aData1;
                // start "discarding" data right away, don't wait
                // check the audio sampling rate and fs right away in case of output port
                // is this output port?
                if (iPortIndexForDynamicReconfig == iOutputPortIndex)
                {

                    OMX_ERRORTYPE Err;
                    // GET the output buffer params and sizes
                    OMX_AUDIO_PARAM_PCMMODETYPE Audio_Pcm_Param;
                    Audio_Pcm_Param.nPortIndex = iOutputPortIndex; // we're looking for output port params

                    CONFIG_SIZE_AND_VERSION(Audio_Pcm_Param);


                    Err = OMX_GetParameter(iOMXDecoder, OMX_IndexParamAudioPcm, &Audio_Pcm_Param);
                    if (Err != OMX_ErrorNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing() PortSettingsChanged -> Cannot get component output parameters"));

                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrResource);

                    }

                    iPCMSamplingRate = Audio_Pcm_Param.nSamplingRate; // can be set to 0 (if unknown)

                    if (iPCMSamplingRate == 0) // use default sampling rate (i.e. 48000)
                        iPCMSamplingRate = PVOMXAUDIODEC_DEFAULT_SAMPLINGRATE;

                    iNumberOfAudioChannels = Audio_Pcm_Param.nChannels;		// should be 1 or 2
                    if (iNumberOfAudioChannels != 1 && iNumberOfAudioChannels != 2)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFOMXAudioDecNode::EventHandlerProcessing() PortSettingsChanged -> Output parameters num channels = %d", iNumberOfAudioChannels));

                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrResource);
                    }
                }
                iDynamicReconfigInProgress = true;
            }

            RunIfNotReady();
            break;
        }//end of case OMX_PortSettingsChanged

        case OMX_EventResourcesAcquired:        //not supported
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::EventHandlerProcessing: OMX_EventResourcesAcquired returned from OMX component - no action taken"));

            RunIfNotReady();

            break;
        }

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFOMXAudioDecNode::EventHandlerProcessing:  Unknown Event returned from OMX component - no action taken"));

            break;
        }

    }//end of switch (eEvent)



    return OMX_ErrorNone;
}




////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Put output buffer in outgoing queue //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFOMXAudioDecNode::QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen)
{

    bool status = true;
    PVMFSharedMediaDataPtr mediaDataOut;
    int32 leavecode = OsclErrNone;

    // NOTE: ASSUMPTION IS THAT OUTGOING QUEUE IS BIG ENOUGH TO QUEUE ALL THE OUTPUT BUFFERS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::QueueOutputFrame: In"));

    // First check if we can put outgoing msg. into the queue
    if (iOutPort->IsOutgoingQueueBusy())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFOMXAudioDecNode::QueueOutputFrame() OutgoingQueue is busy"));
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

        // Set sequence number
        mediaDataOut->setSeqNum(iSeqNum++);
        // set stream id
        mediaDataOut->setStreamID(iStreamID);


        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iDataPathLogger, PVLOGMSG_INFO, (0, ":PVMFOMXAudioDecNode::QueueOutputFrame(): - SeqNum=%d, TS=%d", iSeqNum, iOutTimeStamp));
        int fsiErrorCode = 0;

        // Check if Fsi configuration need to be sent
        if (sendFsi && !iCompactFSISettingSucceeded)
        {

            OsclRefCounterMemFrag FsiMemfrag;

            OSCL_TRY(fsiErrorCode, FsiMemfrag = iFsiFragmentAlloc.get(););

            OSCL_FIRST_CATCH_ANY(fsiErrorCode, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                 (0, "PVMFOMXAudioDecNode::RemoveOutputFrame() Failed to allocate memory for  FSI")));

            if (fsiErrorCode == 0)
            {

                channelSampleInfo* pcminfo = (channelSampleInfo*) FsiMemfrag.getMemFragPtr();
                if (pcminfo != NULL)
                {
                    OSCL_ASSERT(pcminfo != NULL);

                    pcminfo->samplingRate    = iPCMSamplingRate;
                    pcminfo->desiredChannels = iNumberOfAudioChannels;

                    mediaDataOut->setFormatSpecificInfo(FsiMemfrag);


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

                    OSCL_TRY(err, ((PVMFOMXDecPort*)iOutPort)->pvmiSetPortFormatSpecificInfoSync(FsiMemfrag, KvpKey););
                    if (err != OsclErrNone)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "PVMFOMXAudioDecNode::NegotiateComponentParameters - Problem to set FSI"));
                        SetState(EPVMFNodeError);
                        ReportErrorEvent(PVMFErrNoMemory);
                        return false; // this is going to make everyth go out of scope
                    }


                    alloc.deallocate((OsclAny*)(KvpKey));



                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMFOMXAudioDecNode::QueueOutputFrame - Problem allocating Output FSI"));
                    SetState(EPVMFNodeError);
                    ReportErrorEvent(PVMFErrNoMemory);
                    return false; // this is going to make everything go out of scope
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMFOMXAudioDecNode::QueueOutputFrame - Problem allocating Output FSI"));
                return false; // this is going to make everything go out of scope
            }

            // Reset the flag
            sendFsi = false;
        }

        if (fsiErrorCode == 0)
        {
            // Send frame to downstream node
            PVMFSharedMediaMsgPtr mediaMsgOut;
            convertToPVMFMediaMsg(mediaMsgOut, mediaDataOut);

            if (iOutPort && (iOutPort->QueueOutgoingMsg(mediaMsgOut) == PVMFSuccess))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::QueueOutputFrame(): Queued frame OK "));

            }
            else
            {
                // we should not get here because we always check for whether queue is busy or not
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::QueueOutputFrame(): Send frame failed"));
                return false;
            }
        }


    }//end of if (OsclErrNone == leavecode)
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFOMXAudioDecNode::QueueOutputFrame() call PVMFMediaData::createMediaData is failed"));
        return false;
    }

    return status;

}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXAudioDecNode::DoRequestPort(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoRequestPort() In"));
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
            OSCL_TRY(leavecode, iInPort = OSCL_NEW(PVMFOMXDecPort, ((int32)tag, this, (OMX_STRING)PVMF_OMX_AUDIO_DEC_INPUT_PORT_NAME)););
            if (leavecode || iInPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::DoRequestPort: Error - Input port instantiation failed"));
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
            OSCL_TRY(leavecode, iOutPort = OSCL_NEW(PVMFOMXDecPort, ((int32)tag, this, (OMX_STRING)PVMF_OMX_AUDIO_DEC_OUTPUT_PORT_NAME)));
            if (leavecode || iOutPort == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOMXAudioDecNode::DoRequestPort: Error - Output port instantiation failed"));
                CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
                return;
            }
            port = iOutPort;
            break;

        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXAudioDecNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
            return;
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

/////////////////////////////////////////////////////////////////////////////
void PVMFOMXAudioDecNode::DoReleasePort(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVMFPortInterface* temp;
    aCmd.PVMFOMXBaseDecNodeCommandBase::Parse(temp);

    PVMFOMXDecPort* port = (PVMFOMXDecPort*)temp;

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
PVMFStatus PVMFOMXAudioDecNode::DoGetNodeMetadataKey(PVMFOMXBaseDecNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoGetNodeMetadataKey() In"));

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

    if ((starting_index > (iAvailableMetadataKeys.size() - 1)) || max_entries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    int32 leavecode = OsclErrNone;
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
                leavecode = PushKVPKey(iAvailableMetadataKeys[lcv], keylistptr);
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
                    leavecode = PushKVPKey(iAvailableMetadataKeys[lcv], keylistptr);
                    if (OsclErrNone != leavecode)
                    {
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
PVMFStatus PVMFOMXAudioDecNode::DoGetNodeMetadataValue(PVMFOMXBaseDecNodeCommand& aCmd)
{
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

        if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY) == 0))
        {
            // PCM output channels
            if (iNumberOfAudioChannels > 0)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    KeyLen = oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY) + 1; // for "codec-info/audio/channels;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = OsclErrNone;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY, oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXAUDIODECMETADATA_SEMICOLON, oscl_strlen(PVOMXAUDIODECMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = iNumberOfAudioChannels;
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
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY) == 0) &&
                 (iPCMSamplingRate > 0))
        {
            // PCM output sampling rate
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY) + 1; // for "codec-info/audio/sample-rate;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                if (OsclErrNone == leavecode)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY, oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVOMXAUDIODECMETADATA_SEMICOLON, oscl_strlen(PVOMXAUDIODECMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    KeyVal.value.uint32_value = iPCMSamplingRate;
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
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY) == 0) &&
                 iInPort != NULL)
        {
            // Format
            if ((((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MPEG4_AUDIO) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AAC_SIZEHDR) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IF2) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MP3) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMA)

               )
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    KeyLen = oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY) + 1; // for "codec-info/audio/format;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*" and NULL terminator

                    uint32 valuelen = 0;
                    if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_LATM)) + 1; // Value string plus one for NULL terminator
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MPEG4_AUDIO)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_MPEG4_AUDIO)) + 1; // Value string plus one for NULL terminator
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_3640)) + 1; // Value string plus one for NULL terminator
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_ADIF)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IF2)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_AMR_IF2)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_AMR_IETF)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_AMR)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_AMRWB_IETF)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_AMRWB)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MP3)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_MP3)) + 1;
                    }
                    else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMA)
                    {
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_WMA)) + 1;
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
                        oscl_strncpy(KeyVal.key, PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY, oscl_strlen(PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVOMXAUDIODECMETADATA_SEMICOLON, oscl_strlen(PVOMXAUDIODECMETADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_LATM), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MPEG4_AUDIO)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_MPEG4_AUDIO), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_3640), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_ADIF), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IF2)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_AMR_IF2), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_AMR_IETF), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_AMR), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_AMRWB_IETF), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_AMRWB), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MP3)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_MP3), valuelen);
                        }
                        else if (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMA)
                        {
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_WMA), valuelen);
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
bool PVMFOMXAudioDecNode::ReleaseAllPorts()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXAudioDecNode::ReleaseAllPorts() In"));

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
void PVMFOMXAudioDecNode::DoQueryUuid(PVMFOMXBaseDecNodeCommand& aCmd)
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
            || (!exactmatch && *mimetype == PVMF_OMX_AUDIO_DEC_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {

        PVUuid uuid(PVMF_OMX_BASE_DEC_NODE_CUSTOM1_UUID);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

PVMFStatus PVMFOMXAudioDecNode::CreateLATMParser()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXAudioDecNode::CreateLATMParser() In"));

    // First clean up if necessary
    DeleteLATMParser();

    // Instantiate the LATM parser
    iLATMParser = OSCL_NEW(PV_LATM_Parser, ());
    if (!iLATMParser)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXAudioDecNode::CreateLATMParser() LATM parser instantiation failed"));
        return PVMFErrNoMemory;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXAudioDecNode::CreateLATMParser() Out"));
    return PVMFSuccess;
}

PVMFStatus PVMFOMXAudioDecNode::DeleteLATMParser()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXAudioDecNode::DeleteLATMParser() In"));

    // Delete LATM parser if there is one
    if (iLATMParser)
    {
        OSCL_DELETE(iLATMParser);
        iLATMParser = NULL;
    }

    if (iLATMConfigBuffer)
    {
        oscl_free(iLATMConfigBuffer);
        iLATMConfigBuffer = NULL;
        iLATMConfigBufferSize = 0;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFOMXAudioDecNode::DeleteLATMParser() Out"));
    return PVMFSuccess;
}






/////////////////////////////////////////////////////////////////////////////
uint32 PVMFOMXAudioDecNode::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXAudioDecNode::GetNumMetadataKeys() called"));

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
uint32 PVMFOMXAudioDecNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXAudioDecNode::GetNumMetadataValues() called"));

    uint32 numkeys = aKeyList.size();

    if (numkeys <= 0)
    {
        // Don't do anything
        return 0;
    }

    // Count the number of value entries for the provided key list
    uint32 numvalentries = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_CHANNELS_KEY) == 0))
        {
            // Channels
            if (iNumberOfAudioChannels > 0)
            {
                ++numvalentries;
            }
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_SAMPLERATE_KEY) == 0) && (iPCMSamplingRate > 0))
        {
            // Sample rate
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVOMXAUDIODECMETADATA_CODECINFO_AUDIO_FORMAT_KEY) == 0) &&
                 iInPort != NULL)
        {
            // Format
            if ((((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_LATM) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MPEG4_AUDIO) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_3640) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_ADIF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IF2) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMR) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_AMRWB) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_MP3) ||
                    (((PVMFOMXDecPort*)iInPort)->iFormat == PVMF_MIME_WMA)

               )

            {
                ++numvalentries;
            }
        }
    }

    return numvalentries;
}


// needed for WMA parameter verification
bool PVMFOMXAudioDecNode::VerifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);
    if (pv_mime_strcmp(aParameters->key, PVMF_BITRATE_VALUE_KEY) == 0)
    {
        if (((PVMFOMXDecPort*)iOutPort)->verifyConnectedPortParametersSync(PVMF_BITRATE_VALUE_KEY, &(aParameters->value.uint32_value)) != PVMFSuccess)
        {
            return false;
        }
        return true;
    }
    else if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) < 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXAudioDecNode::VerifyParametersSync() - Unsupported Key"));
        OSCL_ASSERT(false);
    }

    bool cap_exchange_status = false;

    pvAudioConfigParserInputs aInputs;
    OMXConfigParserInputs aInputParameters;
    AudioOMXConfigParserOutputs aOutputParameters;

    aInputs.inPtr = (uint8*)(aParameters->value.key_specific_value);
    aInputs.inBytes = (int32)aParameters->capacity;
    aInputs.iMimeType = PVMF_MIME_WMA;
    aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.wma";
    aInputParameters.inBytes = aInputs.inBytes;
    aInputParameters.inPtr = aInputs.inPtr;


    if (aInputs.inBytes == 0 || aInputs.inPtr == NULL)
    {
        return cap_exchange_status;
    }


    OMX_BOOL status = OMX_FALSE;
    OMX_U32 num_comps = 0, ii;
    OMX_STRING *CompOfRole;
    //	uint32 ii;
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
        // if no components support the role, nothing else to do
        return false;
    }

    if (OMX_FALSE != status)
    {
        cap_exchange_status = true;

        iPCMSamplingRate = aOutputParameters.SamplesPerSec;
        iNumberOfAudioChannels = aOutputParameters.Channels;

        if ((iNumberOfAudioChannels != 1 && iNumberOfAudioChannels != 2) ||
                (iPCMSamplingRate <= 0))
        {
            cap_exchange_status = false;
        }
    }

    return cap_exchange_status;

}


PVMFStatus PVMFOMXAudioDecNode::DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements)
{
    OSCL_UNUSED_ARG(aNumElements);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoCapConfigVerifyParameters() In\n"));

    pvAudioConfigParserInputs aInputs;
    OMXConfigParserInputs aInputParameters;
    AudioOMXConfigParserOutputs aOutputParameters;

    aInputs.inPtr = (uint8*)(aParameters->value.key_specific_value);
    aInputs.inBytes = (int32)aParameters->capacity;
    aInputs.iMimeType = iNodeConfig.iMimeType;
    aInputParameters.inBytes = aInputs.inBytes;
    aInputParameters.inPtr = aInputs.inPtr;

    if (aInputs.inBytes == 0 || aInputs.inPtr == NULL)
    {
        // in case of following formats - config codec data is expected to
        // be present in the query. If not, config parser cannot be called
        if (aInputs.iMimeType == PVMF_MIME_WMA ||
                aInputs.iMimeType == PVMF_MIME_MPEG4_AUDIO ||
                aInputs.iMimeType == PVMF_MIME_3640 ||
                aInputs.iMimeType == PVMF_MIME_LATM ||
                aInputs.iMimeType == PVMF_MIME_ADIF ||
                aInputs.iMimeType == PVMF_MIME_ASF_MPEG4_AUDIO ||
                aInputs.iMimeType == PVMF_MIME_AAC_SIZEHDR)
        {
            if (aInputs.iMimeType == PVMF_MIME_LATM)
            {
                return PVMFErrNotSupported;
            }
            else
            {
                // DV TO_DO: remove this
                OSCL_LEAVE(OsclErrNotSupported);
            }
        }
    }


    if (aInputs.iMimeType == PVMF_MIME_MPEG4_AUDIO ||
            aInputs.iMimeType == PVMF_MIME_3640 ||
            aInputs.iMimeType == PVMF_MIME_LATM ||
            aInputs.iMimeType == PVMF_MIME_ADIF ||
            aInputs.iMimeType == PVMF_MIME_ASF_MPEG4_AUDIO ||
            aInputs.iMimeType == PVMF_MIME_AAC_SIZEHDR)
    {
        aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.aac";
    }
    // AMR
    else if (aInputs.iMimeType == PVMF_MIME_AMR_IF2 ||
             aInputs.iMimeType == PVMF_MIME_AMR_IETF ||
             aInputs.iMimeType == PVMF_MIME_AMR)
    {
        aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.amrnb";
    }
    else if (aInputs.iMimeType == PVMF_MIME_AMRWB_IETF ||
             aInputs.iMimeType == PVMF_MIME_AMRWB)
    {
        aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.amrwb";
    }
    else if (aInputs.iMimeType == PVMF_MIME_MP3)
    {
        aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.mp3";
    }
    else if (aInputs.iMimeType ==  PVMF_MIME_WMA)
    {
        aInputParameters.cComponentRole = (OMX_STRING)"audio_decoder.wma";
    }
    else
    {
        // Illegal codec specified.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "%s::PVMFOMXAudioDecNode::DoCapConfigVerifyParameters() Input port format other then codec type", iName.Str()));
    }

    OMX_BOOL status = OMX_FALSE;
    OMX_U32 num_comps = 0, ii;
    OMX_STRING *CompOfRole;
    //	uint32 ii;
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
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::DoCapConfigVerifyParameters() No omx component supports this role PVMFErrNotSupported\n"));
        return PVMFErrNotSupported;
    }

    if (status == OMX_FALSE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::DoCapConfigVerifyParameters() ->OMXConfigParser() PVMFErrNotSupported\n"));
        return PVMFErrNotSupported;
    }

    if (aInputs.iMimeType == PVMF_MIME_WMA)
    {
        iNumberOfAudioChannels = aOutputParameters.Channels;
        iPCMSamplingRate = aOutputParameters.SamplesPerSec;
    }
    else if (aInputs.iMimeType == PVMF_MIME_MPEG4_AUDIO ||
             aInputs.iMimeType == PVMF_MIME_3640 ||
             aInputs.iMimeType == PVMF_MIME_LATM ||
             aInputs.iMimeType == PVMF_MIME_ADIF ||
             aInputs.iMimeType == PVMF_MIME_ASF_MPEG4_AUDIO ||
             aInputs.iMimeType == PVMF_MIME_AAC_SIZEHDR)
    {
        iNumberOfAudioChannels = aOutputParameters.Channels;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoCapConfigVerifyParameters() Out\n"));
    return PVMFSuccess;
}

void PVMFOMXAudioDecNode::DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoCapConfigSetParameters() In\n"));
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRetKVP);

    // find out if the audio dec format key is used for the query
    if (pv_mime_strcmp(aParameters->key, PVMF_AUDIO_DEC_FORMAT_TYPE_VALUE_KEY) == 0)
    {
        // set the mime type if audio format is being used
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::DoCapConfigSetParameters() set audio dec format type to %s\n", aParameters->value.pChar_value));

        iNodeConfig.iMimeType = aParameters->value.pChar_value;
    }

    else
    {
        // For now, ignore other queries
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFOMXAudioDecNode::DoCapConfigSetParameters() Key not used"));

        // indicate "error" by setting return KVP to the original
        aRetKVP = aParameters;
    }




    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFOMXAudioDecNode::DoCapConfigSetParameters() Out\n"));
}



