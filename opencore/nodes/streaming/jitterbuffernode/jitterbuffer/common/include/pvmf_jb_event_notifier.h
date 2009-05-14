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
#define PVMF_JB_EVENT_NOTIFIER_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_INTERNAL_H
#include "pvmf_jitter_buffer_common_internal.h"
#endif

enum CLOCK_NOTIFICATION_INTF_TYPE
{
    CLOCK_NOTIFICATION_INTF_TYPE_UNKNOWN,
    CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING,
    CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK,
    CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER
};

class PVMFJBEventNotifierObserver
{
    public:
        virtual void ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus) = 0;
};
class PVMFJBEventNotificationRequestInfo;
class PVMFJBEventNotifierRequestInfo
{
    public:
        PVMFJBEventNotifierRequestInfo(): ipRequestInfo(NULL)
                , iCallBackId(0) {}
        PVMFJBEventNotificationRequestInfo*	ipRequestInfo;
        uint32	iCallBackId;
};

class NonDecClkNotificationInterfaceObserver: public PVMFMediaClockNotificationsObs
{
    public:
        NonDecClkNotificationInterfaceObserver(Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& aJBEvntNtfrRequestInfoVectNonDecClk)
                : irJBEvntNtfrRequestInfoVectNonDecClk(aJBEvntNtfrRequestInfoVectNonDecClk)
        {
            ipLogger = PVLogger::GetLoggerObject("PVMFJBEventNotifier");
        }
        void ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus);
        virtual void NotificationsInterfaceDestroyed();
    private:
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& irJBEvntNtfrRequestInfoVectNonDecClk;
        PVLogger* ipLogger;
};

class EstimatedSrvrClkNotificationInterfaceObserver: public PVMFMediaClockNotificationsObs
{
    public:
        EstimatedSrvrClkNotificationInterfaceObserver(Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& aJBEvntNtfrRequestInfoVectEstimatedServClk)
                : irJBEvntNtfrRequestInfoVectEstimatedServClk(aJBEvntNtfrRequestInfoVectEstimatedServClk)
        {
            ipLogger = PVLogger::GetLoggerObject("PVMFJBEventNotifier");
        }
        void ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus);
        virtual void NotificationsInterfaceDestroyed();
    private:
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& irJBEvntNtfrRequestInfoVectEstimatedServClk;
        PVLogger* ipLogger;
};

class ClientPlaybackClkNotificationInterfaceObserver: public PVMFMediaClockNotificationsObs
{
    public:
        ClientPlaybackClkNotificationInterfaceObserver(Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& aJBEvntNtfrRequestInfoVectClientPlaybackClk):
                irJBEvntNtfrRequestInfoVectClientPlaybackClk(aJBEvntNtfrRequestInfoVectClientPlaybackClk)
        {
            ipLogger = PVLogger::GetLoggerObject("PVMFJBEventNotifier");
        }
        void ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta, const OsclAny* aContextData, PVMFStatus aStatus);
        virtual void NotificationsInterfaceDestroyed();
    private:
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator>& irJBEvntNtfrRequestInfoVectClientPlaybackClk;
        PVLogger* ipLogger;
};

class PVMFJBEventNotificationRequestInfo
{
    public:
        PVMFJBEventNotificationRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationsIntfType, PVMFJBEventNotifierObserver* aObserver, OsclAny* aContext = NULL)
                : iClockNotificationIntfType(aClockNotificationsIntfType)
                , ipObserver(aObserver)
                , ipContext(aContext)
        {
        }

        PVMFJBEventNotificationRequestInfo(const PVMFJBEventNotificationRequestInfo& aJBNotificationReqinfo)
        {
            iClockNotificationIntfType = aJBNotificationReqinfo.iClockNotificationIntfType;
            ipObserver = aJBNotificationReqinfo.ipObserver;
            ipContext = aJBNotificationReqinfo.ipContext;
        }

        const OsclAny* GetContextData() const
        {
            return ipContext;
        }

        CLOCK_NOTIFICATION_INTF_TYPE GetMediaClockNotificationsInterfaceType() const
        {
            return  iClockNotificationIntfType;
        }

        PVMFJBEventNotifierObserver* GetObserver() const
        {
            return ipObserver;
        }

    private:
        CLOCK_NOTIFICATION_INTF_TYPE iClockNotificationIntfType;
        PVMFJBEventNotifierObserver* ipObserver;
        const OsclAny* ipContext;
};

class PVMFJBEventNotifier
{
    public:
        OSCL_IMPORT_REF static PVMFJBEventNotifier* New(PVMFMediaClock& aNonDecreasingClock, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aEstimatedServerClock);
        OSCL_IMPORT_REF bool RequestCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aDelay, uint32& aCallBkId);
        OSCL_IMPORT_REF bool RequestAbsoluteTimeCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aAbsoluteTime, uint32& aCallBkId);
        OSCL_IMPORT_REF void CancelCallBack(const PVMFJBEventNotificationRequestInfo& aNotificationRequestInfo, uint32 aCallBkId);
        OSCL_IMPORT_REF void CancelAllPendingCallbacks(CLOCK_NOTIFICATION_INTF_TYPE aType, PVMFJBEventNotifierObserver* aObserver = NULL);
        OSCL_IMPORT_REF void CancelAllPendingCallbacks();
        OSCL_IMPORT_REF ~PVMFJBEventNotifier();
    private:
        /**

        */
        PVMFJBEventNotifier(PVMFMediaClock& aNonDecreasingClock, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aEstimatedServerClock)
                : irNonDecreasingClock(aNonDecreasingClock)
                , irClientPlaybackClock(aClientPlaybackClock)
                , irEstimatedServerClock(aEstimatedServerClock)
                , ipNonDecreasingClockNotificationsInf(NULL)
                , ipClientPlayBackClockNotificationsInf(NULL)
                , ipEstimatedClockNotificationsInf(NULL)
                , ipNonDecreasingClkNotificationInterfaceObserver(NULL)
                , ipClientPlaybackClkNotificationInterfaceObserver(NULL)
                , ipEstimatedSrvrClkNotificationInterfaceObserver(NULL)
        {}

        //Can leave: Yes
        //Possible leave code (s): OsclErrNoMemory
        void Construct();
        void CleanUp();
        void CancelCallBack(CLOCK_NOTIFICATION_INTF_TYPE aType, PVMFJBEventNotifierObserver* aObserver, uint32& aCallBkId);

        PVMFMediaClock&	irNonDecreasingClock;
        PVMFMediaClock&	irClientPlaybackClock;
        PVMFMediaClock&	irEstimatedServerClock;

        //Notification interfaces
        PVMFMediaClockNotificationsInterface* ipNonDecreasingClockNotificationsInf;
        PVMFMediaClockNotificationsInterface* ipClientPlayBackClockNotificationsInf;
        PVMFMediaClockNotificationsInterface* ipEstimatedClockNotificationsInf;

        NonDecClkNotificationInterfaceObserver*	ipNonDecreasingClkNotificationInterfaceObserver;
        ClientPlaybackClkNotificationInterfaceObserver* ipClientPlaybackClkNotificationInterfaceObserver;
        EstimatedSrvrClkNotificationInterfaceObserver* ipEstimatedSrvrClkNotificationInterfaceObserver;

        //Todo: Need to consider data structure for efficeincy of removing elements in between (linked list?)
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator> iJBEvntNtfrRequestInfoVectEstimatedServClk;
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator> iJBEvntNtfrRequestInfoVectClientPlaybackClk;
        Oscl_Vector<PVMFJBEventNotifierRequestInfo*, OsclMemAllocator> iJBEvntNtfrRequestInfoVectNonDecClk;

        PVLogger* ipLogger;
};
#endif
