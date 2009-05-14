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
#if !defined(PVT_PARAMS_H)
#define PVT_PARAMS_H

#include "oscl_stdstring.h"
#include "oscl_base.h"
#include "control_msg_hdr.h"
#include "pvt_common.h"
#include "oscl_mem.h"

class CPVChannelParam : public CPVTrackInfo
{
    public:
        CPVChannelParam(TPVDirection dir, TPVChannelId id, TPVDirectionality directionality, ErrorProtectionLevel_t epl):
                iDirection(dir), iChannelId(id), iDirectionality(directionality), iEpl(epl),
                iNumSduSizes(0), iSduSizes(NULL), iMediaParam(NULL)
        {
        }
        CPVChannelParam(CPVChannelParam& that) : CPVTrackInfo(that),
                iDirection(that.iDirection), iChannelId(that.iChannelId), iDirectionality(that.iDirectionality), iEpl(that.iEpl),
                iNumSduSizes(that.iNumSduSizes), iSduSizes(NULL), iMediaParam(NULL)
        {
            if (iNumSduSizes)
            {
                iSduSizes = (int*)OSCL_DEFAULT_MALLOC(iNumSduSizes * sizeof(int));
                oscl_memcpy(iSduSizes, that.iSduSizes, iNumSduSizes*sizeof(int));
            }
            if (that.iMediaParam)
            {
                iMediaParam = (CPVMediaParam*)that.iMediaParam->Copy();
            }

        }
        virtual ~CPVChannelParam()
        {
            if (iSduSizes)
                OSCL_DEFAULT_FREE(iSduSizes);
            if (iMediaParam)
                OSCL_DELETE(iMediaParam);
        }

        void SetChannelParams(TPVDirection dir, TPVChannelId id, TPVDirectionality directionality, ErrorProtectionLevel_t epl)
        {
            iDirection = dir;
            iChannelId = id;
            iDirectionality = directionality;
            iEpl = epl;
        }

        void SetMediaParam(CPVMediaParam* media_param)
        {
            if (iMediaParam)
            {
                OSCL_DELETE(iMediaParam);
                iMediaParam = NULL;
            }
            if (media_param)
            {
                iMediaParam = (CPVMediaParam*)media_param->Copy();
            }
        }

        int SetSduSizes(int num_sizes, int* sizes)
        {
            iNumSduSizes = num_sizes;
            if (iSduSizes)
            {
                OSCL_DEFAULT_FREE(iSduSizes);
                iSduSizes = NULL;
            }
            if (num_sizes)
            {
                iSduSizes = (int*)OSCL_DEFAULT_MALLOC(iNumSduSizes * sizeof(int));

                for (int i = 0; i < num_sizes; i++)
                {
                    iSduSizes[i] = sizes[i];
                }
            }
            return num_sizes;
        }

        void Clear()
        {
            SetChannelParams(OUTGOING, CHANNEL_ID_UNKNOWN, EPVT_UNI_DIRECTIONAL, E_EP_LOW);
            SetMediaParam(NULL);
            SetSduSizes(0, NULL);
        }

        /* Get methods */
        TPVDirection GetDirection()
        {
            return iDirection;
        }
        TPVChannelId GetChannelId()
        {
            return iChannelId;
        }

        TPVDirectionality GetDirectionality()
        {
            return iDirectionality;
        }

        ErrorProtectionLevel_t GetErrorProtectionLevel()
        {
            return iEpl;
        }

        int GetNumSduSizes()
        {
            return iNumSduSizes;
        }
        int GetSduSize(int index = 0)
        {
            return iSduSizes[index];
        }
        int* GetSduSizes()
        {
            if (iNumSduSizes)
                return iSduSizes;
            return NULL;
        }
        CPVMediaParam* GetMediaParam()
        {
            return iMediaParam;
        }

        CPVTrackInfo* Copy()
        {
            return OSCL_NEW(CPVChannelParam, (*this));
        }

    protected:
        CPVChannelParam() : iDirection(OUTGOING), iChannelId(CHANNEL_ID_UNKNOWN), iDirectionality(EPVT_UNI_DIRECTIONAL), iEpl(E_EP_LOW),
                iNumSduSizes(0), iSduSizes(NULL), iMediaParam(NULL)
        {
        }

        TPVDirection iDirection;
        TPVChannelId iChannelId;
        TPVDirectionality iDirectionality;
        ErrorProtectionLevel_t iEpl;
        int iNumSduSizes;
        int* iSduSizes;
        CPVMediaParam* iMediaParam;
};


class CPVDataParam : public CPVParam
{
    public:
        CPVDataParam() {}
};

#define min2(a, b) ((a > b) ? b : a)

#endif
