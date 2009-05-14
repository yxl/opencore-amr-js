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
#ifndef __AVCConfigurationAtom_H__
#define __AVCConfigurationAtom_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"


#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#define NAL_UNIT_LENGTH 255

class PVA_FF_AVCConfigurationAtom : public PVA_FF_Atom
{

    public:
        class PVA_FF_ParameterSet
        {
            public:
                PVA_FF_ParameterSet()
                {
                    _parameterSetLength = 0;
                    _pParameterSet = NULL;
                };

                PVA_FF_ParameterSet(uint16 length, uint8* sample)
                {
                    _parameterSetLength = 0;
                    _pParameterSet = NULL;


                    if ((int16)(length) > 0)
                    {
                        _parameterSetLength = length;

                        PV_MP4_FF_ARRAY_NEW(NULL, uint8, _parameterSetLength, _pParameterSet);



                        uint8* temp = sample;
                        for (int32 i = 0;i < _parameterSetLength;i++)
                        {
                            _pParameterSet[i] = temp[i];
                        }



                    }
                }
                virtual ~PVA_FF_ParameterSet()
                {
                    //_pParameterSet = NULL;
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



            private:
                uint16 _parameterSetLength;
                uint8 *_pParameterSet;
        };


        PVA_FF_AVCConfigurationAtom(uint8 profile = 1, uint8 profileComp = 0xFF, uint8 level = 0xFF);
        virtual ~PVA_FF_AVCConfigurationAtom();
        void setSample(void* samp, uint32 size);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();
        bool checkIfPSWritten()
        {
            if (_lengthSize != 0)
                return false;
            else
                return true;

        }
    private:
        uint8* _sample;
        uint8 _configurationVersion;
        uint8 _avcProfileIndication;
        uint8 _profileCompatibility;
        uint8 _avcLevelIndication;
        uint8 _lengthSize;
        uint8 _numSequenceParameterSets;
        uint8 _numPictureParameterSets;
        uint8* _pPictureParameterSet;
        uint8* _pSequenceParameterSet;
        uint8 _reserved_zero_5bits;
        uint16 _seqParameterSetLen;
        uint16 _picParameterSetLen;
        uint32 _totalSeqParameterSetLength;
        uint32 _totalPicutureParameterSetLength;
        Oscl_Vector<PVA_FF_ParameterSet*, OsclMemAllocator> *_sequenceParameterSetVec;
        Oscl_Vector<PVA_FF_ParameterSet*, OsclMemAllocator> *_pictureParameterSetVec;



};


#endif
