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
#include "oscl_configfile_list.h"
#include "oscl_string.h"
#include "oscl_file_io.h"
#include "oscl_file_find.h"
#include "oscl_file_types.h"
#include "pvlogger.h"
#include "oscl_uuid.h"

#define CONFIG_FILE_EXTENSION "*.cfg"
#define OSCL_NUMBER_OF_CONFIG_FILES	16

OSCL_EXPORT_REF	OsclLibStatus OsclLibraryUtil::SetupLibraryEnv(const char* aEnvName, const char* aEnvValue)
{
    // currently only MSWIN is supported;
    // Linux is not implemented since its library search path needs to be set at compilation time
    OsclLibStatus aStatus = OsclLibSuccess;

    OSCL_UNUSED_ARG(aEnvName);
    OSCL_UNUSED_ARG(aEnvValue);
    aStatus = OsclLibNotSupported;
    return aStatus;
}


OSCL_EXPORT_REF OsclConfigFileList::OsclConfigFileList()
{
    ipLogger = PVLogger::GetLoggerObject("oscllib");
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.oscllib.osclconfigfilelist");
    iCfgFileNum = 0;
#endif
    int32 err = 0;
    OSCL_TRY(err,
             iCfgList.reserve(OSCL_NUMBER_OF_CONFIG_FILES);
            );
    if (err)
    {
        iCfgList.clear();
        OSCL_LEAVE(err);
    }
}

OSCL_EXPORT_REF OsclConfigFileList::~OsclConfigFileList()
{
    ipLogger = NULL;
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = NULL;
#endif
    iCfgList.clear();
}

// This method finds all files with extention .cfg in given directory
// using oscl_file_find and stores in a vector in iCfgList
// each cfg in iCfgList stored with whole pathname
// @Return: OsclLibSuccess if any config file found, OsclLibNotFound if not any, OsclLibFail otherwise

OSCL_EXPORT_REF OsclLibStatus OsclConfigFileList::Populate(const OSCL_String& aConfigFileDir, OsclConfigFileList::SortType aSortType)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG,
                    (0, "OsclConfigFileList::Populate '%s' IN", aConfigFileDir.get_cstr()));
#if OSCL_LIBRARY_PERF_LOGGING
    TICK starttime;
    SET_TICK(starttime);
    iCfgFileNum = 0;
#endif
    Oscl_FileFind aCfgFind;
    OsclLibStatus status = OsclLibSuccess;
    char* strbuf = (char*)OSCL_MALLOC(OSCL_IO_FILENAME_MAXLEN * sizeof(char));
    if (NULL == strbuf)
        return OsclLibOutOfMemory;
    if (NULL == aCfgFind.FindFirst(aConfigFileDir.get_str(), CONFIG_FILE_EXTENSION, strbuf, OSCL_IO_FILENAME_MAXLEN))
    {
        OSCL_FREE(strbuf);
        strbuf = NULL;
        if (Oscl_FileFind::E_NO_MATCH == aCfgFind.GetLastError())
        {
            status = OsclLibNotFound;
        }
        else
        {
            iCfgList.clear();
            return OsclLibFail;
        }
    }
    else
    {
        //oscl_file_find return file with pathname.
        iCfgList.push_front(strbuf);
#if OSCL_LIBRARY_PERF_LOGGING
        iCfgFileNum++;
#endif
        // found first config file, continue until Oscl_FileFind::E_NO_MATCH
        while (aCfgFind.FindNext(strbuf, OSCL_IO_FILENAME_MAXLEN) && Oscl_FileFind::E_NO_MATCH != aCfgFind.GetLastError())
        {
            iCfgList.push_back(strbuf);
#if OSCL_LIBRARY_PERF_LOGGING
            iCfgFileNum++;
#endif
        }
#if OSCL_LIBRARY_PERF_LOGGING
        uint32 difftime;
        DIFF_TICK(starttime, difftime);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                        (0, "OsclConfigFileList::Populate - Searching path %s ...", aConfigFileDir.get_cstr()));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                        (0, "                                   Time taken = %d ticks", difftime));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                        (0, "                                   Number of config files found = %d", iCfgFileNum));
#endif
    }
    OSCL_FREE(strbuf);
    // error if there's none config file found and loaded in the vector,
    if (iCfgList.size() <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                        (0, "OsclConfigFileList::Populate, Didn't find any config file in %s", aConfigFileDir.get_str()));
        status = OsclLibNotFound;
    }
    // Sort the list if requested
    else if (aSortType != OsclConfigFileList::ENoSort)
    {
        Sort();
    }

    return status;
}

OSCL_EXPORT_REF uint32 OsclConfigFileList::Size()
{
    return iCfgList.size();
}

OSCL_EXPORT_REF const OSCL_String& OsclConfigFileList::GetConfigfileAt(uint32 n)
{
    return iCfgList[n];
}

void OsclConfigFileList::Sort()
{
    uint32 numConfigs = Size();
    uint32 ii = 0;
    bool swapped = false;

    // Bubblesort iCfgList - As long as there aren't very many config files
    // in the list, efficiency shouldn't be an issue. If the list gets to be
    // too large (100+) then a new sort algorithm should be used.
    do
    {
        swapped = false;
        for (ii = 0; ii < numConfigs - 1; ii++)
        {
            if (oscl_strncmp(iCfgList[ii].get_cstr(),
                             iCfgList[ii + 1].get_cstr(),
                             oscl_strlen(iCfgList[ii].get_cstr())) > 0)
            {
                OSCL_StackString<OSCL_IO_FILENAME_MAXLEN> temp = iCfgList[ii];
                iCfgList[ii] = iCfgList[ii + 1];
                iCfgList[ii + 1] = temp;
                swapped = true;
            }
        }
    }
    while (swapped);
}

