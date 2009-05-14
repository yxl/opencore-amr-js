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
/**
 * @file pvmf_audio_encnode_extension.h
 */

#ifndef PVMF_AUDIO_ENCNODE_EXTENSION_H_INCLUDED
#define PVMF_AUDIO_ENCNODE_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif


// The AMR enc node interface was modified to audio enc node interface
//#define PVAMREncExtensionUUID PVUuid(0xd282cc91,0x54a6,0x4f36,0xbc,0x5b,0x2d,0xab,0xb4,0x40,0x69,0x26)

#define PVAudioEncExtensionUUID PVUuid(0xd282cc91,0x54a6,0x4f36,0xbc,0x5b,0x2d,0xab,0xb4,0x40,0x69,0x27)

typedef enum
{
    GSM_AMR_4_75 = 0, // AMR NB bitrates i.e. modes start here
    GSM_AMR_5_15,
    GSM_AMR_5_90,
    GSM_AMR_6_70,
    GSM_AMR_7_40,
    GSM_AMR_7_95,
    GSM_AMR_10_2,
    GSM_AMR_12_2,  // AMR NB bitrates end
    GSM_AMR_6_60, // AMR WB bitrates start here
    GSM_AMR_8_85,
    GSM_AMR_12_65,
    GSM_AMR_14_25,
    GSM_AMR_15_85,
    GSM_AMR_18_25,
    GSM_AMR_19_85,
    GSM_AMR_23_05,
    GSM_AMR_23_85, // AMR WB bitrates end here
    GSM_AMR_DTX,
    GSM_AMR_N_MODES      /* number of (SPC) modes */
} PVMF_GSMAMR_Rate;


////////////////////////////////////////////////////////////////////////////
class PVAudioEncExtensionInterface : public PVInterface
{
    public:
        /** Increment reference counter for this interface. */
        virtual void addRef() = 0;

        /** Decrement reference counter for this interface. */
        virtual void removeRef() = 0;

        /**
         * Query for a pointer to an instance of the interface specified by the UUID.
         *
         * @param uuid UUID of the interface to be queried.
         * @param iface Output parameter where a pointer to an instance of the requested
         * interface is stored if the interface is supported.
         * @return true if successful, else false.
         */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * Sets the bitrate of encoded output.
         * @param aBitRate Bitrate in bits per second.
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetOutputBitRate(uint32 aBitRate) = 0;

        /**
         * Sets the number of channels of encoded output (default to source num channel).
         * @param aNumChannel Number of channels.
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetOutputNumChannel(uint32 aNumChannel) = 0;

        /**
         * Sets the sampling of encoded output (default equal source sampling rate).
         * @param aSamplingRate Sampling rate in Hz.
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetOutputSamplingRate(uint32 aSamplingRate) = 0;

        //// From AMREncExtensionInterface
        /**
         * Sets the bitrate of encoded output.
         * @param aBitRate Bitrate in bits per second.
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetOutputBitRate(PVMF_GSMAMR_Rate aBitRate) = 0;

        /**
         * Sets the maximum number of output frames per media buffer
         * @param aNumOutputFrames, maximum number of output frames
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetMaxNumOutputFramesPerBuffer(uint32 aNumOutputFrames) = 0;
};

#endif // PVMF_AUDIO_ENCNODE_EXTENSION_H_INCLUDED





