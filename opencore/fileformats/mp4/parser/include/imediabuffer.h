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
/*                           MPEG-4 Util: IMediaBuffer                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This IMediaBuffer Class is the interface that consumers of MediaBuffers will
	use.  This includes PVPlayer and PVServer.  It is mainly used when retrieving
	Mpeg-4 data from file and returning a data buffer to either PVPlayer or
	PVServer.
*/


#ifndef IMEDIABUFFER_H_INCLUDED
#define IMEDIABUFFER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

class IMediaBuffer
{

    public:
        virtual ~IMediaBuffer() {}; // So destructor of MediaBuffer gets called

        // Member get methods

        // Returns the size of the data in the buffer
        virtual uint32 getSize() const = 0;

        // Returns a pointer to the data in the buffer
        virtual uint8 *getDataPtr() = 0;
        // Returns a read-only pointer to the data in the buffer - USE THIS METHOD
        virtual uint8 *getReadOnlyDataPtr() const = 0;

        // This method MUST be called when you are done with the buffer.  This allows
        // the underlying buffer manager to handle memory management by reusing the
        // buffer when you are done.
        virtual void freeBuf() = 0;

};

#endif  // IMEDIABUFFER_H_INCLUDED

