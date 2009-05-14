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

#ifndef OSCL_SOCKET_RECV_FROM_H_INCLUDED
#define OSCL_SOCKET_RECV_FROM_H_INCLUDED

#include "oscl_socket_serv_imp.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of RecvFrom
  */
class OsclRecvFromRequest;

class OsclRecvFromMethod : public OsclSocketMethod
{
    public:
        static OsclRecvFromMethod *NewL(OsclIPSocketI& c);

        ~OsclRecvFromMethod();

        TPVSocketEvent RecvFrom(uint8* &aPtr, uint32 aMaxLen,
                                OsclNetworkAddress &aAddress,
                                int32 aTimeout,
                                uint32 aMultiMax,
                                Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
                                Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource);

        uint8 *GetRecvData(int32 *aLength);

    private:
        OsclRecvFromMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclrecvfrommethod", EPVSocketRecvFrom)
        {}

        void ConstructL();

    public:
        OsclRecvFromRequest *RecvFromRequest()
        {
            return (OsclRecvFromRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclRecvFromRequest : public OsclSocketRequestAO
{
    public:
        uint8 *GetRecvData(int32 *aLength);

        OsclRecvFromRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclRecvFromRequest")
        {}

        void RecvFrom(uint8* &aPtr, uint32 aMaxLen,
                      OsclNetworkAddress& aAddress, uint32 aMultiMax,
                      Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
                      Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource);
        void Success();

    private:
        RecvFromParam*Param()
        {
            return(RecvFromParam*)iParam;
        }
};


#endif

