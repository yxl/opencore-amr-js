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
/**
* @file pvmf_jitter_buffer_port.h
*/
#ifndef PVMF_JITTER_BUFFER_PORT_H_INCLUDED
#define PVMF_JITTER_BUFFER_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif
#ifndef PVMI_PORT_CONFIG_KVP_H_INCLUDED
#include "pvmi_port_config_kvp.h"
#endif

#define PVMF_JB_PORT_OVERRIDE 1

class PVMFJitterBuffer;

//Default vector reserve size
#define PVMF_JITTER_BUFFER_NODE_PORT_VECTOR_RESERVE 10

// Capability mime strings
#define PVMF_JITTER_BUFFER_PORT_SPECIFIC_ALLOCATOR			"x-pvmf/pvmfstreaming/socketmemallocator"
#define PVMF_JITTER_BUFFER_PORT_SPECIFIC_ALLOCATOR_VALTYPE  "x-pvmf/pvmfstreaming/socketmemallocator;valtype=ksv"

/** Enumerated list of port tags supported by this port */
typedef enum
{
    PVMF_JITTER_BUFFER_PORT_TYPE_UNKNOWN = -1,
    PVMF_JITTER_BUFFER_PORT_TYPE_INPUT = 0,
    PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT = 1,
    //Feedback Port: Only used in case of RTSP based streaming to send feedback
    //reports (RTCP reports) for the RTP session going on at the input port
    PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK = 2
} PVMFJitterBufferNodePortTag;

class PVMFJitterBufferNode;
class PVMFJitterBufferPort;
class PVMFJitterBufferPortParams
{
    public:
        void ResetParams()
        {
            iJitterBufferEmpty = false;
            iProcessIncomingMessages = true;
            iProcessOutgoingMessages = true;
            iMonitorForRemoteActivity = true;
        };

        PVMFJitterBufferPortParams(PVMFJitterBufferPort& aPort): irPort(aPort)
        {
            iId = -1;
            iTag = PVMF_JITTER_BUFFER_PORT_TYPE_UNKNOWN;

            ipJitterBuffer = NULL;				//Only Input ports will have the jitter buffer associated with them
            iTimeScale = 0;
            iBitrate = 0;
            iLastMsgTimeStamp = 0;
            iNumMediaMsgsRecvd = 0;
            iNumMediaMsgsSent = 0;
            iJitterBufferEmpty = true;
            iProcessIncomingMessages = true;
            iProcessOutgoingMessages = true;
            iCanReceivePktFromJB = false;
            iMonitorForRemoteActivity = false;
        }

        int32                       iId;
        PVMFJitterBufferNodePortTag iTag;
        PVMFJitterBufferPort&		irPort;
        PVMFJitterBuffer*			ipJitterBuffer;				//Only Input ports will have the jitter buffer associated with them
        uint32						iTimeScale;
        uint32						iBitrate;
        MediaClockConverter			iMediaClockConverter;
        PVMFTimestamp				iLastMsgTimeStamp;
        uint32						iNumMediaMsgsRecvd;
        uint32						iNumMediaMsgsSent;
        bool						iJitterBufferEmpty;
        bool						iProcessIncomingMessages;
        bool						iProcessOutgoingMessages;
        bool						iCanReceivePktFromJB;
        bool						iMonitorForRemoteActivity;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
};

/**
 * Input (sink) ports have a simple flow control scheme.
 * Ports report "busy" when their queue is full, then when the
 * queue goes to half-empty they issue a "get data" to the connected
 * port.  The media message in the "get data" is empty and is
 * meant to be discarded.
 * Output (source) ports assume the connected port uses the
 * same flow-control scheme.
 */
class PVMFJitterBufferPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfig
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFJitterBufferPort(int32 aTag, PVMFJitterBufferNode& aNode, const char*);

        /**
         * Constructor that allows the node to configure the data queues of this port.
         * @param aTag Port tag
         * @param aNode Container node
         * @param aSize Data queue capacity. The data queue size will not grow beyond this capacity.
         * @param aReserve Size of data queue for which memory is reserved. This must be
         * less than or equal to the capacity. If this is less than capacity, memory will be
         * allocated when the queue grows beyond the reserve size, but will stop growing at
         * capacity.
         * @param aThreshold Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         * This value should be between 0 - 100.
         */
        PVMFJitterBufferPort(int32 aTag,
                             PVMFJitterBufferNode& aNode,
                             uint32 aInCapacity,
                             uint32 aInReserve,
                             uint32 aInThreshold,
                             uint32 aOutCapacity,
                             uint32 aOutReserve,
                             uint32 aOutThreshold, const char*);

        /** Destructor */
        ~PVMFJitterBufferPort();

        /* Over ride Connect() */
        PVMFStatus Connect(PVMFPortInterface* aPort);

        /* Over ride QueryInterface - this port supports config interface */
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        // Unsupported PvmiCapabilityAndConfig methods
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_parameter_elements);
        };
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        }



        PVMFJitterBufferPortParams* GetPortParams()
        {
            return iPortParams;
        }
        //overrides from PVMFPortInterface
        PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg);
        bool IsOutgoingQueueBusy();
    private:
        void Construct();

        PVMFFormatType						iFormat;
        PVMFJitterBufferNodePortTag			iPortType;
        PVMFJitterBufferPortParams*			iPortParams;
        // Corresponding port paired with current port
        PVMFJitterBufferPort*				iPortCounterpart;
        // Parameters of port paired with current port
        PVMFJitterBufferPortParams*			iCounterpartPortParams;

        PVMFJitterBufferNode&				irJitterBufferNode;
        PVLogger*							ipLogger;

        friend class PVMFJitterBufferNode;
        friend class PVMFJitterBufferExtensionInterfaceImpl;
};

#endif // PVMF_JITTER_BUFFER_PORT_H_INCLUDED



