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
/** class CCRGB16toYUV420.cpp
*/
#include "ccrgb16toyuv420.h"

OSCL_EXPORT_REF ColorConvertBase* CCRGB16toYUV420 :: New()
{
    CCRGB16toYUV420* self = OSCL_NEW(CCRGB16toYUV420, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCRGB16toYUV420::CCRGB16toYUV420()
{
}


OSCL_EXPORT_REF CCRGB16toYUV420 :: ~CCRGB16toYUV420()
{
// add destructor code here
    if (_mInitialized == true)
    {
        freeRGB2YUVTables();
    }
}


void CCRGB16toYUV420::freeRGB2YUVTables()
{
    if (iY_Table) oscl_free(iY_Table);
    if (iCb_Table) oscl_free(iCb_Table);
    if (iCr_Table) oscl_free(iCr_Table);

    iY_Table  = NULL;
    iCb_Table = iCr_Table = ipCb_Table = ipCr_Table = NULL;

}


int32 CCRGB16toYUV420:: Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width,
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

    mUseColorKey = false;
    _mInitialized = true;

    SetMode(0); // called after init

    return 1;
}


int32 CCRGB16toYUV420::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return ((((_mSrc_width + 15) >> 4) << 4) * (((_mSrc_height + 15) >> 4) << 4) * 3 / 2);
}


int32 CCRGB16toYUV420::SetMode(int32 nMode)
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


void CCRGB16toYUV420:: SetColorkey(int16 colorkey)
{
    OSCL_ASSERT(_mInitialized == true);

    mColorKey = colorkey;
    mUseColorKey = true;
}


int32 CCRGB16toYUV420::SetYuvFullRange(bool range)
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
    int32 ccrgb16toyuv(uint8 *rgb16, uint8 *yuv[], uint32 *param, uint8 *table[]);
    int32 ccrgb16toyuv_wo_colorkey(uint8 *rgb16, uint8 *yuv[], uint32 *param, uint8 *table[]);
}

int32 CCRGB16toYUV420::Convert(uint8 *rgb16, uint8 *yuv420)
{
    uint32 param[7];
    uint8 *table[3];
    int32 size16 = _mDst_pitch * _mDst_mheight;
    uint8 *yuv[3];

    OSCL_ASSERT(rgb16);
    OSCL_ASSERT(yuv420);
    OSCL_ASSERT(_mInitialized == true);

    param[0] = (uint32) _mDst_width;
    param[1] = (uint32) _mDst_height;
    param[2] = (uint32) _mDst_pitch;
    param[3] = (uint32) _mDst_mheight;
    param[4] = (uint32) _mSrc_pitch;
    param[5] = (uint32) mColorKey;
    param[6] = (uint32) iBottomUp;

    table[0] = iY_Table;
    table[1] = ipCb_Table;
    table[2] = ipCr_Table;

    yuv[0] = yuv420;
    yuv[1] = yuv420 + size16;
    yuv[2] = yuv[1] + (size16 >> 2);

    if (mUseColorKey == true)
    {
        return ccrgb16toyuv(rgb16, yuv, param, table);
    }
    else
    {
        return ccrgb16toyuv_wo_colorkey(rgb16, yuv, param, table);
    }
}

extern "C"
{

    int32 ccrgb16toyuv(uint8 *rgb16, uint8 *yuv[], uint32 *param, uint8 *table[])
    {
        uint16 *inputRGB = (uint16*)rgb16;
        int32 width_dst = param[0];
        int32 height_dst = param[1];
        int32 pitch_dst = param[2];
        int32 pitch_src = param[4];
        uint16 colorkey = param[5];
        uint8 *y_tab = table[0];
        uint8 *cb_tab = table[1];
        uint8 *cr_tab = table[2];

        int32 i, j, count;
        int32 ilimit, jlimit;
        uint8 *tempY, *tempU, *tempV;
        uint16 pixels;
        uint8 yuv_value;
        int32 R_ds; /* "_ds" is the downsample version */
        int32 G_ds; /* "_ds" is the downsample version */
        int32 B_ds; /* "_ds" is the downsample version */
        int   tmp;
        uint32 temp;
        uint16 cb, cr;

        //for writing to the YUV buffer which is a pointer to uint32
        tempY = yuv[0];
        tempU = yuv[1];
        tempV = yuv[2];

        jlimit = height_dst;
        ilimit = width_dst;

#define ALPHA 413		// 413  = (int) ((0.0722/0.7152)* (2^12)) for B, 2^12 is for hitting 16bit range for SIMD calculation,
#define BETA  1218		// 1218 = (int) ((0.2126/0.7152)* (2^12)) for R  5bit B/R => ALPHA and BETA < 2^11
#define SHIFT_INDEX1 9  // 9    = 12 - 3, 3 refers to the number of bits right shifted for 5 bit B/R in an 8-bit octet
#define SHIFT_INDEX2 25 // 25   = 9  + 16

        for (j = 0; j < jlimit; j += 2)
        {
            for (i = 0; i < ilimit; i += 2)
            {
                G_ds = B_ds = R_ds = count = 0;

                pixels =  inputRGB[i];
                if (pixels != colorkey)
                {
                    temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                    yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                    *tempY = yuv_value;

                    G_ds    += (pixels >> 1) & 0x03E0;
                    B_ds    += (pixels << 5) & 0x03E0;
                    R_ds    += (pixels >> 6) & 0x03E0;
                    count++;
                }

                pixels =  inputRGB[i+pitch_src];
                if (pixels != colorkey)
                {
                    temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                    yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                    *(tempY + pitch_dst) = yuv_value;

                    G_ds    += (pixels >> 1) & 0x03E0;
                    B_ds    += (pixels << 5) & 0x03E0;
                    R_ds    += (pixels >> 6) & 0x03E0;
                    count++;
                }

                tempY++;

                pixels =  inputRGB[i+1];
                if (pixels != colorkey)
                {
                    temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                    yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                    *tempY = yuv_value;

                    G_ds    += (pixels >> 1) & 0x03E0;
                    B_ds    += (pixels << 5) & 0x03E0;
                    R_ds    += (pixels >> 6) & 0x03E0;
                    count++;
                }

                pixels =  inputRGB[i+1+pitch_src];
                if (pixels != colorkey)
                {
                    temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                    yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                    *(tempY + pitch_dst) = yuv_value;

                    G_ds    += (pixels >> 1) & 0x03E0;
                    B_ds    += (pixels << 5) & 0x03E0;
                    R_ds    += (pixels >> 6) & 0x03E0;
                    count++;
                }

                tempY++;

                if (count == 1) // three matches
                {
                    tmp = B_ds - R_ds;

                    cb = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18];
                    cr = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18];

                    *tempU = (cb + 3 * (*tempU)) >> 2;
                    *tempV = (cr + 3 * (*tempV)) >> 2;

                }
                else if (count == 2) // two matches
                {
                    R_ds >>= 1;
                    B_ds >>= 1;
                    G_ds >>= 1;

                    tmp = B_ds - R_ds;

                    cb = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18];
                    cr = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18];

                    *tempU = (cb + (*tempU)) >> 1;
                    *tempV = (cr + (*tempV)) >> 1;

                }
                else if (count == 3) // one match
                {
                    R_ds = R_ds / 3;
                    B_ds = B_ds / 3;
                    G_ds = G_ds / 3;

                    tmp = B_ds - R_ds;

                    cb = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18];
                    cr = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18];

                    *tempU = (3 * cb + *tempU) >> 2;
                    *tempV = (3 * cr + *tempV) >> 2;

                }
                else if (count == 4) // no match
                {
                    R_ds >>= 2;
                    B_ds >>= 2;
                    G_ds >>= 2;

                    tmp = B_ds - R_ds;

                    *tempU = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18];
                    *tempV = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18];
                }

                tempU++;
                tempV++;
            }

            inputRGB += pitch_src << 1;
            tempY += (pitch_dst - width_dst) + pitch_dst;
            tempU += ((pitch_dst - width_dst)) >> 1;
            tempV += ((pitch_dst - width_dst)) >> 1;
        }

        return 1;
    }


    int32 ccrgb16toyuv_wo_colorkey(uint8 *rgb16, uint8 *yuv[], uint32 *param, uint8 *table[])
    {
        uint16 *inputRGB = (uint16*)rgb16;
        int32 width_dst = param[0];
        int32 height_dst = param[1];
        int32 pitch_dst = param[2];
        int32 pitch_src = param[4];
        uint32 iBottomUp = param[6];
        uint8 *y_tab = table[0];
        uint8 *cb_tab = table[1];
        uint8 *cr_tab = table[2];

        int32 i, j, count;
        int32 ilimit, jlimit;
        uint8 *tempY, *tempU, *tempV;
        uint16 pixels;
        uint8 yuv_value;
        int32 R_ds; /* "_ds" is the downsample version */
        int32 G_ds; /* "_ds" is the downsample version */
        int32 B_ds; /* "_ds" is the downsample version */
        int   tmp;
        uint32 temp;

        //for writing to the YUV buffer which is a pointer to uint32
        tempY = yuv[0];
        tempU = yuv[1];
        tempV = yuv[2];

        jlimit = height_dst;
        ilimit = width_dst;

        if (iBottomUp == 1)
        {
            inputRGB += (jlimit - 1) * width_dst; // move to last row
            pitch_src = -pitch_src;
        }

#define ALPHA 413		// 413  = (int) ((0.0722/0.7152)* (2^12)) for B, 2^12 is for hitting 16bit range for SIMD calculation,
#define BETA  1218		// 1218 = (int) ((0.2126/0.7152)* (2^12)) for R  5bit B/R => ALPHA and BETA < 2^11
#define SHIFT_INDEX1 9  // 9    = 12 - 3, 3 refers to the number of bits right shifted for 5 bit B/R in an 8-bit octet
#define SHIFT_INDEX2 25 // 25   = 9  + 16

        for (j = 0; j < jlimit; j += 2)
        {
            for (i = 0; i < ilimit; i += 2)
            {
                G_ds = B_ds = R_ds = count = 0;

                pixels =  inputRGB[i];
                temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                *tempY = yuv_value;

                G_ds    += (pixels >> 1) & 0x03E0;
                B_ds    += (pixels << 5) & 0x03E0;
                R_ds    += (pixels >> 6) & 0x03E0;

                pixels =  inputRGB[i+pitch_src];
                temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                *(tempY + pitch_dst) = yuv_value;

                G_ds    += (pixels >> 1) & 0x03E0;
                B_ds    += (pixels << 5) & 0x03E0;
                R_ds    += (pixels >> 6) & 0x03E0;

                tempY++;

                pixels =  inputRGB[i+1];
                temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                *tempY = yuv_value;

                G_ds    += (pixels >> 1) & 0x03E0;
                B_ds    += (pixels << 5) & 0x03E0;
                R_ds    += (pixels >> 6) & 0x03E0;

                pixels =  inputRGB[i+1+pitch_src];
                temp = (ALPHA * (pixels & 0x001F) + BETA * (pixels >> 11));
                yuv_value = y_tab[(temp>>SHIFT_INDEX1) + ((pixels>>3) & 0x00FC)];
                *(tempY + pitch_dst) = yuv_value;

                G_ds    += (pixels >> 1) & 0x03E0;
                B_ds    += (pixels << 5) & 0x03E0;
                R_ds    += (pixels >> 6) & 0x03E0;

                tempY++;

                R_ds >>= 2;
                B_ds >>= 2;
                G_ds >>= 2;

                tmp = B_ds - R_ds;

                *tempU = cb_tab[(((B_ds-G_ds)<<16) + 19525*tmp)>>18];
                *tempV = cr_tab[(((R_ds-G_ds)<<16) -  6640*tmp)>>18];

                tempU++;
                tempV++;
            }

            inputRGB += pitch_src << 1;
            tempY += (pitch_dst - width_dst) + pitch_dst;
            tempU += ((pitch_dst - width_dst)) >> 1;
            tempV += ((pitch_dst - width_dst)) >> 1;
        }

        return 1;
    }

}

// in this overload, yuv420 is the output, rgb16 is input
int32 CCRGB16toYUV420::Convert(uint8 *rgb16, uint8 **yuv420)
{
    uint32 param[6];
    uint8 *table[3];

    OSCL_ASSERT(rgb16);
    OSCL_ASSERT(yuv420);
    OSCL_ASSERT(yuv420[0]);
    OSCL_ASSERT(yuv420[1]);
    OSCL_ASSERT(yuv420[2]);
    OSCL_ASSERT(_mInitialized == true);

    param[0] = (uint32) _mDst_width;
    param[1] = (uint32) _mDst_height;
    param[2] = (uint32) _mDst_pitch;
    param[3] = (uint32) _mDst_mheight;
    param[4] = (uint32) _mSrc_pitch;
    param[5] = (uint32) mColorKey;

    table[0] = iY_Table;
    table[1] = ipCb_Table;
    table[2] = ipCr_Table;

    if (mUseColorKey == true)
    {
        return ccrgb16toyuv(rgb16, yuv420, param, table);
    }
    else
    {
        return ccrgb16toyuv_wo_colorkey(rgb16, yuv420, param, table);
    }
}


