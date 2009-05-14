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
#ifndef PVMF_MP4FFPARSER_NODE_H_INCLUDED
#define PVMF_MP4FFPARSER_NODE_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif

#ifndef OSCL_BIN_STREAM_H_INCLUDED
#include "oscl_bin_stream.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
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

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVMF_MP4FFPARSER_DEFS_H_INCLUDED
#include "pvmf_mp4ffparser_defs.h"
#endif

#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif

#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif

#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

#ifndef PVMF_TRACK_LEVEL_INFO_EXTENSION_H_INCLUDED
#include "pvmf_track_level_info_extension.h"
#endif

#ifndef PVMF_MP4_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#include "pvmf_mp4_progdownload_support_extension.h"
#endif

#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif

#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_access_interface.h"
#endif

#ifndef PVMF_LOCAL_DATA_SOURCE_H_INCLUDED
#include "pvmf_local_data_source.h"
#endif

#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_TIMEDTEXT_H_INCLUDED
#include "pvmf_timedtext.h"
#endif

#ifndef PV_GAU_H_
#include "pv_gau.h"
#endif

#ifndef PVMF_FORMAT_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#include "pvmf_format_progdownload_support_extension.h"
#endif

#ifndef PVMF_DOWNLOAD_PROGRESS_EXTENSION_H
#include "pvmf_download_progress_interface.h"
#endif

#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif

#ifndef PVMF_MP4FFPARSER_OUTPORT_H_INCLUDED
#include "pvmf_mp4ffparser_outport.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

#ifndef PVMF_DATA_SOURCE_DIRECTION_CONTROL_H_INCLUDED
#include "pvmf_data_source_direction_control.h"
#endif

/**
* Node command handling
*/

class PVMFMP4ParserNodeLoggerDestructDealloc : public OsclDestructDealloc
{
    public:
        void destruct_and_dealloc(OsclAny* ptr)
        {
            PVLoggerAppender* p = OSCL_REINTERPRET_CAST(PVLoggerAppender*, ptr);
            BinaryFileAppender* binPtr = OSCL_REINTERPRET_CAST(BinaryFileAppender*, p);
            if (!binPtr)
                return;
            OSCL_DELETE(binPtr);
        }
};

#define PVMF_MP4FFPARSERNODE_UNDERFLOW_STATUS_TIMER_ID 1

#define PVMF_MP4FFPARSERNODE_MAX_CPM_METADATA_KEYS 256

#define NORMAL_PLAYRATE 100000

typedef PVMFGenericNodeCommand<OsclMemAllocator> PVMFMP4FFParserNodeCommandBase;

enum PVMFMP4FFParserNodeCommandType
{
    PVMP4FF_NODE_CMD_SETDATASOURCEPOSITION = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVMP4FF_NODE_CMD_QUERYDATASOURCEPOSITION
    , PVMP4FF_NODE_CMD_SETDATASOURCERATE
    , PVMP4FF_NODE_CMD_GETNODEMETADATAKEYS
    , PVMP4FF_NODE_CMD_GETNODEMETADATAVALUES
    , PVMP4FF_NODE_CMD_GET_LICENSE_W
    , PVMP4FF_NODE_CMD_GET_LICENSE
    , PVMP4FF_NODE_CMD_CANCEL_GET_LICENSE
    , PVMF_MP4_PARSER_NODE_CAPCONFIG_SETPARAMS
    , PVMP4FF_NODE_CMD_SETDATASOURCEDIRECTION
};

class PVMFMP4FFParserNodeCommand : public PVMFMP4FFParserNodeCommandBase
{
    public:

        // Constructor and parser for GetNodeMetadataKeys
        void Construct(PVMFSessionId s, int32 cmd
                       , PVMFMetadataList& aKeyList
                       , uint32 aStartingIndex
                       , int32 aMaxEntries
                       , char* aQueryKey
                       , const OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*)aStartingIndex;
            iParam3 = (OsclAny*)aMaxEntries;
            if (aQueryKey)
            {
                //allocate a copy of the query key string.
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                iParam4 = str.ALLOC_AND_CONSTRUCT(aQueryKey);
            }
        }
        void Parse(PVMFMetadataList*& MetaDataListPtr, uint32 &aStartingIndex, int32 &aMaxEntries, char*& aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = (uint32)iParam2;
            aMaxEntries = (int32)iParam3;
            aQueryKey = NULL;
            if (iParam4)
            {
                OSCL_HeapString<OsclMemAllocator>* keystring = (OSCL_HeapString<OsclMemAllocator>*)iParam4;
                aQueryKey = keystring->get_str();
            }
        }

        // Constructor and parser for GetNodeMetadataValue
        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartIndex, int32 aMaxEntries, const OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*) & aValueList;
            iParam3 = (OsclAny*)aStartIndex;
            iParam4 = (OsclAny*)aMaxEntries;

        }
        void Parse(PVMFMetadataList* &aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList, uint32 &aStartingIndex, int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = (uint32)iParam3;
            aMaxEntries = (int32)iParam4;
        }

        /* Constructor and parser for setParametersAsync */
        void Construct(PVMFSessionId s, int32 cmd, PvmiMIOSession aSession,
                       PvmiKvp* aParameters, int num_elements,
                       PvmiKvp*& aRet_kvp, OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aSession;
            iParam2 = (OsclAny*)aParameters;
            iParam3 = (OsclAny*)num_elements;
            iParam4 = (OsclAny*) & aRet_kvp;
        }
        void Parse(PvmiMIOSession& aSession, PvmiKvp*& aParameters,
                   int &num_elements, PvmiKvp** &ppRet_kvp)
        {
            aSession = (PvmiMIOSession)iParam1;
            aParameters = (PvmiKvp*)iParam2;
            num_elements = (int)iParam3;
            ppRet_kvp = (PvmiKvp**)iParam4;
        }

        // Constructor and parser for SetDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS,
                       bool aSeekToSyncPoint, uint32 aStreamID, const OsclAny*aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*) & aActualMediaDataTS;
            iParam4 = (OsclAny*)aSeekToSyncPoint;
            iParam5 = (OsclAny*)aStreamID;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, PVMFTimestamp*& aActualMediaDataTS, bool& aSeekToSyncPoint, uint32& aStreamID)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aSeekToSyncPoint = (iParam4) ? true : false;
            aStreamID = (uint32)iParam5;
        }

        // Constructor and parser for QueryDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp &aActualNPT,
                       bool aSeekToSyncPoint, const OsclAny*aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        // Constructor and parser for QueryDataSourcePosition with aSeekPointBeforeTargetNPT and aSeekPointAfterTargetNPT
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT,
                       PVMFTimestamp& aSeekPointBeforeTargetNPT, PVMFTimestamp& aSeekPointAfterTargetNPT,
                       const OsclAny*aContext, bool aSeekToSyncPoint)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = (OsclAny*) & aSeekPointBeforeTargetNPT;
            iParam5 = (OsclAny*) & aSeekPointAfterTargetNPT;
        }


        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3) ? true : false;
        }

        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aSeekPointBeforeTargetNPT,
                   bool& aSeekToSyncPoint, PVMFTimestamp*& aSeekPointAfterTargetNPT)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aSeekPointBeforeTargetNPT = (PVMFTimestamp*)iParam4;
            aSeekPointAfterTargetNPT = (PVMFTimestamp*)iParam5;
            aSeekToSyncPoint = (iParam3) ? true : false;
        }
        /* Constructor and parser for SetDataSourceDirection */
        void Construct(PVMFSessionId s, int32 cmd, int32 aDirection,
                       PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS,
                       PVMFTimebase* aTimebase, OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aDirection;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*) & aActualMediaDataTS;
            iParam4 = (OsclAny*)aTimebase;
            iParam5 = NULL;
        }
        void Parse(int32& aDirection,
                   PVMFTimestamp*& aActualNPT,
                   PVMFTimestamp*& aActualMediaDataTS,
                   PVMFTimebase*& aTimebase)
        {
            aDirection = (int32)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aTimebase = (PVMFTimebase*)iParam4;
        }

        // Constructor and parser for SetDataSourceRate
        void Construct(PVMFSessionId s, int32 cmd, int32 aRate, PVMFTimebase* aTimebase, const OsclAny*aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aRate;
            iParam2 = (OsclAny*)aTimebase;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(int32& aRate, PVMFTimebase*& aTimebase)
        {
            aRate = (int32)iParam1;
            aTimebase = (PVMFTimebase*)iParam2;
        }

        /* Constructor and parser for GetLicenseW */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_wString& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_wString*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_wString*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }

        /* Constructor and parser for GetLicense */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_String& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFMP4FFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_String*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_String*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }


        // Need to overlaod the base Destroy routine to cleanup metadata key.
        void Destroy()
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Destroy();
            switch (iCmd)
            {
                case PVMP4FF_NODE_CMD_GETNODEMETADATAKEYS:
                    if (iParam4)
                    {
                        //cleanup the allocated string
                        Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                        str.destruct_and_dealloc(iParam4);
                    }
                    break;
                default:
                    break;
            }
        }

        // Need to overlaod the base Copy routine to copy metadata key.
        void Copy(const PVMFGenericNodeCommand<OsclMemAllocator>& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            switch (aCmd.iCmd)
            {
                case PVMP4FF_NODE_CMD_GETNODEMETADATAKEYS:
                    if (aCmd.iParam4)
                    {
                        //copy the allocated string
                        OSCL_HeapString<OsclMemAllocator>* aStr = (OSCL_HeapString<OsclMemAllocator>*)aCmd.iParam4;
                        Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                        iParam4 = str.ALLOC_AND_CONSTRUCT(*aStr);
                    }
                    break;
                default:
                    break;
            }
        }

};
typedef PVMFNodeCommandQueue<PVMFMP4FFParserNodeCommand, OsclMemAllocator> PVMFMP4FFParserNodeCmdQueue;

//Command queue type
typedef PVMFNodeCommandQueue<PVMFMP4FFParserNodeCommand, OsclMemAllocator> PVMFMP4FFParserNodeCmdQueue;


/**
* The Node class
*/
class IMpeg4File;
class PVMFMP4FFParserOutPort;
class PVMFMP4FFPortIter;
class PVLogger;
class PVMFMediaClock;

enum BaseKeys_SelectionType
{
    INVALID = 0,
    NET,
    FILE_IO
};

#define PVMFFFPARSERNODE_MAX_NUM_TRACKS	6

class PVMFMP4FFParserNode : public OsclTimerObject,
            public PVMFNodeInterface,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PvmfDataSourcePlaybackControlInterface,
            public PVMFMetadataExtensionInterface,
            public PVMFTrackLevelInfoExtensionInterface,
            public PVMFCPMStatusObserver,
            public PvmiDataStreamObserver,
            public PVMIDatastreamuserInterface,
            public PVMFFormatProgDownloadSupportInterface,
            public OsclTimerObserver,
            public PVMFCPMPluginLicenseInterface,
            public PvmiCapabilityAndConfig,
            public PVMFMediaClockStateObserver, // For observing the playback clock states
            public PvmfDataSourceDirectionControlInterface
{
    public:
        PVMFMP4FFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);
        virtual ~PVMFMP4FFParserNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        PVMFCommandId QueryUUID(PVMFSessionId aSessionId,
                                const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                bool aExactUuidsOnly = false,
                                const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId aSessionId,
                                     const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr,
                                     const OsclAny* aContext = NULL);

        PVMFCommandId RequestPort(PVMFSessionId aSessionId, int32 aPortTag,
                                  const PvmfMimeString* aPortConfig = NULL,
                                  const OsclAny* aContext = NULL);
        PVMFStatus ReleasePort(PVMFSessionId aSessionId, PVMFPortInterface& aPort,
                               const OsclAny* aContext = NULL);

        PVMFCommandId Init(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);

        PVMFCommandId CancelAllCommands(PVMFSessionId aSessionId, const OsclAny* aContextData = NULL);
        PVMFCommandId CancelCommand(PVMFSessionId aSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);
        PVMFStatus QueryInterfaceSync(PVMFSessionId aSession,
                                      const PVUuid& aUuid,
                                      PVInterface*& aInterfacePtr);

        /* cap config interface */
        virtual void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            ciObserver = aObserver;
        }

        virtual PVMFStatus getParametersSync(PvmiMIOSession aSession,
                                             PvmiKeyType aIdentifier,
                                             PvmiKvp*& aParameters,
                                             int& aNumParamElements,
                                             PvmiCapabilityContext aContext);
        virtual PVMFStatus releaseParameters(PvmiMIOSession aSession,
                                             PvmiKvp* aParameters,
                                             int num_elements);
        virtual void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        virtual void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                          PvmiKvp* aParameters, int num_parameter_elements);
        virtual void DeleteContext(PvmiMIOSession aSession,
                                   PvmiCapabilityContext& aContext);
        virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                       int num_elements, PvmiKvp * & aRet_kvp);
        virtual PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
                PvmiKvp* aParameters,
                int num_elements,
                PvmiKvp*& aRet_kvp,
                OsclAny* context = NULL);
        virtual uint32 getCapabilityMetric(PvmiMIOSession aSession);
        virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                                                PvmiKvp* aParameters,
                                                int num_elements);

        PVMFStatus GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements,
                                      int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus VerifyAndSetConfigParameter(int index, PvmiKvp& aParameter, bool set);


        // From PVInterface
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        // From PVMFDataSourceInitializationExtensionInterface
        PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL, PVMFFormatType& aSourceFormat, OsclAny* aSourceData);
        PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        // From PVMFTrackSelectionExtensionInterface
        PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        // From PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        uint32 GetNumImotionMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, int32 aMaxKeyEntries,
                                          char* aQueryKeyString = NULL, const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                            Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, int32 aMaxValueEntries, const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, uint32 aEndKeyIndex);
        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, uint32 aEndValueIndex);

        // From PvmfDataSourcePlaybackControlInterface
        PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                                            PVMFTimestamp& aActualMediaDataTS, bool aSeekToSyncPoint = true, uint32 aStreamID = 0, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                                              bool aSeekToSyncPoint = true, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT,
                                              PVMFTimestamp& aSeekPointBeforeTargetNPT, PVMFTimestamp& aSeekPointAfterTargetNPT,  OsclAny* aContext = NULL, bool aSeekToSyncPoint = true);

        PVMFCommandId SetDataSourceRate(PVMFSessionId aSession, int32 aRate, PVMFTimebase* aTimebase = NULL, OsclAny* aContext = NULL);
        PVMFCommandId SetDataSourceDirection(PVMFSessionId aSessionId, int32 aDirection, PVMFTimestamp& aActualNPT,
                                             PVMFTimestamp& aActualMediaDataTS, PVMFTimebase* aTimebase, OsclAny* aContext);

        // From PVMFTrackLevelInfoExtensionInterface
        PVMFStatus GetAvailableTracks(Oscl_Vector<PVMFTrackInfo, OsclMemAllocator>& aTracks);
        PVMFStatus GetTimestampForSampleNumber(PVMFTrackInfo& aTrackInfo, uint32 aSampleNum, PVMFTimestamp& aTimestamp);
        PVMFStatus GetSampleNumberForTimestamp(PVMFTrackInfo& aTrackInfo, PVMFTimestamp aTimestamp, uint32& aSampleNum);
        PVMFStatus GetNumberOfSyncSamples(PVMFTrackInfo& aTrackInfo, int32& aNumSyncSamples);
        PVMFStatus GetSyncSampleInfo(PVMFTrackInfo& aTrackInfo, PVMFSampleNumTSList& aList, uint32 aStartIndex = 0, int32 aMaxEntries = -1);
        PVMFStatus GetSyncSampleInfo(PVMFSampleNumTSList& aList, PVMFTrackInfo& aTrackInfo, int32 aTargetTimeInMS, uint32 aHowManySamples = 1);
        PVMFStatus GetTimestampForDataPosition(PVMFTrackInfo& aTrackInfo, uint32 aDataPosition, PVMFTimestamp& aTimestamp);
        PVMFStatus GetDataPositionForTimestamp(PVMFTrackInfo& aTrackInfo, PVMFTimestamp aTimestamp, uint32& aDataPosition);


        // From PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // From PVMFCPMStatusObserver
        void CPMCommandCompleted(const PVMFCmdResp& aResponse);

        /* From PVMIDatastreamuserInterface */
        void PassDatastreamFactory(PVMFDataStreamFactory& aFactory,
                                   int32 aFactoryTag,
                                   const PvmfMimeString* aFactoryConfig = NULL);
        void PassDatastreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObserver);


        /* From PVMFFormatProgDownloadSupportInterface */
        int32 convertSizeToTime(uint32 fileSize, uint32& aNPTInMS);
        void setFileSize(const uint32 aFileSize);
        void setDownloadProgressInterface(PVMFDownloadProgressInterface*);
        void playResumeNotification(bool aDownloadComplete);
        void notifyDownloadComplete();

        // From OsclTimer
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        /* From PVMFCPMPluginLicenseInterface */
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OSCL_wString& aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OSCL_String&  aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData) ;

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData);

        PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                                       , PVMFCommandId aCmdId
                                       , OsclAny* aContextData);

        PVMFStatus GetLicenseStatus(
            PVMFCPMLicenseStatus& aStatus) ;

        //from PVMFMediaClockStateObserver
        void ClockStateUpdated();
        void NotificationsInterfaceDestroyed();

    private:
        // from OsclTimerObject
        void Run();

        //Command processing
        PVMFCommandId QueueCommandL(PVMFMP4FFParserNodeCommand& aCmd);
        void MoveCmdToCurrentQueue(PVMFMP4FFParserNodeCommand& aCmd);
        void MoveCmdToCancelQueue(PVMFMP4FFParserNodeCommand& aCmd);
        void ProcessCommand();
        void CommandComplete(PVMFMP4FFParserNodeCmdQueue& aCmdQueue, PVMFMP4FFParserNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aData = NULL, PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        bool FlushPending();
        PVMFMP4FFParserNodeCmdQueue iInputCommands;
        PVMFMP4FFParserNodeCmdQueue iCurrentCommand;
        PVMFMP4FFParserNodeCmdQueue iCancelCommand;

        // Event reporting
        void ReportMP4FFParserErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        void ReportMP4FFParserInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        void ChangeNodeState(TPVMFNodeInterfaceState aNewState);

        // Node command handlers
        PVMFStatus DoQueryUuid(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoQueryInterface(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoRequestPort(PVMFMP4FFParserNodeCommand& aCmd, PVMFPortInterface*&);
        void GetTrackMaxParameters(PVMFFormatType aFormatType, uint32& aMaxDataSize, uint32& aMaxQueueDepth);
        PVMFStatus DoReleasePort(PVMFMP4FFParserNodeCommand& aCmd);

        PVMFStatus DoInit(PVMFMP4FFParserNodeCommand& aCmd);
        bool ParseMP4File(PVMFMP4FFParserNodeCmdQueue& aCmdQ,
                          PVMFMP4FFParserNodeCommand& aCmd);
        void PushToAvailableMetadataKeysList(const char* aKeystr, char* aOptionalParam = NULL);
        PVMFStatus InitMetaData();
        PVMFStatus InitImotionMetaData();
        uint32 CountImotionMetaDataKeys();
        int32 CountMetaDataKeys();
        void CompleteInit(PVMFMP4FFParserNodeCmdQueue& aCmdQ,
                          PVMFMP4FFParserNodeCommand& aCmd);
        void CompleteCancelAfterInit();


        PVMFStatus DoPrepare(PVMFMP4FFParserNodeCommand& aCmd);
        void CompletePrepare(PVMFStatus aStatus);
        PVMFStatus DoStart(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoStop(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoFlush(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoPause(PVMFMP4FFParserNodeCommand& aCmd);

        PVMFStatus DoReset(PVMFMP4FFParserNodeCommand& aCmd);
        void CompleteReset(PVMFMP4FFParserNodeCmdQueue& aCmdQ, PVMFMP4FFParserNodeCommand& aCmd);

        PVMFStatus DoCancelAllCommands(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoCancelCommand(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoCancelCurrentCommand(PVMFMP4FFParserNodeCommand& aCmd);

        // For metadata extention interface
        PVMFStatus DoGetMetadataKeys(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus CompleteGetMetadataKeys(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoGetImotionMetadataValues(PVMFMP4FFParserNodeCommand& aCmd, int32 &numentriesadded);
        PVMFStatus DoGetMetadataValues(PVMFMP4FFParserNodeCommand& aCmd);
        void CompleteGetMetaDataValues();
        int32 AddToValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, PvmiKvp& aNewValue);
        PVMFStatus GetIndexParamValues(const char* aString, uint32& aStartIndex, uint32& aEndIndex);
        void DeleteAPICStruct(PvmfApicStruct*& aAPICStruct);
        PVMFStatus GetVideoFrameWidth(uint32 aId, int32& aWidth, int32& aDisplayWidth);
        PVMFStatus GetVideoFrameHeight(uint32 aId, int32& aHeight, int32& aDisplayHeight);
        int32 FindVideoWidth(uint32 aId);
        int32 FindVideoHeight(uint32 aId);
        int32 FindVideoDisplayWidth(uint32 aId);
        int32 FindVideoDisplayHeight(uint32 aId);
        PVMFStatus PopulateVideoDimensions(uint32 aId);
        uint32 GetNumAudioChannels(uint32 aId);
        uint32 GetAudioSampleRate(uint32 aId);
        uint32 GetAudioBitsPerSample(uint32 aId);
        PVMFStatus FindBestThumbnailKeyFrame(uint32 aId, uint32& aKeyFrameNum);

        // For data source position extension interface
        PVMFStatus DoSetDataSourcePosition(PVMFMP4FFParserNodeCommand& aCmd, PVMFStatus &aEventCode, PVUuid &aEventUuid);
        PVMFStatus DoQueryDataSourcePosition(PVMFMP4FFParserNodeCommand& aCmd);
        PVMFStatus DoSetDataSourceRate(PVMFMP4FFParserNodeCommand& aCmd);

        PVMFStatus DoSetDataSourceDirection(PVMFMP4FFParserNodeCommand& aCmd);

        void HandleTrackState();
        bool RetrieveTrackConfigInfo(uint32 aTrackId,
                                     PVMFFormatType aFormatType,
                                     OsclRefCounterMemFrag &aConfig);
        bool RetrieveTrackConfigInfoAndFirstSample(uint32 aTrackId,
                PVMFFormatType aFormatType,
                OsclRefCounterMemFrag &aConfig);
        bool RetrieveTrackData(PVMP4FFNodeTrackPortInfo& aTrackPortInfo);
        bool SendTrackData(PVMP4FFNodeTrackPortInfo& aTrackPortInfo);
        bool GenerateAVCNALGroup(PVMP4FFNodeTrackPortInfo& aTrackPortInfo, OsclSharedPtr<PVMFMediaDataImpl>& aMediaFragGroup);
        bool GenerateAACFrameFrags(PVMP4FFNodeTrackPortInfo& aTrackPortInfo, OsclSharedPtr<PVMFMediaDataImpl>& aMediaFragGroup);
        bool GetAVCNALLength(OsclBinIStreamBigEndian& stream, uint32& lengthSize, int32& len);
        bool UpdateTextSampleEntry(PVMP4FFNodeTrackPortInfo& aTrackPortInfo, uint32 aEntryIndex, PVMFTimedTextMediaData& aTextMediaData);
        bool SendEndOfTrackCommand(PVMP4FFNodeTrackPortInfo& aTrackPortInfo);
        bool SendBeginOfMediaStreamCommand(PVMP4FFNodeTrackPortInfo& aTrackPortInfo);

        // Port processing
        void ProcessPortActivity();
        void QueuePortActivity(const PVMFPortActivity& aActivity);
        PVMFStatus ProcessIncomingMsg(PVMFPortInterface* aPort);
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        Oscl_Vector<PVMFPortActivity, OsclMemAllocator> iPortActivityQueue;

        friend class PVMFMP4FFParserOutPort;

        PVMFFormatType GetFormatTypeFromMIMEType(PvmfMimeString* aMIMEString);

        void ResetAllTracks();
        bool ReleaseAllPorts();
        void RemoveAllCommands();
        void CleanupFileSource();

        // For comparison with download progress clock
        bool checkTrackPosition(PVMP4FFNodeTrackPortInfo& aTrackPortInfo, uint32 numsamples);

        bool GetTrackPortInfoForTrackID(PVMP4FFNodeTrackPortInfo*& aInfo,
                                        uint32 aTrackID);

        bool GetTrackPortInfoForPort(PVMP4FFNodeTrackPortInfo*& aInfo,
                                     PVMFPortInterface* aPort);

        OSCL_wHeapString<OsclMemAllocator> iFilename;
        PVMFFormatType iSourceFormat;
        PVMFMediaClock* iClientPlayBackClock;
        PVMFMediaClockNotificationsInterface *iClockNotificationsInf;
        bool iUseCPMPluginRegistry;
        PVMFLocalDataSource iCPMSourceData;
        PVMFSourceContextData iSourceContextData;
        OsclFileHandle* iFileHandle;
        Oscl_FileServer iFileServer;
        uint32 iParsingMode;
        bool iProtectedFile;
        IMpeg4File* iMP4FileHandle;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;
        uint32 iMP4ParserNodeMetadataValueCount;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iCPMMetadataKeys;
        Oscl_Vector<PVMP4FFNodeTrackPortInfo, OsclMemAllocator> iNodeTrackPortList;
        Oscl_Vector<PVMFTrackInfo, OsclMemAllocator> iSelectedTrackInfoList;
        Oscl_Vector<VideoTrackDimensionInfo, OsclMemAllocator> iVideoDimensionInfoVec;

        PVMFMP4FFPortIter* iPortIter;

        // stream id
        uint32 iStreamID;

        PVMFNodeCapability iCapability;
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iAVCDataPathLogger;
        PVLogger* iClockLogger;
        PVLogger* iDiagnosticsLogger;
        // Reference counter for extension
        uint32 iExtensionRefCount;

        // variables to support download autopause
        OsclSharedPtr<PVMFMediaClock> download_progress_clock;
        PVMFDownloadProgressInterface* download_progress_interface;
        uint32 iDownloadFileSize;
        bool autopaused;

        void DataStreamCommandCompleted(const PVMFCmdResp& aResponse);
        void DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent);
        void DataStreamErrorEvent(const PVMFAsyncEvent& aEvent);

        bool MapMP4ErrorCodeToEventCode(int32 aMP4ErrCode, PVUuid& aEventUUID, int32& aEventCode);

        GAU iGau;

        bool iThumbNailMode;

        // Content Policy Manager related
        bool iSourceContextDataValid;
        bool iPreviewMode;
        PVMFCPM* iCPM;
        PVMFSessionId iCPMSessionID;
        PVMFCPMContentType iCPMContentType;
        PVMFCPMPluginAccessInterfaceFactory* iCPMContentAccessFactory;
        PVMFMetadataExtensionInterface* iCPMMetaDataExtensionInterface;
        PVMFCPMPluginLicenseInterface* iCPMLicenseInterface;
        PVInterface* iCPMLicenseInterfacePVI;
        PVMFCPMPluginAccessUnitDecryptionInterface* iDecryptionInterface;
        PvmiKvp iRequestedUsage;
        PvmiKvp iApprovedUsage;
        PvmiKvp iAuthorizationDataKvp;
        PVMFCPMUsageID iUsageID;
        bool oWaitingOnLicense;
        bool iPoorlyInterleavedContentEventSent;

        PVMFCommandId iCPMInitCmdId;
        PVMFCommandId iCPMOpenSessionCmdId;
        PVMFCommandId iCPMRegisterContentCmdId;
        PVMFCommandId iCPMRequestUsageId;
        PVMFCommandId iCPMUsageCompleteCmdId;
        PVMFCommandId iCPMCloseSessionCmdId;
        PVMFCommandId iCPMResetCmdId;
        PVMFCommandId iCPMGetMetaDataKeysCmdId;
        PVMFCommandId iCPMGetMetaDataValuesCmdId;
        PVMFCommandId iCPMGetLicenseInterfaceCmdId;
        PVMFCommandId iCPMGetLicenseCmdId;
        PVMFCommandId iCPMCancelGetLicenseCmdId;
        void InitCPM();
        void OpenCPMSession();
        void CPMRegisterContent();
        bool GetCPMContentAccessFactory();
        void GetCPMContentType();
        bool GetCPMMetaDataExtensionInterface();
        void GetCPMLicenseInterface();
        void RequestUsage(PVMP4FFNodeTrackOMA2DRMInfo* aInfo);
        void SendUsageComplete();
        void CloseCPMSession();
        void ResetCPM();
        void GetCPMMetaDataKeys();
        PVMFStatus CheckCPMCommandCompleteStatus(PVMFCommandId, PVMFStatus);
        PVMFStatus iCPMRequestUsageCommandStatus;

        PVMFStatus DoGetLicense(PVMFMP4FFParserNodeCommand& aCmd,
                                bool aWideCharVersion = false);
        PVMFStatus DoCancelGetLicense(PVMFMP4FFParserNodeCommand& aCmd);
        void CompleteGetLicense();

        void PopulateOMA1DRMInfo();
        /*
         * OMA2 DRM Related Methods
         */
        Oscl_Vector<PVMP4FFNodeTrackOMA2DRMInfo, OsclMemAllocator> iOMA2DRMInfoVec;
        PVMP4FFNodeTrackOMA2DRMInfo* LookUpOMA2TrackInfoForTrack(uint32 aTrackID);
        PVMFStatus InitOMA2DRMInfo();
        void PopulateOMA2DRMInfo(PVMP4FFNodeTrackOMA2DRMInfo* aInfo);
        PVMFStatus CheckForOMA2AuthorizationComplete(PVMP4FFNodeTrackOMA2DRMInfo*& aInfo);
        void OMA2TrackAuthorizationComplete();
        bool CheckForOMA2UsageApproval();
        void ResetOMA2Flags();
        uint8* iOMA2DecryptionBuffer;

        PVMFStatus GetFileOffsetForAutoResume(uint32& aOffset, bool aPortsAvailable = true);
        PVMFStatus GetFileOffsetForAutoResume(uint32& aOffset, PVMP4FFNodeTrackPortInfo* aInfo);

        PVMFStatus CheckForUnderFlow(PVMP4FFNodeTrackPortInfo* aInfo);

        void getLanguageCode(uint16 langcode, int8 *LangCode);
        void getBrand(uint32 langcode, char *LangCode);

        PVMFStatus CheckForMP4HeaderAvailability();
        int32 CreateErrorInfoMsg(PVMFBasicErrorInfoMessage** aErrorMsg, PVUuid aEventUUID, int32 aEventCode);
        void CreateDurationInfoMsg(uint32 adurationms);
        PVMFStatus PushKVPToMetadataValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>* aVecPtr, PvmiKvp& aKvpVal);
        PVMFStatus CreateNewArray(uint32** aTrackidList, uint32 aNumTracks);
        PVMFStatus PushValueToList(Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &aRefMetadataKeys,
                                   PVMFMetadataList *&aKeyListPtr,
                                   uint32 aLcv);
        PVMIDataStreamSyncInterface* iDataStreamInterface;
        PVMFDataStreamFactory* iDataStreamFactory;
        PVMFDataStreamReadCapacityObserver* iDataStreamReadCapacityObserver;
        PvmiDataStreamSession iDataStreamSessionID;
        PvmiDataStreamCommandId iRequestReadCapacityNotificationID;
        uint32 iMP4HeaderSize;
        bool iDownloadComplete;
        bool iProgressivelyDownlodable;
        uint32 iLastNPTCalcInConvertSizeToTime;
        uint32 iFileSizeLastConvertedToTime;
        bool iFastTrackSession;

        /* External PseudoStreaming related */
        bool iExternalDownload;

        bool iUnderFlowEventReported;
        PVMFStatus ReportUnderFlow();
        OsclTimer<OsclMemAllocator> *iUnderFlowCheckTimer;

        /* bitstream logging */
        void LogMediaData(PVMFSharedMediaDataPtr data,
                          PVMFPortInterface* aPort);
        bool iPortDataLog;
        char iLogFileIndex;
        OSCL_HeapString<OsclMemAllocator> portLogPath;

        uint32 minTime;
        uint32 avgTime;
        uint32 maxTime;
        uint32 sumTime;
        bool iDiagnosticsLogged;
        void LogDiagnostics();
        uint32 iTimeTakenInReadMP4File;
        bool iBackwardReposFlag; /* To avoid backwardlooping :: A flag to remember backward repositioning */
        bool iForwardReposFlag;
        uint32 iCurPos;
        bool iEOTForTextSentToMIO;

        bool iSetTextSampleDurationZero;

        /* To take into account if we get negative TS for text track after repositionings*/
        bool iTextInvalidTSAfterReposition;

        uint32 iDelayAddToNextTextSample;

        uint32 iCacheSize;
        uint32 iAsyncReadBuffSize;
        bool iPVLoggerEnableFlag;
        bool iPVLoggerStateEnableFlag;
        uint32 iNativeAccessMode;

        BaseKeys_SelectionType iBaseKey;
        uint32 iJitterBufferDurationInMs;
        bool iDataStreamRequestPending;
        bool iCPMSequenceInProgress;
        bool oIsAACFramesFragmented;

        int32 iPlayBackDirection;
        int32 iStartForNextTSSearch;
        int32 iPrevSampleTS;
        bool iParseAudioDuringFF;
        bool iParseAudioDuringREW;
        bool iParseVideoOnly;
        int32 iDataRate;

        int32 minFileOffsetTrackID;
};


class PVMFMP4FFPortIter : public PVMFPortIter
{
    public:
        PVMFMP4FFPortIter(Oscl_Vector<PVMP4FFNodeTrackPortInfo, OsclMemAllocator>& aTrackList)
        {
            iIndex = 0;
            iTrackList = &aTrackList;
        };

        virtual ~PVMFMP4FFPortIter() {};

        uint16 NumPorts()
        {
            if (iTrackList->size() < 0xFFFF)
            {
                return (uint16)(iTrackList->size());
            }
            else
            {
                return 0xFFFF;
            }
        };

        PVMFPortInterface* GetNext()
        {
            if (iIndex < iTrackList->size())
            {
                PVMFPortInterface* portiface = (*iTrackList)[iIndex].iPortInterface;
                ++iIndex;
                return portiface;
            }
            else
            {
                return NULL;
            }
        };

        void Reset()
        {
            iIndex = 0;
        };

    private:
        Oscl_Vector<PVMP4FFNodeTrackPortInfo, OsclMemAllocator>* iTrackList;
        uint32 iIndex;
};

#endif // PVMF_MP4FFPARSER_NODE_H_INCLUDED

