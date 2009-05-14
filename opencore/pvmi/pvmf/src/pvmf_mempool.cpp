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
#include "pvmf_mempool.h"

#include "pvmf_media_msg_format_ids.h"


#define MPLOGDATAPATH(x)	PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDatapathLogger, PVLOGMSG_DEBUG, x);
#define MPLOGDATAPATH_ERR(x)	PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDatapathLogger, PVLOGMSG_ERROR, x);


OSCL_EXPORT_REF PVMFMemPoolFixedChunkAllocator::PVMFMemPoolFixedChunkAllocator(const char* name, const uint32 numchunk, const uint32 chunksize, Oscl_DefAlloc* gen_alloc)
        : OsclMemPoolFixedChunkAllocator(numchunk, chunksize, gen_alloc)
{
    if (name)
    {
        iName = name;
    }
    iDatapathLogger = PVLogger::GetLoggerObject("datapath");
    iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
}


OSCL_EXPORT_REF PVMFMemPoolFixedChunkAllocator::~PVMFMemPoolFixedChunkAllocator()
{
#if OSCL_MEM_CHECK_ALL_MEMPOOL_CHUNKS_ARE_RETURNED
    if (iFreeMemChunkList.size() < iNumChunk)
    {
        // All of the chunks were not returned-- usage error
        MPLOGDATAPATH_ERR((0, ("MEMP %s Error: %d Free Chunks Not Returned!"
                               , iName.get_cstr(), iNumChunk - iFreeMemChunkList.size())));

        //OsclError::Panic("PVMFMEMPOOL",1);
    }
#endif

    iDatapathLogger = NULL;

    MPLOGDATAPATH((0, "MEMP %s Destroyed", iName.get_cstr()));
}


OSCL_EXPORT_REF OsclAny* PVMFMemPoolFixedChunkAllocator::allocate(const uint32 n)
{
    OsclAny* freechunk = NULL;

    int32 leavecode = OsclErrNone;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, leavecode, freechunk = OsclMemPoolFixedChunkAllocator::allocate(n));
    if (OsclErrNone != leavecode)
    {
        if (OsclErrNoResources == leavecode)
        {
            // No free chunk is available
            MPLOGDATAPATH((0, "MEMP %s Allocate %d chunk failed--No Free Chunks!", iName.get_cstr(), n));
            OSCL_LEAVE(leavecode);
        }
        else
        {
            OSCL_LEAVE(leavecode);
        }
    }

    MPLOGDATAPATH((0, "MEMP %s Chunk allocated, %d/%d in use"
                   , iName.get_cstr(), iNumChunk - iFreeMemChunkList.size(), iNumChunk));

    return freechunk;
}


OSCL_EXPORT_REF void PVMFMemPoolFixedChunkAllocator::deallocate(OsclAny* p)
{
    if (iRefCount > 0)
    {
        MPLOGDATAPATH((0, "MEMP %s Chunk freed, %d/%d in use"
                       , iName.get_cstr(), iNumChunk - iFreeMemChunkList.size(), iNumChunk));
    }
    OsclMemPoolFixedChunkAllocator::deallocate(p);
}


OSCL_EXPORT_REF void PVMFMemPoolFixedChunkAllocator::LogMediaDataInfo(PVMFSharedMediaDataPtr aMediaData)
{
    OSCL_UNUSED_ARG(aMediaData);
    MPLOGDATAPATH(
        (0, "MEMP %s MediaData SeqNum %d, SId %d, TS %d", iName.get_cstr()
         , aMediaData->getSeqNum()
         , aMediaData->getStreamID()
         , aMediaData->getTimestamp()));
}


OSCL_EXPORT_REF void PVMFMemPoolFixedChunkAllocator::LogMediaCmdInfo(PVMFSharedMediaCmdPtr aMediaCmd)
{
    if (aMediaCmd->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        MPLOGDATAPATH((0, "MEMP %s MediaCmd FmtId %s, SeqNum %d, SId %d, TS %d", iName.get_cstr()
                       , "EOS", aMediaCmd->getSeqNum(), aMediaCmd->getStreamID(), aMediaCmd->getTimestamp()));
    }
    else
    {
        MPLOGDATAPATH((0, "MEMP %s MediaCmd FmtId %d, SeqNum %d, SId %d, TS %d", iName.get_cstr()
                       , aMediaCmd->getFormatID(), aMediaCmd->getSeqNum(), aMediaCmd->getStreamID(), aMediaCmd->getTimestamp()));
    }
}


void PVMFMemPoolFixedChunkAllocator::createmempool()
{
    OsclMemPoolFixedChunkAllocator::createmempool();

    MPLOGDATAPATH((0, "MEMP %s Created, Numchunk %d Size %d"
                   , iName.get_cstr(), iNumChunk, iChunkSizeMemAligned));
}
