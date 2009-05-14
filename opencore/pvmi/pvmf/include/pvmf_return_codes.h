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

/**
 *  @file pv_return_codes.h
 *  @brief This file defines the general return and event codes to be used by PVMF elements.
 *         Theses base-level codes are unique. Error codes are negative values and informational
 *         codes are positive values.
 *
 */

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#define PVMF_RETURN_CODES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif


typedef int32 PVMFStatus;

// Return codes
/*
 Return code for general success
 */
const PVMFStatus PVMFSuccess = 1;
/*
 Return code for pending completion
 */
const PVMFStatus PVMFPending = 0;
/*
 Return code for never set
 */
const PVMFStatus PVMFNotSet = 2;


// Error codes (negative values)

/*
 Definition of first error event in range (not an actual error code).
 */
const PVMFStatus PVMFErrFirst = (-1);
/*
 Return code for general failure
 */
const PVMFStatus PVMFFailure = (-1);
/*
 Error due to cancellation
 */
const PVMFStatus PVMFErrCancelled = (-2);
/*
 Error due to no memory being available
 */
const PVMFStatus PVMFErrNoMemory = (-3);
/*
 Error due to request not being supported
 */
const PVMFStatus PVMFErrNotSupported = (-4);
/*
 Error due to invalid argument
 */
const PVMFStatus PVMFErrArgument = (-5);
/*
 Error due to invalid resource handle being specified
 */
const PVMFStatus PVMFErrBadHandle = (-6);
/*
 Error due to resource already exists and another one cannot be created
 */
const PVMFStatus PVMFErrAlreadyExists = (-7);
/*
 Error due to resource being busy and request cannot be handled
 */
const PVMFStatus PVMFErrBusy = (-8);
/*
 Error due to resource not ready to accept request
 */
const PVMFStatus PVMFErrNotReady = (-9);
/*
 Error due to data corruption being detected
 */
const PVMFStatus PVMFErrCorrupt = (-10);
/*
 Error due to request timing out
 */
const PVMFStatus PVMFErrTimeout = (-11);
/*
 Error due to general overflow
 */
const PVMFStatus PVMFErrOverflow = (-12);
/*
 Error due to general underflow
 */
const PVMFStatus PVMFErrUnderflow = (-13);
/*
 Error due to resource being in wrong state to handle request
 */
const PVMFStatus PVMFErrInvalidState = (-14);
/*
 Error due to resource not being available
 */
const PVMFStatus PVMFErrNoResources = (-15);
/*
 Error due to invalid configuration of resource
 */
const PVMFStatus PVMFErrResourceConfiguration = (-16);
/*
 Error due to general error in underlying resource
 */
const PVMFStatus PVMFErrResource = (-17);
/*
 Error due to general data processing
 */
const PVMFStatus PVMFErrProcessing = (-18);
/*
 Error due to general port processing
 */
const PVMFStatus PVMFErrPortProcessing = (-19);
/*
 Error due to lack of authorization to access a resource.
 */
const PVMFStatus PVMFErrAccessDenied = (-20);
/*
 Error due to the lack of a valid license for the content
 */
const PVMFStatus PVMFErrLicenseRequired = (-21);
/*
 Error due to the lack of a valid license for the content.  However
 a preview is available.
 */
const PVMFStatus PVMFErrLicenseRequiredPreviewAvailable = (-22);
/*
 Error due to the download content length larger than the maximum request size
 */
const PVMFStatus PVMFErrContentTooLarge = (-23);
/*
 Error due to a maximum number of objects in use
 */
const PVMFStatus PVMFErrMaxReached = (-24);
/*
 Return code for low disk space
 */
const PVMFStatus PVMFLowDiskSpace = (-25);
/*
 Error due to the requirement of user-id and password input from app for HTTP basic/digest authentication
 */
const PVMFStatus PVMFErrHTTPAuthenticationRequired = (-26);
/*
 PVMFMediaClock specific error. Callback has become invalid due to change in direction of NPT clock.
*/
const PVMFStatus PVMFErrCallbackHasBecomeInvalid = (-27);
/*
 PVMFMediaClock specific error. Callback is called as clock has stopped.
*/
const PVMFStatus PVMFErrCallbackClockStopped = (-28);
/*
 Error due to missing call for ReleaseMatadataValue() API
 */
const PVMFStatus PVMFErrReleaseMetadataValueNotDone = (-29);
/*
 Error due to the redirect error
*/
const PVMFStatus PVMFErrRedirect = (-30);
/*
 Error if a given method or API is not implemented. This is NOT the same as PVMFErrNotSupported.
*/
const PVMFStatus PVMFErrNotImplemented = (-31);
/*
 Placeholder for last event in range.
 */
const PVMFStatus PVMFErrLast = (-100);
/*
 Macro to tell if a value is in PVMFErr range
 */
#define IsPVMFErrCode(s) ((PVMFErrLast<=s)&&(s<=PVMFErrFirst))

// Informational codes (positive values)

const PVMFStatus PVMFInfoFirst = 10;

/*
 Notification that a port was created
 */
const PVMFStatus PVMFInfoPortCreated = 10;
/*
 Notification that a port was deleted
 */
const PVMFStatus PVMFInfoPortDeleted = 11;
/*
 Notification that a port was connected
 */
const PVMFStatus PVMFInfoPortConnected = 12;
/*
 Notification that a port was disconnected
 */
const PVMFStatus PVMFInfoPortDisconnected = 13;
/*
 Notification that an overflow occurred (not fatal error)
 */
const PVMFStatus PVMFInfoOverflow = 14;
/*
 Notification that an underflow occurred (not fatal error)
 */
const PVMFStatus PVMFInfoUnderflow = 15;
/*
 Notification that a processing failure occurred (not fatal error)
 */
const PVMFStatus PVMFInfoProcessingFailure = 16;
/*
 Notification that end of data stream has been reached
 */
const PVMFStatus PVMFInfoEndOfData = 17;
/*
 Notification that a data buffer has been created
 */
const PVMFStatus PVMFInfoBufferCreated = 18;
/*
 Notification that buffering of data has started
 */
const PVMFStatus PVMFInfoBufferingStart = 19;
/*
 Notification for data buffering level status
 */
const PVMFStatus PVMFInfoBufferingStatus = 20;
/*
 Notification that data buffering has completed
 */
const PVMFStatus PVMFInfoBufferingComplete = 21;
/*
 Notification that data is ready for use
 */
const PVMFStatus PVMFInfoDataReady = 22;
/*
 Notification for position status
 */
const PVMFStatus PVMFInfoPositionStatus = 23;
/*
 Notification for node state change
 */
const PVMFStatus PVMFInfoStateChanged = 24;
/*
 Notification that data was discarded during synchronization.
 */
const PVMFStatus PVMFInfoDataDiscarded = 25;
/*
 Notification that error handling has started
 */
const PVMFStatus PVMFInfoErrorHandlingStart = 26;
/*
 Notification that error handling has completed
 */
const PVMFStatus PVMFInfoErrorHandlingComplete = 27;
/*
 Notification from a remote source
 */
const PVMFStatus PVMFInfoRemoteSourceNotification = 28;
/*
 Notification that license acquisition has started.
 */
const PVMFStatus PVMFInfoLicenseAcquisitionStarted = 29;
/*
 Notification that download content length is available
 */
const PVMFStatus PVMFInfoContentLength = 30;
/*
 Notification that downloaded content reaches the maximum request size, and will
 be truncated, especially for the case of unavailable content length
 */
const PVMFStatus PVMFInfoContentTruncated = 31;
/*
 Notification that source format is not supported, typically sent
 during protocol rollover
 */
const PVMFStatus PVMFInfoSourceFormatNotSupported = 32;
/*
 Notification that a clip transition has occurred while playing a playlist
 */
const PVMFStatus PVMFInfoPlayListClipTransition = 33;
/*
 Notification that content type for download or HTTP streaming is available
 */
const PVMFStatus PVMFInfoContentType = 34;
/*
 Notification that paticular track is disable. This one is on a per track basis.
 */
const PVMFStatus PVMFInfoTrackDisable = 35;
/*
 Notification that unexpected data has been obtained, especially for download,
 when client receives from server more data than requested in content-length header
 */
const PVMFStatus PVMFInfoUnexpectedData = 36;
/*
 Notification that server discnnect happens after download is complete
 */
const PVMFStatus PVMFInfoSessionDisconnect = 37;
/*
 Notification that new meadi stream has been started
 */
const PVMFStatus PVMFInfoStartOfData = 38;
/*
 Notification that node has processed a command with ReportObserver marker info
 */
const PVMFStatus PVMFInfoReportObserverRecieved = 39;
/*
 Notification that meta data is available with source node
 */
const PVMFStatus PVMFInfoMetadataAvailable = 40;
/*
 Notification that duration is available with source node
 */
const PVMFStatus PVMFInfoDurationAvailable = 41;
/*
 Notification that Change Position request not supported
 */
const PVMFStatus PVMFInfoChangePlaybackPositionNotSupported = 42;
/*
 Notification that the content is poorly inter-leaved
 */
const PVMFStatus PVMFInfoPoorlyInterleavedContent = 43;
/*
 Notification for actual playback position after repositioning
 */
const PVMFStatus PVMFInfoActualPlaybackPosition = 44;
/*
 Notification that the live buffer is empty
 */
const PVMFStatus PVMFInfoLiveBufferEmpty = 45;
/*
 Notification that a server has responded with 200 OK to a Playlist play request
 */
const PVMFStatus PVMFInfoPlayListSwitch = 46;
/*
 Notification of configuration complete
 */
const PVMFStatus PVMFMIOConfigurationComplete = 47;
/*
 Notification that the video track is falling behind
 */
const PVMFStatus PVMFInfoVideoTrackFallingBehind = 48;
/*
 Notification that memory is not available for new RTP packets
 */
const PVMFStatus PVMFInfoSourceOverflow = 49;
/*
 Notification for Media data length in shoutcast session
 */
const PVMFStatus PVMFInfoShoutcastMediaDataLength = 50;
/*
 Notification for clip bitrate in shoutcast session
 */
const PVMFStatus PVMFInfoShoutcastClipBitrate = 51;
/*
 Notification for shoutcast session
 */
const PVMFStatus PVMFInfoIsShoutcastSesssion = 52;
/*
 Placeholder for end of range
 */
const PVMFStatus PVMFInfoLast = 100;
/*
 Macro to tell if a code is in PVMFInfo range
 */
#define IsPVMFInfoCode(s) ((PVMFInfoFirst<=s)&&(s<=PVMFInfoLast))

#endif
