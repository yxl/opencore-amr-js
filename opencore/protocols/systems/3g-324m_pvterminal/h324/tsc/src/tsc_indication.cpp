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

#include    "tscmain.h"
#include    "tsc_sub.h"     /* Sub Routine Information Header                */
#include    "tsc_component.h"

#define MAX_VENDOR_ID_OID_LEN 512 /* Max len of object identifier sequence */
/*****************************************************************************/
/*  function name        : Tsc_IdcVi                                         */
/*  function outline     : VendorIdentification procedure                    */
/*  function discription : Tsc_IdcVi( void )                                 */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.11.29                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::Tsc_IdcVi(void)
{
    S_ControlMsgHeader infHeader;
    PS_VendorIdentification      pVendorIdentification;

    if (iVendor == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m:Tsc_IdcVi - Not sending vendor id\n"));
        return;
    }
    /* Buffer Allocate */
    pVendorIdentification = (PS_VendorIdentification)OSCL_DEFAULT_MALLOC(sizeof(S_VendorIdentification));
    oscl_memset(pVendorIdentification, 0, sizeof(S_VendorIdentification));
    if (iVendor->GetVendorType() == EObjectIdentifier)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m:Tsc_IdcVi - EObjectIdentifier\n"));
        pVendorIdentification->vendor.index = 0; /* object */
        pVendorIdentification->vendor.object = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
        uint16 sz = 0;
        uint8* data = ((TPVH245VendorObjectIdentifier*)iVendor)->GetVendor(&sz);
        pVendorIdentification->vendor.object->size = sz;
        pVendorIdentification->vendor.object->data = (uint8*)OSCL_DEFAULT_MALLOC(sz);
        oscl_memcpy(pVendorIdentification->vendor.object->data, data, sz);
    }
    else if (iVendor->GetVendorType() == EH221NonStandard)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m:Tsc_IdcVi - EH221NonStandard\n"));
        pVendorIdentification->vendor.index = 1; /* H221Nonstandard */
        pVendorIdentification->vendor.h221NonStandard =
            (struct _H221NonStandard *)OSCL_DEFAULT_MALLOC(sizeof(struct _H221NonStandard));
        pVendorIdentification->vendor.h221NonStandard->manufacturerCode =
            (uint16)((TPVVendorH221NonStandard*)iVendor)->GetManufacturerCode();
        pVendorIdentification->vendor.h221NonStandard->t35CountryCode =
            ((TPVVendorH221NonStandard*)iVendor)->GetT35CountryCode();
        pVendorIdentification->vendor.h221NonStandard->t35Extension =
            ((TPVVendorH221NonStandard*)iVendor)->GetT35Extension();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m:Tsc_IdcVi - ERROR - Invalid vendor type\n"));
        OSCL_DEFAULT_FREE(pVendorIdentification);
        return;
    }
    pVendorIdentification->option_of_productNumber = false;
    if (iProductNumberLen)
    {
        pVendorIdentification->option_of_productNumber = true;
        pVendorIdentification->productNumber.size =
            (uint16)iProductNumberLen;
        pVendorIdentification->productNumber.data =
            (uint8*)OSCL_DEFAULT_MALLOC(iProductNumberLen);
        oscl_memcpy(pVendorIdentification->productNumber.data,
                    iProductNumber, iProductNumberLen);
    }
    pVendorIdentification->option_of_versionNumber = false;
    if (iVersionNumberLen)
    {
        pVendorIdentification->option_of_versionNumber = true;
        pVendorIdentification->versionNumber.size =
            (uint16)iVersionNumberLen;
        pVendorIdentification->versionNumber.data =
            (uint8*)OSCL_DEFAULT_MALLOC(iVersionNumberLen);
        oscl_memcpy(pVendorIdentification->versionNumber.data,
                    iVersionNumber, iVersionNumberLen);
    }

    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_Vi, 0, 0,
                    (uint8*)pVendorIdentification, sizeof(S_VendorIdentification));
    /* Primitive Send */
    iH245->DispatchControlMessage(&infHeader);
    Delete_VendorIdentification(pVendorIdentification);
    OSCL_DEFAULT_FREE(pVendorIdentification);
    return;
}

uint32 TSC_324m::VendorIdRecv(PS_ControlMsgHeader  pReceiveInf)
{
    PS_VendorIdentification  pVendorIdentification =
        (PS_VendorIdentification)pReceiveInf->pParameter;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: Vendor Id Received - vendor index(%d), opn(%d), ovn(%d)\n",
                     pVendorIdentification->vendor.index,
                     pVendorIdentification->option_of_productNumber,
                     pVendorIdentification->option_of_versionNumber));
    if (iVendorR)
    {
        OSCL_DELETE(iVendorR);
        iVendorR = NULL;
    }

    if (iProductNumberR)
    {
        OSCL_DEFAULT_FREE(iProductNumberR);
        iProductNumberR = NULL;
        iProductNumberLenR = 0;
    }
    if (iVersionNumberR)
    {
        OSCL_DEFAULT_FREE(iVersionNumberR);
        iVersionNumberR = NULL;
        iVersionNumberLenR = 0;
    }

    if (pVendorIdentification->vendor.index == 0) // object identifier
    {
        if (pVendorIdentification->vendor.object->size &&
                (pVendorIdentification->vendor.object->size < MAX_VENDOR_ID_OID_LEN))
        {
            if (pVendorIdentification->vendor.object->data[pVendorIdentification->vendor.object->size - 1] == '\0')
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TSC_324m: Vendor Id - %s\n",
                                 pVendorIdentification->vendor.object->data));
            }
            iVendorR = new TPVH245VendorObjectIdentifier((uint8*)pVendorIdentification->vendor.object->data,
                    pVendorIdentification->vendor.object->size);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m:VendorIdRecv - Error - invalid length(%d)\n",
                             pVendorIdentification->vendor.object->size));
            return iTerminalStatus;
        }
    }
    else if (pVendorIdentification->vendor.index == 1) // H221NonStandard
    {
        iVendorR = new TPVVendorH221NonStandard(pVendorIdentification->vendor.h221NonStandard->t35CountryCode,
                                                pVendorIdentification->vendor.h221NonStandard->t35Extension,
                                                pVendorIdentification->vendor.h221NonStandard->manufacturerCode);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m:VendorIdRecv - Error - invalid index(%d)\n",
                         pVendorIdentification->vendor.index));
        return iTerminalStatus;
    }
    if (pVendorIdentification->option_of_productNumber)
    {
        iProductNumberLenR = pVendorIdentification->productNumber.size;
    }
    if (iProductNumberLenR)
    {
        if (pVendorIdentification->productNumber.data[iProductNumberLenR-1] == '\0')
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: Product Number - %s\n",
                             pVendorIdentification->productNumber.data));
        }
        iProductNumberR = (uint8*)OSCL_DEFAULT_MALLOC(iProductNumberLenR);
        oscl_memcpy(iProductNumberR, pVendorIdentification->productNumber.data,
                    iProductNumberLenR);
    }
    if (pVendorIdentification->option_of_versionNumber)
    {
        iVersionNumberLenR = pVendorIdentification->versionNumber.size;
    }
    if (iVersionNumberLenR)
    {
        if (pVendorIdentification->versionNumber.data[iVersionNumberLenR-1] == '\0')
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: Version Number - %s\n",
                             pVendorIdentification->versionNumber.data));
        }
        iVersionNumberR = (uint8*)OSCL_DEFAULT_MALLOC(iVersionNumberLenR);
        oscl_memcpy(iVersionNumberR, pVendorIdentification->versionNumber.data, iVersionNumberLenR);
    }
    if (iTSC_324mObserver)
    {
        iTSC_324mObserver->IncomingVendorId(iVendorR, iProductNumberR,
                                            (uint16)iProductNumberLenR, iVersionNumberR, (uint16)iVersionNumberLenR);
    }
    return iTerminalStatus;
}


//-----------------------------------------------------------------------------
// Tsc_UII_DTMF()                                              (RAN-UII)
//
// This routine sends a DTMF signal via H.245 UserInputIndication message.
// It takes two input values:
//
//   uint8   dtmf_value      (ascii value of a character in [0123456789#*ABCD!])
//   uint16  dtmf_duration   (duration of pulse in mSec; 0 means no duration)
//-----------------------------------------------------------------------------
void TSC_324m::Tsc_UII_DTMF(uint8 dtmf_value, uint16 dtmf_duration)
{
    S_ControlMsgHeader			 infHeader;
    PS_UserInputIndication       pUserInputIndication;

    // Allocate the UII structure
    pUserInputIndication = (PS_UserInputIndication)OSCL_DEFAULT_MALLOC(sizeof(S_UserInputIndication));

    // Fill in the 'signal'
    pUserInputIndication->index = 3;  /* signal */
    pUserInputIndication->signal = (PS_Signal) OSCL_DEFAULT_MALLOC(sizeof(S_Signal));

    pUserInputIndication->signal->option_of_signalRtp = 0;
    pUserInputIndication->signal->signalType.size = 1;
    pUserInputIndication->signal->signalType.data = (uint8*) OSCL_DEFAULT_MALLOC(1 * sizeof(uint8));
    pUserInputIndication->signal->signalType.data[0] = dtmf_value;

    if (dtmf_duration == 0)
    {
        pUserInputIndication->signal->option_of_duration = 0;
    }
    else
    {
        pUserInputIndication->signal->option_of_duration = 1;
        pUserInputIndication->signal->duration = dtmf_duration;
    }

    // Send the message
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_Ui, 0, 0, (uint8*)pUserInputIndication,
                    sizeof(S_UserInputIndication));
    iH245->DispatchControlMessage(&infHeader);
    Delete_UserInputIndication(pUserInputIndication);
    OSCL_DEFAULT_FREE(pUserInputIndication);
    return;
}

void TSC_324m::Tsc_UII_Alphanumeric(const uint8* str, uint16 str_len)
{
    S_ControlMsgHeader			 infHeader;
    PS_UserInputIndication       pUserInputIndication;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m:Tsc_UII_Alphanumeric - len(%d)", str_len));

    // Allocate the UII structure
    pUserInputIndication = (PS_UserInputIndication)OSCL_DEFAULT_MALLOC(sizeof(S_UserInputIndication));

    // Fill in the 'signal'
    pUserInputIndication->index = 1;  /* alphanumeric */
    pUserInputIndication->alphanumeric = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
    pUserInputIndication->alphanumeric->size = str_len;
    pUserInputIndication->alphanumeric->data = (uint8*) OSCL_DEFAULT_MALLOC(str_len * sizeof(uint8));
    oscl_memcpy(pUserInputIndication->alphanumeric->data, str, str_len);

    // Send the message
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_Ui, 0, 0, (uint8*)pUserInputIndication,
                    sizeof(S_UserInputIndication));
    iH245->DispatchControlMessage(&infHeader);
    Delete_UserInputIndication(pUserInputIndication);
    OSCL_DEFAULT_FREE(pUserInputIndication);
    return;
}

uint32 TSC_324m::UserInputIndicationRecv(PS_ControlMsgHeader  pReceiveInf)
{
    PS_UserInputIndication pUserInputIndication  = (PS_UserInputIndication)pReceiveInf->pParameter;
    CPVUserInput* uii = NULL;
    uint16 duration = 0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m: User Input Indication Received - index(%d)\n", pUserInputIndication->index));
    switch (pUserInputIndication->index)
    {
        case 1: /* alphanumeric */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: User Input Indication Received - alphanumeric size(%d)",
                             pUserInputIndication->alphanumeric->size));
            uii = OSCL_NEW(CPVUserInputAlphanumeric, (pUserInputIndication->alphanumeric->data,
                           pUserInputIndication->alphanumeric->size));
            break;
        case 3: /* signal */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: User Input Indication Received - signal option_of_duration(%d), option_of_signalRtp(%d), signalType(%d)", pUserInputIndication->signal->option_of_duration, pUserInputIndication->signal->option_of_signalRtp, pUserInputIndication->signal->signalType.size));
            if (pUserInputIndication->signal->option_of_duration)
            {
                duration = pUserInputIndication->signal->duration;
            }
            uii = OSCL_NEW(CPVUserInputDtmf, (*pUserInputIndication->signal->signalType.data,
                                              false, duration));
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m: User Input Indication Received - unrecognized type\n"));
    }
    if (uii)
    {
        if (iTSC_324mObserver)
            iTSC_324mObserver->UserInputReceived(uii);
        OSCL_DELETE(uii);
    }

    return iTerminalStatus;
}

void TSC_324m::IndicationMisc(TIndicationMisc type,
                              TPVChannelId channelId,
                              uint32 param,
                              OsclAny* param1)
{
    OSCL_UNUSED_ARG(param1);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::IndicationMisc type(%d), channelId(%d), param(%d), param1(%x)",
                     type, channelId, param, param1));

    S_ControlMsgHeader infHeader;
    S_MiscellaneousIndication miscellaneousIndication;
    oscl_memset(&miscellaneousIndication, 0, sizeof(S_MiscellaneousIndication));

    switch (type)
    {
        case EVideoTemporalSpatialTradeOffIdc:
            miscellaneousIndication.logicalChannelNumber = (uint16)channelId;
            miscellaneousIndication.miType.index = 9;
            miscellaneousIndication.miType.videoTemporalSpatialTradeOff = (uint8)param;
            break;
        default:
            return;
    }

    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_Mscl, 0, 0,
                    (uint8*)&miscellaneousIndication, sizeof(S_MiscellaneousIndication));
    iH245->DispatchControlMessage(&infHeader);
}

void TSC_324m::Tsc_IdcSkew(TPVChannelId lcn1, TPVChannelId lcn2, uint16 skew)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::Tsc_IdcSkew lcn1(%d), lcn2(%d), skew(%d)",
                     lcn1, lcn2, skew));
    S_ControlMsgHeader infHeader;
    S_H223SkewIndication skewIndication;
    oscl_memset(&skewIndication, 0, sizeof(S_H223SkewIndication));
    skewIndication.logicalChannelNumber1 = (uint16)lcn1;
    skewIndication.logicalChannelNumber2 = (uint16)lcn2;
    skewIndication.skew = skew;

    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_H223skw, 0, 0,
                    (uint8*)&skewIndication, sizeof(S_H223SkewIndication));
    iH245->DispatchControlMessage(&infHeader);
}

uint32 TSC_324m::MiscIndicationRecv(PS_ControlMsgHeader  pReceiveInf)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MiscIndicationRecv"));
    PS_MiscellaneousIndication indication = (PS_MiscellaneousIndication)pReceiveInf->pParameter;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MiscIndicationRecv lcn(%d),index(%d)",
                     indication->logicalChannelNumber, indication->miType.index));
    switch (indication->miType.index)
    {
        case 9:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::MiscIndicationRecv videoSpatialTemporalTradeoff(%d)",
                             indication->miType.videoTemporalSpatialTradeOff));
            if (iTSC_324mObserver)
            {
                iTSC_324mObserver->VideoSpatialTemporalTradeoffIndicationReceived(indication->logicalChannelNumber,
                        indication->miType.videoTemporalSpatialTradeOff);
            }
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_324m::MiscIndicationRecv Indication not handled"));
            break;
    }
    return iTerminalStatus;
}

uint32 TSC_324m::SkewIndicationRecv(PS_ControlMsgHeader  pReceiveInf)
{
    PS_H223SkewIndication indication = (PS_H223SkewIndication)pReceiveInf->pParameter;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SkewIndicationRecv lcn1(%d), lcn2(%d), skew(%d)",
                     indication->logicalChannelNumber1,
                     indication->logicalChannelNumber2,
                     indication->skew));
    uint16 skew = indication->skew;
    if (skew > PV_2WAY_MAX_SKEW_MS)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_324m::SkewIndicationRecv skew(%d) > max skew(%d)",
                         indication->skew, PV_2WAY_MAX_SKEW_MS));
        skew = PV_2WAY_MAX_SKEW_MS;
    }

    /* Validate the skew indication */
    OlcParam* olcparam = iTSCcomponent->FindOlc(INCOMING, PV_AUDIO, OLC_ESTABLISHED);
    if (!olcparam)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SkewIndicationRecv Established incoming audio lcn not found"));
        return iTerminalStatus ;
    }
    if (indication->logicalChannelNumber2 != olcparam->GetChannelId())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SkewIndicationRecv indication->logicalChannelNumber2=%d != audio lcn id=%d",
                         indication->logicalChannelNumber2, olcparam->GetChannelId()));
        //return iTerminalStatus ;
    }

    olcparam = iTSCcomponent->FindOlc(INCOMING, PV_VIDEO, OLC_ESTABLISHED);
    if (!olcparam)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SkewIndicationRecv Established incoming video lcn not found"));
        return iTerminalStatus ;
    }
    if (indication->logicalChannelNumber1 != olcparam->GetChannelId())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_324m::SkewIndicationRecv indication->logicalChannelNumber1=%d != video lcn id=%d",
                         indication->logicalChannelNumber1, olcparam->GetChannelId()));
        //return iTerminalStatus ;
    }

    if (iTSC_324mObserver)
    {
        iTSC_324mObserver->SkewIndicationReceived(indication->logicalChannelNumber1,
                indication->logicalChannelNumber2, skew);
    }
    //Obtain logical channel #1, and transmit the skew duration
    OsclSharedPtr<H223IncomingChannel> channel;
    PVMFStatus retVal = iH223->GetIncomingChannel(indication->logicalChannelNumber1, channel);

    if (retVal != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "TSC_324m::SkewIndicationRecv Error - Failed to lookup logical channel %d", indication->logicalChannelNumber1));
        return iTerminalStatus;
    }

    channel->SetTimestampOffset(skew);

    return iTerminalStatus;
}

void TSC_324m::SendFunctionNotSupportedIndication(uint16 cause,
        uint8* function,
        uint16 len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendFunctionNotSupportedIndication"));
    S_ControlMsgHeader infHeader;
    S_FunctionNotSupported fnsIndication;
    oscl_memset(&fnsIndication, 0, sizeof(S_FunctionNotSupported));
    fnsIndication.fnsCause.index = cause;
    if (function && len)
    {
        fnsIndication.option_of_returnedFunction = 1;
        fnsIndication.returnedFunction.size = len;
        fnsIndication.returnedFunction.data = function;
    }
    Tsc_SendDataSet(&infHeader, H245_PRIMITIVE, E_PtvId_Idc_Fns, 0, 0,
                    (uint8*)&fnsIndication, sizeof(S_FunctionNotSupported));
    iH245->DispatchControlMessage(&infHeader);
    return;
}

uint32 TSC_324m::FunctionNotSupportedIndicationReceived(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::FunctionNotSupportedIndicationReceived"));
    return iTerminalStatus;
}

uint32 TSC_324m::FlowControlIndicationReceived(PS_ControlMsgHeader  pReceiveInf)
{
    OSCL_UNUSED_ARG(pReceiveInf);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::FlowControlIndicationReceived"));
    return iTerminalStatus;
}

