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
#ifndef MP4FFFREC_H_INCLUDED
#define MP4FFFREC_H_INCLUDED

#ifndef OSCL_FILE_SERVER_H_INCLUDED
#include "oscl_file_server.h"
#endif

#ifndef OSCL_FILE_HANDLE_H_INCLUDED
#include "oscl_file_handle.h"
#endif

/* CPM Related Header Files */
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif

class MP4FileRecognizer
{

    public:

        OSCL_IMPORT_REF static bool IsMP4File(OSCL_wString& filename,
                                              Oscl_FileServer* fileServSession = NULL);

        OSCL_IMPORT_REF static bool IsMP4File(Oscl_File* filePtr);

        OSCL_IMPORT_REF static bool IsMP4File(PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                                              Oscl_FileServer* aFileServSession = NULL,
                                              OsclFileHandle* aHandle = NULL);

};

#endif
