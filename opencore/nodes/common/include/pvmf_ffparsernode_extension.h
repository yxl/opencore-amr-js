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
 * @file pvmf_ffparsernode_extension.h
 * @brief Extension interface for PVMF FF parser nodes
 */

#ifndef PVMF_FFPARSERNODE_EXTENSION_H_INCLUDED
#define PVMF_FFPARSERNODE_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif


#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

// UUID for the extension interface
// The below commented out line won't build on ARM5 - have to replace w/ #define. Global data not allowed on ARM5.
//const PVUuid KPVMFFFParserNodeExtensionUuid = PVUuid(0x72B76D8C,0xB4CC,0x4887,0xB1,0x0B,0x7E,0xE6,0x48,0x47,0xB2,0x05);
#define KPVMFFFParserNodeExtensionUuid  PVUuid(0x72B76D8C,0xB4CC,0x4887,0xB1,0x0B,0x7E,0xE6,0x48,0x47,0xB2,0x05)

// Defines
#define PVMFFFPARSERNODE_MAX_NUM_TRACKS	6

// Structs
struct PVMFFFParserNodeTrackInfo
{
    int32 iNumTracks;
    uint32 iTrackId[PVMFFFPARSERNODE_MAX_NUM_TRACKS];
    PVMFFormatType iFormatType[PVMFFFPARSERNODE_MAX_NUM_TRACKS];
    uint32 iBitrate[PVMFFFPARSERNODE_MAX_NUM_TRACKS];
    uint32 iTimescale[PVMFFFPARSERNODE_MAX_NUM_TRACKS];
};

/**
 * PVMFFFParserNodeExtensionInterface specifies extended interfaces for file format parser nodes
 */
class PVMFFFParserNodeExtensionInterface : public PVInterface
{
    public:
        virtual void addRef() = 0;
        virtual void removeRef() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * Sets the path and filename for the input file to be used for the data source
         *
         * @param aFilename Wide character OSCL_String.reference to the input filename
         * @return PVMFSuccess if successful. PVMFFailure if not
         */
        virtual PVMFStatus SetSourceFilename(OSCL_wString& aFilename) = 0;

        /**
         * Retrieves the track information for the currently set and initialized file
         *
         * @param aTrackInfo Reference to a PVMFFFParserNodeTrackInfo struct that will be filled with track info
         * @return PVMFSuccess if successful. PVMFFailure if not
         */
        virtual PVMFStatus GetTrackInfo(PVMFFFParserNodeTrackInfo& aTrackInfo) = 0;
};

#endif // PVMF_FFPARSERNODE_EXTENSION_H_INCLUDED
