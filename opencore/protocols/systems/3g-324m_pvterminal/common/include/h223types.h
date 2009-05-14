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
#ifndef	_h223types_H
#define _h223types_H

#include "oscl_base.h"
#include "oscl_mem.h"
#include "pvt_params.h"
#include "h245pri.h"
#include "h245_deleter.h"
#include "h245_copier.h"

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#include "oscl_map.h"
#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#define MAX_LCNS 5
#define FIRST_NULL_LCN 1000
#define MAX_SEGMENTABLE_LCN_NUM 2

#define H223_MAX_MUX_PDU_SIZE 512
#define H223_MAX_MUX_PDU_SIZE_LEVEL2 255
#define H223_MAX_DEMUX_PDU_SIZE 1024
#define H223_DEF_PDU_SIZE 240

#define H223Error_t int
typedef H223Error_t H223Ret_t;
#define H223_OK 0
#define H223_ERROR 1
#define H223_ERROR_OUT_OF_MEMORY 2
#define H223_ERROR_OUT_OF_DEMUX_MEMORY 3
#define H223_INSUFFICIENT_DATA 4
#define H223_ERROR_BUFFER_SZ 5
#define H223_STATE_BUSY 6
#define H223_ERROR_INVALID_LCN 7
#define H223_ERROR_MAX_LCNS 8
#define H223_ERROR_INVALID_PARAM 9

#define	NO_ERR	    0
#define	CRC_ERR	    1
#define	SN_ERR	    2
#define H223_RC_UCF 1
#define H223_RC_FINITE 0
#define H223_MUXTYPE_LCN 0
#define H223_MUXTYPE_SEL 1

#define MAX_H223_PDU_PACKETS_PER_SEND 20
#define MAX_FRAGMENTS_PER_H223_PDU_PACKET 10
#define H223_MAX_PDU_SIZE 254
#define H223_MIN_PDU_SIZE 80
#define H223_DEFAULT_PDU_SIZE 160
#define H223_MIN_SEGMENTABLE_PAYLOAD_SIZE 80
#define H223_DEFAULT_MT_SN -1
#define H223_DEFAULT_MT_NUM 0

class CPVMultiplexDescriptor : public CPVParam
{
    public:
        CPVMultiplexDescriptor(PS_MuxDescriptor desc, bool delete_multiplexEntryDescriptors = false)
                : descriptor(desc), iDelete_multiplexEntryDescriptors(delete_multiplexEntryDescriptors) {}
        ~CPVMultiplexDescriptor()
        {
            if (descriptor)
            {
                if (descriptor->size_of_multiplexEntryDescriptors)
                {
                    for (int i = 0; i < descriptor->size_of_multiplexEntryDescriptors; i++)
                    {
                        Delete_MultiplexEntryDescriptor(&descriptor->multiplexEntryDescriptors[i]);
                    }
                    if (iDelete_multiplexEntryDescriptors)
                    {
                        OSCL_DEFAULT_FREE(descriptor->multiplexEntryDescriptors);
                        descriptor->multiplexEntryDescriptors = NULL;
                    }
                }
                OSCL_DEFAULT_FREE(descriptor);
                descriptor = NULL;
            }
        }
        CPVParam* Copy()
        {
            return NULL;
        }

        PS_MuxDescriptor GetDescriptor()
        {
            return descriptor;
        }
        PS_MuxDescriptor descriptor;
        bool iDelete_multiplexEntryDescriptors;
};

class H223ChannelParam : public CPVChannelParam
{
    public:
        H223ChannelParam(TPVChannelId id, PS_H223LogicalChannelParameters lcp, PS_DataType dt);
        H223ChannelParam(TPVChannelId id,
                         PS_H223LogicalChannelParameters lcp,
                         unsigned bitrate,
                         unsigned sample_interval);

        H223ChannelParam(const H223ChannelParam & that);
        ~H223ChannelParam();
        H223ChannelParam& operator =(const H223ChannelParam& that);

        void SetLcnParams(PS_H223LogicalChannelParameters lcp);

        void SetDataType(PS_DataType dt);

        void Clear();

        PS_H223LogicalChannelParameters GetLcnParams();

        PS_DataType GetDataType();

        void SetChannelId(TPVChannelId id);

        TPVChannelId GetChannelId();
        unsigned GetBitrate();
        unsigned GetSampleInterval();
        unsigned GetFormatSpecificInfo(uint8*& fsi);
    private:
        TPVChannelId lcn;
        PS_H223LogicalChannelParameters pH223Lcp;
        PS_DataType pDataType;
        unsigned bitrate;
        unsigned sample_interval;
};

enum OlcState
{
    OLC_IDLE = 0x1,
    OLC_PENDING = 0x2,
    OLC_ESTABLISHED = 0x4,
    OLC_CLOSING = 0x8,
    OLC_CLOSED = 0x10,
    OLC_STATE_UNKNOWN = 0x1F
};

enum MtState
{
    MT_IDLE = 0x1,
    MT_PENDING = 0x2,
    MT_COMPLETE = 0x4,
    MT_RELEASING = 0x8,
    MT_RELEASED = 0x10,
    MT_STATE_UNKNOWN = 0x1F
};

class OlcParam
{
    public:
        static OlcParam* NewL(TPVDirection dir, TPVChannelId iId, H223ChannelParam* forward_params, H223ChannelParam* reverse_params = NULL);
        ~OlcParam();
        void SetState(OlcState state);

        TPVDirection GetDirection();
        TPVChannelId GetChannelId();
        OlcState GetState();
        TPVDirectionality GetDirectionality();
        H223ChannelParam* GetForwardParams();
        H223ChannelParam* GetReverseParams();
        void SetMtState(MtState state);
        MtState GetMtState();
        void SetMtSn(int32 sn);
        int32 GetMtSn()const;
        void SetMtNum(uint32 num);
        uint32 GetMtNum()const;
        void SetReplacementFor(TPVChannelId id);
        TPVChannelId GetReplacementFor();
        void InitOlc(TPVDirection dir,
                     TPVChannelId id,
                     PS_DataType dt,
                     PS_H223LogicalChannelParameters lcp,
                     TPVChannelId idRvs = CHANNEL_ID_UNKNOWN,
                     PS_DataType dtRvs = NULL,
                     PS_H223LogicalChannelParameters lcpRvs = NULL);
        bool IsMyType(int type)
        {
            return type == iMyType;
        }
        friend class OlcList;
    protected:
        OlcParam();
        void Set(TPVDirection dir, TPVChannelId iId, H223ChannelParam* forward_params, H223ChannelParam* reverse_params);
        int iMyType;

    private:
        TPVDirection iDir;
        TPVChannelId iId;
        OlcState iState;
        H223ChannelParam* iForwardParams;
        H223ChannelParam* iReverseParams;
        MtState iMtState;
        int32 iMtSn; /* while Pending, it is >0 */
        uint32 iMtNum; /* Is valid while pending and established */
        TPVChannelId iReplacementForChannelId;
};

class OlcKey
{
    public:
        OlcKey(): iDir(PV_DIRECTION_BOTH), iId(0) {}
        OlcKey(TPVDirection dir, TPVChannelId id) : iDir(dir), iId(id) {}
        TPVDirection iDir;
        TPVChannelId iId;
};

inline int operator < (const OlcKey& a, const OlcKey& b)
{
    if (a.iDir == b.iDir)
    {
        return (a.iId < b.iId);
    }
    return (a.iDir < b.iDir);
}

class OlcList:  public Oscl_Map<OlcKey, OlcParam*, OsclMemAllocator>
{
    public:
        OlcList();
        ~OlcList();
        unsigned FindCodecs(TPVDirection dir,
                            PV2WayMediaType media_type,
                            unsigned states,
                            TPVDirection owner,
                            Oscl_Vector<OlcFormatInfo, OsclMemAllocator>& list);
        bool FindCodec(TPVDirection dir,
                       PV2WayMediaType media_type,
                       unsigned states,
                       TPVDirection owner,
                       OlcFormatInfo& info);
        unsigned FindOutgoingOlcsByMtState(unsigned states,
                                           Oscl_Vector<OlcParam*, OsclMemAllocator>& list);

        bool IsSymmetric(PV2WayMediaType media_type,
                         unsigned outgoing_states,
                         unsigned incoming_states);
        bool IsSymmetric(PV2WayMediaType media_type,
                         TPVDirection out_owner,
                         unsigned outgoing_states,
                         TPVDirection in_owner,
                         unsigned incoming_states);
        void Clear();

        bool HasOlcs(TPVDirection dir,
                     PV2WayMediaType media_type,
                     unsigned states);
        bool HasOlc(TPVDirection dir, TPVChannelId lcn);
        bool HasOlc(TPVDirection dir, TPVChannelId lcn, unsigned state);
        void SetCurrLcn(TPVChannelId aCurLcn);
        TPVChannelId GetNextAvailLcn();
        unsigned FindOlcs(TPVDirection dir,
                          PV2WayMediaType media_type,
                          unsigned states,
                          Oscl_Vector<OlcParam*, OsclMemAllocator>& list);
        OlcParam* FindOlc(TPVDirection dir,
                          PV2WayMediaType media_type,
                          unsigned states);
        OlcParam* FindOlcGivenChannel(TPVDirection dir, TPVChannelId lcn);

        OlcParam* FindOlcByMtSn(uint32 sn);

        void AppendOlc(OlcParam* param,
                       TPVDirection dir,
                       TPVChannelId id);

        OlcParam* AppendOlc(TPVDirection dir,
                            TPVChannelId id,
                            PS_DataType dt,
                            PS_H223LogicalChannelParameters lcp,
                            TPVChannelId idRvs = CHANNEL_ID_UNKNOWN,
                            PS_DataType dtRvs = NULL,
                            PS_H223LogicalChannelParameters lcpRvs = NULL);


    private:
        TPVChannelId iCurLcn;
};


class TPVMuxDescriptorSlot
{
    public:
        unsigned lcn;
        unsigned min_size;
        unsigned max_size;
        int is_fit(unsigned that_lcn, unsigned size);
};

typedef Oscl_Map<unsigned , TPVMuxDescriptorSlot, PoolMemAlloc_OsclMemAllocator_10> TPVMuxDescriptorSlotList;

class CPVMultiplexEntryDescriptor
{
    public:
        OSCL_IMPORT_REF static CPVMultiplexEntryDescriptor* NewL(PS_MultiplexEntryDescriptor descriptor, unsigned max_pdu_size);
        OSCL_IMPORT_REF CPVMultiplexEntryDescriptor(const CPVMultiplexEntryDescriptor& that);
        OSCL_IMPORT_REF ~CPVMultiplexEntryDescriptor();
        OSCL_IMPORT_REF unsigned NumLcns();
        OSCL_IMPORT_REF int FindLcn(uint16 channel_id, uint16 len, TPVMuxDescriptorSlot& slot_info);
        OSCL_IMPORT_REF PS_MultiplexEntryDescriptor GetH245descriptor();
    private:
        CPVMultiplexEntryDescriptor();
        void ConstructL(PS_MultiplexEntryDescriptor descriptor, unsigned max_pdu_size);

        unsigned FindLcns(
            PS_MultiplexElement pElement,
            int ListSize,
            int max_size,
            TPVMuxDescriptorSlotList& lcns,
            int* sublelement_size);

        PS_MultiplexEntryDescriptor iDescriptor;
        unsigned iMaxPduSize;
        TPVMuxDescriptorSlotList iLcns;
};

class CPVMultiplexEntryDescriptorVector : public Oscl_Vector<CPVMultiplexEntryDescriptor*, OsclMemAllocator>
{
    public:
        OSCL_IMPORT_REF CPVMultiplexEntryDescriptorVector();
        OSCL_IMPORT_REF CPVMultiplexEntryDescriptorVector(const CPVMultiplexEntryDescriptorVector& that);
        OSCL_IMPORT_REF ~CPVMultiplexEntryDescriptorVector();
        OSCL_IMPORT_REF void Clear();
};

typedef PoolMemAlloc<8> PoolMemAlloc_OsclMemAllocator_8;
typedef PoolMemAlloc<MAX_H223_PDU_PACKETS_PER_SEND, 512> PoolMemAlloc_OsclMemAllocator_MAX_H223_PDU_PACKETS_PER_SEND;
typedef Oscl_Vector<OsclSharedPtr<PVMFMediaDataImpl>, OsclMemAllocator> MuxPduPacketList;


#endif	/* _h223api_H */
