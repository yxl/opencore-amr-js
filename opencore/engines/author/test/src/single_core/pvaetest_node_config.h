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
/**
 * @file pvaetest_node_config.h
 * @brief Utility to perform build configuration specific node configuration. This file
 * is for tye single core build configurations.
 */

#ifndef PVAETEST_NODE_CONFIG_H_INCLUDED
#define PVAETEST_NODE_CONFIG_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVAETESTINPUT_H_INCLUDED
#include "pvaetestinput.h"
#endif
#ifndef PVMF_AUDIO_ENCNODE_EXTENSION_H_INCLUDED
#include "pvmf_audio_encnode_extension.h"
#endif
#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H
#include "test_pv_author_engine_typedefs.h"
#endif
#ifndef PV_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif

extern const uint32 KNum20msFramesPerChunk;
extern const uint32 KAudioBitrate;
extern const uint32 KAudioBitrateWB;
extern const uint32 KAACAudioBitrate;

class PVAETestNodeConfig
{
    public:
        static bool ConfigureAudioEncoder(PVInterface* aInterface, const PvmfMimeString& aMimeType, uint32 aAudioBitrate = 0)
        {
            if (!aInterface)
            {
                // if the interface is missing, this method should not be called
                OSCL_ASSERT(aInterface != NULL);
                return false;
            }


            PVAudioEncExtensionInterface* config = OSCL_STATIC_CAST(PVAudioEncExtensionInterface*, aInterface);

            if ((pv_mime_strcmp(aMimeType.get_cstr(), (char*)KAMRNbEncMimeType) == 0) || (pv_mime_strcmp(aMimeType.get_cstr(), (char*)KAMRWbEncMimeType) == 0))
            {
                if (config->SetMaxNumOutputFramesPerBuffer(KNum20msFramesPerChunk) != PVMFSuccess)
                    return false;

                uint32 audioBitrate = aAudioBitrate;

                if (audioBitrate == 0)
                {
                    if (pv_mime_strcmp(aMimeType.get_cstr(), (char*)KAMRNbEncMimeType) == 0)
                        audioBitrate = KAudioBitrate;
                    else if (pv_mime_strcmp(aMimeType.get_cstr(), (char*)KAMRWbEncMimeType) == 0)
                        audioBitrate = KAudioBitrateWB;
                }

                switch (audioBitrate)
                {
                    case 4750:
                        if (config->SetOutputBitRate(GSM_AMR_4_75) != PVMFSuccess)
                            return false;
                        break;
                    case 5150:
                        if (config->SetOutputBitRate(GSM_AMR_5_15) != PVMFSuccess)
                            return false;
                        break;
                    case 5900:
                        if (config->SetOutputBitRate(GSM_AMR_5_90) != PVMFSuccess)
                            return false;
                        break;
                    case 6700:
                        if (config->SetOutputBitRate(GSM_AMR_6_70) != PVMFSuccess)
                            return false;
                        break;
                    case 7400:
                        if (config->SetOutputBitRate(GSM_AMR_7_40) != PVMFSuccess)
                            return false;
                        break;
                    case 7950:
                        if (config->SetOutputBitRate(GSM_AMR_7_95) != PVMFSuccess)
                            return false;
                        break;
                    case 10200:
                        if (config->SetOutputBitRate(GSM_AMR_10_2) != PVMFSuccess)
                            return false;
                        break;
                    case 12200:
                        if (config->SetOutputBitRate(GSM_AMR_12_2) != PVMFSuccess)
                            return false;
                        break;

                    case 6600: // AMR WB bitrates start here
                        if (config->SetOutputBitRate(GSM_AMR_6_60) != PVMFSuccess)
                            return false;
                        break;
                    case 8850:
                        if (config->SetOutputBitRate(GSM_AMR_8_85) != PVMFSuccess)
                            return false;
                        break;
                    case 12650:
                        if (config->SetOutputBitRate(GSM_AMR_12_65) != PVMFSuccess)
                            return false;
                        break;
                    case 14250:
                        if (config->SetOutputBitRate(GSM_AMR_14_25) != PVMFSuccess)
                            return false;
                        break;
                    case 15850:
                        if (config->SetOutputBitRate(GSM_AMR_15_85) != PVMFSuccess)
                            return false;
                        break;
                    case 18250:
                        if (config->SetOutputBitRate(GSM_AMR_18_25) != PVMFSuccess)
                            return false;
                        break;
                    case 19850:
                        if (config->SetOutputBitRate(GSM_AMR_19_85) != PVMFSuccess)
                            return false;
                        break;
                    case 23050:
                        if (config->SetOutputBitRate(GSM_AMR_23_05) != PVMFSuccess)
                            return false;
                        break;
                    case 23850:
                        if (config->SetOutputBitRate(GSM_AMR_23_85) != PVMFSuccess)
                            return false;
                        break;
                    default:
                        return false;
                }
            }
            else if (aMimeType == KAACADIFEncMimeType || aMimeType == KAACADTSEncMimeType)
            {
                if (config->SetOutputBitRate(KAACAudioBitrate) != PVMFSuccess)
                    return false;
                //config->SetOutputNumChannel();  do not set, use the input ones
                //config->SetOutputSamplingRate();
            }

            return true;
        }
};

#endif // PVAETEST_NODE_CONFIG_H_INCLUDED



