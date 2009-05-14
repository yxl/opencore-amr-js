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
#ifndef CCYUV420SEMITOYUV420_H_INCLUDED
#define CCYUV420SEMITOYUV420_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef CCZOOMROTATIONBASE_H_INCLUDED
#include "cczoomrotationbase.h"
#endif

#ifndef COLORCONV_CONFIG_H_INCLUDED
#include " colorconv_config.h"
#endif

#define SWAP_4(x)	((x<<24) | ((x&0xFF00)<<8) | ((x>>8)&0xFF00) | (x>>24))


class CCYUV420SEMItoYUV420 : public ColorConvertBase
{

    public:

        OSCL_IMPORT_REF static ColorConvertBase* New();
        OSCL_IMPORT_REF ~CCYUV420SEMItoYUV420();

        /**
        	*	@brief The function initializes necessary lookup tables and verify the capability of the library before starting the operation.

        	*	@param Src_width specifies the width in pixel from the source to be color converted.
        	*	@param Src_height specifies the height in pixel from the source to be color converted.
        	*	@param Src_pitch is the actual memory width or stride of the source.
        	*	@param Dst_width specifies the width in pixel of the output.
        	*	@param Dst_height specifies the height in pixel of the output.
        	*	@param Dst_pitch is the stride size of the destination memory.
        	*	@param nRotation specifies whether rotation is to be applied. The value can be one of the followings
        	*	Rotation0, Rotation90,Rotation180, Rotation270
        	*	When rotation is chosen, the Dst_width and Dst_height is still relative to the source coordinate,
        	*	i.e., to rotate a QCIF image, the output width will be 144 and height will be 176.
        	*	@return  Returns 1 if success, 0 if fail, i.e.,if output dimensions are different from the input dimensions,hence no scaling.
        */

        int32 Init(int32 Src_width,
                   int32 Src_height,
                   int32 Src_pitch,
                   int32 Dst_width,
                   int32 Dst_height,
                   int32 Dst_pitch,
                   int32 nRotation = 0);

        /**
        *	@brief As opposed to the definition defined in cczoomrotationbase.h, this function
        sets the memory height of the YUV buffer which is the output instead of the input.
        */

        void SetMemHeight(int32 a_mHeight)
        {
            _mDst_mheight = a_mHeight;
        };


        /**
        	*	@brief This function specifies whether the output will use the attribute specified
        	*	in the Init(.) function or perform regular color conversion without scaling or rotation.
        	*	@param nMode When set to 0, 1-to-1 color conversion only is done. When NMode is 1,
        	*	the output is be of the size and orientation specified in Init().
        	*	@return 0 if fails (capability not supported or not initialized), 1 if success.
        */
        int32 SetMode(int32 nMode);

        /**
        	*	@brief These functions convert input YUV420SEMI into corresponding YUV420 output.
        	*	@param inyuv is a pointer to an input buffer.
        	*	@param outyuv  is a pointer to an output buffer of Y plane assuming that the U and V planes are contiguous to the Y plane.
        	*	@return This function return 1 if success, 0 if fail.
        */
        int32 Convert(uint8 *inyuv, uint8 *outyuv);

        /**
        	*	@brief These functions convert input YUV420SEMI into corresponding YUV420 output.
        	*	@param yuvBuf is an array of pointers to Y,U and V plane in increasing order.
        	*	@param outyuvBuf is a pointer to an output buffer.
        	*	@return This function return 1 if success, 0 if fail in the case of the rgbBuf
        	*	and/or yuvBuf[0] address are not word-aligned (multiple of 4).
        */
        int32 Convert(uint8 *inyuvBuf, uint8 **outyuvBuf);


        /**
        	* @brief This function gives the size of the output YUV420 buffer
        	* @return buffer size in bytes
        **/
        int32 GetOutputBufferSize(void);

        /**
        *         *       @brief This function specifies the range of the YCbCr input such that the
        *                 *       conversion to RGB is done accordingly (see ISO/IEC 14496-2:2004/FPDAM 3)..
        *                         *       @param range  a boolean, false or zero means the range of the Y is 16-235,
        *                                 *   true or one means the full range of 0-255 is used. The default range is false.
        *                                         */

        virtual int32  SetYuvFullRange(bool range);

    private:
        CCYUV420SEMItoYUV420();

        int32 _mSrc_width, _mSrc_height, _mSrc_pitch, _mDst_width, _mDst_height, _mDst_pitch;
        int32 _mRotation;
        int32 _mDst_mheight;

};

#endif // CCYUV420SEMITOYUV420_H_INCLUDED

