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

#ifndef OSCL_SOCKET_METHOD_H_INCLUDED
#define OSCL_SOCKET_METHOD_H_INCLUDED

#include "osclconfig_io.h"
#include "oscl_socket_types.h"
//#include "oscl_socket_serv_imp.h"
#include "oscl_scheduler_ao.h"
#include "oscl_socket_request.h"
#include "pvlogger.h"
#include "oscl_socket_tuneables.h"
#include "oscl_ip_socket.h"

#define MSEC_TO_MICROSEC 1000

class OsclSocketI;
class OsclSocketObserver;
class OsclSocketRequestAO;
class OsclIPSocketI;

/** OsclSocketMethod is the base class for all socket methods.
* Two AOs are required for each socket operation-- one to provide
* a timeout, and one to detect request completion.  The OsclSocketMethod
* class implements the timeout and contains the request completion AO.
*/
class OsclSocketMethod : public OsclTimerObject
{
    public:
        OsclSocketMethod(OsclIPSocketI& aContainer, const char *name, TPVSocketFxn fxn)
                : OsclTimerObject(PV_SOCKET_REQUEST_AO_PRIORITY, name)
                , iContainer(aContainer)
                , iSocketFxn(fxn)
                , iSocketRequestAO(NULL)
        {
        }
        virtual ~OsclSocketMethod()
        {}

        void Abort()
        {
            Cancel();
        }

        inline void AbortAll();

        inline void CancelMethod();

        OsclIPSocketI& iContainer;

        TPVSocketFxn iSocketFxn;

        Oscl_DefAlloc& Alloc()
        {
            return iContainer.Alloc();
        }

    protected:
        void ConstructL(OsclSocketRequestAO *aAO)
        {
            if (!aAO)
                OsclError::Leave(OsclErrGeneral);
            iSocketRequestAO = aAO;
        }

        bool StartMethod(int32 aTimeoutMsec);

        inline void MethodDone();

        void Run();

        OsclSocketRequestAO *iSocketRequestAO;
};

#include "oscl_socket_imp.h"

/** This is the base class for all the AOs that
   interact with the socket server.
   This object is contained within an OsclSocketMethod object
*/
class OsclSocketRequestAO : public OsclActiveObject
{
    public:
        void ConstructL()
        {
        }
    protected:

        OsclSocketRequestAO(OsclSocketMethod& aContainer, const char *name)
                : OsclActiveObject(PV_SOCKET_REQUEST_AO_PRIORITY, name)
                , iContainer(aContainer)
                , iSocketError(0)
                , iParam(NULL)
                , iParamSize(0)
        {}

        virtual ~OsclSocketRequestAO()
        {
            CleanupParam(true);
        }

        OsclAny* NewRequest(const uint32 size) ;
        void CleanupParam(bool deallocate = false);


        void Abort()
        {
            Cancel();
        }

        void RequestDone()
        {
            iContainer.Abort();
        }

        inline int GetSocketError();

        void DoCancel()
        {
            SocketI()->CancelFxn(iContainer.iSocketFxn);
            //we launch the cancellation process here.  oscl scheduler
            //will wait on completion of this AO request, which will
            //happen in the server thread.
        }

        void Run();

        virtual void Success()
        {}

        OsclSocketMethod& iContainer;
        int32 iSocketError;
        SocketRequestParam *iParam;
        uint32 iParamSize;

        OsclSocketI *SocketI()
        {
            return iContainer.iContainer.iSocket;
        }

        OsclSocketObserver* SocketObserver()
        {
            return iContainer.iContainer.iObserver;
        }
        uint32 Id()
        {
            return iContainer.iContainer.iId;
        }
        Oscl_DefAlloc& Alloc()
        {
            return iContainer.iContainer.Alloc();
        }

        friend class OsclSocketI;
        friend class OsclSocketMethod;
        friend class OsclSocketRequest;
};

inline void OsclSocketMethod::AbortAll()
{
    Abort();
    if (iSocketRequestAO)
        iSocketRequestAO->Abort();
}

inline void OsclSocketMethod::CancelMethod()
{
    //cancel the timeout if any
    Abort();
    //cancel the request to the socket server.
    iSocketRequestAO->DoCancel();
}

inline void OsclSocketMethod::MethodDone()
{
    iSocketRequestAO->Abort();
}

#endif

