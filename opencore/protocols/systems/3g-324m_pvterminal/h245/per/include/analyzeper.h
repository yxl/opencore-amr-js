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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------

// ============================================================

// FILE: AnalyzePER.h

//

// DESCRIPTION: PER analysis support routine prototypes.

//   These routines provide support for the automatically

//   generated functions in h245_analysis.[ch].

//

// Written by Ralph Neff, PacketVideo, 3/6/2000

// (c) 2000 PacketVideo Corp.

// ============================================================


#ifndef ANALYZEPER

#define ANALYZEPER

#include "genericper.h"

#define ANALYZER_PERE	0x0002	// (assume tags are fixed)
#define ANALYZER_PEREH	0x0004
#define ANALYZER_PERD	0x0008
#define ANALYZER_PERDH	0x0010

/*=========================================================*/

/*============ Function Prototypes ========================*/

/*=========================================================*/

/* Base-Level Interface */

void Show245(uint16 tag, uint16 indent, const char *inString);

void ShowHexData(uint16 tag, uint16 indent, uint16 size, uint8* data);

/* Low-Level Objects */

void ShowPERNull(uint16 tag, uint16 indent, const char *label);

void ShowPERBoolean(uint16 tag, uint16 indent, const char *label, uint32 value);

void ShowPERInteger(uint16 tag, uint16 indent, const char *label, uint32 value);

void ShowPERSignedInteger(uint16 tag, uint16 indent, const char *label, int32 value);

void ShowPERUnboundedInteger(uint16 tag, uint16 indent, const char *label, uint32 value);

void ShowPEROctetString(uint16 tag, uint16 indent, const char *label, PS_OCTETSTRING x);

void ShowPERBitString(uint16 tag, uint16 indent, const char *label, PS_BITSTRING x);

void ShowPERCharString(uint16 tag, uint16 indent, const char *label, PS_int8STRING x);

void ShowPERObjectID(uint16 tag, uint16 indent, const char *label, PS_OBJECTIDENT x);

/* Higher Level Objects */

void ShowPERChoice(uint16 tag, uint16 indent, const char *label, const char *typestring);

void ShowPERSequence(uint16 tag, uint16 indent, const char *label, const char *typestring);

void ShowPERSequenceof(uint16 tag, uint16 indent, const char *label, const char *typestring);

void ShowPERSetof(uint16 tag, uint16 indent, const char *label, const char *typestring);

void ShowPERClosure(uint16 tag, uint16 indent, const char *label);

/* Arrays of Multiple Objects */

void ShowPERIntegers(uint16 tag, uint16 indent, const char *label, uint32 value, uint32 number);

void ShowPEROctetStrings(uint16 tag, uint16 indent, const char *label, PS_OCTETSTRING x, uint32 number);

#endif
