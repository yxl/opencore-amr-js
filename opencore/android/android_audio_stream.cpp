/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "AudioStream"
#include <utils/Log.h>

#include "android_audio_stream.h"

#include <sys/prctl.h>
#include <sys/resource.h>
#include <utils/threads.h>
#include <media/AudioTrack.h>

using namespace android;

// buffer to convert 8-bit samples to 16-bit (in 16-bit samples)
static uint32 kConversionBufferSize = 4096;

/*
/ Packet Video Audio MIO component
/
/ This implementation routes audio to a stream interface
*/
OSCL_EXPORT_REF AndroidAudioStream::AndroidAudioStream() :
    AndroidAudioMIO("AndroidAudioStream"),
    iActiveTiming(NULL), mClockUpdated(false)
{
    // create active timing object
    LOGV("constructor");
    OsclMemAllocator alloc;
    OsclAny*ptr=alloc.allocate(sizeof(AndroidAudioMIOActiveTimingSupport));
    if (ptr) {
        iActiveTiming = new(ptr)AndroidAudioMIOActiveTimingSupport(0, 0);
    }
}

OSCL_EXPORT_REF AndroidAudioStream::~AndroidAudioStream()
{
    LOGV("destructor");
    // cleanup active timing object
    if (iActiveTiming) {
        iActiveTiming->~AndroidAudioMIOActiveTimingSupport();
        OsclMemAllocator alloc;
        alloc.deallocate(iActiveTiming);
    }
}

PVMFCommandId AndroidAudioStream::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    // check for active timing extension
    if (iActiveTiming && (aUuid == PvmiClockExtensionInterfaceUuid)) {
        PvmiClockExtensionInterface* myInterface = OSCL_STATIC_CAST(PvmiClockExtensionInterface*,iActiveTiming);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return QueueCmdResponse(PVMFSuccess, aContext);
    }

    // pass to base class
    else return AndroidAudioMIO::QueryInterface(aUuid, aInterfacePtr, aContext);
}

PVMFCommandId AndroidAudioStream::QueryUUID(const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                        bool aExactUuidsOnly, const OsclAny* aContext)
{
    aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID);
    if (iActiveTiming) {
        PVUuid uuid;
        iActiveTiming->queryUuid(uuid);
        aUuids.push_back(uuid);
    }
    return QueueCmdResponse(PVMFSuccess, aContext);
}

void AndroidAudioStream::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                        int num_elements, PvmiKvp * & aRet_kvp)
{
    AndroidAudioMIO::setParametersSync(aSession, aParameters, num_elements, aRet_kvp);

    // initialize audio sink when we have enough information
    if (iAudioSamplingRateValid && iAudioNumChannelsValid && iAudioFormat != PVMF_MIME_FORMAT_UNKNOWN) {
        mAudioSink->open(iAudioSamplingRate, iAudioNumChannels, ((iAudioFormat==PVMF_MIME_PCM8)?AudioSystem::PCM_8_BIT:AudioSystem::PCM_16_BIT));

        // reset flags for next time
        iAudioSamplingRateValid = false;
        iAudioNumChannelsValid  = false;
        iAudioFormat = PVMF_MIME_FORMAT_UNKNOWN;
    }
}

void AndroidAudioStream::writeAudioBuffer(uint8* aData, uint32 aDataLen, PVMFCommandId cmdId, OsclAny* aContext, PVMFTimestamp aTimestamp)
{
	mAudioSink->write(aData, aDataLen);
    sendResponse(cmdId, aContext, aTimestamp);
}

