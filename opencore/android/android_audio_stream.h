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

#ifndef ANDROID_AUDIO_STREAM_H
#define ANDROID_AUDIO_STREAM_H

#include <utils/Errors.h>

#include "android_audio_mio.h"
#include "utils/Timers.h"

using namespace android;

class AndroidAudioStream : public AndroidAudioMIO
{
public:
    OSCL_IMPORT_REF AndroidAudioStream();
    OSCL_IMPORT_REF ~AndroidAudioStream();

    virtual PVMFCommandId QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid,
            OsclMemAllocator>& aUuids, bool aExactUuidsOnly=false,
            const OsclAny* aContext=NULL);

    virtual PVMFCommandId QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr,
            const OsclAny* aContext=NULL);

    virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
            int num_elements, PvmiKvp * & aRet_kvp);

private:

    virtual void writeAudioBuffer(uint8* aData, uint32 aDataLen, PVMFCommandId cmdId,
            OsclAny* aContext, PVMFTimestamp aTimestamp);

    // active timing
    AndroidAudioMIOActiveTimingSupport* iActiveTiming;

    // number of bytes in an input frame
    int iInputFrameSizeInBytes;
    bool mClockUpdated;
};

#endif // ANDROID_AUDIO_STREAM_H

