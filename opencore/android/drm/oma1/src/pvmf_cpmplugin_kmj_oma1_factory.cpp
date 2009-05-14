/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "pvmf_cpmplugin_kmj_oma1.h"
#include "pvmf_cpmplugin_kmj_oma1_factory.h"

PVMFCPMPluginInterface* PVMFOma1KmjPluginFactory::CreateCPMPlugin()
{
    return PVMFCPMKmjPlugInOMA1::CreatePlugIn();
}

void PVMFOma1KmjPluginFactory::DestroyCPMPlugin(PVMFCPMPluginInterface* aPlugIn)
{
    PVMFCPMKmjPlugInOMA1::DestroyPlugIn(aPlugIn);
}


