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


#define __IMPLEMENT_UserDataAtom__

#include "userdataatom.h"
#include "atomdefs.h"


Atom *UserDataAtom::getAtomOfType(uint32 type)
{
    if (type == PVUSER_DATA_ATOM)
        return _pPVUserDataAtom;


    else if (type == META_DATA_ATOM)
        return _pMetaDataAtom;

    else if (type == PV_CONTENT_TYPE_ATOM)
        return _pPVContentTypeAtom;
    else
        return NULL;
}

typedef Oscl_Vector<AssetInfoTitleAtom*, OsclMemAllocator> assetInfoTitleAtomVecType;
typedef Oscl_Vector<AssetInfoDescAtom*, OsclMemAllocator> assetInfoDescAtomVecType;
typedef Oscl_Vector<CopyRightAtom*, OsclMemAllocator> copyRightAtomVecType;
typedef Oscl_Vector<AssetInfoPerformerAtom*, OsclMemAllocator> assetInfoPerformerAtomVecType;
typedef Oscl_Vector<AssetInfoAuthorAtom*, OsclMemAllocator> assetInfoAuthorAtomVecType;
typedef Oscl_Vector<AssetInfoGenreAtom*, OsclMemAllocator> assetInfoGenreAtomVecType;
typedef Oscl_Vector<AssetInfoRatingAtom*, OsclMemAllocator> assetInfoRatingAtomVecType;
typedef Oscl_Vector<AssetInfoClassificationAtom*, OsclMemAllocator> assetInfoClassificationAtomVecType;
typedef Oscl_Vector<AssetInfoKeyWordAtom*, OsclMemAllocator> assetInfoKeyWordAtomVecType;
typedef Oscl_Vector<AssetInfoLocationAtom*, OsclMemAllocator> assetInfoLocationAtomVecType;
typedef Oscl_Vector<AssetInfoAlbumAtom*, OsclMemAllocator> assetInfoAlbumAtomVecType;
typedef Oscl_Vector<AssetInfoRecordingYearAtom*, OsclMemAllocator> assetInfoRecordingYearAtomVecType;


// Stream-in Constructor
UserDataAtom::UserDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _pPVUserDataAtom = NULL;

    _pMetaDataAtom = NULL;

    _pPVContentTypeAtom = NULL;


    PV_MP4_FF_NEW(fp->auditCB, assetInfoTitleAtomVecType, (), _pAssetInfoTitleAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoDescAtomVecType, (), _pAssetInfoDescAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, copyRightAtomVecType, (), _pCopyRightAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoPerformerAtomVecType, (), _pAssetInfoPerformerAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoAuthorAtomVecType, (), _pAssetInfoAuthorAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoGenreAtomVecType, (), _pAssetInfoGenreAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoRatingAtomVecType, (), _pAssetInfoRatingAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoClassificationAtomVecType, (), _pAssetInfoClassificationAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoKeyWordAtomVecType, (), _pAssetInfoKeyWordAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoLocationAtomVecType, (), _pAssetInfoLocationAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoAlbumAtomVecType, (), _pAssetInfoAlbumAtomArray);
    PV_MP4_FF_NEW(fp->auditCB, assetInfoRecordingYearAtomVecType, (), _pAssetInfoRecordingYearArray);

    uint32 count = _size - DEFAULT_ATOM_SIZE;

    if (_success)
    {
        while (count > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;
            uint32 currPtr = 0;

            currPtr = AtomUtils::getCurrentFilePosition(fp);
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomSize > count)
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, count);
                count = 0;
                return;
            }

            if (atomType == COPYRIGHT_ATOM)
            {
                CopyRightAtom *pCopyRightAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, CopyRightAtom, (fp, atomSize, atomType), pCopyRightAtom);
                if (!pCopyRightAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, CopyRightAtom, pCopyRightAtom);
                    pCopyRightAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pCopyRightAtomArray).push_back(pCopyRightAtom);
                    pCopyRightAtom->setParent(this);
                    count -= pCopyRightAtom->getSize();
                }
            }
            else if (atomType == PVUSER_DATA_ATOM)
            {
                PV_MP4_FF_NEW(fp->auditCB, PVUserDataAtom, (fp, atomSize, atomType), _pPVUserDataAtom);
                if (!_pPVUserDataAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, PVUserDataAtom, _pPVUserDataAtom);
                    _pPVUserDataAtom = NULL;
                    count -= atomSize;
                }
                else
                    count -= _pPVUserDataAtom->getSize();
            }
            else if (atomType == PV_CONTENT_TYPE_ATOM)
            {
                PV_MP4_FF_NEW(fp->auditCB, PVContentTypeAtom, (fp, atomSize, atomType), _pPVContentTypeAtom);
                if (!_pPVContentTypeAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, PVContentTypeAtom, _pPVContentTypeAtom);
                    _pPVContentTypeAtom = NULL;
                    count -= atomSize;
                }
                else
                    count -= _pPVContentTypeAtom->getSize();
            }
            else if (atomType == ASSET_INFO_TITLE_ATOM)
            {
                AssetInfoTitleAtom *pAssetInfoTitleAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoTitleAtom, (fp, atomSize, atomType), pAssetInfoTitleAtom);

                if (!pAssetInfoTitleAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoTitleAtom, pAssetInfoTitleAtom);
                    pAssetInfoTitleAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoTitleAtomArray).push_back(pAssetInfoTitleAtom);
                    pAssetInfoTitleAtom->setParent(this);
                    count -= pAssetInfoTitleAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_DESCP_ATOM)
            {
                AssetInfoDescAtom *pAssetInfoDescAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoDescAtom, (fp, atomSize, atomType), pAssetInfoDescAtom);

                if (!pAssetInfoDescAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoDescAtom, pAssetInfoDescAtom);
                    pAssetInfoDescAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoDescAtomArray).push_back(pAssetInfoDescAtom);
                    pAssetInfoDescAtom->setParent(this);
                    count -= pAssetInfoDescAtom->getSize();
                }

            }
            else if (atomType == ASSET_INFO_PERF_ATOM)
            {
                AssetInfoPerformerAtom * pAssetInfoPerformerAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoPerformerAtom, (fp, atomSize, atomType), pAssetInfoPerformerAtom);

                if (!pAssetInfoPerformerAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoPerformerAtom, pAssetInfoPerformerAtom);
                    pAssetInfoPerformerAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoPerformerAtomArray).push_back(pAssetInfoPerformerAtom);
                    pAssetInfoPerformerAtom->setParent(this);
                    count -= pAssetInfoPerformerAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_AUTHOR_ATOM)
            {
                AssetInfoAuthorAtom * pAssetInfoAuthorAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoAuthorAtom, (fp, atomSize, atomType), pAssetInfoAuthorAtom);
                if (!pAssetInfoAuthorAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoAuthorAtom, pAssetInfoAuthorAtom);
                    pAssetInfoAuthorAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoAuthorAtomArray).push_back(pAssetInfoAuthorAtom);
                    pAssetInfoAuthorAtom->setParent(this);
                    count -= pAssetInfoAuthorAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_GENRE_ATOM)
            {
                AssetInfoGenreAtom * pAssetInfoGenreAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoGenreAtom, (fp, atomSize, atomType), pAssetInfoGenreAtom);

                if (!pAssetInfoGenreAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoGenreAtom, pAssetInfoGenreAtom);
                    pAssetInfoGenreAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoGenreAtomArray).push_back(pAssetInfoGenreAtom);
                    pAssetInfoGenreAtom->setParent(this);
                    count -= pAssetInfoGenreAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_RATING_ATOM)
            {
                AssetInfoRatingAtom * pAssetInfoRatingAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoRatingAtom, (fp, atomSize, atomType), pAssetInfoRatingAtom);

                if (!pAssetInfoRatingAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoRatingAtom, pAssetInfoRatingAtom);
                    pAssetInfoRatingAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoRatingAtomArray).push_back(pAssetInfoRatingAtom);
                    pAssetInfoRatingAtom->setParent(this);
                    count -= pAssetInfoRatingAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_CLSF_ATOM)
            {
                AssetInfoClassificationAtom * pAssetInfoClassificationAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoClassificationAtom, (fp, atomSize, atomType), pAssetInfoClassificationAtom);

                if (!pAssetInfoClassificationAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoClassificationAtom, pAssetInfoClassificationAtom);
                    pAssetInfoClassificationAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoClassificationAtomArray).push_back(pAssetInfoClassificationAtom);
                    pAssetInfoClassificationAtom->setParent(this);
                    count -= pAssetInfoClassificationAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_KEYWORD_ATOM)
            {
                AssetInfoKeyWordAtom * pAssetInfoKeyWordAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoKeyWordAtom, (fp, atomSize, atomType), pAssetInfoKeyWordAtom);

                if (!pAssetInfoKeyWordAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoKeyWordAtom, pAssetInfoKeyWordAtom);
                    pAssetInfoKeyWordAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoKeyWordAtomArray).push_back(pAssetInfoKeyWordAtom);
                    pAssetInfoKeyWordAtom->setParent(this);
                    count -= pAssetInfoKeyWordAtom->getSize();
                }
            }

            else if (atomType == ASSET_INFO_LOCATION_ATOM)
            {
                AssetInfoLocationAtom * pAssetInfoLocationAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoLocationAtom, (fp, atomSize, atomType), pAssetInfoLocationAtom);

                if (!pAssetInfoLocationAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoLocationAtom, pAssetInfoLocationAtom);
                    pAssetInfoLocationAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoLocationAtomArray).push_back(pAssetInfoLocationAtom);
                    pAssetInfoLocationAtom->setParent(this);
                    count -= pAssetInfoLocationAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_ALBUM_ATOM)
            {
                AssetInfoAlbumAtom * pAssetInfoAlbumAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoAlbumAtom, (fp, atomSize, atomType), pAssetInfoAlbumAtom);

                if (!pAssetInfoAlbumAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoAlbumAtom, pAssetInfoAlbumAtom);
                    pAssetInfoAlbumAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoAlbumAtomArray).push_back(pAssetInfoAlbumAtom);
                    pAssetInfoAlbumAtom->setParent(this);
                    count -= pAssetInfoAlbumAtom->getSize();
                }
            }
            else if (atomType == ASSET_INFO_YRRC_ATOM)
            {
                AssetInfoRecordingYearAtom * pAssetInfoRecordingYearAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, AssetInfoRecordingYearAtom, (fp, atomSize, atomType), pAssetInfoRecordingYearAtom);

                if (!pAssetInfoRecordingYearAtom->MP4Success())
                {
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                    PV_MP4_FF_DELETE(NULL, AssetInfoRecordingYearAtom, pAssetInfoRecordingYearAtom);
                    pAssetInfoRecordingYearAtom = NULL;
                    count -= atomSize;
                }
                else
                {
                    (*_pAssetInfoRecordingYearArray).push_back(pAssetInfoRecordingYearAtom);
                    pAssetInfoRecordingYearAtom->setParent(this);
                    count -= pAssetInfoRecordingYearAtom->getSize();
                }
            }

            else if (atomType == META_DATA_ATOM)
            {
                if (_pMetaDataAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MetaDataAtom, (fp, atomSize, atomType), _pMetaDataAtom);

                    if (!_pMetaDataAtom->MP4Success())
                    {
                        AtomUtils::seekFromStart(fp, currPtr);
                        AtomUtils::seekFromCurrPos(fp, atomSize);
                        PV_MP4_FF_DELETE(NULL, MetaDataAtom, _pMetaDataAtom);
                        _pMetaDataAtom = NULL;
                        count -= atomSize;
                    }
                    else
                        count -= _pMetaDataAtom->getSize();
                }
                else
                {
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }

            else
            {
                // skip unknown atom
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    //lost sync
                    AtomUtils::seekFromStart(fp, currPtr);
                    AtomUtils::seekFromCurrPos(fp, count);
                    count = 0;
                    return;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
        }
    }
    else
    {
        _mp4ErrorCode = READ_USER_DATA_ATOM_FAILED;
    }

}


// Destructor
UserDataAtom::~UserDataAtom()
{
    if (_pPVUserDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVUserDataAtom, _pPVUserDataAtom);
    }

    if (_pPVContentTypeAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVContentTypeAtom, _pPVContentTypeAtom);
    }


    if (_pMetaDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MetaDataAtom, _pMetaDataAtom);
    }


    if (_pCopyRightAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pCopyRightAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, CopyRightAtom, (*_pCopyRightAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, copyRightAtomVecType, Oscl_Vector, _pCopyRightAtomArray);
    }
    if (_pAssetInfoTitleAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoTitleAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoTitleAtom, (*_pAssetInfoTitleAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoTitleAtomVecType, Oscl_Vector, _pAssetInfoTitleAtomArray);
    }
    if (_pAssetInfoDescAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoDescAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoDescAtom, (*_pAssetInfoDescAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoDescAtomVecType, Oscl_Vector, _pAssetInfoDescAtomArray);
    }
    if (_pAssetInfoPerformerAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoPerformerAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoPerformerAtom, (*_pAssetInfoPerformerAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoPerformerAtomVecType, Oscl_Vector, _pAssetInfoPerformerAtomArray);
    }
    if (_pAssetInfoAuthorAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoAuthorAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoAuthorAtom, (*_pAssetInfoAuthorAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoAuthorAtomVecType, Oscl_Vector, _pAssetInfoAuthorAtomArray);
    }
    if (_pAssetInfoGenreAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoGenreAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoGenreAtom, (*_pAssetInfoGenreAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoGenreAtomVecType, Oscl_Vector, _pAssetInfoGenreAtomArray);
    }
    if (_pAssetInfoRatingAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoRatingAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoRatingAtom, (*_pAssetInfoRatingAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoRatingAtomVecType, Oscl_Vector, _pAssetInfoRatingAtomArray);
    }
    if (_pAssetInfoClassificationAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoClassificationAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoClassificationAtom, (*_pAssetInfoClassificationAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoClassificationAtomVecType, Oscl_Vector, _pAssetInfoClassificationAtomArray);
    }
    if (_pAssetInfoKeyWordAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoKeyWordAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoKeyWordAtom, (*_pAssetInfoKeyWordAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoKeyWordAtomVecType, Oscl_Vector, _pAssetInfoKeyWordAtomArray);
    }
    if (_pAssetInfoLocationAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoLocationAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoLocationAtom, (*_pAssetInfoLocationAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoLocationAtomVecType, Oscl_Vector, _pAssetInfoLocationAtomArray);
    }

    if (_pAssetInfoAlbumAtomArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoAlbumAtomArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoAlbumAtom, (*_pAssetInfoAlbumAtomArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoAlbumAtomVecType, Oscl_Vector, _pAssetInfoAlbumAtomArray);
    }

    if (_pAssetInfoRecordingYearArray != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pAssetInfoRecordingYearArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, AssetInfoRecordingYearAtom, (*_pAssetInfoRecordingYearArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, assetInfoRecordingYearAtomVecType, Oscl_Vector, _pAssetInfoRecordingYearArray);
    }



}


CopyRightAtom	*
UserDataAtom::getCopyRightAtomAt(int32 index)
{
    if ((_pCopyRightAtomArray->size() == 0) ||
            ((uint32)index >= (_pCopyRightAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pCopyRightAtomArray)[index];
    }
}

AssetInfoTitleAtom *
UserDataAtom::getAssetInfoTitleAtomAt(int32 index)
{
    if ((_pAssetInfoTitleAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoTitleAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoTitleAtomArray)[index];
    }
}

AssetInfoDescAtom *
UserDataAtom::getAssetInfoDescAtomAt(int32 index)
{
    if ((_pAssetInfoDescAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoDescAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoDescAtomArray)[index];
    }
}

AssetInfoPerformerAtom *
UserDataAtom::getAssetInfoPerformerAtomAt(int32 index)
{
    if ((_pAssetInfoPerformerAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoPerformerAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoPerformerAtomArray)[index];
    }
}

AssetInfoAuthorAtom *
UserDataAtom::getAssetInfoAuthorAtomAt(int32 index)
{
    if ((_pAssetInfoAuthorAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoAuthorAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoAuthorAtomArray)[index];
    }
}

AssetInfoGenreAtom *
UserDataAtom::getAssetInfoGenreAtomAt(int32 index)
{
    if ((_pAssetInfoGenreAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoGenreAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoGenreAtomArray)[index];
    }
}

AssetInfoRatingAtom *
UserDataAtom::getAssetInfoRatingAtomAt(int32 index)
{
    if ((_pAssetInfoRatingAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoRatingAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoRatingAtomArray)[index];
    }
}

AssetInfoClassificationAtom *
UserDataAtom::getAssetInfoClassificationAtomAt(int32 index)
{
    if ((_pAssetInfoClassificationAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoClassificationAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoClassificationAtomArray)[index];
    }
}

AssetInfoKeyWordAtom *
UserDataAtom::getAssetInfoKeyWordAtomAt(int32 index)
{
    if ((_pAssetInfoKeyWordAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoKeyWordAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoKeyWordAtomArray)[index];
    }
}

AssetInfoLocationAtom *
UserDataAtom::getAssetInfoLocationAtomAt(int32 index)
{
    if ((_pAssetInfoLocationAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoLocationAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoLocationAtomArray)[index];
    }
}

AssetInfoAlbumAtom *
UserDataAtom::getAssetInfoAlbumAtomAt(int32 index)
{
    if ((_pAssetInfoAlbumAtomArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoAlbumAtomArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoAlbumAtomArray)[index];
    }
}

AssetInfoRecordingYearAtom *
UserDataAtom::getAssetInfoRecordingYearAtomAt(int32 index)
{
    if ((_pAssetInfoRecordingYearArray->size() == 0) ||
            ((uint32)index >= (_pAssetInfoRecordingYearArray->size())))
    {
        return NULL;
    }
    else
    {
        return (*_pAssetInfoRecordingYearArray)[index];
    }
}

