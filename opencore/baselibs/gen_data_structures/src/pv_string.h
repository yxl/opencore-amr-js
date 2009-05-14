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

#ifndef PV_STRING_H
#define PV_STRING_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "oscl_mutex.h"

#include "oscl_stdstring.h"
#include "oscl_mem_basic_functions.h"
#include "oscl_mem.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class String
{
    private:
        struct Srep;
        Srep *rep;

    public:

        String();
        String(const char *);
        String(const char *src, int length);
        String(const String&);
        String& operator=(const char *);
        String& operator=(const String&);


        friend int operator==(const String& a, const String& b);
        friend int operator!=(const String& a, const String& b);
        friend int operator<(const String& a, const String& b);
        friend int operator<=(const String& a, const String& b);
        friend int operator>(const String& a, const String& b);
        friend int operator>=(const String& a, const String& b);

        ~String();

        char read(int index) const;
        void write(int index, char c);
        void write(unsigned int offset, unsigned int length, char *ptr);
        int get_size() const;
        int size() const
        {
            return get_size();
        };

        const char * get_cstr() const;
        const char * c_str() const
        {
            return get_cstr();
        };

        String& operator+=(const char* src);
        String& operator+=(const String& src);
        String& operator+=(const char);

        char operator[](int index) const;

        char hash() const;
};



struct String::Srep
{
public:
    char *buffer; // holds actual string value
    OsclMutex mutex;
    int size; // number of elements;
    int refcnt; // reference count;

    Srep(int nsz, const char *src)
    {
        refcnt = 1;
        size = nsz;
        buffer = OSCL_ARRAY_NEW(char, size + 1); // allocate enough space including terminator
        oscl_strncpy(buffer, src, size);
        buffer[size] = '\0';

    }

    ~Srep()
    {
        OSCL_ARRAY_DELETE(buffer);
    }

    Srep* get_own_copy()
    {
        mutex.Lock();
        if (1 == refcnt)
        {
            // already a private copy so return
            mutex.Unlock();
            return this;
        }


        --refcnt; // decrement reference

        Srep *tmp = OSCL_NEW(Srep, (size, buffer));
        mutex.Unlock();

        return tmp;
    }

    void assign(int nsz, const char *src)
    {

        if (size != nsz)
        {
            delete[] buffer;
            size = nsz;
            buffer = OSCL_ARRAY_NEW(char, size + 1);
        }

        oscl_strncpy(buffer, src, size);
        buffer[size] = '\0';

    }

private:
    Srep(const Srep&);
    Srep& operator=(const Srep&);

};


inline char String::read(int index) const
{
    return rep->buffer[index];
};

inline void String::write(int index, char c)
{
    rep = rep->get_own_copy();
    rep->buffer[index] = c;
}

inline void String::write(unsigned int offset, unsigned int length, char *ptr)
{
    rep = rep->get_own_copy();
    int to_copy = (length > rep->size - offset) ? rep->size - offset : length;
    oscl_memcpy(rep->buffer + offset, ptr, to_copy);
    to_copy += offset;
    if (to_copy < rep->size)
    {
        rep->buffer[to_copy] = '\0';
    };
}

inline const char * String::get_cstr() const
{
    return rep->buffer;
}

inline int String::get_size() const
{
    return rep->size;
};

inline int operator==(const String& a, const String& b)
{
    return (!oscl_strcmp(a.rep->buffer, b.rep->buffer));
}

inline int operator!=(const String& a, const String& b)
{
    return (oscl_strcmp(a.rep->buffer, b.rep->buffer) != 0);
}

inline int operator>(const String& a, const String& b)
{
    return (oscl_strcmp(a.rep->buffer, b.rep->buffer) > 0);
}

inline int operator>=(const String& a, const String& b)
{
    return (oscl_strcmp(a.rep->buffer, b.rep->buffer) >= 0);
}

inline int operator<=(const String& a, const String& b)
{
    return (oscl_strcmp(a.rep->buffer, b.rep->buffer) <= 0);
}

inline int operator<(const String& a, const String& b)
{
    return (oscl_strcmp(a.rep->buffer, b.rep->buffer) < 0);
}

#endif
