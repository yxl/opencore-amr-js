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

//====================================================================
// FILE: codemain.c
//
//   Main contact routines for PER encoding/decoding of H.245 messages.
//====================================================================

#include "h245.h"
#include "h245def.h"
#include "h245msg.h"				    /* H245 Message I/F  Header	    	    */
#include "per.h"
#include "se.h"
#include "genericper.h"
#include "h245_decoder.h"
#include "h245_encoder.h"
#include "h245_deleter.h"
#include "h245_analysis.h"
#include "oscl_mem.h"
#include "pvlogger.h"
#include "oscl_file_io.h"
#include "oscl_mem.h"

/* ---------------------------------------- */
/* H.245 PER Analysis Routines              */
/* ---------------------------------------- */
#ifdef PVANALYZER

#include "analyzeper.h"

void MultiSysCtrlMessage_analyze(uint16 tag, uint8 type1, uint8 type2, uint8* message);
void RequestMessage_analyze(uint16 tag, uint8 type2, uint8* message);
void ResponseMessage_analyze(uint16 tag, uint8 type2, uint8* message);
void CommandMessage_analyze(uint16 tag, uint8 type2, uint8* message);
void IndicationMessage_analyze(uint16 tag, uint8 type2, uint8* message);
void ShowH245Hex(uint16 tag, uint8 type1, uint8 type2, uint16 size, uint8* data);
void ShowHexData(uint16 tag, uint16 indent, uint16 size, uint8* data);

#endif //PVANALYZER

// ========================================================
// Constructor()
//
// ========================================================
OSCL_EXPORT_REF PER::PER() :
        MyH245(NULL)
        , MySe(NULL)
        , MyLogger(NULL)
{
    MyLogger = PVLogger::GetLoggerObject("3g324m.h245.per");
}

// ========================================================
// Destructor()
//
// ========================================================
OSCL_EXPORT_REF PER::~PER()
{
}

// ========================================================
// Encode()
//
// This is the main call for the PER Encoder.  It encodes
// the H.245 message in pMsg, calls the analyzer functions
// as needed, and then sends the binary coded message to
// SRP.
// ========================================================
OSCL_EXPORT_REF void PER::Encode(PS_H245Msg pMsg)
{
    uint16		GetSize = 0;
    uint8*		pCodeData;

    // -----------------------
    // Analyzer output
    // -----------------------
#ifdef PVANALYZER
    MultiSysCtrlMessage_analyze(ANALYZER_PERE, pMsg->Type1, pMsg->Type2, pMsg->pData);
#endif

    // -----------------------
    // Message encoding
    // -----------------------
    MultiSysCtrlMessage_encode(&GetSize, &pCodeData, pMsg->pData, pMsg->Type1, pMsg->Type2);

    // -----------------------
    // Analyzer output (HEX)
    // -----------------------
#ifdef PVANALYZER
    ShowH245Hex(ANALYZER_PEREH, pMsg->Type1, pMsg->Type2, GetSize, pCodeData);
#endif

    // send encoded message
    if (MyH245)
    {
        MyH245->LowerLayerTx((unsigned char*)pCodeData, (int)GetSize);
    }

    OSCL_DEFAULT_FREE(pCodeData);
}

// ========================================================
// Reset()
//
// ========================================================
OSCL_EXPORT_REF void PER::Reset()
{
    // tells the decoder to stop processing remaining messages
    decodeSizeLeft = 0;
}

class DecodedMsgDestructDealloc : public OsclDestructDealloc
{
    public:
        DecodedMsgDestructDealloc(uint8 amsgtype1, uint8 amsgtype2, uint8* apdata) :
                msgtype1(amsgtype1), msgtype2(amsgtype2), pdata(apdata)
        {}

        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            // delete message
            PER::DeleteMessage(msgtype1, msgtype2, pdata);
            // destruct ref counter
            ((OsclRefCounterDA*)ptr)->~OsclRefCounterDA();
            // destruct this
            this->~DecodedMsgDestructDealloc();
            // free memory holding all
            OSCL_DEFAULT_FREE(ptr);
        }

    private:
        uint8 msgtype1;
        uint8 msgtype2;
        uint8 *pdata;
};

// ========================================================
// Decode()
//
// ========================================================
OSCL_EXPORT_REF void PER::Decode(uint8* pBuffer, uint32 size)
{
    uint8		MsgType1 = 0;
    uint8       MsgType2 = 0;
    uint16		GetSize = 0;
    uint8*		pData = NULL;
    uint8*		pBufferTop = NULL;
    int			nBytes = 0;

    decodeSizeLeft =  size;

    // Save original start of pBuffer (for later freeing)
    pBufferTop = pBuffer;


    // ----------------------------------------------------------
    // Decode messages from pBuffer.  There may be multiple msgs.
    // ----------------------------------------------------------
    while (decodeSizeLeft > 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, MyLogger, PVLOGMSG_DEBUG, (0, "PER::Decode - decodeSizeLeft(%d)", decodeSizeLeft));

        // ------------------------------------
        // Decode the current message to pData
        // ------------------------------------
        if (!Decode(nBytes, pBuffer, GetSize, pData, MsgType1, MsgType2))
        {
            return;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, MyLogger, PVLOGMSG_DEBUG, (0, "PER::Decode - num bytes decoded(%d), bytes:", nBytes));
        PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, MyLogger, PVLOGMSG_DEBUG, (0, 1, nBytes, pBuffer));

#ifdef PVANALYZER
        // ----------------------
        // Analyzer output
        // ----------------------
        MultiSysCtrlMessage_analyze(ANALYZER_PERD, MsgType1, MsgType2, pData);
#endif

        // ----------------------------
        // Send decoded message to SE
        // ----------------------------
        if (GetSize)
        {
            // pass message up via smart pointer.  Some messages are held by user.
            uint8 *space = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(OsclRefCounterDA) + sizeof(DecodedMsgDestructDealloc));
            DecodedMsgDestructDealloc *destructor = new(space + sizeof(OsclRefCounterDA)) DecodedMsgDestructDealloc(MsgType1, MsgType2, pData);
            OsclRefCounter *decodedMsgRefCnter    = new(space)                          OsclRefCounterDA(space, destructor);
            OsclSharedPtr<uint8> decodedMsgPtr(pData, decodedMsgRefCnter);

            MySe->MessageRecv(MsgType1, MsgType2, decodedMsgPtr, pBuffer, nBytes);
        }

        // Skip to the next message in pBuffer
        decodeSizeLeft -= nBytes;
        if (decodeSizeLeft < 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, MyLogger, PVLOGMSG_DEBUG, (0, "PER::Decode - decodeSizeLeft=%d", decodeSizeLeft));
        }
        pBuffer += nBytes;

    }

    // Free the original pBuffer space
    OSCL_DEFAULT_FREE(pBufferTop);
}


bool PER::Decode(int& nBytes, uint8* pBuffer, uint16& GetSize, uint8*& pData,
                 uint8&	MsgType1, uint8& MsgType2)
{
    int32 leave_status = 0;
    OSCL_TRY(leave_status, nBytes = MultiSysCtrlMessage_decode(pBuffer, &GetSize,
                                    &pData, &MsgType1, &MsgType2));
    OSCL_FIRST_CATCH_ANY(leave_status,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, MyLogger, PVLOGMSG_ERR, (0, "PER::Decode - caught LEAVE during decode"));
                         return false;
                        );
    return true;
}
// ========================================================
// DeleteMessage()
// Deep delete the message structure
// ========================================================
void PER::DeleteMessage(uint8 MsgType1, uint8 MsgType2, uint8* pData)
{
    switch (MsgType1)
    {
        case 0:
            S_RequestMessage request;
            request.index = MsgType2;
            request.nonStandard = (PS_NonStandardMessage)pData;
            Delete_RequestMessage(&request);
            break;
        case 1:
            S_ResponseMessage response;
            response.index = MsgType2;
            response.nonStandard = (PS_NonStandardMessage)pData;
            Delete_ResponseMessage(&response);
            break;
        case 2:
            S_CommandMessage command;
            command.index = MsgType2;
            command.nonStandard = (PS_NonStandardMessage)pData;
            Delete_CommandMessage(&command);
            break;
        case 3:
            S_IndicationMessage indication;
            indication.index = MsgType2;
            indication.nonStandard = (PS_NonStandardMessage)pData;
            Delete_IndicationMessage(&indication);
            break;
    }
}

// ========================================================
// MultiSysCtrlMessage_encode()
//
// ========================================================
void PER::MultiSysCtrlMessage_encode(uint16* pGetSize,
                                     uint8* *ppDataOut,
                                     uint8* pDataIn,
                                     uint8 type1,
                                     uint8 type2)
{
    PS_OutStream stream;

    stream = NewOutStream();
    PutChoiceIndex(4, 1, type1, stream);
    switch (type1)
    {
        case 0:
            RequestMessage_encode(pDataIn, type2, stream);
            break;
        case 1:
            ResponseMessage_encode(pDataIn, type2, stream);
            break;
        case 2:
            CommandMessage_encode(pDataIn, type2, stream);
            break;
        case 3:
            IndicationMessage_encode(pDataIn, type2, stream);
            break;
        default:
            ErrorMessageAndLeave("MultiSysCtrlMessage_encode: Illegal CHOICE index");
    }

    WriteRemainingBits(stream);
    *ppDataOut = stream->data;
    *pGetSize = stream->byteIndex;
    OSCL_DEFAULT_FREE(stream);

    return;
}

// ========================================================
// RequestMessage_encode()
//
// ========================================================
void PER::RequestMessage_encode(uint8* pDataIn,
                                uint8 type2,
                                PS_OutStream stream)
{
    PutChoiceIndex(11, 1, type2, stream);
    switch (type2)
    {
        case 0:
            Encode_NonStandardMessage((PS_NonStandardMessage)pDataIn, stream);
            //Delete_NonStandardMessage((PS_NonStandardMessage)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 1:
            Encode_MasterSlaveDetermination((PS_MasterSlaveDetermination)pDataIn, stream);
            //Delete_MasterSlaveDetermination((PS_MasterSlaveDetermination)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 2:
            Encode_TerminalCapabilitySet((PS_TerminalCapabilitySet)pDataIn, stream);
            //Delete_TerminalCapabilitySet((PS_TerminalCapabilitySet)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 3:
            Encode_OpenLogicalChannel((PS_OpenLogicalChannel)pDataIn, stream);
            //Delete_OpenLogicalChannel((PS_OpenLogicalChannel)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 4:
            Encode_CloseLogicalChannel((PS_CloseLogicalChannel)pDataIn, stream);
            //Delete_CloseLogicalChannel((PS_CloseLogicalChannel)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 5:
            Encode_RequestChannelClose((PS_RequestChannelClose)pDataIn, stream);
            //Delete_RequestChannelClose((PS_RequestChannelClose)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 6:
            Encode_MultiplexEntrySend((PS_MultiplexEntrySend)pDataIn, stream);
            //Delete_MultiplexEntrySend((PS_MultiplexEntrySend)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 7:
            Encode_RequestMultiplexEntry((PS_RequestMultiplexEntry)pDataIn, stream);
            //Delete_RequestMultiplexEntry((PS_RequestMultiplexEntry)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 8:
            Encode_RequestMode((PS_RequestMode)pDataIn, stream);
            //Delete_RequestMode((PS_RequestMode)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 9:
            Encode_RoundTripDelayRequest((PS_RoundTripDelayRequest)pDataIn, stream);
            //Delete_RoundTripDelayRequest((PS_RoundTripDelayRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 10:
            Encode_MaintenanceLoopRequest((PS_MaintenanceLoopRequest)pDataIn, stream);
            //Delete_MaintenanceLoopRequest((PS_MaintenanceLoopRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            PutExtensionItem(EPASS Encode_CommunicationModeRequest, pDataIn, stream);
            //Delete_CommunicationModeRequest((PS_CommunicationModeRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 12:
            PutExtensionItem(EPASS Encode_ConferenceRequest, pDataIn, stream);
            //Delete_ConferenceRequest((PS_ConferenceRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 13:
            PutExtensionItem(EPASS Encode_MultilinkRequest, pDataIn, stream);
            //Delete_MultilinkRequest((PS_MultilinkRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 14:
            PutExtensionItem(EPASS Encode_LogicalChannelRateRequest, pDataIn, stream);
            //Delete_LogicalChannelRateRequest((PS_LogicalChannelRateRequest)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        default:
            ErrorMessageAndLeave("Encode_RequestMessage: Illegal CHOICE index");
    }
}


// ========================================================
// ResponseMessage_encode()
//
// ========================================================
void PER::ResponseMessage_encode(uint8* pDataIn,
                                 uint8 type2,
                                 PS_OutStream stream)
{
    PutChoiceIndex(19, 1, type2, stream);
    switch (type2)
    {
        case 0:
            Encode_NonStandardMessage((PS_NonStandardMessage)pDataIn, stream);
            //Delete_NonStandardMessage((PS_NonStandardMessage)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 1:
            Encode_MasterSlaveDeterminationAck((PS_MasterSlaveDeterminationAck)pDataIn, stream);
            //Delete_MasterSlaveDeterminationAck((PS_MasterSlaveDeterminationAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 2:
            Encode_MasterSlaveDeterminationReject((PS_MasterSlaveDeterminationReject)pDataIn, stream);
            //Delete_MasterSlaveDeterminationReject((PS_MasterSlaveDeterminationReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 3:
            Encode_TerminalCapabilitySetAck((PS_TerminalCapabilitySetAck)pDataIn, stream);
            //Delete_TerminalCapabilitySetAck((PS_TerminalCapabilitySetAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 4:
            Encode_TerminalCapabilitySetReject((PS_TerminalCapabilitySetReject)pDataIn, stream);
            //Delete_TerminalCapabilitySetReject((PS_TerminalCapabilitySetReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 5:
            Encode_OpenLogicalChannelAck((PS_OpenLogicalChannelAck)pDataIn, stream);
            //Delete_OpenLogicalChannelAck((PS_OpenLogicalChannelAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 6:
            Encode_OpenLogicalChannelReject((PS_OpenLogicalChannelReject)pDataIn, stream);
            //Delete_OpenLogicalChannelReject((PS_OpenLogicalChannelReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 7:
            Encode_CloseLogicalChannelAck((PS_CloseLogicalChannelAck)pDataIn, stream);
            //Delete_CloseLogicalChannelAck((PS_CloseLogicalChannelAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 8:
            Encode_RequestChannelCloseAck((PS_RequestChannelCloseAck)pDataIn, stream);
            //Delete_RequestChannelCloseAck((PS_RequestChannelCloseAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 9:
            Encode_RequestChannelCloseReject((PS_RequestChannelCloseReject)pDataIn, stream);
            //Delete_RequestChannelCloseReject((PS_RequestChannelCloseReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 10:
            Encode_MultiplexEntrySendAck((PS_MultiplexEntrySendAck)pDataIn, stream);
            //Delete_MultiplexEntrySendAck((PS_MultiplexEntrySendAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 11:
            Encode_MultiplexEntrySendReject((PS_MultiplexEntrySendReject)pDataIn, stream);
            //Delete_MultiplexEntrySendReject((PS_MultiplexEntrySendReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 12:
            Encode_RequestMultiplexEntryAck((PS_RequestMultiplexEntryAck)pDataIn, stream);
            //Delete_RequestMultiplexEntryAck((PS_RequestMultiplexEntryAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 13:
            Encode_RequestMultiplexEntryReject((PS_RequestMultiplexEntryReject)pDataIn, stream);
            //Delete_RequestMultiplexEntryReject((PS_RequestMultiplexEntryReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 14:
            Encode_RequestModeAck((PS_RequestModeAck)pDataIn, stream);
            //Delete_RequestModeAck((PS_RequestModeAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 15:
            Encode_RequestModeReject((PS_RequestModeReject)pDataIn, stream);
            //Delete_RequestModeReject((PS_RequestModeReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 16:
            Encode_RoundTripDelayResponse((PS_RoundTripDelayResponse)pDataIn, stream);
            //Delete_RoundTripDelayResponse((PS_RoundTripDelayResponse)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 17:
            Encode_MaintenanceLoopAck((PS_MaintenanceLoopAck)pDataIn, stream);
            //Delete_MaintenanceLoopAck((PS_MaintenanceLoopAck)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 18:
            Encode_MaintenanceLoopReject((PS_MaintenanceLoopReject)pDataIn, stream);
            //Delete_MaintenanceLoopReject((PS_MaintenanceLoopReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            PutExtensionItem(EPASS Encode_CommunicationModeResponse, pDataIn, stream);
            //Delete_CommunicationModeResponse((PS_CommunicationModeResponse)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_ConferenceResponse, pDataIn, stream);
            //Delete_ConferenceResponse((PS_ConferenceResponse)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_MultilinkResponse, pDataIn, stream);
            //Delete_MultilinkResponse((PS_MultilinkResponse)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 22:
            PutExtensionItem(EPASS Encode_LogicalChannelRateAcknowledge, pDataIn, stream);
            //Delete_LogicalChannelRateAcknowledge((PS_LogicalChannelRateAcknowledge)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 23:
            PutExtensionItem(EPASS Encode_LogicalChannelRateReject, pDataIn, stream);
            //Delete_LogicalChannelRateReject((PS_LogicalChannelRateReject)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        default:
            ErrorMessageAndLeave("Encode_ResponseMessage: Illegal CHOICE index");
    }
}

// ========================================================
// CommandMessage_encode()
//
// ========================================================
void PER::CommandMessage_encode(uint8* pDataIn,
                                uint8 type2,
                                PS_OutStream stream)
{
    PutChoiceIndex(7, 1, type2, stream);
    switch (type2)
    {
        case 0:
            Encode_NonStandardMessage((PS_NonStandardMessage)pDataIn, stream);
            //Delete_NonStandardMessage((PS_NonStandardMessage)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 1:
            Encode_MaintenanceLoopOffCommand((PS_MaintenanceLoopOffCommand)pDataIn, stream);
            //Delete_MaintenanceLoopOffCommand((PS_MaintenanceLoopOffCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 2:
            Encode_SendTerminalCapabilitySet((PS_SendTerminalCapabilitySet)pDataIn, stream);
            //Delete_SendTerminalCapabilitySet((PS_SendTerminalCapabilitySet)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 3:
            Encode_EncryptionCommand((PS_EncryptionCommand)pDataIn, stream);
            //Delete_EncryptionCommand((PS_EncryptionCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 4:
            Encode_FlowControlCommand((PS_FlowControlCommand)pDataIn, stream);
            //Delete_FlowControlCommand((PS_FlowControlCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 5:
            Encode_EndSessionCommand((PS_EndSessionCommand)pDataIn, stream);
            //Delete_EndSessionCommand((PS_EndSessionCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 6:
            Encode_MiscellaneousCommand((PS_MiscellaneousCommand)pDataIn, stream);
            //Delete_MiscellaneousCommand((PS_MiscellaneousCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            PutExtensionItem(EPASS Encode_CommunicationModeCommand, pDataIn, stream);
            //Delete_CommunicationModeCommand((PS_CommunicationModeCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 8:
            PutExtensionItem(EPASS Encode_ConferenceCommand, pDataIn, stream);
            //Delete_ConferenceCommand((PS_ConferenceCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 9:
            PutExtensionItem(EPASS Encode_H223MultiplexReconfiguration, pDataIn, stream);
            //Delete_H223MultiplexReconfiguration((PS_H223MultiplexReconfiguration)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 10:
            PutExtensionItem(EPASS Encode_NewATMVCCommand, pDataIn, stream);
            //Delete_NewATMVCCommand((PS_NewATMVCCommand)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        default:
            ErrorMessageAndLeave("Encode_CommandMessage: Illegal CHOICE index");
    }
}

// ========================================================
// IndicationMessage_encode()
//
// ========================================================
void PER::IndicationMessage_encode(uint8* pDataIn,
                                   uint8 type2,
                                   PS_OutStream stream)
{
    PutChoiceIndex(14, 1, type2, stream);
    switch (type2)
    {
        case 0:
            Encode_NonStandardMessage((PS_NonStandardMessage)pDataIn, stream);
            //Delete_NonStandardMessage((PS_NonStandardMessage)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 1:
            Encode_FunctionNotUnderstood((PS_FunctionNotUnderstood)pDataIn, stream);
            //Delete_FunctionNotUnderstood((PS_FunctionNotUnderstood)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 2:
            Encode_MasterSlaveDeterminationRelease((PS_MasterSlaveDeterminationRelease)pDataIn, stream);
            //Delete_MasterSlaveDeterminationRelease((PS_MasterSlaveDeterminationRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 3:
            Encode_TerminalCapabilitySetRelease((PS_TerminalCapabilitySetRelease)pDataIn, stream);
            //Delete_TerminalCapabilitySetRelease((PS_TerminalCapabilitySetRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 4:
            Encode_OpenLogicalChannelConfirm((PS_OpenLogicalChannelConfirm)pDataIn, stream);
            //Delete_OpenLogicalChannelConfirm((PS_OpenLogicalChannelConfirm)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 5:
            Encode_RequestChannelCloseRelease((PS_RequestChannelCloseRelease)pDataIn, stream);
            //Delete_RequestChannelCloseRelease((PS_RequestChannelCloseRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 6:
            Encode_MultiplexEntrySendRelease((PS_MultiplexEntrySendRelease)pDataIn, stream);
            //Delete_MultiplexEntrySendRelease((PS_MultiplexEntrySendRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 7:
            Encode_RequestMultiplexEntryRelease((PS_RequestMultiplexEntryRelease)pDataIn, stream);
            //Delete_RequestMultiplexEntryRelease((PS_RequestMultiplexEntryRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 8:
            Encode_RequestModeRelease((PS_RequestModeRelease)pDataIn, stream);
            //Delete_RequestModeRelease((PS_RequestModeRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 9:
            Encode_MiscellaneousIndication((PS_MiscellaneousIndication)pDataIn, stream);
            //Delete_MiscellaneousIndication((PS_MiscellaneousIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 10:
            Encode_JitterIndication((PS_JitterIndication)pDataIn, stream);
            //Delete_JitterIndication((PS_JitterIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 11:
            Encode_H223SkewIndication((PS_H223SkewIndication)pDataIn, stream);
            //Delete_H223SkewIndication((PS_H223SkewIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 12:
            Encode_NewATMVCIndication((PS_NewATMVCIndication)pDataIn, stream);
            //Delete_NewATMVCIndication((PS_NewATMVCIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 13:
            Encode_UserInputIndication((PS_UserInputIndication)pDataIn, stream);
            //Delete_UserInputIndication((PS_UserInputIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            PutExtensionItem(EPASS Encode_H2250MaximumSkewIndication, pDataIn, stream);
            //Delete_H2250MaximumSkewIndication((PS_H2250MaximumSkewIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 15:
            PutExtensionItem(EPASS Encode_MCLocationIndication, pDataIn, stream);
            //Delete_MCLocationIndication((PS_MCLocationIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 16:
            PutExtensionItem(EPASS Encode_ConferenceIndication, pDataIn, stream);
            //Delete_ConferenceIndication((PS_ConferenceIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 17:
            PutExtensionItem(EPASS Encode_VendorIdentification, pDataIn, stream);
            //Delete_VendorIdentification((PS_VendorIdentification)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 18:
            PutExtensionItem(EPASS Encode_FunctionNotSupported, pDataIn, stream);
            //Delete_FunctionNotSupported((PS_FunctionNotSupported)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 19:
            PutExtensionItem(EPASS Encode_MultilinkIndication, pDataIn, stream);
            //Delete_MultilinkIndication((PS_MultilinkIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 20:
            PutExtensionItem(EPASS Encode_LogicalChannelRateRelease, pDataIn, stream);
            //Delete_LogicalChannelRateRelease((PS_LogicalChannelRateRelease)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        case 21:
            PutExtensionItem(EPASS Encode_FlowControlIndication, pDataIn, stream);
            //Delete_FlowControlIndication((PS_FlowControlIndication)pDataIn);
            //OSCL_DEFAULT_FREE(pDataIn);
            break;
        default:
            ErrorMessageAndLeave("Encode_IndicationMessage: Illegal CHOICE index");
    }
}

// ========================================================
// MultiSysCtrlMessage_decode()
//
// ========================================================
int PER::MultiSysCtrlMessage_decode(uint8*	pDataIn,
                                    uint16*	pGetSize,
                                    uint8*	*ppDataOut,
                                    uint8*	pType1,
                                    uint8*	pType2)
{
    PS_InStream stream;

    stream = (PS_InStream) OSCL_DEFAULT_MALLOC(sizeof(S_InStream));
    stream->bitIndex = 0;
    stream->data = pDataIn;
    *pType1 = (uint8)GetChoiceIndex(4, 1, stream);
    switch (*pType1)
    {
        case 0:
            RequestMessage_decode(ppDataOut, pType2, stream);
            break;
        case 1:
            ResponseMessage_decode(ppDataOut, pType2, stream);
            break;
        case 2:
            CommandMessage_decode(ppDataOut, pType2, stream);
            break;
        case 3:
            IndicationMessage_decode(ppDataOut, pType2, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        default:
            ErrorMessage("MultiSysCtrlMessage_decode: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
    ReadRemainingBits(stream);
    *pGetSize = (uint16)(stream->data - pDataIn);
    OSCL_DEFAULT_FREE(stream);
    return *pGetSize;
}

// ========================================================
// RequestMessage_decode()
//
// ========================================================
void PER::RequestMessage_decode(uint8* *ppDataOut, uint8* pType2, PS_InStream stream)
{
    *pType2 = (uint8)GetChoiceIndex(11, 1, stream);
    switch (*pType2)
    {
        case 0:
            *ppDataOut = (uint8*) OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage((PS_NonStandardMessage) *ppDataOut, stream);
            break;
        case 1:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDetermination));
            Decode_MasterSlaveDetermination((PS_MasterSlaveDetermination) *ppDataOut, stream);
            break;
        case 2:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySet));
            Decode_TerminalCapabilitySet((PS_TerminalCapabilitySet) *ppDataOut, stream);
            break;
        case 3:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannel));
            Decode_OpenLogicalChannel((PS_OpenLogicalChannel) *ppDataOut, stream);
            break;
        case 4:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_CloseLogicalChannel));
            Decode_CloseLogicalChannel((PS_CloseLogicalChannel) *ppDataOut, stream);
            break;
        case 5:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelClose));
            Decode_RequestChannelClose((PS_RequestChannelClose) *ppDataOut, stream);
            break;
        case 6:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySend));
            Decode_MultiplexEntrySend((PS_MultiplexEntrySend) *ppDataOut, stream);
            break;
        case 7:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntry));
            Decode_RequestMultiplexEntry((PS_RequestMultiplexEntry) *ppDataOut, stream);
            break;
        case 8:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMode));
            Decode_RequestMode((PS_RequestMode) *ppDataOut, stream);
            break;
        case 9:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RoundTripDelayRequest));
            Decode_RoundTripDelayRequest((PS_RoundTripDelayRequest) *ppDataOut, stream);
            break;
        case 10:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopRequest));
            Decode_MaintenanceLoopRequest((PS_MaintenanceLoopRequest) *ppDataOut, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeRequest));
            Decode_CommunicationModeRequest((PS_CommunicationModeRequest) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 12:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceRequest));
            Decode_ConferenceRequest((PS_ConferenceRequest) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 13:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkRequest));
            Decode_MultilinkRequest((PS_MultilinkRequest) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 14:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateRequest));
            Decode_LogicalChannelRateRequest((PS_LogicalChannelRateRequest) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_RequestMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

// ========================================================
// ResponseMessage_decode()
//
// ========================================================
void PER::ResponseMessage_decode(uint8* *ppDataOut, uint8* pType2, PS_InStream stream)
{
    *pType2 = (uint8) GetChoiceIndex(19, 1, stream);
    switch (*pType2)
    {
        case 0:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage((PS_NonStandardMessage) *ppDataOut, stream);
            break;
        case 1:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationAck));
            Decode_MasterSlaveDeterminationAck((PS_MasterSlaveDeterminationAck) *ppDataOut, stream);
            break;
        case 2:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationReject));
            Decode_MasterSlaveDeterminationReject((PS_MasterSlaveDeterminationReject) *ppDataOut, stream);
            break;
        case 3:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetAck));
            Decode_TerminalCapabilitySetAck((PS_TerminalCapabilitySetAck) *ppDataOut, stream);
            break;
        case 4:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetReject));
            Decode_TerminalCapabilitySetReject((PS_TerminalCapabilitySetReject) *ppDataOut, stream);
            break;
        case 5:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelAck));
            Decode_OpenLogicalChannelAck((PS_OpenLogicalChannelAck) *ppDataOut, stream);
            break;
        case 6:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelReject));
            Decode_OpenLogicalChannelReject((PS_OpenLogicalChannelReject) *ppDataOut, stream);
            break;
        case 7:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_CloseLogicalChannelAck));
            Decode_CloseLogicalChannelAck((PS_CloseLogicalChannelAck) *ppDataOut, stream);
            break;
        case 8:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseAck));
            Decode_RequestChannelCloseAck((PS_RequestChannelCloseAck) *ppDataOut, stream);
            break;
        case 9:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseReject));
            Decode_RequestChannelCloseReject((PS_RequestChannelCloseReject) *ppDataOut, stream);
            break;
        case 10:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendAck));
            Decode_MultiplexEntrySendAck((PS_MultiplexEntrySendAck) *ppDataOut, stream);
            break;
        case 11:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendReject));
            Decode_MultiplexEntrySendReject((PS_MultiplexEntrySendReject) *ppDataOut, stream);
            break;
        case 12:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryAck));
            Decode_RequestMultiplexEntryAck((PS_RequestMultiplexEntryAck) *ppDataOut, stream);
            break;
        case 13:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryReject));
            Decode_RequestMultiplexEntryReject((PS_RequestMultiplexEntryReject) *ppDataOut, stream);
            break;
        case 14:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeAck));
            Decode_RequestModeAck((PS_RequestModeAck) *ppDataOut, stream);
            break;
        case 15:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeReject));
            Decode_RequestModeReject((PS_RequestModeReject) *ppDataOut, stream);
            break;
        case 16:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RoundTripDelayResponse));
            Decode_RoundTripDelayResponse((PS_RoundTripDelayResponse) *ppDataOut, stream);
            break;
        case 17:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopAck));
            Decode_MaintenanceLoopAck((PS_MaintenanceLoopAck) *ppDataOut, stream);
            break;
        case 18:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopReject));
            Decode_MaintenanceLoopReject((PS_MaintenanceLoopReject) *ppDataOut, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeResponse));
            Decode_CommunicationModeResponse((PS_CommunicationModeResponse) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceResponse));
            Decode_ConferenceResponse((PS_ConferenceResponse) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkResponse));
            Decode_MultilinkResponse((PS_MultilinkResponse) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 22:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateAcknowledge));
            Decode_LogicalChannelRateAcknowledge((PS_LogicalChannelRateAcknowledge) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 23:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateReject));
            Decode_LogicalChannelRateReject((PS_LogicalChannelRateReject) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_ResponseMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

// ========================================================
// CommandMessage_decode()
//
// ========================================================
void PER::CommandMessage_decode(uint8* *ppDataOut, uint8* pType2, PS_InStream stream)
{
    *pType2 = (uint8) GetChoiceIndex(7, 1, stream);
    switch (*pType2)
    {
        case 0:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage((PS_NonStandardMessage) *ppDataOut, stream);
            break;
        case 1:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MaintenanceLoopOffCommand));
            Decode_MaintenanceLoopOffCommand((PS_MaintenanceLoopOffCommand) *ppDataOut, stream);
            break;
        case 2:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_SendTerminalCapabilitySet));
            Decode_SendTerminalCapabilitySet((PS_SendTerminalCapabilitySet) *ppDataOut, stream);
            break;
        case 3:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_EncryptionCommand));
            Decode_EncryptionCommand((PS_EncryptionCommand) *ppDataOut, stream);
            break;
        case 4:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_FlowControlCommand));
            Decode_FlowControlCommand((PS_FlowControlCommand) *ppDataOut, stream);
            break;
        case 5:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_EndSessionCommand));
            Decode_EndSessionCommand((PS_EndSessionCommand) *ppDataOut, stream);
            break;
        case 6:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MiscellaneousCommand));
            Decode_MiscellaneousCommand((PS_MiscellaneousCommand) *ppDataOut, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_CommunicationModeCommand));
            Decode_CommunicationModeCommand((PS_CommunicationModeCommand) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 8:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceCommand));
            Decode_ConferenceCommand((PS_ConferenceCommand) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 9:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_H223MultiplexReconfiguration));
            Decode_H223MultiplexReconfiguration((PS_H223MultiplexReconfiguration) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 10:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_NewATMVCCommand));
            Decode_NewATMVCCommand((PS_NewATMVCCommand) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_CommandMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

// ========================================================
// IndicationMessage_decode()
//
// ========================================================
void PER::IndicationMessage_decode(uint8* *ppDataOut, uint8* pType2, PS_InStream stream)
{
    *pType2 = (uint8)GetChoiceIndex(14, 1, stream);
    switch (*pType2)
    {
        case 0:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_NonStandardMessage));
            Decode_NonStandardMessage((PS_NonStandardMessage) *ppDataOut, stream);
            break;
        case 1:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_FunctionNotUnderstood));
            Decode_FunctionNotUnderstood((PS_FunctionNotUnderstood) *ppDataOut, stream);
            break;
        case 2:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MasterSlaveDeterminationRelease));
            Decode_MasterSlaveDeterminationRelease((PS_MasterSlaveDeterminationRelease) *ppDataOut, stream);
            break;
        case 3:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySetRelease));
            Decode_TerminalCapabilitySetRelease((PS_TerminalCapabilitySetRelease) *ppDataOut, stream);
            break;
        case 4:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_OpenLogicalChannelConfirm));
            Decode_OpenLogicalChannelConfirm((PS_OpenLogicalChannelConfirm) *ppDataOut, stream);
            break;
        case 5:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestChannelCloseRelease));
            Decode_RequestChannelCloseRelease((PS_RequestChannelCloseRelease) *ppDataOut, stream);
            break;
        case 6:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntrySendRelease));
            Decode_MultiplexEntrySendRelease((PS_MultiplexEntrySendRelease) *ppDataOut, stream);
            break;
        case 7:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMultiplexEntryRelease));
            Decode_RequestMultiplexEntryRelease((PS_RequestMultiplexEntryRelease) *ppDataOut, stream);
            break;
        case 8:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_RequestModeRelease));
            Decode_RequestModeRelease((PS_RequestModeRelease) *ppDataOut, stream);
            break;
        case 9:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MiscellaneousIndication));
            Decode_MiscellaneousIndication((PS_MiscellaneousIndication) *ppDataOut, stream);
            break;
        case 10:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_JitterIndication));
            Decode_JitterIndication((PS_JitterIndication) *ppDataOut, stream);
            break;
        case 11:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_H223SkewIndication));
            Decode_H223SkewIndication((PS_H223SkewIndication) *ppDataOut, stream);
            break;
        case 12:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_NewATMVCIndication));
            Decode_NewATMVCIndication((PS_NewATMVCIndication) *ppDataOut, stream);
            break;
        case 13:
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_UserInputIndication));
            Decode_UserInputIndication((PS_UserInputIndication) *ppDataOut, stream);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_H2250MaximumSkewIndication));
            Decode_H2250MaximumSkewIndication((PS_H2250MaximumSkewIndication) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 15:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MCLocationIndication));
            Decode_MCLocationIndication((PS_MCLocationIndication) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 16:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_ConferenceIndication));
            Decode_ConferenceIndication((PS_ConferenceIndication) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 17:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_VendorIdentification));
            Decode_VendorIdentification((PS_VendorIdentification) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 18:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_FunctionNotSupported));
            Decode_FunctionNotSupported((PS_FunctionNotSupported) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 19:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_MultilinkIndication));
            Decode_MultilinkIndication((PS_MultilinkIndication) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 20:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_LogicalChannelRateRelease));
            Decode_LogicalChannelRateRelease((PS_LogicalChannelRateRelease) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        case 21:
            GetLengthDet(stream);
            *ppDataOut = (uint8*)OSCL_DEFAULT_MALLOC(sizeof(S_FlowControlIndication));
            Decode_FlowControlIndication((PS_FlowControlIndication) *ppDataOut, stream);
            ReadRemainingBits(stream);
            break;
        default:
            ErrorMessage("Decode_IndicationMessage: Unsupported extension (skipping)");
            SkipOneExtension(stream);
    }
}

#ifdef PVANALYZER

/* ---------------------------------------- */
/* ---------------------------------------- */
/*   (RAN) H.245 PER Analysis Routines      */
/* ---------------------------------------- */
/* ---------------------------------------- */

/* ======================================================= */
/* PER Analysis for Top Level H.245 Messages (RAN)         */
/* Input Arguments:                                        */
/*    uint16 tag;            Identifies source of message  */
/*    uint8 type1,type2;     Choice indices (top 2 levels) */
/*    uint8* message;        Message data                  */
/* ======================================================= */
void MultiSysCtrlMessage_analyze(uint16 tag, uint8 type1, uint8 type2, uint8* message)
{
    Show245(tag, 0, "<============= BEGIN H.245 MESSAGE ANALYSIS =============>");
    switch (type1)
    {
        case 0:
            RequestMessage_analyze(tag, type2, message);
            break;
        case 1:
            ResponseMessage_analyze(tag, type2, message);
            break;
        case 2:
            CommandMessage_analyze(tag, type2, message);
            break;
        case 3:
            IndicationMessage_analyze(tag, type2, message);
            break;
        default:
            ErrorMessage("MultiSysCtrlMessage_analyze: Illegal CHOICE index");
    }
    Show245(tag, 0, "<============== END H.245 MESSAGE ANALYSIS ==============>");
    Show245(tag, 0, " ");
}

/* ================================================= */
/* PER Analysis for RequestMessage (RAN)             */
/* ================================================= */
void RequestMessage_analyze(uint16 tag, uint8 type2, uint8* message)
{
    int8 origin[] = "requestMessage";
    switch (type2)
    {
        case 0:
            Analyze_NonStandardMessage((PS_NonStandardMessage)message, origin, tag, 0);
            break;
        case 1:
            Analyze_MasterSlaveDetermination((PS_MasterSlaveDetermination)message, origin, tag, 0);
            break;
        case 2:
            Analyze_TerminalCapabilitySet((PS_TerminalCapabilitySet)message, origin, tag, 0);
            break;
        case 3:
            Analyze_OpenLogicalChannel((PS_OpenLogicalChannel)message, origin, tag, 0);
            break;
        case 4:
            Analyze_CloseLogicalChannel((PS_CloseLogicalChannel)message, origin, tag, 0);
            break;
        case 5:
            Analyze_RequestChannelClose((PS_RequestChannelClose)message, origin, tag, 0);
            break;
        case 6:
            Analyze_MultiplexEntrySend((PS_MultiplexEntrySend)message, origin, tag, 0);
            break;
        case 7:
            Analyze_RequestMultiplexEntry((PS_RequestMultiplexEntry)message, origin, tag, 0);
            break;
        case 8:
            Analyze_RequestMode((PS_RequestMode)message, origin, tag, 0);
            break;
        case 9:
            Analyze_RoundTripDelayRequest((PS_RoundTripDelayRequest)message, origin, tag, 0);
            break;
        case 10:
            Analyze_MaintenanceLoopRequest((PS_MaintenanceLoopRequest)message, origin, tag, 0);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 11:
            Analyze_CommunicationModeRequest((PS_CommunicationModeRequest)message, origin, tag, 0);
            break;
        case 12:
            Analyze_ConferenceRequest((PS_ConferenceRequest)message, origin, tag, 0);
            break;
        case 13:
            Analyze_MultilinkRequest((PS_MultilinkRequest)message, origin, tag, 0);
            break;
        case 14:
            Analyze_LogicalChannelRateRequest((PS_LogicalChannelRateRequest)message, origin, tag, 0);
            break;
        default:
            ErrorMessage("RequestMessage_analyze: Illegal CHOICE index");
    }
}

/* ================================================= */
/* PER Analysis for ResponseMessage (RAN)             */
/* ================================================= */
void ResponseMessage_analyze(uint16 tag, uint8 type2, uint8* message)
{
    int8 origin[] = "responseMessage";
    switch (type2)
    {
        case 0:
            Analyze_NonStandardMessage((PS_NonStandardMessage)message, origin, tag, 0);
            break;
        case 1:
            Analyze_MasterSlaveDeterminationAck((PS_MasterSlaveDeterminationAck)message, origin, tag, 0);
            break;
        case 2:
            Analyze_MasterSlaveDeterminationReject((PS_MasterSlaveDeterminationReject)message, origin, tag, 0);
            break;
        case 3:
            Analyze_TerminalCapabilitySetAck((PS_TerminalCapabilitySetAck)message, origin, tag, 0);
            break;
        case 4:
            Analyze_TerminalCapabilitySetReject((PS_TerminalCapabilitySetReject)message, origin, tag, 0);
            break;
        case 5:
            Analyze_OpenLogicalChannelAck((PS_OpenLogicalChannelAck)message, origin, tag, 0);
            break;
        case 6:
            Analyze_OpenLogicalChannelReject((PS_OpenLogicalChannelReject)message, origin, tag, 0);
            break;
        case 7:
            Analyze_CloseLogicalChannelAck((PS_CloseLogicalChannelAck)message, origin, tag, 0);
            break;
        case 8:
            Analyze_RequestChannelCloseAck((PS_RequestChannelCloseAck)message, origin, tag, 0);
            break;
        case 9:
            Analyze_RequestChannelCloseReject((PS_RequestChannelCloseReject)message, origin, tag, 0);
            break;
        case 10:
            Analyze_MultiplexEntrySendAck((PS_MultiplexEntrySendAck)message, origin, tag, 0);
            break;
        case 11:
            Analyze_MultiplexEntrySendReject((PS_MultiplexEntrySendReject)message, origin, tag, 0);
            break;
        case 12:
            Analyze_RequestMultiplexEntryAck((PS_RequestMultiplexEntryAck)message, origin, tag, 0);
            break;
        case 13:
            Analyze_RequestMultiplexEntryReject((PS_RequestMultiplexEntryReject)message, origin, tag, 0);
            break;
        case 14:
            Analyze_RequestModeAck((PS_RequestModeAck)message, origin, tag, 0);
            break;
        case 15:
            Analyze_RequestModeReject((PS_RequestModeReject)message, origin, tag, 0);
            break;
        case 16:
            Analyze_RoundTripDelayResponse((PS_RoundTripDelayResponse)message, origin, tag, 0);
            break;
        case 17:
            Analyze_MaintenanceLoopAck((PS_MaintenanceLoopAck)message, origin, tag, 0);
            break;
        case 18:
            Analyze_MaintenanceLoopReject((PS_MaintenanceLoopReject)message, origin, tag, 0);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 19:
            Analyze_CommunicationModeResponse((PS_CommunicationModeResponse)message, origin, tag, 0);
            break;
        case 20:
            Analyze_ConferenceResponse((PS_ConferenceResponse)message, origin, tag, 0);
            break;
        case 21:
            Analyze_MultilinkResponse((PS_MultilinkResponse)message, origin, tag, 0);
            break;
        case 22:
            Analyze_LogicalChannelRateAcknowledge((PS_LogicalChannelRateAcknowledge)message, origin, tag, 0);
            break;
        case 23:
            Analyze_LogicalChannelRateReject((PS_LogicalChannelRateReject)message, origin, tag, 0);
            break;
        default:
            ErrorMessage("ResponseMessage_analyze: Illegal CHOICE index");
    }
}

/* ================================================= */
/* PER Analysis for CommandMessage (RAN)             */
/* ================================================= */
void CommandMessage_analyze(uint16 tag, uint8 type2, uint8* message)
{
    int8 origin[] = "commandMessage";
    switch (type2)
    {
        case 0:
            Analyze_NonStandardMessage((PS_NonStandardMessage)message, origin, tag, 0);
            break;
        case 1:
            Analyze_MaintenanceLoopOffCommand((PS_MaintenanceLoopOffCommand)message, origin, tag, 0);
            break;
        case 2:
            Analyze_SendTerminalCapabilitySet((PS_SendTerminalCapabilitySet)message, origin, tag, 0);
            break;
        case 3:
            Analyze_EncryptionCommand((PS_EncryptionCommand)message, origin, tag, 0);
            break;
        case 4:
            Analyze_FlowControlCommand((PS_FlowControlCommand)message, origin, tag, 0);
            break;
        case 5:
            Analyze_EndSessionCommand((PS_EndSessionCommand)message, origin, tag, 0);
            break;
        case 6:
            Analyze_MiscellaneousCommand((PS_MiscellaneousCommand)message, origin, tag, 0);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 7:
            Analyze_CommunicationModeCommand((PS_CommunicationModeCommand)message, origin, tag, 0);
            break;
        case 8:
            Analyze_ConferenceCommand((PS_ConferenceCommand)message, origin, tag, 0);
            break;
        case 9:
            Analyze_H223MultiplexReconfiguration((PS_H223MultiplexReconfiguration)message, origin, tag, 0);
            break;
        case 10:
            Analyze_NewATMVCCommand((PS_NewATMVCCommand)message, origin, tag, 0);
            break;
        default:
            ErrorMessage("CommandMessage_analyze: Illegal CHOICE index");
    }
}

/* ================================================= */
/* PER Analysis for IndicationMessage (RAN)          */
/* ================================================= */
void IndicationMessage_analyze(uint16 tag, uint8 type2, uint8* message)
{
    int8 origin[] = "indicationMessage";
    switch (type2)
    {
        case 0:
            Analyze_NonStandardMessage((PS_NonStandardMessage)message, origin, tag, 0);
            break;
        case 1:
            Analyze_FunctionNotUnderstood((PS_FunctionNotUnderstood)message, origin, tag, 0);
            break;
        case 2:
            Analyze_MasterSlaveDeterminationRelease((PS_MasterSlaveDeterminationRelease)message, origin, tag, 0);
            break;
        case 3:
            Analyze_TerminalCapabilitySetRelease((PS_TerminalCapabilitySetRelease)message, origin, tag, 0);
            break;
        case 4:
            Analyze_OpenLogicalChannelConfirm((PS_OpenLogicalChannelConfirm)message, origin, tag, 0);
            break;
        case 5:
            Analyze_RequestChannelCloseRelease((PS_RequestChannelCloseRelease)message, origin, tag, 0);
            break;
        case 6:
            Analyze_MultiplexEntrySendRelease((PS_MultiplexEntrySendRelease)message, origin, tag, 0);
            break;
        case 7:
            Analyze_RequestMultiplexEntryRelease((PS_RequestMultiplexEntryRelease)message, origin, tag, 0);
            break;
        case 8:
            Analyze_RequestModeRelease((PS_RequestModeRelease)message, origin, tag, 0);
            break;
        case 9:
            Analyze_MiscellaneousIndication((PS_MiscellaneousIndication)message, origin, tag, 0);
            break;
        case 10:
            Analyze_JitterIndication((PS_JitterIndication)message, origin, tag, 0);
            break;
        case 11:
            Analyze_H223SkewIndication((PS_H223SkewIndication)message, origin, tag, 0);
            break;
        case 12:
            Analyze_NewATMVCIndication((PS_NewATMVCIndication)message, origin, tag, 0);
            break;
        case 13:
            Analyze_UserInputIndication((PS_UserInputIndication)message, origin, tag, 0);
            break;
            /* ------------------------------- */
            /* ---- Extension Begins Here ---- */
            /* ------------------------------- */
        case 14:
            Analyze_H2250MaximumSkewIndication((PS_H2250MaximumSkewIndication)message, origin, tag, 0);
            break;
        case 15:
            Analyze_MCLocationIndication((PS_MCLocationIndication)message, origin, tag, 0);
            break;
        case 16:
            Analyze_ConferenceIndication((PS_ConferenceIndication)message, origin, tag, 0);
            break;
        case 17:
            Analyze_VendorIdentification((PS_VendorIdentification)message, origin, tag, 0);
            break;
        case 18:
            Analyze_FunctionNotSupported((PS_FunctionNotSupported)message, origin, tag, 0);
            break;
        case 19:
            Analyze_MultilinkIndication((PS_MultilinkIndication)message, origin, tag, 0);
            break;
        case 20:
            Analyze_LogicalChannelRateRelease((PS_LogicalChannelRateRelease)message, origin, tag, 0);
            break;
        case 21:
            Analyze_FlowControlIndication((PS_FlowControlIndication)message, origin, tag, 0);
            break;
        default:
            ErrorMessage("IndicationMessage_analyze: Illegal CHOICE index");
    }
}

/*********************************************/
/* Text Descriptions of Codewords	         */
/* (For printing to codeword.txt)            */
/*********************************************/
char *EncodeDescription[ 4][ 24 ] =
{
    /* Request Message */
    {   "NonStandardMessage",		/* 0: NonStandardMessage		    */
        "MasterSlaveDetermination",	/* 1: MasterSlaveDetermination	    */
        "TerminalCapabilitySet",		/* 2: TerminalCapabilitySet		    */
        "OpenLogicalChannel",		/* 3: OpenLogicalChannel 		    */
        "CloseLogicalChannel",		/* 4: CloseLogicalChannel		    */
        "RequestChannelClose",		/* 5: RequestChannelClose		    */
        "MultiplexEntrySend",		/* 6: MultiplexEntrySend		    */
        "RequestMultiplexEntry",		/* 7: RequestMultiplexEntry		    */
        "RequestMode",				/* 8: RequestMode			    */
        "RoundTripDelayRequest",		/* 9: RoundTripDelayRequest		    */
        "MaintenanceLoopRequest",	/* A: MaintenanceLoopRequest	    */
        "CommunicationModeRequest",	/* B: CommunicationModeRequest	    */
        "ConferenceRequest",			/* C: ConferenceRequest		    */
        "H223AnnexAReConfiguration", /* D: H223AnnexAReconfiguration	    */
        "(Not Defined)",					    /* E: ( Not Define  )		    */
        "(Not Defined)",					    /* 10: ( Not Define  )		    */
        "(Not Defined)",					    /* 11: ( Not Define  )		    */
        "(Not Defined)",					    /* 12: ( Not Define  )		    */
        "(Not Defined)",					    /* 13: ( Not Define  )		    */
        "(Not Defined)",					    /* 14: ( Not Define  )		    */
        "(Not Defined)",					    /* 15: ( Not Define  )		    */
        "(Not Defined)" },				    /* 16: ( Not Define  )		    */

    /* Response Messeage */
    {   "NonStandardMessage",			    /* 0: NonStandradMessage		    */
        "MasterSlaveDeterminationAck",             /* 1: MasterSlaveDeterminationAck	    */
        "MasterSlaveDeterminationReject",          /* 2: MasterSlaveDeterminationReject    */
        "TerminalCapabilitySetAck",                /* 3: TerminalCapabilitySetAck	    */
        "TerminalCapabilitySetReject",             /* 4: TerminalCapabilitySetReject	    */
        "OpenLogicalChannelAck",                   /* 5: OpenLogicalChannelAck		    */
        "OpenLogicalChannelReject",                /* 6: OpenLogicalChannelReject	    */
        "CloseLogicalChannelAck",                  /* 7: CloseLogicalChannelAck	    */
        "RequestChannelCloseAck",                  /* 8: RequestChannelCloseAck	    */
        "RequestChannelCloseReject",               /* 9: RequestChannelCloseReject	    */
        "MultiplexEntrySendAck",                   /* A: MultiplexEntrySendAck		    */
        "MultiplexEntrySendReject",                /* B: MultiplexEntrySendReject	    */
        "RequestMultiplexEntryAck",                /* C: RequestMultiplexEntryAck	    */
        "RequestMultiplexEntryReject",	    	    /* D: RequestMultiplexEntryReject	    */
        "RequestModeAck",                          /* E: RequestModeAck		    */
        "RequestModeReject",                       /* F RequestModeReject		    */
        "RoundTripDelayResponse",                  /* 10 RoundTripDelayResponse	    */
        "MaintenanceLoopAck",                      /* 11 MaintenanceLoopAck		    */
        "MaintenanceLoopReject",                   /* 12 MaintenanceLoopReject		    */
        "CommunicationModeResponse",               /* 13 CommunicationModeResponse	    */
        "ConferenceResponse",		    	    /* 14 ConferenceResponse		    */
        "H223AnnexAReConfigurationAck",	    /* 15 H223AnnexAReconfigurationAck	    */
        "H223AnnexAReConfigurationReject" },	    /* 16 H223AnnexAReconfigurationReject   */

    /* Command Messeage */
    {   "NonStandardMessage",			    /* 0: NonStandradMessage		    */
        "MaintenanceLoopOffCommand",               /* 1: MaintenanceLoopOffCommand	    */
        "SendTerminalCapabilitySet",               /* 2: SendTerminalCapabilitySet	    */
        "EncryptionCommand",                       /* 3: EncryptionCommand		    */
        "FlowControlCommand",                      /* 4: FlowControlCommand		    */
        "EndSessionCommand",                       /* 5: EndSessionCommand		    */
        "MiscellaneousCommand",                    /* 6: MiscellaneousCommand		    */
        "CommunicationModeCommand",                /* 7: CommunicationModeCommand	    */
        "ConferenceCommand",			    /* 8: ConferenceCommand		    */
        "H223MultiplexReconfiguration",
        "(Not Defined)",					    /* B:  ( Not Define  )		    */
        "(Not Defined)",					    /* C:  ( Not Define  )		    */
        "(Not Defined)",					    /* D:  ( Not Define  )		    */
        "(Not Defined)",					    /* E:  ( Not Define  )		    */
        "(Not Defined)",					    /* F:  ( Not Define  )		    */
        "(Not Defined)",					    /* 10: ( Not Define  )		    */
        "(Not Defined)",					    /* 11: ( Not Define  )		    */
        "(Not Defined)",					    /* 12: ( Not Define  )		    */
        "(Not Defined)",					    /* 13: ( Not Define  )		    */
        "(Not Defined)",					    /* 14: ( Not Define  )		    */
        "(Not Defined)",					    /* 15: ( Not Define  )		    */
        "(Not Defined)" },				    /* 16: ( Not Define  )		    */

    /* Indication Message */
    {   "NonStandardMessage",			    /* 0: NonStandradMessage		    */
        "FunctionNotUnderstood",                   /* 1: FunctionNotUnderstood		    */
        "MasterSlaveDeterminationRelease",         /* 2: MasterSlaveDeterminationRelease   */
        "TerminalCapabilitySetRelease",            /* 3: TerminalCapabilitySetRelease	    */
        "OpenLogicalChannelConfirm",               /* 4: OpenLogicalChannelConfirm	    */
        "RequestChannelCloseRelease",              /* 5: RequestChannelCloseRelease	    */
        "MultiplexEntrySendRelease",               /* 6: MultiplexEntrySendRelease	    */
        "RequestMultiplexEntryRelease",            /* 7: RequestMultiplexEntryRelease	    */
        "RequestModeRelease",                      /* 8: RequestModeRelease		    */
        "MiscellaneousIndication",		    /* 9: MiscellaneousIndication	    */
        "JitterIndication",                        /* A: JitterIndication		    */
        "H223SkewIndication",                      /* B: H223SkewIndication		    */
        "NewATMVCIndication",                      /* C: NewATMVCIndication		    */
        "UserInputIndication",                     /* D: UserInputIndication		    */
        "H2250MaximumSkewIndication",              /* E: H2250MaximumSkewIndication	    */
        "MCLocationIndication",                    /* F: MCLocationIndication		    */
        "ConferenceIndication",                    /* 10:ConferenceIndication		    */
        "VendorIdentification",                    /* 11:VendorIndication		    */
        "FunctionNotSupported",		    /* 12:FunctionNotSupported		    */
        "(Not Defined)",					    /* 13: ( Not Define  )		    */
        "(Not Defined)",					    /* 14: ( Not Define  )		    */
        "FlowControlCommand",	/* WWU_IND 15: FlowControl		*/
        "(Not Defined)" }					    /* 16: ( Not Define  )		    */
};

/* ================================================= */
/* Display an H.245 Message Bitstream as Hex         */
/* ================================================= */
void ShowH245Hex(uint16 tag, uint8 type1, uint8 type2, uint16 size, uint8* data)
{
    Show245(tag, 0, "<============= BEGIN H.245 MESSAGE (HEX) =============>");
    Show245(tag, 0, EncodeDescription[type1][type2]);
    ShowHexData(tag, 2, size, data);
    Show245(tag, 0, "<============== END H.245 MESSAGE (HEX) ==============>");
    Show245(tag, 0, " ");
}

#endif  /* end PVANALYZER */
