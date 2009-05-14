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
/** class CCRGB24toYUV420.cpp
*/
#include "ccrgb24toyuv420.h"

OSCL_EXPORT_REF ColorConvertBase* CCRGB24toYUV420 :: New()
{
    CCRGB24toYUV420* self = OSCL_NEW(CCRGB24toYUV420, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCRGB24toYUV420::CCRGB24toYUV420()
{
}


OSCL_EXPORT_REF CCRGB24toYUV420 :: ~CCRGB24toYUV420()
{
// add destructor code here
    if (_mInitialized == true)
    {
        freeRGB2YUVTables();
    }
}


void CCRGB24toYUV420::freeRGB2YUVTables()
{
    if (iY_Table) oscl_free(iY_Table);
    if (iCb_Table) oscl_free(iCb_Table);
    if (iCr_Table) oscl_free(iCr_Table);

    iY_Table  = NULL;
    iCb_Table = iCr_Table = ipCb_Table = ipCr_Table = NULL;

}


int32 CCRGB24toYUV420:: Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width,
                             int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    int i;
    uint8 *pTable;

    if ((Src_width != Dst_width) || (Src_height != Dst_height) || (nRotation != 0 && nRotation != CCBOTTOM_UP))
    {
        return 0;
    }

    iBottomUp = false;
    if (nRotation == CCBOTTOM_UP)
    {
        iBottomUp = true;
    }


    if (_mInitialized == true)
    {
        freeRGB2YUVTables();
        _mInitialized = false;
    }

    /* memory allocation */
    if ((iY_Table = (uint8*)oscl_malloc(384)) == NULL)
        return 0;

    if ((iCb_Table = (uint8*)oscl_malloc(768 * 2)) == NULL)
        return 0;

    if ((iCr_Table = (uint8*)oscl_malloc(768 * 2)) == NULL)
        return 0;

#define pv_max(a, b)	((a) >= (b) ? (a) : (b))
#define pv_min(a, b)	((a) <= (b) ? (a) : (b))

    /* Table generation */
    for (i = 0; i < 384; i++)
        iY_Table[i] = (uint8) pv_max(pv_min(255, (int32)(0.7152 * i + 16 + 0.5)), 0);

    pTable = iCb_Table + 384;
    for (i = -384; i < 384; i++)
        pTable[i] = (uint8) pv_max(pv_min(255, (int32)(0.386 * i + 128 + 0.5)), 0);
    ipCb_Table = iCb_Table + 384;

    pTable = iCr_Table + 384;
    for (i = -384; i < 384; i++)
        pTable[i] = (uint8) pv_max(pv_min(255, (int32)(0.454 * i + 128 + 0.5)), 0);
    ipCr_Table = iCr_Table + 384;


    _mSrc_width  = Src_width;
    _mSrc_height = Src_height;
    _mSrc_pitch = Src_pitch;
    _mDst_width = Dst_width;
    _mDst_height = Dst_height;
    _mDst_mheight = Dst_height;
    _mDst_pitch = Dst_pitch;
    _mInitialized = true;

    SetMode(0);

    return 1;
}


int32 CCRGB24toYUV420::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    // only return value for _mState = 0 case
    return ((((_mSrc_width + 15) >> 4) << 4) * (((_mSrc_height + 15) >> 4) << 4) * 3 / 2);
}


int32 CCRGB24toYUV420::SetMode(int32 nMode)
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 1) // do not allow scaling nor rotation
    {
        _mState = 1;
        return 0;
    }
    else
    {
        _mState = 0;
        return 1;
    }
}

int32 CCRGB24toYUV420::SetYuvFullRange(bool range)
{
    OSCL_ASSERT(_mInitialized == true);
    if (range == true)
    {
        return 0; // not supported yet
    }
    else
    {
        return 1;
    }
}

extern "C"
{
    int32 ccrgb24toyuv(uint8 *rgb24, uint8 *yuv[], uint32 *param, uint8 *table[]);
}

int32 CCRGB24toYUV420::Convert(uint8 *rgb24, uint8 *yuv420)
{
    uint32 param[3];
    uint8 *table[3];
    int32 size16 = _mDst_pitch * _mDst_mheight;
    uint8 *yuv[3];

    OSCL_ASSERT(rgb24);
    OSCL_ASSERT(yuv420);
    OSCL_ASSERT(_mInitialized == true);

    param[0] = (uint32) _mSrc_width;
    param[1] = (uint32) _mSrc_height;
    param[2] = (uint32) iBottomUp;

    table[0] = iY_Table;
    table[1] = ipCb_Table;
    table[2] = ipCr_Table;

    yuv[0] = yuv420;
    yuv[1] = yuv420 + size16;
    yuv[2] = yuv[1] + (size16 >> 2);

    return ccrgb24toyuv(rgb24, yuv, param, table);

}

// in this overload, yuv420 is the output, rgb24 is input
int32 CCRGB24toYUV420::Convert(uint8 *rgb24, uint8 **yuv420)
{
    uint32 param[3];
    uint8 *table[3];

    OSCL_ASSERT(rgb24);
    OSCL_ASSERT(yuv420);
    OSCL_ASSERT(yuv420[0]);
    OSCL_ASSERT(yuv420[1]);
    OSCL_ASSERT(yuv420[2]);
    OSCL_ASSERT(_mInitialized == true);

    param[0] = (uint32) _mSrc_width;
    param[1] = (uint32) _mSrc_height;
    param[2] = (uint32) iBottomUp;

    table[0] = iY_Table;
    table[1] = ipCb_Table;
    table[2] = ipCr_Table;

    return ccrgb24toyuv(rgb24, yuv420, param, table);

}

int32 ccrgb24toyuv(uint8 *rgb24, uint8 *yuv[], uint32 *param, uint8 *table[])
{
    uint8 *inputRGB = NULL;
    int i, j, ilimit, jlimit;
    uint8 *tempY, *tempU, *tempV;
    uint8 *inputRGB_prevRow = NULL;

    int32 width = param[0];
    int32 height = param[1];
    int32 width_16 = ((width + 15) >> 4) << 4;
    int32 height_16 = ((height + 15) >> 4) << 4;
    uint32 iBottomUp = param[2];
    uint8 *y_tab = table[0];
    uint8 *cb_tab = table[1];
    uint8 *cr_tab = table[2];
    int32 size16 = height_16 * width_16;
    int32 adjust = (width + (width << 1));

    inputRGB = rgb24;

    /* do padding at the bottom first */
    /* do padding if input RGB size(height) is different from the output YUV size(height_16) */
    if (height < height_16 || width < width_16) /* if padding */
    {
        int offset = (height < height_16) ? height : height_16;

        offset = (offset * width_16);

        if (width < width_16)
        {
            offset -= (width_16 - width);
        }
        tempY = yuv[0] + offset;
        oscl_memset((uint8 *)tempY, 16, size16 - offset); /* pad with zeros */

        tempU = yuv[0] + size16 + (offset >> 2);
        oscl_memset((uint8 *)tempU, 128, (size16 - offset) >> 2);

        tempV = yuv[0] + size16 + (size16 >> 2) + (offset >> 2);
        oscl_memset((uint8 *)tempV, 128, (size16 - offset) >> 2);
    }

    /* then do padding on the top */
    tempY = yuv[0]; /* Normal order */
    tempU = yuv[0] + size16;
    tempV = tempU + (size16 >> 2);

    /* To center the output */
    if (height_16 > height)
    {
        if (width_16 >= width)
        {
            i = ((height_16 - height) >> 1) * width_16 + (((width_16 - width) >> 3) << 2);
            /* make sure that (width_16-width)>>1 is divisible by 4 */
            j = ((height_16 - height) >> 2) * (width_16 >> 1) + (((width_16 - width) >> 4) << 2);
            /* make sure that (width_16-width)>>2 is divisible by 4 */
        }
        else
        {
            i = ((height_16 - height) >> 1) * width_16;
            j = ((height_16 - height) >> 2) * (width_16 >> 1);
            inputRGB += ((width - width_16) >> 1) * 3;
        }
        oscl_memset((uint8 *)tempY, 16, i);
        tempY += i;
        oscl_memset((uint8 *)tempU, 128, j);
        tempU += j;
        oscl_memset((uint8 *)tempV, 128, j);
        tempV += j;
    }
    else
    {
        if (width_16 >= width)
        {
            i = (((width_16 - width) >> 3) << 2);
            /* make sure that (width_16-width)>>1 is divisible by 4 */
            j = (((width_16 - width) >> 4) << 2);
            /* make sure that (width_16-width)>>2 is divisible by 4 */
            inputRGB += (((height - height_16) >> 1) * width) * 3;
        }
        else
        {
            i = 0;
            j = 0;
            inputRGB += (((height - height_16) >> 1) * width + ((width - width_16) >> 1)) * 3;
        }
        oscl_memset((uint8 *)tempY, 16, i);
        tempY += i;
        oscl_memset((uint8 *)tempU, 128, j);
        tempU += j;
        oscl_memset((uint8 *)tempV, 128, j);
        tempV += j;
    }

    /* ColorConv RGB24-to-YUV420 with cropping or zero-padding */
    if (height < height_16)
        jlimit = height;
    else
        jlimit = height_16;

    if (width < width_16)
        ilimit = width;
    else
        ilimit = width_16;

    if (iBottomUp == 1)
    {
        inputRGB += (jlimit - 1) * width * 3 ; // move to last row
        adjust = -adjust;
    }

    for (j = 0; j < jlimit; j++)
    {
        for (i = 0; i < ilimit*3; i += 3)
        {

            *tempY++ = y_tab[(6616*inputRGB[i] + (inputRGB[i+1] << 16) + 19481 * inputRGB[i+2]) >> 16];

            /* downsampling U, V */
            if (j % 2 == 1 && i % 2 == 1)
            {

                *tempU++ = (unsigned char)((cb_tab[((inputRGB[i] << 16) - (inputRGB[i+1] << 16) + 19525 * (inputRGB[i] - inputRGB[i+2])) >> 16] + /* bottom right(current) */
                                            cb_tab[((inputRGB[i-3] << 16) - (inputRGB[i-2] << 16) + 19525 * (inputRGB[i-3] - inputRGB[i-1])) >> 16] + /* bottom left */
                                            cb_tab[((inputRGB_prevRow[i] << 16) - (inputRGB_prevRow[i+1] << 16) + 19525 * (inputRGB_prevRow[i] - inputRGB_prevRow[i+2])) >> 16] + /* top right */
                                            cb_tab[((inputRGB_prevRow[i-3] << 16) - (inputRGB_prevRow[i-2] << 16) + 19525 * (inputRGB_prevRow[i-3] - inputRGB_prevRow[i-1])) >> 16]  + /* top left */
                                            2) >> 2);


                *tempV++ = (unsigned char)((cr_tab[((inputRGB[i+2] << 16) - (inputRGB[i+1] << 16) + 6640 * (inputRGB[i+2] - inputRGB[i])) >> 16] + /* bottom right(current) */
                                            cr_tab[((inputRGB[i-1] << 16) - (inputRGB[i-2] << 16) + 6640 * (inputRGB[i-1] - inputRGB[i-3])) >> 16] + /* bottom left */
                                            cr_tab[((inputRGB_prevRow[i+2] << 16) - (inputRGB_prevRow[i+1] << 16) + 6640 * (inputRGB_prevRow[i+2] - inputRGB_prevRow[i])) >> 16] + /* top right */
                                            cr_tab[((inputRGB_prevRow[i-1] << 16) - (inputRGB_prevRow[i-2] << 16) + 6640 * (inputRGB_prevRow[i-1] - inputRGB_prevRow[i-3])) >> 16]  + /* top left */
                                            2) >> 2);

            }
        }

        /* do padding if input RGB size(width) is different from the output YUV size(width_16) */
        if (width < width_16 && j < jlimit - 1)
        {
            oscl_memset(tempY, 16/* *(tempY-1)*/, width_16 - width);
            tempY += (width_16 - width);

            if (j % 2 == 1)
            {
                oscl_memset(tempU, 128/* *(tempU-1)*/, (width_16 - width) >> 1);
                tempU += (width_16 - width) >> 1;
                oscl_memset(tempV, 128/* *(tempV-1)*/, (width_16 - width) >> 1);
                tempV += (width_16 - width) >> 1;
            }
        }

        inputRGB_prevRow = inputRGB;
        inputRGB += adjust ; /* move to the next row */
    }

    return 1;
}
