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
#ifndef PVMF_FORMAT_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#define PVMF_FORMAT_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED

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
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#define PVMF_FF_PROGDOWNLOAD_SUPPORT_INTERFACE_MIMETYPE "x-pvmf/pvmf/ff/progdownload-support"
#define PVMF_FF_PROGDOWNLOAD_SUPPORT_INTERFACE_UUID PVUuid(0x00f80b00, 0x4bd4, 0x4656, 0x8e, 0x0f, 0x63, 0xe0, 0x3d, 0x7a, 0x5f, 0x39)

class PVMFDownloadProgressInterface;

class PVMFFormatProgDownloadSupportInterface:  public PVInterface
{
    public:

        /**
         * This API return the timestamp of a sample that is closest to the given fileSize.
         *
         * @param aFileSize
         *
         * @param uint32& timeStamp - gives the NPT in milliseconds of the closest sample to the fileSize byte offset.
         *
         * @return EVERYTHING_FINE - In case a valid sample and corresponding time stamp was located.
         * INSUFFICIENT_DATA in case the very first sample location is past the fileSize
         * Any other return value indicates ERROR.
         *
         */
        virtual int32 convertSizeToTime(uint32 aFileSize, uint32& aNPTInMS) = 0;

        /**
         * This API sets the file size to parser node. If the file size is not available, then this API shouldn't be called
         * @param aFileSize
        */
        virtual void setFileSize(const uint32 aFileSize) = 0;

        /**
         * Sets the download progress interface which will be used to determine if there is sufficient
         * data downloaded to proceed with playback and request notification play resume if autopause is
         * necessary.
         *
         * @param download_progress - this parameter is an interface pointer to the set of APIs for download progress.
         *
         * @return - none
         *
         */
        virtual void setDownloadProgressInterface(PVMFDownloadProgressInterface* download_progress) = 0;

        /**
         * Callback in response to the requestResumeNotification call in PVMFDownloadProgressInterface when
         * playback can continue
         *
         * @param aDownloadComplete Set to true if download operation has fully completed between requestResumeNotification and this callback
         *
         * @return - none
         */
        virtual void playResumeNotification(bool aDownloadComplete) = 0;

        /**
         * Notifies that the download is complete.
         *
         * @param - none
         *
         * @return - none
         */
        virtual void notifyDownloadComplete() = 0;

        /**
         * Set the protocol information that is defined as kvp
         *
         * @param aInfoKvpVec, a kvp vector of all protocol information
         *
         * @return true means success, and false means any exceptional case, such as aInfoKvpVec has nothing
         */
        virtual bool setProtocolInfo(Oscl_Vector<PvmiKvp*, OsclMemAllocator>& aInfoKvpVec)
        {
            OSCL_UNUSED_ARG(aInfoKvpVec);
            return true;
        }

};

#endif //PVMF_FF_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED


