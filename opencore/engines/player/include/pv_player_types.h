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
#ifndef PV_PLAYER_TYPES_H_INCLUDED
#define PV_PLAYER_TYPES_H_INCLUDED

/**
 * An enumeration of the major states of the pvPlayer engine.
 **/
typedef enum
{
    /**
    	The state immediately after the pvPlayer instance has been successfully created or instantiated.
    	pvPlayer also returns to this state after successful completion of reset command.
    	The player data source can be added during this state.
    	No resources have been allocated yet.
    **/
    PVP_STATE_IDLE			= 1,

    /**
    	pvPlayer is in this state after successfully completing initialization of data source or after stopping playback.
    	Player data sinks and user defined track selector can be added to the player in this state. Also metadata from the
    	data source can be queried and other playback parameters can be set at this state.
    **/
    PVP_STATE_INITIALIZED	= 2,

    /**
    	pvPlayer is in this state when it has successfully completed preparations and ready to start playback.
    **/
    PVP_STATE_PREPARED		= 3,

    /**
     *  pvPlayer is in this state when it is playing back the specified data source. Even if playback is auto-paused, pvPlayer
     *  will be in this state.
    **/
    PVP_STATE_STARTED		= 4,

    /**
    	pvPlayer is in this state after successfully completiong pausing of playback. All media datapaths have been stopped,
    	but the buffered data has not been flushed. The playback can be resumed or stopped from this state.
    **/
    PVP_STATE_PAUSED		= 5,

    /**
    	pvPlayer enters this state when it encounters an error. This is a transitional state and after pvPlayer performs
    	error recovery, it will end up in PVP_STATE_IDLE state.
    **/
    PVP_STATE_ERROR			= 6
} PVPlayerState;

typedef union PVPPlaybackPositionValueUnion
{
    uint32 millisec_value;
    uint32 sec_value;
    uint32 min_value;
    uint32 hour_value;
    uint8 smpte_value[4];
    uint8 percent_value;
    uint32 samplenum_value;
    uint32 datapos_value;
} _PVPPlaybackPositionValueUnion;

typedef enum _PVPPlaybackPositionUnit
{
    PVPPBPOSUNIT_UNKNOWN = -1,
    PVPPBPOSUNIT_MILLISEC = 0,
    PVPPBPOSUNIT_SEC	= 1,
    PVPPBPOSUNIT_MIN	= 2,
    PVPPBPOSUNIT_HOUR	= 3,
    PVPPBPOSUNIT_SMPTE	= 4,
    PVPPBPOSUNIT_PERCENT = 5,
    PVPPBPOSUNIT_SAMPLENUMBER = 6,
    PVPPBPOSUNIT_DATAPOSITION = 7,
    PVPPBPOSUNIT_PLAYLIST = 8
}PVPPlaybackPositionUnit;

/**
* Playback positon mode indicates when the provided playback position is expected to take effect.
* 1) Now => implies that pause any current playback session and start playback from the new position
* 2) End of current play element => A play session can be composed of multiple play elements (say a
* playlist session) and the provided playlist position is to take effect once the current play
* element is complete (say reposition to song 7 after the current song is done)
* 3) End of current play session => This playback position is to take effect once the entire current
* play session is done.
*
*/
typedef enum _PVPPlaybackPositionMode
{
    PVPPBPOS_MODE_UNKNOWN = -1,
    PVPPBPOS_MODE_NOW = 0,
    PVPPBPOS_MODE_END_OF_CURRENT_PLAY_ELEMENT = 1,
    PVPPBPOS_MODE_END_OF_CURRENT_PLAY_SESSION = 2,
}PVPPlaybackPositionMode;

typedef struct _PVPPlaybackPosition
{
    _PVPPlaybackPositionValueUnion iPosValue;
    PVPPlaybackPositionUnit	iPosUnit;
#ifdef __cplusplus
    bool iIndeterminate;
#else
    c_bool iIndeterminate;
#endif
    PVPPlaybackPositionMode iMode;
    /*
     * Typically provided in case of a seek within a playsession
     * with multiple elements. This is the index ON which the
     * position info provided above applies. Say go to 30 seconds
     * into play element 7.
     */
    int32 iPlayElementIndex;
    _PVPPlaybackPositionValueUnion iPlayListPosValue;
    PVPPlaybackPositionUnit	iPlayListPosUnit;
    char* iPlayListUri;
} PVPPlaybackPosition;

#endif


