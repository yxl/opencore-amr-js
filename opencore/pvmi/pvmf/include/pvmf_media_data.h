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
 *  @file pvmf_media_data.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media data
 *  class which is used as the interface to the media data containers that
 *  are passed between nodes along the normal media data path.
 *
 */

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#define PVMF_MEDIA_DATA_H_INCLUDED

#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif


struct PVMFMediaMsgHeader;
class PVMFMediaDataImpl;

// media data error masks
#define PVMF_MEDIA_DATA_BIT_ERRORS 0x1
#define PVMF_MEDIA_DATA_PACKET_LOSS 0x2

//marker info masks
//marker info is a 32 bit field, with each bit signalling different info
// LSB->MSB; Bit 0 - M bit (indicates whether the media data contains end of a media unit)
// Bit 1 - Signals whether the media data has a valid duration field, if this bit is not
// set then duration value returned in "getDuration" is undefined and should not be used.
// Bit 2 - Signals whether the media data should be rendered or not. If set to 1, this
// means the media data contained is not to be rendered. Applies only to media data and not
// to media commands
// Bit 3 - Signals whether the media data is or part of a key sample. If set to 1, this
// means the media data contained is is or part of a key sample.
// Applies only to media data and not to media commands
// Bit 4 - Signals whether the node needs to report the completion of this message to the
// Observer. If set to 1, this means the node needs to report info event after processing
// this message
// Bit 5 - Indicates for H.264/AVC if fragment marks the end of a NAL.  This is false for
// the first and middle fragments and true for the last fragment and for complete NALs
// (single or aggregate).
// Bits 6 through 31 - Reserved
#define PVMF_MEDIA_DATA_MARKER_INFO_M_BIT                   0x00000001
#define PVMF_MEDIA_DATA_MARKER_INFO_DURATION_AVAILABLE_BIT  0x00000002
#define PVMF_MEDIA_DATA_MARKER_INFO_NO_RENDER_BIT           0x00000004
#define PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT 0x00000008
#define PVMF_MEDIA_DATA_MARKER_INFO_REPORT_OBSERVER_BIT		0x00000010
#define PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT          0x00000020

class PVMFMediaData : public PVMFMediaMsg
{


    public:
        OSCL_IMPORT_REF virtual ~PVMFMediaData();
        OSCL_IMPORT_REF virtual PVMFTimestamp getTimestamp();
        OSCL_IMPORT_REF virtual uint32 getDuration();
        OSCL_IMPORT_REF virtual uint32 getStreamID();
        OSCL_IMPORT_REF virtual uint32 getSeqNum();
        OSCL_IMPORT_REF virtual PVUid32 getFormatID();
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF virtual bool getFormatSpecificInfo(OsclRefCounterMemFrag& memfrag);
        OSCL_IMPORT_REF virtual void setFormatSpecificInfo(OsclRefCounterMemFrag& memfrag);
        OSCL_IMPORT_REF virtual bool setMediaFragFilledLen(uint32 index, uint32 len);
        OSCL_IMPORT_REF virtual void setTimestamp(const PVMFTimestamp& timestamp);
        OSCL_IMPORT_REF virtual void setDuration(const uint32& duration);
        OSCL_IMPORT_REF virtual void setStreamID(uint32 id);
        OSCL_IMPORT_REF virtual void setSeqNum(uint32 seqnum);

        OSCL_IMPORT_REF uint32 getMarkerInfo();
        OSCL_IMPORT_REF bool setMarkerInfo(uint32 aMarker);
        OSCL_IMPORT_REF uint32 getErrorsFlag();
        OSCL_IMPORT_REF uint32 getNumFragments();
        OSCL_IMPORT_REF bool getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag);
        OSCL_IMPORT_REF bool getMediaDataImpl(OsclSharedPtr<PVMFMediaDataImpl>&
                                              media_data_impl);
        OSCL_IMPORT_REF uint32 getFilledSize();
        OSCL_IMPORT_REF uint32 getCapacity();
        OSCL_IMPORT_REF const PVMFMediaMsgHeader* getMessageHeader();


        OSCL_IMPORT_REF static OsclSharedPtr<PVMFMediaData>
        createMediaData(OsclSharedPtr<PVMFMediaDataImpl>& in_impl_ptr,
                        Oscl_DefAlloc* gen_alloc = NULL);

        OSCL_IMPORT_REF static OsclSharedPtr<PVMFMediaData>
        createMediaData(OsclSharedPtr<PVMFMediaDataImpl>& in_impl_ptr,
                        const PVMFMediaMsgHeader* msgHeader,
                        Oscl_DefAlloc* gen_alloc = NULL);

    private:
        PVMFMediaData(): hdr_ptr(0), impl_ptr() {};
        PVMFMediaMsgHeader* hdr_ptr;
        OsclSharedPtr<PVMFMediaDataImpl> impl_ptr;
};

typedef OsclSharedPtr<PVMFMediaData> PVMFSharedMediaDataPtr;

OSCL_IMPORT_REF void convertToPVMFMediaMsg(PVMFSharedMediaMsgPtr& dest, PVMFSharedMediaDataPtr& source);
OSCL_IMPORT_REF void convertToPVMFMediaData(PVMFSharedMediaDataPtr& dest, PVMFSharedMediaMsgPtr& source);

#endif
