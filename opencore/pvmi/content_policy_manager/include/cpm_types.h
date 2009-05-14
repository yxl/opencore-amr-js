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
#ifndef CPM_TYPES_H
#define CPM_TYPES_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

typedef uint32 PVMFCPMContentType;

/* CPM Usage Key Strings */
#define PVMF_CPM_REQUEST_USE_KEY_STRING "x-pvmf/cpm/intent;valtype=bitarray32;name=pvmfcpmintent;version=1.0"
#define PVMF_CPM_AUTHORIZATION_DATA_KEY_STRING "x-pvmf/cpm/intent;valtype=uint8*"

#define PVMF_CPM_DEFAULT_PLUGIN_AUTHORIZATION_TIMEOUT_IN_MS 3000

#define BITMASK_PVMF_CPM_DRM_INTENT_PLAY          0x00000001
#define BITMASK_PVMF_CPM_DRM_INTENT_PAUSE         0x00000002
#define BITMASK_PVMF_CPM_DRM_INTENT_RESUME        0x00000004
#define BITMASK_PVMF_CPM_DRM_INTENT_SEEK_FORWARD  0x00000008
#define BITMASK_PVMF_CPM_DRM_INTENT_SEEK_BACK     0x00000010
#define BITMASK_PVMF_CPM_DRM_INTENT_STOP          0x00000020
#define BITMASK_PVMF_CPM_DRM_INTENT_PRINT         0x00000040
#define BITMASK_PVMF_CPM_DRM_INTENT_DOWNLOAD      0x00000080
#define BITMASK_PVMF_CPM_DRM_INTENT_SAVE          0x00000100
#define BITMASK_PVMF_CPM_DRM_INTENT_EXECUTE       0x00000200
#define BITMASK_PVMF_CPM_DRM_INTENT_PREVIEW       0x00000400

/* MIME strings for CPM Plugins */
#define PVMF_CPM_MIME_PV_PARENTAL_CONTROL	"X-PV-PARENTAL-CONTROL"

/* MIME strings for CPM Content Formats */
#define PVMF_CPM_MIME_CONTENT_FORMAT_UKNOWN "X-PV-CPM-UNKNOWN-CONTENT-TYPE"

/* CPM Content Formats */
#define PVMF_FIRST_CPM_CONTENT_FORMAT	2000
#define PVMF_CPM_CONTENT_FORMAT_UNKNOWN				PVMF_FIRST_CPM_CONTENT_FORMAT
#define PVMF_CPM_FORMAT_OMA1						PVMF_FIRST_CPM_CONTENT_FORMAT+1
#define PVMF_CPM_FORMAT_OMA2						PVMF_FIRST_CPM_CONTENT_FORMAT+2
#define PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS		PVMF_FIRST_CPM_CONTENT_FORMAT+3
#define PVMF_CPM_FORMAT_ACCESS_BEFORE_AUTHORIZE		PVMF_FIRST_CPM_CONTENT_FORMAT+4
/*
 * When adding types to this range, please update the following
 * value to equal the last defined format.
 */
#define PVMF_LAST_CPM_CONTENT_FORMAT	PVMF_FIRST_CPM_CONTENT_FORMAT+4

#endif //CPM_TYPES_H

