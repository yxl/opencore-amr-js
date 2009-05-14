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
#ifndef PVMF_MP4_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#define PVMF_MP4_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED

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

#define PVMF_MP4_PROGDOWNLOAD_SUPPORT_INTERFACE_MIMETYPE "x-pvmf/pvmf/mp4ff/progdownload-support"
#define PVMF_MP4_PROGDOWNLOAD_SUPPORT_INTERFACE_UUID PVUuid(0x00f80b00, 0x4bd4, 0x4656, 0x8e, 0x0f, 0x63, 0xe0, 0x3d, 0x7a, 0x5f, 0x39)

class Oscl_File;
class PVMFDownloadProgressInterface;

class PVMFMP4ProgDownloadSupportInterface:  public PVInterface
{
    public:
        /**
         * Determines if the file is a progressively downloadable MP4 file.
         *
         * @param  FilePointer - File pointer to the MP4/3GP
         * file. Please note that the file open and close are handled by the
         * caller, so the should already be opened and it will not be closed by
         * this API.
         *
         * @param fileSize - Size of the downloaded file, thus far.
         *
         * @param uint32& currFilePosition - This API parses the top level
         * atoms of the file to determine the relative positions of the media
         * and meta data. This might take a few passes, depending on the how
         * large the file is, and how the top-level atom layout. This value is
         * used by the API to avoid parsing the same data all over again. In
         * case the return value is INSUFFICIENT_DATA, then the caller needs to
         * save the "currFilePosition" from the current call, in the next
         * call.  It should be initialized to zero.
         *
         * @param bool& oIsProgressiveDownloadable - Set to true if the clip is
         * is progressive dowmloadable.
         *
         * @param uint32& metaDataSize - If the clip is progressive
         * downloadable then this API also returns the meta data size. Player
         * needs to wait for the file to grow past the metaDataSize before
         * starting playback.This param is valid only if oIsProgressiveDownloadable
         * is set to TRUE.
         *
         * @return MP4_ERROR_CODE - EVERYTHING_FINE, if a conclusion is reached
         * either way on whether a clip is progressive downloadable or not.
         * INSUFFICIENT_DATA, if more calls to this API are needed to reach a
         * decision
         * Any other return value indicates error. See isuceedfail.h in the mp4 fileformat
         * parser for more information on the error codes.
         */
        virtual int32 isProgressiveDownloadable(Oscl_File* FilePointer,
                                                uint32 fileSize,
                                                bool& oIsProgressiveDownloadable,
                                                uint32& metaDataSize) = 0;

        /**
        * This API return the timestamp of a sample that is closest to the given
        * fileSize.
        *
        * @param trackID
        *
        * @param fileSize
        *
        * @param uint32& timeStamp - gives the NPT in milliseconds of the closest sample to the fileSize byte offset.
        *
        * @return EVERYTHING_FINE - In case a valid sample and corresponding time
        * stamp was located.
        * INSUFFICIENT_DATA in case the very first sample location is past the fileSize
        * NOT_SUPPORTED - in case "parsingMode" is set to 1, in "readMP4File"
        * call
        * Any other return value indicates ERROR.
        *
        */
        virtual int32 getMaxTrackTimeStamp(uint32 fileSize, uint32& timeStamp) = 0;

        /**
         * Returns the length of the entire presentation in the units of the movie timescale.
         *
         * @return - movie duration in units of the movie timescale.
         *
         */
        virtual uint32 getMovieDuration() = 0;


        /**
         * Returns the movie timescale.
         *
         * @return - movie timescale (i.e., ticks per second).
         *
         */
        virtual uint32 getMovieTimescale() = 0;

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

};

#endif //PVMF_MP4_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED

