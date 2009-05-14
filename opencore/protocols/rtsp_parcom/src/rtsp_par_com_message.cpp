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

#include "oscl_base.h"
#include "rtsp_par_com.h"

OSCL_EXPORT_REF void
RTSPGenericMessage::reset()
{
    secondaryBufferSpace = secondaryBuffer;
    secondaryBufferSizeUsed = 0;

    numPtrFields = 0;

    msgType       = RTSPUnknownMsg;
    method        = METHOD_UNRECOGNIZED;
    statusCode    = 0;

    methodString.setPtrLen("", 0);

    reasonString.setPtrLen("", 0);

    cseq = 0;
    cseqIsSet = false;

    bufferSize = 0;
    bufferSizeIsSet = false;

    authorizationFlag = 0;
    authorizationFlagIsSet = 0;

    // transport = ... ?
    numOfTransportEntries = 0;
    for (int jj = 0; jj < RTSP_MAX_NUMBER_OF_TRANSPORT_ENTRIES; ++jj)
    {
        transport[jj].protocolIsSet = false;
        transport[jj].profileIsSet = false;
        transport[jj].transportTypeIsSet = false;
        transport[jj].deliveryIsSet = false;
        transport[jj].destinationIsSet = false;
        transport[jj].channelIsSet = false;
        transport[jj].appendIsSet = false;
        transport[jj].ttlIsSet = false;
        transport[jj].layersIsSet = false;
        transport[jj].portIsSet = false;
        transport[jj].client_portIsSet = false;
        transport[jj].server_portIsSet = false;
        transport[jj].modeIsSet = false;
        transport[jj].ssrcIsSet = false;
    }


    // range = ... ?
    rangeIsSet = false;

    sessionIdIsSet = false;
    numOfRtpInfoEntries = 0;
    for (int ii = 0; ii < RTSP_MAX_NUMBER_OF_RTP_INFO_ENTRIES; ++ii)
    {
        rtpInfo[ii].urlIsSet = false;
        rtpInfo[ii].seqIsSet = false;
        rtpInfo[ii].rtptimeIsSet = false;
    }

    contentType.setPtrLen("", 0);
    contentTypeIsSet = false;

    contentBase.setPtrLen("", 0);
    contentBaseMode = NO_CONTENT_BASE;

    contentLength = 0;
    contentLengthIsSet = false;

    userAgentIsSet = false;
    acceptIsSet = false;
    requireIsSet = false;

    originalURI = "";

#ifdef RTSP_PLAYLIST_SUPPORT
    methodEosIsSet = false;
    comPvServerPlaylistIsSet = false;
#endif

#ifdef ASF_STREAMING
    eofField.setPtrLen("", 0);
    eofFieldIsSet = false;
    XMLIsSet = false;
#endif
    timeout = 60;
}

OSCL_EXPORT_REF const StrPtrLen *
RTSPGenericMessage::queryField(const StrCSumPtrLen & query) const
{
    for (uint32 ii = 0; ii < numPtrFields; ++ii)
    {
        if (fieldKeys[ ii ].isCIEquivalentTo(query))
        {
            return fieldVals + ii;
        }
    }

    return NULL;
}
