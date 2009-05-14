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
#include "osclconfig_lib.h"
#include "oscl_shared_library.h"
#include "oscl_string.h"
#include "pvlogger.h"
#include "oscl_utf8conv.h"
#include "oscl_uuid.h"
#include "oscl_file_types.h"

#define BUFFER_SIZE 256

typedef OsclSharedLibraryInterface*(*PVGetInterface_t)();
typedef void (*PVReleaseInterface_t)(OsclSharedLibraryInterface*);

OSCL_EXPORT_REF OsclSharedLibrary::OsclSharedLibrary()
{
    ipLogger = PVLogger::GetLoggerObject("oscllib");
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.oscllib.osclsharedlibrary");
    iLibCount = 0;
    iLibLoadTime = 0;
#endif
    pSharedLibInterface = NULL;
    iRefCount = 0;
    ipHandle = NULL;

}

OSCL_EXPORT_REF OsclSharedLibrary::OsclSharedLibrary(const OSCL_String& aPath)
{
    ipLogger = PVLogger::GetLoggerObject("oscllib");
#if OSCL_LIBRARY_PERF_LOGGING
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.oscllib.osclsharedlibrary");
    iLibCount = 0;
    iLibLoadTime = 0;
#endif
    pSharedLibInterface = NULL;

    iRefCount = 0;
    ipHandle = NULL;
    iLibPath = aPath;
}

OSCL_EXPORT_REF OsclSharedLibrary::~OsclSharedLibrary()
{
#if OSCL_LIBRARY_PERF_LOGGING
    PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                    (0, "OsclSharedLibrary - Loading %d libraries took %d ticks ...", iLibCount, iLibLoadTime));
    iDiagnosticsLogger = NULL;
#endif
    ipLogger = NULL;

    if (NULL != ipHandle)
    {
        Close();
    }
}

OSCL_EXPORT_REF OsclLibStatus OsclSharedLibrary::LoadLib(const OSCL_String& aPath)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG, (0, "OsclSharedLibrary::LoadLib %s IN", aPath.get_cstr()));
#if OSCL_LIBRARY_PERF_LOGGING
    iLibCount++;
    TICK starttime;
    SET_TICK(starttime);
#endif
    iLibPath = aPath;
    // First look for debug version of the library ending with _debug
    int32 dotpos = aPath.get_size() - 1;
    while (dotpos >= 0)
    {
        if (aPath[dotpos] == '.')
            break;
        dotpos--;
    }
    if (dotpos >= 0 && dotpos != (int32)(aPath.get_size() - 1))
    {   // extension exists
#if OSCL_LIB_READ_DEBUG_LIBS
        OSCL_HeapString<OsclMemAllocator> path1(aPath.get_cstr(), dotpos);
        OSCL_HeapString<OsclMemAllocator> debugPath(path1);
        debugPath += "_debug";
        debugPath += (aPath.get_cstr() + dotpos);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG, (0, "OsclSharedLibrary::LoadLib looking for Debug lib %s", debugPath.get_cstr()));
        // Open the library
        if (OsclLibSuccess == loadlibrary(debugPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", debugPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;  // debug dll loaded successfully
        }
        // _debug load failed, try original
#endif
        if (OsclLibSuccess ==  loadlibrary(aPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", aPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;
        }
        return OsclLibFail;
    }
    else if (dotpos == (int32)(aPath.get_size() - 1))
    {   // libname ended with "." no extension
        OSCL_HeapString<OsclMemAllocator> path1(aPath.get_cstr(), dotpos);
        OSCL_HeapString<OsclMemAllocator> debugPath(path1);
#if OSCL_LIB_READ_DEBUG_LIBS
        debugPath += "_debug.";
        debugPath += PV_RUNTIME_LIB_FILENAME_EXTENSION;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG, (0, "OsclSharedLibrary::LoadLib looking for Debug lib %s", debugPath.get_cstr()));
        // Open the library
        if (OsclLibSuccess == loadlibrary(debugPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", debugPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;  // debug dll loaded successfully
        }
        // _debug load failed, try original
        debugPath = path1;
#endif
        debugPath += PV_RUNTIME_LIB_FILENAME_EXTENSION;
        if (OsclLibSuccess ==  loadlibrary(debugPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", debugPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;
        }
        return OsclLibFail;
    }
    else    // libname without extension
    {
        OSCL_HeapString<OsclMemAllocator> path1(aPath.get_cstr());
        OSCL_HeapString<OsclMemAllocator> debugPath(path1);
#if OSCL_LIB_READ_DEBUG_LIBS
        debugPath += "_debug.";
        debugPath += PV_RUNTIME_LIB_FILENAME_EXTENSION;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG, (0, "OsclSharedLibrary::LoadLib looking for Debug lib %s", debugPath.get_cstr()));
        // Open the library
        if (OsclLibSuccess == loadlibrary(debugPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", debugPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;  // debug dll loaded successfully
        }
        // _debug load failed, try original
        debugPath = path1;
#endif
        debugPath += ".";
        debugPath += PV_RUNTIME_LIB_FILENAME_EXTENSION;
        if (OsclLibSuccess ==  loadlibrary(debugPath))
        {
#if OSCL_LIBRARY_PERF_LOGGING
            DIFF_TICK(starttime, delta);
            iLibLoadTime += delta;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iDiagnosticsLogger, PVLOGMSG_INFO,
                            (0, "OsclSharedLibrary - Loading library %s took %d ticks ...", debugPath.get_cstr(), delta));
#endif
            return OsclLibSuccess;
        }
        return OsclLibFail;
    }

}

OSCL_EXPORT_REF OsclLibStatus OsclSharedLibrary::LoadLib()
{
    return LoadLib(iLibPath);
}

OSCL_EXPORT_REF void OsclSharedLibrary::SetLibPath(const OSCL_String& aPath)
{
    iLibPath = aPath;
}

OSCL_EXPORT_REF OsclLibStatus OsclSharedLibrary::QueryInterface(const OsclUuid& aInterfaceId,
        OsclAny*& aInterfacePtr)
{
    aInterfacePtr = NULL;
    // Look up the PVGetInterface method
    if (NULL == ipHandle)
    {
        return OsclLibNotLoaded;
    }
    PVGetInterface_t getInterface =
        (PVGetInterface_t)dlsym(ipHandle, "PVGetInterface");
    // dlsym() returns NULL if there were any issues getting the
    // address of the symbol
    if (NULL == getInterface)
    {
        // check for errors
        const char* pErr = dlerror();
        if (NULL == pErr)
        {
            // No error reported, but no symbol was found
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                            (0, "OsclLib::QueryInterface: Could not access PVGetInterface "
                             "symbol in library but no error reported"));
        }
        else
        {
            // Error reported
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                            (0, "OsclLib::QueryInterface: Could not access PVGetInterface "
                             "symbol in library: %s", pErr));
        }
        return OsclLibFail;
    }

    // Get the OsclSharedLibraryInterface if needed.  There can be multiple calls
    // to query interface so check whether the interface has already been fetched.
    if (!pSharedLibInterface)
    {
        pSharedLibInterface = (OsclSharedLibraryInterface*)getInterface();
    }
    if (NULL == pSharedLibInterface)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                        (0, "OsclSharedLibrary::QueryInterface: Could not access the "
                         "library pointer function"));
        return OsclLibFail;
    }
    // Lookup the interface ID
    aInterfacePtr = pSharedLibInterface->SharedLibraryLookup(aInterfaceId);
    if (NULL == aInterfacePtr)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                        (0, "OsclLib::QueryInterface: NO library interface found for aInterfaceId."));
        return OsclLibFail;
    }
    return OsclLibSuccess;
}

OSCL_EXPORT_REF OsclLibStatus OsclSharedLibrary::Close()
{
    if (iRefCount > 0)
    {
        return OsclLibFail;
    }
    if (pSharedLibInterface)
    {
        PVReleaseInterface_t releaseInterface =
            (PVReleaseInterface_t)dlsym(ipHandle, "PVReleaseInterface");
        // dlsym() returns NULL if there were any issues getting the
        // address of the symbol
        if (NULL == releaseInterface)
        {
            // check for errors
            const char* pErr = dlerror();
            if (NULL == pErr)
            {
                // No error reported, but no symbol was found
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                                (0, "OsclLib::Close: Could not access PVReleaseInterface "
                                 "symbol in library but no error reported"));
            }
            else
            {
                // Error reported
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_INFO,
                                (0, "OsclLib::Close: Could not access PVReleaseInterface "
                                 "symbol in library: %s", pErr));
            }
            return OsclLibFail;
        }
        releaseInterface(pSharedLibInterface);
        pSharedLibInterface = NULL;
    }

    if (ipHandle)
    {
        if (0 != dlclose(ipHandle))
        {
            // dlclose() returns non-zero value if close failed, check for errors
            const char* pErr = dlerror();
            if (NULL != pErr)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                                (0, "OsclSharedLibrary::Close: Error closing library: %s", pErr));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                                (0, "OsclSharedLibrary::Close: Error closing library, no error reported"
                                 ""));
            }
            return OsclLibFail;
        }
        ipHandle = NULL;
    }
    return OsclLibSuccess;
}

OSCL_EXPORT_REF void OsclSharedLibrary::AddRef()
{
    ++iRefCount;
}

OSCL_EXPORT_REF void OsclSharedLibrary::RemoveRef()
{
    --iRefCount;
}

//
//OsclSharedLibraryList
//
#include "oscl_configfile_list.h"
#include "oscl_library_list.h"

OSCL_EXPORT_REF OsclSharedLibraryList::OsclSharedLibraryList()
{
    iLogger = PVLogger::GetLoggerObject("oscllib");
}

OSCL_EXPORT_REF OsclSharedLibraryList::~OsclSharedLibraryList()
{
    CloseAll();
}
/*
** Give a config file path, search all config files for libraries that
** support the given interface ID.
*/
OSCL_EXPORT_REF void OsclSharedLibraryList::Populate(const OSCL_String& aPath, const OsclUuid& aInterfaceId)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclSharedLibraryList::Populate '%s' IN", aPath.get_cstr()));

    iInterfaceId = aInterfaceId;
    OsclConfigFileList configFileList;
    configFileList.Populate(aPath);
    for (uint32 i = 0;i < configFileList.Size();i++)
    {
        OsclLibraryList libList;
        libList.Populate(aInterfaceId, configFileList.GetConfigfileAt(i));
        for (uint32 j = 0;j < libList.Size();j++)
        {
            OsclSharedLibrary* sharedLib = OSCL_NEW(OsclSharedLibrary, (libList.GetLibraryPathAt(j)));
            iList.push_back(sharedLib);
        }
    }
}

/*
** Query interface for the given library.
*/
OSCL_EXPORT_REF OsclLibStatus OsclSharedLibraryList::QueryInterfaceAt(uint32 aIndex, OsclAny*& aInterfacePtr)
{
    aInterfacePtr = NULL;
    OsclLibStatus status = OsclLibFail;

    if (aIndex < iList.size())
    {
        status = iList[aIndex]->QueryInterface(iInterfaceId, aInterfacePtr);
        //Load lib if needed & repeat the query.
        if (status == OsclLibNotLoaded)
        {
            status = iList[aIndex]->LoadLib();
            if (status == OsclLibSuccess)
                status = iList[aIndex]->QueryInterface(iInterfaceId, aInterfacePtr);
        }
    }
    return status;
}

/*
** Close all open libraries
*/
OSCL_EXPORT_REF void OsclSharedLibraryList::CloseAll()
{
    while (!iList.empty())
    {
        iList.front()->Close();
        OSCL_DELETE(iList.front());
        iList.erase(&iList.front());
    }
}

OsclLibStatus OsclSharedLibrary::loadlibrary(const OSCL_String& alib)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_DEBUG, (0, "OsclSharedLibrary::loadlibrary %s IN", alib.get_cstr()));
    // Clear any errors
    dlerror();
    // Open the library
    void* library = dlopen(alib.get_cstr(), RTLD_NOW);
    // dlopen() returns NULL if there were any issues opening the library
    if (NULL == library)
    {
        // check for errors
        const char* pErr = dlerror();
        if (NULL == pErr)
        {
            // No error reported, but no handle to the library
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                            (0, "OsclLib::loadlibrary: Error opening "
                             "library (%s) but no error reported",
                             alib.get_cstr(), pErr));
        }
        else
        {
            // Error reported
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, ipLogger, PVLOGMSG_WARNING,
                            (0, "OsclLib::loadlibrary: Error opening "
                             "library (%s): %s", alib.get_cstr(), pErr));
        }
        return OsclLibFail;
    }
    ipHandle = library;
    return OsclLibSuccess;

}

