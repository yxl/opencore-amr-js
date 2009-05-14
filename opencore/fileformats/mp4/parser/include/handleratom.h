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
/*                          MPEG-4 HandlerAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This HandlerAtom Class declares the nature of the media in the stream.
*/


#ifndef HANDLERATOM_H_INCLUDED
#define HANDLERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class HandlerAtom : public FullAtom
{

    public:
        HandlerAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Stream-in ctor
        virtual ~HandlerAtom();

        // Private member gets
        uint32 getHandlerType() const
        {
            return _handlerType;
        }
        OSCL_wString& getName()
        {
            return _name;
        }

    private:

        uint32 reserved1;
        uint32 _handlerType;
        OSCL_wHeapString<OsclMemAllocator> _name;
};


#endif // HANDLERATOM_H_INCLUDED


