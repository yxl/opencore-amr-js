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
#ifndef AVC_ENC_H_INCLUDED
#define AVC_ENC_H_INCLUDED


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef AVCENC_API_H_INCLUDED
#include "avcenc_api.h"
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

class AvcEncoder_OMX
{
    public:

        AvcEncoder_OMX();
        ~AvcEncoder_OMX();

        OMX_ERRORTYPE AvcEncInit(OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
                                 OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
                                 OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
                                 OMX_VIDEO_PARAM_AVCTYPE aEncodeAvcParam,
                                 OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
                                 OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
                                 OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
                                 OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
                                 OMX_VIDEO_PARAM_VBSMCTYPE aVbsmcType);


        AVCEnc_Status AvcEncodeVideo(OMX_U8*    aOutBuffer,
                                     OMX_U32*   aOutputLength,
                                     OMX_BOOL*  aBufferOverRun,
                                     OMX_U8**   aOverBufferPointer,
                                     OMX_U8*    aInBuffer,
                                     OMX_U32*   aInBufSize,
                                     OMX_TICKS  aInTimeStamp,
                                     OMX_TICKS* aOutTimeStamp,
                                     OMX_BOOL*  aSyncFlag);

        AVCEnc_Status AvcEncodeSendInput(OMX_U8*    aInBuffer,
                                         OMX_U32*   aInBufSize,
                                         OMX_TICKS  aInTimeStamp);


        OMX_ERRORTYPE AvcEncDeinit();

        OMX_ERRORTYPE AvcRequestIFrame();
        OMX_BOOL AvcUpdateBitRate(OMX_U32 aEncodedBitRate);
        OMX_BOOL AvcUpdateFrameRate(OMX_U32 aEncodeFramerate);
        OMX_BOOL GetSpsPpsHeaderFlag();

        /* for avc encoder lib callback functions */
        int		AVC_DPBAlloc(uint frame_size_in_mbs, uint num_buffers);
        int		AVC_FrameBind(int indx, uint8** yuv);
        void	AVC_FrameUnbind(int indx);


    private:

        void CopyToYUVIn(uint8* YUV, int width, int height, int width_16, int height_16);

        /* RGB->YUV conversion */
        ColorConvertBase *ccRGBtoYUV;

        int		iSrcWidth;
        int		iSrcHeight;
        int		iFrameOrientation;

        OMX_COLOR_FORMATTYPE 	iVideoFormat;

        /* variables needed in operation */
        AVCHandle iAvcHandle;
        AVCFrameIO iVidIn;
        uint8*	iYUVIn;
        uint8*	iVideoIn;
        uint8*	iVideoOut;
        uint32	iTimeStamp;
        OMX_BOOL	iIDR;
        int		iDispOrd;

        uint8*	iDPB;
        bool*	iFrameUsed;
        uint8** iFramePtr;
        int		iNumFrames;

        OMX_BOOL  iInitialized;
        OMX_BOOL  iSpsPpsHeaderFlag;
        OMX_BOOL  iReadyForNextFrame;


};


#endif ///#ifndef AVC_ENC_H_INCLUDED
