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
/*                      MPEG-4 HintMediaHeaderAtom Class                         */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This HintMediaHeaderAtom Class contains general information,
    independent of protocol, for the hint tracks within the stream.
*/


#ifndef HINTMEDIAHEADERATOM_H_INCLUDED
#define HINTMEDIAHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef MEDIAINFORMATIONHEADERATOM_H_INCLUDED
#include "mediainformationheaderatom.h"
#endif

class HintMediaHeaderAtom : public MediaInformationHeaderAtom
{

    public:
        HintMediaHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~HintMediaHeaderAtom();

        virtual uint32 getMediaInformationHeaderType() const;

        // Member gets
        uint16 getMaxPDUSize() const
        {
            return _maxPDUSize;
        }
        uint16 getAvgPDUSize() const
        {
            return _avgPDUSize;
        }
        uint32 getMaxBitrate() const
        {
            return _maxBitrate;
        }
        uint32 getAvgBitrate() const
        {
            return _avgBitrate;
        }
        uint32 getSlidingAvgBitrate() const
        {
            return _slidingAvgBitrate;
        }

    private:
        uint16 _maxPDUSize;
        uint16 _avgPDUSize;
        uint32 _maxBitrate;
        uint32 _avgBitrate;
        uint32 _slidingAvgBitrate;
};

#endif // HINTMEDIAHEADERATOM_H_INCLUDED


