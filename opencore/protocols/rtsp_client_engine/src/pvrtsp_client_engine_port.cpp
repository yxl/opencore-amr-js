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
#include "pvrtsp_client_engine_port.h"

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

PVMFRTSPPort::PVMFRTSPPort(int32 aSdpTrackID, bool aIsMedia, int32 aTag, PVMFNodeInterface* aNode)
        : PvmfPortBaseImpl(aTag, aNode)
{
    iSdpTrackID = aSdpTrackID;
    iRdtStreamId = -1;
    bIsMedia = aIsMedia,
               bIsChannelIDSet = false;;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFRTSPPort::PVMFRTSPPort(int32 aSdpTrackID, bool aIsMedia, int32 aTag, PVMFNodeInterface* aNode
                           , uint32 aInCapacity
                           , uint32 aInReserve
                           , uint32 aInThreshold
                           , uint32 aOutCapacity
                           , uint32 aOutReserve
                           , uint32 aOutThreshold)
        : PvmfPortBaseImpl(aTag, aNode, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold)
{
    iSdpTrackID = aSdpTrackID;
    bIsMedia = aIsMedia,
               bIsChannelIDSet = false;;
    iRdtStreamId = -1;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFRTSPPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFRTSPPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
//	iNumFramesGenerated=0;
    iNumFramesConsumed = 0;
    PvmiCapabilityAndConfigPortFormatImpl::Construct(
        PVMF_RTSP_PORT_IO_FORMATS
        , PVMF_RTSP_PORT_IO_FORMATS_VALTYPE);
}

////////////////////////////////////////////////////////////////////////////
PVMFRTSPPort::~PVMFRTSPPort()
{
    Disconnect();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFRTSPPort::IsFormatSupported(PVMFFormatType aFmt)
{
//	return (aFmt==PVMF_INET_UDP)||(aFmt==PVMF_INET_TCP);
    if (aFmt == PVMF_MIME_INET_TCP)
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
void PVMFRTSPPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFRTSPPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}

void PVMFRTSPPort::setParametersSync(PvmiMIOSession aSession,
                                     PvmiKvp* aParameters,
                                     int num_elements,
                                     PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFRTSPPort::getParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d, aRet_kvp=0x%x",
                    aSession, aParameters, num_elements, aRet_kvp));

    if (!aParameters || (num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, PVMF_RTSP_PORT_IO_FORMATS_VALTYPE) != 0))
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
    if (aParameters->value.key_specific_value == NULL)
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
    else
    {
        aRet_kvp = NULL;
        //iAllocSharedPtr = *((OsclSharedPtr<PVMFSharedSocketDataBufferAlloc>*)(aParameters->value.key_specific_value));
    }
}

PVMFStatus PVMFRTSPPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFRTSPPort::getParametersSync: aSession=0x%x, aIdentifier=%s, aParameters=0x%x, num_parameters_elements=%d, aContext=0x%x",
                    aSession, aIdentifier, aParameters, num_parameter_elements, aContext));

    num_parameter_elements = 0;
    if (!pvmiGetPortInPlaceDataProcessingInfoSync(PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_KEY, aParameters))
    {
        return PVMFFailure;
    }
    num_parameter_elements = 1;
    return PVMFSuccess;
}

PVMFStatus PVMFRTSPPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFRTSPPort::releaseParameters: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                    aSession, aParameters, num_elements));

    if ((num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_KEY) != 0))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFRTSPPort::releaseParameters: Error - Not a PvmiKvp created by this port"));
        return PVMFFailure;
    }
    OsclMemAllocator alloc;
    //alloc.deallocate((OsclAny*)(aParameters->key));
    alloc.deallocate((OsclAny*)(aParameters));
    return PVMFSuccess;
}

bool
PVMFRTSPPort::pvmiGetPortInPlaceDataProcessingInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    /*
     * Create PvmiKvp for capability settings
     */
    aKvp = NULL;
    OsclMemAllocator alloc;
    uint32 strLen = oscl_strlen(aFormatValType) + 1;
    uint8* ptr = (uint8*)alloc.ALLOCATE(sizeof(PvmiKvp) + strLen);
    if (ptr == NULL)
    {
        return false;
    }
    aKvp = new(ptr) PvmiKvp;
    ptr += sizeof(PvmiKvp);
    aKvp->key = (PvmiKeyType)ptr;
    oscl_strncpy(aKvp->key, aFormatValType, strLen);
    aKvp->length = aKvp->capacity = strLen;
    aKvp->value.bool_value = false;
    return true;
}
