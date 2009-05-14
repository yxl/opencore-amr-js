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
    This DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#define DECODERSPECIFICINFO_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef BASEDESCRIPTOR_H_INCLUDED
#include "basedescriptor.h"
#endif

class DecoderSpecificInfo : BaseDescriptor
{

    public:
        DecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack = false,
                            bool read_struct = true); // Stream-in Constructor
        virtual ~DecoderSpecificInfo(); // Destructor

        // Member get methods
        uint32 getInfoSize() const
        {
            return _infoSize;    // Returns the size of the info data
        }
        uint8 *getInfo() const
        {
            return _pinfo;    // Returns the byte pointer to info
        }

        uint32 getSizeOfClass() const
        {
            return _infoSize + 4;
        }

        void setInfoSize(uint32 size)
        {
            _infoSize = size;
        }

        void setInfo(uint8* info)
        {
            _pinfo = info;
        }

    private:
        uint8 *_pinfo;
        int32 _infoSize;
};

#endif // DECODERSPECIFICINFO_H_INCLUDED

