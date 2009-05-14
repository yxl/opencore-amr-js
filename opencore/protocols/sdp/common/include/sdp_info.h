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
#ifndef __SDP_INFO_H__
#define __SDP_INFO_H__

#include "oscl_mem.h"
#include "session_info.h"
#include "media_info.h"
#include "aac_media_info.h"
#include "amr_media_info.h"
#include "m4v_media_info.h"
#include "rfc3640_media_info.h"
#include "h263_media_info.h"
#include "still_image_media_info.h"
#include "bool_array.h"
#include "pcmu_media_info.h"
#include "pcma_media_info.h"

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif



//----------------------------------------------------------------------
// Global Constant Declarations
//----------------------------------------------------------------------
#define MAX_MEDIA_OBJECTS	50
#define MAX_SEGMENTS		10

struct segmentSpecific
{
    bool segmentActive;
    bool segmentPayloadOrderPref;
};

typedef BoolArray<MAX_MEDIA_OBJECTS> SDPSelectionType;
//----------------------------------------------------------------------
// Global Type Declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Data Declarations
//----------------------------------------------------------------------

//======================================================================
//  CLASS DEFINITIONS and FUNCTION DECLARATIONS
//======================================================================
#include "mime_registry.h"

class SDPInfo
{
    public:
        OSCL_IMPORT_REF
        SDPInfo();
        OSCL_IMPORT_REF
        SDPInfo(const SDPInfo &sourceSdpInfo);
        OSCL_IMPORT_REF
        ~SDPInfo();
        OSCL_IMPORT_REF
        void freeLastMediaInfoObject(void);
        OSCL_IMPORT_REF
        sessionDescription *getSessionInfo();
        OSCL_IMPORT_REF
        Oscl_Vector<mediaInfo *, SDPParserAlloc> getMediaInfo(int Object);
        OSCL_IMPORT_REF
        Oscl_Vector<mediaInfo *, SDPParserAlloc> *getMediaArray();
        OSCL_IMPORT_REF
        int getNumMediaObjects();
        OSCL_IMPORT_REF
        void dealloc(void *ptr);
        OSCL_IMPORT_REF
        mediaInfo* getMediaInfoBasedOnID(uint32 trackID);
        OSCL_IMPORT_REF
        mediaInfo* getMediaInfoBasedOnDependsOnID(uint32 trackID);
        OSCL_IMPORT_REF
        void *alloc(const int size, bool alternateMedia);
        OSCL_IMPORT_REF
        void IncrementAlternateMediaInfoVectorIndex();
        OSCL_IMPORT_REF
        void copyFmDefMedia(mediaInfo *media);
        OSCL_IMPORT_REF
        void reset();
        OSCL_IMPORT_REF
        uint32 getMediaObjectIndex();
        OSCL_IMPORT_REF
        bool getMediaInfoInSegment(int segment, Oscl_Vector< mediaInfo *, SDPParserAlloc>& segmentMediaInfo);
        OSCL_IMPORT_REF
        void setSegmentCount(int count);
        OSCL_IMPORT_REF
        int getSegmentCount();
        OSCL_IMPORT_REF
        bool setSegmentActive(int segment, bool status);
        OSCL_IMPORT_REF
        bool isSegmentActive(int segment);
        OSCL_IMPORT_REF
        bool setSegmentPayloadOrderPref(int segment, int* payloadArray, int len);
        OSCL_IMPORT_REF
        bool isPayloadOrderPreferenceSet(int segment);
        OSCL_IMPORT_REF
        mediaInfo* getPreferedMediaInfo(int segment);
        OSCL_IMPORT_REF
        int getPreferedPayloadNumber(int segment);
        OSCL_IMPORT_REF
        const oscl_wchar *getSdpFilename(uint32 &retsize);
        OSCL_IMPORT_REF
        void setSDPFilename(OSCL_wString& aURL);
    private:
        sessionDescription session_info;
        Oscl_Vector< mediaInfo *, SDPParserAlloc> pMediaInfo[MAX_MEDIA_OBJECTS];
        int numMediaObjects;
        uint32 iMediaObjectIndex;
        int segmentCount;
        segmentSpecific segmentInfo[MAX_SEGMENTS];
        OSCL_wHeapString<SDPParserAlloc> SdpFilename;
};
#endif // __SDP_INFO_H__

