/* ------------------------------------------------------------------
 * Copyright (C) 2008 PacketVideo
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

#include "test_pv_player_engine_main.h"
#include "pvmi_media_io_fileoutput.h"

PvmiMIOControl* PVPlayerTestMioFactory::CreateAudioOutput(OsclAny* aParam)
{
	return new PVRefFileOutput(((OSCL_wString*)aParam)->get_cstr());
}

PvmiMIOControl* PVPlayerTestMioFactory::CreateAudioOutput(
	OsclAny* aParam, 
	PVRefFileOutputTestObserver* aObserver, 
	bool aActiveTiming,
	uint32 aQueueLimit,
	bool aSimFlowControl,
	bool logStrings)
{
	return new PVRefFileOutput(
		*(OSCL_wString*)aParam,
		aObserver,
		aActiveTiming,
		aQueueLimit,
		aSimFlowControl,
		logStrings);
}

void PVPlayerTestMioFactory::DestroyAudioOutput(PvmiMIOControl* aMio)
{
	PVRefFileOutput* mio=(PVRefFileOutput*)aMio;
	delete mio;
}

PvmiMIOControl* PVPlayerTestMioFactory::CreateVideoOutput(OsclAny* aParam)
{
	return new PVRefFileOutput(((OSCL_wString*)aParam)->get_cstr());
}

PvmiMIOControl* PVPlayerTestMioFactory::CreateVideoOutput(
        OsclAny* aParam,
        PVRefFileOutputTestObserver* aObserver,
        bool aActiveTiming,
        uint32 aQueueLimit,
        bool aSimFlowControl,
	bool logStrings)
{
        return new PVRefFileOutput(
                *(OSCL_wString*)aParam,
                aObserver,
                aActiveTiming,
                aQueueLimit,
                aSimFlowControl,
                logStrings);
}

void PVPlayerTestMioFactory::DestroyVideoOutput(PvmiMIOControl* aMio)
{
	PVRefFileOutput* mio=(PVRefFileOutput*)aMio;
	delete mio;
}

PvmiMIOControl* PVPlayerTestMioFactory::CreateTextOutput(OsclAny* aParam)
{
	return new PVRefFileOutput(((OSCL_wString*)aParam)->get_cstr());
}

void PVPlayerTestMioFactory::DestroyTextOutput(PvmiMIOControl* aMio)
{
	PVRefFileOutput* mio=(PVRefFileOutput*)aMio;
	delete mio;
}




