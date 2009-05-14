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
#include "pvmf_aacffparser_outport.h"

#include "pvmf_aacffparser_node.h"

#include "media_clock_converter.h"
#include "pvmf_aacffparser_defs.h"

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

PVMFAACFFParserOutPort::PVMFAACFFParserOutPort(int32 aTag, PVMFNodeInterface* aNode)
        : PvmfPortBaseImpl(aTag, aNode, "AacFFParOut(Audio)")
{
    iAACParserNode = OSCL_STATIC_CAST(PVMFAACFFParserNode*, aNode);
    Construct();


}
void PVMFAACFFParserOutPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFAACParserOutPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iNumFramesConsumed = 0;
}

PVMFAACFFParserOutPort::~PVMFAACFFParserOutPort()
{
    Disconnect();
    ClearMsgQueues();
}


bool PVMFAACFFParserOutPort::IsFormatSupported(PVMFFormatType aFmt)
{
    bool formatSupported = false;
    if (aFmt == PVMF_MIME_MPEG4_AUDIO ||
            aFmt == PVMF_MIME_ADIF)
    {
        formatSupported = true;
    }
    return formatSupported;
}

void PVMFAACFFParserOutPort::FormatUpdated()
{
    PVMF_AACPARSERNODE_LOGSTACKTRACE((0, "PVMFAACFFParserOutPort::FormatUpdated"));
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

PVMFStatus PVMFAACFFParserOutPort::Connect(PVMFPortInterface* aPort)
{

    if (!aPort)
    {
        PVMF_AACPARSERNODE_LOGERROR((0, "PVMFAACParserOutPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVMF_AACPARSERNODE_LOGERROR((0, "PVMFAACParserOutPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (config != NULL)
    {
        if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_FORMAT_SPECIFIC_INFO_KEY)))
        {
            PVMF_AACPARSERNODE_LOGERROR((0, "PVMFAACParserOutPort::Connect: Error - Unable To Send Format Specific Info To Peer"));
            return PVMFFailure;
        }

        if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_DATAPATH_PORT_MAX_NUM_MEDIA_MSGS_KEY)))
        {
            PVMF_AACPARSERNODE_LOGERROR((0, "PVMFAACParserOutPort::Connect: Error - Unable To Send Max Num Media Msg Key To Peer"));
            return PVMFFailure;
        }
    }

    /*
     * Automatically connect the peer.
     */
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVMF_AACPARSERNODE_LOGERROR((0, "PVMFAACParserOutPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

PVMFStatus PVMFAACFFParserOutPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aContext);

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


PVMFStatus PVMFAACFFParserOutPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);

    if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        alloc.deallocate((OsclAny*)(aParameters->key));
        return PVMFSuccess;
    }
    return PVMFErrNotSupported;
}

void PVMFAACFFParserOutPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(num_elements);

}

PVMFStatus PVMFAACFFParserOutPort::verifyParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    return PVMFErrNotSupported;
}


bool
PVMFAACFFParserOutPort::pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
        const char* aFormatValType)
{
    /*
     * Create PvmiKvp for capability settings
     */
    PVAACFFNodeTrackPortInfo* trackInfoPtr = NULL;
    if (!(iAACParserNode->GetTrackInfo((OSCL_STATIC_CAST(PVMFPortInterface*, this)), trackInfoPtr)))
    {
        return false;
    }
    if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (trackInfoPtr->iFormatSpecificConfig.getMemFragSize() > 0)
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

            kvp.value.key_specific_value = (OsclAny*)(trackInfoPtr->iFormatSpecificConfig.getMemFragPtr());
            kvp.capacity = trackInfoPtr->iFormatSpecificConfig.getMemFragSize();
            PvmiKvp* retKvp = NULL; // for return value
            int32 err;
            OSCL_TRY(err, aPort->setParametersSync(NULL, &kvp, 1, retKvp););
            /* ignore the error for now */
            alloc.deallocate((OsclAny*)(kvp.key));
        }
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

        kvp.value.uint32_value = PVMF_AAC_PARSER_NODE_MAX_NUM_OUTSTANDING_MEDIA_MSGS;
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
PVMFAACFFParserOutPort::pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    /*
     * Create PvmiKvp for capability settings
     */
    PVAACFFNodeTrackPortInfo* trackInfoPtr = NULL;
    if (!(iAACParserNode->GetTrackInfo((OSCL_STATIC_CAST(PVMFPortInterface*, this)), trackInfoPtr)))
    {
        return false;
    }
    if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (trackInfoPtr->iFormatSpecificConfig.getMemFragSize() > 0)
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

            aKvp->value.key_specific_value = (OsclAny*)(trackInfoPtr->iFormatSpecificConfig.getMemFragPtr());
            aKvp->capacity = trackInfoPtr->iFormatSpecificConfig.getMemFragSize();
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
        aKvp->value.uint32_value = PVMF_AAC_PARSER_NODE_MAX_NUM_OUTSTANDING_MEDIA_MSGS;
        return true;
    }
    return false;
}
