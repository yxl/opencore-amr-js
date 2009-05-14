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
#include "oscl_types.h"
#include "impeg4file.h"
#include "pv_gau.h"
#include "oscl_mem.h"

typedef enum
{
    MP4_PARSE_ERROR = 1,
    SCALABILITY_CHECK_FAILED,
    PRASE_PVTI_AND_GEN_SDP_FAILED,
    GENERIC_SDP_FAILED,
    PEEK_N_FAILED,
    GET_N_FAILED,
    SAMPLE_COUNT_MISMATCH,
    RANDOM_POSITION_FAILED,
    BAD_ARGUMENTS,
    GET_TRACK_DECODER_SPECIFIC_INFO_FAILED
} ERROR_CODES;


struct BSInfo
{
    uint8  *buffPtr;
    uint32 timestamp;
    uint32 frame_size;
    uint32 buff_pos;

    // constructor
    OSCL_IMPORT_REF  BSInfo()
    {
        oscl_memset(this, 0, sizeof(BSInfo));
    }

    // two copy functions
    OSCL_IMPORT_REF void copy(BSInfo& X)
    {
        this->buffPtr    = X.buffPtr;
        this->timestamp  = X.timestamp;
        this->frame_size = X.frame_size;
        this->buff_pos   = X.buff_pos;
    }

    OSCL_IMPORT_REF void copy(BSInfo* pX)
    {
        if (pX)
        {
            this->buffPtr    = pX->buffPtr;
            this->timestamp  = pX->timestamp;
            this->frame_size = pX->frame_size;
            this->buff_pos   = pX->buff_pos;
        }
    }

};

class GauObject
{

    private:
        GAU mGau;
        uint32 mCount;
        uint32 mNumSamples;
        uint8  *mpBuffer;
        BSInfo mBSInfo;

        // for getPrevSample()
        BSInfo mBSInfo_prev;
        bool bGetNew;

        //! Any GauObject object should be bundled with a track in a mp4 bitstream
        uint32 mTrackID;
        IMpeg4File *mp4Bitstream;


        //! low-level getting data function
        int32 updateGauPtr();
        /**
        *	@brief  Purposely make this function private to prevent the mis-use of this function
        *			Note this function should not be called twice consecutively
        *	@return the pointer of the sample we need, NULL for error
        */
        BSInfo *getPrevSample();

        void reset()
        {
            if (mp4Bitstream)
            {
                mp4Bitstream->resetPlayback();
                oscl_memset(&mBSInfo, 0, sizeof(BSInfo));
                oscl_memset(&mBSInfo_prev, 0, sizeof(BSInfo));
                mCount = 0;
                bGetNew = true;
                mNumSamples = 10;
            }
        }

    public:

        //! default constructor
        OSCL_IMPORT_REF GauObject(IMpeg4File *mp4In = NULL, uint32 trackID = 0);

        //! destructor
        OSCL_IMPORT_REF ~GauObject();

        OSCL_IMPORT_REF IMpeg4File *getBitstream();
        OSCL_IMPORT_REF uint32 getTrackID();

        /**
        *	@brief  Hide all the GAU operation to get the next sample from the current bitstream
        *	@return the pointer of the sample we need, NULL for error
        */
        OSCL_IMPORT_REF BSInfo *getNextSample();
        /**
        *	@brief  Hide all the GAU operation to get the sample immediately before a certain timestamp
        *			from the current bitstream
        *	@param  tsIn is the requested point refering to the sample
        *	@return the pointer of the sample we need, NULL for error
        */
        OSCL_IMPORT_REF BSInfo *getPreviousSampleAt(uint32 tsIn);
        /**
        *	@brief  Hide all the GAU operation to get the sample at or immediately after a certain timestamp
        *			from the current bitstream
        *	@param  tsIn is the requested point refering to the sample
        *	@return the pointer of the sample we need, NULL for error
        */
        OSCL_IMPORT_REF BSInfo *getNextSampleAt(uint32 tsIn = 0);
        /**
        *	@brief  Hide all the GAU operation to get the next I frame from the current bitstream, whose timestamp >= tsIn
        *	@param  tsIn is the requested point refering to the sample
        *	@param  bH263Stream is flag showing whether the input bitstream is H263 bitstream
        *	@return the pointer of the sample we need, NULL for error
        */
        OSCL_IMPORT_REF BSInfo *GauObject::getNextIFrame(uint32 tsIn = 0, bool bH263Stream = false);

};

