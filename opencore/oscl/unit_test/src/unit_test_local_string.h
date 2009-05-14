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

//               O S C L_ S T R I N G   C L A S S

//    This is a simple string class without any multithread access
//    protection.

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#ifndef UNIT_TEST_LOCAL_STRING_H
#define UNIT_TEST_LOCAL_STRING_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline int _strlen(const char* s)
{
    for (int i = 0; ;i++)
    {
        if (s[i] == '\0')
            return i;
    }
}

inline char* _strcat(char* dest, const char* src)
{
    char* tmp = dest + _strlen(dest);
    for (uint32 i = 0; *src != '\0'; i++)
    {
        *tmp++ = *src++;
    }
    *tmp = '\0';
    return dest;
}

inline char* _strncpy(char* dest, const char* src, uint32 count)
{
    char* tmp = dest;
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

inline int32 _strcmp(const char* str1, const char* str2)
{
    while ((*str1 == *str2) && (*str1 != '\0'))
    {
        str1++;
        str2++;
    }
    return (*str1 - *str2);
}

// **************************************************************

/** @name UnitTest_String is a simple string class
    which is compatible with regular character array
    strings as well as Unicode wchar_t array strings.

    The class uses a copy-on-write to minimize unnecessary
    copying when multiple instances of a string are created
    for reading.  Allocated memory is automatically freed by
    the class destructor when the last string referencing the
    memory is destroyed.  The class HAS NO thread synchronization
    built-in, so it is NOT MT-SAFE.  External locks should be used
    if the class is to be shared across threads.
*/

class UnitTest_String_Srep : public UnitTest_HeapBase
{
    public:

        char *buffer; // holds actual string value
        int32 size; // number of elements;
        int32 refcnt; // reference count;

        UnitTest_String_Srep(uint32 nsz, const char *src)
        {
            refcnt = 1;
            size = nsz;
            buffer = (char*)unit_test_allocator::allocate(size + 1);          /* allocate enough space
                                      * including terminator
                                      */
            _strncpy(buffer, src, size);

            buffer[size] = '\0';

        }

        ~UnitTest_String_Srep()
        {
            unit_test_allocator::deallocate(buffer);
        }

        UnitTest_String_Srep* get_own_copy()
        {
            if (1 == refcnt)
            {
                // already a private copy so return
                return this;
            }


            --refcnt; // decrement reference

            UnitTest_String_Srep *tmp = new UnitTest_String_Srep(size, buffer);
            return tmp;
        }

        void assign(int32 nsz, const char *src)
        {

            if (size != nsz)
            {
                unit_test_allocator::deallocate(buffer);
                size = nsz;
                buffer = (char*)unit_test_allocator::allocate(size + 1);
            }

            _strncpy(buffer, src, size);
            buffer[size] = '\0';

        }

    private:
        UnitTest_String_Srep(const UnitTest_String_Srep&);
        UnitTest_String_Srep& operator=(const UnitTest_String_Srep&);

};

class UnitTest_String
{

    private:
// Not needed anymore!  struct Srep;   // Note this is a forward declaraton only, allocates no memory.
        typedef UnitTest_String_Srep Srep;
        Srep *rep;

    public:

        /// Default constructor -- simply creates an empty string
        UnitTest_String();

        /// Copy constructor from character array
        UnitTest_String(const char *cp);

        /// Copy constructor from character array, but allocates
        /// length according to the length parameter.
        UnitTest_String(const char *src, uint32 length);

        /// Copy constructor from another UnitTest_String
        UnitTest_String(const UnitTest_String& src);

        /// Assignment operator from a character array
        UnitTest_String& operator=(const char *);

        /// Assignment operator from another UnitTest_String
        UnitTest_String& operator=(const UnitTest_String &);


        friend int32 operator== (const UnitTest_String& a, const UnitTest_String& b);
        friend int32 operator!= (const UnitTest_String& a, const UnitTest_String& b);
        friend int32 operator< (const UnitTest_String& a, const UnitTest_String& b);
        friend int32 operator<= (const UnitTest_String& a, const UnitTest_String& b);
        friend int32 operator> (const UnitTest_String& a, const UnitTest_String& b);
        friend int32 operator>= (const UnitTest_String& a, const UnitTest_String& b);


        ~UnitTest_String();

        /// Access functions for the string size
        int32 get_size() const;
        int32 size() const
        {
            return get_size();
        };

        /// Access function for the C-style string
        const char * get_cstr() const;
        const char * c_str() const
        {
            return get_cstr();
        };

        /// Append a c-style string
        UnitTest_String& operator+=(const char* src);

        /// Append another UnitTest_String to this UnitTest_String
        UnitTest_String& operator+=(const UnitTest_String& src);

        /// Append a single character
        UnitTest_String& operator+=(const char c);

        char operator[](int32 index) const;


};

inline const char * UnitTest_String::get_cstr() const
{
    return rep->buffer;
}

inline int32 UnitTest_String::get_size() const
{
    return rep->size;
}

inline int32 operator==(const UnitTest_String& a, const UnitTest_String& b)
{
    return (!_strcmp(a.rep->buffer, b.rep->buffer));
}

inline int32 operator!=(const UnitTest_String& a, const UnitTest_String& b)
{
    return (_strcmp(a.rep->buffer, b.rep->buffer) != 0);
}

inline int32 operator>(const UnitTest_String& a, const UnitTest_String& b)
{
    return (_strcmp(a.rep->buffer, b.rep->buffer) > 0);
}

inline int32 operator>=(const UnitTest_String& a, const UnitTest_String& b)
{
    return (_strcmp(a.rep->buffer, b.rep->buffer) >= 0);
}

inline int32 operator<=(const UnitTest_String& a, const UnitTest_String& b)
{
    return (_strcmp(a.rep->buffer, b.rep->buffer) <= 0);
}

inline int32 operator<(const UnitTest_String& a, const UnitTest_String& b)
{
    return (_strcmp(a.rep->buffer, b.rep->buffer) < 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline UnitTest_String::UnitTest_String()
{
    char *ptr = NULL;
    rep = new Srep(0, ptr);
}

inline UnitTest_String::UnitTest_String(const char *cp)
{
    if (cp)
        rep = new Srep(_strlen(cp), cp);
    else
        rep = new Srep(0, cp);
}

inline UnitTest_String::UnitTest_String(const char *cp, uint32 length)
{
    rep = new Srep(length, cp);
}

inline UnitTest_String::UnitTest_String(const UnitTest_String& src)
{
    src.rep->refcnt++;
    rep = src.rep;
}

inline UnitTest_String::~UnitTest_String()
{
    if (--rep->refcnt == 0) delete rep;
}

inline UnitTest_String& UnitTest_String::operator=(const UnitTest_String & src)
{

    if (rep == src.rep)
    {
        return *this;  // protect against "str = str"
    }
    src.rep->refcnt++;
    if (--rep->refcnt == 0)
    {
        delete rep;
    }

    rep = src.rep;
    return *this;
}

inline UnitTest_String& UnitTest_String::operator=(const char * cp)
{
    if (--rep->refcnt == 0)
    {
        delete rep;
    }

    if (cp == NULL)
    {
        rep = new Srep(0, cp);
    }
    else
    {
        rep = new Srep(_strlen(cp), cp);
    }
    return *this;
}

inline UnitTest_String& UnitTest_String::operator+=(const char * src)
{
    Srep *new_rep;
    int32 new_size = rep->size + _strlen(src);
    new_rep = new Srep(new_size, rep->buffer);
    _strcat(new_rep->buffer, src);
    if (--rep->refcnt == 0)
    {
        delete rep;
    }
    rep = new_rep;
    return *this;
}

inline UnitTest_String& UnitTest_String::operator+=(const UnitTest_String & src)
{
    Srep *new_rep;
    int32 new_size = rep->size + src.rep->size;
    new_rep = new Srep(new_size, rep->buffer);
    _strcat(new_rep->buffer, src.rep->buffer);
    if (--rep->refcnt == 0)
    {
        delete rep;
    }
    rep = new_rep;
    return *this;
}

inline UnitTest_String& UnitTest_String::operator+=(const char c)
{
    char tmp_str[2];
    tmp_str[0] = c;
    tmp_str[1] = (char)'\0';

    return ((*this) += tmp_str);
}

inline char UnitTest_String::operator[](int32 index) const
{
    if (index < 0 || index >= rep->size)
        return '\0';
    return rep->buffer[index];
}



#endif //UNIT_TEST_LOCAL_STRING_H

