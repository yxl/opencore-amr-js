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

/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/

#include "amr_dec.h"
#include "frame.h"
#include "cnst.h"
#include "d_homing.h"

#define ETS_INPUT_FRAME_SIZE 500

//Compressed audio formats
#define PV_AMR_IETF					0
#define PV_AMR_IF2					1
#define PV_AMR_ETS                  2
#define PV_AMR_IETF_COMBINED        3

//WB modes
#define PV_AMRWB_IETF_PAYLOAD        4
#define PV_AMRWB_IETF		         5


// decoder returns -1 if there is an error in decoding a frame
#define PV_GSMAMR_DECODE_STATUS_ERR -1

// Currently, decoder returns a value >=0 if there is no error.(should be 0)
#define PV_GSMAMR_DECODE_STATUS_OK 0

// Find frame size for each frame type
static const OMX_S32 WBIETFFrameSize[16] =
{
    18		// AMR-WB 6.60 Kbps
    , 24		// AMR-WB 8.85 Kbps
    , 33		// AMR-WB 12.65 Kbps
    , 37		// AMR-WB 14.25 Kbps
    , 41		// AMR-WB 15.85 Kbps
    , 47		// AMR-WB 18.25 Kbps
    , 51		// AMR-WB 19.85 Kbps
    , 59		// AMR-WB 23.05 Kbps
    , 61		// AMR-WB 23.85 Kbps
    , 6		// AMR-WB SID
    , 1
    , 1
    , 1
    , 1
    , 1		// WBAMR Frame No Data
    , 1		// WBAMR Frame No Data
};

//////////////////////////////////////////////////////////////////////////////////
static const OMX_S32 IETFFrameSize[16] =
{
    13		// AMR 4.75 Kbps
    , 14		// AMR 5.15 Kbps
    , 16		// AMR 5.90 Kbps
    , 18		// AMR 6.70 Kbps
    , 20		// AMR 7.40 Kbps
    , 21		// AMR 7.95 Kbps
    , 27		// AMR 10.2 Kbps
    , 32		// AMR 12.2 Kbps
    , 6		// GsmAmr comfort noise
    , 7		// Gsm-Efr comfort noise
    , 6		// IS-641 comfort noise
    , 6		// Pdc-Efr comfort noise
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// AMR Frame No Data
};

static const OMX_S32 IF2FrameSize[16] =
{
    13		// AMR 4.75 Kbps
    , 14		// AMR 5.15 Kbps
    , 16		// AMR 5.90 Kbps
    , 18		// AMR 6.70 Kbps
    , 19		// AMR 7.40 Kbps
    , 21		// AMR 7.95 Kbps
    , 26		// AMR 10.2 Kbps
    , 31		// AMR 12.2 Kbps
    , 6		// AMR Frame SID
    , 6		// AMR Frame GSM EFR SID
    , 6		// AMR Frame TDMA EFR SID
    , 6		// AMR Frame PDC EFR SID
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// future use; 0 length but set to 1 to skip the frame type byte
    , 1		// AMR Frame No Data
};

OmxAmrDecoder::OmxAmrDecoder()
{
    iOmxInputFormat = PV_AMR_ETS;
    iAMRFramesinTOC = 0;
    iAmrInitFlag = 0;
    iNarrowBandFlag = OMX_TRUE;

    //Output frame size in NB would be double the L_FRAME, due to char* output buffer in case of openmax instead of short* in console app
    iOutputFrameSize = L_FRAME * 2;

    iCodecExternals = NULL;
    iAudioAmrDecoder = NULL;
    iTocTablePtr = NULL;
    /* Initialize decoder homing flags */
    iDecHomingFlag = 0;
    iDecHomingFlagOld = 1;
}

/* Decoder Initialization function */
OMX_BOOL OmxAmrDecoder::AmrDecInit(OMX_AUDIO_AMRFRAMEFORMATTYPE aInFormat, OMX_AUDIO_AMRBANDMODETYPE aInMode)
{
    OMX_S32 Status = 0;

    iAmrInitFlag = 0;

    if ((aInMode >= OMX_AUDIO_AMRBandModeNB0) && (aInMode <= OMX_AUDIO_AMRBandModeNB7))
    {
        iAudioAmrDecoder = CDecoder_AMR_NB::NewL();

        if (!iAudioAmrDecoder)
        {
            return OMX_FALSE;
        }

        iNarrowBandFlag = OMX_TRUE;
        iOutputFrameSize = L_FRAME * 2;
    }

    else if ((aInMode >= OMX_AUDIO_AMRBandModeWB0) && (aInMode <= OMX_AUDIO_AMRBandModeWB8))
    {
        iAudioAmrDecoder = CDecoder_AMR_WB::NewL();

        if (!iAudioAmrDecoder)
        {
            return OMX_FALSE;
        }

        iNarrowBandFlag = OMX_FALSE;
        iOutputFrameSize = L_FRAME * 4;
    }
    else
    {
        return OMX_FALSE;
    }


    if (!iCodecExternals)
    {
        iCodecExternals = OSCL_NEW(tPVAmrDecoderExternal, ());
        if (!iCodecExternals)
        {
            return OMX_FALSE;
        }
    }

    //initialize all fields to 0
    oscl_memset(iCodecExternals, 0, sizeof(tPVAmrDecoderExternal));
    iCodecExternals->quality = 1;  // assume its always good data

    //Extracting the input format information
    if (OMX_AUDIO_AMRFrameFormatConformance == aInFormat)
    {
        iOmxInputFormat = PV_AMR_ETS;
        iCodecExternals->input_format = ETS;
    }
    else if (OMX_AUDIO_AMRFrameFormatIF2 == aInFormat)
    {
        iOmxInputFormat = PV_AMR_IF2;
        iCodecExternals->input_format = IF2;
    }
    else if (OMX_AUDIO_AMRFrameFormatRTPPayload == aInFormat)
    {
        if (OMX_TRUE == iNarrowBandFlag)
        {
            iOmxInputFormat = PV_AMR_IETF_COMBINED;
        }
        else
        {
            iOmxInputFormat = PV_AMRWB_IETF_PAYLOAD;
        }

        iCodecExternals->input_format = MIME_IETF;
    }
    else if (OMX_AUDIO_AMRFrameFormatFSF == aInFormat)
    {
        if (OMX_TRUE == iNarrowBandFlag)
        {
            iOmxInputFormat = PV_AMR_IETF;
        }
        else
        {
            iOmxInputFormat = PV_AMRWB_IETF;
        }

        iCodecExternals->input_format = MIME_IETF;
    }

    Status = iAudioAmrDecoder->StartL(iCodecExternals, false, false);

    if (Status)
    {
        return OMX_FALSE;
    }

    return OMX_TRUE;
}


/* Decoder De-Initialization function */
void OmxAmrDecoder::AmrDecDeinit()
{
    /* This function call is platform-specific */
    if (iAudioAmrDecoder)
    {
        iAudioAmrDecoder->TerminateDecoderL();
        OSCL_DELETE(iAudioAmrDecoder);
        iAudioAmrDecoder = NULL;

        if (iCodecExternals)
        {
            OSCL_DELETE(iCodecExternals);
            iCodecExternals = NULL;
        }
    }
}


void OmxAmrDecoder::ResetDecoder()
{
    if (iAudioAmrDecoder)
    {
        iAudioAmrDecoder->ResetDecoderL();
    }
    iAMRFramesinTOC = 0;
}


/* Find the start point & size of TOC table in case of IETF_Combined format */
void OmxAmrDecoder::GetStartPointsForIETFCombinedMode
(OMX_U8* aPtrIn, OMX_U32 aLength, OMX_U8* &aTocPtr, OMX_S32* aNumOfBytes)
{
    OMX_U8 Fbit		= 0x80;
    OMX_U32 FrameCnt = 0;

    /* Count number of frames */
    aTocPtr = aPtrIn;
    while ((*(aTocPtr + FrameCnt) & Fbit) && (FrameCnt < aLength))
    {
        FrameCnt++;
    }

    FrameCnt++;
    *aNumOfBytes = FrameCnt;
}


/* Decode function for all the input formats */
OMX_BOOL OmxAmrDecoder::AmrDecodeFrame(OMX_S16* aOutputBuffer,
                                       OMX_U32* aOutputLength, OMX_U8** aInBuffer,
                                       OMX_U32* aInBufSize, OMX_S32* aIsFirstBuffer,
                                       OMX_BOOL* aResizeFlag)
{
    OMX_BOOL Status = OMX_TRUE;

    *aResizeFlag = OMX_FALSE;

    OMX_S32 ByteOffset, ii;
    TXFrameType  TxFrame;

    /* 3GPP Frame Type Buffer */
    Frame_Type_3GPP FrameType3gpp;

    /* Takes care of extra bytes above the decoded ones
     * e.g. toc length for ietf_combined, frame header length &
     * one frame type byte for ietf format.
     */
    OMX_S32 FrameBytesProcessed = 0, FrameLength;

    /* Reset speech_bits buffer pointer */
    OMX_U8* pSpeechBits = *aInBuffer;
    OMX_U8 *pTocPtr;
    //ETS mode requires a 16-bit pointer
    OMX_S16* pEtsSpeechBits = (OMX_S16*) * aInBuffer;

    if ((PV_AMR_IETF_COMBINED == iOmxInputFormat) || (PV_AMR_IETF == iOmxInputFormat)
            || (PV_AMRWB_IETF_PAYLOAD == iOmxInputFormat) || (PV_AMRWB_IETF == iOmxInputFormat))
    {
        if ((PV_AMR_IETF_COMBINED == iOmxInputFormat) || (PV_AMRWB_IETF_PAYLOAD == iOmxInputFormat))
        {
            if (0 == iAMRFramesinTOC)
            {
                pTocPtr = NULL;
                GetStartPointsForIETFCombinedMode(pSpeechBits, *aInBufSize,
                                                  pTocPtr, &iAMRFramesinTOC);
                pSpeechBits += iAMRFramesinTOC;
                FrameBytesProcessed = iAMRFramesinTOC;

                iTocTablePtr = pTocPtr;
                iAMRFramesinTOC--; // ctr of amr frames
            }
            else
            {
                iAMRFramesinTOC--;
                FrameBytesProcessed = 0;
            }

            FrameType3gpp = GetFrameTypeLength(iTocTablePtr, &FrameLength);
        }
        else //iOmxInputFormat == PV_AMR_IETF or (PV_AMRWB_IETF == iOmxInputFormat)
        {
            if (0 == iAmrInitFlag)
            {
                if ('#' == pSpeechBits[0])
                {
                    pSpeechBits += 6;
                    FrameBytesProcessed = 6;
                }
                iAmrInitFlag = 1;
            }

            FrameType3gpp = GetFrameTypeLength(pSpeechBits, &FrameLength);
        }

        // check if the frame size exceeds buffer boundaries
        if ((FrameLength + FrameBytesProcessed) <= (OMX_S32) *aInBufSize)
        {
            /* Set up pointer to the start of frame to be decoded */
            iCodecExternals->mode = (uint32)FrameType3gpp;
            iCodecExternals->pInputBuffer = (uint8*) pSpeechBits;
            iCodecExternals->pOutputBuffer = (int16*) aOutputBuffer;

            ByteOffset = iAudioAmrDecoder->ExecuteL(iCodecExternals);

            if (PV_GSMAMR_DECODE_STATUS_ERR == ByteOffset)
            {
                *aInBufSize = 0;
                *aOutputLength = 0;
                iAMRFramesinTOC = 0; // make sure the TOC table (if necessary) gets initialized for the next time
                Status = OMX_FALSE;
            }
            else
            {
                *aInBufSize -= (FrameLength + FrameBytesProcessed);
                *aInBuffer += (FrameLength + FrameBytesProcessed);
                *aOutputLength = iOutputFrameSize;
                // in case of TOC, make sure that
                // a) if no more data in the buffer and TOC indicates more data, reset TOC
                // b) if TOC indicates no more data, and there is more data in the buffer, reset the buffer
                if ((PV_AMR_IETF_COMBINED == iOmxInputFormat) || (PV_AMRWB_IETF_PAYLOAD == iOmxInputFormat))
                {
                    if ((0 == iAMRFramesinTOC) || (0 == *aInBufSize))
                    {
                        *aInBufSize = 0;
                        iAMRFramesinTOC = 0;
                    }
                }

            }
        }
        else
        {
            *aInBufSize = 0;
            *aOutputLength = 0;
            iAMRFramesinTOC = 0; // make sure the TOC table (if necessary) gets initialized for the next time
            Status = OMX_FALSE; // treat buffer overrun as an error
        }

    }
    else if (PV_AMR_IF2 == iOmxInputFormat)
    {
        FrameType3gpp = (Frame_Type_3GPP)(pSpeechBits[0] & 0xF);
        FrameLength = IF2FrameSize[FrameType3gpp];

        // check if the frame size exceeds buffer boundaries
        if ((FrameLength + FrameBytesProcessed) <= (OMX_S32) *aInBufSize)
        {
            /* Set up pointer to the start of frame to be decoded */
            iCodecExternals->mode = (uint32)FrameType3gpp;
            iCodecExternals->pInputBuffer = (uint8*) pSpeechBits;
            iCodecExternals->pOutputBuffer = (int16*) aOutputBuffer;

            ByteOffset = iAudioAmrDecoder->ExecuteL(iCodecExternals);

            if (PV_GSMAMR_DECODE_STATUS_ERR == ByteOffset)
            {
                Status = OMX_FALSE;
            }

            if (ByteOffset <= (OMX_S32)*aInBufSize)
            {
                *aInBufSize -= ByteOffset;
                *aInBuffer += ByteOffset;
                *aOutputLength = iOutputFrameSize;
            }
            else
            {
                *aInBufSize = 0;
                *aOutputLength = 0;
                Status = OMX_FALSE;
            }
        }
        else
        {
            *aInBufSize = 0;
            *aOutputLength = 0;
            Status = OMX_FALSE; // treat buffer overrun as an error
        }
    }
    else if (PV_AMR_ETS == iOmxInputFormat)
    {
        FrameType3gpp = (enum Frame_Type_3GPP) pSpeechBits[(1 + MAX_SERIAL_SIZE) * 2];

        /* Get TX frame type */
        TxFrame = (TXFrameType)pEtsSpeechBits[0];

        /* Convert TX frame type to RX frame type */
        switch (TxFrame)
        {
            case TX_SPEECH_GOOD:
                pEtsSpeechBits[0] = RX_SPEECH_GOOD;
                break;

            case TX_SPEECH_DEGRADED:
                pEtsSpeechBits[0] = RX_SPEECH_DEGRADED;
                break;

            case TX_SPEECH_BAD:
                pEtsSpeechBits[0] = RX_SPEECH_BAD;
                break;

            case TX_SID_FIRST:
                pEtsSpeechBits[0] = RX_SID_FIRST;
                break;

            case TX_SID_UPDATE:
                pEtsSpeechBits[0] = RX_SID_UPDATE;
                break;

            case TX_SID_BAD:
                pEtsSpeechBits[0] = RX_SID_BAD;
                break;

            case TX_ONSET:
                pEtsSpeechBits[0] = RX_ONSET;
                break;

            case TX_NO_DATA:
                pEtsSpeechBits[0] = RX_NO_DATA;
                FrameType3gpp = (enum Frame_Type_3GPP) iCodecExternals->mode;
                break;

            default:
                break;
        }

        /* if homed: check if this frame is another homing frame */
        if (1 == iDecHomingFlagOld)
        {
            /* only check until end of first subframe */
            iDecHomingFlag = decoder_homing_frame_test_first(
                                 (OMX_S16*) & pEtsSpeechBits[1],
                                 (enum Mode) FrameType3gpp);
        }

        /* produce encoder homing frame if homed & input=decoder homing frame */
        if ((0 != iDecHomingFlag) && (0 != iDecHomingFlagOld))
        {
            for (ii = 0; ii < L_FRAME; ii++)
            {
                aOutputBuffer[ii] = EHF_MASK;
            }
        }
        else
        {
            /* Set up pointer to the start of frame to be decoded */
            iCodecExternals->mode = (uint32)FrameType3gpp;
            iCodecExternals->pInputBuffer = (uint8*) pEtsSpeechBits;
            iCodecExternals->pOutputBuffer = (int16*) aOutputBuffer;

            ByteOffset = iAudioAmrDecoder->ExecuteL(iCodecExternals);

            if (PV_GSMAMR_DECODE_STATUS_ERR == ByteOffset)
            {
                Status = OMX_FALSE;
            }

        }

        /* if not homed: check whether current frame is a homing frame */
        if (0 == iDecHomingFlagOld)
        {
            /* check whole frame */
            iDecHomingFlag = decoder_homing_frame_test(
                                 (OMX_S16*) & pEtsSpeechBits[1],
                                 (enum Mode) FrameType3gpp);
        }
        /* reset decoder if current frame is a homing frame */
        if (0 != iDecHomingFlag)
        {
            iAudioAmrDecoder->ResetDecoderL();
        }

        iDecHomingFlagOld = iDecHomingFlag;

        //Input buffer requirement per frame is constant at ETS_INPUT_FRAME_SIZE
        *aInBufSize -= ETS_INPUT_FRAME_SIZE;
        *aInBuffer += ETS_INPUT_FRAME_SIZE;
        *aOutputLength = iOutputFrameSize;
    }

    (*aIsFirstBuffer)++;

    //After decoding the first frame, modify all the input & output port settings
    if (1 == *aIsFirstBuffer)
    {
        //Set the Resize flag to send the port settings changed callback
        *aResizeFlag = OMX_TRUE;
    }

    return Status;
}

/* Decode function for all the input formats */
OMX_BOOL OmxAmrDecoder::AmrDecodeSilenceFrame(OMX_S16* aOutputBuffer,
        OMX_U32* aOutputLength)
{
    OMX_BOOL Status = OMX_TRUE;
    OMX_S32 ByteOffset;
    OMX_U8 FrameType = 15; // silence frame

    iCodecExternals->mode = (uint32) FrameType;
    iCodecExternals->pInputBuffer = (uint8*) & FrameType;
    iCodecExternals->pOutputBuffer = (int16*) aOutputBuffer;

    ByteOffset = iAudioAmrDecoder->ExecuteL(iCodecExternals);

    if (PV_GSMAMR_DECODE_STATUS_ERR == ByteOffset)
    {
        Status = OMX_FALSE;
    }
    else
    {
        *aOutputLength = iOutputFrameSize;
    }

    return Status;
}


/* Get Frame type for format == PVMF_AMR_IETF or PVMF_AMR_IETF_COMBINED and the WB counterparts*/
Frame_Type_3GPP OmxAmrDecoder::GetFrameTypeLength(OMX_U8* &aFrame, OMX_S32* aFrameLength)
{
    Frame_Type_3GPP  FrameType3gpp;


    FrameType3gpp = (Frame_Type_3GPP)((aFrame[0] >> 3) & 0x0F);

    //Narrow Band AMR
    if (OMX_TRUE == iNarrowBandFlag)
    {
        *aFrameLength = IETFFrameSize[FrameType3gpp];
    }
    else
    {
        *aFrameLength = WBIETFFrameSize[FrameType3gpp];
    }



    if (PV_AMR_IETF_COMBINED == iOmxInputFormat || PV_AMRWB_IETF_PAYLOAD == iOmxInputFormat)
    {
        // move ptr for TOC
        aFrame++;
        (*aFrameLength)--; // account for the 1 byte of length being not in the frame, but in the TOC
    }
    else if ((PV_AMR_IETF == iOmxInputFormat) || (PV_AMRWB_IETF == iOmxInputFormat))
    {
        aFrame++; // move ptr to data to skip the frame type/size field
    }


    return (FrameType3gpp);
}
