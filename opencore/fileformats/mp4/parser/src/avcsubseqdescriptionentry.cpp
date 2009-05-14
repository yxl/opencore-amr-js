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

#include "avcsubseqdescriptionentry.h"
#include "atomutils.h"
#include "atomdefs.h"

AVCSubSequenceEntry::AVCSubSequenceEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    if (_success)
    {
        if (!AtomUtils::read16(fp, _subSequenceIdentifier))
        {
            _success = false;
        }
        if (!AtomUtils::read8(fp, _layerNumber))
        {
            _success = false;
        }

        if (!AtomUtils::read8(fp, _reserved))
        {
            _success = false;
        }

        _durationFlag = (uint8)(_reserved & ~0x7F);
        _avgRateFlag = (uint8)(_reserved & ~0xBF);

        if (_durationFlag)
        {
            if (!AtomUtils::read32(fp, _duration))
            {
                _success = false;
            }
        }
        if (_avgRateFlag)
        {
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
        if (!AtomUtils::read8(fp, numReferences))
        {
            _success = false;
        }

        PV_MP4_FF_NEW(fp->auditCB, DependencyInfo, (fp), dependencyinfo);

        if (dependencyinfo == NULL)
        {
            _success = false;
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_AVC_SUBSEQUENCE_DESCRIPTION_BOX_FAILED;
    }
}

DependencyInfo::DependencyInfo(MP4_FF_FILE *fp)
{
    _subSeqCount = 0;
    _layerNumber = 0;
    _subSequenceIdentifier = 0;

    if (!AtomUtils::read8(fp, _subSeqCount))
    {
        return;
    }
    if (!AtomUtils::read8(fp, _layerNumber))
    {
        return;
    }
    if (!AtomUtils::read16(fp, _subSequenceIdentifier))
    {
        return;
    }
}

// Destructor
AVCSubSequenceEntry::~AVCSubSequenceEntry()
{
    if (dependencyinfo)
        PV_MP4_FF_DELETE(NULL, DependencyInfo, dependencyinfo);
}
