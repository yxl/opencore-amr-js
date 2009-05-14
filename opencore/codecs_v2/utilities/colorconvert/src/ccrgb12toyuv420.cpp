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
/** class CCRGB12toYUV420.cpp
*/
#include "ccrgb12toyuv420.h"

OSCL_EXPORT_REF ColorConvertBase* CCRGB12toYUV420 :: New()
{
    CCRGB12toYUV420* self = OSCL_NEW(CCRGB12toYUV420, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCRGB12toYUV420::CCRGB12toYUV420()
{
}


OSCL_EXPORT_REF CCRGB12toYUV420 :: ~CCRGB12toYUV420()
{
// add destructor code here
    if (_mInitialized == true)
    {
        freeRGB2YUVTables();
    }
}


void CCRGB12toYUV420::freeRGB2YUVTables()
{
    if (iY_Table) oscl_free(iY_Table);
    if (iCb_Table) oscl_free(iCb_Table);
    if (iCr_Table) oscl_free(iCr_Table);

    iY_Table  = NULL;
    iCb_Table = iCr_Table = ipCb_Table = ipCr_Table = NULL;

}


int32 CCRGB12toYUV420:: Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width,
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


int32 CCRGB12toYUV420::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    // only return value for _mState = 0 case
    return ((((_mSrc_width + 15) >> 4) << 4) * (((_mSrc_height + 15) >> 4) << 4) * 3 / 2);
}


int32 CCRGB12toYUV420::SetMode(int32 nMode)
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

int32 CCRGB12toYUV420::SetYuvFullRange(bool range)
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
    int32 ccrgb12toyuv(uint8 *rgb12, uint8 *yuv[], uint32 *param, uint8 *table[]);
}

int32 CCRGB12toYUV420::Convert(uint8 *rgb12, uint8 *yuv420)
{
    uint32 param[3];
    uint8 *table[3];
    int32 size16 = _mDst_pitch * _mDst_mheight;
    uint8 *yuv[3];

    OSCL_ASSERT(rgb12);
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

    return ccrgb12toyuv(rgb12, yuv, param, table);

}

int32 CCRGB12toYUV420::Convert(uint8 *rgb12, uint8 **yuv420)
{
    uint32 param[3];
    uint8 *table[3];

    OSCL_ASSERT(rgb12);
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

    return ccrgb12toyuv(rgb12, yuv420, param, table);

}

int32 ccrgb12toyuv(uint8 *rgb12, uint8 *yuv[], uint32 *param, uint8 *table[])
{
    uint32 *inputRGB = NULL;
    int i, j, ilimit, jlimit;
    uint32 *tempY, *tempU, *tempV;
    uint32 pixels;
    uint32 pixels_nextRow;
    uint32 yuv_value;
    uint32 yuv_value1;
    uint32 yuv_value2;
    int R_ds; /* "_ds" is the downsample version */
    int G_ds; /* "_ds" is the downsample version */
    int B_ds; /* "_ds" is the downsample version */
    int tmp;
    uint32 temp;

    int32 width = param[0];
    int32 height = param[1];
    int32 width_16 = ((width + 15) >> 4) << 4;
    int32 height_16 = ((height + 15) >> 4) << 4;
    uint32 iBottomUp = param[2];
    uint8 *y_tab = table[0];
    uint8 *cb_tab = table[1];
    uint8 *cr_tab = table[2];
    int32 size16 = height_16 * width_16;
    int adjust = (width >> 1);

    inputRGB = (uint32*) rgb12;

    /* do padding at the bottom first */
    /* do padding if input RGB size(height) is different from the output YUV size(height_16) */
    if (height < height_16 || width < width_16)
    { /* if padding */
        int offset = (height < height_16) ? height : height_16;

        offset = (offset * width_16);

        if (width < width_16)
        {
            offset -= (width_16 - width);
        }

        tempY = (uint32 *)yuv[0] + (offset >> 2);
        oscl_memset((uint8 *)tempY, 16, size16 - offset); /* pad with zeros */

        tempU = (uint32 *)yuv[0] + (size16 >> 2) + (offset >> 4);
        oscl_memset((uint8 *)tempU, 128, (size16 - offset) >> 2);

        tempV = (uint32 *)yuv[0] + (size16 >> 2) + (size16 >> 4) + (offset >> 4);
        oscl_memset((uint8 *)tempV, 128, (size16 - offset) >> 2);
    }

    /* then do padding on the top */
    tempY = (uint32 *)yuv[0]; /* Normal order */
    tempU = tempY + ((size16) >> 2);
    tempV = tempU + ((size16) >> 4);

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
            inputRGB += (width - width_16) >> 2;
        }
        oscl_memset((uint8 *)tempY, 16, i);
        tempY += (i >> 2);
        oscl_memset((uint8 *)tempU, 128, j);
        tempU += (j >> 2);
        oscl_memset((uint8 *)tempV, 128, j);
        tempV += (j >> 2);
    }
    else
    {
        if (width_16 >= width)
        {
            i = (((width_16 - width) >> 3) << 2);
            /* make sure that (width_16-width)>>1 is divisible by 4 */
            j = (((width_16 - width) >> 4) << 2);
            /* make sure that (width_16-width)>>2 is divisible by 4 */
            inputRGB += (((height - height_16) >> 1) * width) >> 1;

            oscl_memset((uint8 *)tempY, 16, i);
            tempY += (i >> 2);
            oscl_memset((uint8 *)tempU, 128, j);
            tempU += (j >> 2);
            oscl_memset((uint8 *)tempV, 128, j);
            tempV += (j >> 2);

        }
        else
        {
            i = 0;
            j = 0;
            inputRGB += (((height - height_16) >> 1) * width + ((width - width_16) >> 1)) >> 1 ;
        }
    }

    /* ColorConv RGB12-to-YUV420 with cropping or zero-padding */
    if (height < height_16)
    {
        jlimit = height;
    }
    else
    {
        jlimit = height_16;
    }

    if (width < width_16)
    {
        ilimit = width >> 1;
    }
    else
    {
        ilimit = width_16 >> 1;
    }

    if (iBottomUp == 1)
    {
        inputRGB += (jlimit - 1) * (width >> 1) ; // move to last row
        adjust = -adjust;
    }

    width = width_16 - width;


    for (j = 0; j < jlimit; j++)
    {

        for (i = 0; i < ilimit; i += 4)
        {
            pixels =  inputRGB[i];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            yuv_value = (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]    |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8));

            pixels =  inputRGB[i+1];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            *tempY++ = (yuv_value                                                         |
                        (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]     |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8)) << 16);

            pixels =  inputRGB[i+2];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            yuv_value = (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]    |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8));

            pixels =  inputRGB[i+3];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            *tempY++ = (yuv_value                                                         |
                        (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]     |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8)) << 16);


            /* downsampling U, V */

            pixels_nextRow = inputRGB[i+3+adjust/*(width>>1)*/];
            G_ds    =  pixels & 0x00F000F0;
            G_ds   += (pixels_nextRow & 0x00F000F0);
            G_ds   += (G_ds >> 16);

            G_ds   -= 2; /* equivalent to adding constant 2<<16 = 131072 */

            pixels &= 0x0F0F0F0F;
            pixels += (pixels_nextRow & 0x0F0F0F0F);

            pixels += (pixels >> 16);

            B_ds = (pixels & 0x0003F) << 4;

            R_ds = (pixels & 0x03F00) >> 4;

            tmp  = B_ds - R_ds;

            yuv_value1 = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18] << 24;
            yuv_value2 = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18] << 24;

            pixels =  inputRGB[i+2];
            pixels_nextRow = inputRGB[i+2+adjust/*(width>>1)*/];

            G_ds    =  pixels & 0x00F000F0;
            G_ds   += (pixels_nextRow & 0x00F000F0);
            G_ds   += (G_ds >> 16);

            G_ds   -= 2; /* equivalent to adding constant 2<<16 = 131072 */

            pixels &= 0x0F0F0F0F;
            pixels += (pixels_nextRow & 0x0F0F0F0F);

            pixels += (pixels >> 16);

            B_ds = (pixels & 0x0003F) << 4;

            R_ds = (pixels & 0x03F00) >> 4;
            tmp  = B_ds - R_ds;

            yuv_value1 |= cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18] << 16;
            yuv_value2 |= cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18] << 16;

            pixels =  inputRGB[i+1];
            pixels_nextRow = inputRGB[i+1+adjust /*(width>>1)*/];

            G_ds    =  pixels & 0x00F000F0;
            G_ds   += (pixels_nextRow & 0x00F000F0);
            G_ds   += (G_ds >> 16);

            G_ds   -= 2; /* equivalent to adding constant 2<<16 = 131072 */

            pixels &= 0x0F0F0F0F;
            pixels += (pixels_nextRow & 0x0F0F0F0F);

            pixels += (pixels >> 16);

            B_ds = (pixels & 0x0003F) << 4;

            R_ds = (pixels & 0x03F00) >> 4;
            tmp  = B_ds - R_ds;


            yuv_value1 |= cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18] << 8;
            yuv_value2 |= cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18] << 8;

            pixels =  inputRGB[i];
            pixels_nextRow = inputRGB[i+adjust/*(width>>1)*/];

            G_ds    =  pixels & 0x00F000F0;
            G_ds   += (pixels_nextRow & 0x00F000F0);
            G_ds   += (G_ds >> 16);

            G_ds   -= 2; /* equivalent to adding constant 2<<16 = 131072 */

            pixels &= 0x0F0F0F0F;
            pixels += (pixels_nextRow & 0x0F0F0F0F);

            pixels += (pixels >> 16);

            B_ds = (pixels & 0x0003F) << 4;

            R_ds = (pixels & 0x03F00) >> 4;
            tmp  = B_ds - R_ds;

            *tempU++ = yuv_value1 | (cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18]);
            *tempV++ = yuv_value2 | (cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18]);
        }

        /* do padding if input RGB size(width) is different from the output YUV size(width_16) */

        if ((width > 0) && j < jlimit - 1)
        {
            oscl_memset((uint8 *)tempY, 16/*(*(tempY-1))>>24*/, width);
            tempY += width >> 2;
        }

        if (j++ == (jlimit - 1))
        {
            break; /* dealing with a odd height */
        }

        if ((width > 0) && j < jlimit - 1)
        {
            oscl_memset((uint8 *)tempU, 128/*(*(tempU-1))>>24*/, width >> 1);
            tempU += width >> 3;
            oscl_memset((uint8 *)tempV, 128/*(*(tempV-1))>>24*/, width >> 1);
            tempV += width >> 3;
        }

        inputRGB += adjust; /* (160/2 = 80 ) */ /*(width>>1)*/; /* move to the next row */

        for (i = 0; i < ilimit; i += 4)
        {
            pixels =  inputRGB[i];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            yuv_value = (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]    |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8));

            pixels =  inputRGB[i+1];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            *tempY++ = (yuv_value                                                         |
                        (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]     |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8)) << 16);

            pixels =  inputRGB[i+2];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            yuv_value = (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]    |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8));

            pixels =  inputRGB[i+3];
            temp = (827 * (pixels & 0x000F000F) + 2435 * ((pixels & 0x0F000F00) >> 8));
            *tempY++ = (yuv_value                                                         |
                        (y_tab[((temp&0x0FFFF)>> 9) + (pixels & 0x000000F0)]     |
                         (y_tab[(temp         >>25) + ((pixels & 0x00F00000)>>16)] << 8)) << 16);

        }

        /* do padding if input RGB size(width) is different from the output YUV size(width_16) */
        if ((width > 0) && j < jlimit - 1)
        {
            oscl_memset((uint8 *)tempY, 16/*(*(tempY-1))>>24*/, width);
            tempY += width >> 2;
        }

        inputRGB += adjust; /* (160/2 = 80 ) */ /*(width>>1)*/; /* move to the next row */

    } /* for(j=0; j<jlimit; j++)*/

    return 1;
}
