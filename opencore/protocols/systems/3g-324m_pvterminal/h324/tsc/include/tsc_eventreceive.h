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

/*  file name            : tsc_eventreceive.h                                */

/*  file contents        : Event Id Receive Header                           */

/*  draw                 : '96.10.04                                         */

/*---------------------------------------------------------------------------*/

/*  amendment                                                                */

/*              Copyright (C) 1996 NTT DoCoMo                                */

/*****************************************************************************/

#if !defined (_TSC_EVENTRECEIVE)

#define _TSC_EVENTRECEIVE
#include "pvt_params.h"


/*---------------------------------------------------------------------------*/

/*  Prototype Declare                                                        */

/*---------------------------------------------------------------------------*/

uint32 Tsc_EventReceive(PS_ControlMsgHeader pReceiveInf);

uint32 Tsc_UserEventLookup(PS_ControlMsgHeader pReceiveInf);



#endif

