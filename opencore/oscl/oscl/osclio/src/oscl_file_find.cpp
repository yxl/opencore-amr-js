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

#include "oscl_file_find.h"
#include "oscl_utf8conv.h"
#include "oscl_stdstring.h"
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

OSCL_EXPORT_REF Oscl_FileFind::Oscl_FileFind()
{
    lastError = Oscl_FileFind::E_OK;
#if   ! OSCL_HAS_GLOB
#define OSCL_FILEFIND_NUMBER_OF_FILES_ENTRY 256
    int err = 0;
    OSCL_TRY(err,
             iDirEntVec.reserve(OSCL_FILEFIND_NUMBER_OF_FILES_ENTRY);
            );
    if (err)
    {
        iDirEntVec.clear();
        OSCL_LEAVE(err);
        lastError = Oscl_FileFind::E_OTHER;
    }
#endif
    delimeter = OSCL_FILE_CHAR_PATH_DELIMITER;
    nullchar = _STRLIT_CHAR("\0");
    count = 0;
    foundfirst = false;
    appendPathDelimiter = false;
    pathname = NULL;
    type = Oscl_FileFind::INVALID_TYPE;
}

OSCL_EXPORT_REF Oscl_FileFind::~Oscl_FileFind()
{
    Close();
}

static bool oscl_strglob(const char *str, const char *p);
OSCL_EXPORT_REF const char *Oscl_FileFind::FindFirst(const char *directory, const char *pattern, char *buf, uint32 buflen)
{
    const char *def_pattern = "*";
    lastError = Oscl_FileFind::E_OK;
    type = Oscl_FileFind::INVALID_TYPE;
    if (directory == NULL || buf == NULL || buflen <= 0)
    {
        lastError = Oscl_FileFind::E_INVALID_ARG;
        return NULL;
    }
    if (pattern == NULL) pattern = def_pattern;
    if (foundfirst)
    {
        lastError = Oscl_FileFind::E_INVALID_STATE;
        return NULL;
    }
#if   (OSCL_HAS_GLOB)
    if (!setpathanddelimiter(directory))
        return NULL;
    int retval;
    OSCL_HeapString<OsclMemAllocator> path(pathname);
    path += pattern;

    if ((retval = glob(path.get_cstr(), GLOB_ERR | GLOB_NOSORT , NULL, &hFind)) == 0)
    {
        foundfirst = true;
        if (hFind.gl_pathc > 0)
        {
            if (strlen(hFind.gl_pathv[count]) > buflen)
            {
                lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
                return NULL;
            }
            oscl_strncpy(buf, hFind.gl_pathv[count++], buflen);
            struct stat statbuf;
            if (stat(buf, &statbuf) == 0)
            {
                type = (S_ISDIR(statbuf.st_mode)) ? DIR_TYPE : FILE_TYPE;
            }
            else
            {
                type = FILE_TYPE;
            }
            return buf;
        }
        else
        {
            Close();
        }
    }
    else
    {
        if (GLOB_NOMATCH == retval)
        {
            lastError = Oscl_FileFind::E_NO_MATCH;
        }
        else if (GLOB_ABORTED)
        {
            lastError = Oscl_FileFind::E_PATH_NOT_FOUND;
        }
        else
        {
            lastError = Oscl_FileFind::E_OTHER;
        }
    }
#else
    // support linux having no glob.h support in glob pattern matching
    if (!setpathanddelimiter(directory))
        return NULL;
    DIR* pDir;
    struct dirent* pEnt;
    uint32 itr = 0;
    struct stat statbuf;
    if (oscl_strlen(directory) > 0)
    {
        pDir = opendir(directory);
    }
    else
    {
        // empty directory, replaced with searching current dir
        // make the behavior consistent with the glob-based implementation
        OSCL_HeapString<OsclMemAllocator> curpath(".");
        curpath += OSCL_FILE_CHAR_PATH_DELIMITER;
        pDir = opendir(curpath.get_cstr());
    }
    if (pDir == NULL)
    {
        lastError = Oscl_FileFind::E_PATH_NOT_FOUND;
        return NULL;
    }
    // parsing thru dirent structure
    while ((pEnt = readdir(pDir)) != NULL)
    {
        if (oscl_strglob(pEnt->d_name, pattern) &&
                oscl_strcmp(pEnt->d_name, ".") &&
                oscl_strcmp(pEnt->d_name, ".."))	// excluded out '.' and '..' from readdir
        {	// pattern matched
            buf[0] = *nullchar;
            oscl_strcat(buf, pathname);
            oscl_strcat(buf, pEnt->d_name);
            iDirEntVec.push_back(buf);
            // d_type is not all available on all lunix system, using stat() instead
            if (itr == 0)
            {   // first find filetype
                if (stat(pEnt->d_name, &statbuf) == 0)
                {
                    type = (S_ISDIR(statbuf.st_mode)) ? DIR_TYPE : FILE_TYPE;
                }
                else
                {
                    type = FILE_TYPE;
                }
            }
            itr++;
        }
    }
    closedir(pDir);
    if (iDirEntVec.size())
    {
        if (strlen(iDirEntVec[0].get_cstr()) > buflen)
        {
            lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
            return NULL;
        }
        // copy and return the first found element
        buf[0] = *nullchar;
        oscl_strcat(buf, iDirEntVec[0].get_cstr());
        foundfirst = true;
        count = 1; // advance to next element, used for findnext()
        return buf;
    }
    lastError = Oscl_FileFind::E_NO_MATCH;
#endif
    return NULL;
}

OSCL_EXPORT_REF const oscl_wchar *Oscl_FileFind::FindFirst(const oscl_wchar *directory, const oscl_wchar *pattern, oscl_wchar *buf, uint32 buflen)
{
    const oscl_wchar *def_pattern = _STRLIT_WCHAR("*");
    lastError = Oscl_FileFind::E_OK;
    type = Oscl_FileFind::INVALID_TYPE;
    if (directory == NULL || buf == NULL || buflen <= 0)
    {
        lastError = Oscl_FileFind::E_INVALID_ARG;
        return NULL;
    }
    if (pattern == NULL) pattern = def_pattern;
    if (foundfirst)
    {
        lastError = Oscl_FileFind::E_INVALID_STATE;
        return NULL;
    }
    // non-symbain OSs are converted to char type FindFirst()
    char* convpattern = (char*) OSCL_MALLOC(oscl_strlen(pattern) + 1);
    char* convdir = (char*) OSCL_MALLOC(oscl_strlen(directory) + 1);
    char* utf8buf = (char*) OSCL_MALLOC(buflen / sizeof(chartype));
    if (!(convpattern && convdir && utf8buf))
    {
        lastError = Oscl_FileFind::E_MEMORY_ERROR;
        OSCL_FREE(convdir);
        OSCL_FREE(convpattern);
        OSCL_FREE(utf8buf);
        return NULL;
    }
    if ((0 == oscl_UnicodeToUTF8(directory, oscl_strlen(directory), convdir, oscl_strlen(directory) + 1)
            && oscl_strlen(directory))
            || (0 == oscl_UnicodeToUTF8(pattern, oscl_strlen(pattern), convpattern, oscl_strlen(pattern) + 1)
                && oscl_strlen(pattern)))
    {
        lastError = Oscl_FileFind::E_PATH_TOO_LONG;
        OSCL_FREE(convdir);
        OSCL_FREE(convpattern);
        OSCL_FREE(utf8buf);
        return NULL;
    }
    const char* retval = FindFirst(convdir, convpattern, utf8buf, (buflen / sizeof(chartype)));
    OSCL_FREE(convdir);
    OSCL_FREE(convpattern);
    if (retval != NULL)
    {
        int32 err = oscl_UTF8ToUnicode(retval, oscl_strlen(retval), buf, buflen);
        OSCL_FREE(utf8buf);
        if (!err && oscl_strlen(retval))
        {
            lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
            return NULL;
        }
        return buf;
    }
    OSCL_FREE(utf8buf);
    return NULL;
}

OSCL_EXPORT_REF char *Oscl_FileFind::FindNext(char *buf, uint32 buflen)
{
    lastError = Oscl_FileFind::E_OK;
    type = Oscl_FileFind::INVALID_TYPE;
    if (!buf || buflen <= 0)
    {
        lastError = Oscl_FileFind::E_INVALID_ARG;
        return NULL;
    }
    if (!foundfirst)
    {
        lastError = Oscl_FileFind::E_INVALID_STATE;
        return NULL;
    }
#if   (OSCL_HAS_GLOB)
    if (count >= hFind.gl_pathc)
    {
        lastError = Oscl_FileFind::E_NO_MATCH;
        return NULL;
    }
    if (oscl_strlen(hFind.gl_pathv[count]) > buflen)
    {
        lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
    }
    else
    {
        oscl_strncpy(buf, hFind.gl_pathv[count++], buflen);
        struct stat statbuf;
        if (stat(buf, &statbuf) == 0)
        {
            type = (S_ISDIR(statbuf.st_mode)) ? DIR_TYPE : FILE_TYPE;
        }
        else
        {
            type = FILE_TYPE;
        }
        return buf;
    }
#else
    if (count >= iDirEntVec.size())
    {
        lastError = Oscl_FileFind::E_NO_MATCH;
        return NULL;
    }
    if (oscl_strlen(iDirEntVec[count].get_cstr()) > buflen)
    {
        lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
    }
    else
    {
        buf[0] = *nullchar;
        oscl_strcat(buf, iDirEntVec[count++].get_cstr());
        struct stat statbuf;
        if (stat(buf, &statbuf) == 0)
        {
            type = (S_ISDIR(statbuf.st_mode)) ? DIR_TYPE : FILE_TYPE;
        }
        else
        {
            type = FILE_TYPE;
        }
        return buf;
    }
#endif
    return NULL;
}

OSCL_EXPORT_REF oscl_wchar *Oscl_FileFind::FindNext(oscl_wchar *buf, uint32 buflen)
{
    lastError = Oscl_FileFind::E_OK;
    type = Oscl_FileFind::INVALID_TYPE;
    if (!buf || buflen <= 0)
    {
        lastError = Oscl_FileFind::E_INVALID_ARG;
        return NULL;
    }
    if (!foundfirst)
    {
        lastError = Oscl_FileFind::E_INVALID_STATE;
        return NULL;
    }
    char* utf8buf = (char*) OSCL_MALLOC(buflen * sizeof(chartype));
    if (!utf8buf)
    {
        lastError = Oscl_FileFind::E_MEMORY_ERROR;
        return NULL;
    }
    const char* retval = FindNext(utf8buf, buflen * sizeof(chartype));
    if (retval != NULL)
    {
        int32 err = oscl_UTF8ToUnicode(retval, oscl_strlen(retval), buf, buflen);
        OSCL_FREE(utf8buf);
        if (!err && oscl_strlen(retval))
        {
            lastError = Oscl_FileFind::E_BUFFER_TOO_SMALL;
            return NULL;
        }
        return buf;
    }
    lastError = Oscl_FileFind::E_NO_MATCH;
    OSCL_FREE(utf8buf);
    return NULL;
}

OSCL_EXPORT_REF void Oscl_FileFind::Close()
{
#if   (OSCL_HAS_GLOB)
    if (foundfirst)
        globfree(&hFind);
#else
    iDirEntVec.clear();
#endif
    foundfirst = false;
    lastError = Oscl_FileFind::E_OK;
    count = 0;
    appendPathDelimiter = false;
    if (pathname)
    {
        OSCL_FREE(pathname);
        pathname = NULL;
    }
}

OSCL_EXPORT_REF Oscl_FileFind::element_type Oscl_FileFind::GetElementType()
{
    return type;
}

OSCL_EXPORT_REF Oscl_FileFind::error_type Oscl_FileFind::GetLastError()
{
    return lastError;
}

bool Oscl_FileFind::setpathanddelimiter(const chartype* directory)
{
    if (pathname)
    {
        lastError = Oscl_FileFind::E_INVALID_STATE;
        return false;
    }
    if (directory[oscl_strlen(directory)-1] != *delimeter && oscl_strlen(directory))
        appendPathDelimiter = true;
    if (appendPathDelimiter)
        pathname = (chartype*) OSCL_MALLOC((oscl_strlen(directory) + 2) * sizeof(chartype));
    else
        pathname = (chartype*) OSCL_MALLOC((oscl_strlen(directory) + 1) * sizeof(chartype));
    if (!pathname)
    {
        lastError = E_MEMORY_ERROR;
        return false;
    }
    pathname[0] = *nullchar;
    oscl_strcat(pathname, directory);
    if (appendPathDelimiter)
        oscl_strcat(pathname, delimeter);
    return true;
}
// globmatch matches pattern strings p from str, follows linux glob.c man spec.
static bool oscl_strglob(const char *str, const char *p)
{
#define NEGATE	'^'			/* std cset negation char */
    int negate;
    int match;
    int c;

    while (*p)
    {
        if (!*str && *p != '*')
            return false;

        switch (c = *p++)
        {

            case '*':
                while (*p == '*')
                    p++;

                if (!*p)
                    return true;

                if (*p != '?' && *p != '[' && *p != '\\')
                    while (*str && *p != *str)
                        str++;

                while (*str)
                {
                    if (oscl_strglob(str, p))
                        return true;
                    str++;
                }
                return false;

            case '?':
                if (*str)
                    break;
                return false;
                /*
                 * set specification is inclusive, that is [a-z] is a, z and
                 * everything in between. this means [z-a] may be interpreted
                 * as a set that contains z, a and nothing in between.
                 */
            case '[':
                if (*p != NEGATE)
                    negate = false;
                else
                {
                    negate = true;
                    p++;
                }

                match = false;

                while (!match && (c = *p++))
                {
                    if (!*p)
                        return false;
                    if (*p == '-')  	/* c-c */
                    {
                        if (!*++p)
                            return false;
                        if (*p != ']')
                        {
                            if (*str == c || *str == *p ||
                                    (*str > c && *str < *p))
                                match = true;
                        }
                        else  		/* c-] */
                        {
                            if (*str >= c)
                                match = true;
                            break;
                        }
                    }
                    else  			/* cc or c] */
                    {
                        if (c == *str)
                            match = true;
                        if (*p != ']')
                        {
                            if (*p == *str)
                                match = true;
                        }
                        else
                            break;
                    }
                }

                if (negate == match)
                    return false;
                /*
                 * if there is a match, skip past the cset and continue on
                 */
                while (*p && *p != ']')
                    p++;
                if (!*p++)
                    return false;
                break;

            case '\\':
                if (*p)
                    c = *p++;
            default:
                if (c != *str)
                    return false;
                break;

        }
        str++;
    }

    return !*str;
}

