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
/*                        MPEG-4 MediaHeaderAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaHeaderAtom Class contains all the objects that declare information
    about the media data within the stream.
*/


#ifndef MEDIAHEADERATOM_H_INCLUDED
#define MEDIAHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class MediaHeaderAtom : public FullAtom
{

    public:
        MediaHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Stream-in ctor
        virtual ~MediaHeaderAtom();

        // Creation Time gets and sets
        uint64 getCreationTime() const
        {
            if (getVersion() == 1)
                return _creationTime64;
            else
                return _creationTime;
        }

        // Modification Time gets and sets
        uint64 getModificationTime() const
        {
            if (getVersion() == 1)
                return _modificationTime64;
            else
                return _modificationTime;
        }

        // Time Scale gets and sets
        uint32 getTimeScale() const
        {
            return _timeScale;
        }

        // Duration gets and sets
        uint64 getDuration() const
        {
            if (getVersion() == 1)
                return _duration64;
            else
                return _duration;
        }

        // Language gets and sets (actually only 15 bits of the WORD)
        uint16 getLanguage() const
        {
            return _language;
        }


    private:

        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _creationTime64;

        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _modificationTime64;

        uint32 _timeScale; // 4 (32bits)
        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _duration64;

        uint16 _language; // Actually 3 5-bit objects representing the packed ISO-639-2/T language code
        // Rendered as 15 bits with the leading pad bit from above.

        // Static reserved constants
        uint16 _reserved; //= 0 // (16bits)
};



#endif // MEDIAHEADERATOM_H_INCLUDED


