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
#ifndef TSC_H324M_CONFIG_INTERFACE_H_INCLUDED
#define TSC_H324M_CONFIG_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVT_COMMON_H_INCLUDED
#include "pv_2way_h324m_types.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

#define PVH324MConfigUuid PVUuid(0x2b0b54e2,0x7079,0x46c6,0xb2,0x3e,0x04,0xff,0xd3,0x0e,0x14,0x36)


/**
 * H324MConfigInterface Class
 *
 * H324MConfigInterface provides H.324m specific configuration APIs.
 **/
class H324MConfigInterface : public PVInterface
{
    public:
        /**
         * This API allows the user to specify separate observers for the 324m interface.  Otherwise, the default
         * observers will be used
         *
         * @param aObserver        the observer for command status and for unsolicited informational events
         **/
        virtual void SetObservers(PVCommandStatusObserver* aCmdStatusObserver,
                                  PVInformationalEventObserver* aInfoEventObserver,
                                  PVErrorEventObserver* aErrorEventObserver) = 0;

        /**
         * This API allows the user to specify the starting H223 multiplex level
         *
         * @param aLevel
         *			The starting H223 multiplex level.  Note that the final level that is neotiated will depend
         *          on the starting level of the peer
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMultiplexLevel(PVH223Level aLevel,
                                              OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify maximum outgoing sdu sizes for each adaptation layer
         *
         * @param aLayer
         *			The h223 adaptation layer type
         * @param aSize
         *			The sdu size
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMaxSduSize(PVH223AdaptationLayer aLayer,
                                          int32 aSize,
                                          OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify maximum incoming sdu sizes for each adaptation layer.  This is indicated
         * to the peer via the TCS
         *
         * @param aLayer
         *			The h223 adaptation layer type
         * @param aSize
         *			The sdu size
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMaxSduSizeR(PVH223AdaptationLayer aLayer,
                                           int32 aSize,
                                           OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify the configuration for outgoing AL channels
         *
         * @param aMediaTypes
         *          Media types for which configuration is being specified
         * @param aConfig
         *			Adaptation Layer configuration
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetOutgoingChannelConfiguration(int32 aMediaTypes,
                PVH223AlConfig* aConfig,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify the configuration for incoming channels
         *
         * @param iAudioAdaptationLayers
         *          The allowable adaptation layers for incoming audio channels specified as a bitarray
         * @param iVideoAdaptationLayers
         *          The allowable adaptation layers for incoming video channels specified as a bitarray
         * @param iDataAdaptationLayers
         *          The allowable adaptation layers for incoming data channels specified as a bitarray
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetIncomingChannelConfiguration(uint32 iAudioAdaptationLayers,
                uint32 iVideoAdaptationLayers,
                uint32 iDataAdaptationLayers,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to limit the size of the outgoing h223 pdus
         *
         * @param aMaxPduSize
         *			The max pdu size
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMaxPduSize(int32 aMaxPduSize,
                                          OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify the terminal type that is advertized to the peer.  This can be used to
         * force the local terminal to be master/slave when communicating with a peer 324m terminal for testing purposes.
         *
         * @param aTerminalType The terminal type
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetTerminalType(uint8 aTerminalType,
                                            OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify whether Request Multiplex Entry is sent to the remote terminal after TCS
         *
         * @param aSendRme If true, RME is sent to the peer after TCS
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SendRme(OsclAny* aContextData = NULL) = 0;

        /**
         * This API causes a maxMuxPduSize request to be sent to the remote terminal if set to a valid value (64 - 255).
        	   This is done after TCS if the remote terminal supports the maxMuxPduCapability
         *
         * @param aRequestMaxMuxPduSize The max mux pdu size
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMaxMuxPduSize(int32 aRequestMaxMuxPduSize,
                                             OsclAny* aContextData = NULL) = 0;

        /**
         * This API sets the max ccsrl sdu size
         *
         * @param aMaxCcsrlSduSize The max ccsrl sdu size
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetMaxMuxCcsrlSduSize(int32 aMaxCcsrlSduSize,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API may be called only after the media source has been successfully added
         * to the pv2way engine.  It causes the 2way engine to immediately send out a fast update frame
         * specific to the media type identified by the aTrack parameter.
         * @param aChannelId
         *         The identifier for the track
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response

         * @returns A unique command id for asynchronous completion
        **/
        virtual PVCommandId FastUpdate(PVChannelId aChannelId,
                                       OsclAny* aContextData = NULL) = 0;

        /**
         * Sends a Round Trip Determination message to the peer and indicates the round trip delay to the caller
         * on completion of the command.  The round trip delay is stored in 4 bytes in the local buffer of the completion event
         * in network byte order.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SendRtd(OsclAny* aContextData = NULL) = 0;

        /**
         * Sets the vendor identification data.  This does not cause the stack to issue a vendor identifiation request.
         * Set to NULL to disable sending vendor id.  If set to a valid parameter before Connect, it will cause the stack
         * to automatically send it along with the TCS message.
         * @param cc
         *         T35 Country code
         * @param ext
         *         T35 Extension
         * @param mc
         *         T35 Manufacturer code
         * @param aProduct
         *         Product number
         * @param aVersion
         *         Version number
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetVendor(uint8 cc, uint8 ext, uint32 mc,
                                      const uint8* aProduct, uint16 aProductLen,
                                      const uint8* aVersion, uint16 aVersionLen,
                                      OsclAny* aContextData = NULL) = 0;

        /**
         * Sends an end session command to the peer.  Only to be used for testing purposes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SendEndSession(OsclAny* aContextData = NULL) = 0;

        /**
         * Sets the disconnect timeout interval.
         *
         * @param aTimeout The timeout value in seconds
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetEndSessionTimeout(uint32 aTimeout,
                OsclAny* aContextData = NULL) = 0;

        /**
         * Sets an H.324 timer/counter value.  This should be called before ConnectL is invoked.
         * The  supported timers are:
         *		T106	Master Slave Determination (in units of 1s)
         *		T101	Capability Exchange (in units of 1s)
         *		T103	Uni-directional and Bi-directional Logical Channel Signalling (in units of 1s)
         *		T108	Close Logical Channel (in units of 1s)
         *		T104	H.223 Multiplex Table (in units of 1s)
         *		T109	Mode Request (in units of 1s)
         *		T105	Round Trip Delay (in units of 1s)
         *		T107	Request Multiplex Entry (in units of 100ms)
         *      T401    SRP retransmission (in units of 100ms)
         * The supported counters are:
         *      N100	H245 (TCS, MSD)
         *      N401    SRP retransmission
         * @param aTimerCounter
         *         Identifies whether a timer or counter is being set.
         * @param aSeries
         *         Identifies the H.324 timer/counter series.
         * @param aSeriesOffset
         *         Specifies the offset within a particular series.
         *         E.g.
         *              aTimerCounter=EH324Timer, aSeries=1, aSeriesOffset=1 indicates T101.
         *              aTimerCounter=EH324Timer, aSeries=4, aSeriesOffset=1 indicates T401.
         *              aTimerCounter=EH324Counter, aSeries=4, aSeriesOffset=1 indicates T401.
         * @param aValue
         *         The new value for the H.324 timer/counter
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         **/
        virtual PVCommandId SetTimerCounter(PVH324TimerCounter aTimerCounter,
                                            uint8 aSeries, uint32 aSeriesOffset,
                                            uint32 aValue,
                                            OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to specify the supported resolutions for video for transmit and receive.
         *
         * @param aDirection
         *         The direction (Tx/Rx) for which the capability is specified.
         * @param aResolutions
         *         An array of resolutions.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         **/
        virtual PVCommandId SetVideoResolutions(PVDirection aDirection,
                                                Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& aResolutions,
                                                OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to send the vendor id info to the peer.
         * Note:  Calling this API during call-setup negotiations can affect the time for
         * call-setup adversely.
         *
         **/
        virtual PVCommandId SendVendorId(OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to send a videoTemporalSpatialTradeOff command to the peer.
         * It is a request to the remote encoder to adjust its encoding in accordance with the tradeoff value.
         * A value of 0 indicates a high spatial resolution and a value of 31 indicates a high frame rate.
         * The values from 0 to 31 indicate monotonically a higher frame rate. Actual values do not correspond
         * to precise values of spatial resolution or frame rate.
         *
         **/
        virtual PVCommandId SendVideoTemporalSpatialTradeoffCommand(PVChannelId aLogicalChannel,
                uint8 aTradeoff,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to send a videoTemporalSpatialTradeOff command to the peer.
         * It is an indication to the remote decoder that the local encoder has adjusted its encoding parameters
         * according to the tradeoff value.
         * A value of 0 indicates a high spatial resolution and a value of 31 indicates a high frame rate.
         * The values from 0 to 31 indicate monotonically a higher frame rate. Actual values do not correspond
         * to precise values of spatial resolution or frame rate.
         *
         **/
        virtual PVCommandId SendVideoTemporalSpatialTradeoffIndication(PVChannelId aLogicalChannel,
                uint8 aTradeoff,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API allows the user to send a SkewIndication to the peer.
         * Skew is measured in milliseconds, and indicates the maximum number of milliseconds that the data on
         * logicalChannel2 is delayed from the data on logicalChannel1 as delivered to the network transport.
         *
         **/
        virtual PVCommandId SendSkewIndication(PVChannelId aLogicalChannel1,
                                               PVChannelId aLogicalChannel2,
                                               uint16 aSkew,
                                               OsclAny* aContextData = NULL) = 0;

        /**
        * This API allows the user to configure the logical channel buffer sizes for incoming
        * and outgoing logical channels.
         * @param aDirection
         *         The direction (Rx or Tx).
         * @param aBufferingMs
         *         The amount of buffering in milliseconds.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
        **/
        virtual PVCommandId SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
                uint32 aOutBufferingMs,
                OsclAny* aContextData = NULL) = 0;

        /**
         * Causes the pv2way to send the specified user input to the remote terminal using
         * control channel.  The user input can be either DTMF ot Alphanumeric
         * @param user_input A pointer to either CPVUserInputDtmf or CPVUserInputAlphanumeric
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SendUserInput(CPVUserInput* user_input,
                                          OsclAny* aContextData = NULL) = 0;

};

/**
 * PVH324MIndicationType enum
 *
 *  Enumeration of unsolicited H324m specific indications from pv2way.
 *
 **/
enum PVH324MIndicationType
{
    /**
     * Indicates the receipt of a videoSpatialTemporalTradeoff command from the peer.
     * The first 2 bytes of the event local buffer indicate the logical channel (network byte order) and the 3rd
     * byte indicates the tradeoff value.
     *
     **/
    PV_INDICATION_VIDEO_SPATIAL_TEMPORAL_TRADEOFF_COMMAND,
    /**
     * Indicates the receipt of a videoSpatialTemporalTradeoff indication from the peer.
     * The first 2 bytes of the event local buffer indicate the logical channel (network byte order) and the 3rd
     * byte indicates the tradeoff value.
     *
     **/
    PV_INDICATION_VIDEO_SPATIAL_TEMPORAL_TRADEOFF_INDICATION,
    /**
     * Indicates a fast update message from the remote terminal.  The first two bytes of the
     * local buffer encode the logical channel number in network byte order.
     *
     **/
    PV_INDICATION_FAST_UPDATE,
    /**
     * Indicates an incoming RTD command.
     *
     **/
    PV_INDICATION_RTD,
    /**
     * Indicates an incoming request multiplex entry command.
     *
     **/
    PV_INDICATION_RME,
    /**
     * Indicates an incoming vendor id indication message.
     *
     **/
    PV_INDICATION_VENDOR_ID,
    /**
     * Indicates the receipt of user input capability from the remote terminal.  The local buffer
     * contains the indices of the user input formats supported by the peer.
     *
     **/
    PV_INDICATION_USER_INPUT_CAPABILITY,
    /**
     * Indicates the receipt of user input from the remote terminal.  The derived class
     * contains the actual user input sequences received.
     *
     **/
    PV_INDICATION_USER_INPUT,
    /**
     * Indicates the receipt of a an h223SkewIndication indication from the peer.
     * The first 2 bytes of the event local buffer indicate the first logical channel, the 3rd and
     * 4th bytes the second logical channel and the 5th and 6th bytes the value of the skew in milliseconds.
     * All values are in network byte order.
     *
     **/
    PV_INDICATION_SKEW
};

#endif
