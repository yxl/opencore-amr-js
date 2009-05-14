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
// ===================================================================
// FILE: h245_copier.h
//
// DESC: PER Copy routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

// ****************************************************
// ** NOTE: This file is not auto-generated.         **
// ** Add copy routines as needed in the application **
// ****************************************************

#ifndef PER_COPIER
#define PER_COPIER
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
OSCL_IMPORT_REF PS_DataType Copy_DataType(PS_DataType x);
OSCL_IMPORT_REF PS_H223LogicalChannelParameters Copy_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x);
PS_MultiplexEntryDescriptor Copy_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x);
PS_ForwardLogicalChannelParameters Copy_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x);
PS_ReverseLogicalChannelParameters Copy_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x);
PS_TerminalCapabilitySet Copy_TerminalCapabilitySet(PS_TerminalCapabilitySet x);
#endif
