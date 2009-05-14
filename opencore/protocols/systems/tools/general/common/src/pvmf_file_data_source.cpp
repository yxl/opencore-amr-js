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
#include "pvmf_file_data_source.h"
//#include "pv_mime_string_utils.h"

OSCL_EXPORT_REF PVMFFileDataSource::PVMFFileDataSource(int32 aPortTag,
        unsigned bitrate,
        unsigned min_sample_sz,
        unsigned max_sample_sz):
        PVMFBufferDataSource(aPortTag, bitrate, min_sample_sz, max_sample_sz)
{
}

OSCL_EXPORT_REF PVMFFileDataSource::~PVMFFileDataSource()
{
    Stop();
    fclose(iReadFile);
}

uint32 total_bytes_read = 0;
void PVMFFileDataSource::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timerID);
    if (iIsFileDone)
        return;
    unsigned bytesToSend = timeoutInfo;
    int bytes_read = 0;
    if (bytesToSend <= 0)
        return;

    if (!IsConnected())
        return;

    // Create new media data buffer
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc->allocate(bytesToSend);
    PVMFSharedMediaDataPtr mediaData;
    int leavecode = 0;
    OSCL_TRY(leavecode, mediaData = PVMFMediaData::createMediaData(mediaDataImpl));
    OSCL_FIRST_CATCH_ANY(leavecode, return);

    // Retrieve memory fragment to write to
    OsclRefCounterMemFrag refCtrMemFrag;
    mediaData->getMediaFragment(0, refCtrMemFrag);
    if (refCtrMemFrag.getCapacity() < bytesToSend)
        return;

    if (!feof(iReadFile))
        bytes_read = fread((uint8*)refCtrMemFrag.getMemFragPtr(), 1, bytesToSend, iReadFile);
    //if(bytes_read)
    {
        mediaDataImpl->setMediaFragFilledLen(0, bytes_read);
        mediaData->setTimestamp(iTimestamp);
        iTimestamp += iSampleInterval;

        // Send frame to downstream node
        PVMFSharedMediaMsgPtr mediaMsg;
        convertToPVMFMediaMsg(mediaMsg, mediaData);
        QueueOutgoingMsg(mediaMsg);
        total_bytes_read += bytes_read;
    }
    if (!bytes_read)
    {
        OSCL_ASSERT(iObserver);
        iObserver->FileDataFinished();
        // Disconnect and stop
        iIsFileDone = true;
    }
}

