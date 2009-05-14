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
/*
    This PVA_FF_DecoderConfigDescriptor Class
*/


#define IMPLEMENT_DecoderConfigDescriptor

#include "decoderconfigdescriptor.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<PVA_FF_DecoderSpecificInfo*, OsclMemAllocator> PVA_FF_DecoderSpecificInfoVecType;
// Constructor
PVA_FF_DecoderConfigDescriptor::PVA_FF_DecoderConfigDescriptor(int32 streamType, int32 codecType)
        : PVA_FF_BaseDescriptor(0x04)
{
    _codecType = codecType;
    iCurrFilePos = 0;
    init(streamType);
    recomputeSize();
}

// Destructor
PVA_FF_DecoderConfigDescriptor::~PVA_FF_DecoderConfigDescriptor()
{
    if (_pdecSpecificInfoVec != NULL)
    {
        for (uint32 i = 0; i < _pdecSpecificInfoVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, PVA_FF_DecoderSpecificInfo, (*_pdecSpecificInfoVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_DecoderSpecificInfoVecType, Oscl_Vector, _pdecSpecificInfoVec);
    }
}

void
PVA_FF_DecoderConfigDescriptor::init(int32 mediaType)
{
    _pdecSpecificInfoVec = NULL;

    switch (mediaType)
    {
        case MEDIA_TYPE_AUDIO:
        {
            switch (_codecType)
            {
                case CODEC_TYPE_AAC_AUDIO:
                    _objectTypeIndication = 0x40;
                    break;

                default:
                    _objectTypeIndication = 0xFF;
                    break;
            }
            _streamType = 0x05; // 0x05 for AudioStream

            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DecoderSpecificInfoVecType, (), _pdecSpecificInfoVec);

        }
        break;
        case MEDIA_TYPE_VISUAL:
        {
            switch (_codecType)
            {
                case CODEC_TYPE_BASELINE_H263_VIDEO:
                    _objectTypeIndication = 0xc0; // Simple profile
                    break;

                case CODEC_TYPE_MPEG4_VIDEO:
                    _objectTypeIndication = 0x20; // Simple profile
                    break;

                case CODEC_TYPE_AVC_VIDEO:
                    _objectTypeIndication = 0x30;
                    break;
            }
            _streamType = 0x04; // 0x04 for VisualStream
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DecoderSpecificInfoVecType, (), _pdecSpecificInfoVec);
        }
        break;
        default:
        {
            _objectTypeIndication = 0xFF; // No profile specified
            _streamType = 0x20; // user private
        }
        break;
    }

    _upStream = false;
    _reserved = 1;
    _bufferSizeDB = 0; // Need better default value
    _maxBitrate = 0; // Need better default value
    _avgBitrate = 0; // Need better default value
}

void
PVA_FF_DecoderConfigDescriptor::addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *info)
{
    if (_pdecSpecificInfoVec != NULL)
        _pdecSpecificInfoVec->push_back(info);

    info->setParent(this);
    recomputeSize();

    if (_pparent != NULL)
        _pparent->recomputeSize();
}

PVA_FF_DecoderSpecificInfo *
PVA_FF_DecoderConfigDescriptor::getDecoderSpecificInfo() const
{
    if ((_pdecSpecificInfoVec != NULL) && (_pdecSpecificInfoVec->size() > 0))
    {
        return (*_pdecSpecificInfoVec)[(int32)0];
    }
    return NULL;
}


void
PVA_FF_DecoderConfigDescriptor::nextSampleSize(uint32 size)
{
    if (size > _bufferSizeDB)
    {
        _bufferSizeDB = size;
    }
}

// Rendering the Descriptor in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_DecoderConfigDescriptor::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render attributes of the PVA_FF_BaseDescriptor class
    int32 numBytes = renderBaseDescriptorMembers(fp);

    if (numBytes == 0)
    {
        return false;
    }
    rendered += numBytes;

    if (!PVA_FF_AtomUtils::render8(fp, _objectTypeIndication))
    {
        return false;
    }
    rendered += 1;

    // Pack and render stream type, upstream, and reserved
    uint8 data = _reserved; // Initial data byte including reserved bit set
    data |= ((_streamType & 0x3f) << 2);  // 0b00XXXXXX << 2
    if (_upStream)
    {
        data |= 0x02;
    }
    if (!PVA_FF_AtomUtils::render8(fp, data))
    {
        return false;
    }
    rendered += 1;

    iCurrFilePos = PVA_FF_AtomUtils::getCurrentFilePosition(fp);
    if (!PVA_FF_AtomUtils::render24(fp, _bufferSizeDB))
    {
        return false;
    }
    rendered += 3;

    if (!PVA_FF_AtomUtils::render32(fp, _maxBitrate))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _avgBitrate))
    {
        return false;
    }
    rendered += 4;

    if (_pdecSpecificInfoVec != NULL)
    {
        for (uint32 i = 0; i < _pdecSpecificInfoVec->size(); i++)
        {
            if (!(*_pdecSpecificInfoVec)[i]->renderToFileStream(fp))
            {
                return false;
            }
            rendered += (*_pdecSpecificInfoVec)[i]->getSizeOfDescriptorObject();
        }
    }

    return true;
}


void
PVA_FF_DecoderConfigDescriptor::recomputeSize()
{
    int32 contents = 0;

    contents += 1; // (8 bits for OTI)
    contents += 1; // (8 bits for Stream type, upStream flag and reserved)
    contents += 3; // (24 bits for buffer size DB)
    contents += 4; // (32 bits for max bitrate)
    contents += 4; // (32 bits for avg bitrate)

    if (_pdecSpecificInfoVec != NULL)
    {
        for (uint32 i = 0; i < _pdecSpecificInfoVec->size(); i++)
        {
            contents += (*_pdecSpecificInfoVec)[i]->getSizeOfDescriptorObject();
        }
    }

    _sizeOfClass = contents;
    _sizeOfSizeField = PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(contents);

    // Have the parent descriptor recompute its size based on this update
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }

}

void
PVA_FF_DecoderConfigDescriptor::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    if (NULL != _afp->_filePtr)
    {
        PVA_FF_AtomUtils::seekFromStart(_afp, iCurrFilePos);
        PVA_FF_AtomUtils::render24(_afp, _bufferSizeDB);
    }
}

