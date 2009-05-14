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

#include "oscl_tcp_socket.h"
#include "oscl_error.h"
#include "oscl_scheduler_ao.h"
#include "oscl_socket_connect.h"
#include "oscl_socket_shutdown.h"
#include "oscl_socket_send.h"
#include "oscl_socket_recv.h"
#include "oscl_socket_accept.h"
#include "oscl_socket_bind.h"

//Public methods
//////////////////////////////////////////////////////////////////////////////////
OsclTCPSocketI *OsclTCPSocketI::NewL(Oscl_DefAlloc &a, OsclSocketServI *aServ,
                                     OsclSocketObserver *aObserver,
                                     uint32 aId)
{
    OsclAny *p = a.ALLOCATE(sizeof(OsclTCPSocketI));
    OsclError::LeaveIfNull(p);
    OsclTCPSocketI* self = OSCL_PLACEMENT_NEW(p, OsclTCPSocketI(a));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL(aServ, aObserver, aId);
    OsclError::Pop();
    return self;
}

//////////////////////////////////////////////////////////////////////////////////
OsclTCPSocketI *OsclTCPSocketI::NewL(Oscl_DefAlloc &a, OsclSocketServI *aServ,
                                     OsclSocketI *aSocket,
                                     OsclSocketObserver *aObserver,
                                     uint32 aId)
{
    OsclAny *p = a.ALLOCATE(sizeof(OsclTCPSocketI));
    OsclError::LeaveIfNull(p);
    OsclTCPSocketI* self = OSCL_PLACEMENT_NEW(p, OsclTCPSocketI(a));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL(aServ, aSocket, aObserver, aId);
    OsclError::Pop();
    return self;
}


//////////////////////////////////////////////////////////////////////////////////
OsclTCPSocketI::~OsclTCPSocketI()
{
    //Cancel any outstanding requests.

    iConnectMethod->AbortAll();
    OSCL_ASSERT(!iConnectMethod->IsBusy());
    OSCL_ASSERT(!iConnectMethod->ConnectRequest()->IsBusy());

    iShutdownMethod->AbortAll();
    OSCL_ASSERT(!iShutdownMethod->IsBusy());
    OSCL_ASSERT(!iShutdownMethod->ShutdownRequest()->IsBusy());

    iAcceptMethod->AbortAll();
    OSCL_ASSERT(!iAcceptMethod->IsBusy());
    OSCL_ASSERT(!iAcceptMethod->AcceptRequest()->IsBusy());

    iSendMethod->AbortAll();
    OSCL_ASSERT(!iSendMethod->IsBusy());
    OSCL_ASSERT(!iSendMethod->SendRequest()->IsBusy());

    iRecvMethod->AbortAll();
    OSCL_ASSERT(!iRecvMethod->IsBusy());
    OSCL_ASSERT(!iRecvMethod->RecvRequest()->IsBusy());

    iListenMethod->AbortAll();
    OSCL_ASSERT(!iListenMethod->IsBusy());
    OSCL_ASSERT(!iListenMethod->ListenRequest()->IsBusy());

    iBindMethod->AbortAll();
    OSCL_ASSERT(!iBindMethod->IsBusy());
    OSCL_ASSERT(!iBindMethod->BindRequest()->IsBusy());

    //Delete method objects.

    iConnectMethod->~OsclConnectMethod();
    iAlloc.deallocate(iConnectMethod);

    iShutdownMethod->~OsclShutdownMethod();
    iAlloc.deallocate(iShutdownMethod);

    iAcceptMethod->~OsclAcceptMethod();
    iAlloc.deallocate(iAcceptMethod);

    iSendMethod->~OsclSendMethod();
    iAlloc.deallocate(iSendMethod);

    iRecvMethod->~OsclRecvMethod();
    iAlloc.deallocate(iRecvMethod);

    iListenMethod->~OsclListenMethod();
    iAlloc.deallocate(iListenMethod);

    iBindMethod->~OsclBindMethod();
    iAlloc.deallocate(iBindMethod);

    iSocket->~OsclSocketI();
    iAlloc.deallocate(iSocket);
}

//////////////////////////////////////////////////////////////////////////////////
int32 OsclTCPSocketI::Close()
{
    if (iSocket)
    {
        //Cancel any outstanding requests.
        iConnectMethod->AbortAll();
        iShutdownMethod->AbortAll();
        iAcceptMethod->AbortAll();
        iSendMethod->AbortAll();
        iRecvMethod->AbortAll();
        iListenMethod->AbortAll();
        iBindMethod->AbortAll();
        return iSocket->Close();
    }
    else
        return OsclErrGeneral;
}


//////////////////////////////////////////////////////////////////////////////////
OsclTCPSocketI *OsclTCPSocketI::GetAcceptedSocketL(uint32 aId)
{
    OsclSocketI *sock = iAcceptMethod->GetAcceptedSocket();
    if (!sock)
        OsclError::Leave(OsclErrGeneral);
    return OsclTCPSocketI::NewL(iAlloc, iSocketServ, sock, iObserver, aId);
}


//Private methods
//////////////////////////////////////////////////////////////////////////////////
void OsclTCPSocketI::ConstructL(OsclSocketServI *aServ,
                                OsclSocketObserver *aObserver,
                                uint32 aId)
{
    iSocket = OsclSocketI::NewL(iAlloc);
    OsclIPSocketI::ConstructL(aObserver, iSocket, aServ, aId);

    //Open socket session
    if (iSocket->Open(*iSocketServ, OSCL_AF_INET, OSCL_SOCK_STREAM, OSCL_IPPROTO_TCP) != OsclErrNone)
        OsclError::Leave(OsclErrGeneral);

    //Allocate asynchronous method objects.
    iConnectMethod = OsclConnectMethod::NewL(*this);
    iShutdownMethod = OsclShutdownMethod::NewL(*this);
    iAcceptMethod = OsclAcceptMethod::NewL(*this);
    iSendMethod = OsclSendMethod::NewL(*this);
    iRecvMethod = OsclRecvMethod::NewL(*this);
    iListenMethod = OsclListenMethod::NewL(*this);
    iBindMethod = OsclBindMethod::NewL(*this);
}

//////////////////////////////////////////////////////////////////////////////////
void OsclTCPSocketI::ConstructL(OsclSocketServI *aServ,
                                OsclSocketI *aSocket,
                                OsclSocketObserver *aObserver,
                                uint32 aId)
{
    OsclIPSocketI::ConstructL(aObserver, aSocket, aServ, aId);

    //Allocate asynchronous method objects.
    iConnectMethod = OsclConnectMethod::NewL(*this);
    iShutdownMethod = OsclShutdownMethod::NewL(*this);
    iAcceptMethod = OsclAcceptMethod::NewL(*this);
    iSendMethod = OsclSendMethod::NewL(*this);
    iRecvMethod = OsclRecvMethod::NewL(*this);
    iListenMethod = OsclListenMethod::NewL(*this);
    iBindMethod = OsclBindMethod::NewL(*this);
}
