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

#ifndef PV_ENGINE_OBSERVER_MESSAGE_H_INCLUDED
#define PV_ENGINE_OBSERVER_MESSAGE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif


// CLASS DECLARATION
/**
 * PVCmdResponse Class
 *
 * PVCmdResponse class is used to pass completion status on previously issued commands
 **/
class PVCmdResponse : public PVMFCmdResp
{
    public:
        /**
         * Constructor for PVCmdResponse
         */
        PVCmdResponse(PVCommandId aId,
                      OsclAny* aContext,
                      PVMFStatus aStatus,
                      // Event data will be deprecated
                      OsclAny* aEventData = NULL, int32 aEventDataSize = 0):
                PVMFCmdResp(aId, aContext, aStatus, NULL, aEventData),
                iEventDataSize(aEventDataSize)
        {
        }

        /**
         * Constructor with event extension interface
         */
        PVCmdResponse(PVCommandId aId,
                      OsclAny* aContext,
                      PVMFStatus aStatus,
                      PVInterface* aEventExtInterface = NULL,
                      // Event data will be deprecated
                      OsclAny* aEventData = NULL, int32 aEventDataSize = 0):
                PVMFCmdResp(aId, aContext, aStatus, aEventExtInterface, aEventData),
                iEventDataSize(aEventDataSize)
        {
        }

        /**
        * WILL BE DEPRECATED SINCE IT IS NOT BEING USED. CURRENTLY RETURNS 0
        * @return Returns the type of Response we get
        */
        PVResponseType GetResponseType()const
        {
            return 0;
        }

        /**
         * @return Returns the unique ID associated with a command of this type.
         */
        PVCommandId GetCmdId()const
        {
            return (PVCommandId)(PVMFCmdResp::GetCmdId());
        }

        /**
         * @return Returns the opaque data that was passed in with the command.
         */
        OsclAny* GetContext()const
        {
            return (OsclAny*)(PVMFCmdResp::GetContext());
        }

        /**
         * @return Returns the completion status of the command
         */
        PVMFStatus GetCmdStatus()const
        {
            return PVMFCmdResp::GetCmdStatus();
        }

        /**
         * WILL BE DEPRECATED WHEN PVMFCmdResp REMOVES EVENT DATA
         * @return Returns additional data asociated with the command.  This is to be interpreted
                   based on the command issued and the return status
         */
        OsclAny* GetResponseData()const
        {
            return PVMFCmdResp::GetEventData();
        }

        int32 GetResponseDataSize()const
        {
            return iEventDataSize;
        }

    private:
        int32 iEventDataSize;
};


/**
 * PVAsyncInformationalEvent Class
 *
 * PVAsyncInformationalEvent is used to pass unsolicited informational indications to the
 * user. Additional information can be tagged based on the specific event
 **/
class PVAsyncInformationalEvent : public PVMFAsyncEvent
{
    public:
        /**
         * Constructor for PVAsyncInformationalEvent
         */
        PVAsyncInformationalEvent(PVEventType aEventType,
                                  PVExclusivePtr aEventData = NULL,
                                  uint8* aLocalBuffer = NULL,
                                  int32 aLocalBufferSize = 0):
                PVMFAsyncEvent(PVMFInfoEvent, aEventType, NULL, NULL, aEventData, aLocalBuffer, aLocalBufferSize)
        {
        }

        /**
         * Constructor with context and event extension interface
         */
        PVAsyncInformationalEvent(PVEventType aEventType,
                                  OsclAny* aContext,
                                  PVInterface* aEventExtInterface,
                                  PVExclusivePtr aEventData = NULL,
                                  uint8* aLocalBuffer = NULL,
                                  int32 aLocalBufferSize = 0):
                PVMFAsyncEvent(PVMFInfoEvent, aEventType, aContext, aEventExtInterface, aEventData, aLocalBuffer, aLocalBufferSize)
        {
        }

        /**
         * Destructor
         */
        ~PVAsyncInformationalEvent() {}

        /**
         * WILL BE DEPRECATED SINCE IT IS NOT BEING USED. CURRENTLY RETURNING 0.
         * @return Returns the type of Response we get
         */
        PVResponseType GetResponseType()const
        {
            return 0;
        }

        /**
         * @return Returns the Event type that has been received
         */
        PVEventType GetEventType()const
        {
            return PVMFAsyncEvent::GetEventType();
        }

        /**
        * @return Returns the opaque data asociated with the event.
        */
        void GetEventData(PVExclusivePtr& aPtr)const
        {
            aPtr = PVMFAsyncEvent::GetEventData();
        }
};

/**
 * PVAsyncErrorEvent Class
 *
 * PVAsyncErrorEvent is used to pass unsolicited error indications to the
 * user. Additional information can be tagged based on the specific event
 **/
class PVAsyncErrorEvent : public PVMFAsyncEvent
{
    public:
        /**
         * Constructor for PVAsyncErrorEvent
         */
        PVAsyncErrorEvent(PVEventType aEventType,
                          PVExclusivePtr aEventData = NULL,
                          uint8* aLocalBuffer = NULL,
                          int32 aLocalBufferSize = 0):
                PVMFAsyncEvent(PVMFErrorEvent, aEventType, NULL, NULL, aEventData, aLocalBuffer, aLocalBufferSize)
        {
        }

        /**
         * Constructor with context and event extension interface
         */
        PVAsyncErrorEvent(PVEventType aEventType,
                          OsclAny* aContext,
                          PVInterface* aEventExtInterface,
                          PVExclusivePtr aEventData = NULL,
                          uint8* aLocalBuffer = NULL,
                          int32 aLocalBufferSize = 0):
                PVMFAsyncEvent(PVMFErrorEvent, aEventType, aContext, aEventExtInterface, aEventData, aLocalBuffer, aLocalBufferSize)
        {
        }

        /**
         * Destructor
         */
        ~PVAsyncErrorEvent() {}

        /**
         * WILL BE DEPRECATED SINCE IT IS NOT BEING USED. CURRENTLY RETURNING 0.
         * @return Returns the type of Response we get
         */
        PVResponseType GetResponseType()const
        {
            return 0;
        }

        /**
         * @return Returns the Event type that has been received
         */
        PVEventType GetEventType()const
        {
            return PVMFAsyncEvent::GetEventType();
        }

        /**
        * @return Returns the opaque data asociated with the event.
        */
        void GetEventData(PVExclusivePtr& aPtr)const
        {
            aPtr = PVMFAsyncEvent::GetEventData();
        }
};


#endif // PV_ENGINE_OBSERVER_MESSAGE_H_INCLUDED
