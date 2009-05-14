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

/*! \file oscl_mem_audit.cpp
    \brief This file contains the implementation of MM_Audit class
*/
#include "osclconfig_memory.h"

#if (!OSCL_BYPASS_MEMMGT)

#include "oscl_types.h"
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_mem_audit_internals.h"
#include "oscl_assert.h"
#include "oscl_stdstring.h"


#if MM_AUDIT_FENCE_SUPPORT
const uint32 PRE_PAD_SIZE = sizeof(MM_AllocBlockFence);
const uint32 POST_FENCE_SIZE = sizeof(MM_AllocBlockFence);
#else
const uint32 PRE_PAD_SIZE = 0;
const uint32 POST_FENCE_SIZE = 0;
#endif

const uint32 CONTROL_HEADER_SIZE = sizeof(MM_AllocBlockHdr);
const uint32 BLOCK_HDR_SIZE = CONTROL_HEADER_SIZE + PRE_PAD_SIZE;
const uint32 BLOCK_OVERHEAD_SIZE = BLOCK_HDR_SIZE + POST_FENCE_SIZE;


/**
 * MM_Audit_Imp constructor and descontructor
 */

/* ======================================================================== */
/*	Function : MM_Audit()													*/
/*	Date     : 10/08/2002													*/
/*	Purpose  : Constructor, create the root node in statistics table		*/
/*	In/out   :																*/
/*	Return   :																*/
/*	Modified :																*/
/* ======================================================================== */

const char root_tag[] = "";

MM_Audit_Imp::MM_Audit_Imp()
{
    mpCurrAllocNode = mpAllocNode = NULL;
    mNumAllocNodes = 0;

    prefill_pattern = DEFAULT_PREFILL_PATTERN;
    postfill_pattern = DEFAULT_POSTFILL_PATTERN;

    mpStatsNode = NULL;
    mnAllocNum = 0;
    mnMaxTagLevel = 10;

    mm_audit_per_block_overhead = 0;
    mm_audit_stats_overhead = 0;

    mode = DEFAULT_MM_AUDIT_MODE;

    /* create root node in memory statistics table */
    mpStatsNode = createStatsNode(root_tag);
}

/* ======================================================================== */
/*	Function : ~MM_Audit()													*/
/*	Date     : 10/08/2002													*/
/*	Purpose  : destructor, remove all the nodes in allocation and			*/
/*			   statistics table												*/
/*	In/out   :																*/
/*	Return   :																*/
/*	Modified :																*/
/* ======================================================================== */

MM_Audit_Imp::~MM_Audit_Imp()
{

#if MM_AUDIT_ALLOC_NODE_SUPPORT
    /**
     * Delete all the allocation nodes
     */
    removeALLAllocNodes();
    mpAllocNode = mpCurrAllocNode = NULL;
    mnAllocNum = 0;
#endif

    /**
     * Delete all the statistics nodes
     */
    mpStatsNode = NULL;

    mTagTree.clear();
}


/*
* ===================== API FUNCTIONS: MM_xxx() ==================================
*/

/* ============================================================================ */
/*	Function : MM_allocate()													*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API for a memory allocation attempt: create a new node in		*/
/*			   allocation table and update/create the corresponding node in		*/
/*			   statistics table													*/
/*	In/out   : all parameters are input											*/
/*	Return   : memory pointer, if something is wrong, the pointer will be NULL	*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF void* MM_Audit_Imp::MM_allocate(const OsclMemStatsNode* pInStatsNode,
        uint32 sizeIn,
        const char *pFileName,
        uint32 lineNumber,
        bool allocNodeTracking)
{

#if defined(DEBUG_ENABLE)
    PVUStackTrace();
    if (pFileName)
    {
        printf("In MM_allocate, nBytes %d, file_name is %s, line_num is %d\n", sizeIn, pFileName, lineNumber);
    }
    else
    {
        printf("In MM_allocate with tag %s operator, nBytes %d\n", sizeIn);
    }
#endif

    MMAuditUint8AutoPtr pMem;
    void  *pMem_out = NULL;

#if (MM_AUDIT_ALLOC_NODE_SUPPORT && MM_AUDIT_INCLUDE_ALL_HEAP_VALIDATION)
    if (mode & MM_AUDIT_VALIDATE_ALL_HEAP_FLAG)
    {
        validate_all_heap();
    }
#endif

    OsclMemStatsNode* pStatsNode;
    if (pInStatsNode)
    {
        // remove "constness" of the stats node so it can be updated.
        pStatsNode = const_cast<OsclMemStatsNode*>(pInStatsNode);
    }
    else
    {
        pStatsNode = mpStatsNode; // set to the root node.
    }

    uint32 full_size = sizeIn + BLOCK_OVERHEAD_SIZE;

#if MM_AUDIT_FAILURE_SIMULATION_SUPPORT
    if (!isSetFailure(pStatsNode))
    {
        pMem.allocate(full_size);
    }
#else
    if (full_size > sizeIn)
    {
        pMem.allocate(full_size);
    }
#endif

    if (!pMem.get())
    {
        updateStatsNodeInFailure(pStatsNode);
        return NULL;
    }


    MM_AllocNode* pAllocNode = NULL;
#if MM_AUDIT_ALLOC_NODE_SUPPORT

    if ((mode & MM_AUDIT_ALLOC_NODE_ENABLE_FLAG) || allocNodeTracking ||
            (sizeIn & MM_AllocBlockHdr::ALLOC_NODE_FLAG))
    {
        if ((pAllocNode = addAllocNode((void *)pMem.get(), sizeIn, pStatsNode, pFileName, lineNumber)) == NULL)
        {
            updateStatsNodeInFailure(pStatsNode);
            return NULL;
        }
    }
#endif

    // now write the header to the block
    MM_AllocBlockHdr *block_hdr =
        static_cast<MM_AllocBlockHdr *>(static_cast<void*>(pMem.get()));
    block_hdr->size = sizeIn;
    if (pAllocNode)
    {
        block_hdr->setAllocNodeFlag();
        block_hdr->pNode = pAllocNode;
    }
    else
    {
        block_hdr->pNode =  pStatsNode;
    }

    /**
    * Save the root audit node pointer in the header.
    * This pointer will be used when the block is deallocated.
    */
    OsclAuditCB audit;
    OsclMemInit(audit);
    block_hdr->pRootNode = audit.pAudit;

#if MM_AUDIT_FILL_SUPPORT
    if ((mode & MM_AUDIT_PREFILL_FLAG))
    {
        oscl_memset(pMem.get() + BLOCK_HDR_SIZE, prefill_pattern, sizeIn);
    }
#endif


#if MM_AUDIT_FENCE_SUPPORT
    /* fill the pre-fence */
    MM_AllocBlockFence *pFence =
        static_cast<MM_AllocBlockFence*>(static_cast<void*>(pMem.get() +
                                         CONTROL_HEADER_SIZE));
    pFence->fill_fence();

    /* fill the post-fence */
    uint32 post_fence_offset = sizeIn + BLOCK_HDR_SIZE;
    pFence =
        static_cast<MM_AllocBlockFence*>(static_cast<void*>(pMem.get() +
                                         post_fence_offset));
    pFence->fill_fence();
#endif


    /* 8-byte alignment */
    pMem_out = pMem.get() + BLOCK_HDR_SIZE;
    MM_Stats_t delta;
    delta.numBytes = sizeIn;
    delta.numAllocs = 1;
    delta.numAllocFails = 0;
    updateStatsNode(pStatsNode, delta, true);

    pMem.release(); // release so it doesn't free up memory on exit

    // account for the per-block overhead
    mm_audit_per_block_overhead += BLOCK_OVERHEAD_SIZE;
    return pMem_out;

}

/* ============================================================================ */
/*	Function : MM_deallocate()													*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API for a memory de-allocation attempt: remove a node in			*/
/*			   allocation table and update the corresponding node in			*/
/*			   statistics table													*/
/*	In/out   : all parameters are input											*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF bool  MM_Audit_Imp::MM_deallocate(void *pMemBlockIn)
{
    if (!pMemBlockIn)  return false;

    if (mpStatsNode == NULL ||
            (mpStatsNode && mpStatsNode->pMMStats && mpStatsNode->pMMStats->peakNumBytes == 0)) /* No actual allocation happens */
    {
        return true;
    }

#if (MM_AUDIT_ALLOC_NODE_SUPPORT && MM_AUDIT_INCLUDE_ALL_HEAP_VALIDATION)
    if (mode & MM_AUDIT_VALIDATE_ALL_HEAP_FLAG)
    {
        validate_all_heap();
    }
#endif

    bool status = true;
#if MM_AUDIT_VALIDATE_BLOCK
    if (mode & MM_AUDIT_VALIDATE_ON_FREE_FLAG)
    {
        if (!validate(pMemBlockIn))
        {
            return false;
        }
    }
#endif

    uint8 *pMem = static_cast<uint8 *>(pMemBlockIn);
    pMem -= BLOCK_HDR_SIZE;

    OsclMemStatsNode *pStatsNode = NULL;
    MM_AllocBlockHdr *pMemBlockHdr =
        static_cast<MM_AllocBlockHdr*>(static_cast<void*>(pMem));
    uint32 size = pMemBlockHdr->size;

#if MM_AUDIT_ALLOC_NODE_SUPPORT
    if (size & MM_AllocBlockHdr::ALLOC_NODE_FLAG)
    {
        pStatsNode = removeAllocNode((void *)pMem, size);
        OSCL_ASSERT(pStatsNode);
    }
#endif

    if (!pStatsNode)
    {
        pStatsNode = static_cast<OsclMemStatsNode*>(pMemBlockHdr->pNode);
    }
    /**
     * 3/1. update the node in memory statistics table
     */
    MM_Stats_t delta;
    delta.numAllocs = 1;
    delta.numBytes = size;
    delta.numAllocFails = 0;
    status = updateStatsNode(pStatsNode, delta, false);

#if MM_AUDIT_FILL_SUPPORT
    if ((mode & MM_AUDIT_POSTFILL_FLAG))
    {
        oscl_memset(pMem + BLOCK_HDR_SIZE, postfill_pattern, size);
    }
#endif


    MMAuditUint8AutoPtr::deallocate(pMem);

    // account for the per-block overhead
    mm_audit_per_block_overhead -= BLOCK_OVERHEAD_SIZE;


    return status;
}


/* ============================================================================ */
/*	Function : MM_GetTreeNodes()												*/
/*	Date     : 10/31/2002														*/
/*	Purpose  : API to get the number of tree nodes including the tag node and   */
/*			   its subtree														*/
/*	In/out   : all parameters are input											*/
/*	Return   : 0 means no tag node ; >0 means the number of tree nodes			*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF uint32 MM_Audit_Imp::MM_GetTreeNodes(const char *  tagIn)
{
    int32 count = 0;

    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator iter = mTagTree.find(tag.get());

    if (iter != mTagTree.end())
    {
        count++; //to account for the parent
        count = iter->children.size();
    }
    return ((uint32)count);
}



/* ================================================================================ */
/*	Function : MM_GetStats()														*/
/*	Date     : 10/08/2002															*/
/*	Purpose  : API to get memory statistics through context string(tag)				*/
/*	In/out   : all parameters are input												*/
/*	Return   : the statistics (pointer) for the current tag node					*/
/*			   if something is wrong, return NULL									*/
/*	Modified :																		*/
/* ================================================================================ */


OSCL_EXPORT_REF MM_Stats_t* MM_Audit_Imp::MM_GetStats(const char * const  tagIn)
{
    MM_Stats_t *pMMStats = NULL;
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator iter = mTagTree.find(tag.get());

    if (iter != mTagTree.end())
    {
        /* Got it! */
        OsclMemStatsNode *pStatsNode = (iter->value).get();
        if (pStatsNode)
        {
            pMMStats = pStatsNode->pMMStats;
        }
    }
    return pMMStats;
}


OSCL_EXPORT_REF uint32 MM_Audit_Imp::MM_GetStatsInDepth(const char *tagIn,
        MM_Stats_CB *array_ptr,
        uint32 max_nodes)
{
    uint32 curr_array_index = 0;
    populateChildren(tagIn, array_ptr, curr_array_index, max_nodes);
    return (curr_array_index);
}


/* ============================================================================ */
/*	Function : getNodeChildren()												*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : recursive function to go throught each child node for the 		*/
/*			   current node and counter it										*/
/*	In/out   : all parameters are input											*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

void MM_Audit_Imp::populateChildren(const char *tagIn,
                                    MM_Stats_CB *array_ptr,
                                    uint32 &curr_array_index,
                                    uint32 max_nodes)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());

    if (curr_array_index > max_nodes)
    {
        return;
    }

    if (miter != mTagTree.end() && max_nodes > 0)
    {
        uint32 num_children = miter->children.size();

        miter->sort_children();

        array_ptr[curr_array_index].num_child_nodes = num_children;

        for (uint32 i = 0; i < num_children; i++)
        {
            array_ptr[curr_array_index].tag = (miter->tag).tag;

            OsclMemStatsNode *pStatsNode = (miter->value).get();
            if (pStatsNode)
            {
                array_ptr[curr_array_index].pStats = pStatsNode->pMMStats;
            }
            curr_array_index++;

            if (curr_array_index > max_nodes)
            {
                return;
            }
        }

        /* access its children nodes */
        if (!miter->children.empty())
        {
            /* recursive search */
            for (uint32 i = 0; i < num_children; i++)
            {
                populateChildren((miter->children[i]->tag).tag,
                                 array_ptr,
                                 curr_array_index,
                                 max_nodes);
            }
        }
    }

    return;
}

/* ============================================================================ */
/*	Function : MM_GetTag()														*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API to get an StatsNode or creates one if it doesn't exist */
/*	In/out   : all parameters are input											*/
/*	Return   : pointer to OsclMemStatsNode if operation succeeds, NULL otherwise										*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF const OsclMemStatsNode* MM_Audit_Imp::MM_GetTagNode(const char * tagIn)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator iter = mTagTree.find(tag.get());
    if (iter != mTagTree.end())
        return ((iter->value).get());
    else
        return createStatsNode(tag.get()); /* create a new empty node */
}

/* ============================================================================ */
/*	Function : MM_GetTag()														*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API to get an existing node  */
/*	In/out   : all parameters are input											*/
/*	Return   : pointer to OsclMemStatsNode if operation succeeds, NULL otherwise										*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF const OsclMemStatsNode* MM_Audit_Imp::MM_GetExistingTag(const char * tagIn)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator iter = mTagTree.find(tag.get());
    if (iter != mTagTree.end())
        return ((iter->value).get());
    else
        return NULL;
}




/* ============================================================================ */
/*	Function : MM_Validate()													*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API to check the input pointer is a valid pointer to a chunk of  */
/*			   memory															*/
/*	In/out   : all parameters are input											*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF bool MM_Audit_Imp::MM_Validate(const void *ptrIn)
{
    bool status = validate(const_cast<void*>(ptrIn));
    return status;
}

/* ============================================================================ */
/*	Function : MM_SetTagLevel()													*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : API to set the maximum tag level,i.e. tag level for a.b.c.d = 4	*/
/*	In/out   : all parameters are input											*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF void MM_Audit_Imp::MM_SetTagLevel(uint32 level)
{
    if (level >= 1) mnMaxTagLevel = level;
}


OSCL_EXPORT_REF void MM_Audit_Imp::MM_SetMode(uint32 in_mode)
{
    mode = in_mode;
}


OSCL_EXPORT_REF void MM_Audit_Imp::MM_SetPrefillPattern(uint8 pattern)
{
    prefill_pattern = pattern;
}


OSCL_EXPORT_REF void MM_Audit_Imp::MM_SetPostfillPattern(uint8 pattern)
{
    postfill_pattern = pattern;
}

OSCL_EXPORT_REF MM_AllocQueryInfo* MM_Audit_Imp::MM_CreateAllocNodeInfo(uint32 array_size)
{
    return(MM_AllocQueryInfo*)_oscl_malloc(array_size*sizeof(MM_AllocQueryInfo));
}

OSCL_EXPORT_REF void MM_Audit_Imp::MM_ReleaseAllocNodeInfo(MM_AllocQueryInfo* output_array)
{
    _oscl_free(output_array);
}

OSCL_EXPORT_REF uint32 MM_Audit_Imp::MM_GetAllocNodeInfo(MM_AllocQueryInfo* output_array,
        uint32 max_array_size, uint32 offset)
{
    uint32 num_nodes = 0;

    if (!output_array)
    {
        return 0;
    };


    if (offset >= mNumAllocNodes)
    {
        return 0;
    }

    MM_AllocNode *pAllocNode = mpAllocNode;

    // skip the leading nodes
    uint32 ii;
    for (ii = 0; pAllocNode && ii < offset; ++ii, pAllocNode = pAllocNode->pNext)
    {
    }

    if (ii != offset)
    {
        return 0;
    }

    MM_AllocQueryInfo* pOutNode = output_array;

    while (pAllocNode && (num_nodes < max_array_size))
    {

        pOutNode->allocNum = pAllocNode->pAllocInfo->allocNum;
        pOutNode->lineNo = pAllocNode->pAllocInfo->lineNo;
        pOutNode->size = pAllocNode->pAllocInfo->size;
        pOutNode->pMemBlock = pAllocNode->pAllocInfo->pMemBlock;
        pOutNode->size = pAllocNode->pAllocInfo->size;

        if (pAllocNode->pAllocInfo->pFileName)
        {
            oscl_strncpy(pOutNode->fileName, pAllocNode->pAllocInfo->pFileName,
                         oscl_strlen(pAllocNode->pAllocInfo->pFileName) + 1);
            pOutNode->fileName[MM_ALLOC_MAX_QUERY_FILENAME_LEN-1] = '\0';
        }
        else
        {
            pOutNode->fileName[0] = '\0';
        }

        if (pAllocNode->pAllocInfo->pStatsNode->tag)
        {
            oscl_strncpy(pOutNode->tag, pAllocNode->pAllocInfo->pStatsNode->tag,
                         oscl_strlen(pAllocNode->pAllocInfo->pStatsNode->tag) + 1);
            pOutNode->tag[MM_ALLOC_MAX_QUERY_TAG_LEN-1] = '\0';
        }
        else
        {
            pOutNode->tag[0] = '\0';
        }

        ++num_nodes;
        ++pOutNode;
        pAllocNode = pAllocNode->pNext;
    }
    return num_nodes;
}

/* ============================================================================ */
/*	Function : MM_SetFailurePoint()												*/
/*	Date     : 11/05/2002														*/
/*	Purpose  : API to insert allocation failure deterministically according to  */
/*			   allocation number associated with tag							*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF bool MM_Audit_Imp::MM_SetFailurePoint(const char * tagIn, uint32 alloc_number)
{
    if (alloc_number == 0) return false;

    bool status = true;
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());

    if (miter != mTagTree.end())
    {
        /* found */
        OsclMemStatsNode *pStatsNode = (miter->value).get();

        if (pStatsNode == NULL || pStatsNode->pMMFIParam == NULL)
        {
            return false;
        }
        pStatsNode->pMMFIParam->nAllocNum = alloc_number;
    }
    else
    {
        /* Needs to create a new node */
        OsclMemStatsNode *pStatsNode = createStatsNode(tag.get());
        if (pStatsNode == NULL || pStatsNode->pMMFIParam == NULL)
        {
            return false;
        }
        pStatsNode->pMMFIParam->nAllocNum = alloc_number;
    }

    return status;
}


/* ============================================================================ */
/*	Function : MM_UnsetFailurePoint()											*/
/*	Date     : 11/05/2002														*/
/*	Purpose  : API to cancel the allocation failure point associated with tag	*/
/*	In/out   : all parameters are input											*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

OSCL_EXPORT_REF void MM_Audit_Imp::MM_UnsetFailurePoint(const char * tagIn)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag); //tagIn = NULL means root tag
    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());

    if (miter != mTagTree.end())
    {
        /* found */
        OsclMemStatsNode *pStatsNode = (miter->value).get();
        if (pStatsNode && pStatsNode->pMMFIParam)
        {
            pStatsNode->pMMFIParam->nAllocNum = 0;
        }
    }
}


/*
* ===================== PRIVATE (SUPPORTING) FUNCTIONS ===========================
*/


#if MM_AUDIT_ALLOC_NODE_SUPPORT
/* ============================================================================ */
/*	Function : addAllocNode()													*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : add a node in memory allocation table triggered by an allocation	*/
/*			   attempt, i.e. MM_allocate(); then update/creat the corresponding */
/*			   node in memory statistics table									*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/

MM_AllocNode* MM_Audit_Imp::addAllocNode(void *pMemBlockIn, uint32 sizeIn,
        OsclMemStatsNode *pStatsNode,
        const char *pFileName, uint32 lineNumber
                                        )
{
    uint32 tmp_overhead_size = 0;

    /**
     * 1. allocate memory for a new node
     */

    MM_AllocNodeAutoPtr currAllocAutoPtr(new MM_AllocNode);
    if (! currAllocAutoPtr.get())
    {
        return NULL;
    }
    tmp_overhead_size += sizeof(MM_AllocNode);

    MM_AllocInfo* pAllocInfo = currAllocAutoPtr->pAllocInfo = new MM_AllocInfo;
    if (pAllocInfo == NULL)
    {
        return NULL;
    }
    tmp_overhead_size += sizeof(MM_AllocInfo);

    // allocate space for the filename in AllocInfo
    if (pFileName)
    {
        int len = oscl_strlen(pFileName);
        Oscl_TAlloc<char, OsclMemBasicAllocator> charAlloc;
        if ((pAllocInfo->pFileName =
                    charAlloc.allocate(len + 1)) == NULL)
        {
            return NULL;
        }
        oscl_strncpy(const_cast<char*>(pAllocInfo->pFileName), pFileName, oscl_strlen(pFileName) + 1);
        tmp_overhead_size += len + 1;
    }

    pAllocInfo->allocNum  = mnAllocNum;

    //To find memory leaks by allocation number, put a debug breakpoint here, like this:
    //if(mnAllocNum==113)
    //{
    //  mnAllocNum++;
    //  mnAllocNum--;
    //}

    //To find allocations with a NULL filename, put a debug breakpoint here, like this:
    //if(pFileName==NULL)
    //{
    //  mnAllocNum++;
    //  mnAllocNum--;
    //}

    mnAllocNum++;

    pAllocInfo->pMemBlock = pMemBlockIn;
    pAllocInfo->size	  = sizeIn;
    pAllocInfo->lineNo  = lineNumber;
    pAllocInfo->pStatsNode = pStatsNode;

    if (!mpCurrAllocNode)  /* First node */
    {
        /* mpAllocNode always represents the very first node */
        mpAllocNode = mpCurrAllocNode = currAllocAutoPtr.release();
    }
    else
    {
        OSCL_ASSERT(!mpCurrAllocNode->pNext);
        mpCurrAllocNode->pNext = currAllocAutoPtr.release();
        currAllocAutoPtr->pPrev = mpCurrAllocNode;
        mpCurrAllocNode = mpCurrAllocNode->pNext;
    }


    /* increment the list counter */
    ++mNumAllocNodes;

    mm_audit_per_block_overhead += tmp_overhead_size;
    return mpCurrAllocNode;

}

/* ============================================================================ */
/*	Function : removeAllocNode()												*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : remove a node in memory allocation table triggered by an			*/
/*			   de-allocationattempt, i.e. MM_deallocate()						*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/

OsclMemStatsNode* MM_Audit_Imp::removeAllocNode(void *pMemBlockIn, uint32& size)
{

    if (pMemBlockIn == NULL)
        return NULL;

    OsclMemStatsNode *pStatsNode = NULL;

    OSCL_ASSERT(mpAllocNode);
    OSCL_ASSERT(mpCurrAllocNode);
    OSCL_ASSERT(mNumAllocNodes);

    /* take the allocation node pointer from the control info header */
    MM_AllocBlockHdr *pMemBlockHdr = static_cast<MM_AllocBlockHdr*>(pMemBlockIn);
    MM_AllocNode *pAllocNode = static_cast<MM_AllocNode*>(pMemBlockHdr->pNode);
    if (!pAllocNode)  return NULL;

    /*
     * 1. remove the current node from the list
     */
    if (pAllocNode == mpAllocNode)
    {
        /* head of the list*/
        mpAllocNode = mpAllocNode->pNext;
        if (mpAllocNode)
        {
            mpAllocNode->pPrev = NULL;
        }
        else
        {
            // list is empty
            mpCurrAllocNode = mpAllocNode = NULL;
        }
    }
    else if (pAllocNode == mpCurrAllocNode) /* end of the list */
    {
        mpCurrAllocNode = mpCurrAllocNode->pPrev;
        if (mpCurrAllocNode)
        {
            mpCurrAllocNode->pNext = NULL;
        }
        else
        {
            // list is empty
            mpCurrAllocNode = mpAllocNode = NULL;
        }
    }
    else   /* somewhere in the list */
    {
        MM_AllocNode *pPrevNode = pAllocNode->pPrev;
        MM_AllocNode *pNextNode = pAllocNode->pNext;

        OSCL_ASSERT(pPrevNode && pNextNode);

        pPrevNode->pNext = pNextNode;
        pNextNode->pPrev = pPrevNode;
    }

    /**
     * 2. free the memory for the current node
     */
    OSCL_ASSERT(pAllocNode->pAllocInfo);
    pStatsNode = pAllocNode->pAllocInfo->pStatsNode;
    size = pAllocNode->pAllocInfo->size;

    // adjust the overhead accounting
    uint32 filename_len = (pAllocNode->pAllocInfo->pFileName) ? oscl_strlen(pAllocNode->pAllocInfo->pFileName) + 1 : 0;
    mm_audit_per_block_overhead -= sizeof(MM_AllocNode) + sizeof(MM_AllocInfo) +
                                   filename_len;


    OSCL_DELETE(pAllocNode);



    /*
     * decrement the list counter
     */
    --mNumAllocNodes;




    return pStatsNode;

}

/* ============================================================================ */
/*	Function : removeALLAllocNodes()											*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : remove all the nodes in memory allocation table needed in 		*/
/*			   destructor														*/
/*	In/out   :																	*/
/*	Return   :																	*/
/*	Modified :																	*/
/* ============================================================================	*/

void MM_Audit_Imp::removeALLAllocNodes()
{

    while (mpCurrAllocNode)
    {
        MM_AllocNode *pTmpNode = mpCurrAllocNode;
        mpCurrAllocNode = mpCurrAllocNode->pPrev;
        if (mpCurrAllocNode)
            mpCurrAllocNode->pNext = NULL;
        --mNumAllocNodes;
        OSCL_DELETE(pTmpNode);
    }
}


bool MM_Audit_Imp::validate_all_heap()
{

    // walk the list of allocated nodes
    MM_AllocNode *ptr = mpAllocNode;

    if (!ptr)
    {
        OSCL_ASSERT(mNumAllocNodes == 0);
        return true;
    }

    uint32 counter;

    OSCL_ASSERT(ptr->pPrev == NULL);
    for (counter = 0; ptr; ptr = ptr->pNext, ++counter)
    {
        MM_AllocBlockHdr *pMemBlockHdr = static_cast<MM_AllocBlockHdr*>(ptr->pAllocInfo->pMemBlock);

        OSCL_ASSERT(pMemBlockHdr->pNode == (void *)ptr);
        if (ptr->pNext)
        {
            OSCL_ASSERT(ptr->pNext->pPrev == ptr);
        }
        else
        {
            OSCL_ASSERT(ptr == mpCurrAllocNode);
        }

        uint8* pMem = static_cast<uint8*>(static_cast<void*>(pMemBlockHdr));
        pMem += BLOCK_HDR_SIZE;
        bool status = validate(pMem);
        OSCL_ASSERT(status);
        if (!status)
        {
            return false;
        }
    }

    OSCL_ASSERT(counter == mNumAllocNodes);

    return true;

}


#endif // #if MM_AUDIT_ALLOC_NODE_SUPPORT

/* ============================================================================ */
/*	Function : validate()														*/
/*	Date     : 11/05/2002														*/
/*	Purpose  : validate the input pointer to a chunk of memory					*/
/*	In/out   : input the memory pointer						*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/

bool MM_Audit_Imp::validate(void *ptrIn)
{
    if (!ptrIn) return false;

    uint8 *pMem = static_cast<uint8*>(const_cast<void*>(ptrIn));

    pMem -= BLOCK_HDR_SIZE;

    MM_AllocBlockHdr *pMemBlockHdr =
        static_cast<MM_AllocBlockHdr*>((void *)pMem);

    OSCL_ASSERT(pMemBlockHdr->pNode);
    if (!pMemBlockHdr->pNode)
    {
        return false;
    }


    OsclMemStatsNode *pStatsNode = NULL;
#if MM_AUDIT_ALLOC_NODE_SUPPORT
#if MM_AUDIT_FENCE_SUPPORT
    uint32 size = pMemBlockHdr->size;
#endif
    if (pMemBlockHdr->isAllocNodePtr())
    {
        MM_AllocNode *pAllocNode = NULL;
        pAllocNode = static_cast<MM_AllocNode*>(pMemBlockHdr->pNode);
        OSCL_ASSERT(pAllocNode->pAllocInfo);
#if MM_AUDIT_FENCE_SUPPORT
        OSCL_ASSERT(((size ^ pAllocNode->pAllocInfo->size) & (~MM_AllocBlockHdr::ALLOC_NODE_FLAG)) == 0);
        size = pAllocNode->pAllocInfo->size;
#endif
        pStatsNode =  pAllocNode->pAllocInfo->pStatsNode;
    }
#endif

    if (!pStatsNode)
    {
        // must be a stats node
        pStatsNode = static_cast<OsclMemStatsNode*>(pMemBlockHdr->pNode);
    }

    OSCL_ASSERT(pStatsNode->pMMStats);

#if MM_AUDIT_FENCE_SUPPORT
    bool status;

    // check the pre-fence
    MM_AllocBlockFence *fence =
        static_cast<MM_AllocBlockFence*>(static_cast<void*>(pMem +
                                         CONTROL_HEADER_SIZE));
    status = fence->check_fence();
    if (!status)
    {
        OSCL_ASSERT(status);
        return status;
    }

    // check the post fence
    fence =
        static_cast<MM_AllocBlockFence*>(static_cast<void*>(pMem +
                                         BLOCK_HDR_SIZE
                                         + size));
    status = fence->check_fence();
    if (!status)
    {
        OSCL_ASSERT(status);
        return status;
    }
#endif

    return true;
}

/** Retrieve the audit root pointer from within an allocation block*/
OsclMemAudit * MM_Audit_Imp::getAuditRoot(void *ptrIn)
{
    if (!ptrIn) return false;

    uint8 *pMem = static_cast<uint8*>(const_cast<void*>(ptrIn));

    pMem -= BLOCK_HDR_SIZE;

    MM_AllocBlockHdr *pMemBlockHdr =
        static_cast<MM_AllocBlockHdr*>((void *)pMem);

    return (OsclMemAudit*)pMemBlockHdr->pRootNode;
}

uint32 MM_Audit_Imp::getSize(void *ptrIn)
{//get original allocation size.

    if (!ptrIn)
        return 0;

    uint8 *pMem = static_cast<uint8*>(const_cast<void*>(ptrIn));

    pMem -= BLOCK_HDR_SIZE;

    MM_AllocBlockHdr *pMemBlockHdr = static_cast<MM_AllocBlockHdr*>((void *)pMem);

    //if it's an alloc node we have to strip out the alloc node bit
    //from the size field.
    if (pMemBlockHdr->isAllocNodePtr())
    {
        return pMemBlockHdr->size & ~MM_AllocBlockHdr::ALLOC_NODE_FLAG;
    }
    else
    {
        return pMemBlockHdr->size;
    }
}


/* ============================================================================ */
/*	Function : createStatsNode()												*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : creat a new node in memory statistics table triggered by an		*/
/*			   allocation attempt,												*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/


OsclMemStatsNode* MM_Audit_Imp::createStatsNode(const char *  tagIn)
{
    OsclTagTreeType::iterator iter;
    MMAuditCharAutoPtr currentTag;
    OsclMemStatsNode* statsNode;

    /* If the input tag already exists in the tagtree, it should have a pointer value of NULL */
    OSCL_ASSERT((mTagTree.find(const_cast<char* const&>(tagIn)) == mTagTree.end()) ||
                (mTagTree.find(const_cast<char* const&>(tagIn)))->value.get() == 0);

    statsNode = new OsclMemStatsNode;
    if (statsNode == NULL) return NULL;

    currentTag.allocate(oscl_strlen(tagIn) + 1);
    if (!currentTag.get()) return NULL;
    oscl_strncpy(currentTag.get(), tagIn, oscl_strlen(tagIn) + 1);
    statsNode->tag = currentTag.release();

    if ((statsNode->pMMStats = new MM_Stats_t) == NULL) return NULL;

    if ((statsNode->pMMFIParam = new MM_FailInsertParam) == NULL) return NULL;

    OsclMemStatsNodeAutoPtr statsNodeAutoPtr(statsNode);

    // reassign ownership of the StatsNode to the tag tree
    mTagTree[statsNodeAutoPtr->tag] = statsNodeAutoPtr;

    // account for the overhead memory
    mm_audit_stats_overhead += sizeof(MM_Stats_t) +
                               sizeof(MM_FailInsertParam) +
                               sizeof(OsclMemStatsNode) +
                               oscl_strlen(currentTag.get()) + 1;

    // how many levels deep is the node we just inserted?
    iter = mTagTree.find(statsNodeAutoPtr->tag);
    uint32 depth = iter->depth();

    // the tag tree will automatically create the parent, grandparent, etc.
    // make sure each ancestor's stats node is initialized, i.e. initialize each ancestor
    // until you reach one that is already initialized.
    OsclTagTreeType::node_ptr parent = iter->parent;
    for (uint32 ii = 0; ii < depth; ii++)
    {

        OSCL_ASSERT(parent != 0);

        // if initialized then we're done
        OsclMemStatsNode* tmpStatsNode = (parent->value).get();
        if (tmpStatsNode != NULL) break;

        // create new stats node
        tmpStatsNode = new OsclMemStatsNode;
        if (tmpStatsNode == NULL) return NULL;

        // copy tag already created by the tag tree
        currentTag.allocate(oscl_strlen(parent->tag.tag) + 1);
        if (!currentTag.get()) return NULL;
        oscl_strncpy(currentTag.get(), parent->tag.tag, oscl_strlen(parent->tag.tag) + 1);
        tmpStatsNode->tag = currentTag.release();

        if ((tmpStatsNode->pMMStats = new MM_Stats_t) == NULL) return NULL;

        if ((tmpStatsNode->pMMFIParam = new MM_FailInsertParam) == NULL) return NULL;

        // set the new stats node to be held by the tag tree
        parent->value.takeOwnership(tmpStatsNode);

        // account for the overhead memory
        mm_audit_stats_overhead += sizeof(OsclMemStatsNode) +
                                   oscl_strlen(currentTag.get()) + 1 +
                                   sizeof(MM_Stats_t) +
                                   sizeof(MM_FailInsertParam);

        parent = parent->parent;
    }

    return statsNode;
}


/* ============================================================================ */
/*	Function : updateStatsNode()												*/
/*	Date     : 10/08/2002														*/
/*	Purpose  : update the node in memory statistics table triggered by an		*/
/*			   allocation/de-alocation attempt,									*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/


bool MM_Audit_Imp::updateStatsNode(OsclMemStatsNode *pCurrStatsNode, const MM_Stats_t& delta, bool add)
{
    MMAuditCharAutoPtr tag;
    makeValidTag((const char*)(pCurrStatsNode->tag), tag);

    //Update
    if (!pCurrStatsNode->pMMStats) return false;
    pCurrStatsNode->pMMStats->update(delta, add);

    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());

    uint32 depth = miter->depth();

    OsclTagTreeType::node_ptr parent = miter->parent;

    for (uint32 i = 0; i < depth; i++)
    {
        if (!parent->value->pMMStats) return false;
        parent->value->pMMStats->update(delta, add);
        parent = miter->parent;
    }

    return true;
}


/* ============================================================================ */
/*	Function : updateStatsNodeInFailure()										*/
/*	Date     : 11/05/2002														*/
/*	Purpose  : update "pMMStats->numAllocFails" for the current tag node with 	*/
/*			   its parent tag nodes												*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/
bool MM_Audit_Imp::updateStatsNodeInFailure(const char * tagIn)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag);
    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());
    OsclMemStatsNode *pStatsNode;
    if (miter != mTagTree.end())
    {/* found */
        pStatsNode = (miter->value).get();
    }
    else
    {
        pStatsNode = mpStatsNode; // update the root node
    }

    return updateStatsNodeInFailure(pStatsNode);
}

bool MM_Audit_Imp::updateStatsNodeInFailure(OsclMemStatsNode * pStatsNode)
{
    MMAuditCharAutoPtr tag;

    if (!pStatsNode) return false;

    makeValidTag((const char*)(pStatsNode->tag), tag);

    if (!pStatsNode->pMMStats) return false;
    pStatsNode->pMMStats->numAllocFails++;

    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());

    uint32 depth = miter->depth();

    OsclTagTreeType::node_ptr parent = miter->parent;

    for (uint32 i = 0; i < depth; i++)
    {
        if (!parent->value->pMMStats) return false;
        parent->value->pMMStats->numAllocFails++;
        parent = miter->parent;
    }

    return true;
}

/* ============================================================================ */
/*	Function : isSetFailure()													*/
/*	Date     : 11/05/2002														*/
/*	Purpose  : do allocation failure check										*/
/*	In/out   : all parameters are input											*/
/*	Return   : true if operation succeeds										*/
/*	Modified :																	*/
/* ============================================================================	*/

bool MM_Audit_Imp::isSetFailure(const char * tagIn)
{
    MMAuditCharAutoPtr tag;
    makeValidTag(tagIn, tag);
    OsclTagTreeType::iterator miter = mTagTree.find(tag.get());
    if (miter != mTagTree.end()) /* found */
    {
        OsclMemStatsNode *pStatsNode = (miter->value).get();
        return isSetFailure(pStatsNode);
    }

    return false;
}

bool MM_Audit_Imp::isSetFailure(OsclMemStatsNode* pStatsNode)
{
    if (pStatsNode == NULL) return false;

    if (pStatsNode->pMMFIParam == NULL) return false;

    /* decision for deterministic failure insertion */
    if (pStatsNode->pMMFIParam->nAllocNum > 0)
    {
        /* the "+ 1" term in the computation of total_allocs is to
         * count the current allocation
         */
        uint32 total_allocs = pStatsNode->pMMStats->peakNumAllocs + 1 +
                              pStatsNode->pMMStats->numAllocFails ;

        if (total_allocs == pStatsNode->pMMFIParam->nAllocNum)
            return true;
    }

    return false;
}
/* ========================================================================================== */
/*	Function : retrieveParentTagLength()													  */
/*	Date     : 10/16/2002																	  */
/*	Purpose  : get the length of the parent tag(i.e."a.b.c") of the input tag(i.e. "a.b.c.d") */
/*			   current node and counter it													  */
/*	In/out   : all parameters are input														  */
/*	Return   : length of the parent tag(sub-string)	of an input tag							  */
/*	Modified :																				  */
/* ========================================================================================== */

int32 MM_Audit_Imp::retrieveParentTagLength(const char *tag, int32 bound)
{
    if (!tag)  return 0;

#define PV_MIN(a,b)	  ((a)<(b)? (a):(b))

    int count = 0;
    int len = PV_MIN((int32)(oscl_strlen(tag)), bound);
    if (len <= 0)
        return 0;
    else
    {
        bool bFound = false;
        for (count = len - 1; count >= 0; count--)
        {
            if (tag[count] == '.')
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)	 count = 0;
    }

    return count + 1;
}

void MM_Audit_Imp::retrieveParentTag(char *tag)
{
    if (!tag)  return;

    int32 len = oscl_strlen(tag);
    if (len == 1)
        tag[0] = '\0';
    else
    {
        bool bFound = false;
        for (int32 i = len - 1; i >= 0; i--)
        {
            if (tag[i] == '.')
            {
                tag[i] = '\0';
                bFound = true;
                break;
            }
        }
        if (!bFound)
            tag[0] = '\0';
    }
}

/* ========================================================================================== */
/*	Function : makeValidTag()																  */
/*	Date     : 10/25/2002																	  */
/*	Purpose  : check the input tag and make sure its level would be bounded in maximum tag    */
/*			   level, if its level is larger, then truncate it								  */
/*			   Note that level of "a.b.c.d" = 4												  */
/*	In/out   : In: tagIn ; Out: *bFree														  */
/*	Return   : a valid tag within the maximum tag level constraint							  */
/*	Modified :																				  */
/* ========================================================================================== */

void MM_Audit_Imp::makeValidTag(const char * tagIn, MMAuditCharAutoPtr& autoptr)
{

    if (tagIn == NULL)
    {
        //tagIn = NULL means root tag
        autoptr.setWithoutOwnership(const_cast<char*>(root_tag));
    }

    else
    {
        uint32 len = getTagActualSize(tagIn);
        if (len == 0)
        {
            /* len = 0 meaning no need of truncation */
            autoptr.setWithoutOwnership(const_cast<char*>(tagIn));
        }

        else
        {
            autoptr.allocate(len + 1);
            oscl_strncpy(autoptr.get(), tagIn, len);
            *(autoptr.get() + len) = '\0';
        }
    }
}

/* ========================================================================================== */
/*	Function : getTagActualSize()															  */
/*	Date     : 10/25/2002																	  */
/*	Purpose  : get the actual size of an input tag within the maximum tag level constraint    */
/*			   Note that level of "a.b.c.d" = 4												  */
/*	In/out   : In: tagIn ; Out: *bFree														  */
/*	Return   : 0 means no truncation ; >0 means truncated size								  */
/*	Modified :																				  */
/* ========================================================================================== */

uint32 MM_Audit_Imp::getTagActualSize(const char * tagIn)
{
    uint32 i, len;
    uint32 level = 0, count = 0;

    len = oscl_strlen(tagIn);
    if (len <= 2*mnMaxTagLevel - 1)
        return 0; /* no truncation */

    for (i = 0; i < len; i++)
    {
        if (tagIn[i] == '.')
        {
            if (++level == mnMaxTagLevel)
            {
                count = i;
                break;
            }
        }
    }
    return count;
}


#endif //if OSCL_BYPASS_MEMMGT



