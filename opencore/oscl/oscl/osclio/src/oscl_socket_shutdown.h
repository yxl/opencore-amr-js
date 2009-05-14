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

#ifndef OSCL_SOCKET_SHUTDOWN_H_INCLUDED
#define OSCL_SOCKET_SHUTDOWN_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_socket_method.h"

/**
  Internal implementation of Shutdown
  */
class OsclShutdownRequest;

class OsclShutdownMethod : public OsclSocketMethod
{
    public:
        static OsclShutdownMethod *NewL(OsclIPSocketI& c);

        ~OsclShutdownMethod();

        TPVSocketEvent Shutdown(TPVSocketShutdown aHow,
                                int32 aTimeout);

    private:
        OsclShutdownMethod(OsclIPSocketI& c)
                : OsclSocketMethod(c, "osclshutdownmethod", EPVSocketShutdown)
        {}

        void ConstructL();

    public:
        OsclShutdownRequest *ShutdownRequest()
        {
            return (OsclShutdownRequest*)iSocketRequestAO;
        }
};

/**
  This is the AO that interacts with the socket server
  */
class OsclShutdownRequest : public OsclSocketRequestAO
{
    public:
        OsclShutdownRequest(OsclSocketMethod& c)
                : OsclSocketRequestAO(c, "osclShutdownRequest")
        {}

        void Shutdown(TPVSocketShutdown aHow);

    private:
        ShutdownParam*Param()
        {
            return (ShutdownParam*)iParam;
        }
};

#endif

