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
/*! \file oscl_file_server.cpp
    \brief This file contains file io APIs
*/

#include "oscl_file_server.h"
#include "oscl_file_types.h"
#include "oscl_stdstring.h"
#include "oscl_utf8conv.h"


OSCL_EXPORT_REF Oscl_FileServer::Oscl_FileServer()
{
}

OSCL_EXPORT_REF Oscl_FileServer::~Oscl_FileServer()
{
}

OSCL_EXPORT_REF int32 Oscl_FileServer::Connect()
{
    //default-- assume there is nothing needed.
    return 0;
}

OSCL_EXPORT_REF int32 Oscl_FileServer::Close()
{
    //default-- assume there is nothing needed.
    return 0;
}

OSCL_EXPORT_REF int32 Oscl_FileServer::Oscl_DeleteFile(const oscl_wchar* filename)
{
    {
        //Convert to UTF8
        char convfilename[OSCL_IO_FILENAME_MAXLEN];
        if (0 == oscl_UnicodeToUTF8(filename, oscl_strlen(filename), convfilename, OSCL_IO_FILENAME_MAXLEN))
            return (-1);

        return unlink(convfilename);

    }
}

OSCL_EXPORT_REF int32 Oscl_FileServer::Oscl_DeleteFile(const char* filename)
{
    {
        return unlink(filename);
    }
}



