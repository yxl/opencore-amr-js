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
    This PVA_FF_SLConfigDescriptor (SyncLayerDescriptor) Class
*/


#ifndef __SLConfigDescriptor_H__
#define __SLConfigDescriptor_H__

#include "basedescriptor.h"


class PVA_FF_SLConfigDescriptor : public PVA_FF_BaseDescriptor
{

    public:
        PVA_FF_SLConfigDescriptor(); // Constructor
        virtual ~PVA_FF_SLConfigDescriptor();

        void init();

        // ALL the member gets and sets
        // if(_predefined == 0x00)
        bool getUseAccessUnitStartFlag() const
        {
            return _useAccessUnitStartFlag;
        }
        bool getUseAccessUnitEndFlag() const
        {
            return _useAccessUnitEndFlag;
        }
        bool getUseRandomAccessPointFlag() const
        {
            return _useRandomAccessPointFlag;
        }
        bool getUseRandomAccessUnitsOnlyFlag() const
        {
            return _useRandomAccessUnitsOnlyFlag;
        }
        bool getUsePaddingFlag() const
        {
            return _usePaddingFlag;
        }
        bool getUseTimeStampsFlag() const
        {
            return _useTimeStampsFlag;
        }
        bool getUseIdleFlag() const
        {
            return _useIdleFlag;
        }
        bool getDurationFlag() const
        {
            return _durationFlag;
        }
        void setUseAccessUnitStartFlag(bool flag)
        {
            _useAccessUnitStartFlag = flag;
        }
        void setUseAccessUnitEndFlag(bool flag)
        {
            _useAccessUnitEndFlag = flag;
        }
        void setUseRandomAccessPointFlag(bool flag)
        {
            _useRandomAccessPointFlag = flag;
        }
        void setUseRandomAccessUnitsOnlyFlag(bool flag)
        {
            _useRandomAccessUnitsOnlyFlag = flag;
        }
        void setUsePaddingFlag(bool flag)
        {
            _usePaddingFlag = flag;
        }
        void setUseTimeStampsFlag(bool flag)
        {
            _useTimeStampsFlag = flag;
        }
        void setUseIdleFlag(bool flag)
        {
            _useIdleFlag = flag;
        }
        void setDurationFlag(bool flag)
        {
            _durationFlag = flag;
        }

        uint32 getTimeStampResolution() const
        {
            return _timeStampResolution;
        }
        uint32 getOCRResolution() const
        {
            return _OCRResolution;
        }
        uint8 getTimeStampLength() const
        {
            return _timeStampLength;
        }
        uint8 getOCRLength() const
        {
            return _OCRLength;
        }
        uint8 getAULength() const
        {
            return _AULength;
        }
        uint8 getInstantBitrateLength() const
        {
            return _instantBitrateLength;
        }
        uint8 getDegradationPriorityLength() const
        {
            return _degradationPriorityLength;
        }
        uint8 getAUSeqNumLength() const
        {
            return _AUSeqNumLength;
        }
        uint8 getPacketSeqNumLength() const
        {
            return _packetSeqNumLength;
        }
        void setTimeStampResolution(uint32 resolution)
        {
            _timeStampResolution = resolution;
        }
        void setOCRResolution(uint32 resolution)
        {
            _OCRResolution = resolution;
        }
        void setTimeStampLength(uint8 length)
        {
            _timeStampLength = length;
        }
        void setOCRLength(uint8 length)
        {
            _OCRLength = length;
        }
        void setAULength(uint8 length)
        {
            _AULength = length;
        }
        void setInstantBitrateLength(uint8 length)
        {
            _instantBitrateLength = length;
        }
        void setDegradationPriorityLength(uint8 length)
        {
            _degradationPriorityLength = length;
        }
        void setAUSeqNumLength(uint8 length)
        {
            _AUSeqNumLength = length;
        }
        void setPacketSeqNumLength(uint8 length)
        {
            _packetSeqNumLength = length;
        }

        uint32 getTimeScale() const
        {
            return _timeScale;
        }
        uint16 getSccessUnitDuration() const
        {
            return _accessUnitDuration;
        }
        uint16 getCompositionUnitDuration() const
        {
            return _compositionUnitDuration;
        }
        void setTimeScale(uint32 ts)
        {
            _timeScale = ts;
        }
        void setSccessUnitDuration(uint16 duration)
        {
            _accessUnitDuration = duration;
        }
        void setCompositionUnitDuration(uint16 duration)
        {
            _compositionUnitDuration = duration;
        }

        uint64 getStartDecodingTimeStamp() const
        {
            return _startDecodingTimeStamp;
        }
        uint64 getStartCompositionTimeStamp() const
        {
            return _startCompositionTimeStamp;
        }
        void setStartDecodingTimeStamp(uint64 ts)
        {
            ts = _startDecodingTimeStamp;
        }
        void setStartCompositionTimeStamp(uint64 ts)
        {
            ts = _startCompositionTimeStamp;
        }
        // End if(_predefined == 0x00)

        // Combine into a single byte when rendering to a stream
        bool getOCRStreamFlag() const
        {
            return _OCRStreamFlag;
        }
        uint16 getOCRESID() const
        {
            return _OCRESID;
        }


        // Rendering the Descriptor in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();

    private:
        uint8 _predefined; // (8)

        // Following defined ONLY if(_predefined == 0x00)
        // Combine all below bit flags into a single byte when rendering to a stream
        bool _useAccessUnitStartFlag; // (1)
        bool _useAccessUnitEndFlag; // (1)
        bool _useRandomAccessPointFlag; // (1)
        bool _useRandomAccessUnitsOnlyFlag; // (1)
        bool _usePaddingFlag; // (1)
        bool _useTimeStampsFlag; // (1)
        bool _useIdleFlag; // (1)
        bool _durationFlag; // (1)

        uint32 _timeStampResolution; // (32)
        uint32 _OCRResolution; // (32)
        uint8 _timeStampLength; // (8) Must be <= 64
        uint8 _OCRLength; // (8) Must be <= 64
        uint8 _AULength; // (8) Must be <= 32
        uint8 _instantBitrateLength; // (8)
        uint8 _degradationPriorityLength; // (4)
        uint8 _AUSeqNumLength; // (5)  Must be <= 16
        uint8 _packetSeqNumLength; // (5)  Must be <= 16
        uint8 _reserved1; // = 0b11; // (2)

        uint32 _timeScale; // (32)
        uint16 _accessUnitDuration; // (16)
        uint16 _compositionUnitDuration; // (16)

        uint64 _startDecodingTimeStamp; // (_timeStampLength)
        uint64 _startCompositionTimeStamp; // (_timeStampLength)
        // End if(_predefined == 0x00)

        // Combine into a single byte when rendering to a stream
        bool _OCRStreamFlag; // (1)
        uint8 _reserved2; // = 0b1111111; // (7)

        uint16 _OCRESID; // (16)


};



#endif

