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
#ifndef PVMF_SOCKET_BUFFER_ALLOCATORS_H_INCLUDED
#define PVMF_SOCKET_BUFFER_ALLOCATORS_H_INCLUDED

#ifndef PVMF_CLIENTSERVER_SOCKET_TUNEABLES_H_INCLUDED
#include "pvmf_clientserver_socket_tuneables.h"
#endif
#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#define PVMF_RESIZE_ALLOC_OVERHEAD 8
#define PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET 4

// constants
const uint PVMF_SOCKET_BUF_DEFAULT_SIZE = 200;

#define PVMF_SOCKALLOC_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SOCKALLOC_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_SOCKALLOC_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKALLOC_LOGINFO(m) PVMF_SOCKALLOC_LOGINFOLOW(m)
#define PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC(m) PVMF_SOCKALLOC_LOGINFO(m)
#define PVMF_SOCKALLOC_LOG_SEQNUM_ALLOC_DEALLOC(m) PVMF_SOCKALLOC_LOGINFO(m)
#define PVMF_SOCKALLOC_LOG_OUT_OF_ORDER_DEALLOC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iOOOLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKALLOC_LOG_MEMCALLBACK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iCallBackLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKALLOC_LOG_AVAILABILITY(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iSizeLogger,PVLOGMSG_INFO,m);

#define PVMF_SOCKALLOC_FILL_MEMBLK_ON_ALLOC 1

static const char PVMF_SOCKALLOC_FEEDBACK_PORT_NAME[] = "PVMFJitterBufferPortFeedBack";

class PVMFSMSharedBufferAllocWithReSizeAllocDeallocObserver
{
    public:
        virtual void chunkdeallocated(OsclAny* aContextData) = 0;
};

/*
 * This allocator is used to allocate socket buffers, for the purpose of
 * doing a receive.
 */
class PVMFSocketBufferAllocator : public Oscl_DefAlloc
{
    public:
        PVMFSocketBufferAllocator(uint32 numChunks, uint32 chunkSize)
                : alloc(numChunks, chunkSize)
        {
            iNumChunks = numChunks;
            iChunkSize = chunkSize;
            iNumOutStandingBuffers = 0;
            IncrementKeepAliveCount();
        };

        virtual ~PVMFSocketBufferAllocator() {};

        void IncrementKeepAliveCount()
        {
            iNumOutStandingBuffers++;
        }

        void DecrementKeepAliveCount()
        {
            iNumOutStandingBuffers--;
        }

        OsclAny* allocate(const uint32 n)
        {
            iNumOutStandingBuffers++;
            return (alloc.allocate(n));
        }

        void deallocate(OsclAny* p)
        {
            iNumOutStandingBuffers--;
            alloc.deallocate(p);
        }

        uint32 getNumOutStandingBuffers()
        {
            return iNumOutStandingBuffers;
        }

        uint32 getAvailableBufferSpace(bool aFirstParentChunkOnly = false)
        {
            OSCL_UNUSED_ARG(aFirstParentChunkOnly);
            if (iNumChunks > iNumOutStandingBuffers)
            {
                return ((iNumChunks - iNumOutStandingBuffers)*iChunkSize);
            }
            return 0;
        }
        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& aObserver,
                                      uint32 aSize,
                                      OsclAny* aContextData = NULL)
        {
            OSCL_UNUSED_ARG(aSize);
            alloc.notifyfreechunkavailable(aObserver, aContextData);
        }

        void CancelFreeChunkAvailableCallback()
        {
            alloc.CancelFreeChunkAvailableCallback();
        }

    private:
        uint32 iNumChunks;
        uint32 iChunkSize;
        OsclMemPoolFixedChunkAllocator alloc;
        uint32 iNumOutStandingBuffers;
};

class PVMFSocketBufferCleanupDA : public OsclDestructDealloc
{
    public:
        PVMFSocketBufferCleanupDA(Oscl_DefAlloc* in_gen_alloc) :


                gen_alloc(in_gen_alloc) {};

        virtual ~PVMFSocketBufferCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            gen_alloc->deallocate(ptr);
            /*
             * in case there are no outstanding buffers delete the allocator
             */
            PVMFSocketBufferAllocator* socketDataAllocator =
                reinterpret_cast<PVMFSocketBufferAllocator*>(gen_alloc);

            uint32 numBuffers = socketDataAllocator->getNumOutStandingBuffers();

            if (numBuffers == 0)
            {
                OSCL_DELETE((socketDataAllocator));
            }
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};

class PVMFSMSharedBufferAllocWithReSize : public Oscl_DefAlloc
{
    public:
        PVMFSMSharedBufferAllocWithReSize(uint32 aParentChunkSize, const char name[])
        {
            iLogger = NULL;
            iSizeLogger = NULL;
            iOOOLogger = NULL;
            iLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize");
            iSizeLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.availability");
            iCallBackLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.memcallback");
            iOOOLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.ooo");
            iName = name;

            iNumOutOfOrderDeallocs = 0;
            iLastDeallocatedSeqNum = 0;
            iSeqCount = 1;
            iNumOutStandingBuffers = 0;

            iCallbackPending = false;
            iCallbackRequestSize = 0;;
            iObserver = NULL;
            iNextAvailableContextData = NULL;

            iJJDataSize = 0;
            iJJDataDbgSize = 0;

            if ((int32)aParentChunkSize <= 0)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize - Invalid Parent Chunk size"));
                OSCL_LEAVE(OsclErrArgument);
            }

            // set buffer resizing to default values
            if (oscl_strcmp(iName.get_cstr(), PVMF_SOCKALLOC_FEEDBACK_PORT_NAME) == 0)
            {
                iiRegrowthSize = DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_FEEDBACK_PORT;
            }
            else
            {
                iiRegrowthSize = DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT;
            }
            iiMaxNumGrows = DEFAULT_MAX_NUM_SOCKETMEMPOOL_RESIZES;
            iiNumGrows = 0;

            iDeallocObserver = NULL;
            iDeallocNotificationContextData = NULL;

            CreateParentChunk(aParentChunkSize);

            IncrementKeepAliveCount();
        }

        PVMFSMSharedBufferAllocWithReSize(uint32 aParentChunkSize, const char name[],
                                          int aMaxNumGrows, int aGrowSize)
        {
            iLogger = NULL;
            iSizeLogger = NULL;
            iOOOLogger = NULL;
            iLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize");
            iSizeLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.availability");
            iCallBackLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.memcallback");
            iOOOLogger = PVLogger::GetLoggerObject("PVMFSMSharedBufferAllocWithReSize.ooo");
            iName = name;
            iCallbackPending = false;
            iNumOutOfOrderDeallocs = 0;
            iLastDeallocatedSeqNum = -1;
            iSeqCount = 0;
            iNumOutStandingBuffers = 0;

            iJJDataSize = 0;
            iJJDataDbgSize = 0;
            if ((int32)aParentChunkSize <= 0)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize - Invalid Parent Chunk size"));
                OSCL_LEAVE(OsclErrArgument);
            }

            // set buffer resizing to specified values
            iiRegrowthSize = aGrowSize;
            iiMaxNumGrows = aMaxNumGrows;
            iiNumGrows = 0;

            iDeallocObserver = NULL;
            iDeallocNotificationContextData = NULL;

            CreateParentChunk(aParentChunkSize);
            IncrementKeepAliveCount();
        }

        void CreateParentChunk(uint32 aSize)
        {
            ParentChunkContainer parentChunkContainer;

            parentChunkContainer.id = iParentChunkContainerVec.size();
            parentChunkContainer.iParentChunkSize = aSize;
            parentChunkContainer.iParentChunkStart =
                (uint8*)(alloc.ALLOCATE(aSize));
            parentChunkContainer.iParentChunkEnd =
                parentChunkContainer.iParentChunkStart + aSize;
            parentChunkContainer.iAllocationPtr =
                parentChunkContainer.iParentChunkStart;

            iParentChunkContainerVec.push_back(parentChunkContainer);
            PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::CreateParentChunk - Name=%s, Size=%d", iName.get_cstr(), aSize));
        }

        virtual ~PVMFSMSharedBufferAllocWithReSize()
        {
            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;

            for (it = iParentChunkContainerVec.begin();
                    it != iParentChunkContainerVec.end();
                    it++)
            {
                alloc.deallocate(it->iParentChunkStart);
            }

            if (iNumOutOfOrderDeallocs != 0)
            {
                OSCL_ASSERT(false);
            }

            iParentChunkContainerVec.clear();
            iLogger = NULL;
            iJJDataSize = 0;
            iJJDataDbgSize = 0;
            iNumOutStandingBuffers = 0;
            iSeqCount = 0;
            iLastDeallocatedSeqNum = -1;
            iNumOutOfOrderDeallocs = 0;
            iDeallocObserver = NULL;
        };

        /*
         * This allocator is used to create shared buffer which could have a lifetime
         * that is longer than the module that actually created the allocator. Hence
         * it is implemented in such a way that this allocator would de destroyed when
         * all the outstanding buffers allocated by this alloacator are deallocated. Or
         * in other words this allocator would be destroyed if and when iNumOutStandingBuffers
         * becomes zero. This could cause problems when due to transient nature of data flow
         * iNumOutStandingBuffers could become zero when the data flow drys up temporarily.
         * Under such a scenario we would end up deleting the allocator prematurely. Inorder
         * to avoid this, these keep alive mechanisms are provided. This ensures that the
         * module that created the allocator can make sure that the allocator is alive atleast
         * till its lifetime. Therefore allocator's life is the maximum of:
         * (buffer life times, allocator creator's life time)
         */
        void IncrementKeepAliveCount()
        {
            iDecKeepAliveCalled = false;
            iNumOutStandingBuffers++;
            PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::IncrementKeepAliveCount - Name=%s, iNumOutStandingBuffers=%d", iName.get_cstr(), iNumOutStandingBuffers));
        }

        void DecrementKeepAliveCount()
        {
            iDecKeepAliveCalled = true;
            iNumOutStandingBuffers--;
            PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::DecrementKeepAliveCount - Name=%s, iNumOutStandingBuffers=%d", iName.get_cstr(), iNumOutStandingBuffers));
        }

        struct StatBlock
        {
            StatBlock()
            {
                size = 0;
                seqNum = 0;
            };

            uint32 size;
            uint32 seqNum;
        };

        struct OutOfOrderBlockContainer
        {
            OutOfOrderBlockContainer()
            {
                ptr = NULL;
                seqNum = 0;
            };
            OutOfOrderBlockContainer(const OsclAny* p, uint32 s)
            {
                ptr = p;
                seqNum = s;
            };

            const OsclAny* ptr;
            uint32 seqNum;
        };

        class ReassemblyBlock
        {

            public:
                ReassemblyBlock(const OsclAny* ptr, uint32 seqNum, ReassemblyBlock* p = NULL) : pnext(p)
                {
                    rangeStart = rangeEnd = seqNum;
                    blocks.push_back(OutOfOrderBlockContainer(ptr, seqNum));
                }

                ~ReassemblyBlock() { }

                bool IsInRange(uint32 seq)
                {
                    return (seq >= rangeStart - 1) && (seq <= rangeEnd + 1);
                }
                bool IsInRange(ReassemblyBlock& block)
                {
                    return (block.rangeStart == rangeEnd + 1);
                }
                bool IsLess(uint32 seq)
                {
                    return seq < rangeStart;
                }
                void Merge(ReassemblyBlock& block)
                {
                    for (uint i = 0; i < block.blocks.size(); i++)
                    {
                        blocks.push_back(block.blocks[i]);
                        rangeEnd++;
                    }

                    rangeEnd = block.rangeEnd;
                }
                bool Insert(const OsclAny* ptr, uint32 seqNum)
                {
                    if (seqNum == rangeEnd + 1)
                    {
                        blocks.push_back(OutOfOrderBlockContainer(ptr, seqNum));
                        rangeEnd++;
                        return true;
                    }
                    else if (seqNum == rangeStart - 1)
                    {
                        blocks.push_front(OutOfOrderBlockContainer(ptr, seqNum));
                        rangeStart--;
                        return true;
                    }
                    else
                    {
                        //OSCL_ASSERT(false);
                        return false;
                    }
                }

                Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* Blocks()
                {
                    return &blocks;
                }
                ReassemblyBlock* pnext;

            private:
                uint32 rangeStart;
                uint32 rangeEnd;
                Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator> blocks;
        };

        class ReassemblyBlockList
        {
            public:
                ReassemblyBlockList()
                {
                    phead = NULL;
                }
                ~ReassemblyBlockList() { }

                bool Insert(const OsclAny* ptr, uint32 seq)
                {
                    bool oRet = true;
                    ReassemblyBlock** pp = &phead;
                    while (1)
                    {
                        if (*pp == NULL)
                        {
                            *pp = OSCL_NEW(ReassemblyBlock, (ptr, seq));
                            break;
                        }
                        else if ((*pp)->IsInRange(seq))
                        {
                            oRet = (*pp)->Insert(ptr, seq);
                            break;
                        }
                        else if ((*pp)->IsLess(seq))
                        {
                            ReassemblyBlock* p = *pp;
                            *pp = OSCL_NEW(ReassemblyBlock, (ptr, seq, p));
                            break;
                        }
                        else
                        {
                            pp = &(*pp)->pnext;
                        }
                    }
                    pp = &phead;
                    while ((*pp)->pnext != NULL)
                    {
                        if ((*pp)->IsInRange(*((*pp)->pnext)))
                        {
                            (*pp)->Merge(*((*pp)->pnext));
                            ReassemblyBlock* p = (*pp)->pnext;
                            (*pp)->pnext = (*pp)->pnext->pnext;
                            OSCL_DELETE(p);
                        }
                        else
                        {
                            pp = &(*pp)->pnext;
                        }
                    }
                    return oRet;
                }

                Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* GetDeletables(uint32 seqNum)
                {
                    if (phead == NULL)
                    {
                        return NULL;
                    }
                    else if (phead->IsInRange(seqNum))
                    {
                        return phead->Blocks();
                    }
                    else
                    {
                        return NULL;
                    }
                }
                void Prune()
                {
                    OSCL_ASSERT(phead != NULL);
                    ReassemblyBlock* p = phead;
                    phead = p->pnext;
                    OSCL_DELETE(p);
                }
/////// START: clean the head
                Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* GetHeadDeletables(uint32 headSeqNum)
                {
                    ReassemblyBlock *my_ptr = phead;

                    while (my_ptr != NULL)
                    {
                        if (my_ptr->IsInRange(headSeqNum))
                        { //ok, we got a consecutive chunk end with headSeqNum-1
                            return my_ptr->Blocks();
                        }

                        my_ptr = my_ptr->pnext;
                    }
                    return NULL;
                }
                void PruneHead(Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* ptrBlk)
                {
                    OSCL_ASSERT(phead != NULL);
                    if (phead->Blocks() == ptrBlk)
                    {
                        OSCL_DELETE(phead);
                        phead = NULL;
                        return;
                    }

                    ReassemblyBlock *myParent_ptr = phead;
                    ReassemblyBlock *my_ptr = myParent_ptr->pnext;

                    while (my_ptr != NULL)
                    {
                        if (my_ptr->Blocks() == ptrBlk)
                        {
                            OSCL_DELETE(my_ptr);
                            myParent_ptr->pnext = NULL;
                            return;
                        }

                        myParent_ptr = my_ptr;
                        my_ptr = my_ptr->pnext;
                    }
                    OSCL_ASSERT(false);
                }
/////// END: clean the head
            private:
                ReassemblyBlock* phead;
        };


        struct ParentChunkContainer
        {
            ParentChunkContainer()
            {
                id = 0xFFFFFFFF;
                iParentChunkStart = NULL;
                iParentChunkEnd = NULL;
                iAllocationPtr = NULL;
                iEndOfLastDeallocatedBlock = NULL;
                iParentChunkSize = 0;
            };
            uint32 id;
            uint8* iParentChunkStart;
            uint8* iParentChunkEnd;
            uint32 iParentChunkSize;
            uint8* iAllocationPtr;
            uint8* iEndOfLastDeallocatedBlock;
        };

        enum PVMF_RESIZE_ALLOC_ERROR_CODE
        {
            PVMF_RESIZE_ALLOC_SUCCESS,
            PVMF_RESIZE_ALLOC_NO_MEMORY,
            PVMF_RESIZE_ALLOC_MEMORY_CORRUPT,
            PVMF_RESIZE_OUT_OF_ORDER_BLOCK
        };

        PVMF_RESIZE_ALLOC_ERROR_CODE
        AllocateInParentChunk(ParentChunkContainer* aPCContainer,
                              OsclAny*& aRequestPtr,
                              const uint32 aReqBlkSize)
        {
            uint32 n = aReqBlkSize + PVMF_RESIZE_ALLOC_OVERHEAD;
            uint32 availSize = FindLargestContiguousFreeBlock(aPCContainer);
            if (n > availSize)
            {
                PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::AllocateInParentChunk - No Memory1 Name=%s, ChunkID=%d, ReqSize=%d, AvailSize=%d", iName.get_cstr(), aPCContainer->id, n, availSize));
                return (PVMF_RESIZE_ALLOC_NO_MEMORY);
            }
            /*
             * Check and see if the block being allocated would
             * cause a wrap around of the "iAllocationPtr"
             */
            if ((aPCContainer->iAllocationPtr + n) > aPCContainer->iParentChunkEnd)
            {
                /*
                 * This implies that the requested block is larger
                 * than whats left towards the end of the parent chunk.
                 * Since we cannot provide memory in fragments, we
                 * have to ignore this chunk, wrap the allocation ptr
                 * to start of the parent chunk and check for alloc size.
                 */
                aPCContainer->iAllocationPtr = aPCContainer->iParentChunkStart;
            }
            /*
             * Next check if we can accomodate this request in one
             * contiguous block between the current alloc ptr and
             * last deallocated ptr. Please note that we make use
             * of the fact that deallocation happens sequentially.
             * Or in other words blocks are freed in the order
             * in which they are allocated
             *
             * There are two possibilites:
             * 1) iAllocationPtr < iEndOfLastDeallocatedBlock - This means
             * that a wrap around has happenned, and we need to check
             * if there is a large enough block between the two ptrs.
             * Please note that we assume that the space between
             * iAllocationPtr and iEndOfLastDeallocatedBlock is all free.
             * Reason being that blocks are deallocated in the same
             * order in which they are allocated.
             *
             * 2) iAllocationPtr > iEndOfLastDeallocatedBlock - This means
             * we are ok. Any discrepancies must have been detected
             * by earlier checks.
             */
            if (aPCContainer->iAllocationPtr < aPCContainer->iEndOfLastDeallocatedBlock)
            {
                uint32 diff = (aPCContainer->iEndOfLastDeallocatedBlock -
                               aPCContainer->iAllocationPtr);
                if (diff >= n)
                {
                    iNumOutStandingBuffers++;
                }
                else
                {
                    /*
                     * We do not have a large enough block to
                     * satisfy this request.
                     */
                    PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::AllocateInParentChunk - No Memory2 Name=%s, ChunkID=%d, ReqSize=%d, AvailSize=%d", iName.get_cstr(), aPCContainer->id, n, diff));
                    return (PVMF_RESIZE_ALLOC_NO_MEMORY);
                }
            }
            else
            {
                iNumOutStandingBuffers++;
            }
            /* Set sequence number */
            oscl_memcpy(aPCContainer->iAllocationPtr, &iSeqCount, sizeof(uint32));
            /* Set size */
            oscl_memcpy((aPCContainer->iAllocationPtr + PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET),
                        &aReqBlkSize,
                        sizeof(uint32));
            aRequestPtr = (OsclAny*)(aPCContainer->iAllocationPtr +
                                     PVMF_RESIZE_ALLOC_OVERHEAD);

#if PVMF_SOCKALLOC_FILL_MEMBLK_ON_ALLOC
            uint32 value = 0x00;
            oscl_memset(aRequestPtr, value, aReqBlkSize);
#endif
            // need to remove begin and end variables because of compiler warnings
            PVMF_SOCKALLOC_LOG_SEQNUM_ALLOC_DEALLOC((0, "PVMFSMSharedBufferAllocWithReSize::allocate - Alloc SeqNum=%d", iSeqCount));

            PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC((0, "PVMFSMReSize::allocate - "
                                                    "SeqNum=%d, PtrS=0x%x, PtrE=0x%x, Begin=%d, End=%d, AllocSize=%d, LastDeallocPtr=0x%x, Avail=%d",
                                                    iSeqCount,
                                                    aPCContainer->iAllocationPtr,
                                                    aPCContainer->iAllocationPtr + n,
                                                    (aPCContainer->iAllocationPtr - aPCContainer->iParentChunkStart),
                                                    (aPCContainer->iAllocationPtr - aPCContainer->iParentChunkStart) + n,
                                                    n,
                                                    aPCContainer->iEndOfLastDeallocatedBlock,
                                                    getTotalAvailableBufferSpace()));
            aPCContainer->iAllocationPtr += n;

            PVMF_SOCKALLOC_LOG_AVAILABILITY((0, "PVMFSMSBAWithReSize::allocate - Name=%s, SN=%d, Alloc=%d, Avail=%d, BufsOS=%d",
                                             iName.get_cstr(), iSeqCount, n, getTotalAvailableBufferSpace(), iNumOutStandingBuffers));

            iSeqCount++;

            iJJDataSize += aReqBlkSize;
            iJJDataDbgSize += aReqBlkSize;

            return (PVMF_RESIZE_ALLOC_SUCCESS);
        }

        OsclAny* allocate(const uint32 n)
        {
            OsclAny* requestPtr = NULL;
            PVMF_RESIZE_ALLOC_ERROR_CODE errCode;

            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;

            for (it = iParentChunkContainerVec.begin();
                    it != iParentChunkContainerVec.end();
                    it++)
            {
                errCode = AllocateInParentChunk(it, requestPtr, n);

                if (errCode == PVMF_RESIZE_ALLOC_MEMORY_CORRUPT)
                {
                    PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::allocate - MEMORY CHUNKS CORRUPT!!! Name=%s", iName.get_cstr()));
                    OSCL_LEAVE(OsclErrCorrupt);
                }
                else if (errCode == PVMF_RESIZE_ALLOC_SUCCESS)
                {
                    return requestPtr;
                }
            }
            /*
             * This implies that we cannot accomodate the request in any
             * of the existing chunks. Allocate a new parent chunk.
             */
            if (iiNumGrows == iiMaxNumGrows)
            {
                //PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::allocate - Unable to add an another buffer. Limit exceeded.", iName.get_cstr()));
                OSCL_LEAVE(OsclErrNoMemory);  // not graceful - can I return NULL instead? TBD.
            }
            iiNumGrows++;
            CreateParentChunk(iiRegrowthSize);

            PVMF_SOCKALLOC_LOGINFO((0, "PVMFSMSharedBufferAllocWithReSize::allocate - EXPANDING JITTER BUFFER - Name=%s, Size=%d", iName.get_cstr(), iiRegrowthSize));

            ParentChunkContainer newPCContainer = iParentChunkContainerVec.back();

            errCode = AllocateInParentChunk(&newPCContainer, requestPtr, n);
            iParentChunkContainerVec[iParentChunkContainerVec.size()-1] = newPCContainer;

            if (errCode == PVMF_RESIZE_ALLOC_MEMORY_CORRUPT)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::allocate - MEMORY CHUNKS CORRUPT!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrCorrupt);
            }
            else if (errCode == PVMF_RESIZE_ALLOC_NO_MEMORY)
            {
                /* Too big of a request */
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::allocate - TOO BIG A REQUEST Name=%s, ReqSize=%d", iName.get_cstr(), n));
                OSCL_LEAVE(OsclErrNoMemory);
            }
            return requestPtr;
        }

        ParentChunkContainer* FindParentChunk(const OsclAny* p)
        {
            const uint8* deallocPtr = reinterpret_cast<const uint8*>(p);

            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;

            for (it = iParentChunkContainerVec.begin();
                    it != iParentChunkContainerVec.end();
                    it++)
            {
                if ((deallocPtr >= it->iParentChunkStart) &&
                        (deallocPtr < it->iParentChunkEnd))
                {
                    return (it);
                }
            }
            return NULL;
        }

        PVMF_RESIZE_ALLOC_ERROR_CODE
        DeallocateFromParentChunk(ParentChunkContainer* aPCContainer,
                                  const OsclAny* aDeallocPtr,
                                  bool oEnableOutofOrderCheck = true)
        {
            uint32 seqNum = 0;
            uint32 blkSize = 0;
            uint8* p = ((uint8*)(aDeallocPtr) - PVMF_RESIZE_ALLOC_OVERHEAD);
            /* Get the corresponding block size & seqnum from the pointer itself */
            oscl_memcpy(&seqNum, (OsclAny*)p, sizeof(uint32));
            oscl_memcpy(&blkSize, (OsclAny*)(p + PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET), sizeof(uint32));
            blkSize += PVMF_RESIZE_ALLOC_OVERHEAD;
            if (oEnableOutofOrderCheck)
            {
                iJJDataSize -= (blkSize - PVMF_RESIZE_ALLOC_OVERHEAD);
                if (seqNum != (uint32)(iLastDeallocatedSeqNum + 1))
                {
                    if (seqNum + 1 == iSeqCount)
                    {//just allocated, now free, so we pretend we didn't allocate this one to increase mem efficiency
                        if (aPCContainer->iAllocationPtr != p + blkSize)
                        {
                            PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC((0, "SM ATTN seqNum %d blkSize %d iAlloc 0x%x p 0x%x Ln %d"
                                                                    , seqNum, blkSize, aPCContainer->iAllocationPtr, p, __LINE__));
                            PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC((0, "SM ATTN start 0x%x end 0x%x lastD 0x%x Ln %d"
                                                                    , aPCContainer->iParentChunkStart, aPCContainer->iParentChunkEnd, aPCContainer->iEndOfLastDeallocatedBlock, __LINE__));
                        }

                        iJJDataDbgSize -= (blkSize - PVMF_RESIZE_ALLOC_OVERHEAD);

                        aPCContainer->iAllocationPtr = p;

                        iNumOutStandingBuffers--;
                        iSeqCount--;
                        return (PVMF_RESIZE_ALLOC_SUCCESS);
                    }

                    bool oRet = iOutOfOrderBlocks.Insert(aDeallocPtr, seqNum);
                    if (oRet == true)
                    {
                        iNumOutOfOrderDeallocs++;
                    }
                    PVMF_SOCKALLOC_LOG_OUT_OF_ORDER_DEALLOC((0, "PVMFSMSharedBufferAllocWithReSize::DeallocateFromParentChunk - OOO Dealloc SeqNum=%d, Size=%d, Ptr=0x%x, NumOOODeallocs=%d", seqNum, blkSize, p, iNumOutOfOrderDeallocs));
                    return (PVMF_RESIZE_OUT_OF_ORDER_BLOCK);
                }

            }

            iJJDataDbgSize -= (blkSize - PVMF_RESIZE_ALLOC_OVERHEAD);

            iLastDeallocatedSeqNum = (int32)(seqNum);
            aPCContainer->iEndOfLastDeallocatedBlock = ((uint8*)p + blkSize);

            if (aPCContainer->iEndOfLastDeallocatedBlock == aPCContainer->iAllocationPtr)
            {
                aPCContainer->iEndOfLastDeallocatedBlock = NULL;
                aPCContainer->iAllocationPtr = aPCContainer->iParentChunkStart;
            }
            iNumOutStandingBuffers--;

#if PVMF_SOCKALLOC_FILL_MEMBLK_ON_ALLOC
            uint32 value = 0x00;
            oscl_memset(p, value, blkSize);
#endif
            // need to remove begin and end variables because of compiler warnings
            PVMF_SOCKALLOC_LOG_SEQNUM_ALLOC_DEALLOC((0, "PVMFSMSharedBufferAllocWithReSize::DeallocateFromParentChunk - Dealloc SeqNum=%d", iLastDeallocatedSeqNum));
            PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC((0, "PVMFSMReSize::Dealloc - "
                                                    "SeqNum=%d, PtrS=0x%x, PtrE=0x%x, Begin=%d, End=%d, BlockSize=%d, LastDeallocatedPtr=0x%x, Avail=%d",
                                                    seqNum,
                                                    p,
                                                    p + blkSize,
                                                    ((uint8*)p - aPCContainer->iParentChunkStart),
                                                    ((uint8*)p - aPCContainer->iParentChunkStart) + blkSize,
                                                    blkSize,
                                                    aPCContainer->iEndOfLastDeallocatedBlock,
                                                    getTotalAvailableBufferSpace()));
            PVMF_SOCKALLOC_LOG_AVAILABILITY((0, "PVMFSMSBAWithReSize::DeallocateFromParentChunk - Name=%s, SN=%d, Dealloc=%d, Avail=%d, BufsOS=%d",
                                             iName.get_cstr(), seqNum, blkSize, getTotalAvailableBufferSpace(), iNumOutStandingBuffers));
            return (PVMF_RESIZE_ALLOC_SUCCESS);
        }

        PVMF_RESIZE_ALLOC_ERROR_CODE
        SearchAndDeallocateAnyPrevOutofOrderBlocks(uint32 aSeqNum)
        {
            Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* deleteList =
                iOutOfOrderBlocks.GetDeletables(aSeqNum);

            if (deleteList == NULL)
            {
                return (PVMF_RESIZE_ALLOC_SUCCESS);
            }

            for (uint i = 0; i < (*deleteList).size(); i++)
            {
                DeallocateOutofOrderBlock((*deleteList)[i].ptr);
                iNumOutOfOrderDeallocs--;
            }

            iOutOfOrderBlocks.Prune();
            return (PVMF_RESIZE_ALLOC_SUCCESS);
        }

        void DeallocateOutofOrderBlock(const OsclAny* p)
        {
            ParentChunkContainer* parentChunkContainer = FindParentChunk(p);

            if (parentChunkContainer == NULL)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::DeallocateOutofOrderBlock - INVALID PTR!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
            bool oEnableOutofOrderCheck = false;
            if (DeallocateFromParentChunk(parentChunkContainer, p, oEnableOutofOrderCheck) == PVMF_RESIZE_ALLOC_MEMORY_CORRUPT)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::DeallocateOutofOrderBlock - CORRUPT PTR!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
        }

        void PurgeOutofOrderBlockVec()
        {
            if (iOutofOrderBlockVec.size() > 0)
            {
                Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>::iterator it;
                uint32 count = iOutofOrderBlockVec.size();
                if (count != iNumOutOfOrderDeallocs)
                {
                    OSCL_ASSERT(false);
                }
                while (iOutofOrderBlockVec.size() > 0)
                {
                    it = iOutofOrderBlockVec.begin();
                    DeallocateOutofOrderBlock(it->ptr);
                    iOutofOrderBlockVec.erase(it);
                }
            }
        }

        virtual void deallocate(OsclAny* p)
        {
            ParentChunkContainer* parentChunkContainer = FindParentChunk(p);
            if (parentChunkContainer == NULL)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::deallocate - INVALID PTR!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
            /* Deallocate the current ptr */
            PVMF_RESIZE_ALLOC_ERROR_CODE errCode = DeallocateFromParentChunk(parentChunkContainer, p);

            if (errCode == PVMF_RESIZE_ALLOC_MEMORY_CORRUPT)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::deallocate - CORRUPT PTR - 2!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
            else if ((errCode == PVMF_RESIZE_ALLOC_SUCCESS) || (errCode == PVMF_RESIZE_OUT_OF_ORDER_BLOCK))
            {
                if (iDeallocObserver != NULL)
                {
                    iDeallocObserver->chunkdeallocated(iDeallocNotificationContextData);
                }
                /* Deallocate any out of order blocks */
                if (iNumOutOfOrderDeallocs > 0)
                {
                    SearchAndDeallocateAnyPrevOutofOrderBlocks(iLastDeallocatedSeqNum);
                }

                if (iNumOutOfOrderDeallocs > 0)
                {//delete from head, maybe?
                    Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator>* deleteList =
                        iOutOfOrderBlocks.GetHeadDeletables(iSeqCount);

                    if (NULL != deleteList)
                    {
                        for (int32 i = (*deleteList).size() - 1; i >= 0; i--)
                        {
                            uint8* p = (uint8*)((*deleteList)[i].ptr);
                            ParentChunkContainer* parentChunkContainer = FindParentChunk(p);

                            if (parentChunkContainer == NULL)
                            {
                                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::deallocate - INVALID PTR!!! Name=%s Ln %d", iName.get_cstr(), __LINE__));
                                OSCL_LEAVE(OsclErrArgument);
                            }
                            uint32 seqNum = 0;
                            uint32 blkSize = 0;
                            p -= PVMF_RESIZE_ALLOC_OVERHEAD;
                            /* Get the corresponding block size & seqnum from the pointer itself */
                            oscl_memcpy(&seqNum, (OsclAny*)p, sizeof(uint32));
                            oscl_memcpy(&blkSize, (OsclAny*)(p + PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET), sizeof(uint32));

                            if (seqNum + 1 != iSeqCount)
                            {
                                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::deallocate - ERROR seqNum %d iSeqCount %d Name=%s", seqNum, iSeqCount, iName.get_cstr()));
                                OSCL_LEAVE(OsclErrArgument);
                            }

                            if (p + blkSize + PVMF_RESIZE_ALLOC_OVERHEAD != parentChunkContainer->iAllocationPtr)
                            {//just for logging wrap-around
                                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::deallocate - p 0x%x iAllocPtr 0x%x seqNum %d iSeqCount %d Name=%s",
                                                         p, parentChunkContainer->iAllocationPtr, seqNum, iSeqCount, iName.get_cstr()));
                            }
                            parentChunkContainer->iAllocationPtr = p;

                            OSCL_ASSERT(parentChunkContainer->iAllocationPtr != parentChunkContainer->iEndOfLastDeallocatedBlock);

                            iNumOutOfOrderDeallocs--;
                            iNumOutStandingBuffers--;
                            iSeqCount--;
                            iJJDataDbgSize -= blkSize;
                        }

                        iOutOfOrderBlocks.PruneHead(deleteList);
                    }
                }
            }
            CheckAndNotifyFreeChunkAvailable();
        }

        PVMF_RESIZE_ALLOC_ERROR_CODE
        ReSizeFromParentChunk(ParentChunkContainer* aPCContainer,
                              OsclAny* aPtr,
                              uint32 aBytesToReclaim)
        {
            uint32 seqNum = 0;
            uint32 blkSize = 0;
            uint8* p = ((uint8*)(aPtr) - PVMF_RESIZE_ALLOC_OVERHEAD);
            /* Get the corresponding block size & seqnum from the pointer itself */
            oscl_memcpy(&seqNum, (OsclAny*)p, sizeof(uint32));
            oscl_memcpy(&blkSize, (OsclAny*)(p + PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET), sizeof(uint32));
            if (aBytesToReclaim > blkSize)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::ReSizeFromParentChunk - Invalid Reclaim Size - Name=%s, ChunkID=%d, SeqNum=%d, Ptr=0x%x, BlockSize=%d, ReclaimSize=%d", iName.get_cstr(), aPCContainer->id, seqNum, p, blkSize, aBytesToReclaim));
                return (PVMF_RESIZE_ALLOC_MEMORY_CORRUPT);
            }
            blkSize -= aBytesToReclaim;
            aPCContainer->iAllocationPtr -= aBytesToReclaim;

            iJJDataSize -= aBytesToReclaim;
            iJJDataDbgSize -= aBytesToReclaim;
            if (iSeqCount > seqNum + 1)
            {
                //Resize can ONLY be applied to the buffer just allocated.
                PVMF_SOCKALLOC_LOGERROR((0, "Resize ERROR seq %d iSeqCount %d iJJDataSize %d", seqNum, iSeqCount, iJJDataSize));
                return (PVMF_RESIZE_ALLOC_MEMORY_CORRUPT);
            }
            /* reset the block size */
            oscl_memcpy((OsclAny*)(p + PVMF_RESIZE_ALLOC_BLK_SIZE_OFFSET),
                        &blkSize,
                        sizeof(uint32));
            // need to remove begin and end variables because of compiler warnings
            PVMF_SOCKALLOC_LOG_ALLOC_RESIZE_DEALLOC((0, "PVMFSMReSize::ReSize - "
                                                    "SeqNum=%d, PtrS=0x%x, PtrE=0x%x, AllocPtr=0x%x, Begin=%d, End=%d, NewSize=%d, Avail=%d",
                                                    seqNum,
                                                    p,
                                                    p + blkSize + PVMF_RESIZE_ALLOC_OVERHEAD,
                                                    aPCContainer->iAllocationPtr,
                                                    (p - aPCContainer->iParentChunkStart),
                                                    (p - aPCContainer->iParentChunkStart) + blkSize + PVMF_RESIZE_ALLOC_OVERHEAD,
                                                    blkSize,
                                                    getTotalAvailableBufferSpace()));
            PVMF_SOCKALLOC_LOG_AVAILABILITY((0, "PVMFSMSBAWithReSize::ReSizeFromParentChunk - Name=%s, SN=%d, Reclaim=%d, Avail=%d",
                                             iName.get_cstr(), seqNum, aBytesToReclaim, getTotalAvailableBufferSpace()));
            return (PVMF_RESIZE_ALLOC_SUCCESS);
        }

        void resize(OsclAny* p, uint32 aBytesToReclaim)
        {
            ParentChunkContainer* parentChunkContainer = FindParentChunk(p);

            if (parentChunkContainer == NULL)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::resize - INVALID PTR!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
            if (ReSizeFromParentChunk(parentChunkContainer,
                                      p,
                                      aBytesToReclaim) == PVMF_RESIZE_ALLOC_MEMORY_CORRUPT)
            {
                PVMF_SOCKALLOC_LOGERROR((0, "PVMFSMSharedBufferAllocWithReSize::resize - INVALID PTR!!! Name=%s", iName.get_cstr()));
                OSCL_LEAVE(OsclErrArgument);
            }
            CheckAndNotifyFreeChunkAvailable();
        }

        uint32 getNumOutStandingBuffers()
        {
            return iNumOutStandingBuffers;
        }

        uint32 getTotalBufferSize()
        {
            uint32 totalsize = 0;
            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;
            for (it = iParentChunkContainerVec.begin();
                    it != iParentChunkContainerVec.end();
                    it++)
            {
                totalsize += it->iParentChunkSize;
            }
            return (totalsize);
        }

        uint32 getTotalAvailableBufferSpace()
        {
            uint32 freespace = 0;
            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;
            if ((iNumOutStandingBuffers == 1) &&
                    (iDecKeepAliveCalled == false))
            {
                // all outstanding buffers have been freed
                freespace = getTotalBufferSize();
                for (it = iParentChunkContainerVec.begin();
                        it != iParentChunkContainerVec.end();
                        it++)
                {
                    it->iEndOfLastDeallocatedBlock = NULL;
                    it->iAllocationPtr = it->iParentChunkStart;
                }
            }
            else
            {
                for (it = iParentChunkContainerVec.begin();
                        it != iParentChunkContainerVec.end();
                        it++)
                {
                    freespace += FindLargestContiguousFreeBlock(it);
                }
            }
            return (freespace);
        }

        uint32 getActualDataSize()
        {
            return iJJDataSize;
        }

        uint32 getTrueBufferSpace()
        {
            uint32 truespace = 0;
            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;
            if ((iNumOutStandingBuffers == 1) &&
                    (iDecKeepAliveCalled == false))
            {
                // all outstanding buffers have been freed
                truespace = getTotalBufferSize();
                for (it = iParentChunkContainerVec.begin();
                        it != iParentChunkContainerVec.end();
                        it++)
                {
                    it->iEndOfLastDeallocatedBlock = NULL;
                    it->iAllocationPtr = it->iParentChunkStart;
                }
            }
            else
            {
                for (it = iParentChunkContainerVec.begin();
                        it != iParentChunkContainerVec.end();
                        it++)
                {
                    uint32 maxSize = 0;
                    if (it != NULL)
                    {
                        if (it->iEndOfLastDeallocatedBlock == NULL)
                        {
                            /*
                             * Implies that there is no deallocations have happenned
                             */
                            maxSize =
                                (it->iParentChunkEnd - it->iAllocationPtr);
                        }
                        else
                        {
                            if (it->iAllocationPtr <= it->iEndOfLastDeallocatedBlock)
                            {
                                maxSize =
                                    (it->iEndOfLastDeallocatedBlock - it->iAllocationPtr);
                            }
                            else
                            {
                                uint32 free_end =
                                    (it->iParentChunkEnd - it->iAllocationPtr);
                                uint32 free_begin =
                                    (it->iEndOfLastDeallocatedBlock - it->iParentChunkStart);
                                maxSize = free_end + free_begin;
                            }
                        }
                    }
                    truespace += maxSize;
                }
            }
            PVMF_SOCKALLOC_LOG_AVAILABILITY((0, "SM alloc space new1 %d new2 %d kspace %d Ln %d", iJJDataSize, iJJDataDbgSize, truespace, __LINE__));
            return (truespace);
        }


        uint32 FindLargestContiguousFreeBlock(ParentChunkContainer* aPtr)
        {
            ParentChunkContainer* it = aPtr;
            uint32 maxSize = 0;
            if (it != NULL)
            {
                if (it->iEndOfLastDeallocatedBlock == NULL)
                {
                    /*
                     * Implies that there is no deallocations have happenned
                     */
                    maxSize =
                        (it->iParentChunkEnd - it->iAllocationPtr);
                }
                else
                {
                    /*
                     * There are two possibilites:
                     * 1) iAllocationPtr < iEndOfLastDeallocatedBlock - This means
                     * that a wrap around has happenned, and we need to check
                     * if there is a large enough block between the two ptrs.
                     * Please note that we assume that the space between
                     * iAllocationPtr and iEndOfLastDeallocatedBlock is all free.
                     * Reason being that blocks are deallocated in the same
                     * order in which they are allocated.
                     *
                     * 2) iAllocationPtr > iEndOfLastDeallocatedBlock - This means
                     * we are ok.
                     *
                     */
                    if (it->iAllocationPtr <= it->iEndOfLastDeallocatedBlock)
                    {
                        maxSize =
                            (it->iEndOfLastDeallocatedBlock - it->iAllocationPtr);
                    }
                    else
                    {
                        uint32 free_end =
                            (it->iParentChunkEnd - it->iAllocationPtr);
                        uint32 free_begin =
                            (it->iEndOfLastDeallocatedBlock - it->iParentChunkStart);
                        if (free_end > free_begin)
                        {
                            maxSize = free_end;
                        }
                        else
                        {
                            maxSize = free_begin;
                        }
                    }
                }
            }
            return (maxSize);
        }


        uint32 getAvailableBufferSpace(bool aFirstParentChunkOnly = false)
        {
            uint32 freespace = 0;
            Oscl_Vector<ParentChunkContainer, OsclMemAllocator>::iterator it;
            if (aFirstParentChunkOnly)
            {
                /*
                 * Since we cannot inform the server about buffer growth, we
                 * should only send free space from the first allocated chunk.
                 */
                it = iParentChunkContainerVec.begin();
                freespace = FindLargestContiguousFreeBlock(it);
            }
            else
            {
                for (it = iParentChunkContainerVec.begin();
                        it != iParentChunkContainerVec.end();
                        it++)
                {
                    uint32 size = FindLargestContiguousFreeBlock(it);
                    if (size > freespace)
                    {
                        freespace = size;
                    }
                }
            }
            return (freespace);
        }

        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& aObserver,
                                      uint32 aSize,
                                      OsclAny* aContextData = NULL)
        {
            PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "notifyfreechunkavailable - reqSize=%d", (aSize + PVMF_RESIZE_ALLOC_OVERHEAD)));
            iCallbackPending = true;
            iCallbackRequestSize = aSize + PVMF_RESIZE_ALLOC_OVERHEAD;
            iObserver = &aObserver;
            iNextAvailableContextData = aContextData;
        }

        void CancelFreeChunkAvailableCallback()
        {
            iCallbackPending = false;
            iCallbackRequestSize = 0;
            iObserver = NULL;
        }

        void CheckAndNotifyFreeChunkAvailable()
        {
            if (iCallbackPending == true)
            {
                uint32 availSize = getTrueBufferSpace();

                PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "CheckAndNotifyFreeChunkAvailable() availSize %d", availSize));
                if (availSize > iCallbackRequestSize)
                {
                    PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "CheckAndNotifyFreeChunkAvailable() availSize(%d) > iCallbackRequestSize(%d)", availSize, iCallbackRequestSize));
                    if (iObserver == NULL)
                        return;
                    OsclMemPoolFixedChunkAllocatorObserver* MyObserver = iObserver;
                    PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "CheckAndNotifyFreeChunkAvailable() MyObserver(0x%x) iObserver(0x%x) ", MyObserver, iObserver));
                    CancelFreeChunkAvailableCallback();
                    PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "CheckAndNotifyFreeChunkAvailable MyObserver(0x%x)", MyObserver));
                    MyObserver->freechunkavailable(iNextAvailableContextData);
                    PVMF_SOCKALLOC_LOG_MEMCALLBACK((0, "CheckAndNotifyFreeChunkAvailable() iNextAvailableContextData %d out", iNextAvailableContextData));
                }
            }
        }

        void NotifyDeallocations(PVMFSMSharedBufferAllocWithReSizeAllocDeallocObserver& aObserver,
                                 OsclAny* aContextData = NULL)
        {
            iDeallocObserver = &aObserver;
            iDeallocNotificationContextData = aContextData;
        }

        void CancelDeallocationNotifications()
        {
            iDeallocObserver = NULL;
            iDeallocNotificationContextData = NULL;
        }

    private:
        Oscl_Vector<ParentChunkContainer, OsclMemAllocator> iParentChunkContainerVec;

        OsclMemAllocator alloc;

        uint32 iNumOutOfOrderDeallocs;
        Oscl_Vector<OutOfOrderBlockContainer, OsclMemAllocator> iOutofOrderBlockVec;

        int32 iLastDeallocatedSeqNum;
        uint32 iSeqCount;
        uint32 iNumOutStandingBuffers;
        PVLogger *iLogger;
        PVLogger *iSizeLogger;
        PVLogger *iCallBackLogger;
        PVLogger *iOOOLogger;
        uint32 iJJDataSize;
        uint32 iJJDataDbgSize;
        OSCL_HeapString<OsclMemAllocator> iName;
        ReassemblyBlockList iOutOfOrderBlocks;

        // buffer resize parameters
        uint iiMaxNumGrows;
        uint iiNumGrows;
        uint iiRegrowthSize;

        //callback related params
        bool iCallbackPending;
        uint32 iCallbackRequestSize;
        OsclMemPoolFixedChunkAllocatorObserver* iObserver;
        OsclAny* iNextAvailableContextData;

        bool iDecKeepAliveCalled;

        PVMFSMSharedBufferAllocWithReSizeAllocDeallocObserver* iDeallocObserver;
        OsclAny* iDeallocNotificationContextData;
};

class PVMFSMSharedBufferAllocWithReSizeCleanupDA : public OsclDestructDealloc
{
    public:
        PVMFSMSharedBufferAllocWithReSizeCleanupDA(Oscl_DefAlloc* in_gen_alloc) :
                gen_alloc(in_gen_alloc) {};

        virtual ~PVMFSMSharedBufferAllocWithReSizeCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            /*
             * get number of outstanding buffers from the allocator
             */
            Oscl_DefAlloc* myalloc = gen_alloc;
            PVMFSMSharedBufferAllocWithReSize* socketDataAllocator =
                reinterpret_cast<PVMFSMSharedBufferAllocWithReSize*>(myalloc);
            gen_alloc->deallocate(ptr);

            uint32 numBuffers = socketDataAllocator->getNumOutStandingBuffers();

            /*
             * get the number of buffers deallocated, after the pointer has
             * been deallocated. A single dealloc call can potentially deallocate
             * multiple buffers (if there are many out of order blocks).
             */
            /*
             * in case there are no outstanding buffers delete the allocator
             */
            if (numBuffers == 0)
            {
                OSCL_DELETE((socketDataAllocator));
            }
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};

class PVMFSharedSocketDataBufferAlloc
{
    public:
        PVMFSharedSocketDataBufferAlloc(Oscl_DefAlloc* in_gen_alloc)
        {
            iResizeAlloc = false;
            if (in_gen_alloc)
            {
                gen_alloc = in_gen_alloc;
                iBufferOverhead = 0;
            }
            else
            {
                OSCL_LEAVE(OsclErrArgument);
            }
        }

        PVMFSharedSocketDataBufferAlloc(PVMFSMSharedBufferAllocWithReSize* in_gen_alloc)
        {
            iResizeAlloc = false;
            if (in_gen_alloc)
            {
                iResizeAlloc = true;
                gen_alloc = in_gen_alloc;
                iBufferOverhead = 0;
                uint aligned_class_size =
                    oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
                uint aligned_cleanup_size =
                    oscl_mem_aligned_size(sizeof(PVMFSMSharedBufferAllocWithReSizeCleanupDA));
                uint aligned_refcnt_size =
                    oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
                iBufferOverhead = (aligned_refcnt_size +
                                   aligned_cleanup_size +
                                   aligned_class_size);

            }
            else
            {
                OSCL_LEAVE(OsclErrArgument);
            }
        };

        virtual ~PVMFSharedSocketDataBufferAlloc()
        {
        };

        OsclSharedPtr<PVMFMediaDataImpl> createSharedBuffer(uint32 size)
        {
            if (size == 0)
            {
                size = PVMF_SOCKET_BUF_DEFAULT_SIZE;
            }

            uint aligned_in_size = oscl_mem_aligned_size(size);

            if (iResizeAlloc)
            {
                uint8* my_ptr;
                OsclRefCounter* my_refcnt;

                uint aligned_class_size =
                    oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));

                uint aligned_cleanup_size =
                    oscl_mem_aligned_size(sizeof(PVMFSMSharedBufferAllocWithReSizeCleanupDA));

                uint aligned_refcnt_size =
                    oscl_mem_aligned_size(sizeof(OsclRefCounterDA));

                my_ptr = (uint8*) gen_alloc->allocate(aligned_refcnt_size +
                                                      aligned_cleanup_size +
                                                      aligned_class_size +
                                                      aligned_in_size);

                PVMFSMSharedBufferAllocWithReSizeCleanupDA *my_cleanup =
                    OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PVMFSMSharedBufferAllocWithReSizeCleanupDA(gen_alloc));

                my_refcnt =
                    OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));

                my_ptr += aligned_refcnt_size + aligned_cleanup_size;

                void* ptr;
                ptr = my_ptr + aligned_class_size;

                PVMFMediaDataImpl* media_data_ptr =
                    new(my_ptr) PVMFSimpleMediaBuffer(ptr,
                                                      aligned_in_size,
                                                      my_refcnt);

                OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr,
                        my_refcnt);
                return shared_media_data;
            }
            else
            {
                uint8* my_ptr;
                OsclRefCounter* my_refcnt;

                uint aligned_class_size =
                    oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));

                uint aligned_cleanup_size =
                    oscl_mem_aligned_size(sizeof(PVMFSocketBufferCleanupDA));

                uint aligned_refcnt_size =
                    oscl_mem_aligned_size(sizeof(OsclRefCounterDA));

                my_ptr = (uint8*) gen_alloc->allocate(aligned_refcnt_size +
                                                      aligned_cleanup_size +
                                                      aligned_class_size +
                                                      aligned_in_size);

                PVMFSocketBufferCleanupDA *my_cleanup =
                    OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PVMFSocketBufferCleanupDA(gen_alloc));

                my_refcnt =
                    OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));

                my_ptr += aligned_refcnt_size + aligned_cleanup_size;

                void* ptr;
                ptr = my_ptr + aligned_class_size;

                PVMFMediaDataImpl* media_data_ptr =
                    new(my_ptr) PVMFSimpleMediaBuffer(ptr,
                                                      aligned_in_size,
                                                      my_refcnt);

                OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr,
                        my_refcnt);
                return shared_media_data;
            }
        }

        void ResizeMemoryFragment(OsclSharedPtr<PVMFMediaDataImpl>& aSharedBuffer)
        {
            if (iResizeAlloc)
            {
                OsclRefCounterMemFrag memFrag;
                aSharedBuffer->getMediaFragment(0, memFrag);
                uint32 currCapacity = memFrag.getCapacity();
                uint32 bytesUsed = memFrag.getMemFragSize();

                //uint32 alignedBytesUsed = bytesUsed;
                uint32 alignedBytesUsed = oscl_mem_aligned_size(bytesUsed);

                if (alignedBytesUsed < currCapacity)
                {
                    uint32 bytesToReclaim = (currCapacity - alignedBytesUsed);
                    PVMFSMSharedBufferAllocWithReSize* socketDataAllocator =
                        reinterpret_cast<PVMFSMSharedBufferAllocWithReSize*>(gen_alloc);
                    /* Account for the overhead */
                    uint8* memFragPtr = (uint8*)(memFrag.getMemFragPtr());
                    uint8* ptr = (memFragPtr - iBufferOverhead);
                    socketDataAllocator->resize((OsclAny*)ptr, bytesToReclaim);
                    aSharedBuffer->setCapacity(alignedBytesUsed);
                }
            }
        }

        uint32 getAvailableBufferSpace(bool aFirstParentChunkOnly = false)
        {
            if (iResizeAlloc)
            {
                PVMFSMSharedBufferAllocWithReSize* socketDataAllocator =
                    reinterpret_cast<PVMFSMSharedBufferAllocWithReSize*>(gen_alloc);
                return (socketDataAllocator->getAvailableBufferSpace(aFirstParentChunkOnly));
            }
            else
            {
                PVMFSocketBufferAllocator* socketDataAllocator =
                    reinterpret_cast<PVMFSocketBufferAllocator*>(gen_alloc);
                return (socketDataAllocator->getAvailableBufferSpace(aFirstParentChunkOnly));
            }
        }
        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& aObserver,
                                      uint32 aSize,
                                      OsclAny* aContextData = NULL)
        {
            if (iResizeAlloc)
            {
                PVMFSMSharedBufferAllocWithReSize* socketDataAllocator =
                    reinterpret_cast<PVMFSMSharedBufferAllocWithReSize*>(gen_alloc);
                socketDataAllocator->notifyfreechunkavailable(aObserver, (aSize + iBufferOverhead), aContextData);
            }
            else
            {
                PVMFSocketBufferAllocator* socketDataAllocator =
                    reinterpret_cast<PVMFSocketBufferAllocator*>(gen_alloc);
                socketDataAllocator->notifyfreechunkavailable(aObserver, (aSize + iBufferOverhead), aContextData);
            }
        }

        void CancelFreeChunkAvailableCallback()
        {
            if (iResizeAlloc)
            {
                PVMFSMSharedBufferAllocWithReSize* socketDataAllocator =
                    reinterpret_cast<PVMFSMSharedBufferAllocWithReSize*>(gen_alloc);
                socketDataAllocator->CancelFreeChunkAvailableCallback();
            }
            else
            {
                PVMFSocketBufferAllocator* socketDataAllocator =
                    reinterpret_cast<PVMFSocketBufferAllocator*>(gen_alloc);
                socketDataAllocator->CancelFreeChunkAvailableCallback();
            }
        }

    private:
        bool iResizeAlloc;
        uint iBufferOverhead;
        Oscl_DefAlloc* gen_alloc;
};

class PVMFSharedSocketDataBufferAllocCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~PVMFSharedSocketDataBufferAllocCleanupSA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;

            uint aligned_refcnt_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterSA<PVMFSharedSocketDataBufferAllocCleanupSA>));

            tmp_ptr += aligned_refcnt_size;
            PVMFSharedSocketDataBufferAlloc* socketDataBufferAlloc =
                reinterpret_cast<PVMFSharedSocketDataBufferAlloc*>(tmp_ptr);

            socketDataBufferAlloc->~PVMFSharedSocketDataBufferAlloc();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};


#endif //PVMF_STREAMING_BUFFER_ALLOCATORS_H_INCLUDED
