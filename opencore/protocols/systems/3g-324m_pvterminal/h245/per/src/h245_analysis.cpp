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
// ============================================================(Auto)=
// FILE: h245_analysis.c
//
// DESC: PER Analyzer routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

#include "per_headers.h"
#include "h245def.h"
#include "h245_analysis.h"
#include "analyzeper.h"


/* <========================================================> */
/*  PER-Analyzer for MultimediaSystemControlMessage (CHOICE)  */
/* <========================================================> */
void Analyze_MultimediaSystemControlMessage(PS_MultimediaSystemControlMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultimediaSystemControlMessage");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_RequestMessage(x->request, "request", tag, indent);
            break;
        case 1:
            Analyze_ResponseMessage(x->response, "response", tag, indent);
            break;
        case 2:
            Analyze_CommandMessage(x->command, "command", tag, indent);
            break;
        case 3:
            Analyze_IndicationMessage(x->indication, "indication", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MultimediaSystemControlMessage: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultimediaSystemControlMessage");
}

/* <========================================> */
/*  PER-Analyzer for RequestMessage (CHOICE)  */
/* <========================================> */
void Analyze_RequestMessage(PS_RequestMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RequestMessage");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_MasterSlaveDetermination(x->masterSlaveDetermination, "masterSlaveDetermination", tag, indent);
            break;
        case 2:
            Analyze_TerminalCapabilitySet(x->terminalCapabilitySet, "terminalCapabilitySet", tag, indent);
            break;
        case 3:
            Analyze_OpenLogicalChannel(x->openLogicalChannel, "openLogicalChannel", tag, indent);
            break;
        case 4:
            Analyze_CloseLogicalChannel(x->closeLogicalChannel, "closeLogicalChannel", tag, indent);
            break;
        case 5:
            Analyze_RequestChannelClose(x->requestChannelClose, "requestChannelClose", tag, indent);
            break;
        case 6:
            Analyze_MultiplexEntrySend(x->multiplexEntrySend, "multiplexEntrySend", tag, indent);
            break;
        case 7:
            Analyze_RequestMultiplexEntry(x->requestMultiplexEntry, "requestMultiplexEntry", tag, indent);
            break;
        case 8:
            Analyze_RequestMode(x->requestMode, "requestMode", tag, indent);
            break;
        case 9:
            Analyze_RoundTripDelayRequest(x->roundTripDelayRequest, "roundTripDelayRequest", tag, indent);
            break;
        case 10:
            Analyze_MaintenanceLoopRequest(x->maintenanceLoopRequest, "maintenanceLoopRequest", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            Analyze_CommunicationModeRequest(x->communicationModeRequest, "communicationModeRequest", tag, indent);
            break;
        case 12:
            Analyze_ConferenceRequest(x->conferenceRequest, "conferenceRequest", tag, indent);
            break;
        case 13:
            Analyze_MultilinkRequest(x->multilinkRequest, "multilinkRequest", tag, indent);
            break;
        case 14:
            Analyze_LogicalChannelRateRequest(x->logicalChannelRateRequest, "logicalChannelRateRequest", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_RequestMessage: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RequestMessage");
}

/* <=========================================> */
/*  PER-Analyzer for ResponseMessage (CHOICE)  */
/* <=========================================> */
void Analyze_ResponseMessage(PS_ResponseMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ResponseMessage");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_MasterSlaveDeterminationAck(x->masterSlaveDeterminationAck, "masterSlaveDeterminationAck", tag, indent);
            break;
        case 2:
            Analyze_MasterSlaveDeterminationReject(x->masterSlaveDeterminationReject, "masterSlaveDeterminationReject", tag, indent);
            break;
        case 3:
            Analyze_TerminalCapabilitySetAck(x->terminalCapabilitySetAck, "terminalCapabilitySetAck", tag, indent);
            break;
        case 4:
            Analyze_TerminalCapabilitySetReject(x->terminalCapabilitySetReject, "terminalCapabilitySetReject", tag, indent);
            break;
        case 5:
            Analyze_OpenLogicalChannelAck(x->openLogicalChannelAck, "openLogicalChannelAck", tag, indent);
            break;
        case 6:
            Analyze_OpenLogicalChannelReject(x->openLogicalChannelReject, "openLogicalChannelReject", tag, indent);
            break;
        case 7:
            Analyze_CloseLogicalChannelAck(x->closeLogicalChannelAck, "closeLogicalChannelAck", tag, indent);
            break;
        case 8:
            Analyze_RequestChannelCloseAck(x->requestChannelCloseAck, "requestChannelCloseAck", tag, indent);
            break;
        case 9:
            Analyze_RequestChannelCloseReject(x->requestChannelCloseReject, "requestChannelCloseReject", tag, indent);
            break;
        case 10:
            Analyze_MultiplexEntrySendAck(x->multiplexEntrySendAck, "multiplexEntrySendAck", tag, indent);
            break;
        case 11:
            Analyze_MultiplexEntrySendReject(x->multiplexEntrySendReject, "multiplexEntrySendReject", tag, indent);
            break;
        case 12:
            Analyze_RequestMultiplexEntryAck(x->requestMultiplexEntryAck, "requestMultiplexEntryAck", tag, indent);
            break;
        case 13:
            Analyze_RequestMultiplexEntryReject(x->requestMultiplexEntryReject, "requestMultiplexEntryReject", tag, indent);
            break;
        case 14:
            Analyze_RequestModeAck(x->requestModeAck, "requestModeAck", tag, indent);
            break;
        case 15:
            Analyze_RequestModeReject(x->requestModeReject, "requestModeReject", tag, indent);
            break;
        case 16:
            Analyze_RoundTripDelayResponse(x->roundTripDelayResponse, "roundTripDelayResponse", tag, indent);
            break;
        case 17:
            Analyze_MaintenanceLoopAck(x->maintenanceLoopAck, "maintenanceLoopAck", tag, indent);
            break;
        case 18:
            Analyze_MaintenanceLoopReject(x->maintenanceLoopReject, "maintenanceLoopReject", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            Analyze_CommunicationModeResponse(x->communicationModeResponse, "communicationModeResponse", tag, indent);
            break;
        case 20:
            Analyze_ConferenceResponse(x->conferenceResponse, "conferenceResponse", tag, indent);
            break;
        case 21:
            Analyze_MultilinkResponse(x->multilinkResponse, "multilinkResponse", tag, indent);
            break;
        case 22:
            Analyze_LogicalChannelRateAcknowledge(x->logicalChannelRateAcknowledge, "logicalChannelRateAcknowledge", tag, indent);
            break;
        case 23:
            Analyze_LogicalChannelRateReject(x->logicalChannelRateReject, "logicalChannelRateReject", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ResponseMessage: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ResponseMessage");
}

/* <========================================> */
/*  PER-Analyzer for CommandMessage (CHOICE)  */
/* <========================================> */
void Analyze_CommandMessage(PS_CommandMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CommandMessage");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_MaintenanceLoopOffCommand(x->maintenanceLoopOffCommand, "maintenanceLoopOffCommand", tag, indent);
            break;
        case 2:
            Analyze_SendTerminalCapabilitySet(x->sendTerminalCapabilitySet, "sendTerminalCapabilitySet", tag, indent);
            break;
        case 3:
            Analyze_EncryptionCommand(x->encryptionCommand, "encryptionCommand", tag, indent);
            break;
        case 4:
            Analyze_FlowControlCommand(x->flowControlCommand, "flowControlCommand", tag, indent);
            break;
        case 5:
            Analyze_EndSessionCommand(x->endSessionCommand, "endSessionCommand", tag, indent);
            break;
        case 6:
            Analyze_MiscellaneousCommand(x->miscellaneousCommand, "miscellaneousCommand", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            Analyze_CommunicationModeCommand(x->communicationModeCommand, "communicationModeCommand", tag, indent);
            break;
        case 8:
            Analyze_ConferenceCommand(x->conferenceCommand, "conferenceCommand", tag, indent);
            break;
        case 9:
            Analyze_H223MultiplexReconfiguration(x->h223MultiplexReconfiguration, "h223MultiplexReconfiguration", tag, indent);
            break;
        case 10:
            Analyze_NewATMVCCommand(x->newATMVCCommand, "newATMVCCommand", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_CommandMessage: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CommandMessage");
}

/* <===========================================> */
/*  PER-Analyzer for IndicationMessage (CHOICE)  */
/* <===========================================> */
void Analyze_IndicationMessage(PS_IndicationMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IndicationMessage");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_FunctionNotUnderstood(x->functionNotUnderstood, "functionNotUnderstood", tag, indent);
            break;
        case 2:
            Analyze_MasterSlaveDeterminationRelease(x->masterSlaveDeterminationRelease, "masterSlaveDeterminationRelease", tag, indent);
            break;
        case 3:
            Analyze_TerminalCapabilitySetRelease(x->terminalCapabilitySetRelease, "terminalCapabilitySetRelease", tag, indent);
            break;
        case 4:
            Analyze_OpenLogicalChannelConfirm(x->openLogicalChannelConfirm, "openLogicalChannelConfirm", tag, indent);
            break;
        case 5:
            Analyze_RequestChannelCloseRelease(x->requestChannelCloseRelease, "requestChannelCloseRelease", tag, indent);
            break;
        case 6:
            Analyze_MultiplexEntrySendRelease(x->multiplexEntrySendRelease, "multiplexEntrySendRelease", tag, indent);
            break;
        case 7:
            Analyze_RequestMultiplexEntryRelease(x->requestMultiplexEntryRelease, "requestMultiplexEntryRelease", tag, indent);
            break;
        case 8:
            Analyze_RequestModeRelease(x->requestModeRelease, "requestModeRelease", tag, indent);
            break;
        case 9:
            Analyze_MiscellaneousIndication(x->miscellaneousIndication, "miscellaneousIndication", tag, indent);
            break;
        case 10:
            Analyze_JitterIndication(x->jitterIndication, "jitterIndication", tag, indent);
            break;
        case 11:
            Analyze_H223SkewIndication(x->h223SkewIndication, "h223SkewIndication", tag, indent);
            break;
        case 12:
            Analyze_NewATMVCIndication(x->newATMVCIndication, "newATMVCIndication", tag, indent);
            break;
        case 13:
            Analyze_UserInputIndication(x->userInput, "userInput", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            Analyze_H2250MaximumSkewIndication(x->h2250MaximumSkewIndication, "h2250MaximumSkewIndication", tag, indent);
            break;
        case 15:
            Analyze_MCLocationIndication(x->mcLocationIndication, "mcLocationIndication", tag, indent);
            break;
        case 16:
            Analyze_ConferenceIndication(x->conferenceIndication, "conferenceIndication", tag, indent);
            break;
        case 17:
            Analyze_VendorIdentification(x->vendorIdentification, "vendorIdentification", tag, indent);
            break;
        case 18:
            Analyze_FunctionNotSupported(x->functionNotSupported, "functionNotSupported", tag, indent);
            break;
        case 19:
            Analyze_MultilinkIndication(x->multilinkIndication, "multilinkIndication", tag, indent);
            break;
        case 20:
            Analyze_LogicalChannelRateRelease(x->logicalChannelRateRelease, "logicalChannelRateRelease", tag, indent);
            break;
        case 21:
            Analyze_FlowControlIndication(x->flowControlIndication, "flowControlIndication", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_IndicationMessage: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IndicationMessage");
}

/* <==============================================> */
/*  PER-Analyzer for NonStandardMessage (SEQUENCE)  */
/* <==============================================> */
void Analyze_NonStandardMessage(PS_NonStandardMessage x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "NonStandardMessage");
    indent += 2;
    Analyze_NonStandardParameter(&x->nonStandardData, "nonStandardData", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "NonStandardMessage");
}

/* <================================================> */
/*  PER-Analyzer for NonStandardParameter (SEQUENCE)  */
/* <================================================> */
void Analyze_NonStandardParameter(PS_NonStandardParameter x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "NonStandardParameter");
    indent += 2;
    Analyze_NonStandardIdentifier(&x->nonStandardIdentifier, "nonStandardIdentifier", tag, indent);
    ShowPEROctetString(tag, indent, "data", &x->data);
    ShowPERClosure(tag, indent, "NonStandardParameter");
}

/* <===============================================> */
/*  PER-Analyzer for NonStandardIdentifier (CHOICE)  */
/* <===============================================> */
void Analyze_NonStandardIdentifier(PS_NonStandardIdentifier x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "NonStandardIdentifier");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERObjectID(tag, indent, "object", x->object);
            break;
        case 1:
            Analyze_H221NonStandard(x->h221NonStandard, "h221NonStandard", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_NonStandardIdentifier: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "NonStandardIdentifier");
}

/* <===========================================> */
/*  PER-Analyzer for H221NonStandard (SEQUENCE)  */
/* <===========================================> */
void Analyze_H221NonStandard(PS_H221NonStandard x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H221NonStandard");
    indent += 2;
    ShowPERInteger(tag, indent, "t35CountryCode", (int32)x->t35CountryCode);
    ShowPERInteger(tag, indent, "t35Extension", (int32)x->t35Extension);
    ShowPERInteger(tag, indent, "manufacturerCode", (int32)x->manufacturerCode);
    ShowPERClosure(tag, indent, "H221NonStandard");
}

/* <====================================================> */
/*  PER-Analyzer for MasterSlaveDetermination (SEQUENCE)  */
/* <====================================================> */
void Analyze_MasterSlaveDetermination(PS_MasterSlaveDetermination x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MasterSlaveDetermination");
    indent += 2;
    ShowPERInteger(tag, indent, "terminalType", (int32)x->terminalType);
    ShowPERInteger(tag, indent, "statusDeterminationNumber", (int32)x->statusDeterminationNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MasterSlaveDetermination");
}

/* <=======================================================> */
/*  PER-Analyzer for MasterSlaveDeterminationAck (SEQUENCE)  */
/* <=======================================================> */
void Analyze_MasterSlaveDeterminationAck(PS_MasterSlaveDeterminationAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MasterSlaveDeterminationAck");
    indent += 2;
    Analyze_Decision(&x->decision, "decision", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MasterSlaveDeterminationAck");
}

/* <==================================> */
/*  PER-Analyzer for Decision (CHOICE)  */
/* <==================================> */
void Analyze_Decision(PS_Decision x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Decision");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "master");
            break;
        case 1:
            ShowPERNull(tag, indent, "slave");
            break;
        default:
            ErrorMessage("Analyze_Decision: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Decision");
}

/* <==========================================================> */
/*  PER-Analyzer for MasterSlaveDeterminationReject (SEQUENCE)  */
/* <==========================================================> */
void Analyze_MasterSlaveDeterminationReject(PS_MasterSlaveDeterminationReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MasterSlaveDeterminationReject");
    indent += 2;
    Analyze_MsdRejectCause(&x->msdRejectCause, "msdRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MasterSlaveDeterminationReject");
}

/* <========================================> */
/*  PER-Analyzer for MsdRejectCause (CHOICE)  */
/* <========================================> */
void Analyze_MsdRejectCause(PS_MsdRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MsdRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "identicalNumbers");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MsdRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MsdRejectCause");
}

/* <===========================================================> */
/*  PER-Analyzer for MasterSlaveDeterminationRelease (SEQUENCE)  */
/* <===========================================================> */
void Analyze_MasterSlaveDeterminationRelease(PS_MasterSlaveDeterminationRelease x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "MasterSlaveDeterminationRelease");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MasterSlaveDeterminationRelease");
}

/* <=================================================> */
/*  PER-Analyzer for TerminalCapabilitySet (SEQUENCE)  */
/* <=================================================> */
void Analyze_TerminalCapabilitySet(PS_TerminalCapabilitySet x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "TerminalCapabilitySet");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_multiplexCapability", x->option_of_multiplexCapability);
    ShowPERBoolean(tag, indent, "option_of_capabilityTable", x->option_of_capabilityTable);
    ShowPERBoolean(tag, indent, "option_of_capabilityDescriptors", x->option_of_capabilityDescriptors);
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERObjectID(tag, indent, "protocolIdentifier", &x->protocolIdentifier);
    if (x->option_of_multiplexCapability)
    {
        Analyze_MultiplexCapability(&x->multiplexCapability, "multiplexCapability", tag, indent);
    }
    if (x->option_of_capabilityTable)
    {
        ShowPERInteger(tag, indent, "size_of_capabilityTable", (uint32)x->size_of_capabilityTable);
        for (i = 0;i < x->size_of_capabilityTable;++i)
        {
            sprintf(tempLabelString, "capabilityTable[%hu]", i);
            Analyze_CapabilityTableEntry(x->capabilityTable + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_capabilityDescriptors)
    {
        ShowPERInteger(tag, indent, "size_of_capabilityDescriptors", (uint32)x->size_of_capabilityDescriptors);
        for (i = 0;i < x->size_of_capabilityDescriptors;++i)
        {
            sprintf(tempLabelString, "capabilityDescriptors[%hu]", i);
            Analyze_CapabilityDescriptor(x->capabilityDescriptors + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalCapabilitySet");
}

/* <================================================> */
/*  PER-Analyzer for CapabilityTableEntry (SEQUENCE)  */
/* <================================================> */
void Analyze_CapabilityTableEntry(PS_CapabilityTableEntry x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CapabilityTableEntry");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_capability", x->option_of_capability);
    ShowPERInteger(tag, indent, "capabilityTableEntryNumber", (int32)x->capabilityTableEntryNumber);
    if (x->option_of_capability)
    {
        Analyze_Capability(&x->capability, "capability", tag, indent);
    }
    ShowPERClosure(tag, indent, "CapabilityTableEntry");
}

/* <================================================> */
/*  PER-Analyzer for CapabilityDescriptor (SEQUENCE)  */
/* <================================================> */
void Analyze_CapabilityDescriptor(PS_CapabilityDescriptor x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "CapabilityDescriptor");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_simultaneousCapabilities", x->option_of_simultaneousCapabilities);
    ShowPERInteger(tag, indent, "capabilityDescriptorNumber", (int32)x->capabilityDescriptorNumber);
    if (x->option_of_simultaneousCapabilities)
    {
        ShowPERInteger(tag, indent, "size_of_simultaneousCapabilities", (uint32)x->size_of_simultaneousCapabilities);
        for (i = 0;i < x->size_of_simultaneousCapabilities;++i)
        {
            sprintf(tempLabelString, "simultaneousCapabilities[%hu]", i);
            Analyze_AlternativeCapabilitySet(x->simultaneousCapabilities + i, tempLabelString, tag, indent);
        }
    }
    ShowPERClosure(tag, indent, "CapabilityDescriptor");
}

/* <=======================================================> */
/*  PER-Analyzer for AlternativeCapabilitySet (SEQUENCE-OF)  */
/* <=======================================================> */
void Analyze_AlternativeCapabilitySet(PS_AlternativeCapabilitySet x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequenceof(tag, indent, label, "AlternativeCapabilitySet");
    indent += 2;
    ShowPERInteger(tag, indent, "size", (uint32)x->size);
    for (i = 0;i < x->size;++i)
    {
        ShowPERIntegers(tag, indent, "item", (uint32)x->item[i], i);
    }
    ShowPERClosure(tag, indent, "AlternativeCapabilitySet");
}

/* <====================================================> */
/*  PER-Analyzer for TerminalCapabilitySetAck (SEQUENCE)  */
/* <====================================================> */
void Analyze_TerminalCapabilitySetAck(PS_TerminalCapabilitySetAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalCapabilitySetAck");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalCapabilitySetAck");
}

/* <=======================================================> */
/*  PER-Analyzer for TerminalCapabilitySetReject (SEQUENCE)  */
/* <=======================================================> */
void Analyze_TerminalCapabilitySetReject(PS_TerminalCapabilitySetReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalCapabilitySetReject");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    Analyze_TcsRejectCause(&x->tcsRejectCause, "tcsRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalCapabilitySetReject");
}

/* <========================================> */
/*  PER-Analyzer for TcsRejectCause (CHOICE)  */
/* <========================================> */
void Analyze_TcsRejectCause(PS_TcsRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "TcsRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecified");
            break;
        case 1:
            ShowPERNull(tag, indent, "undefinedTableEntryUsed");
            break;
        case 2:
            ShowPERNull(tag, indent, "descriptorCapacityExceeded");
            break;
        case 3:
            Analyze_TableEntryCapacityExceeded(x->tableEntryCapacityExceeded, "tableEntryCapacityExceeded", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_TcsRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "TcsRejectCause");
}

/* <====================================================> */
/*  PER-Analyzer for TableEntryCapacityExceeded (CHOICE)  */
/* <====================================================> */
void Analyze_TableEntryCapacityExceeded(PS_TableEntryCapacityExceeded x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "TableEntryCapacityExceeded");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "highestEntryNumberProcessed", (uint32)x->highestEntryNumberProcessed);
            break;
        case 1:
            ShowPERNull(tag, indent, "noneProcessed");
            break;
        default:
            ErrorMessage("Analyze_TableEntryCapacityExceeded: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "TableEntryCapacityExceeded");
}

/* <========================================================> */
/*  PER-Analyzer for TerminalCapabilitySetRelease (SEQUENCE)  */
/* <========================================================> */
void Analyze_TerminalCapabilitySetRelease(PS_TerminalCapabilitySetRelease x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "TerminalCapabilitySetRelease");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalCapabilitySetRelease");
}

/* <====================================> */
/*  PER-Analyzer for Capability (CHOICE)  */
/* <====================================> */
void Analyze_Capability(PS_Capability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Capability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_VideoCapability(x->receiveVideoCapability, "receiveVideoCapability", tag, indent);
            break;
        case 2:
            Analyze_VideoCapability(x->transmitVideoCapability, "transmitVideoCapability", tag, indent);
            break;
        case 3:
            Analyze_VideoCapability(x->receiveAndTransmitVideoCapability, "receiveAndTransmitVideoCapability", tag, indent);
            break;
        case 4:
            Analyze_AudioCapability(x->receiveAudioCapability, "receiveAudioCapability", tag, indent);
            break;
        case 5:
            Analyze_AudioCapability(x->transmitAudioCapability, "transmitAudioCapability", tag, indent);
            break;
        case 6:
            Analyze_AudioCapability(x->receiveAndTransmitAudioCapability, "receiveAndTransmitAudioCapability", tag, indent);
            break;
        case 7:
            Analyze_DataApplicationCapability(x->receiveDataApplicationCapability, "receiveDataApplicationCapability", tag, indent);
            break;
        case 8:
            Analyze_DataApplicationCapability(x->transmitDataApplicationCapability, "transmitDataApplicationCapability", tag, indent);
            break;
        case 9:
            Analyze_DataApplicationCapability(x->receiveAndTransmitDataApplicationCapability, "receiveAndTransmitDataApplicationCapability", tag, indent);
            break;
        case 10:
            ShowPERBoolean(tag, indent, "h233EncryptionTransmitCapability", x->h233EncryptionTransmitCapability);
            break;
        case 11:
            Analyze_H233EncryptionReceiveCapability(x->h233EncryptionReceiveCapability, "h233EncryptionReceiveCapability", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 12:
            Analyze_ConferenceCapability(x->conferenceCapability, "conferenceCapability", tag, indent);
            break;
        case 13:
            Analyze_H235SecurityCapability(x->h235SecurityCapability, "h235SecurityCapability", tag, indent);
            break;
        case 14:
            ShowPERInteger(tag, indent, "maxPendingReplacementFor", (uint32)x->maxPendingReplacementFor);
            break;
        case 15:
            Analyze_UserInputCapability(x->receiveUserInputCapability, "receiveUserInputCapability", tag, indent);
            break;
        case 16:
            Analyze_UserInputCapability(x->transmitUserInputCapability, "transmitUserInputCapability", tag, indent);
            break;
        case 17:
            Analyze_UserInputCapability(x->receiveAndTransmitUserInputCapability, "receiveAndTransmitUserInputCapability", tag, indent);
            break;
        case 18:
            Analyze_GenericCapability(x->genericControlCapability, "genericControlCapability", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_Capability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Capability");
}

/* <===========================================================> */
/*  PER-Analyzer for H233EncryptionReceiveCapability (SEQUENCE)  */
/* <===========================================================> */
void Analyze_H233EncryptionReceiveCapability(PS_H233EncryptionReceiveCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H233EncryptionReceiveCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "h233IVResponseTime", (int32)x->h233IVResponseTime);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H233EncryptionReceiveCapability");
}

/* <==================================================> */
/*  PER-Analyzer for H235SecurityCapability (SEQUENCE)  */
/* <==================================================> */
void Analyze_H235SecurityCapability(PS_H235SecurityCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H235SecurityCapability");
    indent += 2;
    Analyze_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, "encryptionAuthenticationAndIntegrity", tag, indent);
    ShowPERInteger(tag, indent, "mediaCapability", (int32)x->mediaCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H235SecurityCapability");
}

/* <=============================================> */
/*  PER-Analyzer for MultiplexCapability (CHOICE)  */
/* <=============================================> */
void Analyze_MultiplexCapability(PS_MultiplexCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultiplexCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_H222Capability(x->h222Capability, "h222Capability", tag, indent);
            break;
        case 2:
            Analyze_H223Capability(x->h223Capability, "h223Capability", tag, indent);
            break;
        case 3:
            Analyze_V76Capability(x->v76Capability, "v76Capability", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            Analyze_H2250Capability(x->h2250Capability, "h2250Capability", tag, indent);
            break;
        case 5:
            Analyze_GenericCapability(x->genericMultiplexCapability, "genericMultiplexCapability", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_MultiplexCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultiplexCapability");
}

/* <==========================================> */
/*  PER-Analyzer for H222Capability (SEQUENCE)  */
/* <==========================================> */
void Analyze_H222Capability(PS_H222Capability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H222Capability");
    indent += 2;
    ShowPERInteger(tag, indent, "numberOfVCs", (int32)x->numberOfVCs);
    ShowPERInteger(tag, indent, "size_of_vcCapability", (uint32)x->size_of_vcCapability);
    for (i = 0;i < x->size_of_vcCapability;++i)
    {
        sprintf(tempLabelString, "vcCapability[%hu]", i);
        Analyze_VCCapability(x->vcCapability + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H222Capability");
}

/* <========================================> */
/*  PER-Analyzer for VCCapability (SEQUENCE)  */
/* <========================================> */
void Analyze_VCCapability(PS_VCCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VCCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_vccAal1", x->option_of_vccAal1);
    ShowPERBoolean(tag, indent, "option_of_vccAal5", x->option_of_vccAal5);
    if (x->option_of_vccAal1)
    {
        Analyze_VccAal1(&x->vccAal1, "vccAal1", tag, indent);
    }
    if (x->option_of_vccAal5)
    {
        Analyze_VccAal5(&x->vccAal5, "vccAal5", tag, indent);
    }
    ShowPERBoolean(tag, indent, "transportStream", x->transportStream);
    ShowPERBoolean(tag, indent, "programStream", x->programStream);
    Analyze_AvailableBitRates(&x->availableBitRates, "availableBitRates", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_aal1ViaGateway", x->option_of_aal1ViaGateway);
    if (x->option_of_aal1ViaGateway)
    {
        Analyze_Aal1ViaGateway(&x->aal1ViaGateway, "aal1ViaGateway", tag, indent);
    }
    ShowPERClosure(tag, indent, "VCCapability");
}

/* <==========================================> */
/*  PER-Analyzer for Aal1ViaGateway (SEQUENCE)  */
/* <==========================================> */
void Analyze_Aal1ViaGateway(PS_Aal1ViaGateway x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "Aal1ViaGateway");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_gatewayAddress", (uint32)x->size_of_gatewayAddress);
    for (i = 0;i < x->size_of_gatewayAddress;++i)
    {
        sprintf(tempLabelString, "gatewayAddress[%hu]", i);
        Analyze_Q2931Address(x->gatewayAddress + i, tempLabelString, tag, indent);
    }
    ShowPERBoolean(tag, indent, "nullClockRecovery", x->nullClockRecovery);
    ShowPERBoolean(tag, indent, "srtsClockRecovery", x->srtsClockRecovery);
    ShowPERBoolean(tag, indent, "adaptiveClockRecovery", x->adaptiveClockRecovery);
    ShowPERBoolean(tag, indent, "nullErrorCorrection", x->nullErrorCorrection);
    ShowPERBoolean(tag, indent, "longInterleaver", x->longInterleaver);
    ShowPERBoolean(tag, indent, "shortInterleaver", x->shortInterleaver);
    ShowPERBoolean(tag, indent, "errorCorrectionOnly", x->errorCorrectionOnly);
    ShowPERBoolean(tag, indent, "structuredDataTransfer", x->structuredDataTransfer);
    ShowPERBoolean(tag, indent, "partiallyFilledCells", x->partiallyFilledCells);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Aal1ViaGateway");
}

/* <=============================================> */
/*  PER-Analyzer for AvailableBitRates (SEQUENCE)  */
/* <=============================================> */
void Analyze_AvailableBitRates(PS_AvailableBitRates x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "AvailableBitRates");
    indent += 2;
    Analyze_VccAal5Type(&x->vccAal5Type, "vccAal5Type", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "AvailableBitRates");
}

/* <=====================================> */
/*  PER-Analyzer for VccAal5Type (CHOICE)  */
/* <=====================================> */
void Analyze_VccAal5Type(PS_VccAal5Type x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "VccAal5Type");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "singleBitRate", (uint32)x->singleBitRate);
            break;
        case 1:
            Analyze_RangeOfBitRates(x->rangeOfBitRates, "rangeOfBitRates", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_VccAal5Type: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "VccAal5Type");
}

/* <===========================================> */
/*  PER-Analyzer for RangeOfBitRates (SEQUENCE)  */
/* <===========================================> */
void Analyze_RangeOfBitRates(PS_RangeOfBitRates x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RangeOfBitRates");
    indent += 2;
    ShowPERInteger(tag, indent, "lowerBitRate", (int32)x->lowerBitRate);
    ShowPERInteger(tag, indent, "higherBitRate", (int32)x->higherBitRate);
    ShowPERClosure(tag, indent, "RangeOfBitRates");
}

/* <===================================> */
/*  PER-Analyzer for VccAal5 (SEQUENCE)  */
/* <===================================> */
void Analyze_VccAal5(PS_VccAal5 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VccAal5");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardMaximumSDUSize", (int32)x->forwardMaximumSDUSize);
    ShowPERInteger(tag, indent, "backwardMaximumSDUSize", (int32)x->backwardMaximumSDUSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VccAal5");
}

/* <===================================> */
/*  PER-Analyzer for VccAal1 (SEQUENCE)  */
/* <===================================> */
void Analyze_VccAal1(PS_VccAal1 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VccAal1");
    indent += 2;
    ShowPERBoolean(tag, indent, "nullClockRecovery", x->nullClockRecovery);
    ShowPERBoolean(tag, indent, "srtsClockRecovery", x->srtsClockRecovery);
    ShowPERBoolean(tag, indent, "adaptiveClockRecovery", x->adaptiveClockRecovery);
    ShowPERBoolean(tag, indent, "nullErrorCorrection", x->nullErrorCorrection);
    ShowPERBoolean(tag, indent, "longInterleaver", x->longInterleaver);
    ShowPERBoolean(tag, indent, "shortInterleaver", x->shortInterleaver);
    ShowPERBoolean(tag, indent, "errorCorrectionOnly", x->errorCorrectionOnly);
    ShowPERBoolean(tag, indent, "structuredDataTransfer", x->structuredDataTransfer);
    ShowPERBoolean(tag, indent, "partiallyFilledCells", x->partiallyFilledCells);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VccAal1");
}

/* <==========================================> */
/*  PER-Analyzer for H223Capability (SEQUENCE)  */
/* <==========================================> */
void Analyze_H223Capability(PS_H223Capability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223Capability");
    indent += 2;
    ShowPERBoolean(tag, indent, "transportWithI_frames", x->transportWithI_frames);
    ShowPERBoolean(tag, indent, "videoWithAL1", x->videoWithAL1);
    ShowPERBoolean(tag, indent, "videoWithAL2", x->videoWithAL2);
    ShowPERBoolean(tag, indent, "videoWithAL3", x->videoWithAL3);
    ShowPERBoolean(tag, indent, "audioWithAL1", x->audioWithAL1);
    ShowPERBoolean(tag, indent, "audioWithAL2", x->audioWithAL2);
    ShowPERBoolean(tag, indent, "audioWithAL3", x->audioWithAL3);
    ShowPERBoolean(tag, indent, "dataWithAL1", x->dataWithAL1);
    ShowPERBoolean(tag, indent, "dataWithAL2", x->dataWithAL2);
    ShowPERBoolean(tag, indent, "dataWithAL3", x->dataWithAL3);
    ShowPERInteger(tag, indent, "maximumAl2SDUSize", (int32)x->maximumAl2SDUSize);
    ShowPERInteger(tag, indent, "maximumAl3SDUSize", (int32)x->maximumAl3SDUSize);
    ShowPERInteger(tag, indent, "maximumDelayJitter", (int32)x->maximumDelayJitter);
    Analyze_H223MultiplexTableCapability(&x->h223MultiplexTableCapability, "h223MultiplexTableCapability", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_maxMUXPDUSizeCapability", x->option_of_maxMUXPDUSizeCapability);
    ShowPERBoolean(tag, indent, "option_of_nsrpSupport", x->option_of_nsrpSupport);
    ShowPERBoolean(tag, indent, "option_of_mobileOperationTransmitCapability", x->option_of_mobileOperationTransmitCapability);
    ShowPERBoolean(tag, indent, "option_of_h223AnnexCCapability", x->option_of_h223AnnexCCapability);
    if (x->option_of_maxMUXPDUSizeCapability)
    {
        ShowPERBoolean(tag, indent, "maxMUXPDUSizeCapability", x->maxMUXPDUSizeCapability);
    }
    if (x->option_of_nsrpSupport)
    {
        ShowPERBoolean(tag, indent, "nsrpSupport", x->nsrpSupport);
    }
    if (x->option_of_mobileOperationTransmitCapability)
    {
        Analyze_MobileOperationTransmitCapability(&x->mobileOperationTransmitCapability, "mobileOperationTransmitCapability", tag, indent);
    }
    if (x->option_of_h223AnnexCCapability)
    {
        Analyze_H223AnnexCCapability(&x->h223AnnexCCapability, "h223AnnexCCapability", tag, indent);
    }
    ShowPERClosure(tag, indent, "H223Capability");
}

/* <=============================================================> */
/*  PER-Analyzer for MobileOperationTransmitCapability (SEQUENCE)  */
/* <=============================================================> */
void Analyze_MobileOperationTransmitCapability(PS_MobileOperationTransmitCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MobileOperationTransmitCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "modeChangeCapability", x->modeChangeCapability);
    ShowPERBoolean(tag, indent, "h223AnnexA", x->h223AnnexA);
    ShowPERBoolean(tag, indent, "h223AnnexADoubleFlag", x->h223AnnexADoubleFlag);
    ShowPERBoolean(tag, indent, "h223AnnexB", x->h223AnnexB);
    ShowPERBoolean(tag, indent, "h223AnnexBwithHeader", x->h223AnnexBwithHeader);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MobileOperationTransmitCapability");
}

/* <======================================================> */
/*  PER-Analyzer for H223MultiplexTableCapability (CHOICE)  */
/* <======================================================> */
void Analyze_H223MultiplexTableCapability(PS_H223MultiplexTableCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "H223MultiplexTableCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "basic");
            break;
        case 1:
            Analyze_Enhanced(x->enhanced, "enhanced", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_H223MultiplexTableCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "H223MultiplexTableCapability");
}

/* <====================================> */
/*  PER-Analyzer for Enhanced (SEQUENCE)  */
/* <====================================> */
void Analyze_Enhanced(PS_Enhanced x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Enhanced");
    indent += 2;
    ShowPERInteger(tag, indent, "maximumNestingDepth", (int32)x->maximumNestingDepth);
    ShowPERInteger(tag, indent, "maximumElementListSize", (int32)x->maximumElementListSize);
    ShowPERInteger(tag, indent, "maximumSubElementListSize", (int32)x->maximumSubElementListSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Enhanced");
}

/* <================================================> */
/*  PER-Analyzer for H223AnnexCCapability (SEQUENCE)  */
/* <================================================> */
void Analyze_H223AnnexCCapability(PS_H223AnnexCCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223AnnexCCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "videoWithAL1M", x->videoWithAL1M);
    ShowPERBoolean(tag, indent, "videoWithAL2M", x->videoWithAL2M);
    ShowPERBoolean(tag, indent, "videoWithAL3M", x->videoWithAL3M);
    ShowPERBoolean(tag, indent, "audioWithAL1M", x->audioWithAL1M);
    ShowPERBoolean(tag, indent, "audioWithAL2M", x->audioWithAL2M);
    ShowPERBoolean(tag, indent, "audioWithAL3M", x->audioWithAL3M);
    ShowPERBoolean(tag, indent, "dataWithAL1M", x->dataWithAL1M);
    ShowPERBoolean(tag, indent, "dataWithAL2M", x->dataWithAL2M);
    ShowPERBoolean(tag, indent, "dataWithAL3M", x->dataWithAL3M);
    ShowPERBoolean(tag, indent, "alpduInterleaving", x->alpduInterleaving);
    ShowPERInteger(tag, indent, "maximumAL1MPDUSize", (int32)x->maximumAL1MPDUSize);
    ShowPERInteger(tag, indent, "maximumAL2MSDUSize", (int32)x->maximumAL2MSDUSize);
    ShowPERInteger(tag, indent, "maximumAL3MSDUSize", (int32)x->maximumAL3MSDUSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_rsCodeCapability", x->option_of_rsCodeCapability);
    if (x->option_of_rsCodeCapability)
    {
        ShowPERBoolean(tag, indent, "rsCodeCapability", x->rsCodeCapability);
    }
    ShowPERClosure(tag, indent, "H223AnnexCCapability");
}

/* <=========================================> */
/*  PER-Analyzer for V76Capability (SEQUENCE)  */
/* <=========================================> */
void Analyze_V76Capability(PS_V76Capability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V76Capability");
    indent += 2;
    ShowPERBoolean(tag, indent, "suspendResumeCapabilitywAddress", x->suspendResumeCapabilitywAddress);
    ShowPERBoolean(tag, indent, "suspendResumeCapabilitywoAddress", x->suspendResumeCapabilitywoAddress);
    ShowPERBoolean(tag, indent, "rejCapability", x->rejCapability);
    ShowPERBoolean(tag, indent, "sREJCapability", x->sREJCapability);
    ShowPERBoolean(tag, indent, "mREJCapability", x->mREJCapability);
    ShowPERBoolean(tag, indent, "crc8bitCapability", x->crc8bitCapability);
    ShowPERBoolean(tag, indent, "crc16bitCapability", x->crc16bitCapability);
    ShowPERBoolean(tag, indent, "crc32bitCapability", x->crc32bitCapability);
    ShowPERBoolean(tag, indent, "uihCapability", x->uihCapability);
    ShowPERInteger(tag, indent, "numOfDLCS", (int32)x->numOfDLCS);
    ShowPERBoolean(tag, indent, "twoOctetAddressFieldCapability", x->twoOctetAddressFieldCapability);
    ShowPERBoolean(tag, indent, "loopBackTestCapability", x->loopBackTestCapability);
    ShowPERInteger(tag, indent, "n401Capability", (int32)x->n401Capability);
    ShowPERInteger(tag, indent, "maxWindowSizeCapability", (int32)x->maxWindowSizeCapability);
    Analyze_V75Capability(&x->v75Capability, "v75Capability", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V76Capability");
}

/* <=========================================> */
/*  PER-Analyzer for V75Capability (SEQUENCE)  */
/* <=========================================> */
void Analyze_V75Capability(PS_V75Capability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V75Capability");
    indent += 2;
    ShowPERBoolean(tag, indent, "audioHeader", x->audioHeader);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V75Capability");
}

/* <===========================================> */
/*  PER-Analyzer for H2250Capability (SEQUENCE)  */
/* <===========================================> */
void Analyze_H2250Capability(PS_H2250Capability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H2250Capability");
    indent += 2;
    ShowPERInteger(tag, indent, "maximumAudioDelayJitter", (int32)x->maximumAudioDelayJitter);
    Analyze_MultipointCapability(&x->receiveMultipointCapability, "receiveMultipointCapability", tag, indent);
    Analyze_MultipointCapability(&x->transmitMultipointCapability, "transmitMultipointCapability", tag, indent);
    Analyze_MultipointCapability(&x->receiveAndTransmitMultipointCapability, "receiveAndTransmitMultipointCapability", tag, indent);
    Analyze_McCapability(&x->mcCapability, "mcCapability", tag, indent);
    ShowPERBoolean(tag, indent, "rtcpVideoControlCapability", x->rtcpVideoControlCapability);
    Analyze_MediaPacketizationCapability(&x->mediaPacketizationCapability, "mediaPacketizationCapability", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_transportCapability", x->option_of_transportCapability);
    ShowPERBoolean(tag, indent, "option_of_redundancyEncodingCapability", x->option_of_redundancyEncodingCapability);
    ShowPERBoolean(tag, indent, "option_of_logicalChannelSwitchingCapability", x->option_of_logicalChannelSwitchingCapability);
    ShowPERBoolean(tag, indent, "option_of_t120DynamicPortCapability", x->option_of_t120DynamicPortCapability);
    if (x->option_of_transportCapability)
    {
        Analyze_TransportCapability(&x->transportCapability, "transportCapability", tag, indent);
    }
    if (x->option_of_redundancyEncodingCapability)
    {
        ShowPERInteger(tag, indent, "size_of_redundancyEncodingCapability", (uint32)x->size_of_redundancyEncodingCapability);
        for (i = 0;i < x->size_of_redundancyEncodingCapability;++i)
        {
            sprintf(tempLabelString, "redundancyEncodingCapability[%hu]", i);
            Analyze_RedundancyEncodingCapability(x->redundancyEncodingCapability + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_logicalChannelSwitchingCapability)
    {
        ShowPERBoolean(tag, indent, "logicalChannelSwitchingCapability", x->logicalChannelSwitchingCapability);
    }
    if (x->option_of_t120DynamicPortCapability)
    {
        ShowPERBoolean(tag, indent, "t120DynamicPortCapability", x->t120DynamicPortCapability);
    }
    ShowPERClosure(tag, indent, "H2250Capability");
}

/* <========================================> */
/*  PER-Analyzer for McCapability (SEQUENCE)  */
/* <========================================> */
void Analyze_McCapability(PS_McCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "McCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "centralizedConferenceMC", x->centralizedConferenceMC);
    ShowPERBoolean(tag, indent, "decentralizedConferenceMC", x->decentralizedConferenceMC);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "McCapability");
}

/* <========================================================> */
/*  PER-Analyzer for MediaPacketizationCapability (SEQUENCE)  */
/* <========================================================> */
void Analyze_MediaPacketizationCapability(PS_MediaPacketizationCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MediaPacketizationCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "h261aVideoPacketization", x->h261aVideoPacketization);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_rtpPayloadType", x->option_of_rtpPayloadType);
    if (x->option_of_rtpPayloadType)
    {
        ShowPERInteger(tag, indent, "size_of_rtpPayloadType", (uint32)x->size_of_rtpPayloadType);
        for (i = 0;i < x->size_of_rtpPayloadType;++i)
        {
            sprintf(tempLabelString, "rtpPayloadType[%hu]", i);
            Analyze_RTPPayloadType(x->rtpPayloadType + i, tempLabelString, tag, indent);
        }
    }
    ShowPERClosure(tag, indent, "MediaPacketizationCapability");
}

/* <==========================================> */
/*  PER-Analyzer for RSVPParameters (SEQUENCE)  */
/* <==========================================> */
void Analyze_RSVPParameters(PS_RSVPParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RSVPParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_qosMode", x->option_of_qosMode);
    ShowPERBoolean(tag, indent, "option_of_tokenRate", x->option_of_tokenRate);
    ShowPERBoolean(tag, indent, "option_of_bucketSize", x->option_of_bucketSize);
    ShowPERBoolean(tag, indent, "option_of_peakRate", x->option_of_peakRate);
    ShowPERBoolean(tag, indent, "option_of_minPoliced", x->option_of_minPoliced);
    ShowPERBoolean(tag, indent, "option_of_maxPktSize", x->option_of_maxPktSize);
    if (x->option_of_qosMode)
    {
        Analyze_QOSMode(&x->qosMode, "qosMode", tag, indent);
    }
    if (x->option_of_tokenRate)
    {
        ShowPERInteger(tag, indent, "tokenRate", (int32)x->tokenRate);
    }
    if (x->option_of_bucketSize)
    {
        ShowPERInteger(tag, indent, "bucketSize", (int32)x->bucketSize);
    }
    if (x->option_of_peakRate)
    {
        ShowPERInteger(tag, indent, "peakRate", (int32)x->peakRate);
    }
    if (x->option_of_minPoliced)
    {
        ShowPERInteger(tag, indent, "minPoliced", (int32)x->minPoliced);
    }
    if (x->option_of_maxPktSize)
    {
        ShowPERInteger(tag, indent, "maxPktSize", (int32)x->maxPktSize);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RSVPParameters");
}

/* <=================================> */
/*  PER-Analyzer for QOSMode (CHOICE)  */
/* <=================================> */
void Analyze_QOSMode(PS_QOSMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "QOSMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "guaranteedQOS");
            break;
        case 1:
            ShowPERNull(tag, indent, "controlledLoad");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_QOSMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "QOSMode");
}

/* <=========================================> */
/*  PER-Analyzer for ATMParameters (SEQUENCE)  */
/* <=========================================> */
void Analyze_ATMParameters(PS_ATMParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ATMParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "maxNTUSize", (int32)x->maxNTUSize);
    ShowPERBoolean(tag, indent, "atmUBR", x->atmUBR);
    ShowPERBoolean(tag, indent, "atmrtVBR", x->atmrtVBR);
    ShowPERBoolean(tag, indent, "atmnrtVBR", x->atmnrtVBR);
    ShowPERBoolean(tag, indent, "atmABR", x->atmABR);
    ShowPERBoolean(tag, indent, "atmCBR", x->atmCBR);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ATMParameters");
}

/* <=========================================> */
/*  PER-Analyzer for QOSCapability (SEQUENCE)  */
/* <=========================================> */
void Analyze_QOSCapability(PS_QOSCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "QOSCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandardData", x->option_of_nonStandardData);
    ShowPERBoolean(tag, indent, "option_of_rsvpParameters", x->option_of_rsvpParameters);
    ShowPERBoolean(tag, indent, "option_of_atmParameters", x->option_of_atmParameters);
    if (x->option_of_nonStandardData)
    {
        Analyze_NonStandardParameter(&x->nonStandardData, "nonStandardData", tag, indent);
    }
    if (x->option_of_rsvpParameters)
    {
        Analyze_RSVPParameters(&x->rsvpParameters, "rsvpParameters", tag, indent);
    }
    if (x->option_of_atmParameters)
    {
        Analyze_ATMParameters(&x->atmParameters, "atmParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "QOSCapability");
}

/* <============================================> */
/*  PER-Analyzer for MediaTransportType (CHOICE)  */
/* <============================================> */
void Analyze_MediaTransportType(PS_MediaTransportType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MediaTransportType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "ip_UDP");
            break;
        case 1:
            ShowPERNull(tag, indent, "ip_TCP");
            break;
        case 2:
            ShowPERNull(tag, indent, "atm_AAL5_UNIDIR");
            break;
        case 3:
            ShowPERNull(tag, indent, "atm_AAL5_BIDIR");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            Analyze_Atm_AAL5_compressed(x->atm_AAL5_compressed, "atm_AAL5_compressed", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_MediaTransportType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MediaTransportType");
}

/* <===============================================> */
/*  PER-Analyzer for Atm_AAL5_compressed (SEQUENCE)  */
/* <===============================================> */
void Analyze_Atm_AAL5_compressed(PS_Atm_AAL5_compressed x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Atm_AAL5_compressed");
    indent += 2;
    ShowPERBoolean(tag, indent, "variable_delta", x->variable_delta);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Atm_AAL5_compressed");
}

/* <==================================================> */
/*  PER-Analyzer for MediaChannelCapability (SEQUENCE)  */
/* <==================================================> */
void Analyze_MediaChannelCapability(PS_MediaChannelCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MediaChannelCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_mediaTransport", x->option_of_mediaTransport);
    if (x->option_of_mediaTransport)
    {
        Analyze_MediaTransportType(&x->mediaTransport, "mediaTransport", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MediaChannelCapability");
}

/* <===============================================> */
/*  PER-Analyzer for TransportCapability (SEQUENCE)  */
/* <===============================================> */
void Analyze_TransportCapability(PS_TransportCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "TransportCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    ShowPERBoolean(tag, indent, "option_of_qOSCapabilities", x->option_of_qOSCapabilities);
    ShowPERBoolean(tag, indent, "option_of_mediaChannelCapabilities", x->option_of_mediaChannelCapabilities);
    if (x->option_of_nonStandard)
    {
        Analyze_NonStandardParameter(&x->nonStandard, "nonStandard", tag, indent);
    }
    if (x->option_of_qOSCapabilities)
    {
        ShowPERInteger(tag, indent, "size_of_qOSCapabilities", (uint32)x->size_of_qOSCapabilities);
        for (i = 0;i < x->size_of_qOSCapabilities;++i)
        {
            sprintf(tempLabelString, "qOSCapabilities[%hu]", i);
            Analyze_QOSCapability(x->qOSCapabilities + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_mediaChannelCapabilities)
    {
        ShowPERInteger(tag, indent, "size_of_mediaChannelCapabilities", (uint32)x->size_of_mediaChannelCapabilities);
        for (i = 0;i < x->size_of_mediaChannelCapabilities;++i)
        {
            sprintf(tempLabelString, "mediaChannelCapabilities[%hu]", i);
            Analyze_MediaChannelCapability(x->mediaChannelCapabilities + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TransportCapability");
}

/* <========================================================> */
/*  PER-Analyzer for RedundancyEncodingCapability (SEQUENCE)  */
/* <========================================================> */
void Analyze_RedundancyEncodingCapability(PS_RedundancyEncodingCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RedundancyEncodingCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_secondaryEncoding", x->option_of_secondaryEncoding);
    Analyze_RedundancyEncodingMethod(&x->redundancyEncodingMethod, "redundancyEncodingMethod", tag, indent);
    ShowPERInteger(tag, indent, "primaryEncoding", (int32)x->primaryEncoding);
    if (x->option_of_secondaryEncoding)
    {
        ShowPERInteger(tag, indent, "size_of_secondaryEncoding", (uint32)x->size_of_secondaryEncoding);
        for (i = 0;i < x->size_of_secondaryEncoding;++i)
        {
            ShowPERIntegers(tag, indent, "secondaryEncoding", (uint32)x->secondaryEncoding[i], i);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RedundancyEncodingCapability");
}

/* <==================================================> */
/*  PER-Analyzer for RedundancyEncodingMethod (CHOICE)  */
/* <==================================================> */
void Analyze_RedundancyEncodingMethod(PS_RedundancyEncodingMethod x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RedundancyEncodingMethod");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "rtpAudioRedundancyEncoding");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Analyze_RTPH263VideoRedundancyEncoding(x->rtpH263VideoRedundancyEncoding, "rtpH263VideoRedundancyEncoding", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_RedundancyEncodingMethod: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RedundancyEncodingMethod");
}

/* <==========================================================> */
/*  PER-Analyzer for RTPH263VideoRedundancyEncoding (SEQUENCE)  */
/* <==========================================================> */
void Analyze_RTPH263VideoRedundancyEncoding(PS_RTPH263VideoRedundancyEncoding x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RTPH263VideoRedundancyEncoding");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_containedThreads", x->option_of_containedThreads);
    ShowPERInteger(tag, indent, "numberOfThreads", (int32)x->numberOfThreads);
    ShowPERInteger(tag, indent, "framesBetweenSyncPoints", (int32)x->framesBetweenSyncPoints);
    Analyze_FrameToThreadMapping(&x->frameToThreadMapping, "frameToThreadMapping", tag, indent);
    if (x->option_of_containedThreads)
    {
        ShowPERInteger(tag, indent, "size_of_containedThreads", (uint32)x->size_of_containedThreads);
        for (i = 0;i < x->size_of_containedThreads;++i)
        {
            ShowPERIntegers(tag, indent, "containedThreads", (uint32)x->containedThreads[i], i);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RTPH263VideoRedundancyEncoding");
}

/* <==============================================> */
/*  PER-Analyzer for FrameToThreadMapping (CHOICE)  */
/* <==============================================> */
void Analyze_FrameToThreadMapping(PS_FrameToThreadMapping x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "FrameToThreadMapping");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "roundrobin");
            break;
        case 1:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "custom[%hu]", i);
                Analyze_RTPH263VideoRedundancyFrameMapping(x->custom + i, tempLabelString, tag, indent);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_FrameToThreadMapping: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FrameToThreadMapping");
}

/* <==============================================================> */
/*  PER-Analyzer for RTPH263VideoRedundancyFrameMapping (SEQUENCE)  */
/* <==============================================================> */
void Analyze_RTPH263VideoRedundancyFrameMapping(PS_RTPH263VideoRedundancyFrameMapping x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RTPH263VideoRedundancyFrameMapping");
    indent += 2;
    ShowPERInteger(tag, indent, "threadNumber", (int32)x->threadNumber);
    ShowPERInteger(tag, indent, "size_of_frameSequence", (uint32)x->size_of_frameSequence);
    for (i = 0;i < x->size_of_frameSequence;++i)
    {
        ShowPERIntegers(tag, indent, "frameSequence", (uint32)x->frameSequence[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RTPH263VideoRedundancyFrameMapping");
}

/* <================================================> */
/*  PER-Analyzer for MultipointCapability (SEQUENCE)  */
/* <================================================> */
void Analyze_MultipointCapability(PS_MultipointCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultipointCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "multicastCapability", x->multicastCapability);
    ShowPERBoolean(tag, indent, "multiUniCastConference", x->multiUniCastConference);
    ShowPERInteger(tag, indent, "size_of_mediaDistributionCapability", (uint32)x->size_of_mediaDistributionCapability);
    for (i = 0;i < x->size_of_mediaDistributionCapability;++i)
    {
        sprintf(tempLabelString, "mediaDistributionCapability[%hu]", i);
        Analyze_MediaDistributionCapability(x->mediaDistributionCapability + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultipointCapability");
}

/* <=======================================================> */
/*  PER-Analyzer for MediaDistributionCapability (SEQUENCE)  */
/* <=======================================================> */
void Analyze_MediaDistributionCapability(PS_MediaDistributionCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MediaDistributionCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_centralizedData", x->option_of_centralizedData);
    ShowPERBoolean(tag, indent, "option_of_distributedData", x->option_of_distributedData);
    ShowPERBoolean(tag, indent, "centralizedControl", x->centralizedControl);
    ShowPERBoolean(tag, indent, "distributedControl", x->distributedControl);
    ShowPERBoolean(tag, indent, "centralizedAudio", x->centralizedAudio);
    ShowPERBoolean(tag, indent, "distributedAudio", x->distributedAudio);
    ShowPERBoolean(tag, indent, "centralizedVideo", x->centralizedVideo);
    ShowPERBoolean(tag, indent, "distributedVideo", x->distributedVideo);
    if (x->option_of_centralizedData)
    {
        ShowPERInteger(tag, indent, "size_of_centralizedData", (uint32)x->size_of_centralizedData);
        for (i = 0;i < x->size_of_centralizedData;++i)
        {
            sprintf(tempLabelString, "centralizedData[%hu]", i);
            Analyze_DataApplicationCapability(x->centralizedData + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_distributedData)
    {
        ShowPERInteger(tag, indent, "size_of_distributedData", (uint32)x->size_of_distributedData);
        for (i = 0;i < x->size_of_distributedData;++i)
        {
            sprintf(tempLabelString, "distributedData[%hu]", i);
            Analyze_DataApplicationCapability(x->distributedData + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MediaDistributionCapability");
}

/* <=========================================> */
/*  PER-Analyzer for VideoCapability (CHOICE)  */
/* <=========================================> */
void Analyze_VideoCapability(PS_VideoCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "VideoCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_H261VideoCapability(x->h261VideoCapability, "h261VideoCapability", tag, indent);
            break;
        case 2:
            Analyze_H262VideoCapability(x->h262VideoCapability, "h262VideoCapability", tag, indent);
            break;
        case 3:
            Analyze_H263VideoCapability(x->h263VideoCapability, "h263VideoCapability", tag, indent);
            break;
        case 4:
            Analyze_IS11172VideoCapability(x->is11172VideoCapability, "is11172VideoCapability", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Analyze_GenericCapability(x->genericVideoCapability, "genericVideoCapability", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_VideoCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "VideoCapability");
}

/* <===============================================> */
/*  PER-Analyzer for H261VideoCapability (SEQUENCE)  */
/* <===============================================> */
void Analyze_H261VideoCapability(PS_H261VideoCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H261VideoCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_qcifMPI", x->option_of_qcifMPI);
    ShowPERBoolean(tag, indent, "option_of_cifMPI", x->option_of_cifMPI);
    if (x->option_of_qcifMPI)
    {
        ShowPERInteger(tag, indent, "qcifMPI", (int32)x->qcifMPI);
    }
    if (x->option_of_cifMPI)
    {
        ShowPERInteger(tag, indent, "cifMPI", (int32)x->cifMPI);
    }
    ShowPERBoolean(tag, indent, "temporalSpatialTradeOffCapability", x->temporalSpatialTradeOffCapability);
    ShowPERInteger(tag, indent, "maxBitRate", (int32)x->maxBitRate);
    ShowPERBoolean(tag, indent, "stillImageTransmission", x->stillImageTransmission);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H261VideoCapability");
}

/* <===============================================> */
/*  PER-Analyzer for H262VideoCapability (SEQUENCE)  */
/* <===============================================> */
void Analyze_H262VideoCapability(PS_H262VideoCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H262VideoCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_videoBitRate", x->option_of_videoBitRate);
    ShowPERBoolean(tag, indent, "option_of_vbvBufferSize", x->option_of_vbvBufferSize);
    ShowPERBoolean(tag, indent, "option_of_samplesPerLine", x->option_of_samplesPerLine);
    ShowPERBoolean(tag, indent, "option_of_linesPerFrame", x->option_of_linesPerFrame);
    ShowPERBoolean(tag, indent, "option_of_framesPerSecond", x->option_of_framesPerSecond);
    ShowPERBoolean(tag, indent, "option_of_luminanceSampleRate", x->option_of_luminanceSampleRate);
    ShowPERBoolean(tag, indent, "profileAndLevel_SPatML", x->profileAndLevel_SPatML);
    ShowPERBoolean(tag, indent, "profileAndLevel_MPatLL", x->profileAndLevel_MPatLL);
    ShowPERBoolean(tag, indent, "profileAndLevel_MPatML", x->profileAndLevel_MPatML);
    ShowPERBoolean(tag, indent, "profileAndLevel_MPatH_14", x->profileAndLevel_MPatH_14);
    ShowPERBoolean(tag, indent, "profileAndLevel_MPatHL", x->profileAndLevel_MPatHL);
    ShowPERBoolean(tag, indent, "profileAndLevel_SNRatLL", x->profileAndLevel_SNRatLL);
    ShowPERBoolean(tag, indent, "profileAndLevel_SNRatML", x->profileAndLevel_SNRatML);
    ShowPERBoolean(tag, indent, "profileAndLevel_SpatialatH_14", x->profileAndLevel_SpatialatH_14);
    ShowPERBoolean(tag, indent, "profileAndLevel_HPatML", x->profileAndLevel_HPatML);
    ShowPERBoolean(tag, indent, "profileAndLevel_HPatH_14", x->profileAndLevel_HPatH_14);
    ShowPERBoolean(tag, indent, "profileAndLevel_HPatHL", x->profileAndLevel_HPatHL);
    if (x->option_of_videoBitRate)
    {
        ShowPERInteger(tag, indent, "videoBitRate", (int32)x->videoBitRate);
    }
    if (x->option_of_vbvBufferSize)
    {
        ShowPERInteger(tag, indent, "vbvBufferSize", (int32)x->vbvBufferSize);
    }
    if (x->option_of_samplesPerLine)
    {
        ShowPERInteger(tag, indent, "samplesPerLine", (int32)x->samplesPerLine);
    }
    if (x->option_of_linesPerFrame)
    {
        ShowPERInteger(tag, indent, "linesPerFrame", (int32)x->linesPerFrame);
    }
    if (x->option_of_framesPerSecond)
    {
        ShowPERInteger(tag, indent, "framesPerSecond", (int32)x->framesPerSecond);
    }
    if (x->option_of_luminanceSampleRate)
    {
        ShowPERInteger(tag, indent, "luminanceSampleRate", (int32)x->luminanceSampleRate);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H262VideoCapability");
}

/* <===============================================> */
/*  PER-Analyzer for H263VideoCapability (SEQUENCE)  */
/* <===============================================> */
void Analyze_H263VideoCapability(PS_H263VideoCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H263VideoCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_sqcifMPI", x->option_of_sqcifMPI);
    ShowPERBoolean(tag, indent, "option_of_qcifMPI", x->option_of_qcifMPI);
    ShowPERBoolean(tag, indent, "option_of_cifMPI", x->option_of_cifMPI);
    ShowPERBoolean(tag, indent, "option_of_cif4MPI", x->option_of_cif4MPI);
    ShowPERBoolean(tag, indent, "option_of_cif16MPI", x->option_of_cif16MPI);
    ShowPERBoolean(tag, indent, "option_of_hrd_B", x->option_of_hrd_B);
    ShowPERBoolean(tag, indent, "option_of_bppMaxKb", x->option_of_bppMaxKb);
    if (x->option_of_sqcifMPI)
    {
        ShowPERInteger(tag, indent, "sqcifMPI", (int32)x->sqcifMPI);
    }
    if (x->option_of_qcifMPI)
    {
        ShowPERInteger(tag, indent, "qcifMPI", (int32)x->qcifMPI);
    }
    if (x->option_of_cifMPI)
    {
        ShowPERInteger(tag, indent, "cifMPI", (int32)x->cifMPI);
    }
    if (x->option_of_cif4MPI)
    {
        ShowPERInteger(tag, indent, "cif4MPI", (int32)x->cif4MPI);
    }
    if (x->option_of_cif16MPI)
    {
        ShowPERInteger(tag, indent, "cif16MPI", (int32)x->cif16MPI);
    }
    ShowPERInteger(tag, indent, "maxBitRate", (int32)x->maxBitRate);
    ShowPERBoolean(tag, indent, "unrestrictedVector", x->unrestrictedVector);
    ShowPERBoolean(tag, indent, "arithmeticCoding", x->arithmeticCoding);
    ShowPERBoolean(tag, indent, "advancedPrediction", x->advancedPrediction);
    ShowPERBoolean(tag, indent, "pbFrames", x->pbFrames);
    ShowPERBoolean(tag, indent, "temporalSpatialTradeOffCapability", x->temporalSpatialTradeOffCapability);
    if (x->option_of_hrd_B)
    {
        ShowPERInteger(tag, indent, "hrd_B", (int32)x->hrd_B);
    }
    if (x->option_of_bppMaxKb)
    {
        ShowPERInteger(tag, indent, "bppMaxKb", (int32)x->bppMaxKb);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_slowSqcifMPI", x->option_of_slowSqcifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowQcifMPI", x->option_of_slowQcifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowCifMPI", x->option_of_slowCifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowCif4MPI", x->option_of_slowCif4MPI);
    ShowPERBoolean(tag, indent, "option_of_slowCif16MPI", x->option_of_slowCif16MPI);
    ShowPERBoolean(tag, indent, "option_of_errorCompensation", x->option_of_errorCompensation);
    ShowPERBoolean(tag, indent, "option_of_enhancementLayerInfo", x->option_of_enhancementLayerInfo);
    ShowPERBoolean(tag, indent, "option_of_h263Options", x->option_of_h263Options);
    if (x->option_of_slowSqcifMPI)
    {
        ShowPERInteger(tag, indent, "slowSqcifMPI", (int32)x->slowSqcifMPI);
    }
    if (x->option_of_slowQcifMPI)
    {
        ShowPERInteger(tag, indent, "slowQcifMPI", (int32)x->slowQcifMPI);
    }
    if (x->option_of_slowCifMPI)
    {
        ShowPERInteger(tag, indent, "slowCifMPI", (int32)x->slowCifMPI);
    }
    if (x->option_of_slowCif4MPI)
    {
        ShowPERInteger(tag, indent, "slowCif4MPI", (int32)x->slowCif4MPI);
    }
    if (x->option_of_slowCif16MPI)
    {
        ShowPERInteger(tag, indent, "slowCif16MPI", (int32)x->slowCif16MPI);
    }
    if (x->option_of_errorCompensation)
    {
        ShowPERBoolean(tag, indent, "errorCompensation", x->errorCompensation);
    }
    if (x->option_of_enhancementLayerInfo)
    {
        Analyze_EnhancementLayerInfo(&x->enhancementLayerInfo, "enhancementLayerInfo", tag, indent);
    }
    if (x->option_of_h263Options)
    {
        Analyze_H263Options(&x->h263Options, "h263Options", tag, indent);
    }
    ShowPERClosure(tag, indent, "H263VideoCapability");
}

/* <================================================> */
/*  PER-Analyzer for EnhancementLayerInfo (SEQUENCE)  */
/* <================================================> */
void Analyze_EnhancementLayerInfo(PS_EnhancementLayerInfo x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "EnhancementLayerInfo");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_snrEnhancement", x->option_of_snrEnhancement);
    ShowPERBoolean(tag, indent, "option_of_spatialEnhancement", x->option_of_spatialEnhancement);
    ShowPERBoolean(tag, indent, "option_of_bPictureEnhancement", x->option_of_bPictureEnhancement);
    ShowPERBoolean(tag, indent, "baseBitRateConstrained", x->baseBitRateConstrained);
    if (x->option_of_snrEnhancement)
    {
        ShowPERInteger(tag, indent, "size_of_snrEnhancement", (uint32)x->size_of_snrEnhancement);
        for (i = 0;i < x->size_of_snrEnhancement;++i)
        {
            sprintf(tempLabelString, "snrEnhancement[%hu]", i);
            Analyze_EnhancementOptions(x->snrEnhancement + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_spatialEnhancement)
    {
        ShowPERInteger(tag, indent, "size_of_spatialEnhancement", (uint32)x->size_of_spatialEnhancement);
        for (i = 0;i < x->size_of_spatialEnhancement;++i)
        {
            sprintf(tempLabelString, "spatialEnhancement[%hu]", i);
            Analyze_EnhancementOptions(x->spatialEnhancement + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_bPictureEnhancement)
    {
        ShowPERInteger(tag, indent, "size_of_bPictureEnhancement", (uint32)x->size_of_bPictureEnhancement);
        for (i = 0;i < x->size_of_bPictureEnhancement;++i)
        {
            sprintf(tempLabelString, "bPictureEnhancement[%hu]", i);
            Analyze_BEnhancementParameters(x->bPictureEnhancement + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EnhancementLayerInfo");
}

/* <==================================================> */
/*  PER-Analyzer for BEnhancementParameters (SEQUENCE)  */
/* <==================================================> */
void Analyze_BEnhancementParameters(PS_BEnhancementParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "BEnhancementParameters");
    indent += 2;
    Analyze_EnhancementOptions(&x->enhancementOptions, "enhancementOptions", tag, indent);
    ShowPERInteger(tag, indent, "numberOfBPictures", (int32)x->numberOfBPictures);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "BEnhancementParameters");
}

/* <==============================================> */
/*  PER-Analyzer for EnhancementOptions (SEQUENCE)  */
/* <==============================================> */
void Analyze_EnhancementOptions(PS_EnhancementOptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "EnhancementOptions");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_sqcifMPI", x->option_of_sqcifMPI);
    ShowPERBoolean(tag, indent, "option_of_qcifMPI", x->option_of_qcifMPI);
    ShowPERBoolean(tag, indent, "option_of_cifMPI", x->option_of_cifMPI);
    ShowPERBoolean(tag, indent, "option_of_cif4MPI", x->option_of_cif4MPI);
    ShowPERBoolean(tag, indent, "option_of_cif16MPI", x->option_of_cif16MPI);
    ShowPERBoolean(tag, indent, "option_of_slowSqcifMPI", x->option_of_slowSqcifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowQcifMPI", x->option_of_slowQcifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowCifMPI", x->option_of_slowCifMPI);
    ShowPERBoolean(tag, indent, "option_of_slowCif4MPI", x->option_of_slowCif4MPI);
    ShowPERBoolean(tag, indent, "option_of_slowCif16MPI", x->option_of_slowCif16MPI);
    ShowPERBoolean(tag, indent, "option_of_h263Options", x->option_of_h263Options);
    if (x->option_of_sqcifMPI)
    {
        ShowPERInteger(tag, indent, "sqcifMPI", (int32)x->sqcifMPI);
    }
    if (x->option_of_qcifMPI)
    {
        ShowPERInteger(tag, indent, "qcifMPI", (int32)x->qcifMPI);
    }
    if (x->option_of_cifMPI)
    {
        ShowPERInteger(tag, indent, "cifMPI", (int32)x->cifMPI);
    }
    if (x->option_of_cif4MPI)
    {
        ShowPERInteger(tag, indent, "cif4MPI", (int32)x->cif4MPI);
    }
    if (x->option_of_cif16MPI)
    {
        ShowPERInteger(tag, indent, "cif16MPI", (int32)x->cif16MPI);
    }
    ShowPERInteger(tag, indent, "maxBitRate", (int32)x->maxBitRate);
    ShowPERBoolean(tag, indent, "unrestrictedVector", x->unrestrictedVector);
    ShowPERBoolean(tag, indent, "arithmeticCoding", x->arithmeticCoding);
    ShowPERBoolean(tag, indent, "temporalSpatialTradeOffCapability", x->temporalSpatialTradeOffCapability);
    if (x->option_of_slowSqcifMPI)
    {
        ShowPERInteger(tag, indent, "slowSqcifMPI", (int32)x->slowSqcifMPI);
    }
    if (x->option_of_slowQcifMPI)
    {
        ShowPERInteger(tag, indent, "slowQcifMPI", (int32)x->slowQcifMPI);
    }
    if (x->option_of_slowCifMPI)
    {
        ShowPERInteger(tag, indent, "slowCifMPI", (int32)x->slowCifMPI);
    }
    if (x->option_of_slowCif4MPI)
    {
        ShowPERInteger(tag, indent, "slowCif4MPI", (int32)x->slowCif4MPI);
    }
    if (x->option_of_slowCif16MPI)
    {
        ShowPERInteger(tag, indent, "slowCif16MPI", (int32)x->slowCif16MPI);
    }
    ShowPERBoolean(tag, indent, "errorCompensation", x->errorCompensation);
    if (x->option_of_h263Options)
    {
        Analyze_H263Options(&x->h263Options, "h263Options", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EnhancementOptions");
}

/* <=======================================> */
/*  PER-Analyzer for H263Options (SEQUENCE)  */
/* <=======================================> */
void Analyze_H263Options(PS_H263Options x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H263Options");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_transparencyParameters", x->option_of_transparencyParameters);
    ShowPERBoolean(tag, indent, "option_of_refPictureSelection", x->option_of_refPictureSelection);
    ShowPERBoolean(tag, indent, "option_of_customPictureClockFrequency", x->option_of_customPictureClockFrequency);
    ShowPERBoolean(tag, indent, "option_of_customPictureFormat", x->option_of_customPictureFormat);
    ShowPERBoolean(tag, indent, "option_of_modeCombos", x->option_of_modeCombos);
    ShowPERBoolean(tag, indent, "advancedIntraCodingMode", x->advancedIntraCodingMode);
    ShowPERBoolean(tag, indent, "deblockingFilterMode", x->deblockingFilterMode);
    ShowPERBoolean(tag, indent, "improvedPBFramesMode", x->improvedPBFramesMode);
    ShowPERBoolean(tag, indent, "unlimitedMotionVectors", x->unlimitedMotionVectors);
    ShowPERBoolean(tag, indent, "fullPictureFreeze", x->fullPictureFreeze);
    ShowPERBoolean(tag, indent, "partialPictureFreezeAndRelease", x->partialPictureFreezeAndRelease);
    ShowPERBoolean(tag, indent, "resizingPartPicFreezeAndRelease", x->resizingPartPicFreezeAndRelease);
    ShowPERBoolean(tag, indent, "fullPictureSnapshot", x->fullPictureSnapshot);
    ShowPERBoolean(tag, indent, "partialPictureSnapshot", x->partialPictureSnapshot);
    ShowPERBoolean(tag, indent, "videoSegmentTagging", x->videoSegmentTagging);
    ShowPERBoolean(tag, indent, "progressiveRefinement", x->progressiveRefinement);
    ShowPERBoolean(tag, indent, "dynamicPictureResizingByFour", x->dynamicPictureResizingByFour);
    ShowPERBoolean(tag, indent, "dynamicPictureResizingSixteenthPel", x->dynamicPictureResizingSixteenthPel);
    ShowPERBoolean(tag, indent, "dynamicWarpingHalfPel", x->dynamicWarpingHalfPel);
    ShowPERBoolean(tag, indent, "dynamicWarpingSixteenthPel", x->dynamicWarpingSixteenthPel);
    ShowPERBoolean(tag, indent, "independentSegmentDecoding", x->independentSegmentDecoding);
    ShowPERBoolean(tag, indent, "slicesInOrder_NonRect", x->slicesInOrder_NonRect);
    ShowPERBoolean(tag, indent, "slicesInOrder_Rect", x->slicesInOrder_Rect);
    ShowPERBoolean(tag, indent, "slicesNoOrder_NonRect", x->slicesNoOrder_NonRect);
    ShowPERBoolean(tag, indent, "slicesNoOrder_Rect", x->slicesNoOrder_Rect);
    ShowPERBoolean(tag, indent, "alternateInterVLCMode", x->alternateInterVLCMode);
    ShowPERBoolean(tag, indent, "modifiedQuantizationMode", x->modifiedQuantizationMode);
    ShowPERBoolean(tag, indent, "reducedResolutionUpdate", x->reducedResolutionUpdate);
    if (x->option_of_transparencyParameters)
    {
        Analyze_TransparencyParameters(&x->transparencyParameters, "transparencyParameters", tag, indent);
    }
    ShowPERBoolean(tag, indent, "separateVideoBackChannel", x->separateVideoBackChannel);
    if (x->option_of_refPictureSelection)
    {
        Analyze_RefPictureSelection(&x->refPictureSelection, "refPictureSelection", tag, indent);
    }
    if (x->option_of_customPictureClockFrequency)
    {
        ShowPERInteger(tag, indent, "size_of_customPictureClockFrequency", (uint32)x->size_of_customPictureClockFrequency);
        for (i = 0;i < x->size_of_customPictureClockFrequency;++i)
        {
            sprintf(tempLabelString, "customPictureClockFrequency[%hu]", i);
            Analyze_CustomPictureClockFrequency(x->customPictureClockFrequency + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_customPictureFormat)
    {
        ShowPERInteger(tag, indent, "size_of_customPictureFormat", (uint32)x->size_of_customPictureFormat);
        for (i = 0;i < x->size_of_customPictureFormat;++i)
        {
            sprintf(tempLabelString, "customPictureFormat[%hu]", i);
            Analyze_CustomPictureFormat(x->customPictureFormat + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_modeCombos)
    {
        ShowPERInteger(tag, indent, "size_of_modeCombos", (uint32)x->size_of_modeCombos);
        for (i = 0;i < x->size_of_modeCombos;++i)
        {
            sprintf(tempLabelString, "modeCombos[%hu]", i);
            Analyze_H263VideoModeCombos(x->modeCombos + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H263Options");
}

/* <==================================================> */
/*  PER-Analyzer for TransparencyParameters (SEQUENCE)  */
/* <==================================================> */
void Analyze_TransparencyParameters(PS_TransparencyParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TransparencyParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "presentationOrder", (int32)x->presentationOrder);
    ShowPERSignedInteger(tag, indent, "offset_x", (int32)x->offset_x);
    ShowPERSignedInteger(tag, indent, "offset_y", (int32)x->offset_y);
    ShowPERInteger(tag, indent, "scale_x", (int32)x->scale_x);
    ShowPERInteger(tag, indent, "scale_y", (int32)x->scale_y);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TransparencyParameters");
}

/* <===============================================> */
/*  PER-Analyzer for RefPictureSelection (SEQUENCE)  */
/* <===============================================> */
void Analyze_RefPictureSelection(PS_RefPictureSelection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RefPictureSelection");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_additionalPictureMemory", x->option_of_additionalPictureMemory);
    if (x->option_of_additionalPictureMemory)
    {
        Analyze_AdditionalPictureMemory(&x->additionalPictureMemory, "additionalPictureMemory", tag, indent);
    }
    ShowPERBoolean(tag, indent, "videoMux", x->videoMux);
    Analyze_VideoBackChannelSend(&x->videoBackChannelSend, "videoBackChannelSend", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RefPictureSelection");
}

/* <==============================================> */
/*  PER-Analyzer for VideoBackChannelSend (CHOICE)  */
/* <==============================================> */
void Analyze_VideoBackChannelSend(PS_VideoBackChannelSend x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "VideoBackChannelSend");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "none");
            break;
        case 1:
            ShowPERNull(tag, indent, "ackMessageOnly");
            break;
        case 2:
            ShowPERNull(tag, indent, "nackMessageOnly");
            break;
        case 3:
            ShowPERNull(tag, indent, "ackOrNackMessageOnly");
            break;
        case 4:
            ShowPERNull(tag, indent, "ackAndNackMessage");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_VideoBackChannelSend: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "VideoBackChannelSend");
}

/* <===================================================> */
/*  PER-Analyzer for AdditionalPictureMemory (SEQUENCE)  */
/* <===================================================> */
void Analyze_AdditionalPictureMemory(PS_AdditionalPictureMemory x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "AdditionalPictureMemory");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_sqcifAdditionalPictureMemory", x->option_of_sqcifAdditionalPictureMemory);
    ShowPERBoolean(tag, indent, "option_of_qcifAdditionalPictureMemory", x->option_of_qcifAdditionalPictureMemory);
    ShowPERBoolean(tag, indent, "option_of_cifAdditionalPictureMemory", x->option_of_cifAdditionalPictureMemory);
    ShowPERBoolean(tag, indent, "option_of_cif4AdditionalPictureMemory", x->option_of_cif4AdditionalPictureMemory);
    ShowPERBoolean(tag, indent, "option_of_cif16AdditionalPictureMemory", x->option_of_cif16AdditionalPictureMemory);
    ShowPERBoolean(tag, indent, "option_of_bigCpfAdditionalPictureMemory", x->option_of_bigCpfAdditionalPictureMemory);
    if (x->option_of_sqcifAdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "sqcifAdditionalPictureMemory", (int32)x->sqcifAdditionalPictureMemory);
    }
    if (x->option_of_qcifAdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "qcifAdditionalPictureMemory", (int32)x->qcifAdditionalPictureMemory);
    }
    if (x->option_of_cifAdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "cifAdditionalPictureMemory", (int32)x->cifAdditionalPictureMemory);
    }
    if (x->option_of_cif4AdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "cif4AdditionalPictureMemory", (int32)x->cif4AdditionalPictureMemory);
    }
    if (x->option_of_cif16AdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "cif16AdditionalPictureMemory", (int32)x->cif16AdditionalPictureMemory);
    }
    if (x->option_of_bigCpfAdditionalPictureMemory)
    {
        ShowPERInteger(tag, indent, "bigCpfAdditionalPictureMemory", (int32)x->bigCpfAdditionalPictureMemory);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "AdditionalPictureMemory");
}

/* <=======================================================> */
/*  PER-Analyzer for CustomPictureClockFrequency (SEQUENCE)  */
/* <=======================================================> */
void Analyze_CustomPictureClockFrequency(PS_CustomPictureClockFrequency x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CustomPictureClockFrequency");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_sqcifMPI", x->option_of_sqcifMPI);
    ShowPERBoolean(tag, indent, "option_of_qcifMPI", x->option_of_qcifMPI);
    ShowPERBoolean(tag, indent, "option_of_cifMPI", x->option_of_cifMPI);
    ShowPERBoolean(tag, indent, "option_of_cif4MPI", x->option_of_cif4MPI);
    ShowPERBoolean(tag, indent, "option_of_cif16MPI", x->option_of_cif16MPI);
    ShowPERInteger(tag, indent, "clockConversionCode", (int32)x->clockConversionCode);
    ShowPERInteger(tag, indent, "clockDivisor", (int32)x->clockDivisor);
    if (x->option_of_sqcifMPI)
    {
        ShowPERInteger(tag, indent, "sqcifMPI", (int32)x->sqcifMPI);
    }
    if (x->option_of_qcifMPI)
    {
        ShowPERInteger(tag, indent, "qcifMPI", (int32)x->qcifMPI);
    }
    if (x->option_of_cifMPI)
    {
        ShowPERInteger(tag, indent, "cifMPI", (int32)x->cifMPI);
    }
    if (x->option_of_cif4MPI)
    {
        ShowPERInteger(tag, indent, "cif4MPI", (int32)x->cif4MPI);
    }
    if (x->option_of_cif16MPI)
    {
        ShowPERInteger(tag, indent, "cif16MPI", (int32)x->cif16MPI);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CustomPictureClockFrequency");
}

/* <===============================================> */
/*  PER-Analyzer for CustomPictureFormat (SEQUENCE)  */
/* <===============================================> */
void Analyze_CustomPictureFormat(PS_CustomPictureFormat x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CustomPictureFormat");
    indent += 2;
    ShowPERInteger(tag, indent, "maxCustomPictureWidth", (int32)x->maxCustomPictureWidth);
    ShowPERInteger(tag, indent, "maxCustomPictureHeight", (int32)x->maxCustomPictureHeight);
    ShowPERInteger(tag, indent, "minCustomPictureWidth", (int32)x->minCustomPictureWidth);
    ShowPERInteger(tag, indent, "minCustomPictureHeight", (int32)x->minCustomPictureHeight);
    Analyze_MPI(&x->mPI, "mPI", tag, indent);
    Analyze_PixelAspectInformation(&x->pixelAspectInformation, "pixelAspectInformation", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CustomPictureFormat");
}

/* <================================================> */
/*  PER-Analyzer for PixelAspectInformation (CHOICE)  */
/* <================================================> */
void Analyze_PixelAspectInformation(PS_PixelAspectInformation x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "PixelAspectInformation");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERBoolean(tag, indent, "anyPixelAspectRatio", x->anyPixelAspectRatio);
            break;
        case 1:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                ShowPERIntegers(tag, indent, "pixelAspectCode", (uint32)x->pixelAspectCode[i], i);
            }
            break;
        case 2:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "extendedPAR[%hu]", i);
                Analyze_ExtendedPARItem(x->extendedPAR + i, tempLabelString, tag, indent);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_PixelAspectInformation: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "PixelAspectInformation");
}

/* <===========================================> */
/*  PER-Analyzer for ExtendedPARItem (SEQUENCE)  */
/* <===========================================> */
void Analyze_ExtendedPARItem(PS_ExtendedPARItem x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ExtendedPARItem");
    indent += 2;
    ShowPERInteger(tag, indent, "width", (int32)x->width);
    ShowPERInteger(tag, indent, "height", (int32)x->height);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ExtendedPARItem");
}

/* <===============================> */
/*  PER-Analyzer for MPI (SEQUENCE)  */
/* <===============================> */
void Analyze_MPI(PS_MPI x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MPI");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_standardMPI", x->option_of_standardMPI);
    ShowPERBoolean(tag, indent, "option_of_customPCF", x->option_of_customPCF);
    if (x->option_of_standardMPI)
    {
        ShowPERInteger(tag, indent, "standardMPI", (int32)x->standardMPI);
    }
    if (x->option_of_customPCF)
    {
        ShowPERInteger(tag, indent, "size_of_customPCF", (uint32)x->size_of_customPCF);
        for (i = 0;i < x->size_of_customPCF;++i)
        {
            sprintf(tempLabelString, "customPCF[%hu]", i);
            Analyze_CustomPCFItem(x->customPCF + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MPI");
}

/* <=========================================> */
/*  PER-Analyzer for CustomPCFItem (SEQUENCE)  */
/* <=========================================> */
void Analyze_CustomPCFItem(PS_CustomPCFItem x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CustomPCFItem");
    indent += 2;
    ShowPERInteger(tag, indent, "clockConversionCode", (int32)x->clockConversionCode);
    ShowPERInteger(tag, indent, "clockDivisor", (int32)x->clockDivisor);
    ShowPERInteger(tag, indent, "customMPI", (int32)x->customMPI);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CustomPCFItem");
}

/* <===============================================> */
/*  PER-Analyzer for H263VideoModeCombos (SEQUENCE)  */
/* <===============================================> */
void Analyze_H263VideoModeCombos(PS_H263VideoModeCombos x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H263VideoModeCombos");
    indent += 2;
    Analyze_H263ModeComboFlags(&x->h263VideoUncoupledModes, "h263VideoUncoupledModes", tag, indent);
    ShowPERInteger(tag, indent, "size_of_h263VideoCoupledModes", (uint32)x->size_of_h263VideoCoupledModes);
    for (i = 0;i < x->size_of_h263VideoCoupledModes;++i)
    {
        sprintf(tempLabelString, "h263VideoCoupledModes[%hu]", i);
        Analyze_H263ModeComboFlags(x->h263VideoCoupledModes + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H263VideoModeCombos");
}

/* <==============================================> */
/*  PER-Analyzer for H263ModeComboFlags (SEQUENCE)  */
/* <==============================================> */
void Analyze_H263ModeComboFlags(PS_H263ModeComboFlags x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H263ModeComboFlags");
    indent += 2;
    ShowPERBoolean(tag, indent, "unrestrictedVector", x->unrestrictedVector);
    ShowPERBoolean(tag, indent, "arithmeticCoding", x->arithmeticCoding);
    ShowPERBoolean(tag, indent, "advancedPrediction", x->advancedPrediction);
    ShowPERBoolean(tag, indent, "pbFrames", x->pbFrames);
    ShowPERBoolean(tag, indent, "advancedIntraCodingMode", x->advancedIntraCodingMode);
    ShowPERBoolean(tag, indent, "deblockingFilterMode", x->deblockingFilterMode);
    ShowPERBoolean(tag, indent, "unlimitedMotionVectors", x->unlimitedMotionVectors);
    ShowPERBoolean(tag, indent, "slicesInOrder_NonRect", x->slicesInOrder_NonRect);
    ShowPERBoolean(tag, indent, "slicesInOrder_Rect", x->slicesInOrder_Rect);
    ShowPERBoolean(tag, indent, "slicesNoOrder_NonRect", x->slicesNoOrder_NonRect);
    ShowPERBoolean(tag, indent, "slicesNoOrder_Rect", x->slicesNoOrder_Rect);
    ShowPERBoolean(tag, indent, "improvedPBFramesMode", x->improvedPBFramesMode);
    ShowPERBoolean(tag, indent, "referencePicSelect", x->referencePicSelect);
    ShowPERBoolean(tag, indent, "dynamicPictureResizingByFour", x->dynamicPictureResizingByFour);
    ShowPERBoolean(tag, indent, "dynamicPictureResizingSixteenthPel", x->dynamicPictureResizingSixteenthPel);
    ShowPERBoolean(tag, indent, "dynamicWarpingHalfPel", x->dynamicWarpingHalfPel);
    ShowPERBoolean(tag, indent, "dynamicWarpingSixteenthPel", x->dynamicWarpingSixteenthPel);
    ShowPERBoolean(tag, indent, "reducedResolutionUpdate", x->reducedResolutionUpdate);
    ShowPERBoolean(tag, indent, "independentSegmentDecoding", x->independentSegmentDecoding);
    ShowPERBoolean(tag, indent, "alternateInterVLCMode", x->alternateInterVLCMode);
    ShowPERBoolean(tag, indent, "modifiedQuantizationMode", x->modifiedQuantizationMode);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H263ModeComboFlags");
}

/* <==================================================> */
/*  PER-Analyzer for IS11172VideoCapability (SEQUENCE)  */
/* <==================================================> */
void Analyze_IS11172VideoCapability(PS_IS11172VideoCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS11172VideoCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_videoBitRate", x->option_of_videoBitRate);
    ShowPERBoolean(tag, indent, "option_of_vbvBufferSize", x->option_of_vbvBufferSize);
    ShowPERBoolean(tag, indent, "option_of_samplesPerLine", x->option_of_samplesPerLine);
    ShowPERBoolean(tag, indent, "option_of_linesPerFrame", x->option_of_linesPerFrame);
    ShowPERBoolean(tag, indent, "option_of_pictureRate", x->option_of_pictureRate);
    ShowPERBoolean(tag, indent, "option_of_luminanceSampleRate", x->option_of_luminanceSampleRate);
    ShowPERBoolean(tag, indent, "constrainedBitstream", x->constrainedBitstream);
    if (x->option_of_videoBitRate)
    {
        ShowPERInteger(tag, indent, "videoBitRate", (int32)x->videoBitRate);
    }
    if (x->option_of_vbvBufferSize)
    {
        ShowPERInteger(tag, indent, "vbvBufferSize", (int32)x->vbvBufferSize);
    }
    if (x->option_of_samplesPerLine)
    {
        ShowPERInteger(tag, indent, "samplesPerLine", (int32)x->samplesPerLine);
    }
    if (x->option_of_linesPerFrame)
    {
        ShowPERInteger(tag, indent, "linesPerFrame", (int32)x->linesPerFrame);
    }
    if (x->option_of_pictureRate)
    {
        ShowPERInteger(tag, indent, "pictureRate", (int32)x->pictureRate);
    }
    if (x->option_of_luminanceSampleRate)
    {
        ShowPERInteger(tag, indent, "luminanceSampleRate", (int32)x->luminanceSampleRate);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS11172VideoCapability");
}

/* <=========================================> */
/*  PER-Analyzer for AudioCapability (CHOICE)  */
/* <=========================================> */
void Analyze_AudioCapability(PS_AudioCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AudioCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERInteger(tag, indent, "g711Alaw64k", (uint32)x->g711Alaw64k);
            break;
        case 2:
            ShowPERInteger(tag, indent, "g711Alaw56k", (uint32)x->g711Alaw56k);
            break;
        case 3:
            ShowPERInteger(tag, indent, "g711Ulaw64k", (uint32)x->g711Ulaw64k);
            break;
        case 4:
            ShowPERInteger(tag, indent, "g711Ulaw56k", (uint32)x->g711Ulaw56k);
            break;
        case 5:
            ShowPERInteger(tag, indent, "g722_64k", (uint32)x->g722_64k);
            break;
        case 6:
            ShowPERInteger(tag, indent, "g722_56k", (uint32)x->g722_56k);
            break;
        case 7:
            ShowPERInteger(tag, indent, "g722_48k", (uint32)x->g722_48k);
            break;
        case 8:
            Analyze_G7231(x->g7231, "g7231", tag, indent);
            break;
        case 9:
            ShowPERInteger(tag, indent, "g728", (uint32)x->g728);
            break;
        case 10:
            ShowPERInteger(tag, indent, "g729", (uint32)x->g729);
            break;
        case 11:
            ShowPERInteger(tag, indent, "g729AnnexA", (uint32)x->g729AnnexA);
            break;
        case 12:
            Analyze_IS11172AudioCapability(x->is11172AudioCapability, "is11172AudioCapability", tag, indent);
            break;
        case 13:
            Analyze_IS13818AudioCapability(x->is13818AudioCapability, "is13818AudioCapability", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            ShowPERInteger(tag, indent, "g729wAnnexB", (uint32)x->g729wAnnexB);
            break;
        case 15:
            ShowPERInteger(tag, indent, "g729AnnexAwAnnexB", (uint32)x->g729AnnexAwAnnexB);
            break;
        case 16:
            Analyze_G7231AnnexCCapability(x->g7231AnnexCCapability, "g7231AnnexCCapability", tag, indent);
            break;
        case 17:
            Analyze_GSMAudioCapability(x->gsmFullRate, "gsmFullRate", tag, indent);
            break;
        case 18:
            Analyze_GSMAudioCapability(x->gsmHalfRate, "gsmHalfRate", tag, indent);
            break;
        case 19:
            Analyze_GSMAudioCapability(x->gsmEnhancedFullRate, "gsmEnhancedFullRate", tag, indent);
            break;
        case 20:
            Analyze_GenericCapability(x->genericAudioCapability, "genericAudioCapability", tag, indent);
            break;
        case 21:
            Analyze_G729Extensions(x->g729Extensions, "g729Extensions", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_AudioCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AudioCapability");
}

/* <=================================> */
/*  PER-Analyzer for G7231 (SEQUENCE)  */
/* <=================================> */
void Analyze_G7231(PS_G7231 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "G7231");
    indent += 2;
    ShowPERInteger(tag, indent, "maxAl_sduAudioFrames", (int32)x->maxAl_sduAudioFrames);
    ShowPERBoolean(tag, indent, "silenceSuppression", x->silenceSuppression);
    ShowPERClosure(tag, indent, "G7231");
}

/* <==========================================> */
/*  PER-Analyzer for G729Extensions (SEQUENCE)  */
/* <==========================================> */
void Analyze_G729Extensions(PS_G729Extensions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "G729Extensions");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_audioUnit", x->option_of_audioUnit);
    if (x->option_of_audioUnit)
    {
        ShowPERInteger(tag, indent, "audioUnit", (int32)x->audioUnit);
    }
    ShowPERBoolean(tag, indent, "annexA", x->annexA);
    ShowPERBoolean(tag, indent, "annexB", x->annexB);
    ShowPERBoolean(tag, indent, "annexD", x->annexD);
    ShowPERBoolean(tag, indent, "annexE", x->annexE);
    ShowPERBoolean(tag, indent, "annexF", x->annexF);
    ShowPERBoolean(tag, indent, "annexG", x->annexG);
    ShowPERBoolean(tag, indent, "annexH", x->annexH);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "G729Extensions");
}

/* <=================================================> */
/*  PER-Analyzer for G7231AnnexCCapability (SEQUENCE)  */
/* <=================================================> */
void Analyze_G7231AnnexCCapability(PS_G7231AnnexCCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "G7231AnnexCCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_g723AnnexCAudioMode", x->option_of_g723AnnexCAudioMode);
    ShowPERInteger(tag, indent, "maxAl_sduAudioFrames", (int32)x->maxAl_sduAudioFrames);
    ShowPERBoolean(tag, indent, "silenceSuppression", x->silenceSuppression);
    if (x->option_of_g723AnnexCAudioMode)
    {
        Analyze_G723AnnexCAudioMode(&x->g723AnnexCAudioMode, "g723AnnexCAudioMode", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "G7231AnnexCCapability");
}

/* <===============================================> */
/*  PER-Analyzer for G723AnnexCAudioMode (SEQUENCE)  */
/* <===============================================> */
void Analyze_G723AnnexCAudioMode(PS_G723AnnexCAudioMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "G723AnnexCAudioMode");
    indent += 2;
    ShowPERInteger(tag, indent, "highRateMode0", (int32)x->highRateMode0);
    ShowPERInteger(tag, indent, "highRateMode1", (int32)x->highRateMode1);
    ShowPERInteger(tag, indent, "lowRateMode0", (int32)x->lowRateMode0);
    ShowPERInteger(tag, indent, "lowRateMode1", (int32)x->lowRateMode1);
    ShowPERInteger(tag, indent, "sidMode0", (int32)x->sidMode0);
    ShowPERInteger(tag, indent, "sidMode1", (int32)x->sidMode1);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "G723AnnexCAudioMode");
}

/* <==================================================> */
/*  PER-Analyzer for IS11172AudioCapability (SEQUENCE)  */
/* <==================================================> */
void Analyze_IS11172AudioCapability(PS_IS11172AudioCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS11172AudioCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "audioLayer1", x->audioLayer1);
    ShowPERBoolean(tag, indent, "audioLayer2", x->audioLayer2);
    ShowPERBoolean(tag, indent, "audioLayer3", x->audioLayer3);
    ShowPERBoolean(tag, indent, "audioSampling32k", x->audioSampling32k);
    ShowPERBoolean(tag, indent, "audioSampling44k1", x->audioSampling44k1);
    ShowPERBoolean(tag, indent, "audioSampling48k", x->audioSampling48k);
    ShowPERBoolean(tag, indent, "singleChannel", x->singleChannel);
    ShowPERBoolean(tag, indent, "twoChannels", x->twoChannels);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS11172AudioCapability");
}

/* <==================================================> */
/*  PER-Analyzer for IS13818AudioCapability (SEQUENCE)  */
/* <==================================================> */
void Analyze_IS13818AudioCapability(PS_IS13818AudioCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS13818AudioCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "audioLayer1", x->audioLayer1);
    ShowPERBoolean(tag, indent, "audioLayer2", x->audioLayer2);
    ShowPERBoolean(tag, indent, "audioLayer3", x->audioLayer3);
    ShowPERBoolean(tag, indent, "audioSampling16k", x->audioSampling16k);
    ShowPERBoolean(tag, indent, "audioSampling22k05", x->audioSampling22k05);
    ShowPERBoolean(tag, indent, "audioSampling24k", x->audioSampling24k);
    ShowPERBoolean(tag, indent, "audioSampling32k", x->audioSampling32k);
    ShowPERBoolean(tag, indent, "audioSampling44k1", x->audioSampling44k1);
    ShowPERBoolean(tag, indent, "audioSampling48k", x->audioSampling48k);
    ShowPERBoolean(tag, indent, "singleChannel", x->singleChannel);
    ShowPERBoolean(tag, indent, "twoChannels", x->twoChannels);
    ShowPERBoolean(tag, indent, "threeChannels2_1", x->threeChannels2_1);
    ShowPERBoolean(tag, indent, "threeChannels3_0", x->threeChannels3_0);
    ShowPERBoolean(tag, indent, "fourChannels2_0_2_0", x->fourChannels2_0_2_0);
    ShowPERBoolean(tag, indent, "fourChannels2_2", x->fourChannels2_2);
    ShowPERBoolean(tag, indent, "fourChannels3_1", x->fourChannels3_1);
    ShowPERBoolean(tag, indent, "fiveChannels3_0_2_0", x->fiveChannels3_0_2_0);
    ShowPERBoolean(tag, indent, "fiveChannels3_2", x->fiveChannels3_2);
    ShowPERBoolean(tag, indent, "lowFrequencyEnhancement", x->lowFrequencyEnhancement);
    ShowPERBoolean(tag, indent, "multilingual", x->multilingual);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS13818AudioCapability");
}

/* <==============================================> */
/*  PER-Analyzer for GSMAudioCapability (SEQUENCE)  */
/* <==============================================> */
void Analyze_GSMAudioCapability(PS_GSMAudioCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "GSMAudioCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "audioUnitSize", (int32)x->audioUnitSize);
    ShowPERBoolean(tag, indent, "comfortNoise", x->comfortNoise);
    ShowPERBoolean(tag, indent, "scrambled", x->scrambled);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "GSMAudioCapability");
}

/* <=====================================================> */
/*  PER-Analyzer for DataApplicationCapability (SEQUENCE)  */
/* <=====================================================> */
void Analyze_DataApplicationCapability(PS_DataApplicationCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "DataApplicationCapability");
    indent += 2;
    Analyze_Application(&x->application, "application", tag, indent);
    ShowPERInteger(tag, indent, "maxBitRate", (int32)x->maxBitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "DataApplicationCapability");
}

/* <=====================================> */
/*  PER-Analyzer for Application (CHOICE)  */
/* <=====================================> */
void Analyze_Application(PS_Application x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Application");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_DataProtocolCapability(x->t120, "t120", tag, indent);
            break;
        case 2:
            Analyze_DataProtocolCapability(x->dsm_cc, "dsm_cc", tag, indent);
            break;
        case 3:
            Analyze_DataProtocolCapability(x->userData, "userData", tag, indent);
            break;
        case 4:
            Analyze_T84(x->t84, "t84", tag, indent);
            break;
        case 5:
            Analyze_DataProtocolCapability(x->t434, "t434", tag, indent);
            break;
        case 6:
            Analyze_DataProtocolCapability(x->h224, "h224", tag, indent);
            break;
        case 7:
            Analyze_Nlpid(x->nlpid, "nlpid", tag, indent);
            break;
        case 8:
            ShowPERNull(tag, indent, "dsvdControl");
            break;
        case 9:
            Analyze_DataProtocolCapability(x->h222DataPartitioning, "h222DataPartitioning", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Analyze_DataProtocolCapability(x->t30fax, "t30fax", tag, indent);
            break;
        case 11:
            Analyze_DataProtocolCapability(x->t140, "t140", tag, indent);
            break;
        case 12:
            Analyze_T38fax(x->t38fax, "t38fax", tag, indent);
            break;
        case 13:
            Analyze_GenericCapability(x->genericDataCapability, "genericDataCapability", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_Application: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Application");
}

/* <==================================> */
/*  PER-Analyzer for T38fax (SEQUENCE)  */
/* <==================================> */
void Analyze_T38fax(PS_T38fax x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "T38fax");
    indent += 2;
    Analyze_DataProtocolCapability(&x->t38FaxProtocol, "t38FaxProtocol", tag, indent);
    Analyze_T38FaxProfile(&x->t38FaxProfile, "t38FaxProfile", tag, indent);
    ShowPERClosure(tag, indent, "T38fax");
}

/* <=================================> */
/*  PER-Analyzer for Nlpid (SEQUENCE)  */
/* <=================================> */
void Analyze_Nlpid(PS_Nlpid x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Nlpid");
    indent += 2;
    Analyze_DataProtocolCapability(&x->nlpidProtocol, "nlpidProtocol", tag, indent);
    ShowPEROctetString(tag, indent, "nlpidData", &x->nlpidData);
    ShowPERClosure(tag, indent, "Nlpid");
}

/* <===============================> */
/*  PER-Analyzer for T84 (SEQUENCE)  */
/* <===============================> */
void Analyze_T84(PS_T84 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "T84");
    indent += 2;
    Analyze_DataProtocolCapability(&x->t84Protocol, "t84Protocol", tag, indent);
    Analyze_T84Profile(&x->t84Profile, "t84Profile", tag, indent);
    ShowPERClosure(tag, indent, "T84");
}

/* <================================================> */
/*  PER-Analyzer for DataProtocolCapability (CHOICE)  */
/* <================================================> */
void Analyze_DataProtocolCapability(PS_DataProtocolCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "DataProtocolCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "v14buffered");
            break;
        case 2:
            ShowPERNull(tag, indent, "v42lapm");
            break;
        case 3:
            ShowPERNull(tag, indent, "hdlcFrameTunnelling");
            break;
        case 4:
            ShowPERNull(tag, indent, "h310SeparateVCStack");
            break;
        case 5:
            ShowPERNull(tag, indent, "h310SingleVCStack");
            break;
        case 6:
            ShowPERNull(tag, indent, "transparent");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            ShowPERNull(tag, indent, "segmentationAndReassembly");
            break;
        case 8:
            ShowPERNull(tag, indent, "hdlcFrameTunnelingwSAR");
            break;
        case 9:
            ShowPERNull(tag, indent, "v120");
            break;
        case 10:
            ShowPERNull(tag, indent, "separateLANStack");
            break;
        case 11:
            Analyze_V76wCompression(x->v76wCompression, "v76wCompression", tag, indent);
            break;
        case 12:
            ShowPERNull(tag, indent, "tcp");
            break;
        case 13:
            ShowPERNull(tag, indent, "udp");
            break;
        default:
            ErrorMessage("Analyze_DataProtocolCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "DataProtocolCapability");
}

/* <=========================================> */
/*  PER-Analyzer for V76wCompression (CHOICE)  */
/* <=========================================> */
void Analyze_V76wCompression(PS_V76wCompression x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "V76wCompression");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_CompressionType(x->transmitCompression, "transmitCompression", tag, indent);
            break;
        case 1:
            Analyze_CompressionType(x->receiveCompression, "receiveCompression", tag, indent);
            break;
        case 2:
            Analyze_CompressionType(x->transmitAndReceiveCompression, "transmitAndReceiveCompression", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_V76wCompression: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "V76wCompression");
}

/* <=========================================> */
/*  PER-Analyzer for CompressionType (CHOICE)  */
/* <=========================================> */
void Analyze_CompressionType(PS_CompressionType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CompressionType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_V42bis(x->v42bis, "v42bis", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CompressionType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CompressionType");
}

/* <==================================> */
/*  PER-Analyzer for V42bis (SEQUENCE)  */
/* <==================================> */
void Analyze_V42bis(PS_V42bis x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V42bis");
    indent += 2;
    ShowPERInteger(tag, indent, "numberOfCodewords", (int32)x->numberOfCodewords);
    ShowPERInteger(tag, indent, "maximumStringLength", (int32)x->maximumStringLength);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V42bis");
}

/* <====================================> */
/*  PER-Analyzer for T84Profile (CHOICE)  */
/* <====================================> */
void Analyze_T84Profile(PS_T84Profile x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "T84Profile");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "t84Unrestricted");
            break;
        case 1:
            Analyze_T84Restricted(x->t84Restricted, "t84Restricted", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_T84Profile: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "T84Profile");
}

/* <=========================================> */
/*  PER-Analyzer for T84Restricted (SEQUENCE)  */
/* <=========================================> */
void Analyze_T84Restricted(PS_T84Restricted x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "T84Restricted");
    indent += 2;
    ShowPERBoolean(tag, indent, "qcif", x->qcif);
    ShowPERBoolean(tag, indent, "cif", x->cif);
    ShowPERBoolean(tag, indent, "ccir601Seq", x->ccir601Seq);
    ShowPERBoolean(tag, indent, "ccir601Prog", x->ccir601Prog);
    ShowPERBoolean(tag, indent, "hdtvSeq", x->hdtvSeq);
    ShowPERBoolean(tag, indent, "hdtvProg", x->hdtvProg);
    ShowPERBoolean(tag, indent, "g3FacsMH200x100", x->g3FacsMH200x100);
    ShowPERBoolean(tag, indent, "g3FacsMH200x200", x->g3FacsMH200x200);
    ShowPERBoolean(tag, indent, "g4FacsMMR200x100", x->g4FacsMMR200x100);
    ShowPERBoolean(tag, indent, "g4FacsMMR200x200", x->g4FacsMMR200x200);
    ShowPERBoolean(tag, indent, "jbig200x200Seq", x->jbig200x200Seq);
    ShowPERBoolean(tag, indent, "jbig200x200Prog", x->jbig200x200Prog);
    ShowPERBoolean(tag, indent, "jbig300x300Seq", x->jbig300x300Seq);
    ShowPERBoolean(tag, indent, "jbig300x300Prog", x->jbig300x300Prog);
    ShowPERBoolean(tag, indent, "digPhotoLow", x->digPhotoLow);
    ShowPERBoolean(tag, indent, "digPhotoMedSeq", x->digPhotoMedSeq);
    ShowPERBoolean(tag, indent, "digPhotoMedProg", x->digPhotoMedProg);
    ShowPERBoolean(tag, indent, "digPhotoHighSeq", x->digPhotoHighSeq);
    ShowPERBoolean(tag, indent, "digPhotoHighProg", x->digPhotoHighProg);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "T84Restricted");
}

/* <=========================================> */
/*  PER-Analyzer for T38FaxProfile (SEQUENCE)  */
/* <=========================================> */
void Analyze_T38FaxProfile(PS_T38FaxProfile x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "T38FaxProfile");
    indent += 2;
    ShowPERBoolean(tag, indent, "fillBitRemoval", x->fillBitRemoval);
    ShowPERBoolean(tag, indent, "transcodingJBIG", x->transcodingJBIG);
    ShowPERBoolean(tag, indent, "transcodingMMR", x->transcodingMMR);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_version", x->option_of_version);
    ShowPERBoolean(tag, indent, "option_of_t38FaxRateManagement", x->option_of_t38FaxRateManagement);
    ShowPERBoolean(tag, indent, "option_of_t38FaxUdpOptions", x->option_of_t38FaxUdpOptions);
    if (x->option_of_version)
    {
        ShowPERInteger(tag, indent, "version", (int32)x->version);
    }
    if (x->option_of_t38FaxRateManagement)
    {
        Analyze_T38FaxRateManagement(&x->t38FaxRateManagement, "t38FaxRateManagement", tag, indent);
    }
    if (x->option_of_t38FaxUdpOptions)
    {
        Analyze_T38FaxUdpOptions(&x->t38FaxUdpOptions, "t38FaxUdpOptions", tag, indent);
    }
    ShowPERClosure(tag, indent, "T38FaxProfile");
}

/* <==============================================> */
/*  PER-Analyzer for T38FaxRateManagement (CHOICE)  */
/* <==============================================> */
void Analyze_T38FaxRateManagement(PS_T38FaxRateManagement x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "T38FaxRateManagement");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "localTCF");
            break;
        case 1:
            ShowPERNull(tag, indent, "transferredTCF");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_T38FaxRateManagement: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "T38FaxRateManagement");
}

/* <============================================> */
/*  PER-Analyzer for T38FaxUdpOptions (SEQUENCE)  */
/* <============================================> */
void Analyze_T38FaxUdpOptions(PS_T38FaxUdpOptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "T38FaxUdpOptions");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_t38FaxMaxBuffer", x->option_of_t38FaxMaxBuffer);
    ShowPERBoolean(tag, indent, "option_of_t38FaxMaxDatagram", x->option_of_t38FaxMaxDatagram);
    if (x->option_of_t38FaxMaxBuffer)
    {
        ShowPERUnboundedInteger(tag, indent, "t38FaxMaxBuffer", (uint32)x->t38FaxMaxBuffer);
    }
    if (x->option_of_t38FaxMaxDatagram)
    {
        ShowPERUnboundedInteger(tag, indent, "t38FaxMaxDatagram", (uint32)x->t38FaxMaxDatagram);
    }
    Analyze_T38FaxUdpEC(&x->t38FaxUdpEC, "t38FaxUdpEC", tag, indent);
    ShowPERClosure(tag, indent, "T38FaxUdpOptions");
}

/* <=====================================> */
/*  PER-Analyzer for T38FaxUdpEC (CHOICE)  */
/* <=====================================> */
void Analyze_T38FaxUdpEC(PS_T38FaxUdpEC x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "T38FaxUdpEC");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "t38UDPFEC");
            break;
        case 1:
            ShowPERNull(tag, indent, "t38UDPRedundancy");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_T38FaxUdpEC: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "T38FaxUdpEC");
}

/* <================================================================> */
/*  PER-Analyzer for EncryptionAuthenticationAndIntegrity (SEQUENCE)  */
/* <================================================================> */
void Analyze_EncryptionAuthenticationAndIntegrity(PS_EncryptionAuthenticationAndIntegrity x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "EncryptionAuthenticationAndIntegrity");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_encryptionCapability", x->option_of_encryptionCapability);
    ShowPERBoolean(tag, indent, "option_of_authenticationCapability", x->option_of_authenticationCapability);
    ShowPERBoolean(tag, indent, "option_of_integrityCapability", x->option_of_integrityCapability);
    if (x->option_of_encryptionCapability)
    {
        Analyze_EncryptionCapability(&x->encryptionCapability, "encryptionCapability", tag, indent);
    }
    if (x->option_of_authenticationCapability)
    {
        Analyze_AuthenticationCapability(&x->authenticationCapability, "authenticationCapability", tag, indent);
    }
    if (x->option_of_integrityCapability)
    {
        Analyze_IntegrityCapability(&x->integrityCapability, "integrityCapability", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EncryptionAuthenticationAndIntegrity");
}

/* <===================================================> */
/*  PER-Analyzer for EncryptionCapability (SEQUENCE-OF)  */
/* <===================================================> */
void Analyze_EncryptionCapability(PS_EncryptionCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequenceof(tag, indent, label, "EncryptionCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "size", (uint32)x->size);
    for (i = 0;i < x->size;++i)
    {
        sprintf(tempLabelString, "item[%hu]", i);
        Analyze_MediaEncryptionAlgorithm(x->item + i, tempLabelString, tag, indent);
    }
    ShowPERClosure(tag, indent, "EncryptionCapability");
}

/* <==================================================> */
/*  PER-Analyzer for MediaEncryptionAlgorithm (CHOICE)  */
/* <==================================================> */
void Analyze_MediaEncryptionAlgorithm(PS_MediaEncryptionAlgorithm x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MediaEncryptionAlgorithm");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERObjectID(tag, indent, "algorithm", x->algorithm);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MediaEncryptionAlgorithm: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MediaEncryptionAlgorithm");
}

/* <====================================================> */
/*  PER-Analyzer for AuthenticationCapability (SEQUENCE)  */
/* <====================================================> */
void Analyze_AuthenticationCapability(PS_AuthenticationCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "AuthenticationCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    if (x->option_of_nonStandard)
    {
        Analyze_NonStandardParameter(&x->nonStandard, "nonStandard", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "AuthenticationCapability");
}

/* <===============================================> */
/*  PER-Analyzer for IntegrityCapability (SEQUENCE)  */
/* <===============================================> */
void Analyze_IntegrityCapability(PS_IntegrityCapability x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IntegrityCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    if (x->option_of_nonStandard)
    {
        Analyze_NonStandardParameter(&x->nonStandard, "nonStandard", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IntegrityCapability");
}

/* <=============================================> */
/*  PER-Analyzer for UserInputCapability (CHOICE)  */
/* <=============================================> */
void Analyze_UserInputCapability(PS_UserInputCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "UserInputCapability");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "nonStandard[%hu]", i);
                Analyze_NonStandardParameter(x->nonStandard + i, tempLabelString, tag, indent);
            }
            break;
        case 1:
            ShowPERNull(tag, indent, "basicString");
            break;
        case 2:
            ShowPERNull(tag, indent, "iA5String");
            break;
        case 3:
            ShowPERNull(tag, indent, "generalString");
            break;
        case 4:
            ShowPERNull(tag, indent, "dtmf");
            break;
        case 5:
            ShowPERNull(tag, indent, "hookflash");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_UserInputCapability: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "UserInputCapability");
}

/* <================================================> */
/*  PER-Analyzer for ConferenceCapability (SEQUENCE)  */
/* <================================================> */
void Analyze_ConferenceCapability(PS_ConferenceCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "ConferenceCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandardData", x->option_of_nonStandardData);
    if (x->option_of_nonStandardData)
    {
        ShowPERInteger(tag, indent, "size_of_nonStandardData", (uint32)x->size_of_nonStandardData);
        for (i = 0;i < x->size_of_nonStandardData;++i)
        {
            sprintf(tempLabelString, "nonStandardData[%hu]", i);
            Analyze_NonStandardParameter(x->nonStandardData + i, tempLabelString, tag, indent);
        }
    }
    ShowPERBoolean(tag, indent, "chairControlCapability", x->chairControlCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_VideoIndicateMixingCapability", x->option_of_VideoIndicateMixingCapability);
    ShowPERBoolean(tag, indent, "option_of_multipointVisualizationCapability", x->option_of_multipointVisualizationCapability);
    if (x->option_of_VideoIndicateMixingCapability)
    {
        ShowPERBoolean(tag, indent, "VideoIndicateMixingCapability", x->VideoIndicateMixingCapability);
    }
    if (x->option_of_multipointVisualizationCapability)
    {
        ShowPERBoolean(tag, indent, "multipointVisualizationCapability", x->multipointVisualizationCapability);
    }
    ShowPERClosure(tag, indent, "ConferenceCapability");
}

/* <=============================================> */
/*  PER-Analyzer for GenericCapability (SEQUENCE)  */
/* <=============================================> */
void Analyze_GenericCapability(PS_GenericCapability x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "GenericCapability");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_maxBitRate", x->option_of_maxBitRate);
    ShowPERBoolean(tag, indent, "option_of_collapsing", x->option_of_collapsing);
    ShowPERBoolean(tag, indent, "option_of_nonCollapsing", x->option_of_nonCollapsing);
    ShowPERBoolean(tag, indent, "option_of_nonCollapsingRaw", x->option_of_nonCollapsingRaw);
    ShowPERBoolean(tag, indent, "option_of_transport", x->option_of_transport);
    Analyze_CapabilityIdentifier(&x->capabilityIdentifier, "capabilityIdentifier", tag, indent);
    if (x->option_of_maxBitRate)
    {
        ShowPERInteger(tag, indent, "maxBitRate", (int32)x->maxBitRate);
    }
    if (x->option_of_collapsing)
    {
        ShowPERInteger(tag, indent, "size_of_collapsing", (uint32)x->size_of_collapsing);
        for (i = 0;i < x->size_of_collapsing;++i)
        {
            sprintf(tempLabelString, "collapsing[%hu]", i);
            Analyze_GenericParameter(x->collapsing + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_nonCollapsing)
    {
        ShowPERInteger(tag, indent, "size_of_nonCollapsing", (uint32)x->size_of_nonCollapsing);
        for (i = 0;i < x->size_of_nonCollapsing;++i)
        {
            sprintf(tempLabelString, "nonCollapsing[%hu]", i);
            Analyze_GenericParameter(x->nonCollapsing + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_nonCollapsingRaw)
    {
        ShowPEROctetString(tag, indent, "nonCollapsingRaw", &x->nonCollapsingRaw);
    }
    if (x->option_of_transport)
    {
        Analyze_DataProtocolCapability(&x->transport, "transport", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "GenericCapability");
}

/* <==============================================> */
/*  PER-Analyzer for CapabilityIdentifier (CHOICE)  */
/* <==============================================> */
void Analyze_CapabilityIdentifier(PS_CapabilityIdentifier x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CapabilityIdentifier");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERObjectID(tag, indent, "standard", x->standard);
            break;
        case 1:
            Analyze_NonStandardParameter(x->h221NonStandard, "h221NonStandard", tag, indent);
            break;
        case 2:
            ShowPEROctetString(tag, indent, "uuid", x->uuid);
            break;
        case 3:
            ShowPERCharString(tag, indent, "domainBased", x->domainBased);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CapabilityIdentifier: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CapabilityIdentifier");
}

/* <============================================> */
/*  PER-Analyzer for GenericParameter (SEQUENCE)  */
/* <============================================> */
void Analyze_GenericParameter(PS_GenericParameter x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "GenericParameter");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_supersedes", x->option_of_supersedes);
    Analyze_ParameterIdentifier(&x->parameterIdentifier, "parameterIdentifier", tag, indent);
    Analyze_ParameterValue(&x->parameterValue, "parameterValue", tag, indent);
    if (x->option_of_supersedes)
    {
        ShowPERInteger(tag, indent, "size_of_supersedes", (uint32)x->size_of_supersedes);
        for (i = 0;i < x->size_of_supersedes;++i)
        {
            sprintf(tempLabelString, "supersedes[%hu]", i);
            Analyze_ParameterIdentifier(x->supersedes + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "GenericParameter");
}

/* <=============================================> */
/*  PER-Analyzer for ParameterIdentifier (CHOICE)  */
/* <=============================================> */
void Analyze_ParameterIdentifier(PS_ParameterIdentifier x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ParameterIdentifier");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "standard", (uint32)x->standard);
            break;
        case 1:
            Analyze_NonStandardParameter(x->h221NonStandard, "h221NonStandard", tag, indent);
            break;
        case 2:
            ShowPEROctetString(tag, indent, "uuid", x->uuid);
            break;
        case 3:
            ShowPERCharString(tag, indent, "domainBased", x->domainBased);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ParameterIdentifier: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ParameterIdentifier");
}

/* <========================================> */
/*  PER-Analyzer for ParameterValue (CHOICE)  */
/* <========================================> */
void Analyze_ParameterValue(PS_ParameterValue x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "ParameterValue");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "logical");
            break;
        case 1:
            ShowPERInteger(tag, indent, "booleanArray", (uint32)x->booleanArray);
            break;
        case 2:
            ShowPERInteger(tag, indent, "unsignedMin", (uint32)x->unsignedMin);
            break;
        case 3:
            ShowPERInteger(tag, indent, "unsignedMax", (uint32)x->unsignedMax);
            break;
        case 4:
            ShowPERInteger(tag, indent, "unsigned32Min", (uint32)x->unsigned32Min);
            break;
        case 5:
            ShowPERInteger(tag, indent, "unsigned32Max", (uint32)x->unsigned32Max);
            break;
        case 6:
            ShowPEROctetString(tag, indent, "octetString", x->octetString);
            break;
        case 7:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "genericParameter[%hu]", i);
                Analyze_GenericParameter(x->genericParameter + i, tempLabelString, tag, indent);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ParameterValue: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ParameterValue");
}

/* <==============================================> */
/*  PER-Analyzer for OpenLogicalChannel (SEQUENCE)  */
/* <==============================================> */
void Analyze_OpenLogicalChannel(PS_OpenLogicalChannel x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "OpenLogicalChannel");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_reverseLogicalChannelParameters", x->option_of_reverseLogicalChannelParameters);
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    Analyze_ForwardLogicalChannelParameters(&x->forwardLogicalChannelParameters, "forwardLogicalChannelParameters", tag, indent);
    if (x->option_of_reverseLogicalChannelParameters)
    {
        Analyze_ReverseLogicalChannelParameters(&x->reverseLogicalChannelParameters, "reverseLogicalChannelParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_separateStack", x->option_of_separateStack);
    ShowPERBoolean(tag, indent, "option_of_encryptionSync", x->option_of_encryptionSync);
    if (x->option_of_separateStack)
    {
        Analyze_NetworkAccessParameters(&x->separateStack, "separateStack", tag, indent);
    }
    if (x->option_of_encryptionSync)
    {
        Analyze_EncryptionSync(&x->encryptionSync, "encryptionSync", tag, indent);
    }
    ShowPERClosure(tag, indent, "OpenLogicalChannel");
}

/* <===========================================================> */
/*  PER-Analyzer for ReverseLogicalChannelParameters (SEQUENCE)  */
/* <===========================================================> */
void Analyze_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ReverseLogicalChannelParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_rlcMultiplexParameters", x->option_of_rlcMultiplexParameters);
    Analyze_DataType(&x->dataType, "dataType", tag, indent);
    if (x->option_of_rlcMultiplexParameters)
    {
        Analyze_RlcMultiplexParameters(&x->rlcMultiplexParameters, "rlcMultiplexParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_reverseLogicalChannelDependency", x->option_of_reverseLogicalChannelDependency);
    ShowPERBoolean(tag, indent, "option_of_replacementFor", x->option_of_replacementFor);
    if (x->option_of_reverseLogicalChannelDependency)
    {
        ShowPERInteger(tag, indent, "reverseLogicalChannelDependency", (int32)x->reverseLogicalChannelDependency);
    }
    if (x->option_of_replacementFor)
    {
        ShowPERInteger(tag, indent, "replacementFor", (int32)x->replacementFor);
    }
    ShowPERClosure(tag, indent, "ReverseLogicalChannelParameters");
}

/* <================================================> */
/*  PER-Analyzer for RlcMultiplexParameters (CHOICE)  */
/* <================================================> */
void Analyze_RlcMultiplexParameters(PS_RlcMultiplexParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RlcMultiplexParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_H223LogicalChannelParameters(x->h223LogicalChannelParameters, "h223LogicalChannelParameters", tag, indent);
            break;
        case 1:
            Analyze_V76LogicalChannelParameters(x->v76LogicalChannelParameters, "v76LogicalChannelParameters", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Analyze_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, "h2250LogicalChannelParameters", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_RlcMultiplexParameters: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RlcMultiplexParameters");
}

/* <===========================================================> */
/*  PER-Analyzer for ForwardLogicalChannelParameters (SEQUENCE)  */
/* <===========================================================> */
void Analyze_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ForwardLogicalChannelParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_portNumber", x->option_of_portNumber);
    if (x->option_of_portNumber)
    {
        ShowPERInteger(tag, indent, "portNumber", (int32)x->portNumber);
    }
    Analyze_DataType(&x->dataType, "dataType", tag, indent);
    Analyze_MultiplexParameters(&x->multiplexParameters, "multiplexParameters", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_forwardLogicalChannelDependency", x->option_of_forwardLogicalChannelDependency);
    ShowPERBoolean(tag, indent, "option_of_replacementFor", x->option_of_replacementFor);
    if (x->option_of_forwardLogicalChannelDependency)
    {
        ShowPERInteger(tag, indent, "forwardLogicalChannelDependency", (int32)x->forwardLogicalChannelDependency);
    }
    if (x->option_of_replacementFor)
    {
        ShowPERInteger(tag, indent, "replacementFor", (int32)x->replacementFor);
    }
    ShowPERClosure(tag, indent, "ForwardLogicalChannelParameters");
}

/* <=============================================> */
/*  PER-Analyzer for MultiplexParameters (CHOICE)  */
/* <=============================================> */
void Analyze_MultiplexParameters(PS_MultiplexParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultiplexParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_H222LogicalChannelParameters(x->h222LogicalChannelParameters, "h222LogicalChannelParameters", tag, indent);
            break;
        case 1:
            Analyze_H223LogicalChannelParameters(x->h223LogicalChannelParameters, "h223LogicalChannelParameters", tag, indent);
            break;
        case 2:
            Analyze_V76LogicalChannelParameters(x->v76LogicalChannelParameters, "v76LogicalChannelParameters", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            Analyze_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, "h2250LogicalChannelParameters", tag, indent);
            break;
        case 4:
            ShowPERNull(tag, indent, "none");
            break;
        default:
            ErrorMessage("Analyze_MultiplexParameters: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultiplexParameters");
}

/* <===================================================> */
/*  PER-Analyzer for NetworkAccessParameters (SEQUENCE)  */
/* <===================================================> */
void Analyze_NetworkAccessParameters(PS_NetworkAccessParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "NetworkAccessParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_distribution", x->option_of_distribution);
    ShowPERBoolean(tag, indent, "option_of_externalReference", x->option_of_externalReference);
    if (x->option_of_distribution)
    {
        Analyze_Distribution(&x->distribution, "distribution", tag, indent);
    }
    Analyze_NetworkAddress(&x->networkAddress, "networkAddress", tag, indent);
    ShowPERBoolean(tag, indent, "associateConference", x->associateConference);
    if (x->option_of_externalReference)
    {
        ShowPEROctetString(tag, indent, "externalReference", &x->externalReference);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_t120SetupProcedure", x->option_of_t120SetupProcedure);
    if (x->option_of_t120SetupProcedure)
    {
        Analyze_T120SetupProcedure(&x->t120SetupProcedure, "t120SetupProcedure", tag, indent);
    }
    ShowPERClosure(tag, indent, "NetworkAccessParameters");
}

/* <============================================> */
/*  PER-Analyzer for T120SetupProcedure (CHOICE)  */
/* <============================================> */
void Analyze_T120SetupProcedure(PS_T120SetupProcedure x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "T120SetupProcedure");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "originateCall");
            break;
        case 1:
            ShowPERNull(tag, indent, "waitForCall");
            break;
        case 2:
            ShowPERNull(tag, indent, "issueQuery");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_T120SetupProcedure: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "T120SetupProcedure");
}

/* <========================================> */
/*  PER-Analyzer for NetworkAddress (CHOICE)  */
/* <========================================> */
void Analyze_NetworkAddress(PS_NetworkAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "NetworkAddress");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_Q2931Address(x->q2931Address, "q2931Address", tag, indent);
            break;
        case 1:
            ShowPERCharString(tag, indent, "e164Address", x->e164Address);
            break;
        case 2:
            Analyze_TransportAddress(x->localAreaAddress, "localAreaAddress", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_NetworkAddress: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "NetworkAddress");
}

/* <======================================> */
/*  PER-Analyzer for Distribution (CHOICE)  */
/* <======================================> */
void Analyze_Distribution(PS_Distribution x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Distribution");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unicast");
            break;
        case 1:
            ShowPERNull(tag, indent, "multicast");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Distribution: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Distribution");
}

/* <========================================> */
/*  PER-Analyzer for Q2931Address (SEQUENCE)  */
/* <========================================> */
void Analyze_Q2931Address(PS_Q2931Address x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Q2931Address");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_subaddress", x->option_of_subaddress);
    Analyze_Address(&x->address, "address", tag, indent);
    if (x->option_of_subaddress)
    {
        ShowPEROctetString(tag, indent, "subaddress", &x->subaddress);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Q2931Address");
}

/* <=================================> */
/*  PER-Analyzer for Address (CHOICE)  */
/* <=================================> */
void Analyze_Address(PS_Address x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Address");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERCharString(tag, indent, "internationalNumber", x->internationalNumber);
            break;
        case 1:
            ShowPEROctetString(tag, indent, "nsapAddress", x->nsapAddress);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Address: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Address");
}

/* <=========================================> */
/*  PER-Analyzer for V75Parameters (SEQUENCE)  */
/* <=========================================> */
void Analyze_V75Parameters(PS_V75Parameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V75Parameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "audioHeaderPresent", x->audioHeaderPresent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V75Parameters");
}

/* <==================================> */
/*  PER-Analyzer for DataType (CHOICE)  */
/* <==================================> */
void Analyze_DataType(PS_DataType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "DataType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "nullData");
            break;
        case 2:
            Analyze_VideoCapability(x->videoData, "videoData", tag, indent);
            break;
        case 3:
            Analyze_AudioCapability(x->audioData, "audioData", tag, indent);
            break;
        case 4:
            Analyze_DataApplicationCapability(x->data, "data", tag, indent);
            break;
        case 5:
            Analyze_EncryptionMode(x->encryptionData, "encryptionData", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Analyze_NonStandardParameter(x->h235Control, "h235Control", tag, indent);
            break;
        case 7:
            Analyze_H235Media(x->h235Media, "h235Media", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_DataType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "DataType");
}

/* <=====================================> */
/*  PER-Analyzer for H235Media (SEQUENCE)  */
/* <=====================================> */
void Analyze_H235Media(PS_H235Media x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H235Media");
    indent += 2;
    Analyze_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, "encryptionAuthenticationAndIntegrity", tag, indent);
    Analyze_MediaType(&x->mediaType, "mediaType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H235Media");
}

/* <===================================> */
/*  PER-Analyzer for MediaType (CHOICE)  */
/* <===================================> */
void Analyze_MediaType(PS_MediaType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MediaType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_VideoCapability(x->videoData, "videoData", tag, indent);
            break;
        case 2:
            Analyze_AudioCapability(x->audioData, "audioData", tag, indent);
            break;
        case 3:
            Analyze_DataApplicationCapability(x->data, "data", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MediaType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MediaType");
}

/* <========================================================> */
/*  PER-Analyzer for H222LogicalChannelParameters (SEQUENCE)  */
/* <========================================================> */
void Analyze_H222LogicalChannelParameters(PS_H222LogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H222LogicalChannelParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_pcr_pid", x->option_of_pcr_pid);
    ShowPERBoolean(tag, indent, "option_of_programDescriptors", x->option_of_programDescriptors);
    ShowPERBoolean(tag, indent, "option_of_streamDescriptors", x->option_of_streamDescriptors);
    ShowPERInteger(tag, indent, "resourceID", (int32)x->resourceID);
    ShowPERInteger(tag, indent, "subChannelID", (int32)x->subChannelID);
    if (x->option_of_pcr_pid)
    {
        ShowPERInteger(tag, indent, "pcr_pid", (int32)x->pcr_pid);
    }
    if (x->option_of_programDescriptors)
    {
        ShowPEROctetString(tag, indent, "programDescriptors", &x->programDescriptors);
    }
    if (x->option_of_streamDescriptors)
    {
        ShowPEROctetString(tag, indent, "streamDescriptors", &x->streamDescriptors);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H222LogicalChannelParameters");
}

/* <========================================================> */
/*  PER-Analyzer for H223LogicalChannelParameters (SEQUENCE)  */
/* <========================================================> */
void Analyze_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223LogicalChannelParameters");
    indent += 2;
    Analyze_AdaptationLayerType(&x->adaptationLayerType, "adaptationLayerType", tag, indent);
    ShowPERBoolean(tag, indent, "segmentableFlag", x->segmentableFlag);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H223LogicalChannelParameters");
}

/* <=============================================> */
/*  PER-Analyzer for AdaptationLayerType (CHOICE)  */
/* <=============================================> */
void Analyze_AdaptationLayerType(PS_AdaptationLayerType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AdaptationLayerType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "al1Framed");
            break;
        case 2:
            ShowPERNull(tag, indent, "al1NotFramed");
            break;
        case 3:
            ShowPERNull(tag, indent, "al2WithoutSequenceNumbers");
            break;
        case 4:
            ShowPERNull(tag, indent, "al2WithSequenceNumbers");
            break;
        case 5:
            Analyze_Al3(x->al3, "al3", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Analyze_H223AL1MParameters(x->al1M, "al1M", tag, indent);
            break;
        case 7:
            Analyze_H223AL2MParameters(x->al2M, "al2M", tag, indent);
            break;
        case 8:
            Analyze_H223AL3MParameters(x->al3M, "al3M", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_AdaptationLayerType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AdaptationLayerType");
}

/* <===============================> */
/*  PER-Analyzer for Al3 (SEQUENCE)  */
/* <===============================> */
void Analyze_Al3(PS_Al3 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Al3");
    indent += 2;
    ShowPERInteger(tag, indent, "controlFieldOctets", (int32)x->controlFieldOctets);
    ShowPERInteger(tag, indent, "sendBufferSize", (int32)x->sendBufferSize);
    ShowPERClosure(tag, indent, "Al3");
}

/* <==============================================> */
/*  PER-Analyzer for H223AL1MParameters (SEQUENCE)  */
/* <==============================================> */
void Analyze_H223AL1MParameters(PS_H223AL1MParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223AL1MParameters");
    indent += 2;
    Analyze_TransferMode(&x->transferMode, "transferMode", tag, indent);
    Analyze_HeaderFEC(&x->headerFEC, "headerFEC", tag, indent);
    Analyze_CrcLength(&x->crcLength, "crcLength", tag, indent);
    ShowPERInteger(tag, indent, "rcpcCodeRate", (int32)x->rcpcCodeRate);
    Analyze_ArqType(&x->arqType, "arqType", tag, indent);
    ShowPERBoolean(tag, indent, "alpduInterleaving", x->alpduInterleaving);
    ShowPERBoolean(tag, indent, "alsduSplitting", x->alsduSplitting);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_rsCodeCorrection", x->option_of_rsCodeCorrection);
    if (x->option_of_rsCodeCorrection)
    {
        ShowPERInteger(tag, indent, "rsCodeCorrection", (int32)x->rsCodeCorrection);
    }
    ShowPERClosure(tag, indent, "H223AL1MParameters");
}

/* <=================================> */
/*  PER-Analyzer for ArqType (CHOICE)  */
/* <=================================> */
void Analyze_ArqType(PS_ArqType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ArqType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noArq");
            break;
        case 1:
            Analyze_H223AnnexCArqParameters(x->typeIArq, "typeIArq", tag, indent);
            break;
        case 2:
            Analyze_H223AnnexCArqParameters(x->typeIIArq, "typeIIArq", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ArqType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ArqType");
}

/* <===================================> */
/*  PER-Analyzer for CrcLength (CHOICE)  */
/* <===================================> */
void Analyze_CrcLength(PS_CrcLength x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CrcLength");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "crc4bit");
            break;
        case 1:
            ShowPERNull(tag, indent, "crc12bit");
            break;
        case 2:
            ShowPERNull(tag, indent, "crc20bit");
            break;
        case 3:
            ShowPERNull(tag, indent, "crc28bit");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            ShowPERNull(tag, indent, "crc8bit");
            break;
        case 5:
            ShowPERNull(tag, indent, "crc16bit");
            break;
        case 6:
            ShowPERNull(tag, indent, "crc32bit");
            break;
        case 7:
            ShowPERNull(tag, indent, "crcNotUsed");
            break;
        default:
            ErrorMessage("Analyze_CrcLength: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CrcLength");
}

/* <===================================> */
/*  PER-Analyzer for HeaderFEC (CHOICE)  */
/* <===================================> */
void Analyze_HeaderFEC(PS_HeaderFEC x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "HeaderFEC");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "sebch16_7");
            break;
        case 1:
            ShowPERNull(tag, indent, "golay24_12");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_HeaderFEC: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "HeaderFEC");
}

/* <======================================> */
/*  PER-Analyzer for TransferMode (CHOICE)  */
/* <======================================> */
void Analyze_TransferMode(PS_TransferMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "TransferMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "framed");
            break;
        case 1:
            ShowPERNull(tag, indent, "unframed");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_TransferMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "TransferMode");
}

/* <==============================================> */
/*  PER-Analyzer for H223AL2MParameters (SEQUENCE)  */
/* <==============================================> */
void Analyze_H223AL2MParameters(PS_H223AL2MParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223AL2MParameters");
    indent += 2;
    Analyze_Al2HeaderFEC(&x->al2HeaderFEC, "al2HeaderFEC", tag, indent);
    ShowPERBoolean(tag, indent, "alpduInterleaving", x->alpduInterleaving);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H223AL2MParameters");
}

/* <======================================> */
/*  PER-Analyzer for Al2HeaderFEC (CHOICE)  */
/* <======================================> */
void Analyze_Al2HeaderFEC(PS_Al2HeaderFEC x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Al2HeaderFEC");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "sebch16_5");
            break;
        case 1:
            ShowPERNull(tag, indent, "golay24_12");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Al2HeaderFEC: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Al2HeaderFEC");
}

/* <==============================================> */
/*  PER-Analyzer for H223AL3MParameters (SEQUENCE)  */
/* <==============================================> */
void Analyze_H223AL3MParameters(PS_H223AL3MParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223AL3MParameters");
    indent += 2;
    Analyze_HeaderFormat(&x->headerFormat, "headerFormat", tag, indent);
    Analyze_Al3CrcLength(&x->al3CrcLength, "al3CrcLength", tag, indent);
    ShowPERInteger(tag, indent, "rcpcCodeRate", (int32)x->rcpcCodeRate);
    Analyze_Al3ArqType(&x->al3ArqType, "al3ArqType", tag, indent);
    ShowPERBoolean(tag, indent, "alpduInterleaving", x->alpduInterleaving);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_rsCodeCorrection", x->option_of_rsCodeCorrection);
    if (x->option_of_rsCodeCorrection)
    {
        ShowPERInteger(tag, indent, "rsCodeCorrection", (int32)x->rsCodeCorrection);
    }
    ShowPERClosure(tag, indent, "H223AL3MParameters");
}

/* <====================================> */
/*  PER-Analyzer for Al3ArqType (CHOICE)  */
/* <====================================> */
void Analyze_Al3ArqType(PS_Al3ArqType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Al3ArqType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noArq");
            break;
        case 1:
            Analyze_H223AnnexCArqParameters(x->typeIArq, "typeIArq", tag, indent);
            break;
        case 2:
            Analyze_H223AnnexCArqParameters(x->typeIIArq, "typeIIArq", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Al3ArqType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Al3ArqType");
}

/* <======================================> */
/*  PER-Analyzer for Al3CrcLength (CHOICE)  */
/* <======================================> */
void Analyze_Al3CrcLength(PS_Al3CrcLength x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Al3CrcLength");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "crc4bit");
            break;
        case 1:
            ShowPERNull(tag, indent, "crc12bit");
            break;
        case 2:
            ShowPERNull(tag, indent, "crc20bit");
            break;
        case 3:
            ShowPERNull(tag, indent, "crc28bit");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            ShowPERNull(tag, indent, "crc8bit");
            break;
        case 5:
            ShowPERNull(tag, indent, "crc16bit");
            break;
        case 6:
            ShowPERNull(tag, indent, "crc32bit");
            break;
        case 7:
            ShowPERNull(tag, indent, "crcNotUsed");
            break;
        default:
            ErrorMessage("Analyze_Al3CrcLength: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Al3CrcLength");
}

/* <======================================> */
/*  PER-Analyzer for HeaderFormat (CHOICE)  */
/* <======================================> */
void Analyze_HeaderFormat(PS_HeaderFormat x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "HeaderFormat");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "sebch16_7");
            break;
        case 1:
            ShowPERNull(tag, indent, "golay24_12");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_HeaderFormat: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "HeaderFormat");
}

/* <===================================================> */
/*  PER-Analyzer for H223AnnexCArqParameters (SEQUENCE)  */
/* <===================================================> */
void Analyze_H223AnnexCArqParameters(PS_H223AnnexCArqParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223AnnexCArqParameters");
    indent += 2;
    Analyze_NumberOfRetransmissions(&x->numberOfRetransmissions, "numberOfRetransmissions", tag, indent);
    ShowPERInteger(tag, indent, "sendBufferSize", (int32)x->sendBufferSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H223AnnexCArqParameters");
}

/* <=================================================> */
/*  PER-Analyzer for NumberOfRetransmissions (CHOICE)  */
/* <=================================================> */
void Analyze_NumberOfRetransmissions(PS_NumberOfRetransmissions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "NumberOfRetransmissions");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "finite", (uint32)x->finite);
            break;
        case 1:
            ShowPERNull(tag, indent, "infinite");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_NumberOfRetransmissions: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "NumberOfRetransmissions");
}

/* <=======================================================> */
/*  PER-Analyzer for V76LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Analyze_V76LogicalChannelParameters(PS_V76LogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V76LogicalChannelParameters");
    indent += 2;
    Analyze_V76HDLCParameters(&x->hdlcParameters, "hdlcParameters", tag, indent);
    Analyze_SuspendResume(&x->suspendResume, "suspendResume", tag, indent);
    ShowPERBoolean(tag, indent, "uIH", x->uIH);
    Analyze_Mode(&x->mode, "mode", tag, indent);
    Analyze_V75Parameters(&x->v75Parameters, "v75Parameters", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V76LogicalChannelParameters");
}

/* <==============================> */
/*  PER-Analyzer for Mode (CHOICE)  */
/* <==============================> */
void Analyze_Mode(PS_Mode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Mode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_ERM(x->eRM, "eRM", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "uNERM");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Mode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Mode");
}

/* <===============================> */
/*  PER-Analyzer for ERM (SEQUENCE)  */
/* <===============================> */
void Analyze_ERM(PS_ERM x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ERM");
    indent += 2;
    ShowPERInteger(tag, indent, "windowSize", (int32)x->windowSize);
    Analyze_Recovery(&x->recovery, "recovery", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ERM");
}

/* <==================================> */
/*  PER-Analyzer for Recovery (CHOICE)  */
/* <==================================> */
void Analyze_Recovery(PS_Recovery x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Recovery");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "rej");
            break;
        case 1:
            ShowPERNull(tag, indent, "sREJ");
            break;
        case 2:
            ShowPERNull(tag, indent, "mSREJ");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Recovery: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Recovery");
}

/* <=======================================> */
/*  PER-Analyzer for SuspendResume (CHOICE)  */
/* <=======================================> */
void Analyze_SuspendResume(PS_SuspendResume x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "SuspendResume");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noSuspendResume");
            break;
        case 1:
            ShowPERNull(tag, indent, "suspendResumewAddress");
            break;
        case 2:
            ShowPERNull(tag, indent, "suspendResumewoAddress");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_SuspendResume: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "SuspendResume");
}

/* <=============================================> */
/*  PER-Analyzer for V76HDLCParameters (SEQUENCE)  */
/* <=============================================> */
void Analyze_V76HDLCParameters(PS_V76HDLCParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "V76HDLCParameters");
    indent += 2;
    Analyze_CRCLength(&x->crcLength, "crcLength", tag, indent);
    ShowPERInteger(tag, indent, "n401", (int32)x->n401);
    ShowPERBoolean(tag, indent, "loopbackTestProcedure", x->loopbackTestProcedure);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "V76HDLCParameters");
}

/* <===================================> */
/*  PER-Analyzer for CRCLength (CHOICE)  */
/* <===================================> */
void Analyze_CRCLength(PS_CRCLength x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CRCLength");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "crc8bit");
            break;
        case 1:
            ShowPERNull(tag, indent, "crc16bit");
            break;
        case 2:
            ShowPERNull(tag, indent, "crc32bit");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CRCLength: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CRCLength");
}

/* <=========================================================> */
/*  PER-Analyzer for H2250LogicalChannelParameters (SEQUENCE)  */
/* <=========================================================> */
void Analyze_H2250LogicalChannelParameters(PS_H2250LogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H2250LogicalChannelParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    ShowPERBoolean(tag, indent, "option_of_associatedSessionID", x->option_of_associatedSessionID);
    ShowPERBoolean(tag, indent, "option_of_mediaChannel", x->option_of_mediaChannel);
    ShowPERBoolean(tag, indent, "option_of_mediaGuaranteedDelivery", x->option_of_mediaGuaranteedDelivery);
    ShowPERBoolean(tag, indent, "option_of_mediaControlChannel", x->option_of_mediaControlChannel);
    ShowPERBoolean(tag, indent, "option_of_mediaControlGuaranteedDelivery", x->option_of_mediaControlGuaranteedDelivery);
    ShowPERBoolean(tag, indent, "option_of_silenceSuppression", x->option_of_silenceSuppression);
    ShowPERBoolean(tag, indent, "option_of_destination", x->option_of_destination);
    ShowPERBoolean(tag, indent, "option_of_dynamicRTPPayloadType", x->option_of_dynamicRTPPayloadType);
    ShowPERBoolean(tag, indent, "option_of_mediaPacketization", x->option_of_mediaPacketization);
    if (x->option_of_nonStandard)
    {
        ShowPERInteger(tag, indent, "size_of_nonStandard", (uint32)x->size_of_nonStandard);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            sprintf(tempLabelString, "nonStandard[%hu]", i);
            Analyze_NonStandardParameter(x->nonStandard + i, tempLabelString, tag, indent);
        }
    }
    ShowPERInteger(tag, indent, "sessionID", (int32)x->sessionID);
    if (x->option_of_associatedSessionID)
    {
        ShowPERInteger(tag, indent, "associatedSessionID", (int32)x->associatedSessionID);
    }
    if (x->option_of_mediaChannel)
    {
        Analyze_TransportAddress(&x->mediaChannel, "mediaChannel", tag, indent);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        ShowPERBoolean(tag, indent, "mediaGuaranteedDelivery", x->mediaGuaranteedDelivery);
    }
    if (x->option_of_mediaControlChannel)
    {
        Analyze_TransportAddress(&x->mediaControlChannel, "mediaControlChannel", tag, indent);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        ShowPERBoolean(tag, indent, "mediaControlGuaranteedDelivery", x->mediaControlGuaranteedDelivery);
    }
    if (x->option_of_silenceSuppression)
    {
        ShowPERBoolean(tag, indent, "silenceSuppression", x->silenceSuppression);
    }
    if (x->option_of_destination)
    {
        Analyze_TerminalLabel(&x->destination, "destination", tag, indent);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        ShowPERInteger(tag, indent, "dynamicRTPPayloadType", (int32)x->dynamicRTPPayloadType);
    }
    if (x->option_of_mediaPacketization)
    {
        Analyze_MediaPacketization(&x->mediaPacketization, "mediaPacketization", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_transportCapability", x->option_of_transportCapability);
    ShowPERBoolean(tag, indent, "option_of_redundancyEncoding", x->option_of_redundancyEncoding);
    ShowPERBoolean(tag, indent, "option_of_source", x->option_of_source);
    ShowPERBoolean(tag, indent, "option_of_symmetricMediaAddress", x->option_of_symmetricMediaAddress);
    if (x->option_of_transportCapability)
    {
        Analyze_TransportCapability(&x->transportCapability, "transportCapability", tag, indent);
    }
    if (x->option_of_redundancyEncoding)
    {
        Analyze_RedundancyEncoding(&x->redundancyEncoding, "redundancyEncoding", tag, indent);
    }
    if (x->option_of_source)
    {
        Analyze_TerminalLabel(&x->source, "source", tag, indent);
    }
    if (x->option_of_symmetricMediaAddress)
    {
        ShowPERBoolean(tag, indent, "symmetricMediaAddress", x->symmetricMediaAddress);
    }
    ShowPERClosure(tag, indent, "H2250LogicalChannelParameters");
}

/* <============================================> */
/*  PER-Analyzer for MediaPacketization (CHOICE)  */
/* <============================================> */
void Analyze_MediaPacketization(PS_MediaPacketization x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MediaPacketization");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "h261aVideoPacketization");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            Analyze_RTPPayloadType(x->rtpPayloadType, "rtpPayloadType", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_MediaPacketization: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MediaPacketization");
}

/* <==========================================> */
/*  PER-Analyzer for RTPPayloadType (SEQUENCE)  */
/* <==========================================> */
void Analyze_RTPPayloadType(PS_RTPPayloadType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RTPPayloadType");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_payloadType", x->option_of_payloadType);
    Analyze_PayloadDescriptor(&x->payloadDescriptor, "payloadDescriptor", tag, indent);
    if (x->option_of_payloadType)
    {
        ShowPERInteger(tag, indent, "payloadType", (int32)x->payloadType);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RTPPayloadType");
}

/* <===========================================> */
/*  PER-Analyzer for PayloadDescriptor (CHOICE)  */
/* <===========================================> */
void Analyze_PayloadDescriptor(PS_PayloadDescriptor x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "PayloadDescriptor");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandardIdentifier, "nonStandardIdentifier", tag, indent);
            break;
        case 1:
            ShowPERInteger(tag, indent, "rfc_number", (uint32)x->rfc_number);
            break;
        case 2:
            ShowPERObjectID(tag, indent, "oid", x->oid);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_PayloadDescriptor: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "PayloadDescriptor");
}

/* <==============================================> */
/*  PER-Analyzer for RedundancyEncoding (SEQUENCE)  */
/* <==============================================> */
void Analyze_RedundancyEncoding(PS_RedundancyEncoding x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RedundancyEncoding");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_secondaryEncoding", x->option_of_secondaryEncoding);
    Analyze_RedundancyEncodingMethod(&x->redundancyEncodingMethod, "redundancyEncodingMethod", tag, indent);
    if (x->option_of_secondaryEncoding)
    {
        Analyze_DataType(&x->secondaryEncoding, "secondaryEncoding", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RedundancyEncoding");
}

/* <==========================================> */
/*  PER-Analyzer for TransportAddress (CHOICE)  */
/* <==========================================> */
void Analyze_TransportAddress(PS_TransportAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "TransportAddress");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_UnicastAddress(x->unicastAddress, "unicastAddress", tag, indent);
            break;
        case 1:
            Analyze_MulticastAddress(x->multicastAddress, "multicastAddress", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_TransportAddress: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "TransportAddress");
}

/* <========================================> */
/*  PER-Analyzer for UnicastAddress (CHOICE)  */
/* <========================================> */
void Analyze_UnicastAddress(PS_UnicastAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "UnicastAddress");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_IPAddress(x->iPAddress, "iPAddress", tag, indent);
            break;
        case 1:
            Analyze_IPXAddress(x->iPXAddress, "iPXAddress", tag, indent);
            break;
        case 2:
            Analyze_IP6Address(x->iP6Address, "iP6Address", tag, indent);
            break;
        case 3:
            ShowPEROctetString(tag, indent, "netBios", x->netBios);
            break;
        case 4:
            Analyze_IPSourceRouteAddress(x->iPSourceRouteAddress, "iPSourceRouteAddress", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            ShowPEROctetString(tag, indent, "nsap", x->nsap);
            break;
        case 6:
            Analyze_NonStandardParameter(x->nonStandardAddress, "nonStandardAddress", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_UnicastAddress: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "UnicastAddress");
}

/* <================================================> */
/*  PER-Analyzer for IPSourceRouteAddress (SEQUENCE)  */
/* <================================================> */
void Analyze_IPSourceRouteAddress(PS_IPSourceRouteAddress x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "IPSourceRouteAddress");
    indent += 2;
    Analyze_Routing(&x->routing, "routing", tag, indent);
    ShowPEROctetString(tag, indent, "network", &x->network);
    ShowPERInteger(tag, indent, "tsapIdentifier", (int32)x->tsapIdentifier);
    ShowPERInteger(tag, indent, "size_of_route", (uint32)x->size_of_route);
    for (i = 0;i < x->size_of_route;++i)
    {
        ShowPEROctetStrings(tag, indent, "route", x->route + i, i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IPSourceRouteAddress");
}

/* <=================================> */
/*  PER-Analyzer for Routing (CHOICE)  */
/* <=================================> */
void Analyze_Routing(PS_Routing x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Routing");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "strict");
            break;
        case 1:
            ShowPERNull(tag, indent, "loose");
            break;
        default:
            ErrorMessage("Analyze_Routing: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Routing");
}

/* <======================================> */
/*  PER-Analyzer for IP6Address (SEQUENCE)  */
/* <======================================> */
void Analyze_IP6Address(PS_IP6Address x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IP6Address");
    indent += 2;
    ShowPEROctetString(tag, indent, "network", &x->network);
    ShowPERInteger(tag, indent, "tsapIdentifier", (int32)x->tsapIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IP6Address");
}

/* <======================================> */
/*  PER-Analyzer for IPXAddress (SEQUENCE)  */
/* <======================================> */
void Analyze_IPXAddress(PS_IPXAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IPXAddress");
    indent += 2;
    ShowPEROctetString(tag, indent, "node", &x->node);
    ShowPEROctetString(tag, indent, "netnum", &x->netnum);
    ShowPEROctetString(tag, indent, "tsapIdentifier", &x->tsapIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IPXAddress");
}

/* <=====================================> */
/*  PER-Analyzer for IPAddress (SEQUENCE)  */
/* <=====================================> */
void Analyze_IPAddress(PS_IPAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IPAddress");
    indent += 2;
    ShowPEROctetString(tag, indent, "network", &x->network);
    ShowPERInteger(tag, indent, "tsapIdentifier", (int32)x->tsapIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IPAddress");
}

/* <==========================================> */
/*  PER-Analyzer for MulticastAddress (CHOICE)  */
/* <==========================================> */
void Analyze_MulticastAddress(PS_MulticastAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MulticastAddress");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_MaIpAddress(x->maIpAddress, "maIpAddress", tag, indent);
            break;
        case 1:
            Analyze_MaIp6Address(x->maIp6Address, "maIp6Address", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            ShowPEROctetString(tag, indent, "nsap", x->nsap);
            break;
        case 3:
            Analyze_NonStandardParameter(x->nonStandardAddress, "nonStandardAddress", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_MulticastAddress: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MulticastAddress");
}

/* <========================================> */
/*  PER-Analyzer for MaIp6Address (SEQUENCE)  */
/* <========================================> */
void Analyze_MaIp6Address(PS_MaIp6Address x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaIp6Address");
    indent += 2;
    ShowPEROctetString(tag, indent, "network", &x->network);
    ShowPERInteger(tag, indent, "tsapIdentifier", (int32)x->tsapIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaIp6Address");
}

/* <=======================================> */
/*  PER-Analyzer for MaIpAddress (SEQUENCE)  */
/* <=======================================> */
void Analyze_MaIpAddress(PS_MaIpAddress x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaIpAddress");
    indent += 2;
    ShowPEROctetString(tag, indent, "network", &x->network);
    ShowPERInteger(tag, indent, "tsapIdentifier", (int32)x->tsapIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaIpAddress");
}

/* <==========================================> */
/*  PER-Analyzer for EncryptionSync (SEQUENCE)  */
/* <==========================================> */
void Analyze_EncryptionSync(PS_EncryptionSync x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "EncryptionSync");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    ShowPERBoolean(tag, indent, "option_of_escrowentry", x->option_of_escrowentry);
    if (x->option_of_nonStandard)
    {
        Analyze_NonStandardParameter(&x->nonStandard, "nonStandard", tag, indent);
    }
    ShowPERInteger(tag, indent, "synchFlag", (int32)x->synchFlag);
    ShowPEROctetString(tag, indent, "h235Key", &x->h235Key);
    if (x->option_of_escrowentry)
    {
        ShowPERInteger(tag, indent, "size_of_escrowentry", (uint32)x->size_of_escrowentry);
        for (i = 0;i < x->size_of_escrowentry;++i)
        {
            sprintf(tempLabelString, "escrowentry[%hu]", i);
            Analyze_EscrowData(x->escrowentry + i, tempLabelString, tag, indent);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EncryptionSync");
}

/* <======================================> */
/*  PER-Analyzer for EscrowData (SEQUENCE)  */
/* <======================================> */
void Analyze_EscrowData(PS_EscrowData x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "EscrowData");
    indent += 2;
    ShowPERObjectID(tag, indent, "escrowID", &x->escrowID);
    ShowPERBitString(tag, indent, "escrowValue", &x->escrowValue);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EscrowData");
}

/* <=================================================> */
/*  PER-Analyzer for OpenLogicalChannelAck (SEQUENCE)  */
/* <=================================================> */
void Analyze_OpenLogicalChannelAck(PS_OpenLogicalChannelAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "OpenLogicalChannelAck");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_ackReverseLogicalChannelParameters", x->option_of_ackReverseLogicalChannelParameters);
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    if (x->option_of_ackReverseLogicalChannelParameters)
    {
        Analyze_AckReverseLogicalChannelParameters(&x->ackReverseLogicalChannelParameters, "ackReverseLogicalChannelParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_separateStack", x->option_of_separateStack);
    ShowPERBoolean(tag, indent, "option_of_forwardMultiplexAckParameters", x->option_of_forwardMultiplexAckParameters);
    ShowPERBoolean(tag, indent, "option_of_encryptionSync", x->option_of_encryptionSync);
    if (x->option_of_separateStack)
    {
        Analyze_NetworkAccessParameters(&x->separateStack, "separateStack", tag, indent);
    }
    if (x->option_of_forwardMultiplexAckParameters)
    {
        Analyze_ForwardMultiplexAckParameters(&x->forwardMultiplexAckParameters, "forwardMultiplexAckParameters", tag, indent);
    }
    if (x->option_of_encryptionSync)
    {
        Analyze_EncryptionSync(&x->encryptionSync, "encryptionSync", tag, indent);
    }
    ShowPERClosure(tag, indent, "OpenLogicalChannelAck");
}

/* <=======================================================> */
/*  PER-Analyzer for ForwardMultiplexAckParameters (CHOICE)  */
/* <=======================================================> */
void Analyze_ForwardMultiplexAckParameters(PS_ForwardMultiplexAckParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ForwardMultiplexAckParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_H2250LogicalChannelAckParameters(x->h2250LogicalChannelAckParameters, "h2250LogicalChannelAckParameters", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ForwardMultiplexAckParameters: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ForwardMultiplexAckParameters");
}

/* <==============================================================> */
/*  PER-Analyzer for AckReverseLogicalChannelParameters (SEQUENCE)  */
/* <==============================================================> */
void Analyze_AckReverseLogicalChannelParameters(PS_AckReverseLogicalChannelParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "AckReverseLogicalChannelParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_portNumber", x->option_of_portNumber);
    ShowPERBoolean(tag, indent, "option_of_ackMultiplexParameters", x->option_of_ackMultiplexParameters);
    ShowPERInteger(tag, indent, "reverseLogicalChannelNumber", (int32)x->reverseLogicalChannelNumber);
    if (x->option_of_portNumber)
    {
        ShowPERInteger(tag, indent, "portNumber", (int32)x->portNumber);
    }
    if (x->option_of_ackMultiplexParameters)
    {
        Analyze_AckMultiplexParameters(&x->ackMultiplexParameters, "ackMultiplexParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_replacementFor", x->option_of_replacementFor);
    if (x->option_of_replacementFor)
    {
        ShowPERInteger(tag, indent, "replacementFor", (int32)x->replacementFor);
    }
    ShowPERClosure(tag, indent, "AckReverseLogicalChannelParameters");
}

/* <================================================> */
/*  PER-Analyzer for AckMultiplexParameters (CHOICE)  */
/* <================================================> */
void Analyze_AckMultiplexParameters(PS_AckMultiplexParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AckMultiplexParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_H222LogicalChannelParameters(x->h222LogicalChannelParameters, "h222LogicalChannelParameters", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            Analyze_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, "h2250LogicalChannelParameters", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_AckMultiplexParameters: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AckMultiplexParameters");
}

/* <====================================================> */
/*  PER-Analyzer for OpenLogicalChannelReject (SEQUENCE)  */
/* <====================================================> */
void Analyze_OpenLogicalChannelReject(PS_OpenLogicalChannelReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "OpenLogicalChannelReject");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    Analyze_OlcRejectCause(&x->olcRejectCause, "olcRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "OpenLogicalChannelReject");
}

/* <========================================> */
/*  PER-Analyzer for OlcRejectCause (CHOICE)  */
/* <========================================> */
void Analyze_OlcRejectCause(PS_OlcRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "OlcRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecified");
            break;
        case 1:
            ShowPERNull(tag, indent, "unsuitableReverseParameters");
            break;
        case 2:
            ShowPERNull(tag, indent, "dataTypeNotSupported");
            break;
        case 3:
            ShowPERNull(tag, indent, "dataTypeNotAvailable");
            break;
        case 4:
            ShowPERNull(tag, indent, "unknownDataType");
            break;
        case 5:
            ShowPERNull(tag, indent, "dataTypeALCombinationNotSupported");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            ShowPERNull(tag, indent, "multicastChannelNotAllowed");
            break;
        case 7:
            ShowPERNull(tag, indent, "insufficientBandwidth");
            break;
        case 8:
            ShowPERNull(tag, indent, "separateStackEstablishmentFailed");
            break;
        case 9:
            ShowPERNull(tag, indent, "invalidSessionID");
            break;
        case 10:
            ShowPERNull(tag, indent, "masterSlaveConflict");
            break;
        case 11:
            ShowPERNull(tag, indent, "waitForCommunicationMode");
            break;
        case 12:
            ShowPERNull(tag, indent, "invalidDependentChannel");
            break;
        case 13:
            ShowPERNull(tag, indent, "replacementForRejected");
            break;
        default:
            ErrorMessage("Analyze_OlcRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "OlcRejectCause");
}

/* <=====================================================> */
/*  PER-Analyzer for OpenLogicalChannelConfirm (SEQUENCE)  */
/* <=====================================================> */
void Analyze_OpenLogicalChannelConfirm(PS_OpenLogicalChannelConfirm x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "OpenLogicalChannelConfirm");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "OpenLogicalChannelConfirm");
}

/* <============================================================> */
/*  PER-Analyzer for H2250LogicalChannelAckParameters (SEQUENCE)  */
/* <============================================================> */
void Analyze_H2250LogicalChannelAckParameters(PS_H2250LogicalChannelAckParameters x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "H2250LogicalChannelAckParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    ShowPERBoolean(tag, indent, "option_of_sessionID", x->option_of_sessionID);
    ShowPERBoolean(tag, indent, "option_of_mediaChannel", x->option_of_mediaChannel);
    ShowPERBoolean(tag, indent, "option_of_mediaControlChannel", x->option_of_mediaControlChannel);
    ShowPERBoolean(tag, indent, "option_of_dynamicRTPPayloadType", x->option_of_dynamicRTPPayloadType);
    if (x->option_of_nonStandard)
    {
        ShowPERInteger(tag, indent, "size_of_nonStandard", (uint32)x->size_of_nonStandard);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            sprintf(tempLabelString, "nonStandard[%hu]", i);
            Analyze_NonStandardParameter(x->nonStandard + i, tempLabelString, tag, indent);
        }
    }
    if (x->option_of_sessionID)
    {
        ShowPERInteger(tag, indent, "sessionID", (int32)x->sessionID);
    }
    if (x->option_of_mediaChannel)
    {
        Analyze_TransportAddress(&x->mediaChannel, "mediaChannel", tag, indent);
    }
    if (x->option_of_mediaControlChannel)
    {
        Analyze_TransportAddress(&x->mediaControlChannel, "mediaControlChannel", tag, indent);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        ShowPERInteger(tag, indent, "dynamicRTPPayloadType", (int32)x->dynamicRTPPayloadType);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_flowControlToZero", x->option_of_flowControlToZero);
    ShowPERBoolean(tag, indent, "option_of_portNumber", x->option_of_portNumber);
    if (x->option_of_flowControlToZero)
    {
        ShowPERBoolean(tag, indent, "flowControlToZero", x->flowControlToZero);
    }
    if (x->option_of_portNumber)
    {
        ShowPERInteger(tag, indent, "portNumber", (int32)x->portNumber);
    }
    ShowPERClosure(tag, indent, "H2250LogicalChannelAckParameters");
}

/* <===============================================> */
/*  PER-Analyzer for CloseLogicalChannel (SEQUENCE)  */
/* <===============================================> */
void Analyze_CloseLogicalChannel(PS_CloseLogicalChannel x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CloseLogicalChannel");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    Analyze_Source(&x->source, "source", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_reason", x->option_of_reason);
    if (x->option_of_reason)
    {
        Analyze_Reason(&x->reason, "reason", tag, indent);
    }
    ShowPERClosure(tag, indent, "CloseLogicalChannel");
}

/* <================================> */
/*  PER-Analyzer for Reason (CHOICE)  */
/* <================================> */
void Analyze_Reason(PS_Reason x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Reason");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unknown");
            break;
        case 1:
            ShowPERNull(tag, indent, "reopen");
            break;
        case 2:
            ShowPERNull(tag, indent, "reservationFailure");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Reason: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Reason");
}

/* <================================> */
/*  PER-Analyzer for Source (CHOICE)  */
/* <================================> */
void Analyze_Source(PS_Source x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Source");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "user");
            break;
        case 1:
            ShowPERNull(tag, indent, "lcse");
            break;
        default:
            ErrorMessage("Analyze_Source: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Source");
}

/* <==================================================> */
/*  PER-Analyzer for CloseLogicalChannelAck (SEQUENCE)  */
/* <==================================================> */
void Analyze_CloseLogicalChannelAck(PS_CloseLogicalChannelAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CloseLogicalChannelAck");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CloseLogicalChannelAck");
}

/* <===============================================> */
/*  PER-Analyzer for RequestChannelClose (SEQUENCE)  */
/* <===============================================> */
void Analyze_RequestChannelClose(PS_RequestChannelClose x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestChannelClose");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_qosCapability", x->option_of_qosCapability);
    ShowPERBoolean(tag, indent, "option_of_rccReason", x->option_of_rccReason);
    if (x->option_of_qosCapability)
    {
        Analyze_QOSCapability(&x->qosCapability, "qosCapability", tag, indent);
    }
    if (x->option_of_rccReason)
    {
        Analyze_RccReason(&x->rccReason, "rccReason", tag, indent);
    }
    ShowPERClosure(tag, indent, "RequestChannelClose");
}

/* <===================================> */
/*  PER-Analyzer for RccReason (CHOICE)  */
/* <===================================> */
void Analyze_RccReason(PS_RccReason x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RccReason");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unknown");
            break;
        case 1:
            ShowPERNull(tag, indent, "normal");
            break;
        case 2:
            ShowPERNull(tag, indent, "reopen");
            break;
        case 3:
            ShowPERNull(tag, indent, "reservationFailure");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RccReason: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RccReason");
}

/* <==================================================> */
/*  PER-Analyzer for RequestChannelCloseAck (SEQUENCE)  */
/* <==================================================> */
void Analyze_RequestChannelCloseAck(PS_RequestChannelCloseAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestChannelCloseAck");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestChannelCloseAck");
}

/* <=====================================================> */
/*  PER-Analyzer for RequestChannelCloseReject (SEQUENCE)  */
/* <=====================================================> */
void Analyze_RequestChannelCloseReject(PS_RequestChannelCloseReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestChannelCloseReject");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    Analyze_RccRejectCause(&x->rccRejectCause, "rccRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestChannelCloseReject");
}

/* <========================================> */
/*  PER-Analyzer for RccRejectCause (CHOICE)  */
/* <========================================> */
void Analyze_RccRejectCause(PS_RccRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RccRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecified");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RccRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RccRejectCause");
}

/* <======================================================> */
/*  PER-Analyzer for RequestChannelCloseRelease (SEQUENCE)  */
/* <======================================================> */
void Analyze_RequestChannelCloseRelease(PS_RequestChannelCloseRelease x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestChannelCloseRelease");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardLogicalChannelNumber", (int32)x->forwardLogicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestChannelCloseRelease");
}

/* <==============================================> */
/*  PER-Analyzer for MultiplexEntrySend (SEQUENCE)  */
/* <==============================================> */
void Analyze_MultiplexEntrySend(PS_MultiplexEntrySend x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultiplexEntrySend");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "size_of_multiplexEntryDescriptors", (uint32)x->size_of_multiplexEntryDescriptors);
    for (i = 0;i < x->size_of_multiplexEntryDescriptors;++i)
    {
        sprintf(tempLabelString, "multiplexEntryDescriptors[%hu]", i);
        Analyze_MultiplexEntryDescriptor(x->multiplexEntryDescriptors + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultiplexEntrySend");
}

/* <====================================================> */
/*  PER-Analyzer for MultiplexEntryDescriptor (SEQUENCE)  */
/* <====================================================> */
void Analyze_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultiplexEntryDescriptor");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_elementList", x->option_of_elementList);
    ShowPERInteger(tag, indent, "multiplexTableEntryNumber", (int32)x->multiplexTableEntryNumber);
    if (x->option_of_elementList)
    {
        ShowPERInteger(tag, indent, "size_of_elementList", (uint32)x->size_of_elementList);
        for (i = 0;i < x->size_of_elementList;++i)
        {
            sprintf(tempLabelString, "elementList[%hu]", i);
            Analyze_MultiplexElement(x->elementList + i, tempLabelString, tag, indent);
        }
    }
    ShowPERClosure(tag, indent, "MultiplexEntryDescriptor");
}

/* <============================================> */
/*  PER-Analyzer for MultiplexElement (SEQUENCE)  */
/* <============================================> */
void Analyze_MultiplexElement(PS_MultiplexElement x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MultiplexElement");
    indent += 2;
    Analyze_MuxType(&x->muxType, "muxType", tag, indent);
    Analyze_RepeatCount(&x->repeatCount, "repeatCount", tag, indent);
    ShowPERClosure(tag, indent, "MultiplexElement");
}

/* <=====================================> */
/*  PER-Analyzer for RepeatCount (CHOICE)  */
/* <=====================================> */
void Analyze_RepeatCount(PS_RepeatCount x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RepeatCount");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "finite", (uint32)x->finite);
            break;
        case 1:
            ShowPERNull(tag, indent, "untilClosingFlag");
            break;
        default:
            ErrorMessage("Analyze_RepeatCount: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RepeatCount");
}

/* <=================================> */
/*  PER-Analyzer for MuxType (CHOICE)  */
/* <=================================> */
void Analyze_MuxType(PS_MuxType x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "MuxType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "logicalChannelNumber", (uint32)x->logicalChannelNumber);
            break;
        case 1:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "subElementList[%hu]", i);
                Analyze_MultiplexElement(x->subElementList + i, tempLabelString, tag, indent);
            }
            break;
        default:
            ErrorMessage("Analyze_MuxType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MuxType");
}

/* <=================================================> */
/*  PER-Analyzer for MultiplexEntrySendAck (SEQUENCE)  */
/* <=================================================> */
void Analyze_MultiplexEntrySendAck(PS_MultiplexEntrySendAck x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultiplexEntrySendAck");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "size_of_multiplexTableEntryNumber", (uint32)x->size_of_multiplexTableEntryNumber);
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        ShowPERIntegers(tag, indent, "multiplexTableEntryNumber", (uint32)x->multiplexTableEntryNumber[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultiplexEntrySendAck");
}

/* <====================================================> */
/*  PER-Analyzer for MultiplexEntrySendReject (SEQUENCE)  */
/* <====================================================> */
void Analyze_MultiplexEntrySendReject(PS_MultiplexEntrySendReject x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultiplexEntrySendReject");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "size_of_rejectionDescriptions", (uint32)x->size_of_rejectionDescriptions);
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        sprintf(tempLabelString, "rejectionDescriptions[%hu]", i);
        Analyze_MultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultiplexEntrySendReject");
}

/* <===============================================================> */
/*  PER-Analyzer for MultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <===============================================================> */
void Analyze_MultiplexEntryRejectionDescriptions(PS_MultiplexEntryRejectionDescriptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MultiplexEntryRejectionDescriptions");
    indent += 2;
    ShowPERInteger(tag, indent, "multiplexTableEntryNumber", (int32)x->multiplexTableEntryNumber);
    Analyze_MeRejectCause(&x->meRejectCause, "meRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultiplexEntryRejectionDescriptions");
}

/* <=======================================> */
/*  PER-Analyzer for MeRejectCause (CHOICE)  */
/* <=======================================> */
void Analyze_MeRejectCause(PS_MeRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MeRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecifiedCause");
            break;
        case 1:
            ShowPERNull(tag, indent, "descriptorTooComplex");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MeRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MeRejectCause");
}

/* <=====================================================> */
/*  PER-Analyzer for MultiplexEntrySendRelease (SEQUENCE)  */
/* <=====================================================> */
void Analyze_MultiplexEntrySendRelease(PS_MultiplexEntrySendRelease x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "MultiplexEntrySendRelease");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_multiplexTableEntryNumber", (uint32)x->size_of_multiplexTableEntryNumber);
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        ShowPERIntegers(tag, indent, "multiplexTableEntryNumber", (uint32)x->multiplexTableEntryNumber[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MultiplexEntrySendRelease");
}

/* <=================================================> */
/*  PER-Analyzer for RequestMultiplexEntry (SEQUENCE)  */
/* <=================================================> */
void Analyze_RequestMultiplexEntry(PS_RequestMultiplexEntry x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestMultiplexEntry");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_entryNumbers", (uint32)x->size_of_entryNumbers);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        ShowPERIntegers(tag, indent, "entryNumbers", (uint32)x->entryNumbers[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMultiplexEntry");
}

/* <====================================================> */
/*  PER-Analyzer for RequestMultiplexEntryAck (SEQUENCE)  */
/* <====================================================> */
void Analyze_RequestMultiplexEntryAck(PS_RequestMultiplexEntryAck x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestMultiplexEntryAck");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_entryNumbers", (uint32)x->size_of_entryNumbers);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        ShowPERIntegers(tag, indent, "entryNumbers", (uint32)x->entryNumbers[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMultiplexEntryAck");
}

/* <=======================================================> */
/*  PER-Analyzer for RequestMultiplexEntryReject (SEQUENCE)  */
/* <=======================================================> */
void Analyze_RequestMultiplexEntryReject(PS_RequestMultiplexEntryReject x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestMultiplexEntryReject");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_entryNumbers", (uint32)x->size_of_entryNumbers);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        ShowPERIntegers(tag, indent, "entryNumbers", (uint32)x->entryNumbers[i], i);
    }
    ShowPERInteger(tag, indent, "size_of_rejectionDescriptions", (uint32)x->size_of_rejectionDescriptions);
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        sprintf(tempLabelString, "rejectionDescriptions[%hu]", i);
        Analyze_RequestMultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMultiplexEntryReject");
}

/* <======================================================================> */
/*  PER-Analyzer for RequestMultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <======================================================================> */
void Analyze_RequestMultiplexEntryRejectionDescriptions(PS_RequestMultiplexEntryRejectionDescriptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestMultiplexEntryRejectionDescriptions");
    indent += 2;
    ShowPERInteger(tag, indent, "multiplexTableEntryNumber", (int32)x->multiplexTableEntryNumber);
    Analyze_RmeRejectCause(&x->rmeRejectCause, "rmeRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMultiplexEntryRejectionDescriptions");
}

/* <========================================> */
/*  PER-Analyzer for RmeRejectCause (CHOICE)  */
/* <========================================> */
void Analyze_RmeRejectCause(PS_RmeRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RmeRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecifiedCause");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RmeRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RmeRejectCause");
}

/* <========================================================> */
/*  PER-Analyzer for RequestMultiplexEntryRelease (SEQUENCE)  */
/* <========================================================> */
void Analyze_RequestMultiplexEntryRelease(PS_RequestMultiplexEntryRelease x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestMultiplexEntryRelease");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_entryNumbers", (uint32)x->size_of_entryNumbers);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        ShowPERIntegers(tag, indent, "entryNumbers", (uint32)x->entryNumbers[i], i);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMultiplexEntryRelease");
}

/* <=======================================> */
/*  PER-Analyzer for RequestMode (SEQUENCE)  */
/* <=======================================> */
void Analyze_RequestMode(PS_RequestMode x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestMode");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "size_of_requestedModes", (uint32)x->size_of_requestedModes);
    for (i = 0;i < x->size_of_requestedModes;++i)
    {
        sprintf(tempLabelString, "requestedModes[%hu]", i);
        Analyze_ModeDescription(x->requestedModes + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestMode");
}

/* <==========================================> */
/*  PER-Analyzer for RequestModeAck (SEQUENCE)  */
/* <==========================================> */
void Analyze_RequestModeAck(PS_RequestModeAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestModeAck");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    Analyze_Response(&x->response, "response", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestModeAck");
}

/* <==================================> */
/*  PER-Analyzer for Response (CHOICE)  */
/* <==================================> */
void Analyze_Response(PS_Response x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Response");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "willTransmitMostPreferredMode");
            break;
        case 1:
            ShowPERNull(tag, indent, "willTransmitLessPreferredMode");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Response: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Response");
}

/* <=============================================> */
/*  PER-Analyzer for RequestModeReject (SEQUENCE)  */
/* <=============================================> */
void Analyze_RequestModeReject(PS_RequestModeReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestModeReject");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    Analyze_RmRejectCause(&x->rmRejectCause, "rmRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestModeReject");
}

/* <=======================================> */
/*  PER-Analyzer for RmRejectCause (CHOICE)  */
/* <=======================================> */
void Analyze_RmRejectCause(PS_RmRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RmRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "modeUnavailable");
            break;
        case 1:
            ShowPERNull(tag, indent, "multipointConstraint");
            break;
        case 2:
            ShowPERNull(tag, indent, "requestDenied");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RmRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RmRejectCause");
}

/* <==============================================> */
/*  PER-Analyzer for RequestModeRelease (SEQUENCE)  */
/* <==============================================> */
void Analyze_RequestModeRelease(PS_RequestModeRelease x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "RequestModeRelease");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestModeRelease");
}

/* <=========================================> */
/*  PER-Analyzer for ModeDescription (SET-OF)  */
/* <=========================================> */
void Analyze_ModeDescription(PS_ModeDescription x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSetof(tag, indent, label, "ModeDescription");
    indent += 2;
    ShowPERInteger(tag, indent, "size", (uint32)x->size);
    for (i = 0;i < x->size;++i)
    {
        sprintf(tempLabelString, "item[%hu]", i);
        Analyze_ModeElement(x->item + i, tempLabelString, tag, indent);
    }
    ShowPERClosure(tag, indent, "ModeDescription");
}

/* <=======================================> */
/*  PER-Analyzer for ModeElement (SEQUENCE)  */
/* <=======================================> */
void Analyze_ModeElement(PS_ModeElement x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ModeElement");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_h223ModeParameters", x->option_of_h223ModeParameters);
    Analyze_ModeType(&x->modeType, "modeType", tag, indent);
    if (x->option_of_h223ModeParameters)
    {
        Analyze_H223ModeParameters(&x->h223ModeParameters, "h223ModeParameters", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_v76ModeParameters", x->option_of_v76ModeParameters);
    ShowPERBoolean(tag, indent, "option_of_h2250ModeParameters", x->option_of_h2250ModeParameters);
    ShowPERBoolean(tag, indent, "option_of_genericModeParameters", x->option_of_genericModeParameters);
    if (x->option_of_v76ModeParameters)
    {
        Analyze_V76ModeParameters(&x->v76ModeParameters, "v76ModeParameters", tag, indent);
    }
    if (x->option_of_h2250ModeParameters)
    {
        Analyze_H2250ModeParameters(&x->h2250ModeParameters, "h2250ModeParameters", tag, indent);
    }
    if (x->option_of_genericModeParameters)
    {
        Analyze_GenericCapability(&x->genericModeParameters, "genericModeParameters", tag, indent);
    }
    ShowPERClosure(tag, indent, "ModeElement");
}

/* <==================================> */
/*  PER-Analyzer for ModeType (CHOICE)  */
/* <==================================> */
void Analyze_ModeType(PS_ModeType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ModeType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_VideoMode(x->videoMode, "videoMode", tag, indent);
            break;
        case 2:
            Analyze_AudioMode(x->audioMode, "audioMode", tag, indent);
            break;
        case 3:
            Analyze_DataMode(x->dataMode, "dataMode", tag, indent);
            break;
        case 4:
            Analyze_EncryptionMode(x->encryptionMode, "encryptionMode", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Analyze_H235Mode(x->h235Mode, "h235Mode", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ModeType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ModeType");
}

/* <====================================> */
/*  PER-Analyzer for H235Mode (SEQUENCE)  */
/* <====================================> */
void Analyze_H235Mode(PS_H235Mode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H235Mode");
    indent += 2;
    Analyze_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, "encryptionAuthenticationAndIntegrity", tag, indent);
    Analyze_MediaMode(&x->mediaMode, "mediaMode", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H235Mode");
}

/* <===================================> */
/*  PER-Analyzer for MediaMode (CHOICE)  */
/* <===================================> */
void Analyze_MediaMode(PS_MediaMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MediaMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_VideoMode(x->videoMode, "videoMode", tag, indent);
            break;
        case 2:
            Analyze_AudioMode(x->audioMode, "audioMode", tag, indent);
            break;
        case 3:
            Analyze_DataMode(x->dataMode, "dataMode", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MediaMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MediaMode");
}

/* <==============================================> */
/*  PER-Analyzer for H223ModeParameters (SEQUENCE)  */
/* <==============================================> */
void Analyze_H223ModeParameters(PS_H223ModeParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223ModeParameters");
    indent += 2;
    Analyze_ModeAdaptationLayerType(&x->modeAdaptationLayerType, "modeAdaptationLayerType", tag, indent);
    ShowPERBoolean(tag, indent, "segmentableFlag", x->segmentableFlag);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H223ModeParameters");
}

/* <=================================================> */
/*  PER-Analyzer for ModeAdaptationLayerType (CHOICE)  */
/* <=================================================> */
void Analyze_ModeAdaptationLayerType(PS_ModeAdaptationLayerType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ModeAdaptationLayerType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "al1Framed");
            break;
        case 2:
            ShowPERNull(tag, indent, "al1NotFramed");
            break;
        case 3:
            ShowPERNull(tag, indent, "al2WithoutSequenceNumbers");
            break;
        case 4:
            ShowPERNull(tag, indent, "al2WithSequenceNumbers");
            break;
        case 5:
            Analyze_ModeAl3(x->modeAl3, "modeAl3", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Analyze_H223AL1MParameters(x->al1M, "al1M", tag, indent);
            break;
        case 7:
            Analyze_H223AL2MParameters(x->al2M, "al2M", tag, indent);
            break;
        case 8:
            Analyze_H223AL3MParameters(x->al3M, "al3M", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ModeAdaptationLayerType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ModeAdaptationLayerType");
}

/* <===================================> */
/*  PER-Analyzer for ModeAl3 (SEQUENCE)  */
/* <===================================> */
void Analyze_ModeAl3(PS_ModeAl3 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ModeAl3");
    indent += 2;
    ShowPERInteger(tag, indent, "controlFieldOctets", (int32)x->controlFieldOctets);
    ShowPERInteger(tag, indent, "sendBufferSize", (int32)x->sendBufferSize);
    ShowPERClosure(tag, indent, "ModeAl3");
}

/* <===========================================> */
/*  PER-Analyzer for V76ModeParameters (CHOICE)  */
/* <===========================================> */
void Analyze_V76ModeParameters(PS_V76ModeParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "V76ModeParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "suspendResumewAddress");
            break;
        case 1:
            ShowPERNull(tag, indent, "suspendResumewoAddress");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_V76ModeParameters: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "V76ModeParameters");
}

/* <===============================================> */
/*  PER-Analyzer for H2250ModeParameters (SEQUENCE)  */
/* <===============================================> */
void Analyze_H2250ModeParameters(PS_H2250ModeParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H2250ModeParameters");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_redundancyEncodingMode", x->option_of_redundancyEncodingMode);
    if (x->option_of_redundancyEncodingMode)
    {
        Analyze_RedundancyEncodingMode(&x->redundancyEncodingMode, "redundancyEncodingMode", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H2250ModeParameters");
}

/* <==================================================> */
/*  PER-Analyzer for RedundancyEncodingMode (SEQUENCE)  */
/* <==================================================> */
void Analyze_RedundancyEncodingMode(PS_RedundancyEncodingMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RedundancyEncodingMode");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_secondaryEncoding", x->option_of_secondaryEncoding);
    Analyze_RedundancyEncodingMethod(&x->redundancyEncodingMethod, "redundancyEncodingMethod", tag, indent);
    if (x->option_of_secondaryEncoding)
    {
        Analyze_SecondaryEncoding(&x->secondaryEncoding, "secondaryEncoding", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RedundancyEncodingMode");
}

/* <===========================================> */
/*  PER-Analyzer for SecondaryEncoding (CHOICE)  */
/* <===========================================> */
void Analyze_SecondaryEncoding(PS_SecondaryEncoding x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "SecondaryEncoding");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_AudioMode(x->audioData, "audioData", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_SecondaryEncoding: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "SecondaryEncoding");
}

/* <===================================> */
/*  PER-Analyzer for VideoMode (CHOICE)  */
/* <===================================> */
void Analyze_VideoMode(PS_VideoMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "VideoMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_H261VideoMode(x->h261VideoMode, "h261VideoMode", tag, indent);
            break;
        case 2:
            Analyze_H262VideoMode(x->h262VideoMode, "h262VideoMode", tag, indent);
            break;
        case 3:
            Analyze_H263VideoMode(x->h263VideoMode, "h263VideoMode", tag, indent);
            break;
        case 4:
            Analyze_IS11172VideoMode(x->is11172VideoMode, "is11172VideoMode", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Analyze_GenericCapability(x->genericVideoMode, "genericVideoMode", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_VideoMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "VideoMode");
}

/* <=========================================> */
/*  PER-Analyzer for H261VideoMode (SEQUENCE)  */
/* <=========================================> */
void Analyze_H261VideoMode(PS_H261VideoMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H261VideoMode");
    indent += 2;
    Analyze_Resolution(&x->resolution, "resolution", tag, indent);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "stillImageTransmission", x->stillImageTransmission);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H261VideoMode");
}

/* <====================================> */
/*  PER-Analyzer for Resolution (CHOICE)  */
/* <====================================> */
void Analyze_Resolution(PS_Resolution x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Resolution");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "qcif");
            break;
        case 1:
            ShowPERNull(tag, indent, "cif");
            break;
        default:
            ErrorMessage("Analyze_Resolution: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Resolution");
}

/* <=========================================> */
/*  PER-Analyzer for H262VideoMode (SEQUENCE)  */
/* <=========================================> */
void Analyze_H262VideoMode(PS_H262VideoMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H262VideoMode");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_videoBitRate", x->option_of_videoBitRate);
    ShowPERBoolean(tag, indent, "option_of_vbvBufferSize", x->option_of_vbvBufferSize);
    ShowPERBoolean(tag, indent, "option_of_samplesPerLine", x->option_of_samplesPerLine);
    ShowPERBoolean(tag, indent, "option_of_linesPerFrame", x->option_of_linesPerFrame);
    ShowPERBoolean(tag, indent, "option_of_framesPerSecond", x->option_of_framesPerSecond);
    ShowPERBoolean(tag, indent, "option_of_luminanceSampleRate", x->option_of_luminanceSampleRate);
    Analyze_ProfileAndLevel(&x->profileAndLevel, "profileAndLevel", tag, indent);
    if (x->option_of_videoBitRate)
    {
        ShowPERInteger(tag, indent, "videoBitRate", (int32)x->videoBitRate);
    }
    if (x->option_of_vbvBufferSize)
    {
        ShowPERInteger(tag, indent, "vbvBufferSize", (int32)x->vbvBufferSize);
    }
    if (x->option_of_samplesPerLine)
    {
        ShowPERInteger(tag, indent, "samplesPerLine", (int32)x->samplesPerLine);
    }
    if (x->option_of_linesPerFrame)
    {
        ShowPERInteger(tag, indent, "linesPerFrame", (int32)x->linesPerFrame);
    }
    if (x->option_of_framesPerSecond)
    {
        ShowPERInteger(tag, indent, "framesPerSecond", (int32)x->framesPerSecond);
    }
    if (x->option_of_luminanceSampleRate)
    {
        ShowPERInteger(tag, indent, "luminanceSampleRate", (int32)x->luminanceSampleRate);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H262VideoMode");
}

/* <=========================================> */
/*  PER-Analyzer for ProfileAndLevel (CHOICE)  */
/* <=========================================> */
void Analyze_ProfileAndLevel(PS_ProfileAndLevel x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ProfileAndLevel");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "profileAndLevel_SPatML");
            break;
        case 1:
            ShowPERNull(tag, indent, "profileAndLevel_MPatLL");
            break;
        case 2:
            ShowPERNull(tag, indent, "profileAndLevel_MPatML");
            break;
        case 3:
            ShowPERNull(tag, indent, "profileAndLevel_MPatH_14");
            break;
        case 4:
            ShowPERNull(tag, indent, "profileAndLevel_MPatHL");
            break;
        case 5:
            ShowPERNull(tag, indent, "profileAndLevel_SNRatLL");
            break;
        case 6:
            ShowPERNull(tag, indent, "profileAndLevel_SNRatML");
            break;
        case 7:
            ShowPERNull(tag, indent, "profileAndLevel_SpatialatH_14");
            break;
        case 8:
            ShowPERNull(tag, indent, "profileAndLevel_HPatML");
            break;
        case 9:
            ShowPERNull(tag, indent, "profileAndLevel_HPatH_14");
            break;
        case 10:
            ShowPERNull(tag, indent, "profileAndLevel_HPatHL");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ProfileAndLevel: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ProfileAndLevel");
}

/* <=========================================> */
/*  PER-Analyzer for H263VideoMode (SEQUENCE)  */
/* <=========================================> */
void Analyze_H263VideoMode(PS_H263VideoMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H263VideoMode");
    indent += 2;
    Analyze_H263Resolution(&x->h263Resolution, "h263Resolution", tag, indent);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "unrestrictedVector", x->unrestrictedVector);
    ShowPERBoolean(tag, indent, "arithmeticCoding", x->arithmeticCoding);
    ShowPERBoolean(tag, indent, "advancedPrediction", x->advancedPrediction);
    ShowPERBoolean(tag, indent, "pbFrames", x->pbFrames);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_errorCompensation", x->option_of_errorCompensation);
    ShowPERBoolean(tag, indent, "option_of_enhancementLayerInfo", x->option_of_enhancementLayerInfo);
    ShowPERBoolean(tag, indent, "option_of_h263Options", x->option_of_h263Options);
    if (x->option_of_errorCompensation)
    {
        ShowPERBoolean(tag, indent, "errorCompensation", x->errorCompensation);
    }
    if (x->option_of_enhancementLayerInfo)
    {
        Analyze_EnhancementLayerInfo(&x->enhancementLayerInfo, "enhancementLayerInfo", tag, indent);
    }
    if (x->option_of_h263Options)
    {
        Analyze_H263Options(&x->h263Options, "h263Options", tag, indent);
    }
    ShowPERClosure(tag, indent, "H263VideoMode");
}

/* <========================================> */
/*  PER-Analyzer for H263Resolution (CHOICE)  */
/* <========================================> */
void Analyze_H263Resolution(PS_H263Resolution x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "H263Resolution");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "sqcif");
            break;
        case 1:
            ShowPERNull(tag, indent, "qcif");
            break;
        case 2:
            ShowPERNull(tag, indent, "cif");
            break;
        case 3:
            ShowPERNull(tag, indent, "cif4");
            break;
        case 4:
            ShowPERNull(tag, indent, "cif16");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_H263Resolution: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "H263Resolution");
}

/* <============================================> */
/*  PER-Analyzer for IS11172VideoMode (SEQUENCE)  */
/* <============================================> */
void Analyze_IS11172VideoMode(PS_IS11172VideoMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS11172VideoMode");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_videoBitRate", x->option_of_videoBitRate);
    ShowPERBoolean(tag, indent, "option_of_vbvBufferSize", x->option_of_vbvBufferSize);
    ShowPERBoolean(tag, indent, "option_of_samplesPerLine", x->option_of_samplesPerLine);
    ShowPERBoolean(tag, indent, "option_of_linesPerFrame", x->option_of_linesPerFrame);
    ShowPERBoolean(tag, indent, "option_of_pictureRate", x->option_of_pictureRate);
    ShowPERBoolean(tag, indent, "option_of_luminanceSampleRate", x->option_of_luminanceSampleRate);
    ShowPERBoolean(tag, indent, "constrainedBitstream", x->constrainedBitstream);
    if (x->option_of_videoBitRate)
    {
        ShowPERInteger(tag, indent, "videoBitRate", (int32)x->videoBitRate);
    }
    if (x->option_of_vbvBufferSize)
    {
        ShowPERInteger(tag, indent, "vbvBufferSize", (int32)x->vbvBufferSize);
    }
    if (x->option_of_samplesPerLine)
    {
        ShowPERInteger(tag, indent, "samplesPerLine", (int32)x->samplesPerLine);
    }
    if (x->option_of_linesPerFrame)
    {
        ShowPERInteger(tag, indent, "linesPerFrame", (int32)x->linesPerFrame);
    }
    if (x->option_of_pictureRate)
    {
        ShowPERInteger(tag, indent, "pictureRate", (int32)x->pictureRate);
    }
    if (x->option_of_luminanceSampleRate)
    {
        ShowPERInteger(tag, indent, "luminanceSampleRate", (int32)x->luminanceSampleRate);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS11172VideoMode");
}

/* <===================================> */
/*  PER-Analyzer for AudioMode (CHOICE)  */
/* <===================================> */
void Analyze_AudioMode(PS_AudioMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AudioMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "g711Alaw64k");
            break;
        case 2:
            ShowPERNull(tag, indent, "g711Alaw56k");
            break;
        case 3:
            ShowPERNull(tag, indent, "g711Ulaw64k");
            break;
        case 4:
            ShowPERNull(tag, indent, "g711Ulaw56k");
            break;
        case 5:
            ShowPERNull(tag, indent, "g722_64k");
            break;
        case 6:
            ShowPERNull(tag, indent, "g722_56k");
            break;
        case 7:
            ShowPERNull(tag, indent, "g722_48k");
            break;
        case 8:
            ShowPERNull(tag, indent, "g728");
            break;
        case 9:
            ShowPERNull(tag, indent, "g729");
            break;
        case 10:
            ShowPERNull(tag, indent, "g729AnnexA");
            break;
        case 11:
            Analyze_ModeG7231(x->modeG7231, "modeG7231", tag, indent);
            break;
        case 12:
            Analyze_IS11172AudioMode(x->is11172AudioMode, "is11172AudioMode", tag, indent);
            break;
        case 13:
            Analyze_IS13818AudioMode(x->is13818AudioMode, "is13818AudioMode", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            ShowPERInteger(tag, indent, "g729wAnnexB", (uint32)x->g729wAnnexB);
            break;
        case 15:
            ShowPERInteger(tag, indent, "g729AnnexAwAnnexB", (uint32)x->g729AnnexAwAnnexB);
            break;
        case 16:
            Analyze_G7231AnnexCMode(x->g7231AnnexCMode, "g7231AnnexCMode", tag, indent);
            break;
        case 17:
            Analyze_GSMAudioCapability(x->gsmFullRate, "gsmFullRate", tag, indent);
            break;
        case 18:
            Analyze_GSMAudioCapability(x->gsmHalfRate, "gsmHalfRate", tag, indent);
            break;
        case 19:
            Analyze_GSMAudioCapability(x->gsmEnhancedFullRate, "gsmEnhancedFullRate", tag, indent);
            break;
        case 20:
            Analyze_GenericCapability(x->genericAudioMode, "genericAudioMode", tag, indent);
            break;
        case 21:
            Analyze_G729Extensions(x->g729Extensions, "g729Extensions", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_AudioMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AudioMode");
}

/* <===================================> */
/*  PER-Analyzer for ModeG7231 (CHOICE)  */
/* <===================================> */
void Analyze_ModeG7231(PS_ModeG7231 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ModeG7231");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noSilenceSuppressionLowRate");
            break;
        case 1:
            ShowPERNull(tag, indent, "noSilenceSuppressionHighRate");
            break;
        case 2:
            ShowPERNull(tag, indent, "silenceSuppressionLowRate");
            break;
        case 3:
            ShowPERNull(tag, indent, "silenceSuppressionHighRate");
            break;
        default:
            ErrorMessage("Analyze_ModeG7231: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ModeG7231");
}

/* <============================================> */
/*  PER-Analyzer for IS11172AudioMode (SEQUENCE)  */
/* <============================================> */
void Analyze_IS11172AudioMode(PS_IS11172AudioMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS11172AudioMode");
    indent += 2;
    Analyze_AudioLayer(&x->audioLayer, "audioLayer", tag, indent);
    Analyze_AudioSampling(&x->audioSampling, "audioSampling", tag, indent);
    Analyze_MultichannelType(&x->multichannelType, "multichannelType", tag, indent);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS11172AudioMode");
}

/* <==========================================> */
/*  PER-Analyzer for MultichannelType (CHOICE)  */
/* <==========================================> */
void Analyze_MultichannelType(PS_MultichannelType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultichannelType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "singleChannel");
            break;
        case 1:
            ShowPERNull(tag, indent, "twoChannelStereo");
            break;
        case 2:
            ShowPERNull(tag, indent, "twoChannelDual");
            break;
        default:
            ErrorMessage("Analyze_MultichannelType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultichannelType");
}

/* <=======================================> */
/*  PER-Analyzer for AudioSampling (CHOICE)  */
/* <=======================================> */
void Analyze_AudioSampling(PS_AudioSampling x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AudioSampling");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "audioSampling32k");
            break;
        case 1:
            ShowPERNull(tag, indent, "audioSampling44k1");
            break;
        case 2:
            ShowPERNull(tag, indent, "audioSampling48k");
            break;
        default:
            ErrorMessage("Analyze_AudioSampling: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AudioSampling");
}

/* <====================================> */
/*  PER-Analyzer for AudioLayer (CHOICE)  */
/* <====================================> */
void Analyze_AudioLayer(PS_AudioLayer x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "AudioLayer");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "audioLayer1");
            break;
        case 1:
            ShowPERNull(tag, indent, "audioLayer2");
            break;
        case 2:
            ShowPERNull(tag, indent, "audioLayer3");
            break;
        default:
            ErrorMessage("Analyze_AudioLayer: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "AudioLayer");
}

/* <============================================> */
/*  PER-Analyzer for IS13818AudioMode (SEQUENCE)  */
/* <============================================> */
void Analyze_IS13818AudioMode(PS_IS13818AudioMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IS13818AudioMode");
    indent += 2;
    Analyze_Is13818AudioLayer(&x->is13818AudioLayer, "is13818AudioLayer", tag, indent);
    Analyze_Is13818AudioSampling(&x->is13818AudioSampling, "is13818AudioSampling", tag, indent);
    Analyze_Is13818MultichannelType(&x->is13818MultichannelType, "is13818MultichannelType", tag, indent);
    ShowPERBoolean(tag, indent, "lowFrequencyEnhancement", x->lowFrequencyEnhancement);
    ShowPERBoolean(tag, indent, "multilingual", x->multilingual);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IS13818AudioMode");
}

/* <=================================================> */
/*  PER-Analyzer for Is13818MultichannelType (CHOICE)  */
/* <=================================================> */
void Analyze_Is13818MultichannelType(PS_Is13818MultichannelType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Is13818MultichannelType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "singleChannel");
            break;
        case 1:
            ShowPERNull(tag, indent, "twoChannelStereo");
            break;
        case 2:
            ShowPERNull(tag, indent, "twoChannelDual");
            break;
        case 3:
            ShowPERNull(tag, indent, "threeChannels2_1");
            break;
        case 4:
            ShowPERNull(tag, indent, "threeChannels3_0");
            break;
        case 5:
            ShowPERNull(tag, indent, "fourChannels2_0_2_0");
            break;
        case 6:
            ShowPERNull(tag, indent, "fourChannels2_2");
            break;
        case 7:
            ShowPERNull(tag, indent, "fourChannels3_1");
            break;
        case 8:
            ShowPERNull(tag, indent, "fiveChannels3_0_2_0");
            break;
        case 9:
            ShowPERNull(tag, indent, "fiveChannels3_2");
            break;
        default:
            ErrorMessage("Analyze_Is13818MultichannelType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Is13818MultichannelType");
}

/* <==============================================> */
/*  PER-Analyzer for Is13818AudioSampling (CHOICE)  */
/* <==============================================> */
void Analyze_Is13818AudioSampling(PS_Is13818AudioSampling x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Is13818AudioSampling");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "audioSampling16k");
            break;
        case 1:
            ShowPERNull(tag, indent, "audioSampling22k05");
            break;
        case 2:
            ShowPERNull(tag, indent, "audioSampling24k");
            break;
        case 3:
            ShowPERNull(tag, indent, "audioSampling32k");
            break;
        case 4:
            ShowPERNull(tag, indent, "audioSampling44k1");
            break;
        case 5:
            ShowPERNull(tag, indent, "audioSampling48k");
            break;
        default:
            ErrorMessage("Analyze_Is13818AudioSampling: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Is13818AudioSampling");
}

/* <===========================================> */
/*  PER-Analyzer for Is13818AudioLayer (CHOICE)  */
/* <===========================================> */
void Analyze_Is13818AudioLayer(PS_Is13818AudioLayer x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Is13818AudioLayer");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "audioLayer1");
            break;
        case 1:
            ShowPERNull(tag, indent, "audioLayer2");
            break;
        case 2:
            ShowPERNull(tag, indent, "audioLayer3");
            break;
        default:
            ErrorMessage("Analyze_Is13818AudioLayer: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Is13818AudioLayer");
}

/* <===========================================> */
/*  PER-Analyzer for G7231AnnexCMode (SEQUENCE)  */
/* <===========================================> */
void Analyze_G7231AnnexCMode(PS_G7231AnnexCMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "G7231AnnexCMode");
    indent += 2;
    ShowPERInteger(tag, indent, "maxAl_sduAudioFrames", (int32)x->maxAl_sduAudioFrames);
    ShowPERBoolean(tag, indent, "silenceSuppression", x->silenceSuppression);
    Analyze_ModeG723AnnexCAudioMode(&x->modeG723AnnexCAudioMode, "modeG723AnnexCAudioMode", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "G7231AnnexCMode");
}

/* <===================================================> */
/*  PER-Analyzer for ModeG723AnnexCAudioMode (SEQUENCE)  */
/* <===================================================> */
void Analyze_ModeG723AnnexCAudioMode(PS_ModeG723AnnexCAudioMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ModeG723AnnexCAudioMode");
    indent += 2;
    ShowPERInteger(tag, indent, "highRateMode0", (int32)x->highRateMode0);
    ShowPERInteger(tag, indent, "highRateMode1", (int32)x->highRateMode1);
    ShowPERInteger(tag, indent, "lowRateMode0", (int32)x->lowRateMode0);
    ShowPERInteger(tag, indent, "lowRateMode1", (int32)x->lowRateMode1);
    ShowPERInteger(tag, indent, "sidMode0", (int32)x->sidMode0);
    ShowPERInteger(tag, indent, "sidMode1", (int32)x->sidMode1);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ModeG723AnnexCAudioMode");
}

/* <====================================> */
/*  PER-Analyzer for DataMode (SEQUENCE)  */
/* <====================================> */
void Analyze_DataMode(PS_DataMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "DataMode");
    indent += 2;
    Analyze_DmApplication(&x->dmApplication, "dmApplication", tag, indent);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "DataMode");
}

/* <=======================================> */
/*  PER-Analyzer for DmApplication (CHOICE)  */
/* <=======================================> */
void Analyze_DmApplication(PS_DmApplication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "DmApplication");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_DataProtocolCapability(x->t120, "t120", tag, indent);
            break;
        case 2:
            Analyze_DataProtocolCapability(x->dsm_cc, "dsm_cc", tag, indent);
            break;
        case 3:
            Analyze_DataProtocolCapability(x->userData, "userData", tag, indent);
            break;
        case 4:
            Analyze_DataProtocolCapability(x->t84, "t84", tag, indent);
            break;
        case 5:
            Analyze_DataProtocolCapability(x->t434, "t434", tag, indent);
            break;
        case 6:
            Analyze_DataProtocolCapability(x->h224, "h224", tag, indent);
            break;
        case 7:
            Analyze_DmNlpid(x->dmNlpid, "dmNlpid", tag, indent);
            break;
        case 8:
            ShowPERNull(tag, indent, "dsvdControl");
            break;
        case 9:
            Analyze_DataProtocolCapability(x->h222DataPartitioning, "h222DataPartitioning", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Analyze_DataProtocolCapability(x->t30fax, "t30fax", tag, indent);
            break;
        case 11:
            Analyze_DataProtocolCapability(x->t140, "t140", tag, indent);
            break;
        case 12:
            Analyze_DmT38fax(x->dmT38fax, "dmT38fax", tag, indent);
            break;
        case 13:
            Analyze_GenericCapability(x->genericDataMode, "genericDataMode", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_DmApplication: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "DmApplication");
}

/* <====================================> */
/*  PER-Analyzer for DmT38fax (SEQUENCE)  */
/* <====================================> */
void Analyze_DmT38fax(PS_DmT38fax x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "DmT38fax");
    indent += 2;
    Analyze_DataProtocolCapability(&x->t38FaxProtocol, "t38FaxProtocol", tag, indent);
    Analyze_T38FaxProfile(&x->t38FaxProfile, "t38FaxProfile", tag, indent);
    ShowPERClosure(tag, indent, "DmT38fax");
}

/* <===================================> */
/*  PER-Analyzer for DmNlpid (SEQUENCE)  */
/* <===================================> */
void Analyze_DmNlpid(PS_DmNlpid x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "DmNlpid");
    indent += 2;
    Analyze_DataProtocolCapability(&x->nlpidProtocol, "nlpidProtocol", tag, indent);
    ShowPEROctetString(tag, indent, "nlpidData", &x->nlpidData);
    ShowPERClosure(tag, indent, "DmNlpid");
}

/* <========================================> */
/*  PER-Analyzer for EncryptionMode (CHOICE)  */
/* <========================================> */
void Analyze_EncryptionMode(PS_EncryptionMode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "EncryptionMode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "h233Encryption");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_EncryptionMode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "EncryptionMode");
}

/* <=================================================> */
/*  PER-Analyzer for RoundTripDelayRequest (SEQUENCE)  */
/* <=================================================> */
void Analyze_RoundTripDelayRequest(PS_RoundTripDelayRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RoundTripDelayRequest");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RoundTripDelayRequest");
}

/* <==================================================> */
/*  PER-Analyzer for RoundTripDelayResponse (SEQUENCE)  */
/* <==================================================> */
void Analyze_RoundTripDelayResponse(PS_RoundTripDelayResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RoundTripDelayResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RoundTripDelayResponse");
}

/* <==================================================> */
/*  PER-Analyzer for MaintenanceLoopRequest (SEQUENCE)  */
/* <==================================================> */
void Analyze_MaintenanceLoopRequest(PS_MaintenanceLoopRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaintenanceLoopRequest");
    indent += 2;
    Analyze_MlRequestType(&x->mlRequestType, "mlRequestType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaintenanceLoopRequest");
}

/* <=======================================> */
/*  PER-Analyzer for MlRequestType (CHOICE)  */
/* <=======================================> */
void Analyze_MlRequestType(PS_MlRequestType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MlRequestType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "systemLoop");
            break;
        case 1:
            ShowPERInteger(tag, indent, "mediaLoop", (uint32)x->mediaLoop);
            break;
        case 2:
            ShowPERInteger(tag, indent, "logicalChannelLoop", (uint32)x->logicalChannelLoop);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MlRequestType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MlRequestType");
}

/* <==============================================> */
/*  PER-Analyzer for MaintenanceLoopAck (SEQUENCE)  */
/* <==============================================> */
void Analyze_MaintenanceLoopAck(PS_MaintenanceLoopAck x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaintenanceLoopAck");
    indent += 2;
    Analyze_MlAckType(&x->mlAckType, "mlAckType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaintenanceLoopAck");
}

/* <===================================> */
/*  PER-Analyzer for MlAckType (CHOICE)  */
/* <===================================> */
void Analyze_MlAckType(PS_MlAckType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MlAckType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "systemLoop");
            break;
        case 1:
            ShowPERInteger(tag, indent, "mediaLoop", (uint32)x->mediaLoop);
            break;
        case 2:
            ShowPERInteger(tag, indent, "logicalChannelLoop", (uint32)x->logicalChannelLoop);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MlAckType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MlAckType");
}

/* <=================================================> */
/*  PER-Analyzer for MaintenanceLoopReject (SEQUENCE)  */
/* <=================================================> */
void Analyze_MaintenanceLoopReject(PS_MaintenanceLoopReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaintenanceLoopReject");
    indent += 2;
    Analyze_MlRejectType(&x->mlRejectType, "mlRejectType", tag, indent);
    Analyze_MlRejectCause(&x->mlRejectCause, "mlRejectCause", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaintenanceLoopReject");
}

/* <=======================================> */
/*  PER-Analyzer for MlRejectCause (CHOICE)  */
/* <=======================================> */
void Analyze_MlRejectCause(PS_MlRejectCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MlRejectCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "canNotPerformLoop");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MlRejectCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MlRejectCause");
}

/* <======================================> */
/*  PER-Analyzer for MlRejectType (CHOICE)  */
/* <======================================> */
void Analyze_MlRejectType(PS_MlRejectType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MlRejectType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "systemLoop");
            break;
        case 1:
            ShowPERInteger(tag, indent, "mediaLoop", (uint32)x->mediaLoop);
            break;
        case 2:
            ShowPERInteger(tag, indent, "logicalChannelLoop", (uint32)x->logicalChannelLoop);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MlRejectType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MlRejectType");
}

/* <=====================================================> */
/*  PER-Analyzer for MaintenanceLoopOffCommand (SEQUENCE)  */
/* <=====================================================> */
void Analyze_MaintenanceLoopOffCommand(PS_MaintenanceLoopOffCommand x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "MaintenanceLoopOffCommand");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaintenanceLoopOffCommand");
}

/* <====================================================> */
/*  PER-Analyzer for CommunicationModeCommand (SEQUENCE)  */
/* <====================================================> */
void Analyze_CommunicationModeCommand(PS_CommunicationModeCommand x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "CommunicationModeCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_communicationModeTable", (uint32)x->size_of_communicationModeTable);
    for (i = 0;i < x->size_of_communicationModeTable;++i)
    {
        sprintf(tempLabelString, "communicationModeTable[%hu]", i);
        Analyze_CommunicationModeTableEntry(x->communicationModeTable + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CommunicationModeCommand");
}

/* <====================================================> */
/*  PER-Analyzer for CommunicationModeRequest (SEQUENCE)  */
/* <====================================================> */
void Analyze_CommunicationModeRequest(PS_CommunicationModeRequest x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "CommunicationModeRequest");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CommunicationModeRequest");
}

/* <===================================================> */
/*  PER-Analyzer for CommunicationModeResponse (CHOICE)  */
/* <===================================================> */
void Analyze_CommunicationModeResponse(PS_CommunicationModeResponse x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "CommunicationModeResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "communicationModeTable[%hu]", i);
                Analyze_CommunicationModeTableEntry(x->communicationModeTable + i, tempLabelString, tag, indent);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CommunicationModeResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CommunicationModeResponse");
}

/* <=======================================================> */
/*  PER-Analyzer for CommunicationModeTableEntry (SEQUENCE)  */
/* <=======================================================> */
void Analyze_CommunicationModeTableEntry(PS_CommunicationModeTableEntry x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "CommunicationModeTableEntry");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_nonStandard", x->option_of_nonStandard);
    ShowPERBoolean(tag, indent, "option_of_associatedSessionID", x->option_of_associatedSessionID);
    ShowPERBoolean(tag, indent, "option_of_terminalLabel", x->option_of_terminalLabel);
    ShowPERBoolean(tag, indent, "option_of_mediaChannel", x->option_of_mediaChannel);
    ShowPERBoolean(tag, indent, "option_of_mediaGuaranteedDelivery", x->option_of_mediaGuaranteedDelivery);
    ShowPERBoolean(tag, indent, "option_of_mediaControlChannel", x->option_of_mediaControlChannel);
    ShowPERBoolean(tag, indent, "option_of_mediaControlGuaranteedDelivery", x->option_of_mediaControlGuaranteedDelivery);
    if (x->option_of_nonStandard)
    {
        ShowPERInteger(tag, indent, "size_of_nonStandard", (uint32)x->size_of_nonStandard);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            sprintf(tempLabelString, "nonStandard[%hu]", i);
            Analyze_NonStandardParameter(x->nonStandard + i, tempLabelString, tag, indent);
        }
    }
    ShowPERInteger(tag, indent, "sessionID", (int32)x->sessionID);
    if (x->option_of_associatedSessionID)
    {
        ShowPERInteger(tag, indent, "associatedSessionID", (int32)x->associatedSessionID);
    }
    if (x->option_of_terminalLabel)
    {
        Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    }
    ShowPERCharString(tag, indent, "sessionDescription", &x->sessionDescription);
    Analyze_CmtDataType(&x->cmtDataType, "cmtDataType", tag, indent);
    if (x->option_of_mediaChannel)
    {
        Analyze_TransportAddress(&x->mediaChannel, "mediaChannel", tag, indent);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        ShowPERBoolean(tag, indent, "mediaGuaranteedDelivery", x->mediaGuaranteedDelivery);
    }
    if (x->option_of_mediaControlChannel)
    {
        Analyze_TransportAddress(&x->mediaControlChannel, "mediaControlChannel", tag, indent);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        ShowPERBoolean(tag, indent, "mediaControlGuaranteedDelivery", x->mediaControlGuaranteedDelivery);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_redundancyEncoding", x->option_of_redundancyEncoding);
    ShowPERBoolean(tag, indent, "option_of_sessionDependency", x->option_of_sessionDependency);
    ShowPERBoolean(tag, indent, "option_of_destination", x->option_of_destination);
    if (x->option_of_redundancyEncoding)
    {
        Analyze_RedundancyEncoding(&x->redundancyEncoding, "redundancyEncoding", tag, indent);
    }
    if (x->option_of_sessionDependency)
    {
        ShowPERInteger(tag, indent, "sessionDependency", (int32)x->sessionDependency);
    }
    if (x->option_of_destination)
    {
        Analyze_TerminalLabel(&x->destination, "destination", tag, indent);
    }
    ShowPERClosure(tag, indent, "CommunicationModeTableEntry");
}

/* <=====================================> */
/*  PER-Analyzer for CmtDataType (CHOICE)  */
/* <=====================================> */
void Analyze_CmtDataType(PS_CmtDataType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CmtDataType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_VideoCapability(x->videoData, "videoData", tag, indent);
            break;
        case 1:
            Analyze_AudioCapability(x->audioData, "audioData", tag, indent);
            break;
        case 2:
            Analyze_DataApplicationCapability(x->data, "data", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CmtDataType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CmtDataType");
}

/* <===========================================> */
/*  PER-Analyzer for ConferenceRequest (CHOICE)  */
/* <===========================================> */
void Analyze_ConferenceRequest(PS_ConferenceRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ConferenceRequest");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "terminalListRequest");
            break;
        case 1:
            ShowPERNull(tag, indent, "makeMeChair");
            break;
        case 2:
            ShowPERNull(tag, indent, "cancelMakeMeChair");
            break;
        case 3:
            Analyze_TerminalLabel(x->dropTerminal, "dropTerminal", tag, indent);
            break;
        case 4:
            Analyze_TerminalLabel(x->requestTerminalID, "requestTerminalID", tag, indent);
            break;
        case 5:
            ShowPERNull(tag, indent, "enterH243Password");
            break;
        case 6:
            ShowPERNull(tag, indent, "enterH243TerminalID");
            break;
        case 7:
            ShowPERNull(tag, indent, "enterH243ConferenceID");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            ShowPERNull(tag, indent, "enterExtensionAddress");
            break;
        case 9:
            ShowPERNull(tag, indent, "requestChairTokenOwner");
            break;
        case 10:
            Analyze_RequestTerminalCertificate(x->requestTerminalCertificate, "requestTerminalCertificate", tag, indent);
            break;
        case 11:
            ShowPERInteger(tag, indent, "broadcastMyLogicalChannel", (uint32)x->broadcastMyLogicalChannel);
            break;
        case 12:
            Analyze_TerminalLabel(x->makeTerminalBroadcaster, "makeTerminalBroadcaster", tag, indent);
            break;
        case 13:
            Analyze_TerminalLabel(x->sendThisSource, "sendThisSource", tag, indent);
            break;
        case 14:
            ShowPERNull(tag, indent, "requestAllTerminalIDs");
            break;
        case 15:
            Analyze_RemoteMCRequest(x->remoteMCRequest, "remoteMCRequest", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ConferenceRequest: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ConferenceRequest");
}

/* <======================================================> */
/*  PER-Analyzer for RequestTerminalCertificate (SEQUENCE)  */
/* <======================================================> */
void Analyze_RequestTerminalCertificate(PS_RequestTerminalCertificate x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RequestTerminalCertificate");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_terminalLabel", x->option_of_terminalLabel);
    ShowPERBoolean(tag, indent, "option_of_certSelectionCriteria", x->option_of_certSelectionCriteria);
    ShowPERBoolean(tag, indent, "option_of_sRandom", x->option_of_sRandom);
    if (x->option_of_terminalLabel)
    {
        Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    }
    if (x->option_of_certSelectionCriteria)
    {
        Analyze_CertSelectionCriteria(&x->certSelectionCriteria, "certSelectionCriteria", tag, indent);
    }
    if (x->option_of_sRandom)
    {
        ShowPERInteger(tag, indent, "sRandom", (int32)x->sRandom);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestTerminalCertificate");
}

/* <====================================================> */
/*  PER-Analyzer for CertSelectionCriteria (SEQUENCE-OF)  */
/* <====================================================> */
void Analyze_CertSelectionCriteria(PS_CertSelectionCriteria x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequenceof(tag, indent, label, "CertSelectionCriteria");
    indent += 2;
    ShowPERInteger(tag, indent, "size", (uint32)x->size);
    for (i = 0;i < x->size;++i)
    {
        sprintf(tempLabelString, "item[%hu]", i);
        Analyze_Criteria(x->item + i, tempLabelString, tag, indent);
    }
    ShowPERClosure(tag, indent, "CertSelectionCriteria");
}

/* <====================================> */
/*  PER-Analyzer for Criteria (SEQUENCE)  */
/* <====================================> */
void Analyze_Criteria(PS_Criteria x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Criteria");
    indent += 2;
    ShowPERObjectID(tag, indent, "field", &x->field);
    ShowPEROctetString(tag, indent, "value", &x->value);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Criteria");
}

/* <=========================================> */
/*  PER-Analyzer for TerminalLabel (SEQUENCE)  */
/* <=========================================> */
void Analyze_TerminalLabel(PS_TerminalLabel x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalLabel");
    indent += 2;
    ShowPERInteger(tag, indent, "mcuNumber", (int32)x->mcuNumber);
    ShowPERInteger(tag, indent, "terminalNumber", (int32)x->terminalNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalLabel");
}

/* <============================================> */
/*  PER-Analyzer for ConferenceResponse (CHOICE)  */
/* <============================================> */
void Analyze_ConferenceResponse(PS_ConferenceResponse x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "ConferenceResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_MCTerminalIDResponse(x->mCTerminalIDResponse, "mCTerminalIDResponse", tag, indent);
            break;
        case 1:
            Analyze_TerminalIDResponse(x->terminalIDResponse, "terminalIDResponse", tag, indent);
            break;
        case 2:
            Analyze_ConferenceIDResponse(x->conferenceIDResponse, "conferenceIDResponse", tag, indent);
            break;
        case 3:
            Analyze_PasswordResponse(x->passwordResponse, "passwordResponse", tag, indent);
            break;
        case 4:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "terminalListResponse[%hu]", i);
                Analyze_TerminalLabel(x->terminalListResponse + i, tempLabelString, tag, indent);
            }
            break;
        case 5:
            ShowPERNull(tag, indent, "videoCommandReject");
            break;
        case 6:
            ShowPERNull(tag, indent, "terminalDropReject");
            break;
        case 7:
            Analyze_MakeMeChairResponse(x->makeMeChairResponse, "makeMeChairResponse", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            Analyze_ExtensionAddressResponse(x->extensionAddressResponse, "extensionAddressResponse", tag, indent);
            break;
        case 9:
            Analyze_ChairTokenOwnerResponse(x->chairTokenOwnerResponse, "chairTokenOwnerResponse", tag, indent);
            break;
        case 10:
            Analyze_TerminalCertificateResponse(x->terminalCertificateResponse, "terminalCertificateResponse", tag, indent);
            break;
        case 11:
            Analyze_BroadcastMyLogicalChannelResponse(x->broadcastMyLogicalChannelResponse, "broadcastMyLogicalChannelResponse", tag, indent);
            break;
        case 12:
            Analyze_MakeTerminalBroadcasterResponse(x->makeTerminalBroadcasterResponse, "makeTerminalBroadcasterResponse", tag, indent);
            break;
        case 13:
            Analyze_SendThisSourceResponse(x->sendThisSourceResponse, "sendThisSourceResponse", tag, indent);
            break;
        case 14:
            Analyze_RequestAllTerminalIDsResponse(x->requestAllTerminalIDsResponse, "requestAllTerminalIDsResponse", tag, indent);
            break;
        case 15:
            Analyze_RemoteMCResponse(x->remoteMCResponse, "remoteMCResponse", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ConferenceResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ConferenceResponse");
}

/* <================================================> */
/*  PER-Analyzer for SendThisSourceResponse (CHOICE)  */
/* <================================================> */
void Analyze_SendThisSourceResponse(PS_SendThisSourceResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "SendThisSourceResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "grantedSendThisSource");
            break;
        case 1:
            ShowPERNull(tag, indent, "deniedSendThisSource");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_SendThisSourceResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "SendThisSourceResponse");
}

/* <=========================================================> */
/*  PER-Analyzer for MakeTerminalBroadcasterResponse (CHOICE)  */
/* <=========================================================> */
void Analyze_MakeTerminalBroadcasterResponse(PS_MakeTerminalBroadcasterResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MakeTerminalBroadcasterResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "grantedMakeTerminalBroadcaster");
            break;
        case 1:
            ShowPERNull(tag, indent, "deniedMakeTerminalBroadcaster");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MakeTerminalBroadcasterResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MakeTerminalBroadcasterResponse");
}

/* <===========================================================> */
/*  PER-Analyzer for BroadcastMyLogicalChannelResponse (CHOICE)  */
/* <===========================================================> */
void Analyze_BroadcastMyLogicalChannelResponse(PS_BroadcastMyLogicalChannelResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "BroadcastMyLogicalChannelResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "grantedBroadcastMyLogicalChannel");
            break;
        case 1:
            ShowPERNull(tag, indent, "deniedBroadcastMyLogicalChannel");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_BroadcastMyLogicalChannelResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "BroadcastMyLogicalChannelResponse");
}

/* <=======================================================> */
/*  PER-Analyzer for TerminalCertificateResponse (SEQUENCE)  */
/* <=======================================================> */
void Analyze_TerminalCertificateResponse(PS_TerminalCertificateResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalCertificateResponse");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_terminalLabel", x->option_of_terminalLabel);
    ShowPERBoolean(tag, indent, "option_of_certificateResponse", x->option_of_certificateResponse);
    if (x->option_of_terminalLabel)
    {
        Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    }
    if (x->option_of_certificateResponse)
    {
        ShowPEROctetString(tag, indent, "certificateResponse", &x->certificateResponse);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalCertificateResponse");
}

/* <===================================================> */
/*  PER-Analyzer for ChairTokenOwnerResponse (SEQUENCE)  */
/* <===================================================> */
void Analyze_ChairTokenOwnerResponse(PS_ChairTokenOwnerResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ChairTokenOwnerResponse");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "terminalID", &x->terminalID);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ChairTokenOwnerResponse");
}

/* <====================================================> */
/*  PER-Analyzer for ExtensionAddressResponse (SEQUENCE)  */
/* <====================================================> */
void Analyze_ExtensionAddressResponse(PS_ExtensionAddressResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ExtensionAddressResponse");
    indent += 2;
    ShowPEROctetString(tag, indent, "extensionAddress", &x->extensionAddress);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ExtensionAddressResponse");
}

/* <=============================================> */
/*  PER-Analyzer for MakeMeChairResponse (CHOICE)  */
/* <=============================================> */
void Analyze_MakeMeChairResponse(PS_MakeMeChairResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MakeMeChairResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "grantedChairToken");
            break;
        case 1:
            ShowPERNull(tag, indent, "deniedChairToken");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MakeMeChairResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MakeMeChairResponse");
}

/* <============================================> */
/*  PER-Analyzer for PasswordResponse (SEQUENCE)  */
/* <============================================> */
void Analyze_PasswordResponse(PS_PasswordResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "PasswordResponse");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "password", &x->password);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "PasswordResponse");
}

/* <================================================> */
/*  PER-Analyzer for ConferenceIDResponse (SEQUENCE)  */
/* <================================================> */
void Analyze_ConferenceIDResponse(PS_ConferenceIDResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ConferenceIDResponse");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "conferenceID", &x->conferenceID);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ConferenceIDResponse");
}

/* <==============================================> */
/*  PER-Analyzer for TerminalIDResponse (SEQUENCE)  */
/* <==============================================> */
void Analyze_TerminalIDResponse(PS_TerminalIDResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalIDResponse");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "terminalID", &x->terminalID);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalIDResponse");
}

/* <================================================> */
/*  PER-Analyzer for MCTerminalIDResponse (SEQUENCE)  */
/* <================================================> */
void Analyze_MCTerminalIDResponse(PS_MCTerminalIDResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MCTerminalIDResponse");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "terminalID", &x->terminalID);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MCTerminalIDResponse");
}

/* <=========================================================> */
/*  PER-Analyzer for RequestAllTerminalIDsResponse (SEQUENCE)  */
/* <=========================================================> */
void Analyze_RequestAllTerminalIDsResponse(PS_RequestAllTerminalIDsResponse x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "RequestAllTerminalIDsResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "size_of_terminalInformation", (uint32)x->size_of_terminalInformation);
    for (i = 0;i < x->size_of_terminalInformation;++i)
    {
        sprintf(tempLabelString, "terminalInformation[%hu]", i);
        Analyze_TerminalInformation(x->terminalInformation + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RequestAllTerminalIDsResponse");
}

/* <===============================================> */
/*  PER-Analyzer for TerminalInformation (SEQUENCE)  */
/* <===============================================> */
void Analyze_TerminalInformation(PS_TerminalInformation x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalInformation");
    indent += 2;
    Analyze_TerminalLabel(&x->terminalLabel, "terminalLabel", tag, indent);
    ShowPEROctetString(tag, indent, "terminalID", &x->terminalID);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalInformation");
}

/* <=========================================> */
/*  PER-Analyzer for RemoteMCRequest (CHOICE)  */
/* <=========================================> */
void Analyze_RemoteMCRequest(PS_RemoteMCRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RemoteMCRequest");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "masterActivate");
            break;
        case 1:
            ShowPERNull(tag, indent, "slaveActivate");
            break;
        case 2:
            ShowPERNull(tag, indent, "deActivate");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RemoteMCRequest: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RemoteMCRequest");
}

/* <==========================================> */
/*  PER-Analyzer for RemoteMCResponse (CHOICE)  */
/* <==========================================> */
void Analyze_RemoteMCResponse(PS_RemoteMCResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RemoteMCResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "accept");
            break;
        case 1:
            Analyze_Reject(x->reject, "reject", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RemoteMCResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RemoteMCResponse");
}

/* <================================> */
/*  PER-Analyzer for Reject (CHOICE)  */
/* <================================> */
void Analyze_Reject(PS_Reject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Reject");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "unspecified");
            break;
        case 1:
            ShowPERNull(tag, indent, "functionNotSupported");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Reject: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Reject");
}

/* <==========================================> */
/*  PER-Analyzer for MultilinkRequest (CHOICE)  */
/* <==========================================> */
void Analyze_MultilinkRequest(PS_MultilinkRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultilinkRequest");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_CallInformation(x->callInformation, "callInformation", tag, indent);
            break;
        case 2:
            Analyze_AddConnection(x->addConnection, "addConnection", tag, indent);
            break;
        case 3:
            Analyze_RemoveConnection(x->removeConnection, "removeConnection", tag, indent);
            break;
        case 4:
            Analyze_MaximumHeaderInterval(x->maximumHeaderInterval, "maximumHeaderInterval", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MultilinkRequest: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultilinkRequest");
}

/* <=================================================> */
/*  PER-Analyzer for MaximumHeaderInterval (SEQUENCE)  */
/* <=================================================> */
void Analyze_MaximumHeaderInterval(PS_MaximumHeaderInterval x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MaximumHeaderInterval");
    indent += 2;
    Analyze_RequestType(&x->requestType, "requestType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MaximumHeaderInterval");
}

/* <=====================================> */
/*  PER-Analyzer for RequestType (CHOICE)  */
/* <=====================================> */
void Analyze_RequestType(PS_RequestType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "RequestType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "currentIntervalInformation");
            break;
        case 1:
            ShowPERInteger(tag, indent, "requestedInterval", (uint32)x->requestedInterval);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_RequestType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "RequestType");
}

/* <============================================> */
/*  PER-Analyzer for RemoveConnection (SEQUENCE)  */
/* <============================================> */
void Analyze_RemoveConnection(PS_RemoveConnection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RemoveConnection");
    indent += 2;
    Analyze_ConnectionIdentifier(&x->connectionIdentifier, "connectionIdentifier", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RemoveConnection");
}

/* <=========================================> */
/*  PER-Analyzer for AddConnection (SEQUENCE)  */
/* <=========================================> */
void Analyze_AddConnection(PS_AddConnection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "AddConnection");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    Analyze_DialingInformation(&x->dialingInformation, "dialingInformation", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "AddConnection");
}

/* <===========================================> */
/*  PER-Analyzer for CallInformation (SEQUENCE)  */
/* <===========================================> */
void Analyze_CallInformation(PS_CallInformation x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CallInformation");
    indent += 2;
    ShowPERInteger(tag, indent, "maxNumberOfAdditionalConnections", (int32)x->maxNumberOfAdditionalConnections);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CallInformation");
}

/* <===========================================> */
/*  PER-Analyzer for MultilinkResponse (CHOICE)  */
/* <===========================================> */
void Analyze_MultilinkResponse(PS_MultilinkResponse x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultilinkResponse");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_RespCallInformation(x->respCallInformation, "respCallInformation", tag, indent);
            break;
        case 2:
            Analyze_RespAddConnection(x->respAddConnection, "respAddConnection", tag, indent);
            break;
        case 3:
            Analyze_RespRemoveConnection(x->respRemoveConnection, "respRemoveConnection", tag, indent);
            break;
        case 4:
            Analyze_RespMaximumHeaderInterval(x->respMaximumHeaderInterval, "respMaximumHeaderInterval", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MultilinkResponse: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultilinkResponse");
}

/* <=====================================================> */
/*  PER-Analyzer for RespMaximumHeaderInterval (SEQUENCE)  */
/* <=====================================================> */
void Analyze_RespMaximumHeaderInterval(PS_RespMaximumHeaderInterval x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RespMaximumHeaderInterval");
    indent += 2;
    ShowPERInteger(tag, indent, "currentInterval", (int32)x->currentInterval);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RespMaximumHeaderInterval");
}

/* <================================================> */
/*  PER-Analyzer for RespRemoveConnection (SEQUENCE)  */
/* <================================================> */
void Analyze_RespRemoveConnection(PS_RespRemoveConnection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RespRemoveConnection");
    indent += 2;
    Analyze_ConnectionIdentifier(&x->connectionIdentifier, "connectionIdentifier", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RespRemoveConnection");
}

/* <=============================================> */
/*  PER-Analyzer for RespAddConnection (SEQUENCE)  */
/* <=============================================> */
void Analyze_RespAddConnection(PS_RespAddConnection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RespAddConnection");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    Analyze_ResponseCode(&x->responseCode, "responseCode", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RespAddConnection");
}

/* <======================================> */
/*  PER-Analyzer for ResponseCode (CHOICE)  */
/* <======================================> */
void Analyze_ResponseCode(PS_ResponseCode x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ResponseCode");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "accepted");
            break;
        case 1:
            Analyze_Rejected(x->rejected, "rejected", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_ResponseCode: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ResponseCode");
}

/* <==================================> */
/*  PER-Analyzer for Rejected (CHOICE)  */
/* <==================================> */
void Analyze_Rejected(PS_Rejected x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Rejected");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "connectionsNotAvailable");
            break;
        case 1:
            ShowPERNull(tag, indent, "userRejected");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Rejected: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Rejected");
}

/* <===============================================> */
/*  PER-Analyzer for RespCallInformation (SEQUENCE)  */
/* <===============================================> */
void Analyze_RespCallInformation(PS_RespCallInformation x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "RespCallInformation");
    indent += 2;
    Analyze_DialingInformation(&x->dialingInformation, "dialingInformation", tag, indent);
    ShowPERInteger(tag, indent, "callAssociationNumber", (int32)x->callAssociationNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "RespCallInformation");
}

/* <=============================================> */
/*  PER-Analyzer for MultilinkIndication (CHOICE)  */
/* <=============================================> */
void Analyze_MultilinkIndication(PS_MultilinkIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MultilinkIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            Analyze_CrcDesired(x->crcDesired, "crcDesired", tag, indent);
            break;
        case 2:
            Analyze_ExcessiveError(x->excessiveError, "excessiveError", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_MultilinkIndication: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MultilinkIndication");
}

/* <==========================================> */
/*  PER-Analyzer for ExcessiveError (SEQUENCE)  */
/* <==========================================> */
void Analyze_ExcessiveError(PS_ExcessiveError x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ExcessiveError");
    indent += 2;
    Analyze_ConnectionIdentifier(&x->connectionIdentifier, "connectionIdentifier", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ExcessiveError");
}

/* <======================================> */
/*  PER-Analyzer for CrcDesired (SEQUENCE)  */
/* <======================================> */
void Analyze_CrcDesired(PS_CrcDesired x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "CrcDesired");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CrcDesired");
}

/* <============================================> */
/*  PER-Analyzer for DialingInformation (CHOICE)  */
/* <============================================> */
void Analyze_DialingInformation(PS_DialingInformation x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERChoice(tag, indent, label, "DialingInformation");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERInteger(tag, indent, "size", (uint32)x->size);
            for (i = 0;i < x->size;++i)
            {
                sprintf(tempLabelString, "differential[%hu]", i);
                Analyze_DialingInformationNumber(x->differential + i, tempLabelString, tag, indent);
            }
            break;
        case 2:
            ShowPERInteger(tag, indent, "infoNotAvailable", (uint32)x->infoNotAvailable);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_DialingInformation: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "DialingInformation");
}

/* <====================================================> */
/*  PER-Analyzer for DialingInformationNumber (SEQUENCE)  */
/* <====================================================> */
void Analyze_DialingInformationNumber(PS_DialingInformationNumber x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
    char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "DialingInformationNumber");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_subAddress", x->option_of_subAddress);
    ShowPERCharString(tag, indent, "networkAddress", &x->networkAddress);
    if (x->option_of_subAddress)
    {
        ShowPERCharString(tag, indent, "subAddress", &x->subAddress);
    }
    ShowPERInteger(tag, indent, "size_of_networkType", (uint32)x->size_of_networkType);
    for (i = 0;i < x->size_of_networkType;++i)
    {
        sprintf(tempLabelString, "networkType[%hu]", i);
        Analyze_DialingInformationNetworkType(x->networkType + i, tempLabelString, tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "DialingInformationNumber");
}

/* <=======================================================> */
/*  PER-Analyzer for DialingInformationNetworkType (CHOICE)  */
/* <=======================================================> */
void Analyze_DialingInformationNetworkType(PS_DialingInformationNetworkType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "DialingInformationNetworkType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardMessage(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "n_isdn");
            break;
        case 2:
            ShowPERNull(tag, indent, "gstn");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_DialingInformationNetworkType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "DialingInformationNetworkType");
}

/* <================================================> */
/*  PER-Analyzer for ConnectionIdentifier (SEQUENCE)  */
/* <================================================> */
void Analyze_ConnectionIdentifier(PS_ConnectionIdentifier x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ConnectionIdentifier");
    indent += 2;
    ShowPERInteger(tag, indent, "channelTag", (int32)x->channelTag);
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ConnectionIdentifier");
}

/* <=====================================================> */
/*  PER-Analyzer for LogicalChannelRateRequest (SEQUENCE)  */
/* <=====================================================> */
void Analyze_LogicalChannelRateRequest(PS_LogicalChannelRateRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "LogicalChannelRateRequest");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    ShowPERInteger(tag, indent, "maximumBitRate", (int32)x->maximumBitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "LogicalChannelRateRequest");
}

/* <=========================================================> */
/*  PER-Analyzer for LogicalChannelRateAcknowledge (SEQUENCE)  */
/* <=========================================================> */
void Analyze_LogicalChannelRateAcknowledge(PS_LogicalChannelRateAcknowledge x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "LogicalChannelRateAcknowledge");
    indent += 2;
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    ShowPERInteger(tag, indent, "maximumBitRate", (int32)x->maximumBitRate);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "LogicalChannelRateAcknowledge");
}

/* <====================================================> */
/*  PER-Analyzer for LogicalChannelRateReject (SEQUENCE)  */
/* <====================================================> */
void Analyze_LogicalChannelRateReject(PS_LogicalChannelRateReject x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "LogicalChannelRateReject");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_currentMaximumBitRate", x->option_of_currentMaximumBitRate);
    ShowPERInteger(tag, indent, "sequenceNumber", (int32)x->sequenceNumber);
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    Analyze_LogicalChannelRateRejectReason(&x->rejectReason, "rejectReason", tag, indent);
    if (x->option_of_currentMaximumBitRate)
    {
        ShowPERInteger(tag, indent, "currentMaximumBitRate", (int32)x->currentMaximumBitRate);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "LogicalChannelRateReject");
}

/* <========================================================> */
/*  PER-Analyzer for LogicalChannelRateRejectReason (CHOICE)  */
/* <========================================================> */
void Analyze_LogicalChannelRateRejectReason(PS_LogicalChannelRateRejectReason x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "LogicalChannelRateRejectReason");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "undefinedReason");
            break;
        case 1:
            ShowPERNull(tag, indent, "insufficientResources");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_LogicalChannelRateRejectReason: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "LogicalChannelRateRejectReason");
}

/* <=====================================================> */
/*  PER-Analyzer for LogicalChannelRateRelease (SEQUENCE)  */
/* <=====================================================> */
void Analyze_LogicalChannelRateRelease(PS_LogicalChannelRateRelease x, const char* label, uint16 tag, uint16 indent)
{
    OSCL_UNUSED_ARG(x);

    ShowPERSequence(tag, indent, label, "LogicalChannelRateRelease");
    indent += 2;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "LogicalChannelRateRelease");
}

/* <===================================================> */
/*  PER-Analyzer for SendTerminalCapabilitySet (CHOICE)  */
/* <===================================================> */
void Analyze_SendTerminalCapabilitySet(PS_SendTerminalCapabilitySet x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "SendTerminalCapabilitySet");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_SpecificRequest(x->specificRequest, "specificRequest", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "genericRequest");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_SendTerminalCapabilitySet: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "SendTerminalCapabilitySet");
}

/* <===========================================> */
/*  PER-Analyzer for SpecificRequest (SEQUENCE)  */
/* <===========================================> */
void Analyze_SpecificRequest(PS_SpecificRequest x, const char* label, uint16 tag, uint16 indent)
{
    uint16 i;
//  char tempLabelString[100];
    ShowPERSequence(tag, indent, label, "SpecificRequest");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_capabilityTableEntryNumbers", x->option_of_capabilityTableEntryNumbers);
    ShowPERBoolean(tag, indent, "option_of_capabilityDescriptorNumbers", x->option_of_capabilityDescriptorNumbers);
    ShowPERBoolean(tag, indent, "multiplexCapability", x->multiplexCapability);
    if (x->option_of_capabilityTableEntryNumbers)
    {
        ShowPERInteger(tag, indent, "size_of_capabilityTableEntryNumbers", (uint32)x->size_of_capabilityTableEntryNumbers);
        for (i = 0;i < x->size_of_capabilityTableEntryNumbers;++i)
        {
            ShowPERIntegers(tag, indent, "capabilityTableEntryNumbers", (uint32)x->capabilityTableEntryNumbers[i], i);
        }
    }
    if (x->option_of_capabilityDescriptorNumbers)
    {
        ShowPERInteger(tag, indent, "size_of_capabilityDescriptorNumbers", (uint32)x->size_of_capabilityDescriptorNumbers);
        for (i = 0;i < x->size_of_capabilityDescriptorNumbers;++i)
        {
            ShowPERIntegers(tag, indent, "capabilityDescriptorNumbers", (uint32)x->capabilityDescriptorNumbers[i], i);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "SpecificRequest");
}

/* <===========================================> */
/*  PER-Analyzer for EncryptionCommand (CHOICE)  */
/* <===========================================> */
void Analyze_EncryptionCommand(PS_EncryptionCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "EncryptionCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPEROctetString(tag, indent, "encryptionSE", x->encryptionSE);
            break;
        case 1:
            ShowPERNull(tag, indent, "encryptionIVRequest");
            break;
        case 2:
            Analyze_EncryptionAlgorithmID(x->encryptionAlgorithmID, "encryptionAlgorithmID", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_EncryptionCommand: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "EncryptionCommand");
}

/* <=================================================> */
/*  PER-Analyzer for EncryptionAlgorithmID (SEQUENCE)  */
/* <=================================================> */
void Analyze_EncryptionAlgorithmID(PS_EncryptionAlgorithmID x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "EncryptionAlgorithmID");
    indent += 2;
    ShowPERInteger(tag, indent, "h233AlgorithmIdentifier", (int32)x->h233AlgorithmIdentifier);
    Analyze_NonStandardParameter(&x->associatedAlgorithm, "associatedAlgorithm", tag, indent);
    ShowPERClosure(tag, indent, "EncryptionAlgorithmID");
}

/* <==============================================> */
/*  PER-Analyzer for FlowControlCommand (SEQUENCE)  */
/* <==============================================> */
void Analyze_FlowControlCommand(PS_FlowControlCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "FlowControlCommand");
    indent += 2;
    Analyze_FccScope(&x->fccScope, "fccScope", tag, indent);
    Analyze_FccRestriction(&x->fccRestriction, "fccRestriction", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "FlowControlCommand");
}

/* <========================================> */
/*  PER-Analyzer for FccRestriction (CHOICE)  */
/* <========================================> */
void Analyze_FccRestriction(PS_FccRestriction x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FccRestriction");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "maximumBitRate", (uint32)x->maximumBitRate);
            break;
        case 1:
            ShowPERNull(tag, indent, "noRestriction");
            break;
        default:
            ErrorMessage("Analyze_FccRestriction: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FccRestriction");
}

/* <==================================> */
/*  PER-Analyzer for FccScope (CHOICE)  */
/* <==================================> */
void Analyze_FccScope(PS_FccScope x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FccScope");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "logicalChannelNumber", (uint32)x->logicalChannelNumber);
            break;
        case 1:
            ShowPERInteger(tag, indent, "resourceID", (uint32)x->resourceID);
            break;
        case 2:
            ShowPERNull(tag, indent, "wholeMultiplex");
            break;
        default:
            ErrorMessage("Analyze_FccScope: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FccScope");
}

/* <===========================================> */
/*  PER-Analyzer for EndSessionCommand (CHOICE)  */
/* <===========================================> */
void Analyze_EndSessionCommand(PS_EndSessionCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "EndSessionCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "disconnect");
            break;
        case 2:
            Analyze_GstnOptions(x->gstnOptions, "gstnOptions", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            Analyze_IsdnOptions(x->isdnOptions, "isdnOptions", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_EndSessionCommand: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "EndSessionCommand");
}

/* <=====================================> */
/*  PER-Analyzer for IsdnOptions (CHOICE)  */
/* <=====================================> */
void Analyze_IsdnOptions(PS_IsdnOptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IsdnOptions");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "telephonyMode");
            break;
        case 1:
            ShowPERNull(tag, indent, "v140");
            break;
        case 2:
            ShowPERNull(tag, indent, "terminalOnHold");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IsdnOptions: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IsdnOptions");
}

/* <=====================================> */
/*  PER-Analyzer for GstnOptions (CHOICE)  */
/* <=====================================> */
void Analyze_GstnOptions(PS_GstnOptions x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "GstnOptions");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "telephonyMode");
            break;
        case 1:
            ShowPERNull(tag, indent, "v8bis");
            break;
        case 2:
            ShowPERNull(tag, indent, "v34DSVD");
            break;
        case 3:
            ShowPERNull(tag, indent, "v34DuplexFAX");
            break;
        case 4:
            ShowPERNull(tag, indent, "v34H324");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_GstnOptions: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "GstnOptions");
}

/* <===========================================> */
/*  PER-Analyzer for ConferenceCommand (CHOICE)  */
/* <===========================================> */
void Analyze_ConferenceCommand(PS_ConferenceCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ConferenceCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "broadcastMyLogicalChannel", (uint32)x->broadcastMyLogicalChannel);
            break;
        case 1:
            ShowPERInteger(tag, indent, "cancelBroadcastMyLogicalChannel", (uint32)x->cancelBroadcastMyLogicalChannel);
            break;
        case 2:
            Analyze_TerminalLabel(x->makeTerminalBroadcaster, "makeTerminalBroadcaster", tag, indent);
            break;
        case 3:
            ShowPERNull(tag, indent, "cancelMakeTerminalBroadcaster");
            break;
        case 4:
            Analyze_TerminalLabel(x->sendThisSource, "sendThisSource", tag, indent);
            break;
        case 5:
            ShowPERNull(tag, indent, "cancelSendThisSource");
            break;
        case 6:
            ShowPERNull(tag, indent, "dropConference");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            Analyze_SubstituteConferenceIDCommand(x->substituteConferenceIDCommand, "substituteConferenceIDCommand", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ConferenceCommand: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ConferenceCommand");
}

/* <=========================================================> */
/*  PER-Analyzer for SubstituteConferenceIDCommand (SEQUENCE)  */
/* <=========================================================> */
void Analyze_SubstituteConferenceIDCommand(PS_SubstituteConferenceIDCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "SubstituteConferenceIDCommand");
    indent += 2;
    ShowPEROctetString(tag, indent, "conferenceIdentifier", &x->conferenceIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "SubstituteConferenceIDCommand");
}

/* <================================================> */
/*  PER-Analyzer for MiscellaneousCommand (SEQUENCE)  */
/* <================================================> */
void Analyze_MiscellaneousCommand(PS_MiscellaneousCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MiscellaneousCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    Analyze_McType(&x->mcType, "mcType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MiscellaneousCommand");
}

/* <================================> */
/*  PER-Analyzer for McType (CHOICE)  */
/* <================================> */
void Analyze_McType(PS_McType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "McType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "equaliseDelay");
            break;
        case 1:
            ShowPERNull(tag, indent, "zeroDelay");
            break;
        case 2:
            ShowPERNull(tag, indent, "multipointModeCommand");
            break;
        case 3:
            ShowPERNull(tag, indent, "cancelMultipointModeCommand");
            break;
        case 4:
            ShowPERNull(tag, indent, "videoFreezePicture");
            break;
        case 5:
            ShowPERNull(tag, indent, "videoFastUpdatePicture");
            break;
        case 6:
            Analyze_VideoFastUpdateGOB(x->videoFastUpdateGOB, "videoFastUpdateGOB", tag, indent);
            break;
        case 7:
            ShowPERInteger(tag, indent, "videoTemporalSpatialTradeOff", (uint32)x->videoTemporalSpatialTradeOff);
            break;
        case 8:
            ShowPERNull(tag, indent, "videoSendSyncEveryGOB");
            break;
        case 9:
            ShowPERNull(tag, indent, "videoSendSyncEveryGOBCancel");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Analyze_VideoFastUpdateMB(x->videoFastUpdateMB, "videoFastUpdateMB", tag, indent);
            break;
        case 11:
            ShowPERInteger(tag, indent, "maxH223MUXPDUsize", (uint32)x->maxH223MUXPDUsize);
            break;
        case 12:
            Analyze_EncryptionSync(x->encryptionUpdate, "encryptionUpdate", tag, indent);
            break;
        case 13:
            Analyze_EncryptionUpdateRequest(x->encryptionUpdateRequest, "encryptionUpdateRequest", tag, indent);
            break;
        case 14:
            ShowPERNull(tag, indent, "switchReceiveMediaOff");
            break;
        case 15:
            ShowPERNull(tag, indent, "switchReceiveMediaOn");
            break;
        case 16:
            Analyze_ProgressiveRefinementStart(x->progressiveRefinementStart, "progressiveRefinementStart", tag, indent);
            break;
        case 17:
            ShowPERNull(tag, indent, "progressiveRefinementAbortOne");
            break;
        case 18:
            ShowPERNull(tag, indent, "progressiveRefinementAbortContinuous");
            break;
        default:
            ErrorMessage("Analyze_McType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "McType");
}

/* <======================================================> */
/*  PER-Analyzer for ProgressiveRefinementStart (SEQUENCE)  */
/* <======================================================> */
void Analyze_ProgressiveRefinementStart(PS_ProgressiveRefinementStart x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "ProgressiveRefinementStart");
    indent += 2;
    Analyze_PrsRepeatCount(&x->prsRepeatCount, "prsRepeatCount", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "ProgressiveRefinementStart");
}

/* <========================================> */
/*  PER-Analyzer for PrsRepeatCount (CHOICE)  */
/* <========================================> */
void Analyze_PrsRepeatCount(PS_PrsRepeatCount x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "PrsRepeatCount");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "doOneProgression");
            break;
        case 1:
            ShowPERNull(tag, indent, "doContinuousProgressions");
            break;
        case 2:
            ShowPERNull(tag, indent, "doOneIndependentProgression");
            break;
        case 3:
            ShowPERNull(tag, indent, "doContinuousIndependentProgressions");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_PrsRepeatCount: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "PrsRepeatCount");
}

/* <=============================================> */
/*  PER-Analyzer for VideoFastUpdateMB (SEQUENCE)  */
/* <=============================================> */
void Analyze_VideoFastUpdateMB(PS_VideoFastUpdateMB x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VideoFastUpdateMB");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_firstGOB", x->option_of_firstGOB);
    ShowPERBoolean(tag, indent, "option_of_firstMB", x->option_of_firstMB);
    if (x->option_of_firstGOB)
    {
        ShowPERInteger(tag, indent, "firstGOB", (int32)x->firstGOB);
    }
    if (x->option_of_firstMB)
    {
        ShowPERInteger(tag, indent, "firstMB", (int32)x->firstMB);
    }
    ShowPERInteger(tag, indent, "numberOfMBs", (int32)x->numberOfMBs);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VideoFastUpdateMB");
}

/* <==============================================> */
/*  PER-Analyzer for VideoFastUpdateGOB (SEQUENCE)  */
/* <==============================================> */
void Analyze_VideoFastUpdateGOB(PS_VideoFastUpdateGOB x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VideoFastUpdateGOB");
    indent += 2;
    ShowPERInteger(tag, indent, "firstGOB", (int32)x->firstGOB);
    ShowPERInteger(tag, indent, "numberOfGOBs", (int32)x->numberOfGOBs);
    ShowPERClosure(tag, indent, "VideoFastUpdateGOB");
}

/* <===============================================> */
/*  PER-Analyzer for KeyProtectionMethod (SEQUENCE)  */
/* <===============================================> */
void Analyze_KeyProtectionMethod(PS_KeyProtectionMethod x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "KeyProtectionMethod");
    indent += 2;
    ShowPERBoolean(tag, indent, "secureChannel", x->secureChannel);
    ShowPERBoolean(tag, indent, "sharedSecret", x->sharedSecret);
    ShowPERBoolean(tag, indent, "certProtectedKey", x->certProtectedKey);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "KeyProtectionMethod");
}

/* <===================================================> */
/*  PER-Analyzer for EncryptionUpdateRequest (SEQUENCE)  */
/* <===================================================> */
void Analyze_EncryptionUpdateRequest(PS_EncryptionUpdateRequest x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "EncryptionUpdateRequest");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_keyProtectionMethod", x->option_of_keyProtectionMethod);
    if (x->option_of_keyProtectionMethod)
    {
        Analyze_KeyProtectionMethod(&x->keyProtectionMethod, "keyProtectionMethod", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "EncryptionUpdateRequest");
}

/* <======================================================> */
/*  PER-Analyzer for H223MultiplexReconfiguration (CHOICE)  */
/* <======================================================> */
void Analyze_H223MultiplexReconfiguration(PS_H223MultiplexReconfiguration x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "H223MultiplexReconfiguration");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_H223ModeChange(x->h223ModeChange, "h223ModeChange", tag, indent);
            break;
        case 1:
            Analyze_H223AnnexADoubleFlag(x->h223AnnexADoubleFlag, "h223AnnexADoubleFlag", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_H223MultiplexReconfiguration: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "H223MultiplexReconfiguration");
}

/* <==============================================> */
/*  PER-Analyzer for H223AnnexADoubleFlag (CHOICE)  */
/* <==============================================> */
void Analyze_H223AnnexADoubleFlag(PS_H223AnnexADoubleFlag x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "H223AnnexADoubleFlag");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "start");
            break;
        case 1:
            ShowPERNull(tag, indent, "stop");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_H223AnnexADoubleFlag: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "H223AnnexADoubleFlag");
}

/* <========================================> */
/*  PER-Analyzer for H223ModeChange (CHOICE)  */
/* <========================================> */
void Analyze_H223ModeChange(PS_H223ModeChange x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "H223ModeChange");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "toLevel0");
            break;
        case 1:
            ShowPERNull(tag, indent, "toLevel1");
            break;
        case 2:
            ShowPERNull(tag, indent, "toLevel2");
            break;
        case 3:
            ShowPERNull(tag, indent, "toLevel2withOptionalHeader");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_H223ModeChange: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "H223ModeChange");
}

/* <===========================================> */
/*  PER-Analyzer for NewATMVCCommand (SEQUENCE)  */
/* <===========================================> */
void Analyze_NewATMVCCommand(PS_NewATMVCCommand x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "NewATMVCCommand");
    indent += 2;
    ShowPERInteger(tag, indent, "resourceID", (int32)x->resourceID);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "bitRateLockedToPCRClock", x->bitRateLockedToPCRClock);
    ShowPERBoolean(tag, indent, "bitRateLockedToNetworkClock", x->bitRateLockedToNetworkClock);
    Analyze_CmdAal(&x->cmdAal, "cmdAal", tag, indent);
    Analyze_CmdMultiplex(&x->cmdMultiplex, "cmdMultiplex", tag, indent);
    Analyze_CmdReverseParameters(&x->cmdReverseParameters, "cmdReverseParameters", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "NewATMVCCommand");
}

/* <================================================> */
/*  PER-Analyzer for CmdReverseParameters (SEQUENCE)  */
/* <================================================> */
void Analyze_CmdReverseParameters(PS_CmdReverseParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CmdReverseParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "bitRateLockedToPCRClock", x->bitRateLockedToPCRClock);
    ShowPERBoolean(tag, indent, "bitRateLockedToNetworkClock", x->bitRateLockedToNetworkClock);
    Analyze_Multiplex(&x->multiplex, "multiplex", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CmdReverseParameters");
}

/* <===================================> */
/*  PER-Analyzer for Multiplex (CHOICE)  */
/* <===================================> */
void Analyze_Multiplex(PS_Multiplex x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "Multiplex");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noMultiplex");
            break;
        case 1:
            ShowPERNull(tag, indent, "transportStream");
            break;
        case 2:
            ShowPERNull(tag, indent, "programStream");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_Multiplex: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "Multiplex");
}

/* <======================================> */
/*  PER-Analyzer for CmdMultiplex (CHOICE)  */
/* <======================================> */
void Analyze_CmdMultiplex(PS_CmdMultiplex x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CmdMultiplex");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noMultiplex");
            break;
        case 1:
            ShowPERNull(tag, indent, "transportStream");
            break;
        case 2:
            ShowPERNull(tag, indent, "programStream");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CmdMultiplex: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CmdMultiplex");
}

/* <================================> */
/*  PER-Analyzer for CmdAal (CHOICE)  */
/* <================================> */
void Analyze_CmdAal(PS_CmdAal x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CmdAal");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_CmdAal1(x->cmdAal1, "cmdAal1", tag, indent);
            break;
        case 1:
            Analyze_CmdAal5(x->cmdAal5, "cmdAal5", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CmdAal: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CmdAal");
}

/* <===================================> */
/*  PER-Analyzer for CmdAal5 (SEQUENCE)  */
/* <===================================> */
void Analyze_CmdAal5(PS_CmdAal5 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CmdAal5");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardMaximumSDUSize", (int32)x->forwardMaximumSDUSize);
    ShowPERInteger(tag, indent, "backwardMaximumSDUSize", (int32)x->backwardMaximumSDUSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CmdAal5");
}

/* <===================================> */
/*  PER-Analyzer for CmdAal1 (SEQUENCE)  */
/* <===================================> */
void Analyze_CmdAal1(PS_CmdAal1 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "CmdAal1");
    indent += 2;
    Analyze_CmdClockRecovery(&x->cmdClockRecovery, "cmdClockRecovery", tag, indent);
    Analyze_CmdErrorCorrection(&x->cmdErrorCorrection, "cmdErrorCorrection", tag, indent);
    ShowPERBoolean(tag, indent, "structuredDataTransfer", x->structuredDataTransfer);
    ShowPERBoolean(tag, indent, "partiallyFilledCells", x->partiallyFilledCells);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "CmdAal1");
}

/* <============================================> */
/*  PER-Analyzer for CmdErrorCorrection (CHOICE)  */
/* <============================================> */
void Analyze_CmdErrorCorrection(PS_CmdErrorCorrection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CmdErrorCorrection");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "nullErrorCorrection");
            break;
        case 1:
            ShowPERNull(tag, indent, "longInterleaver");
            break;
        case 2:
            ShowPERNull(tag, indent, "shortInterleaver");
            break;
        case 3:
            ShowPERNull(tag, indent, "errorCorrectionOnly");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CmdErrorCorrection: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CmdErrorCorrection");
}

/* <==========================================> */
/*  PER-Analyzer for CmdClockRecovery (CHOICE)  */
/* <==========================================> */
void Analyze_CmdClockRecovery(PS_CmdClockRecovery x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "CmdClockRecovery");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "nullClockRecovery");
            break;
        case 1:
            ShowPERNull(tag, indent, "srtsClockRecovery");
            break;
        case 2:
            ShowPERNull(tag, indent, "adaptiveClockRecovery");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_CmdClockRecovery: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "CmdClockRecovery");
}

/* <===============================================> */
/*  PER-Analyzer for FunctionNotUnderstood (CHOICE)  */
/* <===============================================> */
void Analyze_FunctionNotUnderstood(PS_FunctionNotUnderstood x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FunctionNotUnderstood");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_RequestMessage(x->request, "request", tag, indent);
            break;
        case 1:
            Analyze_ResponseMessage(x->response, "response", tag, indent);
            break;
        case 2:
            Analyze_CommandMessage(x->command, "command", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_FunctionNotUnderstood: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FunctionNotUnderstood");
}

/* <================================================> */
/*  PER-Analyzer for FunctionNotSupported (SEQUENCE)  */
/* <================================================> */
void Analyze_FunctionNotSupported(PS_FunctionNotSupported x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "FunctionNotSupported");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_returnedFunction", x->option_of_returnedFunction);
    Analyze_FnsCause(&x->fnsCause, "fnsCause", tag, indent);
    if (x->option_of_returnedFunction)
    {
        ShowPEROctetString(tag, indent, "returnedFunction", &x->returnedFunction);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "FunctionNotSupported");
}

/* <==================================> */
/*  PER-Analyzer for FnsCause (CHOICE)  */
/* <==================================> */
void Analyze_FnsCause(PS_FnsCause x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FnsCause");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "syntaxError");
            break;
        case 1:
            ShowPERNull(tag, indent, "semanticError");
            break;
        case 2:
            ShowPERNull(tag, indent, "unknownFunction");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_FnsCause: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FnsCause");
}

/* <==============================================> */
/*  PER-Analyzer for ConferenceIndication (CHOICE)  */
/* <==============================================> */
void Analyze_ConferenceIndication(PS_ConferenceIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "ConferenceIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "sbeNumber", (uint32)x->sbeNumber);
            break;
        case 1:
            Analyze_TerminalLabel(x->terminalNumberAssign, "terminalNumberAssign", tag, indent);
            break;
        case 2:
            Analyze_TerminalLabel(x->terminalJoinedConference, "terminalJoinedConference", tag, indent);
            break;
        case 3:
            Analyze_TerminalLabel(x->terminalLeftConference, "terminalLeftConference", tag, indent);
            break;
        case 4:
            ShowPERNull(tag, indent, "seenByAtLeastOneOther");
            break;
        case 5:
            ShowPERNull(tag, indent, "cancelSeenByAtLeastOneOther");
            break;
        case 6:
            ShowPERNull(tag, indent, "seenByAll");
            break;
        case 7:
            ShowPERNull(tag, indent, "cancelSeenByAll");
            break;
        case 8:
            Analyze_TerminalLabel(x->terminalYouAreSeeing, "terminalYouAreSeeing", tag, indent);
            break;
        case 9:
            ShowPERNull(tag, indent, "requestForFloor");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            ShowPERNull(tag, indent, "withdrawChairToken");
            break;
        case 11:
            Analyze_TerminalLabel(x->floorRequested, "floorRequested", tag, indent);
            break;
        case 12:
            Analyze_TerminalYouAreSeeingInSubPictureNumber(x->terminalYouAreSeeingInSubPictureNumber, "terminalYouAreSeeingInSubPictureNumber", tag, indent);
            break;
        case 13:
            Analyze_VideoIndicateCompose(x->videoIndicateCompose, "videoIndicateCompose", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_ConferenceIndication: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "ConferenceIndication");
}

/* <==================================================================> */
/*  PER-Analyzer for TerminalYouAreSeeingInSubPictureNumber (SEQUENCE)  */
/* <==================================================================> */
void Analyze_TerminalYouAreSeeingInSubPictureNumber(PS_TerminalYouAreSeeingInSubPictureNumber x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "TerminalYouAreSeeingInSubPictureNumber");
    indent += 2;
    ShowPERInteger(tag, indent, "terminalNumber", (int32)x->terminalNumber);
    ShowPERInteger(tag, indent, "subPictureNumber", (int32)x->subPictureNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "TerminalYouAreSeeingInSubPictureNumber");
}

/* <================================================> */
/*  PER-Analyzer for VideoIndicateCompose (SEQUENCE)  */
/* <================================================> */
void Analyze_VideoIndicateCompose(PS_VideoIndicateCompose x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VideoIndicateCompose");
    indent += 2;
    ShowPERInteger(tag, indent, "compositionNumber", (int32)x->compositionNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VideoIndicateCompose");
}

/* <===================================================> */
/*  PER-Analyzer for MiscellaneousIndication (SEQUENCE)  */
/* <===================================================> */
void Analyze_MiscellaneousIndication(PS_MiscellaneousIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MiscellaneousIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    Analyze_MiType(&x->miType, "miType", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MiscellaneousIndication");
}

/* <================================> */
/*  PER-Analyzer for MiType (CHOICE)  */
/* <================================> */
void Analyze_MiType(PS_MiType x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "MiType");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "logicalChannelActive");
            break;
        case 1:
            ShowPERNull(tag, indent, "logicalChannelInactive");
            break;
        case 2:
            ShowPERNull(tag, indent, "multipointConference");
            break;
        case 3:
            ShowPERNull(tag, indent, "cancelMultipointConference");
            break;
        case 4:
            ShowPERNull(tag, indent, "multipointZeroComm");
            break;
        case 5:
            ShowPERNull(tag, indent, "cancelMultipointZeroComm");
            break;
        case 6:
            ShowPERNull(tag, indent, "multipointSecondaryStatus");
            break;
        case 7:
            ShowPERNull(tag, indent, "cancelMultipointSecondaryStatus");
            break;
        case 8:
            ShowPERNull(tag, indent, "videoIndicateReadyToActivate");
            break;
        case 9:
            ShowPERInteger(tag, indent, "videoTemporalSpatialTradeOff", (uint32)x->videoTemporalSpatialTradeOff);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Analyze_VideoNotDecodedMBs(x->videoNotDecodedMBs, "videoNotDecodedMBs", tag, indent);
            break;
        case 11:
            Analyze_TransportCapability(x->transportCapability, "transportCapability", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_MiType: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "MiType");
}

/* <==============================================> */
/*  PER-Analyzer for VideoNotDecodedMBs (SEQUENCE)  */
/* <==============================================> */
void Analyze_VideoNotDecodedMBs(PS_VideoNotDecodedMBs x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VideoNotDecodedMBs");
    indent += 2;
    ShowPERInteger(tag, indent, "firstMB", (int32)x->firstMB);
    ShowPERInteger(tag, indent, "numberOfMBs", (int32)x->numberOfMBs);
    ShowPERInteger(tag, indent, "temporalReference", (int32)x->temporalReference);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VideoNotDecodedMBs");
}

/* <============================================> */
/*  PER-Analyzer for JitterIndication (SEQUENCE)  */
/* <============================================> */
void Analyze_JitterIndication(PS_JitterIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "JitterIndication");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_skippedFrameCount", x->option_of_skippedFrameCount);
    ShowPERBoolean(tag, indent, "option_of_additionalDecoderBuffer", x->option_of_additionalDecoderBuffer);
    Analyze_JiScope(&x->jiScope, "jiScope", tag, indent);
    ShowPERInteger(tag, indent, "estimatedReceivedJitterMantissa", (int32)x->estimatedReceivedJitterMantissa);
    ShowPERInteger(tag, indent, "estimatedReceivedJitterExponent", (int32)x->estimatedReceivedJitterExponent);
    if (x->option_of_skippedFrameCount)
    {
        ShowPERInteger(tag, indent, "skippedFrameCount", (int32)x->skippedFrameCount);
    }
    if (x->option_of_additionalDecoderBuffer)
    {
        ShowPERInteger(tag, indent, "additionalDecoderBuffer", (int32)x->additionalDecoderBuffer);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "JitterIndication");
}

/* <=================================> */
/*  PER-Analyzer for JiScope (CHOICE)  */
/* <=================================> */
void Analyze_JiScope(PS_JiScope x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "JiScope");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "logicalChannelNumber", (uint32)x->logicalChannelNumber);
            break;
        case 1:
            ShowPERInteger(tag, indent, "resourceID", (uint32)x->resourceID);
            break;
        case 2:
            ShowPERNull(tag, indent, "wholeMultiplex");
            break;
        default:
            ErrorMessage("Analyze_JiScope: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "JiScope");
}

/* <==============================================> */
/*  PER-Analyzer for H223SkewIndication (SEQUENCE)  */
/* <==============================================> */
void Analyze_H223SkewIndication(PS_H223SkewIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H223SkewIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "logicalChannelNumber1", (int32)x->logicalChannelNumber1);
    ShowPERInteger(tag, indent, "logicalChannelNumber2", (int32)x->logicalChannelNumber2);
    ShowPERInteger(tag, indent, "skew", (int32)x->skew);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H223SkewIndication");
}

/* <======================================================> */
/*  PER-Analyzer for H2250MaximumSkewIndication (SEQUENCE)  */
/* <======================================================> */
void Analyze_H2250MaximumSkewIndication(PS_H2250MaximumSkewIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "H2250MaximumSkewIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "logicalChannelNumber1", (int32)x->logicalChannelNumber1);
    ShowPERInteger(tag, indent, "logicalChannelNumber2", (int32)x->logicalChannelNumber2);
    ShowPERInteger(tag, indent, "maximumSkew", (int32)x->maximumSkew);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "H2250MaximumSkewIndication");
}

/* <================================================> */
/*  PER-Analyzer for MCLocationIndication (SEQUENCE)  */
/* <================================================> */
void Analyze_MCLocationIndication(PS_MCLocationIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "MCLocationIndication");
    indent += 2;
    Analyze_TransportAddress(&x->signalAddress, "signalAddress", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "MCLocationIndication");
}

/* <================================================> */
/*  PER-Analyzer for VendorIdentification (SEQUENCE)  */
/* <================================================> */
void Analyze_VendorIdentification(PS_VendorIdentification x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "VendorIdentification");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_productNumber", x->option_of_productNumber);
    ShowPERBoolean(tag, indent, "option_of_versionNumber", x->option_of_versionNumber);
    Analyze_NonStandardIdentifier(&x->vendor, "vendor", tag, indent);
    if (x->option_of_productNumber)
    {
        ShowPEROctetString(tag, indent, "productNumber", &x->productNumber);
    }
    if (x->option_of_versionNumber)
    {
        ShowPEROctetString(tag, indent, "versionNumber", &x->versionNumber);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "VendorIdentification");
}

/* <==============================================> */
/*  PER-Analyzer for NewATMVCIndication (SEQUENCE)  */
/* <==============================================> */
void Analyze_NewATMVCIndication(PS_NewATMVCIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "NewATMVCIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "resourceID", (int32)x->resourceID);
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "bitRateLockedToPCRClock", x->bitRateLockedToPCRClock);
    ShowPERBoolean(tag, indent, "bitRateLockedToNetworkClock", x->bitRateLockedToNetworkClock);
    Analyze_IndAal(&x->indAal, "indAal", tag, indent);
    Analyze_IndMultiplex(&x->indMultiplex, "indMultiplex", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERBoolean(tag, indent, "option_of_indReverseParameters", x->option_of_indReverseParameters);
    if (x->option_of_indReverseParameters)
    {
        Analyze_IndReverseParameters(&x->indReverseParameters, "indReverseParameters", tag, indent);
    }
    ShowPERClosure(tag, indent, "NewATMVCIndication");
}

/* <================================================> */
/*  PER-Analyzer for IndReverseParameters (SEQUENCE)  */
/* <================================================> */
void Analyze_IndReverseParameters(PS_IndReverseParameters x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IndReverseParameters");
    indent += 2;
    ShowPERInteger(tag, indent, "bitRate", (int32)x->bitRate);
    ShowPERBoolean(tag, indent, "bitRateLockedToPCRClock", x->bitRateLockedToPCRClock);
    ShowPERBoolean(tag, indent, "bitRateLockedToNetworkClock", x->bitRateLockedToNetworkClock);
    Analyze_IrpMultiplex(&x->irpMultiplex, "irpMultiplex", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IndReverseParameters");
}

/* <======================================> */
/*  PER-Analyzer for IrpMultiplex (CHOICE)  */
/* <======================================> */
void Analyze_IrpMultiplex(PS_IrpMultiplex x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IrpMultiplex");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noMultiplex");
            break;
        case 1:
            ShowPERNull(tag, indent, "transportStream");
            break;
        case 2:
            ShowPERNull(tag, indent, "programStream");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IrpMultiplex: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IrpMultiplex");
}

/* <======================================> */
/*  PER-Analyzer for IndMultiplex (CHOICE)  */
/* <======================================> */
void Analyze_IndMultiplex(PS_IndMultiplex x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IndMultiplex");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "noMultiplex");
            break;
        case 1:
            ShowPERNull(tag, indent, "transportStream");
            break;
        case 2:
            ShowPERNull(tag, indent, "programStream");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IndMultiplex: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IndMultiplex");
}

/* <================================> */
/*  PER-Analyzer for IndAal (CHOICE)  */
/* <================================> */
void Analyze_IndAal(PS_IndAal x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IndAal");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_IndAal1(x->indAal1, "indAal1", tag, indent);
            break;
        case 1:
            Analyze_IndAal5(x->indAal5, "indAal5", tag, indent);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IndAal: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IndAal");
}

/* <===================================> */
/*  PER-Analyzer for IndAal5 (SEQUENCE)  */
/* <===================================> */
void Analyze_IndAal5(PS_IndAal5 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IndAal5");
    indent += 2;
    ShowPERInteger(tag, indent, "forwardMaximumSDUSize", (int32)x->forwardMaximumSDUSize);
    ShowPERInteger(tag, indent, "backwardMaximumSDUSize", (int32)x->backwardMaximumSDUSize);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IndAal5");
}

/* <===================================> */
/*  PER-Analyzer for IndAal1 (SEQUENCE)  */
/* <===================================> */
void Analyze_IndAal1(PS_IndAal1 x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "IndAal1");
    indent += 2;
    Analyze_IndClockRecovery(&x->indClockRecovery, "indClockRecovery", tag, indent);
    Analyze_IndErrorCorrection(&x->indErrorCorrection, "indErrorCorrection", tag, indent);
    ShowPERBoolean(tag, indent, "structuredDataTransfer", x->structuredDataTransfer);
    ShowPERBoolean(tag, indent, "partiallyFilledCells", x->partiallyFilledCells);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "IndAal1");
}

/* <============================================> */
/*  PER-Analyzer for IndErrorCorrection (CHOICE)  */
/* <============================================> */
void Analyze_IndErrorCorrection(PS_IndErrorCorrection x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IndErrorCorrection");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "nullErrorCorrection");
            break;
        case 1:
            ShowPERNull(tag, indent, "longInterleaver");
            break;
        case 2:
            ShowPERNull(tag, indent, "shortInterleaver");
            break;
        case 3:
            ShowPERNull(tag, indent, "errorCorrectionOnly");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IndErrorCorrection: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IndErrorCorrection");
}

/* <==========================================> */
/*  PER-Analyzer for IndClockRecovery (CHOICE)  */
/* <==========================================> */
void Analyze_IndClockRecovery(PS_IndClockRecovery x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "IndClockRecovery");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERNull(tag, indent, "nullClockRecovery");
            break;
        case 1:
            ShowPERNull(tag, indent, "srtsClockRecovery");
            break;
        case 2:
            ShowPERNull(tag, indent, "adaptiveClockRecovery");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_IndClockRecovery: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "IndClockRecovery");
}

/* <=============================================> */
/*  PER-Analyzer for UserInputIndication (CHOICE)  */
/* <=============================================> */
void Analyze_UserInputIndication(PS_UserInputIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "UserInputIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERCharString(tag, indent, "alphanumeric", x->alphanumeric);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Analyze_UserInputSupportIndication(x->userInputSupportIndication, "userInputSupportIndication", tag, indent);
            break;
        case 3:
            Analyze_Signal(x->signal, "signal", tag, indent);
            break;
        case 4:
            Analyze_SignalUpdate(x->signalUpdate, "signalUpdate", tag, indent);
            break;
        default:
            ErrorMessage("Analyze_UserInputIndication: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "UserInputIndication");
}

/* <========================================> */
/*  PER-Analyzer for SignalUpdate (SEQUENCE)  */
/* <========================================> */
void Analyze_SignalUpdate(PS_SignalUpdate x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "SignalUpdate");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_rtp", x->option_of_rtp);
    ShowPERInteger(tag, indent, "duration", (int32)x->duration);
    if (x->option_of_rtp)
    {
        Analyze_Rtp(&x->rtp, "rtp", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "SignalUpdate");
}

/* <===============================> */
/*  PER-Analyzer for Rtp (SEQUENCE)  */
/* <===============================> */
void Analyze_Rtp(PS_Rtp x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Rtp");
    indent += 2;
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Rtp");
}

/* <==================================> */
/*  PER-Analyzer for Signal (SEQUENCE)  */
/* <==================================> */
void Analyze_Signal(PS_Signal x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "Signal");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_duration", x->option_of_duration);
    ShowPERBoolean(tag, indent, "option_of_signalRtp", x->option_of_signalRtp);
    ShowPERCharString(tag, indent, "signalType", &x->signalType);
    if (x->option_of_duration)
    {
        ShowPERInteger(tag, indent, "duration", (int32)x->duration);
    }
    if (x->option_of_signalRtp)
    {
        Analyze_SignalRtp(&x->signalRtp, "signalRtp", tag, indent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "Signal");
}

/* <=====================================> */
/*  PER-Analyzer for SignalRtp (SEQUENCE)  */
/* <=====================================> */
void Analyze_SignalRtp(PS_SignalRtp x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "SignalRtp");
    indent += 2;
    ShowPERBoolean(tag, indent, "option_of_timestamp", x->option_of_timestamp);
    ShowPERBoolean(tag, indent, "option_of_expirationTime", x->option_of_expirationTime);
    if (x->option_of_timestamp)
    {
        ShowPERInteger(tag, indent, "timestamp", (int32)x->timestamp);
    }
    if (x->option_of_expirationTime)
    {
        ShowPERInteger(tag, indent, "expirationTime", (int32)x->expirationTime);
    }
    ShowPERInteger(tag, indent, "logicalChannelNumber", (int32)x->logicalChannelNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "SignalRtp");
}

/* <====================================================> */
/*  PER-Analyzer for UserInputSupportIndication (CHOICE)  */
/* <====================================================> */
void Analyze_UserInputSupportIndication(PS_UserInputSupportIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "UserInputSupportIndication");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            Analyze_NonStandardParameter(x->nonStandard, "nonStandard", tag, indent);
            break;
        case 1:
            ShowPERNull(tag, indent, "basicString");
            break;
        case 2:
            ShowPERNull(tag, indent, "iA5String");
            break;
        case 3:
            ShowPERNull(tag, indent, "generalString");
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Analyze_UserInputSupportIndication: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "UserInputSupportIndication");
}

/* <=================================================> */
/*  PER-Analyzer for FlowControlIndication (SEQUENCE)  */
/* <=================================================> */
void Analyze_FlowControlIndication(PS_FlowControlIndication x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERSequence(tag, indent, label, "FlowControlIndication");
    indent += 2;
    Analyze_FciScope(&x->fciScope, "fciScope", tag, indent);
    Analyze_FciRestriction(&x->fciRestriction, "fciRestriction", tag, indent);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    ShowPERClosure(tag, indent, "FlowControlIndication");
}

/* <========================================> */
/*  PER-Analyzer for FciRestriction (CHOICE)  */
/* <========================================> */
void Analyze_FciRestriction(PS_FciRestriction x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FciRestriction");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "maximumBitRate", (uint32)x->maximumBitRate);
            break;
        case 1:
            ShowPERNull(tag, indent, "noRestriction");
            break;
        default:
            ErrorMessage("Analyze_FciRestriction: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FciRestriction");
}

/* <==================================> */
/*  PER-Analyzer for FciScope (CHOICE)  */
/* <==================================> */
void Analyze_FciScope(PS_FciScope x, const char* label, uint16 tag, uint16 indent)
{
    ShowPERChoice(tag, indent, label, "FciScope");
    indent += 2;
    ShowPERInteger(tag, indent, "index", x->index);
    switch (x->index)
    {
        case 0:
            ShowPERInteger(tag, indent, "logicalChannelNumber", (uint32)x->logicalChannelNumber);
            break;
        case 1:
            ShowPERInteger(tag, indent, "resourceID", (uint32)x->resourceID);
            break;
        case 2:
            ShowPERNull(tag, indent, "wholeMultiplex");
            break;
        default:
            ErrorMessage("Analyze_FciScope: Illegal CHOICE index");
    }
    ShowPERClosure(tag, indent, "FciScope");
}

