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

#include "oscl_string_utils.h"
#include "common_info.h"
#include "rtsp_range_utils.h"
#include "oscl_str_ptr_len.h"
#include "oscl_stdstring.h"
#include "sdp_parsing_utils.h"
#include "oscl_utf8conv.h"
#include "oscl_snprintf.h"
#include "oscl_bin_stream.h"

#define BYTE_ORDER_MASK 0xFEFF
static const char PVSDPMETADATA_LANG_CODE[] = ";iso-639-2-lang=";
static const char PVSDPMETADATA_RATING_ENTITY[] = ";rating-entity=";
static const char PVSDPMETADATA_RATING_CRITERIA[] = ";rating-criteria=";
static const char PVSDPMETADATA_CLASSIFICATION_ENTITY[] = ";classification-entity=";
static const char PVSDPMETADATA_CLASSFICATION_TABLE[] = ";classification-table=";

bool get_next_line(const char *start_ptr, const char * end_ptr,
                   const char *& line_start,
                   const char *& line_end)
{
    // Finds the boundaries of the next non-empty line within start
    // and end ptrs

    // This initializes line_start to the first non-whitespace character
    line_start = skip_whitespace_and_line_term(start_ptr, end_ptr);

    line_end = skip_to_line_term(line_start, end_ptr);

    return (line_start < end_ptr);

}

bool sdp_decodebase64(uint8* aInBuf, uint32 aInBufLen,
                      uint8* aOutBuf, uint32& aOutBufLen, uint32 aMaxOutBufLen)
{
    oscl_memset(aOutBuf, 0, aMaxOutBufLen);
    aOutBufLen = 0;

    int i;
    uint8 dtable[256];

    for (i = 0;i < 255;i++)
    {
        dtable[i] = 0x80;
    }
    for (i = 'A';i <= 'I';i++)
    {
        dtable[i] = 0 + (i - 'A');
    }
    for (i = 'J';i <= 'R';i++)
    {
        dtable[i] = 9 + (i - 'J');
    }
    for (i = 'S';i <= 'Z';i++)
    {
        dtable[i] = 18 + (i - 'S');
    }
    for (i = 'a';i <= 'i';i++)
    {
        dtable[i] = 26 + (i - 'a');
    }
    for (i = 'j';i <= 'r';i++)
    {
        dtable[i] = 35 + (i - 'j');
    }
    for (i = 's';i <= 'z';i++)
    {
        dtable[i] = 44 + (i - 's');
    }
    for (i = '0';i <= '9';i++)
    {
        dtable[i] = 52 + (i - '0');
    }
    dtable[(int)'+'] = 62;
    dtable[(int)'/'] = 63;
    dtable[(int)'='] = 0;

    uint32 read_count = 0;
    uint32 write_count = 0;
    while (read_count < aInBufLen)
    {
        uint8 a[4], b[4], o[3];

        for (i = 0;i < 4;i++)
        {
            uint8 c = *(aInBuf++);
            read_count++;

            if (read_count > aInBufLen)
            {
                //Input incomplete
                return false;
            }
            if (dtable[(int)c]&0x80)
            {
                //Illegal character in
                //return false;
                i--;
                continue;
            }
            a[i] = (uint8)c;
            b[i] = (uint8)dtable[(int)c];
        }
        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];
        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
        oscl_memcpy(aOutBuf, o, i);
        aOutBuf += i;
        write_count += i;
        if (write_count > aMaxOutBufLen)
        {
            return false;
        }
        if (i < 3)
        {
            break;
        }
    }
    aOutBufLen = write_count;
    return true;
}


bool parseQoEMetrics(const char *start_ptr, const char *end_ptr, QoEMetricsType &qoeMetrics)
{
    const char *sptr = start_ptr;
    const char *eptr = end_ptr;

    sptr = skip_whitespace_and_line_term(sptr, end_ptr);

    StrPtrLen rate("rate=");
    StrPtrLen range("range:");

    if (!oscl_strncmp(sptr, "{", 1))
        sptr = sptr + 1;
    else
        return false;

    sptr = skip_whitespace_and_line_term(sptr, end_ptr);
    if (sptr > eptr)
        return false;

    while (sptr < end_ptr)
    {

        if (!oscl_strncmp(sptr, "Initial_Buffering_Duration",
                          oscl_strlen("Initial_Buffering_Duration")))
        {
            qoeMetrics.name[QoEMetricsType::INITIAL_BUFFERING_DURATION] = true;
            sptr = sptr + oscl_strlen("Initial_Buffering_Duration");
        }
        else if (!oscl_strncmp(sptr, "Rebuffering_Duration",
                               oscl_strlen("Rebuffering_Duration")))
        {
            qoeMetrics.name[QoEMetricsType::REBUFFERING_DURATION] = true;
            sptr = sptr + oscl_strlen("Rebuffering_Duration");
        }
        else if (!oscl_strncmp(sptr, "Corruption_Duration",
                               oscl_strlen("Corruption_Duration")))
        {
            qoeMetrics.name[QoEMetricsType::CORRUPTION_DURATION] = true;
            sptr = sptr + oscl_strlen("Corruption_Duration");
        }
        else if (!oscl_strncmp(sptr, "Succssive_Loss",
                               oscl_strlen("Succssive_Loss")))
        {
            qoeMetrics.name[QoEMetricsType::SUCESSIVE_LOSS] = true;
            sptr = sptr + oscl_strlen("Succssive_Loss");
        }
        else if (!oscl_strncmp(sptr, "Framerate_Deviation",
                               oscl_strlen("Framerate_Deviation")))
        {
            qoeMetrics.name[QoEMetricsType::FRAMERATE_DEVIATION] = true;
            sptr = sptr + oscl_strlen("Framerate_Deviation");
        }
        else if (!oscl_strncmp(sptr, "Jitter_Duration",
                               oscl_strlen("Jitter_Duration")))
        {
            qoeMetrics.name[QoEMetricsType::JITTER_DURATION] = true;
            sptr = sptr + oscl_strlen("Jitter_Duration");
        }
        else if (!oscl_strncmp(sptr, "Decoded_Bytes",
                               oscl_strlen("Decoded_Bytes")))
        {
            qoeMetrics.name[QoEMetricsType::DECODED_BYTES] = true;
            sptr = sptr + oscl_strlen("Decoded_Bytes");
        }
        else
            return false;

        sptr = skip_whitespace_and_line_term(sptr, end_ptr);
        if (sptr > end_ptr)
            return false;

        if (!oscl_strncmp(sptr, ",", 1))
            sptr = sptr + 1;
        else if (!oscl_strncmp(sptr, "}", 1))
        {
            sptr = sptr + 1;
            break;
        }
    }

    if (sptr > end_ptr)
        return false;

    if (!oscl_strncmp(sptr, ";", 1))
        sptr = sptr + 1;
    else
        return false;

    if (!oscl_strncmp(sptr, rate.c_str(), rate.length()))
    {
        sptr = sptr + rate.length();
        if (!oscl_strncmp(sptr, "End", oscl_strlen("End")))
        {
            qoeMetrics.rateFmt = QoEMetricsType::END;
            qoeMetrics.rateEnd = 'E';
            sptr = sptr + oscl_strlen("End");

        }
        else
        {
            uint32 temp;
            eptr = sptr;		//get length of range digit
            for (; (*eptr != ';' && eptr < end_ptr); ++eptr);

            qoeMetrics.rateFmt = QoEMetricsType::VAL;

            if (PV_atoi(sptr, 'd', (int)(eptr  - sptr), temp))
            {
                qoeMetrics.rateVal = temp;
                sptr = eptr ;
            }
            else
                return false;
        }

    }
    else
        return false;

    if (sptr == end_ptr) //end of line reached.
        return true;

    if (sptr > end_ptr)
        return false;

    if (!oscl_strncmp(sptr, ";", 1))
        sptr = sptr + 1;
    else
        return false;
    if (!oscl_strncmp(sptr, range.c_str(), range.length()))
    {
        eptr = sptr + range.length();
        for (; *eptr != ';' && eptr != end_ptr ; eptr++); // get length of range.

        if (!parseRtspRange((sptr + range.length()), (eptr - sptr - range.length()),
                            qoeMetrics.range))
            return false;
    }

    sptr = eptr;
    if (sptr == end_ptr)
        return true;    // end of line reached.
    else  //Parameter_Ext
    {
        sptr = eptr + 1;
        if (!oscl_strncmp(sptr, "On", oscl_strlen("On")))
        {
            qoeMetrics.paramExtStat = true;
            qoeMetrics.paramFmt = QoEMetricsType::STATUS;
        }
        else if (!oscl_strncmp(sptr, "Off", oscl_strlen("Off")))
        {
            qoeMetrics.paramExtStat = false;
            qoeMetrics.paramFmt = QoEMetricsType::STATUS;
        }
        else
        {
            uint32 len = OSCL_MIN((uint32)(eptr - sptr), oscl_strlen("."));
            if (oscl_strncmp(sptr, ".", len) == 0)	//if floating point number
            {
                if (!PV_atof(sptr, (int)(eptr - sptr), qoeMetrics.paramExtFdigit))
                    return false;
                qoeMetrics.paramFmt = QoEMetricsType::FDIGIT;
            }
            else	    		// hex digit
            {
                uint32 temp;
                if (PV_atoi(sptr, 'x', (int)(eptr - sptr), temp))
                {
                    qoeMetrics.paramExtIdigit = temp;
                    qoeMetrics.paramFmt = QoEMetricsType::IDIGIT;

                }
                else
                    return false;
            }

        }


    }

    return true;

}

void pvSDPParserGetAssetInfoLanguageCode(uint16 langcode, char* LangCode)
{
    //ISO-639-2/T 3-char Lang Code
    oscl_memset(LangCode, 0, 4);
    LangCode[0] = 0x60 + ((langcode >> 10) & 0x1F);
    LangCode[1] = 0x60 + ((langcode >> 5) & 0x1F);
    LangCode[2] = 0x60 + ((langcode) & 0x1F);
}

bool pvSDPParserParse3GPPAssetInfoLocation(AssetInfoType& ainfo,
        uint8* aBuf,
        uint32 aBufSize)
{
    uint32 count = 0;
    uint16 lang = *aBuf++;
    lang = ((lang << 8) | (*aBuf++));
    count += 2;
    uint16 byteOrderMask = 0;
    byteOrderMask = *aBuf;
    byteOrderMask =
        ((byteOrderMask << 8) | (*(aBuf + 1)));
    if (byteOrderMask == BYTE_ORDER_MASK)
    {
        aBuf += 2;
        uint32 index = 0;
        uint8 firstbyte = aBuf[index];
        uint8 secondbyte = aBuf[index++];
        oscl_wchar wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
        bool nextChar = (wc == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            firstbyte = aBuf[index];
            secondbyte = aBuf[index++];
            wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
            nextChar = (wc == 0) ? false : true;
        }
        ainfo.iLocationStruct._location_name =
            (oscl_wchar*)(oscl_malloc(sizeof(oscl_wchar) * (index + 1)));
        if (ainfo.iLocationStruct._location_name == NULL) return false;
        oscl_memcpy(ainfo.iLocationStruct._location_name, aBuf, (index*2));
    }
    else
    {
        uint32 index = 0;
        bool nextChar = (aBuf[index] == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            index++;
            nextChar = (aBuf[index] == 0) ? false : true;
        }
        oscl_wchar* unicodeBuf = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * (index + 1));
        if (unicodeBuf == NULL) return false;
        oscl_UTF8ToUnicode((char*)aBuf,
                           index,
                           unicodeBuf,
                           index + 1);
        ainfo.iLocationStruct._location_name = unicodeBuf;
    }
    ainfo.iLocationStruct._role = *aBuf++;
    uint32 i;
    ainfo.iLocationStruct._longitude = 0;
    for (i = 0; i < 4; i++)
    {
        ainfo.iLocationStruct._longitude |= *aBuf++;
    }
    ainfo.iLocationStruct._latitude = 0;
    for (i = 0; i < 4; i++)
    {
        ainfo.iLocationStruct._latitude |= *aBuf++;
    }
    ainfo.iLocationStruct._altitude = 0;
    for (i = 0; i < 4; i++)
    {
        ainfo.iLocationStruct._altitude |= *aBuf++;
    }
    byteOrderMask = *aBuf;
    byteOrderMask =
        ((byteOrderMask << 8) | (*(aBuf + 1)));
    if (byteOrderMask == BYTE_ORDER_MASK)
    {
        aBuf += 2;
        uint32 index = 0;
        uint8 firstbyte = aBuf[index];
        uint8 secondbyte = aBuf[index++];
        oscl_wchar wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
        bool nextChar = (wc == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            firstbyte = aBuf[index];
            secondbyte = aBuf[index++];
            wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
            nextChar = (wc == 0) ? false : true;
        }
        ainfo.iLocationStruct._astronomical_body =
            (oscl_wchar*)(oscl_malloc(sizeof(oscl_wchar) * (index + 1)));
        if (ainfo.iLocationStruct._astronomical_body == NULL) return false;
        oscl_memcpy(ainfo.iLocationStruct._astronomical_body, aBuf, (index*2));
    }
    else
    {
        uint32 index = 0;
        bool nextChar = (aBuf[index] == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            index++;
            nextChar = (aBuf[index] == 0) ? false : true;
        }
        oscl_wchar* unicodeBuf = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * (index + 1));
        if (unicodeBuf == NULL) return false;
        oscl_UTF8ToUnicode((char*)aBuf,
                           index,
                           unicodeBuf,
                           index + 1);
        ainfo.iLocationStruct._astronomical_body = unicodeBuf;
    }
    byteOrderMask = *aBuf;
    byteOrderMask =
        ((byteOrderMask << 8) | (*(aBuf + 1)));
    if (byteOrderMask == BYTE_ORDER_MASK)
    {
        aBuf += 2;
        uint32 index = 0;
        uint8 firstbyte = aBuf[index];
        uint8 secondbyte = aBuf[index++];
        oscl_wchar wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
        bool nextChar = (wc == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            firstbyte = aBuf[index];
            secondbyte = aBuf[index++];
            wc = (uint16)(firstbyte << 8 | (uint16) secondbyte);
            nextChar = (wc == 0) ? false : true;
        }
        ainfo.iLocationStruct._additional_notes =
            (oscl_wchar*)(oscl_malloc(sizeof(oscl_wchar) * index));
        if (ainfo.iLocationStruct._additional_notes == NULL) return false;
        oscl_memcpy(ainfo.iLocationStruct._additional_notes, aBuf, (index*2));
    }
    else
    {
        uint32 index = 0;
        bool nextChar = (aBuf[index] == 0) ? false : true;
        while (nextChar && (index < aBufSize))
        {
            index++;
            nextChar = (aBuf[index] == 0) ? false : true;
        }
        oscl_wchar* unicodeBuf = (oscl_wchar*)oscl_malloc(sizeof(oscl_wchar) * (index + 1));
        if (unicodeBuf == NULL) return false;
        oscl_UTF8ToUnicode((char*)aBuf,
                           index,
                           unicodeBuf,
                           index + 1);
        ainfo.iLocationStruct._additional_notes = unicodeBuf;
    }
    return true;
}


bool parseAssetInfo(const char *sptr, const char *line_end_ptr, AssetInfoType &ainfo)
{
    const char *eptr = sptr;
    int assetbox;

    while (eptr < line_end_ptr)
    {
        sptr = skip_whitespace(sptr, line_end_ptr);
        if (oscl_CIstrncmp(sptr, "{", 1))
            return false;
        sptr = sptr + 1;
        sptr = skip_whitespace(sptr, line_end_ptr);
        if (sptr > line_end_ptr)
            return false;

        if (!oscl_CIstrncmp(sptr, "url=", oscl_strlen("url=")))
        {
            sptr = sptr + oscl_strlen("url=");
            sptr = skip_whitespace(sptr, line_end_ptr);
            if (sptr > line_end_ptr)
                return false;
            if (!oscl_CIstrncmp(sptr, "\"", 1))
            {
                sptr = sptr + 1;
                sptr = skip_whitespace(sptr, line_end_ptr);
                if (sptr > line_end_ptr)
                    return false;
                eptr = sptr;

                for (; *eptr != '"'; ++eptr);

                ainfo.URL.set((const char *)sptr, (eptr - sptr));
            }
            else
            {
                eptr = sptr;

                for (; *eptr != '}'; ++eptr);

                ainfo.URL.set((const char *)sptr, (eptr - sptr));
            }
            return true;

        }

        if (!oscl_CIstrncmp(sptr, "Title=", oscl_strlen("Title=")))
        {
            sptr = sptr + oscl_strlen("Title=");
            assetbox = (int) AssetInfoType::TITLE;
            ainfo.oTitlePresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Description=", oscl_strlen("Description=")))
        {
            sptr = sptr + oscl_strlen("Description=");
            assetbox = (int) AssetInfoType::DESCRIPTION;
            ainfo.oDescriptionPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Copyright=", oscl_strlen("Copyright=")))
        {
            sptr = sptr + oscl_strlen("Copyright=");
            assetbox = (int) AssetInfoType::COPYRIGHT;
            ainfo.oCopyRightPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Performer=", oscl_strlen("Performer=")))
        {
            sptr = sptr + oscl_strlen("Performer=");
            assetbox = (int) AssetInfoType::PERFORMER;
            ainfo.oPerformerPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Author=", oscl_strlen("Author=")))
        {
            sptr = sptr + oscl_strlen("Author=");
            assetbox = (int) AssetInfoType::AUTHOR;
            ainfo.oAuthorPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Genre=", oscl_strlen("Genre=")))
        {
            sptr = sptr + oscl_strlen("Genre=");
            assetbox = (int) AssetInfoType::GENRE;
            ainfo.oGenrePresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Rating=", oscl_strlen("Rating=")))
        {
            sptr = sptr + oscl_strlen("Rating=");
            assetbox = (int) AssetInfoType::RATING;
            ainfo.oRatingPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Classification=", oscl_strlen("Classification=")))
        {
            sptr = sptr + oscl_strlen("Classification=");
            assetbox = (int) AssetInfoType::CLASSIFICATION;
            ainfo.oClassificationPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Keywords=", oscl_strlen("Keywords=")))
        {
            sptr = sptr + oscl_strlen("Keywords=");
            assetbox = (int) AssetInfoType::KEYWORDS;
            ainfo.oKeyWordsPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Location=", oscl_strlen("Location=")))
        {
            sptr = sptr + oscl_strlen("Location=");
            assetbox = (int) AssetInfoType::LOCATION;
            ainfo.oLocationPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "Album=", oscl_strlen("Album=")))
        {
            sptr = sptr + oscl_strlen("Album=");
            assetbox = (int) AssetInfoType::ALBUM;
            ainfo.oAlbumPresent = true;
        }
        else if (!oscl_CIstrncmp(sptr, "RecordingYear=", oscl_strlen("RecordingYear=")))
        {
            sptr = sptr + oscl_strlen("RecordingYear=");
            assetbox = (int) AssetInfoType::RECORDINGYEAR;
            ainfo.oRecordingYearPresent = true;
        }
        else// if(!oscl_CIstrncmp(sptr, "asset-extention=", oscl_strlen("asset-extention=")))
        {//asset-extension ignore for now
            //sptr = sptr + oscl_strlen("asset-extention=");
            assetbox = (int) AssetInfoType::ASSET_EXTENTION;
            ainfo.oAssetExtensionPresent = true;
        }

        sptr = skip_whitespace(sptr, line_end_ptr);
        if (sptr > line_end_ptr)
            return false;

        for (eptr = sptr; *eptr != '}'; ++eptr)
        {
            if (eptr > line_end_ptr)
                return false;
        }

        {
            //(here sptr is the complete value of base64 encoded metadata)
            uint8* inBuf = (uint8*)sptr;
            //(length of base64 encode metadata value)
            uint32 inBufLen = (eptr - sptr);
            //(string to collect the base64 decoded data)
            uint8* outBuf = (uint8*)oscl_malloc(inBufLen);
            if (outBuf != NULL)
            {
                //(max length of the base64 decoded data)
                uint32 maxoutBuflen = inBufLen;
                uint32 outBuflen = 0;
                sdp_decodebase64(inBuf, inBufLen, outBuf, outBuflen, maxoutBuflen);
                //4 bytes size, 4 bytes fourcc, 4 bytes flags
                uint32 assetinfoatomoffset = 12;
                if (outBuflen > assetinfoatomoffset)
                {
                    uint8* buf = outBuf + assetinfoatomoffset;
                    if ((assetbox == AssetInfoType::TITLE) ||
                            (assetbox == AssetInfoType::DESCRIPTION) ||
                            (assetbox == AssetInfoType::COPYRIGHT) ||
                            (assetbox == AssetInfoType::PERFORMER) ||
                            (assetbox == AssetInfoType::AUTHOR) ||
                            (assetbox == AssetInfoType::GENRE) ||
                            (assetbox == AssetInfoType::ALBUM) ||
                            (assetbox == AssetInfoType::RATING) ||
                            (assetbox == AssetInfoType::CLASSIFICATION))
                    {
                        char rating_entity[21];
                        char rating_criteria[21];
                        if (assetbox == AssetInfoType::RATING)
                        {
                            oscl_snprintf(rating_entity,
                                          oscl_strlen(PVSDPMETADATA_RATING_ENTITY) + 4,
                                          "%s%s",
                                          PVSDPMETADATA_RATING_ENTITY, buf);
                            buf += 4;
                            rating_entity[20] = '\0';
                            oscl_snprintf(rating_criteria,
                                          oscl_strlen(PVSDPMETADATA_RATING_CRITERIA) + 4,
                                          "%s%s",
                                          PVSDPMETADATA_RATING_CRITERIA, buf);
                            buf += 4;
                            rating_criteria[20] = '\0';
                        }
                        char classification_entity[26];
                        char classification_table[26];
                        if (assetbox == AssetInfoType::CLASSIFICATION)
                        {
                            oscl_snprintf(classification_entity,
                                          oscl_strlen(PVSDPMETADATA_CLASSIFICATION_ENTITY) + 4,
                                          "%s%s",
                                          PVSDPMETADATA_CLASSIFICATION_ENTITY, buf);
                            buf += 4;
                            classification_entity[25] = '\0';
                            oscl_snprintf(classification_table,
                                          oscl_strlen(PVSDPMETADATA_CLASSFICATION_TABLE) + 2,
                                          "%s%s",
                                          PVSDPMETADATA_CLASSFICATION_TABLE, buf);
                            buf += 2;
                            classification_table[25] = '\0';
                        }
                        uint16 lang = *buf++;
                        lang = ((lang << 8) | (*buf++));
                        char LangCode[4];
                        pvSDPParserGetAssetInfoLanguageCode(lang, LangCode);
                        char lang_param[21];
                        oscl_snprintf(lang_param,
                                      oscl_strlen(PVSDPMETADATA_LANG_CODE) + 4,
                                      "%s%s", PVSDPMETADATA_LANG_CODE, LangCode);
                        lang_param[20] = '\0';
                        uint16 byteOrderMask = *buf;
                        byteOrderMask =
                            ((byteOrderMask << 8) | (*(buf + 1)));
                        if (byteOrderMask == BYTE_ORDER_MASK)
                        {
                            buf += 2;
                            OsclBinIStreamBigEndian datastream;
                            datastream.Attach((void *)(buf), (outBuflen - assetinfoatomoffset - 2));
                            //(string to collect the utf8 data)
                            uint8* utf8Buf = (uint8*)oscl_malloc(inBufLen);
                            oscl_memset(utf8Buf, 0, inBufLen);
                            int32 unicodebuflen = (int32)((outBuflen - assetinfoatomoffset - 2) / 2);
                            oscl_wchar* unicodeBuf = (oscl_wchar*)oscl_malloc((unicodebuflen + 1) * sizeof(oscl_wchar));
                            oscl_memset(unicodeBuf, 0, (unicodebuflen + 1)*sizeof(oscl_wchar));
                            for (int32 i = 0; i < unicodebuflen; i++)
                            {
                                uint16 temp = 0;
                                datastream >> temp;
                                unicodeBuf[i] = (uint16)(temp);
                            }
                            oscl_UnicodeToUTF8(unicodeBuf,
                                               unicodebuflen,
                                               (char*)utf8Buf,
                                               inBufLen);
                            ainfo.Box[assetbox].set((const char *)(utf8Buf),
                                                    oscl_strlen((const char*)utf8Buf));
                            oscl_free(utf8Buf);
                            oscl_free(unicodeBuf);
                        }
                        else
                        {
                            ainfo.Box[assetbox].set((const char *)(buf),
                                                    (outBuflen - assetinfoatomoffset));
                        }
                        ainfo.Box[assetbox] += lang_param;
                        if (assetbox == AssetInfoType::RATING)
                        {
                            ainfo.Box[assetbox] += rating_entity;
                            ainfo.Box[assetbox] += rating_criteria;
                        }
                        else if (assetbox == AssetInfoType::CLASSIFICATION)
                        {
                            ainfo.Box[assetbox] += classification_entity;
                            ainfo.Box[assetbox] += classification_table;
                        }
                    }
                    else if (assetbox == AssetInfoType::RECORDINGYEAR)
                    {
                        ainfo.iRecordingYear = 0;
                        ainfo.iRecordingYear = *buf++;
                        ainfo.iRecordingYear = ((ainfo.iRecordingYear << 8) | (*buf++));
                    }
                    else if (assetbox == AssetInfoType::KEYWORDS)
                    {
                        uint16 lang = *buf++;
                        lang = ((lang << 8) | (*buf++));
                        char LangCode[4];
                        pvSDPParserGetAssetInfoLanguageCode(lang, LangCode);
                        char lang_param[21];
                        oscl_snprintf(lang_param,
                                      oscl_strlen(PVSDPMETADATA_LANG_CODE) + 4,
                                      "%s%s", PVSDPMETADATA_LANG_CODE, LangCode);
                        lang_param[20] = '\0';
                        ainfo.iNumKeyWords = (uint32)(*buf++);
                        for (uint32 i = 0; i < ainfo.iNumKeyWords; i++)
                        {
                            OSCL_HeapString<OsclMemAllocator> keyWordString;
                            uint32 keywordsize = (uint32)(*buf++);
                            uint16 byteOrderMask = *buf;
                            byteOrderMask =
                                ((byteOrderMask << 8) | (*(buf + 1)));
                            if (byteOrderMask == BYTE_ORDER_MASK)
                            {
                                buf += 2;
                                keywordsize -= 2;
                                OsclBinIStreamBigEndian datastream;
                                datastream.Attach((void *)(buf), keywordsize);
                                //(string to collect the utf8 data)
                                uint8* utf8Buf = (uint8*)oscl_malloc(keywordsize);
                                oscl_memset(utf8Buf, 0, keywordsize);
                                int32 unicodebuflen = (int32)((keywordsize) / 2);
                                oscl_wchar* unicodeBuf = (oscl_wchar*)oscl_malloc((unicodebuflen + 1) * sizeof(oscl_wchar));
                                oscl_memset(unicodeBuf, 0, (unicodebuflen + 1)*sizeof(oscl_wchar));
                                for (int32 i = 0; i < unicodebuflen; i++)
                                {
                                    uint16 temp = 0;
                                    datastream >> temp;
                                    unicodeBuf[i] = (uint16)(temp);
                                }
                                oscl_UnicodeToUTF8(unicodeBuf,
                                                   unicodebuflen,
                                                   (char*)utf8Buf,
                                                   inBufLen);
                                keyWordString.set((const char *)(utf8Buf),
                                                  oscl_strlen((const char*)utf8Buf));
                                oscl_free(utf8Buf);
                                oscl_free(unicodeBuf);
                                buf += keywordsize;
                            }
                            else
                            {
                                keyWordString.set((const char *)(buf), keywordsize);
                                buf += keywordsize;
                            }
                            keyWordString += lang_param;
                            ainfo.KeyWords[i] = keyWordString;
                        }
                    }
                    else if (assetbox == AssetInfoType::LOCATION)
                    {
                        if (pvSDPParserParse3GPPAssetInfoLocation(ainfo, buf, outBuflen) != true)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        ainfo.Box[assetbox].set((const char *)(buf),
                                                (outBuflen - assetinfoatomoffset));
                    }
                }
                oscl_free(outBuf);
            }
        }
        eptr = eptr + 1;
        sptr = eptr + 1;
    }

    return true;
}


