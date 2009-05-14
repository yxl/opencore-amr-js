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
/*                          MPEG-4 ESDescriptor Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ESDescriptor Class contains information on the Elementary Stream
*/


#ifndef ESDESCRIPTOR_H_INCLUDED
#define ESDESCRIPTOR_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef BASEDESCRIPTOR_H_INCLUDED
#include "basedescriptor.h"
#endif

#ifndef DECODERCONFIGDESCRIPTOR_H_INCLUDED
#include "decoderconfigdescriptor.h"
#endif

#ifndef SLCONFIGDESCRIPTOR_H_INCLUDED
#include "slconfigdescriptor.h"
#endif


class ESDescriptor : public BaseDescriptor
{

    public:
        ESDescriptor(MP4_FF_FILE *fp); // Stream-in constructor
        virtual ~ESDescriptor(); // Destructor

        // Member gets
        uint8 getObjectTypeIndication()
        {
            if (_pdcd != NULL)
            {
                return _pdcd->getObjectTypeIndication();
            }
            else
            {
                return 0xFF;
            }
        }

        uint16 getESID() const
        {
            return _ESID;
        }
        bool getStreamDependenceFlag()
        {
            return _streamDependenceFlag;
        }
        bool getUrlFlag()
        {
            return _urlFlag;
        }
        uint8 getStreamPriority()
        {
            return _streamPriority;
        }
        uint16 getDependsOnESID()
        {
            return _dependsOnESID;
        }
        uint8 getUrlLength()
        {
            return _urlLength;
        }
        // Url length gets set when setting the actual string - defaults to 0

        // Retuns a UNICODE string given the null-terminated byte array
        OSCL_wString& getUrlString();


        const DecoderConfigDescriptor &getDecoderConfigDescriptor()
        {
            return *_pdcd;
        }

        const SLConfigDescriptor &getSLConfigDescriptor()
        {
            return *_pslcd;
        }

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            if (_pdcd != NULL)
            {
                return _pdcd->getDecoderSpecificInfo();
            }
            else
            {
                return NULL;
            }
        }

        // Get the max size buffer needed to retrieve the media samples
        uint32 getMaxBufferSizeDB() const
        {
            if (_pdcd != NULL)
            {
                return _pdcd->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        uint32 getAverageBitrate() const
        {
            if (_pdcd != NULL)
            {
                return _pdcd->getAverageBitrate();
            }
            else
            {
                return 0;
            }
        }

        uint32 getMaxBitrate() const
        {
            if (_pdcd != NULL)
            {
                return _pdcd->getMaxBitrate();
            }
            else
            {
                return 0;
            }
        }

    private:
        uint16 _ESID; // (16)
        bool   _streamDependenceFlag; // (1)
        bool   _urlFlag; // (1)
        uint8  _OCRStreamFlag; // (1)
        uint8  _streamPriority; // (5)
        uint16 OCR_ES_ID;

        uint16 _dependsOnESID; // (16)
        uint8  _urlLength; // (8)
        uint8  *_urlStringPtr; // (8)[_urlLength]

        DecoderConfigDescriptor *_pdcd;
        SLConfigDescriptor      *_pslcd;

        OSCL_wHeapString<OsclMemAllocator> _urlString;
};



#endif  // ESDESCRIPTOR_H_INCLUDED


