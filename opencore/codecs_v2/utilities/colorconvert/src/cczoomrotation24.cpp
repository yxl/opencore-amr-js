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
//                                                                              //
//  File: cczoomrotation24.cpp                                                //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
/** Class ColorConvert24, YUV to RGB24 bit, 8bit per component. */
#include "cczoomrotation24.h"



OSCL_EXPORT_REF ColorConvertBase* ColorConvert24::NewL(void)
{
    ColorConvert24* self = OSCL_NEW(ColorConvert24, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


ColorConvert24::ColorConvert24()
{
    mClip = ((uint8*)mCoefTbl32) + 400;
}


OSCL_EXPORT_REF ColorConvert24::~ColorConvert24()
{
}


int32 ColorConvert24::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    _mInitialized = false;
    if (Src_width&0x3) return 0; // must be multiple of 4.

    //local init
    _mInitialized = true;

    //default no zoom
    SetMode(0); // called after init
    SetYuvFullRange(false);

    return 1;
}


int32 ColorConvert24::SetYuvFullRange(bool range)
{
    OSCL_ASSERT(_mInitialized == true);

    _mYuvRange = range;

    if (_mYuvRange == false)
    {
        *((uint32*)(mClip - 400)) = 0x0000b2ce; //65536*0.813/1.164;
        *((uint32*)(mClip - 396)) =  0x00015f03; //65536*1.596/1.164;
        *((uint32*)(mClip - 392)) =  0x000055fe; //65536*0.391/1.164;
        *((uint32*)(mClip - 388)) =  0x0001bbd2; //65536*2.018/1.164;

        int32 tmp;
        for (int32 i = -384; i < 640; i++)
        {
            tmp = (int32)(1.164 * (i - 16));
            mClip[i] = (tmp < 0) ? 0 : ((tmp > 255) ? 255 : (uint8)tmp);
        }
    }
    else // full range 0-255
    {
        *((uint32*)(mClip - 400)) = (int)(65536 * 0.4681); //0.714);
        *((uint32*)(mClip - 396)) = (int)(65536 * 1.5748);//1.402);
        *((uint32*)(mClip - 392)) = (int)(65536 * 0.1873);//0.344);
        *((uint32*)(mClip - 388)) = (int)(65536 * 1.8556);//1.772);
        int32 tmp;
        for (int32 i = -384; i < 640; i++)
        {
            tmp = i;
            mClip[i] = (tmp < 0) ? 0 : ((tmp > 255) ? 255 : (uint8)tmp);
        }
    }

    return 1;
}

int32 ColorConvert24::SetMode(int32 nMode)	//iMode : 0 Off, 1 On
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 0)
    {
        mPtrYUV2RGB	=	&ColorConvert24::get_frame24;
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
        if (_mIsZoom)
        {
            if (_mRotation&0x1) /* zoom and rotate */
            {
                mPtrYUV2RGB = &ColorConvert24::cc24ZoomRotate;
            }
            else /* zoom only */
            {
                mPtrYUV2RGB	=	&ColorConvert24::cc24ZoomIn;
            }
        }
        else
        {
            if (_mRotation&0x1) /* rotate only*/
            {
                mPtrYUV2RGB = &ColorConvert24::cc24Rotate;
            }
            else /* no zoom, no rotate, SetMode(1) = SetMode(0) */
            {
                mPtrYUV2RGB	=	&ColorConvert24::get_frame24;
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

int32 ColorConvert24::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);
    // for zoom, need extra line of RGB buffer for processing otherwise memory will corrupt.
    if (_mIsZoom)
    {
        return	_mState ? ((_mDst_height + 1)*_mDst_pitch*3) : (_mSrc_width*_mSrc_height*3);
    }
    else
    {
        return	_mState ? ((_mDst_height)*_mDst_pitch*3) : (_mSrc_width*_mSrc_height*3);
    }
}


int32 ColorConvert24::Convert(uint8 **yuvBuf, uint8 *rgbBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(yuvBuf[0]);
    OSCL_ASSERT(yuvBuf[1]);
    OSCL_ASSERT(yuvBuf[2]);
    OSCL_ASSERT(rgbBuf);


    if (((uint)rgbBuf)&0x3 || ((uint)yuvBuf[0])&0x3) /* address is not word align */
    {
        return 0;
    }

    (*this.*mPtrYUV2RGB)(yuvBuf, rgbBuf, &_mDisp, (uint8 *)mClip);

    return 1;
}


int32 ColorConvert24::Convert(uint8 *yuvBuf, uint8 *rgbBuf)
{
    //this conversion will cause problems when do src clipping. However, if they want, they must give more info
    uint8 *TmpYuvBuf[3];

    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(rgbBuf);

    if (((uint)rgbBuf)&0x3 || ((uint)yuvBuf)&0x3) /* address is not word align */
    {
        return 0;
    }

    TmpYuvBuf[0]	=	yuvBuf;
    TmpYuvBuf[1]	=	yuvBuf + (_mSrc_pitch) * (_mSrc_mheight);
    TmpYuvBuf[2]	=	TmpYuvBuf[1] + (_mSrc_pitch * _mSrc_mheight) / 4;
    (*this.*mPtrYUV2RGB)(TmpYuvBuf, rgbBuf, &_mDisp, (uint8 *)mClip);

    return 1;
}

int32 cc24(uint8 **src, uint8 *dst, int32 *disp_prop, uint8 *coeff_tbl);

int32 ColorConvert24::get_frame24(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *clip)
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

//	if(disp_prop[6]^disp_prop[7])	/* flip and rotate 180*/
//	{
//		return 0 ;//not yet implemented cc24Reverse(src,dst,disp_prop,coff_tbl);
//	}
//	else
    {
        return cc24(src, dst, disp_prop, clip);
    }
}



int32 cc24(uint8 **src, uint8 *dst, int32 *disp, uint8 *clip)
{
    uint8 *pCb, *pCr;
    uint16 *pY;
    uint8	*pDst;
    int32	src_pitch, dst_pitch, dst_pitch3, src_width;
    int32	Y, Cb, Cr, Cg;
    int32	deltaY, deltaDst, deltaCbCr;
    int32	row, col;
    int32	tmp0, tmp1, tmp2, rgb, rgb2;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (disp[6]) /* rotate 180 and flip */
    {	/* move the starting point to the bottom-left corner of the picture */
        deltaY = src_pitch * (disp[3] - 1);
        pY = (uint16*)(src[0] + deltaY);
        deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
        pCb = src[1] + deltaY;
        pCr = src[2] + deltaY;
        deltaY = -src_width - (src_pitch << 1);
        deltaCbCr = -((src_width + src_pitch) >> 1);
        src_pitch = -(src_pitch >> 1);
    }
    else
    {
        deltaY		=	(src_pitch << 1) - src_width;
        deltaCbCr	=	(src_pitch - src_width) >> 1;
        pY = (uint16 *)src[0];
        pCb = src[1];
        pCr = src[2];
        src_pitch >>= 1;
    }

    deltaDst	=	((dst_pitch << 1) - src_width) * 3;
    pDst =	dst;

    dst_pitch3	=	3 * dst_pitch;

    for (row = disp[3]; row > 0; row -= 2)
    {

        for (col = src_width; col > 0; col -= 4)
        {

            Cb = *pCb++;
            Cr = *pCr++;

            //load the bottom two pixels
            //Y	=	*(((uint16 *)pY)+src_pitch);
            Y	=	pY[src_pitch];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            tmp0	=	(Y << 16) & 0xFF0000;	//Low endian	left pixel
            tmp1	=	tmp0 - Cg;
            tmp2	=	tmp0 + Cb;
            tmp0	=	tmp0 + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            rgb	=	tmp0 | (tmp1 << 8); /* r1 & g1 */
#else
            rgb	=	tmp2 | (tmp1 << 8); /* b1 & g1 */
#endif
            Y   = (Y << 8) & 0xFF0000;
#if RGB_FORMAT
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
            rgb		|= (tmp0 << 24);
            rgb		|= (tmp2 << 16); // tmp2 | (tmp0<<8); /* b1 & r2 */
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
            rgb		|= (tmp2 << 24);
            rgb		|= (tmp0 << 16); //tmp0 | (tmp2<<8); /* r1 & b2 */
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
#endif
            *((uint32 *)(pDst + dst_pitch3))	=   rgb;

            tmp1	=	Y - Cg;
            tmp1	=	clip[tmp1>>16];
#if RGB_FORMAT
            rgb2	=	tmp1 | (tmp2 << 8); /* g2 & b2 */
#else
            rgb2	=	tmp1 | (tmp0 << 8); /* g2 & r2 */
#endif
            //load the top two pixels
            Y	=	*pY++;

            tmp0	=	(Y << 16) & 0xFF0000;	//Low endian	left pixel
            tmp1	=	tmp0 - Cg;
            tmp2	=	tmp0 + Cb;
            tmp0	=	tmp0 + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            rgb	=	tmp0 | (tmp1 << 8); /* r1 & g1 */
#else
            rgb	=	tmp2 | (tmp1 << 8); /* b1 & g1 */
#endif
            Y   = (Y << 8) & 0xFF0000;

#if RGB_FORMAT
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
            rgb     |= (tmp0 << 24);
            rgb		|= (tmp2 << 16); //tmp2 | (tmp0<<8); /* r1 & b2 */
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
            rgb		|= (tmp2 << 24);
            rgb		|= (tmp0 << 16); //tmp0 | (tmp2<<8); /* b1 & r2 */
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
#endif
            *((uint32 *)(pDst))	=   rgb;

            tmp1	=	Y - Cg;
            tmp1	=	clip[tmp1>>16];

#if RGB_FORMAT
            rgb	=	tmp1 | (tmp2 << 8); /* g2 & b2 */
#else
            rgb	=	tmp1 | (tmp0 << 8); /* g2 & r2 */
#endif

            //// next 2x2 pixels

            Cb = *pCb++;
            Cr = *pCr++;

            //load the bottom two pixels
            //Y	=	*(((uint16 *)pY)+src_pitch);
            Y	=	pY[src_pitch];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            tmp0	=	(Y << 16) & 0xFF0000;	//Low endian	left pixel
            tmp1	=	tmp0 - Cg;
            tmp2	=	tmp0 + Cb;
            tmp0	=	tmp0 + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            rgb2	|= (tmp1 << 24);
            rgb2	|= (tmp0 << 16);
            //rgb	=	tmp0 | (tmp1<<8); /* r3 & g3 */
#else
            rgb2	|= (tmp1 << 24);
            rgb2	|= (tmp2 << 16);
            //rgb	=	tmp2 | (tmp1<<8); /* b3 & g3 */
#endif
            *((uint32 *)(pDst + dst_pitch3 + 4)) = rgb2;

            Y   = (Y << 8) & 0xFF0000;
#if RGB_FORMAT
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
            rgb2	=	tmp2 | (tmp0 << 8); /* b3 & r4 */
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
            rgb2	=	tmp0 | (tmp2 << 8); /* r3 & b4 */
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
#endif
            tmp1	=	Y - Cg;
            tmp1	=	clip[tmp1>>16];
#if RGB_FORMAT
            rgb2	|= (tmp2 << 24);
            rgb2	|= (tmp1 << 16); //	tmp1|(tmp2<<8); /* g2 & b2 */
#else
            rgb2	|= (tmp0 << 24);
            rgb2	|= (tmp1 << 16); // tmp1|(tmp0<<8); /* g2 & r2 */
#endif
            *((uint32 *)(pDst + dst_pitch3 + 8)) = rgb2;

            //load the top two pixels
            Y	=	*pY++;

            tmp0	=	(Y << 16) & 0xFF0000;	//Low endian	left pixel
            tmp1	=	tmp0 - Cg;
            tmp2	=	tmp0 + Cb;
            tmp0	=	tmp0 + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            rgb		|=	(tmp1 << 24);
            rgb		|=	(tmp0 << 16);//tmp0 | (tmp1<<8); /* r1 & g1 */
#else
            rgb		|=	(tmp1 << 24);
            rgb		|=	(tmp2 << 16);//tmp2 | (tmp1<<8); /* b1 & g1 */
#endif
            *((uint32*)(pDst + 4)) = rgb;

            Y   = (Y << 8) & 0xFF0000;

#if RGB_FORMAT
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
            rgb		=	tmp2 | (tmp0 << 8); /* r1 & b2 */
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	Y + Cb;
            tmp2	=	clip[tmp2>>16];
            rgb		=	tmp0 | (tmp2 << 8); /* b1 & r2 */
            tmp0	=	Y + Cr;
            tmp0	=	clip[tmp0>>16];
#endif

            tmp1	=	Y - Cg;
            tmp1	=	clip[tmp1>>16];

#if RGB_FORMAT
            rgb		|=	(tmp2 << 24);
            rgb		|=	(tmp1 << 16); //tmp1|(tmp2<<8); /* g2 & b2 */
#else
            rgb		|=	(tmp0 << 24);
            rgb		|=	(tmp1 << 16); //tmp1|(tmp0<<8); /* g2 & r2 */
#endif
            *((uint32*)(pDst + 8)) = rgb;

            pDst += 12;
        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }

    return 1;
}




int32 ColorConvert24::cc24Rotate(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *clip)
{
#ifdef CCROTATE
    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32		src_pitch, dst_pitch, src_width, dst_pitch3;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaDst, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    int32 half_src_pitch, read_idx, tmp_src_pitch;

    src_pitch	=	disp->src_pitch;
    dst_pitch	=	disp->dst_pitch;
    src_width	=	disp->src_width;

    deltaDst	=	3 * ((dst_pitch << 1) - disp->dst_width); // 3*((dst_pitch<<1) - src_height)

    if (_mRotation == CCROTATE_CLKWISE)
    {	// go from top-left to bottom-left
        deltaY		=  src_pitch * disp->src_height + 2;
        deltaCbCr	= ((src_pitch * disp->src_height) >> 2) + 1;
    }
    else  // rotate counterclockwise
    {   // go from bottom-right back to top-right
        deltaY		=  -(src_pitch * disp->src_height + 2);
        deltaCbCr	=  -(((src_pitch * disp->src_height) >> 2) + 1);
    }

    // map origin of the destination to the source
    if (_mRotation == CCROTATE_CLKWISE)
    {	// goto bottom-left
        pY = src[0] + src_pitch * (disp->src_height - 1);
        pCb = src[1] + ((src_pitch >> 1) * ((disp->src_height >> 1) - 1));
        pCr = src[2] + ((src_pitch >> 1) * ((disp->src_height >> 1) - 1));
    }
    else  // rotate counterclockwise
    {   // goto top-right
        pY = src[0] + src_width - 1;
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;
    }

    if (_mRotation == CCROTATE_CLKWISE)
    {
        half_src_pitch = -(src_pitch >> 1);
        read_idx = 1;
        tmp_src_pitch = -src_pitch;
    }
    else // rotate counterclockwise
    {
        half_src_pitch = (src_pitch >> 1);
        read_idx = -1;
        tmp_src_pitch = src_pitch;
    }

    pDst =	dst;

    dst_pitch3	=	3 * dst_pitch;

    for (row = src_width; row > 0; row -= 2)
    {

        for (col = disp->src_height - 1; col >= 0; col -= 2)
        {

            Cb = *pCb;
            pCb += half_src_pitch;
            Cr = *pCr;
            pCr += half_src_pitch;

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            //process the bottom two pixels in RGB plane
            tmp0	=	pY[read_idx]; /* top-left pixel */

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            *((uint16 *)(pDst + dst_pitch3))	=	tmp0 | (tmp1 << 8); /* r1 & g1 */
#else
            *((uint16 *)(pDst + dst_pitch3))	=	tmp2 | (tmp1 << 8); /* b1 & g1 */
#endif
            Y		=	pY[read_idx+tmp_src_pitch];  /* bottom-left pixel */
#if RGB_FORMAT
            tmp0	=	(Y << 16) + Cr;
            tmp0	=	clip[tmp0>>16];
            *((uint16 *)(pDst + dst_pitch3 + 2))	=   tmp2 | (tmp0 << 8); /* b1 & r2 */
            tmp2	=	(Y << 16) + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	(Y << 16) + Cb;
            tmp2	=	clip[tmp2>>16];
            *((uint16 *)(pDst + dst_pitch3 + 2))	=   tmp0 | (tmp2 << 8); /* r1 & b2 */
            tmp0	=	(Y << 16) + Cr;
            tmp0	=	clip[tmp0>>16];
#endif
            tmp1	=	(Y << 16) - Cg;
            tmp1	=	clip[tmp1>>16];

#if RGB_FORMAT
            *((uint16*)(pDst + dst_pitch3 + 4))	=	tmp1 | (tmp2 << 8); /* g2 & b2 */
#else
            *((uint16*)(pDst + dst_pitch3 + 4))	=	tmp1 | (tmp0 << 8); /* g2 & r2 */
#endif
            //process the top two pixels in RGB plane

            tmp0	=	*pY; /*upper-right pixel */
            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            *((uint16 *)pDst)	=	tmp0 | (tmp1 << 8); /* r1 & g1 */
#else
            *((uint16 *)pDst)	=	tmp2 | (tmp1 << 8); /* b1 & g1 */
#endif
            Y		=	pY[tmp_src_pitch]; /* bottom-right pixel */
            pY += (tmp_src_pitch << 1);
#if RGB_FORMAT
            tmp0	=	(Y << 16) + Cr;
            tmp0	=	clip[tmp0>>16];
            *((uint16 *)(pDst + 2))	=   tmp2 | (tmp0 << 8); /* b1 & r2 */
            tmp2	=	(Y << 16) + Cb;
            tmp2	=	clip[tmp2>>16];
#else
            tmp2	=	(Y << 16) + Cb;
            tmp2	=	clip[tmp2>>16];
            *((uint16 *)(pDst + 2))	=   tmp0 | (tmp2 << 8); /* r1 & b2 */
            tmp0	=	(Y << 16) + Cr;
            tmp0	=	clip[tmp0>>16];
#endif
            tmp1	=	(Y << 16) - Cg;
            tmp1	=	clip[tmp1>>16];
#if RGB_FORMAT
            *((uint16*)(pDst + 4))	=	tmp1 | (tmp2 << 8); /* g2 & r2 */
#else
            *((uint16*)(pDst + 4))	=	tmp1 | (tmp0 << 8); /* g2 & b2 */
#endif
            pDst += 6;

        }//end of COL

        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);
    }

    return 1;
#else
    return 0;
#endif // CCROTATE
}


// platform specific C function
int32 cc24scaling(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip,
                  uint8 *_mRowPix, uint8 *_mColPix);

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 ColorConvert24::cc24ZoomIn(uint8 **src, uint8 *dst,
                                 DisplayProperties *disp, uint8 *clip)
{
    int32 disp_prop[6];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    return cc24scaling(src, dst, disp_prop, clip, _mRowPix, _mColPix);
}


int32 cc24scaling(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip,
                  uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, dst_pitch3, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, dst_width3, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 offset;

    src_pitch = disp[0];
    dst_pitch = disp[1];
    src_width = disp[2];

    deltaY = (src_pitch << 1) - src_width;
    deltaCbCr = (src_pitch - src_width) >> 1;
    dst_width3 = disp[4] * 3;

    pY = src[0];
    pCb = src[1];
    pCr = src[2];

    pDst = dst;

    dst_pitch3 = 3 * dst_pitch;

    for (row = disp[3] - 1; row >= 0; row -= 2)
    {/* decrement index, _mColPix[.] is
													 symmetric to increment index */
        if ((_mColPix[row-1] == 0) && (_mColPix[row] == 0))
        {
            pCb += (src_pitch >> 1);
            pCr += (src_pitch >> 1);
            pY += (src_pitch << 1);
            continue;
        }

        for (col = src_width - 2; col >= 0; col -= 2)
        { /* decrement index, _mRowPix[.] is
													 symmetric to increment index */

            Cb = *pCb++;
            Cr = *pCr++;

            //load the bottom two pixels
            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            if (_mRowPix[col])	/* compute this pixel */
            {
                tmp0	=	pY[src_pitch];			//bottom left

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16);
#else
                rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);
#endif
                Y	=	*pY++;						//upper left
                tmp1	=	(Y << 16) - Cg;
                tmp2	=	(Y << 16) + Cb;
                tmp0	=	(Y << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                tmp2 = ((tmp2 << 16) | (tmp1 << 8) | tmp0);
#else
                tmp2 |= ((tmp0 << 16) | (tmp1 << 8));
#endif
                if (((uint)pDst)&1) /* not word-aligned */
                {

                    *(pDst + dst_pitch3)	=	rgb & 0xFF; /* b */
                    *((uint16*)(pDst + dst_pitch3 + 1)) = rgb >> 8; /* gr */
                    *pDst = tmp2 & 0xFF;  /* b */
                    *((uint16*)(pDst + 1)) = tmp2 >> 8; /* gr */

                    if (_mRowPix[col] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 5) = rgb >> 16; /* r */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* bg */
                        *(pDst + 5) = tmp2 >> 16; /* r */

                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 5)) = (uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 7)) =   rgb >> 8;  /* gr */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 5)) = (uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 7)) =   tmp2 >> 8;  /* gr */
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gr */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gr */
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gr */
                        *((uint16*)(pDst + dst_pitch3 + 6)) = rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 8) = rgb >> 16; /* r */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gr */
                        *((uint16*)(pDst + 6)) = tmp2 & 0xFFFF; /* bg */
                        *(pDst + 8) = tmp2 >> 16; /* r */
                    }
                    else
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 2)	=	rgb >> 16; /* r */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *(pDst + 2)	=	tmp2 >> 16; /* r */
                    }
                }
            }
            else  /* if(_mRowPix[col]) */
            {
                pY++;
            }

            pDst += _mRowPix[col] + _mRowPix[col] + _mRowPix[col];

            if (_mRowPix[col+1]) 	/* compute this pixel */
            {
                //load the top two pixels
                tmp0	=	pY[src_pitch];		//bottom right

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16);
#else
                rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);
#endif
                Y	=	*pY++;							//upper right
                tmp1	=	(Y << 16) - Cg;
                tmp2	=	(Y << 16) + Cb;
                tmp0	=	(Y << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                tmp2 = ((tmp2 << 16) | (tmp1 << 8) | tmp0);
#else
                tmp2 |= ((tmp0 << 16) | (tmp1 << 8));
#endif
                if (((uint)pDst)&1) /* not word-aligned */
                {

                    *(pDst + dst_pitch3)	=	rgb & 0xFF; /* r */
                    *((uint16*)(pDst + dst_pitch3 + 1)) = rgb >> 8; /* gb */
                    *pDst = tmp2 & 0xFF;  /* r */
                    *((uint16*)(pDst + 1)) = rgb >> 8; /* gb */

                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 5) = rgb >> 16; /* b */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* rg */
                        *(pDst + 5) = tmp2 >> 16; /* b */

                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 5)) = (uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 7)) =   rgb >> 8;  /* gb */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 5)) = (uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 7)) =   tmp2 >> 8;  /* gb */
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gb */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gb */
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gb */
                        *((uint16*)(pDst + dst_pitch3 + 6)) = rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 8) = rgb >> 16; /* b */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gb */
                        *((uint16*)(pDst + 6)) = tmp2 & 0xFFFF; /* rg */
                        *(pDst + 8) = tmp2 >> 16; /* b */
                    }
                    else
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 2)	=	rgb >> 16; /* b */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *(pDst + 2)	=	tmp2 >> 16; /* b */
                    }
                }
            }
            else  /* if(_mRowPix[col+1]) */
            {
                pY++;
            }

            pDst	+= _mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1];
        }//end of COL

        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;

        pDst -=	(dst_width3);	//goes back to the beginning of the line;

        //copy down
        offset = (_mColPix[row] * dst_pitch3);

        if (_mColPix[row-1] && _mColPix[row] != 1)
        {
            oscl_memcpy(pDst + offset, pDst + dst_pitch3, dst_width3);
        }
        if (_mColPix[row-1] == 2)
        {
            oscl_memcpy(pDst + offset + dst_pitch3, pDst + dst_pitch3, dst_width3);
        }
        else if (_mColPix[row-1] == 3)
        {
            oscl_memcpy(pDst + offset + dst_pitch3, pDst + dst_pitch3, dst_width3);
            oscl_memcpy(pDst + offset + dst_pitch3*2, pDst + dst_pitch3, dst_width3);
        }

        //copy up
        if (_mColPix[row] == 2)
        {
            oscl_memcpy(pDst + dst_pitch3, pDst, dst_width3);
        }
        else if (_mColPix[row] == 3)
        {
            oscl_memcpy(pDst + dst_pitch3, pDst, dst_width3);
            oscl_memcpy(pDst + dst_pitch3*2, pDst, dst_width3);
        }

        pDst += dst_pitch3 * (_mColPix[row-1] + _mColPix[row]);
    }

    return 1;
#else
    return 0;
#endif // CCSCALING
}


// platform specfic function in C
int32 cc24sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise);

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 ColorConvert24::cc24ZoomRotate(uint8 **src, uint8 *dst,
                                     DisplayProperties *disp, uint8 *clip)
{
    int32 disp_prop[6];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    return cc24sc_rotate(src, dst, disp_prop, clip, _mRowPix, _mColPix, (_mRotation == CCROTATE_CLKWISE));
}


int32 cc24sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise)
{
#if (CCROTATE && CCSCALING)
    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, dst_pitch3, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, dst_width3, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 half_src_pitch, read_idx, tmp_src_pitch;
    int32 offset;

    src_pitch = disp[0];
    dst_pitch = disp[1];
    src_width = disp[2];

    dst_width3 = disp[4] * 3;

    if (_mIsRotateClkwise)
    {
        deltaY = src_pitch * disp[3] + 2;
        deltaCbCr = ((src_pitch * disp[3]) >> 2) + 1;
    }
    else    // rotate counterclockwise
    {
        deltaY = -(src_pitch * disp[3] + 2);
        deltaCbCr = -(((src_pitch * disp[3]) >> 2) + 1);
    }

    // map origin of the destination to the source
    if (_mIsRotateClkwise)
    {
        pY = src[0] + src_pitch * (disp[3] - 1);
        pCb = src[1] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));
        pCr = src[2] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));
    }
    else  // rotate counterclockwise
    {
        pY = src[0] + src_width - 1;
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;
    }

    if (_mIsRotateClkwise)
    {
        half_src_pitch = -(src_pitch >> 1);
        read_idx = 1;
        tmp_src_pitch = -src_pitch;
    }
    else // rotate counterclockwise
    {
        half_src_pitch = (src_pitch >> 1);
        read_idx = -1;
        tmp_src_pitch = src_pitch;
    }

    pDst =	dst;

    dst_pitch3	=	3 * dst_pitch;

    for (row = src_width - 1; row > 0; row -= 2)
    { /* decrement index, _mColPix[.] is
													 symmetric to increment index */
        if ((_mColPix[row-1] == 0) && (_mColPix[row] == 0))
        {
            pCb += read_idx;
            pCr += read_idx;
            pY += (read_idx * 2);
            continue;
        }

        for (col = disp[3] - 2; col >= 0; col -= 2)
        {/* decrement index, _mRowPix[.] is
													 symmetric to increment index */

            Cb = *pCb;
            pCb += half_src_pitch;
            Cr = *pCr;
            pCr += half_src_pitch;

            //load the bottom two pixels
            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            if (_mRowPix[col]) 	/* compute this pixel */
            {

                tmp0	=	pY[read_idx];		// bottom left

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16);
#else
                rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);
#endif
                Y	=	*pY;
                pY += tmp_src_pitch;	// upper right

                tmp1	=	(Y << 16) - Cg;
                tmp2	=	(Y << 16) + Cb;
                tmp0	=	(Y << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                tmp2 = ((tmp2 << 16) | (tmp1 << 8) | tmp0);
#else
                tmp2 |= ((tmp0 << 16) | (tmp1 << 8));
#endif
                if (((uint)pDst)&1) /* not word-aligned */
                {

                    *(pDst + dst_pitch3)	=	rgb & 0xFF; /* b */
                    *((uint16*)(pDst + dst_pitch3 + 1)) = rgb >> 8; /* gr */
                    *pDst = tmp2 & 0xFF;  /* b */
                    *((uint16*)(pDst + 1)) = tmp2 >> 8; /* gr */

                    if (_mRowPix[col] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 5) = rgb >> 16; /* r */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* bg */
                        *(pDst + 5) = tmp2 >> 16; /* r */

                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 5)) = (uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 7)) =   rgb >> 8;  /* gr */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 5)) = (uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 7)) =   tmp2 >> 8;  /* gr */
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gr */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gr */
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* rb */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gr */
                        *((uint16*)(pDst + dst_pitch3 + 6)) = rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 8) = rgb >> 16; /* r */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* rb */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gr */
                        *((uint16*)(pDst + 6)) = tmp2 & 0xFFFF; /* bg */
                        *(pDst + 8) = tmp2 >> 16; /* r */
                    }
                    else
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* bg */
                        *(pDst + dst_pitch3 + 2)	=	rgb >> 16; /* r */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* bg */
                        *(pDst + 2)	=	tmp2 >> 16; /* r */
                    }
                }
            }
            else  /* if(_mRowPix[col]) */
            {
                pY += tmp_src_pitch;	// upper right
            }

            pDst += _mRowPix[col] + _mRowPix[col] + _mRowPix[col];

            if (_mRowPix[col+1]) 	/* compute this pixel */
            {
                //load the top two pixels
                tmp0	=	pY[read_idx];	//bottom right

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16);
#else
                rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);
#endif
                Y	=	*pY;
                pY += tmp_src_pitch;					//upper right

                tmp1	=	(Y << 16) - Cg;
                tmp2	=	(Y << 16) + Cb;
                tmp0	=	(Y << 16) + Cr;

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                tmp2 = ((tmp2 << 16) | (tmp1 << 8) | tmp0);
#else
                tmp2 |= ((tmp0 << 16) | (tmp1 << 8));
#endif
                if (((uint)pDst)&1) /* not word-aligned */
                {

                    *(pDst + dst_pitch3)	=	rgb & 0xFF; /* r */
                    *((uint16*)(pDst + dst_pitch3 + 1)) = rgb >> 8; /* gb */
                    *pDst = tmp2 & 0xFF;  /* r */
                    *((uint16*)(pDst + 1)) = rgb >> 8; /* gb */

                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 5) = rgb >> 16; /* b */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* rg */
                        *(pDst + 5) = tmp2 >> 16; /* b */

                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3 + 3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 5)) = (uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 7)) =   rgb >> 8;  /* gb */
                        *((uint16*)(pDst + 3))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 5)) = (uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 7)) =   tmp2 >> 8;  /* gb */
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gb */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gb */
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *((uint16*)(pDst + dst_pitch3 + 2))	=	(uint16)((rgb << 8) | (rgb >> 16)); /* br */
                        *((uint16*)(pDst + dst_pitch3 + 4)) = rgb >> 8 ; /* gb */
                        *((uint16*)(pDst + dst_pitch3 + 6)) = rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 8) = rgb >> 16; /* b */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *((uint16*)(pDst + 2)) =	(uint16)((tmp2 << 8) | (tmp2 >> 16)); /* br */
                        *((uint16*)(pDst + 4)) = tmp2 >> 8 ; /* gb */
                        *((uint16*)(pDst + 6)) = tmp2 & 0xFFFF; /* rg */
                        *(pDst + 8) = tmp2 >> 16; /* b */
                    }
                    else
                    {
                        *((uint16*)(pDst + dst_pitch3))	=	rgb & 0xFFFF; /* rg */
                        *(pDst + dst_pitch3 + 2)	=	rgb >> 16; /* b */
                        *((uint16*)(pDst))	=	tmp2 & 0xFFFF; /* rg */
                        *(pDst + 2)	=	tmp2 >> 16; /* b */
                    }
                }
            }
            else  /* if(_mRowPix[col]) */
            {
                pY += tmp_src_pitch;	// upper right
            }

            pDst	+= _mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1];
        }//end of COL

        pY	+=	deltaY;
        pCb +=	deltaCbCr;
        pCr +=	deltaCbCr;
        pDst -=	(dst_width3);	//goes back to the beginning of the line;

        //copy down
        offset = (_mColPix[row] * dst_pitch3);

        if (_mColPix[row-1] && _mColPix[row] != 1)
        {
            oscl_memcpy(pDst + offset, pDst + dst_pitch3, dst_width3);
        }
        if (_mColPix[row-1] == 2)
        {
            oscl_memcpy(pDst + offset + dst_pitch3, pDst + dst_pitch3, dst_width3);
        }
        else if (_mColPix[row-1] == 3)
        {
            oscl_memcpy(pDst + offset + dst_pitch3, pDst + dst_pitch3, dst_width3);
            oscl_memcpy(pDst + offset + dst_pitch3*2, pDst + dst_pitch3, dst_width3);
        }

        //copy up
        if (_mColPix[row] == 2)
        {
            oscl_memcpy(pDst + dst_pitch3, pDst, dst_width3);
        }
        else if (_mColPix[row] == 3)
        {
            oscl_memcpy(pDst + dst_pitch3, pDst, dst_width3);
            oscl_memcpy(pDst + dst_pitch3*2, pDst, dst_width3);
        }

        pDst += dst_pitch3 * (_mColPix[row-1] + _mColPix[row]);
    }

    return 1;
#else
    return 0;
#endif // defined(CCROTATE) && defined(CCSCALING)
}
