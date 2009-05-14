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
#include "sdp_mediaparser_factory.h"
#include "aac_media_info_parser.h"
#include "amr_media_info_parser.h"
#include "h263_media_info_parser.h"
#include "h264_media_info_parser.h"
#include "m4v_media_info_parser.h"
#include "rfc3640_media_info_parser.h"
#include "still_image_media_info_parser.h"
#include "pcma_media_info_parser.h"
#include "pcmu_media_info_parser.h"

SDPBaseMediaInfoParser*
SDPAMRMediaParserFactory::createSDPMediaParserInstance()
{
    SDPAMRMediaInfoParser* parser = OSCL_NEW(SDPAMRMediaInfoParser, ());
    return (parser);
}

void
SDPAMRMediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPAACMediaParserFactory::createSDPMediaParserInstance()
{
    SDPAACMediaInfoParser* parser = OSCL_NEW(SDPAACMediaInfoParser, ());
    return (parser);
}

void
SDPAACMediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPH263MediaParserFactory::createSDPMediaParserInstance()
{
    SDPH263MediaInfoParser* parser = OSCL_NEW(SDPH263MediaInfoParser, ());
    return (parser);
}

void
SDPH263MediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPH264MediaParserFactory::createSDPMediaParserInstance()
{
    SDPH264MediaInfoParser* parser = OSCL_NEW(SDPH264MediaInfoParser, ());
    return (parser);
}

void
SDPH264MediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPMPEG4MediaParserFactory::createSDPMediaParserInstance()
{
    SDPMPEG4MediaInfoParser* parser = OSCL_NEW(SDPMPEG4MediaInfoParser, ());
    return (parser);
}

void
SDPMPEG4MediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPRFC3640MediaParserFactory::createSDPMediaParserInstance()
{
    SDPRFC3640MediaInfoParser* parser = OSCL_NEW(SDPRFC3640MediaInfoParser, ());
    return (parser);
}

void
SDPRFC3640MediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPStillImageMediaParserFactory::createSDPMediaParserInstance()
{
    SDPStillImageMediaInfoParser* parser = OSCL_NEW(SDPStillImageMediaInfoParser, ());
    return (parser);
}

void
SDPStillImageMediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPPCMAMediaParserFactory::createSDPMediaParserInstance()
{
    SDPPCMAMediaInfoParser* parser = OSCL_NEW(SDPPCMAMediaInfoParser, ());
    return (parser);
}

void
SDPPCMAMediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}

SDPBaseMediaInfoParser*
SDPPCMUMediaParserFactory::createSDPMediaParserInstance()
{
    SDPPCMUMediaInfoParser* parser = OSCL_NEW(SDPPCMUMediaInfoParser, ());
    return (parser);
}

void
SDPPCMUMediaParserFactory::destruct_and_dealloc(OsclAny* ptr)
{
    OSCL_UNUSED_ARG(ptr); // line added to remove warning "unreferenced formal parameter"
}
