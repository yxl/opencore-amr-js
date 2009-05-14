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
#ifndef PVMF_OMX_BASEDEC_NODE_H_INCLUDED
#define PVMF_OMX_BASEDEC_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
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
#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_OMX_BASEDEC_PORT_H_INCLUDED
#include "pvmf_omx_basedec_port.h"
#endif

#ifndef PVMF_OMX_BASEDEC_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvmf_omx_basedec_node_extension_interface.h"
#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PV_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef PVMF_OMX_BASEDEC_CALLBACKS_H_INCLUDED
#include "pvmf_omx_basedec_callbacks.h"
#endif

#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#if (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_REL)
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#endif

#ifndef PVMF_FIXEDSIZE_BUFFER_ALLOC_H_INCLUDED
#include "pvmf_fixedsize_buffer_alloc.h"
#endif

#define MAX_NAL_PER_FRAME 100

typedef struct OutputBufCtrlStruct
{
    OMX_BUFFERHEADERTYPE *pBufHdr;
}OutputBufCtrlStruct;

typedef struct InputBufCtrlStruct
{
    OMX_BUFFERHEADERTYPE *pBufHdr;
    PVMFSharedMediaDataPtr pMediaData;
} InputBufCtrlStruct;


// fwd class declaration
class PVLogger;
//memory allocator type for this node.
typedef OsclMemAllocator PVMFOMXBaseDecNodeAllocator;


// CALLBACK PROTOTYPES
OMX_ERRORTYPE CallbackEventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                   OMX_OUT OMX_PTR aAppData,
                                   OMX_OUT OMX_EVENTTYPE aEvent,
                                   OMX_OUT OMX_U32 aData1,
                                   OMX_OUT OMX_U32 aData2,
                                   OMX_OUT OMX_PTR aEventData);

OMX_ERRORTYPE CallbackEmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                      OMX_OUT OMX_PTR aAppData,
                                      OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

OMX_ERRORTYPE CallbackFillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                     OMX_OUT OMX_PTR aAppData,
                                     OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);



//Default values for number of Input/Output buffers. If the component needs more than this, it will be
// negotiated. If the component does not need more than this number, the default is used
#define NUMBER_INPUT_BUFFER 5
#define NUMBER_OUTPUT_BUFFER 9

// nal start code is 0001
#define NAL_START_CODE_SIZE 4
const unsigned char NAL_START_CODE[4] = {0, 0, 0, 1};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////
/////////////////////////
/////////////////////////
// CUSTOM DEALLOCATOR FOR MEDIA DATA SHARED PTR WRAPPER:
//						1) Deallocates the underlying output buffer
//						2) Deallocates the pvci buffer wrapper and the rest of accompanying structures
//					  Deallocator is created as part of the wrapper, and travels with the buffer wrapper

class PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA : public OsclDestructDealloc
{
    public:
        PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA(Oscl_DefAlloc* allocator, void *pMempoolData) :
                buf_alloc(allocator), ptr_to_data_to_dealloc(pMempoolData) {};
        virtual ~PVOMXDecBufferSharedPtrWrapperCombinedCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // call buffer deallocator
            if (buf_alloc != NULL)
            {
                buf_alloc->deallocate(ptr_to_data_to_dealloc);
            }

            // finally, free the shared ptr wrapper memory
            oscl_free(ptr);
        }

    private:
        Oscl_DefAlloc* buf_alloc;
        void *ptr_to_data_to_dealloc;
};


#define PVMFOMXBaseDecNodeCommandBase PVMFGenericNodeCommand<PVMFOMXBaseDecNodeAllocator>

class PVMFOMXBaseDecNodeCommand: public PVMFOMXBaseDecNodeCommandBase
{
    public:
        //constructor for Custom2 command
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFOMXBaseDecNodeCommandBase::Construct(s, cmd, aContext);
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

        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList* aKeyList, uint32 aStartIndex, int32 aMaxEntries, char* aQueryKey, const OsclAny* aContext)
        {
            PVMFOMXBaseDecNodeCommandBase::Construct(s, cmd, aContext);
            iStartIndex = aStartIndex;
            iMaxEntries = aMaxEntries;

            if (aQueryKey == NULL)
            {
                query_key[0] = 0;
            }
            else
            {
                if (aQueryKey != NULL)
                    oscl_strncpy(query_key, aQueryKey, oscl_strlen(aQueryKey) + 1);
            }

            iParam1 = (OsclAny*)aKeyList;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        void Parse(PVMFMetadataList*& MetaDataListPtr, uint32 &aStartingIndex, int32 &aMaxEntries, char*&aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = iStartIndex;
            aMaxEntries = iMaxEntries;
            if (query_key[0] == 0)
            {
                aQueryKey = NULL;
            }
            else
            {
                aQueryKey = query_key;
            }
        }

        // Constructor and parser for GetNodeMetadataValue
        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList* aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* aValueList, uint32 aStartIndex, int32 aMaxEntries, const OsclAny* aContext)
        {
            PVMFOMXBaseDecNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aKeyList;
            iParam2 = (OsclAny*)aValueList;

            iStartIndex = aStartIndex;
            iMaxEntries = aMaxEntries;

            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFMetadataList* &aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList, uint32 &aStartingIndex, int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = iStartIndex;
            aMaxEntries = iMaxEntries;
        }

        virtual bool hipri()
        {
            //this routine identifies commands that need to
            //go at the front of the queue.  derived command
            //classes can override it if needed.
            return (iCmd == PVOMXBASEDEC_NODE_CMD_CANCELALL
                    || iCmd == PVOMXBASEDEC_NODE_CMD_CANCELCMD);
        }

        enum PVOMXBaseDecNodeCmdType
        {
            PVOMXBASEDEC_NODE_CMD_QUERYUUID,
            PVOMXBASEDEC_NODE_CMD_QUERYINTERFACE,
            PVOMXBASEDEC_NODE_CMD_INIT,
            PVOMXBASEDEC_NODE_CMD_PREPARE,
            PVOMXBASEDEC_NODE_CMD_REQUESTPORT,
            PVOMXBASEDEC_NODE_CMD_START,
            PVOMXBASEDEC_NODE_CMD_PAUSE,
            PVOMXBASEDEC_NODE_CMD_STOP,
            PVOMXBASEDEC_NODE_CMD_FLUSH,
            PVOMXBASEDEC_NODE_CMD_RELEASEPORT,
            PVOMXBASEDEC_NODE_CMD_RESET,
            PVOMXBASEDEC_NODE_CMD_CANCELCMD,
            PVOMXBASEDEC_NODE_CMD_CANCELALL,
            PVOMXBASEDEC_NODE_CMD_INVALID,
            PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAKEY,
            PVOMXBASEDEC_NODE_CMD_GETNODEMETADATAVALUE
        };

    private:
        uint32 iStartIndex;
        uint32 iMaxEntries;
        char query_key[256];

};

//Default vector reserve size
#define PVMF_OMXBASEDEC_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_OMXBASEDEC_NODE_COMMAND_ID_START 6000

///////////////////////////////////////////////////////////////////////////////////////////////////////
//CAPABILITY AND CONFIG

// Structure to hold the key string info for
// capability-and-config
struct PVOMXBaseDecNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

// The number of characters to allocate for the key string
#define PVOMXBASEDECNODECONFIG_KEYSTRING_SIZE 128

//Mimetypes for the custom interface
#define PVMF_OMX_BASE_DEC_NODE_MIMETYPE "pvxxx/OMXBaseDecNode"
#define PVMF_BASEMIMETYPE "pvxxx"

//Command queue type
typedef PVMFNodeCommandQueue<PVMFOMXBaseDecNodeCommand, PVMFOMXBaseDecNodeAllocator> PVMFOMXBaseDecNodeCmdQ;


class PVMFOMXBaseDecNode
            : public OsclActiveObject
            , public PVMFNodeInterface
            , public OsclMemPoolFixedChunkAllocatorObserver
            , public PVMFOMXBaseDecNodeExtensionInterface
            , public PVMFMetadataExtensionInterface
            , public PvmiCapabilityAndConfig

{
    public:
        OSCL_IMPORT_REF PVMFOMXBaseDecNode(int32 aPriority, const char aAOName[]);
        OSCL_IMPORT_REF virtual ~PVMFOMXBaseDecNode();

        // From PVMFNodeInterface
        OSCL_IMPORT_REF virtual PVMFStatus ThreadLogon() = 0;
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMFOMXBaseDecNodeAllocator>& aUuids,
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

        // From PVMFPortActivityHandler
        OSCL_IMPORT_REF void HandlePortActivity(const PVMFPortActivity& aActivity);

        // From PVInterface
        OSCL_IMPORT_REF virtual void addRef();
        OSCL_IMPORT_REF virtual void removeRef();
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF virtual PVMFStatus SetDecoderNodeConfiguration(PVMFOMXBaseDecNodeConfig& aConfig);

        //**********begin PVMFMetadataExtensionInterface
        OSCL_IMPORT_REF virtual uint32 GetNumMetadataKeys(char* query_key = NULL) = 0;
        OSCL_IMPORT_REF virtual uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList) = 0;
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries,
                char* query_key = NULL, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 starting_index, uint32 end_index);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, uint32 end_index);
        //**********End PVMFMetadataExtensionInterface
        OSCL_IMPORT_REF virtual bool VerifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements) = 0;

        //********** CB Functions to serve OpenMAX Decoder

        //Process callback functions. They will be executed in testapp thread context
        //	These callbacks are used only in the Multithreaded component case
        OSCL_IMPORT_REF OsclReturnCode ProcessCallbackEventHandler_MultiThreaded(OsclAny* P);
        OSCL_IMPORT_REF OsclReturnCode ProcessCallbackEmptyBufferDone_MultiThreaded(OsclAny* P);
        OSCL_IMPORT_REF OsclReturnCode ProcessCallbackFillBufferDone_MultiThreaded(OsclAny* P);

        //Callback objects - again, these are used only in the case of Multithreaded component
        EventHandlerThreadSafeCallbackAO*	 iThreadSafeHandlerEventHandler;
        EmptyBufferDoneThreadSafeCallbackAO* iThreadSafeHandlerEmptyBufferDone;
        FillBufferDoneThreadSafeCallbackAO*  iThreadSafeHandlerFillBufferDone;

        OMX_CALLBACKTYPE       iCallbacks; // structure that contains callback ptrs.
        // OMX CALLBACKS
        // 1) AO OMX component running in the same thread as the OMX node
        //	In this case, the callbacks can be called directly from the component
        //	The callback: OMX Component->CallbackEventHandler->EventHandlerProcessing
        //	The callback can perform do RunIfNotReady

        // 2) Multithreaded component
        //	In this case, the callback is made using the threadsafe callback (TSCB) AO
        //	Component thread : OMX Component->CallbackEventHandler->TSCB(ReceiveEvent)
        //  Node thread		 : TSCB(ProcessEvent)->ProcessCallbackEventHandler_MultiThreaded->EventHandlerProcessing


        //==============================================================================

        OSCL_IMPORT_REF virtual OMX_ERRORTYPE EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                OMX_OUT OMX_PTR aAppData,
                OMX_OUT OMX_EVENTTYPE aEvent,
                OMX_OUT OMX_U32 aData1,
                OMX_OUT OMX_U32 aData2,
                OMX_OUT OMX_PTR aEventData) = 0;

        OSCL_IMPORT_REF OMX_ERRORTYPE EmptyBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                OMX_OUT OMX_PTR aAppData,
                OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        OSCL_IMPORT_REF OMX_ERRORTYPE FillBufferDoneProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                OMX_OUT OMX_PTR aAppData,
                OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        bool IsComponentMultiThreaded()
        {
            return iIsOMXComponentMultiThreaded;
        };

        // From PvmiCapabilityAndConfig
        OSCL_IMPORT_REF void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements);
        OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext = NULL);
        OSCL_IMPORT_REF uint32 getCapabilityMetric(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        OSCL_IMPORT_REF virtual bool ProcessIncomingMsg(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF void Run();
    protected:
        OSCL_IMPORT_REF void CommandComplete(PVMFOMXBaseDecNodeCmdQ& aCmdQ, PVMFOMXBaseDecNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData = NULL);

        virtual void DoQueryUuid(PVMFOMXBaseDecNodeCommand&) = 0;
        void DoQueryInterface(PVMFOMXBaseDecNodeCommand&);
        virtual void DoRequestPort(PVMFOMXBaseDecNodeCommand&) = 0;
        virtual void DoReleasePort(PVMFOMXBaseDecNodeCommand&) = 0;
        void DoInit(PVMFOMXBaseDecNodeCommand&);
        void DoPrepare(PVMFOMXBaseDecNodeCommand&);
        void DoStart(PVMFOMXBaseDecNodeCommand&);
        void DoStop(PVMFOMXBaseDecNodeCommand&);
        void DoPause(PVMFOMXBaseDecNodeCommand&);
        void DoReset(PVMFOMXBaseDecNodeCommand&);
        void DoFlush(PVMFOMXBaseDecNodeCommand&);
        virtual PVMFStatus DoGetNodeMetadataKey(PVMFOMXBaseDecNodeCommand&) = 0;
        virtual PVMFStatus DoGetNodeMetadataValue(PVMFOMXBaseDecNodeCommand&) = 0;
        void DoCancelAllCommands(PVMFOMXBaseDecNodeCommand&);
        void DoCancelCommand(PVMFOMXBaseDecNodeCommand&);

        bool ProcessCommand(PVMFOMXBaseDecNodeCommand& aCmd);
        bool ProcessOutgoingMsg(PVMFPortInterface* aPort);
        PVMFStatus HandleProcessingState();
        virtual PVMFStatus HandlePortReEnable() = 0;

        virtual bool InitDecoder(PVMFSharedMediaDataPtr&) = 0;

        OSCL_IMPORT_REF OsclAny* AllocateKVPKeyArray(int32& aLeaveCode, PvmiKvpValueType aValueType, int32 aNumElements);
        int32 PushKVPKey(OSCL_HeapString<OsclMemAllocator>& aString, PVMFMetadataList* aKeyList)
        {
            int32 leavecode = OsclErrNone;
            OSCL_TRY(leavecode, aKeyList->push_back(aString));
            return leavecode;
        }

        int32 PushKVPKey(const char* aString, PVMFMetadataList& aKeyList)
        {
            int32 leavecode = OsclErrNone;
            OSCL_TRY(leavecode, aKeyList.push_back(aString));
            return leavecode;
        }

        int32 PushKVP(PvmiKvp aKVP, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList)
        {
            int32 leavecode = OsclErrNone;
            OSCL_TRY(leavecode, aValueList.push_back(aKVP));
            return leavecode;
        }

        int32 AllocateChunkFromMemPool(OsclAny*& aCtrlStructPtr, OsclMemPoolFixedChunkAllocator *aMemPool, int32 aAllocSize)
        {
            int32 leavecode = OsclErrNone;
            OSCL_TRY(leavecode, aCtrlStructPtr = (OsclAny *) aMemPool->allocate(aAllocSize));
            return leavecode;
        }

        // DV:
        virtual bool NegotiateComponentParameters(OMX_PTR aOutputParameters) = 0;

        bool SetDefaultCapabilityFlags();
        OSCL_IMPORT_REF bool CreateOutMemPool(uint32 num);
        OSCL_IMPORT_REF bool CreateInputMemPool(uint32 num);
        OSCL_IMPORT_REF bool ProvideBuffersToComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
                uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                uint32 aNumBuffers,    // number of buffers
                uint32 aActualBufferSize, // aactual buffer size
                uint32 aPortIndex,      // port idx
                bool aUseBufferOK,	// can component use OMX_UseBuffer?
                bool	aIsThisInputBuffer // is this input or output
                                                      );

        bool FreeBuffersFromComponent(OsclMemPoolFixedChunkAllocator *aMemPool, // allocator
                                      uint32 aAllocSize,	 // size to allocate from pool (hdr only or hdr+ buffer)
                                      uint32 aNumBuffers,    // number of buffers
                                      uint32 aPortIndex,      // port idx
                                      bool	aIsThisInputBuffer		// is this input or output
                                     );

        OsclSharedPtr<class PVMFMediaDataImpl> WrapOutputBuffer(uint8 *pData, uint32 aDataLen, OsclAny *pContext);
        virtual bool QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen) = 0;

        bool SendOutputBufferToOMXComponent();
        OSCL_IMPORT_REF bool SendInputBufferToOMXComponent();

        OSCL_IMPORT_REF bool SendConfigBufferToOMXComponent(uint8 *initbuffer, uint32 initbufsize);
        bool SendEOSBufferToOMXComponent();

        bool HandleRepositioning(void);
        bool SendBeginOfMediaStreamCommand(void);
        bool SendEndOfTrackCommand(void);

        bool AppendExtraDataToBuffer(InputBufCtrlStruct* aInputBuffer, OMX_EXTRADATATYPE aType, uint8* aExtraData, uint8 aDataLength);

        virtual bool ReleaseAllPorts() = 0;
        bool DeleteOMXBaseDecoder();

        OSCL_IMPORT_REF void ChangeNodeState(TPVMFNodeInterfaceState aNewState);

        OSCL_IMPORT_REF void HandleComponentStateChange(OMX_U32 decoder_state);

        // Capability And Config Helper Methods
        OSCL_IMPORT_REF virtual PVMFStatus DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF virtual PVMFStatus DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements);
        OSCL_IMPORT_REF virtual void DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        OSCL_IMPORT_REF virtual PVMFStatus DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements);

        // From OsclMemPoolFixedChunkAllocatorObserver
        OSCL_IMPORT_REF void freechunkavailable(OsclAny*);

        OSCL_IMPORT_REF PVMFCommandId QueueCommandL(PVMFOMXBaseDecNodeCommand& aCmd);

        friend class PVMFOMXBaseDecPort;

        // Ports pointers
        PVMFPortInterface* iInPort;
        PVMFPortInterface* iOutPort;

        // Commands
        PVMFOMXBaseDecNodeCmdQ iInputCommands;
        PVMFOMXBaseDecNodeCmdQ iCurrentCommand;

        // Shared pointer for Media Msg.Input buffer
        PVMFSharedMediaDataPtr iDataIn;

        // OUTPUT BUFFER RELATED MEMBERS
        // Output buffer memory pool
        OsclMemPoolFixedChunkAllocator *iOutBufMemoryPool;

        // Memory pool for simple media data
        OsclMemPoolFixedChunkAllocator *iMediaDataMemPool;

        // Fragment pool for format specific info
        PVMFBufferPoolAllocator iFsiFragmentAlloc;
        // Fragment pool for private data format specific info
        PVMFBufferPoolAllocator iPrivateDataFsiFragmentAlloc;

        // Size of output buffer (negotiated with component)
        uint32 iOMXComponentOutputBufferSize;

        // size of output to allocate (OMX_ALLOCATE_BUFFER =  size of buf header )
        // (OMX_USE_BUFFER = size of buf header + iOMXCoponentOutputBufferSize)
        uint32 iOutputAllocSize;

        // Number of output buffers (negotiated with component)
        uint32 iNumOutputBuffers;

        // Number of output buffers in possession of the component or downstream,
        // namely, number of unavailable buffers
        uint32 iNumOutstandingOutputBuffers;

        // flag to prevent sending output buffers downstream during flushing etc.
        bool iDoNotSendOutputBuffersDownstreamFlag;

        // flag to prevent freeing the buffers twice
        bool iOutputBuffersFreed;


        OsclAny *ipPrivateData;

        // INPUT BUFFER RELATED MEMBERS
        OsclMemPoolFixedChunkAllocator *iInBufMemoryPool;
        uint32 iOMXComponentInputBufferSize; // size of input buffer that the component sees (negotiated with the component)
        uint32 iInputAllocSize; 	// size of input buffer to allocate (OMX_ALLOCATE_BUFFER =  size of buf header )
        // (OMX_USE_BUFFER = size of buf header + iOMXCoponentInputBufferSize)
        uint32 iNumInputBuffers; // total num of input buffers (negotiated with component)

        uint32 iNumOutstandingInputBuffers; // number of input buffers in use (i.e. unavailable)

        bool iDoNotSaveInputBuffersFlag;

        // flag to prevent freeing buffers twice
        bool iInputBuffersFreed;

        // input buffer fragmentation etc.
        uint32 iCopyPosition;				// for copying memfrag data into a buffer
        uint32 iFragmentSizeRemainingToCopy;
        bool	iIsNewDataFragment;
        // partial frame assembly logic flags
        bool	iFirstPieceOfPartialFrame;
        bool	iObtainNewInputBuffer;
        bool	iKeepDroppingMsgsUntilMarkerBit;
        bool	iFirstDataMsgAfterBOS;
        InputBufCtrlStruct *iInputBufferUnderConstruction;
        bool	iIncompleteFrame;

        OSCL_IMPORT_REF void DropCurrentBufferUnderConstruction();
        OSCL_IMPORT_REF void SendIncompleteBufferUnderConstruction();

        // input data info
        uint32 iCurrFragNum;
        uint32 iCodecSeqNum;	// sequence number tracking
        uint32 iInPacketSeqNum;

        uint32 iInTimestamp;
        uint32 iInDuration;
        uint32 iInNumFrags;
        uint32 iCurrentMsgMarkerBit;

        // DYNAMIC PORT RE-CONFIGURATION
        uint32 iInputPortIndex;
        uint32 iOutputPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE iParamPort;
        uint32 iPortIndexForDynamicReconfig;
        bool iSecondPortReportedChange;
        bool iDynamicReconfigInProgress;
        uint32 iSecondPortToReconfig;
        bool iPauseCommandWasSentToComponent;
        bool iStopCommandWasSentToComponent;

        OMX_BUFFERHEADERTYPE *iInputBufferToResendToComponent; // ptr to input buffer that is not empty, but that the OMX component returned
        // we need to resend this same buffer back to the component

        ////////////////// OMX COMPONENT CAPABILITY RELATED MEMBERS
        bool iOMXComponentSupportsExternalOutputBufferAlloc;
        bool iOMXComponentSupportsExternalInputBufferAlloc;
        bool iOMXComponentSupportsMovableInputBuffers;
        bool iSetMarkerBitForEveryFrag; // is every fragment complete frame (e.g. AVC file playback = each fragment is a NAL)
        bool iIsOMXComponentMultiThreaded;
        bool iOMXComponentSupportsPartialFrames;
        bool iOMXComponentUsesNALStartCodes;
        bool iOMXComponentUsesFullAVCFrames;
        bool iOMXComponentCanHandleIncompleteFrames;

        // State definitions for HandleProcessingState() state machine
        typedef enum
        {
            EPVMFOMXBaseDecNodeProcessingState_Idle,                  //default state after constraction/reset
            EPVMFOMXBaseDecNodeProcessingState_InitDecoder,           //initialization of H264 decoder after handle was obtained
            EPVMFOMXBaseDecNodeProcessingState_WaitForInitCompletion, // waiting for init completion
            EPVMFOMXBaseDecNodeProcessingState_ReadyToDecode,         //nornal operation state of the decoder
            EPVMFOMXBaseDecNodeProcessingState_WaitForOutputBuffer,   //wait state for avalible media output buffer
            EPVMFOMXBaseDecNodeProcessingState_WaitForOutputPort,     //wait state, output port is busy
            EPVMFOMXBaseDecNodeProcessingState_WaitForOutgoingQueue,   //wait state, outgoing queue
            EPVMFOMXBaseDecNodeProcessingState_PortReconfig,			// Dynamic Port Reconfiguration - step 1
            EPVMFOMXBaseDecNodeProcessingState_WaitForBufferReturn,		//	step 2
            EPVMFOMXBaseDecNodeProcessingState_WaitForPortDisable,		// Dynamic Port Reconfiguration - step 3
            EPVMFOMXBaseDecNodeProcessingState_PortReEnable,			// Dynamic Port Reconfiguration - step 4
            EPVMFOMXBaseDecNodeProcessingState_WaitForPortEnable,		// step 5
            EPVMFOMXBaseDecNodeProcessingState_Stopping,				// when STOP command is issued, the node has to wait for component to transition into
            // idle state. The buffers keep coming back , the node is rescheduled
            // to run. Prevent the node from sending buffers back
            EPVMFOMXBaseDecNodeProcessingState_Pausing					// when PAUSE command is issued, the node has to wait for component to transition into
            // paused state.
            // Video: This prevents the node from sending buffers back
            // Audio: The buffers may still keep coming back , the node is rescheduled
            // to run. Prevent the node from sending buffers back to component

        } PVMFOMXBaseDecNode_ProcessingState;

        // State of HandleProcessingState() state machine
        PVMFOMXBaseDecNode_ProcessingState iProcessingState;

        // Handle of OMX Component
        OMX_HANDLETYPE iOMXDecoder;

        // Current State of the component
        OMX_STATETYPE iCurrentDecoderState;

        // BOS
        bool iSendBOS;
        uint32 iStreamID;
        uint32 iBOSTimestamp;

        // repositioning related flags
        bool iIsRepositioningRequestSentToComponent;
        bool iIsRepositionDoneReceivedFromComponent;
        bool iIsOutputPortFlushed;
        bool iIsInputPortFlushed;

        //EOS control flags
        bool iIsEOSSentToComponent;
        bool iIsEOSReceivedFromComponent;

        // Send Fsi configuration flag
        bool	sendFsi;

        // Pointer to input data fragment
        uint8* iBitstreamBuffer;
        // Size of input data fragment
        int32 iBitstreamSize;

        // Output frame sequence counter
        uint32 iSeqNum;

        // Input frame sequence counter
        uint32 iSeqNum_In;

        // Added to Scheduler Flag
        bool iIsAdded;

        // Log related
        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;
        PVLogger *iRunlLogger;

        // Counter of fragment read from current Media Msg.Input buffer
        uint fragnum;
        // Number of fragments in the Media Msg.Input buffer
        uint numfrags;

        // Time stamp to be used on output buffer
        uint32 iOutTimeStamp;

        // Node configuration update
        PVMFOMXBaseDecNodeConfig iNodeConfig;

        // Capability exchange
        PVMFNodeCapability iCapability;

        // Reference counter for extension
        uint32 iExtensionRefCount;

        // Vector for KVP
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;

        // EOS flag
        bool iEndOfDataReached;
        // Time stame upon EOS
        PVMFTimestamp iEndOfDataTimestamp;

        /* Diagnostic log related */
        PVLogger* iDiagnosticsLogger;
        bool iDiagnosticsLogged;
        void LogDiagnostics();

        uint32 iFrameCounter;

        uint32 iAvgBitrateValue;
        bool iResetInProgress;
        bool iResetMsgSent;
        bool iStopInResetMsgSent;

        // AVC NAL counter and size array
        uint32 iNALCount;
        uint32 iNALSizeArray[MAX_NAL_PER_FRAME]; // NAL count shouldn't exceed 100

        OsclAny **out_ctrl_struct_ptr ;
        OsclAny **out_buff_hdr_ptr ;
        OsclAny **in_ctrl_struct_ptr ;
        OsclAny **in_buff_hdr_ptr ;

        PVInterface* ipExternalOutputBufferAllocatorInterface;
        PVMFFixedSizeBufferAlloc* ipFixedSizeBufferAlloc;
        bool iCompactFSISettingSucceeded;

};


#endif // PVMF_OMXBASEDEC_NODE_H_INCLUDED

