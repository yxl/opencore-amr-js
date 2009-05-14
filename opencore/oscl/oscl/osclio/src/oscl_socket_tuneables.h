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

#ifndef OSCL_SOCKET_TUNEABLES_H_INCLUDED
#define OSCL_SOCKET_TUNEABLES_H_INCLUDED

/*!
** This file contains default definitions of all the
** the tuning parameters for the Oscl sockets.
*/

/*!
** If parameters are defined in osclconfig_io.h, those definitions
** will take precedence over the ones in this file.
*/
#include "osclconfig_io.h"
#include "osclconfig_proc.h"

/*!
** PV_SOCKET_REQUEST_AO_PRIORITY sets the priority of the socket request
** completion AOs.
*/
#ifndef PV_SOCKET_REQUEST_AO_PRIORITY
#define PV_SOCKET_REQUEST_AO_PRIORITY OsclActiveObject::EPriorityNominal
#endif

/*!
** Set this to 0 or 1 to enable/disable socket stats logging
** with "OsclSocketStats" node.  This feature is fairly costly so should be off in
** production code.
*/
#ifndef PV_OSCL_SOCKET_STATS_LOGGING
#define PV_OSCL_SOCKET_STATS_LOGGING 0
#endif

/*!
** Enable/disable the PV socket server here.
*/
#ifndef PV_SOCKET_SERVER
#define PV_SOCKET_SERVER 1
#endif

#if(PV_SOCKET_SERVER)
/*!
** PV_SOCKET_SERVER_IS_THREAD chooses either the threaded or AO-based
** implementation of the PV socket server
*/
#ifndef PV_SOCKET_SERVER_IS_THREAD
//choose threaded implementation whenever the OS has threading support.
#define PV_SOCKET_SERVER_IS_THREAD OSCL_HAS_THREAD_SUPPORT
#endif

/*!
** PV_SOCKET_SERVER_SELECT chooses whether to use "select" call or not.
** In threaded mode, select call is required and is forced to "1".
** In AO mode, "select" call is an option that defaults to "0".
** Avoiding any "select" call was found to greatly reduce CPU usage
** on WinMobile devices.
*/
#if PV_SOCKET_SERVER_IS_THREAD
#undef PV_SOCKET_SERVER_SELECT
#define PV_SOCKET_SERVER_SELECT 1
#else
#ifndef PV_SOCKET_SERVER_SELECT
#define PV_SOCKET_SERVER_SELECT 0
#endif
#endif

/*!
** PV_SOCKET_SERVER_THREAD_PRIORITY sets the priority of the PV socket
** server thread.
*/
#ifndef PV_SOCKET_SERVER_THREAD_PRIORITY
#define PV_SOCKET_SERVER_THREAD_PRIORITY ThreadPriorityAboveNormal
#endif

/*!
** PV_SOCKET_SERVER_SELECT_TIMEOUT_MSEC sets duration of the select call
** in the PV socket server thread for the polling select loop implementation.
** When the timeout is -1, the select call will block forever waiting on a new request
** and will use a loopback socket to signal a new request.
** Note: if infinite wait is selected, but loopback socket is not available,
** the implementation will poll at 10 msec intervals.
*/
#ifndef PV_SOCKET_SERVER_SELECT_TIMEOUT_MSEC
#define PV_SOCKET_SERVER_SELECT_TIMEOUT_MSEC (-1)
#endif

/*!
** PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET enables the feature to wakeup the select
** call by writing to a loopback socket each time a new request comes in.
** This option is required to support the blocking select loop option of threaded
** server mode.  This option is forced to "0" in AO mode.
*/
#if PV_SOCKET_SERVER_IS_THREAD
#ifndef PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
#define PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET 1
#endif
#else
#undef PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
#define PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET 0
#endif

/*!
** PV_SOCKET_SERVER_AO_PRIORITY sets priority of the PV socket server
** AO for non-threaded implementations.
*/
#ifndef PV_SOCKET_SERVER_AO_PRIORITY
#define PV_SOCKET_SERVER_AO_PRIORITY (OsclActiveObject::EPriorityNominal)
#endif

/*!
** PV_SOCKET_SERVER_AO_INTERVAL_MSEC sets the AO scheduling interval
** of the PV socket server AO for non-threaded implementations.
*/
#ifndef PV_SOCKET_SERVER_AO_INTERVAL_MSEC
#define PV_SOCKET_SERVER_AO_INTERVAL_MSEC 5
#endif

/*!
** Set this to 0 or 1 to enable/disable PVLogger output from
** PV socket server.
** Note that socket server logging will appear in a different
** file when running threaded mode of socket server.
** This is quite a bit of logging, so it should generally be
** disabled.
*/
#ifndef PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT
#define PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT 0
#endif

/*!
** Set this to 0 or 1 to enable/disable setting the socket receive
** buffer size to 1 MB in the Bind call.  This setting only affects
** PV socket server implementations.
*
** When set to 1, the code will use the OsclSetRecvBufferSize macro
** to set the buffer size in the Bind  call.
*
** This setting was found to improve streaming performance on
** WinMobile devices, but should not generally be used.
*/
#ifndef PV_OSCL_SOCKET_1MB_RECV_BUF
#define PV_OSCL_SOCKET_1MB_RECV_BUF 0
#endif

/*!
** For detailed performance breakdown of time spend in OsclSocketServI AO.
** Output is logged under "OsclSchedulerPerfStats" node.  Should be off in
** production code.  This option is forced to "0" in threaded mode.
*/
#if PV_SOCKET_SERVER_IS_THREAD
#undef PV_SOCKET_SERVI_STATS
#define PV_SOCKET_SERVI_STATS 0
#else
#ifndef PV_SOCKET_SERVI_STATS
#define PV_SOCKET_SERVI_STATS 0
#endif
#endif

#endif//PV_SOCKET_SERVER

#endif



