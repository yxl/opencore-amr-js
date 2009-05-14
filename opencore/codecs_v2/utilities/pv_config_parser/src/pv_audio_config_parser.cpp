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

#include "pv_audio_config_parser.h"
#include "oscl_mem.h"
#include "getactualaacconfig.h"


#include "oscl_dll.h"

//Macros for WMA
#define GetUnalignedWord( pb, w ) \
            (w) = ((uint16) *(pb + 1) << 8) + *pb;

#define GetUnalignedDword( pb, dw ) \
            (dw) = ((uint32) *(pb + 3) << 24) + \
                   ((uint32) *(pb + 2) << 16) + \
                   ((uint16) *(pb + 1) << 8) + *pb;

#define GetUnalignedWordEx( pb, w )     GetUnalignedWord( pb, w ); (pb) += sizeof(uint16);
#define GetUnalignedDwordEx( pb, dw )   GetUnalignedDword( pb, dw ); (pb) += sizeof(uint32);

#define LoadWORD( w, p )    GetUnalignedWordEx( p, w )
#define LoadDWORD( dw, p )  GetUnalignedDwordEx( p, dw )


#define WAVE_FORMAT_MSAUDIO1  0x0160
#define WAVE_FORMAT_WMAUDIO2  0x0161
#define WAVE_FORMAT_WMAUDIO3  0x0162
#define WAVE_FORMAT_WMAUDIO_LOSSLESS  0x0163
#define WAVE_FORMAT_WMAUDIO2_ES  0x0165
#define WAVE_FORMAT_WMASPDIF 0x164
#define WAVE_FORMAT_WMAUDIO3_ES  0x0166
#define WAVE_FORMAT_WMAUDIO_LOSSLESS_ES  0x0167

#define WAVE_FORMAT_MSSPEECH  10

//AdvancedEncodeOpt
#define ENCOPT4_PLUSVER   0xe000
#define ENCOPT4_PLUSV1    0xc000
#define ENCOPT4_PLUSV1ALT 0x8000
#define ENCOPT4_PLUSV2    0x2000
#define ENCOPT4_PLUSV3    0x4000

//The following is from \codecs_v2\wma\dec\src\macros.h
//-------------------------------------------------------------------------------------
//If no build is defined in the pre-processor definitions, then code for all supported wma formats will be included
#if !defined(BUILD_WMASTD) && !defined(BUILD_WMAPRO) && !defined(BUILD_WMALSL) && !defined(BUILD_WMAPROPLUS)
#define BUILD_WMASTD
#define BUILD_WMAPRO
#define BUILD_WMAPROPLUS
//#define BUILD_WMAPRO_PLLM
//#define BUILD_WMALSL
#endif // !BUILD_WMASTD && !BUILD_WMAPRO && !BUILD_WMAPROPLUS && !BUILD_WMALSL

#if defined(BUILD_WMAPROPLUS) && !defined(BUILD_WMAPRO)
#define BUILD_WMAPRO
#endif

#if defined(BUILD_WMAPRO) && !defined(BUILD_WMAPROPLUS)
#define BUILD_WMAPROPLUS
#endif
//-------------------------------------------------------------------------------------

OSCL_DLL_ENTRY_POINT_DEFAULT()


// This routine parses the wma config header and returns Sampling Rate, Number of Channels, and Bits Per Sample
OSCL_EXPORT_REF int32 pv_audio_config_parser(pvAudioConfigParserInputs *aInputs, pvAudioConfigParserOutputs *aOutputs)
{
    bool bBitStreamValid = true;
    bool bBitStreamSupported = true;

    if (aInputs->iMimeType == PVMF_MIME_WMA)
    {
        uint16 wdata;
        uint32 dwdata;
        uint32 AvgBytesPerSec;
        uint16 AdvancedEncodeOpt;

        /**** decoder header *******/
        uint8* tp = aInputs->inPtr;
        LoadWORD(wdata , tp);

        switch (wdata)
        {
                // WMA Lossless
            case WAVE_FORMAT_WMAUDIO_LOSSLESS:
            {
                if (aInputs->inBytes < 36)
                {
                    bBitStreamValid = false;
                    break;
                }

                LoadWORD(wdata, tp);
                aOutputs->Channels        = wdata;
                LoadDWORD(dwdata, tp);
                aOutputs->SamplesPerSec   = dwdata;
                LoadDWORD(dwdata, tp);
                AvgBytesPerSec  = dwdata;

                tp = aInputs->inPtr + 14;
                LoadWORD(wdata , tp);
                // Round up to the byte to get the container size
                aOutputs->BitsPerSample = 8 * ((wdata + 7) / 8);

                // Has V3 specific info
                tp = aInputs->inPtr + 34;
                LoadWORD(wdata , tp);
                AdvancedEncodeOpt = wdata;

#if !defined(BUILD_WMALSL)
                bBitStreamSupported = false;
#endif	// BUILD_WMALSL	

                // more limits according to the current PV WMA implementation
                // do not supoprt multi-channel
                if (aOutputs->Channels > 2)
                {
                    bBitStreamSupported = false;
                    break;
                }

                // do not support 24-bit
                if	(aOutputs->BitsPerSample > 16)
                {
                    bBitStreamSupported = false;
                    break;
                }
            }
            break;

            // WMA Pro, Pro+
            case WAVE_FORMAT_WMAUDIO3:
            {
                if (aInputs->inBytes < 36)
                {
                    bBitStreamValid = false;
                    break;
                }

                LoadWORD(wdata, tp);
                aOutputs->Channels        = wdata;
                LoadDWORD(dwdata, tp);
                aOutputs->SamplesPerSec   = dwdata;
                LoadDWORD(dwdata, tp);
                AvgBytesPerSec  = dwdata;

                tp = aInputs->inPtr + 14;
                LoadWORD(wdata , tp);	//iValidBitsPerSample
                // Round up to the byte to get the container size
                aOutputs->BitsPerSample = 8 * ((wdata + 7) / 8);

                // Has V3 specific info
                tp = aInputs->inPtr + 34;
                LoadWORD(wdata , tp);
                AdvancedEncodeOpt = wdata;

#if !defined(BUILD_WMAPRO)
                bBitStreamSupported = false;
                break;
#endif	// BUILD_WMAPRO

#if !defined(BUILD_WMAPROPLUS)
                if (0 != (AdvancedEncodeOpt & ENCOPT4_PLUSVER))
                {
                    bBitStreamSupported = false;
                    break;
                }
#endif	// BUILD_WMAPROPLUS

                // more limits according to the current PV WMA implementation
                // do not supoprt multi-channel
                if (aOutputs->Channels > 2)
                {
                    bBitStreamSupported = false;
                    break;
                }

                // do not support 24-bit
                if	(aOutputs->BitsPerSample > 16)
                {
                    bBitStreamSupported = false;
                    break;
                }

#if defined(WMAPLUS_64KBPS_PROFILE_ONLY)
                // limit to M0-profile bitrate and sampling rate
                if (AvgBytesPerSec > 192000 || aOutputs->SamplesPerSec > 48000)
                {
                    bBitStreamSupported = false;
                    break;
                }

                // only decode PLUSV1 (not including PLUSV1ALT)
                if	(ENCOPT4_PLUSV1 != (AdvancedEncodeOpt & ENCOPT4_PLUSVER))
                {
                    bBitStreamSupported = false;
                    break;
                }
#endif // WMAPLUS_64KBPS_PROFILE_ONLY
            }
            break;

            //WMA Standard
            case WAVE_FORMAT_WMAUDIO2:
            {
                if (aInputs->inBytes < 28)
                {
                    bBitStreamValid = false;
                    break;
                }

                LoadWORD(wdata, tp);
                aOutputs->Channels        = wdata;
                LoadDWORD(dwdata, tp);
                aOutputs->SamplesPerSec   = dwdata;

                tp = aInputs->inPtr + 14;
                LoadWORD(wdata , tp);
                aOutputs->BitsPerSample = wdata;

                if (aOutputs->SamplesPerSec > 48000)
                {
                    // not a valid sample rate for WMA Std spec
                    bBitStreamValid = false;
                    break;
                }
                if (aOutputs->Channels > 2)
                {
                    // not a valid number of channels for the WMA Std spec
                    bBitStreamValid = false;
                    break;
                }

                if (aOutputs->BitsPerSample != 16)
                {
                    // not a valid number of bits per sample for the WMA Std spec
                    bBitStreamValid = false;
                    break;
                }

#if !defined(BUILD_WMASTD)
                bBitStreamSupported = false;
#endif // BUILD_WMASTD
            }
            break;

            //WMA Standard (bitstream v1)
            case WAVE_FORMAT_MSAUDIO1:
            {
                if (aInputs->inBytes < 22)
                {
                    bBitStreamValid = false;
                    break;
                }

                tp = aInputs->inPtr +  4;
                LoadDWORD(dwdata, tp);
                aOutputs->SamplesPerSec = dwdata;
                tp = aInputs->inPtr +  2;
                LoadWORD(wdata , tp);
                aOutputs->Channels = wdata;

                tp = aInputs->inPtr + 14;
                LoadWORD(wdata , tp);
                aOutputs->BitsPerSample = wdata;

                if (aOutputs->SamplesPerSec > 48000)
                {
                    // not a valid sample rate for WMA Std spec
                    bBitStreamValid = false;
                    break;
                }
                if (aOutputs->Channels > 2)
                {
                    // not a valid number of channels for the WMA Std spec
                    bBitStreamValid = false;
                    break;
                }

                if (aOutputs->BitsPerSample != 16)
                {
                    // not a valid number of bits per sample for the WMA Std spec
                    bBitStreamValid = false;
                    break;
                }

#if !defined(BUILD_WMASTD)
                bBitStreamSupported = false;
#endif // BUILD_WMASTD
            }
            break;

            // WMA Voice
            case WAVE_FORMAT_MSSPEECH:
            {
                if (aInputs->inBytes < 18)
                {
                    bBitStreamValid = false;
                    break;
                }

                tp = aInputs->inPtr +  4;
                LoadDWORD(dwdata, tp);
                aOutputs->SamplesPerSec  = dwdata;
                tp = aInputs->inPtr +  8;
                LoadDWORD(dwdata, tp);
                //AvgBytesPerSec = dwdata;
                tp = aInputs->inPtr + 12;
                LoadWORD(wdata , tp);
                //BlockAlign     = wdata;
                tp = aInputs->inPtr +  2;
                LoadWORD(wdata , tp);
                aOutputs->Channels       = wdata;
                tp = aInputs->inPtr +  14;
                LoadWORD(wdata , tp);
                aOutputs->BitsPerSample  = wdata;

#if !defined(BUILD_WMAVOICE)
                bBitStreamSupported = false;
#endif // BUILD_WMAVOICE
            }
            break;

            case WAVE_FORMAT_WMASPDIF:
            case WAVE_FORMAT_WMAUDIO2_ES:
            case WAVE_FORMAT_WMAUDIO3_ES:
            case WAVE_FORMAT_WMAUDIO_LOSSLESS_ES:
            {
                // these formats aren't supported
                bBitStreamSupported = false;
            }
            break;

            default:
            {
                // invalid wma format
                bBitStreamValid = false;
            }
        }
    }
    else if (aInputs->iMimeType == PVMF_MIME_MPEG4_AUDIO || // AAC
             aInputs->iMimeType == PVMF_MIME_3640 ||
             aInputs->iMimeType == PVMF_MIME_LATM ||
             aInputs->iMimeType == PVMF_MIME_ADIF ||
             aInputs->iMimeType == PVMF_MIME_ASF_MPEG4_AUDIO ||
             aInputs->iMimeType == PVMF_MIME_AAC_SIZEHDR)
    {
        int32   bytes_consumed, status;
        uint8   aAudioObjectType, SamplingRateIndex;
        uint32  NumChannels;

        bytes_consumed = (int32)aInputs->inBytes;

        status = GetActualAacConfig(aInputs->inPtr,
                                    &aAudioObjectType,
                                    &bytes_consumed,
                                    &SamplingRateIndex,
                                    &NumChannels);

        aOutputs->Channels = (uint16)NumChannels;
        if (aOutputs->Channels > 2)
        {
            // not a valid number of channels for the AAC
            bBitStreamValid = false;
        }

    }

    if (!bBitStreamValid)
    {
        bBitStreamSupported = false;
    }

    return (bBitStreamSupported ? 1 : 0);
}
