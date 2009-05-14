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
 * @file pvmp4ffcn_node.h
 * @brief PVMF node for PVMp4FFComposer
 */

#ifndef PVMP4FFCN_NODE_H_INCLUDED
#define PVMP4FFCN_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef __A_IMpeg4File_H__
#include "a_impeg4file.h"
#endif
#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMP4FFCN_TYPES_H_INCLUDED
#include "pvmp4ffcn_types.h"
#endif
#ifndef PVMP4FFCN_TUNABLES_H_INCLUDED
#include "pvmp4ffcn_tunables.h"
#endif
#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#include "pvmp4ffcn_clipconfig.h"
#endif
#ifndef PVMP4FFCN_TRACKCONFIG_H_INCLUDED
#include "pvmp4ffcn_trackconfig.h"
#endif
#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif
// Forward declaration
class PVMp4FFComposerPort;


//memory allocator type for this node.
typedef OsclMemAllocator PVMp4FFCNAlloc;

/** Node command type */
typedef PVMFGenericNodeCommand<PVMp4FFCNAlloc> PVMp4FFCNCmd;

/** Command queue type */
typedef PVMFNodeCommandQueue<PVMp4FFCNCmd, PVMp4FFCNAlloc> PVMp4FFCNCmdQueue;

/** Port vector type */
typedef PVMFPortVector<PVMp4FFComposerPort, PVMp4FFCNAlloc> PVMp4FFCNPortVector;

#define PROFILING_ON (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_PROF)

#if PROFILING_ON
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#endif

////////////////////////////////////////////////////////////////////////////
class PVMp4FFComposerNode : public PVMFNodeInterface,
            public OsclActiveObject,
            public PVMp4FFCNTrackConfigInterface,
            public PVMp4FFCNClipConfigInterface,
            public PvmfComposerSizeAndDurationInterface,
            public PvmiCapabilityAndConfig
{
    public:
        PVMp4FFComposerNode(int32 aPriority);
        ~PVMp4FFComposerNode();

        // Pure virtual functions from PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId aSession, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMp4FFCNAlloc>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSession, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession, int32 aPortTag,
                const PvmfMimeString* aPortConfig = NULL,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId aSession, PVMFPortInterface& aPort,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId aSession, const OsclAny* aContext = NULL);
        OSCL_EXPORT_REF PVMFCommandId Prepare(PVMFSessionId aSession, const OsclAny* aContext);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId aSession, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId aSession, PVMFCommandId aCmdId,
                const OsclAny* aContextData = NULL);

        // Pure virtual from PVInterface
        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        // Pure virtual functions from PVMp4FFCNClipConfigInterface
        OSCL_IMPORT_REF PVMFStatus SetOutputFileName(const OSCL_wString& aFileName);
        OSCL_IMPORT_REF PVMFStatus SetOutputFileDescriptor(const OsclFileHandle* aFileHandle);
        OSCL_IMPORT_REF PVMFStatus SetAuthoringMode(PVMp4FFCN_AuthoringMode aAuthoringMode = PVMP4FFCN_3GPP_DOWNLOAD_MODE);
        OSCL_IMPORT_REF PVMFStatus SetPresentationTimescale(uint32 aTimescale);
        OSCL_IMPORT_REF PVMFStatus SetVersion(const OSCL_wString& aVersion, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetTitle(const OSCL_wString& aTitle, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetAuthor(const OSCL_wString& aAuthor, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetCopyright(const OSCL_wString& aCopyright, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetDescription(const OSCL_wString& aDescription, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetRating(const OSCL_wString& aRating, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetCreationDate(const OSCL_wString& aCreationDate);
        OSCL_IMPORT_REF PVMFStatus SetRealTimeAuthoring(const bool aRealTime);
        OSCL_IMPORT_REF PVMFStatus SetAlbumInfo(const OSCL_wString& aAlbum_Title, const OSCL_String& aLangCode);
        OSCL_IMPORT_REF PVMFStatus SetRecordingYear(uint16 aRecordingYear);
        OSCL_IMPORT_REF PVMFStatus SetPerformer(const OSCL_wString& aPerformer, const OSCL_String& aLangCode);
        OSCL_EXPORT_REF PVMFStatus SetGenre(const OSCL_wString& aGenre, const OSCL_String& aLangCode);
        OSCL_EXPORT_REF PVMFStatus SetClassification(const OSCL_wString& aClassificationInfo, uint32 aClassificationEntity, uint16 aClassificationTable, const OSCL_String& aLangCode);
        OSCL_EXPORT_REF PVMFStatus SetKeyWord(const OSCL_wString& aKeyWordInfo, const OSCL_String& aLangCode);
        OSCL_EXPORT_REF PVMFStatus SetLocationInfo(PvmfAssetInfo3GPPLocationStruct& aLocation_info);
        OSCL_IMPORT_REF uint16 ConvertLangCode(const OSCL_String& aLang);

        // Pure virtual functions from PVMp4FFCNTrackConfigInterface
        OSCL_IMPORT_REF PVMFStatus SetTrackReference(const PVMFPortInterface& aPort,
                const PVMFPortInterface& aReferencePort);
        OSCL_IMPORT_REF PVMFStatus SetCodecSpecificInfo(const PVMFPortInterface& aPort, uint8* aInfo, int32 aSize);

        // Pure virtual from PvmfComposerSizeAndDurationInterface
        OSCL_IMPORT_REF PVMFStatus SetMaxFileSize(bool aEnable, uint32 aMaxFileSizeBytes);
        OSCL_IMPORT_REF void GetMaxFileSizeConfig(bool& aEnable, uint32& aMaxFileSizeBytes);
        OSCL_IMPORT_REF PVMFStatus SetMaxDuration(bool aEnable, uint32 aMaxDurationMilliseconds);
        OSCL_IMPORT_REF void GetMaxDurationConfig(bool& aEnable, uint32& aMaxDurationMilliseconds);
        OSCL_IMPORT_REF PVMFStatus SetFileSizeProgressReport(bool aEnable, uint32 aReportFrequency);
        OSCL_IMPORT_REF void GetFileSizeProgressReportConfig(bool& aEnable, uint32& aReportFrequency);
        OSCL_IMPORT_REF PVMFStatus SetDurationProgressReport(bool aEnable, uint32 aReportFrequency);
        OSCL_IMPORT_REF void GetDurationProgressReportConfig(bool& aEnable, uint32& aReportFrequency);

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // Friend class
        friend class PVMp4FFComposerPort;
        Oscl_Vector<OsclMemoryFragment*, OsclMemAllocator> memvector_sps;
        Oscl_Vector<OsclMemoryFragment*, OsclMemAllocator> memvector_pps;
        uint8 iNum_SPS_Set;
        uint8 iNum_PPS_Set;

        Oscl_Vector<PVA_FF_TextSampleDescInfo*, OsclMemAllocator> textdecodervector;
        int32 iText_sdIndex;
        // implemetation of PvmiCapabilityAndConfig class functions here

        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);

        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,
                                     PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements);
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL);
        uint32 getCapabilityMetric(PvmiMIOSession aSession);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);

        // function used in getParametersSync of capability class
        PVMFStatus GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);

        // function used in VerifyParametersSync n SetParametersSync of capability class
        PVMFStatus VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam);

    private:

        // Pure virtual from OsclActiveObject
        void Run();

        /////////////////////////////////////////////////////
        //     Command processing routines
        /////////////////////////////////////////////////////
        PVMFCommandId QueueCommandL(PVMp4FFCNCmd& aCmd);
        bool ProcessCommand(PVMp4FFCNCmd& aCmd);
        void CommandComplete(PVMp4FFCNCmdQueue& aQueue, PVMp4FFCNCmd& aCmd, PVMFStatus aStatus, OsclAny* aData = NULL);
        void DoQueryUuid(PVMp4FFCNCmd& aCmd);
        void DoQueryInterface(PVMp4FFCNCmd& aCmd);
        void DoRequestPort(PVMp4FFCNCmd& aCmd);
        void DoReleasePort(PVMp4FFCNCmd& aCmd);
        void DoInit(PVMp4FFCNCmd& aCmd);
        void DoPrepare(PVMp4FFCNCmd& aCmd);
        void DoStart(PVMp4FFCNCmd& aCmd);
        PVMFStatus AddTrack(PVMp4FFComposerPort *aPort);
        void DoStop(PVMp4FFCNCmd& aCmd);
        void DoFlush(PVMp4FFCNCmd& aCmd);
        bool IsFlushPending();
        void FlushComplete();
        void DoPause(PVMp4FFCNCmd& aCmd);
        void DoReset(PVMp4FFCNCmd& aCmd);
        void DoCancelAllCommands(PVMp4FFCNCmd& aCmd);
        void DoCancelCommand(PVMp4FFCNCmd& aCmd);

        /////////////////////////////////////////////////////
        //      Port activity processing routines
        /////////////////////////////////////////////////////
        bool IsProcessIncomingMsgReady();
        /**
         * Process an incoming message of a the specified port
         * @param aPort Port where outgoing message is queued.
         * @return Completion status
         */
        PVMFStatus ProcessIncomingMsg(PVMFPortInterface* aPort);

        PVMFStatus AddMemFragToTrack(Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> aFrame, OsclRefCounterMemFrag& aMemFrag, PVMFFormatType aFormat,
                                     uint32& aTimestamp, int32 aTrackId, PVMp4FFComposerPort *aPort);
        int32 GetIETFFrameSize(uint8 aFrameType, int32 aCodecType);

        /////////////////////////////////////////////////////
        //    Progress and max size / duration routines
        /////////////////////////////////////////////////////
        /**
         * Send file size or duration report if enabled.
         * @param aTimestamp Timestamp of current frame in milliseconds.
         * @return PVMFFailure if informational observer is not set, else PVMFSuccess
         */
        PVMFStatus SendProgressReport(uint32 aTimestamp);

        /**
         * Check if maximum file size or duration is reached if a maximum is set.
         *
         * @param aFrameSize Size of current frame in bytes.
         * @return PVMFSuccess if feature is enabled and the maximum file size / duration is reached.
         *         PVMFPending if feature is enabled and the max file size / duration has not been reached.
         *         PVMFErrNotSupported if feature is not enabled.
         *         PVMFFailure if informational observer is not set or if max file size or duration is set
         *         but the finalizing output file failed.
         */
        PVMFStatus CheckMaxFileSize(uint32 aFrameSize);

        /**
         * Check if maximum file size or duration is reached if a maximum is set.
         *
         * @param aTimestamp Timestamp of current frame in milliseconds.
         * @return PVMFSuccess if feature is enabled and the maximum file size / duration is reached.
         *         PVMFPending if feature is enabled and the max file size / duration has not been reached.
         *         PVMFErrNotSupported if feature is not enabled.
         *         PVMFFailure if informational observer is not set or if max file size or duration is set
         *         but the finalizing output file failed.
         */
        PVMFStatus CheckMaxDuration(uint32 aTimestamp);

        /**
         * Finalize the output file.
         * @return PVMFSuccess if output file is successfully finalized, else PVMFFailure
         */
        PVMFStatus RenderToFile();

        /**
         * Write DecoderSpecific Info at the end of encoding.
         * @return void
         */
        void WriteDecoderSpecificInfo();

        /** Clear all pending port activity after max file size or duration is reached. */
        void ClearPendingPortActivity();

        /////////////////////////////////////////////////////
        //      Event reporting routines
        /////////////////////////////////////////////////////
        void ReportErrorEvent(PvmfMp4FFCNError aEventType, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void SetState(TPVMFNodeInterfaceState);

    private:
        void GenerateDiagnostics(uint32 aTime, uint32 aSize);
        void LogDiagnostics();
        int32 StoreCurrentCommand(PVMp4FFCNCmdQueue&, PVMp4FFCNCmd&, PVMp4FFCNCmdQueue&);

        // Node command queue
        PVMp4FFCNCmdQueue iCmdQueue;
        PVMp4FFCNCmdQueue iCurrentCmd;

        // Vector of ports contained in this node
        PVMp4FFCNPortVector iInPorts;

        // File format
        PVA_FF_IMpeg4File* iMpeg4File;
        OSCL_wHeapString<OsclMemAllocator> iFileName;
        OSCL_wHeapString<OsclMemAllocator> iPostfix;
        OSCL_wHeapString<OsclMemAllocator> iOutputPath;
        Oscl_FileServer iFs;
        int32 iFileType;
        uint32 iAuthoringMode;
        uint32 iPresentationTimescale;
        uint32 iMovieFragmentDuration;
        Oscl_File* iFileObject;
        // Meta data strings
        struct PVMP4FFCN_MetaDataString
        {
public:
            PVMP4FFCN_MetaDataString(): iClassificationEntity(0), iClassificationTable(0), iLangCode(0) {};
            OSCL_wHeapString<OsclMemAllocator> iDataString;
            uint32 iClassificationEntity;
            uint16 iClassificationTable;
            uint16 iLangCode;
        };
        class PVMP4FFCN_KeyWord
        {
            public:
                PVMP4FFCN_KeyWord(): iKeyWordSize(0), iLang_Code(0) {};
                ~PVMP4FFCN_KeyWord() {};
                uint32 iKeyWordSize;
                uint16 iLang_Code;
                OSCL_wHeapString<OsclMemAllocator> iData_String;

                PVMP4FFCN_KeyWord(const OSCL_wString& aData_String, uint32 aKeyWordSize, uint16 aLang_Code)
                {
                    iData_String = aData_String;
                    iKeyWordSize = aKeyWordSize;
                    iLang_Code = aLang_Code;
                }

        };
        PVMP4FFCN_MetaDataString iVersion;
        PVMP4FFCN_MetaDataString iTitle;
        PVMP4FFCN_MetaDataString iAuthor;
        PVMP4FFCN_MetaDataString iCopyright;
        PVMP4FFCN_MetaDataString iDescription;
        PVMP4FFCN_MetaDataString iRating;
        PVMP4FFCN_MetaDataString iAlbumTitle;
        uint16 iRecordingYear;
        PVMP4FFCN_MetaDataString iPerformer;
        PVMP4FFCN_MetaDataString iGenre;
        PVMP4FFCN_MetaDataString iClassification;
        Oscl_Vector<PVMP4FFCN_KeyWord* , OsclMemAllocator> iKeyWordVector;

        PvmfAssetInfo3GPPLocationStruct iLocationInfo;
        OSCL_wHeapString<OsclMemAllocator> iCreationDate;

        // Convert from timescale
        MediaClockConverter iClockConverter;

        // Debug logging
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;

        uint32 iExtensionRefCount;

        bool iRealTimeTS;
        bool iInitTSOffset;
        uint32 iTSOffset;

        //variables for fileoutputduration config
        bool iMaxFileSizeEnabled;
        bool iMaxDurationEnabled;
        uint32 iMaxFileSize;
        uint32 iMaxTimeDuration;

        bool iFileSizeReportEnabled;
        bool iDurationReportEnabled;
        uint32 iFileSizeReportFreq;
        uint32 iDurationReportFreq;
        uint32 iNextDurationReport;
        uint32 iNextFileSizeReport;

        uint32 iCacheSize;
        int32 iConfigSize;
        uint8 *pConfig;
        int32 iTrackId_H264;
        int32 iTrackId_Text;
        int32 iSyncSample;
        PVMFFormatType iformat_h264;
        PVMFFormatType iformat_text;
        bool iNodeEndOfDataReached;
        bool iSampleInTrack;
        bool iFileRendered;

#if PROFILING_ON
        uint32 iMaxSampleAddTime;
        uint32 iMinSampleAddTime;
        uint32 iMinSampleSize;
        uint32 iMaxSampleSize;
        uint32 iNumSamplesAdded;
        PVLogger* iDiagnosticsLogger;
        bool oDiagnosticsLogged;

        // Statistics
        struct PVMp4FFCNStats
        {
            int32 iTrackId;
            uint32 iNumFrames;
            uint32 iDuration;
        };

        PVMp4FFCNStats iStats[3];
#endif
#ifdef _TEST_AE_ERROR_HANDLING
        bool iErrorHandlingAddMemFrag;
        bool iErrorHandlingAddTrack;
        bool iErrorCreateComposer;
        bool iErrorRenderToFile;
        PVMFFormatType iErrorAddTrack;
        uint32 iErrorNodeCmd;
        uint32 iTestFileSize;
        uint32 iTestTimeStamp;
        uint32 iErrorAddSample;
        uint32 iFileSize;
        uint32 iFileDuration;
        uint32 iErrorDataPathStall;
#endif
};

#endif // PVMP4FFC_NODE_H_INCLUDED

