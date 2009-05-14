/*
 * Copyright (C) 2008, The Android Open Source Project
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

#ifndef ANDROID_CAMERA_INPUT_H_INCLUDED
#define ANDROID_CAMERA_INPUT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

using namespace android;

class ISurface;
class ICamera;

/**
 * Enumerated list of asychronous commands for AndroidCameraInput
 */
typedef enum
{
    CMD_QUERY_UUID,
    CMD_QUERY_INTERFACE,
    CMD_INIT,
    CMD_START,
    CMD_PAUSE,
    CMD_FLUSH,
    CMD_STOP,
    CMD_CANCEL_ALL_COMMANDS,
    CMD_CANCEL_COMMAND,
    CMD_RESET,
    DATA_EVENT,
    INVALID_CMD
} AndroidCameraInputCmdType;

#define DEFAULT_FRAME_WIDTH        320
#define DEFAULT_FRAME_HEIGHT       240
#define DEFAULT_FRAME_RATE         20.0

/**
 * Class containing information for a command or data event
 */
class AndroidCameraInputCmd
{
public:
    AndroidCameraInputCmd()
    {
        iId      = 0;
        iType    = INVALID_CMD;
        iContext = NULL;
        iData    = NULL;
    }

    AndroidCameraInputCmd(const AndroidCameraInputCmd& aCmd)
    {
        Copy(aCmd);
    }

    ~AndroidCameraInputCmd() {}

    AndroidCameraInputCmd& operator=(const AndroidCameraInputCmd& aCmd)
    {
        Copy(aCmd);
        return (*this);
    }

    PVMFCommandId iId;       /** ID assigned to this command */
    int32         iType;     /** AndroidCameraInputCmdType value */
    OsclAny*      iContext;  /** Other data associated with this command */
    OsclAny*      iData;     /** Other data associated with this command */

private:

    void Copy(const AndroidCameraInputCmd& aCmd)
    {
        iId      = aCmd.iId;
        iType    = aCmd.iType;
        iContext = aCmd.iContext;
        iData    = aCmd.iData;
    }
};

// NOTE: The copy function does a shallow copy, it does not copy the data
// pointed to by iFrameBuffer. This is as intended, the memory is safe to use
// until camera preview is stopped.
class AndroidCameraInputMediaData
{
public:
    AndroidCameraInputMediaData() {
        memset(this, 0, sizeof(this));
    }

    AndroidCameraInputMediaData(const AndroidCameraInputMediaData& aData) {
       Copy(aData); 
    }

    AndroidCameraInputMediaData& operator=(const AndroidCameraInputMediaData& aData) {
        Copy(aData);
        return (*this);
    }

    PVMFCommandId           iId;
    PvmiMediaXferHeader     iXferHeader;
    sp<IMemory>             iFrameBuffer;
    size_t                  iFrameSize;

private:
    void Copy(const AndroidCameraInputMediaData& aData) {
        iId = aData.iId;
        iXferHeader = aData.iXferHeader;
        iFrameBuffer = aData.iFrameBuffer;  // won't mess up the reference count
        iFrameSize = aData.iFrameSize;
    }
};

class AndroidCameraInput
    : public OsclTimerObject,
      public PvmiMIOControl,
      public PvmiMediaTransfer,
      public PvmiCapabilityAndConfig
{
public:
    AndroidCameraInput();
    virtual ~AndroidCameraInput();

    // Pure virtuals from PvmiMIOControl
    OSCL_IMPORT_REF PVMFStatus connect(PvmiMIOSession& aSession,
        PvmiMIOObserver* aObserver);

    OSCL_IMPORT_REF PVMFStatus disconnect(PvmiMIOSession aSession);
    OSCL_IMPORT_REF PvmiMediaTransfer* createMediaTransfer(PvmiMIOSession& aSession,
        PvmiKvp* read_formats = NULL,
        int32 read_flags = 0,
        PvmiKvp* write_formats = NULL,
        int32 write_flags = 0);

    OSCL_IMPORT_REF void deleteMediaTransfer(PvmiMIOSession& aSession,
                                             PvmiMediaTransfer* media_transfer);
    
    OSCL_IMPORT_REF PVMFCommandId QueryUUID(const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly = false,
        const OsclAny* aContext = NULL);

    OSCL_IMPORT_REF PVMFCommandId QueryInterface(const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext = NULL);

    OSCL_IMPORT_REF PVMFCommandId Init(const OsclAny* aContext=NULL);
    OSCL_IMPORT_REF PVMFCommandId Start(const OsclAny* aContext=NULL);
    OSCL_IMPORT_REF PVMFCommandId Reset(const OsclAny* aContext=NULL);
    OSCL_IMPORT_REF PVMFCommandId Pause(const OsclAny* aContext=NULL);
    OSCL_IMPORT_REF PVMFCommandId Flush(const OsclAny* aContext=NULL);
    OSCL_IMPORT_REF PVMFCommandId DiscardData(PVMFTimestamp aTimestamp,
        const OsclAny* aContext=NULL);

    OSCL_IMPORT_REF PVMFCommandId DiscardData(const OsclAny* aContext = NULL);
    OSCL_IMPORT_REF PVMFCommandId Stop(const OsclAny* aContext = NULL);
    OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFCommandId aCmdId,
        const OsclAny* aContext=NULL);

    OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(const OsclAny* aContext = NULL);
    OSCL_IMPORT_REF void ThreadLogon();
    OSCL_IMPORT_REF void ThreadLogoff();

    // Pure virtuals from PvmiMediaTransfer
    OSCL_IMPORT_REF void setPeer(PvmiMediaTransfer* aPeer);
    OSCL_IMPORT_REF void useMemoryAllocators(OsclMemAllocator* write_alloc = NULL);
    OSCL_IMPORT_REF PVMFCommandId writeAsync(uint8 format_type,
        int32 format_index,
        uint8* data,
        uint32 data_len,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext = NULL);

    OSCL_IMPORT_REF void writeComplete(PVMFStatus aStatus,
        PVMFCommandId write_cmd_id,
        OsclAny* aContext);

    OSCL_IMPORT_REF PVMFCommandId readAsync(uint8* data,
        uint32 max_data_len,
        OsclAny* aContext = NULL,
        int32* formats = NULL,
        uint16 num_formats = 0);

    OSCL_IMPORT_REF void readComplete(PVMFStatus aStatus,
        PVMFCommandId read_cmd_id,
        int32 format_index,
        const PvmiMediaXferHeader& data_header_info,
        OsclAny* aContext);

    OSCL_IMPORT_REF void statusUpdate(uint32 status_flags);
    OSCL_IMPORT_REF void cancelCommand(PVMFCommandId aCmdId);
    OSCL_IMPORT_REF void cancelAllCommands();

    // Pure virtuals from PvmiCapabilityAndConfig
    OSCL_IMPORT_REF void setObserver (PvmiConfigAndCapabilityCmdObserver* aObserver);
    OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters, int& num_parameter_elements,
        PvmiCapabilityContext aContext);

    OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements);

    OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession,
        PvmiCapabilityContext& aContext);

    OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession,
        PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters,
        int num_parameter_elements);

    OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession,
        PvmiCapabilityContext& aContext);

    OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp);

    OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp*& aRet_kvp,
        OsclAny* context = NULL);

    OSCL_IMPORT_REF uint32 getCapabilityMetric (PvmiMIOSession aSession);
    OSCL_IMPORT_REF PVMFStatus verifyParametersSync (PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements);

    // Android-specific stuff
    void SetPreviewSurface(const sp<android::ISurface>& surface);
    void SetFrameSize(int w, int h);
    void SetFrameRate(int frames_per_second);
    void SetCamera(const sp<android::ICamera>& camera);

    // add for Camcorder
    PVMFStatus              postWriteAsync(const sp<IMemory>& frame);

    bool isRecorderStarting() { return iState==STATE_STARTED?true:false; }

private:
    // release all queued recording frames that have not been
    // given the chance to be sent out.
    void ReleaseQueuedFrames();

    void Run();
    void FrameSizeChanged();

    PVMFCommandId AddCmdToQueue(AndroidCameraInputCmdType aType,
        const OsclAny* aContext,
        OsclAny* aData1 = NULL);

    void AddDataEventToQueue(uint32 aMicroSecondsToEvent);
    void DoRequestCompleted(const AndroidCameraInputCmd& aCmd, PVMFStatus aStatus, OsclAny* aEventData=NULL);

    PVMFStatus DoInit();
    PVMFStatus DoStart();
    PVMFStatus DoReset();
    PVMFStatus DoPause();
    PVMFStatus DoFlush(const AndroidCameraInputCmd& aCmd);
    PVMFStatus DoStop(const AndroidCameraInputCmd& aCmd);
    PVMFStatus DoRead();

    /**
     * Allocate a specified number of key-value pairs and set the keys
     *
     * @param aKvp Output parameter to hold the allocated key-value pairs
     * @param aKey Key for the allocated key-value pairs
     * @param aNumParams Number of key-value pairs to be allocated
     * @return Completion status
     */
    PVMFStatus AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams);

    /**
     * Verify one key-value pair parameter against capability of the port and
     * if the aSetParam flag is set, set the value of the parameter corresponding to
     * the key.
     *
     * @param aKvp Key-value pair parameter to be verified
     * @param aSetParam If true, set the value of parameter corresponding to the key.
     * @return PVMFSuccess if parameter is supported, else PVMFFailure
     */
    PVMFStatus VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam=false);

    // Command queue
    uint32 iCmdIdCounter;
    Oscl_Vector<AndroidCameraInputCmd, OsclMemAllocator> iCmdQueue;

    // PvmiMIO sessions
    Oscl_Vector<PvmiMIOObserver*, OsclMemAllocator> iObservers;

    PvmiMediaTransfer* iPeer;

    // Thread logon
    bool iThreadLoggedOn;

    int32 iDataEventCounter;
    int32 iStartTickCount;

    // Timing
    int32 iMilliSecondsPerDataEvent;
    int32 iMicroSecondsPerDataEvent;
    PVMFTimestamp iTimeStamp;

    // Allocator for simple media data buffer
    OsclMemAllocator iAlloc;

    Oscl_Vector<AndroidCameraInputMediaData, OsclMemAllocator> iSentMediaData;

    Oscl_Vector<AndroidCameraInputMediaData, OsclMemAllocator> iFrameQueue;
    OsclMutex iFrameQueueMutex;

    AndroidCameraInputCmd iPendingCmd;

    enum AndroidCameraFlags {
        FLAGS_SET_CAMERA        = 1L << 0,
        FLAGS_HOT_CAMERA        = 1L << 1,
    };

    // Camera specific stuff
    sp<android::ISurface>   mSurface;
    int32                   mSurfaceWidth;
    int32                   mSurfaceHeight;
    int32                   mFrameWidth;
    int32                   mFrameHeight;
    float                   mFrameRate;
    sp<android::Camera>     mCamera;
    sp<IMemoryHeap>         mHeap;
    int32                   mFrameRefCount;
    int32                   mFlags;


    // State machine
    enum AndroidCameraInputState
    {
        STATE_IDLE,
        STATE_INITIALIZED,
        STATE_STARTED,
        STATE_FLUSHING,
        STATE_PAUSED,
        STATE_STOPPING,
        STATE_STOPPED
    };

    AndroidCameraInputState iState;

};

#endif // ANDROID_CAMERA_INPUT_H_INCLUDED
