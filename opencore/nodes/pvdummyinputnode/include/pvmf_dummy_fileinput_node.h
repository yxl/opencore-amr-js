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

#ifndef PVMF_DUMMY_FILEINPUT_NODE_H_INCLUDED
#define PVMF_DUMMY_FILEINPUT_NODE_H_INCLUDED

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_FILEINPUT_PORT_H_INCLUDED
#include "pvmf_fileinput_port.h"
#endif
#ifndef PVMF_FILEINPUT_NODE_EXTENSION_H_INCLUDED
#include "pvmf_fileinput_node_extension.h"
#endif
#ifndef PVMF_FILEINPUT_NODE_EVENTS_H_INCLUDED
#include "pvmf_fileinput_node_events.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVMF_FILEINPUT_NODE_INTERNAL_H_INCLUDED
#include "pvmf_fileinput_node_internal.h"
#endif
#ifndef PVMF_FILEINPUT_SETTINGS_H_INCLUDED
#include "pvmf_fileinput_settings.h"
#endif

// SEE DESCRIPTION IN pvmf_fileinput_node.cpp
//#define FRAGMENTATION_TEST

///////////////////////////////////////////////
// Port tags
///////////////////////////////////////////////

/** Enumerated list of port tags supported by the node,
** for the port requests.
*/
typedef enum
{
    // chesterc: Convention for now is for input/sink port to have tag value 0
    // and output/src port to have tag value 1.  Both author and player engines
    // relies on this assumption, and should be removed later on when capabilities
    // exchange is implemented on the node.  The engine would then query the node
    // for the list of supported tags for a given mime type.
    PVMF_DUMMY_FILEINPUT_NODE_PORT_TYPE_SOURCE = 1
} PVMFDummyFileInputNodePortType;

///////////////////////////////////////////////
// The PVMFDummyFileInputNode Node Implementation Class
///////////////////////////////////////////////

class PVLogger;

class PVMFDummyFileInputNode : public PVMFNodeInterface,
            public OsclTimerObject,
            public PVMFFileInputNodeExtensionInterface
            , public OsclMemPoolFixedChunkAllocatorObserver
{
    public:
        OSCL_IMPORT_REF PVMFDummyFileInputNode(PVMFFileInputSettings* aSettings, int32 aPriority);
        OSCL_IMPORT_REF ~PVMFDummyFileInputNode();

        //from PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMFFileInputNodeAllocator>& aUuids,
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

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        //from PVMFDummyFileInputNodeCustomInterface1
        OSCL_IMPORT_REF PVMFStatus GetStats(PVMFPortInterface* aPort, int32& aNumMsgGenerated, int32& aNumMsgConsumed);

        // Virtual functions of PVMFFileInputNodeExtensionInterface
        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF bool GetBitstreamConfig(OsclRefCounterMemFrag& aConfig);


    private: // private member functions

        //from OsclActiveObject
        void Run();
        void DoCancel();

        //Command processing
        PVMFCommandId QueueCommandL(PVMFFileInputNodeCommand&);
        bool ProcessCommand(PVMFFileInputNodeCommand&);
        void CommandComplete(PVMFFileInputNodeCmdQ&, PVMFFileInputNodeCommand&, PVMFStatus, OsclAny* aData = NULL);
        bool FlushPending();

        //Command handlers.
        void DoReset(PVMFFileInputNodeCommand&);
        void DoQueryUuid(PVMFFileInputNodeCommand&);
        void DoQueryInterface(PVMFFileInputNodeCommand&);
        void DoRequestPort(PVMFFileInputNodeCommand&);
        void DoReleasePort(PVMFFileInputNodeCommand&);
        void DoInit(PVMFFileInputNodeCommand&);
        void DoPrepare(PVMFFileInputNodeCommand&);
        void DoStart(PVMFFileInputNodeCommand&);
        void DoStop(PVMFFileInputNodeCommand&);
        void DoFlush(PVMFFileInputNodeCommand&);
        void DoPause(PVMFFileInputNodeCommand&);
        void DoCancelAllCommands(PVMFFileInputNodeCommand&);
        void DoCancelCommand(PVMFFileInputNodeCommand&);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void SetState(TPVMFNodeInterfaceState);

        // Helper functions

        int32 LocateH263FrameHeader(uint8* video_buffer, int32 vop_size);

        int32 GetIF2FrameSize(uint8 aFrameType);

        void  ConstructInputSettings(PVMFFileInputSettings* aSettings);
        void  CloseInputFile();

        // Initialization function
        PVMFStatus Initialize();

        // Create and send out media messages
        PVMFStatus GenerateMediaMessage(PVMFSharedMediaMsgPtr& aMediaMsg);
#ifdef FRAGMENTATION_TEST
        // Create and send out media messages
        PVMFStatus GenerateMediaMessage2(PVMFSharedMediaMsgPtr& aMediaMsg, PVMFSharedMediaMsgPtr& aMediaMsg2);
#endif

        PVMFStatus SendNewMediaMessage();

    private: // private member variables

        // Vector of ports contained in this node
        PVMFPortVector<PVMFFileDummyInputPort, PVMFFileInputNodeAllocator> iPortVector;
        friend class PVMFFileDummyInputPort;

        // commands
        PVMFFileInputNodeCmdQ iInputCommands;
        PVMFFileInputNodeCmdQ iCurrentCommand;

        // Allocator
        OsclMemAllocator iCmdAlloc;
        OsclMemAllocDestructDealloc<uint8> iAlloc;

        // Allocator for simple media data buffer
        OsclMemPoolFixedChunkAllocator* iMediaBufferMemPool;
        PVMFSimpleMediaBufferCombinedAlloc* iMediaDataAlloc;
        OsclMemPoolFixedChunkAllocator iMediaDataMemPool;

        uint8* iAudioData;
        uint8* iVideoData;
        PVMFNodeCapability iCapability;
        PVLogger *iLogger;
        PVMFFileInputSettings iSettings;

        // Format specific info
        OsclRefCounterMemFrag iFormatSpecificInfo;
        int32 iFormatSpecificInfoSize;
        bool iSetFormatSpecificInfo;

        int32 iFileHeaderSize;

        // Input file
        Oscl_FileServer iFs;
        bool iFsOpen;
        Oscl_File iInputFile;
        bool iFileOpen;

        // Counter and size info for file read
        Oscl_Vector<uint32, OsclMemAllocator> iFrameSizeVector;
        Oscl_Vector<uint32, OsclMemAllocator> iMilliSecFrameDurationVector;
        int32 iDataEventCounter;
        int32 iTotalNumFrames;

        // Timing
        int32 iMicroSecondsPerTick;
        int32 iMilliSecondsPerDataEvent;
        int32 iMicroSecondsPerDataEvent;
        int32 iTSForRunIfInactive;
        PVMFTimestamp iTimeStamp;

        // Reference counter for extension
        uint32 iExtensionRefCount;


        void IncTimestamp();

        //from OsclMemPoolFixedChunkAllocatorObserver
        void freechunkavailable(OsclAny* aContextData);
        bool iWaitingOnFreeChunk;
        bool iEndOfFileReached;
};

#endif


