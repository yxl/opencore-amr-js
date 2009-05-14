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
#ifndef RTSP_TRANSPORT_H
#define RTSP_TRANSPORT_H

#include "oscl_base.h"
#include "rtsp_session_types.h"

struct RtspTransport
{

    typedef enum { RTP_PROTOCOL, RDT_PROTOCOL, UNKNOWN_PROTOCOL } ProtocolType;
    typedef enum { AVP_PROFILE, TCP_PROFILE, UNKNOWN_PROFILE } ProfileType;
    typedef enum { UDP_TRANSPORT, TCP_TRANSPORT, UNKNOWN_TRANSPORT } TransportType;
    typedef enum { UNICAST_DELIVERY, MULTICAST_DELIVERY } DeliveryType;

    ProtocolType protocol;
    bool protocolIsSet;
    ProfileType profile;
    bool profileIsSet;
    TransportType transportType;
    bool transportTypeIsSet;
    DeliveryType delivery;
    bool deliveryIsSet;

    StrPtrLen     destination;
    bool          destinationIsSet;

    uint16 channel1;
    uint16 channel2;
    bool channelIsSet;

    bool append;
    bool appendIsSet;

    uint16 ttl;
    bool ttlIsSet;

    uint32 layers;
    bool layersIsSet;

    uint16 port1;
    uint16 port2;
    bool portIsSet;

    uint16 client_port1;
    uint16 client_port2;
    bool client_portIsSet;

    uint16 server_port1;
    uint16 server_port2;
    bool server_portIsSet;

    struct
    {
unsigned int reserved:
        30;
bool play_mode:
        1;
bool record_mode:
        1;
    } mode;
    bool modeIsSet;

    Ssrc ssrc;
    bool ssrcIsSet;

    RtspTransport()
    {
        oscl_memset(this, 0, sizeof(this));
    }

};


#endif
