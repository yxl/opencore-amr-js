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
 * @file pvmf_mp3ffparser_outport.h
 */

#ifndef PVMF_MP3FFPARSER_OUTPORT_H_INCLUDED
#define PVMF_MP3FFPARSER_OUTPORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

//Default vector reserve size
#define PVMF_MP3FFPARSER_NODE_PORT_VECTOR_RESERVE 10

#define PVMF_MP3FPARSERNODE_LOGINFO(m)  PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MP3FPARSERNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

class PVMFMP3FFParserNode;

/**
 * An example of a PVMF port implementation.
 *
 * Input (sink) ports have a simple flow control scheme.
 * Ports report "busy" when their queue is full, then when the
 * queue goes to half-empty they issue a "get data" to the connected
 * port.  The media message in the "get data" is empty and is
 * meant to be discarded.
 * Output (source) ports assume the connected port uses the
 * same flow-control scheme.
 */
class PVMFMP3FFParserPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFMP3FFParserPort(int32 aTag
                            , PVMFNodeInterface* aNode);

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
        PVMFMP3FFParserPort(int32 aTag
                            , PVMFNodeInterface* aNode
                            , uint32 aInCapacity
                            , uint32 aInReserve
                            , uint32 aInThreshold
                            , uint32 aOutCapacity
                            , uint32 aOutReserve
                            , uint32 aOutThreshold);

        /** Destructor */
        ~PVMFMP3FFParserPort();

        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        /* Over ride Connect() */
        PVMFStatus Connect(PVMFPortInterface* aPort);

        /* Implement pure virtuals from PvmiCapabilityAndConfig interface */
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        /* Unsupported PvmiCapabilityAndConfig methods */
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(num_elements);
        }
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            return PVMFErrNotSupported;
        }
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
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        }

    private:
        void Construct();

        bool pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
                                               const char* aFormatValType);

        bool pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
                                               PvmiKvp*& aKvp);

        PVLogger *iLogger;
        uint32 iNumFramesGenerated; //number of source frames generated.

        PVMFMP3FFParserNode * iMP3ParserNode;
        friend class PVMFMP3FFParserNode;
};

#endif // PVMF_MP3FFPARSER_OUTPORT_H_INCLUDED



