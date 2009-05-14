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

#ifndef OSCL_SOCKET_SERV_IMP_BASE_H_INCLUDED
#define OSCL_SOCKET_SERV_IMP_BASE_H_INCLUDED

#include "oscl_base.h"
#include "oscl_socket_stats.h"

class PVLogger;

/** Socket Server implemenation
*/

/** Base class common to all implementations
*/
class OsclSocketServIBase
{
    public:
        virtual ~OsclSocketServIBase()
        {}

        virtual int32 Connect(uint32 aMessageSlots) = 0;
        virtual void Close(bool) = 0;

    protected:
        OsclSocketServIBase(Oscl_DefAlloc &a)
                : iAlloc(a)
        {
            iServError = 0;
            iServState = ESocketServ_Idle;
            iLogger = NULL;
        }

        Oscl_DefAlloc &iAlloc;

        //server state
        enum TSocketServState
        {
            ESocketServ_Idle
            , ESocketServ_Connected
            , ESocketServ_Error
        };
        TSocketServState iServState;

        TSocketServState State()const
        {
            return iServState;
        }

        bool IsServConnected()const
        {
            return iServState == ESocketServ_Connected;
        }
        int iServError;

#if(PV_OSCL_SOCKET_STATS_LOGGING)
        //server stats
        OsclSocketServStats iServStats;
#endif

    public:
        PVLogger* iLogger;
};



#endif



