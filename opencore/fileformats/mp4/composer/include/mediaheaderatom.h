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
/*
    This PVA_FF_MediaHeaderAtom Class contains all the objects that declare information
    about the media data within the stream.
*/


#ifndef __MediaHeaderAtom_H__
#define __MediaHeaderAtom_H__

#include "fullatom.h"

class PVA_FF_MediaHeaderAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_MediaHeaderAtom(); // Constructor
        virtual ~PVA_FF_MediaHeaderAtom();

        // Creation Time gets and sets
        void setCreationTime(uint32 ct)
        {
            _creationTime = ct;
        }
        uint32 getCreationTime() const
        {
            return _creationTime;
        }

        // Modification Time gets and sets
        void setModificationTime(uint32 mt)
        {
            _modificationTime = mt;
        }
        uint32 getModificationTime() const
        {
            return _modificationTime;
        }

        // Time Scale gets and sets
        void setTimeScale(uint32 ts)
        {
            _timeScale = ts;
        }
        uint32 getTimeScale() const
        {
            return _timeScale;
        }

        // Duration gets and sets
        void setDuration(uint32 d)
        {
            _duration = d;
        }
        uint32 getDuration() const
        {
            return _duration;
        }

        // Language gets and sets (actually only 15 bits of the uint16)
        void setLanguage(uint16 l)
        {
            _language = l;
        }
        uint16 getLanguage() const
        {
            return _language;
        }

        // Allows atom to maintain its _duration (in media time scale - i.e. samples)
        void addSample(uint32 ts);
        void updateLastTSEntry(uint32 ts);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
    private:
        void init();
        virtual void recomputeSize();

        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint32 _timeScale; // 4 (32bits)
        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint16 _language; // Actually 3 5-bit objects representing the packed ISO-639-2/T language code
        // Rendered as 15 bits with the leading pad bit from above.

        // Static reserved constants
        uint16 _reserved; //= 0 // (16bits)

        uint32 _prevTS;
        uint32 _deltaTS;
};



#endif

