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
#ifndef PVMF_SM_FSP_BASE_TYPES_H_INCLUDED
#define PVMF_SM_FSP_BASE_TYPES_H_INCLUDED

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

class PVMFDataSourcePositionParams;
class PVMFTimebase;
/*
 * Structure to contain a node and all ports and extensions associated to it
 */
enum PVMFSMFSPChildNodeCmdState
{
    PVMFSMFSP_NODE_CMD_IDLE,
    PVMFSMFSP_NODE_CMD_PENDING,
    PVMFSMFSP_NODE_CMD_CANCEL_PENDING
};

typedef PVMFGenericNodeCommand <OsclMemAllocator> PVMFSMFSPBaseNodeCommandBase;
class PVMFSMFSPBaseNodeCommand : public PVMFSMFSPBaseNodeCommandBase
{
    public:
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       int32 arg1,
                       int32 arg2,
                       int32& arg3,
                       const OsclAny*aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
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

        /* Constructor and parser for SetDataSourcePosition */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFTimestamp aTargetNPT,
                       PVMFTimestamp* aActualNPT,
                       PVMFTimestamp* aActualMediaDataTS,
                       bool aSeekToSyncPoint,
                       uint32 aStreamID,
                       const OsclAny*aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aActualMediaDataTS;
            iParam4 = (OsclAny*)aSeekToSyncPoint;
            iParam5 = (OsclAny*)aStreamID;
        }

        void Parse(PVMFTimestamp& aTargetNPT,
                   PVMFTimestamp* &aActualNPT,
                   PVMFTimestamp* &aActualMediaDataTS,
                   bool& aSeekToSyncPoint,
                   uint32& aStreamID)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aSeekToSyncPoint = (iParam4 ? true : false);
            aStreamID = (uint32)iParam5;
        }

        /* Constructor and parser for SetDataSourcePosition - Playlist */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFDataSourcePositionParams* aParams,
                       const OsclAny*aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aParams;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        void Parse(PVMFDataSourcePositionParams*& aParams)
        {
            aParams = (PVMFDataSourcePositionParams*)iParam1;
        }


        /* Constructor and parser for QueryDataSourcePosition */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFTimestamp aTargetNPT,
                       PVMFTimestamp* aActualNPT,
                       bool aSeekToSyncPoint,
                       const OsclAny*aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFTimestamp aTargetNPT,
                       PVMFTimestamp* aSeekPointBeforeTargetNPT,
                       PVMFTimestamp* aSeekPointAfterTargetNPT,
                       const OsclAny*aContext,
                       bool aSeekToSyncPoint
                      )
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = aSeekPointBeforeTargetNPT;
            iParam5 = aSeekPointAfterTargetNPT;
        }

        void Parse(PVMFTimestamp& aTargetNPT,
                   PVMFTimestamp* &aActualNPT,
                   bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3 ? true : false);
        }

        void Parse(PVMFTimestamp& aTargetNPT,
                   PVMFTimestamp*& aSeekPointBeforeTargetNPT,
                   bool& aSeekToSyncPoint,
                   PVMFTimestamp*& aSeekPointAfterTargetNPT)

        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aSeekToSyncPoint = (iParam3) ? true : false;
            aSeekPointBeforeTargetNPT = (PVMFTimestamp*)iParam4;
            aSeekPointAfterTargetNPT = (PVMFTimestamp*)iParam5;
        }


        /* Constructor and parser for SetDataSourceRate */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       int32 aRate,
                       PVMFTimebase* aTimebase,
                       const OsclAny* aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aRate;
            iParam2 = (OsclAny*)aTimebase;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        };

        void Parse(int32& aRate, PVMFTimebase*& aTimebase)
        {
            aRate = (int32)iParam1;
            aTimebase = (PVMFTimebase*)iParam2;
        }

        /* Constructor and parser for GetNodeMetadataKeys */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFMetadataList& aKeyList,
                       int32 aStartingIndex,
                       int32 aMaxEntries,
                       char* aQueryKey,
                       const OsclAny* aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*)aStartingIndex;
            iParam3 = (OsclAny*)aMaxEntries;
            if (aQueryKey)
            {
                /*allocate a copy of the query key string */
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                iParam4 = str.ALLOC_AND_CONSTRUCT(aQueryKey);
            }
        }

        void Parse(PVMFMetadataList*& MetaDataListPtr,
                   uint32 &aStartingIndex,
                   int32 &aMaxEntries,
                   char*& aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = (uint32)iParam2;
            aMaxEntries = (int32)iParam3;
            aQueryKey = NULL;
            if (iParam4)
            {
                OSCL_HeapString<OsclMemAllocator>* keystring =
                    (OSCL_HeapString<OsclMemAllocator>*)iParam4;
                aQueryKey = keystring->get_str();
            }
        }

        /* Constructor and parser for GetNodeMetadataValue */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFMetadataList& aKeyList,
                       Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                       uint32 aStartIndex,
                       int32 aMaxEntries,
                       const OsclAny* aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*) & aValueList;
            iParam3 = (OsclAny*)aStartIndex;
            iParam4 = (OsclAny*)aMaxEntries;

        }

        void Parse(PVMFMetadataList* &aKeyList,
                   Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList,
                   uint32 &aStartingIndex,
                   int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = (uint32)iParam3;
            aMaxEntries = (int32)iParam4;
        }

        /* Constructor and parser for GetLicenseW */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_wString& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       const OsclAny* aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
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
                       const OsclAny*aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        };
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

        /* Constructor and parser for setParametersAsync */
        void Construct(PVMFSessionId s, int32 cmd, PvmiMIOSession aSession,
                       PvmiKvp* aParameters, int num_elements,
                       PvmiKvp*& aRet_kvp, OsclAny* aContext)
        {
            PVMFSMFSPBaseNodeCommandBase::Construct(s, cmd, aContext);
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

        virtual bool hipri()
        {
            return ((PVMF_GENERIC_NODE_CANCELALLCOMMANDS == iCmd)
                    || (PVMF_GENERIC_NODE_CANCELCOMMAND == iCmd));
        }
        /* need to overlaod the base Copy routine to copy metadata key */
        void Copy(const PVMFGenericNodeCommand<OsclMemAllocator>& aCmd);

        /* need to overlaod the base Destroy routine to cleanup metadata key */
        void Destroy();
};

class PVMFSMFSPChildNodeContainer
{
    public:
        PVMFSMFSPChildNodeContainer()
        {
            commandStartOffset = 0;
            iNode = NULL;
            iNodeTag = 0;
            iSessionId = 0;
            iNodeCmdState = PVMFSMFSP_NODE_CMD_IDLE;
            iNumRequestPortsPending = 0;
            iAutoPaused = false;
        };

        PVMFSMFSPChildNodeContainer(const PVMFSMFSPChildNodeContainer& aNodeContainer)
        {
            commandStartOffset = aNodeContainer.commandStartOffset;
            iNode = aNodeContainer.iNode;
            iNodeTag = aNodeContainer.iNodeTag;
            iSessionId = aNodeContainer.iSessionId;
            iNodeCmdState = aNodeContainer.iNodeCmdState;
            iNumRequestPortsPending = aNodeContainer.iNumRequestPortsPending;
            iAutoPaused = aNodeContainer.iAutoPaused;
            iInputPorts = aNodeContainer.iInputPorts;
            iOutputPorts = aNodeContainer.iOutputPorts;
            iFeedBackPorts = aNodeContainer.iFeedBackPorts;
            iExtensions = aNodeContainer.iExtensions;
            iExtensionUuids = aNodeContainer.iExtensionUuids;
        }

        PVMFSMFSPChildNodeContainer& operator=(const PVMFSMFSPChildNodeContainer& aNodeContainer)
        {
            if (&aNodeContainer != this)
            {
                commandStartOffset = aNodeContainer.commandStartOffset;
                iNode = aNodeContainer.iNode;
                iNodeTag = aNodeContainer.iNodeTag;
                iSessionId = aNodeContainer.iSessionId;
                iNodeCmdState = aNodeContainer.iNodeCmdState;
                iNumRequestPortsPending = aNodeContainer.iNumRequestPortsPending;
                iAutoPaused = aNodeContainer.iAutoPaused;
                iInputPorts = aNodeContainer.iInputPorts;
                iOutputPorts = aNodeContainer.iOutputPorts;
                iFeedBackPorts = aNodeContainer.iFeedBackPorts;
                iExtensions = aNodeContainer.iExtensions;
                iExtensionUuids = aNodeContainer.iExtensionUuids;
            }
            return *this;
        }

        virtual ~PVMFSMFSPChildNodeContainer() {};

        void Reset()
        {
            commandStartOffset = 0;
            iNode = NULL;
            iNodeTag = 0;
            iSessionId = 0;
            iNodeCmdState = PVMFSMFSP_NODE_CMD_IDLE;
            iNumRequestPortsPending = 0;
            iAutoPaused = false;
            iInputPorts.clear();
            iOutputPorts.clear();
            iFeedBackPorts.clear();
            for (uint32 i = 0; i < iExtensions.size(); i++)
            {
                iExtensions[i]->removeRef();
            }
            iExtensions.clear();
            iExtensionUuids.clear();
        }


        int32				commandStartOffset;
        PVMFNodeInterface*	iNode;
        int32				iNodeTag;
        PVMFSessionId		iSessionId;
        PVMFSMFSPChildNodeCmdState  iNodeCmdState;
        int32				iNumRequestPortsPending;
        bool				iAutoPaused;
        Oscl_Vector<PVMFPortInterface*, OsclMemAllocator> iInputPorts;
        Oscl_Vector<PVMFPortInterface*, OsclMemAllocator> iOutputPorts;
        Oscl_Vector<PVMFPortInterface*, OsclMemAllocator> iFeedBackPorts;
        Oscl_Vector<PVInterface*, OsclMemAllocator>		  iExtensions;
        Oscl_Vector<PVUuid, OsclMemAllocator>			  iExtensionUuids;

};

typedef Oscl_Vector<PVMFSMFSPChildNodeContainer, OsclMemAllocator> PVMFSMFSPChildNodeContainerVector;

typedef PVMFNodeCommandQueue<PVMFSMFSPBaseNodeCommand, OsclMemAllocator> PVMFFSPNodeCmdQ;

typedef struct tagPVMFSMFSPPortContext
{
    tagPVMFSMFSPPortContext()
    {
        trackID = 0;
        portTag = 0;
    }
    uint32 trackID;
    uint32 portTag;
} PVMFSMFSPPortContext;

class PVMFSMFSPCommandContext
{
    public:
        PVMFSMFSPCommandContext()
        {
            oInternalCmd = false;
            parentCmd = 0;
            cmd = 0;
            oFree = true;
        };

        bool oInternalCmd;
        int32 parentCmd;
        int32 cmd;
        bool  oFree;
        PVMFSMFSPPortContext portContext;
};

class PVMFSMFSPSessionSourceInfo
{
    public:
        PVMFSMFSPSessionSourceInfo()
        {
            Reset();
        }

        void Reset()
        {
            iDRMProtected = false;
            _sessionType = PVMF_MIME_FORMAT_UNKNOWN;
            _sourceData = NULL;
            _sessionURL = NULL;
        }

        bool iDRMProtected;
        PVMFFormatType _sessionType;
        OsclAny*    _sourceData;
        OSCL_wHeapString<OsclMemAllocator>    _sessionURL;

};

/*
*  class for saving kvps for CPM
*/
typedef Oscl_Vector<PvmiKvp, OsclMemAllocator> PVMFKvpVector;
class PVMFSMNodeKVPStore
{
    public:
        // add kvp string with W-string value
        PVMFStatus addKVPString(const char* aKeyTypeString, OSCL_wString& aValString);
        // add kvp string with normal string value
        PVMFStatus addKVPString(const char* aKeyTypeString, const char* aValString);
        // add kvp string with normal uint32 value
        PVMFStatus addKVPuint32Value(const char* aKeyTypeString, uint32 aValue);

        // get the vector for the all constructed KVPs
        PVMFKvpVector* getKVPStore()
        {
            return &iKvpVector;
        }

        // check emptiness of the store
        bool isEmpty() const
        {
            return iKvpVector.empty();
        }

        // release memory for all the constructed KVPs and clear the vector
        void destroy()
        {
            releaseMemory();
            clear();
        }

        // constructor and destructor
        PVMFSMNodeKVPStore()
        {
            clear();
        }
        ~PVMFSMNodeKVPStore()
        {
            destroy();
        }

    private:
        enum KVPValueTypeForMemoryRelease
        {
            KVPValueTypeForMemoryRelease_NoInterest = 0,
            KVPValueTypeForMemoryRelease_String,
            KVPValueTypeForMemoryRelease_WString
        };

        void clear()
        {
            iKvpVector.clear();
            iKVPValueTypeForMemoryRelease.clear();
        }
        void releaseMemory();
        PVMFStatus pushKVPToVector(const PvmiKvp&, const KVPValueTypeForMemoryRelease&);

    private:
        PVMFKvpVector iKvpVector;
        Oscl_Vector<uint32, OsclMemAllocator> iKVPValueTypeForMemoryRelease;
};

template <class T> class PVMFSMSharedPtrAlloc: public OsclDestructDealloc
{
    public:
        T* allocate()
        {
            OsclMemAllocator alloc;
            T* ptr = OSCL_PLACEMENT_NEW(alloc.allocate(sizeof(T)), T());
            return ptr;
        }

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            T* tptr ;
            tptr = reinterpret_cast<T*>(ptr);
            tptr->~T();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};
#endif
