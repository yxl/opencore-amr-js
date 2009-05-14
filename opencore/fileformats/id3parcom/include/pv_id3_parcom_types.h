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
/**
 * @file pv_id3_parcom_types.h
 * @brief Type definitions for ID3 Parser-Composer
 */

#ifndef PV_ID3_PARCOM_TYPES_H_INCLUDED
#define PV_ID3_PARCOM_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif

/** Enumerated list of supported ID3 versions */
typedef enum
{
    PV_ID3_INVALID_VERSION = -1,	/**< Invalid ID3 version */
    PV_ID3_V1 = 0,		/**< ID3 Version 1 */
    PV_ID3_V1_1 = 1,	/**< ID3 Version 1.1 */
    PV_ID3_V2_2 = 2,	/**< ID3 Version 2.2 */
    PV_ID3_V2_3 = 3,	/**< ID3 Version 2.3 */
    PV_ID3_V2_4 = 4		/**< ID3 Version 2.4 */
} PVID3Version;

/** Shared pointer of a key-value pair */
typedef OsclSharedPtr<PvmiKvp> PvmiKvpSharedPtr;

/** Vector of shared pointer of a key-value pair */
typedef Oscl_Vector<PvmiKvpSharedPtr, OsclMemAllocator> PvmiKvpSharedPtrVector;

#endif // PV_ID3_PARCOM_TYPES_H_INCLUDED


