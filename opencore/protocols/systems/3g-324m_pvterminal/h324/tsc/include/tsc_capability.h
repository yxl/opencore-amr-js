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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
/*****************************************************************************/
/*  file name            : tsc_capability.h                                  */
/*  file contents        :                                                   */
/*  draw                 : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/

#ifndef TSCCAPABILILTY_H_INCLUDED
#define TSCCAPABILILTY_H_INCLUDED


#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef TSC_H_INCLUDED
#include "tsc.h"
#endif

#ifndef H245DEF_H_INCLUDED
#include "h245def.h"
#endif

#ifndef H245PRI_H_INCLUDED
#include "h245pri.h"
#endif

#ifndef TSC_CONSTANTS_H_INCLUDED
#include "tsc_constants.h"
#endif


class TSC_statemanager;
class TSC_component;

class TSC_capability
{
    public:
        TSC_capability(TSC_statemanager& aTSCstatemanager) :
                iTSCstatemanager(aTSCstatemanager),
                iRemoteCapability(NULL)
        {
            iLogger = PVLogger::GetLoggerObject("3g324m.h245user");
        };

        ~TSC_capability();

        void SetMembers(TSC_component* aTSCcomponent)
        {
            iTSCcomponent = aTSCcomponent;
        }
        void InitVarsSession();
        void InitVarsLocal();

        CPvtTerminalCapability* GetRemoteCapability();
        uint32 GetRemoteBitrate(PVCodecType_t codec_type);
        void ResetCapability();
        void CreateNewCapability(Oscl_Vector<CPvtMediaCapability*, OsclMemAllocator> aCapabilityItems)
        {
            iRemoteCapability = new CPvtTerminalCapability(aCapabilityItems);
        }
        void ExtractTcsParameters(PS_VideoCapability apVideo, CPvtH263Capability *aMedia_capability);
        void ExtractTcsParameters(PS_VideoCapability pVideo, CPvtMpeg4Capability *aMedia_capability);
        void ParseTcsCapabilities(S_Capability &aCapability, Oscl_Vector<CPvtMediaCapability*, OsclMemAllocator> &aMedia_capability, uint32 userInputCapabilities, S_UserInputCapability *aUserInputCapability);
        bool IsSegmentable(TPVDirection direction, PV2WayMediaType media_type);

        PS_DataType GetOutgoingDataType(PVCodecType_t codecType, uint32 bitrate, uint16 csi_len, uint8* csi);
        PS_H223LogicalChannelParameters GetOutgoingLcnParams(PV2WayMediaType media_type,
                PS_AdaptationLayerType adaptation_layer);
        PVMFStatus ValidateIncomingDataType(bool forRev, PS_DataType pDataType);
        PVMFStatus ValidateIncomingH223LcnParams(PS_H223LogicalChannelParameters h223params, TPVDirection dir);
        PVMFStatus ValidateForwardReverseParams(PS_ForwardReverseParam forRevParams, TPVDirection dir);
        bool VerifyReverseParameters(PS_ForwardReverseParam forRevParams,
                                     TSCObserver* aObserver,
                                     PVMFStatus& status);
        uint32 GetMaxBitrateForOutgoingChannel(PVCodecType_t codecType);
        PS_DataType GetDataType(PVCodecType_t codecType, uint32 bitrate, const uint8* dci, uint16 dci_len);
        uint16 GetSupportedCodecCapabilityInfo(TPVDirection dir,
                                               PV2WayMediaType mediaType,
                                               Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& cci);
        bool HasSymmetryConstraint(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& codec_list);
        CodecCapabilityInfo* SelectOutgoingCodec(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* remote_list,
                Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* local_list);
        CodecCapabilityInfo* SelectOutgoingCodec(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* remote_list);


        Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator> GetResolutions(TPVDirection dir);
        void SetVideoResolutions(TPVDirection dir,
                                 Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& resolutions);


    private:

        // -------------------------------------------------
        // Parameters extracted from incoming TCS (RAN-32K)
        // -------------------------------------------------
        uint32 iTcsIn_H263_sqcifMPI;		// Units 1/30 second
        uint32 iTcsIn_H263_qcifMPI;		// Units 1/30 second
        uint32 iTcsIn_H263_cifMPI;		// Units 1/30 second
        uint32 iTcsIn_H263_4cifMPI;		// Units 1/30 second
        uint32 iTcsIn_H263_16cifMPI;		// Units 1/30 second


        /* Video resolutions */
        Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator> iResolutionsRx;
        Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator> iResolutionsTx;

        TSC_statemanager& iTSCstatemanager;
        /* Capability of local and remote and mutual capabiliites */
        CPvtTerminalCapability* iRemoteCapability;
        PVLogger* iLogger;


        TSC_component* iTSCcomponent;
};

#endif

