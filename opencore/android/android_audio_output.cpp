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
#define LOG_TAG "AudioOutput"
#include <utils/Log.h>

#include "android_audio_output.h"

#include <sys/prctl.h>
#include <sys/resource.h>
#include <utils/threads.h>
#include <media/AudioTrack.h>

using namespace android;

// TODO: dynamic buffer count based on sample rate and # channels
static const int kNumOutputBuffers = 4;

// maximum allowed clock drift before correction
static const int32 kMaxClockDriftInMsecs = 25;    // should be tight enough for reasonable sync
static const int32 kMaxClockCorrection = 100;     // maximum clock correction per update

/*
/ Packet Video Audio MIO component
/
/ This implementation routes audio to AudioFlinger. Audio buffers are
/ enqueued in a message queue to a separate audio output thread. Once
/ the buffers have been successfully written, they are returned through
/ another message queue to the MIO and from there back to the engine.
/ This separation is necessary because most of the PV API is not
/ thread-safe.
*/
OSCL_EXPORT_REF AndroidAudioOutput::AndroidAudioOutput() :
    AndroidAudioMIO("AndroidAudioOutput"),
    iExitAudioThread(false),
    iReturnBuffers(false),
    iActiveTiming(NULL)
{
    LOGV("constructor");
    iClockTimeOfWriting_ns = 0;
    iInputFrameSizeInBytes = 0;

    // semaphore used to communicate between this  mio and the audio output thread
    iAudioThreadSem = new OsclSemaphore();
    iAudioThreadSem->Create(0);
    iAudioThreadTermSem = new OsclSemaphore();
    iAudioThreadTermSem->Create(0);
    iAudioThreadReturnSem = new OsclSemaphore();
    iAudioThreadReturnSem->Create(0);
    iAudioThreadCreatedSem = new OsclSemaphore();
    iAudioThreadCreatedSem->Create(0);

    // locks to access the queues by this mio and by the audio output thread
    iOSSRequestQueueLock.Create();
    iOSSRequestQueue.reserve(iWriteResponseQueue.capacity());

    // create active timing object
    OsclMemAllocator alloc;
    OsclAny*ptr=alloc.allocate(sizeof(AndroidAudioMIOActiveTimingSupport));
    if (ptr) {
        iActiveTiming=new(ptr)AndroidAudioMIOActiveTimingSupport(kMaxClockDriftInMsecs, kMaxClockCorrection);
        iActiveTiming->setThreadSemaphore(iAudioThreadSem);
    }
}

OSCL_EXPORT_REF AndroidAudioOutput::~AndroidAudioOutput()
{
    LOGV("destructor");

    // make sure output thread has exited
    RequestAndWaitForThreadExit();

    // cleanup active timing object
    if (iActiveTiming) {
        iActiveTiming->~AndroidAudioMIOActiveTimingSupport();
        OsclMemAllocator alloc;
        alloc.deallocate(iActiveTiming);
    }

    // clean up some thread interface objects
    iAudioThreadSem->Close();
    delete iAudioThreadSem;
    iAudioThreadTermSem->Close();
    delete iAudioThreadTermSem;
    iAudioThreadReturnSem->Close();
    delete iAudioThreadReturnSem;
    iAudioThreadCreatedSem->Close();
    delete iAudioThreadCreatedSem;

    iOSSRequestQueueLock.Close();
}

PVMFCommandId AndroidAudioOutput::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    LOGV("QueryInterface in");
    // check for active timing extension
    if (iActiveTiming && (aUuid == PvmiClockExtensionInterfaceUuid)) {
        PvmiClockExtensionInterface* myInterface = OSCL_STATIC_CAST(PvmiClockExtensionInterface*,iActiveTiming);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return QueueCmdResponse(PVMFSuccess, aContext);
    }

    // pass to base class
    else return AndroidAudioMIO::QueryInterface(aUuid, aInterfacePtr, aContext);
}

PVMFCommandId AndroidAudioOutput::QueryUUID(const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                        bool aExactUuidsOnly, const OsclAny* aContext)
{
    LOGV("QueryUUID in");
    int32 err;
    OSCL_TRY(err,
        aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID);
        if (iActiveTiming) {
            PVUuid uuid;
            iActiveTiming->queryUuid(uuid);
            aUuids.push_back(uuid);
        }
    );
    return QueueCmdResponse(err == OsclErrNone ? PVMFSuccess : PVMFFailure, aContext);
}

PVMFCommandId AndroidAudioOutput::Stop(const OsclAny* aContext)
{
    LOGV("AndroidAudioOutput Stop (%p)", aContext);
    // return all buffer by writecomplete
    returnAllBuffers();
    return AndroidAudioMIO::Stop(aContext);
}

PVMFCommandId AndroidAudioOutput::Reset(const OsclAny* aContext)
{
    LOGV("AndroidAudioOutput Reset (%p)", aContext);
    // return all buffer by writecomplete
    returnAllBuffers();
    // request output thread to exit
    RequestAndWaitForThreadExit();
    return AndroidAudioMIO::Reset(aContext);
}

void AndroidAudioOutput::cancelCommand(PVMFCommandId command_id)
{
    LOGV("cancelCommand (%u) RequestQ size %d", command_id,iOSSRequestQueue.size());
    iOSSRequestQueueLock.Lock();
    for (uint32 i = 0; i < iOSSRequestQueue.size(); i++) {
        if (iOSSRequestQueue[i].iCmdId == command_id) {
            iDataQueued -= iOSSRequestQueue[i].iDataLen;
            if (iPeer)
                iPeer->writeComplete(PVMFSuccess, iOSSRequestQueue[i].iCmdId, (OsclAny*)iOSSRequestQueue[i].iContext);
            iOSSRequestQueue.erase(&iOSSRequestQueue[i]);
            break;
        }
    }
    iOSSRequestQueueLock.Unlock();
    LOGV("cancelCommand data queued = %u", iDataQueued);

    ProcessWriteResponseQueue();
}

void AndroidAudioOutput::returnAllBuffers()
{
    LOGV("returnAllBuffers RequestQ size %d",iOSSRequestQueue.size());
    iOSSRequestQueueLock.Lock();
    while (iOSSRequestQueue.size()) {
        iDataQueued -= iOSSRequestQueue[0].iDataLen;
        if (iPeer)
            iPeer->writeComplete(PVMFSuccess, iOSSRequestQueue[0].iCmdId, (OsclAny*)iOSSRequestQueue[0].iContext);
        iOSSRequestQueue.erase(&iOSSRequestQueue[0]);
    }
    iOSSRequestQueueLock.Unlock();
    LOGV("returnAllBuffers data queued = %u", iDataQueued);
    if (iAudioThreadSem && iAudioThreadCreatedAndMIOConfigured) {
        LOGV("signal thread to return buffers");
        iReturnBuffers = true;
        iAudioThreadSem->Signal();
        while (iAudioThreadReturnSem->Wait() != OsclProcStatus::SUCCESS_ERROR)
            ;
        LOGV("return buffers signal completed");
    }
}


PVMFCommandId AndroidAudioOutput::DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext)
{
    LOGV("DiscardData timestamp(%u) RequestQ size %d", aTimestamp,iOSSRequestQueue.size());
   
    if(iActiveTiming){
        LOGV("Force clock update");
        iActiveTiming->ForceClockUpdate();
	}

    bool sched = false;
    PVMFCommandId audcmdid;
    const OsclAny* context;
    PVMFTimestamp timestamp;

    // the OSSRequest queue should be drained
    // all the buffers in them should be returned to the engine
    // writeComplete cannot be called from here
    // thus the best way is to queue the buffers onto the write response queue
    // and then call RunIfNotReady
    iOSSRequestQueueLock.Lock();
    for (int32 i = (iOSSRequestQueue.size() - 1); i >= 0; i--) {
        if (iOSSRequestQueue[i].iTimestamp < aTimestamp) {
            audcmdid = iOSSRequestQueue[i].iCmdId;
            context = iOSSRequestQueue[i].iContext;
            timestamp = iOSSRequestQueue[i].iTimestamp;
            iDataQueued -= iOSSRequestQueue[i].iDataLen;
            LOGV("discard buffer (%d) context(%p) timestamp(%u) Datalen(%d)", audcmdid,context, timestamp,iOSSRequestQueue[i].iDataLen);
            iOSSRequestQueue.erase(&iOSSRequestQueue[i]);
            sched = true;

            WriteResponse resp(PVMFSuccess, audcmdid, context, timestamp);
            iWriteResponseQueueLock.Lock();
            iWriteResponseQueue.push_back(resp);
            iWriteResponseQueueLock.Unlock();
        }
    }
    LOGV("DiscardData data queued = %u, setting flush pending", iDataQueued);
    iFlushPending=true;

    iOSSRequestQueueLock.Unlock();

    if (sched)
        RunIfNotReady();

    return AndroidAudioMIO::DiscardData(aTimestamp, aContext);
}

void AndroidAudioOutput::RequestAndWaitForThreadExit()
{
    LOGV("RequestAndWaitForThreadExit In");
    if (iAudioThreadSem && iAudioThreadCreatedAndMIOConfigured) {
        LOGV("signal thread for exit");
        iExitAudioThread = true;
        iAudioThreadSem->Signal();
        while (iAudioThreadTermSem->Wait() != OsclProcStatus::SUCCESS_ERROR)
            ;
        LOGV("thread term signal received");
        iAudioThreadCreatedAndMIOConfigured = false;
    }
}

void AndroidAudioOutput::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                        int num_elements, PvmiKvp * & aRet_kvp)
{
    LOGV("AndroidAudioOutput setParametersSync In");
    AndroidAudioMIO::setParametersSync(aSession, aParameters, num_elements, aRet_kvp);

    // initialize thread when we have enough information
    if (iAudioSamplingRateValid && iAudioNumChannelsValid && iAudioFormat != PVMF_MIME_FORMAT_UNKNOWN) {
        LOGV("start audio thread");
        OsclThread AudioOutput_Thread;
        iExitAudioThread = false;
        iReturnBuffers = false;
        OsclProcStatus::eOsclProcError ret = AudioOutput_Thread.Create((TOsclThreadFuncPtr)start_audout_thread_func,
                                                    0, (TOsclThreadFuncArg)this, Start_on_creation);

        //Don't signal the MIO node that the configuration is complete until the driver latency has been set
        while (iAudioThreadCreatedSem->Wait() != OsclProcStatus::SUCCESS_ERROR) 
           ;

        if(OsclProcStatus::SUCCESS_ERROR == ret){
            iAudioThreadCreatedAndMIOConfigured = true;
            if(iObserver){
                LOGV("event PVMFMIOConfigurationComplete to peer");
                iObserver->ReportInfoEvent(PVMFMIOConfigurationComplete);
            }
        }
        else{
            iAudioThreadCreatedAndMIOConfigured = false;
            if(iObserver){
                LOGE("event PVMFErrResourceConfiguration to peer");
                iObserver->ReportErrorEvent(PVMFErrResourceConfiguration);
            }
        }
    }
    LOGV("AndroidAudioOutput setParametersSync out");
}

void AndroidAudioOutput::Run()
{
    // if running, update clock
    if ((iState == STATE_MIO_STARTED) && iInputFrameSizeInBytes) {
        uint32 msecsQueued = iDataQueued / iInputFrameSizeInBytes * iActiveTiming->msecsPerFrame();
        LOGV("%u msecs of data queued, %u bytes of data queued", msecsQueued,iDataQueued);
        iActiveTiming->UpdateClock();
    }
    AndroidAudioMIO::Run();
}

void AndroidAudioOutput::writeAudioBuffer(uint8* aData, uint32 aDataLen, PVMFCommandId cmdId, OsclAny* aContext, PVMFTimestamp aTimestamp)
{
    // queue up buffer and signal audio thread to process it
    LOGV("writeAudioBuffer :: DataLen(%d), cmdId(%d), Context(%p), Timestamp (%d)",aDataLen, cmdId, aContext, aTimestamp);
    OSSRequest req(aData, aDataLen, cmdId, aContext, aTimestamp);
    iOSSRequestQueueLock.Lock();
    iOSSRequestQueue.push_back(req);
    iDataQueued += aDataLen;

    // wake up the audio output thread to process this buffer only if clock has started running
    if (iActiveTiming->clockState() == PVMFMediaClock::RUNNING) {
        LOGV("clock is ticking signal thread for data");
        iAudioThreadSem->Signal();
    }
    iOSSRequestQueueLock.Unlock();
}

//------------------------------------------------------------------------
// audio thread
//

#undef LOG_TAG
#define LOG_TAG "audiothread"

// this is the audio output thread
// used to send data to the linux audio output device
// communicates with the audio MIO via a semaphore, a request queue and a response queue
/*static*/ int AndroidAudioOutput::start_audout_thread_func(TOsclThreadFuncArg arg)
{
    LOGV("start_audout_thread_func in");
    AndroidAudioOutput *obj = (AndroidAudioOutput *)arg;
    prctl(PR_SET_NAME, (unsigned long) "audio out", 0, 0, 0);
    int err = obj->audout_thread_func();
    LOGV("start_audout_thread_func out return code %d",err);
    return err;
}

int AndroidAudioOutput::audout_thread_func()
{
    enum { IDLE, STOPPED, STARTED, PAUSED } state = IDLE;
    int64_t lastClock = 0;

    // LOGD("audout_thread_func");

#if defined(HAVE_SCHED_SETSCHEDULER) && defined(HAVE_GETTID)
    setpriority(PRIO_PROCESS, gettid(), ANDROID_PRIORITY_AUDIO);
#endif

    if (iAudioNumChannelsValid == false || iAudioSamplingRateValid == false || iAudioFormat == PVMF_MIME_FORMAT_UNKNOWN) {
        LOGE("channel count or sample rate is invalid");
        return -1;
    }

    LOGV("Creating AudioTrack object: rate=%d, channels=%d, buffers=%d", iAudioSamplingRate, iAudioNumChannels, kNumOutputBuffers);
    status_t ret = mAudioSink->open(iAudioSamplingRate, iAudioNumChannels, ((iAudioFormat==PVMF_MIME_PCM8)?AudioSystem::PCM_8_BIT:AudioSystem::PCM_16_BIT), kNumOutputBuffers);
    iAudioSamplingRateValid = false; // purpose of these flags is over here, reset these for next validation recording.
    iAudioNumChannelsValid  = false;
    iAudioFormat = PVMF_MIME_FORMAT_UNKNOWN;
    if (ret != 0) {
        iAudioThreadCreatedAndMIOConfigured = false;
        LOGE("Error creating AudioTrack");
        return -1;
    }

    // calculate timing data
    int outputFrameSizeInBytes = mAudioSink->frameSize();
    float msecsPerFrame = mAudioSink->msecsPerFrame();
    uint32 latency = mAudioSink->latency();
    LOGV("driver latency(%u),outputFrameSizeInBytes(%d),msecsPerFrame(%f),frame count(%d)", latency,outputFrameSizeInBytes,msecsPerFrame,mAudioSink->frameCount());

    // initialize active timing
    iActiveTiming->setFrameRate(msecsPerFrame);
    iActiveTiming->setDriverLatency(latency);

    iAudioThreadCreatedSem->Signal();
    // this must be set after iActiveTiming->setFrameRate to prevent race
    // condition in Run()
    iInputFrameSizeInBytes = outputFrameSizeInBytes;

    // buffer management
    uint32 bytesAvailInBuffer = 0;
    uint32 bytesToWrite;
    uint32 bytesWritten;
    uint8* data = 0;
    uint32 len = 0;
    PVMFCommandId cmdid = 0;
    const OsclAny* context = 0;
    PVMFTimestamp timestamp = 0;

    // wait for signal from MIO thread
    LOGV("wait for signal");
    iAudioThreadSem->Wait();
    LOGV("ready to work");

    while (1)
    {
        // if paused, stop the output track
        switch (iActiveTiming->clockState()) {
        case PVMFMediaClock::RUNNING:
            // start output
            if (state != STARTED) {
                if (iFlushPending) {
                    LOGV("flush");
                    mAudioSink->flush();
                    iFlushPending = false;
                    bytesAvailInBuffer = 0;
                    iClockTimeOfWriting_ns = 0;
                    // discard partial buffer and send response to MIO
                    if (data && len) {
                        LOGV("discard partial buffer and send response to MIO");
                        sendResponse(cmdid, context, timestamp);
                        data = 0;
                        len = 0;
                    }
                }
                if (iDataQueued) {
					LOGV("start");
					mAudioSink->start();
					state = STARTED;
                } else {
                    LOGV("clock running and no data queued - don't start track");
                }
            }
            else{
                LOGV("audio sink already in started state");
            }
            break;
        case PVMFMediaClock::STOPPED:
             LOGV("clock has been stopped...");
        case PVMFMediaClock::PAUSED:
            if (state == STARTED) {
                LOGV("pause");
                mAudioSink->pause();
            }
            state = PAUSED;
            if(!iExitAudioThread && !iReturnBuffers) {
                LOGV("wait");
                iAudioThreadSem->Wait();
                LOGV("awake");
            }
            break;
        default:
            break;
        }
        // if out of data, check the request queue
        if (len == 0) {
            //LOGV("no playable data, Request Q size %d",iOSSRequestQueue.size());
            iOSSRequestQueueLock.Lock();
            bool empty = iOSSRequestQueue.empty();
            if (!empty) {
                data = iOSSRequestQueue[0].iData;
                len = iOSSRequestQueue[0].iDataLen;
                cmdid = iOSSRequestQueue[0].iCmdId;
                context = iOSSRequestQueue[0].iContext;
                timestamp = iOSSRequestQueue[0].iTimestamp;
                iDataQueued -= len;
                iOSSRequestQueue.erase(&iOSSRequestQueue[0]);
                LOGV("receive buffer (%d), timestamp = %u data queued = %u", cmdid, timestamp,iDataQueued);
            }
            iOSSRequestQueueLock.Unlock();

            // if queue is empty, wait for more work
            // FIXME: Why do end up here so many times when stopping?
            if (empty && !iExitAudioThread && !iReturnBuffers) {
                LOGV("queue is empty, wait for more work");
                iAudioThreadSem->Wait();
            }

            // empty buffer means "End-Of-Stream" - send response to MIO
            else if (len == 0) {
                LOGV("EOS");
                state = STOPPED;
                mAudioSink->stop();
                if(!iExitAudioThread){
                    nsecs_t interval_nanosec = 0; // Interval between last writetime and EOS processing time in nanosec
                    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
                    LOGV("now = %lld ,iClockTimeOfWriting_ns = %lld",now, iClockTimeOfWriting_ns);
                    if(now >= iClockTimeOfWriting_ns){
                        interval_nanosec = now - iClockTimeOfWriting_ns;
                    }
                    else{ //when timevalue wraps
                         interval_nanosec = 0;
                    }
                    LOGV(" I am early,going for sleep for latency = %u millsec, interval_nanosec = %lld",latency, interval_nanosec);
                    struct timespec requested_time_delay, remaining;
                    requested_time_delay.tv_sec = latency/1000; 
                    nsecs_t latency_nanosec = (latency%1000)*1000*1000;
                    if(interval_nanosec < latency_nanosec){
                        requested_time_delay.tv_nsec = latency_nanosec - interval_nanosec;
                        nanosleep (&requested_time_delay, &remaining);
                        LOGV(" Wow, what a great siesta....send response to engine");
                    }
                    else{// interval is greater than latency so no need of sleep
                        LOGV(" No time to sleep :( send response to engine anyways");
                    }
                    iClockTimeOfWriting_ns = 0;
                    sendResponse(cmdid, context, timestamp);
                }
            }
        }

        if (iReturnBuffers) {
            LOGV("Return buffers from the audio thread");
            if (len) sendResponse(cmdid, context, timestamp);
            iReturnBuffers=false;
            data = 0;
            len = 0;
            iAudioThreadReturnSem->Signal();
        }

        // check for exit signal
        if (iExitAudioThread) {
            LOGV("exit received");
            if (len) sendResponse(cmdid, context, timestamp);
            break;
        }

        // data to output?
        if (len && (state == STARTED) && !iExitAudioThread) {

            // always align to AudioFlinger buffer boundary
            if (bytesAvailInBuffer == 0)
                bytesAvailInBuffer = mAudioSink->bufferSize();

                bytesToWrite = bytesAvailInBuffer > len ? len : bytesAvailInBuffer;
                //LOGV("16 bit :: cmdid = %d, len = %u, bytesAvailInBuffer = %u, bytesToWrite = %u", cmdid, len, bytesAvailInBuffer, bytesToWrite);
                bytesWritten = mAudioSink->write(data, bytesToWrite);
                if (bytesWritten != bytesToWrite) {
                    LOGE("Error writing audio data");
                    iAudioThreadSem->Wait();
                }
                data += bytesWritten;
                len -= bytesWritten;
                iClockTimeOfWriting_ns = systemTime(SYSTEM_TIME_MONOTONIC);


            // count bytes sent
            bytesAvailInBuffer -= bytesWritten;

            // update frame count for latency calculation
            iActiveTiming->incFrameCount(bytesWritten / outputFrameSizeInBytes);
            //LOGV("outputFrameSizeInBytes = %u,bytesWritten = %u,bytesAvailInBuffer = %u", outputFrameSizeInBytes,bytesWritten,bytesAvailInBuffer);
            // if done with buffer - send response to MIO
            if (data && !len) {
                LOGV("done with the data cmdid %d, context %p, timestamp %d ",cmdid, context, timestamp);
                sendResponse(cmdid, context, timestamp);
                data = 0;
            }
        }
    } // while loop

    LOGV("stop and delete track");
    mAudioSink->stop();
    iClockTimeOfWriting_ns = 0;

    // LOGD("audout_thread_func exit");
    iAudioThreadTermSem->Signal();

    return 0;
}

