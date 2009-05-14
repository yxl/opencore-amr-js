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

#ifndef PVMI_FILEIO_KVP_H_INCLUDED
#define PVMI_FILEIO_KVP_H_INCLUDED

/*
 * File type modifiers.  These may be used with any of the KVPs
 */
#define PVMI_FILEIO_FILE_TYPE_SOURCE "filetype=source"
#define PVMI_FILEIO_FILE_TYPE_LICENSE_STORE "filetype=license-store"

/*
 * Define the PV read/write cache size.  Zero disables the PV cache
 */
#define PVMI_FILEIO_PV_CACHE_SIZE "fileio/pv-cache-size"
#define PVMI_FILEIO_PV_CACHE_SIZE_VALUE "fileio/pv-cache-sizeval;type=uint32"

/*
 * Define the PV Async Read buffer size.  Zero disables the async read.
 */
#define PVMI_FILEIO_ASYNC_READ_BUFFER_SIZE "fileio/async-read-buffer-size"
#define PVMI_FILEIO_ASYNC_READ_BUFFER_SIZE_VALUE "fileio/async-read-buffer-size;valtype=uint32"

/*
 * Set PV Logger enable option for this file.  This enables complete logging.
 */
#define PVMI_FILEIO_PVLOGGER_ENABLE "fileio/logger-enable"
#define PVMI_FILEIO_PVLOGGER_ENABLE_VALUE "fileio/logger-enable;valtype=bool"

/*
 * Set PV Logger statistics enable option for this file.  This enables summary stats logging only.
 */
#define PVMI_FILEIO_PVLOGGER_STATS_ENABLE "fileio/logger-stats-enable"
#define PVMI_FILEIO_PVLOGGER_STATS_ENABLE_VALUE "fileio/logger-stats-enable;valtype=bool"

/*
 * Select native file access mode, for systems that have more than one type of file
 * access.  Values for Symbian, RFile=0, RFileBuf=1.
 */
#define PVMI_FILEIO_NATIVE_ACCESS_MODE "fileio/native-access-mode"
#define PVMI_FILEIO_NATIVE_ACCESS_MODE_VALUE "fileio/native-access-mode;valtype=uint32"

/*
 * Select file handle access mode.  The key-specific value is an OsclFileHandle*
 */
#define PVMI_FILEIO_FILE_HANDLE "fileio/file-handle"
#define PVMI_FILEIO_FILE_HANDLE_VALUE "fileio/file-handle;valtype=ksv"



#endif // OSCL_FILEIO_CONFIG_H_INCLUDED




