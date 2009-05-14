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
#ifndef PVMF_NODE_CMD_MSG_H_INCLUDED
#define PVMF_NODE_CMD_MSG_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif



class PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgBase(PVMFCommandId aId, OsclAny *aContext) : iId(aId), iType(aType) {};

        virtual ~PVMFNodeCmdMsgBase() {};
        PVMFCommandId GetId()
        {
            return iId;
        }
        OsclAny *GetContext()
        {
            return iContext;
        }

        friend int32 operator<(const PVMFNodeCmdMsgBase &a, const PVMFNodeCmdMsgBase &b);

    private:
        PVMFCommandId iId;
        OsclAny *iContext;
};

class PVMFNodeCmdMsgInit : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgInit(PVMFCommandId aId, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_INIT, aContext)
        {};

        ~PVMFNodeCmdMsgInit() {};
};

class PVMFNodeCmdMsgStart : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgStart(PVMFCommandId aId, PVMFPortInterface *aPort, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_START, aContext),
                iPort(aPort)
        {};

        ~PVMFNodeCmdMsgStart() {};

        PVMFPortInterface *GetPort()
        {
            return iPort;
        }

    private:
        PVMFPortInterface *iPort;
};

class PVMFNodeCmdMsgStop : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgStop(PVMFCommandId aId, PVMFPortInterface *aPort, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_STOP, aContext),
                iPort(aPort)
        {};

        ~PVMFNodeCmdMsgStop() {};

        PVMFPortInterface *GetPort()
        {
            return iPort;
        }

    private:
        PVMFPortInterface *iPort;
};

class PVMFNodeCmdMsgPause : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgPause(PVMFCommandId aId, PVMFPortInterface *aPort, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_PAUSE, aContext),
                iPort(aPort)
        {};

        ~PVMFNodeCmdMsgPause() {};

        PVMFPortInterface *GetPort()
        {
            return iPort;
        }

    private:
        PVMFPortInterface *iPort;
};

class PVMFNodeCmdMsgReset : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgReset(PVMFCommandId aId, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_RESET, aContext)
        {};

        ~PVMFNodeCmdMsgReset() {};
};

class PVMFNodeCmdMsgRequestPort : public PVMFNodeCmdMsgBase
{
    public:
        PVMFNodeCmdMsgRequestPort(PVMFCommandId aId, const PVMFPortProperty& aPortProperty, OsclAny *aContext) : PVMFNodeCmdMsgBase(aId, PVMF_NODE_CMD_REQUEST_PORT, aContext),
                iPortProperty(aPortProperty)
        {};

        ~PVMFNodeCmdMsgRequestPort() {};

        const PVMFPortProperty &GetPortProperty()
        {
            return iPortProperty;
        }

    private:
        const PVMFPortProperty &iPortProperty;
};


inline int32 operator<(const PVMFNodeCmdMsgBase &a, const PVMFNodeCmdMsgBase &b)
{
    return (a.iId < b.iId);
}

#endif
