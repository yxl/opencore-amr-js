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
#include "tsc_capability.h"

#include "tsc_statemanager.h"
#include "tsc_component.h"

void TSC_capability::InitVarsSession()
{
    iTcsIn_H263_sqcifMPI = 0;		// Units 1/30 second
    iTcsIn_H263_qcifMPI = 2;		// Units 1/30 second
    iTcsIn_H263_cifMPI = 0;		// Units 1/30 second
    iTcsIn_H263_4cifMPI = 0;		// Units 1/30 second
    iTcsIn_H263_16cifMPI = 0;		// Units 1/30 second
}

void TSC_capability::InitVarsLocal()
{
    /* Initialize video resolutions */
    PVMFVideoResolutionRange qcif_range(PVMF_RESOLUTION_QCIF, PVMF_RESOLUTION_QCIF);
    iResolutionsRx.push_back(qcif_range);
    iResolutionsTx.push_back(qcif_range);
}
void TSC_capability::ResetCapability()
{
    if (iRemoteCapability)
    {
        delete iRemoteCapability;
        iRemoteCapability = NULL;
    }
}

TSC_capability::~TSC_capability()
{
    ResetCapability();
}

Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>
TSC_capability::GetResolutions(TPVDirection dir)
{
    if (dir == OUTGOING)
    {
        return iResolutionsTx;
    }
    return iResolutionsRx;
}

void TSC_capability::SetVideoResolutions(TPVDirection dir,
        Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& resolutions)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::SetVideoResolutions dir(%d), size(%d)", dir, resolutions.size()));
    if (dir == OUTGOING)
    {
        iResolutionsTx = resolutions;
    }
    else if (dir == INCOMING)
    {
        iResolutionsRx = resolutions;
    }
}


uint32 TSC_capability::GetRemoteBitrate(PVCodecType_t codec_type)
{
    /* lookup the bitrate from remote capabilities */
    for (uint16 i = 0; i < iRemoteCapability->GetNumCapabilityItems();i++)
    {
        if (iRemoteCapability->GetCapabilityItem(i)->GetFormatType() == PVCodecTypeToPVMFFormatType(codec_type))
        {
            uint32 br = iRemoteCapability->GetCapabilityItem(i)->GetBitrate();
            return br;
        }
    }
    return 0;
}

void TSC_capability::ExtractTcsParameters(PS_VideoCapability pVideo, CPvtH263Capability *aMedia_capability)
{
    int frame_rate = GetMaxFrameRate_H263(pVideo->h263VideoCapability);
    if (pVideo->h263VideoCapability->option_of_sqcifMPI)
    {
        iTcsIn_H263_sqcifMPI = pVideo->h263VideoCapability->sqcifMPI;
        aMedia_capability->SetMaxResolution(128, 96, frame_rate);

    }
    if (pVideo->h263VideoCapability->option_of_qcifMPI)
    {
        iTcsIn_H263_qcifMPI = pVideo->h263VideoCapability->qcifMPI;
        aMedia_capability->SetMaxResolution(176, 144, frame_rate);
    }
    if (pVideo->h263VideoCapability->option_of_cifMPI)
    {
        iTcsIn_H263_cifMPI = pVideo->h263VideoCapability->cifMPI;
        aMedia_capability->SetMaxResolution(352, 288, frame_rate);
    }
    if (pVideo->h263VideoCapability->option_of_cif4MPI)
    {
        iTcsIn_H263_4cifMPI = pVideo->h263VideoCapability->cif4MPI;
        aMedia_capability->SetMaxResolution(704, 576, frame_rate);
    }
    if (pVideo->h263VideoCapability->option_of_cif16MPI)
    {
        iTcsIn_H263_16cifMPI = pVideo->h263VideoCapability->cif16MPI;
        aMedia_capability->SetMaxResolution(1408, 1192, frame_rate);
    }
}

void TSC_capability::ExtractTcsParameters(PS_VideoCapability pVideo, CPvtMpeg4Capability *aMedia_capability)
{
    int frame_rate = GetMaxFrameRate_M4V(pVideo->genericVideoCapability);
    aMedia_capability->SetMaxResolution(176, 144, frame_rate);
}

////////////////////////////////////////////////////////////////////////////
// ParseTcsCapabilities()
//
// This routine takes the incoming TerminalCapability
//   and parsed all capabilities - Audio, Video, UserInput.
//
////////////////////////////////////////////////////////////////////////////

void TSC_capability::ParseTcsCapabilities(S_Capability &aCapability, Oscl_Vector<CPvtMediaCapability*, OsclMemAllocator> &aMedia_capability, uint32 aUserInputCapabilities, S_UserInputCapability *aUserInputCapability)
{
    CodecCapabilityInfo codec_info;
    PS_VideoCapability pVideo = NULL;
    PS_AudioCapability pAudio = NULL;
    PVMFFormatType format_type = PVMF_MIME_FORMAT_UNKNOWN;

    pVideo = NULL;
    pAudio = NULL;
    switch (aCapability.index)
    {
        case 1: // ReceiveVideo
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote video caps ReceiveVideo\n"));
            pVideo = aCapability.receiveVideoCapability;
            break;
        case 3: // ReceiveAndTransmitVideo
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote video caps ReceiveAndTransmitVideo\n"));
            pVideo = aCapability.receiveAndTransmitVideoCapability;
            break;
        case 4:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote video caps ReceiveAudio\n"));
            pAudio = aCapability.receiveAudioCapability;
            break;
        case 6:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote video caps ReceiveAndTransmitAudio\n"));
            pAudio = aCapability.receiveAndTransmitAudioCapability;
            break;
        case 15:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote caps receiveUserInputCapability"));
            aUserInputCapability = aCapability.receiveUserInputCapability;
            break;
        case 17:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability: Remote caps receiveAndTransmitUserInputCapability"));
            aUserInputCapability = aCapability.receiveAndTransmitUserInputCapability;
            break;
        default:
            return;
    }
    GetCodecInfo(&aCapability, codec_info);
    if (codec_info.codec == PV_CODEC_TYPE_NONE)
        return;
    format_type = PVCodecTypeToPVMFFormatType(codec_info.codec);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::ParseTcsCapabilities CapabilityTable codec=%d,format=%s",
                     codec_info.codec, format_type.getMIMEStrPtr()));
    CPvtMediaCapability* media_capability = NULL;

    if (pVideo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::ParseTcsCapabilities Remote video caps Video index(%d)\n",
                         pVideo->index));
        if (format_type == PVMF_MIME_H2632000) 			// H263VideoCapability
        {
            pVideo->index = 3;
            media_capability = new CPvtH263Capability();

            media_capability->iBitrate = pVideo->h263VideoCapability->maxBitRate;
            // Extract H263 Parameters
            ExtractTcsParameters(pVideo, (CPvtH263Capability*)media_capability);
            ((CPvtH263Capability *)media_capability)->iH263VideoCapability = pVideo->h263VideoCapability;
            aMedia_capability.push_back(media_capability);

        }
        else if (format_type == PVMF_MIME_M4V)
        {

            media_capability = new CPvtMpeg4Capability();
            media_capability->iBitrate = pVideo->genericVideoCapability->maxBitRate;
            ExtractTcsParameters(pVideo, (CPvtMpeg4Capability*)media_capability);
            ((CPvtMpeg4Capability*)media_capability)->iGenericCapability = pVideo->genericVideoCapability;
            aMedia_capability.push_back(media_capability);
        }
    }
    else if (pAudio)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability: Remote caps Audio index(%d)\n", pAudio->index));
        if (pAudio->index == 8)
        {
            media_capability = new CPvtAudioCapability(format_type);
            aMedia_capability.push_back(media_capability);
            media_capability->iBitrate = 64;
        }
        else if (pAudio->index == 20)
        {
            media_capability = new CPvtAudioCapability(format_type);
            aMedia_capability.push_back(media_capability);
            media_capability->iBitrate = pAudio->genericAudioCapability->maxBitRate;
        }
    }
    else if (aUserInputCapability)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability: Remote caps UI index(%d)\n",
                         aUserInputCapability->index));
        aUserInputCapabilities |= (1 << aUserInputCapability->index);
    }


}

PS_DataType TSC_capability::GetOutgoingDataType(PVCodecType_t codecType,
        uint32 bitrate,
        uint16 csi_len,
        uint8* csi)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::GetOutgoingDataType - codecType(%d), bitrate(%d)", codecType, bitrate));

    PS_DataType pDataType = (PS_DataType) OSCL_DEFAULT_MALLOC(sizeof(S_DataType));
    oscl_memset(pDataType, 0, sizeof(S_DataType));
    PS_GenericCapability	genericCap = NULL;
    uint16 index = 0;

    /* lookup the bitrate from remote capabilities */
    for (index = 0; index < iRemoteCapability->GetNumCapabilityItems();index++)
    {
        if (iRemoteCapability->GetCapabilityItem(index)->GetFormatType() ==
                PVCodecTypeToPVMFFormatType(codecType))
        {
            bitrate = iRemoteCapability->GetCapabilityItem(index)->GetBitrate();
            break;

        }
    }
    switch (codecType)
    {
        case PV_AUD_TYPE_G723: /* WWURM: change H324_AUDIO_RECV to H324_AUDIO_SEND */
            /* (LCN=2): G723 Audio */
            PS_G7231		g723Cap;
            pDataType->index = 3;
            /* NEW245: allocate memory for audioData*/
            pDataType->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            pDataType->audioData->index = 8;
            /* NEW245: allocate memory for g7231 */
            pDataType->audioData->g7231 =
                g723Cap =
                    (PS_G7231) OSCL_DEFAULT_MALLOC(sizeof(S_G7231));
            g723Cap->maxAl_sduAudioFrames = 1;
            g723Cap->silenceSuppression = false;
            break;
        case PV_AUD_TYPE_GSM:
            /* (LCN=2): Amr Audio */
            pDataType->index = 3;
            /* NEW245: allocate memory for audioData */
            pDataType->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            pDataType->audioData->index = 20;
            /* NEW245: allocate memory for genericAudioCapability */
            pDataType->audioData->genericAudioCapability =
                genericCap =
                    (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            genericCap->capabilityIdentifier.index = 0;
            /* NEW245: allocate memory for standard */
            genericCap->capabilityIdentifier.standard = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            genericCap->capabilityIdentifier.standard->size = 7;
            genericCap->capabilityIdentifier.standard->data = (uint8*) OSCL_DEFAULT_MALLOC(7 * sizeof(uint8));
            genericCap->capabilityIdentifier.standard->data[0] = 0x00;
            genericCap->capabilityIdentifier.standard->data[1] = 0x08;
            genericCap->capabilityIdentifier.standard->data[2] = 0x81;
            genericCap->capabilityIdentifier.standard->data[3] = 0x75;
            genericCap->capabilityIdentifier.standard->data[4] = 0x01;
            genericCap->capabilityIdentifier.standard->data[5] = 0x01;
            genericCap->capabilityIdentifier.standard->data[6] = 0x01;
            genericCap->option_of_maxBitRate = true;
            genericCap->maxBitRate = bitrate;
            genericCap->option_of_collapsing = true;
            genericCap->size_of_collapsing = 1;
            genericCap->collapsing = (PS_GenericParameter) OSCL_DEFAULT_MALLOC(1 * sizeof(S_GenericParameter));
            genericCap->collapsing[0].parameterIdentifier.index = 0;
            genericCap->collapsing[0].parameterIdentifier.standard = 0;
            genericCap->collapsing[0].parameterValue.index = 2;
            genericCap->collapsing[0].parameterValue.unsignedMin = 1;
            genericCap->collapsing[0].option_of_supersedes = false;

            genericCap->option_of_nonCollapsing = false;
            genericCap->option_of_nonCollapsingRaw = false;
            genericCap->option_of_transport = false;
            break;
        case PV_VID_TYPE_H263:
            /* (LCN=3): H263 Video */
            PS_H263VideoCapability	h263VideoCap;
            pDataType->index = 2;
            /* NEW245: allocate memory for videoData */
            pDataType->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            oscl_memset(pDataType->videoData, 0, sizeof(S_VideoCapability));
            pDataType->videoData->index = 3;
            /* NEW245: allocate memory for h263VideoCapability */
            pDataType->videoData->h263VideoCapability =
                h263VideoCap =
                    (PS_H263VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H263VideoCapability));
            oscl_memset(pDataType->videoData->h263VideoCapability, 0, sizeof(S_H263VideoCapability));

            if (iTcsIn_H263_sqcifMPI && IsResolutionSupported(PVMF_RESOLUTION_SQCIF, iResolutionsTx))
            {
                h263VideoCap->option_of_sqcifMPI = true;
                h263VideoCap->sqcifMPI = (uint8)iTcsIn_H263_sqcifMPI;
            }

            if (iTcsIn_H263_qcifMPI && IsResolutionSupported(PVMF_RESOLUTION_QCIF, iResolutionsTx))
            {
                h263VideoCap->option_of_qcifMPI = true;
                h263VideoCap->qcifMPI = 2;
            }

            if (iTcsIn_H263_cifMPI && IsResolutionSupported(PVMF_RESOLUTION_CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cifMPI = true;
                h263VideoCap->cifMPI = (uint8)iTcsIn_H263_cifMPI;
            }

            if (iTcsIn_H263_4cifMPI && IsResolutionSupported(PVMF_RESOLUTION_4CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cif4MPI = true;
                h263VideoCap->cif4MPI = (uint8)iTcsIn_H263_4cifMPI;
            }

            if (iTcsIn_H263_16cifMPI && IsResolutionSupported(PVMF_RESOLUTION_16CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cif16MPI = true;
                h263VideoCap->cif16MPI = (uint8)iTcsIn_H263_16cifMPI;
            }

            h263VideoCap->option_of_cifMPI = false;
            h263VideoCap->option_of_cif4MPI = false;
            h263VideoCap->option_of_cif16MPI = false;
            h263VideoCap->maxBitRate = bitrate;
            h263VideoCap->unrestrictedVector = false;
            h263VideoCap->arithmeticCoding = false;
            h263VideoCap->advancedPrediction = false;
            h263VideoCap->pbFrames = false;
            h263VideoCap->temporalSpatialTradeOffCapability = ON;
            h263VideoCap->option_of_hrd_B = false;
            h263VideoCap->option_of_bppMaxKb = false;
            h263VideoCap->option_of_slowSqcifMPI = false;
            h263VideoCap->option_of_slowQcifMPI = false;
            h263VideoCap->option_of_slowCifMPI = false;
            h263VideoCap->option_of_slowCif4MPI = false;
            h263VideoCap->option_of_slowCif16MPI = false;
            h263VideoCap->errorCompensation = false;
            h263VideoCap->option_of_enhancementLayerInfo = false;
            h263VideoCap->option_of_h263Options = false;
            break;
        case PV_VID_TYPE_MPEG4:
        {
            /* (LCN=3): MPEG4 Video */
            pDataType->index = 2;
            /* NEW245: allocate memory for videoData */
            pDataType->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            pDataType->videoData->index = 5;
            /* NEW245: allocate memory for genericVideoCapability */
            pDataType->videoData->genericVideoCapability =
                genericCap =
                    (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            genericCap->capabilityIdentifier.index = 0;
            /* NEW245: allocate memory for standard */
            genericCap->capabilityIdentifier.standard = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            genericCap->capabilityIdentifier.standard->size = 7;
            genericCap->capabilityIdentifier.standard->data = (uint8*) OSCL_DEFAULT_MALLOC(7 * sizeof(uint8));
            genericCap->capabilityIdentifier.standard->data[0] = 0x00;
            genericCap->capabilityIdentifier.standard->data[1] = 0x08;
            genericCap->capabilityIdentifier.standard->data[2] = 0x81;
            genericCap->capabilityIdentifier.standard->data[3] = 0x75;
            genericCap->capabilityIdentifier.standard->data[4] = 0x01;
            genericCap->capabilityIdentifier.standard->data[5] = 0x00;
            genericCap->capabilityIdentifier.standard->data[6] = 0x00;
            genericCap->option_of_maxBitRate = true;
            genericCap->maxBitRate = bitrate;
            genericCap->option_of_collapsing = false;
            genericCap->option_of_nonCollapsing = true;
            genericCap->size_of_nonCollapsing = 3;
            genericCap->nonCollapsing = (PS_GenericParameter) OSCL_DEFAULT_MALLOC(3 * sizeof(S_GenericParameter));
            genericCap->nonCollapsing[0].parameterIdentifier.index = 0;
            genericCap->nonCollapsing[0].parameterIdentifier.standard = 0;
            genericCap->nonCollapsing[0].parameterValue.index = 3;
            // Value on next line changed to 8 (RAN - PandL)
            genericCap->nonCollapsing[0].parameterValue.unsignedMax = 8;	/* simple profile level 0 */
            genericCap->nonCollapsing[0].option_of_supersedes = false;

            genericCap->nonCollapsing[1].parameterIdentifier.index = 0;
            genericCap->nonCollapsing[1].parameterIdentifier.standard = 1;
            genericCap->nonCollapsing[1].parameterValue.index = 3;
            genericCap->nonCollapsing[1].parameterValue.unsignedMax = 1;	/* simple profile object */
            genericCap->nonCollapsing[1].option_of_supersedes = false;

            /* WWU_VOAL2: BLCMP4 temporally off */
            genericCap->nonCollapsing[2].parameterIdentifier.index = 0;
            genericCap->nonCollapsing[2].parameterIdentifier.standard = 2;
            genericCap->nonCollapsing[2].parameterValue.index = 6;
            /* NEW245: allocate memory for octetString */
            genericCap->nonCollapsing[2].parameterValue.octetString =
                (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
            if (csi && csi_len)
            {
                genericCap->nonCollapsing[2].parameterValue.octetString->data = (uint8*)OSCL_DEFAULT_MALLOC(csi_len);
                oscl_memcpy(genericCap->nonCollapsing[2].parameterValue.octetString->data,
                            csi,
                            csi_len);
                genericCap->nonCollapsing[2].parameterValue.octetString->size = csi_len;
            }
            else  // need to set it to FILLER FSI otherwise PER copy/delete routines will Leave
            {
                genericCap->nonCollapsing[2].parameterValue.octetString->data = (uint8*)OSCL_DEFAULT_MALLOC(PV2WAY_FILLER_FSI_LEN);
                SetFillerFsi(genericCap->nonCollapsing[2].parameterValue.octetString->data, PV2WAY_FILLER_FSI_LEN);
                genericCap->nonCollapsing[2].parameterValue.octetString->size = PV2WAY_FILLER_FSI_LEN;
            }
            genericCap->nonCollapsing[2].option_of_supersedes = false;

            genericCap->option_of_nonCollapsingRaw = false;
            genericCap->option_of_transport = false;
        }
        break;
        default:
            /* NULL data type */
            pDataType->index = 1;
    }
    return pDataType;
}

/* The following routine verifies if an incoming datatype is syntactically valid and supported */
PVMFStatus TSC_capability::ValidateIncomingDataType(bool forRev, PS_DataType pDataType)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::ValidateIncomingDataType(%d,%x)", forRev, pDataType));
    if (!pDataType)
        return PVMFFailure;

    if (pDataType->index == 1) // null data
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::ValidateIncomingDataType - null data type received"));
        if (forRev)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateIncomingDataType Error - null data not acceptable for forward parameters"));
            return PVMFFailure;
        }
        return PVMFSuccess;
    }
    else if (pDataType->index == 2)  // videoData
    {
        PS_VideoCapability video_cap = pDataType->videoData;
        if (video_cap->index == 3) /* H263 */
        {
            PS_H263VideoCapability	h263Cap = video_cap->h263VideoCapability;
            // checks only valid on forward parameters
            if (forRev)
            {
                // check if any unsupported resolution is indicated
                if ((h263Cap->option_of_sqcifMPI && !IsResolutionSupported(PVMF_RESOLUTION_SQCIF, iResolutionsRx)) ||
                        (h263Cap->option_of_qcifMPI && !IsResolutionSupported(PVMF_RESOLUTION_QCIF, iResolutionsRx)) ||
                        (h263Cap->option_of_cifMPI && !IsResolutionSupported(PVMF_RESOLUTION_CIF, iResolutionsRx)) ||
                        (h263Cap->option_of_cif4MPI && !IsResolutionSupported(PVMF_RESOLUTION_4CIF, iResolutionsRx)) ||
                        (h263Cap->option_of_cif16MPI && !IsResolutionSupported(PVMF_RESOLUTION_16CIF, iResolutionsRx)))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                    (0, "TSC_capability::ValidateIncomingDataType ERROR  - Unsuported resolution"));
                    return PVMFErrNotSupported;
                }
                // check if atleast one resolution is enabled
                if (!(h263Cap->option_of_sqcifMPI ||
                        h263Cap->option_of_qcifMPI ||
                        h263Cap->option_of_cifMPI ||
                        h263Cap->option_of_cif4MPI ||
                        h263Cap->option_of_cif16MPI))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                    (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid datatype for H.263.  No resolutions indicated."));
                    return PVMFFailure;
                }
            }
            else // checks only valid on reverse parameters
            {
                unsigned num_ok_resolutions = 0;
                if (h263Cap->option_of_sqcifMPI &&
                        IsResolutionSupported(PVMF_RESOLUTION_SQCIF, iResolutionsTx))
                {
                    if (h263Cap->sqcifMPI < 1 || h263Cap->sqcifMPI > 30)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                        (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid H263 SQCIF mpi"));
                    }
                    else
                        num_ok_resolutions++;
                }
                if (h263Cap->option_of_qcifMPI &&
                        IsResolutionSupported(PVMF_RESOLUTION_QCIF, iResolutionsTx))
                {
                    if (h263Cap->qcifMPI < 1 || h263Cap->qcifMPI > 30)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                        (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid H263 QCIF mpi"));
                    }
                    else
                        num_ok_resolutions++;
                }
                if (h263Cap->option_of_cifMPI &&
                        IsResolutionSupported(PVMF_RESOLUTION_CIF, iResolutionsTx))
                {
                    if (h263Cap->cifMPI < 1 || h263Cap->cifMPI > 30)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                        (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid H263 QCIF mpi"));
                    }
                    else
                        num_ok_resolutions++;
                }
                if (h263Cap->option_of_cif4MPI &&
                        IsResolutionSupported(PVMF_RESOLUTION_4CIF, iResolutionsTx))
                {
                    if (h263Cap->cif4MPI < 1 || h263Cap->cif4MPI > 30)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                        (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid H263 QCIF mpi"));
                    }
                    else
                        num_ok_resolutions++;
                }
                if (h263Cap->option_of_cif16MPI &&
                        IsResolutionSupported(PVMF_RESOLUTION_16CIF, iResolutionsTx))
                {
                    if (h263Cap->cif16MPI < 1 || h263Cap->cif16MPI > 30)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                        (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid H263 QCIF mpi"));
                    }
                    else
                        num_ok_resolutions++;
                }
                if (num_ok_resolutions == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                    (0, "TSC_capability::ValidateIncomingDataType ERROR  - Cannot transmit using this codec"));
                    return PVMFFailure;
                }
            }
            if (h263Cap->maxBitRate > 640)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                (0, "TSC_capability::ValidateIncomingDataType ERROR  - Invalid bitrate(%d)", h263Cap->maxBitRate));
                return PVMFFailure;
            }
        }
        else if (video_cap->index == 5) /* MPEG 4 */
        {
            //PS_GenericCapability m4vCap=video_cap->genericVideoCapability;
            uint8* fsi = NULL;
            unsigned fsisz =::GetFormatSpecificInfo(pDataType, fsi);
            if (fsi != NULL && fsisz != 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                (0, "TSC_capability::ValidateIncomingDataType VOL header dump"));
                printBuffer(iLogger, fsi, (uint16)fsisz);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                                (0, "TSC_capability::ValidateIncomingDataType VOL header not given"));
            }
        }
        else
        {
            return PVMFErrNotSupported;
        }
    }
    else if (pDataType->index == 3)  // audioData
    {
        PS_AudioCapability audio_cap = pDataType->audioData;
        if (audio_cap->index == 8) /* G.723 */
        {
            //PS_G7231 g723Cap = audio_cap->g7231;

        }
        else if (audio_cap->index == 20) /* AMR */
        {
            //PS_GenericCapability amrCap=audio_cap->genericAudioCapability;
        }
        else
        {

            return PVMFErrNotSupported;
        }
    }
    else
    {
        return PVMFErrNotSupported;
    }
    return PVMFSuccess;
}

/* The following routines return our preferences for DataType and H223LogicalChannelParameters */
bool TSC_capability::IsSegmentable(TPVDirection direction, PV2WayMediaType media_type)
{
    OSCL_UNUSED_ARG(direction);
    switch (media_type)
    {
        case PV_AUDIO:
            return false;
        case PV_VIDEO:
            return true;
        default:
            return true;
    }
}

PS_H223LogicalChannelParameters
TSC_capability::GetOutgoingLcnParams(PV2WayMediaType media_type, PS_AdaptationLayerType adaptation_layer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability:GetLcnParams - media_type(%d), layer(%x)", media_type, adaptation_layer));
    OSCL_ASSERT(media_type == PV_AUDIO || media_type == PV_VIDEO);
    PS_H223LogicalChannelParameters pParameter = (PS_H223LogicalChannelParameters)OSCL_DEFAULT_MALLOC(sizeof(S_H223LogicalChannelParameters));
    oscl_memset(pParameter , 0, sizeof(S_H223LogicalChannelParameters));

    pParameter->segmentableFlag = IsSegmentable(OUTGOING, media_type);
    pParameter->adaptationLayerType.index = adaptation_layer->index;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability: GetLcnParams al index(%d) \n", pParameter->adaptationLayerType.index));

    if (pParameter->adaptationLayerType.index == 5)
    {
        /* AL3(Video) */
        pParameter->adaptationLayerType.al3 = (PS_Al3) OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
        oscl_memcpy(pParameter->adaptationLayerType.al3, adaptation_layer->al3, sizeof(S_Al3));
        pParameter->adaptationLayerType.al3->sendBufferSize = 0;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability: GetLcnParams controlFieldOctets(%d),  sendBufferSize(%d)\n",
                         pParameter->adaptationLayerType.al3->controlFieldOctets,
                         pParameter->adaptationLayerType.al3->sendBufferSize));
    }
    return pParameter;
}

PVMFStatus TSC_capability::ValidateIncomingH223LcnParams(PS_H223LogicalChannelParameters h223params,
        TPVDirection dir)
{
    OSCL_UNUSED_ARG(dir);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::ValidateIncomingH223LcnParams(%x),dir(%d)",
                     h223params, dir));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::ValidateIncomingH223LcnParams AL index(%d)",
                     h223params->adaptationLayerType.index));
    if (h223params->adaptationLayerType.index == 0 ||
            h223params->adaptationLayerType.index > 5)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::ValidateIncomingH223LcnParams invalid index(%d)",
                         h223params->adaptationLayerType.index));
        return PVMFErrNotSupported;
    }

    if (h223params->adaptationLayerType.index == 5)
    {
        PS_Al3 al3 = h223params->adaptationLayerType.al3;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::ValidateIncomingH223LcnParams al3->controlFieldOctets(%d),al3->sendBufferSize(%d)", al3->controlFieldOctets, al3->sendBufferSize));
        if (al3->controlFieldOctets > 2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                            (0, "TSC_capability::ValidateIncomingH223LcnParams ERROR  - Invalid al3->controlFieldOctets(%d)", al3->controlFieldOctets));
            return PVMFFailure;
        }
    }
    return PVMFSuccess;
}

PVMFStatus TSC_capability::ValidateForwardReverseParams(PS_ForwardReverseParam forRevParams,
        TPVDirection dir)
{
    if (forRevParams == NULL || dir == PV_DIRECTION_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::ValidateForwardReverseParams forRevParams==NULL || dir==PV_DIRECTION_NONE"));
        return PVMFFailure;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::ValidateForwardReverseParams params(%x), option_of_reverse(%d)",
                     forRevParams,
                     forRevParams->option_of_reverseLogicalChannelParameters));
    if (dir&INCOMING)
    {
        PS_ForwardLogicalChannelParameters forwardParams = &forRevParams->forwardLogicalChannelParameters;
        /* validate datatype */
        PVMFStatus datatypeCheck = ValidateIncomingDataType(true, &forwardParams->dataType);
        if (datatypeCheck != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                            (0, "TSC_capability::ValidateForwardReverseParams ERROR  - forward datatype not supported."));
            return datatypeCheck;
        }
        /* Validate lcp */
        if (forwardParams->multiplexParameters.index != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateForwardReverseParams Invalid index for forward multipleParameters(%d)", forwardParams->multiplexParameters.index));
            return PVMFFailure;
        }
        if (forwardParams->multiplexParameters.h223LogicalChannelParameters == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateForwardReverseParams forward multipleParameters==NULL"));
            return PVMFFailure;
        }
        PVMFStatus h223paramsCheck = ValidateIncomingH223LcnParams(forwardParams->multiplexParameters.h223LogicalChannelParameters, INCOMING);
        if (h223paramsCheck != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                            (0, "TSC_capability::ValidateForwardReverseParams ERROR  - forward h223params not supported."));
            return h223paramsCheck;
        }
    }

    if (dir&OUTGOING)
    {
        if (!forRevParams->option_of_reverseLogicalChannelParameters)
            return PVMFSuccess;
        PS_ReverseLogicalChannelParameters reverseParams = &forRevParams->reverseLogicalChannelParameters;
        /* validate datatype */
        PVMFStatus datatypeCheck = ValidateIncomingDataType(false, &reverseParams->dataType);
        if (datatypeCheck != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                            (0, "TSC_capability::ValidateForwardReverseParams ERROR  - reverse datatype not supported."));
            return datatypeCheck;
        }
        if (!reverseParams->option_of_rlcMultiplexParameters)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateForwardReverseParams no option for reverse multipleParameters"));
            return PVMFFailure;
        }
        /* Validate lcp */
        if (reverseParams->rlcMultiplexParameters.index != 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateForwardReverseParams Invalid index for reverse multipleParameters(%d)", reverseParams->rlcMultiplexParameters.index));
            return PVMFFailure;
        }
        if (reverseParams->rlcMultiplexParameters.h223LogicalChannelParameters == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::ValidateForwardReverseParams reverse multipleParameters==NULL"));
            return PVMFFailure;
        }
        PVMFStatus h223paramsCheck = ValidateIncomingH223LcnParams(reverseParams->rlcMultiplexParameters.h223LogicalChannelParameters, OUTGOING);
        if (h223paramsCheck != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_EMERG,
                            (0, "TSC_capability::ValidateForwardReverseParams ERROR  - reverse h223params not supported."));
            return h223paramsCheck;
        }
    }
    return PVMFSuccess;
}

bool TSC_capability::VerifyReverseParameters(PS_ForwardReverseParam forRevParams,
        TSCObserver* aObserver,
        PVMFStatus& status)
{
    OSCL_UNUSED_ARG(aObserver);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::VerifyReverseParameters"));
    status = PVMFSuccess;
    status = ValidateForwardReverseParams(forRevParams, OUTGOING);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability:VerifyReverseParameters - Reverse params invalid"));
        return true;
    }

    PVCodecType_t codec = GetCodecType(&forRevParams->reverseLogicalChannelParameters.dataType);
    GetMediaType(codec);
    if (!CodecRequiresFsi(codec))
    {
        status = PVMFSuccess;
        return true;
    }
    status = PVMFFailure;
    return false;
}

CPvtTerminalCapability* TSC_capability::GetRemoteCapability()
{
    return iRemoteCapability;
}

uint32 TSC_capability::GetMaxBitrateForOutgoingChannel(PVCodecType_t codecType)
{
    uint32 bitrate = 0;
    /* lookup the bitrate from remote capabilities */
    for (uint16 i = 0; i < iRemoteCapability->GetNumCapabilityItems();i++)
    {
        if (iRemoteCapability->GetCapabilityItem(i)->GetFormatType() == PVCodecTypeToPVMFFormatType(codecType))
            bitrate = iRemoteCapability->GetCapabilityItem(i)->GetBitrate();
    }
    return bitrate;
}

PS_DataType
TSC_capability::GetDataType(PVCodecType_t codecType,
                            uint32 bitrate,
                            const uint8* dci,
                            uint16 dci_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_capability::GetDataType - codecType(%d),bitrate(%d),dci(%x),dci_len(%d)",
                     codecType, bitrate, dci, dci_len));
    bitrate /= 100;
    PS_DataType pDataType = (PS_DataType) OSCL_DEFAULT_MALLOC(sizeof(S_DataType));
    oscl_memset(pDataType , 0, sizeof(S_DataType));
    PS_GenericCapability	genericCap = NULL;

    switch (codecType)
    {
        case PV_AUD_TYPE_G723: /* WWURM: change H324_AUDIO_RECV to H324_AUDIO_SEND */
            /* (LCN=2): G723 Audio */
            PS_G7231		g723Cap;
            pDataType->index = 3;
            /* NEW245: allocate memory for audioData*/
            pDataType->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            pDataType->audioData->index = 8;
            /* NEW245: allocate memory for g7231 */
            pDataType->audioData->g7231 =
                g723Cap =
                    (PS_G7231) OSCL_DEFAULT_MALLOC(sizeof(S_G7231));
            g723Cap->maxAl_sduAudioFrames = 1;
            g723Cap->silenceSuppression = false;
            break;
        case PV_AUD_TYPE_GSM:
            /* (LCN=2): Amr Audio */
            pDataType->index = 3;
            /* NEW245: allocate memory for audioData */
            pDataType->audioData = (PS_AudioCapability) OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            pDataType->audioData->index = 20;
            /* NEW245: allocate memory for genericAudioCapability */
            pDataType->audioData->genericAudioCapability =
                genericCap =
                    (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            genericCap->capabilityIdentifier.index = 0;
            /* NEW245: allocate memory for standard */
            genericCap->capabilityIdentifier.standard = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            genericCap->capabilityIdentifier.standard->size = 7;
            genericCap->capabilityIdentifier.standard->data = (uint8*) OSCL_DEFAULT_MALLOC(7 * sizeof(uint8));
            genericCap->capabilityIdentifier.standard->data[0] = 0x00;
            genericCap->capabilityIdentifier.standard->data[1] = 0x08;
            genericCap->capabilityIdentifier.standard->data[2] = 0x81;
            genericCap->capabilityIdentifier.standard->data[3] = 0x75;
            genericCap->capabilityIdentifier.standard->data[4] = 0x01;
            genericCap->capabilityIdentifier.standard->data[5] = 0x01;
            genericCap->capabilityIdentifier.standard->data[6] = 0x01;
            genericCap->option_of_maxBitRate = true;
            genericCap->maxBitRate = bitrate;
            genericCap->option_of_collapsing = true;
            genericCap->size_of_collapsing = 1;
            genericCap->collapsing = (PS_GenericParameter) OSCL_DEFAULT_MALLOC(1 * sizeof(S_GenericParameter));
            genericCap->collapsing[0].parameterIdentifier.index = 0;
            genericCap->collapsing[0].parameterIdentifier.standard = 0;
            genericCap->collapsing[0].parameterValue.index = 2;
            genericCap->collapsing[0].parameterValue.unsignedMin = 1;
            genericCap->collapsing[0].option_of_supersedes = false;

            genericCap->option_of_nonCollapsing = false;
            genericCap->option_of_nonCollapsingRaw = false;
            genericCap->option_of_transport = false;
            break;
        case PV_VID_TYPE_H263:
            /* (LCN=3): H263 Video */
            PS_H263VideoCapability	h263VideoCap;
            pDataType->index = 2;
            /* NEW245: allocate memory for videoData */
            pDataType->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            oscl_memset(pDataType->videoData, 0, sizeof(S_VideoCapability));
            pDataType->videoData->index = 3;
            /* NEW245: allocate memory for h263VideoCapability */
            pDataType->videoData->h263VideoCapability =
                h263VideoCap =
                    (PS_H263VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_H263VideoCapability));
            oscl_memset(pDataType->videoData->h263VideoCapability, 0, sizeof(S_H263VideoCapability));

            if (iTcsIn_H263_sqcifMPI && IsResolutionSupported(PVMF_RESOLUTION_SQCIF, iResolutionsTx))
            {
                h263VideoCap->option_of_sqcifMPI = true;
                h263VideoCap->sqcifMPI = (uint8)iTcsIn_H263_sqcifMPI;
            }

            if (iTcsIn_H263_qcifMPI && IsResolutionSupported(PVMF_RESOLUTION_QCIF, iResolutionsTx))
            {
                h263VideoCap->option_of_qcifMPI = true;
                h263VideoCap->qcifMPI = 2;
            }

            if (iTcsIn_H263_cifMPI && IsResolutionSupported(PVMF_RESOLUTION_CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cifMPI = true;
                h263VideoCap->cifMPI = (uint8)iTcsIn_H263_cifMPI;
            }

            if (iTcsIn_H263_4cifMPI && IsResolutionSupported(PVMF_RESOLUTION_4CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cif4MPI = true;
                h263VideoCap->cif4MPI = (uint8)iTcsIn_H263_4cifMPI;
            }

            if (iTcsIn_H263_16cifMPI && IsResolutionSupported(PVMF_RESOLUTION_16CIF, iResolutionsTx))
            {
                h263VideoCap->option_of_cif16MPI = true;
                h263VideoCap->cif16MPI = (uint8)iTcsIn_H263_16cifMPI;
            }

            h263VideoCap->option_of_cifMPI = false;
            h263VideoCap->option_of_cif4MPI = false;
            h263VideoCap->option_of_cif16MPI = false;
            h263VideoCap->maxBitRate = bitrate;
            h263VideoCap->unrestrictedVector = false;
            h263VideoCap->arithmeticCoding = false;
            h263VideoCap->advancedPrediction = false;
            h263VideoCap->pbFrames = false;
            h263VideoCap->temporalSpatialTradeOffCapability = ON;
            h263VideoCap->option_of_hrd_B = false;
            h263VideoCap->option_of_bppMaxKb = false;
            h263VideoCap->option_of_slowSqcifMPI = false;
            h263VideoCap->option_of_slowQcifMPI = false;
            h263VideoCap->option_of_slowCifMPI = false;
            h263VideoCap->option_of_slowCif4MPI = false;
            h263VideoCap->option_of_slowCif16MPI = false;
            h263VideoCap->errorCompensation = false;
            h263VideoCap->option_of_enhancementLayerInfo = false;
            h263VideoCap->option_of_h263Options = false;
            break;
        case PV_VID_TYPE_MPEG4:
        {
            /* (LCN=3): MPEG4 Video */
            pDataType->index = 2;
            /* NEW245: allocate memory for videoData */
            pDataType->videoData = (PS_VideoCapability) OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            pDataType->videoData->index = 5;
            /* NEW245: allocate memory for genericVideoCapability */
            pDataType->videoData->genericVideoCapability =
                genericCap =
                    (PS_GenericCapability) OSCL_DEFAULT_MALLOC(sizeof(S_GenericCapability));
            genericCap->capabilityIdentifier.index = 0;
            /* NEW245: allocate memory for standard */
            genericCap->capabilityIdentifier.standard = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
            genericCap->capabilityIdentifier.standard->size = 7;
            genericCap->capabilityIdentifier.standard->data = (uint8*) OSCL_DEFAULT_MALLOC(7 * sizeof(uint8));
            genericCap->capabilityIdentifier.standard->data[0] = 0x00;
            genericCap->capabilityIdentifier.standard->data[1] = 0x08;
            genericCap->capabilityIdentifier.standard->data[2] = 0x81;
            genericCap->capabilityIdentifier.standard->data[3] = 0x75;
            genericCap->capabilityIdentifier.standard->data[4] = 0x01;
            genericCap->capabilityIdentifier.standard->data[5] = 0x00;
            genericCap->capabilityIdentifier.standard->data[6] = 0x00;
            genericCap->option_of_maxBitRate = true;
            genericCap->maxBitRate = bitrate;
            genericCap->option_of_collapsing = false;
            genericCap->option_of_nonCollapsing = true;
            genericCap->size_of_nonCollapsing = (uint16)((dci && dci_len) ? 3 : 2);
            genericCap->nonCollapsing = (PS_GenericParameter) OSCL_DEFAULT_MALLOC(3 * sizeof(S_GenericParameter));
            genericCap->nonCollapsing[0].parameterIdentifier.index = 0;
            genericCap->nonCollapsing[0].parameterIdentifier.standard = 0;
            genericCap->nonCollapsing[0].parameterValue.index = 3;
            // Value on next line changed to 8 (RAN - PandL)
            genericCap->nonCollapsing[0].parameterValue.unsignedMax = 8;	/* simple profile level 0 */
            genericCap->nonCollapsing[0].option_of_supersedes = false;

            genericCap->nonCollapsing[1].parameterIdentifier.index = 0;
            genericCap->nonCollapsing[1].parameterIdentifier.standard = 1;
            genericCap->nonCollapsing[1].parameterValue.index = 3;
            genericCap->nonCollapsing[1].parameterValue.unsignedMax = 1;	/* simple profile object */
            genericCap->nonCollapsing[1].option_of_supersedes = false;

            if (dci && dci_len)
            {
                /* WWU_VOAL2: BLCMP4 temporally off */
                genericCap->nonCollapsing[2].parameterIdentifier.index = 0;
                genericCap->nonCollapsing[2].parameterIdentifier.standard = 2;
                genericCap->nonCollapsing[2].parameterValue.index = 6;
                /* NEW245: allocate memory for octetString */
                genericCap->nonCollapsing[2].parameterValue.octetString =
                    (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
                genericCap->nonCollapsing[2].parameterValue.octetString->data =
                    (uint8*)OSCL_DEFAULT_MALLOC(dci_len);
                oscl_memcpy(genericCap->nonCollapsing[2].parameterValue.octetString->data,
                            dci, dci_len);
                genericCap->nonCollapsing[2].parameterValue.octetString->size = dci_len;
                genericCap->nonCollapsing[2].option_of_supersedes = false;
            }

            genericCap->option_of_nonCollapsingRaw = false;
            genericCap->option_of_transport = false;
        }
        break;

        default:
            /* NULL data type */
            pDataType->index = 1;
    }
    return pDataType;
}

uint16
TSC_capability::GetSupportedCodecCapabilityInfo(TPVDirection dir,
        PV2WayMediaType mediaType,
        Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& codec_info_list)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                    (0, "TSC_capability::GetSupportedCodecCapabilityInfo dir=%d, mediaType=%d",
                     dir, mediaType));
    Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>* channel_config =
        iTSCcomponent->GetChannelConfig(dir);
    if (channel_config == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "TSC_capability::GetSupportedCodecCapabilityInfo channel config for this direction == NULL"));
        return 0;
    }
    for (unsigned n = 0;n < channel_config->size();n++)
    {
        PV2WayMediaType channelMediaType = (*channel_config)[n].GetMediaType();
        if (channelMediaType != mediaType)
        {
            continue;
        }
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* formats = (*channel_config)[n].GetCodecs();
        if (!formats)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                            (0, "TSC_capability::GetSupportedCodecCapabilityInfo No formats specified for format type(%d)", channelMediaType));
            continue;
        }
        for (unsigned m = 0;m < formats->size();m++)
        {
            CodecCapabilityInfo* info = NULL;
            PVCodecType_t codec_type = PVMFFormatTypeToPVCodecType((*formats)[m].format);
            TPVDirection dir = (*formats)[m].dir;
            if (GetMediaType(codec_type) == PV_VIDEO)
            {
                info = new VideoCodecCapabilityInfo;
                ((VideoCodecCapabilityInfo*)info)->resolutions = (dir == OUTGOING) ? iResolutionsTx : iResolutionsRx;
            }
            else
            {
                info = new CodecCapabilityInfo;
            }
            info->codec = codec_type;
            info->dir = dir;
            codec_info_list.push_back(info);
        }
    }
    return (uint16)codec_info_list.size();
}

bool
TSC_capability::HasSymmetryConstraint(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& codec_list)
{
    for (unsigned i = 0;i < codec_list.size();i++)
    {
        if (codec_list[i]->dir == PV_DIRECTION_BOTH)
            return true;
    }
    return false;
}

CodecCapabilityInfo*
TSC_capability::SelectOutgoingCodec(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* remote_list,
                                    Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* local_list)
{
    Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* list1 = remote_list;
    Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>* list2 = local_list;
    if (iTSCstatemanager.ReadState(TSC_MSD_DECISION) == MASTER)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_capability::SelectOutgoingCodec Selecting our preferred codec for mediaType since we are master"));
        list1 = local_list;
        list2 = remote_list;
    }
    for (unsigned i = 0;i < (*list1).size();i++)
    {
        CodecCapabilityInfo* supported_codec_info =::IsSupported((*list1)[i], *list2);
        if (supported_codec_info == NULL)
            continue;
        // check if we support transmitting this codec
        FormatCapabilityInfo capability_info;
        if (iTSCcomponent->IsSupported(OUTGOING,
                                       supported_codec_info->codec, capability_info))
        {
            return (list1 == remote_list) ? (*list1)[i] : supported_codec_info;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TSC_capability::SelectOutgoingCodec Codec=%d not supported in outgoing direction",
                             supported_codec_info->codec));
        }
    }
    return NULL;
}

CodecCapabilityInfo*
TSC_capability::SelectOutgoingCodec(Oscl_Vector < CodecCapabilityInfo*,
                                    OsclMemAllocator > * remote_list)
{
    for (unsigned i = 0;i < (*remote_list).size();i++)
    {
        // check if this includes receive capability
        if ((*remote_list)[i]->dir == OUTGOING)
        {
            continue;
        }
        // check if we support transmitting this codec
        FormatCapabilityInfo capability_info;
        if (iTSCcomponent->IsSupported(OUTGOING,
                                       (*remote_list)[i]->codec, capability_info))
        {
            return (*remote_list)[i];
        }
    }
    return NULL;
}


