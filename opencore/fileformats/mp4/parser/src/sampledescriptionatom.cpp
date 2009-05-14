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
    This SampleDescriptionAtom Class gives detailed information about the codeing
    type used, and any initialization information needed for coding.
*/


#define IMPLEMENT_SampleDescriptionAtom

#include "sampledescriptionatom.h"

#include "visualsampleentry.h"
#include "audiosampleentry.h"
#include "mpegsampleentry.h"


#include "textsampleentry.h"

#include "atomutils.h"
#include "atomdefs.h"
#include "objectdescriptor.h"

#include "amrsampleentry.h"
#include "h263sampleentry.h"

#include "oma2boxes.h"

#define MAX_ALLOWED_MEDIA_SAMPLE_ENTRIES 16

typedef Oscl_Vector<SampleEntry*, OsclMemAllocator> sampleEntryVecType;
typedef Oscl_Vector<AVCSampleEntry*, OsclMemAllocator> AVCSampleEntryVecType;

// Stream-in ctor
SampleDescriptionAtom::SampleDescriptionAtom(MP4_FF_FILE *fp,
        uint32 mediaType,
        uint32 size,
        uint32 type)
        : FullAtom(fp, size, type),
        _pMediaType(mediaType)
{
    _psampleEntryVec      = NULL;
    _pAMRSampleEntryAtom  = NULL;
    _pH263SampleEntryAtom = NULL;
    _pAVCSampleEntryVec   = NULL;
    _o3GPPAMR = false;
    _o3GPPH263 = false;
    _o3GPPWBAMR = false;
    _oAVC = false;


    uint32 count = size - DEFAULT_ATOM_SIZE;
    count -= 4; //for 4-byte handle_type

    _pProtectionSchemeInformationBox = NULL;

    if (_success)
    {
        if (!AtomUtils::read32(fp, _entryCount))
        {
            _success = false;
        }
        count -= 4;
        int32 tmp = (int32)_entryCount;

        if (tmp <= 0)
        {
            _success = false;
            _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
            return;
        }

        if ((mediaType != MEDIA_TYPE_HINT) &&
                (mediaType != MEDIA_TYPE_TEXT))
        {
            if (tmp > MAX_ALLOWED_MEDIA_SAMPLE_ENTRIES)
            {
                _success = false;
                _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
                return;
            }
        }

        if (_success)
        {
            PV_MP4_FF_NEW(fp->auditCB, sampleEntryVecType, (), _psampleEntryVec);
            PV_MP4_FF_NEW(fp->auditCB, AVCSampleEntryVecType, (), _pAVCSampleEntryVec);


            for (uint32 i = 0; i < _entryCount; i++)
            {
                SampleEntry *entry = NULL;

                uint32 atomType = UNKNOWN_ATOM;
                uint32 atomSize = 0;

                AtomUtils::getNextAtomType(fp, atomSize, atomType);
                count -= atomSize;

                switch (mediaType)
                {
                    case MEDIA_TYPE_AUDIO:
                    {
                        if (atomType == ENCRYPTED_AUDIO_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, EcnaBox, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                EcnaBox *ptr = (EcnaBox *)entry;
                                PV_MP4_FF_DELETE(NULL, EcnaBox, ptr);
                                return;
                            }
                            _pProtectionSchemeInformationBox =
                                ((EcnaBox*)entry)->_pProtectionSchemeInformationBox;
                        }
                        else if (atomType == AMR_SAMPLE_ENTRY_ATOM)
                        {
                            if (_o3GPPAMR == false)
                            {
                                PV_MP4_FF_NEW(fp->auditCB, AMRSampleEntry, (fp, atomSize, atomType), _pAMRSampleEntryAtom);

                                if (!_pAMRSampleEntryAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pAMRSampleEntryAtom->GetMP4Error();
                                    PV_MP4_FF_DELETE(NULL, AMRSampleEntry, _pAMRSampleEntryAtom);
                                    _pAMRSampleEntryAtom = NULL;
                                    return;
                                }
                                else
                                {
                                    _pAMRSampleEntryAtom->setParent(this);
                                }
                                _o3GPPAMR = true;
                            }
                            else
                            {
                                // Multiple AMR Sample Entries are illegal
                                _success = false;
                                _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
                                return;
                            }
                        }
                        else if (atomType == AMRWB_SAMPLE_ENTRY_ATOM)
                        {
                            if (_o3GPPWBAMR == false)
                            {
                                PV_MP4_FF_NEW(fp->auditCB, AMRSampleEntry, (fp, atomSize, atomType), _pAMRSampleEntryAtom);

                                if (!_pAMRSampleEntryAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pAMRSampleEntryAtom->GetMP4Error();
                                    PV_MP4_FF_DELETE(NULL, AMRSampleEntry, _pAMRSampleEntryAtom);
                                    _pAMRSampleEntryAtom = NULL;
                                    return;
                                }
                                else
                                {
                                    _pAMRSampleEntryAtom->setParent(this);
                                }
                                _o3GPPWBAMR = true;
                            }
                            else
                            {
                                // Multiple AMR Sample Entries are illegal
                                _success = false;
                                _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
                                return;
                            }
                        }
                        else if (atomType == AUDIO_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, AudioSampleEntry, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                AudioSampleEntry *ptr = (AudioSampleEntry *)entry;
                                PV_MP4_FF_DELETE(NULL, AudioSampleEntry, ptr);
                                return;
                            }
                        }

                        else
                        {
                            atomSize -= DEFAULT_ATOM_SIZE;
                            AtomUtils::seekFromCurrPos(fp, atomSize);
                        }
                        _handlerType = MEDIA_TYPE_AUDIO;
                        break;
                    }

                    case MEDIA_TYPE_TEXT:
                    {
// TIMED_TEXT_SUPPORT start
                        if (atomType == ENCRYPTED_TEXT_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, EnctBox, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                EnctBox *ptr = (EnctBox *)entry;
                                PV_MP4_FF_DELETE(NULL, EnctBox, ptr);
                                return;
                            }
                            _pProtectionSchemeInformationBox =
                                ((EnctBox*)entry)->_pProtectionSchemeInformationBox;

                        }
                        else if (atomType == TEXT_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, TextSampleEntry, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                TextSampleEntry *ptr = (TextSampleEntry *)entry;
                                PV_MP4_FF_DELETE(NULL, TextSampleEntry, ptr);
                                return;
                            }

                        }
                        else
// TIMED_TEXT_SUPPORT end
                        {
                            atomSize -= DEFAULT_ATOM_SIZE;
                            AtomUtils::seekFromCurrPos(fp, atomSize);
                        }
                        _handlerType = MEDIA_TYPE_TEXT;
                    }
                    break;

                    case MEDIA_TYPE_VISUAL:
                    {
                        if (atomType == ENCRYPTED_VIDEO_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, EcnvBox, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                EcnvBox *ptr = (EcnvBox *)entry;
                                PV_MP4_FF_DELETE(NULL, EcnvBox, ptr);
                                return;
                            }
                            _pProtectionSchemeInformationBox =
                                ((EcnvBox*)entry)->_pProtectionSchemeInformationBox;

                        }
                        else if (atomType == H263_SAMPLE_ENTRY_ATOM)
                        {
                            if (_o3GPPH263 == false)
                            {
                                PV_MP4_FF_NEW(fp->auditCB, H263SampleEntry, (fp, atomSize, atomType), _pH263SampleEntryAtom);

                                if (!_pH263SampleEntryAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pH263SampleEntryAtom->GetMP4Error();
                                    return;
                                }
                                else
                                {
                                    _pH263SampleEntryAtom->setParent(this);
                                }
                                _o3GPPH263 = true;
                            }
                            else
                            {
                                // Multiple H263 Sample Entries are illegal
                                _success = false;
                                _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
                                return;
                            }
                        }
                        else if (atomType == VIDEO_SAMPLE_ENTRY)
                        {
                            PV_MP4_FF_NEW(fp->auditCB, VisualSampleEntry, (fp, atomSize, atomType), entry);
                            if (!entry->MP4Success())
                            {
                                _success = false;
                                _mp4ErrorCode = entry->GetMP4Error();
                                VisualSampleEntry *ptr = (VisualSampleEntry *)entry;
                                PV_MP4_FF_DELETE(NULL, VisualSampleEntry, ptr);
                                return;
                            }
                        }
                        else if (atomType == AVC_SAMPLE_ENTRY)
                        {
                            if (_oAVC == false)
                            {
                                AVCSampleEntry* pAVCSampleEntry = NULL;
                                PV_MP4_FF_NEW(fp->auditCB, AVCSampleEntry, (fp, atomSize, atomType), pAVCSampleEntry);
                                if (!pAVCSampleEntry->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = pAVCSampleEntry->GetMP4Error();
                                    PV_MP4_FF_DELETE(NULL, AVCSampleEntry, pAVCSampleEntry);
                                    return;
                                }
                                else
                                {
                                    pAVCSampleEntry->setParent(this);
                                }
                                _oAVC = true;

                                if (pAVCSampleEntry != NULL)
                                {
                                    (*_pAVCSampleEntryVec).push_back(pAVCSampleEntry);
                                }
                            }
                            else
                            {
                                // Multiple AVC Sample Entries are illegal
                                _success = false;
                                _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
                                return;
                            }
                        }
                        else
                        {
                            atomSize -= DEFAULT_ATOM_SIZE;
                            AtomUtils::seekFromCurrPos(fp, atomSize);
                        }
                        _handlerType = MEDIA_TYPE_VISUAL;
                        break;
                    }

                    default:
                    {
                        atomSize -= DEFAULT_ATOM_SIZE;
                        AtomUtils::seekFromCurrPos(fp, atomSize);
                    }
                }

                if (entry != NULL)
                {
                    if (!entry->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = entry->GetMP4Error();
                        break; // Break out of the for loop
                    }
                    else
                    {
                        entry->setParent(this);
                        (*_psampleEntryVec).push_back(entry);
                    }
                }
            } // end for loop
            if (count > 0)
            {
                //skip the rest of bytes
                AtomUtils::seekFromCurrPos(fp, count);
            }

        } // end if success

    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_SAMPLE_DESCRIPTION_ATOM_FAILED;
    }
}

// Destructor
SampleDescriptionAtom::~SampleDescriptionAtom()
{
    if (_pAMRSampleEntryAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AMRSampleEntry, _pAMRSampleEntryAtom);
        _pAMRSampleEntryAtom = NULL;
    }

    if (_pH263SampleEntryAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, H263SampleEntry, _pH263SampleEntryAtom);
    }

    if (_psampleEntryVec != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _psampleEntryVec->size(); i++)
        {
            SampleEntry * pSampleEntryPtr = (SampleEntry*)(*_psampleEntryVec)[i];

            if (pSampleEntryPtr->getType() == AMR_SAMPLE_ENTRY_ATOM)
            {
                AMRSampleEntry *ptr = (AMRSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, AMRSampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == AUDIO_SAMPLE_ENTRY)
            {
                AudioSampleEntry *ptr = (AudioSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, AudioSampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == TEXT_SAMPLE_ENTRY)
            {
                TextSampleEntry *ptr = (TextSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, TextSampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == H263_SAMPLE_ENTRY_ATOM)
            {
                H263SampleEntry *ptr = (H263SampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, H263SampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == VIDEO_SAMPLE_ENTRY)
            {
                VisualSampleEntry *ptr = (VisualSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, VisualSampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == AVC_SAMPLE_ENTRY)
            {
                AVCSampleEntry *ptr = (AVCSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, AVCSampleEntry, ptr);
            }
            else if (pSampleEntryPtr->getType() == MPEG_SAMPLE_ENTRY)
            {
                MpegSampleEntry *ptr = (MpegSampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, MpegSampleEntry, ptr);
            }

            else
            {
                SampleEntry *ptr = (SampleEntry *)(*_psampleEntryVec)[i];
                PV_MP4_FF_DELETE(NULL, SampleEntry, ptr);

            }
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, sampleEntryVecType, Oscl_Vector, _psampleEntryVec);
    }

    if (_pAVCSampleEntryVec != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAVCSampleEntryVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AVCSampleEntry, (*_pAVCSampleEntryVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, AVCSampleEntryVecType, Oscl_Vector, _pAVCSampleEntryVec);
    }
}

// Returns the ESID of the first ESdescriptor of the track - which is the ONLY
// ESID for that track.  Each track has a single unique ESID.
uint32
SampleDescriptionAtom::getESID(uint32 index)
{
    switch (_pMediaType)
    {
        case MEDIA_TYPE_HINT:
            return 0; // This is actually an undefined situation
        default:
            const SampleEntry* entry = getSampleEntryAt(index);
            if (!entry)
                return 0;
            return entry->getESID();
    }
}

// Return the first ESDescriptor for the track - undefined if a HINT track
const ESDescriptor *
SampleDescriptionAtom::getESDescriptor(uint32 index)
{
    switch (_pMediaType)
    {
        case MEDIA_TYPE_HINT:
            return NULL; // This is actually an undefined situation
        default:
            const SampleEntry* entry = getSampleEntryAt(index);
            if (!entry)
                return NULL;
            return entry->getESDescriptor();
    }
}

// Getting and setting the Mpeg4 VOL header
DecoderSpecificInfo *
SampleDescriptionAtom::getDecoderSpecificInfo(uint32 index)
{
    if (_o3GPPH263)
    {
        if (_pH263SampleEntryAtom != NULL)
        {
            return (_pH263SampleEntryAtom->get3GPPH263DecoderSpecificInfo());
        }
        else
        {
            return NULL;
        }
    }

    if ((_o3GPPAMR) || (_o3GPPWBAMR))
    {
        if (_pAMRSampleEntryAtom != NULL)
        {
            return (_pAMRSampleEntryAtom->getDecoderSpecificInfo(index));
        }
        else
        {
            return NULL;
        }
    }

    if (_oAVC)
    {
        return (((*_pAVCSampleEntryVec)[index])->getDecoderSpecificInfo());
    }

    const SampleEntry *se = getSampleEntryAt(index);
    if ((se != NULL) &&
            ((_pMediaType == MEDIA_TYPE_VISUAL) || (_pMediaType == MEDIA_TYPE_AUDIO)))
    {
        return se->getDecoderSpecificInfo();
    }
    return NULL;
}

uint8  SampleDescriptionAtom::getObjectTypeIndication()
{
    if (_o3GPPAMR)
    {
        return (AMR_AUDIO_3GPP);
    }

    if (_o3GPPWBAMR)
    {
        return (AMRWB_AUDIO_3GPP);
    }

    if (_o3GPPH263)
    {
        return (H263_VIDEO);
    }

    if (_oAVC)
        return (AVC_VIDEO);

    // ok to continue if size()==0, will be
    // caught on MEDIA_TYPE_AUDIO and MEDIA_TYPE_VISUAL
    // but not on MEDIA_TYPE_TEXT or default case
    const SampleEntry  *entry = getSampleEntryAt(0);
    if (! entry)
        return TYPE_UNKNOWN;
    return entry->getObjectTypeIndication();
}


int32 SampleDescriptionAtom::getAverageBitrate()
{
    uint32 averageBitrate = 0;
    uint32 i = 0;

    if (_o3GPPAMR)
    {
        if (_pAMRSampleEntryAtom != NULL)
        {
            return (_pAMRSampleEntryAtom->getBitRate());
        }
        else
        {
            return 0;
        }
    }

    if (_o3GPPH263)
    {
        if (_pH263SampleEntryAtom != NULL)
        {
            return (_pH263SampleEntryAtom->getAverageBitRate());
        }
        else
        {
            return 0;
        }
    }

    if (_oAVC)
    {
        if (_pAVCSampleEntryVec->size() > 0)
        {
            return (*_pAVCSampleEntryVec)[0]->getAvgBitRate();
        }
        else
        {
            return 0;
        }
    }
    const SampleEntry* entry = getSampleEntryAt(i);
    if (!entry)		// will also pick off size() == 0
        return 0;

    averageBitrate = entry->getAverageBitrate();
    return (averageBitrate);

}

int32 SampleDescriptionAtom::getWidth()
{
    uint32 width = 0;
    uint32 i = 0;

    if (_o3GPPH263)
    {
        if (_pH263SampleEntryAtom != NULL)
        {
            return (_pH263SampleEntryAtom->getWidth());
        }
        else
        {
            return 0;
        }
    }
    if (_oAVC)
    {
        return (((*_pAVCSampleEntryVec)[i])->getWidth());
    }

    const SampleEntry* entry = getSampleEntryAt(i);
    if (!entry)		// will also pick off size() == 0
        return 0;

    width = entry->getWidth();
    return (width);

}

int32 SampleDescriptionAtom::getHeight()
{

    uint32 i = 0;
    if (_o3GPPH263)
    {
        if (_pH263SampleEntryAtom != NULL)
        {
            return (_pH263SampleEntryAtom->getHeight());
        }
        else
        {
            return 0;
        }
    }
    if (_oAVC)
    {
        return (((*_pAVCSampleEntryVec)[i])->getHeight());
    }

    const SampleEntry* entry = getSampleEntryAt(i);
    if (!entry)		// also catch size() == 0
        return 0;
    return (uint32)entry->getHeight();
}


uint32
SampleDescriptionAtom::getMaxBufferSizeDB()
{
    uint32 MaxBufSize = 0;

    if (_o3GPPAMR)
    {
        if (_pAMRSampleEntryAtom != NULL)
        {
            return 512;
        }
    }


    if (_psampleEntryVec->size() == 0)
    {
        return 0;
    }

    for (uint32 i = 0; i < _entryCount; i++)
    {
        const SampleEntry *entry = getSampleEntryAt(i);
        if (!entry)
            return 0;
        if (entry->getMaxBufferSizeDB() > MaxBufSize)
        {
            MaxBufSize = entry->getMaxBufferSizeDB();
        }
    }
    return (MaxBufSize);
}

//PASP box
uint32 SampleDescriptionAtom::getHspacing()
{
    uint32 hSpacing = 0;
    uint32 i = 0;

    if (_oAVC)
    {
        return (((*_pAVCSampleEntryVec)[i])->getHspacing());
    }

    return (hSpacing);
}

uint32 SampleDescriptionAtom::getVspacing()
{
    uint32 vSpacing = 0;
    uint32 i = 0;

    if (_oAVC)
    {
        return (((*_pAVCSampleEntryVec)[i])->getVspacing());
    }

    return (vSpacing);

}

uint32
SampleDescriptionAtom::getSampleProtocol(uint32 index)
{

    // Return the atom type of the sampleEntryAtom - for the first one in the vector
    const SampleEntry* entry = getSampleEntryAt(index);
    if (!entry)		// will also pick off size() == 0
        return 0;

    return entry->getType();
}


// Watch out:  this can now return NULL, so callers need to
//  check for that before derefencing the return value.
// It will catch the case where _psampleEntryVec->size() == 0,
//  so callers don't have to check for that.  (the check on
//  index < size() will fail if size==0)
const SampleEntry*
SampleDescriptionAtom::getSampleEntryAt(int32 index) const
{
    if ((uint32)index < _psampleEntryVec->size())
    {
        return (SampleEntry*)(*_psampleEntryVec)[index];
    }
    else
    {
        return NULL;
    }
}

SampleEntry *
SampleDescriptionAtom::getMutableSampleEntryAt(int32 index)
{
    if ((uint32)index < _entryCount)
    {
        return (SampleEntry*) &(*_psampleEntryVec)[index];
    }
    else
    {
        return NULL;
    }
}

// TIMED_TEXT_SUPPORT start
SampleEntry *
SampleDescriptionAtom:: getTextSampleEntryAt(uint32 index)
{
    if ((_psampleEntryVec->size() == 0) ||
            (index >= (uint32)(_psampleEntryVec->size())))
    {
        return NULL;
    }

    if (_pMediaType == MEDIA_TYPE_TEXT)
    {
        return (SampleEntry*)(*_psampleEntryVec)[index];
    }

    return NULL;
}
// TIMED_TEXT_SUPPORT end

uint32 SampleDescriptionAtom::getTrackLevelOMA2DRMInfoSize()
{
    if (_pProtectionSchemeInformationBox != NULL)
    {
        return (_pProtectionSchemeInformationBox->getTrackLevelOMA2DRMInfoSize());
    }
    return 0;
}

uint8* SampleDescriptionAtom::getTrackLevelOMA2DRMInfo()
{
    if (_pProtectionSchemeInformationBox != NULL)
    {
        return (_pProtectionSchemeInformationBox->getTrackLevelOMA2DRMInfo());
    }
    return NULL;
}


void SampleDescriptionAtom::getMIMEType(OSCL_String& aMimeType)
{
    uint8 objectType;
    objectType = getObjectTypeIndication();

    OSCL_HeapString<OsclMemAllocator> mimeType;

    mimeType.set(PVMF_MIME_FORMAT_UNKNOWN, oscl_strlen(PVMF_MIME_FORMAT_UNKNOWN));

    if (objectType == AMR_AUDIO)
    {
        mimeType.set(PVMF_MIME_AMR, oscl_strlen(PVMF_MIME_AMR));
    }
    else if	(objectType == AMR_AUDIO_3GPP)
    {
        mimeType.set(PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF));
    }
    else if (objectType == AMRWB_AUDIO_3GPP)
    {
        mimeType.set(PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF));
    }
    else if (objectType == MPEG4_AUDIO)
    {
        mimeType.set(PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO));
    }
    else if (objectType == MPEG2_AUDIO_LC)
    {
        mimeType.set(_STRLIT_CHAR(PVMF_MIME_MPEG4_AUDIO), oscl_strlen(PVMF_MIME_MPEG4_AUDIO));
    }
    else if (objectType == MPEG4_VIDEO)
    {
        mimeType.set(PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V));
    }
    else if (objectType == H263_VIDEO)
    {
        mimeType.set(PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000));
    }
    else if (objectType == AVC_VIDEO)
    {
        mimeType.set(PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4));
    }
    else
    {
        if (_pMediaType == MEDIA_TYPE_TEXT)
        {
            mimeType.set(PVMF_MIME_3GPP_TIMEDTEXT, oscl_strlen(PVMF_MIME_3GPP_TIMEDTEXT));
        }
    }

    aMimeType = mimeType;
}



