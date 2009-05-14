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
    This PVA_FF_DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#ifndef __DecoderSpecificInfo_H__
#define __DecoderSpecificInfo_H__

#include "basedescriptor.h"
#include "textsampledescinfo.h"

class PVA_FF_DecoderSpecificInfo : public PVA_FF_BaseDescriptor
{

    public:
        PVA_FF_DecoderSpecificInfo(); // Default constructor

        PVA_FF_DecoderSpecificInfo(uint8 *pdata, uint32 size); // Constructor
        PVA_FF_DecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pdata, uint32 size); //Constructor for timed text case

        virtual ~PVA_FF_DecoderSpecificInfo(); // Destructor

        void addInfo(uint8 *info, uint32 size);

        // Member get methods
        uint32 getInfoSize() const
        {
            return _infoSize;    // Returns the size of the info data
        }
        uint8  *getInfo() const
        {
            return _pinfo;    // Returns the byte pointer to info
        }

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();

    private:
        uint32 _infoSize;
        uint8  *_pinfo;
};

#endif

