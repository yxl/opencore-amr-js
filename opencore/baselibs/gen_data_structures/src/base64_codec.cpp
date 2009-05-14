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
#ifndef BASE64_CODEC_H_INCLUDED
#include "base64_codec.h"
#endif


OSCL_EXPORT_REF PVBase64Codec::PVBase64Codec()
{
    int32 i;
    //encoder table
    {
        for (i = 0;i < 9;i++)
        {
            etable[i] = OSCL_STATIC_CAST(uint8, 'A' + i);
            etable[i+9] = OSCL_STATIC_CAST(uint8, 'J' + i);
            etable[26+i] = OSCL_STATIC_CAST(uint8, 'a' + i);
            etable[26+i+9] = OSCL_STATIC_CAST(uint8, 'j' + i);
        }
        for (i = 0;i < 8;i++)
        {
            etable[i+18] = OSCL_STATIC_CAST(uint8, 'S' + i);
            etable[26+i+18] = OSCL_STATIC_CAST(uint8, 's' + i);
        }
        for (i = 0;i < 10;i++)
        {
            etable[52+i] = OSCL_STATIC_CAST(uint8, '0' + i);
        }
        etable[62] = '+';
        etable[63] = '/';
    }

    //deocder table
    for (i = 0;i < 255;i++)
    {
        dtable[i] = 0x80;
    }
    for (i = 'A';i <= 'I';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 0 + (i - 'A'));
    }
    for (i = 'J';i <= 'R';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 9 + (i - 'J'));
    }
    for (i = 'S';i <= 'Z';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 18 + (i - 'S'));
    }
    for (i = 'a';i <= 'i';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 26 + (i - 'a'));
    }
    for (i = 'j';i <= 'r';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 35 + (i - 'j'));
    }
    for (i = 's';i <= 'z';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 44 + (i - 's'));
    }
    for (i = '0';i <= '9';i++)
    {
        dtable[i] = OSCL_STATIC_CAST(uint8, 52 + (i - '0'));
    }
    dtable[(uint16)'+'] = 62;
    dtable[(uint16)'/'] = 63;
    dtable[(uint16)'='] = 0;
}

bool PVBase64Codec::Decode(uint8* aInBuf, uint32 aInBufLen,
                           uint8* aOutBuf, uint32& aOutBufLen, uint32 aMaxOutBufLen)
{
    oscl_memset(aOutBuf, 0, aMaxOutBufLen);
    aOutBufLen = 0;

    uint32 read_count = 0;
    uint32 write_count = 0;
    while (read_count < aInBufLen)
    {
        uint8 a[4], b[4], o[3];

        int i;
        for (i = 0;i < 4;i++)
        {
            uint8 c = *(aInBuf++);
            read_count++;

            if (read_count > aInBufLen)
            {
                //Input incomplete
                return false;
            }
            if (dtable[(uint16)c]&0x80)
            {
                //Illegal character in
                //return false;
                i--;
                continue;
            }
            a[i] = (uint8)c;
            b[i] = (uint8)dtable[(uint16)c];
        }
        o[0] = OSCL_STATIC_CAST(uint8, (b[0] << 2) | (b[1] >> 4));
        o[1] = OSCL_STATIC_CAST(uint8, (b[1] << 4) | (b[2] >> 2));
        o[2] = OSCL_STATIC_CAST(uint8, (b[2] << 6) | b[3]);
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

OSCL_EXPORT_REF bool PVBase64Codec::Encode(uint8* aInBuf, uint32 aInBufLen,
        uint8* aOutBuf, uint32& aOutBufLen, uint32 aMaxOutBufLen)
{
    int i;
    uint32 read_count = 0;
    uint32 write_count = 0;
    while (read_count < aInBufLen)
    {
        uint8 igroup[3], ogroup[4];
        igroup[0] = igroup[1] = igroup[2] = 0;
        int n;
        for (n = 0; n < 3; n++)
        {
            uint8 c = *(aInBuf++);
            read_count++;
            if (read_count > aInBufLen)
            {
                //Input incomplete
                //return false;
                break;
            }
            igroup[n] = (uint8) c;
        }
        if (n > 0)
        {
            ogroup[0] = etable [igroup[0] >> 2];
            ogroup[1] = etable [((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
            ogroup[2] = etable [((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
            ogroup[3] = etable [igroup[2] & 0x3F];
            /* Replace characters in output stream with ”=” pad
            characters if fewer than three characters were read from
            the end of the input stream. */
            if (n < 3)
            {
                ogroup[3] = '=';
                if (n < 2)
                {
                    ogroup[2] = '=';
                }
            }
            for (i = 0; i < 4; i++)
            {
                aOutBuf[write_count] = ogroup[i];
                write_count++;
                if (write_count > aMaxOutBufLen)
                {
                    return false;
                }
            }
        }
    }
    aOutBufLen = write_count;
    return true;
}


