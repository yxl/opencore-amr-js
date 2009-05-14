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
#include "sdp_info.h"
#endif

#ifndef SDPPARSER_CONFIG_H_INCLUDED
#include "sdpparser_config.h"
#endif

#ifndef SDPPARSER_HAS_REAL_MEDIA_SUPPORT
#error "sdp config file missing"
#endif

#if SDPPARSER_HAS_REAL_MEDIA_SUPPORT
#ifndef RM_MEDIAINFO_H
#include "rm_media_info.h"
#endif
#endif


OSCL_EXPORT_REF
SDPInfo::SDPInfo()
{
    numMediaObjects = 0;
    iMediaObjectIndex = 0;
    segmentCount = 0;
    for (int ii = 0; ii < MAX_SEGMENTS; ii++)
    {
        segmentInfo[ii].segmentActive = true;
        segmentInfo[ii].segmentPayloadOrderPref = false;
    }
}

OSCL_EXPORT_REF
SDPInfo::SDPInfo(const SDPInfo &sourceSdpInfo)
{
    //expecting the = oper of sessionDescription to be overloaded
    session_info = sourceSdpInfo.session_info;
    SdpFilename = sourceSdpInfo.SdpFilename;
    segmentCount = sourceSdpInfo.segmentCount;
    iMediaObjectIndex = sourceSdpInfo.iMediaObjectIndex;
    numMediaObjects = sourceSdpInfo.numMediaObjects;

    for (int jj = 0; jj < MAX_SEGMENTS; jj++)
    {
        segmentInfo[jj] = sourceSdpInfo.segmentInfo[jj];
    }

    for (int ii = 0; ii < MAX_MEDIA_OBJECTS; ii++)
    {
        Oscl_Vector< mediaInfo *, SDPParserAlloc>& destMediaInfoVect = pMediaInfo[ii];
        const Oscl_Vector< mediaInfo *, SDPParserAlloc>& srcMediaInfoVect = sourceSdpInfo.pMediaInfo[ii];
        const int32 srcMediaInfoVectSz = srcMediaInfoVect.size();
        for (int ss = 0; ss < srcMediaInfoVectSz; ss++)
        {
            mediaInfo* destMediaInfo = NULL;
            const char *mimeType = srcMediaInfoVect[ss]->getMIMEType();
            bool alternateMedia = (0 == ss) ? false : true;

            if (!oscl_strncmp(mimeType, "AAC", oscl_strlen("AAC")) || !oscl_strncmp(mimeType, "MP4A-LATM", oscl_strlen("MP4A-LATM")))
            {
                aac_mediaInfo *pSourceAAC = OSCL_REINTERPRET_CAST(aac_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(aac_mediaInfo), alternateMedia);
                aac_mediaInfo *pAAC = OSCL_PLACEMENT_NEW(memory, aac_mediaInfo());
                *pAAC = *pSourceAAC;
                destMediaInfo = pAAC;
            }
            else if (!oscl_strncmp(mimeType, "AMR", oscl_strlen("AMR")))
            {
                amr_mediaInfo *pSourceAMR = OSCL_REINTERPRET_CAST(amr_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(amr_mediaInfo), alternateMedia);
                amr_mediaInfo *pAMR = OSCL_PLACEMENT_NEW(memory, amr_mediaInfo());
                *pAMR = *pSourceAMR;
                destMediaInfo = pAMR;
            }
            else if (!oscl_strncmp(mimeType, "MP4V-ES", oscl_strlen("MP4V-ES")))
            {
                m4v_mediaInfo *pSourceM4V = OSCL_REINTERPRET_CAST(m4v_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(m4v_mediaInfo), alternateMedia);
                m4v_mediaInfo *pM4V = OSCL_PLACEMENT_NEW(memory, m4v_mediaInfo());
                *pM4V = *pSourceM4V;
                destMediaInfo = pM4V;
            }
            else if (!oscl_strncmp(mimeType, "H263-1998", oscl_strlen("H263-1998")) || !oscl_strncmp(mimeType, "H263-2000", oscl_strlen("H263-2000")))
            {
                h263_mediaInfo *pSourceH263 = OSCL_REINTERPRET_CAST(h263_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(h263_mediaInfo), alternateMedia);
                h263_mediaInfo *pH263 = OSCL_PLACEMENT_NEW(memory, h263_mediaInfo());
                *pH263 = *pSourceH263;
                destMediaInfo = pH263;
            }
            else if (!oscl_strncmp(mimeType, "PVMP4V-ES", oscl_strlen("PVMP4V-ES")))
            {
                m4v_mediaInfo *pSourceM4V = OSCL_REINTERPRET_CAST(m4v_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(m4v_mediaInfo), alternateMedia);
                m4v_mediaInfo *pM4V = OSCL_PLACEMENT_NEW(memory, m4v_mediaInfo());
                *pM4V = *pSourceM4V;
                destMediaInfo = pM4V;
            }
            else if (!oscl_strncmp(mimeType, "mpeg4-generic", oscl_strlen("mpeg4-generic")))
            {
                rfc3640_mediaInfo *pSourceRFC3640 = OSCL_REINTERPRET_CAST(rfc3640_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(rfc3640_mediaInfo), alternateMedia);
                rfc3640_mediaInfo *pRFC3640 = OSCL_PLACEMENT_NEW(memory, rfc3640_mediaInfo());
                *pRFC3640 = *pSourceRFC3640;
                destMediaInfo = pRFC3640;
            }
            else if (!oscl_strncmp(mimeType, "X-MP4V-IMAGE", oscl_strlen("X-MP4V-IMAGE")))
            {
                still_image_mediaInfo *pSourceImage = OSCL_REINTERPRET_CAST(still_image_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(still_image_mediaInfo), alternateMedia);
                still_image_mediaInfo *pImage = OSCL_PLACEMENT_NEW(memory, still_image_mediaInfo());
                *pImage = *pSourceImage;
                destMediaInfo = pImage;
            }
#if SDPPARSER_HAS_REAL_MEDIA_SUPPORT
            else if (!oscl_strncmp(mimeType, "x-pn-realaudio", oscl_strlen("x-pn-realaudio")) || !oscl_strncmp(mimeType, "x-pn-realvideo", oscl_strlen("x-pn-realvideo")))
            {
                rm_mediaInfo *rmSource = OSCL_REINTERPRET_CAST(rm_mediaInfo*, srcMediaInfoVect[ss]);
                void *memory = alloc(sizeof(rm_mediaInfo), alternateMedia);
                rm_mediaInfo *prm = OSCL_PLACEMENT_NEW(memory, rm_mediaInfo());
                *prm = *rmSource;
                destMediaInfo = prm;
            }
#endif
            if (destMediaInfo)
                destMediaInfoVect.push_back(destMediaInfo);
        }
    }
}


OSCL_EXPORT_REF
SDPInfo::~SDPInfo()
{
    int ii = 0;
    int ss = 0;
    for (ii = 0; ii < numMediaObjects; ii++)
    {
        for (ss = 0; ss < (int)pMediaInfo[ii].size();ss++)
        {
            pMediaInfo[ii][ss]->~mediaInfo();
            dealloc(pMediaInfo[ii][ss]);
        }
    }
    numMediaObjects = 0;

}

OSCL_EXPORT_REF
void SDPInfo::freeLastMediaInfoObject(void)
{
    if (numMediaObjects < 0)
    {
        return;
    }

    for (int ss = 0; ss < (int)pMediaInfo[numMediaObjects].size();ss++)
    {
        pMediaInfo[numMediaObjects][ss]->~mediaInfo();
        dealloc(pMediaInfo[numMediaObjects][ss]);
    }
}

OSCL_EXPORT_REF
sessionDescription* SDPInfo::getSessionInfo()
{
    return &session_info;
}

OSCL_EXPORT_REF
Oscl_Vector<mediaInfo *, SDPParserAlloc> SDPInfo::getMediaInfo(int Object)
{
    if ((Object >= 0) && (Object < MAX_MEDIA_OBJECTS))
    {
        return pMediaInfo[Object];
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF
Oscl_Vector<mediaInfo *, SDPParserAlloc> * SDPInfo::getMediaArray()
{
    return pMediaInfo;
}

OSCL_EXPORT_REF
int SDPInfo::getNumMediaObjects()
{
    return numMediaObjects;
}

OSCL_EXPORT_REF
void SDPInfo::dealloc(void *ptr)
{
    oscl_free(ptr);
}

OSCL_EXPORT_REF
mediaInfo* SDPInfo::getMediaInfoBasedOnID(uint32 trackID)
{
    int numObjects = getNumMediaObjects();

    for (int i = 0; i < numObjects; i++)
    {
        Oscl_Vector<mediaInfo*, SDPParserAlloc> mediaInfoVec;
        mediaInfoVec = getMediaInfo(i);

        for (uint32 j = 0; j < mediaInfoVec.size(); j++)
        {
            mediaInfo* minfo = mediaInfoVec[j];

            if (minfo != NULL)
            {
                if (minfo->getMediaInfoID() == trackID)
                {
                    return minfo;
                }
            }
        }
    }
    return NULL;
}

OSCL_EXPORT_REF
mediaInfo* SDPInfo::getMediaInfoBasedOnDependsOnID(uint32 trackID)
{
    if (trackID == 0)
    {
        return NULL;
    }

    int numObjects = getNumMediaObjects();

    for (int i = 0; i < numObjects; i++)
    {
        Oscl_Vector<mediaInfo*, SDPParserAlloc> mediaInfoVec;
        mediaInfoVec = getMediaInfo(i);

        for (uint32 j = 0; j < mediaInfoVec.size(); j++)
        {
            mediaInfo* minfo = mediaInfoVec[j];

            if (minfo != NULL)
            {
                if ((uint32)(minfo->getControlTrackID()) == trackID)
                {
                    return minfo;
                }
            }
        }
    }
    return NULL;
}

OSCL_EXPORT_REF
void * SDPInfo::alloc(const int size, bool alternateMedia)
{
    OSCL_UNUSED_ARG(alternateMedia);

    if (numMediaObjects < MAX_MEDIA_OBJECTS)
    {
        void *mem = oscl_malloc(size * sizeof(char));
        if (mem != NULL)
        {
            iMediaObjectIndex++;
            pMediaInfo[numMediaObjects].push_back((mediaInfo *)mem);
        }
        return mem;
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF
void SDPInfo::IncrementAlternateMediaInfoVectorIndex()
{
    pMediaInfo[numMediaObjects][0]->setSegmentNumber(segmentCount);
    numMediaObjects++;
}

OSCL_EXPORT_REF
void SDPInfo::copyFmDefMedia(mediaInfo *media)
{
    *media = *pMediaInfo[numMediaObjects][0];
}

OSCL_EXPORT_REF
void SDPInfo::reset()
{
    session_info.resetSessionDescription();
    int ii = 0;
    int ss = 0;
    for (ii = 0; ii < numMediaObjects; ii++)
    {
        for (ss = 0; ss < (int)pMediaInfo[ii].size(); ss++)
        {
            pMediaInfo[ii][ss]->~mediaInfo();
            dealloc(pMediaInfo[ii][ss]);
        }
    }
    numMediaObjects = 0;
}

OSCL_EXPORT_REF
uint32 SDPInfo::getMediaObjectIndex()
{
    return iMediaObjectIndex;
}

OSCL_EXPORT_REF
bool SDPInfo::getMediaInfoInSegment(int segment, Oscl_Vector< mediaInfo *, SDPParserAlloc>& segmentMediaInfo)
{
    if (segmentCount == 0)
        return false;
    for (int ii = 0; ii < numMediaObjects; ii++)
    {
        if (pMediaInfo[ii][0]->getSegmentNumber() == (uint)segment)
        {
            segmentMediaInfo.push_back(pMediaInfo[ii][0]);
        }
    }
    return true;
}

OSCL_EXPORT_REF
void SDPInfo::setSegmentCount(int count)
{
    segmentCount = count;
}

OSCL_EXPORT_REF
int SDPInfo::getSegmentCount()
{
    return segmentCount;
}

OSCL_EXPORT_REF
bool SDPInfo::setSegmentActive(int segment, bool status)
{
    if (segment <= segmentCount)
    {
        segmentInfo[segment].segmentActive = status;
        return true;
    }
    else
        return false;
}

OSCL_EXPORT_REF
bool SDPInfo::isSegmentActive(int segment)
{
    if (segment <= segmentCount)
        return segmentInfo[segment].segmentActive;
    else
        return false;
}

OSCL_EXPORT_REF
bool SDPInfo::setSegmentPayloadOrderPref(int segment, int* payloadArray, int len)
{
    segmentInfo[segment].segmentPayloadOrderPref = false;
    if (segment <= segmentCount)
    {
        Oscl_Vector< mediaInfo *, SDPParserAlloc> segmentMediaInfo;
        if (getMediaInfoInSegment(segment, segmentMediaInfo) == true)
        {
            for (int ii = 0; ii < len; ii++)
            {
                for (uint32 jj = 0; jj < segmentMediaInfo.size(); jj++)
                {
                    if (segmentMediaInfo[jj]->getPayloadSpecificInfoVector()[0]->getPayloadNumber() == (uint32)payloadArray[ii])
                    {
                        segmentMediaInfo[jj]->setPayloadPreference(ii);
                        break;
                    }
                }
            }
        }
    }
    else
        return false;
    segmentInfo[segment].segmentPayloadOrderPref = true;
    return true;
}

OSCL_EXPORT_REF
bool SDPInfo::isPayloadOrderPreferenceSet(int segment)
{
    if (segment <= segmentCount)
        return segmentInfo[segment].segmentPayloadOrderPref;
    else
        return false;
}

OSCL_EXPORT_REF
mediaInfo* SDPInfo::getPreferedMediaInfo(int segment)
{
    mediaInfo* media = NULL;
    Oscl_Vector< mediaInfo *, SDPParserAlloc> segmentMediaInfo;
    if (getMediaInfoInSegment(segment, segmentMediaInfo) == true)
    {
        for (uint32 jj = 0; jj < segmentMediaInfo.size(); jj++)
        {
            if (segmentMediaInfo[jj]->isMatched() == true)
            {
                media = segmentMediaInfo[jj];
                break;
            }
        }
    }
    return media;
}

OSCL_EXPORT_REF
int SDPInfo::getPreferedPayloadNumber(int segment)
{
    int payload = -1;
    Oscl_Vector< mediaInfo *, SDPParserAlloc> segmentMediaInfo;
    if (getMediaInfoInSegment(segment, segmentMediaInfo) == true)
    {
        for (uint32 jj = 0; jj < segmentMediaInfo.size(); jj++)
        {
            if (segmentMediaInfo[jj]->isMatched() == true)
            {
                payload = segmentMediaInfo[jj]->getPayloadSpecificInfoVector()[0]->getPayloadNumber();
                break;
            }
        }
    }
    return payload;
}

OSCL_EXPORT_REF
const oscl_wchar * SDPInfo::getSdpFilename(uint32 &retsize)
{
    retsize = SdpFilename.get_size();
    return SdpFilename.get_cstr();
}

OSCL_EXPORT_REF
void SDPInfo::setSDPFilename(OSCL_wString& aURL)
{
    SdpFilename = aURL;
}
