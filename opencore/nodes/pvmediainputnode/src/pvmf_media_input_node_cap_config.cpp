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

#include "pvmf_media_input_node.h"
#include "pvmf_media_input_node_outport.h"

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

struct MediaInputNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

#ifdef _TEST_AE_ERROR_HANDLING
#define MEDIAINPUTNODECONFIG_BASE_NUMKEYS 12
#else
#define MEDIAINPUTNODECONFIG_BASE_NUMKEYS 2
#endif
#define MEDIAINPUTCONFIG_KEYSTRING_SIZE 128

const MediaInputNodeKeyStringData MediaInputNodeConfig_BaseKeys[] =
{
    {"parameter1", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"parameter2", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#ifdef _TEST_AE_ERROR_HANDLING
    , {"error_adddatasource_start", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error_adddatasource_stop", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error_no_memorybuffer_avaliable", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error_out_queue_busy", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-time-stamp", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_KSV}
    , {"error-sendmiorequest", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-cancelmiorequest", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-corruptinputdata", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-node-cmd", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-data-path-stall", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#endif
};

enum BaseKeys_IndexMapType
{
    PARAMETER1 = 0,
    PARAMETER2
#ifdef _TEST_AE_ERROR_HANDLING
    , ERROR_ADDDATASOURCE_START
    , ERROR_ADDDATASOURCE_STOP
    , ERROR_NO_MEMORY
    , ERROR_OUT_QUEUE_BUSY
    , ERROR_TIME_STAMP
    , ERROR_SENDMIOREQUEST
    , ERROR_CANCELMIOREQUEST
    , ERROR_CORRUPT_INPUTDATA
    , ERROR_NODE_CMD
    , ERROR_DATAPATH_STALL
#endif
};

const uint MediaInputNodeConfig_NumBaseKeys =
    (sizeof(MediaInputNodeConfig_BaseKeys) /
     sizeof(MediaInputNodeKeyStringData));



////////////////////////////////////////////////////////////////////////////
//					PvmiCapConfigInterface
////////////////////////////////////////////////////////////////////////////

void PvmfMediaInputNode::createContext(PvmiMIOSession aSession,
                                       PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

void PvmfMediaInputNode::setContextParameters(PvmiMIOSession aSession,
        PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters,
        int aNumElements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

void PvmfMediaInputNode::DeleteContext(PvmiMIOSession aSession,
                                       PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

PVMFCommandId PvmfMediaInputNode::setParametersAsync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int aNumElements,
        PvmiKvp*& aRet_kvp,
        OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aNumElements);
    OSCL_UNUSED_ARG(aRet_kvp);

    return 0;
}

uint32 PvmfMediaInputNode::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);
    return 0;
}

PVMFStatus PvmfMediaInputNode::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::verifyParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::verifyParametersSync() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/datasource")) < 0) || compcount < 3)
        {
            // First 3 components should be "x-pvmf/datasource" and there must
            // be at least four components
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::verifyParametersSync() Unsupported key"));
            return PVMFErrNotSupported;
        }

        if (3 == compcount)
        {
            // Verify and set the passed-in composer node setting
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], false);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::verifyParametersSync() Setting parameter %d failed", paramind));
                return retval;
            }
        }
        else
        {
            // Do not support more than 3 components right now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::verifyParametersSync() Unsupported key"));
            return PVMFErrNotSupported;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::verifyParametersSync() Out"));
    return PVMFSuccess;
}

PVMFStatus PvmfMediaInputNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::releaseParameters()"));
    OSCL_UNUSED_ARG(aSession);

    if (aParameters == NULL || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::releaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/datasource")) < 0) || compcount < 2)
    {
        // First 2 component should be "x-pvmf/datasource" and there must
        // be at least three components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::releaseParameters() Unsupported key"));
        return PVMFErrNotSupported;
    }

    // Retrieve the third component from the key string
    pv_mime_string_extract_type(2, aParameters[0].key, compstr);

    // Go through each KVP and release memory for value if allocated from heap
    for (int32 ii = 0; ii < aNumElements; ++ii)
    {
        // Next check if it is a value type that allocated memory
        PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[ii].key);
        if (kvptype == PVMI_KVPTYPE_VALUE || kvptype == PVMI_KVPTYPE_UNKNOWN)
        {
            PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[ii].key);
            if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::releaseParameters() Valtype not specified in key string"));
                return PVMFErrNotSupported;
            }

            if (keyvaltype == PVMI_KVPVALTYPE_CHARPTR && aParameters[ii].value.pChar_value != NULL)
            {
                oscl_free(aParameters[ii].value.pChar_value);
                aParameters[ii].value.pChar_value = NULL;
            }
            else if (keyvaltype == PVMI_KVPVALTYPE_KSV && aParameters[ii].value.key_specific_value != NULL)
            {
                oscl_free(aParameters[ii].value.key_specific_value);
                aParameters[ii].value.key_specific_value = NULL;
            }
            else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_UINT32 && aParameters[ii].value.key_specific_value != NULL)
            {
                range_uint32* rui32 = (range_uint32*)aParameters[ii].value.key_specific_value;
                aParameters[ii].value.key_specific_value = NULL;
                oscl_free(rui32);
            }
            // @TODO Add more types if media io node starts returning more types
        }
    }

    // media io node allocated its key strings in one chunk so just free the first key string ptr
    oscl_free(aParameters[0].key);

    // Free memory for the parameter list
    oscl_free(aParameters);
    aParameters = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::releaseParameters() Out"));
    return PVMFSuccess;
}

PVMFStatus PvmfMediaInputNode::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::getParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    OSCL_UNUSED_ARG(aContext);

    // Initialize the output parameters
    aNumParamElements = 0;
    aParameters = NULL;

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aIdentifier);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aIdentifier, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/datasource")) < 0) || compcount < 2)
    {
        // First 2 components should be "x-pvmf/datasource" and there must
        // be at least 2 components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::getParametersSync() Invalid key string"));
        return PVMFErrNotSupported;
    }

    // Retrieve the third component from the key string
    pv_mime_string_extract_type(2, aIdentifier, compstr);

    for (int32 mediaiovalidind = 0; mediaiovalidind < MEDIAINPUTNODECONFIG_BASE_NUMKEYS; ++mediaiovalidind)
    {
        // Go through each media io component string at 3rd level
        if (pv_mime_strcmp(compstr, (char*)(MediaInputNodeConfig_BaseKeys[mediaiovalidind].iString)) >= 0)
        {
            if (3 == compcount)
            {
                // Determine what is requested
                PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                if (reqattr == PVMI_KVPATTR_UNKNOWN)
                {
                    reqattr = PVMI_KVPATTR_CUR;
                }

                // Return the requested info
                PVMFStatus retval = GetConfigParameter(aParameters, aNumParamElements, mediaiovalidind, reqattr);
                if (PVMFSuccess != retval)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::getParametersSync() Retrieving media io node parameter failed"));
                    return retval;
                }
            }
            else
            {
                // Right now media io node doesn't support more than 3 components
                // for this sub-key string so error out
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::getParametersSync() Unsupported key"));
                return PVMFErrNotSupported;
            }
            // Breakout of the for(mediaiovalidind) loop
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::getParametersSync() Out"));
    if (0 == aNumParamElements)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::getParametersSync() Unsupported key"));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

void PvmfMediaInputNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::setParametersSync()"));
    OSCL_UNUSED_ARG(aSession);
    // Complete the request synchronously

    if (NULL == aParameters || aNumElements < 1)
    {
        if (aParameters)
        {
            aRetKVP = aParameters;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::setParametersSync() Passed in parameter invalid"));
        return;
    }

    // Go through each parameter
    for (int32 paramind = 0; paramind < aNumElements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);
        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/datasource")) < 0) || compcount < 2)
        {
            // First 2 components should be "x-pvmf/datasource" and there must
            // be at least four components
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::setParametersSync() Unsupported key"));
            return;
        }

        if (3 == compcount)
        {
            // Verify and set the passed-in media input setting
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], true);
            if (PVMFSuccess != retval)
            {
                aRetKVP = &aParameters[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::setParametersSync() Setting parameter %d failed", paramind));
                return;
            }
        }
        else
        {
            // Do not support more than 3 components right now
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::setParametersSync() Unsupported key"));
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::setParametersSync() Out"));
}


PVMFStatus PvmfMediaInputNode::VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::VerifyAndSetConfigParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::VerifyAndSetConfigParameter() Valtype in key string unknown"));
        return PVMFErrNotSupported;
    }
    // Retrieve the third component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(2, aParameter.key, compstr);

    int32 mediaiovalidind;
    for (mediaiovalidind = 0; mediaiovalidind < MEDIAINPUTNODECONFIG_BASE_NUMKEYS; ++mediaiovalidind)
    {
        // Go through each component string at 3rd level
        if (pv_mime_strcmp(compstr, (char*)(MediaInputNodeConfig_BaseKeys[mediaiovalidind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (MEDIAINPUTNODECONFIG_BASE_NUMKEYS <= mediaiovalidind)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::VerifyAndSetConfigParameter() Unsupported key or non-leaf node"));
        return PVMFErrNotSupported;
    }

    // Verify the valtype
    if (keyvaltype != MediaInputNodeConfig_BaseKeys[mediaiovalidind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::VerifyAndSetConfigParameter() Valtype does not match for key"));
        return PVMFErrNotSupported;
    }

    switch (mediaiovalidind)
    {
        case PARAMETER1: // parameter1
            // Change the parameter
            if (aSetParam)
            {
                // set the parameter here
            }
            break;

        case PARAMETER2:  // parameter2
            // change the parameter
            if (aSetParam)
            {
                // set the parameter here
            }
            break;
#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_ADDDATASOURCE_START://error_adddatasource_start
            if (aSetParam)
            {
                iErrorHandlingStartFailed = aParameter.value.bool_value;
            }
            break;
        case ERROR_ADDDATASOURCE_STOP://error_adddatasource_stop
            if (aSetParam)
            {
                iErrorHandlingStopFailed = aParameter.value.bool_value;
            }
            break;
        case ERROR_NO_MEMORY:
            if (aSetParam)
            {
                iError_No_Memory = aParameter.value.bool_value;
            }
            break;
        case ERROR_OUT_QUEUE_BUSY:
            if (aSetParam)
            {
                iError_Out_Queue_Busy = aParameter.value.bool_value;
            }
            break;
        case ERROR_TIME_STAMP: //error-time-stamp
            if (aSetParam)
            {
                TimeStamp_KSV* TempTimeStamp = NULL;
                TempTimeStamp = OSCL_DYNAMIC_CAST(TimeStamp_KSV*, aParameter.value.key_specific_value);
                iErrorTimeStamp.mode = TempTimeStamp->mode;
                iErrorTimeStamp.duration = TempTimeStamp->duration;
                iErrorTimeStamp.track_no = TempTimeStamp->track_no;
            }
            break;
        case ERROR_SENDMIOREQUEST: //error-sendmiorequest
            if (aSetParam)
            {
                iErrorSendMioRequest = aParameter.value.uint32_value;
            }
            break;
        case ERROR_CANCELMIOREQUEST:  //error-cancelmiorequest
            if (aSetParam)
            {
                iErrorCancelMioRequest = aParameter.value.bool_value;
            }
            break;
        case ERROR_CORRUPT_INPUTDATA:  //error-corrupt-input-data
            if (aSetParam)
            {
                iChunkCount = aParameter.value.uint32_value;
                char* x = (char*)oscl_strstr(aParameter.key, "=");
                x = x + 1;
                PV_atoi(x, 'd', oscl_strlen(x), iTrackID);
            }
            break;
        case ERROR_NODE_CMD: //error-node-cmd
            if (aSetParam)
            {
                iErrorNodeCmd = aParameter.value.uint32_value;
            }
            break;
        case ERROR_DATAPATH_STALL: //error-data-path-stall
            if (aSetParam)
            {
                iErrorTrackID = aParameter.value.uint32_value;
            }
            break;
#endif
        default:
            OSCL_ASSERT(0);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::VerifyAndSetConfigParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PvmfMediaInputNode::GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr aReqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::GetConfigParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));

    if (NULL == aParameters)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::GetConfigParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }

    oscl_memset(aParameters, 0, sizeof(PvmiKvp));

    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(MEDIAINPUTCONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::GetConfigParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }

    oscl_strset(memblock, 0, MEDIAINPUTCONFIG_KEYSTRING_SIZE * sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/datasource/"), 21);
    oscl_strncat(aParameters[0].key, MediaInputNodeConfig_BaseKeys[aIndex].iString, oscl_strlen(MediaInputNodeConfig_BaseKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";valtype="), 20);

    switch (MediaInputNodeConfig_BaseKeys[aIndex].iValueType)
    {
        case PVMI_KVPVALTYPE_BITARRAY32:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BITARRAY32_STRING), oscl_strlen(PVMI_KVPVALTYPE_BITARRAY32_STRING));
            break;

        case PVMI_KVPVALTYPE_KSV:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
            break;

        case PVMI_KVPVALTYPE_BOOL:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
            break;

        case PVMI_KVPVALTYPE_INT32:
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            break;
        case PVMI_KVPVALTYPE_UINT32:
        default:
            if (PVMI_KVPATTR_CAP == aReqattr)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            else
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
            }
            break;
    }
    aParameters[0].key[MEDIAINPUTCONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case PARAMETER1:	// "parameter1"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // get the parameter here
                // parameter1
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
            }
            else
            {
                // Return capability
            }
            break;

        case PARAMETER2:	// "parameter2"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // get the parameter here
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
            }
            else
            {
                // Return capability
            }
            break;
#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_ADDDATASOURCE_START: //error_adddatasource_start
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iErrorHandlingStartFailed;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
            }
            break;
        case ERROR_ADDDATASOURCE_STOP: //error_adddatasource_stop
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iErrorHandlingStopFailed;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
            }
            break;
        case ERROR_NO_MEMORY:
        {
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iError_No_Memory;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
            }
        }
        break;
        case ERROR_OUT_QUEUE_BUSY:
        {
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iError_Out_Queue_Busy;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // Return default
                aParameters[0].value.bool_value = true;
            }
            else
            {
                // Return capability
            }
        }
        break;
#endif
        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PvmfMediaInputNode::GetConfigParameter() Invalid index to media io node parameter"));
            return PVMFErrNotSupported;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfMediaInputNode::GetConfigParameter() Out"));
    return PVMFSuccess;
}

void PvmfMediaInputNode::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    ciObserver = aObserver;
}
