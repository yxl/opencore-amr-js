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
#ifndef MPEG4_ENC_H_INCLUDED
#define MPEG4_ENC_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef _MP4ENC_API_H_
#include "mp4enc_api.h"
#endif

#ifndef CCRGB24TOYUV420_H_INCLUDED
#include "ccrgb24toyuv420.h"
#endif

#ifndef CCRGB12TOYUV420_H_INCLUDED
#include "ccrgb12toyuv420.h"
#endif

#ifndef CCYUV420SEMITOYUV420_H_INCLUDED
#include "ccyuv420semitoyuv420.h"
#endif

const uint32 DEFAULT_VOL_HEADER_LENGTH = 28;
const uint32 PVMFTIMESTAMP_LESSTHAN_THRESHOLD = 0x80000000;

enum
{
    MODE_H263 = 0,
    MODE_MPEG4
};


class Mpeg4Encoder_OMX
{
    public:

        Mpeg4Encoder_OMX();

        OMX_ERRORTYPE Mp4EncInit(OMX_S32 iEncMode,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
                                 OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
                                 OMX_VIDEO_PARAM_MPEG4TYPE aEncodeMpeg4Param,
                                 OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE aErrorCorrection,
                                 OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
                                 OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
                                 OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
                                 OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
                                 OMX_VIDEO_PARAM_H263TYPE aH263Type,
                                 OMX_VIDEO_PARAM_PROFILELEVELTYPE* aProfileLevel);


        OMX_BOOL Mp4EncodeVideo(OMX_U8*    aOutBuffer,
                                OMX_U32*   aOutputLength,
                                OMX_BOOL*  aBufferOverRun,
                                OMX_U8**   aOverBufferPointer,
                                OMX_U8*    aInBuffer,
                                OMX_U32    aInBufSize,
                                OMX_TICKS  aInTimeStamp,
                                OMX_TICKS* aOutTimeStamp,
                                OMX_BOOL*  aSyncFlag);

        OMX_ERRORTYPE Mp4RequestIFrame();
        OMX_BOOL Mp4UpdateBitRate(OMX_U32 aEncodedBitRate);
        OMX_BOOL Mp4UpdateFrameRate(OMX_U32 aEncodeFramerate);

        OMX_ERRORTYPE Mp4EncDeinit();


    private:

        void CopyToYUVIn(uint8* YUV, int width, int height, int width_16, int height_16);

        /* RGB->YUV conversion */
        ColorConvertBase *ccRGBtoYUV;

        //Routine to compare two timestamps a and b
        OMX_BOOL LessThan(uint32& a, uint32& b, uint32 Threshold);

        VideoEncControls	 iEncoderControl;
        OMX_BOOL			 iInitialized;
        OMX_COLOR_FORMATTYPE iVideoFormat;

        int		iSrcWidth;
        int		iSrcHeight;
        int		iFrameOrientation;
        uint32	iSrcFrameRate;
        uint8*	iYUVIn;
        uint8*	iVideoIn;
        uint8*	iVideoOut;
        uint32	iNextModTime;
        MP4HintTrack iHintTrack;
        MP4EncodingMode	ENC_Mode;

        OMX_U8 iVolHeader[DEFAULT_VOL_HEADER_LENGTH]; /** Vol header */
        OMX_U32 iVolHeaderSize;
        OMX_BOOL iVolHeaderFlag;

        bool iSupport64bitTimestamp;

};


#endif ///#ifndef MPEG4_ENC_H_INCLUDED
