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

//               P V S T R I N G   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "pv_string.h"
#include "oscl_mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

String::String()
{
    rep = OSCL_NEW(Srep, (0, ""));
}

String::String(const String& src)
{
    src.rep->mutex.Lock();
    src.rep->refcnt++;
    rep = src.rep;
    src.rep->mutex.Unlock();
}

String::~String()
{
    rep->mutex.Lock();
    if (--rep->refcnt == 0)
    {
        OSCL_DELETE(rep);
    }
    else rep->mutex.Unlock();
}

String& String::operator=(const String & src)
{

    if (rep == src.rep)
    {
        return *this;  // protect against "str = str"
    }
    src.rep->mutex.Lock();
    src.rep->refcnt++;
    rep->mutex.Lock();
    if (--rep->refcnt == 0)
    {
        OSCL_DELETE(rep);
    }
    else
    {
        rep->mutex.Unlock();
    }

    rep = src.rep;
    src.rep->mutex.Unlock();
    return *this;

}

String& String::operator+=(const char * src)
{
    Srep *new_rep;
    int new_size = rep->size + oscl_strlen(src);
    new_rep = OSCL_NEW(Srep, (new_size, rep->buffer));
    oscl_strcat(new_rep->buffer, src);
    rep->mutex.Lock();
    if (--rep->refcnt == 0)
    {
        OSCL_DELETE(rep);
    }
    else
    {
        rep->mutex.Unlock();
    }
    rep = new_rep;
    return *this;
}

String& String::operator+=(const String & src)
{
    Srep *new_rep;
    int new_size = rep->size + src.rep->size;
    new_rep = OSCL_NEW(Srep, (new_size, rep->buffer));

    oscl_strcat(new_rep->buffer, src.rep->buffer);
    rep->mutex.Lock();
    if (--rep->refcnt == 0)
    {
        OSCL_DELETE(rep);
    }
    else
    {
        rep->mutex.Unlock();
    }
    rep = new_rep;
    return *this;
}

String & String::operator+=(const char c)
{
    char tmp_str[2];
    tmp_str[0] = c;
    tmp_str[1] = '\0';

    return ((*this) += tmp_str);
}



String::String(const char *src)
{
    rep = OSCL_NEW(Srep, (oscl_strlen(src), src));
}

String::String(const char *src, int length)
{
    rep = OSCL_NEW(Srep, (length, src));
}


String& String::operator=(const char * src)
{
    rep->mutex.Lock();
    if (rep->refcnt == 1)
    {
        rep->assign(oscl_strlen(src), src);
        rep->mutex.Unlock();
    }
    else
    {
        rep->refcnt--;
        rep->mutex.Unlock();
        rep = OSCL_NEW(Srep, (oscl_strlen(src), src));
    }

    return *this;
}



char String::operator[](int index) const
{
    if (index < 0 || index >= rep->size)
        return '\0';
    return rep->buffer[index];
}


char String::hash() const
{
    unsigned long h = 0;
    char uc = 0;
    int ii;
    char *ptr;

    for (ii = 0, ptr = rep->buffer ; ii < rep->size; ++ii, ++ptr)
    {
        h = 5 * h + *ptr;
    }

    for (ii = 0; ii < 4; ++ii)
    {
        uc ^= h & 0xFF;
        h >>= 8;
    }
    return uc;
}
