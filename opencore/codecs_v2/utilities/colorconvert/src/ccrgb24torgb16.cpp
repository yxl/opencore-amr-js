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
/** Class CCRGB24toRGB16. convert RGB24 to RGB16 by truncating the least significant bits. */
#include "colorconv_config.h"
#include "ccrgb24torgb16.h"


OSCL_EXPORT_REF ColorConvertBase* CCRGB24toRGB16::NewL(void)
{
    CCRGB24toRGB16* self = OSCL_NEW(CCRGB24toRGB16, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCRGB24toRGB16::CCRGB24toRGB16()
{
}


OSCL_EXPORT_REF CCRGB24toRGB16::~CCRGB24toRGB16()
{
}

int32 CCRGB24toRGB16::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{

    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    //default no zoom
    _mInitialized = true;

    SetMode(0); // called after init

    return 1;
}


int32 CCRGB24toRGB16::SetYuvFullRange(bool range)
{
    OSCL_UNUSED_ARG(range);
    OSCL_ASSERT(_mInitialized == true);

    return 1;  // do nothing here.
}

int32 CCRGB24toRGB16::SetMode(int32 nMode)	//nMode : 0 Off, 1 On
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 0)
    {
        //		mPtrConv = cc16Rotate;
        //		_mState		= 1;
        mPtrConv	=	&CCRGB24toRGB16::getframe;
        _mState		=	0;
        _mDisp.src_pitch = _mSrc_pitch  ;
        _mDisp.dst_pitch = _mSrc_width  ;
        _mDisp.src_width = _mSrc_width	;
        _mDisp.src_height = _mSrc_height ;
        _mDisp.dst_width = _mSrc_width	;
        _mDisp.dst_height = _mSrc_height ;
    }
    else
    {
#if CCSCALING
        if (_mIsZoom)
        {
#if CCROTATE
            if (_mRotation&0x1) /* zoom and rotate */
            {
                mPtrConv = &CCRGB24toRGB16::zoomRotate;
            }
            else /* zoom only */
#endif
            {
                mPtrConv	=	&CCRGB24toRGB16::zoom;
            }
        }
        else
#endif
        {
#if CCROTATE
            if (_mRotation&0x1) /* rotate only*/
            {
                mPtrConv = &CCRGB24toRGB16::rotate;
            }
            else /* no zoom, no rotate, SetMode(1) = SetMode(0) */
#endif
            {
                mPtrConv	=	&CCRGB24toRGB16::getframe;
            }
        }
        _mState		=	nMode;
        _mDisp.src_pitch = _mSrc_pitch  ;
        _mDisp.dst_pitch = _mDst_pitch	;
        _mDisp.src_width = _mSrc_width	;
        _mDisp.src_height = _mSrc_height ;
        _mDisp.dst_width = _mDst_width	;
        _mDisp.dst_height = _mDst_height ;
    }

    return 1;
}


int32 CCRGB24toRGB16::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    if (_mIsZoom)
    {
        // for zoom, need extra line of RGB_FORMAT buffer for processing otherwise memory will corrupt.
        return	_mState ? ((_mDst_height + 2)*_mDst_pitch*2) : (_mSrc_width*_mSrc_height*2);
    }
    else
    {
        return	_mState ? ((_mDst_height)*_mDst_pitch*2) : (_mSrc_width*_mSrc_height*2);
    }
}


int32 CCRGB24toRGB16::Convert(uint8 **srcBuf, uint8 *destBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(srcBuf);
    OSCL_ASSERT(srcBuf[0]);
    OSCL_ASSERT(srcBuf[1]);
    OSCL_ASSERT(srcBuf[2]);
    OSCL_ASSERT(destBuf);

    if (((uint32)destBuf)&0x3 || ((uint32)srcBuf[0])&0x3) /* address is not word align */
    {
        return 0;
    }

    return (*this.*mPtrConv)(srcBuf[0], destBuf, &_mDisp);
}


int32 CCRGB24toRGB16::Convert(uint8 *srcBuf, uint8 *destBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(srcBuf);
    OSCL_ASSERT(destBuf);

    if (((uint32)srcBuf)&0x3 || ((uint32)destBuf)&0x3) /* address is not word align */
    {
        return 0;
    }

    return (*this.*mPtrConv)(srcBuf, destBuf, &_mDisp);
}

int32 reverse(uint8 *src, uint8 *dst, int32 *disp);
int32 normal(uint8 *src, uint8 *dst, int32 *disp);

int32 CCRGB24toRGB16::getframe(uint8 *src, uint8 *dst, DisplayProperties *disp)
{
    int32 disp_prop[8];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;
    disp_prop[6] = (_mRotation > 0 ? 1 : 0);
    disp_prop[7] = _mIsFlip;

    if (disp_prop[6] ^ disp_prop[7])	/* either flip or rotate 180*/
    {
#if (!CCROTATE)
        return 0;
#else
        return reverse(src, dst, disp_prop);
#endif
    }
    else
    {
        return normal(src, dst, disp_prop);
    }
}

int32 normal(uint8 *src, uint8 *dst, int32 *disp)
{
    uint8	*pSrc;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width, src_height;
    int32		half1, half2;
    int32		deltaSrc, deltaDst;
    int32		row, col;
    uint32		rgb, tmp0;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    src_height  =   disp[3];

    if (disp[6]) /* rotate 180 and flip */
    {	/* move the starting point to the bottom-left corner of the picture */
        deltaSrc = src_pitch * (src_height - 1) * 3;
        pSrc = (uint8*)(src + deltaSrc);
        deltaSrc = -(src_width + src_pitch) * 3;
    }
    else
    {
        deltaSrc	=	(src_pitch - src_width) * 3;
        pSrc =  src;
    }

    deltaDst	=	(dst_pitch - src_width);
    pDst =	(uint16 *)dst;

    for (row = src_height; row > 0; row--)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {


            half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
            half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
            rgb = half1 & 0xF8;  /* get 5 bits of R0 */
            half1 = (half1 >> 5) & 0x7E0;  /* get 6 bits of G0 */
            rgb = (rgb << 8) | half1;				/*  R0, G0 */
            half1 = (half2 >> 3) & 0x1F;		/* get 5 bits of B0 */
            rgb |= half1;			/*  R0,G0,B0 */

            half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */

            half2 &= 0xF800;			/* get 5 bits of R1 */
            tmp0 = half1 & 0xFC;			/* get 6 bits of G1 */
            tmp0 = (tmp0 << 3) | half2;  /* R1 G1 */
            tmp0 |= (half1 >> 11);		/* R1 G1 B1 */
#else //BGR
            rgb = half2 & 0xF8;  /* get 5 bits of R0 */
            tmp0 = (half1 >> 5) & 0x7E0;  /* get 6 bits of G0 */
            rgb = (rgb << 8) | tmp0;				/*  R0, G0 */
            half1 = (half1 >> 3) & 0x1F;		/* get 5 bits of B0 */
            rgb |= half1;			/*  R0,G0,B0 */

            half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */

            tmp0 = half1 & 0xF800;		/* get 5 bits of R1 */
            half1 = (half1 << 3) & 0x7C0;  /* get G1 in the middle */
            tmp0 |= half1;             /* R1 G1 */
            tmp0 |= (half2 >> 11);     /* R1 G1 B1 */
#endif
            rgb = rgb | (tmp0 << 16);

            *((uint32*)pDst) = rgb;
            pDst += 2;
            pSrc += 2;

        }

        pSrc += deltaSrc;
        pDst += deltaDst;
    }

    return 1;
}

#if CCROTATE
int32 reverse(uint8 *src, uint8 *dst, int32 *disp)
{
    uint8	*pSrc;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width, src_height;
    int32		half1, half2;
    int32		deltaSrc, deltaDst;
    int32		row, col;
    uint32		rgb, tmp0;
    int	nextrow, mIsFlip;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    src_height  =   disp[3];
    mIsFlip		=   disp[7];

    deltaDst	=	(dst_pitch - src_width);
    pDst =	(uint16 *)dst;

    if (disp[6]) /* rotation, only */
    {  /* move the starting point to the bottom-right corner of the picture */
        nextrow = src_pitch * src_height * 3;
        pSrc = (src + nextrow - 6);
        deltaSrc	=	(src_width - src_pitch) * 3;
    }
    else	/* flip only */
    {   /* move the starting point to the top-right corner of the picture */
        pSrc = (src + src_width * 3 - 6);
        deltaSrc = (src_width + src_pitch) * 3;
    }

    for (row = src_height; row > 0; row--)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {


            half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
            half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
            rgb = half1 & 0xF8;  /* get 5 bits of R0 */
            half1 = (half1 >> 5) & 0x7E0;  /* get 6 bits of G0 */
            rgb = (rgb << 8) | half1;				/*  R0, G0 */
            half1 = (half2 >> 3) & 0x1F;		/* get 5 bits of B0 */
            rgb |= half1;			/*  R0,G0,B0 */

            half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */

            half2 &= 0xF800;			/* get 5 bits of R1 */
            tmp0 = half1 & 0xFC;			/* get 6 bits of G1 */
            tmp0 = (tmp0 << 3) | half2;  /* R1 G1 */
            tmp0 |= (half1 >> 11);		/* R1 G1 B1 */
#else //BGR
            rgb = half2 & 0xF8;  /* get 5 bits of R0 */
            tmp0 = (half1 >> 5) & 0x7E0;  /* get 6 bits of G0 */
            rgb = (rgb << 8) | tmp0;				/*  R0, G0 */
            half1 = (half1 >> 3) & 0x1F;		/* get 5 bits of B0 */
            rgb |= half1;			/*  R0,G0,B0 */

            half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */

            tmp0 = half1 & 0xF800;		/* get 5 bits of R1 */
            half1 = (half1 << 3) & 0x7C0;  /* get G1 in the middle */
            tmp0 |= half1;             /* R1 G1 */
            tmp0 |= (half2 >> 11);     /* R1 G1 B1 */
#endif
            rgb = tmp0 | (rgb << 16);

            *((uint32*)pDst) = rgb;
            pDst += 2;
            pSrc -= 10;

        }

        pSrc += deltaSrc;
        pDst += deltaDst;
    }

    return 1;
}


int32 CCRGB24toRGB16::rotate(uint8 *src, uint8 *dst, DisplayProperties *disp)
{
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
// To be implemented
    return 1;
}

#endif // CCROTATE

int32 scaledown(uint8*src, uint8 *dst, int32 *disp_prop, uint8 *_mRowPix, uint8 *_mColPix);
int32 scaleup(uint8*src, uint8 *dst, int32 *disp_prop, uint8 *_mRowPix, uint8 *_mColPix);

int32 CCRGB24toRGB16::zoom(uint8 *src, uint8 *dst, DisplayProperties *disp)
{
#if (!CCSCALING)
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    return 0;
#else
    int32 disp_prop[8];
    int32 src_width, src_height, dst_width, dst_height;

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = src_width = disp->src_width;
    disp_prop[3] = src_height = disp->src_height;
    disp_prop[4] = dst_width = disp->dst_width;
    disp_prop[5] = dst_height = disp->dst_height;
    disp_prop[6] = (_mRotation > 0 ? 1 : 0);
    disp_prop[7] = _mIsFlip;

    if (src_width > dst_width) /* scale down in width */
    {
        return scaledown(src, dst, disp_prop, _mRowPix, _mColPix);
    }
    else
    {
        return scaleup(src, dst, disp_prop, _mRowPix, _mColPix);
    }

    return 1;
#endif // CCSCALING
}

#if CCSCALING
int32 scaledown(uint8*src, uint8 *dst, int32 *disp, uint8 *_mRowPix, uint8 *_mColPix)
{
    uint8	*pSrc;
    uint16	*pDst;
    int32	src_pitch, dst_pitch, src_width, src_height, dst_width;
    int32	half1, half2;
    int32	deltaSrc;
    int32	row, col;
    uint32	rgb, R0, G0, B0, R1, G1, B1, R2, G2, B2;
    int		temp, denom, row_interp;
    int		 mIsFlip;
    uint8 *rowpix, *colpix;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    src_height  =   disp[3];
    dst_width   =   disp[4];
    mIsFlip		=   disp[7];

    if ((disp[6] ^ mIsFlip) == 1) /* rotate 180 and  no flip || rotate 0 and  with flip */
    {
        if (disp[6] == 1) /*rotate 180 and  no flip */
        {
            temp = src_pitch * src_height * 3;
            pSrc = (src + temp - 6);
            deltaSrc = (src_width - src_pitch) * 3;
        }
        else /* rotate 0 and  with flip */
        {
            pSrc = (src + src_width * 3 - 6);
            deltaSrc = (src_width + src_pitch) * 3;
        }
        pDst =	(uint16 *)dst;
        colpix = _mColPix + src_height - 1;
        row_interp = 0;

        for (row = src_height; row > 0; row--)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */

            /*	if(*colpix--==0)
            	{
            		pSrc+=src_width*3;
            		pSrc+=deltaSrc;
            		continue;
            	}*/

            rowpix = _mRowPix + src_width - 1;
            denom = 1;
            R2 = G2 = B2 = 0;

            for (col = src_width; col > 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
                half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
                R1 = (half1 >> 3) & 0x1F;
                G1 = (half1 >> 10);
                B1 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */
                R0 = half2 >> 11;
                G0 = (half1 >> 2) & 0x3F;
                B0 = half1 >> 11;
#else
                B1 = (half1 >> 3) & 0x1F;
                G1 = (half1 >> 10);
                R1 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */
                B0 = half2 >> 11;
                G0 = (half1 >> 2) & 0x3F;
                R0 = half1 >> 11;
#endif
                pSrc -= 10;

                temp = *rowpix--;

                R2 += R0;
                G2 += G0;
                B2 += B0;
                if (temp)
                {
                    R2 = (R2 + (denom / 2)) / denom;
                    G2 = (G2 + (denom / 2)) / denom;
                    B2 = (B2 + (denom / 2)) / denom;
                    // re-use R0,G0,B0,half1, do row interpolation
                    if (row_interp)
                    {
                        half1 = *pDst;
                        R0 = half1 >> 11;
                        G0 = (half1 >> 5) & 0x3F;
                        B0 = half1 & 0x1F;
                        R2 = (R2 + R0 + 1) >> 1;
                        G2 = (G2 + G0 + 1) >> 1;
                        B2 = (B2 + B0 + 1) >> 1;
                    }
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    R2 = G2 = B2 = 0;
                    denom = 1;
                    *pDst++ = rgb;
                }
                else
                {
                    denom++;
                }

                temp = *rowpix--;
                R2 += R1;
                G2 += G1;
                B2 += B1;
                if (temp)
                {
                    R2 = (R2 + (denom / 2)) / denom;
                    G2 = (G2 + (denom / 2)) / denom;
                    B2 = (B2 + (denom / 2)) / denom;
                    // re-use R0,G0,B0,half1, do row interpolation
                    if (row_interp)
                    {
                        half1 = *pDst;
                        R0 = half1 >> 11;
                        G0 = (half1 >> 5) & 0x3F;
                        B0 = half1 & 0x1F;
                        R2 = (R2 + R0 + 1) >> 1;
                        G2 = (G2 + G0 + 1) >> 1;
                        B2 = (B2 + B0 + 1) >> 1;
                    }
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    R2 = G2 = B2 = 0;
                    denom = 1;
                    *pDst++ = rgb;
                }
                else
                {
                    denom++;
                }
            }
            if (*colpix-- == 1)
            {
                pDst += (dst_pitch - dst_width);
                row_interp = 0;
            }
            else
            {
                pDst -= dst_width;
                row_interp = 1;
            }
            pSrc += deltaSrc;
        }
    }
    else
    {
        if (disp[6] == 1) /* rotate 180 and  with flip */
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaSrc = src_pitch * (src_height - 1);
            pSrc = src + deltaSrc * 3;
            deltaSrc = -(src_pitch + src_width) * 3;
        }
        else
        {	// only scale down, no rotation ,no flip
            deltaSrc	=	(src_pitch - src_width) * 3;
            pSrc = src;
        }

        pDst =	(uint16 *)dst;
        colpix = _mColPix + src_height - 1;
        row_interp = 0;

        for (row = src_height; row > 0; row--)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */

            /*	if(*colpix--==0)
            	{
            		pSrc+=src_width*3;
            		pSrc+=deltaSrc;
            		continue;
            	}*/

            rowpix = _mRowPix + src_width - 1;
            denom = 1;
            R2 = G2 = B2 = 0;

            for (col = src_width; col > 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
                half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
                R0 = (half1 >> 3) & 0x1F;
                G0 = (half1 >> 10);
                B0 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */
                R1 = half2 >> 11;
                G1 = (half1 >> 2) & 0x3F;
                B1 = half1 >> 11;
#else
                B0 = (half1 >> 3) & 0x1F;
                G0 = (half1 >> 10);
                R0 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */
                B1 = half2 >> 11;
                G1 = (half1 >> 2) & 0x3F;
                R1 = half1 >> 11;
#endif
                pSrc += 2;

                temp = *rowpix--;

                R2 += R0;
                G2 += G0;
                B2 += B0;
                if (temp)
                {
                    R2 = (R2 + (denom / 2)) / denom;
                    G2 = (G2 + (denom / 2)) / denom;
                    B2 = (B2 + (denom / 2)) / denom;
                    // re-use R0,G0,B0,half1, do row interpolation
                    if (row_interp)
                    {
                        half1 = *pDst;
                        R0 = half1 >> 11;
                        G0 = (half1 >> 5) & 0x3F;
                        B0 = half1 & 0x1F;
                        R2 = (R2 + R0 + 1) >> 1;
                        G2 = (G2 + G0 + 1) >> 1;
                        B2 = (B2 + B0 + 1) >> 1;
                    }
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    R2 = G2 = B2 = 0;
                    denom = 1;
                    *pDst++ = rgb;
                }
                else
                {
                    denom++;
                }

                temp = *rowpix--;
                R2 += R1;
                G2 += G1;
                B2 += B1;
                if (temp)
                {
                    R2 = (R2 + (denom / 2)) / denom;
                    G2 = (G2 + (denom / 2)) / denom;
                    B2 = (B2 + (denom / 2)) / denom;
                    // re-use R0,G0,B0,half1, do row interpolation
                    if (row_interp)
                    {
                        half1 = *pDst;
                        R0 = half1 >> 11;
                        G0 = (half1 >> 5) & 0x3F;
                        B0 = half1 & 0x1F;
                        R2 = (R2 + R0 + 1) >> 1;
                        G2 = (G2 + G0 + 1) >> 1;
                        B2 = (B2 + B0 + 1) >> 1;
                    }
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    R2 = G2 = B2 = 0;
                    denom = 1;
                    *pDst++ = rgb;
                }
                else
                {
                    denom++;
                }
            }
            if (*colpix-- == 1)
            {
                pDst += (dst_pitch - dst_width);
                row_interp = 0;
            }
            else
            {
                pDst -= dst_width;
                row_interp = 1;
            }
            pSrc += deltaSrc;
        }
    }

    return 1;
}

int32 scaleup(uint8*src, uint8 *dst, int32 *disp, uint8 *_mRowPix, uint8 *_mColPix)
{
    uint8	*pSrc = NULL;
    uint16	*pDst = NULL, *pTop = NULL, *pLine = NULL;
    int32	src_pitch, dst_pitch, src_width, src_height, dst_width;
    int32	half1, half2;
    int32	deltaSrc;
    int32	row, col;
    uint32	rgb, tmp, R0, G0, B0, R1, G1, B1, R2, G2, B2;
    int		temp;
    int	nextrow, mIsFlip;
    uint8 *rowpix, *colpix;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    src_height  =   disp[3];
    dst_width   =   disp[4];
    mIsFlip		=   disp[7];

    if (((disp[6] == 0) && (mIsFlip == 1)) || ((disp[6] == 1) && (mIsFlip == 0))) /* rotate 180 and  no flip || rotate 0 and  with flip */
    {
        if (disp[6] == 1) /*rotate 0 and  with flip */
        {
            nextrow = src_pitch * src_height * 3;
            pSrc = (src + nextrow - 6);
            deltaSrc = (src_width - src_pitch) * 3;
        }
        else /* rotate 180 and  no flip */
        {
            pSrc = (src + src_width * 3 - 6);
            deltaSrc = (src_width + src_pitch) * 3;
        }

        pDst =	(uint16 *)dst;
        colpix = _mColPix + disp[3] - 1;

        for (row = src_height; row > 0; row--)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */
            rowpix = _mRowPix + src_width - 1;

            for (col = src_width; col > 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
                half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
                R1 = (half1 >> 3) & 0x1F;
                G1 = (half1 >> 10);
                B1 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */
                R0 = half2 >> 11;
                G0 = (half1 >> 2) & 0x3F;
                B0 = half1 >> 11;
#else
                B1 = (half1 >> 3) & 0x1F;
                G1 = (half1 >> 10);
                R1 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */
                B0 = half2 >> 11;
                G0 = (half1 >> 2) & 0x3F;
                R0 = half1 >> 11;
#endif
                pSrc -= 10;

                temp = rowpix[0] + rowpix[-1];
                rowpix -= 2;
                rgb = (R0 << 6) | G0;
                rgb = (rgb << 5) | B0;
                tmp = (R1 << 6) | G1;
                tmp = (tmp << 5) | B1;

                *pDst++ = rgb;
                if (temp == 3)
                {
                    R0 = (R0 + R1 + 1) >> 1;
                    G0 = (G0 + G1 + 1) >> 1;
                    B0 = (B0 + B1 + 1) >> 1;
                    rgb = (R0 << 6) | G0;
                    rgb = (rgb << 5) | B0;
                    *pDst++ = rgb;
                }
                else if (temp == 4)
                {
                    R2 = (2 * R0 + R1 + 1) / 3;
                    R1 = (2 * R1 + R0 + 1) / 3;
                    G2 = (2 * G0 + G1 + 1) / 3;
                    G1 = (2 * G1 + G0 + 1) / 3;
                    rgb = (R2 << 6) | G2;
                    B2 = (2 * B0 + B1 + 1) / 3;
                    B1 = (2 * B1 + B0 + 1) / 3;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    rgb = (R1 << 6) | G1;
                    rgb = (rgb << 5) | B1;
                    *pDst++ = rgb;
                }
                else if (temp == 5)
                {
                    R2 = (3 * R0 + R1 + 2) / 4;
                    G2 = (3 * G0 + G1 + 2) / 4;
                    B2 = (3 * B0 + B1 + 2) / 4;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + R1 + 1) / 2;
                    G2 = (G0 + G1 + 1) / 2;
                    B2 = (B0 + B1 + 1) / 2;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + 3 * R1 + 2) / 4;
                    G2 = (G0 + 3 * G1 + 2) / 4;
                    B2 = (B0 + 3 * B1 + 2) / 4;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                }
                else if (temp == 6)
                {
                    R2 = (4 * R0 + R1 + 2) / 5;
                    G2 = (4 * G0 + G1 + 2) / 5;
                    B2 = (4 * B0 + B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (3 * R0 + 2 * R1 + 2) / 5;
                    G2 = (3 * G0 + 2 * G1 + 2) / 5;
                    B2 = (3 * B0 + 2 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (2 * R0 + 3 * R1 + 2) / 5;
                    G2 = (2 * G0 + 3 * G1 + 2) / 5;
                    B2 = (2 * B0 + 3 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + 4 * R1 + 2) / 5;
                    G2 = (G0 + 4 * G1 + 2) / 5;
                    B2 = (B0 + 4 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                }

                *pDst++ = tmp;
            }

            if (!(row&1)) /* even row */
            {
                // have to leave some space for row interpolation
                pDst -= dst_width;
                pTop = pDst;
                pDst += dst_pitch * (colpix[0] + colpix[-1] - 1);
                colpix--;
            }
            else /* odd row */ // do the row interpolation
            {
                temp = colpix[0] + colpix[1];
                pDst -= dst_width; // back to the beginning of bottom line

                if (temp != 2) // need to interpolate
                {
                    pLine = pDst - dst_pitch;
                    for (col = dst_width; col > 0; col--)
                    {
                        rgb = *pTop++;
                        tmp = *pDst++;
                        // now averager rgb and tmp
                        R0 = rgb >> 11;
                        R1 = tmp >> 11;
                        G0 = (rgb >> 5) & 0x3F;
                        G1 = (tmp >> 5) & 0x3F;
                        B0 = rgb & 0x1F;
                        B1 = tmp & 0x1F;
                        if (temp == 3)
                        {
                            R2 = (R0 + R1 + 1) >> 1;
                            G2 = (G0 + G1 + 1) >> 1;
                            B2 = (B0 + B1 + 1) >> 1;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                        else if (temp == 4)
                        {
                            R2 = (2 * R0 + R1 + 1) / 3;
                            R1 = (2 * R1 + R0 + 1) / 3;
                            G2 = (2 * G0 + G1 + 1) / 3;
                            G1 = (2 * G1 + G0 + 1) / 3;
                            rgb = (R2 << 6) | G2;
                            B2 = (2 * B0 + B1 + 1) / 3;
                            B1 = (2 * B1 + B0 + 1) / 3;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb; // line above
                            rgb = (R1 << 6) | G1;
                            rgb = (rgb << 5) | B1;
                            *pLine++ = rgb;
                        }
                        else if (temp == 5)
                        {
                            R2 = (3 * R0 + R1 + 2) / 4;
                            G2 = (3 * G0 + G1 + 2) / 4;
                            B2 = (3 * B0 + B1 + 2) / 4;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-(dst_pitch<<1)] = rgb;
                            R2 = (R0 + R1 + 1) / 2;
                            G2 = (G0 + G1 + 1) / 2;
                            B2 = (B0 + B1 + 1) / 2;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb;
                            R2 = (R0 + 3 * R1 + 2) / 4;
                            G2 = (G0 + 3 * G1 + 2) / 4;
                            B2 = (B0 + 3 * B1 + 2) / 4;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                        else if (temp == 6)
                        {
                            R2 = (4 * R0 + R1 + 2) / 5;
                            G2 = (4 * G0 + G1 + 2) / 5;
                            B2 = (4 * B0 + B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch*3] = rgb;
                            R2 = (3 * R0 + 2 * R1 + 2) / 5;
                            G2 = (3 * G0 + 2 * G1 + 2) / 5;
                            B2 = (3 * B0 + 2 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-(dst_pitch<<1)] = rgb;
                            R2 = (2 * R0 + 3 * R1 + 2) / 5;
                            G2 = (2 * G0 + 3 * G1 + 2) / 5;
                            B2 = (2 * B0 + 3 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb;
                            R2 = (R0 + 4 * R1 + 2) / 5;
                            G2 = (G0 + 4 * G1 + 2) / 5;
                            B2 = (B0 + 4 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                    }
                    pDst -= dst_width;
                }
                pDst += dst_pitch;
                colpix--;
            }

            pSrc += deltaSrc;
        }
    }
    else  /* rotate 180 and  with flip || no rotation ,no flip */
    {
        if (disp[6] == 1) /* rotate 180 and  with flip */
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaSrc = src_pitch * (src_height - 1);
            pSrc = src + deltaSrc * 3;
            deltaSrc = -(src_pitch + src_width) * 3;
        }
        else
        {	// only scale up, no rotation ,no flip
            deltaSrc	=	(src_pitch - src_width) * 3;
            pSrc = src;
        }

        pDst =	(uint16 *)dst;
        colpix = _mColPix + src_height - 1;

        for (row = src_height; row > 0; row--)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */
            rowpix = _mRowPix + src_width - 1;

            for (col = src_width; col > 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                half1 = *((uint16*)(pSrc));  /* read 2 bytes, G0 R0 or G0 B0 */
                half2 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 B0 or B1 R0 */

#if RGB_FORMAT
                R0 = (half1 >> 3) & 0x1F;
                G0 = (half1 >> 10);
                B0 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, B1 G1 */
                R1 = half2 >> 11;
                G1 = (half1 >> 2) & 0x3F;
                B1 = half1 >> 11;
#else
                B0 = (half1 >> 3) & 0x1F;
                G0 = (half1 >> 10);
                R0 = (half2 >> 3) & 0x1F;
                half1 = *((uint16*)(pSrc += 2)); /* read 2 bytes, R1 G1 */
                B1 = half2 >> 11;
                G1 = (half1 >> 2) & 0x3F;
                R1 = half1 >> 11;
#endif
                pSrc += 2;

                temp = rowpix[0] + rowpix[-1];
                rowpix -= 2;
                rgb = (R0 << 6) | G0;
                rgb = (rgb << 5) | B0;
                tmp = (R1 << 6) | G1;
                tmp = (tmp << 5) | B1;

                *pDst++ = rgb;
                if (temp == 3)
                {
                    R0 = (R0 + R1 + 1) >> 1;
                    G0 = (G0 + G1 + 1) >> 1;
                    B0 = (B0 + B1 + 1) >> 1;
                    rgb = (R0 << 6) | G0;
                    rgb = (rgb << 5) | B0;
                    *pDst++ = rgb;
                }
                else if (temp == 4)
                {
                    R2 = (2 * R0 + R1 + 1) / 3;
                    R1 = (2 * R1 + R0 + 1) / 3;
                    G2 = (2 * G0 + G1 + 1) / 3;
                    G1 = (2 * G1 + G0 + 1) / 3;
                    rgb = (R2 << 6) | G2;
                    B2 = (2 * B0 + B1 + 1) / 3;
                    B1 = (2 * B1 + B0 + 1) / 3;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    rgb = (R1 << 6) | G1;
                    rgb = (rgb << 5) | B1;
                    *pDst++ = rgb;
                }
                else if (temp == 5)
                {
                    R2 = (3 * R0 + R1 + 2) / 4;
                    G2 = (3 * G0 + G1 + 2) / 4;
                    B2 = (3 * B0 + B1 + 2) / 4;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + R1 + 1) / 2;
                    G2 = (G0 + G1 + 1) / 2;
                    B2 = (B0 + B1 + 1) / 2;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + 3 * R1 + 2) / 4;
                    G2 = (G0 + 3 * G1 + 2) / 4;
                    B2 = (B0 + 3 * B1 + 2) / 4;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                }
                else if (temp == 6)
                {
                    R2 = (4 * R0 + R1 + 2) / 5;
                    G2 = (4 * G0 + G1 + 2) / 5;
                    B2 = (4 * B0 + B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (3 * R0 + 2 * R1 + 2) / 5;
                    G2 = (3 * G0 + 2 * G1 + 2) / 5;
                    B2 = (3 * B0 + 2 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (2 * R0 + 3 * R1 + 2) / 5;
                    G2 = (2 * G0 + 3 * G1 + 2) / 5;
                    B2 = (2 * B0 + 3 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                    R2 = (R0 + 4 * R1 + 2) / 5;
                    G2 = (G0 + 4 * G1 + 2) / 5;
                    B2 = (B0 + 4 * B1 + 2) / 5;
                    rgb = (R2 << 6) | G2;
                    rgb = (rgb << 5) | B2;
                    *pDst++ = rgb;
                }

                *pDst++ = tmp;
            }

            if (!(row&1)) /* even row */
            {
                // have to leave some space for row interpolation
                pDst -= dst_width;
                pTop = pDst;
                pDst += dst_pitch * (colpix[0] + colpix[-1] - 1);
                colpix--;
            }
            else /* odd row */ // do the row interpolation
            {
                temp = colpix[0] + colpix[1];
                pDst -= dst_width; // back to the beginning of bottom line

                if (temp != 2) // need to interpolate
                {
                    pLine = pDst - dst_pitch;
                    for (col = dst_width; col > 0; col--)
                    {
                        rgb = *pTop++;
                        tmp = *pDst++;
                        // now averager rgb and tmp
                        R0 = rgb >> 11;
                        R1 = tmp >> 11;
                        G0 = (rgb >> 5) & 0x3F;
                        G1 = (tmp >> 5) & 0x3F;
                        B0 = rgb & 0x1F;
                        B1 = tmp & 0x1F;
                        if (temp == 3)
                        {
                            R2 = (R0 + R1 + 1) >> 1;
                            G2 = (G0 + G1 + 1) >> 1;
                            B2 = (B0 + B1 + 1) >> 1;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                        else if (temp == 4)
                        {
                            R2 = (2 * R0 + R1 + 1) / 3;
                            R1 = (2 * R1 + R0 + 1) / 3;
                            G2 = (2 * G0 + G1 + 1) / 3;
                            G1 = (2 * G1 + G0 + 1) / 3;
                            rgb = (R2 << 6) | G2;
                            B2 = (2 * B0 + B1 + 1) / 3;
                            B1 = (2 * B1 + B0 + 1) / 3;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb; // line above
                            rgb = (R1 << 6) | G1;
                            rgb = (rgb << 5) | B1;
                            *pLine++ = rgb;
                        }
                        else if (temp == 5)
                        {
                            R2 = (3 * R0 + R1 + 2) / 4;
                            G2 = (3 * G0 + G1 + 2) / 4;
                            B2 = (3 * B0 + B1 + 2) / 4;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-(dst_pitch<<1)] = rgb;
                            R2 = (R0 + R1 + 1) / 2;
                            G2 = (G0 + G1 + 1) / 2;
                            B2 = (B0 + B1 + 1) / 2;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb;
                            R2 = (R0 + 3 * R1 + 2) / 4;
                            G2 = (G0 + 3 * G1 + 2) / 4;
                            B2 = (B0 + 3 * B1 + 2) / 4;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                        else if (temp == 6)
                        {
                            R2 = (4 * R0 + R1 + 2) / 5;
                            G2 = (4 * G0 + G1 + 2) / 5;
                            B2 = (4 * B0 + B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch*3] = rgb;
                            R2 = (3 * R0 + 2 * R1 + 2) / 5;
                            G2 = (3 * G0 + 2 * G1 + 2) / 5;
                            B2 = (3 * B0 + 2 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-(dst_pitch<<1)] = rgb;
                            R2 = (2 * R0 + 3 * R1 + 2) / 5;
                            G2 = (2 * G0 + 3 * G1 + 2) / 5;
                            B2 = (2 * B0 + 3 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            pLine[-dst_pitch] = rgb;
                            R2 = (R0 + 4 * R1 + 2) / 5;
                            G2 = (G0 + 4 * G1 + 2) / 5;
                            B2 = (B0 + 4 * B1 + 2) / 5;
                            rgb = (R2 << 6) | G2;
                            rgb = (rgb << 5) | B2;
                            *pLine++ = rgb;
                        }
                    }
                    pDst -= dst_width;
                }
                pDst += dst_pitch;
                colpix--;
            }

            pSrc += deltaSrc;
        }
    }
    return 1;
}
#endif //CCSCALING

int32 CCRGB24toRGB16::zoomRotate(uint8 *src, uint8 *dst, DisplayProperties *disp)
{
    // To be implemented
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    return 1;
}



