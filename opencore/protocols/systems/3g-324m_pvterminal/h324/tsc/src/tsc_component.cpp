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

#include "h223.h"
#include "tsc_h324m_config.h"
#include "tsc_component.h"
#include "tsc_constants.h"
#include "tsc_statemanager.h"
#include "tsc_capability.h"
#include "tsc_lc.h"
#include "tsc_blc.h"
#include "tsc_clc.h"
#include "tsc_channelcontrol.h"
#include "tsc_mt.h"
#ifdef MEM_TRACK
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#endif



#define PV_2WAY_TSC_WAIT_FOR_OBLC_TIMER_ID "PV_2WAY_TSC_WAIT_FOR_OBLC_TIMER"
#define PV_2WAY_TSC_WAIT_FOR_OBLC_TIMER_INTERVAL 1 /* 1 s */

#define TSC_MAX_OUTSTANDING_PREFMSG_PDUS 32


TSC_component::TSC_component(TSC_statemanager& aTSCStateManager,
                             TSC_capability& aTSCcapability,
                             TSC_lc& aTSClc,
                             TSC_blc& aTSCblc,
                             TSC_clc& aTSCclc,
                             TSC_mt& aTSCmt):
        iTSCstatemanager(aTSCStateManager),
        iTSCcapability(aTSCcapability),
        iTSClc(aTSClc),
        iTSCblc(aTSCblc),
        iTSCclc(aTSCclc),
        iTSCmt(aTSCmt),
        iH245(NULL),
        iH223(NULL),
        iLocalTcs(NULL),
        iWaitingForOblcTimer(NULL),
        iOutgoingChannelConfig(NULL),
        iIncomingChannelConfig(NULL),
        iTSCObserver(NULL),
        iTSCchannelcontrol(iOlcs, aTSCStateManager, aTSCblc,
                           aTSCmt, aTSClc, aTSCcapability, aTSCclc, *this)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h245user");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::TSC_component"));

}

void TSC_component::SetMembers(H245* aH245, H223* aH223, TSCObserver* aTSCObserver)
{
    iTSCObserver = aTSCObserver;
    iTSCchannelcontrol.SetMembers(aH223, aTSCObserver);
    iH245 = aH245;
    iH223 = aH223;
    MembersSet();
}


void TSC_component::InitVarsSession()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::InitVarsSession"));
    iRemoteAl1Audio = OFF;
    iRemoteAl2Audio = OFF;
    iRemoteAl3Audio = OFF;
    iRemoteAl1Video = OFF;
    iRemoteAl2Video = OFF;
    iRemoteAl3Video = OFF;
    iVideoLayer = PVT_AL_UNKNOWN;
    if (iLocalTcs)
    {
        Delete_TerminalCapabilitySet(iLocalTcs);
        OSCL_DEFAULT_FREE(iLocalTcs);
        iLocalTcs = NULL;
    }
    iRemoteTcs.Unbind();

    iOlcs.SetCurrLcn(FIRST_OUTGOING_LCN);
}

void TSC_component::InitVarsLocal()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::InitVarsLocal"));
    iAl3ControlFieldOctets = DEFAULT_AL3_CONTROL_FIELD_OCTETS;
    iAl2WithSn = true;

    iAllowAl1Video = ON;
    iAllowAl2Video = ON;
    iAllowAl3Video = ON;
    iAllowAl1Audio = OFF;
    iAllowAl2Audio = ON;
    iAllowAl3Audio = OFF;
    iUseAl1Video = true;
    iUseAl2Video = true;
    iUseAl3Video = true;
}

void TSC_component::InitTsc()
{
    iWaitingForOblcTimer = OSCL_NEW(OsclTimer<OsclMemAllocator>,
                                    (PV_2WAY_TSC_WAIT_FOR_OBLC_TIMER_ID, PV_2WAY_TSC_WAIT_FOR_OBLC_TIMER_INTERVAL));
    iWaitingForOblcTimer->SetObserver(this);
}

void TSC_component::ResetTsc()
{
    if (iOutgoingChannelConfig)
    {
        OSCL_DELETE(iOutgoingChannelConfig);
        iOutgoingChannelConfig = NULL;
    }

    if (iIncomingChannelConfig)
    {
        OSCL_DELETE(iIncomingChannelConfig);
        iIncomingChannelConfig = NULL;
    }

    if (iWaitingForOblcTimer)
    {
        iWaitingForOblcTimer->Clear();
        OSCL_DELETE(iWaitingForOblcTimer);
        iWaitingForOblcTimer = NULL;
    }
}

void TSC_component::Disconnect()
{

    iWaitingForOblcTimer->Clear();
}

CPVMultiplexEntryDescriptor* TSC_component::GenerateSingleDescriptor(uint8 entry_num,
        TPVChannelId lcn1)
{
    PS_MultiplexEntryDescriptor h245_desc =
        (PS_MultiplexEntryDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntryDescriptor));
    h245_desc->multiplexTableEntryNumber = entry_num;
    h245_desc->option_of_elementList = true;
    h245_desc->size_of_elementList = 1;
    h245_desc->elementList =
        (PS_MultiplexElement)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexElement));

    PS_MultiplexElement elem = 	h245_desc->elementList;
    elem->muxType.index = 0; // logical channel = 0, 1 = sub-elem list
    elem->muxType.logicalChannelNumber = (uint16)lcn1;
    elem->muxType.size = 1; // size of element list
    elem->repeatCount.index = 1; // ucf
    elem->repeatCount.finite = 0;

    CPVMultiplexEntryDescriptor* ret =
        CPVMultiplexEntryDescriptor::NewL(h245_desc, 128);
    Delete_MultiplexEntryDescriptor(h245_desc);
    OSCL_DEFAULT_FREE(h245_desc);

    return ret;
}

PS_AdaptationLayerType
TSC_component::GetOutgoingLayer(PV2WayMediaType media_type, uint32 max_sample_size)
{
    PS_AdaptationLayerType al_type =
        (PS_AdaptationLayerType)OSCL_DEFAULT_MALLOC(sizeof(S_AdaptationLayerType));
    int room_for_sn = 0;

    oscl_memset(al_type, 0, sizeof(S_AdaptationLayerType));
    uint32 max_sdu_size = 0;
    switch (media_type)
    {
        case PV_AUDIO:
            if (iRemoteAl2Audio)
            {
                max_sdu_size = iH223->GetSduSize(OUTGOING, E_EP_MEDIUM);
                room_for_sn = max_sdu_size - max_sample_size - 1; /* 1 byte for CRC */
                room_for_sn = (room_for_sn > 1) ? 1 : room_for_sn;
                if (room_for_sn >= 0)
                {
                    al_type->index = (uint16)(3 + room_for_sn);
                    return al_type;
                }
            }
            if (iRemoteAl3Audio)
            {
                max_sdu_size = iH223->GetSduSize(OUTGOING, E_EP_HIGH);
                room_for_sn = max_sdu_size - max_sample_size - 2; /* 2 bytes for CRC */
                if (room_for_sn >= 0)
                {
                    PS_Al3 al3 = (PS_Al3)OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
                    al3->controlFieldOctets = (int8)((room_for_sn >
                                                      (int)iAl3ControlFieldOctets) ? iAl3ControlFieldOctets : room_for_sn);
                    al3->sendBufferSize = DEF_AL3_SEND_BUFFER_SIZE;
                    al_type->index = 5;
                    al_type->al3 = al3;
                    return al_type;
                }
            }
            if (iRemoteAl1Audio)
            {
                max_sdu_size = iH223->GetSduSize(OUTGOING, E_EP_LOW);
                room_for_sn = max_sdu_size - max_sample_size;
                room_for_sn = (room_for_sn > 1) ? 1 : room_for_sn;
                if (max_sdu_size >= max_sample_size)
                {
                    al_type->index = 1;
                    return al_type;
                }
            }
            break;
        case PV_VIDEO:
            if (iUseAl2Video && iRemoteAl2Video)
            {
                al_type->index = (uint16)(iAl2WithSn ? 4 : 3);
                return al_type;
            }
            if (iUseAl3Video && iRemoteAl3Video)
            {
                PS_Al3 al3 = (PS_Al3)OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
                al3->controlFieldOctets = (int8)iAl3ControlFieldOctets;
                al3->sendBufferSize = DEF_AL3_SEND_BUFFER_SIZE;
                al_type->index = 5;
                al_type->al3 = al3;
                return al_type;
            }
            if (iUseAl1Video && iRemoteAl1Video)
            {
                al_type->index = 1;
                return al_type;
            }
            break;
        default:
            break;
    }
    OSCL_DEFAULT_FREE(al_type);
    return NULL;
}

void TSC_component::SetAlConfig(PV2WayMediaType media_type,
                                TPVAdaptationLayer layer,
                                bool allow)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::SetAlConfig"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::SetAlConfig media type(%d), layer(%d), allow(%d)",
                     media_type, layer, allow));

    switch (media_type)
    {
        case PV_AUDIO:
            if (layer == PVT_AL1)
                iAllowAl1Audio = allow;
            else if (layer == PVT_AL2)
                iAllowAl2Audio = allow;
            else if (layer == PVT_AL3)
                iAllowAl3Audio = allow;
            break;
        case PV_VIDEO:
            if (layer == PVT_AL1)
                iAllowAl1Video = allow;
            else if (layer == PVT_AL2)
                iAllowAl2Video = allow;
            else if (layer == PVT_AL3)
                iAllowAl3Video = allow;
            break;
        case PV_DATA:
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::SetAlConfig Invalid media type"));
    }
}

//////////////////////////////////////////////////////////////////////////
// Start the CE process by sending this terminals capabilites to the peer terminal
//////////////////////////////////////////////////////////////////////////
bool TSC_component::CEStart()
{
    if (!iIncomingChannelConfig || !iIncomingChannelConfig->size())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::CEStart- Incoming channel config not set"));
        return false;
    }

    iTSCstatemanager.WriteState(TSC_CE_SEND, STARTED);

    MultiplexCapabilityInfo mux_cap_info;
    mux_cap_info.iAllowAl1Video = iAllowAl1Video;
    mux_cap_info.iAllowAl2Video = iAllowAl2Video;
    mux_cap_info.iAllowAl3Video = iAllowAl3Video;
    mux_cap_info.iAllowAl1Audio = iAllowAl1Audio;
    mux_cap_info.iAllowAl2Audio = iAllowAl2Audio;
    mux_cap_info.iAllowAl3Audio = iAllowAl3Audio;
    mux_cap_info.iMaximumAl2SDUSize = iH223->GetSduSize(INCOMING, E_EP_MEDIUM);
    mux_cap_info.iMaximumAl3SDUSize = iH223->GetSduSize(INCOMING, E_EP_HIGH);
    Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> outgoing_codecs;
    Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> incoming_codecs;
    for (unsigned n = 0;n < iIncomingChannelConfig->size();n++)
    {
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* codecs =
            (*iIncomingChannelConfig)[n].GetCodecs();
        if (!codecs)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::CEStart No codecs specified for format type(%d)",
                             (*iIncomingChannelConfig)[n].GetMediaType()));
            continue;
        }
        for (unsigned m = 0;m < codecs->size();m++)
        {
            CodecCapabilityInfo* info = NULL;
            PVCodecType_t codec_type = PVMFFormatTypeToPVCodecType((*codecs)[m].format);
            TPVDirection dir = (*codecs)[m].dir;
            if (GetMediaType(codec_type) == PV_VIDEO)
            {
                info = new VideoCodecCapabilityInfo;
                ((VideoCodecCapabilityInfo*)info)->resolutions =
                    iTSCcapability.GetResolutions(dir);
            }
            else
            {
                info = new CodecCapabilityInfo;
            }
            info->codec = codec_type;
            info->dir = dir;
            incoming_codecs.push_back(info);
        }
    }
    if (iLocalTcs)
    {
        Delete_TerminalCapabilitySet(iLocalTcs);
        OSCL_DEFAULT_FREE(iLocalTcs);
        iLocalTcs = NULL;
    }
    iLocalTcs = GenerateTcs(mux_cap_info, outgoing_codecs, incoming_codecs);
    CustomGenerateTcs(iLocalTcs);

    CE* Ce = iH245->GetCE();
    if (Ce) Ce->TransferRequest(iLocalTcs);

    for (unsigned i = 0;i < incoming_codecs.size();i++)
    {
        delete incoming_codecs[i];
    }
    return true;
}

TPVStatusCode TSC_component::SetTerminalParam(CPVTerminalParam& params)
{
    CPVH324MParam* h324params = (CPVH324MParam*) & params;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component:SetTerminalParam - AL1(%d), AL2(%d), AL3(%d)",
                     h324params->iAllowAl1Video,
                     h324params->iAllowAl2Video,
                     h324params->iAllowAl3Video));

    iAllowAl1Video = h324params->iAllowAl1Video;
    iAllowAl2Video = h324params->iAllowAl2Video;
    iAllowAl3Video = h324params->iAllowAl3Video;
    iUseAl1Video = h324params->iUseAl1Video;
    iUseAl2Video = h324params->iUseAl2Video;
    iUseAl3Video = h324params->iUseAl3Video;

    iVideoLayer = h324params->iVideoLayer;

    return EPVT_Success;
}

void TSC_component::GetTerminalParam(CPVH324MParam& ah324param)
{
    ah324param.iAllowAl1Video = iAllowAl1Video ? true : false;
    ah324param.iAllowAl2Video = iAllowAl2Video ? true : false;
    ah324param.iAllowAl3Video = iAllowAl3Video ? true : false;
    ah324param.iVideoLayer = iVideoLayer;
}

bool TSC_component::IsSupported(TPVDirection dir,
                                PVCodecType_t codec,
                                FormatCapabilityInfo& capability_info)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::IsSupported dir(%d), codec(%d)", dir, codec));
    if (codec == PV_CODEC_TYPE_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::IsSupported No codec is always ok"));
        return true;
    }
    Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* config = (dir == OUTGOING) ?
            iOutgoingChannelConfig : iIncomingChannelConfig;
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::IsSupported No config available"));
        return false;
    }
    for (unsigned n = 0;n < config->size();n++)
    {
        H324ChannelParameters& param = (*config)[n];
        if (param.GetMediaType() != GetMediaType(codec))
            continue;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* codecs = param.GetCodecs();
        if (!codecs)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::IsSupported No codecs specified"));
            return false;
        }
        for (unsigned m = 0;m < codecs->size();m++)
        {
            if ((*codecs)[m].format == PVCodecTypeToPVMFFormatType(codec))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::IsSupported Match found"));
                capability_info = (*codecs)[m];
                return true;
            }
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::IsSupported No match found"));
    return false;
}

bool TSC_component::IsSupported(TPVDirection dir,
                                PV2WayMediaType media_type,
                                CodecCapabilityInfo& codec_info)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::IsSupported dir(%d), media_type(%d)", dir, media_type));
    if (media_type == PV_MEDIA_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::IsSupported No media is always ok"));
        return true;
    }
    Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* config = (dir == OUTGOING) ?
            iOutgoingChannelConfig : iIncomingChannelConfig;
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::IsSupported No config available"));
        return false;
    }
    for (unsigned n = 0;n < config->size();n++)
    {
        H324ChannelParameters& param = (*config)[n];
        if (param.GetMediaType() != media_type)
            continue;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* codecs = param.GetCodecs();
        if (!codecs)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::IsSupported No codecs specified"));
            return false;
        }
        codec_info.codec = PVMFFormatTypeToPVCodecType((*codecs)[0].format);
        codec_info.dir = (*codecs)[0].dir;
        return true;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::IsSupported No match found"));
    return false;
}
////////////////////////////////////////////////////////////////////////////
// ExtractTcsParameters()						(RAN-32K)
//
// This routine takes the incoming TerminalCapabilitySet
//   and extracts the following useful parameters:
//      {h263_qcifMPI, h263_maxBitRate, mpeg4_maxBitRate}
// The parameters are stored in globals and may be sent
//   later to the application.
////////////////////////////////////////////////////////////////////////////
void TSC_component::ExtractTcsParameters(PS_TerminalCapabilitySet pTcs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::ExtractTcsParameters"));

    if (pTcs->option_of_multiplexCapability)
    {
        PS_MultiplexCapability muxcaps = &pTcs->multiplexCapability;
        if (muxcaps->index == 2)
        {

            PS_H223Capability h223caps = muxcaps->h223Capability;
            iRemoteAl1Audio = h223caps->audioWithAL1;
            iRemoteAl2Audio = h223caps->audioWithAL2;
            iRemoteAl3Audio = h223caps->audioWithAL3;
            iRemoteAl1Video = h223caps->videoWithAL1;
            iRemoteAl2Video = h223caps->videoWithAL2;
            iRemoteAl3Video = h223caps->videoWithAL3;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::ExtractTcsParameters Remote audio caps AL1(%d), AL2(%d), AL3(%d)",
                             iRemoteAl1Audio, iRemoteAl2Audio, iRemoteAl3Audio));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::ExtractTcsParameters Remote video caps AL1(%d), AL2(%d), AL3(%d)",
                             iRemoteAl1Video, iRemoteAl2Video, iRemoteAl3Video));
            // -------------------------------------
            // Decide which Video Layer to use (RAN)
            // -------------------------------------
            /* If both terminals support AL2, use AL2 */
            if (iUseAl2Video && iRemoteAl2Video)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::ExtractTcsParameters Video Layer Decision is AL2"));
                iVideoLayer = PVT_AL2;
            }
            /* If not, check for mutual AL3 support */
            else if (iUseAl3Video && iRemoteAl3Video)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::ExtractTcsParameters Video Layer Decision is AL3"));
                iVideoLayer = PVT_AL3;
            }
            /* If not, check for mutual AL1 support */
            else if (iUseAl1Video && iRemoteAl1Video)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::ExtractTcsParameters Video Layer Decision is AL1"));
                iVideoLayer = PVT_AL1;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////
// CE User - Transfer.Indication primitive received from H.245
////////////////////////////////////////////////////////////////////////
void TSC_component::CETransferIndication(OsclSharedPtr<S_TerminalCapabilitySet> tcs,
        uint32 aTerminalStatus)
{
    if (aTerminalStatus == PhaseD_CSUP)
    {
        iRemoteTcs = tcs;
    }

    else if (aTerminalStatus == PhaseE_Comm)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::CETransferIndication TCS received during PhaseE_Comm"));
        if (tcs->option_of_multiplexCapability)
        {
            /* Save any required CE data */
            iRemoteAl2Video =
                tcs->multiplexCapability.h223Capability->videoWithAL2;
            iRemoteAl3Video =
                tcs->multiplexCapability.h223Capability->videoWithAL3;
        }
    }
}

// ========================================================
// SetAl2Al3VideoFlags()                              (RAN)
//
// New API from application layer.  Sets the flags as follows:
//   INPUT		gAllowAl2Video		gAllowAl3Video
//     0               ON                 OFF
//     1               OFF                ON
//     2               ON                 ON
// ========================================================
void TSC_component::SetAl2Al3VideoFlags(int32 userInput)
{
    iAllowAl2Video = iAllowAl3Video = ON;
    if (userInput == 0)
    {
        iAllowAl3Video = OFF;
    }
    else if (userInput == 1)
    {
        iAllowAl2Video = OFF;
    }
}

// ========================================================
// GetAl2Al3VideoFlags()                              (RAN)
//
// Complements SetAl2Al3VideoFlags()
// ========================================================
int32 TSC_component::GetAl2Al3VideoFlags(void)
{
    return(iAllowAl2Video + 2 * iAllowAl3Video - 1);
}

////////////////////////////////////////////////
//
////////////////////////////////////////////////
Oscl_Vector < H324ChannelParameters,
PVMFTscAlloc > * TSC_component::GetChannelConfig(TPVDirection dir)
{
    if (dir == OUTGOING)
    {
        return iOutgoingChannelConfig;
    }
    return iIncomingChannelConfig;
}

void TSC_component::SetAl3ControlFieldOctets(unsigned cfo)
{
    iAl3ControlFieldOctets = cfo;
}

void TSC_component::SetAl2Sn(int width)
{
    iAl2WithSn = width ? true : false;
}


/*****************************************************************************/
/*  function name        : LcEtbIdc           E_PtvId_Lc_Etb_Idc  */
/*  function outline     : Status04/Event09 procedure                        */
/*  function discription : Status04Event09( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_component::LcEtbIdc(PS_ControlMsgHeader  pReceiveInf)
{
    TPVChannelId OpenLcn = (TPVChannelId)pReceiveInf->InfSupplement1 +
                           TSC_INCOMING_CHANNEL_MASK;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcEtbIdc lcn(%d)", OpenLcn));
    PS_ForwardReverseParam pLcParam = (PS_ForwardReverseParam) pReceiveInf->pParameter;
    /* validate forRevParams */
    PVMFStatus forRevCheck = iTSCcapability.ValidateForwardReverseParams(pLcParam, INCOMING);
    if (forRevCheck != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                        (0, "TSC_component::LcEtbIdc ERROR  - Incoming forRevParams not supported.  Rejecting."));
        uint16 reason = (uint16)((forRevCheck == PVMFErrNotSupported) ? 2/* dataTypeNotSupported */ : 0/*unspecified*/);
        TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, reason);
        RemoveOlc(dir, OpenLcn);
        return PhaseE_Comm;
    }
    PS_H223LogicalChannelParameters pH223Lcp =
        pLcParam->forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters;
    PS_DataType pDataType = &pLcParam->forwardLogicalChannelParameters.dataType;
    PV2WayMediaType media_type  = PV_MEDIA_NONE;
    PVCodecType_t incoming_codec_type = PV_CODEC_TYPE_NONE;

    incoming_codec_type = ::GetCodecType(pDataType);
    media_type = ::GetMediaType(pDataType);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcEtbIdc lcn(%d), media type(%d), codec type(%d)",
                     OpenLcn, media_type, incoming_codec_type));

    OlcKey key(INCOMING, OpenLcn);

    if (iOlcs.count(key))
    {
        if (iOlcs[key]->GetState() == OLC_ESTABLISHED)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::LcEtbIdc Established incoming OLC found for same channel id. Rejecting OLC(%d)",
                             media_type, OpenLcn));
            TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 0);  // unspecified
            RemoveOlc(dir, OpenLcn);
            return PhaseE_Comm;
        }
        else
        {
            ReleasePendingIncomingChannel(OpenLcn);
        }
    }

    /* pending incoming OLC for the same media type */
    Oscl_Vector<OlcParam*, OsclMemAllocator> pending_olc_list;
    // Search for pending and established channels
    if (iOlcs.FindOlcs(INCOMING, media_type, OLC_PENDING | OLC_ESTABLISHED,
                       pending_olc_list))
    {
        for (unsigned i = 0; i < pending_olc_list.size(); i++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::LcEtbIdc Pending incoming OLC found for media type=%d, lcn=%d",
                             media_type, pending_olc_list[i]->GetChannelId()));
            if (!ReleasedPendingIncomingChannel(pending_olc_list[i]))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::LcEtbIdc Established incoming OLC found for same media type. Rejecting OLC(%d)",
                                 media_type, OpenLcn));
                TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 0); /* unspecified */
                RemoveOlc(dir, OpenLcn);
                return PhaseE_Comm;
            }
        }
    }

    OlcParam* pending_outgoing_olc = NULL;
    PVCodecType_t to_be_opened_codec = PV_CODEC_TYPE_NONE;

    // Add pending olc to list
    iOlcs.AppendOlc(INCOMING, OpenLcn, pDataType, pH223Lcp);
    // is there is a pending outgoing OLC for the same media type ?
    if (iOlcs.FindOlcs(OUTGOING, media_type, OLC_PENDING, pending_olc_list))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::LcEtbIdc Pending outgoing OLCs(%d) found for incoming media type(%d).",
                         pending_olc_list.size(), media_type, OpenLcn));
        OSCL_ASSERT(pending_olc_list.size() == 1);
        OlcParam* param = pending_outgoing_olc = pending_olc_list[0];
        if (param->GetDirectionality() == EPVT_BI_DIRECTIONAL)
        {
            pending_outgoing_olc = NULL;
            // There is an OLC/OBLC conflict
            if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == MASTER)
            {
                // We are master, hence we reject the OLC
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::LcEtbIdc BLC Already initiated by local. Rejecting OLC cause we are Master"));
                TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 10);  // Master slave conflict
                RemoveOlc(dir, OpenLcn);
                return PhaseE_Comm;
            }
            else  // We are Slave
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::LcEtbIdc BLC Already initiated by local. Closing BLC cause we are Slave"));
                to_be_opened_codec = GetCodecType(param->GetForwardParams()->GetDataType());;
                // Release the pending blc
                iTSCblc.BlcRlsReq(RELEASE_CLOSE, param->GetChannelId(), 0);
                ChannelReleased(OUTGOING, param->GetChannelId(), PVMFErrCancelled);
                // Logical channel will be deleted from the mux when the engine calls ReleasePort
            }
        }
    }


    PVMFStatus tscCheck = ValidateOlcsWithTcs();
    if (tscCheck != PVMFSuccess)
    {
        if (pending_outgoing_olc)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::LcEtbIdc pending olcs."));
            if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == MASTER)
            {
                // We are master, hence we reject the incoming OLC with code M/S conflict
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::LcEtbIdc TCS violated with pending olcs from Master."));
                TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 10);  // Master slave conflict
                RemoveOlc(dir, OpenLcn);
                return PhaseE_Comm;
            }
            /* We should close the conflicting codec and reopen a replacement.  Assumption here is that the
               conflict is due to the media type of the current incoming channel.  This will not work if there
               are symmetry inter-dependencies between audio and video codecs in the TCS.  */
            pending_outgoing_olc->GetForwardParams()->GetBitrate();
            iTSClc.LcRlsReq(RELEASE_CLOSE,
                            pending_outgoing_olc->GetChannelId(), 0);
            ChannelReleased(OUTGOING,
                            pending_outgoing_olc->GetChannelId(), PVMFErrCancelled);
            FormatCapabilityInfo codec_caps;
            if (IsSupported(OUTGOING, incoming_codec_type, codec_caps))
            {
                /* Verify if the capability sets can support it */
                TPVChannelId tmp_lcn = iOlcs.GetNextAvailLcn();
                iOlcs.AppendOlc(OUTGOING, tmp_lcn, pDataType, pH223Lcp);
                if (ValidateOlcsWithTcs() == PVMFSuccess)
                {
                    to_be_opened_codec = incoming_codec_type;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                    (0, "TSC_component::LcEtbIdc Cannot open a replacement channel as incoming codec is not supported"));
                }
                RemoveOlc(OUTGOING, tmp_lcn);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::LcEtbIdc Incoming codec not supported for transmit"));
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::LcEtbIdc no pending olcs."));
            // There were no pending OLCs from the local terminal.
            TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 3);  // dataTypeNotAvailable
            RemoveOlc(dir, OpenLcn);
            return PhaseE_Comm;
        }
    }

    int leave_status = OpenLogicalChannel(OpenLcn, pDataType, pH223Lcp);
    if (leave_status != 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::LcEtbIdc - Memory Allocation Failed."));
        return leave_status;
    }

    uint8* fsi = NULL;
    uint32 fsi_len = ::GetFormatSpecificInfo(pDataType, fsi);
    iTSCObserver->IncomingChannel(OpenLcn, incoming_codec_type, fsi, fsi_len);

    // ESTABLISH.response(LC) Primitive Send
    iTSClc.LcEtbRps(OpenLcn);

    SetCustomMultiplex(pReceiveInf, media_type);
    if (to_be_opened_codec == PV_CODEC_TYPE_NONE)
        return PhaseE_Comm;
    FormatCapabilityInfo fci;
    IsSupported(OUTGOING, to_be_opened_codec, fci);
    PS_AdaptationLayerType al_type = GetOutgoingLayer(media_type, fci.max_sample_size);
    if (al_type == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    OpenOutgoingChannel(to_be_opened_codec, al_type);
    Delete_AdaptationLayerType(al_type);
    OSCL_DEFAULT_FREE(al_type);
    iTSCmt.MtTrfReq(iOlcs);
    return PhaseE_Comm;
}

uint32 TSC_component::OpenLogicalChannel(TPVChannelId OpenLcn,
        PS_DataType pDataType,
        PS_H223LogicalChannelParameters pH223Lcp)
{
    int leave_status = 0;
    OlcParam* param = NULL;
    OSCL_TRY(leave_status, param = OpenLogicalChannel(INCOMING,
                                   OpenLcn, CHANNEL_ID_UNKNOWN, pDataType, pH223Lcp));
    OSCL_FIRST_CATCH_ANY(leave_status, void());
    if (leave_status != 0)
    {
        TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcn, 0);  // unspecified
        RemoveOlc(dir, OpenLcn);
        return PhaseE_Comm;
    }
    param->SetState(OLC_ESTABLISHED);
    return leave_status;
}

/*****************************************************************************/
/*  function name        : Status04Event14          E_PtvId_Blc_Etb_Idc  */
/*  function outline     : Status04/Event14 procedure                        */
/*  function discription : Status04Event14( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_component::BlcEtbIdc(PS_ControlMsgHeader  pReceiveInf)
{
    TPVChannelId OpenLcnB = pReceiveInf->InfSupplement1 + TSC_INCOMING_CHANNEL_MASK; /* incoming lcn */
    TPVChannelId OpenLcnF = CHANNEL_ID_UNKNOWN; /* outgoing lcn */
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::BlcEtbIdc lcn(%d)", OpenLcnB));
    /* Function prototypes */
    uint8* GetDecoderConfigFromOLC(PS_ForwardReverseParam pPara, uint32 forRev, uint16 *nOctets);
    PS_ForwardReverseParam forRevParams = (PS_ForwardReverseParam)pReceiveInf->pParameter;

    /* validate forRevParams */
    PVMFStatus forRevCheck = iTSCcapability.ValidateForwardReverseParams(forRevParams, INCOMING);
    if (forRevCheck != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                        (0, "TSC_component::BlcEtbIdc ERROR  - Incoming forRevParams not supported.  Rejecting."));
        uint16 reason = (uint16)((forRevCheck == PVMFErrNotSupported) ? 2/* dataTypeNotSupported */ : 0/*unspecified*/);
        TPVDirection dir = iTSClc.LcRlsReq(RELEASE_REJECT, OpenLcnB, reason);
        RemoveOlc(dir, OpenLcnB);
        return PhaseE_Comm;
    }

    PS_ForwardLogicalChannelParameters forwardParams = &forRevParams->forwardLogicalChannelParameters;
    PS_ReverseLogicalChannelParameters reverseParams = &forRevParams->reverseLogicalChannelParameters;
    PVCodecType_t in_codec_type = ::GetCodecType(&forwardParams->dataType);
    PVCodecType_t out_codec_type = ::GetCodecType(&reverseParams->dataType);
    PV2WayMediaType out_media_type = ::GetMediaType(&reverseParams->dataType);
    OSCL_UNUSED_ARG(out_media_type);
    PV2WayMediaType in_media_type = ::GetMediaType(&forwardParams->dataType);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::BlcEtbIdc in media type(%d), in codec type(%d), out media type(%d), out codec type(%d)", in_media_type, in_codec_type, out_media_type, out_codec_type));

    /* Do we support the outgoing codec ? */
    FormatCapabilityInfo codec_caps;
    if (!IsSupported(OUTGOING, out_codec_type, codec_caps) ||
            !IsSupported(INCOMING, in_codec_type, codec_caps))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::BlcEtbIdc Outgoing/incoming codec not supported for transmit"));
        TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 2); /* dataTypeNotSupported */
        RemoveOlc(dir, OpenLcnB);
        return(PhaseE_Comm);
    }

    unsigned outgoing_bitrate = 0;
    if (out_codec_type != PV_CODEC_TYPE_NONE)
    {
        outgoing_bitrate = GetOutgoingBitrate(out_codec_type);
        if (outgoing_bitrate <= 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::BlcEtbIdc No bandwidth allocated for outgoing media type(%d)",
                             out_media_type));
            TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 3); /* dataTypeNotAvailable */
            RemoveOlc(dir, OpenLcnB);
            return PhaseE_Comm ;
        }
    }

    /* Cancel waiting for OBLC */
    if (iWaitingForOblc)
    {
        if (out_codec_type != iWaitingForOblcCodec)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::BlcEtbIdc Reverse codec in BLC(%d) does not match iWaitingForOblcCodec(%d)",
                             out_codec_type, iWaitingForOblcCodec));
        }
    }
    iWaitingForOblc = false;
    iWaitingForOblcTimer->Clear();
    iWaitingForOblcCodec = PV_CODEC_TYPE_NONE;

    Oscl_Vector<OlcParam*, OsclMemAllocator> pending_olc_list;
    /* Pending incoming OLC for the same media type */
    if (iOlcs.FindOlcs(INCOMING, in_media_type, OLC_PENDING | OLC_ESTABLISHED, pending_olc_list))
    {
        for (unsigned i = 0; i < pending_olc_list.size(); i++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::BlcEtbIdc Pending incoming OLC found for media type=%d, lcn=%d",
                             in_media_type, pending_olc_list[i]->GetChannelId()));
            if (!ReleasedPendingIncomingChannel(pending_olc_list[i]))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::BlcEtbIdc Established incoming OLC found for same media type. Rejecting OLC(%d)", in_media_type, OpenLcnB));
                TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 0); /* unspecified */
                RemoveOlc(dir, OpenLcnB);
                return PhaseE_Comm;
            }

        }
    }

    /* if there are established outgoing channels for the same media type, close them */
    if (out_codec_type != PV_CODEC_TYPE_NONE &&
            iOlcs.FindOlcs(OUTGOING, in_media_type, OLC_ESTABLISHED, pending_olc_list))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::BlcEtbIdc Established outgoing OLC found for same media type(%d). Closing down the OLCS", in_media_type));
        for (unsigned i = 0; i < pending_olc_list.size(); i++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::BlcEtbIdc Established outgoing OLC found for media type=%d, lcn=%d",
                             in_media_type, pending_olc_list[i]->GetChannelId()));
            ChannelReleased(OUTGOING, pending_olc_list[i]->GetChannelId(), PVMFFailure);
        }
    }

    PVCodecType_t to_be_opened_codec = PV_CODEC_TYPE_NONE;
    /* is there is a pending outgoing OLC for the same media type ?*/
    if (iOlcs.FindOlcs(OUTGOING, in_media_type, OLC_PENDING, pending_olc_list))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::BlcEtbIdc Pending outgoing OLCs(%d) found for incoming media type(%d).",
                         pending_olc_list.size(), in_media_type, OpenLcnB));
        OSCL_ASSERT(pending_olc_list.size() == 1);
        OlcParam* param = pending_olc_list[0];
        if (param->GetDirectionality() == EPVT_BI_DIRECTIONAL || /* Bi-dir: always causes conflict*/
                out_codec_type != PV_CODEC_TYPE_NONE)  /*Uni-dir: no conflict if reverse params are NULL */
        {
            /* OLCs are conflicting */
            if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == MASTER)
            {
                /* We are master, hence we reject the incoming OLC */
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::BlcEtbIdc BLC Already initiated by local. Rejecting OBLC cause we are Master"));
                TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 10);  /* Master slave conflict */
                RemoveOlc(dir, OpenLcnB);
                return PhaseE_Comm ;
            }
            /* we are slave */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::BlcEtbIdc LC Already initiated by local. Closing it cause we are Slave"));
            if (out_codec_type == PV_CODEC_TYPE_NONE)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::BlcEtbIdc Reverse DataType==NULL; Need to open replacement OLC/OBLC"));
                to_be_opened_codec = GetCodecType(param->GetForwardParams()->GetDataType());
            }
            ReleaseOlc(param, 0);
            ChannelReleased(OUTGOING, param->GetChannelId(), PVMFErrCancelled);
        }
    }
    pending_olc_list.clear();
    /* Generate logical channel number */
    OpenLcnF = iOlcs.GetNextAvailLcn();
    OlcParam* prm = iOlcs.AppendOlc(INCOMING, OpenLcnB,
                                    &forwardParams->dataType,
                                    forwardParams->multiplexParameters.h223LogicalChannelParameters,
                                    OpenLcnF,
                                    &reverseParams->dataType,
                                    reverseParams->rlcMultiplexParameters.h223LogicalChannelParameters);

    prm->GetReverseParams()->SetChannelId(OpenLcnF);
    /* Validate the TCS's */
    PVMFStatus tscCheck = ValidateOlcsWithTcs();
    bool transfer_mux_tables = false;
    if (tscCheck != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::BlcEtbIdc TCS check failed"));
        TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 2);  /* dataTypeNotSupported */
        RemoveOlc(dir, OpenLcnB);
    }
    else
    {
        PVMFStatus RvsParametersOkay = VerifyReverseParameters(forRevParams, iTSCObserver);
        if (RvsParametersOkay == PVMFSuccess)
        {
            AcceptBLCRequest(OpenLcnF, OpenLcnB, forRevParams);
        }
        else
        {
            /* RvsParameters are unsuitable; reject the OLC */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::BlcEtbIdc - Rejecting BLC request (%d).  Reverse parameters not ok.",
                             OpenLcnB));
            TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_REJECT, OpenLcnB, 1);  /* unsuitableReverseParameters */
            RemoveOlc(dir, OpenLcnB);
            /* Fill the data type for the outgoing codec */
            PS_DataType pDataType = GetOutgoingDataType(out_codec_type, outgoing_bitrate);
            /*  Fill the outgoing h223 logical channel parameters */
            PS_H223LogicalChannelParameters pH223Params =
                GetH223LogicalChannelParameters((uint8)IndexForAdaptationLayer(PVT_AL3),
                                                iTSCcapability.IsSegmentable(OUTGOING, in_media_type),
                                                iAl3ControlFieldOctets);

            // Use AL3 for OBLC
            PS_AdaptationLayerType al_type = (PS_AdaptationLayerType)OSCL_DEFAULT_MALLOC(sizeof(S_AdaptationLayerType));
            if (al_type == NULL)
            {
                OSCL_LEAVE(PVMFErrNoMemory);
            }
            PS_Al3 al3 = (PS_Al3)OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
            al3->controlFieldOctets = iAl3ControlFieldOctets;
            al3->sendBufferSize = DEF_AL3_SEND_BUFFER_SIZE;
            al_type->index = 5;
            al_type->al3 = al3;

            OpenOutgoingChannel(out_codec_type,
                                al_type,
                                &forRevParams->forwardLogicalChannelParameters.dataType,
                                forRevParams->forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);

            iOlcs.AppendOlc(OUTGOING, OpenLcnF, pDataType, pH223Params,
                            CHANNEL_ID_UNKNOWN, &forRevParams->forwardLogicalChannelParameters.dataType,
                            forRevParams->forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters);
            transfer_mux_tables = true;
            Delete_AdaptationLayerType(al_type);
            OSCL_DEFAULT_FREE(al_type);
            Delete_DataType(pDataType);
            OSCL_DEFAULT_FREE(pDataType);
            Delete_H223LogicalChannelParameters(pH223Params);
            OSCL_DEFAULT_FREE(pH223Params);
        }
    }

    if (to_be_opened_codec != PV_CODEC_TYPE_NONE)
    {
        FormatCapabilityInfo fci;
        IsSupported(OUTGOING, to_be_opened_codec, fci);
        PS_AdaptationLayerType al_type = GetOutgoingLayer(::GetMediaType(to_be_opened_codec),
                                         fci.max_sample_size);
        if (al_type == NULL)
        {
            OSCL_LEAVE(PVMFErrNoMemory);
        }
        OpenOutgoingChannel(to_be_opened_codec, al_type);
        Delete_AdaptationLayerType(al_type);
        OSCL_DEFAULT_FREE(al_type);
        transfer_mux_tables = true;
    }
    if (transfer_mux_tables)
    {
        iTSCmt.MtTrfReq(iOlcs);
    }
    return(PhaseE_Comm);
}

PVMFStatus TSC_component::VerifyReverseParameters(PS_ForwardReverseParam forRevParams,
        TSCObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::VerifyReverseParameters"));
    PVMFStatus status;
    bool returnNow = iTSCcapability.VerifyReverseParameters(forRevParams, iTSCObserver, status);

    if (returnNow)
    {
        return status;
    }
    PVCodecType_t codec = GetCodecType(&forRevParams->reverseLogicalChannelParameters.dataType);
    PV2WayMediaType media_type = GetMediaType(codec);
    uint8* decodeConfigInfoOblc = NULL;
    unsigned decodeConfigInfoSzOblc =
        ::GetFormatSpecificInfo(&forRevParams->reverseLogicalChannelParameters.dataType,
                                decodeConfigInfoOblc);
    // get the outgoing FSI if any
    for (unsigned n = 0;n < iOutgoingChannelConfig->size();n++)
    {
        PV2WayMediaType channelMediaType = (*iOutgoingChannelConfig)[n].GetMediaType();
        if (channelMediaType != media_type)
        {
            continue;
        }
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* formats =
            (*iOutgoingChannelConfig)[n].GetCodecs();
        if (!formats)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::VerifyReverseParameters No formats specified for media type(%d)",
                             channelMediaType));
            continue;
        }
        for (unsigned m = 0;m < formats->size();m++)
        {
            PVCodecType_t codec_type_outgoing = PVMFFormatTypeToPVCodecType((*formats)[m].format);
            if (codec_type_outgoing != codec)
                continue;
            if ((*formats)[m].fsi == NULL || (*formats)[m].fsi_len == 0)
            {
                // There are no FSI restrictions
                return PVMFSuccess;
            }
            if (decodeConfigInfoSzOblc == (*formats)[m].fsi_len &&
                    oscl_memcmp(decodeConfigInfoOblc, (*formats)[m].fsi, decodeConfigInfoSzOblc) == 0)
            {
                return PVMFSuccess;
            }
        }
    }

    return PVMFFailure;
}


PVMFStatus TSC_component::ValidateOlcsWithTcs()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::ValidateOlcsWithTcs"));
    /* Verify if local TCS is satisfied */
    Oscl_Vector<OlcFormatInfo, OsclMemAllocator> incoming_codecs;
    if (!iOlcs.FindCodecs(INCOMING, PV_MEDIA_NONE, OLC_ESTABLISHED,
                          PV_DIRECTION_BOTH, incoming_codecs))
        return PVMFSuccess;
    /* Set symmetry info in the codecs */
    unsigned n = 0;
    for (n = 0;n < incoming_codecs.size();n++)
    {
        PV2WayMediaType media_type = GetMediaType(incoming_codecs[n].iCodec);
        incoming_codecs[n].isSymmetric = iOlcs.IsSymmetric(media_type,
                                         PV_DIRECTION_BOTH, OLC_PENDING | OLC_ESTABLISHED,
                                         PV_DIRECTION_BOTH, OLC_PENDING | OLC_ESTABLISHED);
    }
    PVMFStatus status = VerifyCodecs(iLocalTcs, incoming_codecs, iLogger);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::ValidateOlcsWithTcs Codecs not compatible with local TCS"));
        return status;
    }

    /* Verify if remote TCS is satisfied */
    Oscl_Vector<OlcFormatInfo, OsclMemAllocator> outgoing_codecs;
    if (!iOlcs.FindCodecs(OUTGOING, PV_MEDIA_NONE,
                          OLC_PENDING | OLC_ESTABLISHED, PV_DIRECTION_BOTH, outgoing_codecs))
        return PVMFSuccess;
    /* Set symmetry info in the codecs */
    for (n = 0;n < outgoing_codecs.size();n++)
    {
        PV2WayMediaType media_type = GetMediaType(outgoing_codecs[n].iCodec);
        outgoing_codecs[n].isSymmetric = iOlcs.IsSymmetric(media_type,
                                         PV_DIRECTION_BOTH,
                                         OLC_PENDING | OLC_ESTABLISHED,
                                         PV_DIRECTION_BOTH,
                                         OLC_PENDING | OLC_ESTABLISHED);
    }
    status = VerifyCodecs(iRemoteTcs, outgoing_codecs, iLogger);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::ValidateOlcsWithTcs Codecs not compatible with remote TCS"));
    }
    return status;
}

OsclAny TSC_component::TcsMsdComplete()
{
    ClipCodecs(iRemoteTcs);

    if (!iOutgoingChannelConfig || iOutgoingChannelConfig->size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::TcsMsdComplete No outgoing channels configured(%x)",
                         iOutgoingChannelConfig));
        return;
    }

    //Oscl_Vector<OlcParam*, OsclMemAllocator> olc_list;
    CPVMultiplexEntryDescriptorVector descriptors;

    // start OLCs
    for (unsigned olcnum = 0; olcnum < iOutgoingChannelConfig->size(); olcnum++)
    {
        int index = -1;
        PV2WayMediaType media_type = (*iOutgoingChannelConfig)[olcnum].GetMediaType();
        if (!FindCodecForMediaType(media_type, iOutCodecList, &index))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::TcsMsdComplete No outgoing codec selected for media type=%d",
                             media_type));
            continue;
        }
        if (AlreadyAssigned(media_type))
        {
            continue;
        }


        PVCodecType_t incoming_codec = PV_CODEC_TYPE_NONE;
        FormatCapabilityInfo fci;
        IsSupported(OUTGOING, iOutCodecList[index]->codec, fci);
        PS_AdaptationLayerType al_type = GetOutgoingLayer(media_type, fci.max_sample_size);
        if (al_type == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::TcsMsdComplete Failed to allocate adaptation layer for media type=%d",
                             media_type));
            continue;
        }
        PS_DataType pDataTypeRvs = NULL;
        PS_H223LogicalChannelParameters pH223ParamsRvs = NULL;
        if (al_type->index == 5) /* AL3 */
        {
            incoming_codec = iOutCodecList[index]->codec;
            pDataTypeRvs = GetOutgoingDataType(incoming_codec,
                                               GetOutgoingBitrate(incoming_codec));
            pH223ParamsRvs = GetH223LogicalChannelParameters((uint8)IndexForAdaptationLayer(PVT_AL3),
                             iTSCcapability.IsSegmentable(INCOMING, media_type),
                             iAl3ControlFieldOctets);
        }

        OlcParam* olc_param = OpenOutgoingChannel(iOutCodecList[index]->codec,
                              al_type, pDataTypeRvs, pH223ParamsRvs);
        if (pH223ParamsRvs)
        {
            Delete_H223LogicalChannelParameters(pH223ParamsRvs);
            OSCL_DEFAULT_FREE(pH223ParamsRvs);
        }
        if (pDataTypeRvs)
        {
            Delete_DataType(pDataTypeRvs);
            OSCL_DEFAULT_FREE(pDataTypeRvs);
        }
        Delete_AdaptationLayerType(al_type);
        OSCL_DEFAULT_FREE(al_type);
        StartOlc(olc_param, media_type, descriptors);
    }
    if (FinishTcsMsdComplete(descriptors))
    {
        iTSCmt.MtTrfReq(iOlcs);
    }
    // Reset flags used to force AL
    iUseAl1Video = true;
    iUseAl2Video = true;
    iUseAl3Video = true;
#ifdef MEM_TRACK
    printf("\n Memory Stats After TcsMsdComplete");
    MemStats();
#endif
}

void TSC_component::SetOutgoingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& out_channel_config)
{
    if (iOutgoingChannelConfig)
    {
        OSCL_DELETE(iOutgoingChannelConfig);
        iOutgoingChannelConfig = NULL;
    }
    iOutgoingChannelConfig = new Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>(out_channel_config);

}

void TSC_component::SetIncomingChannelConfig(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& in_channel_config)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::SetIncomingChannelConfig size(%d)\n", in_channel_config.size()));
    if (iIncomingChannelConfig)
    {
        OSCL_DELETE(iIncomingChannelConfig);
        iIncomingChannelConfig = NULL;
    }
    iIncomingChannelConfig = new Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>(in_channel_config);
}

/*****************************************************************************/
/*  function name        : Status04Event10           E_PtvId_Lc_Etb_Cfm  */
/*  function outline     : Status04/Event10 procedure                        */
/*  function discription : Status04Event10( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_component::LcEtbCfm(PS_ControlMsgHeader pReceiveInf)
{
    TPVChannelId lcn = pReceiveInf->InfSupplement1;
    /*OlcParam* olc_param = OpenLogicalChannel(OUTGOING, lcn);
    }
    return PhaseE_Comm;
    */
    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(OUTGOING, lcn);
    if (olc_param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::LcEtbCfm ERROR Unable to lookup channel"));
        return (PhaseE_Comm);
    }
    olc_param->SetState(OLC_ESTABLISHED);
    CheckOutgoingChannel(olc_param, PVMFSuccess);
    return PhaseE_Comm;

}

unsigned TSC_component::GetOutgoingBitrate(PVCodecType_t codec_type)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::GetOutgoingBitrate codec_type=%d", codec_type));
    PV2WayMediaType media_type = GetMediaType(codec_type);
    if (!iOutgoingChannelConfig || !iOutgoingChannelConfig->size())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::GetOutgoingBitrate outgoing channel config not found."));
        return 0;
    }
    uint32 bitrate = 0;
    for (unsigned n = 0; n < iOutgoingChannelConfig->size(); n++)
    {
        H324ChannelParameters& params = (*iOutgoingChannelConfig)[n];
        if (params.GetMediaType() == media_type)
        {
            bitrate = params.GetBandwidth();
            break;
        }
    }
    if (bitrate == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::GetOutgoingBitrate outgoing channel bitrate=0."));
        return bitrate;
    }
    // lookup the bitrate from remote capabilities
    uint32 br = iTSCcapability.GetRemoteBitrate(codec_type);
    bitrate = (bitrate > br) ? br : bitrate;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::GetOutgoingBitrate outgoing channel bitrate=%d.", bitrate));
    return bitrate;
}

void TSC_component::GetChannelFormatAndCapabilities(TPVDirection dir,
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& formats)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::GetChannelFormatAndCapabilities"));
    OlcList::iterator it = iOlcs.begin();

    while (it != iOlcs.end())
    {
        OlcList::value_type& val = (*it++);
        OlcParam* olc = val.second;
        H223ChannelParam* param = NULL;
        if (olc->GetDirection() == dir)
        {
            param = olc->GetForwardParams();
        }
        else if (olc->GetReverseParams())
        {
            param = olc->GetReverseParams();
        }
        FormatCapabilityInfo fci;
        fci.id = param->GetChannelId();
        fci.dir = dir;
        fci.format = PVCodecTypeToPVMFFormatType(param->GetMediaParam()->GetCodecType());
        fci.bitrate = param->GetBitrate();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::GetChannelFormatAndCapabilities Adding(%d,%d,%s,%d) to list",
                         fci.dir, fci.id, fci.format.getMIMEStrPtr(), fci.bitrate));
        formats.push_back(fci);
    }
}

bool TSC_component::HasOlc(TPVDirection direction,
                           TPVChannelId id,
                           unsigned state)
{
    if (state)
    {
        return iOlcs.HasOlc(direction, id, state);
    }
    else
    {
        return iOlcs.HasOlc(direction, id);
    }
}

OlcParam* TSC_component::FindOlcGivenChannel(TPVDirection direction,
        TPVChannelId id)
{
    return iOlcs.FindOlcGivenChannel(direction, id);
}

OlcParam* TSC_component::FindOlc(TPVDirection direction,
                                 PV2WayMediaType media_type,
                                 unsigned state)
{
    return iOlcs.FindOlc(direction, media_type, state);
}

void TSC_component::LcnDataDetected(TPVChannelId lcn)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcnDataDetected - lcn=%d", lcn));
    OlcParam* param = iOlcs.FindOlcGivenChannel(INCOMING, lcn);
    if (param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::LcnDataDetected - Failed to lookup channel, lcn=%d", lcn));
        return;
    }
    if (param->GetReplacementFor() != CHANNEL_ID_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::LcnDataDetected - Replaced channel id=%d",
                         param->GetReplacementFor()));
        ChannelReleased(INCOMING, param->GetReplacementFor(), PV2WayErrReplaced);
        param->SetReplacementFor((TPVChannelId)NULL);
    }
}

// =======================================================
// AcceptBLCRequest()
//
// WWUAPI - New Function
// =======================================================
OsclAny TSC_component::AcceptBLCRequest(TPVChannelId OpenLcnF,
                                        TPVChannelId OpenLcnB,
                                        PS_ForwardReverseParam forRevParams)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC: AcceptBLCRequest reverse(%d), forward(%d)\n", OpenLcnB, OpenLcnF));
    /* RvsParameters are okay; accept the OLC */
    iTSCblc.BlcEtbRps(OpenLcnB - TSC_INCOMING_CHANNEL_MASK, OpenLcnF);
    // Open the incoming and outgoing logical channels in the mux
    OlcParam* param = OpenLogicalChannel(INCOMING,
                                         OpenLcnB,
                                         OpenLcnF,
                                         &forRevParams->forwardLogicalChannelParameters.dataType,
                                         forRevParams->forwardLogicalChannelParameters.multiplexParameters.h223LogicalChannelParameters,
                                         &forRevParams->reverseLogicalChannelParameters.dataType,
                                         forRevParams->reverseLogicalChannelParameters.rlcMultiplexParameters.h223LogicalChannelParameters);
    param->SetState(OLC_ESTABLISHED);
    /* Send updated MuxTable for outgoing part of the BLC */
    iTSCmt.MtTrfReq(iOlcs);

    uint8* fsi = NULL;
    uint32 fsi_len = ::GetFormatSpecificInfo(&forRevParams->forwardLogicalChannelParameters.dataType, fsi);
    iTSCObserver->IncomingChannel(OpenLcnB,
                                  GetCodecType(&forRevParams->forwardLogicalChannelParameters.dataType),
                                  fsi, fsi_len);

    if (OpenLcnF == CHANNEL_ID_UNKNOWN ||
            GetCodecType(param->GetReverseParams()->GetDataType()) == PV_CODEC_TYPE_NONE)
        return;

    // Pause the channel untill OlcAck+MtAck is received
    H223OutgoingChannelPtr outgoing_channel;
    iH223->GetOutgoingChannel(OpenLcnF, outgoing_channel);
    outgoing_channel->Pause();
    // Notify outgoing channel
    fsi_len = GetFormatSpecificInfo(&forRevParams->reverseLogicalChannelParameters.dataType, fsi);
    iTSCObserver->OutgoingChannelEstablished(OpenLcnF,
            GetCodecType(&forRevParams->reverseLogicalChannelParameters.dataType),
            fsi, fsi_len);
}

// =============================================================
// Status04Event11()                     E_PtvId_Lc_Rls_Idc
//
// This is LCSE RELEASE.indication.
// =============================================================
uint32 TSC_component::LcRlsIdc(PS_ControlMsgHeader  pReceiveInf)
{
    TPVDirection dir = (pReceiveInf->Dir == S_ControlMsgHeader::INCOMING) ? INCOMING : OUTGOING;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcRlsIdc dir(%d), lcn(%d).",
                     dir, pReceiveInf->InfSupplement1));
    TPVChannelId lcn = (dir == INCOMING) ? (TPVChannelId)(pReceiveInf->InfSupplement1 +
                       TSC_INCOMING_CHANNEL_MASK) : (TPVChannelId)pReceiveInf->InfSupplement1;
    PS_SourceCause_LcBlc sourceCause = (PS_SourceCause_LcBlc)pReceiveInf->pParameter;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcRlsIdc sourceCause(%x), cause index(%d)",
                     sourceCause, sourceCause->Cause.index));
    PVMFStatus status = PVMFSuccess;

    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(dir, lcn);
    if (olc_param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::LcRlsIdc ERROR Unable to lookup channel"));
        return (PhaseE_Comm);
    }

    if (dir == OUTGOING)
    {
        if (olc_param->GetState() == OLC_PENDING)
        {
            // only valid rejection code is m/s conflict
            if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == SLAVE &&
                    sourceCause->Cause.index == 10)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::LcRlsIdc Reject due to M/S conflict"));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::LcRlsIdc Olc failure"));
                status = PVMFFailure;
            }
        }
    }
    else if (dir == INCOMING)
    {
        RemoveMultiplex(olc_param);
    }
    ChannelReleased(dir, lcn, status);
    return(PhaseE_Comm);
}

// =============================================================
// Status04Event16()                   E_PtvId_Blc_Rls_Idc
//
// This is BLCSE RELEASE.indication.  It is called when
// a Bi-Dir OLCReject is received.  It could be from an incoming or outgoing SE
// =============================================================
uint32 TSC_component::BlcRlsIdc(PS_ControlMsgHeader  pReceiveInf)
{
    PS_SourceCause_LcBlc sourceCause = (PS_SourceCause_LcBlc)pReceiveInf->pParameter;
    int32 causeIndex = sourceCause->Cause.index;
    TPVDirection dir = OUTGOING;
    TPVChannelId lcn = (TPVChannelId)pReceiveInf->InfSupplement1;;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::BlcRlsIdc BLC rejected. dir(%d), forward lcn(%d), reverse lcn(%d), cause index(%d)",
                     dir, pReceiveInf->InfSupplement1, pReceiveInf->InfSupplement2, causeIndex));

    if (pReceiveInf->Dir == S_ControlMsgHeader::INCOMING)
    {
        dir = INCOMING;
        lcn += TSC_INCOMING_CHANNEL_MASK;
    }

    PVMFStatus status = PVMFSuccess;
    PVCodecType_t to_be_opened_codec = PV_CODEC_TYPE_NONE;
    PV2WayMediaType media_type = PV_MEDIA_NONE;

    if (dir == OUTGOING)
    {
        OlcParam* olc_param = iOlcs.FindOlcGivenChannel(OUTGOING, lcn);
        if (olc_param != NULL)
        {
            media_type = GetMediaType(olc_param->GetForwardParams()->GetDataType());
            if (olc_param->GetState() == OLC_PENDING)
            {
                if (causeIndex == 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "TSC_component::BlcRlsIdc UnsuitableReverseParams. Entering wait state ..."));
                    /* Cause is unsuitableReverseParameters; Enter wait state */
                    iWaitingForOblc = true;
                    iWaitingForOblcCodec = GetCodecType(olc_param->GetForwardParams()->GetDataType());
                    iWaitingForOblcTimer->Request(PV_TSC_WAITING_FOR_OBLC_TIMER_ID,
                                                  PV_TSC_WAITING_FOR_OBLC_TIMER_ID , WAITING_FOR_OBLC_TIMEOUT_SECONDS, this);
                }
                else if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == SLAVE &&
                         causeIndex == 10)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "TSC_component::BlcRlsIdc Master/Slave conflict"));
                    // is there is a pending outgoing OLC for the same media type ?
                    Oscl_Vector<OlcParam*, OsclMemAllocator> pending_olc_list;
                    if (iOlcs.FindOlcs(OUTGOING, media_type, OLC_PENDING, pending_olc_list))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                        (0, "TSC_component::BlcRlsIdc Pending outgoing OLCs(%d) found for incoming media type(%d).",
                                         pending_olc_list.size(), media_type));
                        OSCL_ASSERT(pending_olc_list.size() == 1);
                        OlcParam* param = pending_olc_list[0];
                        to_be_opened_codec = GetCodecType(param->GetForwardParams()->GetDataType());
                    }
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "TSC_component::BlcRlsIdc Olc failure"));
                    status = PVMFFailure;
                }
            }
        }
    }
    else if (dir == INCOMING)
    {
        lcn += TSC_INCOMING_CHANNEL_MASK;
    }
    ChannelReleased(dir, lcn, status);

    if (to_be_opened_codec == PV_CODEC_TYPE_NONE)
        return PhaseE_Comm;
    FormatCapabilityInfo fci;
    IsSupported(OUTGOING, to_be_opened_codec, fci);
    PS_AdaptationLayerType al_type = GetOutgoingLayer(media_type, fci.max_sample_size);
    if (al_type == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    OpenOutgoingChannel(to_be_opened_codec, al_type);
    Delete_AdaptationLayerType(al_type);
    OSCL_DEFAULT_FREE(al_type);
    iTSCmt.MtTrfReq(iOlcs);
    return(PhaseE_Comm);
}

OsclAny TSC_component::MuxTableSendComplete(uint32 sn, PVMFStatus status)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC::MuxTableSendComplete sn(%d), status(%d)", sn, status));
    if (!iTSCmt.MuxTableSendComplete(sn))
    {
        return;
    }
    Oscl_Vector<OlcParam*, OsclMemAllocator> olc_list;
    unsigned num_pending = iOlcs.FindOutgoingOlcsByMtState(MT_PENDING, olc_list);
    for (unsigned lcn = 0;lcn < num_pending;lcn++)
    {
        //OSCL_ASSERT(olc_list[lcn]->GetMtSn()==sn);
        olc_list[lcn]->SetMtState(status == PVMFSuccess ? MT_COMPLETE : MT_RELEASED);
        if (olc_list[lcn]->GetState() == OLC_ESTABLISHED)
        {
            CheckOutgoingChannel(olc_list[lcn], status);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::MuxTableSendComplete Mux table completed, but channel still pending. channel id=(%d)", olc_list[lcn]->GetChannelId()));
        }
    }
    iTSCmt.ReleaseMuxTables();
}

// ===============================================================
// StopData()
//
// Sets flags in the H324 system table which stop data transmission
// for each open, outgoing logical channel.
// ================================================================
OsclAny TSC_component::StopData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::StopData"));
    OlcList::iterator it = iOlcs.begin();
    while (it != iOlcs.end())
    {
        OlcParam* olc = (*it++).second;
        iH223->StopChannel(olc->GetDirection(), olc->GetChannelId());
        if (olc->GetReverseParams())
        {
            iH223->StopChannel(olc->GetReverseParams()->GetDirection(),
                               olc->GetReverseParams()->GetChannelId());
        }
    }
}

// bool whether level is unknown
bool TSC_component::Connect1LevelKnown()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_NOTICE,
                    (0, "TSC_component::Connect"));
    bool levelknown = true;
    iWaitingForOblc = false;
    iWaitingForOblcCodec = PV_CODEC_TYPE_NONE;


    if (iOutgoingChannelConfig == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    if (iIncomingChannelConfig == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    iWaitingForOblcTimer->Clear();
    return levelknown;
}

void TSC_component::Connect2()
{
    H223PduParcomSharedPtr parcom;
    iH223->Start(parcom);
}

// =======================================================
// ClipCodecs()									(RAN-32K)
//
// This one reconciles the desired outgoing codecs with the
//   capabilities of the remote terminal.  The outgoing
//   codecs have been specified by the application via calls
//   to pH324.SetVideoType(), pH324.SetAudioType().  The
//   remote terminal capabilities from the received
//   TerminalCapabilitySet are passed in (pTcs).
// If insufficient capabilities are found, the routine will
//   modify the outgoing codecs stored in pH324.
// =======================================================
OsclAny TSC_component::ClipCodecs(PS_TerminalCapabilitySet pTcs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::ClipCodecs(%d,%d)",
                     pTcs->size_of_capabilityTable,
                     pTcs->size_of_capabilityDescriptors));
    if (!(pTcs->option_of_capabilityTable && pTcs->option_of_capabilityDescriptors))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::ClipCodecs - Remote terminal is incapable of decoding anything"));
        return;
    }
    if (!iOutgoingChannelConfig || iOutgoingChannelConfig->size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::ClipCodecs - We dont want to send anything.  Skip ClipCodecs."));
        return;
    }
    PS_CapabilityDescriptor pCapDesc = NULL;
    PS_AlternativeCapabilitySet pAltCapSet = NULL;
    for (unsigned i = 0;i < pTcs->size_of_capabilityDescriptors;++i)
    {
        pCapDesc = pTcs->capabilityDescriptors + i;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::ClipCodecs - descriptor(%d),size_of_simultaneousCapabilities(%d)",
                         i, pCapDesc->size_of_simultaneousCapabilities));
        /* Temporary list of selected codecs.  Retain the codecs for descriptor with most matching codecs */
        Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> codec_list;
        /* A changing list of available media for outgoing channels.  Once a codec is selected,
           the entry for that media is removed from this list */
        Oscl_Vector<H324ChannelParameters, PVMFTscAlloc> outgoing_media(*iOutgoingChannelConfig);
        if (pCapDesc->size_of_simultaneousCapabilities != iOutgoingChannelConfig->size())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_component::ClipCodecs - descriptor(%d),size_of_simultaneousCapabilities does not match num channels(%d)", i, iOutgoingChannelConfig->size()));
        }

        for (unsigned j = 0;j < pCapDesc->size_of_simultaneousCapabilities;++j)
        {
            bool txOnly = true;
            pAltCapSet = pCapDesc->simultaneousCapabilities + j;
            // Get the list of codecs in this ACS
            Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> codecs_acs;
            uint16 num_media_types = GetCodecCapabilityInfo(pTcs, pAltCapSet, codecs_acs);
            if (num_media_types != 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::ClipCodecs - ERROR:  Badly formed AlternativeCapabilitySet j=%d, num media types=%d", j, num_media_types));
                Deallocate(codecs_acs);
                continue;
            }

            //Check for tx only capabilities
            for (unsigned k = 0; k < codecs_acs.size(); ++k)
            {
                if (codecs_acs[k]->dir != OUTGOING)
                {
                    txOnly = false;
                    break;
                }
            }
            if (txOnly)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::ClipCodecs: tx only codecs"));
                Deallocate(codecs_acs);
                continue;
            }

            PV2WayMediaType mediaType = GetMediaType(codecs_acs[0]->codec);
            if (mediaType != PV_AUDIO && mediaType != PV_VIDEO)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                                (0, "TSC_component::ClipCodecs:  Media type in AlternativeCapabilitySet j=%d is neither audio/video, media type=%d", j, mediaType));
                Deallocate(codecs_acs);
                continue;
            }
            // Get the list of our incoming codecs for this media type
            Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator> in_codecs_for_media_type;
            iTSCcapability.GetSupportedCodecCapabilityInfo(INCOMING, mediaType, in_codecs_for_media_type);

            // Do either side have symmetry constraints ?
            bool local_has_symmetry_constraint = iTSCcapability.HasSymmetryConstraint(in_codecs_for_media_type);
            bool remote_has_symmetry_constraint = iTSCcapability.HasSymmetryConstraint(codecs_acs);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::ClipCodecs mediaType=%d, local_has_symmetry_constraint=%d, remote_has_symmetry_constraint=%d", mediaType, local_has_symmetry_constraint, remote_has_symmetry_constraint));
            CodecCapabilityInfo* selected_codec_info;
            if (local_has_symmetry_constraint || remote_has_symmetry_constraint)
            {
                selected_codec_info = iTSCcapability.SelectOutgoingCodec(&codecs_acs,
                                      &in_codecs_for_media_type);
            }
            else
            {
                selected_codec_info = iTSCcapability.SelectOutgoingCodec(&codecs_acs);
            }
            if (selected_codec_info)
                codec_list.push_back(selected_codec_info->Copy());
            Deallocate(codecs_acs);
            Deallocate(in_codecs_for_media_type);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::ClipCodecs codec_list size=%d, iOutCodecList size=%d",
                         codec_list.size(), iOutCodecList.size()));
        if (codec_list.size() > iOutCodecList.size())
        {
            /* found a match */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::ClipCodecs Match found for descriptor(%d), size(%d)",
                             i, pCapDesc->size_of_simultaneousCapabilities));
            Deallocate(iOutCodecList);
            iOutCodecList = codec_list;
            for (unsigned num = 0;num < iOutCodecList.size();num++)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_component::ClipCodecs Selected codec(%d)=%d",
                                 num, codec_list[num]->codec));
            }
        }
        else
        {
            Deallocate(codec_list);
        }
    }
}

void TSC_component::Start()
{
    if (iOutgoingChannelConfig == NULL || iIncomingChannelConfig == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::Start here"));
    }

}

void TSC_component::StartDisconnect(bool terminate)
{
    if (terminate)
    {
        StopData();
        CloseChannels();
        Deallocate(iOutCodecList);
    }
}

void TSC_component::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timeoutInfo);
    if (timerID == PV_TSC_WAITING_FOR_OBLC_TIMER_ID)
    {
        iWaitingForOblc = false;
        iWaitingForOblcCodec = PV_CODEC_TYPE_NONE;
    }
}

/*****************************************************************************/
/*  function name        : Status08Event19          E_PtvId_Clc_Cls_Idc  */
/*  function outline     : Status08/Event19 procedure                        */
/*  function discription : Status08Event19( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_component::Status08Event19(PS_ControlMsgHeader pReceiveInf)
{
    TPVChannelId ClcLcn = CHANNEL_ID_UNKNOWN;
    uint32 Directional = 0;          /* UNI=1, BI=2 */

    /* (RECEIVED AN INCOMING RequestChannelClose) */
    /* Input parameters */
    ClcLcn = (TPVChannelId)pReceiveInf->InfSupplement1;         /* Channel requested to be closed */
    Directional = pReceiveInf->InfSupplement2;    /* Directionality */
    if (Directional != 1 && Directional != 2)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::Status08Event19 Invalid directionality"));
        iTSCclc.ClcRjtReq(ClcLcn);
        return (PhaseE_Comm);
    }

    if (!HasOlc(OUTGOING, ClcLcn))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_component::Status08Event19 unable to lookup outgoing channel id(%d)",
                         ClcLcn));
        iTSCclc.ClcRjtReq(ClcLcn);
        return (PhaseE_Comm);
    }

    /* Primitive Send */
    iTSCclc.ClcClsRps(ClcLcn);            /* Send RCCAck Message */

    ChannelReleased(OUTGOING, ClcLcn, PVMFSuccess);

    /* Send the H.245 CloseLogicalChannel message */
    if (Directional == 1)          /* UniDirectional */
    {
        TPVDirection dir = iTSClc.LcRlsReq(RELEASE_CLOSE, ClcLcn, 0);
        RemoveOlc(dir, ClcLcn);
    }
    else if (Directional == 2)     /* BiDirectional */
    {
        TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_CLOSE, ClcLcn, 0);
        RemoveOlc(dir, ClcLcn);
    }
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status04Event15          E_PtvId_Blc_Etb_Cfm  */
/*  function outline     : Status04/Event15 procedure                        */
/*  function discription : Status04Event15( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment career(x)  :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
uint32 TSC_component::BlcEtbCfm(PS_ControlMsgHeader pReceiveInf)
{
    TPVChannelId incoming_lcn = pReceiveInf->InfSupplement2 + TSC_INCOMING_CHANNEL_MASK;
    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(OUTGOING, pReceiveInf->InfSupplement1);
    if (olc_param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::BlcEtbCfm ERROR Unable to lookup channel"));
        return PhaseE_Comm;
    }
    olc_param->SetState(OLC_ESTABLISHED);

    // Resume the channel if MT is also complete
    CheckOutgoingChannel(olc_param, PVMFSuccess);

    if (olc_param->GetReverseParams())
    {
        // Set the incoming logical channel number
        olc_param->GetReverseParams()->SetChannelId(incoming_lcn);
        // Open the incoming channel in the mux
        OpenPort(INCOMING,
                 olc_param->GetReverseParams()->GetChannelId(),
                 olc_param->GetReverseParams());

        uint8* fsi = NULL;
        uint32 fsi_len = ::GetFormatSpecificInfo(olc_param->GetReverseParams()->GetDataType(), fsi);
        iTSCObserver->IncomingChannel(incoming_lcn,
                                      GetCodecType(olc_param->GetReverseParams()->GetDataType()),
                                      fsi, fsi_len);
    }
    return PhaseE_Comm;
}


// =============================================================
// Status04Event50()                  E_PtvId_Blc_Etb_Cfm2
//
// This is "BLCSE ESTABLISH.confirm2"
// It is called when SE receives an OLCConfirm (Bi-Dir).
// =============================================================
uint32 TSC_component::BlcEtbCfm2(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::BlcEtbCfm2 forward(%d), reverse(%d))\n",
                     pReceiveInf->InfSupplement1, pReceiveInf->InfSupplement2));
    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(INCOMING,
                          pReceiveInf->InfSupplement1 + TSC_INCOMING_CHANNEL_MASK);
    if (olc_param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::BlcEtbCfm ERROR Unable to lookup channel"));
        return PhaseE_Comm;
    }
    olc_param->SetState(OLC_ESTABLISHED);
    // Resume the channel if MT is also complete
    CheckOutgoingChannel(olc_param, PVMFSuccess);
    return PhaseE_Comm;
}


/*****************************************************************************/
/*  function name        : Status08Event12           E_PtvId_Lc_Rls_Cfm  */
/*  function outline     : Status08/Event12 procedure                        */
/*  function discription : Status08Event12( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '00.4.13                                          */
/*---------------------------------------------------------------------------*/
uint32 TSC_component::LcRlsCfm(PS_ControlMsgHeader  pReceiveInf)
{
    TPVChannelId lcn = (TPVChannelId) pReceiveInf->InfSupplement1;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcRlsCfm - lcn(%d)\n", lcn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::LcRlsCfm - lcn(%d)\n", lcn));
    ChannelReleased(OUTGOING, lcn, PVMFSuccess);
    return(PhaseE_Comm);
}

/*****************************************************************************/
/*  function name        : Status08Event17          E_PtvId_Blc_Rls_Cfm  */
/*  function outline     : Status08/Event17 procedure                        */
/*  function discription : Status08Event17( pReceiveInf )                */
/*  input data           : PS_ControlMsgHeader         Receive InfHeader Pointer    */
/*  output data          : uint32                 Terminal Status              */
/*  draw time            : '00.4.13                                         */
/*---------------------------------------------------------------------------*/
/* RAN - Bi-Dir OLCAck */
uint32 TSC_component::BlcRlsCfm(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::BlcRlsCfm - forward(%d), reverse(%d)",
                     pReceiveInf->InfSupplement1, pReceiveInf->InfSupplement2));
    ChannelReleased(OUTGOING,
                    pReceiveInf->InfSupplement1, PVMFSuccess);
    return PhaseE_Comm;
}

LogicalChannelInfo* TSC_component::GetLogicalChannelInfo(PVMFPortInterface& port)
{
    return iTSCchannelcontrol.GetLogicalChannelInfo(port);
}

void TSC_component::ReceivedFormatSpecificInfo(TPVChannelId channel_id,
        uint8* fsi,
        uint32 fsi_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::ReceivedFormatSpecificInfo lcn=%d, len=%d",
                     channel_id, fsi_len));
    iTSCchannelcontrol.ReceivedFormatSpecificInfo(channel_id, fsi, fsi_len);
}

bool TSC_component::IsEstablishedLogicalChannel(TPVDirection aDir,
        TPVChannelId aChannelId)
{
    return iTSCchannelcontrol.IsEstablishedLogicalChannel(aDir, aChannelId);
}


void TSC_component::RemoveOlc(TPVDirection dir, TPVChannelId lcn)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::RemoveOlc lcn(%d), dir(%d)", lcn, dir));
    OlcKey key(dir, lcn);
    OlcList::iterator iter = iOlcs.find(key);
    if (iter == iOlcs.end())
        return;
    OlcParam* param = (*iter).second;

    if ((dir == OUTGOING || param->GetReverseParams()) &&
            (param->GetMtNum() && (param->GetMtSn() >= 0)) &&
            IsRemovable(lcn))
    {
        iTSCmt.DeleteMuxEntry(param->GetMtNum());
    }

    iTSCmt.ReleaseMuxTables();

    if (iter != iOlcs.end())
    {
        delete(*iter).second;
        iOlcs.erase(iter);
    }
}

void TSC_component::ReleaseOlc(OlcParam* olc, uint16 cause)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::ReleaseOlc olc(%x), reason(%d))", olc, cause));
    int release_type = (olc->GetDirection() == OUTGOING) ? RELEASE_CLOSE : RELEASE_REJECT;

    if (olc->GetDirectionality() == EPVT_UNI_DIRECTIONAL)
    {
        iTSClc.LcRlsReq(release_type, olc->GetChannelId(), cause);
    }
    else
    {
        iTSCblc.BlcRlsReq(release_type, olc->GetChannelId(), cause);
    }
}

/* This function just does the h.245 signalling for closing a logical channel - incoming/outgoing */
void TSC_component::SignalChannelClose(TPVDirection dir,
                                       TPVChannelId lcn,
                                       TPVDirectionality directionality)
{
    /* Multiplex entries must have failed.  Close the channel */
    if (dir == OUTGOING)
    {
        if (directionality == EPVT_BI_DIRECTIONAL)
        {
            TPVDirection dir = iTSCblc.BlcRlsReq(RELEASE_CLOSE, lcn, 0);
            RemoveOlc(dir, lcn);
        }
        else
        {
            TPVDirection dir = iTSClc.LcRlsReq(RELEASE_CLOSE, lcn, 0);
            RemoveOlc(dir, lcn);
        }
    }
    else
    {
        iTSCclc.ClcClsReq(lcn);
    }
}

void TSC_component::CheckOutgoingChannel(OlcParam* olc_param, PVMFStatus status)
{
    TPVChannelId id = (olc_param->GetDirection() == OUTGOING) ?
                      olc_param->GetChannelId() : olc_param->GetReverseParams()->GetChannelId();
    PVCodecType_t codec_type = olc_param->GetDirection() == OUTGOING ?
                               GetCodecType(olc_param->GetForwardParams()->GetDataType()) :
                               GetCodecType(olc_param->GetReverseParams()->GetDataType());
    OSCL_UNUSED_ARG(codec_type);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::CheckOutgoingChannel channel id=%d, codec type=%d, status=%d",
                     olc_param->GetChannelId(), codec_type, status));
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::CheckOutgoingChannel Channel open failed"));
        ChannelReleased(OUTGOING, olc_param->GetChannelId(), status);
        return;
    }
    if (olc_param->GetMtState() != MT_COMPLETE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::CheckOutgoingChannel channel id=%d Mux table send not complete",
                         olc_param->GetChannelId()));
        return;
    }
    // OLC+MT is complete.  Resume the channel.
    H223OutgoingChannelPtr outgoing_channel;
    status = iH223->GetOutgoingChannel(id, outgoing_channel);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::CheckOutgoingChannel ERROR Failed to lookup channel."));
        return;
    }
    outgoing_channel->Resume();
    // Request fast update from the engine
    iTSCObserver->RequestFrameUpdate(outgoing_channel);
}

OsclAny TSC_component::ChannelReleased(TPVDirection dir, TPVChannelId lcn, PVMFStatus status)
{
    OSCL_UNUSED_ARG(status);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::ChannelReleased dir(%d), lcn(%d), status(%d)",
                     dir, lcn, status));
    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(dir, lcn);
    if (olc_param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::ChannelReleased Failed to lookup channel(%d), lcn(%d)\n",
                         dir, lcn));
        return;
    }

    if (dir == OUTGOING || olc_param->GetState() == OLC_ESTABLISHED)
    {
        iTSCObserver->ChannelClosed(dir,
                                    lcn,
                                    ::GetCodecType(olc_param->GetForwardParams()->GetDataType()),
                                    status);
    }
    if (olc_param->GetDirectionality() == EPVT_BI_DIRECTIONAL)
    {
        TPVChannelId rvs_lcn = olc_param->GetReverseParams()->GetChannelId();
        if (rvs_lcn &&
                (rvs_lcn != CHANNEL_ID_UNKNOWN) &&
                (dir == INCOMING || olc_param->GetState() == OLC_ESTABLISHED))
        {
            iTSCObserver->ChannelClosed(REVERSE_DIR(dir),
                                        rvs_lcn,
                                        ::GetCodecType(olc_param->GetReverseParams()->GetDataType()),
                                        status);
        }
    }

    RemoveOlc(dir, lcn);
}

void TSC_component::CloseChannels()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::CloseChannels "));
    OlcList::iterator it = iOlcs.begin();
    while (it != iOlcs.end())
    {
        OlcParam* olc = (*it++).second;
        ChannelReleased(olc->GetDirection(), olc->GetChannelId(), PVMFSuccess);
        it = iOlcs.begin();
    }
}


OlcParam* TSC_component::OpenLogicalChannel(TPVDirection dir,
        TPVChannelId lcn,
        TPVChannelId lcnRvs,
        PS_DataType dt,
        PS_H223LogicalChannelParameters lcp,
        PS_DataType dtRvs,
        PS_H223LogicalChannelParameters lcpRvs)
{

    OSCL_UNUSED_ARG(lcpRvs);
    OSCL_UNUSED_ARG(dtRvs);
    OSCL_UNUSED_ARG(lcp);
    OSCL_UNUSED_ARG(dt);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::OpenLogicalChannel dir(%d), lcn(%d), dt(%x), lcp(%x), lcnRvs(%d), dtRvs(%x), lcpRvs(%x)\n", dir, lcn, dt, lcp, lcnRvs, dtRvs, lcpRvs));
    // add to list of channels
    OlcParam* olc_param = iOlcs.FindOlcGivenChannel(dir, lcn);
    if (olc_param == NULL)
    {
        SignalChannelClose(dir, lcn, lcnRvs == CHANNEL_ID_UNKNOWN ? EPVT_UNI_DIRECTIONAL : EPVT_BI_DIRECTIONAL);
        return NULL;
    }

    /* Is this a replacement channel outgoing channel ? */
    if (dir == OUTGOING && olc_param->GetReplacementFor() != CHANNEL_ID_UNKNOWN)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::OpenLogicalChannel Replacement for outgoing channel id=%d",
                         olc_param->GetReplacementFor()));
        ChannelReleased(OUTGOING, olc_param->GetReplacementFor(), PV2WayErrReplaced);
        olc_param->SetReplacementFor(0);
    }

    if (olc_param->GetReverseParams())
        olc_param->GetReverseParams()->SetChannelId(lcnRvs);

    TPVDirection rvs_dir = REVERSE_DIR(dir);
    OpenPort(dir, lcn, olc_param->GetForwardParams());
    if (olc_param->GetReverseParams() &&
            olc_param->GetReverseParams()->GetChannelId() != CHANNEL_ID_UNKNOWN)
    {
        OpenPort(rvs_dir, olc_param->GetReverseParams()->GetChannelId(),
                 olc_param->GetReverseParams());
    }
    return olc_param;
}
void TSC_component::OpenPort(TPVDirection dir, TPVChannelId lcn, H223ChannelParam* param)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::OpenPort dir(%d), lcn(%d), param(%x)\n",
                     dir, lcn, param));
    PVCodecType_t codec_type = ::GetCodecType(param->GetDataType());
    if (codec_type == PV_CODEC_TYPE_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::OpenPort codec_type==NONE", codec_type));
        return;
    }
    iH223->OpenChannel(dir, lcn, param);
}


PS_DataType TSC_component::GetOutgoingDataType(PVCodecType_t codecType,
        uint32 bitrate)
{
    uint8* csi = NULL;
    uint16 csi_len = 0;
    return iTSCcapability.GetOutgoingDataType(codecType, bitrate, csi_len, csi);
}

OlcParam* TSC_component::OpenOutgoingChannel(PVCodecType_t out_codec_type,
        PS_AdaptationLayerType adaptation_layer,
        PS_DataType pDataTypeRvs,
        PS_H223LogicalChannelParameters pH223ParamsRvs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_component::OpenOutgoingChannel codec(%d), layer(%x)",
                     out_codec_type, adaptation_layer));
    OlcParam* ret = NULL;
    // allocate a channel id for this channel
    TPVChannelId channel_id = iOlcs.GetNextAvailLcn();
    uint32 bitrate = GetOutgoingBitrate(out_codec_type);

    /* Fill the data type for the outgoing codec */
    PS_DataType pDataType = GetOutgoingDataType(out_codec_type, bitrate);

    /*  Fill the outgoing h223 logical channel parameters */
    PS_H223LogicalChannelParameters pH223Params =
        iTSCcapability.GetOutgoingLcnParams(GetMediaType(out_codec_type),
                                            adaptation_layer);
    if (adaptation_layer->index == 5)
    { /* AL3 */
        S_DataType nullDataType;
        if (pDataTypeRvs == NULL)
        {
            nullDataType.index = 1;
            pDataTypeRvs = &nullDataType;
        }
        if (pH223ParamsRvs == NULL)
        {
            pH223ParamsRvs = pH223Params;
        }
        PVCodecType_t in_codec_type = GetCodecType(pDataTypeRvs);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_component::OpenOutgoingChannel reverse codec(%d)", in_codec_type));
        if (in_codec_type != out_codec_type)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_component::OpenOutgoingChannel in codec type != out codec type"));
        }
        if (!CodecRequiresFsi(out_codec_type))
        {
            iTSCblc.BlcEtbReq(channel_id, pDataType, pH223Params,
                              pDataTypeRvs, pH223ParamsRvs);
        }
        ret = iOlcs.AppendOlc(OUTGOING, channel_id, pDataType, pH223Params,
                              CHANNEL_ID_UNKNOWN, pDataTypeRvs,
                              pH223ParamsRvs);
    }
    else
    {
        if (!CodecRequiresFsi(out_codec_type))
        {
            iTSClc.LcEtbReq(channel_id, pDataType, pH223Params);
        }

        ret = iOlcs.AppendOlc(OUTGOING, channel_id, pDataType, pH223Params);
    }

    // Open the channel in the mux
    OpenLogicalChannel(OUTGOING, ret->GetChannelId());
    // Pause the channel untill OlcAck+MtAck is received
    H223OutgoingChannelPtr outgoing_channel;
    iH223->GetOutgoingChannel(ret->GetChannelId(), outgoing_channel);
    if (Pausable())
    {
        outgoing_channel->Pause();
    }
    // Notify outgoing channel
    iTSCObserver->OutgoingChannelEstablished(ret->GetChannelId(), out_codec_type,
            NULL, 0); /* FSI will be generated by video source/encoder */

    Delete_DataType(pDataType);
    OSCL_DEFAULT_FREE(pDataType);
    Delete_H223LogicalChannelParameters(pH223Params);
    OSCL_DEFAULT_FREE(pH223Params);
    return ret;
}


bool TSC_component::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    // Only returns the component interface
    if (uuid == PVUuidH324ComponentInterface)
    {
        PVMFComponentInterface* myInterface = OSCL_STATIC_CAST(PVMFComponentInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }

    return true;
}

#ifdef MEM_TRACK
void TSC_component::MemStats()
{
#if !(OSCL_BYPASS_MEMMGT)

    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            printf("\n###################Memory Stats Start#################\n");
            printf("  numBytes %d\n", stats->numBytes);
            printf("  peakNumBytes %d\n", stats->peakNumBytes);
            printf("  numAllocs %d\n", stats->numAllocs);
            printf("  peakNumAllocs %d\n", stats->peakNumAllocs);
            printf("  numAllocFails %d\n", stats->numAllocFails);
            printf("  totalNumAllocs %d\n", stats->totalNumAllocs);
            printf("  totalNumBytes %d\n", stats->totalNumBytes);
            printf("\n###################Memory Stats End###################\n");
        }

    }
#endif
}
#endif

