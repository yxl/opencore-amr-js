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
#ifndef PVMF_FILE_DATA_SOURCE_H_INCLUDED
#define PVMF_FILE_DATA_SOURCE_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

#ifndef PVMF_BUFFER_DATA_SOURCE_H_INCLUDED
#include "pvmf_buffer_data_source.h"
#endif

class PVMFFileDataSourceObserver
{
    public:
        virtual void FileDataFinished() = 0;
};

class PVMFFileDataSource : public PVMFBufferDataSource
{
    public:
        OSCL_IMPORT_REF PVMFFileDataSource(int32 aPortTag,
                                           unsigned bitrate,
                                           unsigned min_sample_sz,
                                           unsigned max_sample_sz);
        OSCL_IMPORT_REF virtual ~PVMFFileDataSource();

        // timer observer
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        bool OpenFile(char *aFileName)
        {
            iReadFile = fopen(aFileName, "rb");
            if (iReadFile)
            {
                iIsFileDone = false;
                return true;
            }
            return false;
        }

        void SetObserver(PVMFFileDataSourceObserver *aObserver)
        {
            iObserver = aObserver;
        }

    private:
        FILE *iReadFile;
        PVMFFileDataSourceObserver *iObserver;
        bool iIsFileDone;
};
#endif
