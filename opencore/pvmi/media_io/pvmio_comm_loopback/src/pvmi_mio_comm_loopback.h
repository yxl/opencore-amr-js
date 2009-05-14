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
#ifndef PVMI_MIO_COMM_LOOPBACK_H_INCLUDED
#define PVMI_MIO_COMM_LOOPBACK_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMI_MIO_COMM_DATA_BUFFER_H_INCLUDED
#include "pvmi_mio_comm_data_buffer.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMI_MIO_COMM_LOOPBACK_FACTORY_H_INCLUDED
#include "pvmi_mio_comm_loopback_factory.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif


/**
 * Enumerated list of asychronous commands for PvmiMIOCommLoopback
 */
typedef enum
{
    CMD_QUERY_UUID,
    CMD_QUERY_INTERFACE,
    CMD_INIT,
    CMD_START,
    CMD_PAUSE,
    CMD_FLUSH,
    CMD_STOP,
    CMD_CANCEL_ALL_COMMANDS,
    CMD_CANCEL_COMMAND,
    CMD_RESET,
    DATA_EVENT,
    INVALID_CMD
} PvmiMIOCommLoopbackCmdType;


/**
 * Class containing information for a command or data event
 */
class PvmiMIOCommLoopbackCmd
{
    public:
        PvmiMIOCommLoopbackCmd()
        {
            iId = 0;
            iType = INVALID_CMD;
            iContext = NULL;
            iData1 = NULL;
        }

        PvmiMIOCommLoopbackCmd(const PvmiMIOCommLoopbackCmd& aCmd)
        {
            Copy(aCmd);
        }

        ~PvmiMIOCommLoopbackCmd() {}

        PvmiMIOCommLoopbackCmd& operator=(const PvmiMIOCommLoopbackCmd& aCmd)
        {
            Copy(aCmd);
            return (*this);
        }

        PVMFCommandId iId; /** ID assigned to this command */
        int32 iType;  /** PvmiMIOCommLoopbackCmdType value */
        OsclAny* iContext;  /** Other data associated with this command */
        OsclAny* iData1;  /** Other data associated with this command */

    private:

        void Copy(const PvmiMIOCommLoopbackCmd& aCmd)
        {
            iId = aCmd.iId;
            iType = aCmd.iType;
            iContext = aCmd.iContext;
            iData1 = aCmd.iData1;
        }
};

class PvmiMIOCommLoopbackMediaData
{
    public:
        PvmiMIOCommLoopbackMediaData()
        {
            iId = 0;
            iData = NULL;
        }

        PvmiMIOCommLoopbackMediaData(const PvmiMIOCommLoopbackMediaData& aData)
        {
            iId = aData.iId;
            iData = aData.iData;
        }

        PVMFCommandId iId;
        OsclAny* iData;
};


class PvmiMIOCommLoopback : public OsclTimerObject,
            public PvmiMIOControl,
            public PvmiMediaTransfer,
            public PvmiCapabilityAndConfig
{
    public:
        PvmiMIOCommLoopback(const PvmiMIOCommLoopbackSettings& aSettings);
        virtual ~PvmiMIOCommLoopback();

        // Pure virtuals from PvmiMIOControl
        OSCL_IMPORT_REF PVMFStatus connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus disconnect(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PvmiMediaTransfer* createMediaTransfer(PvmiMIOSession& aSession,
                PvmiKvp* read_formats = NULL,
                int32 read_flags = 0,
                PvmiKvp* write_formats = NULL,
                int32 write_flags = 0);
        OSCL_IMPORT_REF void deleteMediaTransfer(PvmiMIOSession& aSession,
                PvmiMediaTransfer* media_transfer);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId DiscardData(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF void ThreadLogon();
        OSCL_IMPORT_REF void ThreadLogoff();

        // Pure virtuals from PvmiMediaTransfer
        OSCL_IMPORT_REF void setPeer(PvmiMediaTransfer* aPeer);
        OSCL_IMPORT_REF void useMemoryAllocators(OsclMemAllocator* write_alloc = NULL);
        OSCL_IMPORT_REF PVMFCommandId writeAsync(uint8 format_type, int32 format_index,
                uint8* data, uint32 data_len,
                const PvmiMediaXferHeader& data_header_info,
                OsclAny* aContext = NULL);
        OSCL_IMPORT_REF void writeComplete(PVMFStatus aStatus, PVMFCommandId aCmdId,
                                           OsclAny* aContext);
        OSCL_IMPORT_REF PVMFCommandId readAsync(uint8* data, uint32 max_data_len, OsclAny* aContext = NULL,
                                                int32* formats = NULL, uint16 num_formats = 0);
        OSCL_IMPORT_REF void readComplete(PVMFStatus aStatus, PVMFCommandId read_cmd_id,
                                          int32 format_index,
                                          const PvmiMediaXferHeader& data_header_info,
                                          OsclAny* aContext);
        OSCL_IMPORT_REF void statusUpdate(uint32 status_flags);
        OSCL_IMPORT_REF void cancelCommand(PVMFCommandId aCmdId);
        OSCL_IMPORT_REF void cancelAllCommands();

        // Pure virtuals from PvmiCapabilityAndConfig
        OSCL_IMPORT_REF void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                PvmiKvp*& aParameters, int& num_parameter_elements,
                PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements);
        OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                PvmiKvp* aParameters, int num_parameter_elements);
        OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                               int num_elements, PvmiKvp * & aRet_kvp);
        OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements, PvmiKvp*& aRet_kvp,
                OsclAny* context = NULL);
        OSCL_IMPORT_REF uint32 getCapabilityMetric(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                PvmiKvp* aParameters, int num_elements);

    private:
        void Run();
        void ConstructL(const PvmiMIOCommLoopbackSettings& aSettings);

        int32 WriteAsync(int32& cmdId,
                         OsclRefCounterMemFrag frag,
                         PvmiMediaXferHeader data_hdr);


        PVMFCommandId AddCmdToQueue(PvmiMIOCommLoopbackCmdType aType, const OsclAny* aContext, OsclAny* aData1 = NULL);
        void DoRequestCompleted(const PvmiMIOCommLoopbackCmd& aCmd, PVMFStatus aStatus, OsclAny* aEventData = NULL);
        PVMFStatus DoInit();
        PVMFStatus DoStart();
        PVMFStatus DoPause();
        PVMFStatus DoFlush();
        PVMFStatus DoReset();
        PVMFStatus DoStop();
        PVMFStatus DoRead();
        void DoLoopback();

        /**
         * Allocate a specified number of key-value pairs and set the keys
         *
         * @param aKvp Output parameter to hold the allocated key-value pairs
         * @param aKey Key for the allocated key-value pairs
         * @param aNumParams Number of key-value pairs to be allocated
         * @return Completion status
         */
        PVMFStatus AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams);

        /**
         * Verify one key-value pair parameter against capability of the port and
         * if the aSetParam flag is set, set the value of the parameter corresponding to
         * the key.
         *
         * @param aKvp Key-value pair parameter to be verified
         * @param aSetParam If true, set the value of parameter corresponding to the key.
         * @return PVMFSuccess if parameter is supported, else PVMFFailure
         */
        PVMFStatus VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam = false);

        // Command queue and command id generator
        PVMFCommandId iCommandCounter;
        Oscl_Vector<PvmiMIOCommLoopbackCmd, OsclMemBasicAllocator > iCmdQueue;

        // PvmiMIO sessions
        Oscl_Vector<PvmiMIOObserver*, OsclMemBasicAllocator > iObservers;

        PvmiMediaTransfer* iPeer;

        // Thread logon
        bool iThreadLoggedOn;

        PvmiMIOCommLoopbackSettings iSettings;

        // For generic allocations
        OsclMemAllocator iAlloc;

        // Allocator for media data buffer to store received
        // data for loopback
        PVMFSimpleMediaBufferCombinedAlloc iMediaDataAlloc;
        //PvmiMIOCommDataBufferAlloc iMediaDataAlloc;
        OsclMemPoolFixedChunkAllocator iMediaDataMemPool;

        Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemBasicAllocator > iOutgoingQueue;

        // Logger
        PVLogger* iLogger;

        // State machine
        enum PvmiMIOCommLoopbackState
        {
            STATE_IDLE,
            STATE_INITIALIZED,
            STATE_STARTED,
            STATE_FLUSHING,
            STATE_PAUSED,
            STATE_STOPPED
        };
        PvmiMIOCommLoopbackState iState;

        // sent to signal completion of asynchronous write
        class WriteResponse
        {
            public:
                WriteResponse(PVMFStatus s, PVMFCommandId id, const OsclAny* ctx, const PVMFTimestamp& ts)
                        : iStatus(s), iCmdId(id), iContext(ctx), iTimestamp(ts)
                {}

                PVMFStatus iStatus;
                PVMFCommandId iCmdId;
                const OsclAny* iContext;
                PVMFTimestamp iTimestamp;
        };
        Oscl_Vector<WriteResponse, OsclMemBasicAllocator > iWriteResponseQueue;

        // queue for data that is being sent asynchronously.  can't
        // delete it until we get writeComplete() call for it.
        class CleanupQueueElement
        {
            public:
                CleanupQueueElement(PVMFSharedMediaDataPtr d, PVMFCommandId id): iData(d), iCmdId(id) {}
                PVMFSharedMediaDataPtr iData;
                PVMFCommandId iCmdId;
        };

        enum WriteState
        {
            EWriteBusy,
            EWriteWait,
            EWriteOK
        };
        WriteState iWriteState;
        // In case any of the writeAsync calls fail
        bool iResend;
        uint32 iResendSeqNum;
        uint32 iResendFragment;
        uint32 iWriteAsyncContext;

        Oscl_Vector<CleanupQueueElement, OsclMemBasicAllocator > iCleanupQueue;
};

#endif // PVMI_MIO_LOOPBACK_REFERENCE_H_INCLUDED
