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
#include "pvmf_videoparser_port.h"
#include "pvmf_videoparser_node.h"
#include "pv_mime_string_utils.h"


PVMFVideoParserPort::PVMFVideoParserPort(int32 aTag,
        PVMFFormatType format,
        PVMFNodeInterface* aNode,
        uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen,
        char*name)
        : PvmfPortBaseImpl(aTag, aNode, name),
        iFormatSpecificInfo(NULL),
        iFormatSpecificInfoLen(0)
{
    Construct(aTag, format, aFormatSpecificInfo, aFormatSpecificInfoLen);
}

////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserPort::Construct(int32 aTag, PVMFFormatType format, uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen)
{
    if (aTag == PVMF_VIDEOPARSER_NODE_PORT_TYPE_SINK)
    {
        iLogger = PVLogger::GetLoggerObject("PVMFVideoParserPort(Input)");
        PvmiCapabilityAndConfigPortFormatImpl::Construct(INPUT_FORMATS_CAP_QUERY , INPUT_FORMATS_VALTYPE);
    }
    else
    {
        iLogger = PVLogger::GetLoggerObject("PVMFVideoParserPort(Output)");
        PvmiCapabilityAndConfigPortFormatImpl::Construct(OUTPUT_FORMATS_CAP_QUERY , OUTPUT_FORMATS_VALTYPE);
    }
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iFormat = format;

    if (aFormatSpecificInfo && aFormatSpecificInfoLen)
    {
        iFormatSpecificInfo = (uint8*)OSCL_MALLOC(aFormatSpecificInfoLen);
        if (iFormatSpecificInfo == NULL)
        {
            OSCL_LEAVE(PVMFErrNoMemory);
        }
        oscl_memcpy(iFormatSpecificInfo, aFormatSpecificInfo, aFormatSpecificInfoLen);
        iFormatSpecificInfoLen = aFormatSpecificInfoLen;
    }
}


PVMFVideoParserPort::~PVMFVideoParserPort()
{
    if (iFormatSpecificInfo) delete iFormatSpecificInfo;

    Disconnect();
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFVideoParserPort::IsFormatSupported(PVMFFormatType aFmt)
{
    return ((aFmt == PVMF_MIME_M4V) || (aFmt == PVMF_MIME_H2631998) || (aFmt == PVMF_MIME_H2632000));
}

////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}


PVMFStatus PVMFVideoParserPort::Connect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::Connect: aPort=0x%x", aPort));
    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                        , (0, "PVMFVideoParserPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                        , (0, "PVMFVideoParserPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* config = NULL;

    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, config);

    if (config != NULL)
    {
        if (!(pvmiSetPortFormatSpecificInfoSync(OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, config),
                                                PVMF_FORMAT_SPECIFIC_INFO_KEY)))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                            , (0, "PVMFVideoParserPort::Connect: Error - Unable To Send Format Specific Info To Peer"));
            return PVMFFailure;
        }
    }

    /*
     * Automatically connect the peer.
     */
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                        , (0, "PVMFVideoParserPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

PVMFStatus PVMFVideoParserPort::PeerConnect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFVideoParserPort::PeerConnect: aPort=0x%x", this, aPort));

    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFVideoParserPort::PeerConnect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFVideoParserPort::PeerConnect: Error - Already connected", this));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    OsclAny *config = NULL;

    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, config);

    if (config != NULL)
    {
        if (!(pvmiSetPortFormatSpecificInfoSync(OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, config), PVMF_FORMAT_SPECIFIC_INFO_KEY)))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                            , (0, "PVMFVideoParserPort::PeerConnect: Error - Unable To Send Format Specific Info To Peer"));
            return PVMFFailure;
        }
    }

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

PVMFStatus PVMFVideoParserPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aContext);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::getParametersSync: aSession=0x%x, aIdentifier=%s, aParameters=0x%x, num_parameters_elements=%d, aContext=0x%x",
                       aSession, aIdentifier, aParameters, num_parameter_elements, aContext));

    num_parameter_elements = 0;

    if (pv_mime_strcmp(aIdentifier, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (!pvmiGetPortFormatSpecificInfoSync(PVMF_FORMAT_SPECIFIC_INFO_KEY, aParameters))
        {
            return PVMFFailure;
        }
        num_parameter_elements = 1;
        return PVMFSuccess;
    }
    return PvmiCapabilityAndConfigPortFormatImpl::getParametersSync(aSession, aIdentifier, aParameters, num_parameter_elements, aContext);
}


PVMFStatus PVMFVideoParserPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::releaseParameters: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                       aSession, aParameters, num_elements));

    if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        alloc.deallocate((OsclAny*)(aParameters->key));
        return PVMFSuccess;
    }

    return PvmiCapabilityAndConfigPortFormatImpl::releaseParameters(aSession, aParameters, num_elements);
}

void PVMFVideoParserPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(num_elements);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::getParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d, aRet_kvp=0x%x",
                       aSession, aParameters, num_elements, aRet_kvp));
}

PVMFStatus PVMFVideoParserPort::verifyParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFVideoParserPort::verifyParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                       aSession, aParameters, num_elements));
    return PVMFErrNotSupported;
}


bool
PVMFVideoParserPort::pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
        const char* aFormatValType)
{
    /*
     * Create PvmiKvp for capability settings
     */
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
        if (iFormatSpecificInfoLen == 0)
        {
            kvp.value.key_specific_value = 0;
            kvp.capacity = 0;
        }
        else
        {
            kvp.value.key_specific_value = (OsclAny*)iFormatSpecificInfo;
            kvp.capacity = iFormatSpecificInfoLen;
        }

        PvmiKvp* retKvp = NULL; // for return value
        int32 err;
        OSCL_TRY(err, aPort->setParametersSync(NULL, &kvp, 1, retKvp););
        /* ignore the error for now */
        alloc.deallocate((OsclAny*)(kvp.key));
        return true;
    }
    return false;
}

bool
PVMFVideoParserPort::pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    /*
     * Create PvmiKvp for capability settings
     */

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
        if (iFormatSpecificInfoLen == 0)
        {
            aKvp->value.key_specific_value = 0;
            aKvp->capacity = 0;
        }
        else
        {
            aKvp->value.key_specific_value = (OsclAny*)iFormatSpecificInfo;
            aKvp->capacity = iFormatSpecificInfoLen;
        }
        return true;
    }
    return false;
}
