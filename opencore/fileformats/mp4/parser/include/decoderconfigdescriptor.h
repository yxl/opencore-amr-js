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
    This DecoderConfigDescriptor Class
*/


#ifndef DECODERCONFIGDESCRIPTOR_H_INCLUDED
#define DECODERCONFIGDESCRIPTOR_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef BASEDESCRIPTOR_H_INCLUDED
#include "basedescriptor.h"
#endif
#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif

class DecoderConfigDescriptor : public BaseDescriptor
{

    public:
        DecoderConfigDescriptor(MP4_FF_FILE *fp); // Stream-in ctor Constructor
        virtual ~DecoderConfigDescriptor();

        // Member gets
        uint8 getObjectTypeIndication()
        {
            return _objectTypeIndication;
        }
        uint8 getStreamType()
        {
            return _streamType;
        }
        bool getUpstream()
        {
            return _upStream;
        }
        uint32 getMaxBitrate()
        {
            return _maxBitrate;
        }

        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return _pdecSpecificInfo;
        }

        uint32 getMaxBufferSizeDB()
        {
            return _bufferSizeDB;
        }
        uint32 getAverageBitrate()
        {
            return _avgBitrate;
        }

    private:
        uint8 _objectTypeIndication; // (8)
        uint8 _streamType; // (6) 0x04 for VisualStream, 0x05 for AudioStream
        bool _upStream; // (1)
        uint8 _reserved; // (1)
        uint32 _bufferSizeDB; // (24)
        uint32 _maxBitrate; // (32)
        uint32 _avgBitrate; // (32)

        // Change to support CE port
        DecoderSpecificInfo  *_pdecSpecificInfo;
};

#endif // DECODERCONFIGDESCRIPTOR_H_INCLUDED

