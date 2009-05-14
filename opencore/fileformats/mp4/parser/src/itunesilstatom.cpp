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
#include "itunesilstatom.h"
#include "atomdefs.h"
#include "atomutils.h"
#include "oscl_int64_utils.h"
#include "oscl_utf8conv.h"

typedef Oscl_Vector<ItunesMeaningAtom*, OsclMemAllocator> ItunesMeaningAtomVecType;
typedef Oscl_Vector<ItunesNameAtom*, OsclMemAllocator> ItunesNameAtomVecType;

//************************************MeaningAtom Class Starts  **********************************
ItunesMeaningAtom::ItunesMeaningAtom(MP4_FF_FILE *fp,
                                     uint32 size,
                                     uint32 type)
        : FullAtom(fp, size, type)
{
    int32 nSize = (int32)(size - DEFAULT_FULL_ATOM_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (!AtomUtils::readByteData(fp, nSize, buf))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;


            }
            else
            {
                oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                _meaningString = temp;
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;


        }

        /*Delete the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;

    }

}


ItunesMeaningAtom::~ItunesMeaningAtom()
{


}

//************************************NameAtom Class Starts  **********************************
ItunesNameAtom::ItunesNameAtom(MP4_FF_FILE *fp,
                               uint32 size,
                               uint32 type)
        : FullAtom(fp, size, type)
{

    int32 nSize = (int32)(size - DEFAULT_FULL_ATOM_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (!AtomUtils::readByteData(fp, nSize, buf))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;

            }
            else
            {
                oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                _nameString = temp;
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;

        }

        /*Delete the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;

    }

}


ItunesNameAtom::~ItunesNameAtom()
{
}

//************************************BaseTypes for the MetaData **********************************
ITunesMetaDataAtom::ITunesMetaDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type): Atom(fp, size, type)
{

    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    Oscl_Int64_Utils::set_uint64(STRING_PREFIX, 0x00000001, 0);
    Oscl_Int64_Utils::set_uint64(INTEGER_PREFIX, 0x00000000, 0);
    Oscl_Int64_Utils::set_uint64(OTHER_PREFIX, 0x00000015, 0);
    Oscl_Int64_Utils::set_uint64(IMAGE_PREFIX_JFIF, 0x0000000D, 0);
    Oscl_Int64_Utils::set_uint64(IMAGE_PREFIX_PNG, 0x0000000E, 0);
}

ITunesMetaDataAtom::~ITunesMetaDataAtom()
{

}

//************************************ Title Class Starts  **********************************
ITunesTitleAtom::ITunesTitleAtom(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTitleAtom::ITunesTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _name = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTitleAtom::ITunesTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }

            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTitleAtom::ITunesTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTitleAtom::ITunesTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}


ITunesTitleAtom::~ITunesTitleAtom()
{
}

//************************************ Track's Subtitle Class Starts **********************************

ITunesTrackSubTitleAtom::ITunesTrackSubTitleAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTrackSubTitleAtom::ITunesTrackSubTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _trackTitle = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTrackSubTitleAtom::ITunesTrackSubTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }


            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTrackSubTitleAtom::ITunesTrackSubTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTrackSubTitleAtom::ITunesTrackSubTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesTrackSubTitleAtom::~ITunesTrackSubTitleAtom()
{
}


//************************************ Artist / Performer Class Starts  **********************************
ITunesArtistAtom::ITunesArtistAtom(MP4_FF_FILE *fp,
                                   uint32 size,
                                   uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesArtistAtom::ITunesArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _artist = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesArtistAtom::ITunesArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }

            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesArtistAtom::ITunesArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesArtistAtom::ITunesArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesArtistAtom::~ITunesArtistAtom()
{
}

//************************************AlbumArtist Class Starts  **********************************
ITunesAlbumArtistAtom::ITunesAlbumArtistAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumArtistAtom::ITunesAlbumArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _albumArtist = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumArtistAtom::ITunesAlbumArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }

            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumArtistAtom::ITunesAlbumArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumArtistAtom::ITunesAlbumArtistAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesAlbumArtistAtom::~ITunesAlbumArtistAtom()
{
}

//************************************ Album Class Starts  **********************************
ITunesAlbumAtom::ITunesAlbumAtom(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumAtom::ITunesAlbumAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _album = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumAtom::ITunesAlbumAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }


            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumAtom::ITunesAlbumAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesAlbumAtom::ITunesAlbumAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesAlbumAtom::~ITunesAlbumAtom()
{
}

//************************************ Genre Class Starts  **********************************
ITunesGenreAtom::ITunesGenreAtom(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    _gnreString = NULL;
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                _gnreVersion = STRING_GENRE;
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGenreAtom::ITunesGenreAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _gnreString = temp;
                    }
                }
                else if (_prefix == INTEGER_PREFIX)
                {
                    _gnreVersion = INTEGER_GENRE;
                    if (!AtomUtils::read16(fp, _gnreID))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGenreAtom::ITunesGenreAtom READ_ITUNES_ILST_META_DATA_FAILED  _prefix == INTEGER_PREFIX"));
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string/Integer"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGenreAtom::ITunesGenreAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }

            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGenreAtom::ITunesGenreAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGenreAtom::ITunesGenreAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesGenreAtom::~ITunesGenreAtom()
{
}

//************************************ Day Class Starts  **********************************
ITunesYearAtom::ITunesYearAtom(MP4_FF_FILE *fp,
                               uint32 size,
                               uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesYearAtom::ITunesYearAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _day = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTitleAtom::ITunesTitleAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesYearAtom::ITunesYearAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesYearAtom::ITunesYearAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesYearAtom::~ITunesYearAtom()
{
}

//************************************ Tool Class Starts  **********************************
ITunesToolAtom::ITunesToolAtom(MP4_FF_FILE *fp,
                               uint32 size,
                               uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesToolAtom::ITunesToolAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _tool = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesToolAtom::ITunesToolAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesToolAtom::ITunesToolAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesToolAtom::ITunesToolAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesToolAtom::~ITunesToolAtom()
{
}

//************************************ EncodedBy Class Starts  **********************************
ITunesEncodedByAtom::ITunesEncodedByAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesEncodedByAtom::ITunesEncodedByAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _encodedBy = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesEncodedByAtom::ITunesEncodedByAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesEncodedByAtom::ITunesEncodedByAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesEncodedByAtom::ITunesEncodedByAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}

ITunesEncodedByAtom::~ITunesEncodedByAtom()
{
}

//************************************ Writer Class Starts  **********************************
ITunesWriterAtom::ITunesWriterAtom(MP4_FF_FILE *fp,
                                   uint32 size,
                                   uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesWriterAtom::ITunesWriterAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _writer = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesWriterAtom::ITunesWriterAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesWriterAtom::ITunesWriterAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesWriterAtom::ITunesWriterAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesWriterAtom::~ITunesWriterAtom()
{
}

//************************************ Group Class Starts  **********************************
ITunesGroupAtom::ITunesGroupAtom(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGroupAtom::ITunesGroupAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _group = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGroupAtom::ITunesGroupAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGroupAtom::ITunesGroupAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesGroupAtom::ITunesGroupAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}


ITunesGroupAtom::~ITunesGroupAtom()
{
}

//************************************ Comment Class Starts  **********************************
ITunesCommentAtom::ITunesCommentAtom(MP4_FF_FILE *fp,
                                     uint32 size,
                                     uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCommentAtom::ITunesCommentAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _comment = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCommentAtom::ITunesCommentAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCommentAtom::ITunesCommentAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCommentAtom::ITunesCommentAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}


ITunesCommentAtom::~ITunesCommentAtom()
{

}


//************************************ Track Class Starts  **********************************
ITunesTracktAtom::ITunesTracktAtom(MP4_FF_FILE *fp,
                                   uint32 size,
                                   uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType;
    uint32 atomSize;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);

    if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
    {
        if (_prefix == INTEGER_PREFIX)
        {
            uint16 junk; // 2- Bytes representing 0x0000

            if (!AtomUtils::read16(fp, junk))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTracktAtom::ITunesTracktAtom READ_ITUNES_ILST_META_DATA_FAILED  !AtomUtils::read16(fp,junk)"));
                return;
            }

            if (!AtomUtils::read16read16(fp, _thisTrackNo, _totalTracks))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTracktAtom::ITunesTracktAtom READ_ITUNES_ILST_META_DATA_FAILED  !AtomUtils::read16read16(fp, _thisTrackNo, _totalTracks)"));
                return;
            }

            if (!AtomUtils::read16(fp, junk))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTracktAtom::ITunesTracktAtom READ_ITUNES_ILST_META_DATA_FAILED  !AtomUtils::read16(fp,junk)"));
                return;
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTracktAtom::ITunesTracktAtom READ_ITUNES_ILST_META_DATA_FAILED  else "));


        }


    }
}

ITunesTracktAtom::~ITunesTracktAtom()
{
}

//************************************ Compile Part Class Starts  **********************************
ITunesCompileAtom::ITunesCompileAtom(MP4_FF_FILE *fp,
                                     uint32 size,
                                     uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType, atomSize;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
    {
        if (_prefix == OTHER_PREFIX)
        {
            uint8 cplData;
            if (!AtomUtils::read8(fp, cplData))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCompileAtom::ITunesCompileAtom READ_ITUNES_ILST_META_DATA_FAILED  if(_prefix == OTHER_PREFIX)"));
                return;
            }
            if (cplData)
                _compilationPart = true;
            else
                _compilationPart = false;
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCompileAtom::ITunesCompileAtom READ_ITUNES_ILST_META_DATA_FAILED  else"));
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCompileAtom::ITunesCompileAtom READ_ITUNES_ILST_META_DATA_FAILED  if(_prefix == OTHER_PREFIX)"));
        return;

    }
}


ITunesCompileAtom::~ITunesCompileAtom()
{
}

//********************************* Content Rating Class Starts  ********************************
ITunesContentRatingAtom::ITunesContentRatingAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType, atomSize;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
    {
        if (_prefix == OTHER_PREFIX)
        {
            uint8 ratingData;
            if (!AtomUtils::read8(fp, ratingData))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesContentRatingAtom::ITunesContentRatingAtom READ_ITUNES_ILST_META_DATA_FAILED  if(_prefix == OTHER_PREFIX)"));
                return;
            }
            if (ratingData)
                _contentRating = true;
            else
                _contentRating = false;
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesContentRatingAtom::ITunesContentRatingAtom READ_ITUNES_ILST_META_DATA_FAILED  else"));
            return;
        }
    }
}


ITunesContentRatingAtom::~ITunesContentRatingAtom()
{
}




//************************************ Tempo Class Starts  **********************************
ITunesTempoAtom::ITunesTempoAtom(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType, atomSize;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
    {
        if (_prefix == OTHER_PREFIX)
        {
            if (!AtomUtils::read16(fp, _beatsPerMin))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTempoAtom::ITunesTempoAtom READ_ITUNES_ILST_META_DATA_FAILED  if(_prefix == OTHER_PREFIX)"));
                return;
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesTempoAtom::ITunesTempoAtom READ_ITUNES_ILST_META_DATA_FAILED  else"));
            return;
        }
    }
}


ITunesTempoAtom::~ITunesTempoAtom()
{

}

//************************************ Copyright Class Starts  **********************************
ITunesCopyrightAtom::ITunesCopyrightAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCopyrightAtom::ITunesCopyrightAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _cprt = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCopyrightAtom::ITunesCopyrightAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCopyrightAtom::ITunesCopyrightAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCopyrightAtom::ITunesCopyrightAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}


ITunesCopyrightAtom::~ITunesCopyrightAtom()
{

}

//************************************ Description Class Starts  **********************************
ITunesDescriptionAtom::ITunesDescriptionAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDescriptionAtom::ITunesDescriptionAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _desc = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDescriptionAtom::ITunesDescriptionAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDescriptionAtom::ITunesDescriptionAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDescriptionAtom::ITunesDescriptionAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }

}


ITunesDescriptionAtom::~ITunesDescriptionAtom()
{
}

//************************************ Disk Data Starts  **********************************
ITunesDiskDatatAtom::ITunesDiskDatatAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType;
    uint32 atomSize;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);

    if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
    {
        if (_prefix == INTEGER_PREFIX)
        {
            uint16 junk; // 2- Bytes representing 0x0000

            if (!AtomUtils::read16(fp, junk))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDiskDatatAtom::ITunesDiskDatatAtom READ_ITUNES_ILST_META_DATA_FAILED  if(!AtomUtils::read16(fp,junk))"));
                return;
            }

            if (!AtomUtils::read16read16(fp, _thisDiskNo, _totalDisks))
            {
                _success = false;
                _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDiskDatatAtm::ITunesDiskDatatAtom READ_ITUNES_ILST_META_DATA_FAILED  if(!AtomUtils::read16read16(fp, _thisDiskNo, _totalDisks))"));
                return;
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesDiskDatatAtm::ITunesDiskDatatAtom READ_ITUNES_ILST_META_DATA_FAILED  else"));
            return;
        }
    }
}


ITunesDiskDatatAtom::~ITunesDiskDatatAtom()
{

}

//************************************ Free Form Data Class Starts  **********************************
ITunesFreeFormDataAtom::ITunesFreeFormDataAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = type;
    int32 nSize = 0;
    nSize = (int32)(size - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesFreeFormDataAtom::ITunesFreeFormDataAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _StringData = temp;
                    }
                }
                else if (_prefix == OTHER_PREFIX)
                {
                    // reading the data to keep atom alignment
                    uint32 readData;
                    if (!AtomUtils::read32(fp, readData))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesFreeFormDataAtom::ITunesFreeFormDataAtom Read four bytes (%d) in OTHER_PREFIX", readData));
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix  doesnt match.
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesFreeFormDataAtom::ITunesFreeFormDataAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }




            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesFreeFormDataAtom::ITunesFreeFormDataAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesFreeFormDataAtom::ITunesFreeFormDataAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }


}


ITunesFreeFormDataAtom::~ITunesFreeFormDataAtom()
{

}

//************************************ Lyrics Class Starts  **********************************
ITunesLyricsAtom::ITunesLyricsAtom(MP4_FF_FILE *fp,
                                   uint32 size,
                                   uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM, atomSize = 0;
    int32 nSize = 0;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    nSize = (int32)(atomSize - PREFIX_SIZE);
    if (nSize > 0)
    {
        uint8* buf = NULL;
        uint8* outbuf = NULL;
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, nSize, buf);
        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, ((nSize + 1)*sizeof(oscl_wchar)), outbuf);
        if (buf && outbuf)
        {
            if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
            {
                if (_prefix == STRING_PREFIX)
                {
                    if (!AtomUtils::readByteData(fp, nSize, buf))
                    {
                        _success = false;
                        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesLyricsAtom::ITunesLyricsAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                    }
                    else
                    {
                        oscl_UTF8ToUnicode((const char *)buf, nSize, (oscl_wchar*)outbuf, nSize + 1);
                        OSCL_wHeapString<OsclMemAllocator> temp((const oscl_wchar *)outbuf);
                        _lyrics = temp;
                    }
                }
                else //if atomType is not "DataAtom" and/or _prefix is not "string"
                {

                    _success = false;
                    _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
                    PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesLyricsAtom::ITunesLyricsAtom READ_ITUNES_ILST_META_DATA_FAILED"));
                }



            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
            PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesLyricsAtom::ITunesLyricsAtom READ_ITUNES_ILST_META_DATA_FAILED"));

        }
        /*Deleting the buffers*/
        if (buf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
            buf = NULL;
        }
        if (outbuf)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, outbuf);
            outbuf = NULL;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_ITUNES_ILST_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesLyricsAtom::ITunesLyricsAtom READ_ITUNES_ILST_META_DATA_FAILED"));

    }
}


ITunesLyricsAtom::~ITunesLyricsAtom()
{
}

//************************************ CoverImage Class Starts  **********************************
ITunesCoverImageAtom::ITunesCoverImageAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : ITunesMetaDataAtom(fp, size, type)
{
    uint32 atomType = UNKNOWN_ATOM;
    uint32 atomSize = 0;
    _ImageData = NULL;
    AtomUtils::getNextAtomType(fp, atomSize, atomType);
    int32 count = (int32)(atomSize - DEFAULT_ATOM_SIZE);
    if (count > 0)
    {

        if (atomType == ITUNES_ILST_DATA_ATOM && AtomUtils::read64(fp, _prefix))
        {
            count -= 8;
            if (count < ITUNES_MAX_COVER_IMAGE_SIZE)
            {
                //treat rest of the atom as image
                PV_MP4_FF_NEW(fp->auditCB, PvmfApicStruct, (), _ImageData);
                PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, uint8, count, _ImageData->iGraphicData);
                AtomUtils::readByteData(fp, count, _ImageData->iGraphicData);
                _ImageData->iGraphicDataLen = count;
            }
            else
            {
                _success = false;
                PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCoverImageAtom::ITunesCoverImageAtom READ_ITUNES_ILST_META_DATA_FAILED  else[if (_prefix == IMAGE_PREFIX_PNG)]  )"));
            }
        }

    }
    else
    {
        _success = false;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>ITunesCoverImageAtom::ITunesCoverImageAtom READ_ITUNES_ILST_META_DATA_FAILED  else[if (_prefix == IMAGE_PREFIX_PNG)]  )"));

    }

}


ITunesCoverImageAtom::~ITunesCoverImageAtom()
{
    if (_ImageData != NULL)
    {
        if (_ImageData->iGraphicData != NULL)
        {
            PV_MP4_ARRAY_FREE(fp->auditCB, _ImageData->iGraphicData);

        }
        PV_MP4_FF_DELETE(fp->auditCB, PvmfApicStruct, _ImageData);
        _ImageData = NULL;
    }
}
//************************************ Lyrics Class Ends  **********************************
ITunesILSTAtom::ITunesILSTAtom(MP4_FF_FILE *fp, uint32 size, uint32 type): Atom(fp, size, type)
{
    _success = true;

    _pITunesMeaningAtom = NULL;
    _pITunesNameAtom    = NULL;
    _pMeaningAtomVec    = NULL;
    _pNameAtomVec       = NULL;
    _pITunesTitleAtom = NULL;
    _pITunesTrackSubTitleAtom = NULL;
    _pITunesCompileAtom = NULL;
    _pITunesContentRatingAtom = NULL;
    _pITunesTempoAtom =  NULL;
    _pITunesCopyrightAtom =  NULL;
    _pITunesDescriptionAtom =  NULL;
    _pITunesToolAtom = NULL;
    _pITunesEncodedByAtom = NULL;
    _pITunesNormalizationFreeFormDataAtom = NULL;
    _pITunesNormalizationFreeFormDataToolAtom = NULL;
    _iITunesCDIdentifierFreeFormDataAtomNum = 0;
    for (uint8 i = 0; i < MAX_CD_IDENTIFIER_FREE_DATA_ATOM; i++)
    {
        _pITunesCDIdentifierFreeFormDataAtom[i] = NULL;
    }
    //Create the vectors
    PV_MP4_FF_NEW(fp->auditCB, ItunesMeaningAtomVecType, (), _pMeaningAtomVec);
    PV_MP4_FF_NEW(fp->auditCB, ItunesNameAtomVecType, (), _pNameAtomVec);


    _pITunesCDTrackNumberFreeFormDataAtom = NULL;
    _pITunesCDDB1FreeFormDataAtom = NULL;
    _pITunesAlbumAtom = NULL;
    _pITunesArtistAtom = NULL;
    _pITunesAlbumArtistAtom = NULL;
    _pITunesGenreAtom = NULL;
    _pITunesYearAtom = NULL;
    _pITunesWriterAtom = NULL;
    _pITunesGroupAtom = NULL;
    _pITunesCommentAtom = NULL;
    _pITunesTracktAtom = NULL;
    _pITunesDiskDatatAtom = NULL;
    _pITunesLyricsAtom = NULL;
    _pITunesCoverImageAtom = NULL;

    uint32 count = _size - DEFAULT_ATOM_SIZE;

    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    while (count > 0)
    {
        uint32 atomsize = 0;
        uint32 atomType = 0;
        uint32 currPtr = AtomUtils::getCurrentFilePosition(fp);
        AtomUtils::getNextAtomType(fp, atomsize, atomType);

        if (count < atomsize)
        {
            AtomUtils::seekFromStart(fp, currPtr);
            AtomUtils::seekFromCurrPos(fp, count);
            count = 0;
            return;
        }

        if (atomType == ITUNES_SONG_TITLE_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesTitleAtom, (fp, atomsize, atomType), _pITunesTitleAtom);

            if (!_pITunesTitleAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesTitleAtom, _pITunesTitleAtom);
                _pITunesTitleAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesTitleAtom->getSize();
        }
        else if (atomType == ITUNES_TRACK_SUBTITLE_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesTrackSubTitleAtom, (fp, atomsize, atomType), _pITunesTrackSubTitleAtom);

            if (!_pITunesTrackSubTitleAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesTrackSubTitleAtom, _pITunesTrackSubTitleAtom);
                _pITunesTrackSubTitleAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesTrackSubTitleAtom->getSize();
        }
        else if (atomType == ITUNES_COMPILATION_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesCompileAtom, (fp, atomsize, atomType), _pITunesCompileAtom);

            if (!_pITunesCompileAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesCompileAtom, _pITunesCompileAtom);
                _pITunesCompileAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesCompileAtom->getSize();
        }
        else if (atomType == ITUNES_CONTENT_RATING_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesContentRatingAtom, (fp, atomsize, atomType), _pITunesContentRatingAtom);

            if (!_pITunesContentRatingAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesContentRatingAtom, _pITunesContentRatingAtom);
                _pITunesContentRatingAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesContentRatingAtom->getSize();
        }
        else if (atomType == ITUNES_BPM_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesTempoAtom, (fp, atomsize, atomType), _pITunesTempoAtom);

            if (!_pITunesTempoAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                if (_pITunesTempoAtom != NULL)
                {
                    PV_MP4_FF_DELETE(NULL, ITunesTempoAtom, _pITunesTempoAtom);
                    _pITunesTempoAtom = NULL;
                }
                count -= atomsize;
            }
            else
                count -= _pITunesTempoAtom->getSize();
        }
        else if (atomType == ITUNES_COPYRIGHT_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesCopyrightAtom, (fp, atomsize, atomType), _pITunesCopyrightAtom);

            if (!_pITunesCopyrightAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesCopyrightAtom, _pITunesCopyrightAtom);
                _pITunesCopyrightAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesCopyrightAtom->getSize();
        }
        else if (atomType == ITUNES_DESCRIPTION_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesDescriptionAtom, (fp, atomsize, atomType), _pITunesDescriptionAtom);

            if (!_pITunesDescriptionAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesDescriptionAtom, _pITunesDescriptionAtom);
                _pITunesDescriptionAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesDescriptionAtom->getSize();
        }
        else if (atomType == ITUNES_ENCODER_TOOL_ATOM)
        {
            //Software(tool) which encoded the recording.
            PV_MP4_FF_NEW(fp->auditCB, ITunesToolAtom, (fp, atomsize, atomType), _pITunesToolAtom);

            if (!_pITunesToolAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesToolAtom, _pITunesToolAtom);
                _pITunesToolAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesToolAtom->getSize();
        }
        else if (atomType == ITUNES_ENCODEDBY_ATOM)
        {
            //Person or company that encoded the recording.
            PV_MP4_FF_NEW(fp->auditCB, ITunesEncodedByAtom, (fp, atomsize, atomType), _pITunesEncodedByAtom);

            if (!_pITunesEncodedByAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesEncodedByAtom, _pITunesEncodedByAtom);
                _pITunesEncodedByAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesEncodedByAtom->getSize();
        }

        else if (atomType == ITUNES_FREE_FORM_ATOM)
        {

            uint32 FreeFormAtomType = UNKNOWN_ATOM;
            uint32 FreeFormAtomSize = 0;
            count -= DEFAULT_ATOM_SIZE;
            atomsize -= DEFAULT_ATOM_SIZE;

            while (((FreeFormAtomType == ITUNES_MEAN_ATOM) ||
                    (FreeFormAtomType == ITUNES_FREE_FORM_DATA_NAME_ATOM) ||
                    (FreeFormAtomType == ITUNES_ILST_DATA_ATOM) ||
                    (FreeFormAtomType == UNKNOWN_ATOM)) &&
                    (atomsize > 0))
            {

                uint32 currPos = AtomUtils::getCurrentFilePosition(fp);
                AtomUtils::getNextAtomType(fp, FreeFormAtomSize, FreeFormAtomType);
                if (FreeFormAtomType == ITUNES_MEAN_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, ItunesMeaningAtom, (fp, FreeFormAtomSize, FreeFormAtomType), _pITunesMeaningAtom);
                    if (!_pITunesMeaningAtom->MP4Success())
                    {
                        AtomUtils::seekFromStart(fp, currPos);
                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                        PV_MP4_FF_DELETE(NULL, ItunesMeaningAtom, _pITunesMeaningAtom);
                        _pITunesMeaningAtom = NULL;
                        count -= FreeFormAtomSize;
                    }
                    else
                    {
                        count -= _pITunesMeaningAtom->getSize();
                    }
                    atomsize -= FreeFormAtomSize;
                    (*_pMeaningAtomVec).push_back(_pITunesMeaningAtom);

                }
                else if (FreeFormAtomType == ITUNES_FREE_FORM_DATA_NAME_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, ItunesNameAtom, (fp, FreeFormAtomSize, FreeFormAtomType), _pITunesNameAtom);
                    if (!_pITunesNameAtom->MP4Success())
                    {
                        AtomUtils::seekFromStart(fp, currPos);
                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                        PV_MP4_FF_DELETE(NULL, ItunesNameAtom, _pITunesNameAtom);
                        _pITunesNameAtom = NULL;
                        count -= FreeFormAtomSize;
                    }
                    else
                    {
                        count -= _pITunesNameAtom->getSize();
                    }
                    atomsize -= FreeFormAtomSize;
                    (*_pNameAtomVec).push_back(_pITunesNameAtom);

                }
                else if (FreeFormAtomType == ITUNES_ILST_DATA_ATOM)
                {
                    OSCL_wHeapString<OsclMemAllocator> tempNameString = NULL;
                    if (_pITunesNameAtom)
                        tempNameString =  _pITunesNameAtom->getNameString();
                    const oscl_wchar *wStringPtr = tempNameString.get_cstr();
                    uint32 nameStringLen = tempNameString.get_size();
                    if (nameStringLen > 0)
                    {
                        char* buf = NULL;
                        PV_MP4_FF_ARRAY_MALLOC(fp->auditCB, char, (nameStringLen + 1), buf);
                        if (buf)
                        {

                            oscl_UnicodeToUTF8(wStringPtr, nameStringLen, buf, nameStringLen + 1);
                            if (!(oscl_strcmp(buf, ITUNES_FREE_FORM_DATA_ATOM_TYPE_NORM)))
                            {
                                if (_pITunesNormalizationFreeFormDataAtom == NULL)
                                {
                                    PV_MP4_FF_NEW(fp->auditCB,
                                                  ITunesFreeFormDataAtom,
                                                  (fp, FreeFormAtomSize, FreeFormAtomType),
                                                  _pITunesNormalizationFreeFormDataAtom);
                                    if (!_pITunesNormalizationFreeFormDataAtom->MP4Success())
                                    {
                                        AtomUtils::seekFromStart(fp, currPos);
                                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                                    }
                                    count -= _pITunesNormalizationFreeFormDataAtom->getSize();
                                    atomsize -= _pITunesNormalizationFreeFormDataAtom->getSize();
                                }
                                else //Duplicate atom
                                {
                                    AtomUtils::seekFromStart(fp, currPos);
                                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                                    count -= FreeFormAtomSize;
                                    atomsize -= FreeFormAtomSize;

                                }

                            }
                            else if (!(oscl_strcmp(buf, ITUNES_FREE_FORM_DATA_ATOM_TYPE_TOOL)))
                            {
                                if (_pITunesNormalizationFreeFormDataToolAtom == NULL)
                                {
                                    PV_MP4_FF_NEW(fp->auditCB,
                                                  ITunesFreeFormDataAtom,
                                                  (fp, FreeFormAtomSize, FreeFormAtomType),
                                                  _pITunesNormalizationFreeFormDataToolAtom);
                                    if (!_pITunesNormalizationFreeFormDataToolAtom->MP4Success())
                                    {
                                        AtomUtils::seekFromStart(fp, currPos);
                                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                                    }
                                    count -= _pITunesNormalizationFreeFormDataToolAtom->getSize();
                                    atomsize -= _pITunesNormalizationFreeFormDataToolAtom->getSize();
                                }
                                else //Duplicate atom
                                {
                                    AtomUtils::seekFromStart(fp, currPos);
                                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                                    count -= FreeFormAtomSize;
                                    atomsize -= FreeFormAtomSize;
                                }
                            }
                            else if (!(oscl_strcmp(buf, ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB1)))
                            {
                                if (_pITunesCDDB1FreeFormDataAtom == NULL)
                                {
                                    PV_MP4_FF_NEW(fp->auditCB,
                                                  ITunesFreeFormDataAtom,
                                                  (fp, FreeFormAtomSize, FreeFormAtomType),
                                                  _pITunesCDDB1FreeFormDataAtom);
                                    if (!_pITunesCDDB1FreeFormDataAtom->MP4Success())
                                    {
                                        AtomUtils::seekFromStart(fp, currPos);
                                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                                    }
                                    count -= _pITunesCDDB1FreeFormDataAtom->getSize();
                                    atomsize -= _pITunesCDDB1FreeFormDataAtom->getSize();
                                }
                                else //Duplicate atom
                                {
                                    AtomUtils::seekFromStart(fp, currPos);
                                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                                    count -= FreeFormAtomSize;
                                    atomsize -= FreeFormAtomSize;

                                }
                            }
                            else if (!(oscl_strcmp(buf, ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB_TRACKNUMBER)))
                            {
                                if (_pITunesCDTrackNumberFreeFormDataAtom == NULL)
                                {
                                    PV_MP4_FF_NEW(fp->auditCB,
                                                  ITunesFreeFormDataAtom,
                                                  (fp, FreeFormAtomSize, FreeFormAtomType),
                                                  _pITunesCDTrackNumberFreeFormDataAtom);
                                    if (!_pITunesCDTrackNumberFreeFormDataAtom->MP4Success())
                                    {
                                        AtomUtils::seekFromStart(fp, currPos);
                                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                                    }
                                    count -= _pITunesCDTrackNumberFreeFormDataAtom->getSize();
                                    atomsize -=  _pITunesCDTrackNumberFreeFormDataAtom->getSize();
                                }
                                else //Duplicate atom
                                {
                                    AtomUtils::seekFromStart(fp, currPos);
                                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                                    count -= FreeFormAtomSize;
                                    atomsize -= FreeFormAtomSize;

                                }
                            }
                            else if (!(oscl_strcmp(buf, ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB_IDS)))
                            {

                                PV_MP4_FF_NEW(fp->auditCB,
                                              ITunesFreeFormDataAtom,
                                              (fp, FreeFormAtomSize, FreeFormAtomType),
                                              _pITunesCDIdentifierFreeFormDataAtom[_iITunesCDIdentifierFreeFormDataAtomNum]);

                                if (!_pITunesCDIdentifierFreeFormDataAtom[_iITunesCDIdentifierFreeFormDataAtomNum]->MP4Success())
                                {
                                    AtomUtils::seekFromStart(fp, currPos);
                                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                                }
                                count -= _pITunesCDIdentifierFreeFormDataAtom[_iITunesCDIdentifierFreeFormDataAtomNum]->getSize();
                                atomsize -= _pITunesCDIdentifierFreeFormDataAtom[_iITunesCDIdentifierFreeFormDataAtomNum]->getSize();
                                _iITunesCDIdentifierFreeFormDataAtomNum++;

                            }
                            else /*Ignore the DataAtom */
                            {
                                atomsize -= FreeFormAtomSize;
                                count -= FreeFormAtomSize;
                                AtomUtils::seekFromStart(fp, currPos);
                                AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                            }
                        }
                        else
                        {
                            /*Skip the DataAtom when there is memory error*/
                            atomsize -= FreeFormAtomSize;
                            count -= FreeFormAtomSize;
                            AtomUtils::seekFromStart(fp, currPos);
                            AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                        }
                        /*Delete the buffer */
                        if (buf)
                        {
                            PV_MP4_ARRAY_FREE(fp->auditCB, buf);
                            buf = NULL;
                        }

                    }
                    else
                    {
                        /*Skip the DataAtom when there is no NameAtom*/
                        atomsize -= FreeFormAtomSize;
                        count -= FreeFormAtomSize;
                        AtomUtils::seekFromStart(fp, currPos);
                        AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);

                    }
                }
                else /*Ignore the unrecognized atom */
                {
                    atomsize -= FreeFormAtomSize;
                    count -= FreeFormAtomSize;
                    AtomUtils::seekFromStart(fp, currPos);
                    AtomUtils::seekFromCurrPos(fp, FreeFormAtomSize);
                }

            } //End of While

        }
        else if (atomType == ITUNES_ALBUM_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesAlbumAtom, (fp, atomsize, atomType), _pITunesAlbumAtom);

            if (!_pITunesAlbumAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesAlbumAtom, _pITunesAlbumAtom);
                _pITunesAlbumAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesAlbumAtom->getSize();
        }
        else if ((atomType == ITUNES_ARTIST1_ATOM) || (atomType == ITUNES_ARTIST2_ATOM))
        {
            if (_pITunesArtistAtom == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB, ITunesArtistAtom, (fp, atomsize, atomType), _pITunesArtistAtom);

                if (!_pITunesArtistAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomsize);
                    PV_MP4_FF_DELETE(NULL, ITunesArtistAtom, _pITunesArtistAtom);
                    _pITunesArtistAtom = NULL;
                    count -= atomsize;
                }
                else
                    count -= _pITunesArtistAtom->getSize();
            }
            else //Skip it
            {
                count -= atomsize;
                atomsize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomsize);

            }
        }
        else if (atomType == ITUNES_ALBUM_ARTIST_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesAlbumArtistAtom, (fp, atomsize, atomType), _pITunesAlbumArtistAtom);

            if (!_pITunesAlbumArtistAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesAlbumArtistAtom, _pITunesAlbumArtistAtom);
                _pITunesAlbumArtistAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesAlbumArtistAtom->getSize();
        }
        else if (atomType == ITUNES_GENRE1_ATOM || atomType == ITUNES_GENRE2_ATOM)
        {
            if (_pITunesGenreAtom == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB, ITunesGenreAtom, (fp, atomsize, atomType), _pITunesGenreAtom);
                if (!_pITunesGenreAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomsize);
                    PV_MP4_FF_DELETE(NULL, ITunesGenreAtom, _pITunesGenreAtom);
                    _pITunesGenreAtom = NULL;
                    count -= atomsize;
                }
                else
                    count -= _pITunesGenreAtom->getSize();
            }
            else
            {
                count -= atomsize;
                atomsize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomsize);
            }
        }
        else if (atomType == ITUNES_YEAR_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesYearAtom, (fp, atomsize, atomType), _pITunesYearAtom);

            if (!_pITunesYearAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesYearAtom, _pITunesYearAtom);
                _pITunesYearAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesYearAtom->getSize();
        }
        else if (atomType == ITUNES_COMPOSER_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesWriterAtom, (fp, atomsize, atomType), _pITunesWriterAtom);

            if (!_pITunesWriterAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesWriterAtom, _pITunesWriterAtom);
                _pITunesWriterAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesWriterAtom->getSize();
        }
        else if ((atomType == ITUNES_GROUPING1_ATOM) || (atomType == ITUNES_GROUPING2_ATOM))
        {
            if (_pITunesGroupAtom == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB, ITunesGroupAtom, (fp, atomsize, atomType), _pITunesGroupAtom);

                if (!_pITunesGroupAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomsize);
                    PV_MP4_FF_DELETE(NULL, ITunesGroupAtom, _pITunesGroupAtom);
                    _pITunesGroupAtom = NULL;
                    count -= atomsize;
                }
                else
                    count -= _pITunesGroupAtom->getSize();
            }
            else
            {
                count -= atomsize;
                atomsize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomsize);
            }
        }
        else if (atomType == ITUNES_COMMENT_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesCommentAtom, (fp, atomsize, atomType), _pITunesCommentAtom);

            if (!_pITunesCommentAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesCommentAtom, _pITunesCommentAtom);
                _pITunesCommentAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesCommentAtom->getSize();
        }
        else if (atomType == ITUNES_TRACK_NUMBER_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesTracktAtom, (fp, atomsize, atomType), _pITunesTracktAtom);

            if (!_pITunesTracktAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesTracktAtom, _pITunesTracktAtom);
                _pITunesTracktAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesTracktAtom->getSize();
        }
        else if (atomType == ITUNES_ART_WORK_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesCoverImageAtom, (fp, atomsize, atomType), _pITunesCoverImageAtom);

            if (!_pITunesCoverImageAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesCoverImageAtom, _pITunesCoverImageAtom);
                _pITunesCoverImageAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesCoverImageAtom->getSize();
        }
        else if (atomType == ITUNES_DISK_NUMBER_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesDiskDatatAtom, (fp, atomsize, atomType), _pITunesDiskDatatAtom);

            if (!_pITunesDiskDatatAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesDiskDatatAtom, _pITunesDiskDatatAtom);
                _pITunesDiskDatatAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesDiskDatatAtom->getSize();
        }
        else if (atomType == ITUNES_LYRICS_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesLyricsAtom, (fp, atomsize, atomType), _pITunesLyricsAtom);

            if (!_pITunesLyricsAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomsize);
                PV_MP4_FF_DELETE(NULL, ITunesLyricsAtom, _pITunesLyricsAtom);
                _pITunesLyricsAtom = NULL;
                count -= atomsize;
            }
            else
                count -= _pITunesLyricsAtom->getSize();
        }
        else
        {
            if (atomsize > DEFAULT_ATOM_SIZE)
            {
                count -= atomsize;
                atomsize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomsize);
            }
            else
            {
                // skip rest of the atom
                AtomUtils::seekFromCurrPos(fp, atomsize);
                count = 0;
            }
        }
    }
}

ITunesILSTAtom::~ITunesILSTAtom()
{

    if (_pMeaningAtomVec != NULL)
    {
        for (uint32 i = 0; i < _pMeaningAtomVec->size(); i++)
        {
            ItunesMeaningAtom *ptr = (ItunesMeaningAtom *)(*_pMeaningAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, ItunesMeaningAtom, ptr);

        }
    }
    if (_pNameAtomVec != NULL)
    {
        for (uint32 i = 0; i < _pNameAtomVec->size(); i++)
        {
            ItunesNameAtom *ptr = (ItunesNameAtom *)(*_pNameAtomVec)[i];
            PV_MP4_FF_DELETE(NULL, ItunesNameAtom, ptr);

        }
    }
    //Delete the  vectors
    PV_MP4_FF_TEMPLATED_DELETE(NULL, ItunesNameAtomVecType, Oscl_Vector, _pNameAtomVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, ItunesMeaningAtomVecType, Oscl_Vector, _pMeaningAtomVec);

    if (_pITunesTitleAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesTitleAtom, _pITunesTitleAtom);
    }
    if (_pITunesTrackSubTitleAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesTrackSubTitleAtom, _pITunesTrackSubTitleAtom);
    }

    if (_pITunesCompileAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesCompileAtom, _pITunesCompileAtom);
    }
    if (_pITunesContentRatingAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesContentRatingAtom, _pITunesContentRatingAtom);
    }
    if (_pITunesTempoAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesTempoAtom, _pITunesTempoAtom);
    }
    if (_pITunesCopyrightAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesCopyrightAtom, _pITunesCopyrightAtom);
    }
    if (_pITunesDescriptionAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesDescriptionAtom, _pITunesDescriptionAtom);
    }
    if (_pITunesToolAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesToolAtom, _pITunesToolAtom);
    }
    if (_pITunesEncodedByAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesEncodedByAtom, _pITunesEncodedByAtom);
    }
    if (_pITunesNormalizationFreeFormDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesFreeFormDataAtom, _pITunesNormalizationFreeFormDataAtom);
    }
    if (_pITunesNormalizationFreeFormDataToolAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesFreeFormDataAtom, _pITunesNormalizationFreeFormDataToolAtom);
    }
    for (uint8 ii = 0; ii < _iITunesCDIdentifierFreeFormDataAtomNum; ii++)
    {
        if (_pITunesCDIdentifierFreeFormDataAtom[ii] != NULL)
        {
            PV_MP4_FF_DELETE(NULL, ITunesFreeFormDataAtom, _pITunesCDIdentifierFreeFormDataAtom[ii]);
        }
    }

    if (_pITunesCDDB1FreeFormDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesFreeFormDataAtom, _pITunesCDDB1FreeFormDataAtom);
    }
    if (_pITunesCDTrackNumberFreeFormDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesFreeFormDataAtom, _pITunesCDTrackNumberFreeFormDataAtom);
    }
    if (_pITunesAlbumAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesAlbumAtom, _pITunesAlbumAtom);
    }
    if (_pITunesArtistAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesArtistAtom, _pITunesArtistAtom);
    }
    if (_pITunesAlbumArtistAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesAlbumArtistAtom, _pITunesAlbumArtistAtom);
    }
    if (_pITunesGenreAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesGenreAtom, _pITunesGenreAtom);
    }
    if (_pITunesYearAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesYearAtom, _pITunesYearAtom);
    }
    if (_pITunesWriterAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesWriterAtom, _pITunesWriterAtom);
    }
    if (_pITunesGroupAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesGroupAtom, _pITunesGroupAtom);
    }
    if (_pITunesCommentAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesCommentAtom, _pITunesCommentAtom);
    }
    if (_pITunesTracktAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesTracktAtom, _pITunesTracktAtom);
    }
    if (_pITunesDiskDatatAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesDiskDatatAtom, _pITunesDiskDatatAtom);
    }
    if (_pITunesLyricsAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesLyricsAtom, _pITunesLyricsAtom);
    }
    if (_pITunesCoverImageAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesCoverImageAtom, _pITunesCoverImageAtom);
    }
}


