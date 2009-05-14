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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------

#ifndef _H245_H_
#define _H245_H_

#include "layer.h"
#include "packet_io.h"
#include "media_packet.h"
#include "per.h"
#include "se.h"
#include "oscl_mem.h"


//===============================================================
//---------------------------------------------------------------
// CLASS DEFINITIONS
//---------------------------------------------------------------
//===============================================================
class H245Observer
{
    public:
        virtual ~H245Observer() {}
        virtual void Handle(PS_ControlMsgHeader msg) = 0;
};

class H245 : public Layer, public PacketInput, public PacketOutput
{
    public:

        // Constructor
        H245() : Observer(0)
        {
            // Initialize PER
            MyPer.SetH245(this);
            MyPer.SetSe(&MySe);

            // Initialize SE
            MySe.SetH245(this);
            MySe.SetPer(&MyPer);

            Pack = iMediaPktAlloc.allocate_packet();
        }

        // Destructor
        virtual ~H245()
        {
            iMediaPktAlloc.deallocate_packet(Pack);
        }

        // resets the states for a new session
        void Reset()
        {
            // need to reset SEs' state
            MySe.Reset();
            // reset PER so it stops processing incoming messages
            MyPer.Reset();
        }

        PacketInput *GetPacketInput()
        {
            return this;
        }
        PacketOutput *GetPacketOutput()
        {
            return this;
        }

        // Send a message to H.245 PER layer (normally from Srp).
        // Leaves if out-of-memory
        void PacketIn(Packet *pack)
        {
            uint8 *pData = (uint8 *)OSCL_DEFAULT_MALLOC(pack->GetMediaSize());
            OsclError::LeaveIfNull(pData);

            oscl_memcpy(pData, pack->GetMediaPtr(), pack->GetMediaSize());
            pack->ClearMediaPtr();
            MyPer.Decode(pData, pack->GetMediaSize());
        }

        void PacketOut(unsigned char *pData, int size)
        {
            if (pPktOutput)
            {
                // create media fragment and copy data to fragment
                MediaFragment *frag = DefFragAlloc.allocate_fragment(size);
                oscl_memcpy(frag->GetFragment()->ptr, pData, size);
                // add fragment to packet and send
                Pack->AddMediaFragment(frag);
                DefFragAlloc.deallocate_fragment(frag);
                pPktOutput->PacketIn(Pack);
                // decrement fragment use by this
                Pack->Clear();
            }
        }

        void SetObserver(H245Observer *observer)
        {
            Observer = observer;
        }

        // Send a message to Controls Observer (normally Tsc)
        void ObserverMessage(unsigned char *pData, int size)
        {
            OSCL_UNUSED_ARG(size);
            if (Observer) Observer->Handle((PS_ControlMsgHeader)pData);
        }

        // Send a message to Lower Layer (normally Srp)
        void LowerLayerTx(unsigned char *pData, int size)
        {
            PacketOut(pData, size);
        }
        // Send a message to H.245 SE layer (normally from Tsc).
        void DispatchControlMessage(PS_ControlMsgHeader msg)
        {
            MySe.InformationRecv((PS_InfHeader)msg);
        }

        MSD* GetMSD()
        {
            return MySe.GetMSD();
        }
        CE*  GetCE()
        {
            return MySe.GetCE();
        }

        void SetTimers(H245TimerValues& timer_vals)
        {
            MySe.SetTimers(timer_vals);
        }
    private:
        H245(H245&);

        PER MyPer;
        SE MySe;

        H245Observer *Observer;
        MediaPacket *Pack;
        DefFragmentAllocator DefFragAlloc;
        MediaPacketAllocator iMediaPktAlloc;
};
//===============================================================


#endif // _H245_H_
