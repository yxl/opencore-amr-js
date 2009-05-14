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
/*                        MPEG-4 DataReferenceAtom Class                         */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DataReferenceAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#define IMPLEMENT_DataReferenceAtom

#include "datareferenceatom.h"
#include "dataentryurlatom.h"
#include "atomdefs.h"
#include "atomutils.h"

typedef Oscl_Vector<DataEntryUrlAtom*, OsclMemAllocator> dataEntryUrlAtomVecType;
// Stream-in ctor
DataReferenceAtom::DataReferenceAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{

    _pdataEntryVec = NULL;

    if (_success)
    {

        _pparent = NULL;

        PV_MP4_FF_NEW(fp->auditCB, dataEntryUrlAtomVecType, (), _pdataEntryVec);

        if (!AtomUtils::read32(fp, _entryCount))
        {
            _success = false;
            _mp4ErrorCode = READ_DATA_REFERENCE_ATOM_FAILED;
        }
        else
        {
            // THERE MUST BE ATLEAST ONE ENTRY
            int32 temp = (int32)(_entryCount);

            if (temp <= 0)
            {
                _success = false;
                _mp4ErrorCode = READ_DATA_REFERENCE_ATOM_FAILED;
            }
        }

        if (_success)
        {
            DataEntryUrlAtom *deua = NULL;
            for (uint32 i = 0; i < _entryCount; i++)
            {
                PV_MP4_FF_NEW(fp->auditCB, DataEntryUrlAtom, (fp), deua);
                if (!deua->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = deua->GetMP4Error();
                    if (deua != NULL)
                    {
                        PV_MP4_FF_DELETE(NULL, DataEntryUrlAtom, deua);
                        deua = NULL;
                    }
                    break;
                }
                else
                {
                    (*_pdataEntryVec).push_back(deua);
                    deua->setParent(this);
                }
            }
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_DATA_REFERENCE_ATOM_FAILED;
    }

}

// Destructor
DataReferenceAtom::~DataReferenceAtom()
{
    // Clean up vector of DataEntryAtoms
    if (_pdataEntryVec != NULL)
    {
        for (uint32 i = 0; i < _pdataEntryVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, DataEntryUrlAtom, (*_pdataEntryVec)[i]);
        }

        PV_MP4_FF_TEMPLATED_DELETE(NULL, dataEntryUrlAtomVecType, Oscl_Vector, _pdataEntryVec);
        _pdataEntryVec = NULL;
    }
}

DataEntryAtom *
DataReferenceAtom::getEntryAt(int32 index)
{
    if ((uint32)index < _pdataEntryVec->size())
    {
        return (DataEntryAtom*)(*_pdataEntryVec)[index];
    }
    else
    {
        return NULL;
    }
}
