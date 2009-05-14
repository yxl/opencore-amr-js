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
/*                                                                               */
/*********************************************************************************/

/*
**   File:   rtcp_decoder.h
**
**   Description:
**      This module defines the RTCP class. This class is used to encode and
**      decode RTCP packets. Please refer to the RTCP design document for
**      details.
*/

#ifndef RTCP_DECODER_H
#define RTCP_DECODER_H

/*
** Includes
*/
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_BIN_STREAM_H_INCLUDED
#include "oscl_bin_stream.h"
#endif
#ifndef RTCP_H
#include "rtcp.h"
#endif

/*
** Constants
*/

/*
** Classes
*/


/*
**      This class is used to encode and
**      decode RTCP packets. Please refer to the RTCP design document for
**      details.
*/
class RTCP_Decoder : public RTCP_Base
{
    public:

        typedef enum
        {
            RTCP_SUCCESS,
            FAIL,
            UNSUPPORTED_RTCP_VERSION,
            RTCP_PACKET_TYPE_MISMATCH,
            RTCP_LENGTH_MISMATCH,
            EOS_ON_READ,
            UNSUPPORTED_RTCP_PVSS_APP
        } Error_t;

        OSCL_IMPORT_REF   RTCP_Decoder(const uint8 version = DEFAULT_RTPRTCP_VERSION);
        OSCL_IMPORT_REF   ~RTCP_Decoder();


        /*
         * The scan_compound_packet routine will process the compound packet
         * and fill the array of packet types and MemoryFragments (up to
         * the array_size specified).  The caller provides the space for the
         * arrays that are filled by this routine.
         */
        OSCL_IMPORT_REF    Error_t scan_compound_packet(OsclMemoryFragment& input_packet,
                int32 max_array_size, int32& filled_size,
                RTCPPacketType *array_of_packet_types,
                OsclMemoryFragment *array_of_packets);

        OSCL_IMPORT_REF  Error_t DecodeRR(OsclMemoryFragment& input_packet,
                                          RTCP_RR& rr_packet);

        OSCL_IMPORT_REF    Error_t DecodeSR(const OsclMemoryFragment& input_packet,
                                            RTCP_SR& sr_packet);

        OSCL_IMPORT_REF Error_t DecodeSDES(const OsclMemoryFragment& input_packet,
                                           RTCP_SDES& sdes_packet);


        OSCL_IMPORT_REF    Error_t DecodeBYE(const OsclMemoryFragment& input_packet,
                                             RTCP_BYE& bye_packet);

        OSCL_IMPORT_REF  Error_t DecodeAPP(const OsclMemoryFragment& input_packet,
                                           RTCP_APP& bye_packet);


    private:

        Error_t DecodeReportBlock(/* Input stream reference */
            OsclBinIStreamBigEndian & inStream,
            RTCP_ReportBlock* report
        );

        Error_t DecodeSDESChunk(OsclBinIStreamBigEndian & inStream,
                                SDES_chunk* sdes_chunk,
                                OsclMemoryFragment& chunk_data);



};

#endif
