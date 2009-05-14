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
#define IMPLEMENT_AVCSampleEntry

#include "avcsampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"


AVCSampleEntry::AVCSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _mp4ErrorCode = READ_AVC_SAMPLE_ENTRY_FAILED;

    uint32 count = _size - DEFAULT_ATOM_SIZE;

    _pAVCConfigurationBox = NULL;
    _pMPEG4BitRateBox     = NULL;
    _decoderSpecificInfo  = NULL;
    _pPASPBox             = NULL ; //PASP box
    if (_success)
    {
        _success = false;

        uint16 data16;
        uint32 data32;

        _pparent = NULL;
        // Read reserved values
        if (!AtomUtils::readByteData(fp, 6, _reserved))
        {
            return;
        }
        count -= 6;

        if (!AtomUtils::read16(fp, _dataReferenceIndex))
        {
            return;
        }
        count -= 2;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _preDefined1 = (int16)data16;
        count -= 2;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _reserved1 = (int16)data16;
        count -= 2;

        // Read in all reserved members
        for (int32 i = 0; i < 3; i++)
        {
            if (!AtomUtils::read32(fp, data32))
            {
                return;
            }
            _predefined2[i] = (int32)data32;
            count -= 4;
        }

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _width = (int16)data16;
        count -= 2;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _height = (int16)data16;
        count -= 2;

        if (!AtomUtils::read32(fp, data32))
        {
            return;
        }
        _horizResolution = (int32)data32;
        count -= 4;

        if (!AtomUtils::read32(fp, data32))
        {
            return;
        }
        _vertResolution = (int32)data32;
        count -= 4;

        if (!AtomUtils::read32(fp, data32))
        {
            return;
        }
        _reserved2 = (int32)data32;
        count -= 4;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _preDefined2 = (int16)data16;
        count -= 2;

        if (!AtomUtils::readByteData(fp, 32, _compressorName))
        {
            return;
        }
        count -= 32;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _depth = (int16)data16;
        count -= 2;

        if (!AtomUtils::read16(fp, data16))
        {
            return;
        }
        _predefined3 = (int16)data16;
        count -= 2;

        while (count >= DEFAULT_ATOM_SIZE)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == AVC_CONFIGURATION_BOX)
            {
                PV_MP4_FF_NEW(fp->auditCB, AVCConfigurationBox, (fp, atomSize, atomType), _pAVCConfigurationBox);

                if (!_pAVCConfigurationBox->MP4Success())
                {
                    _mp4ErrorCode = READ_AVC_CONFIG_BOX_FAILED;
                    return;
                }
                count -= atomSize;
            }
            else if (atomType == MPEG4_BITRATE_BOX)
            {
                PV_MP4_FF_NEW(fp->auditCB, MPEG4BitRateBox, (fp, atomSize, atomType), _pMPEG4BitRateBox);

                if (!_pMPEG4BitRateBox->MP4Success())
                {
                    _mp4ErrorCode = READ_MPEG4_BITRATE_BOX_FAILED;
                    return;
                }
                count -= atomSize;
            }
            else if (atomType == PIXELASPECTRATIO_BOX) //PASP Box
            {
                PV_MP4_FF_NEW(fp->auditCB, PASPBox, (fp, atomSize, atomType), _pPASPBox);

                if (!_pPASPBox->MP4Success())
                {
                    _mp4ErrorCode = READ_PIXELASPECTRATIO_BOX_FAILED;
                    return;
                }
                count -= atomSize;
            }
            else
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    return;
                }
                if (count < (uint32)atomSize)
                {
                    _mp4ErrorCode = READ_FAILED;
                    break;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }

        }
        if (count > 0)
        {
            //skip over any left over bytes
            AtomUtils::seekFromCurrPos(fp, count);
            count = 0;
        }
        if (createDecoderSpecificInfo(fp))
        {
            _success = true;
            _mp4ErrorCode = EVERYTHING_FINE;
        }
    }
}

// Destructor
AVCSampleEntry::~AVCSampleEntry()
{
    if (_pAVCConfigurationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AVCConfigurationBox, _pAVCConfigurationBox);
    }
    if (_pMPEG4BitRateBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MPEG4BitRateBox, _pMPEG4BitRateBox);
    }
    if (_pPASPBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PASPBox, _pPASPBox);
    }
    if (_decoderSpecificInfo != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, _decoderSpecificInfo);
    }
}

bool
AVCSampleEntry::createDecoderSpecificInfo(MP4_FF_FILE *fp)
{
    uint32 numSPS = getNumSequenceParamSets();
    uint32 numPPS = getNumPictureParamSets();
    uint32 totalSPSLen = getTotalSeqParameterSetLength();
    uint32 totalPPSLen = getTotalPictureParameterSetLength();
    uint32 len = (numSPS * 2) + (numPPS * 2) + totalSPSLen + totalPPSLen;

    if ((int32)len > 0)
    {
        PV_MP4_FF_NEW(fp->auditCB, DecoderSpecificInfo, (fp, true), _decoderSpecificInfo);

        uint8* info = (uint8*)(oscl_malloc(sizeof(uint8) * len));
        if (!info)
            return false;	// malloc failed (unlikely)
        uint8* destPtr = info;
        if (numSPS > 0)
        {
            for (uint32 i = 0; i < numSPS; i++)
            {
                uint16 len = 0;
                uint8* ptr = NULL;
                if (getSequenceParamSet(i, len, ptr) == false)
                {
                    OSCL_FREE(info);
                    return false;
                }
                oscl_memcpy(destPtr, &len, sizeof(uint16));
                destPtr += sizeof(uint16);
                oscl_memcpy(destPtr, ptr, len);
                destPtr += len;
            }
        }
        if (numPPS > 0)
        {
            for (uint32 i = 0; i < numPPS; i++)
            {
                uint16 len = 0;
                uint8* ptr = NULL;
                if (getPictureParamSet(i, len, ptr) == false)
                {
                    OSCL_FREE(info);
                    return false;
                }
                oscl_memcpy(destPtr, &len, sizeof(uint16));
                destPtr += sizeof(uint16);
                oscl_memcpy(destPtr, ptr, len);
                destPtr += len;
            }
        }
        _decoderSpecificInfo->setInfoSize(len);
        _decoderSpecificInfo->setInfo(info);
    }
    return true;
}
