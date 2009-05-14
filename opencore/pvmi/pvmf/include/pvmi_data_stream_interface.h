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
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#define PVMI_DATA_STREAM_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif


#define PVMI_DATA_STREAM_INTERFACE_MIMETYPE "pvxxx/pvmf/stream"
#define PVMIDataStreamSyncInterfaceUuid PVUuid(0x6d32c608,0x6307,0x4538,0x83,0xe7,0x34,0x0e,0x7a,0xba,0xb9,0x8a)

typedef int32 PvmiDataStreamSession;
typedef int32 PvmiDataStreamCommandId;

class PVMFCmdResp;
class PVMFAsyncEvent;

class PvmiDataStreamObserver
{
    public:
        virtual void DataStreamCommandCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual void DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent) = 0;
        virtual void DataStreamErrorEvent(const PVMFAsyncEvent& aEvent) = 0;
        virtual ~PvmiDataStreamObserver() {}
};


typedef enum
{
    PVDS_SUCCESS,
    PVDS_FAILURE,
    PVDS_NOT_SUPPORTED,
    PVDS_PERMISSION_DENIED,
    PVDS_UNSUPPORTED_MODE,
    PVDS_INVALID_SESSION,
    PVDS_INVALID_REQUEST,
    PVDS_UNSUPPORTED_RANDOM_ACCESS,
    PVDS_END_OF_STREAM,
    PVDS_PENDING
} PvmiDataStreamStatus;

typedef enum
{
    PVDS_SEQUENTIAL_ACCESS_ONLY,
    PVDS_REWIND_TO_START_ONLY,
    PVDS_LIMITED_SEEKING, /* only allows seeking to certain points in stream */
    PVDS_FULL_RANDOM_ACCESS
} PvmiDataStreamRandomAccessType;

typedef enum
{
    PVDS_SEEK_SET,
    PVDS_SEEK_CUR,
    PVDS_SEEK_END,
    PVDS_SKIP_SET, /* MBDS only, to trigger http get request, without changing read pointer position */
    PVDS_SKIP_CUR, /* MBDS only, to trigger http get request, without changing read pointer position */
    PVDS_SKIP_END  /* MBDS only, to trigger http get request, without changing read pointer position */
} PvmiDataStreamSeekType;

typedef enum
{
    PVDS_READ_ONLY,
    PVDS_WRITE_ONLY,
    PVDS_READ_WRITE,
    PVDS_APPEND,
    PVDS_READ_PLUS,
    PVDS_REWRITE
} PvmiDataStreamMode;


// requests from the data stream to the writer of the stream
typedef enum
{
    PVDS_REQUEST_MEM_FRAG_RELEASED,
    PVDS_REQUEST_REPOSITION
} PvmiDataStreamRequest;

class PVMFDataStreamReadCapacityObserver
{
    public:
        virtual PvmiDataStreamStatus GetStreamReadCapacity(uint32& aCapacity) = 0;
};


class PvmiDataStreamRequestObserver
{
    public:
        virtual PvmiDataStreamCommandId DataStreamRequest(PvmiDataStreamSession aSessionID,
                PvmiDataStreamRequest aRequestID,
                OsclAny* aRequestData,
                OsclAny* aContext) = 0;

        virtual PvmiDataStreamStatus DataStreamRequestSync(PvmiDataStreamSession aSessionID,
                PvmiDataStreamRequest aRequestID,
                OsclAny* aRequestData) = 0;

        virtual ~PvmiDataStreamRequestObserver() {}
};


class PVMIDataStreamSyncInterface : public PVInterface
{
    public:
        /**
        * Opens a session with the data stream.  Returns a session ID in
        * the output parameter.
        *
        * @param aSessionID - [out] this is an output parameter.  The session ID
        *                    will be written to this parameter if the session can
        *                    be established successfully.
        *
        * @param aMode - [in] The mode indicates the way the stream will be used
        *                     (e.g., read, write, append, etc)
        *
        * @param aNonBlocking - [in] The boolean indicates if the access session
        *                      being requested ought to be blocking or non-blocking.
        *                      Default is blocking. If non-blocking is not supported
        *                      this API should return PVDS_UNSUPPORTED_MODE.
        *
        * @return returns 0 if successful and a non-zero value otherwise
        */
        virtual PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                PvmiDataStreamMode aMode,
                bool aNonBlocking = false) = 0;

        /**
        * Closes a previously established session.  Buffers will be flushed
        * if needed before closing the session.
        *
        * @param aSessionID - [in] A valid session ID of a previously opened session.
        *
        * @return returns 0 if successful, and a non-zero value otherwise
        */
        virtual PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID) = 0;

        /**
        * Query the random access capabilities. This does not require a session
        * to be established.
        *
        * @return returns an enumeration value that characterizes the random
        * access capability.
        *
        */
        virtual PvmiDataStreamRandomAccessType QueryRandomAccessCapability() = 0;


        /**
        * Query the current read capacity.  The value returned indicates
        * the amount of data immediately available for reading beyond the
        * position of the current read pointer.  If the data is unavailable
        * then it will be indicated in the return value.
        *
        * @param aSessionID - [in] A valid session ID of a previously opened session.
        *
        * @param aCapacity - [out] Amount of data available for immediate reading.
        *
        * @return returns 0 if successful, and a non-zero value otherwise
        *
        */
        virtual PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                uint32& aCapacity) = 0;

        /**
        * Request notification when the read capacity reaches a certain level.
        * The observer registered with this request will be used for the callback
        * when the requested capacity is available.  Only one pending notification
        * is allowed, so if the API is called before a pending notification has
        * been sent then the new value will replace the previous one.
        *
        * @param aSessionID the session identifier of the stream
        * @param observer  the observer that will receive the callback
        * @param capacity  the requested capacity (bytes available past
        *                                          the read pointer)
        * @param aContextData An opaque data pointer that would be returned in the callback.
        *
        * @return This will return a non-zero command id on success.
        *         if the asynchronous notification is not supported this method
        *         leaves with OsclErrNotSupported
        */
        virtual PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& aObserver,
                uint32 aCapacity,
                OsclAny* aContextData = NULL) = 0;


        /**
        * Query the current write capacity.  The value returned indicates
        * the amount of data immediately available for writing beyond the
        * position of the current write pointer.  If the data is unavailable
        * then it will be indicated in the return value.
        *
        * @param aSessionID - [in] A valid session ID of a previously opened session.
        *
        * @param aCapacity - [out] Amount of data available for immediate writing.
        *
        * @return returns 0 if successful, and a non-zero value otherwise
        */
        virtual PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                uint32& aCapacity) = 0;

        /**
        * Request notification when the write capacity reaches a certain level.
        * The observer registered with this request will be used for the callback
        * when the requested capacity is available.  Only one pending notification
        * is allowed, so if the API is called before a pending notification has
        * been sent then the new value will replace the previous one.
        *
        * @param aSessionID the session identifier of the stream
        * @param observer  the observer that will receive the callback
        * @param capacity  the requested capacity (bytes available past
        *                                          the write pointer)
        * @param aContextData An opaque data pointer that would be returned in the callback.
        *
        * @return This will return a non-zero command id on success.
        *         if the asynchronous notification is not supported this method
        *         leaves with OsclErrNotSupported
        *
        */
        virtual PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& aObserver,
                uint32 aCapacity,
                OsclAny* aContextData = NULL) = 0;

        /**
        * Cancel notification for either write or read capacity that was issued earlier.
        * The observer registered with this request will be used for the callback
        * when the notification is cancelled.
        *
        * @param aSessionID the session identifier of the stream
        * @param observer  the observer that will receive the callback
        * @param capacity  the command id of the command that needs to be cancelled
        * @param aContextData An opaque data pointer that would be returned in the callback.
        *
        * @return This will return a non-zero command id on success.
        *         if the asynchronous notification is not supported this method
        *         leaves with OsclErrNotSupported
        *
        */
        virtual PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL) = 0;
        /**
        * Cancel notification for either write or read capacity that was issued earlier.
        * Cancellation is done synchronously.
        *
        * @param aSessionID the session identifier of the stream
        *
        * @return status of the cancel operation
        *
        */
        virtual PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID)
        {
            OSCL_UNUSED_ARG(aSessionID);
            return PVDS_NOT_SUPPORTED;
        }


        /**
        * Reads from the data stream into the buffer a maximum of 'numelements'
        * of size 'size'.
        *
        * @param aSessionID the session identifier of the stream
        * @param buffer pointer to buffer of type uint8*
        * @param size   element size in bytes
        * @param numelements
        *        max number of elements to read.  The value will be updated
        *        with the actual number of elements read.
        *
        * The number of full elements actually read, which
        * may be less than count if an error occurs or if the end
        * of the file is encountered before reaching count. Also if
        * non-blocking mode is used it may return a smaller count.
        * The return status will indicate why it returned a smaller value.
        */
        virtual PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID,
                                          uint8* aBuffer,
                                          uint32 aSize,
                                          uint32& aNumElements) = 0;

        /**
        * Writes from the buffer to the data stream a maximum of 'numelements'
        * of size 'size'.
        *
        * @param aSessionID the session identifier of the stream
        * @param frag pointer to OsclRefCounterMemFrag containing the mem frag info
        * @param numelements
        *        returns the number of elements actually written
        *
        * @return status
        */
        virtual PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                           OsclRefCounterMemFrag* frag,
                                           uint32& aNumElements)
        {
            OSCL_UNUSED_ARG(aSessionID);
            OSCL_UNUSED_ARG(frag);
            OSCL_UNUSED_ARG(aNumElements);
            return PVDS_NOT_SUPPORTED;
        }

        /**
        * Writes from the buffer to the data stream a maximum of 'numelements'
        * of size 'size'.
        *
        * @param aSessionID the session identifier of the stream
        * @param buffer pointer to buffer of type uint8*
        * @param size   element size in bytes
        * @param numelements
        *        max number of elements to read
        *
        * The number of full elements actually read, which
        * may be less than count if an error occurs or if the end
        * of the file is encountered before reaching count. Also
        * if non-blocking mode is used it may return a smaller count.
        * The return status will indicate the cause of the error.
        */
        virtual PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                           uint8* aBuffer,
                                           uint32 aSize,
                                           uint32& aNumElements) = 0;

        /**
        * Seek operation
        * Sets the position for the read/write pointer.
        *
        * @param aSessionID the session identifier of the stream
        * @param offset offset from the specified origin.
        * @param origin starting point
        *
        * @return returns the status of the operation.
        */
        virtual PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                                          int32 aOffset,
                                          PvmiDataStreamSeekType aOrigin) = 0;

        /**
        * Returns the current position (i.e., byte offset from the beginning
        * of the data stream for the read/write pointer.
        */
        virtual uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) = 0;


        /**
        * On a stream with write capability causes any buffered
        * but unwritten data to be flushed.
        *
        * @return returns the status of the operation.
        */
        virtual PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID) = 0;

        /**
        * Sometimes datastream implementations need to rely on an external source
        * to provide them with updated stream size (take a case of progressive dowmnload
        * where in module A is doing the download and module B is doing the reads). This
        * API is one way to make these datastreams connect with each other.
        *
        * @leaves by default, derived implementations can choose to override if needed
        */
        virtual void SetStreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObs)
        {
            OSCL_UNUSED_ARG(aObs);
            OSCL_LEAVE(OsclErrNotSupported);
        }

        /**
        * Sets the content length, if known, of the media clip
        * Used in progressive playback where the HTTP server sends the content length
        * in the HEAD response
        *
        * @param aLength length of clip in bytes
        */
        virtual void SetContentLength(uint32 aLength)
        {
            OSCL_UNUSED_ARG(aLength);
        }


        /**
        * Returns the content length, if known, of the media clip
        * Used in progressive playback where the HTTP server sends the content length
        * in the HEAD response
        *
        * @return returns the content lenght (0 = unknown)
        */
        virtual uint32 GetContentLength()
        {
            return 0;
        }

        /**
        * Returns the data stream buffering capacity, if it is a memory buffer data stream (MBDS)
        * Used in progressive playback where MBDS has a finite cache size
        *
        * @return returns buffering capacity (0 = not MBDS)
        *
        */
        virtual uint32 QueryBufferingCapacity()
        {
            return 0;
        }


        /**
        * Sets the request observer usually in the stream writer
        * Used in progressive playback for repositioning requests, etc
        *
        * @param aObserver the observer
        *
            * @return returns the status of the operation.
            */
        virtual PvmiDataStreamStatus SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
            return PVDS_NOT_SUPPORTED;
        }

        /**
        * Callback from stream writer when request has completed
        * Used in progressive playback when reposition requests are sent to the writer (protocol engine)
        * which in turn issues HTTP GET requests with Range header to the server
        *
        * @param aResponse command response
        */
        virtual void SourceRequestCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        }

        /**
        * Keep the data in a persistent storage during playback
        * Used in progressive playback, mainly for moov atom storage
        *
        * @param aOffset file offset of data
        * @param aSize bytes from file offset to be made persistent
        *
        * @return returns the status of the operation.
        */
        virtual PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize)
        {
            OSCL_UNUSED_ARG(aOffset);
            OSCL_UNUSED_ARG(aSize);
            return PVDS_NOT_SUPPORTED;
        }

        /**
            * Informs the MBDS that the source has been disconnected
            * Used in progressive playback when the network connection times out
            *
            */
        virtual void SourceDisconnected()
        {
            ;
        }

        /**
        * For reader to find out the byte range in the memory buffer data stream
        * Used in progressive playback
        *
        * @return aCurrentFirstByteOffset first byte offset inclusive
        * @return aCurrentLastByteOffset last byte offset inclusive
        */
        virtual void GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset)
        {
            aCurrentFirstByteOffset = 0;
            aCurrentLastByteOffset = 0;
        }
};



#endif //PVMI_DATA_STREAM_INTERFACE_H_INCLUDED

