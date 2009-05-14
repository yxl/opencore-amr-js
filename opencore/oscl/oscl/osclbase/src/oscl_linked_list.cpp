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
#include "oscl_linked_list.h"



OSCL_EXPORT_REF void Oscl_Linked_List_Base::construct(Oscl_Opaque_Type_Alloc_LL* op)
{
    num_elements = 0;
    head = tail = iterator = NULL;
    pOpaqueType = op;
}

OSCL_EXPORT_REF void Oscl_Linked_List_Base::destroy()
{
    OsclAny* tmp;
    while (num_elements && head)
    {
        tmp = pOpaqueType->get_next(head);
        pOpaqueType->destroy(head);
        pOpaqueType->deallocate(head);
        --num_elements;
        head = tmp;
    }
    head = tail = iterator = NULL;
}

/**
 * Return the first element of list in passed parameter,
 * @param ele return the value of first element of list in this parameter
 * @return 32-bit interger,If first element found, it returns 1
 * otherwise it returns 0
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::get_first(OsclAny* ele)
{
    if (!head) return 0;
    iterator = head;
    pOpaqueType->get_data(iterator, ele);
    return 1;
};

/**
 * Return the next element of list in passed parameter,
 * @param ele return the value of next element of list in this parameter
 * @return 32-bit interger ,if next element is found in list,it returns 1
 * otherwise it returns 0
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::get_next(OsclAny* ele)
{
    if (iterator == tail) return 0;
    if (! iterator)
    {
        if (!head) return 0;
        iterator = head;
    }
    else
    {
        iterator = pOpaqueType->get_next(iterator);
    }
    pOpaqueType->get_data(iterator, ele);
    return 1;
};

/**
 * Debug routine: Checks the list for elements.
 * @return 32-bit integer, if node count is equal to number of node added
 * to the list then returns 1 otherwise returns 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::check_list()
{
    OsclAny *tmp;

    int32 ii;
    for (tmp = head, ii = 0; tmp ; ++ii)
        tmp = pOpaqueType->get_next(tmp);

    return (ii == num_elements);
}

/**
 * Adds new element to the list.if list is already there then it adds element at end of list otherwise
 * it create the list and add the element as first element of list.
 * @param new_element the element to be add in the list.
 * @return 32-bit integer on the success returns 1.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::add_element(OsclAny* new_element)
{
    if (!tail)
    {
        OsclAny* ptr = pOpaqueType->allocate(sizeof_T);
        pOpaqueType->construct(ptr, new_element);
        head = tail = ptr;
        if (head == NULL)
            return 0; // Memory Allocation Error
    }
    else
    {
        OsclAny* ptr = pOpaqueType->allocate(sizeof_T);
        pOpaqueType->construct(ptr, new_element);
        pOpaqueType->set_next(tail, ptr);
        if (pOpaqueType->get_next(tail) == NULL)
            return 0; // Memory Allocation Error
        tail = pOpaqueType->get_next(tail);
    }

    ++num_elements;
    return 1;
}

/**
 * Adds new element at the start of the list.if list is already
 * exist then it adds element at start of list otherwise
 * it create the list and add the element as first element of list.
 * @param new_element the element to be add in the list.
 * @return 32-bit integer on the success returns 1.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::add_to_front(const OsclAny* new_element)
{
    if (!head)
    {
        OsclAny* ptr = pOpaqueType->allocate(sizeof_T);
        pOpaqueType->construct(ptr, new_element);
        head = tail = ptr;
        if (head == NULL)
            return 0; // Memory Allocation Error
    }
    else
    {
        OsclAny* ptr = pOpaqueType->allocate(sizeof_T);
        pOpaqueType->construct(ptr, new_element);
        if (ptr == NULL)
            return 0; // Memory Allocation Error
        pOpaqueType->set_next(ptr, head);
        head = ptr;
    }
    ++num_elements;
    return 1;
}

/**
 * Search and returs the element in the list for passed index.
 * @param index, element The index is the count for the node.
 * @return 32-bit integer on success returns 1 otherwise returns 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::get_element(int32 index, OsclAny* element)
{

    OsclAny *tmp;
    int32 ii;

    if (index < 0 || index >= num_elements)
    {
        return 0;
    }

    for (tmp = head, ii = 0; ii < index; ++ii, tmp = pOpaqueType->get_next(tmp))
    {
    }

    pOpaqueType->get_data(tmp, element);
    return 1;
}

/**
 * Removes the element from the list.
 * @param data_to_remove
 * @return 32-bit integer on if  element fount in the list returns 1 otherwise returns 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::remove_element(const OsclAny* data_to_remove)
{
    OsclAny *tmp;
    OsclAny *prev;
    int32 found = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = pOpaqueType->get_next(tmp))
    {

        if (pOpaqueType->compare_data(tmp, data_to_remove))
        {
            found = 1;
            if (prev)
            {
                pOpaqueType->set_next(prev, pOpaqueType->get_next(tmp));
                if (iterator == tmp) iterator = prev;
            }
            else
            {
                head = pOpaqueType->get_next(tmp);
                if (iterator == tmp) iterator = NULL;
            }
            if (tmp == tail)
            {
                tail = prev;
            }
            pOpaqueType->destroy(tmp);
            pOpaqueType->deallocate(tmp);
            --num_elements;
            break;
        }

    }
    return found;
}

/**
 * Returns the index for requested element.
 * @param data the element for which index to be return.
 * @return 32-bit integer if data is found in the list it returns index
 * otherwise it returns -1.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::get_index(const OsclAny* data)
{
    OsclAny *tmp;
    int32 index = 0;
    int32 found = 0;

    for (tmp = head, index = 0; tmp; tmp = pOpaqueType->get_next(tmp), ++index)
    {

        if (pOpaqueType->compare_data(tmp, data))
        {
            found = 1;
            break;
        }
    }
    if (found)
        return index;

    return -1;
}

/**
 * Removes the element for requested index.
 * @param index_to_remove
 * @return on success return 1 otherwise return 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::remove_element(const int32 index_to_remove)
{
    OsclAny *tmp;
    OsclAny *prev;
    int32 ii;

    if (index_to_remove < 0 || index_to_remove >= num_elements)
    {
        return 0;
    }

    // skip to desired element
    for (tmp = head, prev = NULL, ii = 0; tmp && ii < index_to_remove;
            ++ii, prev = tmp, tmp = pOpaqueType->get_next(tmp))
    {
    }

    if (ii != index_to_remove)
    {
        return 0;
    }

    if (prev)
    {
        pOpaqueType->set_next(prev, pOpaqueType->get_next(tmp));
        if (iterator == tmp) iterator = prev;
    }
    else
    {
        head = pOpaqueType->get_next(tmp);
        if (iterator == tmp) iterator = NULL;
    }
    if (tmp == tail)
    {
        tail = prev;
    }

    pOpaqueType->destroy(tmp);
    pOpaqueType->deallocate(tmp);
    --num_elements;

    return 1;
}

/**
 * Moves the element to end of the list
 * @param data_to_move
 * @return On success returns 1 otherwise returns 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::move_to_end(const OsclAny* data_to_move)
{
    OsclAny *tmp;
    OsclAny *prev;
    uint32 found = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = pOpaqueType->get_next(tmp))
    {

        if (pOpaqueType->compare_data(tmp, data_to_move))
        {
            found = 1;
            if (tmp == tail)
            {
                return 1; // nothing to do
            }
            if (prev)
            {
                pOpaqueType->set_next(prev, pOpaqueType->get_next(tmp));
                if (iterator == tmp) iterator = prev;
            }
            if (tmp == head)
            {
                head = pOpaqueType->get_next(tmp);
                if (iterator == tmp) iterator = NULL;
            }
            pOpaqueType->set_next(tail, tmp);
            pOpaqueType->set_next(tmp, NULL);
            tail = tmp;

            return 1;
        }
    }

    return 0;
}

/**
 * Moves the element to front of the list
 * @param data_to_move
 * @return On success returns 1 otherwise returns 0.
 */
OSCL_EXPORT_REF int32 Oscl_Linked_List_Base::move_to_front(const OsclAny* data_to_move)
{
    OsclAny *tmp;
    OsclAny *prev;
    int32 found = 0;

    for (tmp = head, prev = NULL; tmp; prev = tmp, tmp = pOpaqueType->get_next(tmp))
    {

        if (pOpaqueType->compare_data(tmp, data_to_move))
        {
            found = 1;
            if (tmp == head)
            {
                return 1; // nothing to do
            }

            if (prev)
            {
                pOpaqueType->set_next(prev, pOpaqueType->get_next(tmp));

                if (iterator == tmp)
                {
                    iterator = prev;
                }
            }

            if (tmp == tail)
            {
                tail = prev;
            }
            pOpaqueType->set_next(tmp, head);
            head = tmp;

            return 1;
        }
    }

    return 0;
}



