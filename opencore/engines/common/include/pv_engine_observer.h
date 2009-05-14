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

#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#define PV_ENGINE_OBSERVER_H_INCLUDED

#ifndef PV_ENGINE_OBSERVER_MESSAGE_H_INCLUDED
#include "pv_engine_observer_message.h"
#endif

/**
 * PVErrorEventObserver Class
 *
 * PVErrorEventObserver is the PV SDK event observer class. It is used
 * for communicating unsolicited error events back to the user of the SDK.
 *
 * Applications using the PV SDKs must have a class derived from
 * PVErrorEventObserver and implement the pure virtual function in
 * order to receive error notifications from a PV SDK.
 *
 **/
class PVErrorEventObserver
{
    public:
        /**
         * Handle an error event that has been generated.
         *
         * @param "aEvent" "The event to be handled."
         */
        virtual void HandleErrorEvent(const PVAsyncErrorEvent& aEvent) = 0;
        virtual ~PVErrorEventObserver() {}
};

/**
 * PVInformationalEventObserver Class
 *
 * PVInformationalEventObserver is the PV SDK event observer class. It is used
 * for communicating unsolicited informational events back to the user of the SDK.
 *
 * Applications using the PV SDKs must have a class derived from
 * PVInformationalEventObserver and implement the pure virtual function in
 * order to receive informational event notifications from a PV SDK.
 *
 **/
class PVInformationalEventObserver
{
    public:
        /**
         * Handle an informational event that has been generated.
         *
         * @param "aEvent" "The event to be handled."
         */
        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent) = 0;
        virtual ~PVInformationalEventObserver() {}
};


/**
 * PVCommandStatusObserver Class
 *
 * PVCommandStatusObserver is the PV SDK observer class for notifying the
 * status of issued command messages. The API provides a mechanism for
 * the status of each command to be passed back along with context specific
 * information where applicable.
 * Applications using the PV SDKs must have a class derived from
 * PVCommandStatusObserver and implement the pure virtual function in
 * order to receive event notifications from a PV SDK.  Additional
 * information is optionally provided via derived classes.
 **/
class PVCommandStatusObserver
{
    public:
        /**
        Handle an event that has been generated.

        @param "aResponse"	"The response to a previously issued command."
        */
        virtual void CommandCompleted(const PVCmdResponse& aResponse) = 0;
        virtual ~PVCommandStatusObserver() {}
};

#endif // PV_ENGINE_OBSERVER_H_INCLUDED



