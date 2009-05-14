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
#ifndef PVMF_MP4FFPARSER_OUTPORT_H_INCLUDED
#define PVMF_MP4FFPARSER_OUTPORT_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_TIMEDTEXT_H_INCLUDED
#include "pvmf_timedtext.h"
#endif

#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif

#include "pvmf_mp4ffparser_node.h"

#define PVMF_MP4FFPARSERNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_MP4FFPARSERNODE_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_MP4FFPARSERNODE_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGINFO(m) PVMF_MP4FFPARSERNODE_LOGINFOMED(m)
#define PVMF_MP4FFPARSERNODE_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGDATATRAFFIC_AVC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iAVCDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGCLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iClockLogger,PVLOGMSG_INFO,m);
#define PVMF_MP4FFPARSERNODE_LOGBIN(iPortLogger, m) PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iPortLogger, PVLOGMSG_ERR, m);
#define PVMF_MP4FFPARSERNODE_LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iDiagnosticsLogger,PVLOGMSG_INFO,m);
/**
* Port/Track information
*/
class MediaClockConverter;

class VideoTrackDimensionInfo
{
    public:
        VideoTrackDimensionInfo() {};
        virtual ~VideoTrackDimensionInfo() {};
        VideoTrackDimensionInfo(const VideoTrackDimensionInfo& aSrc)
        {
            iTrackId = aSrc.iTrackId;
            iWidth = aSrc.iWidth;
            iHeight = aSrc.iHeight;
            iDisplayWidth = aSrc.iDisplayWidth;
            iDisplayHeight = aSrc.iDisplayHeight;

        };

        uint32 iTrackId;
        int32 iWidth;
        int32 iHeight;
        int32 iDisplayWidth;
        int32 iDisplayHeight;
};

class PVMP4FFNodeTrackPortInfo : public OsclMemPoolFixedChunkAllocatorObserver,
            public OsclMemPoolResizableAllocatorObserver
{
    public:
        enum TrackState
        {
            TRACKSTATE_UNINITIALIZED,
            TRACKSTATE_INITIALIZED,
            TRACKSTATE_TRANSMITTING_GETDATA,
            TRACKSTATE_TRANSMITTING_SENDDATA,
            TRACKSTATE_TRACKDATAPOOLEMPTY,
            TRACKSTATE_MEDIADATAPOOLEMPTY,
            TRACKSTATE_MEDIADATAFRAGGROUPPOOLEMPTY,
            TRACKSTATE_DESTFULL,
            TRACKSTATE_INSUFFICIENTDATA,
            TRACKSTATE_ENDOFTRACK,
            TRACKSTATE_ERROR,
            TRACKSTATE_DOWNLOAD_AUTOPAUSE,
            TRACKSTATE_SEND_ENDOFTRACK,
            TRACKSTATE_TRACKMAXDATASIZE_RESIZE,
            TRACKSTATE_SKIP_CORRUPT_SAMPLE
        };

        PVMP4FFNodeTrackPortInfo()
        {
            iTrackId = -1;
            iPortInterface = NULL;
            iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            iFormatTypeInteger = 0;
            iClockConverter = NULL;
            iState = TRACKSTATE_UNINITIALIZED;
            iTrackMaxDataSize = 0;
            iTrackMaxQueueDepth = 0;
            iTrackDataMemoryPool = NULL;
            iMediaDataImplAlloc = NULL;
            iTextMediaDataImplAlloc = NULL;
            iMediaDataMemPool = NULL;
            iMediaDataGroupImplMemPool = NULL;
            iMediaDataGroupAlloc = NULL;
            iNode = NULL;
            iTimestamp = 0;
            iFirstFrameAfterRepositioning = false;
            iSeqNum = 0;
            iSendBOS = false;

            iNumAMRSamplesToRetrieve = 0;
            iCurrentTextSampleEntryIndex = 0;

            /* bitstream logging */
            iPortLogger = NULL;
            oFormatSpecificInfoLogged = false;
            iMinTime = 0;
            iMaxTime = 0;
            iSumTime = 0;
            iNumTimesMediaSampleRead = 0;
            iNumSamples = 0;

            //thumb nail mode
            iThumbSampleDone = false;

            iTargetNPTInMediaTimeScale = 0;
            iLogger = PVLogger::GetLoggerObject("datapath.sourcenode.mp4parsernode");
            //PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"PVMP4FFNodeTrackPortInfo::PVMP4FFNodeTrackPortInfo"));

        }

        PVMP4FFNodeTrackPortInfo(const PVMP4FFNodeTrackPortInfo& aSrc) :
                OsclMemPoolFixedChunkAllocatorObserver(aSrc),
                OsclMemPoolResizableAllocatorObserver(aSrc)
        {
            iTrackId = aSrc.iTrackId;
            iPortInterface = aSrc.iPortInterface;
            iMimeType = aSrc.iMimeType;
            iFormatType = aSrc.iFormatType;
            iFormatTypeInteger = aSrc.iFormatTypeInteger;
            iClockConverter = aSrc.iClockConverter;
            iFormatSpecificConfig = aSrc.iFormatSpecificConfig;
            iFormatSpecificConfigAndFirstSample = aSrc.iFormatSpecificConfigAndFirstSample;
            iMediaData = aSrc.iMediaData;
            iState = aSrc.iState;
            iTrackMaxDataSize = aSrc.iTrackMaxDataSize;
            iTrackMaxQueueDepth = aSrc.iTrackMaxQueueDepth;
            iTrackDataMemoryPool = aSrc.iTrackDataMemoryPool;
            iMediaDataImplAlloc = aSrc.iMediaDataImplAlloc;
            iTextMediaDataImplAlloc = aSrc.iTextMediaDataImplAlloc;
            iMediaDataMemPool = aSrc.iMediaDataMemPool;
            iMediaDataGroupImplMemPool = aSrc.iMediaDataGroupImplMemPool;
            iMediaDataGroupAlloc = aSrc.iMediaDataGroupAlloc;
            iNode = aSrc.iNode;
            iTimestamp = aSrc.iTimestamp;
            iFirstFrameAfterRepositioning = aSrc.iFirstFrameAfterRepositioning;
            iSeqNum = aSrc.iSeqNum;
            iSendBOS = aSrc.iSendBOS;

            iNumAMRSamplesToRetrieve = aSrc.iNumAMRSamplesToRetrieve;
            iCurrentTextSampleEntryIndex = aSrc.iCurrentTextSampleEntryIndex;
            iCurrentTextSampleEntry = aSrc.iCurrentTextSampleEntry;

            /* bitstream logging */
            iPortLogger = aSrc.iPortLogger;
            iLogFile = aSrc.iLogFile;
            oFormatSpecificInfoLogged = aSrc.oFormatSpecificInfoLogged;
            iBinAppenderPtr = aSrc.iBinAppenderPtr;
            iNumSamples = aSrc.iNumSamples;
            iMinTime = aSrc.iMinTime;
            iMaxTime = aSrc.iMaxTime;
            iSumTime = aSrc.iSumTime;
            iNumTimesMediaSampleRead = aSrc.iNumTimesMediaSampleRead;
            iThumbSampleDone = aSrc.iThumbSampleDone;

            iTargetNPTInMediaTimeScale = aSrc.iTargetNPTInMediaTimeScale;

            iLogger = aSrc.iLogger;
        }

        virtual ~PVMP4FFNodeTrackPortInfo()
        {
            iCurrentTextSampleEntry.Unbind();
        }

        // From OsclMemPoolFixedChunkAllocatorObserver
        void freechunkavailable(OsclAny*)
        {

            if (iState == TRACKSTATE_MEDIADATAPOOLEMPTY || iState == TRACKSTATE_MEDIADATAFRAGGROUPPOOLEMPTY)
            {
                //PVLogger* iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.mp4parsernode");
                //PVMF_MP4FFPARSERNODE_LOGDATATRAFFIC((0, "PVMP4PN - freechunkavailable - %s, TrackID=%d", this->iMimeType.get_cstr(), this->iTrackId));
                // Change state
                iState = TRACKSTATE_TRANSMITTING_GETDATA;
                if (iNode)
                {
                    if (iNode->IsAdded())
                    {
                        // Activate the parent node if necessary
                        iNode->RunIfNotReady();
                    }
                }
            }
        }

        // From OsclMemPoolResizableAllocatorObserver
        // Callback handler when mempool's deallocate() is called after
        // calling notifyfreeblockavailable() on the mempool
        void freeblockavailable(OsclAny*)
        {
            // Check if track is waiting for track data to be available
            if (iState == TRACKSTATE_TRACKDATAPOOLEMPTY)
            {
                //PVLogger* iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.mp4parsernode");
                //PVMF_MP4FFPARSERNODE_LOGDATATRAFFIC((0, "PVMP4PN - freechunkavailable - %s, TrackID=%d", this->iMimeType.get_cstr(), this->iTrackId));
                // Change state
                iState = TRACKSTATE_TRANSMITTING_GETDATA;
                if (iNode)
                {
                    if (iNode->IsAdded())
                    {
                        // Activate the parent node if necessary
                        iNode->RunIfNotReady();
                    }
                }
            }
        }

        // Track ID number in MP4 FF
        int32 iTrackId;
        // Output port to send the data downstream
        PVMFPortInterface* iPortInterface;
        // Mime type for the port
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        // Format type for the port
        PVMFFormatType iFormatType;
        // Integer Format type for the port
        uint32 iFormatTypeInteger;
        // Converter to convert from track timescale to milliseconds
        MediaClockConverter* iClockConverter;
        // Shared memory pointer holding the decoder specific config info for this track
        OsclRefCounterMemFrag iFormatSpecificConfig;
        // Shared memory pointer holding the decoder specific config info plus
        // the first sample for this track
        OsclRefCounterMemFrag iFormatSpecificConfigAndFirstSample;
        // Shared memory pointer holding the currently retrieved track data
        PVMFSharedMediaDataPtr iMediaData;
        // Current state of this track
        TrackState iState;
        // The maximum expected track data size (depends on format)
        uint32 iTrackMaxDataSize;
        // The maximum queue depth for the track data (depend on format)
        uint32 iTrackMaxQueueDepth;
        // Output buffer memory pool
        OsclMemPoolResizableAllocator *iTrackDataMemoryPool;
        // Allocator for simple media data buffer impl
        PVMFResizableSimpleMediaMsgAlloc *iMediaDataImplAlloc;
        // Allocator for text track simple media data buffer impl
        PVMFTimedTextMediaDataAlloc* iTextMediaDataImplAlloc;
        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator *iMediaDataMemPool;
        // Memory pool for media data buffer impl that holds multiple media fragments
        OsclMemPoolFixedChunkAllocator* iMediaDataGroupImplMemPool;
        // Allocator for media frag group
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iMediaDataGroupAlloc;

        // MP4 FF parser node handle as AO class
        OsclTimerObject* iNode;
        // Timestamp
        uint32 iTimestamp;
        // Flag to indicate that next frame will be the first frame after repositioning
        bool iFirstFrameAfterRepositioning;
        // Sequence Number
        uint32 iSeqNum;
        // bos flag
        bool iSendBOS;

        // Format specific variables
        // AMR track
        uint32 iNumAMRSamplesToRetrieve;
        // Timed text track
        uint32 iCurrentTextSampleEntryIndex;
        OsclSharedPtr<PVMFTimedTextSampleEntry> iCurrentTextSampleEntry;

        /* bitstream logging */
        PVLogger* iPortLogger;
        PVLogger *iLogger;

        OSCL_HeapString<OsclMemAllocator> iLogFile;
        bool oFormatSpecificInfoLogged;
        OsclSharedPtr<PVLoggerAppender> iBinAppenderPtr;
        uint32 iMinTime;
        uint32 iMaxTime;
        uint32 iSumTime;
        uint32 iNumSamples;
        uint32 iNumTimesMediaSampleRead;

        //thumb nail mode
        bool iThumbSampleDone;

        // no-render related
        uint32 iTargetNPTInMediaTimeScale;
};

class PVMP4FFNodeTrackOMA2DRMInfo
{
    public:
        PVMP4FFNodeTrackOMA2DRMInfo()
        {
            iTrackId = 0;
            iDRMInfoSize = 0;
            iDRMInfo = NULL;
            iOMA2TrackAuthorizationInProgress = false;
            iOMA2TrackAuthorizationComplete = false;
            iOMA2TrackAuthorized = false;
        };

        PVMP4FFNodeTrackOMA2DRMInfo(const PVMP4FFNodeTrackOMA2DRMInfo& aSrc)
        {
            Copy(aSrc);
        };


        virtual ~PVMP4FFNodeTrackOMA2DRMInfo()
        {
            if (iDRMInfo != NULL)
            {
                OSCL_ARRAY_DELETE(iDRMInfo);
                iDRMInfo = NULL;
            }
        };

        void Copy(const PVMP4FFNodeTrackOMA2DRMInfo& aSrc)
        {
            iTrackId = aSrc.iTrackId;
            iDRMInfoSize = aSrc.iDRMInfoSize;
            if (iDRMInfoSize > 0)
            {
                iDRMInfo = OSCL_ARRAY_NEW(uint8, iDRMInfoSize);
                oscl_memcpy((OsclAny*)iDRMInfo, (OsclAny*)(aSrc.iDRMInfo), iDRMInfoSize);
            }
            iOMA2TrackAuthorizationInProgress = aSrc.iOMA2TrackAuthorizationInProgress;
            iOMA2TrackAuthorizationComplete = aSrc.iOMA2TrackAuthorizationComplete;
            iOMA2TrackAuthorized = aSrc.iOMA2TrackAuthorized;
        };

        uint32 iTrackId;
        uint32 iDRMInfoSize;
        uint8* iDRMInfo;
        bool iOMA2TrackAuthorizationInProgress;
        bool iOMA2TrackAuthorizationComplete;
        bool iOMA2TrackAuthorized;
};

class PVMFMP4FFParserNode;

class PVMFMP4FFParserOutPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        PVMFMP4FFParserOutPort(int32 aTag, PVMFNodeInterface* aNode, const char*);
        PVMFMP4FFParserOutPort(int32 aTag
                               , PVMFNodeInterface* aNode
                               , uint32 aInCapacity
                               , uint32 aInReserve
                               , uint32 aInThreshold
                               , uint32 aOutCapacity
                               , uint32 aOutReserve
                               , uint32 aOutThreshold
                               , const char*name);
        ~PVMFMP4FFParserOutPort();

        // From PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // To support config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
            {
                aPtr = (PvmiCapabilityAndConfig*)this;
            }
            else
            {
                aPtr = NULL;
            }
        }
        /* Over ride Connect() */
        PVMFStatus Connect(PVMFPortInterface* aPort);

        /* Implement pure virtuals from PvmiCapabilityAndConfig interface */
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        /* Unsupported PvmiCapabilityAndConfig methods */
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_parameter_elements);
        };
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL)
        {
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        }


    private:
        void Construct();
        bool pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
                                               const char* aFormatValType);

        bool pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
                                               PvmiKvp*& aKvp);
        PVLogger *iLogger;
        uint32 iNumFramesGenerated; //number of source frames generated.
        uint32 iNumFramesConsumed; //number of frames consumed & discarded.

        friend class PVMFMP4FFParserNode;
        PVMFMP4FFParserNode* iMP4FFParserNode;
};

#endif // PVMF_MP4FFPARSER_OUTPORT_H_INCLUDED
