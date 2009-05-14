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

#ifndef ANDROID_AUDIO_MIO_H
#define ANDROID_AUDIO_MIO_H

#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMI_MEDIA_IO_OBSERVER_H_INCLUDED
#include "pvmi_media_io_observer.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMI_MEDIA_IO_CLOCK_EXTENSION_H_INCLUDED
#include "pvmi_media_io_clock_extension.h"
#endif

#ifndef ANDROID_AUDIO_OUTPUT_THREADSAFE_CALLBACK_AO_H_INCLUDED
#include "android_audio_output_threadsafe_callbacks.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifdef PERFORMANCE_MEASUREMENTS_ENABLED
#ifndef _PVPROFILE_H
#include "pvprofile.h"
#endif
#endif

#include <media/MediaPlayerInterface.h>

class PVLogger;
class PVMFMediaClock;

using namespace android;

// Write command handling
// the audio output thread queues the responses, this mio dequeues and processes them
class WriteResponse
{
public:
    WriteResponse(PVMFStatus s, PVMFCommandId id, const OsclAny* ctx, const PVMFTimestamp& ts) :
        iStatus(s), iCmdId(id), iContext(ctx), iTimestamp(ts)
    {}

        PVMFStatus iStatus;
        PVMFCommandId iCmdId;
        const OsclAny* iContext;
        PVMFTimestamp iTimestamp;
};

// Active timing support
class AndroidAudioMIOActiveTimingSupport :
    public PvmiClockExtensionInterface, public PVMFMediaClockStateObserver 
{
public:

    AndroidAudioMIOActiveTimingSupport(int32 minCorrection=0, int32 maxCorrection=0) :
        iClock(NULL),
        iClockNotificationsInf(NULL),
        iClockState(PVMFMediaClock::STOPPED),
        iAudioThreadSem(0),
        iStartTime(0),
        iFrameCount(0),
        iMsecsPerFrame(25),
        iDriverLatency(0),
        iUpdateClock(true),
        iMinCorrection(minCorrection),
        iMaxCorrection(maxCorrection)
   {
   }

    ~AndroidAudioMIOActiveTimingSupport()
    {
        if (iClock && iClockNotificationsInf)
        {
            iClockNotificationsInf->RemoveClockStateObserver(*this);
            iClock->DestroyMediaClockNotificationsInterface(iClockNotificationsInf);
            iClockNotificationsInf = NULL;
        }
    }

    //from PvmiClockExtensionInterface
    OSCL_IMPORT_REF PVMFStatus SetClock(PVMFMediaClock* clockVal);

    // from PVMFMediaClockStateObserver
    void ClockStateUpdated();

    //from PVInterface
    OSCL_IMPORT_REF void addRef() {}
    OSCL_IMPORT_REF void removeRef() {}
    OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
    void NotificationsInterfaceDestroyed();

    void queryUuid(PVUuid& uuid);

    void ForceClockUpdate() { iUpdateClock = true; }
    void UpdateClock();

    PVMFMediaClock::PVMFMediaClockState clockState() { return iClockState; }
    void setThreadSemaphore(OsclSemaphore* s) { iAudioThreadSem = s; }

    void setDriverLatency(uint32 latency);
    void incFrameCount(uint32_t numFrames) { iFrameCount += numFrames; }
    void setFrameRate(float msecsPerFrame) { iMsecsPerFrame = msecsPerFrame; }
    float msecsPerFrame() { return iMsecsPerFrame; }

private:
    PVMFMediaClock* iClock;
    PVMFMediaClockNotificationsInterface *iClockNotificationsInf;
    PVMFMediaClock::PVMFMediaClockState iClockState;
    OsclSemaphore* iAudioThreadSem;

    uint32 iStartTime;
    uint32 iFrameCount;
    float iMsecsPerFrame;
    uint32 iDriverLatency;
    bool iUpdateClock;
    int32 iMinCorrection;
    int32 iMaxCorrection;
};

// Audio MIO component
class AndroidAudioMIO : public OsclTimerObject,
                 public PvmiMIOControl,
                 public PvmiMediaTransfer,
                 public PvmiCapabilityAndConfig
{
public:
    OSCL_IMPORT_REF AndroidAudioMIO(const char* name);

    OSCL_IMPORT_REF ~AndroidAudioMIO();

    // APIs from PvmiMIOControl

    PVMFStatus connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver);

    PVMFStatus disconnect(PvmiMIOSession aSession);

    virtual PVMFCommandId QueryUUID(const PvmfMimeString& aMimeType,
            Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
            bool aExactUuidsOnly=false, const OsclAny* aContext=NULL);

    virtual PVMFCommandId QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext=NULL);

    PvmiMediaTransfer* createMediaTransfer(PvmiMIOSession& aSession, PvmiKvp* read_formats=NULL, int32 read_flags=0,
                        PvmiKvp* write_formats=NULL, int32 write_flags=0);

    void deleteMediaTransfer(PvmiMIOSession& aSession, PvmiMediaTransfer* media_transfer) {}

    PVMFCommandId Init(const OsclAny* aContext=NULL);

    PVMFCommandId Reset(const OsclAny* aContext=NULL);

    PVMFCommandId Start(const OsclAny* aContext=NULL);

    PVMFCommandId Pause(const OsclAny* aContext=NULL);

    PVMFCommandId Flush(const OsclAny* aContext=NULL);

    virtual PVMFCommandId DiscardData(const OsclAny* aContext=NULL);

    virtual PVMFCommandId DiscardData(PVMFTimestamp aTimestamp=0, const OsclAny* aContext=NULL);

    virtual PVMFCommandId Stop(const OsclAny* aContext=NULL);

    virtual PVMFCommandId CancelAllCommands(const OsclAny* aContext=NULL);

    virtual PVMFCommandId CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext=NULL);

    virtual void cancelCommand(PVMFCommandId aCmdId) {}

    void ThreadLogon();

    void ThreadLogoff();

    // APIs from PvmiMediaTransfer

    void setPeer(PvmiMediaTransfer* aPeer);

    void useMemoryAllocators(OsclMemAllocator* write_alloc=NULL) {}

    PVMFCommandId writeAsync(uint8 format_type, int32 format_index,
                            uint8* data, uint32 data_len,
                            const PvmiMediaXferHeader& data_header_info,
                            OsclAny* aContext=NULL);

    void writeComplete(PVMFStatus aStatus, PVMFCommandId  write_cmd_id, OsclAny* aContext) {}

    PVMFCommandId readAsync(uint8* data, uint32 max_data_len, OsclAny* aContext=NULL,
            int32* formats=NULL, uint16 num_formats=0) {
        OsclError::Leave(OsclErrNotSupported);
        return -1;
    }

    void readComplete(PVMFStatus aStatus, PVMFCommandId  read_cmd_id, int32 format_index,
                    const PvmiMediaXferHeader& data_header_info, OsclAny* aContext) {}

    void statusUpdate(uint32 status_flags) {}

    virtual void cancelAllCommands() { returnAllBuffers(); }

    // Pure virtuals from PvmiCapabilityAndConfig

    void setObserver (PvmiConfigAndCapabilityCmdObserver* aObserver) { OSCL_UNUSED_ARG(aObserver); }

    PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters, int& num_parameter_elements, PvmiCapabilityContext aContext);

    PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

    void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext) {
        OsclError::Leave(OsclErrNotSupported);
    }

    void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters, int num_parameter_elements) {
        OsclError::Leave(OsclErrNotSupported);
    }

    void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext) {
        OsclError::Leave(OsclErrNotSupported);
    }

    virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp * & aRet_kvp);

    PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context=NULL) {
        OsclError::Leave(OsclErrNotSupported);
        return -1;
    }

    uint32 getCapabilityMetric (PvmiMIOSession aSession) { return 0; }

    PVMFStatus verifyParametersSync (PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

    // Functions specific to this MIO
    void setAudioSink(const sp<MediaPlayerInterface::AudioSink>& audioSink);

    virtual void writeAudioBuffer(uint8* aData, uint32 aDataLen, PVMFCommandId cmdId,
            OsclAny* aContext, PVMFTimestamp aTimestamp) {}

protected:
    // From OsclTimerObject
    void Run();
    void sendResponse(PVMFCommandId cmdid, const OsclAny* context, PVMFTimestamp timestamp);

    // Specific to this MIO
    void initData();

    void Cleanup();

    virtual void returnAllBuffers() {}

    void ResetData();

    // request active object which the audio output thread uses to schedule this timer object to run
    AndroidAudioOutputThreadSafeCallbackAO *iWriteCompleteAO;

    // write response queue, needs to use lock mechanism to access
    Oscl_Vector<WriteResponse,OsclMemAllocator> iWriteResponseQueue;

    // lock used to access the write response queue
    OsclMutex iWriteResponseQueueLock;

    // From setPeer
    PvmiMediaTransfer* iPeer;

    // The PvmiMIOControl class observer.
    PvmiMIOObserver* iObserver;

    // for generating command IDs
    uint32 iCommandCounter;

    //State
    enum PVAudioMIOState
    {
        STATE_MIO_IDLE
        ,STATE_MIO_LOGGED_ON
        ,STATE_MIO_INITIALIZED
        ,STATE_MIO_STARTED
        ,STATE_MIO_PAUSED
    };
    PVAudioMIOState iState;

    // Control command handling.
    class CommandResponse
    {
    public:
        CommandResponse(PVMFStatus s,PVMFCommandId id,const OsclAny* ctx)
            :iStatus(s),iCmdId(id),iContext(ctx)
        {}

        PVMFStatus iStatus;
        PVMFCommandId iCmdId;
        const OsclAny* iContext;
    };
    Oscl_Vector<CommandResponse,OsclMemAllocator> iCommandResponseQueue;
    PVMFCommandId QueueCmdResponse(PVMFStatus status, const OsclAny* aContext);
    void ProcessWriteResponseQueue();

    // Audio parameters.
    PVMFFormatType iAudioFormat;
    int32 iAudioNumChannels;
    bool iAudioNumChannelsValid;
    int32 iAudioSamplingRate;
    bool iAudioSamplingRateValid;

    bool iAudioThreadCreatedAndMIOConfigured;

    // For logging
    PVLogger* iLogger;

    // For implementing the write flow control
    bool iWriteBusy;
    bool CheckWriteBusy(uint32);

    bool iFlushPending;
    uint32 iDataQueued;

    sp<MediaPlayerInterface::AudioSink>   mAudioSink;
};

#endif // ANDROID_AUDIO_MIO_H

