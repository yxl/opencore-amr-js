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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               P V M I _ M I O _ C O N T R O L

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup pvmimiocontrol PvmiMioControl
 *
 * @{
 */


/*! \file pvmi_mio_control.h
    \brief This file contains the abstractions for media input-output control.
*/

#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#define PVMI_MIO_CONTROL_H_INCLUDED

/*
This class defines an abstract interface for media input-output control
*/
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef PVMI_MEDIA_IO_OBSERVER_H_INCLUDED
#include "pvmi_media_io_observer.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_FORMAT_TYPES_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

/** \class PvmiMIOControl
** This class defines an abstract interface for media input-output control
*/

class PvmiMIOControl
{

    public:

        virtual ~PvmiMIOControl() {}

        /**
         * This method establishes a connection to the media I/O component. This is a prerequisite
         * to any other interaction
         *
         * @param aSession      This output parametre will be filled in with an opaque session ID
         *                      which will be used to identify the connection session.
         * @param aObserver     Specifies an observer class to be registered
         *                      for handling the results of asynchronous requests
         * @returns             PVMFSuccess if the operation is succesful. PVMFFailure otherwise.
         */

        virtual PVMFStatus connect(PvmiMIOSession& aSession,
                                   PvmiMIOObserver* aObserver) = 0;



        /**
         * Once all interaction with the media I/O component is complete, the session can be
         * terminated with the disconnect call.  Any pending asynchronous requests associated
         * with that session will be cancelled and a cancel status will be returned for each
         * them using the observer callback.
         *
         * @param aSession      The opaque session ID which is used to identify the connection session.
         * @returns   PvmiSuccess if operation is succesful. PvmiFailure otherwise.
         */

        virtual PVMFStatus disconnect(PvmiMIOSession aSession) = 0;


        /**
         * The media I/O component provides a factory API to allocate and return a reference to PvmiMediaTransfer interface.
         * The call succeeds only if the supplied parameters to the function are within the capabilities of the
         * component as specified in the previous section.
         *
         * @param aSession        The opaque session ID which is used to identify the connection session.
         * @param read_formats    list of formats that will be used for reading with this end point
         * @param read_flags      Specify the modes that will be used for reading.
         * @param write_formats   list of formats that will be used for writing this end point
         * @param write_flags     Specify the modes that will be used for reading.
         * @returns               If all the parameters are verified, a reference to a PvmiMediaTransfer
         *                        interface shall be returned.
         * @throw The function shall leave with PvmfErrNotSupported,
         *        if any of the parameters are outside the  capabilities
         *        of the component.
         */

        virtual PvmiMediaTransfer* createMediaTransfer(
            PvmiMIOSession& aSession,
            PvmiKvp* read_formats = NULL, int32 read_flags = 0,
            PvmiKvp* write_formats = NULL, int32 write_flags = 0) = 0;



        /**
         * A media transfer session can be terminated by using this API
         *
         * @param aSession          The opaque session ID which is used to identify the connection session.
         * @param media_transfer    list of formats that will be used for reading with this end point
         * @returns A pre-requisite to deleting a PvmiMediaTransfer interface is
         *          that any outstanding read or write buffers must be returned
         *          to the interface.
         * @throw The function shall leave with PvmfErrBusy if an attempt is made
         *        to delete it while there are outstanding buffers.
         */

        virtual void deleteMediaTransfer(
            PvmiMIOSession& aSession,
            PvmiMediaTransfer* media_transfer) = 0;


        // Async requests that will trigger callback events
        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */
        virtual PVMFCommandId QueryUUID(const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                        bool aExactUuidsOnly = false,
                                        const OsclAny* aContext = NULL) = 0;


        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         * TBD:  Define the UIID, InterfacePtr structures
         *
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */
        virtual PVMFCommandId QueryInterface(const PVUuid& aUuid,
                                             PVInterface*& aInterfacePtr,
                                             const OsclAny* aContext = NULL) = 0;


        /**
         * Initializes the I/O component so that it is ready to operate.
         * This API may need to reserve exclusive control, power up the device, and initialize
         * the state of the device.
         * Because it is not unusual for that process to take some time, the Init() API is asynchronous.
         * A callback is made to the observer once the initialization is complete
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */
        virtual PVMFCommandId Init(const OsclAny* aContext = NULL) = 0;

        /**
         * Resets the I/O component to free resources and to return to the pre-initialized state.
         * Because it is not unusual for that process to take some time, the Reset() API is asynchronous.
         * A callback is made to the observer once the initialization is complete
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */
        virtual PVMFCommandId Reset(const OsclAny* aContext = NULL) = 0;

        /**
         * The start API call actually transitions the media I/O component to a state where it is
         * ready to process media data.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId Start(const OsclAny* aContext = NULL) = 0;

        /**
         * This API is used to signal that input or output should be paused momentarily.  Use Start() to resume.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId Pause(const OsclAny* aContext = NULL) = 0;

        /**
         * An API to complete any pending data.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId Flush(const OsclAny* aContext = NULL) = 0;

        /**
         * An API to discard any pending data without processing
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId DiscardData(const OsclAny* aContext = NULL) = 0;

        /**
         * An API to discard any pending data without processing up to the specified timestamp value
         * @param aTimestamp Timestamp value to discard data to
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId DiscardData(PVMFTimestamp aTimestamp, const OsclAny* aContext = NULL) = 0;

        /**
         * The stop API call immediately stops the component.  It doesn't complete any pending data
         * and tries to stop as quickly as possible.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual PVMFCommandId Stop(const OsclAny* aContext = NULL) = 0;

        /**
         * Cancels a specific outstanding async command
         * @param aCmd  a specific async command to cancel
         * @param aContext Optional opaque data to be passed back to
         *                 user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw  if the cancel request cannot be accepted, it leaves with an
         *         appropriate error
         */

        virtual PVMFCommandId CancelCommand(PVMFCommandId aCmd, const OsclAny* aContext = NULL) = 0;

        /**
         * Cancels all outstanding async commands
         * @param aContext Optional opaque data to be passed back to
         *                 user with the command response
         * @returns A unique command id for asynchronous completion
         * @throw  if the cancel request cannot be accepted, it leaves with an
         *         appropriate error
         */

        virtual PVMFCommandId CancelAllCommands(const OsclAny* aContext = NULL) = 0;

        /**
         * The purpose of this API is to indicate the thread context where the other APIs will be called.
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual void ThreadLogon() = 0;

        /**
         * Resets the thread context.  No other APIs should be within the same thread context after this call.
         * @throw in case of failure, it leaves with an appropriate error
         */

        virtual void ThreadLogoff() = 0;
};


#endif //PVMI_MIO_CONTROL_H_INCLUDED
