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

//                     P V M F _ T I M E S T A M P

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/**
 *  @file pvmf_timestamp.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) timestamp class
 *  which holds a 32-bit continuous timestamp that periodically wraps back to zero.
 *
 */

#ifndef PVMF_TIMESTAMP_H_INCLUDED
#define PVMF_TIMESTAMP_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif


/**
 * PVMFTimestamp is a circular (i.e., value wraps after max int)
 * 32-bit timestamp container.
 */
typedef uint32 PVMFTimestamp;

const PVMFTimestamp PVMFTIMESTAMP_LESSTHAN_THRESHOLD = 0x80000000;

/**
 * Compares two timestamps and reports if the first occurred before (is less than)
 * the second value.
 * @param a first timestamp value in the comparison
 * @param b second timestamp value in the comparison
 * @return true if a < b, false otherwise.
 *
 */
OSCL_IMPORT_REF bool lessthan(const PVMFTimestamp& a, const PVMFTimestamp& b,
                              PVMFTimestamp threshold = PVMFTIMESTAMP_LESSTHAN_THRESHOLD);


typedef struct PVMFTimeValue
{
    uint32 time_val;
    uint32 time_units;
} _PVMFTimeValue;

#endif
