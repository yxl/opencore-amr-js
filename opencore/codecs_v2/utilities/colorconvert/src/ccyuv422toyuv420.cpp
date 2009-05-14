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
 Class :: CCYUV422toYUV420.cpp
------------------------------------
The YUV422 is fully interleaved and output format is YUV420 planar

********************************************************************************************/
#include "colorconv_config.h"
#include "ccyuv422toyuv420.h"


OSCL_EXPORT_REF ColorConvertBase* CCYUV422toYUV420 :: New()
{
    CCYUV422toYUV420* self = OSCL_NEW(CCYUV422toYUV420, ());
    return OSCL_STATIC_CAST(ColorConvertBase*, self);
}


CCYUV422toYUV420 :: CCYUV422toYUV420()
{
}


OSCL_EXPORT_REF CCYUV422toYUV420 :: ~CCYUV422toYUV420()
{
}


int32 CCYUV422toYUV420:: Init(int32 SrcWidth, int32 SrcHeight, int32 SrcPitch, int32 DstWidth, int32 DstHeight, int32 DstPitch, int32 nRotation)
{
    /* no scaled outputs */
    _mInitialized = false;
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

        SetMode(0); // called after init

        return 1;
    }

}


int32 CCYUV422toYUV420:: GetOutputBufferSize(void)
{
    OSCL_ASSERT(_mInitialized == true);

    return(_mDst_pitch * _mDst_mheight*3 / 2);
}


int32 CCYUV422toYUV420::SetMode(int32 nMode)
{
    OSCL_UNUSED_ARG(nMode);
    OSCL_ASSERT(_mInitialized == true);

    return 1;
}

int32 CCYUV422toYUV420::SetYuvFullRange(bool range)
{
    OSCL_UNUSED_ARG(range);
    OSCL_ASSERT(_mInitialized == true);

    return 1;  // has no meaning in this class. Always return 1
}


int32 CCYUV422toYUV420::Convert(uint8 *inyuv, uint8 *outyuv)
{

    int32 i, j;
    uint8 *outy, *outcb, *outcr;
    uint32 *inyuv_4, temp;
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

            outy = outyuv;
            outcb = outy + outYsize;
            outcr = outcb + (outYsize >> 2);

            offset = lpitch - _mSrc_width;

            for (i = lheight >> 1; i > 0; i--)
            {
                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    *outcb++ = (uint8)(temp & 0xFF);
                    *outy++ = (uint8)((temp >> 8) & 0xFF);
                    *outcr++ = (uint8)((temp >> 16) & 0xFF);
                    *outy++ = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    *outcb++ = (uint8)((temp >> 24) & 0xFF);
                    *outy++ = (uint8)((temp >> 16) & 0xFF);
                    *outcr++ = (uint8)((temp >> 8) & 0xFF);
                    *outy++ = (uint8)(temp & 0xFF);
#endif
#if ENDIAN_3
                    *outcb++ = (uint8)((temp >> 24) & 0xFF);
                    *outy++ = (uint8)(temp & 0xFF);
                    *outcr++ = (uint8)((temp >> 8) & 0xFF);
                    *outy++ = (uint8)((temp >> 16) & 0xFF);
#endif
                }
                /* in case the dest pitch is larger than width */
                outcb += (offset >> 1);
                outcr += (offset >> 1);
                outy += offset;

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    *outy++ = (uint8)((temp >> 8) & 0xFF) ;
                    *outy++ = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    *outy++ = (uint8)((temp >> 16) & 0xFF);
                    *outy++ = (uint8)(temp & 0xFF);
#endif
#if ENDIAN_3
                    *outy++ = (uint8)(temp & 0xFF);
                    *outy++ = (uint8)((temp >> 16) & 0xFF);
#endif
                }
                outy += offset;
            }
            break;

        case 1: // Rotation90
            offset = lpitch - lheight;

            outy = outyuv + outYsize;
            outcb = outy + (outYsize >> 2);
            outcr = outcb + (outYsize >> 2);

            for (i = lheight >> 1; i > 0; i--)
            {

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)(temp & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 8) & 0xFF);
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 16) & 0xFF);
                    *outy = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)((temp >> 24) & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 16) & 0xFF);
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 8) & 0xFF);
                    *outy = (uint8)(temp & 0xFF);
#endif
#if ENDIAN_3
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 24) & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)(temp & 0xFF);
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)((temp >> 8) & 0xFF);
                    *outy = (uint8)((temp >> 16) & 0xFF);
#endif
                }

                outy += (outYsize + 1);

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 8) & 0xFF);
                    *outy = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 16) & 0xFF);
                    *outy = (uint8)(temp & 0xFF);
#endif
#if ENDIAN_3
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)(temp & 0xFF);
                    *outy = (uint8)((temp >> 16) & 0xFF);
#endif
                }

                outy += (outYsize + 1);
                outcb += ((outYsize >> 2) + 1);
                outcr += ((outYsize >> 2) + 1);
            }
            break;


        case 2://Rotation180

            outy = outyuv;
            offset = lpitch - _mSrc_width;
            outy += (outYsize - 1) - offset;
            outcb = outy + (outYsize >> 2) + offset;
            outcr = outcb + (outYsize >> 2) + offset;

            for (i = lheight >> 1; i > 0; i--)
            {
                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    *outcb-- = (uint8)(temp & 0xFF);
                    *outy-- = (uint8)((temp >> 8) & 0xFF);
                    *outcr-- = (uint8)((temp >> 16) & 0xFF);
                    *outy-- = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    *outcr-- = (uint8)((temp >> 8) & 0xFF);
                    *outy-- = (uint8)((temp >> 16) & 0xFF);
                    *outy-- = (uint8)(temp & 0xFF);
                    *outcb-- = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_3
                    *outcb-- = (uint8)((temp >> 8) & 0xFF);
                    *outy-- = (uint8)(temp & 0xFF);
                    *outy-- = (uint8)((temp >> 16) & 0xFF);
                    *outcr-- = (uint8)((temp >> 24) & 0xFF);
#endif
                }
                /* in case the dest pitch is larger than width */
                outcb -= (offset >> 1);
                outcr -= (offset >> 1);
                outy -= offset;

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *inyuv_4++;
#if ENDIAN_1
                    *outy-- = (uint8)((temp >> 8) & 0xFF) ;
                    *outy-- = (uint8)((temp >> 24) & 0xFF);
#endif
#if ENDIAN_2
                    *outy-- = (uint8)((temp >> 16) & 0xFF);
                    *outy-- = (uint8)(temp & 0xFF);
#endif
#if ENDIAN_3
                    *outy-- = (uint8)(temp & 0xFF);
                    *outy-- = (uint8)((temp >> 16) & 0xFF);
#endif
                }
                outy -= offset;
            }
            break;

        case 3: // Rotation270
            offset = lpitch - lheight;

            outy = outyuv + outYsize;
            outcb = outy + (outYsize >> 2);
            outcr = outcb + (outYsize >> 2);

            inyuv_4 = (uint32 *)(inyuv + ((_mSrc_width << 1) * lheight));

            for (i = lheight >> 1; i > 0; i--)
            {

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *--inyuv_4;
#if ENDIAN_1
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)(temp & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 24) & 0xFF);
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 16) & 0xFF);
                    *outy = (uint8)((temp >> 8) & 0xFF);
#endif
#if ENDIAN_2
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)((temp >> 24) & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)(temp & 0xFF);
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 8) & 0xFF);
                    *outy = (uint8)((temp >> 16) & 0xFF);
#endif
#if ENDIAN_3
                    outcr -= (lpitch >> 1);
                    *outcr = (uint8)((temp >> 24) & 0xFF);
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 16) & 0xFF);
                    outcb -= (lpitch >> 1);
                    *outcb = (uint8)((temp >> 8) & 0xFF);
                    *outy = (uint8)(temp & 0xFF);
#endif
                }

                outy += (outYsize + 1);

                for (j = _mSrc_width >> 1; j > 0; j--)
                {
                    temp = *--inyuv_4;
#if ENDIAN_1
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 24) & 0xFF);
                    *outy = (uint8)((temp >> 8) & 0xFF);
#endif
#if ENDIAN_2
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)(temp & 0xFF);
                    *outy = (uint8)((temp >> 16) & 0xFF);  // FIXED: 26 -> 16
#endif
#if ENDIAN_3
                    outy -= (lpitch << 1);
                    outy[lheight] = (uint8)((temp >> 16) & 0xFF);
                    *outy = (uint8)(temp & 0xFF);  // FIXED: 26 -> 16
#endif
                }

                outy += (outYsize + 1);
                outcb += ((outYsize >> 2) + 1);
                outcr += ((outYsize >> 2) + 1);
            }
            break;
        default:
            break;

    }//switch



    return 1;
}


int32 CCYUV422toYUV420::Convert(uint8 **inyuvBuf, uint8 *outyuvBuf)
{
    //TBD
    OSCL_ASSERT(inyuvBuf);

    Convert(inyuvBuf[0], outyuvBuf);

    return 1;
}



