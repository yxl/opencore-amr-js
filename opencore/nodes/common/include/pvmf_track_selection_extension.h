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
#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#define PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

#define PVMF_TRACK_SELECTION_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmftrackselectioninterface"
#define PVMF_TRACK_SELECTION_INTERFACE_UUID PVUuid(0x3afa4e1b,0x989c,0x41c3,0x90,0xef,0x11,0xa9,0xf5,0xa4,0x59,0xd7)

class PVMFTrackSelectionExtensionInterface : public PVInterface
{
    public:
        virtual void addRef() = 0;
        virtual void removeRef() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
        /**
        * Gets the media presentation information from a source node.
        *
        * @param aInfo, a pointer to PVMFMediaPresentationInfo
        * @return PVMFSuccess if successfully set.PVMFFailure for errors.
        */
        virtual PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo) = 0;
        /**
        * API to provide information on selected tracks
        *
        * @param aInfo, a pointer to PVMFMediaPresentationInfo, that contains
        * a possibly pruned list of selected trackinfo.
        * @return PVMFSuccess if successfully set.PVMFFailure for errors.
        */
        virtual PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo) = 0;
};

#endif //PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED

