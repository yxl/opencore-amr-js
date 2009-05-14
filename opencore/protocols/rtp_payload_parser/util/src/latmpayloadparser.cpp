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
#include "latmpayloadparser.h"

#include "e_tmp4audioobjecttype.h"
#include "e_mp4ff_const.h"
#include "e_progconfigconst.h"
#include "e_rawbitstreamconst.h"
#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif

#define MAX_NUM_COMPOSITE_FRAMES            32
/* Mempool size of latm parser should be at least 1 more than OMX component
   mempool size, which is 10 */
#define PVLATMPARSER_MEDIADATA_POOLNUM      12
#define PVLATMPARSER_LATMDATA_CHUNKSIZE     1536*10 // 10 maximum aac frames
#define PVLATMPARSER_MEDIADATA_CHUNKSIZE    128

static uint32 BufferShowBits(uint8 *inbuf, uint32 pos1, uint32 pos2);
static uint32 BufferReadBits(uint8 *inbuf, uint32 *pos1, int32 len);


#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()


OSCL_EXPORT_REF PV_LATM_Parser::PV_LATM_Parser() :
        last_timestamp(0),
        last_mbit(0),
        offset(0),
        framestart(false),
        framesize(0),
        currLen(0),
        firstBlock(true),
        bytesRead(0),
        myBool(false),
        frameNum(0),
        compositenumframes(0),
        iMediaDataSimpleAlloc(&iLATMDataMemPool),
        iLATMDataMemPool(PVLATMPARSER_MEDIADATA_POOLNUM, PVLATMPARSER_LATMDATA_CHUNKSIZE),
        iMediaDataMemPool(PVLATMPARSER_MEDIADATA_POOLNUM, PVLATMPARSER_MEDIADATA_CHUNKSIZE),
        sMC(NULL),
        // used only for composemultipleframe, allow at least 4 AAC frames
        currSize(PVLATMPARSER_LATMDATA_CHUNKSIZE),
        firstPacket(true),
        dropFrames(false),
        firstSMC(true),
        startedParsing(false),
        framePos(NULL),
        frameCount(0)
{
    multiFrameBuf = (uint8 *) oscl_calloc(currSize, sizeof(uint8));

    // calculate a max frame size to check if the latm parser gets out of sync
    maxFrameSize = currSize; // currently allows 4 max length case AAC frames

    iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
}


OSCL_EXPORT_REF PV_LATM_Parser::~PV_LATM_Parser()
{
    if (sMC != NULL)
    {
        if (sMC->audioSpecificConfigPtr != NULL)
        {
            oscl_free(sMC->audioSpecificConfigPtr);
            sMC->audioSpecificConfigPtr = NULL;
        }
        oscl_free(sMC);
        sMC = NULL;
    }
    if (multiFrameBuf != NULL)
    {
        oscl_free(multiFrameBuf);
        multiFrameBuf = NULL;
    }

    mediaDataOut.Unbind();
}


/* ======================================================================== */
/*  Function : compose()                                                    */
/*  Purpose  : parse AAC LATM payload                                       */
/*  In/out   :                                                              */
/*  Return   :                                                              */
/*  Note     :                                                              */
/*  Modified :                                                              */
/* ======================================================================== */

OSCL_EXPORT_REF uint8 PV_LATM_Parser::compose(PVMFSharedMediaDataPtr& mediaDataIn)
{
    uint8 retVal = 0;

    OsclRefCounterMemFrag memFragIn;
    mediaDataIn->getMediaFragment(0, memFragIn);

    // Don't need the ref to iMediaData so unbind it
    mediaDataOut.Unbind();

    int errcode = 0;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, errcode, mediaDataImpl = iMediaDataSimpleAlloc.allocate((uint32)memFragIn.getMemFrag().len));
    OSCL_FIRST_CATCH_ANY(errcode, return FRAME_OUTPUTNOTAVAILABLE);

    errcode = 0;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, errcode, mediaDataOut = PVMFMediaData::createMediaData(mediaDataImpl, &iMediaDataMemPool));
    OSCL_FIRST_CATCH_ANY(errcode, return FRAME_OUTPUTNOTAVAILABLE);

    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);

    /*
     *  Latch for very first packet, sequence number is not established yet.
     */
    int32 seqNum = mediaDataIn->getSeqNum();

    if (!firstPacket)
    {
        if ((seqNum - last_sequence_num) > 1)    /* detect any gap in sequence */
        {
            // means we missed an RTP packet.
            dropFrames = true;
        }
    }
    else
    {
        firstPacket = false;
    }

    last_timestamp = mediaDataIn->getTimestamp();
    last_sequence_num = seqNum;
    last_mbit = mediaDataIn->getMarkerInfo();

    if (dropFrames)
    {
        if (mediaDataIn->getMarkerInfo())
        {
            /*
             *  try to recover packet as sequencing was broken, new packet could be valid
             *  it is possible that the received packet contains a complete audioMuxElement()
             *  so try to retrieve it.
             */

            dropFrames = false;
        }
        else
        {

            /*
             *  we are in the middle of a spread audioMuxElement(), or faulty rtp header
             *  return error
             */

            framesize = 0;
            frameNum = 0;
            bytesRead = 0;
            compositenumframes = 0;

            /*
             *  Drop frame as we are not certain if it is a valid frame
             */
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);

            firstBlock = true; // set for next call
            return FRAME_ERROR;
        }
    }


    if (sMC->numSubFrames > 0 || (sMC->cpresent == 1 && ((*(uint8*)(memFragIn.getMemFrag().ptr)) &(0x80))))
    {
        // this is a less efficient version that must be used when you know an AudioMuxElement has
        // more than one subFrame -- I also added the case where the StreamMuxConfig is inline
        // The reason for this is that the StreamMuxConfig can be possibly large and there is no
        // way to know its size without parsing it. (the problem is it can straddle an RTP boundary)
        // it is less efficient because it composes the AudioMuxElement in a separate buffer (one
        // oscl_memcpy() per rtp packet) then parses it (one oscl_memcpy() per audio frame to the output
        // buffer (newpkt->outptr)) when it gets a whole AudioMuxElement.
        // The function below does a oscl_memcpy() directly into the output buffer
        // note, composeMultipleFrame will also work for the simple case in case there is another reason
        // to have to use it..

        retVal = composeMultipleFrame(mediaDataIn);
    }
    else
    {
        // this is an efficient version that can be used when you know an AudioMuxElement has
        // only one subFrame
        retVal = composeSingleFrame(mediaDataIn);
    }

    // set this to drop frames in the future -- till we find another marker bit
    if (retVal == FRAME_ERROR)
    {
        dropFrames = true;

        framesize = 0;
        frameNum = 0;
        bytesRead = 0;
        compositenumframes = 0;

        //changed
        memFragOut.getMemFrag().len = 0;
        mediaDataOut->setMediaFragFilledLen(0, 0);

        firstBlock = true; // set for next call

    }
    return retVal;
}

OSCL_EXPORT_REF uint8 PV_LATM_Parser::compose(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit)
{
    uint8 retVal = 0;

    // Don't need the ref to iMediaData so unbind it
    mediaDataOut.Unbind();

    int errcode = 0;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, errcode, mediaDataImpl = iMediaDataSimpleAlloc.allocate(aDataLen));
    OSCL_FIRST_CATCH_ANY(errcode, return FRAME_OUTPUTNOTAVAILABLE);

    errcode = 0;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, errcode, mediaDataOut = PVMFMediaData::createMediaData(mediaDataImpl, &iMediaDataMemPool));
    OSCL_FIRST_CATCH_ANY(errcode, return FRAME_OUTPUTNOTAVAILABLE);

    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);

    /*
     *  Latch for very first packet, sequence number is not established yet.
     */

    if (!firstPacket)
    {
        if ((aSeqNum - last_sequence_num) > 1)    /* detect any gap in sequence */
        {
            // means we missed an RTP packet.
            dropFrames = true;
        }
    }
    else
    {
        firstPacket = false;
    }

    last_timestamp = aTimestamp;
    last_sequence_num = aSeqNum;
    last_mbit = aMbit;

    if (dropFrames)
    {
        if (last_mbit)
        {
            /*
             *  try to recover packet as sequencing was broken, new packet could be valid
             *  it is possible that the received packet contains a complete audioMuxElement()
             *  so try to retrieve it.
             */

            dropFrames = false;
        }
        else
        {

            /*
             *  we are in the middle of a spread audioMuxElement(), or faulty rtp header
             *  return error
             */

            framesize = 0;
            frameNum = 0;
            bytesRead = 0;
            compositenumframes = 0;

            /*
             *  Drop frame as we are not certain if it is a valid frame
             */
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);

            firstBlock = true; // set for next call
            return FRAME_ERROR;
        }
    }


    if (sMC->numSubFrames > 0 || (sMC->cpresent == 1 && ((*aData) & (0x80))))
    {
        // this is a less efficient version that must be used when you know an AudioMuxElement has
        // more than one subFrame -- I also added the case where the StreamMuxConfig is inline
        // The reason for this is that the StreamMuxConfig can be possibly large and there is no
        // way to know its size without parsing it. (the problem is it can straddle an RTP boundary)
        // it is less efficient because it composes the AudioMuxElement in a separate buffer (one
        // oscl_memcpy() per rtp packet) then parses it (one oscl_memcpy() per audio frame to the output
        // buffer (newpkt->outptr)) when it gets a whole AudioMuxElement.
        // The function below does a oscl_memcpy() directly into the output buffer
        // note, composeMultipleFrame will also work for the simple case in case there is another reason
        // to have to use it..

        retVal = composeMultipleFrame(aData, aDataLen, aTimestamp, aSeqNum, aMbit);
    }
    else
    {
        // this is an efficient version that can be used when you know an AudioMuxElement has
        // only one subFrame
        retVal = composeSingleFrame(aData, aDataLen, aTimestamp, aSeqNum, aMbit);
    }

    // set this to drop frames in the future -- till we find another marker bit
    if (retVal == FRAME_ERROR)
    {
        dropFrames = true;

        framesize = 0;
        frameNum = 0;
        bytesRead = 0;
        compositenumframes = 0;

        //changed
        memFragOut.getMemFrag().len = 0;
        mediaDataOut->setMediaFragFilledLen(0, 0);

        firstBlock = true; // set for next call

    }
    return retVal;
}
/* ======================================================================== */
/*  Function : composeSingleFrame()                                         */
/*  Purpose  : parse AAC LATM payload                                       */
/*  In/out   :                                                              */
/*  Return   :                                                              */
/*  Note     :                                                              */
/*  Modified :                                                              */
/* ======================================================================== */

uint8 PV_LATM_Parser::composeSingleFrame(PVMFSharedMediaDataPtr& mediaDataIn)
{
    int32 tmp = 0;

    //changed
    OsclRefCounterMemFrag memFragIn;
    mediaDataIn->getMediaFragment(0, memFragIn);

    // pool made for output data
    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);

    //uint8 * myData = newpkt->data;
    uint8 * myData = (uint8*)memFragIn.getMemFrag().ptr;

    /*
     *  Total Payload length, in bytes, includes
     *      length of the AudioMuxElement()
     *      AudioMuxElement()
     *      Other data (for RF3016 not supported)
     */
    int32 pktsize = memFragIn.getMemFrag().len;

    int32 m_bit = mediaDataIn->getMarkerInfo();

    /*
     *  All streams have same time framing (there is only one stream anyway)
     */
    if (firstBlock)
    {
        /*
         *  AudioMuxElement() fits in a single rtp packet or this is the first
         *  block of an AudioMuxElement() spread accross more than one rtp packet
         */


        int32 bUsed = 0;

        /*
         *      PayLoadlenghtInfo( )
         */

        do
        {
            tmp = *(myData++);      /* get payload lenght  8-bit in bytes */
            framesize += tmp;
            bUsed++;
        }
        while (tmp == 0xff);      /* 0xff is the escape sequence for values bigger than 255 */


        /*
         *      PayLoadMux( )
         */

        bytesRead = (pktsize - bUsed);

        // framesize must be equal to the bytesRead if mbit is 1
        // or greater than bytesRead if mbit is 0
        if ((m_bit && framesize != bytesRead && !sMC->otherDataPresent) ||
                (!m_bit && framesize < bytesRead && !sMC->otherDataPresent))
        {
            // to update number of bytes copied
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);
            bytesRead = 0;

            return FRAME_ERROR;
        }

        oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr, myData, bytesRead); //ptr +1 changed

        if (sMC->otherDataPresent)
        {
            ;   /* dont' care at this point, no MUX other than aac supported */
        }

    }
    else
    {
        /*
         *  We have an AudioMuxElement() spread accross more than one rtp packet
         */
        if ((m_bit && framesize != pktsize + (bytesRead - 1) && !sMC->otherDataPresent) /* last block */ ||
                (!m_bit && framesize <  pktsize + (bytesRead - 1) && !sMC->otherDataPresent) /* intermediate block */)
        {

            // to update number of bytes copied
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);

            return FRAME_ERROR;
        }

        /*
         *  Accumulate  blocks until the full frame is complete
         */
        oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr + bytesRead, myData, pktsize);
        bytesRead += pktsize;
    }


    // to update number of bytes copied
    memFragOut.getMemFrag().len = bytesRead;
    mediaDataOut->setMediaFragFilledLen(0, bytesRead);
    mediaDataOut->setSeqNum(mediaDataIn->getSeqNum());
    mediaDataOut->setTimestamp(mediaDataIn->getTimestamp());


    firstBlock = false;     /* we already processed the first block, so this should be false  */

    if (m_bit)              /* check if it is a complete packet (m bit ==1) */
    {
        firstBlock = true;  /* if m-bit is "1", then the farme fits in a block or this was the last
                               block of the frame, set for next call */
        framesize = 0;
        frameNum = 0;
        bytesRead = 0;
        compositenumframes = 0;
    }
    else
    {
        /*
         *  We have an AudioMuxElement() spread accross more than one rtp packet
         */
        compositenumframes++;

        if (compositenumframes < MAX_NUM_COMPOSITE_FRAMES)
        {
            // this is not yet a finished packet
            return FRAME_INCOMPLETE;
        }
        else
        {
            return FRAME_ERROR;
        }

    }
    return FRAME_COMPLETE;
}


// this below is to choose between a version that returns blocks of frames
// to the cadi, or buffers those frames and returns one at a time
// set to 1 for original version that returns blocks of frames
// set to 0 for new version that buffers and returns only one frame per call


/* ======================================================================== */
/*  Function : composeMulitpleFrame()                                       */
/*  Purpose  : parse AAC LATM payload                                       */
/*  In/out   :                                                              */
/*  Return   :                                                              */
/*  Note     :                                                              */
/*  Modified :                                                              */
/* ======================================================================== */
uint8 PV_LATM_Parser::composeMultipleFrame(PVMFSharedMediaDataPtr& mediaDataIn)
{

    uint32 tmp;
    uint8 * myData;
    uint32 i;


    OsclRefCounterMemFrag memFragIn;
    mediaDataIn->getMediaFragment(0, memFragIn);

    // pool made for output data
    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);

    int32 pktsize = memFragIn.getMemFrag().len;

    // make sure we have enough memory to hold the data
    if (bytesRead + pktsize > currSize)
    {
        uint8 * tempPtr = (uint8*) oscl_calloc(bytesRead + pktsize, sizeof(uint8));
        if (tempPtr == NULL)
        {
            // memory problem?
            return FRAME_ERROR;
        }
        currSize = bytesRead + pktsize;
        oscl_memcpy(tempPtr, multiFrameBuf, bytesRead);
        oscl_free(multiFrameBuf);
        multiFrameBuf = tempPtr;
    }

    oscl_memcpy(multiFrameBuf + bytesRead, memFragIn.getMemFrag().ptr, pktsize);

    bytesRead += pktsize;
    //newpkt->frame_size = bytesRead;

    // to update number of bytes copied
    memFragOut.getMemFrag().len = bytesRead;
    mediaDataOut->setMediaFragFilledLen(0, bytesRead);
    mediaDataOut->setSeqNum(mediaDataIn->getSeqNum());
    mediaDataOut->setTimestamp(mediaDataIn->getTimestamp());

    if (mediaDataIn->getMarkerInfo())
    {
        // means this is the last packet for this audioMuxElement

        myData = multiFrameBuf;

        uint32 outPtrPos = 0;
        for (i = 0;i <= sMC->numSubFrames;i++)
        {
            framesize = 0;
            do
            {
                tmp = *(myData);
                framesize += tmp;
            }
            while (*(myData++) == 0xff);

            //int32 bUsed = (framesize/255)+1; // 0-254: 1, 255-511: 2 ...
            // do a check on the last one
            if (i == sMC->numSubFrames && !sMC->otherDataPresent)
            {
                if (framesize != bytesRead - (myData - multiFrameBuf))
                {
                    // to update number of bytes copied
                    memFragOut.getMemFrag().len = 0;
                    mediaDataOut->setMediaFragFilledLen(0, 0);

                    return FRAME_INCOMPLETE;
                }
            }
            oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr + outPtrPos, myData, framesize);
            myData += framesize;
            outPtrPos += framesize;
        }



        // to update number of bytes copied
        memFragOut.getMemFrag().len = outPtrPos;
        mediaDataOut->setMediaFragFilledLen(0, outPtrPos);

        bytesRead = 0;
        framesize = 0;
        compositenumframes = 0;

    }
    else
    {
        compositenumframes++;

        if (compositenumframes < MAX_NUM_COMPOSITE_FRAMES)
        {
            return FRAME_INCOMPLETE;
        }
        else
        {
            return FRAME_ERROR;
        }

    }

    return FRAME_COMPLETE;
}

///////////////////////////////////Create a function to decode the StreamMuxConfig
// note this function should ideally also get a reference to an object that holds the values
// for the streammuxconfig... these are alse needed in the mediaInfo class (to pass to
// the parser constructor) and can be gotten here.  for now just get the audiospecificconfig
OSCL_EXPORT_REF uint8 * PV_LATM_Parser::ParseStreamMuxConfig(uint8* decoderSpecificConfig, int32 * size)
{
    uint32 SMC_SUCCESS = 0;
    uint32 SMC_INVALID_MUX_VERSION = 1;
    uint32 SMC_INVALID_NUM_PROGRAM = 2;
    uint32 SMC_INVALID_NUM_LAYER = 4;
    uint32 SMC_INVALID_OBJECT_TYPE = 8;
    uint32 SMC_USED_RESERVED_SAMPLING_FREQ = 16;

    uint32 samplingFreqTable[] =
    {
        96000, 88200, 64000, 48000, 44100,
        32000, 24000, 22050, 16000, 12000,
        11025, 8000, 7350
    };

    if (*size == 0)
    {
        // means there is nothing to parse
        return NULL;
    }


    // size should be the length of the decoderSpecificConfig.. the AudioSpecificConfing cant
    // be larger than that, so just allocate that number of bytes
    // we wont know until we've parsed it how big it is.
    OsclMemAllocator alloc;
    uint8* ASCPtr = (uint8*)(alloc.allocate(sizeof(uint8) * (*size)));
    if (ASCPtr == NULL)
    {
        // memory allocation problem?
        *size = 0;
        return NULL;
    }
    oscl_memset(ASCPtr, 0, *size);

    OsclExclusivePtrA<uint8, OsclMemAllocator> ascAutoPtr;
    ascAutoPtr.set(ASCPtr);

    //streamMuxConfig * sMC;
    sMC = (streamMuxConfig *) oscl_calloc(1, sizeof(streamMuxConfig));
    if (sMC == NULL)
    {		// unlikely: calloc failure
        return NULL;
    }


    sMC->parseResult = SMC_SUCCESS;  // set default result

    uint32 bitPos = 0;
    uint32 ASCPos = 0;

    int32 temp;
    int32 numProgram = 0;
    int32 prog, lay;
    int32 numLayer;
    int32 count;
    int32 dependsOnCoreCoder;

    // audio mux version
    sMC->audioMuxVersion = BufferReadBits(decoderSpecificConfig, &bitPos, 1);
    if (sMC->audioMuxVersion == 0)
    {
        // should not be anything other than 0!!

        // all streams same time framing
        sMC->allStreamsSameTimeFraming = BufferReadBits(decoderSpecificConfig, &bitPos, 1);

        /*
         *  numSubFrames -- how many payloadmux() are multiplexed
         */
        sMC->numSubFrames = BufferReadBits(decoderSpecificConfig, &bitPos, 6);

        /*
         *  numPrograms  -- how many programs are multiplexed
         */
        numProgram = BufferReadBits(decoderSpecificConfig, &bitPos, 4);

        if (numProgram != 0)
        {
            sMC->parseResult |= SMC_INVALID_NUM_PROGRAM;
            //numProgram = 0;
            // really should exit
            *size = 0;
            return NULL;
        }

        // loop through programs -- happens only once now
        for (prog = 0; prog <= numProgram; prog++)
        {
            // can only be one numProgram (RFC3016)
            numLayer = BufferReadBits(decoderSpecificConfig, &bitPos, 3);
            /*
             *  Number of scalable layers, only one is indicated in rfc3016
             */
            if (numLayer != 0)
            {
                sMC->parseResult |= SMC_INVALID_NUM_LAYER;
                //numLayer = 0;
                // really should exit
                *size = 0;
                return NULL;
            }

            for (lay = 0;lay <= numLayer;lay++)
            {
                //  can only be one numLayer (RFC3016)
                if (prog == 0 && lay == 0)
                {
                    /*
                     *  audioSpecificConfig
                     *
                     * it starts at byte 1's last (lsb) bit
                     * basically copy all the rest of the bytes into the ASCPtr
                     * then shift these over to be byte aligned
                     */
                    ASCPos = bitPos;

                    sMC->audioObjectType = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_OBJ_TYPE);

                    if (sMC->audioObjectType != MP4AUDIO_AAC_LC &&
                            sMC->audioObjectType != MP4AUDIO_LTP &&
                            sMC->audioObjectType != MP4AUDIO_PS &&
                            sMC->audioObjectType != MP4AUDIO_SBR)
                    {
                        sMC->parseResult |= SMC_INVALID_OBJECT_TYPE;
                        *size = 0;
                        return NULL;
                    }


                    // SamplingFrequencyIndex -- see audio spec for meanings
                    temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE_IDX);

                    if (temp == 13 || temp == 14)
                    {
                        sMC->parseResult |= SMC_USED_RESERVED_SAMPLING_FREQ;
                    }


                    if (temp <= 12)
                    {
                        sMC->samplingFrequency = samplingFreqTable[temp];
                    }

                    if (temp == 0xf)
                    {
                        // means the sampling frequency is specified directly in the next 24 bits
                        temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE);
                    }

                    // ChannelConfiguration
                    sMC->channelConfiguration = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_CHAN_CONFIG);

                    sMC->sbrPresentFlag = -1;

                    if (sMC->audioObjectType == MP4AUDIO_SBR ||
                            sMC->audioObjectType == MP4AUDIO_PS)
                    {
                        /* to disable explicit backward compatiblity check */
                        sMC->extensionAudioObjectType = sMC->audioObjectType;

                        sMC->sbrPresentFlag = 1;

                        sMC->extensionSamplingFrequencyIndex = /* extensionSamplingFrequencyIndex */
                            BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE_IDX);

                        if (sMC->extensionSamplingFrequencyIndex == 0x0f)
                        {
                            /*
                             * sampling rate not listed in Table 1.6.2,
                             * this release does not support this
                             */
                            sMC->extensionSamplingFrequency =  /* extensionSamplingFrequency */
                                BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE);
                        }


                        sMC->audioObjectType =  BufferReadBits(decoderSpecificConfig, &bitPos, LEN_OBJ_TYPE);
                    }


                    if (sMC->audioObjectType == MP4AUDIO_AAC_LC || sMC->audioObjectType == MP4AUDIO_LTP)
                    {
                        //  GASpecificConfig

                        // frameLengthFlag
                        temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_FRAME_LEN_FLAG);

                        // dependsOnCoreCoder
                        dependsOnCoreCoder = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_DEPEND_ON_CORE);


                        if (dependsOnCoreCoder == 1)
                        {
                            // means there are 14 more bits of coreCoderDelay
                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_CORE_DELAY);
                        }
                        // ExtensionFlag
                        int extensionFlag = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_EXT_FLAG);

                        if (sMC->channelConfiguration == 0)
                        {
                            // there should be a program_config_element
                            // defined in 4.4.1.1 of 3995 sp4
                            // note, since we are only parsing this to get the size of the
                            // audioSpecificConfig, we dont care about the values except to know
                            // how many loops to do in the parsing process... save these loop
                            // variables in an array
                            uint32 loopVars[6] = {0, 0, 0, 0, 0, 0};

                            // dont actually need these values, just increment bit pointer
                            bitPos += LEN_TAG; //temp = BufferReadBits(ASCPtr, &bitPos, 4); // element_instance_tag
                            bitPos += LEN_PROFILE; //temp = BufferReadBits(ASCPtr, &bitPos, 2); // object_type
                            bitPos += LEN_SAMP_IDX; //temp = BufferReadBits(ASCPtr, &bitPos, 4); // sampling frequency index
                            loopVars[0] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_ELE); // num front channel elems
                            loopVars[1] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_ELE); // num side channel elems
                            loopVars[2] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_ELE); // num back channel elems
                            loopVars[3] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_LFE); // num lfe channel elems
                            loopVars[3] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_DAT); // num assoc data elems
                            loopVars[3] = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_NUM_CCE); // num valid cc elems

                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_MIX_PRES); // mono mixdown present
                            if (temp)
                            {
                                bitPos += LEN_NUM_ELE;
                            }
                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_MIX_PRES); // stereo mixdown present
                            if (temp)
                            {
                                bitPos += LEN_NUM_ELE;
                            }
                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_MIX_PRES); // matrix mixdown present
                            if (temp)
                            {
                                bitPos += LEN_NUM_DAT;
                            }

                            bitPos += (loopVars[0] * 5);  // front channel info
                            bitPos += (loopVars[1] * 5);  // side channel info
                            bitPos += (loopVars[2] * 5);  // back channel info
                            bitPos += (loopVars[3] * 4);  // lfe channel info
                            bitPos += (loopVars[4] * 4);  // assoc data info
                            bitPos += (loopVars[5] * 5);  // valid cc info

                            // then the spec says byte_alignement() .. need to add bits to byte align
                            // divide by 8, add 1, multiply by 8.  wont work if already byte aligned
                            // check with a mod 8
                            if (bitPos % 8 != 0)
                            {
                                bitPos = ((bitPos >> 3) + 1) << 3;
                            }

                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_COMMENT_BYTES); // comment field bytes
                            bitPos += (temp << 3);

                        }

                        // this below obviously cant happen at this point, but show it for clarity's sake
                        if (sMC->audioObjectType == MP4AUDIO_AAC_SCALABLE ||
                                sMC->audioObjectType == MP4AUDIO_ER_AAC_SCALABLE)
                        {
                        }

                        if (extensionFlag)
                        {
                            if (sMC->audioObjectType == MP4AUDIO_ER_BSAC)
                            {
                                // cant ever happen here
                            }
                            if (sMC->audioObjectType == MP4AUDIO_ER_AAC_LC ||
                                    sMC->audioObjectType == 18 ||
                                    sMC->audioObjectType == MP4AUDIO_ER_AAC_LTP ||
                                    sMC->audioObjectType == MP4AUDIO_ER_AAC_SCALABLE ||
                                    sMC->audioObjectType == MP4AUDIO_ER_TWINVQ ||
                                    sMC->audioObjectType == MP4AUDIO_ER_AAC_LD)
                            {
                                // cant ever happen here
                            }
                            // extensionFlag3 -- theoretically possible -- but should only see in future, if ever
                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, 1);
                            if (temp)
                            {
                                // tbd in version 3
                            }
                        }
                    }
                    else
                    {
                        sMC->parseResult |= SMC_INVALID_OBJECT_TYPE;
                        *size = 0;
                        return NULL;
                    }



                    /*
                     *  SBR tool explicit signaling ( backward compatible )
                     */

                    if (sMC->extensionAudioObjectType != MP4AUDIO_SBR)
                    {
                        int32 syncExtensionType = /* syncExtensionType */
                            BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SYNC_EXTENSION_TYPE);

                        if (syncExtensionType == 0x2b7)
                        {
                            sMC->extensionAudioObjectType = /* extensionAudioObjectType */
                                BufferReadBits(decoderSpecificConfig, &bitPos, LEN_OBJ_TYPE);

                            if (sMC->extensionAudioObjectType == MP4AUDIO_SBR)
                            {
                                /* sbrPresentFlag */
                                sMC->sbrPresentFlag = BufferReadBits(decoderSpecificConfig, &bitPos, 1);

                                if (sMC->sbrPresentFlag == 1)
                                {
                                    sMC->extensionSamplingFrequencyIndex =  /* extensionSamplingFrequencyIndex */
                                        BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE_IDX);

                                    if (sMC->extensionSamplingFrequencyIndex == 0x0f)
                                    {
                                        /*
                                         * sampling rate not listed in Table 1.6.2,
                                         * this release does not support this
                                         */
                                        int32 sampling_rate = 0;
                                        sampling_rate = BufferReadBits(decoderSpecificConfig, &bitPos, LEN_SAMP_RATE);

                                    }
                                }
                            }
                        }
                        else
                        {
                            /*
                             * Rewind bitstream pointer so that the syncExtensionType reading has no
                             * effect when decoding raw bitstream
                             */
                            bitPos -= LEN_SYNC_EXTENSION_TYPE;

                        }
                    }

                    // this is the end of the audioSpecificConfig



                    /*
                     *  Byte Align
                     */
                    int32 ASCLen = bitPos - ASCPos;  // length in bits -- will need to be byte aligned
                    // change length in bytes
                    if (ASCLen % 8 == 0)
                    {
                        ASCLen >>= 3;
                    }
                    else
                    {
                        ASCLen = (ASCLen >> 3) + 1;
                    }
                    int ASCOffset = ASCPos % 8; // use for masking -- do i need this?

                    if (ASCOffset == 0)
                    {
                        oscl_memcpy(ASCPtr, decoderSpecificConfig + ASCPos, ASCLen);
                    }
                    else
                    {
                        for (count = 0;count < ASCLen;count++)
                        {
                            ASCPtr[count] = (uint8)BufferReadBits(decoderSpecificConfig, &ASCPos, 8);
                        }
                    }
                    *size = ASCLen;


                    // go back to using decoderSpecificInfo
                    sMC->frameLengthType = BufferReadBits(decoderSpecificConfig, &bitPos, 3);
                    if (sMC->frameLengthType == 0)
                    {
                        sMC->bufferFullness = BufferReadBits(decoderSpecificConfig, &bitPos, 8);
                    }

                    sMC->otherDataPresent = BufferReadBits(decoderSpecificConfig, &bitPos, 1) ? 1 : 0;
                    if (sMC->otherDataPresent)
                    {
                        do
                        {
                            temp = BufferReadBits(decoderSpecificConfig, &bitPos, 1); // get escape flag
                            bitPos += 8; // skip over data itself
                        }
                        while (temp == 1);
                    }

                    sMC->crcCheckPresent = BufferReadBits(decoderSpecificConfig, &bitPos, 1) ? 1 : 0;
                    if (sMC->crcCheckPresent)
                    {
                        bitPos += 8;
                    }

                    // that should be the end of the stream mux config
                }
            }
        }

    }
    else
    {
        // spec says tbd
        sMC->parseResult |= SMC_INVALID_MUX_VERSION;
        *size = 0;
        return NULL;
    }

    ascAutoPtr.release();
    return ASCPtr;
}
uint8 PV_LATM_Parser::composeSingleFrame(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit)
{
    int32 tmp = 0;

    // pool made for output data
    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);

    //uint8 * myData = newpkt->data;
    uint8 * myData = aData;

    /*
     *  Total Payload length, in bytes, includes
     *      length of the AudioMuxElement()
     *      AudioMuxElement()
     *      Other data (for RF3016 not supported)
     */
    int32 pktsize = aDataLen;

    int32 m_bit = aMbit;

    /*
     *  All streams have same time framing (there is only one stream anyway)
     */
    if (firstBlock)
    {
        /*
         *  AudioMuxElement() fits in a single rtp packet or this is the first
         *  block of an AudioMuxElement() spread accross more than one rtp packet
         */


        int32 bUsed = 0;

        /*
         *      PayLoadlenghtInfo( )
         */

        do
        {
            tmp = *(myData++);      /* get payload lenght  8-bit in bytes */
            framesize += tmp;
            bUsed++;
        }
        while (tmp == 0xff);      /* 0xff is the escape sequence for values bigger than 255 */


        /*
         *      PayLoadMux( )
         */

        bytesRead = (pktsize - bUsed);

        // framesize must be equal to the bytesRead if mbit is 1
        // or greater than bytesRead if mbit is 0
        if ((m_bit && framesize != bytesRead && !sMC->otherDataPresent) ||
                (!m_bit && framesize < bytesRead && !sMC->otherDataPresent))
        {
            // to update number of bytes copied
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);
            bytesRead = 0;

            return FRAME_ERROR;
        }

        oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr, myData, bytesRead); //ptr +1 changed

        if (sMC->otherDataPresent)
        {
            ;   /* dont' care at this point, no MUX other than aac supported */
        }

    }
    else
    {
        /*
         *  We have an AudioMuxElement() spread accross more than one rtp packet
         */
        if ((m_bit && framesize != pktsize + (bytesRead - 1) && !sMC->otherDataPresent) /* last block */ ||
                (!m_bit && framesize <  pktsize + (bytesRead - 1) && !sMC->otherDataPresent) /* intermediate block */)
        {

            // to update number of bytes copied
            memFragOut.getMemFrag().len = 0;
            mediaDataOut->setMediaFragFilledLen(0, 0);

            return FRAME_ERROR;
        }

        /*
         *  Accumulate  blocks until the full frame is complete
         */
        oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr + bytesRead, myData, pktsize);
        bytesRead += pktsize;
    }


    // to update number of bytes copied
    memFragOut.getMemFrag().len = bytesRead;
    mediaDataOut->setMediaFragFilledLen(0, bytesRead);
    mediaDataOut->setSeqNum(aSeqNum);
    mediaDataOut->setTimestamp(aTimestamp);


    firstBlock = false;     /* we already processed the first block, so this should be false  */

    if (m_bit)              /* check if it is a complete packet (m bit ==1) */
    {
        firstBlock = true;  /* if m-bit is "1", then the farme fits in a block or this was the last
                               block of the frame, set for next call */
        framesize = 0;
        frameNum = 0;
        bytesRead = 0;
        compositenumframes = 0;
    }
    else
    {
        /*
         *  We have an AudioMuxElement() spread accross more than one rtp packet
         */
        compositenumframes++;

        if (compositenumframes < MAX_NUM_COMPOSITE_FRAMES)
        {
            // this is not yet a finished packet
            return FRAME_INCOMPLETE;
        }
        else
        {
            return FRAME_ERROR;
        }

    }
    return FRAME_COMPLETE;
}



uint8 PV_LATM_Parser::composeMultipleFrame(uint8* aData, uint32 aDataLen, uint32 aTimestamp, uint32 aSeqNum, uint32 aMbit)
{

    uint32 tmp;
    uint8 * myData;
    uint32 i;

    int32 pktsize = aDataLen;
    // pool made for output data
    OsclRefCounterMemFrag memFragOut;
    mediaDataOut->getMediaFragment(0, memFragOut);
    // make sure we have enough memory to hold the data
    if (bytesRead + pktsize > currSize)
    {
        uint8 * tempPtr = (uint8*) oscl_calloc(bytesRead + pktsize, sizeof(uint8));
        if (tempPtr == NULL)
        {
            // memory problem?
            return FRAME_ERROR;
        }
        currSize = bytesRead + pktsize;
        oscl_memcpy(tempPtr, multiFrameBuf, bytesRead);
        oscl_free(multiFrameBuf);
        multiFrameBuf = tempPtr;
    }

    oscl_memcpy(multiFrameBuf + bytesRead, aData, pktsize);

    bytesRead += pktsize;
    //newpkt->frame_size = bytesRead;

    // to update number of bytes copied
    memFragOut.getMemFrag().len = bytesRead;
    mediaDataOut->setMediaFragFilledLen(0, bytesRead);
    mediaDataOut->setSeqNum(aSeqNum);
    mediaDataOut->setTimestamp(aTimestamp);

    if (aMbit)
    {
        // means this is the last packet for this audioMuxElement

        myData = multiFrameBuf;

        uint32 outPtrPos = 0;
        for (i = 0;i <= sMC->numSubFrames;i++)
        {
            framesize = 0;
            do
            {
                tmp = *(myData);
                framesize += tmp;
            }
            while (*(myData++) == 0xff);

            //int32 bUsed = (framesize/255)+1; // 0-254: 1, 255-511: 2 ...
            // do a check on the last one
            if (i == sMC->numSubFrames && !sMC->otherDataPresent)
            {
                if (framesize != bytesRead - (myData - multiFrameBuf))
                {
                    // to update number of bytes copied
                    memFragOut.getMemFrag().len = 0;
                    mediaDataOut->setMediaFragFilledLen(0, 0);

                    return FRAME_INCOMPLETE;
                }
            }
            oscl_memcpy((uint8*)memFragOut.getMemFrag().ptr + outPtrPos, myData, framesize);
            myData += framesize;
            outPtrPos += framesize;
        }



        // to update number of bytes copied
        memFragOut.getMemFrag().len = outPtrPos;
        mediaDataOut->setMediaFragFilledLen(0, outPtrPos);

        bytesRead = 0;
        framesize = 0;
        compositenumframes = 0;

    }
    else
    {
        compositenumframes++;

        if (compositenumframes < MAX_NUM_COMPOSITE_FRAMES)
        {
            return FRAME_INCOMPLETE;
        }
        else
        {
            return FRAME_ERROR;
        }

    }

    return FRAME_COMPLETE;
}

uint32 BufferShowBits(uint8 *inbuf, uint32 pos1, uint32 pos2)
{
    uint32 ptr1 = pos1 % 8, ptr2 = pos2 % 8, Ptr1 = pos1 >> 3, Ptr2 = pos2 >> 3;
    uint32 tmpvar = 0;
    // this really should be static, but not supported in EPOC
    uint8 mask[] = {255, 127, 63, 31, 15, 7, 3, 1, 0};

    if (Ptr1 == Ptr2) // When all the bits to copy are in the same byte
    {
        return ((inbuf[Ptr1] & (mask[ptr1] - mask[ptr2+1])) >> (7 - ptr2));
    }
    else // Otherwise
    {
        tmpvar = inbuf[Ptr1] & mask[ptr1];
        for (Ptr1 ++; Ptr1 < Ptr2; Ptr1 ++)
        {
            tmpvar <<= 8;
            tmpvar += inbuf[Ptr1];
        }
        tmpvar <<= (ptr2 + 1);
        tmpvar += (inbuf[Ptr2] & (255 - mask[ptr2+1])) >> (7 - ptr2);
    }
    return tmpvar;
}

uint32 BufferReadBits(uint8 *inbuf, uint32 *pos1, int32 len)
{
    uint32 rval = BufferShowBits(inbuf, *pos1, (*pos1) + len - 1);
    *pos1 += len;
    return rval;
}

