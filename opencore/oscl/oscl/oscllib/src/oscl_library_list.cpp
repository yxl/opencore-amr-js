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
#include "oscl_library_list.h"
#include "oscl_string.h"
#include "oscl_file_io.h"
#include "oscl_file_types.h"
#include "pvlogger.h"
#include "oscl_uuid.h"

#define HASH '#'
#define NEWLINE '\n'
#define OPEN_PAREN '('
#define CLOSE_PAREN ')'
#define QUOTE '"'
#define COMMA ','
#define OSCL_NUMBER_OF_SHARED_LIBS	16
#define BUFFER_SIZE 256


OSCL_EXPORT_REF OsclLibraryList::OsclLibraryList()
{
    ipLogger = PVLogger::GetLoggerObject("oscllib");
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.oscllib.oscllibrarylist");
    iLinesRead = 0;
    iLibHit = 0;
#endif
    int32 err = 0;
    OSCL_TRY(err,
             iLibList.reserve(OSCL_NUMBER_OF_SHARED_LIBS);
            );
    if (err)
    {
        iLibList.clear();
        OSCL_LEAVE(err);
    }

}

OSCL_EXPORT_REF OsclLibraryList::~OsclLibraryList()
{
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = NULL;
#endif
    ipLogger = NULL;
    iLibList.clear();
}

//  This method actually parses through a dll config file, retrieving the paths
// of dlls which implement a specific OsclUuid
OSCL_EXPORT_REF OsclLibStatus OsclLibraryList::Populate(const OsclUuid& aInterfaceId, const OSCL_String& aConfigFile)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG,
                    (0, "OsclLibraryList::Populate '%s' IN", aConfigFile.get_cstr()));

    // Open config file
    Oscl_FileServer fileserver;
    if (0 != fileserver.Connect())
    {
        // Failed to connect to file server, return failure
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_ERR,
                        (0, "OsclLibraryList::Populate - Unable to connect to fileserver"));
        return OsclLibFail;
    }
    Oscl_File configFile;
    if (0 != configFile.Open(aConfigFile.get_cstr(), Oscl_File::MODE_READ, fileserver))
    {
        // Failed to open config file, return failure
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_ERR,
                        (0, "OsclLibraryList::Populate - Unable to open configFile %s", aConfigFile.get_cstr()));
        return OsclLibFail;
    }

#if OSCL_LIBRARY_PERF_LOGGING
    // Start collecting stats for current config file
    iLinesRead = 0;
    iLibHit = 0;
    TICK starttime;
    SET_TICK(starttime);
#endif

    // Read in a byte at a time
    uint8 buf[1];
    while (1 == configFile.Read(buf, 1, 1))
    {
        if (HASH == buf[0])
        {
#if OSCL_LIBRARY_PERF_LOGGING
            iLinesRead++;
#endif
            // Ignore comments - begin with '#'
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG,
                            (0, "OsclLibraryList::Populate - Found a comment, skipping"));
            // Advance to end of line
            while (1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
            {
                // ignore
            }
        }
        else if (OPEN_PAREN == buf[0])
        {
            // Parse UUID from line - begins with "("
            uint8 uuidBuf[BUFFER_SIZE];
            int i = 0;
            uuidBuf[i++] = buf[0];
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG,
                            (0, "OsclLibraryList::Populate - Found a (, reading uuid"));
            // Read a character at a time - stop if newline or eof is reached or buffer is filled
            while (i < BUFFER_SIZE && 1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
            {
                uuidBuf[i++] = buf[0];
                if (CLOSE_PAREN == buf[0])
                {
                    break;
                }
            }
            uuidBuf[i] = '\0';
            if (NEWLINE == buf[0])
            {
#if OSCL_LIBRARY_PERF_LOGGING
                iLinesRead++;
#endif
                // Reached the end of line but did not find the closing parentheses
                // Skip this malformed line
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_WARNING,
                                (0, "OsclLibraryList::Populate - incomplete uuid, skipping line"));
            }
            else if (BUFFER_SIZE == i && CLOSE_PAREN != buf[0])
            {
                // Buffer is filled but did not reach the end of UUID - skip this malformed line
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_WARNING,
                                (0, "OsclLibraryList::Populate - uuid too long, skipping line"));
                // Advance to end of line
                while (1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
                {
                    // ignore
                }
            }
            else
            {
                // Create an instance of OsclUuid
                OsclUuid tempUuidStr((char*)uuidBuf);
                if (tempUuidStr == aInterfaceId)
                {
                    // Parse path from line
                    bool commaFound = false;
                    bool quoteFound = false;
                    while (!(commaFound && quoteFound) && 1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
                    {
                        // Advance past ',' and '"'
                        if (COMMA == buf[0])
                        {
                            // If already found a comma, break and skip this malformed line
                            if (commaFound) break;
                            commaFound = true;
                        }
                        else if (QUOTE == buf[0])
                        {
                            // If already found a quote, break and skip this malformed line
                            if (quoteFound) break;
                            quoteFound = true;
                        }
                    }
                    if (!(commaFound && quoteFound) || NEWLINE == buf[0])
                    {
#if OSCL_LIBRARY_PERF_LOGGING
                        iLinesRead++;
#endif
                        // Did not find both ',' and '"' - Skip this malformed line
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_WARNING,
                                        (0, "OsclLibraryList::Populate - missing ' or \", skipping line"));
                    }
                    else
                    {
                        uint8 pathBuf[BUFFER_SIZE];
                        i = 0;
                        // Read a character at a time - stop if newline is reached, ending quote is reached, or buffer is filled
                        // - leave room for terminating null character in buffer
                        while (1 == configFile.Read(buf, 1, 1) && buf[0] != QUOTE && buf[0] != NEWLINE && i < (BUFFER_SIZE - 1))
                        {
                            pathBuf[i++] = buf[0];
                        }
                        if (NEWLINE == buf[0])
                        {
#if OSCL_LIBRARY_PERF_LOGGING
                            iLinesRead++;
#endif
                            // Reached the end of line but did not find the closing quote
                            // Skip this malformed line
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_WARNING,
                                            (0, "OsclLibraryList::Populate - incomplete path, skipping line"));
                        }
                        else if ((BUFFER_SIZE - 1) == i && QUOTE != buf[0])
                        {
                            // Buffer is filled but did not reach the end of path - skip this malformed line
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_WARNING,
                                            (0, "OsclLibraryList::Populate - path too long, skipping line"));
#if OSCL_LIBRARY_PERF_LOGGING
                            iLinesRead++;
#endif
                            // Advance to end of line
                            while (1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
                            {
                                // ignore
                            }
                        }
                        else
                        {
                            // Read in the path, end with terminating character
                            pathBuf[i] = NULL_TERM_CHAR;
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_INFO,
                                            (0, "OsclLibraryList::Populate - found a match, adding to list: %s",
                                             (char*)pathBuf));
                            // Add path to library list
                            iLibList.push_back((char*)pathBuf);
#if OSCL_LIBRARY_PERF_LOGGING
                            iLibHit++;
                            iLinesRead++;
#endif
                        }
                    }
                }
                else
                {
                    // The UUID from this line does not match aInterfaceId - Advance to end of line
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_INFO,
                                    (0, "OsclLibraryList::Populate - uuid not a match, skipping line"));
#if OSCL_LIBRARY_PERF_LOGGING
                    iLinesRead++;
#endif
                    while (1 == configFile.Read(buf, 1, 1) && buf[0] != NEWLINE)
                    {
                        // ignore
                    }
                }
            }
        }
        // else continue on to next byte
    };

#if OSCL_LIBRARY_PERF_LOGGING
    // End stats for current config file
    uint32 difftime;
    DIFF_TICK(starttime, difftime);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                    (0, "OsclLibraryList::Populate - Parsing configFile %s ...", aConfigFile.get_cstr()));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                    (0, "                                   Time taken = %d ticks", difftime));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                    (0, "                                   Lines read = %d", iLinesRead));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                    (0, "                                   Libraries found = %d", iLibHit));
#endif

    //If there's atleast one library path name loaded in the vector,
    //it means the parsing is successful for at least a particular OsclUuid

    if (iLibList.size() > 0)
    {
        return OsclLibSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                        (0, "OsclLibraryList::Populate, Didn't find any DLL for the OsclUuid"));
        return OsclLibFail;
    }
}

OSCL_EXPORT_REF uint32 OsclLibraryList::Size()
{
    return iLibList.size();
}

OSCL_EXPORT_REF const OSCL_String& OsclLibraryList::GetLibraryPathAt(uint32 n)
{
    return iLibList[n];
}

