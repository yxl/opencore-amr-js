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

#ifndef OSCL_SOCKET_RECV_H_INCLUDED
#define OSCL_SOCKET_RECV_H_INCLUDED

#include "oscl_socket_serv_imp.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_method.h"


/**
  Internal implementation of Recv
  */
class OsclRecvRequest;

class OsclRecvMethod : public OsclSocketMethod
{
    public:
        static OsclRecvMethod *NewL(OsclIPSocketI& c);

        ~OsclRecvMethod();

        TPVSocketEvent Recv(uint8* &aPtr, uint32 aMaxLen,
                            int32 aTimeout);

        uint8 *GetRecvData(int32 *aLength);

    private:
        OsclRecvMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclrecvmethod", EPVSocketRecv)
        {}

        void ConstructL();

    public:
        OsclRecvRequest *RecvRequest()
        {
            return (OsclRecvRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclRecvRequest : public OsclSocketRequestAO
{
    public:
        uint8 *GetRecvData(int32 *aLength);

        OsclRecvRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclRecvRequest")
        {}

        void Recv(uint8* &aPtr, uint32 aMaxLen);
        void Success();

    private:

        RecvParam*Param()
        {
            return(RecvParam*)iParam;
        }

};



#endif

