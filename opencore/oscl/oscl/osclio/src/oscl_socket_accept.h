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

#ifndef OSCL_SOCKET_ACCEPT_H_INCLUDED
#define OSCL_SOCKET_ACCEPT_H_INCLUDED


#include "oscl_socket_imp.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of Accept
  */
class OsclAcceptRequest;

class OsclAcceptMethod : public OsclSocketMethod
{
    public:
        static OsclAcceptMethod *NewL(OsclIPSocketI& c);

        ~OsclAcceptMethod();

        TPVSocketEvent Accept(int32 aTimeout);

        void DiscardAcceptedSocket();
        OsclSocketI *GetAcceptedSocket();

    private:

        OsclAcceptMethod(OsclIPSocketI &c)
                : OsclSocketMethod(c, "osclacceptmethod", EPVSocketAccept)
                , iAcceptedSocket(NULL)
        {}

        void ConstructL();

        void Run();

        OsclSocketServI *SocketServ()
        {
            return iContainer.SocketServ();
        }
    public:
        OsclAcceptRequest *AcceptRequest()
        {
            return (OsclAcceptRequest*)iSocketRequestAO;
        }
    private:
        OsclSocketI *iAcceptedSocket;
};

/**
  This is the AO that interacts with the socket server
  */
class AcceptParam;

class OsclAcceptRequest : public OsclSocketRequestAO
{
    public:
        OsclAcceptRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclacceptreq")
        {}

        void Accept(OsclSocketI &aSocket);

    private:
        void Run();

        AcceptParam*Param()
        {
            return(AcceptParam*)iParam;
        }
};




#endif

