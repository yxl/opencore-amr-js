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
 * @file pvaetestinput.cpp
 * @brief Test input to PVAuthorEngine unit test, using single core encoders
 */

#ifndef PVAETESTINPUT_H_INCLUDED
#include "pvaetestinput.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_MEDIA_INPUT_NODE_FACTORY_H_INCLUDED
#include "pvmf_media_input_node_factory.h"
#endif
#ifndef PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED
#include "pvmi_mio_fileinput_factory.h"
#endif
#ifndef TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_author_engine_config.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

extern const uint32 KVideoTimescale;
extern const uint32 KNum20msFramesPerChunk;
extern const uint32 KTextTimescale;
extern const uint32 KTextFrameWidth;
extern const uint32 KTextFrameHeight;


////////////////////////////////////////////////////////////////////////////
PVAETestInput::PVAETestInput(const PVAETestInput& aInput)
{
    Copy(aInput);
}

////////////////////////////////////////////////////////////////////////////
PVAETestInput& PVAETestInput::operator=(const PVAETestInput & aInput)
{
    Copy(aInput);
    return (*this);
}

////////////////////////////////////////////////////////////////////////////
bool PVAETestInput::IsTestInputTypeSupported(PVAETestInputType aType)
{
    switch (aType)
    {
        case YUV_FILE:
        case H263_FILE:
        case H264_FILE:
        case AMR_IETF_FILE:
        case AAC_ADIF_FILE:
        case AAC_ADTS_FILE:
        case PCM16_FILE:
        case YUV_WRONG_FILE:
        case TEXT_FILE:
        case AMRWB_IETF_FILE:
            return true;

        case M4V_FILE:
            return false;

        default:
            return false;
    }
}

////////////////////////////////////////////////////////////////////////////
int PVAETestInput::CreateInputNode(PVAETestInputType aType, const OSCL_wString& aFileName, AVTConfig iAVTConfig)
{
    int status = false;
    switch (aType)
    {
        case YUV_FILE:
        case H263_FILE:
        case H264_FILE:
        case AMR_IETF_FILE:
        case AAC_ADIF_FILE:
        case AAC_ADTS_FILE:
        case PCM16_FILE:
        case YUV_WRONG_FILE:
        case TEXT_FILE:
        case AMRWB_IETF_FILE:
            status = CreateMIOInputNode(aType, aFileName, iAVTConfig);
            break;

        case M4V_FILE:
        default:
            status = -1;
            break;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
bool PVAETestInput::DeleteInputNode()
{
    for (uint32 i = 0; i < iExtensions.size(); i++)
        iExtensions[i]->removeRef();
    iExtensions.clear();

    switch (iType)
    {
        case YUV_FILE:
        case H263_FILE:
        case H264_FILE:
        case AMR_IETF_FILE:
        case AAC_ADIF_FILE:
        case AAC_ADTS_FILE:
        case PCM16_FILE:
        case YUV_WRONG_FILE:
        case TEXT_FILE:
        case AMRWB_IETF_FILE:
            if (iNode)
                PvmfMediaInputNodeFactory::Delete(iNode);
            if (iMediaInput)
                PvmiMIOFileInputFactory::Delete(iMediaInput);
        case M4V_FILE:
        default:
            break;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
PVAETestInput::PVAETestInput()
{
    iLogger = PVLogger::GetLoggerObject("PVAETestInput");
    iNode = NULL;
    iType = INVALID_INPUT_TYPE;
    iMediaInput = NULL;
}

////////////////////////////////////////////////////////////////////////////
void PVAETestInput::Copy(const PVAETestInput& aInput)
{
    iNode = aInput.iNode;
    iType = aInput.iType;
    iSettings = aInput.iSettings;
    iExtensions = aInput.iExtensions;
    iMediaInput = aInput.iMediaInput;
}

////////////////////////////////////////////////////////////////////////////
bool PVAETestInput::CreateMIOInputNode(PVAETestInputType aType, const OSCL_wString& aFileName, AVTConfig iAVTConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVAETestInput::CreateMIOInputNode: aType=%d", aType));

    iType = aType;

    if (aFileName.get_size() != 0)
    {
        iSettings.iFileName = aFileName;
    }
    switch (aType)
    {

        case H263_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KH263TestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_H2631998;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iTimescale = KVideoTimescale;
            iSettings.iFrameHeight = iAVTConfig.iHeight; //KVideoFrameHeight;
            iSettings.iFrameWidth = iAVTConfig.iWidth;//KVideoFrameWidth;
            iSettings.iFrameRate = iAVTConfig.iFps;//KVideoFrameRate;
            break;

        case H264_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KH264TestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_YUV420;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iTimescale = KVideoTimescale;
            iSettings.iFrameHeight = iAVTConfig.iHeight; //KVideoFrameHeight;
            iSettings.iFrameWidth = iAVTConfig.iWidth;//KVideoFrameWidth;
            iSettings.iFrameRate = iAVTConfig.iFps;//KVideoFrameRate;
            break;

        case YUV_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KYUVTestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_YUV420;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iTimescale = KVideoTimescale;
            iSettings.iFrameHeight = iAVTConfig.iHeight;
            iSettings.iFrameWidth = iAVTConfig.iWidth;
            iSettings.iFrameRate = iAVTConfig.iFps;
            break;

        case AMR_IETF_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KAMRTestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_AMR_IETF;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iSamplingFrequency = iAVTConfig.iSamplingRate; //KAudioTimescale;
            iSettings.iNumChannels = iAVTConfig.iNumChannels;// KAudioNumChannels;
            iSettings.iNum20msFramesPerChunk = KNum20msFramesPerChunk;
            break;

        case AMRWB_IETF_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KAMRWBTestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_AMRWB_IETF;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iSamplingFrequency = 16000; //AMR-WB is always sampled at 16KHz
            iSettings.iNumChannels = 1; //AMR-WB is always mono
            iSettings.iNum20msFramesPerChunk = KNum20msFramesPerChunk;
            break;

        case AAC_ADIF_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KAACADIFTestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_ADIF;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iSamplingFrequency = iAVTConfig.iSamplingRate; //KAudioTimescale;
            iSettings.iNumChannels = iAVTConfig.iNumChannels;// KAudioNumChannels;
            iSettings.iNum20msFramesPerChunk = KNum20msFramesPerChunk;
            break;
        case AAC_ADTS_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KAACADTSTestInput;
            }
            iSettings.iMediaFormat = PVMF_MIME_ADTS;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iSamplingFrequency = iAVTConfig.iSamplingRate; //KAudioTimescale;
            iSettings.iNumChannels = iAVTConfig.iNumChannels;// KAudioNumChannels;
            iSettings.iNum20msFramesPerChunk = KNum20msFramesPerChunk;
            break;

        case TEXT_FILE:
            if (aFileName.get_size() == 0)
            {
                iSettings.iFileName = KTEXTTestInput;
                iSettings.iLogFileName = KTEXTLogTestInput;
                iSettings.iTextFileName = KTEXTTxtFileTestInput;
            }
            else
            {
                if (iAVTConfig.iTextLogFile.get_size() == 0)
                {
                    iSettings.iLogFileName = KTEXTLogTestInput;
                }
                else
                {
                    oscl_wchar output1[ARRAY_SIZE];
                    oscl_UTF8ToUnicode(iAVTConfig.iTextLogFile.get_cstr(), iAVTConfig.iTextLogFile.get_size(), output1, ARRAY_SIZE);
                    iSettings.iLogFileName.set(output1, oscl_strlen(output1));
                }

                if (iAVTConfig.iTextConfigFile.get_size() == 0)
                {
                    iSettings.iTextFileName = KTEXTTxtFileTestInput;
                }
                else
                {
                    oscl_wchar output2[ARRAY_SIZE];
                    oscl_UTF8ToUnicode(iAVTConfig.iTextConfigFile.get_cstr(), iAVTConfig.iTextConfigFile.get_size(), output2, ARRAY_SIZE);
                    iSettings.iTextFileName.set(output2, oscl_strlen(output2));
                }
            }
            iSettings.iMediaFormat = PVMF_MIME_3GPP_TIMEDTEXT;
            iSettings.iLoopInputFile = iAVTConfig.iLoopingEnable;
            iSettings.iSamplingFrequency = 0;
            iSettings.iNumChannels = 0;
            iSettings.iNum20msFramesPerChunk = 0;
            iSettings.iTimescale = KTextTimescale;
            iSettings.iFrameHeight = KTextFrameWidth;
            iSettings.iFrameWidth = KTextFrameHeight;
            break;
        default:
            return false;
    }

    iMediaInput = PvmiMIOFileInputFactory::Create(iSettings);
    if (!iMediaInput)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVAETestInput::CreateMIOInputNode: Error - PvmiMIOFileInputFactory::Create failed"));
        return false;
    }

    iNode = PvmfMediaInputNodeFactory::Create(iMediaInput);
    if (!iNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVAETestInput::CreateMIOInputNode: Error - PvmfMediaInputNodeFactory::Create failed"));
        return false;
    }

    return true;
}



