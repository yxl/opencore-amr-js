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
#include "oscl_stdstring.h"

#define OSCL_ASCII_CASE_MAGIC_BIT 0x20

OSCL_EXPORT_REF uint32 oscl_strlen(const char* str)
{
    return (uint32)strlen(str);
}

OSCL_EXPORT_REF uint32 oscl_strlen(const oscl_wchar* str)
{
    uint32 len = 0;
    while (*str != '\0')
    {
        str++;
        len++;
    }
    return len;
}

OSCL_EXPORT_REF char* oscl_strncpy(char* dest, const char* src, uint32 count)
{
    return strncpy(dest, src, count);
}

OSCL_EXPORT_REF oscl_wchar* oscl_strncpy(oscl_wchar *dest, const oscl_wchar *src, uint32 count)
{
    oscl_wchar* tmp = dest;
    uint32 ii;
    for (ii = 0; ii < count && *src != '\0'; ii++)
    {
        *tmp++ = *src++;
    }
    // pad with null chars upto length count
    for (; ii < count; ii++)
    {
        *tmp++ = '\0';
    }
    return dest;
}

OSCL_EXPORT_REF int32 oscl_strcmp(const char* str1, const char* str2)
{
    return (int32)strcmp(str1, str2);
}

OSCL_EXPORT_REF int32 oscl_strncmp(const char* str1, const char* str2, uint32 count)
{
    return (int32)strncmp(str1, str2, count);
}


OSCL_EXPORT_REF int32 oscl_strcmp(const oscl_wchar *str1, const oscl_wchar *str2)
{
    while ((*str1 == *str2) && (*str1 != '\0'))
    {
        str1++;
        str2++;
    }
    return (*str1 - *str2);
}


OSCL_EXPORT_REF int32 oscl_strncmp(const oscl_wchar *str1, const oscl_wchar *str2, uint32 count)
{
    for (uint32 ii = 0; ii < count; ii++)
    {
        if (*str1 != *str2 || *str1 == '\0')
        {
            return (*str1 - *str2);
        }
        str1++;
        str2++;
    }
    return 0;
}


OSCL_EXPORT_REF char* oscl_strncat(char* dest, const char* src, uint32 count)
{
    return strncat(dest, src, count);
}

OSCL_EXPORT_REF oscl_wchar* oscl_strncat(oscl_wchar* dest, const oscl_wchar* src, uint32 count)
{
    oscl_wchar* tmp = dest + oscl_strlen(dest);
    for (uint32 ii = 0; ii < count && *src != '\0'; ii++)
    {
        *tmp++ = *src++;
    }
    *tmp = '\0';
    return dest;
}

OSCL_EXPORT_REF const char* oscl_strchr(const char *str, int32 c)
{
    if (str)
    {
        while (*str != '\0')
        {
            if (*str == (char)c)
                return str;
            str++;
        }
        if (*str == (char)c)
            return str;
    }
    return NULL;
}

OSCL_EXPORT_REF char* oscl_strchr(char *str, int32 c)
{
    return (char*)oscl_strchr((const char*)str, c);
}

OSCL_EXPORT_REF const oscl_wchar* oscl_strchr(const oscl_wchar *str, int32 c)
{
    if (str)
    {
        while (*str != '\0')
        {
            if (*str == (oscl_wchar)c)
                return str;
            str++;
        }
        if (*str == (oscl_wchar)c)
            return str;
    }
    return NULL;
}

OSCL_EXPORT_REF oscl_wchar* oscl_strchr(oscl_wchar *str, int32 c)
{
    return (oscl_wchar*)oscl_strchr((const oscl_wchar*)str, c);
}

OSCL_EXPORT_REF const char* oscl_strrchr(const char *str, int32 c)
{
    if (!str)
        return NULL;

    const char*start = str;

    while (*str != '\0')
        str++;

    while (str >= start)
    {
        if (*str == (char)c)
            return str;
        if (str == start)
            return NULL;
        str--;
    }
    return NULL;
}

OSCL_EXPORT_REF char* oscl_strrchr(char *str, int32 c)
{
    return (char*)oscl_strrchr((const char*)str, c);
}

OSCL_EXPORT_REF const oscl_wchar* oscl_strrchr(const oscl_wchar *str, int32 c)
{
    if (!str)
        return NULL;

    const oscl_wchar*start = str;

    while (*str != '\0')
        str++;

    while (str >= start)
    {
        if (*str == (oscl_wchar)c)
            return str;
        if (str == start)
            return NULL;
        str--;
    }
    return NULL;
}

OSCL_EXPORT_REF oscl_wchar* oscl_strrchr(oscl_wchar *str, int32 c)
{
    return (oscl_wchar*)oscl_strrchr((const oscl_wchar*)str, c);
}

OSCL_EXPORT_REF char* oscl_strset(char* dest, char val, uint32 count)
{
    for (uint32 ii = 0; ii < count; ii++)
    {
        *dest++ = val;
    }
    return dest;
}

OSCL_EXPORT_REF oscl_wchar* oscl_strset(oscl_wchar* dest, oscl_wchar val, uint32 count)
{
    for (uint32 ii = 0; ii < count; ii++)
    {
        *dest++ = val;
    }
    return dest;
}

OSCL_EXPORT_REF bool oscl_isLetter(const char c)
{
    return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122));
}

OSCL_EXPORT_REF int32 oscl_CIstrcmp(const char *str1, const char *str2)
{

    while ((*str1 != '\0') && (oscl_tolower(*str1) == oscl_tolower(*str2)))
    {
        str1++;
        str2++;
    }
    return (oscl_tolower(*str1) - oscl_tolower(*str2));
}

OSCL_EXPORT_REF int32 oscl_CIstrcmp(const oscl_wchar *str1, const oscl_wchar *str2)
{
    while ((*str1 != '\0') && (oscl_tolower(*str1) == oscl_tolower(*str2)))
    {
        str1++;
        str2++;
    }
    return (oscl_tolower(*str1) - oscl_tolower(*str2));

}

OSCL_EXPORT_REF int32 oscl_CIstrncmp(const char *str1, const char *str2, uint32 count)
{
    for (uint32 i = 0;i < count;i++)
    {
        if ((*str1 == '\0') || (oscl_tolower(*str1) != oscl_tolower(*str2)))
            return (oscl_tolower(*str1) - oscl_tolower(*str2));
        str1++;
        str2++;
    }
    return 0;
}

OSCL_EXPORT_REF int32 oscl_CIstrncmp(const oscl_wchar *str1, const oscl_wchar *str2, uint32 count)
{
    for (uint32 i = 0;i < count;i++)
    {
        if ((*str1 == '\0') || (oscl_tolower(*str1) != oscl_tolower(*str2)))
            return (oscl_tolower(*str1) - oscl_tolower(*str2));
        str1++;
        str2++;
    }
    return 0;
}

OSCL_EXPORT_REF char oscl_tolower(const char car)
{
    return tolower(car);
}

OSCL_EXPORT_REF oscl_wchar oscl_tolower(const oscl_wchar car)
{
    if (car >= 65 && car <= 90)
        return (OSCL_STATIC_CAST(oscl_wchar, (car + OSCL_ASCII_CASE_MAGIC_BIT)));
    else
        return car;
}

OSCL_EXPORT_REF  const char* oscl_strstr(const char* str1,  const char* str2)
{
    return (const char*)strstr(str1, str2);
}

OSCL_EXPORT_REF  char* oscl_strstr(char* str1,  const char* str2)
{
    return (char*)strstr(str1, str2);
}

OSCL_EXPORT_REF  const oscl_wchar* oscl_strstr(const oscl_wchar* str1,  const oscl_wchar* str2)
{
    uint32 size = oscl_strlen(str1);
    uint32 size2 = oscl_strlen(str2);
    const oscl_wchar* p = (oscl_wchar*) str1;
    while ((*p != '\0') && (size >= size2))
    {
        if (!oscl_strncmp(p, str2, size2))
            return p;
        else
        {
            p += 1;
            size -= 1;
        }
    }
    return 0;
}

OSCL_EXPORT_REF  oscl_wchar* oscl_strstr(oscl_wchar* str1,  const oscl_wchar* str2)
{
    uint32 size = oscl_strlen(str1);
    uint32 size2 = oscl_strlen(str2);
    oscl_wchar* p = (oscl_wchar*) str1;
    while ((*p != '\0') && (size >= size2))
    {
        if (!oscl_strncmp(p, str2, size2))
            return p;
        else
        {
            p += 1;
            size -= 1;
        }
    }
    return 0;
}

OSCL_EXPORT_REF char* oscl_strcat(char* dest, const char* src)
{
    return strcat(dest, src);
}

OSCL_EXPORT_REF oscl_wchar* oscl_strcat(oscl_wchar* dest, const oscl_wchar* src)
{
    oscl_wchar* tmp = dest + oscl_strlen(dest);
    for (uint32 i = 0; *src != '\0'; i++)
    {
        *tmp++ = *src++;
    }
    *tmp = '\0';
    return dest;
}
