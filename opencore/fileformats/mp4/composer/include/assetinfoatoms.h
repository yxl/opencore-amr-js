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
#ifndef A_ISUCCEEDFAIL_H_INCLUDED
#include "a_isucceedfail.h"
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

#define BYTE_ORDER_MASK 0xFEFF

class PVA_FF_AssetInfoTitleAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoTitleAtom();
        virtual ~PVA_FF_AssetInfoTitleAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets
        void setTitleLangCode(const uint16  aLangCode)
        {
            _langCode = aLangCode;
        }
        void setTitleNotice(const PVA_FF_UNICODE_HEAP_STRING aTitle)
        {
            _title = aTitle;
            recomputeSize();
        }
        virtual	void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _title;
};

class PVA_FF_AssetInfoDescAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoDescAtom();
        virtual ~PVA_FF_AssetInfoDescAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets
        void setDescLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }
        void setDescNotice(PVA_FF_UNICODE_HEAP_STRING aDescription)
        {
            _description = aDescription;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _description;
};

class PVA_FF_AssetInfoCopyRightAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoCopyRightAtom();
        virtual ~PVA_FF_AssetInfoCopyRightAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets
        void setCopyRightLangCode(const uint16  aLangCode)
        {
            _langCode = aLangCode;
        }
        void setCopyRightString(const PVA_FF_UNICODE_HEAP_STRING aCprt)
        {
            _cprt = aCprt;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _cprt;
};

class PVA_FF_AssetInfoPerformerAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoPerformerAtom();
        virtual ~PVA_FF_AssetInfoPerformerAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setPerfLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }
        void setPerfNotice(PVA_FF_UNICODE_HEAP_STRING aPerformer)
        {
            _performer = aPerformer;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _performer;
};

class PVA_FF_AssetInfoAuthorAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoAuthorAtom();
        virtual ~PVA_FF_AssetInfoAuthorAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setAuthorLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }
        void setAuthorNotice(PVA_FF_UNICODE_HEAP_STRING aAuthor)
        {
            _author = aAuthor;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _author;
};

class PVA_FF_AssetInfoGenreAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoGenreAtom();
        virtual ~PVA_FF_AssetInfoGenreAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setGenreLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }
        void setGenreNotice(PVA_FF_UNICODE_HEAP_STRING aGenre)
        {
            _genre = aGenre;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _genre;

};

class PVA_FF_AssetInfoRatingAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoRatingAtom();
        virtual ~PVA_FF_AssetInfoRatingAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setRatingLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }

        void setRatingEntity(uint32 aRatingEntity)
        {
            _ratingEntity = aRatingEntity;
        }

        void setRatingCriteria(uint32 aRatingCriteria)
        {
            _ratingCriteria = aRatingCriteria;
        }

        void setRatingInfoNotice(PVA_FF_UNICODE_HEAP_STRING aRatingInfo)
        {
            _ratingInfo = aRatingInfo;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        uint32 _ratingEntity;
        uint32 _ratingCriteria;
        PVA_FF_UNICODE_HEAP_STRING _ratingInfo;
};

class PVA_FF_AssetInfoClassificationAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoClassificationAtom();
        virtual ~PVA_FF_AssetInfoClassificationAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setClassificationLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }

        void setClassificationEntity(uint32 aClassificationEntity)
        {
            _classificationEntity = aClassificationEntity;
        }


        void setClassificationTable(uint32 aClassificationTable)
        {
            _classificationTable = aClassificationTable;
        }

        void setClassificationInfoNotice(PVA_FF_UNICODE_HEAP_STRING aClassificationInfo)
        {
            _classificationInfo = aClassificationInfo;
            recomputeSize();
        }

        virtual	void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        uint16 _classificationTable;
        uint32 _classificationEntity;

        PVA_FF_UNICODE_HEAP_STRING _classificationInfo;

};

class PVA_FF_AssetInfoKeyWordStruct
{

    public:
        PVA_FF_AssetInfoKeyWordStruct() {};
        PVA_FF_AssetInfoKeyWordStruct(uint8 , PVA_FF_UNICODE_HEAP_STRING);
        virtual ~PVA_FF_AssetInfoKeyWordStruct() {};
        uint32 getSizeofStruct();
        bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        uint8 _keyWordSize;
        PVA_FF_UNICODE_HEAP_STRING _keyWordInfo;
        uint16  _byteOrderMask;

};

class PVA_FF_AssetInfoKeyWordsAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoKeyWordsAtom();
        virtual ~PVA_FF_AssetInfoKeyWordsAtom();//{};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets

        void setKeyWordsLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }

        void setKeyWord(uint32 aKeyWordBinarySize,
                        PVA_FF_UNICODE_HEAP_STRING aKeyWord);
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint8 _keyWordCnt;
        PVA_FF_AssetInfoKeyWordStruct PVA_FF_KeyWordStruct;
        Oscl_Vector<PVA_FF_AssetInfoKeyWordStruct*, OsclMemAllocator> *_pKeyWordVect;

};

class PVA_FF_AssetInfoLocationInfoAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoLocationInfoAtom();
        virtual ~PVA_FF_AssetInfoLocationInfoAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        // Member gets
        void setLocationInfoLangCode(uint16 aLangCode)
        {
            _langCode = aLangCode;
        }

        void setLocationInfoRole(uint8 aLocationInfoRole)
        {
            _locationInfoRole = aLocationInfoRole;
        }

        void setLocationInfoLongitude(uint32 _aLocationInfoLongitude)
        {
            _locationInfoLongitude = _aLocationInfoLongitude;
        }

        void setLocationInfoLatitude(uint32 _aLocationInfoLatitude)
        {
            _locationInfoLatitude = _aLocationInfoLatitude;
        }

        void setLocationInfoAltitude(uint32 _aLocationInfoAltitude)
        {
            _locationInfoAltitude = _aLocationInfoAltitude;
        }

        void setLocationInfoName(PVA_FF_UNICODE_HEAP_STRING aLocationInfoName)
        {
            _locationInfoName = aLocationInfoName;
            recomputeSize();
        }

        void setLocationInfoAstrBody(PVA_FF_UNICODE_HEAP_STRING aLocationInfoAstrBody)
        {
            _locationInfoAstrBody = aLocationInfoAstrBody;
            recomputeSize();
        }

        void setLocationInfoAddNotes(PVA_FF_UNICODE_HEAP_STRING aLocationInfoAddNotes)
        {
            _locationInfoAddNotes = aLocationInfoAddNotes;
            recomputeSize();
        }
        virtual void recomputeSize();
    private:
        uint16 _langCode;
        uint8  _locationInfoRole;

        uint32 _locationInfoLongitude;
        uint32 _locationInfoLatitude;
        uint32 _locationInfoAltitude;

        PVA_FF_UNICODE_HEAP_STRING _locationInfoName;
        PVA_FF_UNICODE_HEAP_STRING _locationInfoAstrBody;
        PVA_FF_UNICODE_HEAP_STRING _locationInfoAddNotes;
        uint16  _byteOrderMask;

};

class PVA_FF_AssetInfoAlbumAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoAlbumAtom();
        virtual ~PVA_FF_AssetInfoAlbumAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets
        void setAlbumLangCode(uint16  aLangCode)
        {
            _langCode = aLangCode;
        }
        void setAlbumInfo(PVA_FF_UNICODE_HEAP_STRING aAlbumTitle)
        {
            _albumTitle = aAlbumTitle;
            recomputeSize();
        }
        virtual	void recomputeSize();
    private:
        uint16 _langCode;
        uint16  _byteOrderMask;
        PVA_FF_UNICODE_HEAP_STRING _albumTitle;
};

class PVA_FF_AssetInfoRecordingYearAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_AssetInfoRecordingYearAtom();
        virtual ~PVA_FF_AssetInfoRecordingYearAtom() {};
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets
        void setRecordingYear(uint16 aRecordingYear)
        {
            _recordingYear = aRecordingYear;
            recomputeSize();
        }
        virtual	void recomputeSize();
    private:
        uint16 _recordingYear;
};

#endif  // ASSETINFOATOMS_H_INCLUDED
