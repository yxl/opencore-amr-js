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
#ifndef PVMF_JB_EVENT_NOTIFIER_H
#include "pvmf_jb_event_notifier.h"
#endif


OSCL_EXPORT_REF PVMFJBEventNotifier* PVMFJBEventNotifier::New(PVMFMediaClock& aNonDecreasingClock, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aEstimatedServerClock)
{
    PVMFJBEventNotifier* ptr = NULL;
    int32 err = OsclErrNone;
    OSCL_TRY(err, ptr = OSCL_NEW(PVMFJBEventNotifier, (aNonDecreasingClock, aClientPlaybackClock, aEstimatedServerClock));
             ptr->Construct(););
    if (err != OsclErrNone && ptr)
    {
        OSCL_DELETE(ptr);
        ptr = NULL;
    }
    return ptr;
}

void PVMFJBEventNotifier::Construct()
{
    ipLogger = PVLogger::GetLoggerObject("PVMFJBEventNotifier");
    int32 err = OsclErrNone;
    OSCL_TRY(err,

             ipNonDecreasingClkNotificationInterfaceObserver = OSCL_NEW(NonDecClkNotificationInterfaceObserver, (iJBEvntNtfrRequestInfoVectNonDecClk));
             ipClientPlaybackClkNotificationInterfaceObserver = OSCL_NEW(ClientPlaybackClkNotificationInterfaceObserver, (iJBEvntNtfrRequestInfoVectClientPlaybackClk));
             ipEstimatedSrvrClkNotificationInterfaceObserver = OSCL_NEW(EstimatedSrvrClkNotificationInterfaceObserver, (iJBEvntNtfrRequestInfoVectEstimatedServClk));

             irNonDecreasingClock.ConstructMediaClockNotificationsInterface(ipNonDecreasingClockNotificationsInf, *ipNonDecreasingClkNotificationInterfaceObserver);
             irClientPlaybackClock.ConstructMediaClockNotificationsInterface(ipClientPlayBackClockNotificationsInf, *ipClientPlaybackClkNotificationInterfaceObserver);
             irEstimatedServerClock.ConstructMediaClockNotificationsInterface(ipEstimatedClockNotificationsInf, *ipEstimatedSrvrClkNotificationInterfaceObserver);

            );

    if (err != OsclErrNone || ((NULL == ipNonDecreasingClockNotificationsInf) || (NULL == ipClientPlayBackClockNotificationsInf) || (NULL == ipEstimatedClockNotificationsInf)))
    {
        CleanUp();
    }
}

OSCL_EXPORT_REF PVMFJBEventNotifier::~PVMFJBEventNotifier()
{
    CleanUp();
}

void PVMFJBEventNotifier::CleanUp()
{
    CancelAllPendingCallbacks();

    PVMF_JB_LOGEVENTNOTIFIER((0, "Est clock Vect Size %d", iJBEvntNtfrRequestInfoVectEstimatedServClk.size()));
    PVMF_JB_LOGEVENTNOTIFIER((0, "Client clock Vect Size %d", iJBEvntNtfrRequestInfoVectClientPlaybackClk.size()));
    PVMF_JB_LOGEVENTNOTIFIER((0, "Non dec clock Vect Size %d", iJBEvntNtfrRequestInfoVectNonDecClk.size()));

    if (ipEstimatedClockNotificationsInf)
    {
        irEstimatedServerClock.DestroyMediaClockNotificationsInterface(ipEstimatedClockNotificationsInf);
        ipEstimatedClockNotificationsInf = NULL;
    }

    if (ipClientPlayBackClockNotificationsInf)
    {
        irClientPlaybackClock.DestroyMediaClockNotificationsInterface(ipClientPlayBackClockNotificationsInf);
        ipClientPlayBackClockNotificationsInf = NULL;
    }

    if (ipNonDecreasingClockNotificationsInf)
    {
        irNonDecreasingClock.DestroyMediaClockNotificationsInterface(ipNonDecreasingClockNotificationsInf);
        ipNonDecreasingClockNotificationsInf = NULL;
    }

    if (ipNonDecreasingClkNotificationInterfaceObserver)
    {
        OSCL_DELETE(ipNonDecreasingClkNotificationInterfaceObserver);
    }

    if (ipClientPlaybackClkNotificationInterfaceObserver)
    {
        OSCL_DELETE(ipClientPlaybackClkNotificationInterfaceObserver);
    }

    if (ipEstimatedSrvrClkNotificationInterfaceObserver)
    {
        OSCL_DELETE(ipEstimatedSrvrClkNotificationInterfaceObserver);
    }
}

OSCL_EXPORT_REF bool PVMFJBEventNotifier::RequestCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aDelay, uint32& aCallBkId)
{
    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack In - Interface type[%d] Observer[0x%x] aDelay[%d]", aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver() , aDelay));
    bool retval = false;
    CLOCK_NOTIFICATION_INTF_TYPE interfaceType = aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType();
    PVMFMediaClockNotificationsInterface* eventNotificationInterface = NULL;
    PVMFMediaClockNotificationsObs* callbackObserver = NULL;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>* jbEvntNtfrRequestInfoVect = NULL;

    switch (interfaceType)
    {
        case CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING:
        {
            //Make sure clock is in running state
            if (irNonDecreasingClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }

            eventNotificationInterface = ipNonDecreasingClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectNonDecClk;
            callbackObserver = ipNonDecreasingClkNotificationInterfaceObserver;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK:
        {
            //Make sure clock is in running state
            if (irClientPlaybackClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }
            eventNotificationInterface  = ipClientPlayBackClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectClientPlaybackClk;
            callbackObserver = ipClientPlaybackClkNotificationInterfaceObserver;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER:
        {
            //Make sure clock is in running state
            if (irEstimatedServerClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }
            eventNotificationInterface = ipEstimatedClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectEstimatedServClk;
            callbackObserver = ipEstimatedSrvrClkNotificationInterfaceObserver;
        }
        break;
        default:
        {
            OSCL_ASSERT(false);
        }
    }

    if (eventNotificationInterface && callbackObserver && aDelay > 0)
    {
        const int32 toleranceWndForCallback = 0;
        PVMFStatus status = eventNotificationInterface->SetCallbackDeltaTime(aDelay, //delta time in clock when callBack should be called
                            toleranceWndForCallback,
                            callbackObserver, //observer object to be called on timeout
                            false, //no threadLock
                            aNotificationRequestInfo.GetContextData(), //no context
                            aCallBkId); //ID used to identify the timer for cancellation
        if (PVMFSuccess != status)
        {
            OSCL_ASSERT(false);
        }
        else
        {
            //Replicate aNotificationRequestInfo
            PVMFJBEventNotifierRequestInfo *notifierReqInfo = OSCL_NEW(PVMFJBEventNotifierRequestInfo, ());
            if (notifierReqInfo)
            {
                PVMFJBEventNotificationRequestInfo*	obsRequestInfo = OSCL_NEW(PVMFJBEventNotificationRequestInfo, (aNotificationRequestInfo));
                notifierReqInfo->ipRequestInfo = obsRequestInfo;
                notifierReqInfo->iCallBackId = aCallBkId;
                jbEvntNtfrRequestInfoVect->push_back(notifierReqInfo);
                retval = true;
            }
            PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack - Allocated at 0x%x", notifierReqInfo));
        }
    }
    else
    {
        OSCL_ASSERT(false);
    }

    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack Out - retval[%d] Interface type[%d] Observer[0x%x] aDelay[%d], aCallBkId[%d]", retval, aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver() , aDelay, aCallBkId));
    return retval;
}

OSCL_EXPORT_REF bool PVMFJBEventNotifier::RequestAbsoluteTimeCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aAbsoluteTime, uint32& aCallBkId)
{
    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestAbsoluteTimeCallBack In - Interface type[%d] Observer[0x%x] aAbsoluteTime[%d]", aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver() , aAbsoluteTime));
    bool retval = false;
    CLOCK_NOTIFICATION_INTF_TYPE interfaceType = aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType();
    PVMFMediaClockNotificationsInterface* eventNotificationInterface = NULL;
    PVMFMediaClockNotificationsObs* callbackObserver = NULL;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>* jbEvntNtfrRequestInfoVect = NULL;

    switch (interfaceType)
    {
        case CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING:
        {
            //Make sure clock is in running state
            if (irNonDecreasingClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }

            eventNotificationInterface = ipNonDecreasingClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectNonDecClk;
            callbackObserver = ipNonDecreasingClkNotificationInterfaceObserver;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK:
        {
            //Make sure clock is in running state
            if (irClientPlaybackClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }
            eventNotificationInterface  = ipClientPlayBackClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectClientPlaybackClk;
            callbackObserver = ipClientPlaybackClkNotificationInterfaceObserver;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER:
        {
            //Make sure clock is in running state
            if (irEstimatedServerClock.GetState() != PVMFMediaClock::RUNNING)
            {
                return false;
            }
            eventNotificationInterface = ipEstimatedClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectEstimatedServClk;
            callbackObserver = ipEstimatedSrvrClkNotificationInterfaceObserver;
        }
        break;
        default:
        {
            OSCL_ASSERT(false);
        }
    }

    if (eventNotificationInterface && callbackObserver && aAbsoluteTime > 0)
    {
        const int32 toleranceWndForCallback = 0;
        PVMFStatus status = eventNotificationInterface->SetCallbackAbsoluteTime(aAbsoluteTime,
                            toleranceWndForCallback,
                            callbackObserver, //observer object to be called on timeout
                            false, //no threadLock
                            aNotificationRequestInfo.GetContextData(), //no context
                            aCallBkId); //ID used to identify the timer for cancellation
        if (PVMFSuccess != status)
        {
            OSCL_ASSERT(false);
        }
        else
        {
            //Replicate aNotificationRequestInfo
            PVMFJBEventNotifierRequestInfo *notifierReqInfo = OSCL_NEW(PVMFJBEventNotifierRequestInfo, ());
            if (notifierReqInfo)
            {
                PVMFJBEventNotificationRequestInfo*	obsRequestInfo = OSCL_NEW(PVMFJBEventNotificationRequestInfo, (aNotificationRequestInfo));
                notifierReqInfo->ipRequestInfo = obsRequestInfo;
                notifierReqInfo->iCallBackId = aCallBkId;
                jbEvntNtfrRequestInfoVect->push_back(notifierReqInfo);
                retval = true;
            }
        }
    }
    else
    {
        OSCL_ASSERT(false);
    }

    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack Out - retval[%d] Interface type[%d] Observer[0x%x] aAbsoluteTime[%d], aCallBkId[%d]", retval, aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver() , aAbsoluteTime, aCallBkId));
    return retval;
}

OSCL_EXPORT_REF void PVMFJBEventNotifier::CancelCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aCallBkId)
{
    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::CancelCallBack In - Interface type[%d] Observer[0x%x] aCallBkId[%d]", aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver() , aCallBkId));
    CancelCallBack(aNotificationRequestInfo.GetMediaClockNotificationsInterfaceType(), aNotificationRequestInfo.GetObserver(), aCallBkId);
}

void PVMFJBEventNotifier::CancelCallBack(CLOCK_NOTIFICATION_INTF_TYPE aType, PVMFJBEventNotifierObserver* aObserver, uint32& aCallBkId)
{
    OSCL_UNUSED_ARG(aObserver);
    PVMFMediaClockNotificationsInterface* eventNotificationInterface = NULL;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>* jbEvntNtfrRequestInfoVect = NULL;

    switch (aType)
    {
        case CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING:
        {
            eventNotificationInterface = ipNonDecreasingClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectNonDecClk;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK:
        {
            eventNotificationInterface  = ipClientPlayBackClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectClientPlaybackClk;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER:
        {
            eventNotificationInterface = ipEstimatedClockNotificationsInf;
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectEstimatedServClk;
        }
        break;
        default:
        {
            OSCL_ASSERT(false);
        }
    }

    if (eventNotificationInterface && jbEvntNtfrRequestInfoVect)
    {
        eventNotificationInterface->CancelCallback(aCallBkId, false);
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>::iterator iter;
        if (jbEvntNtfrRequestInfoVect->size())
        {
            for (iter = jbEvntNtfrRequestInfoVect->end() - 1; iter >= jbEvntNtfrRequestInfoVect->begin(); iter--)
            {
                PVMFJBEventNotifierRequestInfo* eventNotifierReqinfo = *iter;
                if (aCallBkId == eventNotifierReqinfo->iCallBackId)
                {
                    OSCL_DELETE(eventNotifierReqinfo->ipRequestInfo);
                    OSCL_DELETE(eventNotifierReqinfo);
                    PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack - Dellocated at 0x%x", eventNotifierReqinfo));
                    jbEvntNtfrRequestInfoVect->erase(iter);
                    break;
                }
            }
        }
    }
}

OSCL_EXPORT_REF void PVMFJBEventNotifier::CancelAllPendingCallbacks(CLOCK_NOTIFICATION_INTF_TYPE aType, PVMFJBEventNotifierObserver* aObserver)
{
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>* jbEvntNtfrRequestInfoVect = NULL;
    switch (aType)
    {
        case CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING:
        {
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectNonDecClk;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK:
        {
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectClientPlaybackClk;
        }
        break;
        case CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER:
        {
            jbEvntNtfrRequestInfoVect = &iJBEvntNtfrRequestInfoVectEstimatedServClk;
        }
        break;
        default:
        {
            OSCL_ASSERT(false);
        }
    }

    if (jbEvntNtfrRequestInfoVect)
    {
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>::iterator iter;
        if (jbEvntNtfrRequestInfoVect->size())
        {
            for (iter = jbEvntNtfrRequestInfoVect->end() - 1; iter >= jbEvntNtfrRequestInfoVect->begin(); iter--)
            {
                PVMFJBEventNotifierRequestInfo* ptr = *iter;
                CancelCallBack(aType, aObserver, ptr->iCallBackId);
            }
        }
    }
}

OSCL_EXPORT_REF void PVMFJBEventNotifier::CancelAllPendingCallbacks()
{
    CancelAllPendingCallbacks(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING);
    CancelAllPendingCallbacks(CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK);
    CancelAllPendingCallbacks(CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER);
}

///////////////////////////////////////////////////////////////////////////////
//NonDecClkNotificationInterfaceObserver
///////////////////////////////////////////////////////////////////////////////
void NonDecClkNotificationInterfaceObserver::ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aTimerAccuracy);
    OSCL_UNUSED_ARG(aDelta);
    bool found = false;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>::iterator iter;
    PVMFJBEventNotifierRequestInfo* eventNotifierReqinfo = NULL;

    for (iter = irJBEvntNtfrRequestInfoVectNonDecClk.begin(); iter != irJBEvntNtfrRequestInfoVectNonDecClk.end(); iter++)
    {
        eventNotifierReqinfo = *iter;
        if (aCallBackID == eventNotifierReqinfo->iCallBackId)
        {
            found = true;
            break;
        }
    }
    if (found && eventNotifierReqinfo)
    {

        PVMFJBEventNotifierObserver* observer = eventNotifierReqinfo->ipRequestInfo->GetObserver();
        OSCL_DELETE(eventNotifierReqinfo->ipRequestInfo);
        OSCL_DELETE(eventNotifierReqinfo);
        PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack - Dellocated at 0x%x", eventNotifierReqinfo));
        irJBEvntNtfrRequestInfoVectNonDecClk.erase(iter);
        observer->ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, aCallBackID, aContextData, aStatus);
    }
    else
        OSCL_ASSERT(false);
}

void NonDecClkNotificationInterfaceObserver::NotificationsInterfaceDestroyed()
{

}

///////////////////////////////////////////////////////////////////////////////
//ClientPlaybackClkNotificationInterfaceObserver
///////////////////////////////////////////////////////////////////////////////
void ClientPlaybackClkNotificationInterfaceObserver::ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aTimerAccuracy);
    OSCL_UNUSED_ARG(aDelta);
    bool found = false;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>::iterator iter;
    PVMFJBEventNotifierRequestInfo* eventNotifierReqinfo = NULL;
    for (iter = irJBEvntNtfrRequestInfoVectClientPlaybackClk.begin(); iter != irJBEvntNtfrRequestInfoVectClientPlaybackClk.end(); iter++)
    {
        eventNotifierReqinfo = *iter;
        if (aCallBackID == eventNotifierReqinfo->iCallBackId)
        {
            found = true;
            break;
        }
    }
    if (found && eventNotifierReqinfo)
    {
        PVMFJBEventNotifierObserver* observer = eventNotifierReqinfo->ipRequestInfo->GetObserver();
        OSCL_DELETE(eventNotifierReqinfo->ipRequestInfo);
        OSCL_DELETE(eventNotifierReqinfo);
        PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack - Dellocated at 0x%x", eventNotifierReqinfo));
        irJBEvntNtfrRequestInfoVectClientPlaybackClk.erase(iter);
        observer->ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK, aCallBackID, aContextData, aStatus);
    }
    else
        OSCL_ASSERT(false);
}

void ClientPlaybackClkNotificationInterfaceObserver::NotificationsInterfaceDestroyed()
{

}

///////////////////////////////////////////////////////////////////////////////
//EstimatedSrvrClkNotificationInterfaceObserver
///////////////////////////////////////////////////////////////////////////////
void EstimatedSrvrClkNotificationInterfaceObserver::ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aTimerAccuracy);
    OSCL_UNUSED_ARG(aDelta);
    bool found = false;
    Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>::iterator iter;
    PVMFJBEventNotifierRequestInfo* eventNotifierReqinfo = NULL;
    for (iter = irJBEvntNtfrRequestInfoVectEstimatedServClk.begin(); iter != irJBEvntNtfrRequestInfoVectEstimatedServClk.end(); iter++)
    {
        eventNotifierReqinfo = *iter;
        if (aCallBackID == eventNotifierReqinfo->iCallBackId)
        {
            found = true;
            break;
        }
    }
    if (found && eventNotifierReqinfo)
    {
        PVMFJBEventNotifierObserver* observer = eventNotifierReqinfo->ipRequestInfo->GetObserver();
        OSCL_DELETE(eventNotifierReqinfo->ipRequestInfo);
        OSCL_DELETE(eventNotifierReqinfo);
        PVMF_JB_LOGEVENTNOTIFIER((0, "PVMFJBEventNotifier::RequestCallBack - Dellocated at 0x%x", eventNotifierReqinfo));
        irJBEvntNtfrRequestInfoVectEstimatedServClk.erase(iter);
        observer->ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER, aCallBackID, aContextData, aStatus);
    }
    else
        OSCL_ASSERT(false);
}

void EstimatedSrvrClkNotificationInterfaceObserver::NotificationsInterfaceDestroyed()
{

}
