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

#ifndef SDP_PARSING_UTILS_H
#define SDP_PARSING_UTILS_H


bool get_next_line(const char *start_ptr, const char * end_ptr,
                   const char *& line_start,
                   const char *& line_end);

bool parseQoEMetrics(const char *start_ptr,
                     const char *end_ptr,
                     QoEMetricsType &qoeMetrics);
bool parseAssetInfo(const char *sptr, const char *line_end_ptr,
                    AssetInfoType &ainfo);

bool sdp_decodebase64(uint8* aInBuf, uint32 aInBufLen,
                      uint8* aOutBuf, uint32& aOutBufLen, uint32 aMaxOutBufLen);

void pvSDPParserGetAssetInfoLanguageCode(uint16 langcode, char* LangCode);

bool pvSDPParserParse3GPPAssetInfoLocation(AssetInfoType& aInfo,
        uint8* aBuf,
        uint32 aBufSize);

#endif


