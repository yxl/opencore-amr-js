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
#ifndef PV_2WAY_SDKINFO_H_INCLUDED
#include "pv_2way_sdkinfo.h"
#endif

#ifndef PV_2WAY_ENGINE_H_INCLUDED
#include "pv_2way_engine.h"
#endif

#include "pv_2way_dec_data_channel_datapath.h"
#include "pv_2way_enc_data_channel_datapath.h"
#include "pv_2way_mux_datapath.h"

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
#include "pv_2way_preview_datapath.h"
#include "playfromfilenode.h"
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
#include "pv_2way_rec_datapath.h"

#include "pvmp4ffcn_factory.h"
#include "pvmp4ffcn_trackconfig.h"
#include "pvmp4ffcn_clipconfig.h"
#include "pvmf_composer_size_and_duration.h"
#endif

#if (defined(PV_RECORD_TO_FILE_SUPPORT) || defined(PV_PLAY_FROM_FILE_SUPPORT))
#include "pvmf_splitter_node.h"
#endif

#include "pvmf_videoparser_node.h"

#ifdef PV2WAY_USE_OMX
#include "OMX_Core.h"
#include "pvmf_omx_videodec_factory.h"
#include "pvmf_omx_enc_factory.h"
#include "pvmf_omx_audiodec_factory.h"
#include "pvmf_audio_encnode_extension.h"
#else
#include "pvmf_videodec_factory.h"
#include "pvmf_videoenc_node_factory.h"
#include "pvmfamrencnode_extension.h"
#include "pvmf_gsmamrdec_factory.h"
#include "pvmf_amrenc_node_factory.h"
#endif

#include "pvmf_video.h"
#include "pvmp4h263encextension.h"

#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif

#ifndef PV_DISABLE_VIDRECNODE
#include "pvvideoencmdfnode_factory.h"
#endif

#ifndef PV_DISABLE_DEVSOUNDNODES
#include "pvdevsound_node_base.h"
#endif

#include "pvlogger.h"

#include "oscl_dll.h"

#ifndef NO_2WAY_324
#include "tsc_h324m_config_interface.h"
#endif


#include "pvmf_nodes_sync_control.h"

#include "pv_2way_track_info_impl.h"

#include "pvmi_config_and_capability.h"


#ifdef MEM_TRACK
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

//Record defaults
#define DEFAULT_RECORDED_CALL_FILENAME _STRLIT("c:\\recorded_call.mp4")
#define DEFAULT_RECORDED_CALL_TIMESCALE 1000
#define DEFAULT_RECORDED_CALL_TYPE PVMP4FFCN_NO_TEMP_FILE_AUTHORING_MODE

#define NUM_MANDATORY_2WAY_AUDIO_CODECS 1
#define NUM_MANDATORY_2WAY_VIDEO_CODECS 2
#define PV_VIDEO_FRAME_RATE_NUMERATOR 10
#define PV_VIDEO_FRAME_RATE_DENOMINATOR 1

//Default skipMediaData params
const uint32 resume_timestamp = 0;
#define STREAMID 0
#define PBPOSITION_CONTINUOUS false
//Early and late margins for audio and video frames
#define SYNC_EARLY_MARGIN 100
#define SYNC_LATE_MARGIN 100

//Preferred codecs
#define VIDEO_CODEC_MPEG4 1
#define VIDEO_CODEC_H263 2
#define AUDIO_CODEC_GSM 3
#define AUDIO_CODEC_G723 4
#define PREFERRED_VIDEO_CODEC VIDEO_CODEC_MPEG4
#define PREFERRED_AUDIO_CODEC AUDIO_CODEC_GSM

const uint32 KSamplingRate  = 8000;
const uint32 KBitsPerSample = 16;
const uint32 KNumChannels   = 1;
const uint32 KNumPCMFrames  = 2; // 10

//TEMP -RH
#define PV2WAY_UNKNOWN_PORT -1
#define PV2WAY_IN_PORT 0
#define PV2WAY_OUT_PORT 1
#define PV2WAY_IO_PORT 3

#define INVALID_TRACK_ID 255

#define AUDIO_FIRST 1

#ifndef PV_DISABLE_VIDRECNODE
#define CREATE_VIDEO_ENC_NODE()  PVVideoEncMDFNodeFactory::Create(this,this,this)
#define DELETE_VIDEO_ENC_NODE(n) PVVideoEncMDFNodeFactory::Delete(n)
#else
#ifndef PV2WAY_USE_OMX
#define	CREATE_VIDEO_ENC_NODE()  PVMFVideoEncNodeFactory::CreateVideoEncNode()
#define DELETE_VIDEO_ENC_NODE(n) PVMFVideoEncNodeFactory::DeleteVideoEncNode(n)
#endif // PV2WAY_USE_OMX
#endif

#ifndef PV_DISABLE_DEVVIDEOPLAYNODE
#define CREATE_VIDEO_DEC_NODE()  PVDevVideoPlayNode::Create()
#define DELETE_VIDEO_DEC_NODE(n) OSCL_DELETE(n)
#else
#ifdef PV2WAY_USE_OMX
#define	CREATE_OMX_VIDEO_DEC_NODE()  PVMFOMXVideoDecNodeFactory::CreatePVMFOMXVideoDecNode()
#define DELETE_OMX_VIDEO_DEC_NODE(n) PVMFOMXVideoDecNodeFactory::DeletePVMFOMXVideoDecNode(n)
#endif // PV2WAY_USE_OMX
#define	CREATE_VIDEO_DEC_NODE()  PVMFVideoDecNodeFactory::CreatePVMFVideoDecNode()
#define DELETE_VIDEO_DEC_NODE(n) PVMFVideoDecNodeFactory::DeletePVMFVideoDecNode(n)
#endif

#ifdef PV2WAY_USE_OMX
#define	CREATE_OMX_ENC_NODE()  PVMFOMXEncNodeFactory::CreatePVMFOMXEncNode()
#define DELETE_OMX_ENC_NODE(n) PVMFOMXEncNodeFactory::DeletePVMFOMXEncNode(n);
#endif // PV2WAY_USE_OMX

#ifndef PV2WAY_USE_OMX
#define CREATE_AUDIO_ENC_NODE() PvmfAmrEncNodeFactory::Create()
#define DELETE_AUDIO_ENC_NODE(n) PvmfAmrEncNodeFactory::Delete(n)
#endif // PV2WAY_USE_OMX


#ifdef PV2WAY_USE_OMX
#define CREATE_OMX_AUDIO_DEC_NODE() PVMFOMXAudioDecNodeFactory::CreatePVMFOMXAudioDecNode()
#define DELETE_OMX_AUDIO_DEC_NODE(n) PVMFOMXAudioDecNodeFactory::DeletePVMFOMXAudioDecNode(n)
#else
#define CREATE_AUDIO_DEC_NODE() PVMFGSMAMRDecNodeFactory::CreatePVMFGSMAMRDecNode()
#define DELETE_AUDIO_DEC_NODE(n) PVMFGSMAMRDecNodeFactory::DeletePVMFGSMAMRDecNode(n)
#endif // PV2WAY_USE_OMX



#define FILL_FORMAT_INFO(format_type, format_info)\
GetSampleSize(format_type,&format_info.min_sample_size,&format_info.max_sample_size);\
format_info.format = format_type;

OSCL_EXPORT_REF CPV324m2Way *CPV324m2Way::NewL(PVMFNodeInterface* aTsc,
        TPVTerminalType aTerminalType,
        PVCommandStatusObserver* aCmdStatusObserver,
        PVInformationalEventObserver *aInfoEventObserver,
        PVErrorEventObserver *aErrorEventObserver)
{
    CPV324m2Way* aRet = OSCL_NEW(CPV324m2Way, ());
    if (aRet)
    {
        int32 error = Construct(aRet, aTsc, aTerminalType, aCmdStatusObserver,
                                aInfoEventObserver, aErrorEventObserver);
        if (error)
        {
            OSCL_DELETE(aRet);
            aRet = NULL;
            OSCL_LEAVE(error);
        }
    }
    else
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    return aRet;
}

int32 CPV324m2Way::Construct(CPV324m2Way* aRet,
                             PVMFNodeInterface* aTsc,
                             TPVTerminalType aTerminalType,
                             PVCommandStatusObserver* aCmdStatusObserver,
                             PVInformationalEventObserver *aInfoEventObserver,
                             PVErrorEventObserver *aErrorEventObserver)
{
    int32 error = 0;
    OSCL_TRY(error, aRet->ConstructL(aTsc,
                                     aTerminalType,
                                     aCmdStatusObserver,
                                     aInfoEventObserver,
                                     aErrorEventObserver));
    return error;
}

OSCL_EXPORT_REF void CPV324m2Way::Delete(CPV324m2Way *aTerminal)
{
    OSCL_DELETE(aTerminal);
}

CPV324m2Way::CPV324m2Way() :
        OsclActiveObject(OsclActiveObject::EPriorityNominal, "PV2WayEngine"),
        iState(EIdle),
        iLastState(EIdle),
        iCmdStatusObserver(NULL),
        iInfoEventObserver(NULL),
        iErrorEventObserver(NULL),
        iCommandId(0),
        iVideoEncDatapath(NULL),
        iVideoDecDatapath(NULL),
        iAudioEncDatapath(NULL),
        iAudioDecDatapath(NULL),
        iIsStackConnected(false),
        iMuxDatapath(NULL),
        iInitInfo(NULL),
        iConnectInfo(NULL),
        iDisconnectInfo(NULL),
        iResetInfo(NULL),
        iCancelInfo(NULL),
        iSessionParamsInfo(NULL),
        iLogger(NULL),
        iMinIFrameRequestInterval(DEFAULT_MIN_IFRAME_REQ_INT),
        iIFrameReqTimer("IFrameReqTimer"),
        iEndSessionTimer(NULL),
        iRemoteDisconnectTimer(NULL),
        isIFrameReqTimerActive(false),
#ifndef NO_2WAY_324
        iIncomingAudioTrackTag(INVALID_TRACK_ID),
        iIncomingVideoTrackTag(INVALID_TRACK_ID),
        iOutgoingAudioTrackTag(INVALID_TRACK_ID),
        iOutgoingVideoTrackTag(INVALID_TRACK_ID),
#endif
        iVideoEncQueryIntCmdId(-1),
#if defined(PV_RECORD_TO_FILE_SUPPORT)
        iRecordFileState(File2WayIdle),
        iInitRecFileInfo(NULL),
        iResetRecFileInfo(NULL),
        iFFComposerNode(NULL),
        iAudioRecDatapath(NULL),
        iVideoRecDatapath(NULL),
        iRecFileSizeNotificationInterval(0),
        iRecFileSizeNotificationTimer("iRecFileSizeNotificationTimer"),
        isRecFileSizeNotificationTimerActive(false),
#endif
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
        iAudioPreviewDatapath(NULL),
        iVideoPreviewDatapath(NULL),
        iPlayFileState(File2WayIdle),
        iUsePlayFileAsSource(false),
        iInitPlayFileInfo(NULL),
        iResetPlayFileInfo(NULL),
        iPlayFileCmdInfo(NULL),
        iPlayFromFileNode(NULL),
#endif
        iTSCInterface(NULL),
        iTSC324mInterface(NULL),
        iPendingTscReset(-1),
        iPendingAudioEncReset(-1),
        iPendingVideoEncReset(-1),
        iAudioDatapathLatency(0),
        iVideoDatapathLatency(0)
{
    iLogger = PVLogger::GetLoggerObject("2wayEngine");
    iSyncControlPVUuid = PvmfNodesSyncControlUuid;
    iVideoEncPVUuid = PVMp4H263EncExtensionUUID;
    iCapConfigPVUuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;

#ifdef PV2WAY_USE_OMX
    iAudioEncPVUuid = PVAudioEncExtensionUUID;
#else
    iAudioEncPVUuid = PVAMREncExtensionUUID;
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    iFFClipConfigPVUuid = KPVMp4FFCNClipConfigUuid;
    iFFTrackConfigPVUuid = KPVMp4FFCNTrackConfigUuid;
    iFFSizeAndDurationPVUuid = PvmfComposerSizeAndDurationUuid;
#endif
    iAddDataSourceVideoCmd = NULL;
#ifdef PV2WAY_USE_OMX
    OMX_Init();
#endif // PV2WAY_USE_OMX

    //creating timers
    iEndSessionTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>, (END_SESSION_TIMER, END_SESSION_TIMER_FREQUENCY));
    iRemoteDisconnectTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>, (REMOTE_DISCONNECT_TIMER, REMOTE_DISCONNECT_TIMER_FREQUENCY));
}

CPV324m2Way::~CPV324m2Way()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::~CPV324m2Way\n"));

    Oscl_Map<PVMFFormatType, CPvtMediaCapability*, OsclMemAllocator, pvmf_format_type_key_compare_class>::iterator it = iStackSupportedFormats.begin();
    while (it != iStackSupportedFormats.end())
    {
        CPvtMediaCapability* media_capability = (*it++).second;
        OSCL_DELETE(media_capability);
    }
    iStackSupportedFormats.clear();

    Cancel();
    iIncomingChannelParams.clear();
    iOutgoingChannelParams.clear();

    iIncomingAudioCodecs.clear();
    iOutgoingAudioCodecs.clear();
    iIncomingVideoCodecs.clear();
    iOutgoingVideoCodecs.clear();
    iFormatCapability.clear();
    iClock.Stop();
    iSinkNodeList.clear();
    ClearVideoEncNode();

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    if (iVideoPreviewDatapath)
    {
        OSCL_DELETE(iVideoPreviewDatapath);
        iVideoPreviewDatapath = NULL;
    }

    if (iAudioPreviewDatapath)
    {
        OSCL_DELETE(iAudioPreviewDatapath);
        iAudioPreviewDatapath = NULL;
    }
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    if (iVideoRecDatapath)
    {
        OSCL_DELETE(iVideoRecDatapath);
        iVideoRecDatapath = NULL;
    }

    if (iAudioRecDatapath)
    {
        OSCL_DELETE(iAudioRecDatapath);
        iAudioRecDatapath = NULL;
    }
#endif

    if (iVideoEncDatapath)
    {
        OSCL_DELETE(iVideoEncDatapath);
        iVideoEncDatapath = NULL;
    }

    if (iVideoDecDatapath)
    {
        OSCL_DELETE(iVideoDecDatapath);
        iVideoDecDatapath = NULL;
    }

    if (iAudioEncDatapath)
    {
        OSCL_DELETE(iAudioEncDatapath);
        iAudioEncDatapath = NULL;
    }

    if (iAudioDecDatapath)
    {
        OSCL_DELETE(iAudioDecDatapath);
        iAudioDecDatapath = NULL;
    }

    if (iMuxDatapath)
    {
        OSCL_DELETE(iMuxDatapath);
        iMuxDatapath = NULL;
    }

    PVMFNodeInterface * nodeIFace = (PVMFNodeInterface *)iTscNode;
    if (nodeIFace)
    {
        OSCL_DELETE(nodeIFace);
        iTscNode.Clear();
    }

#ifdef PV2WAY_USE_OMX
    OMX_Deinit();
#endif

    if (iEndSessionTimer)
    {
        iEndSessionTimer->Clear();
        OSCL_DELETE(iEndSessionTimer);
        iEndSessionTimer = NULL;
    }

    if (iRemoteDisconnectTimer)
    {
        iRemoteDisconnectTimer->Clear();
        OSCL_DELETE(iRemoteDisconnectTimer);
        iRemoteDisconnectTimer = NULL;
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::~CPV324m2Way - done\n"));
}

void CPV324m2Way::ClearVideoEncNode()
{
    PVMFNodeInterface * nodeIFace = (PVMFNodeInterface *)iVideoEncNode;
    if (nodeIFace)
    {
        nodeIFace->ThreadLogoff();
        if (iVideoEncNodeInterface.iInterface) iVideoEncNodeInterface.iInterface->removeRef();
#ifndef PV_DISABLE_VIDRECNODE
        PVVideoEncMDFNodeFactory::Delete(nodeIFace);
#else

#ifdef PV2WAY_USE_OMX
        DELETE_OMX_ENC_NODE(nodeIFace);
#else
        DELETE_VIDEO_ENC_NODE(nodeIFace);
#endif // PV2WAY_USE_OMX
#endif // PV_DISABLE_VIDRECNODE
        iVideoEncNode.Clear() ;
        iVideoEncNodeInterface.Reset();
    }
}

PVCommandId CPV324m2Way::GetSDKInfo(PVSDKInfo &aSDKInfo, OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::GetSDKInfo\n"));

    FillSDKInfo(aSDKInfo);

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_GET_SDK_INFO;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;
    Dispatch(cmd);
    return iCommandId++;
}

PVCommandId CPV324m2Way::GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSDKModuleInfo);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::GetSDKModuleInfo\n"));

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_GET_SDK_MODULE_INFO;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;
    Dispatch(cmd);
    return iCommandId++;
}

void CPV324m2Way::PreInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::PreInit\n"));

    PVMFNodeSessionInfo sessionInfo;
    bool allocSuccessful = true;

    switch (iState)
    {
        case EIdle:

            if (iTerminalType == PV_324M)
            {
#ifndef NO_2WAY_324
                iTscNode = TPV2WayNode(new TSC_324m(PV_LOOPBACK_MUX));
                iTSC324mInterface = (TSC_324m *)iTscNode.iNode;
                iTSCInterface = (TSC *)iTSC324mInterface;
                // Create the list of stack supported formats
                GetStackSupportedFormats();
#endif
            }

            if (((PVMFNodeInterface *)iTscNode) == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::PreInit unable to allocate tsc node\n"));
                allocSuccessful = false;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::PreInit created TSC Node(%x)", (PVMFNodeInterface *)iTscNode));

            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::Init Error - invalid state\n"));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }
}

PVCommandId CPV324m2Way::Init(PV2WayInitInfo& aInitInfo,
                              OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::InitL\n"));

    PVMFNodeSessionInfo sessionInfo;

    bool allocSuccessful = true;

    switch (iState)
    {
        case EIdle:
        {
            if (iInitInfo)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "CPV324m2Way::PreInit cmd already sent out"));
                OSCL_LEAVE(PVMFErrBusy);
            }

            ((TSC_324m*)(iTscNode.iNode))->SetTscObserver(this);
            InitiateSession(iTscNode);

            ((TSC_324m*)(iTscNode.iNode))->SetMultiplexingDelayMs(0);
            ((TSC_324m*)(iTscNode.iNode))->SetClock(&iClock);

            SetPreferredCodecs(aInitInfo);

#if defined(PV_RECORD_TO_FILE_SUPPORT)
            OSCL_TRY(error, iVideoDecSplitterNode =
                         TPV2WayNode(PVMFSplitterNode::Create()););
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "CPV324m2Way::PreInit unable to allocate video splitter node\n"));
                                 allocSuccessful = false;);

            OSCL_TRY(error, iFFComposerNode =
                         TPV2WayNode(PVMp4FFComposerNodeFactory::CreateMp4FFComposer(this, this, this));;
                     iFFComposerNode->SetClock(&iClock););
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "CPV324m2Way::PreInit unable to allocate ff composer node\n"));
                                 allocSuccessful = false;);
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
            OSCL_TRY(error, iAudioSrcSplitterNode =
                         TPV2WayNode(PVMFSplitterNode::Create()););
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "CPV324m2Way::PreInit unable to allocate audio src splitter node\n"));
                                 allocSuccessful = false;);

            OSCL_TRY(error, iVideoSrcSplitterNode =
                         TPV2WayNode(PVMFSplitterNode::Create()););
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "CPV324m2Way::PreInit unable to allocate video src splitter node\n"));
                                 allocSuccessful = false;);


            OSCL_TRY(error, iPlayFromFileNode =
                         TPV2WayNode(PlayFromFileNode::NewL());
                     iPlayFromFileNode->SetClock(&iClock););
            if (iPlayFromFileNode == NULL) error = PVMFErrNoMemory;
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                 (0, "CPV324m2Way::PreInit unable to allocate playfromfile node\n"));
                                 allocSuccessful = false;);
#endif

            if (!allocSuccessful)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::Init allocation failed\n"));
            }
            else
            {


#if defined(PV_RECORD_TO_FILE_SUPPORT)
                InitiateSession(iVideoDecSplitterNode);
                InitiateSession(iFFComposerNode);
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
                InitiateSession(iAudioSrcSplitterNode);
                InitiateSession(iVideoSrcSplitterNode);
                InitiateSession(iPlayFromFileNode);
#endif
                //Set incoming channel capabilities.
                // TBD: Incoming capabilities need to be queried from the registry and passed to the stack
                H324ChannelParameters inAudioChannelParams(INCOMING, PVMF_MIME_AMR_IF2, MAX_AUDIO_BITRATE);
                H324ChannelParameters inVideoChannelParams(INCOMING, PVMF_MIME_H2632000, MAX_VIDEO_BITRATE);
                H324ChannelParameters inDtmfParams(INCOMING, PVMF_MIME_USERINPUT_BASIC_STRING, 0);

                ConvertMapToVector(iIncomingAudioCodecs, iFormatCapability);
                inAudioChannelParams.SetCodecs(iFormatCapability);

                ConvertMapToVector(iIncomingVideoCodecs, iFormatCapability);
                inVideoChannelParams.SetCodecs(iFormatCapability);
                inDtmfParams.SetCodecs(iIncomingUserInputFormats);
                iIncomingChannelParams.push_back(inAudioChannelParams);
                iIncomingChannelParams.push_back(inVideoChannelParams);
                iIncomingChannelParams.push_back(inDtmfParams);

                //Set outgoing channel capabilities.
                H324ChannelParameters outAudioChannelParams(OUTGOING,
                        PVMF_MIME_AMR_IF2, MAX_AUDIO_BITRATE);
                ConvertMapToVector(iOutgoingAudioCodecs, iFormatCapability);
                outAudioChannelParams.SetCodecs(iFormatCapability);
                iOutgoingChannelParams.push_back(outAudioChannelParams);

                H324ChannelParameters outVideoChannelParams(OUTGOING,
                        PVMF_MIME_H2632000, MAX_VIDEO_BITRATE);

                ConvertMapToVector(iOutgoingVideoCodecs, iFormatCapability);
                outVideoChannelParams.SetCodecs(iFormatCapability);
                iOutgoingChannelParams.push_back(outVideoChannelParams);
            }

            iInitInfo = GetCmdInfoL();
            iInitInfo->type = PVT_COMMAND_INIT;
            iInitInfo->contextData = aContextData;
            iInitInfo->id = iCommandId;

            SetState(EInitializing);

            CheckState();
#ifdef MEM_TRACK
            printf("\nMemStats at Engine Init\n");
            MemStats();
#endif
            break;
        }

        case ESetup:
            iInitInfo = GetCmdInfoL();
            iInitInfo->type = PVT_COMMAND_INIT;
            iInitInfo->id = iCommandId;
            iInitInfo->contextData = aContextData;
            iInitInfo->status = PVMFSuccess;
            Dispatch(iInitInfo);
            iInitInfo = NULL;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::Init Error - invalid state\n"));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    return iCommandId++;
}


PVCommandId CPV324m2Way::Reset(OsclAny* aContextData)
{
    uint32 ii = 0;
    //checking if any sources or sinks still added.
    for (ii = 0; ii < iSinkNodes.size(); ii++)
    {
        if (iSinkNodes[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::ResetL SinkNodes not removed before Reset"));
            OSCL_LEAVE(PVMFFailure);
        }
    }

    for (ii = 0; ii < iSourceNodes.size(); ii++)
    {
        if (iSourceNodes[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::ResetL SourceNodes not removed before Reset"));
            OSCL_LEAVE(PVMFFailure);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::ResetL %d\n", iState));

    if (iResetInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ResetL cmd already sent out\n"));
        OSCL_LEAVE(PVMFErrBusy);
    }

    switch (iState)
    {
        case EIdle:
            iResetInfo = GetCmdInfoL();
            iResetInfo->type = PVT_COMMAND_RESET;
            iResetInfo->id = iCommandId;
            iResetInfo->contextData = aContextData;
            iResetInfo->status = PVMFSuccess;
            Dispatch(iResetInfo);
            iResetInfo = NULL;
            break;

        case EInitializing:
            //Notify application that init command has been cancelled.
            iInitInfo->status = PVMFErrCancelled;
            Dispatch(iInitInfo);
            iInitInfo = NULL;
            //Fall through to next case.

        case ESetup:
            iResetInfo = GetCmdInfoL();
            iResetInfo->type = PVT_COMMAND_RESET;
            iResetInfo->contextData = aContextData;
            iResetInfo->id = iCommandId;

            InitiateReset();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::ResetL - invalid state %d\n", iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }
#ifdef MEM_TRACK
    printf("\nMemStats After Engine Reset\n");
    MemStats();
#endif
    return iCommandId++;
}

PVCommandId CPV324m2Way::AddDataSource(PVTrackId aChannelId,
                                       PVMFNodeInterface& aDataSource,
                                       OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::AddDataSourceL aChannelId=%d, (%x, %x, %x)",
                     aChannelId, &aDataSource, 0, aContextData));
    if (!((TSC_324m *)(PVMFNodeInterface *)iTscNode.iNode)->IsEstablishedLogicalChannel(OUTGOING,
            aChannelId))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::AddDataSourceL Not an established logical channel in the stack"));
        OSCL_LEAVE(PVMFErrArgument);
    }
    TPV2WayNode* srcNode;
    PVMFNodeInterface *node = &aDataSource;
    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    switch (iState)
    {
        case EIdle:
        case EInitializing:
        case EResetting:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::AddDataSourceL - invalid state(%d)", iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
        default:
            //State check okay.
            break;
    }

    //As of v4, we'll need to initialize the node first before
    //querying its capabilities

    // Add the Data Source to the list of source nodes.
    srcNode = OSCL_NEW(TPV2WayNode, (node));
    InitiateSession(*srcNode);
    iSourceNodes.push_back(srcNode);

    cmd = GetCmdInfoL();
    cmd->type = PVT_COMMAND_ADD_DATA_SOURCE;
    cmd->status = PVMFSuccess;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->iPvtCmdData = aChannelId;

    SendNodeCmdL(PV2WAY_NODE_CMD_INIT, srcNode, this, cmd);
    return iCommandId++;
}

void CPV324m2Way::DoAddDataSource(TPV2WayNode& aNode,
                                  const PVMFCmdResp& aResponse)
{
    TPV2WayNode* srcNode = &aNode;
    PVMFNodeInterface *node = srcNode->iNode;
    PVMFNodeCapability capability;
    CPVDatapathNode datapathnode;
    CPV2WayNodeContextData *data = (CPV2WayNodeContextData *) aResponse.GetContext();
    TPV2WayCmdInfo *cmd = (TPV2WayCmdInfo *)data->iContextData;

    cmd->status = aResponse.GetCmdStatus();

    if (node->GetCapability(capability) != PVMFSuccess || !capability.iOutputFormatCapability.size())
    {
        OSCL_DELETE(srcNode);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::AddDataSourceL - unable to get capability"));
        OSCL_LEAVE(PVMFFailure);
    }

    CPV2WayEncDataChannelDatapath* datapath = NULL;
    PVMFFormatType media_type = capability.iOutputFormatCapability[0];
    if (media_type.isAudio())
    {
        datapath = iAudioEncDatapath;
    }
    else if (media_type.isVideo())
    {
        datapath = iVideoEncDatapath;
    }
    else
    {
        OSCL_LEAVE(PVMFErrArgument);
    }

    bool formatSupported = false;
    for (uint i = 0; i < capability.iOutputFormatCapability.size(); i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "CPV324m2Way::AddDataSourceL - format %s\n", (capability.iOutputFormatCapability[i]).getMIMEStrPtr()));
        if (datapath->GetSourceSinkFormat() == capability.iOutputFormatCapability[i])
        {
            formatSupported = true;
            break;
        }
    }
    if (!formatSupported)
    {
        OSCL_LEAVE(PVMFErrNotSupported);
    }

    if (datapath->GetSourceSinkFormat() == PVMF_MIME_YUV420)
    {
        // video media type
        if (datapath->GetState() == EClosed)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::AddDataSource - creating video datapath, channel id =%d\n",
                             cmd->iPvtCmdData));
            datapath->SetChannelId(cmd->iPvtCmdData);

            //Add source node to datapath
            datapathnode.iNode = *srcNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iLoggoffOnReset = true;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
            datapath->AddNode(datapathnode);

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
            //Add video src splitter node to datapath
            datapathnode.iNode = iVideoSrcSplitterNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iInputPort.iPortSetType = EUserDefined;
            datapathnode.iInputPort.iFormatType = PVMF_MIME_YUV420;
            datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iCanCancelPort = false;
            datapathnode.iOutputPort.iPortSetType = EUserDefined;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_YUV420;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
            datapath->AddNode(datapathnode);
#endif
            //Add video enc node to datapath
            datapathnode.iNode = iVideoEncNode;
            datapathnode.iConfigure = this;
            datapathnode.iConfigTime = EConfigBeforeInit;
            datapathnode.iCanNodePause = true;
            datapathnode.iLoggoffOnReset = false;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iInputPort.iPortSetType = EUserDefined;
            datapathnode.iInputPort.iFormatType = PVMF_MIME_YUV420;
            datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
            datapath->AddNode(datapathnode);

            //Add tsc node to datapath
            datapathnode.iNode = iTscNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iLoggoffOnReset = false;
            datapathnode.iIgnoreNodeState = true;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeStarted;
            datapathnode.iInputPort.iCanCancelPort = true;
            datapathnode.iInputPort.iPortSetType = EAppDefined;
            datapathnode.iInputPort.iFormatType = datapath->GetFormat();
            datapathnode.iInputPort.iPortTag = cmd->iPvtCmdData;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
            datapath->AddNode(datapathnode);

            // Check if FSI exists and Extension Interface is queried
            uint32 fsi_len = 0;
            if (datapath->GetFormatSpecificInfo(&fsi_len) &&
                    iVideoEncNodeInterface.iState == PV2WayNodeInterface::NoInterface)
            {
                iAddDataSourceVideoCmd = cmd;
            }
            else
            {
                datapath->SetCmd(cmd);
            }
        }
        else
        {
            OSCL_LEAVE(PVMFErrInvalidState);
        }
    }

    else if ((datapath->GetSourceSinkFormat() == PVMF_MIME_H2632000) || (datapath->GetSourceSinkFormat() == PVMF_MIME_H2631998) || (datapath->GetSourceSinkFormat() == PVMF_MIME_M4V))
    {
        // video media type
        if (datapath->GetState() == EClosed)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::AddDataSource - creating video datapath, channel id=%d",
                             cmd->iPvtCmdData));
            datapath->SetChannelId(cmd->iPvtCmdData);
            //Add source node to datapath
            datapathnode.iNode = *srcNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iLoggoffOnReset = true;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;

            datapath->AddNode(datapathnode);
            //Add tsc node to datapath
            datapathnode.iNode = iTscNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iLoggoffOnReset = false;
            datapathnode.iIgnoreNodeState = true;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeStarted;
            datapathnode.iInputPort.iCanCancelPort = true;
            datapathnode.iInputPort.iPortSetType = EAppDefined;
            datapathnode.iInputPort.iFormatType = datapath->GetFormat();
            datapathnode.iInputPort.iPortTag = cmd->iPvtCmdData;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
            datapath->AddNode(datapathnode);

            datapath->SetCmd(cmd);
        }
        else
        {
            OSCL_LEAVE(PVMFErrInvalidState);
        }
    }

    else if ((datapath->GetSourceSinkFormat() == PVMF_MIME_AMR_IF2) || (datapath->GetSourceSinkFormat() == PVMF_MIME_AMR_IETF) || (datapath->GetSourceSinkFormat() == PVMF_MIME_PCM16))
    {
        if (datapath->GetState() == EClosed)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::AddDataSourceL - creating audio datapath\n"));
            datapath->SetChannelId(cmd->iPvtCmdData);

            //Add source node to datapath
            datapathnode.iNode = *srcNode;

#ifndef PV_DISABLE_DEVSOUNDNODES
            datapathnode.iConfigure = this;
            datapathnode.iConfigTime = EConfigBeforeInit;
#else
            datapathnode.iConfigure = NULL;
#endif
            datapathnode.iCanNodePause = true;
            datapathnode.iLoggoffOnReset = true;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
            datapath->AddNode(datapathnode);

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
            //Add audio src splitter node to datapath
            datapathnode.iNode = iAudioSrcSplitterNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iInputPort.iPortSetType = EUseOtherNodePortFormat;
            datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iOutputPort.iCanCancelPort = false;
            datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
            datapath->AddNode(datapathnode);
#endif

            if (datapath->GetSourceSinkFormat() == PVMF_MIME_PCM16)
            {
                //Add audio enc node to datapath
                datapathnode.iNode = iAudioEncNode;
                datapathnode.iConfigure = this;
                datapathnode.iConfigTime = EConfigBeforeInit;
                datapathnode.iCanNodePause = true;
                datapathnode.iLoggoffOnReset = false;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EUserDefined;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_PCM16;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iOutputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                datapath->AddNode(datapathnode);
            }

            //Add tsc node to datapath
            datapathnode.iNode = iTscNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iLoggoffOnReset = false;
            datapathnode.iIgnoreNodeState = true;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeStarted;
            datapathnode.iInputPort.iCanCancelPort = true;
            datapathnode.iInputPort.iPortSetType = EAppDefined;
            datapathnode.iInputPort.iPortTag = cmd->iPvtCmdData;
            datapathnode.iInputPort.iFormatType = datapath->GetFormat();
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
            datapath->AddNode(datapathnode);

            datapath->SetCmd(cmd);
        }
        else
        {
            OSCL_LEAVE(PVMFErrInvalidState);
        }
    }

    return;
}

PVCommandId CPV324m2Way::DoRemoveDataSourceSink(PVMFNodeInterface& aEndPt,
        OsclAny* aContextData)
{
    CPV2WayDataChannelDatapath *datapath = NULL;
    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    if ((iVideoEncDatapath) && iVideoEncDatapath->IsNodeInDatapath(&aEndPt))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::RemoveDataSource remove video source, state %d\n",
                         iVideoEncDatapath->GetState()));
        datapath = iVideoEncDatapath;
        cmd->type = PVT_COMMAND_REMOVE_DATA_SOURCE;
    }
    else if ((iAudioEncDatapath) && iAudioEncDatapath->IsNodeInDatapath(&aEndPt))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::RemoveDataSource remove audio source, state %d\n",
                         iAudioEncDatapath->GetState()));
        datapath = iAudioEncDatapath;
        cmd->type = PVT_COMMAND_REMOVE_DATA_SOURCE;
    }
    else if ((iVideoDecDatapath) && iVideoDecDatapath->IsNodeInDatapath(&aEndPt))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::RemoveDataSink remove video sink, state %d\n",
                         iVideoDecDatapath->GetState()));
        datapath = iVideoDecDatapath;
        cmd->type = PVT_COMMAND_REMOVE_DATA_SINK;
    }
    else if ((iAudioDecDatapath) && iAudioDecDatapath->IsNodeInDatapath(&aEndPt))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::RemoveDataSink remove audio sink, state %d\n",
                         iAudioDecDatapath->GetState()));
        datapath = iAudioDecDatapath;
        cmd->type = PVT_COMMAND_REMOVE_DATA_SINK;
    }
    else
    {
        // Just remove the node from sink and source nodes list if still in the list

        TPV2WayNode* node = 0;

        node = RemoveTPV2WayNode(iSinkNodes, &aEndPt);

        if (!node)
        {
            // Not there in sink node list . Check in source nodes
            node = RemoveTPV2WayNode(iSourceNodes, &aEndPt);
        }

        if (node)
        {
            //Successfully found and removed the node from  sink or source nodes ,so delete it.
            OSCL_DELETE(node);
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::RemoveDataSinkSource unknown sink!\n"));
        OSCL_LEAVE(PVMFErrArgument);
    }

    switch (datapath->GetState())
    {
        case EClosing:
            //Close command already in progress
            if (datapath->GetCmdInfo())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "CPV324m2Way::RemoveDataSourceL cmd already sent out"));
                OSCL_LEAVE(PVMFErrBusy);
            }
            //Already closing because of error or remote close
            else
            {
                cmd->id = iCommandId;
                cmd->contextData = aContextData;
                datapath->SetCmd(cmd);
            }
            break;

        case EOpened:
        case EOpening:
        case EPaused:
        case EPausing:
        case EUnpausing:
        {
            cmd->id = iCommandId;
            cmd->contextData = aContextData;
            datapath->SetCmd(cmd);
        }
        break;
        case EClosed:
            // Remove the node if exists in sink or source even data path is closed
            break;

        default:

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::RemoveDataSourceL - invalid path state\n"));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    TPV2WayNode* node = 0;

    if (cmd->type == PVT_COMMAND_REMOVE_DATA_SINK)
    {
        node = RemoveTPV2WayNode(iSinkNodes, &aEndPt);
    }
    else if (cmd->type == PVT_COMMAND_REMOVE_DATA_SOURCE)
    {
        node = RemoveTPV2WayNode(iSourceNodes, &aEndPt);
    }
    OSCL_DELETE(node);
    return iCommandId++;
}

PVCommandId CPV324m2Way::RemoveDataSource(PVMFNodeInterface& aDataSource,
        OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveDataSourceL(%x, %x, %x)",
                     &aDataSource, 0, aContextData));

    switch (iState)
    {
        case EIdle:
        case EInitializing:
        case EResetting:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::RemoveDataSourceL - invalid state(%d)",
                             iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;

        default:
            //State check okay.
            break;
    }
    return DoRemoveDataSourceSink(aDataSource, aContextData);
}


PVCommandId CPV324m2Way::AddDataSink(PVTrackId aChannelId,
                                     PVMFNodeInterface& aDataSink,
                                     OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::AddDataSinkL(%x, %d, %x)", &aDataSink, 0,
                     aContextData));
    TPV2WayNode* sinkNode;
    CPVDatapathNode datapathnode;
    TPV2WayCmdInfo *cmd = 0;

    switch (iState)
    {
        case EIdle:
        case EInitializing:
        case EResetting:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::AddDataSinkL - invalid state(%d)",
                             iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;

        default:
            //State check okay.
            break;
    }

    //As of v4, we'll need to initialize the node first before
    //querying its capabilities.

    sinkNode = OSCL_NEW(TPV2WayNode, (&aDataSink));
    InitiateSession(*sinkNode);
    iSinkNodes.push_back(sinkNode);
    SupportedSinkNodeInterfaces(sinkNode);
    cmd = GetCmdInfoL();
    cmd->type = PVT_COMMAND_ADD_DATA_SINK;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->iPvtCmdData = aChannelId;
    SendNodeCmdL(PV2WAY_NODE_CMD_INIT, sinkNode, this, cmd);
    return iCommandId++;
}

void CPV324m2Way::DoAddDataSink(TPV2WayNode& aNode,
                                const PVMFCmdResp& aResponse)
{
    TPV2WayNode* sinkNode = &aNode;
    PVMFNodeCapability capability;
    PVMFNodeInterface *node = sinkNode->iNode;
    CPVDatapathNode datapathnode;
    CPV2WayNodeContextData *data = (CPV2WayNodeContextData *) aResponse.GetContext();
    TPV2WayCmdInfo *cmd = (TPV2WayCmdInfo *)data->iContextData;
    cmd->status = aResponse.GetCmdStatus();

    if (node->GetCapability(capability) != PVMFSuccess)
    {
        OSCL_DELETE(sinkNode);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::AddDataSinkL - unable to get capability\n"));
        OSCL_LEAVE(PVMFFailure);
    }

    CPV2WayDecDataChannelDatapath* datapath = NULL;
    PVMFFormatType media_type = capability.iInputFormatCapability[0];
    if (media_type.isAudio())
    {
        datapath = iAudioDecDatapath;
    }
    else if (media_type.isVideo())
    {
        datapath = iVideoDecDatapath;
    }
    else
    {
        OSCL_LEAVE(PVMFErrArgument);
    }

    bool formatSupported = false;
    for (uint i = 0; i < capability.iInputFormatCapability.size(); i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "CPV324m2Way::AddDataSinkL - format %s\n", (capability.iInputFormatCapability[i]).getMIMEStrPtr()));
        if (datapath->GetSourceSinkFormat() == capability.iInputFormatCapability[i])
        {
            formatSupported = true;
            break;
        }
    }
    if (!formatSupported)
    {
        OSCL_LEAVE(PVMFErrNotSupported);
    }

    if ((datapath->GetSourceSinkFormat() == PVMF_MIME_H2632000) || (datapath->GetSourceSinkFormat() == PVMF_MIME_M4V))
    {
        if (datapath)
        {
            if (datapath->GetState() == EClosed)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "CPV324m2Way::AddDataSinkL - creating video datapath\n"));

                //Add tsc node to datapath
                datapathnode.iNode = iTscNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                datapathnode.iIgnoreNodeState = true;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeStarted;
                datapathnode.iOutputPort.iCanCancelPort = true;
                datapathnode.iOutputPort.iPortSetType = EAppDefined;
                datapathnode.iOutputPort.iFormatType = datapath->GetFormat();
                datapathnode.iOutputPort.iPortTag = -cmd->iPvtCmdData;
                datapath->AddNode(datapathnode);

                //Add video parser node to datapath
                datapathnode.iNode = iVideoParserNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iOutputPort.iCanCancelPort = false;
                datapathnode.iOutputPort.iPortSetType = EUseOtherNodePortFormat;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                datapath->AddNode(datapathnode);

                //Add sink node to datapath
                datapathnode.iNode.iNode = sinkNode->iNode;
                datapathnode.iNode.iSessionId = sinkNode->iSessionId;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                datapathnode.iLoggoffOnReset = true;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
                datapath->AddNode(datapathnode);

                datapath->SetCmd(cmd);
                datapath->SetChannelId(cmd->iPvtCmdData);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::AddDataSinkL - invalid video dec datapath state %d\n",
                                 datapath->GetState()));
                OSCL_LEAVE(PVMFErrInvalidState);
            }
        }
    }

    else if ((datapath->GetSourceSinkFormat() == PVMF_MIME_YUV420))
    {
        if (datapath)
        {
            if (datapath->GetState() == EClosed)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "CPV324m2Way::AddDataSinkL - creating video datapath\n"));

                //Add tsc node to datapath
                datapathnode.iNode = iTscNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                datapathnode.iIgnoreNodeState = true;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeStarted;
                datapathnode.iOutputPort.iCanCancelPort = true;
                datapathnode.iOutputPort.iPortSetType = EAppDefined;
                datapathnode.iOutputPort.iFormatType = datapath->GetFormat();
                //datapathnode.iOutputPort.iPortTag = GetStackNodePortTag(EPV2WayVideoOut);
                datapathnode.iOutputPort.iPortTag = -cmd->iPvtCmdData;
                datapath->AddNode(datapathnode);

                //Add video parser node to datapath
                datapathnode.iNode = iVideoParserNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iOutputPort.iCanCancelPort = false;
                datapathnode.iOutputPort.iPortSetType = EUseOtherNodePortFormat;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                datapath->AddNode(datapathnode);

                //Add video dec node to datapath
                datapathnode.iNode = iVideoDecNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = true;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iOutputPort.iPortSetType = EUserDefined;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_YUV420;
                datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                datapath->AddNode(datapathnode);

                //Add sink node to datapath
                datapathnode.iNode.iNode = sinkNode->iNode;
                datapathnode.iNode.iSessionId = sinkNode->iSessionId;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = false;
                if (datapath->GetSourceSinkFormat() == PVMF_MIME_PCM16)
                {
                    //Add audio dec node to datapath
                    datapathnode.iNode = iAudioDecNode;
                    datapathnode.iConfigure = NULL;
                    datapathnode.iCanNodePause = true;
                    datapathnode.iIgnoreNodeState = false;
                    datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                    datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                    datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                    datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                    datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                    datapathnode.iOutputPort.iPortSetType = EUserDefined;
                    datapathnode.iOutputPort.iFormatType = PVMF_MIME_PCM16;
                    datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                    iAudioDecDatapath->AddNode(datapathnode);
                }

                //Add sink node to datapath
                datapathnode.iNode = *sinkNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = true;
                datapathnode.iLoggoffOnReset = true;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
                datapath->AddNode(datapathnode);

                datapath->SetChannelId(cmd->iPvtCmdData);
                datapath->SetCmd(cmd);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::AddDataSinkL - invalid video dec datapath state %d\n",
                                 iVideoDecDatapath->GetState()));
                OSCL_LEAVE(PVMFErrInvalidState);
            }
        }
    }

    else if ((datapath->GetSourceSinkFormat() == PVMF_MIME_AMR_IF2) || (datapath->GetSourceSinkFormat() == PVMF_MIME_AMR_IETF) || (datapath->GetSourceSinkFormat() == PVMF_MIME_G723) || (datapath->GetSourceSinkFormat() == PVMF_MIME_PCM16))
    {
        if (datapath->GetState() == EClosed)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::AddDataSinkL - adding - audio sink node\n"));

            //Add tsc node to datapath
            datapathnode.iNode = iTscNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = false;
            datapathnode.iIgnoreNodeState = true;
            datapathnode.iOutputPort.iRequestPortState = EPVMFNodeStarted;
            datapathnode.iOutputPort.iCanCancelPort = true;
            datapathnode.iOutputPort.iPortSetType = EAppDefined;
            datapathnode.iOutputPort.iFormatType = datapath->GetFormat();
            // Need to put in the LC number here
            //datapathnode.iOutputPort.iPortTag = GetStackNodePortTag(EPV2WayAudioOut);
            datapathnode.iOutputPort.iPortTag = -cmd->iPvtCmdData;
            datapath->AddNode(datapathnode);

            if (datapath->GetSourceSinkFormat() == PVMF_MIME_PCM16)
            {
                //Add audio dec node to datapath
                datapathnode.iNode = iAudioDecNode;
                datapathnode.iConfigure = NULL;
                datapathnode.iCanNodePause = true;
                datapathnode.iIgnoreNodeState = false;
                datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
                datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
                datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
                datapathnode.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
                datapathnode.iOutputPort.iPortSetType = EUserDefined;
                datapathnode.iOutputPort.iFormatType = PVMF_MIME_PCM16;
                datapathnode.iOutputPort.iPortTag = PV2WAY_OUT_PORT;
                datapath->AddNode(datapathnode);
            }

            //Add sink node to datapath
            datapathnode.iNode = *sinkNode;
            datapathnode.iConfigure = NULL;
            datapathnode.iCanNodePause = true;
            datapathnode.iLoggoffOnReset = true;
            datapathnode.iIgnoreNodeState = false;
            datapathnode.iInputPort.iRequestPortState = EPVMFNodeInitialized;
            datapathnode.iInputPort.iPortSetType = EConnectedPortFormat;
            datapathnode.iInputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iInputPort.iPortTag = PV2WAY_IN_PORT;
            datapathnode.iOutputPort.iCanCancelPort = false;
            datapathnode.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            datapathnode.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
            datapath->AddNode(datapathnode);

            datapath->SetChannelId(cmd->iPvtCmdData);

            datapath->SetCmd(cmd);
        }
        else
        {
            OSCL_ASSERT(datapath);
        }
    }

}

PVCommandId CPV324m2Way::RemoveDataSink(PVMFNodeInterface& aDataSink,
                                        OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveDataSinkL(%x, %x, %x)", 0, 0,
                     aContextData));


    switch (iState)
    {
        case EIdle:
        case EInitializing:
        case EResetting:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::RemoveDataSinkL - invalid state(%d)",
                             iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;

        default:
            //State check okay.
            break;
    }

    return DoRemoveDataSourceSink(aDataSink, aContextData);
}

PVCommandId CPV324m2Way::Connect(const PV2WayConnectOptions& aOptions,
                                 PVMFNodeInterface* aCommServer,
                                 OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::ConnectL()"));
    CPVDatapathNode node;

    // validate aCommServer
    if (aCommServer == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::ConnectL comm server is null"));
        OSCL_LEAVE(PVMFErrArgument);
    }

    if (iConnectInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConnectL cmd already sent out"));
        OSCL_LEAVE(PVMFErrBusy);
    }

    /* set clock to 0 and start */
    uint32 startTime = 0;
    bool overflowFlag = false;

    if (!iClock.SetStartTime32(startTime, PVMF_MEDIA_CLOCK_MSEC, overflowFlag))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::Connect: unable to set clock time"));
        OSCL_LEAVE(PVMFFailure);
    }

    switch (iState)
    {
        case ESetup:
            iConnectInfo = GetCmdInfoL();

            iLoopbackMode = aOptions.iLoopbackMode;
            ((TSC_324m *)(PVMFNodeInterface *)iTscNode.iNode)->SetLoopbackMode(iLoopbackMode);
            ((TSC_324m *)(PVMFNodeInterface *)iTscNode.iNode)->SetEndSessionTimeout(((PV2Way324ConnectOptions *)(&aOptions))->iDisconnectTimeoutInterval);

            // Store reference to comm server
            iCommNode = TPV2WayNode(aCommServer);
            InitiateSession(iCommNode);

            //Add tsc node to datapath
            node.iNode = iTscNode;
            node.iConfigure = this;
            node.iIgnoreNodeState = false;
            node.iConfigTime = EConfigBeforeStart;
            node.iOutputPort.iRequestPortState = EPVMFNodeInitialized;
            node.iOutputPort.iPortSetType = EUserDefined;
            node.iOutputPort.iFormatType = PVMF_MIME_H223;
            //node.iOutputPort.iPortType = EPVIOPort;
            node.iOutputPort.iPortTag = PV_MULTIPLEXED;
            iMuxDatapath->AddNode(node);

            //Add rcomm node to datapath
            node.iNode = iCommNode;
            node.iLoggoffOnReset = true;
            node.iConfigure = NULL;
            node.iIgnoreNodeState = false;
            node.iLoggoffOnReset = false;
            node.iInputPort.iRequestPortState = EPVMFNodeInitialized;
            node.iInputPort.iPortSetType = EUserDefined;
            node.iInputPort.iFormatType = PVMF_MIME_H223;
            node.iInputPort.iPortTag = PV2WAY_IO_PORT;
            //node.iInputPort.iProperty.iPortType = EPVIOPort;
            node.iOutputPort.iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            //node.iOutputPort.iPortType = EPVInvalidPortType;
            node.iOutputPort.iPortTag = PV2WAY_UNKNOWN_PORT;
            iMuxDatapath->AddNode(node);

            iConnectInfo->type = PVT_COMMAND_CONNECT;
            iConnectInfo->id = iCommandId;
            iConnectInfo->contextData = aContextData;
            SetState(EConnecting);

            iMuxDatapath->Open();
            break;

        case EConnected:
            iConnectInfo = GetCmdInfoL();
            iConnectInfo->type = PVT_COMMAND_CONNECT;
            iConnectInfo->status = PVMFSuccess;
            iConnectInfo->id = iCommandId;
            iConnectInfo->contextData = aContextData;
            Dispatch(iConnectInfo);
            iConnectInfo = NULL;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::ConnectL - invalid state(%d)", iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    /*
    	// start enc datapaths that are already created
    	if (iAudioEncDatapath->GetState() != EClosed)
    	{
    		iAudioEncDatapath->CheckOpen();
    	}
    	if (iVideoEncDatapath->GetState() != EClosed)
    	{
    		iVideoEncDatapath->CheckOpen();
    	}
    	*/
    return iCommandId++;
}

PVCommandId CPV324m2Way::Disconnect(OsclAny* aContextData)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::Disconnect()"));

    if (iDisconnectInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::Disconnect cmd already sent out"));
        OSCL_LEAVE(PVMFErrBusy);
    }

    switch (iState)
    {
        case EConnecting:
            //Connect in progress, notify application that it has been cancelled.
            iConnectInfo->status = PVMFErrCancelled;
            Dispatch(iConnectInfo);
            iConnectInfo = NULL;
            //Fall through to next case

        case EConnected:

            iTSC324mInterface->EndSessionCommand();

            iEndSessionTimer->SetObserver(this);
            iEndSessionTimer->Request(END_SESSION_TIMER_ID, END_SESSION_TIMER_ID,
                                      END_SESSION_TIMER_VALUE, this);

            iDisconnectInfo = GetCmdInfoL();

            iDisconnectInfo->type = PVT_COMMAND_DISCONNECT;
            iDisconnectInfo->contextData = aContextData;
            iDisconnectInfo->id = iCommandId;

            //We wait to InitiateDisconnect() till iEndSessionTimer timer expires
            break;

        case EDisconnecting:
            //If at this point, then remote disconnect is in progress, just treat as user disconnect.
            iDisconnectInfo = GetCmdInfoL();

            iDisconnectInfo->type = PVT_COMMAND_DISCONNECT;
            iDisconnectInfo->contextData = aContextData;
            iDisconnectInfo->id = iCommandId;
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::Disconnect - invalid state(%d)", iState));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    return iCommandId++;
}

void CPV324m2Way::InitiateDisconnect()
{
    SetState(EDisconnecting);
    CheckState();
}

void CPV324m2Way::InitiateReset()
{
    SetState(EResetting);

    if (isIFrameReqTimerActive)
    {
        iIFrameReqTimer.Cancel(IFRAME_REQ_TIMERID);
        isIFrameReqTimerActive = false;
    }

    if ((iAudioDecDatapath != NULL) && (iAudioDecDatapath->GetState() != EClosed))
    {
        iAudioDecDatapath->SetCmd(NULL);
    }

    if ((iAudioEncDatapath != NULL) && (iAudioEncDatapath->GetState() != EClosed))
    {
        iAudioEncDatapath->SetCmd(NULL);
    }

    if ((iVideoDecDatapath != NULL) && (iVideoDecDatapath->GetState() != EClosed))
    {
        iVideoDecDatapath->SetCmd(NULL);
    }

    if ((iVideoEncDatapath != NULL) && (iVideoEncDatapath->GetState() != EClosed))
    {
        iVideoEncDatapath->SetCmd(NULL);
    }

    CheckState();
}

void CPV324m2Way::CheckState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckState state %d\n", iState));
    switch (iState)
    {
        case EInitializing:
            CheckInit();
            break;

        case EConnecting:
            CheckConnect();
            break;

        case EDisconnecting:
            CheckDisconnect();
            break;

        case EResetting:
            CheckReset();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "CPV324m2Way::CheckState warning: static state!"));
            break;
    }
}

void CPV324m2Way::CheckInit()
{
//	PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0,"CPV324m2Way::CheckInit state %d, video enc node state %d, interface state %d\n", iState, ((PVMFNodeInterface *)iVideoEncNode)->GetState(), iVideoEncNodeInterface.iState));
    int32 error;

    if (((PVMFNodeInterface *)iTscNode)->GetState() == EPVMFNodeIdle)
    {
        OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_INIT, &iTscNode, this));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::CheckInit unable to init tsc node!\n"));
                             SetState(EResetting);
                             CheckState();
                             return;);
    }
    if (((PVMFNodeInterface *)iTscNode)->GetState() == EPVMFNodeInitialized)
    {

        SetState(ESetup);

        iInitInfo->status = PVMFSuccess;
        Dispatch(iInitInfo);
        iInitInfo = NULL;
    }
}

void CPV324m2Way::CheckConnect()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::CheckConnect state %d, comm state %d, tsc state %d\n",
                     iState, ((PVMFNodeInterface *)iCommNode)->GetState(),
                     ((PVMFNodeInterface *)iTscNode)->GetState()));

    if ((iMuxDatapath->GetState() == EOpened) && iIsStackConnected)
    {
        /* Increase video encoder bitrate if required */
        //	PVMp4H263EncExtensionInterface *ptr = (PVMp4H263EncExtensionInterface *) iVideoEncNodeInterface.iInterface;
        //	ptr->SetOutputBitRate(0, VIDEO_ENCODER_BITRATE);
        SetState(EConnected);

        iConnectInfo->status = PVMFSuccess;
        Dispatch(iConnectInfo);
        iConnectInfo = NULL;
    }
}


void CPV324m2Way::CheckDisconnect()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::CheckDisconnect state %d, mux datapath state %d, stack connected %d\n",
                     iState, iMuxDatapath->GetState(), iIsStackConnected));
    if ((iMuxDatapath->GetState() == EClosed) &&
            !iIsStackConnected)
    {
        SetState(ESetup);

        //Connect failed
        if (iConnectInfo)
        {
            iConnectInfo->status = PVMFFailure;
            Dispatch(iConnectInfo);
            iConnectInfo = NULL;
        }
        //Else command cancelled
        else if (iCancelInfo)
        {
            iCancelInfo->status = PVMFSuccess;
            Dispatch(iCancelInfo);
            iCancelInfo = NULL;
        }
        //Else local disconnect
        else if (iDisconnectInfo)
        {
            iDisconnectInfo->status = PVMFSuccess;
            Dispatch(iDisconnectInfo);
            iDisconnectInfo = NULL;
        }
        //Else remote disconnect
        else
        {
            TPV2WayEventInfo* aEvent = NULL;
            if (!GetEventInfo(aEvent))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::CheckDisconnect unable to notify app!\n"));
                return;
            }
            aEvent->type = PVT_INDICATION_DISCONNECT;
            Dispatch(aEvent);
        }
    }
    else
    {
        iMuxDatapath->Close();
    }
}

void CPV324m2Way::CheckReset()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::CheckReset state %d \n", iState));
    int32 error;

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    switch (iRecordFileState)
    {
        case File2WayInitializing:
        case File2WayInitialized:
            InitiateResetRecordFile();
            break;

        case File2WayIdle:
        case File2WayResetting:
            break;
    }
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    switch (iPlayFileState)
    {
        case File2WayInitializing:
        case File2WayInitialized:
            InitiateResetPlayFile();
            break;

        case File2WayIdle:
        case File2WayResetting:
            break;
    }
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    if (iRecordFileState != File2WayIdle) return;
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    if (iPlayFileState != File2WayIdle) return;
#endif



    if ((iAudioEncDatapath != NULL) && (iAudioEncDatapath->GetState() == EClosed) &&
            (iAudioDecDatapath != NULL) && (iAudioDecDatapath->GetState() == EClosed) &&
            (iVideoEncDatapath != NULL) && (iVideoEncDatapath->GetState() == EClosed) &&
            (iVideoDecDatapath != NULL) && (iVideoDecDatapath->GetState() == EClosed))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::CheckReset state %d, AD state %d, VD state %d, AE state %d, VE state %d\n",
                         iState,
                         iAudioDecDatapath->GetState(),
                         iVideoDecDatapath->GetState(),
                         iAudioEncDatapath->GetState(),
                         iVideoEncDatapath->GetState()));

        TPVMFNodeInterfaceState vidEncState;

        if (iVideoEncNode != NULL)
        {
            vidEncState = ((PVMFNodeInterface *)iVideoEncNode)->GetState() ;
            if ((vidEncState == EPVMFNodeInitialized) || (vidEncState == EPVMFNodeError))
            {
                OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_RESET, &iVideoEncNode, this));
                OSCL_FIRST_CATCH_ANY(error,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                     (0, "CPV324m2Way::CheckReset unable to reset video encoder node!\n"));
                                     return;);
            }
        }

        if (iAudioEncNode != NULL)
        {
            if ((iAudioEncNode.iNode->GetState() == EPVMFNodeInitialized) ||
                    (iAudioEncNode.iNode->GetState() == EPVMFNodeError))
            {
                OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_RESET, &iAudioEncNode, this));
                OSCL_FIRST_CATCH_ANY(error,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                     (0, "CPV324m2Way::CheckReset unable to reset audio encoder node!\n"));
                                     return;);
            }
        }
    }

    TPVMFNodeInterfaceState tscState = ((PVMFNodeInterface *)iTscNode)->GetState() ;

    if ((tscState == EPVMFNodeInitialized) ||
            (tscState == EPVMFNodeError))
    {
        OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_RESET, &iTscNode, this));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::CheckReset unable to reset tsc node!\n"));
                             return;);
    }
    bool aFlag = false;
    if ((tscState == EPVMFNodeIdle) &&
            (iVideoEncNodeInterface.iState != PV2WayNodeInterface::QueryInterface))
    {
        if (iVideoEncNode.iNode != NULL)
            aFlag = IsNodeReset(*(iVideoEncNode.iNode));
        else
            aFlag = true;

    }

    if (aFlag == true)
    {
        iIncomingChannelParams.clear();
        iOutgoingChannelParams.clear();

        SetState(EIdle);

        //Init failed
        if (iInitInfo)
        {
            iInitInfo->status = PVMFFailure;
            Dispatch(iInitInfo);
            iInitInfo = NULL;
        }
        //Else command cancelled
        else if (iCancelInfo)
        {
            iCancelInfo->status = PVMFSuccess;
            Dispatch(iCancelInfo);
            iCancelInfo = NULL;
        }
        //Else local reset
        else
        {
            iResetInfo->status = PVMFSuccess;
            Dispatch(iResetInfo);
            iResetInfo = NULL;
        }
    }

}


void CPV324m2Way::RemoveAudioDecPath()
{
    if (iAudioDecDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::RemoveAudioDecPath audio dec path state %d\n",
                         iAudioDecDatapath->GetState()));
    }

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    RemoveAudioRecPath();
#endif

    if ((iAudioDecDatapath != NULL) &&
            (iAudioDecDatapath->GetState() == EClosed))
    {
        iAudioDecDatapath->ResetDatapath();
        iAudioSinkNode.Clear();
    }
}

void CPV324m2Way::RemoveAudioEncPath()
{
    if (iAudioEncDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::RemoveAudioEncPath audio enc path state %d\n",
                         iAudioEncDatapath->GetState()));
    }

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    RemoveAudioPreviewPath();
#endif

    if ((iAudioEncDatapath != NULL) &&
            (iAudioEncDatapath->GetState() == EClosed))
    {
        iAudioEncDatapath->SetSourceInputPort(NULL);
        iAudioEncDatapath->ResetDatapath();
        iAudioSrcNode.Clear();
    }
}

void CPV324m2Way::RemoveVideoDecPath()
{
    if (iVideoDecDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::RemoveVideoDecPath video dec path state %d\n",
                         iVideoDecDatapath->GetState()));
    }

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    RemoveVideoRecPath();
#endif

    if ((iVideoDecDatapath != NULL) &&
            (iVideoDecDatapath->GetState() == EClosed))
    {
        iVideoDecDatapath->ResetDatapath();
    }
}

void CPV324m2Way::RemoveVideoEncPath()
{
    if (iVideoEncDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::RemoveVideoEncPath video enc path state %d\n",
                         iVideoEncDatapath->GetState()));
    }
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    RemoveVideoPreviewPath();
#endif

    if ((iVideoEncDatapath != NULL) &&
            (iVideoEncDatapath->GetState() == EClosed))
    {
        //Video encoder will be deleted at reset time.

        iVideoEncDatapath->ResetDatapath();
    }
}

void CPV324m2Way::HandleCommNodeCmd(PV2WayNodeCmdType aType,
                                    const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleCommNodeCmd type %d\n", aType));

    switch (aType)
    {
        case PV2WAY_NODE_CMD_INIT:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                SetState(EResetting);
            }

            CheckState();
            break;

        case PV2WAY_NODE_CMD_RESET:
            CheckState();
            break;

        default:
            break;
    }
}

void CPV324m2Way::HandleTscNodeCmd(PV2WayNodeCmdType aType,
                                   const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleTscNodeCmd type %d\n", aType));

    switch (aType)
    {
        case PV2WAY_NODE_CMD_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                ((TSC_324m *)((PVMFNodeInterface *)iTscNode))->SetIncomingChannelConfig(iIncomingChannelParams);
                ((TSC_324m *)((PVMFNodeInterface *)iTscNode))->SetOutgoingChannelConfig(iOutgoingChannelParams);
            }
            else
            {
                SetState(EResetting);
            }

            CheckState();
            break;

        case PV2WAY_NODE_CMD_RESET:
            CheckState();
            break;

        default:
            break;
    }
}


void CPV324m2Way::HandleVideoDecNodeCmd(PV2WayNodeCmdType aType,
                                        const PVMFCmdResp& aResponse)
{
    OSCL_UNUSED_ARG(aType);
    OSCL_UNUSED_ARG(aResponse);
}

void CPV324m2Way::HandleVideoEncNodeCmd(PV2WayNodeCmdType aType,
                                        const PVMFCmdResp& aResponse)
{
    if (iVideoEncDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::HandleVideoEncNodeCmd type %d, video enc path state %d\n",
                         aType, iVideoEncDatapath->GetState()));
    }

    switch (aType)
    {
        case PV2WAY_NODE_CMD_QUERY_INTERFACE:

            if (aResponse.GetCmdId() == iVideoEncQueryIntCmdId)
            {
                iVideoEncQueryIntCmdId = -1;
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoEncNodeInterface.iState = PV2WayNodeInterface::HasInterface;
                    // Set the FormatSpecificInfo if available
                    uint32 fsi_len = 0;
                    uint8* fsi = iVideoEncDatapath->GetFormatSpecificInfo(&fsi_len);
                    if (fsi && fsi_len)
                    {
                        OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface *,
                                         iVideoEncNodeInterface.iInterface)->SetFSIParam(fsi, fsi_len);
                        if (iAddDataSourceVideoCmd)
                        {
                            iVideoEncDatapath->SetCmd(iAddDataSourceVideoCmd);
                            iAddDataSourceVideoCmd = NULL;
                        }
                    }
                }
                else
                {
                    iVideoEncNodeInterface.iState = PV2WayNodeInterface::NoInterface;
                    SetState(EResetting);
                }

            }

            CheckState();
            break;

        case PV2WAY_NODE_CMD_INIT:
            CheckState();
            break;

        case PV2WAY_NODE_CMD_RESET:
            CheckState();
            break;

        default:
            break;
    }
}

void CPV324m2Way::HandleSinkNodeCmd(PV2WayNodeCmdType aType,
                                    const PVMFCmdResp& aResponse,
                                    TPV2WayNode* aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleSinkNodeCmd type %d\n", aType));
    switch (aType)
    {
        case PV2WAY_NODE_CMD_INIT:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                SetState(EResetting);
                CheckState();
            }
            else
            {
                DoAddDataSink(*aNode, aResponse);
            }
            break;
        case PV2WAY_NODE_CMD_QUERY_INTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                for (uint32 ii = 0; ii < iSinkNodeList.size();ii++)
                {
                    if ((aNode == iSinkNodeList[ii].iSinkNode) &&
                            (aResponse.GetCmdId() == iSinkNodeList[ii].iNodeInterface.iId))
                    {
                        iSinkNodeList[ii].iNodeInterface.iInterface =
                            iClockSyncInterface.iInterface;
                        iClockSyncInterface.Reset();
                        if (iSinkNodeList[ii].iNodeInterface.iInterface != NULL)
                        {
                            iSinkNodeList[ii].iNodeInterface.iState =
                                PV2WayNodeInterface::HasInterface;
                            PvmfNodesSyncControlInterface* ptr = NULL;
                            ptr = OSCL_STATIC_CAST(PvmfNodesSyncControlInterface*,
                                                   iSinkNodeList[ii].iNodeInterface.iInterface);
                            ptr->SetClock(&iClock);
                        }
                        break;
                    }
                }
            }
            else
            {
                SetState(EResetting);
                CheckState();
            }
            break;
        case PV2WAY_NODE_CMD_SKIP_MEDIA_DATA:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                            (0, "CPV324m2Way::HandleSinkNodeCmd type %d, SkipComplete for Node %x ",
                             aType, aNode->iNode));
            if (iAudioDecDatapath) iAudioDecDatapath->SkipComplete(aNode);
            if (iVideoDecDatapath) iVideoDecDatapath->SkipComplete(aNode);
            RunIfNotReady();
            break;
        default:
            break;
    }
}

void CPV324m2Way::SupportedSinkNodeInterfaces(TPV2WayNode* aNode)
{

    int32 error;

    //Currently this only checks if the sink node support PvmfSyncNodeControlInterface

    TPV2WayNodeQueryInterfaceParams queryParam;
    queryParam.iUuid = (PVUuid *) & iSyncControlPVUuid;
    SinkNodeIFList sinkNode;
    sinkNode.iSinkNode = aNode;
    queryParam.iInterfacePtr = &iClockSyncInterface.iInterface;
    OSCL_TRY(error, sinkNode.iNodeInterface.iId =
                 SendNodeCmdL(PV2WAY_NODE_CMD_QUERY_INTERFACE, aNode, this, &queryParam));

    iSinkNodeList.push_back(sinkNode);

    OSCL_FIRST_CATCH_ANY(error,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                         (0, "CPV324m2Way::SupportedSinkNodeInterfaces unable to query for MediaOutputNode extension interface!\n"));
                         SetState(EResetting);
                         CheckState();
                         return;);
}


void CPV324m2Way::HandleAudioEncNodeCmd(PV2WayNodeCmdType aType,
                                        const PVMFCmdResp& aResponse)
{
    if (iAudioEncDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::HandleAudioEncNodeCmd type %d, audio enc path state %d\n",
                         aType, iAudioEncDatapath->GetState()));
    }
    OSCL_UNUSED_ARG(aResponse);
    OSCL_UNUSED_ARG(aType);

    switch (aType)
    {
        case PV2WAY_NODE_CMD_QUERY_INTERFACE:

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iAudioEncNodeInterface.iState = PV2WayNodeInterface::HasInterface;

            }
            else
            {
                iAudioEncNodeInterface.iState = PV2WayNodeInterface::NoInterface;
                SetState(EResetting);
            }

            CheckState();
            break;

        case PV2WAY_NODE_CMD_INIT:
            CheckState();
            break;

        case PV2WAY_NODE_CMD_RESET:
            CheckState();
            break;

        default:
            break;
    }
}

void CPV324m2Way::GenerateIFrame(PVMFPortInterface *aPort)
{
    if (iVideoEncDatapath)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::GenerateIFrame, vid enc path state %d\n",
                         iVideoEncDatapath->GetState()));

        if ((iVideoEncDatapath->IsPortInDatapath(aPort)) &&
                (iVideoEncDatapath->GetState() == EOpened))
        {
            if (!((PVMp4H263EncExtensionInterface *) iVideoEncNodeInterface.iInterface)->RequestIFrame())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::GenerateIFrame - unable to generate iframe"));
            }
        }
    }
}

void CPV324m2Way::RequestRemoteIFrame(PVMFPortInterface *aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "CPV324m2Way::RequestRemoteIFrame, timer active %d\n", isIFrameReqTimerActive));
    TSC_324m *nodeIface = (TSC_324m *)((PVMFNodeInterface *)iTscNode);
    if (nodeIface &&
            !isIFrameReqTimerActive &&
            (nodeIface->RequestFrameUpdate(aPort) == EPVT_Success))
    {
        //Still need to actually send an iframe request!!!!
        iIFrameReqTimer.Request(IFRAME_REQ_TIMERID, (int32)this,
                                iMinIFrameRequestInterval, this);
        isIFrameReqTimerActive = true;
    }
}

PVCommandId CPV324m2Way::GetState(PV2WayState& aState,
                                  OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::GetPV2WayState %d\n", iState));

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_GET_PV2WAY_STATE;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;

    aState = iState;

    Dispatch(cmd);

    return iCommandId++;
}


PVCommandId CPV324m2Way::SetLatencyQualityTradeoff(PVMFNodeInterface& aTrack,
        int32 aTradeoff,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aTrack);
    OSCL_UNUSED_ARG(aTradeoff);
    OSCL_UNUSED_ARG(aContextData);
    return iCommandId++;
}


PVCommandId CPV324m2Way::Pause(PV2WayDirection aDirection,
                               PVTrackId aTrackId,
                               OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV324m2Way::Pause\n"));
    OSCL_UNUSED_ARG(aDirection);
    OSCL_UNUSED_ARG(aTrackId);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(PVMFErrNotSupported);
    return iCommandId++;
}


PVCommandId CPV324m2Way::Resume(PV2WayDirection aDirection,
                                PVTrackId aTrackId,
                                OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::Resume\n"));
    OSCL_UNUSED_ARG(aDirection);
    OSCL_UNUSED_ARG(aTrackId);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(PVMFErrNotSupported);

    return iCommandId++;
}


PVCommandId CPV324m2Way::SetLogAppender(const char * aTag,
                                        OsclSharedPtr<PVLoggerAppender>& aAppender,
                                        OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::SetLogAppenderL\n"));

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    PVLogger *logger = PVLogger::GetLoggerObject(aTag);
    logger->AddAppender(aAppender);

    // print sdk info
    PVSDKInfo sdkinfo;
    FillSDKInfo(sdkinfo);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PV RELEASE LABEL = %s", sdkinfo.iLabel.get_cstr()));

    cmd->type = PVT_COMMAND_SET_LOG_APPENDER;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;

    Dispatch(cmd);
    return iCommandId++;
}

PVCommandId CPV324m2Way::RemoveLogAppender(const char * aTag,
        OsclSharedPtr<PVLoggerAppender>& aAppender,
        OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveLogAppenderL\n"));

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_REMOVE_LOG_APPENDER;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;

    PVLogger *logger = PVLogger::GetLoggerObject(aTag);
    logger->RemoveAppender(aAppender);

    Dispatch(cmd);
    return iCommandId++;
}

PVCommandId CPV324m2Way::SetLogLevel(const char * aTag,
                                     int32 aLevel,
                                     bool aSetSubtree,
                                     OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::SetLogLevelL\n"));
    OSCL_UNUSED_ARG(aSetSubtree);

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_SET_LOG_LEVEL;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;

    PVLogger *logger = PVLogger::GetLoggerObject(aTag);
    logger->SetLogLevel(aLevel);

    Dispatch(cmd);
    return iCommandId++;
}

PVCommandId CPV324m2Way::GetLogLevel(const char * aTag,
                                     int32& aLogInfo,
                                     OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::GetLogLevelL\n"));

    TPV2WayCmdInfo *cmd = GetCmdInfoL();

    cmd->type = PVT_COMMAND_GET_LOG_LEVEL;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFSuccess;

    PVLogger *logger = PVLogger::GetLoggerObject(aTag);
    aLogInfo = logger->GetLogLevel();

    Dispatch(cmd);
    return iCommandId++;
}


PVCommandId CPV324m2Way::QueryInterface(const PVUuid& aUuid,
                                        PVInterface*& aInterfacePtr,
                                        OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::QueryInterface()\n"));

    TPV2WayNodeQueryInterfaceParams queryParam;
    queryParam.iUuid = (PVUuid*) & aUuid;
    queryParam.iInterfacePtr = &aInterfacePtr;

    TPV2WayCmdInfo *cmd = GetCmdInfoL();
    cmd->type = PVT_COMMAND_QUERY_INTERFACE;
    cmd->id = iCommandId;
    cmd->contextData = aContextData;
    cmd->status = PVMFPending;
    aInterfacePtr = NULL;

    if (aUuid == PVH324MConfigUuid && ((PVMFNodeInterface *)iTscNode))
    {
        int32 error = 0;
        OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_QUERY_INTERFACE,
                                     &iTscNode, this, &queryParam, cmd));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::QueryInterface failed!\n"));
                             cmd->status = PVMFFailure;
                             Dispatch(cmd););
        cmd->status = PVMFSuccess; //ks added
    }
    else if (aUuid == PVMp4H263EncExtensionUUID &&
             ((PVMFNodeInterface *)iVideoEncNode))
    {
        int32 error = 0;
        OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_QUERY_INTERFACE, &iVideoEncNode,
                                     this, &queryParam, cmd));
        OSCL_FIRST_CATCH_ANY(error,
                             cmd->status = PVMFFailure;
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::QueryInterface unable to query for video encoder interface!\n")););
    }
    else
    {
        aInterfacePtr = NULL;
        cmd->status = PVMFErrNotSupported;
    }
    if (cmd->status != PVMFPending)
        Dispatch(cmd);

    return iCommandId++;
}

PVCommandId CPV324m2Way::QueryUUID(const PvmfMimeString& aMimeType,
                                   Oscl_Vector<PVUuid, BasicAlloc>& aUuids,
                                   bool aExactUuidsOnly,
                                   OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aUuids);
    OSCL_UNUSED_ARG(aExactUuidsOnly);
    OSCL_UNUSED_ARG(aContextData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::QueryUUID\n"));
    OSCL_LEAVE(PVMFErrNotSupported);
    return 0;
}

PVCommandId CPV324m2Way::CancelAllCommands(OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CancelAllCommands state %d\n", iState));

    if (iCancelInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPV324m2Way::CancelAllCommands, cancel in progress!\n"));
        OSCL_LEAVE(PVMFErrBusy);
    }

    switch (iState)
    {
        case EInitializing:
            iCancelInfo = GetCmdInfoL();
            iCancelInfo->type = PVT_COMMAND_CANCEL_ALL_COMMANDS;
            iCancelInfo->id = iCommandId;
            iCancelInfo->contextData = aContextData;
            SetState(EResetting);

            if (!iInitInfo)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::CancelAllCommands, no init info!\n"));
            }
            else
            {
                iInitInfo->status = PVMFErrCancelled;
                Dispatch(iInitInfo);
                iInitInfo = NULL;
            }

            CheckState();
            break;

        case EConnecting:
            iCancelInfo = GetCmdInfoL();
            iCancelInfo->type = PVT_COMMAND_CANCEL_ALL_COMMANDS;
            iCancelInfo->id = iCommandId;
            iCancelInfo->contextData = aContextData;
            SetState(EDisconnecting);

            if (!iConnectInfo)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::CancelAllCommands, no connect info!\n"));
            }
            else
            {
                iConnectInfo->status = PVMFErrCancelled;
                Dispatch(iConnectInfo);
                iConnectInfo = NULL;
            }

            CheckState();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::CancelAllCommands invalid state!\n"));
            OSCL_LEAVE(PVMFErrInvalidState);
            break;
    }

    return iCommandId++;
}


void CPV324m2Way::ConstructL(PVMFNodeInterface* aTsc,
                             TPVTerminalType aType,
                             PVCommandStatusObserver* aCmdStatusObserver,
                             PVInformationalEventObserver *aInfoEventObserver,
                             PVErrorEventObserver *aErrorEventObserver)
{
    OSCL_UNUSED_ARG(aTsc);

    iTerminalType = aType;

    /* Initialize the clock */
    if (!iClock.SetClockTimebase(iTickCountTimeBase))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::ConstructL: unable to initialize clock"));
        OSCL_LEAVE(PVMFFailure);
    }

    FormatCapabilityInfo inFormat;
    inFormat.dir = INCOMING;

    /* Add user input types */
    inFormat.format = PVMF_MIME_USERINPUT_DTMF;
    iIncomingUserInputFormats.push_back(inFormat);
    inFormat.format = PVMF_MIME_USERINPUT_IA5_STRING;
    iIncomingUserInputFormats.push_back(inFormat);
    inFormat.format = PVMF_MIME_USERINPUT_BASIC_STRING;
    iIncomingUserInputFormats.push_back(inFormat);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::ConstructL (%x, %x, %x, %x)",
                     aTsc, aCmdStatusObserver,
                     aInfoEventObserver, aErrorEventObserver));
    iMuxDatapath = CPV2WayMuxDatapath::NewL(iLogger, PVMF_MIME_H223, this);

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    iRecFilename = DEFAULT_RECORDED_CALL_FILENAME;

    iAudioRecDatapath = CPV2WayRecDatapath::NewL(iLogger, PVMF_COMPRESSED_AUDIO_FORMAT,
                        *iAudioDecDatapath, this);
    iVideoRecDatapath = CPV2WayRecDatapath::NewL(iLogger, PVMF_COMPRESSED_VIDEO_FORMAT,
                        *iVideoDecDatapath, this);

    //Set up recording datapath dependencies
    iAudioDecDatapath->AddDependentDatapath(*iAudioRecDatapath);
    iVideoDecDatapath->AddDependentDatapath(*iVideoRecDatapath);
    iAudioRecDatapath->AddParentDatapath(*iAudioDecDatapath);
    iVideoRecDatapath->AddParentDatapath(*iVideoDecDatapath);
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    iAudioPreviewDatapath = CPV2WayPreviewDatapath::NewL(iLogger,
                            PVMF_COMPRESSED_AUDIO_FORMAT, this);
    iVideoPreviewDatapath = CPV2WayPreviewDatapath::NewL(iLogger,
                            PVMF_COMPRESSED_VIDEO_FORMAT, this);

    //Set up preview sink datapath dependencies
    iAudioEncDatapath->AddDependentDatapath(*iAudioPreviewDatapath);
    iVideoEncDatapath->AddDependentDatapath(*iVideoPreviewDatapath);
    iAudioPreviewDatapath->AddParentDatapath(*iAudioEncDatapath);
    iVideoPreviewDatapath->AddParentDatapath(*iVideoEncDatapath);
#endif

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "Full pv_2way_engine\n"));
#ifdef PV_DISABLE_VIDRECNODE
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "VidRec node disabled\n"));
#else
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "VidRec node enabled\n"));
#endif

#ifdef PV_DISABLE_DEVSOUNDNODES
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "DevSound nodes disabled\n"));
#else
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "DevSound nodes enabled\n"));
#endif

    SetDefaults();
    iCmdStatusObserver = aCmdStatusObserver;
    iInfoEventObserver = aInfoEventObserver;
    iErrorEventObserver = aErrorEventObserver;

    iPendingNotifications.reserve(MAX_PENDING_2WAY_COMMANDS + MAX_PENDING_2WAY_EVENTS);
    iPendingNodeCmdInfo.reserve(MAX_PENDING_2WAY_NODE_COMMANDS);
    iIncomingChannelParams.reserve(MAX_LOGICAL_CHANNEL_PARAMS);
    iOutgoingChannelParams.reserve(MAX_LOGICAL_CHANNEL_PARAMS);

    int32 i;
    for (i = 0; i < MAX_PENDING_2WAY_COMMANDS; i++)
    {
        iFreeCmdInfo.push_back(&iCmdInfo[i]);
    }

    for (i = 0; i < MAX_PENDING_2WAY_EVENTS; i++)
    {
        iFreeEventInfo.push_back(&iEventInfo[i]);
    }

    for (i = 0; i < MAX_PENDING_2WAY_NODE_COMMANDS; i++)
    {
        iFreeNodeCmdInfo.push_back(&iNodeCmdInfo[i]);
    }

    iSourceNodes.reserve(3);
    iSinkNodes.reserve(3);

    AddToScheduler();

    PreInit();

    return;
}

void CPV324m2Way::SetDefaults()
{
    uint32 i;
    SetState(EIdle);

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    if (iPlayFromFileNode.iNode)
    {
        iPlayFromFileNode.iNode->ThreadLogoff();
        OSCL_DELETE(iPlayFromFileNode.iNode);
        iPlayFromFileNode.Clear();;
    }

    if (iVideoSrcSplitterNode.iNode)
    {
        iVideoSrcSplitterNode.iNode->ThreadLogoff();
        OSCL_DELETE(iVideoSrcSplitterNode.iNode);
        iVideoSrcSplitterNode = NULL;
    }

    if (iAudioSrcSplitterNode)
    {
        iAudioSrcSplitterNode->ThreadLogoff();
        OSCL_DELETE(iAudioSrcSplitterNode);
        iAudioSrcSplitterNode = NULL;
    }
#endif

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    if (iFFComposerNode)
    {
        iFFComposerNode->ThreadLogoff();
        PVMp4FFComposerNodeFactory::DeleteMp4FFComposer(iFFComposerNode);
        iFFComposerNode = NULL;
    }

    if (iVideoDecSplitterNode)
    {
        iVideoDecSplitterNode->ThreadLogoff();
        OSCL_DELETE(iVideoDecSplitterNode);
        iVideoDecSplitterNode = NULL;
    }
#endif

    if (iVideoParserNode.iNode)
    {
        iVideoParserNode.iNode->ThreadLogoff();
        OSCL_DELETE(iVideoParserNode.iNode);
        iVideoParserNode.Clear();
    }

    if (iVideoDecNode.iNode)
    {
        iVideoDecNode.iNode->ThreadLogoff();

#ifdef PV2WAY_USE_OMX
        DELETE_OMX_VIDEO_DEC_NODE(iVideoDecNode.iNode);
#else
        DELETE_VIDEO_DEC_NODE(iVideoDecNode.iNode);
#endif // PV2WAY_USE_OMX

        iVideoDecNode.Clear();
    }

    if (iAudioDecNode.iNode)
    {
        iAudioDecNode.iNode->ThreadLogoff();
#ifdef PV2WAY_USE_OMX
        DELETE_OMX_AUDIO_DEC_NODE(iAudioDecNode.iNode);
#else
        DELETE_AUDIO_DEC_NODE(iAudioDecNode.iNode);
#endif // PV2WAY_USE_OMX

        iAudioDecNode.Clear();
    }

    if (iTscNode.iNode)
    {
        iTscNode.iNode->ThreadLogoff();
    }

    if (iAudioEncNode.iNode)
    {
        iAudioEncNode.iNode->ThreadLogoff();
        if (iAudioEncNodeInterface.iInterface)
            iAudioEncNodeInterface.iInterface->removeRef();
#ifdef PV2WAY_USE_OMX
        DELETE_OMX_ENC_NODE(iAudioEncNode.iNode);
#else
        DELETE_AUDIO_ENC_NODE(iAudioEncNode.iNode);
#endif
        iAudioEncNode.Clear() ;
        iAudioEncNodeInterface.Reset();
    }

    ClearVideoEncNode();

    if (iCommNode.iNode)
    {
        iCommNode.iNode->ThreadLogoff();
        iCommNode.Clear();
    }

    iFormatCapability.clear();
    iIncomingAudioCodecs.clear();
    iOutgoingAudioCodecs.clear();
    iIncomingVideoCodecs.clear();
    iOutgoingVideoCodecs.clear();

    /* Stop the clock */
    iClock.Stop();

    /* Delete the list of sink/source nodes */
    for (i = 0;i < iSourceNodes.size();i++)
    {
        TPV2WayNode* lNode = iSourceNodes[i];
        OSCL_DELETE(lNode);
        iSourceNodes[i] = 0;
    }
    iSourceNodes.clear();
    iSourceNodes.destroy();

    for (i = 0;i < iSinkNodes.size();i++)
    {
        TPV2WayNode* lNode = iSinkNodes[i] ;
        OSCL_DELETE(lNode);
        iSinkNodes[i] = 0;
    }
    iSinkNodes.clear();
    iSinkNodes.destroy();
    iAddDataSourceVideoCmd = NULL;
    return;
}

void CPV324m2Way::DoCancel()
{

}

void CPV324m2Way::Run()
{
    int32 error;
    TPV2WayCmdInfo* cmd = NULL;
    TPV2WayEventInfo* event = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::Run state %d, number of notifications pending %d\n",
                     iState, iPendingNotifications.size()));

    while (!iPendingNotifications.empty())
    {
        if (iPendingNotifications[0]->notificationType ==
                TPV2WayNotificationInfo::EPV2WayCommandType)
        {
            cmd = (TPV2WayCmdInfo *) iPendingNotifications[0];

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                            (0, "CPV324m2Way::Run Calling CommandCompleted CmdType %d CmdId %d CmdStatus %d\n",
                             cmd->type, cmd->id, cmd->status));

            switch (cmd->type)
            {
                case PVT_COMMAND_INIT:
                    if ((cmd->status != PVMFSuccess) &&
                            (cmd->status != PVMFErrCancelled))
                    {
                        SetDefaults();
                    }
                    break;

                case PVT_COMMAND_RESET:
                    RemoveAudioEncPath();
                    RemoveVideoEncPath();
                    RemoveAudioDecPath();
                    RemoveVideoDecPath();
                    SetDefaults();
                    break;

                case PVT_COMMAND_CONNECT:
                    if ((cmd->status != PVMFSuccess) &&
                            (cmd->status != PVMFErrCancelled))
                    {
                        iMuxDatapath->ResetDatapath();
                        iCommNode.iNode->ThreadLogoff();
                        iCommNode.Clear();
                    }
                    break;

                case PVT_COMMAND_DISCONNECT:
                    iMuxDatapath->ResetDatapath();
                    iCommNode.iNode->ThreadLogoff();
                    iCommNode.Clear();
                    break;

                case PVT_COMMAND_ADD_DATA_SOURCE:
                    if (cmd->status == PVMFSuccess)
                    {
                        if (iAudioEncDatapath && (iAudioEncDatapath->GetState() == EOpened) &&
                                iVideoEncDatapath && (iVideoEncDatapath->GetState() == EOpened))
                        {
                            TSC_324m *tsc_node = (TSC_324m *)(iTscNode.iNode);
                            tsc_node->SetSkewReference(iVideoEncDatapath->GetTSCPort(),
                                                       iAudioEncDatapath->GetTSCPort());
                        }
                    }
                    else
                    {
                        if (cmd->status != PVMFErrCancelled)
                        {
                            RemoveAudioEncPath();
                            RemoveVideoEncPath();
                        }
                    }
                    break;

                case PVT_COMMAND_ADD_DATA_SINK:
                {
                    CPV2WayDecDataChannelDatapath* datapath = NULL;
                    if (cmd->iPvtCmdData == iAudioDecDatapath->GetChannelId())
                    {
                        datapath = iAudioDecDatapath;
                    }
                    else if (cmd->iPvtCmdData == iVideoDecDatapath->GetChannelId())
                    {
                        datapath = iVideoDecDatapath;
                    }
                    if (datapath == NULL)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                        (0, "CPV324m2Way::Run ERROR Failed to lookup dec datapath.\n"));
                        break;
                    }

                    if ((cmd->status != PVMFSuccess) &&
                            (cmd->status != PVMFErrCancelled))
                    {
                        if (datapath == iAudioDecDatapath)
                            RemoveAudioDecPath();
                        else if (datapath == iVideoDecDatapath)
                            RemoveVideoDecPath();
                        break;
                    }

                    if (!datapath->IsSkipComplete())
                        return;
                }
                break;

                case PVT_COMMAND_REMOVE_DATA_SOURCE:
                    RemoveAudioEncPath();
                    RemoveVideoEncPath();
                    break;

                case PVT_COMMAND_REMOVE_DATA_SINK:
                    RemoveAudioDecPath();
                    RemoveVideoDecPath();
                    break;

#if defined(PV_RECORD_TO_FILE_SUPPORT)
                case PVT_COMMAND_START_RECORD:
                    if ((cmd->status != PVMFSuccess) &&
                            (cmd->status != PVMFErrCancelled))
                    {
                        RemoveAudioRecPath();
                        RemoveVideoRecPath();
                    }
                    break;

                case PVT_COMMAND_STOP_RECORD:
                    RemoveAudioRecPath();
                    RemoveVideoRecPath();
                    break;
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
                case PVT_COMMAND_ADD_PREVIEW_SINK:
                    if ((cmd->status != PVMFSuccess) &&
                            (cmd->status != PVMFErrCancelled))
                    {
                        RemoveAudioPreviewPath();
                        RemoveVideoPreviewPath();
                    }
                    break;

                case PVT_COMMAND_REMOVE_PREVIEW_SINK:
                    RemoveAudioPreviewPath();
                    RemoveVideoPreviewPath();
                    break;

#endif
                case PVT_COMMAND_CANCEL_ALL_COMMANDS:
                    //Init cancelled
                    if (iState == EIdle)
                    {
                        SetDefaults();
                    }
                    if (iState == ESetup)
                    {
                        if (iMuxDatapath->GetState() == EClosed)
                        {
                            iMuxDatapath->ResetDatapath();
                            iCommNode.iNode->ThreadLogoff();
                            iCommNode.Clear();
                        }
                    }
                    break;

                default:
                    break;
            }

            {
                PVCmdResponse resp(cmd->id, cmd->contextData, cmd->status, NULL, 0);
                OSCL_TRY(error, iCmdStatusObserver->CommandCompleted(resp));
                OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                                     PVLOGMSG_ERR, (0, "CPV324m2Way::Run CommandCompletedL leave %d\n",
                                                    error)));
            }

            iPendingNotifications.erase(iPendingNotifications.begin());
            FreeCmdInfo(cmd);

        }
        else if (iPendingNotifications[0]->notificationType ==
                 TPV2WayNotificationInfo::EPV2WayEventType)
        {
            event = (TPV2WayEventInfo *) iPendingNotifications[0];

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                            (0, "CPV324m2Way::Run Calling HandleInformationalEventL EventType %d\n",
                             event->type));

            switch (event->type)
            {
                case PVT_INDICATION_CLOSE_TRACK:
                {
                    TPVChannelId* id = (TPVChannelId*)(event->localBuffer + 4);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                    (0, "CPV324m2Way::Run PVT_INDICATION_CLOSE_TRACK direction %d, channel id=%d",
                                     event->localBuffer[0], *id));
                    if (event->localBuffer[0] == INCOMING)
                    {
                        if (iAudioDecDatapath && (*id == iAudioDecDatapath->GetChannelId()))
                        {
                            RemoveAudioDecPath();
                        }
                        else if (iVideoDecDatapath && (*id == iVideoDecDatapath->GetChannelId()))
                        {
                            RemoveVideoDecPath();
                        }
                    }
                    else if (event->localBuffer[0] == OUTGOING)
                    {
                        if (iAudioEncDatapath && (*id == iAudioEncDatapath->GetChannelId()))
                        {
                            RemoveAudioEncPath();
                        }
                        else if (iVideoEncDatapath && (*id == iVideoEncDatapath->GetChannelId()))
                        {
                            RemoveVideoEncPath();
                        }
                    }

                    CheckState();
                }
                break;

                case PVT_INDICATION_DISCONNECT:
                    iMuxDatapath->ResetDatapath();
                    iCommNode.Clear();
                    break;

#if defined(PV_RECORD_TO_FILE_SUPPORT)
                case PVT_INDICATION_RECORDING_ERROR:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                    (0, "CPV324m2Way::Run PVT_INDICATION_RECORD_ERROR media %d\n",
                                     event->localBuffer[0]));
                    if (event->localBuffer[0] == PV_AUDIO)
                    {
                        RemoveAudioRecPath();
                    }
                    else
                    {
                        RemoveVideoRecPath();
                    }
                    break;
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)

                case PVT_INDICATION_PREVIEW_ERROR:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                    (0, "CPV324m2Way::Run PVT_INDICATION_PREVIEW_ERROR media %d\n",
                                     event->localBuffer[0]));
                    if (event->localBuffer[0] == PV_AUDIO)
                    {
                        RemoveAudioPreviewPath();
                    }
                    else
                    {
                        RemoveVideoPreviewPath();
                    }
                    break;
#endif
                default:
                    break;
            }

            {
                PVAsyncInformationalEvent aEvent(event->type, event->exclusivePtr,
                                                 &event->localBuffer[0], event->localBufferSize);
                OSCL_TRY(error, iInfoEventObserver->HandleInformationalEvent(aEvent));
                OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                                     PVLOGMSG_ERR, (0, "CPV324m2Way::Run HandleInformationalEventL leave %d\n",
                                                    error)));
            }

            iPendingNotifications.erase(iPendingNotifications.begin());
            FreeEventInfo(event);
        }
    }
}

void CPV324m2Way::Dispatch(TPV2WayCmdInfo* aCmdInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::Dispatch Appending command to queue CmdType %d CmdId %d CmdStatus %d\n",
                     aCmdInfo->type, aCmdInfo->id, aCmdInfo->status));
    if (aCmdInfo->status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::Dispatch Command failed\n"));
    }
    iPendingNotifications.push_back(aCmdInfo);
    RunIfNotReady();
}

void CPV324m2Way::Dispatch(TPV2WayEventInfo* aEventInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::Dispatch Appending event to queue event type %d\n",
                     aEventInfo->type));

    iPendingNotifications.push_back(aEventInfo);
    RunIfNotReady();
}


bool CPV324m2Way::IsNodeInList(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList,
                               PVMFNodeInterface* aNode)
{
    for (uint32 i = 0;i < aList.size();i++)
    {
        TPV2WayNode* lNode = aList[i];
        if (lNode && lNode->iNode == aNode)
            return true;
    }
    return false;
}

bool CPV324m2Way::IsSourceNode(PVMFNodeInterface* aNode)
{
    return IsNodeInList(iSourceNodes, aNode);
}

bool CPV324m2Way::IsSinkNode(PVMFNodeInterface* aNode)
{
    return IsNodeInList(iSinkNodes, aNode);
}

TPV2WayNode* CPV324m2Way::GetTPV2WayNode(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList,
        PVMFNodeInterface* aNode)
{
    for (uint32 i = 0;i < aList.size();i++)
    {
        TPV2WayNode* lNode = aList[i];
        if (lNode && lNode->iNode == aNode)
            return lNode;
    }
    return NULL;
}

TPV2WayNode* CPV324m2Way::RemoveTPV2WayNode(Oscl_Vector<TPV2WayNode*, OsclMemAllocator>& aList,
        PVMFNodeInterface* aNode)
{
    for (uint32 i = 0;i < aList.size();i++)
    {
        TPV2WayNode* lNode = aList[i];
        if (lNode && lNode->iNode == aNode)
        {
            aList[i] = 0;
            return lNode;
        }
    }
    return NULL;
}

// from PVMFNodeCmdEventObserver
void CPV324m2Way::NodeCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::NodeCommandCompleted status %d, context %x\n",
                     aResponse.GetCmdStatus(), aResponse.GetContext()));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::NodeCommandCompleted Command failed"));
    }
    CPV2WayNodeContextData *data = (CPV2WayNodeContextData *) aResponse.GetContext();
    TPV2WayNodeCmdInfo *info = FindPendingNodeCmd(data->iNode, aResponse.GetCmdId());

    data->iObserver->CommandHandler(info->type, aResponse);

    // check if node cmd response requires engine cmd response
    if (info->engineCmdInfo != NULL)
    {
        info->engineCmdInfo->status = aResponse.GetCmdStatus();
        Dispatch(info->engineCmdInfo);
    }

    //Remove the command from the pending list.
    RemovePendingNodeCmd(data->iNode, aResponse.GetCmdId());
}

// from PVMFNodeInfoEventObserver
void CPV324m2Way::HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleNodeInformationalEvent type %d\n",
                     aEvent.GetEventType()));

    if (aEvent.GetContext() == iTscNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent tsc node\n"));
        uint8 *buf = aEvent.GetLocalBuffer();
        switch (buf[0])
        {
            case PV_H324COMPONENT_H223:
                switch (buf[2])
                {
                    case INCOMING:
                    {
                        TPVChannelId id = CHANNEL_ID_UNKNOWN;
                        if (buf[1] == PV_MUX_COMPONENT_LOGICAL_CHANNEL)
                        {
                            id = *((TPVChannelId*)(buf + 4));

                            // See if error is in video datapath
                            if (id == iVideoDecDatapath->GetChannelId())
                            {
                                // request for I-frame
                                RequestRemoteIFrame(iVideoDecDatapath->GetTSCPort());
                            }
                        }
                    }
                    break;

                    case OUTGOING:
                        GenerateIFrame(iVideoEncDatapath->GetTSCPort());
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
    else if (aEvent.GetContext() == iCommNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent comm node\n"));
    }
#if defined(PV_RECORD_TO_FILE_SUPPORT)
    else if (aEvent.GetContext() == iFFComposerNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent ff composer node\n"));
    }
#endif
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    else if (aEvent.GetContext() == iPlayFromFileNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent playfromfile node\n"));
        TPV2WayEventInfo* event = NULL;
        switch (aEvent.GetEventType())
        {
            case PFF_NODE_INFO_EVENT_EOS_INFO_EVENT:
                if (!GetEventInfo(event))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "CPV324m2Way::HandleNodeInformationalEvent unable to notify app!\n"));
                    return;
                }
                event->type = PVT_INDICATION_PLAY_EOS;
                event->localBufferSize = 1;

                //Check which port received the EOS.
                if (iAudioPlayPort.GetPort() == aEvent.GetEventData())
                {
                    event->localBuffer[0] = PV_AUDIO;
                    Dispatch(event);
                }
                else if (iVideoPlayPort.GetPort() == aEvent.GetEventData())
                {
                    event->localBuffer[0] = PV_VIDEO;
                    Dispatch(event);
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                    (0, "CPV324m2Way::HandleNodeInformationalEvent unknown pff port for EOS %x\n",
                                     aEvent.GetEventData()));
                    FreeEventInfo(event);
                }

                break;

            default:
                break;
        }

    }
#endif
    else if (aEvent.GetContext() == iVideoDecNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent video dec node\n"));
    }
    else if (aEvent.GetContext() == iVideoParserNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent video parser node\n"));
    }
    else if (aEvent.GetContext() == iVideoEncNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent video encoder node\n"));
    }
    else if (iAudioEncDatapath && iAudioEncDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent audio enc datapath\n"));
    }
    else if (iAudioDecDatapath && iAudioDecDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent audio dec datapath\n"));
        PVMFEventType event = aEvent.GetEventType();
        if (event == PVMFInfoStartOfData)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::HandleNodeInformationalEvent audio dec datapath PVMFInfoStartOfData received, Clock started\n"));
        }
    }
    else if ((iVideoEncDatapath != NULL) &&
             (iVideoEncDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext())))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent video enc datapath\n"));
    }
    else if ((iVideoDecDatapath != NULL) &&
             (iVideoDecDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext())))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent video dec datapath\n"));
        PVMFEventType event = aEvent.GetEventType();
        if (event == PVMFInfoStartOfData)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::HandleNodeInformationalEvent video dec datapath PVMFInfoStartOfData received, Clock started\n"));
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::HandleNodeInformationalEvent unknown node!"));
    }
}

// from PVMFNodeErrorEventObserver
void CPV324m2Way::HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleNodeErrorEvent type %d\n", aEvent.GetEventType()));

    if (aEvent.GetContext() == iTscNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                        (0, "CPV324m2Way::HandleNodeErrorEvent tsc node\n"));

        switch (iState)
        {
            case EDisconnecting:
                CheckState();
                break;

            case EConnecting:
            case EConnected:
                //InitiateDisconnect();
                break;

            default:
                break;
        }
    }
    else if (aEvent.GetContext() == iCommNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent comm node\n"));
        switch (iState)
        {
            case EDisconnecting:
                CheckState();
                break;

            case EConnecting:
            case EConnected:
                InitiateDisconnect();
                break;

            default:
                break;
        }
    }
#if defined(PV_RECORD_TO_FILE_SUPPORT)
    else if (aEvent.GetContext() == iFFComposerNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent ff composer node state %d\n",
                         iRecordFileState));
        switch (iRecordFileState)
        {
            case File2WayResetting:
                CheckRecordFileState();
                break;

            case File2WayInitializing:
            case File2WayInitialized:
                InitiateResetRecordFile();
                break;

            default:
                break;
        }
    }
#endif
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    else if (aEvent.GetContext() == iPlayFromFileNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent playfromfile node state %d\n",
                         iPlayFromFileNode->GetState()));
        switch (iPlayFileState)
        {
            case File2WayResetting:
                CheckPlayFileState();
                break;

            case File2WayInitializing:
            case File2WayInitialized:
                InitiateResetPlayFile();
                break;

            default:
                break;
        }
    }
#endif
    else if (iVideoEncDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent video enc datapath\n"));
        iVideoEncDatapath->SetCmd(NULL);
    }
    else if (iVideoDecDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent video dec datapath\n"));
        iVideoDecDatapath->SetCmd(NULL);
    }
    else if (iAudioEncDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent audio enc datapath\n"));

        iAudioEncDatapath->UseFilePlayPort(false);
        iAudioEncDatapath->SetSourceInputPort(NULL);

#ifndef PV_DISABLE_DEVSOUNDNODES
        switch (aEvent.GetEventType())
        {

            case PVMF_DEVSOUND_ERR_PORT_GETDATA_ERROR:
            case PVMF_DEVSOUND_ERR_PORT_PUTDATA_ERROR:
            case PVMF_DEVSOUND_ERR_SOURCE_SINK_EVENT_ERROR:
            case PVMF_DEVSOUND_ERR_BITSTREAM_ERROR:
            case PVMF_DEVSOUND_ERR_PORT_FRAME_TRANSFER_ERROR:
            case PVMF_DEVSOUND_ERR_SOURCE_SINK_FRAME_TRANSFER_ERROR:
            case PVMF_DEVSOUND_ERR_DATA_PROCESSING_ERROR:
            case PVMF_DEVSOUND_ERR_RECORD_DATA_LOST:
            case PVMF_DEVSOUND_ERR_MEMPOOL_ALLOC_ERROR:
            case PVMF_DEVSOUND_ERR_MEDIADATAALLOC_ALLOC_ERROR:
                //data dropped, recording will continue
                break;

            default:
                iAudioEncDatapath->SetCmd(NULL);
                break;
        }
#else
        iAudioEncDatapath->SetCmd(NULL);
#endif
    }
    else if (iAudioDecDatapath->IsNodeInDatapath((PVMFNodeInterface *) aEvent.GetContext()))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::HandleNodeErrorEvent audio dec datapath\n"));

#ifndef PV_DISABLE_DEVSOUNDNODES
        switch (aEvent.GetEventType())
        {
            case PVMF_DEVSOUND_ERR_PORT_GETDATA_ERROR:
            case PVMF_DEVSOUND_ERR_PORT_PUTDATA_ERROR:
            case PVMF_DEVSOUND_ERR_SOURCE_SINK_EVENT_ERROR:
            case PVMF_DEVSOUND_ERR_BITSTREAM_ERROR:
            case PVMF_DEVSOUND_ERR_PORT_FRAME_TRANSFER_ERROR:
            case PVMF_DEVSOUND_ERR_SOURCE_SINK_FRAME_TRANSFER_ERROR:
            case PVMF_DEVSOUND_ERR_DATA_PROCESSING_ERROR:
            case PVMF_DEVSOUND_ERR_MEMPOOL_ALLOC_ERROR:
            case PVMF_DEVSOUND_ERR_MEDIADATAALLOC_ALLOC_ERROR:
                //data dropped, playback will continue
                break;

            default:
                iAudioDecDatapath->SetCmd(NULL);
                break;
        }
#else
        iAudioDecDatapath->SetCmd(NULL);

#endif
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::HandleNodeErrorEvent unknown node!"));
    }
}

void CPV324m2Way::CommandHandler(PV2WayNodeCmdType aType,
                                 const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "CPV324m2Way::CommandHandler, state %d, type %d\n", iState, aType));

    CPV2WayNodeContextData *data = (CPV2WayNodeContextData *) aResponse.GetContext();

    if (data->iNode == iCommNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler comm node\n"));
        HandleCommNodeCmd(aType, aResponse);
    }
    else if (data->iNode == iTscNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler TSC node\n"));
        HandleTscNodeCmd(aType, aResponse);
    }
    else if (data->iNode == iVideoDecNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler video decoder node\n"));
        HandleVideoDecNodeCmd(aType, aResponse);
    }
    else if (data->iNode == iVideoEncNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler video encoder node\n"));
        HandleVideoEncNodeCmd(aType, aResponse);
    }
    else if (IsSourceNode(data->iNode))
    {
        /* Do Add Data Source Here */
        TPV2WayNode *source_node = GetTPV2WayNode(iSourceNodes, data->iNode);
        OSCL_ASSERT(source_node);
        DoAddDataSource(*source_node, aResponse);
    }
    else if (IsSinkNode(data->iNode))
    {
        TPV2WayNode *sink_node = GetTPV2WayNode(iSinkNodes, data->iNode);
        OSCL_ASSERT(sink_node);
        HandleSinkNodeCmd(aType, aResponse, sink_node);
    }
    else if (data->iNode == iAudioEncNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler audio encoder node\n"));
        HandleAudioEncNodeCmd(aType, aResponse);
    }
#if defined(PV_RECORD_TO_FILE_SUPPORT)
    else if (data->iNode == iFFComposerNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler ff composer node\n"));
        HandleFFComposerNodeCmd(aType, aResponse);
    }
#endif
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
    else if (data->iNode == iPlayFromFileNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler pff node\n"));
        HandlePFFNodeCmd(aType, aResponse);
    }
    else if (data->iNode == iAudioSrcNode.iNode)
    {
        // This will happen only after the node had been added to a datapath succussfully
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CommandHandler audio src node\n"));
        HandleAudioSrcNodeCmd(aType, aResponse);
    }
#endif
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::CommandHandler unknown node!"));
    }
}

PVMFStatus CPV324m2Way::ConfigureNode(CPVDatapathNode *aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "CPV324m2Way::ConfigureNode, state %d\n", iState));

    PVMFNodeInterface *node = aNode->iNode.iNode;

    if (node == iTscNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring tsc node\n"));
        return PVMFSuccess;
    }
    else if (node == iCommNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring comm node\n"));
        return PVMFSuccess;
    }
    else if (node == iVideoEncNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "CPV324m2Way::ConfigureNode configuring video enc node\n"));


        PVMp4H263EncExtensionInterface *ptr =
            (PVMp4H263EncExtensionInterface *) iVideoEncNodeInterface.iInterface;


        uint32 bitrate_bps_100 = VIDEO_ENCODER_BITRATE / 100;
        PVMFVideoResolution aVideoResolution(VIDEO_ENCODER_WIDTH,
                                             VIDEO_ENCODER_HEIGHT);
        double aFrameRate = VIDEO_ENCODER_FRAME_RATE;

        LogicalChannelInfo* lcn_info = NULL;

        if (aNode->iOutputPort.iPortPair->iDestPort.GetStatus() != EHasPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::ConfigureNode waiting for tsc port to determine video codec type.\n"));
            return PVMFPending;
        }
#ifndef NO_2WAY_324
        if (iTerminalType == PV_324M)
        {
            OsclAny * tempInterface = NULL;
            aNode->iOutputPort.iPortPair->iDestPort.GetPort()->QueryInterface(
                PVH324MLogicalChannelInfoUuid, tempInterface);
            lcn_info = OSCL_STATIC_CAST(LogicalChannelInfo*, tempInterface);
            if (lcn_info == NULL)
            {
                return PVMFFailure;
            }
            PVMFFormatType aFormatType = lcn_info->GetFormatType();

            if (iTSC324mInterface != NULL)
            {
                CPvtTerminalCapability* remote_caps = iTSC324mInterface->GetRemoteCapability();

                if (remote_caps)
                {
                    for (uint16 i = 0; i < remote_caps->GetNumCapabilityItems();i++)
                    {
                        CPvtMediaCapability * RemoteCapItem =
                            remote_caps->GetCapabilityItem(i);
                        if (RemoteCapItem->GetFormatType() == aFormatType)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                            (0, "CPV324m2Way::ConfigureNode Found codec match for bitrate - capability(%d), default(%d)", remote_caps->GetCapabilityItem(i)->GetBitrate(), bitrate_bps_100));
                            if (RemoteCapItem->GetBitrate() < bitrate_bps_100)
                            {
                                bitrate_bps_100 = RemoteCapItem->GetBitrate();
                            }

                            PVMFVideoResolution *video_resolution;
                            uint32 frame_rate;
                            video_resolution = ((CPvtVideoCapability*)RemoteCapItem)->GetMaxResolution(frame_rate);
                            if ((video_resolution->width < aVideoResolution.width) &&
                                    (video_resolution->height <  aVideoResolution.height))
                            {
                                aVideoResolution.width = video_resolution->width;
                                aVideoResolution.height = video_resolution->height;

                            }

                            if (frame_rate < aFrameRate)
                                aFrameRate = frame_rate;

                            break;
                        }
                    }
                }
            }
        }
#endif
        ptr->SetNumLayers(1);
        ptr->SetOutputBitRate(0, bitrate_bps_100*100);
        ptr->SetOutputFrameSize(0, aVideoResolution.width, aVideoResolution.height);
        ptr->SetOutputFrameRate(0, (float)aFrameRate);

        ptr->SetSegmentTargetSize(0, VIDEO_ENCODER_SEGMENT_SIZE);
        ptr->SetRateControlType(0, VIDEO_ENCODER_RATE_CONTROL);
        ptr->SetDataPartitioning(VIDEO_ENCODER_DATA_PARTITIONING);
        ptr->SetRVLC(VIDEO_ENCODER_RVLC);
        ptr->SetIFrameInterval(VIDEO_ENCODER_I_FRAME_INTERVAL);
        return PVMFSuccess;

    }
    else if (node == iVideoDecNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring video dec node\n"));
        return PVMFSuccess;
    }
#ifndef PV_DISABLE_DEVSOUNDNODES
    else if (node == iAudioSrcNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring audio node\n"));
        PVMFPortProperty prop;
        if (aNode->iOutputPort.iPortPair->iDestPort.GetStatus() != EHasPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::ConfigureNode waiting for tsc port to determine audio codec type.\n"));
            return PVMFPending;
        }

        aNode->iOutputPort.iPortPair->iDestPort.GetPort()->Query(prop);

        //Set video encoder parameters
        if (prop.iFormatType == PVMF_MIME_AMR_IF2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::ConfigureNode AMR IF2.\n"));
            //Can't set audio codec type yet
        }
        else if (prop.iFormatType == PVMF_MIME_G723)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::ConfigureNode G723.\n"));
            //Can't set audio codec type yet
        }

        uint32 bitrate_bps = MAX_AUDIO_BITRATE;

        CPvtTerminalCapability* remote_caps = iTscNode.node->GetRemoteCapability();
        if (remote_caps)
        {
            for (uint16 i = 0; i < remote_caps->GetNumCapabilityItems(); i++)
            {
                if ((remote_caps->GetCapabilityItem(i)->GetFormatType() == PVMF_AMR_IF2 ||
                        remote_caps->GetCapabilityItem(i)->GetFormatType() == PVMF_MIME_G723) &&
                        remote_caps->GetCapabilityItem(i)->GetBitrate() < bitrate_bps / 100)
                {
                    bitrate_bps = remote_caps->GetCapabilityItem(i)->GetBitrate() * 100;
                }
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode audio bitrate %d.\n", bitrate_bps));

        // set max bitrate in devsound
        PVDevSoundOptions options;
        ((PVDevSoundNodeBase *) iAudioSrcNode)->GetOptions(options);
        if (bitrate_bps >= 12200)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate122;
        }
        else if (bitrate_bps >= 10200)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate102;
        }
        else if (bitrate_bps >= 7950)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate795;
        }
        else if (bitrate_bps >= 7400)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate74;
        }
        else if (bitrate_bps >= 6700)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate67;
        }
        else if (bitrate_bps >= 5900)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate59;
        }
        else if (bitrate_bps >= 5150)
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate515;
        }
        else
        {
            options.iRecordAmrBitrate = PVMFAmrEncBitrate475;
        }

        ((PVDevSoundNodeBase *) iAudioSrcNode)->UpdateOptions(options);

        return PVMFSuccess;
    }
#endif
    else if (node == iAudioEncNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring audio enc node\n"));
        //PVMFPortProperty prop;
#ifdef PV2WAY_USE_OMX
        PVAudioEncExtensionInterface *ptr =
            (PVAudioEncExtensionInterface *) iAudioEncNodeInterface.iInterface;
#else
        PVAMREncExtensionInterface *ptr =
            (PVAMREncExtensionInterface *) iAudioEncNodeInterface.iInterface;
#endif // PV2WAY_USE_OMX
        if (aNode->iOutputPort.iPortPair->iDestPort.GetStatus() != EHasPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "CPV324m2Way::ConfigureNode waiting for tsc port to determine audio codec type.\n"));
            return PVMFPending;
        }

        //aNode->iOutputPort.iPortPair->iDestPort.GetPort()->Query(prop);

        //ptr->SetOutputFormat(PVMF_AMR_IF2);
        //ptr->SetInputSamplingRate(KSamplingRate);
        //ptr->SetInputBitsPerSample(KBitsPerSample);
        //ptr->SetInputNumChannels(KNumChannels);
        ptr->SetOutputBitRate(GSM_AMR_12_2);
        ptr->SetMaxNumOutputFramesPerBuffer(KNumPCMFrames);

        return PVMFSuccess;

    }
    else if (node == iAudioDecNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring audio dec node\n"));
        return PVMFSuccess;
    }
    else if (node == iVideoParserNode.iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::ConfigureNode configuring video parser node\n"));
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::ConfigureNode unknown node\n"));
        return PVMFFailure;
    }
    return PVMFFailure;
}

// Implementations of TSC Observer virtuals
void CPV324m2Way::ConnectComplete(PVMFStatus status)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "CPV324m2Way::ConnectComplete, state %d, status %d\n", iState, status));
    if (status == PVMFSuccess)
    {
        iIsStackConnected = true;
    }
    else
    {
        iIsStackConnected = false;
        SetState(EDisconnecting);
    }

    CheckState();
}

void CPV324m2Way::InternalError()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "CPV324m2Way::InternalError, state %d\n", iState));

    switch (iState)
    {
        case EDisconnecting:
            iAudioDecDatapath->TSCPortClosed();
            iAudioEncDatapath->TSCPortClosed();
            iVideoDecDatapath->TSCPortClosed();
            iVideoEncDatapath->TSCPortClosed();

            CheckState();
            break;

        case EConnecting:
        case EConnected:
            iAudioDecDatapath->TSCPortClosed();
            iAudioEncDatapath->TSCPortClosed();
            iVideoDecDatapath->TSCPortClosed();
            iVideoEncDatapath->TSCPortClosed();

            InitiateDisconnect();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::InternalError invalid state\n"));
            break;
    }
}

void CPV324m2Way::DisconnectRequestReceived()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::DisconnectRequestReceived state %d\n", iState));

    iIsStackConnected = false;
    if (iDisconnectInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::DisconnectRequestReceived Doing nothing as  disconnect is in progress"));
    }
    else
    {

        switch (iState)
        {
            case EDisconnecting:
                iAudioDecDatapath->TSCPortClosed();
                iAudioEncDatapath->TSCPortClosed();
                iVideoDecDatapath->TSCPortClosed();
                iVideoEncDatapath->TSCPortClosed();

                CheckState();
                break;

            case EConnecting:
            case EConnected:
                iAudioDecDatapath->TSCPortClosed();
                iAudioEncDatapath->TSCPortClosed();
                iVideoDecDatapath->TSCPortClosed();
                iVideoEncDatapath->TSCPortClosed();


                iRemoteDisconnectTimer->SetObserver(this);
                iRemoteDisconnectTimer->Request(REMOTE_DISCONNECT_TIMER_ID, REMOTE_DISCONNECT_TIMER_ID,
                                                REMOTE_DISCONNECT_TIMER_VALUE, this);


                //We do InitiateDisconnect() once above timer expires
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::DisconnectRequestReceived invalid state\n"));
                break;
        }
    }
}

PVMFStatus CPV324m2Way::EstablishChannel(TPVDirection aDir,
        TPVChannelId aId,
        PVCodecType_t aCodec,
        uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::EstablishChannel aDir=%d, channel id=%d, codec %d\n",
                     aDir, aId, aCodec));

    PV2WayMediaType media_type = ::GetMediaType(aCodec);
    OSCL_ASSERT(media_type == PV_AUDIO || media_type == PV_VIDEO);
    PVMFFormatType aFormatType = PVCodecTypeToPVMFFormatType(aCodec);
    PVMFFormatType aAppFormatType = PVMF_MIME_FORMAT_UNKNOWN;

    TPV2WayEventInfo* aEvent = NULL;
    if (!GetEventInfo(aEvent))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::EstablishChannel Memory allocation failed!\n"));
        return PVMFErrNoMemory;
    }

    Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class>* codec_list = NULL;
    Oscl_Map<PVMFFormatType, PVMFFormatType, OsclMemAllocator, pvmf_format_type_key_compare_class>* app_format_for_engine_format = NULL;

    CPV2WayDataChannelDatapath* datapath = NULL;

    if (aDir == INCOMING)
    {
        if (media_type == PV_AUDIO)
        {
            if (!(iAudioDecDatapath))
            {
                iAudioDecDatapath = CPV2WayDecDataChannelDatapath::NewL(iLogger,
                                    aFormatType, this);

            }

            AddAudioDecoderNode();
            uint32 audioLatency = LookupMioLatency(PVCodecTypeToPVMFFormatType(aCodec), true);
            ((TSC_324m*)(iTscNode.iNode))->SetMioLatency((audioLatency + iAudioDatapathLatency), true);

            datapath = iAudioDecDatapath;
            codec_list = &iIncomingAudioCodecs;
        }
        else if (media_type == PV_VIDEO)
        {
            if (!(iVideoDecDatapath))
            {
                iVideoDecDatapath = CPV2WayDecDataChannelDatapath::NewL(iLogger, aFormatType, this);
            }
            iVideoDecDatapath->SetFormatSpecificInfo(aFormatSpecificInfo, (uint16)aFormatSpecificInfoLen);

            AddVideoDecoderNode(aFormatSpecificInfo, aFormatSpecificInfoLen);
            uint32 videoLatency = LookupMioLatency(PVCodecTypeToPVMFFormatType(aCodec), false);
            ((TSC_324m*)(iTscNode.iNode))->SetMioLatency((videoLatency + iVideoDatapathLatency), false);

            datapath = iVideoDecDatapath;
            codec_list = &iIncomingVideoCodecs;
        }
        app_format_for_engine_format = &iAppFormatForEngineFormatIncoming;
        iClock.Start();
    }

    else
    {
        if (media_type == PV_AUDIO)
        {
            if (!(iAudioEncDatapath))
            {
                iAudioEncDatapath = CPV2WayEncDataChannelDatapath::NewL(iLogger,
                                    aFormatType, this);
            }

            AddAudioEncoderNode();

            datapath = iAudioEncDatapath;
            codec_list = &iOutgoingAudioCodecs;
        }
        else if (media_type == PV_VIDEO)
        {
            if (!(iVideoEncDatapath))
            {
                iVideoEncDatapath = CPV2WayEncDataChannelDatapath::NewL(iLogger, aFormatType, this);
            }
            iVideoEncDatapath->SetFormatSpecificInfo(aFormatSpecificInfo, (uint16)aFormatSpecificInfoLen);

            AddVideoEncoderNode();
            datapath = iVideoEncDatapath;
            codec_list = &iOutgoingVideoCodecs;
        }
        app_format_for_engine_format = &iAppFormatForEngineFormatOutgoing;
    }
    Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class>::iterator it = codec_list->find(aFormatType);

    if (it == codec_list->end())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::EstablishChannel Failed to lookup codec=%d\n", aCodec));
        return PVMFFailure;
    }

    if ((*it).second.iPriority == ENG)
    {
        // Set the app format to the stored raw format type
        aAppFormatType = (*app_format_for_engine_format)[aFormatType];
    }
    else
    {
        // Set the app format to the compressed type
        aAppFormatType = aFormatType;
    }
    datapath->SetFormat(aFormatType);
    datapath->SetSourceSinkFormat(aAppFormatType);

    // Send the informational event to the app
    aEvent->localBuffer[0] = (uint8) media_type;
    // bytes 1,2,3 are unused
    *((TPVChannelId*)(aEvent->localBuffer + 4)) = aId;
    aEvent->localBufferSize = 8;

    PVEventType aEventType = (aDir == INCOMING) ? PVT_INDICATION_INCOMING_TRACK : PVT_INDICATION_OUTGOING_TRACK;
    PVUuid puuid = PV2WayTrackInfoInterfaceUUID;
    PV2WayTrackInfoInterface* pTrackInfo = OSCL_NEW(PV2WayTrackInfoImpl,
                                           (aAppFormatType, aFormatSpecificInfo, aFormatSpecificInfoLen, aEventType, puuid));
    PVAsyncInformationalEvent infoEvent(aEventType, NULL,
                                        OSCL_STATIC_CAST(PVInterface*, pTrackInfo), NULL,
                                        aEvent->localBuffer, aEvent->localBufferSize);
    if (iInfoEventObserver != NULL)
    {
        iInfoEventObserver->HandleInformationalEvent(infoEvent);
    }
    pTrackInfo->removeRef();

    return EPVT_Success;
}

void CPV324m2Way::OutgoingChannelEstablished(TPVChannelId aId,
        PVCodecType_t aCodec,
        uint8* aFormatSpecificInfo,
        uint32 aFormatSpecificInfoLen)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::OutgoingChannelEstablished id=%d, codec=%d, fsi=%x, fsi_len=%d",
                     aId, aCodec, aFormatSpecificInfo, aFormatSpecificInfoLen));
    EstablishChannel(OUTGOING, aId, aCodec, aFormatSpecificInfo, aFormatSpecificInfoLen);
}

TPVStatusCode CPV324m2Way::IncomingChannel(TPVChannelId aId,
        PVCodecType_t aCodec,
        uint8* aFormatSpecificInfo,
        uint32 aFormatSpecificInfoLen)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::IncomingChannel channel id=%d, codec %d\n",
                     aId, aCodec));
    return EstablishChannel(INCOMING, aId, aCodec, aFormatSpecificInfo, aFormatSpecificInfoLen);
}

bool CPV324m2Way::GetEventInfo(TPV2WayEventInfo*& event)
{
    int32 error = 0;
    OSCL_TRY(error, event = GetEventInfoL());
    OSCL_FIRST_CATCH_ANY(error,
                         return false);
    return true;
}

void CPV324m2Way::ChannelClosed(TPVDirection direction,
                                TPVChannelId id,
                                PVCodecType_t codec,
                                PVMFStatus status)
{
    OSCL_UNUSED_ARG(status);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::ChannelClosed id %d, codec %d, direction %d\n", id, codec, direction));
    PV2WayMediaType media_type = ::GetMediaType(codec);
    TPV2WayEventInfo* event = NULL;
    bool track_closed = false;
    // Send the closing track indication
    if (!GetEventInfo(event))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::ChannelClosed unable to allocate memory"));
        return;
    }

    event->type = PVT_INDICATION_CLOSING_TRACK;
    event->localBufferSize = 8;
    event->localBuffer[0] = (uint8)direction;
    // bytes 1,2,3 are unused
    *((TPVChannelId*)(event->localBuffer + 4)) = id;
    Dispatch(event);

    switch (media_type)
    {
        case PV_AUDIO:
            switch (direction)
            {
                case INCOMING:
                    if (iAudioDecDatapath)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                        (0, "CPV324m2Way::ChannelClosed audio dec path state %d, id %d\n",
                                         iAudioDecDatapath->GetState(), iAudioDecDatapath->GetChannelId()));
                        if (iAudioDecDatapath->GetChannelId() == CHANNEL_ID_UNKNOWN)
                        {
                            track_closed = true;
                        }
                        else if (id == iAudioDecDatapath->GetChannelId())
                        {
                            switch (iAudioDecDatapath->GetState())
                            {
                                case EClosing:
                                    break;
                                case EClosed:
                                    track_closed = true;
                                    break;
                                default:
                                    iAudioDecDatapath->SetCmd(NULL);
                                    break;
                            }
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                            (0, "CPV324m2Way::ChannelClosed ERROR Channel id mismatch id=%d, datapath id=%d\n",
                                             id, iAudioDecDatapath->GetChannelId()));
                        }
                    }
                    break;

                case OUTGOING:
                    if (iAudioEncDatapath)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                        (0, "CPV324m2Way::ChannelClosed audio enc path state %d, id %d\n",
                                         iAudioEncDatapath->GetState(), iAudioEncDatapath->GetChannelId()));
                        if (iAudioEncDatapath->GetChannelId() == CHANNEL_ID_UNKNOWN)
                        {
                            track_closed = true;
                        }
                        else if (id == iAudioEncDatapath->GetChannelId())
                        {
                            switch (iAudioEncDatapath->GetState())
                            {
                                case EClosing:
                                    break;
                                case EClosed:
                                    track_closed = true;
                                    break;
                                default:
                                    iAudioEncDatapath->SetCmd(NULL);
                                    break;
                            }
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                            (0, "CPV324m2Way::ChannelClosed ERROR Channel id mismatch id=%d, datapath id=%d\n",
                                             id, iAudioEncDatapath->GetChannelId()));
                        }
                    }
                    break;

                default:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "CPV324m2Way::ChannelClosed unknown audio direction %d\n",
                                     direction));
                    break;
            }
            break;
        case PV_VIDEO:
            switch (direction)
            {
                case INCOMING:
                    if (iVideoDecDatapath)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                        (0, "CPV324m2Way::ChannelClosed video dec path state %d, id %d\n",
                                         iVideoDecDatapath->GetState(),
                                         iVideoDecDatapath->GetChannelId()));
                        if (iVideoDecDatapath->GetChannelId() == CHANNEL_ID_UNKNOWN)
                        {
                            track_closed = true;
                        }
                        else if (id == iVideoDecDatapath->GetChannelId())
                        {
                            switch (iVideoDecDatapath->GetState())
                            {
                                case EClosing:
                                    break;
                                case EClosed:
                                    track_closed = true;
                                    break;
                                default:
                                    iVideoDecDatapath->SetCmd(NULL);
                                    break;
                            }
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                            (0, "CPV324m2Way::ChannelClosed ERROR Channel id mismatch id=%d, datapath id=%d\n",
                                             id, iVideoDecDatapath->GetChannelId()));
                        }
                    }
                    break;

                case OUTGOING:
                    if (iVideoEncDatapath)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                        (0, "CPV324m2Way::ChannelClosed video enc path state %d, id %d\n",
                                         iVideoEncDatapath->GetState(),
                                         iVideoEncDatapath->GetChannelId()));
                        if (iVideoEncDatapath->GetChannelId() == CHANNEL_ID_UNKNOWN)
                        {
                            track_closed = true;
                        }
                        else if (id == iVideoEncDatapath->GetChannelId())
                        {
                            switch (iVideoEncDatapath->GetState())
                            {
                                case EClosing:
                                    break;
                                case EClosed:
                                    track_closed = true;
                                    break;
                                default:
                                    iVideoEncDatapath->SetCmd(NULL);
                                    break;
                            }
                        }
                        else
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                            (0, "CPV324m2Way::ChannelClosed ERROR Channel id mismatch id=%d, datapath id=%d\n",
                                             id, iVideoEncDatapath->GetChannelId()));
                        }
                    }
                    break;

                default:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "CPV324m2Way::ChannelClosed unknown video direction %d\n",
                                     direction));
                    break;
            }
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::ChannelClosed unknown media type %d\n",
                             media_type));
            break;
    }

    if (!track_closed)
        return;

    if (!GetEventInfo(event))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::ChannelClosed unable to allocate memory"));
        return;
    }
    event->type = PVT_INDICATION_CLOSE_TRACK;
    event->localBufferSize = 8;
    event->localBuffer[0] = (uint8)direction;
    // bytes 1,2,3 are unused
    *((TPVChannelId*)(event->localBuffer + 4)) = id;
    Dispatch(event);
}

void CPV324m2Way::RequestFrameUpdate(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RequestFrameUpdate\n"));
    if (iVideoEncDatapath)
    {
        GenerateIFrame(aPort);
    }
}


void  CPV324m2Way::TimeoutOccurred(int32 timerID,
                                   int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timeoutInfo);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::TimeoutOccurred id %d, info %d\n", timerID, timeoutInfo));

    if (timerID == IFRAME_REQ_TIMERID)
    {
        isIFrameReqTimerActive = false;
    }
    else if ((timerID == END_SESSION_TIMER_ID) || (timerID == REMOTE_DISCONNECT_TIMER_ID))
    {
        // Cancel out both timers if any one expires, as both do InitiateDisconnect()
        if (iEndSessionTimer)
        {
            iEndSessionTimer->Cancel(END_SESSION_TIMER_ID);
        }

        if (iRemoteDisconnectTimer)
        {
            iRemoteDisconnectTimer->Cancel(REMOTE_DISCONNECT_TIMER_ID);
        }

        InitiateDisconnect();
    }

#if defined(PV_RECORD_TO_FILE_SUPPORT)
    if (timerID == RECORDED_FILESIZE_NOTIFICATION_TIMERID)
    {
        int32 error;
        uint32 fileSize;
        TPV2WayEventInfo* aEvent = NULL;
        PvmfComposerSizeAndDurationInterface *ptr =
            (PvmfComposerSizeAndDurationInterface *) iFFSizeAndDuration.iInterface;

        if (!GetEventInfo(aEvent))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::TimeoutOccurred unable to notify app of recorded filesize!\n"));
            return;
        }
        aEvent->type = PVT_INDICATION_RECORDED_FILE_SIZE;

        ptr->GetFileSize(fileSize);
        *((uint32 *)(aEvent->localBuffer)) = fileSize;

        aEvent->localBufferSize = sizeof(uint32);
        Dispatch(aEvent);
    }
#endif
}

TPV2WayCmdInfo *CPV324m2Way::GetCmdInfoL()
{
    if (iFreeCmdInfo.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::GetFreeCmdInfo unable to allocate cmd info!"));
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    else
    {
        TPV2WayCmdInfo *cmd = (TPV2WayCmdInfo *)iFreeCmdInfo[0];
        iFreeCmdInfo.erase(iFreeCmdInfo.begin());
        return cmd;
    }

    return NULL;
}

void CPV324m2Way::FreeCmdInfo(TPV2WayCmdInfo *info)
{
    info->Clear();
    iFreeCmdInfo.push_back(info);
}

TPV2WayEventInfo *CPV324m2Way::GetEventInfoL()
{
    if (iFreeEventInfo.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::GetFreeEventInfo unable to allocate event info!"));
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    else
    {
        TPV2WayEventInfo *cmd = (TPV2WayEventInfo *)iFreeEventInfo[0];
        iFreeEventInfo.erase(iFreeEventInfo.begin());
        return cmd;
    }

    return NULL;
}

void CPV324m2Way::FreeEventInfo(TPV2WayEventInfo *info)
{
    info->Clear();
    iFreeEventInfo.push_back(info);
}

PVMFCommandId CPV324m2Way::SendNodeCmdL(PV2WayNodeCmdType aCmd,
                                        TPV2WayNode *aNode,
                                        CPV2WayNodeCommandObserver *aObserver,
                                        void *aParam,
                                        TPV2WayCmdInfo *a2WayCmdInfo)
{
    int32 error = 0;
    PVMFCommandId id = 0;
    TPV2WayNodeCmdInfo *info;
    PVMFNodeInterface * nodeIFace = (PVMFNodeInterface *)aNode->iNode;
    PvmfNodesSyncControlInterface* ptr = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::SendNodeCmdL state %d, cmd %d, session %d\n",
                     iState, aCmd, aNode->iSessionId));

    if (aNode == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::SendNodeCmdL node ptr is null!\n"));
        OSCL_LEAVE(PVMFErrArgument);
    }

    if (iFreeNodeCmdInfo.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPV324m2Way::SendNodeCmdL unable to allocate node command info!\n"));
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    info = (TPV2WayNodeCmdInfo *)iFreeNodeCmdInfo[0];
    iFreeNodeCmdInfo.erase(iFreeNodeCmdInfo.begin());

    info->type = aCmd;
    info->context.iObserver = aObserver;
    info->context.iNode = nodeIFace;
    info->engineCmdInfo = a2WayCmdInfo;

    PVMFSessionId sessionId = aNode->GetSessionId();

    switch (aCmd)
    {
        case PV2WAY_NODE_CMD_QUERY_UUID:
            if (aParam != NULL)
            {
                TPV2WayNodeQueryUuidParams *queryParam = (TPV2WayNodeQueryUuidParams *) aParam;
                OSCL_TRY(error, id = nodeIFace->QueryUUID(sessionId,
                                     queryParam->mimetype, *queryParam->iUuids,
                                     true, (OsclAny *) & info->context));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::SendNodeCmdL RequestPort param is null!\n"));
                error = PVMFErrArgument;
            }
            break;

        case PV2WAY_NODE_CMD_QUERY_INTERFACE:
            if (aParam != NULL)
            {
                TPV2WayNodeQueryInterfaceParams *queryParam =
                    (TPV2WayNodeQueryInterfaceParams *) aParam;
                OSCL_TRY(error, id = nodeIFace->QueryInterface(sessionId,
                                     *queryParam->iUuid, *queryParam->iInterfacePtr,
                                     (OsclAny *) & info->context));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::SendNodeCmdL RequestPort param is null!\n"));
                error = PVMFErrArgument;
            }
            break;

        case PV2WAY_NODE_CMD_INIT:
            info->context.iContextData = aParam;
            OSCL_TRY(error, id = nodeIFace->Init(sessionId,
                                                 (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_REQUESTPORT:
            if (aParam != NULL)
            {
                OSCL_HeapString<OsclMemAllocator> mimeType;
                TPV2WayNodeRequestPortParams *params = (TPV2WayNodeRequestPortParams *) aParam;
                mimeType = params->format.getMIMEStrPtr();
                //Get mime string from format type
                OSCL_TRY(error, id = nodeIFace->RequestPort(sessionId,
                                     params->portTag, &mimeType, (OsclAny *) & info->context));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::SendNodeCmdL RequestPort param is null!\n"));
                error = PVMFErrArgument;
            }
            break;

        case PV2WAY_NODE_CMD_PREPARE:
            OSCL_TRY(error, id = nodeIFace->Prepare(sessionId,
                                                    (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_START:
            OSCL_TRY(error, id = nodeIFace->Start(sessionId,
                                                  (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_PAUSE:
            OSCL_TRY(error, id = nodeIFace->Pause(sessionId,
                                                  (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_STOP:
            OSCL_TRY(error, id = nodeIFace->Stop(sessionId,
                                                 (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_RELEASEPORT:
            if (aParam != NULL)
            {
                OSCL_TRY(error, id = nodeIFace->ReleasePort(sessionId,
                                     *((PVMFPortInterface *) aParam), (OsclAny *) & info->context));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::SendNodeCmdL ReleasePort param is null!\n"));
                error = PVMFErrArgument;
            }
            break;

        case PV2WAY_NODE_CMD_RESET:
            OSCL_TRY(error, id = nodeIFace->Reset(sessionId,
                                                  (OsclAny *) & info->context));
            break;

        case PV2WAY_NODE_CMD_CANCELCMD:
            if (aParam != NULL)
            {
                OSCL_TRY(error, id = nodeIFace->CancelCommand(sessionId,
                                     *((PVMFCommandId *) aParam), (OsclAny *) & info->context));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::SendNodeCmdL CancelCommand param is null!\n"));
                error = PVMFErrArgument;
            }

            //Remove commands on pending list
            if (!error)
            {
                RemovePendingNodeCmd((PVMFNodeInterface *)aNode,
                                     *((PVMFCommandId *) aParam));
            }
            break;

        case PV2WAY_NODE_CMD_CANCELALL:
            OSCL_TRY(error, id = nodeIFace->CancelAllCommands(sessionId,
                                 (OsclAny *) & info->context));

            //Remove commands on pending list
            if (!error)
            {
                RemovePendingNodeCmd((PVMFNodeInterface *)aNode, 0, true);
            }
            break;

        case PV2WAY_NODE_CMD_SKIP_MEDIA_DATA:
        {
            for (uint32 ii = 0;ii < iSinkNodeList.size();ii++)
            {
                if ((aNode == iSinkNodeList[ii].iSinkNode)
                        && (iSinkNodeList[ii].iNodeInterface.iState ==
                            PV2WayNodeInterface::HasInterface))
                {
                    ptr = (PvmfNodesSyncControlInterface*)
                          iSinkNodeList[ii].iNodeInterface.iInterface;
                    if (ptr != NULL)
                    {
                        //Pause the clock, since this gives a chance to register
                        // the clock observer notifications
                        iClock.Pause();
                        ptr->SetClock(&iClock);
                        ptr->SetMargins(SYNC_EARLY_MARGIN, SYNC_LATE_MARGIN);
                        OSCL_TRY(error, id =
                                     ptr->SkipMediaData(aNode->iSessionId,
                                                        resume_timestamp, STREAMID, false,
                                                        (OsclAny *) & info->context));
                        //Re-start the clock, since by now, the sink node and MIO component
                        // would've registered itself as the clock observer
                        iClock.Start();
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "CPV324m2Way::SendNodeCmdL SkipMediaData param is null!\n"));
                        error = PVMFErrArgument;
                    }
                    break;
                }
            }
        }
        break;
        case PV2WAY_NODE_CMD_INVALID:
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::SendNodeCmdL invalid command!\n"));
            OSCL_LEAVE(PVMFErrArgument);
            break;
    }

    if (error)
    {
        info->Clear();
        iFreeNodeCmdInfo.push_back(info);
        OSCL_LEAVE(error);
    }

    info->id = id;

    iPendingNodeCmdInfo.push_back(info);
    return id;
}

TPV2WayNodeCmdInfo *CPV324m2Way::FindPendingNodeCmd(PVMFNodeInterface *aNode,
        PVMFCommandId aId)
{
    for (uint32 i = 0; i < iPendingNodeCmdInfo.size(); i++)
    {
        if ((iPendingNodeCmdInfo[i]->context.iNode == aNode) &&
                (iPendingNodeCmdInfo[i]->id == aId))
        {
            return iPendingNodeCmdInfo[i];
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "CPV324m2Way::FindPendingNodeCmd unable to find command, node %x, id %d!\n", aNode, aId));
    return NULL;
}

void CPV324m2Way::RemovePendingNodeCmd(PVMFNodeInterface *aNode,
                                       PVMFCommandId aId,
                                       bool aAllCmds)
{
    TPV2WayNodeCmdInfo **info = NULL;

    info = iPendingNodeCmdInfo.begin();
    while (info != iPendingNodeCmdInfo.end())
    {
        if (((*info)->context.iNode == aNode) &&
                (aAllCmds || ((*info)->id == aId)))
        {
            (*info)->Clear();
            iFreeNodeCmdInfo.push_back(*info);
            iPendingNodeCmdInfo.erase(info);
            info = iPendingNodeCmdInfo.begin();
            continue;
        }

        info++;
    }

    return;
}


void CPV324m2Way::FillSDKInfo(PVSDKInfo &aSDKInfo)
{
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
    //              (0, "CPV324m2Way::FillSDKInfo"));
    aSDKInfo.iLabel = PV2WAY_ENGINE_SDKINFO_LABEL;
    aSDKInfo.iDate = PV2WAY_ENGINE_SDKINFO_DATE;
}

bool CPV324m2Way::CheckMandatoryCodecs(const PVMFFormatType *aMandatoryList,
                                       uint32 aMandatorySize,
                                       Oscl_Vector<PVMFFormatType, OsclMemAllocator> &aCodecList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckMandatoryCodecs"));
    uint32 i, j;
    bool found;

    if (aCodecList.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "CPV324m2Way::CheckMandatoryCodecs empty codecs list, use default"));
        return true;
    }

    for (i = 0; i < aMandatorySize; i++)
    {
        found = false;
        for (j = 0; j < aCodecList.size(); j++)
        {
            if (aMandatoryList[i] == aCodecList[j])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::CheckMandatoryCodecs %s not found!",
                             (aMandatoryList[i]).getMIMEStrPtr()));
            return false;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                    (0, "CPV324m2Way::CheckMandatoryCodecs all codecs found"));
    return true;
}

void CPV324m2Way::InitiateSession(TPV2WayNode& aNode)
{
    PVMFNodeInterface * nodeIFace = (PVMFNodeInterface *)aNode ;
    PVMFNodeSessionInfo session(this, this, aNode, this, aNode);
    aNode.iSessionId =  nodeIFace->Connect(session);
    nodeIFace->ThreadLogon();
}

bool CPV324m2Way::IsNodeReset(PVMFNodeInterface& aNode)
{
    TPVMFNodeInterfaceState node_state = aNode.GetState();

    if (node_state == EPVMFNodeCreated || node_state == EPVMFNodeIdle)
        return true;
    return false;
}


void CPV324m2Way::SetPreferredCodecs(TPVDirection aDir,
                                     Oscl_Vector<const char*, OsclMemAllocator>& aAppAudioFormats,
                                     Oscl_Vector<const char*, OsclMemAllocator>& aAppVideoFormats)
{
    /* Iterate over formats supported by the stack */
    Oscl_Map<PVMFFormatType, CPvtMediaCapability*, OsclMemAllocator, pvmf_format_type_key_compare_class>::iterator it = iStackSupportedFormats.begin();
    while (it != iStackSupportedFormats.end())
    {
        CPvtMediaCapability* media_capability = (*it++).second;
        const char* format_str = NULL;
        // Is format present in application formats ?
        format_str = FindFormatType(media_capability->GetFormatType(), aAppAudioFormats, aAppVideoFormats);
        if (format_str)
        {
            DoSelectFormat(aDir, media_capability->GetFormatType(), format_str, APP);
        }
        else
        {
            PV2WayMediaType media_type = ::GetMediaType(PVMFFormatTypeToPVCodecType(media_capability->GetFormatType()));
            const char* can_convert_format = NULL;

            if (media_type == PV_AUDIO)
            {
                can_convert_format = CanConvertFormat(aDir, media_capability->GetFormatType(), aAppAudioFormats);
            }
            else if (media_type == PV_VIDEO)
            {
                can_convert_format = CanConvertFormat(aDir, media_capability->GetFormatType(), aAppVideoFormats);
            }

            if (can_convert_format)
            {
                // Engine can convert the format using a conversion node
                DoSelectFormat(aDir, media_capability->GetFormatType(), format_str, ENG, can_convert_format);
            }
            else
            {
                // Check if it is a mandatory codec
                if (media_capability->IsMandatory())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "CPV324m2Way::SetPreferredCodecs, ERROR Mandatory codec=%s not supported",
                                     (media_capability->GetFormatType()).getMIMEStrPtr()));
                    OSCL_LEAVE(PVMFErrResource);
                }
            }
        }
    }
}

void CPV324m2Way::SetPreferredCodecs(PV2WayInitInfo& aInitInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::SetPreferredCodecs"));
    SetPreferredCodecs(INCOMING, aInitInfo.iIncomingAudioFormats, aInitInfo.iIncomingVideoFormats);
    SetPreferredCodecs(OUTGOING, aInitInfo.iOutgoingAudioFormats, aInitInfo.iOutgoingVideoFormats);
}


#if defined(PV_RECORD_TO_FILE_SUPPORT)
void CPV324m2Way::HandleFFComposerNodeCmd(PV2WayNodeCmdType aType,
        const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleFFComposerNodeCmd type %d, status %d\n",
                     aType, aResponse.GetCmdStatus()));

    switch (aType)
    {
        case PV2WAY_NODE_CMD_QUERY_INTERFACE:
            if (aResponse.GetCmdId() == iFFClipConfig.iId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iFFClipConfig.iState = PV2WayNodeInterface::HasInterface;
                }
                else
                {
                    iFFClipConfig.iState = PV2WayNodeInterface::NoInterface;
                    iRecordFileState = File2WayResetting;
                }
            }
            else if (aResponse.GetCmdId() == iFFTrackConfig.iId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iFFTrackConfig.iState = PV2WayNodeInterface::HasInterface;
                }
                else
                {
                    iFFTrackConfig.iState = PV2WayNodeInterface::NoInterface;
                    iRecordFileState = File2WayResetting;
                }
            }
            break;

        case PV2WAY_NODE_CMD_INIT:
        case PV2WAY_NODE_CMD_START:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                iRecordFileState = File2WayResetting;
            }
            break;

        case PV2WAY_NODE_CMD_STOP:
        case PV2WAY_NODE_CMD_RESET:
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::HandleFFComposerNodeCmd unhandled command\n"));
            break;
    }

    CheckRecordFileState();
    return;
}

void CPV324m2Way::RemoveAudioRecPath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveAudioRecPath audio rec path state %d\n",
                     iAudioRecDatapath->GetState()));
    if (iAudioRecDatapath->GetState() == EClosed)
    {
        iAudioRecDatapath->ResetDatapath();
    }
}

void CPV324m2Way::RemoveVideoRecPath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveVideoRecPath video rec path state %d\n",
                     iVideoRecDatapath->GetState()));
    if (iVideoRecDatapath->GetState() == EClosed)
    {
        iVideoRecDatapath->ResetDatapath();
    }
}

void CPV324m2Way::CheckRecordFileState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckRecordFileState state %d\n",
                     iRecordFileState));

    switch (iRecordFileState)
    {
        case File2WayInitializing:
            CheckRecordFileInit();
            break;

        case File2WayResetting:
            CheckRecordFileReset();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "CPV324m2Way::CheckRecordFileState warning: static state!"));
            break;
    }
}

void CPV324m2Way::CheckRecordFileInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckRecordFileInit ff composer node state %d\n",
                     iFFComposerNode->GetState()));

//	int32 error;
    return;
}

void CPV324m2Way::CheckRecordFileReset()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckRecordFileReset audio rec state %d, video rec state %d\n",
                     iAudioRecDatapath->GetState(), iVideoRecDatapath->GetState()));

//	int32 error;
    return;
}

void CPV324m2Way::InitiateResetRecordFile()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::InitiateResetRecordFile state %d, record state %d\n",
                     iState, iRecordFileState));

    iRecordFileState = File2WayResetting;

    if (iAudioRecDatapath->GetState() != EClosed)
    {
        iAudioRecDatapath->SetCmd(NULL);
    }

    if (iVideoRecDatapath->GetState() != EClosed)
    {
        iVideoRecDatapath->SetCmd(NULL);
    }

    CheckRecordFileState();
}
#endif

#if defined(PV_PLAY_FROM_FILE_SUPPORT)
void CPV324m2Way::RemoveAudioPreviewPath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveAudioPreviewPath audio preview path state %d\n",
                     iAudioPreviewDatapath->GetState()));
    if (iAudioPreviewDatapath->GetState() == EClosed)
    {
        iAudioPreviewDatapath->ResetDatapath();
    }
}

void CPV324m2Way::RemoveVideoPreviewPath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::RemoveVideoPreviewPath video preview path state %d\n",
                     iVideoPreviewDatapath->GetState()));
    if (iVideoPreviewDatapath->GetState() == EClosed)
    {
        iVideoPreviewDatapath->ResetDatapath();
    }
}

void CPV324m2Way::HandlePFFNodeCmd(PV2WayNodeCmdType aType,
                                   const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandlePFFNodeCmd type %d, status %d\n",
                     aType, aResponse.GetCmdStatus()));

    CPV2WayPort *port;

    switch (aType)
    {
        case PV2WAY_NODE_CMD_INIT:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                iPlayFileState = File2WayResetting;
            }
            break;

        case PV2WAY_NODE_CMD_REQUESTPORT:
            if (aResponse.GetCmdId() == iAudioPlayPort.GetCmdId())
            {
                port = &iAudioPlayPort;
            }
            else if (aResponse.GetCmdId() == iVideoPlayPort.GetCmdId())
            {
                port = &iVideoPlayPort;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::HandlePFFNodeCmd unknown req port id %d\n",
                                 aResponse.GetCmdId()));
                iPlayFileState = File2WayResetting;
                break;
            }

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                port->SetPort((PVMFPortInterface *) aResponse.GetEventData());
            }
            else
            {
                port->SetPort(NULL);
                iPlayFileState = File2WayResetting;
            }
            break;

        case PV2WAY_NODE_CMD_START:
        case PV2WAY_NODE_CMD_PAUSE:
        case PV2WAY_NODE_CMD_STOP:
            if (iPlayFileCmdInfo)
            {
                iPlayFileCmdInfo->status = aResponse.GetCmdStatus();
                Dispatch(iPlayFileCmdInfo);
                iPlayFileCmdInfo = NULL;
            }
            break;

        case PV2WAY_NODE_CMD_RESET:
            CheckPlayFileState();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::HandlePFFNodeCmd unhandled command\n"));
            break;
    }

    CheckPlayFileState();
    return;
}

void CPV324m2Way::CheckPlayFileState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::CheckPlayFileState state %d\n",
                     iRecordFileState));

    switch (iPlayFileState)
    {
        case File2WayInitializing:
            CheckPlayFileInit();
            break;

        case File2WayResetting:
            CheckPlayFileReset();
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "CPV324m2Way::CheckPlayFileState warning: static state!"));
            break;
    }
}

void CPV324m2Way::CheckPlayFileInit()
{
    return;
}

void CPV324m2Way::CheckPlayFileReset()
{
    return;
}

void CPV324m2Way::InitiateResetPlayFile()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::InitiateResetPlayFile state %d, record state %d\n",
                     iState, iPlayFileState));

    //Use play file as data source
    if (iAudioEncDatapath->GetState() == EOpened)
    {
        iAudioEncDatapath->UseFilePlayPort(false);
    }

    if (iVideoEncDatapath->GetState() == EOpened)
    {
        iVideoEncDatapath->UseFilePlayPort(false);
    }

    iUsePlayFileAsSource = false;

    iPlayFileState = File2WayResetting;

    CheckPlayFileState();
}

void CPV324m2Way::CheckAudioSourceMixingPort()
{
    return;
}

void CPV324m2Way::HandleAudioSrcNodeCmd(PV2WayNodeCmdType aType,
                                        const PVMFCmdResp& aResponse)
{
    int32 error;
    TPV2WayEventInfo* aEvent = NULL;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV324m2Way::HandleAudioSrcNodeCmd type %d, status %d\n",
                     aType, aResponse.GetCmdStatus()));

    //The only response this handler is expecting is a request for an audio src node input port

    switch (aType)
    {
        case PV2WAY_NODE_CMD_REQUESTPORT:
            if (aResponse.GetCmdId() ==
                    iAudioEncDatapath->GetSourceInputPort()->GetCmdId())
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iAudioEncDatapath->SetSourceInputPort((PVMFPortInterface *)
                                                          aResponse.GetEventData());
                    iAudioEncDatapath->UseFilePlayPort(iUsePlayFileAsSource);
                }
                else
                {
                    iAudioEncDatapath->SetSourceInputPort(NULL);

                    if (!GetEventInfo(aEvent))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "CPV324m2Way::HandleAudioSrcNodeCmdn unable to allocate event %d!\n",
                                         error));
                        return;
                    }

                    aEvent->type = PVT_INDICATION_PLAY_ERROR;
                    aEvent->localBuffer[0] = (uint8) PV_AUDIO;
                    aEvent->localBufferSize = 1;
                    Dispatch(aEvent);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::HandleAudioSrcNodeCmd unknown req port id %d\n",
                                 aResponse.GetCmdId()));
            }
            break;

        case PV2WAY_NODE_CMD_RELEASEPORT:
            if (aResponse.GetCmdId() == iAudioEncDatapath->GetSourceInputPort()->GetCmdId())
            {
                iAudioEncDatapath->SetSourceInputPort(NULL);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPV324m2Way::HandleAudioSrcNodeCmd unknown req port id %d\n",
                                 aResponse.GetCmdId()));
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV324m2Way::HandleAudioSrcNodeCmd unhandled command\n"));
            break;
    }

    CheckAudioSourceMixingPort();

    // Audio encode datapath will wait for stable mixing port before continuing to close.
    if (iAudioEncDatapath->GetState() == EClosing)
    {
        iAudioEncDatapath->CheckPath();
    }

    return;
}
#endif

bool CPV324m2Way::Supports(PVMFNodeCapability &capability,
                           PVMFFormatType aFormat,
                           bool isInput/*=true*/)
{
    if (isInput)
    {
        for (uint16 i = 0; i < capability.iInputFormatCapability.size(); i++)
        {
            if (capability.iInputFormatCapability[i] == aFormat)
                return true;
        }
    }
    else
    {
        for (uint16 i = 0; i < capability.iOutputFormatCapability.size(); i++)
        {
            if (capability.iOutputFormatCapability[i] == aFormat)
                return true;
        }
    }

    return false;
}

int32 CPV324m2Way::GetStackNodePortTag(TPV2WayPortTagType tagType)
{
    switch (tagType)
    {

        case EPV2WayVideoIn:
            if (iTerminalType == PV_324M)
            {
                return PV_VIDEO;
            }
            else
            {
                return -1;
            }
            break;

        case EPV2WayAudioIn:
            if (iTerminalType == PV_324M)
            {
                return PV_AUDIO;
            }
            else
            {
                return -1;
            }
            break;

        case EPV2WayVideoOut:
            if (iTerminalType == PV_324M)
            {
                return iVideoDecDatapath->GetTSCPortTag();
            }
            else
            {
                return -1;
            }
            break;

        case EPV2WayAudioOut:
            if (iTerminalType == PV_324M)
            {
                return iAudioDecDatapath->GetTSCPortTag();
            }
            else
            {
                return -1;
            }
            break;

        default:
            break;

    }
    return -1;
}

#ifndef NO_2WAY_324

bool CPV324m2Way::AllChannelsOpened()
{
    return ((iIncomingAudioTrackTag != INVALID_TRACK_ID ||
             !iIncomingAudioCodecs.size()) &&
            (iIncomingVideoTrackTag != INVALID_TRACK_ID ||
             !iIncomingVideoCodecs.size()) &&
            (iOutgoingAudioTrackTag != INVALID_TRACK_ID ||
             !iOutgoingAudioCodecs.size()) &&
            (iOutgoingVideoTrackTag != INVALID_TRACK_ID ||
             !iOutgoingVideoCodecs.size()));
}

#endif //NO_2WAY_324

void CPV324m2Way::ConvertMapToVector(Oscl_Map < PVMFFormatType,
                                     FormatCapabilityInfo,
                                     OsclMemAllocator,
                                     pvmf_format_type_key_compare_class > & aCodecs,
                                     Oscl_Vector < FormatCapabilityInfo,
                                     OsclMemAllocator > & aFormatCapability)
{
    iFormatCapability.clear();
    Oscl_Map < PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator,
    pvmf_format_type_key_compare_class >::iterator it;
    it = aCodecs.begin();
    for (it = aCodecs.begin() ; it != aCodecs.end(); it++)
    {
        iFormatCapability.push_back(aCodecs[(*it).first]);
    }

    aFormatCapability = iFormatCapability;

}


void CPV324m2Way::AddVideoEncoderNode()
{
    int32 error;

    if (iVideoEncNode != NULL)
        return;
#ifdef PV2WAY_USE_OMX
    iVideoEncNode = TPV2WayNode(CREATE_OMX_ENC_NODE());
#else
    iVideoEncNode = TPV2WayNode(CREATE_VIDEO_ENC_NODE());
#endif // PV2WAY_USE_OMX

    if (iVideoEncNode.iNode == NULL)
        OSCL_LEAVE(PVMFErrNoMemory);
    InitiateSession(iVideoEncNode);

    if (iVideoEncNodeInterface.iState == PV2WayNodeInterface::NoInterface)
    {
        TPV2WayNodeQueryInterfaceParams queryParam;
        queryParam.iInterfacePtr = &iVideoEncNodeInterface.iInterface;

        queryParam.iUuid = (PVUuid *) & iVideoEncPVUuid;

        OSCL_TRY(error, iVideoEncQueryIntCmdId = SendNodeCmdL(PV2WAY_NODE_CMD_QUERY_INTERFACE,
                 &iVideoEncNode, this, &queryParam));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::CheckInit unable to query for video encoder interface!\n"));
                             SetState(EResetting);
                             CheckState();
                             return;);

        iVideoEncNodeInterface.iState = PV2WayNodeInterface::QueryInterface;
    }


}
void CPV324m2Way::AddAudioEncoderNode()
{
    int32 error;

    if (iAudioEncNode != NULL)
        return;

#ifdef PV2WAY_USE_OMX
    OSCL_TRY(error, iAudioEncNode = TPV2WayNode(CREATE_OMX_ENC_NODE()));
#else
    OSCL_TRY(error, iAudioEncNode =
                 TPV2WayNode(CREATE_AUDIO_ENC_NODE()););
#endif
    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,
                         iLogger, PVLOGMSG_ERR,
                         (0, "CPV324m2Way::InitL unable to allocate audio encoder node\n")));

    InitiateSession(iAudioEncNode);

    if (iAudioEncNodeInterface.iState == PV2WayNodeInterface::NoInterface)
    {
        TPV2WayNodeQueryInterfaceParams queryParam;
        queryParam.iInterfacePtr = &iAudioEncNodeInterface.iInterface;

        queryParam.iUuid = (PVUuid *) & iAudioEncPVUuid;

        OSCL_TRY(error, iAudioEncNodeInterface.iId =
                     SendNodeCmdL(PV2WAY_NODE_CMD_QUERY_INTERFACE,
                                  &iAudioEncNode, this, &queryParam));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,
                                             iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::CheckInit unable to query for audio encoder interface!\n"));
                             SetState(EResetting);
                             CheckState();
                             return;);

        iAudioEncNodeInterface.iState = PV2WayNodeInterface::QueryInterface;
    }
    else if ((iAudioEncNode.iNode)->GetState() == EPVMFNodeError)
    {
        OSCL_TRY(error, SendNodeCmdL(PV2WAY_NODE_CMD_RESET, &iAudioEncNode, this));
        OSCL_FIRST_CATCH_ANY(error,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                             (0, "CPV324m2Way::CheckInit unable to reset audio encoder node after error!\n"));
                             return;);
    }


    // start enc datapaths that are already created
    if (iAudioEncDatapath->GetState() != EClosed)
    {
        iAudioEncDatapath->CheckOpen();
    }

}
void CPV324m2Way::AddVideoDecoderNode(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen)
{
    int32 error = 0;
    if (iVideoDecNode != NULL)
        return;

#ifdef PV2WAY_USE_OMX
    OSCL_TRY(error, iVideoDecNode = TPV2WayNode(CREATE_OMX_VIDEO_DEC_NODE()););
#else
    OSCL_TRY(error, iVideoDecNode = TPV2WayNode(CREATE_VIDEO_DEC_NODE()););
#endif // PV2WAY_USE_OMX


    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                         PVLOGMSG_ERR, (0, "CPV324m2Way::InitL unable to allocate video decoder node\n")));

    OSCL_TRY(error, iVideoParserNode = TPV2WayNode(PVMFVideoParserNode::Create(aFormatSpecificInfo, aFormatSpecificInfoLen)););
    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                         PVLOGMSG_ERR, (0, "CPV324m2Way::InitL unable to allocate video parser node\n")));

    InitiateSession(iVideoDecNode);
    InitiateSession(iVideoParserNode);
}

void CPV324m2Way::AddAudioDecoderNode()
{
    int32 error;

    if (iAudioDecNode != NULL)
        return;

#ifdef PV2WAY_USE_OMX
    OSCL_TRY(error, iAudioDecNode =
                 TPV2WayNode(CREATE_OMX_AUDIO_DEC_NODE()););
#else
    OSCL_TRY(error, iAudioDecNode =
                 TPV2WayNode(CREATE_AUDIO_DEC_NODE());
             /*iAudioDecNode->SetClock(&iClock);*/);
#endif // PV2WAY_USE_OMX

    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                         PVLOGMSG_ERR, (0, "CPV324m2Way::InitL unable to allocate audio decoder node\n")));

    InitiateSession(iAudioDecNode);
}

void CPV324m2Way::RegisterMioLatency(const char* aMimeStr,
                                     bool aAudio,
                                     PVMFFormatType aFmtType)
{
    uint32 latencyVal = 0;
    if (aMimeStr != NULL)
    {
        const char* latencyStr = oscl_strstr(aMimeStr, "latency");
        if (latencyStr != NULL)
        {
            const char* latVal = oscl_strstr(latencyStr, "=");
            if (latVal != NULL)
            {
                latVal += 1;
                PV_atoi(latVal, 'd', latencyVal);
            }
        }
    }
    if (aAudio)
    {
        iAudioLatency[(char*)aFmtType.getMIMEStrPtr()] = latencyVal;
    }
    else
    {
        iVideoLatency[(char*)aFmtType.getMIMEStrPtr()] = latencyVal;
    }
}

uint32 CPV324m2Way::LookupMioLatency(PVMFFormatType aFmtType,
                                     bool aAudio)
{
    Oscl_Map<char*, uint32, OsclMemAllocator>::iterator it;
    if (aAudio)
    {
        it = iAudioLatency.find((char*)(aFmtType.getMIMEStrPtr()));
        if (!(it == iAudioLatency.end()))
        {
            return (((*it).second));
        }
        else
        {
            return 0; //if no latency is specified, then default is 0
        }
    }
    else
    {
        it = iVideoLatency.find((char*)aFmtType.getMIMEStrPtr());
        if (!(it == iVideoLatency.end()))
        {
            return (((*it).second));
        }
        else
        {
            return 0; //if no latency is specified, then default is 0
        }
    }
}

#ifdef MEM_TRACK
void CPV324m2Way::MemStats()
{
#if !(OSCL_BYPASS_MEMMGT)

    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            printf("  numBytes %d\n", stats->numBytes);
            printf("  peakNumBytes %d\n", stats->peakNumBytes);
            printf("  numAllocs %d\n", stats->numAllocs);
            printf("  peakNumAllocs %d\n", stats->peakNumAllocs);
            printf("  numAllocFails %d\n", stats->numAllocFails);
            printf("  totalNumAllocs %d\n", stats->totalNumAllocs);
            printf("  totalNumBytes %d\n", stats->totalNumBytes);
        }

    }
#endif
}
#endif

/* This should be changed to query the node registry */
bool CPV324m2Way::IsSupported(const PVMFFormatType& aInputFmtType, const PVMFFormatType& aOutputFmtType)
{
    if (aInputFmtType == PVMF_MIME_AMR_IF2)
    {
        if ((aOutputFmtType == PVMF_MIME_PCM8) || (aOutputFmtType == PVMF_MIME_PCM16))
        {
            return true;
        }
        return false;
    }
    else if ((aInputFmtType ==  PVMF_MIME_M4V) || (aInputFmtType ==  PVMF_MIME_H2632000))
    {
        if (aOutputFmtType == PVMF_MIME_YUV420)
        {
            return true;
        }
        return false;
    }
    else if ((aInputFmtType ==  PVMF_MIME_PCM8) || (aInputFmtType ==  PVMF_MIME_PCM16))
    {
        if (aOutputFmtType == PVMF_MIME_AMR_IF2)
        {
            return true;
        }
        return false;
    }
    else if ((aInputFmtType ==  PVMF_MIME_YUV420))
    {
        if (aOutputFmtType == PVMF_MIME_M4V || aOutputFmtType == PVMF_MIME_H2632000)
        {
            return true;
        }
        return false;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV324m2Way::IsSupported() Not supported format\n"));
        return false;
    }
}

/* This should be changed to query the formats from the stack */
void CPV324m2Way::GetStackSupportedFormats()
{
    iStackSupportedFormats[PVMF_MIME_AMR_IF2] = OSCL_NEW(CPvtAudioCapability, (PVMF_MIME_AMR_IF2, MAX_AMR_BITRATE));
    iStackSupportedFormats[PVMF_MIME_M4V] = OSCL_NEW(CPvtMpeg4Capability, (MAX_VIDEO_BITRATE));
    iStackSupportedFormats[PVMF_MIME_H2632000] = OSCL_NEW(CPvtH263Capability, (MAX_VIDEO_BITRATE));
}

const char* CPV324m2Way::FindFormatType(PVMFFormatType aFormatType,
                                        Oscl_Vector<const char*, OsclMemAllocator>& aAudioFormats,
                                        Oscl_Vector<const char*, OsclMemAllocator>& aVideoFormats)
{
    PVMFFormatType aThatFormatType = PVMF_MIME_FORMAT_UNKNOWN;
    uint32 i = 0;

    for (i = 0; i < aAudioFormats.size(); i++)
    {
        aThatFormatType = aAudioFormats[i];
        if (aFormatType == aThatFormatType)
        {
            return aAudioFormats[i];
        }
    }

    for (i = 0; i < aVideoFormats.size(); i++)
    {
        aThatFormatType  = aVideoFormats[i];
        if (aFormatType == aThatFormatType)
        {
            return aVideoFormats[i];
        }
    }
    return NULL;
}

const char* CPV324m2Way::CanConvertFormat(TPVDirection aDir, const PVMFFormatType& aThisFmtType, Oscl_Vector<const char*, OsclMemAllocator>& aThatFormatList)
{
    PVMFFormatType aInputFmtType = PVMF_MIME_FORMAT_UNKNOWN;
    PVMFFormatType aOutputFmtType = PVMF_MIME_FORMAT_UNKNOWN;

    OSCL_ASSERT(aDir == INCOMING || aDir == OUTGOING);

    for (uint32 i = 0; i < aThatFormatList.size(); i++)
    {
        PVMFFormatType thatFmtType = aThatFormatList[i];
        aInputFmtType = (aDir == INCOMING) ? aThisFmtType : thatFmtType;
        aOutputFmtType = (aDir == INCOMING) ? thatFmtType : aThisFmtType;
        if (IsSupported(aInputFmtType, aOutputFmtType))
        {
            return aThatFormatList[i];
        }
    }
    return NULL;
}

void CPV324m2Way::DoSelectFormat(TPVDirection aDir, PVMFFormatType aFormatType, const char* aFormatStr, TPVPriority aPriority, PVMFFormatType aFormatTypeApp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::DoSelectFormat, aDir=%d, aFormatType=%s, aPriority=%d",
                     aDir, aFormatType.getMIMEStrPtr(), aPriority));

    FormatCapabilityInfo format_cap_info;
    PV2WayMediaType media_type = GetMediaType(PVMFFormatTypeToPVCodecType(aFormatType));
    Oscl_Map<PVMFFormatType, FormatCapabilityInfo, OsclMemAllocator, pvmf_format_type_key_compare_class>* the_engine_map = NULL;
    Oscl_Map<PVMFFormatType, PVMFFormatType, OsclMemAllocator, pvmf_format_type_key_compare_class>* the_app_map = NULL;

    format_cap_info.dir = aDir;
    FILL_FORMAT_INFO(aFormatType, format_cap_info);
    format_cap_info.iPriority = aPriority;

    switch (aDir)
    {
        case OUTGOING:
            the_engine_map = (media_type == PV_AUDIO) ? &iOutgoingAudioCodecs : &iOutgoingVideoCodecs;
            the_app_map = &iAppFormatForEngineFormatOutgoing;
            break;
        case INCOMING:
            the_engine_map = (media_type == PV_AUDIO) ? &iIncomingAudioCodecs : &iIncomingVideoCodecs;
            the_app_map = &iAppFormatForEngineFormatIncoming;
            break;
        default:
            return;
    }
    (*the_engine_map)[aFormatType] = format_cap_info;
    (*the_app_map)[aFormatType] = aFormatTypeApp;

    RegisterMioLatency(aFormatStr, true, aFormatType);
}

// This function returns a priority index for each format type.
#define PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_START 0
#define PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_END 0xFF
uint32 GetPriorityIndexForPVMFFormatType(PVMFFormatType aFormatType)
{
    if (aFormatType == PVMF_MIME_AMR_IF2)
        return PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_START;
    else if (aFormatType == PVMF_MIME_M4V)
        return PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_START + 1;
    else if (aFormatType == PVMF_MIME_H2632000)
        return PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_START + 2;
    else
        return PV2WAY_ENGINE_PRIORITY_INDEX_FOR_FORMAT_TYPE_END;
}
