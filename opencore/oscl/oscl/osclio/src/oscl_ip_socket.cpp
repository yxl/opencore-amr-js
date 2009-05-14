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

#include "oscl_ip_socket.h"
#include "oscl_error.h"
#include "oscl_socket_imp.h"
#include "pvlogger.h"

void OsclIPSocketI::ConstructL(OsclSocketObserver *aObs, OsclSocketI* aSock, OsclSocketServI* aServ, uint32 aId)
{
    if (!aObs || !aSock || !aServ)
        OsclError::Leave(OsclErrGeneral);
    iObserver = aObs;
    iSocket = aSock;
    iSocketServ = aServ;
    iId = aId;
    iLogger = PVLogger::GetLoggerObject("osclsocket");
}


int32 OsclIPSocketI::Bind(OsclNetworkAddress &aAddress)
{
    if (iSocket)
    {
        iAddress.ipAddr.Set(aAddress.ipAddr.Str());
        iAddress.port = aAddress.port;
        return iSocket->Bind(aAddress);
    }
    else
    {
        return OsclErrGeneral;
    }
}

int32 OsclIPSocketI::Join(OsclNetworkAddress &aAddress)
{
    if (iSocket)
    {
        iAddress.ipAddr.Set(aAddress.ipAddr.Str());
        iAddress.port = aAddress.port;
        return iSocket->Join(aAddress);
    }
    else
    {
        return OsclErrGeneral;
    }
}

int32 OsclIPSocketI::SetRecvBufferSize(uint32 size)
{
    if (iSocket)
    {
        return iSocket->SetRecvBufferSize(size);
    }
    return OsclErrGeneral;
}





