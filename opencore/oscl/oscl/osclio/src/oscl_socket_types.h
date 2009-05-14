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


#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#define OSCL_SOCKET_TYPES_H_INCLUDED

#include "osclconfig_io.h"
#include "oscl_types.h"
#include "oscl_scheduler_types.h"
#include "oscl_namestring.h"
#include "oscl_stdstring.h"

enum TPVSocketFxn
{
    EPVSocketSend = 0
    , EPVSocketSendTo
    , EPVSocketRecv
    , EPVSocketRecvFrom
    , EPVSocketConnect
    , EPVSocketAccept
    , EPVSocketShutdown
    , EPVSocketBind
    , EPVSocketListen
    , EPVSocket_Last //placeholder
} ;

/** Return codes for asynchronous APIs
*/
enum TPVSocketEvent
{
    EPVSocketSuccess
    , EPVSocketPending
    , EPVSocketTimeout
    , EPVSocketFailure
    , EPVSocketCancel
} ;

enum TPVSocketShutdown
{
    EPVSocketSendShutdown
    , EPVSocketRecvShutdown
    , EPVSocketBothShutdown
} ;

#define PVNETWORKADDRESS_LEN 50

class OsclNetworkAddress
{
    public:
        OsclNetworkAddress(): port(0)
        {
        }
        OsclNetworkAddress(const char *addr, int p)
        {
            ipAddr.Set(addr);
            port = p;
        }

        //Network address in dotted decimal string format.
        OsclNameString<PVNETWORKADDRESS_LEN> ipAddr;

        //Port number.
        int	port;
        //@cmember equality comparison operator
        bool operator == (const OsclNetworkAddress & rhs) const
        {
            if (port == rhs.port)
            {
                if (0 == oscl_strcmp((const char*)ipAddr.Str(), (const char*)rhs.ipAddr.Str()))
                    return true;
            }
            return false;
        };

} ;

/**
* Socket event observer.  The client implements this to get
* asynchronous command completion.
*/
class OsclSocketObserver
{
    public:
        /**
         * Socket Event callback.
         *
         * @param aId: The ID that was supplied when
         *    the socket was created.
         * @param aFxn: Type of socket function call.
         * @param aEvent: Function completion event.  Will be
         *    EPVSocketSuccess, EPVSocketTimeout, or EPVSocketFailure.
         * @param aError: When the event is EPVSocketFailure, this
         *    may contain a platform-specific error code, or zero if
         *    none is available.
         */
        OSCL_IMPORT_REF virtual void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError) = 0;
        virtual ~OsclSocketObserver() {}
};


#endif
