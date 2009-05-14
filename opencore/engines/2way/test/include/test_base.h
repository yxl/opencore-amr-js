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
#ifndef TEST_BASE_H_INCLUDED
#define TEST_BASE_H_INCLUDED

#include "test_engine.h"

#define TEST_DURATION 10000

class test_base : public engine_test,
            public H324MConfigObserver
{
    public:

        test_base(PVMFFormatType aAudSrcFormatType,
                  PVMFFormatType aAudSinkFormatType,
                  PVMFFormatType aVidSrcFormatType,
                  PVMFFormatType aVidSinkFormatType,
                  bool aUseProxy = false,
                  int aMaxRuns = 1,
                  bool isSIP = false) :
                engine_test(aUseProxy, aMaxRuns),
                iH324MConfig(NULL),
                iEncoderIFCommandId(-1),
                i324mIFCommandId(-1),
                iCancelCmdId(-1),
                iQueryInterfaceCmdId(-1),
                iStackIFSet(false),
                iEncoderIFSet(false),
                iSIP(isSIP),
                iAudSrcFormatType(aAudSrcFormatType),
                iAudSinkFormatType(aAudSinkFormatType),
                iVidSrcFormatType(aVidSrcFormatType),
                iVidSinkFormatType(aVidSinkFormatType),
                iTempH324MConfigIterface(NULL)
        {
        }

        virtual ~test_base()
        {
        }

        template<class DestructClass>
        class AppenderDestructDealloc : public OsclDestructDealloc
        {
            public:
                virtual void destruct_and_dealloc(OsclAny *ptr)
                {
                    delete((DestructClass*)ptr);
                }
        };

    protected:
        void InitializeLogs();

        virtual bool Init();
        virtual void CommandCompleted(const PVCmdResponse& aResponse);

        void CreateH324Component(bool aCreateH324 = true);

        void H324MConfigCommandCompletedL(PVMFCmdResp& aResponse);
        void H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification);

        virtual void QueryInterfaceSucceeded();

        //------------------- Functions overridden in test classes for specific behavior------
        virtual bool start_async_test();

        virtual void InitSucceeded();
        virtual void InitFailed();
        virtual void InitCancelled();

        virtual void ConnectSucceeded();
        virtual void ConnectFailed();
        virtual void ConnectCancelled();

        virtual void CancelCmdCompleted();


        virtual void RstCmdCompleted();
        virtual void DisCmdSucceeded();
        virtual void DisCmdFailed();

        virtual void EncoderIFSucceeded();
        virtual void EncoderIFFailed();


        // audio
        virtual void AudioAddSinkCompleted();
        virtual void AudioAddSourceCompleted();
        virtual void AudioRemoveSourceCompleted();
        virtual void AudioRemoveSinkCompleted();

        // video
        virtual void VideoAddSinkSucceeded();
        virtual void VideoAddSinkFailed();
        virtual void VideoAddSourceSucceeded();
        virtual void VideoAddSourceFailed();
        virtual void VideoRemoveSourceCompleted();
        virtual void VideoRemoveSinkCompleted();
        //------------------- END Functions overridden in test classes for specific behavior------

        H324MConfigInterface* iH324MConfig;
        PVCommandId iEncoderIFCommandId;
        PVCommandId i324mIFCommandId;
        PVCommandId iCancelCmdId;
        PVCommandId iQueryInterfaceCmdId;

        bool iStackIFSet;
        bool iEncoderIFSet;
        bool iSIP;

        PVMFFormatType iAudSrcFormatType, iAudSinkFormatType;
        PVMFFormatType iVidSrcFormatType, iVidSinkFormatType;
        PVInterface* iTempH324MConfigIterface;
};


#endif


