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
/*                         MPEG-4 ChunkOffsetAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ChunkOffsetAtom Class gives the index of each chunk into the
    containing FILE.
*/


#define IMPLEMENT_ChunkOffsetAtom

#include "chunkoffsetatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Ctor to create a new ChunkOffsetAtom by reading in from an ifstream
ChunkOffsetAtom::ChunkOffsetAtom(MP4_FF_FILE *fp, uint32 size, uint32 type,
                                 OSCL_wString& filename,
                                 uint32 parsingMode)
        : FullAtom(fp, size, type)
{
    _pchunkOffsets = NULL;
    _stbl_buff_size = MAX_CACHED_TABLE_ENTRIES_FILE;
    _next_buff_number = 0;
    _curr_buff_number = 0;
    _curr_entry_point = 0;
    _stbl_fptr_vec = NULL;
    _parsed_entry_cnt = 0;
    _parsingMode = parsingMode;
    _fileptr = NULL;

    if (_success)
    {
        _currentDataOffset = 0;
        if (!AtomUtils::read32(fp, _entryCount))
        {
            _success = false;
        }

        uint32 dataSize = _size - (DEFAULT_FULL_ATOM_SIZE + 4);

        uint32 entrySize = 4;

        if ((_entryCount*entrySize) > dataSize)
        {
            _success = false;
        }

        if (_success)
        {
            if (_entryCount > 0)
            {
                if (_parsingMode == 1)
                {
                    // cache size is 4K so that optimization should work if entry_count is greater than 4K
                    if ((_entryCount > _stbl_buff_size))
                    {
                        uint32 fptrBuffSize = (_entryCount / _stbl_buff_size) + 1;

                        PV_MP4_FF_ARRAY_NEW(NULL, uint32, (fptrBuffSize), _stbl_fptr_vec);
                        if (_stbl_fptr_vec == NULL)
                        {
                            _success = false;
                            _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                            return;
                        }

                        PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_stbl_buff_size), _pchunkOffsets);
                        if (_pchunkOffsets == NULL)
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
                        _parsingMode = 0;
                        _stbl_buff_size = _entryCount;
                    }

                }
                else
                {
                    _stbl_buff_size = _entryCount;
                }

                _parsingMode = 0;
                PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_entryCount), _pchunkOffsets);

                uint32 offset = 0;
                for (uint32 i = 0; i < _entryCount; i++)
                {
                    if (!AtomUtils::read32(fp, offset))
                    {
                        _success = false;
                        break;
                    }
                    _pchunkOffsets[i] = offset;
                }
                _parsed_entry_cnt = _entryCount;
            }
            else
            {
                _pchunkOffsets = NULL;
            }

        }

        if (!_success)
        {
            _mp4ErrorCode = READ_CHUNK_OFFSET_ATOM_FAILED;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_CHUNK_OFFSET_ATOM_FAILED;
    }
}

// Destructor
ChunkOffsetAtom::~ChunkOffsetAtom()
{
    if (_pchunkOffsets != NULL)
    {
        // Cleanup vector
        PV_MP4_ARRAY_DELETE(NULL, _pchunkOffsets);
    }
    if (_parsingMode)
    {
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
}

bool ChunkOffsetAtom::ParseEntryUnit(uint32 sample_cnt)
{

    if (_parsingMode)
    {
        const uint32 threshold = 512;
        sample_cnt += threshold;

        if (sample_cnt > _entryCount)
            sample_cnt = _entryCount;

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

            uint32 offset = 0;
            if (!AtomUtils::read32(_fileptr, offset))
            {
                return false;
            }
            _pchunkOffsets[_curr_entry_point] = offset;
            _parsed_entry_cnt++;
        }
    }
    return true;
}


// Returns the chunk offset for the chunk at 'index'
// In this case, 'index' is the actual chunk number
int32
ChunkOffsetAtom::getChunkOffsetAt(int32 index)
{
    if (_pchunkOffsets == NULL)
    {
        return PV_ERROR;
    }

    if (index < (int32)_entryCount)
    {
        if (_parsingMode == 1)
        {
            if ((uint32)index >= _parsed_entry_cnt)
            {
                ParseEntryUnit(index);
            }
            else
            {
                uint32 entryLoc = index / _stbl_buff_size;
                if (_curr_buff_number != entryLoc)
                {
                    _parsed_entry_cnt = entryLoc * _stbl_buff_size;
                    while (_parsed_entry_cnt <= (uint32)index)
                        ParseEntryUnit(_parsed_entry_cnt);
                }
            }
            return (_pchunkOffsets[index%_stbl_buff_size]);
        }
        else
        {
            return (_pchunkOffsets[index]);
        }
    }
    else
    {
        return PV_ERROR;
    }
}

int32
ChunkOffsetAtom::getChunkClosestToOffset(uint32 offSet, int32& index)
{
    index = -1;

    if (_pchunkOffsets == NULL)
    {
        return PV_ERROR;
    }

    uint32 prevIndex = 0;
    for (uint32 i = 0; i < _entryCount; i++)
    {
        if (_parsingMode == 1)
        {
            if (i >= _parsed_entry_cnt)
            {
                ParseEntryUnit(i);
            }
            else
            {
                uint32 entryLoc = i / _stbl_buff_size;
                if (_curr_buff_number != entryLoc)
                {
                    _parsed_entry_cnt = entryLoc * _stbl_buff_size;
                    while (_parsed_entry_cnt <= i)
                        ParseEntryUnit(_parsed_entry_cnt);
                }
            }
        }
        if ((uint32)_pchunkOffsets[i%_stbl_buff_size] < offSet)
        {
            prevIndex = i;
            continue;
        }
        else
        {
            index = prevIndex;
            return (EVERYTHING_FINE);
        }
    }
    return (INSUFFICIENT_DATA);
}



