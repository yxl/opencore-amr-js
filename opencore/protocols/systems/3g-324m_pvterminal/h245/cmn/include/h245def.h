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
// FILE: h245def.h
//
// DESC: H.245 Structure definitions for PER encode/decode routines
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

#ifndef H245DEF
#define H245DEF

#include "genericper.h"

typedef struct _MultimediaSystemControlMessage     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _RequestMessage *request;
        struct _ResponseMessage *response;
        struct _CommandMessage *command;
        struct _IndicationMessage *indication;
        /*[...]*/
    };
} S_MultimediaSystemControlMessage;
typedef S_MultimediaSystemControlMessage *PS_MultimediaSystemControlMessage;

/*-------------------------------------------------------*/

typedef struct _RequestMessage     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _MasterSlaveDetermination *masterSlaveDetermination;
        struct _TerminalCapabilitySet *terminalCapabilitySet;
        struct _OpenLogicalChannel *openLogicalChannel;
        struct _CloseLogicalChannel *closeLogicalChannel;
        struct _RequestChannelClose *requestChannelClose;
        struct _MultiplexEntrySend *multiplexEntrySend;
        struct _RequestMultiplexEntry *requestMultiplexEntry;
        struct _RequestMode *requestMode;
        struct _RoundTripDelayRequest *roundTripDelayRequest;
        struct _MaintenanceLoopRequest *maintenanceLoopRequest;
        /*[...]*/
        struct _CommunicationModeRequest *communicationModeRequest;
        struct _ConferenceRequest *conferenceRequest;
        struct _MultilinkRequest *multilinkRequest;
        struct _LogicalChannelRateRequest *logicalChannelRateRequest;
    };
} S_RequestMessage;
typedef S_RequestMessage *PS_RequestMessage;

/*-------------------------------------------------------*/

typedef struct _ResponseMessage     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _MasterSlaveDeterminationAck *masterSlaveDeterminationAck;
        struct _MasterSlaveDeterminationReject *masterSlaveDeterminationReject;
        struct _TerminalCapabilitySetAck *terminalCapabilitySetAck;
        struct _TerminalCapabilitySetReject *terminalCapabilitySetReject;
        struct _OpenLogicalChannelAck *openLogicalChannelAck;
        struct _OpenLogicalChannelReject *openLogicalChannelReject;
        struct _CloseLogicalChannelAck *closeLogicalChannelAck;
        struct _RequestChannelCloseAck *requestChannelCloseAck;
        struct _RequestChannelCloseReject *requestChannelCloseReject;
        struct _MultiplexEntrySendAck *multiplexEntrySendAck;
        struct _MultiplexEntrySendReject *multiplexEntrySendReject;
        struct _RequestMultiplexEntryAck *requestMultiplexEntryAck;
        struct _RequestMultiplexEntryReject *requestMultiplexEntryReject;
        struct _RequestModeAck *requestModeAck;
        struct _RequestModeReject *requestModeReject;
        struct _RoundTripDelayResponse *roundTripDelayResponse;
        struct _MaintenanceLoopAck *maintenanceLoopAck;
        struct _MaintenanceLoopReject *maintenanceLoopReject;
        /*[...]*/
        struct _CommunicationModeResponse *communicationModeResponse;
        struct _ConferenceResponse *conferenceResponse;
        struct _MultilinkResponse *multilinkResponse;
        struct _LogicalChannelRateAcknowledge *logicalChannelRateAcknowledge;
        struct _LogicalChannelRateReject *logicalChannelRateReject;
    };
} S_ResponseMessage;
typedef S_ResponseMessage *PS_ResponseMessage;

/*-------------------------------------------------------*/

typedef struct _CommandMessage     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _MaintenanceLoopOffCommand *maintenanceLoopOffCommand;
        struct _SendTerminalCapabilitySet *sendTerminalCapabilitySet;
        struct _EncryptionCommand *encryptionCommand;
        struct _FlowControlCommand *flowControlCommand;
        struct _EndSessionCommand *endSessionCommand;
        struct _MiscellaneousCommand *miscellaneousCommand;
        /*[...]*/
        struct _CommunicationModeCommand *communicationModeCommand;
        struct _ConferenceCommand *conferenceCommand;
        struct _H223MultiplexReconfiguration *h223MultiplexReconfiguration;
        struct _NewATMVCCommand *newATMVCCommand;
    };
} S_CommandMessage;
typedef S_CommandMessage *PS_CommandMessage;

/*-------------------------------------------------------*/

typedef struct _IndicationMessage     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _FunctionNotUnderstood *functionNotUnderstood;
        struct _MasterSlaveDeterminationRelease *masterSlaveDeterminationRelease;
        struct _TerminalCapabilitySetRelease *terminalCapabilitySetRelease;
        struct _OpenLogicalChannelConfirm *openLogicalChannelConfirm;
        struct _RequestChannelCloseRelease *requestChannelCloseRelease;
        struct _MultiplexEntrySendRelease *multiplexEntrySendRelease;
        struct _RequestMultiplexEntryRelease *requestMultiplexEntryRelease;
        struct _RequestModeRelease *requestModeRelease;
        struct _MiscellaneousIndication *miscellaneousIndication;
        struct _JitterIndication *jitterIndication;
        struct _H223SkewIndication *h223SkewIndication;
        struct _NewATMVCIndication *newATMVCIndication;
        struct _UserInputIndication *userInput;
        /*[...]*/
        struct _H2250MaximumSkewIndication *h2250MaximumSkewIndication;
        struct _MCLocationIndication *mcLocationIndication;
        struct _ConferenceIndication *conferenceIndication;
        struct _VendorIdentification *vendorIdentification;
        struct _FunctionNotSupported *functionNotSupported;
        struct _MultilinkIndication *multilinkIndication;
        struct _LogicalChannelRateRelease *logicalChannelRateRelease;
        struct _FlowControlIndication *flowControlIndication;
    };
} S_IndicationMessage;
typedef S_IndicationMessage *PS_IndicationMessage;

/*-------------------------------------------------------*/

typedef struct _NonStandardIdentifier     /* CHOICE */
{
    uint16 index;
    union
    {
        PS_OBJECTIDENT object;
        struct _H221NonStandard *h221NonStandard;
    };
} S_NonStandardIdentifier;
typedef S_NonStandardIdentifier *PS_NonStandardIdentifier;

/*-------------------------------------------------------*/

typedef struct _NonStandardParameter     /* SEQUENCE */
{
    struct _NonStandardIdentifier nonStandardIdentifier;
    S_OCTETSTRING data;
} S_NonStandardParameter;
typedef S_NonStandardParameter *PS_NonStandardParameter;

/*-------------------------------------------------------*/

typedef struct _NonStandardMessage     /* SEQUENCE */
{
    struct _NonStandardParameter nonStandardData;
    /*[...]*/
} S_NonStandardMessage;
typedef S_NonStandardMessage *PS_NonStandardMessage;

/*-------------------------------------------------------*/

typedef struct _H221NonStandard     /* SEQUENCE */
{
    uint8 t35CountryCode;
    uint8 t35Extension;
    uint16 manufacturerCode;
} S_H221NonStandard;
typedef S_H221NonStandard *PS_H221NonStandard;

/*-------------------------------------------------------*/

typedef struct _MasterSlaveDetermination     /* SEQUENCE */
{
    uint8 terminalType;
    uint32 statusDeterminationNumber;
    /*[...]*/
} S_MasterSlaveDetermination;
typedef S_MasterSlaveDetermination *PS_MasterSlaveDetermination;

/*-------------------------------------------------------*/

typedef struct _Decision     /* CHOICE */
{
    uint16 index;
    /* (NULL) master */
    /* (NULL) slave */
} S_Decision;
typedef S_Decision *PS_Decision;

/*-------------------------------------------------------*/

typedef struct _MasterSlaveDeterminationAck     /* SEQUENCE */
{
    struct _Decision decision;
    /*[...]*/
} S_MasterSlaveDeterminationAck;
typedef S_MasterSlaveDeterminationAck *PS_MasterSlaveDeterminationAck;

/*-------------------------------------------------------*/

typedef struct _MsdRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) identicalNumbers */
    /*[...]*/
} S_MsdRejectCause;
typedef S_MsdRejectCause *PS_MsdRejectCause;

/*-------------------------------------------------------*/

typedef struct _MasterSlaveDeterminationReject     /* SEQUENCE */
{
    struct _MsdRejectCause msdRejectCause;
    /*[...]*/
} S_MasterSlaveDeterminationReject;
typedef S_MasterSlaveDeterminationReject *PS_MasterSlaveDeterminationReject;

/*-------------------------------------------------------*/

typedef struct _MasterSlaveDeterminationRelease     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_MasterSlaveDeterminationRelease;
typedef S_MasterSlaveDeterminationRelease *PS_MasterSlaveDeterminationRelease;

/*-------------------------------------------------------*/

typedef struct _CapabilityDescriptor     /* SEQUENCE */
{
uint32 option_of_simultaneousCapabilities :
    1;
    uint8 capabilityDescriptorNumber;
    uint16 size_of_simultaneousCapabilities;
    struct _AlternativeCapabilitySet *simultaneousCapabilities;
} S_CapabilityDescriptor;
typedef S_CapabilityDescriptor *PS_CapabilityDescriptor;

/*-------------------------------------------------------*/

typedef struct _AlternativeCapabilitySet     /* SEQUENCE OF */
{
    uint32* item;
    uint16 size;
} S_AlternativeCapabilitySet;
typedef S_AlternativeCapabilitySet *PS_AlternativeCapabilitySet;

/*-------------------------------------------------------*/

typedef struct _TerminalCapabilitySetAck     /* SEQUENCE */
{
    uint8 sequenceNumber;
    /*[...]*/
} S_TerminalCapabilitySetAck;
typedef S_TerminalCapabilitySetAck *PS_TerminalCapabilitySetAck;

/*-------------------------------------------------------*/

typedef struct _TcsRejectCause     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) unspecified */
        /* (NULL) undefinedTableEntryUsed */
        /* (NULL) descriptorCapacityExceeded */
        struct _TableEntryCapacityExceeded *tableEntryCapacityExceeded;
        /*[...]*/
    };
} S_TcsRejectCause;
typedef S_TcsRejectCause *PS_TcsRejectCause;

/*-------------------------------------------------------*/

typedef struct _TerminalCapabilitySetReject     /* SEQUENCE */
{
    uint8 sequenceNumber;
    struct _TcsRejectCause tcsRejectCause;
    /*[...]*/
} S_TerminalCapabilitySetReject;
typedef S_TerminalCapabilitySetReject *PS_TerminalCapabilitySetReject;

/*-------------------------------------------------------*/

typedef struct _TableEntryCapacityExceeded     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 highestEntryNumberProcessed;
        /* (NULL) noneProcessed */
    };
} S_TableEntryCapacityExceeded;
typedef S_TableEntryCapacityExceeded *PS_TableEntryCapacityExceeded;

/*-------------------------------------------------------*/

typedef struct _TerminalCapabilitySetRelease     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_TerminalCapabilitySetRelease;
typedef S_TerminalCapabilitySetRelease *PS_TerminalCapabilitySetRelease;

/*-------------------------------------------------------*/

typedef struct _Capability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _VideoCapability *receiveVideoCapability;
        struct _VideoCapability *transmitVideoCapability;
        struct _VideoCapability *receiveAndTransmitVideoCapability;
        struct _AudioCapability *receiveAudioCapability;
        struct _AudioCapability *transmitAudioCapability;
        struct _AudioCapability *receiveAndTransmitAudioCapability;
        struct _DataApplicationCapability *receiveDataApplicationCapability;
        struct _DataApplicationCapability *transmitDataApplicationCapability;
        struct _DataApplicationCapability *receiveAndTransmitDataApplicationCapability;
        uint8 h233EncryptionTransmitCapability;
        struct _H233EncryptionReceiveCapability *h233EncryptionReceiveCapability;
        /*[...]*/
        struct _ConferenceCapability *conferenceCapability;
        struct _H235SecurityCapability *h235SecurityCapability;
        uint8 maxPendingReplacementFor;
        struct _UserInputCapability *receiveUserInputCapability;
        struct _UserInputCapability *transmitUserInputCapability;
        struct _UserInputCapability *receiveAndTransmitUserInputCapability;
        struct _GenericCapability *genericControlCapability;
    };
} S_Capability;
typedef S_Capability *PS_Capability;

/*-------------------------------------------------------*/

typedef struct _CapabilityTableEntry     /* SEQUENCE */
{
uint32 option_of_capability :
    1;
    uint16 capabilityTableEntryNumber;
    struct _Capability capability;
} S_CapabilityTableEntry;
typedef S_CapabilityTableEntry *PS_CapabilityTableEntry;

/*-------------------------------------------------------*/

typedef struct _H233EncryptionReceiveCapability     /* SEQUENCE */
{
    uint8 h233IVResponseTime;
    /*[...]*/
} S_H233EncryptionReceiveCapability;
typedef S_H233EncryptionReceiveCapability *PS_H233EncryptionReceiveCapability;

/*-------------------------------------------------------*/

typedef struct _MultiplexCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _H222Capability *h222Capability;
        struct _H223Capability *h223Capability;
        struct _V76Capability *v76Capability;
        /*[...]*/
        struct _H2250Capability *h2250Capability;
        struct _GenericCapability *genericMultiplexCapability;
    };
} S_MultiplexCapability;
typedef S_MultiplexCapability *PS_MultiplexCapability;

/*-------------------------------------------------------*/

typedef struct _TerminalCapabilitySet     /* SEQUENCE */
{
uint32 option_of_multiplexCapability :
    1;
uint32 option_of_capabilityTable :
    1;
uint32 option_of_capabilityDescriptors :
    1;
    uint8 sequenceNumber;
    S_OBJECTIDENT protocolIdentifier;
    struct _MultiplexCapability multiplexCapability;
    uint16 size_of_capabilityTable;
    struct _CapabilityTableEntry *capabilityTable;
    uint16 size_of_capabilityDescriptors;
    struct _CapabilityDescriptor *capabilityDescriptors;
    /*[...]*/
uint32 option_of_genericInformation :
    1;
    uint16 size_of_genericInformation;
    struct _GenericInformation *genericInformation;
} S_TerminalCapabilitySet;
typedef S_TerminalCapabilitySet *PS_TerminalCapabilitySet;

/*-------------------------------------------------------*/

typedef struct _H222Capability     /* SEQUENCE */
{
    uint16 numberOfVCs;
    uint16 size_of_vcCapability;
    struct _VCCapability *vcCapability;
    /*[...]*/
} S_H222Capability;
typedef S_H222Capability *PS_H222Capability;

/*-------------------------------------------------------*/

typedef struct _Aal1ViaGateway     /* SEQUENCE */
{
    uint16 size_of_gatewayAddress;
    struct _Q2931Address *gatewayAddress;
uint32 nullClockRecovery :
    1;
uint32 srtsClockRecovery :
    1;
uint32 adaptiveClockRecovery :
    1;
uint32 nullErrorCorrection :
    1;
uint32 longInterleaver :
    1;
uint32 shortInterleaver :
    1;
uint32 errorCorrectionOnly :
    1;
uint32 structuredDataTransfer :
    1;
uint32 partiallyFilledCells :
    1;
    /*[...]*/
} S_Aal1ViaGateway;
typedef S_Aal1ViaGateway *PS_Aal1ViaGateway;

/*-------------------------------------------------------*/

typedef struct _VccAal5Type     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 singleBitRate;
        struct _RangeOfBitRates *rangeOfBitRates;
    };
} S_VccAal5Type;
typedef S_VccAal5Type *PS_VccAal5Type;

/*-------------------------------------------------------*/

typedef struct _AvailableBitRates     /* SEQUENCE */
{
    struct _VccAal5Type vccAal5Type;
    /*[...]*/
} S_AvailableBitRates;
typedef S_AvailableBitRates *PS_AvailableBitRates;

/*-------------------------------------------------------*/

typedef struct _RangeOfBitRates     /* SEQUENCE */
{
    uint16 lowerBitRate;
    uint16 higherBitRate;
} S_RangeOfBitRates;
typedef S_RangeOfBitRates *PS_RangeOfBitRates;

/*-------------------------------------------------------*/

typedef struct _VccAal5     /* SEQUENCE */
{
    uint16 forwardMaximumSDUSize;
    uint16 backwardMaximumSDUSize;
    /*[...]*/
} S_VccAal5;
typedef S_VccAal5 *PS_VccAal5;

/*-------------------------------------------------------*/

typedef struct _VccAal1     /* SEQUENCE */
{
uint32 nullClockRecovery :
    1;
uint32 srtsClockRecovery :
    1;
uint32 adaptiveClockRecovery :
    1;
uint32 nullErrorCorrection :
    1;
uint32 longInterleaver :
    1;
uint32 shortInterleaver :
    1;
uint32 errorCorrectionOnly :
    1;
uint32 structuredDataTransfer :
    1;
uint32 partiallyFilledCells :
    1;
    /*[...]*/
} S_VccAal1;
typedef S_VccAal1 *PS_VccAal1;

/*-------------------------------------------------------*/

typedef struct _VCCapability     /* SEQUENCE */
{
uint32 option_of_vccAal1 :
    1;
uint32 option_of_vccAal5 :
    1;
    struct _VccAal1 vccAal1;
    struct _VccAal5 vccAal5;
uint32 transportStream :
    1;
uint32 programStream :
    1;
    struct _AvailableBitRates availableBitRates;
    /*[...]*/
uint32 option_of_aal1ViaGateway :
    1;
    struct _Aal1ViaGateway aal1ViaGateway;
} S_VCCapability;
typedef S_VCCapability *PS_VCCapability;

/*-------------------------------------------------------*/

typedef struct _MobileOperationTransmitCapability     /* SEQUENCE */
{
uint32 modeChangeCapability :
    1;
uint32 h223AnnexA :
    1;
uint32 h223AnnexADoubleFlag :
    1;
uint32 h223AnnexB :
    1;
uint32 h223AnnexBwithHeader :
    1;
    /*[...]*/
} S_MobileOperationTransmitCapability;
typedef S_MobileOperationTransmitCapability *PS_MobileOperationTransmitCapability;

/*-------------------------------------------------------*/

typedef struct _H223MultiplexTableCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) basic */
        struct _Enhanced *enhanced;
    };
} S_H223MultiplexTableCapability;
typedef S_H223MultiplexTableCapability *PS_H223MultiplexTableCapability;

/*-------------------------------------------------------*/

typedef struct _Enhanced     /* SEQUENCE */
{
    uint8 maximumNestingDepth;
    uint8 maximumElementListSize;
    uint8 maximumSubElementListSize;
    /*[...]*/
} S_Enhanced;
typedef S_Enhanced *PS_Enhanced;

/*-------------------------------------------------------*/

typedef struct _H223AnnexCCapability     /* SEQUENCE */
{
uint32 videoWithAL1M :
    1;
uint32 videoWithAL2M :
    1;
uint32 videoWithAL3M :
    1;
uint32 audioWithAL1M :
    1;
uint32 audioWithAL2M :
    1;
uint32 audioWithAL3M :
    1;
uint32 dataWithAL1M :
    1;
uint32 dataWithAL2M :
    1;
uint32 dataWithAL3M :
    1;
uint32 alpduInterleaving :
    1;
    uint16 maximumAL1MPDUSize;
    uint16 maximumAL2MSDUSize;
    uint16 maximumAL3MSDUSize;
    /*[...]*/
uint32 option_of_rsCodeCapability :
    1;
uint32 rsCodeCapability :
    1;
} S_H223AnnexCCapability;
typedef S_H223AnnexCCapability *PS_H223AnnexCCapability;

/*-------------------------------------------------------*/

typedef struct _H223Capability     /* SEQUENCE */
{
uint32 transportWithI_frames :
    1;
uint32 videoWithAL1 :
    1;
uint32 videoWithAL2 :
    1;
uint32 videoWithAL3 :
    1;
uint32 audioWithAL1 :
    1;
uint32 audioWithAL2 :
    1;
uint32 audioWithAL3 :
    1;
uint32 dataWithAL1 :
    1;
uint32 dataWithAL2 :
    1;
uint32 dataWithAL3 :
    1;
    uint16 maximumAl2SDUSize;
    uint16 maximumAl3SDUSize;
    uint16 maximumDelayJitter;
    struct _H223MultiplexTableCapability h223MultiplexTableCapability;
    /*[...]*/
uint32 option_of_maxMUXPDUSizeCapability :
    1;
uint32 option_of_nsrpSupport :
    1;
uint32 option_of_mobileOperationTransmitCapability :
    1;
uint32 option_of_h223AnnexCCapability :
    1;
uint32 maxMUXPDUSizeCapability :
    1;
uint32 nsrpSupport :
    1;
    struct _MobileOperationTransmitCapability mobileOperationTransmitCapability;
    struct _H223AnnexCCapability h223AnnexCCapability;
} S_H223Capability;
typedef S_H223Capability *PS_H223Capability;

/*-------------------------------------------------------*/

typedef struct _V75Capability     /* SEQUENCE */
{
uint32 audioHeader :
    1;
    /*[...]*/
} S_V75Capability;
typedef S_V75Capability *PS_V75Capability;

/*-------------------------------------------------------*/

typedef struct _V76Capability     /* SEQUENCE */
{
uint32 suspendResumeCapabilitywAddress :
    1;
uint32 suspendResumeCapabilitywoAddress :
    1;
uint32 rejCapability :
    1;
uint32 sREJCapability :
    1;
uint32 mREJCapability :
    1;
uint32 crc8bitCapability :
    1;
uint32 crc16bitCapability :
    1;
uint32 crc32bitCapability :
    1;
uint32 uihCapability :
    1;
    uint16 numOfDLCS;
uint32 twoOctetAddressFieldCapability :
    1;
uint32 loopBackTestCapability :
    1;
    uint16 n401Capability;
    uint8 maxWindowSizeCapability;
    struct _V75Capability v75Capability;
    /*[...]*/
} S_V76Capability;
typedef S_V76Capability *PS_V76Capability;

/*-------------------------------------------------------*/

typedef struct _McCapability     /* SEQUENCE */
{
uint32 centralizedConferenceMC :
    1;
uint32 decentralizedConferenceMC :
    1;
    /*[...]*/
} S_McCapability;
typedef S_McCapability *PS_McCapability;

/*-------------------------------------------------------*/

typedef struct _MediaPacketizationCapability     /* SEQUENCE */
{
uint32 h261aVideoPacketization :
    1;
    /*[...]*/
uint32 option_of_rtpPayloadType :
    1;
    uint16 size_of_rtpPayloadType;
    struct _RTPPayloadType *rtpPayloadType;
} S_MediaPacketizationCapability;
typedef S_MediaPacketizationCapability *PS_MediaPacketizationCapability;

/*-------------------------------------------------------*/

typedef struct _QOSMode     /* CHOICE */
{
    uint16 index;
    /* (NULL) guaranteedQOS */
    /* (NULL) controlledLoad */
    /*[...]*/
} S_QOSMode;
typedef S_QOSMode *PS_QOSMode;

/*-------------------------------------------------------*/

typedef struct _RSVPParameters     /* SEQUENCE */
{
uint32 option_of_qosMode :
    1;
uint32 option_of_tokenRate :
    1;
uint32 option_of_bucketSize :
    1;
uint32 option_of_peakRate :
    1;
uint32 option_of_minPoliced :
    1;
uint32 option_of_maxPktSize :
    1;
    struct _QOSMode qosMode;
    uint32 tokenRate;
    uint32 bucketSize;
    uint32 peakRate;
    uint32 minPoliced;
    uint32 maxPktSize;
    /*[...]*/
} S_RSVPParameters;
typedef S_RSVPParameters *PS_RSVPParameters;

/*-------------------------------------------------------*/

typedef struct _ATMParameters     /* SEQUENCE */
{
    uint16 maxNTUSize;
uint32 atmUBR :
    1;
uint32 atmrtVBR :
    1;
uint32 atmnrtVBR :
    1;
uint32 atmABR :
    1;
uint32 atmCBR :
    1;
    /*[...]*/
} S_ATMParameters;
typedef S_ATMParameters *PS_ATMParameters;

/*-------------------------------------------------------*/

typedef struct _QOSCapability     /* SEQUENCE */
{
uint32 option_of_nonStandardData :
    1;
uint32 option_of_rsvpParameters :
    1;
uint32 option_of_atmParameters :
    1;
    struct _NonStandardParameter nonStandardData;
    struct _RSVPParameters rsvpParameters;
    struct _ATMParameters atmParameters;
    /*[...]*/
} S_QOSCapability;
typedef S_QOSCapability *PS_QOSCapability;

/*-------------------------------------------------------*/

typedef struct _MediaTransportType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) ip_UDP */
        /* (NULL) ip_TCP */
        /* (NULL) atm_AAL5_UNIDIR */
        /* (NULL) atm_AAL5_BIDIR */
        /*[...]*/
        struct _Atm_AAL5_compressed *atm_AAL5_compressed;
    };
} S_MediaTransportType;
typedef S_MediaTransportType *PS_MediaTransportType;

/*-------------------------------------------------------*/

typedef struct _Atm_AAL5_compressed     /* SEQUENCE */
{
uint32 variable_delta :
    1;
    /*[...]*/
} S_Atm_AAL5_compressed;
typedef S_Atm_AAL5_compressed *PS_Atm_AAL5_compressed;

/*-------------------------------------------------------*/

typedef struct _MediaChannelCapability     /* SEQUENCE */
{
uint32 option_of_mediaTransport :
    1;
    struct _MediaTransportType mediaTransport;
    /*[...]*/
} S_MediaChannelCapability;
typedef S_MediaChannelCapability *PS_MediaChannelCapability;

/*-------------------------------------------------------*/

typedef struct _TransportCapability     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
uint32 option_of_qOSCapabilities :
    1;
uint32 option_of_mediaChannelCapabilities :
    1;
    struct _NonStandardParameter nonStandard;
    uint16 size_of_qOSCapabilities;
    struct _QOSCapability *qOSCapabilities;
    uint16 size_of_mediaChannelCapabilities;
    struct _MediaChannelCapability *mediaChannelCapabilities;
    /*[...]*/
} S_TransportCapability;
typedef S_TransportCapability *PS_TransportCapability;

/*-------------------------------------------------------*/

typedef struct _RedundancyEncodingMethod     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) rtpAudioRedundancyEncoding */
        /*[...]*/
        struct _RTPH263VideoRedundancyEncoding *rtpH263VideoRedundancyEncoding;
    };
} S_RedundancyEncodingMethod;
typedef S_RedundancyEncodingMethod *PS_RedundancyEncodingMethod;

/*-------------------------------------------------------*/

typedef struct _RedundancyEncodingCapability     /* SEQUENCE */
{
uint32 option_of_secondaryEncoding :
    1;
    struct _RedundancyEncodingMethod redundancyEncodingMethod;
    uint16 primaryEncoding;
    uint16 size_of_secondaryEncoding;
    uint32* secondaryEncoding;
    /*[...]*/
} S_RedundancyEncodingCapability;
typedef S_RedundancyEncodingCapability *PS_RedundancyEncodingCapability;

/*-------------------------------------------------------*/

typedef struct _FrameToThreadMapping     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) roundrobin */
        struct _RTPH263VideoRedundancyFrameMapping *custom;
        /*[...]*/
    };
    uint16 size;
} S_FrameToThreadMapping;
typedef S_FrameToThreadMapping *PS_FrameToThreadMapping;

/*-------------------------------------------------------*/

typedef struct _RTPH263VideoRedundancyEncoding     /* SEQUENCE */
{
uint32 option_of_containedThreads :
    1;
    uint8 numberOfThreads;
    uint16 framesBetweenSyncPoints;
    struct _FrameToThreadMapping frameToThreadMapping;
    uint16 size_of_containedThreads;
    uint32* containedThreads;
    /*[...]*/
} S_RTPH263VideoRedundancyEncoding;
typedef S_RTPH263VideoRedundancyEncoding *PS_RTPH263VideoRedundancyEncoding;

/*-------------------------------------------------------*/

typedef struct _RTPH263VideoRedundancyFrameMapping     /* SEQUENCE */
{
    uint8 threadNumber;
    uint16 size_of_frameSequence;
    uint32* frameSequence;
    /*[...]*/
} S_RTPH263VideoRedundancyFrameMapping;
typedef S_RTPH263VideoRedundancyFrameMapping *PS_RTPH263VideoRedundancyFrameMapping;

/*-------------------------------------------------------*/

typedef struct _MultipointCapability     /* SEQUENCE */
{
uint32 multicastCapability :
    1;
uint32 multiUniCastConference :
    1;
    uint16 size_of_mediaDistributionCapability;
    struct _MediaDistributionCapability *mediaDistributionCapability;
    /*[...]*/
} S_MultipointCapability;
typedef S_MultipointCapability *PS_MultipointCapability;

/*-------------------------------------------------------*/

typedef struct _H2250Capability     /* SEQUENCE */
{
    uint16 maximumAudioDelayJitter;
    struct _MultipointCapability receiveMultipointCapability;
    struct _MultipointCapability transmitMultipointCapability;
    struct _MultipointCapability receiveAndTransmitMultipointCapability;
    struct _McCapability mcCapability;
uint32 rtcpVideoControlCapability :
    1;
    struct _MediaPacketizationCapability mediaPacketizationCapability;
    /*[...]*/
uint32 option_of_transportCapability :
    1;
uint32 option_of_redundancyEncodingCapability :
    1;
uint32 option_of_logicalChannelSwitchingCapability :
    1;
uint32 option_of_t120DynamicPortCapability :
    1;
    struct _TransportCapability transportCapability;
    uint16 size_of_redundancyEncodingCapability;
    struct _RedundancyEncodingCapability *redundancyEncodingCapability;
uint32 logicalChannelSwitchingCapability :
    1;
uint32 t120DynamicPortCapability :
    1;
} S_H2250Capability;
typedef S_H2250Capability *PS_H2250Capability;

/*-------------------------------------------------------*/

typedef struct _MediaDistributionCapability     /* SEQUENCE */
{
uint32 option_of_centralizedData :
    1;
uint32 option_of_distributedData :
    1;
uint32 centralizedControl :
    1;
uint32 distributedControl :
    1;
uint32 centralizedAudio :
    1;
uint32 distributedAudio :
    1;
uint32 centralizedVideo :
    1;
uint32 distributedVideo :
    1;
    uint16 size_of_centralizedData;
    struct _DataApplicationCapability *centralizedData;
    uint16 size_of_distributedData;
    struct _DataApplicationCapability *distributedData;
    /*[...]*/
} S_MediaDistributionCapability;
typedef S_MediaDistributionCapability *PS_MediaDistributionCapability;

/*-------------------------------------------------------*/

typedef struct _VideoCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _H261VideoCapability *h261VideoCapability;
        struct _H262VideoCapability *h262VideoCapability;
        struct _H263VideoCapability *h263VideoCapability;
        struct _IS11172VideoCapability *is11172VideoCapability;
        /*[...]*/
        struct _GenericCapability *genericVideoCapability;
    };
} S_VideoCapability;
typedef S_VideoCapability *PS_VideoCapability;

/*-------------------------------------------------------*/

typedef struct _H261VideoCapability     /* SEQUENCE */
{
uint32 option_of_qcifMPI :
    1;
uint32 option_of_cifMPI :
    1;
    uint8 qcifMPI;
    uint8 cifMPI;
uint32 temporalSpatialTradeOffCapability :
    1;
    uint16 maxBitRate;
uint32 stillImageTransmission :
    1;
    /*[...]*/
} S_H261VideoCapability;
typedef S_H261VideoCapability *PS_H261VideoCapability;

/*-------------------------------------------------------*/

typedef struct _H262VideoCapability     /* SEQUENCE */
{
uint32 option_of_videoBitRate :
    1;
uint32 option_of_vbvBufferSize :
    1;
uint32 option_of_samplesPerLine :
    1;
uint32 option_of_linesPerFrame :
    1;
uint32 option_of_framesPerSecond :
    1;
uint32 option_of_luminanceSampleRate :
    1;
uint32 profileAndLevel_SPatML :
    1;
uint32 profileAndLevel_MPatLL :
    1;
uint32 profileAndLevel_MPatML :
    1;
uint32 profileAndLevel_MPatH_14 :
    1;
uint32 profileAndLevel_MPatHL :
    1;
uint32 profileAndLevel_SNRatLL :
    1;
uint32 profileAndLevel_SNRatML :
    1;
uint32 profileAndLevel_SpatialatH_14 :
    1;
uint32 profileAndLevel_HPatML :
    1;
uint32 profileAndLevel_HPatH_14 :
    1;
uint32 profileAndLevel_HPatHL :
    1;
    uint32 videoBitRate;
    uint32 vbvBufferSize;
    uint16 samplesPerLine;
    uint16 linesPerFrame;
    uint8 framesPerSecond;
    uint32 luminanceSampleRate;
    /*[...]*/
} S_H262VideoCapability;
typedef S_H262VideoCapability *PS_H262VideoCapability;

/*-------------------------------------------------------*/

typedef struct _EnhancementLayerInfo     /* SEQUENCE */
{
uint32 option_of_snrEnhancement :
    1;
uint32 option_of_spatialEnhancement :
    1;
uint32 option_of_bPictureEnhancement :
    1;
uint32 baseBitRateConstrained :
    1;
    uint16 size_of_snrEnhancement;
    struct _EnhancementOptions *snrEnhancement;
    uint16 size_of_spatialEnhancement;
    struct _EnhancementOptions *spatialEnhancement;
    uint16 size_of_bPictureEnhancement;
    struct _BEnhancementParameters *bPictureEnhancement;
    /*[...]*/
} S_EnhancementLayerInfo;
typedef S_EnhancementLayerInfo *PS_EnhancementLayerInfo;

/*-------------------------------------------------------*/

typedef struct _TransparencyParameters     /* SEQUENCE */
{
    uint16 presentationOrder;
    int32 offset_x;
    int32 offset_y;
    uint8 scale_x;
    uint8 scale_y;
    /*[...]*/
} S_TransparencyParameters;
typedef S_TransparencyParameters *PS_TransparencyParameters;

/*-------------------------------------------------------*/

typedef struct _VideoBackChannelSend     /* CHOICE */
{
    uint16 index;
    /* (NULL) none */
    /* (NULL) ackMessageOnly */
    /* (NULL) nackMessageOnly */
    /* (NULL) ackOrNackMessageOnly */
    /* (NULL) ackAndNackMessage */
    /*[...]*/
} S_VideoBackChannelSend;
typedef S_VideoBackChannelSend *PS_VideoBackChannelSend;

/*-------------------------------------------------------*/

typedef struct _AdditionalPictureMemory     /* SEQUENCE */
{
uint32 option_of_sqcifAdditionalPictureMemory :
    1;
uint32 option_of_qcifAdditionalPictureMemory :
    1;
uint32 option_of_cifAdditionalPictureMemory :
    1;
uint32 option_of_cif4AdditionalPictureMemory :
    1;
uint32 option_of_cif16AdditionalPictureMemory :
    1;
uint32 option_of_bigCpfAdditionalPictureMemory :
    1;
    uint16 sqcifAdditionalPictureMemory;
    uint16 qcifAdditionalPictureMemory;
    uint16 cifAdditionalPictureMemory;
    uint16 cif4AdditionalPictureMemory;
    uint16 cif16AdditionalPictureMemory;
    uint16 bigCpfAdditionalPictureMemory;
    /*[...]*/
} S_AdditionalPictureMemory;
typedef S_AdditionalPictureMemory *PS_AdditionalPictureMemory;

/*-------------------------------------------------------*/

typedef struct _RefPictureSelection     /* SEQUENCE */
{
uint32 option_of_additionalPictureMemory :
    1;
    struct _AdditionalPictureMemory additionalPictureMemory;
uint32 videoMux :
    1;
    struct _VideoBackChannelSend videoBackChannelSend;
    /*[...]*/
} S_RefPictureSelection;
typedef S_RefPictureSelection *PS_RefPictureSelection;

/*-------------------------------------------------------*/

typedef struct _H263Options     /* SEQUENCE */
{
uint32 option_of_transparencyParameters :
    1;
uint32 option_of_refPictureSelection :
    1;
uint32 option_of_customPictureClockFrequency :
    1;
uint32 option_of_customPictureFormat :
    1;
uint32 option_of_modeCombos :
    1;
uint32 advancedIntraCodingMode :
    1;
uint32 deblockingFilterMode :
    1;
uint32 improvedPBFramesMode :
    1;
uint32 unlimitedMotionVectors :
    1;
uint32 fullPictureFreeze :
    1;
uint32 partialPictureFreezeAndRelease :
    1;
uint32 resizingPartPicFreezeAndRelease :
    1;
uint32 fullPictureSnapshot :
    1;
uint32 partialPictureSnapshot :
    1;
uint32 videoSegmentTagging :
    1;
uint32 progressiveRefinement :
    1;
uint32 dynamicPictureResizingByFour :
    1;
uint32 dynamicPictureResizingSixteenthPel :
    1;
uint32 dynamicWarpingHalfPel :
    1;
uint32 dynamicWarpingSixteenthPel :
    1;
uint32 independentSegmentDecoding :
    1;
uint32 slicesInOrder_NonRect :
    1;
uint32 slicesInOrder_Rect :
    1;
uint32 slicesNoOrder_NonRect :
    1;
uint32 slicesNoOrder_Rect :
    1;
uint32 alternateInterVLCMode :
    1;
uint32 modifiedQuantizationMode :
    1;
uint32 reducedResolutionUpdate :
    1;
    struct _TransparencyParameters transparencyParameters;
uint32 separateVideoBackChannel :
    1;
    struct _RefPictureSelection refPictureSelection;
    uint16 size_of_customPictureClockFrequency;
    struct _CustomPictureClockFrequency *customPictureClockFrequency;
    uint16 size_of_customPictureFormat;
    struct _CustomPictureFormat *customPictureFormat;
    uint16 size_of_modeCombos;
    struct _H263VideoModeCombos *modeCombos;
    /*[...]*/
} S_H263Options;
typedef S_H263Options *PS_H263Options;

/*-------------------------------------------------------*/

typedef struct _H263VideoCapability     /* SEQUENCE */
{
uint32 option_of_sqcifMPI :
    1;
uint32 option_of_qcifMPI :
    1;
uint32 option_of_cifMPI :
    1;
uint32 option_of_cif4MPI :
    1;
uint32 option_of_cif16MPI :
    1;
uint32 option_of_hrd_B :
    1;
uint32 option_of_bppMaxKb :
    1;
    uint8 sqcifMPI;
    uint8 qcifMPI;
    uint8 cifMPI;
    uint8 cif4MPI;
    uint8 cif16MPI;
    uint32 maxBitRate;
uint32 unrestrictedVector :
    1;
uint32 arithmeticCoding :
    1;
uint32 advancedPrediction :
    1;
uint32 pbFrames :
    1;
uint32 temporalSpatialTradeOffCapability :
    1;
    uint32 hrd_B;
    uint16 bppMaxKb;
    /*[...]*/
uint32 option_of_slowSqcifMPI :
    1;
uint32 option_of_slowQcifMPI :
    1;
uint32 option_of_slowCifMPI :
    1;
uint32 option_of_slowCif4MPI :
    1;
uint32 option_of_slowCif16MPI :
    1;
uint32 option_of_errorCompensation :
    1;
uint32 option_of_enhancementLayerInfo :
    1;
uint32 option_of_h263Options :
    1;
    uint16 slowSqcifMPI;
    uint16 slowQcifMPI;
    uint16 slowCifMPI;
    uint16 slowCif4MPI;
    uint16 slowCif16MPI;
uint32 errorCompensation :
    1;
    struct _EnhancementLayerInfo enhancementLayerInfo;
    struct _H263Options h263Options;
} S_H263VideoCapability;
typedef S_H263VideoCapability *PS_H263VideoCapability;

/*-------------------------------------------------------*/

typedef struct _EnhancementOptions     /* SEQUENCE */
{
uint32 option_of_sqcifMPI :
    1;
uint32 option_of_qcifMPI :
    1;
uint32 option_of_cifMPI :
    1;
uint32 option_of_cif4MPI :
    1;
uint32 option_of_cif16MPI :
    1;
uint32 option_of_slowSqcifMPI :
    1;
uint32 option_of_slowQcifMPI :
    1;
uint32 option_of_slowCifMPI :
    1;
uint32 option_of_slowCif4MPI :
    1;
uint32 option_of_slowCif16MPI :
    1;
uint32 option_of_h263Options :
    1;
    uint8 sqcifMPI;
    uint8 qcifMPI;
    uint8 cifMPI;
    uint8 cif4MPI;
    uint8 cif16MPI;
    uint32 maxBitRate;
uint32 unrestrictedVector :
    1;
uint32 arithmeticCoding :
    1;
uint32 temporalSpatialTradeOffCapability :
    1;
    uint16 slowSqcifMPI;
    uint16 slowQcifMPI;
    uint16 slowCifMPI;
    uint16 slowCif4MPI;
    uint16 slowCif16MPI;
uint32 errorCompensation :
    1;
    struct _H263Options h263Options;
    /*[...]*/
} S_EnhancementOptions;
typedef S_EnhancementOptions *PS_EnhancementOptions;

/*-------------------------------------------------------*/

typedef struct _BEnhancementParameters     /* SEQUENCE */
{
    struct _EnhancementOptions enhancementOptions;
    uint8 numberOfBPictures;
    /*[...]*/
} S_BEnhancementParameters;
typedef S_BEnhancementParameters *PS_BEnhancementParameters;

/*-------------------------------------------------------*/

typedef struct _CustomPictureClockFrequency     /* SEQUENCE */
{
uint32 option_of_sqcifMPI :
    1;
uint32 option_of_qcifMPI :
    1;
uint32 option_of_cifMPI :
    1;
uint32 option_of_cif4MPI :
    1;
uint32 option_of_cif16MPI :
    1;
    uint16 clockConversionCode;
    uint8 clockDivisor;
    uint16 sqcifMPI;
    uint16 qcifMPI;
    uint16 cifMPI;
    uint16 cif4MPI;
    uint16 cif16MPI;
    /*[...]*/
} S_CustomPictureClockFrequency;
typedef S_CustomPictureClockFrequency *PS_CustomPictureClockFrequency;

/*-------------------------------------------------------*/

typedef struct _PixelAspectInformation     /* CHOICE */
{
    uint16 index;
    union
    {
        uint8 anyPixelAspectRatio;
        uint32* pixelAspectCode;
        struct _ExtendedPARItem *extendedPAR;
        /*[...]*/
    };
    uint16 size;
} S_PixelAspectInformation;
typedef S_PixelAspectInformation *PS_PixelAspectInformation;

/*-------------------------------------------------------*/

typedef struct _ExtendedPARItem     /* SEQUENCE */
{
    uint8 width;
    uint8 height;
    /*[...]*/
} S_ExtendedPARItem;
typedef S_ExtendedPARItem *PS_ExtendedPARItem;

/*-------------------------------------------------------*/

typedef struct _MPI     /* SEQUENCE */
{
uint32 option_of_standardMPI :
    1;
uint32 option_of_customPCF :
    1;
    uint8 standardMPI;
    uint16 size_of_customPCF;
    struct _CustomPCFItem *customPCF;
    /*[...]*/
} S_MPI;
typedef S_MPI *PS_MPI;

/*-------------------------------------------------------*/

typedef struct _CustomPictureFormat     /* SEQUENCE */
{
    uint16 maxCustomPictureWidth;
    uint16 maxCustomPictureHeight;
    uint16 minCustomPictureWidth;
    uint16 minCustomPictureHeight;
    struct _MPI mPI;
    struct _PixelAspectInformation pixelAspectInformation;
    /*[...]*/
} S_CustomPictureFormat;
typedef S_CustomPictureFormat *PS_CustomPictureFormat;

/*-------------------------------------------------------*/

typedef struct _CustomPCFItem     /* SEQUENCE */
{
    uint16 clockConversionCode;
    uint8 clockDivisor;
    uint16 customMPI;
    /*[...]*/
} S_CustomPCFItem;
typedef S_CustomPCFItem *PS_CustomPCFItem;

/*-------------------------------------------------------*/

typedef struct _H263ModeComboFlags     /* SEQUENCE */
{
uint32 unrestrictedVector :
    1;
uint32 arithmeticCoding :
    1;
uint32 advancedPrediction :
    1;
uint32 pbFrames :
    1;
uint32 advancedIntraCodingMode :
    1;
uint32 deblockingFilterMode :
    1;
uint32 unlimitedMotionVectors :
    1;
uint32 slicesInOrder_NonRect :
    1;
uint32 slicesInOrder_Rect :
    1;
uint32 slicesNoOrder_NonRect :
    1;
uint32 slicesNoOrder_Rect :
    1;
uint32 improvedPBFramesMode :
    1;
uint32 referencePicSelect :
    1;
uint32 dynamicPictureResizingByFour :
    1;
uint32 dynamicPictureResizingSixteenthPel :
    1;
uint32 dynamicWarpingHalfPel :
    1;
uint32 dynamicWarpingSixteenthPel :
    1;
uint32 reducedResolutionUpdate :
    1;
uint32 independentSegmentDecoding :
    1;
uint32 alternateInterVLCMode :
    1;
uint32 modifiedQuantizationMode :
    1;
    /*[...]*/
} S_H263ModeComboFlags;
typedef S_H263ModeComboFlags *PS_H263ModeComboFlags;

/*-------------------------------------------------------*/

typedef struct _H263VideoModeCombos     /* SEQUENCE */
{
    struct _H263ModeComboFlags h263VideoUncoupledModes;
    uint16 size_of_h263VideoCoupledModes;
    struct _H263ModeComboFlags *h263VideoCoupledModes;
    /*[...]*/
} S_H263VideoModeCombos;
typedef S_H263VideoModeCombos *PS_H263VideoModeCombos;

/*-------------------------------------------------------*/

typedef struct _IS11172VideoCapability     /* SEQUENCE */
{
uint32 option_of_videoBitRate :
    1;
uint32 option_of_vbvBufferSize :
    1;
uint32 option_of_samplesPerLine :
    1;
uint32 option_of_linesPerFrame :
    1;
uint32 option_of_pictureRate :
    1;
uint32 option_of_luminanceSampleRate :
    1;
uint32 constrainedBitstream :
    1;
    uint32 videoBitRate;
    uint32 vbvBufferSize;
    uint16 samplesPerLine;
    uint16 linesPerFrame;
    uint8 pictureRate;
    uint32 luminanceSampleRate;
    /*[...]*/
} S_IS11172VideoCapability;
typedef S_IS11172VideoCapability *PS_IS11172VideoCapability;

/*-------------------------------------------------------*/

typedef struct _AudioCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        uint16 g711Alaw64k;
        uint16 g711Alaw56k;
        uint16 g711Ulaw64k;
        uint16 g711Ulaw56k;
        uint16 g722_64k;
        uint16 g722_56k;
        uint16 g722_48k;
        struct _G7231 *g7231;
        uint16 g728;
        uint16 g729;
        uint16 g729AnnexA;
        struct _IS11172AudioCapability *is11172AudioCapability;
        struct _IS13818AudioCapability *is13818AudioCapability;
        /*[...]*/
        uint16 g729wAnnexB;
        uint16 g729AnnexAwAnnexB;
        struct _G7231AnnexCCapability *g7231AnnexCCapability;
        struct _GSMAudioCapability *gsmFullRate;
        struct _GSMAudioCapability *gsmHalfRate;
        struct _GSMAudioCapability *gsmEnhancedFullRate;
        struct _GenericCapability *genericAudioCapability;
        struct _G729Extensions *g729Extensions;
    };
} S_AudioCapability;
typedef S_AudioCapability *PS_AudioCapability;

/*-------------------------------------------------------*/

typedef struct _G7231     /* SEQUENCE */
{
    uint16 maxAl_sduAudioFrames;
uint32 silenceSuppression :
    1;
} S_G7231;
typedef S_G7231 *PS_G7231;

/*-------------------------------------------------------*/

typedef struct _G729Extensions     /* SEQUENCE */
{
uint32 option_of_audioUnit :
    1;
    uint16 audioUnit;
uint32 annexA :
    1;
uint32 annexB :
    1;
uint32 annexD :
    1;
uint32 annexE :
    1;
uint32 annexF :
    1;
uint32 annexG :
    1;
uint32 annexH :
    1;
    /*[...]*/
} S_G729Extensions;
typedef S_G729Extensions *PS_G729Extensions;

/*-------------------------------------------------------*/

typedef struct _G723AnnexCAudioMode     /* SEQUENCE */
{
    uint8 highRateMode0;
    uint8 highRateMode1;
    uint8 lowRateMode0;
    uint8 lowRateMode1;
    uint8 sidMode0;
    uint8 sidMode1;
    /*[...]*/
} S_G723AnnexCAudioMode;
typedef S_G723AnnexCAudioMode *PS_G723AnnexCAudioMode;

/*-------------------------------------------------------*/

typedef struct _G7231AnnexCCapability     /* SEQUENCE */
{
uint32 option_of_g723AnnexCAudioMode :
    1;
    uint16 maxAl_sduAudioFrames;
uint32 silenceSuppression :
    1;
    struct _G723AnnexCAudioMode g723AnnexCAudioMode;
    /*[...]*/
} S_G7231AnnexCCapability;
typedef S_G7231AnnexCCapability *PS_G7231AnnexCCapability;

/*-------------------------------------------------------*/

typedef struct _IS11172AudioCapability     /* SEQUENCE */
{
uint32 audioLayer1 :
    1;
uint32 audioLayer2 :
    1;
uint32 audioLayer3 :
    1;
uint32 audioSampling32k :
    1;
uint32 audioSampling44k1 :
    1;
uint32 audioSampling48k :
    1;
uint32 singleChannel :
    1;
uint32 twoChannels :
    1;
    uint16 bitRate;
    /*[...]*/
} S_IS11172AudioCapability;
typedef S_IS11172AudioCapability *PS_IS11172AudioCapability;

/*-------------------------------------------------------*/

typedef struct _IS13818AudioCapability     /* SEQUENCE */
{
uint32 audioLayer1 :
    1;
uint32 audioLayer2 :
    1;
uint32 audioLayer3 :
    1;
uint32 audioSampling16k :
    1;
uint32 audioSampling22k05 :
    1;
uint32 audioSampling24k :
    1;
uint32 audioSampling32k :
    1;
uint32 audioSampling44k1 :
    1;
uint32 audioSampling48k :
    1;
uint32 singleChannel :
    1;
uint32 twoChannels :
    1;
uint32 threeChannels2_1 :
    1;
uint32 threeChannels3_0 :
    1;
uint32 fourChannels2_0_2_0 :
    1;
uint32 fourChannels2_2 :
    1;
uint32 fourChannels3_1 :
    1;
uint32 fiveChannels3_0_2_0 :
    1;
uint32 fiveChannels3_2 :
    1;
uint32 lowFrequencyEnhancement :
    1;
uint32 multilingual :
    1;
    uint16 bitRate;
    /*[...]*/
} S_IS13818AudioCapability;
typedef S_IS13818AudioCapability *PS_IS13818AudioCapability;

/*-------------------------------------------------------*/

typedef struct _GSMAudioCapability     /* SEQUENCE */
{
    uint16 audioUnitSize;
uint32 comfortNoise :
    1;
uint32 scrambled :
    1;
    /*[...]*/
} S_GSMAudioCapability;
typedef S_GSMAudioCapability *PS_GSMAudioCapability;

/*-------------------------------------------------------*/

typedef struct _Application     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _DataProtocolCapability *t120;
        struct _DataProtocolCapability *dsm_cc;
        struct _DataProtocolCapability *userData;
        struct _T84 *t84;
        struct _DataProtocolCapability *t434;
        struct _DataProtocolCapability *h224;
        struct _Nlpid *nlpid;
        /* (NULL) dsvdControl */
        struct _DataProtocolCapability *h222DataPartitioning;
        /*[...]*/
        struct _DataProtocolCapability *t30fax;
        struct _DataProtocolCapability *t140;
        struct _T38fax *t38fax;
        struct _GenericCapability *genericDataCapability;
    };
} S_Application;
typedef S_Application *PS_Application;

/*-------------------------------------------------------*/

typedef struct _DataApplicationCapability     /* SEQUENCE */
{
    struct _Application application;
    uint32 maxBitRate;
    /*[...]*/
} S_DataApplicationCapability;
typedef S_DataApplicationCapability *PS_DataApplicationCapability;

/*-------------------------------------------------------*/

typedef struct _DataProtocolCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) v14buffered */
        /* (NULL) v42lapm */
        /* (NULL) hdlcFrameTunnelling */
        /* (NULL) h310SeparateVCStack */
        /* (NULL) h310SingleVCStack */
        /* (NULL) transparent */
        /*[...]*/
        /* (NULL) segmentationAndReassembly */
        /* (NULL) hdlcFrameTunnelingwSAR */
        /* (NULL) v120 */
        /* (NULL) separateLANStack */
        struct _V76wCompression *v76wCompression;
        /* (NULL) tcp */
        /* (NULL) udp */
    };
} S_DataProtocolCapability;
typedef S_DataProtocolCapability *PS_DataProtocolCapability;

/*-------------------------------------------------------*/

typedef struct _Nlpid     /* SEQUENCE */
{
    struct _DataProtocolCapability nlpidProtocol;
    S_OCTETSTRING nlpidData;
} S_Nlpid;
typedef S_Nlpid *PS_Nlpid;

/*-------------------------------------------------------*/

typedef struct _V76wCompression     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _CompressionType *transmitCompression;
        struct _CompressionType *receiveCompression;
        struct _CompressionType *transmitAndReceiveCompression;
        /*[...]*/
    };
} S_V76wCompression;
typedef S_V76wCompression *PS_V76wCompression;

/*-------------------------------------------------------*/

typedef struct _CompressionType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _V42bis *v42bis;
        /*[...]*/
    };
} S_CompressionType;
typedef S_CompressionType *PS_CompressionType;

/*-------------------------------------------------------*/

typedef struct _V42bis     /* SEQUENCE */
{
    uint32 numberOfCodewords;
    uint16 maximumStringLength;
    /*[...]*/
} S_V42bis;
typedef S_V42bis *PS_V42bis;

/*-------------------------------------------------------*/

typedef struct _T84Profile     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) t84Unrestricted */
        struct _T84Restricted *t84Restricted;
    };
} S_T84Profile;
typedef S_T84Profile *PS_T84Profile;

/*-------------------------------------------------------*/

typedef struct _T84     /* SEQUENCE */
{
    struct _DataProtocolCapability t84Protocol;
    struct _T84Profile t84Profile;
} S_T84;
typedef S_T84 *PS_T84;

/*-------------------------------------------------------*/

typedef struct _T84Restricted     /* SEQUENCE */
{
uint32 qcif :
    1;
uint32 cif :
    1;
uint32 ccir601Seq :
    1;
uint32 ccir601Prog :
    1;
uint32 hdtvSeq :
    1;
uint32 hdtvProg :
    1;
uint32 g3FacsMH200x100 :
    1;
uint32 g3FacsMH200x200 :
    1;
uint32 g4FacsMMR200x100 :
    1;
uint32 g4FacsMMR200x200 :
    1;
uint32 jbig200x200Seq :
    1;
uint32 jbig200x200Prog :
    1;
uint32 jbig300x300Seq :
    1;
uint32 jbig300x300Prog :
    1;
uint32 digPhotoLow :
    1;
uint32 digPhotoMedSeq :
    1;
uint32 digPhotoMedProg :
    1;
uint32 digPhotoHighSeq :
    1;
uint32 digPhotoHighProg :
    1;
    /*[...]*/
} S_T84Restricted;
typedef S_T84Restricted *PS_T84Restricted;

/*-------------------------------------------------------*/

typedef struct _T38FaxRateManagement     /* CHOICE */
{
    uint16 index;
    /* (NULL) localTCF */
    /* (NULL) transferredTCF */
    /*[...]*/
} S_T38FaxRateManagement;
typedef S_T38FaxRateManagement *PS_T38FaxRateManagement;

/*-------------------------------------------------------*/

typedef struct _T38FaxUdpEC     /* CHOICE */
{
    uint16 index;
    /* (NULL) t38UDPFEC */
    /* (NULL) t38UDPRedundancy */
    /*[...]*/
} S_T38FaxUdpEC;
typedef S_T38FaxUdpEC *PS_T38FaxUdpEC;

/*-------------------------------------------------------*/

typedef struct _T38FaxUdpOptions     /* SEQUENCE */
{
uint32 option_of_t38FaxMaxBuffer :
    1;
uint32 option_of_t38FaxMaxDatagram :
    1;
    uint32 t38FaxMaxBuffer;
    uint32 t38FaxMaxDatagram;
    struct _T38FaxUdpEC t38FaxUdpEC;
} S_T38FaxUdpOptions;
typedef S_T38FaxUdpOptions *PS_T38FaxUdpOptions;

/*-------------------------------------------------------*/

typedef struct _T38FaxProfile     /* SEQUENCE */
{
uint32 fillBitRemoval :
    1;
uint32 transcodingJBIG :
    1;
uint32 transcodingMMR :
    1;
    /*[...]*/
uint32 option_of_version :
    1;
uint32 option_of_t38FaxRateManagement :
    1;
uint32 option_of_t38FaxUdpOptions :
    1;
    uint8 version;
    struct _T38FaxRateManagement t38FaxRateManagement;
    struct _T38FaxUdpOptions t38FaxUdpOptions;
} S_T38FaxProfile;
typedef S_T38FaxProfile *PS_T38FaxProfile;

/*-------------------------------------------------------*/

typedef struct _T38fax     /* SEQUENCE */
{
    struct _DataProtocolCapability t38FaxProtocol;
    struct _T38FaxProfile t38FaxProfile;
} S_T38fax;
typedef S_T38fax *PS_T38fax;

/*-------------------------------------------------------*/

typedef struct _EncryptionCapability     /* SEQUENCE OF */
{
    struct _MediaEncryptionAlgorithm *item;
    uint16 size;
} S_EncryptionCapability;
typedef S_EncryptionCapability *PS_EncryptionCapability;

/*-------------------------------------------------------*/

typedef struct _MediaEncryptionAlgorithm     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        PS_OBJECTIDENT algorithm;
        /*[...]*/
    };
} S_MediaEncryptionAlgorithm;
typedef S_MediaEncryptionAlgorithm *PS_MediaEncryptionAlgorithm;

/*-------------------------------------------------------*/

typedef struct _AuthenticationCapability     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
    struct _NonStandardParameter nonStandard;
    /*[...]*/
} S_AuthenticationCapability;
typedef S_AuthenticationCapability *PS_AuthenticationCapability;

/*-------------------------------------------------------*/

typedef struct _IntegrityCapability     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
    struct _NonStandardParameter nonStandard;
    /*[...]*/
} S_IntegrityCapability;
typedef S_IntegrityCapability *PS_IntegrityCapability;

/*-------------------------------------------------------*/

typedef struct _EncryptionAuthenticationAndIntegrity     /* SEQUENCE */
{
uint32 option_of_encryptionCapability :
    1;
uint32 option_of_authenticationCapability :
    1;
uint32 option_of_integrityCapability :
    1;
    struct _EncryptionCapability encryptionCapability;
    struct _AuthenticationCapability authenticationCapability;
    struct _IntegrityCapability integrityCapability;
    /*[...]*/
} S_EncryptionAuthenticationAndIntegrity;
typedef S_EncryptionAuthenticationAndIntegrity *PS_EncryptionAuthenticationAndIntegrity;

/*-------------------------------------------------------*/

typedef struct _H235SecurityCapability     /* SEQUENCE */
{
    struct _EncryptionAuthenticationAndIntegrity encryptionAuthenticationAndIntegrity;
    uint16 mediaCapability;
    /*[...]*/
} S_H235SecurityCapability;
typedef S_H235SecurityCapability *PS_H235SecurityCapability;

/*-------------------------------------------------------*/

typedef struct _UserInputCapability     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) basicString */
        /* (NULL) iA5String */
        /* (NULL) generalString */
        /* (NULL) dtmf */
        /* (NULL) hookflash */
        /*[...]*/
    };
    uint16 size;
} S_UserInputCapability;
typedef S_UserInputCapability *PS_UserInputCapability;

/*-------------------------------------------------------*/

typedef struct _ConferenceCapability     /* SEQUENCE */
{
uint32 option_of_nonStandardData :
    1;
    uint16 size_of_nonStandardData;
    struct _NonStandardParameter *nonStandardData;
uint32 chairControlCapability :
    1;
    /*[...]*/
uint32 option_of_VideoIndicateMixingCapability :
    1;
uint32 option_of_multipointVisualizationCapability :
    1;
uint32 VideoIndicateMixingCapability :
    1;
uint32 multipointVisualizationCapability :
    1;
} S_ConferenceCapability;
typedef S_ConferenceCapability *PS_ConferenceCapability;

/*-------------------------------------------------------*/

typedef struct _CapabilityIdentifier     /* CHOICE */
{
    uint16 index;
    union
    {
        PS_OBJECTIDENT standard;
        struct _NonStandardParameter *h221NonStandard;
        PS_OCTETSTRING uuid;
        PS_int8STRING domainBased;
        /*[...]*/
    };
} S_CapabilityIdentifier;
typedef S_CapabilityIdentifier *PS_CapabilityIdentifier;

/*-------------------------------------------------------*/

typedef struct _GenericInformation     /* SEQUENCE */
{
uint32 option_of_subMessageIdentifier :
    1;
uint32 option_of_messageContent :
    1;
    struct _CapabilityIdentifier messageIdentifier;
    uint8 subMessageIdentifier;
    uint16 size_of_messageContent;
    struct _GenericParameter *messageContent;
    /*[...]*/
} S_GenericInformation;
typedef S_GenericInformation *PS_GenericInformation;

/*-------------------------------------------------------*/

typedef struct _GenericCapability     /* SEQUENCE */
{
uint32 option_of_maxBitRate :
    1;
uint32 option_of_collapsing :
    1;
uint32 option_of_nonCollapsing :
    1;
uint32 option_of_nonCollapsingRaw :
    1;
uint32 option_of_transport :
    1;
    struct _CapabilityIdentifier capabilityIdentifier;
    uint32 maxBitRate;
    uint16 size_of_collapsing;
    struct _GenericParameter *collapsing;
    uint16 size_of_nonCollapsing;
    struct _GenericParameter *nonCollapsing;
    S_OCTETSTRING nonCollapsingRaw;
    struct _DataProtocolCapability transport;
    /*[...]*/
} S_GenericCapability;
typedef S_GenericCapability *PS_GenericCapability;

/*-------------------------------------------------------*/

typedef struct _ParameterIdentifier     /* CHOICE */
{
    uint16 index;
    union
    {
        uint8 standard;
        struct _NonStandardParameter *h221NonStandard;
        PS_OCTETSTRING uuid;
        PS_int8STRING domainBased;
        /*[...]*/
    };
} S_ParameterIdentifier;
typedef S_ParameterIdentifier *PS_ParameterIdentifier;

/*-------------------------------------------------------*/

typedef struct _ParameterValue     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) logical */
        uint8 booleanArray;
        uint16 unsignedMin;
        uint16 unsignedMax;
        uint32 unsigned32Min;
        uint32 unsigned32Max;
        PS_OCTETSTRING octetString;
        struct _GenericParameter *genericParameter;
        /*[...]*/
    };
    uint16 size;
} S_ParameterValue;
typedef S_ParameterValue *PS_ParameterValue;

/*-------------------------------------------------------*/

typedef struct _GenericParameter     /* SEQUENCE */
{
uint32 option_of_supersedes :
    1;
    struct _ParameterIdentifier parameterIdentifier;
    struct _ParameterValue parameterValue;
    uint16 size_of_supersedes;
    struct _ParameterIdentifier *supersedes;
    /*[...]*/
} S_GenericParameter;
typedef S_GenericParameter *PS_GenericParameter;

/*-------------------------------------------------------*/

typedef struct _RlcMultiplexParameters     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _H223LogicalChannelParameters *h223LogicalChannelParameters;
        struct _V76LogicalChannelParameters *v76LogicalChannelParameters;
        /*[...]*/
        struct _H2250LogicalChannelParameters *h2250LogicalChannelParameters;
    };
} S_RlcMultiplexParameters;
typedef S_RlcMultiplexParameters *PS_RlcMultiplexParameters;

/*-------------------------------------------------------*/

typedef struct _MultiplexParameters     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _H222LogicalChannelParameters *h222LogicalChannelParameters;
        struct _H223LogicalChannelParameters *h223LogicalChannelParameters;
        struct _V76LogicalChannelParameters *v76LogicalChannelParameters;
        /*[...]*/
        struct _H2250LogicalChannelParameters *h2250LogicalChannelParameters;
        /* (NULL) none */
    };
} S_MultiplexParameters;
typedef S_MultiplexParameters *PS_MultiplexParameters;

/*-------------------------------------------------------*/

typedef struct _T120SetupProcedure     /* CHOICE */
{
    uint16 index;
    /* (NULL) originateCall */
    /* (NULL) waitForCall */
    /* (NULL) issueQuery */
    /*[...]*/
} S_T120SetupProcedure;
typedef S_T120SetupProcedure *PS_T120SetupProcedure;

/*-------------------------------------------------------*/

typedef struct _NetworkAddress     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _Q2931Address *q2931Address;
        PS_int8STRING e164Address;
        struct _TransportAddress *localAreaAddress;
        /*[...]*/
    };
} S_NetworkAddress;
typedef S_NetworkAddress *PS_NetworkAddress;

/*-------------------------------------------------------*/

typedef struct _Distribution     /* CHOICE */
{
    uint16 index;
    /* (NULL) unicast */
    /* (NULL) multicast */
    /*[...]*/
} S_Distribution;
typedef S_Distribution *PS_Distribution;

/*-------------------------------------------------------*/

typedef struct _NetworkAccessParameters     /* SEQUENCE */
{
uint32 option_of_distribution :
    1;
uint32 option_of_externalReference :
    1;
    struct _Distribution distribution;
    struct _NetworkAddress networkAddress;
uint32 associateConference :
    1;
    S_OCTETSTRING externalReference;
    /*[...]*/
uint32 option_of_t120SetupProcedure :
    1;
    struct _T120SetupProcedure t120SetupProcedure;
} S_NetworkAccessParameters;
typedef S_NetworkAccessParameters *PS_NetworkAccessParameters;

/*-------------------------------------------------------*/

typedef struct _Address     /* CHOICE */
{
    uint16 index;
    union
    {
        PS_int8STRING internationalNumber;
        PS_OCTETSTRING nsapAddress;
        /*[...]*/
    };
} S_Address;
typedef S_Address *PS_Address;

/*-------------------------------------------------------*/

typedef struct _Q2931Address     /* SEQUENCE */
{
uint32 option_of_subaddress :
    1;
    struct _Address address;
    S_OCTETSTRING subaddress;
    /*[...]*/
} S_Q2931Address;
typedef S_Q2931Address *PS_Q2931Address;

/*-------------------------------------------------------*/

typedef struct _V75Parameters     /* SEQUENCE */
{
uint32 audioHeaderPresent :
    1;
    /*[...]*/
} S_V75Parameters;
typedef S_V75Parameters *PS_V75Parameters;

/*-------------------------------------------------------*/

typedef struct _DataType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) nullData */
        struct _VideoCapability *videoData;
        struct _AudioCapability *audioData;
        struct _DataApplicationCapability *data;
        struct _EncryptionMode *encryptionData;
        /*[...]*/
        struct _NonStandardParameter *h235Control;
        struct _H235Media *h235Media;
    };
} S_DataType;
typedef S_DataType *PS_DataType;

/*-------------------------------------------------------*/

typedef struct _ReverseLogicalChannelParameters     /* SEQUENCE */
{
uint32 option_of_rlcMultiplexParameters :
    1;
    struct _DataType dataType;
    struct _RlcMultiplexParameters rlcMultiplexParameters;
    /*[...]*/
uint32 option_of_reverseLogicalChannelDependency :
    1;
uint32 option_of_replacementFor :
    1;
    uint16 reverseLogicalChannelDependency;
    uint16 replacementFor;
} S_ReverseLogicalChannelParameters;
typedef S_ReverseLogicalChannelParameters *PS_ReverseLogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _ForwardLogicalChannelParameters     /* SEQUENCE */
{
uint32 option_of_portNumber :
    1;
    uint16 portNumber;
    struct _DataType dataType;
    struct _MultiplexParameters multiplexParameters;
    /*[...]*/
uint32 option_of_forwardLogicalChannelDependency :
    1;
uint32 option_of_replacementFor :
    1;
    uint16 forwardLogicalChannelDependency;
    uint16 replacementFor;
} S_ForwardLogicalChannelParameters;
typedef S_ForwardLogicalChannelParameters *PS_ForwardLogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _MediaType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _VideoCapability *videoData;
        struct _AudioCapability *audioData;
        struct _DataApplicationCapability *data;
        /*[...]*/
    };
} S_MediaType;
typedef S_MediaType *PS_MediaType;

/*-------------------------------------------------------*/

typedef struct _H235Media     /* SEQUENCE */
{
    struct _EncryptionAuthenticationAndIntegrity encryptionAuthenticationAndIntegrity;
    struct _MediaType mediaType;
    /*[...]*/
} S_H235Media;
typedef S_H235Media *PS_H235Media;

/*-------------------------------------------------------*/

typedef struct _H222LogicalChannelParameters     /* SEQUENCE */
{
uint32 option_of_pcr_pid :
    1;
uint32 option_of_programDescriptors :
    1;
uint32 option_of_streamDescriptors :
    1;
    uint16 resourceID;
    uint16 subChannelID;
    uint16 pcr_pid;
    S_OCTETSTRING programDescriptors;
    S_OCTETSTRING streamDescriptors;
    /*[...]*/
} S_H222LogicalChannelParameters;
typedef S_H222LogicalChannelParameters *PS_H222LogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _AdaptationLayerType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) al1Framed */
        /* (NULL) al1NotFramed */
        /* (NULL) al2WithoutSequenceNumbers */
        /* (NULL) al2WithSequenceNumbers */
        struct _Al3 *al3;
        /*[...]*/
        struct _H223AL1MParameters *al1M;
        struct _H223AL2MParameters *al2M;
        struct _H223AL3MParameters *al3M;
    };
} S_AdaptationLayerType;
typedef S_AdaptationLayerType *PS_AdaptationLayerType;

/*-------------------------------------------------------*/

typedef struct _H223LogicalChannelParameters     /* SEQUENCE */
{
    struct _AdaptationLayerType adaptationLayerType;
uint32 segmentableFlag :
    1;
    /*[...]*/
} S_H223LogicalChannelParameters;
typedef S_H223LogicalChannelParameters *PS_H223LogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _Al3     /* SEQUENCE */
{
    uint8 controlFieldOctets;
    uint32 sendBufferSize;
} S_Al3;
typedef S_Al3 *PS_Al3;

/*-------------------------------------------------------*/

typedef struct _ArqType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) noArq */
        struct _H223AnnexCArqParameters *typeIArq;
        struct _H223AnnexCArqParameters *typeIIArq;
        /*[...]*/
    };
} S_ArqType;
typedef S_ArqType *PS_ArqType;

/*-------------------------------------------------------*/

typedef struct _CrcLength     /* CHOICE */
{
    uint16 index;
    /* (NULL) crc4bit */
    /* (NULL) crc12bit */
    /* (NULL) crc20bit */
    /* (NULL) crc28bit */
    /*[...]*/
    /* (NULL) crc8bit */
    /* (NULL) crc16bit */
    /* (NULL) crc32bit */
    /* (NULL) crcNotUsed */
} S_CrcLength;
typedef S_CrcLength *PS_CrcLength;

/*-------------------------------------------------------*/

typedef struct _HeaderFEC     /* CHOICE */
{
    uint16 index;
    /* (NULL) sebch16_7 */
    /* (NULL) golay24_12 */
    /*[...]*/
} S_HeaderFEC;
typedef S_HeaderFEC *PS_HeaderFEC;

/*-------------------------------------------------------*/

typedef struct _TransferMode     /* CHOICE */
{
    uint16 index;
    /* (NULL) framed */
    /* (NULL) unframed */
    /*[...]*/
} S_TransferMode;
typedef S_TransferMode *PS_TransferMode;

/*-------------------------------------------------------*/

typedef struct _H223AL1MParameters     /* SEQUENCE */
{
    struct _TransferMode transferMode;
    struct _HeaderFEC headerFEC;
    struct _CrcLength crcLength;
    uint8 rcpcCodeRate;
    struct _ArqType arqType;
uint32 alpduInterleaving :
    1;
uint32 alsduSplitting :
    1;
    /*[...]*/
uint32 option_of_rsCodeCorrection :
    1;
    uint8 rsCodeCorrection;
} S_H223AL1MParameters;
typedef S_H223AL1MParameters *PS_H223AL1MParameters;

/*-------------------------------------------------------*/

typedef struct _Al2HeaderFEC     /* CHOICE */
{
    uint16 index;
    /* (NULL) sebch16_5 */
    /* (NULL) golay24_12 */
    /*[...]*/
} S_Al2HeaderFEC;
typedef S_Al2HeaderFEC *PS_Al2HeaderFEC;

/*-------------------------------------------------------*/

typedef struct _H223AL2MParameters     /* SEQUENCE */
{
    struct _Al2HeaderFEC al2HeaderFEC;
uint32 alpduInterleaving :
    1;
    /*[...]*/
} S_H223AL2MParameters;
typedef S_H223AL2MParameters *PS_H223AL2MParameters;

/*-------------------------------------------------------*/

typedef struct _Al3ArqType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) noArq */
        struct _H223AnnexCArqParameters *typeIArq;
        struct _H223AnnexCArqParameters *typeIIArq;
        /*[...]*/
    };
} S_Al3ArqType;
typedef S_Al3ArqType *PS_Al3ArqType;

/*-------------------------------------------------------*/

typedef struct _Al3CrcLength     /* CHOICE */
{
    uint16 index;
    /* (NULL) crc4bit */
    /* (NULL) crc12bit */
    /* (NULL) crc20bit */
    /* (NULL) crc28bit */
    /*[...]*/
    /* (NULL) crc8bit */
    /* (NULL) crc16bit */
    /* (NULL) crc32bit */
    /* (NULL) crcNotUsed */
} S_Al3CrcLength;
typedef S_Al3CrcLength *PS_Al3CrcLength;

/*-------------------------------------------------------*/

typedef struct _HeaderFormat     /* CHOICE */
{
    uint16 index;
    /* (NULL) sebch16_7 */
    /* (NULL) golay24_12 */
    /*[...]*/
} S_HeaderFormat;
typedef S_HeaderFormat *PS_HeaderFormat;

/*-------------------------------------------------------*/

typedef struct _H223AL3MParameters     /* SEQUENCE */
{
    struct _HeaderFormat headerFormat;
    struct _Al3CrcLength al3CrcLength;
    uint8 rcpcCodeRate;
    struct _Al3ArqType al3ArqType;
uint32 alpduInterleaving :
    1;
    /*[...]*/
uint32 option_of_rsCodeCorrection :
    1;
    uint8 rsCodeCorrection;
} S_H223AL3MParameters;
typedef S_H223AL3MParameters *PS_H223AL3MParameters;

/*-------------------------------------------------------*/

typedef struct _NumberOfRetransmissions     /* CHOICE */
{
    uint16 index;
    union
    {
        uint8 finite;
        /* (NULL) infinite */
        /*[...]*/
    };
} S_NumberOfRetransmissions;
typedef S_NumberOfRetransmissions *PS_NumberOfRetransmissions;

/*-------------------------------------------------------*/

typedef struct _H223AnnexCArqParameters     /* SEQUENCE */
{
    struct _NumberOfRetransmissions numberOfRetransmissions;
    uint32 sendBufferSize;
    /*[...]*/
} S_H223AnnexCArqParameters;
typedef S_H223AnnexCArqParameters *PS_H223AnnexCArqParameters;

/*-------------------------------------------------------*/

typedef struct _Mode     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _ERM *eRM;
        /* (NULL) uNERM */
        /*[...]*/
    };
} S_Mode;
typedef S_Mode *PS_Mode;

/*-------------------------------------------------------*/

typedef struct _Recovery     /* CHOICE */
{
    uint16 index;
    /* (NULL) rej */
    /* (NULL) sREJ */
    /* (NULL) mSREJ */
    /*[...]*/
} S_Recovery;
typedef S_Recovery *PS_Recovery;

/*-------------------------------------------------------*/

typedef struct _ERM     /* SEQUENCE */
{
    uint8 windowSize;
    struct _Recovery recovery;
    /*[...]*/
} S_ERM;
typedef S_ERM *PS_ERM;

/*-------------------------------------------------------*/

typedef struct _SuspendResume     /* CHOICE */
{
    uint16 index;
    /* (NULL) noSuspendResume */
    /* (NULL) suspendResumewAddress */
    /* (NULL) suspendResumewoAddress */
    /*[...]*/
} S_SuspendResume;
typedef S_SuspendResume *PS_SuspendResume;

/*-------------------------------------------------------*/

typedef struct _CRCLength     /* CHOICE */
{
    uint16 index;
    /* (NULL) crc8bit */
    /* (NULL) crc16bit */
    /* (NULL) crc32bit */
    /*[...]*/
} S_CRCLength;
typedef S_CRCLength *PS_CRCLength;

/*-------------------------------------------------------*/

typedef struct _V76HDLCParameters     /* SEQUENCE */
{
    struct _CRCLength crcLength;
    uint16 n401;
uint32 loopbackTestProcedure :
    1;
    /*[...]*/
} S_V76HDLCParameters;
typedef S_V76HDLCParameters *PS_V76HDLCParameters;

/*-------------------------------------------------------*/

typedef struct _V76LogicalChannelParameters     /* SEQUENCE */
{
    struct _V76HDLCParameters hdlcParameters;
    struct _SuspendResume suspendResume;
uint32 uIH :
    1;
    struct _Mode mode;
    struct _V75Parameters v75Parameters;
    /*[...]*/
} S_V76LogicalChannelParameters;
typedef S_V76LogicalChannelParameters *PS_V76LogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _MediaPacketization     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) h261aVideoPacketization */
        /*[...]*/
        struct _RTPPayloadType *rtpPayloadType;
    };
} S_MediaPacketization;
typedef S_MediaPacketization *PS_MediaPacketization;

/*-------------------------------------------------------*/

typedef struct _PayloadDescriptor     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandardIdentifier;
        uint16 rfc_number;
        PS_OBJECTIDENT oid;
        /*[...]*/
    };
} S_PayloadDescriptor;
typedef S_PayloadDescriptor *PS_PayloadDescriptor;

/*-------------------------------------------------------*/

typedef struct _RTPPayloadType     /* SEQUENCE */
{
uint32 option_of_payloadType :
    1;
    struct _PayloadDescriptor payloadDescriptor;
    uint8 payloadType;
    /*[...]*/
} S_RTPPayloadType;
typedef S_RTPPayloadType *PS_RTPPayloadType;

/*-------------------------------------------------------*/

typedef struct _RedundancyEncoding     /* SEQUENCE */
{
uint32 option_of_secondaryEncoding :
    1;
    struct _RedundancyEncodingMethod redundancyEncodingMethod;
    struct _DataType secondaryEncoding;
    /*[...]*/
} S_RedundancyEncoding;
typedef S_RedundancyEncoding *PS_RedundancyEncoding;

/*-------------------------------------------------------*/

typedef struct _TransportAddress     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _UnicastAddress *unicastAddress;
        struct _MulticastAddress *multicastAddress;
        /*[...]*/
    };
} S_TransportAddress;
typedef S_TransportAddress *PS_TransportAddress;

/*-------------------------------------------------------*/

typedef struct _UnicastAddress     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _IPAddress *iPAddress;
        struct _IPXAddress *iPXAddress;
        struct _IP6Address *iP6Address;
        PS_OCTETSTRING netBios;
        struct _IPSourceRouteAddress *iPSourceRouteAddress;
        /*[...]*/
        PS_OCTETSTRING nsap;
        struct _NonStandardParameter *nonStandardAddress;
    };
} S_UnicastAddress;
typedef S_UnicastAddress *PS_UnicastAddress;

/*-------------------------------------------------------*/

typedef struct _Routing     /* CHOICE */
{
    uint16 index;
    /* (NULL) strict */
    /* (NULL) loose */
} S_Routing;
typedef S_Routing *PS_Routing;

/*-------------------------------------------------------*/

typedef struct _IPSourceRouteAddress     /* SEQUENCE */
{
    struct _Routing routing;
    S_OCTETSTRING network;
    uint16 tsapIdentifier;
    uint16 size_of_route;
    PS_OCTETSTRING route;
    /*[...]*/
} S_IPSourceRouteAddress;
typedef S_IPSourceRouteAddress *PS_IPSourceRouteAddress;

/*-------------------------------------------------------*/

typedef struct _IP6Address     /* SEQUENCE */
{
    S_OCTETSTRING network;
    uint16 tsapIdentifier;
    /*[...]*/
} S_IP6Address;
typedef S_IP6Address *PS_IP6Address;

/*-------------------------------------------------------*/

typedef struct _IPXAddress     /* SEQUENCE */
{
    S_OCTETSTRING node;
    S_OCTETSTRING netnum;
    S_OCTETSTRING tsapIdentifier;
    /*[...]*/
} S_IPXAddress;
typedef S_IPXAddress *PS_IPXAddress;

/*-------------------------------------------------------*/

typedef struct _IPAddress     /* SEQUENCE */
{
    S_OCTETSTRING network;
    uint16 tsapIdentifier;
    /*[...]*/
} S_IPAddress;
typedef S_IPAddress *PS_IPAddress;

/*-------------------------------------------------------*/

typedef struct _MulticastAddress     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _MaIpAddress *maIpAddress;
        struct _MaIp6Address *maIp6Address;
        /*[...]*/
        PS_OCTETSTRING nsap;
        struct _NonStandardParameter *nonStandardAddress;
    };
} S_MulticastAddress;
typedef S_MulticastAddress *PS_MulticastAddress;

/*-------------------------------------------------------*/

typedef struct _MaIp6Address     /* SEQUENCE */
{
    S_OCTETSTRING network;
    uint16 tsapIdentifier;
    /*[...]*/
} S_MaIp6Address;
typedef S_MaIp6Address *PS_MaIp6Address;

/*-------------------------------------------------------*/

typedef struct _MaIpAddress     /* SEQUENCE */
{
    S_OCTETSTRING network;
    uint16 tsapIdentifier;
    /*[...]*/
} S_MaIpAddress;
typedef S_MaIpAddress *PS_MaIpAddress;

/*-------------------------------------------------------*/

typedef struct _EncryptionSync     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
uint32 option_of_escrowentry :
    1;
    struct _NonStandardParameter nonStandard;
    uint8 synchFlag;
    S_OCTETSTRING h235Key;
    uint16 size_of_escrowentry;
    struct _EscrowData *escrowentry;
    /*[...]*/
} S_EncryptionSync;
typedef S_EncryptionSync *PS_EncryptionSync;

/*-------------------------------------------------------*/

typedef struct _OpenLogicalChannel     /* SEQUENCE */
{
uint32 option_of_reverseLogicalChannelParameters :
    1;
    uint16 forwardLogicalChannelNumber;
    struct _ForwardLogicalChannelParameters forwardLogicalChannelParameters;
    struct _ReverseLogicalChannelParameters reverseLogicalChannelParameters;
    /*[...]*/
uint32 option_of_separateStack :
    1;
uint32 option_of_encryptionSync :
    1;
    struct _NetworkAccessParameters separateStack;
    struct _EncryptionSync encryptionSync;
} S_OpenLogicalChannel;
typedef S_OpenLogicalChannel *PS_OpenLogicalChannel;

/*-------------------------------------------------------*/

typedef struct _EscrowData     /* SEQUENCE */
{
    S_OBJECTIDENT escrowID;
    S_BITSTRING escrowValue;
    /*[...]*/
} S_EscrowData;
typedef S_EscrowData *PS_EscrowData;

/*-------------------------------------------------------*/

typedef struct _ForwardMultiplexAckParameters     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _H2250LogicalChannelAckParameters *h2250LogicalChannelAckParameters;
        /*[...]*/
    };
} S_ForwardMultiplexAckParameters;
typedef S_ForwardMultiplexAckParameters *PS_ForwardMultiplexAckParameters;

/*-------------------------------------------------------*/

typedef struct _AckMultiplexParameters     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _H222LogicalChannelParameters *h222LogicalChannelParameters;
        /*[...]*/
        struct _H2250LogicalChannelParameters *h2250LogicalChannelParameters;
    };
} S_AckMultiplexParameters;
typedef S_AckMultiplexParameters *PS_AckMultiplexParameters;

/*-------------------------------------------------------*/

typedef struct _AckReverseLogicalChannelParameters     /* SEQUENCE */
{
uint32 option_of_portNumber :
    1;
uint32 option_of_ackMultiplexParameters :
    1;
    uint16 reverseLogicalChannelNumber;
    uint16 portNumber;
    struct _AckMultiplexParameters ackMultiplexParameters;
    /*[...]*/
uint32 option_of_replacementFor :
    1;
    uint16 replacementFor;
} S_AckReverseLogicalChannelParameters;
typedef S_AckReverseLogicalChannelParameters *PS_AckReverseLogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _OpenLogicalChannelAck     /* SEQUENCE */
{
uint32 option_of_ackReverseLogicalChannelParameters :
    1;
    uint16 forwardLogicalChannelNumber;
    struct _AckReverseLogicalChannelParameters ackReverseLogicalChannelParameters;
    /*[...]*/
uint32 option_of_separateStack :
    1;
uint32 option_of_forwardMultiplexAckParameters :
    1;
uint32 option_of_encryptionSync :
    1;
    struct _NetworkAccessParameters separateStack;
    struct _ForwardMultiplexAckParameters forwardMultiplexAckParameters;
    struct _EncryptionSync encryptionSync;
} S_OpenLogicalChannelAck;
typedef S_OpenLogicalChannelAck *PS_OpenLogicalChannelAck;

/*-------------------------------------------------------*/

typedef struct _OlcRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) unspecified */
    /* (NULL) unsuitableReverseParameters */
    /* (NULL) dataTypeNotSupported */
    /* (NULL) dataTypeNotAvailable */
    /* (NULL) unknownDataType */
    /* (NULL) dataTypeALCombinationNotSupported */
    /*[...]*/
    /* (NULL) multicastChannelNotAllowed */
    /* (NULL) insufficientBandwidth */
    /* (NULL) separateStackEstablishmentFailed */
    /* (NULL) invalidSessionID */
    /* (NULL) masterSlaveConflict */
    /* (NULL) waitForCommunicationMode */
    /* (NULL) invalidDependentChannel */
    /* (NULL) replacementForRejected */
} S_OlcRejectCause;
typedef S_OlcRejectCause *PS_OlcRejectCause;

/*-------------------------------------------------------*/

typedef struct _OpenLogicalChannelReject     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    struct _OlcRejectCause olcRejectCause;
    /*[...]*/
} S_OpenLogicalChannelReject;
typedef S_OpenLogicalChannelReject *PS_OpenLogicalChannelReject;

/*-------------------------------------------------------*/

typedef struct _OpenLogicalChannelConfirm     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    /*[...]*/
} S_OpenLogicalChannelConfirm;
typedef S_OpenLogicalChannelConfirm *PS_OpenLogicalChannelConfirm;

/*-------------------------------------------------------*/

typedef struct _H2250LogicalChannelAckParameters     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
uint32 option_of_sessionID :
    1;
uint32 option_of_mediaChannel :
    1;
uint32 option_of_mediaControlChannel :
    1;
uint32 option_of_dynamicRTPPayloadType :
    1;
    uint16 size_of_nonStandard;
    struct _NonStandardParameter *nonStandard;
    uint8 sessionID;
    struct _TransportAddress mediaChannel;
    struct _TransportAddress mediaControlChannel;
    uint8 dynamicRTPPayloadType;
    /*[...]*/
uint32 option_of_flowControlToZero :
    1;
uint32 option_of_portNumber :
    1;
uint32 flowControlToZero :
    1;
    uint16 portNumber;
} S_H2250LogicalChannelAckParameters;
typedef S_H2250LogicalChannelAckParameters *PS_H2250LogicalChannelAckParameters;

/*-------------------------------------------------------*/

typedef struct _Reason     /* CHOICE */
{
    uint16 index;
    /* (NULL) unknown */
    /* (NULL) reopen */
    /* (NULL) reservationFailure */
    /*[...]*/
} S_Reason;
typedef S_Reason *PS_Reason;

/*-------------------------------------------------------*/

typedef struct _Source     /* CHOICE */
{
    uint16 index;
    /* (NULL) user */
    /* (NULL) lcse */
} S_Source;
typedef S_Source *PS_Source;

/*-------------------------------------------------------*/

typedef struct _CloseLogicalChannel     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    struct _Source source;
    /*[...]*/
uint32 option_of_reason :
    1;
    struct _Reason reason;
} S_CloseLogicalChannel;
typedef S_CloseLogicalChannel *PS_CloseLogicalChannel;

/*-------------------------------------------------------*/

typedef struct _CloseLogicalChannelAck     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    /*[...]*/
} S_CloseLogicalChannelAck;
typedef S_CloseLogicalChannelAck *PS_CloseLogicalChannelAck;

/*-------------------------------------------------------*/

typedef struct _RccReason     /* CHOICE */
{
    uint16 index;
    /* (NULL) unknown */
    /* (NULL) normal */
    /* (NULL) reopen */
    /* (NULL) reservationFailure */
    /*[...]*/
} S_RccReason;
typedef S_RccReason *PS_RccReason;

/*-------------------------------------------------------*/

typedef struct _RequestChannelClose     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    /*[...]*/
uint32 option_of_qosCapability :
    1;
uint32 option_of_rccReason :
    1;
    struct _QOSCapability qosCapability;
    struct _RccReason rccReason;
} S_RequestChannelClose;
typedef S_RequestChannelClose *PS_RequestChannelClose;

/*-------------------------------------------------------*/

typedef struct _RequestChannelCloseAck     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    /*[...]*/
} S_RequestChannelCloseAck;
typedef S_RequestChannelCloseAck *PS_RequestChannelCloseAck;

/*-------------------------------------------------------*/

typedef struct _RccRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) unspecified */
    /*[...]*/
} S_RccRejectCause;
typedef S_RccRejectCause *PS_RccRejectCause;

/*-------------------------------------------------------*/

typedef struct _RequestChannelCloseReject     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    struct _RccRejectCause rccRejectCause;
    /*[...]*/
} S_RequestChannelCloseReject;
typedef S_RequestChannelCloseReject *PS_RequestChannelCloseReject;

/*-------------------------------------------------------*/

typedef struct _RequestChannelCloseRelease     /* SEQUENCE */
{
    uint16 forwardLogicalChannelNumber;
    /*[...]*/
} S_RequestChannelCloseRelease;
typedef S_RequestChannelCloseRelease *PS_RequestChannelCloseRelease;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntrySend     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 size_of_multiplexEntryDescriptors;
    struct _MultiplexEntryDescriptor *multiplexEntryDescriptors;
    /*[...]*/
} S_MultiplexEntrySend;
typedef S_MultiplexEntrySend *PS_MultiplexEntrySend;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntryDescriptor     /* SEQUENCE */
{
uint32 option_of_elementList :
    1;
    uint8 multiplexTableEntryNumber;
    uint16 size_of_elementList;
    struct _MultiplexElement *elementList;
} S_MultiplexEntryDescriptor;
typedef S_MultiplexEntryDescriptor *PS_MultiplexEntryDescriptor;

/*-------------------------------------------------------*/

typedef struct _RepeatCount     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 finite;
        /* (NULL) untilClosingFlag */
    };
} S_RepeatCount;
typedef S_RepeatCount *PS_RepeatCount;

/*-------------------------------------------------------*/

typedef struct _MuxType     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 logicalChannelNumber;
        struct _MultiplexElement *subElementList;
    };
    uint16 size;
} S_MuxType;
typedef S_MuxType *PS_MuxType;

/*-------------------------------------------------------*/

typedef struct _MultiplexElement     /* SEQUENCE */
{
    struct _MuxType muxType;
    struct _RepeatCount repeatCount;
} S_MultiplexElement;
typedef S_MultiplexElement *PS_MultiplexElement;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntrySendAck     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 size_of_multiplexTableEntryNumber;
    uint32* multiplexTableEntryNumber;
    /*[...]*/
} S_MultiplexEntrySendAck;
typedef S_MultiplexEntrySendAck *PS_MultiplexEntrySendAck;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntrySendReject     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 size_of_rejectionDescriptions;
    struct _MultiplexEntryRejectionDescriptions *rejectionDescriptions;
    /*[...]*/
} S_MultiplexEntrySendReject;
typedef S_MultiplexEntrySendReject *PS_MultiplexEntrySendReject;

/*-------------------------------------------------------*/

typedef struct _MeRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) unspecifiedCause */
    /* (NULL) descriptorTooComplex */
    /*[...]*/
} S_MeRejectCause;
typedef S_MeRejectCause *PS_MeRejectCause;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntryRejectionDescriptions     /* SEQUENCE */
{
    uint8 multiplexTableEntryNumber;
    struct _MeRejectCause meRejectCause;
    /*[...]*/
} S_MultiplexEntryRejectionDescriptions;
typedef S_MultiplexEntryRejectionDescriptions *PS_MultiplexEntryRejectionDescriptions;

/*-------------------------------------------------------*/

typedef struct _MultiplexEntrySendRelease     /* SEQUENCE */
{
    uint16 size_of_multiplexTableEntryNumber;
    uint32* multiplexTableEntryNumber;
    /*[...]*/
} S_MultiplexEntrySendRelease;
typedef S_MultiplexEntrySendRelease *PS_MultiplexEntrySendRelease;

/*-------------------------------------------------------*/

typedef struct _RequestMultiplexEntry     /* SEQUENCE */
{
    uint16 size_of_entryNumbers;
    uint32* entryNumbers;
    /*[...]*/
} S_RequestMultiplexEntry;
typedef S_RequestMultiplexEntry *PS_RequestMultiplexEntry;

/*-------------------------------------------------------*/

typedef struct _RequestMultiplexEntryAck     /* SEQUENCE */
{
    uint16 size_of_entryNumbers;
    uint32* entryNumbers;
    /*[...]*/
} S_RequestMultiplexEntryAck;
typedef S_RequestMultiplexEntryAck *PS_RequestMultiplexEntryAck;

/*-------------------------------------------------------*/

typedef struct _RequestMultiplexEntryReject     /* SEQUENCE */
{
    uint16 size_of_entryNumbers;
    uint32* entryNumbers;
    uint16 size_of_rejectionDescriptions;
    struct _RequestMultiplexEntryRejectionDescriptions *rejectionDescriptions;
    /*[...]*/
} S_RequestMultiplexEntryReject;
typedef S_RequestMultiplexEntryReject *PS_RequestMultiplexEntryReject;

/*-------------------------------------------------------*/

typedef struct _RmeRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) unspecifiedCause */
    /*[...]*/
} S_RmeRejectCause;
typedef S_RmeRejectCause *PS_RmeRejectCause;

/*-------------------------------------------------------*/

typedef struct _RequestMultiplexEntryRejectionDescriptions     /* SEQUENCE */
{
    uint8 multiplexTableEntryNumber;
    struct _RmeRejectCause rmeRejectCause;
    /*[...]*/
} S_RequestMultiplexEntryRejectionDescriptions;
typedef S_RequestMultiplexEntryRejectionDescriptions *PS_RequestMultiplexEntryRejectionDescriptions;

/*-------------------------------------------------------*/

typedef struct _RequestMultiplexEntryRelease     /* SEQUENCE */
{
    uint16 size_of_entryNumbers;
    uint32* entryNumbers;
    /*[...]*/
} S_RequestMultiplexEntryRelease;
typedef S_RequestMultiplexEntryRelease *PS_RequestMultiplexEntryRelease;

/*-------------------------------------------------------*/

typedef struct _RequestMode     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 size_of_requestedModes;
    struct _ModeDescription *requestedModes;
    /*[...]*/
} S_RequestMode;
typedef S_RequestMode *PS_RequestMode;

/*-------------------------------------------------------*/

typedef struct _Response     /* CHOICE */
{
    uint16 index;
    /* (NULL) willTransmitMostPreferredMode */
    /* (NULL) willTransmitLessPreferredMode */
    /*[...]*/
} S_Response;
typedef S_Response *PS_Response;

/*-------------------------------------------------------*/

typedef struct _RequestModeAck     /* SEQUENCE */
{
    uint8 sequenceNumber;
    struct _Response response;
    /*[...]*/
} S_RequestModeAck;
typedef S_RequestModeAck *PS_RequestModeAck;

/*-------------------------------------------------------*/

typedef struct _RmRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) modeUnavailable */
    /* (NULL) multipointConstraint */
    /* (NULL) requestDenied */
    /*[...]*/
} S_RmRejectCause;
typedef S_RmRejectCause *PS_RmRejectCause;

/*-------------------------------------------------------*/

typedef struct _RequestModeReject     /* SEQUENCE */
{
    uint8 sequenceNumber;
    struct _RmRejectCause rmRejectCause;
    /*[...]*/
} S_RequestModeReject;
typedef S_RequestModeReject *PS_RequestModeReject;

/*-------------------------------------------------------*/

typedef struct _RequestModeRelease     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_RequestModeRelease;
typedef S_RequestModeRelease *PS_RequestModeRelease;

/*-------------------------------------------------------*/

typedef struct _ModeDescription     /* SET OF */
{
    struct _ModeElement *item;
    uint16 size;
} S_ModeDescription;
typedef S_ModeDescription *PS_ModeDescription;

/*-------------------------------------------------------*/

typedef struct _ModeType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _VideoMode *videoMode;
        struct _AudioMode *audioMode;
        struct _DataMode *dataMode;
        struct _EncryptionMode *encryptionMode;
        /*[...]*/
        struct _H235Mode *h235Mode;
    };
} S_ModeType;
typedef S_ModeType *PS_ModeType;

/*-------------------------------------------------------*/

typedef struct _MediaMode     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _VideoMode *videoMode;
        struct _AudioMode *audioMode;
        struct _DataMode *dataMode;
        /*[...]*/
    };
} S_MediaMode;
typedef S_MediaMode *PS_MediaMode;

/*-------------------------------------------------------*/

typedef struct _H235Mode     /* SEQUENCE */
{
    struct _EncryptionAuthenticationAndIntegrity encryptionAuthenticationAndIntegrity;
    struct _MediaMode mediaMode;
    /*[...]*/
} S_H235Mode;
typedef S_H235Mode *PS_H235Mode;

/*-------------------------------------------------------*/

typedef struct _ModeAdaptationLayerType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) al1Framed */
        /* (NULL) al1NotFramed */
        /* (NULL) al2WithoutSequenceNumbers */
        /* (NULL) al2WithSequenceNumbers */
        struct _ModeAl3 *modeAl3;
        /*[...]*/
        struct _H223AL1MParameters *al1M;
        struct _H223AL2MParameters *al2M;
        struct _H223AL3MParameters *al3M;
    };
} S_ModeAdaptationLayerType;
typedef S_ModeAdaptationLayerType *PS_ModeAdaptationLayerType;

/*-------------------------------------------------------*/

typedef struct _H223ModeParameters     /* SEQUENCE */
{
    struct _ModeAdaptationLayerType modeAdaptationLayerType;
uint32 segmentableFlag :
    1;
    /*[...]*/
} S_H223ModeParameters;
typedef S_H223ModeParameters *PS_H223ModeParameters;

/*-------------------------------------------------------*/

typedef struct _ModeAl3     /* SEQUENCE */
{
    uint8 controlFieldOctets;
    uint32 sendBufferSize;
} S_ModeAl3;
typedef S_ModeAl3 *PS_ModeAl3;

/*-------------------------------------------------------*/

typedef struct _V76ModeParameters     /* CHOICE */
{
    uint16 index;
    /* (NULL) suspendResumewAddress */
    /* (NULL) suspendResumewoAddress */
    /*[...]*/
} S_V76ModeParameters;
typedef S_V76ModeParameters *PS_V76ModeParameters;

/*-------------------------------------------------------*/

typedef struct _SecondaryEncoding     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _AudioMode *audioData;
        /*[...]*/
    };
} S_SecondaryEncoding;
typedef S_SecondaryEncoding *PS_SecondaryEncoding;

/*-------------------------------------------------------*/

typedef struct _RedundancyEncodingMode     /* SEQUENCE */
{
uint32 option_of_secondaryEncoding :
    1;
    struct _RedundancyEncodingMethod redundancyEncodingMethod;
    struct _SecondaryEncoding secondaryEncoding;
    /*[...]*/
} S_RedundancyEncodingMode;
typedef S_RedundancyEncodingMode *PS_RedundancyEncodingMode;

/*-------------------------------------------------------*/

typedef struct _H2250ModeParameters     /* SEQUENCE */
{
uint32 option_of_redundancyEncodingMode :
    1;
    struct _RedundancyEncodingMode redundancyEncodingMode;
    /*[...]*/
} S_H2250ModeParameters;
typedef S_H2250ModeParameters *PS_H2250ModeParameters;

/*-------------------------------------------------------*/

typedef struct _ModeElement     /* SEQUENCE */
{
uint32 option_of_h223ModeParameters :
    1;
    struct _ModeType modeType;
    struct _H223ModeParameters h223ModeParameters;
    /*[...]*/
uint32 option_of_v76ModeParameters :
    1;
uint32 option_of_h2250ModeParameters :
    1;
uint32 option_of_genericModeParameters :
    1;
    struct _V76ModeParameters v76ModeParameters;
    struct _H2250ModeParameters h2250ModeParameters;
    struct _GenericCapability genericModeParameters;
} S_ModeElement;
typedef S_ModeElement *PS_ModeElement;

/*-------------------------------------------------------*/

typedef struct _VideoMode     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _H261VideoMode *h261VideoMode;
        struct _H262VideoMode *h262VideoMode;
        struct _H263VideoMode *h263VideoMode;
        struct _IS11172VideoMode *is11172VideoMode;
        /*[...]*/
        struct _GenericCapability *genericVideoMode;
    };
} S_VideoMode;
typedef S_VideoMode *PS_VideoMode;

/*-------------------------------------------------------*/

typedef struct _Resolution     /* CHOICE */
{
    uint16 index;
    /* (NULL) qcif */
    /* (NULL) cif */
} S_Resolution;
typedef S_Resolution *PS_Resolution;

/*-------------------------------------------------------*/

typedef struct _H261VideoMode     /* SEQUENCE */
{
    struct _Resolution resolution;
    uint16 bitRate;
uint32 stillImageTransmission :
    1;
    /*[...]*/
} S_H261VideoMode;
typedef S_H261VideoMode *PS_H261VideoMode;

/*-------------------------------------------------------*/

typedef struct _ProfileAndLevel     /* CHOICE */
{
    uint16 index;
    /* (NULL) profileAndLevel_SPatML */
    /* (NULL) profileAndLevel_MPatLL */
    /* (NULL) profileAndLevel_MPatML */
    /* (NULL) profileAndLevel_MPatH_14 */
    /* (NULL) profileAndLevel_MPatHL */
    /* (NULL) profileAndLevel_SNRatLL */
    /* (NULL) profileAndLevel_SNRatML */
    /* (NULL) profileAndLevel_SpatialatH_14 */
    /* (NULL) profileAndLevel_HPatML */
    /* (NULL) profileAndLevel_HPatH_14 */
    /* (NULL) profileAndLevel_HPatHL */
    /*[...]*/
} S_ProfileAndLevel;
typedef S_ProfileAndLevel *PS_ProfileAndLevel;

/*-------------------------------------------------------*/

typedef struct _H262VideoMode     /* SEQUENCE */
{
uint32 option_of_videoBitRate :
    1;
uint32 option_of_vbvBufferSize :
    1;
uint32 option_of_samplesPerLine :
    1;
uint32 option_of_linesPerFrame :
    1;
uint32 option_of_framesPerSecond :
    1;
uint32 option_of_luminanceSampleRate :
    1;
    struct _ProfileAndLevel profileAndLevel;
    uint32 videoBitRate;
    uint32 vbvBufferSize;
    uint16 samplesPerLine;
    uint16 linesPerFrame;
    uint8 framesPerSecond;
    uint32 luminanceSampleRate;
    /*[...]*/
} S_H262VideoMode;
typedef S_H262VideoMode *PS_H262VideoMode;

/*-------------------------------------------------------*/

typedef struct _H263Resolution     /* CHOICE */
{
    uint16 index;
    /* (NULL) sqcif */
    /* (NULL) qcif */
    /* (NULL) cif */
    /* (NULL) cif4 */
    /* (NULL) cif16 */
    /*[...]*/
} S_H263Resolution;
typedef S_H263Resolution *PS_H263Resolution;

/*-------------------------------------------------------*/

typedef struct _H263VideoMode     /* SEQUENCE */
{
    struct _H263Resolution h263Resolution;
    uint16 bitRate;
uint32 unrestrictedVector :
    1;
uint32 arithmeticCoding :
    1;
uint32 advancedPrediction :
    1;
uint32 pbFrames :
    1;
    /*[...]*/
uint32 option_of_errorCompensation :
    1;
uint32 option_of_enhancementLayerInfo :
    1;
uint32 option_of_h263Options :
    1;
uint32 errorCompensation :
    1;
    struct _EnhancementLayerInfo enhancementLayerInfo;
    struct _H263Options h263Options;
} S_H263VideoMode;
typedef S_H263VideoMode *PS_H263VideoMode;

/*-------------------------------------------------------*/

typedef struct _IS11172VideoMode     /* SEQUENCE */
{
uint32 option_of_videoBitRate :
    1;
uint32 option_of_vbvBufferSize :
    1;
uint32 option_of_samplesPerLine :
    1;
uint32 option_of_linesPerFrame :
    1;
uint32 option_of_pictureRate :
    1;
uint32 option_of_luminanceSampleRate :
    1;
uint32 constrainedBitstream :
    1;
    uint32 videoBitRate;
    uint32 vbvBufferSize;
    uint16 samplesPerLine;
    uint16 linesPerFrame;
    uint8 pictureRate;
    uint32 luminanceSampleRate;
    /*[...]*/
} S_IS11172VideoMode;
typedef S_IS11172VideoMode *PS_IS11172VideoMode;

/*-------------------------------------------------------*/

typedef struct _AudioMode     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) g711Alaw64k */
        /* (NULL) g711Alaw56k */
        /* (NULL) g711Ulaw64k */
        /* (NULL) g711Ulaw56k */
        /* (NULL) g722_64k */
        /* (NULL) g722_56k */
        /* (NULL) g722_48k */
        /* (NULL) g728 */
        /* (NULL) g729 */
        /* (NULL) g729AnnexA */
        struct _ModeG7231 *modeG7231;
        struct _IS11172AudioMode *is11172AudioMode;
        struct _IS13818AudioMode *is13818AudioMode;
        /*[...]*/
        uint16 g729wAnnexB;
        uint16 g729AnnexAwAnnexB;
        struct _G7231AnnexCMode *g7231AnnexCMode;
        struct _GSMAudioCapability *gsmFullRate;
        struct _GSMAudioCapability *gsmHalfRate;
        struct _GSMAudioCapability *gsmEnhancedFullRate;
        struct _GenericCapability *genericAudioMode;
        struct _G729Extensions *g729Extensions;
    };
} S_AudioMode;
typedef S_AudioMode *PS_AudioMode;

/*-------------------------------------------------------*/

typedef struct _ModeG7231     /* CHOICE */
{
    uint16 index;
    /* (NULL) noSilenceSuppressionLowRate */
    /* (NULL) noSilenceSuppressionHighRate */
    /* (NULL) silenceSuppressionLowRate */
    /* (NULL) silenceSuppressionHighRate */
} S_ModeG7231;
typedef S_ModeG7231 *PS_ModeG7231;

/*-------------------------------------------------------*/

typedef struct _MultichannelType     /* CHOICE */
{
    uint16 index;
    /* (NULL) singleChannel */
    /* (NULL) twoChannelStereo */
    /* (NULL) twoChannelDual */
} S_MultichannelType;
typedef S_MultichannelType *PS_MultichannelType;

/*-------------------------------------------------------*/

typedef struct _AudioSampling     /* CHOICE */
{
    uint16 index;
    /* (NULL) audioSampling32k */
    /* (NULL) audioSampling44k1 */
    /* (NULL) audioSampling48k */
} S_AudioSampling;
typedef S_AudioSampling *PS_AudioSampling;

/*-------------------------------------------------------*/

typedef struct _AudioLayer     /* CHOICE */
{
    uint16 index;
    /* (NULL) audioLayer1 */
    /* (NULL) audioLayer2 */
    /* (NULL) audioLayer3 */
} S_AudioLayer;
typedef S_AudioLayer *PS_AudioLayer;

/*-------------------------------------------------------*/

typedef struct _IS11172AudioMode     /* SEQUENCE */
{
    struct _AudioLayer audioLayer;
    struct _AudioSampling audioSampling;
    struct _MultichannelType multichannelType;
    uint16 bitRate;
    /*[...]*/
} S_IS11172AudioMode;
typedef S_IS11172AudioMode *PS_IS11172AudioMode;

/*-------------------------------------------------------*/

typedef struct _Is13818MultichannelType     /* CHOICE */
{
    uint16 index;
    /* (NULL) singleChannel */
    /* (NULL) twoChannelStereo */
    /* (NULL) twoChannelDual */
    /* (NULL) threeChannels2_1 */
    /* (NULL) threeChannels3_0 */
    /* (NULL) fourChannels2_0_2_0 */
    /* (NULL) fourChannels2_2 */
    /* (NULL) fourChannels3_1 */
    /* (NULL) fiveChannels3_0_2_0 */
    /* (NULL) fiveChannels3_2 */
} S_Is13818MultichannelType;
typedef S_Is13818MultichannelType *PS_Is13818MultichannelType;

/*-------------------------------------------------------*/

typedef struct _Is13818AudioSampling     /* CHOICE */
{
    uint16 index;
    /* (NULL) audioSampling16k */
    /* (NULL) audioSampling22k05 */
    /* (NULL) audioSampling24k */
    /* (NULL) audioSampling32k */
    /* (NULL) audioSampling44k1 */
    /* (NULL) audioSampling48k */
} S_Is13818AudioSampling;
typedef S_Is13818AudioSampling *PS_Is13818AudioSampling;

/*-------------------------------------------------------*/

typedef struct _Is13818AudioLayer     /* CHOICE */
{
    uint16 index;
    /* (NULL) audioLayer1 */
    /* (NULL) audioLayer2 */
    /* (NULL) audioLayer3 */
} S_Is13818AudioLayer;
typedef S_Is13818AudioLayer *PS_Is13818AudioLayer;

/*-------------------------------------------------------*/

typedef struct _IS13818AudioMode     /* SEQUENCE */
{
    struct _Is13818AudioLayer is13818AudioLayer;
    struct _Is13818AudioSampling is13818AudioSampling;
    struct _Is13818MultichannelType is13818MultichannelType;
uint32 lowFrequencyEnhancement :
    1;
uint32 multilingual :
    1;
    uint16 bitRate;
    /*[...]*/
} S_IS13818AudioMode;
typedef S_IS13818AudioMode *PS_IS13818AudioMode;

/*-------------------------------------------------------*/

typedef struct _ModeG723AnnexCAudioMode     /* SEQUENCE */
{
    uint8 highRateMode0;
    uint8 highRateMode1;
    uint8 lowRateMode0;
    uint8 lowRateMode1;
    uint8 sidMode0;
    uint8 sidMode1;
    /*[...]*/
} S_ModeG723AnnexCAudioMode;
typedef S_ModeG723AnnexCAudioMode *PS_ModeG723AnnexCAudioMode;

/*-------------------------------------------------------*/

typedef struct _G7231AnnexCMode     /* SEQUENCE */
{
    uint16 maxAl_sduAudioFrames;
uint32 silenceSuppression :
    1;
    struct _ModeG723AnnexCAudioMode modeG723AnnexCAudioMode;
    /*[...]*/
} S_G7231AnnexCMode;
typedef S_G7231AnnexCMode *PS_G7231AnnexCMode;

/*-------------------------------------------------------*/

typedef struct _DmApplication     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        struct _DataProtocolCapability *t120;
        struct _DataProtocolCapability *dsm_cc;
        struct _DataProtocolCapability *userData;
        struct _DataProtocolCapability *t84;
        struct _DataProtocolCapability *t434;
        struct _DataProtocolCapability *h224;
        struct _DmNlpid *dmNlpid;
        /* (NULL) dsvdControl */
        struct _DataProtocolCapability *h222DataPartitioning;
        /*[...]*/
        struct _DataProtocolCapability *t30fax;
        struct _DataProtocolCapability *t140;
        struct _DmT38fax *dmT38fax;
        struct _GenericCapability *genericDataMode;
    };
} S_DmApplication;
typedef S_DmApplication *PS_DmApplication;

/*-------------------------------------------------------*/

typedef struct _DataMode     /* SEQUENCE */
{
    struct _DmApplication dmApplication;
    uint32 bitRate;
    /*[...]*/
} S_DataMode;
typedef S_DataMode *PS_DataMode;

/*-------------------------------------------------------*/

typedef struct _DmT38fax     /* SEQUENCE */
{
    struct _DataProtocolCapability t38FaxProtocol;
    struct _T38FaxProfile t38FaxProfile;
} S_DmT38fax;
typedef S_DmT38fax *PS_DmT38fax;

/*-------------------------------------------------------*/

typedef struct _DmNlpid     /* SEQUENCE */
{
    struct _DataProtocolCapability nlpidProtocol;
    S_OCTETSTRING nlpidData;
} S_DmNlpid;
typedef S_DmNlpid *PS_DmNlpid;

/*-------------------------------------------------------*/

typedef struct _EncryptionMode     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) h233Encryption */
        /*[...]*/
    };
} S_EncryptionMode;
typedef S_EncryptionMode *PS_EncryptionMode;

/*-------------------------------------------------------*/

typedef struct _RoundTripDelayRequest     /* SEQUENCE */
{
    uint8 sequenceNumber;
    /*[...]*/
} S_RoundTripDelayRequest;
typedef S_RoundTripDelayRequest *PS_RoundTripDelayRequest;

/*-------------------------------------------------------*/

typedef struct _RoundTripDelayResponse     /* SEQUENCE */
{
    uint8 sequenceNumber;
    /*[...]*/
} S_RoundTripDelayResponse;
typedef S_RoundTripDelayResponse *PS_RoundTripDelayResponse;

/*-------------------------------------------------------*/

typedef struct _MlRequestType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) systemLoop */
        uint16 mediaLoop;
        uint16 logicalChannelLoop;
        /*[...]*/
    };
} S_MlRequestType;
typedef S_MlRequestType *PS_MlRequestType;

/*-------------------------------------------------------*/

typedef struct _MaintenanceLoopRequest     /* SEQUENCE */
{
    struct _MlRequestType mlRequestType;
    /*[...]*/
} S_MaintenanceLoopRequest;
typedef S_MaintenanceLoopRequest *PS_MaintenanceLoopRequest;

/*-------------------------------------------------------*/

typedef struct _MlAckType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) systemLoop */
        uint16 mediaLoop;
        uint16 logicalChannelLoop;
        /*[...]*/
    };
} S_MlAckType;
typedef S_MlAckType *PS_MlAckType;

/*-------------------------------------------------------*/

typedef struct _MaintenanceLoopAck     /* SEQUENCE */
{
    struct _MlAckType mlAckType;
    /*[...]*/
} S_MaintenanceLoopAck;
typedef S_MaintenanceLoopAck *PS_MaintenanceLoopAck;

/*-------------------------------------------------------*/

typedef struct _MlRejectCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) canNotPerformLoop */
    /*[...]*/
} S_MlRejectCause;
typedef S_MlRejectCause *PS_MlRejectCause;

/*-------------------------------------------------------*/

typedef struct _MlRejectType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) systemLoop */
        uint16 mediaLoop;
        uint16 logicalChannelLoop;
        /*[...]*/
    };
} S_MlRejectType;
typedef S_MlRejectType *PS_MlRejectType;

/*-------------------------------------------------------*/

typedef struct _MaintenanceLoopReject     /* SEQUENCE */
{
    struct _MlRejectType mlRejectType;
    struct _MlRejectCause mlRejectCause;
    /*[...]*/
} S_MaintenanceLoopReject;
typedef S_MaintenanceLoopReject *PS_MaintenanceLoopReject;

/*-------------------------------------------------------*/

typedef struct _MaintenanceLoopOffCommand     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_MaintenanceLoopOffCommand;
typedef S_MaintenanceLoopOffCommand *PS_MaintenanceLoopOffCommand;

/*-------------------------------------------------------*/

typedef struct _CommunicationModeCommand     /* SEQUENCE */
{
    uint16 size_of_communicationModeTable;
    struct _CommunicationModeTableEntry *communicationModeTable;
    /*[...]*/
} S_CommunicationModeCommand;
typedef S_CommunicationModeCommand *PS_CommunicationModeCommand;

/*-------------------------------------------------------*/

typedef struct _CommunicationModeRequest     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_CommunicationModeRequest;
typedef S_CommunicationModeRequest *PS_CommunicationModeRequest;

/*-------------------------------------------------------*/

typedef struct _CommunicationModeResponse     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _CommunicationModeTableEntry *communicationModeTable;
        /*[...]*/
    };
    uint16 size;
} S_CommunicationModeResponse;
typedef S_CommunicationModeResponse *PS_CommunicationModeResponse;

/*-------------------------------------------------------*/

typedef struct _CmtDataType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _VideoCapability *videoData;
        struct _AudioCapability *audioData;
        struct _DataApplicationCapability *data;
        /*[...]*/
    };
} S_CmtDataType;
typedef S_CmtDataType *PS_CmtDataType;

/*-------------------------------------------------------*/

typedef struct _ConferenceRequest     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) terminalListRequest */
        /* (NULL) makeMeChair */
        /* (NULL) cancelMakeMeChair */
        struct _TerminalLabel *dropTerminal;
        struct _TerminalLabel *requestTerminalID;
        /* (NULL) enterH243Password */
        /* (NULL) enterH243TerminalID */
        /* (NULL) enterH243ConferenceID */
        /*[...]*/
        /* (NULL) enterExtensionAddress */
        /* (NULL) requestChairTokenOwner */
        struct _RequestTerminalCertificate *requestTerminalCertificate;
        uint16 broadcastMyLogicalChannel;
        struct _TerminalLabel *makeTerminalBroadcaster;
        struct _TerminalLabel *sendThisSource;
        /* (NULL) requestAllTerminalIDs */
        struct _RemoteMCRequest *remoteMCRequest;
    };
} S_ConferenceRequest;
typedef S_ConferenceRequest *PS_ConferenceRequest;

/*-------------------------------------------------------*/

typedef struct _CertSelectionCriteria     /* SEQUENCE OF */
{
    struct _Criteria *item;
    uint16 size;
} S_CertSelectionCriteria;
typedef S_CertSelectionCriteria *PS_CertSelectionCriteria;

/*-------------------------------------------------------*/

typedef struct _Criteria     /* SEQUENCE */
{
    S_OBJECTIDENT field;
    S_OCTETSTRING value;
    /*[...]*/
} S_Criteria;
typedef S_Criteria *PS_Criteria;

/*-------------------------------------------------------*/

typedef struct _TerminalLabel     /* SEQUENCE */
{
    uint8 mcuNumber;
    uint8 terminalNumber;
    /*[...]*/
} S_TerminalLabel;
typedef S_TerminalLabel *PS_TerminalLabel;

/*-------------------------------------------------------*/

typedef struct _H2250LogicalChannelParameters     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
uint32 option_of_associatedSessionID :
    1;
uint32 option_of_mediaChannel :
    1;
uint32 option_of_mediaGuaranteedDelivery :
    1;
uint32 option_of_mediaControlChannel :
    1;
uint32 option_of_mediaControlGuaranteedDelivery :
    1;
uint32 option_of_silenceSuppression :
    1;
uint32 option_of_destination :
    1;
uint32 option_of_dynamicRTPPayloadType :
    1;
uint32 option_of_mediaPacketization :
    1;
    uint16 size_of_nonStandard;
    struct _NonStandardParameter *nonStandard;
    uint8 sessionID;
    uint8 associatedSessionID;
    struct _TransportAddress mediaChannel;
uint32 mediaGuaranteedDelivery :
    1;
    struct _TransportAddress mediaControlChannel;
uint32 mediaControlGuaranteedDelivery :
    1;
uint32 silenceSuppression :
    1;
    struct _TerminalLabel destination;
    uint8 dynamicRTPPayloadType;
    struct _MediaPacketization mediaPacketization;
    /*[...]*/
uint32 option_of_transportCapability :
    1;
uint32 option_of_redundancyEncoding :
    1;
uint32 option_of_source :
    1;
uint32 option_of_symmetricMediaAddress :
    1;
    struct _TransportCapability transportCapability;
    struct _RedundancyEncoding redundancyEncoding;
    struct _TerminalLabel source;
uint32 symmetricMediaAddress :
    1;
} S_H2250LogicalChannelParameters;
typedef S_H2250LogicalChannelParameters *PS_H2250LogicalChannelParameters;

/*-------------------------------------------------------*/

typedef struct _CommunicationModeTableEntry     /* SEQUENCE */
{
uint32 option_of_nonStandard :
    1;
uint32 option_of_associatedSessionID :
    1;
uint32 option_of_terminalLabel :
    1;
uint32 option_of_mediaChannel :
    1;
uint32 option_of_mediaGuaranteedDelivery :
    1;
uint32 option_of_mediaControlChannel :
    1;
uint32 option_of_mediaControlGuaranteedDelivery :
    1;
    uint16 size_of_nonStandard;
    struct _NonStandardParameter *nonStandard;
    uint8 sessionID;
    uint8 associatedSessionID;
    struct _TerminalLabel terminalLabel;
    S_int8STRING sessionDescription;
    struct _CmtDataType cmtDataType;
    struct _TransportAddress mediaChannel;
uint32 mediaGuaranteedDelivery :
    1;
    struct _TransportAddress mediaControlChannel;
uint32 mediaControlGuaranteedDelivery :
    1;
    /*[...]*/
uint32 option_of_redundancyEncoding :
    1;
uint32 option_of_sessionDependency :
    1;
uint32 option_of_destination :
    1;
    struct _RedundancyEncoding redundancyEncoding;
    uint8 sessionDependency;
    struct _TerminalLabel destination;
} S_CommunicationModeTableEntry;
typedef S_CommunicationModeTableEntry *PS_CommunicationModeTableEntry;

/*-------------------------------------------------------*/

typedef struct _RequestTerminalCertificate     /* SEQUENCE */
{
uint32 option_of_terminalLabel :
    1;
uint32 option_of_certSelectionCriteria :
    1;
uint32 option_of_sRandom :
    1;
    struct _TerminalLabel terminalLabel;
    struct _CertSelectionCriteria certSelectionCriteria;
    uint32 sRandom;
    /*[...]*/
} S_RequestTerminalCertificate;
typedef S_RequestTerminalCertificate *PS_RequestTerminalCertificate;

/*-------------------------------------------------------*/

typedef struct _ConferenceResponse     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _MCTerminalIDResponse *mCTerminalIDResponse;
        struct _TerminalIDResponse *terminalIDResponse;
        struct _ConferenceIDResponse *conferenceIDResponse;
        struct _PasswordResponse *passwordResponse;
        struct _TerminalLabel *terminalListResponse;
        /* (NULL) videoCommandReject */
        /* (NULL) terminalDropReject */
        struct _MakeMeChairResponse *makeMeChairResponse;
        /*[...]*/
        struct _ExtensionAddressResponse *extensionAddressResponse;
        struct _ChairTokenOwnerResponse *chairTokenOwnerResponse;
        struct _TerminalCertificateResponse *terminalCertificateResponse;
        struct _BroadcastMyLogicalChannelResponse *broadcastMyLogicalChannelResponse;
        struct _MakeTerminalBroadcasterResponse *makeTerminalBroadcasterResponse;
        struct _SendThisSourceResponse *sendThisSourceResponse;
        struct _RequestAllTerminalIDsResponse *requestAllTerminalIDsResponse;
        struct _RemoteMCResponse *remoteMCResponse;
    };
    uint16 size;
} S_ConferenceResponse;
typedef S_ConferenceResponse *PS_ConferenceResponse;

/*-------------------------------------------------------*/

typedef struct _SendThisSourceResponse     /* CHOICE */
{
    uint16 index;
    /* (NULL) grantedSendThisSource */
    /* (NULL) deniedSendThisSource */
    /*[...]*/
} S_SendThisSourceResponse;
typedef S_SendThisSourceResponse *PS_SendThisSourceResponse;

/*-------------------------------------------------------*/

typedef struct _MakeTerminalBroadcasterResponse     /* CHOICE */
{
    uint16 index;
    /* (NULL) grantedMakeTerminalBroadcaster */
    /* (NULL) deniedMakeTerminalBroadcaster */
    /*[...]*/
} S_MakeTerminalBroadcasterResponse;
typedef S_MakeTerminalBroadcasterResponse *PS_MakeTerminalBroadcasterResponse;

/*-------------------------------------------------------*/

typedef struct _BroadcastMyLogicalChannelResponse     /* CHOICE */
{
    uint16 index;
    /* (NULL) grantedBroadcastMyLogicalChannel */
    /* (NULL) deniedBroadcastMyLogicalChannel */
    /*[...]*/
} S_BroadcastMyLogicalChannelResponse;
typedef S_BroadcastMyLogicalChannelResponse *PS_BroadcastMyLogicalChannelResponse;

/*-------------------------------------------------------*/

typedef struct _TerminalCertificateResponse     /* SEQUENCE */
{
uint32 option_of_terminalLabel :
    1;
uint32 option_of_certificateResponse :
    1;
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING certificateResponse;
    /*[...]*/
} S_TerminalCertificateResponse;
typedef S_TerminalCertificateResponse *PS_TerminalCertificateResponse;

/*-------------------------------------------------------*/

typedef struct _ChairTokenOwnerResponse     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING terminalID;
    /*[...]*/
} S_ChairTokenOwnerResponse;
typedef S_ChairTokenOwnerResponse *PS_ChairTokenOwnerResponse;

/*-------------------------------------------------------*/

typedef struct _ExtensionAddressResponse     /* SEQUENCE */
{
    S_OCTETSTRING extensionAddress;
    /*[...]*/
} S_ExtensionAddressResponse;
typedef S_ExtensionAddressResponse *PS_ExtensionAddressResponse;

/*-------------------------------------------------------*/

typedef struct _MakeMeChairResponse     /* CHOICE */
{
    uint16 index;
    /* (NULL) grantedChairToken */
    /* (NULL) deniedChairToken */
    /*[...]*/
} S_MakeMeChairResponse;
typedef S_MakeMeChairResponse *PS_MakeMeChairResponse;

/*-------------------------------------------------------*/

typedef struct _PasswordResponse     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING password;
    /*[...]*/
} S_PasswordResponse;
typedef S_PasswordResponse *PS_PasswordResponse;

/*-------------------------------------------------------*/

typedef struct _ConferenceIDResponse     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING conferenceID;
    /*[...]*/
} S_ConferenceIDResponse;
typedef S_ConferenceIDResponse *PS_ConferenceIDResponse;

/*-------------------------------------------------------*/

typedef struct _TerminalIDResponse     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING terminalID;
    /*[...]*/
} S_TerminalIDResponse;
typedef S_TerminalIDResponse *PS_TerminalIDResponse;

/*-------------------------------------------------------*/

typedef struct _MCTerminalIDResponse     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING terminalID;
    /*[...]*/
} S_MCTerminalIDResponse;
typedef S_MCTerminalIDResponse *PS_MCTerminalIDResponse;

/*-------------------------------------------------------*/

typedef struct _RequestAllTerminalIDsResponse     /* SEQUENCE */
{
    uint16 size_of_terminalInformation;
    struct _TerminalInformation *terminalInformation;
    /*[...]*/
} S_RequestAllTerminalIDsResponse;
typedef S_RequestAllTerminalIDsResponse *PS_RequestAllTerminalIDsResponse;

/*-------------------------------------------------------*/

typedef struct _TerminalInformation     /* SEQUENCE */
{
    struct _TerminalLabel terminalLabel;
    S_OCTETSTRING terminalID;
    /*[...]*/
} S_TerminalInformation;
typedef S_TerminalInformation *PS_TerminalInformation;

/*-------------------------------------------------------*/

typedef struct _RemoteMCRequest     /* CHOICE */
{
    uint16 index;
    /* (NULL) masterActivate */
    /* (NULL) slaveActivate */
    /* (NULL) deActivate */
    /*[...]*/
} S_RemoteMCRequest;
typedef S_RemoteMCRequest *PS_RemoteMCRequest;

/*-------------------------------------------------------*/

typedef struct _RemoteMCResponse     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) accept */
        struct _Reject *reject;
        /*[...]*/
    };
} S_RemoteMCResponse;
typedef S_RemoteMCResponse *PS_RemoteMCResponse;

/*-------------------------------------------------------*/

typedef struct _Reject     /* CHOICE */
{
    uint16 index;
    /* (NULL) unspecified */
    /* (NULL) functionNotSupported */
    /*[...]*/
} S_Reject;
typedef S_Reject *PS_Reject;

/*-------------------------------------------------------*/

typedef struct _MultilinkRequest     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _CallInformation *callInformation;
        struct _AddConnection *addConnection;
        struct _RemoveConnection *removeConnection;
        struct _MaximumHeaderInterval *maximumHeaderInterval;
        /*[...]*/
    };
} S_MultilinkRequest;
typedef S_MultilinkRequest *PS_MultilinkRequest;

/*-------------------------------------------------------*/

typedef struct _RequestType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) currentIntervalInformation */
        uint16 requestedInterval;
        /*[...]*/
    };
} S_RequestType;
typedef S_RequestType *PS_RequestType;

/*-------------------------------------------------------*/

typedef struct _MaximumHeaderInterval     /* SEQUENCE */
{
    struct _RequestType requestType;
    /*[...]*/
} S_MaximumHeaderInterval;
typedef S_MaximumHeaderInterval *PS_MaximumHeaderInterval;

/*-------------------------------------------------------*/

typedef struct _CallInformation     /* SEQUENCE */
{
    uint16 maxNumberOfAdditionalConnections;
    /*[...]*/
} S_CallInformation;
typedef S_CallInformation *PS_CallInformation;

/*-------------------------------------------------------*/

typedef struct _MultilinkResponse     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _RespCallInformation *respCallInformation;
        struct _RespAddConnection *respAddConnection;
        struct _RespRemoveConnection *respRemoveConnection;
        struct _RespMaximumHeaderInterval *respMaximumHeaderInterval;
        /*[...]*/
    };
} S_MultilinkResponse;
typedef S_MultilinkResponse *PS_MultilinkResponse;

/*-------------------------------------------------------*/

typedef struct _RespMaximumHeaderInterval     /* SEQUENCE */
{
    uint16 currentInterval;
    /*[...]*/
} S_RespMaximumHeaderInterval;
typedef S_RespMaximumHeaderInterval *PS_RespMaximumHeaderInterval;

/*-------------------------------------------------------*/

typedef struct _ResponseCode     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) accepted */
        struct _Rejected *rejected;
        /*[...]*/
    };
} S_ResponseCode;
typedef S_ResponseCode *PS_ResponseCode;

/*-------------------------------------------------------*/

typedef struct _RespAddConnection     /* SEQUENCE */
{
    uint8 sequenceNumber;
    struct _ResponseCode responseCode;
    /*[...]*/
} S_RespAddConnection;
typedef S_RespAddConnection *PS_RespAddConnection;

/*-------------------------------------------------------*/

typedef struct _Rejected     /* CHOICE */
{
    uint16 index;
    /* (NULL) connectionsNotAvailable */
    /* (NULL) userRejected */
    /*[...]*/
} S_Rejected;
typedef S_Rejected *PS_Rejected;

/*-------------------------------------------------------*/

typedef struct _MultilinkIndication     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _CrcDesired *crcDesired;
        struct _ExcessiveError *excessiveError;
        /*[...]*/
    };
} S_MultilinkIndication;
typedef S_MultilinkIndication *PS_MultilinkIndication;

/*-------------------------------------------------------*/

typedef struct _CrcDesired     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_CrcDesired;
typedef S_CrcDesired *PS_CrcDesired;

/*-------------------------------------------------------*/

typedef struct _DialingInformation     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        struct _DialingInformationNumber *differential;
        uint16 infoNotAvailable;
        /*[...]*/
    };
    uint16 size;
} S_DialingInformation;
typedef S_DialingInformation *PS_DialingInformation;

/*-------------------------------------------------------*/

typedef struct _AddConnection     /* SEQUENCE */
{
    uint8 sequenceNumber;
    struct _DialingInformation dialingInformation;
    /*[...]*/
} S_AddConnection;
typedef S_AddConnection *PS_AddConnection;

/*-------------------------------------------------------*/

typedef struct _RespCallInformation     /* SEQUENCE */
{
    struct _DialingInformation dialingInformation;
    uint32 callAssociationNumber;
    /*[...]*/
} S_RespCallInformation;
typedef S_RespCallInformation *PS_RespCallInformation;

/*-------------------------------------------------------*/

typedef struct _DialingInformationNumber     /* SEQUENCE */
{
uint32 option_of_subAddress :
    1;
    S_int8STRING networkAddress;
    S_int8STRING subAddress;
    uint16 size_of_networkType;
    struct _DialingInformationNetworkType *networkType;
    /*[...]*/
} S_DialingInformationNumber;
typedef S_DialingInformationNumber *PS_DialingInformationNumber;

/*-------------------------------------------------------*/

typedef struct _DialingInformationNetworkType     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardMessage *nonStandard;
        /* (NULL) n_isdn */
        /* (NULL) gstn */
        /*[...]*/
    };
} S_DialingInformationNetworkType;
typedef S_DialingInformationNetworkType *PS_DialingInformationNetworkType;

/*-------------------------------------------------------*/

typedef struct _ConnectionIdentifier     /* SEQUENCE */
{
    uint32 channelTag;
    uint32 sequenceNumber;
    /*[...]*/
} S_ConnectionIdentifier;
typedef S_ConnectionIdentifier *PS_ConnectionIdentifier;

/*-------------------------------------------------------*/

typedef struct _RemoveConnection     /* SEQUENCE */
{
    struct _ConnectionIdentifier connectionIdentifier;
    /*[...]*/
} S_RemoveConnection;
typedef S_RemoveConnection *PS_RemoveConnection;

/*-------------------------------------------------------*/

typedef struct _RespRemoveConnection     /* SEQUENCE */
{
    struct _ConnectionIdentifier connectionIdentifier;
    /*[...]*/
} S_RespRemoveConnection;
typedef S_RespRemoveConnection *PS_RespRemoveConnection;

/*-------------------------------------------------------*/

typedef struct _ExcessiveError     /* SEQUENCE */
{
    struct _ConnectionIdentifier connectionIdentifier;
    /*[...]*/
} S_ExcessiveError;
typedef S_ExcessiveError *PS_ExcessiveError;

/*-------------------------------------------------------*/

typedef struct _LogicalChannelRateRequest     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 logicalChannelNumber;
    uint32 maximumBitRate;
    /*[...]*/
} S_LogicalChannelRateRequest;
typedef S_LogicalChannelRateRequest *PS_LogicalChannelRateRequest;

/*-------------------------------------------------------*/

typedef struct _LogicalChannelRateAcknowledge     /* SEQUENCE */
{
    uint8 sequenceNumber;
    uint16 logicalChannelNumber;
    uint32 maximumBitRate;
    /*[...]*/
} S_LogicalChannelRateAcknowledge;
typedef S_LogicalChannelRateAcknowledge *PS_LogicalChannelRateAcknowledge;

/*-------------------------------------------------------*/

typedef struct _LogicalChannelRateRejectReason     /* CHOICE */
{
    uint16 index;
    /* (NULL) undefinedReason */
    /* (NULL) insufficientResources */
    /*[...]*/
} S_LogicalChannelRateRejectReason;
typedef S_LogicalChannelRateRejectReason *PS_LogicalChannelRateRejectReason;

/*-------------------------------------------------------*/

typedef struct _LogicalChannelRateReject     /* SEQUENCE */
{
uint32 option_of_currentMaximumBitRate :
    1;
    uint8 sequenceNumber;
    uint16 logicalChannelNumber;
    struct _LogicalChannelRateRejectReason rejectReason;
    uint32 currentMaximumBitRate;
    /*[...]*/
} S_LogicalChannelRateReject;
typedef S_LogicalChannelRateReject *PS_LogicalChannelRateReject;

/*-------------------------------------------------------*/

typedef struct _LogicalChannelRateRelease     /* SEQUENCE */
{
    uint8(empty_sequence);
    /*[...]*/
} S_LogicalChannelRateRelease;
typedef S_LogicalChannelRateRelease *PS_LogicalChannelRateRelease;

/*-------------------------------------------------------*/

typedef struct _SendTerminalCapabilitySet     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _SpecificRequest *specificRequest;
        /* (NULL) genericRequest */
        /*[...]*/
    };
} S_SendTerminalCapabilitySet;
typedef S_SendTerminalCapabilitySet *PS_SendTerminalCapabilitySet;

/*-------------------------------------------------------*/

typedef struct _SpecificRequest     /* SEQUENCE */
{
uint32 option_of_capabilityTableEntryNumbers :
    1;
uint32 option_of_capabilityDescriptorNumbers :
    1;
uint32 multiplexCapability :
    1;
    uint16 size_of_capabilityTableEntryNumbers;
    uint32* capabilityTableEntryNumbers;
    uint16 size_of_capabilityDescriptorNumbers;
    uint32* capabilityDescriptorNumbers;
    /*[...]*/
} S_SpecificRequest;
typedef S_SpecificRequest *PS_SpecificRequest;

/*-------------------------------------------------------*/

typedef struct _EncryptionCommand     /* CHOICE */
{
    uint16 index;
    union
    {
        PS_OCTETSTRING encryptionSE;
        /* (NULL) encryptionIVRequest */
        struct _EncryptionAlgorithmID *encryptionAlgorithmID;
        /*[...]*/
    };
} S_EncryptionCommand;
typedef S_EncryptionCommand *PS_EncryptionCommand;

/*-------------------------------------------------------*/

typedef struct _EncryptionAlgorithmID     /* SEQUENCE */
{
    uint8 h233AlgorithmIdentifier;
    struct _NonStandardParameter associatedAlgorithm;
} S_EncryptionAlgorithmID;
typedef S_EncryptionAlgorithmID *PS_EncryptionAlgorithmID;

/*-------------------------------------------------------*/

typedef struct _FccRestriction     /* CHOICE */
{
    uint16 index;
    union
    {
        uint32 maximumBitRate;
        /* (NULL) noRestriction */
    };
} S_FccRestriction;
typedef S_FccRestriction *PS_FccRestriction;

/*-------------------------------------------------------*/

typedef struct _FccScope     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 logicalChannelNumber;
        uint16 resourceID;
        /* (NULL) wholeMultiplex */
    };
} S_FccScope;
typedef S_FccScope *PS_FccScope;

/*-------------------------------------------------------*/

typedef struct _FlowControlCommand     /* SEQUENCE */
{
    struct _FccScope fccScope;
    struct _FccRestriction fccRestriction;
    /*[...]*/
} S_FlowControlCommand;
typedef S_FlowControlCommand *PS_FlowControlCommand;

/*-------------------------------------------------------*/

typedef struct _EndSessionCommand     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) disconnect */
        struct _GstnOptions *gstnOptions;
        /*[...]*/
        struct _IsdnOptions *isdnOptions;
    };
} S_EndSessionCommand;
typedef S_EndSessionCommand *PS_EndSessionCommand;

/*-------------------------------------------------------*/

typedef struct _IsdnOptions     /* CHOICE */
{
    uint16 index;
    /* (NULL) telephonyMode */
    /* (NULL) v140 */
    /* (NULL) terminalOnHold */
    /*[...]*/
} S_IsdnOptions;
typedef S_IsdnOptions *PS_IsdnOptions;

/*-------------------------------------------------------*/

typedef struct _GstnOptions     /* CHOICE */
{
    uint16 index;
    /* (NULL) telephonyMode */
    /* (NULL) v8bis */
    /* (NULL) v34DSVD */
    /* (NULL) v34DuplexFAX */
    /* (NULL) v34H324 */
    /*[...]*/
} S_GstnOptions;
typedef S_GstnOptions *PS_GstnOptions;

/*-------------------------------------------------------*/

typedef struct _ConferenceCommand     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 broadcastMyLogicalChannel;
        uint16 cancelBroadcastMyLogicalChannel;
        struct _TerminalLabel *makeTerminalBroadcaster;
        /* (NULL) cancelMakeTerminalBroadcaster */
        struct _TerminalLabel *sendThisSource;
        /* (NULL) cancelSendThisSource */
        /* (NULL) dropConference */
        /*[...]*/
        struct _SubstituteConferenceIDCommand *substituteConferenceIDCommand;
    };
} S_ConferenceCommand;
typedef S_ConferenceCommand *PS_ConferenceCommand;

/*-------------------------------------------------------*/

typedef struct _SubstituteConferenceIDCommand     /* SEQUENCE */
{
    S_OCTETSTRING conferenceIdentifier;
    /*[...]*/
} S_SubstituteConferenceIDCommand;
typedef S_SubstituteConferenceIDCommand *PS_SubstituteConferenceIDCommand;

/*-------------------------------------------------------*/

typedef struct _McType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) equaliseDelay */
        /* (NULL) zeroDelay */
        /* (NULL) multipointModeCommand */
        /* (NULL) cancelMultipointModeCommand */
        /* (NULL) videoFreezePicture */
        /* (NULL) videoFastUpdatePicture */
        struct _VideoFastUpdateGOB *videoFastUpdateGOB;
        uint8 videoTemporalSpatialTradeOff;
        /* (NULL) videoSendSyncEveryGOB */
        /* (NULL) videoSendSyncEveryGOBCancel */
        /*[...]*/
        struct _VideoFastUpdateMB *videoFastUpdateMB;
        uint16 maxH223MUXPDUsize;
        struct _EncryptionSync *encryptionUpdate;
        struct _EncryptionUpdateRequest *encryptionUpdateRequest;
        /* (NULL) switchReceiveMediaOff */
        /* (NULL) switchReceiveMediaOn */
        struct _ProgressiveRefinementStart *progressiveRefinementStart;
        /* (NULL) progressiveRefinementAbortOne */
        /* (NULL) progressiveRefinementAbortContinuous */
    };
} S_McType;
typedef S_McType *PS_McType;

/*-------------------------------------------------------*/

typedef struct _MiscellaneousCommand     /* SEQUENCE */
{
    uint16 logicalChannelNumber;
    struct _McType mcType;
    /*[...]*/
} S_MiscellaneousCommand;
typedef S_MiscellaneousCommand *PS_MiscellaneousCommand;

/*-------------------------------------------------------*/

typedef struct _PrsRepeatCount     /* CHOICE */
{
    uint16 index;
    /* (NULL) doOneProgression */
    /* (NULL) doContinuousProgressions */
    /* (NULL) doOneIndependentProgression */
    /* (NULL) doContinuousIndependentProgressions */
    /*[...]*/
} S_PrsRepeatCount;
typedef S_PrsRepeatCount *PS_PrsRepeatCount;

/*-------------------------------------------------------*/

typedef struct _ProgressiveRefinementStart     /* SEQUENCE */
{
    struct _PrsRepeatCount prsRepeatCount;
    /*[...]*/
} S_ProgressiveRefinementStart;
typedef S_ProgressiveRefinementStart *PS_ProgressiveRefinementStart;

/*-------------------------------------------------------*/

typedef struct _VideoFastUpdateMB     /* SEQUENCE */
{
uint32 option_of_firstGOB :
    1;
uint32 option_of_firstMB :
    1;
    uint8 firstGOB;
    uint16 firstMB;
    uint16 numberOfMBs;
    /*[...]*/
} S_VideoFastUpdateMB;
typedef S_VideoFastUpdateMB *PS_VideoFastUpdateMB;

/*-------------------------------------------------------*/

typedef struct _VideoFastUpdateGOB     /* SEQUENCE */
{
    uint8 firstGOB;
    uint8 numberOfGOBs;
} S_VideoFastUpdateGOB;
typedef S_VideoFastUpdateGOB *PS_VideoFastUpdateGOB;

/*-------------------------------------------------------*/

typedef struct _KeyProtectionMethod     /* SEQUENCE */
{
uint32 secureChannel :
    1;
uint32 sharedSecret :
    1;
uint32 certProtectedKey :
    1;
    /*[...]*/
} S_KeyProtectionMethod;
typedef S_KeyProtectionMethod *PS_KeyProtectionMethod;

/*-------------------------------------------------------*/

typedef struct _EncryptionUpdateRequest     /* SEQUENCE */
{
uint32 option_of_keyProtectionMethod :
    1;
    struct _KeyProtectionMethod keyProtectionMethod;
    /*[...]*/
} S_EncryptionUpdateRequest;
typedef S_EncryptionUpdateRequest *PS_EncryptionUpdateRequest;

/*-------------------------------------------------------*/

typedef struct _H223MultiplexReconfiguration     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _H223ModeChange *h223ModeChange;
        struct _H223AnnexADoubleFlag *h223AnnexADoubleFlag;
        /*[...]*/
    };
} S_H223MultiplexReconfiguration;
typedef S_H223MultiplexReconfiguration *PS_H223MultiplexReconfiguration;

/*-------------------------------------------------------*/

typedef struct _H223AnnexADoubleFlag     /* CHOICE */
{
    uint16 index;
    /* (NULL) start */
    /* (NULL) stop */
    /*[...]*/
} S_H223AnnexADoubleFlag;
typedef S_H223AnnexADoubleFlag *PS_H223AnnexADoubleFlag;

/*-------------------------------------------------------*/

typedef struct _H223ModeChange     /* CHOICE */
{
    uint16 index;
    /* (NULL) toLevel0 */
    /* (NULL) toLevel1 */
    /* (NULL) toLevel2 */
    /* (NULL) toLevel2withOptionalHeader */
    /*[...]*/
} S_H223ModeChange;
typedef S_H223ModeChange *PS_H223ModeChange;

/*-------------------------------------------------------*/

typedef struct _Multiplex     /* CHOICE */
{
    uint16 index;
    /* (NULL) noMultiplex */
    /* (NULL) transportStream */
    /* (NULL) programStream */
    /*[...]*/
} S_Multiplex;
typedef S_Multiplex *PS_Multiplex;

/*-------------------------------------------------------*/

typedef struct _CmdReverseParameters     /* SEQUENCE */
{
    uint16 bitRate;
uint32 bitRateLockedToPCRClock :
    1;
uint32 bitRateLockedToNetworkClock :
    1;
    struct _Multiplex multiplex;
    /*[...]*/
} S_CmdReverseParameters;
typedef S_CmdReverseParameters *PS_CmdReverseParameters;

/*-------------------------------------------------------*/

typedef struct _CmdMultiplex     /* CHOICE */
{
    uint16 index;
    /* (NULL) noMultiplex */
    /* (NULL) transportStream */
    /* (NULL) programStream */
    /*[...]*/
} S_CmdMultiplex;
typedef S_CmdMultiplex *PS_CmdMultiplex;

/*-------------------------------------------------------*/

typedef struct _CmdAal     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _CmdAal1 *cmdAal1;
        struct _CmdAal5 *cmdAal5;
        /*[...]*/
    };
} S_CmdAal;
typedef S_CmdAal *PS_CmdAal;

/*-------------------------------------------------------*/

typedef struct _NewATMVCCommand     /* SEQUENCE */
{
    uint16 resourceID;
    uint16 bitRate;
uint32 bitRateLockedToPCRClock :
    1;
uint32 bitRateLockedToNetworkClock :
    1;
    struct _CmdAal cmdAal;
    struct _CmdMultiplex cmdMultiplex;
    struct _CmdReverseParameters cmdReverseParameters;
    /*[...]*/
} S_NewATMVCCommand;
typedef S_NewATMVCCommand *PS_NewATMVCCommand;

/*-------------------------------------------------------*/

typedef struct _CmdAal5     /* SEQUENCE */
{
    uint16 forwardMaximumSDUSize;
    uint16 backwardMaximumSDUSize;
    /*[...]*/
} S_CmdAal5;
typedef S_CmdAal5 *PS_CmdAal5;

/*-------------------------------------------------------*/

typedef struct _CmdErrorCorrection     /* CHOICE */
{
    uint16 index;
    /* (NULL) nullErrorCorrection */
    /* (NULL) longInterleaver */
    /* (NULL) shortInterleaver */
    /* (NULL) errorCorrectionOnly */
    /*[...]*/
} S_CmdErrorCorrection;
typedef S_CmdErrorCorrection *PS_CmdErrorCorrection;

/*-------------------------------------------------------*/

typedef struct _CmdClockRecovery     /* CHOICE */
{
    uint16 index;
    /* (NULL) nullClockRecovery */
    /* (NULL) srtsClockRecovery */
    /* (NULL) adaptiveClockRecovery */
    /*[...]*/
} S_CmdClockRecovery;
typedef S_CmdClockRecovery *PS_CmdClockRecovery;

/*-------------------------------------------------------*/

typedef struct _CmdAal1     /* SEQUENCE */
{
    struct _CmdClockRecovery cmdClockRecovery;
    struct _CmdErrorCorrection cmdErrorCorrection;
uint32 structuredDataTransfer :
    1;
uint32 partiallyFilledCells :
    1;
    /*[...]*/
} S_CmdAal1;
typedef S_CmdAal1 *PS_CmdAal1;

/*-------------------------------------------------------*/

typedef struct _FunctionNotUnderstood     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _RequestMessage *request;
        struct _ResponseMessage *response;
        struct _CommandMessage *command;
    };
} S_FunctionNotUnderstood;
typedef S_FunctionNotUnderstood *PS_FunctionNotUnderstood;

/*-------------------------------------------------------*/

typedef struct _FnsCause     /* CHOICE */
{
    uint16 index;
    /* (NULL) syntaxError */
    /* (NULL) semanticError */
    /* (NULL) unknownFunction */
    /*[...]*/
} S_FnsCause;
typedef S_FnsCause *PS_FnsCause;

/*-------------------------------------------------------*/

typedef struct _FunctionNotSupported     /* SEQUENCE */
{
uint32 option_of_returnedFunction :
    1;
    struct _FnsCause fnsCause;
    S_OCTETSTRING returnedFunction;
    /*[...]*/
} S_FunctionNotSupported;
typedef S_FunctionNotSupported *PS_FunctionNotSupported;

/*-------------------------------------------------------*/

typedef struct _ConferenceIndication     /* CHOICE */
{
    uint16 index;
    union
    {
        uint8 sbeNumber;
        struct _TerminalLabel *terminalNumberAssign;
        struct _TerminalLabel *terminalJoinedConference;
        struct _TerminalLabel *terminalLeftConference;
        /* (NULL) seenByAtLeastOneOther */
        /* (NULL) cancelSeenByAtLeastOneOther */
        /* (NULL) seenByAll */
        /* (NULL) cancelSeenByAll */
        struct _TerminalLabel *terminalYouAreSeeing;
        /* (NULL) requestForFloor */
        /*[...]*/
        /* (NULL) withdrawChairToken */
        struct _TerminalLabel *floorRequested;
        struct _TerminalYouAreSeeingInSubPictureNumber *terminalYouAreSeeingInSubPictureNumber;
        struct _VideoIndicateCompose *videoIndicateCompose;
    };
} S_ConferenceIndication;
typedef S_ConferenceIndication *PS_ConferenceIndication;

/*-------------------------------------------------------*/

typedef struct _TerminalYouAreSeeingInSubPictureNumber     /* SEQUENCE */
{
    uint8 terminalNumber;
    uint8 subPictureNumber;
    /*[...]*/
} S_TerminalYouAreSeeingInSubPictureNumber;
typedef S_TerminalYouAreSeeingInSubPictureNumber *PS_TerminalYouAreSeeingInSubPictureNumber;

/*-------------------------------------------------------*/

typedef struct _VideoIndicateCompose     /* SEQUENCE */
{
    uint8 compositionNumber;
    /*[...]*/
} S_VideoIndicateCompose;
typedef S_VideoIndicateCompose *PS_VideoIndicateCompose;

/*-------------------------------------------------------*/

typedef struct _MiType     /* CHOICE */
{
    uint16 index;
    union
    {
        /* (NULL) logicalChannelActive */
        /* (NULL) logicalChannelInactive */
        /* (NULL) multipointConference */
        /* (NULL) cancelMultipointConference */
        /* (NULL) multipointZeroComm */
        /* (NULL) cancelMultipointZeroComm */
        /* (NULL) multipointSecondaryStatus */
        /* (NULL) cancelMultipointSecondaryStatus */
        /* (NULL) videoIndicateReadyToActivate */
        uint8 videoTemporalSpatialTradeOff;
        /*[...]*/
        struct _VideoNotDecodedMBs *videoNotDecodedMBs;
        struct _TransportCapability *transportCapability;
    };
} S_MiType;
typedef S_MiType *PS_MiType;

/*-------------------------------------------------------*/

typedef struct _MiscellaneousIndication     /* SEQUENCE */
{
    uint16 logicalChannelNumber;
    struct _MiType miType;
    /*[...]*/
} S_MiscellaneousIndication;
typedef S_MiscellaneousIndication *PS_MiscellaneousIndication;

/*-------------------------------------------------------*/

typedef struct _VideoNotDecodedMBs     /* SEQUENCE */
{
    uint16 firstMB;
    uint16 numberOfMBs;
    uint8 temporalReference;
    /*[...]*/
} S_VideoNotDecodedMBs;
typedef S_VideoNotDecodedMBs *PS_VideoNotDecodedMBs;

/*-------------------------------------------------------*/

typedef struct _JiScope     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 logicalChannelNumber;
        uint16 resourceID;
        /* (NULL) wholeMultiplex */
    };
} S_JiScope;
typedef S_JiScope *PS_JiScope;

/*-------------------------------------------------------*/

typedef struct _JitterIndication     /* SEQUENCE */
{
uint32 option_of_skippedFrameCount :
    1;
uint32 option_of_additionalDecoderBuffer :
    1;
    struct _JiScope jiScope;
    uint8 estimatedReceivedJitterMantissa;
    uint8 estimatedReceivedJitterExponent;
    uint8 skippedFrameCount;
    uint32 additionalDecoderBuffer;
    /*[...]*/
} S_JitterIndication;
typedef S_JitterIndication *PS_JitterIndication;

/*-------------------------------------------------------*/

typedef struct _H223SkewIndication     /* SEQUENCE */
{
    uint16 logicalChannelNumber1;
    uint16 logicalChannelNumber2;
    uint16 skew;
    /*[...]*/
} S_H223SkewIndication;
typedef S_H223SkewIndication *PS_H223SkewIndication;

/*-------------------------------------------------------*/

typedef struct _H2250MaximumSkewIndication     /* SEQUENCE */
{
    uint16 logicalChannelNumber1;
    uint16 logicalChannelNumber2;
    uint16 maximumSkew;
    /*[...]*/
} S_H2250MaximumSkewIndication;
typedef S_H2250MaximumSkewIndication *PS_H2250MaximumSkewIndication;

/*-------------------------------------------------------*/

typedef struct _MCLocationIndication     /* SEQUENCE */
{
    struct _TransportAddress signalAddress;
    /*[...]*/
} S_MCLocationIndication;
typedef S_MCLocationIndication *PS_MCLocationIndication;

/*-------------------------------------------------------*/

typedef struct _VendorIdentification     /* SEQUENCE */
{
uint32 option_of_productNumber :
    1;
uint32 option_of_versionNumber :
    1;
    struct _NonStandardIdentifier vendor;
    S_OCTETSTRING productNumber;
    S_OCTETSTRING versionNumber;
    /*[...]*/
} S_VendorIdentification;
typedef S_VendorIdentification *PS_VendorIdentification;

/*-------------------------------------------------------*/

typedef struct _IrpMultiplex     /* CHOICE */
{
    uint16 index;
    /* (NULL) noMultiplex */
    /* (NULL) transportStream */
    /* (NULL) programStream */
    /*[...]*/
} S_IrpMultiplex;
typedef S_IrpMultiplex *PS_IrpMultiplex;

/*-------------------------------------------------------*/

typedef struct _IndReverseParameters     /* SEQUENCE */
{
    uint16 bitRate;
uint32 bitRateLockedToPCRClock :
    1;
uint32 bitRateLockedToNetworkClock :
    1;
    struct _IrpMultiplex irpMultiplex;
    /*[...]*/
} S_IndReverseParameters;
typedef S_IndReverseParameters *PS_IndReverseParameters;

/*-------------------------------------------------------*/

typedef struct _IndMultiplex     /* CHOICE */
{
    uint16 index;
    /* (NULL) noMultiplex */
    /* (NULL) transportStream */
    /* (NULL) programStream */
    /*[...]*/
} S_IndMultiplex;
typedef S_IndMultiplex *PS_IndMultiplex;

/*-------------------------------------------------------*/

typedef struct _IndAal     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _IndAal1 *indAal1;
        struct _IndAal5 *indAal5;
        /*[...]*/
    };
} S_IndAal;
typedef S_IndAal *PS_IndAal;

/*-------------------------------------------------------*/

typedef struct _NewATMVCIndication     /* SEQUENCE */
{
    uint16 resourceID;
    uint16 bitRate;
uint32 bitRateLockedToPCRClock :
    1;
uint32 bitRateLockedToNetworkClock :
    1;
    struct _IndAal indAal;
    struct _IndMultiplex indMultiplex;
    /*[...]*/
uint32 option_of_indReverseParameters :
    1;
    struct _IndReverseParameters indReverseParameters;
} S_NewATMVCIndication;
typedef S_NewATMVCIndication *PS_NewATMVCIndication;

/*-------------------------------------------------------*/

typedef struct _IndAal5     /* SEQUENCE */
{
    uint16 forwardMaximumSDUSize;
    uint16 backwardMaximumSDUSize;
    /*[...]*/
} S_IndAal5;
typedef S_IndAal5 *PS_IndAal5;

/*-------------------------------------------------------*/

typedef struct _IndErrorCorrection     /* CHOICE */
{
    uint16 index;
    /* (NULL) nullErrorCorrection */
    /* (NULL) longInterleaver */
    /* (NULL) shortInterleaver */
    /* (NULL) errorCorrectionOnly */
    /*[...]*/
} S_IndErrorCorrection;
typedef S_IndErrorCorrection *PS_IndErrorCorrection;

/*-------------------------------------------------------*/

typedef struct _IndClockRecovery     /* CHOICE */
{
    uint16 index;
    /* (NULL) nullClockRecovery */
    /* (NULL) srtsClockRecovery */
    /* (NULL) adaptiveClockRecovery */
    /*[...]*/
} S_IndClockRecovery;
typedef S_IndClockRecovery *PS_IndClockRecovery;

/*-------------------------------------------------------*/

typedef struct _IndAal1     /* SEQUENCE */
{
    struct _IndClockRecovery indClockRecovery;
    struct _IndErrorCorrection indErrorCorrection;
uint32 structuredDataTransfer :
    1;
uint32 partiallyFilledCells :
    1;
    /*[...]*/
} S_IndAal1;
typedef S_IndAal1 *PS_IndAal1;

/*-------------------------------------------------------*/

typedef struct _UserInputIndication     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        PS_int8STRING alphanumeric;
        /*[...]*/
        struct _UserInputSupportIndication *userInputSupportIndication;
        struct _Signal *signal;
        struct _SignalUpdate *signalUpdate;
    };
} S_UserInputIndication;
typedef S_UserInputIndication *PS_UserInputIndication;

/*-------------------------------------------------------*/

typedef struct _Rtp     /* SEQUENCE */
{
    uint16 logicalChannelNumber;
    /*[...]*/
} S_Rtp;
typedef S_Rtp *PS_Rtp;

/*-------------------------------------------------------*/

typedef struct _SignalUpdate     /* SEQUENCE */
{
uint32 option_of_rtp :
    1;
    uint16 duration;
    struct _Rtp rtp;
    /*[...]*/
} S_SignalUpdate;
typedef S_SignalUpdate *PS_SignalUpdate;

/*-------------------------------------------------------*/

typedef struct _SignalRtp     /* SEQUENCE */
{
uint32 option_of_timestamp :
    1;
uint32 option_of_expirationTime :
    1;
    uint32 timestamp;
    uint32 expirationTime;
    uint16 logicalChannelNumber;
    /*[...]*/
} S_SignalRtp;
typedef S_SignalRtp *PS_SignalRtp;

/*-------------------------------------------------------*/

typedef struct _Signal     /* SEQUENCE */
{
uint32 option_of_duration :
    1;
uint32 option_of_signalRtp :
    1;
    S_int8STRING signalType;
    uint16 duration;
    struct _SignalRtp signalRtp;
    /*[...]*/
} S_Signal;
typedef S_Signal *PS_Signal;

/*-------------------------------------------------------*/

typedef struct _UserInputSupportIndication     /* CHOICE */
{
    uint16 index;
    union
    {
        struct _NonStandardParameter *nonStandard;
        /* (NULL) basicString */
        /* (NULL) iA5String */
        /* (NULL) generalString */
        /*[...]*/
    };
} S_UserInputSupportIndication;
typedef S_UserInputSupportIndication *PS_UserInputSupportIndication;

/*-------------------------------------------------------*/

typedef struct _FciRestriction     /* CHOICE */
{
    uint16 index;
    union
    {
        uint32 maximumBitRate;
        /* (NULL) noRestriction */
    };
} S_FciRestriction;
typedef S_FciRestriction *PS_FciRestriction;

/*-------------------------------------------------------*/

typedef struct _FciScope     /* CHOICE */
{
    uint16 index;
    union
    {
        uint16 logicalChannelNumber;
        uint16 resourceID;
        /* (NULL) wholeMultiplex */
    };
} S_FciScope;
typedef S_FciScope *PS_FciScope;

/*-------------------------------------------------------*/

typedef struct _FlowControlIndication     /* SEQUENCE */
{
    struct _FciScope fciScope;
    struct _FciRestriction fciRestriction;
    /*[...]*/
} S_FlowControlIndication;
typedef S_FlowControlIndication *PS_FlowControlIndication;

/*-------------------------------------------------------*/

#endif
