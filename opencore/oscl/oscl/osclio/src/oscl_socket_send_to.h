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

#ifndef OSCL_SOCKET_SEND_TO_H_INCLUDED
#define OSCL_SOCKET_SEND_TO_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of SendTo
  */
class OsclSendToRequest;

class OsclSendToMethod : public OsclSocketMethod
{
    public:
        static OsclSendToMethod *NewL(OsclIPSocketI& c);
        ~OsclSendToMethod();

        TPVSocketEvent SendTo(const uint8* &aPtr, uint32 aLen,
                              OsclNetworkAddress& aAddress,
                              int32 aTimeout);

        uint8 *GetSendData(int32 *aLength);

    private:
        OsclSendToMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclsendtomethod", EPVSocketSendTo)
        {};

        void ConstructL();

    public:
        OsclSendToRequest *SendToRequest()
        {
            return (OsclSendToRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclSendToRequest : public OsclSocketRequestAO
{
    public:
        OsclSendToRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclSendToRequest")
        {}

        void SendTo(const uint8* &aPtr, uint32 aLen, OsclNetworkAddress &aAddress);
        void Success();

        uint8 *GetSendData(int32 *aLength);

    private:

        SendToParam*Param()
        {
            return(SendToParam*)iParam;
        }
};





#endif

