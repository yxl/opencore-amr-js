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
#ifndef PVMF_RECOGNIZER_TYPES_H_INCLUDED
#define PVMF_RECOGNIZER_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif


typedef Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> PVMFRecognizerMIMEStringList;

typedef enum _PVMFRecognizerConfidence
{
    PVMFRecognizerConfidenceNotCertain,		// 100% sure not the format
    PVMFRecognizerConfidenceNotPossible,	// Maybe not the format
    PVMFRecognizerConfidenceUnknown,		// Not sure one way or the other
    PVMFRecognizerConfidencePossible,		// Maybe the format
    PVMFRecognizerConfidenceCertain			// 100% sure of the format
} PVMFRecognizerConfidence;


class PVMFRecognizerResult
{
    public:
        PVMFRecognizerResult()
        {
        };

        // Copy constructor for use in Oscl_Vector
        PVMFRecognizerResult(const PVMFRecognizerResult& aSrc)
        {
            iRecognizedFormat = aSrc.iRecognizedFormat;
            iRecognitionConfidence = aSrc.iRecognitionConfidence;
            //	iRecognizerSubFormatList=aSrc.iRecognizerSubFormatList;
        };

        ~PVMFRecognizerResult()
        {
        };

        // The format of interest as a MIME string
        OSCL_HeapString<OsclMemAllocator> iRecognizedFormat;
        // The confidence level of recognition
        PVMFRecognizerConfidence iRecognitionConfidence;
        // If the format is a container format, the format of content within
//	Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator> iRecognizerSubFormatList;
};


/**
 * PVMFRecognizerCommmandHandler Class
 *
 * PVMFRecognizerCommmandHandler is the PVMF Recognizer observer class for notifying the
 * status of asynchronous requests. The API provides a mechanism for the status of each
 * command to be passed back along with context specific information where applicable.
 * User of the recognizer registry must have a class derived from PVMFNodeCmdStatusObserver
 * and implement the pure virtual function in order to receive event notifications from
 * PVMF Recognizer Registry.
 **/
class PVMFRecognizerCommmandHandler
{
    public:
        /**
         * Handle an event that has been generated.
         *
         * @param aResponse
         *        The response to a previously issued command
         */
        virtual void RecognizerCommandCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual ~PVMFRecognizerCommmandHandler() {}
};

#endif // PVMF_RECOGNIZER_TYPES_H_INCLUDED

