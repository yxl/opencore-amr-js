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
*  Name        : pv_player_factory.h
*  Part of     :
*  Interface   :
*  Description : Factory classes to create pvPlayer engine
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_PLAYER_FACTORY_H_INCLUDED
#define PV_PLAYER_FACTORY_H_INCLUDED

// Forward declaration
class PVPlayerInterface;
class PVCommandStatusObserver;
class PVInformationalEventObserver;
class PVErrorEventObserver;
class PVPlayerDataSink;
class PvmiMIOControl;

/**
 * PVPlayerFactory class is a singleton class which instantiates and provides
 * access to pvPlayer engine. It returns an PVPlayerInterface
 * reference, the interface class of the pvPlayer SDK.
 *
 * The application is expected to contain and maintain a pointer to the
 * PVPlayerInterface instance at all time that pvPlayer engine is active.
 **/
class PVPlayerFactory
{
    public:

        /**
         * Creates an instance of a pvPlayer engine. If the creation fails, this function will leave.
         *
         * @param aCmdStatusObserver     The observer for command status
         * @param aErrorEventObserver    The observer for unsolicited error events
         * @param aInfoEventObserver     The observer for unsolicited informational events
         *
         * @returns A pointer to a player or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVPlayerInterface* CreatePlayer(PVCommandStatusObserver* aCmdStatusObserver,
                PVErrorEventObserver *aErrorEventObserver,
                PVInformationalEventObserver *aInfoEventObserver);
        /**
         * This function allows the application to delete an instance of a pvPlayer
         * and reclaim all allocated resources.  A player can be deleted only in
         * the idle state. An attempt to delete a player in any other state will
         * fail and return false.
         *
         * @param aPlayer The player to be deleted.
         *
         * @returns A status code indicating success or failure.
         **/
        OSCL_IMPORT_REF static bool DeletePlayer(PVPlayerInterface* aPlayer);
};


/**
 * PVPlayerDataSinkFactory class is a singleton class which instantiates and provides
 * access to player data sink. It returns an PVPlayerDataSink
 * reference, the interface class of the player data sink.
 *
 **/

class PVPlayerDataSinkFactory
{
    public:
        /**
         * Creates an instance of a pvPlayer engine. If the creation fails, this function will leave.
         *
         * @param io_interface_ptr   A pointer to a media io interface
         *
         * @returns A pointer to a player data sink or leaves if instantiation fails
         **/

        OSCL_IMPORT_REF static PVPlayerDataSink* CreateDataSink(PvmiMIOControl* io_interface_ptr);

        /**
         * This function allows the application to delete an instance of the player data sink
         * and reclaim all allocated resources.
         *
         * @param data_sink_ptr The pointer to the player data sink to be deleted
         *
         * @returns A status code indicating success or failure.
         **/

        OSCL_IMPORT_REF static bool DeletePVPlayerSink(PVPlayerDataSink* data_sink_ptr);
};


#endif // PV_PLAYER_FACTORY_H_INCLUDED
