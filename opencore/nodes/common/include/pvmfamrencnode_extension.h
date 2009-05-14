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
 * @file pvmfamrencnode_extension.h
 */

#ifndef PVMFAMRENCNODE_EXTENSION_H_INCLUDED
#define PVMFAMRENCNODE_EXTENSION_H_INCLUDED

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

#define PVAMREncExtensionUUID PVUuid(0xd282cc91,0x54a6,0x4f36,0xbc,0x5b,0x2d,0xab,0xb4,0x40,0x69,0x26)

/**
 * Enumerated list of output bitrate
 */

typedef enum
{
    GSM_AMR_4_75 = 0,
    GSM_AMR_5_15,
    GSM_AMR_5_90,
    GSM_AMR_6_70,
    GSM_AMR_7_40,
    GSM_AMR_7_95,
    GSM_AMR_10_2,
    GSM_AMR_12_2,
    GSM_AMR_DTX,
    GSM_AMR_N_MODES      /* number of (SPC) modes */
} PVMF_GSMAMR_Rate;


////////////////////////////////////////////////////////////////////////////
class PVAMREncExtensionInterface : public PVInterface
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
        virtual PVMFStatus SetOutputBitRate(PVMF_GSMAMR_Rate aBitRate) = 0;

        /**
         * Sets the maximum number of output frames per media buffer
         * @param aNumOutputFrames, maximum number of output frames
         * @return PVMFSuccess if successful, else see PVMF return code.
         */
        virtual PVMFStatus SetMaxNumOutputFramesPerBuffer(uint32 aNumOutputFrames) = 0;
};

#endif // PVMFAMRENCNODE_EXTENSION_H_INCLUDED





