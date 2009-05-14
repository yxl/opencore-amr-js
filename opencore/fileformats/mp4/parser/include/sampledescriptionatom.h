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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                       MPEG-4 SampleDescriptionAtom Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleDescriptionAtom Class gives detailed information about the coding
    type used, and any initialization information needed for coding.
*/


#ifndef SAMPLEDESCRIPTIONATOM_H_INCLUDED
#define SAMPLEDESCRIPTIONATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef SAMPLEENTRY_H_INCLUDED
#include "sampleentry.h"
#endif

#ifndef ESDESCRIPTOR_H_INCLUDED
#include "esdescriptor.h"
#endif

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif

#ifndef AMRSAMPLEENTRY_H_INCLUDED
#include "amrsampleentry.h"
#endif

#ifndef H263SAMPLEENTRY_H_INCLUDED
#include "h263sampleentry.h"
#endif

#ifndef AVCSAMPLEENTRY_H_INCLUDED
#include "avcsampleentry.h"
#endif

#ifndef AVCSUBSEQDESCRIPTIONENTRY_H_INCLUDED
#include "avcsubseqdescriptionentry.h"
#endif

#ifndef AVCLAYERDESCRIPTIONENTRY_H_INCLUDED
#include "avclayerentry.h"
#endif


class ProtectionSchemeInformationBox;

class SampleDescriptionAtom : public FullAtom
{

    public:
        SampleDescriptionAtom(MP4_FF_FILE *fp,
                              uint32 mediaType,
                              uint32 size,
                              uint32 type);

        virtual ~SampleDescriptionAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }
        uint32 getHandlerType() const
        {
            return _handlerType;
        }

        const SampleEntry* getSampleEntryAt(int32 index) const;
        SampleEntry* getMutableSampleEntryAt(int32 index);

        uint32 getNumSampleEntries()
        {
            if (_psampleEntryVec != NULL)
            {
                return _psampleEntryVec->size();
            }
            else
            {
                return 0;
            }
        }

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo(uint32 index);

        void getMIMEType(OSCL_String& aMimeType);//Move to SampleDescription.cpp

        uint8  getObjectTypeIndication();

        uint32 getMaxBufferSizeDB();	// Get the max size buffer needed to retrieve the media samples
        int32  getAverageBitrate();
        int32  getMaxBitrate();

        uint32 getHspacing();
        uint32 getVspacing();

        // Return the ESID for the track - undefined if a HINT track
        uint32 getESID(uint32 index);

        // Return the first ESDescriptor for the track - undefined if a HINT track
        const ESDescriptor *getESDescriptor(uint32 index);

        // Get the type of SampleEntry atom (i.e. MPEG_SAMPLE_ENTRY, ... '<protocol>', etc.)
        uint32 getSampleProtocol(uint32 index);

        uint32 getMediaType()
        {
            return  _pMediaType;
        }

        AMRSampleEntry *getAMRSampleEntry()
        {
            return _pAMRSampleEntryAtom;
        }

        H263SampleEntry *getH263SampleEntry()
        {
            return _pH263SampleEntryAtom;
        }

        bool Is3GPPAMR()
        {
            if (_o3GPPAMR)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        AVCSampleEntry* getAVCSampleEntry(uint32 index)
        {
            return (*_pAVCSampleEntryVec)[index];
        }

        uint32 getNumAVCSampleEntries()
        {
            if (_pAVCSampleEntryVec != NULL)
                return _pAVCSampleEntryVec->size();
            else
                return 0;
        }

        SampleEntry *getTextSampleEntryAt(uint32 index);

        uint32 getAVCNALLengthSize(uint32 index)
        {
            if (_oAVC)
            {
                return (((*_pAVCSampleEntryVec)[index])->getNALLengthSize());
            }
            return 0;
        }

        uint32 getTrackLevelOMA2DRMInfoSize();
        uint8* getTrackLevelOMA2DRMInfo();

        int32 getHeight();
        int32 getWidth();
    private:
        uint32 _entryCount;
        uint32 _handlerType;
        Oscl_Vector<SampleEntry*, OsclMemAllocator> *_psampleEntryVec;

        uint32 _pMediaType;

        AMRSampleEntry  *_pAMRSampleEntryAtom;
        H263SampleEntry *_pH263SampleEntryAtom;
        Oscl_Vector<AVCSampleEntry*, OsclMemAllocator> *_pAVCSampleEntryVec;
        AVCSubSequenceEntry *_pavcSubSequenceEntry;
        AVCLayerDescEntry *_pavcLayerDescEntry;

        bool _o3GPPAMR;
        bool _o3GPPH263;
        bool _o3GPPWBAMR;
        bool _oAVC;

        ProtectionSchemeInformationBox* _pProtectionSchemeInformationBox;
};


#endif // SAMPLEDESCRIPTIONATOM_H_INCLUDED
