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
#ifndef ITUNESILSTMETADATAATOM_H_INCLUDED
#define ITUNESILSTMETADATAATOM_H_INCLUDED

#include "atom.h"
#include "pvmi_kvp.h"


class ITunesILstMetaDataAtom
{
    public:
        ITunesILstMetaDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~ITunesILstMetaDataAtom();

        OSCL_wHeapString<OsclMemAllocator> getName() const
        {
            return _name;
        }

        OSCL_wHeapString<OsclMemAllocator> getArtist() const
        {
            return _artist;
        }

        OSCL_wHeapString<OsclMemAllocator> getAlbum() const
        {
            return _album;
        }

        // Gnre ** Starts **
        uint16 getGnreID() const
        {
            return _gnreID;
        }
        OSCL_wHeapString<OsclMemAllocator> getGnreString() const
        {
            return _gnreString;
        }

        //This function will tell the type of Genre-- TRUE- If it is Integer, FALSE- if it is String
        bool getGnreVesion() const
        {
            return _gnreVersion;
        }
        // Gnre ** Ends **


        // Returns the 4-byte YEAR when the song was recorded
        OSCL_wHeapString<OsclMemAllocator> getDay() const
        {
            return _day;
        }

        OSCL_wHeapString<OsclMemAllocator> getTool() const
        {
            return _tool;
        }

        OSCL_wHeapString<OsclMemAllocator> getWriter()
        {
            return _writer;
        }

        OSCL_wHeapString<OsclMemAllocator> getGroup()
        {
            return _group;
        }

        OSCL_wHeapString<OsclMemAllocator> getComment()
        {
            return _comment;
        }

        uint16 getThisTrackNo()
        {
            return _thisTrackNo;
        }
        uint16 getTotalTracks()
        {
            return _totalTracks;
        }

        bool IsCompilationPart()
        {
            return _compilationPart;
        }

        uint16 getBeatsPerMinute()
        {
            return _beatsPerMin;
        }

        PvmfApicStruct getPNGImageData()
        {
            return _PNGimageData;
        }

        uint16 getThisDiskNo()
        {
            return _thisDiskNo;
        }

        uint16 getTotalDisks()
        {
            return _totalDisks;
        }

        OSCL_wHeapString<OsclMemAllocator> getLyrics()
        {
            return _lyrics;
        }

        OSCL_wHeapString<OsclMemAllocator> getNormalizationData()
        {
            return _normlizationData;
        }

        OSCL_wHeapString<OsclMemAllocator> getCDIdentifier()
        {
            return _cdIdentifier;
        }


    private:

        // Title of Song
        OSCL_wHeapString<OsclMemAllocator> _name;

        // Artist/Performer of the Song
        OSCL_wHeapString<OsclMemAllocator> _artist;

        // Album of Song
        OSCL_wHeapString<OsclMemAllocator> _album;

        // Genre
        OSCL_wHeapString<OsclMemAllocator> _gnreString; // Customized String of Genre
        uint16 _gnreID; // Genre ID
        bool	_gnreVersion; // Whether gnre is String or ID Number

        // 4 byte String representing Year, when song was recorded.
        OSCL_wHeapString<OsclMemAllocator> _day;

        // Tool/Encoder used for creation of this file.
        OSCL_wHeapString<OsclMemAllocator> _tool;

        // Writer of the Song
        OSCL_wHeapString<OsclMemAllocator> _writer;

        // Group data.
        OSCL_wHeapString<OsclMemAllocator> _group;

        // Comment
        OSCL_wHeapString<OsclMemAllocator> _comment;

        // Track Number
        uint16 _thisTrackNo;
        uint16 _totalTracks;

        // Whether this file is the Part of Compilation or not.
        bool _compilationPart;

        // Number of Beats per Minute
        uint16 _beatsPerMin;

        // Album Art Data- PNG Image data
        PvmfApicStruct _PNGimageData;

        // Disk Number
        uint16 _thisDiskNo;
        uint16 _totalDisks;

        // Lyrics of the Song
        OSCL_wHeapString<OsclMemAllocator> _lyrics;

        // Free Form Data/ Normalization Data.
        OSCL_wHeapString<OsclMemAllocator> _normlizationData;

        //CD Identifier
        OSCL_wHeapString<OsclMemAllocator> _cdIdentifier;

};


#endif //ITUNESILSTMETADATAATOM_H_INCLUDED

