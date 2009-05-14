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
// m4v_payload_parser_factory.cpp
//
// Implementation of MPEG-4 video payload parser factory.
//
///////////////////////////////////////////////////////////////////////////////

#include "m4v_payload_parser_factory.h"
#include "m4v_payload_parser.h"

OSCL_EXPORT_REF IPayloadParser* M4VPayloadParserFactory::createPayloadParser()
{
    return OSCL_NEW(M4VPayloadParser, ());
}

OSCL_EXPORT_REF void M4VPayloadParserFactory::destroyPayloadParser(IPayloadParser* parser)
{
    OSCL_DELETE(parser);
}

