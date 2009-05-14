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
 * @file pvmf_mp3ffparser_outport.cpp
 */

#ifndef PVMF_MP3FFPARSER_OUTPORT_H_INCLUDED
#include "pvmf_mp3ffparser_outport.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_MP3FFPARSER_DEFS_H_INCLUDED
#include "pvmf_mp3ffparser_defs.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef PVMF_MP3FFPARSER_NODE_H_INCLUDED
#include "pvmf_mp3ffparser_node.h"
#endif

////////////////////////////////////////////////////////////////////////////
PVMFMP3FFParserPort::PVMFMP3FFParserPort(int32 aTag, PVMFNodeInterface* aNode)
        : PvmfPortBaseImpl(aTag, aNode, "Mp3ParOut(Audio)")
{
    iMP3ParserNode = OSCL_STATIC_CAST(PVMFMP3FFParserNode*, aNode);
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFMP3FFParserPort::PVMFMP3FFParserPort(int32 aTag, PVMFNodeInterface* aNode,
        uint32 aInCapacity,
        uint32 aInReserve,
        uint32 aInThreshold,
        uint32 aOutCapacity,
        uint32 aOutReserve,
        uint32 aOutThreshold)
        : PvmfPortBaseImpl(aTag, aNode,
                           aInCapacity,
                           aInReserve,
                           aInThreshold,
                           aOutCapacity,
                           aOutReserve,
                           aOutThreshold,
                           "Mp3ParOut(Audio)")
{
    iMP3ParserNode = OSCL_STATIC_CAST(PVMFMP3FFParserNode*, aNode);
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFMP3FFParserPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFMP3FFParserPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    PvmiCapabilityAndConfigPortFormatImpl::Construct(PVMF_MP3FFPARSER_OUTPORT_FORMATS
            , PVMF_MP3FFPARSER_OUTPORT_FORMATS_VALTYPE);
}

////////////////////////////////////////////////////////////////////////////
PVMFMP3FFParserPort::~PVMFMP3FFParserPort()
{
    // Disconnect the port
    Disconnect();
    // Clear the queued messages
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFMP3FFParserPort::IsFormatSupported(PVMFFormatType aFmt)
{
    bool formatSupported = false;
    if (aFmt == PVMF_MIME_MP3)
    {
        formatSupported = true;
    }
    return formatSupported;
}

////////////////////////////////////////////////////////////////////////////
void PVMFMP3FFParserPort::FormatUpdated()
{
    return;
}

PVMFStatus PVMFMP3FFParserPort::Connect(PVMFPortInterface* aPort)
{
    PVMF_MP3FPARSERNODE_LOGINFO((0, "PVMFMP3FFParserPort::Connect: aPort=0x%x", aPort));

    if (!aPort)
    {
        PVMF_MP3FPARSERNODE_LOGERROR((0, "PVMFMP3FFParserPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVMF_MP3FPARSERNODE_LOGERROR((0, "PVMFMP3FFParserPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (config != NULL)
    {
        if (!(pvmiSetPortFormatSpecificInfoSync(config, PVMF_FORMAT_SPECIFIC_INFO_KEY)))
        {
            PVMF_MP3FPARSERNODE_LOGERROR((0, "PVMFMP3FFParserPort::Connect: Error - Unable To Send Format Specific Info To Peer"));
            return PVMFFailure;
        }
    }

    /*
     * Automatically connect the peer.
     */
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVMF_MP3FPARSERNODE_LOGERROR((0, "PVMFMP3FFParserPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

bool
PVMFMP3FFParserPort::pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
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
        int32 KeyLenth = oscl_strlen(aFormatValType) + 1; // +1 for \0
        kvp.key = (PvmiKeyType)alloc.ALLOCATE(KeyLenth);
        if (kvp.key == NULL)
        {
            return false;
        }
        oscl_strncpy(kvp.key, aFormatValType, KeyLenth);
        if (iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragSize() == 0)
        {
            kvp.value.key_specific_value = 0;
            kvp.length = kvp.capacity = 0;
        }
        else
        {
            kvp.value.key_specific_value = (OsclAny*)(iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragPtr());
            kvp.length = kvp.capacity = iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragSize();
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
PVMFMP3FFParserPort::pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
        PvmiKvp*& aKvp)
{
    /*
     * Create PvmiKvp for capability settings
     */
    if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        aKvp->key = NULL;
        int32 KeyLenth = oscl_strlen(aFormatValType) + 1; // +1 for \0
        aKvp->key = (PvmiKeyType)alloc.ALLOCATE(KeyLenth);
        if (aKvp->key == NULL)
        {
            return false;
        }
        oscl_strncpy(aKvp->key, aFormatValType, KeyLenth);
        if (iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragSize() == 0)
        {
            aKvp->value.key_specific_value = 0;
            aKvp->length = aKvp->capacity = 0;
        }
        else
        {
            aKvp->value.key_specific_value = (OsclAny*)(iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragPtr());
            aKvp->length = aKvp->capacity = iMP3ParserNode->iDecodeFormatSpecificInfo.getMemFragSize();
        }
        return true;
    }
    return false;
}

PVMFStatus PVMFMP3FFParserPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    PVMF_MP3FPARSERNODE_LOGINFO((0, "PVMFMP3FFParserPort::getParametersSync: aSession=0x%x, aIdentifier=%s, aParameters=0x%x, num_parameters_elements=%d, aContext=0x%x",
                                 aSession, aIdentifier, aParameters, num_parameter_elements, aContext));

    num_parameter_elements = 0;

    if (pv_mime_strcmp(aIdentifier, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (!pvmiGetPortFormatSpecificInfoSync(PVMF_FORMAT_SPECIFIC_INFO_KEY, aParameters))
        {
            return PVMFFailure;
        }
    }

    num_parameter_elements = 1;
    return PVMFSuccess;
}

PVMFStatus PVMFMP3FFParserPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_elements);

    PVMF_MP3FPARSERNODE_LOGINFO((0, "PVMFMP3FFParserPort::releaseParameters: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                                 aSession, aParameters, num_elements));

    if (pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        OsclMemAllocator alloc;
        alloc.deallocate((OsclAny*)(aParameters->key));
    }
    return PVMFSuccess;
}
