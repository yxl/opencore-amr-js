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
    This PVA_FF_HintMediaHeaderAtom Class contains general information,
    independent of protocol, for the hint tracks within the stream.
*/


#ifndef __HintMediaHeaderAtom_H__
#define __HintMediaHeaderAtom_H__

#include "mediainformationheaderatom.h"

class PVA_FF_HintMediaHeaderAtom : public PVA_FF_MediaInformationHeaderAtom
{

    public:
        PVA_FF_HintMediaHeaderAtom(); // Constructor
        virtual ~PVA_FF_HintMediaHeaderAtom();

        virtual uint32 getMediaInformationHeaderType() const;

        // Member gets and sets
        void setMaxPDUSize(uint16 max)
        {
            _maxPDUSize = max;
        }
        uint16 getMaxPDUSize() const
        {
            return _maxPDUSize;
        }

        void setAvgPDUSize(uint16 avg)
        {
            _avgPDUSize = avg;
        }
        uint16 getAvgPDUSize() const
        {
            return _avgPDUSize;
        }

        void setMaxBitrate(int32 max)
        {
            _maxBitrate = max;
        }
        int32 getMaxBitrate() const
        {
            return _maxBitrate;
        }

        void setAvgBitrate(int32 avg)
        {
            _avgBitrate = avg;
        }
        int32 getAvgBitrate() const
        {
            return _avgBitrate;
        }

        void setSlidingAvgBitrate(int32 avg)
        {
            _slidingAvgBitrate = avg;
        }
        int32 getSlidingAvgBitrate() const
        {
            return _slidingAvgBitrate;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        void init();
        virtual void recomputeSize();

        uint16 _maxPDUSize;
        uint16 _avgPDUSize;
        int32 _maxBitrate;
        int32 _avgBitrate;
        int32 _slidingAvgBitrate;
};



#endif

