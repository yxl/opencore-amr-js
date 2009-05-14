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
 * @file pvmp4ffcn_node.cpp
 * @brief Node for PV MPEG4 file format composer
 */

#ifndef PVMP4FFCN_NODE_H_INCLUDED
#include "pvmp4ffcn_node.h"
#endif
#ifndef PVMP4FFCN_FACTORY_H_INCLUDED
#include "pvmp4ffcn_factory.h"
#endif
#ifndef PVMP4FFCN_PORT_H_INCLUDED
#include "pvmp4ffcn_port.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif

#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);

#ifdef _TEST_AE_ERROR_HANDLING
const uint32 FAIL_NODE_CMD_START = 2;
const uint32 FAIL_NODE_CMD_STOP = 3;
const uint32 FAIL_NODE_CMD_FLUSH = 4;
const uint32 FAIL_NODE_CMD_PAUSE = 5;
const uint32 FAIL_NODE_CMD_RELEASE_PORT = 7;
#endif

#define SLASH '/'

#define LANG_CODE_SIZE 3

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMp4FFComposerNodeFactory::CreateMp4FFComposer(int32 aPriority)
{
    int32 err = 0;
    PVMFNodeInterface* node = NULL;

    OSCL_TRY(err,
             node = (PVMFNodeInterface*)OSCL_NEW(PVMp4FFComposerNode, (aPriority));
             if (!node)
             OSCL_LEAVE(OsclErrNoMemory);
            );

    OSCL_FIRST_CATCH_ANY(err, return NULL;);
    return node;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMp4FFComposerNodeFactory::DeleteMp4FFComposer(PVMFNodeInterface* aComposer)
{
    if (aComposer)
    {
        PVMp4FFComposerNode* node = (PVMp4FFComposerNode*)aComposer;
        OSCL_DELETE(node);
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
PVMp4FFComposerNode::PVMp4FFComposerNode(int32 aPriority)
        : OsclActiveObject(aPriority, "PVMp4FFComposerNode")
        , iMpeg4File(NULL)
        , iFileType(0)
        , iAuthoringMode(PVMP4FF_3GPP_DOWNLOAD_MODE)
        , iPresentationTimescale(1000)
        , iMovieFragmentDuration(2000)
        , iRecordingYear(0)
        , iClockConverter(8000)
        , iExtensionRefCount(0)
        , iRealTimeTS(false)
        , iInitTSOffset(false)
        , iTSOffset(0)
        , iMaxFileSizeEnabled(false)
        , iMaxDurationEnabled(false)
        , iMaxFileSize(0)
        , iMaxTimeDuration(0)
        , iFileSizeReportEnabled(false)
        , iDurationReportEnabled(false)
        , iFileSizeReportFreq(0)
        , iDurationReportFreq(0)
        , iNextDurationReport(0)
        , iNextFileSizeReport(0)
        , iCacheSize(0)
        , iConfigSize(0)
        , pConfig(NULL)
        , iTrackId_H264(0)
        , iTrackId_Text(0)
        , iSyncSample(0)
        , iformat_h264(PVMF_MIME_FORMAT_UNKNOWN)
        , iformat_text(PVMF_MIME_FORMAT_UNKNOWN)
        , iNodeEndOfDataReached(false)
        , iSampleInTrack(false)
        , iFileRendered(false)
{
    iInterfaceState = EPVMFNodeCreated;
    iNum_PPS_Set = 0;
    iNum_SPS_Set = 0;
    iText_sdIndex = 0;
    iFileObject = NULL;
#if PROFILING_ON
    iMaxSampleAddTime = 0;
    iMinSampleAddTime = 0;
    iMinSampleSize = 0;
    iMaxSampleSize = 0;
    iNumSamplesAdded = 0;
    oDiagnosticsLogged = false;
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvauthordiagnostics.composer.mp4");
    // Statistics
    for (uint32 i = 0; i < 3; i++)
        oscl_memset(&(iStats[i]), 0, sizeof(PVMp4FFCNStats));
#endif

    iLogger = PVLogger::GetLoggerObject("PVMp4FFComposerNode");
    iDataPathLogger = PVLogger::GetLoggerObject("datapath.sinknode.mp4composer");
    int32 err;
    OSCL_TRY(err,
             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iCmdQueue.Construct(PVMF_MP4FFCN_COMMAND_ID_START, PVMF_MP4FFCN_COMMAND_VECTOR_RESERVE);
             iCurrentCmd.Construct(0, 1); // There's only 1 current command


             //Create the port vector.
             iInPorts.Construct(PVMF_MP4FFCN_PORT_VECTOR_RESERVE);
            );

    OSCL_FIRST_CATCH_ANY(err,
                         //if a leave happened, cleanup and re-throw the error
                         iCmdQueue.clear();
                         iCurrentCmd.clear();
                         iInPorts.clear();
                         memvector_sps.clear();
                         memvector_pps.clear();
                         OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
                         OSCL_CLEANUP_BASE_CLASS(OsclActiveObject);
                         OSCL_LEAVE(err);
                        );

#ifdef _TEST_AE_ERROR_HANDLING
    iErrorHandlingAddMemFrag = false;
    iErrorHandlingAddTrack = false;
    iErrorCreateComposer = false;
    iErrorRenderToFile = false;
    iErrorAddTrack = PVMF_MIME_FORMAT_UNKNOWN;
    iErrorNodeCmd = 0;
    iTestFileSize = 0;
    iTestTimeStamp = 0;
    iErrorAddSample = 0;
    iFileSize = 0;
    iFileDuration = 0;
    iErrorDataPathStall = 0;
#endif
}

////////////////////////////////////////////////////////////////////////////
PVMp4FFComposerNode::~PVMp4FFComposerNode()
{
#if PROFILING_ON
    if (!oDiagnosticsLogged)
    {
        LogDiagnostics();
    }
#endif
    if (iMpeg4File)
    {
        PVA_FF_IMpeg4File::DestroyMP4FileObject(iMpeg4File);

        if (!iFileRendered)
        {
            iFs.Connect();
            iFs.Oscl_DeleteFile(iFileName.get_cstr());
            iFs.Close();
        }
    }
    if (iFileObject)
    {
        iFileObject->Close();
        OSCL_DELETE(iFileObject);
        iFileObject = NULL;
    }
    for (uint32 i = 0; i < iKeyWordVector.size() ; i++)
    {
        if (iKeyWordVector[i] != NULL)
        {
            OSCL_DELETE(iKeyWordVector[i]);
            iKeyWordVector[i] = NULL;
        }

    }

    if (pConfig != NULL)
    {
        OSCL_FREE(pConfig);
        iConfigSize = 0;
    }

    if (iLocationInfo._location_name != NULL)
    {
        OSCL_FREE(iLocationInfo._location_name);
    }

    if (iLocationInfo._astronomical_body != NULL)
    {
        OSCL_FREE(iLocationInfo._astronomical_body);
    }

    if (iLocationInfo._additional_notes != NULL)
    {
        OSCL_FREE(iLocationInfo._additional_notes);
    }
    // Cleanup allocated ports
    while (!iInPorts.empty())
    {
        iInPorts.Erase(&iInPorts.front());

    }
    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCmdQueue.empty())
    {
        CommandComplete(iCmdQueue, iCmdQueue[0], PVMFFailure);
    }

    while (!iCurrentCmd.empty())
    {
        CommandComplete(iCurrentCmd, iCurrentCmd[0], PVMFFailure);
    }
    iNodeEndOfDataReached = false;

    Cancel();
    if (iInterfaceState != EPVMFNodeCreated)
        iInterfaceState = EPVMFNodeIdle;

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::ThreadLogon()
{
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMp4FFComposerNode:ThreadLogoff"));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            iDataPathLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    aNodeCapability.iCanSupportMultipleInputPorts = true;
    aNodeCapability.iCanSupportMultipleOutputPorts = false;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    aNodeCapability.iMaxNumberOfPorts = PVMF_MP4FFCN_MAX_INPUT_PORT + PVMF_MP4FFCN_MAX_OUTPUT_PORT;
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_MP4);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2631998);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR_IETF);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_AMRWB_IETF);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_3GPP_TIMEDTEXT);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_MPEG4_AUDIO);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVMp4FFComposerNode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);
    iInPorts.Reset();
    return &iInPorts;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::QueryUUID(PVMFSessionId aSession, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::QueryUUID"));

    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::QueryInterface(PVMFSessionId aSession, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMp4FFComposerNode::QueryInterface"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Init(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Init"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Prepare(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Prepare"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::RequestPort(PVMFSessionId aSession,
        int32 aPortTag,
        const PvmfMimeString* aPortConfig,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::RequestPort"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::ReleasePort(PVMFSessionId aSession,
        PVMFPortInterface& aPort,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::ReleasePort"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Start(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Start"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Stop(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Stop"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Pause(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Pause"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Flush(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Flush"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::Reset(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::Reset"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::CancelAllCommands(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::CancelAllCommands"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerNode::CancelCommand(PVMFSessionId aSession, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::CancelCommand"));
    PVMp4FFCNCmd cmd;
    cmd.Construct(aSession, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMp4FFComposerNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == KPVMp4FFCNClipConfigUuid)
    {
        PVMp4FFCNClipConfigInterface* myInterface = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == KPVMp4FFCNTrackConfigUuid)
    {
        PVMp4FFCNTrackConfigInterface* myInterface = OSCL_STATIC_CAST(PVMp4FFCNTrackConfigInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PvmfComposerSizeAndDurationUuid)
    {
        PvmfComposerSizeAndDurationInterface* myInterface = OSCL_STATIC_CAST(PvmfComposerSizeAndDurationInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else
    {
        iface = NULL;
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
//            PVMp4FFCNClipConfigInterface routines
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint16 PVMp4FFComposerNode::ConvertLangCode(const OSCL_String & aLang)
{
    int i = 0;
    char lang[LANG_CODE_SIZE] = {0};
    oscl_strncpy(lang, aLang.get_cstr(), LANG_CODE_SIZE);

    uint16 lang_code = ((((uint16)lang[i] - 0x60) << 10) | (((uint16)lang[i+1] - 0x60) << 5) | ((uint16)lang[i+2] - 0x60));

    return lang_code;
}
/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetOutputFileName(const OSCL_wString& aFileName)
{
    if (iInterfaceState != EPVMFNodeIdle && iInterfaceState != EPVMFNodeInitialized)
        return false;

    iFileName = aFileName;
    return PVMFSuccess;
}
//////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetOutputFileDescriptor(const OsclFileHandle* aFileHandle)
{
    if (iInterfaceState != EPVMFNodeIdle && iInterfaceState != EPVMFNodeInitialized)
        return false;

    iFileObject = OSCL_NEW(Oscl_File, (0, (OsclFileHandle *)aFileHandle));
    iFileObject->SetPVCacheSize(0);
    iFileObject->SetAsyncReadBufferSize(0);
    iFileObject->SetNativeBufferSize(0);
    iFileObject->SetLoggingEnable(false);
    iFileObject->SetSummaryStatsLoggingEnable(false);
    iFileObject->SetFileHandle((OsclFileHandle*)aFileHandle);

    //call open
    int32 retval = iFileObject->Open(_STRLIT_CHAR("dummy"),
                                     Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY,
                                     iFs);

    if (retval == 0)
    {
        return PVMFSuccess;
    }
    return PVMFFailure;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetAuthoringMode(PVMp4FFCN_AuthoringMode aAuthoringMode)
{
    if (iInterfaceState != EPVMFNodeIdle && iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    iAuthoringMode = aAuthoringMode;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetPresentationTimescale(uint32 aTimescale)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iPresentationTimescale = aTimescale;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetVersion(const OSCL_wString& aVersion, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iVersion.iDataString = aVersion;
    iVersion.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetTitle(const OSCL_wString& aTitle, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iTitle.iDataString = aTitle;
    iTitle.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetAuthor(const OSCL_wString& aAuthor, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iAuthor.iDataString = aAuthor;
    iAuthor.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetCopyright(const OSCL_wString& aCopyright, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iCopyright.iDataString = aCopyright;
    iCopyright.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetDescription(const OSCL_wString& aDescription, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iDescription.iDataString = aDescription;
    iDescription.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetRating(const OSCL_wString& aRating, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iRating.iDataString = aRating;
    iRating.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetCreationDate(const OSCL_wString& aCreationDate)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iCreationDate = aCreationDate;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::SetRealTimeAuthoring(const bool aRealTime)
{
    if (iInterfaceState != EPVMFNodeIdle && iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    iRealTimeTS = aRealTime;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetAlbumInfo(const OSCL_wString& aAlbumTitle, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iAlbumTitle.iDataString = aAlbumTitle;
    iAlbumTitle.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetRecordingYear(uint16 aRecordingYear)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iRecordingYear = aRecordingYear;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetPerformer(const OSCL_wString& aPerformer, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iPerformer.iDataString = aPerformer;
    iPerformer.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetGenre(const OSCL_wString& aGenre, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iGenre.iDataString = aGenre;
    iGenre.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetClassification(const OSCL_wString& aClassificationInfo, uint32 aClassificationEntity, uint16 aClassificationTable, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iClassification.iDataString = aClassificationInfo;
    iClassification.iClassificationEntity = aClassificationEntity;
    iClassification.iClassificationTable = aClassificationTable;
    iClassification.iLangCode = ConvertLangCode(aLangCode);
    return PVMFSuccess;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetKeyWord(const OSCL_wString& aKeyWordInfo, const OSCL_String& aLangCode)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    PVMP4FFCN_KeyWord *KeyWord = NULL;

    uint16 langCode = ConvertLangCode(aLangCode);
    KeyWord = OSCL_NEW(PVMP4FFCN_KeyWord, (aKeyWordInfo, aKeyWordInfo.get_size(), langCode));

    iKeyWordVector.push_back(KeyWord);


    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetLocationInfo(PvmfAssetInfo3GPPLocationStruct& aLocation_info)
{
    if (iInterfaceState != EPVMFNodeIdle &&
            iInterfaceState != EPVMFNodeInitialized &&
            iInterfaceState != EPVMFNodePrepared)
        return PVMFErrInvalidState;

    iLocationInfo._location_name = NULL;
    uint32 size = oscl_strlen(aLocation_info._location_name);
    iLocationInfo._location_name = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(iLocationInfo._location_name, aLocation_info._location_name, size);
    iLocationInfo._location_name[size+1] = 0;

    iLocationInfo._astronomical_body = NULL;
    size = oscl_strlen(aLocation_info._astronomical_body);
    iLocationInfo._astronomical_body = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(iLocationInfo._astronomical_body, aLocation_info._astronomical_body, size);
    iLocationInfo._astronomical_body[size+1] = 0;

    iLocationInfo._additional_notes = NULL;
    size = oscl_strlen(aLocation_info._additional_notes);
    iLocationInfo._additional_notes = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * size + 10);
    oscl_strncpy(iLocationInfo._additional_notes, aLocation_info._additional_notes, size);
    iLocationInfo._additional_notes[size+1] = 0;

    iLocationInfo._role = aLocation_info._role;
    iLocationInfo._longitude = aLocation_info._longitude;
    iLocationInfo._latitude = aLocation_info._latitude;
    iLocationInfo._altitude = aLocation_info._altitude;
    iLocationInfo._langCode = ConvertLangCode(aLocation_info.Lang_code);

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
//                PVMp4FFCNTrackConfigInterface routines
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetTrackReference(const PVMFPortInterface& aPort,
        const PVMFPortInterface& aReferencePort)
{
    if (iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    int32 portIndex = -1;
    int32 refPortIndex = -1;
    PVMp4FFComposerPort* port = OSCL_REINTERPRET_CAST(PVMp4FFComposerPort*, &aPort);
    PVMp4FFComposerPort* refPort = OSCL_REINTERPRET_CAST(PVMp4FFComposerPort*, &aReferencePort);

    for (uint32 i = 0; i < iInPorts.size(); i++)
    {
        if (iInPorts[i] == port)
            portIndex = i;
        if (iInPorts[i] == refPort)
            refPortIndex = i;
    }

    if (portIndex > 0 && refPortIndex > 0)
    {
        iInPorts[portIndex]->SetReferencePort(iInPorts[refPortIndex]);
        return PVMFSuccess;
    }
    else
        return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetCodecSpecificInfo(const PVMFPortInterface& aPort,
        uint8* aInfo, int32 aSize)
{
    PVMFStatus status = PVMFFailure;

    if ((status == PVMFSuccess) &&
            (iInterfaceState == EPVMFNodeStarted))
    {
        PVMp4FFComposerPort* port = OSCL_STATIC_CAST(PVMp4FFComposerPort*, &aPort);
        iMpeg4File->setDecoderSpecificInfo(aInfo, aSize, port->GetTrackId());
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
//          PvmfComposerSizeAndDurationInterface routines
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetMaxFileSize(bool aEnable, uint32 aMaxFileSizeBytes)
{
    iMaxFileSizeEnabled = aEnable;
    if (iMaxFileSizeEnabled)
    {
        iMaxFileSize = aMaxFileSizeBytes;
    }
    else
    {
        iMaxFileSize = 0;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::GetMaxFileSizeConfig(bool& aEnable, uint32& aMaxFileSizeBytes)
{
    aEnable = iMaxFileSizeEnabled;
    aMaxFileSizeBytes = iMaxFileSize;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetMaxDuration(bool aEnable, uint32 aMaxDurationMilliseconds)
{
    iMaxDurationEnabled = aEnable;
    if (iMaxDurationEnabled)
    {
        iMaxTimeDuration = aMaxDurationMilliseconds;
    }
    else
    {
        iMaxTimeDuration = 0;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::GetMaxDurationConfig(bool& aEnable, uint32& aMaxDurationMilliseconds)
{
    aEnable = iMaxDurationEnabled;
    aMaxDurationMilliseconds = iMaxTimeDuration;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetFileSizeProgressReport(bool aEnable, uint32 aReportFrequency)
{
    iFileSizeReportEnabled = aEnable;
    if (iFileSizeReportEnabled)
    {
        iFileSizeReportFreq = aReportFrequency;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::GetFileSizeProgressReportConfig(bool& aEnable, uint32& aReportFrequency)
{
    aEnable = iFileSizeReportEnabled;
    aReportFrequency = iFileSizeReportFreq;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerNode::SetDurationProgressReport(bool aEnable, uint32 aReportFrequency)
{
    iDurationReportEnabled = aEnable;
    if (iDurationReportEnabled)
    {
        iDurationReportFreq = aReportFrequency;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerNode::GetDurationProgressReportConfig(bool& aEnable, uint32& aReportFrequency)
{
    aEnable = iDurationReportEnabled;
    aReportFrequency = iDurationReportFreq;
}

////////////////////////////////////////////////////////////////////////////
//            PVMFPortActivityHandler routines
////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    OSCL_UNUSED_ARG(aActivity);
    // Scheduling to process port activities are handled in the port itself
}

////////////////////////////////////////////////////////////////////////////
//                    OsclActiveObject routines
////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::Run()
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::Run: iInterfaceState=%d", iInterfaceState));

    if (!iCmdQueue.empty())
    {
        if (ProcessCommand(iCmdQueue.front()))
        {
            //note: need to check the state before re-scheduling
            //since the node could have been reset in the ProcessCommand
            //call.
            if (iInterfaceState != EPVMFNodeCreated)
                RunIfNotReady();
            return;
        }
    }

    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::Run: Out. iInterfaceState=%d", iInterfaceState));
}


////////////////////////////////////////////////////////////////////////////
//                   Command Processing routines
////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMp4FFComposerNode::QueueCommandL(PVMp4FFCNCmd& aCmd)
{
    int32 err = 0;
    PVMFCommandId id = 0;

    OSCL_TRY(err, id = iCmdQueue.AddL(aCmd););
    OSCL_FIRST_CATCH_ANY(err,
                         OSCL_LEAVE(err);
                         return 0;
                        );

    // Wakeup the AO
    RunIfNotReady();
    return id;
}

////////////////////////////////////////////////////////////////////////////
bool PVMp4FFComposerNode::ProcessCommand(PVMp4FFCNCmd& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCmd.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_GENERIC_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_GENERIC_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_GENERIC_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_GENERIC_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_GENERIC_NODE_RESET:
            DoReset(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
            DoCancelCommand(aCmd);
            break;

        default://unknown command type
            CommandComplete(iCmdQueue, aCmd, PVMFFailure);
            break;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::CommandComplete(PVMp4FFCNCmdQueue& aCmdQueue, PVMp4FFCNCmd& aCmd,
        PVMFStatus aStatus, OsclAny* aEventData)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode:CommandComplete: Id %d Cmd %d Status %d Context %d Data %d"
                     , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQueue.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoQueryUuid(PVMp4FFCNCmd& aCmd)
{
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.Parse(mimetype, uuidvec, exactmatch);

    uuidvec->push_back(KPVMp4FFCNClipConfigUuid);
    uuidvec->push_back(KPVMp4FFCNTrackConfigUuid);
    uuidvec->push_back(PvmfComposerSizeAndDurationUuid);

    CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoQueryInterface(PVMp4FFCNCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMp4FFComposerNode::DoQueryInterface"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.Parse(uuid, ptr);

    if (queryInterface(*uuid, *ptr))
    {
        CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
    }
    else
    {
        CommandComplete(iCmdQueue, aCmd, PVMFFailure);
    }
}


//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoRequestPort(PVMp4FFCNCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMp4FFComposerNode::DoRequestPort() In"));

    int32 tag;
    OSCL_String* portconfig;
    aCmd.Parse(tag, portconfig);

    //validate the tag...
    switch (tag)
    {
        case PVMF_MP4FFCN_PORT_TYPE_SINK:
            if (iInPorts.size() >= PVMF_MP4FFCN_MAX_INPUT_PORT)
            {
                LOG_ERR((0, "PVMp4FFComposerNode::DoRequestPort: Error - Max number of input port already allocated"));
                CommandComplete(iCmdQueue, aCmd, PVMFFailure);
                return;
            }
            break;

        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMp4FFComposerNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iCmdQueue, aCmd, PVMFFailure);
            return;
    }

    //Allocate a new port
    OsclAny *ptr = NULL;
    int32 err;
    OSCL_TRY(err,
             ptr = iInPorts.Allocate();
             if (!ptr)
             OSCL_LEAVE(OsclErrNoMemory);
            );

    OSCL_FIRST_CATCH_ANY(err,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                         (0, "PVMp4FFComposerNode::DoRequestPort: Error - iInPorts Out of memory"));
                         CommandComplete(iCmdQueue, aCmd, PVMFErrNoMemory);
                         return;
                        );

    OSCL_StackString<20> portname;
    portname = "PVMP4ComposerIn";

    PVMp4FFComposerPort* port = OSCL_PLACEMENT_NEW(ptr, PVMp4FFComposerPort(tag, this, Priority(), portname.get_cstr()));

    // if format was provided in mimestring, set it now.
    if (portconfig)
    {
        PVMFFormatType format = portconfig->get_str();
        if (format == PVMF_MIME_3GPP_TIMEDTEXT ||
                format == PVMF_MIME_H264_VIDEO_MP4 ||
                format == PVMF_MIME_M4V ||
                format == PVMF_MIME_H2631998 ||
                format == PVMF_MIME_H2632000 ||
                format == PVMF_MIME_AMR_IETF ||
                format == PVMF_MIME_AMRWB_IETF ||
                format == PVMF_MIME_ADIF ||
                format == PVMF_MIME_ADTS ||
                format == PVMF_MIME_MPEG4_AUDIO)
        {
            port->SetFormat(format);
        }
        else
        {
            CommandComplete(iCmdQueue, aCmd, PVMFErrNotSupported);
            return;
        }
    }

    //Add the port to the port vector.
    OSCL_TRY(err, iInPorts.AddL(port););
    OSCL_FIRST_CATCH_ANY(err,
                         iInPorts.DestructAndDealloc(port);
                         CommandComplete(iCmdQueue, aCmd, PVMFErrNoMemory);
                         return;
                        );

    // Return the port pointer to the caller.
    CommandComplete(iCmdQueue, aCmd, PVMFSuccess, (OsclAny*)port);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoReleasePort(PVMp4FFCNCmd& aCmd)
{
    //Find the port in the port vector
    PVMFPortInterface* p = NULL;

    for (uint32 i = 0; i < iInPorts.size(); i++)
    {
        aCmd.Parse(p);

        PVMp4FFComposerPort* port = (PVMp4FFComposerPort*)p;

        PVMp4FFComposerPort** portPtr = iInPorts.FindByValue(port);
        if (portPtr)
        {
            //delete the port.
            iInPorts.Erase(portPtr);

#ifdef _TEST_AE_ERROR_HANDLING
            if (FAIL_NODE_CMD_RELEASE_PORT == iErrorNodeCmd)
            {
                CommandComplete(iCmdQueue, aCmd, PVMFFailure);

            }
            else
            {
                CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            }
#else
            CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
#endif

        }
        else
        {
            //port not found.
            CommandComplete(iCmdQueue, aCmd, PVMFFailure);
        }

    }
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoInit(PVMp4FFCNCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::DoInitNode() In"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            // Creation of file format library is done in DoStart. Nothing to do here.
            SetState(EPVMFNodeInitialized);
            CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            break;
        case EPVMFNodeInitialized:
            CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iCmdQueue, aCmd, PVMFErrInvalidState);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoPrepare(PVMp4FFCNCmd& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
            // Creation of file format library is done in DoStart. Nothing to do here.
            SetState(EPVMFNodePrepared);
            CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            break;
        case EPVMFNodePrepared:
            CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iCmdQueue, aCmd, PVMFErrInvalidState);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoStart(PVMp4FFCNCmd& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    uint32 i = 0;
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_START == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        {
            iPostfix = _STRLIT("00");
            iOutputPath = _STRLIT("");
            int32 pos = 0;
            for (pos = iFileName.get_size() - 1; pos >= 0; pos--)
            {
                if (iFileName[pos] == SLASH)
                    break;
            }

            if (pos == -1)
            {
                iOutputPath = _STRLIT(".");
            }
            else
            {
                for (i = 0; i <= (uint32) pos; i++)
                    iOutputPath += iFileName[i];
            }



            iFileType = 0;
            for (i = 0; i < iInPorts.size(); i++)
            {
                if (iInPorts[i]->GetFormat() == PVMF_MIME_H264_VIDEO_MP4 ||
                        iInPorts[i]->GetFormat() == PVMF_MIME_M4V ||
                        iInPorts[i]->GetFormat() == PVMF_MIME_H2631998 ||
                        iInPorts[i]->GetFormat() == PVMF_MIME_H2632000)
                {
                    iFileType |= FILE_TYPE_VIDEO;
                }
                else if (iInPorts[i]->GetFormat() == PVMF_MIME_AMR_IETF ||
                         iInPorts[i]->GetFormat() == PVMF_MIME_AMRWB_IETF ||
                         iInPorts[i]->GetFormat() == PVMF_MIME_MPEG4_AUDIO)
                {
                    iFileType |= FILE_TYPE_AUDIO;
                }
                else if (iInPorts[i]->GetFormat() == PVMF_MIME_3GPP_TIMEDTEXT)
                {
                    iFileType |= FILE_TYPE_TIMED_TEXT;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::DoStart: Error - Unsupported format"));
                    return;
                }
            }

            if (iMpeg4File)
            {
                LOG_ERR((0, "PVMp4FFComposerNode::DoStart: Error - File format library already exists"));
                CommandComplete(iCmdQueue, aCmd, PVMFFailure);
                return;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMp4FFComposerNode::DoStart: Calling PVA_FF_IMpeg4File::createMP4File(%d,0x%x,%d)",
                             iFileType, &iFs, iAuthoringMode));
#ifdef _TEST_AE_ERROR_HANDLING //test case to fail mp4 file parser
            if (iErrorCreateComposer)
            {
                //to fail createMP4File()
                OSCL_wHeapString<OsclMemAllocator> ErrFileName;

                iMpeg4File = PVA_FF_IMpeg4File::createMP4File(iFileType, iOutputPath, iPostfix,
                             (void*) & iFs, iAuthoringMode, ErrFileName, iCacheSize);

            }
            else
            {
                if (iFileObject != NULL)
                {
                    iMpeg4File = PVA_FF_IMpeg4File::createMP4File(iFileType, iAuthoringMode, iFileObject, iCacheSize);

                }
                else
                {

                    iMpeg4File = PVA_FF_IMpeg4File::createMP4File(iFileType, iOutputPath, iPostfix,
                                 (void*) & iFs, iAuthoringMode, iFileName, iCacheSize);

                }
            }
#else
            if (iFileObject != NULL)
            {
                iMpeg4File = PVA_FF_IMpeg4File::createMP4File(iFileType, iAuthoringMode, iFileObject, iCacheSize);

            }
            else
            {
                iMpeg4File = PVA_FF_IMpeg4File::createMP4File(iFileType, iOutputPath, iPostfix,
                             (void*) & iFs, iAuthoringMode, iFileName, iCacheSize);
            }
#endif
            if (!iMpeg4File)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMp4FFComposerNode::DoStart: Error - PVA_FF_IMpeg4File::createMP4File failed"));
                CommandComplete(iCmdQueue, aCmd, PVMFFailure);
                return;
            }

            iMpeg4File->setPresentationTimescale(iPresentationTimescale);
            iMpeg4File->setVersion(iVersion.iDataString, iVersion.iLangCode);
            iMpeg4File->setTitle(iTitle.iDataString, iTitle.iLangCode);
            iMpeg4File->setAuthor(iAuthor.iDataString, iAuthor.iLangCode);
            iMpeg4File->setCopyright(iCopyright.iDataString, iCopyright.iLangCode);
            iMpeg4File->setDescription(iDescription.iDataString, iDescription.iLangCode);
            iMpeg4File->setRating(iRating.iDataString, iRating.iLangCode);
            iMpeg4File->setCreationDate(iCreationDate);
            iMpeg4File->setMovieFragmentDuration(iMovieFragmentDuration);
            iMpeg4File->setAlbumInfo(iAlbumTitle.iDataString, iAlbumTitle.iLangCode);
            iMpeg4File->setRecordingYear(iRecordingYear);

            iMpeg4File->setPerformer(iPerformer.iDataString, iPerformer.iLangCode);
            iMpeg4File->setGenre(iGenre.iDataString, iGenre.iLangCode);
            iMpeg4File->setClassification(iClassification.iDataString, iClassification.iClassificationEntity, iClassification.iClassificationTable, iClassification.iLangCode);

            for (i = 0; i < iKeyWordVector.size() ; i++)
            {
                iMpeg4File->setKeyWord(iKeyWordVector[i]->iKeyWordSize, iKeyWordVector[i]->iData_String, iKeyWordVector[i]->iLang_Code);
            }

            iMpeg4File->setLocationInfo(&iLocationInfo);
            for (i = 0; i < iInPorts.size(); i++)
            {
                status = AddTrack(iInPorts[i]);
                if (status != PVMFSuccess)
                {
                    CommandComplete(iCmdQueue, aCmd, status);
                    return;
                }
            }

            // Check for and set reference tracks after track IDs are assigned
            PVMp4FFComposerPort* refPort = NULL;
            for (i = 0; i < iInPorts.size(); i++)
            {
                refPort = OSCL_STATIC_CAST(PVMp4FFComposerPort*, iInPorts[i]->GetReferencePort());
                if (refPort)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMp4FFComposerNode::DoStart: Calling addTrackReference(%d, %d)",
                                     iInPorts[i]->GetTrackId(), refPort->GetTrackId()));
                    iMpeg4File->addTrackReference(iInPorts[i]->GetTrackId(), refPort->GetTrackId());
                }
            }

            iMpeg4File->prepareToEncode();

            iInitTSOffset = true;
            iTSOffset = 0;
            SetState(EPVMFNodeStarted);
            break;
        }

        case EPVMFNodePaused:
            SetState(EPVMFNodeStarted);
            for (i = 0; i < iInPorts.size(); i++)
                ((PVMp4FFComposerPort*)iInPorts[i])->ProcessIncomingMsgReady();
            break;
        case EPVMFNodeStarted:
            status = PVMFSuccess;
            break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iCmdQueue, aCmd, status);
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::AddTrack(PVMp4FFComposerPort *aPort)
{
    int32 codecType = 0;
    int32 mediaType = 0;
    int32 trackId = 0;
    PVMP4FFCNFormatSpecificConfig* config = aPort->GetFormatSpecificConfig();
    if (!config)
    {
        LOG_ERR((0, "PVMp4FFComposerNode::AddTrack: Error - GetFormatSpecificConfig failed"));
        return PVMFFailure;
    }

    if (aPort->GetFormat() == PVMF_MIME_3GPP_TIMEDTEXT)
    {
        codecType = CODEC_TYPE_TIMED_TEXT;
        mediaType = MEDIA_TYPE_TEXT;
    }
    else if (aPort->GetFormat() == PVMF_MIME_H264_VIDEO_MP4)
    {
        codecType = CODEC_TYPE_AVC_VIDEO;
        mediaType = MEDIA_TYPE_VISUAL;
    }
    else if (aPort->GetFormat() == PVMF_MIME_M4V)
    {
        codecType = CODEC_TYPE_MPEG4_VIDEO;
        mediaType = MEDIA_TYPE_VISUAL;
    }
    else if (aPort->GetFormat() == PVMF_MIME_H2631998 ||
             aPort->GetFormat() == PVMF_MIME_H2632000)
    {
        codecType = CODEC_TYPE_BASELINE_H263_VIDEO;
        mediaType = MEDIA_TYPE_VISUAL;
    }
    else if (aPort->GetFormat() == PVMF_MIME_AMR_IETF)
    {
        codecType = CODEC_TYPE_AMR_AUDIO;
        mediaType = MEDIA_TYPE_AUDIO;
    }
    else if (aPort->GetFormat() == PVMF_MIME_AMRWB_IETF)
    {
        codecType = CODEC_TYPE_AMR_WB_AUDIO;
        mediaType = MEDIA_TYPE_AUDIO;
    }
    else if (aPort->GetFormat() ==  PVMF_MIME_MPEG4_AUDIO)
    {
        codecType = CODEC_TYPE_AAC_AUDIO;
        mediaType = MEDIA_TYPE_AUDIO;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMp4FFComposerNode::AddTrack: Error - Unsupported format"));
        return PVMFFailure;
    }

    aPort->SetCodecType(codecType);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PVMp4FFComposerNode::AddTrack: Calling PVA_FF_IMpeg4File::addTrack(0x%x,0x%x)",
                     mediaType, codecType));
#ifdef _TEST_AE_ERROR_HANDLING
    if (aPort->GetFormat() == iErrorAddTrack)
    {
        return PVMFFailure;
    }
#endif
    trackId = iMpeg4File->addTrack(mediaType, codecType);
#ifdef _TEST_AE_ERROR_HANDLING
    if (iErrorHandlingAddTrack)
    {
        trackId = 0;
    }
#endif
    if (trackId == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMp4FFComposerNode::AddTrack: Error - PVA_FF_IMpeg4File::addTrack failed"));
        return PVMFFailure;
    }
    aPort->SetTrackId(trackId);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PVMp4FFComposerNode::AddTrack: PVA_FF_IMpeg4File::addTrack success. trackID=%d", trackId));
#if PROFILING_ON

    for (uint32 k = 0; k < 3; k++)
    {
        if (iStats[k].iTrackId == 0)
        {
            iStats[k].iTrackId = trackId;
            break;
        }
    }
#endif

    switch (mediaType)
    {
        case MEDIA_TYPE_AUDIO:
            iMpeg4File->setTargetBitRate(trackId, config->iBitrate);
            iMpeg4File->setTimeScale(trackId, config->iTimescale);
            break;

        case MEDIA_TYPE_VISUAL:
            switch (codecType)
            {
                case CODEC_TYPE_BASELINE_H263_VIDEO:
                    iMpeg4File->setH263ProfileLevel(trackId, config->iH263Profile, config->iH263Level);
                    // Don't break here. Continue to set other video properties
                case CODEC_TYPE_AVC_VIDEO:
                case CODEC_TYPE_MPEG4_VIDEO:
                    iMpeg4File->setTargetBitRate(trackId, config->iBitrate);
                    iMpeg4File->setTimeScale(trackId, config->iTimescale);
                    iMpeg4File->setVideoParams(trackId, (float)config->iFrameRate,
                                               (uint16)config->iIFrameInterval, config->iWidth, config->iHeight);
                    break;
            }
            break;
        case MEDIA_TYPE_TEXT:
            iMpeg4File->setTargetBitRate(trackId, config->iBitrate);
            iMpeg4File->setTimeScale(trackId, config->iTimescale);
            break;

    }

    return PVMFSuccess;
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoStop(PVMp4FFCNCmd& aCmd)
{
    PVMFStatus status = PVMFSuccess;
#if PROFILING_ON
    if (!oDiagnosticsLogged)
    {
        LogDiagnostics();
    }
#endif
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_STOP == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            if (!iNodeEndOfDataReached)
            {
                WriteDecoderSpecificInfo();
                if (iSampleInTrack)
                {
                    status = RenderToFile();
                }

                iSampleInTrack = false;
            }

            iNodeEndOfDataReached = false;
            for (uint32 ii = 0; ii < iInPorts.size(); ii++)
            {
                iInPorts[ii]->iEndOfDataReached = false;
            }
        }
        SetState(EPVMFNodePrepared);
        break;
        case EPVMFNodePrepared:
            status = PVMFSuccess;
            break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iCmdQueue, aCmd, status);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::WriteDecoderSpecificInfo()
{
    uint32 i;
    uint32 offset = 0;
    iConfigSize = 0;
    int32 trackId;

    if (iformat_h264 == PVMF_MIME_H264_VIDEO_MP4)
    {
        trackId = iTrackId_H264;

        for (i = 0;i < memvector_sps.size();i++)
        {
            iConfigSize += 2;//2 bytes for SPS_len
            iConfigSize += memvector_sps[i]->len;
        }

        for (i = 0;i < memvector_pps.size();i++)
        {
            iConfigSize += 2;//2 bytes for PPS_len
            iConfigSize += memvector_pps[i]->len;
        }
        iConfigSize = iConfigSize + 2;//extra two bytes for nunSPS and NumPPS
        pConfig = (uint8*)(OSCL_MALLOC(sizeof(uint8) * iConfigSize));


        //currently we are ignoring NAL Length information
        oscl_memcpy((void*)(pConfig + offset), (const void*)&iNum_SPS_Set, 1);//Writing Number of SPS sets
        offset += 1;

        for (i = 0;i < memvector_sps.size();i++)
        {
            oscl_memcpy((void*)(pConfig + offset), (const void*)&memvector_sps[i]->len, 2);//Writing length of SPS
            offset += 2;
            oscl_memcpy((void*)(pConfig + offset), memvector_sps[i]->ptr, memvector_sps[i]->len);
            offset = offset + memvector_sps[i]->len;
        }

        oscl_memcpy((void*)(pConfig + offset), (const void*)&iNum_PPS_Set, 1);//Writing Number of PPS sets
        offset += 1;

        for (i = 0;i < memvector_pps.size();i++)
        {
            oscl_memcpy((void*)(pConfig + offset), (const void*)&memvector_pps[i]->len, 2);//Writing length of PPS
            offset += 2;//2 bytes for PPS Length
            oscl_memcpy((void*)(pConfig + offset), memvector_pps[i]->ptr, memvector_pps[i]->len);
            offset = offset + memvector_pps[i]->len;
        }
        iMpeg4File->setDecoderSpecificInfo(pConfig, iConfigSize, trackId);
    }

    if (iformat_text == PVMF_MIME_3GPP_TIMEDTEXT)
    {
        for (uint32 ii = 0;ii < textdecodervector.size();ii++)
        {
            trackId = iTrackId_Text;
            iMpeg4File->setTextDecoderSpecificInfo(textdecodervector[ii], trackId);
        }
    }

}
//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::RenderToFile()
{
    PVMFStatus status = PVMFSuccess;

    // Clear queued messages in ports
    uint32 i;
    for (i = 0; i < iInPorts.size(); i++)
        iInPorts[i]->ClearMsgQueues();
#ifdef _TEST_AE_ERROR_HANDLING //to fail renderToFile
    if (iErrorRenderToFile)
    {
        if (iMpeg4File)
        {
            PVA_FF_IMpeg4File::DestroyMP4FileObject(iMpeg4File);
            iMpeg4File = NULL;
        }
    }
#endif

    if (!iMpeg4File || !iMpeg4File->renderToFile(iFileName))
    {
        LOG_ERR((0, "PVMp4FFComposerNode::RenderToFile: Error - renderToFile failed"));
        ReportErrorEvent(PVMF_MP4FFCN_ERROR_FINALIZE_OUTPUT_FILE_FAILED);
        status = PVMFFailure;
    }
    else
    {
#if PROFILING_ON
        // Statistics

        for (i = 0; i < 3; i++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMp4FFComposerNode Stats: TrackId=%d, NumFrame=%d, Duration=%d",
                             iStats[i].iTrackId, iStats[i].iNumFrames, iStats[i].iDuration));
            oscl_memset(&(iStats[i]), 0, sizeof(PVMp4FFCNStats));
        }
#endif

        LOGDATATRAFFIC((0, "PVMp4FFComposerNode::RenderToFile() Done"));
        // Delete file format library
        if (iMpeg4File)
        {
            PVA_FF_IMpeg4File::DestroyMP4FileObject(iMpeg4File);
            iMpeg4File = NULL;
        }

        // Change state
        SetState(EPVMFNodePrepared);
        status = PVMFSuccess;
    }

    if (PVMFSuccess == status)
    {
        iFileRendered = true;
    }
    return status;
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoFlush(PVMp4FFCNCmd& aCmd)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::DoFlush() iInterfaceState:%d", iInterfaceState));
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_FLUSH == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            int32 err;
            uint32 i;
            bool msgPending;
            msgPending = false;

            for (i = 0; i < iInPorts.size(); i++)
            {
                if (iInPorts[i]->IncomingMsgQueueSize() > 0)
                    msgPending = true;
                iInPorts[i]->SuspendInput();
                if (iInterfaceState != EPVMFNodeStarted)
                {
                    // Port is in idle if node state is not started. Call ProcessIncomingMsgReady
                    // to wake up port AO
                    ((PVMp4FFComposerPort*)iInPorts[i])->ProcessIncomingMsgReady();
                }
            }

            // Move the command from the input command queue to the current command, where
            // it will remain until the flush completes.
            err = StoreCurrentCommand(iCurrentCmd, aCmd, iCmdQueue);
            if (0 != err)
                return;

            iCmdQueue.Erase(&aCmd);

            if (!msgPending)
            {
                FlushComplete();
                return;
            }
            break;

        default:
            CommandComplete(iCmdQueue, aCmd, PVMFFailure);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
bool PVMp4FFComposerNode::IsFlushPending()
{
    return (iCurrentCmd.size() > 0
            && iCurrentCmd.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::FlushComplete()
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::FlushComplete"));
    uint32 i = 0;
    PVMFStatus status = PVMFSuccess;
    // Flush is complete only when all queues of all ports are clear.
    // Other wise, just return from this method and wait for FlushComplete
    // from the remaining ports.
    for (i = 0; i < iInPorts.size(); i++)
    {
        if (iInPorts[i]->IncomingMsgQueueSize() > 0 ||
                iInPorts[i]->OutgoingMsgQueueSize() > 0)
        {
            return;
        }
    }
    if (!iNodeEndOfDataReached)
    {
        WriteDecoderSpecificInfo();
        // Finalize output file
        if (iSampleInTrack)
        {
            status = RenderToFile();
        }

        iSampleInTrack = false;

        if (status != PVMFSuccess)
            LOG_ERR((0, "PVMp4FFComposerNode::FlushComplete: Error - RenderToFile failed"));
    }

    // Resume port input so the ports can be re-started.
    for (i = 0; i < iInPorts.size(); i++)
        iInPorts[i]->ResumeInput();

    if (!iCurrentCmd.empty())
    {
        CommandComplete(iCurrentCmd, iCurrentCmd[0], status);
    }

    if (!iCmdQueue.empty())
    {
        // If command queue is not empty, schedule to process the next command
        RunIfNotReady();
    }


}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoPause(PVMp4FFCNCmd& aCmd)
{
    PVMFStatus status = PVMFSuccess;

#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_PAUSE == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
            SetState(EPVMFNodePaused);
            break;
        case EPVMFNodePaused:
            break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iCmdQueue, aCmd, status);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoReset(PVMp4FFCNCmd& aCmd)
{
    PVMFStatus status = PVMFSuccess;
#if PROFILING_ON
    if (!oDiagnosticsLogged)
    {
        LogDiagnostics();
    }
#endif

    if (IsAdded())
    {
        if (iSampleInTrack)
        {
            status = RenderToFile();
            iSampleInTrack = false;
        }

        //delete all ports and notify observer.
        while (!iInPorts.empty())
            iInPorts.Erase(&iInPorts.front());

        //restore original port vector reserve.
        iInPorts.Reconstruct();
        iNodeEndOfDataReached = false;

        //logoff & go back to Created state.
        SetState(EPVMFNodeIdle);
        status = PVMFSuccess;
    }
    else
    {
        OSCL_LEAVE(OsclErrInvalidState);
    }

    CommandComplete(iCmdQueue, aCmd, status);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoCancelAllCommands(PVMp4FFCNCmd& aCmd)
{
    //first cancel the current command if any
    while (!iCurrentCmd.empty())
        CommandComplete(iCurrentCmd, iCurrentCmd[0], PVMFErrCancelled);

    //next cancel all queued commands
    //start at element 1 since this cancel command is element 0.
    while (iCmdQueue.size() > 1)
        CommandComplete(iCmdQueue, iCmdQueue[1], PVMFErrCancelled);

    //finally, report cancel complete.
    CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
}

//////////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::DoCancelCommand(PVMp4FFCNCmd& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.Parse(id);

    //first check "current" command if any
    PVMp4FFCNCmd* cmd = iCurrentCmd.FindById(id);
    if (cmd)
    {
        //cancel the queued command
        CommandComplete(iCurrentCmd, *cmd, PVMFErrCancelled);
        //report cancel success
        CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
        return;
    }

    //next check input queue.
    //start at element 1 since this cancel command is element 0.
    cmd = iCmdQueue.FindById(id, 1);
    if (cmd)
    {
        //cancel the queued command
        CommandComplete(iCmdQueue, *cmd, PVMFErrCancelled);
        //report cancel success
        CommandComplete(iCmdQueue, aCmd, PVMFSuccess);
        return;
    }

    //if we get here the command isn't queued so the cancel fails.
    CommandComplete(iCmdQueue, aCmd, PVMFFailure);
}

//////////////////////////////////////////////////////////////////////////////////
//                  Port activity processing routines
//////////////////////////////////////////////////////////////////////////////////
bool PVMp4FFComposerNode::IsProcessIncomingMsgReady()
{
    if (iInterfaceState == EPVMFNodeStarted || IsFlushPending())
        return true;
    else
        return false;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::ProcessIncomingMsg: aPort=0x%x", aPort));
    PVMFStatus status = PVMFSuccess;

    switch (aPort->GetPortTag())
    {
        case PVMF_MP4FFCN_PORT_TYPE_SINK:
        {
            PVMp4FFComposerPort* port = OSCL_REINTERPRET_CAST(PVMp4FFComposerPort*, aPort);
            if (!IsProcessIncomingMsgReady())
            {
                LOG_ERR((0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - Not ready."));
                return PVMFErrBusy;
            }

            PVMFSharedMediaMsgPtr msg;
            status = port->DequeueIncomingMsg(msg);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed"));
                return status;
            }
            if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
            {
                LOGDATATRAFFIC((0, "PVMp4FFComposerNode::ProcessIncomingMsg: EOS Recvd - TrackID=%d, StreamID=%d, TS=%d, Mime=%s",
                                port->GetTrackId(), msg->getStreamID(), msg->getTimestamp(), port->GetMimeType().get_cstr()));

                port->iEndOfDataReached = true;
                //check if EOS has been received on all connected ports.
                uint32 ii = 0;
                iNodeEndOfDataReached = true;
                for (ii = 0; ii < iInPorts.size(); ii++)
                {
                    if (!iInPorts[ii]->iEndOfDataReached)
                    {
                        iNodeEndOfDataReached = false;
                    }
                }

                if (iNodeEndOfDataReached)
                {
                    //Close the file since EOS is received on every connected port
                    WriteDecoderSpecificInfo();
                    if (iSampleInTrack)
                    {
                        status = RenderToFile();
                        iSampleInTrack = false;
                    }

                    //report EOS info to engine
                    ReportInfoEvent(PVMF_COMPOSER_EOS_REACHED);
                }

                //since we do not have data to process, we can safely break here.
                break;
            }
            PVMFSharedMediaDataPtr mediaDataPtr;
            convertToPVMFMediaData(mediaDataPtr, msg);

            int32 trackId = port->GetTrackId();
            if ((mediaDataPtr->getSeqNum() == 0) && (port->GetFormat() == PVMF_MIME_M4V))
            {
                // Set VOL Header
                OsclRefCounterMemFrag volHeader;
                if (mediaDataPtr->getFormatSpecificInfo(volHeader) == false ||
                        volHeader.getMemFragSize() == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - VOL Header not available"));
                    return PVMFFailure;
                }

                iMpeg4File->setDecoderSpecificInfo((uint8*)volHeader.getMemFragPtr(),
                                                   (int32)volHeader.getMemFragSize(), trackId);
            }
            if ((mediaDataPtr->getSeqNum() == 0) && (port->GetFormat() == PVMF_MIME_H264_VIDEO_MP4))
            {
                iTrackId_H264 = port->GetTrackId();
                iformat_h264 = port->GetFormat();
            }
            if (port->GetFormat() == PVMF_MIME_3GPP_TIMEDTEXT)
            {
                iTrackId_Text = port->GetTrackId();
                iformat_text = port->GetFormat();
                OsclRefCounterMemFrag textconfiginfo;

                if (mediaDataPtr->getFormatSpecificInfo(textconfiginfo) == false ||
                        textconfiginfo.getMemFragSize() == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - VOL Header not available"));
                    return PVMFFailure;
                }
                int32* pVal = (int32*)textconfiginfo.getMemFragPtr();
                iText_sdIndex = *pVal;
            }
            if (((port->GetFormat() == PVMF_MIME_AMR_IETF) ||
                    (port->GetFormat() == PVMF_MIME_AMRWB_IETF)) && mediaDataPtr->getErrorsFlag())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE,
                                (0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error flag set for AMR!"));
                return PVMFSuccess;
            }

            if ((mediaDataPtr->getSeqNum() == 0) && (port->GetFormat() == PVMF_MIME_MPEG4_AUDIO))
            {
                // Set AAC Config
                OsclRefCounterMemFrag decSpecInfo;
                if (mediaDataPtr->getFormatSpecificInfo(decSpecInfo) == false ||
                        decSpecInfo.getMemFragSize() == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - Decoder Specific not available"));
                    return PVMFFailure;
                }

                iMpeg4File->setDecoderSpecificInfo((uint8*)decSpecInfo.getMemFragPtr(),
                                                   (int32)decSpecInfo.getMemFragSize(), trackId);
            }

            // Retrieve data from incoming queue
            OsclRefCounterMemFrag memFrag;
            uint32 numFrags = mediaDataPtr->getNumFragments();
            uint32 timestamp = mediaDataPtr->getTimestamp();
            iSyncSample = 0;
            if (mediaDataPtr->getMarkerInfo()&PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT)
            {
                iSyncSample = 1;
            }

            Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> pFrame; //vector to store the nals in the particular case of AVC
            for (uint32 i = 0; (i < numFrags) && status == PVMFSuccess; i++)
            {
                if (!mediaDataPtr->getMediaFragment(i, memFrag))
                {
                    status = PVMFFailure;
                }
                else
                {
                    OsclMemoryFragment memfragment;
                    memfragment.len = memFrag.getMemFragSize();
                    memfragment.ptr = memFrag.getMemFragPtr();
                    pFrame.push_back(memfragment);
                }
            }
            status = AddMemFragToTrack(pFrame, memFrag, port->GetFormat(), timestamp,
                                       trackId, (PVMp4FFComposerPort*)aPort);

            if (status == PVMFFailure)
                ReportErrorEvent(PVMF_MP4FFCN_ERROR_ADD_SAMPLE_TO_TRACK_FAILED, (OsclAny*)aPort);
        }
        break;

        default:
            LOG_ERR((0, "PVMp4FFComposerNode::ProcessIncomingMsg: Error - Invalid port tag"));
            ReportErrorEvent(PVMF_MP4FFCN_ERROR_ADD_SAMPLE_TO_TRACK_FAILED, (OsclAny*)aPort);
            status = PVMFFailure;
            break;
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::AddMemFragToTrack(Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> aFrame, OsclRefCounterMemFrag& aMemFrag,
        PVMFFormatType aFormat,
        uint32& aTimestamp,
        int32 aTrackId,
        PVMp4FFComposerPort *aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: aFormat=%s, aTimestamp=%d, aTrackId=%d",
                     aFormat.getMIMEStrPtr(), aTimestamp, aTrackId));

    if (iRealTimeTS)
    {
        if (iInitTSOffset && (aMemFrag.getMemFragSize() > 0))
        {
            iTSOffset = aTimestamp;
            iInitTSOffset = false;
        }

        aTimestamp = aTimestamp - iTSOffset;
    }

    uint32 timeScale = 0;
    PVMP4FFCNFormatSpecificConfig* config = aPort->GetFormatSpecificConfig();
    if (config)
    {
        timeScale = config->iTimescale;
    }

    uint32 i = 0;
#if PROFILING_ON
    PVMp4FFCNStats* stats = NULL;
    for (i = 0; i < 3; i++)
    {
        if (aTrackId == iStats[i].iTrackId)
        {
            stats = &(iStats[i]);
            break;
        }
    }
#endif

    PVMFStatus status = PVMFSuccess;
    uint8 flags = 0;
    uint32 size = 0;
    uint8* data = NULL;
    for (i = 0;i < aFrame.size();i++)
    {
        size = aFrame[i].len;
        data = OSCL_REINTERPRET_CAST(uint8*, aFrame[i].ptr);
        if (!data || size == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - Invalid data or data size"));
            return PVMFFailure;
        }
    }

    if (aFormat == PVMF_MIME_3GPP_TIMEDTEXT ||
            aFormat == PVMF_MIME_H264_VIDEO_MP4 ||
            aFormat == PVMF_MIME_M4V ||
            aFormat == PVMF_MIME_H2631998 ||
            aFormat == PVMF_MIME_H2632000)
    {
        status = CheckMaxDuration(aTimestamp);
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxDuration failed"));
            return status;
        }
        else if (status == PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum duration reached"));
            return status;
        }

        for (i = 0; i < aFrame.size(); i++)
        {
            size = aFrame[i].len;
            status = CheckMaxFileSize(size);
            if (status == PVMFFailure)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxFileSize failed"));
                return status;
            }
            else if (status == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum file size reached"));
                return status;
            }

            //No data for some reason.
            if (size == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_NOTICE,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: no data in frag!"));
                return PVMFSuccess;
            }
        }
        uint8 codingType = CODING_TYPE_P;

        if (iRealTimeTS)
        {
            if (aTimestamp <= aPort->GetLastTS())
            {
                aTimestamp = aPort->GetLastTS() + 1;
            }

            aPort->SetLastTS(aTimestamp);
        }

        //iSyncSample is obtained from the marker info
        //to identify the I Frame
        if (iSyncSample)
        {
            codingType = CODING_TYPE_I;
        }

        // Format: mtb (1) | layer_id (3) | coding_type (2) | ref_select_code (2)
        // flags |= ((stream->iHintTrack.MTB & 0x01) << 7);
        // flags |= ((stream->iHintTrack.LayerID & 0x07) << 4);
        flags |= ((codingType & 0x03) << 2);
        // flags |= (stream->iHintTrack.RefSelCode & 0x03);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMp4FFComposerNode::AddMemFragToTrack: Calling addSampleToTrack(%d, 0x%x, %d, %d, %d)",
                         aTrackId, data, size, aTimestamp, flags));

        LOGDATATRAFFIC((0, "PVMp4FFComposerNode::AddMemFragToTrack: TrackID=%d, Size=%d, TS=%d, Flags=%d, Mime=%s",
                        aTrackId, size, aTimestamp, flags, aPort->GetMimeType().get_cstr()));

        if (aFormat == PVMF_MIME_3GPP_TIMEDTEXT)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Calling addtextSampleToTrack(%d, 0x%x, %d, %d, %d)",
                             aTrackId, data, size, aTimestamp, flags));
            int32 index = iText_sdIndex;

            if (index >= 0)
            {
#if PROFILING_ON
                uint32 start = OsclTickCount::TickCount();
#endif
                if (!iMpeg4File->addTextSampleToTrack(aTrackId, aFrame, aTimestamp, flags, index, NULL))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addTextSampleToTrack for Timed Text failed"));
                    return PVMFFailure;
                }
                iSampleInTrack = true;
#if PROFILING_ON
                uint32 stop = OsclTickCount::TickCount();
                uint32 comptime = OsclTickCount::TicksToMsec(stop - start);
                uint32 dataSize = 0;
                for (uint32 ii = 0; ii < aFrame.size(); ii++)
                {
                    dataSize += aFrame[ii].len;
                }
                GenerateDiagnostics(comptime, dataSize);
#endif
            }
        }
        else
        {

#if PROFILING_ON
            uint32 start = OsclTickCount::TickCount();
#endif

#ifdef _TEST_AE_ERROR_HANDLING

            if (1 == iErrorAddSample)
            {
                if (iTestFileSize <= iFileSize) //iTestFileSize set in sendProgressReport()
                {
                    if (!iMpeg4File->addSampleToTrack(aTrackId, aFrame, aTimestamp, flags))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
                        return PVMFFailure;
                    }
                }
            }
            else if (2 == iErrorAddSample)
            {

                if (aTimestamp <= iFileDuration)
                {
                    if (!iMpeg4File->addSampleToTrack(aTrackId, aFrame, aTimestamp, flags))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
                        return PVMFFailure;
                    }
                }
            }
            else
            {
                if (!iMpeg4File->addSampleToTrack(aTrackId, aFrame, aTimestamp, flags))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
                    return PVMFFailure;
                }
            }

#else
            if (!iMpeg4File->addSampleToTrack(aTrackId, aFrame, aTimestamp, flags))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
                return PVMFFailure;
            }
#endif
            iSampleInTrack = true;
#ifdef _TEST_AE_ERROR_HANDLING
            if (iErrorHandlingAddMemFrag == true)
            {
                return PVMFFailure; //Just to trigger error handling
            }
#endif

#if PROFILING_ON
            uint32 stop = OsclTickCount::TickCount();
            uint32 comptime = OsclTickCount::TicksToMsec(stop - start);
            uint32 dataSize = 0;
            for (uint32 ii = 0; ii < aFrame.size(); ii++)
            {
                dataSize += aFrame[ii].len;
            }
            GenerateDiagnostics(comptime, dataSize);
#endif
        }


        // Send progress report after sample is successfully added
        SendProgressReport(aTimestamp);

#if PROFILING_ON
        ++(stats->iNumFrames);
        stats->iDuration = aTimestamp;
#endif
    }

    else if ((aFormat == PVMF_MIME_AMR_IETF) ||
             (aFormat == PVMF_MIME_AMRWB_IETF))
    {
        if (iRealTimeTS)
        {
            if (((int32) aTimestamp - (int32) aPort->GetLastTS()) < 20)
            {
                aTimestamp = aPort->GetLastTS() + 20;
            }

            aPort->SetLastTS(aTimestamp);
        }

        uint32 bytesProcessed = 0;
        uint32 frameSize = 0;
        Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> amrfrags;
        for (i = 0; i < aFrame.size(); i++)
        {
            bytesProcessed = 0;
            size = aFrame[i].len;
            data = OSCL_REINTERPRET_CAST(uint8*, aFrame[i].ptr);
            // Parse audio data and add one 20ms frame to track at a time
            while (bytesProcessed < size)
            {
                // Check for max duration
                status = CheckMaxDuration(aTimestamp);
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxDuration failed"));
                    return status;
                }
                else if (status == PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum duration reached"));
                    return status;
                }

                // Update clock converter
                iClockConverter.set_timescale(timeScale);
                iClockConverter.set_clock_other_timescale(aTimestamp, 1000);

                // Check max file size
                int32 frSize = GetIETFFrameSize(data[0], aPort->GetCodecType());
                if (frSize == -1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - Frame Type Not Supported - Skipping"));
                    LOGDATATRAFFIC((0, "PVMp4FFComposerNode::AddMemFragToTrack - Invalid Frame: TrackID=%d, Byte=0x%x, Mime=%s",
                                    aTrackId, data[0], aPort->GetMimeType().get_cstr()));
                    return PVMFFailure;
                }
                frameSize = (uint32)frSize;

                status = CheckMaxFileSize(frameSize);
                if (status == PVMFFailure)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxFileSize failed"));
                    return status;
                }
                else if (status == PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum file size reached"));
                    return status;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Calling addSampleToTrack(%d, 0x%x, %d, %d, %d)",
                                 aTrackId, data, frameSize, iClockConverter.get_current_timestamp(), flags));


                OsclMemoryFragment amr_memfrag;
                amr_memfrag.len = frameSize;
                amr_memfrag.ptr = data;
                amrfrags.push_back(amr_memfrag);

#if PROFILING_ON
                uint32 start = OsclTickCount::TickCount();
#endif
                uint32 amrts = iClockConverter.get_current_timestamp();

                LOGDATATRAFFIC((0, "PVMp4FFComposerNode::AddMemFragToTrack: TrackID=%d, Size=%d, TS=%d, Flags=%d, Mime=%s",
                                aTrackId, frameSize, amrts, flags, aPort->GetMimeType().get_cstr()));

                if (!iMpeg4File->addSampleToTrack(aTrackId, amrfrags, amrts, flags))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
                    return PVMFFailure;
                }
                iSampleInTrack = true;
#if PROFILING_ON
                uint32 stop = OsclTickCount::TickCount();
                uint32 comptime = OsclTickCount::TicksToMsec(stop - start);
                uint32 dataSize = 0;
                for (uint32 ii = 0; ii < amrfrags.size(); ii++)
                {
                    dataSize += amrfrags[ii].len;
                }
                GenerateDiagnostics(comptime, dataSize);

#endif

                // Send progress report after sample is successfully added
                SendProgressReport(aTimestamp);

#if PROFILING_ON
                ++(stats->iNumFrames);
                stats->iDuration = aTimestamp;
#endif
                data += frameSize;
                bytesProcessed += frameSize;
                aTimestamp += 20;
                amrfrags.clear();
            }
        }
        if (iRealTimeTS)
        {
            aPort->SetLastTS(aTimestamp - 20);
        }
    }

    else if (aFormat == PVMF_MIME_MPEG4_AUDIO)
    {
        status = CheckMaxDuration(aTimestamp);
        if (status == PVMFFailure)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxDuration failed"));
            return status;
        }
        else if (status == PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum duration reached"));
            return status;
        }

        for (i = 0; i < aFrame.size(); i++)
        {
            size = aFrame[i].len;
            status = CheckMaxFileSize(size);
            if (status == PVMFFailure)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - CheckMaxFileSize failed"));
                return status;
            }
            else if (status == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: Maxmimum file size reached"));
                return status;
            }

            //No data for some reason.
            if (size == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_NOTICE,
                                (0, "PVMp4FFComposerNode::AddMemFragToTrack: no data in frag!"));
                return PVMFSuccess;
            }
        }

        if (iRealTimeTS)
        {
            if (aTimestamp <= aPort->GetLastTS())
            {
                aTimestamp = aPort->GetLastTS() + 1;
            }

            aPort->SetLastTS(aTimestamp);
        }

        iClockConverter.set_timescale(timeScale);
        iClockConverter.set_clock_other_timescale(aTimestamp, 1000);
        uint32 aacTS = iClockConverter.get_current_timestamp();

        if (!iMpeg4File->addSampleToTrack(aTrackId, aFrame, aacTS, flags))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMp4FFComposerNode::AddMemFragToTrack: Error - addSampleToTrack failed"));
            return PVMFFailure;
        }
        iSampleInTrack = true;
        // Send progress report after sample is successfully added
        SendProgressReport(aTimestamp);

#if PROFILING_ON
        ++(stats->iNumFrames);
        stats->iDuration = aTimestamp;
#endif
    }

    return PVMFSuccess;
}


void PVMp4FFComposerNode::GenerateDiagnostics(uint32 aTime, uint32 aSize)
{
#if PROFILING_ON
    if ((iMinSampleAddTime > aTime) || (0 == iMinSampleAddTime))
    {
        iMinSampleAddTime = aTime;
    }
    if (iMaxSampleAddTime < aTime)
    {
        iMaxSampleAddTime = aTime;
    }

    if ((iMinSampleSize > aSize) || (0 == iMinSampleSize))
    {
        iMinSampleSize = aSize;
    }
    if (iMaxSampleSize < aSize)
    {
        iMaxSampleSize = aSize;
    }
    iNumSamplesAdded++;
#endif
    OSCL_UNUSED_ARG(aTime);
    OSCL_UNUSED_ARG(aSize);
}
//////////////////////////////////////////////////////////////////////////////////
int32 PVMp4FFComposerNode::GetIETFFrameSize(uint8 aFrameType,
        int32 aCodecType)
{
    uint8 frameType = (uint8)(aFrameType >> 3) & 0x0f;
    if (aCodecType == CODEC_TYPE_AMR_AUDIO)
    {
        // Find frame size for each frame type
        switch (frameType)
        {
            case 0: // AMR 4.75 Kbps
                return 13;
            case 1: // AMR 5.15 Kbps
                return 14;
            case 2: // AMR 5.90 Kbps
                return 16;
            case 3: // AMR 6.70 Kbps
                return 18;
            case 4: // AMR 7.40 Kbps
                return 20;
            case 5: // AMR 7.95 Kbps
                return 21;
            case 6: // AMR 10.2 Kbps
                return 27;
            case 7: // AMR 12.2 Kbps
                return 32;
            case 8: // AMR Frame SID
                return 6;
            case 9: // AMR Frame GSM EFR SID
                return 7;
            case 10:// AMR Frame TDMA EFR SID
            case 11:// AMR Frame PDC EFR SID
                return 6;
            case 15: // AMR Frame No Data
                return 1;
            default: // Error - For Future Use
                return -1;
        }
    }
    else if (aCodecType == CODEC_TYPE_AMR_WB_AUDIO)
    {
        // Find frame size for each frame type
        switch (frameType)
        {
            case 0: // AMR-WB 6.60 Kbps
                return 18;
            case 1: // AMR-WB 8.85 Kbps
                return 24;
            case 2: // AMR-WB 12.65 Kbps
                return 33;
            case 3: // AMR-WB 14.25 Kbps
                return 37;
            case 4: // AMR-WB 15.85 Kbps
                return 41;
            case 5: // AMR-WB 18.25 Kbps
                return 47;
            case 6: // AMR-WB 19.85 Kbps
                return 51;
            case 7: // AMR-WB 23.05 Kbps
                return 59;
            case 8: // AMR-WB 23.85 Kbps
                return 61;
            case 9: // AMR-WB SID Frame
                return 6;
            case 10: //Reserved
            case 11: //Reserved
            case 12: //Reserved
            case 13: //Reserved
                return -1;
            case 14: // AMR-WB Frame Lost
            case 15: // AMR-WB Frame No Data
                return 1;
            default: // Error - For Future Use
                return -1;
        }
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////////////////
//                 Progress and max size / duration routines
//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::SendProgressReport(uint32 aTimestamp)
{
    if (iDurationReportEnabled &&
            aTimestamp >= iNextDurationReport)
    {
        iNextDurationReport = aTimestamp - (aTimestamp % iDurationReportFreq) + iDurationReportFreq;
        ReportInfoEvent(PVMF_COMPOSER_DURATION_PROGRESS, (OsclAny*)aTimestamp);
    }
    else if (iFileSizeReportEnabled)
    {
        uint32 metaDataSize = 0;
        uint32 mediaDataSize = 0;
        uint32 fileSize = 0;

        iMpeg4File->getTargetFileSize(metaDataSize, mediaDataSize);
        fileSize = metaDataSize + mediaDataSize;

        if (fileSize >= iNextFileSizeReport)
        {
            iNextFileSizeReport = fileSize - (fileSize % iFileSizeReportFreq) + iFileSizeReportFreq;
            ReportInfoEvent(PVMF_COMPOSER_FILESIZE_PROGRESS, (OsclAny*)fileSize);
        }
#ifdef _TEST_AE_ERROR_HANDLING
        iTestFileSize = fileSize; //iTestTimeStamp to fail the addSampleTrack() once a particulare time duration is reached as specified in testapp.
#endif
    }

    return PVMFSuccess;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::CheckMaxFileSize(uint32 aFrameSize)
{
    if (iMaxFileSizeEnabled)
    {
        uint32 metaDataSize = 0;
        uint32 mediaDataSize = 0;
        iMpeg4File->getTargetFileSize(metaDataSize, mediaDataSize);

        if ((metaDataSize + mediaDataSize + aFrameSize) >= iMaxFileSize)
        {
            // Finalized output file
            if (iSampleInTrack)
            {
                iSampleInTrack = false;
                if (RenderToFile() != PVMFSuccess)
                    return PVMFFailure;
            }

            ReportInfoEvent(PVMF_COMPOSER_MAXFILESIZE_REACHED, NULL);
            return PVMFSuccess;
        }

        return PVMFPending;
    }

    return PVMFErrNotSupported;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::CheckMaxDuration(uint32 aTimestamp)
{
    //if(!iInfoObserver)
    //	return PVMFFailure;

    if (iMaxDurationEnabled)
    {
        if (aTimestamp >= iMaxTimeDuration)
        {
            // Finalize output file
            if (iSampleInTrack)
            {
                iSampleInTrack = false;
                if (RenderToFile() != PVMFSuccess)
                    return PVMFFailure;
            }


            ReportInfoEvent(PVMF_COMPOSER_MAXDURATION_REACHED, NULL);
            return PVMFSuccess;
        }

        return PVMFPending;
    }

    return PVMFErrNotSupported;
}

////////////////////////////////////////////////////////////////////////////
//                   Event reporting routines.
////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerNode::SetState(TPVMFNodeInterfaceState aState)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode::SetState: aState=%d", aState));
    PVMFNodeInterface::SetState(aState);
}

void PVMp4FFComposerNode::ReportErrorEvent(PvmfMp4FFCNError aErrorEvent, OsclAny* aEventData)
{
    LOG_ERR((0, "PVMp4FFComposerNode:ReportErrorEvent: aEventType=%d, aEventData=0x%x", aErrorEvent, aEventData));
    switch (aErrorEvent)
    {
        case PVMF_MP4FFCN_ERROR_FINALIZE_OUTPUT_FILE_FAILED:
        case PVMF_MP4FFCN_ERROR_ADD_SAMPLE_TO_TRACK_FAILED:
            PVMFNodeInterface::ReportErrorEvent(PVMFErrResourceConfiguration, aEventData);
            break;
        default:
            PVMFNodeInterface::ReportErrorEvent(PVMFFailure, aEventData);
            break;
    }
}

void PVMp4FFComposerNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerNode:ReportInfoEvent: aEventType=%d, aEventData=0x%x", aEventType, aEventData));
    PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}



void PVMp4FFComposerNode::LogDiagnostics()
{
#if PROFILING_ON
    oDiagnosticsLogged = true;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iDiagnosticsLogger, PVLOGMSG_DEBUG, (0, "PVMp4FFComposerNode Stats:Sample Add time (Min:%d, Max:%d), Sample Size(Min:%d, Max:%d), number of samples added:%d\n", iMinSampleAddTime, iMaxSampleAddTime, iMinSampleSize, iMaxSampleSize, iNumSamplesAdded));
#endif
}

int32 PVMp4FFComposerNode::StoreCurrentCommand(PVMp4FFCNCmdQueue& aCurrentCmd, PVMp4FFCNCmd& aCmd, PVMp4FFCNCmdQueue& aCmdQueue)
{
    int32 err = 0;
    OSCL_TRY(err, aCurrentCmd.StoreL(aCmd););
    OSCL_FIRST_CATCH_ANY(err,
                         CommandComplete(aCmdQueue, aCmd, PVMFErrNoMemory);
                         return err;
                        );
    return err;
}





