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

#include "oscl_udp_socket.h"
#include "oscl_error.h"
#include "oscl_scheduler_ao.h"
#include "oscl_socket_send_to.h"
#include "oscl_socket_recv_from.h"
#include "oscl_socket_bind.h"

//Public methods
//////////////////////////////////////////////////////////////////////////////////
OsclUDPSocketI *OsclUDPSocketI::NewL(Oscl_DefAlloc &a, OsclSocketServI *aServ,
                                     OsclSocketObserver *aObserver,
                                     uint32 aId)
{
    OsclAny*p = a.ALLOCATE(sizeof(OsclUDPSocketI));
    OsclError::LeaveIfNull(p);
    OsclUDPSocketI* self = OSCL_PLACEMENT_NEW(p, OsclUDPSocketI(a));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL(aServ, aObserver, aId);
    OsclError::Pop();
    return self;
}

//////////////////////////////////////////////////////////////////////////////////
OsclUDPSocketI::~OsclUDPSocketI()
{
    //Cancel any outstanding requests.
    iSendToMethod->AbortAll();
    OSCL_ASSERT(!iSendToMethod->IsBusy());
    OSCL_ASSERT(!iSendToMethod->SendToRequest()->IsBusy());

    iRecvFromMethod->AbortAll();
    OSCL_ASSERT(!iRecvFromMethod->IsBusy());
    OSCL_ASSERT(!iRecvFromMethod->RecvFromRequest()->IsBusy());

    iBindMethod->AbortAll();
    OSCL_ASSERT(!iBindMethod->IsBusy());
    OSCL_ASSERT(!iBindMethod->BindRequest()->IsBusy());

    iSendToMethod->~OsclSendToMethod();
    iAlloc.deallocate(iSendToMethod);
    iRecvFromMethod->~OsclRecvFromMethod();
    iAlloc.deallocate(iRecvFromMethod);
    iBindMethod->~OsclBindMethod();
    iAlloc.deallocate(iBindMethod);

    iSocket->~OsclSocketI();
    iAlloc.deallocate(iSocket);
}

//////////////////////////////////////////////////////////////////////////////////
int32 OsclUDPSocketI::Close()
{
    if (iSocket)
    {
        //Cancel any outstanding requests.
        iSendToMethod->AbortAll();
        iRecvFromMethod->AbortAll();
        iBindMethod->AbortAll();
        return iSocket->Close();
    }
    else
        return OsclErrGeneral;
}

//Private methods
//////////////////////////////////////////////////////////////////////////////////
void OsclUDPSocketI::ConstructL(OsclSocketServI *aServ,
                                OsclSocketObserver *aObserver,
                                uint32 aId)
{
    iSocket = OsclSocketI::NewL(iAlloc);

    OsclIPSocketI::ConstructL(aObserver, iSocket, aServ, aId);

    //Allocate asynchronous method objects.
    iSendToMethod = OsclSendToMethod::NewL(*this);
    iRecvFromMethod = OsclRecvFromMethod::NewL(*this);
    iBindMethod = OsclBindMethod::NewL(*this);

    //Open socket session
    if (iSocket->Open(*iSocketServ, OSCL_AF_INET, OSCL_SOCK_DATAGRAM, OSCL_IPPROTO_UDP) != OsclErrNone)
        OsclError::Leave(OsclErrGeneral);

}


