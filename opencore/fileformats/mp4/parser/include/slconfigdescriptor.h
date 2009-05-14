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
/*                        MPEG-4 SLConfigDescriptor Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SLConfigDescriptor (SyncLayerDescriptor) Class
*/


#ifndef SLCONFIGDESCRIPTOR_H_INCLUDED
#define SLCONFIGDESCRIPTOR_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef BASEDESCRIPTOR_H_INCLUDED
#include "basedescriptor.h"
#endif

class SLConfigDescriptor : public BaseDescriptor
{

    public:
        SLConfigDescriptor(MP4_FF_FILE *fp); // Stream-in Constructor
        virtual ~SLConfigDescriptor();

        // ALL the member gets and sets

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

        uint64 getStartDecodingTimeStamp() const
        {
            return _startDecodingTimeStamp;
        }
        uint64 getStartCompositionTimeStamp() const
        {
            return _startCompositionTimeStamp;
        }

        // Combine into a single byte when rendering to a stream
        bool getOCRStreamFlag() const
        {
            return _OCRStreamFlag;
        }
        uint16 getOCRESID() const
        {
            return _OCRESID;
        }

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


        // Combine into a single byte when rendering to a stream
        bool _OCRStreamFlag; // (1)
        uint8 _reserved2; // = 0b1111111; // (7)

        uint16 _OCRESID; // (16)
};

#endif // SLCONFIGDESCRIPTOR_H_INCLUDED


