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
#ifndef TSC_CONSTANTS_H_INCLUDED
#define TSC_CONSTANTS_H_INCLUDED

#define TSC_INCOMING_CHANNEL_MASK (1<<16)

typedef enum _status
{
    Phase0_Idle = 1 ,
    PhaseA          ,
    PhaseB          ,
    PhaseC          ,
    PhaseD_CSUP		,  /* Call Setup */
    PhaseE_Comm     ,  /* Ongoing Communication */
    PhaseF_Clc      ,  /* Closing all outgoing LCNs */
    PhaseF_End      ,  /* End of Session */
    PhaseG_Dis
} ENUM_Status;


/* From tsc_lc.h */
#define     RELEASE_REJECT      1           /* RELEASE.request(LC) - Reject  */
#define     RELEASE_CLOSE       2           /* RELEASE.request(LC) - Close   */


#define PV_TSC_WAITING_FOR_OBLC_TIMER_ID 4


#define  TSC_CE_SEND           0
#define  TSC_CE_RECEIVE        1
#define  TSC_MSD               2
#define  TSC_MSD_RECEIVE       12
#define  TSC_MT_SEND           3
#define  TSC_MT_RECEIVE        4
#define  TSC_LC_AUDIO_SEND     5
#define  TSC_LC_AUDIO_RECEIVE  6
#define  TSC_LC_VIDEO_SEND     7    // I may need to modify this to accom * /
#define  TSC_LC_VIDEO_RECEIVE  8    // *  both Uni- and Bi- cases. * /
//-----------------------------* /
#define  TSC_MSD_DECISION      9    // * either MASTER or SLAVE * /
#define  TSC_MSD_RETRYCOUNT    10
#define  TSC_MUXTABLE_CHANGE   11   // * Mux Table Change Flag * /
//----------------------------- * /
#define  MAX_TSC_STATES        13   // * Max number of states * /


#define NOT_STARTED   0
#define STARTED       1
#define COMPLETE      2
#define WAIT_FOR_OLC  3  // BLC Open; We have received Reject(URP) and


/* MasterSlaveDetermination Decision */
#define MASTER 0
#define SLAVE 1

#define N100_DEFAULT 3


#endif

