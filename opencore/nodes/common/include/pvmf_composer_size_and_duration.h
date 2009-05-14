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
#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#define PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

/** Uuid of PVMFComposerSizeAndDurationEvent group of events */
#define PvmfComposerSizeAndDurationEventsUuid PVUuid(0x68cd880f,0xf0fa,0x4ed9,0xaa,0x39,0x6f,0x33,0xc8,0xda,0xb1,0x82)

/**
 * Enumerated list of informational events for maximum file size and duration, and
 * recording progress report.
 */
typedef enum
{
    PVMF_COMPOSER_MAXFILESIZE_REACHED = 2048,	/**< Maximum file size reached */
    PVMF_COMPOSER_MAXDURATION_REACHED,	/**< Maximum duration reached */
    PVMF_COMPOSER_FILESIZE_PROGRESS,	/**< Recording progress report in terms of file size written */
    PVMF_COMPOSER_DURATION_PROGRESS,		/**< Recording progress report in terms of duration recorded */
    PVMF_COMPOSER_EOS_REACHED
} PVMFComposerSizeAndDurationEvent;

/** Uuid of PvmfComposerSizeAndDurationInterface */
#define PvmfComposerSizeAndDurationUuid PVUuid(0x09b87b61,0x34bc,0x4ccb,0x85,0x04,0x53,0x93,0xdb,0x5f,0x35,0x2d)

/**
 * Configuration interface for maximum file size and duration, and recording progress
 * reporting features.  To use the features provided by this interface, the client
 * must implement PVInformationalEventObserver interface to receive events generated.
 */
class PvmfComposerSizeAndDurationInterface : public PVInterface
{
    public:

        /**
         * Set maximum file size.
         *
         * If maximum file size is enabled, the composer will stop the recording session
         * when it detects that writing further media data to the output file will cause the
         * size of output file to go beyond the maximum size.  The client will receive
         * an informational event PVMF_COMPOSER_MAXFILESIZE_REACHED after the maximum file
         * size is reached and the recording session has stopped successfully.  This method
         * is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Set to true to enable maximum file size, else set to false.
         * @param aMaxFileSizeBytes If aEnable is set to true, this parameter specifies
         *        the maximum file size in bytes, else this parameter is ignored.
         * @return Completion status.
         */
        virtual PVMFStatus SetMaxFileSize(bool aEnable, uint32 aMaxFileSizeBytes) = 0;

        /**
         * Query maximum file size configuration.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if maximum file size
         *                is enabled, else false.
         * @param aMaxFileSizeBytes Output parameter which will be set to the maximum file
         *        size in bytes if aEnable is set to true, else this parameter is ignored.
         */
        virtual void GetMaxFileSizeConfig(bool& aEnable, uint32& aMaxFileSizeBytes) = 0;

        /**
         * Set maximum recording duration.
         *
         * If maximum recording duration is enabled, the composer will stop the recording session
         * when it detects that writing further media data to the output file will cause the
         * duration of output file to go beyond the maximum duration.  The client will receive
         * an informational event PVMF_COMPOSER_MAXDURATION_REACHED after the maximum recording
         * duration is reached and the recording session has stopped successfully.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Set to true to enable maximum duration, else set to false.
         * @param aMaxDurationMilliseconds If aEnable is set to true, this parameter specifies
         *        the maximum recording duration in milliseconds, else this parameter is ignored.
         * @return Completion status.
         */
        virtual PVMFStatus SetMaxDuration(bool aEnable, uint32 aMaxDurationMilliseconds) = 0;

        /**
         * Query maximum recording duration configuration.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if maximum recording duration
         *                is enabled, else false.
         * @param aMaxFileSizeBytes Output parameter which will be set to the maximum recording duration
         *        in milliseconds if aEnable is set to true, else this parameter is ignored.
         */
        virtual void GetMaxDurationConfig(bool& aEnable, uint32& aMaxDurationMilliseconds) = 0;

        /**
         * Configure recording progress report in terms of file size written.
         *
         * If recording progress report in terms of file size written is enabled, the composer will
         * send informational event PVMF_COMPOSER_FILESIZE_PROGRESS periodically to the client
         * to report progress.  The frequency of progress report is set through the aReportFrequency
         * parameter, which specifies the number of bytes written to output file per progress report.
         * The progress can be retrieved by calling GetEventData method on the PVAsyncInformationalEvent
         * object sent to the client in the PVInformationalEventObserver::HandleInformationalEvent
         * observer function, and then cast the retrieved data to int32 type.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if recording progress report
         *                in terms of file size written is enabled, else false.
         * @param aReportFrequency Output parameter which will be set to the frequency of recording
         *        progress report in terms of bytes written per report.
         */
        virtual PVMFStatus SetFileSizeProgressReport(bool aEnable, uint32 aReportFrequency) = 0;

        /**
         * Query configuration for recording progress report in terms of file size written.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if recording progress report
         *                in terms of file size written is enabled, else false.
         * @param aReportFrequency Output parameter which will be set to the frequency of recording
         *        progress report in terms of bytes written per report.
         */
        virtual void GetFileSizeProgressReportConfig(bool& aEnable, uint32& aReportFrequency) = 0;

        /**
         * Configure recording progress report in terms of duration of output file.
         *
         * If recording progress report in terms of duration of output file is enabled, the composer will
         * send informational event PVMF_COMPOSER_DURATION_PROGRESS periodically to the client
         * to report progress.  The frequency of progress report is set through the aReportFrequency
         * parameter, which specifies the duration output file in milliseconds per progress report.
         * The progress can be retrieved by calling GetEventData method on the PVAsyncInformationalEvent
         * object sent to the client in the PVInformationalEventObserver::HandleInformationalEvent
         * observer function, and then cast the retrieved data to int32 type.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if recording progress report
         *                in terms of duration of output file is enabled, else false.
         * @param aReportFrequency Output parameter which will be set to the frequency of recording
         *        progress report in terms of milliseconds recorded per report.
         */
        virtual PVMFStatus SetDurationProgressReport(bool aEnable, uint32 aReportFrequency) = 0;

        /**
         * Query configuration for recording progress report in terms of duration of output file.
         *
         * This method is valid only after a composer has been selected through
         * PVAuthorEngineInterface::SelectComposer() method.
         *
         * @param aEnable Output parameter which will be set to true if recording progress report
         *                in terms of duration of output file is enabled, else false.
         * @param aReportFrequency Output parameter which will be set to the frequency of recording
         *        progress report in terms of milliseconds recorded per report.
         */
        virtual void GetDurationProgressReportConfig(bool& aEnable, uint32& aReportFrequency) = 0;
};

#endif // PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED




