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
#ifndef PVMI_MEDIA_IO_OBSERVER_H_INCLUDED
#define PVMI_MEDIA_IO_OBSERVER_H_INCLUDED

#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif

/**
 * Observer interface for asynchronous PvmiMIOControl requests. Clients using PvmiMIOControl interface
 * must implement this observer interface to receive callbacks for PvmiMIOControl requests.
 */
class PvmiMIOObserver
{
    public:
        virtual ~PvmiMIOObserver() {}

        /**
         * Signal of completion for asynchronous PvmiMIOControl requests.
         * @param aResponse Completion response containing status and other data
         */
        virtual void RequestCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual void ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL) = 0;
        virtual void ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL) = 0;
};

#endif // PVMI_MEDIA_IO_OBSERVER_H_INCLUDED
