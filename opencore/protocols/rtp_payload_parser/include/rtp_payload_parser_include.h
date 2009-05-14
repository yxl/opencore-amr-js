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
#ifndef RTP_PAYLOAD_PARSER_INCLUDE_H
#define RTP_PAYLOAD_PARSER_INCLUDE_H

#include "oscl_types.h"

typedef struct AMR_INFO
{
    enum AMR_TYPE
    {
        AMR_TYPE_NONE,
        AMR_TYPE_AMR,
        AMR_TYPE_WB,

    };

    AMR_TYPE		amr_type;
    uint16			modeSet;
    uint16			channels;
    bool			octetAligned;
    bool			crc;
    bool			interleave;

} S_AMR_INFO;

typedef struct H263_INFO
{
    bool P_Bit;	//	indicates whether picture start code is present or not
    bool V_Bit;	//	indicates presence of VRC data
    uint8 PLEN;		//	Length in bytes of extra picture header
    uint8 PEBIT;	//	indicates number of bits to be ignored in last byte of picture header

    //	VRC Fields
    uint8 TID;		//
    uint8 Trun;		//
    uint8 S_Bit;	//	sync frame indication


} H263_INFO;

#endif // RTP_PAYLOAD_PARSER_INCLUDE_H

