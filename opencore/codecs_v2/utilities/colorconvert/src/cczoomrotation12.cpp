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
//  File: cczoomrotation12.cpp                                                //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
/** Class ColorConvert12, YUV to RGB12 bit with dithering. 4bit empty */
#include "colorconv_config.h"
#include "cczoomrotation12.h"

//for debug only
//#define	OUTPUT_RGB_565


OSCL_EXPORT_REF ColorConvertBase* ColorConvert12::NewL(void)
{
    ColorConvert12* self = OSCL_NEW(ColorConvert12, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


ColorConvert12::ColorConvert12()
{
    mErr_horz[0] = (NULL);
    mClip = NULL;
}


OSCL_EXPORT_REF ColorConvert12::~ColorConvert12()
{
    if (mErr_horz[0])
    {
        OSCL_ARRAY_DELETE(mErr_horz[0]);
    }

    if (mClip)
    {
        mClip -= 384;
        OSCL_ARRAY_DELETE(mClip);
    }
}


int32 ColorConvert12::Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation)
{
    if (ColorConvertBase::Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, nRotation) == 0)
    {
        return 0;
    }

    //local init
    _mInitialized = false;

    if (mErr_horz[0])
    {
        OSCL_ARRAY_DELETE(mErr_horz[0]);
        mErr_horz[0] = NULL;
    }

    int32 leavecode = 0;
    if (_mRotation&0x1)
    {
        OSCL_TRY(leavecode, mErr_horz[0] = OSCL_ARRAY_NEW(uint8, _mSrc_height * 4));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             return 0;
                            );

#if USE_ARM_ASM_FOR_COLORCONVERT
        oscl_memset((void*)(mErr_horz[0]), 0, _mSrc_height*4);
#else
        mErr_horz[1] = mErr_horz[0] + _mSrc_height + 1;
        mErr_horz[2] = mErr_horz[1] + _mSrc_height + 1;
#endif
    }
    else
    {
        OSCL_TRY(leavecode, mErr_horz[0] = OSCL_ARRAY_NEW(uint8, _mSrc_width * 4));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             return 0;
                            );

#if USE_ARM_ASM_FOR_COLORCONVERT
        oscl_memset((void*)(mErr_horz[0]), 0, _mSrc_width*4);
#else
        mErr_horz[1] = mErr_horz[0] + _mSrc_width + 1;
        mErr_horz[2] = mErr_horz[1] + _mSrc_width + 1;
#endif
    }

    if (mClip)
    {
        mClip -= 384;
        OSCL_ARRAY_DELETE(mClip);
        mClip = NULL;
    }
    leavecode = 0;
    OSCL_TRY(leavecode, mClip = OSCL_ARRAY_NEW(uint8, 1024));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         return 0;
                        );
    mClip += 384;

    int32 tmp;
    for (int32 i = -384; i < 640; i++)
    {
        tmp	= (int32)(1.164 * (i - 16));
        mClip[i] = (tmp < 0) ? 0 : ((tmp > 255) ? 255 : (uint8)tmp);
    }

    //default no zoom
    _mInitialized = true;

    SetMode(0); // called after init

    return 1;
}

int32  ColorConvert12::SetYuvFullRange(bool range)
{
    OSCL_ASSERT(_mInitialized == true);

    if (range == true)
        return 0;  // error this mode is not supported yet

    return 1;
}


int32 ColorConvert12::SetMode(int32 nMode)	//nMode : 0 Off, 1 On
{
    OSCL_ASSERT(_mInitialized == true);

    if (nMode == 0)
    {
        mPtrYUV2RGB	=	&ColorConvert12::get_frame12;
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
                mPtrYUV2RGB = &ColorConvert12::cc12ZoomRotate;
            }
            else /* zoom only */
            {
                mPtrYUV2RGB	=	&ColorConvert12::cc12ZoomIn;
            }
        }
        else
        {
            if (_mRotation&0x1) /* rotate only*/
            {
                mPtrYUV2RGB = &ColorConvert12::cc12Rotate;
            }
            else /* no zoom, no rotate, SetMode(1) = SetMode(0) */
            {
                mPtrYUV2RGB	=	&ColorConvert12::get_frame12;
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


int32 ColorConvert12::GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    if (_mIsZoom)
    {
        // for zoom, need extra line of RGB buffer for processing otherwise memory will corrupt.
        return	_mState ? ((_mDst_height + 1)*_mDst_pitch*2) : (_mSrc_width*_mSrc_height*2);
    }
    else
    {
        return	_mState ? ((_mDst_height)*_mDst_pitch*2) : (_mSrc_width*_mSrc_height*2);
    }

}


int32 ColorConvert12::Convert(uint8 **yuvBuf, uint8 *rgbBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(yuvBuf[0]);
    OSCL_ASSERT(yuvBuf[1]);
    OSCL_ASSERT(yuvBuf[2]);
    OSCL_ASSERT(rgbBuf);

    //reset the row dither error buffer
    if (((uint)rgbBuf)&0x3 || ((uint)yuvBuf[0])&0x3) /* address is not word align */
    {
        return 0;
    }

    if (_mRotation&0x1)
    {
        oscl_memset((void*)(mErr_horz[0]), 0, _mSrc_height*4);
    }
    else
    {
        oscl_memset((void*)(mErr_horz[0]), 0, _mSrc_width*4);
    }

    (*this.*mPtrYUV2RGB)(yuvBuf, rgbBuf, &_mDisp, mClip, (uint8 *)(mErr_horz[0]));

    return 1;
}


int32 ColorConvert12::Convert(uint8 *yuvBuf, uint8 *rgbBuf)
{
    OSCL_ASSERT(_mInitialized == true);
    OSCL_ASSERT(yuvBuf);
    OSCL_ASSERT(rgbBuf);

    //this conversion will cause problems when do src clipping. However, if they want, they must give more info
    uint8 *TmpYuvBuf[3];

    if (((uint)rgbBuf)&0x3 || ((uint)yuvBuf)&0x3) /* address is not word align */
    {
        return 0;
    }

    TmpYuvBuf[0]	=	yuvBuf;
    TmpYuvBuf[1]	=	yuvBuf + (_mSrc_pitch) * (_mSrc_mheight);
    TmpYuvBuf[2]	=	TmpYuvBuf[1] + (_mSrc_pitch * _mSrc_mheight) / 4;

    (*this.*mPtrYUV2RGB)(TmpYuvBuf, rgbBuf, &_mDisp, mClip, (uint8 *)(mErr_horz[0]));

    return 1;
}


#if USE_ARM_ASM_FOR_COLORCONVERT
int32 ColorConvert12::get_frame12(uint8 **src, uint8 *dst,
                                  DisplayProperties *disp, uint8 *ClipTable, uint8 *pExtraParam)
{
    {
        asm("stmfd		sp!, {r0, r4-r11, r14}");
        asm("mov		r0, r1");	//src
        asm("mov		r1, r2");	//dst
        asm("mov		r2, r3");	//disp
        asm("ldr		r3, [sp, #40]");	//COFF_TBL

        asm("ldmfd		r2, {r4-r9};");

        asm("rsb		r8, r6, r4, lsl #1;");	//r8 = src_pitch*2-src_width
        asm("sub		r9, r4, r6;");			//r9 = src_pitch - src_width
        asm("mov		r9, r9, lsr #1;");		//r9 = (src_pitch - src_width)/2
        asm("rsb		r10,r6, r5, lsl #1;");	//r10= (dst_pitch - src_width)
        asm("mov		r10, r10, lsl #1;");	//r10= (dst_pitch - src_width)*2
        asm("sub		r6, r6, #1;");			//r6 = src_width-1
        asm("orr		r6, r4, r6, lsl #18;");	//r6 = src_pitch | ( (src_width-1)<<18)
        //dst_pitch, r6, src_height, src_pitch*2-src_width, (src_pitch - src_width)/2, (dst_pitch - src_width)*2
        asm("stmfd		sp!, {r5-r10};");
        asm("ldmfd		r0, {r4, r5, r7};");	//pY, pCb, pCr
        asm("stmfd		sp!, { r5, r7 };");		//push pCb, pCr

        asm("LOOP_ROW:");
        asm("ldr		r9, [sp, #76]");		//load r9 = ErrRow

        asm("mov		r8, #0");				//;	reset error from left pixel
        asm("LOOP_COL:");
        asm("ldmfd		sp!, { r5, r7 }");		//load pCb, pCr
        asm("ldrb		r0, [r5], #1");			//Cb
        asm("ldrb		r2, [r7], #1");			//Cr
        asm("stmfd		sp!, { r5, r7 }");
        asm("mov		r10, r6, lsl #16");		//r10 = src_pitch <<16;
        asm("ldrb		r10, [r4, r10, lsr #16]");//r10 = Y (bottom left)

        asm("sub		r0, r0, #128;");		//r0 = Cb-128
        asm("sub		r2, r2, #128;");		//r2 = Cr-128

        asm("ldr		r7,   =45774");
        asm("ldr		r11,  =22014");
        asm("mul		r7, r2, r7");			//r7	=	(Cr-128)*JCoeff[0];
        asm("mla		r7, r0, r11, r7");		//r7	=	(Cb-128)*22014 + (Cr-128)*JCoeff[0];
        asm("ldr		r11, =89859");
        asm("mul		r2, r11, r2");			//r2	=	(Cr-128)*89859
        asm("ldr		r11, =113618");
        asm("mul		r0, r11, r0");			//r0	=	(Cb-128)*113618

        asm("add		r11, r2, r10, lsl #16");	// (Cr-128)*89859 + (Y<<16)
        asm("rsb		r12, r7, r10, lsl #16");	// (Y<<16) - ((Cb-128)*22014 + (Cr-128)*JCoeff[0])
        asm("add		r14, r0, r10, lsl #16");	// (Cb-128)*113618 + (Y<<16)

        asm("ldr		r10, [r9, r6, lsr #16]");	//;load error from upper row/pixel
        asm("add		r8, r8, r10, lsr #1");		//get the bottom error, only the LSB 16 bits is meaningful

        asm("and		r10, r8, #0xF");			//error B
        asm("add		r14, r14, r10");
        asm("and		r10, r8, #0xF0");			//error G
        asm("add		r12, r12, r10, lsr #4");
        asm("and		r10, r8, #0xF00");			//error R

        asm("add		r11, r11, r10, lsr #8");

        asm("ldrb		r11, [r3, r11, asr #16]");
        asm("ldrb		r12, [r3, r12, asr #16]");
        asm("ldrb		r14, [r3, r14, asr #16]");

#ifdef	OUTPUT_RGB_565
        // get RGB_565
        asm("mov		r10, r11, lsr #3");
        asm("mov		r10, r10, lsl #6");
        asm("orr		r10, r10, r12, lsr #2");
        asm("mov		r10, r10, lsl #5");
        asm("orr		r10, r10, r14, lsr #3");
#else
        //RGB_444
        asm("and		r10, r12, #0xF0");			//G
        asm("orr		r10, r10, r14, lsr #4");	//B
        asm("and		r11, r11, #0xF0");
        asm("orr		r10, r10, r11, lsl #4");	//R
#endif

        asm("and		r11, r11, #0xE");
        asm("and		r12, r12, #0xE");
        asm("and		r14, r14, #0xE");

        asm("orr		r11, r11, r12, lsl #4");

        asm("ldr		r12, [sp, #8]");		//should be det_pitch
        asm("add		r1, r1, r12, lsl #1");	//pDst move to bottom left pixel

        asm("strh		r10, [r1]");			//	store bottom left pixel
        asm("sub		r1, r1, r12, lsl #1");

        //////**************************/////////////////////
        asm("orr		r10, r11, r14, lsl #8");	//RGB dither error

        asm("mov		r8, r8, lsr #16");
        asm("mov		r8, r8, lsl #16");
        asm("orr		r8, r8, r10, lsr #1");

        asm("ldrb		r14, [r4], #1");		// p14 = Y (top left)

        asm("add		r11, r2, r14, lsl #16");
        asm("rsb		r12, r7, r14, lsl #16");
        asm("add		r14, r0, r14, lsl #16");

        asm("add		r8, r8, r10, lsl #15");	//get the top error, only the HSB 16 bits is meaningful

        asm("and		r10, r8,	#0xF0000");		//error B
        asm("add		r14, r14, r10, lsr #16");
        asm("and		r10, r8, #0xF00000");		//error G
        asm("add		r12, r12, r10, lsr #20");
        asm("and		r10, r8, #0xF000000");		//error R
        asm("add		r11, r11, r10, lsr #24");

        //clip

        asm("ldrb		r11, [r3, r11, asr #16]");
        asm("ldrb		r12, [r3, r12, asr #16]");
        asm("ldrb		r14, [r3, r14, asr #16]");

#ifdef	OUTPUT_RGB_565
        // get RGB_565
        asm("mov		r10, r11, lsr #3");
        asm("mov		r10, r10, lsl #6");
        asm("orr		r10, r10, r12, lsr #2");
        asm("mov		r10, r10, lsl #5");
        asm("orr		r10, r10, r14, lsr #3");
#else
        //RGB_444
        asm("and		r10, r12, #0xF0");			//G
        asm("orr		r10, r10, r14, lsr #4");	//B
        asm("and		r11, r11, #0xF0");
        asm("orr		r10, r10, r11, lsl #4");	//R
#endif


        asm("strh		r10, [r1], #2");	//strore top left RGB

        asm("and		r11, r11, #0xE");
        asm("and		r12, r12, #0xE");
        asm("and		r14, r14, #0xE");

        asm("orr		r11, r11, r12, lsl #4");
        asm("orr		r11, r11, r14, lsl #8");

        asm("mov		r8, r8, lsl #16");
        asm("mov		r8, r8, lsr #16");
        asm("orr		r8, r8, r11, lsl #15");

        asm("str		r11, [r9, r6, lsr #16]");

        asm("sub		r6, r6, #0x40000");		//col

        /********************************LEFT END, RIGHT BEGIN****************/

        asm("mov		r10, r6, lsl #16");
        asm("ldrb		r10, [r4, r10, lsr #16]");

        asm("add		r11, r2, r10, lsl #16");
        asm("rsb		r12, r7, r10, lsl #16");
        asm("add		r14, r0, r10, lsl #16");

        asm("ldr		r10, [r9, r6, lsr #16]");

        asm("add		r8, r8, r10, lsr #1");

        asm("and		r10, r8,	#0xF");
        asm("add		r14, r14, r10");
        asm("and		r10, r8, #0xF0");
        asm("add		r12, r12, r10, lsr #4");
        asm("and		r10, r8, #0xF00");
        asm("add		r11, r11, r10, lsr #8");

        asm("ldrb		r11, [r3, r11, asr #16]");
        asm("ldrb		r12, [r3, r12, asr #16]");
        asm("ldrb		r14, [r3, r14, asr #16]");

#ifdef	OUTPUT_RGB_565
        // get RGB_565
        asm("mov		r10, r11, lsr #3");
        asm("mov		r10, r10, lsl #6");
        asm("orr		r10, r10, r12, lsr #2");
        asm("mov		r10, r10, lsl #5");
        asm("orr		r10, r10, r14, lsr #3");
#else
        //RGB_444
        asm("and		r10, r12, #0xF0");			//G
        asm("orr		r10, r10, r14, lsr #4");	//B
        asm("and		r11, r11, #0xF0");
        asm("orr		r10, r10, r11, lsl #4");	//R
#endif

        asm("and		r11, r11, #0xE");
        asm("and		r12, r12, #0xE");
        asm("and		r14, r14, #0xE");

        asm("orr		r11, r11, r12, lsl #4");

        asm("ldr		r12, [sp, #8]");

        asm("add		r1, r1, r12, lsl #1");

        asm("strh		r10, [r1]");			//store RGB bottom right
        asm("sub		r1, r1, r12, lsl #1");

        asm("orr		r10, r11, r14, lsl #8");

        asm("mov		r8, r8, lsr #16");
        asm("mov		r8, r8, lsl #16");
        asm("orr		r8, r8, r10, lsr #1");

        asm("ldrb		r14, [r4], #1");		//r14 = Y (top right pixel)

        asm("add		r11, r2, r14, lsl #16");
        asm("rsb		r12, r7, r14, lsl #16");
        asm("add		r14, r0, r14, lsl #16");

        asm("add		r8, r8, r10, lsl #15");

        asm("and		r10, r8,	#0xF0000");
        asm("add		r14, r14, r10, lsr #16");
        asm("and		r10, r8, #0xF00000");
        asm("add		r12, r12, r10, lsr #20");
        asm("and		r10, r8, #0xF000000");
        asm("add		r11, r11, r10, lsr #24");

        asm("ldrb		r11, [r3, r11, asr #16]");
        asm("ldrb		r12, [r3, r12, asr #16]");
        asm("ldrb		r14, [r3, r14, asr #16]");
#ifdef	OUTPUT_RGB_565
        // get RGB_565
        asm("mov		r10, r11, lsr #3");
        asm("mov		r10, r10, lsl #6");
        asm("orr		r10, r10, r12, lsr #2");
        asm("mov		r10, r10, lsl #5");
        asm("orr		r10, r10, r14, lsr #3");
#else
        //RGB_444
        asm("and		r10, r12, #0xF0");			//G
        asm("orr		r10, r10, r14, lsr #4");	//B
        asm("and		r11, r11, #0xF0");
        asm("orr		r10, r10, r11, lsl #4");	//R
#endif

        asm("strh		r10, [r1], #2");		//store RGB (top right pixel)

        asm("and		r11, r11, #0xE");
        asm("and		r12, r12, #0xE");
        asm("and		r14, r14, #0xE");

        asm("orr		r11, r11, r12, lsl #4");
        asm("orr		r11, r11, r14, lsl #8");

        asm("mov		r8, r8, lsl #16");
        asm("mov		r8, r8, lsr #16");
        asm("orr		r8, r8, r11, lsl #15");

        asm("str		r11, [r9, r6, lsr #16]	; ");//Att: error left shift by 1 bit

        asm("subs		r6, r6, #0x40000");

        asm("bgt		LOOP_COL");

        asm("add		sp, sp, #8");
        asm("ldmfd		sp, {r5-r10}");
        asm("add		r4, r4, r8");
        asm("add		r5, r5, r9");
        asm("add		r7, r7, r9");
        asm("add		r1, r1, r10");
        asm("subs		r7, r7, #2;");
        asm("str		r7, [sp, #8]");

        asm("sub		sp, sp, #8");
        asm("bgt		LOOP_ROW");

        asm("mov		r0, #1");
        asm("add		sp, sp, #32");
        asm("ldmfd		sp!, {r0, r4-r11, pc}");
//		asm(".ltorg");
    }
    return 1;
}

#else // straight C version
/////////////////////////////////////////////////////////////////////////////
int32 ColorConvert12::get_frame12(uint8 **src, uint8 *dst,
                                  DisplayProperties *disp, uint8 *clip, uint8 *pErr_horz)
{
    const static uint32	JCoeff[4] =
    {
        45774,	//65536*0.813/1.164;
        89859,	//65536*1.596/1.164;
        22014,	//65536*0.391/1.164;
        113618	//65536*2.018/1.164;
    };

    uint8 *pCb, *pCr;
    uint16	*pY;
    uint16	*pDst;
    int32		src_width, dst_width, display_width;
    int32		Y, Cb, Cr, Cg;
    uint32	left_err_t, left_err_b; /* error from the left pixels */
    uint32	top_err_l, top_err_r; /* error from the top pixels */
    int32		deltaY, deltaDst, deltaCbCr;
    int32		row, col;
    int32		tmp0, tmp1, tmp2;
    uint32	rgb;


    src_width	=	disp->src_pitch;
    dst_width	=	disp->dst_pitch;
    display_width	=	disp->src_width;

    deltaY		=	(src_width << 1) - display_width;
    deltaCbCr	=	(src_width - display_width) >> 1;
    deltaDst	=	(dst_width << 1) - display_width;

    pY = (uint16 *) src[0];
    src_width >>= 1;
    pCb = src[1];
    pCr = src[2];

    pDst =	(uint16 *)dst;

    for (row = disp->src_height; row > 0; row -= 2)
    {

        left_err_t = left_err_b = 0;  /* reset left pixel errors */

        for (col = display_width - 2; col >= 0; col -= 2)
        {

            Cb = *pCb++;
            Cr = *pCr++;

            //load the bottom two pixels
            Y = pY[src_width];

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * JCoeff[0];
            Cg	+=	Cb * JCoeff[2];

            Cr	*=	JCoeff[1];
            Cb	*=	JCoeff[3];

            tmp0	=	Y & 0xFF;	//Low endian	left pixel
            //tmp0	=	pY[src_width];

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            //add the error from top and left
            //This is not exact error diffusion since
            //we process the bottom two pixels first !!
            tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
            tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
            tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

            top_err_l = *((uint*)(pErr_horz + (col << 2)));
            tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
            tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
            tmp2	+=	(((top_err_l >> 16) & 0xFF) << 16);//mErr_horz[2][col];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		= (tmp0 >> 3);
            rgb		= (tmp1 >> 2) | (rgb << 6);
            rgb		= (tmp2 >> 3) | (rgb << 5);
#else
            //RGB_444
            rgb		= (tmp0 & 0xF0) << 4;
            rgb		|= (tmp1 & 0xF0);
            rgb		|= (tmp2 >> 4);
#endif
            tmp0	&= 0xE;
            tmp1	&= 0xE;
            tmp2	&= 0xE;
            tmp0	>>= 1;
            tmp1	>>= 1;
            tmp2	>>= 1;

            //save error
            top_err_l = tmp0 | (tmp1 << 8) | (tmp2 << 16);

            Y	>>=	8;
            //Y	=	pY[src_width+1];

            // error from the left already available
            tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
            tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
            tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

            //add the error from top
            top_err_r = *((uint*)(pErr_horz + (col << 2) + 4));
            tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
            tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
            tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		|= ((tmp0 >> 3) << 27);
            rgb		|= ((tmp1 >> 2) << 21);
            rgb		|= ((tmp2 >> 3) << 16);
#else
            //RGB_444
            rgb		|= (tmp0 & 0xF0) << 20;
            rgb		|= (tmp1 & 0xF0) << 16;
            rgb		|= (tmp2 >> 4) << 16;
#endif
            *((uint*)(pDst + dst_width))	= rgb;

            tmp0	&= 0xE;
            tmp1	&= 0xE;
            tmp2	&= 0xE;
            tmp0	>>= 1;
            tmp1	>>= 1;
            tmp2	>>= 1;

            //save error
            left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16);
            top_err_r = left_err_b;

            //load the top two pixels
            //Y	=	*((uint16 *)pY)++;
            Y = *pY++;

            tmp0	=	Y & 0xFF;	//Low endian	left pixel
            //tmp0	=	*pY++;
            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            //add the error from top and left
            tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
            tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
            tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

            tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
            tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
            tmp2	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[2][col];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		= (tmp0 >> 3);
            rgb		= (tmp1 >> 2) | (rgb << 6);
            rgb		= (tmp2 >> 3) | (rgb << 5);
#else
            //RGB_444
            rgb		= (tmp0 & 0xF0) << 4;
            rgb		|= (tmp1 & 0xF0);
            rgb		|= (tmp2 >> 4);
#endif
            // calculate error
            tmp0	&= 0xE;
            tmp1	&= 0xE;
            tmp2	&= 0xE;
            tmp0 >>= 1;
            tmp1 >>= 1;
            tmp2 >>= 1;

            top_err_l = tmp0 | (tmp1 << 8) | (tmp2 << 16);
            *((uint*)(pErr_horz + (col << 2))) = top_err_l;

            Y	>>=	8;
            //Y	=	*pY++;

            //error from the left is already available
            tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
            tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
            tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

            //add the error from top
            tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
            tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
            tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		|= ((tmp0 >> 3) << 27);
            rgb		|= ((tmp1 >> 2) << 21);
            rgb		|= ((tmp2 >> 3) << 16);
#else
            //RGB_444
            rgb		|= (tmp0 & 0xF0) << 20;
            rgb		|= (tmp1 & 0xF0) << 16;
            rgb		|= (tmp2 >> 4) << 16;
#endif

            //			*( (uint *)pDst)++	= rgb;
            *((uint *)pDst)	= rgb;
            pDst += 2;

            tmp0	&= 0xE;
            tmp1	&= 0xE;
            tmp2	&= 0xE;
            tmp0 >>= 1;
            tmp1 >>= 1;
            tmp2 >>= 1;

            // save error
            left_err_t = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_r
            *((uint*)(pErr_horz + (col << 2) + 4)) = left_err_t;

        }//end of COL

        pY	+=	(deltaY >> 1);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
}
#endif // USE_ARM_ASM_FOR_COLORCONVERT


#if USE_ARM_ASM_FOR_COLORCONVERT
int32 ColorConvert12::cc12Rotate(uint8 **src, uint8 *dst,
                                 DisplayProperties *disp, uint8 *clip, uint8 *pExtraParam)
{
#if CCROTATE
    {
        asm("STMFD    sp!,{r0-r11,lr}");
        asm("SUB      sp,sp,#0x3c");
        asm("LDR      r3,[sp,#0x48]");
        asm("LDR      r10,[sp,#0x74]");
        asm("LDR      lr,[sp,#0x70]");
        asm("LDMIA    r3,{r9,r11}");
        asm("MOV      r12,r1");
        asm("STR      r11,[sp,#0x30]");
        asm("LDR      r3,[r3,#8]");
        asm("LDR      r4,[sp,#0x48]");
        asm("MOV      r5,#1");
        asm("LDR      r1,[r4,#0x10]");
        asm("RSB      r1,r1,r11,LSL #1");
        asm("STR      r1,[sp,#0x24]");
        asm("LDR      r7,[r0,#0x50]");
        asm("LDMIA    r12,{r0,r1,r4}");
        asm("LDR      r12,[sp,#0x48]");
        asm("CMP      r7,#0");
        asm("LDR      r8,[r12,#0]");
        asm("LDR      r12,[r12,#0xc]");
        asm("MUL      r6,r8,r12");
        asm("ADD      r5,r5,r6,ASR #2");
        asm("ADD      r6,r6,#2");
        asm("STRNE    r6,[sp,#0x28]");
        asm("STRNE    r5,[sp,#0x20]");
        asm("MOV      r8,r8,ASR #1");
        asm("BNE      L1.1716");
        asm("RSB      r6,r6,#0");
        asm("RSB      r5,r5,#0");
        asm("STR      r5,[sp,#0x20]");
        asm("STR      r6,[sp,#0x28]");
        asm("L1.1716:");
        asm("BEQ      L1.1748");
        asm("SUB      r5,r12,#1");
        asm("MLA      r0,r5,r9,r0");
        asm("MVN      r5,#0");
        asm("ADD      r12,r5,r12,ASR #1");
        asm("MLA      r1,r12,r8,r1");
        asm("MLA      r12,r8,r12,r4");
        asm("B        L1.1772");
        asm("L1.1748:");
        asm("ADD      r0,r0,r3");
        asm("ADD      r1,r1,r3,ASR #1");
        asm("ADD      r12,r4,r3,ASR #1");
        asm("SUB      r12,r12,#1");
        asm("SUB      r1,r1,#1");
        asm("SUB      r0,r0,#1");
        asm("L1.1772:");
        asm("BEQ      L1.1800");
        asm("RSB      r8,r8,#0");
        asm("MOV      r4,#1");
        asm("STMIB    sp,{r4,r8}");
        asm("RSB      r9,r9,#0");
        asm("STR      r9,[sp,#0]");
        asm("B        L1.1812");
        asm("L1.1800:");
        asm("MVN      r4,#0");
        asm("STMIB    sp,{r4,r8}");
        asm("STR      r9,[sp,#0]");
        asm("L1.1812:");
        asm("STR      r3,[sp,#0x1c]");
        asm("CMP      r3,#0");
        asm("BLE      L1.2824");
        asm("LDR      r3,[sp,#4]");
        asm("LDR      r9,[sp,#0]");
        asm("ADD      r3,r3,r9");
        asm("STR      r3,[sp,#0x38]");
        asm("L1.1840:");
        asm("LDR      r4,[sp,#0x48]");
        asm("MOV      r3,#0");
        asm("LDR      r4,[r4,#0xc]");
        asm("MOV      r5,r3");
        asm("SUBS     r4,r4,#2");
        asm("BMI      L1.2776");
        asm("L1.1864:");
        asm("LDRB     r6,[r1,#0]");
        asm("LDR      r8,[sp,#8]");
        asm("MOV      r9,#0xce");
        asm("LDRB     r7,[r12],r8");
        asm("ADD      r9,r9,#0xb200");
        asm("ADD      r1,r1,r8");
        asm("SUB      r7,r7,#0x80");
        asm("MOV      r11,#0xff");
        asm("ADD      r11,r11,#0x2a00");
        asm("MUL      r8,r9,r7");
        asm("SUB      r6,r6,#0x80");
        asm("MUL      r9,r11,r6");
        asm("ADD      r8,r8,r9,LSL #1");
        asm("LDR      r9,L1.2840");//asm("LDR	  r9, =0x00015f03 ");
        asm("MUL      r7,r9,r7");
        asm("LDR      r9,L1.2844");//asm("LDR	  r9, =0x0001bbd2");
        asm("MUL      r6,r9,r6");
        asm("LDR      r9,[sp,#4]");
        asm("LDRB     r9,[r0,r9]");
        asm("RSB      r11,r8,r9,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("ADD      r11,r6,r9,LSL #16");
        asm("STR      r11,[sp,#0x10]");
        asm("AND      r11,r3,#0xff");
        asm("ADD      r9,r7,r9,LSL #16");
        asm("ADD      r11,r9,r11,LSL #16");
        asm("MOV      r9,#0xff0000");
        asm("STR      r11,[sp,#0x18]");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r9,r9,r3,LSL #8");
        asm("ADD      r11,r9,r11");
        asm("LDR      r9,[sp,#0x10]");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r9,r3,r9");
        asm("STR      r9,[sp,#0x10]");
        asm("STR      r11,[sp,#0x14]");
        asm("LDR      r3,[r10,r4,LSL #2]");
        asm("LDR      r11,[sp,#0x18]");
        asm("AND      r9,r3,#0xff");
        asm("ADD      r9,r11,r9,LSL #16");
        asm("STR      r9,[sp,#0x18]");
        asm("MOV      r9,#0xff0000");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r9,r9,r3,LSL #8");
        asm("ADD      r11,r9,r11");
        asm("LDR      r9,[sp,#0x10]");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r3,r3,r9");
        asm("LDR      r9,[sp,#0x18]");
        asm("LDRB     r9,[lr,r9,ASR #16]");
        asm("STR      r9,[sp,#0x18]");
        asm("LDRB     r9,[lr,r11,ASR #16]");
        asm("STR      r9,[sp,#0x14]");
        asm("LDRB     r3,[lr,r3,ASR #16]");
        asm("LDR      r9,[sp,#0x18]");
        asm("AND      r9,r9,#0xf0");
        asm("MOV      r11,r9,LSL #4");
        asm("LDR      r9,[sp,#0x14]");
        asm("AND      r9,r9,#0xf0");
        asm("ORR      r9,r9,r11");
        asm("ORR      r11,r9,r3,ASR #4");
        asm("STR      r11,[sp,#0xc]");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r3,r3,#0xe");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,LSR #1");
        asm("STR      r11,[sp,#0x14]");
        asm("MOV      r11,r3,LSR #1");
        asm("LDR      r9,[sp,#0x18]");
        asm("STR      r11,[sp,#0x10]");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r9,r9,#0xe");
        asm("MOV      r9,r9,LSR #1");
        asm("ORR      r3,r9,r11,LSL #8");
        asm("LDR      r11,[sp,#0x10]");
        asm("STR      r9,[sp,#0x18]");
        asm("ORR      r9,r3,r11,LSL #16");
        asm("LDR      r3,[sp,#0x38]");
        asm("STR      r9,[sp,#0x2c]");
        asm("LDRB     r3,[r0,r3]");
        asm("LDR      r11,[sp,#0x14]");
        asm("RSB      r9,r8,r3,LSL #16");
        asm("ADD      r11,r9,r11,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("LDR      r11,[sp,#0x10]");
        asm("ADD      r9,r6,r3,LSL #16");
        asm("ADD      r11,r9,r11,LSL #16");
        asm("LDR      r9,[sp,#0x18]");
        asm("STR      r11,[sp,#0x10]");
        asm("ADD      r3,r7,r3,LSL #16");
        asm("ADD      r11,r3,r9,LSL #16");
        asm("ADD      r3,r10,r4,LSL #2");
        asm("STR      r3,[sp,#0x34]");
        asm("LDR      r9,[r3,#4]");
        asm("AND      r3,r9,#0xff");
        asm("ADD      r3,r11,r3,LSL #16");
        asm("STR      r3,[sp,#0x18]");
        asm("MOV      r3,#0xff0000");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r3,r3,r9,LSL #8");
        asm("ADD      r3,r3,r11");
        asm("LDR      r11,[sp,#0x10]");
        asm("AND      r9,r9,#0xff0000");
        asm("ADD      r9,r9,r11");
        asm("LDR      r11,[sp,#0x18]");
        asm("LDRB     r11,[lr,r11,ASR #16]");
        asm("STR      r11,[sp,#0x18]");
        asm("LDRB     r3,[lr,r3,ASR #16]");
        asm("STR      r3,[sp,#0x14]");
        asm("LDRB     r3,[lr,r9,ASR #16]");
        asm("AND      r9,r11,#0xf0");
        asm("LDR      r11,[sp,#0xc]");
        asm("ORR      r9,r11,r9,LSL #20");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r11,r11,#0xf0");
        asm("ORR      r9,r9,r11,LSL #16");
        asm("MOV      r11,r3,LSR #4");
        asm("ORR      r9,r9,r11,LSL #16");
        asm("LDR      r11,[sp,#0x30]");
        asm("AND      r3,r3,#0xe");
        asm("STR      r9,[r2,r11,LSL #1]");
        asm("LDR      r11,[sp,#0x18]");
        asm("MOV      r3,r3,ASR #1");
        asm("AND      r9,r11,#0xe");
        asm("LDR      r11,[sp,#0x14]");
        asm("MOV      r9,r9,ASR #1");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,ASR #1");
        asm("ORR      r9,r9,r11,LSL #8");
        asm("ORR      r3,r9,r3,LSL #16");
        asm("LDRB     r9,[r0,#0]");
        asm("RSB      r11,r8,r9,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("ADD      r11,r6,r9,LSL #16");
        asm("STR      r11,[sp,#0x10]");
        asm("ADD      r11,r7,r9,LSL #16");
        asm("AND      r9,r5,#0xff");
        asm("ADD      r9,r11,r9,LSL #16");
        asm("STR      r9,[sp,#0x18]");
        asm("MOV      r9,#0xff0000");
        asm("AND      r9,r9,r5,LSL #8");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r5,r5,#0xff0000");
        asm("ADD      r9,r9,r11");
        asm("STR      r9,[sp,#0x14]");
        asm("LDR      r11,[sp,#0x10]");
        asm("LDR      r9,[sp,#0x2c]");
        asm("ADD      r5,r5,r11");
        asm("AND      r11,r9,#0xff");
        asm("LDR      r9,[sp,#0x18]");
        asm("ADD      r9,r9,r11,LSL #16");
        asm("STR      r9,[sp,#0x18]");
        asm("LDR      r9,[sp,#0x2c]");
        asm("MOV      r11,#0xff0000");
        asm("AND      r9,r11,r9,LSL #8");
        asm("LDR      r11,[sp,#0x14]");
        asm("ADD      r5,r9,r5");
        asm("ADD      r11,r9,r11");
        asm("LDR      r9,[sp,#0x18]");
        asm("LDRB     r9,[lr,r9,ASR #16]");
        asm("STR      r9,[sp,#0x18]");
        asm("LDRB     r9,[lr,r11,ASR #16]");
        asm("STR      r9,[sp,#0x14]");
        asm("LDRB     r5,[lr,r5,ASR #16]");
        asm("LDR      r9,[sp,#0x18]");
        asm("AND      r9,r9,#0xf0");
        asm("MOV      r11,r9,LSL #4");
        asm("LDR      r9,[sp,#0x14]");
        asm("AND      r9,r9,#0xf0");
        asm("ORR      r9,r9,r11");
        asm("ORR      r9,r9,r5,ASR #4");
        asm("STR      r9,[sp,#0xc]");
        asm("LDR      r9,[sp,#0x18]");
        asm("AND      r5,r5,#0xe");
        asm("AND      r11,r9,#0xe");
        asm("LDR      r9,[sp,#0x14]");
        asm("MOV      r11,r11,LSR #1");
        asm("STR      r11,[sp,#0x18]");
        asm("AND      r9,r9,#0xe");
        asm("MOV      r11,r9,LSR #1");
        asm("LDR      r9,[sp,#0x18]");
        asm("MOV      r5,r5,LSR #1");
        asm("ORR      r9,r9,r11,LSL #8");
        asm("ORR      r9,r9,r5,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("STR      r9,[r10,r4,LSL #2]");
        asm("LDR      r9,[sp,#0]");
        asm("LDRB     r9,[r0,r9]");
        asm("LDR      r11,[sp,#0]");
        asm("ADD      r0,r0,r11,LSL #1");
        asm("LDR      r11,[sp,#0x14]");
        asm("RSB      r8,r8,r9,LSL #16");
        asm("ADD      r6,r6,r9,LSL #16");
        asm("ADD      r5,r6,r5,LSL #16");
        asm("ADD      r8,r8,r11,LSL #16");
        asm("LDR      r11,[sp,#0x18]");
        asm("ADD      r6,r7,r9,LSL #16");
        asm("ADD      r6,r6,r11,LSL #16");
        asm("AND      r7,r3,#0xff");
        asm("ADD      r6,r6,r7,LSL #16");
        asm("MOV      r9,#0xff0000");
        asm("AND      r7,r9,r3,LSL #8");
        asm("ADD      r7,r7,r8");
        asm("AND      r8,r3,r9");
        asm("ADD      r5,r8,r5");
        asm("LDRB     r6,[lr,r6,ASR #16]");
        asm("LDRB     r7,[lr,r7,ASR #16]");
        asm("LDRB     r5,[lr,r5,ASR #16]");
        asm("AND      r8,r6,#0xf0");
        asm("LDR      r9,[sp,#0xc]");
        asm("AND      r6,r6,#0xe");
        asm("ORR      r8,r9,r8,LSL #20");
        asm("AND      r9,r7,#0xf0");
        asm("ORR      r8,r8,r9,LSL #16");
        asm("MOV      r9,r5,ASR #4");
        asm("AND      r7,r7,#0xe");
        asm("MOV      r7,r7,ASR #1");
        asm("AND      r5,r5,#0xe");
        asm("MOV      r6,r6,ASR #1");
        asm("ORR      r6,r6,r7,LSL #8");
        asm("MOV      r5,r5,ASR #1");
        asm("ORR      r8,r8,r9,LSL #16");
        asm("STR      r8,[r2],#4");
        asm("ORR      r5,r6,r5,LSL #16");
        asm("LDR      r6,[sp,#0x34]");
        asm("SUBS     r4,r4,#2");
        asm("STR      r5,[r6,#4]");
        asm("BPL      L1.1864");
        asm("L1.2776:");
        asm("LDR      r3,[sp,#0x28]");
        asm("ADD      r0,r0,r3");
        asm("LDR      r3,[sp,#0x20]");
        asm("ADD      r1,r1,r3");
        asm("ADD      r12,r12,r3");
        asm("LDR      r3,[sp,#0x24]");
        asm("ADD      r2,r2,r3,LSL #1");
        asm("LDR      r3,[sp,#0x1c]");
        asm("SUB      r3,r3,#2");
        asm("STR      r3,[sp,#0x1c]");
        asm("CMP      r3,#0");
        asm("BGT      L1.1840");
        asm("L1.2824:");
        asm("ADD      sp,sp,#0x4c");
        asm("LDMFD    sp!,{r4-r11,lr}");
        asm("MOV      r0,#1");
//        asm("BX       lr");
        asm("L1.2836:");
        asm(".align 0");
        asm("L1.2840:");
        asm(".word      0x00015f03");
        asm("L1.2844:");
        asm(".word      0x0001bbd2");
    }
    return 1;
#else
    return 0;
#endif // CCROTATE
}

#else  // straight C version

int32 ColorConvert12::cc12Rotate(uint8 **src, uint8 *dst,
                                 DisplayProperties *disp, uint8 *clip, uint8 *pErr_horz)
{
#if CCROTATE
    const static uint32	JCoeff[4] =
    {
        45774,	//65536*0.813/1.164;
        89859,	//65536*1.596/1.164;
        22014,	//65536*0.391/1.164;
        113618	//65536*2.018/1.164;
    };

    uint8 *pCb, *pCr;
    uint8 *pY;
    uint16 *pDst;
    int32 src_width, dst_width, display_width;
    int32 Y, Cb, Cr, Cg;
    uint32 left_err_t, left_err_b; /* error from the left pixels */
    uint32 top_err_l, top_err_r; /* error from the top pixels */
    int32 deltaY, deltaDst, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;


    src_width	=	disp->src_pitch;
    dst_width	=	disp->dst_pitch;
    display_width	=	disp->src_width;

    deltaDst	=	(disp->dst_pitch << 1) - disp->dst_width; // (dst_pitch<<1) - src_height
    if (_mRotation == CCROTATE_CLKWISE)
    {	// go from top-left to bottom-left
        deltaY		=  src_width * disp->src_height + 2;
        deltaCbCr	= ((src_width * disp->src_height) >> 2) + 1;
    }
    else  // rotate counterclockwise
    {   // go from bottom-right back to top-right
        deltaY		=  -(src_width * disp->src_height + 2);
        deltaCbCr	=  -(((src_width * disp->src_height) >> 2) + 1);
    }

    // map origin of the destination to the source
    if (_mRotation == CCROTATE_CLKWISE)
    {	// goto bottom-left
        pY = src[0] + src_width * (disp->src_height - 1);
        pCb = src[1] + ((src_width >> 1) * ((disp->src_height >> 1) - 1));
        pCr = src[2] + ((src_width >> 1) * ((disp->src_height >> 1) - 1));
    }
    else  // rotate counterclockwise
    {   // goto top-right
        pY = src[0] + display_width - 1;
        pCb = src[1] + (display_width >> 1) - 1;
        pCr = src[2] + (display_width >> 1) - 1;
    }

    pDst =	(uint16 *)dst;

    int half_src_width, read_idx, tmp_src_width;
    if (_mRotation == CCROTATE_CLKWISE)
    {
        half_src_width = -(src_width >> 1);
        read_idx = 1;
        tmp_src_width = -src_width;
    }
    else // rotate counterclockwise
    {
        half_src_width = (src_width >> 1);
        read_idx = -1;
        tmp_src_width = src_width;
    }

    for (row = display_width; row > 0; row -= 2)
    {

        left_err_t = left_err_b = 0;  /* reset left pixel errors */

        for (col = disp->src_height - 2; col >= 0; col -= 2)
        {

            Cb = *pCb;
            pCb += half_src_width;
            Cr = *pCr;
            pCr += half_src_width;

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * JCoeff[0];
            Cg	+=	Cb * JCoeff[2];

            Cr	*=	JCoeff[1];
            Cb	*=	JCoeff[3];

            //process the bottom two pixels in RGB plane
            tmp0	=	pY[read_idx]; /* top-left pixel */

            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            //add the error from top and left
            //This is not exact error diffusion since
            //we process the bottom two pixels first !!
            tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
            tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
            tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

            top_err_l = *((uint*)(pErr_horz + (col << 2)));
            tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
            tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
            tmp2	+=	(((top_err_l >> 16) & 0xFF) << 16);//mErr_horz[2][col];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		= (tmp0 >> 3);
            rgb		= (tmp1 >> 2) | (rgb << 6);
            rgb		= (tmp2 >> 3) | (rgb << 5);
#else
            //RGB_444
            rgb		= (tmp0 & 0xF0) << 4;
            rgb		|= (tmp1 & 0xF0);
            rgb		|= (tmp2 >> 4);
#endif
            tmp0	&=	0xE;
            tmp1	&=	0xE;
            tmp2	&=	0xE;
            tmp0	>>= 1;
            tmp1	>>= 1;
            tmp2	>>= 1;

            //save error
            top_err_l = tmp0 | (tmp1 << 8) | (tmp2 << 16);

            Y		=	pY[read_idx+tmp_src_width];  /* bottom-left pixel */

            // error from the left already available
            tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
            tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
            tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

            //add the error from top
            top_err_r = *((uint*)(pErr_horz + (col << 2) + 4));
            tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
            tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
            tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		|= ((tmp0 >> 3) << 27);
            rgb		|= ((tmp1 >> 2) << 21);
            rgb		|= ((tmp2 >> 3) << 16);
#else
            //RGB_444
            rgb		|= (tmp0 & 0xF0) << 20;
            rgb		|= (tmp1 & 0xF0) << 16;
            rgb		|= (tmp2 >> 4) << 16;
#endif
            *((uint*)(pDst + dst_width))	= rgb;

            tmp0	&=	0xE;
            tmp1	&=	0xE;
            tmp2	&=	0xE;
            tmp0	>>= 1;
            tmp1	>>= 1;
            tmp2	>>= 1;

            //save error
            left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16);
            top_err_r = left_err_b;

            //process the top two pixels in RGB plane

            tmp0	=	*pY; /*upper-right pixel */
            tmp1	=	(tmp0 << 16) - Cg;
            tmp2	=	(tmp0 << 16) + Cb;
            tmp0	=	(tmp0 << 16) + Cr;

            //add the error from top and left
            tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
            tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
            tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

            tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
            tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
            tmp2	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[2][col];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		= (tmp0 >> 3);
            rgb		= (tmp1 >> 2) | (rgb << 6);
            rgb		= (tmp2 >> 3) | (rgb << 5);
#else
            //RGB_444
            rgb		= (tmp0 & 0xF0) << 4;
            rgb		|= (tmp1 & 0xF0);
            rgb		|= (tmp2 >> 4);
#endif

            tmp0	&=	0xE;
            tmp1	&=	0xE;
            tmp2	&=	0xE;
            tmp0 >>= 1;
            tmp1 >>= 1;
            tmp2 >>= 1;

            top_err_l = tmp0 | (tmp1 << 8) | (tmp2 << 16);
            *((uint*)(pErr_horz + (col << 2))) = top_err_l;

            Y		=	pY[tmp_src_width]; /* bottom-right pixel */
            pY += (tmp_src_width << 1);

            //error from the left is already available
            tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
            tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
            tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

            //add the error from top
            tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
            tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
            tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

            tmp0	=	clip[tmp0>>16];
            tmp1	=	clip[tmp1>>16];
            tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
            //RGB_565
            rgb		|= ((tmp0 >> 3) << 27);
            rgb		|= ((tmp1 >> 2) << 21);
            rgb		|= ((tmp2 >> 3) << 16);
#else
            //RGB_444
            rgb		|= (tmp0 & 0xF0) << 20;
            rgb		|= (tmp1 & 0xF0) << 16;
            rgb		|= (tmp2 >> 4) << 16;
#endif

            *((uint *)pDst)	= rgb;

            pDst += 2;

            tmp0	&=	0xE;
            tmp1	&=	0xE;
            tmp2	&=	0xE;
            tmp0 >>= 1;
            tmp1 >>= 1;
            tmp2 >>= 1;

            // save error
            left_err_t = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_r
            *((uint*)(pErr_horz + (col << 2) + 4)) = left_err_t;

        }//end of COL

        pY	+=	deltaY;
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;
        pDst +=	(deltaDst);	//coz pDst defined as UINT *
    }
    return 1;
#else
    return 0;
#endif // CCROTATE
}
#endif // USE_ARM_ASM_FOR_COLORCONVERT


// platform specific C function
int32 cc12scaling(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip, uint8 *pErr_horz,
                  uint8 *_mRowPix, uint8 *_mColPix);

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().

int32 ColorConvert12::cc12ZoomIn(uint8 **src, uint8 *dst,
                                 DisplayProperties *disp, uint8 *clip, uint8 *pErr_horz)
{
    int32 disp_prop[6];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    return cc12scaling(src, dst, disp_prop, clip, pErr_horz, _mRowPix, _mColPix);
}

#if USE_ARM_ASM_FOR_COLORCONVERT
int32 cc12scaling(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip, uint8 *pErr_horz,
                  uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING
    {
        asm("STMFD    sp!,{r4-r11,lr}");
        asm("SUB      sp,sp,#0x44");
        asm("LDR      r7,[sp,#0x6c]");
        asm("LDR      r12,[r2,#0]");
        asm("MOV      r5,r3");
        asm("STR      r12,[sp,#0x24]");
        asm("LDR      r3,[r2,#8]");
        asm("LDR      r4,[r2,#4]");
        asm("RSB      r12,r3,r12,LSL #1");
        asm("STR      r12,[sp,#0x1c]");
        asm("LDR      r12,[sp,#0x24]");
        asm("SUB      r12,r12,r3");
        asm("MOV      r12,r12,ASR #1");
        asm("STR      r12,[sp,#0x14]");
        asm("LDR      r12,[r2,#0x10]");
        asm("STR      r12,[sp,#0x18]");
        asm("LDMIA    r0,{r6,lr}");
        asm("STR      lr,[sp,#0x2c]");
        asm("LDR      r8,[r0,#8]");
        asm("MOV      r0,r1");
        asm("STR      r8,[sp,#0x28]");
        asm("LDR      r1,[r2,#0xc]");
        asm("SUB      r2,r1,#1");
        asm("STR      r2,[sp,#0x10]");
        asm("CMP      r2,#0");
        asm("BLT      L1.4684");
        asm("SUB      r1,r3,#2");
        asm("STR      r1,[sp,#0x40]");
        asm("LDR      r1,[sp,#0x18]");
        asm("MOV      r1,r1,LSL #1");
        asm("STR      r1,[sp,#0x3c]");
        asm("L1.3020:");
        asm("LDR      r1,[sp,#0x70]");
        asm("LDR      r2,[sp,#0x10]");
        asm("ADD      r1,r1,r2");
        asm("STR      r1,[sp,#0x38]");
        asm("LDRB     r1,[r1,#-1]");
        asm("CMP      r1,#0");
        asm("BNE      L1.3104");
        asm("LDR      r1,[sp,#0x70]");
        asm("LDR      r2,[sp,#0x10]");
        asm("LDRB     r1,[r1,r2]");
        asm("CMP      r1,#0");
        asm("BNE      L1.3104");
        asm("LDR      r3,[sp,#0x24]");
        asm("LDR      lr,[sp,#0x2c]");
        asm("LDR      r8,[sp,#0x28]");
        asm("ADD      lr,lr,r3,ASR #1");
        asm("ADD      r8,r8,r3,ASR #1");
        asm("STR      r8,[sp,#0x28]");
        asm("STR      lr,[sp,#0x2c]");
        asm("ADD      r6,r6,r3,LSL #1");
        asm("B        L1.4664");
        asm("L1.3104:");
        asm("LDR      r1,[sp,#0x40]");
        asm("MOV      r2,#0");
        asm("MOV      r12,r2");
        asm("CMP      r1,#0");
        asm("BLT      L1.4364");
        asm("L1.3124:");
        asm("LDR      lr,[sp,#0x2c]");
        asm("MOV      r9,#0xce");
        asm("LDRB     r3,[lr],#1");
        asm("LDR      r8,[sp,#0x28]");
        asm("STR      lr,[sp,#0x2c]");
        asm("LDRB     lr,[r8],#1");
        asm("STR      r8,[sp,#0x28]");
        asm("ADD      r9,r9,#0xb200");
        asm("SUB      lr,lr,#0x80");
        asm("MOV      r10,#0xff");
        asm("ADD      r10,r10,#0x2a00");
        asm("MUL      r8,r9,lr");
        asm("SUB      r3,r3,#0x80");
        asm("MUL      r9,r10,r3");
        asm("LDR      r10,L1.4692");
        asm("ADD      r9,r8,r9,LSL #1");
        asm("MUL      r8,r10,lr");
        asm("LDR      r10,L1.4696");
        asm("MUL      lr,r10,r3");
        asm("LDRB     r3,[r7,r1]");
        asm("CMP      r3,#0");
        asm("ADDEQ    r6,r6,#1");
        asm("BEQ      L1.3796");
        asm("LDR      r3,[sp,#0x24]");
        asm("TST      r0,#3");
        asm("LDRB     r10,[r6,r3]");
        asm("RSB      r11,r9,r10,LSL #16");
        asm("STR      r11,[sp,#8]");
        asm("ADD      r11,r8,r10,LSL #16");
        asm("ADD      r3,lr,r10,LSL #16");
        asm("AND      r10,r12,#0xff");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("MOV      r10,#0xff0000");
        asm("STR      r11,[sp,#0xc]");
        asm("LDR      r11,[sp,#8]");
        asm("AND      r10,r10,r12,LSL #8");
        asm("ADD      r11,r10,r11");
        asm("STR      r11,[sp,#8]");
        asm("LDR      r10,[sp,#0x68]");
        asm("AND      r12,r12,#0xff0000");
        asm("ADD      r12,r12,r3");
        asm("LDR      r3,[r10,r1,LSL #2]");
        asm("LDR      r11,[sp,#0xc]");
        asm("AND      r10,r3,#0xff");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("STR      r10,[sp,#0xc]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r3,LSL #8");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r3,r3,r12");
        asm("LDR      r11,[sp,#8]");
        asm("LDR      r12,[sp,#0xc]");
        asm("ADD      r10,r10,r11");
        asm("LDRB     r11,[r5,r12,ASR #16]");
        asm("STR      r11,[sp,#0xc]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("LDRB     r12,[r5,r3,ASR #16]");
        asm("AND      r3,r11,#0xf0");
        asm("MOV      r11,r3,LSL #4");
        asm("AND      r3,r10,#0xf0");
        asm("ORR      r3,r3,r11");
        asm("LDR      r11,[sp,#0xc]");
        asm("AND      r10,r10,#0xe");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,LSR #1");
        asm("STR      r11,[sp,#0xc]");
        asm("MOV      r11,r10,LSR #1");
        asm("ORR      r3,r3,r12,ASR #4");
        asm("AND      r12,r12,#0xe");
        asm("STR      r11,[sp,#8]");
        asm("MOV      r11,r12,LSR #1");
        asm("STR      r11,[sp,#4]");
        asm("LDR      r11,[sp,#0xc]");
        asm("LDR      r12,[sp,#8]");
        asm("ORR      r12,r11,r12,LSL #8");
        asm("LDR      r11,[sp,#4]");
        asm("LDRB     r10,[r6],#1");
        asm("ORR      r12,r12,r11,LSL #16");
        asm("LDR      r11,[sp,#8]");
        asm("STR      r10,[sp,#0x20]");
        asm("RSB      r10,r9,r10,LSL #16");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#8]");
        asm("LDR      r10,[sp,#0x20]");
        asm("LDR      r11,[sp,#4]");
        asm("ADD      r10,lr,r10,LSL #16");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#4]");
        asm("LDR      r10,[sp,#0x20]");
        asm("LDR      r11,[sp,#0xc]");
        asm("ADD      r10,r8,r10,LSL #16");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("AND      r11,r2,#0xff");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#0xc]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r11,r10,r2,LSL #8");
        asm("LDR      r10,[sp,#8]");
        asm("AND      r2,r2,#0xff0000");
        asm("ADD      r11,r11,r10");
        asm("LDR      r10,[sp,#4]");
        asm("ADD      r2,r2,r10");
        asm("LDR      r10,[sp,#0xc]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("STR      r10,[sp,#0xc]");
        asm("LDRB     r11,[r5,r11,ASR #16]");
        asm("LDRB     r10,[r5,r2,ASR #16]");
        asm("STR      r10,[sp,#4]");
        asm("LDR      r10,[sp,#0xc]");
        asm("MOV      r2,r10,LSL #28");
        asm("MOV      r2,r2,LSR #29");
        asm("AND      r10,r11,#0xe");
        asm("ORR      r2,r2,r10,LSL #7");
        asm("LDR      r10,[sp,#4]");
        asm("AND      r11,r11,#0xf0");
        asm("AND      r10,r10,#0xe");
        asm("ORR      r2,r2,r10,LSL #15");
        asm("LDR      r10,[sp,#0x68]");
        asm("STR      r2,[r10,r1,LSL #2]");
        asm("LDR      r10,[sp,#0xc]");
        asm("AND      r10,r10,#0xf0");
        asm("MOV      r10,r10,LSL #4");
        asm("STR      r10,[sp,#0xc]");
        asm("ORR      r10,r11,r10");
        asm("LDR      r11,[sp,#4]");
        asm("ORR      r10,r10,r11,ASR #4");
        asm("LDRB     r11,[r7,r1]");
        asm("BEQ      L1.3720");
        asm("CMP      r11,#2");
        asm("BNE      L1.3668");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRH     r3,[r11,#0]");
        asm("STRH     r3,[r11,#2]");
        asm("STRH     r10,[r0,#0]");
        asm("STRH     r10,[r0,#2]");
        asm("B        L1.3796");
        asm("L1.3668:");
        asm("CMP      r11,#3");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRNEH   r3,[r11,#0]");
        asm("STRNEH   r10,[r0,#0]");
        asm("BNE      L1.3796");
        asm("STRH     r3,[r11],#2");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r11,#0]");
        asm("STRH     r10,[r0,#0]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("MOV      r10,#2");
        asm("STR      r3,[r10,r0]");
        asm("B        L1.3796");
        asm("L1.3720:");
        asm("CMP      r11,#2");
        asm("BNE      L1.3748");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r0,r4,LSL #1]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("B        L1.3796");
        asm("L1.3748:");
        asm("CMP      r11,#3");
        asm("ADDNE    r11,r0,r4,LSL #1");
        asm("STRNEH   r3,[r11,#0]");
        asm("STRNEH   r10,[r0,#0]");
        asm("BNE      L1.3796");
        asm("ORR      r11,r3,r3,LSL #16");
        asm("STR      r11,[r0,r4,LSL #1]");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRH     r3,[r11,#4]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("STRH     r10,[r0,#4]");
        asm("L1.3796:");
        asm("LDRB     r3,[r7,r1]");
        asm("ADD      r0,r0,r3,LSL #1");
        asm("ADD      r3,r7,r1");
        asm("STR      r3,[sp,#0x34]");
        asm("LDRB     r3,[r3,#1]");
        asm("CMP      r3,#0");
        asm("ADDEQ    r6,r6,#1");
        asm("BEQ      L1.4344");
        asm("LDR      r3,[sp,#0x24]");
        asm("TST      r0,#3");
        asm("LDRB     r10,[r6,r3]");
        asm("RSB      r11,r9,r10,LSL #16");
        asm("STR      r11,[sp,#8]");
        asm("ADD      r11,r8,r10,LSL #16");
        asm("ADD      r3,lr,r10,LSL #16");
        asm("AND      r10,r12,#0xff");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("MOV      r10,#0xff0000");
        asm("STR      r11,[sp,#0xc]");
        asm("LDR      r11,[sp,#8]");
        asm("AND      r10,r10,r12,LSL #8");
        asm("ADD      r11,r10,r11");
        asm("STR      r11,[sp,#8]");
        asm("LDR      r10,[sp,#0x68]");
        asm("AND      r12,r12,#0xff0000");
        asm("ADD      r12,r12,r3");
        asm("ADD      r3,r10,r1,LSL #2");
        asm("STR      r3,[sp,#0x30]");
        asm("LDR      r3,[r3,#4]");
        asm("LDR      r11,[sp,#0xc]");
        asm("AND      r10,r3,#0xff");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("STR      r10,[sp,#0xc]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r3,LSL #8");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r3,r3,r12");
        asm("LDR      r11,[sp,#8]");
        asm("LDR      r12,[sp,#0xc]");
        asm("ADD      r10,r10,r11");
        asm("LDRB     r11,[r5,r12,ASR #16]");
        asm("STR      r11,[sp,#0xc]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("LDRB     r12,[r5,r3,ASR #16]");
        asm("AND      r3,r11,#0xf0");
        asm("AND      r11,r10,#0xf0");
        asm("ORR      r3,r11,r3,LSL #4");
        asm("AND      r10,r10,#0xe");
        asm("MOV      r10,r10,LSR #1");
        asm("ORR      r3,r3,r12,ASR #4");
        asm("AND      r12,r12,#0xe");
        asm("STR      r10,[sp,#8]");
        asm("LDR      r11,[sp,#0xc]");
        asm("MOV      r10,r12,LSR #1");
        asm("STR      r10,[sp,#4]");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,LSR #1");
        asm("LDR      r10,[sp,#8]");
        asm("STR      r11,[sp,#0xc]");
        asm("ORR      r12,r11,r10,LSL #8");
        asm("LDR      r10,[sp,#4]");
        asm("LDRB     r11,[r6],#1");
        asm("ORR      r12,r12,r10,LSL #16");
        asm("LDR      r10,[sp,#8]");
        asm("RSB      r9,r9,r11,LSL #16");
        asm("ADD      r9,r9,r10,LSL #16");
        asm("ADD      lr,lr,r11,LSL #16");
        asm("LDR      r10,[sp,#4]");
        asm("ADD      r8,r8,r11,LSL #16");
        asm("LDR      r11,[sp,#0xc]");
        asm("ADD      lr,lr,r10,LSL #16");
        asm("AND      r10,r2,#0xff");
        asm("ADD      r8,r8,r11,LSL #16");
        asm("ADD      r8,r8,r10,LSL #16");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r2,LSL #8");
        asm("AND      r2,r2,#0xff0000");
        asm("ADD      r2,r2,lr");
        asm("LDRB     lr,[r5,r8,ASR #16]");
        asm("ADD      r9,r10,r9");
        asm("LDRB     r8,[r5,r9,ASR #16]");
        asm("LDRB     r9,[r5,r2,ASR #16]");
        asm("MOV      r2,lr,LSL #28");
        asm("AND      r10,r8,#0xe");
        asm("MOV      r2,r2,LSR #29");
        asm("ORR      r2,r2,r10,LSL #7");
        asm("AND      r10,r9,#0xe");
        asm("ORR      r2,r2,r10,LSL #15");
        asm("LDR      r10,[sp,#0x30]");
        asm("AND      r8,r8,#0xf0");
        asm("AND      lr,lr,#0xf0");
        asm("ORR      lr,r8,lr,LSL #4");
        asm("STR      r2,[r10,#4]");
        asm("LDR      r8,[sp,#0x34]");
        asm("ORR      lr,lr,r9,ASR #4");
        asm("LDRB     r8,[r8,#1]");
        asm("BEQ      L1.4268");
        asm("CMP      r8,#2");
        asm("BNE      L1.4216");
        asm("ADD      r8,r0,r4,LSL #1");
        asm("STRH     r3,[r8,#0]");
        asm("STRH     r3,[r8,#2]");
        asm("STRH     lr,[r0,#0]");
        asm("STRH     lr,[r0,#2]");
        asm("B        L1.4344");
        asm("L1.4216:");
        asm("CMP      r8,#3");
        asm("ADD      r8,r0,r4,LSL #1");
        asm("STRNEH   r3,[r8,#0]");
        asm("STRNEH   lr,[r0,#0]");
        asm("BNE      L1.4344");
        asm("STRH     r3,[r8],#2");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r8,#0]");
        asm("STRH     lr,[r0,#0]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("MOV      lr,#2");
        asm("STR      r3,[lr,r0]");
        asm("B        L1.4344");
        asm("L1.4268:");
        asm("CMP      r8,#2");
        asm("BNE      L1.4296");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r0,r4,LSL #1]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("B        L1.4344");
        asm("L1.4296:");
        asm("CMP      r8,#3");
        asm("ADDNE    r8,r0,r4,LSL #1");
        asm("STRNEH   r3,[r8,#0]");
        asm("STRNEH   lr,[r0,#0]");
        asm("BNE      L1.4344");
        asm("ORR      r8,r3,r3,LSL #16");
        asm("STR      r8,[r0,r4,LSL #1]");
        asm("ADD      r8,r0,r4,LSL #1");
        asm("STRH     r3,[r8,#4]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("STRH     lr,[r0,#4]");
        asm("L1.4344:");
        asm("LDR      r3,[sp,#0x34]");
        asm("SUBS     r1,r1,#2");
        asm("LDRB     r3,[r3,#1]");
        asm("ADD      r0,r0,r3,LSL #1");
        asm("BPL      L1.3124");
        asm("L1.4364:");
        asm("LDR      r1,[sp,#0x1c]");
        asm("LDR      lr,[sp,#0x2c]");
        asm("ADD      r6,r6,r1");
        asm("LDR      r1,[sp,#0x14]");
        asm("LDR      r8,[sp,#0x28]");
        asm("ADD      lr,lr,r1");
        asm("ADD      r8,r8,r1");
        asm("LDR      r1,[sp,#0x18]");
        asm("STR      r8,[sp,#0x28]");
        asm("SUB      r8,r0,r1,LSL #1");
        asm("LDR      r1,[sp,#0x70]");
        asm("LDR      r2,[sp,#0x10]");
        asm("STR      lr,[sp,#0x2c]");
        asm("LDRB     r0,[r1,r2]");
        asm("LDR      r1,[sp,#0x38]");
        asm("LDRB     r1,[r1,#-1]");
        asm("MUL      r9,r4,r0");
        asm("CMP      r1,#0");
        asm("BEQ      L1.4552");
        asm("CMP      r0,#1");
        asm("BEQ      L1.4464");
        asm("ADD      r1,r8,r4,LSL #1");
        asm("ADD      r0,r8,r9,LSL #1");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("L1.4464:");
        asm("LDR      r1,[sp,#0x38]");
        asm("LDRB     r0,[r1,#-1]");
        asm("CMP      r0,#2");
        asm("BNE      L1.4504");
        asm("ADD      r0,r8,r9,LSL #1");
        asm("ADD      r0,r0,r4,LSL #1");
        asm("ADD      r1,r8,r4,LSL #1");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("B        L1.4552");
        asm("L1.4504:");
        asm("CMP      r0,#3");
        asm("BNE      L1.4552");
        asm("ADD      r10,r8,r9,LSL #1");
        asm("ADD      r9,r8,r4,LSL #1");
        asm("MOV      r1,r9");
        asm("ADD      r0,r10,r4,LSL #1");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("ADD      r0,r10,r4,LSL #2");
        asm("MOV      r1,r9");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("L1.4552:");
        asm("LDR      r1,[sp,#0x70]");
        asm("LDR      r2,[sp,#0x10]");
        asm("LDRB     r0,[r1,r2]");
        asm("CMP      r0,#2");
        asm("BNE      L1.4592");
        asm("ADD      r0,r8,r4,LSL #1");
        asm("MOV      r1,r8");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("B        L1.4632");
        asm("L1.4592:");
        asm("CMP      r0,#3");
        asm("BNE      L1.4632");
        asm("ADD      r0,r8,r4,LSL #1");
        asm("MOV      r1,r8");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("ADD      r0,r8,r4,LSL #2");
        asm("MOV      r1,r8");
        asm("LDR      r2,[sp,#0x3c]");
        asm("BL       memcpy");
        asm("L1.4632:");
        asm("LDR      r1,[sp,#0x38]");
        asm("LDRB     r0,[r1,#-1]");
        asm("LDR      r1,[sp,#0x70]");
        asm("LDR      r2,[sp,#0x10]");
        asm("LDRB     r1,[r1,r2]");
        asm("ADD      r0,r0,r1");
        asm("MUL      r0,r4,r0");
        asm("ADD      r0,r8,r0,LSL #1");
        asm("L1.4664:");
        asm("LDR      r2,[sp,#0x10]");
        asm("SUB      r2,r2,#2");
        asm("STR      r2,[sp,#0x10]");
        asm("CMP      r2,#0");
        asm("BGE      L1.3020");
        asm("L1.4684:");
        asm("ADD      sp,sp,#0x44");
        asm("LDMFD    sp!,{r4-r11,lr}");
        asm("MOV      r0,#1");
//        asm("BX       lr");
        asm("L1.4688:");
        asm(".align		0");
        asm("L1.4692:");
        asm(".word      0x00015f03");
        asm("L1.4696:");
        asm(".word      0x0001bbd2");
    }
    return 1;
#else
    return 0;
#endif // CCSCALING
}

#else // straight C version

int32 cc12scaling(uint8 **src, uint8 *dst, int *disp,
                  uint8 *clip, uint8 *pErr_horz,
                  uint8 *_mRowPix, uint8 *_mColPix)
{
#if CCSCALING
    const static uint32	JCoeff[4] =
    {
        45774,	//65536*0.813/1.164;
        89859,	//65536*1.596/1.164;
        22014,	//65536*0.391/1.164;
        113618	//65536*2.018/1.164;
    };

    uint8 *pCb, *pCr, *pY;
    uint16 *pDst;
    int32 src_pitch, dst_pitch, src_width;
    int32 Y, Cb, Cr, Cg;
    uint32 left_err_t, left_err_b; /* error from the left pixels */
    uint32 top_err_l, top_err_r; /* error from the top pixels */
    int32 deltaY, dst_width, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 offset;


    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];

    deltaY		=	(src_pitch << 1) - src_width;
    deltaCbCr	=	(src_pitch - src_width) >> 1;
    dst_width	=	disp[4];

    pY = src[0];
    pCb = src[1];
    pCr = src[2];

    pDst =	(uint16 *)dst;

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

        left_err_t = left_err_b = 0;  /* reset left pixel errors */

        for (col = src_width - 2; col >= 0; col -= 2)
        { /* decrement index, _mRowPix[.] is
													 symmetric to increment index */

            Cb = *pCb++;
            Cr = *pCr++;

            //load the bottom two pixels
            //Y	=	*(((uint16 *)pY)+src_pitch);
            //Y	=	*((uint16 *)(((uint16 *)pY) + src_pitch));

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * JCoeff[0];
            Cg	+=	Cb * JCoeff[2];

            Cr	*=	JCoeff[1];
            Cb	*=	JCoeff[3];

            if (_mRowPix[col]) /* compute this pixel */
            {
                tmp0	=	pY[src_pitch];			//bottom left

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
                tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
                tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

                top_err_l = *((uint*)(pErr_horz + (col << 2)));
                tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
                tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
                tmp2	+=	(((top_err_l >> 16) & 0xFF) << 16);//mErr_horz[2][col];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#ifdef	OUTPUT_RGB_565
                //RGB_565
                rgb		= (tmp0 >> 3);
                rgb		= (tmp1 >> 2) | (rgb << 6);
                rgb		= (tmp2 >> 3) | (rgb << 5);
#else
                //RGB_444
                rgb		= (tmp0 & 0xF0) << 4;
                rgb		|= (tmp1 & 0xF0);
                rgb		|= (tmp2 >> 4);
#endif
                tmp0	&= 0xE;
                tmp1	&= 0xE;
                tmp2	&= 0xE;
                tmp0	>>= 1;
                tmp1	>>= 1;
                tmp2	>>= 1;

                //save error
                left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_l also

                Y	=	*pY++;						//upper left

                // error from the top already available
                tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
                tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
                tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

                //add the error from left
                tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
                tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
                tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

                //save error
                left_err_t = ((tmp0 & 0xE) >> 1) | ((tmp1 & 0xE) << 7) | ((tmp2 & 0xE) << 15); //= top_err_l also
                *((uint*)(pErr_horz + (col << 2))) = left_err_t; // =top_err_l;

#ifdef	OUTPUT_RGB_565
                //RGB_565
                tmp0		= (tmp0 >> 3);
                tmp0		= (tmp1 >> 2) | (tmp0 << 6);
                tmp0		= (tmp2 >> 3) | (tmp0 << 5);
#else
                //RGB_444
                tmp0		= (tmp0 & 0xF0) << 4;
                tmp0		|= (tmp1 & 0xF0);
                tmp0		|= (tmp2 >> 4);
#endif
                if (((uint)pDst) & 3)  //half word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst + dst_pitch + 1)	=	rgb;
                        *(pDst)	=	tmp0;
                        *(pDst + 1)	=	tmp0;
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *((uint*)(pDst + dst_pitch + 1))	= rgb | (rgb << 16);

                        *(pDst)	=	tmp0;
                        *((uint*)(pDst + 1))	= tmp0 | (tmp0 << 16);
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *(pDst + dst_pitch + 2)	=	rgb;

                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                        *(pDst + 2)	=	tmp0;
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
            } /*	if(_mRowPix[col])  */
            else
            {
                pY++;
            }

            pDst	+= _mRowPix[col];

            if (_mRowPix[col+1]) /* compute this pixel */
            {
                //load the top two pixels
                //Y	=	*((uint16 *)pY)++;
                tmp0	=	pY[src_pitch];		//bottom right

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                //add the error from top and left
                tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
                tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
                tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

                top_err_r = *((uint*)(pErr_horz + (col << 2) + 4));
                tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
                tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
                tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
                //RGB_565
                rgb		= (tmp0 >> 3);
                rgb		= (tmp1 >> 2) | (rgb << 6);
                rgb		= (tmp2 >> 3) | (rgb << 5);
#else
                //RGB_444
                rgb		= (tmp0 & 0xF0) << 4;
                rgb		|= (tmp1 & 0xF0);
                rgb		|= (tmp2 >> 4);
#endif

                tmp0	&=	0xE;
                tmp1	&=	0xE;
                tmp2	&=	0xE;
                tmp0	>>= 1;
                tmp1	>>= 1;
                tmp2	>>= 1;

                //save error
                left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_r also

                Y	=	*pY++;

                // error from the top already available
                tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
                tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
                tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

                //add the error from left
                tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
                tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
                tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

                //save error
                left_err_t = ((tmp0 & 0xE) >> 1) | ((tmp1 & 0xE) << 7) | ((tmp2 & 0xE) << 15);//= top_err_r also
                *((uint*)(pErr_horz + (col << 2) + 4)) = left_err_t;// =top_err_r;

#ifdef	OUTPUT_RGB_565
                //RGB_565
                tmp0		= (tmp0 >> 3);
                tmp0		= (tmp1 >> 2) | (tmp0 << 6);
                tmp0		= (tmp2 >> 3) | (tmp0 << 5);
#else
                //RGB_444
                tmp0		= (tmp0 & 0xF0) << 4;
                tmp0		|= (tmp1 & 0xF0);
                tmp0		|= (tmp2 >> 4);
#endif

                if (((uint)pDst) & 3)  //half word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst + dst_pitch + 1)	=	rgb;
                        *(pDst)	=	tmp0;
                        *(pDst + 1)	=	tmp0;
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *((uint*)(pDst + dst_pitch + 1))	= rgb | (rgb << 16);

                        *(pDst)	=	tmp0;
                        *((uint*)(pDst + 1))	= tmp0 | (tmp0 << 16);
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *(pDst + dst_pitch + 2)	=	rgb;

                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                        *(pDst + 2)	=	tmp0;
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
            } /*	if(_mRowPix[col])  */
            else
            {
                pY++;
            }

            pDst	+= _mRowPix[col+1];
        }//end of COL

        pY	+=	(deltaY);
        pCb	+=	deltaCbCr;
        pCr	+=	deltaCbCr;

        pDst -=	(dst_width);	//goes back to the beginning of the line;

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

        //copy up
        if (_mColPix[row] == 2)
            oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
        else if (_mColPix[row] == 3)
        {
            oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
            oscl_memcpy(pDst + dst_pitch*2, pDst, dst_width*2);
        }

        pDst	+=	dst_pitch * (_mColPix[row-1] + _mColPix[row]);
    }
    return 1;
#else
    return 0;
#endif // CCSCALING
}
#endif // USE_ARM_ASM_FOR_COLORCONVERT


// platform specfic function in C
int32 cc12sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *pErr_horz,
                    uint8 *_mRowPix, uint8 *_mColPix,
                    bool _mIsRotateClkwise);

int32 ColorConvert12::cc12ZoomRotate(uint8 **src, uint8 *dst,
                                     DisplayProperties *disp, uint8 *clip, uint8 *pErr_horz)
{
    int disp_prop[6];

    disp_prop[0] = disp->src_pitch;
    disp_prop[1] = disp->dst_pitch;
    disp_prop[2] = disp->src_width;
    disp_prop[3] = disp->src_height;
    disp_prop[4] = disp->dst_width;
    disp_prop[5] = disp->dst_height;

    return cc12sc_rotate(src, dst, disp_prop, clip, pErr_horz, _mRowPix, _mColPix, (_mRotation == CCROTATE_CLKWISE));
}


#if USE_ARM_ASM_FOR_COLORCONVERT
/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 cc12sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *pErr_horz,
                    uint8 *_mRowPix, uint8 *_mColPix,
                    bool _mIsRotateClkwise)
{
#if (CCROTATE && CCSCALING)
    {
        asm("STMFD    sp!,{r0-r11,lr}");
        asm("SUB      sp,sp,#0x4c");
        asm("MOV      lr,r0");
        asm("MOV      r0,r1");
        asm("LDR      r1,[sp,#0x54]");
        asm("LDR      r9,[sp,#0x84]");
        asm("LDR      r11,[sp,#0x8c]");
        asm("LDR      r2,[r1,#8]");
        asm("LDR      r4,[r1,#4]");
        asm("LDR      r10,[r1,#0]");
        asm("LDR      r1,[r1,#0x10]");
        asm("MOV      r5,r3");
        asm("STR      r1,[sp,#0x24]");
        asm("LDMIA    lr,{r6,r12}");
        asm("LDR      r3,[lr,#8]");
        asm("LDR      r1,[sp,#0x54]");
        asm("MOV      r7,#1");
        asm("LDR      lr,[r1,#0]");
        asm("LDR      r1,[r1,#0xc]");
        asm("CMP      r11,#0");
        asm("MUL      r8,lr,r1");
        asm("ADD      r7,r7,r8,ASR #2");
        asm("ADD      r8,r8,#2");
        asm("STRNE    r8,[sp,#0x28]");
        asm("STRNE    r7,[sp,#0x20]");
        asm("MOV      lr,lr,ASR #1");
        asm("BNE      L1.4940");
        asm("RSB      r8,r8,#0");
        asm("RSB      r7,r7,#0");
        asm("STR      r7,[sp,#0x20]");
        asm("STR      r8,[sp,#0x28]");
        asm("L1.4940:");
        asm("BEQ      L1.4972");
        asm("SUB      r7,r1,#1");
        asm("MLA      r6,r7,r10,r6");
        asm("MVN      r7,#0");
        asm("ADD      r1,r7,r1,ASR #1");
        asm("MLA      r7,r1,lr,r12");
        asm("MLA      r8,r1,lr,r3");
        asm("B        L1.4996");
        asm("L1.4972:");
        asm("ADD      r1,r6,r2");
        asm("SUB      r6,r1,#1");
        asm("ADD      r1,r12,r2,ASR #1");
        asm("SUB      r7,r1,#1");
        asm("ADD      r1,r3,r2,ASR #1");
        asm("SUB      r8,r1,#1");
        asm("L1.4996:");
        asm("BEQ      L1.5028");
        asm("RSB      lr,lr,#0");
        asm("MOV      r3,#1");
        asm("ADD      r1,sp,#8");
        asm("STMIA    r1,{r3,lr}");
        asm("RSB      r10,r10,#0");
        asm("STR      r10,[sp,#4]");
        asm("B        L1.5044");
        asm("L1.5028:");
        asm("MVN      r3,#0");
        asm("ADD      r1,sp,#8");
        asm("STMIA    r1,{r3,lr}");
        asm("STR      r10,[sp,#4]");
        asm("L1.5044:");
        asm("SUB      r2,r2,#1");
        asm("STR      r2,[sp,#0x1c]");
        asm("CMP      r2,#0");
        asm("BLE      L1.6816");
        asm("LDR      r1,[sp,#0x24]");
        asm("MOV      r1,r1,LSL #1");
        asm("STR      r1,[sp,#0x48]");
        asm("L1.5072:");
        asm("LDR      r1,[sp,#0x88]");
        asm("LDR      r2,[sp,#0x1c]");
        asm("ADD      r1,r1,r2");
        asm("STR      r1,[sp,#0x44]");
        asm("LDRB     r1,[r1,#-1]");
        asm("CMP      r1,#0");
        asm("BNE      L1.5140");
        asm("LDR      r1,[sp,#0x88]");
        asm("LDR      r2,[sp,#0x1c]");
        asm("LDRB     r1,[r1,r2]");
        asm("CMP      r1,#0");
        asm("BNE      L1.5140");
        asm("LDR      r3,[sp,#8]");
        asm("ADD      r7,r7,r3");
        asm("ADD      r8,r8,r3");
        asm("ADD      r6,r6,r3,LSL #1");
        asm("B        L1.6796");
        asm("L1.5140:");
        asm("LDR      r1,[sp,#0x54]");
        asm("MOV      r2,#0");
        asm("LDR      r1,[r1,#0xc]");
        asm("MOV      r12,r2");
        asm("SUBS     r1,r1,#2");
        asm("BMI      L1.6504");
        asm("L1.5164:");
        asm("LDRB     r3,[r7,#0]");
        asm("LDR      lr,[sp,#0xc]");
        asm("LDRB     r10,[r8],lr");
        asm("SUB      r3,r3,#0x80");
        asm("STR      r3,[sp,#0x30]");
        asm("ADD      r7,r7,lr");
        asm("MOV      lr,#0xce");
        asm("SUB      r3,r10,#0x80");
        asm("ADD      lr,lr,#0xb200");
        asm("MUL      r11,lr,r3");
        asm("LDR      lr,[sp,#0x30]");
        asm("MOV      r10,#0xff");
        asm("ADD      r10,r10,#0x2a00");
        asm("MUL      lr,r10,lr");
        asm("LDR      r10,L1.6824");
        asm("ADD      lr,r11,lr,LSL #1");
        asm("MUL      r11,r10,r3");
        asm("LDR      r3,[sp,#0x30]");
        asm("LDR      r10,L1.6828");
        asm("STR      r11,[sp,#0x2c]");
        asm("MUL      r11,r10,r3");
        asm("STR      r11,[sp,#0x30]");
        asm("LDRB     r3,[r9,r1]");
        asm("CMP      r3,#0");
        asm("LDREQ    r10,[sp,#4]");
        asm("ADDEQ    r6,r6,r10");
        asm("BEQ      L1.5880");
        asm("LDR      r3,[sp,#8]");
        asm("TST      r0,#3");
        asm("LDRB     r10,[r6,r3]");
        asm("RSB      r11,lr,r10,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("LDR      r11,[sp,#0x30]");
        asm("ADD      r3,r11,r10,LSL #16");
        asm("LDR      r11,[sp,#0x2c]");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("AND      r10,r12,#0xff");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r12,LSL #8");
        asm("STR      r11,[sp,#0x18]");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r12,r12,#0xff0000");
        asm("ADD      r11,r10,r11");
        asm("LDR      r10,[sp,#0x80]");
        asm("STR      r11,[sp,#0x14]");
        asm("ADD      r12,r12,r3");
        asm("LDR      r3,[r10,r1,LSL #2]");
        asm("LDR      r11,[sp,#0x18]");
        asm("AND      r10,r3,#0xff");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("STR      r10,[sp,#0x18]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r3,LSL #8");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r3,r3,r12");
        asm("LDR      r11,[sp,#0x14]");
        asm("LDR      r12,[sp,#0x18]");
        asm("ADD      r10,r10,r11");
        asm("LDRB     r11,[r5,r12,ASR #16]");
        asm("STR      r11,[sp,#0x18]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("LDRB     r12,[r5,r3,ASR #16]");
        asm("AND      r3,r11,#0xf0");
        asm("MOV      r11,r3,LSL #4");
        asm("AND      r3,r10,#0xf0");
        asm("ORR      r3,r3,r11");
        asm("LDR      r11,[sp,#0x18]");
        asm("AND      r10,r10,#0xe");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,LSR #1");
        asm("STR      r11,[sp,#0x18]");
        asm("MOV      r11,r10,LSR #1");
        asm("ORR      r3,r3,r12,ASR #4");
        asm("AND      r12,r12,#0xe");
        asm("STR      r11,[sp,#0x14]");
        asm("MOV      r11,r12,LSR #1");
        asm("STR      r11,[sp,#0x10]");
        asm("LDR      r11,[sp,#0x18]");
        asm("LDR      r12,[sp,#0x14]");
        asm("ORR      r12,r11,r12,LSL #8");
        asm("LDR      r11,[sp,#0x10]");
        asm("LDRB     r10,[r6,#0]");
        asm("ORR      r12,r12,r11,LSL #16");
        asm("STR      r10,[sp,#0x34]");
        asm("LDR      r10,[sp,#4]");
        asm("LDR      r11,[sp,#0x14]");
        asm("ADD      r6,r6,r10");
        asm("LDR      r10,[sp,#0x34]");
        asm("RSB      r10,lr,r10,LSL #16");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#0x14]");
        asm("LDR      r10,[sp,#0x34]");
        asm("LDR      r11,[sp,#0x30]");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("LDR      r11,[sp,#0x10]");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#0x10]");
        asm("LDR      r10,[sp,#0x34]");
        asm("LDR      r11,[sp,#0x2c]");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("LDR      r11,[sp,#0x18]");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("AND      r11,r2,#0xff");
        asm("ADD      r10,r10,r11,LSL #16");
        asm("STR      r10,[sp,#0x18]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r11,r10,r2,LSL #8");
        asm("LDR      r10,[sp,#0x14]");
        asm("AND      r2,r2,#0xff0000");
        asm("ADD      r11,r11,r10");
        asm("LDR      r10,[sp,#0x10]");
        asm("ADD      r2,r2,r10");
        asm("LDR      r10,[sp,#0x18]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("STR      r10,[sp,#0x18]");
        asm("LDRB     r11,[r5,r11,ASR #16]");
        asm("LDRB     r10,[r5,r2,ASR #16]");
        asm("STR      r10,[sp,#0x10]");
        asm("LDR      r10,[sp,#0x18]");
        asm("MOV      r2,r10,LSL #28");
        asm("MOV      r2,r2,LSR #29");
        asm("AND      r10,r11,#0xe");
        asm("ORR      r2,r2,r10,LSL #7");
        asm("LDR      r10,[sp,#0x10]");
        asm("AND      r11,r11,#0xf0");
        asm("AND      r10,r10,#0xe");
        asm("ORR      r2,r2,r10,LSL #15");
        asm("LDR      r10,[sp,#0x80]");
        asm("STR      r2,[r10,r1,LSL #2]");
        asm("LDR      r10,[sp,#0x18]");
        asm("AND      r10,r10,#0xf0");
        asm("MOV      r10,r10,LSL #4");
        asm("STR      r10,[sp,#0x18]");
        asm("ORR      r10,r11,r10");
        asm("LDR      r11,[sp,#0x10]");
        asm("ORR      r10,r10,r11,ASR #4");
        asm("LDRB     r11,[r9,r1]");
        asm("BEQ      L1.5804");
        asm("CMP      r11,#2");
        asm("BNE      L1.5752");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRH     r3,[r11,#0]");
        asm("STRH     r3,[r11,#2]");
        asm("STRH     r10,[r0,#0]");
        asm("STRH     r10,[r0,#2]");
        asm("B        L1.5880");
        asm("L1.5752:");
        asm("CMP      r11,#3");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRNEH   r3,[r11,#0]");
        asm("STRNEH   r10,[r0,#0]");
        asm("BNE      L1.5880");
        asm("STRH     r3,[r11],#2");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r11,#0]");
        asm("STRH     r10,[r0,#0]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("MOV      r10,#2");
        asm("STR      r3,[r10,r0]");
        asm("B        L1.5880");
        asm("L1.5804:");
        asm("CMP      r11,#2");
        asm("BNE      L1.5832");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r0,r4,LSL #1]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("B        L1.5880");
        asm("L1.5832:");
        asm("CMP      r11,#3");
        asm("ADDNE    r11,r0,r4,LSL #1");
        asm("STRNEH   r3,[r11,#0]");
        asm("STRNEH   r10,[r0,#0]");
        asm("BNE      L1.5880");
        asm("ORR      r11,r3,r3,LSL #16");
        asm("STR      r11,[r0,r4,LSL #1]");
        asm("ADD      r11,r0,r4,LSL #1");
        asm("STRH     r3,[r11,#4]");
        asm("ORR      r3,r10,r10,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("STRH     r10,[r0,#4]");
        asm("L1.5880:");
        asm("LDRB     r3,[r9,r1]");
        asm("ADD      r0,r0,r3,LSL #1");
        asm("ADD      r3,r9,r1");
        asm("STR      r3,[sp,#0x40]");
        asm("LDRB     r3,[r3,#1]");
        asm("CMP      r3,#0");
        asm("LDREQ    r10,[sp,#4]");
        asm("ADDEQ    r6,r6,r10");
        asm("BEQ      L1.6484");
        asm("LDR      r3,[sp,#8]");
        asm("TST      r0,#3");
        asm("LDRB     r10,[r6,r3]");
        asm("RSB      r11,lr,r10,LSL #16");
        asm("STR      r11,[sp,#0x14]");
        asm("LDR      r11,[sp,#0x30]");
        asm("ADD      r3,r11,r10,LSL #16");
        asm("LDR      r11,[sp,#0x2c]");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("AND      r10,r12,#0xff");
        asm("ADD      r11,r11,r10,LSL #16");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r12,LSL #8");
        asm("STR      r11,[sp,#0x18]");
        asm("LDR      r11,[sp,#0x14]");
        asm("AND      r12,r12,#0xff0000");
        asm("ADD      r11,r10,r11");
        asm("LDR      r10,[sp,#0x80]");
        asm("ADD      r12,r12,r3");
        asm("ADD      r3,r10,r1,LSL #2");
        asm("STR      r3,[sp,#0x3c]");
        asm("STR      r11,[sp,#0x14]");
        asm("LDR      r3,[r3,#4]");
        asm("LDR      r11,[sp,#0x18]");
        asm("AND      r10,r3,#0xff");
        asm("ADD      r10,r11,r10,LSL #16");
        asm("STR      r10,[sp,#0x18]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r3,LSL #8");
        asm("AND      r3,r3,#0xff0000");
        asm("ADD      r3,r3,r12");
        asm("LDR      r11,[sp,#0x14]");
        asm("LDR      r12,[sp,#0x18]");
        asm("ADD      r10,r10,r11");
        asm("LDRB     r11,[r5,r12,ASR #16]");
        asm("STR      r11,[sp,#0x18]");
        asm("LDRB     r10,[r5,r10,ASR #16]");
        asm("LDRB     r12,[r5,r3,ASR #16]");
        asm("AND      r3,r11,#0xf0");
        asm("AND      r11,r10,#0xf0");
        asm("ORR      r3,r11,r3,LSL #4");
        asm("LDR      r11,[sp,#0x18]");
        asm("ORR      r3,r3,r12,ASR #4");
        asm("AND      r11,r11,#0xe");
        asm("MOV      r11,r11,LSR #1");
        asm("STR      r11,[sp,#0x18]");
        asm("AND      r10,r10,#0xe");
        asm("MOV      r11,r10,LSR #1");
        asm("AND      r12,r12,#0xe");
        asm("MOV      r10,r12,LSR #1");
        asm("LDR      r12,[sp,#0x18]");
        asm("STR      r10,[sp,#0x10]");
        asm("ORR      r12,r12,r11,LSL #8");
        asm("ORR      r12,r12,r10,LSL #16");
        asm("LDRB     r10,[r6,#0]");
        asm("STR      r10,[sp,#0x34]");
        asm("LDR      r10,[sp,#4]");
        asm("ADD      r6,r6,r10");
        asm("LDR      r10,[sp,#0x34]");
        asm("RSB      lr,lr,r10,LSL #16");
        asm("ADD      lr,lr,r11,LSL #16");
        asm("STR      lr,[sp,#0x14]");
        asm("LDR      lr,[sp,#0x30]");
        asm("ADD      lr,lr,r10,LSL #16");
        asm("LDR      r10,[sp,#0x10]");
        asm("ADD      r11,lr,r10,LSL #16");
        asm("LDR      lr,[sp,#0x2c]");
        asm("LDR      r10,[sp,#0x34]");
        asm("ADD      lr,lr,r10,LSL #16");
        asm("LDR      r10,[sp,#0x18]");
        asm("ADD      lr,lr,r10,LSL #16");
        asm("AND      r10,r2,#0xff");
        asm("ADD      lr,lr,r10,LSL #16");
        asm("STR      lr,[sp,#0x18]");
        asm("LDR      lr,[sp,#0x14]");
        asm("MOV      r10,#0xff0000");
        asm("AND      r10,r10,r2,LSL #8");
        asm("ADD      r10,r10,lr");
        asm("LDR      lr,[sp,#0x18]");
        asm("AND      r2,r2,#0xff0000");
        asm("LDRB     lr,[r5,lr,ASR #16]");
        asm("ADD      r2,r2,r11");
        asm("STR      lr,[sp,#0x18]");
        asm("LDRB     lr,[r5,r10,ASR #16]");
        asm("LDRB     r10,[r5,r2,ASR #16]");
        asm("LDR      r2,[sp,#0x18]");
        asm("AND      r11,lr,#0xe");
        asm("MOV      r2,r2,LSL #28");
        asm("MOV      r2,r2,LSR #29");
        asm("ORR      r2,r2,r11,LSL #7");
        asm("AND      r11,r10,#0xe");
        asm("ORR      r2,r2,r11,LSL #15");
        asm("LDR      r11,[sp,#0x3c]");
        asm("AND      lr,lr,#0xf0");
        asm("STR      r2,[r11,#4]");
        asm("LDR      r11,[sp,#0x18]");
        asm("AND      r11,r11,#0xf0");
        asm("ORR      lr,lr,r11,LSL #4");
        asm("ORR      lr,lr,r10,ASR #4");
        asm("LDR      r10,[sp,#0x40]");
        asm("LDRB     r10,[r10,#1]");
        asm("BEQ      L1.6408");
        asm("CMP      r10,#2");
        asm("BNE      L1.6356");
        asm("ADD      r10,r0,r4,LSL #1");
        asm("STRH     r3,[r10,#0]");
        asm("STRH     r3,[r10,#2]");
        asm("STRH     lr,[r0,#0]");
        asm("STRH     lr,[r0,#2]");
        asm("B        L1.6484");
        asm("L1.6356:");
        asm("CMP      r10,#3");
        asm("ADD      r10,r0,r4,LSL #1");
        asm("STRNEH   r3,[r10,#0]");
        asm("STRNEH   lr,[r0,#0]");
        asm("BNE      L1.6484");
        asm("STRH     r3,[r10],#2");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r10,#0]");
        asm("STRH     lr,[r0,#0]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("MOV      lr,#2");
        asm("STR      r3,[lr,r0]");
        asm("B        L1.6484");
        asm("L1.6408:");
        asm("CMP      r10,#2");
        asm("BNE      L1.6436");
        asm("ORR      r3,r3,r3,LSL #16");
        asm("STR      r3,[r0,r4,LSL #1]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("B        L1.6484");
        asm("L1.6436:");
        asm("CMP      r10,#3");
        asm("ADDNE    r10,r0,r4,LSL #1");
        asm("STRNEH   r3,[r10,#0]");
        asm("STRNEH   lr,[r0,#0]");
        asm("BNE      L1.6484");
        asm("ORR      r10,r3,r3,LSL #16");
        asm("STR      r10,[r0,r4,LSL #1]");
        asm("ADD      r10,r0,r4,LSL #1");
        asm("STRH     r3,[r10,#4]");
        asm("ORR      r3,lr,lr,LSL #16");
        asm("STR      r3,[r0,#0]");
        asm("STRH     lr,[r0,#4]");
        asm("L1.6484:");
        asm("LDR      r3,[sp,#0x40]");
        asm("SUBS     r1,r1,#2");
        asm("LDRB     r3,[r3,#1]");
        asm("ADD      r0,r0,r3,LSL #1");
        asm("BPL      L1.5164");
        asm("L1.6504:");
        asm("LDR      r1,[sp,#0x28]");
        asm("LDR      r2,[sp,#0x1c]");
        asm("ADD      r6,r6,r1");
        asm("LDR      r1,[sp,#0x20]");
        asm("ADD      r7,r7,r1");
        asm("ADD      r8,r8,r1");
        asm("LDR      r1,[sp,#0x24]");
        asm("SUB      r10,r0,r1,LSL #1");
        asm("LDR      r1,[sp,#0x88]");
        asm("LDRB     r0,[r1,r2]");
        asm("LDR      r1,[sp,#0x44]");
        asm("LDRB     r1,[r1,#-1]");
        asm("MUL      r11,r4,r0");
        asm("CMP      r1,#0");
        asm("BEQ      L1.6684");
        asm("CMP      r0,#1");
        asm("BEQ      L1.6588");
        asm("ADD      r1,r10,r4,LSL #1");
        asm("ADD      r0,r10,r11,LSL #1");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("L1.6588:");
        asm("LDR      r1,[sp,#0x44]");
        asm("LDRB     r0,[r1,#-1]");
        asm("CMP      r0,#2");
        asm("BNE      L1.6628");
        asm("ADD      r0,r10,r11,LSL #1");
        asm("ADD      r0,r0,r4,LSL #1");
        asm("ADD      r1,r10,r4,LSL #1");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("B        L1.6684");
        asm("L1.6628:");
        asm("CMP      r0,#3");
        asm("BNE      L1.6684");
        asm("ADD      r0,r10,r11,LSL #1");
        asm("STR      r0,[sp,#0x38]");
        asm("ADD      r11,r10,r4,LSL #1");
        asm("MOV      r1,r11");
        asm("ADD      r0,r0,r4,LSL #1");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("LDR      r0,[sp,#0x38]");
        asm("MOV      r1,r11");
        asm("LDR      r2,[sp,#0x48]");
        asm("ADD      r0,r0,r4,LSL #2");
        asm("BL       memcpy");
        asm("L1.6684:");
        asm("LDR      r1,[sp,#0x88]");
        asm("LDR      r2,[sp,#0x1c]");
        asm("LDRB     r0,[r1,r2]");
        asm("CMP      r0,#2");
        asm("BNE      L1.6724");
        asm("ADD      r0,r10,r4,LSL #1");
        asm("MOV      r1,r10");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("B        L1.6764");
        asm("L1.6724:");
        asm("CMP      r0,#3");
        asm("BNE      L1.6764");
        asm("ADD      r0,r10,r4,LSL #1");
        asm("MOV      r1,r10");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("ADD      r0,r10,r4,LSL #2");
        asm("MOV      r1,r10");
        asm("LDR      r2,[sp,#0x48]");
        asm("BL       memcpy");
        asm("L1.6764:");
        asm("LDR      r1,[sp,#0x44]");
        asm("LDRB     r0,[r1,#-1]");
        asm("LDR      r1,[sp,#0x88]");
        asm("LDR      r2,[sp,#0x1c]");
        asm("LDRB     r1,[r1,r2]");
        asm("ADD      r0,r0,r1");
        asm("MUL      r0,r4,r0");
        asm("ADD      r0,r10,r0,LSL #1");
        asm("L1.6796:");
        asm("LDR      r2,[sp,#0x1c]");
        asm("SUB      r2,r2,#2");
        asm("STR      r2,[sp,#0x1c]");
        asm("CMP      r2,#0");
        asm("BGT      L1.5072");
        asm("L1.6816:");
        asm("ADD      sp,sp,#0x5c");
        asm("LDMFD    sp!,{r4-r11,lr}");
        asm("MOV      r0,#1");
//        asm("BX       lr");
        asm("L1.6820:");
        asm(".align		0");
        asm("L1.6824:");
        asm(".word      0x00015f03");
        asm("L1.6828:");
        asm(".word      0x0001bbd2");

    }
    return 1;
#else
    return 0;
#endif // defined(CCROTATE) && defined(CCSCALING)
}

#else // straight C version

/////////////////////////////////////////////////////////////////////////////
// Note:: This zoom algorithm needs an extra line of RGB buffer. So, users
// have to use GetRGBBufferSize API to get the size it needs. See GetRGBBufferSize().
int32 cc12sc_rotate(uint8 **src, uint8 *dst, int *disp,
                    uint8 *clip, uint8 *pErr_horz,
                    uint8 *_mRowPix, uint8 *_mColPix,
                    bool _mIsRotateClkwise)
{
#if (CCROTATE && CCSCALING)
    const static uint32 JCoeff[4] =
    {
        45774,	//65536*0.813/1.164;
        89859,	//65536*1.596/1.164;
        22014,	//65536*0.391/1.164;
        113618	//65536*2.018/1.164;
    };

    uint8 *pCb, *pCr, *pY;
    uint16	*pDst;
    int32 src_pitch, dst_pitch, src_width;
    int32 Y, Cb, Cr, Cg;
    uint32 left_err_t, left_err_b; /* error from the left pixels */
    uint32 top_err_l, top_err_r; /* error from the top pixels */
    int32 deltaY, dst_width, deltaCbCr;
    int32 row, col;
    int32 tmp0, tmp1, tmp2;
    uint32 rgb;
    int32 offset;


    src_pitch	=	disp[0];
    dst_pitch	=	disp[1];
    src_width	=	disp[2];
    dst_width	=  disp[4];

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

    pDst =	(uint16 *)dst;

    int half_src_pitch, read_idx, tmp_src_pitch;
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

        left_err_t = left_err_b = 0;  /* reset left pixel errors */

        for (col = disp[3] - 2; col >= 0; col -= 2)
        {/* decrement index, _mRowPix[.] is
													 symmetric to increment index */

            Cb = *pCb;
            pCb += half_src_pitch;
            Cr = *pCr;
            pCr += half_src_pitch;

            //Cb = *pCb++;	Cr = *pCr++;

            //load the bottom two pixels
            //Y	=	*(((uint16 *)pY)+src_pitch);
            //Y	=	*((uint16 *)(((uint16 *)pY) + src_pitch));

            Cb -= 128;
            Cr -= 128;
            Cg	=	Cr * JCoeff[0];
            Cg	+=	Cb * JCoeff[2];

            Cr	*=	JCoeff[1];
            Cb	*=	JCoeff[3];

            if (_mRowPix[col]) /* compute this pixel */
            {
                //tmp0	=	pY[src_pitch];			//bottom left

                tmp0	=	pY[read_idx];

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                //add the error from top and left
                tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
                tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
                tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

                top_err_l = *((uint*)(pErr_horz + (col << 2)));
                tmp0	+=	((top_err_l & 0xFF) << 16);//pErr_horz[col];
                tmp1	+=	(((top_err_l >> 8) & 0xFF) << 16);//mErr_horz[1][col];
                tmp2	+=	(((top_err_l >> 16) & 0xFF) << 16);//mErr_horz[2][col];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];
#ifdef	OUTPUT_RGB_565
                //RGB_565
                rgb		= (tmp0 >> 3);
                rgb		= (tmp1 >> 2) | (rgb << 6);
                rgb		= (tmp2 >> 3) | (rgb << 5);
#else
                //RGB_444
                rgb		= (tmp0 & 0xF0) << 4;
                rgb		|= (tmp1 & 0xF0);
                rgb		|= (tmp2 >> 4);
#endif
                tmp0	&=	0xE;
                tmp1	&=	0xE;
                tmp2	&=	0xE;
                tmp0	>>= 1;
                tmp1	>>= 1;
                tmp2	>>= 1;

                //save error
                left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_l also

                //Y	=	*pY++;						//upper left
                Y	=	*pY;
                pY += tmp_src_pitch;

                // error from the top already available
                tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
                tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
                tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

                //add the error from left
                tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
                tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
                tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

                //save error
                left_err_t = ((tmp0 & 0xE) >> 1) | ((tmp1 & 0xE) << 7) | ((tmp2 & 0xE) << 15); //= top_err_l also
                *((uint*)(pErr_horz + (col << 2))) = left_err_t; // =top_err_l;

#ifdef	OUTPUT_RGB_565
                //RGB_565
                tmp0		= (tmp0 >> 3);
                tmp0		= (tmp1 >> 2) | (tmp0 << 6);
                tmp0		= (tmp2 >> 3) | (tmp0 << 5);
#else
                //RGB_444
                tmp0		= (tmp0 & 0xF0) << 4;
                tmp0		|= (tmp1 & 0xF0);
                tmp0		|= (tmp2 >> 4);
#endif
                if (((uint)pDst) & 3)  //half word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst + dst_pitch + 1)	=	rgb;
                        *(pDst)	=	tmp0;
                        *(pDst + 1)	=	tmp0;
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *((uint*)(pDst + dst_pitch + 1))	= rgb | (rgb << 16);

                        *(pDst)	=	tmp0;
                        *((uint*)(pDst + 1))	= tmp0 | (tmp0 << 16);
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col] == 2)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                    }
                    else if (_mRowPix[col] == 3)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *(pDst + dst_pitch + 2)	=	rgb;

                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                        *(pDst + 2)	=	tmp0;
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
            } /*	if(_mRowPix[col]) */
            else
            {

                pY += tmp_src_pitch;
            }
            pDst	+= _mRowPix[col];

            if (_mRowPix[col+1]) /* compute this pixel */
            {
                //load the top two pixels
                //Y	=	*((uint16 *)pY)++;

                tmp0	=   pY[read_idx];
                //tmp0	=	pY[src_pitch];		//bottom right

                tmp1	=	(tmp0 << 16) - Cg;
                tmp2	=	(tmp0 << 16) + Cb;
                tmp0	=	(tmp0 << 16) + Cr;

                //add the error from top and left
                tmp0	+=	((left_err_b & 0xFF) << 16);// err_vert[0];
                tmp1	+=	(((left_err_b >> 8) & 0xFF) << 16);//err_vert[1];
                tmp2	+=	(((left_err_b >> 16) & 0xFF) << 16);//err_vert[2];

                top_err_r = *((uint*)(pErr_horz + (col << 2) + 4));
                tmp0	+=	((top_err_r & 0xFF) << 16);//pErr_horz[col+1];
                tmp1	+=	(((top_err_r >> 8) & 0xFF) << 16);//mErr_horz[1][col+1];
                tmp2	+=	(((top_err_r >> 16) & 0xFF) << 16);//mErr_horz[2][col+1];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

#ifdef	OUTPUT_RGB_565
                //RGB_565
                rgb		= (tmp0 >> 3);
                rgb		= (tmp1 >> 2) | (rgb << 6);
                rgb		= (tmp2 >> 3) | (rgb << 5);
#else
                //RGB_444
                rgb		= (tmp0 & 0xF0) << 4;
                rgb		|= (tmp1 & 0xF0);
                rgb		|= (tmp2 >> 4);
#endif

                tmp0	&=	0xE;
                tmp1	&=	0xE;
                tmp2	&=	0xE;
                tmp0	>>= 1;
                tmp1	>>= 1;
                tmp2	>>= 1;

                //save error
                left_err_b = tmp0 | (tmp1 << 8) | (tmp2 << 16); // = top_err_r also

                Y	=	*pY;
                pY += tmp_src_pitch;
                //			Y	=	*pY++;

                // error from the top already available
                tmp1	=	(Y << 16) - Cg + (tmp1 << 16);
                tmp2	=	(Y << 16) + Cb + (tmp2 << 16);
                tmp0	=	(Y << 16) + Cr + (tmp0 << 16);

                //add the error from left
                tmp0	+=	((left_err_t & 0xFF) << 16);// err_vert[3];
                tmp1	+=	(((left_err_t >> 8) & 0xFF) << 16);//err_vert[4];
                tmp2	+=	(((left_err_t >> 16) & 0xFF) << 16);//err_vert[5];

                tmp0	=	clip[tmp0>>16];
                tmp1	=	clip[tmp1>>16];
                tmp2	=	clip[tmp2>>16];

                //save error
                left_err_t = ((tmp0 & 0xE) >> 1) | ((tmp1 & 0xE) << 7) | ((tmp2 & 0xE) << 15);//= top_err_r also
                *((uint*)(pErr_horz + (col << 2) + 4)) = left_err_t;// =top_err_r;

#ifdef	OUTPUT_RGB_565
                //RGB_565
                tmp0		= (tmp0 >> 3);
                tmp0		= (tmp1 >> 2) | (tmp0 << 6);
                tmp0		= (tmp2 >> 3) | (tmp0 << 5);
#else
                //RGB_444
                tmp0		= (tmp0 & 0xF0) << 4;
                tmp0		|= (tmp1 & 0xF0);
                tmp0		|= (tmp2 >> 4);
#endif

                if (((uint)pDst) & 3)  //half word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst + dst_pitch + 1)	=	rgb;
                        *(pDst)	=	tmp0;
                        *(pDst + 1)	=	tmp0;
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *((uint*)(pDst + dst_pitch + 1))	= rgb | (rgb << 16);

                        *(pDst)	=	tmp0;
                        *((uint*)(pDst + 1))	= tmp0 | (tmp0 << 16);
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
                else //word aligned
                {
                    if (_mRowPix[col+1] == 2)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                    }
                    else if (_mRowPix[col+1] == 3)
                    {
                        *((uint*)(pDst + dst_pitch))	= rgb | (rgb << 16);
                        *(pDst + dst_pitch + 2)	=	rgb;

                        *((uint*)(pDst))	= tmp0 | (tmp0 << 16);
                        *(pDst + 2)	=	tmp0;
                    }
                    else
                    {
                        *(pDst + dst_pitch)	=	rgb;
                        *(pDst)	=	tmp0;
                    }
                }
            } /*	if(_mRowPix[col]) */
            else
            {
                pY += tmp_src_pitch;
            }
            pDst	+= _mRowPix[col+1];

        }//end of COL

        pY	+=	(deltaY);
        pCb +=	deltaCbCr;
        pCr +=	deltaCbCr;

        pDst -=	(dst_width); //goes back to the beginning of the line;

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

        //copy up
        if (_mColPix[row] == 2)
            oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
        else if (_mColPix[row] == 3)
        {
            oscl_memcpy(pDst + dst_pitch, pDst, dst_width*2);
            oscl_memcpy(pDst + dst_pitch*2, pDst, dst_width*2);
        }

        pDst	+=	dst_pitch * (_mColPix[row-1] + _mColPix[row]);
    }
    return 1;
#else
    return 0;
#endif // defined(CCROTATE) && defined(CCSCALING)
}
#endif // USE_ARM_ASM_FOR_COLORCONVERT






