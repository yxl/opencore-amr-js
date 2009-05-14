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
#include "pvmf_buffer_data_sink.h"

OSCL_EXPORT_REF PVMFBufferDataSink::PVMFBufferDataSink(int32 aPortTag)
        : PvmfPortBaseImpl(aPortTag, this)
{
    iNumBytesReceived = 0;
    iNumPktsReceived = 0;
    iNumPktErrorsReceived = 0;
}

OSCL_EXPORT_REF PVMFBufferDataSink::~PVMFBufferDataSink()
{
}

void PVMFBufferDataSink::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    if (aActivity.iType != PVMF_PORT_ACTIVITY_INCOMING_MSG)
        return;
    PVMFSharedMediaMsgPtr aMsg;
    PVMFStatus aStatus;
    while (IncomingMsgQueueSize())
    {
        aStatus = DequeueIncomingMsg(aMsg);
        PutData(aMsg);
    }
}

// PVMFPortInterface virtuals

PVMFStatus PVMFBufferDataSink::PutData(PVMFSharedMediaMsgPtr aMsg)
{
    OSCL_UNUSED_ARG(aMsg);
    iNumPktsReceived++;
    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, aMsg);
    iNumBytesReceived += (mediaData->getFilledSize());
    if (mediaData->getErrorsFlag())
        iNumPktErrorsReceived++;
    return PVMFSuccess;
}

PVMFStatus PVMFBufferDataSink::GetData(PVMFSharedMediaMsgPtr aMsg)
{
    OSCL_UNUSED_ARG(aMsg);
    return PVMFSuccess;
}

OSCL_EXPORT_REF unsigned PVMFBufferDataSink::GetNumBytesReceived()
{
    return iNumBytesReceived;
}

OSCL_EXPORT_REF unsigned PVMFBufferDataSink::GetNumPktsReceived()
{
    return iNumPktsReceived;
}

OSCL_EXPORT_REF unsigned PVMFBufferDataSink::GetNumPktErrorsReceived()
{
    return iNumPktErrorsReceived;
}

OSCL_EXPORT_REF void PVMFBufferDataSink::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSink::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters, int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
    OSCL_UNUSED_ARG(aContext);

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSink::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    return PVMFSuccess;
}

OSCL_EXPORT_REF void PVMFBufferDataSink::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
}

OSCL_EXPORT_REF void PVMFBufferDataSink::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

OSCL_EXPORT_REF void PVMFBufferDataSink::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
}

OSCL_EXPORT_REF void PVMFBufferDataSink::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
}

OSCL_EXPORT_REF PVMFCommandId PVMFBufferDataSink::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(context);

    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF uint32 PVMFBufferDataSink::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);

    return 1;
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSink::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    OSCL_LEAVE(OsclErrNotSupported);
    return PVMFFailure;
}

OSCL_EXPORT_REF void PVMFBufferDataSink::QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr)
{
    aPtr = NULL;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        aPtr = (PvmiCapabilityAndConfig*)this;
    }
    else
    {
        OSCL_LEAVE(OsclErrNotSupported);
    }
}
