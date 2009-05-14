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

#ifndef BOOL_ARRAY_H
#define BOOL_ARRAY_H

#include "oscl_base.h"

template <uint32 max_array_size> class BoolArray
{

    private:

        static uint32 mask_array[33];



    public:

        BoolArray()
        {
            size = max_array_size;
            elements = (max_array_size + 31) / 32;
        };

        ~BoolArray() {};


        bool set_size(int in_size)
        {
            if (size <= 0 || size > (int) max_array_size)
            {
                return false;
            }
            size = in_size;
            return true;
        };


        bool set_range(bool value, int first_index = 0, int last_index = -1)
        {
            if (last_index < 0)
            {
                last_index = size - 1;
            };
            if (first_index < 0 || first_index > last_index || last_index >= size)
            {
                return false;
            }


            static const uint32 mask_array[33] =
            {
                0, 0x1, 0x3, 0x7,
                0xF, 0x1F, 0x3F, 0x7F,
                0xFF, 0x1FF, 0x3FF, 0x7FF,
                0xFFF, 0x1FFF, 0x3FFF, 0x7FFF,
                0xFFFF, 0x1FFFF, 0x3FFFF, 0x7FFFF,
                0xFFFFF, 0x1FFFFF, 0x3FFFFF, 0x7FFFFF,
                0xFFFFFF, 0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF,
                0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
                0xFFFFFFFF
            };


            int starting_word = first_index / 32;
            int ending_word = (last_index) / 32;

            for (int ii = starting_word; ii <= ending_word; ++ii)
            {

                if (ii == ending_word)
                {
                    // compute the word offset
                    int word_index = (last_index % 32) + 1;
                    uint32 mask = mask_array[word_index];
                    if (value)
                    {
                        array[ii] |= (mask);
                    }
                    else
                    {
                        array[ii] &= (~mask);
                    }
                }
                else if (ii == starting_word)
                {
                    // compute the word offset
                    int word_index = first_index % 32;
                    uint32 mask = mask_array[word_index];
                    if (value)
                    {
                        array[ii] |= (~mask);
                    }
                    else
                    {
                        array[ii] &= (mask);
                    }

                }
                else
                {
                    uint32 mask = mask_array[32];
                    if (value)
                    {
                        array[ii] |= (mask);
                    }
                    else
                    {
                        array[ii] &= (~mask);
                    }

                }

            }
            return true;
        }

        bool set_value(int index, bool value)
        {

            if (index < 0 || index >= size)
            {
                return false;
            }

            int array_index = (index) / 32;
            int word_index = index % 32;
            uint32 mask = 1 << word_index;

            if (value)
            {
                array[array_index] |= mask;
            }
            else
            {
                array[array_index] &= (~mask);
            }

            return true;
        }


        bool get_value(int index) const
        {
            if (index < 0 || index >= size)
            {
                return false;
            }

            int array_index = (index) / 32;
            int word_index = index % 32;
            uint32 mask = 1 << word_index;

            return ((array[array_index] & mask) != 0);
        }





        bool operator[](int index) const
        {
            return get_value(index);
        };

    private:
        int size;
        int elements;
        uint32 array[(max_array_size+31)/32];

};






#endif
