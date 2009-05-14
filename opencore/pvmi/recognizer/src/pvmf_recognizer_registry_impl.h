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
#ifndef PVMF_RECOGNIZER_REGISTRY_IMPL_H_INCLUDED
#define PVMF_RECOGNIZER_REGISTRY_IMPL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_RECOGNIZER_TYPES_H_INCLUDED
#include "pvmf_recognizer_types.h"
#endif

#ifndef PVMF_RECOGNIZER_PLUGIN_H_INCLUDED
#include "pvmf_recognizer_plugin.h"
#endif

#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif


/**
 * Data structure to hold the recognizer registry session information
 **/
class PVMFRecRegSessionInfo
{
    public:
        PVMFRecRegSessionInfo()
        {
        };

        PVMFRecRegSessionInfo(const PVMFRecRegSessionInfo& aSrc)
        {
            iRecRegSessionId = aSrc.iRecRegSessionId;
            iRecRegCmdHandler = aSrc.iRecRegCmdHandler;
        };

        ~PVMFRecRegSessionInfo()
        {
        };

        PVMFSessionId iRecRegSessionId;
        PVMFRecognizerCommmandHandler* iRecRegCmdHandler;
};


typedef union PVMFRecRegImplCommandParamUnion
{
    bool  bool_value;
    float float_value;
    double double_value;
    uint8 uint8_value;
    int32 int32_value;
    uint32 uint32_value;
    oscl_wchar* pWChar_value;
    char* pChar_value;
    uint8* pUint8_value;
    int32* pInt32_value;
    uint32* pUint32_value;
    OsclAny* pOsclAny_value;
} _PVMFRecRegImplCommandParamUnion;


/**
 * PVMFRecRegImplCommand Class
 *
 * PVMFRecRegImplCommand class is a data class to hold issued commands inside the recognizer registry impl
 **/
class PVMFRecRegImplCommand
{
    public:
        /**
         * The constructor for PVMFRecRegImplCommand which allows the data values to be set.
         *
         * @param aCmdType The command type value for this command.
         * @param aCmdId The command ID assigned for this command.
         * @param aContextData The pointer to the passed-in context data for this command.
         * @param aParamVector The optional pointer to a list of parameters
         *
         * @returns None
         **/
        PVMFRecRegImplCommand(PVMFSessionId aSessionId, int32 aCmdType, PVMFCommandId aCmdId, OsclAny* aContextData = NULL,
                              Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator>* aParamVector = NULL, bool aAPICommand = true) :
                iSessionId(aSessionId), iCmdType(aCmdType), iCmdId(aCmdId), iContextData(aContextData), iAPICommand(aAPICommand)
        {
            iParamVector.clear();
            if (aParamVector)
            {
                iParamVector = *aParamVector;
            }
        }

        /**
         * The copy constructor for PVMFRecRegImplCommand. Used mainly for Oscl_Vector.
         *
         * @param aCmd The reference to the source PVMFRecRegImplCommand to copy the data values from.
         *
         * @returns None
         **/
        PVMFRecRegImplCommand(const PVMFRecRegImplCommand& aCmd)
        {
            iSessionId = aCmd.iSessionId;
            iCmdType = aCmd.iCmdType;
            iCmdId = aCmd.iCmdId;
            iContextData = aCmd.iContextData;
            iAPICommand = aCmd.iAPICommand;
            iParamVector = aCmd.iParamVector;
        }

        /**
         * This function returns the session ID for the command
         *
         * @returns The session ID value for this command.
         **/
        PVMFSessionId GetSessionId()const
        {
            return iSessionId;
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
         * @returns The PVMFCommandId value for this command.
         **/
        PVMFCommandId GetCmdId()const
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
         * This function tells whether the command is an API command or not
         *
         * @returns true if API command, false if not.
         **/
        bool IsAPICommand()const
        {
            return iAPICommand;
        }

        /**
         * This function returns the command parameter from the specified index.
         * If the specified index is not available, empty parameter will be returned
         *
         * @param aIndex The index of the parameter to return
         *
         * @returns The stored parameter for this command
         **/
        PVMFRecRegImplCommandParamUnion GetParam(uint32 aIndex)const
        {
            if (aIndex >= iParamVector.size())
            {
                PVMFRecRegImplCommandParamUnion param;
                oscl_memset(&param, 0, sizeof(PVMFRecRegImplCommandParamUnion));
                return param;
            }
            else
            {
                return iParamVector[aIndex];
            }
        }

        bool operator==(const PVMFRecRegImplCommand& x)const
        {
            return iCmdId == x.iCmdId;
        }

        PVMFSessionId iSessionId;
        int32 iCmdType;
        PVMFCommandId iCmdId;
        OsclAny* iContextData;
        bool iAPICommand;
        Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator> iParamVector;
};

/**
 * PVMFRecRegImplCommandType enum
 *
 *  Enumeration of commands that can be issued to the PVMF Recognizer Registry
 *
 **/
typedef enum
{
    PVMFRECREG_COMMAND_RECOGNIZE = 1,
    PVMFRECREG_COMMAND_CANCELCOMMAND,
} PVMFRecRegImplCommandType;


/**
 * PVMFRecRegImplCommandCompareLess Class
 *
 * PVMFRecRegImplCommandCompareLess class is a utility class to allow the OSCL priority queue perform command priority comparison.
 * The class is meant to be used inside the recognizer registry impl and not exposed to the outside.
 **/
class PVMFRecRegImplCommandCompareLess
{
    public:
        /**
        * The algorithm used in OsclPriorityQueue needs a compare function
        * that returns true when A's priority is less than B's
        * @return true if A's priority is less than B's, else false
        */
        int compare(PVMFRecRegImplCommand& a, PVMFRecRegImplCommand& b) const
        {
            int a_pri = PVMFRecRegImplCommandCompareLess::GetPriority(a);
            int b_pri = PVMFRecRegImplCommandCompareLess::GetPriority(b);
            if (a_pri < b_pri)
            {
                // Higher priority
                return true;
            }
            else if (a_pri == b_pri)
            {
                // Same priority so look at the command ID to maintain FIFO
                return (a.GetCmdId() > b.GetCmdId());
            }
            else
            {
                // Lower priority
                return false;
            }
        }

        /**
        * Returns the priority of each command
        * @return A 0-based priority number. A lower number indicates lower priority.
        */
        static int GetPriority(PVMFRecRegImplCommand& aCmd)
        {
            switch (aCmd.GetCmdType())
            {
                case PVMFRECREG_COMMAND_RECOGNIZE:
                    return 5;
                case PVMFRECREG_COMMAND_CANCELCOMMAND:
                    return 3;
                default:
                    return 0;
            }
        }
};


/**
 * Implementation of the recognizer registry. The recognizer interface class should only
 * use this class.
 **/
class PVMFRecognizerRegistryImpl : public OsclTimerObject,
            public PvmiDataStreamObserver
{
    public:
        PVMFRecognizerRegistryImpl();
        ~PVMFRecognizerRegistryImpl();

        PVMFStatus RegisterPlugin(PVMFRecognizerPluginFactory& aPluginFactory);
        PVMFStatus RemovePlugin(PVMFRecognizerPluginFactory& aPluginFactory);
        PVMFStatus OpenSession(PVMFSessionId& aSessionId, PVMFRecognizerCommmandHandler& aCmdHandler);
        PVMFStatus CloseSession(PVMFSessionId aSessionId);
        PVMFCommandId Recognize(PVMFSessionId aSessionId, PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHint,
                                Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult, OsclAny* aCmdContext, uint32 aTimeout);
        PVMFCommandId CancelCommand(PVMFSessionId aSessionId, PVMFCommandId aCommandToCancelId, OsclAny* aCmdContext);

        // Reference count for the registry implementation
        int32 iRefCount;

    private:
        PVMFSessionId iNextSessionId;
        PVMFCommandId iNextCommandId;

        // From OsclTimerObject
        void Run();

        // Vector to hold the active sessions
        Oscl_Vector<PVMFRecRegSessionInfo, OsclMemAllocator> iRecognizerSessionList;

        // Vector to hold the available recognizer plug-in
        Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator> iRecognizerPluginFactoryList;

        int32 FindPluginFactory(PVMFRecognizerPluginFactory& aFactory);
        PVMFRecognizerPluginInterface* CreateRecognizerPlugin(PVMFRecognizerPluginFactory& aFactory);
        void DestroyRecognizerPlugin(PVMFRecognizerPluginFactory& aFactory, PVMFRecognizerPluginInterface* aPlugin);

        // Vector to hold pending, current, and to-cancel commands
        OsclPriorityQueue<PVMFRecRegImplCommand, OsclMemAllocator, Oscl_Vector<PVMFRecRegImplCommand, OsclMemAllocator>, PVMFRecRegImplCommandCompareLess> iRecognizerPendingCmdList;
        Oscl_Vector<PVMFRecRegImplCommand, OsclMemAllocator> iRecognizerCurrentCmd;
        Oscl_Vector<PVMFRecRegImplCommand, OsclMemAllocator> iRecognizerCmdToCancel;

        PVMFCommandId AddRecRegCommand(PVMFSessionId aSessionId, int32 aCmdType, OsclAny* aContextData = NULL, Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator>* aParamVector = NULL, bool aAPICommand = true);
        void CompleteCurrentRecRegCommand(PVMFStatus aStatus, const uint32 aCurrCmdIndex = 0, PVInterface* aExtInterface = NULL);
        bool FindCommandByID(Oscl_Vector<PVMFRecRegImplCommand, OsclMemAllocator> &aCmdQueue, const PVMFCommandId aCmdId);

        // Command handling functions
        void DoRecognize();
        void CompleteRecognize(PVMFStatus aStatus);
        void DoCancelCommand(PVMFRecRegImplCommand& aCmd);

        PVMFDataStreamFactory* iDataStreamFactory;
        PVMIDataStreamSyncInterface* iDataStream;
        PvmiDataStreamSession iDataStreamSessionID;
        PvmiDataStreamCommandId iRequestReadCapacityNotificationID;

        PVMFStatus GetMaxRequiredSizeForRecognition(uint32& aMaxSize);
        PVMFStatus GetMinRequiredSizeForRecognition(uint32& aMinSize);
        PVMFStatus CheckForDataAvailability();

        //logger
        PVLogger* iLogger;

        /* From PvmiDataStreamObserver */
        void DataStreamCommandCompleted(const PVMFCmdResp& aResponse);
        void DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent);
        void DataStreamErrorEvent(const PVMFAsyncEvent& aEvent);

        bool oRecognizePending;
        PVMFStatus iDataStreamCallBackStatus;
};

#endif // PVMF_RECOGNIZER_REGISTRY_IMPL_H_INCLUDED

