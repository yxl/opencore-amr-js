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
#ifndef PVMP4FFCN_TYPES_H_INCLUDED
#define PVMP4FFCN_TYPES_H_INCLUDED

/** Port tags */
enum PvmfMp4FFCNPortTag
{
    PVMF_MP4FFCN_PORT_TYPE_SINK = 0
};

/** Enumerated list of errors */
typedef enum
{
    PVMF_MP4FFCN_ERROR_INVALID_MEDIA_DATA = PVMF_NODE_ERROR_EVENT_LAST,
    PVMF_MP4FFCN_ERROR_ADD_SAMPLE_TO_TRACK_FAILED,
    PVMF_MP4FFCN_ERROR_FINALIZE_OUTPUT_FILE_FAILED
}PvmfMp4FFCNError;

#endif // PVMP4FFCN_TYPES_H_INCLUDED


