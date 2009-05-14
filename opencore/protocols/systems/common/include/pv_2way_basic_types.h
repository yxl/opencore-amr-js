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
/*
* ==============================================================================
*  Name        : pv_2way_basic_types
*  Part of     :
*  Interface   :
*  Description : Basic / low-level enums and types used by PV2Way engine
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_2WAY_BASIC_TYPES_H_INCLUDED
#define PV_2WAY_BASIC_TYPES_H_INCLUDED

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

// CONSTANTS
/** The starting error code for 2way specific errors **/
const int PV2WayErrorStatusStart = (-10500);
/** There was an error dispatching muxed data to the downstream node **/
const int PV2WayDispatchError = PV2WayErrorStatusStart - 1;
/** The request was rejected by the peer **/
const int PV2WayErrorRejected = PV2WayErrorStatusStart - 5;
/** Signals replacement of an existing resource **/
const int PV2WayErrReplaced = PV2WayErrorStatusStart - 6;

// DATA TYPES
/**
 *  TPVTerminalType enum
 *  TPVTerminalType enumerates the possible 2-way protocols
 **/
typedef enum TPVTerminalType
{
    PV_323,/* based on H.323 */
    PV_324M, /* based on H.324m */
    PV_SIP, /* based on SIP */
    PV_TERMINAL_TYPE_NONE
}PV2WayTerminalType;

/**
 * TPVLoopbackMode Enum
 *
 * TPVLoopbackMode emumerates the possible loopback options that can be used with the pv2way SDK
 **/
typedef enum TPVLoopbackMode
{
    PV_LOOPBACK_NONE,
    PV_LOOPBACK_COMM, /* Loops back from Comm source to Comm sink */
    PV_LOOPBACK_ENGINE, /* Loops back media at the engine level, no mux involved */
    PV_LOOPBACK_MUX /* Loops back at the output of the mux */
}PV2WayLoopbackMode;

/**
 * TPVDirection Enum
 *
 * TPVDirection emumerates the direction of the track.
 **/
typedef enum TPVDirection
{
    PV_DIRECTION_NONE = 0,
    INCOMING = 1,
    OUTGOING = 2,
    PV_DIRECTION_BOTH = 3
}PV2WayDirection;

/**
Enumeration of high level media types supported by the SDK
 **/
typedef enum TPVMediaType_t
{
    PV_MEDIA_NONE = 0,
    PV_CONTROL = 1,
    PV_AUDIO = 2,
    PV_VIDEO = 4,
    PV_DATA = 8,
    PV_USER_INPUT = 16,
    PV_MULTIPLEXED = 32,
    PV_MEDIA_ALL = 0xFFFF
}PV2WayMediaType;

/**
 * PVTrackId uniquely identifies a track for transferring audio/video in a particular direction -
 * receive or transmit.
 *
 **/
typedef unsigned int PVTrackId;

/**
 * TPV2WayState Class
 *
 * An enumeration of the major states of the pv2way engine.
 **/
typedef enum
{
    /**
    	The state immediately after the pv2way instance has been successfully created or instantiated.
    	No resources have been allocated yet.
    **/
    EIdle = 0,
    /**
    	The pv2way is in this state when it is initializing from the EIdle to the ESetup state.  The terminal queries the
    	available device capabilities (encode, decode, mux), acquires resources to make a two-way call (codecs, formats, memory etc)
    	and transitions to the ESetup state when it will be ready to accept setup parameters and Connect.
    	If initializing fails, the pv2way relinquishes the resources and reverts to the EIdle state.
    **/
    EInitializing,
    /**
    	The state where the pv2way instance is in the process of receiving setup parameters from the application, for encoding,
    	multiplexing, capturing and rendering.  Each time a new set of parameters is passed in, validation will take place and a
    	status will be returned accordingly.  A valid data source and data sink for the communications port are to be added to the
    	terminal in this state before it can be transitioned to the Econnecting state.  Media sources and sinks can also be added
    	at this time.
    **/
    ESetup,
    /**
    	The state where the pv2way instance has received a call to start connecting.  It starts communication with the remote terminal
    	to exchange media capabilities and channel configuration in preparation for the establishment of media channels.
    **/
    EConnecting,
    /**
    	The state after all control signaling is completed.  The terminal is now able to open media tracks for audio and video.
    **/
    EConnected,
    /**
    	The state where the terminal is shutting down all tracks and the multiplex.
    **/
    EDisconnecting,
    /**
    	The state where the terminal is releasing all resources and transitioning to the EIdle state.
    **/
    EResetting
} PV2WayState;

/**
 * TPVTIndicationType enum
 *
 *  Enumeration of unsolicited indications from pv2way.
 *
 **/
enum TPVTIndicationType
{

    /**
     * Indicates that the peer terminal has established an incoming track.  The
     * local buffer specifies the media type associated with the track.
     * The first octet of the local buffer indicates the media type.
     * The second,third and fourth octets are reserved.
     * The four octets from five to eight are to be interpreted as a unique track id.
     * The format type and additional capabilities are indicated using the PV2WayTrackInfoInterface extension interface.
     **/
    PVT_INDICATION_INCOMING_TRACK,
    /**
     * Indicates that the local terminal has established an outgoing track that is
     * acceptable to the peer.  The local buffer specifies the media type associated with the track.
     * The first octet of the local buffer indicates the media type.
     * The second,third and fourth octets are reserved.
     * The four octets from five to eight are to be interpreted as a unique track id.
     * The format type and additional capabilities are indicated using the PV2WayTrackInfoInterface extension interface.
     **/
    PVT_INDICATION_OUTGOING_TRACK,
    /**
     * Indicates that 2way engine has ended the current telephony session.  The app
     * can now either reset the engine or make a subsequent call.
     *
     **/
    PVT_INDICATION_DISCONNECT,
    /**
     * Indicates the start of unsolicited closure of an incoming/outgoing track.  The PVT_INDICATION_CLOSE_TRACK
     * indication will be sent when the track is completely close.
     * The first octet of the local buffer indicates the direction of the track.
     * The second and third octets indicates the track id.
     **/
    PVT_INDICATION_CLOSING_TRACK,
    /**
     * Indicates an unsolicited closure of an incoming/outgoing track.  Any media sink/source
     * associated with this will be stopped and returned to the application.
     * The first octet of the local buffer indicates the media type of the track.
     * The second octet indicates the direction.
     * The third octet indicates whether there is a replacement for this track available.  If true,
     * the application may add data source/sink for this track again.
     **/
    PVT_INDICATION_CLOSE_TRACK,
    /**
     * Indicates that the remote terminal has paused an incoming track.  Any media sink
     * associated with this will be stopped.
     *
     **/

    PVT_INDICATION_PAUSE_TRACK,
    /**
     * Indicates that the remote terminal has resumed an incoming track.  Any media sink
     * associated with this will be restarted.
     *
     **/
    PVT_INDICATION_RESUME_TRACK,
    /**
     * Indicates an internal error in the pv2way engine.  The derived class provides
     * further information about the actual error.
     *
     **/
    PVT_INDICATION_INTERNAL_ERROR,
};


// CLASS DECLARATION

/**
* PV2WayInitInfo Class
*
* PV2WayInitInfo is an interface required
* for protocols specific classes pass to the PV2WayInterface's
* InitL() method
*
**/
class PV2WayInitInfo
{
    public:
        /**
        *  pure virtual method that must be overridden to return
        *  the classname of the actual subclass
         **/
        virtual void GetInitInfoClassName(OSCL_wString& aClassName) = 0;

        virtual ~PV2WayInitInfo() { }

        /**
        *  The list of audio formats that can be transmitted
         **/
        Oscl_Vector<const char*, OsclMemAllocator> iOutgoingAudioFormats;

        /**
        *  The list of video formats that can be transmitted
         **/
        Oscl_Vector<const char*, OsclMemAllocator> iOutgoingVideoFormats;

        /**
        *  The list of audio formats that can be received
         **/
        Oscl_Vector<const char*, OsclMemAllocator> iIncomingAudioFormats;

        /**
        *  The list of video formats that can be received
         **/
        Oscl_Vector<const char*, OsclMemAllocator> iIncomingVideoFormats;

};


/**
* PV2WayConnectOptions Class
 *
* PV2WayConnectOptions class contains options to be specified during connect
 **/
class PV2WayConnectOptions
{
    public:
        /**
         * Default Constructor
         **/
        PV2WayConnectOptions()
                : iLoopbackMode(PV_LOOPBACK_NONE),
                iLocalId(NULL),
                iLocalIdSize(0),
                iRemoteId(NULL),
                iRemoteIdSize(0) {}

        /**
         * Constructor
         * @param aLoopbackMode
         *         The loopback mode to used during Connect
         * @param aLocalId, aLocalIdSize
         *         A unique octet string identifying the local terminal
         * @param aRemoteId, aRemoteIdSize
         *         A unique octet string identifying the peer (Used only in 2-Stage dialling)
         * @returns void
         *
         **/
        PV2WayConnectOptions(TPVLoopbackMode aLoopbackMode,
                             uint8* aLocalId, uint32 aLocalIdSize,
                             uint8* aRemoteId, uint32 aRemoteIdSize)
                : iLoopbackMode(aLoopbackMode),
                iLocalId(aLocalId),
                iLocalIdSize(aLocalIdSize),
                iRemoteId(aRemoteId),
                iRemoteIdSize(aRemoteIdSize)
        {}

        /**
        * Pure virtual method that must be overridden.  Retrieves class name
        *
        * @param aClassName
        *         A reference to an OSCL_wString, which is to hold the subclass
        *          name
        * @returns void
        **/
        virtual void GetConnectInfoClassName(OSCL_wString &aClassName) = 0;

        /**
        *  The loopback mode
        **/
        TPVLoopbackMode iLoopbackMode;

        /**
        *  The id of the local terminal
        **/
        uint8* iLocalId;
        /**
        *  The size of the local id
        **/
        uint32 iLocalIdSize;

        /**
        *  The id of the peer
        **/
        uint8* iRemoteId;
        /**
        *  The size of the remote id
        **/
        uint32 iRemoteIdSize;
};
#endif
