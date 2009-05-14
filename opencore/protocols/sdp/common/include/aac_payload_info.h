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

#ifndef AAC_PAYLOAD_INFO_H
#define AAC_PAYLOAD_INFO_H

#include "payload_info.h"

#define PVMF_SDP_DEFAULT_LATM_SAMPLE_RATE 90000

class AacPayloadSpecificInfoType : public PayloadSpecificInfoTypeBase
{
    public:
        AacPayloadSpecificInfoType(int payload)
        {
            payloadNumber = payload;

            aac_cpresent = false;
            aacplusSBRenabled = false;
            configSize = 0;
        };

        inline void setcpresent(bool arg)
        {
            aac_cpresent = arg;
        };

        inline void setAACplusSBRenabled(bool arg)
        {
            aacplusSBRenabled = arg;
        };

    private:
        bool	aac_cpresent;
        bool	aacplusSBRenabled;
        int		configSize;
};

#endif
