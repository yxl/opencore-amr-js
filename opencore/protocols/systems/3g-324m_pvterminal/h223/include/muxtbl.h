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
#if !defined(MUXTBL_H)
#define MUXTBL_H
#include "oscl_types.h"
#include "h245def.h"
#include "h245pri.h"
#include "h223types.h"
#include "logicalchannel.h"

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#define MAX_MUX_ENTRIES 16
#define MAX_LCN_NUM 4

#define ACTIVATE 1
#define DEACTIVATE 0

class MuxTableMgr
{
    public:
        MuxTableMgr();
        ~MuxTableMgr();

        OsclAny SetIncomingDescriptors(PS_MuxDescriptor mux_desc);
        OsclAny SetIncomingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors, bool replace = true);
        void SetOutgoingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors);
        void RemoveOutgoingMuxDescriptor(uint8 muxTblNum);
        void RemoveIncomingMuxDescriptor(uint8 muxTblNum);
        PS_MultiplexEntryDescriptor GetIncomingDescriptor(uint8 tblNum);
        PS_MultiplexEntryDescriptor GetOutgoingDescriptor(MuxSduDataList& data_list);
        PS_MultiplexEntryDescriptor GetOutgoingDescriptor(OsclSharedPtr<H223OutgoingChannel>& lcn, PVMFSharedMediaDataPtr sdu);

        OsclAny ResetStats();
        OsclAny LogStats(TPVDirection dir);
        uint32 GetMuxEntryAccessCount(TPVDirection dir, uint8 tblNum);
    private:
        /* function prototypes */
        bool RemoveDescriptorR(uint8 tblNum);
        OsclAny AddIncomingControlDescriptor();
        void AddControlDescriptor(CPVMultiplexEntryDescriptorVector& descriptors);
        PS_MultiplexEntryDescriptor GetControlDescriptor(uint16 mux_entry_num);

        /* multiplex table information */
        uint16		iOutgoingMuxTblCount;
        uint16		iIncomingMuxTblCount;

        uint8		iMuxDescriptorFlagR[MAX_MUX_ENTRIES];
        PS_MultiplexEntryDescriptor	iMuxDescriptorR[MAX_MUX_ENTRIES];

        uint32 iNumCorruptMcRx;
        uint32 iMcAccessCnt[MAX_MUX_ENTRIES];
        uint32 iMcAccessCntR[MAX_MUX_ENTRIES];
        CPVMultiplexEntryDescriptorVector* iOutgoingDescriptors;
        MuxSduDataList iMuxSduDataList;
        PVLogger* iLogger;
};

#endif
