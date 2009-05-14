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
#ifndef VIRTUAL_BUFFER_H_INCLUDED
#define VIRTUAL_BUFFER_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef OSCL_ASSERT_H_INCLUDED
#include "oscl_assert.h"
#endif

template<class TBufferAccess>
class VirtualBuffer
{
    public:
        VirtualBuffer();
        VirtualBuffer(const Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator>& buffers);
        virtual ~VirtualBuffer();

        // copy constructor
//    VirtualBuffer<TBufferAccess>(const VirtualBuffer<TBufferAccess>& original);


        VirtualBuffer<TBufferAccess>(const VirtualBuffer<TBufferAccess>& original)
        {
            pCurrFrag = NULL;
            pCurrFragPos = NULL;
            currIndex = 0;

            for (uint i = 0; i < original.vFragments.size(); i++)
            {
                appendBuffer(original.vFragments[i]);
            }

            currIndex = original.currIndex;
            pCurrFrag = &vFragments[currIndex];
            pCurrFragPos = original.pCurrFragPos;
        }



        // buffer operations
        void appendBuffer(const OsclRefCounterMemFrag& frag);
        void reset();

        // operators
        TBufferAccess& operator*();                         // indirection
        VirtualBuffer& operator=(const VirtualBuffer&);     // assignment
        operator TBufferAccess*()
        {
            return pCurrFragPos;
        }; 	// cast
        VirtualBuffer& operator++();                        // prefix increment
        VirtualBuffer operator++(int);                      // postfix increment
        VirtualBuffer& operator+=(const int increment);     // addition

        // create a new virtual buffer from the current position
        // for the specified range
        void createSubBuffer(uint length, VirtualBuffer<TBufferAccess>& subRange);

        // create a new virtual buffer from the current position
        void createSubBuffer(VirtualBuffer<TBufferAccess>& subRange);

        // copy fragments into vector
        void copy(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator>& vOutFrags);

        // fragment vector accessor
        const Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator>& fragments()
        {
            return vFragments;
        }

        // return the virtual length
        uint length();

    protected:
        void advance();
        void setCurrPointer()
        {
            pCurrFragPos = reinterpret_cast<TBufferAccess*>(pCurrFrag->getMemFragPtr());
        }

        Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> vFragments;
        OsclRefCounterMemFrag* pCurrFrag;
        int currIndex;
        TBufferAccess* pCurrFragPos;
};


template<class TBufferAccess>
VirtualBuffer<TBufferAccess>::VirtualBuffer()
{
    pCurrFrag = NULL;
    pCurrFragPos = NULL;
    currIndex = 0;
}


template<class TBufferAccess>
VirtualBuffer<TBufferAccess>::VirtualBuffer(const Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator>& buffers)
{
    OSCL_ASSERT(buffers.size() > 0);

    pCurrFrag = NULL;
    pCurrFragPos = NULL;
    currIndex = 0;

    for (uint i = 0; i < buffers.size(); i++)
    {
        appendBuffer(buffers[i]);
    }
}

template<class TBufferAccess>
VirtualBuffer<TBufferAccess>::~VirtualBuffer()
{
}

template<class TBufferAccess>
VirtualBuffer<TBufferAccess>& VirtualBuffer<TBufferAccess>::operator=(const VirtualBuffer & original)
{
    if (&original != this)
    {
        vFragments.clear();
        for (uint i = 0; i < original.vFragments.size(); i++)
        {
            appendBuffer(original.vFragments[i]);
        }

        currIndex = original.currIndex;
        pCurrFrag = &vFragments[currIndex];
        pCurrFragPos = original.pCurrFragPos;
    }

    return *this;
}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::appendBuffer(const OsclRefCounterMemFrag& frag)
{
    vFragments.push_back(frag);
    if (pCurrFrag == NULL)
    {
        currIndex = 0;
        pCurrFrag = &vFragments[currIndex];

        setCurrPointer();
    }
    else
    {
        pCurrFrag = &vFragments[currIndex];
    }

}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::reset()
{
    pCurrFrag = NULL;
    currIndex = 0;
    vFragments.clear();
}

template<class TBufferAccess>
TBufferAccess& VirtualBuffer<TBufferAccess>::operator*()
{
    OSCL_ASSERT(pCurrFragPos != NULL);
    return *pCurrFragPos;
}

template<class TBufferAccess>
VirtualBuffer<TBufferAccess>& VirtualBuffer<TBufferAccess>::operator++()
{
    advance();
    return *this;
}

template<class TBufferAccess>
VirtualBuffer<TBufferAccess> VirtualBuffer<TBufferAccess>::operator++(int)
{
    VirtualBuffer<TBufferAccess> temp(*this);
    advance();
    return temp;
}

template<class TBufferAccess>
VirtualBuffer<TBufferAccess>& VirtualBuffer<TBufferAccess>::operator+=(const int increment)
{
    for (int i = 0; i < increment; i++)
    {
        advance();
    }

    return *this;
}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::createSubBuffer(uint length,
        VirtualBuffer<TBufferAccess>& subRange)
{
    subRange.reset();

    uint bytesNeeded = length;

    TBufferAccess* position = pCurrFragPos;
    OsclRefCounterMemFrag* pFragment = pCurrFrag;

    while (1 && pCurrFrag != NULL)
    {
        OsclRefCounterMemFrag frag(*pFragment);
        frag.getMemFrag().ptr = position;
        uint32 bytesInFrag = ((TBufferAccess*)pFragment->getMemFragPtr() + pFragment->getMemFragSize()) - position;
        frag.getMemFrag().len = bytesInFrag > bytesNeeded ? bytesNeeded : bytesInFrag;
        subRange.appendBuffer(frag);

        bytesNeeded -= frag.getMemFrag().len;

        if (bytesNeeded == 0)
        {
            break;
        }

        pFragment++;

        if (pFragment != vFragments.end())
        {
            // move position to the front of the next buffer
            position = reinterpret_cast<TBufferAccess*>(pFragment->getMemFragPtr());
        }
        else
        {
            // ran out of data
            break;
        }
    }
}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::copy(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator>& vOutFrags)
{
    for (uint i = 0; i < vFragments.size(); i++)
    {
        vOutFrags.push_back(vFragments[i]);
    }
}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::createSubBuffer(VirtualBuffer<TBufferAccess>& subRange)
{
    subRange.reset();
    TBufferAccess* position = pCurrFragPos;
    OsclRefCounterMemFrag* pFragment = pCurrFrag;

    if (pCurrFrag == NULL)
    {
        //no fragment left
        return;
    }

    while (1)
    {
        OsclRefCounterMemFrag frag(*pFragment);
        frag.getMemFrag().ptr = position;
        frag.getMemFrag().len = ((TBufferAccess*)pFragment->getMemFragPtr() + pFragment->getMemFragSize()) - position;
        subRange.appendBuffer(frag);

        pFragment++;

        if (pFragment != vFragments.end())
        {
            // move position to the front of the next buffer
            position = reinterpret_cast<TBufferAccess*>(pFragment->getMemFragPtr());
        }
        else
        {
            break;
        }
    }
}

template<class TBufferAccess>
void VirtualBuffer<TBufferAccess>::advance()
{
    // advance our intra buffer pointer
    ++pCurrFragPos;

    // check if we've exhausted the current buffer
    // NOTE: The check assumes that the len field  in the
    //       OsclMemoryFragments is a byte count.
    if ((TBufferAccess*)(pCurrFragPos) == (TBufferAccess*)pCurrFrag->getMemFragPtr() +
            pCurrFrag->getMemFragSize())
    {
        // advance to the next fragment
        pCurrFrag++;
        if (pCurrFrag == vFragments.end())
        {
            // no fragments left
            pCurrFrag = NULL;
            pCurrFragPos = NULL;
            currIndex = 0;
            return;
        }
        currIndex++;
        setCurrPointer();
    }
}

template<class TBufferAccess>
uint VirtualBuffer<TBufferAccess>::length()
{
    uint len = 0;

    for (uint i = 0; i < vFragments.size(); i++)
    {
        len += vFragments[i].getMemFrag().len;
    }

    return len;
}

#endif // VIRTUAL_BUFFER_H_INCLUDED

