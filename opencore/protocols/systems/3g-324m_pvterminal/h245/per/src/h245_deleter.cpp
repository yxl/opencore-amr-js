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
// FILE: h245_deleter.c
//
// DESC: PER Delete routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

#include "oscl_base.h"
#include "oscl_mem.h"
#include "per_headers.h"
#include "h245def.h"
#include "h245_deleter.h"

/* <=======================================================> */
/*  PER-Deleter for MultimediaSystemControlMessage (CHOICE)  */
/* <=======================================================> */
void Delete_MultimediaSystemControlMessage(PS_MultimediaSystemControlMessage x)
{
    switch (x->index)
    {
        case 0:
            Delete_RequestMessage(x->request);
            OSCL_DEFAULT_FREE(x->request);
            break;
        case 1:
            Delete_ResponseMessage(x->response);
            OSCL_DEFAULT_FREE(x->response);
            break;
        case 2:
            Delete_CommandMessage(x->command);
            OSCL_DEFAULT_FREE(x->command);
            break;
        case 3:
            Delete_IndicationMessage(x->indication);
            OSCL_DEFAULT_FREE(x->indication);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MultimediaSystemControlMessage: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for RequestMessage (CHOICE)  */
/* <=======================================> */
void Delete_RequestMessage(PS_RequestMessage x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_MasterSlaveDetermination(x->masterSlaveDetermination);
            OSCL_DEFAULT_FREE(x->masterSlaveDetermination);
            break;
        case 2:
            Delete_TerminalCapabilitySet(x->terminalCapabilitySet);
            OSCL_DEFAULT_FREE(x->terminalCapabilitySet);
            break;
        case 3:
            Delete_OpenLogicalChannel(x->openLogicalChannel);
            OSCL_DEFAULT_FREE(x->openLogicalChannel);
            break;
        case 4:
            Delete_CloseLogicalChannel(x->closeLogicalChannel);
            OSCL_DEFAULT_FREE(x->closeLogicalChannel);
            break;
        case 5:
            Delete_RequestChannelClose(x->requestChannelClose);
            OSCL_DEFAULT_FREE(x->requestChannelClose);
            break;
        case 6:
            Delete_MultiplexEntrySend(x->multiplexEntrySend);
            OSCL_DEFAULT_FREE(x->multiplexEntrySend);
            break;
        case 7:
            Delete_RequestMultiplexEntry(x->requestMultiplexEntry);
            OSCL_DEFAULT_FREE(x->requestMultiplexEntry);
            break;
        case 8:
            Delete_RequestMode(x->requestMode);
            OSCL_DEFAULT_FREE(x->requestMode);
            break;
        case 9:
            Delete_RoundTripDelayRequest(x->roundTripDelayRequest);
            OSCL_DEFAULT_FREE(x->roundTripDelayRequest);
            break;
        case 10:
            Delete_MaintenanceLoopRequest(x->maintenanceLoopRequest);
            OSCL_DEFAULT_FREE(x->maintenanceLoopRequest);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            Delete_CommunicationModeRequest(x->communicationModeRequest);
            OSCL_DEFAULT_FREE(x->communicationModeRequest);
            break;
        case 12:
            Delete_ConferenceRequest(x->conferenceRequest);
            OSCL_DEFAULT_FREE(x->conferenceRequest);
            break;
        case 13:
            Delete_MultilinkRequest(x->multilinkRequest);
            OSCL_DEFAULT_FREE(x->multilinkRequest);
            break;
        case 14:
            Delete_LogicalChannelRateRequest(x->logicalChannelRateRequest);
            OSCL_DEFAULT_FREE(x->logicalChannelRateRequest);
            break;
        default:
            ErrorMessage("Delete_RequestMessage: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for ResponseMessage (CHOICE)  */
/* <========================================> */
void Delete_ResponseMessage(PS_ResponseMessage x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_MasterSlaveDeterminationAck(x->masterSlaveDeterminationAck);
            OSCL_DEFAULT_FREE(x->masterSlaveDeterminationAck);
            break;
        case 2:
            Delete_MasterSlaveDeterminationReject(x->masterSlaveDeterminationReject);
            OSCL_DEFAULT_FREE(x->masterSlaveDeterminationReject);
            break;
        case 3:
            Delete_TerminalCapabilitySetAck(x->terminalCapabilitySetAck);
            OSCL_DEFAULT_FREE(x->terminalCapabilitySetAck);
            break;
        case 4:
            Delete_TerminalCapabilitySetReject(x->terminalCapabilitySetReject);
            OSCL_DEFAULT_FREE(x->terminalCapabilitySetReject);
            break;
        case 5:
            Delete_OpenLogicalChannelAck(x->openLogicalChannelAck);
            OSCL_DEFAULT_FREE(x->openLogicalChannelAck);
            break;
        case 6:
            Delete_OpenLogicalChannelReject(x->openLogicalChannelReject);
            OSCL_DEFAULT_FREE(x->openLogicalChannelReject);
            break;
        case 7:
            Delete_CloseLogicalChannelAck(x->closeLogicalChannelAck);
            OSCL_DEFAULT_FREE(x->closeLogicalChannelAck);
            break;
        case 8:
            Delete_RequestChannelCloseAck(x->requestChannelCloseAck);
            OSCL_DEFAULT_FREE(x->requestChannelCloseAck);
            break;
        case 9:
            Delete_RequestChannelCloseReject(x->requestChannelCloseReject);
            OSCL_DEFAULT_FREE(x->requestChannelCloseReject);
            break;
        case 10:
            Delete_MultiplexEntrySendAck(x->multiplexEntrySendAck);
            OSCL_DEFAULT_FREE(x->multiplexEntrySendAck);
            break;
        case 11:
            Delete_MultiplexEntrySendReject(x->multiplexEntrySendReject);
            OSCL_DEFAULT_FREE(x->multiplexEntrySendReject);
            break;
        case 12:
            Delete_RequestMultiplexEntryAck(x->requestMultiplexEntryAck);
            OSCL_DEFAULT_FREE(x->requestMultiplexEntryAck);
            break;
        case 13:
            Delete_RequestMultiplexEntryReject(x->requestMultiplexEntryReject);
            OSCL_DEFAULT_FREE(x->requestMultiplexEntryReject);
            break;
        case 14:
            Delete_RequestModeAck(x->requestModeAck);
            OSCL_DEFAULT_FREE(x->requestModeAck);
            break;
        case 15:
            Delete_RequestModeReject(x->requestModeReject);
            OSCL_DEFAULT_FREE(x->requestModeReject);
            break;
        case 16:
            Delete_RoundTripDelayResponse(x->roundTripDelayResponse);
            OSCL_DEFAULT_FREE(x->roundTripDelayResponse);
            break;
        case 17:
            Delete_MaintenanceLoopAck(x->maintenanceLoopAck);
            OSCL_DEFAULT_FREE(x->maintenanceLoopAck);
            break;
        case 18:
            Delete_MaintenanceLoopReject(x->maintenanceLoopReject);
            OSCL_DEFAULT_FREE(x->maintenanceLoopReject);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            Delete_CommunicationModeResponse(x->communicationModeResponse);
            OSCL_DEFAULT_FREE(x->communicationModeResponse);
            break;
        case 20:
            Delete_ConferenceResponse(x->conferenceResponse);
            OSCL_DEFAULT_FREE(x->conferenceResponse);
            break;
        case 21:
            Delete_MultilinkResponse(x->multilinkResponse);
            OSCL_DEFAULT_FREE(x->multilinkResponse);
            break;
        case 22:
            Delete_LogicalChannelRateAcknowledge(x->logicalChannelRateAcknowledge);
            OSCL_DEFAULT_FREE(x->logicalChannelRateAcknowledge);
            break;
        case 23:
            Delete_LogicalChannelRateReject(x->logicalChannelRateReject);
            OSCL_DEFAULT_FREE(x->logicalChannelRateReject);
            break;
        default:
            ErrorMessage("Delete_ResponseMessage: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for CommandMessage (CHOICE)  */
/* <=======================================> */
void Delete_CommandMessage(PS_CommandMessage x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_MaintenanceLoopOffCommand(x->maintenanceLoopOffCommand);
            OSCL_DEFAULT_FREE(x->maintenanceLoopOffCommand);
            break;
        case 2:
            Delete_SendTerminalCapabilitySet(x->sendTerminalCapabilitySet);
            OSCL_DEFAULT_FREE(x->sendTerminalCapabilitySet);
            break;
        case 3:
            Delete_EncryptionCommand(x->encryptionCommand);
            OSCL_DEFAULT_FREE(x->encryptionCommand);
            break;
        case 4:
            Delete_FlowControlCommand(x->flowControlCommand);
            OSCL_DEFAULT_FREE(x->flowControlCommand);
            break;
        case 5:
            Delete_EndSessionCommand(x->endSessionCommand);
            OSCL_DEFAULT_FREE(x->endSessionCommand);
            break;
        case 6:
            Delete_MiscellaneousCommand(x->miscellaneousCommand);
            OSCL_DEFAULT_FREE(x->miscellaneousCommand);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            Delete_CommunicationModeCommand(x->communicationModeCommand);
            OSCL_DEFAULT_FREE(x->communicationModeCommand);
            break;
        case 8:
            Delete_ConferenceCommand(x->conferenceCommand);
            OSCL_DEFAULT_FREE(x->conferenceCommand);
            break;
        case 9:
            Delete_H223MultiplexReconfiguration(x->h223MultiplexReconfiguration);
            OSCL_DEFAULT_FREE(x->h223MultiplexReconfiguration);
            break;
        case 10:
            Delete_NewATMVCCommand(x->newATMVCCommand);
            OSCL_DEFAULT_FREE(x->newATMVCCommand);
            break;
        default:
            ErrorMessage("Delete_CommandMessage: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for IndicationMessage (CHOICE)  */
/* <==========================================> */
void Delete_IndicationMessage(PS_IndicationMessage x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_FunctionNotUnderstood(x->functionNotUnderstood);
            OSCL_DEFAULT_FREE(x->functionNotUnderstood);
            break;
        case 2:
            Delete_MasterSlaveDeterminationRelease(x->masterSlaveDeterminationRelease);
            OSCL_DEFAULT_FREE(x->masterSlaveDeterminationRelease);
            break;
        case 3:
            Delete_TerminalCapabilitySetRelease(x->terminalCapabilitySetRelease);
            OSCL_DEFAULT_FREE(x->terminalCapabilitySetRelease);
            break;
        case 4:
            Delete_OpenLogicalChannelConfirm(x->openLogicalChannelConfirm);
            OSCL_DEFAULT_FREE(x->openLogicalChannelConfirm);
            break;
        case 5:
            Delete_RequestChannelCloseRelease(x->requestChannelCloseRelease);
            OSCL_DEFAULT_FREE(x->requestChannelCloseRelease);
            break;
        case 6:
            Delete_MultiplexEntrySendRelease(x->multiplexEntrySendRelease);
            OSCL_DEFAULT_FREE(x->multiplexEntrySendRelease);
            break;
        case 7:
            Delete_RequestMultiplexEntryRelease(x->requestMultiplexEntryRelease);
            OSCL_DEFAULT_FREE(x->requestMultiplexEntryRelease);
            break;
        case 8:
            Delete_RequestModeRelease(x->requestModeRelease);
            OSCL_DEFAULT_FREE(x->requestModeRelease);
            break;
        case 9:
            Delete_MiscellaneousIndication(x->miscellaneousIndication);
            OSCL_DEFAULT_FREE(x->miscellaneousIndication);
            break;
        case 10:
            Delete_JitterIndication(x->jitterIndication);
            OSCL_DEFAULT_FREE(x->jitterIndication);
            break;
        case 11:
            Delete_H223SkewIndication(x->h223SkewIndication);
            OSCL_DEFAULT_FREE(x->h223SkewIndication);
            break;
        case 12:
            Delete_NewATMVCIndication(x->newATMVCIndication);
            OSCL_DEFAULT_FREE(x->newATMVCIndication);
            break;
        case 13:
            Delete_UserInputIndication(x->userInput);
            OSCL_DEFAULT_FREE(x->userInput);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            Delete_H2250MaximumSkewIndication(x->h2250MaximumSkewIndication);
            OSCL_DEFAULT_FREE(x->h2250MaximumSkewIndication);
            break;
        case 15:
            Delete_MCLocationIndication(x->mcLocationIndication);
            OSCL_DEFAULT_FREE(x->mcLocationIndication);
            break;
        case 16:
            Delete_ConferenceIndication(x->conferenceIndication);
            OSCL_DEFAULT_FREE(x->conferenceIndication);
            break;
        case 17:
            Delete_VendorIdentification(x->vendorIdentification);
            OSCL_DEFAULT_FREE(x->vendorIdentification);
            break;
        case 18:
            Delete_FunctionNotSupported(x->functionNotSupported);
            OSCL_DEFAULT_FREE(x->functionNotSupported);
            break;
        case 19:
            Delete_MultilinkIndication(x->multilinkIndication);
            OSCL_DEFAULT_FREE(x->multilinkIndication);
            break;
        case 20:
            Delete_LogicalChannelRateRelease(x->logicalChannelRateRelease);
            OSCL_DEFAULT_FREE(x->logicalChannelRateRelease);
            break;
        case 21:
            Delete_FlowControlIndication(x->flowControlIndication);
            OSCL_DEFAULT_FREE(x->flowControlIndication);
            break;
        default:
            ErrorMessage("Delete_IndicationMessage: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for GenericInformation (SEQUENCE)  */
/* <=============================================> */
void Delete_GenericInformation(PS_GenericInformation x)
{
    uint16 i = 0;
    Delete_CapabilityIdentifier(&x->messageIdentifier);
    if (x->option_of_subMessageIdentifier)
    {
    }
    if (x->option_of_messageContent)
    {
        for (i = 0;i < x->size_of_messageContent;++i)
        {
            Delete_GenericParameter(x->messageContent + i);
        }
        OSCL_DEFAULT_FREE(x->messageContent);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for NonStandardMessage (SEQUENCE)  */
/* <=============================================> */
void Delete_NonStandardMessage(PS_NonStandardMessage x)
{
    Delete_NonStandardParameter(&x->nonStandardData);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for NonStandardParameter (SEQUENCE)  */
/* <===============================================> */
void Delete_NonStandardParameter(PS_NonStandardParameter x)
{
    if (x->data.size > 0) OSCL_DEFAULT_FREE(x->data.data);
    Delete_NonStandardIdentifier(&x->nonStandardIdentifier);
}

/* <==============================================> */
/*  PER-Deleter for NonStandardIdentifier (CHOICE)  */
/* <==============================================> */
void Delete_NonStandardIdentifier(PS_NonStandardIdentifier x)
{
    switch (x->index)
    {
        case 0:
            FreeObjectID(x->object);
            break;
        case 1:
            Delete_H221NonStandard(x->h221NonStandard);
            OSCL_DEFAULT_FREE(x->h221NonStandard);
            break;
        default:
            ErrorMessage("Delete_NonStandardIdentifier: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for H221NonStandard (SEQUENCE)  */
/* <==========================================> */
void Delete_H221NonStandard(PS_H221NonStandard x)
{
    OSCL_UNUSED_ARG(x);
}

/* <===================================================> */
/*  PER-Deleter for MasterSlaveDetermination (SEQUENCE)  */
/* <===================================================> */
void Delete_MasterSlaveDetermination(PS_MasterSlaveDetermination x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for MasterSlaveDeterminationAck (SEQUENCE)  */
/* <======================================================> */
void Delete_MasterSlaveDeterminationAck(PS_MasterSlaveDeterminationAck x)
{
    Delete_Decision(&x->decision);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for Decision (CHOICE)  */
/* <=================================> */
void Delete_Decision(PS_Decision x)
{
    switch (x->index)
    {
        case 0:
            /* (master is NULL) */
            break;
        case 1:
            /* (slave is NULL) */
            break;
        default:
            ErrorMessage("Delete_Decision: Illegal CHOICE index");
    }
}

/* <=========================================================> */
/*  PER-Deleter for MasterSlaveDeterminationReject (SEQUENCE)  */
/* <=========================================================> */
void Delete_MasterSlaveDeterminationReject(PS_MasterSlaveDeterminationReject x)
{
    Delete_MsdRejectCause(&x->msdRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for MsdRejectCause (CHOICE)  */
/* <=======================================> */
void Delete_MsdRejectCause(PS_MsdRejectCause x)
{
    switch (x->index)
    {
        case 0:
            /* (identicalNumbers is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MsdRejectCause: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Deleter for MasterSlaveDeterminationRelease (SEQUENCE)  */
/* <==========================================================> */
void Delete_MasterSlaveDeterminationRelease(PS_MasterSlaveDeterminationRelease x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for TerminalCapabilitySet (SEQUENCE)  */
/* <================================================> */
void Delete_TerminalCapabilitySet(PS_TerminalCapabilitySet x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_genericInformation;
    OSCL_DEFAULT_FREE(x->protocolIdentifier.data);
    if (x->option_of_multiplexCapability)
    {
        Delete_MultiplexCapability(&x->multiplexCapability);
    }
    if (x->option_of_capabilityTable)
    {
        for (i = 0;i < x->size_of_capabilityTable;++i)
        {
            Delete_CapabilityTableEntry(x->capabilityTable + i);
        }
        OSCL_DEFAULT_FREE(x->capabilityTable);
    }
    if (x->option_of_capabilityDescriptors)
    {
        for (i = 0;i < x->size_of_capabilityDescriptors;++i)
        {
            Delete_CapabilityDescriptor(x->capabilityDescriptors + i);
        }
        OSCL_DEFAULT_FREE(x->capabilityDescriptors);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_genericInformation)
    {
        for (i = 0;i < x->size_of_genericInformation;++i)
        {
            Delete_GenericInformation(x->genericInformation + i);
        }
        OSCL_DEFAULT_FREE(x->genericInformation);
    }
}

/* <===============================================> */
/*  PER-Deleter for CapabilityTableEntry (SEQUENCE)  */
/* <===============================================> */
void Delete_CapabilityTableEntry(PS_CapabilityTableEntry x)
{
    if (x->option_of_capability)
    {
        Delete_Capability(&x->capability);
    }
}

/* <===============================================> */
/*  PER-Deleter for CapabilityDescriptor (SEQUENCE)  */
/* <===============================================> */
void Delete_CapabilityDescriptor(PS_CapabilityDescriptor x)
{
    uint16 i = 0;
    if (x->option_of_simultaneousCapabilities)
    {
        for (i = 0;i < x->size_of_simultaneousCapabilities;++i)
        {
            Delete_AlternativeCapabilitySet(x->simultaneousCapabilities + i);
        }
        OSCL_DEFAULT_FREE(x->simultaneousCapabilities);
    }
}

/* <======================================================> */
/*  PER-Deleter for AlternativeCapabilitySet (SEQUENCE-OF)  */
/* <======================================================> */
void Delete_AlternativeCapabilitySet(PS_AlternativeCapabilitySet x)
{
    OSCL_DEFAULT_FREE(x->item);
}

/* <===================================================> */
/*  PER-Deleter for TerminalCapabilitySetAck (SEQUENCE)  */
/* <===================================================> */
void Delete_TerminalCapabilitySetAck(PS_TerminalCapabilitySetAck x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for TerminalCapabilitySetReject (SEQUENCE)  */
/* <======================================================> */
void Delete_TerminalCapabilitySetReject(PS_TerminalCapabilitySetReject x)
{
    Delete_TcsRejectCause(&x->tcsRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for TcsRejectCause (CHOICE)  */
/* <=======================================> */
void Delete_TcsRejectCause(PS_TcsRejectCause x)
{
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
            Delete_TableEntryCapacityExceeded(x->tableEntryCapacityExceeded);
            OSCL_DEFAULT_FREE(x->tableEntryCapacityExceeded);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_TcsRejectCause: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Deleter for TableEntryCapacityExceeded (CHOICE)  */
/* <===================================================> */
void Delete_TableEntryCapacityExceeded(PS_TableEntryCapacityExceeded x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            /* (noneProcessed is NULL) */
            break;
        default:
            ErrorMessage("Delete_TableEntryCapacityExceeded: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Deleter for TerminalCapabilitySetRelease (SEQUENCE)  */
/* <=======================================================> */
void Delete_TerminalCapabilitySetRelease(PS_TerminalCapabilitySetRelease x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Deleter for Capability (CHOICE)  */
/* <===================================> */
void Delete_Capability(PS_Capability x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_VideoCapability(x->receiveVideoCapability);
            OSCL_DEFAULT_FREE(x->receiveVideoCapability);
            break;
        case 2:
            Delete_VideoCapability(x->transmitVideoCapability);
            OSCL_DEFAULT_FREE(x->transmitVideoCapability);
            break;
        case 3:
            Delete_VideoCapability(x->receiveAndTransmitVideoCapability);
            OSCL_DEFAULT_FREE(x->receiveAndTransmitVideoCapability);
            break;
        case 4:
            Delete_AudioCapability(x->receiveAudioCapability);
            OSCL_DEFAULT_FREE(x->receiveAudioCapability);
            break;
        case 5:
            Delete_AudioCapability(x->transmitAudioCapability);
            OSCL_DEFAULT_FREE(x->transmitAudioCapability);
            break;
        case 6:
            Delete_AudioCapability(x->receiveAndTransmitAudioCapability);
            OSCL_DEFAULT_FREE(x->receiveAndTransmitAudioCapability);
            break;
        case 7:
            Delete_DataApplicationCapability(x->receiveDataApplicationCapability);
            OSCL_DEFAULT_FREE(x->receiveDataApplicationCapability);
            break;
        case 8:
            Delete_DataApplicationCapability(x->transmitDataApplicationCapability);
            OSCL_DEFAULT_FREE(x->transmitDataApplicationCapability);
            break;
        case 9:
            Delete_DataApplicationCapability(x->receiveAndTransmitDataApplicationCapability);
            OSCL_DEFAULT_FREE(x->receiveAndTransmitDataApplicationCapability);
            break;
        case 10:
            break;
        case 11:
            Delete_H233EncryptionReceiveCapability(x->h233EncryptionReceiveCapability);
            OSCL_DEFAULT_FREE(x->h233EncryptionReceiveCapability);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 12:
            Delete_ConferenceCapability(x->conferenceCapability);
            OSCL_DEFAULT_FREE(x->conferenceCapability);
            break;
        case 13:
            Delete_H235SecurityCapability(x->h235SecurityCapability);
            OSCL_DEFAULT_FREE(x->h235SecurityCapability);
            break;
        case 14:
            break;
        case 15:
            Delete_UserInputCapability(x->receiveUserInputCapability);
            OSCL_DEFAULT_FREE(x->receiveUserInputCapability);
            break;
        case 16:
            Delete_UserInputCapability(x->transmitUserInputCapability);
            OSCL_DEFAULT_FREE(x->transmitUserInputCapability);
            break;
        case 17:
            Delete_UserInputCapability(x->receiveAndTransmitUserInputCapability);
            OSCL_DEFAULT_FREE(x->receiveAndTransmitUserInputCapability);
            break;
        case 18:
            Delete_GenericCapability(x->genericControlCapability);
            OSCL_DEFAULT_FREE(x->genericControlCapability);
            break;
        default:
            ErrorMessage("Delete_Capability: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Deleter for H233EncryptionReceiveCapability (SEQUENCE)  */
/* <==========================================================> */
void Delete_H233EncryptionReceiveCapability(PS_H233EncryptionReceiveCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for H235SecurityCapability (SEQUENCE)  */
/* <=================================================> */
void Delete_H235SecurityCapability(PS_H235SecurityCapability x)
{
    Delete_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for MultiplexCapability (CHOICE)  */
/* <============================================> */
void Delete_MultiplexCapability(PS_MultiplexCapability x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_H222Capability(x->h222Capability);
            OSCL_DEFAULT_FREE(x->h222Capability);
            break;
        case 2:
            Delete_H223Capability(x->h223Capability);
            OSCL_DEFAULT_FREE(x->h223Capability);
            break;
        case 3:
            Delete_V76Capability(x->v76Capability);
            OSCL_DEFAULT_FREE(x->v76Capability);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 4:
            Delete_H2250Capability(x->h2250Capability);
            OSCL_DEFAULT_FREE(x->h2250Capability);
            break;
        case 5:
            Delete_GenericCapability(x->genericMultiplexCapability);
            OSCL_DEFAULT_FREE(x->genericMultiplexCapability);
            break;
        default:
            ErrorMessage("Delete_MultiplexCapability: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for H222Capability (SEQUENCE)  */
/* <=========================================> */
void Delete_H222Capability(PS_H222Capability x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_vcCapability;++i)
    {
        Delete_VCCapability(x->vcCapability + i);
    }
    OSCL_DEFAULT_FREE(x->vcCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for VCCapability (SEQUENCE)  */
/* <=======================================> */
void Delete_VCCapability(PS_VCCapability x)
{
    uint32 extension;

    extension = x->option_of_aal1ViaGateway;
    if (x->option_of_vccAal1)
    {
        Delete_VccAal1(&x->vccAal1);
    }
    if (x->option_of_vccAal5)
    {
        Delete_VccAal5(&x->vccAal5);
    }
    Delete_AvailableBitRates(&x->availableBitRates);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_aal1ViaGateway)
    {
        Delete_Aal1ViaGateway(&x->aal1ViaGateway);
    }
}

/* <=========================================> */
/*  PER-Deleter for Aal1ViaGateway (SEQUENCE)  */
/* <=========================================> */
void Delete_Aal1ViaGateway(PS_Aal1ViaGateway x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_gatewayAddress;++i)
    {
        Delete_Q2931Address(x->gatewayAddress + i);
    }
    OSCL_DEFAULT_FREE(x->gatewayAddress);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for AvailableBitRates (SEQUENCE)  */
/* <============================================> */
void Delete_AvailableBitRates(PS_AvailableBitRates x)
{
    Delete_VccAal5Type(&x->vccAal5Type);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Deleter for VccAal5Type (CHOICE)  */
/* <====================================> */
void Delete_VccAal5Type(PS_VccAal5Type x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            Delete_RangeOfBitRates(x->rangeOfBitRates);
            OSCL_DEFAULT_FREE(x->rangeOfBitRates);
            break;
        default:
            ErrorMessage("Delete_VccAal5Type: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for RangeOfBitRates (SEQUENCE)  */
/* <==========================================> */
void Delete_RangeOfBitRates(PS_RangeOfBitRates x)
{
    OSCL_UNUSED_ARG(x);
}

/* <==================================> */
/*  PER-Deleter for VccAal5 (SEQUENCE)  */
/* <==================================> */
void Delete_VccAal5(PS_VccAal5 x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for VccAal1 (SEQUENCE)  */
/* <==================================> */
void Delete_VccAal1(PS_VccAal1 x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for H223Capability (SEQUENCE)  */
/* <=========================================> */
void Delete_H223Capability(PS_H223Capability x)
{
    uint32 extension;

    extension = x->option_of_maxMUXPDUSizeCapability |
                x->option_of_nsrpSupport |
                x->option_of_mobileOperationTransmitCapability |
                x->option_of_h223AnnexCCapability;
    Delete_H223MultiplexTableCapability(&x->h223MultiplexTableCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_maxMUXPDUSizeCapability)
    {
    }
    if (x->option_of_nsrpSupport)
    {
    }
    if (x->option_of_mobileOperationTransmitCapability)
    {
        Delete_MobileOperationTransmitCapability(&x->mobileOperationTransmitCapability);
    }
    if (x->option_of_h223AnnexCCapability)
    {
        Delete_H223AnnexCCapability(&x->h223AnnexCCapability);
    }
}

/* <============================================================> */
/*  PER-Deleter for MobileOperationTransmitCapability (SEQUENCE)  */
/* <============================================================> */
void Delete_MobileOperationTransmitCapability(PS_MobileOperationTransmitCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Deleter for H223MultiplexTableCapability (CHOICE)  */
/* <=====================================================> */
void Delete_H223MultiplexTableCapability(PS_H223MultiplexTableCapability x)
{
    switch (x->index)
    {
        case 0:
            /* (basic is NULL) */
            break;
        case 1:
            Delete_Enhanced(x->enhanced);
            OSCL_DEFAULT_FREE(x->enhanced);
            break;
        default:
            ErrorMessage("Delete_H223MultiplexTableCapability: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Deleter for Enhanced (SEQUENCE)  */
/* <===================================> */
void Delete_Enhanced(PS_Enhanced x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for H223AnnexCCapability (SEQUENCE)  */
/* <===============================================> */
void Delete_H223AnnexCCapability(PS_H223AnnexCCapability x)
{
    uint32 extension;

    extension = x->option_of_rsCodeCapability;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_rsCodeCapability)
    {
    }
}

/* <========================================> */
/*  PER-Deleter for V76Capability (SEQUENCE)  */
/* <========================================> */
void Delete_V76Capability(PS_V76Capability x)
{
    Delete_V75Capability(&x->v75Capability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for V75Capability (SEQUENCE)  */
/* <========================================> */
void Delete_V75Capability(PS_V75Capability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for H2250Capability (SEQUENCE)  */
/* <==========================================> */
void Delete_H2250Capability(PS_H2250Capability x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_transportCapability |
                x->option_of_redundancyEncodingCapability |
                x->option_of_logicalChannelSwitchingCapability |
                x->option_of_t120DynamicPortCapability;
    Delete_MultipointCapability(&x->receiveMultipointCapability);
    Delete_MultipointCapability(&x->transmitMultipointCapability);
    Delete_MultipointCapability(&x->receiveAndTransmitMultipointCapability);
    Delete_McCapability(&x->mcCapability);
    Delete_MediaPacketizationCapability(&x->mediaPacketizationCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_transportCapability)
    {
        Delete_TransportCapability(&x->transportCapability);
    }
    if (x->option_of_redundancyEncodingCapability)
    {
        for (i = 0;i < x->size_of_redundancyEncodingCapability;++i)
        {
            Delete_RedundancyEncodingCapability(x->redundancyEncodingCapability + i);
        }
        OSCL_DEFAULT_FREE(x->redundancyEncodingCapability);
    }
    if (x->option_of_logicalChannelSwitchingCapability)
    {
    }
    if (x->option_of_t120DynamicPortCapability)
    {
    }
}

/* <=======================================> */
/*  PER-Deleter for McCapability (SEQUENCE)  */
/* <=======================================> */
void Delete_McCapability(PS_McCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Deleter for MediaPacketizationCapability (SEQUENCE)  */
/* <=======================================================> */
void Delete_MediaPacketizationCapability(PS_MediaPacketizationCapability x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_rtpPayloadType;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_rtpPayloadType)
    {
        for (i = 0;i < x->size_of_rtpPayloadType;++i)
        {
            Delete_RTPPayloadType(x->rtpPayloadType + i);
        }
        OSCL_DEFAULT_FREE(x->rtpPayloadType);
    }
}

/* <=========================================> */
/*  PER-Deleter for RSVPParameters (SEQUENCE)  */
/* <=========================================> */
void Delete_RSVPParameters(PS_RSVPParameters x)
{
    if (x->option_of_qosMode)
    {
        Delete_QOSMode(&x->qosMode);
    }
    if (x->option_of_tokenRate)
    {
    }
    if (x->option_of_bucketSize)
    {
    }
    if (x->option_of_peakRate)
    {
    }
    if (x->option_of_minPoliced)
    {
    }
    if (x->option_of_maxPktSize)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Deleter for QOSMode (CHOICE)  */
/* <================================> */
void Delete_QOSMode(PS_QOSMode x)
{
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
            ErrorMessage("Delete_QOSMode: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for ATMParameters (SEQUENCE)  */
/* <========================================> */
void Delete_ATMParameters(PS_ATMParameters x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for QOSCapability (SEQUENCE)  */
/* <========================================> */
void Delete_QOSCapability(PS_QOSCapability x)
{
    if (x->option_of_nonStandardData)
    {
        Delete_NonStandardParameter(&x->nonStandardData);
    }
    if (x->option_of_rsvpParameters)
    {
        Delete_RSVPParameters(&x->rsvpParameters);
    }
    if (x->option_of_atmParameters)
    {
        Delete_ATMParameters(&x->atmParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for MediaTransportType (CHOICE)  */
/* <===========================================> */
void Delete_MediaTransportType(PS_MediaTransportType x)
{
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
            Delete_Atm_AAL5_compressed(x->atm_AAL5_compressed);
            OSCL_DEFAULT_FREE(x->atm_AAL5_compressed);
            break;
        default:
            ErrorMessage("Delete_MediaTransportType: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Deleter for Atm_AAL5_compressed (SEQUENCE)  */
/* <==============================================> */
void Delete_Atm_AAL5_compressed(PS_Atm_AAL5_compressed x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for MediaChannelCapability (SEQUENCE)  */
/* <=================================================> */
void Delete_MediaChannelCapability(PS_MediaChannelCapability x)
{
    if (x->option_of_mediaTransport)
    {
        Delete_MediaTransportType(&x->mediaTransport);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for TransportCapability (SEQUENCE)  */
/* <==============================================> */
void Delete_TransportCapability(PS_TransportCapability x)
{
    uint16 i = 0;
    if (x->option_of_nonStandard)
    {
        Delete_NonStandardParameter(&x->nonStandard);
    }
    if (x->option_of_qOSCapabilities)
    {
        for (i = 0;i < x->size_of_qOSCapabilities;++i)
        {
            Delete_QOSCapability(x->qOSCapabilities + i);
        }
        OSCL_DEFAULT_FREE(x->qOSCapabilities);
    }
    if (x->option_of_mediaChannelCapabilities)
    {
        for (i = 0;i < x->size_of_mediaChannelCapabilities;++i)
        {
            Delete_MediaChannelCapability(x->mediaChannelCapabilities + i);
        }
        OSCL_DEFAULT_FREE(x->mediaChannelCapabilities);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Deleter for RedundancyEncodingCapability (SEQUENCE)  */
/* <=======================================================> */
void Delete_RedundancyEncodingCapability(PS_RedundancyEncodingCapability x)
{
    Delete_RedundancyEncodingMethod(&x->redundancyEncodingMethod);
    if (x->option_of_secondaryEncoding)
    {
        OSCL_DEFAULT_FREE(x->secondaryEncoding);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for RedundancyEncodingMethod (CHOICE)  */
/* <=================================================> */
void Delete_RedundancyEncodingMethod(PS_RedundancyEncodingMethod x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            /* (rtpAudioRedundancyEncoding is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Delete_RTPH263VideoRedundancyEncoding(x->rtpH263VideoRedundancyEncoding);
            OSCL_DEFAULT_FREE(x->rtpH263VideoRedundancyEncoding);
            break;
        default:
            ErrorMessage("Delete_RedundancyEncodingMethod: Illegal CHOICE index");
    }
}

/* <=========================================================> */
/*  PER-Deleter for RTPH263VideoRedundancyEncoding (SEQUENCE)  */
/* <=========================================================> */
void Delete_RTPH263VideoRedundancyEncoding(PS_RTPH263VideoRedundancyEncoding x)
{
    Delete_FrameToThreadMapping(&x->frameToThreadMapping);
    if (x->option_of_containedThreads)
    {
        OSCL_DEFAULT_FREE(x->containedThreads);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for FrameToThreadMapping (CHOICE)  */
/* <=============================================> */
void Delete_FrameToThreadMapping(PS_FrameToThreadMapping x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            /* (roundrobin is NULL) */
            break;
        case 1:
            for (i = 0;i < x->size;++i)
            {
                Delete_RTPH263VideoRedundancyFrameMapping(x->custom + i);
            }
            OSCL_DEFAULT_FREE(x->custom);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_FrameToThreadMapping: Illegal CHOICE index");
    }
}

/* <=============================================================> */
/*  PER-Deleter for RTPH263VideoRedundancyFrameMapping (SEQUENCE)  */
/* <=============================================================> */
void Delete_RTPH263VideoRedundancyFrameMapping(PS_RTPH263VideoRedundancyFrameMapping x)
{
    OSCL_DEFAULT_FREE(x->frameSequence);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for MultipointCapability (SEQUENCE)  */
/* <===============================================> */
void Delete_MultipointCapability(PS_MultipointCapability x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_mediaDistributionCapability;++i)
    {
        Delete_MediaDistributionCapability(x->mediaDistributionCapability + i);
    }
    OSCL_DEFAULT_FREE(x->mediaDistributionCapability);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for MediaDistributionCapability (SEQUENCE)  */
/* <======================================================> */
void Delete_MediaDistributionCapability(PS_MediaDistributionCapability x)
{
    uint16 i = 0;
    if (x->option_of_centralizedData)
    {
        for (i = 0;i < x->size_of_centralizedData;++i)
        {
            Delete_DataApplicationCapability(x->centralizedData + i);
        }
        OSCL_DEFAULT_FREE(x->centralizedData);
    }
    if (x->option_of_distributedData)
    {
        for (i = 0;i < x->size_of_distributedData;++i)
        {
            Delete_DataApplicationCapability(x->distributedData + i);
        }
        OSCL_DEFAULT_FREE(x->distributedData);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for VideoCapability (CHOICE)  */
/* <========================================> */
void Delete_VideoCapability(PS_VideoCapability x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_H261VideoCapability(x->h261VideoCapability);
            OSCL_DEFAULT_FREE(x->h261VideoCapability);
            break;
        case 2:
            Delete_H262VideoCapability(x->h262VideoCapability);
            OSCL_DEFAULT_FREE(x->h262VideoCapability);
            break;
        case 3:
            Delete_H263VideoCapability(x->h263VideoCapability);
            OSCL_DEFAULT_FREE(x->h263VideoCapability);
            break;
        case 4:
            Delete_IS11172VideoCapability(x->is11172VideoCapability);
            OSCL_DEFAULT_FREE(x->is11172VideoCapability);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Delete_GenericCapability(x->genericVideoCapability);
            OSCL_DEFAULT_FREE(x->genericVideoCapability);
            break;
        default:
            ErrorMessage("Delete_VideoCapability: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Deleter for H261VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Delete_H261VideoCapability(PS_H261VideoCapability x)
{
    if (x->option_of_qcifMPI)
    {
    }
    if (x->option_of_cifMPI)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for H262VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Delete_H262VideoCapability(PS_H262VideoCapability x)
{
    if (x->option_of_videoBitRate)
    {
    }
    if (x->option_of_vbvBufferSize)
    {
    }
    if (x->option_of_samplesPerLine)
    {
    }
    if (x->option_of_linesPerFrame)
    {
    }
    if (x->option_of_framesPerSecond)
    {
    }
    if (x->option_of_luminanceSampleRate)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for H263VideoCapability (SEQUENCE)  */
/* <==============================================> */
void Delete_H263VideoCapability(PS_H263VideoCapability x)
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
    if (x->option_of_sqcifMPI)
    {
    }
    if (x->option_of_qcifMPI)
    {
    }
    if (x->option_of_cifMPI)
    {
    }
    if (x->option_of_cif4MPI)
    {
    }
    if (x->option_of_cif16MPI)
    {
    }
    if (x->option_of_hrd_B)
    {
    }
    if (x->option_of_bppMaxKb)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_slowSqcifMPI)
    {
    }
    if (x->option_of_slowQcifMPI)
    {
    }
    if (x->option_of_slowCifMPI)
    {
    }
    if (x->option_of_slowCif4MPI)
    {
    }
    if (x->option_of_slowCif16MPI)
    {
    }
    if (x->option_of_errorCompensation)
    {
    }
    if (x->option_of_enhancementLayerInfo)
    {
        Delete_EnhancementLayerInfo(&x->enhancementLayerInfo);
    }
    if (x->option_of_h263Options)
    {
        Delete_H263Options(&x->h263Options);
    }
}

/* <===============================================> */
/*  PER-Deleter for EnhancementLayerInfo (SEQUENCE)  */
/* <===============================================> */
void Delete_EnhancementLayerInfo(PS_EnhancementLayerInfo x)
{
    uint16 i = 0;
    if (x->option_of_snrEnhancement)
    {
        for (i = 0;i < x->size_of_snrEnhancement;++i)
        {
            Delete_EnhancementOptions(x->snrEnhancement + i);
        }
        OSCL_DEFAULT_FREE(x->snrEnhancement);
    }
    if (x->option_of_spatialEnhancement)
    {
        for (i = 0;i < x->size_of_spatialEnhancement;++i)
        {
            Delete_EnhancementOptions(x->spatialEnhancement + i);
        }
        OSCL_DEFAULT_FREE(x->spatialEnhancement);
    }
    if (x->option_of_bPictureEnhancement)
    {
        for (i = 0;i < x->size_of_bPictureEnhancement;++i)
        {
            Delete_BEnhancementParameters(x->bPictureEnhancement + i);
        }
        OSCL_DEFAULT_FREE(x->bPictureEnhancement);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for BEnhancementParameters (SEQUENCE)  */
/* <=================================================> */
void Delete_BEnhancementParameters(PS_BEnhancementParameters x)
{
    Delete_EnhancementOptions(&x->enhancementOptions);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for EnhancementOptions (SEQUENCE)  */
/* <=============================================> */
void Delete_EnhancementOptions(PS_EnhancementOptions x)
{
    if (x->option_of_sqcifMPI)
    {
    }
    if (x->option_of_qcifMPI)
    {
    }
    if (x->option_of_cifMPI)
    {
    }
    if (x->option_of_cif4MPI)
    {
    }
    if (x->option_of_cif16MPI)
    {
    }
    if (x->option_of_slowSqcifMPI)
    {
    }
    if (x->option_of_slowQcifMPI)
    {
    }
    if (x->option_of_slowCifMPI)
    {
    }
    if (x->option_of_slowCif4MPI)
    {
    }
    if (x->option_of_slowCif16MPI)
    {
    }
    if (x->option_of_h263Options)
    {
        Delete_H263Options(&x->h263Options);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for H263Options (SEQUENCE)  */
/* <======================================> */
void Delete_H263Options(PS_H263Options x)
{
    uint16 i = 0;
    if (x->option_of_transparencyParameters)
    {
        Delete_TransparencyParameters(&x->transparencyParameters);
    }
    if (x->option_of_refPictureSelection)
    {
        Delete_RefPictureSelection(&x->refPictureSelection);
    }
    if (x->option_of_customPictureClockFrequency)
    {
        for (i = 0;i < x->size_of_customPictureClockFrequency;++i)
        {
            Delete_CustomPictureClockFrequency(x->customPictureClockFrequency + i);
        }
        OSCL_DEFAULT_FREE(x->customPictureClockFrequency);
    }
    if (x->option_of_customPictureFormat)
    {
        for (i = 0;i < x->size_of_customPictureFormat;++i)
        {
            Delete_CustomPictureFormat(x->customPictureFormat + i);
        }
        OSCL_DEFAULT_FREE(x->customPictureFormat);
    }
    if (x->option_of_modeCombos)
    {
        for (i = 0;i < x->size_of_modeCombos;++i)
        {
            Delete_H263VideoModeCombos(x->modeCombos + i);
        }
        OSCL_DEFAULT_FREE(x->modeCombos);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for TransparencyParameters (SEQUENCE)  */
/* <=================================================> */
void Delete_TransparencyParameters(PS_TransparencyParameters x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for RefPictureSelection (SEQUENCE)  */
/* <==============================================> */
void Delete_RefPictureSelection(PS_RefPictureSelection x)
{
    if (x->option_of_additionalPictureMemory)
    {
        Delete_AdditionalPictureMemory(&x->additionalPictureMemory);
    }
    Delete_VideoBackChannelSend(&x->videoBackChannelSend);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for VideoBackChannelSend (CHOICE)  */
/* <=============================================> */
void Delete_VideoBackChannelSend(PS_VideoBackChannelSend x)
{
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
            ErrorMessage("Delete_VideoBackChannelSend: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Deleter for AdditionalPictureMemory (SEQUENCE)  */
/* <==================================================> */
void Delete_AdditionalPictureMemory(PS_AdditionalPictureMemory x)
{
    if (x->option_of_sqcifAdditionalPictureMemory)
    {
    }
    if (x->option_of_qcifAdditionalPictureMemory)
    {
    }
    if (x->option_of_cifAdditionalPictureMemory)
    {
    }
    if (x->option_of_cif4AdditionalPictureMemory)
    {
    }
    if (x->option_of_cif16AdditionalPictureMemory)
    {
    }
    if (x->option_of_bigCpfAdditionalPictureMemory)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for CustomPictureClockFrequency (SEQUENCE)  */
/* <======================================================> */
void Delete_CustomPictureClockFrequency(PS_CustomPictureClockFrequency x)
{
    if (x->option_of_sqcifMPI)
    {
    }
    if (x->option_of_qcifMPI)
    {
    }
    if (x->option_of_cifMPI)
    {
    }
    if (x->option_of_cif4MPI)
    {
    }
    if (x->option_of_cif16MPI)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for CustomPictureFormat (SEQUENCE)  */
/* <==============================================> */
void Delete_CustomPictureFormat(PS_CustomPictureFormat x)
{
    Delete_MPI(&x->mPI);
    Delete_PixelAspectInformation(&x->pixelAspectInformation);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for PixelAspectInformation (CHOICE)  */
/* <===============================================> */
void Delete_PixelAspectInformation(PS_PixelAspectInformation x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            OSCL_DEFAULT_FREE(x->pixelAspectCode);
            break;
        case 2:
            for (i = 0;i < x->size;++i)
            {
                Delete_ExtendedPARItem(x->extendedPAR + i);
            }
            OSCL_DEFAULT_FREE(x->extendedPAR);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_PixelAspectInformation: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for ExtendedPARItem (SEQUENCE)  */
/* <==========================================> */
void Delete_ExtendedPARItem(PS_ExtendedPARItem x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================> */
/*  PER-Deleter for MPI (SEQUENCE)  */
/* <==============================> */
void Delete_MPI(PS_MPI x)
{
    uint16 i = 0;
    if (x->option_of_standardMPI)
    {
    }
    if (x->option_of_customPCF)
    {
        for (i = 0;i < x->size_of_customPCF;++i)
        {
            Delete_CustomPCFItem(x->customPCF + i);
        }
        OSCL_DEFAULT_FREE(x->customPCF);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for CustomPCFItem (SEQUENCE)  */
/* <========================================> */
void Delete_CustomPCFItem(PS_CustomPCFItem x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for H263VideoModeCombos (SEQUENCE)  */
/* <==============================================> */
void Delete_H263VideoModeCombos(PS_H263VideoModeCombos x)
{
    uint16 i = 0;
    Delete_H263ModeComboFlags(&x->h263VideoUncoupledModes);
    for (i = 0;i < x->size_of_h263VideoCoupledModes;++i)
    {
        Delete_H263ModeComboFlags(x->h263VideoCoupledModes + i);
    }
    OSCL_DEFAULT_FREE(x->h263VideoCoupledModes);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for H263ModeComboFlags (SEQUENCE)  */
/* <=============================================> */
void Delete_H263ModeComboFlags(PS_H263ModeComboFlags x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for IS11172VideoCapability (SEQUENCE)  */
/* <=================================================> */
void Delete_IS11172VideoCapability(PS_IS11172VideoCapability x)
{
    if (x->option_of_videoBitRate)
    {
    }
    if (x->option_of_vbvBufferSize)
    {
    }
    if (x->option_of_samplesPerLine)
    {
    }
    if (x->option_of_linesPerFrame)
    {
    }
    if (x->option_of_pictureRate)
    {
    }
    if (x->option_of_luminanceSampleRate)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for AudioCapability (CHOICE)  */
/* <========================================> */
void Delete_AudioCapability(PS_AudioCapability x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            Delete_G7231(x->g7231);
            OSCL_DEFAULT_FREE(x->g7231);
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            break;
        case 12:
            Delete_IS11172AudioCapability(x->is11172AudioCapability);
            OSCL_DEFAULT_FREE(x->is11172AudioCapability);
            break;
        case 13:
            Delete_IS13818AudioCapability(x->is13818AudioCapability);
            OSCL_DEFAULT_FREE(x->is13818AudioCapability);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            break;
        case 15:
            break;
        case 16:
            Delete_G7231AnnexCCapability(x->g7231AnnexCCapability);
            OSCL_DEFAULT_FREE(x->g7231AnnexCCapability);
            break;
        case 17:
            Delete_GSMAudioCapability(x->gsmFullRate);
            OSCL_DEFAULT_FREE(x->gsmFullRate);
            break;
        case 18:
            Delete_GSMAudioCapability(x->gsmHalfRate);
            OSCL_DEFAULT_FREE(x->gsmHalfRate);
            break;
        case 19:
            Delete_GSMAudioCapability(x->gsmEnhancedFullRate);
            OSCL_DEFAULT_FREE(x->gsmEnhancedFullRate);
            break;
        case 20:
            Delete_GenericCapability(x->genericAudioCapability);
            OSCL_DEFAULT_FREE(x->genericAudioCapability);
            break;
        case 21:
            Delete_G729Extensions(x->g729Extensions);
            OSCL_DEFAULT_FREE(x->g729Extensions);
            break;
        default:
            ErrorMessage("Delete_AudioCapability: Illegal CHOICE index");
    }
}

/* <================================> */
/*  PER-Deleter for G7231 (SEQUENCE)  */
/* <================================> */
void Delete_G7231(PS_G7231 x)
{
    OSCL_UNUSED_ARG(x);
}

/* <=========================================> */
/*  PER-Deleter for G729Extensions (SEQUENCE)  */
/* <=========================================> */
void Delete_G729Extensions(PS_G729Extensions x)
{
    if (x->option_of_audioUnit)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for G7231AnnexCCapability (SEQUENCE)  */
/* <================================================> */
void Delete_G7231AnnexCCapability(PS_G7231AnnexCCapability x)
{
    if (x->option_of_g723AnnexCAudioMode)
    {
        Delete_G723AnnexCAudioMode(&x->g723AnnexCAudioMode);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for G723AnnexCAudioMode (SEQUENCE)  */
/* <==============================================> */
void Delete_G723AnnexCAudioMode(PS_G723AnnexCAudioMode x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for IS11172AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Delete_IS11172AudioCapability(PS_IS11172AudioCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for IS13818AudioCapability (SEQUENCE)  */
/* <=================================================> */
void Delete_IS13818AudioCapability(PS_IS13818AudioCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for GSMAudioCapability (SEQUENCE)  */
/* <=============================================> */
void Delete_GSMAudioCapability(PS_GSMAudioCapability x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Deleter for DataApplicationCapability (SEQUENCE)  */
/* <====================================================> */
void Delete_DataApplicationCapability(PS_DataApplicationCapability x)
{
    Delete_Application(&x->application);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Deleter for Application (CHOICE)  */
/* <====================================> */
void Delete_Application(PS_Application x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_DataProtocolCapability(x->t120);
            OSCL_DEFAULT_FREE(x->t120);
            break;
        case 2:
            Delete_DataProtocolCapability(x->dsm_cc);
            OSCL_DEFAULT_FREE(x->dsm_cc);
            break;
        case 3:
            Delete_DataProtocolCapability(x->userData);
            OSCL_DEFAULT_FREE(x->userData);
            break;
        case 4:
            Delete_T84(x->t84);
            OSCL_DEFAULT_FREE(x->t84);
            break;
        case 5:
            Delete_DataProtocolCapability(x->t434);
            OSCL_DEFAULT_FREE(x->t434);
            break;
        case 6:
            Delete_DataProtocolCapability(x->h224);
            OSCL_DEFAULT_FREE(x->h224);
            break;
        case 7:
            Delete_Nlpid(x->nlpid);
            OSCL_DEFAULT_FREE(x->nlpid);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            Delete_DataProtocolCapability(x->h222DataPartitioning);
            OSCL_DEFAULT_FREE(x->h222DataPartitioning);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Delete_DataProtocolCapability(x->t30fax);
            OSCL_DEFAULT_FREE(x->t30fax);
            break;
        case 11:
            Delete_DataProtocolCapability(x->t140);
            OSCL_DEFAULT_FREE(x->t140);
            break;
        case 12:
            Delete_T38fax(x->t38fax);
            OSCL_DEFAULT_FREE(x->t38fax);
            break;
        case 13:
            Delete_GenericCapability(x->genericDataCapability);
            OSCL_DEFAULT_FREE(x->genericDataCapability);
            break;
        default:
            ErrorMessage("Delete_Application: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Deleter for T38fax (SEQUENCE)  */
/* <=================================> */
void Delete_T38fax(PS_T38fax x)
{
    Delete_DataProtocolCapability(&x->t38FaxProtocol);
    Delete_T38FaxProfile(&x->t38FaxProfile);
}

/* <================================> */
/*  PER-Deleter for Nlpid (SEQUENCE)  */
/* <================================> */
void Delete_Nlpid(PS_Nlpid x)
{
    if (x->nlpidData.size > 0) OSCL_DEFAULT_FREE(x->nlpidData.data);
    Delete_DataProtocolCapability(&x->nlpidProtocol);
}

/* <==============================> */
/*  PER-Deleter for T84 (SEQUENCE)  */
/* <==============================> */
void Delete_T84(PS_T84 x)
{
    Delete_DataProtocolCapability(&x->t84Protocol);
    Delete_T84Profile(&x->t84Profile);
}

/* <===============================================> */
/*  PER-Deleter for DataProtocolCapability (CHOICE)  */
/* <===============================================> */
void Delete_DataProtocolCapability(PS_DataProtocolCapability x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            break;
        case 8:
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            Delete_V76wCompression(x->v76wCompression);
            OSCL_DEFAULT_FREE(x->v76wCompression);
            break;
        case 12:
            break;
        case 13:
            break;
        default:
            ErrorMessage("Delete_DataProtocolCapability: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for V76wCompression (CHOICE)  */
/* <========================================> */
void Delete_V76wCompression(PS_V76wCompression x)
{
    switch (x->index)
    {
        case 0:
            Delete_CompressionType(x->transmitCompression);
            OSCL_DEFAULT_FREE(x->transmitCompression);
            break;
        case 1:
            Delete_CompressionType(x->receiveCompression);
            OSCL_DEFAULT_FREE(x->receiveCompression);
            break;
        case 2:
            Delete_CompressionType(x->transmitAndReceiveCompression);
            OSCL_DEFAULT_FREE(x->transmitAndReceiveCompression);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_V76wCompression: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for CompressionType (CHOICE)  */
/* <========================================> */
void Delete_CompressionType(PS_CompressionType x)
{
    switch (x->index)
    {
        case 0:
            Delete_V42bis(x->v42bis);
            OSCL_DEFAULT_FREE(x->v42bis);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_CompressionType: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Deleter for V42bis (SEQUENCE)  */
/* <=================================> */
void Delete_V42bis(PS_V42bis x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Deleter for T84Profile (CHOICE)  */
/* <===================================> */
void Delete_T84Profile(PS_T84Profile x)
{
    switch (x->index)
    {
        case 0:
            /* (t84Unrestricted is NULL) */
            break;
        case 1:
            Delete_T84Restricted(x->t84Restricted);
            OSCL_DEFAULT_FREE(x->t84Restricted);
            break;
        default:
            ErrorMessage("Delete_T84Profile: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for T84Restricted (SEQUENCE)  */
/* <========================================> */
void Delete_T84Restricted(PS_T84Restricted x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for T38FaxProfile (SEQUENCE)  */
/* <========================================> */
void Delete_T38FaxProfile(PS_T38FaxProfile x)
{
    uint32 extension;

    extension = x->option_of_version |
                x->option_of_t38FaxRateManagement |
                x->option_of_t38FaxUdpOptions;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_version)
    {
    }
    if (x->option_of_t38FaxRateManagement)
    {
        Delete_T38FaxRateManagement(&x->t38FaxRateManagement);
    }
    if (x->option_of_t38FaxUdpOptions)
    {
        Delete_T38FaxUdpOptions(&x->t38FaxUdpOptions);
    }
}

/* <=============================================> */
/*  PER-Deleter for T38FaxRateManagement (CHOICE)  */
/* <=============================================> */
void Delete_T38FaxRateManagement(PS_T38FaxRateManagement x)
{
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
            ErrorMessage("Delete_T38FaxRateManagement: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for T38FaxUdpOptions (SEQUENCE)  */
/* <===========================================> */
void Delete_T38FaxUdpOptions(PS_T38FaxUdpOptions x)
{
    if (x->option_of_t38FaxMaxBuffer)
    {
    }
    if (x->option_of_t38FaxMaxDatagram)
    {
    }
    Delete_T38FaxUdpEC(&x->t38FaxUdpEC);
}

/* <====================================> */
/*  PER-Deleter for T38FaxUdpEC (CHOICE)  */
/* <====================================> */
void Delete_T38FaxUdpEC(PS_T38FaxUdpEC x)
{
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
            ErrorMessage("Delete_T38FaxUdpEC: Illegal CHOICE index");
    }
}

/* <===============================================================> */
/*  PER-Deleter for EncryptionAuthenticationAndIntegrity (SEQUENCE)  */
/* <===============================================================> */
void Delete_EncryptionAuthenticationAndIntegrity(PS_EncryptionAuthenticationAndIntegrity x)
{
    if (x->option_of_encryptionCapability)
    {
        Delete_EncryptionCapability(&x->encryptionCapability);
    }
    if (x->option_of_authenticationCapability)
    {
        Delete_AuthenticationCapability(&x->authenticationCapability);
    }
    if (x->option_of_integrityCapability)
    {
        Delete_IntegrityCapability(&x->integrityCapability);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for EncryptionCapability (SEQUENCE-OF)  */
/* <==================================================> */
void Delete_EncryptionCapability(PS_EncryptionCapability x)
{
    uint16 i;
    for (i = 0;i < x->size;++i)
    {
        Delete_MediaEncryptionAlgorithm(x->item + i);
    }
    OSCL_DEFAULT_FREE(x->item);
}

/* <=================================================> */
/*  PER-Deleter for MediaEncryptionAlgorithm (CHOICE)  */
/* <=================================================> */
void Delete_MediaEncryptionAlgorithm(PS_MediaEncryptionAlgorithm x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            FreeObjectID(x->algorithm);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MediaEncryptionAlgorithm: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Deleter for AuthenticationCapability (SEQUENCE)  */
/* <===================================================> */
void Delete_AuthenticationCapability(PS_AuthenticationCapability x)
{
    if (x->option_of_nonStandard)
    {
        Delete_NonStandardParameter(&x->nonStandard);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for IntegrityCapability (SEQUENCE)  */
/* <==============================================> */
void Delete_IntegrityCapability(PS_IntegrityCapability x)
{
    if (x->option_of_nonStandard)
    {
        Delete_NonStandardParameter(&x->nonStandard);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for UserInputCapability (CHOICE)  */
/* <============================================> */
void Delete_UserInputCapability(PS_UserInputCapability x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            for (i = 0;i < x->size;++i)
            {
                Delete_NonStandardParameter(x->nonStandard + i);
            }
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            ErrorMessage("Delete_UserInputCapability: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Deleter for ConferenceCapability (SEQUENCE)  */
/* <===============================================> */
void Delete_ConferenceCapability(PS_ConferenceCapability x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_VideoIndicateMixingCapability |
                x->option_of_multipointVisualizationCapability;
    if (x->option_of_nonStandardData)
    {
        for (i = 0;i < x->size_of_nonStandardData;++i)
        {
            Delete_NonStandardParameter(x->nonStandardData + i);
        }
        OSCL_DEFAULT_FREE(x->nonStandardData);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_VideoIndicateMixingCapability)
    {
    }
    if (x->option_of_multipointVisualizationCapability)
    {
    }
}

/* <============================================> */
/*  PER-Deleter for GenericCapability (SEQUENCE)  */
/* <============================================> */
void Delete_GenericCapability(PS_GenericCapability x)
{
    uint16 i = 0;
    Delete_CapabilityIdentifier(&x->capabilityIdentifier);
    if (x->option_of_maxBitRate)
    {
    }
    if (x->option_of_collapsing)
    {
        for (i = 0;i < x->size_of_collapsing;++i)
        {
            Delete_GenericParameter(x->collapsing + i);
        }
        OSCL_DEFAULT_FREE(x->collapsing);
    }
    if (x->option_of_nonCollapsing)
    {
        for (i = 0;i < x->size_of_nonCollapsing;++i)
        {
            Delete_GenericParameter(x->nonCollapsing + i);
        }
        OSCL_DEFAULT_FREE(x->nonCollapsing);
    }
    if (x->option_of_nonCollapsingRaw)
    {
        if (x->nonCollapsingRaw.size > 0) OSCL_DEFAULT_FREE(x->nonCollapsingRaw.data);
    }
    if (x->option_of_transport)
    {
        Delete_DataProtocolCapability(&x->transport);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for CapabilityIdentifier (CHOICE)  */
/* <=============================================> */
void Delete_CapabilityIdentifier(PS_CapabilityIdentifier x)
{
    switch (x->index)
    {
        case 0:
            FreeObjectID(x->standard);
            break;
        case 1:
            Delete_NonStandardParameter(x->h221NonStandard);
            OSCL_DEFAULT_FREE(x->h221NonStandard);
            break;
        case 2:
            FreeOctetString(x->uuid);
            break;
        case 3:
            FreeCharString(x->domainBased);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_CapabilityIdentifier: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for GenericParameter (SEQUENCE)  */
/* <===========================================> */
void Delete_GenericParameter(PS_GenericParameter x)
{
    uint16 i = 0;
    Delete_ParameterIdentifier(&x->parameterIdentifier);
    Delete_ParameterValue(&x->parameterValue);
    if (x->option_of_supersedes)
    {
        for (i = 0;i < x->size_of_supersedes;++i)
        {
            Delete_ParameterIdentifier(x->supersedes + i);
        }
        OSCL_DEFAULT_FREE(x->supersedes);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for ParameterIdentifier (CHOICE)  */
/* <============================================> */
void Delete_ParameterIdentifier(PS_ParameterIdentifier x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            Delete_NonStandardParameter(x->h221NonStandard);
            OSCL_DEFAULT_FREE(x->h221NonStandard);
            break;
        case 2:
            FreeOctetString(x->uuid);
            break;
        case 3:
            FreeCharString(x->domainBased);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_ParameterIdentifier: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for ParameterValue (CHOICE)  */
/* <=======================================> */
void Delete_ParameterValue(PS_ParameterValue x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            /* (logical is NULL) */
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            FreeOctetString(x->octetString);
            break;
        case 7:
            for (i = 0;i < x->size;++i)
            {
                Delete_GenericParameter(x->genericParameter + i);
            }
            OSCL_DEFAULT_FREE(x->genericParameter);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_ParameterValue: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for OpenLogicalChannel (SEQUENCE)  */
/* <=============================================> */
void Delete_OpenLogicalChannel(PS_OpenLogicalChannel x)
{
    uint32 extension;

    extension = x->option_of_separateStack |
                x->option_of_encryptionSync;
    Delete_ForwardLogicalChannelParameters(&x->forwardLogicalChannelParameters);
    if (x->option_of_reverseLogicalChannelParameters)
    {
        Delete_ReverseLogicalChannelParameters(&x->reverseLogicalChannelParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_separateStack)
    {
        Delete_NetworkAccessParameters(&x->separateStack);
    }
    if (x->option_of_encryptionSync)
    {
        Delete_EncryptionSync(&x->encryptionSync);
    }
}

/* <==========================================================> */
/*  PER-Deleter for ReverseLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Delete_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x)
{
    uint32 extension;

    extension = x->option_of_reverseLogicalChannelDependency |
                x->option_of_replacementFor;
    Delete_DataType(&x->dataType);
    if (x->option_of_rlcMultiplexParameters)
    {
        Delete_RlcMultiplexParameters(&x->rlcMultiplexParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_reverseLogicalChannelDependency)
    {
    }
    if (x->option_of_replacementFor)
    {
    }
}

/* <===============================================> */
/*  PER-Deleter for RlcMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Delete_RlcMultiplexParameters(PS_RlcMultiplexParameters x)
{
    switch (x->index)
    {
        case 0:
            Delete_H223LogicalChannelParameters(x->h223LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h223LogicalChannelParameters);
            break;
        case 1:
            Delete_V76LogicalChannelParameters(x->v76LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->v76LogicalChannelParameters);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Delete_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h2250LogicalChannelParameters);
            break;
        default:
            ErrorMessage("Delete_RlcMultiplexParameters: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Deleter for ForwardLogicalChannelParameters (SEQUENCE)  */
/* <==========================================================> */
void Delete_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x)
{
    uint32 extension;

    extension = x->option_of_forwardLogicalChannelDependency |
                x->option_of_replacementFor;
    if (x->option_of_portNumber)
    {
    }
    Delete_DataType(&x->dataType);
    Delete_MultiplexParameters(&x->multiplexParameters);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_forwardLogicalChannelDependency)
    {
    }
    if (x->option_of_replacementFor)
    {
    }
}

/* <============================================> */
/*  PER-Deleter for MultiplexParameters (CHOICE)  */
/* <============================================> */
void Delete_MultiplexParameters(PS_MultiplexParameters x)
{
    switch (x->index)
    {
        case 0:
            Delete_H222LogicalChannelParameters(x->h222LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h222LogicalChannelParameters);
            break;
        case 1:
            Delete_H223LogicalChannelParameters(x->h223LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h223LogicalChannelParameters);
            break;
        case 2:
            Delete_V76LogicalChannelParameters(x->v76LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->v76LogicalChannelParameters);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            Delete_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h2250LogicalChannelParameters);
            break;
        case 4:
            break;
        default:
            ErrorMessage("Delete_MultiplexParameters: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Deleter for NetworkAccessParameters (SEQUENCE)  */
/* <==================================================> */
void Delete_NetworkAccessParameters(PS_NetworkAccessParameters x)
{
    uint32 extension;

    extension = x->option_of_t120SetupProcedure;
    if (x->option_of_distribution)
    {
        Delete_Distribution(&x->distribution);
    }
    Delete_NetworkAddress(&x->networkAddress);
    if (x->option_of_externalReference)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_t120SetupProcedure)
    {
        Delete_T120SetupProcedure(&x->t120SetupProcedure);
    }
}

/* <===========================================> */
/*  PER-Deleter for T120SetupProcedure (CHOICE)  */
/* <===========================================> */
void Delete_T120SetupProcedure(PS_T120SetupProcedure x)
{
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
            ErrorMessage("Delete_T120SetupProcedure: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for NetworkAddress (CHOICE)  */
/* <=======================================> */
void Delete_NetworkAddress(PS_NetworkAddress x)
{
    switch (x->index)
    {
        case 0:
            Delete_Q2931Address(x->q2931Address);
            OSCL_DEFAULT_FREE(x->q2931Address);
            break;
        case 1:
            FreeCharString(x->e164Address);
            break;
        case 2:
            Delete_TransportAddress(x->localAreaAddress);
            OSCL_DEFAULT_FREE(x->localAreaAddress);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_NetworkAddress: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for Distribution (CHOICE)  */
/* <=====================================> */
void Delete_Distribution(PS_Distribution x)
{
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
            ErrorMessage("Delete_Distribution: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for Q2931Address (SEQUENCE)  */
/* <=======================================> */
void Delete_Q2931Address(PS_Q2931Address x)
{
    Delete_Address(&x->address);
    if (x->option_of_subaddress)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Deleter for Address (CHOICE)  */
/* <================================> */
void Delete_Address(PS_Address x)
{
    switch (x->index)
    {
        case 0:
            FreeCharString(x->internationalNumber);
            break;
        case 1:
            FreeOctetString(x->nsapAddress);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_Address: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for V75Parameters (SEQUENCE)  */
/* <========================================> */
void Delete_V75Parameters(PS_V75Parameters x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for DataType (CHOICE)  */
/* <=================================> */
OSCL_EXPORT_REF void Delete_DataType(PS_DataType x)
{
    switch (x->index)
    {
        case 0:
            if (x->nonStandard)
            {
                Delete_NonStandardParameter(x->nonStandard);
                OSCL_DEFAULT_FREE(x->nonStandard);
            }
            break;
        case 1:
            /* (nullData is NULL) */
            break;
        case 2:
            Delete_VideoCapability(x->videoData);
            OSCL_DEFAULT_FREE(x->videoData);
            break;
        case 3:
            Delete_AudioCapability(x->audioData);
            OSCL_DEFAULT_FREE(x->audioData);
            break;
        case 4:
            Delete_DataApplicationCapability(x->data);
            OSCL_DEFAULT_FREE(x->data);
            break;
        case 5:
            Delete_EncryptionMode(x->encryptionData);
            OSCL_DEFAULT_FREE(x->encryptionData);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Delete_NonStandardParameter(x->h235Control);
            OSCL_DEFAULT_FREE(x->h235Control);
            break;
        case 7:
            Delete_H235Media(x->h235Media);
            OSCL_DEFAULT_FREE(x->h235Media);
            break;
        default:
            ErrorMessage("Delete_DataType: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Deleter for H235Media (SEQUENCE)  */
/* <====================================> */
void Delete_H235Media(PS_H235Media x)
{
    Delete_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity);
    Delete_MediaType(&x->mediaType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for MediaType (CHOICE)  */
/* <==================================> */
void Delete_MediaType(PS_MediaType x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_VideoCapability(x->videoData);
            OSCL_DEFAULT_FREE(x->videoData);
            break;
        case 2:
            Delete_AudioCapability(x->audioData);
            OSCL_DEFAULT_FREE(x->audioData);
            break;
        case 3:
            Delete_DataApplicationCapability(x->data);
            OSCL_DEFAULT_FREE(x->data);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MediaType: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Deleter for H222LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
void Delete_H222LogicalChannelParameters(PS_H222LogicalChannelParameters x)
{
    if (x->option_of_pcr_pid)
    {
    }
    if (x->option_of_programDescriptors)
    {
        if (x->programDescriptors.size > 0) OSCL_DEFAULT_FREE(x->programDescriptors.data);
    }
    if (x->option_of_streamDescriptors)
    {
        if (x->streamDescriptors.size > 0) OSCL_DEFAULT_FREE(x->streamDescriptors.data);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Deleter for H223LogicalChannelParameters (SEQUENCE)  */
/* <=======================================================> */
OSCL_EXPORT_REF void Delete_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x)
{
    Delete_AdaptationLayerType(&x->adaptationLayerType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for AdaptationLayerType (CHOICE)  */
/* <============================================> */
void Delete_AdaptationLayerType(PS_AdaptationLayerType x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            Delete_Al3(x->al3);
            OSCL_DEFAULT_FREE(x->al3);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Delete_H223AL1MParameters(x->al1M);
            OSCL_DEFAULT_FREE(x->al1M);
            break;
        case 7:
            Delete_H223AL2MParameters(x->al2M);
            OSCL_DEFAULT_FREE(x->al2M);
            break;
        case 8:
            Delete_H223AL3MParameters(x->al3M);
            OSCL_DEFAULT_FREE(x->al3M);
            break;
        default:
            ErrorMessage("Delete_AdaptationLayerType: Illegal CHOICE index");
    }
}

/* <==============================> */
/*  PER-Deleter for Al3 (SEQUENCE)  */
/* <==============================> */
void Delete_Al3(PS_Al3 x)
{
    OSCL_UNUSED_ARG(x);
}

/* <=============================================> */
/*  PER-Deleter for H223AL1MParameters (SEQUENCE)  */
/* <=============================================> */
void Delete_H223AL1MParameters(PS_H223AL1MParameters x)
{
    uint32 extension;

    extension = x->option_of_rsCodeCorrection;
    Delete_TransferMode(&x->transferMode);
    Delete_HeaderFEC(&x->headerFEC);
    Delete_CrcLength(&x->crcLength);
    Delete_ArqType(&x->arqType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_rsCodeCorrection)
    {
    }
}

/* <================================> */
/*  PER-Deleter for ArqType (CHOICE)  */
/* <================================> */
void Delete_ArqType(PS_ArqType x)
{
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            Delete_H223AnnexCArqParameters(x->typeIArq);
            OSCL_DEFAULT_FREE(x->typeIArq);
            break;
        case 2:
            Delete_H223AnnexCArqParameters(x->typeIIArq);
            OSCL_DEFAULT_FREE(x->typeIIArq);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_ArqType: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for CrcLength (CHOICE)  */
/* <==================================> */
void Delete_CrcLength(PS_CrcLength x)
{
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
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        default:
            ErrorMessage("Delete_CrcLength: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for HeaderFEC (CHOICE)  */
/* <==================================> */
void Delete_HeaderFEC(PS_HeaderFEC x)
{
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
            ErrorMessage("Delete_HeaderFEC: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for TransferMode (CHOICE)  */
/* <=====================================> */
void Delete_TransferMode(PS_TransferMode x)
{
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
            ErrorMessage("Delete_TransferMode: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for H223AL2MParameters (SEQUENCE)  */
/* <=============================================> */
void Delete_H223AL2MParameters(PS_H223AL2MParameters x)
{
    Delete_Al2HeaderFEC(&x->al2HeaderFEC);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for Al2HeaderFEC (CHOICE)  */
/* <=====================================> */
void Delete_Al2HeaderFEC(PS_Al2HeaderFEC x)
{
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
            ErrorMessage("Delete_Al2HeaderFEC: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for H223AL3MParameters (SEQUENCE)  */
/* <=============================================> */
void Delete_H223AL3MParameters(PS_H223AL3MParameters x)
{
    uint32 extension;

    extension = x->option_of_rsCodeCorrection;
    Delete_HeaderFormat(&x->headerFormat);
    Delete_Al3CrcLength(&x->al3CrcLength);
    Delete_Al3ArqType(&x->al3ArqType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_rsCodeCorrection)
    {
    }
}

/* <===================================> */
/*  PER-Deleter for Al3ArqType (CHOICE)  */
/* <===================================> */
void Delete_Al3ArqType(PS_Al3ArqType x)
{
    switch (x->index)
    {
        case 0:
            /* (noArq is NULL) */
            break;
        case 1:
            Delete_H223AnnexCArqParameters(x->typeIArq);
            OSCL_DEFAULT_FREE(x->typeIArq);
            break;
        case 2:
            Delete_H223AnnexCArqParameters(x->typeIIArq);
            OSCL_DEFAULT_FREE(x->typeIIArq);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_Al3ArqType: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for Al3CrcLength (CHOICE)  */
/* <=====================================> */
void Delete_Al3CrcLength(PS_Al3CrcLength x)
{
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
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        default:
            ErrorMessage("Delete_Al3CrcLength: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for HeaderFormat (CHOICE)  */
/* <=====================================> */
void Delete_HeaderFormat(PS_HeaderFormat x)
{
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
            ErrorMessage("Delete_HeaderFormat: Illegal CHOICE index");
    }
}

/* <==================================================> */
/*  PER-Deleter for H223AnnexCArqParameters (SEQUENCE)  */
/* <==================================================> */
void Delete_H223AnnexCArqParameters(PS_H223AnnexCArqParameters x)
{
    Delete_NumberOfRetransmissions(&x->numberOfRetransmissions);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for NumberOfRetransmissions (CHOICE)  */
/* <================================================> */
void Delete_NumberOfRetransmissions(PS_NumberOfRetransmissions x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            /* (infinite is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_NumberOfRetransmissions: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Deleter for V76LogicalChannelParameters (SEQUENCE)  */
/* <======================================================> */
void Delete_V76LogicalChannelParameters(PS_V76LogicalChannelParameters x)
{
    Delete_V76HDLCParameters(&x->hdlcParameters);
    Delete_SuspendResume(&x->suspendResume);
    Delete_Mode(&x->mode);
    Delete_V75Parameters(&x->v75Parameters);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================> */
/*  PER-Deleter for Mode (CHOICE)  */
/* <=============================> */
void Delete_Mode(PS_Mode x)
{
    switch (x->index)
    {
        case 0:
            Delete_ERM(x->eRM);
            OSCL_DEFAULT_FREE(x->eRM);
            break;
        case 1:
            /* (uNERM is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_Mode: Illegal CHOICE index");
    }
}

/* <==============================> */
/*  PER-Deleter for ERM (SEQUENCE)  */
/* <==============================> */
void Delete_ERM(PS_ERM x)
{
    Delete_Recovery(&x->recovery);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for Recovery (CHOICE)  */
/* <=================================> */
void Delete_Recovery(PS_Recovery x)
{
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
            ErrorMessage("Delete_Recovery: Illegal CHOICE index");
    }
}

/* <======================================> */
/*  PER-Deleter for SuspendResume (CHOICE)  */
/* <======================================> */
void Delete_SuspendResume(PS_SuspendResume x)
{
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
            ErrorMessage("Delete_SuspendResume: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Deleter for V76HDLCParameters (SEQUENCE)  */
/* <============================================> */
void Delete_V76HDLCParameters(PS_V76HDLCParameters x)
{
    Delete_CRCLength(&x->crcLength);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for CRCLength (CHOICE)  */
/* <==================================> */
void Delete_CRCLength(PS_CRCLength x)
{
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
            ErrorMessage("Delete_CRCLength: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Deleter for H2250LogicalChannelParameters (SEQUENCE)  */
/* <========================================================> */
void Delete_H2250LogicalChannelParameters(PS_H2250LogicalChannelParameters x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_transportCapability |
                x->option_of_redundancyEncoding |
                x->option_of_source |
                x->option_of_symmetricMediaAddress;
    if (x->option_of_nonStandard)
    {
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Delete_NonStandardParameter(x->nonStandard + i);
        }
        OSCL_DEFAULT_FREE(x->nonStandard);
    }
    if (x->option_of_associatedSessionID)
    {
    }
    if (x->option_of_mediaChannel)
    {
        Delete_TransportAddress(&x->mediaChannel);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
    }
    if (x->option_of_mediaControlChannel)
    {
        Delete_TransportAddress(&x->mediaControlChannel);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
    }
    if (x->option_of_silenceSuppression)
    {
    }
    if (x->option_of_destination)
    {
        Delete_TerminalLabel(&x->destination);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
    }
    if (x->option_of_mediaPacketization)
    {
        Delete_MediaPacketization(&x->mediaPacketization);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_transportCapability)
    {
        Delete_TransportCapability(&x->transportCapability);
    }
    if (x->option_of_redundancyEncoding)
    {
        Delete_RedundancyEncoding(&x->redundancyEncoding);
    }
    if (x->option_of_source)
    {
        Delete_TerminalLabel(&x->source);
    }
    if (x->option_of_symmetricMediaAddress)
    {
    }
}

/* <===========================================> */
/*  PER-Deleter for MediaPacketization (CHOICE)  */
/* <===========================================> */
void Delete_MediaPacketization(PS_MediaPacketization x)
{
    switch (x->index)
    {
        case 0:
            /* (h261aVideoPacketization is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            Delete_RTPPayloadType(x->rtpPayloadType);
            OSCL_DEFAULT_FREE(x->rtpPayloadType);
            break;
        default:
            ErrorMessage("Delete_MediaPacketization: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for RTPPayloadType (SEQUENCE)  */
/* <=========================================> */
void Delete_RTPPayloadType(PS_RTPPayloadType x)
{
    Delete_PayloadDescriptor(&x->payloadDescriptor);
    if (x->option_of_payloadType)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for PayloadDescriptor (CHOICE)  */
/* <==========================================> */
void Delete_PayloadDescriptor(PS_PayloadDescriptor x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandardIdentifier);
            OSCL_DEFAULT_FREE(x->nonStandardIdentifier);
            break;
        case 1:
            break;
        case 2:
            FreeObjectID(x->oid);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_PayloadDescriptor: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for RedundancyEncoding (SEQUENCE)  */
/* <=============================================> */
void Delete_RedundancyEncoding(PS_RedundancyEncoding x)
{
    Delete_RedundancyEncodingMethod(&x->redundancyEncodingMethod);
    if (x->option_of_secondaryEncoding)
    {
        Delete_DataType(&x->secondaryEncoding);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for TransportAddress (CHOICE)  */
/* <=========================================> */
void Delete_TransportAddress(PS_TransportAddress x)
{
    switch (x->index)
    {
        case 0:
            Delete_UnicastAddress(x->unicastAddress);
            OSCL_DEFAULT_FREE(x->unicastAddress);
            break;
        case 1:
            Delete_MulticastAddress(x->multicastAddress);
            OSCL_DEFAULT_FREE(x->multicastAddress);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_TransportAddress: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for UnicastAddress (CHOICE)  */
/* <=======================================> */
void Delete_UnicastAddress(PS_UnicastAddress x)
{
    switch (x->index)
    {
        case 0:
            Delete_IPAddress(x->iPAddress);
            OSCL_DEFAULT_FREE(x->iPAddress);
            break;
        case 1:
            Delete_IPXAddress(x->iPXAddress);
            OSCL_DEFAULT_FREE(x->iPXAddress);
            break;
        case 2:
            Delete_IP6Address(x->iP6Address);
            OSCL_DEFAULT_FREE(x->iP6Address);
            break;
        case 3:
            FreeOctetString(x->netBios);
            break;
        case 4:
            Delete_IPSourceRouteAddress(x->iPSourceRouteAddress);
            OSCL_DEFAULT_FREE(x->iPSourceRouteAddress);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            FreeOctetString(x->nsap);
            break;
        case 6:
            Delete_NonStandardParameter(x->nonStandardAddress);
            OSCL_DEFAULT_FREE(x->nonStandardAddress);
            break;
        default:
            ErrorMessage("Delete_UnicastAddress: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Deleter for IPSourceRouteAddress (SEQUENCE)  */
/* <===============================================> */
void Delete_IPSourceRouteAddress(PS_IPSourceRouteAddress x)
{
    Delete_Routing(&x->routing);
    OSCL_DEFAULT_FREE(x->route);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Deleter for Routing (CHOICE)  */
/* <================================> */
void Delete_Routing(PS_Routing x)
{
    switch (x->index)
    {
        case 0:
            /* (strict is NULL) */
            break;
        case 1:
            /* (loose is NULL) */
            break;
        default:
            ErrorMessage("Delete_Routing: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for IP6Address (SEQUENCE)  */
/* <=====================================> */
void Delete_IP6Address(PS_IP6Address x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for IPXAddress (SEQUENCE)  */
/* <=====================================> */
void Delete_IPXAddress(PS_IPXAddress x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Deleter for IPAddress (SEQUENCE)  */
/* <====================================> */
void Delete_IPAddress(PS_IPAddress x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for MulticastAddress (CHOICE)  */
/* <=========================================> */
void Delete_MulticastAddress(PS_MulticastAddress x)
{
    switch (x->index)
    {
        case 0:
            Delete_MaIpAddress(x->maIpAddress);
            OSCL_DEFAULT_FREE(x->maIpAddress);
            break;
        case 1:
            Delete_MaIp6Address(x->maIp6Address);
            OSCL_DEFAULT_FREE(x->maIp6Address);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            FreeOctetString(x->nsap);
            break;
        case 3:
            Delete_NonStandardParameter(x->nonStandardAddress);
            OSCL_DEFAULT_FREE(x->nonStandardAddress);
            break;
        default:
            ErrorMessage("Delete_MulticastAddress: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for MaIp6Address (SEQUENCE)  */
/* <=======================================> */
void Delete_MaIp6Address(PS_MaIp6Address x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for MaIpAddress (SEQUENCE)  */
/* <======================================> */
void Delete_MaIpAddress(PS_MaIpAddress x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for EncryptionSync (SEQUENCE)  */
/* <=========================================> */
void Delete_EncryptionSync(PS_EncryptionSync x)
{
    uint16 i = 0;
    if (x->option_of_nonStandard)
    {
        Delete_NonStandardParameter(&x->nonStandard);
    }
    if (x->option_of_escrowentry)
    {
        for (i = 0;i < x->size_of_escrowentry;++i)
        {
            Delete_EscrowData(x->escrowentry + i);
        }
        OSCL_DEFAULT_FREE(x->escrowentry);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for EscrowData (SEQUENCE)  */
/* <=====================================> */
void Delete_EscrowData(PS_EscrowData x)
{
    OSCL_DEFAULT_FREE(x->escrowID.data);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for OpenLogicalChannelAck (SEQUENCE)  */
/* <================================================> */
void Delete_OpenLogicalChannelAck(PS_OpenLogicalChannelAck x)
{
    uint32 extension;

    extension = x->option_of_separateStack |
                x->option_of_forwardMultiplexAckParameters |
                x->option_of_encryptionSync;
    if (x->option_of_ackReverseLogicalChannelParameters)
    {
        Delete_AckReverseLogicalChannelParameters(&x->ackReverseLogicalChannelParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_separateStack)
    {
        Delete_NetworkAccessParameters(&x->separateStack);
    }
    if (x->option_of_forwardMultiplexAckParameters)
    {
        Delete_ForwardMultiplexAckParameters(&x->forwardMultiplexAckParameters);
    }
    if (x->option_of_encryptionSync)
    {
        Delete_EncryptionSync(&x->encryptionSync);
    }
}

/* <======================================================> */
/*  PER-Deleter for ForwardMultiplexAckParameters (CHOICE)  */
/* <======================================================> */
void Delete_ForwardMultiplexAckParameters(PS_ForwardMultiplexAckParameters x)
{
    switch (x->index)
    {
        case 0:
            Delete_H2250LogicalChannelAckParameters(x->h2250LogicalChannelAckParameters);
            OSCL_DEFAULT_FREE(x->h2250LogicalChannelAckParameters);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_ForwardMultiplexAckParameters: Illegal CHOICE index");
    }
}

/* <=============================================================> */
/*  PER-Deleter for AckReverseLogicalChannelParameters (SEQUENCE)  */
/* <=============================================================> */
void Delete_AckReverseLogicalChannelParameters(PS_AckReverseLogicalChannelParameters x)
{
    uint32 extension;

    extension = x->option_of_replacementFor;
    if (x->option_of_portNumber)
    {
    }
    if (x->option_of_ackMultiplexParameters)
    {
        Delete_AckMultiplexParameters(&x->ackMultiplexParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_replacementFor)
    {
    }
}

/* <===============================================> */
/*  PER-Deleter for AckMultiplexParameters (CHOICE)  */
/* <===============================================> */
void Delete_AckMultiplexParameters(PS_AckMultiplexParameters x)
{
    switch (x->index)
    {
        case 0:
            Delete_H222LogicalChannelParameters(x->h222LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h222LogicalChannelParameters);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 1:
            Delete_H2250LogicalChannelParameters(x->h2250LogicalChannelParameters);
            OSCL_DEFAULT_FREE(x->h2250LogicalChannelParameters);
            break;
        default:
            ErrorMessage("Delete_AckMultiplexParameters: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Deleter for OpenLogicalChannelReject (SEQUENCE)  */
/* <===================================================> */
void Delete_OpenLogicalChannelReject(PS_OpenLogicalChannelReject x)
{
    Delete_OlcRejectCause(&x->olcRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for OlcRejectCause (CHOICE)  */
/* <=======================================> */
void Delete_OlcRejectCause(PS_OlcRejectCause x)
{
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
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            break;
        default:
            ErrorMessage("Delete_OlcRejectCause: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Deleter for OpenLogicalChannelConfirm (SEQUENCE)  */
/* <====================================================> */
void Delete_OpenLogicalChannelConfirm(PS_OpenLogicalChannelConfirm x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================================> */
/*  PER-Deleter for H2250LogicalChannelAckParameters (SEQUENCE)  */
/* <===========================================================> */
void Delete_H2250LogicalChannelAckParameters(PS_H2250LogicalChannelAckParameters x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_flowControlToZero |
                x->option_of_portNumber;
    if (x->option_of_nonStandard)
    {
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Delete_NonStandardParameter(x->nonStandard + i);
        }
        OSCL_DEFAULT_FREE(x->nonStandard);
    }
    if (x->option_of_sessionID)
    {
    }
    if (x->option_of_mediaChannel)
    {
        Delete_TransportAddress(&x->mediaChannel);
    }
    if (x->option_of_mediaControlChannel)
    {
        Delete_TransportAddress(&x->mediaControlChannel);
    }
    if (x->option_of_dynamicRTPPayloadType)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_flowControlToZero)
    {
    }
    if (x->option_of_portNumber)
    {
    }
}

/* <==============================================> */
/*  PER-Deleter for CloseLogicalChannel (SEQUENCE)  */
/* <==============================================> */
void Delete_CloseLogicalChannel(PS_CloseLogicalChannel x)
{
    uint32 extension;

    extension = x->option_of_reason;
    Delete_Source(&x->source);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_reason)
    {
        Delete_Reason(&x->reason);
    }
}

/* <===============================> */
/*  PER-Deleter for Reason (CHOICE)  */
/* <===============================> */
void Delete_Reason(PS_Reason x)
{
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
            ErrorMessage("Delete_Reason: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Deleter for Source (CHOICE)  */
/* <===============================> */
void Delete_Source(PS_Source x)
{
    switch (x->index)
    {
        case 0:
            /* (user is NULL) */
            break;
        case 1:
            /* (lcse is NULL) */
            break;
        default:
            ErrorMessage("Delete_Source: Illegal CHOICE index");
    }
}

/* <=================================================> */
/*  PER-Deleter for CloseLogicalChannelAck (SEQUENCE)  */
/* <=================================================> */
void Delete_CloseLogicalChannelAck(PS_CloseLogicalChannelAck x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for RequestChannelClose (SEQUENCE)  */
/* <==============================================> */
void Delete_RequestChannelClose(PS_RequestChannelClose x)
{
    uint32 extension;

    extension = x->option_of_qosCapability |
                x->option_of_rccReason;
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_qosCapability)
    {
        Delete_QOSCapability(&x->qosCapability);
    }
    if (x->option_of_rccReason)
    {
        Delete_RccReason(&x->rccReason);
    }
}

/* <==================================> */
/*  PER-Deleter for RccReason (CHOICE)  */
/* <==================================> */
void Delete_RccReason(PS_RccReason x)
{
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
            ErrorMessage("Delete_RccReason: Illegal CHOICE index");
    }
}

/* <=================================================> */
/*  PER-Deleter for RequestChannelCloseAck (SEQUENCE)  */
/* <=================================================> */
void Delete_RequestChannelCloseAck(PS_RequestChannelCloseAck x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Deleter for RequestChannelCloseReject (SEQUENCE)  */
/* <====================================================> */
void Delete_RequestChannelCloseReject(PS_RequestChannelCloseReject x)
{
    Delete_RccRejectCause(&x->rccRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for RccRejectCause (CHOICE)  */
/* <=======================================> */
void Delete_RccRejectCause(PS_RccRejectCause x)
{
    switch (x->index)
    {
        case 0:
            /* (unspecified is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_RccRejectCause: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Deleter for RequestChannelCloseRelease (SEQUENCE)  */
/* <=====================================================> */
void Delete_RequestChannelCloseRelease(PS_RequestChannelCloseRelease x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for MultiplexEntrySend (SEQUENCE)  */
/* <=============================================> */
void Delete_MultiplexEntrySend(PS_MultiplexEntrySend x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_multiplexEntryDescriptors;++i)
    {
        Delete_MultiplexEntryDescriptor(x->multiplexEntryDescriptors + i);
    }
    OSCL_DEFAULT_FREE(x->multiplexEntryDescriptors);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for MultiplexEntryDescriptor (SEQUENCE)  */
/* <===================================================> */
OSCL_EXPORT_REF void Delete_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x)
{
    uint16 i = 0;
    if (x->option_of_elementList)
    {
        for (i = 0;i < x->size_of_elementList;++i)
        {
            Delete_MultiplexElement(x->elementList + i);
        }
        OSCL_DEFAULT_FREE(x->elementList);
    }
}

/* <===========================================> */
/*  PER-Deleter for MultiplexElement (SEQUENCE)  */
/* <===========================================> */
void Delete_MultiplexElement(PS_MultiplexElement x)
{
    Delete_MuxType(&x->muxType);
    Delete_RepeatCount(&x->repeatCount);
}

/* <====================================> */
/*  PER-Deleter for RepeatCount (CHOICE)  */
/* <====================================> */
void Delete_RepeatCount(PS_RepeatCount x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            /* (untilClosingFlag is NULL) */
            break;
        default:
            ErrorMessage("Delete_RepeatCount: Illegal CHOICE index");
    }
}

/* <================================> */
/*  PER-Deleter for MuxType (CHOICE)  */
/* <================================> */
void Delete_MuxType(PS_MuxType x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            for (i = 0;i < x->size;++i)
            {
                Delete_MultiplexElement(x->subElementList + i);
            }
            OSCL_DEFAULT_FREE(x->subElementList);
            break;
        default:
            ErrorMessage("Delete_MuxType: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for MultiplexEntrySendAck (SEQUENCE)  */
/* <================================================> */
void Delete_MultiplexEntrySendAck(PS_MultiplexEntrySendAck x)
{
    OSCL_DEFAULT_FREE(x->multiplexTableEntryNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for MultiplexEntrySendReject (SEQUENCE)  */
/* <===================================================> */
void Delete_MultiplexEntrySendReject(PS_MultiplexEntrySendReject x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Delete_MultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i);
    }
    OSCL_DEFAULT_FREE(x->rejectionDescriptions);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================================> */
/*  PER-Deleter for MultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <==============================================================> */
void Delete_MultiplexEntryRejectionDescriptions(PS_MultiplexEntryRejectionDescriptions x)
{
    Delete_MeRejectCause(&x->meRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for MeRejectCause (CHOICE)  */
/* <======================================> */
void Delete_MeRejectCause(PS_MeRejectCause x)
{
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
            ErrorMessage("Delete_MeRejectCause: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Deleter for MultiplexEntrySendRelease (SEQUENCE)  */
/* <====================================================> */
void Delete_MultiplexEntrySendRelease(PS_MultiplexEntrySendRelease x)
{
    OSCL_DEFAULT_FREE(x->multiplexTableEntryNumber);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for RequestMultiplexEntry (SEQUENCE)  */
/* <================================================> */
void Delete_RequestMultiplexEntry(PS_RequestMultiplexEntry x)
{
    OSCL_DEFAULT_FREE(x->entryNumbers);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for RequestMultiplexEntryAck (SEQUENCE)  */
/* <===================================================> */
void Delete_RequestMultiplexEntryAck(PS_RequestMultiplexEntryAck x)
{
    OSCL_DEFAULT_FREE(x->entryNumbers);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for RequestMultiplexEntryReject (SEQUENCE)  */
/* <======================================================> */
void Delete_RequestMultiplexEntryReject(PS_RequestMultiplexEntryReject x)
{
    uint16 i = 0;
    OSCL_DEFAULT_FREE(x->entryNumbers);
    for (i = 0;i < x->size_of_rejectionDescriptions;++i)
    {
        Delete_RequestMultiplexEntryRejectionDescriptions(x->rejectionDescriptions + i);
    }
    OSCL_DEFAULT_FREE(x->rejectionDescriptions);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================================> */
/*  PER-Deleter for RequestMultiplexEntryRejectionDescriptions (SEQUENCE)  */
/* <=====================================================================> */
void Delete_RequestMultiplexEntryRejectionDescriptions(PS_RequestMultiplexEntryRejectionDescriptions x)
{
    Delete_RmeRejectCause(&x->rmeRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for RmeRejectCause (CHOICE)  */
/* <=======================================> */
void Delete_RmeRejectCause(PS_RmeRejectCause x)
{
    switch (x->index)
    {
        case 0:
            /* (unspecifiedCause is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_RmeRejectCause: Illegal CHOICE index");
    }
}

/* <=======================================================> */
/*  PER-Deleter for RequestMultiplexEntryRelease (SEQUENCE)  */
/* <=======================================================> */
void Delete_RequestMultiplexEntryRelease(PS_RequestMultiplexEntryRelease x)
{
    OSCL_DEFAULT_FREE(x->entryNumbers);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for RequestMode (SEQUENCE)  */
/* <======================================> */
void Delete_RequestMode(PS_RequestMode x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_requestedModes;++i)
    {
        Delete_ModeDescription(x->requestedModes + i);
    }
    OSCL_DEFAULT_FREE(x->requestedModes);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for RequestModeAck (SEQUENCE)  */
/* <=========================================> */
void Delete_RequestModeAck(PS_RequestModeAck x)
{
    Delete_Response(&x->response);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for Response (CHOICE)  */
/* <=================================> */
void Delete_Response(PS_Response x)
{
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
            ErrorMessage("Delete_Response: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Deleter for RequestModeReject (SEQUENCE)  */
/* <============================================> */
void Delete_RequestModeReject(PS_RequestModeReject x)
{
    Delete_RmRejectCause(&x->rmRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for RmRejectCause (CHOICE)  */
/* <======================================> */
void Delete_RmRejectCause(PS_RmRejectCause x)
{
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
            ErrorMessage("Delete_RmRejectCause: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for RequestModeRelease (SEQUENCE)  */
/* <=============================================> */
void Delete_RequestModeRelease(PS_RequestModeRelease x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for ModeDescription (SET-OF)  */
/* <========================================> */
void Delete_ModeDescription(PS_ModeDescription x)
{
    uint16 i;
    for (i = 0;i < x->size;++i)
    {
        Delete_ModeElement(x->item + i);
    }
    OSCL_DEFAULT_FREE(x->item);
}

/* <======================================> */
/*  PER-Deleter for ModeElement (SEQUENCE)  */
/* <======================================> */
void Delete_ModeElement(PS_ModeElement x)
{
    uint32 extension;

    extension = x->option_of_v76ModeParameters |
                x->option_of_h2250ModeParameters |
                x->option_of_genericModeParameters;
    Delete_ModeType(&x->modeType);
    if (x->option_of_h223ModeParameters)
    {
        Delete_H223ModeParameters(&x->h223ModeParameters);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_v76ModeParameters)
    {
        Delete_V76ModeParameters(&x->v76ModeParameters);
    }
    if (x->option_of_h2250ModeParameters)
    {
        Delete_H2250ModeParameters(&x->h2250ModeParameters);
    }
    if (x->option_of_genericModeParameters)
    {
        Delete_GenericCapability(&x->genericModeParameters);
    }
}

/* <=================================> */
/*  PER-Deleter for ModeType (CHOICE)  */
/* <=================================> */
void Delete_ModeType(PS_ModeType x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_VideoMode(x->videoMode);
            OSCL_DEFAULT_FREE(x->videoMode);
            break;
        case 2:
            Delete_AudioMode(x->audioMode);
            OSCL_DEFAULT_FREE(x->audioMode);
            break;
        case 3:
            Delete_DataMode(x->dataMode);
            OSCL_DEFAULT_FREE(x->dataMode);
            break;
        case 4:
            Delete_EncryptionMode(x->encryptionMode);
            OSCL_DEFAULT_FREE(x->encryptionMode);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Delete_H235Mode(x->h235Mode);
            OSCL_DEFAULT_FREE(x->h235Mode);
            break;
        default:
            ErrorMessage("Delete_ModeType: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Deleter for H235Mode (SEQUENCE)  */
/* <===================================> */
void Delete_H235Mode(PS_H235Mode x)
{
    Delete_EncryptionAuthenticationAndIntegrity(&x->encryptionAuthenticationAndIntegrity);
    Delete_MediaMode(&x->mediaMode);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for MediaMode (CHOICE)  */
/* <==================================> */
void Delete_MediaMode(PS_MediaMode x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_VideoMode(x->videoMode);
            OSCL_DEFAULT_FREE(x->videoMode);
            break;
        case 2:
            Delete_AudioMode(x->audioMode);
            OSCL_DEFAULT_FREE(x->audioMode);
            break;
        case 3:
            Delete_DataMode(x->dataMode);
            OSCL_DEFAULT_FREE(x->dataMode);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MediaMode: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for H223ModeParameters (SEQUENCE)  */
/* <=============================================> */
void Delete_H223ModeParameters(PS_H223ModeParameters x)
{
    Delete_ModeAdaptationLayerType(&x->modeAdaptationLayerType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for ModeAdaptationLayerType (CHOICE)  */
/* <================================================> */
void Delete_ModeAdaptationLayerType(PS_ModeAdaptationLayerType x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            Delete_ModeAl3(x->modeAl3);
            OSCL_DEFAULT_FREE(x->modeAl3);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 6:
            Delete_H223AL1MParameters(x->al1M);
            OSCL_DEFAULT_FREE(x->al1M);
            break;
        case 7:
            Delete_H223AL2MParameters(x->al2M);
            OSCL_DEFAULT_FREE(x->al2M);
            break;
        case 8:
            Delete_H223AL3MParameters(x->al3M);
            OSCL_DEFAULT_FREE(x->al3M);
            break;
        default:
            ErrorMessage("Delete_ModeAdaptationLayerType: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for ModeAl3 (SEQUENCE)  */
/* <==================================> */
void Delete_ModeAl3(PS_ModeAl3 x)
{
    OSCL_UNUSED_ARG(x);
}

/* <==========================================> */
/*  PER-Deleter for V76ModeParameters (CHOICE)  */
/* <==========================================> */
void Delete_V76ModeParameters(PS_V76ModeParameters x)
{
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
            ErrorMessage("Delete_V76ModeParameters: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Deleter for H2250ModeParameters (SEQUENCE)  */
/* <==============================================> */
void Delete_H2250ModeParameters(PS_H2250ModeParameters x)
{
    if (x->option_of_redundancyEncodingMode)
    {
        Delete_RedundancyEncodingMode(&x->redundancyEncodingMode);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for RedundancyEncodingMode (SEQUENCE)  */
/* <=================================================> */
void Delete_RedundancyEncodingMode(PS_RedundancyEncodingMode x)
{
    Delete_RedundancyEncodingMethod(&x->redundancyEncodingMethod);
    if (x->option_of_secondaryEncoding)
    {
        Delete_SecondaryEncoding(&x->secondaryEncoding);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for SecondaryEncoding (CHOICE)  */
/* <==========================================> */
void Delete_SecondaryEncoding(PS_SecondaryEncoding x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_AudioMode(x->audioData);
            OSCL_DEFAULT_FREE(x->audioData);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_SecondaryEncoding: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for VideoMode (CHOICE)  */
/* <==================================> */
void Delete_VideoMode(PS_VideoMode x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_H261VideoMode(x->h261VideoMode);
            OSCL_DEFAULT_FREE(x->h261VideoMode);
            break;
        case 2:
            Delete_H262VideoMode(x->h262VideoMode);
            OSCL_DEFAULT_FREE(x->h262VideoMode);
            break;
        case 3:
            Delete_H263VideoMode(x->h263VideoMode);
            OSCL_DEFAULT_FREE(x->h263VideoMode);
            break;
        case 4:
            Delete_IS11172VideoMode(x->is11172VideoMode);
            OSCL_DEFAULT_FREE(x->is11172VideoMode);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 5:
            Delete_GenericCapability(x->genericVideoMode);
            OSCL_DEFAULT_FREE(x->genericVideoMode);
            break;
        default:
            ErrorMessage("Delete_VideoMode: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for H261VideoMode (SEQUENCE)  */
/* <========================================> */
void Delete_H261VideoMode(PS_H261VideoMode x)
{
    Delete_Resolution(&x->resolution);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Deleter for Resolution (CHOICE)  */
/* <===================================> */
void Delete_Resolution(PS_Resolution x)
{
    switch (x->index)
    {
        case 0:
            /* (qcif is NULL) */
            break;
        case 1:
            /* (cif is NULL) */
            break;
        default:
            ErrorMessage("Delete_Resolution: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for H262VideoMode (SEQUENCE)  */
/* <========================================> */
void Delete_H262VideoMode(PS_H262VideoMode x)
{
    Delete_ProfileAndLevel(&x->profileAndLevel);
    if (x->option_of_videoBitRate)
    {
    }
    if (x->option_of_vbvBufferSize)
    {
    }
    if (x->option_of_samplesPerLine)
    {
    }
    if (x->option_of_linesPerFrame)
    {
    }
    if (x->option_of_framesPerSecond)
    {
    }
    if (x->option_of_luminanceSampleRate)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for ProfileAndLevel (CHOICE)  */
/* <========================================> */
void Delete_ProfileAndLevel(PS_ProfileAndLevel x)
{
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
            ErrorMessage("Delete_ProfileAndLevel: Illegal CHOICE index");
    }
}

/* <========================================> */
/*  PER-Deleter for H263VideoMode (SEQUENCE)  */
/* <========================================> */
void Delete_H263VideoMode(PS_H263VideoMode x)
{
    uint32 extension;

    extension = x->option_of_errorCompensation |
                x->option_of_enhancementLayerInfo |
                x->option_of_h263Options;
    Delete_H263Resolution(&x->h263Resolution);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_errorCompensation)
    {
    }
    if (x->option_of_enhancementLayerInfo)
    {
        Delete_EnhancementLayerInfo(&x->enhancementLayerInfo);
    }
    if (x->option_of_h263Options)
    {
        Delete_H263Options(&x->h263Options);
    }
}

/* <=======================================> */
/*  PER-Deleter for H263Resolution (CHOICE)  */
/* <=======================================> */
void Delete_H263Resolution(PS_H263Resolution x)
{
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
            ErrorMessage("Delete_H263Resolution: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for IS11172VideoMode (SEQUENCE)  */
/* <===========================================> */
void Delete_IS11172VideoMode(PS_IS11172VideoMode x)
{
    if (x->option_of_videoBitRate)
    {
    }
    if (x->option_of_vbvBufferSize)
    {
    }
    if (x->option_of_samplesPerLine)
    {
    }
    if (x->option_of_linesPerFrame)
    {
    }
    if (x->option_of_pictureRate)
    {
    }
    if (x->option_of_luminanceSampleRate)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for AudioMode (CHOICE)  */
/* <==================================> */
void Delete_AudioMode(PS_AudioMode x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            Delete_ModeG7231(x->modeG7231);
            OSCL_DEFAULT_FREE(x->modeG7231);
            break;
        case 12:
            Delete_IS11172AudioMode(x->is11172AudioMode);
            OSCL_DEFAULT_FREE(x->is11172AudioMode);
            break;
        case 13:
            Delete_IS13818AudioMode(x->is13818AudioMode);
            OSCL_DEFAULT_FREE(x->is13818AudioMode);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            break;
        case 15:
            break;
        case 16:
            Delete_G7231AnnexCMode(x->g7231AnnexCMode);
            OSCL_DEFAULT_FREE(x->g7231AnnexCMode);
            break;
        case 17:
            Delete_GSMAudioCapability(x->gsmFullRate);
            OSCL_DEFAULT_FREE(x->gsmFullRate);
            break;
        case 18:
            Delete_GSMAudioCapability(x->gsmHalfRate);
            OSCL_DEFAULT_FREE(x->gsmHalfRate);
            break;
        case 19:
            Delete_GSMAudioCapability(x->gsmEnhancedFullRate);
            OSCL_DEFAULT_FREE(x->gsmEnhancedFullRate);
            break;
        case 20:
            Delete_GenericCapability(x->genericAudioMode);
            OSCL_DEFAULT_FREE(x->genericAudioMode);
            break;
        case 21:
            Delete_G729Extensions(x->g729Extensions);
            OSCL_DEFAULT_FREE(x->g729Extensions);
            break;
        default:
            ErrorMessage("Delete_AudioMode: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for ModeG7231 (CHOICE)  */
/* <==================================> */
void Delete_ModeG7231(PS_ModeG7231 x)
{
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
            ErrorMessage("Delete_ModeG7231: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for IS11172AudioMode (SEQUENCE)  */
/* <===========================================> */
void Delete_IS11172AudioMode(PS_IS11172AudioMode x)
{
    Delete_AudioLayer(&x->audioLayer);
    Delete_AudioSampling(&x->audioSampling);
    Delete_MultichannelType(&x->multichannelType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=========================================> */
/*  PER-Deleter for MultichannelType (CHOICE)  */
/* <=========================================> */
void Delete_MultichannelType(PS_MultichannelType x)
{
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
            ErrorMessage("Delete_MultichannelType: Illegal CHOICE index");
    }
}

/* <======================================> */
/*  PER-Deleter for AudioSampling (CHOICE)  */
/* <======================================> */
void Delete_AudioSampling(PS_AudioSampling x)
{
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
            ErrorMessage("Delete_AudioSampling: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Deleter for AudioLayer (CHOICE)  */
/* <===================================> */
void Delete_AudioLayer(PS_AudioLayer x)
{
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
            ErrorMessage("Delete_AudioLayer: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for IS13818AudioMode (SEQUENCE)  */
/* <===========================================> */
void Delete_IS13818AudioMode(PS_IS13818AudioMode x)
{
    Delete_Is13818AudioLayer(&x->is13818AudioLayer);
    Delete_Is13818AudioSampling(&x->is13818AudioSampling);
    Delete_Is13818MultichannelType(&x->is13818MultichannelType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================================> */
/*  PER-Deleter for Is13818MultichannelType (CHOICE)  */
/* <================================================> */
void Delete_Is13818MultichannelType(PS_Is13818MultichannelType x)
{
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
            ErrorMessage("Delete_Is13818MultichannelType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for Is13818AudioSampling (CHOICE)  */
/* <=============================================> */
void Delete_Is13818AudioSampling(PS_Is13818AudioSampling x)
{
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
            ErrorMessage("Delete_Is13818AudioSampling: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for Is13818AudioLayer (CHOICE)  */
/* <==========================================> */
void Delete_Is13818AudioLayer(PS_Is13818AudioLayer x)
{
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
            ErrorMessage("Delete_Is13818AudioLayer: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for G7231AnnexCMode (SEQUENCE)  */
/* <==========================================> */
void Delete_G7231AnnexCMode(PS_G7231AnnexCMode x)
{
    Delete_ModeG723AnnexCAudioMode(&x->modeG723AnnexCAudioMode);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for ModeG723AnnexCAudioMode (SEQUENCE)  */
/* <==================================================> */
void Delete_ModeG723AnnexCAudioMode(PS_ModeG723AnnexCAudioMode x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================> */
/*  PER-Deleter for DataMode (SEQUENCE)  */
/* <===================================> */
void Delete_DataMode(PS_DataMode x)
{
    Delete_DmApplication(&x->dmApplication);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for DmApplication (CHOICE)  */
/* <======================================> */
void Delete_DmApplication(PS_DmApplication x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_DataProtocolCapability(x->t120);
            OSCL_DEFAULT_FREE(x->t120);
            break;
        case 2:
            Delete_DataProtocolCapability(x->dsm_cc);
            OSCL_DEFAULT_FREE(x->dsm_cc);
            break;
        case 3:
            Delete_DataProtocolCapability(x->userData);
            OSCL_DEFAULT_FREE(x->userData);
            break;
        case 4:
            Delete_DataProtocolCapability(x->t84);
            OSCL_DEFAULT_FREE(x->t84);
            break;
        case 5:
            Delete_DataProtocolCapability(x->t434);
            OSCL_DEFAULT_FREE(x->t434);
            break;
        case 6:
            Delete_DataProtocolCapability(x->h224);
            OSCL_DEFAULT_FREE(x->h224);
            break;
        case 7:
            Delete_DmNlpid(x->dmNlpid);
            OSCL_DEFAULT_FREE(x->dmNlpid);
            break;
        case 8:
            /* (dsvdControl is NULL) */
            break;
        case 9:
            Delete_DataProtocolCapability(x->h222DataPartitioning);
            OSCL_DEFAULT_FREE(x->h222DataPartitioning);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Delete_DataProtocolCapability(x->t30fax);
            OSCL_DEFAULT_FREE(x->t30fax);
            break;
        case 11:
            Delete_DataProtocolCapability(x->t140);
            OSCL_DEFAULT_FREE(x->t140);
            break;
        case 12:
            Delete_DmT38fax(x->dmT38fax);
            OSCL_DEFAULT_FREE(x->dmT38fax);
            break;
        case 13:
            Delete_GenericCapability(x->genericDataMode);
            OSCL_DEFAULT_FREE(x->genericDataMode);
            break;
        default:
            ErrorMessage("Delete_DmApplication: Illegal CHOICE index");
    }
}

/* <===================================> */
/*  PER-Deleter for DmT38fax (SEQUENCE)  */
/* <===================================> */
void Delete_DmT38fax(PS_DmT38fax x)
{
    Delete_DataProtocolCapability(&x->t38FaxProtocol);
    Delete_T38FaxProfile(&x->t38FaxProfile);
}

/* <==================================> */
/*  PER-Deleter for DmNlpid (SEQUENCE)  */
/* <==================================> */
void Delete_DmNlpid(PS_DmNlpid x)
{
    if (x->nlpidData.size > 0) OSCL_DEFAULT_FREE(x->nlpidData.data);
    Delete_DataProtocolCapability(&x->nlpidProtocol);
}

/* <=======================================> */
/*  PER-Deleter for EncryptionMode (CHOICE)  */
/* <=======================================> */
void Delete_EncryptionMode(PS_EncryptionMode x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            /* (h233Encryption is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_EncryptionMode: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for RoundTripDelayRequest (SEQUENCE)  */
/* <================================================> */
void Delete_RoundTripDelayRequest(PS_RoundTripDelayRequest x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for RoundTripDelayResponse (SEQUENCE)  */
/* <=================================================> */
void Delete_RoundTripDelayResponse(PS_RoundTripDelayResponse x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================================> */
/*  PER-Deleter for MaintenanceLoopRequest (SEQUENCE)  */
/* <=================================================> */
void Delete_MaintenanceLoopRequest(PS_MaintenanceLoopRequest x)
{
    Delete_MlRequestType(&x->mlRequestType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for MlRequestType (CHOICE)  */
/* <======================================> */
void Delete_MlRequestType(PS_MlRequestType x)
{
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            break;
        case 2:
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MlRequestType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for MaintenanceLoopAck (SEQUENCE)  */
/* <=============================================> */
void Delete_MaintenanceLoopAck(PS_MaintenanceLoopAck x)
{
    Delete_MlAckType(&x->mlAckType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for MlAckType (CHOICE)  */
/* <==================================> */
void Delete_MlAckType(PS_MlAckType x)
{
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            break;
        case 2:
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MlAckType: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for MaintenanceLoopReject (SEQUENCE)  */
/* <================================================> */
void Delete_MaintenanceLoopReject(PS_MaintenanceLoopReject x)
{
    Delete_MlRejectType(&x->mlRejectType);
    Delete_MlRejectCause(&x->mlRejectCause);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================> */
/*  PER-Deleter for MlRejectCause (CHOICE)  */
/* <======================================> */
void Delete_MlRejectCause(PS_MlRejectCause x)
{
    switch (x->index)
    {
        case 0:
            /* (canNotPerformLoop is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MlRejectCause: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for MlRejectType (CHOICE)  */
/* <=====================================> */
void Delete_MlRejectType(PS_MlRejectType x)
{
    switch (x->index)
    {
        case 0:
            /* (systemLoop is NULL) */
            break;
        case 1:
            break;
        case 2:
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MlRejectType: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Deleter for MaintenanceLoopOffCommand (SEQUENCE)  */
/* <====================================================> */
void Delete_MaintenanceLoopOffCommand(PS_MaintenanceLoopOffCommand x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for CommunicationModeCommand (SEQUENCE)  */
/* <===================================================> */
void Delete_CommunicationModeCommand(PS_CommunicationModeCommand x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_communicationModeTable;++i)
    {
        Delete_CommunicationModeTableEntry(x->communicationModeTable + i);
    }
    OSCL_DEFAULT_FREE(x->communicationModeTable);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for CommunicationModeRequest (SEQUENCE)  */
/* <===================================================> */
void Delete_CommunicationModeRequest(PS_CommunicationModeRequest x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for CommunicationModeResponse (CHOICE)  */
/* <==================================================> */
void Delete_CommunicationModeResponse(PS_CommunicationModeResponse x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            for (i = 0;i < x->size;++i)
            {
                Delete_CommunicationModeTableEntry(x->communicationModeTable + i);
            }
            OSCL_DEFAULT_FREE(x->communicationModeTable);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_CommunicationModeResponse: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Deleter for CommunicationModeTableEntry (SEQUENCE)  */
/* <======================================================> */
void Delete_CommunicationModeTableEntry(PS_CommunicationModeTableEntry x)
{
    uint16 i = 0;
    uint32 extension;

    extension = x->option_of_redundancyEncoding |
                x->option_of_sessionDependency |
                x->option_of_destination;
    if (x->option_of_nonStandard)
    {
        for (i = 0;i < x->size_of_nonStandard;++i)
        {
            Delete_NonStandardParameter(x->nonStandard + i);
        }
        OSCL_DEFAULT_FREE(x->nonStandard);
    }
    if (x->option_of_associatedSessionID)
    {
    }
    if (x->option_of_terminalLabel)
    {
        Delete_TerminalLabel(&x->terminalLabel);
    }
    Delete_CmtDataType(&x->cmtDataType);
    if (x->option_of_mediaChannel)
    {
        Delete_TransportAddress(&x->mediaChannel);
    }
    if (x->option_of_mediaGuaranteedDelivery)
    {
    }
    if (x->option_of_mediaControlChannel)
    {
        Delete_TransportAddress(&x->mediaControlChannel);
    }
    if (x->option_of_mediaControlGuaranteedDelivery)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_redundancyEncoding)
    {
        Delete_RedundancyEncoding(&x->redundancyEncoding);
    }
    if (x->option_of_sessionDependency)
    {
    }
    if (x->option_of_destination)
    {
        Delete_TerminalLabel(&x->destination);
    }
}

/* <====================================> */
/*  PER-Deleter for CmtDataType (CHOICE)  */
/* <====================================> */
void Delete_CmtDataType(PS_CmtDataType x)
{
    switch (x->index)
    {
        case 0:
            Delete_VideoCapability(x->videoData);
            OSCL_DEFAULT_FREE(x->videoData);
            break;
        case 1:
            Delete_AudioCapability(x->audioData);
            OSCL_DEFAULT_FREE(x->audioData);
            break;
        case 2:
            Delete_DataApplicationCapability(x->data);
            OSCL_DEFAULT_FREE(x->data);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_CmtDataType: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for ConferenceRequest (CHOICE)  */
/* <==========================================> */
void Delete_ConferenceRequest(PS_ConferenceRequest x)
{
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
            Delete_TerminalLabel(x->dropTerminal);
            OSCL_DEFAULT_FREE(x->dropTerminal);
            break;
        case 4:
            Delete_TerminalLabel(x->requestTerminalID);
            OSCL_DEFAULT_FREE(x->requestTerminalID);
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
            break;
        case 9:
            break;
        case 10:
            Delete_RequestTerminalCertificate(x->requestTerminalCertificate);
            OSCL_DEFAULT_FREE(x->requestTerminalCertificate);
            break;
        case 11:
            break;
        case 12:
            Delete_TerminalLabel(x->makeTerminalBroadcaster);
            OSCL_DEFAULT_FREE(x->makeTerminalBroadcaster);
            break;
        case 13:
            Delete_TerminalLabel(x->sendThisSource);
            OSCL_DEFAULT_FREE(x->sendThisSource);
            break;
        case 14:
            break;
        case 15:
            Delete_RemoteMCRequest(x->remoteMCRequest);
            OSCL_DEFAULT_FREE(x->remoteMCRequest);
            break;
        default:
            ErrorMessage("Delete_ConferenceRequest: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Deleter for RequestTerminalCertificate (SEQUENCE)  */
/* <=====================================================> */
void Delete_RequestTerminalCertificate(PS_RequestTerminalCertificate x)
{
    if (x->option_of_terminalLabel)
    {
        Delete_TerminalLabel(&x->terminalLabel);
    }
    if (x->option_of_certSelectionCriteria)
    {
        Delete_CertSelectionCriteria(&x->certSelectionCriteria);
    }
    if (x->option_of_sRandom)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for CertSelectionCriteria (SEQUENCE-OF)  */
/* <===================================================> */
void Delete_CertSelectionCriteria(PS_CertSelectionCriteria x)
{
    uint16 i;
    for (i = 0;i < x->size;++i)
    {
        Delete_Criteria(x->item + i);
    }
    OSCL_DEFAULT_FREE(x->item);
}

/* <===================================> */
/*  PER-Deleter for Criteria (SEQUENCE)  */
/* <===================================> */
void Delete_Criteria(PS_Criteria x)
{
    OSCL_DEFAULT_FREE(x->field.data);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for TerminalLabel (SEQUENCE)  */
/* <========================================> */
void Delete_TerminalLabel(PS_TerminalLabel x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for ConferenceResponse (CHOICE)  */
/* <===========================================> */
void Delete_ConferenceResponse(PS_ConferenceResponse x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            Delete_MCTerminalIDResponse(x->mCTerminalIDResponse);
            OSCL_DEFAULT_FREE(x->mCTerminalIDResponse);
            break;
        case 1:
            Delete_TerminalIDResponse(x->terminalIDResponse);
            OSCL_DEFAULT_FREE(x->terminalIDResponse);
            break;
        case 2:
            Delete_ConferenceIDResponse(x->conferenceIDResponse);
            OSCL_DEFAULT_FREE(x->conferenceIDResponse);
            break;
        case 3:
            Delete_PasswordResponse(x->passwordResponse);
            OSCL_DEFAULT_FREE(x->passwordResponse);
            break;
        case 4:
            for (i = 0;i < x->size;++i)
            {
                Delete_TerminalLabel(x->terminalListResponse + i);
            }
            OSCL_DEFAULT_FREE(x->terminalListResponse);
            break;
        case 5:
            /* (videoCommandReject is NULL) */
            break;
        case 6:
            /* (terminalDropReject is NULL) */
            break;
        case 7:
            Delete_MakeMeChairResponse(x->makeMeChairResponse);
            OSCL_DEFAULT_FREE(x->makeMeChairResponse);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 8:
            Delete_ExtensionAddressResponse(x->extensionAddressResponse);
            OSCL_DEFAULT_FREE(x->extensionAddressResponse);
            break;
        case 9:
            Delete_ChairTokenOwnerResponse(x->chairTokenOwnerResponse);
            OSCL_DEFAULT_FREE(x->chairTokenOwnerResponse);
            break;
        case 10:
            Delete_TerminalCertificateResponse(x->terminalCertificateResponse);
            OSCL_DEFAULT_FREE(x->terminalCertificateResponse);
            break;
        case 11:
            Delete_BroadcastMyLogicalChannelResponse(x->broadcastMyLogicalChannelResponse);
            OSCL_DEFAULT_FREE(x->broadcastMyLogicalChannelResponse);
            break;
        case 12:
            Delete_MakeTerminalBroadcasterResponse(x->makeTerminalBroadcasterResponse);
            OSCL_DEFAULT_FREE(x->makeTerminalBroadcasterResponse);
            break;
        case 13:
            Delete_SendThisSourceResponse(x->sendThisSourceResponse);
            OSCL_DEFAULT_FREE(x->sendThisSourceResponse);
            break;
        case 14:
            Delete_RequestAllTerminalIDsResponse(x->requestAllTerminalIDsResponse);
            OSCL_DEFAULT_FREE(x->requestAllTerminalIDsResponse);
            break;
        case 15:
            Delete_RemoteMCResponse(x->remoteMCResponse);
            OSCL_DEFAULT_FREE(x->remoteMCResponse);
            break;
        default:
            ErrorMessage("Delete_ConferenceResponse: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Deleter for SendThisSourceResponse (CHOICE)  */
/* <===============================================> */
void Delete_SendThisSourceResponse(PS_SendThisSourceResponse x)
{
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
            ErrorMessage("Delete_SendThisSourceResponse: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Deleter for MakeTerminalBroadcasterResponse (CHOICE)  */
/* <========================================================> */
void Delete_MakeTerminalBroadcasterResponse(PS_MakeTerminalBroadcasterResponse x)
{
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
            ErrorMessage("Delete_MakeTerminalBroadcasterResponse: Illegal CHOICE index");
    }
}

/* <==========================================================> */
/*  PER-Deleter for BroadcastMyLogicalChannelResponse (CHOICE)  */
/* <==========================================================> */
void Delete_BroadcastMyLogicalChannelResponse(PS_BroadcastMyLogicalChannelResponse x)
{
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
            ErrorMessage("Delete_BroadcastMyLogicalChannelResponse: Illegal CHOICE index");
    }
}

/* <======================================================> */
/*  PER-Deleter for TerminalCertificateResponse (SEQUENCE)  */
/* <======================================================> */
void Delete_TerminalCertificateResponse(PS_TerminalCertificateResponse x)
{
    if (x->option_of_terminalLabel)
    {
        Delete_TerminalLabel(&x->terminalLabel);
    }
    if (x->option_of_certificateResponse)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for ChairTokenOwnerResponse (SEQUENCE)  */
/* <==================================================> */
void Delete_ChairTokenOwnerResponse(PS_ChairTokenOwnerResponse x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for ExtensionAddressResponse (SEQUENCE)  */
/* <===================================================> */
void Delete_ExtensionAddressResponse(PS_ExtensionAddressResponse x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for MakeMeChairResponse (CHOICE)  */
/* <============================================> */
void Delete_MakeMeChairResponse(PS_MakeMeChairResponse x)
{
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
            ErrorMessage("Delete_MakeMeChairResponse: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for PasswordResponse (SEQUENCE)  */
/* <===========================================> */
void Delete_PasswordResponse(PS_PasswordResponse x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for ConferenceIDResponse (SEQUENCE)  */
/* <===============================================> */
void Delete_ConferenceIDResponse(PS_ConferenceIDResponse x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for TerminalIDResponse (SEQUENCE)  */
/* <=============================================> */
void Delete_TerminalIDResponse(PS_TerminalIDResponse x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for MCTerminalIDResponse (SEQUENCE)  */
/* <===============================================> */
void Delete_MCTerminalIDResponse(PS_MCTerminalIDResponse x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================================> */
/*  PER-Deleter for RequestAllTerminalIDsResponse (SEQUENCE)  */
/* <========================================================> */
void Delete_RequestAllTerminalIDsResponse(PS_RequestAllTerminalIDsResponse x)
{
    uint16 i = 0;
    for (i = 0;i < x->size_of_terminalInformation;++i)
    {
        Delete_TerminalInformation(x->terminalInformation + i);
    }
    OSCL_DEFAULT_FREE(x->terminalInformation);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================================> */
/*  PER-Deleter for TerminalInformation (SEQUENCE)  */
/* <==============================================> */
void Delete_TerminalInformation(PS_TerminalInformation x)
{
    Delete_TerminalLabel(&x->terminalLabel);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for RemoteMCRequest (CHOICE)  */
/* <========================================> */
void Delete_RemoteMCRequest(PS_RemoteMCRequest x)
{
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
            ErrorMessage("Delete_RemoteMCRequest: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for RemoteMCResponse (CHOICE)  */
/* <=========================================> */
void Delete_RemoteMCResponse(PS_RemoteMCResponse x)
{
    switch (x->index)
    {
        case 0:
            /* (accept is NULL) */
            break;
        case 1:
            Delete_Reject(x->reject);
            OSCL_DEFAULT_FREE(x->reject);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_RemoteMCResponse: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Deleter for Reject (CHOICE)  */
/* <===============================> */
void Delete_Reject(PS_Reject x)
{
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
            ErrorMessage("Delete_Reject: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for MultilinkRequest (CHOICE)  */
/* <=========================================> */
void Delete_MultilinkRequest(PS_MultilinkRequest x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_CallInformation(x->callInformation);
            OSCL_DEFAULT_FREE(x->callInformation);
            break;
        case 2:
            Delete_AddConnection(x->addConnection);
            OSCL_DEFAULT_FREE(x->addConnection);
            break;
        case 3:
            Delete_RemoveConnection(x->removeConnection);
            OSCL_DEFAULT_FREE(x->removeConnection);
            break;
        case 4:
            Delete_MaximumHeaderInterval(x->maximumHeaderInterval);
            OSCL_DEFAULT_FREE(x->maximumHeaderInterval);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MultilinkRequest: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for MaximumHeaderInterval (SEQUENCE)  */
/* <================================================> */
void Delete_MaximumHeaderInterval(PS_MaximumHeaderInterval x)
{
    Delete_RequestType(&x->requestType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Deleter for RequestType (CHOICE)  */
/* <====================================> */
void Delete_RequestType(PS_RequestType x)
{
    switch (x->index)
    {
        case 0:
            /* (currentIntervalInformation is NULL) */
            break;
        case 1:
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_RequestType: Illegal CHOICE index");
    }
}

/* <===========================================> */
/*  PER-Deleter for RemoveConnection (SEQUENCE)  */
/* <===========================================> */
void Delete_RemoveConnection(PS_RemoveConnection x)
{
    Delete_ConnectionIdentifier(&x->connectionIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================> */
/*  PER-Deleter for AddConnection (SEQUENCE)  */
/* <========================================> */
void Delete_AddConnection(PS_AddConnection x)
{
    Delete_DialingInformation(&x->dialingInformation);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for CallInformation (SEQUENCE)  */
/* <==========================================> */
void Delete_CallInformation(PS_CallInformation x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for MultilinkResponse (CHOICE)  */
/* <==========================================> */
void Delete_MultilinkResponse(PS_MultilinkResponse x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_RespCallInformation(x->respCallInformation);
            OSCL_DEFAULT_FREE(x->respCallInformation);
            break;
        case 2:
            Delete_RespAddConnection(x->respAddConnection);
            OSCL_DEFAULT_FREE(x->respAddConnection);
            break;
        case 3:
            Delete_RespRemoveConnection(x->respRemoveConnection);
            OSCL_DEFAULT_FREE(x->respRemoveConnection);
            break;
        case 4:
            Delete_RespMaximumHeaderInterval(x->respMaximumHeaderInterval);
            OSCL_DEFAULT_FREE(x->respMaximumHeaderInterval);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MultilinkResponse: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Deleter for RespMaximumHeaderInterval (SEQUENCE)  */
/* <====================================================> */
void Delete_RespMaximumHeaderInterval(PS_RespMaximumHeaderInterval x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for RespRemoveConnection (SEQUENCE)  */
/* <===============================================> */
void Delete_RespRemoveConnection(PS_RespRemoveConnection x)
{
    Delete_ConnectionIdentifier(&x->connectionIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for RespAddConnection (SEQUENCE)  */
/* <============================================> */
void Delete_RespAddConnection(PS_RespAddConnection x)
{
    Delete_ResponseCode(&x->responseCode);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for ResponseCode (CHOICE)  */
/* <=====================================> */
void Delete_ResponseCode(PS_ResponseCode x)
{
    switch (x->index)
    {
        case 0:
            /* (accepted is NULL) */
            break;
        case 1:
            Delete_Rejected(x->rejected);
            OSCL_DEFAULT_FREE(x->rejected);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_ResponseCode: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Deleter for Rejected (CHOICE)  */
/* <=================================> */
void Delete_Rejected(PS_Rejected x)
{
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
            ErrorMessage("Delete_Rejected: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Deleter for RespCallInformation (SEQUENCE)  */
/* <==============================================> */
void Delete_RespCallInformation(PS_RespCallInformation x)
{
    Delete_DialingInformation(&x->dialingInformation);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <============================================> */
/*  PER-Deleter for MultilinkIndication (CHOICE)  */
/* <============================================> */
void Delete_MultilinkIndication(PS_MultilinkIndication x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            Delete_CrcDesired(x->crcDesired);
            OSCL_DEFAULT_FREE(x->crcDesired);
            break;
        case 2:
            Delete_ExcessiveError(x->excessiveError);
            OSCL_DEFAULT_FREE(x->excessiveError);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_MultilinkIndication: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for ExcessiveError (SEQUENCE)  */
/* <=========================================> */
void Delete_ExcessiveError(PS_ExcessiveError x)
{
    Delete_ConnectionIdentifier(&x->connectionIdentifier);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for CrcDesired (SEQUENCE)  */
/* <=====================================> */
void Delete_CrcDesired(PS_CrcDesired x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for DialingInformation (CHOICE)  */
/* <===========================================> */
void Delete_DialingInformation(PS_DialingInformation x)
{
    uint16 i;
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            for (i = 0;i < x->size;++i)
            {
                Delete_DialingInformationNumber(x->differential + i);
            }
            OSCL_DEFAULT_FREE(x->differential);
            break;
        case 2:
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_DialingInformation: Illegal CHOICE index");
    }
}

/* <===================================================> */
/*  PER-Deleter for DialingInformationNumber (SEQUENCE)  */
/* <===================================================> */
void Delete_DialingInformationNumber(PS_DialingInformationNumber x)
{
    uint16 i = 0;
    if (x->option_of_subAddress)
    {
    }
    for (i = 0;i < x->size_of_networkType;++i)
    {
        Delete_DialingInformationNetworkType(x->networkType + i);
    }
    OSCL_DEFAULT_FREE(x->networkType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <======================================================> */
/*  PER-Deleter for DialingInformationNetworkType (CHOICE)  */
/* <======================================================> */
void Delete_DialingInformationNetworkType(PS_DialingInformationNetworkType x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardMessage(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            ErrorMessage("Delete_DialingInformationNetworkType: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Deleter for ConnectionIdentifier (SEQUENCE)  */
/* <===============================================> */
void Delete_ConnectionIdentifier(PS_ConnectionIdentifier x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================================> */
/*  PER-Deleter for LogicalChannelRateRequest (SEQUENCE)  */
/* <====================================================> */
void Delete_LogicalChannelRateRequest(PS_LogicalChannelRateRequest x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <========================================================> */
/*  PER-Deleter for LogicalChannelRateAcknowledge (SEQUENCE)  */
/* <========================================================> */
void Delete_LogicalChannelRateAcknowledge(PS_LogicalChannelRateAcknowledge x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for LogicalChannelRateReject (SEQUENCE)  */
/* <===================================================> */
void Delete_LogicalChannelRateReject(PS_LogicalChannelRateReject x)
{
    Delete_LogicalChannelRateRejectReason(&x->rejectReason);
    if (x->option_of_currentMaximumBitRate)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================================> */
/*  PER-Deleter for LogicalChannelRateRejectReason (CHOICE)  */
/* <=======================================================> */
void Delete_LogicalChannelRateRejectReason(PS_LogicalChannelRateRejectReason x)
{
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
            ErrorMessage("Delete_LogicalChannelRateRejectReason: Illegal CHOICE index");
    }
}

/* <====================================================> */
/*  PER-Deleter for LogicalChannelRateRelease (SEQUENCE)  */
/* <====================================================> */
void Delete_LogicalChannelRateRelease(PS_LogicalChannelRateRelease x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for SendTerminalCapabilitySet (CHOICE)  */
/* <==================================================> */
void Delete_SendTerminalCapabilitySet(PS_SendTerminalCapabilitySet x)
{
    switch (x->index)
    {
        case 0:
            Delete_SpecificRequest(x->specificRequest);
            OSCL_DEFAULT_FREE(x->specificRequest);
            break;
        case 1:
            /* (genericRequest is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_SendTerminalCapabilitySet: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for SpecificRequest (SEQUENCE)  */
/* <==========================================> */
void Delete_SpecificRequest(PS_SpecificRequest x)
{
    if (x->option_of_capabilityTableEntryNumbers)
    {
        OSCL_DEFAULT_FREE(x->capabilityTableEntryNumbers);
    }
    if (x->option_of_capabilityDescriptorNumbers)
    {
        OSCL_DEFAULT_FREE(x->capabilityDescriptorNumbers);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==========================================> */
/*  PER-Deleter for EncryptionCommand (CHOICE)  */
/* <==========================================> */
void Delete_EncryptionCommand(PS_EncryptionCommand x)
{
    switch (x->index)
    {
        case 0:
            FreeOctetString(x->encryptionSE);
            break;
        case 1:
            /* (encryptionIVRequest is NULL) */
            break;
        case 2:
            Delete_EncryptionAlgorithmID(x->encryptionAlgorithmID);
            OSCL_DEFAULT_FREE(x->encryptionAlgorithmID);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_EncryptionCommand: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for EncryptionAlgorithmID (SEQUENCE)  */
/* <================================================> */
void Delete_EncryptionAlgorithmID(PS_EncryptionAlgorithmID x)
{
    Delete_NonStandardParameter(&x->associatedAlgorithm);
}

/* <=============================================> */
/*  PER-Deleter for FlowControlCommand (SEQUENCE)  */
/* <=============================================> */
void Delete_FlowControlCommand(PS_FlowControlCommand x)
{
    Delete_FccScope(&x->fccScope);
    Delete_FccRestriction(&x->fccRestriction);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for FccRestriction (CHOICE)  */
/* <=======================================> */
void Delete_FccRestriction(PS_FccRestriction x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessage("Delete_FccRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Deleter for FccScope (CHOICE)  */
/* <=================================> */
void Delete_FccScope(PS_FccScope x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessage("Delete_FccScope: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for EndSessionCommand (CHOICE)  */
/* <==========================================> */
void Delete_EndSessionCommand(PS_EndSessionCommand x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            /* (disconnect is NULL) */
            break;
        case 2:
            Delete_GstnOptions(x->gstnOptions);
            OSCL_DEFAULT_FREE(x->gstnOptions);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 3:
            Delete_IsdnOptions(x->isdnOptions);
            OSCL_DEFAULT_FREE(x->isdnOptions);
            break;
        default:
            ErrorMessage("Delete_EndSessionCommand: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Deleter for IsdnOptions (CHOICE)  */
/* <====================================> */
void Delete_IsdnOptions(PS_IsdnOptions x)
{
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
            ErrorMessage("Delete_IsdnOptions: Illegal CHOICE index");
    }
}

/* <====================================> */
/*  PER-Deleter for GstnOptions (CHOICE)  */
/* <====================================> */
void Delete_GstnOptions(PS_GstnOptions x)
{
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
            ErrorMessage("Delete_GstnOptions: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for ConferenceCommand (CHOICE)  */
/* <==========================================> */
void Delete_ConferenceCommand(PS_ConferenceCommand x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            Delete_TerminalLabel(x->makeTerminalBroadcaster);
            OSCL_DEFAULT_FREE(x->makeTerminalBroadcaster);
            break;
        case 3:
            /* (cancelMakeTerminalBroadcaster is NULL) */
            break;
        case 4:
            Delete_TerminalLabel(x->sendThisSource);
            OSCL_DEFAULT_FREE(x->sendThisSource);
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
            Delete_SubstituteConferenceIDCommand(x->substituteConferenceIDCommand);
            OSCL_DEFAULT_FREE(x->substituteConferenceIDCommand);
            break;
        default:
            ErrorMessage("Delete_ConferenceCommand: Illegal CHOICE index");
    }
}

/* <========================================================> */
/*  PER-Deleter for SubstituteConferenceIDCommand (SEQUENCE)  */
/* <========================================================> */
void Delete_SubstituteConferenceIDCommand(PS_SubstituteConferenceIDCommand x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for MiscellaneousCommand (SEQUENCE)  */
/* <===============================================> */
void Delete_MiscellaneousCommand(PS_MiscellaneousCommand x)
{
    Delete_McType(&x->mcType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================> */
/*  PER-Deleter for McType (CHOICE)  */
/* <===============================> */
void Delete_McType(PS_McType x)
{
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
            Delete_VideoFastUpdateGOB(x->videoFastUpdateGOB);
            OSCL_DEFAULT_FREE(x->videoFastUpdateGOB);
            break;
        case 7:
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
            Delete_VideoFastUpdateMB(x->videoFastUpdateMB);
            OSCL_DEFAULT_FREE(x->videoFastUpdateMB);
            break;
        case 11:
            break;
        case 12:
            Delete_EncryptionSync(x->encryptionUpdate);
            OSCL_DEFAULT_FREE(x->encryptionUpdate);
            break;
        case 13:
            Delete_EncryptionUpdateRequest(x->encryptionUpdateRequest);
            OSCL_DEFAULT_FREE(x->encryptionUpdateRequest);
            break;
        case 14:
            break;
        case 15:
            break;
        case 16:
            Delete_ProgressiveRefinementStart(x->progressiveRefinementStart);
            OSCL_DEFAULT_FREE(x->progressiveRefinementStart);
            break;
        case 17:
            break;
        case 18:
            break;
        default:
            ErrorMessage("Delete_McType: Illegal CHOICE index");
    }
}

/* <=====================================================> */
/*  PER-Deleter for ProgressiveRefinementStart (SEQUENCE)  */
/* <=====================================================> */
void Delete_ProgressiveRefinementStart(PS_ProgressiveRefinementStart x)
{
    Delete_PrsRepeatCount(&x->prsRepeatCount);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for PrsRepeatCount (CHOICE)  */
/* <=======================================> */
void Delete_PrsRepeatCount(PS_PrsRepeatCount x)
{
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
            ErrorMessage("Delete_PrsRepeatCount: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Deleter for VideoFastUpdateMB (SEQUENCE)  */
/* <============================================> */
void Delete_VideoFastUpdateMB(PS_VideoFastUpdateMB x)
{
    if (x->option_of_firstGOB)
    {
    }
    if (x->option_of_firstMB)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for VideoFastUpdateGOB (SEQUENCE)  */
/* <=============================================> */
void Delete_VideoFastUpdateGOB(PS_VideoFastUpdateGOB x)
{
    OSCL_UNUSED_ARG(x);
}

/* <==============================================> */
/*  PER-Deleter for KeyProtectionMethod (SEQUENCE)  */
/* <==============================================> */
void Delete_KeyProtectionMethod(PS_KeyProtectionMethod x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for EncryptionUpdateRequest (SEQUENCE)  */
/* <==================================================> */
void Delete_EncryptionUpdateRequest(PS_EncryptionUpdateRequest x)
{
    if (x->option_of_keyProtectionMethod)
    {
        Delete_KeyProtectionMethod(&x->keyProtectionMethod);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Deleter for H223MultiplexReconfiguration (CHOICE)  */
/* <=====================================================> */
void Delete_H223MultiplexReconfiguration(PS_H223MultiplexReconfiguration x)
{
    switch (x->index)
    {
        case 0:
            Delete_H223ModeChange(x->h223ModeChange);
            OSCL_DEFAULT_FREE(x->h223ModeChange);
            break;
        case 1:
            Delete_H223AnnexADoubleFlag(x->h223AnnexADoubleFlag);
            OSCL_DEFAULT_FREE(x->h223AnnexADoubleFlag);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_H223MultiplexReconfiguration: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for H223AnnexADoubleFlag (CHOICE)  */
/* <=============================================> */
void Delete_H223AnnexADoubleFlag(PS_H223AnnexADoubleFlag x)
{
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
            ErrorMessage("Delete_H223AnnexADoubleFlag: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for H223ModeChange (CHOICE)  */
/* <=======================================> */
void Delete_H223ModeChange(PS_H223ModeChange x)
{
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
            ErrorMessage("Delete_H223ModeChange: Illegal CHOICE index");
    }
}

/* <==========================================> */
/*  PER-Deleter for NewATMVCCommand (SEQUENCE)  */
/* <==========================================> */
void Delete_NewATMVCCommand(PS_NewATMVCCommand x)
{
    Delete_CmdAal(&x->cmdAal);
    Delete_CmdMultiplex(&x->cmdMultiplex);
    Delete_CmdReverseParameters(&x->cmdReverseParameters);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for CmdReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Delete_CmdReverseParameters(PS_CmdReverseParameters x)
{
    Delete_Multiplex(&x->multiplex);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for Multiplex (CHOICE)  */
/* <==================================> */
void Delete_Multiplex(PS_Multiplex x)
{
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
            ErrorMessage("Delete_Multiplex: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for CmdMultiplex (CHOICE)  */
/* <=====================================> */
void Delete_CmdMultiplex(PS_CmdMultiplex x)
{
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
            ErrorMessage("Delete_CmdMultiplex: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Deleter for CmdAal (CHOICE)  */
/* <===============================> */
void Delete_CmdAal(PS_CmdAal x)
{
    switch (x->index)
    {
        case 0:
            Delete_CmdAal1(x->cmdAal1);
            OSCL_DEFAULT_FREE(x->cmdAal1);
            break;
        case 1:
            Delete_CmdAal5(x->cmdAal5);
            OSCL_DEFAULT_FREE(x->cmdAal5);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_CmdAal: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for CmdAal5 (SEQUENCE)  */
/* <==================================> */
void Delete_CmdAal5(PS_CmdAal5 x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for CmdAal1 (SEQUENCE)  */
/* <==================================> */
void Delete_CmdAal1(PS_CmdAal1 x)
{
    Delete_CmdClockRecovery(&x->cmdClockRecovery);
    Delete_CmdErrorCorrection(&x->cmdErrorCorrection);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for CmdErrorCorrection (CHOICE)  */
/* <===========================================> */
void Delete_CmdErrorCorrection(PS_CmdErrorCorrection x)
{
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
            ErrorMessage("Delete_CmdErrorCorrection: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for CmdClockRecovery (CHOICE)  */
/* <=========================================> */
void Delete_CmdClockRecovery(PS_CmdClockRecovery x)
{
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
            ErrorMessage("Delete_CmdClockRecovery: Illegal CHOICE index");
    }
}

/* <==============================================> */
/*  PER-Deleter for FunctionNotUnderstood (CHOICE)  */
/* <==============================================> */
void Delete_FunctionNotUnderstood(PS_FunctionNotUnderstood x)
{
    switch (x->index)
    {
        case 0:
            Delete_RequestMessage(x->request);
            OSCL_DEFAULT_FREE(x->request);
            break;
        case 1:
            Delete_ResponseMessage(x->response);
            OSCL_DEFAULT_FREE(x->response);
            break;
        case 2:
            Delete_CommandMessage(x->command);
            OSCL_DEFAULT_FREE(x->command);
            break;
        default:
            ErrorMessage("Delete_FunctionNotUnderstood: Illegal CHOICE index");
    }
}

/* <===============================================> */
/*  PER-Deleter for FunctionNotSupported (SEQUENCE)  */
/* <===============================================> */
void Delete_FunctionNotSupported(PS_FunctionNotSupported x)
{
    Delete_FnsCause(&x->fnsCause);
    if (x->option_of_returnedFunction)
    {
        if (x->returnedFunction.size > 0) OSCL_DEFAULT_FREE(x->returnedFunction.data);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for FnsCause (CHOICE)  */
/* <=================================> */
void Delete_FnsCause(PS_FnsCause x)
{
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
            ErrorMessage("Delete_FnsCause: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for ConferenceIndication (CHOICE)  */
/* <=============================================> */
void Delete_ConferenceIndication(PS_ConferenceIndication x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            Delete_TerminalLabel(x->terminalNumberAssign);
            OSCL_DEFAULT_FREE(x->terminalNumberAssign);
            break;
        case 2:
            Delete_TerminalLabel(x->terminalJoinedConference);
            OSCL_DEFAULT_FREE(x->terminalJoinedConference);
            break;
        case 3:
            Delete_TerminalLabel(x->terminalLeftConference);
            OSCL_DEFAULT_FREE(x->terminalLeftConference);
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
            Delete_TerminalLabel(x->terminalYouAreSeeing);
            OSCL_DEFAULT_FREE(x->terminalYouAreSeeing);
            break;
        case 9:
            /* (requestForFloor is NULL) */
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            break;
        case 11:
            Delete_TerminalLabel(x->floorRequested);
            OSCL_DEFAULT_FREE(x->floorRequested);
            break;
        case 12:
            Delete_TerminalYouAreSeeingInSubPictureNumber(x->terminalYouAreSeeingInSubPictureNumber);
            OSCL_DEFAULT_FREE(x->terminalYouAreSeeingInSubPictureNumber);
            break;
        case 13:
            Delete_VideoIndicateCompose(x->videoIndicateCompose);
            OSCL_DEFAULT_FREE(x->videoIndicateCompose);
            break;
        default:
            ErrorMessage("Delete_ConferenceIndication: Illegal CHOICE index");
    }
}

/* <=================================================================> */
/*  PER-Deleter for TerminalYouAreSeeingInSubPictureNumber (SEQUENCE)  */
/* <=================================================================> */
void Delete_TerminalYouAreSeeingInSubPictureNumber(PS_TerminalYouAreSeeingInSubPictureNumber x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for VideoIndicateCompose (SEQUENCE)  */
/* <===============================================> */
void Delete_VideoIndicateCompose(PS_VideoIndicateCompose x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================================> */
/*  PER-Deleter for MiscellaneousIndication (SEQUENCE)  */
/* <==================================================> */
void Delete_MiscellaneousIndication(PS_MiscellaneousIndication x)
{
    Delete_MiType(&x->miType);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================> */
/*  PER-Deleter for MiType (CHOICE)  */
/* <===============================> */
void Delete_MiType(PS_MiType x)
{
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
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 10:
            Delete_VideoNotDecodedMBs(x->videoNotDecodedMBs);
            OSCL_DEFAULT_FREE(x->videoNotDecodedMBs);
            break;
        case 11:
            Delete_TransportCapability(x->transportCapability);
            OSCL_DEFAULT_FREE(x->transportCapability);
            break;
        default:
            ErrorMessage("Delete_MiType: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for VideoNotDecodedMBs (SEQUENCE)  */
/* <=============================================> */
void Delete_VideoNotDecodedMBs(PS_VideoNotDecodedMBs x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for JitterIndication (SEQUENCE)  */
/* <===========================================> */
void Delete_JitterIndication(PS_JitterIndication x)
{
    Delete_JiScope(&x->jiScope);
    if (x->option_of_skippedFrameCount)
    {
    }
    if (x->option_of_additionalDecoderBuffer)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <================================> */
/*  PER-Deleter for JiScope (CHOICE)  */
/* <================================> */
void Delete_JiScope(PS_JiScope x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessage("Delete_JiScope: Illegal CHOICE index");
    }
}

/* <=============================================> */
/*  PER-Deleter for H223SkewIndication (SEQUENCE)  */
/* <=============================================> */
void Delete_H223SkewIndication(PS_H223SkewIndication x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================================> */
/*  PER-Deleter for H2250MaximumSkewIndication (SEQUENCE)  */
/* <=====================================================> */
void Delete_H2250MaximumSkewIndication(PS_H2250MaximumSkewIndication x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for MCLocationIndication (SEQUENCE)  */
/* <===============================================> */
void Delete_MCLocationIndication(PS_MCLocationIndication x)
{
    Delete_TransportAddress(&x->signalAddress);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===============================================> */
/*  PER-Deleter for VendorIdentification (SEQUENCE)  */
/* <===============================================> */
void Delete_VendorIdentification(PS_VendorIdentification x)
{
    Delete_NonStandardIdentifier(&x->vendor);
    if (x->option_of_productNumber)
    {
        OSCL_DEFAULT_FREE(x->productNumber.data);
        x->productNumber.data = NULL;
    }
    if (x->option_of_versionNumber)
    {
        OSCL_DEFAULT_FREE(x->versionNumber.data);
        x->versionNumber.data = NULL;
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=============================================> */
/*  PER-Deleter for NewATMVCIndication (SEQUENCE)  */
/* <=============================================> */
void Delete_NewATMVCIndication(PS_NewATMVCIndication x)
{
    uint32 extension;

    extension = x->option_of_indReverseParameters;
    Delete_IndAal(&x->indAal);
    Delete_IndMultiplex(&x->indMultiplex);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
    if (!extension) return; /* (No items present?) */
    if (x->option_of_indReverseParameters)
    {
        Delete_IndReverseParameters(&x->indReverseParameters);
    }
}

/* <===============================================> */
/*  PER-Deleter for IndReverseParameters (SEQUENCE)  */
/* <===============================================> */
void Delete_IndReverseParameters(PS_IndReverseParameters x)
{
    Delete_IrpMultiplex(&x->irpMultiplex);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=====================================> */
/*  PER-Deleter for IrpMultiplex (CHOICE)  */
/* <=====================================> */
void Delete_IrpMultiplex(PS_IrpMultiplex x)
{
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
            ErrorMessage("Delete_IrpMultiplex: Illegal CHOICE index");
    }
}

/* <=====================================> */
/*  PER-Deleter for IndMultiplex (CHOICE)  */
/* <=====================================> */
void Delete_IndMultiplex(PS_IndMultiplex x)
{
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
            ErrorMessage("Delete_IndMultiplex: Illegal CHOICE index");
    }
}

/* <===============================> */
/*  PER-Deleter for IndAal (CHOICE)  */
/* <===============================> */
void Delete_IndAal(PS_IndAal x)
{
    switch (x->index)
    {
        case 0:
            Delete_IndAal1(x->indAal1);
            OSCL_DEFAULT_FREE(x->indAal1);
            break;
        case 1:
            Delete_IndAal5(x->indAal5);
            OSCL_DEFAULT_FREE(x->indAal5);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("Delete_IndAal: Illegal CHOICE index");
    }
}

/* <==================================> */
/*  PER-Deleter for IndAal5 (SEQUENCE)  */
/* <==================================> */
void Delete_IndAal5(PS_IndAal5 x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==================================> */
/*  PER-Deleter for IndAal1 (SEQUENCE)  */
/* <==================================> */
void Delete_IndAal1(PS_IndAal1 x)
{
    Delete_IndClockRecovery(&x->indClockRecovery);
    Delete_IndErrorCorrection(&x->indErrorCorrection);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===========================================> */
/*  PER-Deleter for IndErrorCorrection (CHOICE)  */
/* <===========================================> */
void Delete_IndErrorCorrection(PS_IndErrorCorrection x)
{
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
            ErrorMessage("Delete_IndErrorCorrection: Illegal CHOICE index");
    }
}

/* <=========================================> */
/*  PER-Deleter for IndClockRecovery (CHOICE)  */
/* <=========================================> */
void Delete_IndClockRecovery(PS_IndClockRecovery x)
{
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
            ErrorMessage("Delete_IndClockRecovery: Illegal CHOICE index");
    }
}

/* <============================================> */
/*  PER-Deleter for UserInputIndication (CHOICE)  */
/* <============================================> */
void Delete_UserInputIndication(PS_UserInputIndication x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
            break;
        case 1:
            FreeCharString(x->alphanumeric);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 2:
            Delete_UserInputSupportIndication(x->userInputSupportIndication);
            OSCL_DEFAULT_FREE(x->userInputSupportIndication);
            break;
        case 3:
            Delete_Signal(x->signal);
            OSCL_DEFAULT_FREE(x->signal);
            break;
        case 4:
            Delete_SignalUpdate(x->signalUpdate);
            OSCL_DEFAULT_FREE(x->signalUpdate);
            break;
        default:
            ErrorMessage("Delete_UserInputIndication: Illegal CHOICE index");
    }
}

/* <=======================================> */
/*  PER-Deleter for SignalUpdate (SEQUENCE)  */
/* <=======================================> */
void Delete_SignalUpdate(PS_SignalUpdate x)
{
    if (x->option_of_rtp)
    {
        Delete_Rtp(&x->rtp);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <==============================> */
/*  PER-Deleter for Rtp (SEQUENCE)  */
/* <==============================> */
void Delete_Rtp(PS_Rtp x)
{
    OSCL_UNUSED_ARG(x);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=================================> */
/*  PER-Deleter for Signal (SEQUENCE)  */
/* <=================================> */
void Delete_Signal(PS_Signal x)
{
    if (x->signalType.data)
    {
        OSCL_DEFAULT_FREE(x->signalType.data);
    }
    if (x->option_of_duration)
    {
    }
    if (x->option_of_signalRtp)
    {
        Delete_SignalRtp(&x->signalRtp);
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <====================================> */
/*  PER-Deleter for SignalRtp (SEQUENCE)  */
/* <====================================> */
void Delete_SignalRtp(PS_SignalRtp x)
{
    if (x->option_of_timestamp)
    {
    }
    if (x->option_of_expirationTime)
    {
    }
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <===================================================> */
/*  PER-Deleter for UserInputSupportIndication (CHOICE)  */
/* <===================================================> */
void Delete_UserInputSupportIndication(PS_UserInputSupportIndication x)
{
    switch (x->index)
    {
        case 0:
            Delete_NonStandardParameter(x->nonStandard);
            OSCL_DEFAULT_FREE(x->nonStandard);
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
            ErrorMessage("Delete_UserInputSupportIndication: Illegal CHOICE index");
    }
}

/* <================================================> */
/*  PER-Deleter for FlowControlIndication (SEQUENCE)  */
/* <================================================> */
void Delete_FlowControlIndication(PS_FlowControlIndication x)
{
    Delete_FciScope(&x->fciScope);
    Delete_FciRestriction(&x->fciRestriction);
    /* ------------------------------- */
    /* ---- Extension Begins Here ---- */
    /* ------------------------------- */
}

/* <=======================================> */
/*  PER-Deleter for FciRestriction (CHOICE)  */
/* <=======================================> */
void Delete_FciRestriction(PS_FciRestriction x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            /* (noRestriction is NULL) */
            break;
        default:
            ErrorMessage("Delete_FciRestriction: Illegal CHOICE index");
    }
}

/* <=================================> */
/*  PER-Deleter for FciScope (CHOICE)  */
/* <=================================> */
void Delete_FciScope(PS_FciScope x)
{
    switch (x->index)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            /* (wholeMultiplex is NULL) */
            break;
        default:
            ErrorMessage("Delete_FciScope: Illegal CHOICE index");
    }
}

