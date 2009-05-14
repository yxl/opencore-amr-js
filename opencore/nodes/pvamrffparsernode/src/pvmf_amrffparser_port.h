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
#ifndef PVMF_AMRFFPARSER_OUTPORT_H_INCLUDED
#define PVMF_AMRFFPARSER_OUTPORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
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

#ifndef OSCL_BIN_STREAM_H_INCLUDED
#include "oscl_bin_stream.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
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

#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif

typedef OsclMemAllocator PVMFAMRParserNodeAllocator;

#define PVMF_AMRPARSERNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_AMRPARSERNODE_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_AMRPARSERNODE_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_AMRPARSERNODE_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_AMRPARSERNODE_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_AMRPARSERNODE_LOGINFO(m) PVMF_AMRPARSERNODE_LOGINFOMED(m)
#define PVMF_AMRPARSERNODE_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_AMRPARSERNODE_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_AMRPARSERNODE_LOGCLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iClockLogger,PVLOGMSG_INFO,m);
#define PVMF_AMRPARSERNODE_LOGBIN(iPortLogger, m) PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iPortLogger, PVLOGMSG_ERR, m);


class PVMFAMRParserNodeLoggerDestructDealloc : public OsclDestructDealloc
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

/**
* Track/Port information
*/
class MediaClockConverter;
class PVMFAMRFFParserNode;

class PVAMRFFNodeTrackPortInfo : public OsclMemPoolFixedChunkAllocatorObserver,
            public OsclMemPoolResizableAllocatorObserver
{
    public:

        PVAMRFFNodeTrackPortInfo()
        {
            iTrackId = -1;
            iPort = NULL;
            iClockConverter = NULL;
            iTrackDataMemoryPool = NULL;
            iMediaDataImplAlloc = NULL;
            iMediaDataMemPool = NULL;
            iNode = NULL;
            iSeqNum = 0;
            iTimestampOffset = 0;
            iFormatType                   = PVMF_MIME_FORMAT_UNKNOWN;

            iResizableDataMemoryPool      = NULL;
            iResizableSimpleMediaMsgAlloc = NULL;

            iTimestamp                    = 0;
            iFirstFrame                   = false;

            oEOSReached                   = false;
            oEOSSent                      = false;
            oTrackSelected                = false;
            oQueueOutgoingMessages        = true;
            oProcessOutgoingMessages      = true;
            iTrackBitRate                 = 0;
            iTrackDuration                = 0;
            iContinuousTimeStamp          = 0;


            iResizableDataMemoryPoolSize  = 0;
            iTrackMaxSampleSize           = 0;

            iAudioSampleRate              = 0;
            iAudioNumChannels             = 0;

            iAudioBitsPerSample           = 0;
            iCodecName					  = NULL;
            iCodecDescription			  = NULL;

            iSendBOS = false;

            iLogger = PVLogger::GetLoggerObject("PVMFASFParserNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.asfparsernode");
            if (iDataPathLogger)
                iDataPathLogger->DisableAppenderInheritance();
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iPortLogger = NULL;
            oFormatSpecificInfoLogged = false;
        }

        PVAMRFFNodeTrackPortInfo(const PVAMRFFNodeTrackPortInfo& aSrc) :
                OsclMemPoolFixedChunkAllocatorObserver(aSrc),
                OsclMemPoolResizableAllocatorObserver(aSrc)
        {
            iTrackId = aSrc.iTrackId;
            iPort = aSrc.iPort;
            iTag = aSrc.iTag;
            iClockConverter = aSrc.iClockConverter;
            iMediaData = aSrc.iMediaData;
            iTrackDataMemoryPool = aSrc.iTrackDataMemoryPool;
            iMediaDataImplAlloc = aSrc.iMediaDataImplAlloc;
            iMediaDataMemPool = aSrc.iMediaDataMemPool;
            iNode = aSrc.iNode;
            iSeqNum = aSrc.iSeqNum;
            iTimestampOffset = aSrc.iTimestampOffset;
            iFirstFrame = aSrc.iFirstFrame;

            iFormatType                        = aSrc.iFormatType;
            iTrackMimeType                     = aSrc.iTrackMimeType;
            iClockConverter                    = aSrc.iClockConverter;
            iFormatSpecificConfig              = aSrc.iFormatSpecificConfig;
            iResizableDataMemoryPool           = aSrc.iResizableDataMemoryPool;
            iResizableSimpleMediaMsgAlloc      = aSrc.iResizableSimpleMediaMsgAlloc;
            iMediaDataMemPool                  = aSrc.iMediaDataMemPool;
            iTimestamp                         = aSrc.iTimestamp;
            oEOSReached                        = aSrc.oEOSReached;
            oEOSSent                           = aSrc.oEOSSent;
            oTrackSelected                     = aSrc.oTrackSelected;
            oQueueOutgoingMessages             = aSrc.oQueueOutgoingMessages;
            oProcessOutgoingMessages           = aSrc.oProcessOutgoingMessages;
            iTrackBitRate                      = aSrc.iTrackBitRate;
            iTrackDuration                     = aSrc.iTrackDuration;
            iContinuousTimeStamp               = aSrc.iContinuousTimeStamp;


            iResizableDataMemoryPoolSize       = aSrc.iResizableDataMemoryPoolSize;
            iTrackMaxSampleSize                = aSrc.iTrackMaxSampleSize;
            iLogger					           = aSrc.iLogger;
            iDataPathLogger			           = aSrc.iDataPathLogger;
            iClockLogger			           = aSrc.iClockLogger;
            iPortLogger                        = aSrc.iPortLogger;
            oFormatSpecificInfoLogged          = aSrc.oFormatSpecificInfoLogged;
            iAudioSampleRate                   = aSrc.iAudioSampleRate;
            iAudioNumChannels                  = aSrc.iAudioNumChannels;
            iAudioBitsPerSample                = aSrc.iAudioBitsPerSample;
            iCodecName						   = aSrc.iCodecName;;
            iCodecDescription				   = aSrc.iCodecDescription;

            iSendBOS = aSrc.iSendBOS;

        }

        virtual ~PVAMRFFNodeTrackPortInfo()
        {
        }

        // From OsclMemPoolFixedChunkAllocatorObserver
        // Callback handler when mempool's deallocate() is called after
        // calling notifyfreechunkavailable() on the mempool
        void freechunkavailable(OsclAny* aContextData)
        {
            OSCL_UNUSED_ARG(aContextData);
            PVMF_AMRPARSERNODE_LOGINFO((0, "MimeType = %s, freeblockavailable", iTrackMimeType.get_cstr()));
            oQueueOutgoingMessages = true;
            if (iNode)
            {
                /* Activate the parent node if necessary */
                iNode->RunIfNotReady();
            }
        }

        /*
         * From OsclMemPoolResizableAllocatorObserver
         * Callback handler when mempool's deallocate() is called after
         * calling notifyfreeblockavailable() on the mempool
         */
        void freeblockavailable(OsclAny* aContextData)
        {
            OSCL_UNUSED_ARG(aContextData);
            PVMF_AMRPARSERNODE_LOGINFO((0, "MimeType = %s, freeblockavailable", iTrackMimeType.get_cstr()));
            oQueueOutgoingMessages = true;
            if (iNode)
            {
                /* Activate the parent node if necessary */
                iNode->RunIfNotReady();
            }
        }

        // Track ID number in AMR FF
        int32 iTrackId;
        // Output port to send the data downstream
        PVMFPortInterface* iPort;
        int32 iTag;

        /* Track Duration */
        uint64 iTrackDuration;

        /* PVMF mime type for track */
        OSCL_HeapString<PVMFAMRParserNodeAllocator> iTrackMimeType;

        /* Format type for the port */
        PVMFFormatType iFormatType;

        // Converter to convert from track timescale to milliseconds
        MediaClockConverter* iClockConverter;

        /* Shared memory pointer holding the decoder specific config info for this track */
        OsclRefCounterMemFrag iFormatSpecificConfig;

        /////////////////////////////////////////////////////////////////////
        // Shared memory pointer holding the currently retrieved track data
        PVMFSharedMediaDataPtr iMediaData;

        /* Resizable mem allocator */
        OsclMemPoolResizableAllocator *iResizableDataMemoryPool;

        /* Allocator for simple media buffer impl */
        PVMFResizableSimpleMediaMsgAlloc* iResizableSimpleMediaMsgAlloc;

        /* Timestamp */
        uint32 iTimestamp;

        /* Track Bitrate */
        uint32 iTrackBitRate;

        /* port flow control */
        bool oQueueOutgoingMessages;
        bool oProcessOutgoingMessages;

        /* End of Track */
        bool oEOSReached;
        bool oEOSSent;

        /* Track Selection */
        bool oTrackSelected;

        /* Continuous timestamp */
        uint64 iContinuousTimeStamp;

        /* Mem pool size */
        uint32 iResizableDataMemoryPoolSize;

        /* Track Max Sample Size */
        uint32 iTrackMaxSampleSize;

        /* Audio - Video specfic info */
        uint32 iAudioSampleRate;
        uint32 iAudioNumChannels;
        uint32 iAudioBitsPerSample;
        OSCL_wHeapString<OsclMemAllocator> iCodecName;
        OSCL_wHeapString<OsclMemAllocator> iCodecDescription;

        // Output buffer memory pool
        OsclMemPoolFixedChunkAllocator *iTrackDataMemoryPool;
        // Allocator for simple media data buffer impl
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataImplAlloc;
        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator *iMediaDataMemPool;
        // AMR FF parser node handle
        OsclTimerObject* iNode;
        // Sequence number
        uint32 iSeqNum;
        // Timestamp offset
        uint32 iTimestampOffset;
        // Set if first frame after repositioning
        bool iFirstFrame;

        // bos flag
        bool iSendBOS;
        /* Loggers */
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;
        /* bitstream logging */
        PVLogger* iPortLogger;
        OSCL_HeapString<PVMFAMRParserNodeAllocator> iLogFile;
        bool oFormatSpecificInfoLogged;

        OsclSharedPtr<PVLoggerAppender> iBinAppenderPtr;
};

class PVMFAMRFFParserOutPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfig
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFAMRFFParserOutPort(int32 aTag
                               , PVMFNodeInterface* aNode);

        /**
         * Constructor that allows the node to configure the data queues of this port.
         * @param aTag Port tag
         * @param aNode Container node
         * @param aSize Data queue capacity. The data queue size will not grow beyond this capacity.
         * @param aReserve Size of data queue for which memory is reserved. This must be
         * less than or equal to the capacity. If this is less than capacity, memory will be
         * allocated when the queue grows beyond the reserve size, but will stop growing at
         * capacity.
         * @param aThreshold Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         * This value should be between 0 - 100.
         */
        PVMFAMRFFParserOutPort(int32 aTag
                               , PVMFNodeInterface* aNode
                               , uint32 aInCapacity
                               , uint32 aInReserve
                               , uint32 aInThreshold
                               , uint32 aOutCapacity
                               , uint32 aOutReserve
                               , uint32 aOutThreshold);

        /** Destructor */
        ~PVMFAMRFFParserOutPort();

        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
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

        PVMFAMRFFParserNode* iAMRParserNode;
};

#endif // PVMF_AMRFFPARSER_OUTPORT_H_INCLUDED
