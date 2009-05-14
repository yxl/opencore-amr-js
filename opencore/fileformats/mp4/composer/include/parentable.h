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
/*
	This PVA_FF_Parentable Class is the base class for all other classes here - including
	both Atoms and Descriptors.  This allows the maintenance of the tree structure
	where every element has a parent - and that parent has three methods, getParent(),
	setParent(), and recomputeSize().  This allows the classes to recompute the size
	of their parent class when a child class is changed or added to.
*/


#ifndef __Parentable_H__
#define __Parentable_H__


class PVA_FF_Parentable
{

    public:
        void setParent(PVA_FF_Parentable *parent)
        {
            _pparent = parent;
        }
        PVA_FF_Parentable *getParent()
        {
            return _pparent;
        }

        virtual void recomputeSize() = 0; // Must be overridden by all other concrete classes
        virtual ~PVA_FF_Parentable() {}
    protected:
        PVA_FF_Parentable *_pparent;


};



#endif

