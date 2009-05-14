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
#ifndef PVMF_DATA_SOURCE_DIRECTION_CONTROL_H_INCLUDED
#define PVMF_DATA_SOURCE_DIRECTION_CONTROL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#define PVMF_DATA_SOURCE_DIRECTION_CONTROL_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmfdatasourcedirectioncontrolinterface"
#define PvmfDataSourceDirectionControlUuid PVUuid(0xd2f06f90,0x9827,0x11da,0xa7,0x2b,0x08,0x00,0x20,0x0c,0x9a,0x66)
//d2f06f90-9827-11da-a7-2b-08-00-20-0c-9a-66

//values for the aDirection parameter of SetDataSourceDirection
#define PVMF_DATA_SOURCE_DIRECTION_FORWARD 0
#define PVMF_DATA_SOURCE_DIRECTION_REVERSE 1

/**
 * Configuration interface to control data source nodes for playback
 */
class PvmfDataSourceDirectionControlInterface : public PVInterface
{
    public:

        /**
         * Asynchronous method to set the playback direction of the data source
         *
         * The playback direction is either forward or backward.  The direction change is assumed
         * to occur from the current source position.
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aDirection The new playback direction, 0=forward, 1=reverse.
         * @param aTimebase (optional) The timebase specified to use for the playback clock.
         * @param aActualNPT The actual normal-play-time timestamp after changing direction will be saved
         *                   to this parameter.
         * @param aActualMediaDataTS The media data timestamp corresponding to the actual NPT time. This
         *                           will be the timestamp of the first media data after changing direction.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVMFCommandId SetDataSourceDirection(PVMFSessionId aSessionId,
                int32 aDirection,
                PVMFTimestamp& aActualNPT,
                PVMFTimestamp& aActualMediaDataTS,
                PVMFTimebase* aTimebase = NULL,
                OsclAny* aContext = NULL) = 0;

};

#endif // PVMF_DATA_SOURCE_DIRECTION_CONTROL_H_INCLUDED




