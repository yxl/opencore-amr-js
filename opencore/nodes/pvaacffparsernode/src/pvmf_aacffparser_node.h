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
#ifndef PVMF_AACFFPARSER_NODE_H_INCLUDED
#define PVMF_AACFFPARSER_NODE_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
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
#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif
#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMF_AACFFPARSER_DEFS_H_INCLUDED
#include "pvmf_aacffparser_defs.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif
#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif
#ifndef AACFILEPARSER_H_INCLUDED
#include "aacfileparser.h"
#endif
#ifndef PVMF_LOCAL_DATA_SOURCE_H_INCLUDED
#include "pvmf_local_data_source.h"
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
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif
#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
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
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_access_interface.h"
#endif
#ifndef PVMF_AACFFPARSER_OUTPORT_H_INCLUDED
#include "pvmf_aacffparser_outport.h"
#endif
#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

/**
 * Node command handling
 */
#define PVMF_AAC_PARSER_NODE_NEW(auditCB,T,params,ptr)\
{\
	ptr = OSCL_NEW(T,params);\
}

#define PVMF_AAC_PARSER_NODE_DELETE(auditCB,T,ptr)\
{\
	OSCL_DELETE(ptr);\
}

#define PVMF_AAC_PARSER_NODE_TEMPLATED_DELETE(auditCB, T, Tsimple, ptr)\
{\
	OSCL_DELETE(ptre);\
}

#define PV_AAC_PARSER_NODE_ARRAY_NEW(auditCB, T, count, ptr)\
{\
	ptr = OSCL_ARRAY_NEW(T, count);\
}

#define PV_AAC_PARSER_NODE_ARRAY_DELETE(auditCB, ptr)\
{\
	OSCL_ARRAY_DELETE(ptr);\
}



/** Track info
*/

// Allocator wrapper for the memory pool that saves the last block pointer allocated
// so it can be resized later
class TrackDataMemPoolProxyAlloc : public Oscl_DefAlloc
{
    public:
        TrackDataMemPoolProxyAlloc(OsclMemPoolResizableAllocator& aMemPool)
        {
            iMemPoolAllocPtr = &aMemPool;
            iLastAllocatedBlockPtr = NULL;
        }

        virtual ~TrackDataMemPoolProxyAlloc()
        {
        }

        OsclAny* allocate(const uint32 size)
        {
            OSCL_ASSERT(iMemPoolAllocPtr);
            iLastAllocatedBlockPtr = iMemPoolAllocPtr->allocate(size);
            return iLastAllocatedBlockPtr;
        }

        void deallocate(OsclAny* p)
        {
            OSCL_ASSERT(iMemPoolAllocPtr);
            iMemPoolAllocPtr->deallocate(p);
        }

        bool trim(uint32 aBytesToFree)
        {
            OSCL_ASSERT(iMemPoolAllocPtr);
            OSCL_ASSERT(iLastAllocatedBlockPtr);
            return iMemPoolAllocPtr->trim(iLastAllocatedBlockPtr, aBytesToFree);
        }

        OsclMemPoolResizableAllocator* iMemPoolAllocPtr;
        OsclAny* iLastAllocatedBlockPtr;
};

class MediaClockConverter;
class PVMFAACFFParserNode;
class PVAACFFNodeTrackPortInfo : public OsclMemPoolFixedChunkAllocatorObserver,
            public OsclMemPoolResizableAllocatorObserver
{
    public:

        PVAACFFNodeTrackPortInfo()
        {
            oADTS = false;
            iTrackId = -1;
            iPort = NULL;
            iClockConverter = NULL;
            iTrackDataMemoryPool = NULL;
            iTrackDataMemoryPoolProxy = NULL;
            iMediaDataImplAlloc = NULL;
            iMediaDataMemPool = NULL;
            iResizableSimpleMediaMsgAlloc = NULL;
            iNode = NULL;
            iSeqNum = 0;
            iTimestampOffset = 0;
            iSendBOS = false;

            /////////////////////////////////////////////////////
            iFormatType                   = PVMF_MIME_FORMAT_UNKNOWN;
            iSeqNum                       = 0;
            oEOSReached                   = false;
            oEOSSent                      = false;
            oQueueOutgoingMessages        = true;
            oProcessOutgoingMessages      = true;
            iTrackBitRate                 = 0;
            iTrackDuration                = 0;
            iContinuousTimeStamp          = 0;
            iPrevSampleTimeStamp          = 0;
            iTrackMaxSampleSize           = 0;

            iLogger = PVLogger::GetLoggerObject("PVMFAACParserNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.aacparsernode");
            if (iDataPathLogger)
                iDataPathLogger->DisableAppenderInheritance();
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iPortLogger = NULL;
            oFormatSpecificInfoLogged = false;

            iAudioSampleRate              = 0;
            iAudioNumChannels             = 0;
            iAudioBitsPerSample           = 0;
            iCodecName					  = NULL;
            iCodecDescription			  = NULL;
            iResizableSimpleMediaMsgAlloc = NULL;
            /////////////////////////////////////////////////////////
        }

        PVAACFFNodeTrackPortInfo(const PVAACFFNodeTrackPortInfo& aSrc) :
                OsclMemPoolFixedChunkAllocatorObserver(aSrc),
                OsclMemPoolResizableAllocatorObserver(aSrc)
        {
            oADTS = aSrc.oADTS;
            iTrackId = aSrc.iTrackId;
            iPort = aSrc.iPort;
            iClockConverter = aSrc.iClockConverter;
            iFormatSpecificConfig = aSrc.iFormatSpecificConfig;
            iMediaData = aSrc.iMediaData;
            iTrackDataMemoryPool = aSrc.iTrackDataMemoryPool;
            iTrackDataMemoryPoolProxy = aSrc.iTrackDataMemoryPoolProxy;
            iMediaDataImplAlloc = aSrc.iMediaDataImplAlloc;
            iMediaDataMemPool = aSrc.iMediaDataMemPool;
            iNode = aSrc.iNode;
            iSeqNum = aSrc.iSeqNum;
            iTimestampOffset = aSrc.iTimestampOffset;
            iSendBOS = aSrc.iSendBOS;

            /////////////////////////////////////////////////
            iSeqNum                            = aSrc.iSeqNum;
            oEOSReached                        = aSrc.oEOSReached;
            oEOSSent                           = aSrc.oEOSSent;
            oQueueOutgoingMessages             = aSrc.oQueueOutgoingMessages;
            oProcessOutgoingMessages           = aSrc.oProcessOutgoingMessages;
            iTrackBitRate                      = aSrc.iTrackBitRate;
            iTrackDuration                     = aSrc.iTrackDuration;
            iContinuousTimeStamp               = aSrc.iContinuousTimeStamp;
            iPrevSampleTimeStamp               = aSrc.iPrevSampleTimeStamp;
            iTrackMaxSampleSize                = aSrc.iTrackMaxSampleSize;
            iLogger					           = aSrc.iLogger;
            iDataPathLogger			           = aSrc.iDataPathLogger;
            iClockLogger			           = aSrc.iClockLogger;
            iPortLogger                        = aSrc.iPortLogger;
            oFormatSpecificInfoLogged          = aSrc.oFormatSpecificInfoLogged;
            iAudioSampleRate                   = aSrc.iAudioSampleRate;
            iAudioNumChannels                  = aSrc.iAudioNumChannels;

            iResizableSimpleMediaMsgAlloc      = aSrc.iResizableSimpleMediaMsgAlloc;
            iMediaDataMemPool                  = aSrc.iMediaDataMemPool;

            iAudioBitsPerSample                = aSrc.iAudioBitsPerSample;
            iCodecName						   = aSrc.iCodecName;;
            iCodecDescription				   = aSrc.iCodecDescription;
        }

        virtual ~PVAACFFNodeTrackPortInfo()
        {
            iLogger					           = NULL;
            iDataPathLogger			           = NULL;
            iClockLogger			           = NULL;
            iPortLogger                        = NULL;
        }

        // From OsclMemPoolFixedChunkAllocatorObserver
        // Callback handler when mempool's deallocate() is called after
        // calling notifyfreechunkavailable() on the mempool
        void freechunkavailable(OsclAny* aContextData)
        {
            OSCL_UNUSED_ARG(aContextData);
            PVMF_AACPARSERNODE_LOGINFO((0, "MimeType = %s, freeblockavailable#############", iTrackMimeType.get_cstr()));
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
            PVMF_AACPARSERNODE_LOGINFO((0, "MimeType = %s, freeblockavailable", iTrackMimeType.get_cstr()));
            oQueueOutgoingMessages = true;
            if (iNode)
            {
                /* Activate the parent node if necessary */
                iNode->RunIfNotReady();
            }
        }
        bool oADTS;

        // Track ID number in AAC FF
        int32 iTrackId;
        /* Track Bitrate */
        uint32 iTrackBitRate;
        /* Track Duration */
        uint64 iTrackDuration;
        // Output port to send the data downstream
        PVMFPortInterface* iPort;

        /* PVMF mime type for track */
        OSCL_HeapString<PVMFAACParserNodeAllocator> iTrackMimeType;

        /* Format type for the port */
        PVMFFormatType iFormatType;

        /* bos flag */
        bool iSendBOS;

        // Settings for the output port
        // Converter to convert from track timescale to milliseconds
        MediaClockConverter* iClockConverter;
        // Shared memory pointer holding the decoder specific config info for this track
        OsclRefCounterMemFrag iFormatSpecificConfig;

        // Shared memory pointer holding the currently retrieved track data
        PVMFSharedMediaDataPtr iMediaData;
        // Output buffer memory pool
        OsclMemPoolResizableAllocator *iTrackDataMemoryPool;
        // Allocator wrapper for the output buffer memory pool
        TrackDataMemPoolProxyAlloc* iTrackDataMemoryPoolProxy;
        // Allocator for simple media data buffer impl
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataImplAlloc;

        /* Allocator for simple media buffer impl */
        PVMFResizableSimpleMediaMsgAlloc* iResizableSimpleMediaMsgAlloc;

        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator *iMediaDataMemPool;
        // AAC FF parser node handle
        OsclTimerObject* iNode;
        // Sequence number
        uint32 iSeqNum;
        // An offset for timestamps after repositioning.
        int32 iTimestampOffset;

        /* port flow control */
        bool oQueueOutgoingMessages;
        bool oProcessOutgoingMessages;
        /* End of Track */
        bool oEOSReached;
        bool oEOSSent;
        /* Continuous timestamp */
        uint64 iContinuousTimeStamp;
        uint64 iPrevSampleTimeStamp;
        /* Mem pool size */
        uint32 iTrackDataMemoryPoolSize;
        /* Track Max Sample Size */
        uint32 iTrackMaxSampleSize;

        /* Loggers */
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;
        /* bitstream logging */
        PVLogger* iPortLogger;
        OSCL_HeapString<PVMFAACParserNodeAllocator> iLogFile;
        bool oFormatSpecificInfoLogged;
        OsclSharedPtr<PVLoggerAppender> iBinAppenderPtr;

        /* Audio - specfic info */
        uint32 iAudioSampleRate;
        uint32 iAudioNumChannels;
        uint32 iAudioBitsPerSample;
        OSCL_wHeapString<OsclMemAllocator> iCodecName;
        OSCL_wHeapString<OsclMemAllocator> iCodecDescription;
};



/**
* Node command handling
*/


/* Default vector reserve size */
#define PVMF_AAC_PARSER_NODE_VECTOR_RESERVE 10
/* Starting value for command IDs  */
#define PVMF_AAC_PARSER_NODE_COMMAND_ID_START 9000

enum TPVMFAACParserNodeCommand
{
    PVMF_AAC_PARSER_NODE_QUERYUUID,
    PVMF_AAC_PARSER_NODE_QUERYINTERFACE,
    PVMF_AAC_PARSER_NODE_REQUESTPORT,
    PVMF_AAC_PARSER_NODE_RELEASEPORT,
    PVMF_AAC_PARSER_NODE_INIT,
    PVMF_AAC_PARSER_NODE_PREPARE,
    PVMF_AAC_PARSER_NODE_START,
    PVMF_AAC_PARSER_NODE_STOP,
    PVMF_AAC_PARSER_NODE_FLUSH,
    PVMF_AAC_PARSER_NODE_PAUSE,
    PVMF_AAC_PARSER_NODE_RESET,
    PVMF_AAC_PARSER_NODE_CANCELALLCOMMANDS,
    PVMF_AAC_PARSER_NODE_CANCELCOMMAND,
    PVMF_AAC_PARSER_NODE_SET_DATASOURCE_POSITION,
    PVMF_AAC_PARSER_NODE_QUERY_DATASOURCE_POSITION,
    PVMF_AAC_PARSER_NODE_SET_DATASOURCE_RATE,
    PVMF_AAC_PARSER_NODE_GETNODEMETADATAKEYS,
    PVMF_AAC_PARSER_NODE_GETNODEMETADATAVALUES,
    PVMF_AAC_PARSER_NODE_SET_DATASOURCE_DIRECTION,
    PVMF_AAC_PARSER_NODE_GET_LICENSE_W,
    PVMF_AAC_PARSER_NODE_CANCEL_GET_LICENSE,
    PVMF_AAC_PARSER_NODE_GET_LICENSE,
    PVMF_AAC_PARSER_NODE_COMMAND_LAST
};


typedef PVMFGenericNodeCommand<OsclMemAllocator> PVMFAACFFParserNodeCommandBase;

enum PVMFAACFFParserNodeCommandType
{
    PVAACFF_NODE_CMD_SETDATASOURCEPOSITION = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVAACFF_NODE_CMD_QUERYDATASOURCEPOSITION
    , PVAACFF_NODE_CMD_SETDATASOURCERATE
    , PVAACFF_NODE_CMD_GETNODEMETADATAKEYS
    , PVAACFF_NODE_CMD_GETNODEMETADATAVALUES
};

class PVMFAACFFParserNodeCommand : public PVMFAACFFParserNodeCommandBase
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
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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

        // Constructor and parser for SetDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS,
                       bool aSeekToSyncPoint, uint32 aStreamID, const OsclAny*aContext)
        {
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                       bool aSeekToSyncPoint, const OsclAny*aContext)
        {
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3) ? true : false;
        }

        // Constructor and parser for SetDataSourceRate
        void Construct(PVMFSessionId s, int32 cmd, int32 aRate, PVMFTimebase* aTimebase, const OsclAny*aContext)
        {
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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
            PVMFAACFFParserNodeCommandBase::Construct(s, cmd, aContext);
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


        //need to overlaod the base Destroy routine to cleanup metadata key.
        void Destroy()
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Destroy();
            switch (iCmd)
            {
                case PVMF_AAC_PARSER_NODE_GETNODEMETADATAKEYS:
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

        //need to overlaod the base Copy routine to copy metadata key.
        void Copy(const PVMFGenericNodeCommand<OsclMemAllocator>& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            switch (aCmd.iCmd)
            {
                case PVMF_AAC_PARSER_NODE_GETNODEMETADATAKEYS:
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

typedef PVMFNodeCommandQueue<PVMFAACFFParserNodeCommand, OsclMemAllocator> PVMFAACFFParserNodeCmdQ;



/** The memory allocator for this node
*/
typedef OsclMemAllocator PVMFAACFFParserNodeAllocator;

/** The AAC file parser node class
*/
class CAACFileParser;
class PVMFAACFFParserOutPort;
class PVLogger;
class PVMFLocalDataSource;

class PVMFAACFFParserNode :  public OsclTimerObject
            , public PVMFNodeInterface
            , public PVMFDataSourceInitializationExtensionInterface
            , public PVMFTrackSelectionExtensionInterface
            , public PvmfDataSourcePlaybackControlInterface
            , public PVMFMetadataExtensionInterface
            , public PVMFCPMStatusObserver
            , public PVMIDatastreamuserInterface
            , public PvmiDataStreamObserver
            , public PVMFFormatProgDownloadSupportInterface
            , public PVMFCPMPluginLicenseInterface
{
    public:
        PVMFAACFFParserNode(int32 aPriority);
        ~PVMFAACFFParserNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, PVMFAACFFParserNodeAllocator>& aUuids,
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
        PVMFStatus QueryInterfaceSync(PVMFSessionId aSession,
                                      const PVUuid& aUuid,
                                      PVInterface*& aInterfacePtr);

        //From PVMFDataSourceInitializationExtensionInterface
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& uuid, PVInterface *& iface);

        PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL, PVMFFormatType& aSourceFormat, OsclAny* aSourceData);
        PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        //From PVMFTrackSelectionExtensionInterface
        PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        /* From PVMFCPMStatusObserver */
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
        void notifyDownloadComplete() {};


        // From PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                          uint32 aStartingKeyIndex, int32 aMaxKeyEntries, char* aQueryKeyString = NULL, const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                                            uint32 aStartingKeyIndex, int32 aMaxKeyEntries, const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, uint32 aEndKeyIndex);
        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, uint32 aEndValueIndex);

        /* From PvmiDataStreamObserver */
        void DataStreamCommandCompleted(const PVMFCmdResp& aResponse);
        void DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent);
        void DataStreamErrorEvent(const PVMFAsyncEvent& aEvent);

        // From PvmfDataSourcePlaybackControlInterface
        PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId
                                            , PVMFTimestamp aTargetNPT
                                            , PVMFTimestamp& aActualNPT
                                            , PVMFTimestamp& aActualMediaDataTS
                                            , bool aSeekToSyncPoint = true
                                                                      , uint32 aStreamID = -1
                                                                                           , OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId
                                              , PVMFTimestamp aTargetNPT
                                              , PVMFTimestamp& aActualNPT
                                              , bool aSeekToSyncPoint = true
                                                                        , OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId
                                              , PVMFTimestamp aTargetNPT
                                              , PVMFTimestamp& aSeekPointBeforeTargetNPT
                                              , PVMFTimestamp& aSeekPointAfterTargetNPT
                                              , OsclAny* aContext = NULL
                                                                    , bool aSeekToSyncPoint = true);
        PVMFCommandId SetDataSourceRate(PVMFSessionId aSession
                                        , int32 aRate
                                        , PVMFTimebase* aTimebase = NULL
                                                                    , OsclAny* aContext = NULL);

        /* From PVMFCPMPluginLicenseInterface */
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OSCL_wString& aLicenseURL)
        {
            //must use Async method.
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            return PVMFErrNotSupported;
        }
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OSCL_String&  aLicenseURL)
        {
            //must use Async method.
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
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

    private:

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        //from OsclTimerObject
        void Run();

        // Port processing
///////////////////////////////////////////////////////////////////////////////////
        bool ProcessPortActivity(PVAACFFNodeTrackPortInfo*);
        PVMFStatus RetrieveMediaSample(PVAACFFNodeTrackPortInfo* aTrackInfoPtr,
                                       PVMFSharedMediaDataPtr& aSharedPtr);
        PVMFStatus QueueMediaSample(PVAACFFNodeTrackPortInfo* aTrackInfoPtr);
        PVMFStatus ProcessOutgoingMsg(PVAACFFNodeTrackPortInfo* aTrackInfoPtr);
        bool GetTrackInfo(PVMFPortInterface* aPort,
                          PVAACFFNodeTrackPortInfo*& aTrackInfoPtr);
        bool GetTrackInfo(int32 aTrackID,
                          PVAACFFNodeTrackPortInfo*& aTrackInfoPtr);
        PVMFStatus GenerateAndSendEOSCommand(PVAACFFNodeTrackPortInfo* aTrackInfoPtr);
        bool SendBeginOfMediaStreamCommand(PVAACFFNodeTrackPortInfo* aTrackPortInfo);
        bool CheckForPortRescheduling();
        bool CheckForPortActivityQueues();
////////////////////////////////////////////////////////////////////////////////////


        /* Command processing */
        void MoveCmdToCurrentQueue(PVMFAACFFParserNodeCommand& aCmd);
        void MoveCmdToCancelQueue(PVMFAACFFParserNodeCommand& aCmd);
        void CommandComplete(PVMFAACFFParserNodeCmdQ& aCmdQueue,
                             PVMFAACFFParserNodeCommand& aCmd,
                             PVMFStatus aStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL,
                             PVInterface* aExtMsg = NULL);
        PVMFCommandId QueueCommandL(PVMFAACFFParserNodeCommand&);
        void ProcessCommand();
        bool FlushPending();
        PVMFAACFFParserNodeCmdQ iInputCommands;
        PVMFAACFFParserNodeCmdQ iCurrentCommand;
        PVMFAACFFParserNodeCmdQ iCancelCommand;

        //Command handlers.
        void DoReset(PVMFAACFFParserNodeCommand&);
        void  CompleteReset();

        void DoQueryUuid(PVMFAACFFParserNodeCommand&);
        void DoQueryInterface(PVMFAACFFParserNodeCommand&);
        void DoRequestPort(PVMFAACFFParserNodeCommand&, PVMFPortInterface*&);
        void DoReleasePort(PVMFAACFFParserNodeCommand&);
        void DoInit(PVMFAACFFParserNodeCommand&);
        void CompleteInit();

        void DoPrepare(PVMFAACFFParserNodeCommand&);
        void DoStart(PVMFAACFFParserNodeCommand&);
        void DoStop(PVMFAACFFParserNodeCommand&);
        void DoFlush(PVMFAACFFParserNodeCommand&);
        void DoPause(PVMFAACFFParserNodeCommand&);
        void DoCancelAllCommands(PVMFAACFFParserNodeCommand&);
        void DoCancelCommand(PVMFAACFFParserNodeCommand&);
        // For metadata extention interface
        PVMFStatus DoGetMetadataKeys(PVMFAACFFParserNodeCommand& aCmd);
        PVMFStatus DoGetMetadataValues(PVMFAACFFParserNodeCommand& aCmd);

        void CompleteGetMetaDataValues();
        int32 AddToValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, PvmiKvp& aNewValue);
        PVMFStatus CompleteGetMetadataKeys(PVMFAACFFParserNodeCommand& aCmd);

        // For data source position extension interface
        PVMFStatus DoSetDataSourcePosition(PVMFAACFFParserNodeCommand& aCmd);
        PVMFStatus DoQueryDataSourcePosition(PVMFAACFFParserNodeCommand& aCmd);
        PVMFStatus DoSetDataSourceRate(PVMFAACFFParserNodeCommand& aCmd);

        // Event reporting
        void ReportAACFFParserErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        void ReportAACFFParserInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        void SetState(TPVMFNodeInterfaceState);

        // Track data processing
        bool RetrieveTrackConfigInfo(PVAACFFNodeTrackPortInfo& aTrackPortInfo);
        bool RetrieveTrackData(PVAACFFNodeTrackPortInfo& aTrackPortInfo);
        bool SendEndOfTrackCommand(PVAACFFNodeTrackPortInfo& aTrackPortInfo);

        void ResetAllTracks();
        bool ReleaseTrack();
        void CleanupFileSource();
        void ReleaseMetadataValue(PvmiKvp& aValueKVP);


    private:  // private data members
        void ConstructL();

        PVMFNodeCapability iCapability;
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;


        void ReleaseAllPorts();
        void RemoveAllCommands();

        // ports contained in this node
        PVMFAACFFParserOutPort* iOutPort;
        friend class PVMFAACFFParserOutPort;

        PVMFCommandId iCurrentCmdId;

        //Source data
        OSCL_wHeapString<OsclMemAllocator> iSourceURL;
        PVMFFormatType iSourceFormat;
        bool iUseCPMPluginRegistry;
        OsclFileHandle* iFileHandle;
        PVMFLocalDataSource iCPMSourceData;

        Oscl_Vector<PVAACFFNodeTrackPortInfo, OsclMemAllocator> iSelectedTrackList;

        Oscl_FileServer iFileServer;
        CAACFileParser* iAACParser;

        PVAACFFNodeTrackPortInfo iTrack;
        bool iFirstFrame;

        PVMFMediaClock* iClientClock;
        void ResetSourceFile();
        bool oSourceIsCurrent;

        // Reference counter for extension
        uint32 iExtensionRefCount;

        //for meta-data
        PVMFMetadataList iAvailableMetadataKeys;
        bool iID3DataValid;
        TPVAacFileInfo iAACFileInfo;
        bool iAACFileInfoValid;

        /* Meta data related */
        void PushToAvailableMetadataKeysList(const char* aKeystr, char* aOptionalParam = NULL);
        PVMFStatus InitMetaData();
        Oscl_Vector<OSCL_HeapString<PVMFAACParserNodeAllocator>, PVMFAACParserNodeAllocator> iCPMMetadataKeys;
        PVMFStatus GetIndexParamValues(char* aString,
                                       uint32& aStartIndex,
                                       uint32& aEndIndex);
        PVMFStatus GetMaxSizeValue(char* aString, uint32& aMaxSize);
        PVMFStatus GetTruncateFlagValue(char* aString, uint32& aTruncateFlag);


        uint32 iAACParserNodeMetadataValueCount;
        uint32 iGenreIndex;
        uint32 iLyricsIndex;

        //bos flag
        bool iSendBOS;
        //stream id
        uint32 iStreamID;

        //for CPM

        /* Content Policy Manager related */
        bool iPreviewMode;
        PVMFCPM* iCPM;
        PVMFSessionId iCPMSessionID;
        PVMFCPMContentType iCPMContentType;
        PVMFCPMPluginAccessInterfaceFactory* iCPMContentAccessFactory;
        PVMFMetadataExtensionInterface* iCPMMetaDataExtensionInterface;
        PVMFCPMPluginLicenseInterface* iCPMLicenseInterface;
        PVInterface* iCPMLicenseInterfacePVI;
        PvmiKvp iRequestedUsage;
        PvmiKvp iApprovedUsage;
        PvmiKvp iAuthorizationDataKvp;
        PVMFCPMUsageID iUsageID;
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
        bool GetCPMMetaDataExtensionInterface();
        void GetCPMLicenseInterface();
        void RequestUsage();
        void SendUsageComplete();
        void CloseCPMSession();
        void ResetCPM();
        void PopulateDRMInfo();
        void GetCPMMetaDataKeys();
        void GetCPMMetaDataValues();
        PVMFStatus CheckCPMCommandCompleteStatus(PVMFCommandId, PVMFStatus);
        int32 CreateNewArray(char*& aPtr, char *aKey);
        int32 CreateNewArray(char*& aPtr, int32 aLen);
        int32 CreateNewArray(oscl_wchar*& aPtr, int32 aLen);
        int32 PushBackKeyVal(Oscl_Vector<PvmiKvp, OsclMemAllocator>*& aValueListPtr, PvmiKvp &aKeyVal);
        PVMFStatus PushValueToList(Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &aRefMetadataKeys,
                                   PVMFMetadataList *&aKeyListPtr,
                                   uint32 aLcv);
        PVMFStatus iCPMRequestUsageCommandStatus;

        PVMFStatus DoGetLicense(PVMFAACFFParserNodeCommand& aCmd,
                                bool aWideCharVersion = false);
        void CompleteGetLicense();
        void DoCancelGetLicense(PVMFAACFFParserNodeCommand& aCmd);

        /* Progressive download related */
        PVMFStatus CheckForAACHeaderAvailability();
        void PauseAllMediaRetrieval();
        uint64 iAACHeaderSize;
        bool iAutoPaused;
        bool iDownloadComplete;
        PVMFDownloadProgressInterface* iDownloadProgressInterface;
        uint32 iDownloadFileSize;
        PVMIDataStreamSyncInterface* iDataStreamInterface;
        PVMFDataStreamFactory* iDataStreamFactory;
        PVMFDataStreamReadCapacityObserver* iDataStreamReadCapacityObserver;
        PvmiDataStreamSession iDataStreamSessionID;
        PvmiDataStreamCommandId iRequestReadCapacityNotificationID;
        uint32 iLastNPTCalcInConvertSizeToTime;
        uint32 iFileSizeLastConvertedToTime;

        /* bitstream logging */
        void LogMediaData(PVMFSharedMediaDataPtr data,
                          PVMFPortInterface* aPort);
        bool iPortDataLog;
        char iLogFileIndex;
        OSCL_HeapString<PVMFAACParserNodeAllocator> portLogPath;

        PVMFStatus ParseAACFile();
};

#endif // PVMF_AACFFPARSER_NODE_H_INCLUDED

