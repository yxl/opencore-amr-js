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
/*******************************************************************************************
 Class :: CCYUV420SEMItoYUV420.cpp
------------------------------------
The YUV420SEMI is semi interleaved and output format is YUV420 planar

********************************************************************************************/
#include "colorconv_config.h"
#include "ccyuv420semitoyuv420.h"


OSCL_EXPORT_REF ColorConvertBase* CCYUV420SEMItoYUV420 :: New()
{
    CCYUV420SEMItoYUV420* self = OSCL_NEW(CCYUV420SEMItoYUV420, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCYUV420SEMItoYUV420 :: CCYUV420SEMItoYUV420()
{
}


OSCL_EXPORT_REF CCYUV420SEMItoYUV420 :: ~CCYUV420SEMItoYUV420()
{
}


int32 CCYUV420SEMItoYUV420:: Init(int32 SrcWidth, int32 SrcHeight, int32 SrcPitch, int32 DstWidth, int32 DstHeight, int32 DstPitch, int32 nRotation)
{
    /* no scaled outputs */
    if (!(nRotation&0x1))
    {
        if ((SrcWidth != DstWidth) || (SrcHeight != DstHeight))
        {
            return 0;
        }
    }
    else // with rotation
    {
        if ((SrcWidth != DstHeight) || (SrcHeight != DstWidth))
        {
            return 0;
        }
    }

    if (SrcPitch != SrcWidth) // not support source cropping
    {
        return 0;
    }
    else
    {
        _mSrc_width = SrcWidth;
        _mSrc_height = SrcHeight;
        _mSrc_pitch = SrcPitch;
        _mDst_width = DstWidth;
        _mDst_height = DstHeight;
        _mDst_pitch = DstPitch;
        _mDst_mheight = DstHeight;
        _mRotation = nRotation;

        _mInitialized = true;

        return 1;
    }

}


int32 CCYUV420SEMItoYUV420:: GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return((_mDst_pitch * _mDst_mheight*3) >> 1);
}


int32 CCYUV420SEMItoYUV420::SetMode(int32 nMode)
{
    OSCL_UNUSED_ARG(nMode);
    OSCL_ASSERT(_mInitialized == true);

    return 1;
}

int32 CCYUV420SEMItoYUV420::SetYuvFullRange(bool range)
{
    OSCL_UNUSED_ARG(range);
    OSCL_ASSERT(_mInitialized == true);

    return 1;  // has no meaning in this class. Always return 1
}

int32 CCYUV420SEMItoYUV420::Convert(uint8 *inyuvBuf, uint8 *outyuvBuf)
{
    uint8 *yuvBuf[3] = {NULL, NULL, NULL };
    int outYsize = (_mDst_pitch * _mDst_mheight);
    OSCL_ASSERT(inyuvBuf);
    OSCL_ASSERT(outyuvBuf);

    yuvBuf[0] = outyuvBuf;
    yuvBuf[1] = (outyuvBuf + outYsize);
    yuvBuf[2] = (outyuvBuf + outYsize + (outYsize >> 2));

    return (Convert(inyuvBuf, yuvBuf));
}


int32 CCYUV420SEMItoYUV420::Convert(uint8 *inyuv, uint8 **outyuv)
{

    int32 i, j;
    uint16 *outcb, *outcr;
    uint32 *outy;
    uint32 *inyuv_4;
    uint32 temp, tempU, tempV;
    int outYsize, offset;

    int lpitch = _mDst_pitch;
    int lheight = _mSrc_height;

    OSCL_ASSERT(inyuv);
    OSCL_ASSERT(outyuv);
    OSCL_ASSERT(_mInitialized == true);

    outYsize = (lpitch * _mDst_mheight);

    inyuv_4 = (uint32 *)inyuv;

    switch (_mRotation)
    {
        case 0://Rotation0

            outy = (uint32*) outyuv[0];
            outcb = (uint16*) outyuv[1];
            outcr = (uint16*) outyuv[2];

            offset = lpitch - _mSrc_width;

            /* Y copying */
            for (i = lheight; i > 0; i--)
            {
                for (j = _mSrc_width >> 2; j > 0; j--)
                {
                    temp = *inyuv_4++;
                    *outy++ = temp;
                }
                /* in case the dest pitch is larger than width */
                outy += (offset >> 2);
            }

            /* U & V copying */
            for (i = lheight >> 1; i > 0; i--)
            {
                for (j = _mSrc_width >> 2; j > 0; j--)
                {
                    temp = *inyuv_4++; /* V1U1V0U0 */
                    tempU = temp & 0xFF; // U0
                    tempU = tempU | ((temp >> 8) & 0xFF00); //U1U0

                    tempV = (temp >> 8) & 0xFF; // V0
                    tempV = tempV | ((temp >> 16) & 0xFF00); //V1V0

                    *outcb++ = tempU;
                    *outcr++ = tempV;
                }
                /* in case the dest pitch is larger than width */
                outcb += (offset >> 2);
                outcr += (offset >> 2);
            }

            break;

        case 1: // Rotation90 CW
            /* To traverse in raster scan for output and vertical scan on the input.
            ** You can write output 4 bytes at a time.
            ** However, it takes more number of variables and more code size.
            */
            uint32 temp1;
            outy = (uint32*) outyuv[0];
            outcb = (uint16*) outyuv[1];
            outcr = (uint16*) outyuv[2];

            outy  = outy + (lpitch >> 2) - 1;
            outcb = outcb + (lpitch >> 2) - 1;
            outcr = outcr + (lpitch >> 2) - 1;

            /* Y copying */
            for (i = _mSrc_width; i > 0; i--)
            {
                for (j = lheight >> 2; j > 0; j--)
                {
                    temp = (*inyuv);
                    inyuv += _mSrc_width;
                    temp = (temp << 8) | (*inyuv);
                    inyuv += _mSrc_width;
                    temp = (temp << 8) | (*inyuv);
                    inyuv += _mSrc_width;
                    temp = (temp << 8) | (*inyuv);
                    inyuv += _mSrc_width;
                    *outy = temp;
                    outy--;
                }
                inyuv = inyuv - (lheight * _mSrc_width) + 1;
                outy = outy + (lheight >> 2) + (lpitch >> 2);
                /* in case the dest pitch is larger than width */
            }

            inyuv = inyuv + (lheight * _mSrc_width) - _mSrc_width;

            /* U & V copying */
            for (i = _mSrc_width >> 1; i > 0; i--)
            {
                for (j = lheight >> 2; j > 0; j--)
                {
                    temp = *inyuv++; //U0
                    temp1 = *inyuv--; //V0
                    inyuv += _mSrc_width;
                    temp = (temp << 8) | (*inyuv); //U1U0
                    inyuv++;
                    temp1 = (temp1 << 8) | (*inyuv); //V1V0
                    inyuv--;
                    inyuv += _mSrc_width;

                    *outcb = temp;
                    *outcr = temp1;
                    outcb--;
                    outcr--;
                }
                /* in case the dest pitch is larger than width */
                inyuv = inyuv - ((lheight >> 1) * _mSrc_width) + 2;
                outcb = outcb + (lheight >> 2) + (lpitch >> 2);
                outcr = outcr + (lheight >> 2) + (lpitch >> 2);
            }
            break;

        case 2://Rotation180

            outy  = (uint32*)((uint8*)outyuv[0] + outYsize - 4);
            outcb = (uint16*)((uint8*)outyuv[1] + (outYsize >> 2) - 4);
            outcr = (uint16*)((uint8*)outyuv[2] + (outYsize >> 2) - 4);

            offset = lpitch - _mSrc_width;

            /* Y copying */
            for (i = lheight; i > 0; i--)
            {
                for (j = _mSrc_width >> 2; j > 0; j--)
                {
                    temp = *inyuv_4++;
                    *outy-- = SWAP_4(temp);
                }
                /* in case the dest pitch is larger than width */
                outy -= (offset >> 2);
            }

            /* U & V copying */
            for (i = lheight >> 1; i > 0; i--)
            {
                for (j = _mSrc_width >> 2; j > 0; j--)
                {
                    temp = *inyuv_4++; /* V1U1V0U0 */
                    tempU = temp & 0xFF; // U0
                    tempU = (tempU << 8) | ((temp >> 16) & 0xFF); //U0U1

                    tempV = (temp >> 8) & 0xFF; // V0
                    tempV = (tempV << 8) | (temp >> 24); //V0V1

                    *outcb-- = tempU;
                    *outcr-- = tempV;
                }
                /* in case the dest pitch is larger than width */
                outcb -= (offset >> 2);
                outcr -= (offset >> 2);
            }

            break;

        case 3: // Rotation270	CW
            /* To traverse in raster scan for output and vertical scan on the input.
            ** You can write output 4 bytes at a time.
            ** However, it takes more number of variables and more code size.
            */
            outy = (uint32*) outyuv[0];
            outcb = (uint16*) outyuv[1];
            outcr = (uint16*) outyuv[2];

            outy  = outy + ((lpitch >> 2) * (_mSrc_width - 1));
            outcb = outcb + ((lpitch >> 2) * ((_mSrc_width >> 1) - 1));
            outcr = outcr + ((lpitch >> 2) * ((_mSrc_width >> 1) - 1));

            /* Y copying */
            for (i = _mSrc_width; i > 0; i--)
            {
                for (j = lheight >> 2; j > 0; j--)
                {
                    temp = (*inyuv);
                    inyuv += _mSrc_width;
                    temp |= ((*inyuv) << 8);
                    inyuv += _mSrc_width;
                    temp |= ((*inyuv) << 16);
                    inyuv += _mSrc_width;
                    temp |= ((*inyuv) << 24);
                    inyuv += _mSrc_width;
                    *outy = temp;
                    outy++;
                }
                inyuv = inyuv - (lheight * _mSrc_width) + 1;
                outy = outy - ((lheight >> 2) + (lpitch >> 2));
                /* in case the dest pitch is larger than width */
            }

            inyuv = inyuv + (lheight * _mSrc_width) - _mSrc_width;

            /* U & V copying */
            for (i = _mSrc_width >> 1; i > 0; i--)
            {
                for (j = lheight >> 2; j > 0; j--)
                {
                    temp = *inyuv++; //U0
                    temp1 = *inyuv--; //V0
                    inyuv += _mSrc_width;
                    temp |= ((*inyuv) << 8); //U0U1
                    inyuv++;
                    temp1 |= ((*inyuv) << 8); //V0V1
                    inyuv--;
                    inyuv += _mSrc_width;

                    *outcb = temp;
                    *outcr = temp1;
                    outcb++;
                    outcr++;
                }
                /* in case the dest pitch is larger than width */
                inyuv = inyuv - ((lheight >> 1) * _mSrc_width) + 2;
                outcb = outcb - ((lheight >> 2) + (lpitch >> 2));
                outcr = outcr - ((lheight >> 2) + (lpitch >> 2));
            }
            break;

        default:
            break;

    }//switch

    return 1;
}





