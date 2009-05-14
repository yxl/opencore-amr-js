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
/*                            MPEG-4 IMpeg4File Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    The IMpeg4File Class is INTERFACE that exsposes only those necessary
    methods of the underlying Mpeg4File class.
*/

#define IMPLEMENT_IMpeg4File_H__

#define GENERIC_ATOM_SIZE 8

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#include "atom.h"
#include "atomutils.h"
#include "atomdefs.h"

#include "pvuserdataatom.h"
#include "oscl_utf8conv.h"
#include "isucceedfail.h"

#include "impeg4file.h"
#include "mpeg4file.h"

#include "oscl_string.h"


// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


/* ======================================================================== */
OSCL_EXPORT_REF  IMpeg4File *IMpeg4File::readMP4File(OSCL_wString& aFilename,
        PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
        OsclFileHandle* aHandle,
        uint32 aParsingMode,
        Oscl_FileServer* aFileServSession)
{
    //optimized mode is not supported if multiple file ptrs are not allowed
    if (aParsingMode == 1)
    {
#ifndef OPEN_FILE_ONCE_PER_TRACK
        aParsingMode = 0;
#endif
    }

    MP4_FF_FILE fileStruct;
    MP4_FF_FILE *fp = &fileStruct;

    fp->_fileServSession = aFileServSession;
    fp->_pvfile.SetCPM(aCPMAccessFactory);
    fp->_pvfile.SetFileHandle(aHandle);

    if (AtomUtils::OpenMP4File(aFilename,
                               Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                               fp) != 0)
    {
        return NULL;
    }

    uint32 fileSize;
    AtomUtils::getCurrentFileSize(fp, fileSize);
    fp->_fileSize = (int32)fileSize;

    Mpeg4File *mp4 = NULL;
    PV_MP4_FF_NEW(fp->auditCB, Mpeg4File, (fp, aFilename, aParsingMode), mp4);

#ifdef OPEN_FILE_ONCE_PER_TRACK
    if (mp4 != NULL)
    {
        if (!mp4->IsMovieFragmentsPresent())
        {
            if (fp->IsOpen())
                AtomUtils::CloseMP4File(fp);
        }
    }
#endif

    return mp4;
}

//IsXXXable	*******************************************
/*
oMoovBeforeMdat: as input,
				1: IsMobileMP4 check
				2: IsPseudoStreamable() check
				3: IsPlayable() check
				 as output,
				 1:	moov atom is in front of the mdat atom
				 0: there is mdat atom in front of moov atom

metaDataSize   : as input. It is the available data size
				 as output. It is the offset of the file where the full "moov" atom is available.
*/

OSCL_EXPORT_REF
int32
IMpeg4File::IsXXXable(OSCL_wString& filename,
                      int32 &metaDataSize,
                      int32  &oMoovBeforeMdat,
                      uint32 *pMajorBrand,
                      uint32 *pCompatibleBrands,
                      Oscl_FileServer* fileServSession)
{
    const int ISMOBILEMP4 = 1, ISPSEUDOSTREMABLE = 2, ISPLAYABLE = 3;

    if (metaDataSize <= DEFAULT_ATOM_SIZE)
        return READ_FAILED;

    if ((oMoovBeforeMdat < ISMOBILEMP4) || (oMoovBeforeMdat > ISPLAYABLE))
        return DEFAULT_ERROR;

    int32 checkType = oMoovBeforeMdat;
    int32 fileSize = metaDataSize;

    MP4_FF_FILE fileStruct;
    MP4_FF_FILE *fp = &fileStruct;

    fp->_fileServSession = fileServSession;

    if (AtomUtils::OpenMP4File(filename,
                               Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                               fp) != 0)
    {
        return (FILE_OPEN_FAILED);
    }

    fp->_fileSize = fileSize;

    int32	mp4ErrorCode = DEFAULT_ERROR;

    uint32 atomType = UNKNOWN_ATOM;
    uint32 atomSize = 0;

    AtomUtils::getNextAtomType(fp, atomSize, atomType);

    if (atomType == FILE_TYPE_ATOM)
    {
        //"ftyp"
        if (fileSize < (int32)atomSize)
        {
            AtomUtils::CloseMP4File(fp);
            return READ_FAILED;
        }
        FileTypeAtom *_pFileTypeAtom = NULL;
        PV_MP4_FF_NEW(fp->auditCB, FileTypeAtom, (fp, atomSize, atomType), _pFileTypeAtom);

        if (!_pFileTypeAtom->MP4Success())
        {
            PV_MP4_FF_DELETE(NULL, FileTypeAtom, _pFileTypeAtom);
            AtomUtils::CloseMP4File(fp);
            return READ_FILE_TYPE_ATOM_FAILED;
        }
        //get file type
        {
            *pMajorBrand = ENoFileType;

            uint32 majorBrand = _pFileTypeAtom->getMajorBrand();

            //conversion
            if (majorBrand == MOBILE_MP4)	*pMajorBrand = EMMP4;
            else if (majorBrand == BRAND_3GPP4)	*pMajorBrand = E3GP4;
            else if (majorBrand == BRAND_3GPP5)	*pMajorBrand = E3GP5;
            else if (majorBrand == BRAND_ISOM)	*pMajorBrand = EISOM;
            else if (majorBrand == BRAND_MP41)	*pMajorBrand = EMP41;
            else if (majorBrand == BRAND_MP42)	*pMajorBrand = EMP42;
            else if (majorBrand == WMF_BRAND)	*pMajorBrand = EWMF;
            // else .. set to ENoFileType above

            Oscl_Vector<uint32, OsclMemAllocator> *compatibleBrandArray =
                _pFileTypeAtom->getCompatibleBrand();

            *pCompatibleBrands = ENoFileType;

            if (compatibleBrandArray != NULL)
            {
                for (uint32 i = 0; i < compatibleBrandArray->size(); i++)
                {
                    uint32 compatibleBrand = (*compatibleBrandArray)[i];

                    if (compatibleBrand == MOBILE_MP4)			*pCompatibleBrands |= EMMP4;
                    else if (compatibleBrand == BRAND_3GPP4)	*pCompatibleBrands |= E3GP4;
                    else if (compatibleBrand == BRAND_3GPP5)	*pCompatibleBrands |= E3GP5;
                    else if (compatibleBrand == BRAND_ISOM)		*pCompatibleBrands |= EISOM;
                    else if (compatibleBrand == BRAND_MP41)		*pCompatibleBrands |= EMP41;
                    else if (compatibleBrand == BRAND_MP42)		*pCompatibleBrands |= EMP42;
                    else if (compatibleBrand == WMF_BRAND)		*pCompatibleBrands |= EWMF;
                }
            }
        }//end of get file type

        int32 fpos = _pFileTypeAtom->getSize();
        PV_MP4_FF_DELETE(NULL, FileTypeAtom, _pFileTypeAtom);

        if (checkType == ISMOBILEMP4)
        {
            AtomUtils::CloseMP4File(fp);
            return EVERYTHING_FINE;
        }

        oMoovBeforeMdat = true;
        metaDataSize = 0;

        if ((fpos + DEFAULT_ATOM_SIZE) > fileSize)
        {
            AtomUtils::CloseMP4File(fp);
            return READ_FAILED;
        }

        while (fpos < fileSize)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            fpos += DEFAULT_ATOM_SIZE;

            if ((atomType == FREE_SPACE_ATOM)
                    || (atomType == SKIP_ATOM)
                    || (atomType == USER_DATA_ATOM)
                    || (atomType == UUID_ATOM)
                    || (atomType == MOVIE_ATOM)
                    || (atomType == MEDIA_DATA_ATOM)
                    || (atomType == UNKNOWN_ATOM))
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                atomSize -= DEFAULT_ATOM_SIZE;
                if (atomType == MOVIE_ATOM)
                {
                    //this is an exit
                    //no check of the "size" validity
                    metaDataSize = fpos + atomSize;
                    mp4ErrorCode = EVERYTHING_FINE;
                    break;
                }
                else if (atomType == MEDIA_DATA_ATOM)
                {
                    oMoovBeforeMdat = false;
                    if (checkType == ISPSEUDOSTREMABLE)
                    {
                        mp4ErrorCode = EVERYTHING_FINE;
                        break;
                    }
                }

                if ((int32)(fpos + atomSize) > fileSize)
                {
                    mp4ErrorCode = READ_FAILED;
                    break;
                }
                AtomUtils::seekFromCurrPos(fp, atomSize);
                fpos += atomSize;

            }
            else if (atomType == 0)
            {
                //return READ_FAILED;
                //since extra check is put above, it should NEVER fall in this bracket
                mp4ErrorCode = READ_FAILED;
                break;
            }
            else
            {	//error: other atoms should not be in file level
                mp4ErrorCode = DEFAULT_ERROR;
                break;
            }
        }
    }
    AtomUtils::CloseMP4File(fp);
    return (mp4ErrorCode);
}

OSCL_EXPORT_REF
int32
IMpeg4File::IsXXXable(MP4_FF_FILE_REFERENCE fileRef,
                      int32 &metaDataSize,
                      int32  &oMoovBeforeMdat,
                      uint32 *pMajorBrand,
                      uint32 *pCompatibleBrands)
{
    const int ISMOBILEMP4 = 1, ISPSEUDOSTREMABLE = 2, ISPLAYABLE = 3;

    if (metaDataSize <= DEFAULT_ATOM_SIZE)
        return READ_FAILED;

    if ((oMoovBeforeMdat < ISMOBILEMP4) || (oMoovBeforeMdat > ISPLAYABLE))
        return DEFAULT_ERROR;

    int32 checkType = oMoovBeforeMdat;
    int32 fileSize = metaDataSize;

    MP4_FF_FILE fileStruct;
    MP4_FF_FILE *fp = &fileStruct;

    fp->_pvfile.SetFilePtr(fileRef);

    AtomUtils::seekFromStart(fp, 0);
    fp->_fileSize = fileSize;

    int32	mp4ErrorCode = DEFAULT_ERROR;

    uint32 atomType = UNKNOWN_ATOM;
    uint32 atomSize = 0;

    AtomUtils::getNextAtomType(fp, atomSize, atomType);

    if (atomType == FILE_TYPE_ATOM)
    {
        //"ftyp"
        if (fileSize < (int32)atomSize)
        {
            AtomUtils::CloseMP4File(fp);
            return READ_FAILED;
        }
        FileTypeAtom *_pFileTypeAtom = NULL;
        PV_MP4_FF_NEW(fp->auditCB, FileTypeAtom, (fp, atomSize, atomType), _pFileTypeAtom);

        if (!_pFileTypeAtom->MP4Success())
        {
            PV_MP4_FF_DELETE(NULL, FileTypeAtom, _pFileTypeAtom);
            AtomUtils::CloseMP4File(fp);
            return READ_FILE_TYPE_ATOM_FAILED;
        }
        //get file type
        {
            *pMajorBrand = ENoFileType;

            uint32 majorBrand = _pFileTypeAtom->getMajorBrand();

            //conversion
            if (majorBrand == MOBILE_MP4)	*pMajorBrand = EMMP4;
            else if (majorBrand == BRAND_3GPP4)	*pMajorBrand = E3GP4;
            else if (majorBrand == BRAND_3GPP5)	*pMajorBrand = E3GP5;
            else if (majorBrand == BRAND_ISOM)	*pMajorBrand = EISOM;
            else if (majorBrand == BRAND_MP41)	*pMajorBrand = EMP41;
            else if (majorBrand == BRAND_MP42)	*pMajorBrand = EMP42;
            else if (majorBrand == WMF_BRAND)	*pMajorBrand = EWMF;


            Oscl_Vector<uint32, OsclMemAllocator> *compatibleBrandArray =
                _pFileTypeAtom->getCompatibleBrand();


            *pCompatibleBrands = ENoFileType;

            if (compatibleBrandArray != NULL)
            {
                for (uint32 i = 0; i < compatibleBrandArray->size(); i++)
                {
                    uint32 compatibleBrand = (*compatibleBrandArray)[i];

                    if (compatibleBrand == MOBILE_MP4)			*pCompatibleBrands |= EMMP4;
                    else if (compatibleBrand == BRAND_3GPP4)	*pCompatibleBrands |= E3GP4;
                    else if (compatibleBrand == BRAND_3GPP5)	*pCompatibleBrands |= E3GP5;
                    else if (compatibleBrand == BRAND_ISOM)		*pCompatibleBrands |= EISOM;
                    else if (compatibleBrand == BRAND_MP41)		*pCompatibleBrands |= EMP41;
                    else if (compatibleBrand == BRAND_MP42)		*pCompatibleBrands |= EMP42;
                    else if (compatibleBrand == WMF_BRAND)		*pCompatibleBrands |= EWMF;
                }
            }
        }//end of get file type

        int32 fpos = _pFileTypeAtom->getSize();
        PV_MP4_FF_DELETE(NULL, FileTypeAtom, _pFileTypeAtom);

        if (checkType == ISMOBILEMP4)
        {
            AtomUtils::CloseMP4File(fp);
            return EVERYTHING_FINE;
        }

        oMoovBeforeMdat = true;
        metaDataSize = 0;

        if ((fpos + DEFAULT_ATOM_SIZE) > fileSize)
        {
            AtomUtils::CloseMP4File(fp);
            return READ_FAILED;
        }

        while (fpos < fileSize)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            fpos += DEFAULT_ATOM_SIZE;

            if ((atomType == FREE_SPACE_ATOM)
                    || (atomType == SKIP_ATOM)
                    || (atomType == USER_DATA_ATOM)
                    || (atomType == UUID_ATOM)
                    || (atomType == MOVIE_ATOM)
                    || (atomType == MEDIA_DATA_ATOM)
                    || (atomType == UNKNOWN_ATOM))
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                atomSize -= DEFAULT_ATOM_SIZE;
                if (atomType == MOVIE_ATOM)
                {
                    //this is an exit
                    //no check of the "size" validity
                    metaDataSize = fpos + atomSize;
                    mp4ErrorCode = EVERYTHING_FINE;
                    break;
                }
                else if (atomType == MEDIA_DATA_ATOM)
                {
                    oMoovBeforeMdat = false;
                    if (checkType == ISPSEUDOSTREMABLE)
                    {
                        mp4ErrorCode = EVERYTHING_FINE;
                        break;
                    }
                }

                if ((int32)(fpos + atomSize) > fileSize)
                {
                    mp4ErrorCode = READ_FAILED;
                    break;
                }
                AtomUtils::seekFromCurrPos(fp, atomSize);
                fpos += atomSize;

            }
            else if (atomType == 0)
            {
                //return READ_FAILED;
                //since extra check is put above, it should NEVER fall in this bracket
                mp4ErrorCode = READ_FAILED;
                break;
            }
            else
            {	//error: other atoms should not be in file level
                mp4ErrorCode = DEFAULT_ERROR;
                break;
            }
        }
    }
    AtomUtils::CloseMP4File(fp);
    return (mp4ErrorCode);
}


OSCL_EXPORT_REF MP4_ERROR_CODE
IMpeg4File::IsProgressiveDownloadable(MP4_FF_FILE_REFERENCE filePtr,
                                      uint32 fileSize,
                                      bool& oIsProgressiveDownloadable,
                                      uint32& metaDataSize)
{
    oIsProgressiveDownloadable = false;
    metaDataSize  = 0;

    MP4_FF_FILE fileStruct;
    MP4_FF_FILE *fp = &fileStruct;

    fp->_pvfile.SetFilePtr(filePtr);

    if (fileSize <= DEFAULT_ATOM_SIZE)
    {
        return INSUFFICIENT_DATA;
    }

    int32 filePointer = AtomUtils::getCurrentFilePosition(fp);
    AtomUtils::seekFromStart(fp, 0);

    fp->_fileSize = fileSize;

    uint32 atomType          = UNKNOWN_ATOM;
    uint32 atomSize          = 0;
    bool oMovieAtomFound     = false;
    bool oMediaDataAtomFound = false;

    int32 fpos = 0;

    MP4_ERROR_CODE mp4ErrorCode = INSUFFICIENT_DATA;

    while ((uint32)(fpos + DEFAULT_ATOM_SIZE) < fileSize)
    {
        AtomUtils::getNextAtomType(fp, atomSize, atomType);
        if (atomSize < DEFAULT_ATOM_SIZE)
        {
            mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
            break;
        }

        if ((atomType == FILE_TYPE_ATOM) ||
                (atomType == FREE_SPACE_ATOM) ||
                (atomType == SKIP_ATOM) ||
                (atomType == USER_DATA_ATOM) ||
                (atomType == UUID_ATOM) ||
                (atomType == UNKNOWN_ATOM))
        {
            fpos += atomSize;
            if ((uint32)fpos > fileSize)
            {
                break;
            }
            AtomUtils::seekFromStart(fp, fpos);
            continue;
        }
        else if (atomType == MOVIE_ATOM)
        {
            fpos += atomSize;
            oMovieAtomFound = true;
            metaDataSize = fpos;
        }
        else if (atomType == MEDIA_DATA_ATOM)
        {
            fpos += atomSize;
            oMediaDataAtomFound  = true;
        }
        else
        {
            //error: should never get here
            mp4ErrorCode = DEFAULT_ERROR;
        }
        break;
    }

    if (oMovieAtomFound || oMediaDataAtomFound)
    {
        //at most one of two can be true
        oIsProgressiveDownloadable = oMovieAtomFound;
        mp4ErrorCode = EVERYTHING_FINE;
    }

    AtomUtils::seekFromStart(fp, filePointer);

    return (mp4ErrorCode);
}

OSCL_EXPORT_REF MP4_ERROR_CODE
IMpeg4File::GetMetaDataSize(PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                            bool& oIsProgressiveDownloadable,
                            uint32& metaDataSize)
{
    oIsProgressiveDownloadable = false;
    metaDataSize  = 0;

    /* use a dummy string for file name */
    OSCL_wHeapString<OsclMemAllocator> filename;

    MP4_FF_FILE fileStruct;
    MP4_FF_FILE *fp = &fileStruct;
    fp->_pvfile.SetCPM(aCPMAccessFactory);

    if (AtomUtils::OpenMP4File(filename,
                               Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                               fp) != 0)
    {
        return FILE_OPEN_FAILED;
    }

    uint32 fileSize;
    AtomUtils::getCurrentFileSize(fp, fileSize);
    fp->_fileSize = (int32)fileSize;

    AtomUtils::seekFromStart(fp, 0);

    if (fileSize <= DEFAULT_ATOM_SIZE)
    {
        return INSUFFICIENT_DATA;
    }

    uint32 atomType          = UNKNOWN_ATOM;
    uint32 atomSize          = 0;
    bool oMovieAtomFound     = false;
    bool oMediaDataAtomFound = false;

    int32 fpos = 0;

    MP4_ERROR_CODE mp4ErrorCode = INSUFFICIENT_DATA;

    while ((uint32)(fpos + DEFAULT_ATOM_SIZE) < fileSize)
    {
        AtomUtils::getNextAtomType(fp, atomSize, atomType);
        if (atomSize < DEFAULT_ATOM_SIZE)
        {
            mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
            break;
        }

        if ((atomType == FILE_TYPE_ATOM) ||
                (atomType == FREE_SPACE_ATOM) ||
                (atomType == SKIP_ATOM) ||
                (atomType == USER_DATA_ATOM) ||
                (atomType == UUID_ATOM) ||
                (atomType == UNKNOWN_ATOM))
        {
            fpos += atomSize;
            metaDataSize = fpos;
            if ((uint32)fpos > fileSize)
            {
                break;
            }
            AtomUtils::seekFromStart(fp, fpos);
            continue;
        }
        else if (atomType == MOVIE_ATOM)
        {
            fpos += atomSize;
            oMovieAtomFound = true;
            metaDataSize = fpos;
        }
        else if (atomType == MEDIA_DATA_ATOM)
        {
            fpos += atomSize;
            oMediaDataAtomFound  = true;
            metaDataSize = fpos;
        }
        else
        {
            //error: should never get here
            mp4ErrorCode = DEFAULT_ERROR;
        }
        break;
    }

    if (oMovieAtomFound || oMediaDataAtomFound)
    {
        //at most one of two can be true
        oIsProgressiveDownloadable = oMovieAtomFound;
        mp4ErrorCode = EVERYTHING_FINE;
    }

    if (!oMovieAtomFound && (0 != AtomUtils::getFileBufferingCapacity(fp)))
    {
        // can't support progressive playback if no movie atom found
        mp4ErrorCode = NOT_PROGRESSIVE_STREAMABLE;
    }

    AtomUtils::CloseMP4File(fp);

    return (mp4ErrorCode);
}

OSCL_EXPORT_REF void IMpeg4File::DestroyMP4FileObject(IMpeg4File* aMP4FileObject)
{
    Mpeg4File* ptr = OSCL_STATIC_CAST(Mpeg4File*, aMP4FileObject);
    PV_MP4_FF_DELETE(NULL, Mpeg4File, ptr);
}
