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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                            MPEG-4 Track Fragment Random Access Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef TRACKFRAGMENTRANDOMACCESSATOM_H_INCLUDED
#define TRACKFRAGMENTRANDOMACCESSATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#ifndef OSCL_INT64_UTILS_H_INCLUDED
#include "oscl_int64_utils.h"
#endif

class TFRAEntries
{
    public:
        TFRAEntries(MP4_FF_FILE *fp , uint32 version,
                    uint8 length_size_of_traf_num,
                    uint8 length_size_of_trun_num,
                    uint8 length_size_of_sample_num);

        ~TFRAEntries() {};

        uint64 _time64;
        uint64 _moof_offset64;
        uint32 _time32;
        uint32 _moof_offset32;
        ////
        uint32 _traf_number;
        uint32 _trun_number;
        uint32 _sample_number;
        uint32 _version;
        uint32 getTimeStamp()
        {
            if (_version == 1)
            {
                return Oscl_Int64_Utils::get_uint64_lower32(_time64);
            }
            else
                return _time32;
            //version check and then return 32 bit value
        }
        uint32 getTimeMoofOffset()
        {
            if (_version == 1)
            {
                return Oscl_Int64_Utils::get_uint64_lower32(_moof_offset64);
            }
            else
                return _moof_offset32;
            //version check and then return 32 bit value
        }

    private:

};

class TrackFragmentRandomAccessAtom : public FullAtom
{

    public:
        TrackFragmentRandomAccessAtom(MP4_FF_FILE *fp,
                                      uint32 size,
                                      uint32 type);

        virtual ~TrackFragmentRandomAccessAtom();

        uint32 getTrackID()
        {
            return _trackId;
        }
        uint32 _entry_count;
        Oscl_Vector<TFRAEntries*, OsclMemAllocator>* getTrackFragmentRandomAccessEntries()
        {
            if (_pTFRAEntriesVec != NULL)
                return _pTFRAEntriesVec;
            return NULL;
        }
    private:
        uint32 _trackId;
        uint32 _reserved;
        uint8 _length_size_of_traf_num;
        uint8 _length_size_of_trun_num;
        uint8 _length_size_of_sample_num;

        Oscl_Vector<TFRAEntries*, OsclMemAllocator>  *_pTFRAEntriesVec;
        uint32 _version;

        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
};

#endif
