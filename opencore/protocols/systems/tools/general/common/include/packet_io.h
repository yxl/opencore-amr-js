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
// -*- c++ -*-
#ifndef PACKET_IO_H
#define PACKET_IO_H

#include "packet.h"

typedef enum
{
    PACKET_IO_SUCCESS = 0,
    PACKET_IO_FAILURE
}TPvPacketIoStatus;

class PacketInputObserver
{
    public:
        virtual ~PacketInputObserver() {}
        virtual void PacketInComplete(TPvPacketIoStatus status) = 0;
};

/* A PacketInput interface is implemented to receive packets.  */
class PacketInput
{
    public:
        PacketInput() : iPacketInputObserver(NULL)
        {

        }
        virtual void PacketIn(Packet* pack) = 0;
        virtual void PacketIn(Packet* pack, bool /*crc_error*/, int32 /*seq_num_error*/)
        {
            PacketIn(pack);
        }
        virtual ~PacketInput() {}
        virtual void SetPacketInputObserver(PacketInputObserver* observer)
        {
            iPacketInputObserver = observer;
        }
    protected:
        PacketInputObserver* iPacketInputObserver;
};

/* PacketOutput interface is implemented to indicate packets via the PacketInput interface */
class PacketOutput
{
    public:
        PacketOutput() : pPktOutput(NULL)
        {
        }
        void SetPacketOutput(PacketInput* packet_output)
        {
            pPktOutput = packet_output;
        }
        void ResetPacketOutput(void)
        {
            pPktOutput = NULL;
        }
    protected:
        PacketInput* pPktOutput;
};

#endif
