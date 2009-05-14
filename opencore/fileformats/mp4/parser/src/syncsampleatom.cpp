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
/*                         MPEG-4 SyncSampleAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SyncSampleAtom Class provides a compact marking of the random access
    points within the stream.
*/


#define IMPLEMENT_SyncSampleAtom

#include "syncsampleatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Stream-in constructor
SyncSampleAtom::SyncSampleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _psampleNumbers = NULL;

    if (_success)
    {

        _nextSampleNumber = 0;

        if (!AtomUtils::read32(fp, _entryCount))
        {
            _success = false;
        }

        uint32 dataSize = _size - (DEFAULT_FULL_ATOM_SIZE + 4);

        uint32 entrySize = (4);

        if ((_entryCount*entrySize) > dataSize)
        {
            _success = false;
        }

        if (_success)
        {

            PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_entryCount), _psampleNumbers);

            uint32 sync;
            for (uint32 i = 0; i < _entryCount; i++)
            {
                if (!AtomUtils::read32(fp, sync))
                {
                    _success = false;
                    break;
                }
                _psampleNumbers[i] = (sync);
            }
        }

        if (!_success)
            _mp4ErrorCode = READ_SYNC_SAMPLE_ATOM_FAILED;
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_SYNC_SAMPLE_ATOM_FAILED;
    }
}

// Destructor
SyncSampleAtom::~SyncSampleAtom()
{
    // Cleanup vector
    if (_psampleNumbers != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _psampleNumbers);
        _psampleNumbers = NULL;
    }
}

// Returns the sync sample number at vector location 'index'
int32
SyncSampleAtom::getSampleNumberAt(int32 index) const
{
    if (index < (int32)_entryCount)
    {
        return (_psampleNumbers[index] - 1);
    }
    else
    {
        return PV_ERROR;
    }
}

// Returns the first sync sample that occurs at or after 'sampleNum'.  This is used
// when seeking in the bitstream trying to find a random access sample.  This method
// returns the random access sample that is closest immediately following 'sampleNum'
int32
SyncSampleAtom::getSyncSampleFollowing(uint32 sampleNum) const
{
    uint32 sync = 0;
    int32 count = 0;
    while (sync < sampleNum)
    {
        if (count < (int32)_entryCount)
        {
            sync = _psampleNumbers[count] - 1;
            count ++;
        }
        else
        {
            return PV_ERROR;
        }
    }

    // Incase the sampleNum is one of the sync sample the existing sync
    // is same as sampleNum In this way the sample return from getSyncSampleFollowing
    // and getSyncSampleBefore become same causing repos issue. This check
    // will advance the sync to next I frame incase it is available
    if (sync == sampleNum)
    {
        if (count < (int32)_entryCount)
        {
            sync = _psampleNumbers[count] - 1;
        }

        // No I frame ahead exists, return 0 will ensure that engine ignore reposition and continue playing from current position
        else
        {
            sync = 0;
        }
    }

    return sync;
}

// Returns the first sync sample that occurs at or after 'sampleNum'.  This is used
// when seeking in the bitstream trying to find a random access sample.  This method
// returns the random access sample that is closest immediately following 'sampleNum'
int32
SyncSampleAtom::getSyncSampleBefore(uint32 sampleNum) const
{
    uint32 sync = 0;
    int32 count = 0;

    //fix resetPlayback at the end of bitstream
    //_asm {int 3};

    while ((sync < sampleNum) &&
            (count < (int32)_entryCount))
    {
        sync = _psampleNumbers[count] - 1;
        count ++;
    }

    //the nearest I frame before sampleNum
    if (sync <= sampleNum)
        return sync;
    else
    {
        count = count - 2;
        sync = _psampleNumbers[count] - 1;
        return sync;
    }
}


bool
SyncSampleAtom::IsSyncSample(uint32 sampleNum) const
{
    for (int32 count = 0; count < (int32)_entryCount; count++)
    {
        if ((_psampleNumbers[count] - 1) == sampleNum)
        {
            return true;
        }
    }

    return false;
}


