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
/*! \file pvvideoblend.h
 *  \brief This file contains APIs for video blending functions.
 */


#ifndef PVVIDEOBLEND_H_INCLUDED
#define PVVIDEOBLEND_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#include "oscl_mem.h"
#endif
#ifndef CCZOOMROTATION16_H_INCLUDED
#include "cczoomrotation16.h"
#endif
#ifndef CCRGB16TOYUV420_H_INCLUDED
#include "ccrgb16toyuv420.h"
#endif
#ifndef CCYUV422TOYUV420_H_INCLUDED
#include "ccyuv422toyuv420.h"
#endif

/**
 The type of YUV format. All are separate planar Y, U, and V.
 **/
typedef enum
{
    YUV420 = 0,
    YUV422 = 1
} PVYUVFormat;


/**
 This enumeration is for the clockwise rotation angle.
 **/
typedef enum
{
    Rotation0 = 0,
    Rotation90 = 1,
    Rotation180 = 2,
    Rotation270 = 3,
} PVRotationCLKWise;

/**
	This is for the mirror effect. */
#define MirrorDisable 0
#define MirrorEnable  4

/**
 This structure contains the YUV data and related information.
 **/
typedef struct
{
    uint8 *pBuffer; 	/*!< contain the YUV data in contiguous manner */
    uint32 bufSize;		/*!< size of the allocated pBuffer, can be larger than needed. */
    int32 width;		/*!< in pixels for Y component */
    int32 height;		/*!< in lines for Y component */
    PVYUVFormat	format;	/*!< indicates the specific YUV format for pBuffer */
} PVBlendFrame;


/**
 This structure contains the bitmap data and related information. The bitmap memory is to be allocated by users.
 **/
typedef struct
{
    uint8 *pBuffer;		/*!< RGB565 data */
    uint32 bufSize;		/*!< size of the allocated pBuffer, can be larger than needed. */
    int32 width;		/*!< in pixels */
    int32 height;		/*!< in lines */
    int16 color_key;	/*!< the color key for pBuffer data, to be ignored by the blend function */
} PVBitmap;


/**
 This structure contains the dimension of a frame.
 **/
typedef struct
{
    int32 width;		/*!< width in pixels */
    int32 height;		/*!< height in lines */

} PVDimension;

/**
 This structure contains the top-left coordinate of a rectangle window.
 **/
typedef struct
{
    int32 x;		/*!< horizontal */
    int32 y;		/*!< vertical */

} PVCoordinate;


class CPVVideoBlend
{
    public:

        OSCL_IMPORT_REF static CPVVideoBlend* New();
        OSCL_IMPORT_REF ~CPVVideoBlend();

        /**
         This function performs initialization to the input blend functionality, especially, for the RGB16 to YUV
         conversion part. Look at BlendInput function for detailed operation.
         @param frameSize Pointer to the size of the camera input frame, assuming the bitmap (if present)
        	has the same frame size as the camera input frame.
         @param rotation Angle of rotation, for the input from camera.
         @return 1 for success and 0 for fail.
         **/
        OSCL_IMPORT_REF int32 InitInputBlend(PVDimension *frameSize, PVRotationCLKWise rotation);

        /**
         This function performs YUV422 to YUV420 conversion and rotation on the input from the camera
         and output to a separate YUV buffer. It performs the RGB-to-YUV420 conversion on a bitmap
         input (no scaling involved), if present, and overwrite the pixel output onto the output from the first operation
         if the pixel value is different from the colorkey.

         No additional persistent memory is allocated by the function.

         Note on YUV422 format, the data in pCameraInput->pBuffer is of following:
         Y2 Cr1 Y1 Cb1 Y4 Cr2 Y3 Cb2..... going from low to high address where Y2
         represents pixel to the right of Y1 (notice the swapping of position in the buffer).

         @param pCameraInput Pointer to the structure of the input frame from camera.
         @param pBitmap Pointer to the bitmap structure, set to NULL if not present.
         @param pToEncode Pointer to the output frame structure.
         @return 1 for success and 0 for fail.
         **/
        OSCL_IMPORT_REF int32 BlendInput(PVBlendFrame *pCameraInput, PVBitmap *pBitmap, PVBlendFrame *pToEncode);


        /**
         This function performs some initializations according to parameters for output blending function.
         The input arguments represent parameters that do not change from frame to frame.
         For clarity, we make the following association.
         Input #1 is the frame coming out from the decoder.
         Input #2 is the frame going into the encoder.

          @param srcDecodeFrm The original dimension of Input #1. The top_left element is ignored.
          @param dstDecodeFrm The output dimension after the scaling of Input #1.
          @param srcEncodeFrm The original dimension of Input #2. The top_left element is ignored.
          @param dstEncodeFrm The output dimension after the scaling of Input #2.
          @param rotateMirrorEncFrm A flag to signify whether the Input#2 is to be rotated and/or mirror, for
        					example, it can be set to Rotation90|MirrorEnable.
          @param pitch	The pitch of the bitmap memory to be input as pDisplayOutput in FrameBlendOutput API.
          @return 1 for success and 0 for fail.
        */

        OSCL_IMPORT_REF int32 InitOutputBlend(PVDimension *srcDecodeFrm,
                                              PVDimension *dstDecodeFrm,
                                              PVDimension *srcEncodeFrm,
                                              PVDimension *dstEncodeFrm,
                                              int32 rotateMirrorForEncFrm,
                                              int32 pitch);

        /**
         This function takes 3 inputs, pFromDecode, pToEncode, and pDropDownMenu and performs one or more of the
         following operations on them:
         - color conversion into RGB565
         - scaling
         - mirroring
         - copying
        according to the initialized parameters specified in InitOutputBlend.
        pDisplayOutput is an I/O parameter.

         Each input is sequentially written
         onto the display bitmap (pDisplayOutput) according to this order:
         pFromDecode, pToEncode and pDromDownMenu. If there are overlapped regions
         among them, the previous outputs will be overwritten by the later outputs.
         However, when changeOutputOrder flag is set to 1, the output order becomes
         pToEncode, pFromDecode and pDropDownMenu.


          No additional persistent memory is allocated by the function.

         When any of the input frames (pFromDecode, pToEncode or pDropDownMenu) is NULL,
         no operations are performed regarding the NULL input. pDisplayOutput is required to be present,
         otherwise there will be no operations on any of the inputs.

         @param pFromDecode Pointer to the decoded frame from the incoming call, to be color converted and scaled.
         @param pos1 The location of the output associated with pFromDecode relative to the display output memory.
         @param pToEncode Pointer to the frame to outgoing call, to be color converted, scaled, rotated, and/or mirrored.
         @param pos2 The location of the output associated with pToEncode relative to the display output memory.
         @param pDropDownMenu Pointer to the drop-down menu bitmap, to be copied, set to NULL if not present.
         @param pos3 The location of the output associated with pDropDownMenu relative to the display output memory.
         @param changeOutputOrder  A flag to specify an alternative output order (see function description).
         @param pDisplayOutput Pointer to the final bitmap output. This is also an input bitmap as the background
         (canvas) for all the blending to be done over it.
         @return 1 for success and 0 for fail.
         **/
        OSCL_IMPORT_REF int32 BlendOutput(PVBlendFrame *pFromDecode, PVCoordinate *pos1,
                                          PVBlendFrame *pToEncode, PVCoordinate *pos2,
                                          PVBitmap *pDropDownMenu, PVCoordinate *pos3,
                                          uint32 changeOutputOrder,
                                          PVBitmap *pDisplayOutput);


    private:
        CPVVideoBlend();

        CCYUV422toYUV420 *CameraInput;
        CCRGB16toYUV420  *BitmapInput;
        ColorConvert16 *DecoderOutput;
        ColorConvert16 *EncoderInput;

        int32 Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, Rotation;
        bool mInitInputBlend;
        bool mInitOutputBlend;
};

#endif // PVVIDEOBLEND_H_INCLUDED
