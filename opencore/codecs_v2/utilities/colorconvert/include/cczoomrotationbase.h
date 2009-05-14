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
#ifndef CCZOOMROTATIONBASE_H_INCLUDED
#define CCZOOMROTATIONBASE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif


#ifndef COLORCONV_CONFIG_H_INCLUDED
#include "colorconv_config.h"
#endif

/* add capability support */
#define  CCSUPPORT_ROTATION		0x1
#define  CCSUPPORT_SCALING		0x2

/* rotation orientation, values for nRotation */
#define CCROTATE_NONE            0
#define CCROTATE_CNTRCLKWISE     1
#define CCROTATE_180			 2
#define CCROTATE_CLKWISE         3
#define	CCFLIP					 4
#define CCBOTTOM_UP				 8

/**
*	Description - This is the base class of color converter classes.
*   Each of the 3 libraries needs to include this class in the project.
*
*	ASSUMPTIONS:
*	1. zoom ratio must be no greater than 3
*	2. cannot do zoom in in one dimenstion and zoom out in another
*	3. caller should take care of the aspect ratio control
*	4. if there are margins, the caller should paint the background to the desired color
*	5. all the starting address MUST be multiple of 4, which is normally the case.
*	4. the dest pitch MUST be multiple of 4
*
*/
// Definition of ColorConvertBase class

class ColorConvertBase
{
    public:

        //! Constructor
        ColorConvertBase();

        //! Destructor
        OSCL_IMPORT_REF virtual ~ColorConvertBase();

        /**
        *	@brief  This function returns a value indicate the capability of the library at run-time.
        *	@return It returns one of or a union of CCSUPPORT_ROTATION and CCSUPPORT_SCALING
        */
        virtual int16 GetCapability(void);

        /**
        *	@brief The function initializes necessary lookup tables and verify the capability of the library before starting the operation.
        *	@param Src_width specifies the width in pixel from the source to be color converted.
        *	@param Src_height specifies the height in pixel from the source to be color converted.
        *	@param Src_pitch is the actual memory width or stride of the source.
        *	@param Dst_width specifies the width in pixel of the output.
        *	@param Dst_height specifies the height in pixel of the output.
        *	@param Dst_pitch is the stride size of the destination memory.
        *	@param nRotation specifies whether rotation is to be applied. The value can be one of the followings
        *	CCROTATE_NONE (0), CCROTATE_CNTRCLKWISE (1) or CCROTATE_CLKWISE (3).
        *	When rotation is chosen, the Dst_width and Dst_height is still relative to the source coordinate,
        *	i.e., to rotate a QCIF image, the output width will be 144 and height will be 176.
        *	@return It returns 1 if success, 0 if fail, i.e., rotation or scaling not supported or
        *	any of the above parameters is an odd number.
        */

        virtual int32 Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation = 0);

        /**
        *	@brief This function specifies the height of the Y plane which may be larger
        *	than the height of the displayed region. For example, the output from PVM4Vdec
        *	always has width and height to be multiple of 16 pixels but the actual size may be not.
        *	In this case, users will set the src_width and src_height to the actual size,
        *	but set the src_pitch and SetMemHeight to the allocated Y plane size.
        *	For WMV, the Y plane output from WMV decoder has the exact dimension (no padding
        *	for multiple of 16).
        */
        virtual void SetMemHeight(int32 a_mHeight)
        {
            _mSrc_mheight = a_mHeight;
        };

        /**
        *	@brief This function specifies whether the output will use the attribute specified
        *	in the Init(.) function or perform regular color conversion without scaling or rotation.
        *	@param nMode When set to 0, 1-to-1 color conversion only is done. When NMode is 1,
        *	the output is be of the size and orientation specified in Init().
        *	@return 0 if fails (capability not supported or not initialized), 1 if success.
        */
        virtual int32 SetMode(int32 nMode) = 0;	//nMode : 0 Off, 1 On

        /**
        *	@brief These functions convert the input buffer data into the output format
        *   and write the converted data to the output buffer.
        *	@param srcBuf is a pointer to the Y plane assuming that the U and V planes are contiguous to the Y plane.
        *	@param destBuf is a pointer to an output buffer.
        *	@return This function return 1 if success, 0 if fail.in the case of the destBuf
        *	and/or srcBuf address are not word-aligned (multiple of 4).
        */

        virtual	int32 Convert(uint8 *srcBuf, uint8 *destBuf) = 0;

        /**
        *	@brief This function returns the size of output buffer that users need
        *	to allocate and pass it in as an input argument to Convert() function.
        *	Depending on the input argument to SetMode(), the buffer size can change.
        *   Furthermore, the color conversion library MAY REQUIRE BUFFER TO BE LARGER
        *	THAN what users EXPECT for extra processing space.
        *	@return	An integer specifying the size of the output buffer.
        */
        virtual int32 GetOutputBufferSize(void) = 0;

        /**
        *	@brief This function allows the destination pitch to be set at run-time,
        *	e.g., switching between zoom & nonZoom mode.
        *	@return 1.
        */
        int32 SetDstPitch(int32 iDstPitch)
        {
            _mDisp.dst_pitch = iDstPitch;
            return 1;
        };


        /**
        *	@brief This function specifies the range of the YCbCr input such that the
        *	conversion to RGB is done accordingly (see ISO/IEC 14496-2:2004/FPDAM 3)..
        *	@param range  a boolean, false or zero means the range of the Y is 16-235,
        *   true or one means the full range of 0-255 is used. The default range is false.
        */

        virtual int32  SetYuvFullRange(bool range) = 0;

    protected:

        /** Internal structure for display property. This structure contains all configuration related
        *	parameters thus unifying the interface to variations of color convert functions
        */
        typedef struct
        {
            /** @brief Pitch of the input buffer (in pixel) */
            int32 src_pitch;
            /** @brief Pitch of the output buffer (in pixel) */
            int32 dst_pitch;
            /** @brief Width of the content of the input (in pixel) */
            int32 src_width;
            /** @brief Height of the content of the input (in pixel) */
            int32 src_height;
            /** @brief Width of the desired output (in pixel) */
            int32 dst_width;
            /**	@brief Height of the desired output (in pixel) */
            int32 dst_height;
        } DisplayProperties ;
        DisplayProperties _mDisp;

        uint32 _mSrc_width, _mSrc_height, _mSrc_mheight, _mSrc_pitch, _mDst_width, _mDst_height, _mDst_pitch, _mRotation;
        uint8 *_mRowPix, *_mColPix;

        bool _mInitialized; // initialized yet?
        bool _mIsZoom;	//Is zoomable?
        int32 _mState;	//Zoom? Rotation? etc
        bool _mIsFlip;
        bool _mYuvRange;

    private:
        /**
        *	@brief This function calculates the number of repetitions for each input pixel to output
        *	pixel such that the total output size is as specified. Users have to call this function
        *	twice, one for horizontal scaling and one for vertical scaling.
        *	@param	pLinePix is a pointer to an array of number of repetition (zero-order interpolation).
        *	@param	iSrcLen is the input size.
        *	@param	iDstLen is the output size.
        */
        void StretchLine(uint8 *pLinePix, int32 iSrcLen, int32 iDstLen);

};
#endif // CCZOOMROTATIONBASE_H_INCLUDED

