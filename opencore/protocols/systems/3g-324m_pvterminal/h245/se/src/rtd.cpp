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
/*  file name       : sertd.c                                           */
/*  file contents   : Round Trip Deray Signalling Entity Management     */
/*  draw            : '96.11.13                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/


/************************************************************************/
/*  Headerfile Include                                                  */
/************************************************************************/
#include "oscl_base.h"
#include "oscl_time.h"
#include "semsgque.h"
#include "rtd.h"

/************************************************************************/
/*  function name       : _0800_0000                              */
/*  function outline    : Event     TRANSFER.request                    */
/*                      : Status    IDLE                                */
/*  function discription: void Se_Rtd_0800_0000( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::_0800_0000(void)
{
    /* Increment out-sequence number */
    OutSqcInc() ;
    /* RoundTripDelayRequest message send */
    StatusWrite(RTD_AWTING_RPS) ;
    MsgRequestSend() ;
    /* Start mesurement of delay time */
    StartMeasurement() ;
    /* Start timer for waiting response */
    T105TimerStart() ;
}

/************************************************************************/
/*  function name       : Se_Rtd_0800_0010                              */
/*  function outline    : Event     TRANSFER.request                    */
/*                      : Status    AWAITING RESPONSE                   */
/*  function discription: void Se_Rtd_0800_0010( void )                 */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::_0800_0010(void)
{
    /* Stop timer for waiting response */
    T105TimerStop() ;
    /* Increment out-sequence number */
    OutSqcInc() ;
    /* Update status */
    StatusWrite(RTD_AWTING_RPS) ;
    /* RoundTripDelayRequest message send */
    MsgRequestSend() ;
    /* Start mesurement of delay time */
    StartMeasurement() ;
    /* Start timer for waiting response */
    T105TimerStart() ;
}

/************************************************************************/
/*  function name       : Se_Rtd_0801_0000                              */
/*  function outline    : Event     RoundTripDelayRequest               */
/*                      : Status    IDLE or AWAITING RESPONSE           */
/*  function discription: void Se_Rtd_0801_0000(                        */
/*                                           PS_RoundTripDelayRequest ) */
/*  input data          : RoundTripDelayRequest message parameter       */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::_0801_0000(PS_RoundTripDelayRequest pRoundTripDelayRequest)
{
    /* RoundTripDelayResponse message send */
    MsgResponseSend(pRoundTripDelayRequest) ;
}

/************************************************************************/
/*  function name       : Se_Rtd_0802_0010                              */
/*  function outline    : Event     RoundTripDelayResponse              */
/*                      : Status    AWAITING RESPONSE                   */
/*  function discription: void Se_Rtd_0802_0010(                        */
/*                                           PS_RoundTripDelayRequest ) */
/*  input data          : RoundTripDelayResponse message parameter      */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::_0802_0010(PS_RoundTripDelayResponse pRoundTripDelayResponse)
{
    int32 Delay ;

    if (pRoundTripDelayResponse->sequenceNumber == OutSqcRead())    /* Ckeck Sequence number */
    {
        /* Stop timer for waiting response */
        T105TimerStop() ;
        /* Stop measurement and get delay time */
        Delay = StopMeasurement() ;
        /* Update status */
        StatusWrite(RTD_IDLE) ;
        /* TRANSFER.confirm send */
        PtvTrnsCfmSend(Delay) ;
    }
}

/************************************************************************/
/*  function name       : Se_Rtd_0803_0010                              */
/*  function outline    : Event     T105 Timeout                        */
/*                      : Status    AWAITING RESPONSE                   */
/*  function discription: void Se_Rtd_0803_0010( int32 )                  */
/*  input data          : Timer sequence numeber                        */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::_0803_0010(int32 Param)
{
    if (Param == TmrSqcNumberRead())    /* check timer-sequence number */
    {
        /* Update status */
        StatusWrite(RTD_IDLE) ;
        /* EXPIRY.indication send */
        PtvExpIndSend() ;
    }
}

/************************************************************************/
/*  function name       : Se_RtdMsgRequestSend                          */
/*  function outline    : RoundTripDelayRequest message edit & send     */
/*  function discription: void Se_RtdMsgRequestSend( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::MsgRequestSend(void)
{
    S_RoundTripDelayRequest roundTripDelayRequest ;
    S_H245Msg               h245Msg ;

    /* Allocate message parameter area and clear it */
    oscl_memset(&roundTripDelayRequest, 0, sizeof(S_RoundTripDelayRequest)) ;
    /* Set sequence number */
    roundTripDelayRequest.sequenceNumber = (uint8) OutSqcRead() ;

    /* Set message type (Request message) */
    h245Msg.Type1 = H245_MSG_REQ ;
    /* Set message id (RoundTripDelayRequest) */
    h245Msg.Type2 = MSGTYP_RTD_REQ ;
    /* Set parameter address */
    h245Msg.pData = (uint8*) & roundTripDelayRequest ;

    /* Send message to corder */
    MessageSend(&h245Msg) ;
}

/************************************************************************/
/*  function name       : Se_RtdMsgResponseSend                         */
/*  function outline    : RoundTripDelayResponse message edit & send    */
/*  function discription: void Se_RtdMsgResponseSend(                   */
/*                                           PS_RoundTripDelayRequest ) */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::MsgResponseSend(PS_RoundTripDelayRequest pRoundTripDelayRequest)
{
    S_RoundTripDelayResponse roundTripDelayResponse ;
    S_H245Msg                h245Msg ;

    oscl_memset(&roundTripDelayResponse, 0, sizeof(S_RoundTripDelayResponse)) ;
    /* Set sequence number */
    roundTripDelayResponse.sequenceNumber = pRoundTripDelayRequest->sequenceNumber ;

    /* Set message type (Response message) */
    h245Msg.Type1 = H245_MSG_RPS ;
    /* Set message id (RoundTripDelayResponse) */
    h245Msg.Type2 = MSGTYP_RTD_RPS ;
    /* Set parameter address */
    h245Msg.pData = (uint8*) & roundTripDelayResponse ;

    /* Send message to corder */
    MessageSend(&h245Msg) ;
}

/************************************************************************/
/*  function name       : Se_RtdPtvTrnsCfmSend                          */
/*  function outline    : TRANSFER.confirm primitive edit & send        */
/*  function discription: void Se_RtdPtvTrnsCfmSend( int32 )              */
/*  input data          : Delay time                                    */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::PtvTrnsCfmSend(int32 Delay)
{
    S_InfHeader        header ;
    S_Delay_Rtd        delay_Rtd ;

    /* Allocate primitive parameter area and clear it */
    oscl_memset(&delay_Rtd  , 0 , sizeof(S_Delay_Rtd)) ;
    /* Set delay time */
    delay_Rtd.Delay_Value = Delay ;

    /* Set information type */
    header.InfType = H245_PRIMITIVE ;
    /* Set promitive id */
    header.InfId = E_PtvId_Rtd_Trf_Cfm ;
    /* clear supplement */
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    /* Set parameter address */
    header.pParameter = (uint8*) & delay_Rtd ;
    /* Set parameter size */
    header.Size = sizeof(S_Delay_Rtd) ;

    /* Send primitive to tsc */
    PrimitiveSend(&header) ;
}

/************************************************************************/
/*  function name       : Se_RtdPtvExpIndSend                           */
/*  function outline    : EXPIRY.indication primitive edit & send        */
/*  function discription: void Se_RtdPtvExpIndSend( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::PtvExpIndSend(void)
{
    S_InfHeader        header ;

    /* Set information type */
    header.InfType = H245_PRIMITIVE ;
    /* Set promitive id */
    header.InfId = E_PtvId_Rtd_Exp_Idc ;
    /* clear supplement */
    header.InfSupplement1 = 0 ;
    header.InfSupplement2 = 0 ;
    /* Clear parameter address */
    header.pParameter = NULL ;
    /* Clear parameter size */
    header.Size = 0 ;

    /* Send primitive to tsc */
    PrimitiveSend(&header) ;
}

#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */

#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
void Show245(uint16 tag, uint16 indent, char* inString);

// =========================================================
// Se_RtdStatusShow()
//
// This function displays state transition information for
// the RTD signaling entity.
// =========================================================
void RTD::StatusShow(uint8 oldStatus, uint8 newStatus)
{
    char tempString[80];

    Show245(ANALYZER_SE, 0, "RTD State Transition:");
    sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
    Show245(ANALYZER_SE, 0, tempString);
    sprintf(tempString, "    to--> %s", StateLabel(newStatus));
    Show245(ANALYZER_SE, 0, tempString);
    Show245(ANALYZER_SE, 0, " ");
}

// ==========================================================
// Se_RtdStateLabel()
//
// Returns a pointer to an approprate state label string.
// ==========================================================
char* RTD::StateLabel(uint8 status)
{
    switch (status)
    {
        case RTD_IDLE:
            return("IDLE");
            break;
        case RTD_AWTING_RPS:
            return("AWAITING RESPONSE");
            break;
        default:
            return("UNKNOWN STATE");
    }
}
#endif            /* --------------------------------- */

/************************************************************************/
/*  function name       : Se_RtdT105TimerStart                          */
/*  function outline    : T105 Timer Start                              */
/*  function discription: void Se_RtdT105TimerStart( void )             */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::T105TimerStart(void)
{
    /* Increment timer-sequence number */
    TmrSqcNumberInc() ;
    /* Start timer (T105) */
    RequestTimer(E_TmrId_Rtd_T105 , TmrSqcNumberRead() ,
                 TimerDuration) ;
}


/************************************************************************/
/*  function name       : Se_RtdT105TimerStop                           */
/*  function outline    : T105 Timer Stop                               */
/*  function discription: void Se_RtdT105TimerStop( void )              */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::T105TimerStop(void)
{
    /* Stop timer (T105) */
    CancelTimer(E_TmrId_Rtd_T105) ;
}

/************************************************************************/
/*  function name       : Se_RtdStartMeasurement                        */
/*  function outline    : Start mesurement of Round Trip Delay          */
/*  function discription: void Se_RtdStartMeasurement( void )           */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void RTD::StartMeasurement(void)
{
    StartTime.set_to_current_time();
}

/************************************************************************/
/*  function name       : Se_RtdStopMeasurement                         */
/*  function outline    : Stop mesurement of Round Trip Delay           */
/*  function discription: int32 Se_RtdSStopMeasurement( void )            */
/*  input data          : None                                          */
/*  output data         : Round Trip Delay time (milli second)          */
/*  draw time           : '96.11.13                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                                                                      */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int32 RTD::StopMeasurement(void)
{
    TimeValue StopTime;

    StopTime.set_to_current_time();
    return (StopTime.to_msec() - StartTime.to_msec());
}
