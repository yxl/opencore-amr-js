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
 * @file pvmp4ffcn_trackconfig.h
 * @brief Track level configuration of PVMp4FFComposerNode
 */

#ifndef PVMP4FFCN_TRACKCONFIG_H_INCLUDED
#define PVMP4FFCN_TRACKCONFIG_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define KPVMp4FFCNTrackConfigUuid PVUuid(0x81c0c80e,0xe3dd,0x4786,0xab,0xc8,0x7b,0x48,0x41,0x4f,0x6c,0x12)

/**
 * PVMp4FFCNTrackConfigInterface allows a client to do track level configuration
 * of a PVMp4FFComposerNode
 */
class PVMp4FFCNTrackConfigInterface : public PVInterface
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
         * Sets reference to a media track.
         *
         * @param aPort The port associated with the media track that references another media track.
         * @param @ReferencePort The port associated with the reference media track.
         * @return Result of this method.
         */
        virtual PVMFStatus SetTrackReference(const PVMFPortInterface& aPort, const PVMFPortInterface& aReferencePort) = 0;

        /**
         * Sets codec specific information for a media track.
         *
         * @param aPort The port to which the codec specific information is associated with.
         * @param aInfo Codec specific information.
         * @param aSize Size of codec specific information.
         * @return Result of this method.
         */
        virtual PVMFStatus SetCodecSpecificInfo(const PVMFPortInterface& aPort, uint8* aInfo, int32 aSize) = 0;
};

#endif // PVMP4FFCN_TRACKCONFIG_H_INCLUDED
