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
#ifndef CCZOOMROTATION32_H_INCLUDED
#define CCZOOMROTATION32_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef CCZOOMROTATIONBASE_H_INCLUDED
#include "cczoomrotationbase.h"
#endif

/**
*	This class is for 32 bit color conversion. The APIs usage is the same as
*	ColorConvertBase. The output format for RGB is 8-8-8 bits.
*/
class ColorConvert32: public ColorConvertBase
{
    public:

        OSCL_IMPORT_REF static ColorConvertBase* NewL(void);
        OSCL_IMPORT_REF ~ColorConvert32();

        int32 Init(int32 Src_width, int32 Src_height, int32 Src_pitch, int32 Dst_width, int32 Dst_height, int32 Dst_pitch, int32 nRotation = 0);
        int32 SetYuvFullRange(bool range);
        int32 SetMode(int32 nMode);	//nMode : 0 Off, 1 On
        int32 Convert(uint8 *yuvBuf, uint8 *rgbBuf);
        int32 Convert(uint8 **yuvBuf, uint8 *rgbBuf);
        int32 GetOutputBufferSize(void);

    private:
        ColorConvert32();

        uint32 mCoefTbl32[260];
        uint8  *mClip;

        int32 get_frame32(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL);
        int32 cc32ZoomIn(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL);
        int32 cc32Rotate(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL);
        int32 cc32ZoomRotate(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL);
        int32(ColorConvert32::*mPtrYUV2RGB)(uint8 **src, uint8 *dst, DisplayProperties *disp, uint8 *COFF_TBL);
};

#endif // CCZOOMROTATION32_H_INCLUDED






