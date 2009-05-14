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
#ifndef PV_PLAYER_EVENTS_H_INCLUDED
#define PV_PLAYER_EVENTS_H_INCLUDED

/** C++ defintion of UUID for pvPlayer error and information event type codes */
#define PVPlayerErrorInfoEventTypesUUID PVUuid(0x46fca5ac,0x5b57,0x4cc2,0x82,0xc3,0x03,0x10,0x60,0xb7,0xb5,0x98)
/** C defintion of UUID for pvPlayer error and information event type coddes */
#define PVPlayerErrorInfoEventTypesUUID_C PVUuid_Construct(0x46fca5ac,0x5b57,0x4cc2,0x82,0xc3,0x03,0x10,0x60,0xb7,0xb5,0x98)

/**
 * An enumeration of error types from pvPlayer
 **/
typedef enum
{
    /**
     pvPlayer sends this error event when the user
     provides an invalid data source for multimedia playback
    **/
    PVPlayerErrSourceInvalid = 1024,

    /**
     pvPlayer sends this error event when an error occurs
     during initialization of data source
    **/
    PVPlayerErrSourceInit,

    /**
     pvPlayer sends this error event for general non-fatal error
     from the data source
    **/
    PVPlayerErrSource,

    /**
     pvPlayer sends this error event for general fatal error from
     the data source
    **/
    PVPlayerErrSourceFatal,

    /**
     pvPlayer sends this error event when the data source contains
     no media track for playback
    **/
    PVPlayerErrSourceNoMediaTrack,

    /**
     pvPlayer sends this error event when the user provides an invalid
     data sink for multimedia playback
    **/
    PVPlayerErrSinkInvalid,

    /**
     pvPlayer sends this error event when an error occurs
     during initialization of data sink
    **/
    PVPlayerErrSinkInit,

    /**
     pvPlayer sends this error event for general non-fatal error
     from the data sink
    **/
    PVPlayerErrSink,

    /**
     pvPlayer sends this error event for general fatal error from
     the data sink
    **/
    PVPlayerErrSinkFatal,

    /**
     pvPlayer sends this error event when no supported media track
     for playback was found
    **/
    PVPlayerErrNoSupportedTrack,

    /**
     pvPlayer sends this error event when an error occurs while
     initializing the datapath or its nodes
    **/
    PVPlayerErrDatapathInit,

    /**
     pvPlayer sends this error event for general non-fatal error from
     the datapath or its nodes
    **/
    PVPlayerErrDatapath,

    /**
     pvPlayer sends this error event for general fatal error from
     the datapath or its nodes
    **/
    PVPlayerErrDatapathFatal,

    /**
     pvPlayer sends this error event when the data source runs out of
     media data
    **/
    PVPlayerErrSourceMediaDataUnavailable,

    /**
     pvPlayer sends this error event when the data source encounters a
     general error in the media data
    **/
    PVPlayerErrSourceMediaData,

    /**
     pvPlayer sends this error event when the data sink encounters a
     general error in the media data
    **/
    PVPlayerErrSinkMediaData,

    /**
     pvPlayer sends this error event when the datapath or its node encounters a
     general error in the media data
    **/
    PVPlayerErrDatapathMediaData,

    /**
     pvPlayer sends this error event when an error occurs during shutdown
     of data source
    **/
    PVPlayerErrSourceShutdown,

    /**
     pvPlayer sends this error event when an error occurs during shutdown
     of data sink
    **/
    PVPlayerErrSinkShutdown,

    /**
     pvPlayer sends this error event when an error occurs during shutdown
     of datapath or its nodes
    **/
    PVPlayerErrDatapathShutdown,

    /**
     pvPlayer sends this error event when an error occurs during interaction
     with CPM plug-in
    **/
    PVPlayerErrCPMPlugin,

    /**
     Placeholder for the last pvPlayer error event
     **/
    PVPlayerErrLast = 8191
} PVPlayerErrorEventType;

/**
 * An enumeration of informational event types from pvPlayer
 **/
typedef enum
{
    /**
     pvPlayer sends this event when the playback has reached
     the specified end time and playback has been paused.
     **/
    PVPlayerInfoEndTimeReached	= 8192,

    /**
     pvPlayer sends this event when reporting periodic playback
     position status
     This event will be sent with general event code PVMFInfoPositionStatus and the event's local
     buffer contains the playback position in milliseconds, seconds, or minutes. The format of local buffer is as follows:
     Byte 1: 0x01
     Byte 2: 0x00
     Byte 3: 0x00
     Byte 4: 0x00
     Byte 5-8: uint32 (playback position value in milliseconds, seconds, or minutes depending on the
     current settings for playback position status units)
     **/
    PVPlayerInfoPlaybackPositionStatus,

    /**
     pvPlayer sends this event when the playback has reached
     the end of clip (all sink nodes reported PVMFInfoEndOfData)
     and playback has been paused.
     **/
    PVPlayerInfoEndOfClipReached,

    /**
     pvPlayer sends this event when playback has started
     from the new specified begin time after SetPlaybackRange() call
     **/
    PVPlayerInfoPlaybackFromBeginTime,

    /**
     pvPlayer sends this event when it recvs any info event from
     the source
     **/
    PVPlayerInfoSourceMediaData,

    /**
     pvPlayer sends this event when it is doing a source rollover,
     or in other words attempting alternate sources.
     This event's local buffer contains the format type for the new source.
     The format of local buffer is as follows:
     Byte 1: 0x01
     Byte 2: 0x00
     Byte 3: 0x00
     Byte 4: 0x00
     Byte 5-8: uint32 (format type for the new source)
     **/
    PVPlayerInfoAttemptingSourceRollOver,

    /**
     pvPlayer sends this event when it is doing SFR,
     or in other words a repos requested is satisfied by skipping to the requested point
     from the current playback position ASAP.
     **/
    PVPlayerInfoAttemptingSFRAsPartOfSetPlayBackRange,

    /**
     pvPlayer sends this event when engine is changing position and
     Source node sends NotSupported to Change Position request.
     **/
    PVPlayerInfoChangePlaybackPositionNotSupported,

    /**
     Placeholder for the last pvPlayer informational event
     **/
    PVPlayerInfoLast = 10000

} PVPlayerInformationalEventType;


#endif // PV_PLAYER_EVENTS_H_INCLUDED


