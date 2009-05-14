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
/*! \file   pvmp3_decoder.h
 *  \brief  Header file for the MP3 decoder library
 *
 */

#ifndef PVMP3_DECODER_H
#define PVMP3_DECODER_H


#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#include "pvmp3decoder_api.h"
#include "oscl_base.h"


/*!
 *  \brief  number of samples per frame (decoded frames)
 */


#define KMP3_MAX_OUTPUT_SIZE	4608

/*!
 *  \class  CPvMP3_Decoder "include/pvmp3_decoder.h"
 *  \brief  MP3 Decoder class
 */


// CPvMP3_Decoder

/*!
 *  \class  CPvMP3_Decoder
 *  \brief  MP3 Decoder class
 */


// CPvMP3_Decoder
class CPvMP3_Decoder
{
    public:
        OSCL_IMPORT_REF void ConstructL();
        OSCL_IMPORT_REF static CPvMP3_Decoder *NewL();
        OSCL_IMPORT_REF ~CPvMP3_Decoder();

        OSCL_IMPORT_REF int32 StartL(tPVMP3DecoderExternal * pExt,
                                     bool aAllocateInputBuffer = false,
                                     bool aAllocateOutputBuffer = false,
                                     bool crcEnabler = false,
                                     e_equalization equalizType = flat
                                    );
        OSCL_IMPORT_REF int32 ExecuteL(tPVMP3DecoderExternal * pExt);
        OSCL_IMPORT_REF void ResetDecoderL();
        OSCL_IMPORT_REF void StopL();

        OSCL_IMPORT_REF void TerminateDecoderL();
        OSCL_IMPORT_REF int32 SeekMp3Synchronization(tPVMP3DecoderExternal * pExt);



    private:
        bool iAllocateInputBuffer;
        bool iAllocateOutputBuffer;

        uint8 *pMem;

        uint8* iInputBuf;
        int16* iOutputBuf;
};



#endif

