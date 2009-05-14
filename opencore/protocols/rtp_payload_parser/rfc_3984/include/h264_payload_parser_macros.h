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
#ifndef H264_PAYLOAD_PARSER_MACROS_H
#define H264_PAYLOAD_PARSER_MACROS_H


/////////////////////////////////////////////////////////////////////////////////
///////////////////// macros defintion //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// NAL type
#define H264_RTP_PAYLOAD_SPS		7
#define H264_RTP_PAYLOAD_PPS		8

#define H264_RTP_PAYLOAD_STAP_A		24
#define H264_RTP_PAYLOAD_STAP_B		25
#define H264_RTP_PAYLOAD_MTAP16		26
#define H264_RTP_PAYLOAD_MTAP24		27
#define H264_RTP_PAYLOAD_FU_A		28
#define H264_RTP_PAYLOAD_FU_B		29

// Bit mask
#define NAL_TYPE_BIT_MASK			0x1f
#define NAL_F_NRI_BIT_MASK			0xe0
#define FU_S_BIT_MASK				0x80
#define FU_E_BIT_MASK				0x40
#define BIT_SHIFT_FOR_FU_COUNTER	11


// others
#define H264_RTP_PAYLOAD_MIN_TYPE_NUM	 4
#define H264_RTP_PAYLOAD_MEMFRAG_POOLNUM 32			// this macro can be tunable
#define H264_RTP_PAYLOAD_DEFAULT_INTERLEAVE_DEPTH 8 // this macro can be tunable



#endif // H264_PAYLOAD_PARSER_MACROS_H

