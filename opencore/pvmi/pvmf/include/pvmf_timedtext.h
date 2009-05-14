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
 *  @file pvmf_timedtext.h
 *  @brief This file defines structures/utilities specific to timed text media
 *
 */

#ifndef PVMF_TIMEDTEXT_H_INCLUDED
#define PVMF_TIMEDTEXT_H_INCLUDED

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

const PVUid32 PVMFTimedTextFormatSpecificInfo_UID = 0x1;

#define PVMFTIMEDTEXT_TRANSLATION_MATRIXSIZE 9
#define PVMFTIMEDTEXT_RGBA_ARRAYSIZE 4

/**
 * Timed text track level parameters that would be transmitted as
 * format specific info
 **/
struct PVMFTimedTextFormatSpecificInfo
{
    PVUid32 iUID32;
    int16 iLayer;
    int32 iTranslationMatrix[PVMFTIMEDTEXT_TRANSLATION_MATRIXSIZE];
    uint32 iWidth;
    uint32 iHeight;
};

/**
 * Timed text font record entry that describes the font used
 * in the text sample entry
 **/
struct PVMFTimedTextFontRecord
{
    uint16 iFontID;
    uint8 iFontNameLength;
    uint8* iFontName;
};

#define PVMF_TIMED_TEXT_UTF_16_MARKER_BYTE_1 0xFE
#define PVMF_TIMED_TEXT_UTF_16_MARKER_BYTE_2 0xFF

enum PVMFTimedTextStringFormatType
{
    PVMF_TIMED_TEXT_STRING_FORMAT_UNKNOWN,
    PVMF_TIMED_TEXT_STRING_FORMAT_UTF8,
    PVMF_TIMED_TEXT_STRING_FORMAT_UTF16,
    PVMF_TIMED_TEXT_STRING_FORMAT_UTF16_LE
};


/**
 * Timed text sample entry that describes the associated
 * text sample
 **/
class PVMFTimedTextSampleEntry
{
    public:
        PVMFTimedTextSampleEntry()
        {
            iDisplayFlags = 0;
            iHorizontalJustification = 0;
            iVerticalJustification = 0;
            oscl_memset(iBackgroundRGBA, 0, PVMFTIMEDTEXT_RGBA_ARRAYSIZE);

            iBoxTop = 0;
            iBoxLeft = 0;
            iBoxBottom = 0;
            iBoxRight = 0;

            iStyleStartChar = 0;
            iStyleEndChar = 0;
            iStyleFontID = 0;
            iStyleFontStyleFlags = 0;
            iStyleFontSize = 0;
            oscl_memset(iStyleTextColorRGBA, 0, PVMFTIMEDTEXT_RGBA_ARRAYSIZE);

            iFontEntryCount = 0;
            iFontRecordList = NULL;
        }

        ~PVMFTimedTextSampleEntry()
        {
            if (iFontRecordList)
            {
                for (int32 i = 0; i < iFontEntryCount; ++i)
                {
                    if (iFontRecordList[i].iFontName != NULL)
                    {
                        OSCL_ARRAY_DELETE(iFontRecordList[i].iFontName);
                        iFontRecordList[i].iFontName = NULL;
                    }
                }

                OSCL_ARRAY_DELETE(iFontRecordList);
                iFontRecordList = NULL;
            }
        }

        // Text sample entry info
        uint32 iDisplayFlags;
        int8 iHorizontalJustification;
        int8 iVerticalJustification;
        uint8 iBackgroundRGBA[PVMFTIMEDTEXT_RGBA_ARRAYSIZE];

        // Box record info
        int16 iBoxTop;
        int16 iBoxLeft;
        int16 iBoxBottom;
        int16 iBoxRight;

        // Style record info
        uint16 iStyleStartChar;
        uint16 iStyleEndChar;
        uint16 iStyleFontID;
        uint8 iStyleFontStyleFlags;
        uint8 iStyleFontSize;
        uint8 iStyleTextColorRGBA[PVMFTIMEDTEXT_RGBA_ARRAYSIZE];

        // Font table
        uint16 iFontEntryCount;
        PVMFTimedTextFontRecord* iFontRecordList;
};

/**
 * Container class transported in PVMFMediaData that contains
 * a shared pointer to the text sample entry object and a pointer
 * to the text sample data
 **/
class PVMFTimedTextMediaData
{
    public:
        PVMFTimedTextMediaData()
        {
            iTextSample = NULL;
            iFormatType = PVMF_TIMED_TEXT_STRING_FORMAT_UNKNOWN;
            iTextStringLengthInBytes = 0;
            iTextSampleLength = 0;
            iTextSampleCapacity = 0;
            iTextSampleDuration = 0;
        }

        ~PVMFTimedTextMediaData()
        {
            iTextSampleEntry.Unbind();
        }

        // Shared pointer to a text sample entry object
        // Use the GetRep() method to check for and retrieve the pointer
        // to PVMFTimedTextSampleEntry object
        OsclSharedPtr<PVMFTimedTextSampleEntry> iTextSampleEntry;

        // Text sample data
        // Pointer to the text sample data
        uint8* iTextSample;
        // length in bytes of actual text string
        uint32 iTextStringLengthInBytes;
        // Length of the valid text sample data including the text string and the modifiers
        // in order to calculate the text sample modifier length, do :
        // modifierLength = (iTextSampleLength - iTextStringLengthInBytes)
        uint32 iTextSampleLength;
        // Buffer capacity for iTextSample pointer. Can be equal to or larger than iTextSampleLength
        uint32 iTextSampleCapacity;
        // Sample duration in media timescale
        uint32 iTextSampleDuration;
        // Sample format
        PVMFTimedTextStringFormatType iFormatType;
        // Timestamp for the text sample in NPT (normal playback time) in milliseconds
        // MIGHT BE DEPRECATED IN THE FUTURE SINCE NOT MAINTAINABLE.
        PVMFTimestamp iTextSampleTimestampNPT;
};

/**
 * The PVMFTimedTextMediaDataCleanup deallocator class
 * takes care of calling the destructor for PVMFTimedTextMediaData
 * before freeing the memory
 */

class PVMFTimedTextMediaDataCleanup :  public OsclDestructDealloc
{
    public:
        PVMFTimedTextMediaDataCleanup(PVMFTimedTextMediaData& textmediadata, Oscl_DefAlloc* in_gen_alloc = NULL) :
                iTextMediaData(&textmediadata), iGenAlloc(in_gen_alloc) {};
        virtual ~PVMFTimedTextMediaDataCleanup() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // Call the text media data's destructor
            iTextMediaData->~PVMFTimedTextMediaData();

            if (!iGenAlloc)
            {
                OsclMemAllocator my_alloc;
                my_alloc.deallocate(ptr);
            }
            else
            {
                iGenAlloc->deallocate(ptr);
            }
        };

    private:
        PVMFTimedTextMediaData* iTextMediaData;
        Oscl_DefAlloc* iGenAlloc;
};

/**
 * The PVMFTimedTextMediaDataAlloc allocator class
 * takes care of allocating the refcounter, PVMFSimpleMediaBuffer container,
 * PVMFTimedTextMediaData object, and the actual buffer space for the text sample in a single block of memory.
 */

class PVMFTimedTextMediaDataAlloc
{
    public:
        PVMFTimedTextMediaDataAlloc(OsclMemPoolResizableAllocator* in_gen_alloc)
        {
            if (in_gen_alloc)
            {
                gen_alloc = in_gen_alloc;
                iBufferOverhead = 0;
                uint32 aligned_refcnt_size =
                    oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
                uint32 aligned_cleanup_size =
                    oscl_mem_aligned_size(sizeof(PVMFTimedTextMediaDataCleanup));
                uint32 aligned_simplemb_size =
                    oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
                uint32 aligned_textmediadata_size =
                    oscl_mem_aligned_size(sizeof(PVMFTimedTextMediaData));
                iBufferOverhead = aligned_refcnt_size +
                                  aligned_cleanup_size +
                                  aligned_simplemb_size +
                                  aligned_textmediadata_size;

            }
            else
            {
                OSCL_LEAVE(OsclErrArgument);
            }
        };

        virtual ~PVMFTimedTextMediaDataAlloc()
        {
        };

        OsclSharedPtr<PVMFMediaDataImpl> allocate(uint32 requested_size)
        {
            uint32 aligned_refcnt_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterDA));

            uint32 aligned_cleanup_size =
                oscl_mem_aligned_size(sizeof(PVMFTimedTextMediaDataCleanup));

            uint32 aligned_simplemb_size =
                oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));

            uint32 aligned_textmediadata_size =
                oscl_mem_aligned_size(sizeof(PVMFTimedTextMediaData));

            uint32 aligned_requested_size =
                oscl_mem_aligned_size(requested_size);

            uint32 totalmem_size = aligned_refcnt_size +
                                   aligned_cleanup_size +
                                   aligned_simplemb_size +
                                   aligned_textmediadata_size +
                                   aligned_requested_size;

            // Allocate the memory
            uint8* mem_ptr = NULL;
            if (!gen_alloc)
            {
                OsclMemAllocator my_alloc;
                mem_ptr = (uint8*) my_alloc.allocate(totalmem_size);
            }
            else
            {
                mem_ptr = (uint8*) gen_alloc->allocate(totalmem_size);
            }

            // Memory map
            // | RefCtr | Cleanup | PVMFSimpleMediaBuffer | PVMFTimedTextMediaData | Text sample data |

            // Create the text media data
            PVMFTimedTextMediaData* textmediadata_ptr = OSCL_PLACEMENT_NEW(mem_ptr + aligned_refcnt_size + aligned_cleanup_size + aligned_simplemb_size, PVMFTimedTextMediaData());
            textmediadata_ptr->iTextSample = mem_ptr + aligned_refcnt_size + aligned_cleanup_size + aligned_simplemb_size + aligned_textmediadata_size;
            textmediadata_ptr->iTextSampleLength = 0;
            textmediadata_ptr->iTextSampleCapacity = aligned_requested_size;

            // Create the cleanup object
            PVMFTimedTextMediaDataCleanup* cleanup_ptr = OSCL_PLACEMENT_NEW(mem_ptr + aligned_refcnt_size, PVMFTimedTextMediaDataCleanup(*textmediadata_ptr, gen_alloc));

            // Create the refcount object
            OsclRefCounter* my_refcnt = OSCL_PLACEMENT_NEW(mem_ptr, OsclRefCounterDA(mem_ptr, cleanup_ptr));

            // Create the simple media buffer
            PVMFMediaDataImpl* media_data_ptr = OSCL_PLACEMENT_NEW(mem_ptr + aligned_refcnt_size + aligned_cleanup_size, PVMFSimpleMediaBuffer((OsclAny*)textmediadata_ptr, aligned_textmediadata_size + aligned_requested_size, my_refcnt));
            media_data_ptr->setMediaFragFilledLen(0, aligned_textmediadata_size + aligned_requested_size);

            // Return as shared pointer
            OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr, my_refcnt);
            return shared_media_data;
        }

        void ResizeMemoryFragment(OsclSharedPtr<PVMFMediaDataImpl>& aSharedBuffer)
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
                OsclMemPoolResizableAllocator* dataAllocator =
                    reinterpret_cast<OsclMemPoolResizableAllocator*>(gen_alloc);
                /* Account for the overhead */
                uint8* memFragPtr = (uint8*)(memFrag.getMemFragPtr());
                uint8* ptr = (memFragPtr - iBufferOverhead);
                dataAllocator->trim((OsclAny*)ptr, bytesToReclaim);
                aSharedBuffer->setCapacity(alignedBytesUsed);
            }
        }

    private:
        uint32 iBufferOverhead;
        OsclMemPoolResizableAllocator* gen_alloc;
};


/**
 * The PVMFTimedTextSampleEntryCleanupSA deallocator class
 * takes care of calling the destructor for PVMFTimedTextSampleEntry
 * before freeing the memory using a static allocator
 */

class PVMFTimedTextSampleEntryCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~PVMFTimedTextSampleEntryCleanupSA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<PVMFTimedTextSampleEntryCleanupSA>));
            tmp_ptr += aligned_refcnt_size;
            PVMFTimedTextSampleEntry* tse_ptr = reinterpret_cast<PVMFTimedTextSampleEntry*>(tmp_ptr);
            tse_ptr->~PVMFTimedTextSampleEntry();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

/**
 * The PVMFTimedTextSampleEntryCleanupDA deallocator class
 * takes care of calling the destructor for PVMFTimedTextSampleEntry
 * before freeing the memory using a passed-in allocator
 */

class PVMFTimedTextSampleEntryCleanupDA : public OsclDestructDealloc
{
    public:
        PVMFTimedTextSampleEntryCleanupDA(Oscl_DefAlloc& in_gen_alloc) :
                gen_alloc(&in_gen_alloc) {};
        virtual ~PVMFTimedTextSampleEntryCleanupDA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint32 aligned_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
            // skip the refcounter
            tmp_ptr += aligned_size;
            // skip the cleanup
            aligned_size = oscl_mem_aligned_size(sizeof(PVMFTimedTextSampleEntryCleanupDA));
            tmp_ptr += aligned_size;

            PVMFTimedTextSampleEntry* tse_ptr = reinterpret_cast<PVMFTimedTextSampleEntry*>(tmp_ptr);
            tse_ptr->~PVMFTimedTextSampleEntry();
            gen_alloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};


/**
 * The PVMFTimedTextSampleEntryUtil allocator utility class
 * takes care of creating a shared pointer for PVMFTimedTextSampleEntry including
 * the refcounter, cleanup object if allocator is passed-in, and the PVMFTimedTextSampleEntry object
 */

class PVMFTimedTextSampleEntryUtil
{
    public:
        PVMFTimedTextSampleEntryUtil() {};
        ~PVMFTimedTextSampleEntryUtil() {};

        static OsclSharedPtr<PVMFTimedTextSampleEntry> CreatePVMFTimedTextSampleEntry(Oscl_DefAlloc* gen_alloc = NULL)
        {
            // Allocate enough room
            uint8* my_ptr = NULL;
            OsclRefCounter* my_refcnt;
            uint32 aligned_tse_size = oscl_mem_aligned_size(sizeof(PVMFTimedTextSampleEntry));

            // Must compute the aligned size for PVMFTimedTextSampleEntry.
            if (gen_alloc)
            {
                // Memory map
                // | RefCtr | Cleanup | PVMFTimedTextSampleEntry |

                uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
                uint32 aligned_cleanup_size = oscl_mem_aligned_size(sizeof(PVMFTimedTextSampleEntryCleanupDA));
                my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size + aligned_cleanup_size + aligned_tse_size);

                PVMFTimedTextSampleEntryCleanupDA* my_cleanup = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PVMFTimedTextSampleEntryCleanupDA(*gen_alloc));
                my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));
                my_ptr += aligned_refcnt_size + aligned_cleanup_size;
            }
            else
            {
                // Memory map
                // | RefCtr | PVMFTimedTextSampleEntry |

                OsclMemAllocator my_alloc;
                uint32 aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<PVMFTimedTextSampleEntryCleanupSA>));
                my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size + aligned_tse_size);
                my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<PVMFTimedTextSampleEntryCleanupSA>(my_ptr));
                my_ptr += aligned_refcnt_size;
            }

            PVMFTimedTextSampleEntry* tse_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFTimedTextSampleEntry());

            OsclSharedPtr<PVMFTimedTextSampleEntry> shared_tse(tse_ptr, my_refcnt);
            return shared_tse;
        }
};

#endif

