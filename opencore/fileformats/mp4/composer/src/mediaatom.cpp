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
    This PVA_FF_MediaAtom Class contains all the objects that declare information
    about the media data within the stream.
*/

#define IMPLEMENT_MediaAtom_H__

#include "mediaatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

// Constructor
PVA_FF_MediaAtom::PVA_FF_MediaAtom(int32 mediaType,
                                   int32 codecType,
                                   uint32 fileAuthoringFlags,
                                   uint32 protocol,
                                   uint8 profile,
                                   uint8 profileComp,
                                   uint8 level)

        : PVA_FF_Atom(MEDIA_ATOM)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MediaHeaderAtom, (), _pmediaHeader);

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_HandlerAtom, (mediaType, (uint8)0, (uint8)0), _phandler);

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MediaInformationAtom, (mediaType,
                  codecType,
                  fileAuthoringFlags,
                  protocol, profile,
                  profileComp, level), _pmediaInformation);

    recomputeSize();
    _pmediaHeader->setParent(this);
    _phandler->setParent(this);
    _pmediaInformation->setParent(this);
}

// Destructor
PVA_FF_MediaAtom::~PVA_FF_MediaAtom()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_MediaHeaderAtom, _pmediaHeader);
    PV_MP4_FF_DELETE(NULL, PVA_FF_HandlerAtom, _phandler);
    PV_MP4_FF_DELETE(NULL, PVA_FF_MediaInformationAtom, _pmediaInformation);
}

void
PVA_FF_MediaAtom::nextSample(int32 mediaType,
                             void *psample,
                             uint32 size,
                             uint32 ts,
                             uint8 flags,
                             uint32 baseOffset,
                             bool oChunkStart)
{
    _pmediaInformation->nextSample(mediaType, psample, size, ts,
                                   flags, baseOffset, oChunkStart);
    _pmediaHeader->addSample(ts);
}

void
PVA_FF_MediaAtom::nextTextSample(int32 mediaType,
                                 void *psample,
                                 uint32 size,
                                 uint32 ts,
                                 uint8 flags,
                                 int32 index,
                                 uint32 baseOffset,
                                 bool oChunkStart)
{
    _pmediaInformation->nextTextSample(mediaType, psample, size, ts,
                                       flags, index, baseOffset, oChunkStart);
    _pmediaHeader->addSample(ts);
}

void
PVA_FF_MediaAtom::nextSample(int32 mediaType,
                             Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                             uint32 size,
                             uint32 ts,
                             uint8 flags,
                             uint32 baseOffset,
                             bool oChunkStart)
{
    _pmediaInformation->nextSample(mediaType, fragmentList, size, ts,
                                   flags, baseOffset, oChunkStart);

    _pmediaHeader->addSample(ts);
}

void
PVA_FF_MediaAtom::nextTextSample(int32 mediaType,
                                 Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                 uint32 size,
                                 uint32 ts,
                                 uint8 flags,
                                 int32 index,
                                 uint32 baseOffset,
                                 bool oChunkStart)
{
    _pmediaInformation->nextTextSample(mediaType, fragmentList, size, ts,
                                       flags, index, baseOffset, oChunkStart);

    _pmediaHeader->addSample(ts);
}

bool
PVA_FF_MediaAtom::reAuthorFirstSample(uint32 size,
                                      uint32 baseOffset)
{
    return(
              _pmediaInformation->reAuthorFirstSample(size,
                                                      baseOffset));
}

// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_MediaAtom::updateLastTSEntry(uint32 ts)
{

    _pmediaInformation->updateLastTSEntry(ts);

    _pmediaHeader->updateLastTSEntry(ts);
}


void
PVA_FF_MediaAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // Get size of base class members

    // Include sizes of all members
    size += _pmediaHeader->getSize();
    size += _phandler->getSize();
    size += _pmediaInformation->getSize();

    _size = size;

    // Update size of parent atom if it exists
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_MediaAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render all member atoms
    if (!_pmediaHeader->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pmediaHeader->getSize();

    if (!_phandler->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _phandler->getSize();

    if (!_pmediaInformation->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pmediaInformation->getSize();

    return true;
}

void
PVA_FF_MediaAtom::SetMaxSampleSize(uint32 aSize)
{
    _pmediaInformation->SetMaxSampleSize(aSize);
}

void
PVA_FF_MediaAtom::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    _pmediaInformation->writeMaxSampleSize(_afp);
}
