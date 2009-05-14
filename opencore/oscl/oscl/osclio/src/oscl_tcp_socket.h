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

#ifndef OSCL_TCP_SOCKET_H_INCLUDED
#define OSCL_TCP_SOCKET_H_INCLUDED

#ifndef OSCL_IP_SOCKET_H_INCLUDED
#include "oscl_ip_socket.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

class OsclBindMethod;
class OsclListenMethod;
class OsclConnectMethod;
class OsclShutdownMethod;
class OsclAcceptMethod;
class OsclSendMethod;
class OsclRecvMethod;

/**Internal implementation class for OsclTCPSocket
*/
class OsclTCPSocketI : public OsclIPSocketI
{
    public:
        //Synchronous methods
        static OsclTCPSocketI *NewL(Oscl_DefAlloc &a,
                                    OsclSocketServI *aServ,
                                    OsclSocketObserver *aObserver,
                                    uint32 aId);

        virtual ~OsclTCPSocketI();

        int32 Close();
        inline int32 Listen(int aQueueSize);
        //the returned value is platform-specific

        OsclTCPSocketI *GetAcceptedSocketL(uint32 aId);

        inline uint8 *GetRecvData(int32 *aLength) ;
        inline uint8 *GetSendData(int32 *aLength);

        //Asynchronous methods
        inline TPVSocketEvent BindAsync(OsclNetworkAddress& aAddress,
                                        int32 aTimeoutMsec = -1);
        inline void CancelBind();

        inline TPVSocketEvent ListenAsync(uint32 qsize,
                                          int32 aTimeoutMsec = -1);
        inline void CancelListen();

        inline TPVSocketEvent Connect(OsclNetworkAddress& aAddress,
                                      int32 aTimeoutMsec = -1);
        inline void CancelConnect();

        inline TPVSocketEvent Shutdown(TPVSocketShutdown  aHow,
                                       int32 aTimeoutMsec = -1);
        inline void CancelShutdown();

        inline TPVSocketEvent Accept(int32 aTimeout = -1);
        inline void CancelAccept();

        inline TPVSocketEvent Send(const uint8* &aPtr, uint32 aLen,
                                   int32 aTimeoutMsec = -1);
        inline void CancelSend();

        inline TPVSocketEvent Recv(uint8* &aPtr, uint32 aMaxLen,
                                   int32 aTimeoutMsec = -1);
        inline void CancelRecv();

    private:
        static OsclTCPSocketI *NewL(Oscl_DefAlloc &a,
                                    OsclSocketServI *aServ,
                                    OsclSocketI *aSocket,
                                    OsclSocketObserver *aObserver,
                                    uint32 aId);

        OsclTCPSocketI(Oscl_DefAlloc &a) : OsclIPSocketI(a),
                iConnectMethod(NULL),
                iShutdownMethod(NULL),
                iAcceptMethod(NULL),
                iSendMethod(NULL),
                iRecvMethod(NULL)
        {}

        void ConstructL(OsclSocketServI *aServ,
                        OsclSocketObserver *aObserver,
                        uint32 aId);

        void ConstructL(OsclSocketServI *aServ,
                        OsclSocketI *aSocket,
                        OsclSocketObserver *aObserver,
                        uint32 aId);

        OsclBindMethod *iBindMethod;
        OsclListenMethod *iListenMethod;
        OsclConnectMethod *iConnectMethod;
        OsclShutdownMethod *iShutdownMethod;
        OsclAcceptMethod *iAcceptMethod;
        OsclSendMethod *iSendMethod;
        OsclRecvMethod *iRecvMethod;
};

#include "oscl_socket_listen.h"
#include "oscl_socket_recv.h"
#include "oscl_socket_send.h"
#include "oscl_socket_accept.h"
#include "oscl_socket_shutdown.h"
#include "oscl_socket_connect.h"
#include "oscl_socket_bind.h"

//////////////////////////////////////////////////////////////////////////////////
inline int32 OsclTCPSocketI::Listen(int aQueueSize)
{
    return iSocket->Listen(aQueueSize) ;
}

//////////////////////////////////////////////////////////////////////////////////
inline uint8 *OsclTCPSocketI::GetRecvData(int32 *aLength)
{
    return iRecvMethod->GetRecvData(aLength);
}

//////////////////////////////////////////////////////////////////////////////////
inline uint8 *OsclTCPSocketI::GetSendData(int32 *aLength)
{
    return iSendMethod->GetSendData(aLength);
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::BindAsync(OsclNetworkAddress& aAddress,
        int32 aTimeout)
{
    if (!OsclSocketIBase::HasAsyncBind())
        return EPVSocketFailure;//not available.

    iAddress.ipAddr.Set(aAddress.ipAddr.Str());
    iAddress.port = aAddress.port;
    return (iBindMethod->Bind(aAddress, aTimeout));
}

inline void OsclTCPSocketI::CancelBind()
{
    iBindMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::ListenAsync(uint32 qsize,
        int32 aTimeout)
{
    if (!OsclSocketIBase::HasAsyncListen())
        return EPVSocketFailure;//not available

    return (iListenMethod->Listen(qsize, aTimeout));
}

inline void OsclTCPSocketI::CancelListen()
{
    iListenMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::Connect(OsclNetworkAddress& aAddress,
        int32 aTimeout)
{
    return (iConnectMethod->Connect(aAddress, aTimeout));
}

inline void OsclTCPSocketI::CancelConnect()
{
    iConnectMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::Shutdown(TPVSocketShutdown  aHow,
        int32 aTimeout)
{
    return (iShutdownMethod->Shutdown(aHow, aTimeout));
}

inline void OsclTCPSocketI::CancelShutdown()
{
    iShutdownMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::Accept(int32 aTimeout)
{
    return (iAcceptMethod->Accept(aTimeout));
}

inline void OsclTCPSocketI::CancelAccept()
{
    iAcceptMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::Send(const uint8* &aPtr, uint32 aLen,
        int32 aTimeoutMsec)
{
    return (iSendMethod->Send(aPtr, aLen, aTimeoutMsec));
}

inline void OsclTCPSocketI::CancelSend()
{
    iSendMethod->CancelMethod();
}

//////////////////////////////////////////////////////////////////////////////////
inline TPVSocketEvent OsclTCPSocketI::Recv(uint8* &aPtr, uint32 aMaxLen,
        int32 aTimeout)
{
    return (iRecvMethod->Recv(aPtr, aMaxLen, aTimeout));
}

inline void OsclTCPSocketI::CancelRecv()
{
    iRecvMethod->CancelMethod();
}

#endif

