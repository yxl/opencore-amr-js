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
#ifndef OMA2BOXES_H_INCLUDED
#include "oma2boxes.h"
#endif

#include "h263decoderspecificinfo.h"
#include "boxrecord.h"
#include "fontrecord.h"
#include "stylerecord.h"
#include "fonttableatom.h"
#include "amrdecoderspecificinfo.h"
#include "h263decoderspecificinfo.h"

typedef Oscl_Vector<DecoderSpecificInfo*, OsclMemAllocator> decoderSpecificInfoVecType;

OMADRMKMSBox:: OMADRMKMSBox(MP4_FF_FILE *fp,
                            uint32 size,
                            uint32 type)
        :	FullAtom(fp, size, type)
{
    _pODKMData = NULL;

    if (!_success)
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_OMADRM_KMS_BOX_FAILED;
        }
        return;
    }

    /* Seek back to atom start */
    AtomUtils::rewindFilePointerByN(fp, DEFAULT_FULL_ATOM_SIZE);

    /* Read the entire contents of the atom into memory */
    _pODKMData = (uint8*)(oscl_malloc(sizeof(uint8) * (_size)));

    if (!AtomUtils::readByteData(fp, _size, _pODKMData))
    {
        _success = false;
        _mp4ErrorCode = READ_OMADRM_KMS_BOX_FAILED;
        return;
    }
    return;
}

OMADRMKMSBox::~OMADRMKMSBox()
{
    if (_pODKMData != NULL)
    {
        oscl_free(_pODKMData);
    }
};

MutableDRMInformationBox:: MutableDRMInformationBox(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        :	Atom(fp, size, type)
{
}

SchemeInformationBox:: SchemeInformationBox(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        :	Atom(fp, size, type)
{
    _pOMADRMKMSBox = NULL;

    if (!_success)
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_SCHEME_INFORMATION_BOX_FAILED;
        }
        return;
    }
    uint32 count = (_size - DEFAULT_ATOM_SIZE);

    while (count > 0)
    {
        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;
        AtomUtils::getNextAtomType(fp, atomSize, atomType);
        if (atomType == OMADRM_KMS_BOX)
        {
            if (_pOMADRMKMSBox == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB, OMADRMKMSBox, (fp, atomSize, atomType), _pOMADRMKMSBox);

                if (!_pOMADRMKMSBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pOMADRMKMSBox->GetMP4Error();
                    break;
                }
                _pOMADRMKMSBox->setParent(this);
            }
            else
            {
                _success = false;
                _mp4ErrorCode = DUPLICATE_OMADRM_KMS_BOX;
                break;
            }
        }
        else
        {
            //skip over
            atomSize -= DEFAULT_ATOM_SIZE;
            AtomUtils::seekFromCurrPos(fp, atomSize);
        }
        count -= atomSize;
    }
    return;
}

SchemeInformationBox::~SchemeInformationBox()
{
    if (_pOMADRMKMSBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, OMADRMKMSBox, _pOMADRMKMSBox);
    }
}

ProtectionSchemeInformationBox:: ProtectionSchemeInformationBox(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        :	Atom(fp, size, type)
{
    _pSchemeInformationBox = NULL;
    _pOriginalFormatBox = NULL;

    if (!_success)
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
        {
            _mp4ErrorCode = READ_SCHEME_INFORMATION_BOX_FAILED;
        }
        return;
    }
    uint32 count = (_size - DEFAULT_ATOM_SIZE);

    while (count > 0)
    {
        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;
        AtomUtils::getNextAtomType(fp, atomSize, atomType);
        if (atomType == SCHEME_INFORMATION_BOX)
        {
            if (_pSchemeInformationBox == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB,
                              SchemeInformationBox,
                              (fp, atomSize, atomType),
                              _pSchemeInformationBox);

                if (!_pSchemeInformationBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pSchemeInformationBox->GetMP4Error();
                    break;
                }
                _pSchemeInformationBox->setParent(this);
            }
            else
            {
                //skip over
                AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
            }
        }

        else if (atomType == ORIGINAL_FORMAT_BOX)
        {
            if (_pOriginalFormatBox == NULL)
            {
                PV_MP4_FF_NEW(fp->auditCB,
                              OriginalFormatbox,
                              (fp, atomSize, atomType),
                              _pOriginalFormatBox);

                if (!_pOriginalFormatBox ->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pOriginalFormatBox ->GetMP4Error();
                    break;
                }
                _pOriginalFormatBox ->setParent(this);
            }
            else
            {
                //skip over
                AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
            }
        }

        else
        {
            //skip over
            AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
        }
        count -= atomSize;
    }
    return;
}

ProtectionSchemeInformationBox::~ProtectionSchemeInformationBox()
{
    if (_pSchemeInformationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, SchemeInformationBox, _pSchemeInformationBox);
    }
    if (_pOriginalFormatBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, OriginalFormatBox, _pOriginalFormatBox);
    }
}

OriginalFormatbox::OriginalFormatbox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        :	Atom(fp, size, type)
{
    if (!AtomUtils::read32(fp, _data_format))
        _success = false;
}

OriginalFormatbox::~OriginalFormatbox()
{
}

EcnaBox::EcnaBox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pes = NULL;
    _pparent = NULL;
    _pProtectionSchemeInformationBox = NULL;
    _pAMRSpecificAtom = NULL;
    _pAMRDecSpecInfoArray = NULL;
    _pAMRWBSpecificAtom = NULL;
    _pAMRWBDecSpecInfoArray = NULL;

    uint32 count = (_size - DEFAULT_ATOM_SIZE);

    if (_success)
    {
        // Read reserved values
        if (!AtomUtils::read32read32(fp, _reserved1[0], _reserved1[1]))
            _success = false;
        count -= 8;

        if (!AtomUtils::read16read16(fp, _reserved2, _reserved3))
            _success = false;
        count -= 4;

        if (!AtomUtils::read32(fp, _reserved4))
            _success = false;
        count -= 4;

        if (!AtomUtils::read16read16(fp, _timeScale, _reserved5))
            _success = false;
        count -= 4;

        if (_success)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            while (count > 0)
            {
                AtomUtils::getNextAtomType(fp, atomSize, atomType);
                if (atomSize > count)
                {
                    _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
                    break;
                }
                count -= atomSize;

                if (atomType == ESD_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, ESDAtom, (fp, atomSize, atomType), _pes);

                    if (!_pes->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _pes->GetMP4Error();
                        break;
                    }
                    else
                    {
                        _pes->setParent(this);
                    }
                }
                else if (atomType == AMR_SPECIFIC_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, AMRSpecificAtom, (fp, atomSize, atomType), _pAMRSpecificAtom);

                    if (!_pAMRSpecificAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
                    }

                    PV_MP4_FF_NEW(fp->auditCB, decoderSpecificInfoVecType, (), _pAMRDecSpecInfoArray);

                    for (uint32 i = 0; i < 16; i++)
                    {
                        AMRDecoderSpecificInfo *pinfo = NULL;
                        PV_MP4_FF_NEW(fp->auditCB, AMRDecoderSpecificInfo, (fp, true), pinfo);

                        pinfo->_frame_type = (uint8)i;
                        pinfo->_codec_version = _pAMRSpecificAtom->getDecoderVersion();
                        pinfo->_mode_change_period = _pAMRSpecificAtom->getModeChangePeriod();
                        pinfo->_mode_set = _pAMRSpecificAtom->getModeSet();
                        pinfo->_mode_change_neighbour = false;

                        (*_pAMRDecSpecInfoArray).push_back(pinfo);
                    }
                }
                else if (atomType == AMRWB_SAMPLE_ENTRY_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, AMRSpecificAtom, (fp, atomSize, atomType), _pAMRWBSpecificAtom);

                    if (!_pAMRWBSpecificAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
                    }

                    PV_MP4_FF_NEW(fp->auditCB, decoderSpecificInfoVecType, (), _pAMRWBDecSpecInfoArray);

                    for (uint32 i = 0; i < 16; i++)
                    {
                        AMRDecoderSpecificInfo *pinfo = NULL;
                        PV_MP4_FF_NEW(fp->auditCB, AMRDecoderSpecificInfo, (fp, true), pinfo);

                        pinfo->_frame_type = (uint8)i;
                        pinfo->_codec_version = _pAMRWBSpecificAtom->getDecoderVersion();
                        pinfo->_mode_change_period = _pAMRWBSpecificAtom->getModeChangePeriod();
                        pinfo->_mode_set = _pAMRWBSpecificAtom->getModeSet();
                        pinfo->_mode_change_neighbour = false;

                        (*_pAMRWBDecSpecInfoArray).push_back(pinfo);
                    }
                }
                else if (atomType == PROTECTION_SCHEME_INFO_BOX)
                {
                    if (_pProtectionSchemeInformationBox == NULL)
                    {
                        PV_MP4_FF_NEW(fp->auditCB,
                                      ProtectionSchemeInformationBox,
                                      (fp, atomSize, atomType),
                                      _pProtectionSchemeInformationBox);

                        if (!_pProtectionSchemeInformationBox->MP4Success())
                        {
                            _success = false;
                            _mp4ErrorCode = _pProtectionSchemeInformationBox->GetMP4Error();
                            break;
                        }
                        _pProtectionSchemeInformationBox->setParent(this);
                    }
                }
                else
                {
                    AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
                }
            }
            if (count > 0)
            {
                AtomUtils::seekFromCurrPos(fp, (count));
            }
        }
        else
        {
            _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
    }

}

uint32 EcnaBox::getESID() const
{
    if (_pes != NULL)
    {
        if (_pes->getESDescriptorPtr() != NULL)
        {
            return _pes->getESDescriptorPtr()->getESID();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

const ESDescriptor* EcnaBox::getESDescriptor() const
{
    if (_pes != NULL)
    {
        return _pes->getESDescriptorPtr();
    }
    else
    {
        return NULL;
    }
}

uint8 EcnaBox::getObjectTypeIndication() const
{
    if (_pes != NULL)
    {
        return _pes->getObjectTypeIndication();
    }
    else if (_pAMRSpecificAtom != NULL)
    {
        return (AMR_AUDIO_3GPP);
    }
    else if (_pAMRWBSpecificAtom != NULL)
    {
        return (AMRWB_AUDIO_3GPP);
    }
    else
    {
        return 0xFF;
    }
}

DecoderSpecificInfo* EcnaBox::getDecoderSpecificInfo() const
{
    uint32 index = 0;
    if (_pes != NULL)
    {
        return _pes->getDecoderSpecificInfo();
    }
    else if (_pAMRSpecificAtom != NULL)
    {
        if ((_pAMRDecSpecInfoArray->size() == 0) ||
                (index >= (uint32)(_pAMRDecSpecInfoArray->size())))
        {
            return NULL;
        }
        else
        {
            return ((*_pAMRDecSpecInfoArray)[index]);
        }
    }
    else if (_pAMRWBSpecificAtom != NULL)
    {
        if ((_pAMRWBDecSpecInfoArray->size() == 0) ||
                (index >= (uint32)(_pAMRWBDecSpecInfoArray->size())))
        {
            return NULL;
        }
        else
        {
            return ((*_pAMRWBDecSpecInfoArray)[index]);
        }
    }
    else
    {
        return NULL;
    }
}

uint32 EcnaBox::getAverageBitrate() const
{
    if (_pes != NULL)
    {
        return _pes->getAverageBitrate();
    }
    else if (_pAMRSpecificAtom != NULL)
    {
        return (_pAMRSpecificAtom->getBitRate());
    }
    else
    {
        return 0;
    }
}

uint32 EcnaBox::getMaxBitrate() const
{
    if (_pes != NULL)
    {
        return _pes->getMaxBitrate();
    }
    else
    {
        return 0;
    }
}

// Get the max size buffer needed to retrieve the media samples
uint32 EcnaBox::getMaxBufferSizeDB() const
{
    if (_pes != NULL)
    {
        return _pes->getMaxBufferSizeDB();
    }
    else
    {
        return 0;
    }
}

// Destructor
EcnaBox::~EcnaBox()
{
    if (_pes != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, ESDAtom, _pes);
    }
    if (_pProtectionSchemeInformationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ProtectionSchemeInformationBox, _pProtectionSchemeInformationBox);
    }
    if (_pAMRSpecificAtom != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, AMRSpecificAtom, _pAMRSpecificAtom);
    }
    if (_pAMRDecSpecInfoArray != NULL)
    {
        for (uint32 i = 0; i < _pAMRDecSpecInfoArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, (*_pAMRDecSpecInfoArray)[i]);
        }
    }

    if (_pAMRDecSpecInfoArray != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, decoderSpecificInfoVecType, Oscl_Vector, _pAMRDecSpecInfoArray);
        _pAMRDecSpecInfoArray = NULL;
    }
}

EcnvBox::EcnvBox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pes = NULL;
    _pProtectionSchemeInformationBox = NULL;
    _pH263SpecificAtom = NULL;
    _pH263decSpecificInfo = NULL;
    _pAVCConfigurationBox = NULL;
    _pMPEG4BitRateBox     = NULL;
    _decoderSpecificInfo  = NULL;

    uint32 count = (_size - DEFAULT_ATOM_SIZE);

    if (_success)
    {
        _pparent = NULL;

        // Read in all reserved members
        for (int32 i = 0; i < 4; i++)
        {
            if (!AtomUtils::read32(fp, _reserved1[i]))
            {
                _success = false;
                break;
            }
            count -= 4;
        }

        if (_success)
        {
            if (!AtomUtils::read32read32(fp, _reserved2, _reserved3))
                _success = false;
            count -= 8;

            if (!AtomUtils::read32read32(fp, _reserved4, _reserved5))
                _success = false;
            count -= 8;

            if (!AtomUtils::read16(fp, _reserved6))
                _success = false;
            count -= 2;

            for (int32 i = 0; i < 32; i++)
            {
                if (!AtomUtils::read8(fp, _reserved7[i]))
                {
                    _success = false;
                    break;
                }
                count -= 1;
            }

            if (!AtomUtils::read16read16(fp, _reserved8, _reserved9))
                _success = false;
            count -= 4;
        }

        if (_success)
        {
            bool avcPresent = false;
            while (count > 0)
            {
                uint32 atomType = UNKNOWN_ATOM;
                uint32 atomSize = 0;

                AtomUtils::getNextAtomType(fp, atomSize, atomType);

                if (atomSize > count)
                {
                    _mp4ErrorCode = READ_VISUAL_SAMPLE_ENTRY_FAILED;
                    break;
                }

                count -= atomSize;

                if (atomType == ESD_ATOM)
                {

                    PV_MP4_FF_NEW(fp->auditCB, ESDAtom, (fp, atomSize, atomType), _pes);
                    if (!_pes->MP4Success())
                    {
                        _mp4ErrorCode = _pes->GetMP4Error();
                        _success = false;
                    }
                    else
                    {
                        if (_pes->getObjectTypeIndication() == H263_VIDEO)
                        {
                            const ESDescriptor *_pdescriptor = _pes->getESDescriptorPtr();
                            if (NULL != _pdescriptor)
                            {
                                H263DecoderSpecificInfo *_pH263decSpecificInfo =
                                    (H263DecoderSpecificInfo *)(_pdescriptor->getDecoderSpecificInfo());
                                if (NULL != _pH263decSpecificInfo)
                                {
                                    if (_pH263decSpecificInfo->_max_height <= 0)
                                        _pH263decSpecificInfo->_max_height = (uint16)((_reserved2 << 16) >> 16);
                                    if (_pH263decSpecificInfo->_max_width <= 0)
                                        _pH263decSpecificInfo->_max_width = (uint16)(_reserved2 >> 16);
                                }
                            }
                        }
                    }
                }
                else if (atomType == H263_SPECIFIC_ATOM)
                {
                    PV_MP4_FF_NEW(fp->auditCB, H263SpecficAtom, (fp, atomSize, atomType), _pH263SpecificAtom);

                    if (_pH263SpecificAtom->MP4Success())
                    {
                        PV_MP4_FF_NEW(fp->auditCB, H263DecoderSpecificInfo, (fp, true), _pH263decSpecificInfo);

                        _pH263decSpecificInfo->_VendorCode = _pH263SpecificAtom->getVendorCode();
                        _pH263decSpecificInfo->_codec_version = _pH263SpecificAtom->getDecoderVersion();
                        _pH263decSpecificInfo->_codec_level = _pH263SpecificAtom->getCodecLevel();
                        _pH263decSpecificInfo->_codec_profile = _pH263SpecificAtom->getCodecProfile();

                        uint32 width = _reserved2 & 0xFFFF0000;
                        _pH263decSpecificInfo->_max_width = width >> 16;
                        _pH263decSpecificInfo->_max_height = _reserved2 & 0x0000FFFF;
                    }

                }
                else if (atomType == AVC_CONFIGURATION_BOX)
                {
                    avcPresent = true;
                    PV_MP4_FF_NEW(fp->auditCB, AVCConfigurationBox, (fp, atomSize, atomType), _pAVCConfigurationBox);

                    if (!_pAVCConfigurationBox->MP4Success())
                    {
                        _mp4ErrorCode = READ_AVC_CONFIG_BOX_FAILED;
                        return;
                    }
                }
                else if (atomType == MPEG4_BITRATE_BOX)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MPEG4BitRateBox, (fp, atomSize, atomType), _pMPEG4BitRateBox);

                    if (!_pMPEG4BitRateBox->MP4Success())
                    {
                        _mp4ErrorCode = READ_MPEG4_BITRATE_BOX_FAILED;
                        return;
                    }
                }
                else if (atomType == PROTECTION_SCHEME_INFO_BOX)
                {
                    if (_pProtectionSchemeInformationBox == NULL)
                    {
                        PV_MP4_FF_NEW(fp->auditCB,
                                      ProtectionSchemeInformationBox,
                                      (fp, atomSize, atomType),
                                      _pProtectionSchemeInformationBox);

                        if (!_pProtectionSchemeInformationBox->MP4Success())
                        {
                            _success = false;
                            _mp4ErrorCode = _pProtectionSchemeInformationBox->GetMP4Error();
                            break;
                        }
                        _pProtectionSchemeInformationBox->setParent(this);
                    }
                }

                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_VISUAL_SAMPLE_ENTRY_FAILED;
                }

                if (avcPresent)
                {
                    avcPresent = false;
                    if (createAVCDecoderSpecificInfo(fp))
                    {
                        _success = true;
                        _mp4ErrorCode = EVERYTHING_FINE;
                    }
                }
            }
            if (count > 0)
            {
                AtomUtils::seekFromCurrPos(fp, (count));
            }

        }
        else
        {
            _mp4ErrorCode = READ_VISUAL_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_VISUAL_SAMPLE_ENTRY_FAILED;
    }

}

bool
EcnvBox::createAVCDecoderSpecificInfo(MP4_FF_FILE *fp)
{
    if (_pAVCConfigurationBox == NULL)
        return false;

    uint32 numSPS = _pAVCConfigurationBox->getNumSequenceParamSets();
    uint32 numPPS = _pAVCConfigurationBox->getNumPictureParamSets();
    uint32 totalSPSLen = _pAVCConfigurationBox->getTotalSeqParameterSetLength();
    uint32 totalPPSLen = _pAVCConfigurationBox->getTotalPictureParameterSetLength();
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
                if (_pAVCConfigurationBox->getSequenceParamSet(i, len, ptr) == false)
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
                if (_pAVCConfigurationBox->getPictureParamSet(i, len, ptr) == false)
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

uint32 EcnvBox::getESID() const
{
    if (_pes != NULL)
    {
        if (_pes->getESDescriptorPtr() != NULL)
        {
            return _pes->getESDescriptorPtr()->getESID();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

const ESDescriptor* EcnvBox::getESDescriptor() const
{
    if (_pes != NULL)
    {
        return _pes->getESDescriptorPtr();
    }
    else
    {
        return NULL;
    }
}

// Getting and setting the Mpeg4 VOL header
DecoderSpecificInfo* EcnvBox::getDecoderSpecificInfo() const
{
    if (_pes != NULL)
    {
        return _pes->getDecoderSpecificInfo();
    }
    else if (_pH263decSpecificInfo != NULL)
    {
        return _pH263decSpecificInfo;
    }
    else if (_pAVCConfigurationBox != NULL)
    {
        if (_decoderSpecificInfo != NULL)
            return _decoderSpecificInfo;
        else
            return NULL;
    }
    else
    {
        return NULL;
    }
}

uint8 EcnvBox::getObjectTypeIndication() const
{
    if (_pes != NULL)
    {
        return _pes->getObjectTypeIndication();
    }
    else
    {
        if (_pH263SpecificAtom != NULL)
            return (H263_VIDEO);
        else if (_pAVCConfigurationBox != NULL)
            return (AVC_VIDEO);
        else
            return 0xFF;
    }
}


// Get the max size buffer needed to retrieve the media samples
uint32 EcnvBox::getMaxBufferSizeDB() const
{
    if (_pes != NULL)
    {
        return _pes->getMaxBufferSizeDB();
    }
    else if (_pMPEG4BitRateBox != NULL)
    {
        return _pMPEG4BitRateBox->getBufferSizeDB();
    }
    else
    {
        return 0;
    }
}

uint32 EcnvBox::getAverageBitrate() const
{
    if (_pes != NULL)
    {
        return _pes->getAverageBitrate();
    }
    else if (_pH263SpecificAtom != NULL)
    {
        return (_pH263SpecificAtom->getAverageBitRate());
    }
    else if (_pMPEG4BitRateBox != NULL)
    {
        return _pMPEG4BitRateBox->getAvgBitRate();
    }
    else
    {
        return 0;
    }
}

uint32 EcnvBox::getMaxBitrate() const
{
    if (_pes != NULL)
    {
        return _pes->getMaxBitrate();
    }
    else if (_pH263SpecificAtom != NULL)
    {
        return (_pH263SpecificAtom->getMaxBitRate());
    }
    else if (_pMPEG4BitRateBox != NULL)
    {
        return _pMPEG4BitRateBox->getMaxBitRate();
    }
    else
    {
        return 0;
    }
}

uint16 EcnvBox::getWidth() const
{
    uint32 width = _reserved2 & 0xFFFF0000;
    uint32 ret = width >> 16;
    return (uint16)ret;

}

uint16 EcnvBox::getHeight() const
{
    return (uint16)(_reserved2 & 0x0000FFFF);
}


// Destructor
EcnvBox::~EcnvBox()
{
    if (_pes != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, ESDAtom, _pes);
    }
    if (_pH263SpecificAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, H263SpecficAtom, _pH263SpecificAtom);
    }

    if (_pH263decSpecificInfo != NULL)
    {
        PV_MP4_FF_DELETE(NULL, H263DecoderSpecificInfo, _pH263decSpecificInfo);
    }
    if (_pAVCConfigurationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, AVCConfigurationBox, _pAVCConfigurationBox);
    }
    if (_pMPEG4BitRateBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MPEG4BitRateBox, _pMPEG4BitRateBox);
    }
    if (_decoderSpecificInfo != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, _decoderSpecificInfo);
    }
    if (_decoderSpecificInfo != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, _decoderSpecificInfo);
    }
    if (_pProtectionSchemeInformationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ProtectionSchemeInformationBox, _pProtectionSchemeInformationBox);
    }
}


OSCL_EXPORT_REF EnctBox:: EnctBox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pparent = NULL;

    _pBackgroundRGBA = NULL;
    _pBoxRecord  	 = NULL;
    _pStyleRecord	 = NULL;
    _pFontTableAtom  = NULL;
    _pProtectionSchemeInformationBox = NULL;

    uint32 count = (_size - DEFAULT_ATOM_SIZE);

    if (_success)
    {

        AtomUtils::read32(fp, _displayFlags);
        count -= 4;

        uint8 horz = 0;
        AtomUtils::read8(fp, horz);
        count -= 1;

        _horzJustification = (int8)horz;

        uint8 vert = 0;
        AtomUtils::read8(fp, vert);
        count -= 1;
        _vertJustification = (int8)vert;

        PV_MP4_FF_ARRAY_NEW(NULL, uint8, (4), _pBackgroundRGBA);

        AtomUtils::readByteData(fp, 4, _pBackgroundRGBA);
        count -= 4;

        uint32 fpos1 = AtomUtils::getCurrentFilePosition(fp);
        PV_MP4_FF_NEW(fp->auditCB, BoxRecord, (fp), _pBoxRecord);
        if (!_pBoxRecord->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pBoxRecord->GetMP4Error();
            return;
        }
        uint32 fpos2 = AtomUtils::getCurrentFilePosition(fp);
        count -= (fpos2 - fpos1);


        fpos1 = AtomUtils::getCurrentFilePosition(fp);
        PV_MP4_FF_NEW(fp->auditCB, StyleRecord, (fp), _pStyleRecord);
        if (!_pStyleRecord->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pStyleRecord->GetMP4Error();
            return;
        }
        fpos2 = AtomUtils::getCurrentFilePosition(fp);
        count -= (fpos2 - fpos1);

        PV_MP4_FF_NEW(fp->auditCB, FontTableAtom, (fp), _pFontTableAtom);
        if (!_pFontTableAtom->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pFontTableAtom->GetMP4Error();
            return;
        }
        count -= _pFontTableAtom->getSize();

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        while (count > 0)
        {
            AtomUtils::getNextAtomType(fp, atomSize, atomType);
            if (atomSize > count)
            {
                _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
                break;
            }
            count -= atomSize;

            if (atomType == PROTECTION_SCHEME_INFO_BOX)
            {
                if (_pProtectionSchemeInformationBox == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB,
                                  ProtectionSchemeInformationBox,
                                  (fp, atomSize, atomType),
                                  _pProtectionSchemeInformationBox);

                    if (!_pProtectionSchemeInformationBox->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _pProtectionSchemeInformationBox->GetMP4Error();
                        break;
                    }
                    _pProtectionSchemeInformationBox->setParent(this);
                }
            }
            else
            {
                AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
            }
        }
        if (count > 0)
        {
            AtomUtils::seekFromCurrPos(fp, (count));
        }

    }
}

// Destructor
OSCL_EXPORT_REF EnctBox::~EnctBox()
{
    if (_pBackgroundRGBA != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pBackgroundRGBA);
    }
    if (_pBoxRecord != NULL)
    {
        PV_MP4_FF_DELETE(NULL, BoxRecord, _pBoxRecord);
    }
    if (_pStyleRecord != NULL)
    {
        PV_MP4_FF_DELETE(NULL, StyleRecord, _pStyleRecord);
    }
    if (_pFontTableAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, FontTableAtom, _pFontTableAtom);
    }
    if (_pProtectionSchemeInformationBox != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ProtectionSchemeInformationBox, _pProtectionSchemeInformationBox);
    }

}

OSCL_EXPORT_REF int16 EnctBox::getBoxTop()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxTop());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 EnctBox::getBoxLeft()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxLeft());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 EnctBox::getBoxBottom()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxBottom());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 EnctBox::getBoxRight()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxRight());
    }
    else
    {
        return -1;
    }
}
OSCL_EXPORT_REF uint16 EnctBox::getFontID()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getFontID();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint16 EnctBox::getStartChar()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getStartChar();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint16 EnctBox::getEndChar()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getEndChar();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8 EnctBox::getFontStyleFlags()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getFontStyleFlags();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8 EnctBox::getfontSize()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord->getfontSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8* EnctBox::getTextColourRGBA()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getTextColourRGBA();
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF uint16 EnctBox:: getFontListSize()
{
    if (_pFontTableAtom != NULL)
    {
        return _pFontTableAtom-> getFontListSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF FontRecord* EnctBox::getFontRecordAt(uint16 index)
{
    if (_pFontTableAtom != NULL)
    {
        return _pFontTableAtom->getFontRecordAt(index);
    }
    else
    {
        return NULL;
    }
}









