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
/*                            MPEG-4 Mpeg4File Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    The Mpeg4File Class is the class that will construct and maintain all the
    mecessary data structures to be able to render a valid MP4 file to disk.
    Format.
*/


#define IMPLEMENT_Mpeg4File

#include "oscl_int64_utils.h"

#ifndef MPEG4FILE_H_INCLUDED
#include "mpeg4file.h"
#endif

#include "atomdefs.h"
#include "atomutils.h"
#include "filetypeatom.h"
#include "oscl_utf8conv.h"
#include "oscl_string.h"
#include "oscl_snprintf.h"
#include "amrdecoderspecificinfo.h"
#include "h263decoderspecificinfo.h"
#include "media_clock_converter.h"

typedef Oscl_Vector<TrackAtom*, OsclMemAllocator> trackAtomVecType;
typedef Oscl_Vector<MovieFragmentAtom*, OsclMemAllocator> movieFragmentAtomVecType;
typedef Oscl_Vector<uint32, OsclMemAllocator> movieFragmentOffsetVecType;
typedef Oscl_Vector<TrackDurationInfo*, OsclMemAllocator> trackDurationInfoVecType;
typedef Oscl_Vector<MovieFragmentRandomAccessAtom*, OsclMemAllocator> movieFragmentRandomAccessAtomVecType;

// Stream-in Constructor
Mpeg4File::Mpeg4File(MP4_FF_FILE *fp,
                     OSCL_wString& filename,
                     uint32 parsingMode)
{

    _pmovieAtom = NULL;
    _puserDataAtom = NULL;
    _pFileTypeAtom = NULL;
    _pMovieFragmentAtom = NULL;
    _mp4ErrorCode = EVERYTHING_FINE;
    _isMovieFragmentsPresent = false;
    _pointerMovieAtomEnd = 0;
    _movieFragmentFilePtr = NULL;
    _pMovieFragmentAtomVec = NULL;
    _pMfraOffsetAtom = NULL;
    _pMovieFragmentRandomAccessAtomVec = NULL;
    _pTrackExtendsAtomVec = NULL;
    _pMoofOffsetVec = NULL;
    _ptrMoofEnds = 0;
    _parsing_mode = parsingMode;
    _pTrackDurationContainer = NULL;
    oMfraFound = false;
    _oVideoTrackPresent = false;
    parseMoofCompletely = true;
    isResetPlayBackCalled = false;

    parseMoofCompletely = true;
    moofParsingCompleted = true;
    moofSize = 0;
    moofType = UNKNOWN_ATOM;
    moofCount = 0;
    moofPtrPos = 0;
    currMoofNum = 0;
    countOfTrunsParsed = 0;

    // Create miscellaneous vector of atoms
    PV_MP4_FF_NEW(fp->auditCB, trackAtomVecType, (), _pTrackAtomVec);
    PV_MP4_FF_NEW(fp->auditCB, movieFragmentAtomVecType, (), _pMovieFragmentAtomVec);
    PV_MP4_FF_NEW(fp->auditCB, movieFragmentOffsetVecType, (), _pMoofOffsetVec);
    PV_MP4_FF_NEW(fp->auditCB, movieFragmentRandomAccessAtomVecType, (), _pMovieFragmentRandomAccessAtomVec);
    PV_MP4_FF_NEW(fp->auditCB, PVID3ParCom, (), _pID3Parser);


    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    iStateVarLogger = PVLogger::GetLoggerObject("mp4ffparser_mediasamplestats");
    iParsedDataLogger = PVLogger::GetLoggerObject("mp4ffparser_parseddata");

    _success = true; // Initial state

    int32 fileSize;
    int32 filePointer;
    filePointer = AtomUtils::getCurrentFilePosition(fp);
    uint32 fsize = 0;
    AtomUtils::getCurrentFileSize(fp, fsize);
    fileSize = (int32)fsize;

    _oPVContent = false;
    _oPVContentDownloadable = false;
    _commonFilePtr = NULL;
    _fileSize = fsize;

    int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;

    //top level moov, mdat, udat
    while (count > 0)
    {
        // Read in atoms until reach end of file
        //there is a case that next atom is valid, but not in top level
        //so only check top level now
        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        AtomUtils::getNextAtomType(fp, atomSize, atomType);

        if ((atomType == SKIP_ATOM)
                || (atomType == FREE_SPACE_ATOM)
                || (atomType == UUID_ATOM)
                || (atomType == UNKNOWN_ATOM)
                || (atomType == MEDIA_DATA_ATOM))
        {
            if (atomSize == 1)
            {
                uint64 largeSize = 0;
                AtomUtils::read64(fp, largeSize);
                uint32 size =
                    Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                count -= size;
                size -= 8; //for large size
                size -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, size);
            }
            else
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _success = false;
                    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                if (count < (int32)atomSize)
                {
                    _success = false;
                    _mp4ErrorCode = READ_FAILED;
                    break;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
        }
        else if (atomType == USER_DATA_ATOM)
        {
            //"udta"
            // Check for 'pvmm' to see if it is "our" 'udta' atom

            uint32 isPVMMAtom = AtomUtils::peekNextNthBytes(fp, 2);

            if (isPVMMAtom == PVUSER_DATA_ATOM)
            {
                if (_puserDataAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, UserDataAtom, (fp, atomSize, atomType), _puserDataAtom);
                    _oPVContent = true;
                    uint32 contentType = getContentType();
                    if (contentType == DEFAULT_AUTHORING_MODE)
                    {
                        _oPVContentDownloadable = true;
                    }
                    count -= _puserDataAtom->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_USER_DATA_ATOM_FAILED;
                    break;
                }
            }
            else
            {
                // Skip third party user data atom
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _success = false;
                    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                if (count < (int32)atomSize)
                {
                    _success = false;
                    _mp4ErrorCode = READ_FAILED;
                    break;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }


        }
        else if (atomType == FILE_TYPE_ATOM)
        {
            if (_pFileTypeAtom == NULL)
            {
                //"ftyp"
                PV_MP4_FF_NEW(fp->auditCB, FileTypeAtom, (fp, atomSize, atomType), _pFileTypeAtom);

                if (!_pFileTypeAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = READ_FILE_TYPE_ATOM_FAILED;
                    break;
                }

                uint32 majorBrand = _pFileTypeAtom->getMajorBrand();
                uint32  majorBrandInfo = ENoFileType;
                uint32	compatibleBrandInfo = ENoFileType;


                switch (majorBrand)
                {
                    case WMF_BRAND:
                        majorBrandInfo |= EWMF;
                        break;

                    case BRAND_3GPP4:
                        majorBrandInfo |= E3GP4;
                        break;

                    case BRAND_3GPP5:
                        majorBrandInfo |= E3GP5;
                        break;

                    case MOBILE_MP4:
                        majorBrandInfo |= EMMP4;
                        break;

                    case BRAND_MP41:
                        majorBrandInfo |= EMP41;
                        break;

                    case BRAND_MP42:
                        majorBrandInfo |= EMP42;
                        _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                        break;

                    case BRAND_ISOM:
                        majorBrandInfo |= EISOM;
                        _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                        break;

                    default:
                        majorBrandInfo |= EUNKNOWN_TYPE;
                        _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                        break;
                }

                Oscl_Vector<uint32, OsclMemAllocator> *compatibleBrandArray =
                    _pFileTypeAtom->getCompatibleBrand();

                if (compatibleBrandArray != NULL)
                {
                    for (uint32 i = 0; i < compatibleBrandArray->size(); i++)
                    {
                        uint32 compatibleBrand = (*compatibleBrandArray)[i];

                        switch (compatibleBrand)
                        {
                            case WMF_BRAND:
                                compatibleBrandInfo |= EWMF;
                                break;

                            case BRAND_3GPP4:
                                compatibleBrandInfo |= E3GP4;
                                break;

                            case BRAND_3GPP5:
                                compatibleBrandInfo |= E3GP5;
                                break;

                            case MOBILE_MP4:
                                compatibleBrandInfo |= EMMP4;
                                break;

                            case BRAND_MP41:
                                compatibleBrandInfo |= EMP41;
                                break;

                            case BRAND_MP42:
                                compatibleBrandInfo |= EMP42;
                                _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                                break;

                            case BRAND_ISOM:
                                compatibleBrandInfo |= EISOM;
                                _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                                break;

                            default:
                                compatibleBrandInfo |= EUNKNOWN_TYPE;
                                _mp4ErrorCode = UNSUPPORTED_FILE_TYPE;
                                break;
                        }
                    }
                }

                count -= _pFileTypeAtom->getSize();
            }
            else
            {
                //multiple "ftyp" atom not allowed.skipping
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
        }
        else if (atomType == MOVIE_ATOM)
        {


            //"moov"
            if (_pmovieAtom == NULL)
            {
                // Only 1 movie atom allowed!
                PV_MP4_FF_NEW(fp->auditCB, MovieAtom,
                              (fp,
                               filename,
                               atomSize,
                               atomType,
                               _oPVContent,
                               _oPVContentDownloadable,
                               parsingMode
                              ),
                              _pmovieAtom);

                if (!_pmovieAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmovieAtom->GetMP4Error();
                    break;
                }
                _isMovieFragmentsPresent = _pmovieAtom->IsMovieFragmentPresent();
                populateTrackDurationVec();
                _pTrackExtendsAtomVec = _pmovieAtom->getTrackExtendsAtomVec();

                if (_isMovieFragmentsPresent)
                {
                    atomSize -= DEFAULT_ATOM_SIZE;
                    _pointerMovieAtomEnd =  AtomUtils::getCurrentFilePosition(fp);
                    _ptrMoofEnds = _pointerMovieAtomEnd;
                    OsclAny*ptr = oscl_malloc(sizeof(MP4_FF_FILE));
                    if (ptr == NULL)
                    {
                        _success = false;
                        _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                        return;
                    }
                    _movieFragmentFilePtr = OSCL_PLACEMENT_NEW(ptr, MP4_FF_FILE(*fp));
                    _movieFragmentFilePtr ->_pvfile.Copy(fp->_pvfile);
                    _movieFragmentFilePtr ->_pvfile.SetCPM(fp->_pvfile.GetCPM());
                }
                count -= _pmovieAtom->getSize();
                _scalability = _pmovieAtom->getScalability();
                _fileType    = _pmovieAtom->getFileType();

                if (parsingMode != 0)
                {
                    if (_isMovieFragmentsPresent)
                    {
                        parseMFRA();
                    }
                    break;
                }
                if (!_isMovieFragmentsPresent)
                {
                    //no moofs, exit the parsing loop since
                    //we are done parsing the moov atom
                    break;
                }


            }
            else
            { //after the change above, we will never hit here.
                _success = false;
                _mp4ErrorCode = DUPLICATE_MOVIE_ATOMS;
                break;
            }
        }
        else if (atomType == MOVIE_FRAGMENT_ATOM)
        {
            uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(fp);
            moofStartOffset -= DEFAULT_ATOM_SIZE;
            _pMoofOffsetVec->push_back(moofStartOffset);

            MovieFragmentAtom *pMovieFragmentAtom = NULL;
            PV_MP4_FF_NEW(fp->auditCB, MovieFragmentAtom, (fp, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), pMovieFragmentAtom);

            if (!pMovieFragmentAtom->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = pMovieFragmentAtom->GetMP4Error();
                break;
            }
            pMovieFragmentAtom->setParent(this);
            count -= pMovieFragmentAtom->getSize();
            _ptrMoofEnds = AtomUtils::getCurrentFilePosition(fp);
            _pMovieFragmentAtomVec->push_back(pMovieFragmentAtom);
        }
        else if (atomType == MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM)
        {
            MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = NULL;
            PV_MP4_FF_NEW(fp->auditCB, MovieFragmentRandomAccessAtom, (fp, atomSize, atomType), pMovieFragmentRandomAccessAtom);

            if (!pMovieFragmentRandomAccessAtom->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = pMovieFragmentRandomAccessAtom->GetMP4Error();
                break;
            }
            pMovieFragmentRandomAccessAtom->setParent(this);
            count -= pMovieFragmentRandomAccessAtom->getSize();
            _pMovieFragmentRandomAccessAtomVec->push_back(pMovieFragmentRandomAccessAtom);
            oMfraFound = true;
        }
        else
        {
            if (count > 0)
            {
                _mp4ErrorCode = READ_UNKNOWN_ATOM;
                _success = false;
            }
            break;
        }
    }

    if (_success)
    {
        // Check that the movie atom was in fact read in
        if (_pmovieAtom == NULL)
        {
            _success = false;
            _mp4ErrorCode = NO_MOVIE_ATOM_PRESENT;
        }
        else
        {
            // CHECK IF THERE ARE ANY VALID MEDIA TRACKS IN THE FILE
            int32 numMediaTracks = getNumTracks();
            if (numMediaTracks == 0)
            {
                _success = false;
                _mp4ErrorCode = NO_META_DATA_FOR_MEDIA_TRACKS;
            }
            if (_success)
            {
                uint32 bufferCapacity = AtomUtils::getFileBufferingCapacity(fp);
                if (0 != bufferCapacity)
                {
                    // progressive playback
                    int32* offsetList = (int32 *)oscl_malloc(sizeof(int32) * numMediaTracks);
                    if (NULL == offsetList)
                    {
                        _success = false;
                        _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                    }
                    else
                    {
                        // get the list of track ids
                        uint32* idList = (uint32 *)oscl_malloc(sizeof(uint32) * numMediaTracks);
                        _pmovieAtom->getTrackIDList(idList, numMediaTracks);

                        // get the first sample file offset of each track
                        for (int32 i = 0; i < numMediaTracks; i++)
                        {
                            int32 retVal = _pmovieAtom->getOffsetByTime(idList[i], 0, &offsetList[i]);
                            if (EVERYTHING_FINE != retVal)
                            {
                                _success = false;
                                _mp4ErrorCode = retVal;
                                break;
                            }
                        }
                        // check if any of the two offsets are too far apart
                        // to coexist in the cache at the same time
                        if (_success)
                        {
                            uint32 largest = 0, temp = 0;
                            for (int i = 0; i < numMediaTracks; i++)
                            {
                                for (int j = 0; j < numMediaTracks; j++)
                                {
                                    // same as abs()
                                    if (offsetList[i] > offsetList[j])
                                    {
                                        temp = offsetList[i] - offsetList[j];
                                    }
                                    else
                                    {
                                        temp = offsetList[j] - offsetList[i];
                                    }

                                    if (temp > largest)
                                    {
                                        largest = temp;
                                    }
                                }
                            }

                            if (largest > bufferCapacity)
                            {
                                // the samples are not interleaved properly
                                // this clip is not authored for progressive playback
                                _success = false;
                                _mp4ErrorCode = INSUFFICIENT_BUFFER_SIZE;
                            }
                        }

                        oscl_free(idList);
                    }

                    oscl_free(offsetList);
                }
            }
        }
    }


    // Check for any atoms that may have read past the EOF that were not
    // already caught by any earlier error handling

    if (filePointer > fileSize)
    {
        _mp4ErrorCode = READ_FAILED; // Read past EOF
        _success = false;
    }

    // skip ID3 tag parsing for progressive playback for now
    uint32 bufferCapacity = AtomUtils::getFileBufferingCapacity(fp);
    if (0 == bufferCapacity)
    {
        parseID3Header(fp);
    }
    //Populate the title vector with all the title metadata values.
    if (!populateMetadataVectors())
    {
        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Mpeg4File::populateTitleVector() Failed"));
    }

}


PVMFStatus Mpeg4File::populateMetadataVectors()
{
    if ((!populateTitleVector()) || (!populateAuthorVector() ||
                                     (!populateAlbumVector()) ||
                                     (!populateArtistVector()) || (!populateGenreVector()) ||
                                     (!populateYearVector()) || (!populateCopyrightVector()) ||
                                     (!populateCommentVector()) || (!populateDescriptionVector()) ||
                                     (!populateRatingVector()))
       )
    {
        return PVMFFailure;
    }

    return PVMFSuccess;
}

uint32 Mpeg4File::getNumTitle()
{
    uint32 numTitle = 0;
    MP4FFParserOriginalCharEnc chartype = ORIGINAL_CHAR_TYPE_UNKNOWN;
    numTitle = getNumAssetInfoTitleAtoms();
    if (getPVTitle(chartype).get_size() > 0)
    {
        numTitle++;
    }
    if (getITunesTitle().get_size() > 0)
    {
        numTitle++;
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "title") != 0)
            {
                numTitle++;
                break;
            }
        }
    }
    return numTitle;
}

//This function populates the Title Vector with values from Asset Info, Itunes, FullMusic and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateTitleVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numTitle = getNumTitle();
    ReserveMemoryForValuesVector(titleValues, numTitle, leavecode);
    ReserveMemoryForLangCodeVector(iTitleLangCode, numTitle, leavecode1);
    OSCL_TRY(leavecode2, iTitleCharType.reserve(numTitle));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoTitle = getNumAssetInfoTitleAtoms();


    if (numAssetInfoTitle > 0)
    {
        for (int32 i = 0; i < numAssetInfoTitle; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoTitleNotice(charType, i);

            titleValues.push_front(valuestring);
            iTitleLangCode.push_front(getAssetInfoTitleLangCode(i));
            iTitleCharType.push_front(charType);
        }
    }
    if (getPVTitle(charType).get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getPVTitle(charType);
        titleValues.push_front(valuestring);
        iTitleLangCode.push_front(0);
        iTitleCharType.push_front(charType);
    }
    if (getITunesTitle().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesTitle();
        titleValues.push_front(valuestring);
        iTitleLangCode.push_front(0);
        iTitleCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "title") != 0)
            {
                uint32 len = oscl_strlen(framevector[i]->value.pChar_value);
                oscl_memset(_id3v1Title, 0, ID3V1_STR_MAX_SIZE);
                oscl_UTF8ToUnicode(framevector[i]->value.pChar_value, len, _id3v1Title, len*2 + 2);
                titleValues.push_front(_id3v1Title);
                iTitleLangCode.push_front(0);
                iTitleCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
                break;
            }
        }
    }
    return PVMFSuccess;
}

// This function returns the titles based on index value, to the parser node.
PVMFStatus Mpeg4File::getTitle(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < titleValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = titleValues[index].get_cstr();
        aLangCode = iTitleLangCode[index];
        aCharEncType = iTitleCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

uint32 Mpeg4File::getNumAuthor()
{
    uint32 numAuthor = 0;
    MP4FFParserOriginalCharEnc chartype = ORIGINAL_CHAR_TYPE_UNKNOWN;
    numAuthor = getNumAssetInfoAuthorAtoms();

    if (getPVAuthor(chartype).get_size() > 0)
    {
        numAuthor++;
    }
    return numAuthor;
}

//This function populates the Author Vector with values from Asset Info and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateAuthorVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numAuthor = getNumAuthor();
    ReserveMemoryForValuesVector(authorValues, numAuthor, leavecode);
    ReserveMemoryForLangCodeVector(iAuthorLangCode, numAuthor, leavecode1);
    OSCL_TRY(leavecode2, iAuthorCharType.reserve(numAuthor));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoAuthor = getNumAssetInfoAuthorAtoms();
    if (numAssetInfoAuthor > 0)
    {
        for (int32 i = 0; i < numAssetInfoAuthor; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoAuthorNotice(charType, i);
            authorValues.push_front(valuestring);
            iAuthorLangCode.push_front(getAssetInfoAuthorLangCode(i));
            iAuthorCharType.push_front(charType);
        }
    }
    if (getPVAuthor(charType).get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getPVAuthor(charType);
        authorValues.push_front(valuestring);
        iAuthorLangCode.push_front(0);
        iAuthorCharType.push_front(charType);
    }
    return PVMFSuccess;
}

// This function returns the Author based on index value, to the parser node.
PVMFStatus Mpeg4File::getAuthor(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < authorValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = authorValues[index].get_cstr();
        aLangCode = iAuthorLangCode[index];
        aCharEncType = iAuthorCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

uint32 Mpeg4File::getNumAlbum()
{
    uint32 numAlbum = 0;
    numAlbum = getNumAssetInfoAlbumAtoms();
    if (getITunesAlbum().get_size() > 0)
    {
        numAlbum++;
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "album") != 0)
            {
                numAlbum++;
                break;
            }
        }
    }
    return numAlbum;
}

//This function populates the Album Vector with values from Asset Info, Itunes, FullMusic Atoms.
PVMFStatus Mpeg4File::populateAlbumVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numAlbum = getNumAlbum();
    ReserveMemoryForValuesVector(albumValues, numAlbum, leavecode);
    ReserveMemoryForLangCodeVector(iAlbumLangCode, numAlbum, leavecode1);
    OSCL_TRY(leavecode2, iAlbumCharType.reserve(numAlbum));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoAlbum = getNumAssetInfoAlbumAtoms();
    if (numAssetInfoAlbum > 0)
    {
        for (int32 i = 0; i < numAssetInfoAlbum; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoAlbumNotice(charType, i);
            albumValues.push_front(valuestring);
            iAlbumLangCode.push_front(getAssetInfoAlbumLangCode(i));
            iAlbumCharType.push_front(charType);
        }
    }
    if (getITunesAlbum().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesAlbum();
        albumValues.push_front(valuestring);
        iAlbumLangCode.push_front(0);
        iAlbumCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "album") != 0)
            {
                uint32 len = oscl_strlen(framevector[i]->value.pChar_value);
                oscl_memset(_id3v1Album, 0, ID3V1_STR_MAX_SIZE);
                oscl_UTF8ToUnicode(framevector[i]->value.pChar_value, len, _id3v1Album, len*2 + 2);
                albumValues.push_front(_id3v1Album);
                iAlbumLangCode.push_front(0);
                iAlbumCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
                break;
            }
        }
    }

    return PVMFSuccess;
}

// This function returns the Album based on index value, to the parser node.
PVMFStatus Mpeg4File::getAlbum(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < albumValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = albumValues[index].get_cstr();
        aLangCode = iAlbumLangCode[index];
        aCharEncType = iAlbumCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

uint32 Mpeg4File::getNumArtist()
{
    uint32 numArtist = 0;
    numArtist = getNumAssetInfoPerformerAtoms();

    if (getITunesArtist().get_size() > 0)
    {
        numArtist++;
    }
    if (getITunesAlbumArtist().get_size() > 0) //AlbumArtist
    {
        numArtist++;
    }

    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "artist") != 0)
            {
                numArtist++;
                break;
            }
        }
    }
    return numArtist;
}


//This function populates the Artist Vector with values from Asset Info, Itunes, FullMusic Atoms.
PVMFStatus Mpeg4File::populateArtistVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numArtist = getNumArtist();
    ReserveMemoryForValuesVector(artistValues, numArtist, leavecode);
    ReserveMemoryForLangCodeVector(iArtistLangCode, numArtist, leavecode1);
    OSCL_TRY(leavecode2, iArtistCharType.reserve(numArtist));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoPerformer = getNumAssetInfoPerformerAtoms();
    if (numAssetInfoPerformer > 0)
    {
        for (int32 i = 0; i < numAssetInfoPerformer; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoPerformerNotice(charType, i);
            artistValues.push_front(valuestring);
            iArtistLangCode.push_front(getAssetInfoPerformerLangCode(i));
            iArtistCharType.push_front(charType);
        }
    }
    if (getITunesArtist().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesArtist();
        artistValues.push_front(valuestring);
        iArtistLangCode.push_front(0);
        iArtistCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    if (getITunesAlbumArtist().get_size() > 0) //AlbumArtist
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesAlbumArtist();
        artistValues.push_front(valuestring);
        iArtistLangCode.push_front(0);
        iArtistCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "artist") != 0)
            {
                uint32 len = oscl_strlen(framevector[i]->value.pChar_value);
                oscl_memset(_id3v1Artist, 0, ID3V1_STR_MAX_SIZE);
                oscl_UTF8ToUnicode(framevector[i]->value.pChar_value, len, _id3v1Artist, len*2 + 2);
                artistValues.push_front(_id3v1Artist);
                iArtistLangCode.push_front(0);
                iArtistCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
                break;
            }
        }
    }
    return PVMFSuccess;
}

// This function returns the Artists based on index value, to the parser node.
PVMFStatus Mpeg4File::getArtist(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < artistValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = artistValues[index].get_cstr();
        aLangCode = iArtistLangCode[index];
        aCharEncType = iArtistCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

uint32 Mpeg4File::getNumGenre()
{
    uint32 numGenre = 0;
    numGenre = getNumAssetInfoGenreAtoms();

    if (getITunesGnreString().get_size() > 0)
    {
        numGenre++;
    }
    if (getITunesGnreID() > 0)
    {
        numGenre++;
    }
    return numGenre;
}

//This function populates the Genre Vector with values from Asset Info, Itunes, FullMusic Atoms.
PVMFStatus Mpeg4File::populateGenreVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numGenre = getNumGenre();
    ReserveMemoryForValuesVector(genreValues, numGenre, leavecode);
    ReserveMemoryForLangCodeVector(iGenreLangCode, numGenre, leavecode1);
    OSCL_TRY(leavecode2, iGenreCharType.reserve(numGenre));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoGenre = getNumAssetInfoGenreAtoms();
    if (numAssetInfoGenre > 0)
    {
        for (int32 i = 0; i < numAssetInfoGenre; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoGenreNotice(charType, i);
            genreValues.push_front(valuestring);
            iGenreLangCode.push_front(getAssetInfoGenreLangCode(i));
            iGenreCharType.push_front(charType);
        }
    }
    if (getITunesGnreString().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesGnreString();
        genreValues.push_front(valuestring);
        iGenreLangCode.push_front(0);
        iGenreCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    return PVMFSuccess;
}

// This function returns the Genres based on index value, to the parser node.
PVMFStatus Mpeg4File::getGenre(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < genreValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = genreValues[index].get_cstr();
        aLangCode = iGenreLangCode[index];
        aCharEncType = iGenreCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}


uint32 Mpeg4File::getNumCopyright()
{
    uint32 numCopyright = 0;
    MP4FFParserOriginalCharEnc chartype = ORIGINAL_CHAR_TYPE_UNKNOWN;
    numCopyright = getNumCopyRightAtoms();
    if (getPVCopyright(chartype).get_size() > 0)
    {
        numCopyright++;
    }
    if (getITunesCopyright().get_size() > 0)
    {
        numCopyright++;
    }
    return numCopyright;
}


//This function populates the Copyright Vector with values from Asset Info, Itunes and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateCopyrightVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numCopyright = getNumCopyright();
    ReserveMemoryForValuesVector(copyrightValues, numCopyright, leavecode);
    ReserveMemoryForLangCodeVector(iCopyrightLangCode, numCopyright, leavecode1);
    OSCL_TRY(leavecode2, iCopyrightCharType.reserve(numCopyright));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoCopyright = getNumCopyRightAtoms();
    if (numAssetInfoCopyright > 0)
    {
        for (int32 i = 0; i < numAssetInfoCopyright; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getCopyRightString(charType, i);
            copyrightValues.push_front(valuestring);
            iCopyrightLangCode.push_front(getCopyRightLanguageCode(i));
            iCopyrightCharType.push_front(charType);
        }
    }
    if (getPVCopyright(charType).get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getPVCopyright(charType);
        copyrightValues.push_front(valuestring);
        iCopyrightLangCode.push_front(0);
        iCopyrightCharType.push_front(charType);
    }
    if (getITunesCopyright().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesCopyright();
        copyrightValues.push_front(valuestring);
        iCopyrightLangCode.push_front(0);
        iCopyrightCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    return PVMFSuccess;
}

// This function returns the Copyrights based on index value, to the parser node.
PVMFStatus Mpeg4File::getCopyright(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < copyrightValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = copyrightValues[index].get_cstr();
        aLangCode = iCopyrightLangCode[index];
        aCharEncType = iCopyrightCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}


uint32 Mpeg4File::getNumComment()
{
    uint32 numComment = 0;


    if (getITunesComment().get_size() > 0)
    {
        numComment++;
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "comment") != 0)
            {
                numComment++;
                break;
            }
        }
    }

    return numComment;
}


//This function populates the Comment Vector with values from Itunes, FullMusic Atoms.
PVMFStatus Mpeg4File::populateCommentVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numComment = getNumComment();
    ReserveMemoryForValuesVector(commentValues, numComment, leavecode);
    ReserveMemoryForLangCodeVector(iCommentLangCode, numComment, leavecode1);
    OSCL_TRY(leavecode2, iCommentCharType.reserve(numComment));
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }

    if (getITunesComment().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesComment();
        commentValues.push_front(valuestring);
        iCommentLangCode.push_front(0);
        iCommentCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "comment") != 0)
            {
                uint32 len = oscl_strlen(framevector[i]->value.pChar_value);
                oscl_memset(_id3v1Comment, 0, ID3V1_STR_MAX_SIZE);
                oscl_UTF8ToUnicode(framevector[i]->value.pChar_value, len, _id3v1Comment, len*2 + 2);
                commentValues.push_front(_id3v1Comment);
                iCommentLangCode.push_front(0);
                iCommentCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
                break;
            }
        }
    }

    return PVMFSuccess;
}

// This function returns the Comments based on index value, to the parser node.
PVMFStatus Mpeg4File::getComment(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType)
{
    if (index < commentValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = commentValues[index].get_cstr();
        aLangCode = iCommentLangCode[index];
        aCharEncType = iCommentCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}


uint32 Mpeg4File::getNumDescription()
{
    uint32 numDescription = 0;
    MP4FFParserOriginalCharEnc chartype = ORIGINAL_CHAR_TYPE_UNKNOWN;
    numDescription = getNumAssetInfoDescAtoms();
    if (getPVDescription(chartype).get_size() > 0)
    {
        numDescription++;
    }
    if (getITunesDescription().get_size() > 0)
    {
        numDescription++;
    }

    return numDescription;
}


//This function populates the Description Vector with values from Asset Info, Itunes and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateDescriptionVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numDescription = getNumDescription();
    ReserveMemoryForValuesVector(descriptionValues, numDescription, leavecode);
    ReserveMemoryForLangCodeVector(iDescriptionLangCode, numDescription, leavecode1);
    OSCL_TRY(leavecode2, iDescriptionCharType.reserve(numDescription));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoDescription = getNumAssetInfoDescAtoms();
    if (numAssetInfoDescription > 0)
    {
        for (int32 i = 0; i < numAssetInfoDescription; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoDescNotice(charType, i);
            descriptionValues.push_front(valuestring);
            iDescriptionLangCode.push_front(getAssetInfoDescLangCode(i));
            iDescriptionCharType.push_front(charType);
        }
    }
    if (getPVDescription(charType).get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getPVDescription(charType);
        descriptionValues.push_front(valuestring);
        iDescriptionLangCode.push_front(0);
        iDescriptionCharType.push_front(charType);
    }

    if (getITunesDescription().get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getITunesDescription();
        descriptionValues.push_front(valuestring);
        iDescriptionLangCode.push_front(0);
        iDescriptionCharType.push_front(ORIGINAL_CHAR_TYPE_UNKNOWN);
    }

    return PVMFSuccess;
}

// This function returns the Descriptions based on index value, to the parser node.
PVMFStatus Mpeg4File::getDescription(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc&
                                     aCharEncType)
{
    if (index < descriptionValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = descriptionValues[index].get_cstr();
        aLangCode = iDescriptionLangCode[index];
        aCharEncType = iDescriptionCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}


uint32 Mpeg4File::getNumRating()
{
    uint32 numRating = 0;
    MP4FFParserOriginalCharEnc chartype = ORIGINAL_CHAR_TYPE_UNKNOWN;
    numRating = getNumAssetInfoRatingAtoms();

    if (getPVRating(chartype).get_size() > 0)
    {
        numRating++;
    }
    return numRating;
}


//This function populates the Rating Vector with values from Asset Info and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateRatingVector()
{
    int32 leavecode = 0, leavecode1 = 0, leavecode2 = 0;
    int32 numRating = getNumRating();
    ReserveMemoryForValuesVector(ratingValues, numRating, leavecode);
    ReserveMemoryForLangCodeVector(iRatingLangCode, numRating, leavecode1);
    OSCL_TRY(leavecode2, iRatingCharType.reserve(numRating));
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    if (leavecode != 0 || leavecode1 != 0 || leavecode2 != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoRating = getNumAssetInfoRatingAtoms();
    if (numAssetInfoRating > 0)
    {
        for (int32 i = 0; i < numAssetInfoRating; i++)
        {
            OSCL_wHeapString<OsclMemAllocator> valuestring = getAssetInfoRatingNotice(charType, i);
            ratingValues.push_front(valuestring);
            iRatingLangCode.push_front(getAssetInfoRatingLangCode(i));
            iRatingCharType.push_front(charType);
        }
    }
    if (getPVRating(charType).get_size() > 0)
    {
        OSCL_wHeapString<OsclMemAllocator> valuestring = getPVRating(charType);
        ratingValues.push_front(valuestring);
        iRatingLangCode.push_front(0);
        iRatingCharType.push_front(charType);
    }

    return PVMFSuccess;
}

// This function returns the Ratings based on index value, to the parser node.
PVMFStatus Mpeg4File::getRating(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc&
                                aCharEncType)
{
    if (index < ratingValues.size())
    {
        aVal = NULL;
        aLangCode = 0;
        aCharEncType = ORIGINAL_CHAR_TYPE_UNKNOWN;
        aVal = ratingValues[index].get_cstr();
        aLangCode = iRatingLangCode[index];
        aCharEncType = iRatingCharType[index];
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}


uint32 Mpeg4File::getNumYear()
{
    uint32 numYear = 0;
    numYear = getNumAssetInfoRecordingYearAtoms();
    if (getITunesYear().get_size() > 0)
    {
        numYear++;
    }

    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "year") != 0)
            {
                numYear++;
                break;
            }
        }
    }
    return numYear;
}


//This function populates the Year Vector with values from Asset Info, Itunes, FullMusic and PV Proprietary Atoms.
PVMFStatus Mpeg4File::populateYearVector()
{
    int32 leavecode = 0;
    int32 numYear = getNumYear();
    OSCL_TRY(leavecode, yearValues.reserve(numYear));

    if (leavecode != 0)
    {
        return PVMFFailure;
    }
    int32 numAssetInfoRecordingYear = getNumAssetInfoRecordingYearAtoms();
    if (numAssetInfoRecordingYear > 0)
    {
        for (int32 i = 0; i < numAssetInfoRecordingYear; i++)
        {
            uint16 valuestring = getAssetInfoRecordingYear(i);
            yearValues.push_front(valuestring);
        }
    }
    if (getITunesYear().get_size() > 0)
    {
        uint32 value, i;
        OSCL_wHeapString<OsclMemAllocator> values1 = getITunesYear();
        char valuestring[256];
        oscl_UnicodeToUTF8(values1.get_cstr(), values1.get_size(), valuestring, 256);
        i = PV_atoi(valuestring, 'd', value);
        yearValues.push_front(value);
    }
    PvmiKvpSharedPtrVector framevector;
    GetID3MetaData(framevector);
    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        if (framevector.size() > 0)
        {
            if (oscl_strstr(framevector[i]->key, "year") != 0)
            {
                PV_atoi(framevector[i]->value.pChar_value, 'd', _id3v1Year);
                yearValues.push_front(_id3v1Year);
                break;
            }
        }
    }

    return PVMFSuccess;
}

// This function returns the Years based on index value, to the parser node.
PVMFStatus Mpeg4File::getYear(uint32 index, uint32& aVal)
{
    if (index < yearValues.size())
    {
        aVal = 0;
        aVal = yearValues[index];

        return PVMFSuccess;
    }
    return PVMFErrArgument;
}
OSCL_wString& Mpeg4File::getPVTitle(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVTitle(charType);
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wString& Mpeg4File::getPVAuthor(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVAuthor(charType);
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wString& Mpeg4File::getPVVersion(MP4FFParserOriginalCharEnc &charType)
{
    OSCL_UNUSED_ARG(charType);

    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVVersion();
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wString& Mpeg4File::getPVCopyright(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVCopyright(charType);
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wString& Mpeg4File::getPVDescription(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVDescription(charType);
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wString& Mpeg4File::getPVRating(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
    }
    else
    {
        return _emptyString;
    }

    if (patom != NULL)
    {
        return patom->getPVRating(charType);
    }
    else
    {
        return _emptyString;
    }
}

OSCL_wHeapString<OsclMemAllocator> Mpeg4File::getCreationDate(MP4FFParserOriginalCharEnc &charType)
{
    PVUserDataAtom *patom = NULL;
    if (_puserDataAtom != NULL)
    {
        patom =
            (PVUserDataAtom*) _puserDataAtom->getAtomOfType(FourCharConstToUint32('p', 'v', 'm', 'm'));
        if (patom != NULL)
        {
            return patom->getPVCreationDate(charType);
        }
        else
        {
            return _emptyString;
        }
    }
    else
    {
        return (_pmovieAtom->getCreationDate());
    }
}

// Destructor
Mpeg4File::~Mpeg4File()
{
    uint32 i;
    // Clean up atoms
    if (_pmovieAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MovieAtom, _pmovieAtom);
    }

    //Delete all the track atoms in the vec
    for (i = 0; i < _pTrackAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, TrackAtom, (*_pTrackAtomVec)[i]);
    }

    // Delete the vectors themselves
    PV_MP4_FF_TEMPLATED_DELETE(NULL, trackAtomVecType, Oscl_Vector, _pTrackAtomVec);


    titleValues.destroy();
    iTitleLangCode.destroy();
    iTitleCharType.destroy();

    authorValues.destroy();
    iAuthorLangCode.destroy();
    iAuthorCharType.destroy();


    albumValues.destroy();
    iAlbumLangCode.destroy();
    iAlbumCharType.destroy();


    artistValues.destroy();
    iArtistLangCode.destroy();
    iArtistCharType.destroy();


    genreValues.destroy();
    iGenreLangCode.destroy();
    iGenreCharType.destroy();


    yearValues.destroy();


    copyrightValues.destroy();
    iCopyrightLangCode.destroy();
    iCopyrightCharType.destroy();


    commentValues.destroy();
    iCommentLangCode.destroy();
    iCommentCharType.destroy();


    descriptionValues.destroy();
    iDescriptionLangCode.destroy();
    iDescriptionCharType.destroy();


    ratingValues.destroy();
    iRatingLangCode.destroy();
    iRatingCharType.destroy()	;


    //delete all movie fragments
    for (i = 0; i < _pMovieFragmentAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, MovieFragmentAtom, (*_pMovieFragmentAtomVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, movieFragmentAtomVecType, Oscl_Vector, _pMovieFragmentAtomVec);
    //delete all movie fragments randomm access box
    for (i = 0; i < _pMovieFragmentRandomAccessAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, MovieFragmentRandomAccessAtom, (*_pMovieFragmentRandomAccessAtomVec)[i]);
    }
    // Delete the vectors themselves
    PV_MP4_FF_TEMPLATED_DELETE(NULL, movieFragmentRandomAccessAtomVecType, Oscl_Vector, _pMovieFragmentRandomAccessAtomVec);

    if (_pMoofOffsetVec != NULL)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, movieFragmentOffsetVecType, Oscl_Vector, _pMoofOffsetVec);


    if (_pMfraOffsetAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MfraOffsetAtom, _pMfraOffsetAtom);
    }

    if (_pTrackDurationContainer != NULL)
    {
        for (i = 0; i < _pTrackDurationContainer->_pTrackdurationInfoVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, TrackDurationInfo, (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, trackDurationInfoVecType, Oscl_Vector, _pTrackDurationContainer->_pTrackdurationInfoVec);

        PV_MP4_FF_DELETE(NULL, TrackDurationContainer, _pTrackDurationContainer);
    }

    // Delete user data if present
    if (_puserDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, UserDataAtom, _puserDataAtom);
    }

    if (_pFileTypeAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, FileTypeAtom, _pFileTypeAtom);
    }

    if (_movieFragmentFilePtr != NULL)
    {
        if (_movieFragmentFilePtr->IsOpen())
        {
            AtomUtils::CloseMP4File(_movieFragmentFilePtr);
        }
        oscl_free(_movieFragmentFilePtr);
    }
    if (_pID3Parser)
    {
        PV_MP4_FF_DELETE(null, PVID3ParCom, _pID3Parser);
        _pID3Parser = NULL;
    }
}


uint64 Mpeg4File::getMovieDuration() const
{
    uint64 overallMovieDuration = 0;
    uint32 id = 0;
    if (_isMovieFragmentsPresent)
    {
        overallMovieDuration = _pmovieAtom->getMovieFragmentDuration();
        if (Oscl_Int64_Utils::get_uint64_lower32(overallMovieDuration) != 0)
        {
            return overallMovieDuration;
        }
        else if (_parsing_mode == 0)
        {
            uint numTracks = _pmovieAtom->getNumTracks();
            uint32 *trackList  = (uint32 *) oscl_malloc(sizeof(uint32) * numTracks);
            if (! trackList)
                return 0;	// malloc failure
            _pmovieAtom->getTrackWholeIDList(trackList);
            uint32 prevtrackDuration = 0, trackduration = 0;
            for (uint32 i = 0; i < numTracks; i++)
            {
                TrackDurationInfo* pTrackDurationInfo = (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i];
                trackduration = pTrackDurationInfo->trackDuration;
                if (prevtrackDuration > trackduration)
                {
                    trackduration = prevtrackDuration;
                }
                else
                {
                    prevtrackDuration = trackduration;
                    id = trackList[i];
                }
            }
            Oscl_Int64_Utils::set_uint64(overallMovieDuration, 0, trackduration);

            TrackAtom *trackAtom = NULL;
            uint32 mediaTimeScale = 0xFFFFFFFE;

            if (_pmovieAtom != NULL)
            {
                trackAtom = _pmovieAtom->getTrackForID(id);
            }
            if (trackAtom != NULL)
            {
                mediaTimeScale = trackAtom->getMediaTimescale();
                if (mediaTimeScale == 0)
                {
                    // unlikely : getMediaTimescale can return 0
                    mediaTimeScale = 0xFFFFFFFE;
                }
            }

            overallMovieDuration  = (overallMovieDuration / (uint64)mediaTimeScale) * (uint64)getMovieTimescale();
            oscl_free(trackList);
            return overallMovieDuration;
        }
        else
        {
            return overallMovieDuration;
        }
    }
    else if (_pmovieAtom != NULL)
    {
        // Get the overall duration of the Mpeg-4 presentation
        return _pmovieAtom->getDuration();
    }
    return 0;
}

uint64 Mpeg4File::getMovieFragmentDuration() const
{
    if (_pmovieAtom != NULL)
    {
        return _pmovieAtom->getMovieFragmentDuration();
    }
    else
        return 0;
}

uint32 Mpeg4File::getTimestampForSampleNumber(uint32 id, uint32 sampleNumber)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);

        if (trackAtom != NULL)
        {
            return trackAtom->getTimestampForSampleNumber(sampleNumber);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

int32 Mpeg4File::getSampleSizeAt(uint32 id, int32 sampleNum)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);

        if (trackAtom != NULL)
        {
            return (trackAtom->getSampleSizeAt(sampleNum));
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

uint64 Mpeg4File::getTrackMediaDurationForMovie(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }
    if (trackAtom != NULL)
    {
        return trackAtom->getTrackDuration();
    }
    else
    {
        return 0;
    }

}
// From TrackHeader
uint64 Mpeg4File::getTrackDuration(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }
    if (_isMovieFragmentsPresent)
    {
        if (_parsing_mode)
            return _pmovieAtom->getMovieFragmentDuration();
        else
        {
            int32 numTracks = _pmovieAtom->getNumTracks();
            uint32 *trackList  = (uint32 *) oscl_malloc(sizeof(uint32) * numTracks);
            if (!trackList)
                return 0;	// malloc failed
            _pmovieAtom->getTrackWholeIDList(trackList);
            uint64 trackduration = 0;
            for (int32 i = 0; i < numTracks; i++)
            {
                if (trackList[i] == id)
                {
                    TrackDurationInfo* pTrackDurationInfo = (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i];
                    oscl_free(trackList);
                    return trackduration = pTrackDurationInfo->trackDuration;
                }
            }
            oscl_free(trackList);
        }
    }
    if (trackAtom != NULL)
    {
        return trackAtom->getTrackDuration();
    }
    else
    {
        return 0;
    }
}

// From TrackReference
uint32  Mpeg4File::trackDependsOn(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->dependsOn();
    }
    else
    {
        return 0;
    }

}

// From MediaHeader
uint64 Mpeg4File::getTrackMediaDuration(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }
    if (_isMovieFragmentsPresent)
    {
        if (_parsing_mode)
            return _pmovieAtom->getMovieFragmentDuration();
        else
        {
            int numTracks = _pmovieAtom->getNumTracks();
            uint32 *trackList  = (uint32 *) oscl_malloc(sizeof(uint32) * numTracks);
            if (!trackList)
                return 0;	// malloc failed
            _pmovieAtom->getTrackWholeIDList(trackList);
            uint32 trackduration = 0;
            for (int32 i = 0; i < numTracks; i++)
            {
                if (trackList[i] == id)
                {
                    TrackDurationInfo* pTrackDurationInfo = (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i];
                    oscl_free(trackList);
                    return trackduration = pTrackDurationInfo->trackDuration;
                }
            }
            oscl_free(trackList);
        }
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getMediaDuration();
    }
    else
    {
        return 0;
    }
}

uint32 Mpeg4File::getTrackMediaTimescale(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFFFFFF);
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getMediaTimescale();
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFFFFFF);
    }
}

uint16 Mpeg4File::getTrackLangCode(uint32 id)
{

    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFF);
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getLanguageCode();
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFF);
    }
}

// From Handler
uint32 Mpeg4File::getTrackMediaType(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFFFFFF);
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getMediaType();
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFFFFFF);
    }

}

// From SampleDescription
int32 Mpeg4File::getTrackNumSampleEntries(uint32 id)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getNumSampleEntries();
    }
    else
    {
        return 0;
    }
}

// From DecoderConfigDescriptor
DecoderSpecificInfo *Mpeg4File::getTrackDecoderSpecificInfo(uint32 id)
{
    TrackAtom *trackAtom;
    if (_pmovieAtom != NULL)
    {
        trackAtom = _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return NULL;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getDecoderSpecificInfo();
    }
    else
    {
        return NULL;
    }

}

// From DecoderConfigDescriptor
DecoderSpecificInfo *
Mpeg4File::getTrackDecoderSpecificInfoAtSDI(uint32 trackID, uint32 index)
{
    if (_pmovieAtom != NULL)
    {
        return (_pmovieAtom->getTrackDecoderSpecificInfoAtSDI(trackID, index));
    }
    else
    {
        return NULL;
    }
}

uint8 *Mpeg4File::getTrackDecoderSpecificInfoContent(uint32 id)
{
    DecoderSpecificInfo	*decoderSpecificInfo;
    decoderSpecificInfo = getTrackDecoderSpecificInfo(id);

    if (decoderSpecificInfo != NULL)
    {
        return decoderSpecificInfo->getInfo();
    }
    else
    {
        return NULL;
    }
}

uint32 Mpeg4File::getTrackDecoderSpecificInfoSize(uint32 id)
{
    DecoderSpecificInfo	*decoderSpecificInfo;
    decoderSpecificInfo = getTrackDecoderSpecificInfo(id);

    if (decoderSpecificInfo != NULL)
    {
        return decoderSpecificInfo->getInfoSize();
    }
    else
    {
        return 0;
    }
}


void Mpeg4File::getTrackMIMEType(uint32 id, OSCL_String& aMimeType) // Based on OTI value
{
    TrackAtom *trackAtom = NULL;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);
    }

    if (trackAtom != NULL)
    {
        trackAtom->getMIMEType(aMimeType);
    }
}


int32 Mpeg4File::getTrackMaxBufferSizeDB(uint32 id)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getMaxBufferSizeDB();
    }
    else
    {
        return 0;
    }
}

int32 Mpeg4File::getTrackAverageBitrate(uint32 id)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getAverageBitrate();
    }
    else
    {
        return 0;
    }
}

// PASP Box
//Hspacing
uint32 Mpeg4File::getHspacing(uint32 id)
{

    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getHspacing();
    }
    else
    {
        return 0;
    }
}

//Vspacing
uint32 Mpeg4File::getVspacing(uint32 id)
{

    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(id);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getVspacing();
    }
    else
    {
        return 0;
    }
}


uint32
Mpeg4File::getMovieTimescale() const
{
    if (_pmovieAtom != NULL)
    {
        // Set the overall timescale of the Mpeg-4 presentation
        return _pmovieAtom->getTimeScale();
    }
    else
    {
        // RETURN UNDEFINED VALUE
        return (0xFFFFFFFF);
    }
}

/* ======================================================================== */
bool
Mpeg4File::IsMobileMP4()
{
    bool oMMP4 = false;

    if (_pFileTypeAtom != NULL)
    {
        uint32 majorBrand = _pFileTypeAtom->getMajorBrand();

        if (majorBrand != MOBILE_MP4)
        {
            Oscl_Vector<uint32, OsclMemAllocator> *_compatibleBrand =
                _pFileTypeAtom->getCompatibleBrand();
            if (_compatibleBrand != NULL)
            {
                for (uint32 i = 0; i < _compatibleBrand->size(); i++)
                {
                    uint32 brand = (*_compatibleBrand)[i];

                    if (brand == MOBILE_MP4)
                    {
                        oMMP4 = true;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            oMMP4 = true;
        }
    }
    else
    {
        return false;
    }

    if (oMMP4 == true)
    {
        if (!(_pmovieAtom->checkMMP4()))
        {
            return false;
        }
    }

    return (oMMP4);
}

uint8
Mpeg4File::parseBufferAndGetNumAMRFrames(uint8* buffer, uint32 size)
{
    uint32 inputBufferSize = size;
    uint8* inputPtr = buffer;
    uint8 numArmFrames = 0;

    if (((int32)(size) <= 0) ||
            (buffer == NULL))
    {
        return 0;
    }

    uint8 aFrameSizes[16] = {12, 13, 15, 17, 19, 20, 26, 31,
                             5,  0,  0,  0,  0,  0,  0,  0
                            };

    while (inputBufferSize > 0)
    {
        uint8 toc_byte = *(inputPtr);

        uint8 frame_type = (uint8)((toc_byte >> 3) & 0x0F);

        inputPtr        += 1;
        inputBufferSize -= 1;

        if ((frame_type > 8) && (frame_type != 15))
        {
            return 0;
        }

        numArmFrames++;
        inputPtr        += aFrameSizes[(uint16)frame_type];
        inputBufferSize -= aFrameSizes[(uint16)frame_type];
    }
    return (numArmFrames);
}


uint32 Mpeg4File::getTrackLevelOMA2DRMInfoSize(uint32 trackID)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(trackID);
    }
    else
    {
        return 0;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getTrackLevelOMA2DRMInfoSize();
    }
    else
    {
        return 0;
    }
}

uint8* Mpeg4File::getTrackLevelOMA2DRMInfo(uint32 trackID)
{
    TrackAtom *trackAtom;

    if (_pmovieAtom != NULL)
    {
        trackAtom =  _pmovieAtom->getTrackForID(trackID);
    }
    else
    {
        return NULL;
    }

    if (trackAtom != NULL)
    {
        return trackAtom->getTrackLevelOMA2DRMInfo();
    }
    else
    {
        return NULL;
    }
}


MP4_ERROR_CODE
Mpeg4File::RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
        uint32 aFileOffset,
        OsclAny* aContextData)
{
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Mpeg4File::RequestReadCapacityNotification In Offset %d", aFileOffset));
    uint32 capacity = 0;
    if (_commonFilePtr != NULL)
    {
        uint32 currPos = (uint32)(AtomUtils::getCurrentFilePosition(_commonFilePtr));
        if (aFileOffset > currPos)
        {
            capacity = (aFileOffset - currPos);
            bool retVal =
                _commonFilePtr->_pvfile.RequestReadCapacityNotification(aObserver,
                        capacity,
                        aContextData);
            if (retVal)
            {
                return EVERYTHING_FINE;
            }
            else
            {
                return DEFAULT_ERROR;
            }
        }
        return SUFFICIENT_DATA_IN_FILE;
    }
    return DEFAULT_ERROR;
}


MP4_ERROR_CODE
Mpeg4File::GetCurrentFileSize(uint32& aFileSize)
{
    aFileSize = 0;
    if (AtomUtils::getCurrentFileSize(_commonFilePtr, aFileSize) == true)
    {
        return EVERYTHING_FINE;
    }
    if (_commonFilePtr == NULL && _fileSize != 0)
    {
        aFileSize = _fileSize;
        return EVERYTHING_FINE;
    }
    return DEFAULT_ERROR;
}

int32 Mpeg4File::getNextBundledAccessUnits(const uint32 trackID,
        uint32 *n,
        GAU    *pgau)
{
    uint32 samplesTobeRead;
    samplesTobeRead = *n;
    uint32 totalSampleRead = 0;
    if (getNumTracks() == 0)
    {
        return -1;
    }

    if (_pmovieAtom != NULL)
    {
        int32 ret = _pmovieAtom->getNextBundledAccessUnits(trackID, n, pgau);
        if (ret == END_OF_TRACK)
        {
            if (!_isMovieFragmentsPresent)
                return ret;

            totalSampleRead += *n;
            bool oAllMoofExhausted = false;
            bool oAllMoofParsed = false;

            if (_parsing_mode == 0)
            {
                if (_pMovieFragmentAtomVec != NULL && _isMovieFragmentsPresent)
                {
                    if (samplesTobeRead >= *n)
                        *n = samplesTobeRead - *n;
                }
                else
                    return ret;

                int32 return1 = 0;
                while (_movieFragmentIdx[trackID] < _pMovieFragmentAtomVec->size())
                {
                    uint32 movieFragmentIdx = _movieFragmentIdx[trackID];
                    MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[movieFragmentIdx];
                    if (pMovieFragmentAtom != NULL)
                    {
                        if ((uint32)pMovieFragmentAtom->getSequenceNumber() == _movieFragmentSeqIdx[trackID])
                        {
                            TrackFragmentAtom *trackfragment = pMovieFragmentAtom->getTrackFragmentforID(trackID);
                            if (trackfragment != NULL)
                            {
                                if (trackfragment->getTrackId() == trackID)
                                {
                                    return1 = pMovieFragmentAtom->getNextBundledAccessUnits(trackID, n, totalSampleRead, pgau);
                                    totalSampleRead += *n;
                                    if (return1 != END_OF_TRACK)
                                    {
                                        *n = totalSampleRead;
                                        return return1;
                                    }
                                    else
                                    {
                                        _movieFragmentSeqIdx[trackID]++;
                                        if (samplesTobeRead >= *n)
                                        {
                                            samplesTobeRead = samplesTobeRead - *n;
                                            *n = samplesTobeRead;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    _movieFragmentIdx[trackID]++;
                }
                if (return1 == END_OF_TRACK)
                {
                    *n = totalSampleRead;
                    _movieFragmentIdx[trackID] = 0;
                    return return1;
                }
            }
            else
            {
                int32 return1 = 0;
                while (!oAllMoofExhausted)
                {
                    if (oAllMoofParsed && (_pMovieFragmentAtomVec->size() < _movieFragmentIdx[trackID]))
                    {
                        oAllMoofExhausted = true;
                        *n = 0;
                        break;
                    }

                    while (!oAllMoofParsed)
                    {
                        if (moofParsingCompleted)
                        {
                            uint32 moofIndex = 0;
                            bool moofToBeParsed = false;
                            if (_pMovieFragmentAtomVec->size() > _movieFragmentIdx[trackID])
                            {
                                MovieFragmentAtom *pMovieFragmentAtom = NULL;
                                uint32 idx = _movieFragmentIdx[trackID];
                                pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[_movieFragmentIdx[trackID]];
                                if (pMovieFragmentAtom == NULL)
                                {
                                    isResetPlayBackCalled = true;
                                    moofToBeParsed = true;
                                    moofIndex = _movieFragmentIdx[trackID];
                                }
                                else if (isResetPlayBackCalled)
                                {
                                    isResetPlayBackCalled = false;

                                    // if moofs are already parsed, so go to the end of MOOF Vector.

                                    uint32 currFilePos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                    if (currFilePos < _ptrMoofEnds)
                                    {
                                        uint32 offset = (_ptrMoofEnds - currFilePos);
                                        AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, offset);
                                    }
                                    else if (currFilePos == _ptrMoofEnds)
                                    {
                                        // no need to seek the File Pointer
                                    }
                                    else
                                    {
                                        AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                                    }

                                    idx = currMoofNum - 1;
                                    uint32 i = idx + 1;
                                    while (i < _pMovieFragmentAtomVec->size())
                                    {
                                        idx++;
                                        pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                                        if (pMovieFragmentAtom == NULL)
                                        {
                                            uint32 moof_start_offset = (*_pMoofOffsetVec)[idx-1];
                                            AtomUtils::seekFromStart(_movieFragmentFilePtr, moof_start_offset);
                                            uint32 atomType = UNKNOWN_ATOM;
                                            uint32 atomSize = 0;
                                            AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                            if (atomType == MOVIE_FRAGMENT_ATOM)
                                            {
                                                atomSize -= DEFAULT_ATOM_SIZE;
                                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                                _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                            }
                                            isResetPlayBackCalled = true;
                                            moofToBeParsed = true;
                                            moofIndex = idx;
                                            break;
                                        }
                                        pMovieFragmentAtom->resetPlayback();
                                        i++;
                                    }
                                    uint32 moof_start_offset = (*_pMoofOffsetVec)[idx];
                                    AtomUtils::seekFromStart(_movieFragmentFilePtr, moof_start_offset);
                                    uint32 atomType = UNKNOWN_ATOM;
                                    uint32 atomSize = 0;
                                    AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                    if (atomType == MOVIE_FRAGMENT_ATOM)
                                    {
                                        atomSize -= DEFAULT_ATOM_SIZE;
                                        AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                        _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                    }
                                }
                            }

                            uint32 fileSize = 0;
                            AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                            uint32 currFilePos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            if (currFilePos < _ptrMoofEnds)
                            {
                                uint32 offset = (_ptrMoofEnds - currFilePos);
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, offset);
                            }
                            else
                            {
                                AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                            }
                            uint32 filePointer = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;

                            while (count > 0)
                            {
                                uint32 atomType = UNKNOWN_ATOM;
                                uint32 atomSize = 0;
                                uint32 currPos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                if ((currPos + atomSize) > fileSize)
                                {
                                    AtomUtils::seekFromStart(_movieFragmentFilePtr, currPos);
                                    if (_movieFragmentIdx[trackID] < _pMovieFragmentAtomVec->size())
                                    {
                                        // dont report insufficient data as we still have a moof/moofs to
                                        // retrieve data. So just go and retrieve the data.
                                        break;
                                    }
                                    else
                                    {
                                        // We have run out of MOOF atoms so report insufficient data.
                                        return 	INSUFFICIENT_DATA;
                                    }
                                }

                                if (atomType == MOVIE_FRAGMENT_ATOM)
                                {
                                    uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                    moofStartOffset -= DEFAULT_ATOM_SIZE;
                                    parseMoofCompletely = false;
                                    moofSize = atomSize;
                                    moofType = atomType;
                                    moofCount = count;
                                    _ptrMoofEnds = moofStartOffset + atomSize;

                                    PV_MP4_FF_NEW(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, (_movieFragmentFilePtr, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), _pMovieFragmentAtom);
                                    moofSize = atomSize;
                                    moofPtrPos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);

                                    currMoofNum = _pMovieFragmentAtom->getSequenceNumber();
                                    if (moofToBeParsed)
                                    {
                                        (*_pMovieFragmentAtomVec)[moofIndex] = _pMovieFragmentAtom;
                                        (*_pMoofOffsetVec)[moofIndex] = moofStartOffset;
                                    }
                                    else
                                    {
                                        _pMoofOffsetVec->push_back(moofStartOffset);
                                        _pMovieFragmentAtomVec->push_back(_pMovieFragmentAtom);
                                    }

                                    if (moofParsingCompleted)
                                    {
                                        if (!_pMovieFragmentAtom->MP4Success())
                                        {
                                            _success = false;
                                            _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                            oAllMoofExhausted = true;
                                            break;
                                        }
                                        _pMovieFragmentAtom->setParent(this);
                                        count -= _pMovieFragmentAtom->getSize();

                                        break;
                                    }

                                    break;
                                }
                                else if (atomType == MEDIA_DATA_ATOM)
                                {
                                    if (atomSize == 1)
                                    {
                                        uint64 largeSize = 0;
                                        AtomUtils::read64(_movieFragmentFilePtr, largeSize);
                                        uint32 size =
                                            Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                                        count -= size;
                                        size -= 8; //for large size
                                        size -= DEFAULT_ATOM_SIZE;
                                        AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, size);
                                    }
                                    else
                                    {
                                        if (atomSize < DEFAULT_ATOM_SIZE)
                                        {
                                            _success = false;
                                            _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                                            oAllMoofExhausted = true;
                                            break;
                                        }
                                        if (count < (int32)atomSize)
                                        {
                                            _success = false;
                                            _mp4ErrorCode = INSUFFICIENT_DATA;
                                            ret = _mp4ErrorCode;
                                            oAllMoofExhausted = true;
                                            AtomUtils::seekFromStart(_movieFragmentFilePtr, currPos);
                                            break;
                                        }
                                        count -= atomSize;
                                        atomSize -= DEFAULT_ATOM_SIZE;
                                        AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                    }
                                }

                                else
                                {
                                    if (count > 0)
                                    {
                                        count -= atomSize;
                                        atomSize -= DEFAULT_ATOM_SIZE;
                                        AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                    }

                                }
                            }
                            if (count <= 0)
                            {
                                oAllMoofParsed = true;
                                break;
                            }
                            break;
                        }
                        else if (!moofParsingCompleted)
                        {
                            if (currMoofNum != (uint32) _pMovieFragmentAtom->getSequenceNumber())
                            {
                                uint32 size = _pMovieFragmentAtomVec->size();
                                _pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[size - 1];
                            }
                            uint32 currPos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            if (currPos > moofPtrPos)
                            {
                                uint32 offset = (currPos - moofPtrPos);
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, -((int32)offset));
                            }
                            else
                            {
                                uint32 offset = (moofPtrPos - currPos);
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, offset);
                            }

                            _pMovieFragmentAtom->ParseMoofAtom(_movieFragmentFilePtr, moofSize, moofType, _pTrackDurationContainer, _pTrackExtendsAtomVec, moofParsingCompleted, countOfTrunsParsed);
                            moofPtrPos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            if (moofParsingCompleted)
                            {
                                if (!_pMovieFragmentAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                    oAllMoofExhausted = true;
                                    break;
                                }
                                _pMovieFragmentAtom->setParent(this);
                                moofCount -= _pMovieFragmentAtom->getSize();
                            }

                            if (currPos > moofPtrPos)
                            {
                                uint32 offset = (currPos - moofPtrPos);
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, offset);
                            }
                            else
                            {
                                uint32 offset = (moofPtrPos - currPos);
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, -((int32)offset));
                            }

                            if (moofCount <= 0)
                            {
                                oAllMoofParsed = true;
                                break;
                            }
                            break;
                        }
                    }

                    if (return1 != END_OF_TRACK)
                    {
                        if (samplesTobeRead >= *n)
                            *n = samplesTobeRead - *n;
                    }

                    uint32 movieFragmentIdx = _movieFragmentIdx[trackID];
                    MovieFragmentAtom *pMovieFragmentAtom = NULL;

                    if (movieFragmentIdx < _pMovieFragmentAtomVec->size())
                        pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[movieFragmentIdx];

                    if (pMovieFragmentAtom != NULL)
                    {
                        uint32 seqNum = pMovieFragmentAtom->getSequenceNumber();
                        if (seqNum == _movieFragmentSeqIdx[trackID])
                        {
                            TrackFragmentAtom *trackfragment = pMovieFragmentAtom->getTrackFragmentforID(trackID);
                            if (trackfragment != NULL)
                            {
                                if (trackfragment->getTrackId() == trackID)
                                {
                                    return1 = pMovieFragmentAtom->getNextBundledAccessUnits(trackID, n, totalSampleRead, pgau);
                                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Mpeg4File::getNextBundledAccessUnits return %d", return1));
                                    totalSampleRead += *n;
                                    if (return1 != END_OF_TRACK)
                                    {
                                        *n = totalSampleRead;
                                        return return1;
                                    }
                                    else
                                    {
                                        _movieFragmentSeqIdx[trackID]++;
                                        if (samplesTobeRead >= *n)
                                        {
                                            samplesTobeRead = samplesTobeRead - *n;
                                            *n = samplesTobeRead;
                                            if (movieFragmentIdx < _pMovieFragmentAtomVec->size())
                                            {
                                                _movieFragmentIdx[trackID]++;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                uint32 movieFragmentIdx2 = _movieFragmentIdx[trackID];

                                if (oAllMoofParsed)
                                {
                                    // look for all moofs
                                    if (movieFragmentIdx2 < _pMovieFragmentAtomVec->size())
                                    {
                                        _movieFragmentIdx[trackID]++;
                                        _movieFragmentSeqIdx[trackID]++;
                                    }
                                    else
                                    {
                                        return END_OF_TRACK;
                                    }

                                }
                                else
                                {
                                    if (movieFragmentIdx2 < _pMovieFragmentAtomVec->size())
                                    {
                                        if ((movieFragmentIdx2 == (_pMovieFragmentAtomVec->size() - 1)) && moofParsingCompleted)
                                        {
                                            _movieFragmentIdx[trackID]++;
                                            _movieFragmentSeqIdx[trackID]++;
                                        }
                                        else if (movieFragmentIdx2 < (_pMovieFragmentAtomVec->size() - 1))
                                        {
                                            _movieFragmentIdx[trackID]++;
                                            _movieFragmentSeqIdx[trackID]++;
                                            *n = 0;
                                            return NO_SAMPLE_IN_CURRENT_MOOF;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            uint32 movieFragmentIdx2 = _movieFragmentIdx[trackID];

                            if (oAllMoofParsed)
                            {
                                // look for all moofs
                                if (movieFragmentIdx2 < _pMovieFragmentAtomVec->size())
                                {
                                    _movieFragmentIdx[trackID]++;
                                    _movieFragmentSeqIdx[trackID]++;
                                }
                                else
                                {
                                    return END_OF_TRACK;
                                }

                            }
                            else
                            {
                                if (movieFragmentIdx2 < _pMovieFragmentAtomVec->size())
                                {
                                    if ((movieFragmentIdx2 == (_pMovieFragmentAtomVec->size() - 1)) && moofParsingCompleted)
                                    {
                                        _movieFragmentIdx[trackID]++;
                                        _movieFragmentSeqIdx[trackID]++;
                                    }
                                    else if (movieFragmentIdx2 < (_pMovieFragmentAtomVec->size() - 1))
                                    {
                                        _movieFragmentIdx[trackID]++;
                                        _movieFragmentSeqIdx[trackID]++;
                                        *n = 0;
                                        return NO_SAMPLE_IN_CURRENT_MOOF;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (movieFragmentIdx < _pMovieFragmentAtomVec->size())
                        {
                            _movieFragmentIdx[trackID]++;
                            _movieFragmentSeqIdx[trackID]++;
                        }
                        else if (oAllMoofParsed)
                        {
                            _movieFragmentIdx[trackID]++;
                            _movieFragmentSeqIdx[trackID]++;
                        }

                    }
                }
            }
        }
        return ret;
    }
    return -1;
}

MovieFragmentAtom * Mpeg4File::getMovieFragmentForTrackId(uint32 id)
{
    MovieFragmentAtom *movieFragmentAtom = NULL;
    uint32 i = 0;

    if (_pMovieFragmentAtomVec == NULL)
        return NULL;

    while (i < _pMovieFragmentAtomVec->size())
    {
        movieFragmentAtom = (*_pMovieFragmentAtomVec)[i];
        if (movieFragmentAtom != NULL)
        {
            TrackFragmentAtom *trackfragment = movieFragmentAtom->getTrackFragmentforID(id);
            if (trackfragment != NULL)
            {
                if (trackfragment->getTrackId() == id)
                {
                    return movieFragmentAtom;
                }
            }
        }
        i++;
    }
    return NULL;
}

void Mpeg4File::populateTrackDurationVec()
{
    uint32 trackDuration = 0;
    if (_pmovieAtom != NULL)
    {
        uint32 ids[256];
        uint32 size = 256;
        _pmovieAtom->getTrackIDList(ids, size);
        int32 numtracks = _pmovieAtom->getNumTracks();
        PV_MP4_FF_NEW(fp->auditCB, TrackDurationContainer, (), _pTrackDurationContainer);
        PV_MP4_FF_NEW(fp->auditCB, trackDurationInfoVecType, (), _pTrackDurationContainer->_pTrackdurationInfoVec);
        for (int32 i = 0; i < numtracks; i++)
        {
            uint32 trackID = ids[i];
            TrackDurationInfo *trackinfo = NULL;
            trackDuration = Oscl_Int64_Utils::get_uint64_lower32(_pmovieAtom->getTrackMediaDuration(trackID));
            PV_MP4_FF_NEW(fp->auditCB, TrackDurationInfo, (trackDuration, trackID), trackinfo);
            (*_pTrackDurationContainer->_pTrackdurationInfoVec).push_back(trackinfo);
            _movieFragmentIdx[trackID] = 0;
            _peekMovieFragmentIdx[trackID] = 0;
            _movieFragmentSeqIdx[trackID] = 1;
            _peekMovieFragmentSeqIdx[trackID] = 1;
        }
    }
}

uint32 Mpeg4File::GetByteOffsetToStartOfAudioFrames()
{
    return _pID3Parser->GetByteOffsetToStartOfAudioFrames();

}

void Mpeg4File::GetID3MetaData(PvmiKvpSharedPtrVector &id3Frames)
{
    _pID3Parser->GetID3Frames(id3Frames);

}

bool Mpeg4File::IsID3Frame(const OSCL_String& frameType)
{
    return _pID3Parser->IsID3FrameAvailable(frameType);
}

void Mpeg4File::GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame)
{
    _pID3Parser->GetID3Frame(aFrameType, aFrame);
}

PVID3Version Mpeg4File::GetID3Version() const
{
    return _pID3Parser->GetID3Version();
}

void Mpeg4File::parseID3Header(MP4_FF_FILE *aFile)
{
    int32 curpos = AtomUtils::getCurrentFilePosition(aFile);
    AtomUtils::seekFromStart(aFile, 0);
    _pID3Parser->ParseID3Tag(&aFile->_pvfile);
    AtomUtils::seekFromStart(aFile, curpos);
}

uint32 Mpeg4File::getContentType()
{
    PVContentTypeAtom *pAtom = NULL;

    if (_puserDataAtom != NULL)
    {
        pAtom =
            (PVContentTypeAtom*) _puserDataAtom->getAtomOfType(PV_CONTENT_TYPE_ATOM);

        if (pAtom != NULL)
        {
            return pAtom->getContentType();
        }
        else
        {
            if (_oPVContent)
            {
                //Old PV Content, that doesnt have this atom
                //All such content is non-interleaved, with meta data
                //towards the very end
                return (DEFAULT_AUTHORING_MODE);
            }
        }
    }

    //Third party content
    return (0xFFFFFFFF);
}


MP4_ERROR_CODE Mpeg4File::getKeyMediaSampleNumAt(uint32 aTrackId,
        uint32 aKeySampleNum,
        GAU    *pgau)
{
    if (_pmovieAtom == NULL)
    {
        return READ_SAMPLE_TABLE_ATOM_FAILED;
    }
    MP4_ERROR_CODE ret = _pmovieAtom->getKeyMediaSampleNumAt(aTrackId, aKeySampleNum, pgau);
    if (ret == READ_FAILED)
    {
        uint32 totalSampleRead = 0;
        if (_isMovieFragmentsPresent)
        {
            uint32 n = 1;
            uint32 movieFragmentIdx = _movieFragmentIdx[aTrackId];
            MovieFragmentAtom *pMovieFragmentAtom = NULL;

            if (movieFragmentIdx < _pMovieFragmentAtomVec->size())
                pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[movieFragmentIdx];

            if (pMovieFragmentAtom != NULL)
            {
                uint32 seqNum = pMovieFragmentAtom->getSequenceNumber();
                if (seqNum == _movieFragmentSeqIdx[aTrackId])
                {
                    TrackFragmentAtom *trackfragment = pMovieFragmentAtom->getTrackFragmentforID(aTrackId);
                    if (trackfragment != NULL)
                    {
                        if (trackfragment->getTrackId() == aTrackId)
                        {
                            return (MP4_ERROR_CODE)pMovieFragmentAtom->getNextBundledAccessUnits(aTrackId, &n, totalSampleRead, pgau);
                        }
                    }
                }
            }
        }
        return READ_FAILED;
    }
    else
    {
        return ret;
    }
}



int32 Mpeg4File::getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset , uint32 jitterbuffertimeinmillisec)
{
    int32 ret =  DEFAULT_ERROR;
    uint32 sigmaAtomSize = 0;
    if (_pmovieAtom != NULL)
    {
        ret = _pmovieAtom->getOffsetByTime(id, ts, sampleFileOffset);
        if (ret == DEFAULT_ERROR || ret == LAST_SAMPLE_IN_MOOV)
        {
            if (_isMovieFragmentsPresent)
            {
                uint32 sigmaTrafDuration = 0;

                for (uint32 idx = 0; idx < _pMovieFragmentAtomVec->size();idx++)
                {
                    MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                    if (pMovieFragmentAtom != NULL)
                    {
                        uint32 currTrafDuration = pMovieFragmentAtom->getCurrentTrafDuration(id);
                        if (currTrafDuration >= ts)
                        {
                            pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                            return pMovieFragmentAtom->getOffsetByTime(id, ts, sampleFileOffset);
                        }
                        sigmaTrafDuration += currTrafDuration;
                    }
                }

                if (_parsing_mode == 1)
                {
                    if (moofParsingCompleted)
                    {
                        // do nothing
                    }
                    else
                    {
                        if ((uint32)_pMovieFragmentAtom->getSequenceNumber() == _movieFragmentSeqIdx[id])
                        {
                            AtomUtils::seekFromStart(_movieFragmentFilePtr, moofPtrPos);

                            while (!moofParsingCompleted)
                            {
                                _pMovieFragmentAtom->ParseMoofAtom(_movieFragmentFilePtr, moofSize, moofType, _pTrackDurationContainer, _pTrackExtendsAtomVec, moofParsingCompleted, countOfTrunsParsed);
                            }

                            if (moofParsingCompleted)
                            {
                                if (!_pMovieFragmentAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                }
                                _pMovieFragmentAtom->setParent(this);
                                moofSize = _pMovieFragmentAtom->getSize();
                                moofCount -= _pMovieFragmentAtom->getSize();
                            }

                            uint32 currTrafDuration = _pMovieFragmentAtom->getCurrentTrafDuration(id);

                            if (currTrafDuration >= ts)
                            {
                                ret = _pMovieFragmentAtom->getOffsetByTime(id, ts, sampleFileOffset);
                                if (*sampleFileOffset == 0)
                                {
                                    // do nothing, continue parsing
                                }
                                else
                                {
                                    return ret;
                                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: ret Sample Offset=> Sample Offset= %d ret %d********@@@@@@@@@@@@@@@@", *sampleFileOffset, ret));
                                }
                            }
                            sigmaTrafDuration += currTrafDuration;
                        }
                        else
                        {
                            // This condition will only happen when the MovieFragmentAtomVec size is
                            // greater than 1.
                            uint32 i = _pMovieFragmentAtomVec->size();
                            _ptrMoofEnds = (*_pMoofOffsetVec)[i-2] + (*_pMovieFragmentAtomVec)[i-2]->getSize();
                            _pMoofOffsetVec->pop_back();
                            _pMovieFragmentAtomVec->pop_back();
                            PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                            parseMoofCompletely = true;
                            moofParsingCompleted = true;
                            moofSize = 0;
                            moofType = UNKNOWN_ATOM;
                            moofCount = 0;
                            moofPtrPos = 0;
                        }
                    }

                    uint32 fileSize = 0;
                    uint32 currfptr = 0;

                    AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime::FileSize %d Track ID %d ********@@@@@@@@@@@@@@@@", fileSize, id));

                    AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                    uint32 filePointer = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                    int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;
                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: TS= %d ********@@@@@@@@@@@@@@@@, count=%d, filePointer=%d", ts, count, filePointer));

                    while (count > 0)
                    {
                        uint32 atomType = UNKNOWN_ATOM;
                        uint32 atomSize = 0;
                        AtomUtils::Flush(_movieFragmentFilePtr);
                        AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: count=%d, AtomSize=%d, atomtype=%d", count, atomSize, atomType));
                        if (atomSize < DEFAULT_ATOM_SIZE)
                        {

                            ret = DEFAULT_ERROR;
                            break;
                        }
                        sigmaAtomSize += atomSize;
                        if (atomType == MOVIE_FRAGMENT_ATOM)
                        {
                            uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            moofStartOffset -= DEFAULT_ATOM_SIZE;
                            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: MovieFragmentAtom moofStartOffset=%d", moofStartOffset));

                            moofSize = atomSize;

                            if ((moofStartOffset + atomSize) > fileSize)
                            {
                                uint32 timeScale = _pmovieAtom->getTrackMediaTimescale(id);
                                if ((timeScale == 0) || (timeScale == 0xFFFFFFFF))
                                {
                                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: Invalid timeScale %d for Id %d", timeScale, id));
                                    return DEFAULT_ERROR;
                                }

                                uint32 trackPlayedSoFarInSec = ts / timeScale - jitterbuffertimeinmillisec / 1000;
                                uint32 rateOfDataUsageKbPerSec = 0;
                                if (trackPlayedSoFarInSec != 0)
                                {
                                    rateOfDataUsageKbPerSec = fileSize / trackPlayedSoFarInSec;
                                }
                                // estimate data for PVMF_MP4FFPARSER_PSEUDO_STREAMING_DURATION_IN_SEC
                                uint32 dataNeededAhead = (rateOfDataUsageKbPerSec * jitterbuffertimeinmillisec) / 1000;

                                *sampleFileOffset = moofStartOffset + atomSize + DEFAULT_ATOM_SIZE + dataNeededAhead;
                                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: Insufficient data ot get Sample Offset= %d ********@@@@@@@@@@@@@@@@", *sampleFileOffset));
                                ret =  EVERYTHING_FINE;
                                break;
                            }

                            _pMoofOffsetVec->push_back(moofStartOffset);
                            parseMoofCompletely = true;

                            PV_MP4_FF_NEW(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, (_movieFragmentFilePtr, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), _pMovieFragmentAtom);

                            if (!_pMovieFragmentAtom->MP4Success())
                            {

                                _success = false;
                                _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                break;
                            }
                            count -= _pMovieFragmentAtom->getSize();
                            _pMovieFragmentAtom->setParent(this);

                            _pMovieFragmentAtomVec->push_back(_pMovieFragmentAtom);
                            _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);

                            uint32 currTrafDuration = _pMovieFragmentAtom->getCurrentTrafDuration(id);

                            if (currTrafDuration >= ts)
                            {
                                ret = _pMovieFragmentAtom->getOffsetByTime(id, ts, sampleFileOffset);
                                if (*sampleFileOffset == 0)
                                {
                                    // do nothing, continue parsing
                                }
                                else
                                {
                                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: ret Sample Offset=> Sample Offset= %d ret %d********@@@@@@@@@@@@@@@@", *sampleFileOffset, ret));
                                    break;
                                }
                            }
                            sigmaTrafDuration += currTrafDuration;
                        }
                        else if (atomType == MEDIA_DATA_ATOM)
                        {

                            if (atomSize == 1)
                            {
                                uint64 largeSize = 0;
                                AtomUtils::read64(_movieFragmentFilePtr, largeSize);
                                uint32 size =
                                    Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                                count -= size;
                                size -= 8; //for large size
                                size -= DEFAULT_ATOM_SIZE;
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, size);
                            }
                            else
                            {
                                currfptr = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                if ((currfptr + atomSize) > fileSize)
                                {
                                    uint32 timeScale = _pmovieAtom->getTrackMediaTimescale(id);
                                    if ((timeScale == 0) || (timeScale == 0xFFFFFFFF))
                                    {
                                        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: Invalid timeScale %d for Id %d", timeScale, id));
                                        return DEFAULT_ERROR;
                                    }
                                    uint32 trackPlayedSoFarInSec = ts / timeScale - jitterbuffertimeinmillisec / 1000;
                                    uint32 rateOfDataUsageKbPerSec = 0;
                                    if (trackPlayedSoFarInSec != 0)
                                    {
                                        rateOfDataUsageKbPerSec = fileSize / trackPlayedSoFarInSec;
                                    }

                                    // estimate data for PVMF_MP4FFPARSER_PSEUDO_STREAMING_DURATION_IN_SEC
                                    uint32 dataNeededAhead = (rateOfDataUsageKbPerSec * jitterbuffertimeinmillisec) / 1000;
                                    *sampleFileOffset = currfptr + atomSize + moofSize + dataNeededAhead;
                                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "getOffsetByTime:: Insufficient data ot get Sample Offset= %d ********@@@@@@@@@@@@@@@@", *sampleFileOffset));
                                    ret = EVERYTHING_FINE;
                                    break;

                                }
                                count -= atomSize;
                                atomSize -= DEFAULT_ATOM_SIZE;
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                            }
                        }

                        else
                        {
                            if (count > 0)
                            {
                                count -= atomSize;
                                atomSize -= DEFAULT_ATOM_SIZE;
                                AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                            }
                        }
                    }
                    if (sigmaTrafDuration == 0)
                    {
                        AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                        *sampleFileOffset = fileSize + 1000;

                        ret = EVERYTHING_FINE;
                    }
                }
            }
            else if (ret == LAST_SAMPLE_IN_MOOV)
            {
                ret = EVERYTHING_FINE;
            }

        }

        return ret;
    }
    else
    {

        return ret;
    }
}

int32 Mpeg4File::getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32 *offsetBuf)
{
    if (_pmovieAtom != NULL)
    {
        uint32 requestedSamples = *num, delta = 0, returnedSampleFromMoov = 0;
        uint32 ret =  _pmovieAtom->getTimestampForRandomAccessPoints(id, num, tsBuf, numBuf, offsetBuf);
        if (ret == 1)
        {
            returnedSampleFromMoov = *num;
            if (requestedSamples != 0)
            {
                if (requestedSamples == returnedSampleFromMoov)
                    return ret;

                if (requestedSamples > returnedSampleFromMoov)
                {
                    delta = requestedSamples - returnedSampleFromMoov;
                }
            }

        }
        else
            delta = *num;

        if (_isMovieFragmentsPresent)
        {
            if (_pMovieFragmentRandomAccessAtomVec != NULL)
            { // Only one mfra possible in a clip so this loop will run only once
                for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                {
                    MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                    ret = pMovieFragmentRandomAccessAtom->getTimestampForRandomAccessPoints(id, &delta, tsBuf, numBuf, offsetBuf, returnedSampleFromMoov);
                    *num = delta;
                    return ret;
                }

            }
        }
        return ret;
    }
    return 0;
}

int32 Mpeg4File::getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
        uint32& numsamplestoget,
        uint32 howManyKeySamples)
{
    if (_pmovieAtom != NULL)
    {
        int32 ret = _pmovieAtom->getTimestampForRandomAccessPointsBeforeAfter(id, ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
        if (ret != 1)
        {
            if (_isMovieFragmentsPresent)
            {
                if (_pMovieFragmentRandomAccessAtomVec != NULL)
                { // Only one mfra possible in a clip so this loop will run only once
                    for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                    {
                        MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                        ret = pMovieFragmentRandomAccessAtom->getTimestampForRandomAccessPointsBeforeAfter(id, ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
                        return ret;
                    }

                }
            }
        }
        return ret;

    }
    else
    {
        return 0;
    }
}



void Mpeg4File::resetAllMovieFragments()
{
    uint32 trackDuration = 0;
    if (_isMovieFragmentsPresent)
    {
        if (_pMovieFragmentAtomVec != NULL)
        {
            int numTracks = _pmovieAtom->getNumTracks();
            uint32 *trackList  = (uint32 *) oscl_malloc(sizeof(uint32) * numTracks);
            if (!trackList)
                return;       // malloc failed

            _pmovieAtom->getTrackWholeIDList(trackList);
            for (int32 i = 0; i < numTracks; i++)
            {
                uint32 trackID = trackList[i];
                _movieFragmentIdx[trackID] = 0;
                _peekMovieFragmentIdx[trackID] = 0;
                _movieFragmentSeqIdx[trackID] = 1;
                _peekMovieFragmentSeqIdx[trackID] = 1;
                TrackDurationInfo *trackinfo = NULL;
                if (_pTrackDurationContainer != NULL)
                {
                    TrackDurationInfo *pTrackDurationInfo = (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i];
                    if (pTrackDurationInfo != NULL)
                    {
                        PV_MP4_FF_DELETE(NULL, TrackDurationInfo, pTrackDurationInfo);
                        pTrackDurationInfo = NULL;
                    }
                }
                trackDuration = Oscl_Int64_Utils::get_uint64_lower32(_pmovieAtom->getTrackMediaDuration(trackID));
                PV_MP4_FF_NEW(fp->auditCB, TrackDurationInfo, (trackDuration, trackID), trackinfo);
                (*_pTrackDurationContainer->_pTrackdurationInfoVec)[i] = trackinfo;
            }
            oscl_free(trackList);
            for (uint32 idx = 0; idx < _pMovieFragmentAtomVec->size(); idx++)
            {
                MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                if (pMovieFragmentAtom != NULL)
                    pMovieFragmentAtom->resetPlayback();

            }
        }
    }
}


uint32 Mpeg4File::resetPlayback(uint32 time, uint16 numTracks, uint32 *trackList, bool bResetToIFrame)
{
    OSCL_UNUSED_ARG(numTracks);

    uint32 modifiedTimeStamp = time;
    uint32 trackID = 0;

    uint32 retVal = 0;
    bool oMoofFound = false;

    uint32 convertedTS = 0;
    uint32 timestamp = 0, returnedTS = 0;

    uint32 moof_offset = 0, traf_number = 0, trun_number = 0, sample_num = 0;

    trackID = *trackList; //  numTracks is the track index in trackList
    if (getTrackMediaType(trackID) == MEDIA_TYPE_VISUAL)
    {
        _oVideoTrackPresent = true;
    }

    if (_isMovieFragmentsPresent)
    {
        if (_pMovieFragmentAtomVec->size() > 1)
        {
            // The boolean is used to reset all MOOFs to start after reposition. This should
            // be true only when number of MOOFs in MOOF vector queue is more than one.
            isResetPlayBackCalled = true;
        }
    }

    if (getTrackMediaType(trackID) == MEDIA_TYPE_VISUAL)
    {
        if (repositionFromMoof(time, trackID))
        {
            //moof
            modifiedTimeStamp = time;

            // convert modifiedTimeStamp (which is in ms) to the appropriate
            // media time scale
            MediaClockConverter mcc1(1000);
            mcc1.update_clock(modifiedTimeStamp);
            convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(trackID));
            if (oMfraFound)
            {
                for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                {
                    MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                    uint32 ret = pMovieFragmentRandomAccessAtom->getSyncSampleInfoClosestToTime(trackID, convertedTS, moof_offset, traf_number, trun_number, sample_num);
                    if (ret == 0)
                    {
                        if (moofParsingCompleted)
                        {
                            // do nothing
                        }
                        else
                        {
                            uint32 i = _pMovieFragmentAtomVec->size();
                            _pMoofOffsetVec->pop_back();
                            _pMovieFragmentAtomVec->pop_back();
                            PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                            parseMoofCompletely = true;
                            moofParsingCompleted = true;
                            moofSize = 0;
                            moofType = UNKNOWN_ATOM;
                            moofCount = 0;
                            moofPtrPos = 0;
                        }

                        for (uint32 idx = 0; idx < _pMoofOffsetVec->size();idx++)
                        {
                            uint32 moof_start_offset = (*_pMoofOffsetVec)[idx];
                            if (moof_start_offset == moof_offset)
                            {
                                _movieFragmentIdx[trackID] = idx;
                                _peekMovieFragmentIdx[trackID] = idx;
                                _movieFragmentSeqIdx[trackID] = (*_pMovieFragmentAtomVec)[idx]->getSequenceNumber();
                                _peekMovieFragmentSeqIdx[trackID] = _movieFragmentSeqIdx[trackID];
                                _pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                                currMoofNum = _pMovieFragmentAtom->getSequenceNumber();
                                oMoofFound = true;

                                AtomUtils::seekFromStart(_movieFragmentFilePtr, moof_offset);
                                uint32 atomType = UNKNOWN_ATOM;
                                uint32 atomSize = 0;
                                AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                if (atomType == MOVIE_FRAGMENT_ATOM)
                                {
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                    _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                }
                                break;
                            }
                        }

                        if (_parsing_mode == 1)
                        {
                            if (!oMoofFound)
                            {
                                uint32 fileSize = 0;
                                _ptrMoofEnds = moof_offset;
                                AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                                AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                                uint32 filePointer = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;

                                while (count > 0)
                                {
                                    uint32 atomType = UNKNOWN_ATOM;
                                    uint32 atomSize = 0;
                                    AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                    if (atomType == MOVIE_FRAGMENT_ATOM)
                                    {
                                        parseMoofCompletely = true;

                                        uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                        moofStartOffset -= DEFAULT_ATOM_SIZE;

                                        if (moofParsingCompleted)
                                        {
                                            // do nothing
                                        }
                                        else
                                        {
                                            uint32 i = _pMovieFragmentAtomVec->size();
                                            _pMoofOffsetVec->pop_back();
                                            _pMovieFragmentAtomVec->pop_back();
                                            PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                                            parseMoofCompletely = true;
                                            moofParsingCompleted = true;
                                            moofSize = 0;
                                            moofType = UNKNOWN_ATOM;
                                            moofCount = 0;
                                            moofPtrPos = 0;
                                        }

                                        PV_MP4_FF_NEW(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, (_movieFragmentFilePtr, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), _pMovieFragmentAtom);

                                        if (!_pMovieFragmentAtom->MP4Success())
                                        {
                                            _success = false;
                                            _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                            break;
                                        }

                                        _pMovieFragmentAtom->setParent(this);
                                        count -= _pMovieFragmentAtom->getSize();

                                        uint32 i = _pMovieFragmentAtomVec->size();

                                        MovieFragmentAtom *pMovieFragmentAtom = NULL;
                                        uint32 prevMoofSeqNum = 0;

                                        if (i > 0)
                                        {
                                            pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[i-1];

                                            if (pMovieFragmentAtom != NULL)
                                                prevMoofSeqNum = (*_pMovieFragmentAtomVec)[i-1]->getSequenceNumber();
                                        }

                                        currMoofNum = _pMovieFragmentAtom->getSequenceNumber();

                                        for (uint32 idx = prevMoofSeqNum; idx < currMoofNum - 1;idx++)
                                        {
                                            _pMovieFragmentAtomVec->push_back(NULL);
                                            _pMoofOffsetVec->push_back(0);
                                        }
                                        if (currMoofNum > i)
                                        {
                                            _pMoofOffsetVec->push_back(moofStartOffset);
                                            _pMovieFragmentAtomVec->push_back(_pMovieFragmentAtom);
                                        }
                                        else if ((*_pMovieFragmentAtomVec)[currMoofNum-1] == NULL)
                                        {
                                            (*_pMovieFragmentAtomVec)[currMoofNum-1] = _pMovieFragmentAtom;
                                            (*_pMoofOffsetVec)[currMoofNum-1] = moofStartOffset;
                                        }
                                        else
                                        {
                                            PV_MP4_FF_DELETE(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, _pMovieFragmentAtom);
                                            _pMovieFragmentAtom = NULL;
                                            break;

                                        }
                                        _movieFragmentSeqIdx[trackID] = currMoofNum;
                                        _movieFragmentIdx[trackID] = currMoofNum - 1;
                                        _peekMovieFragmentIdx[trackID] = currMoofNum - 1;
                                        _peekMovieFragmentSeqIdx[trackID] = currMoofNum;

                                        oMoofFound = true;

                                        _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                        break;
                                    }
                                    else if (atomType == MEDIA_DATA_ATOM)
                                    {
                                        if (atomSize == 1)
                                        {
                                            uint64 largeSize = 0;
                                            AtomUtils::read64(_movieFragmentFilePtr, largeSize);
                                            uint32 size =
                                                Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                                            count -= size;
                                            size -= 8; //for large size
                                            size -= DEFAULT_ATOM_SIZE;
                                            AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, size);
                                        }
                                        else
                                        {
                                            if (atomSize < DEFAULT_ATOM_SIZE)
                                            {
                                                _success = false;
                                                _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                                                break;
                                            }
                                            if (count < (int32)atomSize)
                                            {
                                                _success = false;
                                                _mp4ErrorCode = READ_FAILED;
                                                break;
                                            }
                                            count -= atomSize;
                                            atomSize -= DEFAULT_ATOM_SIZE;
                                            AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                        }
                                    }

                                    else
                                    {
                                        if (count > 0)
                                        {
                                            count -= atomSize;
                                            atomSize -= DEFAULT_ATOM_SIZE;
                                            AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                        }

                                    }
                                }
                            }

                        }

                        if (_pmovieAtom != NULL)
                            _pmovieAtom->resetTrackToEOT();

                        if (_pMovieFragmentAtom != NULL)
                            returnedTS = _pMovieFragmentAtom->resetPlayback(trackID, convertedTS, traf_number, trun_number, sample_num);
                    }
                    else
                    {
                        // Not a valid tfra entries, cannot reposition.
                        return 0;
                    }
                }

            }
            else
                return 0;

            // convert returnedTS (which is in media time scale) to the ms
            MediaClockConverter mcc(getTrackMediaTimescale(trackID));
            mcc.update_clock(returnedTS);
            timestamp = mcc.get_converted_ts(1000);

            if (timestamp <= modifiedTimeStamp)
            {
                modifiedTimeStamp = timestamp;
            }

        }
        else
        {
            if (_isMovieFragmentsPresent)
            {
                if (_pMovieFragmentAtomVec->size() > 0)
                {
                    if (moofParsingCompleted)
                    {
                        // do nothing
                    }
                    else
                    {
                        uint32 i = _pMovieFragmentAtomVec->size();
                        _pMoofOffsetVec->pop_back();
                        _pMovieFragmentAtomVec->pop_back();
                        PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                        parseMoofCompletely = true;
                        moofParsingCompleted = true;
                        moofSize = 0;
                        moofType = UNKNOWN_ATOM;
                        moofCount = 0;
                        moofPtrPos = 0;
                    }
                }
            }

            //movie
            if (_pmovieAtom != NULL)
            {
                resetAllMovieFragments();
                uint32 trackVideo = trackID;
                uint32 numTrackForVideo = 1;
                modifiedTimeStamp =  _pmovieAtom->resetPlayback(modifiedTimeStamp, numTrackForVideo, &trackVideo, bResetToIFrame);
            }
        }
    }
    retVal = modifiedTimeStamp;
    if ((getTrackMediaType(trackID) == MEDIA_TYPE_AUDIO) ||
            (getTrackMediaType(trackID) == MEDIA_TYPE_TEXT))
    {
        if (repositionFromMoof(time, trackID))
        {
            oMoofFound = false;
            //moof
            // convert modifiedTimeStamp (which is in ms) to the appropriate
            // media time scale
            MediaClockConverter mcc1(1000);
            mcc1.update_clock(modifiedTimeStamp);
            convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(trackID));
            if (oMfraFound)
            {
                for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                {
                    MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                    uint32 ret = pMovieFragmentRandomAccessAtom->getSyncSampleInfoClosestToTime(trackID, convertedTS, moof_offset, traf_number, trun_number, sample_num);
                    if (ret == 0)
                    {
                        if (moofParsingCompleted)
                        {
                            // do nothing
                        }
                        else
                        {
                            uint32 i = _pMovieFragmentAtomVec->size();
                            _pMoofOffsetVec->pop_back();
                            _pMovieFragmentAtomVec->pop_back();
                            PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                            parseMoofCompletely = true;
                            moofParsingCompleted = true;
                            moofSize = 0;
                            moofType = UNKNOWN_ATOM;
                            moofCount = 0;
                            moofPtrPos = 0;
                        }
                        //
                        for (idx = 0; idx < _pMoofOffsetVec->size();idx++)
                        {
                            uint32 moof_start_offset = (*_pMoofOffsetVec)[idx];
                            if (moof_start_offset == moof_offset)
                            {
                                _movieFragmentIdx[trackID] = idx;
                                _peekMovieFragmentIdx[trackID] = idx;
                                _movieFragmentSeqIdx[trackID] = (*_pMovieFragmentAtomVec)[idx]->getSequenceNumber();
                                _peekMovieFragmentSeqIdx[trackID] = _movieFragmentSeqIdx[trackID];
                                _pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                                currMoofNum = _pMovieFragmentAtom->getSequenceNumber();
                                oMoofFound = true;

                                AtomUtils::seekFromStart(_movieFragmentFilePtr, moof_offset);
                                uint32 atomType = UNKNOWN_ATOM;
                                uint32 atomSize = 0;
                                AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                                if (atomType == MOVIE_FRAGMENT_ATOM)
                                {
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                    _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                }
                                break;
                            }
                        }
                        //
                    }
                    else
                    {
                        // Not a valid tfra entries, cannot reposition.
                        return 0;
                    }
                }
                if (_parsing_mode == 1 && !oMoofFound)
                {

                    if (!oMoofFound)
                    {
                        _ptrMoofEnds = moof_offset;
                        uint32 fileSize = 0;
                        AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                        AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                        uint32 filePointer = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                        int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;

                        while (count > 0)
                        {
                            uint32 atomType = UNKNOWN_ATOM;
                            uint32 atomSize = 0;
                            AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                            if (atomType == MOVIE_FRAGMENT_ATOM)
                            {
                                uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                moofStartOffset -= DEFAULT_ATOM_SIZE;
                                parseMoofCompletely = true;

                                if (moofParsingCompleted)
                                {
                                    // do nothing
                                }
                                else
                                {
                                    uint32 i = _pMovieFragmentAtomVec->size();
                                    _pMoofOffsetVec->pop_back();
                                    _pMovieFragmentAtomVec->pop_back();
                                    PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                                    parseMoofCompletely = true;
                                    moofParsingCompleted = true;
                                    moofSize = 0;
                                    moofType = UNKNOWN_ATOM;
                                    moofCount = 0;
                                    moofPtrPos = 0;
                                }

                                PV_MP4_FF_NEW(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, (_movieFragmentFilePtr, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), _pMovieFragmentAtom);

                                if (!_pMovieFragmentAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                    break;
                                }

                                _pMovieFragmentAtom->setParent(this);
                                count -= _pMovieFragmentAtom->getSize();
                                uint32 i = _pMovieFragmentAtomVec->size();

                                MovieFragmentAtom *pMovieFragmentAtom = NULL;
                                uint32 prevMoofSeqNum = 0;

                                if (i > 0)
                                {
                                    pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[i-1];

                                    if (pMovieFragmentAtom != NULL)
                                        prevMoofSeqNum = (*_pMovieFragmentAtomVec)[i-1]->getSequenceNumber();
                                }
                                currMoofNum = _pMovieFragmentAtom->getSequenceNumber();

                                for (uint32 idx = prevMoofSeqNum; idx < currMoofNum - 1;idx++)
                                {
                                    _pMovieFragmentAtomVec->push_back(NULL);
                                    _pMoofOffsetVec->push_back(0);
                                }

                                if (currMoofNum > i)
                                {
                                    _pMoofOffsetVec->push_back(moofStartOffset);
                                    _pMovieFragmentAtomVec->push_back(_pMovieFragmentAtom);
                                }
                                else if ((*_pMovieFragmentAtomVec)[currMoofNum-1] == NULL)
                                {
                                    (*_pMovieFragmentAtomVec)[currMoofNum-1] = _pMovieFragmentAtom;
                                    (*_pMoofOffsetVec)[currMoofNum-1] = moofStartOffset;
                                }
                                else
                                {
                                    PV_MP4_FF_DELETE(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, _pMovieFragmentAtom);
                                    _pMovieFragmentAtom = NULL;
                                    break;

                                }
                                if (oMfraFound)
                                {
                                    currMoofNum = _pMovieFragmentAtom->getSequenceNumber();
                                    _movieFragmentIdx[trackID] = currMoofNum - 1 ;
                                    _peekMovieFragmentIdx[trackID] = currMoofNum - 1;
                                    _movieFragmentSeqIdx[trackID] = currMoofNum;
                                    _peekMovieFragmentSeqIdx[trackID] = _movieFragmentSeqIdx[trackID];
                                    oMoofFound = true;
                                    if (!_oVideoTrackPresent)
                                    {
                                        _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                    }
                                    break;
                                }
                                uint32 currTrafDuration = _pMovieFragmentAtom->getCurrentTrafDuration(trackID);
                                if (currTrafDuration >= modifiedTimeStamp)
                                {
                                    currMoofNum = _pMovieFragmentAtom->getSequenceNumber();
                                    _movieFragmentIdx[trackID] = currMoofNum - 1;
                                    _peekMovieFragmentIdx[trackID] = currMoofNum - 1;
                                    _movieFragmentSeqIdx[trackID] = currMoofNum;
                                    _peekMovieFragmentSeqIdx[trackID] = _movieFragmentSeqIdx[trackID];
                                    oMoofFound = true;
                                    if (!_oVideoTrackPresent)
                                    {
                                        _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                    }
                                    break;
                                }

                            }
                            else if (atomType == MEDIA_DATA_ATOM)
                            {
                                if (atomSize == 1)
                                {
                                    uint64 largeSize = 0;
                                    AtomUtils::read64(_movieFragmentFilePtr, largeSize);
                                    uint32 size =
                                        Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                                    count -= size;
                                    size -= 8; //for large size
                                    size -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, size);
                                }
                                else
                                {
                                    if (atomSize < DEFAULT_ATOM_SIZE)
                                    {
                                        _success = false;
                                        _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                                        break;
                                    }
                                    if (count < (int32)atomSize)
                                    {
                                        _success = false;
                                        _mp4ErrorCode = READ_FAILED;
                                        break;
                                    }
                                    count -= atomSize;
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                }
                            }

                            else
                            {
                                if (count > 0)
                                {
                                    count -= atomSize;
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                }

                            }
                        }
                    }

                }

                if (_pmovieAtom != NULL)
                    _pmovieAtom->resetTrackToEOT();

                if (_pMovieFragmentAtom != NULL)
                    returnedTS = _pMovieFragmentAtom->resetPlayback(trackID, convertedTS, traf_number, trun_number, sample_num);
            }
            else
                return 0;

            // convert returnedTS (which is in media time scale) to the ms
            MediaClockConverter mcc(getTrackMediaTimescale(trackID));
            mcc.update_clock(returnedTS);
            timestamp = mcc.get_converted_ts(1000);


            if (timestamp <= modifiedTimeStamp)
            {
                modifiedTimeStamp = timestamp;
            }
            retVal = modifiedTimeStamp;

        }
        else
        {
            if (_isMovieFragmentsPresent)
            {
                if (_pMovieFragmentAtomVec->size() > 0)
                {
                    if (moofParsingCompleted)
                    {
                        // do nothing
                    }
                    else
                    {
                        uint32 i = _pMovieFragmentAtomVec->size();
                        _pMoofOffsetVec->pop_back();
                        _pMovieFragmentAtomVec->pop_back();
                        PV_MP4_FF_DELETE(NULL, MovieFragmentAtom , (*_pMovieFragmentAtomVec)[i-1]);
                        parseMoofCompletely = true;
                        moofParsingCompleted = true;
                        moofSize = 0;
                        moofType = UNKNOWN_ATOM;
                        moofCount = 0;
                        moofPtrPos = 0;
                    }
                }
            }

            //movie
            if (_pmovieAtom != NULL)
            {
                resetAllMovieFragments();
                uint32 trackAudio = trackID;
                uint32 numTrackforAudio = 1;
                retVal = _pmovieAtom->resetPlayback(modifiedTimeStamp, numTrackforAudio, &trackAudio
                                                    , bResetToIFrame);
            }
        }

    }
    return retVal;

}



int32 Mpeg4File::queryRepositionTime(uint32 time,
                                     uint16 numTracks,
                                     uint32 *trackList,
                                     bool bResetToIFrame,
                                     bool bBeforeRequestedTime)
{

    uint32 i = 0;
    uint32 ret = 0;
    uint32 modifiedTimeStamp = time;
    uint32 trackID = 0;
    uint32 trackIds[256];

    bool oVideoTrackFound = false;
    int j = 1;
    for (i = 0; i < numTracks; i++)
    {
        trackID = trackList[i];
        if (getTrackMediaType(trackID) == MEDIA_TYPE_VISUAL)
        {
            trackIds[0] = trackList[i];
            oVideoTrackFound = true;
        }
        else
        {
            trackIds[j++] = trackList[i];
        }
    }

    uint32 convertedTS = 0;
    uint32 timestamp = 0, returnedTS = 0;

    for (i = 0; i < numTracks; i++)
    {
        trackID = trackIds[i];

        if (!oVideoTrackFound)
            trackID = trackList[i];

        if (getTrackMediaType(trackID) == MEDIA_TYPE_VISUAL)
        {
            if (repositionFromMoof(time, trackID))
            {
                //moof
                modifiedTimeStamp = time;

                // convert modifiedTimeStamp (which is in ms) to the appropriate
                // media time scale
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(trackID));
                if (oMfraFound)
                {
                    oMfraFound = true;
                    for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                    {

                        MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                        returnedTS = pMovieFragmentRandomAccessAtom->queryRepositionTime(trackID, convertedTS, bResetToIFrame,
                                     bBeforeRequestedTime);
                        if (returnedTS != 0)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    oMfraFound = false;
                    if (_parsing_mode == 1)
                        return -1;
                }

                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(getTrackMediaTimescale(trackID));
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);

                modifiedTimeStamp = timestamp;

                ret = modifiedTimeStamp;

            }
            else
            {
                //movie
                if (_pmovieAtom != NULL)
                {
                    modifiedTimeStamp =  _pmovieAtom->queryRepositionTime(time,
                                         numTracks,
                                         trackList,
                                         bResetToIFrame,
                                         bBeforeRequestedTime);
                    ret = modifiedTimeStamp;
                }
            }
        }

        if ((getTrackMediaType(trackID) == MEDIA_TYPE_AUDIO) ||
                (getTrackMediaType(trackID) == MEDIA_TYPE_TEXT))
        {
            if (repositionFromMoof(time, trackID))
            {
                //moof
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(trackID));

                for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
                {

                    MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
                    returnedTS = pMovieFragmentRandomAccessAtom->queryRepositionTime(trackID, convertedTS, bResetToIFrame,
                                 bBeforeRequestedTime);
                    if (returnedTS != 0)
                    {
                        break;
                    }
                }
                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(getTrackMediaTimescale(trackID));
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);

                if (!oVideoTrackFound)
                {
                    if (getTrackMediaType(trackID) == MEDIA_TYPE_AUDIO)
                    {
                        modifiedTimeStamp = timestamp;
                    }
                    else if (getTrackMediaType(trackID) == MEDIA_TYPE_TEXT && numTracks == 1)
                    {
                        modifiedTimeStamp = timestamp;
                    }
                }

                return modifiedTimeStamp;

            }
            else
            {
                //movie
                if (_pmovieAtom != NULL)
                {
                    modifiedTimeStamp =   _pmovieAtom->queryRepositionTime(modifiedTimeStamp,
                                          numTracks,
                                          trackList,
                                          bResetToIFrame,
                                          bBeforeRequestedTime);

                    if (!oVideoTrackFound)
                    {
                        if (getTrackMediaType(trackID) == MEDIA_TYPE_AUDIO)
                        {
                            ret = modifiedTimeStamp;
                        }
                        else if (getTrackMediaType(trackID) == MEDIA_TYPE_TEXT && numTracks == 1)
                        {
                            ret = modifiedTimeStamp;
                        }
                    }
                }

            }

        }
    }
    return ret;
}



int32 Mpeg4File::parseMFRA()
{
    uint32 ret = 0;
    uint32 fileSize = 0;
    uint32 MfraStartOffset = 0;
    AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
    AtomUtils::seekFromStart(_movieFragmentFilePtr, fileSize);
    AtomUtils::rewindFilePointerByN(_movieFragmentFilePtr, 16);

    uint32 atomType = UNKNOWN_ATOM;
    uint32 atomSize = 0;
    AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);

    if (atomType == MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_ATOM)
    {
        if (_pMfraOffsetAtom == NULL)
        {
            PV_MP4_FF_NEW(fp->auditCB, MfraOffsetAtom, (_movieFragmentFilePtr, atomSize, atomType), _pMfraOffsetAtom);
            if (!_pMfraOffsetAtom->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = READ_MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_FAILED;
                return _mp4ErrorCode;
            }
            MfraStartOffset = _pMfraOffsetAtom->getSizeStoredInmfro();

        }
    }
    AtomUtils::rewindFilePointerByN(_movieFragmentFilePtr, MfraStartOffset);
    AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
    if (atomType == MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM)
    {
        if (_pMovieFragmentRandomAccessAtomVec->size() == 0)
        {
            MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = NULL;
            PV_MP4_FF_NEW(fp->auditCB, MovieFragmentRandomAccessAtom, (_movieFragmentFilePtr, atomSize, atomType), pMovieFragmentRandomAccessAtom);

            if (!pMovieFragmentRandomAccessAtom->MP4Success())
            {
                PV_MP4_FF_DELETE(NULL, MovieFragmentRandomAccessAtom, pMovieFragmentRandomAccessAtom);
                _success = false;
                _mp4ErrorCode = pMovieFragmentRandomAccessAtom->GetMP4Error();
                return _mp4ErrorCode ;
            }
            pMovieFragmentRandomAccessAtom->setParent(this);
            _pMovieFragmentRandomAccessAtomVec->push_back(pMovieFragmentRandomAccessAtom);
            oMfraFound = true;
        }
    }

    return ret;

}


int32 Mpeg4File::peekNextBundledAccessUnits(const uint32 trackID,
        uint32 *n,
        MediaMetaInfo *mInfo)
{
    // IF THERE ARE NO MEDIA TRACKS, RETURN READ ERROR
    uint32 samplesTobeRead;
    samplesTobeRead = *n;
    uint32 totalSampleRead = 0;
    if (getNumTracks() == 0)
    {
        return -1;
    }
    if (_pmovieAtom != NULL)
    {
        uint32 ret = (_pmovieAtom->peekNextBundledAccessUnits(trackID, n, mInfo));
        if (ret == END_OF_TRACK)
        {
            if (!_isMovieFragmentsPresent)
                return ret;

            bool oAllMoofExhausted = false;

            totalSampleRead += *n;

            if (totalSampleRead == samplesTobeRead)
            {
                *n = totalSampleRead;
                return EVERYTHING_FINE;
            }

            if (_pMovieFragmentAtomVec != NULL)
            {
                if (samplesTobeRead >= *n)
                    *n = samplesTobeRead - *n;
                if (*n == 0)
                    *n = samplesTobeRead;
            }
            else
                return ret;

            if (_parsing_mode == 0)
            {
                int32 return1 = 0;
                while (_peekMovieFragmentIdx[trackID] < _pMovieFragmentAtomVec->size())
                {
                    uint32 peekMovieFragmentIdx = _peekMovieFragmentIdx[trackID];
                    MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[peekMovieFragmentIdx];
                    if (pMovieFragmentAtom != NULL)
                    {
                        if ((uint32)pMovieFragmentAtom->getSequenceNumber() == _peekMovieFragmentSeqIdx[trackID])
                        {
                            TrackFragmentAtom *trackfragment = pMovieFragmentAtom->getTrackFragmentforID(trackID);
                            if (trackfragment != NULL)
                            {
                                if (trackfragment->getTrackId() == trackID)
                                {
                                    return1 = pMovieFragmentAtom->peekNextBundledAccessUnits(trackID, n, totalSampleRead, mInfo);
                                    totalSampleRead += *n;
                                    if (return1 != END_OF_TRACK)
                                    {
                                        *n = totalSampleRead;
                                        return return1;
                                    }
                                    else
                                    {
                                        _peekMovieFragmentSeqIdx[trackID]++;
                                        if (samplesTobeRead >= *n)
                                        {
                                            samplesTobeRead = samplesTobeRead - *n;
                                            *n = samplesTobeRead;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    _peekMovieFragmentIdx[trackID]++;
                }
                if (return1 == END_OF_TRACK)
                {
                    *n = totalSampleRead;
                    _peekMovieFragmentIdx[trackID] = 0;
                    _peekMovieFragmentSeqIdx[trackID] = 1;
                    return return1;
                }
            }
            else
            {

                while (!oAllMoofExhausted)
                {
                    uint32 moofIndex = 0;
                    bool moofToBeParsed = false;

                    if (_pMovieFragmentAtomVec->size() > _peekMovieFragmentIdx[trackID])
                    {
                        MovieFragmentAtom *pMovieFragmentAtom = NULL;
                        pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[_peekMovieFragmentIdx[trackID]];
                        if (pMovieFragmentAtom == NULL)
                        {
                            moofToBeParsed = true;
                            moofIndex = _peekMovieFragmentIdx[trackID];
                        }
                    }
                    if ((_pMovieFragmentAtomVec->size() <= _peekMovieFragmentIdx[trackID]) || moofToBeParsed)
                    {
                        uint32 fileSize = 0;
                        AtomUtils::getCurrentFileSize(_movieFragmentFilePtr, fileSize);
                        AtomUtils::seekFromStart(_movieFragmentFilePtr, _ptrMoofEnds);
                        uint32 filePointer = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                        int32 count = fileSize - filePointer;// -DEFAULT_ATOM_SIZE;

                        while (count > 0)
                        {
                            uint32 atomType = UNKNOWN_ATOM;
                            uint32 atomSize = 0;
                            uint32 currPos = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                            AtomUtils::getNextAtomType(_movieFragmentFilePtr, atomSize, atomType);
                            if ((currPos + atomSize) > fileSize)
                            {
                                AtomUtils::seekFromStart(_movieFragmentFilePtr, currPos);
                                return 	INSUFFICIENT_DATA;
                            }
                            if (atomType == MOVIE_FRAGMENT_ATOM)
                            {
                                uint32 moofStartOffset = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);
                                moofStartOffset -= DEFAULT_ATOM_SIZE;

                                parseMoofCompletely = true;

                                PV_MP4_FF_NEW(_movieFragmentFilePtr->auditCB, MovieFragmentAtom, (_movieFragmentFilePtr, atomSize, atomType, _pTrackDurationContainer, _pTrackExtendsAtomVec, parseMoofCompletely, moofParsingCompleted, countOfTrunsParsed), _pMovieFragmentAtom);

                                if (!_pMovieFragmentAtom->MP4Success())
                                {
                                    _success = false;
                                    _mp4ErrorCode = _pMovieFragmentAtom->GetMP4Error();
                                    oAllMoofExhausted = true;
                                    break;
                                }
                                _pMovieFragmentAtom->setParent(this);
                                count -= _pMovieFragmentAtom->getSize();
                                if (moofToBeParsed)
                                {
                                    (*_pMovieFragmentAtomVec)[moofIndex] = _pMovieFragmentAtom;
                                    (*_pMoofOffsetVec)[moofIndex] = moofStartOffset;
                                }
                                else
                                {
                                    _pMoofOffsetVec->push_back(moofStartOffset);
                                    _pMovieFragmentAtomVec->push_back(_pMovieFragmentAtom);
                                }
                                _ptrMoofEnds = AtomUtils::getCurrentFilePosition(_movieFragmentFilePtr);

                                break;
                            }
                            else if (atomType == MEDIA_DATA_ATOM)
                            {
                                if (atomSize == 1)
                                {
                                    uint64 largeSize = 0;
                                    AtomUtils::read64(_movieFragmentFilePtr, largeSize);
                                    uint32 size =
                                        Oscl_Int64_Utils::get_uint64_lower32(largeSize);
                                    count -= size;
                                    size -= 8; //for large size
                                    size -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, size);
                                }
                                else
                                {
                                    if (atomSize < DEFAULT_ATOM_SIZE)
                                    {
                                        _success = false;
                                        oAllMoofExhausted = true;
                                        _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                                        break;
                                    }
                                    if (count < (int32)atomSize)
                                    {
                                        _success = false;
                                        oAllMoofExhausted = true;
                                        _mp4ErrorCode = INSUFFICIENT_DATA;
                                        AtomUtils::seekFromStart(_movieFragmentFilePtr, currPos);
                                        ret = _mp4ErrorCode;
                                        break;
                                    }
                                    count -= atomSize;
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                }
                            }

                            else
                            {
                                if (count > 0)
                                {
                                    count -= atomSize;
                                    atomSize -= DEFAULT_ATOM_SIZE;
                                    AtomUtils::seekFromCurrPos(_movieFragmentFilePtr, atomSize);
                                }
                                break;
                            }
                        }
                        if (count <= 0)
                        {
                            oAllMoofExhausted = true;
                            if (_pMovieFragmentAtomVec->size() < _peekMovieFragmentIdx[trackID])
                                break;
                        }
                    }

                    int32 return1 = 0;
                    MovieFragmentAtom *pMovieFragmentAtom = NULL;
                    uint32 movieFragmentIdx = _peekMovieFragmentIdx[trackID];

                    if (movieFragmentIdx < _pMovieFragmentAtomVec->size())
                        pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[movieFragmentIdx];

                    if (pMovieFragmentAtom != NULL)
                    {
                        uint32 seqNum = pMovieFragmentAtom->getSequenceNumber();
                        if (seqNum == _peekMovieFragmentSeqIdx[trackID])
                        {
                            TrackFragmentAtom *trackfragment = pMovieFragmentAtom->getTrackFragmentforID(trackID);
                            if (trackfragment != NULL)
                            {
                                if (trackfragment->getTrackId() == trackID)
                                {
                                    return1 = pMovieFragmentAtom->peekNextBundledAccessUnits(trackID, n, totalSampleRead, mInfo);
                                    totalSampleRead += *n;
                                    if (return1 != END_OF_TRACK)
                                    {
                                        *n = totalSampleRead;
                                        return return1;
                                    }
                                    else
                                    {
                                        _peekMovieFragmentSeqIdx[trackID]++;
                                        if (samplesTobeRead >= *n)
                                        {
                                            samplesTobeRead = samplesTobeRead - *n;
                                            *n = samplesTobeRead;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                _peekMovieFragmentIdx[trackID]++;
                                _peekMovieFragmentSeqIdx[trackID]++;
                                *n = 0;
                                return NO_SAMPLE_IN_CURRENT_MOOF;
                            }
                        }
                    }
                    _peekMovieFragmentIdx[trackID]++;

                }
            }
        }
        return ret;
    }
    else
    {
        return -1;
    }
}


uint32 Mpeg4File::getSampleCountInTrack(uint32 id)
{
    uint32 nTotalSamples = 0;
    if (_pmovieAtom != NULL)
    {
        nTotalSamples = (_pmovieAtom->getSampleCountInTrack(id));
        if (!_isMovieFragmentsPresent)
            return nTotalSamples;

        if (_parsing_mode == 0)
        {
            if (_pMovieFragmentAtomVec->size() > 0)
            {
                for (uint32 idx = 0; idx < _pMovieFragmentAtomVec->size(); idx++)
                {
                    MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                    if (pMovieFragmentAtom != NULL)
                    {
                        nTotalSamples += pMovieFragmentAtom->getTotalSampleInTraf(id);
                    }
                }
                return nTotalSamples;
            }
        }
        return nTotalSamples;
    }
    return 0;
}


bool Mpeg4File::IsTFRAPresentForTrack(uint32 TrackId, bool oVideoAudioTextTrack)
{
    if (_pMovieFragmentRandomAccessAtomVec != NULL)
    {
        for (uint32 idx = 0; idx < _pMovieFragmentRandomAccessAtomVec->size();idx++)
        {

            MovieFragmentRandomAccessAtom *pMovieFragmentRandomAccessAtom = (*_pMovieFragmentRandomAccessAtomVec)[idx];
            return pMovieFragmentRandomAccessAtom->IsTFRAPresentForTrack(TrackId, oVideoAudioTextTrack);
        }
    }
    return false;
}


/*
This function has been modified to check the entry count in TFRA for all tracks are equal.
The code change is under macro DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT
*/
bool Mpeg4File::IsTFRAPresentForAllTrack(uint32 numTracks, uint32 *trackList)
{

#if (DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT)
    bool oVideoAudioTextTrack  = false;
// This flag will be true for Video in AVT,VT & AV and for Audio in AT.
// Based on this flag IsTFRAPresentForTrack() functions behaviour is changing
// We are comparing the entry count for all the tracks to entry count of V in case of Vonly,AVT,VT & AV clips
// and in case of Aonly & AT clips, entry count for all the tracks is compared with entry count of audio. For Tonly
// clips, entry count for text track is compared with its own entry count.

    // Support for clips having Video track.
    for (int32 i = 0; i < numTracks; i++)
    {
        uint32 trackID = trackList[i];
        if (getTrackMediaType(trackID) == MEDIA_TYPE_VISUAL)
        {
            oVideoAudioTextTrack  = true;
            if (IsTFRAPresentForTrack(trackID, oVideoAudioTextTrack) == false)
            {
                return false;
            }
            break;
        }
    }
    // Support for clips having Audio track and no Video track.
    if (!oVideoAudioTextTrack)
    {
        for (int32 i = 0; i < numTracks; i++)
        {
            uint32 trackID = trackList[i];
            if (getTrackMediaType(trackID) == MEDIA_TYPE_AUDIO)
            {
                oVideoAudioTextTrack = true;
                if (IsTFRAPresentForTrack(trackID, oVideoAudioTextTrack) == false)
                {
                    return false;
                }
                break;
            }
        }
    }
    // Support for clips having only Text track.
    if (!oVideoAudioTextTrack && numTracks == 1)
    {
        for (uint32 i = 0; i < numTracks; i++)
        {
            uint32 trackID = trackList[i];
            if (getTrackMediaType(trackID) == MEDIA_TYPE_TEXT)
            {
                oVideoAudioTextTrack = true;
                if (IsTFRAPresentForTrack(trackID, oVideoAudioTextTrack) == false)
                {
                    return false;
                }
                break;
            }
        }
    }
#endif // DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT
    for (uint32 idx = 0; idx < numTracks; idx++)
    {
        uint32 trackID = trackList[idx];
        // second argument is false always
        if (IsTFRAPresentForTrack(trackID, false) == false)
        {
            return false;
        }
    }
    return true;
}

void Mpeg4File::resetPlayback()
{
    if (_pmovieAtom == NULL)
        return;

    _pmovieAtom->resetPlayback();

    if (_isMovieFragmentsPresent)
    {
        if (_pMovieFragmentAtomVec != NULL)
        {
            int numTracks = _pmovieAtom->getNumTracks();
            uint32 *trackList  = (uint32 *) oscl_malloc(sizeof(uint32) * numTracks);
            if (!trackList)
                return;       // malloc failed

            _pmovieAtom->getTrackWholeIDList(trackList);
            for (int i = 0; i < numTracks; i++)
            {
                uint32 trackID = trackList[i];
                _peekMovieFragmentIdx[trackID] = 0;
                _movieFragmentIdx[trackID] = 0;
                _movieFragmentSeqIdx[trackID] = 1;
                _peekMovieFragmentSeqIdx[trackID] = 1;
            }
            oscl_free(trackList);
            for (uint32 idx = 0; idx < _pMovieFragmentAtomVec->size(); idx++)
            {
                MovieFragmentAtom *pMovieFragmentAtom = (*_pMovieFragmentAtomVec)[idx];
                if (pMovieFragmentAtom != NULL)
                    pMovieFragmentAtom->resetPlayback();

            }
        }
    }
}

uint32 Mpeg4File::repositionFromMoof(uint32 time, uint32 trackID)
{
    uint32 modifiedTimeStamp = time;
    uint32 convertedTS = 0;
    uint32 trackDuration = Oscl_Int64_Utils::get_uint64_lower32(getTrackMediaDurationForMovie(trackID));//getMovieDuration() - getMovieFragmentDuration();

    MediaClockConverter mcc1(1000);
    mcc1.update_clock(modifiedTimeStamp);
    convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(trackID));

    if (_isMovieFragmentsPresent)
    {
        if (IsTFRAPresentForTrack(trackID, false) == false)
        {
            return 0;
        }
        if (modifiedTimeStamp >= trackDuration)
        {
            return 1; //repos in moof
        }
    }
    return 0; //repos in moov
}

MP4_ERROR_CODE Mpeg4File::CancelNotificationSync()
{
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Mpeg4File::CancelNotificationSync"));

    bool retVal = _commonFilePtr->_pvfile.CancelNotificationSync();

    if (retVal)
    {
        return EVERYTHING_FINE;
    }
    else
    {
        return DEFAULT_ERROR;
    }
}

bool Mpeg4File::CreateDataStreamSessionForExternalDownload(OSCL_wString& aFilename,
        PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
        OsclFileHandle* aHandle,
        Oscl_FileServer* aFileServSession)
{
    OsclAny*ptr = oscl_malloc(sizeof(MP4_FF_FILE));
    if (ptr == NULL)
    {
        _success = false;
        _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
        return false;
    }
    _commonFilePtr = OSCL_PLACEMENT_NEW(ptr, MP4_FF_FILE());

    if (_commonFilePtr != NULL)
    {
        _commonFilePtr->_fileServSession = aFileServSession;
        _commonFilePtr->_pvfile.SetCPM(aCPMAccessFactory);
        _commonFilePtr->_pvfile.SetFileHandle(aHandle);

        if (AtomUtils::OpenMP4File(aFilename,
                                   Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                                   _commonFilePtr) != 0)
        {
            return false;
        }

        uint32 fileSize;
        AtomUtils::getCurrentFileSize(_commonFilePtr, fileSize);
        _commonFilePtr->_fileSize = (int32)fileSize;
    }
    return true;
}

void Mpeg4File::DestroyDataStreamForExternalDownload()
{
    if (_commonFilePtr != NULL)
    {
        if (_commonFilePtr->IsOpen())
        {
            AtomUtils::CloseMP4File(_commonFilePtr);
        }
        oscl_free(_commonFilePtr);
        _commonFilePtr = NULL;
    }
}

//Below APIs are used to supress Warning
void Mpeg4File::ReserveMemoryForLangCodeVector(Oscl_Vector<uint16, OsclMemAllocator> &iLangCode, int32 capacity, int32 &leavecode)
{
    leavecode = 0;
    OSCL_TRY(leavecode, iLangCode.reserve(capacity));

}

void Mpeg4File::ReserveMemoryForValuesVector(Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> &iValues, int32 capacity, int32 &leavecode)
{
    leavecode = 0;
    OSCL_TRY(leavecode, iValues.reserve(capacity));

}


