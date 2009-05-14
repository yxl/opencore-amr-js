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


#ifndef USERDATAATOM_H_INCLUDED
#define USERDATAATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#ifndef PVUSERDATAATOM_H_INCLUDED
#include "pvuserdataatom.h"
#endif

#ifndef COPYRIGHTATOM_H_INCLUDED
#include "copyrightatom.h"
#endif

#ifndef ASSETINFOATOMS_H_INCLUDED
#include "assetinfoatoms.h"
#endif


#ifndef ITUNESILSTATOM_H_INCLUDED
#include "itunesilstatom.h"
#endif


#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif



#include "metadataatom.h"


#include "atomdefs.h"

class UserDataAtom : public Atom
{

    public:
        UserDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Constructor
        virtual ~UserDataAtom();

        uint8 randomAccessDenied();
        Atom *getAtomOfType(uint32 fourCC);

        int32 getNumAssetInfoTitleAtoms()
        {
            if (_pAssetInfoTitleAtomArray != NULL)
            {
                return (_pAssetInfoTitleAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoDescAtoms()
        {
            if (_pAssetInfoDescAtomArray != NULL)
            {
                return (_pAssetInfoDescAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumCopyRightAtoms()
        {
            if (_pCopyRightAtomArray != NULL)
            {
                return (_pCopyRightAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoPerformerAtoms()
        {
            if (_pAssetInfoPerformerAtomArray != NULL)
            {
                return (_pAssetInfoPerformerAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoAuthorAtoms()
        {
            if (_pAssetInfoAuthorAtomArray != NULL)
            {
                return (_pAssetInfoAuthorAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoGenreAtoms()
        {
            if (_pAssetInfoGenreAtomArray != NULL)
            {
                return (_pAssetInfoGenreAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoRatingAtoms()
        {
            if (_pAssetInfoRatingAtomArray != NULL)
            {
                return (_pAssetInfoRatingAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoClassificationAtoms()
        {
            if (_pAssetInfoClassificationAtomArray != NULL)
            {
                return (_pAssetInfoClassificationAtomArray->size());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoKeyWordAtoms()
        {
            if (_pAssetInfoKeyWordAtomArray != NULL)
            {
                return (_pAssetInfoKeyWordAtomArray->size());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoLocationAtoms()
        {
            if (_pAssetInfoLocationAtomArray != NULL)
            {
                return (_pAssetInfoLocationAtomArray->size());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoAlbumAtoms()
        {
            if (_pAssetInfoAlbumAtomArray != NULL)
            {
                return (_pAssetInfoAlbumAtomArray->size());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoRecordingYearAtoms()
        {
            if (_pAssetInfoRecordingYearArray != NULL)
            {
                return (_pAssetInfoRecordingYearArray->size());
            }
            else
            {
                return 0;
            }
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTrackSubTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesTrackSubTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbumArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesAlbumArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbum() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesAlbum();
            else
                return temp;
        }

        // Gnre ** Starts **
        uint16 getITunesGnreID() const
        {
            if (_pMetaDataAtom)
            {
                if (_pMetaDataAtom->getITunesGnreVersion() == INTEGER_GENRE)
                {
                    return _pMetaDataAtom->getITunesGnreID();
                }
                else
                    return 0;
            }
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesGnreString() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
            {
                if (_pMetaDataAtom->getITunesGnreVersion() == STRING_GENRE)
                {
                    return _pMetaDataAtom->getITunesGnreString();
                }
                else
                    return temp;
            }
            else
                return temp;
        }

        //This function will tell the type of Genre--
        GnreVersion getITunesGnreVersion() const
        {
            if (_pMetaDataAtom)
            {
                return _pMetaDataAtom->getITunesGnreVersion();
            }
            else
                // By-default return INTEGER_GENRE
                return INTEGER_GENRE;
        }
        // Gnre ** Ends **


        // Returns the 4-byte YEAR when the song was recorded
        OSCL_wHeapString<OsclMemAllocator> getITunesYear() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesYear();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesTool() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
            {
                return _pMetaDataAtom->getITunesTool();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesEncodedBy() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
            {
                return _pMetaDataAtom->getITunesEncodedBy();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesWriter() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesWriter();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesGroup() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesGroup();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesComment() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesComment();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesCopyright();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesDescription() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesDescription();
            else
                return temp;
        }

        uint16 getITunesThisTrackNo() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesThisTrackNo();
            else
                return 0;
        }

        uint16 getITunesTotalTracks() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesTotalTracks();
            else
                return 0;
        }

        bool IsITunesCompilationPart() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->IsITunesCompilationPart();
            else
                return false;
        }

        bool IsITunesContentRating() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->IsITunesContentRating();
            else
                return false;
        }

        uint16 getITunesBeatsPerMinute() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesBeatsPerMinute();
            else
                return 0;
        }

        PvmfApicStruct* getITunesImageData() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesImageData();
            else
                return NULL;
        }

        uint16 getITunesThisDiskNo() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesThisDiskNo();
            else
                return 0;
        }

        uint16 getITunesTotalDisks() const
        {
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesTotalDisks();
            else
                return 0;
        }



        OSCL_wHeapString<OsclMemAllocator> getITunesNormalizationData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesNormalizationData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDIdentifierData(uint8 index) const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesCDIdentifierData(index);
            else
                return temp;
        }

        uint8 getITunesTotalCDIdentifierData() const
        {

            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesTotalCDIdentifierData();
            else
                return 0;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDTrackNumberData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesCDTrackNumberData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDDB1Data() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesCDDB1Data();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesLyrics() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pMetaDataAtom)
                return _pMetaDataAtom->getITunesLyrics();
            else
                return temp;
        }



        AssetInfoTitleAtom          *getAssetInfoTitleAtomAt(int32 index);
        AssetInfoDescAtom           *getAssetInfoDescAtomAt(int32 index);
        CopyRightAtom	            *getCopyRightAtomAt(int32 index);
        AssetInfoPerformerAtom      *getAssetInfoPerformerAtomAt(int32 index);
        AssetInfoAuthorAtom         *getAssetInfoAuthorAtomAt(int32 index);
        AssetInfoGenreAtom          *getAssetInfoGenreAtomAt(int32 index);
        AssetInfoRatingAtom         *getAssetInfoRatingAtomAt(int32 index);
        AssetInfoClassificationAtom *getAssetInfoClassificationAtomAt(int32 index);
        AssetInfoKeyWordAtom        *getAssetInfoKeyWordAtomAt(int32 index);
        AssetInfoLocationAtom		*getAssetInfoLocationAtomAt(int32 index);
        AssetInfoAlbumAtom			*getAssetInfoAlbumAtomAt(int32 index);
        AssetInfoRecordingYearAtom	*getAssetInfoRecordingYearAtomAt(int32 index);

    private:
        PVUserDataAtom* _pPVUserDataAtom;


        MetaDataAtom* _pMetaDataAtom;


        PVContentTypeAtom* _pPVContentTypeAtom;


        Oscl_Vector<AssetInfoTitleAtom*, OsclMemAllocator>		     *_pAssetInfoTitleAtomArray;
        Oscl_Vector<AssetInfoDescAtom*, OsclMemAllocator>		     *_pAssetInfoDescAtomArray;
        Oscl_Vector<CopyRightAtom*, OsclMemAllocator>			     *_pCopyRightAtomArray;
        Oscl_Vector<AssetInfoPerformerAtom*, OsclMemAllocator>        *_pAssetInfoPerformerAtomArray;
        Oscl_Vector<AssetInfoAuthorAtom*, OsclMemAllocator>	         *_pAssetInfoAuthorAtomArray;
        Oscl_Vector<AssetInfoGenreAtom*, OsclMemAllocator>		     *_pAssetInfoGenreAtomArray;
        Oscl_Vector<AssetInfoRatingAtom*, OsclMemAllocator>   		 *_pAssetInfoRatingAtomArray;
        Oscl_Vector<AssetInfoClassificationAtom*, OsclMemAllocator>   *_pAssetInfoClassificationAtomArray;
        Oscl_Vector<AssetInfoKeyWordAtom*, OsclMemAllocator>		     *_pAssetInfoKeyWordAtomArray;
        Oscl_Vector<AssetInfoLocationAtom*, OsclMemAllocator>		 *_pAssetInfoLocationAtomArray;
        Oscl_Vector<AssetInfoAlbumAtom*, OsclMemAllocator>			 *_pAssetInfoAlbumAtomArray;
        Oscl_Vector<AssetInfoRecordingYearAtom*, OsclMemAllocator>		 *_pAssetInfoRecordingYearArray;
};

#endif  // USERDATAATOM_H_INCLUDED

