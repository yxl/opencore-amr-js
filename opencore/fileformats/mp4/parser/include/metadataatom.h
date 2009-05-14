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
#ifndef METADATAATOM_H_INCLUDED
#define METADATAATOM_H_INCLUDED

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#include "handleratom.h"
#include "atom.h"
#include "itunesilstatom.h"
#include"atomdefs.h"

class MetaDataAtom: public Atom
{
    public:
        MetaDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        ~MetaDataAtom();



        OSCL_wHeapString<OsclMemAllocator> getITunesTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTrackSubTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getTrackSubTitle();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbumArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getAlbumArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbum() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getAlbum();
            else
                return temp;
        }

        // Gnre ** Starts **
        uint16 getITunesGnreID() const
        {
            if (_pITunesILSTAtom)
            {
                if (_pITunesILSTAtom->getGnreVersion() == INTEGER_GENRE)
                {
                    return _pITunesILSTAtom->getGnreID();
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
            if (_pITunesILSTAtom)
            {
                if (_pITunesILSTAtom->getGnreVersion() == STRING_GENRE)
                {
                    return _pITunesILSTAtom->getGnreString();
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
            if (_pITunesILSTAtom)
            {
                return _pITunesILSTAtom->getGnreVersion();
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
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getYear();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesTool() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
            {
                return _pITunesILSTAtom->getTool();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesEncodedBy() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
            {
                return _pITunesILSTAtom->getEncodedBy();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesWriter() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getWriter();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesGroup() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getGroup();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesComment() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getComment();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getITunesCopyright();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesDescription() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getITunesDescription();
            else
                return temp;
        }


        uint16 getITunesThisTrackNo() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getThisTrackNo();
            else
                return 0;
        }

        uint16 getITunesTotalTracks() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getTotalTracks();
            else
                return 0;
        }

        bool IsITunesCompilationPart() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->IsCompilationPart();
            else
                return false;
        }

        bool IsITunesContentRating() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->IsContentRating();
            else
                return false;
        }

        uint16 getITunesBeatsPerMinute() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getBeatsPerMinute();
            else
                return 0;
        }


        PvmfApicStruct* getITunesImageData() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getImageData();
            else
                return NULL;
        }

        uint16 getITunesThisDiskNo() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getThisDiskNo();
            else
                return 0;
        }

        uint16 getITunesTotalDisks() const
        {
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getTotalDisks();
            else
                return 0;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesNormalizationData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getNormalizationData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDIdentifierData(uint8 index) const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getCDIdentifierData(index);
            else
                return temp;
        }


        uint8 getITunesTotalCDIdentifierData() const
        {

            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getTotalCDIdentifierData();
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesCDTrackNumberData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getCDTrackNumberData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDDB1Data() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getCDDB1Data();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesLyrics() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pITunesILSTAtom)
                return _pITunesILSTAtom->getLyrics();
            else
                return temp;
        }


    private:

        HandlerAtom	*_pHdlrAtom;
        // User ilst Data
        ITunesILSTAtom* _pITunesILSTAtom;
        PVLogger *iLogger;
};


#endif // METADATAATOM_H_INCLUDED












































