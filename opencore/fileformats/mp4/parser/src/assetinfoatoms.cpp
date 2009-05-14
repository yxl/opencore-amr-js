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
/*                          MPEG-4 UserDataAtom Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This UserDataAtom Class is a container atom for informative user-data.
*/


#define __IMPLEMENT_AssetInfoAtoms__

#include "assetinfoatoms.h"
#include "atomdefs.h"
#include "oscl_utf8conv.h"
#include "oscl_snprintf.h"

#define UInt32ToFourChar(num, characters) \
{ \
	characters[3] = (num & 0xFF); \
	characters[2] = (num & 0xFF00)>>8; \
	characters[1] = (num & 0xFF0000)>>16; \
	characters[0] = (num & 0xFF000000)>>24; \
}

typedef Oscl_Vector<AssestInfoKeyWord*, OsclMemAllocator> assestInfoKeyWordVecType;

AssestInfoBaseParser::AssestInfoBaseParser(MP4_FF_FILE *fp,
        uint32 size,
        uint32 sizeofDataFieldBeforeString)
{
    _success = true;
    _charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (_success)
    {
        uint32 _count = 0;

        if (sizeofDataFieldBeforeString == 2)
        {
            if (!AtomUtils::read16(fp, _dataPriorToString))
            {
                _success = false;
                return;
            }
            _count += 2;
        }
        else if (sizeofDataFieldBeforeString == 1)
        {
            uint8 data8;

            if (!AtomUtils::read8(fp, data8))
            {
                _success = false;
                return;
            }
            _count += 1;
            _dataPriorToString = (uint16)(data8);
        }
        else
        {
            //error
            _success = false;
            return;
        }

        uint32 delta = (size - _count);

        if (delta > 0)
        {
            if (!AtomUtils::readString(fp, delta, _charType , _infoNotice))
            {
                //error
                _success = false;
                return;
            }
            _count += delta;
        }
        else
        {
            _charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
            _infoNotice = NULL;
        }

        if (_count < size)
            AtomUtils::seekFromCurrPos(fp, (size - _count));
    }
}

AssetInfoTitleAtom::AssetInfoTitleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;
    if (_success)
    {
        uint32 count = getDefaultSize();

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_TITL_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_TITL_FAILED;
        }
    }
}

AssetInfoTitleAtom::~AssetInfoTitleAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);

    }
}

AssetInfoDescAtom::AssetInfoDescAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;
    if (_success)
    {
        uint32 count = getDefaultSize();

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_DSCP_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_DSCP_FAILED;
        }
    }
}

AssetInfoDescAtom::~AssetInfoDescAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
}

AssetInfoPerformerAtom::AssetInfoPerformerAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;

    if (_success)
    {
        uint32 count = getDefaultSize();

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_PERF_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_PERF_FAILED;
        }
    }
}

AssetInfoPerformerAtom::~AssetInfoPerformerAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
}

AssetInfoAuthorAtom::AssetInfoAuthorAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;

    if (_success)
    {
        uint32 count = getDefaultSize();

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_AUTH_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_AUTH_FAILED;
        }
    }
}

AssetInfoAuthorAtom::~AssetInfoAuthorAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
}

AssetInfoGenreAtom::AssetInfoGenreAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;

    if (_success)
    {
        uint32 count = getDefaultSize();

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_GNRE_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_GNRE_FAILED;
        }
    }
}

AssetInfoGenreAtom::~AssetInfoGenreAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
}

AssetInfoRatingAtom::AssetInfoRatingAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;

    if (_success)
    {
        uint32 count = getDefaultSize();

        if (!AtomUtils::read32(fp, _ratingEntity))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_RTNG_FAILED;
            return;
        }
        count += 4;

        if (!AtomUtils::read32(fp, _ratingCriteria))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_RTNG_FAILED;
            return;
        }
        count += 4;

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);
        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_RTNG_FAILED;
            return;
        }
        OSCL_wHeapString<OsclMemAllocator> addnlratingInfo;

        addnlratingInfo += _STRLIT_WCHAR(";rating-criteria=");
        char Criteria[4];
        oscl_memset(Criteria, 0, 4*sizeof(char));
        UInt32ToFourChar(_ratingCriteria, Criteria);
        oscl_wchar wCriteria[5];
        oscl_UTF8ToUnicode(Criteria, 4, wCriteria, 5);
        addnlratingInfo += wCriteria;

        addnlratingInfo += _STRLIT_WCHAR(";rating-entity=");
        char Entity[4];
        oscl_memset(Entity, 0, 4);
        UInt32ToFourChar(_ratingEntity, Entity);
        oscl_wchar wEntity[5];
        oscl_UTF8ToUnicode(Entity, 4, wEntity, 5);
        addnlratingInfo += wEntity;

        _pAssetInfoBaseParser->updateInfoNotice(addnlratingInfo);
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_RTNG_FAILED;
        }
    }
}

AssetInfoRatingAtom::~AssetInfoRatingAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
}

AssetInfoClassificationAtom::AssetInfoClassificationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pAssetInfoBaseParser = NULL;

    if (_success)
    {
        uint32 count = getDefaultSize();

        if (!AtomUtils::read32(fp, _classificationEntity))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_CLSF_FAILED;
            return;
        }
        count += 4;

        if (!AtomUtils::read16(fp, _classificationTable))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_CLSF_FAILED;
            return;
        }
        count += 2;

        PV_MP4_FF_NEW(fp->auditCB, AssestInfoBaseParser, (fp, (_size - count)), _pAssetInfoBaseParser);

        if (!(_pAssetInfoBaseParser->GetMP4Success()))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_CLSF_FAILED;
            return;
        }
        OSCL_wHeapString<OsclMemAllocator> addnlclassificationInfo;

        addnlclassificationInfo += _STRLIT_WCHAR(";classification-table=");
        char Table[4];
        oscl_memset(Table, 0, 4);
        oscl_snprintf(Table, 4, _STRLIT_CHAR("%d"), _classificationTable);
        oscl_wchar wTable[5];
        oscl_UTF8ToUnicode(Table, 4, wTable, 5);
        addnlclassificationInfo += wTable;

        addnlclassificationInfo += _STRLIT_WCHAR(";classification-entity=");
        char Entity[4];
        oscl_memset(Entity, 0, 4);
        UInt32ToFourChar(_classificationEntity, Entity);
        oscl_wchar wEntity[5];
        oscl_UTF8ToUnicode(Entity, 4, wEntity, 5);
        addnlclassificationInfo += wEntity;

        _pAssetInfoBaseParser->updateInfoNotice(addnlclassificationInfo);
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_CLSF_FAILED;
        }
    }
}

AssetInfoClassificationAtom::~AssetInfoClassificationAtom()
{
    if (_pAssetInfoBaseParser != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoBaseParser, _pAssetInfoBaseParser);
    }
};


AssestInfoKeyWord::AssestInfoKeyWord(MP4_FF_FILE *fp)
{
    int32 currfilePos = AtomUtils::getCurrentFilePosition(fp);

    uint8 keywordSize = 0;
    if (!AtomUtils::read8(fp, keywordSize))
    {
        return;
    }
    uint32 temp = AtomUtils::peekNextNthBytes(fp, 1);

    uint16 byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);


    if (byteOrderMask == BYTE_ORDER_MASK)
    {
        _charType = ORIGINAL_CHAR_TYPE_UTF16;
        if (!AtomUtils::read16(fp, byteOrderMask))
        {
            return;
        }

        // Check to see if the string is actually null-terminated
        if (!AtomUtils::readNullTerminatedUnicodeString(fp, _defaultKeyWord))
        {
            return;
        }
    }
    else
    {
        _charType = ORIGINAL_CHAR_TYPE_UTF8;
        // Check to see if the string is actually null-terminated

        if (!AtomUtils::readNullTerminatedString(fp, _defaultKeyWord))
        {
            return;
        }
    }
    int32 newfilePos = AtomUtils::getCurrentFilePosition(fp);
    size = newfilePos - currfilePos;
}


AssetInfoKeyWordAtom::AssetInfoKeyWordAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)

{
    _pAssetInfoKeyWordVec = NULL;

    PV_MP4_FF_NEW(fp->auditCB, assestInfoKeyWordVecType, (), _pAssetInfoKeyWordVec);

    if (_success)
    {
        uint32 count = getDefaultSize();

        if (!AtomUtils::read16(fp, _langCode))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_KYWD_FAILED;
            return;
        }
        count += 2;

        if (!AtomUtils::read8(fp, _keyWordCount))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_KYWD_FAILED;
            return;
        }
        count += 1;

        for (uint8 i = 0; i < _keyWordCount; i++)
        {
            AssestInfoKeyWord * pAssetInfoKeyWord = NULL;

            PV_MP4_FF_NEW(fp->auditCB, AssestInfoKeyWord, (fp), pAssetInfoKeyWord);

            if (pAssetInfoKeyWord == NULL)
            {
                _success = false;
                _mp4ErrorCode = READ_UDTA_KYWD_FAILED;
                return;
            }
            (*_pAssetInfoKeyWordVec).push_back(pAssetInfoKeyWord);
            count += pAssetInfoKeyWord->size;
        }
        if (count < size)
        {
            int32 tmp = size - count;
            AtomUtils::seekFromCurrPos(fp, tmp);
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_UDTA_KYWD_FAILED;
        }
    }
}

AssetInfoKeyWordAtom::~AssetInfoKeyWordAtom()
{
    for (uint32 i = 0; i < _pAssetInfoKeyWordVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, AssestInfoKeyWord, (*_pAssetInfoKeyWordVec)[i]);
        (*_pAssetInfoKeyWordVec)[i] = NULL;
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, assestInfoKeyWordVecType, Oscl_Vector, _pAssetInfoKeyWordVec);
};

OSCL_wString& AssetInfoKeyWordAtom::getKeyWordAt(int32 index)
{
    MP4FFParserOriginalCharEnc charType;
    if ((_pAssetInfoKeyWordVec->size() == 0) ||
            ((uint32)index >= (_pAssetInfoKeyWordVec->size())))
    {
        return _defaultKeyWord;
    }
    else
    {
        return ((*_pAssetInfoKeyWordVec)[index])->getInfoNotice(charType);
    }
}

AssetInfoLocationAtom::AssetInfoLocationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pLocationStruct = NULL;

    int32 count = getSize() - getDefaultSize();

    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, PvmfAssetInfo3GPPLocationStruct, (), _pLocationStruct);

        if (!AtomUtils::read16(fp, _langCode))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }

        count -= 2;

        _pLocationStruct->_langCode = _langCode;

        uint32 temp = AtomUtils::peekNextNthBytes(fp, 1);

        uint16 byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);

        if (byteOrderMask == BYTE_ORDER_MASK)
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF16;
            if (!AtomUtils::read16(fp, byteOrderMask))
            {
                _success = false;
                return;
            }

            count -= 2;
            if (!AtomUtils::readNullTerminatedUnicodeString(fp, _defaultNotice))
            {
                _success = false;
                return;
            }
            count -= (_defaultNotice.get_size() + 1) * 2;
        }
        else
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF8;
            // Check to see if the string is actually null-terminated
            if (!AtomUtils::readNullTerminatedString(fp, _defaultNotice))
            {
                _success = false;
                return;
            }
            count -= _defaultNotice.get_size() + 1;
        }

        PV_MP4_FF_ARRAY_NEW(NULL, oscl_wchar, _defaultNotice.get_size() + 1, _pLocationStruct->_location_name);
        oscl_strncpy(_pLocationStruct->_location_name, _defaultNotice.get_str(), _defaultNotice.get_size());
        _pLocationStruct->_location_name[_defaultNotice.get_size()] = 0;

        if (_defaultNotice.get_size() > size)
        {
            _pLocationStruct->_location_name = NULL;
            _pLocationStruct->_role = 0;
            _pLocationStruct->_longitude = 0;
            _pLocationStruct->_latitude = 0;
            _pLocationStruct->_altitude = 0;
            _pLocationStruct->_additional_notes = NULL;
            _pLocationStruct->_astronomical_body = NULL;
            if (count > 0)
                AtomUtils::seekFromCurrPos(fp, count);
            return;
        }

        if (!AtomUtils::read8(fp, _role))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
        _pLocationStruct->_role = _role;
        count -= 1;

        if (!AtomUtils::read32(fp, _longitude))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
        _pLocationStruct->_longitude = _longitude ;
        count -= 4;

        if (!AtomUtils::read32(fp, _latitude))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
        _pLocationStruct->_latitude = _latitude;
        count -= 4;

        if (!AtomUtils::read32(fp, _altitude))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
        _pLocationStruct->_altitude = _altitude;
        count -= 4;

        temp = AtomUtils::peekNextNthBytes(fp, 1);

        byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);

        if (byteOrderMask == BYTE_ORDER_MASK)
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF16;
            if (!AtomUtils::read16(fp, byteOrderMask))
            {
                _success = false;
                return;
            }
            count -= 2;

            if (!AtomUtils::readNullTerminatedUnicodeString(fp, _astronomical_body))
            {
                _success = false;
                return;
            }
            count -= (_astronomical_body.get_size() + 1) * 2;
        }
        else
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF8;
            // Check to see if the string is actually null-terminated
            if (!AtomUtils::readNullTerminatedString(fp, _astronomical_body))
            {
                _success = false;
                return;
            }
            count -= _astronomical_body.get_size() + 1;
        }
        PV_MP4_FF_ARRAY_NEW(NULL, oscl_wchar, _astronomical_body.get_size() + 1, _pLocationStruct->_astronomical_body);
        oscl_strncpy(_pLocationStruct->_astronomical_body, _astronomical_body.get_str(), _astronomical_body.get_size());
        _pLocationStruct->_astronomical_body[_astronomical_body.get_size()] = 0;

        if (_astronomical_body.get_size() > size)
        {
            _pLocationStruct->_additional_notes = NULL;
            _pLocationStruct->_astronomical_body = NULL;
            if (count > 0)
                AtomUtils::seekFromCurrPos(fp, count);

            return;
        }

        temp = AtomUtils::peekNextNthBytes(fp, 1);

        byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);

        if (byteOrderMask == BYTE_ORDER_MASK)
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF16;
            if (!AtomUtils::read16(fp, byteOrderMask))
            {
                _success = false;
                return;
            }
            count -= 2;

            if (!AtomUtils::readNullTerminatedUnicodeString(fp, _additional_notes))
            {
                _success = false;
                return;
            }
            count -= (_additional_notes.get_size() + 1) * 2;
        }
        else
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF8;
            // Check to see if the string is actually null-terminated
            if (!AtomUtils::readNullTerminatedString(fp, _additional_notes))
            {
                _success = false;
                return;
            }
            count -= _additional_notes.get_size() + 1;
        }
        PV_MP4_FF_ARRAY_NEW(NULL, oscl_wchar, _additional_notes.get_size() + 1, _pLocationStruct->_additional_notes);
        oscl_strncpy(_pLocationStruct->_additional_notes, _additional_notes.get_str(), _additional_notes.get_size());
        _pLocationStruct->_additional_notes[_additional_notes.get_size()] = 0;

        if (_additional_notes.get_size() > size)
        {
            _pLocationStruct->_additional_notes = NULL;
            if (count > 0)
                AtomUtils::seekFromCurrPos(fp, count);

            return;
        }
    }
    if (count > 0)
        AtomUtils::seekFromCurrPos(fp, count);
}

AssetInfoLocationAtom::~AssetInfoLocationAtom()
{
    if (_pLocationStruct != NULL)
    {
        if (_pLocationStruct->_location_name != NULL)
        {
            PV_MP4_FF_DELETE(NULL, oscl_wchar, _pLocationStruct->_location_name);
            _pLocationStruct->_location_name = NULL;
        }
        if (_pLocationStruct->_astronomical_body != NULL)
        {
            PV_MP4_FF_DELETE(NULL, oscl_wchar, _pLocationStruct->_astronomical_body);
            _pLocationStruct->_astronomical_body = NULL;
        }
        if (_pLocationStruct->_additional_notes != NULL)
        {
            PV_MP4_FF_DELETE(NULL, oscl_wchar, _pLocationStruct->_additional_notes);
            _pLocationStruct->_additional_notes = NULL;
        }
    }
    PV_MP4_FF_DELETE(NULL, PvmfAssetInfo3GPPLocationStruct, _pLocationStruct);
    _pLocationStruct = NULL;
}

AssetInfoAlbumAtom::AssetInfoAlbumAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    int32 count = getSize() - getDefaultSize();
    if (_success)
    {
        if (!AtomUtils::read16(fp, _langCode))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
        count -= 2;

        uint32 temp = AtomUtils::peekNextNthBytes(fp, 1);

        uint16 byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);

        if (byteOrderMask == BYTE_ORDER_MASK)
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF16;
            if (!AtomUtils::read16(fp, byteOrderMask))
            {
                _success = false;
                return;
            }
            count -= 2;

            if (!AtomUtils::readNullTerminatedUnicodeString(fp, _defaultNotice))
            {
                _success = false;
                return;
            }
            count -= (_defaultNotice.get_size() + 1) * 2;
        }
        else
        {
            _charType = ORIGINAL_CHAR_TYPE_UTF8;
            // Check to see if the string is actually null-terminated
            if (!AtomUtils::readNullTerminatedString(fp, _defaultNotice))
            {
                _success = false;
                return;
            }
            count -= _defaultNotice.get_size() + 1;
        }
        if (_defaultNotice.get_size() > size)
        {
            _defaultNotice = NULL;
            _trackNumber = 0;

            if (count > 0)
                AtomUtils::seekFromCurrPos(fp, count);

            return;
        }
        if (count > 0)
        {
            if (!AtomUtils::read8(fp, _trackNumber))
            {
                _success = false;
                _mp4ErrorCode = READ_UDTA_LOC_FAILED;
                return;
            }
            count -= 1;
        }
    }
    if (count > 0)
        AtomUtils::seekFromCurrPos(fp, count);

}

AssetInfoRecordingYearAtom::AssetInfoRecordingYearAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    if (_success)
    {
        if (!AtomUtils::read16(fp, _recordingYear))
        {
            _success = false;
            _mp4ErrorCode = READ_UDTA_LOC_FAILED;
            return;
        }
    }
}

