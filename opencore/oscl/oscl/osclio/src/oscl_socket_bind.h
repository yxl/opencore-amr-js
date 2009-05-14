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

#ifndef OSCL_SOCKET_BIND_H_INCLUDED
#define OSCL_SOCKET_BIND_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of async Bind
  */
class OsclBindRequest;

class OsclBindMethod : public OsclSocketMethod
{
    public:
        static OsclBindMethod *NewL(OsclIPSocketI& c);

        ~OsclBindMethod();

        TPVSocketEvent Bind(OsclNetworkAddress& aAddress,
                            int32 aTimeout);

    private:
        OsclBindMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclbindmethod", EPVSocketBind)
        {}

        void ConstructL();

    public:
        OsclBindRequest *BindRequest()
        {
            return (OsclBindRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclBindRequest : public OsclSocketRequestAO
{
    public:
        OsclBindRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclBindRequest")
        {}

        void Bind(OsclNetworkAddress &aAddress);

    private:

        BindParam*Param()
        {
            return(BindParam*)iParam;
        }

};


#endif



