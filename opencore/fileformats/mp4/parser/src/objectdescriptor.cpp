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
/*                        MPEG-4 ObjectDescriptor Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#define IMPLEMENT_ObjectDescriptor

#include "objectdescriptor.h"
#include "atomutils.h"

#include "oscl_utf8conv.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

// Constructor
ObjectDescriptor::ObjectDescriptor(MP4_FF_FILE *fp, bool all)
        : BaseDescriptor(fp)
{
    _pESIDVec = NULL;

    if (_success)
    {

        _pparent = NULL;

        PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pESIDVec);

        // Check needed if directly reading in an ObjectDescriptor (all=true) or
        // an InitialObjectDescriptor (all=false)
        if (all)
        {
            _reserved = 0x1f;

            uint16 data;
            if (!AtomUtils::read16(fp, data))
                _success = false;

            _objectDescriptorID = (uint16)(data >> 6);

            _urlFlag = false;
            if (((data >> 5) & 0x0001) == 1)
            {
                _urlFlag = true;
            }

            if (_urlFlag)
            {

                if (!AtomUtils::read8(fp, _urlLength))
                    _success = false;

                _urlStringPtr = (uint8 *) oscl_malloc(_urlLength + 1); // +1 for the null termination

                if (!AtomUtils::readByteData(fp, _urlLength, _urlStringPtr))
                    _success = false;
            }
            else
            {
                // Read in ESDescriptor references (i.e. ESIDs)
                int32 readIn = AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(_sizeOfClass);
                readIn += 1; // For tag
                readIn += 2; // For 10-bit ODID, etc.
                int32 delta = _sizeOfClass - readIn;

                uint32  tag;

                //check for the next byte to see if it is a tag or 0 for ES_ID
                tag = AtomUtils::peekNextNthBytes(fp,  0);
                if (tag == 0)	//ES_ID, old version
                {
                    uint32 esid;
                    for (int32 i = 0; i < delta; i += 4)
                    {
                        if (!AtomUtils::read32(fp, esid))
                        {
                            _success = false;
                            break;
                        }
                        (*_pESIDVec).push_back(esid);
                    }

                }
                else
                {//new version ES_ID_Inc and ES_ID_Ref
                    //currently I do the parse only
                    ES_ID_Ref *esIDRef;
                    while (delta > 0)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, ES_ID_Ref, (fp), esIDRef);
                        delta -= esIDRef->getSize();
                        (*_pES_ID_Ref).push_back(esIDRef);
                    }
                }

            }
        }

        if (!_success)
            _mp4ErrorCode = READ_OBJECT_DESCRIPTOR_FAILED;
    }
    else
    {
        _mp4ErrorCode = READ_OBJECT_DESCRIPTOR_FAILED;
    }

}

// Destructor
ObjectDescriptor::~ObjectDescriptor()
{
    if (_pESIDVec != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pESIDVec);
    }
}

OSCL_wHeapString<OsclMemAllocator> ObjectDescriptor::getUrlString() const
{

    // Need to convert from a Multibyte array pointer to a wide character array

    // allocate space for wide character buffer
    OSCL_TCHAR *pwbuf = (OSCL_TCHAR*) oscl_malloc(_urlLength * sizeof(OSCL_TCHAR));

    if (!pwbuf)
        return NULL;

    // convert utf8 character array to wide character array
    oscl_UTF8ToUnicode((const char *)_urlStringPtr, _urlLength, pwbuf, _urlLength*sizeof(OSCL_TCHAR));

    // return newly created ZString
    return OSCL_wHeapString<OsclMemAllocator>(pwbuf);
}
