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
 * @file pvmf_sample_port.cpp
 * @brief Sample PVMF Port implementation
 */

#ifndef PVMF_FILEINPUT_PORT_H_INCLUDED
#include "pvmf_fileinput_port.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef PVMF_DUMMY_FILEINPUT_NODE_H_INCLUDED
#include "pvmf_dummy_fileinput_node.h"
#endif

#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);


////////////////////////////////////////////////////////////////////////////
PVMFFileDummyInputPort::PVMFFileDummyInputPort(int32 aTag, PVMFNodeInterface* aNode)
        : PvmfPortBaseImpl(aTag, aNode)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFFileDummyInputPort::PVMFFileDummyInputPort(int32 aTag, PVMFNodeInterface* aNode
        , uint32 aInCapacity
        , uint32 aInReserve
        , uint32 aInThreshold
        , uint32 aOutCapacity
        , uint32 aOutReserve
        , uint32 aOutThreshold)
        : PvmfPortBaseImpl(aTag, aNode, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileDummyInputPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFFileDummyInputPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iTrackConfigFI = NULL;
    iTrackConfigSizeFI = 0;
}

////////////////////////////////////////////////////////////////////////////
PVMFFileDummyInputPort::~PVMFFileDummyInputPort()
{
    Disconnect();
    ClearMsgQueues();

    if (iTrackConfigFI)
    {
        oscl_free(iTrackConfigFI);
        iTrackConfigFI = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFFileDummyInputPort::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFFileDummyInputPort::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    LOG_STACK_TRACE((0, "PVMFFileDummyInputPort::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);


    num_parameter_elements = 0;
    PVMFStatus status = PVMFFailure;
    PVMFDummyFileInputNode* node = (PVMFDummyFileInputNode*)iPortActivityHandler;


    if (pv_mime_strcmp(identifier, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0 && (iFormat == PVMF_MIME_WMV || iFormat == PVMF_MIME_WMA))
    {
        if (pvmiGetPortFormatSpecificInfoSync(PVMF_FORMAT_SPECIFIC_INFO_KEY, parameters) == 0)
        {
            return PVMFFailure;
        }
        else
        {
            num_parameter_elements = 1;
            return PVMFSuccess;
        }

    }

    parameters = NULL;
    if (pv_mime_strcmp(identifier, OUTPUT_FORMATS_CAP_QUERY) == 0 ||
            pv_mime_strcmp(identifier, OUTPUT_FORMATS_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, OSCL_STATIC_CAST(PvmiKeyType, OUTPUT_FORMATS_VALTYPE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::getParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.pChar_value = (char*)node->iSettings.iMediaFormat.getMIMEStrPtr();
        }
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_WIDTH_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, OSCL_STATIC_CAST(PvmiKeyType, VIDEO_OUTPUT_WIDTH_CUR_VALUE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        parameters[0].value.uint32_value = node->iSettings.iFrameWidth;
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_HEIGHT_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, OSCL_STATIC_CAST(PvmiKeyType, VIDEO_OUTPUT_HEIGHT_CUR_VALUE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        parameters[0].value.uint32_value = node->iSettings.iFrameHeight;
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, OSCL_STATIC_CAST(PvmiKeyType, VIDEO_OUTPUT_FRAME_RATE_CUR_VALUE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        parameters[0].value.float_value = node->iSettings.iFrameRate;
    }
    else if (pv_mime_strcmp(identifier, OUTPUT_TIMESCALE_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, OSCL_STATIC_CAST(PvmiKeyType, OUTPUT_TIMESCALE_CUR_VALUE), num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFVideoEncPort::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {
            if ((node->iSettings.iMediaFormat).isAudio())
            {
                parameters[0].value.uint32_value = node->iSettings.iSamplingFrequency;
            }
            else
            {
                parameters[0].value.uint32_value = node->iSettings.iTimescale;
            }
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFFileDummyInputPort::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);

    if (parameters)
    {
        iAlloc.deallocate((OsclAny*)parameters->key);
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFFileDummyInputPort::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFFileDummyInputPort::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFFileDummyInputPort::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFFileDummyInputPort::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    OSCL_UNUSED_ARG(session);
    PVMFStatus status = PVMFSuccess;
    ret_kvp = NULL;

    for (int32 i = 0; i < num_elements; i++)
    {
        status = VerifyAndSetParameter(&(parameters[i]), true);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::setParametersSync: Error - VerifiyAndSetParameter failed on parameter #%d", i));
            ret_kvp = &(parameters[i]);
            OSCL_LEAVE(OsclErrArgument);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFFileDummyInputPort::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PVMFFileDummyInputPort::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFFileDummyInputPort::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    OSCL_UNUSED_ARG(session);

    PVMFStatus status = PVMFSuccess;
    for (int32 i = 0; (i < num_elements) && (status == PVMFSuccess); i++)
        status = VerifyAndSetParameter(&(parameters[i]));

    return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFFileDummyInputPort::Connect(PVMFPortInterface* aPort)
{


    if (!aPort)
    {
        LOG_ERR((0, "PVMFFileDummyInputPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        LOG_ERR((0, "PVMFFileDummyInputPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig* config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (config != NULL)
    {
        if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_FORMAT_SPECIFIC_INFO_KEY)))
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::Connect: Error - Unable To Send Format Specific Info To Peer"));
            return PVMFFailure;
        }
    }

    /*
     * Automatically connect the peer.
     */
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        LOG_ERR((0, "PVMFFileDummyInputPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
bool PVMFFileDummyInputPort::IsFormatSupported(PVMFFormatType aFmt)
{
    return ((aFmt == PVMF_MIME_YUV420)		        ||
            (aFmt == PVMF_MIME_RGB16)		        ||
            (aFmt == PVMF_MIME_M4V)		        ||
            (aFmt == PVMF_MIME_WMV)				||
            (aFmt == PVMF_MIME_WMA)				||
            (aFmt == PVMF_MIME_H2632000)		        ||
            (aFmt == PVMF_MIME_AMR_IF2)	        ||
            (aFmt == PVMF_MIME_AMR_IETF)	        ||
            (aFmt == PVMF_MIME_PCM16)		        ||
            (aFmt == PVMF_MIME_ADTS)		        ||
            (aFmt == PVMF_MIME_MPEG4_AUDIO)        ||
            (aFmt == PVMF_MIME_LATM)		        ||
            (aFmt == PVMF_MIME_MP3)		        ||
            (aFmt == PVMF_MIME_ADIF)		        ||
            (aFmt == PVMF_MIME_AAC_SIZEHDR)		||
            (aFmt == PVMF_MIME_G726)		        ||
            (aFmt == PVMF_MIME_YUV420)				||
            (aFmt == PVMF_MIME_REAL_AUDIO));
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileDummyInputPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFFileDummyInputPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileDummyInputPort::AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams)
{
    LOG_STACK_TRACE((0, "PVMFFileDummyInputPort::AllocateKvp"));
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
             buf = (uint8*)iAlloc.ALLOCATE(aNumParams * (sizeof(PvmiKvp) + keyLen));
             if (!buf)
             OSCL_LEAVE(OsclErrNoMemory);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVMFFileDummyInputPort::AllocateKvp: Error - kvp allocation failed"));
                         return PVMFErrNoMemory;
                        );

    int32 i = 0;
    PvmiKvp* curKvp = aKvp = OSCL_PLACEMENT_NEW(buf, PvmiKvp);
    buf += sizeof(PvmiKvp);
    for (i = 1; i < aNumParams; i++)
    {
        curKvp += i;
        curKvp = OSCL_PLACEMENT_NEW(buf, PvmiKvp);
        buf += sizeof(PvmiKvp);
    }

    for (i = 0; i < aNumParams; i++)
    {
        aKvp[i].key = (char*)buf;
        oscl_strncpy(aKvp[i].key, aKey, oscl_strlen(aKvp[i].key));
        buf += oscl_strlen(aKvp[i].key);
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileDummyInputPort::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    LOG_STACK_TRACE((0, "PVMFFileDummyInputPort::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        LOG_ERR((0, "PVMFFileDummyInputPort::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    PVMFDummyFileInputNode* node = (PVMFDummyFileInputNode*)iPortActivityHandler;

    if (pv_mime_strcmp(aKvp->key, OUTPUT_FORMATS_VALTYPE) == 0)
    {
        if (aKvp->value.pChar_value == node->iSettings.iMediaFormat.getMIMEStrPtr())
        {
            if (aSetParam)
                iFormat = aKvp->value.pChar_value;
            return PVMFSuccess;
        }
        else
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::VerifyAndSetParameter: Error - Unsupported format %d",
                     aKvp->value.uint32_value));
            return PVMFFailure;
        }
    }

    LOG_ERR((0, "PVMFFileDummyInputPort::VerifyAndSetParameter: Error - Unsupported parameter"));
    return PVMFFailure;
}

//////////////////////////////////////////////////////////////////////////////////////
bool PVMFFileDummyInputPort::pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    PVMFDummyFileInputNode* node = (PVMFDummyFileInputNode*)iPortActivityHandler;

    if ((iFormat == PVMF_MIME_WMV) || (iFormat == PVMF_MIME_WMA))
    {
        if (node->iFs.Connect() != 0) return false;
        node->iInputFile.Open(node->iSettings.iFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, node->iFs);
        node->iInputFile.Read((OsclAny*) &iTrackConfigSizeFI, sizeof(uint8), 4);

        iTrackConfigFI = (uint8*) iAlloc.allocate(iTrackConfigSizeFI);
        node->iInputFile.Read((OsclAny*) iTrackConfigFI, sizeof(uint8), iTrackConfigSizeFI);
        node->iInputFile.Close();

        if (iTrackConfigSizeFI > 0)
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

            aKvp->value.key_specific_value = (OsclAny*)(iTrackConfigFI);
            aKvp->capacity = iTrackConfigSizeFI;
        }
        return true;
    }

    else
        return true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool PVMFFileDummyInputPort::pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
        const char* aFormatValType)
{
    PVMFDummyFileInputNode* node = OSCL_STATIC_CAST(PVMFDummyFileInputNode*, iPortActivityHandler);
    PvmiKvp* retKvp = NULL; // for return value
    int32 err = 0;
    PvmiKvp* kvpPtr = NULL;
    if (iFormat == PVMF_MIME_WMV)
    {
        if (node->iFs.Connect() != 0) return false;
        node->iInputFile.Open(node->iSettings.iFileName.get_cstr(),
                              Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, node->iFs);
        node->iInputFile.Read((OsclAny*) &iTrackConfigSizeFI, sizeof(uint8), 4);

        iTrackConfigFI = (uint8*)oscl_malloc(iTrackConfigSizeFI);
        node->iInputFile.Read((OsclAny*) iTrackConfigFI, sizeof(uint8), iTrackConfigSizeFI);
        node->iInputFile.Close();

        if (iTrackConfigSizeFI > 0)
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

            kvp.value.key_specific_value = (OsclAny*) iTrackConfigFI;
            kvp.capacity = iTrackConfigSizeFI;
            OSCL_TRY(err, aPort->setParametersSync(NULL, &kvp, 1, retKvp););
            /* ignore the error for now */
            alloc.deallocate((OsclAny*)(kvp.key));
        }
    }
    else if (iFormat == PVMF_MIME_G726)
    {
        // Send a fake G726 format specific info containing number of bits per sample in the
        // 14th byte of the info buffer.
        if (AllocateKvp(kvpPtr, OSCL_STATIC_CAST(PvmiKeyType, aFormatValType), 1) != PVMFSuccess)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::pvmiSetPortFormatSpecificInfoSync: Error - AllocateKvp failed"));
            return false;
        }

        kvpPtr->value.key_specific_value = OSCL_MALLOC(32);
        if (kvpPtr->value.key_specific_value == NULL)
        {
            LOG_ERR((0, "PVMFFileDummyInputPort::pvmiSetPortFormatSpecificInfoSync: Error - OSCL_MALLOC failed"));
            return false;
        }
        oscl_memset(kvpPtr->value.key_specific_value, 0, 32);

        int16* bitsPerSamplePtr;
        bitsPerSamplePtr = OSCL_STATIC_CAST(int16*,
                                            (OSCL_STATIC_CAST(uint8*, kvpPtr->value.key_specific_value) + 14));
        *bitsPerSamplePtr = (node->iSettings.iBitrate) / (node->iSettings.iSamplingFrequency);

        OSCL_TRY(err, aPort->setParametersSync(NULL, kvpPtr, 1, retKvp););
        OSCL_FIRST_CATCH_ANY(err,
                             LOG_ERR((0, "PVMFFileDummyInputPort::pvmiSetPortFormatSpecificInfoSync: Error - setParametersSync failed. err=%d", err));
                             return false;
                            );

        OSCL_FREE(kvpPtr->value.key_specific_value);
        releaseParameters(NULL, kvpPtr, 1);
    }

    return true;
}
