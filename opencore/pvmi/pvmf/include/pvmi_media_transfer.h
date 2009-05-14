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
/*! \addtogroup pvmimiocontrol PvmiMediaTransfer
 *
 * @{
 */


/*! \file pvmi_media_transfer.h
    \brief Defines an abstract interface for media transfer.
*/

#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#define PVMI_MEDIA_TRANSFER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif

/** status bitmasks */
#define PVMI_MEDIAXFER_STATUS_WRITE 0x01
#define PVMI_MEDIAXFER_STATUS_READ 0x02

/** values for format types */
#define PVMI_MEDIAXFER_FMT_TYPE_DATA 1
#define PVMI_MEDIAXFER_FMT_TYPE_COMMAND 2
#define PVMI_MEDIAXFER_FMT_TYPE_NOTIFICATION 3

/** default values for format index in cases where it is not negotiated
between the peers */
#define PVMI_MEDIAXFER_FMT_INDEX_DATA 1
#define PVMI_MEDIAXFER_FMT_INDEX_FMT_SPECIFIC_INFO 2
#define PVMI_MEDIAXFER_FMT_INDEX_END_OF_STREAM 3
#define PVMI_MEDIAXFER_FMT_INDEX_RE_CONFIG_NOTIFICATION 4

/** Data in writeAsync is an PVMFAsyncEvent object of PVMFInfoEvent event category */
#define PVMI_MEDIAXFER_FMT_INDEX_INFO_EVENT 5

/** Data in writeAsync is an PVMFAsyncEvent object of PVMFErrorEvent event category */
#define PVMI_MEDIAXFER_FMT_INDEX_ERROR_EVENT 6


/* Format for flags that are send as part of write async */
#define PVMI_MEDIAXFER_MEDIA_DATA_FLAG_NONE       0x00000000
#define PVMI_MEDIAXFER_MEDIA_DATA_FLAG_MARKER_BIT 0x00000001
#define PVMI_MEDIAXFER_MEDIA_DATA_FLAG_DURATION_AVAILABLE_BIT 0x00000002
#define PVMI_MEDIAXFER_MEDIA_DATA_FLAG_NO_RENDER_BIT 0x00000004

/* Data structure to hold info associated with media transfer */
typedef struct __PvmiMediaXferHeader
{
    uint32 seq_num;
    PVMFTimestamp timestamp;
    uint32 flags;
    uint32 duration;
    uint32 stream_id;
    OsclAny *private_data_ptr;
    uint32 private_data_length;
} PvmiMediaXferHeader;

/**
 * This class defines an abstract interface for media transfer between two connected
 * PvmiMediaTransfer objects.
 */
class PvmiMediaTransfer
{

    public:
        virtual ~PvmiMediaTransfer() {}

        /**
         * This method is used to set a peer for Media Transfer
         *
         * @param aPeer The peer that would be used for media transfer
         * @throw In case of failure, leaves with an appropriate error
         */
        virtual void setPeer(PvmiMediaTransfer* aPeer) = 0;

        /**
         * This method specifies which memory allocators shall be used for input and output
         *
         * @param read_write_alloc Specifies the read and write allocator
         *
         * @throw If the choice of allocator is unacceptable, the function
         *        leaves with PvmfErrNotSupported.
         */
        virtual void useMemoryAllocators(OsclMemAllocator* read_write_alloc) = 0;


        /**
         * This method is used by a component to send data actively to its peer.
         *
         * @param format_type          Identifies whether the message that is being written is data/command/notification
         * @param format_index         It is used if multiple simultaneous formats are used in media transfer
         * @param data                 Specifies the media data
         * @param data_len             Specifies the media data length
         * @param data_header_info     Contains info (timestamp, flags,...) associated with the data
         * @param aContext             Optional opaque data to be passed back to
         *                             user with the writeComplete response
         *
         * @returns    The function returns a unique command id if accepted.
         * @throw  if request not accepted, it leaves with an appropriate error code
         *         If the function should leave due to a recoverable problem like an
         *         overflow condition, the caller should wait for the status
         *         to change before attempting to send more media data.
         */
        virtual PVMFCommandId writeAsync(
            uint8 format_type, int32 format_index,
            uint8* data, uint32 data_len,
            const PvmiMediaXferHeader& data_header_info,
            OsclAny* aContext = NULL) = 0;

        /**
         * This method is used to report completion of write operation.
         * It can also be assumed that the corresponding data buffer can now be
         * reused
         *
         * @param aStatus     status parameter indicates whether the write
         *                    operation corresponding to write_cmd_id was
         *                    successful or not.
         * @param write_cmd_id corresponding to write_cmd_id returned in writeAsyn
         * @param aContext     The Opaque context, same as the one sent
         *                     in writeAsync request
         *
         * @throw  in case of failure, leaves with an appropriate error
         */
        virtual void writeComplete(
            PVMFStatus aStatus,
            PVMFCommandId write_cmd_id,
            OsclAny* aContext) = 0;

        /**
         * This API should be used on its peer by a component to receive data actively
         *
         * @param data,                  The read buffer.
         * @param max_data_len           The maximum length of the read buffer
         * @param aContext               The context parameter is an opaque handle provided by the caller.
         * @param formats, num_formats   Specify the indices of media data that may be read into this buffer
         *
         * @returns    The function returns a unique command id if accepted.
         * @throw  if request not accepted, it leaves with an appropriate error code
         *         If the function should leave due to a recoverable problem like an
         *         underflow condition, the caller should wait for the status
         *         to change before attempting to send more media data.
         */
        virtual PVMFCommandId readAsync(
            uint8* data, uint32 max_data_len,
            OsclAny* aContext = NULL,
            int32* formats = NULL, uint16 num_formats = 0) = 0;

        /**
         *  This function signals the completion of the read operation
         *
         * @param aStatus              Status parameter indicates whether the read
         *                             operation corresponding to read_cmd_id was
         *                             successful or not.
         * @param read_cmd_id          Corresponding to read_cmd_id returned in readAsync
         * @param format_index         The format_index parameter specifies the format
         *                             of the data that was read
         * @param data_header_info     Contains info (timestamp, flags,...) associated with the data
         * @param aContext             The Opaque context, same as the one sent
         *                             in readAsync request
         *
         * @throw  in case of failure, leaves with an appropriate error
         */
        virtual void readComplete(
            PVMFStatus aStatus,
            PVMFCommandId read_cmd_id,
            int32 format_index,
            const PvmiMediaXferHeader& data_header_info,
            OsclAny* aContext) = 0;

        /**
         *  A media I/O component may need to signal changes in its operating
         *  status like suspension/resumption of read/write operations.
         *  This is useful for recovering from temporary failures during read
         *  and write operations. The status update is communicated using the i
         *  following function.
         *
         * @param status_flags   The status_flags parameter provides the read,
         *                       write and error status using bitmasks i
         *                       PVMI_MEDIAXFER_STATUS_READ,
         *                       PVMI_MEDIAXFER_STATUS_WRITE.
         *                       A status of 1 indicates OK and 0 indicates Not OK.
         *
         * @throw  in case of failure, leaves with an appropriate error
         */
        virtual void statusUpdate(uint32 status_flags) = 0;

        /**
         *  Any outstanding writeAsync or readAsync commands may be cancelled by
         *  specifying the unique command id that was returned for the function
         *  call. The component shall attempt to cancel the operation
         *  if possible, although it cannot guarantee that the operation will be
         *  cancelled.
         *
         * @param command_id  unique command_id returned by writeAsyn or readAsync
         *
         * @throw  If the cancel request cannot be accepted, it leaves with an
         *         appropriate error
         */
        virtual void cancelCommand(PVMFCommandId command_id) = 0;

        /**
         * Cancels all outstanding writeAsync or readAsync commands.
         * The component shall attempt to cancel the operations if possible,
         * although it cannot guarantee that the operations will be cancelled.
         *
         * @throw  If the cancel request cannot be accepted, it leaves with an
         *         appropriate error
         */
        virtual void cancelAllCommands() = 0;
};

#endif // PVMI_MEDIA_TRANSFER_H_INCLUDED
