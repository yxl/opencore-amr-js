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
#ifndef OSCL_VARIABLESIZE_MEMPOOL_H_INCLUDED
#define OSCL_VARIABLESIZE_MEMPOOL_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  Variable size memory pool /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_NUM_CHUNKS_IN_VARIABLE_SIZE_MEMORY_POOL 16

/** \class allocator
** A memory allocator class which allocates and deallocated from a variable size memory pool;
**
*/

class OsclMemPoolVariableChunkAllocatorObserver
{
    public:
        virtual void freeVariableSizeChunkAvailable(OsclAny* aContextData) = 0;
        virtual ~OsclMemPoolVariableChunkAllocatorObserver()
        {
            ;
        }
};

class OsclMemPoolVariableChunkAllocator : public Oscl_DefAlloc
{
    public:

        /**
         * This API throws an exception when the memory allocation for pool fails
         * If maximum pool size must be provided, otherwise it will throw an exception
         *
         * @return void
         *
         */
        OsclMemPoolVariableChunkAllocator(const uint32 maxPoolSize) :
                iMaxPoolSize(maxPoolSize), iMemPool(NULL), iNumRealAlloc(0), iAverAllocSize(0),
                iCheckNextAvailableFreeChunk(false), iObserver(NULL)
        {
            createMempool();
        }


        virtual ~OsclMemPoolVariableChunkAllocator()
        {
            destroyMempool();
        }

        /**
         * This API throws an exception when n is greater than the maximum pool size or when there are no chunk available in the pool for the request size n
         * If the memory pool hasn't been created yet, the pool will be created with the current maximum pool size
         * Exception will be thrown if memory allocation for the memory pool fails.
         *
         * @return pointer to available chunk from memory pool
         *
         */
        OsclAny* allocate(const uint32 n)
        {
            // Create the memory pool if it hasn't been created yet.
            if (iMemPool == NULL) createMempool();

            // sanity check
            uint32 request_size = oscl_mem_aligned_size(n);
            if (!allocateSanityCheck(request_size))
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return NULL;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // do actual allocation
            return doAllocate(request_size);
        }

        /**
         * This API throws an exception when the pointer p passed in is not part of the memory pool.
         * Exception will be thrown if the memory pool is not set up yet.
         *
         * @return void
         *
         */
        void deallocate(OsclAny* p)
        {
            // sanity check for "p"
            if (!deallocateSanityCheck(p))
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // re-claim "p"
            if (!doDeAllocate(p))
            {
                // Returned memory is not aligned to the chunk.
                OSCL_LEAVE(OsclErrNoMemory);
                // return;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // Notify the observer about free chunk available if waiting for such callback
            if (iCheckNextAvailableFreeChunk)
            {
                iCheckNextAvailableFreeChunk = false;
                if (iObserver) iObserver->freeVariableSizeChunkAvailable((OsclAny*)this);
            }
        }

        /**
         * Returns a tail segment of a previously allocated memory segmant back to the memory pool.
         * This function allows the user to allocate a larger size memory segment initially when the amount needed is unknown
         * and then return the unused portion of the segment when the amount becomes known.
         * This API throws an exception if the pointer passed in is not part of the memory pool or the
         * size to return is bigger than the size of the passed-in block.
         * Exception will be thrown if the memory pool is not set up yet.
         *
         * @param aPtr is the starting pointer of the unused portion of memory segment
         * @param aBytesToFree is the length of the unused portion
         * @return bool True if reclaim operation successful.
         *
         */
        bool reclaimUnusedPortion(OsclAny* aPtr, uint32 aBytesToFree)
        {
            // sanity check for "p"
            if (!deallocateSanityCheck(aPtr))
            {
                OSCL_LEAVE(OsclErrNoMemory);
                return false;
            }

            // construct a new memory chunk for this unused portion of pre-allocated memory chunk
            OsclMemoryFragment memChunk;
            memChunk.ptr = aPtr;
            memChunk.len = aBytesToFree;

            // reclaim this segment to iFreeMemChunkList
            return reclaim(&memChunk);
        }

        /**
         * This API will retrieve the maximum free memory chunk size to help user get the idea of how
         * big the next allocation size could be.
         *
         * @return maximum chunk size, if there is no free memory or the memory pool hasn't been created, then return 0
         *
         */
        uint32 getMaxFreeChunkSize()
        {
            if (iFreeMemChunkList.empty() || iMemPool == NULL) return 0;

            uint32 maxChunkSize = 0;
            for (uint32 i = 0; i < iFreeMemChunkList.size(); i++)
            {
                if (maxChunkSize < iFreeMemChunkList[i].len)
                {
                    maxChunkSize = iFreeMemChunkList[i].len;
                }
            }
            return maxChunkSize;
        }

        /**
         * This two APIs will retrieve the total free memory size (the actual size should be smaller due to fragment) and
         * the actual memory usage (8-byte alignement) to help user get the idea of memory usage.
         *
         * @return total free memory size, if there is no free memory or the memory pool hasn't been created, then return 0
         *
         */
        uint32 getTotalAvailableSize()
        {
            if (iFreeMemChunkList.empty() || iMemPool == NULL) return 0;

            uint32 totalSize = 0;
            for (uint32 i = 0; i < iFreeMemChunkList.size(); i++)
            {
                totalSize += iFreeMemChunkList[i].len;
            }
            return totalSize;
        }

        uint32 getCurrMemoryUsage()
        {
            if (iUsedMemChunkList.empty() || iMemPool == NULL) return 0;

            uint32 totalSize = 0;
            for (uint32 i = 0; i < iUsedMemChunkList.size(); i++)
            {
                totalSize += iUsedMemChunkList[i].len;
            }
            return totalSize;
        }

        uint32 getPoolSize() const
        {
            return iMaxPoolSize;
        }


        /**
         * This API clears all the records of all allocations. Please NOTE that, after this API gets called,
         * the memory pool returns to the initial state, i.e. one big free segment.
         *
         */
        void clear()
        {
            // clear the records
            iFreeMemChunkList.clear();
            iUsedMemChunkList.clear();
            iNumRealAlloc = 0;
            iAverAllocSize = 0;

            // set up the first memory segment in the free mem chunk list
            OsclMemoryFragment memChunk;
            memChunk.ptr = iMemPool;
            memChunk.len = iMaxPoolSize;
            iFreeMemChunkList.push_back(memChunk);
        }

        /**
         * This API will set the flag to send a callback via specified observer object when the
         * next memory chunk is deallocated by deallocate() call..
         *
         * @return void
         *
         */
        void notifyFreeChunkAvailable(OsclMemPoolVariableChunkAllocatorObserver& obs)
        {
            iCheckNextAvailableFreeChunk = true;
            iObserver = &obs;
        }

    private:

        //////////////////////////////////////////////////////////////////////////////
        /////// create and destroy memory pool ///////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////

        void createMempool()
        {
            if (iMaxPoolSize == 0)
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // Create one block of memory for the memory pool
            iMaxPoolSize = oscl_mem_aligned_size(iMaxPoolSize);
            iMemPool = OSCL_MALLOC(iMaxPoolSize);
            if (iMemPool == NULL)
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // Set up the free and used mem chunk list vector
            int32 err = 0;
            OSCL_TRY(err,
                     iFreeMemChunkList.reserve(DEFAULT_NUM_CHUNKS_IN_VARIABLE_SIZE_MEMORY_POOL);
                     iUsedMemChunkList.reserve(DEFAULT_NUM_CHUNKS_IN_VARIABLE_SIZE_MEMORY_POOL);
                    );
            if (err)
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return;	This statement was removed to avoid compiler warning for Unreachable Code
            }

            // set up the first memory segment in the free mem chunk list
            OsclMemoryFragment memChunk;
            memChunk.ptr = iMemPool;
            memChunk.len = iMaxPoolSize;
            iFreeMemChunkList.push_back(memChunk);
        }

        void destroyMempool()
        {
            iFreeMemChunkList.clear();
            iUsedMemChunkList.clear();
            if (iMemPool) OSCL_FREE(iMemPool);
        }


        //////////////////////////////////////////////////////////////////////////////
        /////// Supporting functions for API allocate()  /////////////////////////////
        //////////////////////////////////////////////////////////////////////////////
        bool allocateSanityCheck(const uint32 n)
        {
            if (n > iMaxPoolSize) return false;
            if (iFreeMemChunkList.empty())  return false;

            return true;
        }

        int32 PushMemChunkToChunkVector(OsclMemoryFragment* aMemChunk)
        {
            int32 leavecode = OsclErrNone;
            OSCL_TRY(leavecode, iUsedMemChunkList.push_back(*aMemChunk));
            return leavecode;
        }

        OsclAny* doAllocate(const uint32 n)
        {
            OsclMemoryFragment memChunk;
            OsclMemoryFragment *pMemChunk = &memChunk;

            // get the available memory chunk
            uint32 alloc_size = n;
            if (!getFreeMemChunk(pMemChunk, alloc_size))
            {
                // No free chunk is available
                OSCL_LEAVE(OsclErrNoMemory);
                // return NULL;		This statement was removed to avoid compiler warning for Unreachable Code
            }

            // insert the removed segment into the iUsedMemChunkList if there is
            int32 err = PushMemChunkToChunkVector(pMemChunk);
            if (err)
            {
                OSCL_LEAVE(OsclErrNoMemory);
                // return NULL;		This statement was removed to avoid compiler warning for Unreachable Code
            }

            // calculate the average alloc size for next split decision --
            // to split, the remaining size should not be smaller than the so-far average allocated size
            iAverAllocSize = (iAverAllocSize * iNumRealAlloc + alloc_size);
            iAverAllocSize /= (OsclFloat)(++iNumRealAlloc);

            return pMemChunk->ptr;
        }

        bool getFreeMemChunk(OsclMemoryFragment* pMemChunk, uint32 &alloc_size)
        {
            pMemChunk->ptr = NULL;
            pMemChunk->len = 0;

            for (uint32 i = 0; i < iFreeMemChunkList.size(); i++)
            {
                if (iFreeMemChunkList[i].len < alloc_size) continue;

                // Allocation must happen
                if (iFreeMemChunkList[i].len - alloc_size > (uint32)iAverAllocSize)
                {
                    // split the current segment
                    pMemChunk->ptr = iFreeMemChunkList[i].ptr;
                    pMemChunk->len = alloc_size;

                    uint8 *p = (uint8*)iFreeMemChunkList[i].ptr + alloc_size;
                    iFreeMemChunkList[i].ptr = (OsclAny*)p;
                    iFreeMemChunkList[i].len -= alloc_size;
                }
                else
                {
                    // allocate the whole segment
                    pMemChunk->ptr = iFreeMemChunkList[i].ptr;
                    pMemChunk->len = iFreeMemChunkList[i].len;
                    alloc_size = pMemChunk->len;

                    // remove this segment
                    iFreeMemChunkList.erase(iFreeMemChunkList.begin() + i);
                }
                break;
            }

            return (pMemChunk->ptr != NULL);
        }

        //////////////////////////////////////////////////////////////////////////////
        /////// Supporting functions for API deallocate()  ///////////////////////////
        //////////////////////////////////////////////////////////////////////////////
        bool deallocateSanityCheck(OsclAny* p)
        {
            if (iMemPool == NULL)
            {
                // Memory pool hasn't been allocated yet so error
                return false;
            }

            uint8* ptmp = (uint8*)p;
            uint8* mptmp = (uint8*)iMemPool;
            if (ptmp < mptmp || ptmp >= (mptmp + iMaxPoolSize))
            {
                // Returned memory is not part of this memory pool
                return false;
            }

            if (iUsedMemChunkList.empty()) return false;

            return true;
        }

        bool doDeAllocate(OsclAny* p)
        {
            // remove the segment associated with "p" from iUsedMemChunkList
            OsclMemoryFragment memChunk;
            bool bFound = false;
            for (uint32 i = 0; i < iUsedMemChunkList.size(); i++)
            {
                if ((uint8*)iUsedMemChunkList[i].ptr == (uint8*)p)
                {
                    memChunk.ptr = iUsedMemChunkList[i].ptr;
                    memChunk.len = iUsedMemChunkList[i].len;
                    iUsedMemChunkList.erase(iUsedMemChunkList.begin() + i);
                    bFound = true;
                    break;
                }
            }

            if (!bFound) return false; // Not found

            // reclaim this segment to iFreeMemChunkList
            return reclaim(&memChunk);
        }

        bool reclaim(OsclMemoryFragment *aMemChunk)
        {
            // short cut
            if (iFreeMemChunkList.empty())
            {
                int32 err = 0;
                OSCL_TRY(err, iFreeMemChunkList.push_back(*aMemChunk));
                return (err == 0);
            }

            // the input memory segment can be continuous with one or two existing memory segments in iFreeMemChunkList
            // and thus it can be merged into the existing continuous memory segments
            // so search the continuities: left continuity(the input segment is continuous to an existing memory segment on its left side)
            //							   right continuity(the input segment is continuous to an existing memory segment on its right side)
            //							   both continuity(the input segment is continuous to two existing memory segments on both sides)
            int32 index_leftContinuity = -1, index_rightContinuity = -1;
            searchSegmentContinuity(aMemChunk, index_leftContinuity, index_rightContinuity);

            // merge the two or three continuous memory segments if there are
            return doMerge(aMemChunk, index_leftContinuity, index_rightContinuity);
        }

        /**
          * the input memory segment can be continuous with one or two existing memory segments in iFreeMemChunkList
          * and thus it can be merged into the existing continuous memory segments
          * so search the continuities; left continuity(the input segment is continuous to an existing memory segment on its left side)
          *							    right continuity(the input segment is continuous to an existing memory segment on its right side)
          *							    both-sided continuity(the input segment is continuous to two existing memory segments on both sides)
             * @param aMemChunk,			  input memory segment
           * @param index_leftContinuity,  index of the existing memory segment in iFreeMemChunkList that is continuous to
          *								  the input memory segment on its left side
          * @param index_rightContinuity, index of the existing memory segment in iFreeMemChunkList that is continuous to
          *								  the input memory segment on its left side
        **/
        void searchSegmentContinuity(OsclMemoryFragment *aMemChunk, int32 &index_leftContinuity, int32 &index_rightContinuity)
        {
            uint8 *leftPtr  = (uint8 *)aMemChunk->ptr;
            uint8 *rightPtr = (uint8 *)aMemChunk->ptr + aMemChunk->len;

            for (uint32 i = 0; i < iFreeMemChunkList.size(); i++)
            {
                if (leftPtr == (uint8*)iFreeMemChunkList[i].ptr + iFreeMemChunkList[i].len)
                {
                    index_leftContinuity = (int32)i;
                }

                if (rightPtr == (uint8*)iFreeMemChunkList[i].ptr)
                {
                    index_rightContinuity = (int32)i;
                }

                if (index_leftContinuity >= 0 && index_rightContinuity >= 0) break;
            }
        }

        /**
         * merge the two or three continuous memory segments if there are
         *
            * @param aMemChunk,			 input memory segment
          * @param index_leftContinuity,	 index of the existing memory segment in iFreeMemChunkList that is continuous to
         *								 the input memory segment on its left side, or not
         * @param index_rightContinuity, index of the existing memory segment in iFreeMemChunkList that is continuous to
         *								 the input memory segment on its left side, or not
         * @return true => success, false => fail in push_back operation
         **/
        bool doMerge(OsclMemoryFragment *aMemChunk, const int32 index_leftContinuity, const int32 index_rightContinuity)
        {
            if (index_leftContinuity < 0 && index_rightContinuity < 0)
            {
                // no merge, push to the end of iFreeMemChunkList
                int32 err = 0;
                OSCL_TRY(err, iFreeMemChunkList.push_back(*aMemChunk));
                return (err == 0);
            }

            // merge two or three segments
            if (index_leftContinuity >= 0 && index_rightContinuity >= 0)  // merge three segments
            {
                // keep the segment with index_leftContinuity, and remove the segment with index_rightContinuity
                iFreeMemChunkList[index_leftContinuity].len += (aMemChunk->len + iFreeMemChunkList[index_rightContinuity].len);
                iFreeMemChunkList.erase(iFreeMemChunkList.begin() + index_rightContinuity);
            }
            else if (index_leftContinuity >= 0)
            {
                iFreeMemChunkList[index_leftContinuity].len += aMemChunk->len;
            }
            else if (index_rightContinuity >= 0)
            {
                iFreeMemChunkList[index_rightContinuity].ptr =  aMemChunk->ptr;
                iFreeMemChunkList[index_rightContinuity].len += aMemChunk->len;
            }

            return true;
        }

    private:

        uint32 iMaxPoolSize;
        OsclAny* iMemPool;
        uint32 iNumRealAlloc;
        OsclFloat iAverAllocSize;

        Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> iFreeMemChunkList;
        Oscl_Vector<OsclMemoryFragment, OsclMemAllocator> iUsedMemChunkList;

        bool iCheckNextAvailableFreeChunk;
        OsclMemPoolVariableChunkAllocatorObserver* iObserver;
};

#endif
