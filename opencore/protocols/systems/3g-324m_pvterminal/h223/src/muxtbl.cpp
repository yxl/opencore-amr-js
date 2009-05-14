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
#include "oscl_mem.h"
#include "muxtbl.h"
#include "h245_deleter.h"
#include "h245_copier.h"
#include "oscl_mem.h"
#ifndef	_h324utils_h
#include "h324utils.h"
#endif
#define ON 1
#define OFF 0
#define ACTIVATE 1
#define DEACTIVATE 0

#define SRP_MUX_ENTRY_NUMBER 0
#define WNSRP_MUX_ENTRY_NUMBER 15

MuxTableMgr::MuxTableMgr(): iOutgoingDescriptors(NULL)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.muxtable");
    iOutgoingMuxTblCount = 0;
    iIncomingMuxTblCount = 0;

    uint16 entry_num;
    for (entry_num = 0; entry_num < MAX_MUX_ENTRIES; entry_num ++)
    {
        iMuxDescriptorR[entry_num] = NULL;
    }

    AddIncomingControlDescriptor();
    iOutgoingDescriptors = OSCL_NEW(CPVMultiplexEntryDescriptorVector, ());
    AddControlDescriptor(*iOutgoingDescriptors);

    ResetStats();
}

MuxTableMgr::~MuxTableMgr()
{
    for (int32 entry_num = 0; entry_num < MAX_MUX_ENTRIES; entry_num ++)
    {
        if (iMuxDescriptorR[entry_num])
        {
            Delete_MultiplexEntryDescriptor(iMuxDescriptorR[entry_num]);
            OSCL_DEFAULT_FREE(iMuxDescriptorR[entry_num]);
            iMuxDescriptorR[entry_num] = NULL;
        }
    }

    if (iOutgoingDescriptors)
    {
        OSCL_DELETE(iOutgoingDescriptors);
        iOutgoingDescriptors = NULL;
    }
}

PS_MultiplexEntryDescriptor MuxTableMgr::GetControlDescriptor(uint16 mux_entry_num)
{
    PS_MultiplexEntryDescriptor desc;

    desc = (PS_MultiplexEntryDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntryDescriptor));
    desc->multiplexTableEntryNumber = (uint8)mux_entry_num;
    desc->option_of_elementList = true;
    desc->size_of_elementList = 1;
    desc->elementList = (PS_MultiplexElement)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexElement));

    PS_MultiplexElement elem = 	desc->elementList;
    elem->muxType.index = 0; /* logical channel, 1 = sub-elem list */
    elem->muxType.logicalChannelNumber = 0;
    elem->muxType.size = 1; /* size of element list */
    elem->repeatCount.index = 1; /* ucf */
    elem->repeatCount.finite = 0;

    return desc;
}

OsclAny MuxTableMgr::AddIncomingControlDescriptor()
{
    iMuxDescriptorR[SRP_MUX_ENTRY_NUMBER]  = GetControlDescriptor(SRP_MUX_ENTRY_NUMBER);
    iMuxDescriptorFlagR[SRP_MUX_ENTRY_NUMBER] = ACTIVATE;
    iMuxDescriptorR[WNSRP_MUX_ENTRY_NUMBER]  = GetControlDescriptor(WNSRP_MUX_ENTRY_NUMBER);
    iMuxDescriptorFlagR[WNSRP_MUX_ENTRY_NUMBER] = ACTIVATE;
}

void MuxTableMgr::AddControlDescriptor(CPVMultiplexEntryDescriptorVector& descriptors)
{
//	PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"MuxTableMgr::AddControlDescriptor\n"));
    PS_MultiplexEntryDescriptor h245_desc = GetControlDescriptor(SRP_MUX_ENTRY_NUMBER);
    CPVMultiplexEntryDescriptor* descriptor = CPVMultiplexEntryDescriptor::NewL(h245_desc, 128);
    descriptors.push_back(descriptor);
    Delete_MultiplexEntryDescriptor(h245_desc);
    OSCL_DEFAULT_FREE(h245_desc);
}

OsclAny MuxTableMgr::SetIncomingDescriptors(PS_MuxDescriptor mux_desc)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::SetIncomingDescriptors"));
    PS_MultiplexEntryDescriptor  descriptor = mux_desc->multiplexEntryDescriptors;
    uint8 entry_num = 0;
    for (int32 num = 0; num < mux_desc->size_of_multiplexEntryDescriptors; num++)
    {
        entry_num = descriptor->multiplexTableEntryNumber;
        if (iMuxDescriptorR[entry_num])
        {
            RemoveDescriptorR(entry_num);
            iMuxDescriptorFlagR[entry_num] = DEACTIVATE;
        }

        if (descriptor->option_of_elementList == true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::SetIncomingDescriptors Adding mux entry=%d", entry_num));
            iMuxDescriptorR[entry_num] = Copy_MultiplexEntryDescriptor(descriptor);
            iMuxDescriptorFlagR[entry_num] = ACTIVATE;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::SetIncomingDescriptors Removing mux entry=%d", entry_num));
        }
        descriptor++;
    }
}

OsclAny MuxTableMgr::SetIncomingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors, bool replace)
{
    for (unsigned n = 0; n < descriptors.size(); n++)
    {
        uint8 entry_num = descriptors[n]->GetH245descriptor()->multiplexTableEntryNumber;
        if (iMuxDescriptorR[entry_num])
        {
            if (!replace)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "MuxTableMgr::SetIncomingDescriptors descriptor already exists for mux entry=%d and we are not to replace it!", entry_num));
                continue;
            }
            RemoveDescriptorR(entry_num);
            iMuxDescriptorFlagR[entry_num] = DEACTIVATE;
        }
        if (descriptors[n]->GetH245descriptor()->option_of_elementList == true)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::SetIncomingDescriptors Adding mux entry=%d", entry_num));
            iMuxDescriptorR[entry_num] = Copy_MultiplexEntryDescriptor(descriptors[n]->GetH245descriptor());
            iMuxDescriptorFlagR[entry_num] = ACTIVATE;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::SetIncomingDescriptors Removing mux entry=%d", entry_num));
        }
    }
}

void MuxTableMgr::SetOutgoingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors)
{
    if (!iOutgoingDescriptors)
    {
        iOutgoingDescriptors = OSCL_NEW(CPVMultiplexEntryDescriptorVector, ());
    }
    for (unsigned n = 0; n < descriptors.size(); n++)
    {
        iOutgoingDescriptors->push_back(OSCL_NEW(CPVMultiplexEntryDescriptor, (*descriptors[n])));
    }
}

void MuxTableMgr::RemoveOutgoingMuxDescriptor(uint8 muxTblNum)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::RemoveOutgoingMuxDescriptor muxTblNum(%d)", muxTblNum));
    if (iOutgoingDescriptors == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::RemoveOutgoingMuxDescriptor iOutgoingDescriptors==NULL"));
        return;
    }
    CPVMultiplexEntryDescriptorVector::iterator iter = iOutgoingDescriptors->begin();
    while (iter != iOutgoingDescriptors->end())
    {
        if ((*iter)->GetH245descriptor()->multiplexTableEntryNumber == muxTblNum)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::RemoveOutgoingMuxDescriptor Found the mux table entry"));
            OSCL_DELETE(*iter);
            iOutgoingDescriptors->erase(iter);
            return;
        }
        iter++;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::RemoveOutgoingMuxDescriptor Failed to lookup multiplex entry"));
}

void MuxTableMgr::RemoveIncomingMuxDescriptor(uint8 muxTblNum)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "MuxTableMgr::RemoveIncomingMuxDescriptor muxTblNum(%d)", muxTblNum));
    RemoveDescriptorR(muxTblNum);
}

bool MuxTableMgr::RemoveDescriptorR(uint8 entry_num)
{
    if (iMuxDescriptorR[entry_num])
    {
        Delete_MultiplexEntryDescriptor(iMuxDescriptorR[entry_num]);
        OSCL_DEFAULT_FREE(iMuxDescriptorR[entry_num]);
        iMuxDescriptorR[entry_num] = NULL;
        iMuxDescriptorFlagR[entry_num] = DEACTIVATE;
    }
    return true;
}

PS_MultiplexEntryDescriptor
MuxTableMgr::GetOutgoingDescriptor(MuxSduDataList& data_list)
{
    unsigned num_lcns_with_data = data_list.size();
    for (unsigned desc_num = 0; desc_num < iOutgoingDescriptors->size(); desc_num++)
    {
        // check for num lcns match
        if ((*iOutgoingDescriptors)[desc_num]->NumLcns() == num_lcns_with_data)
        {
            // check for individual lcns
            MuxSduDataList::iterator it = data_list.begin();
            bool found_match = true;
            while (it != data_list.end())
            {
                MuxSduData& sdu_data = (*it++);
                TPVMuxDescriptorSlot slot;
                if (!(*iOutgoingDescriptors)[desc_num]->FindLcn((uint16)sdu_data.lcn->GetLogicalChannelNumber(), (uint16)(sdu_data.lcn->IsSegmentable() ? 0 : sdu_data.size), slot))
                {
                    found_match = false;
                    break;
                }
                if (sdu_data.lcn->GetLogicalChannelNumber() == 0)
                {
                    (*iOutgoingDescriptors)[desc_num]->GetH245descriptor()->multiplexTableEntryNumber = 0;
                    OsclRefCounterMemFrag fsi;
                    bool fsi_available = sdu_data.sdu->getFormatSpecificInfo(fsi);
                    if (fsi_available && fsi.getMemFragSize())
                    {
                        uint8* fsi_ptr = (uint8*)fsi.getMemFragPtr();
                        (*iOutgoingDescriptors)[desc_num]->GetH245descriptor()->multiplexTableEntryNumber = fsi_ptr[0];
                    }
                }
            }
            if (found_match)
            {
                return (*iOutgoingDescriptors)[desc_num]->GetH245descriptor();
            }
        }
    }
    return NULL;
}

PS_MultiplexEntryDescriptor MuxTableMgr::GetOutgoingDescriptor(OsclSharedPtr<H223OutgoingChannel>& lcn,
        PVMFSharedMediaDataPtr sdu)
{
    MuxSduData sdu_data;
    sdu_data.lcn = lcn;
    sdu_data.size = 0;
    sdu_data.sdu = sdu;
    iMuxSduDataList.push_back(sdu_data);
    PS_MultiplexEntryDescriptor ret = GetOutgoingDescriptor(iMuxSduDataList);
    iMuxSduDataList.clear();
    return ret;
}

PS_MultiplexEntryDescriptor MuxTableMgr::GetIncomingDescriptor(uint8 tblNum)
{
    int lookup = tblNum & 0x0F;
    PS_MultiplexEntryDescriptor desc = iMuxDescriptorR[lookup];
    PV_STAT_INCR_COND(iNumCorruptMcRx, 1, (desc == NULL))
    PV_STAT_INCR_COND(iMcAccessCntR[lookup], 1, desc);
    return desc;
}

OsclAny MuxTableMgr::ResetStats()
{
    iNumCorruptMcRx = 0;
    for (int32 entry_num = 0; entry_num < MAX_MUX_ENTRIES; entry_num ++)
    {
        iMcAccessCnt[entry_num] = 0;
        iMcAccessCntR[entry_num] = 0;
    }
}

OsclAny MuxTableMgr::LogStats(TPVDirection dir)
{
    if ((dir & OUTGOING) && iOutgoingDescriptors)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux-table(O) Statistics:\n"));
        for (unsigned entry = 0; entry < iOutgoingDescriptors->size(); entry++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num outgoing pdus using descriptor %d = %d\n",
                            (*iOutgoingDescriptors)[entry]->GetH245descriptor()->multiplexTableEntryNumber,
                            iMcAccessCnt[(*iOutgoingDescriptors)[entry]->GetH245descriptor()->multiplexTableEntryNumber]));
        }
    }
    if (dir & INCOMING)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux-table(I) Statistics:\n"));
        for (int32 entry_num = 0; entry_num < MAX_MUX_ENTRIES; entry_num ++)
        {
            if (iMuxDescriptorR[entry_num])
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num incoming pdus using descriptor %d = %d\n", entry_num, iMcAccessCntR[entry_num]));
            }
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num invalid descriptors - %d\n", iNumCorruptMcRx));
    }
}
uint32 MuxTableMgr::GetMuxEntryAccessCount(TPVDirection dir, uint8 tblNum)
{
    if (tblNum >= MAX_MUX_ENTRIES)
        return 0;
    uint32* table = (dir == OUTGOING) ? iMcAccessCnt : iMcAccessCntR;
    return table[tblNum];
}
