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

#ifndef ANDROID_METADATADRIVER_H
#define ANDROID_METADATADRIVER_H

#include <media/mediametadataretriever.h>

#include "pv_frame_metadata_interface.h"
#include "pv_frame_metadata_factory.h"
#include "pv_engine_observer.h"
#include "pv_player_datasourceurl.h"
#include "pvmi_kvp_util.h"
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "oscl_snprintf.h"

#include "oscl_scheduler.h"
#include "oscl_utf8conv.h"
#include "oscl_scheduler_ao.h"
#include "cczoomrotation16.h"  // for color converter
#include "OMX_Core.h"
#include "pv_omxcore.h"

#define BEST_THUMBNAIL_MODE 1

#if BEST_THUMBNAIL_MODE
#include "pvmf_local_data_source.h"
#endif

namespace android {

class VideoFrame;
class MediaAlbumArt;

class MetadataDriver:
public OsclActiveObject,
public PVCommandStatusObserver,
public PVInformationalEventObserver,
public PVErrorEventObserver
{
public:
    // @param mode The intended mode of operations:
    // 1. 0x00: Experimental - just add and remove data source.
    // 2. 0x01: For capture frame/thumbnail only.
    // 3. 0x02: For meta data retrieval only.
    // 4. 0x03: For both frame capture and meta data retrieval.
    // Only the LSB 2 bits of argument mode are checked.
    explicit MetadataDriver(uint32 mode = 0x03);
    ~MetadataDriver();

    // Call this before setDataSource() so that the intended mode of
    // operation becomes effective.
    status_t setMode(int mode) {
        if (mode < 0 || mode > 3) {
            return BAD_VALUE;
        }
        mMode = mode;
        return NO_ERROR;
    }

    // Returns the current mode of operation.
    status_t getMode(int* mode) const { *mode = mMode; return NO_ERROR; }

    // This call may be time consuming.
    // Returns OK if no operation failed; otherwise, it returns UNKNOWN_ERROR.
    status_t setDataSource(const char* srcUrl);

    // Captures a representative frame. Returns NULL if failure.
    VideoFrame *captureFrame();

    // Returns the metadata value if it exists; return NULL if the metadata
    // value does not exit or if operation failure occurs.
    const char* extractMetadata(int keyCode);

    // Returns the optional graphic embedded/stored within the given data
    // source. Returns NULL if no graphic information is found.
    MediaAlbumArt* extractAlbumArt();

    // These callback handlers implement PVCommandStatusObserver,
    // PVInformationalEventObserver, PVErrorEventObserver, and OsclTimerObject.
    // They should never be called directly, although the access modifier
    // is public.
    void Run();
    void CommandCompleted(const PVCmdResponse& aResponse);
    void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
    void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

private:
    // Internal states. The existence of the optional states depend on the
    // value of mMode.
    enum MetadataDriverState {
        STATE_CREATE,
        STATE_ADD_DATA_SOURCE,
        STATE_GET_METADATA_KEYS,        // Optional.
        STATE_GET_METADATA_VALUES,      // Depends on STATE_GET_METADATA_KEYS.
        STATE_GET_FRAME,                // Optional.
        STATE_REMOVE_DATA_SOURCE,
        STATE_CLEANUP_AND_COMPLETE,
    };

    // We support get metadata, or get frame, or get both, or get neigther.
    static const uint32 GET_METADATA_ONLY    = (0x01 << 0);
    static const uint32 GET_FRAME_ONLY       = (0x01 << 1);
    static const uint32 MAX_VIDEO_FRAME_SIZE = 640 * 480 * 4;  // Big enough?
    static const uint32 MAX_METADATA_STRING_LENGTH = 128;
    static const uint32 MAX_STRING_LENGTH = 512;
    static const uint32 NUM_METADATA_KEYS = 21;
    static const char* METADATA_KEYS[NUM_METADATA_KEYS];
    static const char* ALBUM_ART_KEY;

    status_t doSetDataSource(const char* srcUrl);
    status_t doExtractAlbumArt();
    status_t extractExternalAlbumArt(const char* url);
    status_t extractEmbeddedAlbumArt(const PvmfApicStruct* apic);
    void doColorConversion();
    void trimKeys();
    bool containsSupportedKey(const OSCL_HeapString<OsclMemAllocator>& str) const;
    bool isCommandSuccessful(const PVCmdResponse& aResponse) const;
    void handleCommandFailure();
    void handleCreate();
    void handleCleanUp();
    void handleAddDataSource();
    void handleRemoveDataSource();
    void handleGetMetadataKeys();
    void handleGetMetadataValues();
    void handleGetFrame();
    void cacheMetadataRetrievalResults();
    void clearCache();
    status_t extractMetadata(const char* key, char* value, uint32 valueLength);
    static int startDriverThread(void *cookie);
    int retrieverThread();

    OsclSemaphore* mSyncSem;

    uint32 mMode;
    MetadataDriverState mState;
    PVCommandId mCmdId;
    bool mIsSetDataSourceSuccessful;
    uint32 mContextObject;
    uint32 mContextObjectRefValue;
    PVFrameAndMetadataInterface *mUtil;

    // Required for setting/removing data source
    PVPlayerDataSourceURL *mDataSource;
#if BEST_THUMBNAIL_MODE
    PVMFLocalDataSource* mLocalDataSource;
#endif
    OSCL_wHeapString<OsclMemAllocator> mDataSourceUrl;

    // Required for frame retrieval
    PVFrameBufferProperty mFrameBufferProp;
    PVFrameSelector mFrameSelector;
    uint32 mFrameBufferSize;
    uint8 mFrameBuffer[MAX_VIDEO_FRAME_SIZE];
    VideoFrame* mVideoFrame;

    // Required for meta data retrieval
    Oscl_Vector<PvmiKvp,OsclMemAllocator> mMetadataValueList;
    PVPMetadataList mMetadataKeyList;
    PVPMetadataList mActualMetadataKeyList;
    int32 mNumMetadataValues;

    // Keep all these copies because of bug 1201885, otherwise, we can
    // get these out of mMetadataValueList
    char mMetadataValues[NUM_METADATA_KEYS][MAX_METADATA_STRING_LENGTH];
    MediaAlbumArt *mMediaAlbumArt;
};

}; // namespace android

#endif // ANDROID_METADATADRIVER_H
