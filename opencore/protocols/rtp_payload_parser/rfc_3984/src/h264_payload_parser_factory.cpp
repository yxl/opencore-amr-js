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
// h264_payload_parser_factory.cpp
//
// Implementation of H.264 payload parser factory.
//
///////////////////////////////////////////////////////////////////////////////

#include "h264_payload_parser_factory.h"
#include "h264_payload_parser.h"

OSCL_EXPORT_REF IPayloadParser* H264PayloadParserFactory::createPayloadParser()
{
    return OSCL_NEW(H264PayloadParser, ());
}

OSCL_EXPORT_REF void H264PayloadParserFactory::destroyPayloadParser(IPayloadParser* parser)
{
    OSCL_DELETE(parser);
}

