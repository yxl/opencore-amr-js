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
#include "h223types.h"
#include "h324utils.h"

H223ChannelParam::H223ChannelParam(TPVChannelId id, PS_H223LogicalChannelParameters lcp, PS_DataType dt)
        : pH223Lcp(NULL), pDataType(NULL)
{
    lcn = id;
    if (lcp)
    {
        pH223Lcp = Copy_H223LogicalChannelParameters(lcp);
    }
    if (dt)
    {
        pDataType = Copy_DataType(dt);
    }
    bitrate = GetMaxBitrate(pDataType);
    sample_interval = 0;
    unsigned fr = GetMaxFrameRate(pDataType);
    if (fr)
        sample_interval = (unsigned)((double)1000 / (double)fr);
}

H223ChannelParam::H223ChannelParam(TPVChannelId id,
                                   PS_H223LogicalChannelParameters lcp,
                                   unsigned aBitrate,
                                   unsigned aSampleInterval)
        : pH223Lcp(NULL), pDataType(NULL)
{
    lcn = id;
    if (lcp)
    {
        pH223Lcp = Copy_H223LogicalChannelParameters(lcp);
    }
    bitrate = aBitrate;
    sample_interval = aSampleInterval;
}

H223ChannelParam::H223ChannelParam(const H223ChannelParam & that) : CPVChannelParam(),
        pH223Lcp(NULL), pDataType(NULL)
{
    lcn = that.lcn;
    if (that.pH223Lcp)
    {
        pH223Lcp = Copy_H223LogicalChannelParameters(that.pH223Lcp);
    }
    if (that.pDataType)
    {
        pDataType = Copy_DataType(that.pDataType);
    }
    bitrate = that.bitrate;
    sample_interval = that.sample_interval;
}

H223ChannelParam::~H223ChannelParam()
{
    if (pH223Lcp)
    {
        Delete_H223LogicalChannelParameters(pH223Lcp);
        OSCL_DEFAULT_FREE(pH223Lcp);
        pH223Lcp = NULL;
    }
    if (pDataType)
    {
        Delete_DataType(pDataType);
        OSCL_DEFAULT_FREE(pDataType);
        pDataType = NULL;
    }
}

H223ChannelParam& H223ChannelParam::operator =(const H223ChannelParam & that)
{
    Clear();
    lcn = that.lcn;
    SetLcnParams(that.pH223Lcp);
    SetDataType(that.pDataType);
    bitrate = that.bitrate;
    sample_interval = that.sample_interval;
    return *this;
}

void H223ChannelParam::SetLcnParams(PS_H223LogicalChannelParameters lcp)
{
    if (pH223Lcp)
    {
        Delete_H223LogicalChannelParameters(pH223Lcp);
        OSCL_DEFAULT_FREE(pH223Lcp);
        pH223Lcp = NULL;
    }
    if (lcp)
    {
        pH223Lcp = Copy_H223LogicalChannelParameters(lcp);
    }
}

void H223ChannelParam::SetDataType(PS_DataType dt)
{
    if (pDataType)
    {
        Delete_DataType(pDataType);
        OSCL_DEFAULT_FREE(pDataType);
        pDataType = NULL;
    }
    if (dt)
    {
        pDataType = Copy_DataType(dt);
    }
}

void H223ChannelParam::Clear()
{
    lcn = CHANNEL_ID_UNKNOWN;
    SetLcnParams(NULL);
    SetDataType(NULL);
}

PS_H223LogicalChannelParameters H223ChannelParam::GetLcnParams()
{
    return pH223Lcp;
}

PS_DataType H223ChannelParam::GetDataType()
{
    return pDataType;
}

void H223ChannelParam::SetChannelId(TPVChannelId id)
{
    lcn = id;
}

TPVChannelId H223ChannelParam::GetChannelId()
{
    return lcn;
}
unsigned H223ChannelParam::GetBitrate()
{
    return bitrate;
}

unsigned H223ChannelParam::GetSampleInterval()
{
    return sample_interval;
}

unsigned H223ChannelParam::GetFormatSpecificInfo(uint8*& fsi)
{
    return ::GetFormatSpecificInfo(GetDataType(), fsi);
}

OlcParam* OlcParam::NewL(TPVDirection dir,
                         TPVChannelId id,
                         H223ChannelParam* forward_params,
                         H223ChannelParam* reverse_params)
{
    OlcParam* ret = OSCL_NEW(OlcParam, ());
    ret->Set(dir, id, forward_params, reverse_params);
    return ret;
}

OlcParam::OlcParam()
        : iForwardParams(NULL),
        iReverseParams(NULL)
{
    iState = OLC_IDLE;
    iDir = OUTGOING;
    iId = CHANNEL_ID_UNKNOWN;
    iMtState = MT_IDLE;
    iMtSn = H223_DEFAULT_MT_SN;
    iMtNum = H223_DEFAULT_MT_NUM;
    iReplacementForChannelId = CHANNEL_ID_UNKNOWN;
    iMyType = 0;
}

void OlcParam::Set(TPVDirection dir, TPVChannelId id, H223ChannelParam* forward_params, H223ChannelParam* reverse_params)
{
    iDir = dir;
    iId = id;
    if (iForwardParams)
    {
        OSCL_DELETE(iForwardParams);
        iForwardParams = NULL;
    }
    if (iReverseParams)
    {
        OSCL_DELETE(iReverseParams);
        iReverseParams = NULL;
    }

    if (forward_params)
    {
        iForwardParams = OSCL_NEW(H223ChannelParam, (*forward_params));
    }
    if (reverse_params)
    {
        iReverseParams = OSCL_NEW(H223ChannelParam, (*reverse_params));
    }
}

void OlcParam::InitOlc(TPVDirection dir,
                       TPVChannelId id,
                       PS_DataType dt,
                       PS_H223LogicalChannelParameters lcp,
                       TPVChannelId idRvs,
                       PS_DataType dtRvs,
                       PS_H223LogicalChannelParameters lcpRvs)
{
    H223ChannelParam* forward_params = new H223ChannelParam(id, lcp, dt);
    H223ChannelParam* reverse_params = NULL;
    if (lcpRvs)
    {
        reverse_params = new H223ChannelParam(idRvs, lcpRvs, dtRvs);
    }
    // insert into the list of olcs
    Set(dir, id, forward_params, reverse_params);
    SetState(OLC_PENDING);
    delete forward_params;
    if (reverse_params)
        delete reverse_params;
}

OlcParam::~OlcParam()
{
    if (iForwardParams)
    {
        OSCL_DELETE(iForwardParams);
        iForwardParams = NULL;
    }
    if (iReverseParams)
    {
        OSCL_DELETE(iReverseParams);
        iReverseParams = NULL;
    }
}

TPVDirection OlcParam::GetDirection()
{
    return iDir;
}

TPVChannelId OlcParam::GetChannelId()
{
    return iId;
}

void OlcParam::SetState(OlcState state)
{
    iState = state;
}

OlcState OlcParam::GetState()
{
    return iState;
}

TPVDirectionality OlcParam::GetDirectionality()
{
    return iReverseParams ? EPVT_BI_DIRECTIONAL : EPVT_UNI_DIRECTIONAL;
}

H223ChannelParam* OlcParam::GetForwardParams()
{
    return iForwardParams;
}

H223ChannelParam* OlcParam::GetReverseParams()
{
    return iReverseParams;
}

void OlcParam::SetMtState(MtState state)
{
    iMtState = state;
}

MtState OlcParam::GetMtState()
{
    return iMtState;
}

void OlcParam::SetMtSn(int32 sn)
{
    iMtSn = sn;
}

int32 OlcParam::GetMtSn()const
{
    return iMtSn;
}

void OlcParam::SetMtNum(uint32 num)
{
    iMtNum = num;
}

uint32 OlcParam::GetMtNum()const
{
    return iMtNum;
}

void OlcParam::SetReplacementFor(TPVChannelId id)
{
    iReplacementForChannelId = id;
}

TPVChannelId OlcParam::GetReplacementFor()
{
    return iReplacementForChannelId;
}

OlcList::OlcList()
{
}

OlcList::~OlcList()
{
    Clear();
}

void OlcList::Clear()
{
    OlcParam* olc = NULL;
    OlcList::iterator it = begin();
    while (it != end())
    {
        olc = (*it++).second;
        OSCL_DELETE(olc);
    }
    clear();
}

bool OlcList::HasOlcs(TPVDirection dir,
                      PV2WayMediaType media_type,
                      unsigned states)
{
    bool hasOlcs = false;
    OlcList::iterator it = begin();
    while (it != end())
    {
        value_type& val = (*it++);
        PV2WayMediaType mt_cur =::GetMediaType(val.second->GetForwardParams()->GetDataType());
        if (val.first.iDir == dir &&
                (media_type == PV_MEDIA_NONE || mt_cur == media_type) &&
                (states & val.second->GetState()))
        {
            hasOlcs = true;
            break;
        }
    }
    return hasOlcs;
}

unsigned OlcList::FindOutgoingOlcsByMtState(unsigned states,
        Oscl_Vector<OlcParam*, OsclMemAllocator>& list)
{
    OlcList::iterator it = begin();
    list.clear();
    while (it != end())
    {
        value_type& val = (*it++);
        if ((val.first.iDir == OUTGOING || val.second->GetDirectionality() == EPVT_BI_DIRECTIONAL) &&
                states & val.second->GetMtState())
        {
            list.push_back(val.second);
        }
    }
    return list.size();
}

unsigned OlcList::FindCodecs(TPVDirection dir,
                             PV2WayMediaType media_type,
                             unsigned states,
                             TPVDirection owner,
                             Oscl_Vector<OlcFormatInfo, OsclMemAllocator>& list)
{
    OlcFormatInfo info;
    OlcList::iterator it = begin();
    list.clear();
    while (it != end())
    {
        value_type& val = (*it++);
        if ((owner == PV_DIRECTION_BOTH || owner == PV_DIRECTION_NONE || val.first.iDir == owner) &&
                ((val.first.iDir == dir) || (val.second->GetDirectionality() == EPVT_BI_DIRECTIONAL)) &&
                (val.second->GetState() & states))
        {
            H223ChannelParam* h223params = (dir == val.first.iDir) ? val.second->GetForwardParams() :
                                           val.second->GetReverseParams();
            PVCodecType_t codec =::GetCodecType(h223params->GetDataType());
            if (codec == PV_CODEC_TYPE_NONE)
                continue;
            if ((media_type == PV_MEDIA_NONE) ||
                    (::GetMediaType(h223params->GetDataType()) == media_type))
            {
                info.iId = h223params->GetChannelId();
                info.iCodec = codec;
                info.isSymmetric = true;
                list.push_back(info);
            }
        }
    }
    return list.size();
}

bool OlcList::FindCodec(TPVDirection dir,
                        PV2WayMediaType media_type,
                        unsigned states,
                        TPVDirection owner,
                        OlcFormatInfo& info)
{
    OlcList::iterator it = begin();
    while (it != end())
    {
        value_type& val = (*it++);
        if ((owner == PV_DIRECTION_BOTH || owner == PV_DIRECTION_NONE || val.first.iDir == owner) &&
                ((val.first.iDir == dir) || (val.second->GetDirectionality() == EPVT_BI_DIRECTIONAL)) &&
                (val.second->GetState() & states))
        {
            H223ChannelParam* h223params = (dir == val.first.iDir) ? val.second->GetForwardParams() :
                                           val.second->GetReverseParams();
            if ((media_type == PV_MEDIA_NONE) ||
                    (::GetMediaType(h223params->GetDataType()) == media_type))
            {
                info.iId = h223params->GetChannelId();
                info.iCodec = ::GetCodecType(h223params->GetDataType());
                info.isSymmetric = true;
                return true;
            }
        }
    }
    return false;
}

bool OlcList::IsSymmetric(PV2WayMediaType media_type,
                          TPVDirection out_owner,
                          unsigned outgoing_states,
                          TPVDirection in_owner,
                          unsigned incoming_states)
{
    OlcFormatInfo outgoing_info, incoming_info;
    if (FindCodec(OUTGOING,  media_type, outgoing_states, out_owner, outgoing_info) &&
            FindCodec(INCOMING,  media_type, incoming_states, in_owner, incoming_info))
    {
        return (outgoing_info.iCodec == incoming_info.iCodec);
    }
    return true;
}


OlcParam* OlcList::AppendOlc(TPVDirection dir,
                             TPVChannelId id,
                             PS_DataType dt,
                             PS_H223LogicalChannelParameters lcp,
                             TPVChannelId idRvs,
                             PS_DataType dtRvs,
                             PS_H223LogicalChannelParameters lcpRvs)
{
    OlcParam* olc = OSCL_NEW(OlcParam, ());
    olc->InitOlc(dir, id, dt, lcp, idRvs, dtRvs, lcpRvs);
    AppendOlc(olc, dir, id);

    return olc;
}

void OlcList::AppendOlc(OlcParam* param,
                        TPVDirection dir,
                        TPVChannelId id)
{
    OlcKey key(dir, id);
    insert(OlcList::value_type(key, param));
}


unsigned OlcList::FindOlcs(TPVDirection dir,
                           PV2WayMediaType media_type,
                           unsigned states,
                           Oscl_Vector<OlcParam*, OsclMemAllocator>& list)
{
    OlcList::iterator it = begin();
    list.clear();
    while (it != end())
    {
        value_type& val = (*it++);
        PV2WayMediaType mt_cur =::GetMediaType(val.second->GetForwardParams()->GetDataType());
        if (val.first.iDir == dir &&
                (media_type == PV_MEDIA_NONE || (mt_cur == media_type)))
        {
            if (states & val.second->GetState())
            {
                list.push_back(val.second);
            }
        }
    }
    return list.size();
}


OlcParam* OlcList::FindOlcByMtSn(uint32 sn)
{
    OlcList::iterator it = begin();
    while (it != end())
    {
        OlcList::value_type& val = (*it++);
        OlcParam* olc = val.second;
        if (olc->GetDirection() == OUTGOING || olc->GetReverseParams())
        {
            if (olc->GetMtSn() >= 0 && sn == (uint32)olc->GetMtSn())
            {
                return olc;
            }
        }
    }
    return NULL;
}

OlcParam* OlcList::FindOlcGivenChannel(TPVDirection dir, TPVChannelId lcn)
{
    OlcKey key(dir, lcn);
    OlcList::iterator iter = find(key);
    if (iter == end() || ((*iter).second == NULL))
    {
        return NULL;
    }
    return (*iter).second;
}


OlcParam* OlcList::FindOlc(TPVDirection dir, PV2WayMediaType media_type, unsigned state)
{
    OlcList::iterator it = begin();
    while (it != end())
    {
        value_type& val = (*it++);
        PV2WayMediaType mt_cur =::GetMediaType(val.second->GetForwardParams()->GetDataType());
        if (val.first.iDir == dir &&
                (media_type == PV_MEDIA_NONE || (mt_cur == media_type)))
        {
            if (state & val.second->GetState())
            {
                return val.second;
            }
        }
    }
    return NULL;
}

bool OlcList::HasOlc(TPVDirection dir, TPVChannelId lcn)
{
    OlcKey key(dir, lcn);
    OlcList::iterator iter = find(key);
    if (iter == end() || ((*iter).second == NULL))
    {
        return false;
    }
    return true;
}

bool OlcList::HasOlc(TPVDirection dir, TPVChannelId lcn, unsigned state)
{
    OlcKey key(dir, lcn);
    OlcList::iterator iter = find(key);
    if (iter == end() || ((*iter).second == NULL))
    {
        return false;
    }
    else
    {
        // loop through and find one with given state
        while (iter != end())
        {
            OlcParam* olcparam = (*iter++).second;
            int tempState = state;
            if (olcparam->GetState() == tempState)
            {
                return true;
            }
        }
    }
    return false;
}

void OlcList::SetCurrLcn(TPVChannelId aCurLcn)
{
    iCurLcn = aCurLcn;
}

TPVChannelId OlcList::GetNextAvailLcn()
{
    TPVChannelId ret = iCurLcn++;
    bool duplicate = true;
    while (duplicate)
    {
        OlcParam* olc_param = FindOlcGivenChannel(OUTGOING, ret);
        if (ret && (olc_param == NULL))
            duplicate = false;
        else
            ret = iCurLcn++;
    }
    return ret;
}


int TPVMuxDescriptorSlot::is_fit(unsigned that_lcn, unsigned size)
{
    int ret = (lcn == that_lcn);
    if (ret && size && max_size)
    {
        ret = (size >= min_size && size <= max_size);
    }
    return ret;
}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptor* CPVMultiplexEntryDescriptor::NewL(PS_MultiplexEntryDescriptor descriptor,
        unsigned max_pdu_size)
{
    CPVMultiplexEntryDescriptor* ret = OSCL_NEW(CPVMultiplexEntryDescriptor, ());
    ret->ConstructL(descriptor, max_pdu_size);
    return ret;
}


CPVMultiplexEntryDescriptor::CPVMultiplexEntryDescriptor():
        iDescriptor(NULL),
        iMaxPduSize(0)
{
}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptor::CPVMultiplexEntryDescriptor(const CPVMultiplexEntryDescriptor& that)
{
    ConstructL(that.iDescriptor, that.iMaxPduSize);
}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptor::~CPVMultiplexEntryDescriptor()
{
    iLcns.clear();

    if (iDescriptor)
    {
        Delete_MultiplexEntryDescriptor(iDescriptor);
        OSCL_DEFAULT_FREE(iDescriptor);
        iDescriptor = NULL;
    }
}

void CPVMultiplexEntryDescriptor::ConstructL(PS_MultiplexEntryDescriptor descriptor, unsigned max_pdu_size)
{
    OSCL_ASSERT(descriptor);
    bool control_descriptor = false;
    if (descriptor->multiplexTableEntryNumber == 0)
    {
        descriptor->multiplexTableEntryNumber = 1;
        control_descriptor = true;
    }
    iDescriptor = Copy_MultiplexEntryDescriptor(descriptor);
    if (control_descriptor)
    {
        iDescriptor->multiplexTableEntryNumber = 0;
        descriptor->multiplexTableEntryNumber = 0;
    }
    iMaxPduSize = max_pdu_size;

    if (descriptor->option_of_elementList)
    {
        int size = 0;
        FindLcns(descriptor->elementList, descriptor->size_of_elementList, iMaxPduSize, iLcns, &size);
    }
}

OSCL_EXPORT_REF int CPVMultiplexEntryDescriptor::FindLcn(uint16 channel_id, uint16 len, TPVMuxDescriptorSlot& slot_info)
{
    TPVMuxDescriptorSlotList::iterator it = iLcns.find(channel_id);
    int ret = 0;
    if (it != iLcns.end())
    {
        TPVMuxDescriptorSlot& found_slot = (*it).second;
        if (found_slot.is_fit(channel_id, len))
        {
            slot_info = (*it).second;
            ret = 1;
        }
    }
    return ret;
}

unsigned CPVMultiplexEntryDescriptor::FindLcns(
    PS_MultiplexElement pElement,
    int ListSize,
    int max_pdu_size,
    TPVMuxDescriptorSlotList& lcns,
    int* subelement_size)
{
    unsigned num_lcns = 0;
    *subelement_size = 0;

    for (int cnt = ListSize ; cnt != 0 ; cnt --)
    {
        /* SubElemnt Search	*/
        if (pElement->muxType.index)
        {
            int size = 0;
            TPVMuxDescriptorSlotList lcns_local;
            num_lcns += FindLcns(pElement->muxType.subElementList, pElement->muxType.size, max_pdu_size, lcns_local, &size);
            OSCL_ASSERT(size);
            unsigned rc_min = 0;
            unsigned rc_max = 0;
            if (pElement->repeatCount.index)
            {
                // ucf
                rc_max = max_pdu_size / size;
                rc_min = 1;
            }
            else
            {
                // finite
                rc_max = rc_min = pElement->repeatCount.finite;
            }
            /* update the subelement size */
            *subelement_size += (rc_max * size);

            /* update the max size */
            max_pdu_size -= (*subelement_size);

            /* Multiply the max and min sizes in lcns_local by rc, and add to the lcns list */
            TPVMuxDescriptorSlotList::iterator it = lcns_local.begin();

            while (it != lcns_local.end())
            {
                TPVMuxDescriptorSlot slot_info = {0, 0, 0};
                TPVMuxDescriptorSlot& slot = (*it++).second;

                slot.max_size *= rc_max;
                slot.min_size *= rc_min;
                /* is it present in the lcns list ? */
                if (FindLcn((uint16)slot.lcn, 0, slot_info) <= 0)
                {
                    lcns.insert(TPVMuxDescriptorSlotList::value_type(slot.lcn, slot_info));
                }
                slot_info.lcn = slot.lcn;
                slot_info.min_size += slot.min_size;
                slot_info.max_size += slot.max_size;
                lcns[slot.lcn] = slot_info;
            }
        }
        else
        {
            TPVMuxDescriptorSlot slot_info = {0, 0, 0};
            unsigned min_size = 0;
            unsigned max_size = 0;
            if (pElement->repeatCount.index)
            {
                // ucf
                min_size = 1;
                max_size = 0;
            }
            else
            {
                // finite
                min_size = max_size = pElement->repeatCount.finite;
            }
            slot_info.lcn = pElement->muxType.logicalChannelNumber;
            slot_info.min_size = min_size;
            slot_info.max_size = max_size;
            lcns.insert(TPVMuxDescriptorSlotList::value_type(slot_info.lcn, slot_info));
            num_lcns++;
            *subelement_size += max_size;
            max_pdu_size -= max_size;
        }
        pElement ++;
    }
    return num_lcns;
}

OSCL_EXPORT_REF unsigned CPVMultiplexEntryDescriptor::NumLcns()
{
    return iLcns.size();
}

OSCL_EXPORT_REF PS_MultiplexEntryDescriptor CPVMultiplexEntryDescriptor::GetH245descriptor()
{
    return iDescriptor;
}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptorVector::CPVMultiplexEntryDescriptorVector()
{

}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptorVector::CPVMultiplexEntryDescriptorVector(const CPVMultiplexEntryDescriptorVector& that) :  Oscl_Vector<CPVMultiplexEntryDescriptor*, OsclMemAllocator>(that)

{
    CPVMultiplexEntryDescriptor* desc = NULL;
    for (unsigned num = 0; num < that.size(); num++)
    {
        desc = OSCL_NEW(CPVMultiplexEntryDescriptor, (*that[num]));
        push_back(desc);
    }
}

OSCL_EXPORT_REF CPVMultiplexEntryDescriptorVector::~CPVMultiplexEntryDescriptorVector()
{
    Clear();
}

OSCL_EXPORT_REF void CPVMultiplexEntryDescriptorVector::Clear()
{
    CPVMultiplexEntryDescriptor* desc = NULL;
    while (size())
    {
        desc = back();
        OSCL_DELETE(desc);
        pop_back();
    }
}
