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
#define LOG_TAG "AudioMIO"
#include <utils/Log.h>

#include "android_audio_mio.h"
#include "pvlogger.h"
#include "pv_mime_string_utils.h"
#include "oscl_snprintf.h"
#include "oscl_mem.h"
#include "oscl_dll.h"
#include "oscl_mem.h"

#include <sys/prctl.h>

#include <utils/Timers.h>
#include <sys/resource.h>
#include <limits.h>

#include <utils/threads.h>

#include <media/AudioTrack.h>

using namespace android;

// depth of buffer/command queues in MIO
static const int kCommandQueueDepth = 10;

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

// Audio Media IO component base class implementation
OSCL_EXPORT_REF AndroidAudioMIO::AndroidAudioMIO(const char* name)
    : OsclTimerObject(OsclActiveObject::EPriorityNominal, name),
    iWriteCompleteAO(NULL)
{
    initData();
}

OSCL_EXPORT_REF AndroidAudioMIO::~AndroidAudioMIO()
{
    LOGV("destructor");
    Cleanup();
}

void AndroidAudioMIO::initData()
{
    LOGV("initData in");
    ResetData();

    iCommandCounter = 0;
    iLogger = NULL;
    iCommandResponseQueue.reserve(kCommandQueueDepth);
    iWriteResponseQueueLock.Create();
    iWriteResponseQueue.reserve(kCommandQueueDepth);
    iObserver = NULL;
    iLogger = NULL;
    iPeer = NULL;
    iState = STATE_MIO_IDLE;
    iWriteBusy = false;
    iFlushPending = false;
    iDataQueued = 0;
    LOGV("initData out");
}

void AndroidAudioMIO::ResetData()
//reset all data from this session.
{
    LOGV("ResetData in");
    // reset all the received media parameters.
    iAudioFormat = PVMF_MIME_FORMAT_UNKNOWN;
    iAudioNumChannelsValid = false;
    iAudioSamplingRateValid = false;
    iAudioThreadCreatedAndMIOConfigured = false;
    LOGV("ResetData out");
}

//cleanup all allocated memory and release resources.
void AndroidAudioMIO::Cleanup()
{
    LOGV("Cleanup in");
    while (!iCommandResponseQueue.empty()) {
        if (iObserver) {
            iObserver->RequestCompleted(PVMFCmdResp(iCommandResponseQueue[0].iCmdId,
                        iCommandResponseQueue[0].iContext, iCommandResponseQueue[0].iStatus));
        }
        iCommandResponseQueue.erase(&iCommandResponseQueue[0]);
    }

    // return empty buffers
    returnAllBuffers();

    // delete the request active object
    if (iWriteCompleteAO) {
        OSCL_DELETE(iWriteCompleteAO);
        iWriteCompleteAO = NULL;
    }
    iWriteResponseQueueLock.Close();
    LOGV("Cleanup out");
}

PVMFStatus AndroidAudioMIO::connect(PvmiMIOSession& aSession, PvmiMIOObserver* aObserver)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::connect() called"));

    // currently supports only one session
    if (iObserver) return PVMFFailure;
    iObserver = aObserver;
    return PVMFSuccess;
}

PVMFStatus AndroidAudioMIO::disconnect(PvmiMIOSession aSession)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::disconnect() called"));
    iObserver = NULL;
    return PVMFSuccess;
}


PvmiMediaTransfer* AndroidAudioMIO::createMediaTransfer(PvmiMIOSession& aSession,
                            PvmiKvp* read_formats, int32 read_flags,
                            PvmiKvp* write_formats, int32 write_flags)
{
    // create the request active object
    // such when audio output thread is done with a buffer
    // it can put the buffer on the write response queue
    // and schedule this MIO to run, to return the buffer
    // to the engine
    iWriteCompleteAO = OSCL_NEW(AndroidAudioOutputThreadSafeCallbackAO,(this, 5));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::createMediaTransfer() called"));
    return (PvmiMediaTransfer*)this;
}

PVMFCommandId AndroidAudioMIO::QueueCmdResponse(PVMFStatus status, const OsclAny* aContext)
{
    PVMFCommandId cmdId = iCommandCounter++;
    CommandResponse resp(status, cmdId, aContext);
    iCommandResponseQueue.push_back(resp);

    // cancel any timer delay so the command response will happen ASAP.
    if (IsBusy()) Cancel();
    RunIfNotReady();
    return cmdId;
}

// return any held buffers to the engine
void AndroidAudioMIO::ProcessWriteResponseQueue()
{
    //LOGV("ProcessWriteResponseQueue in [Response Q size %d]",iWriteResponseQueue.size());
    PVMFStatus status = 0;
    PVMFCommandId cmdId = 0;
    const OsclAny* context = 0;

    iWriteResponseQueueLock.Lock();
    while (!iWriteResponseQueue.empty()) {
        status = iWriteResponseQueue[0].iStatus;
        cmdId = iWriteResponseQueue[0].iCmdId;
        context = (OsclAny*)iWriteResponseQueue[0].iContext;
        iWriteResponseQueue.erase(&iWriteResponseQueue[0]);
        iWriteResponseQueueLock.Unlock();
        if (iPeer) {
            LOGV("Return buffer(%d) status(%d) context(%p)", cmdId,status,context);
            iPeer->writeComplete(status, cmdId, (OsclAny*)context);
        }
        iWriteResponseQueueLock.Lock();
    }
    iWriteResponseQueueLock.Unlock();
    //LOGV("ProcessWriteResponseQueue out");
}

PVMFCommandId AndroidAudioMIO::QueryUUID(const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                        bool aExactUuidsOnly, const OsclAny* aContext)
{
    LOGV("QueryUUID");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::QueryUUID() called"));
    return QueueCmdResponse(PVMFFailure, aContext);
}

PVMFCommandId AndroidAudioMIO::QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    LOGV("QueryInterface");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::QueryInterface() called"));
    PVMFStatus status=PVMFFailure;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID) {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
        status = PVMFSuccess;
    }
    return QueueCmdResponse(status, aContext);
}

PVMFCommandId AndroidAudioMIO::Init(const OsclAny* aContext)
{
    LOGV("Init (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Init() called"));
    iState=STATE_MIO_INITIALIZED;
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::Reset(const OsclAny* aContext)
{
    LOGV("Reset (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Reset() called"));
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::Start(const OsclAny* aContext)
{
    // Start is NO-OP 
    LOGV("Start (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Start() called"));
    iState = STATE_MIO_STARTED;
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::Pause(const OsclAny* aContext)
{
    LOGV("Pause (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Pause() called"));
    iState = STATE_MIO_PAUSED;
    return QueueCmdResponse(PVMFSuccess, aContext);
}


PVMFCommandId AndroidAudioMIO::Flush(const OsclAny* aContext)
{
    LOGV("Flush (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Flush() called"));
    iState = STATE_MIO_INITIALIZED;
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::DiscardData(const OsclAny* aContext)
{
    LOGV("DiscardData (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::DiscardData() called"));
    return DiscardData(UINT_MAX, aContext);
}

PVMFCommandId AndroidAudioMIO::DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext)
{
    LOGV("DiscardData (%u, %p)", aTimestamp, aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::DiscardData() called"));
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::Stop(const OsclAny* aContext)
{
    LOGV("Stop (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::Stop() called"));
    iState = STATE_MIO_INITIALIZED;
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::CancelAllCommands(const OsclAny* aContext)
{
    LOGV("CancelAllCommands (%p)", aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::CancelAllCommands() called"));

    //commands are executed immediately upon being received, so
    //it isn't really possible to cancel them.
    return QueueCmdResponse(PVMFSuccess, aContext);
}

PVMFCommandId AndroidAudioMIO::CancelCommand(PVMFCommandId aCmdId, const OsclAny* aContext)
{
    LOGV("CancelCommand (%u, %p)", aCmdId, aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::CancelCommand() called"));

    //see if the response is still queued.
    PVMFStatus status = PVMFFailure;
    for (uint32 i = 0; i < iCommandResponseQueue.size(); i++) {
        if (iCommandResponseQueue[i].iCmdId == aCmdId) {
            status = PVMFSuccess;
            break;
        }
    }
    return QueueCmdResponse(PVMFSuccess, aContext);
}

void AndroidAudioMIO::ThreadLogon()
{
    LOGV("ThreadLogon() called ");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::ThreadLogon() called"));
    if (iState == STATE_MIO_IDLE) {
        iLogger = PVLogger::GetLoggerObject("AndroidAudioMIO\n");
        AddToScheduler();
        iState=STATE_MIO_LOGGED_ON;
    }
}

void AndroidAudioMIO::ThreadLogoff()
{
    LOGV("ThreadLogoff() called");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::ThreadLogoff() called"));
    if (iState!=STATE_MIO_IDLE) {
        RemoveFromScheduler();
        iLogger = NULL;
        iState = STATE_MIO_IDLE;
        ResetData();
    }
}

void AndroidAudioMIO::setPeer(PvmiMediaTransfer* aPeer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"AndroidAudioMIO::setPeer() called"));
    // Set the observer
    iPeer = aPeer;
}

//This routine will determine whether data can be accepted in a writeAsync
//call and if not, will return true;
bool AndroidAudioMIO::CheckWriteBusy(uint32 aSeqNum)
{
    // FIXME: this line screws up video output - why?
    // return (iOSSRequestQueue.size() >= 5);
    return false;
}

PVMFCommandId AndroidAudioMIO::writeAsync(uint8 aFormatType, int32 aFormatIndex, uint8* aData, uint32 aDataLen,
                                        const PvmiMediaXferHeader& data_header_info, OsclAny* aContext)
{
    // Do a leave if MIO is not configured except when it is an EOS
    if (!iAudioThreadCreatedAndMIOConfigured
            &&
            !((PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION == aFormatType)
              && (PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM == aFormatIndex)))
    {
        LOGE("ERROR :: data is pumped in before mio configured");
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    uint32 aSeqNum = data_header_info.seq_num;
    PVMFTimestamp aTimestamp = data_header_info.timestamp;
    uint32 flags = data_header_info.flags;

    bool bWriteComplete = true;

    //LOGV("writeAsync() called seqnum %d ts %d flags %d context %d formattype %d formatindex %d",aSeqNum, aTimestamp, flags,aContext,aFormatType,aFormatIndex);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
        (0,"AndroidAudioMIO::writeAsync() seqnum %d ts %d flags %d context %d",
         aSeqNum, aTimestamp, flags,aContext));

    PVMFStatus status=PVMFFailure;

    switch(aFormatType) {
    case PVMI_MEDIAXFER_FMT_TYPE_COMMAND :
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
            (0,"AndroidAudioMIO::writeAsync() called with Command info."));
        //ignore
        LOGV("PVMI_MEDIAXFER_FMT_TYPE_COMMAND");
        status = PVMFSuccess;
        break;

    case PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION :
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
            (0,"AndroidAudioMIO::writeAsync() called with Notification info."));
        switch(aFormatIndex) {
        case PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM:
            LOGV("PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM");
            bWriteComplete = false; //force an empty buffer through the audio thread
            break;
        default:
            break;
        }
        //ignore
        status = PVMFSuccess;
        break;

    case PVMI_MEDIAXFER_FMT_TYPE_DATA :
        switch(aFormatIndex) {
        case PVMI_MEDIAXFER_FMT_INDEX_FMT_SPECIFIC_INFO:
            status = PVMFSuccess;
            LOGV("PVMI_MEDIAXFER_FMT_INDEX_FMT_SPECIFIC_INFO");
            break;

        case PVMI_MEDIAXFER_FMT_INDEX_DATA:
            LOGV("PVMI_MEDIAXFER_FMT_INDEX_DATA");
            //data contains the media bitstream.

            //Check whether we can accept data now and leave if we can't.
            if (CheckWriteBusy(aSeqNum)) {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                    (0,"AndroidAudioMIO::writeAsync: Entering busy state"));

                //schedule an event to re-start the data flow.
                iWriteBusy = true;
                bWriteComplete = false;

                // Rich:: commenting in this line.
                //        Need some timeout here just in case.
                //        I have no evidence of any problems.
                RunIfNotReady(10 * 1000);
                OSCL_LEAVE(OsclErrBusy);
            } else {
                if (aDataLen > 0) {
                    // this buffer will be queued by the audio output thread to process
                    // this buffer cannot be write completed until
                    // it has been processed by the audio output thread
                    bWriteComplete = false;
                } else { 
                       LOGE("writeAsync() called with aDataLen==0");
                       PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0,"AndroidAudioMIO::writeAsync() called aDataLen==0."));
                }
                status = PVMFSuccess;
            }
            break;

        default:
            LOGE("Error unrecognized format index =%u", aFormatIndex);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                (0,"AndroidAudioMIO::writeAsync: Error - unrecognized format index"));
            status = PVMFFailure;
            break;
        }
        break;

    default:
        LOGE("Error unrecognized format type =%u", aFormatType);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
            (0,"AndroidAudioMIO::writeAsync: Error - unrecognized format type"));
        status = PVMFFailure;
        break;
    }

    //Schedule asynchronous response
    PVMFCommandId cmdid=iCommandCounter++;
    if (bWriteComplete) {
        LOGV("write complete (%d)", cmdid);
        WriteResponse resp(status, cmdid, aContext, aTimestamp);
        iWriteResponseQueueLock.Lock();
        iWriteResponseQueue.push_back(resp);
        iWriteResponseQueueLock.Unlock();
        RunIfNotReady();
    } else if (!iWriteBusy) {
        writeAudioBuffer(aData, aDataLen, cmdid, aContext, aTimestamp);
    }
    LOGV("data queued = %u", iDataQueued);

    return cmdid;
}

PVMFStatus AndroidAudioMIO::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                              PvmiKvp*& aParameters, int& num_parameter_elements,
                                              PvmiCapabilityContext aContext)
{
    LOGV("getParametersSync in %s",aIdentifier);
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    aParameters=NULL;
    num_parameter_elements=0;
    PVMFStatus status = PVMFFailure;

    if(pv_mime_strcmp(aIdentifier, INPUT_FORMATS_CAP_QUERY) == 0) {
        //This is a query for the list of supported formats.
        aParameters=(PvmiKvp*)oscl_malloc(2 * sizeof(PvmiKvp));
        if (aParameters) {
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM16;
            aParameters[num_parameter_elements++].value.pChar_value = (char*)PVMF_MIME_PCM8;
            status = PVMFSuccess;
        }
        else{
            status = PVMFErrNoMemory;
        }
    }
    //other queries are not currently supported.

    //unrecognized key.
    LOGV("getParametersSync out status %d",status);
    return status;
}

PVMFStatus AndroidAudioMIO::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    LOGV("releaseParameters in");
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);
    PVMFStatus status = PVMFFailure;
    //release parameters that were allocated by this component.
    if (aParameters) {
        oscl_free(aParameters);
        status = PVMFSuccess;
    }
    LOGV("releaseParameters out status %d",status);
    return status;
}

PVMFStatus AndroidAudioMIO::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    //LOGV("verifyParametersSync in");

    // Go through each parameter
    for (int32 i=0; i<num_elements; i++) {
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[i].key, compstr);
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/media/format-type")) == 0) {
            if ((pv_mime_strcmp(aParameters[i].value.pChar_value, PVMF_MIME_PCM8) == 0) ||
                (pv_mime_strcmp(aParameters[i].value.pChar_value, PVMF_MIME_PCM16) == 0)
               ) {
                return PVMFSuccess;
            }
            else {
                return PVMFErrNotSupported;
            }
        }
    }
    return PVMFSuccess;
    //LOGV("verifyParametersSync out");

}
void AndroidAudioMIO::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                        int num_elements, PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    //LOGV("setParametersSync in");
    aRet_kvp = NULL;

    for (int32 i=0;i<num_elements;i++) {
        //Check against known audio parameter keys...
        if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_FORMAT_KEY) == 0) {
            LOGV("Audio format: %s", aParameters[i].value.pChar_value);
            iAudioFormat=aParameters[i].value.pChar_value;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                (0,"AndroidAudioOutput::setParametersSync() Audio Format Key, Value %s",aParameters[i].value.pChar_value));
        } else if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_SAMPLING_RATE_KEY) == 0) {
            iAudioSamplingRate=(int32)aParameters[i].value.uint32_value;
            iAudioSamplingRateValid=true;
            // LOGD("iAudioSamplingRate=%d", iAudioSamplingRate);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                (0,"AndroidAudioMIO::setParametersSync() Audio Sampling Rate Key, Value %d",iAudioSamplingRate));
        } else if (pv_mime_strcmp(aParameters[i].key, MOUT_AUDIO_NUM_CHANNELS_KEY) == 0) {
            iAudioNumChannels=(int32)aParameters[i].value.uint32_value;
            iAudioNumChannelsValid=true;
            // LOGD("iAudioNumChannels=%d", iAudioNumChannels);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                (0,"AndroidAudioMIO::setParametersSync() Audio Num Channels Key, Value %d",iAudioNumChannels));
        } else {
            //if we get here the key is unrecognized.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                (0,"AndroidAudioMIO::setParametersSync() Error, unrecognized key %s",aParameters[i].key));

            //set the return value to indicate the unrecognized key
            //and return.
            aRet_kvp = &aParameters[i];
            return;
        }
    }

    //LOGV("setParametersSync out");
}

//
// Private section
//

void AndroidAudioMIO::Run()
{
    while (!iCommandResponseQueue.empty()) {
        if (iObserver) {
            iObserver->RequestCompleted(PVMFCmdResp(iCommandResponseQueue[0].iCmdId,
                        iCommandResponseQueue[0].iContext,
                        iCommandResponseQueue[0].iStatus));
        }
        iCommandResponseQueue.erase(&iCommandResponseQueue[0]);
    }

    //send async write completion
    ProcessWriteResponseQueue();

    //Re-start the data transfer if needed.
    if (iWriteBusy) {
        iWriteBusy = false;
        iPeer->statusUpdate(PVMI_MEDIAXFER_STATUS_WRITE);
    }
}

// send response to MIO
void AndroidAudioMIO::sendResponse(PVMFCommandId cmdid, const OsclAny* context, PVMFTimestamp timestamp)
{
    LOGV("sendResponse :: return buffer (%d) timestamp(%d) context(%p)", cmdid, timestamp, context);
    WriteResponse resp(PVMFSuccess, cmdid, context, timestamp);
    iWriteResponseQueueLock.Lock();
    if (iWriteResponseQueue.size() < iWriteResponseQueue.capacity()) {
        iWriteResponseQueue.push_back(resp);
    } else {
        LOGE("Exceeded response queue capacity");
    }
    iWriteResponseQueueLock.Unlock();

    // Create an event for the threadsafe callback AO
    OsclAny* P = NULL;
    iWriteCompleteAO->ReceiveEvent(P);
}

void AndroidAudioMIO::setAudioSink(const sp<MediaPlayerInterface::AudioSink>& audioSink)
{
    LOGV("setAudioSink in");
    mAudioSink = audioSink;
}

//------------------------------------------------------------------------
// Active timing support
//
OSCL_EXPORT_REF PVMFStatus AndroidAudioMIOActiveTimingSupport::SetClock(PVMFMediaClock *clockVal)
{
    LOGV("ATS :: SetClock in");
    iClock=clockVal;
    
    return PVMFSuccess;
}

void AndroidAudioMIOActiveTimingSupport::NotificationsInterfaceDestroyed()
{
    LOGV("ATS :: NotificationsInterfaceDestroyed in");
    iClockNotificationsInf=NULL;
}


OSCL_EXPORT_REF bool AndroidAudioMIOActiveTimingSupport::queryInterface(const PVUuid& aUuid, PVInterface*& aInterface)
{
    aInterface = NULL;
    PVUuid uuid;
    queryUuid(uuid);
    bool status = false;
    if (uuid == aUuid) {
        PvmiClockExtensionInterface* myInterface = OSCL_STATIC_CAST(PvmiClockExtensionInterface*, this);
        aInterface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        status = true;
    }
    LOGV("ATS :: queryInterface out status %d",status);
    return status;
}

void AndroidAudioMIOActiveTimingSupport::queryUuid(PVUuid& uuid)
{
    LOGV("ATS :: queryUuid in");
    uuid = PvmiClockExtensionInterfaceUuid;
}

void AndroidAudioMIOActiveTimingSupport::ClockStateUpdated()
{
    LOGV("ATS :: ClockStateUpdated in");
    if (iClock) {
       
        PVMFMediaClock::PVMFMediaClockState newClockState = iClock->GetState();
        if (newClockState != iClockState) {
            iClockState = newClockState;
            switch (iClockState) {
            case PVMFMediaClock::STOPPED:
                LOGV("A/V clock stopped");
                break;
            case PVMFMediaClock::RUNNING:
                LOGV("A/V clock running");
                // must be seeking, get new clock offset for A/V sync
                if (iUpdateClock) {
                    bool overflowFlag = false;
                    uint32 currentTimeBase32 = 0;
                    iClock->GetCurrentTime32(iStartTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, currentTimeBase32);
                    iFrameCount = 0;
                    iUpdateClock = false;
                    LOGV("update iStartTime: %d", iStartTime);
                }
                LOGV("signal thread to start");
                if (iAudioThreadSem) iAudioThreadSem->Signal();
                break;
            case PVMFMediaClock::PAUSED:
                LOGV("A/V clock paused");
                break;
            default:
                LOGE("Wrong clock state!");
                break;
            }
        }
    }
    LOGV("ATS :: ClockStateUpdated out");
}

void AndroidAudioMIOActiveTimingSupport::UpdateClock()
{
    LOGV("ATS :: UpdateClock in");
    if (iClock && (iClockState == PVMFMediaClock::RUNNING)) {
        uint32 clockTime32, timeBaseTime32, updateClock32;
        int32 correction = 0;
        bool overflowFlag = false;
        // get current time
        iClock->GetCurrentTime32(clockTime32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timeBaseTime32);
        LOGV("PV clock current = %u", clockTime32);

        // calculate sample clock
        updateClock32 = iFrameCount * iMsecsPerFrame;
        LOGV("sample clock = %u frameCount(%u) msecsPerFrame(%f)", updateClock32,iFrameCount,iMsecsPerFrame);

        // startup - force clock backwards to compensate for latency
        if (updateClock32 < iDriverLatency) {
            LOGV("iStartTime = %u , iDriverLatency = %u",iStartTime, iDriverLatency);
            correction = iStartTime - clockTime32;
            LOGV("latency stall - forcing clock (correction = %d)",correction);
        }

        // normal play mode - check delta between PV engine clock and sample clock
        else {
            correction = (updateClock32 - iDriverLatency) - (clockTime32 - iStartTime);
            LOGV("clock drift (correction = (updateClock32(%d)-iDriverLatency(%d))-(clockTime32(%d)-iStartTime(%d))= %d)",updateClock32,iDriverLatency,clockTime32,iStartTime,correction);
        }

        // do clock correction if drift exceeds threshold
        if (OSCL_ABS(correction) > iMinCorrection) {
            if (correction > iMaxCorrection) {
                correction = iMaxCorrection;
            } else if (correction < -iMaxCorrection) {
                correction = -iMaxCorrection;
            }
            updateClock32 = clockTime32 + correction;
            LOGV("drift correction = %d, new clock = %u", correction, updateClock32);
            PVMFMediaClockAdjustTimeStatus adjustmentstatus = 
            iClock->AdjustClockTime32(clockTime32, timeBaseTime32, updateClock32,PVMF_MEDIA_CLOCK_MSEC,overflowFlag);

            if ((PVMF_MEDIA_CLOCK_ADJUST_SUCCESS != adjustmentstatus))
            {
                LOGE("Error adjusting clock status = %d",adjustmentstatus);
            }
            if(overflowFlag) {
            LOGE("Adjusting clock caused overflow");
            }
        }
        else{
            LOGV("ATS :: sample clock and pv clock are close enough no need to update");
        }
    }
    LOGV("ATS :: UpdateClock out");
}
void AndroidAudioMIOActiveTimingSupport::setDriverLatency(uint32 latency)
{
    LOGV("ATS :: setDriverLatency in");
    iDriverLatency = latency;
    if (iClock){
        LOGV("register latency to media clock and set clockobserver");
        PVMFStatus ret = iClock->ConstructMediaClockNotificationsInterface(iClockNotificationsInf,*this,latency);
        if(iClockNotificationsInf && (PVMFSuccess == ret))
        {
            iClockNotificationsInf->SetClockStateObserver(*this);
        }
        else
        {
            LOGE("latency could NOT be set !! set it later ");
        }
    }
    LOGV("ATS :: setDriverLatency out");
}

