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
#ifndef PV_PLAYER_DATASOURCEURL_H_INCLUDED
#define PV_PLAYER_DATASOURCEURL_H_INCLUDED

#ifndef PV_PLAYER_DATASOURCE_H_INCLUDED
#include "pv_player_datasource.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

class PVPlayerDataSourceURL : public PVPlayerDataSource
{
    public:
        PVPlayerDataSourceURL() : iFormatType(PVMF_MIME_FORMAT_UNKNOWN), iURL(NULL), iContext(NULL) {};
        ~PVPlayerDataSourceURL() {};

        PVPDataSourceType GetDataSourceType()
        {
            return PVP_DATASRCTYPE_URL;
        }

        PVMFFormatType GetDataSourceFormatType()
        {
            return iFormatType;
        }

        OSCL_wString& GetDataSourceURL()
        {
            return iURL;
        }

        OsclAny* GetDataSourceContextData()
        {
            return iContext;
        }

        PVMFNodeInterface* GetDataSourceNodeInterface()
        {
            return NULL;
        }

        void SetDataSourceFormatType(PVMFFormatType aFormatType)
        {
            iFormatType = aFormatType;
        }

        void SetDataSourceURL(const OSCL_wString& aURL)
        {
            iURL = aURL;
        }

        void SetDataSourceContextData(const OsclAny* aContext)
        {
            iContext = (OsclAny*)aContext;
        }

        bool SetAlternateSourceFormatType(PVMFFormatType aFormatType)
        {
            int32 err;
            OSCL_TRY(err, iAlternateFormatTypeVec.push_back(aFormatType););
            if (err != OsclErrNone)
            {
                return false;
            }
            return true;
        }

        uint32 GetNumAlternateSourceFormatTypes()
        {
            return (iAlternateFormatTypeVec.size());
        }

        bool GetAlternateSourceFormatType(PVMFFormatType& aFormatType,
                                          uint32 aIndex)
        {
            if (aIndex < iAlternateFormatTypeVec.size())
            {
                aFormatType = iAlternateFormatTypeVec[aIndex];
                return true;
            }
            return false;
        }

    private:
        PVMFFormatType iFormatType;
        OSCL_wHeapString<OsclMemAllocator> iURL;
        OsclAny* iContext;
        Oscl_Vector<PVMFFormatType, OsclMemAllocator> iAlternateFormatTypeVec;
};

#endif // PV_PLAYER_DATASOURCEURL_H_INCLUDED

