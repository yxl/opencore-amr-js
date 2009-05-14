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

//               B Y T E O R D E R   U T I L S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \file byte_order_utils.cpp This file defines functions providing byte ordering utility.
Implementor's note: These four functions are all identical in their implementation except for the ifdefs.
We could probably move all the common code into one function and call it from each of the four?
*/
#include "oscl_byte_order.h"

#if (OSCL_DISABLE_INLINES)
#include "oscl_byte_order.inl"
#endif

