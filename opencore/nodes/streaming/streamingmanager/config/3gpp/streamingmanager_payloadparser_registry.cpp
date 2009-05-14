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
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_STREAMING_MANAGER_INTERNAL_H_INCLUDED
#include "pvmf_streaming_manager_internal.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif
#ifndef AMR_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "amr_payload_parser_factory.h"
#endif
#ifndef H263_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "h263_payload_parser_factory.h"
#endif
#ifndef H264_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "h264_payload_parser_factory.h"
#endif
#ifndef M4V_AUDIO_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "m4v_audio_payload_parser_factory.h"
#endif
#ifndef M4V_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "m4v_payload_parser_factory.h"
#endif
#ifndef PAYLOAD_PARSER_REGISTRY_H_INCLUDED
#include "payload_parser_registry.h"
#endif
#ifndef PVMF_MEDIALAYER_NODE_H_INCLUDED
#include "pvmf_medialayer_node.h"
#endif
#ifndef RFC3640_PAYLOAD_PARSER_FACTORY_H_INCLUDED
#include "rfc3640_payload_parser_factory.h"
#endif

void PVMFStreamingManagerNode::PopulatePayloadParserRegistry()
{
    PayloadParserRegistry* registry =
        PayloadParserRegistry::GetPayloadParserRegistry();
    OSCL_ASSERT(registry == NULL);
    PayloadParserRegistry::Init();
    registry = PayloadParserRegistry::GetPayloadParserRegistry();

    StrPtrLen aac_latm("audio/MP4A-LATM");
    StrPtrLen amr("audio/AMR");
    StrPtrLen h263_old("video/H263-1998");
    StrPtrLen h263("video/H263-2000");
    StrPtrLen m4v("video/MP4V-ES");
    StrPtrLen h264("video/H264");
    StrPtrLen mp4a(PVMF_MIME_MPEG4_AUDIO);
    StrPtrLen rfc3640("audio/mpeg4-generic");


    IPayloadParserFactory* m4vP = OSCL_NEW(M4VPayloadParserFactory, ());
    IPayloadParserFactory* aacP = OSCL_NEW(M4VAudioPayloadParserFactory, ());
    IPayloadParserFactory* amrP = OSCL_NEW(AmrPayloadParserFactory, ());
    IPayloadParserFactory* h263P = OSCL_NEW(H263PayloadParserFactory, ());
    IPayloadParserFactory* h264P = OSCL_NEW(H264PayloadParserFactory, ());

    IPayloadParserFactory* rfc3640P = OSCL_NEW(RFC3640PayloadParserFactory, ());

    registry->addPayloadParserFactoryToRegistry(m4v, m4vP);
    registry->addPayloadParserFactoryToRegistry(h264, h264P);
    registry->addPayloadParserFactoryToRegistry(aac_latm, aacP);
    registry->addPayloadParserFactoryToRegistry(mp4a, aacP);
    registry->addPayloadParserFactoryToRegistry(amr, amrP);
    registry->addPayloadParserFactoryToRegistry(h263_old, h263P);
    registry->addPayloadParserFactoryToRegistry(h263, h263P);
    registry->addPayloadParserFactoryToRegistry(rfc3640,  rfc3640P);


    // pass the registry to the media layer node
    PVMFSMNodeContainer* iMediaLayerNodeContainer =
        getNodeContainer(PVMF_STREAMING_MANAGER_MEDIA_LAYER_NODE);
    PVMFMediaLayerNodeExtensionInterface* mlExtIntf =
        (PVMFMediaLayerNodeExtensionInterface*)(iMediaLayerNodeContainer->iExtensions[0]);
    mlExtIntf->setPayloadParserRegistry(registry);

}

void PVMFStreamingManagerNode::destroyPayloadParserRegistry()
{
    StrPtrLen aac_latm("audio/MP4A-LATM");
    StrPtrLen amr("audio/AMR");
    StrPtrLen h263("video/H263-2000");
    StrPtrLen m4v("video/MP4V-ES");
    StrPtrLen h264("video/H264");
    StrPtrLen rfc3640("audio/mpeg4-generic");

    PayloadParserRegistry* registry =
        PayloadParserRegistry::GetPayloadParserRegistry();
    if (registry == NULL) return;

    OsclMemoryFragment memFrag;

    memFrag.ptr = (OsclAny*)(m4v.c_str());
    memFrag.len = (uint32)m4v.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(h264.c_str());
    memFrag.len = (uint32)h264.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(aac_latm.c_str());
    memFrag.len = (uint32)aac_latm.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(amr.c_str());
    memFrag.len = (uint32)amr.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(h263.c_str());
    memFrag.len = (uint32)h263.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));

    memFrag.ptr = (OsclAny*)(rfc3640.c_str());
    memFrag.len = (uint32)rfc3640.size();
    OSCL_DELETE(registry->lookupPayloadParserFactory(memFrag));
    PayloadParserRegistry::Cleanup();
}
