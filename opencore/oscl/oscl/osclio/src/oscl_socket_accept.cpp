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

#include "oscl_scheduler_ao.h"
#include "oscl_socket_accept.h"

//////////// Method /////////////////////

OsclAcceptMethod *OsclAcceptMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclAcceptMethod));
    OsclError::LeaveIfNull(p);
    OsclAcceptMethod* self = OSCL_PLACEMENT_NEW(p, OsclAcceptMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclAcceptMethod::ConstructL()
{
    OsclAny*p = iContainer.Alloc().ALLOCATE(sizeof(OsclAcceptRequest));
    OsclError::LeaveIfNull(p);
    OsclAcceptRequest* self = OSCL_PLACEMENT_NEW(p, OsclAcceptRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclAcceptMethod::~OsclAcceptMethod()
{
    if (AcceptRequest())
    {
        AcceptRequest()->~OsclAcceptRequest();
        Alloc().deallocate(AcceptRequest());
    }
    DiscardAcceptedSocket();
}

TPVSocketEvent OsclAcceptMethod::Accept(int32 aTimeout)
{
    //in case previous accepted socket was never
    //retrieved...
    DiscardAcceptedSocket();

    iAcceptedSocket = OsclSocketI::NewL(Alloc());

    if (iAcceptedSocket->Open(*SocketServ()) != OsclErrNone)
    {
        DiscardAcceptedSocket();
        return EPVSocketFailure;
    }

    if (!StartMethod(aTimeout))
    {
        DiscardAcceptedSocket();
        return EPVSocketFailure;
    }

    AcceptRequest()->Accept(*iAcceptedSocket);
    return EPVSocketPending;
}

void OsclAcceptMethod::DiscardAcceptedSocket()
{
    if (iAcceptedSocket)
    {
        iAcceptedSocket->~OsclSocketI();
        Alloc().deallocate(iAcceptedSocket);
    }
    iAcceptedSocket = NULL;
}

OsclSocketI *OsclAcceptMethod::GetAcceptedSocket()
{
    if (iAcceptedSocket)
    {
        //Clear accepted socket once it has been retrieved.
        OsclSocketI *sock = iAcceptedSocket;
        iAcceptedSocket = NULL;
        return sock;
    }
    else
        return NULL;
}

void OsclAcceptMethod::Run()
{
    //The request timed out!

    //must discard accepted socket in addition
    //to the usual timeout Run
    DiscardAcceptedSocket();

    //call the base class Run
    OsclSocketMethod::Run();
}

//////////// Request /////////////////////

void OsclAcceptRequest::Accept(OsclSocketI &aSocket)
{
    OsclAny *p = NewRequest(sizeof(AcceptParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, AcceptParam(aSocket));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
            SocketI()->Accept(*Param(), *this);
    }
}

void OsclAcceptRequest::Run()
{
    //The request was completed by the socket server

    //may need to discard accepted socket in addition to
    //the usual request completion.
    if (Status() != OSCL_REQUEST_ERR_NONE)
        ((OsclAcceptMethod*)&iContainer)->DiscardAcceptedSocket();

    //call the base class Run
    OsclSocketRequestAO::Run();
}


