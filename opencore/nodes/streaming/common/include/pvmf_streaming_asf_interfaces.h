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
///////////////////////////////////////////////////////////////////////////////
//
// pvmf_streaming_asf_interfaces.h
//
// Defines abstract interfaces that will be used by streaming nodes to talk to
// modules providing ASF streaming related functionality.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PVMF_STREAMING_ASF_INTERFACES_INCLUDED
#define PVMF_STREAMING_ASF_INTERFACES_INCLUDED

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

class StreamAsfHeader
{
    public:
        StreamAsfHeader()
        {
            iASFHeaderObject = NULL;
        };
        ~StreamAsfHeader();
        OsclAny*      iASFHeaderObject;
};

#endif // PVMF_STREAMING_ASF_INTERFACES_INCLUDED


