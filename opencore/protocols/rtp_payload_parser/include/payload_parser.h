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
// payload_parser.h
//
// Defines the abstract payload parser type.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PAYLOAD_PARSER_H_INCLUDED
#define PAYLOAD_PARSER_H_INCLUDED

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif
#ifndef MEDIAINFO_H_INCLUDED
#include "media_info.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//
// enum PayloadParserStatus
//
// Payload parser return codes.
//
///////////////////////////////////////////////////////////////////////////////

enum PayloadParserStatus
{
    PayloadParserStatus_Failure,
    PayloadParserStatus_Success,
    PayloadParserStatus_InputNotExhausted,
    PayloadParserStatus_MemorAllocFail,
    PayloadParserStatus_DataNotReady,
    PayloadParserStatus_EmptyQueue
};

///////////////////////////////////////////////////////////////////////////////
//
// class IPayloadParser
//
// Abstract payload parser interface.
//
///////////////////////////////////////////////////////////////////////////////

class IPayloadParser
{
    public:

        virtual ~IPayloadParser() { };

        // Init()
        //
        // Applies media info data structure to a given payload parser.
        OSCL_IMPORT_REF virtual bool Init(mediaInfo* config) = 0;

        // Payload structure
        //
        // Used to pass information in and out of the payload parser for
        // a given stream.
        class Payload
        {
            public:
                Payload() : stream(0), timestamp(0), sequence(0),
                        marker(false), randAccessPt(false), incompframe(false), consumed(false), endOfNAL(false) {}
                Payload(uint32 stream, uint32 timestamp, uint32 sequence,
                        bool marker, bool randAccessPt, bool consumed, bool incompframe, bool endOfNAL)
                {
                    this->stream = stream;
                    this->timestamp = timestamp;
                    this->sequence = sequence;
                    this->marker = marker;
                    this->randAccessPt = randAccessPt;
                    this->consumed = consumed;
                    this->incompframe = incompframe;
                    this->endOfNAL = endOfNAL;
                }
                Payload(const Payload& aPayLoad)
                {
                    Copy(aPayLoad);
                }
                Payload& operator=(const Payload& aPayLoad)
                {
                    if (&aPayLoad != this)
                    {
                        Copy(aPayLoad);
                    }
                    return (*this);
                }
                uint32 stream;
                uint32 timestamp;
                uint32 sequence;
                //The marker bit signifies the last packet of an access unit or frame.
                bool marker;
                bool randAccessPt;
                bool incompframe;
                bool consumed;
                //endOfNAL is used by the RFC3984 (H.264 / AVC) parser and decoder.
                //It is set to true for last NAL fragment and whole NALs
                //and set to false for the first and middle fragments.
                bool endOfNAL;
                Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> vfragments;

            private:
                void Copy(const Payload& aPayLoad)
                {
                    stream = aPayLoad.stream;
                    timestamp = aPayLoad.timestamp;
                    sequence = aPayLoad.sequence;
                    marker = aPayLoad.marker;
                    randAccessPt = aPayLoad.randAccessPt;
                    consumed = aPayLoad.consumed;
                    vfragments = aPayLoad.vfragments;
                    incompframe = aPayLoad.incompframe;
                    endOfNAL = aPayLoad.endOfNAL;
                }
        };

        // Parse()
        //
        // Performs parsing of a given input packet consisting of a vector of
        // buffers. The parsed output is a vector of parsed payloads where each
        // element in the vector corresponds to the stream id.
        OSCL_IMPORT_REF virtual PayloadParserStatus Parse(const Payload& inputPacket,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads) = 0;

        // Reposition()
        //
        // Tells the payload parser a repositioning is in effect.
        // Change internal seqnum if adjustSequence is enabled
        OSCL_IMPORT_REF virtual void Reposition(const bool adjustSequence = false, const uint32 stream = 0, const uint32 seqnum = 0) = 0;

        // GetMinCurrTimestamp()
        //
        // Returns the smallest current timestamp among all streams managed
        // by the payload parser.
        OSCL_IMPORT_REF virtual uint32 GetMinCurrTimestamp() = 0;
};



#endif // PAYLOAD_PARSER_H_INCLUDED
