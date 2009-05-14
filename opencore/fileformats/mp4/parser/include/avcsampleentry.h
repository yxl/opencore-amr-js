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
#ifndef __AVCSampleEntry_H__
#define __AVCSampleEntry_H__

#include "atom.h"
#include "oscl_file_io.h"
#include "avcconfigurationbox.h"
#include "mpeg4sampleentryextensions.h"
#include "decoderspecificinfo.h"

#ifndef PASPBOX_H_INCLUDED
#include "paspatom.h"
#endif

class AVCSampleEntry : public Atom
{

    public:
        AVCSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AVCSampleEntry();

        int16  getWidth()
        {
            return (_width);
        }

        int16  getHeight()
        {
            return (_height);
        }

        /* PASP */

        uint32 getHspacing()
        {
            if (_pPASPBox != NULL)
            {
                return _pPASPBox->getHspacing();
            }
            return 0;
        }

        uint32 getVspacing()
        {
            if (_pPASPBox != NULL)
            {
                return _pPASPBox->getVspacing();
            }
            return 0;
        }

        uint32 getBufferSizeDB()
        {
            if (_pMPEG4BitRateBox != NULL)
            {
                return _pMPEG4BitRateBox->getBufferSizeDB();
            }
            return 0;
        }

        uint32 getMaxBitRate()
        {
            if (_pMPEG4BitRateBox != NULL)
            {
                return _pMPEG4BitRateBox->getMaxBitRate();
            }
            return 0;
        }

        uint32 getAvgBitRate()
        {
            if (_pMPEG4BitRateBox != NULL)
            {
                return _pMPEG4BitRateBox->getAvgBitRate();
            }
            return 0;
        }

        uint8 getConfigurationVersion()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getConfigurationVersion());
            }
            else
            {
                //Undefined value - returning zero could be misleading
                return (0xFF);
            }
        }

        uint8 getAVCProfileIndication()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getAVCProfileIndication());
            }
            else
            {
                //Undefined value - returning zero could be misleading
                return (0xFF);
            }
        }

        uint8 getProfileCompatibility()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getProfileCompatibility());
            }
            else
            {
                //Undefined value - returning zero could be misleading
                return (0xFF);
            }
        }

        uint8 getAVCLevelIndication()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getAVCLevelIndication());
            }
            else
            {
                //Undefined value - returning zero could be misleading
                return (0xFF);
            }
        }

        uint8 getNALLengthSize()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getNALLengthSize());
            }
            else
            {
                return (0);
            }
        }

        uint8 getNumSequenceParamSets()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getNumSequenceParamSets());
            }
            else
            {
                return (0);
            }
        }

        uint8 getNumPictureParamSets()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getNumPictureParamSets());
            }
            else
            {
                return (0);
            }
        }

        bool getSequenceParamSet(int32 index, uint16 &length, uint8* &paramSet)
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getSequenceParamSet(index, length, paramSet));
            }
            else
            {
                return (false);
            }
        }

        bool getPictureParamSet(int32 index, uint16 &length, uint8* &paramSet)
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return (_pAVCConfigurationBox->getPictureParamSet(index, length, paramSet));
            }
            else
            {
                return (false);
            }
        }

        uint32 getNumofPictureParamSet()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return _pAVCConfigurationBox->getNumofPictureParamSet();
            }
            return 0;
        }

        uint32 getNumofSequenceParamSet()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return _pAVCConfigurationBox->getNumofSequenceParamSet();
            }
            return 0;
        }

        uint32 getTotalSeqParameterSetLength()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return _pAVCConfigurationBox->getTotalSeqParameterSetLength();
            }
            return 0;
        }

        uint32 getTotalPictureParameterSetLength()
        {
            if (_pAVCConfigurationBox != NULL)
            {
                return _pAVCConfigurationBox->getTotalPictureParameterSetLength();
            }
            return 0;
        }

        DecoderSpecificInfo* getDecoderSpecificInfo()
        {
            return _decoderSpecificInfo;
        }

    private:

        bool createDecoderSpecificInfo(MP4_FF_FILE *fp);

        // Reserved constants
        uint8  _reserved[6];
        uint16 _dataReferenceIndex;
        int16  _preDefined1;
        int16  _reserved1;
        int32  _predefined2[3];
        int16  _width;
        int16  _height;
        int32  _horizResolution;
        int32  _vertResolution;
        int32  _reserved2;
        int16  _preDefined2;
        uint8  _compressorName[32];
        int16  _depth;
        int16  _predefined3;
        DecoderSpecificInfo* _decoderSpecificInfo;

        AVCConfigurationBox          *_pAVCConfigurationBox;
        MPEG4BitRateBox              *_pMPEG4BitRateBox;
        PASPBox                      *_pPASPBox;
};


#endif
