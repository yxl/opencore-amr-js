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
#include "pvamrffrec_plugin.h"
#include "pvfile.h"
#include "oscl_file_io.h"

PVMFStatus PVAMRFFRecognizerPlugin::SupportedFormats(PVMFRecognizerMIMEStringList& aSupportedFormatsList)
{
    // Return AMR as supported type
    OSCL_HeapString<OsclMemAllocator> supportedformat = PVMF_MIME_AMRFF;
    aSupportedFormatsList.push_back(supportedformat);
    return PVMFSuccess;
}


PVMFStatus PVAMRFFRecognizerPlugin::Recognize(PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHint,
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult)
{
    OSCL_UNUSED_ARG(aFormatHint);
    OSCL_wHeapString<OsclMemAllocator> tmpfilename;
    Oscl_FileServer fileServ;
    PVFile pvfile;
    pvfile.SetCPM(&aSourceDataStreamFactory);

    if (!(pvfile.Open(tmpfilename.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, fileServ)))
    {
        char* readData = NULL;

        readData = (char*)(oscl_malloc(sizeof(char) * AMRFF_MIN_DATA_SIZE_FOR_RECOGNITION));
        if (readData != NULL)
        {
            int bytesRead = 0;
            bytesRead = pvfile.Read(readData, sizeof(char), AMRFF_MIN_DATA_SIZE_FOR_RECOGNITION);
            if (bytesRead != AMRFF_MIN_DATA_SIZE_FOR_RECOGNITION)
            {
                pvfile.Close();
                oscl_free(readData);
                return PVMFFailure;
            }
            if (readData[0] == '#' && readData[1] == '!' && readData[2] == 'A' && readData[3] == 'M' && readData[4] == 'R')
            {
                PVMFRecognizerResult result;
                result.iRecognizedFormat = PVMF_MIME_AMRFF;
                result.iRecognitionConfidence = PVMFRecognizerConfidenceCertain;
                aRecognizerResult.push_back(result);
            }
        }
        pvfile.Close();
        oscl_free(readData);
        return PVMFFailure;
    }
    else
    {
        return PVMFFailure;
    }
    return PVMFSuccess;
}


PVMFStatus PVAMRFFRecognizerPlugin::GetRequiredMinBytesForRecognition(uint32& aBytes)
{
    aBytes = AMRFF_MIN_DATA_SIZE_FOR_RECOGNITION;
    return PVMFSuccess;
}


