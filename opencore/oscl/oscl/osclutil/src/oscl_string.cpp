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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               O S C L _ S T R I N G   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "oscl_string.h"
#include "oscl_error.h"
#include "oscl_stdstring.h"

#ifndef OSCL_COMBINED_DLL
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()
#endif

// **************************************************************
//					 OSCL_String Implementation
//					 OSCL_wString Implementation
// **************************************************************

OSCL_EXPORT_REF OSCL_String::OSCL_String()
{}

OSCL_EXPORT_REF OSCL_wString::OSCL_wString()
{}

// **************************************************************
OSCL_EXPORT_REF OSCL_String::~OSCL_String()
{}

OSCL_EXPORT_REF OSCL_wString::~OSCL_wString()
{}

// **************************************************************
OSCL_EXPORT_REF OSCL_String::chartype OSCL_String::read(uint32 index) const
//default implementation-- may be replaced by derived class
{
    //allow reading entire string incl null terminator.
    if (index > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid index.
    return get_cstr()[index];
}

OSCL_EXPORT_REF OSCL_wString::chartype OSCL_wString::read(uint32 index) const
//default implementation-- may be replaced by derived class
{
    //allow reading entire string incl null terminator.
    if (index > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid index.
    return get_cstr()[index];
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator==(const OSCL_String& a) const
{
    return (get_size() == a.get_size()
            && oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) == 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator==(const OSCL_wString& a) const
{
    return (get_size() == a.get_size()
            && oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) == 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator==(const chartype* a) const
{
    uint32 len = (a) ? oscl_strlen(a) : 0;
    return (get_size() == len
            && oscl_strncmp(get_cstr(), a, len) == 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator==(const chartype* a) const
{
    uint32 len = (a) ? oscl_strlen(a) : 0;
    return (get_size() == len
            && oscl_strncmp(get_cstr(), a, len) == 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator!=(const OSCL_String& a) const
{
    return (get_size() != a.get_size()
            || oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) != 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator!=(const OSCL_wString& a) const
{
    return (get_size() != a.get_size()
            || oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) != 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator>(const OSCL_String& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) > 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator>(const OSCL_wString& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) > 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator>=(const OSCL_String& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) >= 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator>=(const OSCL_wString& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) >= 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator<=(const OSCL_String& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) <= 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator<=(const OSCL_wString& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) <= 0);
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::operator<(const OSCL_String& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) < 0);
}

OSCL_EXPORT_REF bool OSCL_wString::operator<(const OSCL_wString& a) const
{
    return (oscl_strncmp(get_cstr(), a.get_cstr(), a.get_size()) < 0);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String::chartype OSCL_String::operator[](uint32 index) const
{
    if (index > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid index.
    return get_cstr()[index];
}

OSCL_EXPORT_REF OSCL_wString::chartype OSCL_wString::operator[](uint32 index) const
{
    if (index > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid index.
    return get_cstr()[index];
}

// **************************************************************
OSCL_EXPORT_REF bool OSCL_String::is_writable() const
//default implementation-- may be replaced by derived class
{
    return (get_str() != NULL);
}

OSCL_EXPORT_REF bool OSCL_wString::is_writable() const
//default implementation-- may be replaced by derived class
{
    return (get_str() != NULL);
}

// **************************************************************
OSCL_EXPORT_REF void OSCL_String::write(uint32 index, chartype c)
//default implementation-- may be replaced by derived class
{
    chartype* curbuf = get_str();
    if (!curbuf)
        OsclError::Leave(OsclErrGeneral);//not writable.

    if (c == '\0')
    {
        if (index > get_size())
            OsclError::Leave(OsclErrGeneral);//invalid index.
        curbuf[index] = c;
        set_len(index);
    }
    else
    {
        if (index >= get_size())
            OsclError::Leave(OsclErrGeneral);//invalid index.
        curbuf[index] = c;
    }
}

OSCL_EXPORT_REF void OSCL_wString::write(uint32 index, chartype c)
//default implementation-- may be replaced by derived class
{
    chartype* curbuf = get_str();
    if (!curbuf)
        OsclError::Leave(OsclErrGeneral);//not writable.

    if (c == '\0')
    {
        if (index > get_size())
            OsclError::Leave(OsclErrGeneral);//invalid index.
        curbuf[index] = c;
        set_len(index);
    }
    else
    {
        if (index >= get_size())
            OsclError::Leave(OsclErrGeneral);//invalid index.
        curbuf[index] = c;
    }
}

// **************************************************************
OSCL_EXPORT_REF void OSCL_String::write(uint32 offset, uint32 length, const chartype* ptr)
//default implementation-- may be replaced by derived class
{
    if (!ptr || length == 0)
        return;

    chartype* curbuf = get_str();
    if (!curbuf)
        OsclError::Leave(OsclErrGeneral);//not writable.

    if (offset > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid offset.

    if (length + offset > get_size())
    {
        //extend length...
        uint32 ncopy = length;
        if (length + offset > get_maxsize())
            ncopy = get_maxsize() - offset;//truncate
        oscl_strncpy(curbuf + offset, ptr, ncopy);
        curbuf[offset+ncopy] = '\0';
        set_len(oscl_strlen(curbuf));
    }
    else
    {
        //write within current length
        oscl_strncpy(curbuf + offset, ptr, length);
    }
}

OSCL_EXPORT_REF void OSCL_wString::write(uint32 offset, uint32 length, const chartype* ptr)
//default implementation-- may be replaced by derived class
{
    if (!ptr || length == 0)
        return;

    chartype* curbuf = get_str();
    if (!curbuf)
        OsclError::Leave(OsclErrGeneral);//not writable.

    if (offset > get_size())
        OsclError::Leave(OsclErrGeneral);//invalid offset.

    if (length + offset > get_size())
    {
        //extend length...
        uint32 ncopy = length;
        if (length + offset > get_maxsize())
            ncopy = get_maxsize() - offset;//truncate
        oscl_strncpy(curbuf + offset, ptr, ncopy);
        curbuf[offset+ncopy] = '\0';
        set_len(oscl_strlen(curbuf));
    }
    else
    {
        //write within current length
        oscl_strncpy(curbuf + offset, ptr, length);
    }
}

// **************************************************************
OSCL_EXPORT_REF int8 OSCL_String::hash() const
//default implementation-- may be replaced by derived class
{
    if (!get_str())
        OsclError::Leave(OsclErrGeneral);//not writable.

    uint32 h = 0;
    int8 uc = 0;
    uint32 ii;
    chartype* ptr;

    for (ii = 0, ptr = get_str() ; ii < get_size(); ++ii, ++ptr)
    {
        h = 5 * h + *ptr;
    }

    for (ii = 0; ii < 4; ++ii)
    {
        uc ^= h & 0xFF;
        h >>= 8;
    }

    return (uc);
}

OSCL_EXPORT_REF int8 OSCL_wString::hash() const
//default implementation-- may be replaced by derived class
{
    if (!get_str())
        OsclError::Leave(OsclErrGeneral);//not writable.

    uint32 h = 0;
    int8 uc = 0;
    uint32 ii;
    chartype* ptr;

    for (ii = 0, ptr = get_str() ; ii < get_size(); ++ii, ++ptr)
    {
        h = 5 * h + *ptr;
    }

    for (ii = 0; ii < 4; ++ii)
    {
        uc ^= h & 0xFF;
        h >>= 8;
    }

    return (uc);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String& OSCL_String::operator=(const OSCL_String & src)
{
    set_rep(src);
    return *this;
}

OSCL_EXPORT_REF OSCL_wString& OSCL_wString::operator=(const OSCL_wString & src)
{
    set_rep(src);
    return *this;
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String& OSCL_String::operator=(const chartype * cp)
{
    set_rep(cp);
    return *this;
}

OSCL_EXPORT_REF OSCL_wString& OSCL_wString::operator=(const chartype * cp)
{
    set_rep(cp);
    return *this;
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String& OSCL_String::operator+=(const OSCL_String & src)
{
    append_rep(src);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wString& OSCL_wString::operator+=(const OSCL_wString & src)
{
    append_rep(src);
    return (*this);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String& OSCL_String::operator+=(const chartype c)
{
    chartype tmp_str[2];
    tmp_str[0] = c;
    tmp_str[1] = '\0';
    append_rep(&tmp_str[0]);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wString& OSCL_wString::operator+=(const chartype c)
{
    chartype tmp_str[2];
    tmp_str[0] = c;
    tmp_str[1] = '\0';
    append_rep(&tmp_str[0]);
    return (*this);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_String& OSCL_String::operator+=(const chartype * cp)
{
    append_rep(cp);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wString& OSCL_wString::operator+=(const chartype * cp)
{
    append_rep(cp);
    return (*this);
}






