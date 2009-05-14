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
/*	=====================================================================	*/
/*	File: LATMPayloadParser.h												*/
/*	Description:															*/
/*																			*/
/*		RTP packet payload parser.                                          */
/*																			*/
/*	Rev:   0.2																*/
/*	=====================================================================	*/
/*																			*/
/*	Revision History:														*/
/*																			*/
/*	Rev:																	*/
/*	Date:																	*/
/*	Description:															*/
/*																			*/
/* //////////////////////////////////////////////////////////////////////// */

#ifndef _LATMPAYLOADPARSER_H_
#define _LATMPAYLOADPARSER_H_

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif


//status return type used by PayloadParser class API, up to 16-bit map
#define FRAME_ERROR     					0x10	//error during composing
#define FRAME_INCOMPLETE					0x01	//frame incomplete
#define FRAME_COMPLETE					    0x80	//frame complete

// Don't understand the organization of the status codes so using available bit
#define FRAME_OUTPUTNOTAVAILABLE			0x20	//memory for parser output data not available



// this is a structure i need to hold information used in demultiplexing
typedef struct _streamMuxConfig
{
    bool cpresent;
    uint32 audioMuxVersion;
    uint32 allStreamsSameTimeFraming;
    uint32 numSubFrames;
    uint32 frameLengthType;  // 0 == 1024 samples, 1 == 960 (not supported)
    uint32 bufferFullness;
    bool otherDataPresent;
    uint32 otherDataLenBits;
    bool crcCheckPresent;
    uint8 crcCheckSum;

    uint32 audioObjectType;
    uint32 samplingFrequency;
    uint32 channelConfiguration;
    /*  AAC+ data */

    int32 sbrPresentFlag;
    uint32 extensionAudioObjectType;
    uint32 extensionSamplingFrequencyIndex;
    uint32 extensionSamplingFrequency;

    // include this because the payload parser may need to deal with inline
    // audioSpecificConfigs and if it is the same as the current one, we dont
    // want to reset the decoder to the same settings..
    uint8 * audioSpecificConfigPtr;
    uint32 audioSpecificConfigSize;

    uint8 ** audioSpecificConfigPtrPtr;
    uint32 * audioSpecificConfigSizePtr;

    uint32 parseResult;
}streamMuxConfig;


//////////////////////////////////AAC LATM parser
class PV_LATM_Parser
{
    public:

        OSCL_IMPORT_REF PV_LATM_Parser();
        OSCL_IMPORT_REF ~PV_LATM_Parser();

        OSCL_IMPORT_REF uint8 compose(PVMFSharedMediaDataPtr&);
        OSCL_IMPORT_REF uint8* ParseStreamMuxConfig(uint8* decoderSpecificConfig, int32 * size);

        //added to return streammuxconfig
        streamMuxConfig *GetStreamMuxConfig()
        {
            return sMC;
        }
        PVMFSharedMediaDataPtr GetOutputBuffer()
        {
            return mediaDataOut;
        }
        OSCL_IMPORT_REF uint8 compose(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit);


    private:
        uint8 composeSingleFrame(PVMFSharedMediaDataPtr&);
        uint8 composeMultipleFrame(PVMFSharedMediaDataPtr&);
        uint8 composeSingleFrame(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit);
        uint8 composeMultipleFrame(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit);


    private:
        uint32	last_timestamp;
        uint32	last_sequence_num;
        uint32  last_mbit;
        uint8*	next_frame;			//next audio data
        int8	frame;			// number of frames in this packet
        int16	offset; 		//for multiple frame in a packet
        bool	framestart; 	//start a frame yet
        int32	bufsize;		//original buffer size
        int32	framesize;	// current composing frame size

        int32   currLen;
        bool    firstBlock;
        int32   bytesRead;
        bool	myBool;
        int32   frameNum;
        int32   compositenumframes;

        //memory pool for output data
        //OsclRefCounterMemFrag memFragOut;
        PVMFSharedMediaDataPtr mediaDataOut;

        PVMFSimpleMediaBufferCombinedAlloc iMediaDataSimpleAlloc;

        // Memory pool for latm data
        OsclMemPoolFixedChunkAllocator iLATMDataMemPool;

        // Memory pool for simple media data
        OsclMemPoolFixedChunkAllocator iMediaDataMemPool;


        streamMuxConfig * sMC;
        uint8*  multiFrameBuf;
        int32     currSize;
        bool firstPacket;
        bool dropFrames;
        uint32  maxFrameSize;
        bool firstSMC;
        bool startedParsing;
        uint8* framePos;
        int32 frameCount;

        OsclErrorTrapImp* iOsclErrorTrapImp;

};

#endif // _LATMPAYLOADPARSER_H_

