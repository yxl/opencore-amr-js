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

//               S O R T E D   L I S T   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#ifndef __SORTED_LIST_H
#define __SORTED_LIST_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "oscl_mutex.h"


const int LIST_DEBUG_ENABLE = 1;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <class LLClass> class SortedList;

template <class LLClass> class SortedListElement
{

    public:
        SortedListElement(LLClass in_data)
        {
            data = in_data;
            next = NULL;
        };

        friend class SortedList<LLClass>;

    private:
        SortedListElement<LLClass>* next;
        LLClass data;
};


template <class LLClass> class SortedList
{

    public:
        SortedList();
        ~SortedList();

        int add_element(const LLClass& new_data);
        int remove_element(LLClass& data_to_remove);
        int remove_element(const int index_to_remove);
        int get_index(const LLClass& data) const;
        int get_num_elements() const
        {
            return num_elements;
        };
        int get_element(LLClass& element) const ;
        int get_element(int index, LLClass& element) const;


        int dequeue_pre_element()
        {

        }

        int get_first(LLClass & element)
        {
            if (NULL == head) return 0;
            iterator = head;
            element = head->data;

            return 1;
        }

        int get_next(LLClass & element)
        {
            if (tail == iterator)
            {
                return 0;
            }
            if (!iterator)
            {
                if (!head) return 0;
                iterator = head;
            }
            else
            {
                iterator = iterator->next;
            }
            element = iterator->data;
            return 1;
        }




    private:
        SortedListElement<LLClass> *head;
        SortedListElement<LLClass> *tail;
        SortedListElement<LLClass> *iterator;

        int num_elements;

        int check_list();
};


template <class LLClass> SortedList<LLClass>::SortedList()
{
    num_elements = 0;
    iterator = head = tail = NULL;
}

template <class LLClass> SortedList<LLClass>::~SortedList()
{
    SortedListElement<LLClass>* tmp;
    while (num_elements && head)
    {
        tmp = head->next;
        OSCL_DELETE(head);
        --num_elements;
        head = tmp;
    }
    head = tail = NULL;
}


template <class LLClass> int SortedList<LLClass>::check_list()
{

    SortedListElement<LLClass> *tmp;
    int ii;

    for (tmp = head, ii = 0; tmp ; ++ii, tmp = tmp->next);

    if (ii != num_elements)
    {
        // bark - number of elements (num_elements) does not match first null (ii)
    }


    return (ii == num_elements);


}


template <class LLClass> int SortedList<LLClass>::add_element(const LLClass& new_element)
{
    if (!tail)
    {
        // empty so just add it at the end
        head = tail = OSCL_NEW(SortedListElement<LLClass>, (new_element));
    }
    else
    {
        // find the proper location in the list
        SortedListElement<LLClass> *prev;
        SortedListElement<LLClass> *cur;
        int ii;
        for (prev = NULL, cur = head, ii = 0; ii < num_elements && cur;
                prev = cur, cur = cur->next, ++ii)
        {
            if (new_element <= cur->data)
            {
                // add at the current location
                if (prev)
                {
                    prev->next = OSCL_NEW(SortedListElement<LLClass>, (new_element));
                    prev->next->next = cur;
                }
                else
                {
                    // this is the new head
                    prev = OSCL_NEW(SortedListElement<LLClass>, (new_element));
                    prev->next = head;
                    head = prev;
                }
                break;
            }
        }

        if (ii >= num_elements)
        {
            // new element goes at the end
            tail->next = OSCL_NEW(SortedListElement<LLClass>, (new_element));
            tail = tail->next;
        }
    }

    ++num_elements;
    return 1;
}


template <class LLClass> int SortedList<LLClass>::get_element(int index, LLClass& element) const
{

    SortedListElement<LLClass> *tmp;
    int ii;

    if (index < 0 || index >= num_elements)
    {
        return 0;
    }

    for (tmp = head, ii = 0; ii < index; ++ii, tmp = tmp->next);

    element = tmp->data;
    return 1;
}


template <class LLClass> int SortedList<LLClass>::get_element(LLClass& element) const
{

    SortedListElement<LLClass> *tmp;
    int ii;
    int found = 0;

    for (tmp = head, ii = 0; ii < num_elements && tmp; ++ii, tmp = tmp->next)
    {
        if (element == tmp->data)
        {
            found  = 1;
            break;
        }
    }
    return found;
}

template <class LLClass> int SortedList<LLClass>::remove_element(LLClass& data_to_remove)
{
    SortedListElement<LLClass> *tmp;
    SortedListElement<LLClass> *prev;
    int found = 0;
    int idx = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = tmp->next)
    {

        if (tmp->data == data_to_remove)
        {
            found = 1;
            if (prev)
            {
                prev->next = tmp->next;
                if (iterator == tmp) iterator = prev;
            }
            else
            {
                head = tmp->next;
                if (iterator == tmp) iterator = NULL;
            }
            if (tmp == tail)
            {
                tail = prev;
            }


            OSCL_DELETE(tmp);
            --num_elements;
            break;
        }
        else if (tmp->data > data_to_remove)
        {
            // not in the list
            break;
        }
        idx++;
    } // of for loop
    return found;
}


template <class LLClass> int SortedList<LLClass>::get_index(const LLClass& data) const
{
    SortedListElement<LLClass> *tmp;
    int index = 0;
    int found = 0;

    for (tmp = head, index = 0; tmp; tmp = tmp->next, ++index)
    {

        if (tmp->data == data)
        {
            found = 1;
            break;
        }
        else if (tmp->data > data)
        {
            // not found
            break;
        }
    }
    if (found)
        return index;

    return -1;
}



template <class LLClass> int SortedList<LLClass>::remove_element(const int index_to_remove)
{
    SortedListElement<LLClass> *tmp;
    SortedListElement<LLClass> *prev;
    int ii;

    if (index_to_remove < 0 || index_to_remove >= num_elements)
    {
        return 0;
    }

    // skip to desired element
    for (tmp = head, prev = NULL, ii = 0; tmp && ii < index_to_remove;
            ++ii, prev = tmp, tmp = tmp->next);

    if (ii != index_to_remove)
    {
        return 0;
    }

    if (prev)
    {
        prev->next = tmp->next;
        if (iterator == tmp) iterator = prev;
    }
    else
    {
        head = tmp->next;
        if (iterator == tmp) iterator = NULL;
    }


    if (tmp == tail)
    {
        tail = prev;
    }


    OSCL_DELETE(tmp);
    --num_elements;


    return 1;
}


// MTSortedList is a multi-threaded version of
// the SortedList.  It has mutex protection to
// allow access by different threads.


template <class LLClass> class MTSortedList
{
    public:

        MTSortedList() {};
        ~MTSortedList() {};

        int add_element(const LLClass& new_data);
        int remove_element(const LLClass& data_to_remove);
        int remove_element(const int index_to_remove);
        int get_index(const LLClass& data) const ;
        int get_num_elements() const
        {
            return the_list.get_num_elements();
        };
        int get_element(LLClass& element) const;
        int get_element(int index, LLClass& element) const ;


    private:
        SortedList<LLClass> the_list;
        PVMutex mutex;

};



template <class LLClass> int MTSortedList<LLClass>::add_element(const LLClass& new_element)
{
    int status;
    mutex.Lock();
    status = the_list.add_element(new_element);
    mutex.Unlock();
    return status;
}

template <class LLClass> int MTSortedList<LLClass>::remove_element(const LLClass& data_to_remove)
{
    int status;

    mutex.Lock();
    status = the_list.remove_element(data_to_remove);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTSortedList<LLClass>::remove_element(const int index_to_remove)
{
    int status;
    mutex.Lock();
    status = the_list.remove_element(index_to_remove);
    mutex.Unlock();
    return status;
}

template <class LLClass> int MTSortedList<LLClass>::get_index(const LLClass& data) const
{
    int status;
    mutex.Lock();
    status = the_list.get_index(data);
    mutex.Unlock();
    return status;
}

template <class LLClass> int MTSortedList<LLClass>::get_element(LLClass& element) const
{

    int status;
    mutex.Lock();
    status = the_list.get_element(element);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTSortedList<LLClass>::get_element(int index, LLClass& element) const
{

    int status;
    mutex.Lock();
    status = the_list.get_element(index, element);
    mutex.Unlock();
    return status;
}











#endif  // __SORTED_LIST_H

