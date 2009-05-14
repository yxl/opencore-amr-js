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
/**
 * @file pvmio_comm_data_buffer.h
 * @brief Media buffer to hold source data from Media Input modules and call
 * writeComplete to release source data memory back to media input modules.
 */

#ifndef PVMI_MIO_COMM_DATA_BUFFER_H_INCLUDED
#define PVMI_MIO_COMM_DATA_BUFFER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif

class PvmiMIOCommDataBufferCleanup : public OsclDestructDealloc
{
    public:
        PvmiMIOCommDataBufferCleanup(Oscl_DefAlloc* in_gen_alloc, PvmiMediaTransfer* aMediaInput,
                                     PVMFCommandId aCmdId, OsclAny* aContext = NULL);
        virtual ~PvmiMIOCommDataBufferCleanup() {};
        virtual void destruct_and_dealloc(OsclAny* ptr);

    private:
        Oscl_DefAlloc* gen_alloc;

        // Variables needed to call writeComplete
        PvmiMediaTransfer* iMediaInput;
        PVMFCommandId iCmdId;
        OsclAny* iContext;

        PVLogger *iLogger;
};

class PvmiMIOCommDataBufferAlloc
{
    public:
        PvmiMIOCommDataBufferAlloc(Oscl_DefAlloc* opt_gen_alloc = 0);
        virtual OsclSharedPtr<PVMFMediaDataImpl> allocate(PvmiMediaTransfer* aMediaInput, uint8* aData,
                uint32 aDataLength, PVMFCommandId aCmdId, OsclAny* aContext = NULL);

    private:
        Oscl_DefAlloc* gen_alloc;
        PVLogger* iLogger;
};

#endif // PVMIO_COMM_DATA_BUFFER_H_INCLUDED

