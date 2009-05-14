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
    This PVA_FF_UserDataAtom Class fp a container atom for informative user-data.
*/


#define __IMPLEMENT_UserDataAtom__

#include "userdataatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"
#include "assetinfoatoms.h"

typedef Oscl_Vector<PVA_FF_Atom*, OsclMemAllocator> PVA_FF_AtomVecType;
// Constructor
PVA_FF_UserDataAtom::PVA_FF_UserDataAtom() : PVA_FF_Atom(USER_DATA_ATOM)
{
    _size = getDefaultSize();
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AtomVecType, (), _pUserDataAtomVec);
}

// Destructor
PVA_FF_UserDataAtom::~PVA_FF_UserDataAtom()
{
    // Delete all USER data Atoms

    for (uint32 i = 0; i < _pUserDataAtomVec->size(); i++)
    {
        PVA_FF_Atom* atomPtr = (*_pUserDataAtomVec)[i];
        if (atomPtr->getType() == ASSET_INFO_TITLE_ATOM)
        {
            PVA_FF_AssetInfoTitleAtom * ptr = (PVA_FF_AssetInfoTitleAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoTitleAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_DESCP_ATOM)
        {
            PVA_FF_AssetInfoDescAtom * ptr = (PVA_FF_AssetInfoDescAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoDescAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_PERF_ATOM)
        {
            PVA_FF_AssetInfoPerformerAtom * ptr = (PVA_FF_AssetInfoPerformerAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoPerformerAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_AUTHOR_ATOM)
        {
            PVA_FF_AssetInfoAuthorAtom * ptr = (PVA_FF_AssetInfoAuthorAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoAuthorAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_GENRE_ATOM)
        {
            PVA_FF_AssetInfoGenreAtom * ptr = (PVA_FF_AssetInfoGenreAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoGenreAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_RATING_ATOM)
        {
            PVA_FF_AssetInfoRatingAtom * ptr = (PVA_FF_AssetInfoRatingAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoRatingAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_KEYWORD_ATOM)
        {
            PVA_FF_AssetInfoKeyWordsAtom * ptr = (PVA_FF_AssetInfoKeyWordsAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoKeyWordsAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_CLSF_ATOM)
        {
            PVA_FF_AssetInfoClassificationAtom * ptr = (PVA_FF_AssetInfoClassificationAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoClassificationAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_LOCINFO_ATOM)
        {
            PVA_FF_AssetInfoLocationInfoAtom * ptr = (PVA_FF_AssetInfoLocationInfoAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoLocationInfoAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_CPRT_ATOM)
        {
            PVA_FF_AssetInfoCopyRightAtom * ptr = (PVA_FF_AssetInfoCopyRightAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoCopyRightAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_ALBUM_TITLE_ATOM)
        {
            PVA_FF_AssetInfoAlbumAtom * ptr = (PVA_FF_AssetInfoAlbumAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoAlbumAtom , ptr);
        }
        else if (atomPtr->getType() == ASSET_INFO_RECORDING_YEAR_ATOM)
        {
            PVA_FF_AssetInfoRecordingYearAtom * ptr = (PVA_FF_AssetInfoRecordingYearAtom*)(*_pUserDataAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoRecordingYearAtom , ptr);
        }

    }

    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_AtomVecType, Oscl_Vector, _pUserDataAtomVec);
    return;
}

void PVA_FF_UserDataAtom::addAtom(PVA_FF_Atom* atom)
{
    // Adds a new vector to the array of user data atom pointers
    _pUserDataAtomVec->push_back(atom);
    atom->setParent(this);
    recomputeSize();
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool PVA_FF_UserDataAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered
    uint32 i = 0;

    recomputeSize();

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render all the user data atoms in sequence to the file
    for (i = 0; i < _pUserDataAtomVec->size(); i++)
    {
        if (!((*_pUserDataAtomVec)[i]->renderToFileStream(fp)))
        {
            return false;
        }
        rendered += (*_pUserDataAtomVec)[i]->getSize();
    }

    return true;
}

void
PVA_FF_UserDataAtom::recomputeSize()
{
    _size = getDefaultSize();

    for (uint32 i = 0; i < _pUserDataAtomVec->size(); i++)
    {
        _size += (*_pUserDataAtomVec)[i]->getSize();
    }

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}





