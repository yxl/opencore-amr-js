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

#ifndef PV_2WAY_INTERFACE_CMD_MESSAGES_H_INCLUDED
#define PV_2WAY_INTERFACE_CMD_MESSAGES_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef PV_INTERFACE_MESSAGE_H_INCLUDED
#include "pv_interface_cmd_message.h"
#endif

#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#include "pv_2way_interface.h"
#endif

#ifndef PV_2WAY_ENGINE_H_INCLUDED
#include "pv_2way_engine.h"
#endif

/*
#ifndef PV_2WAY_SYMBIAN_INTERFACE_H_INCLUDED
#include "pv_2way_symbian_interface.h"
#endif

#ifndef PV_COMMON_SYMBIAN_TYPES_H_INCLUDED
#include "pv_common_symbian_types.h"
#endif
*/

class PV2WayMessageGetSDKInfo : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageGetSDKInfo(PVSDKInfo &aSDKInfo, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_GET_SDK_INFO, aContextData)
                , iSDKInfo(aSDKInfo)
        {
        }

        PVSDKInfo &iSDKInfo;
};

class PV2WayMessageGetSDKModuleInfo : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageGetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_GET_SDK_MODULE_INFO, aContextData)
                , iSDKModuleInfo(aSDKModuleInfo)
        {
        }

        PVSDKModuleInfo &iSDKModuleInfo;
};


class PV2WayMessageInit : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageInit(PV2WayInitInfo& aInitInfo, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_INIT, aContextData)
                , iInitInfo(aInitInfo)
        {
        }

        PV2WayInitInfo& iInitInfo;
};

class PV2WayMessageReset : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageReset(OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_RESET, aContextData)
        {
        }

};

class PV2WayMessageAddDataSource : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageAddDataSource(PVTrackId aTrackId, PVMFNodeInterface& aDataSource, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_ADD_DATA_SOURCE, aContextData)
                , iTrackId(aTrackId)
                , iDataSource(aDataSource)
        {
        }

        PVTrackId iTrackId;
        PVMFNodeInterface& iDataSource;
};

class PV2WayMessageRemoveDataSource : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageRemoveDataSource(PVMFNodeInterface& aDataSource, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_REMOVE_DATA_SOURCE, aContextData)
                , iDataSource(aDataSource)
        {
        }

        PVMFNodeInterface& iDataSource;
};

class PV2WayMessageAddDataSink : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageAddDataSink(PVTrackId aTrackId, PVMFNodeInterface& aDataSink, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_ADD_DATA_SINK, aContextData)
                , iTrackId(aTrackId)
                , iDataSink(aDataSink)
        {
        }

        PVTrackId iTrackId;
        PVMFNodeInterface& iDataSink;
};

class PV2WayMessageRemoveDataSink : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageRemoveDataSink(PVMFNodeInterface& aDataSink, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_REMOVE_DATA_SINK, aContextData)
                , iDataSink(aDataSink)
        {
        }

        PVMFNodeInterface& iDataSink;
};

class PV2WayMessageConnect : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageConnect(const PV2WayConnectOptions& aOptions, PVMFNodeInterface* aCommServer, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_CONNECT, aContextData)
                , iConnectOptions(aOptions),
                iCommServer(aCommServer)
        {
        }

        const PV2WayConnectOptions& iConnectOptions;
        PVMFNodeInterface* iCommServer;
};


class PV2WayMessageDisconnect : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageDisconnect(OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_DISCONNECT, aContextData)
        {
        }

};

class PV2WayMessageGetPV2WayState : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageGetPV2WayState(PV2WayState& aState, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_GET_PV2WAY_STATE, aContextData)
                , iState(aState)
        {
        }

        PV2WayState& iState;
};


class PV2WayMessagePause : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessagePause(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_PAUSE, aContextData)
                , iDirection(aDirection)
                , iTrackId(aTrackId)
        {
        }

        PV2WayDirection iDirection;
        PVTrackId iTrackId;
};


class PV2WayMessageResume : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageResume(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_RESUME, aContextData)
                , iDirection(aDirection)
                , iTrackId(aTrackId)
        {
        }

        PV2WayDirection iDirection;
        PVTrackId iTrackId;
};


class PV2WayMessageSetLogAppender : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageSetLogAppender(OsclSharedPtr<char> &aTag, OsclSharedPtr<PVLoggerAppender> &aAppender, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_SET_LOG_APPENDER, aContextData)
                , iTag(aTag)
                , iAppender(aAppender)
        {
        }

        OsclSharedPtr<char> iTag;
        OsclSharedPtr<PVLoggerAppender> iAppender;
};


class PV2WayMessageRemoveLogAppender : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageRemoveLogAppender(OsclSharedPtr<char> &aTag, OsclSharedPtr<PVLoggerAppender> &aAppender, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_REMOVE_LOG_APPENDER, aContextData)
                , iTag(aTag)
                , iAppender(aAppender)
        {
        }

        OsclSharedPtr<char> iTag;
        OsclSharedPtr<PVLoggerAppender> iAppender;
};


class PV2WayMessageSetLogLevel : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageSetLogLevel(OsclSharedPtr<char> &aTag, int32 aLevel, bool aSetSubtree, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_SET_LOG_LEVEL, aContextData)
                , iTag(aTag)
                , iSetSubtree(aSetSubtree)
                , iLevel(aLevel)
        {
        }

        OsclSharedPtr<char> iTag;
        bool iSetSubtree;
        int32 iLevel;
};


class PV2WayMessageGetLogLevel : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageGetLogLevel(OsclSharedPtr<char> &aTag, int32& aLogLevel, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_GET_LOG_LEVEL, aContextData)
                , iTag(aTag)
                , iLogLevel(aLogLevel)

        {
        }

        OsclSharedPtr<char> iTag;
        int32& iLogLevel;
};

class PV2WayMessageQueryUUID : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageQueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, BasicAlloc> &aUuids, bool aExactUuidsOnly, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_QUERY_UUID, aContextData)
                , iMimeType(aMimeType)
                , iUuids(aUuids)
                , iExactUuidsOnly(aExactUuidsOnly)

        {
        }

        const PvmfMimeString&  iMimeType;
        Oscl_Vector<PVUuid, BasicAlloc> &iUuids;
        bool iExactUuidsOnly;
};


class PV2WayMessageQueryInterface : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageQueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_QUERY_INTERFACE, aContextData)
                , iUuid(aUuid)
                , iInterfacePtr(aInterfacePtr)
        {
        }

        PVUuid iUuid;
        PVInterface* &iInterfacePtr;
};

class PV2WayMessageCancelAllCommands : public CPVCmnInterfaceCmdMessage
{
    public:
        PV2WayMessageCancelAllCommands(OsclAny* aContextData)
                : CPVCmnInterfaceCmdMessage(PVT_COMMAND_CANCEL_ALL_COMMANDS, aContextData)
        {
        }

};

#endif




