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

#ifndef RTSP_PAR_COM_MESSAGE_DS_H_
#define RTSP_PAR_COM_MESSAGE_DS_H_

#include "oscl_base.h"
#include "rtsp_par_com_tunable.h"
#include "rtsp_par_com_basic_ds.h"
#include "rtsp_transport.h"
#include "rtsp_time_formats.h"

class RTSPGenericMessage
{
    protected:

        // for internal manipulation
        //
        char          secondaryBuffer[ RTSP_MAX_FULL_REQUEST_SIZE +1 ];
        char *        secondaryBufferSpace;
        int           secondaryBufferSizeUsed;

        StrCSumPtrLen	fieldKeys[ RTSP_MAX_NUMBER_OF_FIELDS ];
        StrPtrLen 		fieldVals[ RTSP_MAX_NUMBER_OF_FIELDS ];
        uint16						numPtrFields;

    public:

        RTSPMsgType		  msgType;

        RTSPMethod		  method;	// for requests
        RTSPStatusCode	statusCode; // for responses

        StrPtrLen	  methodString;
        StrPtrLen	  reasonString;

        uint32			    cseq;
        bool			      cseqIsSet;

        uint32          bufferSize;
        bool            bufferSizeIsSet;

        uint32			    authorizationFlag;
        bool		      	authorizationFlagIsSet;

        //		RTSPTransport	  transport;
        RtspTransport	  transport[RTSP_MAX_NUMBER_OF_TRANSPORT_ENTRIES];
        uint32          numOfTransportEntries;


        //		RTSPRange		    range;
        RtspRangeType		range;
        bool			      rangeIsSet;

        RTSPSessionId	  sessionId;
        bool			      sessionIdIsSet;


        RTSPRTPInfo     rtpInfo[ RTSP_MAX_NUMBER_OF_RTP_INFO_ENTRIES ];
        uint32          numOfRtpInfoEntries;


        // **************************************
        // items related to an entity body
        // to be filled out by Parser
        //

        StrPtrLen	contentType;
        bool			    contentTypeIsSet;

        StrPtrLen	contentBase;
        enum ContentBaseModeType {NO_CONTENT_BASE = 0, CONTENT_BASE_SET, INCLUDE_TRAILING_SLASH};
        ContentBaseModeType			    contentBaseMode;

        // for Content-Length
        uint32		    contentLength;
        bool			    contentLengthIsSet;

        // some more recognized fields
        StrPtrLen  userAgent;
        bool          userAgentIsSet;

        StrPtrLen  accept;
        bool          acceptIsSet;

        StrPtrLen  require;
        bool          requireIsSet;

        // original requested URI
        StrPtrLen  originalURI;
#ifdef RTSP_PLAYLIST_SUPPORT
        bool methodEosIsSet;
        bool comPvServerPlaylistIsSet;
#endif

        StrPtrLen			    eofField;
        bool			      eofFieldIsSet;
        char XMLBufferPtr[ RTSP_MAX_FULL_REQUEST_SIZE];
        bool XMLIsSet;

        uint32	timeout;
    public:

        OSCL_IMPORT_REF void          reset();

        OSCL_IMPORT_REF const StrPtrLen *	queryField(const StrCSumPtrLen & query) const;
};


class RTSPIncomingMessage
            : public RTSPGenericMessage
{
    protected:

        RTSPStructureStatus amMalformed;

        uint32					totalFields;
        uint32					totalFieldsParsed;
        void					parseRange(char *rangeString, RtspRangeType *range);

#ifdef RTSP_PLAYLIST_SUPPORT
        void 		  parseSupported(const char *supportedString, int length);
#endif /* RTSP_PLAYLIST_SUPPORT */

        void          parseTransport(uint16);
        void          parseOneTransportEntry(char * &, char *);

        void          parseRTPInfo(uint16);
        void          parseOneRTPInfoEntry(char * &, char *);

    public:


        // original requested URI without the last part
        StrPtrLen	  originalURIBase;

        // the control portion of the original URI
        StrPtrLen	  originalURIControlCandidate;

        // for remapped URI
        StrPtrLen	  remappedURI;

#ifdef RTSP_PLAYLIST_SUPPORT
        StrPtrLen playlistRangeField;
        bool playlistRangeFieldIsSet;
        // need to figure out how to store all the error fields.. we may not actually need to
        StrPtrLen playlistErrorField[RTSP_MAX_NUMBER_OF_PLAYLIST_ERROR_ENTRIES];
        bool playlistErrorFieldIsSet;
        int32 playlistErrorFieldCount;
        StrPtrLen     supportedField[ RTSP_MAX_NUMBER_OF_SUPPORTED_ENTRIES ];
        uint32          numOfSupportedEntries;
        bool supportedFieldIsSet;

        StrPtrLen playlistRangeUrl;
        int32 playlistRangeClipIndex;
        int32 playlistRangeClipOffset;
        int32 playlistRangeNptTime;
#endif
        // *************************************
        // general API
        //


        StrCSumPtrLen	rtspVersionString;

        OSCL_IMPORT_REF void            reset();
        OSCL_IMPORT_REF 	RTSPStructureStatus isMalformed()
        {
            return amMalformed;
        }

    protected:

        void parseFirstFields();


    public:

        RTSPIncomingMessage()
        {
            reset();
        }

        OSCL_IMPORT_REF bool
        parseEntityBody(RTSPEntityBody * entityBody);

        // *************************************
        // for handling requests with too many
        // fields
        //
        uint32 getTotalFields()
        {
            return totalFields;
        }
        uint32 getTotalFieldsParsed()
        {
            return totalFieldsParsed;
        }
        // field to save channel id of the RTP data
        uint8 channelID;

    private:
        bool	 parseNextPortion();


    public:

        friend class RTSPParser;
};


class RTSPOutgoingMessage
            : public RTSPGenericMessage
{
    protected:

        RTSPIncomingMessage * boundMessage;

        StrPtrLen  fullRequestPLS;
        void	   composeTransport(char* trans, RtspTransport* rtspTrans);
    public:


        // null-terminated buffer with the original
        // RTSP request
        char	        fullRequestBuffer[ RTSP_MAX_FULL_REQUEST_SIZE +1 ];
        char *        fullRequestBufferSpace;
        int						fullRequestBufferSizeUsed;


        char      rtpInfoUrlBuffer[ RTSP_MAX_NUMBER_OF_RTP_INFO_ENTRIES ][200+1];


        RTSPOutgoingMessage()
        {
            reset();
        }

        // ****************************************
        // for binding to RTSPIncomingMessage
        //
        OSCL_IMPORT_REF void    bind(const RTSPIncomingMessage & theMessage);

        // ****************************************
        // for adding generic fields
        //
        OSCL_IMPORT_REF bool
        addField(StrCSumPtrLen * newFieldName,
                 const StrPtrLen * newFieldValue
                );
        OSCL_IMPORT_REF bool
        addField(StrCSumPtrLen * newFieldName,
                 const char * newValue
                );

        // ****************************************
        // for requesting composition
        //
        OSCL_IMPORT_REF bool
        compose();

        // ***************************************
        // for retrieving composed buffer
        //
        OSCL_IMPORT_REF StrPtrLen *
        retrieveComposedBuffer();

        void
        OSCL_IMPORT_REF reset();
};

#endif // RTSP_PAR_COM_MESSAGE_DS_H_
