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
    This PVA_FF_ESDescriptor Class contains information on the Elementary Stream
*/


#ifndef __ES_ID_Inc_H__
#define __ES_ID_Inc_H__

#include "basedescriptor.h"

class PVA_FF_ES_ID_Inc : public PVA_FF_BaseDescriptor
{

    public:
        PVA_FF_ES_ID_Inc(uint32 trackid); // Alternate constructor
        virtual ~PVA_FF_ES_ID_Inc(); // Destructor

        // Member gets and sets
        uint16 getTrackID() const
        {
            return (uint16)(_trackID);
        }
        void setTrackID(uint32 trackid)
        {
            _trackID = trackid;
        }

        // Recompute the size of the class as is rendered to a file string
        virtual void recomputeSize();

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        uint32 _trackID; // (32)

};



#endif

