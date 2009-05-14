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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                               Parentable Class                                */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This Parentable Class is the base class for all other classes here - including
	both Atoms and Descriptors.  This allows the maintenance of the tree structure
	where every element has a parent - and that parent has three methods, getParent(),
	setParent(), and recomputeSize().  This allows the classes to recompute the size
	of their parent class when a child class is changed or added to.
*/


#ifndef PARENTABLE_H_INCLUDED
#define PARENTABLE_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

class Parentable
{

    public:
        void setParent(Parentable *parent)
        {
            _pparent = parent;
        }
        Parentable *getParent()
        {
            return _pparent;
        }

    protected:
        Parentable *_pparent;
};

#endif // PARENTABLE_H_INCLUDED


