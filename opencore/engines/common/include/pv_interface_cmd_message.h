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
#ifndef PV_INTERFACE_CMD_MESSAGE_H_INCLUDED
#define PV_INTERFACE_CMD_MESSAGE_H_INCLUDED

#ifndef PV_COMMON_TYPES_H_INCLUDED
#include "pv_common_types.h"
#endif

#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

/**
 * CPVInterfaceCmdMessage Class
 *
 * CPVInterfaceCmdMessage is the interface to the pv2way SDK, which
 * allows initialization, control, and termination of a two-way terminal.
 * The application is expected to contain and maintain a pointer to the
 * CPV2WayInterface instance at all times that a call is active.
 * The CPV2WayFactory factory class is to be used to create and
 * delete instances of this class
 **/
class CPVCmnInterfaceCmdMessage
{
    public:
        CPVCmnInterfaceCmdMessage(int aType,
                                  OsclAny* aContextData) : iId(0),
                iType(aType),
                iPriority(0),
                iContextData(aContextData) {};

        CPVCmnInterfaceCmdMessage() {};

        virtual ~CPVCmnInterfaceCmdMessage() {};

        PVCommandId GetCommandId()
        {
            return iId;
        }
        int GetType()
        {
            return iType;
        }
        OsclAny *GetContextData()
        {
            return iContextData;
        }

        /**
         * The algorithm used in OsclPriorityQueue needs a compare function
         * that returns true when A's priority is less than B's
         * @return true if A's priority is less than B's, else false
         */
        int compare(CPVCmnInterfaceCmdMessage* a, CPVCmnInterfaceCmdMessage* b) const
        {
            if (a->GetPriority() < b->GetPriority())
                return 1;
            else //if no priority, use fifo order.
                return (a->GetCommandId() > b->GetCommandId());
        }

        int32 GetPriority()const
        {
            return iPriority;
        }

        friend int32 operator<(const CPVCmnInterfaceCmdMessage& a, const CPVCmnInterfaceCmdMessage& b);

        void SetId(PVCommandId aId)
        {
            iId = aId;
        }

    protected:
        PVCommandId iId;
        int iType;
        int32 iPriority;
        OsclAny* iContextData;

        friend class PVInterfaceProxy;
};

inline int32 operator<(const CPVCmnInterfaceCmdMessage& a, const CPVCmnInterfaceCmdMessage& b)
{
    //Use priority
    if (a.iPriority < b.iPriority)
    {
        return true;
    }
    //If priority is the same, use id.
    else if (a.iPriority == b.iPriority)
    {
        //Smaller id means an older message so process older message first.
        return (a.iId > b.iId);
    }
    else
    {
        return false;
    }
}
#endif


