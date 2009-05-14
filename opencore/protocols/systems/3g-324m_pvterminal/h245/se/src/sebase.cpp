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
#include "sebase.h"
#include "se.h"
#include "per.h"
#include "h245.h"
#include "oscl_snprintf.h"
#include "pvlogger.h"

SEBase::SEBase() : TimerDuration(TIMER_DURATION), MySE(0), MyPer(0), MyH245(0)
{
    Logger = PVLogger::GetLoggerObject("3g324m.h245.se");
}

void SEBase::MessageSend(PS_H245Msg pMsg)
{
    if (MyPer)
    {
        MyPer->Encode(pMsg);
    }
}

OSCL_EXPORT_REF void SEBase::MessageSend(uint32 type1, uint32 type2, uint8 *pdata)
{
    S_H245Msg h245Msg ;

    h245Msg.Type1 = (uint8)type1;
    h245Msg.Type2 = (uint8)type2 ;
    h245Msg.pData = pdata;

    MessageSend(&h245Msg) ;
}

void SEBase::PrimitiveSend(PS_InfHeader pPrimitive)
{
    if (MyH245)
    {
        MyH245->ObserverMessage((unsigned char*)pPrimitive, sizeof(S_InfHeader));
    }
}

void SEBase::RequestTimer(int32 timerID, int32 timeoutInfo, int32 count, OsclTimerObserver *observer)
{
    if (MySE)
    {
        MySE->RequestTimer(timerID, timeoutInfo, count, observer);
    }
}

void SEBase::CancelTimer(int32 timerID)
{
    if (MySE)
    {
        MySE->CancelTimer(timerID);
    }
}

OSCL_EXPORT_REF void SEBase::Print(const char *format, ...)
{
    OSCL_UNUSED_ARG(format);
    PVLOGGER_LOG_USE_ONLY(va_list args);
    PVLOGGER_LOG_USE_ONLY(va_start(args, format));
    PVLOGGER_LOGMSG_V(PVLOGMSG_INST_LLDBG, Logger, PVLOGMSG_VERBOSE, (0, format, args));
    PVLOGGER_LOG_USE_ONLY(va_end(args));
}
