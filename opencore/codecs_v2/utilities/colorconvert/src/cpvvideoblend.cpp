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
//class CPVVideoBlend.cpp
#include "pvvideoblend.h"

// Use default DLL entry point
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF CPVVideoBlend* CPVVideoBlend :: New()
{
    CPVVideoBlend *pobj = OSCL_NEW(CPVVideoBlend, ()) ;


    return pobj;
}

CPVVideoBlend :: CPVVideoBlend()
{
    int err;

    OSCL_TRY(err,
             CameraInput = (CCYUV422toYUV420*) CCYUV422toYUV420::New();
             BitmapInput = (CCRGB16toYUV420*) CCRGB16toYUV420::New();
             DecoderOutput = (ColorConvert16*) ColorConvert16::NewL();
             EncoderInput = (ColorConvert16*) ColorConvert16::NewL();
            );

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
}



OSCL_EXPORT_REF CPVVideoBlend :: ~CPVVideoBlend()
{

}


OSCL_EXPORT_REF int32 CPVVideoBlend::InitInputBlend(PVDimension *frameSize, PVRotationCLKWise rotation)
{
    uint32 width_src, height_src, pitch_src, width_dst, height_dst, pitch_dst;
    int rot = 0;

    mInitInputBlend = false;

    width_src = frameSize->width;
    height_src = frameSize->height;
    pitch_src = frameSize->width;

    switch (rotation)
    {
        case Rotation0	:
            rot = 0;
            break;
        case Rotation90	:
            rot = 3;
            break;
        case Rotation180	:
            rot = 2;
            break;
        case Rotation270	:
            rot = 1;
            break;
    }

    if (rotation&1) /* rotation 90 or 270 degree */
    {
        width_dst = height_src;
        height_dst = width_src;
        pitch_dst = height_src;
    }
    else
    {
        width_dst = width_src;
        height_dst = height_src;
        pitch_dst = pitch_src;
    }

    if (!CameraInput->Init(width_src, height_src, pitch_src, width_dst, height_dst, pitch_dst, rot))
    {
        return 0;
    }

    if (!BitmapInput->Init(width_dst, height_dst, pitch_dst, width_dst, height_dst, pitch_dst, 0))
    {
        return 0;
    }

    mInitInputBlend = true;

    return 1;
}

// Still need some optimization to save time during the init.
// If the param is the same, we shouldn't call init all the time.
OSCL_EXPORT_REF int32 CPVVideoBlend :: BlendInput(PVBlendFrame *pCameraInput, PVBitmap *pBitmap, PVBlendFrame *pToEncode)
{
    uint8 *inputRGB;
    uint8 *outYUV;
    uint8 *srcBuffer, *dstBuffer;
    PVYUVFormat format;
    uint32 width_src, height_src, pitch_src, width_dst, height_dst, pitch_dst;
    int16 colorkey;

    if (mInitInputBlend == false || (pCameraInput == NULL && pBitmap == NULL) || (pToEncode == NULL))
    {
        return 0;
    }

    /*********** For YUV422 to YUV420 conversion **********/
    if (pCameraInput != NULL)
    {
        srcBuffer = pCameraInput->pBuffer;
        width_src = pCameraInput->width;
        height_src = pCameraInput->height;
        format = pCameraInput->format;
        pitch_src = width_src;

        dstBuffer = pToEncode->pBuffer;
        Dst_width = pToEncode->width; // 1:1, so equal to source
        Dst_height = pToEncode->height; // 1:1, so equal to source
        pitch_dst = pToEncode->width; // width of the destination buffer

        if (format == YUV422)
        {
            /* check the buffer size */
            if ((pCameraInput->bufSize < pitch_src*height_src*2) ||
                    (pToEncode->bufSize < ((pitch_dst*Dst_height*3) >> 1)))
            {
                return 0;
            }

            /* perform the conversion */
            if (!CameraInput->Convert(srcBuffer, dstBuffer))
            {
                return 0;
            }

        }
        else
        {
            return 0;
        }
    }

    /********** For RGB16 to YUV420 conversion **********/
    if (pBitmap != NULL)
    {
        inputRGB = pBitmap->pBuffer;
        width_src	 = pBitmap->width;
        height_src	 = pBitmap->height;
        colorkey = pBitmap->color_key;
        pitch_src = width_src; // equal to the source width

        // to YUV420 output
        outYUV = pToEncode->pBuffer;
        width_dst = width_src; // 1:1, set it to source
        height_dst = height_src; // 1:1, set it to source
        pitch_dst = pToEncode->width;
        Dst_height = pToEncode->height; // 1:1, so equal to source

        /* check buffer size */
        if ((pBitmap->bufSize < (pitch_src*height_src*2)) ||
                (pToEncode->bufSize < ((pitch_dst*Dst_height*3) >> 1)))
        {
            return 0;
        }

        if (pCameraInput != NULL)	// use color key when camera input is present
        {
            BitmapInput->SetColorkey(colorkey);
        }

        if (!BitmapInput->Convert(inputRGB, outYUV))
        {
            return 0;
        }
    }

    return 1;
}


OSCL_EXPORT_REF int32 CPVVideoBlend :: InitOutputBlend(PVDimension *srcDecodeFrm,
        PVDimension *dstDecodeFrm,
        PVDimension *srcEncodeFrm,
        PVDimension *dstEncodeFrm,
        int32 rotateMirrorForEncFrm,
        int32 pitch)
{

    mInitOutputBlend = false;

    if (srcDecodeFrm != NULL)
    {
        Src_width = srcDecodeFrm->width;
        Src_height = srcDecodeFrm->height;
        Src_pitch = Src_width;

        Dst_width = dstDecodeFrm->width;
        Dst_height = dstDecodeFrm->height;
        Dst_pitch = pitch;

        Rotation = 0;

        if (!DecoderOutput->Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, Rotation))
        {
            return 0;
        }

        if (!DecoderOutput->SetMode(1))
        {
            return 0;
        }
    }

    if (srcEncodeFrm != NULL)
    {
        Src_width = srcEncodeFrm->width;
        Src_height = srcEncodeFrm->height;
        Src_pitch = Src_width;

        Dst_width = dstEncodeFrm->width;
        Dst_height = dstEncodeFrm->height;
        Dst_pitch = pitch;

        Rotation = rotateMirrorForEncFrm; // for clockwise rotation CC16.lib supports counter clock wise rotation
        switch (Rotation)
        {
            case 0	:
                Rotation = 0;
                break;
            case 1	:
                Rotation = 3;
                break;
            case 2	:
                Rotation = 2;
                break;
            case 3	:
                Rotation = 1;
                break;
            case 4  :
                Rotation = 4;
                break;
            case 5  :
                Rotation = 7;
                break;
            case 6  :
                Rotation = 6;
                break;
            case 7  :
                Rotation = 5;
                break;
            default	:
                break;
        }


        if (!EncoderInput->Init(Src_width, Src_height, Src_pitch, Dst_width, Dst_height, Dst_pitch, Rotation))
        {
            return 0;
        }
        if (!EncoderInput->SetMode(1))
        {
            return 0;
        }
    }

    mInitOutputBlend = true;

    return 1;
}


OSCL_EXPORT_REF int32 CPVVideoBlend :: BlendOutput(PVBlendFrame *pFromDecode, PVCoordinate *pos1,
        PVBlendFrame *pToEncode, PVCoordinate *pos2,
        PVBitmap *pDropDownMenu, PVCoordinate *pos3,
        uint32 changeOutputOrder,
        PVBitmap *pDisplayOutput)
{

    uint8 *yuvBuf[3], *yuvBufDec[3], *rgbBuf = NULL, *temprgbBuf;
    int32 srcFrameSizeEnc = 0;
    int32 srcFrameSizeDec = 0;

    if (mInitOutputBlend == false)
    {
        return 0;
    }

    rgbBuf = pDisplayOutput->pBuffer;

    if (pToEncode)
    {
        srcFrameSizeEnc = pToEncode->width * pToEncode->height;
    }

    if (pFromDecode)
    {
        srcFrameSizeDec = pFromDecode->width * pFromDecode->height;
    }

    if (changeOutputOrder)
    {
        if (pToEncode != NULL)
        {
            yuvBuf[0] = pToEncode->pBuffer;
            yuvBuf[1] = yuvBuf[0] + srcFrameSizeEnc;
            yuvBuf[2] = yuvBuf[1] + srcFrameSizeEnc / 4;
            EncoderInput->Convert(yuvBuf, rgbBuf + (pos2->y * pDisplayOutput->width + pos2->x)*2);
        }
        if (pFromDecode != NULL)
        {
            yuvBufDec[0] = pFromDecode->pBuffer;
            yuvBufDec[1] = yuvBufDec[0] + srcFrameSizeDec;
            yuvBufDec[2] = yuvBufDec[1] + srcFrameSizeDec / 4;
            DecoderOutput->Convert(yuvBufDec, rgbBuf + (pos1->y * pDisplayOutput->width + pos1->x)*2);
        }
    }
    else
    {
        if (pFromDecode != NULL)
        {
            yuvBufDec[0] = pFromDecode->pBuffer;
            yuvBufDec[1] = yuvBufDec[0] + srcFrameSizeDec;
            yuvBufDec[2] = yuvBufDec[1] + srcFrameSizeDec / 4;
            DecoderOutput->Convert(yuvBufDec, rgbBuf + (pos1->y * pDisplayOutput->width + pos1->x)*2);
        }
        if (pToEncode != NULL)
        {
            yuvBuf[0] = pToEncode->pBuffer;
            yuvBuf[1] = yuvBuf[0] + srcFrameSizeEnc;
            yuvBuf[2] = yuvBuf[1] + srcFrameSizeEnc / 4;
            EncoderInput->Convert(yuvBuf, rgbBuf + (pos2->y * pDisplayOutput->width + pos2->x)*2);
        }
    }

    if (pDropDownMenu != NULL)
    {
        temprgbBuf = pDropDownMenu->pBuffer ;
        rgbBuf += (pos3->y * pDisplayOutput->width + pos3->x) * 2;
        for (int i = 0; i < pDropDownMenu->height; i++)
        {
            for (int j = 0; j < pDropDownMenu->width*2; j++)
            {
                *rgbBuf++ = *temprgbBuf++;
            }

            rgbBuf += (Dst_pitch * 2) - (pDropDownMenu->width * 2);
        }
    }

    return 1;
}

