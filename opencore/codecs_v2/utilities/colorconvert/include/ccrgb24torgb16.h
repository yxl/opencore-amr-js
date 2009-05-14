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
#ifndef CCRGB24TORGB16_H_INCLUDED
#define CCRGB24TORGB16_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef CCZOOMROTATIONBASE_H_INCLUDED
#include "cczoomrotationbase.h"
#endif

/**
*	This class is for 16 bit color conversion. The APIs usage is the same as
*	ColorConvertBase. The output format for RGB is 5-6-5 bits.
*/

class CCRGB24toRGB16: public ColorConvertBase
{
    public:

        OSCL_IMPORT_REF static ColorConvertBase* NewL(void);
        OSCL_IMPORT_REF virtual ~CCRGB24toRGB16();

        int32 Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation = 0);
        int32 SetYuvFullRange(bool range);
        int32 SetMode(int32 nMode);	//nMode : 0 Off, 1 On
        int32 Convert(uint8 *srcBuf, uint8 *destBuf);
        int32 Convert(uint8 **srcBuf, uint8 *destBuf);
        int32 GetOutputBufferSize(void);

    private:
        CCRGB24toRGB16();

        int32 getframe(uint8 *src, uint8 *dst, DisplayProperties *disp);

        int32 rotate(uint8 *src, uint8 *dst, DisplayProperties *disp);
        int32 zoom(uint8 *src, uint8 *dst, DisplayProperties *disp);
        int32 zoomRotate(uint8 *src, uint8 *dst, DisplayProperties *disp);

    private:
        int32(CCRGB24toRGB16::*mPtrConv)(uint8 *src, uint8 *dst, DisplayProperties *disp);
};

#endif // CCZOOMROTATION16_H_INCLUDED
