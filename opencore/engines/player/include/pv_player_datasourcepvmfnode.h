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
#ifndef PV_PLAYER_DATASOURCEPVMFNODE_H_INCLUDED
#define PV_PLAYER_DATASOURCEPVMFNODE_H_INCLUDED

#ifndef PV_PLAYER_DATASOURCE_H_INCLUDED
#include "pv_player_datasource.h"
#endif

class PVPlayerDataSourcePVMFNode : public PVPlayerDataSource
{
    public:
        PVPlayerDataSourcePVMFNode() : iFormatType(PVMF_FORMAT_UNKNOWN), iNode(NULL), iEmptyString(NULL) {};
        ~PVPlayerDataSourcePVMFNode() {};

        PVPDataSourceType GetDataSourceType()
        {
            return PVP_DATASRCTYPE_SOURCENODE;
        }

        PVMFFormatType GetDataSourceFormatType()
        {
            return iFormatType;
        }

        OSCL_wString& GetDataSourceURL()
        {
            return iEmptyString;
        }

        OsclAny* GetDataSourceContextData()
        {
            return NULL;
        }

        PVMFNodeInterface* GetDataSourceNodeInterface()
        {
            return iNode;
        }

        void SetDataSourceFormatType(PVMFFormatType aFormatType)
        {
            iFormatType = aFormatType;
        }

        void SetDataSourceNode(PVMFNodeInterface* aNode)
        {
            iNode = aNode;
        }

        bool SetAlternateSourceFormatType(PVMFFormatType aFormatType)
        {
            return false;
        }

        uint32 GetNumAlternateSourceFormatTypes()
        {
            return 0;
        }

        bool GetAlternateSourceFormatType(PVMFFormatType& aFormatType,
                                          uint32 aIndex)
        {
            return false;
        }

    private:
        PVMFFormatType iFormatType;
        PVMFNodeInterface* iNode;
        OSCL_wStackString<1> iEmptyString;
};

#endif // PV_PLAYER_DATASOURCEPVMFNODE_H_INCLUDED

