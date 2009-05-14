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
// h263_payload_parser.h
//
// H.263 payload parser.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef H263_PAYLOAD_PARSER_H_INCLUDED
#define H263_PAYLOAD_PARSER_H_INCLUDED


#include "payload_parser.h"

///////////////////////////////////////////////////////////////////////////////
//
// H.263 PayloadParser Class
//
///////////////////////////////////////////////////////////////////////////////

class H263PayloadParser : public IPayloadParser
{
    public:
        OSCL_IMPORT_REF H263PayloadParser();
        OSCL_IMPORT_REF virtual ~H263PayloadParser();

        OSCL_IMPORT_REF virtual bool Init(mediaInfo* config);
        OSCL_IMPORT_REF virtual PayloadParserStatus Parse(const Payload& inputPacket,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads);

        OSCL_IMPORT_REF virtual void Reposition(const bool adjustSequence = false, const uint32 stream = 0, const uint32 seqnum = 0);
        OSCL_IMPORT_REF virtual uint32 GetMinCurrTimestamp();

    private:
};

#endif //H263_PAYLOAD_PARSER_H_INCLUDED
