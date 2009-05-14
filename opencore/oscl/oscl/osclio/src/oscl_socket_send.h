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

#ifndef OSCL_SOCKET_SEND_H_INCLUDED
#define OSCL_SOCKET_SEND_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of Send
  */
class OsclSendRequest;

class OsclSendMethod : public OsclSocketMethod
{
    public:
        static OsclSendMethod *NewL(OsclIPSocketI& c);

        ~OsclSendMethod();

        TPVSocketEvent Send(const uint8* &aPtr, uint32 aLen,
                            int32 aTimeout);

        uint8 *GetSendData(int32 *aLength);

    private:
        OsclSendMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclsendmethod", EPVSocketSend)
        {};

        void ConstructL();

    public:
        OsclSendRequest *SendRequest()
        {
            return (OsclSendRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class SendParam;
class OsclSendRequest : public OsclSocketRequestAO
{
    public:
        OsclSendRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclSendRequest")
        {}


        void Send(const uint8* &aPtr, uint32 aLen);
        void Success();
        uint8 *GetSendData(int32 *aLength);

    private:

        SendParam*Param()
        {
            return(SendParam*)iParam;
        }

};




#endif

