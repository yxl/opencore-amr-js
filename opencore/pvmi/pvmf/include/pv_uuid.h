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
 *  @file pv_uuid.h
 *  @brief This file defines the PV UUID strcuture used for unique identifies as well as the
 *  short (32-bit) identifiers PVUid32.
 */

#ifndef PV_UUID_H_INCLUDED
#define PV_UUID_H_INCLUDED

#ifndef OSCL_UUID_H_INCLUDED
#include "oscl_uuid.h"
#endif

typedef uint32 PVUid32;

/** PV UUID structure used for unique identification of PV SDK modules and interfaces. */
typedef OsclUuid PVUuid;

#endif

