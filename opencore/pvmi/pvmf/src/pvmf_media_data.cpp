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

#include "pvmf_media_data.h"
#include "pvmf_media_msg_header.h"
#include "pvmf_media_data_impl.h"
#include "oscl_mem.h"


OSCL_EXPORT_REF PVMFTimestamp PVMFMediaData::getTimestamp()
{
    return hdr_ptr->timestamp;
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getDuration()
{
    return hdr_ptr->duration;
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getStreamID()
{
    return hdr_ptr->stream_id;
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getSeqNum()
{
    return hdr_ptr->seqnum;
}

OSCL_EXPORT_REF PVUid32 PVMFMediaData::getFormatID()
{
    return hdr_ptr->format_id;
}

OSCL_EXPORT_REF bool PVMFMediaData::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    OSCL_UNUSED_ARG(uuid);
    iface = NULL;
    return false;
}

OSCL_EXPORT_REF bool PVMFMediaData::getFormatSpecificInfo(OsclRefCounterMemFrag& memfrag)
{
    memfrag = hdr_ptr->format_spec_info;
    return true;
}

OSCL_EXPORT_REF void PVMFMediaData::setFormatSpecificInfo(OsclRefCounterMemFrag& memfrag)
{
    hdr_ptr->format_spec_info = memfrag;
}

OSCL_EXPORT_REF bool PVMFMediaData::setMediaFragFilledLen(uint32 index, uint32 len)
{
    return impl_ptr->setMediaFragFilledLen(index, len);
}

OSCL_EXPORT_REF void PVMFMediaData::setTimestamp(const PVMFTimestamp& ts)
{
    hdr_ptr->timestamp = ts;
}

OSCL_EXPORT_REF void PVMFMediaData::setDuration(const uint32& duration)
{
    hdr_ptr->duration = duration;
}

OSCL_EXPORT_REF void PVMFMediaData::setSeqNum(uint32 seq)
{
    hdr_ptr->seqnum = seq;
}

OSCL_EXPORT_REF void PVMFMediaData::setStreamID(uint32 id)
{
    hdr_ptr->stream_id = id;
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getMarkerInfo()
{
    return impl_ptr->getMarkerInfo();
}

OSCL_EXPORT_REF bool PVMFMediaData::setMarkerInfo(uint32 aMarker)
{
    return impl_ptr->setMarkerInfo(aMarker);
}

OSCL_EXPORT_REF uint32  PVMFMediaData::getErrorsFlag()
{
    return impl_ptr->getErrorsFlag();
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getNumFragments()
{
    return impl_ptr->getNumFragments();
}

OSCL_EXPORT_REF bool PVMFMediaData::getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag)
{
    return impl_ptr->getMediaFragment(index, memfrag);
}

OSCL_EXPORT_REF bool PVMFMediaData::getMediaDataImpl(OsclSharedPtr<PVMFMediaDataImpl>&
        media_data_impl)
{
    media_data_impl = impl_ptr;
    return true;
}


OSCL_EXPORT_REF uint32 PVMFMediaData::getFilledSize()
{
    return impl_ptr->getFilledSize();
}

OSCL_EXPORT_REF uint32 PVMFMediaData::getCapacity()
{
    return impl_ptr->getCapacity();
}

OSCL_EXPORT_REF const PVMFMediaMsgHeader* PVMFMediaData::getMessageHeader()
{
    return hdr_ptr;
}




class MediaDataCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~MediaDataCleanupSA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaDataCleanupSA>));
            tmp_ptr += aligned_refcnt_size;
            PVMFMediaData* mdata_ptr = reinterpret_cast<PVMFMediaData*>(tmp_ptr);
            mdata_ptr->~PVMFMediaData();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

class MediaDataCleanupDA : public OsclDestructDealloc
{
    public:
        MediaDataCleanupDA(Oscl_DefAlloc* in_gen_alloc) : gen_alloc(in_gen_alloc) {};
        virtual ~MediaDataCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;
            uint aligned_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
            // skip the refcounter
            tmp_ptr += aligned_size;
            // skip the MediaData Cleanup
            aligned_size = oscl_mem_aligned_size(sizeof(MediaDataCleanupDA));
            tmp_ptr += aligned_size;

            PVMFMediaData* mdata_ptr = reinterpret_cast<PVMFMediaData*>(tmp_ptr);
            mdata_ptr->~PVMFMediaData();
            gen_alloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};


OSCL_EXPORT_REF OsclSharedPtr<PVMFMediaData>
PVMFMediaData::createMediaData(OsclSharedPtr<PVMFMediaDataImpl>& in_impl_ptr,
                               Oscl_DefAlloc* gen_alloc)
{
    OsclSharedPtr<PVMFMediaData> shared_media_data;

    // allocate enough room
    uint8* my_ptr;
    OsclRefCounter* my_refcnt;

    uint aligned_media_data_size = oscl_mem_aligned_size(sizeof(PVMFMediaData));

    // must compute the aligned size for PVMFMediaData.
    if (gen_alloc)
    {
        uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        uint aligned_cleanup_size = oscl_mem_aligned_size(sizeof(MediaDataCleanupDA));
        my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size +
                                              aligned_cleanup_size +
                                              aligned_media_data_size +
                                              sizeof(PVMFMediaMsgHeader));

        //not allocators leave, so check for NULL ptr
        if (my_ptr == NULL) return shared_media_data;

        MediaDataCleanupDA *my_cleanup = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, MediaDataCleanupDA(gen_alloc));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));
        my_ptr += aligned_refcnt_size + aligned_cleanup_size;

    }
    else
    {
        OsclMemAllocator my_alloc;
        uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaDataCleanupSA>));
        my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size +
                                            aligned_media_data_size + sizeof(PVMFMediaMsgHeader));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<MediaDataCleanupSA>(my_ptr));
        my_ptr += aligned_refcnt_size;
    }

    PVMFMediaData *media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFMediaData());

    media_data_ptr->hdr_ptr = OSCL_PLACEMENT_NEW(my_ptr + aligned_media_data_size, PVMFMediaMsgHeader());

    media_data_ptr->impl_ptr = in_impl_ptr;

    shared_media_data.Bind(media_data_ptr, my_refcnt);

    return shared_media_data;
}

OSCL_EXPORT_REF OsclSharedPtr<PVMFMediaData>
PVMFMediaData::createMediaData(OsclSharedPtr<PVMFMediaDataImpl>& in_impl_ptr,
                               const PVMFMediaMsgHeader* msgHeader,
                               Oscl_DefAlloc* gen_alloc)
{
    OsclSharedPtr<PVMFMediaData> shared_media_data;

    // allocate enough room
    uint8* my_ptr;
    OsclRefCounter* my_refcnt;

    uint aligned_media_data_size = oscl_mem_aligned_size(sizeof(PVMFMediaData));

    // must compute the aligned size for PVMFMediaData.
    if (gen_alloc)
    {
        uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        uint aligned_cleanup_size = oscl_mem_aligned_size(sizeof(MediaDataCleanupDA));
        my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size +
                                              aligned_cleanup_size +
                                              aligned_media_data_size +
                                              sizeof(PVMFMediaMsgHeader));

        //not allocators leave, so check for NULL ptr
        if (my_ptr == NULL) return shared_media_data;

        MediaDataCleanupDA *my_cleanup = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, MediaDataCleanupDA(gen_alloc));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));
        my_ptr += aligned_refcnt_size + aligned_cleanup_size;

    }
    else
    {
        OsclMemAllocator my_alloc;
        uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA<MediaDataCleanupSA>));
        my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size +
                                            aligned_media_data_size + sizeof(PVMFMediaMsgHeader));
        my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<MediaDataCleanupSA>(my_ptr));
        my_ptr += aligned_refcnt_size;
    }

    PVMFMediaData *media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFMediaData());

    media_data_ptr->hdr_ptr = OSCL_PLACEMENT_NEW(my_ptr + aligned_media_data_size, PVMFMediaMsgHeader());

    media_data_ptr->hdr_ptr->timestamp = msgHeader->timestamp;
    media_data_ptr->hdr_ptr->duration = msgHeader->duration;
    media_data_ptr->hdr_ptr->stream_id = msgHeader->stream_id;
    media_data_ptr->hdr_ptr->seqnum = msgHeader->seqnum;
    media_data_ptr->hdr_ptr->format_id = msgHeader->format_id;
    media_data_ptr->hdr_ptr->format_spec_info = msgHeader->format_spec_info;

    media_data_ptr->impl_ptr = in_impl_ptr;

    shared_media_data.Bind(media_data_ptr, my_refcnt);

    return shared_media_data;
}


OSCL_EXPORT_REF PVMFMediaData::~PVMFMediaData()
{

    // call the destructors
    hdr_ptr->~PVMFMediaMsgHeader();

    // the destructor of the impl_ptr will be called by default.

}

OSCL_EXPORT_REF void convertToPVMFMediaMsg(PVMFSharedMediaMsgPtr& dest, PVMFSharedMediaDataPtr& source)
{
    OsclRefCounter* refcnt = source.GetRefCounter();
    /* explicitly increment refcnt since Bind doesn't do it when refcnt and pointer are
     * added separately
     */
    refcnt->addRef();
    dest.Bind(source.GetRep(), refcnt);
}

OSCL_EXPORT_REF void convertToPVMFMediaData(PVMFSharedMediaDataPtr& dest, PVMFSharedMediaMsgPtr& source)
{
    OsclRefCounter* refcnt = source.GetRefCounter();
    /* explicitly increment refcnt since Bind doesn't do it when refcnt and pointer are
     * added separately
     */
    refcnt->addRef();
    dest.Bind(reinterpret_cast<PVMFMediaData*>(source.GetRep()), refcnt);
}


