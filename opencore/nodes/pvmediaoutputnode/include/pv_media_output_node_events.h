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
 * @file pv_media_output_node_events.h
 * @brief
 */

#ifndef PV_MEDIA_OUTPUT_NODE_EVENTS_H_INCLUDED
#define PV_MEDIA_OUTPUT_NODE_EVENTS_H_INCLUDED


/**
 UUID for file format error and information event type codes
 **/
//d62cbd20-abca-11d9-9669-0800200c9a66
#define PVMFMediaOutputNodeEventTypesUUID PVUuid(0xd62cbd20,0xabca,0x11d9,0x96,0x69,0x08,0x00,0x20,0x0c,0x9a,0x66)

/**
 Maximum number of media messages that the sink node should buffer at
 once this number of messages are buffered, sink node sends an infor-
 mational event up to the engine, where the engine decides whether or
 not to start the playback
 **/
#define MIONODE_MAX_BUFFER_SIZE 1

/**
 Event codes for Media Output Node events.
 All the messages are of type PVMFBasicErrorInfoMessage
**/
typedef enum
{
    //This value is just a placeholder-- the event will not be reported.
    PVMFMoutNodeErr_First = 0

    //This error code indicates that a port referenced in a command
    //does not exist.
    , PVMFMoutNodeErr_PortNotExist

    //This error code indicates that a SkipMediaData command failed
    //on the port.
    , PVMFMoutNodeErr_PortSkipMediaData

    //This error code indicates that a Cancel command failed
    //because the indicated command is no longer queueud.
    , PVMFMoutNodeErr_CmdNotQueued

    //This error code indicates that the Media Output component does not exist
    , PVMFMoutNodeErr_MediaIONotExist

    //This error code indicates that the Media Output component was in
    //an unexpected state when the node tried to issue a command.
    , PVMFMoutNodeErr_MediaIOWrongState

    //These error codes indicate that a specific command to the
    //Media Output control component failed.
    , PVMFMoutNodeErr_MediaIOConnect
    , PVMFMoutNodeErr_MediaIOQueryCapConfigInterface//query for capability & config interface
    , PVMFMoutNodeErr_MediaIOSetParameterSync//setparametersync on config-cap failed.
    , PVMFMoutNodeErr_MediaIOInit
    , PVMFMoutNodeErr_MediaIOStart
    , PVMFMoutNodeErr_MediaIOPause
    , PVMFMoutNodeErr_MediaIOStop
    , PVMFMoutNodeErr_MediaIOFlush
    , PVMFMoutNodeErr_MediaIODiscardData
    , PVMFMoutNodeErr_MediaIOCancelCommand
    , PVMFMoutNodeErr_MediaIOReset

    //These error codes indicate errors in the data transfer to the media
    //Output componenet
    , PVMFMoutNodeErr_WriteAsync //unexpeced leave in writeAsync
    , PVMFMoutNodeErr_WriteComplete //unexpected error reported in writeComplete

    //These error codes indicate errors in the LATM parsing.
    , PVMFMoutNodeErr_LatmHeaderConvert //codec header conversion failed.
    , PVMFMoutNodeErr_LatmSampleConvert //audio sample conversion failed.

    //This error indicates an unexpected error in port processing resulting
    //in dropped data.
    , PVMFMoutNodeErr_Unexpected

    //This value is just a placeholder-- the event will not be reported.
    , PVMFMoutNodeErr_Last

    //This value is just a placeholder-- the event will not be reported.
    , PVMFMoutNodeInfo_First

    //Notifications of dropped or skipped frames.
    , PVMFMoutNodeInfo_FramesDropped
    , PVMFMoutNodeInfo_FramesSkipped

    //This value is just a placeholder-- the event will not be reported.
    , PVMFMoutNodeInfo_Last

} PVMFMediaOutputNodeErrorInfoEventTypes;

#endif // PV_MEDIA_OUTPUT_NODE_EVENTS_H_INCLUDED





