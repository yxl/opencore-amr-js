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
#ifndef SDP_MEDIA_PARSER_REGISTRY_FACTORY
#include "sdp_mediaparser_registry_populator.h"
#endif

#ifndef SDP_MEDIAPARSER_FACTORY_H
#include "sdp_mediaparser_factory.h"
#endif

SDPMediaParserRegistry* SDPMediaParserRegistryPopulater::PopulateRegistry()
{
    SDPMediaParserRegistry* sdpMediaParserRegistry = NULL;
    StrPtrLen aac_latm("MP4A-LATM");
    StrPtrLen aac("AAC");
    StrPtrLen amr("AMR");
    StrPtrLen amrwb("AMR-WB");
    StrPtrLen h263_old("H263-1998");
    StrPtrLen h263("H263-2000");
    StrPtrLen m4v("MP4V-ES");
    StrPtrLen author_m4v("PVMP4V-ES");
    StrPtrLen still("X-MP4V-IMAGE");
    StrPtrLen h264("H264");
    StrPtrLen pcma("PCMA");
    StrPtrLen pcmu("PCMU");
    StrPtrLen rfc3640("mpeg4-generic");

    SDPMediaParserRegistry::Init();

    sdpMediaParserRegistry =
        SDPMediaParserRegistry::GetSDPMediaParserRegistry();

    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(aac_latm, OSCL_NEW(SDPAACMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(aac, OSCL_NEW(SDPAACMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(amr, OSCL_NEW(SDPAMRMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(amrwb, OSCL_NEW(SDPAMRMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(h263_old, OSCL_NEW(SDPH263MediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(h263, OSCL_NEW(SDPH263MediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(m4v, OSCL_NEW(SDPMPEG4MediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(author_m4v, OSCL_NEW(SDPMPEG4MediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(still, OSCL_NEW(SDPStillImageMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(h264, OSCL_NEW(SDPH264MediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(pcma, OSCL_NEW(SDPPCMAMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(pcmu, OSCL_NEW(SDPPCMUMediaParserFactory, ()));
    sdpMediaParserRegistry->addMediaParserFactoryToRegistry(rfc3640, OSCL_NEW(SDPRFC3640MediaParserFactory, ()));

    return sdpMediaParserRegistry;
}

void SDPMediaParserRegistryPopulater::CleanupRegistry(SDPMediaParserRegistry*& aMediaParserRegistry)
{
    StrPtrLen rfc3640("mpeg4-generic");
    StrPtrLen pcmu("PCMU");
    StrPtrLen pcma("PCMA");
    StrPtrLen h264("H264");
    StrPtrLen still("X-MP4V-IMAGE");
    StrPtrLen author_m4v("PVMP4V-ES");
    StrPtrLen m4v("MP4V-ES");
    StrPtrLen h263("H263-2000");
    StrPtrLen h263_old("H263-1998");
    StrPtrLen amrwb("AMR-WB");
    StrPtrLen amr("AMR");
    StrPtrLen aac("AAC");
    StrPtrLen aac_latm("MP4A-LATM");

    OsclMemoryFragment memFrag;

    memFrag.ptr = (OsclAny*)(rfc3640.c_str());
    memFrag.len = (uint32)rfc3640.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(pcmu.c_str());
    memFrag.len = (uint32)pcmu.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(pcma.c_str());
    memFrag.len = (uint32)pcma.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(h264.c_str());
    memFrag.len = (uint32)h264.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(still.c_str());
    memFrag.len = (uint32)still.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(author_m4v.c_str());
    memFrag.len = (uint32)author_m4v.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(m4v.c_str());
    memFrag.len = (uint32)m4v.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(h263.c_str());
    memFrag.len = (uint32)h263.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(h263_old.c_str());
    memFrag.len = (uint32)h263_old.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(amrwb.c_str());
    memFrag.len = (uint32)amrwb.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(amr.c_str());
    memFrag.len = (uint32)amr.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(aac.c_str());
    memFrag.len = (uint32)aac.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(aac_latm.c_str());
    memFrag.len = (uint32)aac_latm.size();
    OSCL_DELETE(aMediaParserRegistry->lookupSDPMediaParserFactory(memFrag));

    SDPMediaParserRegistry::Cleanup();
}
