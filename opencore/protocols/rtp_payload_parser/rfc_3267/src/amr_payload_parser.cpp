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
// amr_payload_parser.cpp
//
// Implementation of payload parser for AMR RTP format (RFC 3267).
//
///////////////////////////////////////////////////////////////////////////////

#include "amr_payload_parser.h"
#include "virtual_buffer.h"

///////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF AMRPayloadParser::AMRPayloadParser()
{
    octetAligned = true;
    crc = false;
    amr_type = AMR_TYPE_AMR;
}

OSCL_EXPORT_REF AMRPayloadParser::~AMRPayloadParser()
{

}

///////////////////////////////////////////////////////////////////////////////
//
// Initialization
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF bool AMRPayloadParser::Init(mediaInfo* config)
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
AMRPayloadParser::Parse(const Payload& inputPacket,
                        Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    bool bRet;
    Payload output;
    VirtualBuffer<uint8> vbuf = VirtualBuffer<uint8>(inputPacket.vfragments);
    // parse the payload
    if (octetAligned)
    {
        bRet = doParseOctetAligned(vbuf, output);
    }
    else
    {
        bRet = doParseBandwithEff(vbuf, output);
    }

    if (!bRet)
    {
        return PayloadParserStatus_Failure;
    }

    //drop empty payloads
    if (output.vfragments.size() > 0)
    {
        // setup payload flags
        output.marker = inputPacket.marker;
        output.randAccessPt = inputPacket.randAccessPt;
        output.sequence = inputPacket.sequence + 1;
        output.timestamp = inputPacket.timestamp;
        output.stream = inputPacket.stream;   // only one stream supported
        vParsedPayloads.push_back(output);
    }
    return PayloadParserStatus_Success;
}

bool AMRPayloadParser::doParseBandwithEff(VirtualBuffer<uint8>& vb, Payload& outputPacket)
{
    // not implemented
    OSCL_UNUSED_ARG(vb);
    OSCL_UNUSED_ARG(outputPacket);

    return false;
}


bool AMRPayloadParser::doParseOctetAligned(VirtualBuffer<uint8>& vb, Payload& outputPacket)
{
    if (crc)
    {
        /* Not supported */
        return false;
    }

    // point to the start of the TOC
    vb += 1;

    // fill the outputPacket with the corresponding fragments
    VirtualBuffer<uint8> payload;
    vb.createSubBuffer(payload);
    payload.copy(outputPacket.vfragments);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Repositioning related
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF void AMRPayloadParser::Reposition(const bool adjustSequence, const uint32 stream, const uint32 seqnum)
{
    OSCL_UNUSED_ARG(adjustSequence);
    OSCL_UNUSED_ARG(stream);
    OSCL_UNUSED_ARG(seqnum);
}

OSCL_EXPORT_REF uint32 AMRPayloadParser::GetMinCurrTimestamp()
{
    return 0;
}
