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
#ifndef AVI_TEST_H_INCLUDED
#define AVI_TEST_H_INCLUDED

#include "test_engine.h"
#ifndef PVMI_MIO_AVIFILE_FACTORY_H_INCLUDED
#include "pvmi_mio_avi_wav_file_factory.h"
#endif

class avi_test : public engine_test
{
    public:
        avi_test(bool aUseProxy,
                 PVMFFormatType audio_src_format = PVMF_MIME_PCM16,
                 PVMFFormatType audio_sink_format = PVMF_MIME_PCM16,
                 PVMFFormatType video_src_format = PVMF_MIME_YUV420,
                 PVMFFormatType video_sink_format = PVMF_MIME_YUV420)
                : engine_test(aUseProxy, 1) ,
                iAudSrcFormatType(audio_src_format),
                iAudSinkFormatType(audio_sink_format),
                iVidSrcFormatType(video_src_format),
                iVidSinkFormatType(video_sink_format),
                iPVAviFile(NULL),
                iFileParser(NULL),
                iAudioMediaInput(NULL),
                iVideoMediaInput(NULL)
        {}

        ~avi_test()
        {
            delete iPVAviFile;
            PVAviFile::DeleteAviFileParser((PVAviFile*)iFileParser);
            delete iAudioMediaInput;
            delete iVideoMediaInput;
        }

        void test();

        void Run();

        void DoCancel();

        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        void CommandCompleted(const PVCmdResponse& aResponse);

        void TimerCallback();

        bool HandleAvi();

    private:
        bool start_async_test();
        PVMFFormatType iAudSrcFormatType, iAudSinkFormatType;
        PVMFFormatType iVidSrcFormatType, iVidSinkFormatType;
        PVAviFile*	iPVAviFile;
        OsclAny*				 iFileParser;
        Oscl_FileServer          iFileServer;
        uint32 iAudioNum;
        uint32 iVideoNum;
        PvmiMIOControl* iAudioMediaInput;
        PvmiMIOControl* iVideoMediaInput;
};


#endif


