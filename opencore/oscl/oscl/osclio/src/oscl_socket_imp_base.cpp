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
/*!
** Socket implementation base class
*/

#include "oscl_socket_imp_base.h"

OsclSocketIBase::OsclSocketIBase(Oscl_DefAlloc &a): iAlloc(a)
{}

OsclSocketIBase::~OsclSocketIBase()
{
}

void OsclSocketIBase::CancelFxn(TPVSocketFxn aType)
{
    switch (aType)
    {
        case EPVSocketConnect:
            CancelConnect();
            break;
        case EPVSocketAccept:
            CancelAccept();
            break;
        case EPVSocketShutdown:
            CancelShutdown();
            break;
        case EPVSocketSend:
            CancelSend();
            break;
        case EPVSocketRecv:
            CancelRecv();
            break;
        case EPVSocketSendTo:
            CancelSendTo();
            break;
        case EPVSocketRecvFrom:
            CancelRecvFrom();
            break;
        case EPVSocketBind:
            CancelBind();
            break;
        case EPVSocketListen:
            CancelListen();
            break;
        default:
            OSCL_ASSERT(false);
            break;
    }
}

int OsclSocketIBase::GetShutdown(TPVSocketShutdown aOsclVal)
//map socket API value to platform-specific value.
{
    switch (aOsclVal)
    {
        case EPVSocketRecvShutdown:
            return OSCL_SD_RECEIVE;
        case EPVSocketSendShutdown:
            return OSCL_SD_SEND;
        case EPVSocketBothShutdown:
            return OSCL_SD_BOTH;
        default:
            OSCL_ASSERT(0);
    }
    return 0;
}









