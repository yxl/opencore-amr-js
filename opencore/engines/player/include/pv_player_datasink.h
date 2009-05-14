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
* ==============================================================================
*  Name        : pv_player_datasink.h
*  Part of     :
*  Interface   :
*  Description : Abstract base class for player engine data sink
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_PLAYER_DATASINK_H_INCLUDED
#define PV_PLAYER_DATASINK_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

class PVMFNodeInterface;

typedef enum
{
    PVP_DATASINKTYPE_UNKNOWN,
    PVP_DATASINKTYPE_FILENAME,
    PVP_DATASINKTYPE_SINKNODE
} PVPDataSinkType;


// CLASS DECLARATION
/**
 * PVPlayerDataSink is an abstract base class for player engine data sink.
 * Player engine expects the data sink to be derived from PVPlayerDataSink.
 **/
class PVPlayerDataSink
{
    public:
        /**
         * Object destructor function
         **/
        virtual ~PVPlayerDataSink() {};

        /**
         * Returns this instance's data sink type
         *
         * @returns The PVPlayer data sink type
         **/
        virtual PVPDataSinkType GetDataSinkType() = 0;

        /**
         * Returns this data sink instance's format type
         *
         * @returns The format type
         **/
        virtual PVMFFormatType GetDataSinkFormatType() = 0;

        /**
         * Returns the filename for the data sink
         *
         * @returns The filename as an OSCL_wString
         **/
        virtual OSCL_wString& GetDataSinkFilename() = 0;

        /**
         * Returns the node interface for the data sink
         *
         * @returns The node interface pointer to the data sink
         **/
        virtual PVMFNodeInterface* GetDataSinkNodeInterface() = 0;
};

#endif // PV_PLAYER_DATASINK_H_INCLUDED

