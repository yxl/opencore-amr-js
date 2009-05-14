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

#include "osclconfig_io.h"
#include "oscl_socket_tuneables.h"

#if(PV_SOCKET_SERVER)

#include "oscl_scheduler_ao.h"
#include "oscl_socket_method.h"
#include "oscl_socket_types.h"
#include "oscl_socket_serv_imp_pv.h"
#include "oscl_error.h"
#include "oscl_socket_imp.h"
#include "oscl_assert.h"

//Logger macro for socket server logging.
#if(PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT)
#include "pvlogger.h"
#define LOGSERV(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);
#else
#define LOGSERV(m)
#endif
/*
//disable this if it's too much
#undef LOGSERV
#define LOGSERV
*/

//Macros for server stats logging for use with OsclSocketServI
#if (PV_OSCL_SOCKET_STATS_LOGGING)
#define ADD_STATS(x) iServStats.Add(x)
#define ADD_STATSP(x,y) iServStats.Add(x,y)
#define CONSTRUCT_STATS(x) iServStats.Construct(x)
#define DUMP_STATS iServStats.LogAndDump();
#else
#define ADD_STATS(x)
#define ADD_STATSP(x,y)
#define CONSTRUCT_STATS(x)
#define DUMP_STATS
#endif//PV_OSCL_SOCKET_STATS_LOGGING


#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
//
//OsclSocketServI::LoopbackSocket
//

//Macros for server stats logging for use with OsclSocketServI::LoopbackSocket
#if (PV_OSCL_SOCKET_STATS_LOGGING)
#define ADD_LOOPSTATS(a,b) iStats.Add((TPVSocketFxn)0,a,b);
#define LOG_LOOPSTATS iStats.Log();
#define DUMP_LOOPSTATS iStats.Log();iStats.Clear((TPVSocketFxn)0);
#else
#define ADD_LOOPSTATS(a,b)
#define LOG_LOOPSTATS
#define DUMP_LOOPSTATS
#endif//PV_OSCL_SOCKET_STATS_LOGGING

void OsclSocketServI::LoopbackSocket::Init(OsclSocketServI* aContainer)
//This will intialize the loopback socket that is used
//to wakeup a blocking select call.
{
    iContainer = aContainer;
#if PV_OSCL_SOCKET_STATS_LOGGING
    iStats.Construct(NULL, this);
#endif

    //create the socket
    bool ok;
    int err;
    OsclSocket(iSocket, OSCL_AF_INET, OSCL_SOCK_DATAGRAM, OSCL_IPPROTO_UDP, ok, err);
    if (!ok)
    {
        ADD_LOOPSTATS(EOsclSocketServ_LoopsockError, err);
        return ;
    }

    //set it to non-blocking mode
    OsclSetNonBlocking(iSocket, ok, err);
    if (!ok)
    {//set non-blocking mode failed
        ADD_LOOPSTATS(EOsclSocketServ_LoopsockError, err);
        OsclCloseSocket(iSocket, ok, err);
        return ;
    }

    //bind to any available port between 5000 and 10000.
    OsclNetworkAddress myAddr("127.0.0.1", 5000);
    while (myAddr.port < 10000)
    {
        OsclSocketI::MakeAddr(myAddr, iAddr);
        OsclBind(iSocket, iAddr, ok, err);
        if (!ok)
        {
            myAddr.port++;
        }
        else
        {
            break;//bind success!
        }
    }

    if (!ok)
    {//bind failed
        ADD_LOOPSTATS(EOsclSocketServ_LoopsockError, err);
        OsclCloseSocket(iSocket, ok, err);
        return ;
    }

    //let's test a send & recv here just to be extra sure
    //the loopsock is usable.
    const char tmpBuf[2] = {0, 0};
    int nbytes;
    bool wouldblock;
    wouldblock = false;
    ok = false;
    nbytes = 0;
    OsclSendTo(iSocket, tmpBuf, 1, iAddr, ok, err, nbytes, wouldblock);
    if (!ok)
    {
        if (wouldblock)
        {
            //just skip the test
            ok = true;
        }
        else
        {
            //send failed.
            ADD_LOOPSTATS(EOsclSocketServ_LoopsockError, err);
            OsclCloseSocket(iSocket, ok, err);
            return ;
        }
    }
    else
    {
        //send ok-- now try a recv
        TOsclSockAddr sourceaddr;
        TOsclSockAddrLen sourceaddrlen = sizeof(sourceaddr);
        ok = false;
        nbytes = 0;
        OsclRecvFrom(iSocket, tmpBuf, sizeof(tmpBuf),
                     &sourceaddr,
                     &sourceaddrlen,
                     ok,
                     err,
                     nbytes,
                     wouldblock);
        if (!ok)
        {
            if (wouldblock)
            {
                //just skip the test
                ok = true;
            }
            else
            {//recv failed.
                ADD_LOOPSTATS(EOsclSocketServ_LoopsockError, err);
                OsclCloseSocket(iSocket, ok, err);
                return ;
            }
        }
    }

    //loopsock is ready to use
    ADD_LOOPSTATS(EOsclSocketServ_LoopsockOk, myAddr.port);
#if PV_OSCL_SOCKET_STATS_LOGGING
    iStats.Construct((OsclAny*)iSocket, this);
#endif
    iEnable = true;
}

void OsclSocketServI::LoopbackSocket::Cleanup()
//This will intialize the loopback socket that is used
//to wakeup a blocking select call.
{
    if (iEnable)
    {
        //close the socket
        bool ok;
        int sockerr;
        OsclCloseSocket(iSocket, ok, sockerr);
        iEnable = false;
        DUMP_LOOPSTATS;
    }
}

void OsclSocketServI::LoopbackSocket::Read()
{
    if (!iEnable)
        return;

    //read all queued data on the socket
    if (FD_ISSET(iSocket, &iContainer->iReadset))
    {
        char tmpBuf[2] = {0, 0};
        int nbytes, err;
        bool ok, wouldblock;
        TOsclSockAddr sourceaddr;
        TOsclSockAddrLen sourceaddrlen = sizeof(sourceaddr);

        bool recv = true;
        while (recv)
        {
            OsclRecvFrom(iSocket, tmpBuf, sizeof(tmpBuf),
                         &sourceaddr,
                         &sourceaddrlen,
                         ok,
                         err,
                         nbytes,
                         wouldblock);
            recv = (ok && nbytes > 0);
        }
    }
}

void OsclSocketServI::LoopbackSocket::ProcessSelect(TOsclSocket& maxsocket)
//Do the necessary select loop processing to keep
//the loopback socket going.
{
    if (!iEnable)
        return;

    //Monitor this socket whenever we will be doing a select.
    if (maxsocket)
    {
        FD_SET(iSocket, &iContainer->iReadset);
        if (iSocket > maxsocket)
            maxsocket = iSocket;
    }
}

void OsclSocketServI::LoopbackSocket::Write()
//Write to the loopback socket
{
    if (!iEnable)
        return;

    char tmpBuf[2] = {0, 0};
    int nbytes, err;
    bool wouldblock, ok;
    OsclSendTo(iSocket, tmpBuf, 1, iAddr, ok, err, nbytes, wouldblock);

    //if send failed, the select call will hang forever, so just go ahead and abort now.
    OSCL_ASSERT(ok);
}
#endif //#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET

// Socket server stats for winmobile perf investigation.
#if (PV_SOCKET_SERVI_STATS)

//breakdown of time in the Session
enum TServiSessionStats
{
    EServiSession_All
    , EServiSession_Last
};

//breakdown of time in the Run() call
enum TServiRunStats
{
    EServiRun_Proc = EServiSession_Last
    , EServiRun_Select
    , EServiRun_Reschedule
    , EServiRun_Last
};
//breakdown of time in the ProcessSocketRequests call
enum TServiProcStats
{
    EServiProc_Queue = EServiRun_Last
    , EServiProc_Loop
    , EServiProc_Fdset
    , EServiProc_Last
};
//breakdown of time in the ProcessXXX calls in ProcessSocketRequests
enum TServiProcLoopStats
{
    EServiProcLoop_Cancel = EServiProc_Last
    , EServiProcLoop_Closed
    , EServiProcLoop_Connect
    , EServiProcLoop_Accept
    , EServiProcLoop_Shutdown
    , EServiProcLoop_Recv
    , EServiProcLoop_Send
    , EServiProcLoop_RecvFrom
    , EServiProcLoop_SendTo
    , EServiProcLoop_Last
};
static const char* const TServiStr[] =
{
    "EServiSession_All"
    , "EServiRun_Proc"
    , "EServiRun_Select"
    , "EServiRun_Reschedule"
    , "EServiProc_Queue"
    , "EServiProc_Loop"
    , "EServiProc_Fdset"
    , "EServiProcLoop_Cancel"
    , "EServiProcLoop_Closed"
    , "EServiProcLoop_Connect"
    , "EServiProcLoop_Accept"
    , "EServiProcLoop_Shutdown"
    , "EServiProcLoop_Recv"
    , "EServiProcLoop_Send"
    , "EServiProcLoop_RecvFrom"
    , "EServiProcLoop_SendTo"
};


#define PV_SERVI_STATS_SIZE (EServiProcLoop_Last)
#include "oscl_int64_utils.h"

class PVServiStats
{
    public:
        //use oscl tick count
        typedef uint32 TPVTick;
        const char* Tickstr()
        {
            return "Ticks";
        }
        void settick(TPVTick &tick)
        {
            tick = OsclTickCount::TickCount();
        }
        int64 tickint(TPVTick& tick)
        {
            return tick;
        }

        PVServiStats(): iCount(0), iRunningTotal(0)
        {}
        ~PVServiStats()
        {}

        uint32 iCount;
        int64 iRunningTotal;

        TPVTick _start;

        //start an interval
        void Start()
        {
            settick(_start);
        }

        //end an interval
        void End()
        {
            TPVTick now;
            settick(now);
            int32 delta = tickint(now) - tickint(_start);
            //winmobile clock sometimes goes backward so treat any negative delta as zero.
            if (delta > 0)
                iRunningTotal += delta;
            iCount++;
        }
        //end a long interval
        void LongEnd()
        {
            TPVTick now;
            settick(now);
            int64 delta = tickint(now) - tickint(_start);
            //winmobile clock sometimes goes backward so treat any negative delta as zero.
            if (delta > 0)
                iRunningTotal += delta;
            iCount++;
        }

        static PVServiStats* Create()
        {
            _OsclBasicAllocator alloc;
            OsclAny* ptr = alloc.allocate(PV_SERVI_STATS_SIZE * sizeof(PVServiStats));
            PVServiStats* ptr2 = (PVServiStats*)ptr;
            for (uint32 i = 0;i < PV_SERVI_STATS_SIZE;i++)
            {
                new(ptr2++) PVServiStats();
            }
            return (PVServiStats*)ptr;
        }

        static void Destroy(PVServiStats* aStats)
        {
            if (aStats)
            {
                _OsclBasicAllocator alloc;
                //don't bother with destructor since it's empty.
                alloc.deallocate(aStats);
            }
        }

        void ShowStats(PVLogger* logger, int32 aIndex, PVServiStats& aInterval)
        {
            if (!iCount)
                return;//don't print any that didn't run
            if (aInterval.iRunningTotal == (int64)0)
                return;//to avoid div by zero

            float percent = 100.0 * iRunningTotal / aInterval.iRunningTotal;
            int32 fraction = (int32)percent;
            float decimal = percent - fraction;
            decimal *= 100.0;

            //print results
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO
                            , (0, "  TIME PERCENT %d.%02d, Interval '%s', Count %d, TotalTicks Hi,Lo (0x%x,0x%08x)"
                               , (int32)percent, (int32)decimal
                               , TServiStr[aIndex]
                               , iCount
                               , Oscl_Int64_Utils::get_int64_upper32(iRunningTotal)
                               , Oscl_Int64_Utils::get_int64_lower32(iRunningTotal)
                              ));
        }

        static void ShowSummaryStats(PVServiStats* aArray)
        {
            //lump this logging with Oscl scheduler perf logging.
            uint32 index = 0;
            PVLogger* logger = PVLogger::GetLoggerObject("OsclSchedulerPerfStats");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO, (0, "Session Breakdown:"));
            while (index < EServiSession_Last)
            {
                aArray[index].ShowStats(logger, index, aArray[EServiSession_All]);
                index++;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO, (0, "Run Breakdown:"));
            while (index < EServiRun_Last)
            {
                aArray[index].ShowStats(logger, index, aArray[EServiSession_All]);
                index++;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO, (0, "ProcessRequests Breakdown:"));
            while (index < EServiProc_Last)
            {
                aArray[index].ShowStats(logger, index, aArray[EServiSession_All]);
                index++;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO, (0, "ProcessRequests Loop Breakdown:"));
            while (index < EServiProcLoop_Last)
            {
                aArray[index].ShowStats(logger, index, aArray[EServiSession_All]);
                index++;
            }
        }
};
#define START_SERVI_STATS(x) iServiStats[x].Start()
#define END_SERVI_STATS(x) iServiStats[x].End()
#define LONG_END_SERVI_STATS(x) iServiStats[x].LongEnd()
//2nd level of detail-- switched off for now.
#define START_SERVI_STATS2(x) iServiStats[x].Start()
#define END_SERVI_STATS2(x) iServiStats[x].End()
#else
#define START_SERVI_STATS(x)
#define END_SERVI_STATS(x)
#define LONG_END_SERVI_STATS(x)
#define START_SERVI_STATS2(x)
#define END_SERVI_STATS2(x)
#endif //PV_SOCKET_SERVI_STATS

//
//OsclSocketServI-- PV implementation
//

#if(PV_SOCKET_SERVER_IS_THREAD)
OsclSocketServI::OsclSocketServI(Oscl_DefAlloc &a)
        : OsclSocketServIBase(a)
{
}
#else
OsclSocketServI::OsclSocketServI(Oscl_DefAlloc &a)
        : OsclTimerObject(PV_SOCKET_SERVER_AO_PRIORITY, "OsclSocketServI")
        , OsclSocketServIBase(a)
{
}
#endif

OsclSocketServI::~OsclSocketServI()
{
    Close(false);
    CleanupServImp();
}

OsclSocketServI* OsclSocketServI::NewL(Oscl_DefAlloc &a)
{
    OsclAny*p = a.ALLOCATE(sizeof(OsclSocketServI));
    OsclError::LeaveIfNull(p);
    OsclSocketServI *self = OSCL_PLACEMENT_NEW(p, OsclSocketServI(a));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclSocketServI::ConstructL()
{
    iSockServRequestList.iActiveRequests.reserve(6);
    ConstructServImp();
    CONSTRUCT_STATS(this);
}

int32 OsclSocketServI::Connect(uint32 aMessageSlots)
{
    CONSTRUCT_STATS(this);

#if (PV_SOCKET_SERVI_STATS)
    iServiStats = PVServiStats::Create();
    START_SERVI_STATS(EServiSession_All);
#endif

    //Connect to Oscl socket server

    OSCL_UNUSED_ARG(aMessageSlots);

    //should only connect once
    if (iServState == ESocketServ_Connected)
    {
        return OsclErrGeneral;
    }

#ifdef OsclSocketStartup
    //startup the socket system.
    bool ok;
    OsclSocketStartup(ok);
    if (!ok)
    {
        return OsclErrGeneral;
    }
#endif//OsclSocketStartup

    iServState = ESocketServ_Idle;

    //Start the server thread or AO
    int32 err = StartServImp();
    if (err != OsclErrNone)
    {
        return err;
    }

    //check state.
    if (iServState != ESocketServ_Connected)
    {
        //cleanup after a failure.
        Close(false);
        return OsclErrGeneral;
    }
    return OsclErrNone;
}

void OsclSocketServI::Close(bool aCleanup)
{
    //Close oscl socket server

    if (iServState == ESocketServ_Connected)
    {
        StopServImp();
    }

#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
    iLoopbackSocket.Cleanup();
#endif

#ifdef OsclSocketCleanup
    //close the socket system
    if (aCleanup)
    {
        bool ok;
        OsclSocketCleanup(ok);
    }
#endif//OsclSocketCleanup

    DUMP_STATS;

#if (PV_SOCKET_SERVI_STATS)
    if (iServiStats)
    {
        LONG_END_SERVI_STATS(EServiSession_All);
        PVServiStats::ShowSummaryStats(iServiStats);
        PVServiStats::Destroy(iServiStats);
        iServiStats = NULL;
    }
#endif
}

bool OsclSocketServI::IsServerThread()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    TOsclThreadId tid;
    OsclThread::GetId(tid);
    return tid == iThreadId;
#else
    return true;
#endif
}

/**
 * Process all active socket requests.
 *
 * This is called under the server thread or server AO.
 *
 * @param aNhandles(input/output): nHandles returned by last Select call.
 * @param aNfds(output): Value = 1+maximum socket handle for all sockets
 *    that we are monitoring with the select call.
 */
#if PV_SOCKET_SERVER_SELECT
void OsclSocketServI::ProcessSocketRequests(int& aNhandles, int &aNfds)
#else
void OsclSocketServI::ProcessSocketRequests()
#endif
{
    //process all active requests

#if PV_SOCKET_SERVER_SELECT
    //keep track of max socket handle to monitor.
    TOsclSocket maxsocket = 0;
    aNfds = (int)maxsocket + 1;

    //save input handle count, then clear it until the next select operation.
    int nhandles = aNhandles;
    aNhandles = 0;
#endif

    // Pick up new requests from the app thread.
    START_SERVI_STATS2(EServiProc_Queue);
    iSockServRequestList.Lock();
    {
        iSockServRequestList.GetNewRequests();

#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
        //flush any data on the loopback socket.
        iLoopbackSocket.Read();
#endif
    }
    iSockServRequestList.Unlock();
    END_SERVI_STATS2(EServiProc_Queue);

    if (iSockServRequestList.iActiveRequests.empty())
    {
        //nothing to do!
        return;
    }

    //Make a pass through the open request list and cancel or process each request.
    START_SERVI_STATS2(EServiProc_Loop);
    uint32 i;
    for (i = 0;i < iSockServRequestList.iActiveRequests.size();i++)
    {
        OsclSocketServRequestQElem* elem = &iSockServRequestList.iActiveRequests[i];

        if (elem->iCancel)
        {
            //Request was canceled
            START_SERVI_STATS2(EServiProcLoop_Cancel);
            elem->iSocketRequest->Complete(elem, OSCL_REQUEST_ERR_CANCEL);
            END_SERVI_STATS2(EServiProcLoop_Cancel);
        }
        else if (!IsServConnected())
        {
            //Server died or was closed.
            START_SERVI_STATS2(EServiProcLoop_Closed);
            elem->iSocketRequest->Complete(elem, OSCL_REQUEST_ERR_GENERAL
                                           , (iServError) ? iServError : PVSOCK_ERR_SERV_NOT_CONNECTED);
            END_SERVI_STATS2(EServiProcLoop_Closed);
        }
#if PV_SOCKET_SERVER_SELECT
        else if (nhandles == 0 && elem->iSelect)
        {
            //we're monitoring this socket but there is no current
            //socket activity-- just keep waiting.
            ;
        }
#endif
        else
        {
            //These routines will start the request, or else process
            //the results of prior select call, and also set the select
            //flags for the next call.

            switch (elem->iSocketRequest->Fxn())
            {
                case EPVSocketShutdown:
                    START_SERVI_STATS2(EServiProcLoop_Shutdown);
                    elem->iSocketRequest->iSocketI->ProcessShutdown(elem);
                    END_SERVI_STATS2(EServiProcLoop_Shutdown);
                    break;

                case EPVSocketConnect:
                    START_SERVI_STATS2(EServiProcLoop_Connect);
                    elem->iSocketRequest->iSocketI->ProcessConnect(elem);
                    END_SERVI_STATS2(EServiProcLoop_Connect);
                    break;

                case EPVSocketAccept:
                    START_SERVI_STATS2(EServiProcLoop_Accept);
                    elem->iSocketRequest->iSocketI->ProcessAccept(elem);
                    END_SERVI_STATS2(EServiProcLoop_Accept);
                    break;

                case EPVSocketSend:
                    START_SERVI_STATS2(EServiProcLoop_Send);
                    elem->iSocketRequest->iSocketI->ProcessSend(elem);
                    END_SERVI_STATS2(EServiProcLoop_Send);
                    break;

                case EPVSocketSendTo:
                    START_SERVI_STATS2(EServiProcLoop_SendTo);
                    elem->iSocketRequest->iSocketI->ProcessSendTo(elem);
                    END_SERVI_STATS2(EServiProcLoop_SendTo);
                    break;

                case EPVSocketRecv:
                    START_SERVI_STATS2(EServiProcLoop_Recv);
                    elem->iSocketRequest->iSocketI->ProcessRecv(elem);
                    END_SERVI_STATS2(EServiProcLoop_Recv);
                    break;

                case EPVSocketRecvFrom:
                    START_SERVI_STATS2(EServiProcLoop_RecvFrom);
                    elem->iSocketRequest->iSocketI->ProcessRecvFrom(elem);
                    END_SERVI_STATS2(EServiProcLoop_RecvFrom);
                    break;

                default:
                    OSCL_ASSERT(0);
                    break;
            }
        }
    }
    END_SERVI_STATS2(EServiProc_Loop);

    //Zero out any old select set
    START_SERVI_STATS2(EServiProc_Fdset);
#if PV_SOCKET_SERVER_SELECT
    FD_ZERO(&iReadset);
    FD_ZERO(&iWriteset);
    FD_ZERO(&iExceptset);
#endif
    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests Clearing select set"));

    //Now make a pass to either delete the request or collate the select flags.
    for (i = 0;i < iSockServRequestList.iActiveRequests.size();)
    {
        OsclSocketServRequestQElem* elem = &iSockServRequestList.iActiveRequests[i];

        if (elem->iSocketRequest)
        {
            //request is still active
            i++;

#if PV_SOCKET_SERVER_SELECT
            if (elem->iSelect > 0)
            {
                //Need to do a select call for this socket

                TOsclSocket osock = elem->iSocketRequest->iSocketI->Socket();

                if (osock > maxsocket)
                {
                    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests Setting Maxsocket to %d", osock));
                    maxsocket = osock;
                }

                //Add the socket to the select set.  Keep in mind there can be multiple requests
                //per socket, so check whether the socket is already added before adding.

                if ((elem->iSelect & OSCL_READSET_FLAG) == OSCL_READSET_FLAG
                        && !(FD_ISSET(osock, &iReadset)))
                {
                    FD_SET(osock, &iReadset);
                    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests Setting Readset for %d", osock));
                }

                if ((elem->iSelect & OSCL_WRITESET_FLAG) == OSCL_WRITESET_FLAG
                        && !(FD_ISSET(osock, &iWriteset)))
                {
                    FD_SET(osock, &iWriteset);
                    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests Setting Writeset for %d", osock));
                }

                if ((elem->iSelect & OSCL_EXCEPTSET_FLAG) == OSCL_EXCEPTSET_FLAG
                        && !(FD_ISSET(osock, &iExceptset)))
                {
                    FD_SET(osock, &iExceptset);
                    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests Setting Exceptset for %d", osock));
                }
            }
#endif
        }
        else
        {
            //request is complete and can be deleted.
            iSockServRequestList.iActiveRequests.erase(elem);
        }
    }
    END_SERVI_STATS2(EServiProc_Fdset);

#if PV_SOCKET_SERVER_SELECT
    if (maxsocket)
    {
#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
        //also monitor the loopback socket if we're going to call select.
        iLoopbackSocket.ProcessSelect(maxsocket);
#endif

        //set Nfds to 1+maxsocket handle.
        aNfds = (int)maxsocket + 1;
    }

    LOGSERV((0, "OsclSocketServI::ProcessSocketRequests NFDS %d", aNfds));
#endif

}

void OsclSocketServI::ServerEntry()
//Server entry processing
{
    iLogger = PVLogger::GetLoggerObject("osclsocket_serv");

    iServError = 0;
    iServState = OsclSocketServI::ESocketServ_Connected;

    iSockServRequestList.Open(this);

#if PV_SOCKET_SERVER_SELECT
    FD_ZERO(&iReadset);
    FD_ZERO(&iWriteset);
    FD_ZERO(&iExceptset);
#endif
}

void OsclSocketServI::ServerExit()
//Server exit processing
{
    //change state if this was a normal exit.
    if (iServState == OsclSocketServI::ESocketServ_Connected)
    {
        iServState = OsclSocketServI::ESocketServ_Idle;
    }

    //Go through the active requests one last time.
    //All the requests will complete with errors
    //since the server is no longer connected.
#if PV_SOCKET_SERVER_SELECT
    int nfds;
    int nhandles = 0;
    ProcessSocketRequests(nhandles, nfds);
#else
    ProcessSocketRequests();
#endif

    iSockServRequestList.Close();

    //make sure sets are clear so resources get cleaned up.
#if PV_SOCKET_SERVER_SELECT
    FD_ZERO(&iReadset);
    FD_ZERO(&iWriteset);
    FD_ZERO(&iExceptset);
#endif
}

void OsclSocketServI::ConstructServImp()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    iClose = false;
    iStart.Create();
    iExit.Create();
#endif
}

void OsclSocketServI::CleanupServImp()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    iStart.Close();
    iExit.Close();
#endif
}

#if(PV_SOCKET_SERVER_IS_THREAD)
//socket server thread routine
static TOsclThreadFuncRet OSCL_THREAD_DECL sockthreadmain(TOsclThreadFuncArg arg);
#endif

int32 OsclSocketServI::StartServImp()
{
#if(PV_SOCKET_SERVER_IS_THREAD)

    //setup the loopback socket and/or polling interval.
#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
    iLoopbackSocket.iEnable = false;
#endif
    iSelectPollIntervalMsec = 0;

    //check the select timeout in the configuration.
    int32 selectTimeoutMsec = PV_SOCKET_SERVER_SELECT_TIMEOUT_MSEC;
    if (selectTimeoutMsec <= 0)
    {
        //non-polling option selected.
        //create the select cancel pipe.
#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
        iLoopbackSocket.Init(this);

        //if loopback socket isn't available, we must poll.
        if (!iLoopbackSocket.iEnable)
#endif
        {
            iSelectPollIntervalMsec = 10;
        }
    }
    else
    {
        //polling option selected.
        iSelectPollIntervalMsec = selectTimeoutMsec;
#if(PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET)
        //create the loopback socket.
        iLoopbackSocket.Init(this);
#endif
    }

    //Start the server thread.
    OsclThread thread;
    OsclProcStatus::eOsclProcError err = thread.Create((TOsclThreadFuncPtr)sockthreadmain,
                                         1024,
                                         (TOsclThreadFuncArg)this);
    if (err != OsclErrNone)
        return OsclErrGeneral;

    thread.SetPriority(PV_SOCKET_SERVER_THREAD_PRIORITY);

    //wait til thread starts
    iStart.Wait();

    return OsclErrNone;

#else//PV_SOCKET_SERVER_IS_THREAD

    //Socket server AO startup.

#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
    iLoopbackSocket.iEnable = false;
#endif

    ServerEntry();

    if (!IsAdded())
    {
#if PV_SOCKET_SERVER_SELECT
        iNhandles = 0;
        iNfds = 0;
#endif
        AddToScheduler();
    }
    return OsclErrNone;

#endif //PV_SOCKET_SERVER_IS_THREAD
}

void OsclSocketServI::StopServImp()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    //stop the thread.
    iClose = true;
    iSockServRequestList.Wakeup();//wake up the thread if needed.
    WakeupBlockingSelect();
    //wait til thread exits
    iExit.Wait();

#else//PV_SOCKET_SERVER_IS_THREAD

    //Socket server AO cleanup.
    if (iServState != OsclSocketServI::ESocketServ_Connected)
    {
        return;
    }

    //cancel any active request.
    if (IsAdded())
    {
        Cancel();
    }

    ServerExit();

    //remove AO from scheduler.
    if (IsAdded())
    {
        RemoveFromScheduler();
    }
#endif//PV_SOCKET_SERVER_IS_THREAD
}


#if(PV_SOCKET_SERVER_IS_THREAD)

void OsclSocketServI::InThread()
//Socket server thread implementation.
{
    OsclThread::GetId(iThreadId);

    iClose = false;

    ServerEntry();

    //Let server know thread is started and ready to
    //process requests.
    iStart.Signal();

    //create select timeout structure
    timeval timeout;

    bool ok;
    int nfds;
    int nhandles = 0;

    while (!iClose)
    {
        //process new requests and new socket activity on existing requests.
        ProcessSocketRequests(nhandles, nfds);

        //Make the select call if needed.
        if (nfds > 1)
        {
            //Set the fixed timeout.  The select call may update this value
            //so it needs to be set on each call.
            timeout.tv_sec = 0;

            if (iSelectPollIntervalMsec == 0)
            {
                //wait forever
                timeout.tv_usec = 0x1fffffff;
            }
            else
            {
                //poll
                timeout.tv_usec = iSelectPollIntervalMsec * 1000;
            }

            LOGSERV((0, "OsclSocketServI::InThread Calling select, timeout %d", iSelectPollIntervalMsec));
            OsclSocketSelect(nfds, iReadset, iWriteset, iExceptset, timeout, ok, iServError, nhandles);
            LOGSERV((0, "OsclSocketServI::InThread Select call returned"));
            if (!ok)
            {
                //select error.
                iServState = OsclSocketServI::ESocketServ_Error;
                break;
            }
            if (nhandles)
            {
                ADD_STATS(EOsclSocketServ_SelectActivity);
            }
            else
            {
                ADD_STATS(EOsclSocketServ_SelectNoActivity);
            }
        }
        else
        {
            //wait on new requests from the app side.
            LOGSERV((0, "OsclSocketServI::InThread Waiting on requests"));
            iSockServRequestList.WaitOnRequests();
            LOGSERV((0, "OsclSocketServI::InThread Done Waiting on requests"));
        }

    }//select loop

    ServerExit();

    //signal close complete to caller...
    if (iClose)
    {
        iClose = false;
        iExit.Signal();
    }
}

static TOsclThreadFuncRet OSCL_THREAD_DECL sockthreadmain2(TOsclThreadFuncArg arg);
static TOsclThreadFuncRet OSCL_THREAD_DECL sockthreadmain(TOsclThreadFuncArg arg)
//socket server thread.
{
    OsclBase::Init();
    OsclErrorTrap::Init();

    //once error trap is initialized, run everything else under a trap
    int32 err;
    OSCL_TRY(err,

             OsclMem::Init();
             PVLogger::Init();

             sockthreadmain2(arg);

             PVLogger::Cleanup();
             OsclMem::Cleanup();
            );

    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    return 0;
}


#include "pvlogger.h"

#if(PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT)
#include "pvlogger_time_and_id_layout.h"
#include "pvlogger_file_appender.h"
#include "pvlogger_stderr_appender.h"

#define LOGFILENAME _STRLIT_WCHAR("pvosclsocket_serv.log");

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};
#endif//PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT


static TOsclThreadFuncRet OSCL_THREAD_DECL sockthreadmain2(TOsclThreadFuncArg arg)
{
    OSCL_ASSERT(arg);
    OsclSocketServI *serv = (OsclSocketServI*)arg;

#if(PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT)
    //create logger appenders.
    PVLoggerAppender* appender;

//File logger
    //find an unused filename so we don't over-write any prior logs
    Oscl_FileServer fs;
    Oscl_File file;
    fs.Connect();
    OSCL_wHeapString<OsclMemAllocator> filename;
    oscl_wchar fileid[2];
    fileid[1] = (oscl_wchar)'\0';
    for (char c = 'A';c <= 'Z';c++)
    {
        filename = LOGFILENAME;
        fileid[0] = (oscl_wchar)c;
        filename += fileid;
        filename += _STRLIT_WCHAR(".txt");
        if (file.Open((oscl_wchar*)filename.get_cstr(), Oscl_File::MODE_READ, fs) != 0)
        {
            break;//found a nonexistent file.
        }
        file.Close();
    }
    fs.Close();
    //create appender using the selected filename.
    appender = TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender((OSCL_TCHAR*)filename.get_cstr());
    OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
        new OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(appender);
    //Set logging options.
    OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, appenderRefCounter);
    PVLogger *rootnode = PVLogger::GetLoggerObject("");
    rootnode->AddAppender(appenderPtr);
    rootnode->SetLogLevel(PVLOGMSG_DEBUG);
#endif //PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT

    serv->InThread();

    return 0;
}


#else//PV_SOCKET_SERVER_IS_THREAD
//Non-threaded section

void OsclSocketServI::Run()
//Socket server AO
{
#if !PV_SOCKET_SERVER_SELECT
    //non-select-loop implementation.

    //Process active requests.
    START_SERVI_STATS(EServiRun_Proc);
    ProcessSocketRequests();
    END_SERVI_STATS(EServiRun_Proc);

    //Re-schedule
    START_SERVI_STATS(EServiRun_Reschedule);
    if (!iSockServRequestList.iActiveRequests.empty())
    {
        // Re-schedule after a delay for continued monitoring
        // of active requests.
        // Note: new requests will interrupt this polling interval
        // and schedule ASAP.
        ADD_STATS(EOsclSocketServ_SelectReschedulePoll);
        RunIfNotReady(1000*PV_SOCKET_SERVER_AO_INTERVAL_MSEC);
    }
    END_SERVI_STATS(EServiRun_Reschedule);

#else
    //select loop implementation.

    //loop 2x so we can complete some requests in one call.
    for (uint32 i = 0;i < 2;i++)
    {
        //Process active requests.
        START_SERVI_STATS(EServiRun_Proc);
        ProcessSocketRequests(iNhandles, iNfds);
        END_SERVI_STATS(EServiRun_Proc);

        //Make the select call if needed.
        if (iNfds > 1)
        {
            START_SERVI_STATS(EServiRun_Select);
            //use a delay of zero since we're essentially polling for socket activity.
            //note the select call may update this value so it must be set prior to each call.
            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            bool ok;
            OsclSocketSelect(iNfds, iReadset, iWriteset, iExceptset, timeout, ok, iServError, iNhandles);
            END_SERVI_STATS(EServiRun_Select);
            if (!ok)
            {
                //a select error is fatal.
                StopServImp();//stop the AO
                iServState = OsclSocketServI::ESocketServ_Error;
                return;
            }

            if (iNhandles)
            {
                ADD_STATS(EOsclSocketServ_SelectActivity);
            }
            else
            {
                ADD_STATS(EOsclSocketServ_SelectNoActivity);
            }
        }
    }

    //Re-schedule
    START_SERVI_STATS(EServiRun_Reschedule);
    if (!iSockServRequestList.iActiveRequests.empty())
    {
        if (iNhandles)
        {
            // Re-schedule ASAP when we have socket activity.
            ADD_STATS(EOsclSocketServ_SelectRescheduleAsap);
            RunIfNotReady();
        }
        else
        {
            // Re-schedule after a delay for continued monitoring
            // of active requests.
            // Note: new requests will interrupt this polling interval
            // and schedule ASAP.
            ADD_STATS(EOsclSocketServ_SelectReschedulePoll);
            RunIfNotReady(1000*PV_SOCKET_SERVER_AO_INTERVAL_MSEC);
        }
    }
    END_SERVI_STATS(EServiRun_Reschedule);

#endif //PV_SOCKET_SERVER_SELECT
}

//re-schedule the AO when a new request comes in.
void OsclSocketServI::WakeupAO()
{
    if (IsAdded())
    {
        // To avoid waiting for the polling period to expire,
        // cancel and activate for immediate run
        if (IsBusy())
            Cancel();

        RunIfNotReady();
    }
}


#endif//PV_SOCKET_SERVER_IS_THREAD
#endif //PV_SOCKET_SERVER



