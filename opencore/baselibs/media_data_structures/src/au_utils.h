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
#if !defined(AU_UTILS_H)
#define AU_UTILS_H
#include "oscl_types.h"
#include "oscl_media_status.h"
#include "access_unit.h"
#include "pv_gau.h"

OSCL_IMPORT_REF
AccessUnit* Gau2AU(const GAU* gau, AUImplAllocator * au_impl_alloc , MediaStatusClass::status_t& media_status,
                   bool ignore_sample_info = false);

#endif
