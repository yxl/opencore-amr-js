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
/*
 * File:	crccheck.cpp
 * Author:	Russell Hayashida (rhayashida@packetvideo.com)
 * Date:	February 13, 2002
 * Comment:	CRC procedures
 */

#include "crccheck_cpp.h"
#include <limits.h>

#define CRC_X8 0xe0		/* X8+X2+X+1( 0x107 <-> 0xe0 ) */
#define CRC_X 0x8408U	/* X16+X12+X5+1 ( 0x11021 <-> 0x8408 )  */


//Initialize both tables.
OSCL_EXPORT_REF CRC::CRC()
{
    int i;
    uint8 j, Crc;

    for (i = 0 ; i <= (int) UCHAR_MAX ; i ++)
    {
        Crc = (uint8) i;
        for (j =  0 ; j < CHAR_BIT ; j ++)
        {
            if (Crc & 1)
                Crc = (uint8)((Crc >> 1) ^ CRC_X8);	       /* Generator Polynomial		    */
            else
                Crc >>= 1;
        }
        CrcTbl8[i] = Crc;
    }


    uint16	n, m, Crc16;

    for (n = 0 ; n <= UCHAR_MAX ; n++)
    {
        Crc16 = n;
        for (m =  0 ; m < CHAR_BIT ; m++)
        {
            if (Crc16 & 1)
                Crc16 = (uint16)((Crc16 >> 1) ^ CRC_X);		       /* Genarator Polynomial		    */
            else
                Crc16 >>= 1;
        }
        CrcTbl16[ n ] = Crc16;		    /* CRC Infomaiton Set		    */
    }
}


OSCL_EXPORT_REF uint8 CRC::Crc8Check(uint8 *pAlPdu, int16 Size)
{
    uint8	crc;

    crc = 0;
    while (--Size >= 0)
    {
        crc = (uint8)((crc >> CHAR_BIT) ^ CrcTbl8[ crc ^ *pAlPdu++ ]);
    }
    return (uint8)(crc & 0xffU);
}

OSCL_EXPORT_REF uint8 CRC::Crc8Check(Packet *pPkt)
{
    uint8	crc;
    uint8	*pData = NULL;
    int32	fragIdx, curSize, dataSize;
    BufferFragment* frag = NULL;

    crc = 0;
    fragIdx = 0;
    dataSize = pPkt->GetMediaSize();
    curSize = 0;
    while (--dataSize >= 0)
    {

        if ((--curSize) <= 0)
        {
            frag = pPkt->GetMediaFragment(fragIdx);
            fragIdx++;
            curSize = frag->len;
            pData = (uint8 *) frag->ptr;
        }
        crc = (uint8)((crc >> CHAR_BIT) ^ CrcTbl8[ crc ^ *pData++ ]);
    }
    return (uint8)(crc & 0xffU);
}

OSCL_EXPORT_REF uint8 CRC::Crc8Check(OsclSharedPtr<PVMFMediaDataImpl>& mediaData, bool hasCRC)
{
    uint8	crc;
    uint8	*pData = NULL;
    int32	fragIdx, curSize, dataSize;
    OsclRefCounterMemFrag frag;

    crc = 0;
    fragIdx = 0;
    //Don't include CRC field in calculation
    if (hasCRC)
    {
        dataSize = mediaData->getFilledSize() - 1;
    }
    else
    {
        dataSize = mediaData->getFilledSize();
    }
    curSize = 0;
    while (--dataSize >= 0)
    {

        if ((--curSize) <= 0)
        {
            mediaData->getMediaFragment(fragIdx, frag);
            fragIdx++;
            curSize = frag.getMemFragSize();
            pData = (uint8 *) frag.getMemFragPtr();
        }
        crc = (uint8)((crc >> CHAR_BIT) ^ CrcTbl8[ crc ^ *pData++ ]);
    }
    return (uint8)(crc & 0xffU);
}


OSCL_EXPORT_REF uint16 CRC::Crc16Check(uint8 *pAlPdu, int16 Size)
{
    uint16	crc;

    crc = 0xffffU;
    while (--Size >= 0)
    {
        crc = (uint16)((crc >> CHAR_BIT) ^ CrcTbl16[(uint8)crc ^ *pAlPdu++ ]);
    }
    return (uint16)(crc ^ 0xffffU);
    /*
    crc ^= 0xffffU;
    crc = (crc&0xff)<<8 | crc>>8;
    return crc;
    */
}

OSCL_EXPORT_REF uint16 CRC::Crc16Check(Packet *pPkt)
{
    uint16	crc;
    uint8	*pData = NULL;
    int32	fragIdx, curSize, dataSize;
    BufferFragment* frag = NULL;

    crc = 0xffffU;
    fragIdx = 0;
    dataSize = pPkt->GetMediaSize();
    curSize = 0;
    while (--dataSize >= 0)
    {

        if ((--curSize) <= 0)
        {
            frag = pPkt->GetMediaFragment(fragIdx);
            fragIdx++;
            curSize = frag->len;
            pData = (uint8 *) frag->ptr;
        }
        crc = (uint16)((crc >> CHAR_BIT) ^ CrcTbl16[(uint8)crc ^ *pData++ ]);
    }
    return (uint16)(crc ^ 0xffffU);
    /*
    crc ^= 0xffffU;
    crc = (crc&0xff)<<8 | crc>>8;
    return crc;
    */

}

OSCL_EXPORT_REF uint16 CRC::Crc16Check(OsclSharedPtr<PVMFMediaDataImpl>& mediaData, bool hasCRC)
{
    uint16	crc;
    uint8	*pData = NULL;
    int32	fragIdx, curSize, dataSize;
    OsclRefCounterMemFrag frag;

    crc = 0xffffU;
    fragIdx = 0;
    //Don't include CRC field in calculation
    if (hasCRC)
    {
        dataSize = mediaData->getFilledSize() - 2;
    }
    else
    {
        dataSize = mediaData->getFilledSize();
    }
    curSize = 0;

    while (--dataSize >= 0)
    {

        if ((--curSize) <= 0)
        {
            mediaData->getMediaFragment(fragIdx, frag);
            fragIdx++;
            curSize = frag.getMemFragSize();
            pData = (uint8 *) frag.getMemFragPtr();
        }
        crc = (uint16)((crc >> CHAR_BIT) ^ CrcTbl16[(uint8)crc ^ *pData++ ]);
    }
    return (uint16)(crc ^ 0xffffU);
}
