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
/*! \file oscl_file_io.cpp
    \brief This file contains file io APIs
*/

#ifndef OSCL_FILE_DIR_UTILS_H_INCLUDED
#include "oscl_file_dir_utils.h"
#endif
#ifndef OSCL_FILE_TYPES_H_INCLUDED
#include "oscl_file_types.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef OSCL_STDSTRING_H_INCLUDED
#include "oscl_stdstring.h"
#endif
#ifndef OSCL_INT64_UTILS_H_INCLUDED
#include "oscl_int64_utils.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCLCONFIG_ERROR_H_INCLUDED
#include "osclconfig_error.h"
#endif
#ifndef OSCL_FILE_DIR_UTILS_H_INCLUDED
#include "oscl_file_dir_utils.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_getcwd(char *path, uint32 size)
{
    if (getcwd(path, size) != NULL)
        return OSCL_FILEMGMT_E_OK;
    return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_getcwd(oscl_wchar *path, uint32 size)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (oscl_getcwd(convpathname, size > OSCL_IO_FILENAME_MAXLEN ? OSCL_IO_FILENAME_MAXLEN : size) == OSCL_FILEMGMT_E_OK)
    {
        if (0 == oscl_UTF8ToUnicode(convpathname, oscl_strlen(convpathname), path, size) && oscl_strlen(convpathname) != 0)
        {
            return OSCL_FILEMGMT_E_PATH_TOO_LONG;
        }
        return OSCL_FILEMGMT_E_OK;
    }
    return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_stat(const oscl_wchar *path, OSCL_STAT_BUF *statbuf)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(path, oscl_strlen(path), convpathname, OSCL_IO_FILENAME_MAXLEN) && oscl_strlen(path) != 0)
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return  oscl_stat(convpathname, statbuf);


}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_stat(const char *path, OSCL_STAT_BUF *statbuf)
{
    oscl_memset(statbuf, 0, sizeof(OSCL_STAT_BUF));
    struct stat buf;
    if (stat(path, &buf) == 0)
    {
        if (buf.st_mode & S_IRUSR)
            statbuf->perms |= OSCL_FILEMGMT_PERMS_READ;
        if (buf.st_mode & S_IWUSR)
            statbuf->perms |= OSCL_FILEMGMT_PERMS_WRITE;
        if (buf.st_mode & S_IFDIR)
            statbuf->mode |= OSCL_FILEMGMT_MODE_DIR;
        return OSCL_FILEMGMT_E_OK;
    }
    return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
}


OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_statfs(OSCL_FSSTAT *stats, const char *path)
{
    struct statfs fs_stats;
    if (statfs(path, &fs_stats) == 0)
    {
        stats->freebytes = fs_stats.f_bsize * fs_stats.f_bfree;
        stats->totalbytes = fs_stats.f_bsize * fs_stats.f_blocks;
        return OSCL_FILEMGMT_E_OK;
    }
    return OSCL_FILEMGMT_E_UNKNOWN;

}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_statfs(OSCL_FSSTAT *stats, const oscl_wchar *path)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(path, oscl_strlen(path), convpathname, OSCL_IO_FILENAME_MAXLEN) &&
            oscl_strlen(path) != 0
       )
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return oscl_statfs(stats, convpathname);

}


OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_mkdir(const oscl_wchar *path)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(path, oscl_strlen(path), convpathname, OSCL_IO_FILENAME_MAXLEN) &&
            oscl_strlen(path) != 0)
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return oscl_mkdir(convpathname);
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_mkdir(const char *path)
{
    if (mkdir(path, 0775) == 0)
    {
        return OSCL_FILEMGMT_E_OK;
    }
    else
    {
        switch (errno)
        {
            case EPERM:
            case EACCES:
                return OSCL_FILEMGMT_E_PERMISSION_DENIED;
                break;
            case EEXIST:
                return OSCL_FILEMGMT_E_ALREADY_EXISTS;
                break;
            case ENOENT:
            case ENOTDIR:
                return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
            default:
                break;
        }
    }
    return OSCL_FILEMGMT_E_UNKNOWN;
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_rmdir(const oscl_wchar *path)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(path, oscl_strlen(path), convpathname, OSCL_IO_FILENAME_MAXLEN) &&
            oscl_strlen(path) != 0)
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return oscl_rmdir(convpathname);
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_rmdir(const char *path)
{
    if (rmdir(path) == 0)
    {
        return OSCL_FILEMGMT_E_OK;
    }
    else
    {
        switch (errno)
        {
            case EPERM:
                return OSCL_FILEMGMT_E_PERMISSION_DENIED;
            case ENOENT:
            case ENOTDIR:
                return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
            case ENOTEMPTY:
                return OSCL_FILEMGMT_E_NOT_EMPTY;
            default:
                break;
        }
    }
    return OSCL_FILEMGMT_E_UNKNOWN;
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_chdir(const oscl_wchar *path)
{
    char convpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(path, oscl_strlen(path), convpathname, OSCL_IO_FILENAME_MAXLEN) &&
            oscl_strlen(path) != 0)
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return oscl_chdir(convpathname);
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_chdir(const char *path)
{
    if (chdir(path) == 0)
    {
        return OSCL_FILEMGMT_E_OK;
    }
    else
    {
        switch (errno)
        {
            case ENOENT:
            case ENOTDIR:
                return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
            case EACCES:
                return OSCL_FILEMGMT_E_PERMISSION_DENIED;
            default:
                break;
        }
    }
    return OSCL_FILEMGMT_E_UNKNOWN;
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_rename(const oscl_wchar *oldpath, const oscl_wchar *newpath)
{
    char oldconvpathname[OSCL_IO_FILENAME_MAXLEN];
    char newconvpathname[OSCL_IO_FILENAME_MAXLEN];
    if (0 == oscl_UnicodeToUTF8(oldpath, oscl_strlen(oldpath), oldconvpathname, OSCL_IO_FILENAME_MAXLEN) ||
            0 == oscl_UnicodeToUTF8(newpath, oscl_strlen(newpath), newconvpathname, OSCL_IO_FILENAME_MAXLEN))
    {
        return OSCL_FILEMGMT_E_PATH_TOO_LONG;
    }
    return oscl_rename(oldconvpathname, newconvpathname);
}

OSCL_EXPORT_REF OSCL_FILEMGMT_ERR_TYPE oscl_rename(const char *oldpath, const char *newpath)
{
    if (rename(oldpath, newpath) == 0)
    {
        return OSCL_FILEMGMT_E_OK;
    }
    else
    {
        switch (errno)
        {
            case EISDIR:
            case EEXIST:
            case ENOTEMPTY:
                return OSCL_FILEMGMT_E_ALREADY_EXISTS;
            case ENOTDIR:
                return OSCL_FILEMGMT_E_PATH_NOT_FOUND;
            default:
                break;
        }
    }
    return OSCL_FILEMGMT_E_UNKNOWN;
}
