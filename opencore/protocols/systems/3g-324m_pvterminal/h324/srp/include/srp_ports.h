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
#ifndef SRP_PORTS_H_INCLUDED
#define SRP_PORTS_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef SRP_H
#include "srp.h"
#endif

#include "media_packet.h"

class SRPLowerLayerPortOut : public PvmfPortBaseImpl
{
        // All requests are synchronous
    public:
        SRPLowerLayerPortOut() : PvmfPortBaseImpl(SRP_OUTPUT_PORT_TAG, NULL) {};

        ~SRPLowerLayerPortOut() {}
};

class SRPLowerLayerPortIn : public PvmfPortBaseImpl
{
        // All requests are synchronous
    public:
        SRPLowerLayerPortIn(SRP *aSrp) : PvmfPortBaseImpl(SRP_INPUT_PORT_TAG, NULL),
                iSrp(aSrp)
        {};

        ~SRPLowerLayerPortIn() {};

        virtual PVMFStatus Receive(PVMFSharedMediaMsgPtr aMsg)
        {
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, aMsg);

            // send packet to SRP
            iSrp->LowerLayerRx(mediaData);

            return PVMFSuccess;
        }

    private:
        SRP *iSrp;
};

class SRPUpperLayerPortOut : public PvmfPortBaseImpl
{
        // All requests are synchronous
    public:
        SRPUpperLayerPortOut() : PvmfPortBaseImpl(SRP_OUTPUT_PORT_TAG, NULL) {};

        ~SRPUpperLayerPortOut() {}
};

class SRPUpperLayerPortIn : public PvmfPortBaseImpl
{
        // All requests are synchronous
    public:
        SRPUpperLayerPortIn(SRP *aSrp) : PvmfPortBaseImpl(SRP_INPUT_PORT_TAG, NULL),
                iSrp(aSrp)
        {};

        ~SRPUpperLayerPortIn() {};

        virtual PVMFStatus Receive(PVMFSharedMediaMsgPtr aMsg)
        {
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, aMsg);

            // send packet to SRP
            iSrp->UpperLayerRx(mediaData);

            return PVMFSuccess;
        }

    private:
        SRP *iSrp;
};

#endif
