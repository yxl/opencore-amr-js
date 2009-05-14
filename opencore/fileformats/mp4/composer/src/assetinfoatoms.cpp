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
#include "a_atomdefs.h"
#include "assetinfoatoms.h"

typedef Oscl_Vector<PVA_FF_AssetInfoKeyWordStruct*, OsclMemAllocator> PVA_FF_AssetInfoKeyWordStructVecType;

PVA_FF_AssetInfoTitleAtom::PVA_FF_AssetInfoTitleAtom()
        : PVA_FF_FullAtom(ASSET_INFO_TITLE_ATOM, 0, 0)
{
    _title = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoDescAtom::PVA_FF_AssetInfoDescAtom()
        : PVA_FF_FullAtom(ASSET_INFO_DESCP_ATOM, 0, 0)
{
    _description = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoCopyRightAtom::PVA_FF_AssetInfoCopyRightAtom()
        : PVA_FF_FullAtom(ASSET_INFO_CPRT_ATOM, 0, 0)
{
    _cprt = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoPerformerAtom::PVA_FF_AssetInfoPerformerAtom()
        : PVA_FF_FullAtom(ASSET_INFO_PERF_ATOM, 0, 0)
{
    _performer = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoAuthorAtom::PVA_FF_AssetInfoAuthorAtom()
        : PVA_FF_FullAtom(ASSET_INFO_AUTHOR_ATOM, 0, 0)
{
    _author = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoGenreAtom::PVA_FF_AssetInfoGenreAtom()
        : PVA_FF_FullAtom(ASSET_INFO_GENRE_ATOM, 0, 0)
{
    _genre = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoRatingAtom::PVA_FF_AssetInfoRatingAtom()
        : PVA_FF_FullAtom(ASSET_INFO_RATING_ATOM, 0, 0)
{
    _ratingInfo		=	(_STRLIT(""));
    _ratingEntity    =   RATING_ENTITY_UNKNOWN;
    _ratingCriteria  =   RATING_CRITERIA_UNKNOWN;
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoClassificationAtom::PVA_FF_AssetInfoClassificationAtom()
        : PVA_FF_FullAtom(ASSET_INFO_CLSF_ATOM, 0, 0)
{
    _classificationInfo		=	(_STRLIT(""));
    _classificationEntity    =   0;
    _classificationTable		=   0;
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoKeyWordsAtom::PVA_FF_AssetInfoKeyWordsAtom()
        : PVA_FF_FullAtom(ASSET_INFO_KEYWORD_ATOM, 0, 0)
{


    _keyWordCnt   =   0;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoKeyWordStructVecType, (), _pKeyWordVect);
    _langCode = LANGUAGE_CODE_UNKNOWN;
    recomputeSize();

}

PVA_FF_AssetInfoAlbumAtom::PVA_FF_AssetInfoAlbumAtom()
        : PVA_FF_FullAtom(ASSET_INFO_ALBUM_TITLE_ATOM, 0, 0)
{
    _albumTitle = (_STRLIT(""));
    _langCode = LANGUAGE_CODE_UNKNOWN;
    _byteOrderMask = BYTE_ORDER_MASK;
    recomputeSize();
}

PVA_FF_AssetInfoRecordingYearAtom::PVA_FF_AssetInfoRecordingYearAtom()
        : PVA_FF_FullAtom(ASSET_INFO_RECORDING_YEAR_ATOM, 0, 0)
{
    _recordingYear = 0;
    recomputeSize();
}

PVA_FF_AssetInfoKeyWordsAtom::~PVA_FF_AssetInfoKeyWordsAtom()
{
    if (_pKeyWordVect != NULL)
    {
        for (uint32 idx = 0; idx < _pKeyWordVect->size();idx++)
        {
            if ((*_pKeyWordVect)[idx] != NULL)
            {
                PV_MP4_FF_DELETE(NULL, PVA_FF_AssetInfoKeyWordStruct, (*_pKeyWordVect)[idx]);
                (*_pKeyWordVect)[idx] = NULL;
            }
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_AssetInfoKeyWordStructVecType, Oscl_Vector, _pKeyWordVect);
        _pKeyWordVect = NULL;
    }

}
PVA_FF_AssetInfoKeyWordStruct::PVA_FF_AssetInfoKeyWordStruct(
    uint8 aKeyWordBinarySize,
    PVA_FF_UNICODE_HEAP_STRING aKeyWordInfo)

{
    OSCL_UNUSED_ARG(aKeyWordBinarySize);
    _byteOrderMask = BYTE_ORDER_MASK;
    _keyWordInfo	= aKeyWordInfo;
    _keyWordSize = ((_keyWordInfo.get_size() + 1) * 2) + sizeof(_byteOrderMask);;// 1 for the NULL entry

}

PVA_FF_AssetInfoLocationInfoAtom::PVA_FF_AssetInfoLocationInfoAtom()
        : PVA_FF_FullAtom(ASSET_INFO_LOCINFO_ATOM, 0, 0)
{
    _locationInfoRole		= 0;
    _locationInfoLongitude	= 0;
    _locationInfoLatitude	= 0;
    _locationInfoAltitude	= 0;
    _locationInfoName	    = (_STRLIT(""));
    _locationInfoAstrBody    = (_STRLIT(""));
    _locationInfoAddNotes    = (_STRLIT(""));
    _byteOrderMask = BYTE_ORDER_MASK;
    _langCode = LANGUAGE_CODE_UNKNOWN;
    recomputeSize();
}

void PVA_FF_AssetInfoTitleAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_title.get_size() + 1);
    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool
PVA_FF_AssetInfoTitleAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _title))
    {
        return false;
    }
    rendered += _title.get_size() + 1; // 1 for the NULL entry
    return true;

}

void PVA_FF_AssetInfoDescAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_description.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoDescAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)

{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _description))
    {
        return false;
    }
    rendered += _description.get_size() + 1; // 1 for the NULL entry
    return true;
}

void PVA_FF_AssetInfoCopyRightAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_cprt.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoCopyRightAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)

{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _cprt))
    {
        return false;
    }
    rendered += _cprt.get_size() + 1; // 1 for the NULL entry
    return true;
}

void PVA_FF_AssetInfoPerformerAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_performer.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoPerformerAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _performer))
    {
        return false;
    }
    rendered += _performer.get_size() + 1; // 1 for the NULL entry
    return true;
}

void PVA_FF_AssetInfoAuthorAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_author.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoAuthorAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _author))
    {
        return false;
    }
    rendered += _author.get_size() + 1; // 1 for the NULL entry
    return true;
}

void PVA_FF_AssetInfoGenreAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_genre.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoGenreAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _genre))
    {
        return false;
    }
    rendered += _genre.get_size() + 1; // 1 for the NULL entry
    return true;
}


void PVA_FF_AssetInfoRatingAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += sizeof(_ratingEntity);
    _size += sizeof(_ratingCriteria);
    _size += 2 * (_ratingInfo.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoRatingAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)

{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _ratingEntity))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _ratingCriteria))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _ratingInfo))
    {
        return false;
    }
    rendered += _ratingInfo.get_size() + 1; // 1 for the NULL entry
    return true;
}

void PVA_FF_AssetInfoClassificationAtom::recomputeSize()
{
    _size =	getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_classificationEntity);
    _size += sizeof(_classificationTable);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_classificationInfo.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoClassificationAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)

{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _classificationEntity))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render16(fp, _classificationTable))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _classificationInfo))
    {
        return false;
    }
    rendered += _classificationInfo.get_size() + 1; // 1 for the NULL entry
    return true;
}


uint32 PVA_FF_AssetInfoKeyWordStruct::getSizeofStruct()
{

    uint32 size = ((_keyWordInfo.get_size() + 1) * 2);// 1 for the NULL entry

    size += sizeof(_keyWordSize);
    size += sizeof(_byteOrderMask);
    return size;
}


void PVA_FF_AssetInfoKeyWordsAtom::recomputeSize()
{
    _size = getDefaultSize();

    _size += sizeof(_langCode);
    _size += sizeof(_keyWordCnt);

    for (uint32 i = 0; i < _pKeyWordVect->size(); i++)
    {
        _size += (*_pKeyWordVect)[i]->getSizeofStruct();
    }
    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


void PVA_FF_AssetInfoKeyWordsAtom::setKeyWord(uint32 aKeyWordBinarySize,
        PVA_FF_UNICODE_HEAP_STRING aKeyWord)
{
    PVA_FF_AssetInfoKeyWordStruct* keyWordStruct = NULL;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoKeyWordStruct, (aKeyWordBinarySize, aKeyWord), keyWordStruct);

    _pKeyWordVect->push_back(keyWordStruct);
    _keyWordCnt++;
    recomputeSize();
}



bool PVA_FF_AssetInfoKeyWordStruct::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    if (!PVA_FF_AtomUtils::render8(fp, _keyWordSize))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _keyWordInfo))
    {
        return false;
    }
    rendered += _keyWordInfo.get_size() + 1; // 1 for the NULL entry
    return true;
}

bool PVA_FF_AssetInfoKeyWordsAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)

{
    recomputeSize();
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;


    if (!PVA_FF_AtomUtils::render8(fp, _keyWordCnt))
    {
        return false;
    }
    rendered += 1;
    if (_pKeyWordVect->size() < _keyWordCnt)
    {
        return false;
    }

    // calculate size of each object in the structure
    for (int i = 0; i < _keyWordCnt; i++)
    {
        (*_pKeyWordVect)[i]->renderToFileStream(fp);
        rendered += (*_pKeyWordVect)[i]->getSizeofStruct();

    }
    return true;
}

void PVA_FF_AssetInfoLocationInfoAtom::recomputeSize()
{
    _size =	getDefaultSize();
    _size += sizeof(_langCode);
    _size += sizeof(_locationInfoRole);
    _size += sizeof(_locationInfoLongitude);
    _size += sizeof(_locationInfoLatitude);
    _size += sizeof(_locationInfoAltitude);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_locationInfoName.get_size() + 1);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_locationInfoAstrBody.get_size() + 1);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_locationInfoAddNotes.get_size() + 1);

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool PVA_FF_AssetInfoLocationInfoAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    recomputeSize();

    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _locationInfoName))
    {
        return false;
    }
    rendered += _locationInfoName.get_size() + 1; // 1 for the NULL entry


    if (!PVA_FF_AtomUtils::render8(fp, _locationInfoRole))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render32(fp, _locationInfoLongitude))
    {
        return false;
    }
    rendered += 4;


    if (!PVA_FF_AtomUtils::render32(fp, _locationInfoLatitude))
    {
        return false;
    }
    rendered += 4;


    if (!PVA_FF_AtomUtils::render32(fp, _locationInfoAltitude))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _locationInfoAstrBody))
    {
        return false;
    }
    rendered += _locationInfoAstrBody.get_size() + 1; // 1 for the NULL entry

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _locationInfoAddNotes))
    {
        return false;
    }
    rendered += _locationInfoAddNotes.get_size() + 1; // 1 for the NULL entry
    return true;
}


void PVA_FF_AssetInfoAlbumAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += sizeof(_langCode);
    _size += sizeof(_byteOrderMask);
    _size += 2 * (_albumTitle.get_size() + 1);
    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool
PVA_FF_AssetInfoAlbumAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    //recomputeSize();
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _langCode))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render16(fp, _byteOrderMask))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(fp, _albumTitle))
    {
        return false;
    }
    rendered += _albumTitle.get_size() + 1; // 1 for the NULL entry
    //recomputeSize();
    return true;

}

void PVA_FF_AssetInfoRecordingYearAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += sizeof(_recordingYear);
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

bool
PVA_FF_AssetInfoRecordingYearAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    //recomputeSize();
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _recordingYear))
    {
        return false;
    }
    rendered += 2;
    return true;

}

