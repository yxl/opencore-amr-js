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

#ifndef OSCL_SOCKET_LISTEN_H_INCLUDED
#define OSCL_SOCKET_LISTEN_H_INCLUDEDd

#include "oscl_socket_types.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of async Listen
  */
class OsclListenRequest;

class OsclListenMethod : public OsclSocketMethod
{
    public:
        static OsclListenMethod *NewL(OsclIPSocketI& c);

        ~OsclListenMethod();

        TPVSocketEvent Listen(uint32 qsize,
                              int32 aTimeout);

    private:
        OsclListenMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "oscllistenmethod", EPVSocketListen)
        {}

        void ConstructL();

    public:
        OsclListenRequest *ListenRequest()
        {
            return (OsclListenRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclListenRequest : public OsclSocketRequestAO
{
    public:
        OsclListenRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclListenRequest")
        {}

        void Listen(uint32 qsize);

    private:

        ListenParam*Param()
        {
            return(ListenParam*)iParam;
        }

};


#endif



