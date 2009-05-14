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
#ifndef MPEG4_DEC_H_INCLUDED
#define MPEG4_DEC_H_INCLUDED

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef _MP4DEC_API_H_
#include "mp4dec_api.h"
#endif

#ifndef _MP4DECLIB_H_
#include "mp4dec_lib.h"
#endif

class Mpeg4Decoder_OMX
{
    public:

        Mpeg4Decoder_OMX();

        OMX_S32 InitializeVideoDecode(OMX_S32* aWidth, OMX_S32* aHeight,
                                      OMX_U8** aBuffer, OMX_S32* aSize, OMX_S32 mode);

        OMX_ERRORTYPE Mp4DecInit();

        OMX_BOOL Mp4DecodeVideo(OMX_U8* aOutBuffer, OMX_U32* aOutputLength,
                                OMX_U8** aInputBuf, OMX_U32* aInBufSize,
                                OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
                                OMX_S32* aFrameCount, OMX_BOOL aMarkerFlag, OMX_BOOL *aResizeFlag);

        OMX_ERRORTYPE Mp4DecDeinit();

        OMX_S32 GetVideoHeader(int32 aLayer, uint8 *aBuf, int32 aMaxSize);

        OMX_S32 Mpeg4InitFlag;

    private:
        MP4DecodingMode CodecMode;
        VideoDecControls VideoCtrl;

        OMX_U8* pFrame0, *pFrame1;
        OMX_S32 iDisplay_Width, iDisplay_Height;
        OMX_S32 iShortVideoHeader;

        OMX_U8 VO_START_CODE1[4];
        OMX_U8 VOSH_START_CODE1[4];
        OMX_U8 VOP_START_CODE1[4];
        OMX_U8 H263_START_CODE1[3];
};


#endif ///#ifndef MPEG4_DEC_H_INCLUDED
