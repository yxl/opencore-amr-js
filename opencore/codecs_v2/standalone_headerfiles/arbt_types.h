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
/*! \file oscl_types.h
    \brief This file contains basic type definitions for common use across platforms.

*/



#ifndef ARBT_TYPES_H_INCLUDED
#define ARBT_TYPES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#define ARBT_HAS_MSWIN_SUPPORT         1
#endif

#if defined(_GNU_SOURCE) || (ARBT_HAS_LINUX_SUPPORT)
#define LINUX		1
#endif

//! A typedef for a signed 8 bit integer.
#ifndef int8
typedef signed char int8;
#endif

//! A typedef for an unsigned 8 bit integer.
#ifndef uint8
typedef unsigned char uint8;
#endif

//! A typedef for a signed 16 bit integer.
#ifndef int16
typedef short int16;
#endif

//! A typedef for an unsigned 16 bit integer.
#ifndef uint16
typedef unsigned short uint16;
#endif

//! A typedef for a signed 32 bit integer.
#ifndef int32
typedef long int32;
#endif

//! A typedef for an unsigned 32 bit integer.
#ifndef uint32
typedef unsigned long uint32;
#endif

#ifndef sint8
typedef signed char sint8;
#endif

#ifndef OsclFloat
typedef float OsclFloat;
#endif

#ifndef uint
typedef unsigned int uint;
#endif


#ifndef int64
#define ARBT_HAS_NATIVE_INT64_TYPE 1

#if defined(LINUX)
#define ARBT_NATIVE_INT64_TYPE long long
#else // Other platforms
#define ARBT_NATIVE_INT64_TYPE __int64
#endif // LINUX

typedef ARBT_NATIVE_INT64_TYPE int64;

#endif // int64

#ifndef uint64
#define ARBT_HAS_NATIVE_UINT64_TYPE  1

#if defined(LINUX)
#define ARBT_NATIVE_UINT64_TYPE unsigned long long
#else // Other platforms
#define ARBT_NATIVE_UINT64_TYPE unsigned __int64
#endif // LINUX
typedef ARBT_NATIVE_UINT64_TYPE uint64;
#endif // uint64



#define ARBT_UNUSED_ARG(x) (void)(x)

#ifndef OSCL_EXPORT_REF
#define OSCL_EXPORT_REF
#endif

#ifndef OSCL_IMPORT_REF
#define OSCL_IMPORT_REF
#endif

#if defined(ARBT_DISABLE_INLINES)
#define ARBT_INLINE
#define OSCL_COND_EXPORT_REF OSCL_EXPORT_REF
#define OSCL_COND_IMPORT_REF OSCL_IMPORT_REF
#else
#define ARBT_INLINE inline
#define OSCL_COND_IMPORT_REF
#define OSCL_COND_IMPORT_REF
#endif

#ifndef INT64
#define INT64 int64
#endif

#ifndef UINT64
#define UINT64 uint64
#endif

#ifndef UINT64_HILO
#define UINT64_HILO(a,b) ((a<<32) | b)
#endif


#endif // ARBT_TYPES_H_INCLUDED
