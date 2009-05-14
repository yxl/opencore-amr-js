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
/*                                                                               */
/*********************************************************************************/

/*
**   File:   rtprtcp.h
**
**   Description:
**      This file contains the definitions common to RTP and RTCP
*/

#ifndef RTPRTCP_H
#define RTPRTCP_H

/*
** Includes
*/

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

typedef uint32 RtpSsrc;
typedef uint32 RtpTimeStamp;
typedef uint16 RtpPayloadType;
typedef uint16 RtpSeqType;


/*
** Constants
*/
const int16 NUM_BYTES_IN_UINT_32 = 4;
const uint8 DEFAULT_RTPRTCP_VERSION = 2;
const int16 RTPRTCP_VERSION_BIT_POSITION = 6;
const int16 RTPRTCP_PAD_FLAG_BIT_POSITION = 5;
const uint16 MAX_STR_COUNT = 256;

#endif
