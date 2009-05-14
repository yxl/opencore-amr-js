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
/*                     MPEG-4 InitialObjectDescriptor Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#define IMPLEMENT_InitialObjectDescriptor

#include "initialobjectdescriptor.h"
#include "atomutils.h"

typedef Oscl_Vector<ES_ID_Inc*, OsclMemAllocator> ES_ID_IncVecType;

// Stream-in Constructor
InitialObjectDescriptor::InitialObjectDescriptor(MP4_FF_FILE *fp)
        : ObjectDescriptor(fp, false)
{
    if (_success)
    {

        _pES_ID_Inc = NULL;

        _reserved = 0xf;

        uint16 data;
        if (!AtomUtils::read16(fp, data))
            _success = false;

        _objectDescriptorID = (uint16)(data >> 6);

        _urlFlag = false;
        if (((data >> 5) & 0x0001) == 1)
        {
            _urlFlag = true;
        }

        _includeInlineProfilesFlag = false;
        if (((data >> 4) & 0x0001) == 1)
        {
            _includeInlineProfilesFlag = true;
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
            if (!AtomUtils::read8(fp, _ODProfileLevelIndication))
                _success = false;
            if (!AtomUtils::read8(fp, _sceneProfileLevelIndication))
                _success = false;
            if (!AtomUtils::read8(fp, _audioProfileLevelIndication))
                _success = false;
            if (!AtomUtils::read8(fp, _visualProfileLevelIndication))
                _success = false;
            if (!AtomUtils::read8(fp, _graphicsProfileLevelIndication))
                _success = false;

            // Read in ESDescriptor references (i.e. ESIDs)
            int32 readIn = getSizeOfSizeField();
            readIn += 1; // For tag
            readIn += 5; // For 5 level indicators
            readIn += 2; // For 10-bit ODID, etc.
            int32 delta = _sizeOfClass - readIn;

            uint8  tag;
            tag = AtomUtils::peekNextByte(fp);
            if (tag == 0)	//ES_ID, old version
            {
                for (int32 i = 0; i < delta; i++)
                {
                    uint8 data_8;

                    if (!AtomUtils::read8(fp, data_8))
                    {
                        _success = false;
                        break;
                    }
                }
            }
            else if (tag == ES_DESCRIPTOR_TAG)
            {
                // TO TAKE CARE OF THE CASE WHERE IN
                // WE HAVE THE "ES_DecsrTag" AS OPPOSSED
                // TO AN "ES_ID_IncTag"
                for (int32 i = 0; i < delta; i++)
                {
                    uint8 data_8;
                    if (!AtomUtils::read8(fp, data_8))
                    {
                        _success = false;
                        break;
                    }
                }
            }
            else
            {//new version ES_ID_Inc and ES_ID_Ref
                ES_ID_Inc *esIDInc;

                PV_MP4_FF_NEW(fp->auditCB, ES_ID_IncVecType, (), _pES_ID_Inc);

                while (delta > 0)
                {

                    tag = AtomUtils::peekNextByte(fp);

                    if (tag == ES_ID_INC_TAG)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, ES_ID_Inc, (fp), esIDInc);
                        delta -= esIDInc->getSize();
                        (*_pES_ID_Inc).push_back(esIDInc);
                    }
                    else
                    {
                        ExpandableBaseClass* tempDesc = NULL;
                        PV_MP4_FF_NEW(fp->auditCB, ExpandableBaseClass, (fp, false), tempDesc);
                        if (!tempDesc->MP4Success())
                        {
                            _success = false;
                            break;
                        }
                        uint32 totaltempDescSize = tempDesc->getSize();
                        uint32 tempDescSize = 1; //for tag
                        tempDescSize += tempDesc->getSizeOfSizeField();
                        uint32 remDescSize = (totaltempDescSize - tempDescSize);
                        if (totaltempDescSize > (uint32)delta)
                        {
                            _success = false;
                            break;
                        }
                        AtomUtils::seekFromCurrPos(fp, remDescSize);
                        delta -= totaltempDescSize;

                        PV_MP4_FF_DELETE(NULL, ExpandableBaseClass, tempDesc);
                    }
                }
            }
        }

        if (!_success)
            _mp4ErrorCode = READ_INITIAL_OBJECT_DESCRIPTOR_FAILED;
    }
    else
    {
        _mp4ErrorCode = READ_INITIAL_OBJECT_DESCRIPTOR_FAILED;
    }

}

// Destructor
InitialObjectDescriptor::~InitialObjectDescriptor()
{
    uint32 i;
    if (_pES_ID_Inc != NULL)
    {
        for (i = 0; i < _pES_ID_Inc->size(); i++)
            PV_MP4_FF_DELETE(NULL, ES_ID_Inc, (*_pES_ID_Inc)[i]);

        PV_MP4_FF_TEMPLATED_DELETE(NULL, ES_ID_IncVecType, Oscl_Vector, _pES_ID_Inc);
    }
    // EMPTY - The vector of ESIDs gets handled in the base class destructor
}


