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
** Socket statistics
*/

#include "oscl_socket_tuneables.h"
#if(PV_OSCL_SOCKET_STATS_LOGGING)
#include "oscl_socket_stats.h"
#include "oscl_tickcount.h"
#include "pvlogger.h"
#include "oscl_socket_tuneables.h"
#include "oscl_scheduler_ao.h"
#include "oscl_snprintf.h"

#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_DEBUG,m);

#define PV_SOCKET_STR_LEN 512

static const char* const TPVSocketFxnStrShort[] =
{
    "Send    "
    , "SendTo  "
    , "Recv    "
    , "RecvFrom"
    , "Connect "
    , "Accept  "
    , "Shutdown"
    , "Bind"
    , "Listen"
} ;

OsclSocketStats::OsclSocketStats()
{
    iLogStr = (char*)OSCL_MALLOC(PV_SOCKET_STR_LEN);
    iLogger = PVLogger::GetLoggerObject("OsclSocketStats");
    iSock = NULL;
    iServ = NULL;
    for (uint32 i = 0;i < OsclSocketStats_NUM_ACTIVE_REQUESTS;i++)
    {
        iLastCompletionTime[i] = (-1);
        iExcessPollEvents[i] = 0;
    }
}
OsclSocketStats::~OsclSocketStats()
{
    if (iLogStr)
        OSCL_FREE(iLogStr);
}
void OsclSocketStats::Construct(OsclAny* aContainer, OsclAny*aServ)
{
    iSock = aContainer;
    iServ = aServ;
}

void OsclSocketStats::Add(TPVSocketFxn aFxn, TOsclSocketStatEvent aEvent, int32 aParam)
{
    iLock.Lock();
    if (aEvent == EOsclSocket_ServPoll)
    {
        if (iPollEvents[aFxn].size() >= OsclSocketStats_MAX_POLL_EVENTS)
        {//to avoid having the poll events vector grow without bound
            iExcessPollEvents[aFxn]++;
        }
        else
        {
            OsclSocketStatEventEntry event(aEvent, OsclTickCount::TickCount(), iEvents[aFxn].size());
            iPollEvents[aFxn].push_back(event);
        }
    }
    else
    {
        OsclSocketStatEventEntry event(aEvent, OsclTickCount::TickCount(), aParam);
        iEvents[aFxn].push_back(event);
    }
    iLock.Unlock();
}
void OsclSocketStats::Clear(TPVSocketFxn aFxn)
{
    iLock.Lock();
    if (iEvents[aFxn].size())
        iLastCompletionTime[aFxn] = iEvents[aFxn][iEvents[aFxn].size() - 1].iTicks;
    iEvents[aFxn].clear();
    iPollEvents[aFxn].clear();
    iExcessPollEvents[aFxn] = 0;
    iLock.Unlock();
}

void OsclSocketStats::Log(TPVSocketFxn aFxn)
{
    //log events
    {
        int32 len = 0;
        iLogStr[0] = '\0';
        for (uint32 i = 0;i < iEvents[aFxn].size();i++)
        {
            if (PV_SOCKET_STR_LEN - len <= 0)
                break;
            if (iEvents[aFxn][i].iParam != (-1))
            {
                //log event with a parameter
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "%s (n=%d) %d "
                                     , TPVSocketStatStr[iEvents[aFxn][i].iEvent]
                                     , iEvents[aFxn][i].iParam
                                     , iEvents[aFxn][i].iTicks);
            }
            else
            {
                //log event with no parameter
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "%s %d "
                                     , TPVSocketStatStr[iEvents[aFxn][i].iEvent]
                                     , iEvents[aFxn][i].iTicks);
            }
        }
        LOGINFOMED((0, "OsclSocketStats(Serv=0x%x,Sock=0x%x): %s delta=%d, %s"
                    , iServ, iSock
                    , TPVSocketFxnStrShort[aFxn]
                    , (iLastCompletionTime[aFxn] == (-1)) ? (-1) : iEvents[aFxn][iEvents[aFxn].size() - 1].iTicks - iLastCompletionTime[aFxn]
                    , iLogStr));
    }

    //log polling events
    {
        int32 len = 0;
        iLogStr[0] = '\0';
        int32 lastIndex = (-1);
        for (uint32 i = 0;i < iPollEvents[aFxn].size();i++)
        {
            if (PV_SOCKET_STR_LEN - len <= 0)
                break;
            if (iPollEvents[aFxn][i].iParam != lastIndex)
            {
                //log polling index plus times.
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "(%d) %d "
                                     , iPollEvents[aFxn][i].iParam
                                     , iPollEvents[aFxn][i].iTicks
                                    );
            }
            else
            {
                //log time only
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "%d "
                                     , iPollEvents[aFxn][i].iTicks
                                    );
            }
            lastIndex = iPollEvents[aFxn][i].iParam;
        }
        LOGINFOMED((0, "OsclSocketStats(Serv=0x%x,Sock=0x%x): %s Poll=%d, Times %s"
                    , iServ, iSock
                    , TPVSocketFxnStrShort[aFxn]
                    , iPollEvents[aFxn].size()
                    , iLogStr));
        if (iExcessPollEvents[aFxn] > 0)
        {
            LOGINFOMED((0, "OsclSocketStats(Serv=0x%x,Sock=0x%x): %s Num ExcessPollEvents=%d"
                        , iServ, iSock
                        , TPVSocketFxnStrShort[aFxn]
                        , iExcessPollEvents[aFxn]
                       ));
        }
    }
}

void OsclSocketStats::Log()
{
    //log events not related to a particular socket API
    {
        int32 len = 0;
        iLogStr[0] = '\0';
        for (uint32 i = 0;i < iEvents[0].size();i++)
        {
            if (PV_SOCKET_STR_LEN - len <= 0)
                break;
            if (iEvents[0][i].iParam != (-1))
            {
                //log event with a parameter
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "%s (n=%d) %d "
                                     , TPVSocketStatStr[iEvents[0][i].iEvent]
                                     , iEvents[0][i].iParam
                                     , iEvents[0][i].iTicks);
            }
            else
            {
                //log event with no parameter
                len += oscl_snprintf(iLogStr + len, PV_SOCKET_STR_LEN - len, "%s %d "
                                     , TPVSocketStatStr[iEvents[0][i].iEvent]
                                     , iEvents[0][i].iTicks);
            }
        }
        LOGINFOMED((0, "OsclSocketStats(Serv=0x%x,Sock=0x%x): %s"
                    , iServ, iSock
                    , iLogStr));
    }
}

void OsclSocketStats::LogAndDump()
{
    for (uint32 i = 0;i < OsclSocketStats_NUM_ACTIVE_REQUESTS;i++)
    {
        if (iEvents[i].size()
                || iPollEvents[i].size())
        {
            Log((TPVSocketFxn)i);
        }
        Clear((TPVSocketFxn)i);
    }
}


OsclSocketServStats::OsclSocketServStats()
{
    iLogger = PVLogger::GetLoggerObject("OsclSocketStats");
    iServ = NULL;
    Clear();
}
OsclSocketServStats::~OsclSocketServStats()
{
}
void OsclSocketServStats::Construct(OsclAny*aServ)
{
    iServ = aServ;
}

void OsclSocketServStats::ShowTuneables()
{
    //Log socket server type
#if  (PV_SOCKET_SERVER)
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server: PV Socket Server", iServ));
#else
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server: Unknown Socket Server", iServ));
#endif
    //Log socket request AO priority
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Request AO Priority %d"
                , iServ
                , PV_SOCKET_REQUEST_AO_PRIORITY));

#if(PV_SOCKET_SERVER)
    //Log pv socket server type
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server is %s"
                , iServ
                , (PV_SOCKET_SERVER_IS_THREAD) ? "Threaded" : "AO-Based"
               ));
#if(PV_SOCKET_SERVER_IS_THREAD)
    //pv server threaded params
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server Thread Priority %d", iServ, PV_SOCKET_SERVER_THREAD_PRIORITY));
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server Select Timeout (msec) %d", iServ, PV_SOCKET_SERVER_SELECT_TIMEOUT_MSEC));
#else
    //pv server AO-based params
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server AO Priority %d", iServ, PV_SOCKET_SERVER_AO_PRIORITY));
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server AO Polling Interval (msec) %d", iServ, PV_SOCKET_SERVER_AO_INTERVAL_MSEC));
#endif

    //pv Server logging
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Socket Server Logging %s"
                , iServ
                , (PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT) ? "Enabled" : "Disabled"));

#endif//PV_SOCKET_SERVER
}

void OsclSocketServStats::Add(TOsclSocketServStatEvent aEvent)
{
    iEvents[aEvent]++;
}

void OsclSocketServStats::Log(TOsclSocketServStatEvent aEvent)
{
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): Event %s Count %d"
                , iServ
                , TPVSocketServStatStr[aEvent]
                , iEvents[aEvent]));
}

void OsclSocketServStats::Clear()
{
    for (uint32 i = 0;i < EOsclSocketServ_LastEvent;i++)
        iEvents[i] = 0;
}

void OsclSocketServStats::LogAndDump()
{
    if (!iServ)
        return;
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): ---Begin Stats--------", iServ));
    ShowTuneables();
    for (uint32 i = 0;i < EOsclSocketServ_LastEvent;i++)
        Log((TOsclSocketServStatEvent)i);
    LOGINFOMED((0, "OsclSocketStats(Serv=0x%x): ---End Stats----------", iServ));
    Clear();
    //this acts as thread logoff
    iServ = NULL;
}

#endif






