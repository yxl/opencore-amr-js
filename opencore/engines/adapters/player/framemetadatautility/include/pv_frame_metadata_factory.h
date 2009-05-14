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
*  Name        : pv_frame_metadata_factory.h
*  Part of     :
*  Interface   :
*  Description : Factory classe to create and destroy an instance of pvFrameAndMetadata utility
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_FRAME_METADATA_FACTORY_H_INCLUDED
#define PV_FRAME_METADATA_FACTORY_H_INCLUDED

// Forward declaration
class PVFrameAndMetadataInterface;
class PVCommandStatusObserver;
class PVInformationalEventObserver;
class PVErrorEventObserver;

/**
 * PVFrameAndMetadataFactory class is a singleton class which instantiates and provides
 * access to pvFrameAndMetadata utility. It returns an PVFrameAndMetadataInterface
 * reference, the interface class of the pvFrameAndMetadata utility.
 *
 * The application is expected to contain and maintain a pointer to the
 * PVFrameAndMetadataInterface instance at all time that pvFrameAndMetadata utility is active.
 **/
class PVFrameAndMetadataFactory
{
    public:

        /**
         * Creates an instance of a pvFrameAndMetadata utility. If the creation fails, this function will leave.
         *
         * @param aOutputFormatMIMEType  The output format when retrieving a frame specified as a MIME string
         * @param aCmdStatusObserver     The observer for command status
         * @param aErrorEventObserver    The observer for unsolicited error events
         * @param aInfoEventObserver     The observer for unsolicited informational events
         *
         * @returns An interface pointer to a pvFrameAndMetadata utility or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVFrameAndMetadataInterface* CreateFrameAndMetadataUtility(char *aOutputFormatMIMEType,
                PVCommandStatusObserver* aCmdStatusObserver,
                PVErrorEventObserver *aErrorEventObserver,
                PVInformationalEventObserver *aInfoEventObserver);
        /**
         * This function allows the application to delete an instance of a pvFrameAndMetadata utility
         * and reclaim all allocated resources.  An utility instance can be deleted only in
         * the idle state. An attempt to delete an instance in any other state will
         * fail and return false.
         *
         * @param aUtility The interface pointer to an utility instance to be deleted.
         *
         * @returns A status code indicating success or failure.
         **/
        OSCL_IMPORT_REF static bool DeleteFrameAndMetadataUtility(PVFrameAndMetadataInterface* aUtility);
};


#endif // PV_FRAME_METADATA_FACTORY_H_INCLUDED


