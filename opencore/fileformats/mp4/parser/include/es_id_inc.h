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
/*                          MPEG-4 ES_ID_Inc Class                               */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#ifndef ES_ID_INC_H_INCLUDED
#define ES_ID_INC_H_INCLUDED

#ifndef BASEDESCRIPTOR_H_INCLUDED
#include "basedescriptor.h"
#endif

class ES_ID_Inc : public BaseDescriptor
{

    public:
        ES_ID_Inc(MP4_FF_FILE *fp); // Stream-in constructor
        virtual ~ES_ID_Inc(); // Destructor

        // Member gets and sets
        uint32 getTrackID() const
        {
            return _trackID;
        }


    private:
        uint32  _trackID; // (32)

};



#endif // ES_ID_INC_H_INCLUDED

