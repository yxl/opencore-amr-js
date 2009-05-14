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
#define IMPLEMENT__AVCLayerDescEntry

#include "avclayerentry.h"
#include "atomutils.h"
#include "atomdefs.h"

AVCLayerDescEntry::AVCLayerDescEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : VisualSampleEntry(fp, size, type)
{

    if (_success)
    {
        if (!AtomUtils::read8(fp, _layerNumber))
        {
            _success = false;
        }
        if (!AtomUtils::read8(fp, _accurateStatisticsFlag))
        {
            _success = false;
        }

        if (!AtomUtils::read16(fp, _avgBitRate))
        {
            _success = false;
        }
        if (!AtomUtils::read16(fp, _avgFrameRate))
        {
            _success = false;
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_AVC_LAYER_DESCRIPTION_BOX_FAILED;
    }
}

// Destructor
AVCLayerDescEntry::~AVCLayerDescEntry()
{
}
