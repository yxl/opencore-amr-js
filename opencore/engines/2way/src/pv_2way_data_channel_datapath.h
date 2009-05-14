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
#ifndef PV_2WAY_DATA_CHANNEL_DATAPATH_H_INCLUDED
#define PV_2WAY_DATA_CHANNEL_DATAPATH_H_INCLUDED

#ifndef PV_2WAY_DATAPATH_H_INCLUDED
#include "pv_2way_datapath.h"
#endif

#ifndef PV_2WAY_CMD_CONTROL_DATAPATH_H_INCLUDED
#include "pv_2way_cmd_control_datapath.h"
#endif

class CPV2WayDataChannelDatapath : public CPV2WayCmdControlDatapath
{
    public:
        CPV2WayDataChannelDatapath(PVLogger *aLogger,
                                   TPV2WayDatapathType aType,
                                   PVMFFormatType aFormat,
                                   CPV324m2Way *a2Way) :
                CPV2WayCmdControlDatapath(aLogger, aType, aFormat, a2Way),
                iChannelId(0),
                iTSCPortTag(0)
        {};

        virtual ~CPV2WayDataChannelDatapath() {};

        bool SetCmd(TPV2WayCmdInfo *aCmdInfo);

        void SetTSCPortTag(int32 aPortTag)
        {
            iTSCPortTag = aPortTag;
        }
        int32 GetTSCPortTag()
        {
            return iTSCPortTag;
        }

        virtual void TSCPortClosed() = 0;
        virtual PVMFPortInterface *GetTSCPort() = 0;

        void SetChannelId(TPVChannelId aChannelId)
        {
            iChannelId = aChannelId;
        }
        TPVChannelId GetChannelId()
        {
            return iChannelId;
        }

    protected:
        virtual void OpenComplete()
        {
            CommandComplete(PVMFSuccess);
        }
        void PauseComplete()
        {
            CommandComplete(PVMFSuccess);
        }
        void ResumeComplete()
        {
            CommandComplete(PVMFSuccess);
        }
        void CloseComplete();
        void DatapathError()
        {
            SetCmd(NULL);
        }

        bool ParentIsClosing();
        void CommandComplete(PVMFStatus aStatus);

        TPVChannelId iChannelId;
        int32 iTSCPortTag;

};


#endif //PV_2WAY_DATA_CHANNEL_DATAPATH_H_INCLUDED


