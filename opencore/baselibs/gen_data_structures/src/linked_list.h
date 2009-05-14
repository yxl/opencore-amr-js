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

//               L I N K E D   L I S T   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "oscl_mutex.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <class LLClass> class LinkedList;

template <class LLClass> class LinkedListElement
{

    public:
        LinkedListElement(LLClass in_data)
        {
            data = in_data;
            next = NULL;
        };
        //  ~LinkedListElement() {};

        friend class LinkedList<LLClass>;

    private:
        LinkedListElement<LLClass>* next;
        LLClass data;
};


template <class LLClass> class LinkedList
{

    public:
        LinkedList();
        ~LinkedList();

        int add_element(LLClass& new_data);
        int add_to_front(const LLClass& new_data);
        int add_to_end(const LLClass& new_data);
        int remove_element(const LLClass& data_to_remove);
        int remove_element(const int index_to_remove);
        int move_to_end(const LLClass& data_to_move);
        int move_to_front(const LLClass& data_to_move);
        int get_num_elements()
        {
            return num_elements;
        };

        int get_element(int index, LLClass& element);
        int get_index(const LLClass& data);

        int dequeue_element(LLClass & element)
        {
            get_element(0, element);
            return remove_element(0);
        }
        // get_first() and get_next() together provide iterator function
        int get_first(LLClass & ele)
        {
            if (!head) return 0;
            iterator = head;
            ele = iterator->data;
            return 1;
        };

        int get_next(LLClass & ele)
        {
            if (iterator == tail) return 0;
            if (! iterator)
            {
                if (!head) return 0;
                iterator = head;
            }
            else
            {
                iterator = iterator->next;
            }
            ele = iterator->data;

            return 1;
        };


    protected:
        LinkedListElement<LLClass> *head;
        LinkedListElement<LLClass> *tail;
        LinkedListElement<LLClass> *iterator;
        int num_elements;

        int check_list();
};


template <class LLClass> LinkedList<LLClass>::LinkedList()
{
    num_elements = 0;
    head = tail = iterator = NULL;
}

template <class LLClass> LinkedList<LLClass>::~LinkedList()
{
    LinkedListElement<LLClass>* tmp;
    while (num_elements && head)
    {
        tmp = head->next;
        OSCL_DELETE(head);
        --num_elements;
        head = tmp;
    }
    head = tail = iterator = NULL;
}


template <class LLClass> int LinkedList<LLClass>::check_list()
{
    LinkedListElement<LLClass> *tmp;
    int ii;

    for (tmp = head, ii = 0; tmp ; ++ii, tmp = tmp->next);

    return (ii == num_elements);
}


template <class LLClass> int LinkedList<LLClass>::add_element(LLClass& new_element)
{
    if (!tail)
    {
        head = tail = OSCL_NEW(LinkedListElement<LLClass>, (new_element));
    }
    else
    {
        tail->next = OSCL_NEW(LinkedListElement<LLClass>, (new_element));
        tail = tail->next;
    }

    ++num_elements;
    return 1;
}

template <class LLClass> int LinkedList<LLClass>::add_to_front(const LLClass& new_element)
{
    if (!head)
    {
        head = tail = OSCL_NEW(LinkedListElement<LLClass>, (new_element));
    }
    else
    {
        LinkedListElement<LLClass>* tmp;
        tmp = OSCL_NEW(LinkedListElement<LLClass>, (new_element));
        tmp->next = head;
        head = tmp;
    }

    ++num_elements;
    return 1;
}

template <class LLClass> int LinkedList<LLClass>::get_element(int index, LLClass& element)
{

    LinkedListElement<LLClass> *tmp;
    int ii;

    if (index < 0 || index >= num_elements)
    {
        return 0;
    }

    for (tmp = head, ii = 0; ii < index; ++ii, tmp = tmp->next);

    element = tmp->data;
    return 1;
}


template <class LLClass> int LinkedList<LLClass>::remove_element(const LLClass& data_to_remove)
{
    LinkedListElement<LLClass> *tmp;
    LinkedListElement<LLClass> *prev;
    int found = 0;

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

    }
    return found;
}


template <class LLClass> int LinkedList<LLClass>::get_index(const LLClass& data)
{
    LinkedListElement<LLClass> *tmp;
    int index = 0;
    int found = 0;

    for (tmp = head, index = 0; tmp; tmp = tmp->next, ++index)
    {

        if (tmp->data == data)
        {
            found = 1;
            break;
        }
    }
    if (found)
        return index;

    return -1;
}



template <class LLClass> int LinkedList<LLClass>::remove_element(const int index_to_remove)
{
    LinkedListElement<LLClass> *tmp;
    LinkedListElement<LLClass> *prev;
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


template <class LLClass> int LinkedList<LLClass>::move_to_end(const LLClass& data_to_move)
{
    LinkedListElement<LLClass> *tmp;
    LinkedListElement<LLClass> *prev;
    int found = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = tmp->next)
    {

        if (tmp->data == data_to_move)
        {
            found = 1;
            if (tmp == tail)
            {
                return 1; // nothing to do
            }
            if (prev)
            {
                prev->next = tmp->next;
                if (iterator == tmp) iterator = prev;
            }
            if (tmp == head)
            {
                head = tmp->next;
                if (iterator == tmp) iterator = NULL;
            }
            tail->next = tmp;
            tmp->next = NULL;
            tail = tmp;

            return 1;
        }
    }

    return 0;
}


template <class LLClass> int LinkedList<LLClass>::move_to_front(const LLClass& data_to_move)
{
    LinkedListElement<LLClass> *tmp;
    LinkedListElement<LLClass> *prev;
    int found = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = tmp->next)
    {

        if (tmp->data == data_to_move)
        {
            found = 1;
            if (tmp == head)
            {
                return 1; // nothing to do
            }

            if (prev)
            {
                prev->next = tmp->next;

                if (iterator == tmp)
                {
                    iterator = prev;
                }
            }

            if (tmp == tail)
            {
                tail = prev;
            }
            tmp->next = head;
            head = tmp;


            return 1;
        }
    }

    return 0;
}


// MTLinkedList is a multi-threaded version of
// the LinkedList.  It has mutex protection to
// allow access by different threads.

template <class LLClass> class MTLinkedList
{

    public:
        MTLinkedList() {};
        ~MTLinkedList() {};


        int add_element(LLClass& new_data);
        int add_to_front(LLClass& new_data);
        int remove_element(const LLClass& data_to_remove);
        int remove_element(const int index_to_remove);
        int move_to_end(const LLClass& data_to_move);
        int move_to_front(const LLClass& data_to_move);
        int get_num_elements()
        {
            return the_list.get_num_elements();
        };

        int get_element(int index, LLClass& element);
        int get_index(const LLClass& data);

        int dequeue_element(LLClass & element)
        {
            int status;
            mutex.Lock();
            status = the_list.dequeue_element(element);
            mutex.Unlock();
            return status;
        }


    protected:
        LinkedList<LLClass> the_list;
        PVMutex mutex;


};



template <class LLClass> int MTLinkedList<LLClass>::add_element(LLClass& new_element)
{
    int status;
    mutex.Lock();
    status = the_list.add_element(new_element);
    mutex.Unlock();
    return status;
}

template <class LLClass> int MTLinkedList<LLClass>::add_to_front(LLClass& new_element)
{
    int status;
    mutex.Lock();
    status = the_list.add_to_front(new_element);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTLinkedList<LLClass>::get_element(int index, LLClass& element)
{

    int status;
    mutex.Lock();
    status = the_list.get_element(index, element);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTLinkedList<LLClass>::remove_element(const LLClass& data_to_remove)
{
    int status;

    mutex.Lock();
    status = the_list.remove_element(data_to_remove);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTLinkedList<LLClass>::get_index(const LLClass& data)
{
    int status;
    mutex.Lock();
    status = the_list.get_index(data);
    mutex.Unlock();
    return status;
}



template <class LLClass> int MTLinkedList<LLClass>::remove_element(const int index_to_remove)
{
    int status;
    mutex.Lock();
    status = the_list.remove_element(index_to_remove);
    mutex.Unlock();
    return status;
}


template <class LLClass> int MTLinkedList<LLClass>::move_to_end(const LLClass& data_to_move)
{
    int status;
    mutex.Lock();
    status = the_list.move_to_end(data_to_move);
    mutex.Unlock();
    return status;
}

template <class LLClass> int MTLinkedList<LLClass>::move_to_front(const LLClass& data_to_move)
{
    int status;
    mutex.Lock();
    status = the_list.move_to_front(data_to_move);
    mutex.Unlock();
    return status;
}



#endif  // __LINKED_LIST_H

