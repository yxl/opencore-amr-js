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
#ifndef PV_2WAY_MUX_DATAPATH_H_INCLUDED
#define PV_2WAY_MUX_DATAPATH_H_INCLUDED


#ifndef PV_2WAY_DATAPATH_H_INCLUDED
#include "pv_2way_datapath.h"
#endif

class CPV2WayMuxDatapath : public CPV2WayDatapath
{
    public:
        static CPV2WayMuxDatapath *NewL(PVLogger *aLogger,
                                        PVMFFormatType aFormat,
                                        CPV324m2Way *a2Way);

        virtual ~CPV2WayMuxDatapath() {};
        virtual bool SingleNodeOpen()
        {
            return (i2Way->iTerminalType == PV_SIP);
        };

    private:
        CPV2WayMuxDatapath(PVLogger *aLogger,
                           PVMFFormatType aFormat,
                           CPV324m2Way *a2Way) : CPV2WayDatapath(aLogger, EMuxDatapath, aFormat, a2Way)
        {};

        void OpenComplete();
        void PauseComplete();
        void ResumeComplete();
        void CloseComplete();

        void DatapathError();
};


#endif //PV_2WAY_MUX_DATAPATH_H_INCLUDED


