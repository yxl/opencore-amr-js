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
#ifndef PVMF_DOWNLOAD_PROGRESS_EXTENSION_H
#define PVMF_DOWNLOAD_PROGRESS_EXTENSION_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

#define PVMF_DOWNLOAD_PROGRESS_INTERFACE_MIMETYPE "x-pvmf/pvmf/download/progress"
#define PVMF_DOWNLOAD_PROGRESS_INTERFACE_UUID PVUuid(0x01da7746, 0x7d6b, 0x4673, 0xb9, 0x0c, 0x63, 0x2c, 0x82, 0xdd, 0x33, 0xad)

class PVMFMP4ProgDownloadSupportInterface;
class PVMFFormatProgDownloadSupportInterface;

class PVMFDownloadProgressInterface : public PVInterface
{
    public:

        /**
         * Sets the download support interface which provides access to some MP4 fileformat
         * utilities that are used to determine download progress.
         *
         * @param  download_support_interface - the input parameter that is a pointer to the
         * mp4 download support interface.
         *
         * @return - none
         */
        virtual void setMP4DownloadSupportInterface(PVMFMP4ProgDownloadSupportInterface* download_support_interface) = 0;

        /**
         * Sets the download support interface which provides access to some fileformat (e.g. MP4, ASF or Real)
         * utilities that are used to determine download progress.
         *
         * @param  download_support_interface - the input parameter that is a pointer to the
         * mp4 download support interface.
         *
         * @return - none
         */
        virtual void setFormatDownloadSupportInterface(PVMFFormatProgDownloadSupportInterface* download_support_interface) = 0;

        /**
         * Sets the clip duration for the objects with download support interface
         *
         * @param  aClipDurationMsec - the unit of the duration is milli-seconds
         *
         * @return - none
         */
        virtual void setClipDuration(const uint32 aClipDurationMsec) = 0;


        /**
         * Returns the download progress clock
         *
         * @return OsclSharedPtr<PVMFMediaClock> - the shared pointer to the download progress clock.
         */
        virtual OsclSharedPtr<PVMFMediaClock> getDownloadProgressClock() = 0;

        /**
         * Requests notification when playback can continue uninterrupted based on the
         * play position and the current download rate.
         *
         * @param currentNPTReadPosition The current playback position of the data source as millisecond NPT time
         * @param aDownloadComplete Output parameter which will be set to true if the download has completed
         *
         * @return none
         */
        virtual void requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete) = 0;

        /**
        * Cancels any pending resume notification.
        *
        * @param none
        *
        * @return none
        */
        virtual void cancelResumeNotification() = 0;

};

#endif //PVMF_MP4_PROGDOWNLOAD_SUPPORT_EXTENSION_H

