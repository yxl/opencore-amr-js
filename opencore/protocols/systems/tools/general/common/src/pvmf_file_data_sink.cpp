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
#include "pvmf_file_data_sink.h"
#include "pvmf_media_msg_format_ids.h"

OSCL_EXPORT_REF PVMFFileDataSink::PVMFFileDataSink(int32 aPortTag)
        : PVMFBufferDataSink(aPortTag)
{
}

OSCL_EXPORT_REF PVMFFileDataSink::~PVMFFileDataSink()
{
    fclose(iWriteFile);
}

// PVMFPortInterface virtuals

PVMFStatus PVMFFileDataSink::PutData(PVMFSharedMediaMsgPtr aMsg)
{
    iNumPktsReceived++;
    PVUid32 msgFormatID = aMsg->getFormatID();
    if (msgFormatID < PVMF_MEDIA_CMD_FORMAT_IDS_START)
    {
        //this code should be run for media data
        // object only
        PVMFSharedMediaDataPtr mediaData;
        convertToPVMFMediaData(mediaData, aMsg);
        iNumBytesReceived += mediaData->getFilledSize();

        if (mediaData->getErrorsFlag())
            iNumPktErrorsReceived++;
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
        mediaData->getMediaDataImpl(mediaDataImpl);
        for (uint i = 0; i < mediaDataImpl->getNumFragments() ; i++)
        {
            OsclRefCounterMemFrag copy_frag;
            mediaDataImpl->getMediaFragment(i, copy_frag);
            fwrite(copy_frag.getMemFragPtr(), 1, copy_frag.getMemFragSize(), iWriteFile);
        }
    }
    return PVMFSuccess;
}
