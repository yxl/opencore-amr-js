/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2008 HTC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CameraInput"
#include <utils/Log.h>
#include <ui/CameraParameters.h>
#include <utils/Errors.h>
#include <media/mediarecorder.h>
#include <ui/ISurface.h>
#include <ui/ICamera.h>
#include <ui/Camera.h>

#include "pv_mime_string_utils.h"
#include "oscl_dll.h"
#include "oscl_tickcount.h"

#include "android_camera_input.h"

using namespace android;


// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

AndroidCameraInput::AndroidCameraInput()
    : OsclTimerObject(OsclActiveObject::EPriorityNominal, "AndroidCameraInput")
{
    LOGV("constructor");
    iCmdIdCounter = 0;
    iPeer = NULL;
    iThreadLoggedOn = false;
    iDataEventCounter = 0;
    iStartTickCount = 0;
    iTimeStamp = 0;
    iMilliSecondsPerDataEvent = 0;
    iMicroSecondsPerDataEvent = 0;
    iState = STATE_IDLE;
    mFrameWidth = DEFAULT_FRAME_WIDTH;
    mFrameHeight= DEFAULT_FRAME_HEIGHT;
    mFrameRate  = DEFAULT_FRAME_RATE;
    mCamera = NULL;
    mHeap = 0;

    // FIXME:
    // mFrameRefCount is redundant. iSendMediaData.empty() can be used to
    // determine if there are any frames pending in the encoder.
    mFrameRefCount = 0;
    mFlags = 0;
    iFrameQueue.reserve(5);
    iFrameQueueMutex.Create();
}

void AndroidCameraInput::ReleaseQueuedFrames()
{
    LOGV("ReleaseQueuedFrames");
    iFrameQueueMutex.Lock();
    while (!iFrameQueue.empty()) {
        AndroidCameraInputMediaData data = iFrameQueue[0];
        iFrameQueue.erase(iFrameQueue.begin());
#if (LOG_NDEBUG == 0)
        ssize_t offset = 0;
        size_t size = 0;
        sp<IMemoryHeap> heap = data.iFrameBuffer->getMemory(&offset, &size);
        LOGD("writeComplete: ID = %d, base = %p, offset = %p, size = %d ReleaseQueuedFrames", heap->getHeapID(), heap->base(), offset, size);
#endif
        mCamera->releaseRecordingFrame(data.iFrameBuffer);
    }
    iFrameQueueMutex.Unlock();
}

AndroidCameraInput::~AndroidCameraInput()
{
    LOGV("destructor");
    if (mCamera != NULL) {
        mCamera->setRecordingCallback(NULL, this);
        ReleaseQueuedFrames();
        if ((mFlags & FLAGS_HOT_CAMERA) == 0) {
            LOGV("camera was cold when we started, stopping preview");
            mCamera->stopPreview();
        }
        if (mFlags & FLAGS_SET_CAMERA) {
            LOGV("unlocking camera to return to app");
            mCamera->unlock();
        } else {
            LOGV("disconnect from camera");
            mCamera->disconnect();
        }
        mFlags = 0;
        mCamera.clear();
    }
    if (mFrameRefCount != 0) {
        LOGW("mHeap reference count is not zero?!");
    }
    iFrameQueueMutex.Close();
}

OSCL_EXPORT_REF
PVMFStatus AndroidCameraInput::connect(PvmiMIOSession& aSession,
        PvmiMIOObserver* aObserver)
{
    LOGV("connect");
    if (!aObserver) {
        LOGE("observer is a NULL pointer");
        return PVMFFailure;
    }

    int32 err = 0;
    OSCL_TRY(err, iObservers.push_back(aObserver));
    OSCL_FIRST_CATCH_ANY(err,
        LOGE("Out of memory"); return PVMFErrNoMemory);

    // Session ID is the index of observer in the vector
    aSession = (PvmiMIOSession)(iObservers.size() - 1);
    return PVMFSuccess;
}

OSCL_EXPORT_REF
PVMFStatus AndroidCameraInput::disconnect(PvmiMIOSession aSession)
{
    LOGV("disconnect");
    uint32 index = (uint32) aSession;
    uint32 size  = iObservers.size();
    if (index >= size) {
        LOGE("Invalid session ID %d. Valid range is [0, %d]", index, size - 1);
        return PVMFFailure;
    }

    iObservers.clear();
    return PVMFSuccess;
}

OSCL_EXPORT_REF
PvmiMediaTransfer* AndroidCameraInput::createMediaTransfer(
        PvmiMIOSession& aSession,
        PvmiKvp* read_formats,
        int32 read_flags,
        PvmiKvp* write_formats,
        int32 write_flags)
{
    LOGV("createMediaTransfer");
    OSCL_UNUSED_ARG(read_formats);
    OSCL_UNUSED_ARG(read_flags);
    OSCL_UNUSED_ARG(write_formats);
    OSCL_UNUSED_ARG(write_flags);

    uint32 index = (uint32) aSession;
    uint32 size  = iObservers.size();
    if (index >= size) {
        LOGE("Invalid session ID %d. Valid range is [0, %d]", index, size - 1);
        OSCL_LEAVE(OsclErrArgument);
        return NULL;
    }

    return (PvmiMediaTransfer*)this;
}

OSCL_EXPORT_REF
void AndroidCameraInput::deleteMediaTransfer(PvmiMIOSession& aSession,
        PvmiMediaTransfer* media_transfer)
{
    LOGV("deleteMediaTransfer");
    uint32 index = (uint32) aSession;
    uint32 size  = iObservers.size();
    if (index >= size) {
        LOGE("Invalid session ID %d. Valid range is [0, %d]", index, size - 1);
        OSCL_LEAVE(OsclErrArgument);
        return;
    }
    if (!media_transfer) {
        LOGE("media transfer is a NULL pointer");
        OSCL_LEAVE(OsclErrArgument);
    }

    // TODO:
    // 1. I did not see how the media transfer session has been terminated
    //    after this method call.
    // 2. according to pvmi_mio_control.h, this should also check with there
    //    is any outstanding buffer?
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::QueryUUID(const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid,
        OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    LOGV("QueryUUID");
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aExactUuidsOnly);

    int32 err = 0;
    OSCL_TRY(err, aUuids.push_back(PVMI_CAPABILITY_AND_CONFIG_PVUUID););
    OSCL_FIRST_CATCH_ANY(err,
        LOGE("Out of memory"); OSCL_LEAVE(OsclErrNoMemory));

    return AddCmdToQueue(CMD_QUERY_UUID, aContext);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::QueryInterface(const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    LOGV("QueryInterface");
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID) {
        PvmiCapabilityAndConfig*
           myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*,this);

        aInterfacePtr = OSCL_STATIC_CAST(PVInterface*, myInterface);
    } else {
        aInterfacePtr = NULL;
    }

    return AddCmdToQueue(CMD_QUERY_INTERFACE,
                         aContext,
                         (OsclAny*)&aInterfacePtr);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Init(const OsclAny* aContext)
{
    LOGV("Init");
    if (iState != STATE_IDLE) {
        LOGE("Init called in an invalid state(%d)", iState);
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_INIT, aContext);
}


OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Start(const OsclAny* aContext)
{
    LOGV("Start");
    if (iState != STATE_INITIALIZED && iState != STATE_PAUSED) {
        LOGE("Start called in an invalid state(%d)", iState);
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_START, aContext);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Pause(const OsclAny* aContext)
{
    LOGV("Pause");
    if (iState != STATE_STARTED) {
        LOGE("Pause called in an invalid state(%d)", iState);
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_PAUSE, aContext);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Flush(const OsclAny* aContext)
{
    LOGV("Flush");
    if (iState != STATE_STARTED && iState != STATE_PAUSED) {
        LOGE("Flush called in an invalid state(%d)", iState);
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_FLUSH, aContext);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Reset(const OsclAny* aContext)
{
    LOGV("Reset");
    return AddCmdToQueue(CMD_RESET, aContext);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::DiscardData(PVMFTimestamp aTimestamp,
        const OsclAny* aContext)
{
    LOGV("DiscardData with time stamp");
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aTimestamp);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::DiscardData(const OsclAny* aContext)
{
    LOGV("DiscardData");
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::Stop(const OsclAny* aContext)
{
    LOGV("Stop");
    if (iState != STATE_STARTED && iState != STATE_PAUSED) {
        LOGE("Stop called in an invalid state(%d)", iState);
        OSCL_LEAVE(OsclErrInvalidState);
        return -1;
    }

    return AddCmdToQueue(CMD_STOP, aContext);
}

OSCL_EXPORT_REF
void AndroidCameraInput::ThreadLogon()
{
    LOGV("ThreadLogon");
    if (!iThreadLoggedOn) {
        AddToScheduler();
        iThreadLoggedOn = true;
    }
}

OSCL_EXPORT_REF
void AndroidCameraInput::ThreadLogoff()
{
    LOGV("ThreadLogoff");
    if (iThreadLoggedOn) {
        RemoveFromScheduler();
        iThreadLoggedOn = false;
    }
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::CancelAllCommands(const OsclAny* aContext)
{
    LOGV("CancelAllCommands");
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::CancelCommand(PVMFCommandId aCmdId,
        const OsclAny* aContext)
{
    LOGV("CancelCommand");
    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
void AndroidCameraInput::setPeer(PvmiMediaTransfer* aPeer)
{
    LOGV("setPeer");
    if (iPeer || !aPeer) {
        if (iPeer) {
            LOGE("iPeer already exists");
        } else {
	    LOGE("aPeer is a NULL pointer");
        }
        OSCL_LEAVE(OsclErrGeneral);
        return;
    }

    iPeer = aPeer;
}

OSCL_EXPORT_REF
void AndroidCameraInput::useMemoryAllocators(OsclMemAllocator* write_alloc)
{
    LOGV("useMemoryAllocators");
    OSCL_UNUSED_ARG(write_alloc);
    OSCL_LEAVE(OsclErrNotSupported);
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::writeAsync(uint8 aFormatType,
        int32 aFormatIndex,
        uint8* aData,
        uint32 aDataLen,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext)
{
    LOGV("writeAsync");
    OSCL_UNUSED_ARG(aFormatType);
    OSCL_UNUSED_ARG(aFormatIndex);
    OSCL_UNUSED_ARG(aData);
    OSCL_UNUSED_ARG(aDataLen);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(aContext);
    // This is an active data source. writeAsync is not supported.
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
void AndroidCameraInput::writeComplete(PVMFStatus aStatus,
       PVMFCommandId write_cmd_id,
       OsclAny* aContext)
{
    LOGV("writeComplete");
    OSCL_UNUSED_ARG(aContext);

    iFrameQueueMutex.Lock();
    if (iSentMediaData.empty()) {
        LOGE("Nothing to complete");
        iFrameQueueMutex.Unlock();
        return;
    }

    AndroidCameraInputMediaData data = iSentMediaData[0];
#if (LOG_NDEBUG == 0)
    ssize_t offset = 0;
    size_t size = 0;
    sp<IMemoryHeap> heap = data.iFrameBuffer->getMemory(&offset, &size);
    LOGD("writeComplete: ID = %d, base = %p, offset = %p, size = %d", heap->getHeapID(), heap->base(), offset, size);
#endif
    mCamera->releaseRecordingFrame(data.iFrameBuffer);

    if (mFrameRefCount) {
        --mFrameRefCount;
    }
    //LOGV("@@@@@@@@@@@@@ decrementing frame reference count: %d @@@@@@@@@@@@", mFrameRefCount);
    if (mFrameRefCount <= 0) {
        //LOGV("decrement the reference count for mHeap");
        mFrameRefCount = 0;
        mHeap.clear();
     }

    iSentMediaData.erase(iSentMediaData.begin());
    iFrameQueueMutex.Unlock();

    // reference count is always updated, even if the write fails
    if (aStatus != PVMFSuccess) {
        LOGE("writeAsync failed. aStatus=%d", aStatus);
    }
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::readAsync(uint8* data,
        uint32 max_data_len,
        OsclAny* aContext,
        int32* formats,
        uint16 num_formats)
{
    LOGV("readAsync");
    OSCL_UNUSED_ARG(data);
    OSCL_UNUSED_ARG(max_data_len);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(formats);
    OSCL_UNUSED_ARG(num_formats);
    // This is an active data source. readAsync is not supported.
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
void AndroidCameraInput::readComplete(PVMFStatus aStatus,
        PVMFCommandId read_cmd_id,
        int32 format_index,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext)
{
    LOGV("readComplete");
    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(read_cmd_id);
    OSCL_UNUSED_ARG(format_index);
    OSCL_UNUSED_ARG(data_header_info);
    OSCL_UNUSED_ARG(aContext);
    // This is an active data source. readComplete is not supported.
    OSCL_LEAVE(OsclErrNotSupported);
    return;
}

OSCL_EXPORT_REF
void AndroidCameraInput::statusUpdate(uint32 status_flags)
{
    LOGV("statusUpdate");
    OSCL_UNUSED_ARG(status_flags);
    // Ideally this routine should update the status of media input component.
    // It should check then for the status. If media input buffer is consumed,
    // media input object should be resheduled.
    // Since the Media fileinput component is designed with single buffer, two
    // asynchronous reads are not possible. So this function will not be
    // requiredand hence not been implemented.
    OSCL_LEAVE(OsclErrNotSupported);
}

OSCL_EXPORT_REF
void AndroidCameraInput::cancelCommand(PVMFCommandId aCmdId)
{
    LOGV("cancelCommand");
    OSCL_UNUSED_ARG(aCmdId);

    // This cancel command ( with a small "c" in cancel ) is for the media
    // transfer interface. implementation is similar to the cancel command
    // of the media I/O interface.
    OSCL_LEAVE(OsclErrNotSupported);
}

OSCL_EXPORT_REF
void AndroidCameraInput::cancelAllCommands()
{
    LOGV("cancelAllCommands");
    OSCL_LEAVE(OsclErrNotSupported);
}

OSCL_EXPORT_REF
void AndroidCameraInput::setObserver(
        PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    LOGV("setObserver");
    OSCL_UNUSED_ARG(aObserver);
}

OSCL_EXPORT_REF
PVMFStatus AndroidCameraInput::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& params,
        int& num_params,
        PvmiCapabilityContext context)
{
    LOGV("getParametersSync");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    params = NULL;
    num_params = 0;
    PVMFStatus status = PVMFFailure;

    if (!pv_mime_strcmp(identifier, OUTPUT_FORMATS_CAP_QUERY) ||
        !pv_mime_strcmp(identifier, OUTPUT_FORMATS_CUR_QUERY)) {
        num_params = 1;
        status = AllocateKvp(params, OUTPUT_FORMATS_VALTYPE, num_params);
        if (status != PVMFSuccess) {
            LOGE("AllocateKvp failed for OUTPUT_FORMATS_VALTYP");
            return status;
        }
        params[0].value.pChar_value = (char*)PVMF_MIME_YUV420;
    } else if (!pv_mime_strcmp(identifier, VIDEO_OUTPUT_WIDTH_CUR_QUERY)) {
        num_params = 1;
        status = AllocateKvp(params, VIDEO_OUTPUT_WIDTH_CUR_VALUE, num_params);
        if (status != PVMFSuccess) {
            LOGE("AllocateKvp failed for VIDEO_OUTPUT_WIDTH_CUR_VALUE");
            return status;
        }
        params[0].value.uint32_value = mFrameWidth;
    } else if (!pv_mime_strcmp(identifier, VIDEO_OUTPUT_HEIGHT_CUR_QUERY)) {
        num_params = 1;
        status = AllocateKvp(params, VIDEO_OUTPUT_HEIGHT_CUR_VALUE, num_params);
        if (status != PVMFSuccess) {
            LOGE("AllocateKvp failed for VIDEO_OUTPUT_HEIGHT_CUR_VALUE");
            return status;
        }
        params[0].value.uint32_value = mFrameHeight;
    } else if (!pv_mime_strcmp(identifier, VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY)) {
        num_params = 1;
        status = AllocateKvp(params,
            VIDEO_OUTPUT_FRAME_RATE_CUR_VALUE, num_params);
        if (status != PVMFSuccess) {
            LOGE("AllocateKvp failed for VIDEO_OUTPUT_FRAME_RATE_CUR_VALUE");
            return status;
        }
        params[0].value.float_value = mFrameRate;
    } else if (!pv_mime_strcmp(identifier, OUTPUT_TIMESCALE_CUR_QUERY)) {
        num_params = 1;
        status = AllocateKvp(params, OUTPUT_TIMESCALE_CUR_VALUE, num_params);
        if (status != PVMFSuccess) {
            LOGE("AllocateKvp failed for OUTPUT_TIMESCALE_CUR_VALUE");
            return status;
        }
        // TODO:
        // is it okay to hardcode this as the timescale?
        params[0].value.uint32_value = 1000;
    }

    return status;
}

OSCL_EXPORT_REF
PVMFStatus AndroidCameraInput::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    LOGV("releaseParameters");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);
    
    if (!parameters) {
        LOGE("parameters is a NULL pointer");
        return PVMFFailure;
    }
    iAlloc.deallocate((OsclAny*)parameters);
    return PVMFSuccess;
}

OSCL_EXPORT_REF
void AndroidCameraInput::createContext(PvmiMIOSession session,
        PvmiCapabilityContext& context)
{
    LOGV("createContext");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

OSCL_EXPORT_REF
void AndroidCameraInput::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters,
        int num_parameter_elements)
{
    LOGV("setContextParameters");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

OSCL_EXPORT_REF
void AndroidCameraInput::DeleteContext(PvmiMIOSession session,
        PvmiCapabilityContext& context)
{
    LOGV("DeleteContext");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

OSCL_EXPORT_REF
void AndroidCameraInput::setParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp)
{
    LOGV("setParametersSync");
    OSCL_UNUSED_ARG(session);
    PVMFStatus status = PVMFSuccess;
    ret_kvp = NULL;
    for (int32 i = 0; i < num_elements; ++i) {
        status = VerifyAndSetParameter(&(parameters[i]), true);
        if (status != PVMFSuccess) {
            LOGE("VerifiyAndSetParameter failed on parameter #%d", i);
            ret_kvp = &(parameters[i]);
            OSCL_LEAVE(OsclErrArgument);
            return;
        }
    }
}

OSCL_EXPORT_REF
PVMFCommandId AndroidCameraInput::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    LOGV("setParametersAsync");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF
uint32 AndroidCameraInput::getCapabilityMetric (PvmiMIOSession session)
{
    LOGV("getCapabilityMetric");
    OSCL_UNUSED_ARG(session);
    return 0;
}

OSCL_EXPORT_REF
PVMFStatus AndroidCameraInput::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    LOGV("verifyParametersSync");
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    return PVMFErrNotSupported;
}


void AndroidCameraInput::SetFrameSize(int w, int h)
{
    LOGV("SetFrameSize");
    if (iState != STATE_IDLE) {
        LOGE("SetFrameSize called in an invalid state(%d)", iState);
        return;
    }

    mFrameWidth = w;
    mFrameHeight = h;
    FrameSizeChanged();
}

void AndroidCameraInput::SetFrameRate(int fps)
{
    LOGV("SetFrameRate");
    if (iState != STATE_IDLE) {
        LOGE("SetFrameRate called in an invalid state(%d)", iState);
        return;
    }

    mFrameRate = (float)fps;
}

void AndroidCameraInput::FrameSizeChanged()
{
    LOGV("FrameSizeChanged");
    if (iState != STATE_IDLE) {
        LOGE("FrameSizeChanged called in an invalid state(%d)", iState);
        return;
    }

    // Reinitialize the preview surface in case it was set up before now
    if (mSurface != NULL) {
        SetPreviewSurface(mSurface);
    }
}

void AndroidCameraInput::Run()
{
    LOGV("Run");

    // dequeue frame buffers and write to peer
    iFrameQueueMutex.Lock();
    while (!iFrameQueue.empty()) {
        AndroidCameraInputMediaData data = iFrameQueue[0];

        uint32 writeAsyncID = 0;
        OsclLeaveCode error = OsclErrNone;
        if (NULL == iPeer)
            break;
        OSCL_TRY(error,writeAsyncID = iPeer->writeAsync(PVMI_MEDIAXFER_FMT_TYPE_DATA, 0, (uint8*) (data.iFrameBuffer->pointer()),
                    data.iFrameSize, data.iXferHeader););

        if (OsclErrNone == error) {
	    iFrameQueue.erase(iFrameQueue.begin());
            data.iId = writeAsyncID;
            iSentMediaData.push_back(data);
            ++mFrameRefCount;
        }
    else if ( OsclErrBusy == error) {
	    LOGE("Ln %d Run writeAsync BUSY mFrameRefCount %d", __LINE__, mFrameRefCount);
	    {//release buffer immediately if write fails
		mCamera->releaseRecordingFrame(data.iFrameBuffer);
		if (mFrameRefCount) {
		    --mFrameRefCount;
		}
		//LOGV("@@@@@@@@@@@@@ decrementing frame reference count: %d @@@@@@@@@@@@", mFrameRefCount);
		if (mFrameRefCount <= 0) {
		    //LOGV("decrement the reference count for mHeap");
		    mFrameRefCount = 0;
		    mHeap.clear();
		}
	    }
	    break;
	}
	else
	{
	    LOGE("Ln %d Run writeAsync error %d mFrameRefCount %d", __LINE__, error, mFrameRefCount);
	    {//release buffer immediately if write fails
		mCamera->releaseRecordingFrame(data.iFrameBuffer);
		if (mFrameRefCount) {
		    --mFrameRefCount;
		}
		//LOGV("@@@@@@@@@@@@@ decrementing frame reference count: %d @@@@@@@@@@@@", mFrameRefCount);
		if (mFrameRefCount <= 0) {
		    //LOGV("decrement the reference count for mHeap");
		    mFrameRefCount = 0;
		    mHeap.clear();
		}
	    }
	    break;
	}
    }
    iFrameQueueMutex.Unlock();
    PVMFStatus status = PVMFFailure;

    if (!iCmdQueue.empty()) {
        AndroidCameraInputCmd cmd = iCmdQueue[0];
        iCmdQueue.erase(iCmdQueue.begin());

        switch(cmd.iType) {

        case CMD_INIT:
            status = DoInit();
            break;

        case CMD_START:
            status = DoStart();
            break;

        case CMD_PAUSE:
            status = DoPause();
            break;

        case CMD_FLUSH:
            status = DoFlush(cmd);
            break;

        case CMD_RESET:
            status = DoReset();
            break;

        case CMD_STOP:
            status = DoStop(cmd);
            break;

        case DATA_EVENT:
            // this is internal only - don't send RequestCompleted
            DoRead();
            status = PVMFPending;
            break;

        case CMD_QUERY_UUID:
        case CMD_QUERY_INTERFACE:
            status = PVMFSuccess;
            break;

        // these commands all fail
        case CMD_CANCEL_ALL_COMMANDS:
        case CMD_CANCEL_COMMAND:
        default:
            break;
        }
        // do RequestCompleted unless command is still pending
        if (status != PVMFPending) {
            DoRequestCompleted(cmd, status);
        }
    }

    if (!iCmdQueue.empty()) {
        // Run again if there are more things to process
        RunIfNotReady();
    }
}

PVMFCommandId AndroidCameraInput::AddCmdToQueue(AndroidCameraInputCmdType aType,
        const OsclAny* aContext,
        OsclAny* aData)
{
    LOGV("AddCmdToQueue");
    if (aType == DATA_EVENT) {
        LOGE("Invalid argument");
        OSCL_LEAVE(OsclErrArgument);
        return -1;
    }

    AndroidCameraInputCmd cmd;
    cmd.iType = aType;
    cmd.iContext = OSCL_STATIC_CAST(OsclAny*, aContext);
    cmd.iData = aData;
    cmd.iId = iCmdIdCounter;
    ++iCmdIdCounter;

    // TODO:
    // Check against out of memory failure
    int err = 0;
    OSCL_TRY(err, iCmdQueue.push_back(cmd));
    OSCL_FIRST_CATCH_ANY(err, LOGE("Out of memory"); return -1;);
    RunIfNotReady();
    return cmd.iId;
}

void AndroidCameraInput::AddDataEventToQueue(uint32 aMicroSecondsToEvent)
{
    LOGV("AddDataEventToQueue");
    AndroidCameraInputCmd cmd;
    cmd.iType = DATA_EVENT;

    int err = 0;
    OSCL_TRY(err, iCmdQueue.push_back(cmd));
    OSCL_FIRST_CATCH_ANY(err, LOGE("Out of memory"); return;);
    RunIfNotReady(aMicroSecondsToEvent);
}

void AndroidCameraInput::DoRequestCompleted(const AndroidCameraInputCmd& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    LOGV("DoRequestCompleted");
    PVMFCmdResp response(aCmd.iId, aCmd.iContext, aStatus, aEventData);

    for (uint32 i = 0; i < iObservers.size(); i++) {
        iObservers[i]->RequestCompleted(response);
    }
}

static void recording_frame_callback(const sp<IMemory>& frame, void *cookie)
{
    LOGV("recording_frame_callback");
    AndroidCameraInput* input = (AndroidCameraInput*) cookie;

    // this must not happen, and we can't release the frame if it does happen
    if (!input) {
        LOGE("Error - CameraInput has not been initialized");
        return;
    }

    input->postWriteAsync(frame);
}

PVMFStatus AndroidCameraInput::DoInit()
{
    LOGV("DoInit()");
    iState = STATE_INITIALIZED;
    iMilliSecondsPerDataEvent = (int32)(1000 / mFrameRate);
    iMicroSecondsPerDataEvent = (int32)(1000000 / mFrameRate);
    iDataEventCounter = 0;

    // create a camera if the app didn't supply one
    if (mCamera == 0) {
        mCamera = Camera::connect();
    }

    // always call setPreviewDisplay() regardless whether mCamera is just created or not
    // return failure if no display surface is available
    if (mCamera != NULL && mSurface != NULL) {
        mCamera->setPreviewDisplay(mSurface);
    } else {
        if (mCamera == NULL) {
            LOGE("Camera is not available");
        } else if (mSurface == NULL) {
            LOGE("No surface is available for display");
        }
        return PVMFFailure;
 
    }

    LOGD("Intended mFrameWidth=%d, mFrameHe=%d ",mFrameWidth, mFrameHeight);
    String8 s = mCamera->getParameters();
    CameraParameters p(s);
    p.setPreviewSize(mFrameWidth, mFrameHeight);
    s = p.flatten();
    mCamera->setParameters(s);

    // Since we may not honor the preview size that app has requested
    // It is a good idea to get the actual preview size and used it
    // for video recording.
    CameraParameters newCameraParam(mCamera->getParameters());
    newCameraParam.getPreviewSize(&mFrameWidth, &mFrameHeight);
    LOGD("Actual mFrameWidth=%d, mFrameHeight=%d ",mFrameWidth, mFrameHeight);
    if (mCamera->startPreview() != NO_ERROR) {
        return PVMFFailure;
    }
    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::DoStart()
{
    LOGV("DoStart");
    iState = STATE_STARTED;
    mCamera->setRecordingCallback(recording_frame_callback, this);
    if (mCamera->startRecording() != NO_ERROR) {
        return PVMFFailure;
    }
    AddDataEventToQueue(iMilliSecondsPerDataEvent);
    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::DoPause()
{
    LOGV("DoPause");
    iState = STATE_PAUSED;
    return PVMFSuccess;
}

// Does this work for reset?
PVMFStatus AndroidCameraInput::DoReset()
{
    LOGV("DoReset");
    if (iState != STATE_STOPPED)
    {
        iDataEventCounter = 0;
        if (mCamera != NULL) {
            mCamera->setRecordingCallback(NULL, this);
            mCamera->stopRecording();
            ReleaseQueuedFrames();
        }
    }
    while(!iCmdQueue.empty())
    {
        AndroidCameraInputCmd cmd = iCmdQueue[0];
        iCmdQueue.erase(iCmdQueue.begin());
    }
    Cancel();
    iState = STATE_IDLE;
    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::DoFlush(const AndroidCameraInputCmd& aCmd)
{
    LOGV("DoFlush");
    // This method should stop capturing media data but continue to send
    // captured media data that is already in buffer and then go to
    // stopped state.
    // However, in this case of file input we do not have such a buffer for
    // captured data, so this behaves the same way as stop.
    return DoStop(aCmd);
}

PVMFStatus AndroidCameraInput::DoStop(const AndroidCameraInputCmd& aCmd)
{
    LOGV("DoStop");
    iDataEventCounter = 0;
    mCamera->setRecordingCallback(NULL, this);
    mCamera->stopRecording();
    ReleaseQueuedFrames();
    iState = STATE_STOPPED;
    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::DoRead()
{
    LOGV("DoRead");
    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::AllocateKvp(PvmiKvp*& aKvp,
        PvmiKeyType aKey,
        int32 aNumParams)
{
    LOGV("AllocateKvp");
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
        buf = (uint8*)iAlloc.allocate(aNumParams * (sizeof(PvmiKvp) + keyLen));
        if (!buf) {
            OSCL_LEAVE(OsclErrNoMemory);
        }
    );
    OSCL_FIRST_CATCH_ANY(err,
        LOGE("kvp allocation failed");
        return PVMFErrNoMemory;
    );

    PvmiKvp* curKvp = aKvp = new (buf) PvmiKvp;
    buf += sizeof(PvmiKvp);
    for (int32 i = 1; i < aNumParams; ++i) {
        curKvp += i;
        curKvp = new (buf) PvmiKvp;
        buf += sizeof(PvmiKvp);
    }

    for (int32 i = 0; i < aNumParams; ++i) {
        aKvp[i].key = (char*)buf;
        oscl_strncpy(aKvp[i].key, aKey, keyLen);
        buf += keyLen;
    }

    return PVMFSuccess;
}

PVMFStatus AndroidCameraInput::VerifyAndSetParameter(PvmiKvp* aKvp,
        bool aSetParam)
{
    LOGV("VerifyAndSetParameter");

    if (!aKvp) {
        LOGE("Invalid key-value pair");
        return PVMFFailure;
    }

    if (!pv_mime_strcmp(aKvp->key, OUTPUT_FORMATS_VALTYPE)) {
		if(pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_YUV420) == 0) {
            return PVMFSuccess;
        } else  {
            LOGE("Unsupported format %d", aKvp->value.uint32_value);
            return PVMFFailure;
        }
    }

    LOGE("Unsupported parameter(%s)", aKvp->key);
    return PVMFFailure;
}

void AndroidCameraInput::SetPreviewSurface(const sp<android::ISurface>& surface)
{
    LOGV("SetPreviewSurface");
    mSurface = surface;

    if (mCamera != NULL) {
        mCamera->setPreviewDisplay(surface);
    }
}

void AndroidCameraInput::SetCamera(const sp<android::ICamera>& camera)
{
    LOGV("SetCamera");
    mFlags &= ~ FLAGS_SET_CAMERA | FLAGS_HOT_CAMERA;
    if (camera == NULL) {
        LOGV("camera is NULL");
        return;
    }

    // Connect our client to the camera remote
    mCamera = new Camera(camera);
    if (mCamera == NULL) {
        LOGE("Unable to connect to camera");
        return;
    }

    LOGV("Connected to camera");
    mFlags |= FLAGS_SET_CAMERA;
    if (mCamera->previewEnabled()) {
        mFlags |= FLAGS_HOT_CAMERA;
        LOGV("camera is hot");
    }
}

PVMFStatus AndroidCameraInput::postWriteAsync(const sp<IMemory>& frame)
{
    LOGV("postWriteAsync");

    if (frame == NULL) {
        LOGE("frame is a NULL pointer");
        return PVMFFailure;
    }
    if (iState != STATE_STARTED)
    {
        return PVMFSuccess;
    }

    // release the received recording frame right way
    // if recording has not been started yet or recording has already finished
    if (!isRecorderStarting()) {
        LOGV("Recording is not started, so recording frame is dropped");
        mCamera->releaseRecordingFrame(frame);
        return PVMFSuccess;
    }

    if (!iPeer) {
        LOGW("iPeer is NULL");
        return PVMFSuccess;
    }

    // if first event, set timestamp to zero
    if (iDataEventCounter == 0) {
        iStartTickCount = systemTime(SYSTEM_TIME_MONOTONIC) / 1000000;
        iTimeStamp = 0;
    } else {
        uint32 timeStamp = (systemTime(SYSTEM_TIME_MONOTONIC) / 1000000) - iStartTickCount;
        if (iTimeStamp != timeStamp) {
            iTimeStamp = timeStamp;
        } else {
            ++iTimeStamp;
        }
    }

    // get memory offset for frame buffer
    ssize_t offset = 0;
    size_t size = 0;
    sp<IMemoryHeap> heap = frame->getMemory(&offset, &size);
    //LOGV("ID = %d, base = %p, offset = %p, size = %d", heap->getHeapID(), heap->base(), offset, size);

    //LOGV("@@@@@@@@@@@@@ incrementing reference count (%d) @@@@@@@@@@@@@@@", mFrameRefCount);
    if (mHeap == 0) {
        //LOGV("initialize the reference to frame heap memory");
        mHeap = heap;
    } else if (mHeap != heap) {
        LOGE("mHeap != heap");
        return PVMFFailure;
    }

    // queue data to be sent to peer
    AndroidCameraInputMediaData data;
    data.iXferHeader.seq_num = iDataEventCounter++;
    data.iXferHeader.timestamp = iTimeStamp;
    data.iXferHeader.flags = 0;
    data.iXferHeader.duration = 0;
    data.iXferHeader.stream_id = 0;
    data.iFrameBuffer = frame;
    data.iFrameSize = size;

    // lock mutex and queue frame buffer
    iFrameQueueMutex.Lock();
    iFrameQueue.push_back(data);
    iFrameQueueMutex.Unlock();
    RunIfNotReady();

    return PVMFSuccess; 
}

