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
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  File: decoder_aac.cpp                                                       //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////


#include "decoder_aac.h"
#include "oscl_error_codes.h"
#include "oscl_exception.h"

#include "e_tmp4audioobjecttype.h"

// Use default DLL entry point
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


OSCL_EXPORT_REF CDecoder_AAC *CDecoder_AAC::NewL()
{
    CDecoder_AAC *dec = new CDecoder_AAC;

    if (dec == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    else
    {
        dec->ConstructL();
    }

    return dec;
}


OSCL_EXPORT_REF void CDecoder_AAC::ConstructL()
{
    // Initialize member variables
    iFirstFrame = true;
    iNumSamplesPerFrame = KAAC_NUM_SAMPLES_PER_FRAME;
    pMem = NULL;
    iAllocateInputBuffer  = true;
    iAllocateOutputBuffer = true;
    iInputBuf  = NULL;
    iOutputBuf = NULL;
}


/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    ~CDecoder_AAC

    Empty decoder destructor.

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF CDecoder_AAC::~CDecoder_AAC()
{
    if (pMem != NULL)
    {
        OSCL_ARRAY_DELETE(pMem);
        pMem = NULL;
    }

    if (iAllocateInputBuffer && iInputBuf)
    {
        OSCL_ARRAY_DELETE(iInputBuf);
        iInputBuf = NULL;
    }

    if (iAllocateOutputBuffer && iOutputBuf)
    {
        OSCL_ARRAY_DELETE(iOutputBuf);
        iOutputBuf = NULL;
    }
}


/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    StartL

    Start decoder object. Initialize codec status.

    Parameters:     none

    Return Values:  status

-----------------------------------------------------------------------------
*/

OSCL_EXPORT_REF int32 CDecoder_AAC::StartL(tPVMP4AudioDecoderExternal * pExt,
        uint8  num_channels,
        bool aAllocateInputBuffer,
        bool aAllocateOutputBuffer,
        Int	upsamplingFactor,
        Int	samp_rate,
        tMP4AudioObjectType  audioObjectType)
{
    iFirstFrame = true;

    iAllocateInputBuffer = aAllocateInputBuffer;
    iAllocateOutputBuffer = aAllocateOutputBuffer;

    if (iAllocateInputBuffer)
    {
        iInputBuf = OSCL_ARRAY_NEW(uint8, KAAC_MAX_STREAMING_BUFFER_SIZE);
        if (iInputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }
        pExt->pInputBuffer = iInputBuf;
        pExt->inputBufferMaxLength = KAAC_MAX_STREAMING_BUFFER_SIZE;
    }
    else
    {
        pExt->pInputBuffer = NULL;
        pExt->inputBufferMaxLength = 0;
    }

    if (iAllocateOutputBuffer)
    {
#ifdef AAC_PLUS
        iOutputBuf = OSCL_ARRAY_NEW(int16, 4096);
#else
        iOutputBuf = OSCL_ARRAY_NEW(int16, 2048);
#endif

        if (iOutputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }

        pExt->pOutputBuffer = iOutputBuf;
#ifdef AAC_PLUS
        pExt->pOutputBuffer_plus = &iOutputBuf[2048];
#else
        pExt->pOutputBuffer_plus = NULL;
#endif
    }
    else
    {
        pExt->pOutputBuffer = NULL;
        pExt->pOutputBuffer_plus = NULL;
    }

    pExt->desiredChannels          = num_channels;
    pExt->inputBufferCurrentLength = 0;
    pExt->outputFormat             = OUTPUTFORMAT_16PCM_INTERLEAVED;
    pExt->repositionFlag           = TRUE;
    pExt->inputBufferUsedLength    = 0;
    pExt->remainderBits            = 0;

    int32 memreq =  PVMP4AudioDecoderGetMemRequirements();

    pMem = OSCL_ARRAY_NEW(uint8 , memreq);

    if (pMem == 0)
    {
        return KCAI_CODEC_NO_MEMORY;
    }

    if (PVMP4AudioDecoderInitLibrary(pExt, pMem) != 0)
    {
        return(KCAI_CODEC_INIT_FAILURE);
    }


    if (PVMP4SetAudioConfig(pExt,
                            pMem,
                            upsamplingFactor,
                            samp_rate,
                            num_channels,
                            audioObjectType) != SUCCESS)
    {
        return KCAI_CODEC_INIT_FAILURE;
    }

    iNumSamplesPerFrame = KAAC_NUM_SAMPLES_PER_FRAME;

    pExt->desiredChannels = pExt->encodedChannels;

    iFirstFrame = false;

    return SUCCESS;
}


/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    StartL

    Start decoder object. Initialize codec status.

    Parameters:     none

    Return Values:  status

-----------------------------------------------------------------------------
*/




OSCL_EXPORT_REF int32 CDecoder_AAC::StartL(tPVMP4AudioDecoderExternal * pExt,
        uint8  num_channels,
        bool aAllocateInputBuffer,
        bool aAllocateOutputBuffer,
        bool aAacplusEnabler)
{
    iFirstFrame = true;

    iAllocateInputBuffer = aAllocateInputBuffer;
    iAllocateOutputBuffer = aAllocateOutputBuffer;

    if (iAllocateInputBuffer)
    {
        iInputBuf = OSCL_ARRAY_NEW(uint8, KAAC_MAX_STREAMING_BUFFER_SIZE);
        if (iInputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }
        pExt->pInputBuffer = iInputBuf;
        pExt->inputBufferMaxLength = KAAC_MAX_STREAMING_BUFFER_SIZE;
    }
    else
    {
        pExt->pInputBuffer = NULL;
        pExt->inputBufferMaxLength = 0;
    }

    if (iAllocateOutputBuffer)
    {
#ifdef AAC_PLUS
        iOutputBuf = OSCL_ARRAY_NEW(int16, 4096);
#else
        iOutputBuf = OSCL_ARRAY_NEW(int16, 2048);
#endif

        if (iOutputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }

        pExt->pOutputBuffer = iOutputBuf;
#ifdef AAC_PLUS
        pExt->pOutputBuffer_plus = &iOutputBuf[2048];
#else
        pExt->pOutputBuffer_plus = NULL;
#endif
    }
    else
    {
        pExt->pOutputBuffer = NULL;
        pExt->pOutputBuffer_plus = NULL;
    }

    pExt->desiredChannels          = num_channels;
    pExt->inputBufferCurrentLength = 0;
    pExt->outputFormat             = OUTPUTFORMAT_16PCM_INTERLEAVED;
    pExt->repositionFlag           = TRUE;
    pExt->aacPlusEnabled           = aAacplusEnabler;  /* Dynamically enable AAC+ decoding */
    pExt->inputBufferUsedLength    = 0;
    pExt->remainderBits            = 0;

    int32 memreq =  PVMP4AudioDecoderGetMemRequirements();

    pMem = OSCL_ARRAY_NEW(uint8 , memreq);

    if (pMem == 0)
    {
        return KCAI_CODEC_NO_MEMORY;
    }

    return SUCCESS;
}


/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    ExecuteL

    Execute decoder object. Read one encoded aac frame from the input
    stream,  decode it and write the decoded frame to output stream.

    Parameters:

    Return Values:  status


-----------------------------------------------------------------------------
*/

OSCL_EXPORT_REF int32 CDecoder_AAC::ExecuteL(tPVMP4AudioDecoderExternal * pExt)
{
    int32 nResult = FALSE;

    if (iFirstFrame == false)
    {
        nResult = PVMP4AudioDecodeFrame(pExt, pMem);
    }
    else
    {
        if (PVMP4AudioDecoderInitLibrary(pExt, pMem) != 0)
        {
            return(KCAI_CODEC_INIT_FAILURE);
        }
        // set samples/frame to default value (right now this is the only one supported)

        if (PVMP4AudioDecoderConfig(pExt, pMem) !=  SUCCESS)
        {
            nResult = PVMP4AudioDecodeFrame(pExt, pMem);    /* could be ADIF type  */
        }

        iNumSamplesPerFrame = KAAC_NUM_SAMPLES_PER_FRAME;

        pExt->desiredChannels = pExt->encodedChannels;

        iFirstFrame = false;
    }

    return nResult;
}

/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    StopL

    Stop decoder object. Flush out last frame

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CDecoder_AAC::StopL()
{
    PVMP4AudioDecoderResetBuffer(pMem);
}

/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    ResetDecoderL

    Stop decoder object. Reset decoder.

    Parameters:     none

    Return Values:  status

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CDecoder_AAC::ResetDecoderL()
{
    PVMP4AudioDecoderResetBuffer(pMem);
}


/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    TerminateDecoderL

    Stop decoder object. close decoder.

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CDecoder_AAC::TerminateDecoderL()
{
    if (pMem != NULL)
    {
        OSCL_ARRAY_DELETE(pMem);
        pMem = NULL;
    }

    if (iAllocateInputBuffer && iInputBuf)
    {
        OSCL_ARRAY_DELETE(iInputBuf);
        iInputBuf = NULL;
    }

    if (iAllocateOutputBuffer && iOutputBuf)
    {
        OSCL_ARRAY_DELETE(iOutputBuf);
        iOutputBuf = NULL;
    }
}



/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    RetrieveDecodedStreamTypeL

    Utility to extract the decoding aac stream type being decoded.

    Parameters:     none

    Return Values:  status

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF int32 CDecoder_AAC::RetrieveDecodedStreamTypeL(tPVMP4AudioDecoderExternal * pExt)
{

    if ((pExt->extendedAudioObjectType == MP4AUDIO_AAC_LC) ||
            (pExt->extendedAudioObjectType == MP4AUDIO_LTP))
    {
        return AAC;   /*  AAC */
    }
    else if (pExt->extendedAudioObjectType == MP4AUDIO_SBR)
    {
        return AACPLUS;   /*  AAC+ */
    }
    else if (pExt->extendedAudioObjectType == MP4AUDIO_PS)
    {
        return ENH_AACPLUS;   /*  AAC++ */
    }

    return -1;   /*  Error evaluating the stream type */
}



/*
-----------------------------------------------------------------------------

    CDecoder_AAC

    DisableSbrDecodingL

    Disable aac+/eaac+ decoding.

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CDecoder_AAC::DisableSbrDecodingL(tPVMP4AudioDecoderExternal * pExt)
{
    PVMP4AudioDecoderDisableAacPlus(pExt, pMem);
}




