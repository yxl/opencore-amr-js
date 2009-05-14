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
    This PVA_FF_DecoderConfigDescriptor Class
*/


#ifndef __DecoderConfigDescriptor_H__
#define __DecoderConfigDescriptor_H__

#include "basedescriptor.h"

#include "decoderspecificinfo.h"


class PVA_FF_DecoderConfigDescriptor : public PVA_FF_BaseDescriptor
{

    public:
        PVA_FF_DecoderConfigDescriptor(int32 streamType, int32 codecType); // Constructor

        virtual ~PVA_FF_DecoderConfigDescriptor();

        void init(int32 streamType);

        // Rendering the Descriptor in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Member gets and sets
        uint8 getObjectTypeIndication()
        {
            return _objectTypeIndication;
        }
        void setObjectTypeIndication(uint8 ind)
        {
            _objectTypeIndication = ind;
        }

        uint8 getStreamType()
        {
            return _streamType;
        }
        void setStreamType(uint8 type)
        {
            _streamType = type;
        }

        bool getUpstream()
        {
            return _upStream;
        }
        void setUpstream(bool upStream)
        {
            _upStream = upStream;
        }

        uint32 getBufferSizeDB() const
        {
            return _bufferSizeDB;
        }
        void setBufferSizeDB(uint32 size)
        {
            _bufferSizeDB = size;
        }

        uint32 getMaxBitrate()
        {
            return _maxBitrate;
        }
        void setMaxBitrate(uint32 max)
        {
            _maxBitrate = max;
        }

        uint32 getAvgBitrate()
        {
            return _avgBitrate;
        }
        void setAvgBitrate(uint32 avg)
        {
            _avgBitrate = avg;
        }

        // Getting and setting the Mpeg4 VOL header
        PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const;
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo);

        void nextSampleSize(uint32 size);

        void setMaxBufferSizeDB(uint32 max)
        {
            _bufferSizeDB = max;
        }

        void operator = (PVA_FF_DecoderConfigDescriptor dcd)
        {
            _objectTypeIndication = dcd._objectTypeIndication;
            _streamType = dcd._streamType;
            _upStream = dcd._upStream;
            _bufferSizeDB = dcd._bufferSizeDB;
            _maxBitrate = dcd._maxBitrate;
            _avgBitrate = dcd._avgBitrate;
        }

        virtual void recomputeSize();
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);
    private:
        uint8 _objectTypeIndication; // (8)
        uint8 _streamType; // (6) 0x04 for VisualStream, 0x05 for AudioStream
        bool _upStream; // (1)
        uint8 _reserved; // (1)
        uint32 _bufferSizeDB; // (24)
        uint32 _maxBitrate; // (32)
        uint32 _avgBitrate; // (32)
        int32 _codecType;
        uint32 iCurrFilePos;

        Oscl_Vector<PVA_FF_DecoderSpecificInfo*, OsclMemAllocator> *_pdecSpecificInfoVec;
};



#endif

