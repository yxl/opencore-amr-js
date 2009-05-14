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
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#define PV_ENGINE_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

// DATA TYPES
typedef int32 PVCommandId;
typedef int32 PVEventType;
typedef OsclAny* PVExclusivePtr;
typedef int32 PVResponseType;
typedef int32 PVLogLevelInfo;
typedef Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> PVPMetadataList;
/* Temporary definitions */
typedef int32 PVSDKModuleInfo;
/* END of temp defs */

struct PVSDKInfo
{
    PVSDKInfo()
    {
        iDate = 0x00000000;
    }

    PVSDKInfo& operator=(const PVSDKInfo& aSDKInfo)
    {
        iLabel = aSDKInfo.iLabel;
        iDate = aSDKInfo.iDate;
        return *this;
    }

    OSCL_StackString<80> iLabel;
    uint32 iDate; // 0xyyyymmdd
};


/**
 * PVEngineCommand Class
 *
 * PVEngineCommand class is a data class to hold issued commands. The class is meant to be used inside the engine
 * and not exposed to the interface layer or above.
 **/
class PVEngineCommand
{
    public:
        /**
         * The constructor for PVEngineCommand which allows the data values to be set.
         *
         * @param aCmdType The command type value for this command. The value is an engine-specific 32-bit value.
         * @param aCmdId The command ID assigned by the engine for this command.
         * @param aContextData The pointer to the passed-in context data for this command.
         *
         * @returns None
         **/
        PVEngineCommand(int32 aCmdType, PVCommandId aCmdId, OsclAny* aContextData = NULL, OsclAny* aParam1 = NULL, OsclAny* aParam2 = NULL, OsclAny* aParam3 = NULL) :
                iCmdType(aCmdType), iCmdId(aCmdId), iContextData(aContextData), iParam1(aParam1), iParam2(aParam2), iParam3(aParam3) {}

        /**
         * The copy constructor for PVEngineCommand. Used mainly for Oscl_Vector.
         *
         * @param aCmd The reference to the source PVEngineCommand to copy the data values from.
         *
         * @returns None
         **/
        PVEngineCommand(const PVEngineCommand& aCmd)
        {
            iCmdType = aCmd.iCmdType;
            iCmdId = aCmd.iCmdId;
            iContextData = aCmd.iContextData;
            iParam1 = aCmd.iParam1;
            iParam2 = aCmd.iParam2;
            iParam3 = aCmd.iParam3;
            iMimeType = aCmd.iMimeType;
            iUuid = aCmd.iUuid;
        }

        /**
         * This function returns the stored command type value.
         *
         * @returns The signed 32-bit command type value for this command.
         **/
        int32 GetCmdType()const
        {
            return iCmdType;
        }

        /**
         * This function returns the stored command ID value.
         *
         * @returns The PVCommandId value for this command.
         **/
        PVCommandId GetCmdId()const
        {
            return iCmdId;
        }

        /**
         * This function returns the stored context data pointer.
         *
         * @returns The pointer to the context data for this command
         **/
        OsclAny* GetContext()const
        {
            return iContextData;
        }

        /**
         * This function returns the first stored parameter pointer.
         *
         * @returns The pointer to the first stored parameter for this command
         **/
        OsclAny* GetParam1()const
        {
            return iParam1;
        }

        /**
         * This function returns the second stored parameter pointer.
         *
         * @returns The pointer to the second stored parameter for this command
         **/
        OsclAny* GetParam2()const
        {
            return iParam2;
        }

        /**
         * This function returns the third stored parameter pointer.
         *
         * @returns The pointer to the third stored parameter for this command
         **/
        OsclAny* GetParam3()const
        {
            return iParam3;
        }

        /**
         * This function returns Mime type parameter for this command
         *
         * @returns The Mime type parameter for this command
         */
        const PvmfMimeString& GetMimeType()const
        {
            return iMimeType;
        }

        /**
         * This function returns Uuid parameter for this command
         *
         * @returns The Uuid parameter for this command
         */
        PVUuid GetUuid()const
        {
            return iUuid;
        }

        /**
         * This function stores Mime type parameter of this command
         */
        void SetMimeType(const PvmfMimeString& aMimeType)
        {
            iMimeType = aMimeType;
        }

        /**
         * This function stores the Uuid parameter of this command
         */
        void SetUuid(const PVUuid& aUuid)
        {
            iUuid = aUuid;
        }

        int32 iCmdType;
        PVCommandId iCmdId;
        OsclAny* iContextData;
        OsclAny* iParam1;
        OsclAny* iParam2;
        OsclAny* iParam3;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        PVUuid iUuid;
};


/**
 * PVEngineAsyncEvent Class
 *
 * PVEngineAsyncEvent class is a data class to hold asynchronous events generated by the engine. The class is meant to be used inside the engine
 * and not exposed to the interface layer or above.
 **/
class PVEngineAsyncEvent
{
    public:
        /**
         * The constructor for PVEngineCommand which allows the data values to be set.
         *
         * @param aCmdType The command type value for this command. The value is an engine-specific 32-bit value.
         * @param aCmdId The command ID assigned by the engine for this command.
         * @param aContextData The pointer to the passed-in context data for this command.
         *
         * @returns None
         **/
        PVEngineAsyncEvent(int32 aAsyncEventType) :
                iAsyncEventType(aAsyncEventType) {}

        /**
         * The copy constructor for PVEngineAsyncEvent. Used mainly for Oscl_Vector.
         *
         * @param aAsyncEvent The reference to the source PVEngineAsyncEvent to copy the data values from.
         *
         * @returns None
         **/
        PVEngineAsyncEvent(const PVEngineAsyncEvent& aAsyncEvent)
        {
            iAsyncEventType = aAsyncEvent.iAsyncEventType;
        }

        /**
         * This function returns the stored asynchronous event type value.
         *
         * @returns The signed 32-bit event type value.
         **/
        int32 GetAsyncEventType()const
        {
            return iAsyncEventType;
        }

        int32 iAsyncEventType;
};

#endif

