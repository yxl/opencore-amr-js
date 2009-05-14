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
//  File: cczoomrotation32.cpp                                                //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
/** Class ColorConvert32, convert YUV to RGB32, similar to RGB24 with extra zero byte. */
#include "colorconv_config.h"
#include "cczoomrotation32.h"
#include "osclconfig_compiler_warnings.h"


/*---------------------------------------------------------------------------------
 * ARM V6 INLINE ASSEMBLY ROUTINES
 *---------------------------------------------------------------------------------
 */


OSCL_EXPORT_REF ColorConvertBase* ColorConvert32::NewL(void)
{
    ColorConvert32* self = OSCL_NEW(ColorConvert32, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


ColorConvert32::ColorConvert32()
{
    mClip = ((uint8*)mCoefTbl32) + 400;
}


OSCL_EXPORT_REF ColorConvert32::~ColorConvert32()
{
}


int32 ColorConvert32::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    _mInitialized = true;

    // set default
    SetYuvFullRange(false);
    SetMode(0);

    return 1;
}

int32  ColorConvert32::SetYuvFullRange(bool range)
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


int32 ColorConvert32::SetMode(int32 nMode)	//iMode : 0 Off, 1 On
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 0)
    {
        mPtrYUV2RGB	=	&ColorConvert32::get_frame32;
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
                mPtrYUV2RGB = &ColorConvert32::cc32ZoomRotate;
            }
            else /* zoom only */
            {
                mPtrYUV2RGB	=	&ColorConvert32::cc32ZoomIn;
            }
        }
        else
        {
            if (_mRotation&0x1) /* rotate only*/
            {
                mPtrYUV2RGB = &ColorConvert32::cc32Rotate;
            }
            else /* no zoom, no rotate, SetMode(1) = SetMode(0) */
            {
                mPtrYUV2RGB	=	&ColorConvert32::get_frame32;
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


int32 ColorConvert32::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return	_mState ? (_mDst_height*_mDst_pitch*4) : (_mSrc_width*_mSrc_height*4);
}


int32 ColorConvert32::Convert(uint8 **yuvBuf, uint8 *rgbBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(yuvBuf[0]);
    OSCL_ASSERT(yuvBuf[1]);
    OSCL_ASSERT(yuvBuf[2]);
    OSCL_ASSERT(rgbBuf);

    if (((uint32)rgbBuf)&0x3 || ((uint32)yuvBuf[0])&0x3) /* address is not word align */
    {
        return 0;
    }

    (*this.*mPtrYUV2RGB)(yuvBuf, rgbBuf, &_mDisp, (uint8 *)mClip);

    return 1;
}


int32 ColorConvert32::Convert(uint8 *yuvBuf, uint8 *rgbBuf)
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
    (*this.*mPtrYUV2RGB)(TmpYuvBuf, rgbBuf, &_mDisp, (uint8 *)mClip);

    return 1;
}


int32 cc32(uint8 **src, uint8 *dst, int32 *disp_prop, uint8 *coeff_tbl);
int32 cc32Reverse(uint8 **src, uint8 *dst, int32 *disp_prop, uint8 *coeff_tbl);

int32 ColorConvert32::get_frame32(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *clip)
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

    if (disp_prop[6] ^ disp_prop[7])	/* flip and rotate 180*/
    {
        return cc32Reverse(src, dst, disp_prop, clip);
    }
    else
    {
        return cc32(src, dst, disp_prop, clip);
    }
}

/*---------------------------------------------------------------------------------
 * CC32 WITH ARMV6 ASSEMBLY
 *---------------------------------------------------------------------------------
 */
int32 cc32(uint8 **src, uint8 *dst, int32 *disp, uint8 *clip)
{
    uint8		*pCb, *pCr;
    uint16		*pY;
    uint8		*pDst;
    int32 src_pitch, dst_pitch, dst_pitch4, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, deltaDst, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;


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

    deltaDst	=	((dst_pitch << 1) - src_width) * 4;
    pDst =	dst;
    dst_pitch4	=	4 * dst_pitch;

    for (row = disp[3]; row > 0; row -= 2)
    {
        for (col = src_width; col > 0; col -= 2)
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



            tmp0	=	Y & 0xFF;	//Low endian	left pixel

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            Y	>>=	8;
            tmp0	=	(Y << 16) + Cr;
            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            //load the top two pixels
            Y	=	*pY++;
            tmp0	=	Y & 0xFF;	//Low endian	left pixel
            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            *((uint32 *)(pDst))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            Y	>>=	8;

            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;
            tmp0	=	(Y << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            pDst += 8;
        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }

    return 1;
}

int32 cc32Reverse(uint8 **src, uint8 *dst, int32 *disp, uint8 *clip)
{
    uint8   *pCb, *pCr;
    uint16	*pY;
    uint8	*pDst;
    int32	src_pitch, dst_pitch, dst_pitch4 , src_width;
    int32	Y, Cb, Cr, Cg;
    int32	deltaY, deltaDst, deltaCbCr;
    int32	row, col;
    int32	tmp0, tmp1, tmp2;
    int32	nextrow ;

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (disp[6]) /* rotation, only */
    {  /* move the starting point to the bottom-right corner of the picture */
        nextrow = src_pitch * (disp[3] - 1);
        pY = (uint16*)(src[0] + nextrow + src_width - 2);
        nextrow = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
        pCb = src[1] + nextrow + (src_width >> 1) - 1;
        pCr = src[2] + nextrow + (src_width >> 1) - 1;
        nextrow = -(src_pitch >> 1);
        deltaY		=	src_width - (src_pitch << 1);
        deltaCbCr	=	(src_width - src_pitch) >> 1;
    }
    else	/* flip only */
    {   /* move the starting point to the top-right corner of the picture */
        pY = (uint16 *)(src[0] + src_width - 2);
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;
        nextrow = src_pitch >> 1;
        deltaY = src_width + (src_pitch << 1);
        deltaCbCr = (src_width + src_pitch) >> 1;
    }

    deltaDst	=	((dst_pitch << 1) - src_width) * 4;
    pDst =	dst;
    dst_pitch4	=	4 * dst_pitch;

    for (row = disp[3]; row > 0; row -= 2)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {

            Cb = *pCb--;
            Cr = *pCr--;
            //load the bottom two pixels

            Y = pY[nextrow];
            Cb -= 128;
            Cr -= 128;

            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));

            tmp0   = (Y >> 8) & 0xFF; // First Pixel in reverse order

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];



#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4 + 0))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4 + 0))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif


            Y	=	Y & 0xFF;	//  Second Pixel in reverse order

            tmp0	=	(Y << 16) + Cr;
            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            //load the top two pixels
            Y = *pY--;
            tmp0   = (Y >> 8) & 0xFF; // First Pixel in reverse order ;

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            *((uint32 *)(pDst + 0))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + 0))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            Y	=	Y & 0xFF;	//  Second Pixel in reverse order

            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;
            tmp0	=	(Y << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            pDst += 8;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);
    }
    return 1;
}

int32 ColorConvert32::cc32Rotate(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *clip)
{
    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, src_width, dst_pitch4;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, deltaDst, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    int32 half_src_pitch, read_idx, tmp_src_pitch;
    int32 disp_prop[8];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;
    disp_prop[6] = _mRotation;
    disp_prop[7] = _mIsFlip;

    src_pitch	=	disp_prop[0];
    dst_pitch	=	disp_prop[1];
    src_width	=	disp_prop[2];

    deltaDst	=	4 * ((dst_pitch << 1) -  disp_prop[4]);

    if (_mRotation == CCROTATE_CLKWISE)
    {

        // go from top-left to bottom-left
        deltaY		=  src_pitch * disp_prop[3] + 2;
        deltaCbCr	= ((src_pitch * disp_prop[3]) >> 2) + 1;


        // map origin of the destination to the source
        // goto bottom-left
        pY = src[0] + src_pitch * (disp_prop[3] - 1);
        pCb = src[1] + ((src_pitch >> 1) * ((disp_prop[3] >> 1) - 1));
        pCr = src[2] + ((src_pitch >> 1) * ((disp_prop[3] >> 1) - 1));

        half_src_pitch = -(src_pitch >> 1);
        read_idx = 1;
        tmp_src_pitch = -src_pitch;
    }
    else  // rotate counterclockwise
    {   // go from bottom-right back to top-right
        deltaY		=  -(src_pitch *  disp_prop[3] + 2);
        deltaCbCr	=  -(((src_pitch *  disp_prop[3]) >> 2) + 1);

        // map origin of the destination to the source
        // goto top-right
        pY = src[0] + src_width - 1;
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;

        half_src_pitch = (src_pitch >> 1);
        read_idx = -1;
        tmp_src_pitch = src_pitch;

    }


    pDst =	dst;
    dst_pitch4	=	4 * dst_pitch;

    for (row = src_width; row > 0; row -= 2)
    {
        for (col = disp_prop[3]; col > 0; col -= 2)
        {


            Cb = *pCb;
            pCb += half_src_pitch;
            Cr = *pCr;
            pCr += half_src_pitch;


            //process the bottom two pixels in RGB plane

            Cb -= 128;
            Cr -= 128;

            Cg	=	Cr * (*((int32*)(clip - 400)));
            Cg	+=	Cb * (*((int32*)(clip - 392)));

            Cr	*=	(*((int32*)(clip - 396)));
            Cb	*=	(*((int32*)(clip - 388)));


            tmp0	=	pY[read_idx]; /* top-left pixel */

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            Y		=	pY[read_idx+tmp_src_pitch];  /* bottom-left pixel */
            tmp0	=	(Y << 16) + Cr;
            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + dst_pitch4 + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif

            tmp0	=	*pY; /*upper-right pixel */
            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            *((uint32 *)(pDst))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            Y		=	pY[tmp_src_pitch]; /* bottom-right pixel */
            pY += (tmp_src_pitch << 1);

            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;
            tmp0	=	(Y << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];


#if RGB_FORMAT
            *((uint32 *)(pDst + 4))	=	tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            *((uint32 *)(pDst + 4))	=	tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            pDst += 8;
        }//end of COL

        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);

    }

    return 1;
}


// platform specific C function
int32 cc32scaledown(uint8 **src, uint8 *dst, int32 *disp,
                    uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix);
int32 cc32scaleup(uint8 **src, uint8 *dst, int32 *disp,
                  uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix);

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB_FORMAT buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 ColorConvert32::cc32ZoomIn(uint8 **src, uint8 *dst,
                                 DisplayProperties *disp, uint8 *clip)
{
    int32 disp_prop[8];
    int32 src_width, dst_width;

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = src_width = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = dst_width = disp->dst_width;
    disp_prop[5] = disp->dst_height;
    disp_prop[6] = (_mRotation > 0 ? 1 : 0);
    disp_prop[7] = _mIsFlip;

    if (src_width > dst_width) /* scale down in width */
    {
        return cc32scaledown(src, dst, disp_prop, clip, _mRowPix, _mColPix);
    }
    else
    {
        return cc32scaleup(src, dst, disp_prop, clip, _mRowPix, _mColPix);
    }
}

int32 cc32scaledown(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip,
                    uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING

    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, dst_pitch4, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, dst_width4, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;

    src_pitch = disp[0];
    dst_pitch = disp[1];
    src_width = disp[2];

    deltaY = (src_pitch << 1) - src_width;
    deltaCbCr = (src_pitch - src_width) >> 1;
    dst_width4 = disp[4] * 4;

    pY = src[0];
    pCb = src[1];
    pCr = src[2];

    pDst = dst;
    dst_pitch4 = 4 * dst_pitch;

    for (row = disp[3] - 1; row >= 0; row -= 2)
    {/* decrement index, _mColPix[.] is
													 symmetric to increment index */
        if (_mColPix[row-1] + _mColPix[row] == 0)
        {
            pCb += (src_pitch >> 1);
            pCr += (src_pitch >> 1);
            pY += (src_pitch << 1);
            continue;
        }

        if (_mColPix[row-1] + _mColPix[row] == 1) // one line not skipped
        {
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
                    Y	=	*pY++;						//upper left
                    tmp1	=	(Y << 16) - Cg;
                    tmp2	=	(Y << 16) + Cb;
                    tmp0	=	(Y << 16) + Cr;

                    tmp0	=	clip[tmp0>>16];
                    tmp1	=	clip[tmp1>>16];
                    tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
                    tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
                    tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32*)pDst) = tmp0;

                    pDst += 4;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY++;
                }

                if (_mRowPix[col+1])	/* compute this pixel */
                {
                    Y	=	*pY++;						//upper left
                    tmp1	=	(Y << 16) - Cg;
                    tmp2	=	(Y << 16) + Cb;
                    tmp0	=	(Y << 16) + Cr;

                    tmp0	=	clip[tmp0>>16];
                    tmp1	=	clip[tmp1>>16];
                    tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
                    tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
                    tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32*)pDst) = tmp0;

                    pDst += 4;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY++;
                }

            }//end of COL
        }
        else // both lines not skipped
        {
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
                    rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
                    rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32 *)(pDst + dst_pitch4))	=	rgb;

                    Y	=	*pY++;						//upper left
                    tmp1	=	(Y << 16) - Cg;
                    tmp2	=	(Y << 16) + Cb;
                    tmp0	=	(Y << 16) + Cr;

                    tmp0	=	clip[tmp0>>16];
                    tmp1	=	clip[tmp1>>16];
                    tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
                    tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
                    tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32*)pDst) = tmp0;

                    pDst += 4;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY++;
                }

                if (_mRowPix[col+1])	/* compute this pixel */
                {
                    tmp0	=	pY[src_pitch];			//bottom left

                    tmp1	=	(tmp0 << 16) - Cg;
                    tmp2	=	(tmp0 << 16) + Cb;
                    tmp0	=	(tmp0 << 16) + Cr;

                    tmp0	=	clip[tmp0>>16];
                    tmp1	=	clip[tmp1>>16];
                    tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
                    rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
                    rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32 *)(pDst + dst_pitch4))	=	rgb;

                    Y	=	*pY++;						//upper left
                    tmp1	=	(Y << 16) - Cg;
                    tmp2	=	(Y << 16) + Cb;
                    tmp0	=	(Y << 16) + Cr;

                    tmp0	=	clip[tmp0>>16];
                    tmp1	=	clip[tmp1>>16];
                    tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
                    tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
                    tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
                    *((uint32*)pDst) = tmp0;

                    pDst += 4;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY++;
                }
            }//end of COL
        }
        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;

        pDst -=	(dst_width4);	//goes back to the beginning of the line;
        pDst += dst_pitch4 * (_mColPix[row-1] + _mColPix[row]);
    }


    return 1;
#else
    return 0;
#endif
}



int32 cc32scaleup(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip,
                  uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING

    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, dst_pitch4, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, dst_width4, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 offset, repeat;

    src_pitch = disp[0];
    dst_pitch = disp[1];
    src_width = disp[2];

    deltaY = (src_pitch << 1) - src_width;
    deltaCbCr = (src_pitch - src_width) >> 1;
    dst_width4 = disp[4] * 4;

    pY = src[0];
    pCb = src[1];
    pCr = src[2];

    pDst = dst;

    dst_pitch4 = 4 * dst_pitch;

    for (row = disp[3] - 1; row >= 0; row -= 2)
    {/* decrement index, _mColPix[.] is
													 symmetric to increment index */
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

            tmp0	=	pY[src_pitch];			//bottom left

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            *((uint32 *)(pDst + dst_pitch4))	=	rgb;

            Y	=	*pY++;						//upper left
            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;
            tmp0	=	(Y << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
            tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            *((uint32*)pDst) = tmp0;

            repeat = _mRowPix[col];

            if (repeat == 2)
            {
                *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                *((uint32*)(pDst + 4)) = tmp0 ;
            }
            else if (repeat == 3)
            {
                *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                *((uint32*)(pDst + dst_pitch4 + 8))	=	rgb;
                *((uint32*)(pDst + 4)) = tmp0 ;
                *((uint32*)(pDst + 8)) = tmp0 ;
            }
            // else, assume repeat = 1

            pDst += (repeat << 2);

            tmp0	=	pY[src_pitch];			//bottom left

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];
#if RGB_FORMAT
            rgb = tmp0 | (tmp1 << 8) | (tmp2 << 16)  ; /* bgr */
#else
            rgb = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            *((uint32 *)(pDst + dst_pitch4))	=	rgb;

            Y	=	*pY++;						//upper left
            tmp1	=	(Y << 16) - Cg;
            tmp2	=	(Y << 16) + Cb;
            tmp0	=	(Y << 16) + Cr;

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#if RGB_FORMAT
            tmp0 |= ((tmp1 << 8) | (tmp2 << 16))  ; /* bgr */
#else
            tmp0 = tmp2 | (tmp1 << 8) | (tmp0 << 16);  /* rgb */
#endif
            *((uint32*)pDst) = tmp0;

            repeat = _mRowPix[col+1];

            if (repeat == 2)
            {
                *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                *((uint32*)(pDst + 4)) = tmp0 ;
            }
            else if (repeat == 3)
            {
                *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                *((uint32*)(pDst + dst_pitch4 + 8))	=	rgb;
                *((uint32*)(pDst + 4)) = tmp0 ;
                *((uint32*)(pDst + 8)) = tmp0 ;
            }
            // else, assume repeat = 1

            pDst += (repeat << 2);

        }//end of COL

        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;

        pDst -=	(dst_width4);	//goes back to the beginning of the line;

        //copy down
        offset = (_mColPix[row] * dst_pitch4);

        if (_mColPix[row-1] && _mColPix[row] != 1)
        {
            oscl_memcpy(pDst + offset, pDst + dst_pitch4, dst_width4);
        }
        if (_mColPix[row-1] == 2)
        {
            oscl_memcpy(pDst + offset + dst_pitch4, pDst + dst_pitch4, dst_width4);
        }
        else if (_mColPix[row-1] == 3)
        {
            oscl_memcpy(pDst + offset + dst_pitch4, pDst + dst_pitch4, dst_width4);
            oscl_memcpy(pDst + offset + dst_pitch4*2, pDst + dst_pitch4, dst_width4);
        }

        //copy up
        if (_mColPix[row] == 2)
        {
            oscl_memcpy(pDst + dst_pitch4, pDst, dst_width4);
        }
        else if (_mColPix[row] == 3)
        {
            oscl_memcpy(pDst + dst_pitch4, pDst, dst_width4);
            oscl_memcpy(pDst + dst_pitch4*2, pDst, dst_width4);
        }

        pDst += dst_pitch4 * (_mColPix[row-1] + _mColPix[row]);
    }


    return 1;
#else
    return 0;
#endif
}


// platform specfic function in C
int32 cc32sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise);

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().



int32 ColorConvert32::cc32ZoomRotate(uint8 **src, uint8 *dst,
                                     DisplayProperties *disp, uint8 *clip)
{
    int32 disp_prop[6];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    return cc32sc_rotate(src, dst, disp_prop, clip, _mRowPix, _mColPix, (_mRotation == CCROTATE_CLKWISE));
}


int32 cc32sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise)
{
#if (CCROTATE && CCSCALING)
    uint8 *pCb, *pCr;
    uint8 *pY;
    uint8 *pDst;
    int32 src_pitch, dst_pitch, dst_pitch4, src_width;
    int32 Y, Cb, Cr, Cg;
    int32 deltaY, dst_width4, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 half_src_pitch, read_idx, tmp_src_pitch;
    int32 offset;

    src_pitch = disp[0];
    dst_pitch = disp[1];
    src_width = disp[2];

    dst_width4 = disp[4] * 4;

    if (_mIsRotateClkwise)
    {
        deltaY = src_pitch * disp[3] + 2;
        deltaCbCr = ((src_pitch * disp[3]) >> 2) + 1;

        pY = src[0] + src_pitch * (disp[3] - 1);
        pCb = src[1] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));
        pCr = src[2] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));

        half_src_pitch = -(src_pitch >> 1);
        read_idx = 1;
        tmp_src_pitch = -src_pitch;
    }
    else    // rotate counterclockwise
    {
        deltaY = -(src_pitch * disp[3] + 2);
        deltaCbCr = -(((src_pitch * disp[3]) >> 2) + 1);

        pY = src[0] + src_width - 1;
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;

        half_src_pitch = (src_pitch >> 1);
        read_idx = -1;
        tmp_src_pitch = src_pitch;

    }

    pDst =	dst;
    dst_pitch4	=	4 * dst_pitch;

    for (row = src_width - 1; row > 0; row -= 2)
    { /* decrement index, _mColPix[.] is symmetric to increment index */
        if ((_mColPix[row-1] == 0) && (_mColPix[row] == 0))
        {
            pCb += read_idx;
            pCr += read_idx;
            pY += (read_idx * 2);
            continue;
        }

        if (_mColPix[row-1] + _mColPix[row] == 1) // do only one row, scale down
        {
            for (col = disp[3] - 2; col >= 0; col -= 2)
            {/* decrement index, _mRowPix[.] is symmetric to increment index */

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


                    *((uint32*)pDst) = tmp2;


                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += tmp_src_pitch;	// upper right
                }


                pDst += _mRowPix[col] + _mRowPix[col] + _mRowPix[col] + _mRowPix[col];



                if (_mRowPix[col+1]) 	/* compute this pixel */
                {

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



                    *((uint32*)pDst) = tmp2;


                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += tmp_src_pitch;	// upper right
                }

                pDst	+= (_mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1]);


            }

            pY	+=	deltaY;
            pCb +=	deltaCbCr;
            pCr +=	deltaCbCr;
            pDst += dst_pitch4;
            pDst -=	(dst_width4);	//goes back to the beginning of the line;

        }//end of COL

        else   // do two rows at least, scale up or down
        {
            for (col = disp[3] - 2; col >= 0; col -= 2)
            {/* decrement index, _mRowPix[.] is  symmetric to increment index */

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
                    if (_mRowPix[col] == 2)
                    {

                        *((uint32 *)(pDst + dst_pitch4))	=	rgb;
                        *((uint32*)pDst) = tmp2;
                        *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                        *((uint32*)(pDst + 4)) = tmp2 ;
                    }
                    else if (_mRowPix[col] == 3)
                    {

                        *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                        *((uint32*)(pDst + dst_pitch4 + 8))	=	rgb;
                        *((uint32*)(pDst + 4)) = tmp2 ;
                        *((uint32*)(pDst + 8)) = tmp2 ;

                    }
                    else
                    {

                        *((uint32 *)(pDst + dst_pitch4))	=	rgb;
                        *((uint32*)pDst) = tmp2;
                    }

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += tmp_src_pitch;	// upper right
                }


                pDst += (_mRowPix[col] + _mRowPix[col] + _mRowPix[col] + _mRowPix[col]);



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

                    if (_mRowPix[col+1] == 2)
                    {

                        *((uint32 *)(pDst + dst_pitch4))	=	rgb;
                        *((uint32*)pDst) = tmp2;
                        *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                        *((uint32*)(pDst + 4)) = tmp2 ;
                    }
                    else if (_mRowPix[col+1] == 3)
                    {

                        *((uint32*)(pDst + dst_pitch4 + 4))	=	rgb;
                        *((uint32*)(pDst + dst_pitch4 + 8))	=	rgb;
                        *((uint32*)(pDst + 4)) = tmp2 ;
                        *((uint32*)(pDst + 8)) = tmp2 ;
                    }
                    else
                    {
                        *((uint32 *)(pDst + dst_pitch4))	=	rgb;
                        *((uint32*)pDst) = tmp2;
                    }

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += tmp_src_pitch;	// upper right
                }

                pDst	+= (_mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1] + _mRowPix[col+1]);

            }//end of COL

            pY	+=	deltaY;
            pCb +=	deltaCbCr;
            pCr +=	deltaCbCr;
            pDst -=	(dst_width4);	//goes back to the beginning of the line;

            //copy down
            offset = (_mColPix[row] * dst_pitch4);

            if (_mColPix[row-1] && _mColPix[row] != 1)
            {
                oscl_memcpy(pDst + offset, pDst + dst_pitch4, dst_width4);
            }
            if (_mColPix[row-1] == 2)
            {
                oscl_memcpy(pDst + offset + dst_pitch4, pDst + dst_pitch4, dst_width4);
            }
            else if (_mColPix[row-1] == 3)
            {
                oscl_memcpy(pDst + offset + dst_pitch4, pDst + dst_pitch4, dst_width4);
                oscl_memcpy(pDst + offset + dst_pitch4*2, pDst + dst_pitch4, dst_width4);
            }

            //copy up
            if (_mColPix[row] == 2)
            {
                oscl_memcpy(pDst + dst_pitch4, pDst, dst_width4);
            }
            else if (_mColPix[row] == 3)
            {
                oscl_memcpy(pDst + dst_pitch4, pDst, dst_width4);
                oscl_memcpy(pDst + dst_pitch4*2, pDst, dst_width4);
            }

            pDst += dst_pitch4 * (_mColPix[row-1] + _mColPix[row]);

        }


    } // end of row

    return 1;
#else
    return 0;
#endif // defined(CCROTATE) && defined(CCSCALING)
}




