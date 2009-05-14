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
/*                         MPEG-4 MpegSampleEntry Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MpegSampleEntry is used for hint tracks.
*/


#ifndef MPEGSAMPLEENTRY_H_INCLUDED
#define MPEGSAMPLEENTRY_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef SAMPLEENTRY_H_INCLUDED
#include "sampleentry.h"
#endif

#ifndef ESDATOM_H_INCLUDED
#include "esdatom.h"
#endif

class MpegSampleEntry : public SampleEntry
{

    public:
        MpegSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~MpegSampleEntry();

        uint8 getObjectTypeIndication() const
        {
            if (_pes != NULL)
            {
                return _pes->getObjectTypeIndication();
            }
            else
            {
                return 0xFF;
            }
        }

        // Member gets and sets
        const ESDAtom &getESDAtom() const
        {
            return *_pes;
        }

        virtual uint32 getESID() const
        {
            if (_pes != NULL)
            {
                if (_pes->getESDescriptorPtr() != NULL)
                {
                    return _pes->getESDescriptorPtr()->getESID();
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }

        virtual const ESDescriptor *getESDescriptor() const
        {
            if (_pes != NULL)
            {
                return _pes->getESDescriptorPtr();
            }
            else
            {
                return NULL;
            }
        }

    private:
        ESDAtom *_pes;
};


#endif  // MPEGSAMPLEENTRY_H_INCLUDED

