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
///////////////////////////////////////////////////////////////////////////////
//
// m4v_payload_parser.cpp
//
// Implementation of payload parser for MPEG-4 video RTP format.
//
///////////////////////////////////////////////////////////////////////////////

#include "oscl_mem.h"
#include "m4v_payload_parser.h"

///////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF M4VPayloadParser::M4VPayloadParser()
{

}

OSCL_EXPORT_REF M4VPayloadParser::~M4VPayloadParser()
{

}

///////////////////////////////////////////////////////////////////////////////
//
// Initialization
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF bool M4VPayloadParser::Init(mediaInfo* config)
{
    // \todo: parse here
    OSCL_UNUSED_ARG(config);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// Payload parsing
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PayloadParserStatus
M4VPayloadParser::Parse(const Payload& inputPacket,
                        Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    Payload out;

    out.stream = inputPacket.stream;
    out.marker = inputPacket.marker;
    out.randAccessPt = inputPacket.randAccessPt;
    out.sequence = inputPacket.sequence + 1;
    out.timestamp = inputPacket.timestamp;

    for (uint32 i = 0; i < inputPacket.vfragments.size(); i++)
    {
        out.vfragments.push_back(inputPacket.vfragments[i]);
    }

    vParsedPayloads.push_back(out);

    return PayloadParserStatus_Success;
}

///////////////////////////////////////////////////////////////////////////////
//
// Repositioning related
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF void M4VPayloadParser::Reposition(const bool adjustSequence, const uint32 stream, const uint32 seqnum)
{
    OSCL_UNUSED_ARG(adjustSequence);
    OSCL_UNUSED_ARG(stream);
    OSCL_UNUSED_ARG(seqnum);
}

OSCL_EXPORT_REF uint32 M4VPayloadParser::GetMinCurrTimestamp()
{
    return 0;
}
