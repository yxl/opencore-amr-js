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
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#define PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

enum PVMF_TRACK_INFO_TRACK_ALTERNATE_TYPE
{
    PVMF_TRACK_ALTERNATE_TYPE_UNDEFINED,
    PVMF_TRACK_ALTERNATE_TYPE_LANGUAGE,
    PVMF_TRACK_ALTERNATE_TYPE_BANDWIDTH,
    PVMF_TRACK_ALTERNATE_TYPE_BANDWIDTH_AND_CODEC_PARAMS,
};

class PVMFTrackInfo : public HeapBase
{
    public:
        PVMFTrackInfo()
        {
            iTrackID = -1;
            iPortTag = -1;
            iDurationValue = 0;
            iDurationTimeScale = 0;
            iDependsOn = false;
            iHasAlternates = false;
            iTrackAlternateType = PVMF_TRACK_ALTERNATE_TYPE_UNDEFINED;
            oDurationAvailable = true;
            iTrackFrameRate = 0;
        }

        PVMFTrackInfo(const PVMFTrackInfo& aSourceTrackInfo) : HeapBase(aSourceTrackInfo)
        {
            iTrackMimeType       = aSourceTrackInfo.iTrackMimeType;
            iTrackID             = aSourceTrackInfo.iTrackID;
            iPortTag             = aSourceTrackInfo.iPortTag;
            iDurationValue       = aSourceTrackInfo.iDurationValue;
            iDurationTimeScale   = aSourceTrackInfo.iDurationTimeScale;
            iTrackBitRate        = aSourceTrackInfo.iTrackBitRate;
            iTrackFrameRate		 = aSourceTrackInfo.iTrackFrameRate;
            iTrackConfigInfo     = aSourceTrackInfo.iTrackConfigInfo;
            iDependsOn           = aSourceTrackInfo.iDependsOn;
            iHasAlternates       = aSourceTrackInfo.iHasAlternates;
            iTrackAlternateType  = aSourceTrackInfo.iTrackAlternateType;
            iDependsOnTrackIDVec = aSourceTrackInfo.iDependsOnTrackIDVec;
            iAlternateTrackIDVec = aSourceTrackInfo.iAlternateTrackIDVec;
            oDurationAvailable   = aSourceTrackInfo.oDurationAvailable;
            iLanguage            = aSourceTrackInfo.iLanguage;
        }

        PVMFTrackInfo& operator=(const PVMFTrackInfo& aSourceTrackInfo)
        {
            if (&aSourceTrackInfo != this)
            {
                iTrackMimeType       = aSourceTrackInfo.iTrackMimeType;
                iTrackID             = aSourceTrackInfo.iTrackID;
                iPortTag             = aSourceTrackInfo.iPortTag;
                iDurationValue       = aSourceTrackInfo.iDurationValue;
                iDurationTimeScale   = aSourceTrackInfo.iDurationTimeScale;
                iTrackBitRate        = aSourceTrackInfo.iTrackBitRate;
                iTrackFrameRate      = aSourceTrackInfo.iTrackFrameRate;
                iTrackConfigInfo     = aSourceTrackInfo.iTrackConfigInfo;
                iDependsOn           = aSourceTrackInfo.iDependsOn;
                iHasAlternates       = aSourceTrackInfo.iHasAlternates;
                iTrackAlternateType  = aSourceTrackInfo.iTrackAlternateType;
                iDependsOnTrackIDVec = aSourceTrackInfo.iDependsOnTrackIDVec;
                iAlternateTrackIDVec = aSourceTrackInfo.iAlternateTrackIDVec;
                oDurationAvailable   = aSourceTrackInfo.oDurationAvailable;
                iLanguage            = aSourceTrackInfo.iLanguage;
            }
            return *this;
        }

        ~PVMFTrackInfo()
        {
        }

        /*
         * Track mimetype is a mimestring that uniquely identifies
         * a media track and its format to some extent, within a
         * presentation.
         */
        OSCL_String& getTrackMimeType()
        {
            return iTrackMimeType;
        }
        void setTrackMimeType(OSCL_String& aMimeType)
        {
            iTrackMimeType = aMimeType;
        }

        uint32 getTrackID()
        {
            return iTrackID;
        }
        void setTrackID(uint32 aId)
        {
            iTrackID = aId;
        }

        uint32 getPortTag()
        {
            return iPortTag;
        }
        void setPortTag(uint32 aTag)
        {
            iPortTag = aTag;
        }

        bool IsDurationAvailable()
        {
            return oDurationAvailable;
        }

        void SetDurationAvailable(bool aDurationAvailable)
        {
            oDurationAvailable = aDurationAvailable;
        }

        uint64 getTrackDurationValue()
        {
            return iDurationValue;
        }
        void setTrackDurationValue(uint64 aDuration)
        {
            iDurationValue = aDuration;
        }

        uint64  getTrackDurationTimeScale()
        {
            return iDurationTimeScale;
        }
        void setTrackDurationTimeScale(uint64  aTimeScale)
        {
            iDurationTimeScale = aTimeScale;
        }

        uint32 getTrackBitRate()
        {
            return iTrackBitRate;
        }
        void setTrackBitRate(uint32 aBitRate)
        {
            iTrackBitRate = aBitRate;
        }

        uint32 getTrackFrameRate()
        {
            return iTrackFrameRate;
        }
        void setTrackFrameRate(uint32 aFrameRate)
        {
            iTrackFrameRate = aFrameRate;
        }

        /*
         * TrackConfig info typically holds info that gets passed to
         * decoders to initialize, aka, mpeg4 vol header, mpeg4 audio
         * config etc. For tracks that expose a fixed set of parameters
         * as part of config as opposed to some opaque data, the void
         * pointer in the memory fragement is actually a pointer to a
         * strcuture that the user of this field would know how to
         * interpret. For example in case of H263, we could have a structure
         * that could be defined as:
         * struct H263ConfigInfo
         * {
         *      uint32 maxWidth;
         *      uint32 maxHeight;
         *      uint32 codecProfile;
         *      uint32 codecLevel;
         * };
         * Memory fragment holds a pointer to this strcuture.
         *
         */
        OsclRefCounterMemFrag& getTrackConfigInfo()
        {
            return iTrackConfigInfo;
        }
        void setTrackConfigInfo(OsclRefCounterMemFrag& aConfig)
        {
            iTrackConfigInfo = aConfig;
        }

        void setDependsOn()
        {
            iDependsOn = true;
        }

        bool DoesTrackHaveDependency()
        {
            return (iDependsOn);
        }

        void addDependsOnTrackID(int32 aTrackID)
        {
            iDependsOnTrackIDVec.push_back(aTrackID);
        }

        Oscl_Vector<int32, OsclMemAllocator> getDependsOnTrackIDVec()
        {
            return (iDependsOnTrackIDVec);
        }

        void setTrackAlternates(PVMF_TRACK_INFO_TRACK_ALTERNATE_TYPE aAlternateType)
        {
            if (aAlternateType != PVMF_TRACK_ALTERNATE_TYPE_UNDEFINED)
            {
                iHasAlternates = true;
                iTrackAlternateType = aAlternateType;
            }
        }

        bool DoesTrackHaveAlternates()
        {
            return (iHasAlternates);
        }

        PVMF_TRACK_INFO_TRACK_ALTERNATE_TYPE getTrackAlternateType()
        {
            return iTrackAlternateType;
        }

        void addAlternateTrackID(int32 aTrackID)
        {
            iAlternateTrackIDVec.push_back(aTrackID);
        }

        Oscl_Vector<int32, OsclMemAllocator> getAlternateTrackIDVec()
        {
            return (iAlternateTrackIDVec);
        }

        OSCL_String& getTrackLanguage()
        {
            return iLanguage;
        }

    private:
        OSCL_HeapString<OsclMemAllocator> iTrackMimeType;
        int32 iTrackID;
        int32 iPortTag;
        uint64  iDurationValue;
        uint64  iDurationTimeScale;
        uint32 iTrackBitRate;
        uint32 iTrackFrameRate;
        OsclRefCounterMemFrag iTrackConfigInfo;
        bool iDependsOn;
        Oscl_Vector<int32, OsclMemAllocator> iDependsOnTrackIDVec;
        bool iHasAlternates;
        PVMF_TRACK_INFO_TRACK_ALTERNATE_TYPE iTrackAlternateType;
        Oscl_Vector<int32, OsclMemAllocator> iAlternateTrackIDVec;
        bool oDurationAvailable;
        /*
         * ISO-639-2/T language codes, which are basically 3 character codes
         * assigned to each language. Typically applicable to audio tracks.
         * If the language code exists it will be returned in the iLanguage parameter.
         * If iLanguage is empty English should be assumed.
         */
        OSCL_HeapString<OsclMemAllocator> iLanguage;
};


/**
 * Primary goals of this class are to provide:
 * - a common interface for the track selection modules, across
 *   local playback, streaming and download
 * - provide enough information to perform track selection
 */
class PVMFMediaPresentationInfo
{
    public:
        PVMFMediaPresentationInfo()
        {
            oIsSeekable = false;
            oDurationAvailable = true;
        }

        PVMFMediaPresentationInfo(const PVMFMediaPresentationInfo& aInfo)
        {
            iPresentationType = aInfo.iPresentationType;
            iDurationValue = aInfo.iDurationValue;
            iDurationTimeScale = aInfo.iDurationTimeScale;
            oIsSeekable = aInfo.oIsSeekable;
            iTrackInfoVec = aInfo.iTrackInfoVec;
            oDurationAvailable = aInfo.oDurationAvailable;
        }

        ~PVMFMediaPresentationInfo()
        {
            iTrackInfoVec.clear();
        }

        PVMFMediaPresentationInfo& operator=(const PVMFMediaPresentationInfo& aInfo)
        {
            if (&aInfo != this)
            {
                iPresentationType = aInfo.iPresentationType;
                iDurationValue = aInfo.iDurationValue;
                iDurationTimeScale = aInfo.iDurationTimeScale;
                oIsSeekable = aInfo.oIsSeekable;
                iTrackInfoVec = aInfo.iTrackInfoVec;
                oDurationAvailable = aInfo.oDurationAvailable;
            }
            return *this;
        }

        /*
         * Presentation type is a mime string that identifies the
         * media presentation, viz, mp4, asf, mp3 etc
         */
        OSCL_String& getPresentationType()
        {
            return iPresentationType;
        }

        void setPresentationType(OSCL_String& aPresentationType)
        {
            iPresentationType = aPresentationType;
        }

        bool IsSeekable()
        {
            return oIsSeekable;
        }

        void setSeekableFlag(bool aFlag)
        {
            oIsSeekable = aFlag;
        }

        void addTrackInfo(PVMFTrackInfo& aTrackInfo)
        {
            iTrackInfoVec.push_back(aTrackInfo);
        }

        uint32 getNumTracks() const
        {
            return (iTrackInfoVec.size());
        }

        PVMFTrackInfo* getTrackInfo(uint32 aIndex) const
        {
            if (aIndex >= iTrackInfoVec.size())
            {
                return NULL;
            }
            return (OSCL_CONST_CAST(PVMFTrackInfo*, &(iTrackInfoVec[aIndex])));
        }

        PVMFTrackInfo* getTrackInfoBasedOnTrackID(uint32 aTrackID)
        {
            Oscl_Vector<PVMFTrackInfo, OsclMemAllocator>::iterator it;
            for (it = iTrackInfoVec.begin(); it != iTrackInfoVec.end(); it++)
            {
                if (it->getTrackID() == aTrackID)
                {
                    return (it);
                }
            }
            return NULL;
        }

        bool IsDurationAvailable()
        {
            return oDurationAvailable;
        }

        void SetDurationAvailable(bool aDurationAvailable)
        {
            oDurationAvailable = aDurationAvailable;
        }

        uint64  getDurationValue()
        {
            return iDurationValue;
        }

        void setDurationValue(uint64 aDuration)
        {
            iDurationValue = aDuration;
        }

        uint64  getDurationTimeScale()
        {
            return iDurationTimeScale;
        }

        void setDurationTimeScale(uint64  aTimeScale)
        {
            iDurationTimeScale = aTimeScale;
        }

        void Reset()
        {
            iTrackInfoVec.clear();
            iDurationValue = 0;
            iDurationTimeScale = 0;
            oIsSeekable = false;
            iPresentationType = _STRLIT_CHAR("");
            oDurationAvailable = true;
        }

    private:
        OSCL_HeapString<OsclMemAllocator> iPresentationType;
        uint64  iDurationValue;
        uint64  iDurationTimeScale;
        bool   oIsSeekable;
        Oscl_Vector<PVMFTrackInfo, OsclMemAllocator> iTrackInfoVec;
        bool   oDurationAvailable;
};

#endif //PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED

