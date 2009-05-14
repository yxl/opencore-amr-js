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
#ifndef PVMF_MP4FFPARSER_NODE_H_INCLUDED
#include "pvmf_mp4ffparser_node.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif
#ifndef PVMI_FILEIO_KVP_H_INCLUDED
#include "pvmi_fileio_kvp.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Capability and config interface related constants and definitions
//   - based on pv_player_engine.h
//
///////////////////////////////////////////////////////////////////////////////
struct MP4ParserNodeKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};


// The number of characters to allocate for the key string
#define MP4CONFIG_KEYSTRING_SIZE 128

//The base selection keys for file IO,leading tag needs to be fileio
static const MP4ParserNodeKeyStringData MP4ParserNodeConfig_FileIO_Keys[] =
{
    {"pv-cache-size", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"async-read-buffer-size", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"logger-enable", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL},
    {"logger-stats-enable", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL},
    {"native-access-mode", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"file-handle", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_KSV}
};

//The base selection keys for file IO,leading tag needs to be x-pvmf\net
static const MP4ParserNodeKeyStringData MP4ParserNodeConfig_Net_Keys[] =
{
    {"delay", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
};

static const uint MP4ParserNodeConfig_Num_FileIO_Keys =
    (sizeof(MP4ParserNodeConfig_FileIO_Keys) /
     sizeof(MP4ParserNodeKeyStringData));

static const uint MP4ParserNodeConfig_Num_Net_Keys =
    (sizeof(MP4ParserNodeConfig_Net_Keys) /
     sizeof(MP4ParserNodeKeyStringData));


enum BaseFileIOKeys_IndexMapType
{
    PV_CACHE_SIZE = 0,
    ASYNC_READ_BUFFER_SIZE,
    BASEKEY_PVLOGGER_ENABLE,
    PVLOGGER_STATS_ENABLE,
    NATIVE_ACCESS_MODE,
    FILE_HANDLE,
};

enum BaseNetKeys_IndexMapType
{
    DELAY = 0
};



///////////////////////////////////////////////////////////////////////////////

PVMFStatus PVMFMP4FFParserNode::getParametersSync(
    PvmiMIOSession aSession, PvmiKeyType aIdentifier,
    PvmiKvp*& aParameters, int& aNumParamElements,
    PvmiCapabilityContext aContext)
{
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

    iBaseKey = INVALID;

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("fileio")) < 0) || compcount < 2)
    {
        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
        {

            // First component should be "fileio" or "x-pvmf" and there must
            // be at least two components to go past fileio and x-pvmf
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::getParametersSync() Invalid key string"));
            return PVMFErrArgument;

        }
        else
        {
            // Retrieve the second component from the key string
            pv_mime_string_extract_type(1, aIdentifier, compstr);

            // Check if it is key string for streaming
            if (pv_mime_strcmp(compstr, _STRLIT_CHAR("net")) < 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::getParametersSync() Unsupported key"));
                return PVMFFailure;
            }
            iBaseKey = NET;
        }
    }
    else
    {
        //Since the key string matches that of fileio
        iBaseKey = FILE_IO;
    }

    //If the leading tag is fileio then component count should be 2
    if ((iBaseKey == FILE_IO) && (compcount == 2))
    {
        //BaseKey's leading tag is fileio;
        pv_mime_string_extract_type(1, aIdentifier, compstr);

        // Determine what is requested
        PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
        if (reqattr == PVMI_KVPATTR_UNKNOWN)
        {
            reqattr = PVMI_KVPATTR_CUR;
        }
        uint i;
        for (i = 0; i < MP4ParserNodeConfig_Num_FileIO_Keys; i++)
        {
            if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_FileIO_Keys[i].iString)) >= 0)
            {
                break;
            }
        }

        if (i == MP4ParserNodeConfig_Num_FileIO_Keys)
        {
            // no match found
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::getParametersSync() Unsupported key"));
            return PVMFErrNoMemory;
        }

        PVMFStatus retval = GetConfigParameter(aParameters, aNumParamElements, i, reqattr);
        if (retval != PVMFSuccess)
        {
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::getParametersSync() "
                                          "Retrieving mp4 parser node parameter failed"));
            return retval;
        }
    }
    else if ((iBaseKey == NET) && ((compcount == 2) || (compcount == 3)))
    {
        //	//BaseKey's leading tag is x-pvmf\net

        if (compcount == 2)
        {
            // Since key is "x-pvmf/net" return all
            // nodes available at this level. Ignore attribute
            // since capability is only allowed

            // Allocate memory for the KVP list
            aParameters = (PvmiKvp*)oscl_malloc(MP4ParserNodeConfig_Num_Net_Keys * sizeof(PvmiKvp));
            if (aParameters == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::getParametersSync() Memory allocation for KVP failed"));
                return PVMFErrNoMemory;
            }
            oscl_memset(aParameters, 0, MP4ParserNodeConfig_Num_Net_Keys*sizeof(PvmiKvp));
            // Allocate memory for the key strings in each KVP
            PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(MP4ParserNodeConfig_Num_Net_Keys * MP4CONFIG_KEYSTRING_SIZE * sizeof(char));
            if (memblock == NULL)
            {
                oscl_free(aParameters);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::getParametersSync() Memory allocation for key string failed"));
                return PVMFErrNoMemory;
            }
            oscl_strset(memblock, 0, MP4ParserNodeConfig_Num_Net_Keys*MP4CONFIG_KEYSTRING_SIZE*sizeof(char));
            // Assign the key string buffer to each KVP
            int32 j;
            for (j = 0; j < (int32)MP4ParserNodeConfig_Num_Net_Keys; ++j)
            {
                aParameters[j].key = memblock + (j * MP4CONFIG_KEYSTRING_SIZE);
            }
            // Copy the requested info
            for (j = 0; j < (int32)MP4ParserNodeConfig_Num_Net_Keys; ++j)
            {
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR("x-pvmf/net/"), 11);
                oscl_strncat(aParameters[j].key, MP4ParserNodeConfig_Net_Keys[j].iString, oscl_strlen(MP4ParserNodeConfig_Net_Keys[j].iString));
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";type="), 6);
                switch (MP4ParserNodeConfig_Net_Keys[j].iType)
                {
                    case PVMI_KVPTYPE_AGGREGATE:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_AGGREGATE_STRING), oscl_strlen(PVMI_KVPTYPE_AGGREGATE_STRING));
                        break;

                    case PVMI_KVPTYPE_POINTER:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_POINTER_STRING), oscl_strlen(PVMI_KVPTYPE_POINTER_STRING));
                        break;

                    case PVMI_KVPTYPE_VALUE:
                    default:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPTYPE_VALUE_STRING), oscl_strlen(PVMI_KVPTYPE_VALUE_STRING));
                        break;
                }
                oscl_strncat(aParameters[j].key, _STRLIT_CHAR(";valtype="), 9);
                switch (MP4ParserNodeConfig_Net_Keys[j].iValueType)
                {
                    case PVMI_KVPVALTYPE_RANGE_INT32:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
                        break;

                    case PVMI_KVPVALTYPE_KSV:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING), oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
                        break;

                    case PVMI_KVPVALTYPE_CHARPTR:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING), oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
                        break;

                    case PVMI_KVPVALTYPE_BOOL:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING), oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                        break;

                    case PVMI_KVPVALTYPE_UINT32:
                    default:
                        oscl_strncat(aParameters[j].key, _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING), oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                        break;
                }
                aParameters[j].key[MP4CONFIG_KEYSTRING_SIZE-1] = 0;
            }

            aNumParamElements = MP4ParserNodeConfig_Num_Net_Keys;
        }
        else if (compcount == 3)
        {
            pv_mime_string_extract_type(2, aIdentifier, compstr);

            // Determine what is requested
            PvmiKvpAttr reqattr = GetAttrTypeFromKeyString(aIdentifier);
            if (reqattr == PVMI_KVPATTR_UNKNOWN)
            {
                reqattr = PVMI_KVPATTR_CUR;
            }
            int32 i;
            for (i = 0; i < (int32)MP4ParserNodeConfig_Num_Net_Keys; i++)
            {
                if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_Net_Keys[i].iString)) >= 0)
                {
                    break;
                }
            }

            if (i == (int32)MP4ParserNodeConfig_Num_Net_Keys)
            {
                // no match found
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFMP4FFParserNode::getParametersSync() Unsupported key"));
                return PVMFErrNoMemory;
            }

            PVMFStatus retval = GetConfigParameter(aParameters, aNumParamElements, i, reqattr);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFMP4FFParserNode::getParametersSync() "
                                 "Retrieving streaming manager parameter failed"));
                return retval;
            }
        }
    }
    else
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::getParametersSync() Unsupported key"));
        return PVMFErrArgument;
    }

    return PVMFSuccess;
}

PVMFStatus PVMFMP4FFParserNode::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::releaseParameters() In"));

    if (aParameters == NULL || num_elements < 1)
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::releaseParameters() KVP list is NULL or number of elements is 0"));
        return PVMFErrArgument;
    }

    int32 compcount = pv_mime_string_compcnt(aParameters[0].key);

    // Retrieve the first component from the key string
    char* compstr = NULL;

    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    BaseKeys_SelectionType basekey = INVALID;

    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("fileio")) < 0) || compcount < 2)
    {
        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) < 0) || compcount < 2)
        {
            // First component should be "fileio" or "x-pvmf" and there must
            // be at least two components to go past fileio and x-pvmf
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::getParametersSync() Invalid key string"));
            return PVMFErrArgument;
        }
        else
        {
            // Retrieve the second component from the key string
            pv_mime_string_extract_type(1, aParameters[0].key, compstr);

            // Check if it is key string for streaming
            if (pv_mime_strcmp(compstr, _STRLIT_CHAR("net")) < 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::getParametersSync() Unsupported key"));
                return PVMFErrArgument;
            }
            basekey = NET;
        }
    }
    else
    {
        //Since the key string matches that of fileio
        basekey = FILE_IO;
    }

    pv_mime_string_extract_type(0, aParameters[0].key, compstr);

    //It does not effect in releasing memory what are the base keys either fileio or x-pvmf.
    if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("fileio")) >= 0) || (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) >= 0))
    {
        // Go through each KVP and release memory for value if allocated from heap
        for (int32 i = 0; i < num_elements; ++i)
        {
            // Next check if it is a value type that allocated memory
            PvmiKvpType kvptype = GetTypeFromKeyString(aParameters[i].key);
            if (kvptype == PVMI_KVPTYPE_VALUE || kvptype == PVMI_KVPTYPE_UNKNOWN)
            {
                PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[i].key);
                if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
                {
                    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::releaseParameters() Valtype not specified in key string"));
                    return PVMFErrArgument;
                }

                if (keyvaltype == PVMI_KVPVALTYPE_CHARPTR && aParameters[i].value.pChar_value != NULL)
                {
                    oscl_free(aParameters[i].value.pChar_value);
                    aParameters[i].value.pChar_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_WCHARPTR && aParameters[i].value.pWChar_value != NULL)
                {
                    oscl_free(aParameters[i].value.pWChar_value);
                    aParameters[i].value.pWChar_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_CHARPTR && aParameters[i].value.pChar_value != NULL)
                {
                    oscl_free(aParameters[i].value.pChar_value);
                    aParameters[i].value.pChar_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_KSV && aParameters[i].value.key_specific_value != NULL)
                {
                    oscl_free(aParameters[i].value.key_specific_value);
                    aParameters[i].value.key_specific_value = NULL;
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_INT32 && aParameters[i].value.key_specific_value != NULL)
                {
                    range_int32* ri32 = (range_int32*)aParameters[i].value.key_specific_value;
                    aParameters[i].value.key_specific_value = NULL;
                    oscl_free(ri32);
                }
                else if (keyvaltype == PVMI_KVPVALTYPE_RANGE_UINT32 && aParameters[i].value.key_specific_value != NULL)
                {
                    range_uint32* rui32 = (range_uint32*)aParameters[i].value.key_specific_value;
                    aParameters[i].value.key_specific_value = NULL;
                    oscl_free(rui32);
                }
            }
        }

        oscl_free(aParameters[0].key);

        // Free memory for the parameter list
        oscl_free(aParameters);
        aParameters = NULL;
    }
    else
    {
        // Unknown key string
        return PVMFErrArgument;
    }

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::releaseParameters() Out"));
    return PVMFSuccess;
}

void PVMFMP4FFParserNode::createContext(PvmiMIOSession aSession,
                                        PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

void PVMFMP4FFParserNode::setContextParameters(PvmiMIOSession aSession,
        PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters,
        int num_parameter_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}

void PVMFMP4FFParserNode::DeleteContext(PvmiMIOSession aSession,
                                        PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    // not supported
    OSCL_LEAVE(PVMFErrNotSupported);
}


void PVMFMP4FFParserNode::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp* &aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() In"));


    // Go through each parameter
    for (int32 paramind = 0; paramind < num_elements; ++paramind)
    {
        // Count the number of components and parameters in the key
        int compcount = pv_mime_string_compcnt(aParameters[paramind].key);

        // Retrieve the first component from the key string
        char* compstr = NULL;
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        // First check if it is key string for the mp4 parser node
        if (pv_mime_strcmp(compstr, _STRLIT_CHAR("fileio")) >= 0)
        {
            iBaseKey = FILE_IO;
            if (compcount == 2)
            {
                pv_mime_string_extract_type(1, aParameters[paramind].key, compstr);
                uint i;
                for (i = 0; i < MP4ParserNodeConfig_Num_FileIO_Keys; i++)
                {
                    if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_FileIO_Keys[i].iString)) >= 0)
                    {
                        break;
                    }
                }

                if (MP4ParserNodeConfig_Num_FileIO_Keys == i)
                {
                    // invalid third component
                    aRet_kvp = &aParameters[paramind];
                    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
                    return;
                }

                // Verify and set the passed-in setting
                PVMFStatus retval = VerifyAndSetConfigParameter(i, aParameters[paramind], true);
                if (retval != PVMFSuccess)
                {
                    aRet_kvp = &aParameters[paramind];
                    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Setting "
                                                  "parameter %d failed", paramind));
                    return;
                }
            }
            else
            {
                // Do not support more than 2 components right now
                aRet_kvp = &aParameters[paramind];
                PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
                return;
            }
        }
        else if (pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) >= 0)
        {
            if (compcount == 3)
            {
                // Retrieve the second component from the key string
                pv_mime_string_extract_type(1, aParameters[paramind].key, compstr);
                // First check if it is key string for the streaming manager
                if (pv_mime_strcmp(compstr, _STRLIT_CHAR("net")) >= 0)
                {
                    iBaseKey = NET;
                    int32 i;
                    //Extract the third component from the key string
                    pv_mime_string_extract_type(2, aParameters[paramind].key, compstr);
                    for (i = 0; i < (int32)MP4ParserNodeConfig_Num_Net_Keys; i++)
                    {
                        if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_Net_Keys[i].iString)) >= 0)
                        {
                            break;
                        }
                    }

                    if ((int32)MP4ParserNodeConfig_Num_Net_Keys == i)
                    {
                        // invalid third component
                        aRet_kvp = &aParameters[paramind];
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
                        return;
                    }

                    // Verify and set the passed-in setting
                    PVMFStatus retval = VerifyAndSetConfigParameter(i, aParameters[paramind], true);
                    if (retval != PVMFSuccess)
                    {
                        aRet_kvp = &aParameters[paramind];
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Setting "
                                                      "parameter %d failed", paramind));
                        return;
                    }
                }
                else if (pv_mime_strcmp(compstr, _STRLIT_CHAR("parser/ff_noaudio")) >= 0)
                {
                    // Make sure its a bool value
                    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[paramind].key);
                    if (PVMI_KVPVALTYPE_BOOL != keyvaltype)
                    {
                        aRet_kvp = &aParameters[paramind];
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync Setting "
                                                      "ff_noaudio valtype error"));
                        return;
                    }
                    iParseAudioDuringFF = aParameters[paramind].value.bool_value;
                }
                else if (pv_mime_strcmp(compstr, _STRLIT_CHAR("parser/rew_noaudio")) >= 0)
                {
                    // Make sure its a bool value
                    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameters[paramind].key);
                    if (PVMI_KVPVALTYPE_BOOL != keyvaltype)
                    {
                        aRet_kvp = &aParameters[paramind];
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync Setting "
                                                      "ff_noaudio valtype error"));
                        return;
                    }
                    iParseAudioDuringREW = aParameters[paramind].value.bool_value;
                }
                else
                {
                    // Unknown key string
                    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
                    return;
                }
            }
            else
            {
                // Do not support more than 3 components right now
                aRet_kvp = &aParameters[paramind];
                PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
                return;
            }
        }
        else
        {
            // Unknown key string
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Unsupported key"));
            return;
        }
    }

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::setParametersSync() Out"));
}

PVMFCommandId PVMFMP4FFParserNode::setParametersAsync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp*& aRet_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(context);
    // not supported
    OSCL_LEAVE(OsclErrNotSupported);
    // to satisfy compiler, need to return
    return 0;
}

uint32 PVMFMP4FFParserNode::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);
    return 0;
}

PVMFStatus PVMFMP4FFParserNode::verifyParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::verifyParametersSync() In"));

    if (aParameters == NULL || num_elements < 1)
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::verifyParametersSync() Passed in parameter invalid"));
        return PVMFErrArgument;
    }

    // Go through each parameter and verify
    for (int32 paramind = 0; paramind < num_elements; ++paramind)
    {
        // Retrieve the first component from the key string
        char* compstr = NULL;

        // Retrieve the second component from the key string
        pv_mime_string_extract_type(0, aParameters[paramind].key, compstr);

        // First check if it is key string for this node
        if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("fileio")) >= 0))
        {
            iBaseKey = FILE_IO;
            pv_mime_string_extract_type(1, aParameters[paramind].key, compstr);
            int32 i;
            for (i = 0; i < (int32)MP4ParserNodeConfig_Num_FileIO_Keys; i++)
            {
                if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_FileIO_Keys[i].iString)) >= 0)
                {
                    break;
                }
            }

            if ((int32)MP4ParserNodeConfig_Num_FileIO_Keys == i)
            {
                return PVMFErrArgument;
            }

            // Verify the passed-in player setting
            PVMFStatus retval = VerifyAndSetConfigParameter(i, aParameters[paramind], false);
            if (retval != PVMFSuccess)
            {
                PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                return retval;
            }
        }
        // First check if it is key string for this node
        else if ((pv_mime_strcmp(compstr, _STRLIT_CHAR("x-pvmf")) >= 0))
        {
            pv_mime_string_extract_type(1, aParameters[paramind].key, compstr);
            if (pv_mime_strcmp(compstr, _STRLIT_CHAR("net")) >= 0)
            {
                iBaseKey = NET;
                int32 i;
                pv_mime_string_extract_type(2, aParameters[paramind].key, compstr);
                for (i = 0; i < (int32)MP4ParserNodeConfig_Num_Net_Keys; i++)
                {
                    if (pv_mime_strcmp(compstr, (char*)(MP4ParserNodeConfig_Net_Keys[i].iString)) >= 0)
                    {
                        break;
                    }
                }

                if ((int32)MP4ParserNodeConfig_Num_Net_Keys == i)
                {
                    return PVMFErrArgument;
                }

                // Verify the passed-in player setting
                PVMFStatus retval = VerifyAndSetConfigParameter(i, aParameters[paramind], false);
                if (retval != PVMFSuccess)
                {
                    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoCapConfigVerifyParameters() Verifying parameter %d failed", paramind));
                    return retval;
                }
            }
            else
            {
                // Unknown key string
                return PVMFErrArgument;
            }
        }
        else
        {
            // Unknown key string
            return PVMFErrArgument;
        }

    }

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoCapConfigVerifyParameters() Out"));
    return PVMFSuccess;
}

PVMFStatus PVMFMP4FFParserNode::GetConfigParameter(PvmiKvp*& aParameters,
        int& aNumParamElements,
        int32 aIndex, PvmiKvpAttr reqattr)
{
    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() In"));

    aNumParamElements = 0;

    // Allocate memory for the KVP
    aParameters = (PvmiKvp*)oscl_malloc(sizeof(PvmiKvp));
    if (aParameters == NULL)
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() Memory allocation for KVP failed"));
        return PVMFErrNoMemory;
    }
    oscl_memset(aParameters, 0, sizeof(PvmiKvp));

    // Allocate memory for the key string in KVP
    PvmiKeyType memblock = (PvmiKeyType)oscl_malloc(MP4CONFIG_KEYSTRING_SIZE * sizeof(char));
    if (memblock == NULL)
    {
        oscl_free(aParameters);
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() Memory allocation for key string failed"));
        return PVMFErrNoMemory;
    }
    oscl_strset(memblock, 0, MP4CONFIG_KEYSTRING_SIZE*sizeof(char));

    // Assign the key string buffer to KVP
    aParameters[0].key = memblock;

    // Copy the key string
    if (iBaseKey == FILE_IO)
    {
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR("fileio/"), 7);
        oscl_strncat(aParameters[0].key, MP4ParserNodeConfig_FileIO_Keys[aIndex].iString,
                     oscl_strlen(MP4ParserNodeConfig_FileIO_Keys[aIndex].iString));
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
        switch (MP4ParserNodeConfig_FileIO_Keys[aIndex].iValueType)
        {
            case PVMI_KVPVALTYPE_RANGE_INT32:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
                break;

            case PVMI_KVPVALTYPE_KSV:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
                break;

            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_WCHARPTR_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_WCHARPTR_STRING));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                break;

            case PVMI_KVPVALTYPE_UINT32:
            default:
                if (reqattr == PVMI_KVPATTR_CAP)
                {
                    oscl_strncat(aParameters[0].key,
                                 _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING),
                                 oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
                }
                else
                {
                    oscl_strncat(aParameters[0].key,
                                 _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING),
                                 oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                }
                break;
        }
        aParameters[0].key[MP4CONFIG_KEYSTRING_SIZE-1] = 0;

        // Copy the requested info
        switch (aIndex)
        {
            case PV_CACHE_SIZE:
                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    aParameters[0].value.uint32_value = iCacheSize;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    // Return default
                    aParameters[0].value.uint32_value = DEFAULT_CAHCE_SIZE;
                }
                else
                {
                    // Return capability
                    range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                    if (rui32 == NULL)
                    {
                        oscl_free(aParameters[0].key);
                        oscl_free(aParameters);
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() "
                                                      "Memory allocation for range uint32 failed"));
                        return PVMFErrNoMemory;
                    }
                    rui32->min = MIN_CACHE_SIZE;
                    rui32->max = MAX_CACHE_SIZE;
                    aParameters[0].value.key_specific_value = (void*)rui32;
                }
                break;
            case ASYNC_READ_BUFFER_SIZE:
                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    // Return current value
                    aParameters[0].value.uint32_value = iAsyncReadBuffSize;;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    // Return default
                    aParameters[0].value.uint32_value = DEFAULT_ASYNC_READ_BUFFER_SIZE;
                }
                else
                {
                    // Return capability
                    range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                    if (rui32 == NULL)
                    {
                        oscl_free(aParameters[0].key);
                        oscl_free(aParameters);
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() "
                                                      "Memory allocation for range uint32 failed"));
                        return PVMFErrNoMemory;
                    }
                    rui32->min = MIN_ASYNC_READ_BUFFER_SIZE;
                    rui32->max = MAX_ASYNC_READ_BUFFER_SIZE;
                    aParameters[0].value.key_specific_value = (void*)rui32;
                }
                break;
            case BASEKEY_PVLOGGER_ENABLE:

                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    aParameters[0].value.bool_value = iPVLoggerEnableFlag;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    aParameters[0].value.bool_value = false;
                }
                else
                {
                    // Return capability - no concept of capability for keep alive interval
                    // do nothing
                }
                break;

            case PVLOGGER_STATS_ENABLE:
                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    aParameters[0].value.bool_value = iPVLoggerStateEnableFlag;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    aParameters[0].value.bool_value = false;
                }
                else
                {
                    // Return capability - no concept of capability for keep alive interval
                    // do nothing
                }
                break;
            case NATIVE_ACCESS_MODE:
                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    // Return current value
                    aParameters[0].value.uint32_value = iNativeAccessMode;;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    // Return default
                    aParameters[0].value.uint32_value = DEFAULT_NATIVE_ACCESS_MODE;
                }
                else
                {
                    // Return capability
                }
                break;



            default:
                // Invalid index
                oscl_free(aParameters[0].key);
                oscl_free(aParameters);
                PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoGetPlayerParameter() Invalid index to player parameter"));
                return PVMFErrArgument;
        }
    }
    else if (iBaseKey == NET)
    {
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR("x-pvmf/net/"), 11);
        oscl_strncat(aParameters[0].key, MP4ParserNodeConfig_Net_Keys[aIndex].iString,
                     oscl_strlen(MP4ParserNodeConfig_Net_Keys[aIndex].iString));
        oscl_strncat(aParameters[0].key, _STRLIT_CHAR(";type=value;valtype="), 20);
        switch (MP4ParserNodeConfig_Net_Keys[aIndex].iValueType)
        {
            case PVMI_KVPVALTYPE_RANGE_INT32:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_INT32_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_RANGE_INT32_STRING));
                break;

            case PVMI_KVPVALTYPE_KSV:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_KSV_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_KSV_STRING));
                break;

            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_CHARPTR_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_WCHARPTR_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_WCHARPTR_STRING));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                oscl_strncat(aParameters[0].key,
                             _STRLIT_CHAR(PVMI_KVPVALTYPE_BOOL_STRING),
                             oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING));
                break;

            case PVMI_KVPVALTYPE_UINT32:
            default:
                if (reqattr == PVMI_KVPATTR_CAP)
                {
                    oscl_strncat(aParameters[0].key,
                                 _STRLIT_CHAR(PVMI_KVPVALTYPE_RANGE_UINT32_STRING),
                                 oscl_strlen(PVMI_KVPVALTYPE_RANGE_UINT32_STRING));
                }
                else
                {
                    oscl_strncat(aParameters[0].key,
                                 _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING),
                                 oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING));
                }
                break;
        }
        aParameters[0].key[MP4CONFIG_KEYSTRING_SIZE-1] = 0;

        // Copy the requested info
        switch (aIndex)
        {
            case DELAY:
                if (reqattr == PVMI_KVPATTR_CUR)
                {
                    aParameters[0].value.uint32_value = iJitterBufferDurationInMs;
                }
                else if (reqattr == PVMI_KVPATTR_DEF)
                {
                    // Return default
                    aParameters[0].value.uint32_value = PVMF_MP4FFPARSER_NODE_PSEUDO_STREAMING_BUFFER_DURATION_IN_MS;
                }
                else
                {
                    // Return capability
                    range_uint32* rui32 = (range_uint32*)oscl_malloc(sizeof(range_uint32));
                    if (rui32 == NULL)
                    {
                        oscl_free(aParameters[0].key);
                        oscl_free(aParameters);
                        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::GetConfigParameter() "
                                                      "Memory allocation for range uint32 failed"));
                        return PVMFErrNoMemory;
                    }
                    rui32->min = MIN_JITTER_BUFFER_DURATION_IN_MS;
                    rui32->max = MAX_JITTER_BUFFER_DURATION_IN_MS;
                    aParameters[0].value.key_specific_value = (void*)rui32;
                }
                break;

            default:
                // Invalid index
                oscl_free(aParameters[0].key);
                oscl_free(aParameters);
                PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoGetPlayerParameter() Invalid index to player parameter"));
                return PVMFErrArgument;
        }
    }

    aNumParamElements = 1;
    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVPlayerEngine::DoGetPlayerParameter() Out"));
    return PVMFSuccess;

}


PVMFStatus PVMFMP4FFParserNode::VerifyAndSetConfigParameter(int index, PvmiKvp& aParameter, bool set)
{
    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::VerifyAndSetConfigParameter() In"));

    // Determine the valtype
    PvmiKvpValueType keyvaltype = GetValTypeFromKeyString(aParameter.key);
    if (keyvaltype == PVMI_KVPVALTYPE_UNKNOWN)
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::VerifyAndSetConfigParameter() "
                                      "Valtype in key string unknown"));
        return PVMFErrArgument;
    }

    if (iBaseKey == FILE_IO)
    {	// Verify the valtype
        if (keyvaltype != MP4ParserNodeConfig_FileIO_Keys[index].iValueType)
        {
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::VerifyAndSetConfigParameter() "
                                          "Valtype does not match for key"));
            return PVMFErrArgument;
        }

        switch (index)
        {
            case PV_CACHE_SIZE:
            {
                // Validate
                if (set)
                {
                    // save value locally
                    iCacheSize = aParameter.value.uint32_value;
                }
            }
            break;
            case ASYNC_READ_BUFFER_SIZE:
            {
                if (set)
                {
                    // retrieve and update
                    iAsyncReadBuffSize = aParameter.value.uint32_value;
                }
            }
            break;
            case BASEKEY_PVLOGGER_ENABLE:
            {
                if (set)
                {
                    // retrieve and update
                    iPVLoggerEnableFlag = aParameter.value.bool_value;
                }
            }
            break;
            case PVLOGGER_STATS_ENABLE:
            {
                if (set)
                {
                    // user agent update
                    iPVLoggerStateEnableFlag = aParameter.value.bool_value;
                }
            }
            break;

            case NATIVE_ACCESS_MODE:
            {
                if (set)
                {
                    iNativeAccessMode = aParameter.value.uint32_value;
                }
            }
            break;

            case FILE_HANDLE:
            {
                if (set)
                {
                }
            }
            break;

            default:
                OSCL_ASSERT(0);
        }
    }
    else if (iBaseKey == NET)
    {
        // Verify the valtype
        if (keyvaltype != MP4ParserNodeConfig_FileIO_Keys[index].iValueType)
        {
            PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::VerifyAndSetConfigParameter() "
                                          "Valtype does not match for key"));
            return PVMFErrArgument;
        }

        switch (index)
        {
            case DELAY:
            {
                // Validate
                if (set)
                {
                    // save value locally
                    iJitterBufferDurationInMs = aParameter.value.uint32_value;
                }
            }
            break;

            default:
                return PVMFErrNotSupported;
        }
    }

    PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::VerifyAndSetPlayerParameter() Out"));
    return PVMFSuccess;
}
