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
// FILE: h245_decoder.c
//
// DESC: PER Decode routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

#include "oscl_base.h"
#include "oscl_mem.h"
#include "per_headers.h"
#include "h245def.h"
#include "h245_decoder.h"

/* <=======================================================> */
/*  PER-Decoder for MultimediaSystemControlMessage (CHOICE)  */
/* <=======================================================> */
void Decode_MultimediaSystemControlMessage(PS_MultimediaSystemControlMessage x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->request = (PS_RequestMessage) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMessage));
            Decode_RequestMessage(x->request, stream);
            break;
        case 1:
            x->response = (PS_ResponseMessage) OSCL_DEFAULT_MALLOC(sizeof(S_ResponseMessage));
            Decode_ResponseMessage(x->response, stream);
            break;
        case 2:
            x->command = (PS_CommandMessage) OSCL_DEFAULT_MALLOC(sizeof(S_CommandMessage));
            Decode_CommandMessage(x->command, stream);
            break;
        case 3:
            x->indication = (PS_IndicationMessage) OSCL_DEFAULT_MALLOC(sizeof(S_IndicationMessage));
            Decode_IndicationMessage(x->indication, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MultimediaSystemControlMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for RequestMessage (CHOICE)  */
/* <=======================================> */
void Decode_RequestMessage(PS_RequestMessage x, PS_InStream stream)
{
    x->index = GetChoiceIndex(11, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->masterSlaveDetermination = (PS_MasterSlaveDetermination) OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDetermination));
            Decode_MasterSlaveDetermination(x->masterSlaveDetermination, stream);
            break;
        case 2:
            x->terminalCapabilitySet = (PS_TerminalCapabilitySet) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySet));
            Decode_TerminalCapabilitySet(x->terminalCapabilitySet, stream);
            break;
        case 3:
            x->openLogicalChannel = (PS_OpenLogicalChannel) OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannel));
            Decode_OpenLogicalChannel(x->openLogicalChannel, stream);
            break;
        case 4:
            x->closeLogicalChannel = (PS_CloseLogicalChannel) OSCL_DEFAULT_MALLOC(sizeof(S_CloseLogicalChannel));
            Decode_CloseLogicalChannel(x->closeLogicalChannel, stream);
            break;
        case 5:
            x->requestChannelClose = (PS_RequestChannelClose) OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelClose));
            Decode_RequestChannelClose(x->requestChannelClose, stream);
            break;
        case 6:
            x->multiplexEntrySend = (PS_MultiplexEntrySend) OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySend));
            Decode_MultiplexEntrySend(x->multiplexEntrySend, stream);
            break;
        case 7:
            x->requestMultiplexEntry = (PS_RequestMultiplexEntry) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntry));
            Decode_RequestMultiplexEntry(x->requestMultiplexEntry, stream);
            break;
        case 8:
            x->requestMode = (PS_RequestMode) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMode));
            Decode_RequestMode(x->requestMode, stream);
            break;
        case 9:
            x->roundTripDelayRequest = (PS_RoundTripDelayRequest) OSCL_DEFAULT_MALLOC(sizeof(S_RoundTripDelayRequest));
            Decode_RoundTripDelayRequest(x->roundTripDelayRequest, stream);
            break;
        case 10:
            x->maintenanceLoopRequest = (PS_MaintenanceLoopRequest) OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopRequest));
            Decode_MaintenanceLoopRequest(x->maintenanceLoopRequest, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            GetLengthDet(stream);
            x->communicationModeRequest = (PS_CommunicationModeRequest) OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeRequest));
            Decode_CommunicationModeRequest(x->communicationModeRequest, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->conferenceRequest = (PS_ConferenceRequest) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceRequest));
            Decode_ConferenceRequest(x->conferenceRequest, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->multilinkRequest = (PS_MultilinkRequest) OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkRequest));
            Decode_MultilinkRequest(x->multilinkRequest, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            x->logicalChannelRateRequest = (PS_LogicalChannelRateRequest) OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateRequest));
            Decode_LogicalChannelRateRequest(x->logicalChannelRateRequest, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_RequestMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for ResponseMessage (CHOICE)  */
/* <========================================> */
void Decode_ResponseMessage(PS_ResponseMessage x, PS_InStream stream)
{
    x->index = GetChoiceIndex(19, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->masterSlaveDeterminationAck = (PS_MasterSlaveDeterminationAck) OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationAck));
            Decode_MasterSlaveDeterminationAck(x->masterSlaveDeterminationAck, stream);
            break;
        case 2:
            x->masterSlaveDeterminationReject = (PS_MasterSlaveDeterminationReject) OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationReject));
            Decode_MasterSlaveDeterminationReject(x->masterSlaveDeterminationReject, stream);
            break;
        case 3:
            x->terminalCapabilitySetAck = (PS_TerminalCapabilitySetAck) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetAck));
            Decode_TerminalCapabilitySetAck(x->terminalCapabilitySetAck, stream);
            break;
        case 4:
            x->terminalCapabilitySetReject = (PS_TerminalCapabilitySetReject) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetReject));
            Decode_TerminalCapabilitySetReject(x->terminalCapabilitySetReject, stream);
            break;
        case 5:
            x->openLogicalChannelAck = (PS_OpenLogicalChannelAck) OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelAck));
            Decode_OpenLogicalChannelAck(x->openLogicalChannelAck, stream);
            break;
        case 6:
            x->openLogicalChannelReject = (PS_OpenLogicalChannelReject) OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelReject));
            Decode_OpenLogicalChannelReject(x->openLogicalChannelReject, stream);
            break;
        case 7:
            x->closeLogicalChannelAck = (PS_CloseLogicalChannelAck) OSCL_DEFAULT_MALLOC(sizeof(S_CloseLogicalChannelAck));
            Decode_CloseLogicalChannelAck(x->closeLogicalChannelAck, stream);
            break;
        case 8:
            x->requestChannelCloseAck = (PS_RequestChannelCloseAck) OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseAck));
            Decode_RequestChannelCloseAck(x->requestChannelCloseAck, stream);
            break;
        case 9:
            x->requestChannelCloseReject = (PS_RequestChannelCloseReject) OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseReject));
            Decode_RequestChannelCloseReject(x->requestChannelCloseReject, stream);
            break;
        case 10:
            x->multiplexEntrySendAck = (PS_MultiplexEntrySendAck) OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendAck));
            Decode_MultiplexEntrySendAck(x->multiplexEntrySendAck, stream);
            break;
        case 11:
            x->multiplexEntrySendReject = (PS_MultiplexEntrySendReject) OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendReject));
            Decode_MultiplexEntrySendReject(x->multiplexEntrySendReject, stream);
            break;
        case 12:
            x->requestMultiplexEntryAck = (PS_RequestMultiplexEntryAck) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryAck));
            Decode_RequestMultiplexEntryAck(x->requestMultiplexEntryAck, stream);
            break;
        case 13:
            x->requestMultiplexEntryReject = (PS_RequestMultiplexEntryReject) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryReject));
            Decode_RequestMultiplexEntryReject(x->requestMultiplexEntryReject, stream);
            break;
        case 14:
            x->requestModeAck = (PS_RequestModeAck) OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeAck));
            Decode_RequestModeAck(x->requestModeAck, stream);
            break;
        case 15:
            x->requestModeReject = (PS_RequestModeReject) OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeReject));
            Decode_RequestModeReject(x->requestModeReject, stream);
            break;
        case 16:
            x->roundTripDelayResponse = (PS_RoundTripDelayResponse) OSCL_DEFAULT_MALLOC(sizeof(S_RoundTripDelayResponse));
            Decode_RoundTripDelayResponse(x->roundTripDelayResponse, stream);
            break;
        case 17:
            x->maintenanceLoopAck = (PS_MaintenanceLoopAck) OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopAck));
            Decode_MaintenanceLoopAck(x->maintenanceLoopAck, stream);
            break;
        case 18:
            x->maintenanceLoopReject = (PS_MaintenanceLoopReject) OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopReject));
            Decode_MaintenanceLoopReject(x->maintenanceLoopReject, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            GetLengthDet(stream);
            x->communicationModeResponse = (PS_CommunicationModeResponse) OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeResponse));
            Decode_CommunicationModeResponse(x->communicationModeResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            x->conferenceResponse = (PS_ConferenceResponse) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceResponse));
            Decode_ConferenceResponse(x->conferenceResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            x->multilinkResponse = (PS_MultilinkResponse) OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkResponse));
            Decode_MultilinkResponse(x->multilinkResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 22:
            GetLengthDet(stream);
            x->logicalChannelRateAcknowledge = (PS_LogicalChannelRateAcknowledge) OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateAcknowledge));
            Decode_LogicalChannelRateAcknowledge(x->logicalChannelRateAcknowledge, stream);
            ReadRemainingBits(stream);
            break;
        case 23:
            GetLengthDet(stream);
            x->logicalChannelRateReject = (PS_LogicalChannelRateReject) OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateReject));
            Decode_LogicalChannelRateReject(x->logicalChannelRateReject, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ResponseMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for CommandMessage (CHOICE)  */
/* <=======================================> */
void Decode_CommandMessage(PS_CommandMessage x, PS_InStream stream)
{
    x->index = GetChoiceIndex(7, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->maintenanceLoopOffCommand = (PS_MaintenanceLoopOffCommand) OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopOffCommand));
            Decode_MaintenanceLoopOffCommand(x->maintenanceLoopOffCommand, stream);
            break;
        case 2:
            x->sendTerminalCapabilitySet = (PS_SendTerminalCapabilitySet) OSCL_DEFAULT_MALLOC(sizeof(S_SendTerminalCapabilitySet));
            Decode_SendTerminalCapabilitySet(x->sendTerminalCapabilitySet, stream);
            break;
        case 3:
            x->encryptionCommand = (PS_EncryptionCommand) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionCommand));
            Decode_EncryptionCommand(x->encryptionCommand, stream);
            break;
        case 4:
            x->flowControlCommand = (PS_FlowControlCommand) OSCL_DEFAULT_MALLOC(sizeof(S_FlowControlCommand));
            Decode_FlowControlCommand(x->flowControlCommand, stream);
            break;
        case 5:
            x->endSessionCommand = (PS_EndSessionCommand) OSCL_DEFAULT_MALLOC(sizeof(S_EndSessionCommand));
            Decode_EndSessionCommand(x->endSessionCommand, stream);
            break;
        case 6:
            x->miscellaneousCommand = (PS_MiscellaneousCommand) OSCL_DEFAULT_MALLOC(sizeof(S_MiscellaneousCommand));
            Decode_MiscellaneousCommand(x->miscellaneousCommand, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            GetLengthDet(stream);
            x->communicationModeCommand = (PS_CommunicationModeCommand) OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeCommand));
            Decode_CommunicationModeCommand(x->communicationModeCommand, stream);
            ReadRemainingBits(stream);
            break;
        case 8:
            GetLengthDet(stream);
            x->conferenceCommand = (PS_ConferenceCommand) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceCommand));
            Decode_ConferenceCommand(x->conferenceCommand, stream);
            ReadRemainingBits(stream);
            break;
        case 9:
            GetLengthDet(stream);
            x->h223MultiplexReconfiguration = (PS_H223MultiplexReconfiguration) OSCL_DEFAULT_MALLOC(sizeof(S_H223MultiplexReconfiguration));
            Decode_H223MultiplexReconfiguration(x->h223MultiplexReconfiguration, stream);
            ReadRemainingBits(stream);
            break;
        case 10:
            GetLengthDet(stream);
            x->newATMVCCommand = (PS_NewATMVCCommand) OSCL_DEFAULT_MALLOC(sizeof(S_NewATMVCCommand));
            Decode_NewATMVCCommand(x->newATMVCCommand, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_CommandMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for IndicationMessage (CHOICE)  */
/* <==========================================> */
void Decode_IndicationMessage(PS_IndicationMessage x, PS_InStream stream)
{
    x->index = GetChoiceIndex(14, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->functionNotUnderstood = (PS_FunctionNotUnderstood) OSCL_DEFAULT_MALLOC(sizeof(S_FunctionNotUnderstood));
            Decode_FunctionNotUnderstood(x->functionNotUnderstood, stream);
            break;
        case 2:
            x->masterSlaveDeterminationRelease = (PS_MasterSlaveDeterminationRelease) OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationRelease));
            Decode_MasterSlaveDeterminationRelease(x->masterSlaveDeterminationRelease, stream);
            break;
        case 3:
            x->terminalCapabilitySetRelease = (PS_TerminalCapabilitySetRelease) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetRelease));
            Decode_TerminalCapabilitySetRelease(x->terminalCapabilitySetRelease, stream);
            break;
        case 4:
            x->openLogicalChannelConfirm = (PS_OpenLogicalChannelConfirm) OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelConfirm));
            Decode_OpenLogicalChannelConfirm(x->openLogicalChannelConfirm, stream);
            break;
        case 5:
            x->requestChannelCloseRelease = (PS_RequestChannelCloseRelease) OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseRelease));
            Decode_RequestChannelCloseRelease(x->requestChannelCloseRelease, stream);
            break;
        case 6:
            x->multiplexEntrySendRelease = (PS_MultiplexEntrySendRelease) OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendRelease));
            Decode_MultiplexEntrySendRelease(x->multiplexEntrySendRelease, stream);
            break;
        case 7:
            x->requestMultiplexEntryRelease = (PS_RequestMultiplexEntryRelease) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryRelease));
            Decode_RequestMultiplexEntryRelease(x->requestMultiplexEntryRelease, stream);
            break;
        case 8:
            x->requestModeRelease = (PS_RequestModeRelease) OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeRelease));
            Decode_RequestModeRelease(x->requestModeRelease, stream);
            break;
        case 9:
            x->miscellaneousIndication = (PS_MiscellaneousIndication) OSCL_DEFAULT_MALLOC(sizeof(S_MiscellaneousIndication));
            Decode_MiscellaneousIndication(x->miscellaneousIndication, stream);
            break;
        case 10:
            x->jitterIndication = (PS_JitterIndication) OSCL_DEFAULT_MALLOC(sizeof(S_JitterIndication));
            Decode_JitterIndication(x->jitterIndication, stream);
            break;
        case 11:
            x->h223SkewIndication = (PS_H223SkewIndication) OSCL_DEFAULT_MALLOC(sizeof(S_H223SkewIndication));
            Decode_H223SkewIndication(x->h223SkewIndication, stream);
            break;
        case 12:
            x->newATMVCIndication = (PS_NewATMVCIndication) OSCL_DEFAULT_MALLOC(sizeof(S_NewATMVCIndication));
            Decode_NewATMVCIndication(x->newATMVCIndication, stream);
            break;
        case 13:
            x->userInput = (PS_UserInputIndication) OSCL_DEFAULT_MALLOC(sizeof(S_UserInputIndication));
            Decode_UserInputIndication(x->userInput, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            GetLengthDet(stream);
            x->h2250MaximumSkewIndication = (PS_H2250MaximumSkewIndication) OSCL_DEFAULT_MALLOC(sizeof(S_H2250MaximumSkewIndication));
            Decode_H2250MaximumSkewIndication(x->h2250MaximumSkewIndication, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->mcLocationIndication = (PS_MCLocationIndication) OSCL_DEFAULT_MALLOC(sizeof(S_MCLocationIndication));
            Decode_MCLocationIndication(x->mcLocationIndication, stream);
            ReadRemainingBits(stream);
            break;
        case 16:
            GetLengthDet(stream);
            x->conferenceIndication = (PS_ConferenceIndication) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceIndication));
            Decode_ConferenceIndication(x->conferenceIndication, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            x->vendorIdentification = (PS_VendorIdentification) OSCL_DEFAULT_MALLOC(sizeof(S_VendorIdentification));
            Decode_VendorIdentification(x->vendorIdentification, stream);
            ReadRemainingBits(stream);
            break;
        case 18:
            GetLengthDet(stream);
            x->functionNotSupported = (PS_FunctionNotSupported) OSCL_DEFAULT_MALLOC(sizeof(S_FunctionNotSupported));
            Decode_FunctionNotSupported(x->functionNotSupported, stream);
            ReadRemainingBits(stream);
            break;
        case 19:
            GetLengthDet(stream);
            x->multilinkIndication = (PS_MultilinkIndication) OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkIndication));
            Decode_MultilinkIndication(x->multilinkIndication, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            x->logicalChannelRateRelease = (PS_LogicalChannelRateRelease) OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateRelease));
            Decode_LogicalChannelRateRelease(x->logicalChannelRateRelease, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            x->flowControlIndication = (PS_FlowControlIndication) OSCL_DEFAULT_MALLOC(sizeof(S_FlowControlIndication));
            Decode_FlowControlIndication(x->flowControlIndication, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_IndicationMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for GenericInformation (SEQUENCE)  */
/* <=============================================> */
void Decode_GenericInformation(PS_GenericInformation x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_subMessageIdentifier = GetBoolean(stream);
    x->option_of_messageContent = GetBoolean(stream);
    Decode_CapabilityIdentifier(&x->messageIdentifier, stream);
    if (x->option_of_subMessageIdentifier)
    {
        x->subMessageIdentifier = (uint8)GetInteger(0, 127, stream);
    }
    if (x->option_of_messageContent)
    {
        x->size_of_messageContent = (uint16)GetLengthDet(stream);
        x->messageContent = (PS_GenericParameter)
                            OSCL_DEFAULT_MALLOC(x->size_of_messageContent * sizeof(S_GenericParameter));
        for (i = 0;i < x->size_of_messageContent;++i)
        {
            Decode_GenericParameter(x->messageContent + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessageAndLeave("Decode_GenericInformation: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for NonStandardMessage (SEQUENCE)  */
/* <=============================================> */
void Decode_NonStandardMessage(PS_NonStandardMessage x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_NonStandardParameter(&x->nonStandardData, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_NonStandardMessage: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for NonStandardParameter (SEQUENCE)  */
/* <===============================================> */
void Decode_NonStandardParameter(PS_NonStandardParameter x, PS_InStream stream)
{
    Decode_NonStandardIdentifier(&x->nonStandardIdentifier, stream);
    GetOctetString(1, 0, 0, &x->data, stream);
}

/* <==============================================> */
/*  PER-Decoder for NonStandardIdentifier (CHOICE)  */
/* <==============================================> */
void Decode_NonStandardIdentifier(PS_NonStandardIdentifier x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->object = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            GetObjectID(x->object, stream);
            break;
        case 1:
            x->h221NonStandard = (PS_H221NonStandard) OSCL_DEFAULT_MALLOC(sizeof(S_H221NonStandard));
            Decode_H221NonStandard(x->h221NonStandard, stream);
            break;
        default:
            ErrorMessageAndLeave("Decode_NonStandardIdentifier: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Decoder for H221NonStandard (SEQUENCE)  */
/* <==========================================> */
void Decode_H221NonStandard(PS_H221NonStandard x, PS_InStream stream)
{
    x->t35CountryCode = (uint8) GetInteger(0, 255, stream);
    x->t35Extension = (uint8) GetInteger(0, 255, stream);
    x->manufacturerCode = (uint16) GetInteger(0, 65535, stream);
}

/* <===================================================> */
/*  PER-Decoder for MasterSlaveDetermination (SEQUENCE)  */
/* <===================================================> */
void Decode_MasterSlaveDetermination(PS_MasterSlaveDetermination x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->terminalType = (uint8) GetInteger(0, 255, stream);
    x->statusDeterminationNumber = GetInteger(0, 16777215, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MasterSlaveDetermination: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for MasterSlaveDeterminationAck (SEQUENCE)  */
/* <======================================================> */
void Decode_MasterSlaveDeterminationAck(PS_MasterSlaveDeterminationAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Decision(&x->decision, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MasterSlaveDeterminationAck: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for Decision (CHOICE)  */
/* <=================================> */
void Decode_Decision(PS_Decision x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (master is NULL) */
            break;
        case 1:
            /* (slave is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Decision: Illegal CHOICE index");
    }
}

/* <=========================================================> */
/*  PER-Decoder for MasterSlaveDeterminationReject (SEQUENCE)  */
/* <=========================================================> */
void Decode_MasterSlaveDeterminationReject(PS_MasterSlaveDeterminationReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_MsdRejectCause(&x->msdRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MasterSlaveDeterminationReject: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for MsdRejectCause (CHOICE)  */
/* <=======================================> */
void Decode_MsdRejectCause(PS_MsdRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (identicalNumbers is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MsdRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================================> */
/*  PER-Decoder for MasterSlaveDeterminationRelease (SEQUENCE)  */
/* <==========================================================> */
void Decode_MasterSlaveDeterminationRelease(PS_MasterSlaveDeterminationRelease x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MasterSlaveDeterminationRelease: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for TerminalCapabilitySet (SEQUENCE)  */
/* <================================================> */
void Decode_TerminalCapabilitySet(PS_TerminalCapabilitySet x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_multiplexCapability = GetBoolean(stream);
    x->option_of_capabilityTable = GetBoolean(stream);
    x->option_of_capabilityDescriptors = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    GetObjectID(&x->protocolIdentifier, stream);
    if (x->option_of_multiplexCapability)
    {
        Decode_MultiplexCapability(&x->multiplexCapability, stream);
    }
    if (x->option_of_capabilityTable)
    {
        x->size_of_capabilityTable = (uint16) GetInteger(1, 256, stream);
        x->capabilityTable = (PS_CapabilityTableEntry)
                             OSCL_DEFAULT_MALLOC(x->size_of_capabilityTable * sizeof(S_CapabilityTableEntry));
        for (i = 0;i < x->size_of_capabilityTable;++i)
        {
            Decode_CapabilityTableEntry(x->capabilityTable + i, stream);
        }
    }
    if (x->option_of_capabilityDescriptors)
    {
        x->size_of_capabilityDescriptors = (uint16) GetInteger(1, 256, stream);
        x->capabilityDescriptors = (PS_CapabilityDescriptor)
                                   OSCL_DEFAULT_MALLOC(x->size_of_capabilityDescriptors * sizeof(S_CapabilityDescriptor));
        for (i = 0;i < x->size_of_capabilityDescriptors;++i)
        {
            Decode_CapabilityDescriptor(x->capabilityDescriptors + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_genericInformation = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_genericInformation = SigMapValue(0, map);
        if (x->option_of_genericInformation)
        {
            ExtensionPrep(map, stream);
            x->size_of_genericInformation = (uint16)GetLengthDet(stream);
            x->genericInformation = (PS_GenericInformation)
                                    OSCL_DEFAULT_MALLOC(x->size_of_genericInformation * sizeof(S_GenericInformation));
            for (i = 0;i < x->size_of_genericInformation;++i)
            {
                Decode_GenericInformation(x->genericInformation + i, stream);
            }
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalCapabilitySet: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for CapabilityTableEntry (SEQUENCE)  */
/* <===============================================> */
void Decode_CapabilityTableEntry(PS_CapabilityTableEntry x, PS_InStream stream)
{
    x->option_of_capability = GetBoolean(stream);
    x->capabilityTableEntryNumber = (uint16) GetInteger(1, 65535, stream);
    if (x->option_of_capability)
    {
        Decode_Capability(&x->capability, stream);
    }
}

/* <===============================================> */
/*  PER-Decoder for CapabilityDescriptor (SEQUENCE)  */
/* <===============================================> */
void Decode_CapabilityDescriptor(PS_CapabilityDescriptor x, PS_InStream stream)
{
    uint16 i;
    x->option_of_simultaneousCapabilities = GetBoolean(stream);
    x->capabilityDescriptorNumber = (uint8) GetInteger(0, 255, stream);
    if (x->option_of_simultaneousCapabilities)
    {
        x->size_of_simultaneousCapabilities = (uint16) GetInteger(1, 256, stream);
        x->simultaneousCapabilities = (PS_AlternativeCapabilitySet)
                                      OSCL_DEFAULT_MALLOC(x->size_of_simultaneousCapabilities * sizeof(S_AlternativeCapabilitySet));
        for (i = 0;i < x->size_of_simultaneousCapabilities;++i)
        {
            Decode_AlternativeCapabilitySet(x->simultaneousCapabilities + i, stream);
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for AlternativeCapabilitySet (SEQUENCE-OF)  */
/* <======================================================> */
void Decode_AlternativeCapabilitySet(PS_AlternativeCapabilitySet x, PS_InStream stream)
{
    uint16 i;
    x->size = (uint16) GetInteger(1, 256, stream);
    x->item = (uint32*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint32));
    for (i = 0;i < x->size;++i)
    {
        x->item[i] = GetInteger(1, 65535, stream);
    }
}

/* <===================================================> */
/*  PER-Decoder for TerminalCapabilitySetAck (SEQUENCE)  */
/* <===================================================> */
void Decode_TerminalCapabilitySetAck(PS_TerminalCapabilitySetAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalCapabilitySetAck: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for TerminalCapabilitySetReject (SEQUENCE)  */
/* <======================================================> */
void Decode_TerminalCapabilitySetReject(PS_TerminalCapabilitySetReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    Decode_TcsRejectCause(&x->tcsRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalCapabilitySetReject: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for TcsRejectCause (CHOICE)  */
/* <=======================================> */
void Decode_TcsRejectCause(PS_TcsRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
        case 1:
            /* (undefinedTableEntryUsed is NULL) */
            break;
        case 2:
            /* (descriptorCapacityExceeded is NULL) */
            break;
        case 3:
            x->tableEntryCapacityExceeded = (PS_TableEntryCapacityExceeded) OSCL_DEFAULT_MALLOC(sizeof(S_TableEntryCapacityExceeded));
            Decode_TableEntryCapacityExceeded(x->tableEntryCapacityExceeded, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_TcsRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================================> */
/*  PER-Decoder for TableEntryCapacityExceeded (CHOICE)  */
/* <===================================================> */
void Decode_TableEntryCapacityExceeded(PS_TableEntryCapacityExceeded x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->highestEntryNumberProcessed = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            /* (noneProcessed is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_TableEntryCapacityExceeded: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Decoder for TerminalCapabilitySetRelease (SEQUENCE)  */
/* <=======================================================> */
void Decode_TerminalCapabilitySetRelease(PS_TerminalCapabilitySetRelease x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;


    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalCapabilitySetRelease: Unknown extensions (skipped)");
        }
    }
}

/* <===================================> */
/*  PER-Decoder for Capability (CHOICE)  */
/* <===================================> */
void Decode_Capability(PS_Capability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(12, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->receiveVideoCapability = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->receiveVideoCapability, stream);
            break;
        case 2:
            x->transmitVideoCapability = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->transmitVideoCapability, stream);
            break;
        case 3:
            x->receiveAndTransmitVideoCapability = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->receiveAndTransmitVideoCapability, stream);
            break;
        case 4:
            x->receiveAudioCapability = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->receiveAudioCapability, stream);
            break;
        case 5:
            x->transmitAudioCapability = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->transmitAudioCapability, stream);
            break;
        case 6:
            x->receiveAndTransmitAudioCapability = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->receiveAndTransmitAudioCapability, stream);
            break;
        case 7:
            x->receiveDataApplicationCapability = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->receiveDataApplicationCapability, stream);
            break;
        case 8:
            x->transmitDataApplicationCapability = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->transmitDataApplicationCapability, stream);
            break;
        case 9:
            x->receiveAndTransmitDataApplicationCapability = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->receiveAndTransmitDataApplicationCapability, stream);
            break;
        case 10:
            x->h233EncryptionTransmitCapability = GetBoolean(stream);
            break;
        case 11:
            x->h233EncryptionReceiveCapability = (PS_H233EncryptionReceiveCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H233EncryptionReceiveCapability));
            Decode_H233EncryptionReceiveCapability(x->h233EncryptionReceiveCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 12:
            GetLengthDet(stream);
            x->conferenceCapability = (PS_ConferenceCapability) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceCapability));
            Decode_ConferenceCapability(x->conferenceCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->h235SecurityCapability = (PS_H235SecurityCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H235SecurityCapability));
            Decode_H235SecurityCapability(x->h235SecurityCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            x->maxPendingReplacementFor = (uint8) GetInteger(0, 255, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->receiveUserInputCapability = (PS_UserInputCapability) OSCL_DEFAULT_MALLOC(sizeof(S_UserInputCapability));
            Decode_UserInputCapability(x->receiveUserInputCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 16:
            GetLengthDet(stream);
            x->transmitUserInputCapability = (PS_UserInputCapability) OSCL_DEFAULT_MALLOC(sizeof(S_UserInputCapability));
            Decode_UserInputCapability(x->transmitUserInputCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            x->receiveAndTransmitUserInputCapability = (PS_UserInputCapability) OSCL_DEFAULT_MALLOC(sizeof(S_UserInputCapability));
            Decode_UserInputCapability(x->receiveAndTransmitUserInputCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 18:
            GetLengthDet(stream);
            x->genericControlCapability = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericControlCapability, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_Capability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================================> */
/*  PER-Decoder for H233EncryptionReceiveCapability (SEQUENCE)  */
/* <==========================================================> */
void Decode_H233EncryptionReceiveCapability(PS_H233EncryptionReceiveCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->h233IVResponseTime = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H233EncryptionReceiveCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for H235SecurityCapability (SEQUENCE)  */
/* <=================================================> */
void Decode_H235SecurityCapability(PS_H235SecurityCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    x->mediaCapability = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H235SecurityCapability: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for MultiplexCapability (CHOICE)  */
/* <============================================> */
void Decode_MultiplexCapability(PS_MultiplexCapability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->h222Capability = (PS_H222Capability) OSCL_DEFAULT_MALLOC(sizeof(S_H222Capability));
            Decode_H222Capability(x->h222Capability, stream);
            break;
        case 2:
            x->h223Capability = (PS_H223Capability) OSCL_DEFAULT_MALLOC(sizeof(S_H223Capability));
            Decode_H223Capability(x->h223Capability, stream);
            break;
        case 3:
            x->v76Capability = (PS_V76Capability) OSCL_DEFAULT_MALLOC(sizeof(S_V76Capability));
            Decode_V76Capability(x->v76Capability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            GetLengthDet(stream);
            x->h2250Capability = (PS_H2250Capability) OSCL_DEFAULT_MALLOC(sizeof(S_H2250Capability));
            Decode_H2250Capability(x->h2250Capability, stream);
            ReadRemainingBits(stream);
            break;
        case 5:
            GetLengthDet(stream);
            x->genericMultiplexCapability = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericMultiplexCapability, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_MultiplexCapability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for H222Capability (SEQUENCE)  */
/* <=========================================> */
void Decode_H222Capability(PS_H222Capability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->numberOfVCs = (uint16) GetInteger(1, 256, stream);
    x->size_of_vcCapability = (uint16) GetLengthDet(stream);
    x->vcCapability = (PS_VCCapability)
                      OSCL_DEFAULT_MALLOC(x->size_of_vcCapability * sizeof(S_VCCapability));
    for (i = 0;i < x->size_of_vcCapability;++i)
    {
        Decode_VCCapability(x->vcCapability + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H222Capability: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for VCCapability (SEQUENCE)  */
/* <=======================================> */
void Decode_VCCapability(PS_VCCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_vccAal1 = GetBoolean(stream);
    x->option_of_vccAal5 = GetBoolean(stream);
    if (x->option_of_vccAal1)
    {
        Decode_VccAal1(&x->vccAal1, stream);
    }
    if (x->option_of_vccAal5)
    {
        Decode_VccAal5(&x->vccAal5, stream);
    }
    x->transportStream = GetBoolean(stream);
    x->programStream = GetBoolean(stream);
    Decode_AvailableBitRates(&x->availableBitRates, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_aal1ViaGateway = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_aal1ViaGateway = SigMapValue(0, map);
        if (x->option_of_aal1ViaGateway)
        {
            ExtensionPrep(map, stream);
            Decode_Aal1ViaGateway(&x->aal1ViaGateway, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VCCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for Aal1ViaGateway (SEQUENCE)  */
/* <=========================================> */
void Decode_Aal1ViaGateway(PS_Aal1ViaGateway x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_gatewayAddress = (uint16) GetInteger(1, 256, stream);
    x->gatewayAddress = (PS_Q2931Address)
                        OSCL_DEFAULT_MALLOC(x->size_of_gatewayAddress * sizeof(S_Q2931Address));
    for (i = 0;i < x->size_of_gatewayAddress;++i)
    {
        Decode_Q2931Address(x->gatewayAddress + i, stream);
    }
    x->nullClockRecovery = GetBoolean(stream);
    x->srtsClockRecovery = GetBoolean(stream);
    x->adaptiveClockRecovery = GetBoolean(stream);
    x->nullErrorCorrection = GetBoolean(stream);
    x->longInterleaver = GetBoolean(stream);
    x->shortInterleaver = GetBoolean(stream);
    x->errorCorrectionOnly = GetBoolean(stream);
    x->structuredDataTransfer = GetBoolean(stream);
    x->partiallyFilledCells = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Aal1ViaGateway: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for AvailableBitRates (SEQUENCE)  */
/* <============================================> */
void Decode_AvailableBitRates(PS_AvailableBitRates x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_VccAal5Type(&x->vccAal5Type, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_AvailableBitRates: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for VccAal5Type (CHOICE)  */
/* <====================================> */
void Decode_VccAal5Type(PS_VccAal5Type x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->singleBitRate = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            x->rangeOfBitRates = (PS_RangeOfBitRates) OSCL_DEFAULT_MALLOC(sizeof(S_RangeOfBitRates));
            Decode_RangeOfBitRates(x->rangeOfBitRates, stream);
            break;
        default:
            ErrorMessageAndLeave("Decode_VccAal5Type: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Decoder for RangeOfBitRates (SEQUENCE)  */
/* <==========================================> */
void Decode_RangeOfBitRates(PS_RangeOfBitRates x, PS_InStream stream)
{
    x->lowerBitRate = (uint16) GetInteger(1, 65535, stream);
    x->higherBitRate = (uint16) GetInteger(1, 65535, stream);
}

/* <==================================> */
/*  PER-Decoder for VccAal5 (SEQUENCE)  */
/* <==================================> */
void Decode_VccAal5(PS_VccAal5 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    x->backwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VccAal5: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for VccAal1 (SEQUENCE)  */
/* <==================================> */
void Decode_VccAal1(PS_VccAal1 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->nullClockRecovery = GetBoolean(stream);
    x->srtsClockRecovery = GetBoolean(stream);
    x->adaptiveClockRecovery = GetBoolean(stream);
    x->nullErrorCorrection = GetBoolean(stream);
    x->longInterleaver = GetBoolean(stream);
    x->shortInterleaver = GetBoolean(stream);
    x->errorCorrectionOnly = GetBoolean(stream);
    x->structuredDataTransfer = GetBoolean(stream);
    x->partiallyFilledCells = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VccAal1: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for H223Capability (SEQUENCE)  */
/* <=========================================> */
void Decode_H223Capability(PS_H223Capability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->transportWithI_frames = GetBoolean(stream);
    x->videoWithAL1 = GetBoolean(stream);
    x->videoWithAL2 = GetBoolean(stream);
    x->videoWithAL3 = GetBoolean(stream);
    x->audioWithAL1 = GetBoolean(stream);
    x->audioWithAL2 = GetBoolean(stream);
    x->audioWithAL3 = GetBoolean(stream);
    x->dataWithAL1 = GetBoolean(stream);
    x->dataWithAL2 = GetBoolean(stream);
    x->dataWithAL3 = GetBoolean(stream);
    x->maximumAl2SDUSize = (uint16) GetInteger(0, 65535, stream);
    x->maximumAl3SDUSize = (uint16) GetInteger(0, 65535, stream);
    x->maximumDelayJitter = (uint16) GetInteger(0, 1023, stream);
    Decode_H223MultiplexTableCapability(&x->h223MultiplexTableCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_maxMUXPDUSizeCapability = OFF;
    x->option_of_nsrpSupport = OFF;
    x->option_of_mobileOperationTransmitCapability = OFF;
    x->option_of_h223AnnexCCapability = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_maxMUXPDUSizeCapability = SigMapValue(0, map);
        if (x->option_of_maxMUXPDUSizeCapability)
        {
            ExtensionPrep(map, stream);
            x->maxMUXPDUSizeCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_nsrpSupport = SigMapValue(1, map);
        if (x->option_of_nsrpSupport)
        {
            ExtensionPrep(map, stream);
            x->nsrpSupport = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_mobileOperationTransmitCapability = SigMapValue(2, map);
        if (x->option_of_mobileOperationTransmitCapability)
        {
            ExtensionPrep(map, stream);
            Decode_MobileOperationTransmitCapability(&x->mobileOperationTransmitCapability, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_h223AnnexCCapability = SigMapValue(3, map);
        if (x->option_of_h223AnnexCCapability)
        {
            ExtensionPrep(map, stream);
            Decode_H223AnnexCCapability(&x->h223AnnexCCapability, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223Capability: Unknown extensions (skipped)");
        }
    }
}

/* <============================================================> */
/*  PER-Decoder for MobileOperationTransmitCapability (SEQUENCE)  */
/* <============================================================> */
void Decode_MobileOperationTransmitCapability(PS_MobileOperationTransmitCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->modeChangeCapability = GetBoolean(stream);
    x->h223AnnexA = GetBoolean(stream);
    x->h223AnnexADoubleFlag = GetBoolean(stream);
    x->h223AnnexB = GetBoolean(stream);
    x->h223AnnexBwithHeader = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MobileOperationTransmitCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================================> */
/*  PER-Decoder for H223MultiplexTableCapability (CHOICE)  */
/* <=====================================================> */
void Decode_H223MultiplexTableCapability(PS_H223MultiplexTableCapability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (basic is NULL) */
            break;
        case 1:
            x->enhanced = (PS_Enhanced) OSCL_DEFAULT_MALLOC(sizeof(S_Enhanced));
            Decode_Enhanced(x->enhanced, stream);
            break;
        default:
            ErrorMessageAndLeave("Decode_H223MultiplexTableCapability: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Decoder for Enhanced (SEQUENCE)  */
/* <===================================> */
void Decode_Enhanced(PS_Enhanced x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maximumNestingDepth = (uint8) GetInteger(1, 15, stream);
    x->maximumElementListSize = (uint8) GetInteger(2, 255, stream);
    x->maximumSubElementListSize = (uint8) GetInteger(2, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Enhanced: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for H223AnnexCCapability (SEQUENCE)  */
/* <===============================================> */
void Decode_H223AnnexCCapability(PS_H223AnnexCCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->videoWithAL1M = GetBoolean(stream);
    x->videoWithAL2M = GetBoolean(stream);
    x->videoWithAL3M = GetBoolean(stream);
    x->audioWithAL1M = GetBoolean(stream);
    x->audioWithAL2M = GetBoolean(stream);
    x->audioWithAL3M = GetBoolean(stream);
    x->dataWithAL1M = GetBoolean(stream);
    x->dataWithAL2M = GetBoolean(stream);
    x->dataWithAL3M = GetBoolean(stream);
    x->alpduInterleaving = GetBoolean(stream);
    x->maximumAL1MPDUSize = (uint16) GetInteger(0, 65535, stream);
    x->maximumAL2MSDUSize = (uint16) GetInteger(0, 65535, stream);
    x->maximumAL3MSDUSize = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_rsCodeCapability = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_rsCodeCapability = SigMapValue(0, map);
        if (x->option_of_rsCodeCapability)
        {
            ExtensionPrep(map, stream);
            x->rsCodeCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223AnnexCCapability: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for V76Capability (SEQUENCE)  */
/* <========================================> */
void Decode_V76Capability(PS_V76Capability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->suspendResumeCapabilitywAddress = GetBoolean(stream);
    x->suspendResumeCapabilitywoAddress = GetBoolean(stream);
    x->rejCapability = GetBoolean(stream);
    x->sREJCapability = GetBoolean(stream);
    x->mREJCapability = GetBoolean(stream);
    x->crc8bitCapability = GetBoolean(stream);
    x->crc16bitCapability = GetBoolean(stream);
    x->crc32bitCapability = GetBoolean(stream);
    x->uihCapability = GetBoolean(stream);
    x->numOfDLCS = (uint16) GetInteger(2, 8191, stream);
    x->twoOctetAddressFieldCapability = GetBoolean(stream);
    x->loopBackTestCapability = GetBoolean(stream);
    x->n401Capability = (uint16) GetInteger(1, 4095, stream);
    x->maxWindowSizeCapability = (uint8) GetInteger(1, 127, stream);
    Decode_V75Capability(&x->v75Capability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V76Capability: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for V75Capability (SEQUENCE)  */
/* <========================================> */
void Decode_V75Capability(PS_V75Capability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->audioHeader = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V75Capability: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for H2250Capability (SEQUENCE)  */
/* <==========================================> */
void Decode_H2250Capability(PS_H2250Capability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maximumAudioDelayJitter = (uint16) GetInteger(0, 1023, stream);
    Decode_MultipointCapability(&x->receiveMultipointCapability, stream);
    Decode_MultipointCapability(&x->transmitMultipointCapability, stream);
    Decode_MultipointCapability(&x->receiveAndTransmitMultipointCapability, stream);
    Decode_McCapability(&x->mcCapability, stream);
    x->rtcpVideoControlCapability = GetBoolean(stream);
    Decode_MediaPacketizationCapability(&x->mediaPacketizationCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_transportCapability = OFF;
    x->option_of_redundancyEncodingCapability = OFF;
    x->option_of_logicalChannelSwitchingCapability = OFF;
    x->option_of_t120DynamicPortCapability = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_transportCapability = SigMapValue(0, map);
        if (x->option_of_transportCapability)
        {
            ExtensionPrep(map, stream);
            Decode_TransportCapability(&x->transportCapability, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_redundancyEncodingCapability = SigMapValue(1, map);
        if (x->option_of_redundancyEncodingCapability)
        {
            ExtensionPrep(map, stream);
            x->size_of_redundancyEncodingCapability = (uint16) GetInteger(1, 256, stream);
            x->redundancyEncodingCapability = (PS_RedundancyEncodingCapability)
                                              OSCL_DEFAULT_MALLOC(x->size_of_redundancyEncodingCapability * sizeof(S_RedundancyEncodingCapability));
            for (i = 0;i < x->size_of_redundancyEncodingCapability;++i)
            {
                Decode_RedundancyEncodingCapability(x->redundancyEncodingCapability + i, stream);
            }
            ReadRemainingBits(stream);
        }
        x->option_of_logicalChannelSwitchingCapability = SigMapValue(2, map);
        if (x->option_of_logicalChannelSwitchingCapability)
        {
            ExtensionPrep(map, stream);
            x->logicalChannelSwitchingCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_t120DynamicPortCapability = SigMapValue(3, map);
        if (x->option_of_t120DynamicPortCapability)
        {
            ExtensionPrep(map, stream);
            x->t120DynamicPortCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H2250Capability: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for McCapability (SEQUENCE)  */
/* <=======================================> */
void Decode_McCapability(PS_McCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->centralizedConferenceMC = GetBoolean(stream);
    x->decentralizedConferenceMC = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_McCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================================> */
/*  PER-Decoder for MediaPacketizationCapability (SEQUENCE)  */
/* <=======================================================> */
void Decode_MediaPacketizationCapability(PS_MediaPacketizationCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->h261aVideoPacketization = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_rtpPayloadType = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_rtpPayloadType = SigMapValue(0, map);
        if (x->option_of_rtpPayloadType)
        {
            ExtensionPrep(map, stream);
            x->size_of_rtpPayloadType = (uint16) GetInteger(1, 256, stream);
            x->rtpPayloadType = (PS_RTPPayloadType)
                                OSCL_DEFAULT_MALLOC(x->size_of_rtpPayloadType * sizeof(S_RTPPayloadType));
            for (i = 0;i < x->size_of_rtpPayloadType;++i)
            {
                Decode_RTPPayloadType(x->rtpPayloadType + i, stream);
            }
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MediaPacketizationCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for RSVPParameters (SEQUENCE)  */
/* <=========================================> */
void Decode_RSVPParameters(PS_RSVPParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_qosMode = GetBoolean(stream);
    x->option_of_tokenRate = GetBoolean(stream);
    x->option_of_bucketSize = GetBoolean(stream);
    x->option_of_peakRate = GetBoolean(stream);
    x->option_of_minPoliced = GetBoolean(stream);
    x->option_of_maxPktSize = GetBoolean(stream);
    if (x->option_of_qosMode)
    {
        Decode_QOSMode(&x->qosMode, stream);
    }
    if (x->option_of_tokenRate)
    {
        x->tokenRate = GetInteger(1, 0xffffffff, stream);
    }
    if (x->option_of_bucketSize)
    {
        x->bucketSize = GetInteger(1, 0xffffffff, stream);
    }
    if (x->option_of_peakRate)
    {
        x->peakRate = GetInteger(1, 0xffffffff, stream);
    }
    if (x->option_of_minPoliced)
    {
        x->minPoliced = GetInteger(1, 0xffffffff, stream);
    }
    if (x->option_of_maxPktSize)
    {
        x->maxPktSize = GetInteger(1, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RSVPParameters: Unknown extensions (skipped)");
        }
    }
}

/* <================================> */
/*  PER-Decoder for QOSMode (CHOICE)  */
/* <================================> */
void Decode_QOSMode(PS_QOSMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (guaranteedQOS is NULL) */
            break;
        case 1:
            /* (controlledLoad is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_QOSMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for ATMParameters (SEQUENCE)  */
/* <========================================> */
void Decode_ATMParameters(PS_ATMParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maxNTUSize = (uint16) GetInteger(0, 65535, stream);
    x->atmUBR = GetBoolean(stream);
    x->atmrtVBR = GetBoolean(stream);
    x->atmnrtVBR = GetBoolean(stream);
    x->atmABR = GetBoolean(stream);
    x->atmCBR = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ATMParameters: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for QOSCapability (SEQUENCE)  */
/* <========================================> */
void Decode_QOSCapability(PS_QOSCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandardData = GetBoolean(stream);
    x->option_of_rsvpParameters = GetBoolean(stream);
    x->option_of_atmParameters = GetBoolean(stream);
    if (x->option_of_nonStandardData)
    {
        Decode_NonStandardParameter(&x->nonStandardData, stream);
    }
    if (x->option_of_rsvpParameters)
    {
        Decode_RSVPParameters(&x->rsvpParameters, stream);
    }
    if (x->option_of_atmParameters)
    {
        Decode_ATMParameters(&x->atmParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_QOSCapability: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for MediaTransportType (CHOICE)  */
/* <===========================================> */
void Decode_MediaTransportType(PS_MediaTransportType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (ip_UDP is NULL) */
            break;
        case 1:
            /* (ip_TCP is NULL) */
            break;
        case 2:
            /* (atm_AAL5_UNIDIR is NULL) */
            break;
        case 3:
            /* (atm_AAL5_BIDIR is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            GetLengthDet(stream);
            x->atm_AAL5_compressed = (PS_Atm_AAL5_compressed) OSCL_DEFAULT_MALLOC(sizeof(S_Atm_AAL5_compressed));
            Decode_Atm_AAL5_compressed(x->atm_AAL5_compressed, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_MediaTransportType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================================> */
/*  PER-Decoder for Atm_AAL5_compressed (SEQUENCE)  */
/* <==============================================> */
void Decode_Atm_AAL5_compressed(PS_Atm_AAL5_compressed x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->variable_delta = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Atm_AAL5_compressed: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for MediaChannelCapability (SEQUENCE)  */
/* <=================================================> */
void Decode_MediaChannelCapability(PS_MediaChannelCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_mediaTransport = GetBoolean(stream);
    if (x->option_of_mediaTransport)
    {
        Decode_MediaTransportType(&x->mediaTransport, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MediaChannelCapability: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for TransportCapability (SEQUENCE)  */
/* <==============================================> */
void Decode_TransportCapability(PS_TransportCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    x->option_of_qOSCapabilities = GetBoolean(stream);
    x->option_of_mediaChannelCapabilities = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        Decode_NonStandardParameter(&x->nonStandard, stream);
    }
    if (x->option_of_qOSCapabilities)
    {
        x->size_of_qOSCapabilities = (uint16) GetInteger(1, 256, stream);
        x->qOSCapabilities = (PS_QOSCapability)
                             OSCL_DEFAULT_MALLOC(x->size_of_qOSCapabilities * sizeof(S_QOSCapability));
        for (i = 0;i < x->size_of_qOSCapabilities;++i)
        {
            Decode_QOSCapability(x->qOSCapabilities + i, stream);
        }
    }
    if (x->option_of_mediaChannelCapabilities)
    {
        x->size_of_mediaChannelCapabilities = (uint16) GetInteger(1, 256, stream);
        x->mediaChannelCapabilities = (PS_MediaChannelCapability)
                                      OSCL_DEFAULT_MALLOC(x->size_of_mediaChannelCapabilities * sizeof(S_MediaChannelCapability));
        for (i = 0;i < x->size_of_mediaChannelCapabilities;++i)
        {
            Decode_MediaChannelCapability(x->mediaChannelCapabilities + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TransportCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================================> */
/*  PER-Decoder for RedundancyEncodingCapability (SEQUENCE)  */
/* <=======================================================> */
void Decode_RedundancyEncodingCapability(PS_RedundancyEncodingCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_secondaryEncoding = GetBoolean(stream);
    Decode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    x->primaryEncoding = (uint16) GetInteger(1, 65535, stream);
    if (x->option_of_secondaryEncoding)
    {
        x->size_of_secondaryEncoding = (uint16) GetInteger(1, 256, stream);
        x->secondaryEncoding = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_secondaryEncoding * sizeof(uint32));
        for (i = 0;i < x->size_of_secondaryEncoding;++i)
        {
            x->secondaryEncoding[i] = GetInteger(1, 65535, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RedundancyEncodingCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for RedundancyEncodingMethod (CHOICE)  */
/* <=================================================> */
void Decode_RedundancyEncodingMethod(PS_RedundancyEncodingMethod x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (rtpAudioRedundancyEncoding is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            GetLengthDet(stream);
            x->rtpH263VideoRedundancyEncoding = (PS_RTPH263VideoRedundancyEncoding) OSCL_DEFAULT_MALLOC(sizeof(S_RTPH263VideoRedundancyEncoding));
            Decode_RTPH263VideoRedundancyEncoding(x->rtpH263VideoRedundancyEncoding, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_RedundancyEncodingMethod: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================================> */
/*  PER-Decoder for RTPH263VideoRedundancyEncoding (SEQUENCE)  */
/* <=========================================================> */
void Decode_RTPH263VideoRedundancyEncoding(PS_RTPH263VideoRedundancyEncoding x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_containedThreads = GetBoolean(stream);
    x->numberOfThreads = (uint8) GetInteger(1, 16, stream);
    x->framesBetweenSyncPoints = (uint16) GetInteger(1, 256, stream);
    Decode_FrameToThreadMapping(&x->frameToThreadMapping, stream);
    if (x->option_of_containedThreads)
    {
        x->size_of_containedThreads = (uint16) GetInteger(1, 256, stream);
        x->containedThreads = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_containedThreads * sizeof(uint32));
        for (i = 0;i < x->size_of_containedThreads;++i)
        {
            x->containedThreads[i] = GetInteger(0, 15, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RTPH263VideoRedundancyEncoding: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for FrameToThreadMapping (CHOICE)  */
/* <=============================================> */
void Decode_FrameToThreadMapping(PS_FrameToThreadMapping x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (roundrobin is NULL) */
            break;
        case 1:
            x->size = (uint16) GetInteger(1, 256, stream);
            x->custom = (PS_RTPH263VideoRedundancyFrameMapping)
                        OSCL_DEFAULT_MALLOC(x->size * sizeof(S_RTPH263VideoRedundancyFrameMapping));
            for (i = 0;i < x->size;++i)
            {
                Decode_RTPH263VideoRedundancyFrameMapping(x->custom + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_FrameToThreadMapping: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================================> */
/*  PER-Decoder for RTPH263VideoRedundancyFrameMapping (SEQUENCE)  */
/* <=============================================================> */
void Decode_RTPH263VideoRedundancyFrameMapping(PS_RTPH263VideoRedundancyFrameMapping x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->threadNumber = (uint8) GetInteger(0, 15, stream);
    x->size_of_frameSequence = (uint16) GetInteger(1, 256, stream);
    x->frameSequence = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_frameSequence * sizeof(uint32));
    for (i = 0;i < x->size_of_frameSequence;++i)
    {
        x->frameSequence[i] = GetInteger(0, 255, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RTPH263VideoRedundancyFrameMapping: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for MultipointCapability (SEQUENCE)  */
/* <===============================================> */
void Decode_MultipointCapability(PS_MultipointCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->multicastCapability = GetBoolean(stream);
    x->multiUniCastConference = GetBoolean(stream);
    x->size_of_mediaDistributionCapability = (uint16) GetLengthDet(stream);
    x->mediaDistributionCapability = (PS_MediaDistributionCapability)
                                     OSCL_DEFAULT_MALLOC(x->size_of_mediaDistributionCapability * sizeof(S_MediaDistributionCapability));
    for (i = 0;i < x->size_of_mediaDistributionCapability;++i)
    {
        Decode_MediaDistributionCapability(x->mediaDistributionCapability + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultipointCapability: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for MediaDistributionCapability (SEQUENCE)  */
/* <======================================================> */
void Decode_MediaDistributionCapability(PS_MediaDistributionCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_centralizedData = GetBoolean(stream);
    x->option_of_distributedData = GetBoolean(stream);
    x->centralizedControl = GetBoolean(stream);
    x->distributedControl = GetBoolean(stream);
    x->centralizedAudio = GetBoolean(stream);
    x->distributedAudio = GetBoolean(stream);
    x->centralizedVideo = GetBoolean(stream);
    x->distributedVideo = GetBoolean(stream);
    if (x->option_of_centralizedData)
    {
        x->size_of_centralizedData = (uint16) GetLengthDet(stream);
        x->centralizedData = (PS_DataApplicationCapability)
                             OSCL_DEFAULT_MALLOC(x->size_of_centralizedData * sizeof(S_DataApplicationCapability));
        for (i = 0;i < x->size_of_centralizedData;++i)
        {
            Decode_DataApplicationCapability(x->centralizedData + i, stream);
        }
    }
    if (x->option_of_distributedData)
    {
        x->size_of_distributedData = (uint16) GetLengthDet(stream);
        x->distributedData = (PS_DataApplicationCapability)
                             OSCL_DEFAULT_MALLOC(x->size_of_distributedData * sizeof(S_DataApplicationCapability));
        for (i = 0;i < x->size_of_distributedData;++i)
        {
            Decode_DataApplicationCapability(x->distributedData + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MediaDistributionCapability: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for VideoCapability (CHOICE)  */
/* <========================================> */
void Decode_VideoCapability(PS_VideoCapability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->h261VideoCapability = (PS_H261VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H261VideoCapability));
            Decode_H261VideoCapability(x->h261VideoCapability, stream);
            break;
        case 2:
            x->h262VideoCapability = (PS_H262VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H262VideoCapability));
            Decode_H262VideoCapability(x->h262VideoCapability, stream);
            break;
        case 3:
            x->h263VideoCapability = (PS_H263VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H263VideoCapability));
            Decode_H263VideoCapability(x->h263VideoCapability, stream);
            break;
        case 4:
            x->is11172VideoCapability = (PS_IS11172VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_IS11172VideoCapability));
            Decode_IS11172VideoCapability(x->is11172VideoCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            GetLengthDet(stream);
            x->genericVideoCapability = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericVideoCapability, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_VideoCapability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================================> */
/*  PER-Decoder for H261VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Decode_H261VideoCapability(PS_H261VideoCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_qcifMPI = GetBoolean(stream);
    x->option_of_cifMPI = GetBoolean(stream);
    if (x->option_of_qcifMPI)
    {
        x->qcifMPI = (uint8) GetInteger(1, 4, stream);
    }
    if (x->option_of_cifMPI)
    {
        x->cifMPI = (uint8) GetInteger(1, 4, stream);
    }
    x->temporalSpatialTradeOffCapability = GetBoolean(stream);
    x->maxBitRate = (uint16) GetInteger(1, 19200, stream);
    x->stillImageTransmission = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H261VideoCapability: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for H262VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Decode_H262VideoCapability(PS_H262VideoCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_videoBitRate = GetBoolean(stream);
    x->option_of_vbvBufferSize = GetBoolean(stream);
    x->option_of_samplesPerLine = GetBoolean(stream);
    x->option_of_linesPerFrame = GetBoolean(stream);
    x->option_of_framesPerSecond = GetBoolean(stream);
    x->option_of_luminanceSampleRate = GetBoolean(stream);
    x->profileAndLevel_SPatML = GetBoolean(stream);
    x->profileAndLevel_MPatLL = GetBoolean(stream);
    x->profileAndLevel_MPatML = GetBoolean(stream);
    x->profileAndLevel_MPatH_14 = GetBoolean(stream);
    x->profileAndLevel_MPatHL = GetBoolean(stream);
    x->profileAndLevel_SNRatLL = GetBoolean(stream);
    x->profileAndLevel_SNRatML = GetBoolean(stream);
    x->profileAndLevel_SpatialatH_14 = GetBoolean(stream);
    x->profileAndLevel_HPatML = GetBoolean(stream);
    x->profileAndLevel_HPatH_14 = GetBoolean(stream);
    x->profileAndLevel_HPatHL = GetBoolean(stream);
    if (x->option_of_videoBitRate)
    {
        x->videoBitRate = GetInteger(0, 1073741823, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        x->vbvBufferSize = GetInteger(0, 262143, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        x->samplesPerLine = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        x->linesPerFrame = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_framesPerSecond)
    {
        x->framesPerSecond = (uint8) GetInteger(0, 15, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        x->luminanceSampleRate = GetInteger(0, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H262VideoCapability: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for H263VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Decode_H263VideoCapability(PS_H263VideoCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_sqcifMPI = GetBoolean(stream);
    x->option_of_qcifMPI = GetBoolean(stream);
    x->option_of_cifMPI = GetBoolean(stream);
    x->option_of_cif4MPI = GetBoolean(stream);
    x->option_of_cif16MPI = GetBoolean(stream);
    x->option_of_hrd_B = GetBoolean(stream);
    x->option_of_bppMaxKb = GetBoolean(stream);
    if (x->option_of_sqcifMPI)
    {
        x->sqcifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_qcifMPI)
    {
        x->qcifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cifMPI)
    {
        x->cifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cif4MPI)
    {
        x->cif4MPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cif16MPI)
    {
        x->cif16MPI = (uint8) GetInteger(1, 32, stream);
    }
    x->maxBitRate = GetInteger(1, 192400, stream);
    x->unrestrictedVector = GetBoolean(stream);
    x->arithmeticCoding = GetBoolean(stream);
    x->advancedPrediction = GetBoolean(stream);
    x->pbFrames = GetBoolean(stream);
    x->temporalSpatialTradeOffCapability = GetBoolean(stream);
    if (x->option_of_hrd_B)
    {
        x->hrd_B = GetInteger(0, 524287, stream);
    }
    if (x->option_of_bppMaxKb)
    {
        x->bppMaxKb = (uint16) GetInteger(0, 65535, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_slowSqcifMPI = OFF;
    x->option_of_slowQcifMPI = OFF;
    x->option_of_slowCifMPI = OFF;
    x->option_of_slowCif4MPI = OFF;
    x->option_of_slowCif16MPI = OFF;
    x->option_of_errorCompensation = OFF;
    x->option_of_enhancementLayerInfo = OFF;
    x->option_of_h263Options = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_slowSqcifMPI = SigMapValue(0, map);
        if (x->option_of_slowSqcifMPI)
        {
            ExtensionPrep(map, stream);
            x->slowSqcifMPI = (uint16) GetInteger(1, 3600, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_slowQcifMPI = SigMapValue(1, map);
        if (x->option_of_slowQcifMPI)
        {
            ExtensionPrep(map, stream);
            x->slowQcifMPI = (uint16) GetInteger(1, 3600, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_slowCifMPI = SigMapValue(2, map);
        if (x->option_of_slowCifMPI)
        {
            ExtensionPrep(map, stream);
            x->slowCifMPI = (uint16) GetInteger(1, 3600, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_slowCif4MPI = SigMapValue(3, map);
        if (x->option_of_slowCif4MPI)
        {
            ExtensionPrep(map, stream);
            x->slowCif4MPI = (uint16) GetInteger(1, 3600, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_slowCif16MPI = SigMapValue(4, map);
        if (x->option_of_slowCif16MPI)
        {
            ExtensionPrep(map, stream);
            x->slowCif16MPI = (uint16) GetInteger(1, 3600, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_errorCompensation = SigMapValue(5, map);
        if (x->option_of_errorCompensation)
        {
            ExtensionPrep(map, stream);
            x->errorCompensation = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_enhancementLayerInfo = SigMapValue(6, map);
        if (x->option_of_enhancementLayerInfo)
        {
            ExtensionPrep(map, stream);
            Decode_EnhancementLayerInfo(&x->enhancementLayerInfo, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_h263Options = SigMapValue(7, map);
        if (x->option_of_h263Options)
        {
            ExtensionPrep(map, stream);
            Decode_H263Options(&x->h263Options, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H263VideoCapability: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for EnhancementLayerInfo (SEQUENCE)  */
/* <===============================================> */
void Decode_EnhancementLayerInfo(PS_EnhancementLayerInfo x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_snrEnhancement = GetBoolean(stream);
    x->option_of_spatialEnhancement = GetBoolean(stream);
    x->option_of_bPictureEnhancement = GetBoolean(stream);
    x->baseBitRateConstrained = GetBoolean(stream);
    if (x->option_of_snrEnhancement)
    {
        x->size_of_snrEnhancement = (uint8) GetInteger(1, 14, stream);
        x->snrEnhancement = (PS_EnhancementOptions)
                            OSCL_DEFAULT_MALLOC(x->size_of_snrEnhancement * sizeof(S_EnhancementOptions));
        for (i = 0;i < x->size_of_snrEnhancement;++i)
        {
            Decode_EnhancementOptions(x->snrEnhancement + i, stream);
        }
    }
    if (x->option_of_spatialEnhancement)
    {
        x->size_of_spatialEnhancement = (uint8) GetInteger(1, 14, stream);
        x->spatialEnhancement = (PS_EnhancementOptions)
                                OSCL_DEFAULT_MALLOC(x->size_of_spatialEnhancement * sizeof(S_EnhancementOptions));
        for (i = 0;i < x->size_of_spatialEnhancement;++i)
        {
            Decode_EnhancementOptions(x->spatialEnhancement + i, stream);
        }
    }
    if (x->option_of_bPictureEnhancement)
    {
        x->size_of_bPictureEnhancement = (uint8) GetInteger(1, 14, stream);
        x->bPictureEnhancement = (PS_BEnhancementParameters)
                                 OSCL_DEFAULT_MALLOC(x->size_of_bPictureEnhancement * sizeof(S_BEnhancementParameters));
        for (i = 0;i < x->size_of_bPictureEnhancement;++i)
        {
            Decode_BEnhancementParameters(x->bPictureEnhancement + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EnhancementLayerInfo: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for BEnhancementParameters (SEQUENCE)  */
/* <=================================================> */
void Decode_BEnhancementParameters(PS_BEnhancementParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_EnhancementOptions(&x->enhancementOptions, stream);
    x->numberOfBPictures = (uint8) GetInteger(1, 64, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_BEnhancementParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for EnhancementOptions (SEQUENCE)  */
/* <=============================================> */
void Decode_EnhancementOptions(PS_EnhancementOptions x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_sqcifMPI = GetBoolean(stream);
    x->option_of_qcifMPI = GetBoolean(stream);
    x->option_of_cifMPI = GetBoolean(stream);
    x->option_of_cif4MPI = GetBoolean(stream);
    x->option_of_cif16MPI = GetBoolean(stream);
    x->option_of_slowSqcifMPI = GetBoolean(stream);
    x->option_of_slowQcifMPI = GetBoolean(stream);
    x->option_of_slowCifMPI = GetBoolean(stream);
    x->option_of_slowCif4MPI = GetBoolean(stream);
    x->option_of_slowCif16MPI = GetBoolean(stream);
    x->option_of_h263Options = GetBoolean(stream);
    if (x->option_of_sqcifMPI)
    {
        x->sqcifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_qcifMPI)
    {
        x->qcifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cifMPI)
    {
        x->cifMPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cif4MPI)
    {
        x->cif4MPI = (uint8) GetInteger(1, 32, stream);
    }
    if (x->option_of_cif16MPI)
    {
        x->cif16MPI = (uint8) GetInteger(1, 32, stream);
    }
    x->maxBitRate = GetInteger(1, 192400, stream);
    x->unrestrictedVector = GetBoolean(stream);
    x->arithmeticCoding = GetBoolean(stream);
    x->temporalSpatialTradeOffCapability = GetBoolean(stream);
    if (x->option_of_slowSqcifMPI)
    {
        x->slowSqcifMPI = (uint16) GetInteger(1, 3600, stream);
    }
    if (x->option_of_slowQcifMPI)
    {
        x->slowQcifMPI = (uint16) GetInteger(1, 3600, stream);
    }
    if (x->option_of_slowCifMPI)
    {
        x->slowCifMPI = (uint16) GetInteger(1, 3600, stream);
    }
    if (x->option_of_slowCif4MPI)
    {
        x->slowCif4MPI = (uint16) GetInteger(1, 3600, stream);
    }
    if (x->option_of_slowCif16MPI)
    {
        x->slowCif16MPI = (uint16) GetInteger(1, 3600, stream);
    }
    x->errorCompensation = GetBoolean(stream);
    if (x->option_of_h263Options)
    {
        Decode_H263Options(&x->h263Options, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EnhancementOptions: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for H263Options (SEQUENCE)  */
/* <======================================> */
void Decode_H263Options(PS_H263Options x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_transparencyParameters = GetBoolean(stream);
    x->option_of_refPictureSelection = GetBoolean(stream);
    x->option_of_customPictureClockFrequency = GetBoolean(stream);
    x->option_of_customPictureFormat = GetBoolean(stream);
    x->option_of_modeCombos = GetBoolean(stream);
    x->advancedIntraCodingMode = GetBoolean(stream);
    x->deblockingFilterMode = GetBoolean(stream);
    x->improvedPBFramesMode = GetBoolean(stream);
    x->unlimitedMotionVectors = GetBoolean(stream);
    x->fullPictureFreeze = GetBoolean(stream);
    x->partialPictureFreezeAndRelease = GetBoolean(stream);
    x->resizingPartPicFreezeAndRelease = GetBoolean(stream);
    x->fullPictureSnapshot = GetBoolean(stream);
    x->partialPictureSnapshot = GetBoolean(stream);
    x->videoSegmentTagging = GetBoolean(stream);
    x->progressiveRefinement = GetBoolean(stream);
    x->dynamicPictureResizingByFour = GetBoolean(stream);
    x->dynamicPictureResizingSixteenthPel = GetBoolean(stream);
    x->dynamicWarpingHalfPel = GetBoolean(stream);
    x->dynamicWarpingSixteenthPel = GetBoolean(stream);
    x->independentSegmentDecoding = GetBoolean(stream);
    x->slicesInOrder_NonRect = GetBoolean(stream);
    x->slicesInOrder_Rect = GetBoolean(stream);
    x->slicesNoOrder_NonRect = GetBoolean(stream);
    x->slicesNoOrder_Rect = GetBoolean(stream);
    x->alternateInterVLCMode = GetBoolean(stream);
    x->modifiedQuantizationMode = GetBoolean(stream);
    x->reducedResolutionUpdate = GetBoolean(stream);
    if (x->option_of_transparencyParameters)
    {
        Decode_TransparencyParameters(&x->transparencyParameters, stream);
    }
    x->separateVideoBackChannel = GetBoolean(stream);
    if (x->option_of_refPictureSelection)
    {
        Decode_RefPictureSelection(&x->refPictureSelection, stream);
    }
    if (x->option_of_customPictureClockFrequency)
    {
        x->size_of_customPictureClockFrequency = (uint8) GetInteger(1, 16, stream);
        x->customPictureClockFrequency = (PS_CustomPictureClockFrequency)
                                         OSCL_DEFAULT_MALLOC(x->size_of_customPictureClockFrequency * sizeof(S_CustomPictureClockFrequency));
        for (i = 0;i < x->size_of_customPictureClockFrequency;++i)
        {
            Decode_CustomPictureClockFrequency(x->customPictureClockFrequency + i, stream);
        }
    }
    if (x->option_of_customPictureFormat)
    {
        x->size_of_customPictureFormat = (uint8) GetInteger(1, 16, stream);
        x->customPictureFormat = (PS_CustomPictureFormat)
                                 OSCL_DEFAULT_MALLOC(x->size_of_customPictureFormat * sizeof(S_CustomPictureFormat));
        for (i = 0;i < x->size_of_customPictureFormat;++i)
        {
            Decode_CustomPictureFormat(x->customPictureFormat + i, stream);
        }
    }
    if (x->option_of_modeCombos)
    {
        x->size_of_modeCombos = (uint8) GetInteger(1, 16, stream);
        x->modeCombos = (PS_H263VideoModeCombos)
                        OSCL_DEFAULT_MALLOC(x->size_of_modeCombos * sizeof(S_H263VideoModeCombos));
        for (i = 0;i < x->size_of_modeCombos;++i)
        {
            Decode_H263VideoModeCombos(x->modeCombos + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H263Options: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for TransparencyParameters (SEQUENCE)  */
/* <=================================================> */
void Decode_TransparencyParameters(PS_TransparencyParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->presentationOrder = (uint16) GetInteger(1, 256, stream);
    x->offset_x = GetSignedInteger(-262144, 262143, stream);
    x->offset_y = GetSignedInteger(-262144, 262143, stream);
    x->scale_x = (uint8) GetInteger(1, 255, stream);
    x->scale_y = (uint8) GetInteger(1, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TransparencyParameters: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for RefPictureSelection (SEQUENCE)  */
/* <==============================================> */
void Decode_RefPictureSelection(PS_RefPictureSelection x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_additionalPictureMemory = GetBoolean(stream);
    if (x->option_of_additionalPictureMemory)
    {
        Decode_AdditionalPictureMemory(&x->additionalPictureMemory, stream);
    }
    x->videoMux = GetBoolean(stream);
    Decode_VideoBackChannelSend(&x->videoBackChannelSend, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RefPictureSelection: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for VideoBackChannelSend (CHOICE)  */
/* <=============================================> */
void Decode_VideoBackChannelSend(PS_VideoBackChannelSend x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (none is NULL) */
            break;
        case 1:
            /* (ackMessageOnly is NULL) */
            break;
        case 2:
            /* (nackMessageOnly is NULL) */
            break;
        case 3:
            /* (ackOrNackMessageOnly is NULL) */
            break;
        case 4:
            /* (ackAndNackMessage is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_VideoBackChannelSend: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================================> */
/*  PER-Decoder for AdditionalPictureMemory (SEQUENCE)  */
/* <==================================================> */
void Decode_AdditionalPictureMemory(PS_AdditionalPictureMemory x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_sqcifAdditionalPictureMemory = GetBoolean(stream);
    x->option_of_qcifAdditionalPictureMemory = GetBoolean(stream);
    x->option_of_cifAdditionalPictureMemory = GetBoolean(stream);
    x->option_of_cif4AdditionalPictureMemory = GetBoolean(stream);
    x->option_of_cif16AdditionalPictureMemory = GetBoolean(stream);
    x->option_of_bigCpfAdditionalPictureMemory = GetBoolean(stream);
    if (x->option_of_sqcifAdditionalPictureMemory)
    {
        x->sqcifAdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    if (x->option_of_qcifAdditionalPictureMemory)
    {
        x->qcifAdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    if (x->option_of_cifAdditionalPictureMemory)
    {
        x->cifAdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    if (x->option_of_cif4AdditionalPictureMemory)
    {
        x->cif4AdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    if (x->option_of_cif16AdditionalPictureMemory)
    {
        x->cif16AdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    if (x->option_of_bigCpfAdditionalPictureMemory)
    {
        x->bigCpfAdditionalPictureMemory = (uint16) GetInteger(1, 256, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_AdditionalPictureMemory: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for CustomPictureClockFrequency (SEQUENCE)  */
/* <======================================================> */
void Decode_CustomPictureClockFrequency(PS_CustomPictureClockFrequency x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_sqcifMPI = GetBoolean(stream);
    x->option_of_qcifMPI = GetBoolean(stream);
    x->option_of_cifMPI = GetBoolean(stream);
    x->option_of_cif4MPI = GetBoolean(stream);
    x->option_of_cif16MPI = GetBoolean(stream);
    x->clockConversionCode = (uint16) GetInteger(1000, 1001, stream);
    x->clockDivisor = (uint8) GetInteger(1, 127, stream);
    if (x->option_of_sqcifMPI)
    {
        x->sqcifMPI = (uint16) GetInteger(1, 2048, stream);
    }
    if (x->option_of_qcifMPI)
    {
        x->qcifMPI = (uint16) GetInteger(1, 2048, stream);
    }
    if (x->option_of_cifMPI)
    {
        x->cifMPI = (uint16) GetInteger(1, 2048, stream);
    }
    if (x->option_of_cif4MPI)
    {
        x->cif4MPI = (uint16) GetInteger(1, 2048, stream);
    }
    if (x->option_of_cif16MPI)
    {
        x->cif16MPI = (uint16) GetInteger(1, 2048, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CustomPictureClockFrequency: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for CustomPictureFormat (SEQUENCE)  */
/* <==============================================> */
void Decode_CustomPictureFormat(PS_CustomPictureFormat x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maxCustomPictureWidth = (uint16) GetInteger(1, 2048, stream);
    x->maxCustomPictureHeight = (uint16) GetInteger(1, 2048, stream);
    x->minCustomPictureWidth = (uint16) GetInteger(1, 2048, stream);
    x->minCustomPictureHeight = (uint16) GetInteger(1, 2048, stream);
    Decode_MPI(&x->mPI, stream);
    Decode_PixelAspectInformation(&x->pixelAspectInformation, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CustomPictureFormat: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for PixelAspectInformation (CHOICE)  */
/* <===============================================> */
void Decode_PixelAspectInformation(PS_PixelAspectInformation x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->anyPixelAspectRatio = GetBoolean(stream);
            break;
        case 1:
            x->size = (uint8) GetInteger(1, 14, stream);
            x->pixelAspectCode = (uint32*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint32));
            for (i = 0;i < x->size;++i)
            {
                x->pixelAspectCode[i] = GetInteger(1, 14, stream);
            }
            break;
        case 2:
            x->size = (uint16) GetInteger(1, 256, stream);
            x->extendedPAR = (PS_ExtendedPARItem)
                             OSCL_DEFAULT_MALLOC(x->size * sizeof(S_ExtendedPARItem));
            for (i = 0;i < x->size;++i)
            {
                Decode_ExtendedPARItem(x->extendedPAR + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_PixelAspectInformation: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for ExtendedPARItem (SEQUENCE)  */
/* <==========================================> */
void Decode_ExtendedPARItem(PS_ExtendedPARItem x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->width = (uint8) GetInteger(1, 255, stream);
    x->height = (uint8) GetInteger(1, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ExtendedPARItem: Unknown extensions (skipped)");
        }
    }
}

/* <==============================> */
/*  PER-Decoder for MPI (SEQUENCE)  */
/* <==============================> */
void Decode_MPI(PS_MPI x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_standardMPI = GetBoolean(stream);
    x->option_of_customPCF = GetBoolean(stream);
    if (x->option_of_standardMPI)
    {
        x->standardMPI = (uint8) GetInteger(1, 31, stream);
    }
    if (x->option_of_customPCF)
    {
        x->size_of_customPCF = (uint8) GetInteger(1, 16, stream);
        x->customPCF = (PS_CustomPCFItem)
                       OSCL_DEFAULT_MALLOC(x->size_of_customPCF * sizeof(S_CustomPCFItem));
        for (i = 0;i < x->size_of_customPCF;++i)
        {
            Decode_CustomPCFItem(x->customPCF + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MPI: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for CustomPCFItem (SEQUENCE)  */
/* <========================================> */
void Decode_CustomPCFItem(PS_CustomPCFItem x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->clockConversionCode = (uint16) GetInteger(1000, 1001, stream);
    x->clockDivisor = (uint8) GetInteger(1, 127, stream);
    x->customMPI = (uint16) GetInteger(1, 2048, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CustomPCFItem: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for H263VideoModeCombos (SEQUENCE)  */
/* <==============================================> */
void Decode_H263VideoModeCombos(PS_H263VideoModeCombos x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_H263ModeComboFlags(&x->h263VideoUncoupledModes, stream);
    x->size_of_h263VideoCoupledModes = (uint8) GetInteger(1, 16, stream);
    x->h263VideoCoupledModes = (PS_H263ModeComboFlags)
                               OSCL_DEFAULT_MALLOC(x->size_of_h263VideoCoupledModes * sizeof(S_H263ModeComboFlags));
    for (i = 0;i < x->size_of_h263VideoCoupledModes;++i)
    {
        Decode_H263ModeComboFlags(x->h263VideoCoupledModes + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H263VideoModeCombos: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for H263ModeComboFlags (SEQUENCE)  */
/* <=============================================> */
void Decode_H263ModeComboFlags(PS_H263ModeComboFlags x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->unrestrictedVector = GetBoolean(stream);
    x->arithmeticCoding = GetBoolean(stream);
    x->advancedPrediction = GetBoolean(stream);
    x->pbFrames = GetBoolean(stream);
    x->advancedIntraCodingMode = GetBoolean(stream);
    x->deblockingFilterMode = GetBoolean(stream);
    x->unlimitedMotionVectors = GetBoolean(stream);
    x->slicesInOrder_NonRect = GetBoolean(stream);
    x->slicesInOrder_Rect = GetBoolean(stream);
    x->slicesNoOrder_NonRect = GetBoolean(stream);
    x->slicesNoOrder_Rect = GetBoolean(stream);
    x->improvedPBFramesMode = GetBoolean(stream);
    x->referencePicSelect = GetBoolean(stream);
    x->dynamicPictureResizingByFour = GetBoolean(stream);
    x->dynamicPictureResizingSixteenthPel = GetBoolean(stream);
    x->dynamicWarpingHalfPel = GetBoolean(stream);
    x->dynamicWarpingSixteenthPel = GetBoolean(stream);
    x->reducedResolutionUpdate = GetBoolean(stream);
    x->independentSegmentDecoding = GetBoolean(stream);
    x->alternateInterVLCMode = GetBoolean(stream);
    x->modifiedQuantizationMode = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H263ModeComboFlags: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for IS11172VideoCapability (SEQUENCE)  */
/* <=================================================> */
void Decode_IS11172VideoCapability(PS_IS11172VideoCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_videoBitRate = GetBoolean(stream);
    x->option_of_vbvBufferSize = GetBoolean(stream);
    x->option_of_samplesPerLine = GetBoolean(stream);
    x->option_of_linesPerFrame = GetBoolean(stream);
    x->option_of_pictureRate = GetBoolean(stream);
    x->option_of_luminanceSampleRate = GetBoolean(stream);
    x->constrainedBitstream = GetBoolean(stream);
    if (x->option_of_videoBitRate)
    {
        x->videoBitRate = GetInteger(0, 1073741823, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        x->vbvBufferSize = GetInteger(0, 262143, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        x->samplesPerLine = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        x->linesPerFrame = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_pictureRate)
    {
        x->pictureRate = (uint8) GetInteger(0, 15, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        x->luminanceSampleRate = GetInteger(0, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS11172VideoCapability: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for AudioCapability (CHOICE)  */
/* <========================================> */
void Decode_AudioCapability(PS_AudioCapability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(14, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->g711Alaw64k = (uint16) GetInteger(1, 256, stream);
            break;
        case 2:
            x->g711Alaw56k = (uint16) GetInteger(1, 256, stream);
            break;
        case 3:
            x->g711Ulaw64k = (uint16) GetInteger(1, 256, stream);
            break;
        case 4:
            x->g711Ulaw56k = (uint16) GetInteger(1, 256, stream);
            break;
        case 5:
            x->g722_64k = (uint16) GetInteger(1, 256, stream);
            break;
        case 6:
            x->g722_56k = (uint16) GetInteger(1, 256, stream);
            break;
        case 7:
            x->g722_48k = (uint16) GetInteger(1, 256, stream);
            break;
        case 8:
            x->g7231 = (PS_G7231) OSCL_DEFAULT_MALLOC(sizeof(S_G7231));
            Decode_G7231(x->g7231, stream);
            break;
        case 9:
            x->g728 = (uint16) GetInteger(1, 256, stream);
            break;
        case 10:
            x->g729 = (uint16) GetInteger(1, 256, stream);
            break;
        case 11:
            x->g729AnnexA = (uint16) GetInteger(1, 256, stream);
            break;
        case 12:
            x->is11172AudioCapability = (PS_IS11172AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_IS11172AudioCapability));
            Decode_IS11172AudioCapability(x->is11172AudioCapability, stream);
            break;
        case 13:
            x->is13818AudioCapability = (PS_IS13818AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_IS13818AudioCapability));
            Decode_IS13818AudioCapability(x->is13818AudioCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            GetLengthDet(stream);
            x->g729wAnnexB = (uint16) GetInteger(1, 256, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->g729AnnexAwAnnexB = (uint16) GetInteger(1, 256, stream);
            ReadRemainingBits(stream);
            break;
        case 16:
            GetLengthDet(stream);
            x->g7231AnnexCCapability = (PS_G7231AnnexCCapability) OSCL_DEFAULT_MALLOC(sizeof(S_G7231AnnexCCapability));
            Decode_G7231AnnexCCapability(x->g7231AnnexCCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            x->gsmFullRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmFullRate, stream);
            ReadRemainingBits(stream);
            break;
        case 18:
            GetLengthDet(stream);
            x->gsmHalfRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmHalfRate, stream);
            ReadRemainingBits(stream);
            break;
        case 19:
            GetLengthDet(stream);
            x->gsmEnhancedFullRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmEnhancedFullRate, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            x->genericAudioCapability = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericAudioCapability, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            x->g729Extensions = (PS_G729Extensions) OSCL_DEFAULT_MALLOC(sizeof(S_G729Extensions));
            Decode_G729Extensions(x->g729Extensions, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_AudioCapability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================> */
/*  PER-Decoder for G7231 (SEQUENCE)  */
/* <================================> */
void Decode_G7231(PS_G7231 x, PS_InStream stream)
{
    x->maxAl_sduAudioFrames = (uint16) GetInteger(1, 256, stream);
    x->silenceSuppression = GetBoolean(stream);
}

/* <=========================================> */
/*  PER-Decoder for G729Extensions (SEQUENCE)  */
/* <=========================================> */
void Decode_G729Extensions(PS_G729Extensions x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_audioUnit = GetBoolean(stream);
    if (x->option_of_audioUnit)
    {
        x->audioUnit = (uint16) GetInteger(1, 256, stream);
    }
    x->annexA = GetBoolean(stream);
    x->annexB = GetBoolean(stream);
    x->annexD = GetBoolean(stream);
    x->annexE = GetBoolean(stream);
    x->annexF = GetBoolean(stream);
    x->annexG = GetBoolean(stream);
    x->annexH = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_G729Extensions: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for G7231AnnexCCapability (SEQUENCE)  */
/* <================================================> */
void Decode_G7231AnnexCCapability(PS_G7231AnnexCCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_g723AnnexCAudioMode = GetBoolean(stream);
    x->maxAl_sduAudioFrames = (uint16) GetInteger(1, 256, stream);
    x->silenceSuppression = GetBoolean(stream);
    if (x->option_of_g723AnnexCAudioMode)
    {
        Decode_G723AnnexCAudioMode(&x->g723AnnexCAudioMode, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_G7231AnnexCCapability: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for G723AnnexCAudioMode (SEQUENCE)  */
/* <==============================================> */
void Decode_G723AnnexCAudioMode(PS_G723AnnexCAudioMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->highRateMode0 = (uint8) GetInteger(27, 78, stream);
    x->highRateMode1 = (uint8) GetInteger(27, 78, stream);
    x->lowRateMode0 = (uint8) GetInteger(23, 66, stream);
    x->lowRateMode1 = (uint8) GetInteger(23, 66, stream);
    x->sidMode0 = (uint8) GetInteger(6, 17, stream);
    x->sidMode1 = (uint8) GetInteger(6, 17, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_G723AnnexCAudioMode: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for IS11172AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Decode_IS11172AudioCapability(PS_IS11172AudioCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->audioLayer1 = GetBoolean(stream);
    x->audioLayer2 = GetBoolean(stream);
    x->audioLayer3 = GetBoolean(stream);
    x->audioSampling32k = GetBoolean(stream);
    x->audioSampling44k1 = GetBoolean(stream);
    x->audioSampling48k = GetBoolean(stream);
    x->singleChannel = GetBoolean(stream);
    x->twoChannels = GetBoolean(stream);
    x->bitRate = (uint16) GetInteger(1, 448, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS11172AudioCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for IS13818AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Decode_IS13818AudioCapability(PS_IS13818AudioCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->audioLayer1 = GetBoolean(stream);
    x->audioLayer2 = GetBoolean(stream);
    x->audioLayer3 = GetBoolean(stream);
    x->audioSampling16k = GetBoolean(stream);
    x->audioSampling22k05 = GetBoolean(stream);
    x->audioSampling24k = GetBoolean(stream);
    x->audioSampling32k = GetBoolean(stream);
    x->audioSampling44k1 = GetBoolean(stream);
    x->audioSampling48k = GetBoolean(stream);
    x->singleChannel = GetBoolean(stream);
    x->twoChannels = GetBoolean(stream);
    x->threeChannels2_1 = GetBoolean(stream);
    x->threeChannels3_0 = GetBoolean(stream);
    x->fourChannels2_0_2_0 = GetBoolean(stream);
    x->fourChannels2_2 = GetBoolean(stream);
    x->fourChannels3_1 = GetBoolean(stream);
    x->fiveChannels3_0_2_0 = GetBoolean(stream);
    x->fiveChannels3_2 = GetBoolean(stream);
    x->lowFrequencyEnhancement = GetBoolean(stream);
    x->multilingual = GetBoolean(stream);
    x->bitRate = (uint16) GetInteger(1, 1130, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS13818AudioCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for GSMAudioCapability (SEQUENCE)  */
/* <=============================================> */
void Decode_GSMAudioCapability(PS_GSMAudioCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->audioUnitSize = (uint16) GetInteger(1, 256, stream);
    x->comfortNoise = GetBoolean(stream);
    x->scrambled = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_GSMAudioCapability: Unknown extensions (skipped)");
        }
    }
}

/* <====================================================> */
/*  PER-Decoder for DataApplicationCapability (SEQUENCE)  */
/* <====================================================> */
void Decode_DataApplicationCapability(PS_DataApplicationCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Application(&x->application, stream);
    x->maxBitRate = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_DataApplicationCapability: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for Application (CHOICE)  */
/* <====================================> */
void Decode_Application(PS_Application x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->t120 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t120, stream);
            break;
        case 2:
            x->dsm_cc = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->dsm_cc, stream);
            break;
        case 3:
            x->userData = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->userData, stream);
            break;
        case 4:
            x->t84 = (PS_T84) OSCL_DEFAULT_MALLOC(sizeof(S_T84));
            Decode_T84(x->t84, stream);
            break;
        case 5:
            x->t434 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t434, stream);
            break;
        case 6:
            x->h224 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->h224, stream);
            break;
        case 7:
            x->nlpid = (PS_Nlpid) OSCL_DEFAULT_MALLOC(sizeof(S_Nlpid));
            Decode_Nlpid(x->nlpid, stream);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            x->h222DataPartitioning = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->h222DataPartitioning, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            GetLengthDet(stream);
            x->t30fax = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t30fax, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->t140 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t140, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->t38fax = (PS_T38fax) OSCL_DEFAULT_MALLOC(sizeof(S_T38fax));
            Decode_T38fax(x->t38fax, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->genericDataCapability = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericDataCapability, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_Application: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=================================> */
/*  PER-Decoder for T38fax (SEQUENCE)  */
/* <=================================> */
void Decode_T38fax(PS_T38fax x, PS_InStream stream)
{
    Decode_DataProtocolCapability(&x->t38FaxProtocol, stream);
    Decode_T38FaxProfile(&x->t38FaxProfile, stream);
}

/* <================================> */
/*  PER-Decoder for Nlpid (SEQUENCE)  */
/* <================================> */
void Decode_Nlpid(PS_Nlpid x, PS_InStream stream)
{
    Decode_DataProtocolCapability(&x->nlpidProtocol, stream);
    GetOctetString(1, 0, 0, &x->nlpidData, stream);
}

/* <==============================> */
/*  PER-Decoder for T84 (SEQUENCE)  */
/* <==============================> */
void Decode_T84(PS_T84 x, PS_InStream stream)
{
    Decode_DataProtocolCapability(&x->t84Protocol, stream);
    Decode_T84Profile(&x->t84Profile, stream);
}

/* <===============================================> */
/*  PER-Decoder for DataProtocolCapability (CHOICE)  */
/* <===============================================> */
void Decode_DataProtocolCapability(PS_DataProtocolCapability x, PS_InStream stream)
{
    x->index = GetChoiceIndex(7, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (v14buffered is NULL) */
            break;
        case 2:
            /* (v42lapm is NULL) */
            break;
        case 3:
            /* (hdlcFrameTunnelling is NULL) */
            break;
        case 4:
            /* (h310SeparateVCStack is NULL) */
            break;
        case 5:
            /* (h310SingleVCStack is NULL) */
            break;
        case 6:
            /* (transparent is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            GetLengthDet(stream);
            /* (segmentationAndReassembly is NULL) */
            SkipOneOctet(stream);
            break;
        case 8:
            GetLengthDet(stream);
            /* (hdlcFrameTunnelingwSAR is NULL) */
            SkipOneOctet(stream);
            break;
        case 9:
            GetLengthDet(stream);
            /* (v120 is NULL) */
            SkipOneOctet(stream);
            break;
        case 10:
            GetLengthDet(stream);
            /* (separateLANStack is NULL) */
            SkipOneOctet(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->v76wCompression = (PS_V76wCompression) OSCL_DEFAULT_MALLOC(sizeof(S_V76wCompression));
            Decode_V76wCompression(x->v76wCompression, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            /* (tcp is NULL) */
            SkipOneOctet(stream);
            break;
        case 13:
            GetLengthDet(stream);
            /* (udp is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_DataProtocolCapability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for V76wCompression (CHOICE)  */
/* <========================================> */
void Decode_V76wCompression(PS_V76wCompression x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->transmitCompression = (PS_CompressionType) OSCL_DEFAULT_MALLOC(sizeof(S_CompressionType));
            Decode_CompressionType(x->transmitCompression, stream);
            break;
        case 1:
            x->receiveCompression = (PS_CompressionType) OSCL_DEFAULT_MALLOC(sizeof(S_CompressionType));
            Decode_CompressionType(x->receiveCompression, stream);
            break;
        case 2:
            x->transmitAndReceiveCompression = (PS_CompressionType) OSCL_DEFAULT_MALLOC(sizeof(S_CompressionType));
            Decode_CompressionType(x->transmitAndReceiveCompression, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_V76wCompression: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for CompressionType (CHOICE)  */
/* <========================================> */
void Decode_CompressionType(PS_CompressionType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            x->v42bis = (PS_V42bis) OSCL_DEFAULT_MALLOC(sizeof(S_V42bis));
            Decode_V42bis(x->v42bis, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CompressionType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=================================> */
/*  PER-Decoder for V42bis (SEQUENCE)  */
/* <=================================> */
void Decode_V42bis(PS_V42bis x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->numberOfCodewords = GetInteger(1, 65536, stream);
    x->maximumStringLength = (uint16) GetInteger(1, 256, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V42bis: Unknown extensions (skipped)");
        }
    }
}

/* <===================================> */
/*  PER-Decoder for T84Profile (CHOICE)  */
/* <===================================> */
void Decode_T84Profile(PS_T84Profile x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (t84Unrestricted is NULL) */
            break;
        case 1:
            x->t84Restricted = (PS_T84Restricted) OSCL_DEFAULT_MALLOC(sizeof(S_T84Restricted));
            Decode_T84Restricted(x->t84Restricted, stream);
            break;
        default:
            ErrorMessageAndLeave("Decode_T84Profile: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Decoder for T84Restricted (SEQUENCE)  */
/* <========================================> */
void Decode_T84Restricted(PS_T84Restricted x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->qcif = GetBoolean(stream);
    x->cif = GetBoolean(stream);
    x->ccir601Seq = GetBoolean(stream);
    x->ccir601Prog = GetBoolean(stream);
    x->hdtvSeq = GetBoolean(stream);
    x->hdtvProg = GetBoolean(stream);
    x->g3FacsMH200x100 = GetBoolean(stream);
    x->g3FacsMH200x200 = GetBoolean(stream);
    x->g4FacsMMR200x100 = GetBoolean(stream);
    x->g4FacsMMR200x200 = GetBoolean(stream);
    x->jbig200x200Seq = GetBoolean(stream);
    x->jbig200x200Prog = GetBoolean(stream);
    x->jbig300x300Seq = GetBoolean(stream);
    x->jbig300x300Prog = GetBoolean(stream);
    x->digPhotoLow = GetBoolean(stream);
    x->digPhotoMedSeq = GetBoolean(stream);
    x->digPhotoMedProg = GetBoolean(stream);
    x->digPhotoHighSeq = GetBoolean(stream);
    x->digPhotoHighProg = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_T84Restricted: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for T38FaxProfile (SEQUENCE)  */
/* <========================================> */
void Decode_T38FaxProfile(PS_T38FaxProfile x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->fillBitRemoval = GetBoolean(stream);
    x->transcodingJBIG = GetBoolean(stream);
    x->transcodingMMR = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_version = OFF;
    x->option_of_t38FaxRateManagement = OFF;
    x->option_of_t38FaxUdpOptions = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_version = SigMapValue(0, map);
        if (x->option_of_version)
        {
            ExtensionPrep(map, stream);
            x->version = (uint8) GetInteger(0, 255, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_t38FaxRateManagement = SigMapValue(1, map);
        if (x->option_of_t38FaxRateManagement)
        {
            ExtensionPrep(map, stream);
            Decode_T38FaxRateManagement(&x->t38FaxRateManagement, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_t38FaxUdpOptions = SigMapValue(2, map);
        if (x->option_of_t38FaxUdpOptions)
        {
            ExtensionPrep(map, stream);
            Decode_T38FaxUdpOptions(&x->t38FaxUdpOptions, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_T38FaxProfile: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for T38FaxRateManagement (CHOICE)  */
/* <=============================================> */
void Decode_T38FaxRateManagement(PS_T38FaxRateManagement x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (localTCF is NULL) */
            break;
        case 1:
            /* (transferredTCF is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_T38FaxRateManagement: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===========================================> */
/*  PER-Decoder for T38FaxUdpOptions (SEQUENCE)  */
/* <===========================================> */
void Decode_T38FaxUdpOptions(PS_T38FaxUdpOptions x, PS_InStream stream)
{
    x->option_of_t38FaxMaxBuffer = GetBoolean(stream);
    x->option_of_t38FaxMaxDatagram = GetBoolean(stream);
    if (x->option_of_t38FaxMaxBuffer)
    {
        x->t38FaxMaxBuffer = GetUnboundedInteger(stream);
    }
    if (x->option_of_t38FaxMaxDatagram)
    {
        x->t38FaxMaxDatagram = GetUnboundedInteger(stream);
    }
    Decode_T38FaxUdpEC(&x->t38FaxUdpEC, stream);
}

/* <====================================> */
/*  PER-Decoder for T38FaxUdpEC (CHOICE)  */
/* <====================================> */
void Decode_T38FaxUdpEC(PS_T38FaxUdpEC x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (t38UDPFEC is NULL) */
            break;
        case 1:
            /* (t38UDPRedundancy is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_T38FaxUdpEC: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================================================> */
/*  PER-Decoder for EncryptionAuthenticationAndIntegrity (SEQUENCE)  */
/* <===============================================================> */
void Decode_EncryptionAuthenticationAndIntegrity(PS_EncryptionAuthenticationAndIntegrity x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_encryptionCapability = GetBoolean(stream);
    x->option_of_authenticationCapability = GetBoolean(stream);
    x->option_of_integrityCapability = GetBoolean(stream);
    if (x->option_of_encryptionCapability)
    {
        Decode_EncryptionCapability(&x->encryptionCapability, stream);
    }
    if (x->option_of_authenticationCapability)
    {
        Decode_AuthenticationCapability(&x->authenticationCapability, stream);
    }
    if (x->option_of_integrityCapability)
    {
        Decode_IntegrityCapability(&x->integrityCapability, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EncryptionAuthenticationAndIntegrity: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for EncryptionCapability (SEQUENCE-OF)  */
/* <==================================================> */
void Decode_EncryptionCapability(PS_EncryptionCapability x, PS_InStream stream)
{
    uint16 i;
    x->size = (uint16) GetInteger(1, 256, stream);
    x->item = (PS_MediaEncryptionAlgorithm)
              OSCL_DEFAULT_MALLOC(x->size * sizeof(S_MediaEncryptionAlgorithm));
    for (i = 0;i < x->size;++i)
    {
        Decode_MediaEncryptionAlgorithm(x->item + i, stream);
    }
}

/* <=================================================> */
/*  PER-Decoder for MediaEncryptionAlgorithm (CHOICE)  */
/* <=================================================> */
void Decode_MediaEncryptionAlgorithm(PS_MediaEncryptionAlgorithm x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->algorithm = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            GetObjectID(x->algorithm, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MediaEncryptionAlgorithm: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================================> */
/*  PER-Decoder for AuthenticationCapability (SEQUENCE)  */
/* <===================================================> */
void Decode_AuthenticationCapability(PS_AuthenticationCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        Decode_NonStandardParameter(&x->nonStandard, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_AuthenticationCapability: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for IntegrityCapability (SEQUENCE)  */
/* <==============================================> */
void Decode_IntegrityCapability(PS_IntegrityCapability x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        Decode_NonStandardParameter(&x->nonStandard, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IntegrityCapability: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for UserInputCapability (CHOICE)  */
/* <============================================> */
void Decode_UserInputCapability(PS_UserInputCapability x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(6, 1, stream);
    switch (x->index)
    {
        case 0:
            x->size = (uint8) GetInteger(1, 16, stream);
            x->nonStandard = (PS_NonStandardParameter)
                             OSCL_DEFAULT_MALLOC(x->size * sizeof(S_NonStandardParameter));
            for (i = 0;i < x->size;++i)
            {
                Decode_NonStandardParameter(x->nonStandard + i, stream);
            }
            break;
        case 1:
            /* (basicString is NULL) */
            break;
        case 2:
            /* (iA5String is NULL) */
            break;
        case 3:
            /* (generalString is NULL) */
            break;
        case 4:
            /* (dtmf is NULL) */
            break;
        case 5:
            /* (hookflash is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_UserInputCapability: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================================> */
/*  PER-Decoder for ConferenceCapability (SEQUENCE)  */
/* <===============================================> */
void Decode_ConferenceCapability(PS_ConferenceCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandardData = GetBoolean(stream);
    if (x->option_of_nonStandardData)
    {
        x->size_of_nonStandardData = (uint16) GetLengthDet(stream);
        x->nonStandardData = (PS_NonStandardParameter)
                             OSCL_DEFAULT_MALLOC(x->size_of_nonStandardData * sizeof(S_NonStandardParameter));
        for (i = 0;i < x->size_of_nonStandardData;++i)
        {
            Decode_NonStandardParameter(x->nonStandardData + i, stream);
        }
    }
    x->chairControlCapability = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_VideoIndicateMixingCapability = OFF;
    x->option_of_multipointVisualizationCapability = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_VideoIndicateMixingCapability = SigMapValue(0, map);
        if (x->option_of_VideoIndicateMixingCapability)
        {
            ExtensionPrep(map, stream);
            x->VideoIndicateMixingCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_multipointVisualizationCapability = SigMapValue(1, map);
        if (x->option_of_multipointVisualizationCapability)
        {
            ExtensionPrep(map, stream);
            x->multipointVisualizationCapability = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ConferenceCapability: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for GenericCapability (SEQUENCE)  */
/* <============================================> */
void Decode_GenericCapability(PS_GenericCapability x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_maxBitRate = GetBoolean(stream);
    x->option_of_collapsing = GetBoolean(stream);
    x->option_of_nonCollapsing = GetBoolean(stream);
    x->option_of_nonCollapsingRaw = GetBoolean(stream);
    x->option_of_transport = GetBoolean(stream);
    Decode_CapabilityIdentifier(&x->capabilityIdentifier, stream);
    if (x->option_of_maxBitRate)
    {
        x->maxBitRate = GetInteger(0, 0xffffffff, stream);
    }
    if (x->option_of_collapsing)
    {
        x->size_of_collapsing = (uint16) GetLengthDet(stream);
        x->collapsing = (PS_GenericParameter)
                        OSCL_DEFAULT_MALLOC(x->size_of_collapsing * sizeof(S_GenericParameter));
        for (i = 0;i < x->size_of_collapsing;++i)
        {
            Decode_GenericParameter(x->collapsing + i, stream);
        }
    }
    if (x->option_of_nonCollapsing)
    {
        x->size_of_nonCollapsing = (uint16) GetLengthDet(stream);
        x->nonCollapsing = (PS_GenericParameter)
                           OSCL_DEFAULT_MALLOC(x->size_of_nonCollapsing * sizeof(S_GenericParameter));
        for (i = 0;i < x->size_of_nonCollapsing;++i)
        {
            Decode_GenericParameter(x->nonCollapsing + i, stream);
        }
    }
    if (x->option_of_nonCollapsingRaw)
    {
        GetOctetString(1, 0, 0, &x->nonCollapsingRaw, stream);
    }
    if (x->option_of_transport)
    {
        Decode_DataProtocolCapability(&x->transport, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_GenericCapability: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for CapabilityIdentifier (CHOICE)  */
/* <=============================================> */
void Decode_CapabilityIdentifier(PS_CapabilityIdentifier x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->standard = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            GetObjectID(x->standard, stream);
            break;
        case 1:
            x->h221NonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->h221NonStandard, stream);
            break;
        case 2:
            x->uuid = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 16, 16, x->uuid, stream);
            break;
        case 3:
            x->domainBased = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
            GetCharString("IA5String", 0, 1, 64, NULL, x->domainBased, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CapabilityIdentifier: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===========================================> */
/*  PER-Decoder for GenericParameter (SEQUENCE)  */
/* <===========================================> */
void Decode_GenericParameter(PS_GenericParameter x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_supersedes = GetBoolean(stream);
    Decode_ParameterIdentifier(&x->parameterIdentifier, stream);
    Decode_ParameterValue(&x->parameterValue, stream);
    if (x->option_of_supersedes)
    {
        x->size_of_supersedes = (uint16) GetLengthDet(stream);
        x->supersedes = (PS_ParameterIdentifier)
                        OSCL_DEFAULT_MALLOC(x->size_of_supersedes * sizeof(S_ParameterIdentifier));
        for (i = 0;i < x->size_of_supersedes;++i)
        {
            Decode_ParameterIdentifier(x->supersedes + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_GenericParameter: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for ParameterIdentifier (CHOICE)  */
/* <============================================> */
void Decode_ParameterIdentifier(PS_ParameterIdentifier x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->standard = (uint8) GetInteger(0, 127, stream);
            break;
        case 1:
            x->h221NonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->h221NonStandard, stream);
            break;
        case 2:
            x->uuid = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 16, 16, x->uuid, stream);
            break;
        case 3:
            x->domainBased = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
            GetCharString("IA5String", 0, 1, 64, NULL, x->domainBased, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ParameterIdentifier: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for ParameterValue (CHOICE)  */
/* <=======================================> */
void Decode_ParameterValue(PS_ParameterValue x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(8, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (logical is NULL) */
            break;
        case 1:
            x->booleanArray = (uint8) GetInteger(0, 255, stream);
            break;
        case 2:
            x->unsignedMin = (uint16) GetInteger(0, 65535, stream);
            break;
        case 3:
            x->unsignedMax = (uint16) GetInteger(0, 65535, stream);
            break;
        case 4:
            x->unsigned32Min = GetInteger(0, 0xffffffff, stream);
            break;
        case 5:
            x->unsigned32Max = GetInteger(0, 0xffffffff, stream);
            break;
        case 6:
            x->octetString = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(1, 0, 0, x->octetString, stream);
            break;
        case 7:
            x->size = (uint16) GetLengthDet(stream);
            x->genericParameter = (PS_GenericParameter)
                                  OSCL_DEFAULT_MALLOC(x->size * sizeof(S_GenericParameter));
            for (i = 0;i < x->size;++i)
            {
                Decode_GenericParameter(x->genericParameter + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ParameterValue: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for OpenLogicalChannel (SEQUENCE)  */
/* <=============================================> */
void Decode_OpenLogicalChannel(PS_OpenLogicalChannel x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_reverseLogicalChannelParameters = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_ForwardLogicalChannelParameters(&x->forwardLogicalChannelParameters, stream);
    if (x->option_of_reverseLogicalChannelParameters)
    {
        Decode_ReverseLogicalChannelParameters(&x->reverseLogicalChannelParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_separateStack = OFF;
    x->option_of_encryptionSync = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_separateStack = SigMapValue(0, map);
        if (x->option_of_separateStack)
        {
            ExtensionPrep(map, stream);
            Decode_NetworkAccessParameters(&x->separateStack, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_encryptionSync = SigMapValue(1, map);
        if (x->option_of_encryptionSync)
        {
            ExtensionPrep(map, stream);
            Decode_EncryptionSync(&x->encryptionSync, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_OpenLogicalChannel: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================================> */
/*  PER-Decoder for ReverseLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Decode_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_rlcMultiplexParameters = GetBoolean(stream);
    Decode_DataType(&x->dataType, stream);
    if (x->option_of_rlcMultiplexParameters)
    {
        Decode_RlcMultiplexParameters(&x->rlcMultiplexParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_reverseLogicalChannelDependency = OFF;
    x->option_of_replacementFor = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_reverseLogicalChannelDependency = SigMapValue(0, map);
        if (x->option_of_reverseLogicalChannelDependency)
        {
            ExtensionPrep(map, stream);
            x->reverseLogicalChannelDependency = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_replacementFor = SigMapValue(1, map);
        if (x->option_of_replacementFor)
        {
            ExtensionPrep(map, stream);
            x->replacementFor = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ReverseLogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for RlcMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Decode_RlcMultiplexParameters(PS_RlcMultiplexParameters x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->h223LogicalChannelParameters = (PS_H223LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223LogicalChannelParameters));
            Decode_H223LogicalChannelParameters(x->h223LogicalChannelParameters, stream);
            break;
        case 1:
            x->v76LogicalChannelParameters = (PS_V76LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_V76LogicalChannelParameters));
            Decode_V76LogicalChannelParameters(x->v76LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            GetLengthDet(stream);
            x->h2250LogicalChannelParameters = (PS_H2250LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H2250LogicalChannelParameters));
            Decode_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_RlcMultiplexParameters: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================================> */
/*  PER-Decoder for ForwardLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Decode_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_portNumber = GetBoolean(stream);
    if (x->option_of_portNumber)
    {
        x->portNumber = (uint16) GetInteger(0, 65535, stream);
    }
    Decode_DataType(&x->dataType, stream);
    Decode_MultiplexParameters(&x->multiplexParameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_forwardLogicalChannelDependency = OFF;
    x->option_of_replacementFor = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_forwardLogicalChannelDependency = SigMapValue(0, map);
        if (x->option_of_forwardLogicalChannelDependency)
        {
            ExtensionPrep(map, stream);
            x->forwardLogicalChannelDependency = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_replacementFor = SigMapValue(1, map);
        if (x->option_of_replacementFor)
        {
            ExtensionPrep(map, stream);
            x->replacementFor = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ForwardLogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for MultiplexParameters (CHOICE)  */
/* <============================================> */
void Decode_MultiplexParameters(PS_MultiplexParameters x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->h222LogicalChannelParameters = (PS_H222LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H222LogicalChannelParameters));
            Decode_H222LogicalChannelParameters(x->h222LogicalChannelParameters, stream);
            break;
        case 1:
            x->h223LogicalChannelParameters = (PS_H223LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223LogicalChannelParameters));
            Decode_H223LogicalChannelParameters(x->h223LogicalChannelParameters, stream);
            break;
        case 2:
            x->v76LogicalChannelParameters = (PS_V76LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_V76LogicalChannelParameters));
            Decode_V76LogicalChannelParameters(x->v76LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            GetLengthDet(stream);
            x->h2250LogicalChannelParameters = (PS_H2250LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H2250LogicalChannelParameters));
            Decode_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, stream);
            ReadRemainingBits(stream);
            break;
        case 4:
            GetLengthDet(stream);
            /* (none is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_MultiplexParameters: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================================> */
/*  PER-Decoder for NetworkAccessParameters (SEQUENCE)  */
/* <==================================================> */
void Decode_NetworkAccessParameters(PS_NetworkAccessParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_distribution = GetBoolean(stream);
    x->option_of_externalReference = GetBoolean(stream);
    if (x->option_of_distribution)
    {
        Decode_Distribution(&x->distribution, stream);
    }
    Decode_NetworkAddress(&x->networkAddress, stream);
    x->associateConference = GetBoolean(stream);
    if (x->option_of_externalReference)
    {
        GetOctetString(0, 1, 255, &x->externalReference, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_t120SetupProcedure = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_t120SetupProcedure = SigMapValue(0, map);
        if (x->option_of_t120SetupProcedure)
        {
            ExtensionPrep(map, stream);
            Decode_T120SetupProcedure(&x->t120SetupProcedure, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_NetworkAccessParameters: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for T120SetupProcedure (CHOICE)  */
/* <===========================================> */
void Decode_T120SetupProcedure(PS_T120SetupProcedure x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (originateCall is NULL) */
            break;
        case 1:
            /* (waitForCall is NULL) */
            break;
        case 2:
            /* (issueQuery is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_T120SetupProcedure: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for NetworkAddress (CHOICE)  */
/* <=======================================> */
void Decode_NetworkAddress(PS_NetworkAddress x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->q2931Address = (PS_Q2931Address) OSCL_DEFAULT_MALLOC(sizeof(S_Q2931Address));
            Decode_Q2931Address(x->q2931Address, stream);
            break;
        case 1:
            x->e164Address = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
            GetCharString("IA5String(SIZE(1..128))", 0, 1, 128, "0123456789#*,", x->e164Address, stream);
            break;
        case 2:
            x->localAreaAddress = (PS_TransportAddress) OSCL_DEFAULT_MALLOC(sizeof(S_TransportAddress));
            Decode_TransportAddress(x->localAreaAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_NetworkAddress: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for Distribution (CHOICE)  */
/* <=====================================> */
void Decode_Distribution(PS_Distribution x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unicast is NULL) */
            break;
        case 1:
            /* (multicast is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Distribution: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for Q2931Address (SEQUENCE)  */
/* <=======================================> */
void Decode_Q2931Address(PS_Q2931Address x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_subaddress = GetBoolean(stream);
    Decode_Address(&x->address, stream);
    if (x->option_of_subaddress)
    {
        GetOctetString(0, 1, 20, &x->subaddress, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Q2931Address: Unknown extensions (skipped)");
        }
    }
}

/* <================================> */
/*  PER-Decoder for Address (CHOICE)  */
/* <================================> */
void Decode_Address(PS_Address x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->internationalNumber = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
            GetCharString("NumericString(SIZE(1..16))", 0, 1, 16, NULL, x->internationalNumber, stream);
            break;
        case 1:
            x->nsapAddress = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 1, 20, x->nsapAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Address: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for V75Parameters (SEQUENCE)  */
/* <========================================> */
void Decode_V75Parameters(PS_V75Parameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->audioHeaderPresent = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V75Parameters: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for DataType (CHOICE)  */
/* <=================================> */
void Decode_DataType(PS_DataType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(6, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (nullData is NULL) */
            break;
        case 2:
            x->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->videoData, stream);
            break;
        case 3:
            x->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->audioData, stream);
            break;
        case 4:
            x->data = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->data, stream);
            break;
        case 5:
            x->encryptionData = (PS_EncryptionMode) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionMode));
            Decode_EncryptionMode(x->encryptionData, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            GetLengthDet(stream);
            x->h235Control = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->h235Control, stream);
            ReadRemainingBits(stream);
            break;
        case 7:
            GetLengthDet(stream);
            x->h235Media = (PS_H235Media) OSCL_DEFAULT_MALLOC(sizeof(S_H235Media));
            Decode_H235Media(x->h235Media, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_DataType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================> */
/*  PER-Decoder for H235Media (SEQUENCE)  */
/* <====================================> */
void Decode_H235Media(PS_H235Media x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    Decode_MediaType(&x->mediaType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H235Media: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for MediaType (CHOICE)  */
/* <==================================> */
void Decode_MediaType(PS_MediaType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->videoData, stream);
            break;
        case 2:
            x->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->audioData, stream);
            break;
        case 3:
            x->data = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->data, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MediaType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================================> */
/*  PER-Decoder for H222LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Decode_H222LogicalChannelParameters(PS_H222LogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_pcr_pid = GetBoolean(stream);
    x->option_of_programDescriptors = GetBoolean(stream);
    x->option_of_streamDescriptors = GetBoolean(stream);
    x->resourceID = (uint16) GetInteger(0, 65535, stream);
    x->subChannelID = (uint16) GetInteger(0, 8191, stream);
    if (x->option_of_pcr_pid)
    {
        x->pcr_pid = (uint16) GetInteger(0, 8191, stream);
    }
    if (x->option_of_programDescriptors)
    {
        GetOctetString(1, 0, 0, &x->programDescriptors, stream);
    }
    if (x->option_of_streamDescriptors)
    {
        GetOctetString(1, 0, 0, &x->streamDescriptors, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H222LogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================================> */
/*  PER-Decoder for H223LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Decode_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_AdaptationLayerType(&x->adaptationLayerType, stream);
    x->segmentableFlag = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223LogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for AdaptationLayerType (CHOICE)  */
/* <============================================> */
void Decode_AdaptationLayerType(PS_AdaptationLayerType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(6, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (al1Framed is NULL) */
            break;
        case 2:
            /* (al1NotFramed is NULL) */
            break;
        case 3:
            /* (al2WithoutSequenceNumbers is NULL) */
            break;
        case 4:
            /* (al2WithSequenceNumbers is NULL) */
            break;
        case 5:
            x->al3 = (PS_Al3) OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
            Decode_Al3(x->al3, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            GetLengthDet(stream);
            x->al1M = (PS_H223AL1MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL1MParameters));
            Decode_H223AL1MParameters(x->al1M, stream);
            ReadRemainingBits(stream);
            break;
        case 7:
            GetLengthDet(stream);
            x->al2M = (PS_H223AL2MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL2MParameters));
            Decode_H223AL2MParameters(x->al2M, stream);
            ReadRemainingBits(stream);
            break;
        case 8:
            GetLengthDet(stream);
            x->al3M = (PS_H223AL3MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL3MParameters));
            Decode_H223AL3MParameters(x->al3M, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_AdaptationLayerType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================> */
/*  PER-Decoder for Al3 (SEQUENCE)  */
/* <==============================> */
void Decode_Al3(PS_Al3 x, PS_InStream stream)
{
    x->controlFieldOctets = (uint8) GetInteger(0, 2, stream);
    x->sendBufferSize = GetInteger(0, 16777215, stream);
}

/* <=============================================> */
/*  PER-Decoder for H223AL1MParameters (SEQUENCE)  */
/* <=============================================> */
void Decode_H223AL1MParameters(PS_H223AL1MParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TransferMode(&x->transferMode, stream);
    Decode_HeaderFEC(&x->headerFEC, stream);
    Decode_CrcLength(&x->crcLength, stream);
    x->rcpcCodeRate = (uint8) GetInteger(8, 32, stream);
    Decode_ArqType(&x->arqType, stream);
    x->alpduInterleaving = GetBoolean(stream);
    x->alsduSplitting = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_rsCodeCorrection = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_rsCodeCorrection = SigMapValue(0, map);
        if (x->option_of_rsCodeCorrection)
        {
            ExtensionPrep(map, stream);
            x->rsCodeCorrection = (uint8) GetInteger(0, 127, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223AL1MParameters: Unknown extensions (skipped)");
        }
    }
}

/* <================================> */
/*  PER-Decoder for ArqType (CHOICE)  */
/* <================================> */
void Decode_ArqType(PS_ArqType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            x->typeIArq = (PS_H223AnnexCArqParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AnnexCArqParameters));
            Decode_H223AnnexCArqParameters(x->typeIArq, stream);
            break;
        case 2:
            x->typeIIArq = (PS_H223AnnexCArqParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AnnexCArqParameters));
            Decode_H223AnnexCArqParameters(x->typeIIArq, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ArqType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for CrcLength (CHOICE)  */
/* <==================================> */
void Decode_CrcLength(PS_CrcLength x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (crc4bit is NULL) */
            break;
        case 1:
            /* (crc12bit is NULL) */
            break;
        case 2:
            /* (crc20bit is NULL) */
            break;
        case 3:
            /* (crc28bit is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            GetLengthDet(stream);
            /* (crc8bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 5:
            GetLengthDet(stream);
            /* (crc16bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 6:
            GetLengthDet(stream);
            /* (crc32bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 7:
            GetLengthDet(stream);
            /* (crcNotUsed is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_CrcLength: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for HeaderFEC (CHOICE)  */
/* <==================================> */
void Decode_HeaderFEC(PS_HeaderFEC x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (sebch16_7 is NULL) */
            break;
        case 1:
            /* (golay24_12 is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_HeaderFEC: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for TransferMode (CHOICE)  */
/* <=====================================> */
void Decode_TransferMode(PS_TransferMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (framed is NULL) */
            break;
        case 1:
            /* (unframed is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_TransferMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for H223AL2MParameters (SEQUENCE)  */
/* <=============================================> */
void Decode_H223AL2MParameters(PS_H223AL2MParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Al2HeaderFEC(&x->al2HeaderFEC, stream);
    x->alpduInterleaving = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223AL2MParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for Al2HeaderFEC (CHOICE)  */
/* <=====================================> */
void Decode_Al2HeaderFEC(PS_Al2HeaderFEC x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (sebch16_5 is NULL) */
            break;
        case 1:
            /* (golay24_12 is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Al2HeaderFEC: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for H223AL3MParameters (SEQUENCE)  */
/* <=============================================> */
void Decode_H223AL3MParameters(PS_H223AL3MParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_HeaderFormat(&x->headerFormat, stream);
    Decode_Al3CrcLength(&x->al3CrcLength, stream);
    x->rcpcCodeRate = (uint8) GetInteger(8, 32, stream);
    Decode_Al3ArqType(&x->al3ArqType, stream);
    x->alpduInterleaving = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_rsCodeCorrection = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_rsCodeCorrection = SigMapValue(0, map);
        if (x->option_of_rsCodeCorrection)
        {
            ExtensionPrep(map, stream);
            x->rsCodeCorrection = (uint8) GetInteger(0, 127, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223AL3MParameters: Unknown extensions (skipped)");
        }
    }
}

/* <===================================> */
/*  PER-Decoder for Al3ArqType (CHOICE)  */
/* <===================================> */
void Decode_Al3ArqType(PS_Al3ArqType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            x->typeIArq = (PS_H223AnnexCArqParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AnnexCArqParameters));
            Decode_H223AnnexCArqParameters(x->typeIArq, stream);
            break;
        case 2:
            x->typeIIArq = (PS_H223AnnexCArqParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AnnexCArqParameters));
            Decode_H223AnnexCArqParameters(x->typeIIArq, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Al3ArqType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for Al3CrcLength (CHOICE)  */
/* <=====================================> */
void Decode_Al3CrcLength(PS_Al3CrcLength x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (crc4bit is NULL) */
            break;
        case 1:
            /* (crc12bit is NULL) */
            break;
        case 2:
            /* (crc20bit is NULL) */
            break;
        case 3:
            /* (crc28bit is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            GetLengthDet(stream);
            /* (crc8bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 5:
            GetLengthDet(stream);
            /* (crc16bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 6:
            GetLengthDet(stream);
            /* (crc32bit is NULL) */
            SkipOneOctet(stream);
            break;
        case 7:
            GetLengthDet(stream);
            /* (crcNotUsed is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_Al3CrcLength: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for HeaderFormat (CHOICE)  */
/* <=====================================> */
void Decode_HeaderFormat(PS_HeaderFormat x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (sebch16_7 is NULL) */
            break;
        case 1:
            /* (golay24_12 is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_HeaderFormat: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================================> */
/*  PER-Decoder for H223AnnexCArqParameters (SEQUENCE)  */
/* <==================================================> */
void Decode_H223AnnexCArqParameters(PS_H223AnnexCArqParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_NumberOfRetransmissions(&x->numberOfRetransmissions, stream);
    x->sendBufferSize = GetInteger(0, 16777215, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223AnnexCArqParameters: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for NumberOfRetransmissions (CHOICE)  */
/* <================================================> */
void Decode_NumberOfRetransmissions(PS_NumberOfRetransmissions x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->finite = (uint8) GetInteger(0, 16, stream);
            break;
        case 1:
            /* (infinite is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_NumberOfRetransmissions: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <======================================================> */
/*  PER-Decoder for V76LogicalChannelParameters (SEQUENCE)  */
/* <======================================================> */
void Decode_V76LogicalChannelParameters(PS_V76LogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_V76HDLCParameters(&x->hdlcParameters, stream);
    Decode_SuspendResume(&x->suspendResume, stream);
    x->uIH = GetBoolean(stream);
    Decode_Mode(&x->mode, stream);
    Decode_V75Parameters(&x->v75Parameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V76LogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=============================> */
/*  PER-Decoder for Mode (CHOICE)  */
/* <=============================> */
void Decode_Mode(PS_Mode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->eRM = (PS_ERM) OSCL_DEFAULT_MALLOC(sizeof(S_ERM));
            Decode_ERM(x->eRM, stream);
            break;
        case 1:
            /* (uNERM is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Mode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================> */
/*  PER-Decoder for ERM (SEQUENCE)  */
/* <==============================> */
void Decode_ERM(PS_ERM x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->windowSize = (uint8) GetInteger(1, 127, stream);
    Decode_Recovery(&x->recovery, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ERM: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for Recovery (CHOICE)  */
/* <=================================> */
void Decode_Recovery(PS_Recovery x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (rej is NULL) */
            break;
        case 1:
            /* (sREJ is NULL) */
            break;
        case 2:
            /* (mSREJ is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Recovery: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <======================================> */
/*  PER-Decoder for SuspendResume (CHOICE)  */
/* <======================================> */
void Decode_SuspendResume(PS_SuspendResume x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noSuspendResume is NULL) */
            break;
        case 1:
            /* (suspendResumewAddress is NULL) */
            break;
        case 2:
            /* (suspendResumewoAddress is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_SuspendResume: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <============================================> */
/*  PER-Decoder for V76HDLCParameters (SEQUENCE)  */
/* <============================================> */
void Decode_V76HDLCParameters(PS_V76HDLCParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_CRCLength(&x->crcLength, stream);
    x->n401 = (uint16) GetInteger(1, 4095, stream);
    x->loopbackTestProcedure = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_V76HDLCParameters: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for CRCLength (CHOICE)  */
/* <==================================> */
void Decode_CRCLength(PS_CRCLength x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (crc8bit is NULL) */
            break;
        case 1:
            /* (crc16bit is NULL) */
            break;
        case 2:
            /* (crc32bit is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CRCLength: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================================> */
/*  PER-Decoder for H2250LogicalChannelParameters (SEQUENCE)  */
/* <========================================================> */
void Decode_H2250LogicalChannelParameters(PS_H2250LogicalChannelParameters x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    x->option_of_associatedSessionID = GetBoolean(stream);
    x->option_of_mediaChannel = GetBoolean(stream);
    x->option_of_mediaGuaranteedDelivery = GetBoolean(stream);
    x->option_of_mediaControlChannel = GetBoolean(stream);
    x->option_of_mediaControlGuaranteedDelivery = GetBoolean(stream);
    x->option_of_silenceSuppression = GetBoolean(stream);
    x->option_of_destination = GetBoolean(stream);
    x->option_of_dynamicRTPPayloadType = GetBoolean(stream);
    x->option_of_mediaPacketization = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        x->size_of_nonStandard = (uint16) GetLengthDet(stream);
        x->nonStandard = (PS_NonStandardParameter)
                         OSCL_DEFAULT_MALLOC(x->size_of_nonStandard * sizeof(S_NonStandardParameter));
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Decode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    x->sessionID = (uint8) GetInteger(0, 255, stream);
    if (x->option_of_associatedSessionID)
    {
        x->associatedSessionID = (uint8) GetInteger(1, 255, stream);
    }
    if (x->option_of_mediaChannel)
    {
        Decode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        x->mediaGuaranteedDelivery = GetBoolean(stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Decode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        x->mediaControlGuaranteedDelivery = GetBoolean(stream);
    }
    if (x->option_of_silenceSuppression)
    {
        x->silenceSuppression = GetBoolean(stream);
    }
    if (x->option_of_destination)
    {
        Decode_TerminalLabel(&x->destination, stream);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        x->dynamicRTPPayloadType = (uint8) GetInteger(96, 127, stream);
    }
    if (x->option_of_mediaPacketization)
    {
        Decode_MediaPacketization(&x->mediaPacketization, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_transportCapability = OFF;
    x->option_of_redundancyEncoding = OFF;
    x->option_of_source = OFF;
    x->option_of_symmetricMediaAddress = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_transportCapability = SigMapValue(0, map);
        if (x->option_of_transportCapability)
        {
            ExtensionPrep(map, stream);
            Decode_TransportCapability(&x->transportCapability, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_redundancyEncoding = SigMapValue(1, map);
        if (x->option_of_redundancyEncoding)
        {
            ExtensionPrep(map, stream);
            Decode_RedundancyEncoding(&x->redundancyEncoding, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_source = SigMapValue(2, map);
        if (x->option_of_source)
        {
            ExtensionPrep(map, stream);
            Decode_TerminalLabel(&x->source, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_symmetricMediaAddress = SigMapValue(3, map);
        if (x->option_of_symmetricMediaAddress)
        {
            ExtensionPrep(map, stream);
            x->symmetricMediaAddress = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H2250LogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for MediaPacketization (CHOICE)  */
/* <===========================================> */
void Decode_MediaPacketization(PS_MediaPacketization x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (h261aVideoPacketization is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            GetLengthDet(stream);
            x->rtpPayloadType = (PS_RTPPayloadType) OSCL_DEFAULT_MALLOC(sizeof(S_RTPPayloadType));
            Decode_RTPPayloadType(x->rtpPayloadType, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_MediaPacketization: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for RTPPayloadType (SEQUENCE)  */
/* <=========================================> */
void Decode_RTPPayloadType(PS_RTPPayloadType x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_payloadType = GetBoolean(stream);
    Decode_PayloadDescriptor(&x->payloadDescriptor, stream);
    if (x->option_of_payloadType)
    {
        x->payloadType = (uint8) GetInteger(0, 127, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RTPPayloadType: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for PayloadDescriptor (CHOICE)  */
/* <==========================================> */
void Decode_PayloadDescriptor(PS_PayloadDescriptor x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandardIdentifier = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandardIdentifier, stream);
            break;
        case 1:
            x->rfc_number = (uint16) GetExtendedInteger(1, 32768, stream);
            break;
        case 2:
            x->oid = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            GetObjectID(x->oid, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_PayloadDescriptor: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for RedundancyEncoding (SEQUENCE)  */
/* <=============================================> */
void Decode_RedundancyEncoding(PS_RedundancyEncoding x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_secondaryEncoding = GetBoolean(stream);
    Decode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    if (x->option_of_secondaryEncoding)
    {
        Decode_DataType(&x->secondaryEncoding, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RedundancyEncoding: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for TransportAddress (CHOICE)  */
/* <=========================================> */
void Decode_TransportAddress(PS_TransportAddress x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->unicastAddress = (PS_UnicastAddress) OSCL_DEFAULT_MALLOC(sizeof(S_UnicastAddress));
            Decode_UnicastAddress(x->unicastAddress, stream);
            break;
        case 1:
            x->multicastAddress = (PS_MulticastAddress) OSCL_DEFAULT_MALLOC(sizeof(S_MulticastAddress));
            Decode_MulticastAddress(x->multicastAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_TransportAddress: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for UnicastAddress (CHOICE)  */
/* <=======================================> */
void Decode_UnicastAddress(PS_UnicastAddress x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->iPAddress = (PS_IPAddress) OSCL_DEFAULT_MALLOC(sizeof(S_IPAddress));
            Decode_IPAddress(x->iPAddress, stream);
            break;
        case 1:
            x->iPXAddress = (PS_IPXAddress) OSCL_DEFAULT_MALLOC(sizeof(S_IPXAddress));
            Decode_IPXAddress(x->iPXAddress, stream);
            break;
        case 2:
            x->iP6Address = (PS_IP6Address) OSCL_DEFAULT_MALLOC(sizeof(S_IP6Address));
            Decode_IP6Address(x->iP6Address, stream);
            break;
        case 3:
            x->netBios = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 16, 16, x->netBios, stream);
            break;
        case 4:
            x->iPSourceRouteAddress = (PS_IPSourceRouteAddress) OSCL_DEFAULT_MALLOC(sizeof(S_IPSourceRouteAddress));
            Decode_IPSourceRouteAddress(x->iPSourceRouteAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            GetLengthDet(stream);
            x->nsap = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 1, 20, x->nsap, stream);
            ReadRemainingBits(stream);
            break;
        case 6:
            GetLengthDet(stream);
            x->nonStandardAddress = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandardAddress, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_UnicastAddress: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================================> */
/*  PER-Decoder for IPSourceRouteAddress (SEQUENCE)  */
/* <===============================================> */
void Decode_IPSourceRouteAddress(PS_IPSourceRouteAddress x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Routing(&x->routing, stream);
    GetOctetString(0, 4, 4, &x->network, stream);
    x->tsapIdentifier = (uint16) GetInteger(0, 65535, stream);
    x->size_of_route = (uint16) GetLengthDet(stream);
    x->route = (PS_OCTETSTRING)
               OSCL_DEFAULT_MALLOC(x->size_of_route * sizeof(S_OCTETSTRING));
    for (i = 0;i < x->size_of_route;++i)
    {
        GetOctetString(0, 4, 4, x->route + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IPSourceRouteAddress: Unknown extensions (skipped)");
        }
    }
}

/* <================================> */
/*  PER-Decoder for Routing (CHOICE)  */
/* <================================> */
void Decode_Routing(PS_Routing x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (strict is NULL) */
            break;
        case 1:
            /* (loose is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Routing: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Decoder for IP6Address (SEQUENCE)  */
/* <=====================================> */
void Decode_IP6Address(PS_IP6Address x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 16, 16, &x->network, stream);
    x->tsapIdentifier = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IP6Address: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for IPXAddress (SEQUENCE)  */
/* <=====================================> */
void Decode_IPXAddress(PS_IPXAddress x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 6, 6, &x->node, stream);
    GetOctetString(0, 4, 4, &x->netnum, stream);
    GetOctetString(0, 2, 2, &x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IPXAddress: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for IPAddress (SEQUENCE)  */
/* <====================================> */
void Decode_IPAddress(PS_IPAddress x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 4, 4, &x->network, stream);
    x->tsapIdentifier = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IPAddress: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for MulticastAddress (CHOICE)  */
/* <=========================================> */
void Decode_MulticastAddress(PS_MulticastAddress x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->maIpAddress = (PS_MaIpAddress) OSCL_DEFAULT_MALLOC(sizeof(S_MaIpAddress));
            Decode_MaIpAddress(x->maIpAddress, stream);
            break;
        case 1:
            x->maIp6Address = (PS_MaIp6Address) OSCL_DEFAULT_MALLOC(sizeof(S_MaIp6Address));
            Decode_MaIp6Address(x->maIp6Address, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            GetLengthDet(stream);
            x->nsap = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(0, 1, 20, x->nsap, stream);
            ReadRemainingBits(stream);
            break;
        case 3:
            GetLengthDet(stream);
            x->nonStandardAddress = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandardAddress, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_MulticastAddress: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for MaIp6Address (SEQUENCE)  */
/* <=======================================> */
void Decode_MaIp6Address(PS_MaIp6Address x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 16, 16, &x->network, stream);
    x->tsapIdentifier = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaIp6Address: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for MaIpAddress (SEQUENCE)  */
/* <======================================> */
void Decode_MaIpAddress(PS_MaIpAddress x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 4, 4, &x->network, stream);
    x->tsapIdentifier = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaIpAddress: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for EncryptionSync (SEQUENCE)  */
/* <=========================================> */
void Decode_EncryptionSync(PS_EncryptionSync x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    x->option_of_escrowentry = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        Decode_NonStandardParameter(&x->nonStandard, stream);
    }
    x->synchFlag = (uint8) GetInteger(0, 255, stream);
    GetOctetString(0, 1, 65535, &x->h235Key, stream);
    if (x->option_of_escrowentry)
    {
        x->size_of_escrowentry = (uint16) GetInteger(1, 256, stream);
        x->escrowentry = (PS_EscrowData)
                         OSCL_DEFAULT_MALLOC(x->size_of_escrowentry * sizeof(S_EscrowData));
        for (i = 0;i < x->size_of_escrowentry;++i)
        {
            Decode_EscrowData(x->escrowentry + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EncryptionSync: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for EscrowData (SEQUENCE)  */
/* <=====================================> */
void Decode_EscrowData(PS_EscrowData x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetObjectID(&x->escrowID, stream);
    GetBitString(0, 1, 65535, &x->escrowValue, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EscrowData: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for OpenLogicalChannelAck (SEQUENCE)  */
/* <================================================> */
void Decode_OpenLogicalChannelAck(PS_OpenLogicalChannelAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_ackReverseLogicalChannelParameters = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    if (x->option_of_ackReverseLogicalChannelParameters)
    {
        Decode_AckReverseLogicalChannelParameters(&x->ackReverseLogicalChannelParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_separateStack = OFF;
    x->option_of_forwardMultiplexAckParameters = OFF;
    x->option_of_encryptionSync = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_separateStack = SigMapValue(0, map);
        if (x->option_of_separateStack)
        {
            ExtensionPrep(map, stream);
            Decode_NetworkAccessParameters(&x->separateStack, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_forwardMultiplexAckParameters = SigMapValue(1, map);
        if (x->option_of_forwardMultiplexAckParameters)
        {
            ExtensionPrep(map, stream);
            Decode_ForwardMultiplexAckParameters(&x->forwardMultiplexAckParameters, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_encryptionSync = SigMapValue(2, map);
        if (x->option_of_encryptionSync)
        {
            ExtensionPrep(map, stream);
            Decode_EncryptionSync(&x->encryptionSync, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_OpenLogicalChannelAck: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for ForwardMultiplexAckParameters (CHOICE)  */
/* <======================================================> */
void Decode_ForwardMultiplexAckParameters(PS_ForwardMultiplexAckParameters x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            x->h2250LogicalChannelAckParameters = (PS_H2250LogicalChannelAckParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H2250LogicalChannelAckParameters));
            Decode_H2250LogicalChannelAckParameters(x->h2250LogicalChannelAckParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ForwardMultiplexAckParameters: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================================> */
/*  PER-Decoder for AckReverseLogicalChannelParameters (SEQUENCE)  */
/* <=============================================================> */
void Decode_AckReverseLogicalChannelParameters(PS_AckReverseLogicalChannelParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_portNumber = GetBoolean(stream);
    x->option_of_ackMultiplexParameters = GetBoolean(stream);
    x->reverseLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    if (x->option_of_portNumber)
    {
        x->portNumber = (uint16) GetInteger(0, 65535, stream);
    }
    if (x->option_of_ackMultiplexParameters)
    {
        Decode_AckMultiplexParameters(&x->ackMultiplexParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_replacementFor = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_replacementFor = SigMapValue(0, map);
        if (x->option_of_replacementFor)
        {
            ExtensionPrep(map, stream);
            x->replacementFor = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_AckReverseLogicalChannelParameters: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for AckMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Decode_AckMultiplexParameters(PS_AckMultiplexParameters x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            x->h222LogicalChannelParameters = (PS_H222LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H222LogicalChannelParameters));
            Decode_H222LogicalChannelParameters(x->h222LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            GetLengthDet(stream);
            x->h2250LogicalChannelParameters = (PS_H2250LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H2250LogicalChannelParameters));
            Decode_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_AckMultiplexParameters: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================================> */
/*  PER-Decoder for OpenLogicalChannelReject (SEQUENCE)  */
/* <===================================================> */
void Decode_OpenLogicalChannelReject(PS_OpenLogicalChannelReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_OlcRejectCause(&x->olcRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_OpenLogicalChannelReject: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for OlcRejectCause (CHOICE)  */
/* <=======================================> */
void Decode_OlcRejectCause(PS_OlcRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(6, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
        case 1:
            /* (unsuitableReverseParameters is NULL) */
            break;
        case 2:
            /* (dataTypeNotSupported is NULL) */
            break;
        case 3:
            /* (dataTypeNotAvailable is NULL) */
            break;
        case 4:
            /* (unknownDataType is NULL) */
            break;
        case 5:
            /* (dataTypeALCombinationNotSupported is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            GetLengthDet(stream);
            /* (multicastChannelNotAllowed is NULL) */
            SkipOneOctet(stream);
            break;
        case 7:
            GetLengthDet(stream);
            /* (insufficientBandwidth is NULL) */
            SkipOneOctet(stream);
            break;
        case 8:
            GetLengthDet(stream);
            /* (separateStackEstablishmentFailed is NULL) */
            SkipOneOctet(stream);
            break;
        case 9:
            GetLengthDet(stream);
            /* (invalidSessionID is NULL) */
            SkipOneOctet(stream);
            break;
        case 10:
            GetLengthDet(stream);
            /* (masterSlaveConflict is NULL) */
            SkipOneOctet(stream);
            break;
        case 11:
            GetLengthDet(stream);
            /* (waitForCommunicationMode is NULL) */
            SkipOneOctet(stream);
            break;
        case 12:
            GetLengthDet(stream);
            /* (invalidDependentChannel is NULL) */
            SkipOneOctet(stream);
            break;
        case 13:
            GetLengthDet(stream);
            /* (replacementForRejected is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_OlcRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================================> */
/*  PER-Decoder for OpenLogicalChannelConfirm (SEQUENCE)  */
/* <====================================================> */
void Decode_OpenLogicalChannelConfirm(PS_OpenLogicalChannelConfirm x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_OpenLogicalChannelConfirm: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================================> */
/*  PER-Decoder for H2250LogicalChannelAckParameters (SEQUENCE)  */
/* <===========================================================> */
void Decode_H2250LogicalChannelAckParameters(PS_H2250LogicalChannelAckParameters x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    x->option_of_sessionID = GetBoolean(stream);
    x->option_of_mediaChannel = GetBoolean(stream);
    x->option_of_mediaControlChannel = GetBoolean(stream);
    x->option_of_dynamicRTPPayloadType = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        x->size_of_nonStandard = (uint16) GetLengthDet(stream);
        x->nonStandard = (PS_NonStandardParameter)
                         OSCL_DEFAULT_MALLOC(x->size_of_nonStandard * sizeof(S_NonStandardParameter));
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Decode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    if (x->option_of_sessionID)
    {
        x->sessionID = (uint8) GetInteger(1, 255, stream);
    }
    if (x->option_of_mediaChannel)
    {
        Decode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Decode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        x->dynamicRTPPayloadType = (uint8) GetInteger(96, 127, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_flowControlToZero = OFF;
    x->option_of_portNumber = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_flowControlToZero = SigMapValue(0, map);
        if (x->option_of_flowControlToZero)
        {
            ExtensionPrep(map, stream);
            x->flowControlToZero = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_portNumber = SigMapValue(1, map);
        if (x->option_of_portNumber)
        {
            ExtensionPrep(map, stream);
            x->portNumber = (uint16) GetInteger(0, 65535, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H2250LogicalChannelAckParameters: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for CloseLogicalChannel (SEQUENCE)  */
/* <==============================================> */
void Decode_CloseLogicalChannel(PS_CloseLogicalChannel x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_Source(&x->source, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_reason = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_reason = SigMapValue(0, map);
        if (x->option_of_reason)
        {
            ExtensionPrep(map, stream);
            Decode_Reason(&x->reason, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CloseLogicalChannel: Unknown extensions (skipped)");
        }
    }
}

/* <===============================> */
/*  PER-Decoder for Reason (CHOICE)  */
/* <===============================> */
void Decode_Reason(PS_Reason x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unknown is NULL) */
            break;
        case 1:
            /* (reopen is NULL) */
            break;
        case 2:
            /* (reservationFailure is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Reason: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================> */
/*  PER-Decoder for Source (CHOICE)  */
/* <===============================> */
void Decode_Source(PS_Source x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (user is NULL) */
            break;
        case 1:
            /* (lcse is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Source: Illegal CHOICE index");
    }
}

/* <=================================================> */
/*  PER-Decoder for CloseLogicalChannelAck (SEQUENCE)  */
/* <=================================================> */
void Decode_CloseLogicalChannelAck(PS_CloseLogicalChannelAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CloseLogicalChannelAck: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for RequestChannelClose (SEQUENCE)  */
/* <==============================================> */
void Decode_RequestChannelClose(PS_RequestChannelClose x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_qosCapability = OFF;
    x->option_of_rccReason = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_qosCapability = SigMapValue(0, map);
        if (x->option_of_qosCapability)
        {
            ExtensionPrep(map, stream);
            Decode_QOSCapability(&x->qosCapability, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_rccReason = SigMapValue(1, map);
        if (x->option_of_rccReason)
        {
            ExtensionPrep(map, stream);
            Decode_RccReason(&x->rccReason, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestChannelClose: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for RccReason (CHOICE)  */
/* <==================================> */
void Decode_RccReason(PS_RccReason x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unknown is NULL) */
            break;
        case 1:
            /* (normal is NULL) */
            break;
        case 2:
            /* (reopen is NULL) */
            break;
        case 3:
            /* (reservationFailure is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RccReason: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=================================================> */
/*  PER-Decoder for RequestChannelCloseAck (SEQUENCE)  */
/* <=================================================> */
void Decode_RequestChannelCloseAck(PS_RequestChannelCloseAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestChannelCloseAck: Unknown extensions (skipped)");
        }
    }
}

/* <====================================================> */
/*  PER-Decoder for RequestChannelCloseReject (SEQUENCE)  */
/* <====================================================> */
void Decode_RequestChannelCloseReject(PS_RequestChannelCloseReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_RccRejectCause(&x->rccRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestChannelCloseReject: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for RccRejectCause (CHOICE)  */
/* <=======================================> */
void Decode_RccRejectCause(PS_RccRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RccRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================================> */
/*  PER-Decoder for RequestChannelCloseRelease (SEQUENCE)  */
/* <=====================================================> */
void Decode_RequestChannelCloseRelease(PS_RequestChannelCloseRelease x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardLogicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestChannelCloseRelease: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for MultiplexEntrySend (SEQUENCE)  */
/* <=============================================> */
void Decode_MultiplexEntrySend(PS_MultiplexEntrySend x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->size_of_multiplexEntryDescriptors = (uint8) GetInteger(1, 15, stream);
    x->multiplexEntryDescriptors = (PS_MultiplexEntryDescriptor)
                                   OSCL_DEFAULT_MALLOC(x->size_of_multiplexEntryDescriptors * sizeof(S_MultiplexEntryDescriptor));
    for (i = 0;i < x->size_of_multiplexEntryDescriptors;++i)
    {
        Decode_MultiplexEntryDescriptor(x->multiplexEntryDescriptors + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultiplexEntrySend: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for MultiplexEntryDescriptor (SEQUENCE)  */
/* <===================================================> */
void Decode_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x, PS_InStream stream)
{
    uint16 i;
    x->option_of_elementList = GetBoolean(stream);
    x->multiplexTableEntryNumber = (uint8) GetInteger(1, 15, stream);
    if (x->option_of_elementList)
    {
        x->size_of_elementList = (uint16) GetInteger(1, 256, stream);
        x->elementList = (PS_MultiplexElement)
                         OSCL_DEFAULT_MALLOC(x->size_of_elementList * sizeof(S_MultiplexElement));
        for (i = 0;i < x->size_of_elementList;++i)
        {
            Decode_MultiplexElement(x->elementList + i, stream);
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for MultiplexElement (SEQUENCE)  */
/* <===========================================> */
void Decode_MultiplexElement(PS_MultiplexElement x, PS_InStream stream)
{
    Decode_MuxType(&x->muxType, stream);
    Decode_RepeatCount(&x->repeatCount, stream);
}

/* <====================================> */
/*  PER-Decoder for RepeatCount (CHOICE)  */
/* <====================================> */
void Decode_RepeatCount(PS_RepeatCount x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->finite = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            /* (untilClosingFlag is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_RepeatCount: Illegal CHOICE index");
    }
}

/* <================================> */
/*  PER-Decoder for MuxType (CHOICE)  */
/* <================================> */
void Decode_MuxType(PS_MuxType x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->logicalChannelNumber = (uint16) GetInteger(0, 65535, stream);
            break;
        case 1:
            x->size = (uint8) GetInteger(2, 255, stream);
            x->subElementList = (PS_MultiplexElement)
                                OSCL_DEFAULT_MALLOC(x->size * sizeof(S_MultiplexElement));
            for (i = 0;i < x->size;++i)
            {
                Decode_MultiplexElement(x->subElementList + i, stream);
            }
            break;
        default:
            ErrorMessageAndLeave("Decode_MuxType: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Decoder for MultiplexEntrySendAck (SEQUENCE)  */
/* <================================================> */
void Decode_MultiplexEntrySendAck(PS_MultiplexEntrySendAck x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->size_of_multiplexTableEntryNumber = (uint8) GetInteger(1, 15, stream);
    x->multiplexTableEntryNumber = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_multiplexTableEntryNumber * sizeof(uint32));
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        x->multiplexTableEntryNumber[i] = GetInteger(1, 15, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultiplexEntrySendAck: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for MultiplexEntrySendReject (SEQUENCE)  */
/* <===================================================> */
void Decode_MultiplexEntrySendReject(PS_MultiplexEntrySendReject x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->size_of_rejectionDescriptions = (uint8) GetInteger(1, 15, stream);
    x->rejectionDescriptions = (PS_MultiplexEntryRejectionDescriptions)
                               OSCL_DEFAULT_MALLOC(x->size_of_rejectionDescriptions * sizeof(S_MultiplexEntryRejectionDescriptions));
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Decode_MultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultiplexEntrySendReject: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================================> */
/*  PER-Decoder for MultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <==============================================================> */
void Decode_MultiplexEntryRejectionDescriptions(PS_MultiplexEntryRejectionDescriptions x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->multiplexTableEntryNumber = (uint8) GetInteger(1, 15, stream);
    Decode_MeRejectCause(&x->meRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultiplexEntryRejectionDescriptions: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for MeRejectCause (CHOICE)  */
/* <======================================> */
void Decode_MeRejectCause(PS_MeRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecifiedCause is NULL) */
            break;
        case 1:
            /* (descriptorTooComplex is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MeRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================================> */
/*  PER-Decoder for MultiplexEntrySendRelease (SEQUENCE)  */
/* <====================================================> */
void Decode_MultiplexEntrySendRelease(PS_MultiplexEntrySendRelease x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_multiplexTableEntryNumber = (uint8) GetInteger(1, 15, stream);
    x->multiplexTableEntryNumber = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_multiplexTableEntryNumber * sizeof(uint32));
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        x->multiplexTableEntryNumber[i] = GetInteger(1, 15, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MultiplexEntrySendRelease: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for RequestMultiplexEntry (SEQUENCE)  */
/* <================================================> */
void Decode_RequestMultiplexEntry(PS_RequestMultiplexEntry x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_entryNumbers = (uint8) GetInteger(1, 15, stream);
    x->entryNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_entryNumbers * sizeof(uint32));
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        x->entryNumbers[i] = GetInteger(1, 15, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMultiplexEntry: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for RequestMultiplexEntryAck (SEQUENCE)  */
/* <===================================================> */
void Decode_RequestMultiplexEntryAck(PS_RequestMultiplexEntryAck x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_entryNumbers = (uint8) GetInteger(1, 15, stream);
    x->entryNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_entryNumbers * sizeof(uint32));
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        x->entryNumbers[i] = GetInteger(1, 15, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMultiplexEntryAck: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for RequestMultiplexEntryReject (SEQUENCE)  */
/* <======================================================> */
void Decode_RequestMultiplexEntryReject(PS_RequestMultiplexEntryReject x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_entryNumbers = (uint8) GetInteger(1, 15, stream);
    x->entryNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_entryNumbers * sizeof(uint32));
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        x->entryNumbers[i] = GetInteger(1, 15, stream);
    }
    x->size_of_rejectionDescriptions = (uint8) GetInteger(1, 15, stream);
    x->rejectionDescriptions = (PS_RequestMultiplexEntryRejectionDescriptions)
                               OSCL_DEFAULT_MALLOC(x->size_of_rejectionDescriptions * sizeof(S_RequestMultiplexEntryRejectionDescriptions));
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Decode_RequestMultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMultiplexEntryReject: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================================================> */
/*  PER-Decoder for RequestMultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <=====================================================================> */
void Decode_RequestMultiplexEntryRejectionDescriptions(PS_RequestMultiplexEntryRejectionDescriptions x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->multiplexTableEntryNumber = (uint8) GetInteger(1, 15, stream);
    Decode_RmeRejectCause(&x->rmeRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMultiplexEntryRejectionDescriptions: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for RmeRejectCause (CHOICE)  */
/* <=======================================> */
void Decode_RmeRejectCause(PS_RmeRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecifiedCause is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RmeRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================================> */
/*  PER-Decoder for RequestMultiplexEntryRelease (SEQUENCE)  */
/* <=======================================================> */
void Decode_RequestMultiplexEntryRelease(PS_RequestMultiplexEntryRelease x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_entryNumbers = (uint8) GetInteger(1, 15, stream);
    x->entryNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_entryNumbers * sizeof(uint32));
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        x->entryNumbers[i] = GetInteger(1, 15, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMultiplexEntryRelease: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for RequestMode (SEQUENCE)  */
/* <======================================> */
void Decode_RequestMode(PS_RequestMode x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->size_of_requestedModes = (uint16) GetInteger(1, 256, stream);
    x->requestedModes = (PS_ModeDescription)
                        OSCL_DEFAULT_MALLOC(x->size_of_requestedModes * sizeof(S_ModeDescription));
    for (i = 0;i < x->size_of_requestedModes;++i)
    {
        Decode_ModeDescription(x->requestedModes + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestMode: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for RequestModeAck (SEQUENCE)  */
/* <=========================================> */
void Decode_RequestModeAck(PS_RequestModeAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    Decode_Response(&x->response, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestModeAck: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for Response (CHOICE)  */
/* <=================================> */
void Decode_Response(PS_Response x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (willTransmitMostPreferredMode is NULL) */
            break;
        case 1:
            /* (willTransmitLessPreferredMode is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Response: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <============================================> */
/*  PER-Decoder for RequestModeReject (SEQUENCE)  */
/* <============================================> */
void Decode_RequestModeReject(PS_RequestModeReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    Decode_RmRejectCause(&x->rmRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestModeReject: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for RmRejectCause (CHOICE)  */
/* <======================================> */
void Decode_RmRejectCause(PS_RmRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (modeUnavailable is NULL) */
            break;
        case 1:
            /* (multipointConstraint is NULL) */
            break;
        case 2:
            /* (requestDenied is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RmRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for RequestModeRelease (SEQUENCE)  */
/* <=============================================> */
void Decode_RequestModeRelease(PS_RequestModeRelease x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestModeRelease: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for ModeDescription (SET-OF)  */
/* <========================================> */
void Decode_ModeDescription(PS_ModeDescription x, PS_InStream stream)
{
    uint16 i;
    x->size = (uint16) GetInteger(1, 256, stream);
    x->item = (PS_ModeElement)
              OSCL_DEFAULT_MALLOC(x->size * sizeof(S_ModeElement));
    for (i = 0;i < x->size;++i)
    {
        Decode_ModeElement(x->item + i, stream);
    }
}

/* <======================================> */
/*  PER-Decoder for ModeElement (SEQUENCE)  */
/* <======================================> */
void Decode_ModeElement(PS_ModeElement x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_h223ModeParameters = GetBoolean(stream);
    Decode_ModeType(&x->modeType, stream);
    if (x->option_of_h223ModeParameters)
    {
        Decode_H223ModeParameters(&x->h223ModeParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_v76ModeParameters = OFF;
    x->option_of_h2250ModeParameters = OFF;
    x->option_of_genericModeParameters = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_v76ModeParameters = SigMapValue(0, map);
        if (x->option_of_v76ModeParameters)
        {
            ExtensionPrep(map, stream);
            Decode_V76ModeParameters(&x->v76ModeParameters, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_h2250ModeParameters = SigMapValue(1, map);
        if (x->option_of_h2250ModeParameters)
        {
            ExtensionPrep(map, stream);
            Decode_H2250ModeParameters(&x->h2250ModeParameters, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_genericModeParameters = SigMapValue(2, map);
        if (x->option_of_genericModeParameters)
        {
            ExtensionPrep(map, stream);
            Decode_GenericCapability(&x->genericModeParameters, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ModeElement: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for ModeType (CHOICE)  */
/* <=================================> */
void Decode_ModeType(PS_ModeType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->videoMode = (PS_VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_VideoMode));
            Decode_VideoMode(x->videoMode, stream);
            break;
        case 2:
            x->audioMode = (PS_AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_AudioMode));
            Decode_AudioMode(x->audioMode, stream);
            break;
        case 3:
            x->dataMode = (PS_DataMode) OSCL_DEFAULT_MALLOC(sizeof(S_DataMode));
            Decode_DataMode(x->dataMode, stream);
            break;
        case 4:
            x->encryptionMode = (PS_EncryptionMode) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionMode));
            Decode_EncryptionMode(x->encryptionMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            GetLengthDet(stream);
            x->h235Mode = (PS_H235Mode) OSCL_DEFAULT_MALLOC(sizeof(S_H235Mode));
            Decode_H235Mode(x->h235Mode, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ModeType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================> */
/*  PER-Decoder for H235Mode (SEQUENCE)  */
/* <===================================> */
void Decode_H235Mode(PS_H235Mode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    Decode_MediaMode(&x->mediaMode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H235Mode: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for MediaMode (CHOICE)  */
/* <==================================> */
void Decode_MediaMode(PS_MediaMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->videoMode = (PS_VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_VideoMode));
            Decode_VideoMode(x->videoMode, stream);
            break;
        case 2:
            x->audioMode = (PS_AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_AudioMode));
            Decode_AudioMode(x->audioMode, stream);
            break;
        case 3:
            x->dataMode = (PS_DataMode) OSCL_DEFAULT_MALLOC(sizeof(S_DataMode));
            Decode_DataMode(x->dataMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MediaMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for H223ModeParameters (SEQUENCE)  */
/* <=============================================> */
void Decode_H223ModeParameters(PS_H223ModeParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_ModeAdaptationLayerType(&x->modeAdaptationLayerType, stream);
    x->segmentableFlag = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223ModeParameters: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for ModeAdaptationLayerType (CHOICE)  */
/* <================================================> */
void Decode_ModeAdaptationLayerType(PS_ModeAdaptationLayerType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(6, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (al1Framed is NULL) */
            break;
        case 2:
            /* (al1NotFramed is NULL) */
            break;
        case 3:
            /* (al2WithoutSequenceNumbers is NULL) */
            break;
        case 4:
            /* (al2WithSequenceNumbers is NULL) */
            break;
        case 5:
            x->modeAl3 = (PS_ModeAl3) OSCL_DEFAULT_MALLOC(sizeof(S_ModeAl3));
            Decode_ModeAl3(x->modeAl3, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            GetLengthDet(stream);
            x->al1M = (PS_H223AL1MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL1MParameters));
            Decode_H223AL1MParameters(x->al1M, stream);
            ReadRemainingBits(stream);
            break;
        case 7:
            GetLengthDet(stream);
            x->al2M = (PS_H223AL2MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL2MParameters));
            Decode_H223AL2MParameters(x->al2M, stream);
            ReadRemainingBits(stream);
            break;
        case 8:
            GetLengthDet(stream);
            x->al3M = (PS_H223AL3MParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223AL3MParameters));
            Decode_H223AL3MParameters(x->al3M, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ModeAdaptationLayerType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for ModeAl3 (SEQUENCE)  */
/* <==================================> */
void Decode_ModeAl3(PS_ModeAl3 x, PS_InStream stream)
{
    x->controlFieldOctets = (uint8) GetInteger(0, 2, stream);
    x->sendBufferSize = GetInteger(0, 16777215, stream);
}

/* <==========================================> */
/*  PER-Decoder for V76ModeParameters (CHOICE)  */
/* <==========================================> */
void Decode_V76ModeParameters(PS_V76ModeParameters x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (suspendResumewAddress is NULL) */
            break;
        case 1:
            /* (suspendResumewoAddress is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_V76ModeParameters: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================================> */
/*  PER-Decoder for H2250ModeParameters (SEQUENCE)  */
/* <==============================================> */
void Decode_H2250ModeParameters(PS_H2250ModeParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_redundancyEncodingMode = GetBoolean(stream);
    if (x->option_of_redundancyEncodingMode)
    {
        Decode_RedundancyEncodingMode(&x->redundancyEncodingMode, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H2250ModeParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for RedundancyEncodingMode (SEQUENCE)  */
/* <=================================================> */
void Decode_RedundancyEncodingMode(PS_RedundancyEncodingMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_secondaryEncoding = GetBoolean(stream);
    Decode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    if (x->option_of_secondaryEncoding)
    {
        Decode_SecondaryEncoding(&x->secondaryEncoding, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RedundancyEncodingMode: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for SecondaryEncoding (CHOICE)  */
/* <==========================================> */
void Decode_SecondaryEncoding(PS_SecondaryEncoding x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->audioData = (PS_AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_AudioMode));
            Decode_AudioMode(x->audioData, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_SecondaryEncoding: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for VideoMode (CHOICE)  */
/* <==================================> */
void Decode_VideoMode(PS_VideoMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->h261VideoMode = (PS_H261VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_H261VideoMode));
            Decode_H261VideoMode(x->h261VideoMode, stream);
            break;
        case 2:
            x->h262VideoMode = (PS_H262VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_H262VideoMode));
            Decode_H262VideoMode(x->h262VideoMode, stream);
            break;
        case 3:
            x->h263VideoMode = (PS_H263VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_H263VideoMode));
            Decode_H263VideoMode(x->h263VideoMode, stream);
            break;
        case 4:
            x->is11172VideoMode = (PS_IS11172VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_IS11172VideoMode));
            Decode_IS11172VideoMode(x->is11172VideoMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            GetLengthDet(stream);
            x->genericVideoMode = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericVideoMode, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_VideoMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for H261VideoMode (SEQUENCE)  */
/* <========================================> */
void Decode_H261VideoMode(PS_H261VideoMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Resolution(&x->resolution, stream);
    x->bitRate = (uint16) GetInteger(1, 19200, stream);
    x->stillImageTransmission = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H261VideoMode: Unknown extensions (skipped)");
        }
    }
}

/* <===================================> */
/*  PER-Decoder for Resolution (CHOICE)  */
/* <===================================> */
void Decode_Resolution(PS_Resolution x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (qcif is NULL) */
            break;
        case 1:
            /* (cif is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Resolution: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Decoder for H262VideoMode (SEQUENCE)  */
/* <========================================> */
void Decode_H262VideoMode(PS_H262VideoMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_videoBitRate = GetBoolean(stream);
    x->option_of_vbvBufferSize = GetBoolean(stream);
    x->option_of_samplesPerLine = GetBoolean(stream);
    x->option_of_linesPerFrame = GetBoolean(stream);
    x->option_of_framesPerSecond = GetBoolean(stream);
    x->option_of_luminanceSampleRate = GetBoolean(stream);
    Decode_ProfileAndLevel(&x->profileAndLevel, stream);
    if (x->option_of_videoBitRate)
    {
        x->videoBitRate = GetInteger(0, 1073741823, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        x->vbvBufferSize = GetInteger(0, 262143, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        x->samplesPerLine = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        x->linesPerFrame = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_framesPerSecond)
    {
        x->framesPerSecond = (uint8) GetInteger(0, 15, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        x->luminanceSampleRate = GetInteger(0, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H262VideoMode: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for ProfileAndLevel (CHOICE)  */
/* <========================================> */
void Decode_ProfileAndLevel(PS_ProfileAndLevel x, PS_InStream stream)
{
    x->index = GetChoiceIndex(11, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (profileAndLevel_SPatML is NULL) */
            break;
        case 1:
            /* (profileAndLevel_MPatLL is NULL) */
            break;
        case 2:
            /* (profileAndLevel_MPatML is NULL) */
            break;
        case 3:
            /* (profileAndLevel_MPatH_14 is NULL) */
            break;
        case 4:
            /* (profileAndLevel_MPatHL is NULL) */
            break;
        case 5:
            /* (profileAndLevel_SNRatLL is NULL) */
            break;
        case 6:
            /* (profileAndLevel_SNRatML is NULL) */
            break;
        case 7:
            /* (profileAndLevel_SpatialatH_14 is NULL) */
            break;
        case 8:
            /* (profileAndLevel_HPatML is NULL) */
            break;
        case 9:
            /* (profileAndLevel_HPatH_14 is NULL) */
            break;
        case 10:
            /* (profileAndLevel_HPatHL is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ProfileAndLevel: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================> */
/*  PER-Decoder for H263VideoMode (SEQUENCE)  */
/* <========================================> */
void Decode_H263VideoMode(PS_H263VideoMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_H263Resolution(&x->h263Resolution, stream);
    x->bitRate = (uint16) GetInteger(1, 19200, stream);
    x->unrestrictedVector = GetBoolean(stream);
    x->arithmeticCoding = GetBoolean(stream);
    x->advancedPrediction = GetBoolean(stream);
    x->pbFrames = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_errorCompensation = OFF;
    x->option_of_enhancementLayerInfo = OFF;
    x->option_of_h263Options = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_errorCompensation = SigMapValue(0, map);
        if (x->option_of_errorCompensation)
        {
            ExtensionPrep(map, stream);
            x->errorCompensation = GetBoolean(stream);
            ReadRemainingBits(stream);
        }
        x->option_of_enhancementLayerInfo = SigMapValue(1, map);
        if (x->option_of_enhancementLayerInfo)
        {
            ExtensionPrep(map, stream);
            Decode_EnhancementLayerInfo(&x->enhancementLayerInfo, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_h263Options = SigMapValue(2, map);
        if (x->option_of_h263Options)
        {
            ExtensionPrep(map, stream);
            Decode_H263Options(&x->h263Options, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H263VideoMode: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for H263Resolution (CHOICE)  */
/* <=======================================> */
void Decode_H263Resolution(PS_H263Resolution x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (sqcif is NULL) */
            break;
        case 1:
            /* (qcif is NULL) */
            break;
        case 2:
            /* (cif is NULL) */
            break;
        case 3:
            /* (cif4 is NULL) */
            break;
        case 4:
            /* (cif16 is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_H263Resolution: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===========================================> */
/*  PER-Decoder for IS11172VideoMode (SEQUENCE)  */
/* <===========================================> */
void Decode_IS11172VideoMode(PS_IS11172VideoMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_videoBitRate = GetBoolean(stream);
    x->option_of_vbvBufferSize = GetBoolean(stream);
    x->option_of_samplesPerLine = GetBoolean(stream);
    x->option_of_linesPerFrame = GetBoolean(stream);
    x->option_of_pictureRate = GetBoolean(stream);
    x->option_of_luminanceSampleRate = GetBoolean(stream);
    x->constrainedBitstream = GetBoolean(stream);
    if (x->option_of_videoBitRate)
    {
        x->videoBitRate = GetInteger(0, 1073741823, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        x->vbvBufferSize = GetInteger(0, 262143, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        x->samplesPerLine = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        x->linesPerFrame = (uint16) GetInteger(0, 16383, stream);
    }
    if (x->option_of_pictureRate)
    {
        x->pictureRate = (uint8) GetInteger(0, 15, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        x->luminanceSampleRate = GetInteger(0, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS11172VideoMode: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for AudioMode (CHOICE)  */
/* <==================================> */
void Decode_AudioMode(PS_AudioMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(14, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (g711Alaw64k is NULL) */
            break;
        case 2:
            /* (g711Alaw56k is NULL) */
            break;
        case 3:
            /* (g711Ulaw64k is NULL) */
            break;
        case 4:
            /* (g711Ulaw56k is NULL) */
            break;
        case 5:
            /* (g722_64k is NULL) */
            break;
        case 6:
            /* (g722_56k is NULL) */
            break;
        case 7:
            /* (g722_48k is NULL) */
            break;
        case 8:
            /* (g728 is NULL) */
            break;
        case 9:
            /* (g729 is NULL) */
            break;
        case 10:
            /* (g729AnnexA is NULL) */
            break;
        case 11:
            x->modeG7231 = (PS_ModeG7231) OSCL_DEFAULT_MALLOC(sizeof(S_ModeG7231));
            Decode_ModeG7231(x->modeG7231, stream);
            break;
        case 12:
            x->is11172AudioMode = (PS_IS11172AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_IS11172AudioMode));
            Decode_IS11172AudioMode(x->is11172AudioMode, stream);
            break;
        case 13:
            x->is13818AudioMode = (PS_IS13818AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_IS13818AudioMode));
            Decode_IS13818AudioMode(x->is13818AudioMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            GetLengthDet(stream);
            x->g729wAnnexB = (uint16) GetInteger(1, 256, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->g729AnnexAwAnnexB = (uint16) GetInteger(1, 256, stream);
            ReadRemainingBits(stream);
            break;
        case 16:
            GetLengthDet(stream);
            x->g7231AnnexCMode = (PS_G7231AnnexCMode) OSCL_DEFAULT_MALLOC(sizeof(S_G7231AnnexCMode));
            Decode_G7231AnnexCMode(x->g7231AnnexCMode, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            x->gsmFullRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmFullRate, stream);
            ReadRemainingBits(stream);
            break;
        case 18:
            GetLengthDet(stream);
            x->gsmHalfRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmHalfRate, stream);
            ReadRemainingBits(stream);
            break;
        case 19:
            GetLengthDet(stream);
            x->gsmEnhancedFullRate = (PS_GSMAudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GSMAudioCapability));
            Decode_GSMAudioCapability(x->gsmEnhancedFullRate, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            x->genericAudioMode = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericAudioMode, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            x->g729Extensions = (PS_G729Extensions) OSCL_DEFAULT_MALLOC(sizeof(S_G729Extensions));
            Decode_G729Extensions(x->g729Extensions, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_AudioMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for ModeG7231 (CHOICE)  */
/* <==================================> */
void Decode_ModeG7231(PS_ModeG7231 x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (noSilenceSuppressionLowRate is NULL) */
            break;
        case 1:
            /* (noSilenceSuppressionHighRate is NULL) */
            break;
        case 2:
            /* (silenceSuppressionLowRate is NULL) */
            break;
        case 3:
            /* (silenceSuppressionHighRate is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_ModeG7231: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Decoder for IS11172AudioMode (SEQUENCE)  */
/* <===========================================> */
void Decode_IS11172AudioMode(PS_IS11172AudioMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_AudioLayer(&x->audioLayer, stream);
    Decode_AudioSampling(&x->audioSampling, stream);
    Decode_MultichannelType(&x->multichannelType, stream);
    x->bitRate = (uint16) GetInteger(1, 448, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS11172AudioMode: Unknown extensions (skipped)");
        }
    }
}

/* <=========================================> */
/*  PER-Decoder for MultichannelType (CHOICE)  */
/* <=========================================> */
void Decode_MultichannelType(PS_MultichannelType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (singleChannel is NULL) */
            break;
        case 1:
            /* (twoChannelStereo is NULL) */
            break;
        case 2:
            /* (twoChannelDual is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_MultichannelType: Illegal CHOICE index");
    }
}

/* <======================================> */
/*  PER-Decoder for AudioSampling (CHOICE)  */
/* <======================================> */
void Decode_AudioSampling(PS_AudioSampling x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (audioSampling32k is NULL) */
            break;
        case 1:
            /* (audioSampling44k1 is NULL) */
            break;
        case 2:
            /* (audioSampling48k is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_AudioSampling: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Decoder for AudioLayer (CHOICE)  */
/* <===================================> */
void Decode_AudioLayer(PS_AudioLayer x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (audioLayer1 is NULL) */
            break;
        case 1:
            /* (audioLayer2 is NULL) */
            break;
        case 2:
            /* (audioLayer3 is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_AudioLayer: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Decoder for IS13818AudioMode (SEQUENCE)  */
/* <===========================================> */
void Decode_IS13818AudioMode(PS_IS13818AudioMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_Is13818AudioLayer(&x->is13818AudioLayer, stream);
    Decode_Is13818AudioSampling(&x->is13818AudioSampling, stream);
    Decode_Is13818MultichannelType(&x->is13818MultichannelType, stream);
    x->lowFrequencyEnhancement = GetBoolean(stream);
    x->multilingual = GetBoolean(stream);
    x->bitRate = (uint16) GetInteger(1, 1130, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IS13818AudioMode: Unknown extensions (skipped)");
        }
    }
}

/* <================================================> */
/*  PER-Decoder for Is13818MultichannelType (CHOICE)  */
/* <================================================> */
void Decode_Is13818MultichannelType(PS_Is13818MultichannelType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (singleChannel is NULL) */
            break;
        case 1:
            /* (twoChannelStereo is NULL) */
            break;
        case 2:
            /* (twoChannelDual is NULL) */
            break;
        case 3:
            /* (threeChannels2_1 is NULL) */
            break;
        case 4:
            /* (threeChannels3_0 is NULL) */
            break;
        case 5:
            /* (fourChannels2_0_2_0 is NULL) */
            break;
        case 6:
            /* (fourChannels2_2 is NULL) */
            break;
        case 7:
            /* (fourChannels3_1 is NULL) */
            break;
        case 8:
            /* (fiveChannels3_0_2_0 is NULL) */
            break;
        case 9:
            /* (fiveChannels3_2 is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Is13818MultichannelType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Decoder for Is13818AudioSampling (CHOICE)  */
/* <=============================================> */
void Decode_Is13818AudioSampling(PS_Is13818AudioSampling x, PS_InStream stream)
{
    x->index = GetChoiceIndex(6, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (audioSampling16k is NULL) */
            break;
        case 1:
            /* (audioSampling22k05 is NULL) */
            break;
        case 2:
            /* (audioSampling24k is NULL) */
            break;
        case 3:
            /* (audioSampling32k is NULL) */
            break;
        case 4:
            /* (audioSampling44k1 is NULL) */
            break;
        case 5:
            /* (audioSampling48k is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Is13818AudioSampling: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Decoder for Is13818AudioLayer (CHOICE)  */
/* <==========================================> */
void Decode_Is13818AudioLayer(PS_Is13818AudioLayer x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            /* (audioLayer1 is NULL) */
            break;
        case 1:
            /* (audioLayer2 is NULL) */
            break;
        case 2:
            /* (audioLayer3 is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_Is13818AudioLayer: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Decoder for G7231AnnexCMode (SEQUENCE)  */
/* <==========================================> */
void Decode_G7231AnnexCMode(PS_G7231AnnexCMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maxAl_sduAudioFrames = (uint16) GetInteger(1, 256, stream);
    x->silenceSuppression = GetBoolean(stream);
    Decode_ModeG723AnnexCAudioMode(&x->modeG723AnnexCAudioMode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_G7231AnnexCMode: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for ModeG723AnnexCAudioMode (SEQUENCE)  */
/* <==================================================> */
void Decode_ModeG723AnnexCAudioMode(PS_ModeG723AnnexCAudioMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->highRateMode0 = (uint8) GetInteger(27, 78, stream);
    x->highRateMode1 = (uint8) GetInteger(27, 78, stream);
    x->lowRateMode0 = (uint8) GetInteger(23, 66, stream);
    x->lowRateMode1 = (uint8) GetInteger(23, 66, stream);
    x->sidMode0 = (uint8) GetInteger(6, 17, stream);
    x->sidMode1 = (uint8) GetInteger(6, 17, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ModeG723AnnexCAudioMode: Unknown extensions (skipped)");
        }
    }
}

/* <===================================> */
/*  PER-Decoder for DataMode (SEQUENCE)  */
/* <===================================> */
void Decode_DataMode(PS_DataMode x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_DmApplication(&x->dmApplication, stream);
    x->bitRate = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_DataMode: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for DmApplication (CHOICE)  */
/* <======================================> */
void Decode_DmApplication(PS_DmApplication x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->t120 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t120, stream);
            break;
        case 2:
            x->dsm_cc = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->dsm_cc, stream);
            break;
        case 3:
            x->userData = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->userData, stream);
            break;
        case 4:
            x->t84 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t84, stream);
            break;
        case 5:
            x->t434 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t434, stream);
            break;
        case 6:
            x->h224 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->h224, stream);
            break;
        case 7:
            x->dmNlpid = (PS_DmNlpid) OSCL_DEFAULT_MALLOC(sizeof(S_DmNlpid));
            Decode_DmNlpid(x->dmNlpid, stream);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            x->h222DataPartitioning = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->h222DataPartitioning, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            GetLengthDet(stream);
            x->t30fax = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t30fax, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->t140 = (PS_DataProtocolCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataProtocolCapability));
            Decode_DataProtocolCapability(x->t140, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->dmT38fax = (PS_DmT38fax) OSCL_DEFAULT_MALLOC(sizeof(S_DmT38fax));
            Decode_DmT38fax(x->dmT38fax, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->genericDataMode = (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            Decode_GenericCapability(x->genericDataMode, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_DmApplication: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================> */
/*  PER-Decoder for DmT38fax (SEQUENCE)  */
/* <===================================> */
void Decode_DmT38fax(PS_DmT38fax x, PS_InStream stream)
{
    Decode_DataProtocolCapability(&x->t38FaxProtocol, stream);
    Decode_T38FaxProfile(&x->t38FaxProfile, stream);
}

/* <==================================> */
/*  PER-Decoder for DmNlpid (SEQUENCE)  */
/* <==================================> */
void Decode_DmNlpid(PS_DmNlpid x, PS_InStream stream)
{
    Decode_DataProtocolCapability(&x->nlpidProtocol, stream);
    GetOctetString(1, 0, 0, &x->nlpidData, stream);
}

/* <=======================================> */
/*  PER-Decoder for EncryptionMode (CHOICE)  */
/* <=======================================> */
void Decode_EncryptionMode(PS_EncryptionMode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (h233Encryption is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_EncryptionMode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================================> */
/*  PER-Decoder for RoundTripDelayRequest (SEQUENCE)  */
/* <================================================> */
void Decode_RoundTripDelayRequest(PS_RoundTripDelayRequest x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RoundTripDelayRequest: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for RoundTripDelayResponse (SEQUENCE)  */
/* <=================================================> */
void Decode_RoundTripDelayResponse(PS_RoundTripDelayResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RoundTripDelayResponse: Unknown extensions (skipped)");
        }
    }
}

/* <=================================================> */
/*  PER-Decoder for MaintenanceLoopRequest (SEQUENCE)  */
/* <=================================================> */
void Decode_MaintenanceLoopRequest(PS_MaintenanceLoopRequest x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_MlRequestType(&x->mlRequestType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaintenanceLoopRequest: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for MlRequestType (CHOICE)  */
/* <======================================> */
void Decode_MlRequestType(PS_MlRequestType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            x->mediaLoop = (uint16) GetInteger(1, 65535, stream);
            break;
        case 2:
            x->logicalChannelLoop = (uint16) GetInteger(1, 65535, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MlRequestType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for MaintenanceLoopAck (SEQUENCE)  */
/* <=============================================> */
void Decode_MaintenanceLoopAck(PS_MaintenanceLoopAck x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_MlAckType(&x->mlAckType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaintenanceLoopAck: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for MlAckType (CHOICE)  */
/* <==================================> */
void Decode_MlAckType(PS_MlAckType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            x->mediaLoop = (uint16) GetInteger(1, 65535, stream);
            break;
        case 2:
            x->logicalChannelLoop = (uint16) GetInteger(1, 65535, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MlAckType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================================> */
/*  PER-Decoder for MaintenanceLoopReject (SEQUENCE)  */
/* <================================================> */
void Decode_MaintenanceLoopReject(PS_MaintenanceLoopReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_MlRejectType(&x->mlRejectType, stream);
    Decode_MlRejectCause(&x->mlRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaintenanceLoopReject: Unknown extensions (skipped)");
        }
    }
}

/* <======================================> */
/*  PER-Decoder for MlRejectCause (CHOICE)  */
/* <======================================> */
void Decode_MlRejectCause(PS_MlRejectCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (canNotPerformLoop is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MlRejectCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for MlRejectType (CHOICE)  */
/* <=====================================> */
void Decode_MlRejectType(PS_MlRejectType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            x->mediaLoop = (uint16) GetInteger(1, 65535, stream);
            break;
        case 2:
            x->logicalChannelLoop = (uint16) GetInteger(1, 65535, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MlRejectType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================================> */
/*  PER-Decoder for MaintenanceLoopOffCommand (SEQUENCE)  */
/* <====================================================> */
void Decode_MaintenanceLoopOffCommand(PS_MaintenanceLoopOffCommand x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaintenanceLoopOffCommand: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for CommunicationModeCommand (SEQUENCE)  */
/* <===================================================> */
void Decode_CommunicationModeCommand(PS_CommunicationModeCommand x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_communicationModeTable = (uint16) GetInteger(1, 256, stream);
    x->communicationModeTable = (PS_CommunicationModeTableEntry)
                                OSCL_DEFAULT_MALLOC(x->size_of_communicationModeTable * sizeof(S_CommunicationModeTableEntry));
    for (i = 0;i < x->size_of_communicationModeTable;++i)
    {
        Decode_CommunicationModeTableEntry(x->communicationModeTable + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CommunicationModeCommand: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for CommunicationModeRequest (SEQUENCE)  */
/* <===================================================> */
void Decode_CommunicationModeRequest(PS_CommunicationModeRequest x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CommunicationModeRequest: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for CommunicationModeResponse (CHOICE)  */
/* <==================================================> */
void Decode_CommunicationModeResponse(PS_CommunicationModeResponse x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(1, 1, stream);
    switch (x->index)
    {
        case 0:
            x->size = (uint16) GetInteger(1, 256, stream);
            x->communicationModeTable = (PS_CommunicationModeTableEntry)
                                        OSCL_DEFAULT_MALLOC(x->size * sizeof(S_CommunicationModeTableEntry));
            for (i = 0;i < x->size;++i)
            {
                Decode_CommunicationModeTableEntry(x->communicationModeTable + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CommunicationModeResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <======================================================> */
/*  PER-Decoder for CommunicationModeTableEntry (SEQUENCE)  */
/* <======================================================> */
void Decode_CommunicationModeTableEntry(PS_CommunicationModeTableEntry x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_nonStandard = GetBoolean(stream);
    x->option_of_associatedSessionID = GetBoolean(stream);
    x->option_of_terminalLabel = GetBoolean(stream);
    x->option_of_mediaChannel = GetBoolean(stream);
    x->option_of_mediaGuaranteedDelivery = GetBoolean(stream);
    x->option_of_mediaControlChannel = GetBoolean(stream);
    x->option_of_mediaControlGuaranteedDelivery = GetBoolean(stream);
    if (x->option_of_nonStandard)
    {
        x->size_of_nonStandard = (uint16) GetLengthDet(stream);
        x->nonStandard = (PS_NonStandardParameter)
                         OSCL_DEFAULT_MALLOC(x->size_of_nonStandard * sizeof(S_NonStandardParameter));
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Decode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    x->sessionID = (uint8) GetInteger(1, 255, stream);
    if (x->option_of_associatedSessionID)
    {
        x->associatedSessionID = (uint8) GetInteger(1, 255, stream);
    }
    if (x->option_of_terminalLabel)
    {
        Decode_TerminalLabel(&x->terminalLabel, stream);
    }
    GetCharString("BMPString", 0, 1, 128, NULL, &x->sessionDescription, stream);
    Decode_CmtDataType(&x->cmtDataType, stream);
    if (x->option_of_mediaChannel)
    {
        Decode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        x->mediaGuaranteedDelivery = GetBoolean(stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Decode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        x->mediaControlGuaranteedDelivery = GetBoolean(stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_redundancyEncoding = OFF;
    x->option_of_sessionDependency = OFF;
    x->option_of_destination = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_redundancyEncoding = SigMapValue(0, map);
        if (x->option_of_redundancyEncoding)
        {
            ExtensionPrep(map, stream);
            Decode_RedundancyEncoding(&x->redundancyEncoding, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_sessionDependency = SigMapValue(1, map);
        if (x->option_of_sessionDependency)
        {
            ExtensionPrep(map, stream);
            x->sessionDependency = (uint8) GetInteger(1, 255, stream);
            ReadRemainingBits(stream);
        }
        x->option_of_destination = SigMapValue(2, map);
        if (x->option_of_destination)
        {
            ExtensionPrep(map, stream);
            Decode_TerminalLabel(&x->destination, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CommunicationModeTableEntry: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for CmtDataType (CHOICE)  */
/* <====================================> */
void Decode_CmtDataType(PS_CmtDataType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            Decode_VideoCapability(x->videoData, stream);
            break;
        case 1:
            x->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            Decode_AudioCapability(x->audioData, stream);
            break;
        case 2:
            x->data = (PS_DataApplicationCapability) OSCL_DEFAULT_MALLOC(sizeof(S_DataApplicationCapability));
            Decode_DataApplicationCapability(x->data, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CmtDataType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for ConferenceRequest (CHOICE)  */
/* <==========================================> */
void Decode_ConferenceRequest(PS_ConferenceRequest x, PS_InStream stream)
{
    x->index = GetChoiceIndex(8, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (terminalListRequest is NULL) */
            break;
        case 1:
            /* (makeMeChair is NULL) */
            break;
        case 2:
            /* (cancelMakeMeChair is NULL) */
            break;
        case 3:
            x->dropTerminal = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->dropTerminal, stream);
            break;
        case 4:
            x->requestTerminalID = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->requestTerminalID, stream);
            break;
        case 5:
            /* (enterH243Password is NULL) */
            break;
        case 6:
            /* (enterH243TerminalID is NULL) */
            break;
        case 7:
            /* (enterH243ConferenceID is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            GetLengthDet(stream);
            /* (enterExtensionAddress is NULL) */
            SkipOneOctet(stream);
            break;
        case 9:
            GetLengthDet(stream);
            /* (requestChairTokenOwner is NULL) */
            SkipOneOctet(stream);
            break;
        case 10:
            GetLengthDet(stream);
            x->requestTerminalCertificate = (PS_RequestTerminalCertificate) OSCL_DEFAULT_MALLOC(sizeof(S_RequestTerminalCertificate));
            Decode_RequestTerminalCertificate(x->requestTerminalCertificate, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->broadcastMyLogicalChannel = (uint16) GetInteger(1, 65535, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->makeTerminalBroadcaster = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->makeTerminalBroadcaster, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->sendThisSource = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->sendThisSource, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            /* (requestAllTerminalIDs is NULL) */
            SkipOneOctet(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->remoteMCRequest = (PS_RemoteMCRequest) OSCL_DEFAULT_MALLOC(sizeof(S_RemoteMCRequest));
            Decode_RemoteMCRequest(x->remoteMCRequest, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ConferenceRequest: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================================> */
/*  PER-Decoder for RequestTerminalCertificate (SEQUENCE)  */
/* <=====================================================> */
void Decode_RequestTerminalCertificate(PS_RequestTerminalCertificate x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_terminalLabel = GetBoolean(stream);
    x->option_of_certSelectionCriteria = GetBoolean(stream);
    x->option_of_sRandom = GetBoolean(stream);
    if (x->option_of_terminalLabel)
    {
        Decode_TerminalLabel(&x->terminalLabel, stream);
    }
    if (x->option_of_certSelectionCriteria)
    {
        Decode_CertSelectionCriteria(&x->certSelectionCriteria, stream);
    }
    if (x->option_of_sRandom)
    {
        x->sRandom = GetInteger(1, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestTerminalCertificate: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for CertSelectionCriteria (SEQUENCE-OF)  */
/* <===================================================> */
void Decode_CertSelectionCriteria(PS_CertSelectionCriteria x, PS_InStream stream)
{
    uint16 i;
    x->size = (uint8) GetInteger(1, 16, stream);
    x->item = (PS_Criteria)
              OSCL_DEFAULT_MALLOC(x->size * sizeof(S_Criteria));
    for (i = 0;i < x->size;++i)
    {
        Decode_Criteria(x->item + i, stream);
    }
}

/* <===================================> */
/*  PER-Decoder for Criteria (SEQUENCE)  */
/* <===================================> */
void Decode_Criteria(PS_Criteria x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetObjectID(&x->field, stream);
    GetOctetString(0, 1, 65535, &x->value, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Criteria: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for TerminalLabel (SEQUENCE)  */
/* <========================================> */
void Decode_TerminalLabel(PS_TerminalLabel x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->mcuNumber = (uint8) GetInteger(0, 192, stream);
    x->terminalNumber = (uint8) GetInteger(0, 192, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalLabel: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for ConferenceResponse (CHOICE)  */
/* <===========================================> */
void Decode_ConferenceResponse(PS_ConferenceResponse x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(8, 1, stream);
    switch (x->index)
    {
        case 0:
            x->mCTerminalIDResponse = (PS_MCTerminalIDResponse) OSCL_DEFAULT_MALLOC(sizeof(S_MCTerminalIDResponse));
            Decode_MCTerminalIDResponse(x->mCTerminalIDResponse, stream);
            break;
        case 1:
            x->terminalIDResponse = (PS_TerminalIDResponse) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalIDResponse));
            Decode_TerminalIDResponse(x->terminalIDResponse, stream);
            break;
        case 2:
            x->conferenceIDResponse = (PS_ConferenceIDResponse) OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceIDResponse));
            Decode_ConferenceIDResponse(x->conferenceIDResponse, stream);
            break;
        case 3:
            x->passwordResponse = (PS_PasswordResponse) OSCL_DEFAULT_MALLOC(sizeof(S_PasswordResponse));
            Decode_PasswordResponse(x->passwordResponse, stream);
            break;
        case 4:
            x->size = (uint16) GetInteger(1, 256, stream);
            x->terminalListResponse = (PS_TerminalLabel)
                                      OSCL_DEFAULT_MALLOC(x->size * sizeof(S_TerminalLabel));
            for (i = 0;i < x->size;++i)
            {
                Decode_TerminalLabel(x->terminalListResponse + i, stream);
            }
            break;
        case 5:
            /* (videoCommandReject is NULL) */
            break;
        case 6:
            /* (terminalDropReject is NULL) */
            break;
        case 7:
            x->makeMeChairResponse = (PS_MakeMeChairResponse) OSCL_DEFAULT_MALLOC(sizeof(S_MakeMeChairResponse));
            Decode_MakeMeChairResponse(x->makeMeChairResponse, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            GetLengthDet(stream);
            x->extensionAddressResponse = (PS_ExtensionAddressResponse) OSCL_DEFAULT_MALLOC(sizeof(S_ExtensionAddressResponse));
            Decode_ExtensionAddressResponse(x->extensionAddressResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 9:
            GetLengthDet(stream);
            x->chairTokenOwnerResponse = (PS_ChairTokenOwnerResponse) OSCL_DEFAULT_MALLOC(sizeof(S_ChairTokenOwnerResponse));
            Decode_ChairTokenOwnerResponse(x->chairTokenOwnerResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 10:
            GetLengthDet(stream);
            x->terminalCertificateResponse = (PS_TerminalCertificateResponse) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCertificateResponse));
            Decode_TerminalCertificateResponse(x->terminalCertificateResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->broadcastMyLogicalChannelResponse = (PS_BroadcastMyLogicalChannelResponse) OSCL_DEFAULT_MALLOC(sizeof(S_BroadcastMyLogicalChannelResponse));
            Decode_BroadcastMyLogicalChannelResponse(x->broadcastMyLogicalChannelResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->makeTerminalBroadcasterResponse = (PS_MakeTerminalBroadcasterResponse) OSCL_DEFAULT_MALLOC(sizeof(S_MakeTerminalBroadcasterResponse));
            Decode_MakeTerminalBroadcasterResponse(x->makeTerminalBroadcasterResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->sendThisSourceResponse = (PS_SendThisSourceResponse) OSCL_DEFAULT_MALLOC(sizeof(S_SendThisSourceResponse));
            Decode_SendThisSourceResponse(x->sendThisSourceResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            x->requestAllTerminalIDsResponse = (PS_RequestAllTerminalIDsResponse) OSCL_DEFAULT_MALLOC(sizeof(S_RequestAllTerminalIDsResponse));
            Decode_RequestAllTerminalIDsResponse(x->requestAllTerminalIDsResponse, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            x->remoteMCResponse = (PS_RemoteMCResponse) OSCL_DEFAULT_MALLOC(sizeof(S_RemoteMCResponse));
            Decode_RemoteMCResponse(x->remoteMCResponse, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ConferenceResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================================> */
/*  PER-Decoder for SendThisSourceResponse (CHOICE)  */
/* <===============================================> */
void Decode_SendThisSourceResponse(PS_SendThisSourceResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (grantedSendThisSource is NULL) */
            break;
        case 1:
            /* (deniedSendThisSource is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_SendThisSourceResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================================> */
/*  PER-Decoder for MakeTerminalBroadcasterResponse (CHOICE)  */
/* <========================================================> */
void Decode_MakeTerminalBroadcasterResponse(PS_MakeTerminalBroadcasterResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (grantedMakeTerminalBroadcaster is NULL) */
            break;
        case 1:
            /* (deniedMakeTerminalBroadcaster is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MakeTerminalBroadcasterResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================================> */
/*  PER-Decoder for BroadcastMyLogicalChannelResponse (CHOICE)  */
/* <==========================================================> */
void Decode_BroadcastMyLogicalChannelResponse(PS_BroadcastMyLogicalChannelResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (grantedBroadcastMyLogicalChannel is NULL) */
            break;
        case 1:
            /* (deniedBroadcastMyLogicalChannel is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_BroadcastMyLogicalChannelResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <======================================================> */
/*  PER-Decoder for TerminalCertificateResponse (SEQUENCE)  */
/* <======================================================> */
void Decode_TerminalCertificateResponse(PS_TerminalCertificateResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_terminalLabel = GetBoolean(stream);
    x->option_of_certificateResponse = GetBoolean(stream);
    if (x->option_of_terminalLabel)
    {
        Decode_TerminalLabel(&x->terminalLabel, stream);
    }
    if (x->option_of_certificateResponse)
    {
        GetOctetString(0, 1, 65535, &x->certificateResponse, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalCertificateResponse: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for ChairTokenOwnerResponse (SEQUENCE)  */
/* <==================================================> */
void Decode_ChairTokenOwnerResponse(PS_ChairTokenOwnerResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ChairTokenOwnerResponse: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for ExtensionAddressResponse (SEQUENCE)  */
/* <===================================================> */
void Decode_ExtensionAddressResponse(PS_ExtensionAddressResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 1, 128, &x->extensionAddress, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ExtensionAddressResponse: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for MakeMeChairResponse (CHOICE)  */
/* <============================================> */
void Decode_MakeMeChairResponse(PS_MakeMeChairResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (grantedChairToken is NULL) */
            break;
        case 1:
            /* (deniedChairToken is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MakeMeChairResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===========================================> */
/*  PER-Decoder for PasswordResponse (SEQUENCE)  */
/* <===========================================> */
void Decode_PasswordResponse(PS_PasswordResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 32, &x->password, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_PasswordResponse: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for ConferenceIDResponse (SEQUENCE)  */
/* <===============================================> */
void Decode_ConferenceIDResponse(PS_ConferenceIDResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 32, &x->conferenceID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ConferenceIDResponse: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for TerminalIDResponse (SEQUENCE)  */
/* <=============================================> */
void Decode_TerminalIDResponse(PS_TerminalIDResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalIDResponse: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for MCTerminalIDResponse (SEQUENCE)  */
/* <===============================================> */
void Decode_MCTerminalIDResponse(PS_MCTerminalIDResponse x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MCTerminalIDResponse: Unknown extensions (skipped)");
        }
    }
}

/* <========================================================> */
/*  PER-Decoder for RequestAllTerminalIDsResponse (SEQUENCE)  */
/* <========================================================> */
void Decode_RequestAllTerminalIDsResponse(PS_RequestAllTerminalIDsResponse x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->size_of_terminalInformation = (uint16) GetLengthDet(stream);
    x->terminalInformation = (PS_TerminalInformation)
                             OSCL_DEFAULT_MALLOC(x->size_of_terminalInformation * sizeof(S_TerminalInformation));
    for (i = 0;i < x->size_of_terminalInformation;++i)
    {
        Decode_TerminalInformation(x->terminalInformation + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RequestAllTerminalIDsResponse: Unknown extensions (skipped)");
        }
    }
}

/* <==============================================> */
/*  PER-Decoder for TerminalInformation (SEQUENCE)  */
/* <==============================================> */
void Decode_TerminalInformation(PS_TerminalInformation x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TerminalLabel(&x->terminalLabel, stream);
    GetOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalInformation: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for RemoteMCRequest (CHOICE)  */
/* <========================================> */
void Decode_RemoteMCRequest(PS_RemoteMCRequest x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (masterActivate is NULL) */
            break;
        case 1:
            /* (slaveActivate is NULL) */
            break;
        case 2:
            /* (deActivate is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RemoteMCRequest: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for RemoteMCResponse (CHOICE)  */
/* <=========================================> */
void Decode_RemoteMCResponse(PS_RemoteMCResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (accept is NULL) */
            break;
        case 1:
            x->reject = (PS_Reject) OSCL_DEFAULT_MALLOC(sizeof(S_Reject));
            Decode_Reject(x->reject, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RemoteMCResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================> */
/*  PER-Decoder for Reject (CHOICE)  */
/* <===============================> */
void Decode_Reject(PS_Reject x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
        case 1:
            /* (functionNotSupported is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Reject: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for MultilinkRequest (CHOICE)  */
/* <=========================================> */
void Decode_MultilinkRequest(PS_MultilinkRequest x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->callInformation = (PS_CallInformation) OSCL_DEFAULT_MALLOC(sizeof(S_CallInformation));
            Decode_CallInformation(x->callInformation, stream);
            break;
        case 2:
            x->addConnection = (PS_AddConnection) OSCL_DEFAULT_MALLOC(sizeof(S_AddConnection));
            Decode_AddConnection(x->addConnection, stream);
            break;
        case 3:
            x->removeConnection = (PS_RemoveConnection) OSCL_DEFAULT_MALLOC(sizeof(S_RemoveConnection));
            Decode_RemoveConnection(x->removeConnection, stream);
            break;
        case 4:
            x->maximumHeaderInterval = (PS_MaximumHeaderInterval) OSCL_DEFAULT_MALLOC(sizeof(S_MaximumHeaderInterval));
            Decode_MaximumHeaderInterval(x->maximumHeaderInterval, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MultilinkRequest: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================================> */
/*  PER-Decoder for MaximumHeaderInterval (SEQUENCE)  */
/* <================================================> */
void Decode_MaximumHeaderInterval(PS_MaximumHeaderInterval x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_RequestType(&x->requestType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MaximumHeaderInterval: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for RequestType (CHOICE)  */
/* <====================================> */
void Decode_RequestType(PS_RequestType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (currentIntervalInformation is NULL) */
            break;
        case 1:
            x->requestedInterval = (uint16) GetInteger(0, 65535, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_RequestType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===========================================> */
/*  PER-Decoder for RemoveConnection (SEQUENCE)  */
/* <===========================================> */
void Decode_RemoveConnection(PS_RemoveConnection x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RemoveConnection: Unknown extensions (skipped)");
        }
    }
}

/* <========================================> */
/*  PER-Decoder for AddConnection (SEQUENCE)  */
/* <========================================> */
void Decode_AddConnection(PS_AddConnection x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    Decode_DialingInformation(&x->dialingInformation, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_AddConnection: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for CallInformation (SEQUENCE)  */
/* <==========================================> */
void Decode_CallInformation(PS_CallInformation x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->maxNumberOfAdditionalConnections = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CallInformation: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for MultilinkResponse (CHOICE)  */
/* <==========================================> */
void Decode_MultilinkResponse(PS_MultilinkResponse x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->respCallInformation = (PS_RespCallInformation) OSCL_DEFAULT_MALLOC(sizeof(S_RespCallInformation));
            Decode_RespCallInformation(x->respCallInformation, stream);
            break;
        case 2:
            x->respAddConnection = (PS_RespAddConnection) OSCL_DEFAULT_MALLOC(sizeof(S_RespAddConnection));
            Decode_RespAddConnection(x->respAddConnection, stream);
            break;
        case 3:
            x->respRemoveConnection = (PS_RespRemoveConnection) OSCL_DEFAULT_MALLOC(sizeof(S_RespRemoveConnection));
            Decode_RespRemoveConnection(x->respRemoveConnection, stream);
            break;
        case 4:
            x->respMaximumHeaderInterval = (PS_RespMaximumHeaderInterval) OSCL_DEFAULT_MALLOC(sizeof(S_RespMaximumHeaderInterval));
            Decode_RespMaximumHeaderInterval(x->respMaximumHeaderInterval, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MultilinkResponse: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================================> */
/*  PER-Decoder for RespMaximumHeaderInterval (SEQUENCE)  */
/* <====================================================> */
void Decode_RespMaximumHeaderInterval(PS_RespMaximumHeaderInterval x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->currentInterval = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RespMaximumHeaderInterval: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for RespRemoveConnection (SEQUENCE)  */
/* <===============================================> */
void Decode_RespRemoveConnection(PS_RespRemoveConnection x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RespRemoveConnection: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for RespAddConnection (SEQUENCE)  */
/* <============================================> */
void Decode_RespAddConnection(PS_RespAddConnection x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    Decode_ResponseCode(&x->responseCode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RespAddConnection: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for ResponseCode (CHOICE)  */
/* <=====================================> */
void Decode_ResponseCode(PS_ResponseCode x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (accepted is NULL) */
            break;
        case 1:
            x->rejected = (PS_Rejected) OSCL_DEFAULT_MALLOC(sizeof(S_Rejected));
            Decode_Rejected(x->rejected, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_ResponseCode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=================================> */
/*  PER-Decoder for Rejected (CHOICE)  */
/* <=================================> */
void Decode_Rejected(PS_Rejected x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (connectionsNotAvailable is NULL) */
            break;
        case 1:
            /* (userRejected is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Rejected: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================================> */
/*  PER-Decoder for RespCallInformation (SEQUENCE)  */
/* <==============================================> */
void Decode_RespCallInformation(PS_RespCallInformation x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_DialingInformation(&x->dialingInformation, stream);
    x->callAssociationNumber = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_RespCallInformation: Unknown extensions (skipped)");
        }
    }
}

/* <============================================> */
/*  PER-Decoder for MultilinkIndication (CHOICE)  */
/* <============================================> */
void Decode_MultilinkIndication(PS_MultilinkIndication x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->crcDesired = (PS_CrcDesired) OSCL_DEFAULT_MALLOC(sizeof(S_CrcDesired));
            Decode_CrcDesired(x->crcDesired, stream);
            break;
        case 2:
            x->excessiveError = (PS_ExcessiveError) OSCL_DEFAULT_MALLOC(sizeof(S_ExcessiveError));
            Decode_ExcessiveError(x->excessiveError, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_MultilinkIndication: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for ExcessiveError (SEQUENCE)  */
/* <=========================================> */
void Decode_ExcessiveError(PS_ExcessiveError x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ExcessiveError: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for CrcDesired (SEQUENCE)  */
/* <=====================================> */
void Decode_CrcDesired(PS_CrcDesired x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CrcDesired: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for DialingInformation (CHOICE)  */
/* <===========================================> */
void Decode_DialingInformation(PS_DialingInformation x, PS_InStream stream)
{
    uint16 i;
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            x->size = (uint16) GetInteger(1, 65535, stream);
            x->differential = (PS_DialingInformationNumber)
                              OSCL_DEFAULT_MALLOC(x->size * sizeof(S_DialingInformationNumber));
            for (i = 0;i < x->size;++i)
            {
                Decode_DialingInformationNumber(x->differential + i, stream);
            }
            break;
        case 2:
            x->infoNotAvailable = (uint16) GetInteger(1, 65535, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_DialingInformation: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===================================================> */
/*  PER-Decoder for DialingInformationNumber (SEQUENCE)  */
/* <===================================================> */
void Decode_DialingInformationNumber(PS_DialingInformationNumber x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_subAddress = GetBoolean(stream);
    GetCharString("NumericString", 0, 0, 40, NULL, &x->networkAddress, stream);
    if (x->option_of_subAddress)
    {
        GetCharString("IA5String", 0, 1, 40, NULL, &x->subAddress, stream);
    }
    x->size_of_networkType = (uint8) GetInteger(1, 255, stream);
    x->networkType = (PS_DialingInformationNetworkType)
                     OSCL_DEFAULT_MALLOC(x->size_of_networkType * sizeof(S_DialingInformationNetworkType));
    for (i = 0;i < x->size_of_networkType;++i)
    {
        Decode_DialingInformationNetworkType(x->networkType + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_DialingInformationNumber: Unknown extensions (skipped)");
        }
    }
}

/* <======================================================> */
/*  PER-Decoder for DialingInformationNetworkType (CHOICE)  */
/* <======================================================> */
void Decode_DialingInformationNetworkType(PS_DialingInformationNetworkType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardMessage) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            /* (n_isdn is NULL) */
            break;
        case 2:
            /* (gstn is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_DialingInformationNetworkType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================================> */
/*  PER-Decoder for ConnectionIdentifier (SEQUENCE)  */
/* <===============================================> */
void Decode_ConnectionIdentifier(PS_ConnectionIdentifier x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->channelTag = GetInteger(0, 0xffffffff, stream);
    x->sequenceNumber = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ConnectionIdentifier: Unknown extensions (skipped)");
        }
    }
}

/* <====================================================> */
/*  PER-Decoder for LogicalChannelRateRequest (SEQUENCE)  */
/* <====================================================> */
void Decode_LogicalChannelRateRequest(PS_LogicalChannelRateRequest x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    x->maximumBitRate = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_LogicalChannelRateRequest: Unknown extensions (skipped)");
        }
    }
}

/* <========================================================> */
/*  PER-Decoder for LogicalChannelRateAcknowledge (SEQUENCE)  */
/* <========================================================> */
void Decode_LogicalChannelRateAcknowledge(PS_LogicalChannelRateAcknowledge x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    x->maximumBitRate = GetInteger(0, 0xffffffff, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_LogicalChannelRateAcknowledge: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for LogicalChannelRateReject (SEQUENCE)  */
/* <===================================================> */
void Decode_LogicalChannelRateReject(PS_LogicalChannelRateReject x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_currentMaximumBitRate = GetBoolean(stream);
    x->sequenceNumber = (uint8) GetInteger(0, 255, stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_LogicalChannelRateRejectReason(&x->rejectReason, stream);
    if (x->option_of_currentMaximumBitRate)
    {
        x->currentMaximumBitRate = GetInteger(0, 0xffffffff, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_LogicalChannelRateReject: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================================> */
/*  PER-Decoder for LogicalChannelRateRejectReason (CHOICE)  */
/* <=======================================================> */
void Decode_LogicalChannelRateRejectReason(PS_LogicalChannelRateRejectReason x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (undefinedReason is NULL) */
            break;
        case 1:
            /* (insufficientResources is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_LogicalChannelRateRejectReason: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================================> */
/*  PER-Decoder for LogicalChannelRateRelease (SEQUENCE)  */
/* <====================================================> */
void Decode_LogicalChannelRateRelease(PS_LogicalChannelRateRelease x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    OSCL_UNUSED_ARG(x);

    extension = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_LogicalChannelRateRelease: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for SendTerminalCapabilitySet (CHOICE)  */
/* <==================================================> */
void Decode_SendTerminalCapabilitySet(PS_SendTerminalCapabilitySet x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->specificRequest = (PS_SpecificRequest) OSCL_DEFAULT_MALLOC(sizeof(S_SpecificRequest));
            Decode_SpecificRequest(x->specificRequest, stream);
            break;
        case 1:
            /* (genericRequest is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_SendTerminalCapabilitySet: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for SpecificRequest (SEQUENCE)  */
/* <==========================================> */
void Decode_SpecificRequest(PS_SpecificRequest x, PS_InStream stream)
{
    uint16 i;
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_capabilityTableEntryNumbers = GetBoolean(stream);
    x->option_of_capabilityDescriptorNumbers = GetBoolean(stream);
    x->multiplexCapability = GetBoolean(stream);
    if (x->option_of_capabilityTableEntryNumbers)
    {
        x->size_of_capabilityTableEntryNumbers = (uint16) GetInteger(1, 65535, stream);
        x->capabilityTableEntryNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_capabilityTableEntryNumbers * sizeof(uint32));
        for (i = 0;i < x->size_of_capabilityTableEntryNumbers;++i)
        {
            x->capabilityTableEntryNumbers[i] = GetInteger(1, 65535, stream);
        }
    }
    if (x->option_of_capabilityDescriptorNumbers)
    {
        x->size_of_capabilityDescriptorNumbers = (uint16) GetInteger(1, 256, stream);
        x->capabilityDescriptorNumbers = (uint32*) OSCL_DEFAULT_MALLOC(x->size_of_capabilityDescriptorNumbers * sizeof(uint32));
        for (i = 0;i < x->size_of_capabilityDescriptorNumbers;++i)
        {
            x->capabilityDescriptorNumbers[i] = GetInteger(0, 255, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_SpecificRequest: Unknown extensions (skipped)");
        }
    }
}

/* <==========================================> */
/*  PER-Decoder for EncryptionCommand (CHOICE)  */
/* <==========================================> */
void Decode_EncryptionCommand(PS_EncryptionCommand x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->encryptionSE = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            GetOctetString(1, 0, 0, x->encryptionSE, stream);
            break;
        case 1:
            /* (encryptionIVRequest is NULL) */
            break;
        case 2:
            x->encryptionAlgorithmID = (PS_EncryptionAlgorithmID) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionAlgorithmID));
            Decode_EncryptionAlgorithmID(x->encryptionAlgorithmID, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_EncryptionCommand: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================================> */
/*  PER-Decoder for EncryptionAlgorithmID (SEQUENCE)  */
/* <================================================> */
void Decode_EncryptionAlgorithmID(PS_EncryptionAlgorithmID x, PS_InStream stream)
{
    x->h233AlgorithmIdentifier = (uint8) GetInteger(0, 255, stream);
    Decode_NonStandardParameter(&x->associatedAlgorithm, stream);
}

/* <=============================================> */
/*  PER-Decoder for FlowControlCommand (SEQUENCE)  */
/* <=============================================> */
void Decode_FlowControlCommand(PS_FlowControlCommand x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_FccScope(&x->fccScope, stream);
    Decode_FccRestriction(&x->fccRestriction, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_FlowControlCommand: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for FccRestriction (CHOICE)  */
/* <=======================================> */
void Decode_FccRestriction(PS_FccRestriction x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->maximumBitRate = GetInteger(0, 16777215, stream);
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_FccRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Decoder for FccScope (CHOICE)  */
/* <=================================> */
void Decode_FccScope(PS_FccScope x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            x->resourceID = (uint16) GetInteger(0, 65535, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_FccScope: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Decoder for EndSessionCommand (CHOICE)  */
/* <==========================================> */
void Decode_EndSessionCommand(PS_EndSessionCommand x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (disconnect is NULL) */
            break;
        case 2:
            x->gstnOptions = (PS_GstnOptions) OSCL_DEFAULT_MALLOC(sizeof(S_GstnOptions));
            Decode_GstnOptions(x->gstnOptions, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            GetLengthDet(stream);
            x->isdnOptions = (PS_IsdnOptions) OSCL_DEFAULT_MALLOC(sizeof(S_IsdnOptions));
            Decode_IsdnOptions(x->isdnOptions, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_EndSessionCommand: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================> */
/*  PER-Decoder for IsdnOptions (CHOICE)  */
/* <====================================> */
void Decode_IsdnOptions(PS_IsdnOptions x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (telephonyMode is NULL) */
            break;
        case 1:
            /* (v140 is NULL) */
            break;
        case 2:
            /* (terminalOnHold is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IsdnOptions: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <====================================> */
/*  PER-Decoder for GstnOptions (CHOICE)  */
/* <====================================> */
void Decode_GstnOptions(PS_GstnOptions x, PS_InStream stream)
{
    x->index = GetChoiceIndex(5, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (telephonyMode is NULL) */
            break;
        case 1:
            /* (v8bis is NULL) */
            break;
        case 2:
            /* (v34DSVD is NULL) */
            break;
        case 3:
            /* (v34DuplexFAX is NULL) */
            break;
        case 4:
            /* (v34H324 is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_GstnOptions: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for ConferenceCommand (CHOICE)  */
/* <==========================================> */
void Decode_ConferenceCommand(PS_ConferenceCommand x, PS_InStream stream)
{
    x->index = GetChoiceIndex(7, 1, stream);
    switch (x->index)
    {
        case 0:
            x->broadcastMyLogicalChannel = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            x->cancelBroadcastMyLogicalChannel = (uint16) GetInteger(1, 65535, stream);
            break;
        case 2:
            x->makeTerminalBroadcaster = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->makeTerminalBroadcaster, stream);
            break;
        case 3:
            /* (cancelMakeTerminalBroadcaster is NULL) */
            break;
        case 4:
            x->sendThisSource = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->sendThisSource, stream);
            break;
        case 5:
            /* (cancelSendThisSource is NULL) */
            break;
        case 6:
            /* (dropConference is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            GetLengthDet(stream);
            x->substituteConferenceIDCommand = (PS_SubstituteConferenceIDCommand) OSCL_DEFAULT_MALLOC(sizeof(S_SubstituteConferenceIDCommand));
            Decode_SubstituteConferenceIDCommand(x->substituteConferenceIDCommand, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ConferenceCommand: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <========================================================> */
/*  PER-Decoder for SubstituteConferenceIDCommand (SEQUENCE)  */
/* <========================================================> */
void Decode_SubstituteConferenceIDCommand(PS_SubstituteConferenceIDCommand x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    GetOctetString(0, 16, 16, &x->conferenceIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_SubstituteConferenceIDCommand: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for MiscellaneousCommand (SEQUENCE)  */
/* <===============================================> */
void Decode_MiscellaneousCommand(PS_MiscellaneousCommand x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_McType(&x->mcType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MiscellaneousCommand: Unknown extensions (skipped)");
        }
    }
}

/* <===============================> */
/*  PER-Decoder for McType (CHOICE)  */
/* <===============================> */
void Decode_McType(PS_McType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (equaliseDelay is NULL) */
            break;
        case 1:
            /* (zeroDelay is NULL) */
            break;
        case 2:
            /* (multipointModeCommand is NULL) */
            break;
        case 3:
            /* (cancelMultipointModeCommand is NULL) */
            break;
        case 4:
            /* (videoFreezePicture is NULL) */
            break;
        case 5:
            /* (videoFastUpdatePicture is NULL) */
            break;
        case 6:
            x->videoFastUpdateGOB = (PS_VideoFastUpdateGOB) OSCL_DEFAULT_MALLOC(sizeof(S_VideoFastUpdateGOB));
            Decode_VideoFastUpdateGOB(x->videoFastUpdateGOB, stream);
            break;
        case 7:
            x->videoTemporalSpatialTradeOff = (uint8) GetInteger(0, 31, stream);
            break;
        case 8:
            /* (videoSendSyncEveryGOB is NULL) */
            break;
        case 9:
            /* (videoSendSyncEveryGOBCancel is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            GetLengthDet(stream);
            x->videoFastUpdateMB = (PS_VideoFastUpdateMB) OSCL_DEFAULT_MALLOC(sizeof(S_VideoFastUpdateMB));
            Decode_VideoFastUpdateMB(x->videoFastUpdateMB, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->maxH223MUXPDUsize = (uint16) GetInteger(1, (uint16) 65535, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->encryptionUpdate = (PS_EncryptionSync) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionSync));
            Decode_EncryptionSync(x->encryptionUpdate, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->encryptionUpdateRequest = (PS_EncryptionUpdateRequest) OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionUpdateRequest));
            Decode_EncryptionUpdateRequest(x->encryptionUpdateRequest, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            /* (switchReceiveMediaOff is NULL) */
            SkipOneOctet(stream);
            break;
        case 15:
            GetLengthDet(stream);
            /* (switchReceiveMediaOn is NULL) */
            SkipOneOctet(stream);
            break;
        case 16:
            GetLengthDet(stream);
            x->progressiveRefinementStart = (PS_ProgressiveRefinementStart) OSCL_DEFAULT_MALLOC(sizeof(S_ProgressiveRefinementStart));
            Decode_ProgressiveRefinementStart(x->progressiveRefinementStart, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            /* (progressiveRefinementAbortOne is NULL) */
            SkipOneOctet(stream);
            break;
        case 18:
            GetLengthDet(stream);
            /* (progressiveRefinementAbortContinuous is NULL) */
            SkipOneOctet(stream);
            break;
        default:
            ErrorMessage("Decode_McType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================================> */
/*  PER-Decoder for ProgressiveRefinementStart (SEQUENCE)  */
/* <=====================================================> */
void Decode_ProgressiveRefinementStart(PS_ProgressiveRefinementStart x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_PrsRepeatCount(&x->prsRepeatCount, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_ProgressiveRefinementStart: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for PrsRepeatCount (CHOICE)  */
/* <=======================================> */
void Decode_PrsRepeatCount(PS_PrsRepeatCount x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (doOneProgression is NULL) */
            break;
        case 1:
            /* (doContinuousProgressions is NULL) */
            break;
        case 2:
            /* (doOneIndependentProgression is NULL) */
            break;
        case 3:
            /* (doContinuousIndependentProgressions is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_PrsRepeatCount: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <============================================> */
/*  PER-Decoder for VideoFastUpdateMB (SEQUENCE)  */
/* <============================================> */
void Decode_VideoFastUpdateMB(PS_VideoFastUpdateMB x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_firstGOB = GetBoolean(stream);
    x->option_of_firstMB = GetBoolean(stream);
    if (x->option_of_firstGOB)
    {
        x->firstGOB = (uint8)  GetInteger(0, 255, stream);
    }
    if (x->option_of_firstMB)
    {
        x->firstMB = (uint16) GetInteger(1, 8192, stream);
    }
    x->numberOfMBs = (uint16) GetInteger(1, 8192, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VideoFastUpdateMB: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for VideoFastUpdateGOB (SEQUENCE)  */
/* <=============================================> */
void Decode_VideoFastUpdateGOB(PS_VideoFastUpdateGOB x, PS_InStream stream)
{
    x->firstGOB = (uint8) GetInteger(0, 17, stream);
    x->numberOfGOBs = (uint8) GetInteger(1, 18, stream);
}

/* <==============================================> */
/*  PER-Decoder for KeyProtectionMethod (SEQUENCE)  */
/* <==============================================> */
void Decode_KeyProtectionMethod(PS_KeyProtectionMethod x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->secureChannel = GetBoolean(stream);
    x->sharedSecret = GetBoolean(stream);
    x->certProtectedKey = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_KeyProtectionMethod: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for EncryptionUpdateRequest (SEQUENCE)  */
/* <==================================================> */
void Decode_EncryptionUpdateRequest(PS_EncryptionUpdateRequest x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_keyProtectionMethod = GetBoolean(stream);
    if (x->option_of_keyProtectionMethod)
    {
        Decode_KeyProtectionMethod(&x->keyProtectionMethod, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_EncryptionUpdateRequest: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================================> */
/*  PER-Decoder for H223MultiplexReconfiguration (CHOICE)  */
/* <=====================================================> */
void Decode_H223MultiplexReconfiguration(PS_H223MultiplexReconfiguration x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->h223ModeChange = (PS_H223ModeChange) OSCL_DEFAULT_MALLOC(sizeof(S_H223ModeChange));
            Decode_H223ModeChange(x->h223ModeChange, stream);
            break;
        case 1:
            x->h223AnnexADoubleFlag = (PS_H223AnnexADoubleFlag) OSCL_DEFAULT_MALLOC(sizeof(S_H223AnnexADoubleFlag));
            Decode_H223AnnexADoubleFlag(x->h223AnnexADoubleFlag, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_H223MultiplexReconfiguration: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for H223AnnexADoubleFlag (CHOICE)  */
/* <=============================================> */
void Decode_H223AnnexADoubleFlag(PS_H223AnnexADoubleFlag x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (start is NULL) */
            break;
        case 1:
            /* (stop is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_H223AnnexADoubleFlag: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for H223ModeChange (CHOICE)  */
/* <=======================================> */
void Decode_H223ModeChange(PS_H223ModeChange x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (toLevel0 is NULL) */
            break;
        case 1:
            /* (toLevel1 is NULL) */
            break;
        case 2:
            /* (toLevel2 is NULL) */
            break;
        case 3:
            /* (toLevel2withOptionalHeader is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_H223ModeChange: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==========================================> */
/*  PER-Decoder for NewATMVCCommand (SEQUENCE)  */
/* <==========================================> */
void Decode_NewATMVCCommand(PS_NewATMVCCommand x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->resourceID = (uint16) GetInteger(0, 65535, stream);
    x->bitRate = (uint16) GetInteger(1, 65535, stream);
    x->bitRateLockedToPCRClock = GetBoolean(stream);
    x->bitRateLockedToNetworkClock = GetBoolean(stream);
    Decode_CmdAal(&x->cmdAal, stream);
    Decode_CmdMultiplex(&x->cmdMultiplex, stream);
    Decode_CmdReverseParameters(&x->cmdReverseParameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_NewATMVCCommand: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for CmdReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Decode_CmdReverseParameters(PS_CmdReverseParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->bitRate = (uint16) GetInteger(1, 65535, stream);
    x->bitRateLockedToPCRClock = GetBoolean(stream);
    x->bitRateLockedToNetworkClock = GetBoolean(stream);
    Decode_Multiplex(&x->multiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CmdReverseParameters: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for Multiplex (CHOICE)  */
/* <==================================> */
void Decode_Multiplex(PS_Multiplex x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noMultiplex is NULL) */
            break;
        case 1:
            /* (transportStream is NULL) */
            break;
        case 2:
            /* (programStream is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_Multiplex: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for CmdMultiplex (CHOICE)  */
/* <=====================================> */
void Decode_CmdMultiplex(PS_CmdMultiplex x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noMultiplex is NULL) */
            break;
        case 1:
            /* (transportStream is NULL) */
            break;
        case 2:
            /* (programStream is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CmdMultiplex: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================> */
/*  PER-Decoder for CmdAal (CHOICE)  */
/* <===============================> */
void Decode_CmdAal(PS_CmdAal x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->cmdAal1 = (PS_CmdAal1) OSCL_DEFAULT_MALLOC(sizeof(S_CmdAal1));
            Decode_CmdAal1(x->cmdAal1, stream);
            break;
        case 1:
            x->cmdAal5 = (PS_CmdAal5) OSCL_DEFAULT_MALLOC(sizeof(S_CmdAal5));
            Decode_CmdAal5(x->cmdAal5, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CmdAal: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for CmdAal5 (SEQUENCE)  */
/* <==================================> */
void Decode_CmdAal5(PS_CmdAal5 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    x->backwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CmdAal5: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for CmdAal1 (SEQUENCE)  */
/* <==================================> */
void Decode_CmdAal1(PS_CmdAal1 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_CmdClockRecovery(&x->cmdClockRecovery, stream);
    Decode_CmdErrorCorrection(&x->cmdErrorCorrection, stream);
    x->structuredDataTransfer = GetBoolean(stream);
    x->partiallyFilledCells = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_CmdAal1: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for CmdErrorCorrection (CHOICE)  */
/* <===========================================> */
void Decode_CmdErrorCorrection(PS_CmdErrorCorrection x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (nullErrorCorrection is NULL) */
            break;
        case 1:
            /* (longInterleaver is NULL) */
            break;
        case 2:
            /* (shortInterleaver is NULL) */
            break;
        case 3:
            /* (errorCorrectionOnly is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CmdErrorCorrection: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for CmdClockRecovery (CHOICE)  */
/* <=========================================> */
void Decode_CmdClockRecovery(PS_CmdClockRecovery x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (nullClockRecovery is NULL) */
            break;
        case 1:
            /* (srtsClockRecovery is NULL) */
            break;
        case 2:
            /* (adaptiveClockRecovery is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_CmdClockRecovery: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==============================================> */
/*  PER-Decoder for FunctionNotUnderstood (CHOICE)  */
/* <==============================================> */
void Decode_FunctionNotUnderstood(PS_FunctionNotUnderstood x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            x->request = (PS_RequestMessage) OSCL_DEFAULT_MALLOC(sizeof(S_RequestMessage));
            Decode_RequestMessage(x->request, stream);
            break;
        case 1:
            x->response = (PS_ResponseMessage) OSCL_DEFAULT_MALLOC(sizeof(S_ResponseMessage));
            Decode_ResponseMessage(x->response, stream);
            break;
        case 2:
            x->command = (PS_CommandMessage) OSCL_DEFAULT_MALLOC(sizeof(S_CommandMessage));
            Decode_CommandMessage(x->command, stream);
            break;
        default:
            ErrorMessageAndLeave("Decode_FunctionNotUnderstood: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Decoder for FunctionNotSupported (SEQUENCE)  */
/* <===============================================> */
void Decode_FunctionNotSupported(PS_FunctionNotSupported x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_returnedFunction = GetBoolean(stream);
    Decode_FnsCause(&x->fnsCause, stream);
    if (x->option_of_returnedFunction)
    {
        GetOctetString(1, 0, 0, &x->returnedFunction, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_FunctionNotSupported: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for FnsCause (CHOICE)  */
/* <=================================> */
void Decode_FnsCause(PS_FnsCause x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (syntaxError is NULL) */
            break;
        case 1:
            /* (semanticError is NULL) */
            break;
        case 2:
            /* (unknownFunction is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_FnsCause: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for ConferenceIndication (CHOICE)  */
/* <=============================================> */
void Decode_ConferenceIndication(PS_ConferenceIndication x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 1, stream);
    switch (x->index)
    {
        case 0:
            x->sbeNumber = (uint8) GetInteger(0, 9, stream);
            break;
        case 1:
            x->terminalNumberAssign = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->terminalNumberAssign, stream);
            break;
        case 2:
            x->terminalJoinedConference = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->terminalJoinedConference, stream);
            break;
        case 3:
            x->terminalLeftConference = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->terminalLeftConference, stream);
            break;
        case 4:
            /* (seenByAtLeastOneOther is NULL) */
            break;
        case 5:
            /* (cancelSeenByAtLeastOneOther is NULL) */
            break;
        case 6:
            /* (seenByAll is NULL) */
            break;
        case 7:
            /* (cancelSeenByAll is NULL) */
            break;
        case 8:
            x->terminalYouAreSeeing = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->terminalYouAreSeeing, stream);
            break;
        case 9:
            /* (requestForFloor is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            GetLengthDet(stream);
            /* (withdrawChairToken is NULL) */
            SkipOneOctet(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->floorRequested = (PS_TerminalLabel) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalLabel));
            Decode_TerminalLabel(x->floorRequested, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            x->terminalYouAreSeeingInSubPictureNumber = (PS_TerminalYouAreSeeingInSubPictureNumber) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalYouAreSeeingInSubPictureNumber));
            Decode_TerminalYouAreSeeingInSubPictureNumber(x->terminalYouAreSeeingInSubPictureNumber, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            x->videoIndicateCompose = (PS_VideoIndicateCompose) OSCL_DEFAULT_MALLOC(sizeof(S_VideoIndicateCompose));
            Decode_VideoIndicateCompose(x->videoIndicateCompose, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ConferenceIndication: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=================================================================> */
/*  PER-Decoder for TerminalYouAreSeeingInSubPictureNumber (SEQUENCE)  */
/* <=================================================================> */
void Decode_TerminalYouAreSeeingInSubPictureNumber(PS_TerminalYouAreSeeingInSubPictureNumber x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->terminalNumber = (uint8) GetInteger(0, 192, stream);
    x->subPictureNumber = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_TerminalYouAreSeeingInSubPictureNumber: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for VideoIndicateCompose (SEQUENCE)  */
/* <===============================================> */
void Decode_VideoIndicateCompose(PS_VideoIndicateCompose x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->compositionNumber = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VideoIndicateCompose: Unknown extensions (skipped)");
        }
    }
}

/* <==================================================> */
/*  PER-Decoder for MiscellaneousIndication (SEQUENCE)  */
/* <==================================================> */
void Decode_MiscellaneousIndication(PS_MiscellaneousIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    Decode_MiType(&x->miType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MiscellaneousIndication: Unknown extensions (skipped)");
        }
    }
}

/* <===============================> */
/*  PER-Decoder for MiType (CHOICE)  */
/* <===============================> */
void Decode_MiType(PS_MiType x, PS_InStream stream)
{
    x->index = GetChoiceIndex(10, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (logicalChannelActive is NULL) */
            break;
        case 1:
            /* (logicalChannelInactive is NULL) */
            break;
        case 2:
            /* (multipointConference is NULL) */
            break;
        case 3:
            /* (cancelMultipointConference is NULL) */
            break;
        case 4:
            /* (multipointZeroComm is NULL) */
            break;
        case 5:
            /* (cancelMultipointZeroComm is NULL) */
            break;
        case 6:
            /* (multipointSecondaryStatus is NULL) */
            break;
        case 7:
            /* (cancelMultipointSecondaryStatus is NULL) */
            break;
        case 8:
            /* (videoIndicateReadyToActivate is NULL) */
            break;
        case 9:
            x->videoTemporalSpatialTradeOff = (uint8) GetInteger(0, 31, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            GetLengthDet(stream);
            x->videoNotDecodedMBs = (PS_VideoNotDecodedMBs) OSCL_DEFAULT_MALLOC(sizeof(S_VideoNotDecodedMBs));
            Decode_VideoNotDecodedMBs(x->videoNotDecodedMBs, stream);
            ReadRemainingBits(stream);
            break;
        case 11:
            GetLengthDet(stream);
            x->transportCapability = (PS_TransportCapability) OSCL_DEFAULT_MALLOC(sizeof(S_TransportCapability));
            Decode_TransportCapability(x->transportCapability, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_MiType: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=============================================> */
/*  PER-Decoder for VideoNotDecodedMBs (SEQUENCE)  */
/* <=============================================> */
void Decode_VideoNotDecodedMBs(PS_VideoNotDecodedMBs x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->firstMB = (uint16) GetInteger(1, 8192, stream);
    x->numberOfMBs = (uint16) GetInteger(1, 8192, stream);
    x->temporalReference = (uint8) GetInteger(0, 255, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VideoNotDecodedMBs: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for JitterIndication (SEQUENCE)  */
/* <===========================================> */
void Decode_JitterIndication(PS_JitterIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_skippedFrameCount = GetBoolean(stream);
    x->option_of_additionalDecoderBuffer = GetBoolean(stream);
    Decode_JiScope(&x->jiScope, stream);
    x->estimatedReceivedJitterMantissa = (uint8) GetInteger(0, 3, stream);
    x->estimatedReceivedJitterExponent = (uint8) GetInteger(0, 7, stream);
    if (x->option_of_skippedFrameCount)
    {
        x->skippedFrameCount = (uint8) GetInteger(0, 15, stream);
    }
    if (x->option_of_additionalDecoderBuffer)
    {
        x->additionalDecoderBuffer = GetInteger(0, 262143, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_JitterIndication: Unknown extensions (skipped)");
        }
    }
}

/* <================================> */
/*  PER-Decoder for JiScope (CHOICE)  */
/* <================================> */
void Decode_JiScope(PS_JiScope x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            x->resourceID = (uint16) GetInteger(0, 65535, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_JiScope: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Decoder for H223SkewIndication (SEQUENCE)  */
/* <=============================================> */
void Decode_H223SkewIndication(PS_H223SkewIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->logicalChannelNumber1 = (uint16) GetInteger(1, 65535, stream);
    x->logicalChannelNumber2 = (uint16) GetInteger(1, 65535, stream);
    x->skew = (uint16) GetInteger(0, 4095, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H223SkewIndication: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================================> */
/*  PER-Decoder for H2250MaximumSkewIndication (SEQUENCE)  */
/* <=====================================================> */
void Decode_H2250MaximumSkewIndication(PS_H2250MaximumSkewIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->logicalChannelNumber1 = (uint16) GetInteger(1, 65535, stream);
    x->logicalChannelNumber2 = (uint16) GetInteger(1, 65535, stream);
    x->maximumSkew = (uint16) GetInteger(0, 4095, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_H2250MaximumSkewIndication: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for MCLocationIndication (SEQUENCE)  */
/* <===============================================> */
void Decode_MCLocationIndication(PS_MCLocationIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_TransportAddress(&x->signalAddress, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_MCLocationIndication: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for VendorIdentification (SEQUENCE)  */
/* <===============================================> */
void Decode_VendorIdentification(PS_VendorIdentification x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_productNumber = GetBoolean(stream);
    x->option_of_versionNumber = GetBoolean(stream);
    Decode_NonStandardIdentifier(&x->vendor, stream);
    if (x->option_of_productNumber)
    {
        GetOctetString(0, 1, 256, &x->productNumber, stream);
    }
    if (x->option_of_versionNumber)
    {
        GetOctetString(0, 1, 256, &x->versionNumber, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_VendorIdentification: Unknown extensions (skipped)");
        }
    }
}

/* <=============================================> */
/*  PER-Decoder for NewATMVCIndication (SEQUENCE)  */
/* <=============================================> */
void Decode_NewATMVCIndication(PS_NewATMVCIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->resourceID = (uint16) GetInteger(0, 65535, stream);
    x->bitRate = (uint16) GetInteger(1, 65535, stream);
    x->bitRateLockedToPCRClock = GetBoolean(stream);
    x->bitRateLockedToNetworkClock = GetBoolean(stream);
    Decode_IndAal(&x->indAal, stream);
    Decode_IndMultiplex(&x->indMultiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    x->option_of_indReverseParameters = OFF;
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        x->option_of_indReverseParameters = SigMapValue(0, map);
        if (x->option_of_indReverseParameters)
        {
            ExtensionPrep(map, stream);
            Decode_IndReverseParameters(&x->indReverseParameters, stream);
            ReadRemainingBits(stream);
        }
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_NewATMVCIndication: Unknown extensions (skipped)");
        }
    }
}

/* <===============================================> */
/*  PER-Decoder for IndReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Decode_IndReverseParameters(PS_IndReverseParameters x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->bitRate = (uint16) GetInteger(1, 65535, stream);
    x->bitRateLockedToPCRClock = GetBoolean(stream);
    x->bitRateLockedToNetworkClock = GetBoolean(stream);
    Decode_IrpMultiplex(&x->irpMultiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IndReverseParameters: Unknown extensions (skipped)");
        }
    }
}

/* <=====================================> */
/*  PER-Decoder for IrpMultiplex (CHOICE)  */
/* <=====================================> */
void Decode_IrpMultiplex(PS_IrpMultiplex x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noMultiplex is NULL) */
            break;
        case 1:
            /* (transportStream is NULL) */
            break;
        case 2:
            /* (programStream is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IrpMultiplex: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=====================================> */
/*  PER-Decoder for IndMultiplex (CHOICE)  */
/* <=====================================> */
void Decode_IndMultiplex(PS_IndMultiplex x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (noMultiplex is NULL) */
            break;
        case 1:
            /* (transportStream is NULL) */
            break;
        case 2:
            /* (programStream is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IndMultiplex: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <===============================> */
/*  PER-Decoder for IndAal (CHOICE)  */
/* <===============================> */
void Decode_IndAal(PS_IndAal x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->indAal1 = (PS_IndAal1) OSCL_DEFAULT_MALLOC(sizeof(S_IndAal1));
            Decode_IndAal1(x->indAal1, stream);
            break;
        case 1:
            x->indAal5 = (PS_IndAal5) OSCL_DEFAULT_MALLOC(sizeof(S_IndAal5));
            Decode_IndAal5(x->indAal5, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IndAal: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <==================================> */
/*  PER-Decoder for IndAal5 (SEQUENCE)  */
/* <==================================> */
void Decode_IndAal5(PS_IndAal5 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->forwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    x->backwardMaximumSDUSize = (uint16) GetInteger(0, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IndAal5: Unknown extensions (skipped)");
        }
    }
}

/* <==================================> */
/*  PER-Decoder for IndAal1 (SEQUENCE)  */
/* <==================================> */
void Decode_IndAal1(PS_IndAal1 x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_IndClockRecovery(&x->indClockRecovery, stream);
    Decode_IndErrorCorrection(&x->indErrorCorrection, stream);
    x->structuredDataTransfer = GetBoolean(stream);
    x->partiallyFilledCells = GetBoolean(stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_IndAal1: Unknown extensions (skipped)");
        }
    }
}

/* <===========================================> */
/*  PER-Decoder for IndErrorCorrection (CHOICE)  */
/* <===========================================> */
void Decode_IndErrorCorrection(PS_IndErrorCorrection x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (nullErrorCorrection is NULL) */
            break;
        case 1:
            /* (longInterleaver is NULL) */
            break;
        case 2:
            /* (shortInterleaver is NULL) */
            break;
        case 3:
            /* (errorCorrectionOnly is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IndErrorCorrection: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=========================================> */
/*  PER-Decoder for IndClockRecovery (CHOICE)  */
/* <=========================================> */
void Decode_IndClockRecovery(PS_IndClockRecovery x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 1, stream);
    switch (x->index)
    {
        case 0:
            /* (nullClockRecovery is NULL) */
            break;
        case 1:
            /* (srtsClockRecovery is NULL) */
            break;
        case 2:
            /* (adaptiveClockRecovery is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_IndClockRecovery: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <============================================> */
/*  PER-Decoder for UserInputIndication (CHOICE)  */
/* <============================================> */
void Decode_UserInputIndication(PS_UserInputIndication x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            x->alphanumeric = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
            GetCharString("GeneralString", 1, 0, 0, NULL, x->alphanumeric, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            GetLengthDet(stream);
            x->userInputSupportIndication = (PS_UserInputSupportIndication) OSCL_DEFAULT_MALLOC(sizeof(S_UserInputSupportIndication));
            Decode_UserInputSupportIndication(x->userInputSupportIndication, stream);
            ReadRemainingBits(stream);
            break;
        case 3:
            GetLengthDet(stream);
            x->signal = (PS_Signal) OSCL_DEFAULT_MALLOC(sizeof(S_Signal));
            Decode_Signal(x->signal, stream);
            ReadRemainingBits(stream);
            break;
        case 4:
            GetLengthDet(stream);
            x->signalUpdate = (PS_SignalUpdate) OSCL_DEFAULT_MALLOC(sizeof(S_SignalUpdate));
            Decode_SignalUpdate(x->signalUpdate, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_UserInputIndication: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <=======================================> */
/*  PER-Decoder for SignalUpdate (SEQUENCE)  */
/* <=======================================> */
void Decode_SignalUpdate(PS_SignalUpdate x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_rtp = GetBoolean(stream);
    x->duration = (uint16) GetInteger(1, 65535, stream);
    if (x->option_of_rtp)
    {
        Decode_Rtp(&x->rtp, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_SignalUpdate: Unknown extensions (skipped)");
        }
    }
}

/* <==============================> */
/*  PER-Decoder for Rtp (SEQUENCE)  */
/* <==============================> */
void Decode_Rtp(PS_Rtp x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Rtp: Unknown extensions (skipped)");
        }
    }
}

/* <=================================> */
/*  PER-Decoder for Signal (SEQUENCE)  */
/* <=================================> */
void Decode_Signal(PS_Signal x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_duration = GetBoolean(stream);
    x->option_of_signalRtp = GetBoolean(stream);
    GetCharString("IA5String", 0, 1, 1, "0123456789#*ABCD!", &x->signalType, stream);
    if (x->option_of_duration)
    {
        x->duration = (uint16) GetInteger(1, 65535, stream);
    }
    if (x->option_of_signalRtp)
    {
        Decode_SignalRtp(&x->signalRtp, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_Signal: Unknown extensions (skipped)");
        }
    }
}

/* <====================================> */
/*  PER-Decoder for SignalRtp (SEQUENCE)  */
/* <====================================> */
void Decode_SignalRtp(PS_SignalRtp x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    x->option_of_timestamp = GetBoolean(stream);
    x->option_of_expirationTime = GetBoolean(stream);
    if (x->option_of_timestamp)
    {
        x->timestamp = GetInteger(0, 0xffffffff, stream);
    }
    if (x->option_of_expirationTime)
    {
        x->expirationTime = GetInteger(0, 0xffffffff, stream);
    }
    x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_SignalRtp: Unknown extensions (skipped)");
        }
    }
}

/* <===================================================> */
/*  PER-Decoder for UserInputSupportIndication (CHOICE)  */
/* <===================================================> */
void Decode_UserInputSupportIndication(PS_UserInputSupportIndication x, PS_InStream stream)
{
    x->index = GetChoiceIndex(4, 1, stream);
    switch (x->index)
    {
        case 0:
            x->nonStandard = (PS_NonStandardParameter) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardParameter));
            Decode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (basicString is NULL) */
            break;
        case 2:
            /* (iA5String is NULL) */
            break;
        case 3:
            /* (generalString is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Decode_UserInputSupportIndication: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

/* <================================================> */
/*  PER-Decoder for FlowControlIndication (SEQUENCE)  */
/* <================================================> */
void Decode_FlowControlIndication(PS_FlowControlIndication x, PS_InStream stream)
{
    uint8 extension;
    PS_UnknownSigMap map;

    extension = GetBoolean(stream);
    Decode_FciScope(&x->fciScope, stream);
    Decode_FciRestriction(&x->fciRestriction, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (extension)
    {
        map = GetUnknownSigMap(stream);
        if (SkipUnreadExtensions(map, stream))
        {
            ErrorMessage("Decode_FlowControlIndication: Unknown extensions (skipped)");
        }
    }
}

/* <=======================================> */
/*  PER-Decoder for FciRestriction (CHOICE)  */
/* <=======================================> */
void Decode_FciRestriction(PS_FciRestriction x, PS_InStream stream)
{
    x->index = GetChoiceIndex(2, 0, stream);
    switch (x->index)
    {
        case 0:
            x->maximumBitRate = GetInteger(0, 16777215, stream);
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_FciRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Decoder for FciScope (CHOICE)  */
/* <=================================> */
void Decode_FciScope(PS_FciScope x, PS_InStream stream)
{
    x->index = GetChoiceIndex(3, 0, stream);
    switch (x->index)
    {
        case 0:
            x->logicalChannelNumber = (uint16) GetInteger(1, 65535, stream);
            break;
        case 1:
            x->resourceID = (uint16) GetInteger(0, 65535, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Decode_FciScope: Illegal CHOICE index");
    }
}

