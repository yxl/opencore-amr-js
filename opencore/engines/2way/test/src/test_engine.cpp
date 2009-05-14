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
#include "test_engine.h"

#include "alloc_dealloc_test.h"
#include "init_test.h"
#include "init_cancel_test.h"
#ifndef NO_2WAY_324
#include "video_only_test.h"
#include "av_test.h"
#include "user_input_test.h"
#include "connect_test.h"
#include "connect_cancel_test.h"
#include "audio_only_test.h"
#include "av_duplicate_test.h"
#include "pvmf_fileoutput_factory.h"
#endif

#include "oscl_string_utils.h"
#include "oscl_mem_audit.h"


#include "tsc_h324m_config_interface.h"

#define AUDIO_FIRST 0
#define VIDEO_FIRST 1

#define MAX_SIP_TEST 27
#define MAX_324_TEST 25
#define SIP_TEST_OFFSET 200
#define SIP_TEST_MAP(x) (x+SIP_TEST_OFFSET)
#define NUM_SIP_ARGS 10


int start_test();

FILE* fileoutput;
cmd_line *global_cmd_line;

//Find test range args:
//To run a range of tests by enum ID:
//  -test 17 29

char engine_test::iProfileName[32] = "";
uint32 engine_test::iMediaPorts[2] = { 0, 0 };
char engine_test::iPeerAddress[64] = "";


void FindTestRange(cmd_line* command_line,
                   int32& iFirstTest,
                   int32 &iLastTest,
                   FILE* aFile)
{
    //default is to run all tests.
    iFirstTest = 0;
    iLastTest = MAX_324_TEST;

    int iTestArgument = 0;
    char *iTestArgStr1 = NULL;
    char *iTestArgStr2 = NULL;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-test" argument
    char *iSourceFind = NULL;
    if (cmdline_iswchar)
    {
        iSourceFind = new char[256];
    }

    int iTestSearch = 0;
    while (iTestSearch < count)
    {
        bool iTestFound = false;
        // Go through each argument
        for (; iTestSearch < count; iTestSearch++)
        {
            // Convert to UTF8 if necessary
            if (cmdline_iswchar)
            {
                OSCL_TCHAR* cmd = NULL;
                command_line->get_arg(iTestSearch, cmd);
                oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iSourceFind, 256);
            }
            else
            {
                iSourceFind = NULL;
                command_line->get_arg(iTestSearch, iSourceFind);
            }

            // Do the string compare
            if (oscl_strcmp(iSourceFind, "-help") == 0)
            {
                fprintf(aFile, "Test cases to run option. Default is ALL:\n");
                fprintf(aFile, "  -test x y\n");
                fprintf(aFile, "   Specify a range of test cases to run. To run one test case, use the\n");
                fprintf(aFile, "   same index for x and y.\n");

                fprintf(aFile, "  -test G\n");
                fprintf(aFile, "   Run 324M test cases only.\n");

                exit(0);
            }
            else if (oscl_strcmp(iSourceFind, "-test") == 0)
            {
                iTestFound = true;
                iTestArgument = ++iTestSearch;
                break;
            }
        }

        if (cmdline_iswchar)
        {
            delete[] iSourceFind;
            iSourceFind = NULL;
        }

        if (iTestFound)
        {
            // Convert to UTF8 if necessary
            if (cmdline_iswchar)
            {
                iTestArgStr1 = new char[256];
                OSCL_TCHAR* cmd;
                command_line->get_arg(iTestArgument, cmd);
                if (cmd)
                {
                    oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr1, 256);
                }

                iTestArgStr2 = new char[256];
                command_line->get_arg(iTestArgument + 1, cmd);
                if (cmd)
                {
                    oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iTestArgStr2, 256);
                }
            }
            else
            {
                command_line->get_arg(iTestArgument, iTestArgStr1);
                command_line->get_arg(iTestArgument + 1, iTestArgStr2);
            }

            //Pull out 2 integers...
            if (iTestArgStr1
                    && '0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9'
                    && iTestArgStr2
                    && '0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
            {
                int len = oscl_strlen(iTestArgStr1);
                switch (len)
                {
                    case 3:
                        iFirstTest = 0;
                        if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                        {
                            iFirstTest = iFirstTest + 100 * (iTestArgStr1[0] - '0');
                        }

                        if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                        {
                            iFirstTest = iFirstTest + 10 * (iTestArgStr1[1] - '0');
                        }

                        if ('0' <= iTestArgStr1[2] && iTestArgStr1[2] <= '9')
                        {
                            iFirstTest = iFirstTest + 1 * (iTestArgStr1[2] - '0');
                        }
                        break;

                    case 2:
                        iFirstTest = 0;
                        if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                        {
                            iFirstTest = iFirstTest + 10 * (iTestArgStr1[0] - '0');
                        }

                        if ('0' <= iTestArgStr1[1] && iTestArgStr1[1] <= '9')
                        {
                            iFirstTest = iFirstTest + 1 * (iTestArgStr1[1] - '0');
                        }
                        break;

                    case 1:
                        iFirstTest = 0;
                        if ('0' <= iTestArgStr1[0] && iTestArgStr1[0] <= '9')
                        {
                            iFirstTest = iFirstTest + 1 * (iTestArgStr1[0] - '0');
                        }
                        break;

                    default:
                        break;
                }

                len = oscl_strlen(iTestArgStr2);
                switch (len)
                {
                    case 3:
                        iLastTest = 0;
                        if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                        {
                            iLastTest = iLastTest + 100 * (iTestArgStr2[0] - '0');
                        }

                        if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                        {
                            iLastTest = iLastTest + 10 * (iTestArgStr2[1] - '0');
                        }

                        if ('0' <= iTestArgStr2[2] && iTestArgStr2[2] <= '9')
                        {
                            iLastTest = iLastTest + 1 * (iTestArgStr2[2] - '0');
                        }
                        break;

                    case 2:
                        iLastTest = 0;
                        if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                        {
                            iLastTest = iLastTest + 10 * (iTestArgStr2[0] - '0');
                        }

                        if ('0' <= iTestArgStr2[1] && iTestArgStr2[1] <= '9')
                        {
                            iLastTest = iLastTest + 1 * (iTestArgStr2[1] - '0');
                        }
                        break;

                    case 1:
                        iLastTest = 0;
                        if ('0' <= iTestArgStr2[0] && iTestArgStr2[0] <= '9')
                        {
                            iLastTest = iLastTest + 1 * (iTestArgStr2[0] - '0');
                        }
                        break;

                    default:
                        break;
                }
            }

#ifndef NO_2WAY_324
            else if (iTestArgStr1
                     && iTestArgStr1[0] == 'G')
            {
                //download tests
                iFirstTest = 0;
                iLastTest = MAX_324_TEST;
            }
#endif
        }

        if (cmdline_iswchar)
        {
            if (iTestArgStr1)
            {
                delete[] iTestArgStr1;
                iTestArgStr1 = NULL;
            }

            if (iTestArgStr2)
            {
                delete[] iTestArgStr2;
                iTestArgStr2 = NULL;
            }

            if (iSourceFind)
            {
                delete[] iSourceFind;
                iSourceFind = NULL;
            }
        }
        iTestSearch += 2;
    }
}


engine_test_suite::engine_test_suite() : test_case()
{
    // setting iProxy
    //proxy_tests(false);

    proxy_tests(true);
}


void engine_test_suite::proxy_tests(const bool aProxy)
{
    //Basic 2way tests
    fprintf(fileoutput, "Basic engine tests.\n");

    int32 firstTest = 0;
    int32 lastTest = MAX_324_TEST;
    FindTestRange(global_cmd_line, firstTest, lastTest, fileoutput);
#ifndef NO_2WAY_324
    if (firstTest == 0)
        adopt_test_case(new alloc_dealloc_test(aProxy));
    if (firstTest <= 2 && lastTest >= 2)
        adopt_test_case(new init_test(aProxy, 1));
    if (firstTest <= 3 && lastTest >= 3)
        adopt_test_case(new init_test(aProxy, 2));

    if (firstTest <= 4 && lastTest >= 4)
        adopt_test_case(new init_cancel_test(aProxy));

    if (firstTest <= 5 && lastTest >= 5)
        adopt_test_case(new connect_test(aProxy, 1));

    if (firstTest <= 6 && lastTest >= 6)
        adopt_test_case(new connect_cancel_test(aProxy));

    if (firstTest <= 7 && lastTest >= 7)
    {
        adopt_test_case(new audio_only_test(aProxy, PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2));
    }
    if (firstTest <= 8 && lastTest >= 8)
    {
        adopt_test_case(new audio_only_test(aProxy, PVMF_MIME_PCM16, PVMF_MIME_AMR_IF2));
    }
    if (firstTest <= 9 && lastTest >= 9)
    {
        adopt_test_case(new audio_only_test(aProxy, PVMF_MIME_AMR_IF2, PVMF_MIME_PCM16));
    }

    if (firstTest <= 10 && lastTest >= 10)
    {
        adopt_test_case(new audio_only_test(aProxy, PVMF_MIME_PCM16, PVMF_MIME_PCM16));
    }

    if (firstTest <= 11 && lastTest >= 11)
    {
        adopt_test_case(new video_only_test(PVMF_MIME_YUV420, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 12 && lastTest >= 12)
    {
        adopt_test_case(new video_only_test(PVMF_MIME_YUV420, PVMF_MIME_H2632000, aProxy));
    }
    if (firstTest <= 13 && lastTest >= 13)
    {
        adopt_test_case(new video_only_test(PVMF_MIME_H2632000, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 14 && lastTest >= 14)
    {
        adopt_test_case(new video_only_test(PVMF_MIME_M4V, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 15 && lastTest >= 15)
    {
        adopt_test_case(new video_only_test(PVMF_MIME_YUV420, PVMF_MIME_M4V, aProxy));
    }

    if (firstTest <= 16 && lastTest >= 16)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420, aProxy));
    }

    if (firstTest <= 17 && lastTest >= 17)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 18 && lastTest >= 18)
    {
        adopt_test_case(new av_test(PVMF_MIME_PCM16, PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420, aProxy));

    }
    if (firstTest <= 19 && lastTest >= 19)
    {
        adopt_test_case(new av_test(PVMF_MIME_PCM16, PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420, aProxy));
    }

    if (firstTest <= 20 && lastTest >= 20)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_H2632000, aProxy));
    }
    if (firstTest <= 21 && lastTest >= 21)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_H2632000, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 22 && lastTest >= 22)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_H2632000, PVMF_MIME_H2632000, aProxy));
    }

    if (firstTest <= 23 && lastTest >= 23)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_M4V, aProxy));
    }
    if (firstTest <= 24 && lastTest >= 24)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_M4V, PVMF_MIME_YUV420, aProxy));
    }
    if (firstTest <= 25 && lastTest >= 25)
    {
        adopt_test_case(new av_test(PVMF_MIME_AMR_IF2, PVMF_MIME_AMR_IF2, PVMF_MIME_M4V, PVMF_MIME_M4V, aProxy));
    }

    if (firstTest <= 26 && lastTest >= 26)
    {
        adopt_test_case(new connect_test(aProxy, 1, true));
    }


#endif


}

void engine_test::create_sink_source()
{
#ifndef NO_2WAY_324
    iCommSettings.iMediaFormat = PVMF_MIME_H223;
    iCommSettings.iTestObserver = NULL;
    iCommServerIOControl = PvmiMIOCommLoopbackFactory::Create(iCommSettings);
    bool enableBitstreamLogging = true;
    iCommServer = PVCommsIONodeFactory::Create(iCommServerIOControl, enableBitstreamLogging);
#endif

    // create the audio source
    iAudioSourceFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
    iAudioSourceFileSettings.iLoopInputFile = true;
    iAudioSourceFileSettings.iFileName = AUDIO_SOURCE_FILENAME;
    iAudioSourceFileSettings.iSamplingFrequency = 8000;
    iAudioSourceFileSettings.iNumChannels = 1;
    iAudioSourceIOControl = PvmiMIOFileInputFactory::Create(iAudioSourceFileSettings);
    iAudioSource = PvmfMediaInputNodeFactory::Create(iAudioSourceIOControl);

    iAudioSourceRawFileSettings.iMediaFormat = PVMF_MIME_PCM16;
    iAudioSourceRawFileSettings.iLoopInputFile = true;
    iAudioSourceRawFileSettings.iFileName = AUDIO_SOURCE_RAW_FILENAME;
    iAudioSourceRawFileSettings.iSamplingFrequency = 8000;
    iAudioSourceRawFileSettings.iNumChannels = 1;
    iAudioSourceRawIOControl = PvmiMIOFileInputFactory::Create(iAudioSourceRawFileSettings);
    iAudioSourceRaw = PvmfMediaInputNodeFactory::Create(iAudioSourceRawIOControl);

    iAudioSource2FileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
    iAudioSource2FileSettings.iLoopInputFile = true;
    iAudioSource2FileSettings.iFileName = AUDIO_SOURCE_FILENAME;
    iAudioSource2FileSettings.iSamplingFrequency = 8000;
    iAudioSource2FileSettings.iNumChannels = 1;
    iAudioSource2IOControl = PvmiMIOFileInputFactory::Create(iAudioSource2FileSettings);
    iAudioSource2 = PvmfMediaInputNodeFactory::Create(iAudioSource2IOControl);

    iAudioSource3FileSettings.iMediaFormat = PVMF_MIME_AMR_IETF;
    iAudioSource3FileSettings.iLoopInputFile = true;
    iAudioSource3FileSettings.iFileName = AUDIO_SOURCE3_FILENAME;
    iAudioSource3FileSettings.iSamplingFrequency = 8000;
    iAudioSource3FileSettings.iNum20msFramesPerChunk = 1;
    iAudioSource3FileSettings.iNumChannels = 1;
    iAudioSource3IOControl = PvmiMIOFileInputFactory::Create(iAudioSource3FileSettings);
    iAudioSource3 = PvmfMediaInputNodeFactory::Create(iAudioSource3IOControl);

    // create the audio sinks

    iAudioSinkFileName = AUDIO_SINK_FILENAME;
    iAudioSinkIOControl = new PVRefFileOutput(iAudioSinkFileName, MEDIATYPE_AUDIO, true);
    iAudioSink = PVMediaOutputNodeFactory::CreateMediaOutputNode(iAudioSinkIOControl);

    iAudioSinkRawFileName = AUDIO_SINK_RAW_FILENAME;
    iAudioSinkRawIOControl = new PVRefFileOutput(iAudioSinkRawFileName, MEDIATYPE_AUDIO, false);
    iAudioSinkRaw = PVMediaOutputNodeFactory::CreateMediaOutputNode(iAudioSinkRawIOControl);

    iAudioSink2FileName = AUDIO_SINK2_FILENAME;
    iAudioSink2IOControl = new PVRefFileOutput(iAudioSink2FileName, MEDIATYPE_AUDIO, true);
    iAudioSink2 = PVMediaOutputNodeFactory::CreateMediaOutputNode(iAudioSink2IOControl);

    // create the video sources
    iVideoSourceYUVFileSettings.iMediaFormat = PVMF_MIME_YUV420;
    iVideoSourceYUVFileSettings.iLoopInputFile = true;
    iVideoSourceYUVFileSettings.iFileName = VIDEO_SOURCE_YUV_FILENAME;
    iVideoSourceYUVFileSettings.iTimescale = 1000;
    iVideoSourceYUVFileSettings.iFrameHeight = 144;
    iVideoSourceYUVFileSettings.iFrameWidth = 176;
    iVideoSourceYUVFileSettings.iFrameRate = 5;
    iVideoSourceYUVIOControl = PvmiMIOFileInputFactory::Create(iVideoSourceYUVFileSettings);
    iVideoSourceYUV = PvmfMediaInputNodeFactory::Create(iVideoSourceYUVIOControl);


    iVideoSourceH263FileSettings.iMediaFormat = PVMF_MIME_H2632000;
    iVideoSourceH263FileSettings.iLoopInputFile = true;
    iVideoSourceH263FileSettings.iFileName = VIDEO_SOURCE_H263_FILENAME;
    iVideoSourceH263FileSettings.iTimescale = 1000;
    iVideoSourceH263FileSettings.iFrameHeight = 144;
    iVideoSourceH263FileSettings.iFrameWidth = 176;
    iVideoSourceH263FileSettings.iFrameRate = 5;


    iVideoSourceH263IOControl = PvmiMIOFileInputFactory::Create(iVideoSourceH263FileSettings);
    iVideoSourceH263 = PvmfMediaInputNodeFactory::Create(iVideoSourceH263IOControl);

    // create another video source
    iVideoSourceM4VFileSettings.iMediaFormat = PVMF_MIME_M4V;
    iVideoSourceM4VFileSettings.iLoopInputFile = true;
    iVideoSourceM4VFileSettings.iFileName = VIDEO_SOURCE_M4V_FILENAME;
    iVideoSourceM4VFileSettings.iTimescale = 1000;
    iVideoSourceM4VFileSettings.iFrameHeight = 144;
    iVideoSourceM4VFileSettings.iFrameWidth = 176;
    iVideoSourceM4VFileSettings.iFrameRate = 5;

    iVideoSourceM4VIOControl = PvmiMIOFileInputFactory::Create(iVideoSourceM4VFileSettings);
    iVideoSourceM4V = PvmfMediaInputNodeFactory::Create(iVideoSourceM4VIOControl);

    // create the video sinks
    iVideoSinkYUVFileName = VIDEO_SINK_YUV_FILENAME;
    iVideoSinkYUVIOControl = new PVRefFileOutput(iVideoSinkYUVFileName, MEDIATYPE_VIDEO, false);

    iVideoSinkYUV = PVMediaOutputNodeFactory::CreateMediaOutputNode(iVideoSinkYUVIOControl);

    iVideoSinkH263FileName = VIDEO_SINK_H263_FILENAME;
    iVideoSinkH263IOControl = new PVRefFileOutput(iVideoSinkH263FileName, MEDIATYPE_VIDEO, true);
    iVideoSinkH263 = PVMediaOutputNodeFactory::CreateMediaOutputNode(iVideoSinkH263IOControl);


    iVideoSinkM4VFileName = VIDEO_SINK_M4V_FILENAME;
    iVideoSinkM4VIOControl = new PVRefFileOutput(iVideoSinkM4VFileName, MEDIATYPE_VIDEO, true);
    iVideoSinkM4V = PVMediaOutputNodeFactory::CreateMediaOutputNode(iVideoSinkM4VIOControl);

}

void engine_test::destroy_sink_source()
{
#ifndef NO_2WAY_324
    if (iCommServer)
    {
        PVCommsIONodeFactory::Delete(iCommServer);
        iCommServer = NULL;
    }

    if (iCommServerIOControl)
    {
        PvmiMIOCommLoopbackFactory::Delete(iCommServerIOControl);
        iCommServerIOControl = NULL;
    }
#endif
    if (iAudioSource)
    {
        PvmfMediaInputNodeFactory::Delete(iAudioSource);
        iAudioSource = NULL;
    }

    if (iAudioSourceRaw)
    {
        PvmfMediaInputNodeFactory::Delete(iAudioSourceRaw);
        iAudioSourceRaw = NULL;
    }

    if (iAudioSource2)
    {
        PvmfMediaInputNodeFactory::Delete(iAudioSource2);
        iAudioSource2 = NULL;
    }

    if (iAudioSource3)
    {
        PvmfMediaInputNodeFactory::Delete(iAudioSource3);
        iAudioSource3 = NULL;
    }

    if (iAudioSourceIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSourceIOControl);
        iAudioSourceIOControl = NULL;
    }

    if (iAudioSourceRawIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSourceRawIOControl);
        iAudioSourceRawIOControl = NULL;
    }

    if (iAudioSource2IOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSource2IOControl);
        iAudioSource2IOControl = NULL;
    }

    if (iAudioSource3IOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSource3IOControl);
        iAudioSource3IOControl = NULL;
    }

    if (iVideoSourceYUV)
    {
        PvmfMediaInputNodeFactory::Delete(iVideoSourceYUV);
        iVideoSourceYUV = NULL;
    }

    if (iVideoSourceH263)
    {
        PvmfMediaInputNodeFactory::Delete(iVideoSourceH263);
        iVideoSourceH263 = NULL;
    }

    if (iVideoSourceM4V)
    {
        PvmfMediaInputNodeFactory::Delete(iVideoSourceM4V);
        iVideoSourceM4V = NULL;
    }

    if (iVideoSourceYUVIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSourceYUVIOControl);
        iVideoSourceYUVIOControl = NULL;
    }

    if (iVideoSourceM4VIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSourceM4VIOControl);
        iVideoSourceM4VIOControl = NULL;
    }

    if (iVideoSourceH263IOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSourceH263IOControl);
        iVideoSourceH263IOControl = NULL;
    }

    if (iAudioSink)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioSink);
        iAudioSink = NULL;
    }

    if (iAudioSinkRaw)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioSinkRaw);
        iAudioSinkRaw = NULL;
    }

    if (iAudioSink2)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioSink2);
        iAudioSink2 = NULL;
    }
    if (iAudioSinkIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSinkIOControl);
        iAudioSinkIOControl = NULL;
    }

    if (iAudioSinkRawIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSinkRawIOControl);
        iAudioSinkRawIOControl = NULL;
    }

    if (iAudioSink2IOControl)
    {
        PvmiMIOFileInputFactory::Delete(iAudioSinkIOControl);
        iAudioSink2IOControl = NULL;
    }
    if (iVideoSinkYUV)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoSinkYUV);
        iVideoSinkYUV = NULL;
    }

    if (iVideoSinkH263)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoSinkH263);
        iVideoSinkH263 = NULL;
    }

    if (iVideoSinkM4V)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoSinkM4V);
        iVideoSinkM4V = NULL;
    }

    if (iVideoSinkYUVIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSinkYUVIOControl);
        iVideoSinkYUVIOControl = NULL;
    }

    if (iVideoSinkM4VIOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSinkM4VIOControl);
        iVideoSinkM4VIOControl = NULL;
    }

    if (iVideoSinkH263IOControl)
    {
        PvmiMIOFileInputFactory::Delete(iVideoSinkH263IOControl);
        iVideoSinkH263IOControl = NULL;
    }

}

PVMFNodeInterface *engine_test::get_audio_source(PVMFFormatType format)
{
    if (format == PVMF_MIME_AMR_IF2)
        return iAudioSource;
    else if (format ==  PVMF_MIME_PCM16)
        return iAudioSourceRaw;
    else
        return NULL;

}

PVMFNodeInterface *engine_test::get_audio_sink(PVMFFormatType format)
{
    if (format == PVMF_MIME_AMR_IF2)
        return iAudioSink;
    if (format == PVMF_MIME_PCM16)
        return iAudioSinkRaw;
    else
        return NULL;
}

PVMFNodeInterface *engine_test::get_video_source(PVMFFormatType format)
{
    if (format ==  PVMF_MIME_YUV420)
        return iVideoSourceYUV;
    else if (format ==  PVMF_MIME_M4V)
        return iVideoSourceM4V;
    else if (format ==  PVMF_MIME_H2632000)
        return iVideoSourceH263;
    else
        return NULL;
}

PVMFNodeInterface *engine_test::get_video_sink(PVMFFormatType format)
{
    if (format == PVMF_MIME_YUV420)
        return iVideoSinkYUV;
    else if (format == PVMF_MIME_M4V)
        return iVideoSinkM4V;
    else if (format == PVMF_MIME_H2632000 || format == PVMF_MIME_H2631998)
        return iVideoSinkH263;
    else
        return NULL;
}

void engine_test::init_mime_strings()
{
}
int test_wrapper()
{
    int result;

    OsclErrorTrap::Init();
    OsclScheduler::Init("PV2WayEngineFactory");

    result = start_test();

    OsclScheduler::Cleanup();
    OsclErrorTrap::Cleanup();

    return result;
}


int local_main(FILE* filehandle, cmd_line *command_line)
{
    OSCL_UNUSED_ARG(command_line);
    int result;
    global_cmd_line = command_line;

    fileoutput = filehandle;
    fprintf(fileoutput, "Test Program for PV Engine class.\n");

    CPV2WayEngineFactory::Init();
#ifndef OSCL_BYPASS_MEMMGT
#ifndef NDEBUG
#ifdef MEM_AUDIT_2WAY
    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    auditCB.pAudit->MM_SetMode(auditCB.pAudit->MM_GetMode() |
                               MM_AUDIT_VALIDATE_ON_FREE_FLAG | MM_AUDIT_ALLOC_NODE_ENABLE_FLAG);
#endif
#endif
#endif
    result = test_wrapper();

    PVLogger::Cleanup();
#ifndef OSCL_BYPASS_MEMMGT
#ifndef NDEBUG
#ifdef MEM_AUDIT_2WAY
    //Check for memory leaks before cleaning up OsclMem.

    uint32 leaks = 0;

    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            fprintf(fileoutput, "Memory Stats:\n");
            fprintf(fileoutput, "  peakNumAllocs %d\n", stats->peakNumAllocs);
            fprintf(fileoutput, "  peakNumBytes %d\n", stats->peakNumBytes);
            fprintf(fileoutput, "  numAllocFails %d\n", stats->numAllocFails);
            if (stats->numAllocs)
            {
                fprintf(fileoutput, "  ERROR: Memory Leaks! numAllocs %d, numBytes %d\n", stats->numAllocs, stats->numBytes);
            }
        }
        leaks = auditCB.pAudit->MM_GetNumAllocNodes();
        if (leaks != 0)
        {
            fprintf(fileoutput, "ERROR: %d Memory leaks detected!\n", leaks);
            MM_AllocQueryInfo*info = auditCB.pAudit->MM_CreateAllocNodeInfo(leaks);
            uint32 leakinfo = auditCB.pAudit->MM_GetAllocNodeInfo(info, leaks, 0);
            if (leakinfo != leaks)
            {
                fprintf(fileoutput, "ERROR: Leak info is incomplete.\n");
            }
            for (uint32 i = 0;i < leakinfo;i++)
            {
                fprintf(fileoutput, "Leak Info:\n");
                fprintf(fileoutput, "  allocNum %d\n", info[i].allocNum);
                fprintf(fileoutput, "  fileName %s\n", info[i].fileName);
                fprintf(fileoutput, "  lineNo %d\n", info[i].lineNo);
                fprintf(fileoutput, "  size %d\n", info[i].size);
                fprintf(fileoutput, "  pMemBlock 0x%x\n", info[i].pMemBlock);
                fprintf(fileoutput, "  tag %s\n", info[i].tag);
            }
            auditCB.pAudit->MM_ReleaseAllocNodeInfo(info);
        }
    }
#endif
#endif
#endif
    PVLogger::Init();
    CPV2WayEngineFactory::Cleanup();

    return (result);
}


int start_test()
{
    int32 leave;
    engine_test_suite engine_tests;

    OSCL_TRY(leave, engine_tests.run_test());

    if (leave != 0)
        fprintf(fileoutput, "Leave %d\n", leave);

    text_test_interpreter interp;
    _STRING rs = interp.interpretation(engine_tests.last_result());
    fprintf(fileoutput, rs.c_str());
    const test_result the_result = engine_tests.last_result();

    return(the_result.success_count() != the_result.total_test_count());
}

#if (LINUX_MAIN==1)

int main(int argc, char *argv[])
{
    local_main(stdout, NULL);
    return 0;
}

#endif

