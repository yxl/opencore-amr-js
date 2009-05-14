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
#ifndef ITUNESILSTATOM_H_INCLUDED
#define ITUNESILSTATOM_H_INCLUDED

#include "atom.h"
#include "fullatom.h"
#include "pvmi_kvp.h"
#include "atomdefs.h"

#define MAX_CD_IDENTIFIER_FREE_DATA_ATOM 16
#define ITUNES_MAX_COVER_IMAGE_SIZE (1024*1024) //1 meg
#define PREFIX_SIZE 16

//************************************MeaningAtom Class Starts  **********************************
class ItunesMeaningAtom : public FullAtom
{
    public:
        ItunesMeaningAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~ItunesMeaningAtom();
        //API to retrieve the Meaning String
        OSCL_wHeapString<OsclMemAllocator> getMeaningString() const
        {
            return _meaningString;
        }

    private:

        OSCL_wHeapString<OsclMemAllocator> _meaningString;


};

//************************************NameAtom Class Starts  **********************************

class ItunesNameAtom : public FullAtom
{
    public:
        ItunesNameAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~ItunesNameAtom();
        //API to retrieve the Name String
        OSCL_wHeapString<OsclMemAllocator> getNameString() const
        {
            return _nameString;
        }

    private:

        OSCL_wHeapString<OsclMemAllocator> _nameString;


};

//************************************BaseTypes for the MetaData **********************************
class ITunesMetaDataAtom: public Atom
{
    public:
        ITunesMetaDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~ITunesMetaDataAtom();

    protected:
        uint64		_prefix;
        uint64		INTEGER_PREFIX;
        uint64		STRING_PREFIX;
        uint64		IMAGE_PREFIX_PNG;
        uint64		IMAGE_PREFIX_JFIF;
        uint64		OTHER_PREFIX;
        PVLogger	*iLogger;
};


//************************************Title(Name) Class Starts  **********************************
class ITunesTitleAtom: public ITunesMetaDataAtom
{
    public:
        ITunesTitleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesTitleAtom();

        OSCL_wHeapString<OsclMemAllocator> getTitle() const
        {
            return _name;
        }

    private:
// Title of Song
        OSCL_wHeapString<OsclMemAllocator> _name;
};

//************************************ Track's Subtitle Class Starts  **********************************

class ITunesTrackSubTitleAtom: public ITunesMetaDataAtom
{
    public:
        ITunesTrackSubTitleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesTrackSubTitleAtom();

        OSCL_wHeapString<OsclMemAllocator> getTrackSubTitle() const
        {
            return _trackTitle;
        }

    private:
//SubTitle of the track
        OSCL_wHeapString<OsclMemAllocator> _trackTitle;
};


//************************************ Artist / Performer Class Starts  **********************************
class ITunesArtistAtom: public ITunesMetaDataAtom
{
    public:
        ITunesArtistAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesArtistAtom();

        OSCL_wHeapString<OsclMemAllocator> getArtist() const
        {
            return _artist;
        }

    private:
// Artist / Performer of Song
        OSCL_wHeapString<OsclMemAllocator> _artist;
};

//************************************AlbumArtist Class Starts  **********************************
class ITunesAlbumArtistAtom: public ITunesMetaDataAtom
{
    public:
        ITunesAlbumArtistAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesAlbumArtistAtom();

        OSCL_wHeapString<OsclMemAllocator> getAlbumArtist() const
        {
            return _albumArtist;
        }

    private:
// AlbumArtist
        OSCL_wHeapString<OsclMemAllocator> _albumArtist;
};

//************************************ Album Class Starts  **********************************
class ITunesAlbumAtom: public ITunesMetaDataAtom
{
    public:
        ITunesAlbumAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesAlbumAtom();

        OSCL_wHeapString<OsclMemAllocator> getAlbum() const
        {
            return _album;
        }

    private:
// Album of Song
        OSCL_wHeapString<OsclMemAllocator> _album;
};

//************************************ Genre Class Starts  **********************************
class ITunesGenreAtom: public ITunesMetaDataAtom
{
    public:
        ITunesGenreAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesGenreAtom();

        uint16 getGnreID() const
        {
            return _gnreID;
        }

        OSCL_wHeapString<OsclMemAllocator> getGnreString() const
        {
            return _gnreString;
        }

        //This function will tell the type of Genre-- TRUE- If it is Integer, FALSE- if it is String
        GnreVersion getGnreVersion() const
        {
            return _gnreVersion;
        }


    private:
// Genre of Song
        OSCL_wHeapString<OsclMemAllocator> _gnreString; // Customized String of Genre
        uint16	_gnreID; // Genre ID
        GnreVersion	_gnreVersion; // Whether gnre is String or Integer ID Number*/
};

//************************************ Day Class Starts  **********************************
class ITunesYearAtom: public ITunesMetaDataAtom
{
    public:
        ITunesYearAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesYearAtom();

        OSCL_wHeapString<OsclMemAllocator> getYear() const
        {
            return _day;
        }

    private:
// The 4-Digit Year when Song was recorded
        OSCL_wHeapString<OsclMemAllocator> _day;
};

//************************************ Tool Class Starts  **********************************
class ITunesToolAtom: public ITunesMetaDataAtom
{
    public:
        ITunesToolAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesToolAtom();

        OSCL_wHeapString<OsclMemAllocator> getTool() const
        {
            return _tool;
        }

    private:
        // Tool/Encoder of Song
        OSCL_wHeapString<OsclMemAllocator> _tool;
};

//************************************EncodedBy(Company/Person) Class Starts  **********************************
class ITunesEncodedByAtom: public ITunesMetaDataAtom
{
    public:
        ITunesEncodedByAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesEncodedByAtom();

        OSCL_wHeapString<OsclMemAllocator> getEncodedBy() const
        {
            return _encodedBy;
        }

    private:
        // Person or company that encoded the recording
        OSCL_wHeapString<OsclMemAllocator> _encodedBy;
};


//************************************ Writer Class Starts  **********************************
class ITunesWriterAtom: public ITunesMetaDataAtom
{
    public:
        ITunesWriterAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesWriterAtom();

        OSCL_wHeapString<OsclMemAllocator> getWriter() const
        {
            return _writer;
        }

    private:
        // Writer of Song
        OSCL_wHeapString<OsclMemAllocator> _writer;
};

//************************************ Group Class Starts  **********************************
class ITunesGroupAtom: public ITunesMetaDataAtom
{
    public:
        ITunesGroupAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesGroupAtom();

        OSCL_wHeapString<OsclMemAllocator> getGroup() const
        {
            return _group;
        }

    private:
        // Grouping data of Song
        OSCL_wHeapString<OsclMemAllocator> _group;
};

//************************************ Comment Class Starts  **********************************
class ITunesCommentAtom: public ITunesMetaDataAtom
{
    public:
        ITunesCommentAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesCommentAtom();

        OSCL_wHeapString<OsclMemAllocator> getComment() const
        {
            return _comment;
        }

    private:
        // Comment data of Song
        OSCL_wHeapString<OsclMemAllocator> _comment;
};

//************************************ Track Class Starts  **********************************
class ITunesTracktAtom: public ITunesMetaDataAtom
{
    public:
        ITunesTracktAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesTracktAtom();

        uint16 getThisTrackNo() const
        {
            return _thisTrackNo;
        }

        uint16 getTotalTracks() const
        {
            return _totalTracks;
        }

    private:
        // Track data of Song
        uint16 _thisTrackNo;
        uint16 _totalTracks;
};

//********************************* Compilation Part Class Starts  ********************************
class ITunesCompileAtom: public ITunesMetaDataAtom
{
    public:
        ITunesCompileAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesCompileAtom();

        bool IsCompilationPart() const
        {
            return _compilationPart;
        }

    private:
        // Whether this song is the part of Compilation or not.
        bool _compilationPart;
};

//********************************* Content Rating Class Starts  ********************************
class ITunesContentRatingAtom: public ITunesMetaDataAtom
{
    public:
        ITunesContentRatingAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesContentRatingAtom();

        bool IsContentRating() const
        {
            return _contentRating;
        }

    private:
        // Does song have explicit content?
        bool _contentRating;
};

//************************************ Tempo Class Starts  **********************************
class ITunesTempoAtom: public ITunesMetaDataAtom
{
    public:
        ITunesTempoAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesTempoAtom();

        uint16 getBeatsPerMinute() const
        {
            return _beatsPerMin;
        }

    private:
        // Beats Per Minute in the Song.
        uint16 _beatsPerMin;
};

//************************************ Copyright Class Starts  **********************************
class ITunesCopyrightAtom: public ITunesMetaDataAtom
{
    public:
        ITunesCopyrightAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesCopyrightAtom();

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            return _cprt;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _cprt;
};

//************************************ Description Class Starts  **********************************
class ITunesDescriptionAtom: public ITunesMetaDataAtom
{
    public:
        ITunesDescriptionAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesDescriptionAtom();

        OSCL_wHeapString<OsclMemAllocator>  getITunesDescription() const
        {
            return _desc;
        }

    private:

        OSCL_wHeapString<OsclMemAllocator> _desc;
};


//************************************ Disk Data Class Starts  **********************************
class ITunesDiskDatatAtom: public ITunesMetaDataAtom
{
    public:
        ITunesDiskDatatAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesDiskDatatAtom();

        uint16 getThisDiskNo() const
        {
            return _thisDiskNo;
        }

        uint16 getTotalDisks() const
        {
            return _totalDisks;
        }

    private:
        // Disk data of Song
        uint16 _thisDiskNo;
        uint16 _totalDisks;
};

//********************************* Free Form Data Class Starts  **********************************
class ITunesFreeFormDataAtom: public ITunesMetaDataAtom
{
    public:
        ITunesFreeFormDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesFreeFormDataAtom();

        OSCL_wHeapString<OsclMemAllocator> getString()
        {
            return _StringData;
        }
    private:
        OSCL_wHeapString<OsclMemAllocator> _StringData;
};

//************************************ Lyrics Class Starts  **********************************
class ITunesLyricsAtom: public ITunesMetaDataAtom
{
    public:
        ITunesLyricsAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesLyricsAtom();

        OSCL_wHeapString<OsclMemAllocator> getLyrics() const
        {
            return _lyrics;
        }

    private:
        // Lyrics of Song
        OSCL_wHeapString<OsclMemAllocator> _lyrics;
};

//************************************ Cover/Artwork Class Starts  **********************************
class ITunesCoverImageAtom: public ITunesMetaDataAtom
{
    public:
        ITunesCoverImageAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesCoverImageAtom();

        PvmfApicStruct* getImageData() const
        {
            if (_ImageData)
                return(_ImageData);
            else
                return(NULL);
        }

    private:
        // Artwork/Image of Song
        PvmfApicStruct* _ImageData;
};

//***************************** ILST Atom Class Starts    **************************
class ITunesILSTAtom : public Atom
{
    public:
        ITunesILSTAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~ITunesILSTAtom();

        OSCL_wHeapString<OsclMemAllocator> getTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesTitleAtom)
                return _pITunesTitleAtom->getTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getTrackSubTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesTrackSubTitleAtom)
                return _pITunesTrackSubTitleAtom->getTrackSubTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesArtistAtom)
                return _pITunesArtistAtom->getArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getAlbumArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesAlbumArtistAtom)
                return _pITunesAlbumArtistAtom->getAlbumArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getAlbum() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesAlbumAtom)
                return _pITunesAlbumAtom->getAlbum();
            else
                return temp;
        }

        // Gnre ** Starts **
        uint16 getGnreID() const
        {
            if (_pITunesGenreAtom)
            {
                if (_pITunesGenreAtom->getGnreVersion() == INTEGER_GENRE)
                {
                    return _pITunesGenreAtom->getGnreID();
                }
                else
                    return 0;
            }
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getGnreString() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesGenreAtom)
            {
                if (_pITunesGenreAtom->getGnreVersion() == STRING_GENRE)
                {
                    return _pITunesGenreAtom->getGnreString();
                }
                else
                    return temp;
            }
            else
                return temp;
        }

        //This function will tell the type of Genre--
        GnreVersion getGnreVersion() const
        {
            if (_pITunesGenreAtom)
            {
                return _pITunesGenreAtom->getGnreVersion();
            }
            else
                // By-default return INTEGER_GENRE
                return INTEGER_GENRE;
        }
        // Gnre ** Ends **


        // Returns the 4-byte YEAR when the song was recorded
        OSCL_wHeapString<OsclMemAllocator> getYear() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesYearAtom)
                return _pITunesYearAtom->getYear();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getTool() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesToolAtom)
            {
                return _pITunesToolAtom->getTool();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getEncodedBy() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesEncodedByAtom)
            {
                return _pITunesEncodedByAtom->getEncodedBy();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getWriter() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesWriterAtom)
                return _pITunesWriterAtom->getWriter();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getGroup() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesGroupAtom)
                return _pITunesGroupAtom->getGroup();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getComment() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesCommentAtom)
                return _pITunesCommentAtom->getComment();
            else
                return temp;
        }

        uint16 getThisTrackNo() const
        {
            if (_pITunesTracktAtom)
                return _pITunesTracktAtom->getThisTrackNo();
            else
                return 0;
        }

        uint16 getTotalTracks() const
        {
            if (_pITunesTracktAtom)
                return _pITunesTracktAtom->getTotalTracks();
            else
                return 0;
        }

        bool IsCompilationPart() const
        {
            if (_pITunesCompileAtom)
                return _pITunesCompileAtom->IsCompilationPart();
            else
                return false;
        }

        bool IsContentRating() const
        {
            if (_pITunesContentRatingAtom)
                return _pITunesContentRatingAtom->IsContentRating();
            else
                return false;
        }

        uint16 getBeatsPerMinute() const
        {
            if (_pITunesTempoAtom)
                return _pITunesTempoAtom->getBeatsPerMinute();
            else
                return 0;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesCopyrightAtom)
                return _pITunesCopyrightAtom->getITunesCopyright();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesDescription() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesDescriptionAtom)
                return _pITunesDescriptionAtom->getITunesDescription();
            else
                return temp;
        }


        PvmfApicStruct* getImageData() const
        {
            if (_pITunesCoverImageAtom)
                return _pITunesCoverImageAtom->getImageData();
            else
                return NULL;
        }

        uint16 getThisDiskNo() const
        {
            if (_pITunesDiskDatatAtom)
                return _pITunesDiskDatatAtom->getThisDiskNo();
            else
                return 0;
        }

        uint16 getTotalDisks() const
        {
            if (_pITunesDiskDatatAtom)
                return _pITunesDiskDatatAtom->getTotalDisks();
            else
                return 0;
        }



        OSCL_wHeapString<OsclMemAllocator> getNormalizationData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesNormalizationFreeFormDataAtom)
                return _pITunesNormalizationFreeFormDataAtom->getString();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getCDIdentifierData(uint8 index) const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if ((index >= _iITunesCDIdentifierFreeFormDataAtomNum) || (index > MAX_CD_IDENTIFIER_FREE_DATA_ATOM))
                return temp;

            if (_pITunesCDIdentifierFreeFormDataAtom[index])
                return _pITunesCDIdentifierFreeFormDataAtom[index]->getString();
            else
                return temp;
        }

        uint8 getTotalCDIdentifierData() const
        {

            return	_iITunesCDIdentifierFreeFormDataAtomNum;

        }

        OSCL_wHeapString<OsclMemAllocator> getCDTrackNumberData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesCDTrackNumberFreeFormDataAtom)
                return _pITunesCDTrackNumberFreeFormDataAtom->getString();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getCDDB1Data() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesCDDB1FreeFormDataAtom)
                return _pITunesCDDB1FreeFormDataAtom->getString();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getLyrics() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesLyricsAtom)
                return _pITunesLyricsAtom->getLyrics();
            else
                return temp;
        }

    private:

        //Meaning String
        ItunesMeaningAtom  *_pITunesMeaningAtom;
        Oscl_Vector<ItunesMeaningAtom*, OsclMemAllocator> *_pMeaningAtomVec;
        //Name String
        ItunesNameAtom     *_pITunesNameAtom;
        Oscl_Vector<ItunesNameAtom*, OsclMemAllocator> *_pNameAtomVec;
        // Title/Name Atom
        ITunesTitleAtom		*_pITunesTitleAtom;

        //Track's subtitle
        ITunesTrackSubTitleAtom *_pITunesTrackSubTitleAtom;

        // Artist/Performer of the Song
        ITunesArtistAtom		*_pITunesArtistAtom;

        //Artist for the whole album (if different than the individual tracks)
        ITunesAlbumArtistAtom		*_pITunesAlbumArtistAtom;

        // Album of Song
        ITunesAlbumAtom			*_pITunesAlbumAtom;

        // Genre
        ITunesGenreAtom			*_pITunesGenreAtom;

        // 4 byte String representing Year, when song was recorded.
        ITunesYearAtom			*_pITunesYearAtom;

        // Tool/Encoder used for creation of this file.
        ITunesToolAtom			*_pITunesToolAtom;

        //Person or company that encoded the recording
        ITunesEncodedByAtom     *_pITunesEncodedByAtom;

        // Writer of the Song
        ITunesWriterAtom		*_pITunesWriterAtom;

        // Group data.
        ITunesGroupAtom			*_pITunesGroupAtom;

        // Comment
        ITunesCommentAtom		*_pITunesCommentAtom;

        // Track Number
        ITunesTracktAtom		*_pITunesTracktAtom;

        // Whether this file is the Part of Compilation or not.
        ITunesCompileAtom		*_pITunesCompileAtom;

        // Does song have explicit content?
        ITunesContentRatingAtom *_pITunesContentRatingAtom;

        // Number of Beats per Minute
        ITunesTempoAtom			*_pITunesTempoAtom;

        ITunesCopyrightAtom		*_pITunesCopyrightAtom;

        ITunesDescriptionAtom	*_pITunesDescriptionAtom;


        // Album Art Data- PNG Image data
        //PvmfApicStruct _PNGimageData;
        ITunesCoverImageAtom	*_pITunesCoverImageAtom;

        // Disk Number
        ITunesDiskDatatAtom		*_pITunesDiskDatatAtom;

        // Normalization Free Form Data
        ITunesFreeFormDataAtom	*_pITunesNormalizationFreeFormDataAtom;

        // Normalization Free Form Tool Data
        ITunesFreeFormDataAtom	*_pITunesNormalizationFreeFormDataToolAtom;

        // CD Identifier Free Form Data
        uint8 _iITunesCDIdentifierFreeFormDataAtomNum;
        ITunesFreeFormDataAtom	*_pITunesCDIdentifierFreeFormDataAtom[MAX_CD_IDENTIFIER_FREE_DATA_ATOM];

        //CD Track Number Free Form Data
        ITunesFreeFormDataAtom *_pITunesCDTrackNumberFreeFormDataAtom;

        // CD Identifier Free Form Data
        ITunesFreeFormDataAtom *_pITunesCDDB1FreeFormDataAtom;

        // Lyrics of the Song
        ITunesLyricsAtom		*_pITunesLyricsAtom;

        PVLogger *iLogger;
};


#endif //ITUNESILSTATOM_H_INCLUDED
