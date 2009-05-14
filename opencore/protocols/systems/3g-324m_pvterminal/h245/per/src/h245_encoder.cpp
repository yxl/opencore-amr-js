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
// FILE: h245_encoder.c
//
// DESC: PER Encode routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

#include "per_headers.h"
#include "h245def.h"
#include "h245_encoder.h"

/* <=======================================================> */
/*  PER-Encoder for MultimediaSystemControlMessage (CHOICE)  */
/* <=======================================================> */
void Encode_MultimediaSystemControlMessage(PS_MultimediaSystemControlMessage x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_RequestMessage(x->request, stream);
            break;
        case 1:
            Encode_ResponseMessage(x->response, stream);
            break;
        case 2:
            Encode_CommandMessage(x->command, stream);
            break;
        case 3:
            Encode_IndicationMessage(x->indication, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MultimediaSystemControlMessage: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for RequestMessage (CHOICE)  */
/* <=======================================> */
void Encode_RequestMessage(PS_RequestMessage x, PS_OutStream stream)
{
    PutChoiceIndex(11, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_MasterSlaveDetermination(x->masterSlaveDetermination, stream);
            break;
        case 2:
            Encode_TerminalCapabilitySet(x->terminalCapabilitySet, stream);
            break;
        case 3:
            Encode_OpenLogicalChannel(x->openLogicalChannel, stream);
            break;
        case 4:
            Encode_CloseLogicalChannel(x->closeLogicalChannel, stream);
            break;
        case 5:
            Encode_RequestChannelClose(x->requestChannelClose, stream);
            break;
        case 6:
            Encode_MultiplexEntrySend(x->multiplexEntrySend, stream);
            break;
        case 7:
            Encode_RequestMultiplexEntry(x->requestMultiplexEntry, stream);
            break;
        case 8:
            Encode_RequestMode(x->requestMode, stream);
            break;
        case 9:
            Encode_RoundTripDelayRequest(x->roundTripDelayRequest, stream);
            break;
        case 10:
            Encode_MaintenanceLoopRequest(x->maintenanceLoopRequest, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            PutExtensionItem(EPASS Encode_CommunicationModeRequest, (uint8*)x->communicationModeRequest, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_ConferenceRequest, (uint8*)x->conferenceRequest, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_MultilinkRequest, (uint8*)x->multilinkRequest, stream);
            break;
        case 14:
            PutExtensionItem(EPASS Encode_LogicalChannelRateRequest, (uint8*)x->logicalChannelRateRequest, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_RequestMessage: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for ResponseMessage (CHOICE)  */
/* <========================================> */
void Encode_ResponseMessage(PS_ResponseMessage x, PS_OutStream stream)
{
    PutChoiceIndex(19, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_MasterSlaveDeterminationAck(x->masterSlaveDeterminationAck, stream);
            break;
        case 2:
            Encode_MasterSlaveDeterminationReject(x->masterSlaveDeterminationReject, stream);
            break;
        case 3:
            Encode_TerminalCapabilitySetAck(x->terminalCapabilitySetAck, stream);
            break;
        case 4:
            Encode_TerminalCapabilitySetReject(x->terminalCapabilitySetReject, stream);
            break;
        case 5:
            Encode_OpenLogicalChannelAck(x->openLogicalChannelAck, stream);
            break;
        case 6:
            Encode_OpenLogicalChannelReject(x->openLogicalChannelReject, stream);
            break;
        case 7:
            Encode_CloseLogicalChannelAck(x->closeLogicalChannelAck, stream);
            break;
        case 8:
            Encode_RequestChannelCloseAck(x->requestChannelCloseAck, stream);
            break;
        case 9:
            Encode_RequestChannelCloseReject(x->requestChannelCloseReject, stream);
            break;
        case 10:
            Encode_MultiplexEntrySendAck(x->multiplexEntrySendAck, stream);
            break;
        case 11:
            Encode_MultiplexEntrySendReject(x->multiplexEntrySendReject, stream);
            break;
        case 12:
            Encode_RequestMultiplexEntryAck(x->requestMultiplexEntryAck, stream);
            break;
        case 13:
            Encode_RequestMultiplexEntryReject(x->requestMultiplexEntryReject, stream);
            break;
        case 14:
            Encode_RequestModeAck(x->requestModeAck, stream);
            break;
        case 15:
            Encode_RequestModeReject(x->requestModeReject, stream);
            break;
        case 16:
            Encode_RoundTripDelayResponse(x->roundTripDelayResponse, stream);
            break;
        case 17:
            Encode_MaintenanceLoopAck(x->maintenanceLoopAck, stream);
            break;
        case 18:
            Encode_MaintenanceLoopReject(x->maintenanceLoopReject, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            PutExtensionItem(EPASS Encode_CommunicationModeResponse, (uint8*)x->communicationModeResponse, stream);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_ConferenceResponse, (uint8*)x->conferenceResponse, stream);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_MultilinkResponse, (uint8*)x->multilinkResponse, stream);
            break;
        case 22:
            PutExtensionItem(EPASS Encode_LogicalChannelRateAcknowledge, (uint8*)x->logicalChannelRateAcknowledge, stream);
            break;
        case 23:
            PutExtensionItem(EPASS Encode_LogicalChannelRateReject, (uint8*)x->logicalChannelRateReject, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ResponseMessage: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for CommandMessage (CHOICE)  */
/* <=======================================> */
void Encode_CommandMessage(PS_CommandMessage x, PS_OutStream stream)
{
    PutChoiceIndex(7, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_MaintenanceLoopOffCommand(x->maintenanceLoopOffCommand, stream);
            break;
        case 2:
            Encode_SendTerminalCapabilitySet(x->sendTerminalCapabilitySet, stream);
            break;
        case 3:
            Encode_EncryptionCommand(x->encryptionCommand, stream);
            break;
        case 4:
            Encode_FlowControlCommand(x->flowControlCommand, stream);
            break;
        case 5:
            Encode_EndSessionCommand(x->endSessionCommand, stream);
            break;
        case 6:
            Encode_MiscellaneousCommand(x->miscellaneousCommand, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            PutExtensionItem(EPASS Encode_CommunicationModeCommand, (uint8*)x->communicationModeCommand, stream);
            break;
        case 8:
            PutExtensionItem(EPASS Encode_ConferenceCommand, (uint8*)x->conferenceCommand, stream);
            break;
        case 9:
            PutExtensionItem(EPASS Encode_H223MultiplexReconfiguration, (uint8*)x->h223MultiplexReconfiguration, stream);
            break;
        case 10:
            PutExtensionItem(EPASS Encode_NewATMVCCommand, (uint8*)x->newATMVCCommand, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_CommandMessage: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for IndicationMessage (CHOICE)  */
/* <==========================================> */
void Encode_IndicationMessage(PS_IndicationMessage x, PS_OutStream stream)
{
    PutChoiceIndex(14, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_FunctionNotUnderstood(x->functionNotUnderstood, stream);
            break;
        case 2:
            Encode_MasterSlaveDeterminationRelease(x->masterSlaveDeterminationRelease, stream);
            break;
        case 3:
            Encode_TerminalCapabilitySetRelease(x->terminalCapabilitySetRelease, stream);
            break;
        case 4:
            Encode_OpenLogicalChannelConfirm(x->openLogicalChannelConfirm, stream);
            break;
        case 5:
            Encode_RequestChannelCloseRelease(x->requestChannelCloseRelease, stream);
            break;
        case 6:
            Encode_MultiplexEntrySendRelease(x->multiplexEntrySendRelease, stream);
            break;
        case 7:
            Encode_RequestMultiplexEntryRelease(x->requestMultiplexEntryRelease, stream);
            break;
        case 8:
            Encode_RequestModeRelease(x->requestModeRelease, stream);
            break;
        case 9:
            Encode_MiscellaneousIndication(x->miscellaneousIndication, stream);
            break;
        case 10:
            Encode_JitterIndication(x->jitterIndication, stream);
            break;
        case 11:
            Encode_H223SkewIndication(x->h223SkewIndication, stream);
            break;
        case 12:
            Encode_NewATMVCIndication(x->newATMVCIndication, stream);
            break;
        case 13:
            Encode_UserInputIndication(x->userInput, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            PutExtensionItem(EPASS Encode_H2250MaximumSkewIndication, (uint8*)x->h2250MaximumSkewIndication, stream);
            break;
        case 15:
            PutExtensionItem(EPASS Encode_MCLocationIndication, (uint8*)x->mcLocationIndication, stream);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_ConferenceIndication, (uint8*)x->conferenceIndication, stream);
            break;
        case 17:
            PutExtensionItem(EPASS Encode_VendorIdentification, (uint8*)x->vendorIdentification, stream);
            break;
        case 18:
            PutExtensionItem(EPASS Encode_FunctionNotSupported, (uint8*)x->functionNotSupported, stream);
            break;
        case 19:
            PutExtensionItem(EPASS Encode_MultilinkIndication, (uint8*)x->multilinkIndication, stream);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_LogicalChannelRateRelease, (uint8*)x->logicalChannelRateRelease, stream);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_FlowControlIndication, (uint8*)x->flowControlIndication, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_IndicationMessage: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for GenericInformation (SEQUENCE)  */
/* <=============================================> */
void Encode_GenericInformation(PS_GenericInformation x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_subMessageIdentifier, stream);
    PutBoolean(x->option_of_messageContent, stream);
    Encode_CapabilityIdentifier(&x->messageIdentifier, stream);
    if (x->option_of_subMessageIdentifier)
    {
        PutInteger(0, 127, (uint32)x->subMessageIdentifier, stream);
    }
    if (x->option_of_messageContent)
    {
        PutLengthDet(x->size_of_messageContent, stream);
        for (i = 0;i < x->size_of_messageContent;++i)
        {
            Encode_GenericParameter(x->messageContent + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for NonStandardMessage (SEQUENCE)  */
/* <=============================================> */
void Encode_NonStandardMessage(PS_NonStandardMessage x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_NonStandardParameter(&x->nonStandardData, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for NonStandardParameter (SEQUENCE)  */
/* <===============================================> */
void Encode_NonStandardParameter(PS_NonStandardParameter x, PS_OutStream stream)
{
    Encode_NonStandardIdentifier(&x->nonStandardIdentifier, stream);
    PutOctetString(1, 0, 0, &x->data, stream);
}

/* <==============================================> */
/*  PER-Encoder for NonStandardIdentifier (CHOICE)  */
/* <==============================================> */
void Encode_NonStandardIdentifier(PS_NonStandardIdentifier x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutObjectID(x->object, stream);
            break;
        case 1:
            Encode_H221NonStandard(x->h221NonStandard, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_NonStandardIdentifier: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for H221NonStandard (SEQUENCE)  */
/* <==========================================> */
void Encode_H221NonStandard(PS_H221NonStandard x, PS_OutStream stream)
{
    PutInteger(0, 255, (uint32)x->t35CountryCode, stream);
    PutInteger(0, 255, (uint32)x->t35Extension, stream);
    PutInteger(0, 65535, (uint32)x->manufacturerCode, stream);
}

/* <===================================================> */
/*  PER-Encoder for MasterSlaveDetermination (SEQUENCE)  */
/* <===================================================> */
void Encode_MasterSlaveDetermination(PS_MasterSlaveDetermination x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->terminalType, stream);
    PutInteger(0, 16777215, (uint32)x->statusDeterminationNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for MasterSlaveDeterminationAck (SEQUENCE)  */
/* <======================================================> */
void Encode_MasterSlaveDeterminationAck(PS_MasterSlaveDeterminationAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Decision(&x->decision, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for Decision (CHOICE)  */
/* <=================================> */
void Encode_Decision(PS_Decision x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (master is NULL) */
            break;
        case 1:
            /* (slave is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_Decision: Illegal CHOICE index");
    }
}

/* <=========================================================> */
/*  PER-Encoder for MasterSlaveDeterminationReject (SEQUENCE)  */
/* <=========================================================> */
void Encode_MasterSlaveDeterminationReject(PS_MasterSlaveDeterminationReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_MsdRejectCause(&x->msdRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for MsdRejectCause (CHOICE)  */
/* <=======================================> */
void Encode_MsdRejectCause(PS_MsdRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (identicalNumbers is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MsdRejectCause: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Encoder for MasterSlaveDeterminationRelease (SEQUENCE)  */
/* <==========================================================> */
void Encode_MasterSlaveDeterminationRelease(PS_MasterSlaveDeterminationRelease x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for TerminalCapabilitySet (SEQUENCE)  */
/* <================================================> */
void Encode_TerminalCapabilitySet(PS_TerminalCapabilitySet x, PS_OutStream stream)
{
    uint16 i;
    PS_OutStream tempStream;
    uint32 extension;

    extension = x->option_of_genericInformation;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_multiplexCapability, stream);
    PutBoolean(x->option_of_capabilityTable, stream);
    PutBoolean(x->option_of_capabilityDescriptors, stream);
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutObjectID(&x->protocolIdentifier, stream);
    if (x->option_of_multiplexCapability)
    {
        Encode_MultiplexCapability(&x->multiplexCapability, stream);
    }
    if (x->option_of_capabilityTable)
    {
        PutInteger(1, 256, (uint32)x->size_of_capabilityTable, stream);
        for (i = 0;i < x->size_of_capabilityTable;++i)
        {
            Encode_CapabilityTableEntry(x->capabilityTable + i, stream);
        }
    }
    if (x->option_of_capabilityDescriptors)
    {
        PutInteger(1, 256, (uint32)x->size_of_capabilityDescriptors, stream);
        for (i = 0;i < x->size_of_capabilityDescriptors;++i)
        {
            Encode_CapabilityDescriptor(x->capabilityDescriptors + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_genericInformation, stream);
    if (x->option_of_genericInformation)
    {
        tempStream = NewOutStream();  /* Write to temp stream */
        PutLengthDet(x->size_of_genericInformation, tempStream);
        for (i = 0;i < x->size_of_genericInformation;++i)
        {
            Encode_GenericInformation(x->genericInformation + i, tempStream);
        }
        PutTempStream(tempStream, stream); /* Copy to real stream */
    }
}

/* <===============================================> */
/*  PER-Encoder for CapabilityTableEntry (SEQUENCE)  */
/* <===============================================> */
void Encode_CapabilityTableEntry(PS_CapabilityTableEntry x, PS_OutStream stream)
{
    PutBoolean(x->option_of_capability, stream);
    PutInteger(1, 65535, (uint32)x->capabilityTableEntryNumber, stream);
    if (x->option_of_capability)
    {
        Encode_Capability(&x->capability, stream);
    }
}

/* <===============================================> */
/*  PER-Encoder for CapabilityDescriptor (SEQUENCE)  */
/* <===============================================> */
void Encode_CapabilityDescriptor(PS_CapabilityDescriptor x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(x->option_of_simultaneousCapabilities, stream);
    PutInteger(0, 255, (uint32)x->capabilityDescriptorNumber, stream);
    if (x->option_of_simultaneousCapabilities)
    {
        PutInteger(1, 256, (uint32)x->size_of_simultaneousCapabilities, stream);
        for (i = 0;i < x->size_of_simultaneousCapabilities;++i)
        {
            Encode_AlternativeCapabilitySet(x->simultaneousCapabilities + i, stream);
        }
    }
}

/* <======================================================> */
/*  PER-Encoder for AlternativeCapabilitySet (SEQUENCE-OF)  */
/* <======================================================> */
void Encode_AlternativeCapabilitySet(PS_AlternativeCapabilitySet x, PS_OutStream stream)
{
    uint16 i;
    PutInteger(1, 256, (uint32)x->size, stream);
    for (i = 0;i < x->size;++i)
    {
        PutInteger(1, 65535, (uint32)x->item[i], stream);
    }
}

/* <===================================================> */
/*  PER-Encoder for TerminalCapabilitySetAck (SEQUENCE)  */
/* <===================================================> */
void Encode_TerminalCapabilitySetAck(PS_TerminalCapabilitySetAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for TerminalCapabilitySetReject (SEQUENCE)  */
/* <======================================================> */
void Encode_TerminalCapabilitySetReject(PS_TerminalCapabilitySetReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    Encode_TcsRejectCause(&x->tcsRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for TcsRejectCause (CHOICE)  */
/* <=======================================> */
void Encode_TcsRejectCause(PS_TcsRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            Encode_TableEntryCapacityExceeded(x->tableEntryCapacityExceeded, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_TcsRejectCause: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Encoder for TableEntryCapacityExceeded (CHOICE)  */
/* <===================================================> */
void Encode_TableEntryCapacityExceeded(PS_TableEntryCapacityExceeded x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->highestEntryNumberProcessed, stream);
            break;
        case 1:
            /* (noneProcessed is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_TableEntryCapacityExceeded: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Encoder for TerminalCapabilitySetRelease (SEQUENCE)  */
/* <=======================================================> */
void Encode_TerminalCapabilitySetRelease(PS_TerminalCapabilitySetRelease x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Encoder for Capability (CHOICE)  */
/* <===================================> */
void Encode_Capability(PS_Capability x, PS_OutStream stream)
{
    PutChoiceIndex(12, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_VideoCapability(x->receiveVideoCapability, stream);
            break;
        case 2:
            Encode_VideoCapability(x->transmitVideoCapability, stream);
            break;
        case 3:
            Encode_VideoCapability(x->receiveAndTransmitVideoCapability, stream);
            break;
        case 4:
            Encode_AudioCapability(x->receiveAudioCapability, stream);
            break;
        case 5:
            Encode_AudioCapability(x->transmitAudioCapability, stream);
            break;
        case 6:
            Encode_AudioCapability(x->receiveAndTransmitAudioCapability, stream);
            break;
        case 7:
            Encode_DataApplicationCapability(x->receiveDataApplicationCapability, stream);
            break;
        case 8:
            Encode_DataApplicationCapability(x->transmitDataApplicationCapability, stream);
            break;
        case 9:
            Encode_DataApplicationCapability(x->receiveAndTransmitDataApplicationCapability, stream);
            break;
        case 10:
            PutBoolean(x->h233EncryptionTransmitCapability, stream);
            break;
        case 11:
            Encode_H233EncryptionReceiveCapability(x->h233EncryptionReceiveCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 12:
            PutExtensionItem(EPASS Encode_ConferenceCapability, (uint8*)x->conferenceCapability, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_H235SecurityCapability, (uint8*)x->h235SecurityCapability, stream);
            break;
        case 14:
            PutExtensionInteger(0, 255, (uint32)x->maxPendingReplacementFor, stream);
            break;
        case 15:
            PutExtensionItem(EPASS Encode_UserInputCapability, (uint8*)x->receiveUserInputCapability, stream);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_UserInputCapability, (uint8*)x->transmitUserInputCapability, stream);
            break;
        case 17:
            PutExtensionItem(EPASS Encode_UserInputCapability, (uint8*)x->receiveAndTransmitUserInputCapability, stream);
            break;
        case 18:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericControlCapability, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_Capability: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Encoder for H233EncryptionReceiveCapability (SEQUENCE)  */
/* <==========================================================> */
void Encode_H233EncryptionReceiveCapability(PS_H233EncryptionReceiveCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->h233IVResponseTime, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for H235SecurityCapability (SEQUENCE)  */
/* <=================================================> */
void Encode_H235SecurityCapability(PS_H235SecurityCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    PutInteger(1, 65535, (uint32)x->mediaCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for MultiplexCapability (CHOICE)  */
/* <============================================> */
void Encode_MultiplexCapability(PS_MultiplexCapability x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_H222Capability(x->h222Capability, stream);
            break;
        case 2:
            Encode_H223Capability(x->h223Capability, stream);
            break;
        case 3:
            Encode_V76Capability(x->v76Capability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            PutExtensionItem(EPASS Encode_H2250Capability, (uint8*)x->h2250Capability, stream);
            break;
        case 5:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericMultiplexCapability, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MultiplexCapability: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for H222Capability (SEQUENCE)  */
/* <=========================================> */
void Encode_H222Capability(PS_H222Capability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->numberOfVCs, stream);
    PutLengthDet(x->size_of_vcCapability, stream);
    for (i = 0;i < x->size_of_vcCapability;++i)
    {
        Encode_VCCapability(x->vcCapability + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for VCCapability (SEQUENCE)  */
/* <=======================================> */
void Encode_VCCapability(PS_VCCapability x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_aal1ViaGateway;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_vccAal1, stream);
    PutBoolean(x->option_of_vccAal5, stream);
    if (x->option_of_vccAal1)
    {
        Encode_VccAal1(&x->vccAal1, stream);
    }
    if (x->option_of_vccAal5)
    {
        Encode_VccAal5(&x->vccAal5, stream);
    }
    PutBoolean(x->transportStream, stream);
    PutBoolean(x->programStream, stream);
    Encode_AvailableBitRates(&x->availableBitRates, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_aal1ViaGateway, stream);
    if (x->option_of_aal1ViaGateway)
    {
        PutExtensionItem(EPASS Encode_Aal1ViaGateway, (uint8*)&x->aal1ViaGateway, stream);
    }
}

/* <=========================================> */
/*  PER-Encoder for Aal1ViaGateway (SEQUENCE)  */
/* <=========================================> */
void Encode_Aal1ViaGateway(PS_Aal1ViaGateway x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->size_of_gatewayAddress, stream);
    for (i = 0;i < x->size_of_gatewayAddress;++i)
    {
        Encode_Q2931Address(x->gatewayAddress + i, stream);
    }
    PutBoolean(x->nullClockRecovery, stream);
    PutBoolean(x->srtsClockRecovery, stream);
    PutBoolean(x->adaptiveClockRecovery, stream);
    PutBoolean(x->nullErrorCorrection, stream);
    PutBoolean(x->longInterleaver, stream);
    PutBoolean(x->shortInterleaver, stream);
    PutBoolean(x->errorCorrectionOnly, stream);
    PutBoolean(x->structuredDataTransfer, stream);
    PutBoolean(x->partiallyFilledCells, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for AvailableBitRates (SEQUENCE)  */
/* <============================================> */
void Encode_AvailableBitRates(PS_AvailableBitRates x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_VccAal5Type(&x->vccAal5Type, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Encoder for VccAal5Type (CHOICE)  */
/* <====================================> */
void Encode_VccAal5Type(PS_VccAal5Type x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->singleBitRate, stream);
            break;
        case 1:
            Encode_RangeOfBitRates(x->rangeOfBitRates, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_VccAal5Type: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for RangeOfBitRates (SEQUENCE)  */
/* <==========================================> */
void Encode_RangeOfBitRates(PS_RangeOfBitRates x, PS_OutStream stream)
{
    PutInteger(1, 65535, (uint32)x->lowerBitRate, stream);
    PutInteger(1, 65535, (uint32)x->higherBitRate, stream);
}

/* <==================================> */
/*  PER-Encoder for VccAal5 (SEQUENCE)  */
/* <==================================> */
void Encode_VccAal5(PS_VccAal5 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->forwardMaximumSDUSize, stream);
    PutInteger(0, 65535, (uint32)x->backwardMaximumSDUSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for VccAal1 (SEQUENCE)  */
/* <==================================> */
void Encode_VccAal1(PS_VccAal1 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->nullClockRecovery, stream);
    PutBoolean(x->srtsClockRecovery, stream);
    PutBoolean(x->adaptiveClockRecovery, stream);
    PutBoolean(x->nullErrorCorrection, stream);
    PutBoolean(x->longInterleaver, stream);
    PutBoolean(x->shortInterleaver, stream);
    PutBoolean(x->errorCorrectionOnly, stream);
    PutBoolean(x->structuredDataTransfer, stream);
    PutBoolean(x->partiallyFilledCells, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for H223Capability (SEQUENCE)  */
/* <=========================================> */
void Encode_H223Capability(PS_H223Capability x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_maxMUXPDUSizeCapability |
                x->option_of_nsrpSupport |
                x->option_of_mobileOperationTransmitCapability |
                x->option_of_h223AnnexCCapability;
    PutBoolean(extension, stream);

    PutBoolean(x->transportWithI_frames, stream);
    PutBoolean(x->videoWithAL1, stream);
    PutBoolean(x->videoWithAL2, stream);
    PutBoolean(x->videoWithAL3, stream);
    PutBoolean(x->audioWithAL1, stream);
    PutBoolean(x->audioWithAL2, stream);
    PutBoolean(x->audioWithAL3, stream);
    PutBoolean(x->dataWithAL1, stream);
    PutBoolean(x->dataWithAL2, stream);
    PutBoolean(x->dataWithAL3, stream);
    PutInteger(0, 65535, (uint32)x->maximumAl2SDUSize, stream);
    PutInteger(0, 65535, (uint32)x->maximumAl3SDUSize, stream);
    PutInteger(0, 1023, (uint32)x->maximumDelayJitter, stream);
    Encode_H223MultiplexTableCapability(&x->h223MultiplexTableCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(4, stream); /* Begin Options Map */
    PutBoolean(x->option_of_maxMUXPDUSizeCapability, stream);
    PutBoolean(x->option_of_nsrpSupport, stream);
    PutBoolean(x->option_of_mobileOperationTransmitCapability, stream);
    PutBoolean(x->option_of_h223AnnexCCapability, stream);
    if (x->option_of_maxMUXPDUSizeCapability)
    {
        PutExtensionBoolean(x->maxMUXPDUSizeCapability, stream);
    }
    if (x->option_of_nsrpSupport)
    {
        PutExtensionBoolean(x->nsrpSupport, stream);
    }
    if (x->option_of_mobileOperationTransmitCapability)
    {
        PutExtensionItem(EPASS Encode_MobileOperationTransmitCapability, (uint8*)&x->mobileOperationTransmitCapability, stream);
    }
    if (x->option_of_h223AnnexCCapability)
    {
        PutExtensionItem(EPASS Encode_H223AnnexCCapability, (uint8*)&x->h223AnnexCCapability, stream);
    }
}

/* <============================================================> */
/*  PER-Encoder for MobileOperationTransmitCapability (SEQUENCE)  */
/* <============================================================> */
void Encode_MobileOperationTransmitCapability(PS_MobileOperationTransmitCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->modeChangeCapability, stream);
    PutBoolean(x->h223AnnexA, stream);
    PutBoolean(x->h223AnnexADoubleFlag, stream);
    PutBoolean(x->h223AnnexB, stream);
    PutBoolean(x->h223AnnexBwithHeader, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Encoder for H223MultiplexTableCapability (CHOICE)  */
/* <=====================================================> */
void Encode_H223MultiplexTableCapability(PS_H223MultiplexTableCapability x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (basic is NULL) */
            break;
        case 1:
            Encode_Enhanced(x->enhanced, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_H223MultiplexTableCapability: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Encoder for Enhanced (SEQUENCE)  */
/* <===================================> */
void Encode_Enhanced(PS_Enhanced x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->maximumNestingDepth, stream);
    PutInteger(2, 255, (uint32)x->maximumElementListSize, stream);
    PutInteger(2, 255, (uint32)x->maximumSubElementListSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for H223AnnexCCapability (SEQUENCE)  */
/* <===============================================> */
void Encode_H223AnnexCCapability(PS_H223AnnexCCapability x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_rsCodeCapability;
    PutBoolean(extension, stream);

    PutBoolean(x->videoWithAL1M, stream);
    PutBoolean(x->videoWithAL2M, stream);
    PutBoolean(x->videoWithAL3M, stream);
    PutBoolean(x->audioWithAL1M, stream);
    PutBoolean(x->audioWithAL2M, stream);
    PutBoolean(x->audioWithAL3M, stream);
    PutBoolean(x->dataWithAL1M, stream);
    PutBoolean(x->dataWithAL2M, stream);
    PutBoolean(x->dataWithAL3M, stream);
    PutBoolean(x->alpduInterleaving, stream);
    PutInteger(0, 65535, (uint32)x->maximumAL1MPDUSize, stream);
    PutInteger(0, 65535, (uint32)x->maximumAL2MSDUSize, stream);
    PutInteger(0, 65535, (uint32)x->maximumAL3MSDUSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_rsCodeCapability, stream);
    if (x->option_of_rsCodeCapability)
    {
        PutExtensionBoolean(x->rsCodeCapability, stream);
    }
}

/* <========================================> */
/*  PER-Encoder for V76Capability (SEQUENCE)  */
/* <========================================> */
void Encode_V76Capability(PS_V76Capability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->suspendResumeCapabilitywAddress, stream);
    PutBoolean(x->suspendResumeCapabilitywoAddress, stream);
    PutBoolean(x->rejCapability, stream);
    PutBoolean(x->sREJCapability, stream);
    PutBoolean(x->mREJCapability, stream);
    PutBoolean(x->crc8bitCapability, stream);
    PutBoolean(x->crc16bitCapability, stream);
    PutBoolean(x->crc32bitCapability, stream);
    PutBoolean(x->uihCapability, stream);
    PutInteger(2, 8191, (uint32)x->numOfDLCS, stream);
    PutBoolean(x->twoOctetAddressFieldCapability, stream);
    PutBoolean(x->loopBackTestCapability, stream);
    PutInteger(1, 4095, (uint32)x->n401Capability, stream);
    PutInteger(1, 127, (uint32)x->maxWindowSizeCapability, stream);
    Encode_V75Capability(&x->v75Capability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for V75Capability (SEQUENCE)  */
/* <========================================> */
void Encode_V75Capability(PS_V75Capability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->audioHeader, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for H2250Capability (SEQUENCE)  */
/* <==========================================> */
void Encode_H2250Capability(PS_H2250Capability x, PS_OutStream stream)
{
    uint16 i;
    PS_OutStream tempStream;
    uint32 extension;

    extension = x->option_of_transportCapability |
                x->option_of_redundancyEncodingCapability |
                x->option_of_logicalChannelSwitchingCapability |
                x->option_of_t120DynamicPortCapability;
    PutBoolean(extension, stream);

    PutInteger(0, 1023, (uint32)x->maximumAudioDelayJitter, stream);
    Encode_MultipointCapability(&x->receiveMultipointCapability, stream);
    Encode_MultipointCapability(&x->transmitMultipointCapability, stream);
    Encode_MultipointCapability(&x->receiveAndTransmitMultipointCapability, stream);
    Encode_McCapability(&x->mcCapability, stream);
    PutBoolean(x->rtcpVideoControlCapability, stream);
    Encode_MediaPacketizationCapability(&x->mediaPacketizationCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(4, stream); /* Begin Options Map */
    PutBoolean(x->option_of_transportCapability, stream);
    PutBoolean(x->option_of_redundancyEncodingCapability, stream);
    PutBoolean(x->option_of_logicalChannelSwitchingCapability, stream);
    PutBoolean(x->option_of_t120DynamicPortCapability, stream);
    if (x->option_of_transportCapability)
    {
        PutExtensionItem(EPASS Encode_TransportCapability, (uint8*)&x->transportCapability, stream);
    }
    if (x->option_of_redundancyEncodingCapability)
    {
        tempStream = NewOutStream();  /* Write to temp stream */
        PutInteger(1, 256, (uint32)x->size_of_redundancyEncodingCapability, tempStream);
        for (i = 0;i < x->size_of_redundancyEncodingCapability;++i)
        {
            Encode_RedundancyEncodingCapability(x->redundancyEncodingCapability + i, tempStream);
        }
        PutTempStream(tempStream, stream); /* Copy to real stream */
    }
    if (x->option_of_logicalChannelSwitchingCapability)
    {
        PutExtensionBoolean(x->logicalChannelSwitchingCapability, stream);
    }
    if (x->option_of_t120DynamicPortCapability)
    {
        PutExtensionBoolean(x->t120DynamicPortCapability, stream);
    }
}

/* <=======================================> */
/*  PER-Encoder for McCapability (SEQUENCE)  */
/* <=======================================> */
void Encode_McCapability(PS_McCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->centralizedConferenceMC, stream);
    PutBoolean(x->decentralizedConferenceMC, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Encoder for MediaPacketizationCapability (SEQUENCE)  */
/* <=======================================================> */
void Encode_MediaPacketizationCapability(PS_MediaPacketizationCapability x, PS_OutStream stream)
{
    uint16 i;
    PS_OutStream tempStream;
    uint32 extension;

    extension = x->option_of_rtpPayloadType;
    PutBoolean(extension, stream);

    PutBoolean(x->h261aVideoPacketization, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_rtpPayloadType, stream);
    if (x->option_of_rtpPayloadType)
    {
        tempStream = NewOutStream();  /* Write to temp stream */
        PutInteger(1, 256, (uint32)x->size_of_rtpPayloadType, tempStream);
        for (i = 0;i < x->size_of_rtpPayloadType;++i)
        {
            Encode_RTPPayloadType(x->rtpPayloadType + i, tempStream);
        }
        PutTempStream(tempStream, stream); /* Copy to real stream */
    }
}

/* <=========================================> */
/*  PER-Encoder for RSVPParameters (SEQUENCE)  */
/* <=========================================> */
void Encode_RSVPParameters(PS_RSVPParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_qosMode, stream);
    PutBoolean(x->option_of_tokenRate, stream);
    PutBoolean(x->option_of_bucketSize, stream);
    PutBoolean(x->option_of_peakRate, stream);
    PutBoolean(x->option_of_minPoliced, stream);
    PutBoolean(x->option_of_maxPktSize, stream);
    if (x->option_of_qosMode)
    {
        Encode_QOSMode(&x->qosMode, stream);
    }
    if (x->option_of_tokenRate)
    {
        PutInteger(1, 0xffffffff, (uint32)x->tokenRate, stream);
    }
    if (x->option_of_bucketSize)
    {
        PutInteger(1, 0xffffffff, (uint32)x->bucketSize, stream);
    }
    if (x->option_of_peakRate)
    {
        PutInteger(1, 0xffffffff, (uint32)x->peakRate, stream);
    }
    if (x->option_of_minPoliced)
    {
        PutInteger(1, 0xffffffff, (uint32)x->minPoliced, stream);
    }
    if (x->option_of_maxPktSize)
    {
        PutInteger(1, 0xffffffff, (uint32)x->maxPktSize, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Encoder for QOSMode (CHOICE)  */
/* <================================> */
void Encode_QOSMode(PS_QOSMode x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_QOSMode: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for ATMParameters (SEQUENCE)  */
/* <========================================> */
void Encode_ATMParameters(PS_ATMParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->maxNTUSize, stream);
    PutBoolean(x->atmUBR, stream);
    PutBoolean(x->atmrtVBR, stream);
    PutBoolean(x->atmnrtVBR, stream);
    PutBoolean(x->atmABR, stream);
    PutBoolean(x->atmCBR, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for QOSCapability (SEQUENCE)  */
/* <========================================> */
void Encode_QOSCapability(PS_QOSCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_nonStandardData, stream);
    PutBoolean(x->option_of_rsvpParameters, stream);
    PutBoolean(x->option_of_atmParameters, stream);
    if (x->option_of_nonStandardData)
    {
        Encode_NonStandardParameter(&x->nonStandardData, stream);
    }
    if (x->option_of_rsvpParameters)
    {
        Encode_RSVPParameters(&x->rsvpParameters, stream);
    }
    if (x->option_of_atmParameters)
    {
        Encode_ATMParameters(&x->atmParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for MediaTransportType (CHOICE)  */
/* <===========================================> */
void Encode_MediaTransportType(PS_MediaTransportType x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            PutExtensionItem(EPASS Encode_Atm_AAL5_compressed, (uint8*)x->atm_AAL5_compressed, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MediaTransportType: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Encoder for Atm_AAL5_compressed (SEQUENCE)  */
/* <==============================================> */
void Encode_Atm_AAL5_compressed(PS_Atm_AAL5_compressed x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->variable_delta, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for MediaChannelCapability (SEQUENCE)  */
/* <=================================================> */
void Encode_MediaChannelCapability(PS_MediaChannelCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_mediaTransport, stream);
    if (x->option_of_mediaTransport)
    {
        Encode_MediaTransportType(&x->mediaTransport, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for TransportCapability (SEQUENCE)  */
/* <==============================================> */
void Encode_TransportCapability(PS_TransportCapability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_nonStandard, stream);
    PutBoolean(x->option_of_qOSCapabilities, stream);
    PutBoolean(x->option_of_mediaChannelCapabilities, stream);
    if (x->option_of_nonStandard)
    {
        Encode_NonStandardParameter(&x->nonStandard, stream);
    }
    if (x->option_of_qOSCapabilities)
    {
        PutInteger(1, 256, (uint32)x->size_of_qOSCapabilities, stream);
        for (i = 0;i < x->size_of_qOSCapabilities;++i)
        {
            Encode_QOSCapability(x->qOSCapabilities + i, stream);
        }
    }
    if (x->option_of_mediaChannelCapabilities)
    {
        PutInteger(1, 256, (uint32)x->size_of_mediaChannelCapabilities, stream);
        for (i = 0;i < x->size_of_mediaChannelCapabilities;++i)
        {
            Encode_MediaChannelCapability(x->mediaChannelCapabilities + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Encoder for RedundancyEncodingCapability (SEQUENCE)  */
/* <=======================================================> */
void Encode_RedundancyEncodingCapability(PS_RedundancyEncodingCapability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_secondaryEncoding, stream);
    Encode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    PutInteger(1, 65535, (uint32)x->primaryEncoding, stream);
    if (x->option_of_secondaryEncoding)
    {
        PutInteger(1, 256, (uint32)x->size_of_secondaryEncoding, stream);
        for (i = 0;i < x->size_of_secondaryEncoding;++i)
        {
            PutInteger(1, 65535, (uint32)x->secondaryEncoding[i], stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for RedundancyEncodingMethod (CHOICE)  */
/* <=================================================> */
void Encode_RedundancyEncodingMethod(PS_RedundancyEncodingMethod x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (rtpAudioRedundancyEncoding is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            PutExtensionItem(EPASS Encode_RTPH263VideoRedundancyEncoding, (uint8*)x->rtpH263VideoRedundancyEncoding, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_RedundancyEncodingMethod: Illegal CHOICE index");
    }
}

/* <=========================================================> */
/*  PER-Encoder for RTPH263VideoRedundancyEncoding (SEQUENCE)  */
/* <=========================================================> */
void Encode_RTPH263VideoRedundancyEncoding(PS_RTPH263VideoRedundancyEncoding x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_containedThreads, stream);
    PutInteger(1, 16, (uint32)x->numberOfThreads, stream);
    PutInteger(1, 256, (uint32)x->framesBetweenSyncPoints, stream);
    Encode_FrameToThreadMapping(&x->frameToThreadMapping, stream);
    if (x->option_of_containedThreads)
    {
        PutInteger(1, 256, (uint32)x->size_of_containedThreads, stream);
        for (i = 0;i < x->size_of_containedThreads;++i)
        {
            PutInteger(0, 15, (uint32)x->containedThreads[i], stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for FrameToThreadMapping (CHOICE)  */
/* <=============================================> */
void Encode_FrameToThreadMapping(PS_FrameToThreadMapping x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (roundrobin is NULL) */
            break;
        case 1:
            PutInteger(1, 256, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_RTPH263VideoRedundancyFrameMapping(x->custom + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_FrameToThreadMapping: Illegal CHOICE index");
    }
}

/* <=============================================================> */
/*  PER-Encoder for RTPH263VideoRedundancyFrameMapping (SEQUENCE)  */
/* <=============================================================> */
void Encode_RTPH263VideoRedundancyFrameMapping(PS_RTPH263VideoRedundancyFrameMapping x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 15, (uint32)x->threadNumber, stream);
    PutInteger(1, 256, (uint32)x->size_of_frameSequence, stream);
    for (i = 0;i < x->size_of_frameSequence;++i)
    {
        PutInteger(0, 255, (uint32)x->frameSequence[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for MultipointCapability (SEQUENCE)  */
/* <===============================================> */
void Encode_MultipointCapability(PS_MultipointCapability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->multicastCapability, stream);
    PutBoolean(x->multiUniCastConference, stream);
    PutLengthDet(x->size_of_mediaDistributionCapability, stream);
    for (i = 0;i < x->size_of_mediaDistributionCapability;++i)
    {
        Encode_MediaDistributionCapability(x->mediaDistributionCapability + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for MediaDistributionCapability (SEQUENCE)  */
/* <======================================================> */
void Encode_MediaDistributionCapability(PS_MediaDistributionCapability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_centralizedData, stream);
    PutBoolean(x->option_of_distributedData, stream);
    PutBoolean(x->centralizedControl, stream);
    PutBoolean(x->distributedControl, stream);
    PutBoolean(x->centralizedAudio, stream);
    PutBoolean(x->distributedAudio, stream);
    PutBoolean(x->centralizedVideo, stream);
    PutBoolean(x->distributedVideo, stream);
    if (x->option_of_centralizedData)
    {
        PutLengthDet(x->size_of_centralizedData, stream);
        for (i = 0;i < x->size_of_centralizedData;++i)
        {
            Encode_DataApplicationCapability(x->centralizedData + i, stream);
        }
    }
    if (x->option_of_distributedData)
    {
        PutLengthDet(x->size_of_distributedData, stream);
        for (i = 0;i < x->size_of_distributedData;++i)
        {
            Encode_DataApplicationCapability(x->distributedData + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for VideoCapability (CHOICE)  */
/* <========================================> */
void Encode_VideoCapability(PS_VideoCapability x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_H261VideoCapability(x->h261VideoCapability, stream);
            break;
        case 2:
            Encode_H262VideoCapability(x->h262VideoCapability, stream);
            break;
        case 3:
            Encode_H263VideoCapability(x->h263VideoCapability, stream);
            break;
        case 4:
            Encode_IS11172VideoCapability(x->is11172VideoCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericVideoCapability, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_VideoCapability: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Encoder for H261VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Encode_H261VideoCapability(PS_H261VideoCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_qcifMPI, stream);
    PutBoolean(x->option_of_cifMPI, stream);
    if (x->option_of_qcifMPI)
    {
        PutInteger(1, 4, (uint32)x->qcifMPI, stream);
    }
    if (x->option_of_cifMPI)
    {
        PutInteger(1, 4, (uint32)x->cifMPI, stream);
    }
    PutBoolean(x->temporalSpatialTradeOffCapability, stream);
    PutInteger(1, 19200, (uint32)x->maxBitRate, stream);
    PutBoolean(x->stillImageTransmission, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for H262VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Encode_H262VideoCapability(PS_H262VideoCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_videoBitRate, stream);
    PutBoolean(x->option_of_vbvBufferSize, stream);
    PutBoolean(x->option_of_samplesPerLine, stream);
    PutBoolean(x->option_of_linesPerFrame, stream);
    PutBoolean(x->option_of_framesPerSecond, stream);
    PutBoolean(x->option_of_luminanceSampleRate, stream);
    PutBoolean(x->profileAndLevel_SPatML, stream);
    PutBoolean(x->profileAndLevel_MPatLL, stream);
    PutBoolean(x->profileAndLevel_MPatML, stream);
    PutBoolean(x->profileAndLevel_MPatH_14, stream);
    PutBoolean(x->profileAndLevel_MPatHL, stream);
    PutBoolean(x->profileAndLevel_SNRatLL, stream);
    PutBoolean(x->profileAndLevel_SNRatML, stream);
    PutBoolean(x->profileAndLevel_SpatialatH_14, stream);
    PutBoolean(x->profileAndLevel_HPatML, stream);
    PutBoolean(x->profileAndLevel_HPatH_14, stream);
    PutBoolean(x->profileAndLevel_HPatHL, stream);
    if (x->option_of_videoBitRate)
    {
        PutInteger(0, 1073741823, (uint32)x->videoBitRate, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        PutInteger(0, 262143, (uint32)x->vbvBufferSize, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        PutInteger(0, 16383, (uint32)x->samplesPerLine, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        PutInteger(0, 16383, (uint32)x->linesPerFrame, stream);
    }
    if (x->option_of_framesPerSecond)
    {
        PutInteger(0, 15, (uint32)x->framesPerSecond, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->luminanceSampleRate, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for H263VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Encode_H263VideoCapability(PS_H263VideoCapability x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_slowSqcifMPI |
                x->option_of_slowQcifMPI |
                x->option_of_slowCifMPI |
                x->option_of_slowCif4MPI |
                x->option_of_slowCif16MPI |
                x->option_of_errorCompensation |
                x->option_of_enhancementLayerInfo |
                x->option_of_h263Options;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_sqcifMPI, stream);
    PutBoolean(x->option_of_qcifMPI, stream);
    PutBoolean(x->option_of_cifMPI, stream);
    PutBoolean(x->option_of_cif4MPI, stream);
    PutBoolean(x->option_of_cif16MPI, stream);
    PutBoolean(x->option_of_hrd_B, stream);
    PutBoolean(x->option_of_bppMaxKb, stream);
    if (x->option_of_sqcifMPI)
    {
        PutInteger(1, 32, (uint32)x->sqcifMPI, stream);
    }
    if (x->option_of_qcifMPI)
    {
        PutInteger(1, 32, (uint32)x->qcifMPI, stream);
    }
    if (x->option_of_cifMPI)
    {
        PutInteger(1, 32, (uint32)x->cifMPI, stream);
    }
    if (x->option_of_cif4MPI)
    {
        PutInteger(1, 32, (uint32)x->cif4MPI, stream);
    }
    if (x->option_of_cif16MPI)
    {
        PutInteger(1, 32, (uint32)x->cif16MPI, stream);
    }
    PutInteger(1, 192400, (uint32)x->maxBitRate, stream);
    PutBoolean(x->unrestrictedVector, stream);
    PutBoolean(x->arithmeticCoding, stream);
    PutBoolean(x->advancedPrediction, stream);
    PutBoolean(x->pbFrames, stream);
    PutBoolean(x->temporalSpatialTradeOffCapability, stream);
    if (x->option_of_hrd_B)
    {
        PutInteger(0, 524287, (uint32)x->hrd_B, stream);
    }
    if (x->option_of_bppMaxKb)
    {
        PutInteger(0, 65535, (uint32)x->bppMaxKb, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(8, stream); /* Begin Options Map */
    PutBoolean(x->option_of_slowSqcifMPI, stream);
    PutBoolean(x->option_of_slowQcifMPI, stream);
    PutBoolean(x->option_of_slowCifMPI, stream);
    PutBoolean(x->option_of_slowCif4MPI, stream);
    PutBoolean(x->option_of_slowCif16MPI, stream);
    PutBoolean(x->option_of_errorCompensation, stream);
    PutBoolean(x->option_of_enhancementLayerInfo, stream);
    PutBoolean(x->option_of_h263Options, stream);
    if (x->option_of_slowSqcifMPI)
    {
        PutExtensionInteger(1, 3600, (uint32)x->slowSqcifMPI, stream);
    }
    if (x->option_of_slowQcifMPI)
    {
        PutExtensionInteger(1, 3600, (uint32)x->slowQcifMPI, stream);
    }
    if (x->option_of_slowCifMPI)
    {
        PutExtensionInteger(1, 3600, (uint32)x->slowCifMPI, stream);
    }
    if (x->option_of_slowCif4MPI)
    {
        PutExtensionInteger(1, 3600, (uint32)x->slowCif4MPI, stream);
    }
    if (x->option_of_slowCif16MPI)
    {
        PutExtensionInteger(1, 3600, (uint32)x->slowCif16MPI, stream);
    }
    if (x->option_of_errorCompensation)
    {
        PutExtensionBoolean(x->errorCompensation, stream);
    }
    if (x->option_of_enhancementLayerInfo)
    {
        PutExtensionItem(EPASS Encode_EnhancementLayerInfo, (uint8*)&x->enhancementLayerInfo, stream);
    }
    if (x->option_of_h263Options)
    {
        PutExtensionItem(EPASS Encode_H263Options, (uint8*)&x->h263Options, stream);
    }
}

/* <===============================================> */
/*  PER-Encoder for EnhancementLayerInfo (SEQUENCE)  */
/* <===============================================> */
void Encode_EnhancementLayerInfo(PS_EnhancementLayerInfo x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_snrEnhancement, stream);
    PutBoolean(x->option_of_spatialEnhancement, stream);
    PutBoolean(x->option_of_bPictureEnhancement, stream);
    PutBoolean(x->baseBitRateConstrained, stream);
    if (x->option_of_snrEnhancement)
    {
        PutInteger(1, 14, (uint32)x->size_of_snrEnhancement, stream);
        for (i = 0;i < x->size_of_snrEnhancement;++i)
        {
            Encode_EnhancementOptions(x->snrEnhancement + i, stream);
        }
    }
    if (x->option_of_spatialEnhancement)
    {
        PutInteger(1, 14, (uint32)x->size_of_spatialEnhancement, stream);
        for (i = 0;i < x->size_of_spatialEnhancement;++i)
        {
            Encode_EnhancementOptions(x->spatialEnhancement + i, stream);
        }
    }
    if (x->option_of_bPictureEnhancement)
    {
        PutInteger(1, 14, (uint32)x->size_of_bPictureEnhancement, stream);
        for (i = 0;i < x->size_of_bPictureEnhancement;++i)
        {
            Encode_BEnhancementParameters(x->bPictureEnhancement + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for BEnhancementParameters (SEQUENCE)  */
/* <=================================================> */
void Encode_BEnhancementParameters(PS_BEnhancementParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_EnhancementOptions(&x->enhancementOptions, stream);
    PutInteger(1, 64, (uint32)x->numberOfBPictures, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for EnhancementOptions (SEQUENCE)  */
/* <=============================================> */
void Encode_EnhancementOptions(PS_EnhancementOptions x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_sqcifMPI, stream);
    PutBoolean(x->option_of_qcifMPI, stream);
    PutBoolean(x->option_of_cifMPI, stream);
    PutBoolean(x->option_of_cif4MPI, stream);
    PutBoolean(x->option_of_cif16MPI, stream);
    PutBoolean(x->option_of_slowSqcifMPI, stream);
    PutBoolean(x->option_of_slowQcifMPI, stream);
    PutBoolean(x->option_of_slowCifMPI, stream);
    PutBoolean(x->option_of_slowCif4MPI, stream);
    PutBoolean(x->option_of_slowCif16MPI, stream);
    PutBoolean(x->option_of_h263Options, stream);
    if (x->option_of_sqcifMPI)
    {
        PutInteger(1, 32, (uint32)x->sqcifMPI, stream);
    }
    if (x->option_of_qcifMPI)
    {
        PutInteger(1, 32, (uint32)x->qcifMPI, stream);
    }
    if (x->option_of_cifMPI)
    {
        PutInteger(1, 32, (uint32)x->cifMPI, stream);
    }
    if (x->option_of_cif4MPI)
    {
        PutInteger(1, 32, (uint32)x->cif4MPI, stream);
    }
    if (x->option_of_cif16MPI)
    {
        PutInteger(1, 32, (uint32)x->cif16MPI, stream);
    }
    PutInteger(1, 192400, (uint32)x->maxBitRate, stream);
    PutBoolean(x->unrestrictedVector, stream);
    PutBoolean(x->arithmeticCoding, stream);
    PutBoolean(x->temporalSpatialTradeOffCapability, stream);
    if (x->option_of_slowSqcifMPI)
    {
        PutInteger(1, 3600, (uint32)x->slowSqcifMPI, stream);
    }
    if (x->option_of_slowQcifMPI)
    {
        PutInteger(1, 3600, (uint32)x->slowQcifMPI, stream);
    }
    if (x->option_of_slowCifMPI)
    {
        PutInteger(1, 3600, (uint32)x->slowCifMPI, stream);
    }
    if (x->option_of_slowCif4MPI)
    {
        PutInteger(1, 3600, (uint32)x->slowCif4MPI, stream);
    }
    if (x->option_of_slowCif16MPI)
    {
        PutInteger(1, 3600, (uint32)x->slowCif16MPI, stream);
    }
    PutBoolean(x->errorCompensation, stream);
    if (x->option_of_h263Options)
    {
        Encode_H263Options(&x->h263Options, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for H263Options (SEQUENCE)  */
/* <======================================> */
void Encode_H263Options(PS_H263Options x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_transparencyParameters, stream);
    PutBoolean(x->option_of_refPictureSelection, stream);
    PutBoolean(x->option_of_customPictureClockFrequency, stream);
    PutBoolean(x->option_of_customPictureFormat, stream);
    PutBoolean(x->option_of_modeCombos, stream);
    PutBoolean(x->advancedIntraCodingMode, stream);
    PutBoolean(x->deblockingFilterMode, stream);
    PutBoolean(x->improvedPBFramesMode, stream);
    PutBoolean(x->unlimitedMotionVectors, stream);
    PutBoolean(x->fullPictureFreeze, stream);
    PutBoolean(x->partialPictureFreezeAndRelease, stream);
    PutBoolean(x->resizingPartPicFreezeAndRelease, stream);
    PutBoolean(x->fullPictureSnapshot, stream);
    PutBoolean(x->partialPictureSnapshot, stream);
    PutBoolean(x->videoSegmentTagging, stream);
    PutBoolean(x->progressiveRefinement, stream);
    PutBoolean(x->dynamicPictureResizingByFour, stream);
    PutBoolean(x->dynamicPictureResizingSixteenthPel, stream);
    PutBoolean(x->dynamicWarpingHalfPel, stream);
    PutBoolean(x->dynamicWarpingSixteenthPel, stream);
    PutBoolean(x->independentSegmentDecoding, stream);
    PutBoolean(x->slicesInOrder_NonRect, stream);
    PutBoolean(x->slicesInOrder_Rect, stream);
    PutBoolean(x->slicesNoOrder_NonRect, stream);
    PutBoolean(x->slicesNoOrder_Rect, stream);
    PutBoolean(x->alternateInterVLCMode, stream);
    PutBoolean(x->modifiedQuantizationMode, stream);
    PutBoolean(x->reducedResolutionUpdate, stream);
    if (x->option_of_transparencyParameters)
    {
        Encode_TransparencyParameters(&x->transparencyParameters, stream);
    }
    PutBoolean(x->separateVideoBackChannel, stream);
    if (x->option_of_refPictureSelection)
    {
        Encode_RefPictureSelection(&x->refPictureSelection, stream);
    }
    if (x->option_of_customPictureClockFrequency)
    {
        PutInteger(1, 16, (uint32)x->size_of_customPictureClockFrequency, stream);
        for (i = 0;i < x->size_of_customPictureClockFrequency;++i)
        {
            Encode_CustomPictureClockFrequency(x->customPictureClockFrequency + i, stream);
        }
    }
    if (x->option_of_customPictureFormat)
    {
        PutInteger(1, 16, (uint32)x->size_of_customPictureFormat, stream);
        for (i = 0;i < x->size_of_customPictureFormat;++i)
        {
            Encode_CustomPictureFormat(x->customPictureFormat + i, stream);
        }
    }
    if (x->option_of_modeCombos)
    {
        PutInteger(1, 16, (uint32)x->size_of_modeCombos, stream);
        for (i = 0;i < x->size_of_modeCombos;++i)
        {
            Encode_H263VideoModeCombos(x->modeCombos + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for TransparencyParameters (SEQUENCE)  */
/* <=================================================> */
void Encode_TransparencyParameters(PS_TransparencyParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->presentationOrder, stream);
    PutSignedInteger(-262144, 262143, (int32)x->offset_x, stream);
    PutSignedInteger(-262144, 262143, (int32)x->offset_y, stream);
    PutInteger(1, 255, (uint32)x->scale_x, stream);
    PutInteger(1, 255, (uint32)x->scale_y, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for RefPictureSelection (SEQUENCE)  */
/* <==============================================> */
void Encode_RefPictureSelection(PS_RefPictureSelection x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_additionalPictureMemory, stream);
    if (x->option_of_additionalPictureMemory)
    {
        Encode_AdditionalPictureMemory(&x->additionalPictureMemory, stream);
    }
    PutBoolean(x->videoMux, stream);
    Encode_VideoBackChannelSend(&x->videoBackChannelSend, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for VideoBackChannelSend (CHOICE)  */
/* <=============================================> */
void Encode_VideoBackChannelSend(PS_VideoBackChannelSend x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_VideoBackChannelSend: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Encoder for AdditionalPictureMemory (SEQUENCE)  */
/* <==================================================> */
void Encode_AdditionalPictureMemory(PS_AdditionalPictureMemory x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_sqcifAdditionalPictureMemory, stream);
    PutBoolean(x->option_of_qcifAdditionalPictureMemory, stream);
    PutBoolean(x->option_of_cifAdditionalPictureMemory, stream);
    PutBoolean(x->option_of_cif4AdditionalPictureMemory, stream);
    PutBoolean(x->option_of_cif16AdditionalPictureMemory, stream);
    PutBoolean(x->option_of_bigCpfAdditionalPictureMemory, stream);
    if (x->option_of_sqcifAdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->sqcifAdditionalPictureMemory, stream);
    }
    if (x->option_of_qcifAdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->qcifAdditionalPictureMemory, stream);
    }
    if (x->option_of_cifAdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->cifAdditionalPictureMemory, stream);
    }
    if (x->option_of_cif4AdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->cif4AdditionalPictureMemory, stream);
    }
    if (x->option_of_cif16AdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->cif16AdditionalPictureMemory, stream);
    }
    if (x->option_of_bigCpfAdditionalPictureMemory)
    {
        PutInteger(1, 256, (uint32)x->bigCpfAdditionalPictureMemory, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for CustomPictureClockFrequency (SEQUENCE)  */
/* <======================================================> */
void Encode_CustomPictureClockFrequency(PS_CustomPictureClockFrequency x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_sqcifMPI, stream);
    PutBoolean(x->option_of_qcifMPI, stream);
    PutBoolean(x->option_of_cifMPI, stream);
    PutBoolean(x->option_of_cif4MPI, stream);
    PutBoolean(x->option_of_cif16MPI, stream);
    PutInteger(1000, 1001, (uint32)x->clockConversionCode, stream);
    PutInteger(1, 127, (uint32)x->clockDivisor, stream);
    if (x->option_of_sqcifMPI)
    {
        PutInteger(1, 2048, (uint32)x->sqcifMPI, stream);
    }
    if (x->option_of_qcifMPI)
    {
        PutInteger(1, 2048, (uint32)x->qcifMPI, stream);
    }
    if (x->option_of_cifMPI)
    {
        PutInteger(1, 2048, (uint32)x->cifMPI, stream);
    }
    if (x->option_of_cif4MPI)
    {
        PutInteger(1, 2048, (uint32)x->cif4MPI, stream);
    }
    if (x->option_of_cif16MPI)
    {
        PutInteger(1, 2048, (uint32)x->cif16MPI, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for CustomPictureFormat (SEQUENCE)  */
/* <==============================================> */
void Encode_CustomPictureFormat(PS_CustomPictureFormat x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 2048, (uint32)x->maxCustomPictureWidth, stream);
    PutInteger(1, 2048, (uint32)x->maxCustomPictureHeight, stream);
    PutInteger(1, 2048, (uint32)x->minCustomPictureWidth, stream);
    PutInteger(1, 2048, (uint32)x->minCustomPictureHeight, stream);
    Encode_MPI(&x->mPI, stream);
    Encode_PixelAspectInformation(&x->pixelAspectInformation, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for PixelAspectInformation (CHOICE)  */
/* <===============================================> */
void Encode_PixelAspectInformation(PS_PixelAspectInformation x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutBoolean(x->anyPixelAspectRatio, stream);
            break;
        case 1:
            PutInteger(1, 14, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                PutInteger(1, 14, (uint32)x->pixelAspectCode[i], stream);
            }
            break;
        case 2:
            PutInteger(1, 256, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_ExtendedPARItem(x->extendedPAR + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_PixelAspectInformation: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for ExtendedPARItem (SEQUENCE)  */
/* <==========================================> */
void Encode_ExtendedPARItem(PS_ExtendedPARItem x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 255, (uint32)x->width, stream);
    PutInteger(1, 255, (uint32)x->height, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================> */
/*  PER-Encoder for MPI (SEQUENCE)  */
/* <==============================> */
void Encode_MPI(PS_MPI x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_standardMPI, stream);
    PutBoolean(x->option_of_customPCF, stream);
    if (x->option_of_standardMPI)
    {
        PutInteger(1, 31, (uint32)x->standardMPI, stream);
    }
    if (x->option_of_customPCF)
    {
        PutInteger(1, 16, (uint32)x->size_of_customPCF, stream);
        for (i = 0;i < x->size_of_customPCF;++i)
        {
            Encode_CustomPCFItem(x->customPCF + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for CustomPCFItem (SEQUENCE)  */
/* <========================================> */
void Encode_CustomPCFItem(PS_CustomPCFItem x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1000, 1001, (uint32)x->clockConversionCode, stream);
    PutInteger(1, 127, (uint32)x->clockDivisor, stream);
    PutInteger(1, 2048, (uint32)x->customMPI, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for H263VideoModeCombos (SEQUENCE)  */
/* <==============================================> */
void Encode_H263VideoModeCombos(PS_H263VideoModeCombos x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_H263ModeComboFlags(&x->h263VideoUncoupledModes, stream);
    PutInteger(1, 16, (uint32)x->size_of_h263VideoCoupledModes, stream);
    for (i = 0;i < x->size_of_h263VideoCoupledModes;++i)
    {
        Encode_H263ModeComboFlags(x->h263VideoCoupledModes + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for H263ModeComboFlags (SEQUENCE)  */
/* <=============================================> */
void Encode_H263ModeComboFlags(PS_H263ModeComboFlags x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->unrestrictedVector, stream);
    PutBoolean(x->arithmeticCoding, stream);
    PutBoolean(x->advancedPrediction, stream);
    PutBoolean(x->pbFrames, stream);
    PutBoolean(x->advancedIntraCodingMode, stream);
    PutBoolean(x->deblockingFilterMode, stream);
    PutBoolean(x->unlimitedMotionVectors, stream);
    PutBoolean(x->slicesInOrder_NonRect, stream);
    PutBoolean(x->slicesInOrder_Rect, stream);
    PutBoolean(x->slicesNoOrder_NonRect, stream);
    PutBoolean(x->slicesNoOrder_Rect, stream);
    PutBoolean(x->improvedPBFramesMode, stream);
    PutBoolean(x->referencePicSelect, stream);
    PutBoolean(x->dynamicPictureResizingByFour, stream);
    PutBoolean(x->dynamicPictureResizingSixteenthPel, stream);
    PutBoolean(x->dynamicWarpingHalfPel, stream);
    PutBoolean(x->dynamicWarpingSixteenthPel, stream);
    PutBoolean(x->reducedResolutionUpdate, stream);
    PutBoolean(x->independentSegmentDecoding, stream);
    PutBoolean(x->alternateInterVLCMode, stream);
    PutBoolean(x->modifiedQuantizationMode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for IS11172VideoCapability (SEQUENCE)  */
/* <=================================================> */
void Encode_IS11172VideoCapability(PS_IS11172VideoCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_videoBitRate, stream);
    PutBoolean(x->option_of_vbvBufferSize, stream);
    PutBoolean(x->option_of_samplesPerLine, stream);
    PutBoolean(x->option_of_linesPerFrame, stream);
    PutBoolean(x->option_of_pictureRate, stream);
    PutBoolean(x->option_of_luminanceSampleRate, stream);
    PutBoolean(x->constrainedBitstream, stream);
    if (x->option_of_videoBitRate)
    {
        PutInteger(0, 1073741823, (uint32)x->videoBitRate, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        PutInteger(0, 262143, (uint32)x->vbvBufferSize, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        PutInteger(0, 16383, (uint32)x->samplesPerLine, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        PutInteger(0, 16383, (uint32)x->linesPerFrame, stream);
    }
    if (x->option_of_pictureRate)
    {
        PutInteger(0, 15, (uint32)x->pictureRate, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->luminanceSampleRate, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for AudioCapability (CHOICE)  */
/* <========================================> */
void Encode_AudioCapability(PS_AudioCapability x, PS_OutStream stream)
{
    PutChoiceIndex(14, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            PutInteger(1, 256, (uint32)x->g711Alaw64k, stream);
            break;
        case 2:
            PutInteger(1, 256, (uint32)x->g711Alaw56k, stream);
            break;
        case 3:
            PutInteger(1, 256, (uint32)x->g711Ulaw64k, stream);
            break;
        case 4:
            PutInteger(1, 256, (uint32)x->g711Ulaw56k, stream);
            break;
        case 5:
            PutInteger(1, 256, (uint32)x->g722_64k, stream);
            break;
        case 6:
            PutInteger(1, 256, (uint32)x->g722_56k, stream);
            break;
        case 7:
            PutInteger(1, 256, (uint32)x->g722_48k, stream);
            break;
        case 8:
            Encode_G7231(x->g7231, stream);
            break;
        case 9:
            PutInteger(1, 256, (uint32)x->g728, stream);
            break;
        case 10:
            PutInteger(1, 256, (uint32)x->g729, stream);
            break;
        case 11:
            PutInteger(1, 256, (uint32)x->g729AnnexA, stream);
            break;
        case 12:
            Encode_IS11172AudioCapability(x->is11172AudioCapability, stream);
            break;
        case 13:
            Encode_IS13818AudioCapability(x->is13818AudioCapability, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            PutExtensionInteger(1, 256, (uint32)x->g729wAnnexB, stream);
            break;
        case 15:
            PutExtensionInteger(1, 256, (uint32)x->g729AnnexAwAnnexB, stream);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_G7231AnnexCCapability, (uint8*)x->g7231AnnexCCapability, stream);
            break;
        case 17:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmFullRate, stream);
            break;
        case 18:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmHalfRate, stream);
            break;
        case 19:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmEnhancedFullRate, stream);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericAudioCapability, stream);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_G729Extensions, (uint8*)x->g729Extensions, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_AudioCapability: Illegal CHOICE index");
    }
}

/* <================================> */
/*  PER-Encoder for G7231 (SEQUENCE)  */
/* <================================> */
void Encode_G7231(PS_G7231 x, PS_OutStream stream)
{
    PutInteger(1, 256, (uint32)x->maxAl_sduAudioFrames, stream);
    PutBoolean(x->silenceSuppression, stream);
}

/* <=========================================> */
/*  PER-Encoder for G729Extensions (SEQUENCE)  */
/* <=========================================> */
void Encode_G729Extensions(PS_G729Extensions x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_audioUnit, stream);
    if (x->option_of_audioUnit)
    {
        PutInteger(1, 256, (uint32)x->audioUnit, stream);
    }
    PutBoolean(x->annexA, stream);
    PutBoolean(x->annexB, stream);
    PutBoolean(x->annexD, stream);
    PutBoolean(x->annexE, stream);
    PutBoolean(x->annexF, stream);
    PutBoolean(x->annexG, stream);
    PutBoolean(x->annexH, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for G7231AnnexCCapability (SEQUENCE)  */
/* <================================================> */
void Encode_G7231AnnexCCapability(PS_G7231AnnexCCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_g723AnnexCAudioMode, stream);
    PutInteger(1, 256, (uint32)x->maxAl_sduAudioFrames, stream);
    PutBoolean(x->silenceSuppression, stream);
    if (x->option_of_g723AnnexCAudioMode)
    {
        Encode_G723AnnexCAudioMode(&x->g723AnnexCAudioMode, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for G723AnnexCAudioMode (SEQUENCE)  */
/* <==============================================> */
void Encode_G723AnnexCAudioMode(PS_G723AnnexCAudioMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(27, 78, (uint32)x->highRateMode0, stream);
    PutInteger(27, 78, (uint32)x->highRateMode1, stream);
    PutInteger(23, 66, (uint32)x->lowRateMode0, stream);
    PutInteger(23, 66, (uint32)x->lowRateMode1, stream);
    PutInteger(6, 17, (uint32)x->sidMode0, stream);
    PutInteger(6, 17, (uint32)x->sidMode1, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for IS11172AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Encode_IS11172AudioCapability(PS_IS11172AudioCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->audioLayer1, stream);
    PutBoolean(x->audioLayer2, stream);
    PutBoolean(x->audioLayer3, stream);
    PutBoolean(x->audioSampling32k, stream);
    PutBoolean(x->audioSampling44k1, stream);
    PutBoolean(x->audioSampling48k, stream);
    PutBoolean(x->singleChannel, stream);
    PutBoolean(x->twoChannels, stream);
    PutInteger(1, 448, (uint32)x->bitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for IS13818AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Encode_IS13818AudioCapability(PS_IS13818AudioCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->audioLayer1, stream);
    PutBoolean(x->audioLayer2, stream);
    PutBoolean(x->audioLayer3, stream);
    PutBoolean(x->audioSampling16k, stream);
    PutBoolean(x->audioSampling22k05, stream);
    PutBoolean(x->audioSampling24k, stream);
    PutBoolean(x->audioSampling32k, stream);
    PutBoolean(x->audioSampling44k1, stream);
    PutBoolean(x->audioSampling48k, stream);
    PutBoolean(x->singleChannel, stream);
    PutBoolean(x->twoChannels, stream);
    PutBoolean(x->threeChannels2_1, stream);
    PutBoolean(x->threeChannels3_0, stream);
    PutBoolean(x->fourChannels2_0_2_0, stream);
    PutBoolean(x->fourChannels2_2, stream);
    PutBoolean(x->fourChannels3_1, stream);
    PutBoolean(x->fiveChannels3_0_2_0, stream);
    PutBoolean(x->fiveChannels3_2, stream);
    PutBoolean(x->lowFrequencyEnhancement, stream);
    PutBoolean(x->multilingual, stream);
    PutInteger(1, 1130, (uint32)x->bitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for GSMAudioCapability (SEQUENCE)  */
/* <=============================================> */
void Encode_GSMAudioCapability(PS_GSMAudioCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->audioUnitSize, stream);
    PutBoolean(x->comfortNoise, stream);
    PutBoolean(x->scrambled, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Encoder for DataApplicationCapability (SEQUENCE)  */
/* <====================================================> */
void Encode_DataApplicationCapability(PS_DataApplicationCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Application(&x->application, stream);
    PutInteger(0, 0xffffffff, (uint32)x->maxBitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Encoder for Application (CHOICE)  */
/* <====================================> */
void Encode_Application(PS_Application x, PS_OutStream stream)
{
    PutChoiceIndex(10, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_DataProtocolCapability(x->t120, stream);
            break;
        case 2:
            Encode_DataProtocolCapability(x->dsm_cc, stream);
            break;
        case 3:
            Encode_DataProtocolCapability(x->userData, stream);
            break;
        case 4:
            Encode_T84(x->t84, stream);
            break;
        case 5:
            Encode_DataProtocolCapability(x->t434, stream);
            break;
        case 6:
            Encode_DataProtocolCapability(x->h224, stream);
            break;
        case 7:
            Encode_Nlpid(x->nlpid, stream);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            Encode_DataProtocolCapability(x->h222DataPartitioning, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            PutExtensionItem(EPASS Encode_DataProtocolCapability, (uint8*)x->t30fax, stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_DataProtocolCapability, (uint8*)x->t140, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_T38fax, (uint8*)x->t38fax, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericDataCapability, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_Application: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Encoder for T38fax (SEQUENCE)  */
/* <=================================> */
void Encode_T38fax(PS_T38fax x, PS_OutStream stream)
{
    Encode_DataProtocolCapability(&x->t38FaxProtocol, stream);
    Encode_T38FaxProfile(&x->t38FaxProfile, stream);
}

/* <================================> */
/*  PER-Encoder for Nlpid (SEQUENCE)  */
/* <================================> */
void Encode_Nlpid(PS_Nlpid x, PS_OutStream stream)
{
    Encode_DataProtocolCapability(&x->nlpidProtocol, stream);
    PutOctetString(1, 0, 0, &x->nlpidData, stream);
}

/* <==============================> */
/*  PER-Encoder for T84 (SEQUENCE)  */
/* <==============================> */
void Encode_T84(PS_T84 x, PS_OutStream stream)
{
    Encode_DataProtocolCapability(&x->t84Protocol, stream);
    Encode_T84Profile(&x->t84Profile, stream);
}

/* <===============================================> */
/*  PER-Encoder for DataProtocolCapability (CHOICE)  */
/* <===============================================> */
void Encode_DataProtocolCapability(PS_DataProtocolCapability x, PS_OutStream stream)
{
    PutChoiceIndex(7, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
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
            PutExtensionNull(stream);
            break;
        case 8:
            PutExtensionNull(stream);
            break;
        case 9:
            PutExtensionNull(stream);
            break;
        case 10:
            PutExtensionNull(stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_V76wCompression, (uint8*)x->v76wCompression, stream);
            break;
        case 12:
            PutExtensionNull(stream);
            break;
        case 13:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_DataProtocolCapability: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for V76wCompression (CHOICE)  */
/* <========================================> */
void Encode_V76wCompression(PS_V76wCompression x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_CompressionType(x->transmitCompression, stream);
            break;
        case 1:
            Encode_CompressionType(x->receiveCompression, stream);
            break;
        case 2:
            Encode_CompressionType(x->transmitAndReceiveCompression, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_V76wCompression: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for CompressionType (CHOICE)  */
/* <========================================> */
void Encode_CompressionType(PS_CompressionType x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_V42bis(x->v42bis, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_CompressionType: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Encoder for V42bis (SEQUENCE)  */
/* <=================================> */
void Encode_V42bis(PS_V42bis x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65536, (uint32)x->numberOfCodewords, stream);
    PutInteger(1, 256, (uint32)x->maximumStringLength, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Encoder for T84Profile (CHOICE)  */
/* <===================================> */
void Encode_T84Profile(PS_T84Profile x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (t84Unrestricted is NULL) */
            break;
        case 1:
            Encode_T84Restricted(x->t84Restricted, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_T84Profile: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for T84Restricted (SEQUENCE)  */
/* <========================================> */
void Encode_T84Restricted(PS_T84Restricted x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->qcif, stream);
    PutBoolean(x->cif, stream);
    PutBoolean(x->ccir601Seq, stream);
    PutBoolean(x->ccir601Prog, stream);
    PutBoolean(x->hdtvSeq, stream);
    PutBoolean(x->hdtvProg, stream);
    PutBoolean(x->g3FacsMH200x100, stream);
    PutBoolean(x->g3FacsMH200x200, stream);
    PutBoolean(x->g4FacsMMR200x100, stream);
    PutBoolean(x->g4FacsMMR200x200, stream);
    PutBoolean(x->jbig200x200Seq, stream);
    PutBoolean(x->jbig200x200Prog, stream);
    PutBoolean(x->jbig300x300Seq, stream);
    PutBoolean(x->jbig300x300Prog, stream);
    PutBoolean(x->digPhotoLow, stream);
    PutBoolean(x->digPhotoMedSeq, stream);
    PutBoolean(x->digPhotoMedProg, stream);
    PutBoolean(x->digPhotoHighSeq, stream);
    PutBoolean(x->digPhotoHighProg, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for T38FaxProfile (SEQUENCE)  */
/* <========================================> */
void Encode_T38FaxProfile(PS_T38FaxProfile x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_version |
                x->option_of_t38FaxRateManagement |
                x->option_of_t38FaxUdpOptions;
    PutBoolean(extension, stream);

    PutBoolean(x->fillBitRemoval, stream);
    PutBoolean(x->transcodingJBIG, stream);
    PutBoolean(x->transcodingMMR, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(3, stream); /* Begin Options Map */
    PutBoolean(x->option_of_version, stream);
    PutBoolean(x->option_of_t38FaxRateManagement, stream);
    PutBoolean(x->option_of_t38FaxUdpOptions, stream);
    if (x->option_of_version)
    {
        PutExtensionInteger(0, 255, (uint32)x->version, stream);
    }
    if (x->option_of_t38FaxRateManagement)
    {
        PutExtensionItem(EPASS Encode_T38FaxRateManagement, (uint8*)&x->t38FaxRateManagement, stream);
    }
    if (x->option_of_t38FaxUdpOptions)
    {
        PutExtensionItem(EPASS Encode_T38FaxUdpOptions, (uint8*)&x->t38FaxUdpOptions, stream);
    }
}

/* <=============================================> */
/*  PER-Encoder for T38FaxRateManagement (CHOICE)  */
/* <=============================================> */
void Encode_T38FaxRateManagement(PS_T38FaxRateManagement x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_T38FaxRateManagement: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for T38FaxUdpOptions (SEQUENCE)  */
/* <===========================================> */
void Encode_T38FaxUdpOptions(PS_T38FaxUdpOptions x, PS_OutStream stream)
{
    PutBoolean(x->option_of_t38FaxMaxBuffer, stream);
    PutBoolean(x->option_of_t38FaxMaxDatagram, stream);
    if (x->option_of_t38FaxMaxBuffer)
    {
        PutUnboundedInteger((uint32)x->t38FaxMaxBuffer, stream);
    }
    if (x->option_of_t38FaxMaxDatagram)
    {
        PutUnboundedInteger((uint32)x->t38FaxMaxDatagram, stream);
    }
    Encode_T38FaxUdpEC(&x->t38FaxUdpEC, stream);
}

/* <====================================> */
/*  PER-Encoder for T38FaxUdpEC (CHOICE)  */
/* <====================================> */
void Encode_T38FaxUdpEC(PS_T38FaxUdpEC x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_T38FaxUdpEC: Illegal CHOICE index");
    }
}

/* <===============================================================> */
/*  PER-Encoder for EncryptionAuthenticationAndIntegrity (SEQUENCE)  */
/* <===============================================================> */
void Encode_EncryptionAuthenticationAndIntegrity(PS_EncryptionAuthenticationAndIntegrity x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_encryptionCapability, stream);
    PutBoolean(x->option_of_authenticationCapability, stream);
    PutBoolean(x->option_of_integrityCapability, stream);
    if (x->option_of_encryptionCapability)
    {
        Encode_EncryptionCapability(&x->encryptionCapability, stream);
    }
    if (x->option_of_authenticationCapability)
    {
        Encode_AuthenticationCapability(&x->authenticationCapability, stream);
    }
    if (x->option_of_integrityCapability)
    {
        Encode_IntegrityCapability(&x->integrityCapability, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for EncryptionCapability (SEQUENCE-OF)  */
/* <==================================================> */
void Encode_EncryptionCapability(PS_EncryptionCapability x, PS_OutStream stream)
{
    uint16 i;
    PutInteger(1, 256, (uint32)x->size, stream);
    for (i = 0;i < x->size;++i)
    {
        Encode_MediaEncryptionAlgorithm(x->item + i, stream);
    }
}

/* <=================================================> */
/*  PER-Encoder for MediaEncryptionAlgorithm (CHOICE)  */
/* <=================================================> */
void Encode_MediaEncryptionAlgorithm(PS_MediaEncryptionAlgorithm x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            PutObjectID(x->algorithm, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MediaEncryptionAlgorithm: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Encoder for AuthenticationCapability (SEQUENCE)  */
/* <===================================================> */
void Encode_AuthenticationCapability(PS_AuthenticationCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_nonStandard, stream);
    if (x->option_of_nonStandard)
    {
        Encode_NonStandardParameter(&x->nonStandard, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for IntegrityCapability (SEQUENCE)  */
/* <==============================================> */
void Encode_IntegrityCapability(PS_IntegrityCapability x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_nonStandard, stream);
    if (x->option_of_nonStandard)
    {
        Encode_NonStandardParameter(&x->nonStandard, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for UserInputCapability (CHOICE)  */
/* <============================================> */
void Encode_UserInputCapability(PS_UserInputCapability x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(6, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 16, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_NonStandardParameter(x->nonStandard + i, stream);
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
            ErrorMessageAndLeave("Encode_UserInputCapability: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Encoder for ConferenceCapability (SEQUENCE)  */
/* <===============================================> */
void Encode_ConferenceCapability(PS_ConferenceCapability x, PS_OutStream stream)
{
    uint16 i;
    uint32 extension;

    extension = x->option_of_VideoIndicateMixingCapability |
                x->option_of_multipointVisualizationCapability;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_nonStandardData, stream);
    if (x->option_of_nonStandardData)
    {
        PutLengthDet(x->size_of_nonStandardData, stream);
        for (i = 0;i < x->size_of_nonStandardData;++i)
        {
            Encode_NonStandardParameter(x->nonStandardData + i, stream);
        }
    }
    PutBoolean(x->chairControlCapability, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_VideoIndicateMixingCapability, stream);
    PutBoolean(x->option_of_multipointVisualizationCapability, stream);
    if (x->option_of_VideoIndicateMixingCapability)
    {
        PutExtensionBoolean(x->VideoIndicateMixingCapability, stream);
    }
    if (x->option_of_multipointVisualizationCapability)
    {
        PutExtensionBoolean(x->multipointVisualizationCapability, stream);
    }
}

/* <============================================> */
/*  PER-Encoder for GenericCapability (SEQUENCE)  */
/* <============================================> */
void Encode_GenericCapability(PS_GenericCapability x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_maxBitRate, stream);
    PutBoolean(x->option_of_collapsing, stream);
    PutBoolean(x->option_of_nonCollapsing, stream);
    PutBoolean(x->option_of_nonCollapsingRaw, stream);
    PutBoolean(x->option_of_transport, stream);
    Encode_CapabilityIdentifier(&x->capabilityIdentifier, stream);
    if (x->option_of_maxBitRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->maxBitRate, stream);
    }
    if (x->option_of_collapsing)
    {
        PutLengthDet(x->size_of_collapsing, stream);
        for (i = 0;i < x->size_of_collapsing;++i)
        {
            Encode_GenericParameter(x->collapsing + i, stream);
        }
    }
    if (x->option_of_nonCollapsing)
    {
        PutLengthDet(x->size_of_nonCollapsing, stream);
        for (i = 0;i < x->size_of_nonCollapsing;++i)
        {
            Encode_GenericParameter(x->nonCollapsing + i, stream);
        }
    }
    if (x->option_of_nonCollapsingRaw)
    {
        PutOctetString(1, 0, 0, &x->nonCollapsingRaw, stream);
    }
    if (x->option_of_transport)
    {
        Encode_DataProtocolCapability(&x->transport, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for CapabilityIdentifier (CHOICE)  */
/* <=============================================> */
void Encode_CapabilityIdentifier(PS_CapabilityIdentifier x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutObjectID(x->standard, stream);
            break;
        case 1:
            Encode_NonStandardParameter(x->h221NonStandard, stream);
            break;
        case 2:
            PutOctetString(0, 16, 16, x->uuid, stream);
            break;
        case 3:
            PutCharString("IA5String", 0, 1, 64, NULL, x->domainBased, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_CapabilityIdentifier: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for GenericParameter (SEQUENCE)  */
/* <===========================================> */
void Encode_GenericParameter(PS_GenericParameter x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_supersedes, stream);
    Encode_ParameterIdentifier(&x->parameterIdentifier, stream);
    Encode_ParameterValue(&x->parameterValue, stream);
    if (x->option_of_supersedes)
    {
        PutLengthDet(x->size_of_supersedes, stream);
        for (i = 0;i < x->size_of_supersedes;++i)
        {
            Encode_ParameterIdentifier(x->supersedes + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for ParameterIdentifier (CHOICE)  */
/* <============================================> */
void Encode_ParameterIdentifier(PS_ParameterIdentifier x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 127, (uint32)x->standard, stream);
            break;
        case 1:
            Encode_NonStandardParameter(x->h221NonStandard, stream);
            break;
        case 2:
            PutOctetString(0, 16, 16, x->uuid, stream);
            break;
        case 3:
            PutCharString("IA5String", 0, 1, 64, NULL, x->domainBased, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_ParameterIdentifier: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for ParameterValue (CHOICE)  */
/* <=======================================> */
void Encode_ParameterValue(PS_ParameterValue x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(8, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (logical is NULL) */
            break;
        case 1:
            PutInteger(0, 255, (uint32)x->booleanArray, stream);
            break;
        case 2:
            PutInteger(0, 65535, (uint32)x->unsignedMin, stream);
            break;
        case 3:
            PutInteger(0, 65535, (uint32)x->unsignedMax, stream);
            break;
        case 4:
            PutInteger(0, 0xffffffff, (uint32)x->unsigned32Min, stream);
            break;
        case 5:
            PutInteger(0, 0xffffffff, (uint32)x->unsigned32Max, stream);
            break;
        case 6:
            PutOctetString(1, 0, 0, x->octetString, stream);
            break;
        case 7:
            PutLengthDet(x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_GenericParameter(x->genericParameter + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_ParameterValue: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for OpenLogicalChannel (SEQUENCE)  */
/* <=============================================> */
void Encode_OpenLogicalChannel(PS_OpenLogicalChannel x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_separateStack |
                x->option_of_encryptionSync;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_reverseLogicalChannelParameters, stream);
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    Encode_ForwardLogicalChannelParameters(&x->forwardLogicalChannelParameters, stream);
    if (x->option_of_reverseLogicalChannelParameters)
    {
        Encode_ReverseLogicalChannelParameters(&x->reverseLogicalChannelParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_separateStack, stream);
    PutBoolean(x->option_of_encryptionSync, stream);
    if (x->option_of_separateStack)
    {
        PutExtensionItem(EPASS Encode_NetworkAccessParameters, (uint8*)&x->separateStack, stream);
    }
    if (x->option_of_encryptionSync)
    {
        PutExtensionItem(EPASS Encode_EncryptionSync, (uint8*)&x->encryptionSync, stream);
    }
}

/* <==========================================================> */
/*  PER-Encoder for ReverseLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Encode_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_reverseLogicalChannelDependency |
                x->option_of_replacementFor;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_rlcMultiplexParameters, stream);
    Encode_DataType(&x->dataType, stream);
    if (x->option_of_rlcMultiplexParameters)
    {
        Encode_RlcMultiplexParameters(&x->rlcMultiplexParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_reverseLogicalChannelDependency, stream);
    PutBoolean(x->option_of_replacementFor, stream);
    if (x->option_of_reverseLogicalChannelDependency)
    {
        PutExtensionInteger(1, 65535, (uint32)x->reverseLogicalChannelDependency, stream);
    }
    if (x->option_of_replacementFor)
    {
        PutExtensionInteger(1, 65535, (uint32)x->replacementFor, stream);
    }
}

/* <===============================================> */
/*  PER-Encoder for RlcMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Encode_RlcMultiplexParameters(PS_RlcMultiplexParameters x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_H223LogicalChannelParameters(x->h223LogicalChannelParameters, stream);
            break;
        case 1:
            Encode_V76LogicalChannelParameters(x->v76LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            PutExtensionItem(EPASS Encode_H2250LogicalChannelParameters, (uint8*)x->h2250LogicalChannelParameters, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_RlcMultiplexParameters: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Encoder for ForwardLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Encode_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_forwardLogicalChannelDependency |
                x->option_of_replacementFor;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_portNumber, stream);
    if (x->option_of_portNumber)
    {
        PutInteger(0, 65535, (uint32)x->portNumber, stream);
    }
    Encode_DataType(&x->dataType, stream);
    Encode_MultiplexParameters(&x->multiplexParameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_forwardLogicalChannelDependency, stream);
    PutBoolean(x->option_of_replacementFor, stream);
    if (x->option_of_forwardLogicalChannelDependency)
    {
        PutExtensionInteger(1, 65535, (uint32)x->forwardLogicalChannelDependency, stream);
    }
    if (x->option_of_replacementFor)
    {
        PutExtensionInteger(1, 65535, (uint32)x->replacementFor, stream);
    }
}

/* <============================================> */
/*  PER-Encoder for MultiplexParameters (CHOICE)  */
/* <============================================> */
void Encode_MultiplexParameters(PS_MultiplexParameters x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_H222LogicalChannelParameters(x->h222LogicalChannelParameters, stream);
            break;
        case 1:
            Encode_H223LogicalChannelParameters(x->h223LogicalChannelParameters, stream);
            break;
        case 2:
            Encode_V76LogicalChannelParameters(x->v76LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            PutExtensionItem(EPASS Encode_H2250LogicalChannelParameters, (uint8*)x->h2250LogicalChannelParameters, stream);
            break;
        case 4:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MultiplexParameters: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Encoder for NetworkAccessParameters (SEQUENCE)  */
/* <==================================================> */
void Encode_NetworkAccessParameters(PS_NetworkAccessParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_t120SetupProcedure;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_distribution, stream);
    PutBoolean(x->option_of_externalReference, stream);
    if (x->option_of_distribution)
    {
        Encode_Distribution(&x->distribution, stream);
    }
    Encode_NetworkAddress(&x->networkAddress, stream);
    PutBoolean(x->associateConference, stream);
    if (x->option_of_externalReference)
    {
        PutOctetString(0, 1, 255, &x->externalReference, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_t120SetupProcedure, stream);
    if (x->option_of_t120SetupProcedure)
    {
        PutExtensionItem(EPASS Encode_T120SetupProcedure, (uint8*)&x->t120SetupProcedure, stream);
    }
}

/* <===========================================> */
/*  PER-Encoder for T120SetupProcedure (CHOICE)  */
/* <===========================================> */
void Encode_T120SetupProcedure(PS_T120SetupProcedure x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_T120SetupProcedure: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for NetworkAddress (CHOICE)  */
/* <=======================================> */
void Encode_NetworkAddress(PS_NetworkAddress x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_Q2931Address(x->q2931Address, stream);
            break;
        case 1:
            PutCharString("IA5String(SIZE(1..128))", 0, 1, 128, "0123456789#*,", x->e164Address, stream);
            break;
        case 2:
            Encode_TransportAddress(x->localAreaAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_NetworkAddress: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for Distribution (CHOICE)  */
/* <=====================================> */
void Encode_Distribution(PS_Distribution x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Distribution: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for Q2931Address (SEQUENCE)  */
/* <=======================================> */
void Encode_Q2931Address(PS_Q2931Address x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_subaddress, stream);
    Encode_Address(&x->address, stream);
    if (x->option_of_subaddress)
    {
        PutOctetString(0, 1, 20, &x->subaddress, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Encoder for Address (CHOICE)  */
/* <================================> */
void Encode_Address(PS_Address x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutCharString("NumericString(SIZE(1..16))", 0, 1, 16, NULL, x->internationalNumber, stream);
            break;
        case 1:
            PutOctetString(0, 1, 20, x->nsapAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_Address: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for V75Parameters (SEQUENCE)  */
/* <========================================> */
void Encode_V75Parameters(PS_V75Parameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->audioHeaderPresent, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for DataType (CHOICE)  */
/* <=================================> */
void Encode_DataType(PS_DataType x, PS_OutStream stream)
{
    PutChoiceIndex(6, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (nullData is NULL) */
            break;
        case 2:
            Encode_VideoCapability(x->videoData, stream);
            break;
        case 3:
            Encode_AudioCapability(x->audioData, stream);
            break;
        case 4:
            Encode_DataApplicationCapability(x->data, stream);
            break;
        case 5:
            Encode_EncryptionMode(x->encryptionData, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            PutExtensionItem(EPASS Encode_NonStandardParameter, (uint8*)x->h235Control, stream);
            break;
        case 7:
            PutExtensionItem(EPASS Encode_H235Media, (uint8*)x->h235Media, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_DataType: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Encoder for H235Media (SEQUENCE)  */
/* <====================================> */
void Encode_H235Media(PS_H235Media x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    Encode_MediaType(&x->mediaType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for MediaType (CHOICE)  */
/* <==================================> */
void Encode_MediaType(PS_MediaType x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_VideoCapability(x->videoData, stream);
            break;
        case 2:
            Encode_AudioCapability(x->audioData, stream);
            break;
        case 3:
            Encode_DataApplicationCapability(x->data, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MediaType: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Encoder for H222LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Encode_H222LogicalChannelParameters(PS_H222LogicalChannelParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_pcr_pid, stream);
    PutBoolean(x->option_of_programDescriptors, stream);
    PutBoolean(x->option_of_streamDescriptors, stream);
    PutInteger(0, 65535, (uint32)x->resourceID, stream);
    PutInteger(0, 8191, (uint32)x->subChannelID, stream);
    if (x->option_of_pcr_pid)
    {
        PutInteger(0, 8191, (uint32)x->pcr_pid, stream);
    }
    if (x->option_of_programDescriptors)
    {
        PutOctetString(1, 0, 0, &x->programDescriptors, stream);
    }
    if (x->option_of_streamDescriptors)
    {
        PutOctetString(1, 0, 0, &x->streamDescriptors, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Encoder for H223LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Encode_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_AdaptationLayerType(&x->adaptationLayerType, stream);
    PutBoolean(x->segmentableFlag, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for AdaptationLayerType (CHOICE)  */
/* <============================================> */
void Encode_AdaptationLayerType(PS_AdaptationLayerType x, PS_OutStream stream)
{
    PutChoiceIndex(6, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
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
            Encode_Al3(x->al3, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            PutExtensionItem(EPASS Encode_H223AL1MParameters, (uint8*)x->al1M, stream);
            break;
        case 7:
            PutExtensionItem(EPASS Encode_H223AL2MParameters, (uint8*)x->al2M, stream);
            break;
        case 8:
            PutExtensionItem(EPASS Encode_H223AL3MParameters, (uint8*)x->al3M, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_AdaptationLayerType: Illegal CHOICE index");
    }
}

/* <==============================> */
/*  PER-Encoder for Al3 (SEQUENCE)  */
/* <==============================> */
void Encode_Al3(PS_Al3 x, PS_OutStream stream)
{
    PutInteger(0, 2, (uint32)x->controlFieldOctets, stream);
    PutInteger(0, 16777215, (uint32)x->sendBufferSize, stream);
}

/* <=============================================> */
/*  PER-Encoder for H223AL1MParameters (SEQUENCE)  */
/* <=============================================> */
void Encode_H223AL1MParameters(PS_H223AL1MParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_rsCodeCorrection;
    PutBoolean(extension, stream);

    Encode_TransferMode(&x->transferMode, stream);
    Encode_HeaderFEC(&x->headerFEC, stream);
    Encode_CrcLength(&x->crcLength, stream);
    PutInteger(8, 32, (uint32)x->rcpcCodeRate, stream);
    Encode_ArqType(&x->arqType, stream);
    PutBoolean(x->alpduInterleaving, stream);
    PutBoolean(x->alsduSplitting, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_rsCodeCorrection, stream);
    if (x->option_of_rsCodeCorrection)
    {
        PutExtensionInteger(0, 127, (uint32)x->rsCodeCorrection, stream);
    }
}

/* <================================> */
/*  PER-Encoder for ArqType (CHOICE)  */
/* <================================> */
void Encode_ArqType(PS_ArqType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            Encode_H223AnnexCArqParameters(x->typeIArq, stream);
            break;
        case 2:
            Encode_H223AnnexCArqParameters(x->typeIIArq, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_ArqType: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for CrcLength (CHOICE)  */
/* <==================================> */
void Encode_CrcLength(PS_CrcLength x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            PutExtensionNull(stream);
            break;
        case 5:
            PutExtensionNull(stream);
            break;
        case 6:
            PutExtensionNull(stream);
            break;
        case 7:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_CrcLength: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for HeaderFEC (CHOICE)  */
/* <==================================> */
void Encode_HeaderFEC(PS_HeaderFEC x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_HeaderFEC: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for TransferMode (CHOICE)  */
/* <=====================================> */
void Encode_TransferMode(PS_TransferMode x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_TransferMode: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for H223AL2MParameters (SEQUENCE)  */
/* <=============================================> */
void Encode_H223AL2MParameters(PS_H223AL2MParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Al2HeaderFEC(&x->al2HeaderFEC, stream);
    PutBoolean(x->alpduInterleaving, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for Al2HeaderFEC (CHOICE)  */
/* <=====================================> */
void Encode_Al2HeaderFEC(PS_Al2HeaderFEC x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Al2HeaderFEC: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for H223AL3MParameters (SEQUENCE)  */
/* <=============================================> */
void Encode_H223AL3MParameters(PS_H223AL3MParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_rsCodeCorrection;
    PutBoolean(extension, stream);

    Encode_HeaderFormat(&x->headerFormat, stream);
    Encode_Al3CrcLength(&x->al3CrcLength, stream);
    PutInteger(8, 32, (uint32)x->rcpcCodeRate, stream);
    Encode_Al3ArqType(&x->al3ArqType, stream);
    PutBoolean(x->alpduInterleaving, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_rsCodeCorrection, stream);
    if (x->option_of_rsCodeCorrection)
    {
        PutExtensionInteger(0, 127, (uint32)x->rsCodeCorrection, stream);
    }
}

/* <===================================> */
/*  PER-Encoder for Al3ArqType (CHOICE)  */
/* <===================================> */
void Encode_Al3ArqType(PS_Al3ArqType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            Encode_H223AnnexCArqParameters(x->typeIArq, stream);
            break;
        case 2:
            Encode_H223AnnexCArqParameters(x->typeIIArq, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_Al3ArqType: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for Al3CrcLength (CHOICE)  */
/* <=====================================> */
void Encode_Al3CrcLength(PS_Al3CrcLength x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            PutExtensionNull(stream);
            break;
        case 5:
            PutExtensionNull(stream);
            break;
        case 6:
            PutExtensionNull(stream);
            break;
        case 7:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_Al3CrcLength: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for HeaderFormat (CHOICE)  */
/* <=====================================> */
void Encode_HeaderFormat(PS_HeaderFormat x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_HeaderFormat: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Encoder for H223AnnexCArqParameters (SEQUENCE)  */
/* <==================================================> */
void Encode_H223AnnexCArqParameters(PS_H223AnnexCArqParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_NumberOfRetransmissions(&x->numberOfRetransmissions, stream);
    PutInteger(0, 16777215, (uint32)x->sendBufferSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for NumberOfRetransmissions (CHOICE)  */
/* <================================================> */
void Encode_NumberOfRetransmissions(PS_NumberOfRetransmissions x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 16, (uint32)x->finite, stream);
            break;
        case 1:
            /* (infinite is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_NumberOfRetransmissions: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Encoder for V76LogicalChannelParameters (SEQUENCE)  */
/* <======================================================> */
void Encode_V76LogicalChannelParameters(PS_V76LogicalChannelParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_V76HDLCParameters(&x->hdlcParameters, stream);
    Encode_SuspendResume(&x->suspendResume, stream);
    PutBoolean(x->uIH, stream);
    Encode_Mode(&x->mode, stream);
    Encode_V75Parameters(&x->v75Parameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================> */
/*  PER-Encoder for Mode (CHOICE)  */
/* <=============================> */
void Encode_Mode(PS_Mode x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_ERM(x->eRM, stream);
            break;
        case 1:
            /* (uNERM is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_Mode: Illegal CHOICE index");
    }
}

/* <==============================> */
/*  PER-Encoder for ERM (SEQUENCE)  */
/* <==============================> */
void Encode_ERM(PS_ERM x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 127, (uint32)x->windowSize, stream);
    Encode_Recovery(&x->recovery, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for Recovery (CHOICE)  */
/* <=================================> */
void Encode_Recovery(PS_Recovery x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Recovery: Illegal CHOICE index");
    }
}

/* <======================================> */
/*  PER-Encoder for SuspendResume (CHOICE)  */
/* <======================================> */
void Encode_SuspendResume(PS_SuspendResume x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_SuspendResume: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Encoder for V76HDLCParameters (SEQUENCE)  */
/* <============================================> */
void Encode_V76HDLCParameters(PS_V76HDLCParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_CRCLength(&x->crcLength, stream);
    PutInteger(1, 4095, (uint32)x->n401, stream);
    PutBoolean(x->loopbackTestProcedure, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for CRCLength (CHOICE)  */
/* <==================================> */
void Encode_CRCLength(PS_CRCLength x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_CRCLength: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Encoder for H2250LogicalChannelParameters (SEQUENCE)  */
/* <========================================================> */
void Encode_H2250LogicalChannelParameters(PS_H2250LogicalChannelParameters x, PS_OutStream stream)
{
    uint16 i;
    uint32 extension;

    extension = x->option_of_transportCapability |
                x->option_of_redundancyEncoding |
                x->option_of_source |
                x->option_of_symmetricMediaAddress;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_nonStandard, stream);
    PutBoolean(x->option_of_associatedSessionID, stream);
    PutBoolean(x->option_of_mediaChannel, stream);
    PutBoolean(x->option_of_mediaGuaranteedDelivery, stream);
    PutBoolean(x->option_of_mediaControlChannel, stream);
    PutBoolean(x->option_of_mediaControlGuaranteedDelivery, stream);
    PutBoolean(x->option_of_silenceSuppression, stream);
    PutBoolean(x->option_of_destination, stream);
    PutBoolean(x->option_of_dynamicRTPPayloadType, stream);
    PutBoolean(x->option_of_mediaPacketization, stream);
    if (x->option_of_nonStandard)
    {
        PutLengthDet(x->size_of_nonStandard, stream);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Encode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    PutInteger(0, 255, (uint32)x->sessionID, stream);
    if (x->option_of_associatedSessionID)
    {
        PutInteger(1, 255, (uint32)x->associatedSessionID, stream);
    }
    if (x->option_of_mediaChannel)
    {
        Encode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        PutBoolean(x->mediaGuaranteedDelivery, stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Encode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        PutBoolean(x->mediaControlGuaranteedDelivery, stream);
    }
    if (x->option_of_silenceSuppression)
    {
        PutBoolean(x->silenceSuppression, stream);
    }
    if (x->option_of_destination)
    {
        Encode_TerminalLabel(&x->destination, stream);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        PutInteger(96, 127, (uint32)x->dynamicRTPPayloadType, stream);
    }
    if (x->option_of_mediaPacketization)
    {
        Encode_MediaPacketization(&x->mediaPacketization, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(4, stream); /* Begin Options Map */
    PutBoolean(x->option_of_transportCapability, stream);
    PutBoolean(x->option_of_redundancyEncoding, stream);
    PutBoolean(x->option_of_source, stream);
    PutBoolean(x->option_of_symmetricMediaAddress, stream);
    if (x->option_of_transportCapability)
    {
        PutExtensionItem(EPASS Encode_TransportCapability, (uint8*)&x->transportCapability, stream);
    }
    if (x->option_of_redundancyEncoding)
    {
        PutExtensionItem(EPASS Encode_RedundancyEncoding, (uint8*)&x->redundancyEncoding, stream);
    }
    if (x->option_of_source)
    {
        PutExtensionItem(EPASS Encode_TerminalLabel, (uint8*)&x->source, stream);
    }
    if (x->option_of_symmetricMediaAddress)
    {
        PutExtensionBoolean(x->symmetricMediaAddress, stream);
    }
}

/* <===========================================> */
/*  PER-Encoder for MediaPacketization (CHOICE)  */
/* <===========================================> */
void Encode_MediaPacketization(PS_MediaPacketization x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (h261aVideoPacketization is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            PutExtensionItem(EPASS Encode_RTPPayloadType, (uint8*)x->rtpPayloadType, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MediaPacketization: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for RTPPayloadType (SEQUENCE)  */
/* <=========================================> */
void Encode_RTPPayloadType(PS_RTPPayloadType x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_payloadType, stream);
    Encode_PayloadDescriptor(&x->payloadDescriptor, stream);
    if (x->option_of_payloadType)
    {
        PutInteger(0, 127, (uint32)x->payloadType, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for PayloadDescriptor (CHOICE)  */
/* <==========================================> */
void Encode_PayloadDescriptor(PS_PayloadDescriptor x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandardIdentifier, stream);
            break;
        case 1:
            PutExtendedInteger(1, 32768, (int32)x->rfc_number, stream);
            break;
        case 2:
            PutObjectID(x->oid, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_PayloadDescriptor: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for RedundancyEncoding (SEQUENCE)  */
/* <=============================================> */
void Encode_RedundancyEncoding(PS_RedundancyEncoding x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_secondaryEncoding, stream);
    Encode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    if (x->option_of_secondaryEncoding)
    {
        Encode_DataType(&x->secondaryEncoding, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for TransportAddress (CHOICE)  */
/* <=========================================> */
void Encode_TransportAddress(PS_TransportAddress x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_UnicastAddress(x->unicastAddress, stream);
            break;
        case 1:
            Encode_MulticastAddress(x->multicastAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_TransportAddress: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for UnicastAddress (CHOICE)  */
/* <=======================================> */
void Encode_UnicastAddress(PS_UnicastAddress x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_IPAddress(x->iPAddress, stream);
            break;
        case 1:
            Encode_IPXAddress(x->iPXAddress, stream);
            break;
        case 2:
            Encode_IP6Address(x->iP6Address, stream);
            break;
        case 3:
            PutOctetString(0, 16, 16, x->netBios, stream);
            break;
        case 4:
            Encode_IPSourceRouteAddress(x->iPSourceRouteAddress, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            PutExtensionOctetString(0, 1, 20, x->nsap, stream);
            break;
        case 6:
            PutExtensionItem(EPASS Encode_NonStandardParameter, (uint8*)x->nonStandardAddress, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_UnicastAddress: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Encoder for IPSourceRouteAddress (SEQUENCE)  */
/* <===============================================> */
void Encode_IPSourceRouteAddress(PS_IPSourceRouteAddress x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Routing(&x->routing, stream);
    PutOctetString(0, 4, 4, &x->network, stream);
    PutInteger(0, 65535, (uint32)x->tsapIdentifier, stream);
    PutLengthDet(x->size_of_route, stream);
    for (i = 0;i < x->size_of_route;++i)
    {
        PutOctetString(0, 4, 4, x->route + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Encoder for Routing (CHOICE)  */
/* <================================> */
void Encode_Routing(PS_Routing x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (strict is NULL) */
            break;
        case 1:
            /* (loose is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_Routing: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for IP6Address (SEQUENCE)  */
/* <=====================================> */
void Encode_IP6Address(PS_IP6Address x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 16, 16, &x->network, stream);
    PutInteger(0, 65535, (uint32)x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for IPXAddress (SEQUENCE)  */
/* <=====================================> */
void Encode_IPXAddress(PS_IPXAddress x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 6, 6, &x->node, stream);
    PutOctetString(0, 4, 4, &x->netnum, stream);
    PutOctetString(0, 2, 2, &x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Encoder for IPAddress (SEQUENCE)  */
/* <====================================> */
void Encode_IPAddress(PS_IPAddress x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 4, 4, &x->network, stream);
    PutInteger(0, 65535, (uint32)x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for MulticastAddress (CHOICE)  */
/* <=========================================> */
void Encode_MulticastAddress(PS_MulticastAddress x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_MaIpAddress(x->maIpAddress, stream);
            break;
        case 1:
            Encode_MaIp6Address(x->maIp6Address, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            PutExtensionOctetString(0, 1, 20, x->nsap, stream);
            break;
        case 3:
            PutExtensionItem(EPASS Encode_NonStandardParameter, (uint8*)x->nonStandardAddress, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MulticastAddress: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for MaIp6Address (SEQUENCE)  */
/* <=======================================> */
void Encode_MaIp6Address(PS_MaIp6Address x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 16, 16, &x->network, stream);
    PutInteger(0, 65535, (uint32)x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for MaIpAddress (SEQUENCE)  */
/* <======================================> */
void Encode_MaIpAddress(PS_MaIpAddress x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 4, 4, &x->network, stream);
    PutInteger(0, 65535, (uint32)x->tsapIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for EncryptionSync (SEQUENCE)  */
/* <=========================================> */
void Encode_EncryptionSync(PS_EncryptionSync x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_nonStandard, stream);
    PutBoolean(x->option_of_escrowentry, stream);
    if (x->option_of_nonStandard)
    {
        Encode_NonStandardParameter(&x->nonStandard, stream);
    }
    PutInteger(0, 255, (uint32)x->synchFlag, stream);
    PutOctetString(0, 1, 65535, &x->h235Key, stream);
    if (x->option_of_escrowentry)
    {
        PutInteger(1, 256, (uint32)x->size_of_escrowentry, stream);
        for (i = 0;i < x->size_of_escrowentry;++i)
        {
            Encode_EscrowData(x->escrowentry + i, stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for EscrowData (SEQUENCE)  */
/* <=====================================> */
void Encode_EscrowData(PS_EscrowData x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutObjectID(&x->escrowID, stream);
    PutBitString(0, 1, 65535, &x->escrowValue, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for OpenLogicalChannelAck (SEQUENCE)  */
/* <================================================> */
void Encode_OpenLogicalChannelAck(PS_OpenLogicalChannelAck x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_separateStack |
                x->option_of_forwardMultiplexAckParameters |
                x->option_of_encryptionSync;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_ackReverseLogicalChannelParameters, stream);
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    if (x->option_of_ackReverseLogicalChannelParameters)
    {
        Encode_AckReverseLogicalChannelParameters(&x->ackReverseLogicalChannelParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(3, stream); /* Begin Options Map */
    PutBoolean(x->option_of_separateStack, stream);
    PutBoolean(x->option_of_forwardMultiplexAckParameters, stream);
    PutBoolean(x->option_of_encryptionSync, stream);
    if (x->option_of_separateStack)
    {
        PutExtensionItem(EPASS Encode_NetworkAccessParameters, (uint8*)&x->separateStack, stream);
    }
    if (x->option_of_forwardMultiplexAckParameters)
    {
        PutExtensionItem(EPASS Encode_ForwardMultiplexAckParameters, (uint8*)&x->forwardMultiplexAckParameters, stream);
    }
    if (x->option_of_encryptionSync)
    {
        PutExtensionItem(EPASS Encode_EncryptionSync, (uint8*)&x->encryptionSync, stream);
    }
}

/* <======================================================> */
/*  PER-Encoder for ForwardMultiplexAckParameters (CHOICE)  */
/* <======================================================> */
void Encode_ForwardMultiplexAckParameters(PS_ForwardMultiplexAckParameters x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_H2250LogicalChannelAckParameters(x->h2250LogicalChannelAckParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_ForwardMultiplexAckParameters: Illegal CHOICE index");
    }
}

/* <=============================================================> */
/*  PER-Encoder for AckReverseLogicalChannelParameters (SEQUENCE)  */
/* <=============================================================> */
void Encode_AckReverseLogicalChannelParameters(PS_AckReverseLogicalChannelParameters x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_replacementFor;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_portNumber, stream);
    PutBoolean(x->option_of_ackMultiplexParameters, stream);
    PutInteger(1, 65535, (uint32)x->reverseLogicalChannelNumber, stream);
    if (x->option_of_portNumber)
    {
        PutInteger(0, 65535, (uint32)x->portNumber, stream);
    }
    if (x->option_of_ackMultiplexParameters)
    {
        Encode_AckMultiplexParameters(&x->ackMultiplexParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_replacementFor, stream);
    if (x->option_of_replacementFor)
    {
        PutExtensionInteger(1, 65535, (uint32)x->replacementFor, stream);
    }
}

/* <===============================================> */
/*  PER-Encoder for AckMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Encode_AckMultiplexParameters(PS_AckMultiplexParameters x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_H222LogicalChannelParameters(x->h222LogicalChannelParameters, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            PutExtensionItem(EPASS Encode_H2250LogicalChannelParameters, (uint8*)x->h2250LogicalChannelParameters, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_AckMultiplexParameters: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Encoder for OpenLogicalChannelReject (SEQUENCE)  */
/* <===================================================> */
void Encode_OpenLogicalChannelReject(PS_OpenLogicalChannelReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    Encode_OlcRejectCause(&x->olcRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for OlcRejectCause (CHOICE)  */
/* <=======================================> */
void Encode_OlcRejectCause(PS_OlcRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(6, 1, x->index, stream);
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
            PutExtensionNull(stream);
            break;
        case 7:
            PutExtensionNull(stream);
            break;
        case 8:
            PutExtensionNull(stream);
            break;
        case 9:
            PutExtensionNull(stream);
            break;
        case 10:
            PutExtensionNull(stream);
            break;
        case 11:
            PutExtensionNull(stream);
            break;
        case 12:
            PutExtensionNull(stream);
            break;
        case 13:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_OlcRejectCause: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Encoder for OpenLogicalChannelConfirm (SEQUENCE)  */
/* <====================================================> */
void Encode_OpenLogicalChannelConfirm(PS_OpenLogicalChannelConfirm x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================================> */
/*  PER-Encoder for H2250LogicalChannelAckParameters (SEQUENCE)  */
/* <===========================================================> */
void Encode_H2250LogicalChannelAckParameters(PS_H2250LogicalChannelAckParameters x, PS_OutStream stream)
{
    uint16 i;
    uint32 extension;

    extension = x->option_of_flowControlToZero |
                x->option_of_portNumber;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_nonStandard, stream);
    PutBoolean(x->option_of_sessionID, stream);
    PutBoolean(x->option_of_mediaChannel, stream);
    PutBoolean(x->option_of_mediaControlChannel, stream);
    PutBoolean(x->option_of_dynamicRTPPayloadType, stream);
    if (x->option_of_nonStandard)
    {
        PutLengthDet(x->size_of_nonStandard, stream);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Encode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    if (x->option_of_sessionID)
    {
        PutInteger(1, 255, (uint32)x->sessionID, stream);
    }
    if (x->option_of_mediaChannel)
    {
        Encode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Encode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
        PutInteger(96, 127, (uint32)x->dynamicRTPPayloadType, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_flowControlToZero, stream);
    PutBoolean(x->option_of_portNumber, stream);
    if (x->option_of_flowControlToZero)
    {
        PutExtensionBoolean(x->flowControlToZero, stream);
    }
    if (x->option_of_portNumber)
    {
        PutExtensionInteger(0, 65535, (uint32)x->portNumber, stream);
    }
}

/* <==============================================> */
/*  PER-Encoder for CloseLogicalChannel (SEQUENCE)  */
/* <==============================================> */
void Encode_CloseLogicalChannel(PS_CloseLogicalChannel x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_reason;
    PutBoolean(extension, stream);

    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    Encode_Source(&x->source, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_reason, stream);
    if (x->option_of_reason)
    {
        PutExtensionItem(EPASS Encode_Reason, (uint8*)&x->reason, stream);
    }
}

/* <===============================> */
/*  PER-Encoder for Reason (CHOICE)  */
/* <===============================> */
void Encode_Reason(PS_Reason x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Reason: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Encoder for Source (CHOICE)  */
/* <===============================> */
void Encode_Source(PS_Source x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (user is NULL) */
            break;
        case 1:
            /* (lcse is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_Source: Illegal CHOICE index");
    }
}

/* <=================================================> */
/*  PER-Encoder for CloseLogicalChannelAck (SEQUENCE)  */
/* <=================================================> */
void Encode_CloseLogicalChannelAck(PS_CloseLogicalChannelAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for RequestChannelClose (SEQUENCE)  */
/* <==============================================> */
void Encode_RequestChannelClose(PS_RequestChannelClose x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_qosCapability |
                x->option_of_rccReason;
    PutBoolean(extension, stream);

    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(2, stream); /* Begin Options Map */
    PutBoolean(x->option_of_qosCapability, stream);
    PutBoolean(x->option_of_rccReason, stream);
    if (x->option_of_qosCapability)
    {
        PutExtensionItem(EPASS Encode_QOSCapability, (uint8*)&x->qosCapability, stream);
    }
    if (x->option_of_rccReason)
    {
        PutExtensionItem(EPASS Encode_RccReason, (uint8*)&x->rccReason, stream);
    }
}

/* <==================================> */
/*  PER-Encoder for RccReason (CHOICE)  */
/* <==================================> */
void Encode_RccReason(PS_RccReason x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_RccReason: Illegal CHOICE index");
    }
}

/* <=================================================> */
/*  PER-Encoder for RequestChannelCloseAck (SEQUENCE)  */
/* <=================================================> */
void Encode_RequestChannelCloseAck(PS_RequestChannelCloseAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Encoder for RequestChannelCloseReject (SEQUENCE)  */
/* <====================================================> */
void Encode_RequestChannelCloseReject(PS_RequestChannelCloseReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    Encode_RccRejectCause(&x->rccRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for RccRejectCause (CHOICE)  */
/* <=======================================> */
void Encode_RccRejectCause(PS_RccRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_RccRejectCause: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Encoder for RequestChannelCloseRelease (SEQUENCE)  */
/* <=====================================================> */
void Encode_RequestChannelCloseRelease(PS_RequestChannelCloseRelease x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->forwardLogicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for MultiplexEntrySend (SEQUENCE)  */
/* <=============================================> */
void Encode_MultiplexEntrySend(PS_MultiplexEntrySend x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 15, (uint32)x->size_of_multiplexEntryDescriptors, stream);
    for (i = 0;i < x->size_of_multiplexEntryDescriptors;++i)
    {
        Encode_MultiplexEntryDescriptor(x->multiplexEntryDescriptors + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for MultiplexEntryDescriptor (SEQUENCE)  */
/* <===================================================> */
void Encode_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(x->option_of_elementList, stream);
    PutInteger(1, 15, (uint32)x->multiplexTableEntryNumber, stream);
    if (x->option_of_elementList)
    {
        PutInteger(1, 256, (uint32)x->size_of_elementList, stream);
        for (i = 0;i < x->size_of_elementList;++i)
        {
            Encode_MultiplexElement(x->elementList + i, stream);
        }
    }
}

/* <===========================================> */
/*  PER-Encoder for MultiplexElement (SEQUENCE)  */
/* <===========================================> */
void Encode_MultiplexElement(PS_MultiplexElement x, PS_OutStream stream)
{
    Encode_MuxType(&x->muxType, stream);
    Encode_RepeatCount(&x->repeatCount, stream);
}

/* <====================================> */
/*  PER-Encoder for RepeatCount (CHOICE)  */
/* <====================================> */
void Encode_RepeatCount(PS_RepeatCount x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->finite, stream);
            break;
        case 1:
            /* (untilClosingFlag is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_RepeatCount: Illegal CHOICE index");
    }
}

/* <================================> */
/*  PER-Encoder for MuxType (CHOICE)  */
/* <================================> */
void Encode_MuxType(PS_MuxType x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 65535, (uint32)x->logicalChannelNumber, stream);
            break;
        case 1:
            PutInteger(2, 255, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_MultiplexElement(x->subElementList + i, stream);
            }
            break;
        default:
            ErrorMessageAndLeave("Encode_MuxType: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for MultiplexEntrySendAck (SEQUENCE)  */
/* <================================================> */
void Encode_MultiplexEntrySendAck(PS_MultiplexEntrySendAck x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 15, (uint32)x->size_of_multiplexTableEntryNumber, stream);
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        PutInteger(1, 15, (uint32)x->multiplexTableEntryNumber[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for MultiplexEntrySendReject (SEQUENCE)  */
/* <===================================================> */
void Encode_MultiplexEntrySendReject(PS_MultiplexEntrySendReject x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 15, (uint32)x->size_of_rejectionDescriptions, stream);
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Encode_MultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================================> */
/*  PER-Encoder for MultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <==============================================================> */
void Encode_MultiplexEntryRejectionDescriptions(PS_MultiplexEntryRejectionDescriptions x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->multiplexTableEntryNumber, stream);
    Encode_MeRejectCause(&x->meRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for MeRejectCause (CHOICE)  */
/* <======================================> */
void Encode_MeRejectCause(PS_MeRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_MeRejectCause: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Encoder for MultiplexEntrySendRelease (SEQUENCE)  */
/* <====================================================> */
void Encode_MultiplexEntrySendRelease(PS_MultiplexEntrySendRelease x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->size_of_multiplexTableEntryNumber, stream);
    for (i = 0;i < x->size_of_multiplexTableEntryNumber;++i)
    {
        PutInteger(1, 15, (uint32)x->multiplexTableEntryNumber[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for RequestMultiplexEntry (SEQUENCE)  */
/* <================================================> */
void Encode_RequestMultiplexEntry(PS_RequestMultiplexEntry x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->size_of_entryNumbers, stream);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        PutInteger(1, 15, (uint32)x->entryNumbers[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for RequestMultiplexEntryAck (SEQUENCE)  */
/* <===================================================> */
void Encode_RequestMultiplexEntryAck(PS_RequestMultiplexEntryAck x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->size_of_entryNumbers, stream);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        PutInteger(1, 15, (uint32)x->entryNumbers[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for RequestMultiplexEntryReject (SEQUENCE)  */
/* <======================================================> */
void Encode_RequestMultiplexEntryReject(PS_RequestMultiplexEntryReject x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->size_of_entryNumbers, stream);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        PutInteger(1, 15, (uint32)x->entryNumbers[i], stream);
    }
    PutInteger(1, 15, (uint32)x->size_of_rejectionDescriptions, stream);
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Encode_RequestMultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================================> */
/*  PER-Encoder for RequestMultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <=====================================================================> */
void Encode_RequestMultiplexEntryRejectionDescriptions(PS_RequestMultiplexEntryRejectionDescriptions x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->multiplexTableEntryNumber, stream);
    Encode_RmeRejectCause(&x->rmeRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for RmeRejectCause (CHOICE)  */
/* <=======================================> */
void Encode_RmeRejectCause(PS_RmeRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (unspecifiedCause is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_RmeRejectCause: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Encoder for RequestMultiplexEntryRelease (SEQUENCE)  */
/* <=======================================================> */
void Encode_RequestMultiplexEntryRelease(PS_RequestMultiplexEntryRelease x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 15, (uint32)x->size_of_entryNumbers, stream);
    for (i = 0;i < x->size_of_entryNumbers;++i)
    {
        PutInteger(1, 15, (uint32)x->entryNumbers[i], stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for RequestMode (SEQUENCE)  */
/* <======================================> */
void Encode_RequestMode(PS_RequestMode x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 256, (uint32)x->size_of_requestedModes, stream);
    for (i = 0;i < x->size_of_requestedModes;++i)
    {
        Encode_ModeDescription(x->requestedModes + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for RequestModeAck (SEQUENCE)  */
/* <=========================================> */
void Encode_RequestModeAck(PS_RequestModeAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    Encode_Response(&x->response, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for Response (CHOICE)  */
/* <=================================> */
void Encode_Response(PS_Response x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Response: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Encoder for RequestModeReject (SEQUENCE)  */
/* <============================================> */
void Encode_RequestModeReject(PS_RequestModeReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    Encode_RmRejectCause(&x->rmRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for RmRejectCause (CHOICE)  */
/* <======================================> */
void Encode_RmRejectCause(PS_RmRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_RmRejectCause: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for RequestModeRelease (SEQUENCE)  */
/* <=============================================> */
void Encode_RequestModeRelease(PS_RequestModeRelease x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for ModeDescription (SET-OF)  */
/* <========================================> */
void Encode_ModeDescription(PS_ModeDescription x, PS_OutStream stream)
{
    uint16 i;
    PutInteger(1, 256, (uint32)x->size, stream);
    for (i = 0;i < x->size;++i)
    {
        Encode_ModeElement(x->item + i, stream);
    }
}

/* <======================================> */
/*  PER-Encoder for ModeElement (SEQUENCE)  */
/* <======================================> */
void Encode_ModeElement(PS_ModeElement x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_v76ModeParameters |
                x->option_of_h2250ModeParameters |
                x->option_of_genericModeParameters;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_h223ModeParameters, stream);
    Encode_ModeType(&x->modeType, stream);
    if (x->option_of_h223ModeParameters)
    {
        Encode_H223ModeParameters(&x->h223ModeParameters, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(3, stream); /* Begin Options Map */
    PutBoolean(x->option_of_v76ModeParameters, stream);
    PutBoolean(x->option_of_h2250ModeParameters, stream);
    PutBoolean(x->option_of_genericModeParameters, stream);
    if (x->option_of_v76ModeParameters)
    {
        PutExtensionItem(EPASS Encode_V76ModeParameters, (uint8*)&x->v76ModeParameters, stream);
    }
    if (x->option_of_h2250ModeParameters)
    {
        PutExtensionItem(EPASS Encode_H2250ModeParameters, (uint8*)&x->h2250ModeParameters, stream);
    }
    if (x->option_of_genericModeParameters)
    {
        PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)&x->genericModeParameters, stream);
    }
}

/* <=================================> */
/*  PER-Encoder for ModeType (CHOICE)  */
/* <=================================> */
void Encode_ModeType(PS_ModeType x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_VideoMode(x->videoMode, stream);
            break;
        case 2:
            Encode_AudioMode(x->audioMode, stream);
            break;
        case 3:
            Encode_DataMode(x->dataMode, stream);
            break;
        case 4:
            Encode_EncryptionMode(x->encryptionMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            PutExtensionItem(EPASS Encode_H235Mode, (uint8*)x->h235Mode, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ModeType: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Encoder for H235Mode (SEQUENCE)  */
/* <===================================> */
void Encode_H235Mode(PS_H235Mode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity, stream);
    Encode_MediaMode(&x->mediaMode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for MediaMode (CHOICE)  */
/* <==================================> */
void Encode_MediaMode(PS_MediaMode x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_VideoMode(x->videoMode, stream);
            break;
        case 2:
            Encode_AudioMode(x->audioMode, stream);
            break;
        case 3:
            Encode_DataMode(x->dataMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MediaMode: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for H223ModeParameters (SEQUENCE)  */
/* <=============================================> */
void Encode_H223ModeParameters(PS_H223ModeParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_ModeAdaptationLayerType(&x->modeAdaptationLayerType, stream);
    PutBoolean(x->segmentableFlag, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for ModeAdaptationLayerType (CHOICE)  */
/* <================================================> */
void Encode_ModeAdaptationLayerType(PS_ModeAdaptationLayerType x, PS_OutStream stream)
{
    PutChoiceIndex(6, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
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
            Encode_ModeAl3(x->modeAl3, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            PutExtensionItem(EPASS Encode_H223AL1MParameters, (uint8*)x->al1M, stream);
            break;
        case 7:
            PutExtensionItem(EPASS Encode_H223AL2MParameters, (uint8*)x->al2M, stream);
            break;
        case 8:
            PutExtensionItem(EPASS Encode_H223AL3MParameters, (uint8*)x->al3M, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ModeAdaptationLayerType: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for ModeAl3 (SEQUENCE)  */
/* <==================================> */
void Encode_ModeAl3(PS_ModeAl3 x, PS_OutStream stream)
{
    PutInteger(0, 2, (uint32)x->controlFieldOctets, stream);
    PutInteger(0, 16777215, (uint32)x->sendBufferSize, stream);
}

/* <==========================================> */
/*  PER-Encoder for V76ModeParameters (CHOICE)  */
/* <==========================================> */
void Encode_V76ModeParameters(PS_V76ModeParameters x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_V76ModeParameters: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Encoder for H2250ModeParameters (SEQUENCE)  */
/* <==============================================> */
void Encode_H2250ModeParameters(PS_H2250ModeParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_redundancyEncodingMode, stream);
    if (x->option_of_redundancyEncodingMode)
    {
        Encode_RedundancyEncodingMode(&x->redundancyEncodingMode, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for RedundancyEncodingMode (SEQUENCE)  */
/* <=================================================> */
void Encode_RedundancyEncodingMode(PS_RedundancyEncodingMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_secondaryEncoding, stream);
    Encode_RedundancyEncodingMethod(&x->redundancyEncodingMethod, stream);
    if (x->option_of_secondaryEncoding)
    {
        Encode_SecondaryEncoding(&x->secondaryEncoding, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for SecondaryEncoding (CHOICE)  */
/* <==========================================> */
void Encode_SecondaryEncoding(PS_SecondaryEncoding x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_AudioMode(x->audioData, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_SecondaryEncoding: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for VideoMode (CHOICE)  */
/* <==================================> */
void Encode_VideoMode(PS_VideoMode x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_H261VideoMode(x->h261VideoMode, stream);
            break;
        case 2:
            Encode_H262VideoMode(x->h262VideoMode, stream);
            break;
        case 3:
            Encode_H263VideoMode(x->h263VideoMode, stream);
            break;
        case 4:
            Encode_IS11172VideoMode(x->is11172VideoMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericVideoMode, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_VideoMode: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for H261VideoMode (SEQUENCE)  */
/* <========================================> */
void Encode_H261VideoMode(PS_H261VideoMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Resolution(&x->resolution, stream);
    PutInteger(1, 19200, (uint32)x->bitRate, stream);
    PutBoolean(x->stillImageTransmission, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Encoder for Resolution (CHOICE)  */
/* <===================================> */
void Encode_Resolution(PS_Resolution x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (qcif is NULL) */
            break;
        case 1:
            /* (cif is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_Resolution: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for H262VideoMode (SEQUENCE)  */
/* <========================================> */
void Encode_H262VideoMode(PS_H262VideoMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_videoBitRate, stream);
    PutBoolean(x->option_of_vbvBufferSize, stream);
    PutBoolean(x->option_of_samplesPerLine, stream);
    PutBoolean(x->option_of_linesPerFrame, stream);
    PutBoolean(x->option_of_framesPerSecond, stream);
    PutBoolean(x->option_of_luminanceSampleRate, stream);
    Encode_ProfileAndLevel(&x->profileAndLevel, stream);
    if (x->option_of_videoBitRate)
    {
        PutInteger(0, 1073741823, (uint32)x->videoBitRate, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        PutInteger(0, 262143, (uint32)x->vbvBufferSize, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        PutInteger(0, 16383, (uint32)x->samplesPerLine, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        PutInteger(0, 16383, (uint32)x->linesPerFrame, stream);
    }
    if (x->option_of_framesPerSecond)
    {
        PutInteger(0, 15, (uint32)x->framesPerSecond, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->luminanceSampleRate, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for ProfileAndLevel (CHOICE)  */
/* <========================================> */
void Encode_ProfileAndLevel(PS_ProfileAndLevel x, PS_OutStream stream)
{
    PutChoiceIndex(11, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_ProfileAndLevel: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Encoder for H263VideoMode (SEQUENCE)  */
/* <========================================> */
void Encode_H263VideoMode(PS_H263VideoMode x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_errorCompensation |
                x->option_of_enhancementLayerInfo |
                x->option_of_h263Options;
    PutBoolean(extension, stream);

    Encode_H263Resolution(&x->h263Resolution, stream);
    PutInteger(1, 19200, (uint32)x->bitRate, stream);
    PutBoolean(x->unrestrictedVector, stream);
    PutBoolean(x->arithmeticCoding, stream);
    PutBoolean(x->advancedPrediction, stream);
    PutBoolean(x->pbFrames, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(3, stream); /* Begin Options Map */
    PutBoolean(x->option_of_errorCompensation, stream);
    PutBoolean(x->option_of_enhancementLayerInfo, stream);
    PutBoolean(x->option_of_h263Options, stream);
    if (x->option_of_errorCompensation)
    {
        PutExtensionBoolean(x->errorCompensation, stream);
    }
    if (x->option_of_enhancementLayerInfo)
    {
        PutExtensionItem(EPASS Encode_EnhancementLayerInfo, (uint8*)&x->enhancementLayerInfo, stream);
    }
    if (x->option_of_h263Options)
    {
        PutExtensionItem(EPASS Encode_H263Options, (uint8*)&x->h263Options, stream);
    }
}

/* <=======================================> */
/*  PER-Encoder for H263Resolution (CHOICE)  */
/* <=======================================> */
void Encode_H263Resolution(PS_H263Resolution x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_H263Resolution: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for IS11172VideoMode (SEQUENCE)  */
/* <===========================================> */
void Encode_IS11172VideoMode(PS_IS11172VideoMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_videoBitRate, stream);
    PutBoolean(x->option_of_vbvBufferSize, stream);
    PutBoolean(x->option_of_samplesPerLine, stream);
    PutBoolean(x->option_of_linesPerFrame, stream);
    PutBoolean(x->option_of_pictureRate, stream);
    PutBoolean(x->option_of_luminanceSampleRate, stream);
    PutBoolean(x->constrainedBitstream, stream);
    if (x->option_of_videoBitRate)
    {
        PutInteger(0, 1073741823, (uint32)x->videoBitRate, stream);
    }
    if (x->option_of_vbvBufferSize)
    {
        PutInteger(0, 262143, (uint32)x->vbvBufferSize, stream);
    }
    if (x->option_of_samplesPerLine)
    {
        PutInteger(0, 16383, (uint32)x->samplesPerLine, stream);
    }
    if (x->option_of_linesPerFrame)
    {
        PutInteger(0, 16383, (uint32)x->linesPerFrame, stream);
    }
    if (x->option_of_pictureRate)
    {
        PutInteger(0, 15, (uint32)x->pictureRate, stream);
    }
    if (x->option_of_luminanceSampleRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->luminanceSampleRate, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for AudioMode (CHOICE)  */
/* <==================================> */
void Encode_AudioMode(PS_AudioMode x, PS_OutStream stream)
{
    PutChoiceIndex(14, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
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
            Encode_ModeG7231(x->modeG7231, stream);
            break;
        case 12:
            Encode_IS11172AudioMode(x->is11172AudioMode, stream);
            break;
        case 13:
            Encode_IS13818AudioMode(x->is13818AudioMode, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            PutExtensionInteger(1, 256, (uint32)x->g729wAnnexB, stream);
            break;
        case 15:
            PutExtensionInteger(1, 256, (uint32)x->g729AnnexAwAnnexB, stream);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_G7231AnnexCMode, (uint8*)x->g7231AnnexCMode, stream);
            break;
        case 17:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmFullRate, stream);
            break;
        case 18:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmHalfRate, stream);
            break;
        case 19:
            PutExtensionItem(EPASS Encode_GSMAudioCapability, (uint8*)x->gsmEnhancedFullRate, stream);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericAudioMode, stream);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_G729Extensions, (uint8*)x->g729Extensions, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_AudioMode: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for ModeG7231 (CHOICE)  */
/* <==================================> */
void Encode_ModeG7231(PS_ModeG7231 x, PS_OutStream stream)
{
    PutChoiceIndex(4, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_ModeG7231: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for IS11172AudioMode (SEQUENCE)  */
/* <===========================================> */
void Encode_IS11172AudioMode(PS_IS11172AudioMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_AudioLayer(&x->audioLayer, stream);
    Encode_AudioSampling(&x->audioSampling, stream);
    Encode_MultichannelType(&x->multichannelType, stream);
    PutInteger(1, 448, (uint32)x->bitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Encoder for MultichannelType (CHOICE)  */
/* <=========================================> */
void Encode_MultichannelType(PS_MultichannelType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_MultichannelType: Illegal CHOICE index");
    }
}

/* <======================================> */
/*  PER-Encoder for AudioSampling (CHOICE)  */
/* <======================================> */
void Encode_AudioSampling(PS_AudioSampling x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_AudioSampling: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Encoder for AudioLayer (CHOICE)  */
/* <===================================> */
void Encode_AudioLayer(PS_AudioLayer x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_AudioLayer: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for IS13818AudioMode (SEQUENCE)  */
/* <===========================================> */
void Encode_IS13818AudioMode(PS_IS13818AudioMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_Is13818AudioLayer(&x->is13818AudioLayer, stream);
    Encode_Is13818AudioSampling(&x->is13818AudioSampling, stream);
    Encode_Is13818MultichannelType(&x->is13818MultichannelType, stream);
    PutBoolean(x->lowFrequencyEnhancement, stream);
    PutBoolean(x->multilingual, stream);
    PutInteger(1, 1130, (uint32)x->bitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Encoder for Is13818MultichannelType (CHOICE)  */
/* <================================================> */
void Encode_Is13818MultichannelType(PS_Is13818MultichannelType x, PS_OutStream stream)
{
    PutChoiceIndex(10, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Is13818MultichannelType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for Is13818AudioSampling (CHOICE)  */
/* <=============================================> */
void Encode_Is13818AudioSampling(PS_Is13818AudioSampling x, PS_OutStream stream)
{
    PutChoiceIndex(6, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Is13818AudioSampling: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for Is13818AudioLayer (CHOICE)  */
/* <==========================================> */
void Encode_Is13818AudioLayer(PS_Is13818AudioLayer x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Is13818AudioLayer: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for G7231AnnexCMode (SEQUENCE)  */
/* <==========================================> */
void Encode_G7231AnnexCMode(PS_G7231AnnexCMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->maxAl_sduAudioFrames, stream);
    PutBoolean(x->silenceSuppression, stream);
    Encode_ModeG723AnnexCAudioMode(&x->modeG723AnnexCAudioMode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for ModeG723AnnexCAudioMode (SEQUENCE)  */
/* <==================================================> */
void Encode_ModeG723AnnexCAudioMode(PS_ModeG723AnnexCAudioMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(27, 78, (uint32)x->highRateMode0, stream);
    PutInteger(27, 78, (uint32)x->highRateMode1, stream);
    PutInteger(23, 66, (uint32)x->lowRateMode0, stream);
    PutInteger(23, 66, (uint32)x->lowRateMode1, stream);
    PutInteger(6, 17, (uint32)x->sidMode0, stream);
    PutInteger(6, 17, (uint32)x->sidMode1, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Encoder for DataMode (SEQUENCE)  */
/* <===================================> */
void Encode_DataMode(PS_DataMode x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_DmApplication(&x->dmApplication, stream);
    PutInteger(0, 0xffffffff, (uint32)x->bitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for DmApplication (CHOICE)  */
/* <======================================> */
void Encode_DmApplication(PS_DmApplication x, PS_OutStream stream)
{
    PutChoiceIndex(10, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            Encode_DataProtocolCapability(x->t120, stream);
            break;
        case 2:
            Encode_DataProtocolCapability(x->dsm_cc, stream);
            break;
        case 3:
            Encode_DataProtocolCapability(x->userData, stream);
            break;
        case 4:
            Encode_DataProtocolCapability(x->t84, stream);
            break;
        case 5:
            Encode_DataProtocolCapability(x->t434, stream);
            break;
        case 6:
            Encode_DataProtocolCapability(x->h224, stream);
            break;
        case 7:
            Encode_DmNlpid(x->dmNlpid, stream);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            Encode_DataProtocolCapability(x->h222DataPartitioning, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            PutExtensionItem(EPASS Encode_DataProtocolCapability, (uint8*)x->t30fax, stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_DataProtocolCapability, (uint8*)x->t140, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_DmT38fax, (uint8*)x->dmT38fax, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_GenericCapability, (uint8*)x->genericDataMode, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_DmApplication: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Encoder for DmT38fax (SEQUENCE)  */
/* <===================================> */
void Encode_DmT38fax(PS_DmT38fax x, PS_OutStream stream)
{
    Encode_DataProtocolCapability(&x->t38FaxProtocol, stream);
    Encode_T38FaxProfile(&x->t38FaxProfile, stream);
}

/* <==================================> */
/*  PER-Encoder for DmNlpid (SEQUENCE)  */
/* <==================================> */
void Encode_DmNlpid(PS_DmNlpid x, PS_OutStream stream)
{
    Encode_DataProtocolCapability(&x->nlpidProtocol, stream);
    PutOctetString(1, 0, 0, &x->nlpidData, stream);
}

/* <=======================================> */
/*  PER-Encoder for EncryptionMode (CHOICE)  */
/* <=======================================> */
void Encode_EncryptionMode(PS_EncryptionMode x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (h233Encryption is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_EncryptionMode: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for RoundTripDelayRequest (SEQUENCE)  */
/* <================================================> */
void Encode_RoundTripDelayRequest(PS_RoundTripDelayRequest x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for RoundTripDelayResponse (SEQUENCE)  */
/* <=================================================> */
void Encode_RoundTripDelayResponse(PS_RoundTripDelayResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Encoder for MaintenanceLoopRequest (SEQUENCE)  */
/* <=================================================> */
void Encode_MaintenanceLoopRequest(PS_MaintenanceLoopRequest x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_MlRequestType(&x->mlRequestType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for MlRequestType (CHOICE)  */
/* <======================================> */
void Encode_MlRequestType(PS_MlRequestType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            PutInteger(1, 65535, (uint32)x->mediaLoop, stream);
            break;
        case 2:
            PutInteger(1, 65535, (uint32)x->logicalChannelLoop, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MlRequestType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for MaintenanceLoopAck (SEQUENCE)  */
/* <=============================================> */
void Encode_MaintenanceLoopAck(PS_MaintenanceLoopAck x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_MlAckType(&x->mlAckType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for MlAckType (CHOICE)  */
/* <==================================> */
void Encode_MlAckType(PS_MlAckType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            PutInteger(1, 65535, (uint32)x->mediaLoop, stream);
            break;
        case 2:
            PutInteger(1, 65535, (uint32)x->logicalChannelLoop, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MlAckType: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for MaintenanceLoopReject (SEQUENCE)  */
/* <================================================> */
void Encode_MaintenanceLoopReject(PS_MaintenanceLoopReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_MlRejectType(&x->mlRejectType, stream);
    Encode_MlRejectCause(&x->mlRejectCause, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Encoder for MlRejectCause (CHOICE)  */
/* <======================================> */
void Encode_MlRejectCause(PS_MlRejectCause x, PS_OutStream stream)
{
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (canNotPerformLoop is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MlRejectCause: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for MlRejectType (CHOICE)  */
/* <=====================================> */
void Encode_MlRejectType(PS_MlRejectType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            PutInteger(1, 65535, (uint32)x->mediaLoop, stream);
            break;
        case 2:
            PutInteger(1, 65535, (uint32)x->logicalChannelLoop, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MlRejectType: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Encoder for MaintenanceLoopOffCommand (SEQUENCE)  */
/* <====================================================> */
void Encode_MaintenanceLoopOffCommand(PS_MaintenanceLoopOffCommand x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for CommunicationModeCommand (SEQUENCE)  */
/* <===================================================> */
void Encode_CommunicationModeCommand(PS_CommunicationModeCommand x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 256, (uint32)x->size_of_communicationModeTable, stream);
    for (i = 0;i < x->size_of_communicationModeTable;++i)
    {
        Encode_CommunicationModeTableEntry(x->communicationModeTable + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for CommunicationModeRequest (SEQUENCE)  */
/* <===================================================> */
void Encode_CommunicationModeRequest(PS_CommunicationModeRequest x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for CommunicationModeResponse (CHOICE)  */
/* <==================================================> */
void Encode_CommunicationModeResponse(PS_CommunicationModeResponse x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(1, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 256, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_CommunicationModeTableEntry(x->communicationModeTable + i, stream);
            }
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_CommunicationModeResponse: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Encoder for CommunicationModeTableEntry (SEQUENCE)  */
/* <======================================================> */
void Encode_CommunicationModeTableEntry(PS_CommunicationModeTableEntry x, PS_OutStream stream)
{
    uint16 i;
    uint32 extension;

    extension = x->option_of_redundancyEncoding |
                x->option_of_sessionDependency |
                x->option_of_destination;
    PutBoolean(extension, stream);

    PutBoolean(x->option_of_nonStandard, stream);
    PutBoolean(x->option_of_associatedSessionID, stream);
    PutBoolean(x->option_of_terminalLabel, stream);
    PutBoolean(x->option_of_mediaChannel, stream);
    PutBoolean(x->option_of_mediaGuaranteedDelivery, stream);
    PutBoolean(x->option_of_mediaControlChannel, stream);
    PutBoolean(x->option_of_mediaControlGuaranteedDelivery, stream);
    if (x->option_of_nonStandard)
    {
        PutLengthDet(x->size_of_nonStandard, stream);
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Encode_NonStandardParameter(x->nonStandard + i, stream);
        }
    }
    PutInteger(1, 255, (uint32)x->sessionID, stream);
    if (x->option_of_associatedSessionID)
    {
        PutInteger(1, 255, (uint32)x->associatedSessionID, stream);
    }
    if (x->option_of_terminalLabel)
    {
        Encode_TerminalLabel(&x->terminalLabel, stream);
    }
    PutCharString("BMPString", 0, 1, 128, NULL, &x->sessionDescription, stream);
    Encode_CmtDataType(&x->cmtDataType, stream);
    if (x->option_of_mediaChannel)
    {
        Encode_TransportAddress(&x->mediaChannel, stream);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
        PutBoolean(x->mediaGuaranteedDelivery, stream);
    }
    if (x->option_of_mediaControlChannel)
    {
        Encode_TransportAddress(&x->mediaControlChannel, stream);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
        PutBoolean(x->mediaControlGuaranteedDelivery, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(3, stream); /* Begin Options Map */
    PutBoolean(x->option_of_redundancyEncoding, stream);
    PutBoolean(x->option_of_sessionDependency, stream);
    PutBoolean(x->option_of_destination, stream);
    if (x->option_of_redundancyEncoding)
    {
        PutExtensionItem(EPASS Encode_RedundancyEncoding, (uint8*)&x->redundancyEncoding, stream);
    }
    if (x->option_of_sessionDependency)
    {
        PutExtensionInteger(1, 255, (uint32)x->sessionDependency, stream);
    }
    if (x->option_of_destination)
    {
        PutExtensionItem(EPASS Encode_TerminalLabel, (uint8*)&x->destination, stream);
    }
}

/* <====================================> */
/*  PER-Encoder for CmtDataType (CHOICE)  */
/* <====================================> */
void Encode_CmtDataType(PS_CmtDataType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_VideoCapability(x->videoData, stream);
            break;
        case 1:
            Encode_AudioCapability(x->audioData, stream);
            break;
        case 2:
            Encode_DataApplicationCapability(x->data, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_CmtDataType: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for ConferenceRequest (CHOICE)  */
/* <==========================================> */
void Encode_ConferenceRequest(PS_ConferenceRequest x, PS_OutStream stream)
{
    PutChoiceIndex(8, 1, x->index, stream);
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
            Encode_TerminalLabel(x->dropTerminal, stream);
            break;
        case 4:
            Encode_TerminalLabel(x->requestTerminalID, stream);
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
            PutExtensionNull(stream);
            break;
        case 9:
            PutExtensionNull(stream);
            break;
        case 10:
            PutExtensionItem(EPASS Encode_RequestTerminalCertificate, (uint8*)x->requestTerminalCertificate, stream);
            break;
        case 11:
            PutExtensionInteger(1, 65535, (uint32)x->broadcastMyLogicalChannel, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_TerminalLabel, (uint8*)x->makeTerminalBroadcaster, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_TerminalLabel, (uint8*)x->sendThisSource, stream);
            break;
        case 14:
            PutExtensionNull(stream);
            break;
        case 15:
            PutExtensionItem(EPASS Encode_RemoteMCRequest, (uint8*)x->remoteMCRequest, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ConferenceRequest: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Encoder for RequestTerminalCertificate (SEQUENCE)  */
/* <=====================================================> */
void Encode_RequestTerminalCertificate(PS_RequestTerminalCertificate x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_terminalLabel, stream);
    PutBoolean(x->option_of_certSelectionCriteria, stream);
    PutBoolean(x->option_of_sRandom, stream);
    if (x->option_of_terminalLabel)
    {
        Encode_TerminalLabel(&x->terminalLabel, stream);
    }
    if (x->option_of_certSelectionCriteria)
    {
        Encode_CertSelectionCriteria(&x->certSelectionCriteria, stream);
    }
    if (x->option_of_sRandom)
    {
        PutInteger(1, 0xffffffff, (uint32)x->sRandom, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for CertSelectionCriteria (SEQUENCE-OF)  */
/* <===================================================> */
void Encode_CertSelectionCriteria(PS_CertSelectionCriteria x, PS_OutStream stream)
{
    uint16 i;
    PutInteger(1, 16, (uint32)x->size, stream);
    for (i = 0;i < x->size;++i)
    {
        Encode_Criteria(x->item + i, stream);
    }
}

/* <===================================> */
/*  PER-Encoder for Criteria (SEQUENCE)  */
/* <===================================> */
void Encode_Criteria(PS_Criteria x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutObjectID(&x->field, stream);
    PutOctetString(0, 1, 65535, &x->value, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for TerminalLabel (SEQUENCE)  */
/* <========================================> */
void Encode_TerminalLabel(PS_TerminalLabel x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 192, (uint32)x->mcuNumber, stream);
    PutInteger(0, 192, (uint32)x->terminalNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for ConferenceResponse (CHOICE)  */
/* <===========================================> */
void Encode_ConferenceResponse(PS_ConferenceResponse x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(8, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_MCTerminalIDResponse(x->mCTerminalIDResponse, stream);
            break;
        case 1:
            Encode_TerminalIDResponse(x->terminalIDResponse, stream);
            break;
        case 2:
            Encode_ConferenceIDResponse(x->conferenceIDResponse, stream);
            break;
        case 3:
            Encode_PasswordResponse(x->passwordResponse, stream);
            break;
        case 4:
            PutInteger(1, 256, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_TerminalLabel(x->terminalListResponse + i, stream);
            }
            break;
        case 5:
            /* (videoCommandReject is NULL) */
            break;
        case 6:
            /* (terminalDropReject is NULL) */
            break;
        case 7:
            Encode_MakeMeChairResponse(x->makeMeChairResponse, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            PutExtensionItem(EPASS Encode_ExtensionAddressResponse, (uint8*)x->extensionAddressResponse, stream);
            break;
        case 9:
            PutExtensionItem(EPASS Encode_ChairTokenOwnerResponse, (uint8*)x->chairTokenOwnerResponse, stream);
            break;
        case 10:
            PutExtensionItem(EPASS Encode_TerminalCertificateResponse, (uint8*)x->terminalCertificateResponse, stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_BroadcastMyLogicalChannelResponse, (uint8*)x->broadcastMyLogicalChannelResponse, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_MakeTerminalBroadcasterResponse, (uint8*)x->makeTerminalBroadcasterResponse, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_SendThisSourceResponse, (uint8*)x->sendThisSourceResponse, stream);
            break;
        case 14:
            PutExtensionItem(EPASS Encode_RequestAllTerminalIDsResponse, (uint8*)x->requestAllTerminalIDsResponse, stream);
            break;
        case 15:
            PutExtensionItem(EPASS Encode_RemoteMCResponse, (uint8*)x->remoteMCResponse, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ConferenceResponse: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Encoder for SendThisSourceResponse (CHOICE)  */
/* <===============================================> */
void Encode_SendThisSourceResponse(PS_SendThisSourceResponse x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_SendThisSourceResponse: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Encoder for MakeTerminalBroadcasterResponse (CHOICE)  */
/* <========================================================> */
void Encode_MakeTerminalBroadcasterResponse(PS_MakeTerminalBroadcasterResponse x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_MakeTerminalBroadcasterResponse: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Encoder for BroadcastMyLogicalChannelResponse (CHOICE)  */
/* <==========================================================> */
void Encode_BroadcastMyLogicalChannelResponse(PS_BroadcastMyLogicalChannelResponse x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_BroadcastMyLogicalChannelResponse: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Encoder for TerminalCertificateResponse (SEQUENCE)  */
/* <======================================================> */
void Encode_TerminalCertificateResponse(PS_TerminalCertificateResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_terminalLabel, stream);
    PutBoolean(x->option_of_certificateResponse, stream);
    if (x->option_of_terminalLabel)
    {
        Encode_TerminalLabel(&x->terminalLabel, stream);
    }
    if (x->option_of_certificateResponse)
    {
        PutOctetString(0, 1, 65535, &x->certificateResponse, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for ChairTokenOwnerResponse (SEQUENCE)  */
/* <==================================================> */
void Encode_ChairTokenOwnerResponse(PS_ChairTokenOwnerResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for ExtensionAddressResponse (SEQUENCE)  */
/* <===================================================> */
void Encode_ExtensionAddressResponse(PS_ExtensionAddressResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 1, 128, &x->extensionAddress, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for MakeMeChairResponse (CHOICE)  */
/* <============================================> */
void Encode_MakeMeChairResponse(PS_MakeMeChairResponse x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_MakeMeChairResponse: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for PasswordResponse (SEQUENCE)  */
/* <===========================================> */
void Encode_PasswordResponse(PS_PasswordResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 32, &x->password, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for ConferenceIDResponse (SEQUENCE)  */
/* <===============================================> */
void Encode_ConferenceIDResponse(PS_ConferenceIDResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 32, &x->conferenceID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for TerminalIDResponse (SEQUENCE)  */
/* <=============================================> */
void Encode_TerminalIDResponse(PS_TerminalIDResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for MCTerminalIDResponse (SEQUENCE)  */
/* <===============================================> */
void Encode_MCTerminalIDResponse(PS_MCTerminalIDResponse x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================================> */
/*  PER-Encoder for RequestAllTerminalIDsResponse (SEQUENCE)  */
/* <========================================================> */
void Encode_RequestAllTerminalIDsResponse(PS_RequestAllTerminalIDsResponse x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutLengthDet(x->size_of_terminalInformation, stream);
    for (i = 0;i < x->size_of_terminalInformation;++i)
    {
        Encode_TerminalInformation(x->terminalInformation + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Encoder for TerminalInformation (SEQUENCE)  */
/* <==============================================> */
void Encode_TerminalInformation(PS_TerminalInformation x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TerminalLabel(&x->terminalLabel, stream);
    PutOctetString(0, 1, 128, &x->terminalID, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for RemoteMCRequest (CHOICE)  */
/* <========================================> */
void Encode_RemoteMCRequest(PS_RemoteMCRequest x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_RemoteMCRequest: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for RemoteMCResponse (CHOICE)  */
/* <=========================================> */
void Encode_RemoteMCResponse(PS_RemoteMCResponse x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (accept is NULL) */
            break;
        case 1:
            Encode_Reject(x->reject, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_RemoteMCResponse: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Encoder for Reject (CHOICE)  */
/* <===============================> */
void Encode_Reject(PS_Reject x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Reject: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for MultilinkRequest (CHOICE)  */
/* <=========================================> */
void Encode_MultilinkRequest(PS_MultilinkRequest x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_CallInformation(x->callInformation, stream);
            break;
        case 2:
            Encode_AddConnection(x->addConnection, stream);
            break;
        case 3:
            Encode_RemoveConnection(x->removeConnection, stream);
            break;
        case 4:
            Encode_MaximumHeaderInterval(x->maximumHeaderInterval, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MultilinkRequest: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for MaximumHeaderInterval (SEQUENCE)  */
/* <================================================> */
void Encode_MaximumHeaderInterval(PS_MaximumHeaderInterval x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_RequestType(&x->requestType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Encoder for RequestType (CHOICE)  */
/* <====================================> */
void Encode_RequestType(PS_RequestType x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (currentIntervalInformation is NULL) */
            break;
        case 1:
            PutInteger(0, 65535, (uint32)x->requestedInterval, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_RequestType: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Encoder for RemoveConnection (SEQUENCE)  */
/* <===========================================> */
void Encode_RemoveConnection(PS_RemoveConnection x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Encoder for AddConnection (SEQUENCE)  */
/* <========================================> */
void Encode_AddConnection(PS_AddConnection x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    Encode_DialingInformation(&x->dialingInformation, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for CallInformation (SEQUENCE)  */
/* <==========================================> */
void Encode_CallInformation(PS_CallInformation x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->maxNumberOfAdditionalConnections, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for MultilinkResponse (CHOICE)  */
/* <==========================================> */
void Encode_MultilinkResponse(PS_MultilinkResponse x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_RespCallInformation(x->respCallInformation, stream);
            break;
        case 2:
            Encode_RespAddConnection(x->respAddConnection, stream);
            break;
        case 3:
            Encode_RespRemoveConnection(x->respRemoveConnection, stream);
            break;
        case 4:
            Encode_RespMaximumHeaderInterval(x->respMaximumHeaderInterval, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MultilinkResponse: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Encoder for RespMaximumHeaderInterval (SEQUENCE)  */
/* <====================================================> */
void Encode_RespMaximumHeaderInterval(PS_RespMaximumHeaderInterval x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->currentInterval, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for RespRemoveConnection (SEQUENCE)  */
/* <===============================================> */
void Encode_RespRemoveConnection(PS_RespRemoveConnection x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for RespAddConnection (SEQUENCE)  */
/* <============================================> */
void Encode_RespAddConnection(PS_RespAddConnection x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    Encode_ResponseCode(&x->responseCode, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for ResponseCode (CHOICE)  */
/* <=====================================> */
void Encode_ResponseCode(PS_ResponseCode x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            /* (accepted is NULL) */
            break;
        case 1:
            Encode_Rejected(x->rejected, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_ResponseCode: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Encoder for Rejected (CHOICE)  */
/* <=================================> */
void Encode_Rejected(PS_Rejected x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Rejected: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Encoder for RespCallInformation (SEQUENCE)  */
/* <==============================================> */
void Encode_RespCallInformation(PS_RespCallInformation x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_DialingInformation(&x->dialingInformation, stream);
    PutInteger(0, 0xffffffff, (uint32)x->callAssociationNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Encoder for MultilinkIndication (CHOICE)  */
/* <============================================> */
void Encode_MultilinkIndication(PS_MultilinkIndication x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            Encode_CrcDesired(x->crcDesired, stream);
            break;
        case 2:
            Encode_ExcessiveError(x->excessiveError, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_MultilinkIndication: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for ExcessiveError (SEQUENCE)  */
/* <=========================================> */
void Encode_ExcessiveError(PS_ExcessiveError x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_ConnectionIdentifier(&x->connectionIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for CrcDesired (SEQUENCE)  */
/* <=====================================> */
void Encode_CrcDesired(PS_CrcDesired x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for DialingInformation (CHOICE)  */
/* <===========================================> */
void Encode_DialingInformation(PS_DialingInformation x, PS_OutStream stream)
{
    uint16 i;
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
            break;
        case 1:
            PutInteger(1, 65535, (uint32)x->size, stream);
            for (i = 0;i < x->size;++i)
            {
                Encode_DialingInformationNumber(x->differential + i, stream);
            }
            break;
        case 2:
            PutInteger(1, 65535, (uint32)x->infoNotAvailable, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_DialingInformation: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Encoder for DialingInformationNumber (SEQUENCE)  */
/* <===================================================> */
void Encode_DialingInformationNumber(PS_DialingInformationNumber x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_subAddress, stream);
    PutCharString("NumericString", 0, 0, 40, NULL, &x->networkAddress, stream);
    if (x->option_of_subAddress)
    {
        PutCharString("IA5String", 0, 1, 40, NULL, &x->subAddress, stream);
    }
    PutInteger(1, 255, (uint32)x->size_of_networkType, stream);
    for (i = 0;i < x->size_of_networkType;++i)
    {
        Encode_DialingInformationNetworkType(x->networkType + i, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Encoder for DialingInformationNetworkType (CHOICE)  */
/* <======================================================> */
void Encode_DialingInformationNetworkType(PS_DialingInformationNetworkType x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardMessage(x->nonStandard, stream);
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
            ErrorMessageAndLeave("Encode_DialingInformationNetworkType: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Encoder for ConnectionIdentifier (SEQUENCE)  */
/* <===============================================> */
void Encode_ConnectionIdentifier(PS_ConnectionIdentifier x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 0xffffffff, (uint32)x->channelTag, stream);
    PutInteger(0, 0xffffffff, (uint32)x->sequenceNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Encoder for LogicalChannelRateRequest (SEQUENCE)  */
/* <====================================================> */
void Encode_LogicalChannelRateRequest(PS_LogicalChannelRateRequest x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    PutInteger(0, 0xffffffff, (uint32)x->maximumBitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================================> */
/*  PER-Encoder for LogicalChannelRateAcknowledge (SEQUENCE)  */
/* <========================================================> */
void Encode_LogicalChannelRateAcknowledge(PS_LogicalChannelRateAcknowledge x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    PutInteger(0, 0xffffffff, (uint32)x->maximumBitRate, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for LogicalChannelRateReject (SEQUENCE)  */
/* <===================================================> */
void Encode_LogicalChannelRateReject(PS_LogicalChannelRateReject x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_currentMaximumBitRate, stream);
    PutInteger(0, 255, (uint32)x->sequenceNumber, stream);
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    Encode_LogicalChannelRateRejectReason(&x->rejectReason, stream);
    if (x->option_of_currentMaximumBitRate)
    {
        PutInteger(0, 0xffffffff, (uint32)x->currentMaximumBitRate, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Encoder for LogicalChannelRateRejectReason (CHOICE)  */
/* <=======================================================> */
void Encode_LogicalChannelRateRejectReason(PS_LogicalChannelRateRejectReason x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_LogicalChannelRateRejectReason: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Encoder for LogicalChannelRateRelease (SEQUENCE)  */
/* <====================================================> */
void Encode_LogicalChannelRateRelease(PS_LogicalChannelRateRelease x, PS_OutStream stream)
{
    OSCL_UNUSED_ARG(x);
    PutBoolean(0, stream); /* Extension Bit OFF */
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for SendTerminalCapabilitySet (CHOICE)  */
/* <==================================================> */
void Encode_SendTerminalCapabilitySet(PS_SendTerminalCapabilitySet x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_SpecificRequest(x->specificRequest, stream);
            break;
        case 1:
            /* (genericRequest is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_SendTerminalCapabilitySet: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for SpecificRequest (SEQUENCE)  */
/* <==========================================> */
void Encode_SpecificRequest(PS_SpecificRequest x, PS_OutStream stream)
{
    uint16 i;
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_capabilityTableEntryNumbers, stream);
    PutBoolean(x->option_of_capabilityDescriptorNumbers, stream);
    PutBoolean(x->multiplexCapability, stream);
    if (x->option_of_capabilityTableEntryNumbers)
    {
        PutInteger(1, 65535, (uint32)x->size_of_capabilityTableEntryNumbers, stream);
        for (i = 0;i < x->size_of_capabilityTableEntryNumbers;++i)
        {
            PutInteger(1, 65535, (uint32)x->capabilityTableEntryNumbers[i], stream);
        }
    }
    if (x->option_of_capabilityDescriptorNumbers)
    {
        PutInteger(1, 256, (uint32)x->size_of_capabilityDescriptorNumbers, stream);
        for (i = 0;i < x->size_of_capabilityDescriptorNumbers;++i)
        {
            PutInteger(0, 255, (uint32)x->capabilityDescriptorNumbers[i], stream);
        }
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Encoder for EncryptionCommand (CHOICE)  */
/* <==========================================> */
void Encode_EncryptionCommand(PS_EncryptionCommand x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutOctetString(1, 0, 0, x->encryptionSE, stream);
            break;
        case 1:
            /* (encryptionIVRequest is NULL) */
            break;
        case 2:
            Encode_EncryptionAlgorithmID(x->encryptionAlgorithmID, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_EncryptionCommand: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for EncryptionAlgorithmID (SEQUENCE)  */
/* <================================================> */
void Encode_EncryptionAlgorithmID(PS_EncryptionAlgorithmID x, PS_OutStream stream)
{
    PutInteger(0, 255, (uint32)x->h233AlgorithmIdentifier, stream);
    Encode_NonStandardParameter(&x->associatedAlgorithm, stream);
}

/* <=============================================> */
/*  PER-Encoder for FlowControlCommand (SEQUENCE)  */
/* <=============================================> */
void Encode_FlowControlCommand(PS_FlowControlCommand x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_FccScope(&x->fccScope, stream);
    Encode_FccRestriction(&x->fccRestriction, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for FccRestriction (CHOICE)  */
/* <=======================================> */
void Encode_FccRestriction(PS_FccRestriction x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 16777215, (uint32)x->maximumBitRate, stream);
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_FccRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Encoder for FccScope (CHOICE)  */
/* <=================================> */
void Encode_FccScope(PS_FccScope x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
            break;
        case 1:
            PutInteger(0, 65535, (uint32)x->resourceID, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_FccScope: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for EndSessionCommand (CHOICE)  */
/* <==========================================> */
void Encode_EndSessionCommand(PS_EndSessionCommand x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            /* (disconnect is NULL) */
            break;
        case 2:
            Encode_GstnOptions(x->gstnOptions, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            PutExtensionItem(EPASS Encode_IsdnOptions, (uint8*)x->isdnOptions, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_EndSessionCommand: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Encoder for IsdnOptions (CHOICE)  */
/* <====================================> */
void Encode_IsdnOptions(PS_IsdnOptions x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_IsdnOptions: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Encoder for GstnOptions (CHOICE)  */
/* <====================================> */
void Encode_GstnOptions(PS_GstnOptions x, PS_OutStream stream)
{
    PutChoiceIndex(5, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_GstnOptions: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for ConferenceCommand (CHOICE)  */
/* <==========================================> */
void Encode_ConferenceCommand(PS_ConferenceCommand x, PS_OutStream stream)
{
    PutChoiceIndex(7, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->broadcastMyLogicalChannel, stream);
            break;
        case 1:
            PutInteger(1, 65535, (uint32)x->cancelBroadcastMyLogicalChannel, stream);
            break;
        case 2:
            Encode_TerminalLabel(x->makeTerminalBroadcaster, stream);
            break;
        case 3:
            /* (cancelMakeTerminalBroadcaster is NULL) */
            break;
        case 4:
            Encode_TerminalLabel(x->sendThisSource, stream);
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
            PutExtensionItem(EPASS Encode_SubstituteConferenceIDCommand, (uint8*)x->substituteConferenceIDCommand, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ConferenceCommand: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Encoder for SubstituteConferenceIDCommand (SEQUENCE)  */
/* <========================================================> */
void Encode_SubstituteConferenceIDCommand(PS_SubstituteConferenceIDCommand x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutOctetString(0, 16, 16, &x->conferenceIdentifier, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for MiscellaneousCommand (SEQUENCE)  */
/* <===============================================> */
void Encode_MiscellaneousCommand(PS_MiscellaneousCommand x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    Encode_McType(&x->mcType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================> */
/*  PER-Encoder for McType (CHOICE)  */
/* <===============================> */
void Encode_McType(PS_McType x, PS_OutStream stream)
{
    PutChoiceIndex(10, 1, x->index, stream);
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
            Encode_VideoFastUpdateGOB(x->videoFastUpdateGOB, stream);
            break;
        case 7:
            PutInteger(0, 31, (uint32)x->videoTemporalSpatialTradeOff, stream);
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
            PutExtensionItem(EPASS Encode_VideoFastUpdateMB, (uint8*)x->videoFastUpdateMB, stream);
            break;
        case 11:
            PutExtensionInteger(1, 65535, (uint32)x->maxH223MUXPDUsize, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_EncryptionSync, (uint8*)x->encryptionUpdate, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_EncryptionUpdateRequest, (uint8*)x->encryptionUpdateRequest, stream);
            break;
        case 14:
            PutExtensionNull(stream);
            break;
        case 15:
            PutExtensionNull(stream);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_ProgressiveRefinementStart, (uint8*)x->progressiveRefinementStart, stream);
            break;
        case 17:
            PutExtensionNull(stream);
            break;
        case 18:
            PutExtensionNull(stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_McType: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Encoder for ProgressiveRefinementStart (SEQUENCE)  */
/* <=====================================================> */
void Encode_ProgressiveRefinementStart(PS_ProgressiveRefinementStart x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_PrsRepeatCount(&x->prsRepeatCount, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for PrsRepeatCount (CHOICE)  */
/* <=======================================> */
void Encode_PrsRepeatCount(PS_PrsRepeatCount x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_PrsRepeatCount: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Encoder for VideoFastUpdateMB (SEQUENCE)  */
/* <============================================> */
void Encode_VideoFastUpdateMB(PS_VideoFastUpdateMB x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_firstGOB, stream);
    PutBoolean(x->option_of_firstMB, stream);
    if (x->option_of_firstGOB)
    {
        PutInteger(0, 255, (uint32)x->firstGOB, stream);
    }
    if (x->option_of_firstMB)
    {
        PutInteger(1, 8192, (uint32)x->firstMB, stream);
    }
    PutInteger(1, 8192, (uint32)x->numberOfMBs, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for VideoFastUpdateGOB (SEQUENCE)  */
/* <=============================================> */
void Encode_VideoFastUpdateGOB(PS_VideoFastUpdateGOB x, PS_OutStream stream)
{
    PutInteger(0, 17, (uint32)x->firstGOB, stream);
    PutInteger(1, 18, (uint32)x->numberOfGOBs, stream);
}

/* <==============================================> */
/*  PER-Encoder for KeyProtectionMethod (SEQUENCE)  */
/* <==============================================> */
void Encode_KeyProtectionMethod(PS_KeyProtectionMethod x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->secureChannel, stream);
    PutBoolean(x->sharedSecret, stream);
    PutBoolean(x->certProtectedKey, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for EncryptionUpdateRequest (SEQUENCE)  */
/* <==================================================> */
void Encode_EncryptionUpdateRequest(PS_EncryptionUpdateRequest x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_keyProtectionMethod, stream);
    if (x->option_of_keyProtectionMethod)
    {
        Encode_KeyProtectionMethod(&x->keyProtectionMethod, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Encoder for H223MultiplexReconfiguration (CHOICE)  */
/* <=====================================================> */
void Encode_H223MultiplexReconfiguration(PS_H223MultiplexReconfiguration x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_H223ModeChange(x->h223ModeChange, stream);
            break;
        case 1:
            Encode_H223AnnexADoubleFlag(x->h223AnnexADoubleFlag, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_H223MultiplexReconfiguration: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for H223AnnexADoubleFlag (CHOICE)  */
/* <=============================================> */
void Encode_H223AnnexADoubleFlag(PS_H223AnnexADoubleFlag x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_H223AnnexADoubleFlag: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for H223ModeChange (CHOICE)  */
/* <=======================================> */
void Encode_H223ModeChange(PS_H223ModeChange x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_H223ModeChange: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Encoder for NewATMVCCommand (SEQUENCE)  */
/* <==========================================> */
void Encode_NewATMVCCommand(PS_NewATMVCCommand x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->resourceID, stream);
    PutInteger(1, 65535, (uint32)x->bitRate, stream);
    PutBoolean(x->bitRateLockedToPCRClock, stream);
    PutBoolean(x->bitRateLockedToNetworkClock, stream);
    Encode_CmdAal(&x->cmdAal, stream);
    Encode_CmdMultiplex(&x->cmdMultiplex, stream);
    Encode_CmdReverseParameters(&x->cmdReverseParameters, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for CmdReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Encode_CmdReverseParameters(PS_CmdReverseParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->bitRate, stream);
    PutBoolean(x->bitRateLockedToPCRClock, stream);
    PutBoolean(x->bitRateLockedToNetworkClock, stream);
    Encode_Multiplex(&x->multiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for Multiplex (CHOICE)  */
/* <==================================> */
void Encode_Multiplex(PS_Multiplex x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_Multiplex: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for CmdMultiplex (CHOICE)  */
/* <=====================================> */
void Encode_CmdMultiplex(PS_CmdMultiplex x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_CmdMultiplex: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Encoder for CmdAal (CHOICE)  */
/* <===============================> */
void Encode_CmdAal(PS_CmdAal x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_CmdAal1(x->cmdAal1, stream);
            break;
        case 1:
            Encode_CmdAal5(x->cmdAal5, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_CmdAal: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for CmdAal5 (SEQUENCE)  */
/* <==================================> */
void Encode_CmdAal5(PS_CmdAal5 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->forwardMaximumSDUSize, stream);
    PutInteger(0, 65535, (uint32)x->backwardMaximumSDUSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for CmdAal1 (SEQUENCE)  */
/* <==================================> */
void Encode_CmdAal1(PS_CmdAal1 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_CmdClockRecovery(&x->cmdClockRecovery, stream);
    Encode_CmdErrorCorrection(&x->cmdErrorCorrection, stream);
    PutBoolean(x->structuredDataTransfer, stream);
    PutBoolean(x->partiallyFilledCells, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for CmdErrorCorrection (CHOICE)  */
/* <===========================================> */
void Encode_CmdErrorCorrection(PS_CmdErrorCorrection x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_CmdErrorCorrection: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for CmdClockRecovery (CHOICE)  */
/* <=========================================> */
void Encode_CmdClockRecovery(PS_CmdClockRecovery x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_CmdClockRecovery: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Encoder for FunctionNotUnderstood (CHOICE)  */
/* <==============================================> */
void Encode_FunctionNotUnderstood(PS_FunctionNotUnderstood x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_RequestMessage(x->request, stream);
            break;
        case 1:
            Encode_ResponseMessage(x->response, stream);
            break;
        case 2:
            Encode_CommandMessage(x->command, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_FunctionNotUnderstood: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Encoder for FunctionNotSupported (SEQUENCE)  */
/* <===============================================> */
void Encode_FunctionNotSupported(PS_FunctionNotSupported x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_returnedFunction, stream);
    Encode_FnsCause(&x->fnsCause, stream);
    if (x->option_of_returnedFunction)
    {
        PutOctetString(1, 0, 0, &x->returnedFunction, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for FnsCause (CHOICE)  */
/* <=================================> */
void Encode_FnsCause(PS_FnsCause x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_FnsCause: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for ConferenceIndication (CHOICE)  */
/* <=============================================> */
void Encode_ConferenceIndication(PS_ConferenceIndication x, PS_OutStream stream)
{
    PutChoiceIndex(10, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 9, (uint32)x->sbeNumber, stream);
            break;
        case 1:
            Encode_TerminalLabel(x->terminalNumberAssign, stream);
            break;
        case 2:
            Encode_TerminalLabel(x->terminalJoinedConference, stream);
            break;
        case 3:
            Encode_TerminalLabel(x->terminalLeftConference, stream);
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
            Encode_TerminalLabel(x->terminalYouAreSeeing, stream);
            break;
        case 9:
            /* (requestForFloor is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            PutExtensionNull(stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_TerminalLabel, (uint8*)x->floorRequested, stream);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_TerminalYouAreSeeingInSubPictureNumber, (uint8*)x->terminalYouAreSeeingInSubPictureNumber, stream);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_VideoIndicateCompose, (uint8*)x->videoIndicateCompose, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_ConferenceIndication: Illegal CHOICE index");
    }
}

/* <=================================================================> */
/*  PER-Encoder for TerminalYouAreSeeingInSubPictureNumber (SEQUENCE)  */
/* <=================================================================> */
void Encode_TerminalYouAreSeeingInSubPictureNumber(PS_TerminalYouAreSeeingInSubPictureNumber x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 192, (uint32)x->terminalNumber, stream);
    PutInteger(0, 255, (uint32)x->subPictureNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for VideoIndicateCompose (SEQUENCE)  */
/* <===============================================> */
void Encode_VideoIndicateCompose(PS_VideoIndicateCompose x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 255, (uint32)x->compositionNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Encoder for MiscellaneousIndication (SEQUENCE)  */
/* <==================================================> */
void Encode_MiscellaneousIndication(PS_MiscellaneousIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    Encode_MiType(&x->miType, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================> */
/*  PER-Encoder for MiType (CHOICE)  */
/* <===============================> */
void Encode_MiType(PS_MiType x, PS_OutStream stream)
{
    PutChoiceIndex(10, 1, x->index, stream);
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
            PutInteger(0, 31, (uint32)x->videoTemporalSpatialTradeOff, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            PutExtensionItem(EPASS Encode_VideoNotDecodedMBs, (uint8*)x->videoNotDecodedMBs, stream);
            break;
        case 11:
            PutExtensionItem(EPASS Encode_TransportCapability, (uint8*)x->transportCapability, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_MiType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for VideoNotDecodedMBs (SEQUENCE)  */
/* <=============================================> */
void Encode_VideoNotDecodedMBs(PS_VideoNotDecodedMBs x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 8192, (uint32)x->firstMB, stream);
    PutInteger(1, 8192, (uint32)x->numberOfMBs, stream);
    PutInteger(0, 255, (uint32)x->temporalReference, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for JitterIndication (SEQUENCE)  */
/* <===========================================> */
void Encode_JitterIndication(PS_JitterIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_skippedFrameCount, stream);
    PutBoolean(x->option_of_additionalDecoderBuffer, stream);
    Encode_JiScope(&x->jiScope, stream);
    PutInteger(0, 3, (uint32)x->estimatedReceivedJitterMantissa, stream);
    PutInteger(0, 7, (uint32)x->estimatedReceivedJitterExponent, stream);
    if (x->option_of_skippedFrameCount)
    {
        PutInteger(0, 15, (uint32)x->skippedFrameCount, stream);
    }
    if (x->option_of_additionalDecoderBuffer)
    {
        PutInteger(0, 262143, (uint32)x->additionalDecoderBuffer, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Encoder for JiScope (CHOICE)  */
/* <================================> */
void Encode_JiScope(PS_JiScope x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
            break;
        case 1:
            PutInteger(0, 65535, (uint32)x->resourceID, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_JiScope: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Encoder for H223SkewIndication (SEQUENCE)  */
/* <=============================================> */
void Encode_H223SkewIndication(PS_H223SkewIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber1, stream);
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber2, stream);
    PutInteger(0, 4095, (uint32)x->skew, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Encoder for H2250MaximumSkewIndication (SEQUENCE)  */
/* <=====================================================> */
void Encode_H2250MaximumSkewIndication(PS_H2250MaximumSkewIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber1, stream);
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber2, stream);
    PutInteger(0, 4095, (uint32)x->maximumSkew, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for MCLocationIndication (SEQUENCE)  */
/* <===============================================> */
void Encode_MCLocationIndication(PS_MCLocationIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_TransportAddress(&x->signalAddress, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Encoder for VendorIdentification (SEQUENCE)  */
/* <===============================================> */
void Encode_VendorIdentification(PS_VendorIdentification x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_productNumber, stream);
    PutBoolean(x->option_of_versionNumber, stream);
    Encode_NonStandardIdentifier(&x->vendor, stream);
    if (x->option_of_productNumber)
    {
        PutOctetString(0, 1, 256, &x->productNumber, stream);
    }
    if (x->option_of_versionNumber)
    {
        PutOctetString(0, 1, 256, &x->versionNumber, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Encoder for NewATMVCIndication (SEQUENCE)  */
/* <=============================================> */
void Encode_NewATMVCIndication(PS_NewATMVCIndication x, PS_OutStream stream)
{
    uint32 extension;

    extension = x->option_of_indReverseParameters;
    PutBoolean(extension, stream);

    PutInteger(0, 65535, (uint32)x->resourceID, stream);
    PutInteger(1, 65535, (uint32)x->bitRate, stream);
    PutBoolean(x->bitRateLockedToPCRClock, stream);
    PutBoolean(x->bitRateLockedToNetworkClock, stream);
    Encode_IndAal(&x->indAal, stream);
    Encode_IndMultiplex(&x->indMultiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    PutNormSmallLength(1, stream); /* Begin Options Map */
    PutBoolean(x->option_of_indReverseParameters, stream);
    if (x->option_of_indReverseParameters)
    {
        PutExtensionItem(EPASS Encode_IndReverseParameters, (uint8*)&x->indReverseParameters, stream);
    }
}

/* <===============================================> */
/*  PER-Encoder for IndReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Encode_IndReverseParameters(PS_IndReverseParameters x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->bitRate, stream);
    PutBoolean(x->bitRateLockedToPCRClock, stream);
    PutBoolean(x->bitRateLockedToNetworkClock, stream);
    Encode_IrpMultiplex(&x->irpMultiplex, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Encoder for IrpMultiplex (CHOICE)  */
/* <=====================================> */
void Encode_IrpMultiplex(PS_IrpMultiplex x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_IrpMultiplex: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Encoder for IndMultiplex (CHOICE)  */
/* <=====================================> */
void Encode_IndMultiplex(PS_IndMultiplex x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_IndMultiplex: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Encoder for IndAal (CHOICE)  */
/* <===============================> */
void Encode_IndAal(PS_IndAal x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_IndAal1(x->indAal1, stream);
            break;
        case 1:
            Encode_IndAal5(x->indAal5, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessageAndLeave("Encode_IndAal: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Encoder for IndAal5 (SEQUENCE)  */
/* <==================================> */
void Encode_IndAal5(PS_IndAal5 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(0, 65535, (uint32)x->forwardMaximumSDUSize, stream);
    PutInteger(0, 65535, (uint32)x->backwardMaximumSDUSize, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Encoder for IndAal1 (SEQUENCE)  */
/* <==================================> */
void Encode_IndAal1(PS_IndAal1 x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_IndClockRecovery(&x->indClockRecovery, stream);
    Encode_IndErrorCorrection(&x->indErrorCorrection, stream);
    PutBoolean(x->structuredDataTransfer, stream);
    PutBoolean(x->partiallyFilledCells, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Encoder for IndErrorCorrection (CHOICE)  */
/* <===========================================> */
void Encode_IndErrorCorrection(PS_IndErrorCorrection x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_IndErrorCorrection: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Encoder for IndClockRecovery (CHOICE)  */
/* <=========================================> */
void Encode_IndClockRecovery(PS_IndClockRecovery x, PS_OutStream stream)
{
    PutChoiceIndex(3, 1, x->index, stream);
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
            ErrorMessageAndLeave("Encode_IndClockRecovery: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Encoder for UserInputIndication (CHOICE)  */
/* <============================================> */
void Encode_UserInputIndication(PS_UserInputIndication x, PS_OutStream stream)
{
    PutChoiceIndex(2, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
            break;
        case 1:
            PutCharString("GeneralString", 1, 0, 0, NULL, x->alphanumeric, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            PutExtensionItem(EPASS Encode_UserInputSupportIndication, (uint8*)x->userInputSupportIndication, stream);
            break;
        case 3:
            PutExtensionItem(EPASS Encode_Signal, (uint8*)x->signal, stream);
            break;
        case 4:
            PutExtensionItem(EPASS Encode_SignalUpdate, (uint8*)x->signalUpdate, stream);
            break;
        default:
            ErrorMessageAndLeave("Encode_UserInputIndication: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Encoder for SignalUpdate (SEQUENCE)  */
/* <=======================================> */
void Encode_SignalUpdate(PS_SignalUpdate x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_rtp, stream);
    PutInteger(1, 65535, (uint32)x->duration, stream);
    if (x->option_of_rtp)
    {
        Encode_Rtp(&x->rtp, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================> */
/*  PER-Encoder for Rtp (SEQUENCE)  */
/* <==============================> */
void Encode_Rtp(PS_Rtp x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Encoder for Signal (SEQUENCE)  */
/* <=================================> */
void Encode_Signal(PS_Signal x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_duration, stream);
    PutBoolean(x->option_of_signalRtp, stream);
    PutCharString("IA5String", 0, 1, 1, "0123456789#*ABCD!", &x->signalType, stream);
    if (x->option_of_duration)
    {
        PutInteger(1, 65535, (uint32)x->duration, stream);
    }
    if (x->option_of_signalRtp)
    {
        Encode_SignalRtp(&x->signalRtp, stream);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Encoder for SignalRtp (SEQUENCE)  */
/* <====================================> */
void Encode_SignalRtp(PS_SignalRtp x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    PutBoolean(x->option_of_timestamp, stream);
    PutBoolean(x->option_of_expirationTime, stream);
    if (x->option_of_timestamp)
    {
        PutInteger(0, 0xffffffff, (uint32)x->timestamp, stream);
    }
    if (x->option_of_expirationTime)
    {
        PutInteger(0, 0xffffffff, (uint32)x->expirationTime, stream);
    }
    PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Encoder for UserInputSupportIndication (CHOICE)  */
/* <===================================================> */
void Encode_UserInputSupportIndication(PS_UserInputSupportIndication x, PS_OutStream stream)
{
    PutChoiceIndex(4, 1, x->index, stream);
    switch (x->index)
    {
        case 0:
            Encode_NonStandardParameter(x->nonStandard, stream);
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
            ErrorMessageAndLeave("Encode_UserInputSupportIndication: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Encoder for FlowControlIndication (SEQUENCE)  */
/* <================================================> */
void Encode_FlowControlIndication(PS_FlowControlIndication x, PS_OutStream stream)
{
    PutBoolean(0, stream); /* Extension Bit OFF */
    Encode_FciScope(&x->fciScope, stream);
    Encode_FciRestriction(&x->fciRestriction, stream);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Encoder for FciRestriction (CHOICE)  */
/* <=======================================> */
void Encode_FciRestriction(PS_FciRestriction x, PS_OutStream stream)
{
    PutChoiceIndex(2, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(0, 16777215, (uint32)x->maximumBitRate, stream);
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_FciRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Encoder for FciScope (CHOICE)  */
/* <=================================> */
void Encode_FciScope(PS_FciScope x, PS_OutStream stream)
{
    PutChoiceIndex(3, 0, x->index, stream);
    switch (x->index)
    {
        case 0:
            PutInteger(1, 65535, (uint32)x->logicalChannelNumber, stream);
            break;
        case 1:
            PutInteger(0, 65535, (uint32)x->resourceID, stream);
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessageAndLeave("Encode_FciScope: Illegal CHOICE index");
    }
}

