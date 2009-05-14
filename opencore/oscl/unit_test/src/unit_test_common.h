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
#ifndef UNIT_TEST_COMMON_H
#define UNIT_TEST_COMMON_H

//Note: the Oscl includes must be minimal here, since we use
//this framework to test Oscl.
#include "osclconfig.h"
#include "oscl_types.h"//to get basic types
#include "osclconfig_memory.h" //to get placement "new"

/** A basic allocator that does not rely on other modules.
 *
 */
class unit_test_allocator
{
    public:
        static OsclAny* allocate(const uint32 size)
        {
            return malloc(size);
        }

        static  void deallocate(OsclAny *p)
        {
            free(p);
        }
};

/**
* Using this as a base class will allow using new/delete with the
* basic allocator defined above.  This is used to avoid invoking
* the Oscl global new/delete overload, which relies on Oscl memory manager.
*/

class UnitTest_HeapBase
{
    public:

        static void* operator new(size_t aSize)
        {
            return unit_test_allocator::allocate(aSize);
        }

        static void* operator new[](size_t aSize)
        {
            return unit_test_allocator::allocate(aSize);
        }

        static void* operator new(size_t ,void* aPtr)
        {
            return aPtr;
        }

        static void operator delete(void* aPtr)
        {
            unit_test_allocator::deallocate(aPtr);
        }

        static void operator delete[](void* aPtr)
        {
            unit_test_allocator::deallocate(aPtr);
        }

        UnitTest_HeapBase() {}
        virtual ~UnitTest_HeapBase() {}


};

#include "unit_test_vector.h"
#include "unit_test_local_string.h"

#define _STRING UnitTest_String
#define _APPEND(string,value) string+=(value)
#define _VECTOR(var,allocator) UnitTest_Vector<var,allocator>


#endif //UNIT_TEST_COMMON_H

