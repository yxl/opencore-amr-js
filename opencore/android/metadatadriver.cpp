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
#undef LOG_TAG
#define LOG_TAG "MetadataDriver"
#include <utils/Log.h>

#include <media/thread_init.h>
#include <core/SkBitmap.h>
#include <private/media/VideoFrame.h>

#include "metadatadriver.h"

using namespace android;

const char* MetadataDriver::ALBUM_ART_KEY = "graphic";

const char* MetadataDriver::METADATA_KEYS[NUM_METADATA_KEYS] = {
        "tracknumber",
        "album",
        "artist",
        "author",
        "composer",
        "date",
        "genre",
        "title",
        "year",
        "duration",
        "num-tracks",
        "drm/is-protected",
        "track-info/codec-name",
        "rating",
        "comment",
        "copyright",
        "track-info/bit-rate",
        "track-info/frame-rate",
        "track-info/video/format",
        "track-info/video/height",
        "track-info/video/width",
};

static void dumpkeystolog(PVPMetadataList list)
{
    LOGV("dumpkeystolog");
    uint32 n = list.size();
    for(uint32 i = 0; i < n; ++i) {
        LOGI("@@@@@ wma key: %s", list[i].get_cstr());
    }
}

MetadataDriver::MetadataDriver(uint32 mode): OsclActiveObject(OsclActiveObject::EPriorityNominal, "MetadataDriver")
{
    LOGV("constructor");
    mMode = mode;
    mUtil = NULL;
    mDataSource = NULL;
#if BEST_THUMBNAIL_MODE
    mLocalDataSource = NULL;
#endif
    mCmdId = 0;
    mContextObjectRefValue = 0x5C7A; // Some random number
    mContextObject = mContextObjectRefValue;
    mMediaAlbumArt = NULL;
    mVideoFrame = NULL;
    for (uint32 i = 0; i < NUM_METADATA_KEYS; ++i) {
        mMetadataValues[i][0] = '\0';
    }
    LOGV("constructor: Mode (%d).", mMode);
}

/*static*/ int MetadataDriver::startDriverThread(void *cookie)
{
    LOGV("startDriverThread");
    MetadataDriver *driver = (MetadataDriver *)cookie;
    return driver->retrieverThread();
}

int MetadataDriver::retrieverThread()
{
    LOGV("retrieverThread");
    if (!InitializeForThread()) {
        LOGV("InitializeForThread fail");
        mSyncSem->Signal();
        return -1;
    }

    OMX_Init();
    OsclScheduler::Init("PVAuthorEngineWrapper");
    mState = STATE_CREATE;
    AddToScheduler();
    RunIfNotReady();
    OsclExecScheduler *sched = OsclExecScheduler::Current();
    sched->StartScheduler();

    mSyncSem->Signal();  // Signal that doSetDataSource() is done.
    OsclScheduler::Cleanup();
    OMX_Deinit();
    UninitializeForThread();
    return 0;
}


MetadataDriver::~MetadataDriver()
{
    LOGV("destructor");
    mCmdId = 0;
    delete mVideoFrame;
    mVideoFrame = NULL;
    delete mMediaAlbumArt;
    mMediaAlbumArt = NULL;
    delete mSyncSem;
    mSyncSem = NULL;
}

const char* MetadataDriver::extractMetadata(int keyCode)
{
    LOGV("extractMetadata");
    char *value = NULL;
    if (mMode & GET_METADATA_ONLY) {
        // Comparing int with unsigned int
        if (keyCode < 0 || keyCode >= (int) NUM_METADATA_KEYS) {
            LOGE("extractMetadata: Invalid keyCode: %d.", keyCode);
        } else {
            value = mMetadataValues[keyCode];
        }
    }
    if (value == NULL || value[0] == '\0') {
        return NULL;
    }
    return value;
}

MediaAlbumArt *MetadataDriver::extractAlbumArt()
{
    LOGV("extractAlbumArt");
    if (mMode & GET_METADATA_ONLY) {  // copy out
        if (mMediaAlbumArt != NULL && mMediaAlbumArt->mSize > 0) {
            return new MediaAlbumArt(*mMediaAlbumArt);
        } else {
            LOGE("failed to extract album art");
            return NULL;
        }
    }
    LOGE("extractAlbumArt: invalid mode (%d) to extract album art", mMode);
    return NULL;
}

// How to better manage these constant strings?
bool MetadataDriver::containsSupportedKey(const OSCL_HeapString<OsclMemAllocator>& str) const
{
    LOGV("containsSupportedKey");
    const char* cStr = str.get_cstr();
    for (uint32 i = 0; i < NUM_METADATA_KEYS; ++i) {
        if (strcasestr(cStr, METADATA_KEYS[i])) {
            return true;
        }
    }

    // Key "graphic" is a special metadata key for retrieving album art image.
    if (strcasestr(cStr, "graphic")) {
        return true;
    }
    return false;
}

// Delete unnecessary keys before retrieving the metadata values to avoid
// retrieving all metadata values for all metadata keys
void MetadataDriver::trimKeys()
{
    LOGV("trimKeys");
    dumpkeystolog(mMetadataKeyList);
    mActualMetadataKeyList.clear();
    uint32 n = mMetadataKeyList.size();
    mActualMetadataKeyList.reserve(n);
    for (uint32 i = 0; i < n; ++i) {
        if (containsSupportedKey(mMetadataKeyList[i])) {
            mActualMetadataKeyList.push_back(mMetadataKeyList[i]);
        }
    }
    mMetadataKeyList.clear();
}

// Returns:
// 1. UNKNOWN_ERROR
//    a. If the metadata value(s) is too long, and cannot be hold in valueLength bytes
//    b. If nothing is found
// 2. OK
//    a. If metadata value(s) is found
status_t MetadataDriver::extractMetadata(const char* key, char* value, uint32 valueLength)
{
    LOGV("extractMetadata");
    bool found = false;
    value[0] = '\0';
    for (uint32 i = 0, n = mMetadataValueList.size(); i < n; ++i) {
        if (strcasestr(mMetadataValueList[i].key, key)) {
            found = true;
            switch(GetValTypeFromKeyString(mMetadataValueList[i].key)) {
                case PVMI_KVPVALTYPE_CHARPTR: {
                    uint32 length = oscl_strlen(mMetadataValueList[i].value.pChar_value) + 1;
                    if (length > valueLength) {
                        return UNKNOWN_ERROR;
                    }
                    oscl_snprintf(value, length, "%s", mMetadataValueList[i].value.pChar_value);
                    value[length] = '\0';
                    LOGV("value of char: %s.", mMetadataValueList[i].value.pChar_value);
                    break;
                }
                case PVMI_KVPVALTYPE_WCHARPTR: {
                    // Assume string is in UCS-2 encoding so convert to UTF-8.
                    uint32 length = oscl_strlen(mMetadataValueList[i].value.pWChar_value) + 1;
                    if (length > valueLength) {
                        return UNKNOWN_ERROR;
                    }
                    length = oscl_UnicodeToUTF8(mMetadataValueList[i].value.pWChar_value, length, value, valueLength);
                    value[length] = '\0';
                    LOGV("value of wchar: %ls.", mMetadataValueList[i].value.pWChar_value);
                    break;
                }
                case PVMI_KVPVALTYPE_UINT32:
                    oscl_snprintf(value, valueLength, "%d", mMetadataValueList[i].value.uint32_value);
                    value[valueLength] = '\0';
                    break;

                case PVMI_KVPVALTYPE_INT32:
                    oscl_snprintf(value, valueLength, "%d", mMetadataValueList[i].value.int32_value);
                    value[valueLength] = '\0';
                    break;

                case PVMI_KVPVALTYPE_UINT8:
                    oscl_snprintf(value, valueLength, "%d", mMetadataValueList[i].value.uint8_value);
                    value[valueLength] = '\0';
                    break;

                case PVMI_KVPVALTYPE_FLOAT:
                    oscl_snprintf(value, valueLength, "%f", mMetadataValueList[i].value.float_value);
                    value[valueLength] = '\0';
                    break;

                case PVMI_KVPVALTYPE_DOUBLE:
                    oscl_snprintf(value, valueLength, "%f", mMetadataValueList[i].value.double_value);
                    value[valueLength] = '\0';
                    break;

                case PVMI_KVPVALTYPE_BOOL:
                    oscl_snprintf(value, valueLength, "%s", mMetadataValueList[i].value.bool_value? "true": "false");
                    value[valueLength] = '\0';
                    break;

                default:
                    return UNKNOWN_ERROR;
            }
            break;
        }
    }
    return found? OK: UNKNOWN_ERROR;
}

void MetadataDriver::cacheMetadataRetrievalResults()
{
    LOGV("cacheMetadataRetrievalResults");
#if _METADATA_DRIVER_INTERNAL_DEBUG_ENABLE_
    for (uint32 i = 0, n = mMetadataValueList.size(); i < n; ++i) {
        LOGV("Value %d:   Key string: %s.", (i+1), mMetadataValueList[i].key);
    }
#endif
    for (uint32 i = 0; i < NUM_METADATA_KEYS; ++i) {
        LOGV("extract metadata key: %s", METADATA_KEYS[i]);
        extractMetadata(METADATA_KEYS[i], mMetadataValues[i], MAX_METADATA_STRING_LENGTH - 1);
    }
    doExtractAlbumArt();
}

status_t MetadataDriver::extractEmbeddedAlbumArt(const PvmfApicStruct* apic)
{
    LOGV("extractEmbeddedAlbumArt");
    char* buf  = (char*) apic->iGraphicData;
    uint32 size = apic->iGraphicDataLen;
    LOGV("extractEmbeddedAlbumArt: Embedded graphic or album art (%d bytes) is found.", size);
    if (size && buf) {
        delete mMediaAlbumArt;
        mMediaAlbumArt = new MediaAlbumArt();
        if (mMediaAlbumArt == NULL) {
            LOGE("extractEmbeddedAlbumArt: Not enough memory to hold a MediaAlbumArt object");
            return NO_MEMORY;
        }
        mMediaAlbumArt->mSize = size;
        mMediaAlbumArt->mData = new uint8[size];
        if (mMediaAlbumArt->mData == NULL) {
            LOGE("extractEmbeddedAlbumArt: Not enough memory to hold the binary data of a MediaAlbumArt object");
            delete mMediaAlbumArt;
            mMediaAlbumArt = NULL;
            return NO_MEMORY;
        }
        memcpy(mMediaAlbumArt->mData, buf, size);
        return NO_ERROR;
    }
    return BAD_VALUE;
}

status_t MetadataDriver::extractExternalAlbumArt(const char* url)
{
    LOGV("extractExternalAlbumArt: External graphic or album art is found: %s.", url);
    delete mMediaAlbumArt;
    mMediaAlbumArt = new MediaAlbumArt(url);
    return (mMediaAlbumArt && mMediaAlbumArt->mSize > 0)? OK: BAD_VALUE; 
}

// Finds the first album art and extract it.
status_t MetadataDriver::doExtractAlbumArt()
{
    LOGV("doExtractAlbumArt");
    status_t status = UNKNOWN_ERROR;
    for (uint32 i = 0, n = mMetadataValueList.size(); i < n; ++i) {
        if (strcasestr(mMetadataValueList[i].key, ALBUM_ART_KEY)) {
            LOGV("doExtractAlbumArt: album art key: %s", mMetadataValueList[i].key);
            if (PVMI_KVPVALTYPE_KSV == GetValTypeFromKeyString(mMetadataValueList[i].key)) {
                const char* embeddedKey = "graphic;format=APIC;valtype=ksv";
                const char* externalKey = "graphic;valtype=char*";
                if (strstr(mMetadataValueList[i].key, embeddedKey) && mMetadataValueList[i].value.key_specific_value) {
                    // Embedded album art.
                    status = extractEmbeddedAlbumArt(((PvmfApicStruct*)mMetadataValueList[i].value.key_specific_value));
                } else if (strstr(mMetadataValueList[i].key, externalKey)) {
                    // Album art linked with an external url.
                    status = extractExternalAlbumArt(mMetadataValueList[i].value.pChar_value);
                }

                if (status != OK) {
                    continue;
                }
                return status;  // Found the album art.
            }
        }
    }
    return UNKNOWN_ERROR;
}

void MetadataDriver::clearCache()
{
    LOGV("clearCache");
    delete mVideoFrame;
    mVideoFrame = NULL;
    delete mMediaAlbumArt;
    mMediaAlbumArt = NULL;
    for(uint32 i = 0; i < NUM_METADATA_KEYS; ++i) {
        mMetadataValues[i][0] = '\0';
    }
}

status_t MetadataDriver::setDataSource(const char* srcUrl)
{
    LOGV("setDataSource");
    // Don't let somebody trick us in to reading some random block of memory.
    if (strncmp("mem://", srcUrl, 6) == 0) {
        LOGE("setDataSource: Invalid url (%s).", srcUrl);
        return UNKNOWN_ERROR;
    }
    if (oscl_strlen(srcUrl) > MAX_STRING_LENGTH) {
        LOGE("setDataSource: Data source url length (%d) is too long.", oscl_strlen(srcUrl));
        return UNKNOWN_ERROR;
    }
    clearCache();
    return doSetDataSource(srcUrl);
}

status_t MetadataDriver::doSetDataSource(const char* dataSrcUrl)
{
    LOGV("doSetDataSource");
    if (mMode & GET_FRAME_ONLY) {
#if BEST_THUMBNAIL_MODE
        mFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
        mFrameSelector.iFrameInfo.iTimeOffsetMilliSec = 0;
#else
        mFrameSelector.iSelectionMethod=PVFrameSelector::SPECIFIC_FRAME;
        mFrameSelector.iFrameInfo.iFrameIndex=0;
#endif
    }
    mIsSetDataSourceSuccessful = false;
    oscl_wchar tmpWCharBuf[MAX_STRING_LENGTH];
    oscl_UTF8ToUnicode(dataSrcUrl, oscl_strlen(dataSrcUrl), tmpWCharBuf, sizeof(tmpWCharBuf));
    mDataSourceUrl.set(tmpWCharBuf, oscl_strlen(tmpWCharBuf));
    mSyncSem = new OsclSemaphore();
    mSyncSem->Create();
    createThreadEtc(MetadataDriver::startDriverThread, this, "PVMetadataRetriever");
    mSyncSem->Wait();
    return mIsSetDataSourceSuccessful? OK: UNKNOWN_ERROR;
}

VideoFrame* MetadataDriver::captureFrame()
{
    LOGV("captureFrame");
    if (mMode & GET_FRAME_ONLY) {  // copy out
        if (mVideoFrame != NULL && mVideoFrame->mSize > 0) {
            return new VideoFrame(*mVideoFrame);
        } else {
            LOGE("failed to capture frame");
            return NULL;
        }
    }
    LOGE("captureFrame: invalid mode (%d) to capture a frame", mMode);
    return NULL;
}

void MetadataDriver::doColorConversion()
{
    LOGV("doColorConversion");
    // Do color conversion using PV's color conversion utility
    int width  = mFrameBufferProp.iFrameWidth;
    int height = mFrameBufferProp.iFrameHeight;
    int displayWidth  = mFrameBufferProp.iDisplayWidth;
    int displayHeight = mFrameBufferProp.iDisplayHeight;
    SkBitmap *bitmap = new SkBitmap();
    if (!bitmap) {
        LOGE("doColorConversion: cannot instantiate a SkBitmap object.");
        return;
    }
    bitmap->setConfig(SkBitmap::kRGB_565_Config, displayWidth, displayHeight);
    if (!bitmap->allocPixels()) {
        LOGE("allocPixels failed");
        delete bitmap;
        return;
    }
    ColorConvertBase* colorConverter = ColorConvert16::NewL();
    if (!colorConverter ||
        !colorConverter->Init(width, height, width, displayWidth, displayHeight, displayWidth, CCROTATE_NONE) ||
        !colorConverter->SetMode(1) ||
        !colorConverter->Convert(mFrameBuffer, (uint8*)bitmap->getPixels())) {
        LOGE("failed to do color conversion");
        delete colorConverter;
        delete bitmap;
        return;
    }
    delete colorConverter;

    // Store the SkBitmap pixels in a private shared structure with known
    // internal memory layout so that the pixels can be sent across the
    // binder interface
    delete mVideoFrame;
    mVideoFrame = new VideoFrame();
    if (!mVideoFrame) {
        LOGE("failed to allocate memory for a VideoFrame object");
        delete bitmap;
        return;
    }
    mVideoFrame->mWidth = width;
    mVideoFrame->mHeight = height;
    mVideoFrame->mDisplayWidth  = displayWidth;
    mVideoFrame->mDisplayHeight = displayHeight;
    mVideoFrame->mSize = bitmap->getSize();
    LOGV("display width (%d) and height (%d), and size (%d)", displayWidth, displayHeight, mVideoFrame->mSize);
    mVideoFrame->mData = new uint8[mVideoFrame->mSize];
    if (!mVideoFrame->mData) {
        LOGE("doColorConversion: cannot allocate buffer to hold SkBitmap pixels");
        delete bitmap;
        delete mVideoFrame;
        mVideoFrame = NULL;
        return;
    }
    memcpy(mVideoFrame->mData, (uint8*) bitmap->getPixels(), mVideoFrame->mSize);
    delete bitmap;
}

// Instantiate a frame and metadata utility object.
void MetadataDriver::handleCreate()
{
    LOGV("handleCreate");
    int error = 0;
    OSCL_TRY(error, mUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility((char*)PVMF_MIME_YUV420, this, this, this));
    if (error || mUtil->SetMode(PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL) != PVMFSuccess) {
        handleCommandFailure();
    } else {
        mState = STATE_ADD_DATA_SOURCE;
        RunIfNotReady();
    }
}

// Create a data source and add it.
void MetadataDriver::handleAddDataSource()
{
    LOGV("handleAddDataSource");
    int error = 0;
    mDataSource = new PVPlayerDataSourceURL;
    if (mDataSource) {
        mDataSource->SetDataSourceURL(mDataSourceUrl);
        mDataSource->SetDataSourceFormatType((char*)PVMF_MIME_FORMAT_UNKNOWN);
        if (mMode & GET_FRAME_ONLY) {
#if BEST_THUMBNAIL_MODE
            // Set the intent to thumbnails.
            mLocalDataSource = new PVMFLocalDataSource();
            mLocalDataSource->iIntent = BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS;
            mDataSource->SetDataSourceContextData((OsclAny*)mLocalDataSource);
#endif
        }
        OSCL_TRY(error, mCmdId = mUtil->AddDataSource(*mDataSource, (OsclAny*)&mContextObject));
        OSCL_FIRST_CATCH_ANY(error, handleCommandFailure());
    }
}

void MetadataDriver::handleRemoveDataSource()
{
    LOGV("handleRemoveDataSource");
    int error = 0;
    OSCL_TRY(error, mCmdId = mUtil->RemoveDataSource(*mDataSource, (OsclAny*)&mContextObject));
    OSCL_FIRST_CATCH_ANY(error, handleCommandFailure());
}

// Clean up, due to either failure or task completion.
void MetadataDriver::handleCleanUp()
{
    LOGV("handleCleanUp");
    if (mUtil)
    {
        PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(mUtil);
        mUtil = NULL;
    }
#if BEST_THUMBNAIL_MODE
    delete mLocalDataSource;
    mLocalDataSource = NULL;
#endif
    delete mDataSource;
    mDataSource = NULL;

    OsclExecScheduler *sched=OsclExecScheduler::Current();
    if (sched) {
        sched->StopScheduler();
    }
}

// Retrieve all the available metadata keys.
void MetadataDriver::handleGetMetadataKeys()
{
    LOGV("handleGetMetadataKeys");
    int error = 0;
    mMetadataKeyList.clear();
    OSCL_TRY(error, mCmdId = mUtil->GetMetadataKeys(mMetadataKeyList, 0, -1, NULL, (OsclAny*)&mContextObject));
    OSCL_FIRST_CATCH_ANY(error, handleCommandFailure());
}

// Retrieve a frame and store the contents into an internal buffer.
void MetadataDriver::handleGetFrame()
{
    LOGV("handleGetFrame");
    int error = 0;
    mFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
    OSCL_TRY(error, mCmdId = mUtil->GetFrame(mFrameSelector, mFrameBuffer, mFrameBufferSize, mFrameBufferProp, (OsclAny*)&mContextObject));
    OSCL_FIRST_CATCH_ANY(error, handleCommandFailure());
}

// Retrieve all the available metadata values associated with the given keys.
void MetadataDriver::handleGetMetadataValues()
{
    LOGV("handleGetMetadataValues");
    int error = 0;
    mNumMetadataValues = 0;
    mMetadataValueList.clear();
    trimKeys();  // Switch to use actual supported key list.
    OSCL_TRY(error, mCmdId = mUtil->GetMetadataValues(mActualMetadataKeyList, 0, -1, mNumMetadataValues, mMetadataValueList, (OsclAny*)&mContextObject));
    OSCL_FIRST_CATCH_ANY(error, handleCommandFailure());
}

void MetadataDriver::Run()
{
    LOGV("Run (%d)", mState);
    switch(mState) {
        case STATE_CREATE:
            handleCreate();
            break;
        case STATE_ADD_DATA_SOURCE:
            handleAddDataSource();
            break;
        case STATE_GET_METADATA_KEYS:
            handleGetMetadataKeys();
            break;
        case STATE_GET_METADATA_VALUES:
            handleGetMetadataValues();
            break;
        case STATE_GET_FRAME:
            handleGetFrame();
            break;
        case STATE_REMOVE_DATA_SOURCE:
            handleRemoveDataSource();
            break;
        default:
            handleCleanUp();
            break;
    }
}

bool MetadataDriver::isCommandSuccessful(const PVCmdResponse& aResponse) const
{
    LOGV("isCommandSuccessful");
    bool success = ((aResponse.GetCmdId() == mCmdId) &&
            (aResponse.GetCmdStatus() == PVMFSuccess) &&
            (aResponse.GetContext() == (OsclAny*)&mContextObject));
    if (!success) {
        LOGE("isCommandSuccessful: Command id(%d and expected %d) and status (%d and expected %d), data corruption (%s) at state (%d).",
             aResponse.GetCmdId(), mCmdId, aResponse.GetCmdStatus(), PVMFSuccess, (aResponse.GetContext() == (OsclAny*)&mContextObject)? "false": "true", mState);
    }
    return success;
}

void MetadataDriver::handleCommandFailure()
{
    LOGV("handleCommandFailure");
    if (mState == STATE_REMOVE_DATA_SOURCE) {
        mState = STATE_CLEANUP_AND_COMPLETE;
    }
    else{
        mState = STATE_REMOVE_DATA_SOURCE;
    }
    RunIfNotReady();
}

// Callback handler for a request completion by frameandmetadatautility.
void MetadataDriver::CommandCompleted(const PVCmdResponse& aResponse)
{
    LOGV("CommandCompleted (%d)", mState);
    if (!isCommandSuccessful(aResponse)) {
        handleCommandFailure();
        return;
    }

    switch(mState) {
        case STATE_ADD_DATA_SOURCE:
            if (mMode & GET_METADATA_ONLY) {
                mState = STATE_GET_METADATA_KEYS;
            } else if (mMode & GET_FRAME_ONLY) {
                mState = STATE_GET_FRAME;
            } else {
                LOGV("CommandCompleted: Neither retrieve metadata nor capture frame.");
                mState = STATE_REMOVE_DATA_SOURCE;
            }
            mIsSetDataSourceSuccessful = true;
            break;
        case STATE_GET_METADATA_KEYS:
            mState = STATE_GET_METADATA_VALUES;
            break;
        case STATE_GET_METADATA_VALUES:
            if (mMode & GET_FRAME_ONLY) {
                mState = STATE_GET_FRAME;
            } else {
                mState = STATE_REMOVE_DATA_SOURCE;
            }
            cacheMetadataRetrievalResults();
            break;
        case STATE_GET_FRAME:
            doColorConversion();
            mState = STATE_REMOVE_DATA_SOURCE;
            break;
        case STATE_REMOVE_DATA_SOURCE:
            mState = STATE_CLEANUP_AND_COMPLETE;
            break;
        default:
            mState = STATE_CLEANUP_AND_COMPLETE;
            break;
    }
    RunIfNotReady();
}

void MetadataDriver::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    // Error occurs, clean up and terminate.
    LOGE("HandleErrorEvent: Event [type(%d), response type(%d)] received.", aEvent.GetEventType(), aEvent.GetResponseType());
    handleCommandFailure();
}


void MetadataDriver::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    LOGV("HandleInformationalEvent: Event [type(%d), response type(%d)] received.", aEvent.GetEventType(), aEvent.GetResponseType());
}


//------------------------------------------------------------------------------
#include <media/PVMetadataRetriever.h>

namespace android {

//#define LOG_NDEBUG 0
//#define LOG_TAG "PVMetadataRetriever"

// A concrete subclass of MediaMetadataRetrieverInterface implementation
// Use the MetadataDriver object as a delegate and forward related calls
// to the MetadataDriver object.
PVMetadataRetriever::PVMetadataRetriever()
{
    LOGV("constructor");
    mMetadataDriver = new MetadataDriver();
}

PVMetadataRetriever::~PVMetadataRetriever()
{
    LOGV("destructor");
    Mutex::Autolock lock(mLock);
    delete mMetadataDriver;
}

status_t PVMetadataRetriever::setDataSource(const char *url)
{
    LOGV("setDataSource (%s)", url);
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return INVALID_OPERATION;
    }
    if (url == 0) {
        LOGE("Null pointer is passed as argument");
        return INVALID_OPERATION;
    }
    return mMetadataDriver->setDataSource(url);
}

status_t PVMetadataRetriever::setDataSource(int fd, int64_t offset, int64_t length)
{
    LOGV("setDataSource fd(%d), offset(%lld), length(%lld)", fd, offset, length);
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return INVALID_OPERATION;
    }
    if (offset < 0 || length < 0) {
        if (offset < 0) {
            LOGE("negative offset (%lld)", offset);
        }
        if (length < 0) {
            LOGE("negative length (%lld)", length);
        }
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t PVMetadataRetriever::setMode(int mode)
{
    LOGV("setMode (%d)", mode);
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return INVALID_OPERATION;
    }
    if (mode < 0x00 || mode > 0x03) {
        LOGE("set to invalid mode (%d)", mode);
        return INVALID_OPERATION;
    }
    return mMetadataDriver->setMode(mode);
}

status_t PVMetadataRetriever::getMode(int* mode) const
{
    LOGV("getMode");
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return INVALID_OPERATION;
    }
    if (mode == 0) {
        LOGE("Null pointer is passed as argument");
        return INVALID_OPERATION;
    }
    return mMetadataDriver->getMode(mode);
}

VideoFrame *PVMetadataRetriever::captureFrame()
{
    LOGV("captureFrame");
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return NULL;
    }
    return mMetadataDriver->captureFrame();
}

MediaAlbumArt *PVMetadataRetriever::extractAlbumArt()
{
    LOGV("extractAlbumArt");
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return NULL;
    }
    return mMetadataDriver->extractAlbumArt();
}

const char* PVMetadataRetriever::extractMetadata(int keyCode)
{
    LOGV("extractMetadata");
    Mutex::Autolock lock(mLock);
    if (mMetadataDriver == 0) {
        LOGE("No MetadataDriver available");
        return NULL;
    }
    return mMetadataDriver->extractMetadata(keyCode);
}

};  // android
