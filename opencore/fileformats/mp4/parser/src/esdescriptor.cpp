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


#define IMPLEMENT_ESDescriptor

#include "esdescriptor.h"
#include "atomutils.h"
#include "atomdefs.h"

#include "oscl_utf8conv.h"

// Stream-in Constructor
ESDescriptor::ESDescriptor(MP4_FF_FILE *fp)
        : BaseDescriptor(fp)
{
    _pdcd  = NULL;
    _pslcd = NULL;
    _urlStringPtr = NULL;

    int32 size = (int32)getSize();
    size -= getSizeOfSizeField();
    size -= 1;
    int32 numBytesRead = 0;

    if (_success)
    {
        _pparent = NULL;

        if (!AtomUtils::read16(fp, _ESID))
        {
            _success = false;
            return;
        }
        numBytesRead += 2;


        // Read in and unpack stream dependence flag, url flag, reserved, and priority
        uint8 data;
        if (!AtomUtils::read8(fp, data))
        {
            _success = false;
            return;
        }
        numBytesRead += 1;

        _streamDependenceFlag = ((data >> 7) & 0x1) == 1 ? true : false; // leading bit
        _urlFlag = ((data >> 6) & 0x1) == 1 ? true : false; // next bit
        _OCRStreamFlag = (uint8)((data >> 5) & 0x1);  // next bit
        _streamPriority = (uint8)(data & 0x1f);  // last 5 bits

        if (_success)
        {
            if (_streamDependenceFlag)
            {
                if (!AtomUtils::read16(fp, _dependsOnESID))
                {
                    _success = false;
                    return;
                }
                numBytesRead += 2;
            }

            if (_urlFlag)
            {
                if (!AtomUtils::read8(fp, _urlLength))
                {
                    _success = false;
                    return;
                }
                numBytesRead += 1;

                if (_success)
                {
                    _urlStringPtr = (uint8 *)oscl_malloc(_urlLength + 1); // +1 for null termination
                    if (!AtomUtils::readByteData(fp, _urlLength, _urlStringPtr))
                    {
                        _success = false;
                        return;
                    }
                    _urlStringPtr[_urlLength] = 0; // Set null-termination

                    // allocate space for wide character buffer
                    OSCL_TCHAR *pwbuf = (OSCL_TCHAR*) oscl_malloc(_urlLength * sizeof(OSCL_TCHAR));
                    // convert utf8 character array to wide character array
                    oscl_UTF8ToUnicode((const char *)_urlStringPtr, _urlLength, pwbuf, _urlLength*sizeof(OSCL_TCHAR));
                    _urlString += (pwbuf);
                }
                numBytesRead += _urlLength;
            }

            if (_OCRStreamFlag)
            {
                // JUST TO MAKE SURE THAT NEXT BYTE ISN'T DEC TAG
                // FOR CONFORMANCE WITH OLDER VERSION OF THE SYSTEM SPEC
                uint8 peek_tag = AtomUtils::peekNextByte(fp);

                if (peek_tag != DECODER_CONFIG_DESCRIPTOR_TAG)
                {
                    AtomUtils::read16(fp, OCR_ES_ID);
                    numBytesRead += 2;
                }
            }

        }

        if (_success)
        {
            // Read rest of members
            uint8 descr_tag = AtomUtils::peekNextByte(fp);

            if (descr_tag == DECODER_CONFIG_DESCRIPTOR_TAG)
            {
                PV_MP4_FF_NEW(fp->auditCB, DecoderConfigDescriptor, (fp), _pdcd);

                if (!_pdcd->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pdcd->GetMP4Error();
                }
                else
                {
                    numBytesRead +=
                        (int32)(_pdcd->getSize());

                    descr_tag = AtomUtils::peekNextByte(fp);

                    if (descr_tag == SL_CONFIG_DESCRIPTOR)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, SLConfigDescriptor, (fp), _pslcd);
                        if (!_pslcd->MP4Success())
                        {
                            _success = false;
                            _mp4ErrorCode = _pslcd->GetMP4Error();
                        }

                        numBytesRead +=
                            (int32)(_pslcd->getSize());
                    }
                    else
                    {
                        _success      = false;
                        _mp4ErrorCode = READ_ES_DESCRIPTOR_FAILED;
                    }

                    if (numBytesRead < size)
                    {
                        int32 leftOverBytes = (size - numBytesRead);
                        uint8 data = 0;

                        while (leftOverBytes > 0)
                        {
                            if (!AtomUtils::read8(fp, data))
                            {
                                _success = false;
                                return;
                            }
                            leftOverBytes--;
                        }
                    }
                    else if (numBytesRead > size)
                    {
                        _success = false;
                        return;
                    }
                }
            }
            else
            {
                _success      = false;
                _mp4ErrorCode = READ_ES_DESCRIPTOR_FAILED;
            }
        }

        if (_success)
        {
            _pdcd->setParent(this);
            _pslcd->setParent(this);
        }
    }
    else
    {
        _mp4ErrorCode = READ_ES_DESCRIPTOR_FAILED;
    }
}

// Destructor
ESDescriptor::~ESDescriptor()
{
    // Free space allocated for url string
    if (_urlFlag)
        oscl_free(_urlStringPtr);

    // Cleanup descriptors
    if (_pdcd != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DecoderConfigDescriptor, _pdcd);
    }

    if (_pslcd != NULL)
    {
        PV_MP4_FF_DELETE(NULL, SLConfigDescriptor, _pslcd);
    }
}


OSCL_wString& ESDescriptor::getUrlString()
{
    return _urlString;
}


