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
#ifndef PVMF_RTP_JITTER_BUFFER_FACTORY_H_INCLUDED
#include "pvmf_rtp_jitter_buffer_factory.h"
#endif

#ifndef PVMF_RTP_JITTER_BUFFER_IMPL_H_INCLUDED
#include "pvmf_rtp_jitter_buffer_impl.h"
#endif

OSCL_EXPORT_REF PVMFJitterBuffer* RTPJitterBufferFactory::Create(const PVMFJitterBufferConstructParams& aJBCreationData)
{
    return PVMFRTPJitterBufferImpl::New(aJBCreationData);
}

OSCL_EXPORT_REF void RTPJitterBufferFactory::Destroy(PVMFJitterBuffer*& aJitterBuffer)
{
    OSCL_DELETE(aJitterBuffer);
    aJitterBuffer = NULL;
}
