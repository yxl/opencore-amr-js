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
/*
    This PVA_FF_MediaInformationHeaderAtom Class fp the base class of the other
    MediaInformationHeaderAtoms, PVA_FF_VideoMediaHeaderAtom VIDEO_MEDIA_HEADER_ATOM, PVA_FF_SoundMediaHeaderAtom
    SOUND_MEDIA_HEADER_ATOM, PVA_FF_HintMediaHeaderAtom HINT_MEDIA_HEADER_ATOM, and PVA_FF_Mpeg4MediaHeaderAtom MPEG4_MEDIA_HEADER_ATOM
*/


#define IMPLEMENT_MediaInformationAtom_H__

#include "mediainformationatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

#include "videomediaheaderatom.h"
#include "soundmediaheaderatom.h"
#include "hintmediaheaderatom.h"
#include "mpeg4mediaheaderatom.h"

// Constructor
PVA_FF_MediaInformationAtom::PVA_FF_MediaInformationAtom(uint32 mediaType,
        int32 codecType,
        uint32 fileAuthoringFlags,
        uint32 protocol,
        uint8 profile,
        uint8 profileComp,
        uint8 level)
        : PVA_FF_Atom(MEDIA_INFORMATION_ATOM)
{
    iMediaType = 0;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SampleTableAtom, (mediaType,
                  codecType,
                  fileAuthoringFlags,
                  protocol, profile,
                  profileComp, level), _psampleTableAtom);

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DataInformationAtom, (), _pdataInformationAtom);

    // Create correct PVA_FF_MediaInformationHeaderAtom based on mediaType
    init(mediaType);

    recomputeSize();
}


// Destructor
PVA_FF_MediaInformationAtom::~PVA_FF_MediaInformationAtom()
{
    uint32 mediaType = (uint32)iMediaType;
    PV_MP4_FF_DELETE(NULL, PVA_FF_SampleTableAtom, _psampleTableAtom);

    PV_MP4_FF_DELETE(NULL, PVA_FF_DataInformationAtom, _pdataInformationAtom);

    if (mediaType == MEDIA_TYPE_AUDIO)
    {
        PVA_FF_SoundMediaHeaderAtom *ptr = (PVA_FF_SoundMediaHeaderAtom *)_pmediaInformationHeader;
        PV_MP4_FF_DELETE(NULL, PVA_FF_SoundMediaHeaderAtom, ptr);
    }
    else if (mediaType == MEDIA_TYPE_VISUAL)
    {
        PVA_FF_VideoMediaHeaderAtom *ptr = (PVA_FF_VideoMediaHeaderAtom *)_pmediaInformationHeader;
        PV_MP4_FF_DELETE(NULL, PVA_FF_VideoMediaHeaderAtom, ptr);
    }
    else
    {
        PVA_FF_Mpeg4MediaHeaderAtom *ptr = (PVA_FF_Mpeg4MediaHeaderAtom *)_pmediaInformationHeader;
        PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4MediaHeaderAtom, ptr);
    }



}

void
PVA_FF_MediaInformationAtom::init(int32 mediaType)
{
    iMediaType = mediaType;
    switch (mediaType)
    {
        case MEDIA_TYPE_AUDIO:
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SoundMediaHeaderAtom, (), _pmediaInformationHeader);
            break;
        case MEDIA_TYPE_VISUAL:

            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_VideoMediaHeaderAtom, (), _pmediaInformationHeader);
            break;
        default:

            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_Mpeg4MediaHeaderAtom, (mediaType), _pmediaInformationHeader);
            break;
    }
    _pmediaInformationHeader->setParent(this);
    _pdataInformationAtom->setParent(this);
    _psampleTableAtom->setParent(this);
}

void
PVA_FF_MediaInformationAtom::nextSample(int32 mediaType,
                                        void *psample,
                                        uint32 size,
                                        uint32 ts,
                                        uint8 flags,
                                        uint32 baseOffset,
                                        bool oChunkStart)
{
    _psampleTableAtom->nextSample(mediaType, psample,
                                  size, ts, flags,
                                  baseOffset, oChunkStart);
}

void
PVA_FF_MediaInformationAtom::nextTextSample(int32 mediaType,
        void *psample,
        uint32 size,
        uint32 ts,
        uint8 flags,
        int32 index,
        uint32 baseOffset,
        bool oChunkStart)
{
    _psampleTableAtom->nextTextSample(mediaType, psample,
                                      size, ts, flags, index,
                                      baseOffset, oChunkStart);
}

void
PVA_FF_MediaInformationAtom::nextSample(int32 mediaType,
                                        Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                        uint32 size,
                                        uint32 ts,
                                        uint8 flags,
                                        uint32 baseOffset,
                                        bool oChunkStart)
{
    _psampleTableAtom->nextSample(mediaType, fragmentList,
                                  size, ts, flags,
                                  baseOffset, oChunkStart);

}

void
PVA_FF_MediaInformationAtom::nextTextSample(int32 mediaType,
        Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
        uint32 size,
        uint32 ts,
        uint8 flags,
        int32 index,
        uint32 baseOffset,
        bool oChunkStart)
{
    _psampleTableAtom->nextTextSample(mediaType, fragmentList,
                                      size, ts, flags, index,
                                      baseOffset, oChunkStart);

}

bool
PVA_FF_MediaInformationAtom::reAuthorFirstSample(uint32 size,
        uint32 baseOffset)
{
    return(
              _psampleTableAtom->reAuthorFirstSample(size,
                                                     baseOffset));
}

// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_MediaInformationAtom::updateLastTSEntry(uint32 ts)
{

    _psampleTableAtom->updateLastTSEntry(ts);
}

void
PVA_FF_MediaInformationAtom::recomputeSize()
{
    uint32 size = getDefaultSize();
    // Include size from actual members once they get implemented
    size += _pmediaInformationHeader->getSize();

    size += _pdataInformationAtom->getSize();

    size += _psampleTableAtom->getSize();

    _size = size;

    // Update the size of the parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_MediaInformationAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the data of the members once they get implemented
    if (!_pmediaInformationHeader->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pmediaInformationHeader->getSize();

    if (!_pdataInformationAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pdataInformationAtom->getSize();

    if (!_psampleTableAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _psampleTableAtom->getSize();

    return true;
}
void
PVA_FF_MediaInformationAtom::SetMaxSampleSize(uint32 aSize)
{
    _psampleTableAtom->SetMaxSampleSize(aSize);
}

void
PVA_FF_MediaInformationAtom::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    _psampleTableAtom->writeMaxSampleSize(_afp);
}


