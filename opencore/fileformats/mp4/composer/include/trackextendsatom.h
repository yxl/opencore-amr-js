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
#ifndef __TrackExtendsAtom_H__
#define __TrackExtendsAtom_H__

#include "fullatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

class PVA_FF_TrackExtendsAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_TrackExtendsAtom(uint32	mediaType, int32 codecType, uint32 trackId);

        virtual	~PVA_FF_TrackExtendsAtom();

        // To set parameters according to codec
        void	setSampleParameters(uint32 index, uint32 duration, uint32 size, uint32 flags);

        uint32	getTrackId();

        void	setTrackId(uint32 trackId);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:
        uint32	_mediaType;
        int32	_codecType;

        uint32	_trackId;
        uint32	_defaultSampleDescriptionIndex;
        uint32	_defaultSampleDuration;
        uint32	_defaultSampleSize;
        uint32	_defaultSampleFlags;

        virtual void recomputeSize();

};

#endif
