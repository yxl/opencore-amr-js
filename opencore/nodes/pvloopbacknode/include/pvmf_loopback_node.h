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
 * @file pvmf_loopback_node.h
 * @brief Simple loopback node. Incoming data is immediately routed to outport if it exists
 *
 */

#ifndef PVMF_LOOPBACK_NODE_H_INCLUDED
#define PVMF_LOOPBACK_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
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

#ifndef PVMF_NODE_UTIL_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_LOOPBACK_IOPORT_H_INCLUDED
#include "pvmf_loopback_ioport.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

////////////////////////////////////////////////////////////////////////////


//Default vector reserve size
#define PVMF_LOOPBACK_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_LOOPBACK_NODE_COMMAND_ID_START 6000


typedef enum
{
    PVMF_LOOPBACKNODE_PORT_TYPE_LOOPBACK = 3
} PVMFLoopbackNodePortType;

//memory allocator type for this node.
typedef OsclMemAllocator PVMFLoopbackNodeAllocator;

///////////////////////////////////////////////////////////////////////////
class PVMFLoopbackAlloc : public Oscl_DefAlloc
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


typedef PVMFGenericNodeCommand<PVMFLoopbackNodeAllocator> PVMFLoopbackNodeCommandBase;
class PVMFLoopbackNodeCmd: public PVMFGenericNodeCommand<PVMFLoopbackNodeAllocator>
{
    public:
        //constructor for Custom2 command
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFLoopbackNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }
};

enum PVMFLoopbackNodeCmdType
{
    PVMFLOOPBACK_NODE_CMD_INIT,
    PVMFLOOPBACK_NODE_CMD_REQUESTPORT,
    PVMFLOOPBACK_NODE_CMD_START,
    PVMFLOOPBACK_NODE_CMD_PAUSE,
    PVMFLOOPBACK_NODE_CMD_STOP,
    PVMFLOOPBACK_NODE_CMD_RELEASEPORT,
    PVMFLOOPBACK_NODE_CMD_RESET,
    PVMFLOOPBACK_NODE_CMD_PREPARE,
    PVMFLOOPBACK_NODE_CMD_FLUSH,
    PVMFLOOPBACK_NODE_CMD_CANCELCMD,
    PVMFLOOPBACK_NODE_CMD_CANCELALL,
    PVMFLOOPBACK_NODE_CMD_INVALID
};

typedef PVMFNodeCommandQueue<PVMFLoopbackNodeCmd, PVMFLoopbackNodeAllocator> PVMFLoopbackNodeCmdQ;


// Forward declaration
class PVMFLoopbackIOPort;

////////////////////////////////////////////////////////////////////////////
class PVMFLoopbackNode : public PVMFNodeInterface
            , public OsclActiveObject
{
    public:

        OSCL_IMPORT_REF static PVMFNodeInterface* Create();
        OSCL_IMPORT_REF ~PVMFLoopbackNode();

        // Virtual functions of PVMFNodeInterface
        //from PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL)
        {
            OSCL_UNUSED_ARG(aFilter);
            return &iPortVector;
        }
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMFLoopbackNodeAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId
                , int32 aPortTag, const PvmfMimeString* aPortConfig = NULL, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);


        // For input port to access private function / data
        friend class PVMFLoopbackIOPort;
        //from PVMFPortActivityHandler.
        void HandlePortActivity(const PVMFPortActivity& aActivity)
        {
            OSCL_UNUSED_ARG(aActivity);
        };

        PVMFPortVector<PVMFLoopbackIOPort, PVMFLoopbackNodeAllocator> iPortVector;

    private:
        PVMFLoopbackNode(int32 aPriority);
        void DoCancel();
        void Run();

        //Command processing
        PVMFCommandId QueueCommandL(PVMFLoopbackNodeCmd&);
        bool ProcessCommand(PVMFLoopbackNodeCmd&);
        void CommandComplete(PVMFLoopbackNodeCmdQ&, PVMFLoopbackNodeCmd&, PVMFStatus, OsclAny* aData = NULL);
        bool FlushPending();
        PVMFLoopbackNodeCmdQ iInputCommands;
        PVMFLoopbackNodeCmdQ iCurrentCommand;

        //Command handlers.
        void DoReset(PVMFLoopbackNodeCmd&);
        void DoRequestPort(PVMFLoopbackNodeCmd&);
        void DoReleasePort(PVMFLoopbackNodeCmd&);
        void DoInit(PVMFLoopbackNodeCmd&);
        void DoPrepare(PVMFLoopbackNodeCmd&);
        void DoStart(PVMFLoopbackNodeCmd&);
        void DoStop(PVMFLoopbackNodeCmd&);
        void DoFlush(PVMFLoopbackNodeCmd&);
        void DoPause(PVMFLoopbackNodeCmd&);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void SetState(TPVMFNodeInterfaceState);


        PVMFCommandId iCmdIdCounter;

        // Input port
        PVMFLoopbackIOPort* iIOPort;

        PVMFNodeCapability iCapability;
        PVLogger *iLogger;

        PVMFNodeCmdStatusObserver *iObserver;
};

#endif // PVMF_LOOPBACK_NODE_H_INCLUDED
