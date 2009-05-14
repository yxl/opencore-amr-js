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
#ifndef UNIT_TEST_VECTOR_H
#define UNIT_TEST_VECTOR_H


template<class T, class Alloc>
class UnitTest_TAlloc
{
    public:
        typedef T           value_type;
        typedef T           * pointer;
        typedef const T     * const_pointer;
        typedef uint32      size_type;
        typedef T&			reference;
        typedef const T&	const_reference;

        virtual ~UnitTest_TAlloc() {};

        pointer allocate(uint32 size)
        {
            OsclAny* tmp = alloc.allocate(size * sizeof(value_type));
            return (pointer)tmp;;
        }

        pointer alloc_and_construct(const_reference val)
        {
            OsclAny* tmp = alloc.allocate(sizeof(value_type));
            construct((pointer)tmp, val);
            return (pointer)tmp;
        }

        void deallocate(void* p)
        {
            alloc.deallocate(p);
        }

        void deallocate(void* p, size_type)
        {
            alloc.deallocate(p);
        }

        void destruct_and_dealloc(void* p)
        {
            destroy((pointer)p);
            deallocate(p);
        }

        pointer address(reference r)
        {
            return &r;
        }
        const_pointer address(const_reference r) const
        {
            return &r;
        }

        void construct(pointer p, const_reference val)
        {
            new(p) T(val);
        }
        void destroy(pointer p)
        {
            p->~T();
        }

    private:
        Alloc alloc;
};

template<class T, class Alloc>
class UnitTest_Vector
{

    public:
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef uint32 size_type;
        typedef int32 difference_type;
        typedef T* iterator;
        typedef const T* const_iterator;

        /**
         * Creates an empty vector.
         */
        UnitTest_Vector() :
                elems(0), numelems(0), bufsize(0)
        {}

        /**
         * Creates an empty vector with capacity n.
         * @param n creates a vector with n elements.  The main reason for specifying n
         * is efficiency.  If you know the capacity to which your vector must grow, then
         * it is more efficient to allocate the vector all at once rather than rely on
         * the automatic reallocation scheme.  This also helps cotrol the invalidation
         * of iterators.
         */
        UnitTest_Vector(uint32 n) :
                numelems(0), bufsize(n)
        {
            elems = defAlloc.allocate(bufsize);
        }

        /**
         * Copy Constructor.
         * @param x vector class to copy.
         */
        UnitTest_Vector(const UnitTest_Vector<T, Alloc>& x)
        {
            numelems = x.numelems;
            bufsize = x.numelems; // only allocate enough for current elements
            elems = defAlloc.allocate(bufsize);
            uninitialized_copy(x.begin(), x.end(), begin());
        }

        /**
         * The destructor.
         */
        virtual ~UnitTest_Vector()
        {
            if (elems)
            {
                destroy(begin(), end());
                defAlloc.deallocate(elems);
            }
        }

        /**
         * The assignment operator
         */
        UnitTest_Vector<T, Alloc>& operator=(UnitTest_Vector<T, Alloc>& x)
        {
            if (&x != this)
            {
                if (x.size() > capacity())
                {
                    // allocate space and copy
                    T* tmp = defAlloc.allocate(x.end() - x.begin());
                    uninitialized_copy(x.begin(), x.end(), tmp);
                    destroy(begin(), end());
                    defAlloc.deallocate(elems);
                    elems = tmp;
                    bufsize = x.size();
                }
                else if (size() >= x.size())
                {
                    iterator i = copy(x.begin(), x.end(), begin());
                    destroy(i, end());
                }
                else
                {
                    copy(x.begin(), x.begin() + size(), begin());
                    uninitialized_copy(x.begin() + size(), x.end(), end());
                }
                numelems = x.size();
            }
            return *this;
        }


        /**
         * Returns the size of the vector.
         */
        uint32 size() const
        {
            return numelems;
        }

        /**
         * Returns the allocated memory of the vector.
         */
        uint32 capacity() const
        {
            return bufsize;
        }

        /**
         * Reallocates memory if necessary to a capacity of n
         * The main reason for reserve is efficiency.  If you know the capacity to
         * which your vector must grow, then it is more efficient to allocate the
         * vector all at once rather than rely on the automatic reallocation scheme.
         * This also helps cotrol the invalidation of iterators.
         * @param n size of vector
         */
        void reserve(uint32 n)
        {
            if (n > bufsize)
            {
                T *oldelems = elems;
                elems = defAlloc.allocate(n);
                for (uint32 i = 0; i < numelems; i++)
                {
                    construct(&elems[i], oldelems[i]);
                }
                if (oldelems)
                {
                    destroy(oldelems, oldelems + numelems);
                    defAlloc.deallocate(oldelems);
                }
                bufsize = n;
            }
        }

        /**
         * True if the vector's size is 0.
         */
        bool empty() const
        {
            return numelems == 0;
        }

        /**
         * Inserts a new element at the end.
         * Inserting an element invalidates all iterators if memory reallocation occurs
         * as a result of the insertion.
         * @param x new element
         */
        void push_back(const T& x)
        {
            if (numelems == bufsize)
            {
                uint32 new_bufsize = (bufsize) ? 2 * bufsize : 2;
                reserve(new_bufsize);
            }
            construct(end(), x);
            numelems++;
        }


        /**
         * Returns the n'th element.
         * @param n element position to return
         */
        T& operator[](uint32 n)
        {
            return (*(begin() + n));
        }

        /**
         * Returns the n'th element.
         * @param n element position to return
         */
        const T& operator[](uint32 n) const
        {
            return (*(begin() + n));
        }

        /**
         * Returns the first element.
         */
        T& front()
        {
            return *begin();
        }

        /**
         * Returns the first element.
         */
        const T& front() const
        {
            return *begin();
        }

        /**
         * Returns the last element.
         */
        T& back()
        {
            return (*(end() - 1));
        }

        /**
         * Returns the last element.
         */
        const T& back() const
        {
            return (*(end() - 1));
        }

        /**
         * Removes the last element.
         */
        void pop_back()
        {
            numelems--;
            destroy(end());
        }

        /**
         * Removes all elements.
         */
        void clear()
        {
            erase(begin(), end());
        }

        /**
         * Returns an iterator pointing to the beginning of the vector.
         */
        iterator begin() const
        {
            return elems;
        }

        /**
         * Returns an iterator pointing to the end of the vector..
         */
        iterator end() const
        {
            return elems + numelems;
        }

        /**
         * Erases the element pointed to by iterator pos.
         * Erasing an element invalidates all iterators pointing to elements
         * following the deletion point.
         * @param pos iterator at erase position
         */
        iterator erase(iterator pos)
        {
            if (pos + 1 != end()) copy(pos + 1, end(), pos);
            numelems--;
            destroy(end());
            return pos;
        }

        /**
         * Erases elements in range [first, last).
         * Erasing an element invalidates all iterators pointing to elements
         * following the deletion point.
         * @param first starting position
         * @param last ending position, this position is not erased
         */
        iterator erase(iterator first, iterator last)
        {
            iterator it = copy(last, end(), first);
            destroy(it, end());
            numelems -= (last - first);
            return first;
        }

    private:

        T *elems;
        uint32 numelems;
        uint32 bufsize;
        UnitTest_TAlloc<T, Alloc> defAlloc;

        void construct(pointer p, const_reference x)
        {
            new(p) value_type(x);
        }

        iterator copy(iterator first, iterator last, iterator result)
        {
            while (first != last) *result++ = *first++;
            return result;
        }
        iterator uninitialized_copy(iterator first, iterator last, iterator result)
        {
            while (first != last) construct(result++, *first++);
            return result;
        }

        void destroy(iterator first)
        {
            first->~T();
        }
        void destroy(iterator first, iterator last)
        {
            while (first != last)
            {
                first->~T();
                first++;
            }
        }
};

#endif //UNIT_TEST_VECTOR_H

