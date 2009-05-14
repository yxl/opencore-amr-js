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
#ifndef __AVCConfigurationBox_H__
#define __AVCConfigurationBox_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include "avcconfigurationbox.h"

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif





class AVCConfigurationBox : public Atom
{

    public:
        class ParameterSet
        {
            public:
                ParameterSet()
                {
                    _parameterSetLength = 0;
                    _pParameterSet = NULL;
                    _success = false;
                };

                ParameterSet(uint16 length, MP4_FF_FILE *fp);

                virtual ~ParameterSet()
                {
                    PV_MP4_ARRAY_DELETE(NULL, _pParameterSet);
                }

                uint16 getParameterSetLength()
                {
                    return _parameterSetLength;
                }

                uint8* getParameterSet()
                {
                    return _pParameterSet;
                }

                bool getSuccess()
                {
                    return _success;
                }

            private:
                bool   _success;
                uint16 _parameterSetLength;
                uint8 *_pParameterSet;
        };

        AVCConfigurationBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~AVCConfigurationBox();

        uint8 getConfigurationVersion()
        {
            return _configurationVersion;
        }

        uint8 getAVCProfileIndication()
        {
            return _avcProfileIndication;
        }

        uint8 getProfileCompatibility()
        {
            return _profileCompatibility;
        }

        uint8 getAVCLevelIndication()
        {
            return _avcLevelIndication;
        }

        uint8 getNALLengthSize()
        {
            return (uint8)(_lengthSizeMinusOne + 1);
        }

        uint8 getNumSequenceParamSets()
        {
            return _numSequenceParameterSets;
        }

        uint8 getNumPictureParamSets()
        {
            return _numPictureParameterSets;
        }

        OSCL_IMPORT_REF bool getSequenceParamSet(int32 index, uint16 &length, uint8* &paramSet);


        OSCL_IMPORT_REF bool getPictureParamSet(int32 index, uint16 &length, uint8* &paramSet);

        uint32 getNumofPictureParamSet()
        {
            return _pictureParameterSetVec->size();
        }

        uint32 getNumofSequenceParamSet()
        {
            return _sequenceParameterSetVec->size();
        }

        uint32 getTotalSeqParameterSetLength()
        {
            return _totalSeqParameterSetLength;
        }

        uint32 getTotalPictureParameterSetLength()
        {
            return _totalPicutureParameterSetLength;
        }

    private:
        uint8 _configurationVersion;
        uint8 _avcProfileIndication;
        uint8 _profileCompatibility;
        uint8 _avcLevelIndication;
        uint8 _lengthSizeMinusOne;
        uint8 _numSequenceParameterSets;
        uint8 _numPictureParameterSets;
        uint8 _constraint_set0_flag;
        uint8 _constraint_set1_flag;
        uint8 _constraint_set2_flag;
        uint8 _reserved_zero_5bits;
        uint32 _totalSeqParameterSetLength;
        uint32 _totalPicutureParameterSetLength;

        Oscl_Vector<ParameterSet*, OsclMemAllocator> *_sequenceParameterSetVec;
        Oscl_Vector<ParameterSet*, OsclMemAllocator> *_pictureParameterSetVec;

};


#endif
