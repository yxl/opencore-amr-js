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
#define IMPLEMENT_InterleaveBuffer

#include "interleavebuffer.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<uint8, OsclMemAllocator> uint8VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;

PVA_FF_InterLeaveBuffer::PVA_FF_InterLeaveBuffer(uint32 mediaType,
        uint32 codecType,
        uint32 trackId)
{
    _trackId = trackId;
    _mediaType = mediaType;
    _codecType = codecType;
    _lastChunkEndTime			=	0;
    _maxInterLeaveBufferSize	=	0;
    _currInterLeaveBufferSize	=	0;
    _lastInterLeaveBufferTS		=	0;
    _lastSampleTS				=	0;


    // initialise interleaved buffers for audio or video track( max size)
    if ((uint32) mediaType == MEDIA_TYPE_AUDIO)
    {
        if (codecType == CODEC_TYPE_AMR_AUDIO)
        {
            _interLeaveBuffer =
                (uint8 *)(oscl_malloc(sizeof(uint8) * AMR_INTERLEAVE_BUFFER_SIZE));

            _maxInterLeaveBufferSize = AMR_INTERLEAVE_BUFFER_SIZE;
        }
        else if (codecType == CODEC_TYPE_AAC_AUDIO)
        {
            _interLeaveBuffer =
                (uint8 *)(oscl_malloc(sizeof(uint8) * AAC_INTERLEAVE_BUFFER_SIZE));

            _maxInterLeaveBufferSize = AAC_INTERLEAVE_BUFFER_SIZE;
        }
        else if (codecType == CODEC_TYPE_AMR_WB_AUDIO)
        {
            _interLeaveBuffer =
                (uint8 *)(oscl_malloc(sizeof(uint8) * AMR_WB_INTERLEAVE_BUFFER_SIZE));

            _maxInterLeaveBufferSize = AMR_WB_INTERLEAVE_BUFFER_SIZE;
        }
    }

    if ((uint32) mediaType == MEDIA_TYPE_VISUAL)
    {
        _interLeaveBuffer = (uint8 *)(oscl_malloc(sizeof(uint8) * VIDEO_INTERLEAVE_BUFFER_SIZE));

        _maxInterLeaveBufferSize = VIDEO_INTERLEAVE_BUFFER_SIZE;
    }

    if ((uint32) _mediaType == MEDIA_TYPE_TEXT)
    {
        _interLeaveBuffer = (uint8 *)(oscl_malloc(sizeof(uint8) * TEXT_INTERLEAVE_BUFFER_SIZE));

        _maxInterLeaveBufferSize = TEXT_INTERLEAVE_BUFFER_SIZE;
    }

    // initialise vectors to store sample parameters present in interleaved buffers
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pTimeStampVec);
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pSampleSizeVec);
    PV_MP4_FF_NEW(fp->auditCB, uint8VecType, (), _pSampleFlagsVec);
    PV_MP4_FF_NEW(fp->auditCB, int32VecType, (), _pIndexVec);

}

PVA_FF_InterLeaveBuffer::~PVA_FF_InterLeaveBuffer()
{
    // free interleave buffer and sample parameter vectors
    if (_interLeaveBuffer != NULL)
        oscl_free(_interLeaveBuffer);
    if (_pTimeStampVec != NULL)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pTimeStampVec);
    if (_pSampleSizeVec)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pSampleSizeVec);
    if (_pSampleFlagsVec)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint8VecType, Oscl_Vector, _pSampleFlagsVec);
    if (_pIndexVec)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, int32VecType, Oscl_Vector, _pIndexVec);
}

// given sample is added to interleave buffer and
// sample parameters stored in parameter vectors
bool
PVA_FF_InterLeaveBuffer::addSampleToInterLeaveBuffer(
    Oscl_Vector < OsclMemoryFragment,
    OsclMemAllocator > & fragmentList,
    uint32 size, uint32 ts, uint8 flags, int32 index)
{
    // temporary variables
    uint32 bytesWritten = 0;
    uint32 length = 0;
    uint32 ii = 0;
    OsclBinIStreamBigEndian stream;

    if (_interLeaveBuffer != NULL)
    {
        uint8* currPtr = _interLeaveBuffer + _currInterLeaveBufferSize;

        if (checkInterLeaveBufferSpace(size))
        {
            if (_mediaType == MEDIA_TYPE_VISUAL && _codecType == CODEC_TYPE_AVC_VIDEO)
            {
                for (ii = 0; ii < fragmentList.size(); ii++)
                {
                    // read NAL length in Big Endian format
                    stream.Attach((OsclAny*) &(fragmentList[ii].len), 4);
                    stream >> length;

                    // compose nal length in two bytes
                    oscl_memcpy((OsclAny*)(currPtr + bytesWritten), ((uint8*)&length), 4);

                    // write NAL uint
                    oscl_memcpy((OsclAny*)(currPtr + bytesWritten + 4), (OsclAny*)fragmentList[ii].ptr, fragmentList[ii].len);
                    bytesWritten += (fragmentList[ii].len + 4);
                }
            }
            else
            {
                for (ii = 0; ii < fragmentList.size(); ii++)
                {
                    oscl_memcpy(currPtr + bytesWritten, fragmentList[ii].ptr, fragmentList[ii].len);
                    bytesWritten += fragmentList[ii].len;
                }
            }

            _currInterLeaveBufferSize += size;
            _lastInterLeaveBufferTS = ts;

            //Store meta data params
            _pTimeStampVec->push_back(ts);
            _pSampleSizeVec->push_back(size);
            _pSampleFlagsVec->push_back(flags);
            _pIndexVec->push_back(index);

            return true;
        }
    }

    return false;
}


// returns false if interleave buffer does not have free space = size
bool
PVA_FF_InterLeaveBuffer::checkInterLeaveBufferSpace(uint32 size)
{
    if ((_currInterLeaveBufferSize + size) > _maxInterLeaveBufferSize)
    {
        return false;
    }
    return true;
}

Oscl_Vector<uint32, OsclMemAllocator>*
PVA_FF_InterLeaveBuffer::getTimeStampVec()
{
    return _pTimeStampVec;
}

Oscl_Vector<uint32, OsclMemAllocator>*
PVA_FF_InterLeaveBuffer::getSampleSizeVec()
{
    return _pSampleSizeVec;

}

Oscl_Vector<uint8, OsclMemAllocator>*
PVA_FF_InterLeaveBuffer::getFlagsVec()
{
    return _pSampleFlagsVec;
}

Oscl_Vector<int32, OsclMemAllocator>*
PVA_FF_InterLeaveBuffer::getTextIndexVec()
{
    return _pIndexVec;
}


// reset interleave buffer, reset parameter vectors
uint8*
PVA_FF_InterLeaveBuffer::resetInterLeaveBuffer(uint32 &chunkSize)
{
    chunkSize = _currInterLeaveBufferSize;
    // reset params
    _currInterLeaveBufferSize = 0;
    _lastInterLeaveBufferTS   = 0;

    //Delete and recreate the vectors

    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pTimeStampVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pSampleSizeVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint8VecType, Oscl_Vector, _pSampleFlagsVec);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, int32VecType, Oscl_Vector, _pIndexVec);

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pTimeStampVec);
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pSampleSizeVec);
    PV_MP4_FF_NEW(fp->auditCB, uint8VecType, (), _pSampleFlagsVec);
    PV_MP4_FF_NEW(fp->auditCB, int32VecType, (), _pIndexVec);

    return (_interLeaveBuffer);
}


// return size of interleave buffer
uint32
PVA_FF_InterLeaveBuffer::getCurrentInterLeaveBufferSize()
{
    return (_currInterLeaveBufferSize);
}


uint32
PVA_FF_InterLeaveBuffer::getTrackID()
{
    return _trackId;
}



// retuen last trun end time value
uint32
PVA_FF_InterLeaveBuffer::getLastChunkEndTime()
{
    return _lastChunkEndTime;
}

uint32
PVA_FF_InterLeaveBuffer::getLastSampleTS()
{
    return _lastSampleTS;
}

// set last trun end time value
void
PVA_FF_InterLeaveBuffer::setLastChunkEndTime()
{
    _lastChunkEndTime = _lastInterLeaveBufferTS;
}


// set last trun end time value = given time
void
PVA_FF_InterLeaveBuffer::setLastChunkEndTime(uint32 time)
{
    _lastChunkEndTime = time;
    if (_pTimeStampVec->size() > 0)
    {
        _lastSampleTS  = _pTimeStampVec->back();
    }
    else
    {
        _lastSampleTS = 0;
    }


}


uint32
PVA_FF_InterLeaveBuffer::getFirstTSEntry()
{
    if (_pTimeStampVec->size() > 0)
    {
        return (*_pTimeStampVec)[0];
    }
    return 0;
}
