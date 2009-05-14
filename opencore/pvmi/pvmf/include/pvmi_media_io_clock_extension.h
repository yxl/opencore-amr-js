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
 * @file pvmi_media_io_clock_extension.h
 * @brief Extension clock interface for media i/o components
 */

#ifndef PVMI_MEDIA_IO_CLOCK_EXTENSION_H_INCLUDED
#define PVMI_MEDIA_IO_CLOCK_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif


#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif


#define PvmiClockExtensionInterfaceUuid PVUuid(0x5411dd90,0xe335,0x4b06,0xbe,0xe4,0x7c,0x81,0x40,0x6a,0xf3,0xaa)

/**
 * PvmiClockExtensionInterface allows an external client to set the clock through this
 * extension interface.
 */
class PvmiClockExtensionInterface  : public PVInterface
{
    public:
        /**
         * Register a reference to this interface.
         */
        virtual void addRef() = 0;

        /**
         * Remove a reference to this interface.
         */
        virtual void removeRef() = 0;

        /**
         * Query for an instance of a particular interface.
         *
         * @param uuid Uuid of the requested interface
         * @param iface Output parameter where pointer to an instance of the
         * requested interface will be stored if it is supported by this object
         * @return true if the requested interface is supported, else false
         */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * This method allows the media I/O component to accept an OSCL clock
         *
         * @param clockVal the Instance of OSCL Clock
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetClock(PVMFMediaClock *clockVal) = 0;
};

#endif // PVMI_MEDIA_IO_CLOCK_EXTENSION_H_INCLUDED


