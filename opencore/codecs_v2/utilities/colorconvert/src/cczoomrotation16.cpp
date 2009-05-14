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
/** Class ColorConvert16, convert YUV to RGB16 in 5-6-5 format. */
#include "colorconv_config.h"
#include "cczoomrotation16.h"

#define pvcc_abs(x) ((x)>0? (x): -(x))

#define DITHER_ON
#ifdef DITHER_ON

#define OFFSET_5_0  2
#define OFFSET_6_0  (1+1024)
#define	OFFSET_5_1  6
#define	OFFSET_6_1  (3+1024)

#else  //for the rounding case (no dither)

#define OFFSET_5_0  0 // Set to zero for performance4
#define OFFSET_6_0  (0+1024) //(2+1024)
#define	OFFSET_5_1  0 //4
#define	OFFSET_6_1  (0+1024) //(2+1024)

#endif

OSCL_EXPORT_REF ColorConvertBase* ColorConvert16::NewL(void)
{
    ColorConvert16* self = OSCL_NEW(ColorConvert16, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


ColorConvert16::ColorConvert16()
{
    mCoefTbl = (uint8*)mCoefTbl32;
}


OSCL_EXPORT_REF ColorConvert16::~ColorConvert16()
{
}


int32 ColorConvert16::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    //nRotation = 1;
    //Dst_pitch = Dst_width = Src_height;
    //Dst_height = Src_width;

    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    _mInitialized = false;

    if ((nRotation&1) == 0) // check for either shrinking or zooming both horz and vert. No combination..
    {
        if ((Src_width > Dst_width && Src_height < Dst_height) ||
                (Src_width < Dst_width && Src_height > Dst_height))
        {
            return 0;
        }
    }
    else
    {
        if ((Src_width > Dst_height && Src_height < Dst_width) ||
                (Src_width < Dst_height && Src_height > Dst_width))
        {
            return 0;
        }
    }

    _mInitialized = true;

    // set default
    SetYuvFullRange(false);
    SetMode(0);

    return 1;
}


int32  ColorConvert16::SetYuvFullRange(bool range)
{
    uint8 *clip;
    int32 tmp, i;

    OSCL_ASSERT(_mInitialized == true);

    _mYuvRange = range;

    //local init
    if (_mYuvRange == false) // default Yuv range from 16-235, BT.709
    {
        *((uint32*)mCoefTbl) = 0x0000b2ce; //65536*0.813/1.164;  // 0.533
        *((uint32*)(mCoefTbl + 4)) =  0x00015f03; //65536*1.596/1.164; // 1.793
        *((uint32*)(mCoefTbl + 8)) =  0x000055fe; //65536*0.391/1.164; // 0.213
        *((uint32*)(mCoefTbl + 12)) =  0x0001bbd2; //65536*2.018/1.164; // 2.112

        clip = mCoefTbl + 400;
        /* do 5 bit conversion */

        oscl_memset(&clip[-384], 0, 401*sizeof(*clip));
        oscl_memset(&clip[ 640], 0, 401*sizeof(*clip));

        for (i = 17; i < 236; i++)
        {                       // range of (x>>3) between -24 and 56
            tmp = (int32)(1.164 * (i - 16));   // clip[1.164*((x>>3) - (16>>3))]
            clip[i] = (uint8)(tmp >> 3);
            clip[i+1024] = (uint8)(tmp >> 2);
        }

        oscl_memset(&clip[236], 31, 404*sizeof(*clip));
        oscl_memset(&clip[1260], 63, 404*sizeof(*clip));

    }
    else  // full range 0-255
    {
        *((uint32*)mCoefTbl) = (int)(65536 * 0.4681); //0.714);
        *((uint32*)(mCoefTbl + 4)) = (int)(65536 * 1.5748);//1.402);
        *((uint32*)(mCoefTbl + 8)) = (int)(65536 * 0.1873);//0.344);
        *((uint32*)(mCoefTbl + 12)) = (int)(65536 * 1.8556);//1.772);

        clip = mCoefTbl + 400;
        /* do 5 bit conversion */
        oscl_memset(&clip[-384], 0, 385*sizeof(*clip));
        oscl_memset(&clip[ 640], 0, 385*sizeof(*clip));

        for (i = 1; i < 255; i++)   // range of (x>>3) between -24 and 56
        {
            clip[i] = i >> 3;
            clip[i+1024] = i >> 2;
        }
        oscl_memset(&clip[255], 31, 385*sizeof(*clip));
        oscl_memset(&clip[1279], 63, 385*sizeof(*clip));
    }

    return 1; // success
}


int32 ColorConvert16::SetMode(int32 nMode)	//nMode : 0 Off, 1 On
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 0)
    {
        //		mPtrYUV2RGB = cc16Rotate;
        //		_mState		= 1;
        mPtrYUV2RGB	=	&ColorConvert16::get_frame16;
        _mState		=	0;
        _mDisp.src_pitch = _mSrc_pitch  ;
        _mDisp.dst_pitch = _mDst_pitch  ;
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
                mPtrYUV2RGB = &ColorConvert16::cc16ZoomRotate;
            }
            else /* zoom only */
            {
                mPtrYUV2RGB	=	&ColorConvert16::cc16ZoomIn;
            }
        }
        else
        {
            if (_mRotation&0x1) /* rotate only*/
            {
                mPtrYUV2RGB = &ColorConvert16::cc16Rotate;
            }
            else /* no zoom, no rotate, SetMode(1) = SetMode(0) */
            {
                mPtrYUV2RGB	=	&ColorConvert16::get_frame16;
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


int32 ColorConvert16::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return	_mState ? (_mDst_height*_mDst_pitch*2) : (_mSrc_width*_mSrc_height*2);
}


int32 ColorConvert16::Convert(uint8 **yuvBuf, uint8 *rgbBuf)
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

    (*this.*mPtrYUV2RGB)(yuvBuf, rgbBuf, &_mDisp, (uint8 *)mCoefTbl);

    return 1;
}


int32 ColorConvert16::Convert(uint8 *yuvBuf, uint8 *rgbBuf)
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

    (*this.*mPtrYUV2RGB)(TmpYuvBuf, rgbBuf, &_mDisp, (uint8 *)mCoefTbl);

    return 1;
}


int32 cc16(uint8 **src, uint8 *dst, int32 *disp_prop, uint8 *coeff_tbl);
int32 cc16Reverse(uint8 **src, uint8 *dst, int32 *disp_prop, uint8 *coeff_tbl);

int32 ColorConvert16::get_frame16(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *coff_tbl)
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
        return cc16Reverse(src, dst, disp_prop, coff_tbl);
    }
    else
    {
        return cc16(src, dst, disp_prop, coff_tbl);
    }
}

int32 cc16(uint8 **src, uint8 *dst, int32 *disp, uint8 *coff_tbl)
{

    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaDst, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

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
        pY = (uint16 *) src[0];
        src_pitch >>= 1;
        pCb = src[1];
        pCr = src[2];
    }

    deltaDst	=	(dst_pitch << 1) - src_width;
    pDst =	(uint16 *)dst;

    for (row = disp[3]; row > 0; row -= 2)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {

            Cb = *pCb++;
            Cr = *pCr++;
            Y = pY[src_pitch];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * cc1;
            Cr	*= cc3;

            Cg	+=	Cb * cc2;
            Cb	*=  cc4;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            Y   = (Y >> 8) & 0xFF;

            Y	+= OFFSET_5_1;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];

            //RGB_565

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            rgb		|=	(tmp0 << 16);

            *((uint32*)(pDst + dst_pitch))	= rgb;

            //load the top two pixels
            Y = *pY++;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_1;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            Y   = (Y >> 8) & 0xFF;

            Y	+= OFFSET_5_0;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];

            //RGB_565

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            rgb		|=	(tmp0 << 16);
            *((uint32 *)pDst)	= rgb;
            pDst += 2;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
}

int32 cc16Reverse(uint8 **src, uint8 *dst, int32 *disp, uint8 *coff_tbl)
{
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaDst, deltaCbCr;
    int		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    //	int32		mRotation;
    int	nextrow, mIsFlip;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));


    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    mIsFlip		=   disp[7];

    deltaDst	=	(dst_pitch << 1) - src_width;

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

    pDst =	(uint16 *)dst;

    for (row = disp[3]; row > 0; row -= 2)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {

            Cb = *pCb--;
            Cr = *pCr--;
            //load the bottom two pixels
            //Y	=	*(((uint16 *)pY)+src_pitch);
            //Y	=	*((uint16 *)(((uint16 *)pY) + src_pitch));
            Y = pY[nextrow];

            Cb -= 128;
            Cr -= 128;
            //Cg	=	Cr*JCoeff[0];//*((int32*)(clip - 40))
            Cg	=	Cr * cc1;
            Cr	*= cc3;

            Cg	+=	Cb * cc2;
            Cb	*=  cc4;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            Y   = (Y >> 8) & 0xFF;

            Y	+= OFFSET_5_1;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];
            //RGB_565

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            rgb = (rgb << 16) | tmp0;

            *((uint32*)(pDst + dst_pitch))	= rgb;

            //load the top two pixels
            //Y	=	*((uint16 *)pY)++;
            Y = *pY--;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_1;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            Y   = (Y >> 8) & 0xFF;

            Y	+= OFFSET_5_0;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            rgb	 = (rgb << 16) | tmp0;

            //			*( (unsigned int32 *)pDst)++	= rgb;
            *((uint32 *)pDst)	= rgb;
            pDst += 2;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
}


int32 cc16rotate_P(uint8 **src, uint8 *dst, int32 src_pitch, int32 dst_pitch, int32 src_width, int32 src_height,
                   int32 deltaY, int32 deltaCbCr, int32 deltaDst, uint8 *coeff_tbl);
int32 cc16rotate_N(uint8 **src, uint8 *dst, int32 src_pitch, int32 dst_pitch, int32 src_width, int32 src_height,
                   int32 deltaY, int32 deltaCbCr, int32 deltaDst, uint8 *coeff_tbl);

int32 ColorConvert16::cc16Rotate(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL)
{
    int32 src_pitch, dst_pitch, src_width, src_height;
    int32 deltaY, deltaCbCr, deltaDst;

    src_pitch	=	disp->src_pitch;
    dst_pitch	=   disp->dst_pitch;
    src_width	=	disp->src_width;
    src_height  =   disp->src_height;

    deltaY = (src_pitch << 1) - src_width;
    deltaCbCr = (src_pitch - src_width) >> 1;

    if (_mRotation == CCROTATE_CLKWISE)
    {
        if (!_mIsFlip)
        {	// go from upper-right down and left
            dst += ((src_height - 1) << 1);
            deltaDst  =	 -(dst_pitch * src_width) - 2;

            return cc16rotate_N(src, dst, src_pitch, dst_pitch, src_width, src_height, deltaY,
                                deltaCbCr, deltaDst, COFF_TBL);
        }
        else
        {    // go from origin down and right
            deltaDst  =  -(dst_pitch * src_width) + 2;
            return cc16rotate_P(src, dst, src_pitch, dst_pitch, src_width, src_height, deltaY,
                                deltaCbCr, deltaDst, COFF_TBL);
        }

    }
    else  // rotate counterclockwise
    {
        if (!_mIsFlip)
        {	// go from bottom-left to up right
            dst += (((src_width - 1) * dst_pitch) << 1);
            deltaDst = (dst_pitch * src_width + 2);
            dst_pitch = -dst_pitch;

            return cc16rotate_P(src, dst, src_pitch, dst_pitch, src_width, src_height, deltaY,
                                deltaCbCr, deltaDst, COFF_TBL);
        }
        else
        {	// go from bottom right to top and left
            dst += (((src_height - 1) << 1) + (((src_width - 1) * dst_pitch) << 1));
            deltaDst = (dst_pitch * src_width - 2);
            dst_pitch = -dst_pitch;

            return cc16rotate_N(src, dst, src_pitch, dst_pitch, src_width, src_height, deltaY,
                                deltaCbCr, deltaDst, COFF_TBL);
        }
    }

}

int32 cc16rotate_P(uint8 **src, uint8 *dst, int32 src_pitch, int32 dst_pitch, int32 src_width, int32 src_height,
                   int32 deltaY, int32 deltaCbCr, int32 deltaDst, uint8 *coeff_tbl)
{
#if CCROTATE
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		Y, Cb, Cr, Cg;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coeff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    pY = (uint16*) src[0];
    src_pitch >>= 1;
    pCb = src[1];
    pCr = src[2];

    pDst =	(uint16 *)dst;

    for (row = src_height; row > 0; row -= 2)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {

            Cb = *pCb++;
            Cr = *pCr++;
            //load the bottom two pixels
            Y = pY[src_pitch];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * cc1;
            Cr	*= cc3;

            Cg	+=	Cb * cc2;
            Cb	*=  cc4;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            *(pDst += 1) = rgb;

            Y   = (Y >> 8);
            Y	+= OFFSET_5_1;

            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];
            //RGB_565

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            *(pDst += dst_pitch)	= tmp0;

            //load the top two pixels
            Y = *pY++;

            tmp0	= (Y >> 8); //Low endian	right pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);
            *(pDst -= 1)   =   rgb;

            tmp0   = Y & 0xFF;
            tmp0	+= OFFSET_5_1;

            tmp1	=	(tmp0) - (Cg >> 16);
            tmp2	=	(tmp0) + (Cb >> 16);
            tmp0	=	(tmp0) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            pDst[-dst_pitch] = tmp0;
            pDst += dst_pitch;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(src_pitch);
    OSCL_UNUSED_ARG(dst_pitch);
    OSCL_UNUSED_ARG(src_width);
    OSCL_UNUSED_ARG(src_height);
    OSCL_UNUSED_ARG(deltaY);
    OSCL_UNUSED_ARG(deltaCbCr);
    OSCL_UNUSED_ARG(deltaDst);
    OSCL_UNUSED_ARG(coeff_tbl);

    return 0;
#endif // CCROTATE
}

int32 cc16rotate_N(uint8 **src, uint8 *dst, int32 src_pitch, int32 dst_pitch, int32 src_width, int32 src_height,
                   int32 deltaY, int32 deltaCbCr, int32 deltaDst, uint8 *coeff_tbl)
{
#if CCROTATE
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		Y, Cb, Cr, Cg;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coeff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    pY = (uint16*) src[0];
    src_pitch >>= 1;
    pCb = src[1];
    pCr = src[2];

    pDst =	(uint16 *)dst;

    for (row = src_height; row > 0; row -= 2)
    {

        for (col = src_width - 1; col >= 0; col -= 2)
        {

            Cb = *pCb++;
            Cr = *pCr++;
            //load the bottom two pixels
            Y = pY[src_pitch];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * cc1;
            Cr	*= cc3;

            Cg	+=	Cb * cc2;
            Cb	*=  cc4;

            tmp0    = (Y & 0xFF);   //Low endian    left pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];
            //RGB_565

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);

            *(pDst -= 1) = rgb;

            Y   = (Y >> 8) & 0xFF;

            Y	+= OFFSET_5_1;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];
            //RGB_565

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            *(pDst += dst_pitch)	= tmp0;

            //load the top two pixels
            Y = *pY++;

            tmp0    = (Y >> 8) & 0xFF; //Low endian    right pixel
            tmp0	+= OFFSET_5_0;

            tmp1	=	tmp0 - (Cg >> 16);
            tmp2	=	tmp0 + (Cb >> 16);
            tmp0	=	tmp0 + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
            tmp2	=	clip[tmp2];

            rgb		=	tmp1 | (tmp0 << 6);
            rgb		=	tmp2 | (rgb << 5);
            *(pDst += 1)   =   rgb;

            Y   = (Y & 0xFF);

            Y	+= OFFSET_5_1;
            tmp1	=	(Y) - (Cg >> 16);
            tmp2	=	(Y) + (Cb >> 16);
            tmp0	=	(Y) + (Cr >> 16);

            tmp0	=	clip[tmp0];
            tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
            tmp2	=	clip[tmp2];

            tmp0	=	tmp1 | (tmp0 << 6);
            tmp0	=	tmp2 | (tmp0 << 5);

            pDst[-dst_pitch] = tmp0;
            pDst += dst_pitch;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(src_pitch);
    OSCL_UNUSED_ARG(dst_pitch);
    OSCL_UNUSED_ARG(src_width);
    OSCL_UNUSED_ARG(src_height);
    OSCL_UNUSED_ARG(deltaY);
    OSCL_UNUSED_ARG(deltaCbCr);
    OSCL_UNUSED_ARG(deltaDst);
    OSCL_UNUSED_ARG(coeff_tbl);

    return 0;
#endif // CCROTATE
}



int32 cc16scaledown(uint8 **src, uint8 *dst, int32 *disp,
                    uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix);
int32 cc16scalingHalf(uint8 **src, uint8 *dst, int32 *disp,
                      uint8 *coff_tbl);
int32 cc16scaling34(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl);
int32 cc16scaleup(uint8 **src, uint8 *dst, int32 *disp,
                  uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix);
int32 cc16scaling54(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl);
int32 cc16scaling43(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl);
int32 cc16scaling128x96(uint8 **src, uint8 *dst,
                        int32 *disp, uint8 *coff_tbl);

// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetOutputBufferSize API to get the size it needs. See GetOutputBufferSize().
int32 ColorConvert16::cc16ZoomIn(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *coff_tbl)
{
    int32 disp_prop[8];
    int32 src_width, src_height, dst_width, dst_height;
    int32 tempw, temph;

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
        tempw = (3 * src_width) >> 2;
        temph = (3 * src_height) >> 2;
        /* check for special zoom-out case, 3:4 scaling down */
        if (dst_width == tempw  &&  dst_height == temph && !(src_width&3))
        {
            return cc16scaling34(src, dst, disp_prop, coff_tbl);
        }
        else
        {
            if ((dst_width == (src_width >> 1)) && (dst_height == (src_height >> 1)))
            {
                return cc16scalingHalf(src, dst, disp_prop, coff_tbl);
            }
            else
            {
                return cc16scaledown(src, dst, disp_prop, coff_tbl, _mRowPix, _mColPix);
            }
        }
    }
    else
    {
        tempw = (5 * src_width) / 4;
        temph = (5 * src_height) / 4;

        /* check for special zoom-out case, 5:4 scaling up */
        if (dst_width == tempw  &&  dst_height == temph && !(src_width&3))
        {
            return cc16scaling54(src, dst, disp_prop, coff_tbl);
        }
        else
        {
            tempw = (4 * src_width) / 3;
            temph = (4 * src_height) / 3;
            if (dst_width == tempw  &&  dst_height == temph && !(src_width&3))
            {
                return cc16scaling43(src, dst, disp_prop, coff_tbl);
            }
            else
            {
                tempw = (15 * src_width) / 8;
                temph = (15 * src_height) / 8;
#ifdef SPECIAL_SCALE_128x96
                if (dst_width == tempw  &&  dst_height == temph && !(src_width&7))
                {
                    return cc16scaling128x96(src, dst, disp_prop, coff_tbl);
                }
                else
#endif
                {
                    return cc16scaleup(src, dst, disp_prop, coff_tbl, _mRowPix, _mColPix);
                }
            }
        }

    }
}

#ifdef SPECIAL_SCALE_128x96
int32 cc16scaling128x96(uint8 **src, uint8 *dst, int32 *disp, uint8 *coff_tbl)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width, nextrow;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));
#ifdef INTERPOLATE
    int i, offset = 0;
    uint16 *pI2, *pU2, *pIn, *pNext;
    int32 tmp02, tmp01;
#endif

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (((disp[6] == 1) && (disp[7] == 1)) || ((disp[6] == 0) && (disp[7] == 0)))
    {
        if (disp[6])/* rotate 180 and flip */
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
        else // no rotate,no flip
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }

        pDst = (uint16 *)dst;

        for (row = 0; row < disp[3] - 1; row += 2)
        {
            for (col = 0; col <= src_width - 1; col += 8)
            {
                Cb = *pCb++;
                Cr = *pCr++;
                //load the bottom two pixels
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifndef INTERPOLATE
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = ((pDst + (dst_pitch << 1) + 3));
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifndef INTERPOLATE
                *((pDst + 1)) = rgb;
#endif
                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb++;
                Cr = *pCr++;
                //load the bottom two pixels
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = (pDst + (dst_pitch << 1) + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif
                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb++;
                Cr = *pCr++;
                //load the bottom two pixels
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif
                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = (pDst + (dst_pitch << 1) + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif
                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb++;
                Cr = *pCr++;
                //load the bottom two pixels
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *(pDst + (dst_pitch << 1) + 2) = tmp0;
#ifdef INTERPOLATE
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0    = (Y & 0xFF);   //Low endian    left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif
                Y   = (Y >> 8) & 0xFF;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *(pDst + 2)	= tmp0;
#ifdef INTERPOLATE
                *((pDst + 1)) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                pDst += 3;

            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;

            pDst -= (disp[4]);
#ifdef INTERPOLATE
            /* vertically - 1 1' 2 2' 3 3' 4 4' 5 5' 6 6' 7 7' 8 1 1'2 2'........*/
            if ((row & 0x7))
            {
                pIn = pDst - dst_pitch;
                pNext = pIn - dst_pitch;
                for (i = 0;i < disp[4]; i++)
                {
                    int32 curr = pDst[i];
                    int32 below = pNext[i];
                    pIn[i] = (((((curr      & 0x1F)  + (below     & 0x1F)) / 2) & 0x1F)
                              | ((((((curr >> 5) & 0x3F)  + ((below >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                              | ((((((curr >> 11) & 0x1F)  + ((below >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
                offset++;
            }

            pNext = pDst + (dst_pitch * 2);
            pIn = pDst + dst_pitch;
            for (i = 0;i < disp[4]; i++)
            {
                int32 curr = pDst[i];
                int32 below = pNext[i];
                pIn[i] = (((((curr      & 0x1F)  + (below     & 0x1F)) / 2) & 0x1F)
                          | ((((((curr >> 5) & 0x3F)  + ((below >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                          | ((((((curr >> 11) & 0x1F)  + ((below >> 11) & 0x1F)) / 2) & 0x1F) << 11));
            }

            if (offset == 3)
            {
                pDst = pNext + dst_pitch;
                offset = 0;
            }
            else
                pDst = pNext + dst_pitch * 2;
#else
            oscl_memcpy(pDst + dst_pitch, pDst, (disp[4] << 1));
            pDst += (dst_pitch << 1);

            if (row & 0x7)
            {
                oscl_memcpy(pDst + dst_pitch, pDst, (disp[4] << 1));
                pDst +=	dst_pitch;	//coz pDst defined as UINT *
            }
            pDst += dst_pitch;
#endif
        }
    }
    else
    {
        if (disp[6])/* rotation 180 only */
        {
            /* move the starting point to the bottom-right corner of the picture */
            nextrow = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + nextrow + src_width - 2);
            nextrow = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + nextrow + (src_width >> 1) - 1;
            pCr = src[2] + nextrow + (src_width >> 1) - 1;
            nextrow = -(src_pitch >> 1);
            deltaY		=	src_width - (src_pitch << 1);
            deltaCbCr	=	(src_width - src_pitch) >> 1;
        }
        else /* flip only */
        {
            /* move the starting point to the top-right corner of the picture */
            pY = (uint16 *)(src[0] + src_width - 2);
            pCb = src[1] + (src_width >> 1) - 1;
            pCr = src[2] + (src_width >> 1) - 1;
            nextrow = src_pitch >> 1;
            deltaY = src_width + (src_pitch << 1);
            deltaCbCr = (src_width + src_pitch) >> 1;
        }
        pDst = (uint16 *)dst;
        for (row = 0; row < disp[3] - 1; row += 2)
        {
            for (col = 0; col <= src_width - 1; col += 8)
            {
                Cb = *pCb--;
                Cr = *pCr--;
                //load the bottom two pixels
                Y = pY[nextrow];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifndef INTERPOLATE
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = ((pDst + (dst_pitch << 1) + 3));
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif
                //load the top two pixels
                Y = *pY--;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                //tmp0	=	*pY++;
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifndef INTERPOLATE
                *((pDst + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb--;
                Cr = *pCr--;
                //load the bottom two pixels
                Y = pY[nextrow];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1- OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif
                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = (pDst + (dst_pitch << 1) + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif

                //load the top two pixels
                Y = *pY--;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                //tmp0	=	*pY++;
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb--;
                Cr = *pCr--;
                //load the bottom two pixels
                Y = pY[nextrow];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1- OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp02	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp02;
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pI2 = (pDst + (dst_pitch << 1) + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + (dst_pitch << 1) + 2) = tmp0;
                *(pDst + (dst_pitch << 1) + 3) = tmp0;
#endif

                //load the top two pixels
                Y = *pY--;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                //tmp0	=	*pY++;
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
#ifdef INTERPOLATE
                tmp01	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp01;
                *((pDst + 1)) = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                pU2 = (pDst + 3);
#else
                tmp0	=	tmp2 | (tmp0 << 5);
                *(pDst + 2)	= tmp0;
                *(pDst + 3) = tmp0;
#endif
                pDst += 4;

                Cb = *pCb--;
                Cr = *pCr--;
                //load the bottom two pixels
                Y = pY[nextrow];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1- OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *((pDst + (dst_pitch << 1))) = rgb;
#ifdef INTERPOLATE
                *pI2 = (((((tmp02      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp02 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp02 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + (dst_pitch << 1) + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *(pDst + (dst_pitch << 1) + 2) = tmp0;
#ifdef INTERPOLATE
                *((pDst + (dst_pitch << 1) + 1)) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                                    | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                                    | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                //load the top two pixels
                Y = *pY--;

                tmp0	=	(Y >> 8) & 0xFF;	//Low endian	left pixel
                //tmp0	=	*pY++;
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *((pDst)) = rgb;
#ifdef INTERPOLATE
                *pU2 = (((((tmp01      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                        | ((((((tmp01 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                        | ((((((tmp01 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#else
                *((pDst + 1)) = rgb;
#endif

                Y	    =   Y & 0xFF;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *(pDst + 2)	= tmp0;
#ifdef INTERPOLATE
                *((pDst + 1)) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                pDst += 3;

            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;

            pDst -= (disp[4]);
#ifdef INTERPOLATE
            /* vertically - 1 1' 2 2' 3 3' 4 4' 5 5' 6 6' 7 7' 8 1 1'2 2'........*/
            if ((row & 0x7))
            {
                pIn = pDst - dst_pitch;
                pNext = pIn - dst_pitch;
                for (i = 0;i < disp[4]; i++)
                {
                    int32 curr = pDst[i];
                    int32 below = pNext[i];
                    pIn[i] = (((((curr      & 0x1F)  + (below     & 0x1F)) / 2) & 0x1F)
                              | ((((((curr >> 5) & 0x3F)  + ((below >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                              | ((((((curr >> 11) & 0x1F)  + ((below >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
                offset++;
            }

            pNext = pDst + (dst_pitch * 2);
            pIn = pDst + dst_pitch;
            for (i = 0;i < disp[4]; i++)
            {
                int32 curr = pDst[i];
                int32 below = pNext[i];
                pIn[i] = (((((curr      & 0x1F)  + (below     & 0x1F)) / 2) & 0x1F)
                          | ((((((curr >> 5) & 0x3F)  + ((below >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                          | ((((((curr >> 11) & 0x1F)  + ((below >> 11) & 0x1F)) / 2) & 0x1F) << 11));
            }

            if (offset == 3)
            {
                pDst = pNext + dst_pitch;
                offset = 0;
            }
            else
                pDst = pNext + dst_pitch * 2;
#else
            oscl_memcpy(pDst + dst_pitch, pDst, (disp[4] << 1));
            pDst += (dst_pitch << 1);

            if (row & 0x7)
            {
                oscl_memcpy(pDst + dst_pitch, pDst, (disp[4] << 1));
                pDst +=	dst_pitch;	//coz pDst defined as UINT *
            }
            pDst += dst_pitch;
#endif
        }
    }

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coeff_tbl);
    return 0;
#endif // CCSCALING
}
#endif

/* special function when scale down in width */
int32 cc16scaledown(uint8 **src, uint8 *dst, int32 *disp,
                    uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING
    /*	1. move the dst pointer to the line above the border
    2. do 2 line conversion
    3. copy both up & down
    	*/
    uint8 *pCb, *pCr;
    uint8	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, dst_width, deltaCbCr, src_inc;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint8 *clip = coff_tbl + 400;
    uint8 *rowpix, *colpix;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];


    if (((disp[6] == 0) && (disp[7] == 1)) || ((disp[6] == 1) && (disp[7] == 0)))  // rotate 0 and flip || // rotate 180 and  no flip
    {
        if (disp[6])/* rotation 180 only */
        {
            /* move the starting point to the bottom-right corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (src[0] + deltaY + src_width - 2);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY + (src_width >> 1) - 1;
            pCr = src[2] + deltaY + (src_width >> 1) - 1;
            deltaY		=	src_width - (src_pitch << 1);
            deltaCbCr	=	(src_width - src_pitch) >> 1;
            src_pitch = -src_pitch;
        }
        else /* flip only */
        {
            /* move the starting point to the top-right corner of the picture */
            pY = (src[0] + src_width - 2);
            pCb = src[1] + (src_width >> 1) - 1;
            pCr = src[2] + (src_width >> 1) - 1;
            deltaY = src_width + (src_pitch << 1);
            deltaCbCr = (src_width + src_pitch) >> 1;
        }

        src_inc = -1;
    }
    else // rotate 180 and flip ||  no rotate, no flip
    {
        if (disp[6])	// rotate 180 and flip
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = src[0] + deltaY;
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY;
            pCr = src[2] + deltaY;
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            src_pitch = - src_pitch;
        }
        else  // no rotate, no flip
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            dst_width	=	disp[4];
            pY = src[0];
            pCb = src[1];
            pCr = src[2];
        }

        src_inc = 1;
    }

    pDst = (uint16 *)dst;

    colpix = _mColPix + disp[3] - 1;

    for (row = disp[3] - 1; row >= 0; row -= 2)
    {/* decrement index, _mColPix[.] is
		symmetric to increment index */
        if (colpix[-1] + colpix[0] == 0)
        {
            pCb += (src_pitch >> 1);
            pCr += (src_pitch >> 1);
            pY += (src_pitch << 1);
            colpix -= 2;
            continue;
        }

        rowpix = _mRowPix + src_width - 1;

        if (colpix[-1] + colpix[0] == 1) // one line not skipped
        {
            for (col = src_width - 2; col >= 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                Cb = *pCb;
                (pCb += src_inc);
                Cr = *pCr;
                (pCr += src_inc);

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (*rowpix) /* compute this pixel */
                {
                    Y	=	*pY;
                    (pY += src_inc);						//upper left

                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + 1024];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	= (uint16)tmp0;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += src_inc;
                }
                pDst	+= *rowpix--;

                if (*rowpix) /* compute this pixel */
                {
                    Y	=	*pY;
                    (pY += src_inc);							//upper right

                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + 1024];
                    tmp2	=	clip[tmp2];

                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	= (uint16)tmp0;
                }
                else  /* if(_mRowPix[col+1]) */
                {
                    pY += src_inc;
                }

                pDst	+= *rowpix--;
            }//end of COL

        }  // one line not skipped

        else  //both lines not skipped

        {
            for (col = src_width - 2; col >= 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */

                Cb = *pCb;
                (pCb += src_inc);
                Cr = *pCr;
                (pCr += src_inc);

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (*rowpix) /* compute this pixel */
                {
                    Y	=	pY[src_pitch];						//lower left

                    Y += OFFSET_5_1;
                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst + dst_pitch)	= (uint16)tmp0;

                    Y	=	*pY;
                    (pY += src_inc);						//upper left

                    Y += OFFSET_5_0;
                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	= (uint16)tmp0;

                }
                else  /* if(_mRowPix[col]) */
                {
                    pY += src_inc;
                }
                pDst	+= *rowpix--;

                if (*rowpix) /* compute this pixel */
                {
                    Y	=	pY[src_pitch];						//lower left

                    Y += OFFSET_5_0;
                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst + dst_pitch)	= (uint16)tmp0;

                    Y	=	*pY;
                    (pY += src_inc);							//upper right

                    Y += OFFSET_5_1;
                    tmp1	=	Y - (Cg >> 16);
                    tmp2	=	Y + (Cb >> 16);
                    tmp0	=	Y + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];

                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	= (uint16)tmp0;
                }
                else  /* if(_mRowPix[col+1]) */
                {
                    pY += src_inc;
                }

                pDst	+= *rowpix--;
            }//end of COL

            pDst += dst_pitch;

        }

        pY	+=	(deltaY);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;

        pDst	+=	dst_pitch - disp[4];
        colpix -= 2;

    }

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);
    OSCL_UNUSED_ARG(_mRowPix);
    OSCL_UNUSED_ARG(_mColPix);
    return 0;
#endif // CCSCALING
}

/* special scaling by one-half */
int32 cc16scalingHalf(uint8 **src, uint8 *dst, int32 *disp, uint8 *coff_tbl)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width, nextrow;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (((disp[6] == 1) && (disp[7] == 1)) || ((disp[6] == 0) && (disp[7] == 0)))
    {
        if (disp[6])/* rotate 180 and flip */
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
        else // no rotate,no flip
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }

        pDst = (uint16 *)dst;

        for (row = 0; row < disp[3] - 1; row += 2)
        {
            for (col = 0; col <= src_width - 1; col += 2)
            {
                Cb = *pCb++;
                Cr = *pCr++;
                Y = *pY++;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + 1024];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *pDst++ = rgb;
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            pDst -= (disp[4]);
            pDst += dst_pitch;
        }
    }
    else
    {
        if (disp[6])/* rotation 180 only */
        {
            /* move the starting point to the bottom-right corner of the picture */
            nextrow = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + nextrow + src_width - 2);
            nextrow = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + nextrow + (src_width >> 1) - 1;
            pCr = src[2] + nextrow + (src_width >> 1) - 1;
            nextrow = -(src_pitch >> 1);
            deltaY		=	src_width - (src_pitch << 1);
            deltaCbCr	=	(src_width - src_pitch) >> 1;
        }
        else /* flip only */
        {
            /* move the starting point to the top-right corner of the picture */
            pY = (uint16 *)(src[0] + src_width - 2);
            pCb = src[1] + (src_width >> 1) - 1;
            pCr = src[2] + (src_width >> 1) - 1;
            nextrow = src_pitch >> 1;
            deltaY = src_width + (src_pitch << 1);
            deltaCbCr = (src_width + src_pitch) >> 1;
        }
        pDst = (uint16 *)dst;
        for (row = 0; row < disp[3] - 1; row += 2)
        {
            for (col = 0; col <= src_width - 1; col += 2)
            {
                Cb = *pCb--;
                Cr = *pCr--;
                Y = *pY--;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + 1024];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *pDst++ = rgb;
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;

            pDst -= (disp[4]);
            pDst += dst_pitch;
        }
    }

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);
    return 0;
#endif // CCSCALING
}


/* special 3:4 scale-down color conversion */
int32 cc16scaling34(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaDst, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (((disp[6] == 0) && (disp[7] == 1)) || ((disp[6] == 1) && (disp[7] == 0))) /* rotate 180 and  no flip, rotate 0 and  flip *///Ankur
    {
        if (disp[6] == 0)
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }
        else
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + deltaY);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY;
            pCr = src[2] + deltaY;
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            src_pitch = -(src_pitch >> 1);
        }

        pDst =	(uint16 *)dst + disp[4] - 1;
        deltaDst	=	(dst_pitch << 1) + disp[4]; /* disp[4] is dst_width */

        for (row = disp[3]; row > 0; row -= 2)
        {

            for (col = src_width - 1; col >= 0; col -= 4)  /* do 8 pixels at a time, 4 ups 4 downs */
            {

                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (!(row&0x2)) /* do the bottom row every other times */
                {
                    //load the bottom two pixels
                    Y = pY[src_pitch];

                    tmp0	=	Y & 0xFF;	//Low endian	left pixel
                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];
                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);
                    *(pDst + dst_pitch) = rgb;  /* save left pixel, have to save separately */

                    Y	>>=	8;
                    Y	+= OFFSET_5_1;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];
                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst + dst_pitch - 1) = tmp0; /* save right pixel */
                }
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *pDst--	=	rgb;	/* save left pixel */

                Y	>>=	8;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *pDst--	= tmp0;	/* save right pixel */

                /* now do another 4 pixels but drop 2 pixels in the last column */
                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                //load the bottom two pixels
                if (!(row&0x2)) /* do the bottom row every other times */
                {
                    Y = pY[src_pitch];

                    tmp0	=	Y & 0xFF;	//Low endian	left pixel
                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];
                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    *(pDst + dst_pitch) = rgb;  /* save only one pixel, 2 bytes */
                }
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *pDst	= rgb;
                pDst--; /* save only one pixel, 2 bytes */
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            pDst +=	(deltaDst);	//coz pDst defined as UINT *
            if (row&0x2)
            {
                pDst -= dst_pitch;
            }
        }
    }
    else /* rotate 180 and flip || no rotation,no flip*/
    {
        if (disp[6]) /* rotate 180 and flip */
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + deltaY);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY;
            pCr = src[2] + deltaY;
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            src_pitch = -(src_pitch >> 1);
        }
        else // no rotation,no flip
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }

        deltaDst	=	(dst_pitch << 1) - disp[4]; /* disp[4] is dst_width */
        pDst =	(uint16 *)dst;

        for (row = disp[3]; row > 0; row -= 2)
        {

            for (col = src_width - 1; col >= 0; col -= 4)  /* do 8 pixels at a time, 4 ups 4 downs */
            {

                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (!(row&0x2)) /* do the bottom row every other times */
                {	//load the bottom two pixels
                    Y = pY[src_pitch];

                    tmp0	=	Y & 0xFF;	//Low endian	left pixel
                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];
                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);
                    *(pDst + dst_pitch) = rgb;  /* save left pixel, have to save separately */

                    Y	>>=	8;
                    Y	+= OFFSET_5_1;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];
                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst + dst_pitch + 1) = tmp0; /* save right pixel */
                }
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *pDst++	=	rgb;	/* save left pixel */

                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                *pDst++	= tmp0;	/* save right pixel */

                /* now do another 4 pixels but drop 2 pixels in the last column */
                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                //load the bottom two pixels
                if (!(row&0x2)) /* do the bottom row every other times */
                {
                    Y = pY[src_pitch];

                    tmp0	=	Y & 0xFF;	//Low endian	left pixel
                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];
                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    *(pDst + dst_pitch) = rgb;  /* save only one pixel, 2 bytes */
                }
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *pDst	= rgb;
                pDst++; /* save only one pixel, 2 bytes */
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            pDst +=	(deltaDst);	//coz pDst defined as UINT *
            if (row&0x2)
            {
                pDst -= dst_pitch;
            }
        }
    }
    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);
    return 0;
#endif // CCSCALING
}


int32 cc16scaleup(uint8 **src, uint8 *dst, int32 *disp,
                  uint8 *coff_tbl, uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING
    /*	1. move the dst pointer to the line above the border
    2. do 2 line conversion
    3. copy both up & down
    	*/
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y,	Cb, Cr, Cg;
    int32		deltaY, dst_width, deltaCbCr;
    int32		row, col;
    int32	tmp0, tmp1, tmp2, temp;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32		offset;
    uint8 *rowpix, *colpix;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

#ifdef INTERPOLATE
    int i;
    uint32	copyline = 0;	//maru
    uint16	*prev_pDst = 0;	//maru
    int32	prev_offset = 0;	//maru
#endif

    if (((disp[6] == 0) && (disp[7] == 1)) || ((disp[6] == 1) && (disp[7] == 0))) /* rotate 180 and  no flip || rotate 0 and  with flip */ //Ankur
    {
        if (disp[6] == 0) /*rotate 0 and  with flip */
            pDst =	(uint16 *)dst + disp[4] - 1;
        else /* rotate 180 and  no flip */
        {
            pDst =	((uint16 *)dst) + disp[1] * (disp[5] - 1) + disp[4] - 1;
            dst_pitch = -dst_pitch;
        }

        deltaY		=	(src_pitch << 1) - src_width;
        deltaCbCr	=	(src_pitch - src_width) >> 1;
        dst_width	=	disp[4];
        pY = (uint16*)src[0];
        pCb = src[1];
        pCr = src[2];
        src_pitch >>= 1;
        colpix = _mColPix + disp[3] - 1;

        for (row = disp[3] - 1; row >= 0; row -= 2)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */
            rowpix = _mRowPix + src_width - 1;

            for (col = src_width - 2; col >= 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */
                Cb = *pCb++;
                Cr = *pCr++;
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	Y & 0xFF;			//bottom left
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                //RGB_565

                rgb	=	tmp1 | (tmp0 << 6);
                rgb	=	tmp2 | (rgb << 5);

                Y	>>= 8;						//bottom right
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                pDst += dst_pitch;
                temp = rowpix[0] + rowpix[-1];
                rowpix -= 2;
                if (temp == 2)
                {
                    *pDst = rgb;
                    *(pDst - 1) = tmp0;
                }
                else if (temp == 3)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                    *(pDst - 2) = tmp0;
                }
                else if (temp == 4)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));
#endif
                    *(pDst - 3) = tmp0;
                }
                else if (temp == 5)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = tmp0;
                    *(pDst - 3) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 3) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 3) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 3) / 4) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(pDst - 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F)) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F)) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F)) / 4) & 0x1F) << 11));
#endif
                    *(pDst - 4) = tmp0;
                }
                else /* temp ==6 */
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = rgb;
                    *(pDst - 3) = tmp0;
                    *(pDst - 4) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 4) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 4) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 4) / 5) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F) * 3) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F) * 3) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F) * 3) / 5) & 0x1F) << 11));
                    *(pDst - 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F) * 2) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F) * 2) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F) * 2) / 5) & 0x1F) << 11));
                    *(pDst - 4) = (((((tmp0      & 0x1F) * 4 + (rgb     & 0x1F)) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 4 + ((rgb >> 5) & 0x3F)) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 4 + ((rgb >> 11) & 0x1F)) / 5) & 0x1F) << 11));
#endif
                    *(pDst - 5) = tmp0;
                }

                pDst -= dst_pitch;

                Y = *pY++;
                tmp0	=		Y & 0xFF;		//top left
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb	=	tmp1 | (tmp0 << 6);
                rgb	=	tmp2 | (rgb << 5);

                Y	>>=	8;					//top right

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);
                if (temp == 2)
                {
                    *pDst = rgb;
                    *(pDst - 1) = tmp0;
                }
                else if (temp == 3)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                    *(pDst - 2) = tmp0;
                }
                else if (temp == 4)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));
#endif
                    *(pDst - 3) = tmp0;
                }
                else if (temp == 5)
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = tmp0;
                    *(pDst - 3) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 3) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 3) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 3) / 4) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(pDst - 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F)) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F)) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F)) / 4) & 0x1F) << 11));
#endif
                    *(pDst - 4) = tmp0;
                }
                else /* temp ==6 */
                {
                    *pDst = rgb;
#ifndef INTERPOLATE
                    *(pDst - 1) = rgb;
                    *(pDst - 2) = rgb;
                    *(pDst - 3) = tmp0;
                    *(pDst - 4) = tmp0;
#else
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 4) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 4) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 4) / 5) & 0x1F) << 11));
                    *(pDst - 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F) * 3) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F) * 3) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F) * 3) / 5) & 0x1F) << 11));
                    *(pDst - 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F) * 2) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F) * 2) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F) * 2) / 5) & 0x1F) << 11));
                    *(pDst - 4) = (((((tmp0      & 0x1F) * 4 + (rgb     & 0x1F)) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 4 + ((rgb >> 5) & 0x3F)) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 4 + ((rgb >> 11) & 0x1F)) / 5) & 0x1F) << 11));
#endif
                    *(pDst - 5) = tmp0;
                }
                pDst -= temp;

            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;

            pDst++; // goes back to the beginning of the line

#ifdef INTERPOLATE

            if (copyline&0x1)  	//Maruyama comment:BLACK line in SubQCIF.bmp
            {
                oscl_memcpy(prev_pDst + offset,             prev_pDst + dst_pitch, dst_width*2);
            }
            if (copyline&0x2) 	//Maruyama comment:BLUE line in SubQCIF.bmp
            {
                for (i = 0 ; i < dst_width ; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    //				int32 curtemp = *(pDst+dst_pitch+i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch+i) = 0x1F;
                }
            }

            if (copyline&0x4)
            {
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    //				int32 curtemp = *(pDst+dst_pitch+i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch+i) = 0x3F<<5;
                }
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    int32 curtemp = *(pDst + dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch*2 + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch*2+i) = 0x1F<<11;
                }
            }

            if (copyline&0x8) 	//Maruyama comment:WHITE line in SubQCIF.bmp
            {
                for (i = 0 ; i < dst_width ; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *(pDst - dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch+i) = 0xFFFF;
                }
            }

            if (copyline&0x10)
            {
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    //				int32 curtemp = *(pDst+i);
                    int32 curtemp = *(pDst - dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch+i) = 0x1F|(0x3F<<5);
                }
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch*2 + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch*2+i) = 0x1F|(0x1F<<11);
                }
            }
#endif
            //copy down
            offset = (colpix[0] * dst_pitch);

#ifdef INTERPOLATE
            copyline = 0;	//maru
            prev_pDst = pDst;	//maru
            prev_offset = offset;	//maru
#endif
            if (colpix[-1] && colpix[0] != 1)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset,             pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x1;	//maru
#endif
            }
            if (colpix[-1] == 2)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset + dst_pitch,   pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x2;	//maru
#endif
            }
            else if (colpix[-1] == 3)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset + dst_pitch,   pDst + dst_pitch, dst_width*2);
                oscl_memcpy(pDst + offset + dst_pitch*2, pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x4;	//maru
#endif
            }
            //copy up
            if (colpix[0] == 2)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + dst_pitch,          pDst,           dst_width*2);
#else
                copyline |= 0x8;	//maru
#endif
            }
            else if (colpix[0] == 3)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + dst_pitch,          pDst,           dst_width*2);
                oscl_memcpy(pDst + dst_pitch*2,        pDst,           dst_width*2);
#else
                copyline |= 0x10;	//maru
#endif
            }

            pDst	+=	dst_pitch * (colpix[-1] + colpix[0]) + dst_width - 1;
            colpix -= 2;
        }//end of row

#ifdef INTERPOLATE

        if (copyline&0x1)  	//Maruyama comment:BLACK line in SubQCIF.bmp
        {
            oscl_memcpy(prev_pDst + offset,             prev_pDst + dst_pitch, dst_width*2);
        }
        if (copyline&0x2) 	//Maruyama comment:BLUE line in SubQCIF.bmp
        {
            oscl_memcpy(prev_pDst + offset + dst_pitch,   prev_pDst + dst_pitch, dst_width*2);
        }

        if (copyline&0x4)
        {
            oscl_memcpy(prev_pDst + offset + dst_pitch,   prev_pDst + dst_pitch, dst_width*2);
            oscl_memcpy(prev_pDst + offset + dst_pitch*2, prev_pDst + dst_pitch, dst_width*2);
        }
#endif
    }
    else  /* rotate 180 and  with flip || no rotation ,no flip */
    {
        if (disp[6] == 1) /* rotate 180 and  with flip */ //Ankur
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (uint16 *)src[0] + (deltaY >> 1);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + (deltaY >> 0);
            pCr = src[2] + (deltaY >> 0);
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            dst_width	=	disp[4];
            src_pitch >>= 1;
            src_pitch = -src_pitch;
        }
        else
        {	// only scale up, no rotation ,no flip
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            dst_width	=	disp[4];
            src_pitch >>= 1;
            pY = (uint16*)src[0];
            pCb = src[1];
            pCr = src[2];
        }

        pDst =	(uint16 *)dst;
        colpix = _mColPix + disp[3] - 1;

        for (row = disp[3] - 1; row >= 0; row -= 2)
        {/* decrement index, _mColPix[.] is
			symmetric to increment index */
            rowpix = _mRowPix + src_width - 1;

            for (col = src_width - 2; col >= 0; col -= 2)
            { /* decrement index, _mRowPix[.] is
				symmetric to increment index */
                Cb = *pCb++;
                Cr = *pCr++;
                Y = pY[src_pitch];

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                tmp0	=	Y & 0xFF;			//bottom left
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                //RGB_565

                rgb	=	tmp1 | (tmp0 << 6);
                rgb	=	tmp2 | (rgb << 5);

                Y	>>= 8;						//bottom right
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                pDst += dst_pitch;
                temp = rowpix[0] + rowpix[-1];
                rowpix -= 2;
                if (temp == 2)
                {
                    *pDst = (uint16)rgb;
                    *(pDst + 1) = (uint16)tmp0;
                }
                else if (temp == 3)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                    *(pDst + 2) = (uint16)tmp0;
                }
                else if (temp == 4)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));
#endif
                    *(pDst + 3) = (uint16)tmp0;
                }
                else if (temp == 5)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)tmp0;
                    *(pDst + 3) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 3) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 3) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 3) / 4) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(pDst + 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F)) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F)) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F)) / 4) & 0x1F) << 11));
#endif
                    *(pDst + 4) = (uint16)tmp0;
                }
                else /* temp ==6 */
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)rgb;
                    *(pDst + 3) = (uint16)tmp0;
                    *(pDst + 4) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 4) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 4) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 4) / 5) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F) * 3) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F) * 3) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F) * 3) / 5) & 0x1F) << 11));
                    *(pDst + 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F) * 2) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F) * 2) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F) * 2) / 5) & 0x1F) << 11));
                    *(pDst + 4) = (((((tmp0      & 0x1F) * 4 + (rgb     & 0x1F)) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 4 + ((rgb >> 5) & 0x3F)) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 4 + ((rgb >> 11) & 0x1F)) / 5) & 0x1F) << 11));
#endif
                    *(pDst + 5) = (uint16)tmp0;
                }

                pDst -= dst_pitch;

                Y = *pY++;
                tmp0	=		Y & 0xFF;		//top left
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                rgb	=	tmp1 | (tmp0 << 6);
                rgb	=	tmp2 | (rgb << 5);

                Y	>>=	8;					//top right
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);
                if (temp == 2)
                {
                    *pDst = (uint16)rgb;
                    *(pDst + 1) = (uint16)tmp0;
                }
                else if (temp == 3)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                    *(pDst + 2) = (uint16)tmp0;
                }
                else if (temp == 4)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));
#endif
                    *(pDst + 3) = (uint16)tmp0;
                }
                else if (temp == 5)
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)tmp0;
                    *(pDst + 3) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 3) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 3) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 3) / 4) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(pDst + 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F)) / 4) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F)) / 4) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F)) / 4) & 0x1F) << 11));
#endif
                    *(pDst + 4) = (uint16)tmp0;
                }
                else /* temp ==6 */
                {
                    *pDst = (uint16)rgb;
#ifndef INTERPOLATE
                    *(pDst + 1) = (uint16)rgb;
                    *(pDst + 2) = (uint16)rgb;
                    *(pDst + 3) = (uint16)tmp0;
                    *(pDst + 4) = (uint16)tmp0;
#else
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F) * 4) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 4) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 4) / 5) & 0x1F) << 11));
                    *(pDst + 2) = (((((tmp0      & 0x1F) * 2 + (rgb     & 0x1F) * 3) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F) * 3) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F) * 3) / 5) & 0x1F) << 11));
                    *(pDst + 3) = (((((tmp0      & 0x1F) * 3 + (rgb     & 0x1F) * 2) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 3 + ((rgb >> 5) & 0x3F) * 2) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 3 + ((rgb >> 11) & 0x1F) * 2) / 5) & 0x1F) << 11));
                    *(pDst + 4) = (((((tmp0      & 0x1F) * 4 + (rgb     & 0x1F)) / 5) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F) * 4 + ((rgb >> 5) & 0x3F)) / 5) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F) * 4 + ((rgb >> 11) & 0x1F)) / 5) & 0x1F) << 11));
#endif
                    *(pDst + 5) = (uint16)tmp0;
                }
                pDst += temp;

            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;

            pDst -=	(disp[4]);	//goes back to the beginning of the line;

#ifdef INTERPOLATE

            if (copyline&0x1)  	//Maruyama comment:BLACK line in SubQCIF.bmp
            {
                oscl_memcpy(prev_pDst + offset,             prev_pDst + dst_pitch, dst_width*2);
            }
            if (copyline&0x2) 	//Maruyama comment:BLUE line in SubQCIF.bmp
            {
                for (i = 0 ; i < dst_width ; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    //				int32 curtemp = *(pDst+dst_pitch+i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch+i) = 0x1F;
                }
            }

            if (copyline&0x4)
            {
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    //				int32 curtemp = *(pDst+dst_pitch+i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch+i) = 0x3F<<5;
                }
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + dst_pitch + i);
                    int32 curtemp = *(pDst + dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + prev_offset + dst_pitch*2 + i) = (uint16)coltemp;
                    //				*(prev_pDst+prev_offset+dst_pitch*2+i) = 0x1F<<11;
                }
            }

            if (copyline&0x8) 	//Maruyama comment:WHITE line in SubQCIF.bmp
            {
                for (i = 0 ; i < dst_width ; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *(pDst - dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch+i) = 0xFFFF;
                }
            }

            if (copyline&0x10)
            {
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    //				int32 curtemp = *(pDst+i);
                    int32 curtemp = *(pDst - dst_pitch + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch+i) = 0x1F|(0x3F<<5);
                }
                for (i = 0 ; i < dst_width ; i++)  	//Maruyama comment:Not appeared in SubQCIF.bmp
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *(pDst + i);
                    coltemp   = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                                 | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch*2 + i) = (uint16)coltemp;
                    //				*(prev_pDst+dst_pitch*2+i) = 0x1F|(0x1F<<11);
                }
            }
#endif
            //copy down
            offset = (colpix[0] * dst_pitch);

#ifdef INTERPOLATE
            copyline = 0;	//maru
            prev_pDst = pDst;	//maru
            prev_offset = offset;	//maru
#endif
            if (colpix[-1] && colpix[0] != 1)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset, pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x1;	//maru
#endif
            }
            if (colpix[-1] == 2)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset + dst_pitch, pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x2;	//maru
#endif
            }
            else if (colpix[-1] == 3)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + offset + dst_pitch, pDst + dst_pitch, dst_width*2);
                oscl_memcpy(pDst + offset + dst_pitch*2, pDst + dst_pitch, dst_width*2);
#else
                copyline |= 0x4;	//maru
#endif
            }

            //copy up
            if (colpix[0] == 2)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
#else
                copyline |= 0x8;	//maru
#endif
            }
            else if (colpix[0] == 3)
            {
#ifndef INTERPOLATE
                oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
                oscl_memcpy(pDst + dst_pitch*2, pDst, dst_width*2);
#else
                copyline |= 0x10;	//maru
#endif
            }

            pDst	+=	dst_pitch * (colpix[-1] + colpix[0]);
            colpix -= 2;
        }//end of row

#ifdef INTERPOLATE
        if (copyline&0x1)  	//Maruyama comment:BLACK line in SubQCIF.bmp
        {
            oscl_memcpy(prev_pDst + offset,             prev_pDst + dst_pitch, dst_width*2);
        }
        if (copyline&0x2) 	//Maruyama comment:BLUE line in SubQCIF.bmp
        {
            oscl_memcpy(prev_pDst + offset + dst_pitch, prev_pDst + dst_pitch, dst_width*2);
        }

        if (copyline&0x4)
        {
            oscl_memcpy(prev_pDst + offset + dst_pitch, prev_pDst + dst_pitch, dst_width*2);
            oscl_memcpy(prev_pDst + offset + dst_pitch*2, prev_pDst + dst_pitch, dst_width*2);
        }
#endif
    }

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);
    OSCL_UNUSED_ARG(_mRowPix);
    OSCL_UNUSED_ARG(_mColPix);

    return 0;
#endif // CCSCALING
}

/* special 5:4 scale-up color conversion */
int32 cc16scaling54(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaDst, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32 rgb1, rgb2;
    int32  tmp01, tmp02;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));
#ifdef INTERPOLATE
    int copyline = 0, i;
    uint16 *pFifth, *previous, *current;
#endif

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];


    if (((disp[6] == 1) && (disp[7] == 0)) || ((disp[6] == 0) && (disp[7] == 1))) /* rotate 180, no flip  || rotate 0 and flip */
    {
        if (disp[6])
        {
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
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }

        deltaDst	=	(dst_pitch << 1) + disp[4] - 1; /* disp[4] is dst_width */
        pDst =	(uint16 *)dst + disp[4] - 1;
        for (row = disp[3]; row > 0; row -= 2)
        {

            for (col = src_width - 1; col >= 0; col -= 4)  /* do 8 pixels at a time, 4 ups 2 downs */
            {

                Cb = *pCb++;
                Cr = *pCr++;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *(pDst + dst_pitch) = rgb;  // i1 = p1;/* save left pixel, have to save separately */

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp02	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *(pDst + dst_pitch - 1) = (((((tmp02 & 0x1F) * 3  + (rgb & 0x1F) + 2) / 4) & 0x1F)
                                           | ((((((tmp02 >> 5) & 0x3F) * 3  + ((rgb >> 5) & 0x3F)  + 2) / 4) & 0x3F) << 5)
                                           | ((((((tmp02 >> 11) & 0x1F) * 3  + ((rgb >> 11) & 0x1F)  + 2) / 4) & 0x1F) << 11));
                //i2 = (p1 + 3*p2 + 2)>>2;
#else
                *(pDst + dst_pitch - 1) = tmp02; /* save right pixel */
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *pDst-- =	rgb; // i1 = p1;	/* save left pixel */

                Y	>>=	8;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp01	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *(pDst--) = (((((tmp01 & 0x1F) * 3  + (rgb & 0x1F) + 2) / 4) & 0x1F)
                             | ((((((tmp01 >> 5) & 0x3F) * 3  + ((rgb >> 5) & 0x3F)  + 2) / 4) & 0x3F) << 5)
                             | ((((((tmp01 >> 11) & 0x1F) * 3  + ((rgb >> 11) & 0x1F)  + 2) / 4) & 0x1F) << 11));
                //i2 = (p1 + 3*p2 + 2)>>2;
#else
                *pDst--	= tmp01;	/* save right pixel */
#endif
                /* now do another 4 pixels add 1 pixels in the last column */
                Cb = *pCb++;
                Cr = *pCr++;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb2		=	tmp2 | (rgb << 5);

#ifdef INTERPOLATE
                // i3 = (p2 + p3 + 1)>>1;
                *(pDst + dst_pitch) = (((((tmp02 & 0x1F)  + (rgb2 & 0x1F) + 1) / 2) & 0x1F)
                                       | ((((((tmp02 >> 5) & 0x3F)  + ((rgb2 >> 5) & 0x3F)  + 1) / 2) & 0x3F) << 5)
                                       | ((((((tmp02 >> 11) & 0x1F)  + ((rgb2 >> 11) & 0x1F)  + 1) / 2) & 0x1F) << 11));
#else
                *(pDst + dst_pitch) = rgb2;  /* save left pixel, have to save separately */
#endif

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                //i4 = (3*p3 + p4 + 2)>>2;
                *(pDst + dst_pitch - 1) = (((((tmp0 & 0x1F)  + (rgb2 & 0x1F) * 3 + 1) / 4) & 0x1F)
                                           | ((((((tmp0 >> 5) & 0x3F)  + ((rgb2 >> 5) & 0x3F) * 3  + 1) / 4) & 0x3F) << 5)
                                           | ((((((tmp0 >> 11) & 0x1F)  + ((rgb2 >> 11) & 0x1F) * 3  + 1) / 4) & 0x1F) << 11));
#else
                *(pDst + dst_pitch - 1) = tmp0; /* save right pixel */
#endif
                *(pDst + dst_pitch - 2) = tmp0; // i5 = p4; /* save right pixel */

                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb1	=	tmp2 | (rgb << 5);

#ifdef INTERPOLATE
                *pDst-- = (((((tmp01 & 0x1F)  + (rgb1 & 0x1F) + 1) / 2) & 0x1F)
                           | ((((((tmp01 >> 5) & 0x3F)  + ((rgb1 >> 5) & 0x3F)  + 1) / 2) & 0x3F) << 5)
                           | ((((((tmp01 >> 11) & 0x1F)  + ((rgb1 >> 11) & 0x1F)  + 1) / 2) & 0x1F) << 11));
#else
                *pDst--	=	rgb1;	/* save left pixel */
#endif

                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *pDst-- =	(((((tmp0 & 0x1F)  + (rgb1 & 0x1F) * 3 + 1) / 4) & 0x1F)
                           | ((((((tmp0 >> 5) & 0x3F)  + ((rgb1 >> 5) & 0x3F) * 3  + 1) / 4) & 0x3F) << 5)
                           | ((((((tmp0 >> 11) & 0x1F)  + ((rgb1 >> 11) & 0x1F) * 3  + 1) / 4) & 0x1F) << 11));
#else
                *pDst--	= tmp0;	/* save right pixel */
#endif
                *pDst--	= tmp0;	/* save right pixel */

            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            pDst++; // go back to the beginning of the line
            if (!(row&0x3))
            {
                oscl_memcpy(pDst + (dst_pitch << 1), pDst + dst_pitch, 2*disp[4]);
                pDst += dst_pitch;
#ifdef INTERPOLATE
                previous = pDst - dst_pitch;

                for (i = 0; i < disp[4]; i++)	// linear interpolation
                {
                    int32	last = previous[i];
                    int32 curr = pDst[i];
                    pDst[i] = (((((last      & 0x1F)  + (curr     & 0x1F)) / 2) & 0x1F)
                               | ((((((last >> 5) & 0x3F)  + ((curr >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                               | ((((((last >> 11) & 0x1F)  + ((curr >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
#endif
            }

            pDst +=	(deltaDst);	//coz pDst defined as UINT *
        }
    }
    else
    {
        if (disp[6]) /* rotate 180 and flip */
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + deltaY);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY;
            pCr = src[2] + deltaY;
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            src_pitch = -(src_pitch >> 1);
        }
        else // no rotation,no flip,only scale
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];

        }

        deltaDst	=	(dst_pitch << 1) - disp[4]; /* disp[4] is dst_width */
        pDst =	(uint16 *)dst;

        for (row = disp[3]; row > 0; row -= 2)
        {

            for (col = src_width - 1; col >= 0; col -= 4)  /* do 8 pixels at a time, 4 ups 2 downs */
            {

                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *(pDst + dst_pitch) = rgb;  /* save left pixel, have to save separately */

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp02	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *(pDst + dst_pitch + 1) = (((((tmp02 & 0x1F) * 3  + (rgb & 0x1F) + 2) / 4) & 0x1F)
                                           | ((((((tmp02 >> 5) & 0x3F) * 3  + ((rgb >> 5) & 0x3F)  + 2) / 4) & 0x3F) << 5)
                                           | ((((((tmp02 >> 11) & 0x1F) * 3  + ((rgb >> 11) & 0x1F)  + 2) / 4) & 0x1F) << 11));
                //i2 = (p1 + 3*p2 + 2)>>2;
#else
                *(pDst + dst_pitch + 1) = tmp02; /* save right pixel */
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *pDst++	=	rgb;	/* save left pixel */

                Y	>>=	8;

                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp01	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *(pDst++) = (((((tmp01 & 0x1F) * 3  + (rgb & 0x1F) + 2) / 4) & 0x1F)
                             | ((((((tmp01 >> 5) & 0x3F) * 3  + ((rgb >> 5) & 0x3F)  + 2) / 4) & 0x3F) << 5)
                             | ((((((tmp01 >> 11) & 0x1F) * 3  + ((rgb >> 11) & 0x1F)  + 2) / 4) & 0x1F) << 11));
                //i2 = (p1 + 3*p2 + 2)>>2;
#else
                *pDst++	= tmp01;	/* save right pixel */
#endif
                /* now do another 4 pixels add 1 pixels in the last column */
                Cb = *pCb++;
                Cr = *pCr++;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;
                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb2		=	tmp2 | (rgb << 5);
#ifdef INTERPOLATE
                // i3 = (p2 + p3 + 1)>>1;
                *(pDst + dst_pitch) = (((((tmp02 & 0x1F)  + (rgb2 & 0x1F) + 1) / 2) & 0x1F)
                                       | ((((((tmp02 >> 5) & 0x3F)  + ((rgb2 >> 5) & 0x3F)  + 1) / 2) & 0x3F) << 5)
                                       | ((((((tmp02 >> 11) & 0x1F)  + ((rgb2 >> 11) & 0x1F)  + 1) / 2) & 0x1F) << 11));
#else
                *(pDst + dst_pitch) = rgb2;  /* save left pixel, have to save separately */
#endif
                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *(pDst + dst_pitch + 1) = (((((tmp0 & 0x1F)  + (rgb2 & 0x1F) * 3 + 1) / 4) & 0x1F)
                                           | ((((((tmp0 >> 5) & 0x3F)  + ((rgb2 >> 5) & 0x3F) * 3  + 1) / 4) & 0x3F) << 5)
                                           | ((((((tmp0 >> 11) & 0x1F)  + ((rgb2 >> 11) & 0x1F) * 3  + 1) / 4) & 0x1F) << 11));
#else
                *(pDst + dst_pitch + 1) = tmp0; /* save right pixel */
#endif
                *(pDst + dst_pitch + 2) = tmp0; /* save right pixel */ //Ankur
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb1		=	tmp2 | (rgb << 5);
#ifdef INTERPOLATE
                *pDst++ = (((((tmp01 & 0x1F)  + (rgb1 & 0x1F) + 1) / 2) & 0x1F)
                           | ((((((tmp01 >> 5) & 0x3F)  + ((rgb1 >> 5) & 0x3F)  + 1) / 2) & 0x3F) << 5)
                           | ((((((tmp01 >> 11) & 0x1F)  + ((rgb1 >> 11) & 0x1F)  + 1) / 2) & 0x1F) << 11));
#else
                *pDst++	=	rgb1;	/* save left pixel */
#endif
                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                *pDst++ =	(((((tmp0 & 0x1F)  + (rgb1 & 0x1F) * 3 + 1) / 4) & 0x1F)
                           | ((((((tmp0 >> 5) & 0x3F)  + ((rgb1 >> 5) & 0x3F) * 3  + 1) / 4) & 0x3F) << 5)
                           | ((((((tmp0 >> 11) & 0x1F)  + ((rgb1 >> 11) & 0x1F) * 3  + 1) / 4) & 0x1F) << 11));
#else
                *pDst++	= tmp0;	/* save right pixel */
#endif
                *pDst++	= tmp0;	/* save right pixel */ // Ankur
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            pDst +=	(deltaDst);	//coz pDst defined as UINT *

            /************
            	interpolating 3rd row in between
            	row1
            	row2
            	inter1
            	row3
            	row4
            **************/

            if (!(row&0x3))
            {
#ifdef INTERPOLATE
                pFifth = pDst; // interpolating 3rd line in between
                copyline = 0;
#else
                oscl_memcpy(pDst, pDst - dst_pitch, 2*disp[4]);
#endif
                pDst += dst_pitch;
            }
#ifdef INTERPOLATE
            if (copyline)
            {
                previous = pFifth - dst_pitch;
                current = pDst - (dst_pitch * 2);

                for (i = 0; i < disp[4]; i++)	// linear interpolation
                {
                    int32	last = previous[i];
                    int32 curr = current[i];
                    pFifth[i] = (((((last      & 0x1F)  + (curr     & 0x1F)) / 2) & 0x1F)
                                 | ((((((last >> 5) & 0x3F)  + ((curr >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((last >> 11) & 0x1F)  + ((curr >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }

            }

            copyline++;
#endif
        }
    }
    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);

    return 0;
#endif // CCSCALING
}

/* special 4:3 scale-up color conversion */
int32 cc16scaling43(uint8 **src, uint8 *dst,
                    int32 *disp, uint8 *coff_tbl)
{
#if CCSCALING
    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32 col3, row3;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

#ifdef INTERPOLATE
    int i;
    uint32	copyline = 0;	//maru
    uint16	*prev_pDst = 0;	//maru
    int32	prev_offset = 0;	//maru
#endif

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    if (((disp[6] == 0 && disp[7] == 1)) || ((disp[6] == 1) && (disp[7] == 0)))  /* rotate 0 and flip // rotate 180, no flip*/
    {
        if (disp[6] == 0)
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }
        else
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

        pDst =	((uint16 *)dst) + disp[4] - 1;
        row3 = 2;
        for (row = disp[3]; row > 0; row -= 2)
        {
            col3 = 2;
            if (row3 == 1)
            {
                dst_pitch <<= 1;
            }

            for (col = src_width - 1; col >= 0; col -= 4)
            { /* do 8 pixels at a time, 2 ups 2 downs */
                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *(pDst += dst_pitch) = rgb;  /* save left pixel, have to save separately */

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                if (col3 == 0)
                {
#ifndef INTERPOLATE
                    //*(++pDst) = rgb; /* repeat this pixel */
                    *(--pDst) = rgb; /* repeat this pixel */
#else
                    *(--pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                }
#ifndef INTERPOLATE
                //*(pDst+1) = tmp0; /* save right pixel */Ankur
                *(pDst - 1) = tmp0; /* save right pixel */
                if (col3 == 1)
                {
                    //*(pDst+2) = tmp0; /* repeat this pixel */Ankur
                    *(pDst - 2) = tmp0; /* repeat this pixel */
                }
#else
                if (col3 != 1)
                {
                    *(pDst - 1) = tmp0;
                }
                else
                {
                    *(pDst - 2) = tmp0;
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
#endif

                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifndef INTERPOLATE
                *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                if (col3 == 0)
                {
                    pDst[1] = rgb; /* repeat this pixel */
                }
#else
                if (col3 != 0)
                {
                    *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                }
                else
                {
                    *(pDst -= dst_pitch) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                            | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                            | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    pDst[1] = rgb;
                }
#endif
#ifndef INTERPOLATE
                *(--pDst)	= tmp0;	/* save right pixel */
                if (col3 == 1)
                {
                    *(--pDst)	= tmp0;	/* save right pixel */
                }
#else
                if (col3 != 1)
                {
                    *(--pDst)	= tmp0;	/* save right pixel */
                }
                else
                {
                    *(--pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(--pDst) = tmp0;
                }
#endif

                pDst--;

                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *(pDst += dst_pitch) = rgb;  /* save left pixel, have to save separately */

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];

                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                if (col3 == 2)
                {
#ifndef INTERPOLATE
                    *(--pDst) = rgb;
#else
                    *(--pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                }

#ifndef INTERPOLATE
                *(pDst - 1) = tmp0; /* save right pixel */
                if (col3 == 0)
                {
                    *(pDst - 2) = tmp0; /* repeat this pixel */
                }
#else
                if (col3 != 0)
                {
                    *(pDst - 1) = tmp0; /* save right pixel */
                }
                else
                {
                    *(pDst - 2) = tmp0; /* save right pixel */
                    *(pDst - 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
#endif

                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);
#ifndef INTERPOLATE
                *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                if (col3 == 2)
                {
                    pDst[1] = rgb; /* repeat this pixel */
                }
#else
                if (col3 != 2)
                {
                    *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                }
                else
                {
                    *(pDst -= dst_pitch)	=	(((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                            | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                            | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    pDst[1] = rgb;
                }
#endif

#ifndef INTERPOLATE
                *(--pDst)	= tmp0;	/* save right pixel */
                if (col3 == 0)
                {
                    *(--pDst)	= tmp0;
                }
#else
                if (col3 != 0)
                {
                    *(--pDst)	= tmp0;	/* save right pixel */
                }
                else
                {
                    *(--pDst)	= (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(--pDst)	= tmp0;	/* save right pixel */
                }
#endif

                pDst--;

                col3--;
                if (col3 < 0)
                {
                    col3 = 2;
                }
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            if (row3 == 1)
            {
                dst_pitch >>= 1;
            }
            dst += (dst_pitch << 2);

            if (row3 == 1)
            {
#ifndef INTERPOLATE
                oscl_memcpy(dst - (dst_pitch << 1), dst - (dst_pitch << 2), 2*disp[4]);
#else
                prev_pDst = (uint16*)(dst - (dst_pitch << 2));
                for (i = 0; i < dst_pitch; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *((uint16*)dst + i);
                    coltemp = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                               | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                               | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                }
#endif
                dst += (dst_pitch << 1);
            }
            else if (row3 == 0)
            {
                oscl_memcpy(dst, dst - (dst_pitch << 1), 2*disp[4]);
#ifdef INTERPOLATE
                prev_pDst = (uint16*)(dst - (dst_pitch << 2));
                for (i = 0; i < dst_pitch; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *((uint16*)dst + i);
                    coltemp = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                               | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                               | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                }
#endif
                dst += (dst_pitch << 1);
            }
            row3--;

            if (row3 < 0)
            {
                row3 = 2;
            }

            pDst = ((uint16*)dst) + disp[4] - 1;
        }
    }
    else  /* rotate 180 and flip || no rotation,no flip */
    {
        if (disp[6]) /* rotate 180 and flip */
        {
            /* move the starting point to the bottom-left corner of the picture */
            deltaY = src_pitch * (disp[3] - 1);
            pY = (uint16*)(src[0] + deltaY);
            deltaY = (src_pitch >> 1) * ((disp[3] >> 1) - 1);
            pCb = src[1] + deltaY;
            pCr = src[2] + deltaY;
            deltaY = -src_width - (src_pitch << 1);
            deltaCbCr = -((src_width + src_pitch) >> 1);
            src_pitch = -(src_pitch >> 1);
        }
        else // no rotation,no flip
        {
            deltaY		=	(src_pitch << 1) - src_width;
            deltaCbCr	=	(src_pitch - src_width) >> 1;
            pY = (uint16 *) src[0];
            src_pitch >>= 1;
            pCb = src[1];
            pCr = src[2];
        }
        pDst =	(uint16 *)dst;
        row3 = 2;

        for (row = disp[3]; row > 0; row -= 2)
        {
            col3 = 2;

            if (row3 == 1)
            {
                dst_pitch <<= 1;
            }

            for (col = src_width - 1; col >= 0; col -= 4)
            { /* do 8 pixels at a time, 2 ups 2 downs */
                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];
                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                *(pDst += dst_pitch) = rgb;  /* save left pixel, have to save separately */

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

                if (col3 == 0)
                {
#ifndef INTERPOLATE
                    *(++pDst) = rgb; /* repeat this pixel */
#else
                    *(++pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                }

#ifndef INTERPOLATE
                *(pDst + 1) = tmp0; /* save right pixel */
                if (col3 == 1)
                {
                    *(pDst + 2) = tmp0; /* repeat this pixel */
                }
#else
                if (col3 != 1)
                {
                    *(pDst + 1) = tmp0;
                }
                else
                {
                    *(pDst + 2) = tmp0;
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);
                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);

#ifndef INTERPOLATE
                *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                if (col3 == 0)
                {
                    pDst[-1] = rgb; /* repeat this pixel */
                }
#else
                if (col3 != 0)
                {
                    *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                }
                else
                {
                    *(pDst -= dst_pitch) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                            | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                            | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    pDst[-1] = rgb;
                }
#endif

#ifndef INTERPOLATE
                *(++pDst)	= tmp0;	/* save right pixel */
                if (col3 == 1)
                {
                    *(++pDst)	= tmp0;	/* save right pixel */
                }
#else
                if (col3 != 1)
                {
                    *(++pDst)	= tmp0;	/* save right pixel */
                }
                else
                {
                    *(++pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(++pDst) = tmp0;
                }
#endif
                pDst++;

                Cb = *pCb++;
                Cr = *pCr++;
                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                //load the bottom two pixels
                Y = pY[src_pitch];

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_0;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                //RGB_565

                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                *(pDst += dst_pitch) = rgb;

                Y	>>=	8;
                Y	+= OFFSET_5_1;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];
                //RGB_565
                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);
                if (col3 == 2)
                {
#ifndef INTERPOLATE
                    *(++pDst) = rgb; /* repeat this pixel */
#else
                    *(++pDst) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
#endif
                }

#ifndef INTERPOLATE
                *(pDst + 1) = tmp0; /* save right pixel */
                if (col3 == 0)
                {
                    *(pDst + 2) = tmp0; /* repeat this pixel */
                }
#else
                if (col3 != 0)
                {
                    *(pDst + 1) = tmp0; /* save right pixel */
                }
                else
                {
                    *(pDst + 2) = tmp0; /* save right pixel */
                    *(pDst + 1) = (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                   | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                }
#endif
                //load the top two pixels
                Y = *pY++;

                tmp0	=	Y & 0xFF;	//Low endian	left pixel
                tmp0	+= OFFSET_5_1;

                tmp1	=	tmp0 - (Cg >> 16);
                tmp2	=	tmp0 + (Cb >> 16);
                tmp0	=	tmp0 + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                tmp2	=	clip[tmp2];


                rgb		=	tmp1 | (tmp0 << 6);
                rgb		=	tmp2 | (rgb << 5);

                Y	>>=	8;
                Y	+= OFFSET_5_0;
                tmp1	=	(Y) - (Cg >> 16);
                tmp2	=	(Y) + (Cb >> 16);
                tmp0	=	(Y) + (Cr >> 16);

                tmp0	=	clip[tmp0];
                tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                tmp2	=	clip[tmp2];

                tmp0	=	tmp1 | (tmp0 << 6);
                tmp0	=	tmp2 | (tmp0 << 5);
#ifndef INTERPOLATE
                *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                if (col3 == 2)
                {
                    pDst[-1] = rgb; /* repeat this pixel */
                }
#else
                if (col3 != 2)
                {
                    *(pDst -= dst_pitch)	=	rgb;	/* save left pixel */
                }
                else
                {
                    *(pDst -= dst_pitch)	=	(((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                            | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                            | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    pDst[-1] = rgb;
                }
#endif

#ifndef INTERPOLATE
                *(++pDst)	= tmp0;	/* save right pixel */
                if (col3 == 0)
                {
                    *(++pDst)	= tmp0;	/* save right pixel */
                }
#else
                if (col3 != 0)
                {
                    *(++pDst)	= tmp0;	/* save right pixel */
                }
                else
                {
                    *(++pDst)	= (((((tmp0      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                 | ((((((tmp0 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                 | ((((((tmp0 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(++pDst)	= tmp0;	/* save right pixel */
                }
#endif
                pDst++;

                col3--;
                if (col3 < 0)
                {
                    col3 = 2;
                }
            }//end of COL

            pY	+=	(deltaY >> 1);
            pCb	+=	deltaCbCr;
            pCr	+=	deltaCbCr;
            if (row3 == 1)
            {
                dst_pitch >>= 1;
            }
            dst += (dst_pitch << 2);
            if (row3 == 1)
            {
#ifndef INTERPOLATE
                oscl_memcpy(dst - (dst_pitch << 1), dst - (dst_pitch << 2), 2*disp[4]);
#else
                prev_pDst = (uint16*)(dst - (dst_pitch << 2));
                for (i = 0; i < dst_pitch; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *((uint16*)dst + i);
                    coltemp = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                               | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                               | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                }
#endif
                dst += (dst_pitch << 1);
            }
            else if (row3 == 0)
            {
                oscl_memcpy(dst, dst - (dst_pitch << 1), 2*disp[4]);

#ifdef INTERPOLATE
                prev_pDst = (uint16*)(dst - (dst_pitch << 2));
                for (i = 0; i < dst_pitch; i++)
                {
                    int32 coltemp;
                    int32 pretemp = *(prev_pDst + i);
                    int32 curtemp = *((uint16*)dst + i);
                    coltemp = (((((pretemp      & 0x1F)  + (curtemp     & 0x1F)) / 2) & 0x1F)
                               | ((((((pretemp >> 5) & 0x3F)  + ((curtemp >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                               | ((((((pretemp >> 11) & 0x1F)  + ((curtemp >> 11) & 0x1F)) / 2) & 0x1F) << 11));
                    *(prev_pDst + dst_pitch + i) = (uint16)coltemp;
                }
#endif
                dst += (dst_pitch << 1);
            }

            row3--;
            if (row3 < 0)
            {
                row3 = 2;
            }

            pDst = (uint16*)dst;
        }
    }

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);

    return 0;
#endif // CCSCALING
}


// platform specfic function in C
int32 cc16sc_rotate(uint8 **src, uint8 *dst, int32 *disp,
                    uint8 *coff_tbl, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise, int32 flip); // ankur

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 ColorConvert16::cc16ZoomRotate(uint8 **src, uint8 *dst,
                                     DisplayProperties *disp, uint8 *coff_tbl)
{
    int32 disp_prop[6];
    int32 flip; // ankur

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    flip = _mIsFlip;
    return cc16sc_rotate(src, dst, disp_prop, coff_tbl, _mRowPix, _mColPix, (_mRotation == CCROTATE_CLKWISE), flip);
}

int32 cc16sc_rotate(uint8 **src, uint8 *dst, int32 *disp,
                    uint8 *coff_tbl, uint8 *_mRowPix,
                    uint8 *_mColPix, bool _mIsRotateClkwise, int32 flip) // ankur
{
#if (CCROTATE && CCSCALING)
    /*	1. move the dst pointer to the line above the border
    2. do 2 line conversion
    3. copy both up & down
    	*/
    uint8 *pCb, *pCr;
    uint8	*pY;
    uint16	*pDst;
    int32		src_pitch, dst_pitch, src_width;
    int32		Y, Cb, Cr, Cg;
    int32		deltaY, dst_width, deltaCbCr, dst_inc, dst_start_pos;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;
    uint8 *clip = coff_tbl + 400;
    int32		offset;
    int32  cc1 = (*((int32*)(clip - 400)));
    int32  cc3 = (*((int32*)(clip - 396)));
    int32  cc2 = (*((int32*)(clip - 392)));
    int32  cc4 = (*((int32*)(clip - 388)));

    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    dst_width	=  disp[4];

#ifdef INTERPOLATE
    int32 prgb1;
    int32 prgb2;
    int32 prgb3;
    int32 prgb4;
#endif

    if (_mIsRotateClkwise)
    {
        deltaY		=  src_pitch * disp[3] + 2;
        deltaCbCr	= ((src_pitch * disp[3]) >> 2) + 1;
    }
    else // rotate counterclockwise
    {
        deltaY		=  -(src_pitch * disp[3] + 2);
        deltaCbCr	=  -(((src_pitch * disp[3]) >> 2) + 1);
    }

    // map origin of the destination to the source
    if (_mIsRotateClkwise)
    {
        pY = src[0] + src_pitch * (disp[3] - 1);
        pCb = src[1] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));
        pCr = src[2] + ((src_pitch >> 1) * ((disp[3] >> 1) - 1));
    }
    else // rotate counterclockwise
    {
        pY = src[0] + src_width - 1;
        pCb = src[1] + (src_width >> 1) - 1;
        pCr = src[2] + (src_width >> 1) - 1;
    }

    int32 half_src_pitch, read_idx, tmp_src_pitch;
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

    if (flip == 0)
    {
        dst_start_pos = 0;
        dst_inc = 1;
    }
    else
    {
        dst_start_pos = disp[4] - 1;
        dst_inc = -1;
    }

    pDst =	(uint16 *)dst + dst_start_pos;

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

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (_mRowPix[col]) /* compute this pixel */
                {
                    tmp0	=   pY[read_idx];		//bottom left

                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    Y	=	*pY;
                    pY += tmp_src_pitch; 	//upper left

                    Y	+= OFFSET_5_1;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	=	(uint16)tmp0;

                } /*	if(_mRowPix[col])  */
                else
                {
                    pY += tmp_src_pitch;
                }

                pDst	+= (dst_inc > 0 ? _mRowPix[col] : -_mRowPix[col]);

                if (_mRowPix[col+1]) /* compute this pixel */
                {
                    tmp0	=   pY[read_idx];			//bottom right

                    tmp0	+= OFFSET_5_1;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];


                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    Y	=	*pY;
                    pY += tmp_src_pitch;		//upper right

                    Y	+= OFFSET_5_0;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

                    *(pDst)	=	(uint16)tmp0;
                }/*	if(_mRowPix[col])  */
                else
                {
                    pY += tmp_src_pitch;
                }
                pDst	+= (dst_inc > 0 ? _mRowPix[col+1] : -_mRowPix[col+1]);
            }//end of COL
            //oscl_memcpy() both up & down

            pY	+=	(deltaY);
            pCb +=	deltaCbCr;
            pCr +=	deltaCbCr;

            pDst += dst_pitch;

            if (dst_inc > 0)
            {
                pDst -= dst_width; //goes back to the beginning of the line;
            }
            else
            {
                pDst += dst_width;
            }
        }
        else   // do two rows at least, scale up or down
        {

#ifdef INTERPOLATE
            prgb1 = 0;
            prgb2 = 0;
            prgb3 = 0;
            prgb4 = 0;
#endif
            for (col = disp[3] - 2; col >= 0; col -= 2)
            {/* decrement index, _mRowPix[.] is symmetric to increment index */
                Cb = *pCb;
                pCb += half_src_pitch;
                Cr = *pCr;
                pCr += half_src_pitch;

                Cb -= 128;
                Cr -= 128;
                Cg	=	Cr * cc1;
                Cr	*= cc3;

                Cg	+=	Cb * cc2;
                Cb	*=  cc4;

                if (_mRowPix[col]) /* compute this pixel */
                {
                    tmp0	=   pY[read_idx];		//bottom left

                    tmp0	+= OFFSET_5_0;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    //RGB_565

                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    Y	=	*pY;
                    pY += tmp_src_pitch; 	//upper left

                    Y	+= OFFSET_5_1;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];

                    //RGB_565
                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                    prgb3 = rgb;
                    prgb4 = tmp0;
#endif

                    if (_mRowPix[col] == 2)
                    {
#ifdef INTERPOLATE
                        *(pDst + dst_pitch)	=	(((((prgb1      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                               | ((((((prgb1 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                               | ((((((prgb1 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));

                        *(pDst + dst_pitch + dst_inc)	=	(uint16)rgb;

                        *(pDst)	=	(((((prgb2      & 0x1F)  + (tmp0     & 0x1F)) / 2) & 0x1F)
                                   | ((((((prgb2 >> 5) & 0x3F)  + ((tmp0 >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((prgb2 >> 11) & 0x1F)  + ((tmp0 >> 11) & 0x1F)) / 2) & 0x1F) << 11));

                        *(pDst + dst_inc)	= (uint16)tmp0;

#else
                        *(pDst + dst_pitch)			=	(uint16)rgb;
                        *(pDst + dst_pitch + dst_inc)	=	(uint16)rgb;
                        *(pDst)						= (uint16)tmp0;
                        *(pDst + dst_inc)				= (uint16)tmp0;
#endif
                    }
                    else if (_mRowPix[col] == 3)
                    {
#ifdef INTERPOLATE
                        *(pDst + dst_pitch)	=	(((((prgb1      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                               | ((((((prgb1 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                               | ((((((prgb1 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));

                        *((pDst + dst_pitch + dst_inc))	= (((((prgb1      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                                           | ((((((prgb1 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                                           | ((((((prgb1 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));

                        *((pDst + dst_pitch + (dst_inc << 1)))	= (uint16)rgb ;

                        *(pDst)	=	(((((prgb2      & 0x1F)  + (tmp0     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((prgb2 >> 5) & 0x3F)  + ((tmp0 >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((prgb2 >> 11) & 0x1F)  + ((tmp0 >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));

                        *((pDst + dst_inc))	=	(((((prgb2      & 0x1F) * 2 + (tmp0     & 0x1F)) / 3) & 0x1F)
                                               | ((((((prgb2 >> 5) & 0x3F) * 2 + ((tmp0 >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                               | ((((((prgb2 >> 11) & 0x1F) * 2 + ((tmp0 >> 11) & 0x1F)) / 3) & 0x1F) << 11));

                        *((pDst + (dst_inc << 1)))	= (uint16)tmp0;
#else
                        *(pDst + dst_pitch)	=	(uint16)rgb;
                        *((pDst + dst_pitch + dst_inc))	= (uint16)rgb ;
                        *((pDst + dst_pitch + (dst_inc << 1)))	= (uint16)rgb ;

                        *(pDst)	=	(uint16)tmp0;
                        *((pDst + dst_inc))	= (uint16)tmp0;
                        *((pDst + (dst_inc << 1)))	= (uint16)tmp0;
#endif
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	(uint16)rgb;
                        *(pDst)	=	(uint16)tmp0;
                    }

                } /*	if(_mRowPix[col])  */
                else
                {
                    pY += tmp_src_pitch;
                }

                pDst	+= (dst_inc > 0 ? _mRowPix[col] : -_mRowPix[col]);

                if (_mRowPix[col+1]) /* compute this pixel */
                {
                    tmp0	=   pY[read_idx];			//bottom right

                    tmp0	+= OFFSET_5_1;

                    tmp1	=	tmp0 - (Cg >> 16);
                    tmp2	=	tmp0 + (Cb >> 16);
                    tmp0	=	tmp0 + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_1 - OFFSET_5_1];
                    tmp2	=	clip[tmp2];


                    rgb		=	tmp1 | (tmp0 << 6);
                    rgb		=	tmp2 | (rgb << 5);

                    Y	=	*pY;
                    pY += tmp_src_pitch;		//upper right

                    Y	+= OFFSET_5_0;
                    tmp1	=	(Y) - (Cg >> 16);
                    tmp2	=	(Y) + (Cb >> 16);
                    tmp0	=	(Y) + (Cr >> 16);

                    tmp0	=	clip[tmp0];
                    tmp1	=	clip[tmp1 + OFFSET_6_0 - OFFSET_5_0];
                    tmp2	=	clip[tmp2];

                    tmp0	=	tmp1 | (tmp0 << 6);
                    tmp0	=	tmp2 | (tmp0 << 5);

#ifdef INTERPOLATE
                    prgb1 = rgb;
                    prgb2 = tmp0;
#endif
                    if (_mRowPix[col+1] == 2)
                    {
#ifdef INTERPOLATE
                        *(pDst + dst_pitch)	=	(((((prgb3      & 0x1F)  + (rgb     & 0x1F)) / 2) & 0x1F)
                                               | ((((((prgb3 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                               | ((((((prgb3 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F)) / 2) & 0x1F) << 11));

                        *(pDst + dst_pitch + dst_inc)	=	(uint16)rgb;

                        *(pDst)	=	(((((prgb4      & 0x1F)  + (tmp0     & 0x1F)) / 2) & 0x1F)
                                   | ((((((prgb4 >> 5) & 0x3F)  + ((tmp0 >> 5) & 0x3F)) / 2) & 0x3F) << 5)
                                   | ((((((prgb4 >> 11) & 0x1F)  + ((tmp0 >> 11) & 0x1F)) / 2) & 0x1F) << 11));

                        *(pDst + dst_inc)	= (uint16)tmp0;

#else
                        *(pDst + dst_pitch)			=	(uint16)rgb;
                        *(pDst + dst_pitch + dst_inc)	=	(uint16)rgb;
                        *(pDst)						= (uint16)tmp0;
                        *(pDst + dst_inc)				= (uint16)tmp0;
#endif
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
#ifdef INTERPOLATE
                        *(pDst + dst_pitch)	=	(((((prgb3      & 0x1F)  + (rgb     & 0x1F) * 2) / 3) & 0x1F)
                                               | ((((((prgb3 >> 5) & 0x3F)  + ((rgb >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                               | ((((((prgb3 >> 11) & 0x1F)  + ((rgb >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));

                        *((pDst + dst_pitch + dst_inc))	= (((((prgb3      & 0x1F) * 2 + (rgb     & 0x1F)) / 3) & 0x1F)
                                                           | ((((((prgb3 >> 5) & 0x3F) * 2 + ((rgb >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                                           | ((((((prgb3 >> 11) & 0x1F) * 2 + ((rgb >> 11) & 0x1F)) / 3) & 0x1F) << 11));

                        *((pDst + dst_pitch + (dst_inc << 1)))	= (uint16)rgb ;

                        *(pDst)	=	(((((prgb4      & 0x1F)  + (tmp0     & 0x1F) * 2) / 3) & 0x1F)
                                   | ((((((prgb4 >> 5) & 0x3F)  + ((tmp0 >> 5) & 0x3F) * 2) / 3) & 0x3F) << 5)
                                   | ((((((prgb4 >> 11) & 0x1F)  + ((tmp0 >> 11) & 0x1F) * 2) / 3) & 0x1F) << 11));

                        *((pDst + dst_inc))	=	(((((prgb4      & 0x1F) * 2 + (tmp0     & 0x1F)) / 3) & 0x1F)
                                               | ((((((prgb4 >> 5) & 0x3F) * 2 + ((tmp0 >> 5) & 0x3F)) / 3) & 0x3F) << 5)
                                               | ((((((prgb4 >> 11) & 0x1F) * 2 + ((tmp0 >> 11) & 0x1F)) / 3) & 0x1F) << 11));

                        *((pDst + (dst_inc << 1)))	= (uint16)tmp0;
#else
                        *(pDst + dst_pitch)	=	(uint16)rgb;
                        *((pDst + dst_pitch + dst_inc))	= (uint16)rgb ;
                        *((pDst + dst_pitch + (dst_inc << 1)))	= (uint16)rgb ;

                        *(pDst)	=	(uint16)tmp0;
                        *((pDst + dst_inc))	= (uint16)tmp0;
                        *((pDst + (dst_inc << 1)))	= (uint16)tmp0;
#endif
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	(uint16)rgb;
                        *(pDst)	=	(uint16)tmp0;
                    }
                }/*	if(_mRowPix[col])  */
                else
                {
                    pY += tmp_src_pitch;
                }
                pDst	+= (dst_inc > 0 ? _mRowPix[col+1] : -_mRowPix[col+1]);
            }//end of COL
            //oscl_memcpy() both up & down

            pY	+=	(deltaY);
            pCb +=	deltaCbCr;
            pCr +=	deltaCbCr;

            if (dst_inc > 0)
            {
                pDst -=	(dst_width); //goes back to the beginning of the line;
            }
            else
            {
                pDst += 1;
            }

            //copy down
            offset = (_mColPix[row] * dst_pitch);

            if (_mColPix[row-1] && _mColPix[row] != 1)
            {
                oscl_memcpy(pDst + offset, pDst + dst_pitch, dst_width*2);
            }
            if (_mColPix[row-1] == 2)
            {
                oscl_memcpy(pDst + offset + dst_pitch, pDst + dst_pitch, dst_width*2);
            }
            else if (_mColPix[row-1] == 3)
            {
                oscl_memcpy(pDst + offset + dst_pitch, pDst + dst_pitch, dst_width*2);
                oscl_memcpy(pDst + offset + dst_pitch*2, pDst + dst_pitch, dst_width*2);
            }

            if (_mColPix[row] > 1)
            {
                oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
                if (_mColPix[row] > 2)
                {
                    oscl_memcpy(pDst + dst_pitch*2, pDst, dst_width*2);
                }
            }

            pDst +=	dst_pitch * (_mColPix[row-1] + _mColPix[row]);
            if (dst_inc < 0)
            {
                pDst += (dst_width - 1);
            }
        }
    } // row

    return 1;
#else
    OSCL_UNUSED_ARG(src);
    OSCL_UNUSED_ARG(dst);
    OSCL_UNUSED_ARG(disp);
    OSCL_UNUSED_ARG(coff_tbl);
    OSCL_UNUSED_ARG(_mRowPix);
    OSCL_UNUSED_ARG(_mColPix);
    OSCL_UNUSED_ARG(_mIsRotateClkwise);
    OSCL_UNUSED_ARG(flip);

    return 0;
#endif // defined(CCROTATE) && defined(CCSCALING)
}


