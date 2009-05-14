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


#ifndef __ESDescriptor_H__
#define __ESDescriptor_H__

#include "basedescriptor.h"
#include "decoderconfigdescriptor.h"
#include "slconfigdescriptor.h"

class PVA_FF_ESDescriptor : public PVA_FF_BaseDescriptor
{

    public:
        PVA_FF_ESDescriptor(int32 streamType, int32 codecType); // Constructor

        virtual ~PVA_FF_ESDescriptor(); // Destructor



        // Member gets and sets
        void setESID(uint16 esid)
        {
            _ESID = esid;
        }

        bool getStreamDependenceFlag()
        {
            return _streamDependenceFlag;
        }
        void setStreamDependence(bool flag)
        {
            _streamDependenceFlag = flag;
        }

        bool getUrlFlag()
        {
            return _urlFlag;
        }
        void setUrlFlag(bool flag)
        {
            _urlFlag = flag;
        }

        uint8 getStreamPriority()
        {
            return _streamPriority;
        }
        void setStreamPriority(uint8 priority)
        {
            _streamPriority = priority;
        }

        uint16 getDependsOnESID()
        {
            return _dependsOnESID;
        }
        void setDependsOnESID(uint16 esid)
        {
            _dependsOnESID = esid;
        }

        uint8 getUrlLength()
        {
            return _urlLength;
        }
        // Url length gets set when setting the actual string - defaults to 0

        PVA_FF_UTF8_STRING_PARAM getUrlString()
        {
            return _urlString;
        }
        void setUrlString(PVA_FF_UTF8_STRING_PARAM url)
        {
            _urlString = url;
        }

        void setTargetBitrate(uint32 bitrate)
        {
            //BOTH MAX AND AVG BITRATES ARE SET TO THE SAME VALUE
            _pdcd->setAvgBitrate(bitrate);
            _pdcd->setMaxBitrate(bitrate);
        }

        void nextSampleSize(uint32 size)
        {
            _pdcd->nextSampleSize(size);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _pdcd->setMaxBufferSizeDB(max);
        }

        const PVA_FF_DecoderConfigDescriptor &getDecoderConfigDescriptor()
        {
            return *_pdcd;
        }
        void setDecoderConfigDescriptor(PVA_FF_DecoderConfigDescriptor *pdcd)
        {
            _pdcd = pdcd;
        }

        const PVA_FF_SLConfigDescriptor &getSLConfigDescriptor()
        {
            return *_pslcd;
        }
        void setSLConfigDescriptor(PVA_FF_SLConfigDescriptor *pslcd)
        {
            _pslcd = pslcd;
        }

        // Recompute the size of the class as is rendered to a file string
        virtual void recomputeSize();

        // Getting and setting the Mpeg4 VOL header
        PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return _pdcd->getDecoderSpecificInfo();
        }
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _pdcd->addDecoderSpecificInfo(pinfo);
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
        {
            _pdcd->writeMaxSampleSize(_afp);
        }


    private:
        uint16 _ESID; // (16)
        bool _streamDependenceFlag; // (1)
        bool _urlFlag; // (1)
        uint8 _reserved; // (1)
        uint8 _streamPriority; // (5)

        uint16 _dependsOnESID; // (16)
        uint8 _urlLength; // (8)
        PVA_FF_UTF8_HEAP_STRING _urlString; // (8)[_urlLength]

        PVA_FF_DecoderConfigDescriptor *_pdcd;
        PVA_FF_SLConfigDescriptor *_pslcd;

    private:
        void init();
        uint16 getNextESID();

};



#endif

