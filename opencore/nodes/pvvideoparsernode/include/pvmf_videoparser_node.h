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
 *
 * @file pvmf_videoparser_node.h
 * @brief Video parser node. Parses incoming video bitstream into decodable frames.
 *
 */

#ifndef PVMF_VIDEOPARSER_NODE_H_INCLUDED
#define PVMF_VIDEOPARSER_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
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
#ifndef PVT_COMMON_H_INCLUDED
#include "pvt_common.h"
#endif

#define MAX_VIDEO_FRAME_PARSE_SIZE 22000
#define MAX_VIDEO_FRAMES 3
#define PVVIDEOPARSER_MEDIADATA_SIZE 128

typedef struct vidparser_port_property
{
    PVMFFormatType format;
    OSCL_HeapString<OsclMemAllocator> mimetype;
    int32		porttag;
} VideoParserPortProperty;

////////////////////////////////////////////////////////////////////////////
class PVMFVideoParserAlloc : public Oscl_DefAlloc
{
    public:
        void* allocate(const uint32 size)
        {
            void* tmp = (void*)oscl_malloc(size);
            return tmp;
        }

        void deallocate(void* p)
        {
            oscl_free(p);
        }
};

enum PVMFVideoParserNodeCmdType
{
    PVMFVIDEOPARSER_NODE_CMD_INIT,
    PVMFVIDEOPARSER_NODE_CMD_REQUESTPORT,
    PVMFVIDEOPARSER_NODE_CMD_PREPARE,
    PVMFVIDEOPARSER_NODE_CMD_START,
    PVMFVIDEOPARSER_NODE_CMD_PAUSE,
    PVMFVIDEOPARSER_NODE_CMD_STOP,
    PVMFVIDEOPARSER_NODE_CMD_RELEASEPORT,
    PVMFVIDEOPARSER_NODE_CMD_QUERY_INTERFACE,
    PVMFVIDEOPARSER_NODE_CMD_QUERY_UUID,
    PVMFVIDEOPARSER_NODE_CMD_RESET,
    PVMFVIDEOPARSER_NODE_CMD_FLUSH,
    PVMFVIDEOPARSER_NODE_CMD_CANCELCMD,
    PVMFVIDEOPARSER_NODE_CMD_CANCELALL,
    PVMFVIDEOPARSER_NODE_CMD_INVALID
};

class PVMFVideoParserNodeCmd
{
    public:
        PVMFVideoParserNodeCmd() : iSession(-1),
                iId(-1),
                iType(PVMFVIDEOPARSER_NODE_CMD_INVALID),
                iPort(NULL),
                iContext(NULL),
                iData(NULL)

        {}

        PVMFVideoParserNodeCmd(const PVMFVideoParserNodeCmd& aCmd) :
                iSession(aCmd.iSession),
                iId(aCmd.iId),
                iType(aCmd.iType),
                iPort(aCmd.iPort),
                iContext(aCmd.iContext),
                iData(aCmd.iData)
        {}

        ~PVMFVideoParserNodeCmd()
        {}

        PVMFSessionId iSession;
        int32 iId;
        int32 iType;
        PVMFPortInterface* iPort;
        OsclAny* iContext;
        OsclAny* iData;

        /**
         * Equality operator, for use by OsclPriorityQueue.
         */
        bool operator==(const PVMFVideoParserNodeCmd& aCmd) const
        {
            return iId == aCmd.iId;
        }

};



class PVMFVideoParserNodeCmdCompareLess
{
    public:
        /**
        * The algorithm used in OsclPriorityQueue needs a compare function
        * that returns true when A's priority is less than B's
        * @return true if A's priority is less than B's, else false
        */
        int compare(PVMFVideoParserNodeCmd& a, PVMFVideoParserNodeCmd& b) const
        {
            return (PVMFVideoParserNodeCmdCompareLess::GetPriority(a) < PVMFVideoParserNodeCmdCompareLess::GetPriority(b));
        }

        /**
        * Returns the priority of each command
        * @return A 0-based priority number. A lower number indicates lower priority.
        */
        static int GetPriority(PVMFVideoParserNodeCmd& aCmd)
        {
            switch (aCmd.iType)
            {
                case PVMFVIDEOPARSER_NODE_CMD_INIT:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_REQUESTPORT:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_START:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_PAUSE:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_STOP:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_RELEASEPORT:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_RESET:
                    return 5;
                case PVMFVIDEOPARSER_NODE_CMD_CANCELCMD:
                    return 9;
                case PVMFVIDEOPARSER_NODE_CMD_CANCELALL:
                    return 10;
                default:
                    return 0;
            }
        }
};

class PVLogger;

////////////////////////////////////////////////////////////////////////////
class PVMFVideoParserNode : public OsclActiveObject, public PVMFNodeInterface
{
    public:
        OSCL_IMPORT_REF static PVMFNodeInterface* Create(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen);
        OSCL_IMPORT_REF ~PVMFVideoParserNode();

        // Virtual functions of PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();

        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF void HandlePortActivity(const PVMFPortActivity&);

        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId aSession,
                                                const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSession,
                const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId aSession
                , PVMFPortInterface& aPort
                , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId aSession
                                           , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId aSession
                                              , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId aSession
                                           , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId aSession
                , const OsclAny* aContextData = NULL) ;
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId aSession
                , PVMFCommandId aCmdId
                , const OsclAny* aContextData = NULL) ;


        // For input port to access private function / data
        friend class PVMFVideoParserPort;

        void Run();

    private:
        int AddPortActivity(const PVMFPortActivity& activity);

        PVMFVideoParserNode();
        void ConstructL(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen);
        void DoCancel();

        void QueuePortActivity(const PVMFPortActivity &aActivity);

        bool ProcessPortActivity();
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        PVMFStatus ProcessIncomingMsg(PVMFPortInterface* aPort);

        // Handle command and data events
        PVMFCommandId AddCmdToQueue(PVMFVideoParserNodeCmdType aType, PVMFSessionId s, PVMFPortInterface* aPort, const OsclAny* aContext, OsclAny* aData = NULL);
        PVMFStatus HandleCmdStart(PVMFVideoParserNodeCmd* aCmd);
        PVMFPortInterface *HandleCmdRequestPort(PVMFVideoParserNodeCmd* aCmd, PVMFStatus &aStatus);
        PVMFStatus HandleCmdReleasePort(PVMFVideoParserNodeCmd* aCmd);

        void DataReceived(OsclSharedPtr<PVMFMediaMsg>& aMsg);
        void SendFrame();
        bool FrameMarkerExists(uint8* aDataPtr, int32 aDataSize, uint32 aCrcError);

        Oscl_Vector<PVMFPortActivity, OsclMemAllocator> iPortActivityQueue;
        // Queue of commands
        OsclPriorityQueue<PVMFVideoParserNodeCmd, PVMFVideoParserAlloc, Oscl_Vector<PVMFVideoParserNodeCmd, PVMFVideoParserAlloc>, PVMFVideoParserNodeCmdCompareLess> iCmdQueue;
        PVMFCommandId iCmdIdCounter;

        // Input port
        PVMFPortInterface* iInputPort;
        PVMFPortInterface* iOutputPort;

        // Allocator
        Oscl_DefAlloc* iAlloc;

        uint8 iNumOnes[256];
        PVMFFormatType iFormatType;
        // Integer Format type for the port
        PVCodecType_t iFormatTypeInteger;
        bool iFirstFrameFound;
        PVMFSharedMediaDataPtr iVideoFrame;

        // Frame buffer memory pool
        OsclMemPoolFixedChunkAllocator *iVidParserMemoryPool;
        // Allocator for simple media data buffer
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataAlloc;
        // Memory pool for simple media data
        OsclMemPoolFixedChunkAllocator iMediaDataMemPool;

        PVLogger *iLogger;

        // Format specific info associated with the codec in this datapath
        uint8* iFormatSpecificInfo;
        uint32 iFormatSpecificInfoLen;
};

#endif // PVMF_VIDEOPARSER_NODE_H_INCLUDED
