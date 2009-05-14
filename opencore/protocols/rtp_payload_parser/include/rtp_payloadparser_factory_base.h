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
#ifndef RTP_PAYLOADPARSER_FACTORY_BASE_H
#define RTP_PAYLOADPARSER_FACTORY_BASE_H

#include "oscl_base_alloc.h"

class RTPPayloadParser;

class RTPPayloadParserFactory
{
    public:
        OSCL_IMPORT_REF virtual RTPPayloadParser* createRTPPayloadParserInstance() = 0;
        OSCL_IMPORT_REF virtual void destroyRTPPayloadParserInstance(RTPPayloadParser* parser) = 0;
};

#endif // RTP_PAYLOADPARSER_FACTORY_BASE_H

