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


#include "oscl_scheduler.h"


#include "oscl_error.h"
#include "oscl_tickcount.h"
#include "pvlogger.h"
#include "oscl_error_trapcleanup.h"
#include "pvlogger.h"
#include "oscl_tls.h"
#include "oscl_int64_utils.h"

#define OSCL_DISABLE_WARNING_CONDITIONAL_IS_CONSTANT
#include "osclconfig_compiler_warnings.h"


#include "oscl_scheduler_tuneables.h"


/////////////////////////////////////
// Logger Macros
/////////////////////////////////////

//LOGERROR is for scheduler errors.
//This logging also goes to stderr on platforms with ANSI stdio.
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

//LOGNOTICE is for scheduler start/stop, install/uninstall notices.
#define LOGNOTICE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_NOTICE,m);

#if(PV_SCHED_ENABLE_PERF_LOGGING)
//LOGSTATS is for logging the AO summary statistics.  These are loggged either
//when the AO is deleted, or when the scheduling ends.
#define LOGSTATS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iLogger,PVLOGMSG_INFO,m);
//LOGPERF is for detailed performance logging.
#define LOGPERF_LEVEL PVLOGMSG_INFO
#define LOGPERF(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iLogger,LOGPERF_LEVEL,m);
//LOGPERF2 is for highest detail on scheduler activity.
#define LOGPERF2_LEVEL PVLOGMSG_INFO+1
#define LOGPERF2(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iLogger,LOGPERF2_LEVEL,m);
//max perf log string length
#define LOGPERFMAXSTR 64
#define RESET_LOG_PERF(m)\
	ResetLogPerf();\
	LOGPERF(m);
#else
#define LOGSTATS(m)
#define LOGPERF(m)
#define LOGPERF2(m)
#define LOGPERFMAXSTR 64
#define RESET_LOG_PERF(m)
#endif//(PV_SCHED_ENABLE_PERF_LOGGING)

/////////////////////////////////////
// DLL Entry
/////////////////////////////////////
#ifndef OSCL_COMBINED_DLL
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()
#endif

/////////////////////////////////////
// OsclScheduler
/////////////////////////////////////

OSCL_EXPORT_REF void OsclScheduler::Init(const char *name, Oscl_DefAlloc *alloc, int nreserve)
//Init the scheduler for this thread.
{
    int32 err;
    OSCL_TRY(err,
             OsclExecScheduler *sched = OsclExecScheduler::NewL(name, alloc, nreserve);
             sched->InstallScheduler(););
    if (err != OsclErrNone)
        OsclError::Leave(OsclErrNotInstalled);
}



OSCL_EXPORT_REF void OsclScheduler::Cleanup()
//Cleanup the scheduler for this thread.
{
    OsclExecSchedulerCommonBase *sched = OsclExecSchedulerCommonBase::GetScheduler();
    if (!sched)
        OsclError::Leave(OsclErrNotInstalled);
    sched->UninstallScheduler();
    Oscl_DefAlloc *alloc = sched->iAlloc;
    sched->~OsclExecSchedulerCommonBase();
    alloc->deallocate(sched);
}

/////////////////////////////////////
// OsclExecSchedulerCommonBase
/////////////////////////////////////

/////////////////////////////////////
// OsclExecScheduler
/////////////////////////////////////

//use the TLS registry, or the singleton registry if no TLS.
//Note: singleton registry only works for single-threaded
//scenarios, since this implementation assumes a per-thread registry.
#include "oscl_error.h"
#define PVSCHEDULER_REGISTRY OsclTLSRegistryEx
#define PVSCHEDULER_REGISTRY_ID OSCL_TLS_ID_PVSCHEDULER


/////////////////////////////////////
//For AO statistics.
/////////////////////////////////////

#if !(PV_SCHED_ENABLE_AO_STATS)
//no stats
#define PVTICK uint32
#define PVTICK_INT uint32
#define INIT_TICK(tick)
#define SET_TICK(tick)
#define GET_TICKFREQ(tick)
#define TICK_INT(tick1)
#define TICKSTR ""
#define TICK_INT_STR ""
#define TICK_INT_EXPR(x)
#else
//else use the oscl timer.
#define PVTICK uint32
#define PVTICK_INT uint32
#define INIT_TICK(tick) tick=0
#define SET_TICK(tick) tick=OsclTickCount::TickCount()
#define GET_TICKFREQ(tick) tick=OsclTickCount::TickCountFrequency()
#define TICK_INT(tick1) tick1
#define TICKSTR "Ticks"
#define TICK_INT_STR "%u"
#define TICK_INT_EXPR(x) x
#endif

#if !(PV_SCHED_ENABLE_AO_STATS)
#define DIFF_TICK(tick1,diff)
#define UPDATE_RUNERROR_TIME(x,y)
#define UPDATE_RUNL_TIME(x,y)
#define UPDATE_LEAVE_CODE(x,err)
#else
#define DIFF_TICK(tick1,diff) PVTICK _now;SET_TICK(_now);diff=TICK_INT(_now)-TICK_INT(tick1)
#define UPDATE_RUNERROR_TIME(stats,delta)\
		if(stats->i64Valid) stats->i64TotalTicksInRun+=delta;\
		else stats->iTotalTicksInRun+=delta;\
		if(delta>stats->iMaxTicksInRun) stats->iMaxTicksInRun=delta;\
		stats->iNumRunError++
#define UPDATE_RUNL_TIME(stats,delta)\
		if(stats->i64Valid) stats->i64TotalTicksInRun+=delta;\
		else stats->iTotalTicksInRun+=delta;\
		if(delta>stats->iMaxTicksInRun) stats->iMaxTicksInRun=delta;\
		stats->iNumRun++;
#define UPDATE_LEAVE_CODE(stats,err)if (err!=OsclErrNone)stats->iLeave=err
#endif

#if (PV_SCHED_ENABLE_LOOP_STATS)
#define DECLARE_LOOP_STATS int64 loopdelta = 0; PVTICK looptime;

#define START_LOOP_STATS(stats)SET_TICK(looptime);

#define START_WAIT_LOOP_STATS(rc,stats)\
		if (rc<1)\
			SET_TICK(looptime);

#define END_LOOP_STATS(stats)\
		{\
			DIFF_TICK(looptime,loopdelta);\
			if(stats->i64Valid) stats->i64TotalTicksInRun+=loopdelta;\
			else stats->iTotalTicksInRun+=Oscl_Int64_Utils::get_int64_lower32(loopdelta);\
			stats->iNumRun++;\
			LOGPERF((0,"PVSCHED: Run %d %s AO %s",(int32)loopdelta,TICKSTR,stats->iAOName.get_cstr()));\
		}

#define END_WAIT_LOOP_STATS(rc,stats)\
		if (rc<1)\
		{\
			DIFF_TICK(looptime,loopdelta);\
			if(stats->i64Valid) stats->i64TotalTicksInRun+=loopdelta;\
			else stats->iTotalTicksInRun+=Oscl_Int64_Utils::get_int64_lower32(loopdelta);\
			stats->iNumRun++;\
			LOGPERF((0,"PVSCHED: Run %d %s AO %s",(int32)loopdelta,TICKSTR,stats->iAOName.get_cstr()));\
		}

#else
#define DECLARE_LOOP_STATS
#define START_LOOP_STATS(stats)
#define START_WAIT_LOOP_STATS(rc,stats)
#define END_LOOP_STATS(stats)
#define END_WAIT_LOOP_STATS(rc,stats)

#endif

#if(PV_SCHED_ENABLE_PERF_LOGGING)
void OsclExecSchedulerCommonBase::ResetLogPerf()
{
    //print total time spend in prior interval of continuous Run calls.
    if (iLogPerfTotal > 0)
    {
        LOGPERF((0, "PVSCHED: Prior Interval %d %s", iLogPerfTotal, TICKSTR));
    }
    //reset interval.
    iLogPerfTotal = 0;
    //reset indentation to zero.
    iLogPerfIndentStrLen = 0;
    if (iLogPerfIndentStr)
        iLogPerfIndentStr[iLogPerfIndentStrLen] = '\0';
}

void OsclExecSchedulerCommonBase::IncLogPerf(uint32 delta)
{
    //add to total interval time.
    iLogPerfTotal += delta;
    //add a space to the indent string up to the max.
    if (iLogPerfIndentStr
            && iLogPerfIndentStrLen < LOGPERFMAXSTR)
    {
        iLogPerfIndentStr[iLogPerfIndentStrLen++] = ' ';
        iLogPerfIndentStr[iLogPerfIndentStrLen] = '\0';
    }
}
#endif//(PV_SCHED_ENABLE_PERF_LOGGING)

OsclExecSchedulerCommonBase* OsclExecSchedulerCommonBase::GetScheduler()
//static function to get currently installed scheduler
//for this thread.
{
    OsclExecSchedulerCommonBase *current = (OsclExecSchedulerCommonBase*)PVSCHEDULER_REGISTRY::getInstance(PVSCHEDULER_REGISTRY_ID);
    return current;
}

OsclExecSchedulerCommonBase* OsclExecSchedulerCommonBase::SetScheduler(OsclExecSchedulerCommonBase *a)
//static function to set currently installed scheduler
//for this thread. return previous scheduler, if any.
{
    OsclExecSchedulerCommonBase* temp = GetScheduler();
    PVSCHEDULER_REGISTRY::registerInstance(a, PVSCHEDULER_REGISTRY_ID);
    return temp;
}

OSCL_EXPORT_REF OsclNameString<PVSCHEDNAMELEN> *OsclExecSchedulerCommonBase::GetName()
//static function to get scheduler name for this thread.
{
    OsclExecSchedulerCommonBase *sched = GetScheduler();
    if (sched)
        return &sched->iName;
    else
        return NULL;
}

OSCL_EXPORT_REF uint32 OsclExecSchedulerCommonBase::GetId()
{
    return PVThreadContext::Id();
}

OsclExecScheduler * OsclExecScheduler::NewL(const char *name, Oscl_DefAlloc *alloc, int nreserve)
{
    OsclExecScheduler *self;
    OsclMemAllocator defalloc;
    OsclAny* ptr = (alloc) ? alloc->ALLOCATE(sizeof(OsclExecScheduler))
                   : defalloc.ALLOCATE(sizeof(OsclExecScheduler));
    OsclError::LeaveIfNull(ptr);
    self = OSCL_PLACEMENT_NEW(ptr, OsclExecScheduler(alloc));
    OsclError::PushL(self);
    self->ConstructL(name, nreserve);
    OsclError::Pop();
    return self;
}

OsclExecSchedulerCommonBase::~OsclExecSchedulerCommonBase()
{
    //make sure scheduler is not currently installed in
    //any thread.
    if (IsInstalled())
        OsclError::Leave(OsclErrInvalidState);//scheduler not stopped

    if (iStopper)
    {
        iStopper->~PVSchedulerStopper();
        iAlloc->deallocate(iStopper);
    }
#if(PV_SCHED_ENABLE_PERF_LOGGING)
    if (iLogPerfIndentStr)
        _oscl_free(iLogPerfIndentStr);
#endif
}

OsclExecScheduler::~OsclExecScheduler()
{
}

OsclExecSchedulerCommonBase::OsclExecSchedulerCommonBase(Oscl_DefAlloc *alloc)
{
    iAlloc = (alloc) ? alloc : &iDefAlloc;
#if(PV_SCHED_ENABLE_PERF_LOGGING)
    iLogPerfIndentStr = NULL;
    iLogPerfTotal = 0;
#endif
}

OsclExecScheduler::OsclExecScheduler(Oscl_DefAlloc *alloc)
        : OsclExecSchedulerCommonBase(alloc)
{
}

void OsclExecSchedulerCommonBase::ConstructL(const char *name, int nreserve)
{
    iNumAOAdded = 1;

    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(PVSchedulerStopper));
    OsclError::LeaveIfNull(ptr);
    iStopper = new(ptr) PVSchedulerStopper;

#if(PV_SCHED_ENABLE_AO_STATS)
    ConstructStatQ();
#endif

    InitExecQ(nreserve);

    iBlockingMode = false;
    iNativeMode = false;
    iName.Set(name);
    iLogger = PVLogger::GetLoggerObject("pvscheduler");

#if (PV_SCHED_ENABLE_PERF_LOGGING)
    iLogPerfIndentStr = (char*)_oscl_malloc(LOGPERFMAXSTR + 1);
    OsclError::LeaveIfNull(iLogPerfIndentStr);
    ResetLogPerf();
#endif
}

void OsclExecScheduler::ConstructL(const char *name, int nreserve)
{
    OsclExecSchedulerCommonBase::ConstructL(name, nreserve);
}

void OsclExecSchedulerCommonBase::InstallScheduler()
{
    //make sure this scheduler is not installed in
    //any thread.
    if (IsInstalled())
        OsclError::Leave(OsclErrAlreadyInstalled);
    //make sure no scheduler is installed in this thread.
    if (GetScheduler())
        OsclError::Leave(OsclErrAlreadyInstalled);

    SetScheduler(this);

    iThreadContext.EnterThreadContext();

    iErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
    if (!iErrorTrapImp)
        OsclError::Leave(OsclErrNotInstalled);//error trap not installed.

    if (iStopperCrit.Create() != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);//mutex error

    iResumeSem.Create();
    iDoStop = iDoSuspend = iSuspended = false;

    iReadyQ.ThreadLogon();

    LOGNOTICE((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Installed", iName.Str(), PVThreadContext::Id()));

#if (PV_SCHED_ENABLE_PERF_LOGGING)
    //print tick frequencies that will show up in the perf log.
    PVTICK f;
    GET_TICKFREQ(f);
    PVTICK_INT tickint = TICK_INT(f);
    LOGPERF((0, "PVSCHED: %s frequency %s", TICKSTR, TICK_INT_STR, TICK_INT_EXPR(tickint)));
    OSCL_UNUSED_ARG(tickint);
#endif
}

void OsclExecSchedulerCommonBase::UninstallScheduler()
{
    //make sure this scheduler is currently installed in
    //this thread.
    if (!IsInstalled() || GetScheduler() != this)
        OsclError::Leave(OsclErrNotInstalled);


    if (iBlockingMode)
    {
        //in case a thread error happened, go ahead and end
        //scheduling.
        OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
        if (trap
                && trap->iLeave != OsclErrNone)
            EndScheduling();
        //make sure scheduler is stopped.  If not, leave instead.
        if (IsStarted())
            OsclError::Leave(OsclErrInvalidState);//scheduler not stopped
    }
    else if (IsStarted())
    {
        //end non-blocking scheduling
        EndScheduling();
    }

    SetScheduler(NULL);

    iThreadContext.ExitThreadContext();

    CleanupExecQ();

    //Cleanup the stat queue.
#if(PV_SCHED_ENABLE_AO_STATS)
    CleanupStatQ();
#endif

    if (iStopperCrit.Close() != OsclProcStatus::SUCCESS_ERROR)
        OsclError::Leave(OsclErrSystemCallFailed);//mutex error

    iReadyQ.ThreadLogoff();
    iResumeSem.Close();

    LOGNOTICE((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Uninstalled", iName.Str(), PVThreadContext::Id()));
}

OSCL_EXPORT_REF OsclExecScheduler* OsclExecScheduler::Current()
//static routine to get current scheduler.
{
    return (OsclExecScheduler*)GetScheduler();
}

bool OsclExecSchedulerCommonBase::IsStarted()
{
    iStopperCrit.Lock();
    bool val = (iStopper->IsAdded()) ? true : false;
    iStopperCrit.Unlock();
    return val;
}

inline bool OsclExecSchedulerCommonBase::IsInstalled()
{
    return iThreadContext.iOpen;
}

void OsclExecSchedulerCommonBase::BeginScheduling(bool blocking, bool native)
//called before entering scheduling loop.
{
    //make sure scheduler is installed...
    if (!IsInstalled() || GetScheduler() != this)
        OsclError::Leave(OsclErrNotInstalled);

    //make sure scheduler is idle...
    if (IsStarted())
        OsclError::Leave(OsclErrInvalidState);

    iBlockingMode = blocking;
    iNativeMode = native;

    //Add stopper AO to scheduler.
    iStopperCrit.Lock();
    {
        iStopper->AddToScheduler();
        iStopper->PendForExec();
    }
    iStopperCrit.Unlock();

#if(PV_SCHED_ENABLE_PERF_LOGGING)
    ResetLogPerf();
#endif
#if(PV_SCHED_ENABLE_AO_STATS)
    BeginStats();
#endif
}


void OsclExecSchedulerCommonBase::EndScheduling()
//called after exiting scheduling loop.
{
    //see if it's already stopped..
    if (!IsStarted())
        return;

    //remove stopper AO.
    iStopperCrit.Lock();
    iStopper->RemoveFromScheduler();
    iStopperCrit.Unlock();

#if(PV_SCHED_ENABLE_AO_STATS)
    EndStats();
#endif
}

#if(PV_SCHED_ENABLE_AO_STATS)
void OsclExecSchedulerCommonBase::ConstructStatQ()
{
    //create a placeholder for summary stats for
    //all AOs that are not PVActiveBase.
    for (uint32 i = 0;i < EOtherExecStats_Last;i++)
        iOtherExecStats[i] = NULL;

    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(PVActiveStats));
    OsclError::LeaveIfNull(ptr);
    iOtherExecStats[EOtherExecStats_NativeOS] = OSCL_PLACEMENT_NEW(ptr, PVActiveStats(this, "Sched_TotalNativeOS", NULL));
    //init the stat queue offset.
    {
        int offset = (int) & (iOtherExecStats[EOtherExecStats_NativeOS])->iPVStatQLink - (int)(iOtherExecStats[EOtherExecStats_NativeOS]);
        iPVStatQ.SetOffset(offset);
    }

#if(PV_SCHED_ENABLE_LOOP_STATS)
    //create nodes for summary stats for scheduler loop time

    ptr = iAlloc->ALLOCATE(sizeof(PVActiveStats));
    OsclError::LeaveIfNull(ptr);
    iOtherExecStats[EOtherExecStats_QueueTime] = OSCL_PLACEMENT_NEW(ptr, PVActiveStats(this, "Sched_QueueTime", NULL));

    ptr = iAlloc->ALLOCATE(sizeof(PVActiveStats));
    OsclError::LeaveIfNull(ptr);
    iOtherExecStats[EOtherExecStats_WaitTime] = OSCL_PLACEMENT_NEW(ptr, PVActiveStats(this, "Sched_WaitTime", NULL));

#endif
    //add the non-AO stats nodes to the stat Q.
    {
        for (uint32 i = 0;i < EOtherExecStats_Last;i++)
        {
            if (iOtherExecStats[i])
                iPVStatQ.InsertTail(*iOtherExecStats[i]);
        }
    }
}

void OsclExecSchedulerCommonBase::CleanupStatQ()
{
    while (!iPVStatQ.IsEmpty())
    {
        PVActiveStats* first = iPVStatQ.Head();
        first->iPVStatQLink.Remove();
        first->~PVActiveStats();
        first->iScheduler->iAlloc->deallocate(first);
    }
}

void OsclExecSchedulerCommonBase::ShowStats(PVActiveStats *active)
//static routine to print stats for a PV AO.
{
    if (!active)
        return;

    //don't print any AOs that never ran.
    if ((active->iNumRun + active->iNumCancel) == 0)
        return;

    PVLogger* iLogger = PVLogger::GetLoggerObject("pvscheduler");
    if (active->i64Valid)
    {
        int64 avgTicksPerRun = (active->iNumRun == 0) ? 0 : active->i64TotalTicksInRun / (int64)active->iNumRun;

        LOGSTATS((0, "PVSCHED:Scheduler '%s', AO '%s': Pri %d NumRun %d, MaxTicksInRun 0x%x, AvgTicksPerRun (hi,lo) (0x%x,0x%08x) Units %s, NumCancel %d, NumError %d, LeaveCode %d"
                  , active->iScheduler->iName.Str()
                  , active->iAOName.get_cstr()
                  , active->iPriority
                  , active->iNumRun
                  , active->iMaxTicksInRun
                  , Oscl_Int64_Utils::get_int64_upper32(avgTicksPerRun)
                  , Oscl_Int64_Utils::get_int64_lower32(avgTicksPerRun)
                  , TICKSTR
                  , active->iNumCancel
                  , active->iNumRunError
                  , active->iLeave
                 ));
    }
    else
    {
        uint32 avgTicksPerRun = (active->iNumRun == 0) ? 0 : active->iTotalTicksInRun / active->iNumRun;
        LOGSTATS((0, "PVSCHED:Scheduler '%s', AO '%s': Pri %d NumRun %d, MaxTicksInRun %d, AvgTicksPerRun %d Units %s, NumCancel %d, NumError %d, LeaveCode %d"
                  , active->iScheduler->iName.Str()
                  , active->iAOName.get_cstr()
                  , active->iPriority
                  , active->iNumRun
                  , active->iMaxTicksInRun
                  , avgTicksPerRun
                  , TICKSTR
                  , active->iNumCancel
                  , active->iNumRunError
                  , active->iLeave
                 ));
    }
}

void OsclExecSchedulerCommonBase::ShowSummaryStats(PVActiveStats *active, PVLogger*logger, int64 total, int64& aGrandTotal, float& aTotalPercent)
//static routine to print stats for a PV AO.
{
    if (total == (int64)0)
        return;//to avoid divide by zero

    if (!active)
        return;

    //don't print any AO's that never ran.
    if ((active->iNumRun + active->iNumCancel) == 0)
        return;

    //calculate percent of the total time that was spent in this AO.
    if (active->i64Valid)
    {
        active->iPercent = 100.0 * active->i64TotalTicksInRun / total;
        aGrandTotal += active->i64TotalTicksInRun;
    }
    else
    {
        active->iPercent = 100.0 * active->iTotalTicksInRun / total;
        aGrandTotal += active->iTotalTicksInRun;
    }
    aTotalPercent += active->iPercent;

    int32 fraction = (int32)active->iPercent;
    float decimal = active->iPercent - fraction;
    decimal *= 100.0;

    //print results
    if (active->i64Valid)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO
                        , (0, "  TIME PERCENT %d.%02d, AO '%s', Pri %d NumRun %d, MaxTicksInRun 0x%x, TotalTicksInRun Hi,Lo (0x%x,0x%08x), NumCancel %d, NumError %d, LeaveCode %d, NumInstance %d"
                           , (int32)active->iPercent, (int32)decimal
                           , active->iAOName.get_cstr()
                           , active->iPriority
                           , active->iNumRun
                           , active->iMaxTicksInRun
                           , Oscl_Int64_Utils::get_int64_upper32(active->i64TotalTicksInRun)
                           , Oscl_Int64_Utils::get_int64_lower32(active->i64TotalTicksInRun)
                           , active->iNumCancel
                           , active->iNumRunError
                           , active->iLeave
                           , active->iNumInstances
                          ));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO
                        , (0, "  TIME PERCENT %d.%02d, AO '%s', Pri %d NumRun %d, MaxTicksInRun 0x%x, TotalTicksInRun 0x%x, NumCancel %d, NumError %d, LeaveCode %d, NumInstance %d"
                           , (int32)active->iPercent, (int32)decimal
                           , active->iAOName.get_cstr()
                           , active->iPriority
                           , active->iNumRun
                           , active->iMaxTicksInRun
                           , (int32)active->iTotalTicksInRun
                           , active->iNumCancel
                           , active->iNumRunError
                           , active->iLeave
                           , active->iNumInstances
                          ));
    }
}

void OsclExecSchedulerCommonBase::BeginStats()
//Begin stats for all AOs.
{
    iTotalTicksTemp = (uint8*)OSCL_MALLOC(sizeof(PVTICK));
    SET_TICK(*((PVTICK*)iTotalTicksTemp));
}

void OsclExecSchedulerCommonBase::EndStats()
//End stats for all AOs.
{
    //get the end time for the scheduler run.
    int64 total;
    DIFF_TICK((*((PVTICK*)iTotalTicksTemp)), total);
    OSCL_FREE(iTotalTicksTemp);

    //there may be multiple entries per AO in the stats table, so combine them now.
    if (!iPVStatQ.IsEmpty())
    {
        OsclDoubleRunner<PVActiveStats> iter(iPVStatQ);
        PVActiveStats *item;
        for (iter.SetToHead(); ;iter++)
        {
            item = iter;
            //find all subsequent entries in the list that have
            //the same AO name as this entry.
            if (item->iNumInstances > 0
                    && !iPVStatQ.IsTail(item))
            {
                OsclDoubleRunner<PVActiveStats> iter2(iPVStatQ);
                PVActiveStats* item2;
                for (iter2 = iter, iter2++; ;iter2++)
                {
                    item2 = iter2;
                    if (item2->iAOName == item->iAOName)
                    {
                        item->Combine(*item2);
                        //mark this entry to ignore in further processing.
                        item2->iNumInstances = 0;
                    }
                    if (iPVStatQ.IsTail(item2))
                        break;
                }
            }
            if (iPVStatQ.IsTail(item))
                break;
        }
    }
    //end of multiple-instance combine.

    QUE_ITER_BEGIN(PVActiveStats, iPVStatQ)
    {
        if (item
                && item->iNumInstances > 0)
            OsclExecScheduler::ShowStats(item);
    }
    QUE_ITER_END(iPVStatQ)

    //Show summary stats

    PVLogger* logger = PVLogger::GetLoggerObject("OsclSchedulerPerfStats");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "OSCL SCHEDULER SUMMARY STATISTICS FOR SCHEDULER '%s'", iName.Str())
                   )

    iGrandTotalTicks = 0;
    iTotalPercent = 0.0;
    QUE_ITER_BEGIN(PVActiveStats, iPVStatQ)
    {
        if (item
                && item->iNumInstances > 0)
            OsclExecScheduler::ShowSummaryStats(item, logger, total, iGrandTotalTicks, iTotalPercent);
    }
    QUE_ITER_END(iPVStatQ)

    //split total percent into whole & decimal parts.
    int32 fraction = (int32)iTotalPercent;
    float decimal = iTotalPercent - fraction;
    decimal *= 100.0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "  Total Time (hi,lo): (0x%x,0x%08x) Units: %s", Oscl_Int64_Utils::get_int64_upper32(total), Oscl_Int64_Utils::get_int64_lower32(total), TICKSTR)
                   )
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "  Total Time Accounted (hi,lo): (0x%x,0x%08x) Units: %s", Oscl_Int64_Utils::get_int64_upper32(iGrandTotalTicks), Oscl_Int64_Utils::get_int64_lower32(iGrandTotalTicks), TICKSTR)
                   )
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "  Total Percent Accounted: %d.%02d", (int32)iTotalPercent, (int32)decimal)
                   )

    PVTICK f;
    GET_TICKFREQ(f);
    PVTICK_INT tickint = TICK_INT(f);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "  Tick Units: %s Frequency: %s", TICKSTR, TICK_INT_STR, TICK_INT_EXPR(tickint))
                   )

    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, logger, PVLOGMSG_INFO,
                    (0, "END OSCL SCHEDULER SUMMARY STATISTICS FOR SCHEDULER '%s'", iName.Str())
                   )

}
#endif //PV_SCHED_ENABLE_AO_STATS


void OsclExecSchedulerCommonBase::Error(int32 anError) const
//call this when any AO leaves and its error handler does not handle the error.
{
    LOGERROR((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Error! Reason %d", iName.Str(), PVThreadContext::Id(), anError));
    fprintf(stderr, "PVSCHED:Scheduler '%s', Thread 0x%x: Error! Reason %d\n", iName.Str(), PVThreadContext::Id(), anError);

    //propagate the leave
    OsclError::Leave(anError);
}

OSCL_EXPORT_REF void OsclExecSchedulerCommonBase::StartScheduler(OsclSemaphore *aSignal)
//blocking call to start PV scheduler.
//Will leave if any AO leaves.
{

    BeginScheduling(true, false);//blocking, non-native
    if (aSignal)
        aSignal->Signal();


    LOGNOTICE((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Starting PV Scheduling Loop", iName.Str(), PVThreadContext::Id()));

    int32 err;
    OSCL_TRY(err, BlockingLoopL(););

    LOGNOTICE((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Exited PV Scheduling Loop", iName.Str(), PVThreadContext::Id()));

    EndScheduling();

    if (err)
        OsclError::Leave(err);

}

OSCL_EXPORT_REF void OsclExecSchedulerCommonBase::StartNativeScheduler()
//blocking call to start native scheduler.
//Will leave if any AO leaves.
{
    OsclError::Leave(OsclErrNotSupported);//native scheduler not supported.
}

//scheduler stopper request status codes.
#define STOPPER_REQUEST_STOP_NATIVE 0
#define STOPPER_REQUEST_STOP_PV 1
#define STOPPER_REQUEST_SUSPEND 2

OSCL_EXPORT_REF void OsclExecSchedulerCommonBase::StopScheduler()
//any thread can use this to stop the blocking scheduler.
{
    if (!IsInstalled())
        OsclError::Leave(OsclErrNotInstalled);

    if (!iBlockingMode)
        OsclError::Leave(OsclErrNotReady);
    if (!IsStarted())
        return ;

    if (iStopper->iStatus != OSCL_REQUEST_PENDING)
        OsclError::Leave(OsclErrNotReady);

    //in case scheduler is in the suspend loop...
    if (iDoSuspend || iSuspended)
        iResumeSem.Signal();

    if (iNativeMode)
        iStopper->PendComplete(STOPPER_REQUEST_STOP_NATIVE);
    else
        iStopper->PendComplete(STOPPER_REQUEST_STOP_PV);

}

OSCL_EXPORT_REF void OsclExecSchedulerCommonBase::SuspendScheduler()
//any thread can use this to suspend the blocking scheduler.
{
    if (!IsInstalled())
        OsclError::Leave(OsclErrNotInstalled);

    if (iNativeMode)
        OsclError::Leave(OsclErrNotSupported);

    if (!iBlockingMode)
        OsclError::Leave(OsclErrNotSupported);

    if (!IsStarted())
        return;

    if (iStopper->iStatus != OSCL_REQUEST_PENDING)
        OsclError::Leave(OsclErrNotReady);

    iStopper->PendComplete(STOPPER_REQUEST_SUSPEND);

}

OSCL_EXPORT_REF void OsclExecSchedulerCommonBase::ResumeScheduler()
//any thread can use this to resume the blocking scheduler.
{
    if (!IsInstalled())
        OsclError::Leave(OsclErrNotInstalled);

    if (iDoSuspend || iSuspended)
    {
        iResumeSem.Signal();
        return ;
    }
    else
        OsclError::Leave(OsclErrNotReady); //not suspended.
}

OSCL_EXPORT_REF void OsclExecScheduler::RunSchedulerNonBlocking(int32 aCount, int32 &aReady, uint32 &aShortestDelay)
//run scheduler in non-blocking mode.
//Will leave if any AO leaves.
{


    aReady = 0;
    aShortestDelay = 0;

    //make sure this scheduler is installed.
    if (!IsInstalled())
        OsclError::Leave(OsclErrNotInstalled);

#if !(OSCL_RELEASE_BUILD)  && !defined(NDEBUG)
    //make sure this scheduler is really installed in this
    //thread.
    if (GetScheduler() != this)
        OsclError::Leave(OsclErrNotInstalled);
#endif

    //start scheduling if needed.
    if (!IsStarted())
    {
        BeginScheduling(false, false);//nonblocking, non-native
    }
    else if (iBlockingMode || iNativeMode)
        OsclError::Leave(OsclErrInvalidState);//not stopped

    //Process timers.  All ready timers will get
    //moved to the ready queue.
    UpdateTimersMsec(aShortestDelay);

    //Run until the requested count is reached, or there
    //aren't any AOs ready.
    for (int32 count = 0;count < aCount;)
    {
        //find highest pri ready AO.
        PVActiveBase* pvactive = iReadyQ.PopTop();
        if (pvactive)
        {
            //run it
            count++;
            CallRunExec(pvactive);

            //re-evaluate timers
            UpdateTimersMsec(aShortestDelay);
        }
        else
            break;//nothing ready
    }

    //at this point, either nothing else is ready or the target count was reached.

    aReady = iReadyQ.Depth();

}

OSCL_EXPORT_REF void OsclExecScheduler::RegisterForCallback(OsclSchedulerObserver* aCallback, OsclAny* aCallbackContext)
{
    //Update the callback pointers.
    iReadyQ.RegisterForCallback(aCallback, aCallbackContext);
}

////////////////////////////////////////
// Queue Management
////////////////////////////////////////


PVActiveBase * OsclExecSchedulerCommonBase::FindPVBase(PVActiveBase *active, OsclDoubleList<PVActiveBase> &q)
//Search a PVActiveBase queue, given a PVActiveBase ptr.
{
    QUE_ITER_BEGIN(PVActiveBase, q)
    {
        if (item == active)
            return item;
    }
    QUE_ITER_END(q)
    return NULL;
}

void OsclExecSchedulerCommonBase::CleanupExecQ()
{
    //Cleanup timers.
    {
        PVActiveBase *top;
        while ((top = iExecTimerQ.PopTop()))
            top->RemoveFromScheduler();
    }
    //Cleanup ready AOs.
    PVActiveBase* top = iReadyQ.Top();
    while (top)
    {
        top->RemoveFromScheduler();
        top = iReadyQ.Top();
    }
}

void OsclExecSchedulerCommonBase::InitExecQ(int nreserve)
//init the pvactive queues.
{
    iExecTimerQ.Construct(nreserve);
    iReadyQ.Construct(nreserve);
}

////////////////////////////////////////
// Non-Symbian queue management
////////////////////////////////////////

void OsclExecSchedulerCommonBase::AddToExecTimerQ(PVActiveBase* anActive, uint32 aDelayMicrosec)
//timer implementation.
//Add an AO to the pending timer queue.
{
    OSCL_ASSERT(anActive);//EExecNull

    //make sure this AO is not already added.
    if (anActive->IsInAnyQ())
        OsclError::Leave(OsclErrInvalidState);//EExecAlreadyAdded

    //Set time in ticks when AO should run.

    uint32 tickperiod = OsclTickCount::TickCountPeriod();
    OSCL_ASSERT(tickperiod != 0);

    //Round to the nearest integer with the computation:
    //floor((2*Interval_usec/ticks_per_usec +  1)/2)
    //
    //The computed time may rollover the 32-bit value-- that's OK, because
    //the tick count will also rollover.
    uint32 timenow = OsclTickCount::TickCount();
    anActive->iPVReadyQLink.iTimeToRunTicks = timenow + (aDelayMicrosec * 2 / tickperiod + 1) / 2;

    if (aDelayMicrosec > 0)
    {
        LOGPERF2((0, "PVSCHED:%s AO %s Timer delay %d TimeToRunTicks %d Timenow %d"
                  , iLogPerfIndentStr, anActive->iName.Str()
                  , aDelayMicrosec, anActive->iPVReadyQLink.iTimeToRunTicks
                  , timenow));
    }

    //queue it
    iExecTimerQ.Add(anActive);

    //if this AO is in the front of the queue now, we need to do a
    //callback, because the shortest delay interval has changed.
    if (iReadyQ.Callback()
            && anActive == iExecTimerQ.Top())
    {
        iReadyQ.TimerCallback(aDelayMicrosec);
    }
}

//For 32-bit time comparisons with rollover handling.
//This value is (2^31)-1
const uint32 OsclExecSchedulerCommonBase::iTimeCompareThreshold = 0x7fffffff;

PVActiveBase* OsclExecSchedulerCommonBase::UpdateTimers(uint32 &aShortestDelay)
//timer processing.
//Complete requests for all timers that are ready now,
//then return the pending timer with the shortest delay if any.
//If any pending timer is returned it's the top of the queue so
//it can be discarded later with Pop.
{
    aShortestDelay = 0;

    PVActiveBase *top = iExecTimerQ.Top();

    if (!top)
        return NULL;

    uint32 timenow = OsclTickCount::TickCount();

    //Find all timers that are ready, and the first
    //timer that isn't.  The list is sorted by
    //time then priority.
    for (;top;top = iExecTimerQ.Top())
    {
        //calculate time to run <= timenow, taking possible rollover into account
        uint32 deltaTicks = timenow - top->iPVReadyQLink.iTimeToRunTicks;
        if (deltaTicks <= iTimeCompareThreshold)
        {
            //this timer is ready
            iExecTimerQ.Pop(top);

            PendComplete(top, OSCL_REQUEST_ERR_NONE, EPVThreadContext_InThread);
        }
        else
        {
            //we found the pending timer with the shortest delay.
            //get the delay value
            int32 delayTicks = deltaTicks;
            if (delayTicks < 0)
                delayTicks = (-delayTicks);
            aShortestDelay = delayTicks;
            return top;
        }
    }

    return NULL;//no pending timers.
}

PVActiveBase* OsclExecSchedulerCommonBase::UpdateTimersMsec(uint32 &aShortestDelay)
//Identical to UpdateTimers except the delay returned is milliseconds instead
//of ticks.
{
    aShortestDelay = 0;

    PVActiveBase *top = iExecTimerQ.Top();

    if (!top)
        return NULL;

    uint32 timenow = OsclTickCount::TickCount();

    //Find all timers that are ready, and the first
    //timer that isn't.  The list is sorted by
    //time then priority.
    for (;top;top = iExecTimerQ.Top())
    {
        //calculate time to run <= timenow, taking possible rollover into account
        uint32 deltaTicks = timenow - top->iPVReadyQLink.iTimeToRunTicks;
        if (deltaTicks <= iTimeCompareThreshold)
        {
            //this timer is ready
            iExecTimerQ.Pop(top);

            PendComplete(top, OSCL_REQUEST_ERR_NONE, EPVThreadContext_InThread);
        }
        else
        {
            //we found the pending timer with the shortest delay.
            //get the delay value
            int32 delayTicks = deltaTicks;
            if (delayTicks < 0)
                delayTicks = (-delayTicks);
            aShortestDelay = OsclTickCount::TicksToMsec(delayTicks);

            //if delay became zero after the conversion from ticks to msec,
            //then just consider this timer to be ready now.
            if (aShortestDelay == 0)
            {
                //this timer is ready
                iExecTimerQ.Pop(top);

                PendComplete(top, OSCL_REQUEST_ERR_NONE, EPVThreadContext_InThread);
            }
            else
            {
                return top;
            }
        }
    }

    return NULL;//no pending timers.
}

void OsclExecSchedulerCommonBase::PendComplete(PVActiveBase *pvbase, int32 aReason, TPVThreadContext aThreadContext)
//complete a request for this scheduler.
//Calling context can be any thread.
{
    //During timer cancellation, the AO may still be in the ExecTimerQ.
    //Remove it now, since it won't get removed by the scheduler loop.
    //Check thread context first, to accessing timer queue from out-of-thread.
    if (aThreadContext == EPVThreadContext_InThread)
    {
        LOGPERF2((0, "PVSCHED: %s AO %s Request complete", iLogPerfIndentStr, pvbase->iName.Str()));

        if (iExecTimerQ.IsIn(pvbase))
            iExecTimerQ.Remove(pvbase);
    }

    //Pass this to the ReadyQ so it can do appropriate queue locks
    int32 err = iReadyQ.PendComplete(pvbase, aReason);

    OsclError::LeaveIfError(err);
}

void OsclExecSchedulerCommonBase::RequestCanceled(PVActiveBase* pvbase)
{
    LOGPERF2((0, "PVSCHED: %s AO %s Request canceled", iLogPerfIndentStr, pvbase->iName.Str()));

    //This gets called right after the AO's DoCancel was
    //called.  It must wait on the request to cancel, and remove it
    //from the ready Q.

    //Calling context is always in-thread, since Cancel context is always
    //in-thread.

    //See if the request was completed by the DoCancel.
    bool complete = iReadyQ.IsIn(pvbase);

    DECLARE_LOOP_STATS;

    if (!complete)
    {
        //If request is still pending after DoCancel is called, it
        //means some other thread will complete the request cancellation.
        //If the AO does not have a proper DoCancel, this wait will hang up.

        //reset the perf indent when scheduler gives up CPU...
        RESET_LOG_PERF((0, "PVSCHED: Waiting on cancel... AO '%s'", pvbase->iName.Str()));

#if (PV_SCHED_ENABLE_PERF_LOGGING)
        uint32 ticks = OsclTickCount::TickCount();
#endif
        START_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);
        int32 err = iReadyQ.WaitForRequestComplete(pvbase);
        END_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);

#if (PV_SCHED_ENABLE_PERF_LOGGING)
        LOGPERF((0, "PVSCHED: ...Cancel took %d Ticks", OsclTickCount::TickCount() - ticks));
#endif

        OsclError::LeaveIfError(err);
    }

    //Set request idle and remove from ready Q.
    //The AO will not run
    pvbase->iBusy = false;

    START_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);
    iReadyQ.Remove(pvbase);
    END_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);
}




////////////////////////////////////////
// PV Scheduling Loop Implementation
////////////////////////////////////////


void OsclExecSchedulerCommonBase::CallRunExec(PVActiveBase *pvactive)
//Run a PV AO.
{
    //Set this AO inactive.  This AO may be from the
    //ready Q or the pending Timer Q.
    //The dequeing is done by the caller.
    pvactive->iBusy = false;

    //start stats
#if (PV_SCHED_ENABLE_AO_STATS)
    iPVStats = pvactive->iPVActiveStats;//save value now since it may change in the Run call.
    iDelta = 0;
    INIT_TICK(iTime);
#endif

    SET_TICK(iTime);

    //Call the Run under a trap harness.
    //Pass the ErrorTrapImp pointer to reduce overhead of the Try call.
    //We already did a null ptr check on iErrorTrapImp so it's safe to de-ref here.
    int32 err;
    OSCL_TRY_NO_TLS(iErrorTrapImp, err, pvactive->Run(););

    //end stats
    DIFF_TICK(iTime, iDelta);
    UPDATE_RUNL_TIME(iPVStats, iDelta);
    UPDATE_LEAVE_CODE(iPVStats, err);

#if(PV_SCHED_ENABLE_PERF_LOGGING)
    //show AO time.
    IncLogPerf(iDelta);
    LOGPERF((0, "PVSCHED: %s Run %d %s AO %s", iLogPerfIndentStr, (int32)iDelta, TICKSTR, pvactive->iName.Str()));
#endif

    //check for a leave in the Run...
    if (err != OsclErrNone)
    {
        //start stats
        SET_TICK(iTime);

        //call the AO error handler
        err = pvactive->RunError(err);

        //end stats
        DIFF_TICK(iTime, iDelta);
        UPDATE_RUNERROR_TIME(iPVStats, iDelta);

        //If the AO did not handle the error, indicated by returning
        //ErrNone, then call the scheduler error handler.
        if (err != OsclErrNone)
        {
            LOGERROR((0, "PVSCHED:Scheduler '%s', Thread 0x%x: Error! AO %s Error %d not handled"
                      , iName.Str(), PVThreadContext::Id()
                      , pvactive->iName.Str(), err));
            fprintf(stderr, "PVSCHED:Scheduler '%s', Thread 0x%x: Error! AO %s Error %d not handled\n"
                    , iName.Str(), PVThreadContext::Id()
                    , pvactive->iName.Str(), err);

            Error(err);
        }
    }

}


void OsclExecSchedulerCommonBase::BlockingLoopL()
//Blocking scheduling loop.
//Will leave if any AO leaves.
{

    PVActiveBase* pvactive;


    while (!iDoStop)
    {
        //Get the next AO to run.  This call may block until an AO is ready.
        pvactive = WaitForReadyAO();

        if (pvactive)
        {
            CallRunExec(pvactive);
        }
        else
        {
            OsclError::Leave(OsclErrCorrupt);//EExecStrayEvent
        }

        //check for a suspend signal..
        if (iDoSuspend)
        {
            iSuspended = true;
            iDoSuspend = false;
            iResumeSem.Wait();
            iSuspended = false;
        }

    }//while !dostop

    iDoStop = false;
}


PVActiveBase* OsclExecSchedulerCommonBase::WaitForReadyAO()
//Find the next AO to run-- non-Symbian version.
{
    DECLARE_LOOP_STATS;

    //First process timers.
    //All ready timers will get moved to the run Q.
    uint32 waitTicks;
    START_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);
    PVActiveBase* pvtimer = UpdateTimers(waitTicks);
    END_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);

    //Check for a ready AO.
    START_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);
    PVActiveBase* pvactive = iReadyQ.PopTop();
    END_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);

    if (pvactive)
    {
        //An AO is ready.
        return pvactive;
    }
    else if (pvtimer)
    {
        //No AO is ready, but at least one timer is pending.
        //Wait on shortest timer expiration or a new request.

        //reset the perf logging indent each time scheduler gives up CPU.
        RESET_LOG_PERF((0, "PVSCHED: Waiting on timer... Ticks %d AO %s", waitTicks, pvtimer->iName.Str()));

        START_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);
        pvactive = iReadyQ.WaitAndPopTop(OsclTickCount::TicksToMsec(waitTicks));
        END_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);

        if (pvactive)
        {
            //Another AO's request completed while we were waiting.
            //Run that one instead of the timer.
            return pvactive;
        }
        else
        {
            //It's time to complete this timer's request and run it.
            //To save overhead, don't move it to the ReadyQ, just pop it
            //from its current location on the top of the timer queue.

            pvtimer->iStatus = OSCL_REQUEST_ERR_NONE;

            START_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);
            iExecTimerQ.Pop(pvtimer);
            END_LOOP_STATS(iOtherExecStats[EOtherExecStats_QueueTime]);

            return pvtimer;
        }
    }
    else
    {
        //Nothing is ready and no timer is pending.
        //Wait on a request to be completed by another thread.

        //reset the perf logging indent each time scheduler gives up CPU.
        RESET_LOG_PERF((0, "PVSCHED: Waiting on any request..."));

        START_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);
        pvactive = iReadyQ.WaitAndPopTop();
        END_LOOP_STATS(iOtherExecStats[EOtherExecStats_WaitTime]);

        return pvactive;
    }
}

////////////////////////////////////////
// PVSchedulerStopper
////////////////////////////////////////
PVSchedulerStopper::PVSchedulerStopper()
        : OsclActiveObject((int32)OsclActiveObject::EPriorityHighest, "Stopper")
{
}

PVSchedulerStopper::~PVSchedulerStopper()
{
}

void PVSchedulerStopper::Run()
//This AO just waits for a signal to suspend or stop the scheduler.
{
    //Stop
    switch (Status())
    {
        case STOPPER_REQUEST_STOP_NATIVE:
            break;
        case STOPPER_REQUEST_STOP_PV:
        {
            //stop my scheduling loop
            OsclExecSchedulerCommonBase* myscheduler = OsclExecScheduler::GetScheduler();
            if (myscheduler)
                myscheduler->iDoStop = true;
        }
        break;
        case STOPPER_REQUEST_SUSPEND:
        {
            //suspend my scheduling loop
            OsclExecSchedulerCommonBase* myscheduler = OsclExecScheduler::GetScheduler();
            if (myscheduler)
                myscheduler->iDoSuspend = true;

            //re-schedule ourself
            PendForExec();
        }
        break;
        default:
            break;
    }
}

////////////////////////////////////////
// Symbian Coe Scheduler
////////////////////////////////////////




