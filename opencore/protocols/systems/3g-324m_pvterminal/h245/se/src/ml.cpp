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
/************************************************************************/
/*  file name       : seml.c                                            */
/*  file contents   : Maintenance Loop Signalling Entity Management     */
/*  draw            : '96.11.19                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "h245inf.h"
#include "semsgque.h"
#include "ml.h"


/************************************************************************/
/*  function name       : Se_Ml_0900_0000                               */
/*  function outline    : Event     LOOP.request                        */
/*                      : Status    Outgoing Not Looped                 */
/*  function discription: void Se_Ml_0900_0000( PS_MlRequestType )          */
/*  input data          : PS_MlRequestType pLoop_Type                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0900_0000(PS_MlRequestType pLoop_Type)
{
    StatusWrite(ML_OUTGOING_AWTING_RPS) ;
    MsgLoopReqSend(pLoop_Type) ;
    T102TimerStart() ;
}

/************************************************************************/
/*  function name       : Se_Ml_0901_0010                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Ml_0901_0010( PS_MlRejectCause )           */
/*  input data          : PS_MlRejectCause pCause_Ml                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0901_0010(PS_MlRejectCause pCause_Ml)
{
    OSCL_UNUSED_ARG(pCause_Ml);
    T102TimerStop() ;
    StatusWrite(ML_OUTGOING_NOT_LOOPED) ;
    MsgLoopOffSend() ;
}


/************************************************************************/
/*  function name       : Se_Ml_0901_0011                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Ml_0901_0011( PS_MlRejectCause )           */
/*  input data          : PS_MlRejectCause pCause_Ml                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0901_0011(PS_MlRejectCause pCause_Ml)
{
    StatusWrite(ML_INCOMING_NOT_LOOPED) ;
    MsgLoopRjtSend(pCause_Ml) ;
}


/************************************************************************/
/*  function name       : Se_Ml_0901_0020                               */
/*  function outline    : Event     RELEASE.request                     */
/*                      : Status    Outgoing Looped                     */
/*  function discription: void Se_Ml_0901_0020( PS_MlRejectCause )           */
/*  input data          : PS_MlRejectCause pCause_Ml                         */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0901_0020(PS_MlRejectCause pCause_Ml)
{
    OSCL_UNUSED_ARG(pCause_Ml);
    StatusWrite(ML_OUTGOING_NOT_LOOPED) ;
    MsgLoopOffSend() ;
}


/************************************************************************/
/*  function name       : Se_Ml_0902_0001                               */
/*  function outline    : Event     MaintenanceLoopRequest              */
/*                      : Status    Incoming Not Looped                 */
/*  function discription: void Se_Ml_0902_0001(                         */
/*                                         PS_MaintenanceLoopRequest )  */
/*  input data          : PS_MaintenanceLoopRequest                     */
/*                                              pMaintenanceLoopRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0902_0001(PS_MaintenanceLoopRequest pMaintenanceLoopRequest)
{
    LoopTypeWrite(&pMaintenanceLoopRequest->mlRequestType) ;
    StatusWrite(ML_INCOMING_AWTING_RPS) ;
    PtvLoopIdcSend(pMaintenanceLoopRequest) ;
}


/************************************************************************/
/*  function name       : Se_Ml_0902_0011                               */
/*  function outline    : Event     MaintenanceLoopRequest              */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Ml_0902_0001(                         */
/*                                         PS_MaintenanceLoopRequest )  */
/*  input data          : PS_MaintenanceLoopRequest                     */
/*                                              pMaintenanceLoopRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0902_0011(PS_MaintenanceLoopRequest pMaintenanceLoopRequest)
{
    S_MlRejectCause    cause;

    cause.index = 0 ; /* canNotPerformLoop */

    StatusWrite(ML_INCOMING_AWTING_RPS) ;
    PtvRlsIdcSend(Src_USER, &cause) ;
    LoopTypeWrite(&pMaintenanceLoopRequest->mlRequestType) ;
    PtvLoopIdcSend(pMaintenanceLoopRequest) ;
}


/************************************************************************/
/*  function name       : Se_Ml_0903_0011                               */
/*  function outline    : Event     LOOP.response                       */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Ml_0903_0011( void )                  */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0903_0011(void)
{
    StatusWrite(ML_INCOMING_LOOPED) ;
    MsgLoopAckSend() ;
}


/************************************************************************/
/*  function name       : Se_Ml_0904_0010                               */
/*  function outline    : Event     MaintenanceLoopAck                  */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Ml_0904_0010( PS_MaintenanceLoopAck ) */
/*  input data          : PS_MaintenanceLoopAck pMaintenanceLoopAck     */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0904_0010(PS_MaintenanceLoopAck pMaintenanceLoopAck)
{
    OSCL_UNUSED_ARG(pMaintenanceLoopAck);
    T102TimerStop() ;
    StatusWrite(ML_OUTGOING_LOOPED) ;
    PtvLoopCfmSend() ;
}


/************************************************************************/
/*  function name       : Se_Ml_0905_0010                               */
/*  function outline    : Event     MaintenanceLoopReject               */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Ml_0905_0010(                         */
/*                                         PS_MaintenanceLoopReject )   */
/*  input data          : PS_MaintenanceLoopReject                      */
/*                                               pMaintenanceLoopReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0905_0010(PS_MaintenanceLoopReject pMaintenanceLoopReject)
{
    T102TimerStop() ;
    StatusWrite(ML_OUTGOING_NOT_LOOPED) ;
    PtvRlsIdcSend(Src_USER , &pMaintenanceLoopReject->mlRejectCause) ;
}


/************************************************************************/
/*  function name       : Se_Ml_0905_0020                               */
/*  function outline    : Event     MaintenanceLoopReject               */
/*                      : Status    Outgoing LOOPED                     */
/*  function discription: void Se_Ml_0905_0020(                         */
/*                                         PS_MaintenanceLoopReject )   */
/*  input data          : PS_MaintenanceLoopReject                      */
/*                                               pMaintenanceLoopReject */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0905_0020(PS_MaintenanceLoopReject pMaintenanceLoopReject)
{
    StatusWrite(ML_OUTGOING_NOT_LOOPED) ;
    PtvErrIdcSend(ErrCode_Ml_A) ;
    PtvRlsIdcSend(Src_MLSE , &pMaintenanceLoopReject->mlRejectCause) ;
}


/************************************************************************/
/*  function name       : Se_Ml_0906_0010                               */
/*  function outline    : Event     Timer T102 Timeout                  */
/*                      : Status    Outgoing Awaiting Response          */
/*  function discription: void Se_Ml_0906_0010( int32 )                   */
/*  input data          : int32 Timer_Sequence_Number                     */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0906_0010(int32 Timer_Sequence_Number)
{
    S_MlRejectCause    cause;

    if (Timer_Sequence_Number == TmrSqcNumberRead())    /* Receive Timer Sequense Number Equal Timer Sequence Number */
    {
        StatusWrite(ML_OUTGOING_NOT_LOOPED) ;
        PtvErrIdcSend(ErrCode_Ml_B) ;
        MsgLoopOffSend() ;
        cause.index = 0 ;  /* canNotPerformLoop */

        PtvRlsIdcSend(Src_MLSE , &cause) ;
    }
}


/************************************************************************/
/*  function name       : Se_Ml_0907_0011                               */
/*  function outline    : Event     MaintenanceLoopOffCommand           */
/*                      : Status    Incoming Awaiting Response          */
/*  function discription: void Se_Ml_0907_0011( void )                  */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::_0907_0011(void)
{
    S_MlRejectCause    cause;

    cause.index = 0 ;  /* canNotPerformLoop */

    StatusWrite(ML_INCOMING_NOT_LOOPED) ;
    PtvRlsIdcSend(Src_USER , &cause) ;
}


/************************************************************************/
/*  function name       : Se_MlMsgLoopReqSend                           */
/*  function outline    : MaintenanceLoopRequest Send                   */
/*  function discription: void Se_MlMsgLoopReqSend( PS_MlRequestType )      */
/*  input data          : PS_MlRequestType pLoop_Type                       */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::MsgLoopReqSend(PS_MlRequestType pLoop_Type)
{
    S_MaintenanceLoopRequest   maintenanceLoopRequest ;
    S_H245Msg                  h245Msg ;

    oscl_memcpy((int8*)&maintenanceLoopRequest.mlRequestType, (int8*)pLoop_Type, sizeof(S_MlRequestType)) ;

    h245Msg.Type1 = H245_MSG_REQ ;
    h245Msg.Type2 = MSGTYP_ML_REQ ;
    h245Msg.pData = (uint8*) & maintenanceLoopRequest ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MlMsgLoopAckSend                           */
/*  function outline    : MaintenanceLoopAck Send                       */
/*  function discription: void Se_MlMsgLoopAckSend( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::MsgLoopAckSend(void)
{
    S_MaintenanceLoopAck   maintenanceLoopAck ;
    S_H245Msg              h245Msg ;

    PS_MlAckType temp = &(maintenanceLoopAck.mlAckType);

    LoopTypeRead(OSCL_STATIC_CAST(PS_MlRequestType, temp));

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_ML_ACK ;
    h245Msg.pData = (uint8*) & maintenanceLoopAck ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MlMsgLoopRjtSend                           */
/*  function outline    : MaintenanceLoopReject Send                    */
/*  function discription: void Se_MlMsgLoopRjtSend( PS_MlRejectCause )       */
/*  input data          : PS_MlRejectCause pCause                            */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::MsgLoopRjtSend(PS_MlRejectCause pCause)
{
    S_MaintenanceLoopReject    maintenanceLoopReject ;
    S_H245Msg                  h245Msg ;
    PS_MlRejectType temp = &(maintenanceLoopReject.mlRejectType);

    LoopTypeRead(OSCL_STATIC_CAST(PS_MlRequestType, temp));
    oscl_memcpy((int8*)&maintenanceLoopReject.mlRejectCause , (int8*)pCause , sizeof(S_MlRejectCause)) ;

    h245Msg.Type1 = H245_MSG_RPS ;
    h245Msg.Type2 = MSGTYP_ML_RJT ;
    h245Msg.pData = (uint8*) & maintenanceLoopReject ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MlMsgLoopOffSend                           */
/*  function outline    : MaintenanceLoopOff Send                       */
/*  function discription: void Se_MlMsgLoopOffSend( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::MsgLoopOffSend(void)
{
    S_MaintenanceLoopOffCommand    maintenanceLoopOffCommand ;
    S_H245Msg                      h245Msg ;

    oscl_memset(&maintenanceLoopOffCommand, 0, sizeof(S_MaintenanceLoopOffCommand)) ;

    h245Msg.Type1 = H245_MSG_CMD ;
    h245Msg.Type2 = MSGTYP_ML_OFF_CMD ;
    h245Msg.pData = (uint8*) & maintenanceLoopOffCommand ;

    MessageSend(&h245Msg) ;
}


/************************************************************************/
/*  function name       : Se_MlPtvLoopIdcSend                           */
/*  function outline    : LOOP.indicaton Send                           */
/*  function discription: void Se_MlPtvLoopIdcSend(                     */
/*                                       PS_MaintenanceLoopRequest )    */
/*  input data          : PS_MaintenanceLoopRequest                     */
/*                                              pMaintenanceLoopRequest */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::PtvLoopIdcSend(PS_MaintenanceLoopRequest pMaintenanceLoopRequest)
{
    S_InfHeader         header ;
    S_MlRequestType     loop_Type ;

    oscl_memcpy((int8*)&loop_Type, (int8*)&pMaintenanceLoopRequest->mlRequestType, sizeof(S_MlRequestType)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Ml_Loop_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & loop_Type ;
    header.Size = sizeof(S_MlRequestType) ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_MlPtvRlsIdcSend                            */
/*  function outline    : RELEASE.indication Send                       */
/*  function discription: void Se_MlPtvRlsIdcSend( int32, PS_Cause_mlse ) */
/*  input data          : int32 Source                                    */
/*                      : PS_Cause_mlse pCause                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::PtvRlsIdcSend(int32 Source , PS_MlRejectCause pCause)
{
    S_InfHeader        header ;
    S_SourceCause_Ml   sourceCause_Ml ;

    sourceCause_Ml.Source = (ENUM_Source)Source ;
    oscl_memcpy((int8*)&sourceCause_Ml.Cause , (int8*)pCause , sizeof(S_MlRejectCause)) ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Ml_Rls_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & sourceCause_Ml ;
    header.Size = sizeof(S_SourceCause_Ml) ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_MlPtvLoopCfmSend                           */
/*  function outline    : LOOP.confirm Send                             */
/*  function discription: void Se_MlPtvLoopCfmSend( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::PtvLoopCfmSend(void)
{
    S_InfHeader    header ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Ml_Loop_Cfm ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = NULL ;
    header.Size = 0 ;

    PrimitiveSend(&header) ;
}


/************************************************************************/
/*  function name       : Se_LcPtvErrIdcSend                            */
/*  function outline    : ERROR.indication Send ( Uni-Directional )     */
/*  function discription: Se_LcPtvErrIdcSend( PS_LCB_ENTRY ,            */
/*                      :                                ENUM_ErrCode ) */
/*  input data          : PS_LCB_ENTRY pLcbEntry                        */
/*                      : ENUM_ErrCode ErrCode                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.01                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                            */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::PtvErrIdcSend(ENUM_ErrCode ErrCode)
{
    S_InfHeader    header ;
    S_ErrCode      errCode ;

    errCode.ErrCode = ErrCode ;

    header.InfType = H245_PRIMITIVE ;
    header.InfId = E_PtvId_Ml_Err_Idc ;
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    header.pParameter = (uint8*) & errCode ;
    header.Size = sizeof(S_ErrCode) ;

    PrimitiveSend(&header) ;
}


#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */

#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char* inString);

// =========================================================
// Se_MlStatusShow()
//
// This function displays state transition information for
// the ML signaling entity.
// =========================================================
void ML::StatusShow(uint8 oldStatus, uint8 newStatus)
{
    char tempString[80];

    Show245(ANALYZER_SE, 0, "MLSE State Transition:");
    sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
    Show245(ANALYZER_SE, 0, tempString);
    sprintf(tempString, "    to--> %s", StateLabel(newStatus));
    Show245(ANALYZER_SE, 0, tempString);
    Show245(ANALYZER_SE, 0, " ");
}

// ==========================================================
// Se_MlStateLabel()
//
// Returns a pointer to an approprate state label string.
// ==========================================================
char* ML::StateLabel(uint8 status)
{
    switch (status)
    {
        case ML_OUTGOING_NOT_LOOPED:
            return("Outgoing NOT LOOPED");
            break;
        case ML_OUTGOING_AWTING_RPS:
            return("Outgoing AWAITING RESPONSE");
            break;
        case ML_OUTGOING_LOOPED:
            return("Outgoing LOOPED");
            break;
        case ML_INCOMING_NOT_LOOPED:
            return("Incoming NOT LOOPED");
            break;
        case ML_INCOMING_AWTING_RPS:
            return("Incoming AWAITING RESPONSE");
            break;
        case ML_INCOMING_LOOPED:
            return("Incoming LOOPED");
            break;
        default:
            return("UNKNOWN STATE");
    }
}
#endif            /* --------------------------------- */

/************************************************************************/
/*  function name       : Se_MlT102TimerStart                           */
/*  function outline    : T102 Timer Start                              */
/*  function discription: void Se_MlT102TimerStart( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::T102TimerStart(void)
{
    TmrSqcNumberInc() ;
    RequestTimer(E_TmrId_Ml_T102 , TmrSqcNumberRead() ,
                 TimerDuration) ;
}


/************************************************************************/
/*  function name       : Se_MlT102TimerStop                            */
/*  function outline    : T102 Timer Stop                               */
/*  function discription: void Se_MlT102TimerStop( void )               */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.19                                     */
/*----------------------------------------------------------------------*/
/*  amendent career (x)     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void ML::T102TimerStop(void)
{
    CancelTimer(E_TmrId_Ml_T102) ;
}
