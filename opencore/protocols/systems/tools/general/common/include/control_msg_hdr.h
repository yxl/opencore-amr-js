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
#ifndef CONTROL_MSG_HDR_H
#define CONTROL_MSG_HDR_H

#include "oscl_base.h"

/************************************************************************/
/*  Interface Information Structure Type Definitions                    */
/************************************************************************/
typedef struct _control_msg_header
{
    enum TDirection
    {
        INCOMING = 1,
        OUTGOING = 2
    };

    uint32    InfType ;               /* Information Type */
    uint32    InfId ;                 /* Information Id */
    /* User Operation Id */
    /* Primitive Id */
    /* Internal Error Occurence Routine */
    /* Timeout Id */
    uint32    InfSupplement1 ;        /* Information Supplement 1 */
    uint32    InfSupplement2 ;        /* Information Supplement 2 */
    uint8*    pParameter ;            /* Primitive Information Pointer ( cf.infptv.h ) */
    uint32    Size ;                  /* Information Parameter Size */
    TDirection Dir;                 /* Incoming/Outgoing SE */

    uint8*    EncodedMsg;           /* Per encoded version of this message */
    uint32    EncodedMsgSize;

} S_ControlMsgHeader ;
typedef S_ControlMsgHeader *PS_ControlMsgHeader ;


#endif
