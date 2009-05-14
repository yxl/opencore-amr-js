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
 * @file pv_comms_io_data_buffer.cpp
 * @brief Media buffer to hold source data from Media Input modules and call
 * writeComplete to release source data memory back to media input modules.
 */

#ifndef PVMI_MIO_COMM_DATA_BUFFER_H_INCLUDED
#include "pvmi_mio_comm_data_buffer.h"
#endif
#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

PvmiMIOCommDataBufferCleanup::PvmiMIOCommDataBufferCleanup(Oscl_DefAlloc* in_gen_alloc,
        PvmiMediaTransfer* aMediaInput,
        PVMFCommandId aCmdId,
        OsclAny* aContext)
        : gen_alloc(in_gen_alloc),
        iMediaInput(aMediaInput),
        iCmdId(aCmdId),
        iContext(aContext)
{
    iLogger = PVLogger::GetLoggerObject("PvmiMIOCommDataBufferCleanup");
}

void PvmiMIOCommDataBufferCleanup::destruct_and_dealloc(OsclAny* ptr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVPvmiMIOCommDataBufferCleanup::destruct_and_dealloc: iCmdId=%d", iCmdId));

    if (!ptr)
        return;

    if (iMediaInput)
        iMediaInput->writeComplete(PVMFSuccess, iCmdId, iContext);

    if (!gen_alloc)
    {
        OsclMemAllocator my_alloc;
        my_alloc.deallocate(ptr);
    }
    else
    {
        gen_alloc->deallocate(ptr);
    }
}

PvmiMIOCommDataBufferAlloc::PvmiMIOCommDataBufferAlloc(Oscl_DefAlloc* opt_gen_alloc)
        : gen_alloc(opt_gen_alloc)
{
    iLogger = PVLogger::GetLoggerObject("PvmiMIOCommDataBufferAlloc");
}

OsclSharedPtr<PVMFMediaDataImpl> PvmiMIOCommDataBufferAlloc::allocate(PvmiMediaTransfer* aMediaInput,
        uint8* aData,
        uint32 aDataLength,
        PVMFCommandId aCmdId,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmiMIOCommDataBufferAlloc::allocate: aMediaInput=0x%x, aData=0x%x, aDataLength=%d, aCmdId=%d, aContext=0x%x",
                     aMediaInput, aData, aDataLength, aCmdId, aContext));

    uint aligned_refcnt_size;
    uint aligned_class_size = oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
    OsclRefCounter* my_refcnt;
    uint8* my_ptr;

    aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint aligned_cleanup_size = oscl_mem_aligned_size(sizeof(PvmiMIOCommDataBufferCleanup));
    if (!gen_alloc)
    {
        OsclMemAllocator my_alloc;
        my_ptr = (uint8*) my_alloc.allocate(aligned_refcnt_size + aligned_cleanup_size + aligned_class_size);
    }
    else
    {
        my_ptr = (uint8*) gen_alloc->allocate(aligned_refcnt_size + aligned_cleanup_size + aligned_class_size);
    }

    PvmiMIOCommDataBufferCleanup* cleanup_ptr =
        OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, PvmiMIOCommDataBufferCleanup(gen_alloc, aMediaInput, aCmdId, aContext));
    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, cleanup_ptr));
    my_ptr += aligned_refcnt_size + aligned_cleanup_size;

    PVMFMediaDataImpl* media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFSimpleMediaBuffer((OsclAny*)aData, aDataLength, my_refcnt));
    OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr, my_refcnt);
    return shared_media_data;
}



