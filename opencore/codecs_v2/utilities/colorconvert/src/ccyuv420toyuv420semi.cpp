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
/** Class ColorConvertYUV420SEMI, convert YUV to YUV420SEMI PLANAR */
#include "colorconv_config.h"
#include "ccyuv420toyuv420semi.h"
#include "osclconfig_compiler_warnings.h"

OSCL_EXPORT_REF ColorConvertBase* ColorConvertYUV420SEMI::NewL(void)
{
    ColorConvertYUV420SEMI* self = OSCL_NEW(ColorConvertYUV420SEMI, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


ColorConvertYUV420SEMI::ColorConvertYUV420SEMI()
{
}


OSCL_EXPORT_REF ColorConvertYUV420SEMI::~ColorConvertYUV420SEMI()
{
}


int32 ColorConvertYUV420SEMI::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    _mInitialized = true;

    return 1;
}

int32  ColorConvertYUV420SEMI::SetYuvFullRange(bool range)
{
    OSCL_UNUSED_ARG(range);
    OSCL_ASSERT(_mInitialized == true);
    return 1;
}


int32 ColorConvertYUV420SEMI::SetMode(int32 nMode)	//iMode : 0 Off, 1 On
{
    OSCL_UNUSED_ARG(nMode);
    OSCL_ASSERT(_mInitialized == true);
    return 1;
}


int32 ColorConvertYUV420SEMI::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return	_mState ? ((_mDst_height*_mDst_pitch*3) / 2) : ((_mSrc_width*_mSrc_height*3) / 2);
}


int32 ColorConvertYUV420SEMI::Convert(uint8 **yuvBuf, uint8 *rgbBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(yuvBuf[0]);
    OSCL_ASSERT(yuvBuf[1]);
    OSCL_ASSERT(yuvBuf[2]);
    OSCL_ASSERT(rgbBuf);

    int i, j;
    uint8 *inCb, *inCr;
    uint32 *inY, *outYUV420SEMI;
    uint32 temp, tempU, tempV;
    int offset;

    if (((uint32)rgbBuf)&0x3 || ((uint32)yuvBuf[0])&0x3) /* address is not word align */
    {
        return 0;
    }

    inY = (uint32*) yuvBuf[0];
    inCb = yuvBuf[1];
    inCr = yuvBuf[2];
    outYUV420SEMI = (uint32 *)rgbBuf;

    offset = _mDst_pitch - _mSrc_width;

    /* Y copying */
    for (i = _mSrc_height; i > 0; i--)
    {
        for (j = _mSrc_width >> 2; j > 0; j--)
        {
            temp = *inY++;
            *outYUV420SEMI++ = temp;
        }
        /* in case the dest pitch is larger than width */
        outYUV420SEMI += (offset >> 2);
    }

    /* U & V copying */
    for (i = _mSrc_height >> 1; i > 0; i--)
    {
        for (j = _mSrc_width >> 2; j > 0; j--)
        {
            tempU = *inCb++;	//U0
            tempV = *inCr++;	//V0
            temp = tempU | (tempV << 8) ; //V0U0
            tempU = *inCb++;	//U1
            tempV = *inCr++;	//V1
            tempU = tempU | (tempV << 8) ; //V1U1
            temp |= (tempU << 16); //V1U1V0U0
            *outYUV420SEMI++ = temp;
        }
        /* in case the dest pitch is larger than width */
        outYUV420SEMI += (offset >> 2);
    }

    return 1;
}


int32 ColorConvertYUV420SEMI::Convert(uint8 *yuvBuf, uint8 *rgbBuf)
{
    //this conversion will cause problems when do src clipping. However, if they want, they must give more info
    uint8 *TmpYuvBuf[3];

    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(rgbBuf);

    if (((uint32)rgbBuf)&0x3 || ((uint32)yuvBuf)&0x3) /* address is not word align */
    {
        return 0;
    }

    TmpYuvBuf[0]	=	yuvBuf;
    TmpYuvBuf[1]	=	yuvBuf + (_mSrc_pitch) * (_mSrc_mheight);
    TmpYuvBuf[2]	=	TmpYuvBuf[1] + (_mSrc_pitch * _mSrc_mheight) / 4;

    return (Convert(TmpYuvBuf, rgbBuf));
}


