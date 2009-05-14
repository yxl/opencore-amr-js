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
 * @file pvmf_medialayer_port.cpp
 * @brief PVMF MediaLayer Port implementation
 */
#ifndef PVMF_MEDIALAYER_PORT_H_INCLUDED
#include "pvmf_medialayer_port.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_MEDIALAYER_NODE_H_INCLUDED
#include "pvmf_medialayer_node.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

#define PVMF_ML_PORT_OVERRIDE 1

////////////////////////////////////////////////////////////////////////////
PVMFMediaLayerPort::PVMFMediaLayerPort(int32 aTag, PVMFNodeInterface* aNode, const char*name)
        : PvmfPortBaseImpl(aTag, aNode, name)
{
    iMLNode = (PVMFMediaLayerNode*)aNode;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFMediaLayerPort::PVMFMediaLayerPort(int32 aTag, PVMFNodeInterface* aNode
                                       , uint32 aInCapacity
                                       , uint32 aInReserve
                                       , uint32 aInThreshold
                                       , uint32 aOutCapacity
                                       , uint32 aOutReserve
                                       , uint32 aOutThreshold
                                       , const char*name)
        : PvmfPortBaseImpl(aTag, aNode, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold, name)
{
    iMLNode = (PVMFMediaLayerNode*)aNode;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFMediaLayerPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFMediaLayerPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    /*
     * Odd numbered ports are output
     */
    if (iTag % 2)
    {
        iPortType = PVMF_MEDIALAYER_PORT_TYPE_OUTPUT;
    }
    /*
     * Even numbered ports are input
     */
    else
    {
        iPortType = PVMF_MEDIALAYER_PORT_TYPE_INPUT;

    }
}

////////////////////////////////////////////////////////////////////////////
PVMFMediaLayerPort::~PVMFMediaLayerPort()
{
    Disconnect();
    ClearMsgQueues();

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFMediaLayerPort::Connect(PVMFPortInterface* aPort)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::Connect: aPort=0x%x", aPort));

    if (!aPort)
    {
        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    if (iPortType == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
    {
        OsclAny* temp = NULL;
        aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
        PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

        if (config != NULL)
        {
            if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_FORMAT_SPECIFIC_INFO_KEY)))
            {
                PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::Connect: Error - Unable To Send Format Specific Info To Peer"));
                return PVMFFailure;
            }
            if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY)))
            {
                PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::Connect: Error - Unable To Send Max Num Media Msg Key To Peer"));
                return PVMFFailure;
            }
        }
    }

    /*
     * Automatically connect the peer.
     */
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFMediaLayerPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);

    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::getParametersSync: aSession=0x%x, aIdentifier=%s, aParameters=0x%x, num_parameters_elements=%d, aContext=0x%x",
                         aSession, aIdentifier, aParameters, num_parameter_elements, aContext));

    num_parameter_elements = 0;
    if (pv_mime_strcmp(aIdentifier, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (!pvmiGetPortFormatSpecificInfoSync(PVMF_FORMAT_SPECIFIC_INFO_KEY, aParameters))
        {
            return PVMFFailure;
        }
    }
    else if (pv_mime_strcmp(aIdentifier, PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY) == 0)
    {
        if (!pvmiGetPortFormatSpecificInfoSync(PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY, aParameters))
        {
            return PVMFFailure;
        }
    }
    num_parameter_elements = 1;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFMediaLayerPort::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);

    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::releaseParameters: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                         aSession, aParameters, num_elements));

    if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        alloc.deallocate((OsclAny*)(aParameters->key));
        return PVMFSuccess;
    }
    return PVMFErrNotSupported;
}

////////////////////////////////////////////////////////////////////////////
void PVMFMediaLayerPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);


    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::getParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d, aRet_kvp=0x%x",
                         aSession, aParameters, num_elements, aRet_kvp));

    //OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFMediaLayerPort::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);


    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::verifyParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                         aSession, aParameters, num_elements));

    return PVMFErrNotSupported;
}

////////////////////////////////////////////////////////////////////////////
void PVMFMediaLayerPort::freechunkavailable(OsclAny*)
{
    iMLNode->freechunkavailable(this);
}

////////////////////////////////////////////////////////////////////////////
bool
PVMFMediaLayerPort::pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
        const char* aFormatValType)
{
    /*
     * Create PvmiKvp for capability settings
     */
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;
    if (!(iMLNode->GetPortContainer((OSCL_STATIC_CAST(PVMFPortInterface*, this)), portContainerPtr)))
    {
        return false;
    }
    if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        PvmiKvp kvp;
        kvp.key = NULL;
        kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
        kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
        if (kvp.key == NULL)
        {
            return false;
        }
        oscl_strncpy(kvp.key, aFormatValType, kvp.length);
        if (portContainerPtr->iTrackConfig.getMemFragSize() == 0)
        {
            kvp.value.key_specific_value = 0;
            kvp.capacity = 0;
        }
        else
        {
            kvp.value.key_specific_value = (OsclAny*)(portContainerPtr->iTrackConfig.getMemFragPtr());
            kvp.capacity = portContainerPtr->iTrackConfig.getMemFragSize();
        }
        PvmiKvp* retKvp = NULL; // for return value
        int32 err;
        OSCL_TRY(err, aPort->setParametersSync(NULL, &kvp, 1, retKvp););
        /* ignore the error for now */
        alloc.deallocate((OsclAny*)(kvp.key));
        return true;
    }
    else if (pv_mime_strcmp(aFormatValType, PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY) == 0)
    {
        OsclMemAllocator alloc;
        PvmiKvp kvp;
        kvp.key = NULL;
        kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
        kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
        if (kvp.key == NULL)
        {
            return false;
        }
        oscl_strncpy(kvp.key, aFormatValType, kvp.length);

        kvp.value.uint32_value = MEDIALAYERNODE_MAXNUM_MEDIA_DATA;
        PvmiKvp* retKvp = NULL; // for return value
        int32 err;
        OSCL_TRY(err, aPort->setParametersSync(NULL, &kvp, 1, retKvp););
        /* ignore the error for now */
        alloc.deallocate((OsclAny*)(kvp.key));
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus
PVMFMediaLayerPort::pvmiVerifyPortFormatSpecificInfoSync(const char* aFormatValType,
        OsclAny* aConfig)
{
    /*
     * Create PvmiKvp for capability settings
     */
    PVMFStatus status = PVMFErrNotSupported;
    OsclAny* temp = NULL;
    iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *capConfig = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (capConfig != NULL)
    {
        if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
        {
            OsclRefCounterMemFrag* aFormatValue = (OsclRefCounterMemFrag*)aConfig;
            if (aFormatValue->getMemFragSize() > 0)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, aFormatValType, kvp.length);

                kvp.value.key_specific_value = (OsclAny*)(aFormatValue->getMemFragPtr());
                kvp.capacity = aFormatValue->getMemFragSize();
                int32 err;
                OSCL_TRY(err, status = capConfig->verifyParametersSync(NULL, &kvp, 1););
                /* ignore the error for now */
                alloc.deallocate((OsclAny*)(kvp.key));
                return status;
            }
        }
        else if (pv_mime_strcmp(aFormatValType, PVMF_BITRATE_VALUE_KEY) == 0 ||
                 pv_mime_strcmp(aFormatValType, PVMF_FRAMERATE_VALUE_KEY) == 0)
        {
            if (aConfig != NULL)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, aFormatValType, kvp.length);

                uint32* bitrate = (uint32*)aConfig;
                kvp.value.uint32_value = *bitrate;
                int32 err;
                OSCL_TRY(err, status = capConfig->verifyParametersSync(NULL, &kvp, 1););

                alloc.deallocate((OsclAny*)(kvp.key));

                return status;
            }
        }
        return PVMFErrArgument;
    }
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
bool
PVMFMediaLayerPort::pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    /*
     * Create PvmiKvp for capability settings
     */
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;
    if (!(iMLNode->GetPortContainer((OSCL_STATIC_CAST(PVMFPortInterface*, this)), portContainerPtr)))
    {
        return false;
    }
    if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        aKvp->key = NULL;
        aKvp->length = oscl_strlen(aFormatValType) + 1; // +1 for \0
        aKvp->key = (PvmiKeyType)alloc.ALLOCATE(aKvp->length);
        if (aKvp->key == NULL)
        {
            return false;
        }
        oscl_strncpy(aKvp->key, aFormatValType, aKvp->length);
        if (portContainerPtr->iTrackConfig.getMemFragSize() == 0)
        {
            aKvp->value.key_specific_value = 0;
            aKvp->capacity = 0;
        }
        else
        {
            aKvp->value.key_specific_value = (OsclAny*)(portContainerPtr->iTrackConfig.getMemFragPtr());
            aKvp->capacity = portContainerPtr->iTrackConfig.getMemFragSize();
        }
        return true;
    }
    else if (pv_mime_strcmp(aFormatValType, PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY) == 0)
    {
        OsclMemAllocator alloc;
        aKvp->key = NULL;
        aKvp->length = oscl_strlen(aFormatValType) + 1; // +1 for \0
        aKvp->key = (PvmiKeyType)alloc.ALLOCATE(aKvp->length);
        if (aKvp->key == NULL)
        {
            return false;
        }
        oscl_strncpy(aKvp->key, aFormatValType, aKvp->length);
        aKvp->value.uint32_value = MEDIALAYERNODE_MAXNUM_MEDIA_DATA;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
bool PVMFMediaLayerPort::peekHead(PVMFSharedMediaDataPtr& dataPtr, bool& bEos)
{
    if (iIncomingQueue.iQ.empty())
    {
        return false;
    }

    // get a pointer to the queue head
    PVMFSharedMediaMsgPtr msg = iIncomingQueue.iQ.front();

    // check the format id first - treat EOS special

    if (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        bEos = true;
    }
    else
    {
        convertToPVMFMediaData(dataPtr, msg);
        bEos = false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFMediaLayerPort::QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg)
{
#if PVMF_ML_PORT_OVERRIDE
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg"));

    //If port is not connected, don't accept data on the
    //outgoing queue.
    if (!iConnectedPort)
    {
        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerPort::QueueOutgoingMsg: Error - Port not connected"));
        return PVMFFailure;
    }

    PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);

    // Connected Port incoming Queue is in busy / flushing state.  Do not accept more outgoing messages
    // until the queue is not busy, i.e. queue size drops below specified threshold or FlushComplete
    // is called.
    if (cpPort->iIncomingQueue.iBusy)
    {
        PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg: Connected Port Incoming queue in busy / flushing state - Attempting to Q in output port's outgoing msg q"));
        return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
    }
    // In case there are data pending in iOutgoingQueue, we should try sending them first.
    if (!iOutgoingQueue.iQ.empty())
    {
        PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg: send pending data first"));
        PVMFStatus status = PvmfPortBaseImpl::Send();
        if (status != PVMFSuccess)
        {
            PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg: send pending data not success status = %d:", status));
            return status;
        }
        else if (cpPort->iIncomingQueue.iBusy)
        {
            PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg: Connected Port Incoming queue in busy / flushing state after sending pending data - Attempting to Q in output port's outgoing msg q"));
            return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
        }
    }


    // Add message to outgoing queue and notify the node of the activity
    // There is no need to trap the push_back, since it cannot leave in this usage
    // Reason being that we do a reserve in the constructor and we do not let the
    // port queues grow indefinitely (we either a connected port busy or outgoing Q busy
    // before we reach the reserved limit
    PVMFStatus status = cpPort->Receive(aMsg);

    if (status != PVMFSuccess)
    {
        return PVMFFailure;
    }

    // Outgoing queue size is at capacity and goes into busy state. The owner node is
    // notified of this transition into busy state.
    if (cpPort->isIncomingFull())
    {
        PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerPort::QueueOutgoingMsg: Connected Port incoming queue is full. Goes into busy state"));
        cpPort->iIncomingQueue.iBusy = true;
        PvmfPortBaseImpl::PortActivity(PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY);
    }

    return PVMFSuccess;
#else
    return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
#endif
}

bool PVMFMediaLayerPort::IsOutgoingQueueBusy()
{
#if PVMF_ML_PORT_OVERRIDE
    if (iPortType == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
    {
        if (iConnectedPort != NULL)
        {
            PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);
            return (cpPort->iIncomingQueue.iBusy);
        }
    }
    return (PvmfPortBaseImpl::IsOutgoingQueueBusy());;
#else
    return (PvmfPortBaseImpl::IsOutgoingQueueBusy());
#endif
}


