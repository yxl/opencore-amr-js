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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
/*****************************************************************************/
/*  file name            : tsc_mt.cpp                                        */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include    "tsc_mt.h"
#include    "tsc_sub.h"     /* Sub Routine Information Header                */
#include    "cpvh223multiplex.h"
#include    "tsc_component.h"


#define TSC_FM_H263_MTE 5
#define TSC_FM_MAX_MTE TSC_FM_H263_MTE
#define FIRST_MUX_ENTRY_NUMBER TSC_FM_MAX_MTE + 1
#define LAST_MUX_ENTRY_NUMBER 14

void TSC_mt::ClearVars()
{
    iToBeDeletedMuxEntryNumbers.clear();
    iOutMtSn = 0;
    iPendingMtSn = 0;
}

void TSC_mt::DeleteMuxEntry(uint32 aNum)
{
    iToBeDeletedMuxEntryNumbers.push_back(aNum);
}

void TSC_mt::InitVarsSession()
{
    // Initialize available multiplex entry numbers
    iAvailableMuxEntryNumbers.clear();
    for (int ii = TSC_FM_MAX_MTE + 1; ii <= LAST_MUX_ENTRY_NUMBER; ii++)
    {
        iAvailableMuxEntryNumbers.push_back(ii);
    }
    iOutMtSn = 0;
    iPendingMtSn = 0;
}

bool TSC_mt::MuxTableSendComplete(uint32 sn)
{
    if ((uint32)iPendingMtSn != sn)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "TSC_mt::MuxTableSendComplete Outdated multipex entry send sn(%d), last sn(%d)",
                         sn, iPendingMtSn));
        return false;
    }
    iPendingMtSn = -1;
    return true;
}


bool TSC_mt::CheckMtTrf(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> aOutCodecList,
                        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* aOutgoingChannelConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::CheckMtTrf"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,
                     "TSC_mt::CheckMtTrf codec list size(%d), num h223 channels(%d)",
                     aOutCodecList.size(), iH223->GetNumChannels(OUTGOING)));
    unsigned num_channels = (aOutgoingChannelConfig) ?
                            aOutgoingChannelConfig->size() : 0;
    num_channels = aOutCodecList.size() < num_channels ?
                   aOutCodecList.size() : num_channels;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::CheckMtTrf num opened h223 channels(%d)", num_channels));
    return (iH223->GetNumChannels(OUTGOING) >= num_channels) ? true : false;
}


/*****************************************************************************/
/*  function name        : Tsc_MtTrfRps                                      */
/*  function outline     : Tsc_MtTrfRps procedure                            */
/*  function discription : Tsc_MtTrfRps( void )                              */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_mt::MtTrfRps(uint32 sequenceNumber, PS_MuxDescriptor pMux)
{
    S_ControlMsgHeader infHeader;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mt_Trf_Rps,
                    sequenceNumber,
                    0,
                    (uint8*)pMux,
                    sizeof(S_MuxDescriptor));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    return;
}


/*****************************************************************************/
/*  function name        : Tsc_MtRjtReq                                      */
/*  function outline     : Tsc_MtRjtReq procedure                            */
/*  function discription : Tsc_MtRjtReq( pReceiveInf )                       */
/*  input data           : PS_ControlMsgHeader                                      */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career (x)   :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_mt::MtRjtReq()
{
    S_ControlMsgHeader infHeader;
    S_MeRejectCause parameter;

    // REJECT.request primitive - Parameter[CAUSE]
    parameter.index = 1;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mt_Rjt_Req,
                    0,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_MeRejectCause));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);
}

CPVMultiplexEntryDescriptor* TSC_mt::GenerateCombinedDescriptor(uint8 entry_num,
        TPVChannelId lcn1,
        unsigned lcn1_size,
        TPVChannelId lcn2)
{
    PS_MultiplexEntryDescriptor h245_desc =
        (PS_MultiplexEntryDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntryDescriptor));
    h245_desc->multiplexTableEntryNumber = entry_num;
    h245_desc->option_of_elementList = true;
    h245_desc->size_of_elementList = 2;

    h245_desc->elementList =
        (PS_MultiplexElement)OSCL_DEFAULT_MALLOC(2 * sizeof(S_MultiplexElement));
    oscl_memset(h245_desc->elementList, 0, 2 * sizeof(S_MultiplexElement));
    PS_MultiplexElement elem = h245_desc->elementList;
    elem->muxType.index = 0;
    elem->muxType.logicalChannelNumber = (uint16)lcn1;
    elem->muxType.size = 1;
    elem->repeatCount.index = 0;
    elem->repeatCount.finite = (uint16)lcn1_size;

    elem++;
    elem->muxType.index = 0;
    elem->muxType.logicalChannelNumber = (uint16)lcn2;
    elem->muxType.size = 1;
    elem->repeatCount.index = 1;
    CPVMultiplexEntryDescriptor* ret = CPVMultiplexEntryDescriptor::NewL(h245_desc, 128);
    Delete_MultiplexEntryDescriptor(h245_desc);
    OSCL_DEFAULT_FREE(h245_desc);
    return ret;
}


uint32 TSC_mt::SendMuxTableForLcn(TPVChannelId lcn)
{
    S_ControlMsgHeader infHeader;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::SendMuxTableForLcn lcn(%d)", lcn));
    CPVMultiplexEntryDescriptorVector descriptors;
    if (iAvailableMuxEntryNumbers.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_mt::SendMuxTableForLcn Ran out of mux table entries"));
        OSCL_LEAVE(PVMFErrNoResources);
    }
    int entry_num = iAvailableMuxEntryNumbers[0];
    iAvailableMuxEntryNumbers.erase(iAvailableMuxEntryNumbers.begin());
    CPVMultiplexEntryDescriptor* desc =
        iTSCcomponent->GenerateSingleDescriptor((uint8)entry_num, lcn);
    descriptors.push_back(desc);
    iH223->SetOutgoingMuxDescriptors(descriptors);

    PS_MuxDescriptor mux_descriptor =
        (PS_MuxDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_MuxDescriptor));
    oscl_memset(mux_descriptor, 0, sizeof(S_MuxDescriptor));
    mux_descriptor->size_of_multiplexEntryDescriptors = 1;
    mux_descriptor->multiplexEntryDescriptors = desc->GetH245descriptor();

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mt_Trf_Req,
                    0,
                    0,
                    (uint8*)mux_descriptor,
                    sizeof(S_MuxDescriptor));
    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    OSCL_DEFAULT_FREE(mux_descriptor);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0,
                     "TSC_mt::SendMuxTableForLcn lcn(%d), mt sn(%d), mt num(%d)",
                     lcn, iOutMtSn, entry_num));
    return entry_num;
}

void TSC_mt::ReleaseMuxTables()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::ReleaseMuxTable "));
    S_ControlMsgHeader infHeader;
    if (iToBeDeletedMuxEntryNumbers.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_mt::ReleaseMuxTable No MT entries to be released"));
        return;
    }
    unsigned mem_size = sizeof(S_MuxDescriptor) +
                        iToBeDeletedMuxEntryNumbers.size() * sizeof(S_MultiplexEntryDescriptor);
    uint8* mem_ptr = (uint8*)OSCL_DEFAULT_MALLOC(mem_size);
    if (mem_ptr == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_mt::ReleaseMuxTable ERROR:  Memory allocation failed."));
        return;
    }
    oscl_memset(mem_ptr, 0, mem_size);
    PS_MuxDescriptor mux_descriptor = (PS_MuxDescriptor)mem_ptr;
    mux_descriptor->size_of_multiplexEntryDescriptors =
        iToBeDeletedMuxEntryNumbers.size();
    mux_descriptor->multiplexEntryDescriptors =
        (PS_MultiplexEntryDescriptor)(mem_ptr + sizeof(S_MuxDescriptor));
    for (unsigned ii = 0; ii < iToBeDeletedMuxEntryNumbers.size(); ++ii)
    {
        uint8 mt_num = (uint8)iToBeDeletedMuxEntryNumbers[ii];
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "TSC_mt::ReleaseMuxTable Deleting Entry number=%d, sn=%d",
                         mt_num, iOutMtSn));
        iH223->RemoveOutgoingMuxDescriptor(mt_num);
        mux_descriptor->multiplexEntryDescriptors[ii].option_of_elementList =
            false;
        mux_descriptor->multiplexEntryDescriptors[ii].multiplexTableEntryNumber =
            (int8)mt_num;
        iAvailableMuxEntryNumbers.push_back(mt_num);
    }

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mt_Trf_Req,
                    0,
                    0,
                    (uint8*)mux_descriptor,
                    sizeof(S_MuxDescriptor));
    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);
    OSCL_DEFAULT_FREE(mem_ptr);
    iPendingMtSn = iOutMtSn++;
    iToBeDeletedMuxEntryNumbers.clear();
}

/**
*  Generates and send multiplex table entries for all logical channels for which MT state is idle/pending
**/
void TSC_mt::MtTrfReq(OlcList& aOlcs)
{
    S_ControlMsgHeader infHeader;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::MtTrfReq"));
    CPVMultiplexEntryDescriptorVector descriptors;
    CPVMultiplexEntryDescriptor* desc = NULL;
    Oscl_Vector<OlcParam*, OsclMemAllocator> olc_list;
    unsigned num_pending = aOlcs.FindOutgoingOlcsByMtState(MT_IDLE | MT_PENDING,
                           olc_list);

    if (num_pending == 0)
    {
        // No mux entries to send
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_mt::MtTrfReq No mux entries to send"));
        return;
    }

    if (iAvailableMuxEntryNumbers.size() < num_pending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0,
                         "TSC_mt::MtTrfReq Ran out of mux table entries - needed(%d), have(%d)",
                         num_pending, iAvailableMuxEntryNumbers.size()));
        OSCL_LEAVE(PVMFErrNoResources);
    }

    for (unsigned lcn = 0; lcn < num_pending; ++lcn)
    {
        int entry_num = iAvailableMuxEntryNumbers[0];
        iAvailableMuxEntryNumbers.erase(iAvailableMuxEntryNumbers.begin());
        TPVChannelId channel_id = (olc_list[lcn]->GetDirection() == OUTGOING) ?
                                  olc_list[lcn]->GetChannelId() :
                                  olc_list[lcn]->GetReverseParams()->GetChannelId();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_mt::MtTrfReq Descriptor for lcn(%d)=%d", channel_id, entry_num));
        desc = iTSCcomponent->GenerateSingleDescriptor((uint8)entry_num, channel_id);
        descriptors.push_back(desc);
        olc_list[lcn]->SetMtState(MT_PENDING);
        olc_list[lcn]->SetMtSn(iOutMtSn);
        olc_list[lcn]->SetMtNum(entry_num);
    }
    iH223->SetOutgoingMuxDescriptors(descriptors);

    PS_MultiplexEntryDescriptor temp = NULL;
    PS_MuxDescriptor mux_descriptor =
        (PS_MuxDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_MuxDescriptor));
    oscl_memset(mux_descriptor, 0, sizeof(S_MuxDescriptor));
    mux_descriptor->size_of_multiplexEntryDescriptors = descriptors.size();
    mux_descriptor->multiplexEntryDescriptors =
        (PS_MultiplexEntryDescriptor)OSCL_DEFAULT_MALLOC(
            sizeof(S_MultiplexEntryDescriptor) *
            mux_descriptor->size_of_multiplexEntryDescriptors);
    oscl_memset(mux_descriptor->multiplexEntryDescriptors,
                0,
                sizeof(S_MultiplexEntryDescriptor) *
                mux_descriptor->size_of_multiplexEntryDescriptors);
    PS_MultiplexEntryDescriptor cur_desc =
        mux_descriptor->multiplexEntryDescriptors;
    Oscl_Vector<PS_MultiplexEntryDescriptor, PVMFTscAlloc> to_be_deleted;

    for (unsigned num = 0; num < descriptors.size(); ++num)
    {
        temp = Copy_MultiplexEntryDescriptor(
                   descriptors[num]->GetH245descriptor());
        oscl_memcpy(cur_desc, temp, sizeof(S_MultiplexEntryDescriptor));
        cur_desc++;
        to_be_deleted.push_back(temp);
    }

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mt_Trf_Req,
                    0,
                    0,
                    (uint8*)mux_descriptor,
                    sizeof(S_MuxDescriptor));
    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    while (to_be_deleted.size())
    {
        PS_MultiplexEntryDescriptor desc = to_be_deleted.back();
        Delete_MultiplexEntryDescriptor(desc);
        OSCL_DEFAULT_FREE(desc);
        to_be_deleted.pop_back();
    }
    OSCL_DEFAULT_FREE(mux_descriptor->multiplexEntryDescriptors);
    OSCL_DEFAULT_FREE(mux_descriptor);

    iPendingMtSn = iOutMtSn++;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_mt::MtTrfReq iOutMtSn(%d)", iOutMtSn));
}
