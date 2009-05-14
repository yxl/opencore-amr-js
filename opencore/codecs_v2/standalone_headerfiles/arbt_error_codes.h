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

#ifndef ARBT_ERROR_CODES_H_INCLUDED
#define ARBT_ERROR_CODES_H_INCLUDED


/** Leave Codes
*/
typedef int32 ArbtLeaveCode;

#define ArbtErrNone 0
#define ArbtErrGeneral 100
#define ArbtErrNoMemory 101
#define ArbtErrCancelled 102
#define ArbtErrNotSupported 103
#define ArbtErrArgument 104
#define ArbtErrBadHandle 105
#define ArbtErrAlreadyExists 106
#define ArbtErrBusy 107
#define ArbtErrNotReady 108
#define ArbtErrCorrupt 109
#define ArbtErrTimeout 110
#define ArbtErrOverflow 111
#define ArbtErrUnderflow 112
#define ArbtErrInvalidState 113
#define ArbtErrNoResources 114

/** For backward compatibility with old definitions
*/
#define ARBT_ERR_NONE ArbtErrNone
#define ARBT_BAD_ALLOC_EXCEPTION_CODE ArbtErrNoMemory

/** Return Codes
*/
typedef int32 ArbtReturnCode;

#define  ArbtSuccess 0
#define  ArbtPending 1
#define  ArbtFailure -1

#endif

/*! @} */
