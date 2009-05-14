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
#ifndef PVMP4FFCN_NODE_H_INCLUDED
#include "pvmp4ffcn_node.h"
#endif

#ifndef PVMP4FFCN_PORT_H_INCLUDED
#include "pvmp4ffcn_port.h"
#endif

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

#ifndef PVMI_FILEIO_KVP_H_INCLUDED
#include "pvmi_fileio_kvp.h"
#endif

#define DEFAULT_CAHCE_SIZE 8*1024
#define MIN_CACHE_SIZE 1024
#define MAX_CACHE_SIZE 128*1024

struct MP4ComposerNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

#ifdef _TEST_AE_ERROR_HANDLING
#define MP4COMPOSERNODECONFIG_BASE_NUMKEYS 10
#else
#define MP4COMPOSERNODECONFIG_BASE_NUMKEYS 2
#endif
#define MP4CONFIG_KEYSTRING_SIZE 128

static const MP4ComposerNodeKeyStringData MP4ComposerNodeConfig_BaseKeys[] =
{
    {"presentation-timescale", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"pv-cache-size", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#ifdef _TEST_AE_ERROR_HANDLING
    , {"error_start_addmemfrag", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error_start_addtrack", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-addtrack", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_CHARPTR}
    , {"error-node-cmd", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"error-create-composer", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-render-to-file", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL}
    , {"error-addsample", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
    , {"data-path-stall", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
#endif
};

enum BaseKeys_IndexMapType
{
    PRESENTATION_TIMESCALE = 0,
    PV_CACHE_SIZE
#ifdef _TEST_AE_ERROR_HANDLING
    , ERROR_START_ADDMEMFRAG
    , ERROR_START_ADDTRACK
    , ERROR_ADDTRACK
    , ERROR_NODE_CMD
    , ERROR_CREATE_COMPOSER
    , ERROR_RENDERTOFILE
    , ERROR_ADD_SAMPLE
    , ERROR_DATAPATH_STALL
#endif
};


///////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerNode::GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr aReqattr)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::GetConfigParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (NULL == aParameters)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::GetConfigParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));
    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(MP4CONFIG_KEYSTRING_SIZE * sizeof(char));
    if (NULL == memblock)
    {
        oscl_free(aParameters);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::GetConfigParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, MP4CONFIG_KEYSTRING_SIZE * sizeof(char));
    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/composer/mp4/"), 7);
    oscl_strncat(aParameters[0].key, MP4ComposerNodeConfig_BaseKeys[aIndex].iString, oscl_strlen(MP4ComposerNodeConfig_BaseKeys[aIndex].iString));
    oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
    switch (MP4ComposerNodeConfig_BaseKeys[aIndex].iValueType)
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
            if (aReqattr == PVMI_KVPATTR_CUR)
            {
                oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
            }
            break;
        case PVMI_KVPVALTYPE_CHARPTR:
            oscl_strncat(aParameters[0].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING), oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
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
    aParameters[0].key[MP4CONFIG_KEYSTRING_SIZE-1] = 0;

    // Copy the requested info
    switch (aIndex)
    {
        case PRESENTATION_TIMESCALE:	// "presentation-timescale"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.uint32_value = iPresentationTimescale;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // return default
            }
            else
            {
                // Return capability
            }
            break;

        case PV_CACHE_SIZE:	// "pv-cache-size"
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // set any parameter here
                aParameters[0].value.uint32_value = iCacheSize;

            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // return default
                aParameters[0].value.uint32_value = DEFAULT_CAHCE_SIZE;
            }
            else
            {
                // Return capability
                range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                if (NULL == rui32)
                {
                    oscl_free(aParameters[0].key);
                    oscl_free(aParameters);
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::GetConfigParameter() Memory allocation for range uint32 failed"));
                    return PVMFErrNoMemory;
                }
                rui32->min = MIN_CACHE_SIZE;
                rui32->max = MAX_CACHE_SIZE;
                aParameters[0].value.key_specific_value = (void*)rui32;
            }
            break;
#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_START_ADDMEMFRAG:
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iErrorHandlingAddMemFrag;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // return default
                aParameters[0].value.bool_value  = true;
            }
            else
            {
                // Return capability
            }
            break;
        case ERROR_START_ADDTRACK:
            if (PVMI_KVPATTR_CUR == aReqattr)
            {
                // Return current value
                aParameters[0].value.bool_value = iErrorHandlingAddTrack;
            }
            else if (PVMI_KVPATTR_DEF == aReqattr)
            {
                // return default
                aParameters[0].value.bool_value  = true;
            }
            else
            {
                // Return capability
            }
            break;
#endif
        default:
            // Invalid index
            oscl_free(aParameters[0].key);
            oscl_free(aParameters);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::GetConfigParameter() Invalid index to composer  node parameter"));
            return PVMFErrNotSupported;
    }

    aNumParamElements = 1;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::GetConfigParameter() Out"));
    return PVMFSuccess;
}


PVMFStatus PVMp4FFComposerNode::VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::VerifyAndSetConfigParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::VerifyAndSetConfigParameter() Valtype in key string unknown"));
        return PVMFErrNotSupported;// key is not supported here
    }

    // Retrieve the fourth component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(3, aParameter.key, compstr);

    int32 mp4comp4ind;
    for (mp4comp4ind = 0; mp4comp4ind < MP4COMPOSERNODECONFIG_BASE_NUMKEYS; ++mp4comp4ind)
    {
        // Go through each component string at 4th level
        if (pv_mime_strcmp(compstr, (char*)(MP4ComposerNodeConfig_BaseKeys[mp4comp4ind].iString)) >= 0)
        {
            // Break out of the for loop
            break;
        }
    }

    if (MP4COMPOSERNODECONFIG_BASE_NUMKEYS <= mp4comp4ind)
    {
        // Match couldn't be found or non-leaf node specified
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::VerifyAndSetConfigParameter() Unsupported key or non-leaf node"));
        return PVMFErrNotSupported;
    }

    // Verify the valtype
    if (keyvaltype != MP4ComposerNodeConfig_BaseKeys[mp4comp4ind].iValueType)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::VerifyAndSetConfigParameter() Valtype does not match for key"));
        return PVMFErrNotSupported;
    }

    switch (mp4comp4ind)
    {
        case PRESENTATION_TIMESCALE: // "presentation-timescale"
            // Change the parameter
            if (aSetParam)
            {
                iPresentationTimescale = aParameter.value.uint32_value;
            }
            break;

        case PV_CACHE_SIZE: // "pv-cache-size"
            // change the parameter
            if (aSetParam)
            {
                // set any parameter here
                iCacheSize = aParameter.value.uint32_value ;
            }
            break;
#ifdef _TEST_AE_ERROR_HANDLING
        case ERROR_START_ADDMEMFRAG:
            // change the parameter
            if (aSetParam)
            {
                // set any parameter here
                iErrorHandlingAddMemFrag = aParameter.value.bool_value ;
            }
            break;
        case ERROR_START_ADDTRACK:
            // change the parameter
            if (aSetParam)
            {
                // set any parameter here
                iErrorHandlingAddTrack = aParameter.value.bool_value ;
            }
            break;
        case ERROR_ADDTRACK: //error in AddTrack()
            if (aSetParam)
            {
                //char* paramstr = NULL;
                char* val_key = aParameter.value.pChar_value;
                if (pv_mime_strcmp(val_key, "PVMF_MIME_H264_VIDEO_MP4") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_H264_VIDEO_MP4;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_3GPP_TIMEDTEXT") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_3GPP_TIMEDTEXT;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_M4V") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_M4V;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_H2631998") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_H2631998;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_H2632000") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_H2632000;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_AMR_IETF") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_AMR_IETF;
                }
                if (pv_mime_strcmp(val_key, "PVMF_MIME_AMRWB_IETF") == 0)
                {
                    iErrorAddTrack = PVMF_MIME_AMRWB_IETF;
                }

            }
            break;
        case ERROR_NODE_CMD:
            if (aSetParam)
            {
                iErrorNodeCmd = aParameter.value.uint32_value;
            }
            break;
        case ERROR_CREATE_COMPOSER:
            if (aSetParam)
            {
                iErrorCreateComposer = aParameter.value.bool_value ;
            }
            break;
        case ERROR_RENDERTOFILE:
            if (aSetParam)
            {
                iErrorRenderToFile = aParameter.value.bool_value ;
            }
            break;
        case ERROR_ADD_SAMPLE:
            if (aSetParam)
            {
                char* paramstr = NULL;
                OSCL_HeapString<OsclMemAllocator> mode1 = "mode=filesize";
                OSCL_HeapString<OsclMemAllocator> mode2 = "mode=duration";

                if (pv_mime_string_parse_param(aParameter.key, mode1.get_str(), paramstr) > 0)
                {
                    iFileSize = aParameter.value.uint32_value;
                    iErrorAddSample = 1;
                }
                else if (pv_mime_string_parse_param(aParameter.key, mode2.get_str(), paramstr) > 0)
                {
                    iFileDuration = aParameter.value.uint32_value;
                    iErrorAddSample = 2;
                }

            }
            break;
        case ERROR_DATAPATH_STALL:
            if (aSetParam)
            {
                iErrorDataPathStall = aParameter.value.uint32_value;
            }
            break;
#endif
        default:
            OSCL_ASSERT(0);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::VerifyAndSetConfigParameter() Out"));
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
//					PvmiCapConfigInterface Virtual Functions
////////////////////////////////////////////////////////////////////////////

void PVMp4FFComposerNode::createContext(PvmiMIOSession aSession,
                                        PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

void PVMp4FFComposerNode::setContextParameters(PvmiMIOSession aSession,
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

void PVMp4FFComposerNode::DeleteContext(PvmiMIOSession aSession,
                                        PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

PVMFCommandId PVMp4FFComposerNode::setParametersAsync(PvmiMIOSession aSession,
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

uint32 PVMp4FFComposerNode::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);
    return 0;
}

PVMFStatus PVMp4FFComposerNode::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::verifyParametersSync()"));
    OSCL_UNUSED_ARG(aSession);

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::verifyParametersSync() Passed in parameter invalid"));
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

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/composer/mp4")) < 0) || compcount < 2)
        {
            // First 2 components should be "x-pvmf/composer/mp4" and there must
            // be at least four components
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::verifyParametersSync() Unsupported key"));
            return PVMFErrNotSupported;
        }

        if (2 == compcount)
        {
            // Verify and set the passed-in composer node setting
            // PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], false);
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], false);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::verifyParametersSync() Setting parameter %d failed", paramind));
                return retval;
            }
        }
        else
        {
            // Do not support more than 2 components right now
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::verifyParametersSync() Unsupported key"));
            return PVMFErrNotSupported;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::verifyParametersSync() Out"));
    return PVMFSuccess;
}

PVMFStatus PVMp4FFComposerNode::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::releaseParameters()"));
    OSCL_UNUSED_ARG(aSession);

    if (NULL == aParameters || aNumElements < 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::releaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    // Count the number of components and parameters in the key
    int compcount = pv_mime_string_compcnt(aParameters[0].key);
    // Retrieve the first component from the key string
    char* compstr = NULL;
    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/composer/mp4")) < 0) || compcount < 2)
    {
        // First 2 component should be "x-pvmf/composer/mp4" and there must
        // be at least two components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::releaseParameters() Unsupported key"));
        return PVMFErrNotSupported;
    }

    // Retrieve the third component from the key string
    pv_mime_string_extract_type(1, aParameters[0].key, compstr);

    // Go through each KVP and release memory for value if allocated from heap
    for (int32 ii = 0; ii < aNumElements; ++ii)
    {
        // Next check if it is a value type that allocated memory
        PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[ii].key);
        if (PVMI_KVPTYPE_VALUE == kvptype || PVMI_KVPTYPE_UNKNOWN == kvptype)
        {
            PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[ii].key);
            if (PVMI_KVPVALTYPE_UNKNOWN == keyvaltype)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::releaseParameters() Valtype not specified in key string"));
                return PVMFErrNotSupported;
            }

            if (PVMI_KVPVALTYPE_CHARPTR == keyvaltype && NULL != aParameters[ii].value.pChar_value)
            {
                oscl_free(aParameters[ii].value.pChar_value);
                aParameters[ii].value.pChar_value = NULL;
            }
            else if (PVMI_KVPVALTYPE_KSV == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
            {
                oscl_free(aParameters[ii].value.key_specific_value);
                aParameters[ii].value.key_specific_value = NULL;
            }
            else if (PVMI_KVPVALTYPE_RANGE_INT32 == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
            {
                range_int32* ri32 = (range_int32*)aParameters[ii].value.key_specific_value;
                aParameters[ii].value.key_specific_value = NULL;
                oscl_free(ri32);
            }
            else if (PVMI_KVPVALTYPE_RANGE_UINT32 == keyvaltype && NULL != aParameters[ii].value.key_specific_value)
            {
                range_uint32* rui32 = (range_uint32*)aParameters[ii].value.key_specific_value;
                aParameters[ii].value.key_specific_value = NULL;
                oscl_free(rui32);
            }
            // @TODO Add more types if composer node starts returning more types
        }
    }

    // composer node allocated its key strings in one chunk so just free the first key string ptr
    oscl_free(aParameters[0].key);

    // Free memory for the parameter list
    oscl_free(aParameters);
    aParameters = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::releaseParameters() Out"));
    return PVMFSuccess;
}



PVMFStatus PVMp4FFComposerNode::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::getParametersSync()"));
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

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/composer/mp4")) < 0) || compcount < 2)
    {
        // First 2 components should be "x-pvmf/composer/mp4" and there must
        // be at least 2 components
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::getParametersSync() Invalid key string"));
        return PVMFErrNotSupported;
    }
    // Retrieve the fourth component from the key string
    pv_mime_string_extract_type(1, aIdentifier, compstr);

    for (int32 mp4comp4ind = 0; mp4comp4ind < MP4COMPOSERNODECONFIG_BASE_NUMKEYS; ++mp4comp4ind)
    {
        // Go through each composer  component string at th level
        if (pv_mime_strcmp(compstr, (char*)(MP4ComposerNodeConfig_BaseKeys[mp4comp4ind].iString)) >= 0)
        {
            if (2 == compcount)
            {
                // Determine what is requested
                PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
                if (PVMI_KVPATTR_UNKNOWN == reqattr)
                {
                    reqattr = PVMI_KVPATTR_CUR;
                }
                // Return the requested info
                PVMFStatus retval = GetConfigParameter(aParameters, aNumParamElements, mp4comp4ind, reqattr);
                if (PVMFSuccess != retval)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::getParametersSync() Retrieving composer node parameter failed"));
                    return retval;
                }
            }
            else
            {
                // Right now composer node doesn't support more than 4 components
                // for this sub-key string so error out
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::getParametersSync() Unsupported key"));
                return PVMFErrNotSupported;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::getParametersSync() Out"));
    if (0 == aNumParamElements)
    {
        // If no one could get the parameter, return error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::getParametersSync() Unsupported key"));
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

void PVMp4FFComposerNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::setParametersSync()"));
    OSCL_UNUSED_ARG(aSession);
    // Complete the request synchronously

    if (NULL == aParameters || aNumElements < 1)
    {
        if (aParameters)
        {
            aRetKVP = aParameters;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::setParametersSync() Passed in parameter invalid"));
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

        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf/composer/mp4")) < 0) || compcount < 2)
        {
            // First 2 components should be "x-pvmf/composer/mp4" and there must
            // be at least 2 components
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::setParametersSync() Unsupported key"));
            return;
        }

        if (4 == compcount)
        {
            // Verify and set the passed-in mp4 composer node setting
            PVMFStatus retval = VerifyAndSetConfigParameter(aParameters[paramind], true);
            if (PVMFSuccess != retval)
            {
                aRetKVP = &aParameters[paramind];
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::setParametersSync() Setting parameter %d failed", paramind));
                return;
            }
        }
        else
        {
            // Do not support more than 4 components right now
            aRetKVP = &aParameters[paramind];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMp4FFComposerNode::setParametersSync() Unsupported key"));
            return;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMp4FFComposerNode::setParametersSync() Out"));
}

void PVMp4FFComposerNode::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    ciObserver = aObserver;
}
