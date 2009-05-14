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
#ifndef ASSETINFOATOMS_H_INCLUDED
#define ASSETINFOATOMS_H_INCLUDED

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif
#ifndef ISUCCEEDFAIL_H_INCLUDED
#include "isucceedfail.h"
#endif
#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif
#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

class AssestInfoBaseParser
{
    public:
        AssestInfoBaseParser(MP4_FF_FILE *fp,
                             uint32 size,
                             uint32 sizeofDataFieldBeforeString = 2);

        virtual ~AssestInfoBaseParser() {};

        uint16 getLangCode() const
        {
            return _dataPriorToString;
        }
        OSCL_wString& getInfoNotice(MP4FFParserOriginalCharEnc &charType)
        {
            charType = _charType;
            return _infoNotice;
        }
        void updateInfoNotice(OSCL_wString& aInfo)
        {
            _infoNotice += aInfo;
        }
        bool GetMP4Success()
        {
            return _success;
        }

    private:
        bool        _success;
        uint16      _dataPriorToString;
        OSCL_wHeapString<OsclMemAllocator> _infoNotice;
        MP4FFParserOriginalCharEnc _charType;
};

class AssetInfoTitleAtom : public FullAtom
{

    public:
        AssetInfoTitleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoTitleAtom();

        // Member gets
        uint16 getTitleLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getTitleNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultTitle;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultTitle;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

typedef Oscl_Vector<AssestInfoBaseParser*, OsclMemAllocator> assestInfoBaseParserVecType;

class AssetInfoDescAtom : public FullAtom
{

    public:
        AssetInfoDescAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoDescAtom();

        // Member gets
        uint16 getDescLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getDescNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultDesc;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultDesc;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssetInfoPerformerAtom : public FullAtom
{
    public:
        AssetInfoPerformerAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoPerformerAtom();

        // Member gets
        uint16 getPerfLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getPerfNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultPerf;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultPerf;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssetInfoAuthorAtom : public FullAtom
{
    public:
        AssetInfoAuthorAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoAuthorAtom();

        // Member gets
        uint16 getAuthorLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getAuthorNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultAuthor;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultAuthor;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssetInfoGenreAtom : public FullAtom
{
    public:
        AssetInfoGenreAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoGenreAtom();

        // Member gets
        uint16 getGenreLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getGenreNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultGenre;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultGenre;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssetInfoRatingAtom : public FullAtom
{
    public:
        AssetInfoRatingAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoRatingAtom();

        // Member gets
        uint32      getRatingEntity() const
        {
            return _ratingEntity;
        }

        uint32      getRatingCriteria() const
        {
            return _ratingCriteria;
        }

        uint16      getRatingLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getRatingNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultRating;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultRating;
        uint32				  _ratingEntity;
        uint32				  _ratingCriteria;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssetInfoClassificationAtom : public FullAtom
{
    public:
        AssetInfoClassificationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoClassificationAtom();

        // Member gets
        uint32      getClassificationEntity() const
        {
            return _classificationEntity;
        }
        uint16      getClassificationTable() const
        {
            return _classificationTable;
        }
        uint16      getClassificationLangCode() const
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getLangCode();
            }
            return 0;
        }
        OSCL_wString& getClassificationNotice(MP4FFParserOriginalCharEnc &charType)
        {
            if (_pAssetInfoBaseParser != NULL)
            {
                return _pAssetInfoBaseParser->getInfoNotice(charType);
            }
            return _defaultClassification;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultClassification;
        uint32				  _classificationEntity;
        uint16				  _classificationTable;
        AssestInfoBaseParser* _pAssetInfoBaseParser;
};

class AssestInfoKeyWord
{
    public:
        AssestInfoKeyWord(MP4_FF_FILE *fp);
        OSCL_wHeapString<OsclMemAllocator> _defaultKeyWord;
        MP4FFParserOriginalCharEnc _charType;
        uint32 count;
        uint32 size;

        OSCL_wString& getInfoNotice(MP4FFParserOriginalCharEnc &charType)
        {
            charType = _charType;
            return _defaultKeyWord;
        }
};

class AssetInfoKeyWordAtom : public FullAtom
{
    public:
        AssetInfoKeyWordAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);

        virtual ~AssetInfoKeyWordAtom();

        // Member gets
        uint16      getKeyWordLangCode() const
        {
            return _langCode;
        }

        uint32      getNumKeyWords() const
        {
            return _keyWordCount;
        }

        OSCL_wString& getKeyWordAt(int32 index);

    private:
        uint16				          _langCode;
        uint8						  _keyWordCount;
        Oscl_Vector<AssestInfoKeyWord *, OsclMemAllocator> *_pAssetInfoKeyWordVec;
        OSCL_wHeapString<OsclMemAllocator> _defaultKeyWord;
};

class AssetInfoLocationAtom : public FullAtom
{
    public:
        AssetInfoLocationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoLocationAtom();

        // Member gets
        PvmfAssetInfo3GPPLocationStruct* getAssetInfoLocationStruct()
        {
            return _pLocationStruct;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _defaultNotice;
        OSCL_wHeapString<OsclMemAllocator> _astronomical_body;
        OSCL_wHeapString<OsclMemAllocator> _additional_notes;

        uint8				  _role;
        uint32				  _longitude;
        uint32				  _latitude;
        uint32				  _altitude;
        MP4FFParserOriginalCharEnc _charType;
        uint16 _langCode;
        PvmfAssetInfo3GPPLocationStruct *_pLocationStruct;
};

class AssetInfoAlbumAtom : public FullAtom
{
    public:
        AssetInfoAlbumAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoAlbumAtom()
        {
        };

        // Member gets
        uint16      getAlbumLangCode() const
        {
            return _langCode;
        }
        uint8 getTrackNumber()
        {
            return _trackNumber;
        }
        OSCL_wString& getAlbumNotice(MP4FFParserOriginalCharEnc &charType)
        {
            charType = _charType;
            return _defaultNotice;
        }

    private:

        OSCL_wHeapString<OsclMemAllocator> _defaultNotice;
        uint8				  _trackNumber;
        uint16				  _langCode;
        MP4FFParserOriginalCharEnc _charType;
};


class AssetInfoRecordingYearAtom : public FullAtom
{
    public:
        AssetInfoRecordingYearAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AssetInfoRecordingYearAtom()
        {
        };

        // Member gets
        uint16 getRecordingYear()
        {
            return _recordingYear;
        }

    private:
        uint16 _recordingYear;
};



#endif  // ASSETINFOATOMS_H_INCLUDED

