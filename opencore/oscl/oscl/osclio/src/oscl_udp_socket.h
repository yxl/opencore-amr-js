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

#ifndef OSCL_UDP_SOCKET_H_INCLUDED
#define OSCL_UDP_SOCKET_H_INCLUDED

#include "oscl_ip_socket.h"
#include "oscl_defalloc.h"

class OsclSendToMethod;
class OsclRecvFromMethod;
class OsclBindMethod;

/**Internal implementation class for OsclUDPSocket
*/
class OsclUDPSocketI : public OsclIPSocketI
{
    public:
        static OsclUDPSocketI *NewL(Oscl_DefAlloc &a,
                                    OsclSocketServI *aServ,
                                    OsclSocketObserver *aObserver,
                                    uint32 aId);

        virtual ~OsclUDPSocketI();

        //Synchronous methods
        int32 Close();
        inline uint8 *GetRecvData(int32 *aLength);
        inline uint8 *GetSendData(int32 *aLength);

        //Asynchronous methods
        inline TPVSocketEvent BindAsync(OsclNetworkAddress& aAddress,
                                        int32 aTimeoutMsec = -1);
        inline void CancelBind();

        inline TPVSocketEvent SendTo(const uint8* &aPtr, uint32 aLen,
                                     OsclNetworkAddress& aAddress,
                                     int32 aTimeoutMsec = -1);
        inline void CancelSendTo();

        inline TPVSocketEvent RecvFrom(uint8* &aPtr, uint32 aMaxLen,
                                       OsclNetworkAddress& aAddress,
                                       int32 aTimeoutMsec = -1,
                                       uint32 aMultiMaxLen = 0,
                                       Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen = NULL,
                                       Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource = NULL);
        inline void CancelRecvFrom();

    private:
        OsclUDPSocketI(Oscl_DefAlloc &a) : OsclIPSocketI(a), iSendToMethod(NULL),
                iRecvFromMethod(NULL)
        {}

        void ConstructL(OsclSocketServI *aServ,
                        OsclSocketObserver *aObserver,
                        uint32 aId);

        OsclBindMethod *iBindMethod;
        OsclSendToMethod *iSendToMethod;
        OsclRecvFromMethod *iRecvFromMethod;
};

#include "oscl_socket_recv_from.h"
#include "oscl_socket_send_to.h"
#include "oscl_socket_bind.h"

//////////////////////////////////////////////////////////////////////////////////
inline uint8 *OsclUDPSocketI::GetRecvData(int32 *aLength)
{
    return iRecvFromMethod->GetRecvData(aLength);
}

//////////////////////////////////////////////////////////////////////////////////
inline uint8 *OsclUDPSocketI::GetSendData(int32 *aLength)
{
    return iSendToMethod->GetSendData(aLength);
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclUDPSocketI::BindAsync(OsclNetworkAddress& aAddress,
        int32 aTimeoutMsec)
{
    if (!OsclSocketIBase::HasAsyncBind())
        return EPVSocketFailure;//not available.

    iAddress.ipAddr.Set(aAddress.ipAddr.Str());
    iAddress.port = aAddress.port;
    return (iBindMethod->Bind(aAddress, aTimeoutMsec));
}

inline void OsclUDPSocketI::CancelBind()
{
    iBindMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclUDPSocketI::SendTo(const uint8* &aPtr, uint32 aLen,
        OsclNetworkAddress& aAddress,
        int32 aTimeoutMsec)
{
    return (iSendToMethod->SendTo(aPtr, aLen, aAddress, aTimeoutMsec));
}

inline void OsclUDPSocketI::CancelSendTo()
{
    iSendToMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclUDPSocketI::RecvFrom(uint8* &aPtr, uint32 aMaxLen,
        OsclNetworkAddress& aAddress,
        int32 aTimeoutMsec, uint32 aMultiMax,
        Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
        Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource)
{
    return (iRecvFromMethod->RecvFrom(aPtr, aMaxLen, aAddress, aTimeoutMsec, aMultiMax, aPacketLen, aPacketSource));
}

inline void OsclUDPSocketI::CancelRecvFrom()
{
    iRecvFromMethod->CancelMethod();
}

#endif

