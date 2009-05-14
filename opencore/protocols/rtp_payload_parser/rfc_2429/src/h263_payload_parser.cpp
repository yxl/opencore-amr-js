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
// h263_payload_parser.cpp
//
// H.263 payload parser implementation.
//
///////////////////////////////////////////////////////////////////////////////

#include "oscl_mem.h"
#include "virtual_buffer.h"
#include "h263_payload_parser.h"


///////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF H263PayloadParser::H263PayloadParser()
{

}

OSCL_EXPORT_REF H263PayloadParser::~H263PayloadParser()
{

}

///////////////////////////////////////////////////////////////////////////////
//
// Initialization
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF bool H263PayloadParser::Init(mediaInfo* config)
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
H263PayloadParser::Parse(const Payload& inputPacket,
                         Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    //	H263 Payload Header Masks
    static const uint8 RR = 0xF8;	//	Reserved Bits Mask
    static const uint8 Pbit = 0x04;	//	Picture or GOB Start bit mask
    static const uint8 Vbit = 0x02;	//	VRC mask

    bool bPBit;	    //	indicates whether picture start code is present or not
    bool bVBit;	    //	indicates presence of VRC data

    VirtualBuffer<uint8> input = VirtualBuffer<uint8>(inputPacket.vfragments);
    VirtualBuffer<uint8> vb(input);

    if ((*vb) & (RR))
    {
        //check for 5 reserved bits to be zero
        return PayloadParserStatus_Failure;
    }

    //check for picture start bit
    bPBit = (*vb & Pbit) > 0;

    //check for VRC
    bVBit = (*vb & Vbit) > 0;

    // 1st bit of PLEN in the 1st byte of the payload header
    uint8 plenHI = (*vb & 0x01);
    // the next 5 bits of PLEN in the 2nd byte of the payload header
    uint8 plenLow = (*(vb + 1) & 0xF8);

    // reconstruct PLEN
    uint16 plen = (((uint16)plenHI << 8) + plenLow) >> 3;

    Payload output;

    if (bPBit) // Picture or GOB start code present
    {
        if (plen > 0) // If extra picture header presents in the midlle of the frame
        {
            // skip the extra picture header from the payload
            vb += plen;
            // start the output payload from offset plen
            input += plen;
        }

        // we need to prepend the payload with two zero bytes

        if (bVBit) // VRC Header present - payload starts at 1
        {
            // set bytes at offset 1 and 2 to zero
            vb++;
            *vb = 0;
            vb++;
            *vb = 0;

            // start the output payload from offset 1
            input += 1;
            if (input.length() > 0)
            {
                VirtualBuffer<uint8> outFrags;
                input.createSubBuffer(outFrags);
                outFrags.copy(output.vfragments);
            }
        }
        else	   // VRC header not present - payload starts at 0
        {
            // set bytes at offset 0 and 1 to zero
            *vb = 0;
            vb++;
            *vb = 0;

            if (input.length() > 0)
            {
                VirtualBuffer<uint8> outFrags;
                input.createSubBuffer(outFrags);
                outFrags.copy(output.vfragments);
            }
        }
    }
    else
    {
        if (bVBit) // VRC Header present - payload starts at 3
        {
            input += 3;
            if (input.length() > 0)
            {
                VirtualBuffer<uint8> outFrags;
                input.createSubBuffer(outFrags);
                outFrags.copy(output.vfragments);
            }
        }
        else       // No VRC Header - payload starts at 2
        {
            input += 2;
            if (input.length() > 0)
            {
                VirtualBuffer<uint8> outFrags;
                input.createSubBuffer(outFrags);
                outFrags.copy(output.vfragments);
            }
        }
    }

    if (output.vfragments.size() > 0)
    {
        output.stream = inputPacket.stream;
        output.marker = inputPacket.marker;
        output.sequence = inputPacket.sequence + 1;
        output.timestamp = inputPacket.timestamp;
        vParsedPayloads.push_back(output);
    }

    return PayloadParserStatus_Success;
}

///////////////////////////////////////////////////////////////////////////////
//
// Repositioning related
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF void H263PayloadParser::Reposition(const bool adjustSequence, const uint32 stream, const uint32 seqnum)
{
    OSCL_UNUSED_ARG(adjustSequence);
    OSCL_UNUSED_ARG(stream);
    OSCL_UNUSED_ARG(seqnum);
}

OSCL_EXPORT_REF uint32 H263PayloadParser::GetMinCurrTimestamp()
{
    return 0;
}
