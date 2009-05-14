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
#ifndef PVAACFFREC_PLUGIN_H_INCLUDED
#define PVAACFFREC_PLUGIN_H_INCLUDED

#ifndef PVMF_RECOGNIZER_PLUGIN_H_INCLUDED
#include "pvmf_recognizer_plugin.h"
#endif

class PVAACFFRecognizerPlugin : public PVMFRecognizerPluginInterface
{
    public:
        PVAACFFRecognizerPlugin()
        {
        };

        ~PVAACFFRecognizerPlugin()
        {
        };

        PVMFStatus SupportedFormats(PVMFRecognizerMIMEStringList& aSupportedFormatsList);

        PVMFStatus Recognize(PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHint,
                             Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult);

        PVMFStatus GetRequiredMinBytesForRecognition(uint32& aBytes);
};

#endif // PVAACFFREC_PLUGIN_H_INCLUDED

