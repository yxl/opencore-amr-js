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
#ifndef PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED
#define PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED

#ifndef PV_PLAYER_DATASINK_H_INCLUDED
#include "pv_player_datasink.h"
#endif

class PVPlayerDataSinkPVMFNode : public PVPlayerDataSink
{
    public:
        PVPlayerDataSinkPVMFNode() : iFormatType(PVMF_MIME_FORMAT_UNKNOWN), iNode(NULL), iEmptyString(NULL) {};
        ~PVPlayerDataSinkPVMFNode() {};

        PVPDataSinkType GetDataSinkType()
        {
            return PVP_DATASINKTYPE_SINKNODE;
        }

        PVMFFormatType GetDataSinkFormatType()
        {
            return iFormatType;
        }

        OSCL_wString& GetDataSinkFilename()
        {
            return iEmptyString;
        }

        PVMFNodeInterface* GetDataSinkNodeInterface()
        {
            return iNode;
        }

        void SetDataSinkFormatType(PVMFFormatType aFormatType)
        {
            iFormatType = aFormatType;
        }

        void SetDataSinkNode(PVMFNodeInterface* aNode)
        {
            iNode = aNode;
        }

    private:
        PVMFFormatType iFormatType;
        PVMFNodeInterface* iNode;
        OSCL_wStackString<1> iEmptyString;
};

#endif // PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED

