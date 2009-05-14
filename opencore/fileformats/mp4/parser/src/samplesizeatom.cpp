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
/*                           MPEG-4 SampleSizeAtom Class                         */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#define IMPLEMENT_SampleSizeAtom_H__

#include "samplesizeatom.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in constructor
SampleSizeAtom::SampleSizeAtom(MP4_FF_FILE *fp,
                               uint32 mediaType,
                               uint32 size,
                               uint32 type,
                               OSCL_wString& filename,
                               uint32 parsingMode)
        : FullAtom(fp, size, type),
        _mediaType(mediaType)
{
    _psampleSizeVec = NULL;

    _maxSampleSize = 0;
    _parsed_entry_cnt = 0;
    _fileptr = NULL;

    if (0 != AtomUtils::getFileBufferingCapacity(fp))
    {
        _stbl_buff_size = MAX_CACHED_TABLE_ENTRIES_MBDS;
    }
    else
    {
        _stbl_buff_size = MAX_CACHED_TABLE_ENTRIES_FILE;
    }

    _next_buff_number = 0;
    _curr_buff_number = 0;
    _curr_entry_point = 0;
    _stbl_fptr_vec = NULL;
    _parsing_mode = parsingMode;
    _SkipOldEntry = false;
    if (_success)
    {
        if (_size < (DEFAULT_FULL_ATOM_SIZE + 4 + 4))
        {
            _success = false;
            _mp4ErrorCode = READ_SAMPLE_SIZE_ATOM_FAILED;
            return;
        }

        if (!AtomUtils::read32read32(fp, ((uint32 &)_sampleSize), _sampleCount))
        {
            _success = false;
        }

        uint32 dataSize = _size - (DEFAULT_FULL_ATOM_SIZE + 4 + 4);

        uint32 entrySize = (4);

        if (_sampleSize == 0)
        {	//samples have different size
            if ((_sampleCount*entrySize) > dataSize)
            {
                _success = false;
            }
            else
            {	//the entry is valid

                if (_parsing_mode == 1)
                {
                    if ((_sampleCount > _stbl_buff_size)) // cahce size is 4K so that optimization should work if entry_count is greater than 4K
                    {
                        uint32 fptrBuffSize = (_sampleCount / _stbl_buff_size) + 1;

                        PV_MP4_FF_ARRAY_NEW(NULL, uint32, (fptrBuffSize), _stbl_fptr_vec);
                        if (_stbl_fptr_vec == NULL)
                        {
                            _success = false;
                            _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                            return;
                        }

                        PV_MP4_FF_ARRAY_NEW(NULL, int32, (_stbl_buff_size), _psampleSizeVec);
                        if (_psampleSizeVec == NULL)
                        {
                            _success = false;
                            _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                            return;
                        }

                        {
                            OsclAny* ptr = (MP4_FF_FILE *)(oscl_malloc(sizeof(MP4_FF_FILE)));
                            if (ptr == NULL)
                            {
                                _success = false;
                                _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                                return;
                            }
                            _fileptr = OSCL_PLACEMENT_NEW(ptr, MP4_FF_FILE());
                            _fileptr->_fileServSession = fp->_fileServSession;
                            _fileptr->_pvfile.SetCPM(fp->_pvfile.GetCPM());
                            if (AtomUtils::OpenMP4File(filename,
                                                       Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                                                       _fileptr) != 0)
                            {
                                _success = false;
                                _mp4ErrorCode = FILE_OPEN_FAILED;
                            }

                            _fileptr->_fileSize = fp->_fileSize;
                        }
                        int32 _head_offset = AtomUtils::getCurrentFilePosition(fp);
                        AtomUtils::seekFromCurrPos(fp, dataSize);
                        AtomUtils::seekFromStart(_fileptr, _head_offset);

                        return;
                    }
                    else
                    {
                        _parsing_mode = 0;
                        _stbl_buff_size = _sampleCount;

                    }
                }
                else
                {
                    _stbl_buff_size = _sampleCount;
                }

                PV_MP4_FF_ARRAY_NEW(NULL, int32, (_sampleCount), _psampleSizeVec);

                if (_psampleSizeVec == NULL)
                {
                    _success = false;
                    _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                    return;
                }

                uint32 sample;
                for (uint32 i = 0; i < _sampleCount; i++)
                {
                    if (!AtomUtils::read32(fp, sample))
                    {
                        _success = false;
                        break;
                    }
                    _psampleSizeVec[i] = sample;

                    if (_psampleSizeVec[i] > _maxSampleSize)
                    {
                        _maxSampleSize = _psampleSizeVec[i];
                    }
                    _parsed_entry_cnt++;
                }
            }
        }
        else
            _maxSampleSize = (uint32)_sampleSize;

        if (!_success)
            _mp4ErrorCode = READ_SAMPLE_SIZE_ATOM_FAILED;

    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_SAMPLE_SIZE_ATOM_FAILED;
    }
}

bool SampleSizeAtom::ParseEntryUnit(uint32 sample_cnt)
{

    uint32 threshold = 0;
    if (0 != AtomUtils::getFileBufferingCapacity(_fileptr))
    {
        threshold = TABLE_ENTRIES_THRESHOLD_MBDS;
    }
    else
    {
        threshold = TABLE_ENTRIES_THRESHOLD_FILE;
    }

    sample_cnt += threshold;

    if (sample_cnt > _sampleCount)
        sample_cnt = _sampleCount;


    uint32 sample;
    while (_parsed_entry_cnt < sample_cnt)
    {
        _curr_entry_point = _parsed_entry_cnt % _stbl_buff_size;
        _curr_buff_number = _parsed_entry_cnt / _stbl_buff_size;
        if (_curr_buff_number  == _next_buff_number)
        {
            uint32 currFilePointer = AtomUtils::getCurrentFilePosition(_fileptr);
            _stbl_fptr_vec[_curr_buff_number] = currFilePointer;
            _next_buff_number++;
        }

        if (!_curr_entry_point)
        {
            uint32 currFilePointer = _stbl_fptr_vec[_curr_buff_number];
            AtomUtils::seekFromStart(_fileptr, currFilePointer);
        }

        if (!AtomUtils::read32(_fileptr, sample))
        {
            return false;
        }
        _psampleSizeVec[_curr_entry_point] = sample;

        if (_psampleSizeVec[_curr_entry_point] > _maxSampleSize)
        {
            _maxSampleSize = _psampleSizeVec[_curr_entry_point];
        }
        _parsed_entry_cnt++;
    }
    return true;
}

// Destructor
SampleSizeAtom::~SampleSizeAtom()
{
    if (_psampleSizeVec != NULL)
    {
        // CLEAN UP VECTOR
        PV_MP4_ARRAY_DELETE(NULL, _psampleSizeVec);
    }
    if (_fileptr != NULL)
    {
        if (_fileptr->IsOpen())
        {
            AtomUtils::CloseMP4File(_fileptr);
        }

        oscl_free(_fileptr);
    }
    if (_stbl_fptr_vec != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _stbl_fptr_vec);

}

int32
SampleSizeAtom::getSampleSizeAt(uint32 index)
{
    if (_psampleSizeVec == NULL)
    {
        return _sampleSize;
    }

    if (_parsing_mode == 1)
    {
        if (index < _sampleCount)
        {
            if (index >= _parsed_entry_cnt)
            {
                ParseEntryUnit(index);
            }
            else
            {
                uint32 entryLoc = index / _stbl_buff_size;
                if (!_SkipOldEntry) //Not to use old entries in case of backward Repos.
                {
                    uint32 actualEntryIndex = _parsed_entry_cnt % _stbl_buff_size;
                    uint32 prevEntriesInBuffer = _stbl_buff_size - actualEntryIndex;
                    if ((index > actualEntryIndex) && prevEntriesInBuffer)
                    {
                        uint32 entryFallsInPreviousBuffer = _curr_buff_number - entryLoc;

                        if (entryFallsInPreviousBuffer == 1)
                            return (_psampleSizeVec[index%_stbl_buff_size]);
                    }
                }
                _SkipOldEntry = false;
                if (_curr_buff_number != entryLoc)
                {
                    _parsed_entry_cnt = entryLoc * _stbl_buff_size;
                    while (_parsed_entry_cnt <= index)
                        ParseEntryUnit(_parsed_entry_cnt);
                }
            }
            return (_psampleSizeVec[index%_stbl_buff_size]);
        }
        else
        {
            return PV_ERROR;
        }
    }
    return _psampleSizeVec[index];
}

