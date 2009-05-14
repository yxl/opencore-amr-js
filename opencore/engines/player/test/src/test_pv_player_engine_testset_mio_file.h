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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET_MIO_FILE_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET_MIO_FILE_H_INCLUDED

#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#include "test_pv_player_engine.h"
#endif
#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

class PVPlayerTestFileOutputMioFactory : public PVPlayerTestMioFactory
{
    public:
        PVPlayerTestFileOutputMioFactory() {}
        ~PVPlayerTestFileOutputMioFactory() {}

        PvmiMIOControl* CreateAudioOutput(OsclAny* aParam);
        PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedAudio = false);
        PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true);
        void DestroyAudioOutput(PvmiMIOControl* aMio);
        PvmiMIOControl* CreateVideoOutput(OsclAny* aParam);
        PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedVideo = false);
        PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true);
        void DestroyVideoOutput(PvmiMIOControl* aMio);
        PvmiMIOControl* CreateTextOutput(OsclAny* aParam);
        PvmiMIOControl* CreateTextOutput(OsclAny* aParam, MediaType aMediaType);
        void DestroyTextOutput(PvmiMIOControl* aMio);
};

#endif // TEST_PV_PLAYER_ENGINE_TESTSET_MIO_FILE_H_INCLUDED



