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

#ifndef PVMF_MP3FFPARSER_NODE_INTERNAL_H_INCLUDED
#define PVMF_MP3FFPARSER_NODE_INTERNAL_H_INCLUDED

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_MP3DEC_DEFS_H_INCLUDED
#include "pvmf_mp3ffparser_defs.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif


//memory allocator type for this node.
typedef OsclMemAllocator PVMFMP3FFParserNodeAllocator;


///////////////////////////////////////////////////////
// For Command implementation
///////////////////////////////////////////////////////

//Default vector reserve size
#define PVMF_MP3FFPARSER_NODE_COMMAND_VECTOR_RESERVE 10
//Starting value for command IDs
#define PVMF_MP3FFPARSER_NODE_COMMAND_ID_START 6000
//Node command type, IDs for all of the asynchronous node commands.
enum PVMFMP3FFParserNodeCommandType
{
    PVMP3FF_NODE_CMD_GETNODEMETADATAKEY = PVMF_GENERIC_NODE_COMMAND_LAST,
    PVMP3FF_NODE_CMD_GETNODEMETADATAVALUE,
    PVMP3FF_NODE_CMD_SETDATASOURCEPOSITION,
    PVMP3FF_NODE_CMD_QUERYDATASOURCEPOSITION,
    PVMP3FF_NODE_CMD_SETDATASOURCERATE,
    PVMP3FF_NODE_CMD_GET_LICENSE_W,
    PVMP3FF_NODE_CMD_GET_LICENSE,
    PVMP3FF_NODE_CMD_CANCEL_GET_LICENSE
};

#define PVMFMP3FFParserNodeCommandBase PVMFGenericNodeCommand<PVMFMP3FFParserNodeAllocator>
class PVMFMP3FFParserNodeCommand : public PVMFMP3FFParserNodeCommandBase
{
    public:

        // Constructor and parser for SetDataSourcePosition
        void Construct(PVMFSessionId aSessionId, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp* aActualNPT, PVMFTimestamp* aActualMediaDataTS,
                       bool aSeekToSyncPoint, uint32 aStreamID, const OsclAny*aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(aSessionId, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aActualMediaDataTS;
            iParam4 = (OsclAny*)aSeekToSyncPoint;
            iParam5 = (OsclAny*)aStreamID;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, PVMFTimestamp*& aActualMediaDataTS, bool& aSeekToSyncPoint, uint32& aStreamID)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aSeekToSyncPoint = (iParam4 ? true : false);
            aStreamID = (uint32)iParam5;
        }

        // Constructor and parser for QueryDataSourcePosition
        void Construct(PVMFSessionId aSessionId, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp* aActualNPT,	bool aSeekToSyncPoint, const OsclAny*aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(aSessionId, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3 ? true : false);
        }

        // Constructor and parser for SetDataSourceRate
        void Construct(PVMFSessionId aSessionId, int32 cmd, int32 aRate, PVMFTimebase* aTimebase, const OsclAny* aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(aSessionId, cmd, aContext);
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

        // Constructor and parser for GetNodeMetadataKeys
        void Construct(PVMFSessionId aSessionId, int32 cmd
                       , PVMFMetadataList& aKeyList
                       , uint32 aStartingIndex
                       , int32 aMaxEntries
                       , char* aQueryKey
                       , const OsclAny* aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(aSessionId, cmd, aContext);
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
        void Construct(PVMFSessionId aSessionId, int32 cmd, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartIndex, int32 aMaxEntries, const OsclAny* aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(aSessionId, cmd, aContext);
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

        /* Constructor and parser for GetLicenseW */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_wString& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFMP3FFParserNodeCommandBase::Construct(s, cmd, aContext);
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
            PVMFMP3FFParserNodeCommandBase::Construct(s, cmd, aContext);
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
                case PVMP3FF_NODE_CMD_GETNODEMETADATAKEY:
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
                case PVMP3FF_NODE_CMD_GETNODEMETADATAKEY:
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

//Command queue type
typedef PVMFNodeCommandQueue<PVMFMP3FFParserNodeCommand, PVMFMP3FFParserNodeAllocator> PVMFMP3FFParserNodeCmdQ;

#endif //PVMF_MP3FFPARSER_NODE_INTERNAL_H_INCLUDED
