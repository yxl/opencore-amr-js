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

#ifndef OSCL_IP_SOCKET_H_INCLUDED
#define OSCL_IP_SOCKET_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_vector.h"
#include "oscl_mem.h"

/** OsclIPSocketI is the base class for UDP and TCP socket implementations.
*/
class OsclSocketServ;
class OsclSocketI;
class OsclSocketServI;
class PVLogger;
class OsclIPSocketI
{
    public:
        //Synchronous methods.
        int32 Bind(OsclNetworkAddress& aAddress);
        int32 Join(OsclNetworkAddress& aAddress);
        int32 SetRecvBufferSize(uint32 size);
        virtual int32 Close() = 0;
        virtual uint8 *GetRecvData(int32 *aLength) = 0;
        virtual uint8 *GetSendData(int32 *aLength) = 0;
        virtual ~OsclIPSocketI() {}

        OsclSocketServI* SocketServ()
        {
            return iSocketServ;
        }
        Oscl_DefAlloc& Alloc()
        {
            return iAlloc;
        }

    protected:
        Oscl_DefAlloc &iAlloc;

        OsclNetworkAddress iAddress;
        uint32 iId;
        OsclSocketObserver *iObserver;
        OsclSocketI *iSocket;
        OsclSocketServI *iSocketServ;
        PVLogger* iLogger;

        friend class OsclSocketRequestAO;
        friend class OsclSocketMethod;

        OsclIPSocketI(Oscl_DefAlloc& a): iAlloc(a)
                , iId(0)
                , iObserver(NULL)
                , iSocket(NULL)
                , iSocketServ(NULL)
                , iLogger(NULL)
        {}

        void ConstructL(OsclSocketObserver *aObs, OsclSocketI* aSock, OsclSocketServI* aServ, uint32 aId);

};



#endif


