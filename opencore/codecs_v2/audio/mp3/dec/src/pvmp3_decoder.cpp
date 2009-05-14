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
/*
------------------------------------------------------------------------------

   PacketVideo Corp.
   MP3 Decoder Library

   Filename: pvmp3_decoder.cpp

     Date: 09/25/2007

------------------------------------------------------------------------------
 REVISION HISTORY


 Description:

------------------------------------------------------------------------------


------------------------------------------------------------------------------
 FUNCTION DESCRIPTION

    Entry point to the mp3 library

------------------------------------------------------------------------------
 REQUIREMENTS


------------------------------------------------------------------------------
 REFERENCES

------------------------------------------------------------------------------
 PSEUDO-CODE

------------------------------------------------------------------------------
*/


/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/

#include "pvmp3_decoder.h"
#include "oscl_error_codes.h"
#include "oscl_exception.h"
#include "pvmp3_framedecoder.h"
#include "pvmp3_seek_synch.h"


/*----------------------------------------------------------------------------
; MACROS
; Define module specific macros here
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
; DEFINES
; Include all pre-processor statements here. Include conditional
; compile variables also.
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; LOCAL FUNCTION DEFINITIONS
; Function Prototype declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; LOCAL STORE/BUFFER/POINTER DEFINITIONS
; Variable declaration - defined here and used outside this module
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; EXTERNAL FUNCTION REFERENCES
; Declare functions defined elsewhere and referenced in this module
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; EXTERNAL GLOBAL STORE/BUFFER/POINTER REFERENCES
; Declare variables used in this module but defined elsewhere
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; FUNCTION CODE
----------------------------------------------------------------------------*/


#define KMP3_MAX_STREAMING_BUFFER_SIZE  BUFSIZE

#define KCAI_CODEC_INIT_FAILURE -1

// Use default DLL entry point
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


OSCL_EXPORT_REF CPvMP3_Decoder *CPvMP3_Decoder::NewL()
{
    CPvMP3_Decoder *dec = new CPvMP3_Decoder;

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


OSCL_EXPORT_REF void CPvMP3_Decoder::ConstructL()
{
    // Initialize member variables
    pMem = NULL;
    iInputBuf  = NULL;
    iOutputBuf = NULL;
}


/*
-----------------------------------------------------------------------------

    CPvMP3_Decoder

    ~CPvMP3_Decoder

    Empty decoder destructor.

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF CPvMP3_Decoder::~CPvMP3_Decoder()
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

    CPvMP3_Decoder

    StartL

    Start decoder object. Initialize codec status.

    Parameters:     none

    Return Values:  status

-----------------------------------------------------------------------------
*/

OSCL_EXPORT_REF int32 CPvMP3_Decoder::StartL(tPVMP3DecoderExternal * pExt,
        bool aAllocateInputBuffer,
        bool aAllocateOutputBuffer,
        bool crcEnabler,
        e_equalization equalizType)
{

    iAllocateInputBuffer  = aAllocateInputBuffer;
    iAllocateOutputBuffer = aAllocateOutputBuffer;

    if (iAllocateInputBuffer)
    {
        iInputBuf = OSCL_ARRAY_NEW(uint8, KMP3_MAX_STREAMING_BUFFER_SIZE);
        if (iInputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }
//        pExt->inputBufferMaxLength = KMP3_MAX_STREAMING_BUFFER_SIZE;
        pExt->inputBufferMaxLength = 512;
    }
    else
    {
        iInputBuf = NULL;
        pExt->inputBufferMaxLength = 0;
    }
    pExt->pInputBuffer = iInputBuf;

    if (iAllocateOutputBuffer)
    {
        iOutputBuf = OSCL_ARRAY_NEW(int16, 2304);

        if (iOutputBuf == NULL)
        {
            return KCAI_CODEC_INIT_FAILURE;
        }
    }
    else
    {
        iOutputBuf = NULL;
    }
    pExt->pOutputBuffer = iOutputBuf;

    pExt->crcEnabled               = crcEnabler;
    pExt->equalizerType            = equalizType;  /* Dynamically enable equalizing type */

    int32 memreq =  pvmp3_decoderMemRequirements();

    pMem = OSCL_ARRAY_NEW(uint8 , memreq);

    if (pMem == 0)
    {
        return KCAI_CODEC_INIT_FAILURE;
    }

    pvmp3_InitDecoder(pExt, pMem);


    return SUCCESS;
}


/*
-----------------------------------------------------------------------------

    CPvMP3_Decoder

    ExecuteL

    Execute decoder object. Read one encoded mp3 frame from the input
    stream,  decode it and write the decoded frame to output stream.

    Parameters:
    tPVMP3DecoderExternal * pExt, pointer to decoder external state variables

    Return Values:  status


-----------------------------------------------------------------------------
*/

OSCL_EXPORT_REF int32 CPvMP3_Decoder::ExecuteL(tPVMP3DecoderExternal * pExt)
{
    ERROR_CODE   errorCode = NO_DECODING_ERROR;
    int32 status;

    errorCode = pvmp3_framedecoder(pExt, pMem);


    switch (errorCode)
    {
        case NO_DECODING_ERROR:

            status = MP3DEC_SUCCESS;
            break;

        case NO_ENOUGH_MAIN_DATA_ERROR:

            status = MP3DEC_INCOMPLETE_FRAME;
            break;

        case UNSUPPORTED_LAYER:
        case UNSUPPORTED_FREE_BITRATE:
        case CHANNEL_CONFIG_ERROR:
        case SYNTHESIS_WINDOW_ERROR:
        case SIDE_INFO_ERROR:
        case HUFFMAN_TABLE_ERROR:
        case SYNCH_LOST_ERROR:
        default:

            status = MP3DEC_INVALID_FRAME;
            break;

    }


    return status;
}

/*
-----------------------------------------------------------------------------

    CPvMP3_Decoder

    StopL

    Stop decoder object. Flush out last frame

    Parameters:
    tPVMP3DecoderExternal * pExt, pointer to decoder external state variables


    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CPvMP3_Decoder::StopL()
{
    pvmp3_resetDecoder(pMem);
}

/*
-----------------------------------------------------------------------------

    CPvMP3_Decoder

    ResetDecoderL

    Stop decoder object. Reset decoder.

    Parameters:
    tPVMP3DecoderExternal * pExt, pointer to decoder external state variables


    Return Values:  status

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CPvMP3_Decoder::ResetDecoderL()
{

    pvmp3_resetDecoder(pMem);
}


/*
-----------------------------------------------------------------------------

    CPvMP3_Decoder

    TerminateDecoderL

    Stop decoder object. close decoder.

    Parameters:     none

    Return Values:  none

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF void CPvMP3_Decoder::TerminateDecoderL()
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

    CPvMP3_Decoder

    SeekMp3Synchronization

    Utility to seek for the mp3 frames boundaries.

    Parameters:
    tPVMP3DecoderExternal * pExt, pointer to decoder external state variables

    Return Values:  status

-----------------------------------------------------------------------------
*/
OSCL_EXPORT_REF int32 CPvMP3_Decoder::SeekMp3Synchronization(
    tPVMP3DecoderExternal * pExt)
{
    ERROR_CODE   errorCode = NO_DECODING_ERROR;

    errorCode = pvmp3_frame_synch(pExt, pMem);

    if (errorCode != NO_DECODING_ERROR)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}




