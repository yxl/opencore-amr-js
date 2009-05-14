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

//               H A S H T A B L E   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#ifndef HASHTABLE_H
#define HASHTABLE_H


// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "sorted_list.h"
#include "hash_functions.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <class keyclass, class hashclass> struct HashListElement;

template <class keyclass, class hashclass> int
operator==(const HashListElement<keyclass, hashclass>& a,
           const HashListElement<keyclass, hashclass>& b)
{
    return ((a.fullkey == b.fullkey) && (a.data == b.data));
}

template <class keyclass, class hashclass> struct HashListElement
{
    keyclass fullkey;
    hashclass data;

// #if defined( PV_OS_HPUX )
// friend int operator==
//          ( HashListElement<keyclass,hashclass> &a,
//           HashListElement<keyclass,hashclass> &b );
// #else
    friend int operator==<keyclass, hashclass>
    (const HashListElement<keyclass, hashclass> & a,
     const HashListElement<keyclass, hashclass> & b);
// #endif

};

/*
template <class keyclass, class hashclass> int
operator==( const HashListElement<keyclass,hashclass>& a,
            const HashListElement<keyclass,hashclass>& b)
{
  return ((a.fullkey == b.fullkey) && (a.data == b.data));
}
*/

template <class keyclass, class hashclass> int
operator<(const HashListElement<keyclass, hashclass>& a,
          const HashListElement<keyclass, hashclass>& b)
{
    return ((a.fullkey < b.fullkey));
}

template <class keyclass, class hashclass> int
operator<=(const HashListElement<keyclass, hashclass>& a,
           const HashListElement<keyclass, hashclass>& b)
{
    return ((a.fullkey <= b.fullkey));
}

template <class keyclass, class hashclass> int
operator>(const HashListElement<keyclass, hashclass>& a,
          const HashListElement<keyclass, hashclass>& b)
{
    return ((a.fullkey > b.fullkey));
}

template <class keyclass, class hashclass> int
operator>=(const HashListElement<keyclass, hashclass>& a,
           const HashListElement<keyclass, hashclass>& b)
{
    return ((a.fullkey >= b.fullkey));
}


template <class keyclass, class hashclass> class HashTable
{
    public:
        HashTable(int in_hash_table_size);

        ~HashTable();

        int is_unique(const keyclass& fullkey);
        int add_element(const keyclass& fullkey, const hashclass& data);
        int add_element(uint32 key, const keyclass& fullkey, const hashclass& data);
        int remove_element(uint32 key, const keyclass& fullkey);
        int remove_element(const keyclass& fullkey);
        int get_element(uint32 key, const keyclass& fullkey, hashclass& data);
        int get_element(const keyclass& fullkey, hashclass& data);
        int get_element(int index, uint32& key, keyclass& fullkey,
                        hashclass& data);
        int get_num_elements()
        {
            return num_elements;
        };

    private:
        SortedList<HashListElement<keyclass, hashclass> >* hash_table;

        uint32 hash_mask;
        uint32 hash_table_size;
        int num_elements;

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass> int HashTable<keyclass, hashclass>::is_unique(const keyclass& key)
{
    HashListElement<keyclass, hashclass> tmp;
    uint32 hash = compute_hash(key);
    uint32 masked_hash = hash & hash_mask;

    int list_elements = hash_table[masked_hash].get_num_elements();

    for (int ii = 0; ii < list_elements; ++ii)
    {
        hash_table[masked_hash].get_element(ii, tmp);
//    if (tmp.key == key)
        if (tmp.fullkey == key)
            return 0;
    }

    return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::add_element(const keyclass& fullkey,
        const hashclass& data)
{
    uint32 key = compute_hash(fullkey);

    return add_element(key, fullkey, data);

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::add_element(uint32 key,
        const keyclass& fullkey,
        const hashclass& data)
{
    HashListElement<keyclass, hashclass> tmp;

    uint32 masked_key = key & hash_mask;
    tmp.fullkey = fullkey;
    tmp.data = data;

    hashclass tmp2;

    if (get_element(fullkey, tmp2))
    {
        // bark - hash table corruption

        //#define HASH_CORRUPTION_DEBUG 1
        return 0;
    }

    hash_table[masked_key].add_element(tmp);


    ++num_elements;

    return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::remove_element(const keyclass& fullkey)
{
    uint32 key = compute_hash(fullkey);
    return remove_element(key, fullkey);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::remove_element(uint32 key,
        const keyclass& fullkey)
{
    HashListElement<keyclass, hashclass> tmp;
    uint32 masked_key = key & hash_mask;

    int list_elements = hash_table[masked_key].get_num_elements();

    for (int ii = 0; ii < list_elements; ++ii)
    {
        hash_table[masked_key].get_element(ii, tmp);
        if (tmp.fullkey == fullkey)
        {
            hash_table[masked_key].remove_element(tmp);
            --num_elements;
            return 1;
        }
        else if (tmp.fullkey > fullkey)
        {
            return 0;
        }

    }

    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::get_element(const keyclass& fullkey,
        hashclass& data)
{
    uint32 key = compute_hash(fullkey);

    return get_element(key, fullkey, data);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::get_element(uint32 key,
        const keyclass& fullkey,
        hashclass& data)
{
    HashListElement<keyclass, hashclass> tmp;
    uint32 masked_key = key & hash_mask;

    int list_elements = hash_table[masked_key].get_num_elements();

    for (int ii = 0; ii < list_elements; ++ii)
    {
        hash_table[masked_key].get_element(ii, tmp);
        if (tmp.fullkey == fullkey)
        {
            data = tmp.data;
            return 1;
        }
        else if (tmp.fullkey > fullkey)
        {
            return 0;
        }
    }

    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int HashTable<keyclass, hashclass>::get_element(int index,
        uint32& key, keyclass& fullkey,
        hashclass& data)
{
    HashListElement<keyclass, hashclass> tmp;
    unsigned int hash_idx;

    if (index < 0 || index >= num_elements)
        return 0;

    // find the proper hash element
    for (hash_idx = 0; hash_idx < hash_table_size; ++hash_idx)
    {
        if (index < hash_table[hash_idx].get_num_elements())
            break;
        index -= hash_table[hash_idx].get_num_elements();
    }

    assert(index >= 0);

    if (!hash_table[hash_idx].get_element(index, tmp))
        return 0;

    data = tmp.data;
    key = compute_hash(tmp.fullkey);
    fullkey = tmp.fullkey;

    return 1;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass> HashTable<keyclass, hashclass>::HashTable(int in_hash_table_size)
{
    hash_mask = in_hash_table_size - 1;
    hash_table_size = in_hash_table_size;

    if (hash_mask & in_hash_table_size)
    {
        int tmp, cnt;
        // find next larger power of two
        for (tmp = in_hash_table_size, cnt = 0; tmp; tmp >>= 1, ++cnt);

        hash_table_size = (1 << cnt);
        hash_mask = hash_table_size - 1;

        // bark - hash table size is not a power of two, changing

    }

    hash_table = new SortedList< HashListElement<keyclass, hashclass> >[hash_table_size];

    num_elements = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass> HashTable<keyclass, hashclass>::~HashTable()
{
    delete[] hash_table;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -




template <class keyclass, class hashclass> class MTHashTable
{
    public:
        MTHashTable(int in_hash_table_size);

        ~MTHashTable();

        int is_unique(const keyclass& fullkey);
        int add_element(const keyclass& fullkey, const hashclass& data);
        int add_element(uint32 key, const keyclass& fullkey, const hashclass& data);
        int remove_element(uint32 key, const keyclass& fullkey);
        int remove_element(const keyclass& fullkey);
        int get_element(uint32 key, const keyclass& fullkey, hashclass& data);
        int get_element(const keyclass& fullkey, hashclass& data);
        int get_element(int index, uint32& key, keyclass& fullkey,
                        hashclass& data);
        int get_num_elements()
        {
            if (the_hash)
            {
                return the_hash->get_num_elements();
            }
            return 0;
        };

    private:
        HashTable<keyclass, hashclass> *the_hash;
        PVMutex mutex;
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass> int MTHashTable<keyclass, hashclass>::is_unique(const keyclass& key)
{
    int status;
    mutex.Lock();
    status = the_hash->is_unique(key);
    mutex.Unlock();
    return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::add_element(const keyclass& fullkey,
        const hashclass& data)
{
    int status;
    mutex.Lock();
    status = the_hash->add_element(fullkey, data);
    mutex.Unlock();
    return status;

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::add_element(uint32 key,
        const keyclass& fullkey,
        const hashclass& data)
{
    int status;
    mutex.Lock();
    status = the_hash->add_element(key, fullkey, data);
    mutex.Unlock();
    return status;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::remove_element(const keyclass& fullkey)
{
    int status;
    mutex.Lock();
    status = the_hash->remove_element(fullkey);
    mutex.Unlock();
    return status;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::remove_element(uint32 key,
        const keyclass& fullkey)
{
    int status;
    mutex.Lock();
    status = the_hash->remove_element(key, fullkey);
    mutex.Unlock();
    return status;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::get_element(const keyclass& fullkey,
        hashclass& data)
{
    int status;
    mutex.Lock();
    status = the_hash->get_element(fullkey, data);
    mutex.Unlock();
    return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::get_element(uint32 key,
        const keyclass& fullkey,
        hashclass& data)
{
    int status;
    mutex.Lock();
    status = the_hash->get_element(key, fullkey, data);
    mutex.Unlock();
    return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass>
int MTHashTable<keyclass, hashclass>::get_element(int index,
        uint32& key, keyclass& fullkey,
        hashclass& data)
{
    int status;
    mutex.Lock();
    status = the_hash->get_element(index, key, fullkey, data);
    mutex.Unlock();
    return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <class keyclass, class hashclass> MTHashTable<keyclass, hashclass>::MTHashTable(int in_hash_table_size)
{
    the_hash = OSCL_NEW(HashTable<keyclass, hashclass>, (in_hash_table_size));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


template <class keyclass, class hashclass> MTHashTable<keyclass, hashclass>::~MTHashTable()
{
    OSCL_DELETE(the_hash);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



#endif // HASHTABLE_H
