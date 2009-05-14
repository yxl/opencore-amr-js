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
/*  file name            : tsc_command.c                                     */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include    "oscl_base.h"
#include    "h245def.h"
#include    "tsc_sub.h"         /* Sub Routine Information Header            */
#include	"tscmain.h"

/*****************************************************************************/
/*  function name        : Tsc_EndSessionCommand                             */
/*  function outline     : Command (Change or EndSession) procedure          */
/*  function discription : Tsc_EndSessionCommand( void )                     */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::EndSessionCommand()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::EndSessionCommand"));
    S_ControlMsgHeader infHeader;
    S_EndSessionCommand endSessionCommand;

    oscl_memset(&endSessionCommand, 0, sizeof(S_EndSessionCommand));
    endSessionCommand.index = 1;  // disconnect

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Es,
                    0,
                    0,
                    (uint8*)&endSessionCommand,
                    sizeof(S_EndSessionCommand));
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_CmdStcs                                       */
/*  function outline     : Command (SendTerminalCapabilitySet) procedure     */
/*  function discription : Tsc_CmdStcs( void )                               */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
OSCL_EXPORT_REF void TSC_324m::SendTerminalCapabilitySetCommand()
{
    S_ControlMsgHeader infHeader;
    S_SendTerminalCapabilitySet sendTerminalCapabilitySet;

    oscl_memset(&sendTerminalCapabilitySet, 0,
                sizeof(S_SendTerminalCapabilitySet));

    sendTerminalCapabilitySet.index = 1; // genericRequest

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Stcs,
                    0,
                    0,
                    (uint8*)&sendTerminalCapabilitySet,
                    sizeof(S_SendTerminalCapabilitySet));
    iH245->DispatchControlMessage(&infHeader);
}


/*****************************************************************************/
/*  function name        : Tsc_CmdEcrpt                                      */
/*  function outline     : Command (Encryption) procedure                    */
/*  function discription : Tsc_CmdEcrpt( void )                              */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::CmdEcrpt()
{
    S_ControlMsgHeader infHeader;
    S_EncryptionCommand encryptionCommand;

    oscl_memset(&encryptionCommand, 0, sizeof(S_EncryptionCommand));

    encryptionCommand.index = 1; // encryptionIVRequest

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Ecrpt,
                    0,
                    0,
                    (uint8*)&encryptionCommand,
                    sizeof(S_EncryptionCommand));
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_CmdFc                                         */
/*  function outline     : Command (FlowControl) procedure                   */
/*  function discription : Tsc_CmdFc( void )                                 */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::CmdFc()
{
    S_ControlMsgHeader infHeader;
    S_FlowControlCommand flowControlCommand;

    oscl_memset(&flowControlCommand, 0, sizeof(S_FlowControlCommand));

    /// ORIGINAL CODEWORD : {wholeMultiplex, noRestriction}
    //pFlowControlCommand->fccScope.index = 2;  // wholeMultiplex
    //pFlowControlCommand->fccRestriction.index = 1; // noRestriction

    // ALTERNATE CODEWORD: {logicalChannelNumber=3,maximumBitRate=1000}
    flowControlCommand.fccScope.index = 0;  // logicalChannelNumber
    flowControlCommand.fccScope.logicalChannelNumber = 3;
    flowControlCommand.fccRestriction.index = 0; // maximumBitRate
    flowControlCommand.fccRestriction.maximumBitRate = 1000;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Fc,
                    0,
                    0,
                    (uint8*)&flowControlCommand,
                    sizeof(S_FlowControlCommand));
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_CmdHmr                                  (RAN) */
/*  function outline     : H223MultiplexReconfiguration procedure            */
/*---------------------------------------------------------------------------*/
/*  This routine initiates transmission of H223MultiplexReconfiguration.     */
/*  The option argument is in [0...5] corresponding to:                      */
/*    {toLevel0, toLevel1, toLevel2, toLevel2Hdr, dFlagStart, dFlagStop}     */
/*****************************************************************************/
void TSC_324m::CmdHmr(int32 option)
{
    S_ControlMsgHeader infHeader;
    S_H223MultiplexReconfiguration hmr;
    S_H223ModeChange modeChange;
    S_H223AnnexADoubleFlag doubleFlag;

    oscl_memset(&hmr, 0, sizeof(S_H223MultiplexReconfiguration));

    if (option < 4)
    {
        hmr.index = 0; // h223ModeChange
        hmr.h223ModeChange = &modeChange;
        hmr.h223ModeChange->index = (uint16)option;
    }
    else
    {
        hmr.index = 1; // h223AnnexADoubleFlag
        hmr.h223AnnexADoubleFlag = &doubleFlag;
        hmr.h223AnnexADoubleFlag->index = (uint16)(option - 4);
    }

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Hmr,
                    0,
                    0,
                    (uint8*)&hmr,
                    sizeof(S_H223MultiplexReconfiguration));
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_CmdMscl                                       */
/*  function outline     : Command (MiscellaneousCommands) procedure         */
/*  function discription : Tsc_CmdMscl( int32 )                                */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  NOTE: Input CommandIndex specifies one of the sample commands to transmit*/
/*        Currently these are:                                               */
/*          0 --> videoFastUpdatePicture for Lcn=3                           */
/*          1 --> videoTemporalSpatialTradeOff=29 for Lcn=3                  */
/*          2 --> maxH223MUXPDUsize=256 for Lcn=3                            */
/*****************************************************************************/
void TSC_324m::CmdMisc(TCmdMisc type, TPVChannelId channelId, uint32 param)
{
    S_ControlMsgHeader infHeader;
    S_MiscellaneousCommand miscellaneousCommand;

    oscl_memset(&miscellaneousCommand, 0, sizeof(S_MiscellaneousCommand));

    switch (type)
    {
        case EVideoFastUpdatePicture:
            miscellaneousCommand.logicalChannelNumber = (uint16)channelId;
            miscellaneousCommand.mcType.index = 5; // videoFastUpdatePicture
            break;
        case EVideoTemporalSpatialTradeOff:
            miscellaneousCommand.logicalChannelNumber = (uint16)channelId;
            miscellaneousCommand.mcType.index = 7; // videoTemporalSpatialTradeOff
            miscellaneousCommand.mcType.videoTemporalSpatialTradeOff = (uint8)param;
            break;
        case EMaxH223MUXPDUsize:
            miscellaneousCommand.logicalChannelNumber = (uint16)channelId;
            miscellaneousCommand.mcType.index = 11; // maxH223MUXPDUsize
            miscellaneousCommand.mcType.maxH223MUXPDUsize = (uint16)param;
            break;
        default:
            break;
    }

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Cmd_Mscl,
                    0,
                    0,
                    (uint8*)&miscellaneousCommand,
                    sizeof(S_MiscellaneousCommand));
    iH245->DispatchControlMessage(&infHeader);
}

/*****************************************************************************/
/*  function name        : Tsc_RtdTrfReq                                     */
/*  function outline     : Tsc_RtdTrfReq                                     */
/*  function discription : Tsc_RtdTrfReq( void )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::RtdTrfReq()
{
    S_ControlMsgHeader infHeader;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Rtd_Trf_Req,
                    0, 0, NULL, 0);

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    return;
}

OSCL_EXPORT_REF void TSC_324m::SendModeRequestCommand()
{
    S_ControlMsgHeader infHeader;
    PS_RequestMode          pParameter;
    PS_ModeDescription      pModeDescription;
    PS_ModeElement          pModeElement, pModeElement2;
    PS_VideoMode            pVideoMode; /* RAN */
    PS_AudioMode			pAudioMode;	/* WWURM */
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::SendModeRequestCommand"));

    // Buffer Allocate
    pParameter = (PS_RequestMode)OSCL_DEFAULT_MALLOC(sizeof(S_RequestMode));

    // TRANSFER.request primitive - Parameter[MODE-ELEMENT]
    pParameter->size_of_requestedModes = 1;

    // Buffer Allocate
    pModeDescription = (PS_ModeDescription)OSCL_DEFAULT_MALLOC(
                           sizeof(S_ModeDescription));

    // ModeDescription
    pModeDescription->size = 2;

    // Buffer Allocate
    pModeElement = (PS_ModeElement)OSCL_DEFAULT_MALLOC(
                       pModeDescription->size * sizeof(S_ModeElement));


    pModeElement->option_of_h223ModeParameters = false;
    pModeElement->option_of_v76ModeParameters = false;
    pModeElement->option_of_h2250ModeParameters = false;
    pModeElement->option_of_genericModeParameters = false;
    pModeElement->modeType.index = 1;  // videoMode
    pModeElement->modeType.videoMode =
        pVideoMode = (PS_VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_VideoMode));

    pVideoMode->index = 3;   // H.263 Video
    pVideoMode->h263VideoMode = (PS_H263VideoMode) OSCL_DEFAULT_MALLOC(sizeof(S_H263VideoMode));
    pVideoMode->h263VideoMode->h263Resolution.index = 1; // QCIF
    pVideoMode->h263VideoMode->bitRate = 480;       // 48000 bits/sec
    pVideoMode->h263VideoMode->unrestrictedVector = false;
    pVideoMode->h263VideoMode->arithmeticCoding = false;
    pVideoMode->h263VideoMode->advancedPrediction = false;
    pVideoMode->h263VideoMode->pbFrames = false;
    pVideoMode->h263VideoMode->option_of_errorCompensation = false;
    pVideoMode->h263VideoMode->option_of_enhancementLayerInfo = false;
    pVideoMode->h263VideoMode->option_of_h263Options = false;

    pModeElement2 = &pModeElement[1];
    pModeElement2->option_of_h223ModeParameters = false;
    pModeElement2->option_of_v76ModeParameters = false;
    pModeElement2->option_of_h2250ModeParameters = false;
    pModeElement2->option_of_genericModeParameters = false;
    pModeElement2->modeType.index = 2;  // audioMode
    pModeElement2->modeType.audioMode =

        pAudioMode = (PS_AudioMode) OSCL_DEFAULT_MALLOC(sizeof(S_AudioMode));
    PS_GenericCapability	genericCap;
    pAudioMode->index = 20;
    pAudioMode->genericAudioMode = genericCap =
                                       (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
    genericCap->capabilityIdentifier.index = 0;
    genericCap->capabilityIdentifier.standard =
        (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
    genericCap->capabilityIdentifier.standard->size = 7;
    genericCap->capabilityIdentifier.standard->data =
        (uint8*) OSCL_DEFAULT_MALLOC(7 * sizeof(uint8));
    genericCap->capabilityIdentifier.standard->data[0] = 0x00;
    genericCap->capabilityIdentifier.standard->data[1] = 0x08;
    genericCap->capabilityIdentifier.standard->data[2] = 0x81;
    genericCap->capabilityIdentifier.standard->data[3] = 0x75;
    genericCap->capabilityIdentifier.standard->data[4] = 0x01;
    genericCap->capabilityIdentifier.standard->data[5] = 0x01;
    genericCap->capabilityIdentifier.standard->data[6] = 0x01;
    genericCap->option_of_maxBitRate = true;
    genericCap->maxBitRate = 122;
    genericCap->option_of_collapsing = true;
    genericCap->size_of_collapsing = 1;
    genericCap->collapsing =
        (PS_GenericParameter) OSCL_DEFAULT_MALLOC(sizeof(S_GenericParameter));
    genericCap->collapsing[0].parameterIdentifier.index = 0;
    genericCap->collapsing[0].parameterIdentifier.standard = 0;
    genericCap->collapsing[0].parameterValue.index = 2;
    genericCap->collapsing[0].parameterValue.unsignedMin = 1;
    genericCap->collapsing[0].option_of_supersedes = false;
    genericCap->option_of_nonCollapsing = true;
    genericCap->size_of_nonCollapsing = 1;
    genericCap->nonCollapsing =
        (PS_GenericParameter) OSCL_DEFAULT_MALLOC(sizeof(S_GenericParameter));
    genericCap->nonCollapsing[0].parameterIdentifier.index = 0;
    genericCap->nonCollapsing[0].parameterIdentifier.standard = 1;
    genericCap->nonCollapsing[0].parameterValue.index = 2;
    genericCap->nonCollapsing[0].parameterValue.unsignedMin = 1;
    genericCap->nonCollapsing[0].option_of_supersedes = false;

    genericCap->option_of_nonCollapsingRaw = false;
    genericCap->option_of_transport = false;


    // Link the sub-objects together
    pModeDescription->item = pModeElement;
    pParameter->requestedModes = pModeDescription;

    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mr_Trf_Req,
                    0,
                    0,
                    (uint8*)pParameter,
                    sizeof(S_RequestMode));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    Delete_RequestMode(pParameter);
    OSCL_DEFAULT_FREE(pParameter);

    return;
}

/*****************************************************************************/
/*  function name        : MrRjtReq                                     */
/*  function outline     : MrRjtReq                                     */
/*  function discription : MrRjtReq( void )                             */
/*  input data           : None                                              */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void TSC_324m::MrRjtReq()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_324m::MrRjtReq"));
    S_ControlMsgHeader infHeader;
    S_RmRejectCause     parameter;
    oscl_memset(&parameter, 0, sizeof(S_RmRejectCause));

    // REJECT.request primitive - Parameter[CAUSE]
    parameter.index = 2; // requestDenied
    Tsc_SendDataSet(&infHeader,
                    H245_PRIMITIVE,
                    E_PtvId_Mr_Rjt_Req,
                    0,
                    0,
                    (uint8*)&parameter,
                    sizeof(S_RmRejectCause));

    // Primitive Send
    iH245->DispatchControlMessage(&infHeader);

    return;

}

