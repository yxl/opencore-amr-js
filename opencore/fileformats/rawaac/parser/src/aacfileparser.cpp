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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                 A A C   F I L E   P A R S E R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/**
 *  @file aacfileparser.cpp
 *  @brief This file contains the implementation of the raw AAC file parser.
 */

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "aacfileparser.h"
#include "pv_audio_type_defs.h"
#include "getactualaacconfig.h"
#include "media_clock_converter.h"
#include "pvmp4audiodecoder_api.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()

//! specially used in ResetPlayback(), re-poition the file pointer
int32 AACBitstreamObject::reset(int32 filePos)
{
    iBytesRead  = filePos; // set the initial value
    iBytesProcessed = filePos;
    if (ipAACFile)
    {
        ipAACFile->Seek(filePos, Oscl_File::SEEKSET);
    }
    iPos = AACBitstreamObject::MAIN_BUFF_SIZE;
    return refill();
}

//! read data from bitstream, this is the only function to read data from file
int32 AACBitstreamObject::refill()
{
    if (iBytesRead > 0 && iFileSize > 0 && iBytesRead >= iFileSize)
    {
        //At this point we're near the end of data.
        //Quit reading data but don't return EOF until all data is processed.
        if (iBytesProcessed < iBytesRead)
        {
            return AACBitstreamObject::EVERYTHING_OK;
        }
        else
        {
            return AACBitstreamObject::END_OF_FILE;
        }
    }

    if (!ipAACFile)
    {
        PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::refill- Misc Error"));
        return AACBitstreamObject::MISC_ERROR;
    }

    // Get file size at the very first time
    if (iFileSize == 0)
    {
        if (ipAACFile->Seek(0, Oscl_File::SEEKSET))
        {
            PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::refill- Misc Error"));
            return AACBitstreamObject::MISC_ERROR;
        }
        ipAACFile->GetRemainingBytes((uint32&)iFileSize);
        if (iFileSize <= 0)
        {
            PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::refill- Misc Error"));
            return AACBitstreamObject::MISC_ERROR;
        }
        // first-time read, set the initial value of iPos
        iPos = AACBitstreamObject::MAIN_BUFF_SIZE;
    }

    int32 remain_bytes = AACBitstreamObject::MAIN_BUFF_SIZE - iPos;
    if (remain_bytes > 0)
    {
        // move the remaining stuff to the beginning of iBuffer
        oscl_memcpy(&iBuffer[0], &iBuffer[iPos], remain_bytes);
    }

    // read data
    if ((iActual_size = ipAACFile->Read(&iBuffer[remain_bytes], 1, iMax_size - remain_bytes)) == 0)
    {
        return AACBitstreamObject::READ_ERROR;
    }

    iBytesRead += iActual_size;
    iActual_size += remain_bytes;
    iPos = 0;

    return AACBitstreamObject::EVERYTHING_OK;
}

//! get frame size and number of data blocks for the next frame, in preparation of getNextFrame()
int32 AACBitstreamObject::getNextFrameInfo(int32& frame_size, int32& numDateBlocks)
{
    int32 ret_value = AACBitstreamObject::EVERYTHING_OK;

    // Need to refill?
    if (iFileSize == 0 || iPos + 4 >= iActual_size)
    {
        ret_value = refill();
        if (ret_value)
        {
            return ret_value;
        }
    }

    // Search the 12 bit sync-word 0xff + 0xf0
    while (!(iBuffer[iPos] == 0xff &&
             (iBuffer[iPos+1] & 0xf0) == 0xf0 &&
             iBuffer[iPos+1] == iAACHeaderBuffer[1] &&
             iBuffer[iPos+2] == iAACHeaderBuffer[2] &&
             iBuffer[iPos+3] == iAACHeaderBuffer[3]))
    {
        if (iPos + 4 >= iActual_size)
        {
            ret_value = refill();
            if (ret_value)
            {
                return ret_value;
            }
        }
        iPos++;
        iBytesProcessed++;
    }
    if (iPos + ADTS_HEADER_LENGTH + 2 >= iActual_size)
    {
        ret_value = refill();
        if (ret_value)
        {
            return ret_value;
        }
    }

    // parse the header and get frame size plus the number of data blocks
    uint8 *pBuffer = &iBuffer[iPos];
    frame_size = (int32)((pBuffer[3] & 0x03) << 11)   | // take the lowest 2 bits in the 4th byte of the header
                 (int32)(pBuffer[4] << 3)         |     // take the whole 5th byte(8 bits) of the header
                 (int32)((pBuffer[5] & 0xe0) >> 5);     // take the highest 3 bits in the 6th byte of the header

    if (ibCRC_Check)
    {
        frame_size -= (ADTS_HEADER_LENGTH + 2);
    }
    else
    {
        frame_size -= ADTS_HEADER_LENGTH;
    }

    numDateBlocks = (pBuffer[6] & 0x03) + 1; // take the lowest 2 bits in the 7th byte of the header

    // update CRC check
    ibCRC_Check = ((pBuffer[1] & 0x01) ? false : true);

    return ret_value;
}

//! most important function to get one frame data plus frame type, used in getNextBundledAccessUnits()
int32 AACBitstreamObject::getNextFrame(uint8* frameBuffer, int32& frame_size, int32& aHdrSize, bool bHeaderIncluded)
{
    aHdrSize = 0;
    if (!frameBuffer || (frame_size <= 0 || frame_size > MAX_AAC_FRAME_SIZE))
    {
        PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getNextFrame- Misc Error"));
        return AACBitstreamObject::MISC_ERROR;
    }

    int32 ret_value = AACBitstreamObject::EVERYTHING_OK;

    if (iAACFormat == EAACADTS) //ADTS
    {
        // Need to refill?
        if (iFileSize == 0 || iPos + ADTS_HEADER_LENGTH + 2 + frame_size >= iActual_size)
        {
            ret_value = refill();
            if (ret_value)
            {
                return ret_value;
            }
        }

        // copy the header to the buffer if needed
        uint8 *pBuffer = &iBuffer[iPos];
        int32 header_size = (ibCRC_Check ? ADTS_HEADER_LENGTH + 2 : ADTS_HEADER_LENGTH);
        if (bHeaderIncluded)
        {
            oscl_memcpy(frameBuffer, pBuffer, header_size);
            frameBuffer += header_size;
            aHdrSize = header_size;
        }

        // copy the data to the given buffer
        oscl_memcpy(frameBuffer, &pBuffer[header_size], frame_size);
        iPos += (header_size + frame_size);
        iBytesProcessed += (header_size + frame_size);
    }

    else if (iAACFormat == EAACADIF || iAACFormat == EAACRaw) //ADIF or raw bitstream
    {
        if (iFileSize == 0 || iPos + frame_size >= iActual_size)
        {
            ret_value = refill();
            if (ret_value)
            {
                return ret_value;
            }
        }

        uint8 *pBuffer = &iBuffer[iPos];
        if ((iBytesRead >= iFileSize) && (frame_size > (iBytesRead - iBytesProcessed)))
        {
            frame_size = iBytesRead - iBytesProcessed;
        }

        // copy the data to the given buffer
        oscl_memcpy(frameBuffer, pBuffer, frame_size);
        iPos += frame_size;
        iBytesProcessed += frame_size;
    }

    return ret_value;
}

void AACBitstreamObject::parseID3Header(PVFile& aFile)
{
    if (!ipAACFile
            || !ipAACFile->IsOpen())
        return;//error

    int32 curpos = aFile.Tell();
    aFile.Seek(0, Oscl_File::SEEKSET);
    id3Parser->ParseID3Tag(&aFile);
    aFile.Seek(curpos, Oscl_File::SEEKSET);
}

int32 AACBitstreamObject::isAACFile()
{
    if (!ipAACFile || !ipAACFile->IsOpen())
    {
        return AACBitstreamObject::READ_ERROR;
    }

    int32 retVal = AACBitstreamObject::EVERYTHING_OK;

    // save current file pointer location
    int32 curPos = ipAACFile->Tell();
    ipAACFile->Seek(0, Oscl_File::SEEKSET);

    // check for ID3v2 tag at the beginning of file
    uint32 tagSize = 0;
    if (true == id3Parser->IsID3V2Present(ipAACFile, tagSize))
    {
        // skip over ID3v2 tag
        // move the file read pointer to beginning of audio data
        // but first make sure the data is there
        uint32 aCurrentSize = 0;
        uint32 aRemBytes = 0;
        if (ipAACFile->GetRemainingBytes(aRemBytes))
        {
            uint32 currPos_2 = (uint32)(ipAACFile->Tell());
            aCurrentSize = currPos_2 + aRemBytes;

            if (aCurrentSize >= tagSize)
            {
                if (ipAACFile->Seek(tagSize, Oscl_File::SEEKSET) != 0)
                {
                    retVal = AACBitstreamObject::READ_ERROR;
                }
            }
            else
            {
                // data has not arrived yet
                retVal = AACBitstreamObject::INSUFFICIENT_DATA;
            }
        }
        else
        {
            retVal = AACBitstreamObject::READ_ERROR;
        }
    }

    // file pointer should be at the audio data
    // can be ADTS, ADIF or raw AAC
    // - it takes 8192 bytes (max AAC frame size) to determine that is is ADTS
    // - it takes 4 bytes to determine that it is ADIF
    // - it takes 1536 bytes to determine that it is raw AAC

    // make sure there are at least 8192 bytes left in the file
    if (AACBitstreamObject::EVERYTHING_OK == retVal)
    {
        uint32 aRemBytes = 0;
        if (ipAACFile->GetRemainingBytes(aRemBytes))
        {
            int32 currPos_2 = ipAACFile->Tell();
            retVal = reset(currPos_2);

            if (AACBitstreamObject::EVERYTHING_OK == retVal)
            {
                uint8 *pBuffer = &iBuffer[iPos];

                // default to not enough data
                retVal = AACBitstreamObject::INSUFFICIENT_DATA;
                if (aRemBytes >= MAX_ADTS_PACKET_LENGTH)
                {
                    // check for possible ADTS sync word
                    int32 index = find_adts_syncword(pBuffer);
                    if (index != -1)
                    {
                        // definitely ADTS
                        retVal = AACBitstreamObject::EVERYTHING_OK;
                    }
                }

                if (AACBitstreamObject::INSUFFICIENT_DATA == retVal)
                {
                    // not enough data to determine if it is ADTS
                    // see if it is ADIF
                    if (aRemBytes >= 4)
                    {
                        // check for ADIF header
                        if (pBuffer[0] == 0x41 &&  // 'A'
                                pBuffer[1] == 0x44 &&  // 'D'
                                pBuffer[2] == 0x49 &&  // 'I'
                                pBuffer[3] == 0x46)    // 'F'
                        {
                            // definitely ADIF
                            retVal = AACBitstreamObject::EVERYTHING_OK;
                        }
                        else
                        {
                            // non-recognized format,
                            retVal = AACBitstreamObject::MISC_ERROR;
                        }
                    }
                }
            }
        }
        else
        {
            retVal = AACBitstreamObject::READ_ERROR;
        }
    }

    // retore file pointer
    ipAACFile->Seek(curPos, Oscl_File::SEEKSET);
    return retVal;
}


//! Search for adts synchronization word

int32 AACBitstreamObject::find_adts_syncword(uint8 *pBuffer)
{


    uint32 test_for_syncword = 0;



    uint32 i;
    uint32 buff_length;


    buff_length = OSCL_MIN(MAX_ADTS_PACKET_LENGTH, iActual_size);


    for (i = 0; i < buff_length - 1; i++)
    {
        /*
         *  Sync word is always byte aligned, sync word == 0xFFF
         */
        if (pBuffer[i] == 0xFF)
        {
            if ((pBuffer[i+1] & 0xF0) == 0xF0)
            {
                test_for_syncword = 1;  /*  flag found sync adts word */
                break;
            }
        }
    }

    if (test_for_syncword)    // get here only if match is found
    {
        uint32 index = i;



























        // extract the aac frame lenght where the next sync word should be
        if ((index + 5) < buff_length)
        {
            uint32 length = (uint32)((pBuffer[index +  3] & 0x03) << 11); //  lowest 2 bits in the 4th byte of the header
            length |= (uint32)(pBuffer[index +  4] << 3);            //  whole 5th byte(8 bits) of the header
            length |= (uint32)((pBuffer[index +  5] & 0xe0) >> 5);   //  highest 3 bits in the 6th byte of the header


            // Verify for false sync word in non-adts files, by validating next 2 sync word
            if ((length > ADTS_HEADER_LENGTH)           &&
                    (length < buff_length - index)          &&    // buff_length should account for up to 4 aac frames
                    (pBuffer[index + length    ] == 0xff)  &&
                    ((pBuffer[index + length + 1] & 0xf0) == 0xf0))
            {
                // extract the second aac frame lenght to predict next adts header
                length += index;
                uint32 length_2 = (uint32)((pBuffer[length +  3] & 0x03) << 11);
                length_2 |= (uint32)(pBuffer[length +  4] << 3);
                length_2 |= (uint32)((pBuffer[length+  5] & 0xe0) >> 5);


                if ((length_2 > ADTS_HEADER_LENGTH)       &&
                        (length_2 < buff_length - length)     &&
                        (pBuffer[length + length_2] == 0xff) &&
                        ((pBuffer[length + length_2 + 1] & 0xf0) == 0xf0))

                {

                    return (index);  // sure this is adts
                }
                iPosSyncAdtsFound = index - 1; // allow overlapping on extended search
                return (-1);  // false adts synchronization, it is adif or raw
            }
            else
            {
                iPosSyncAdtsFound = index - 1; // allow overlapping on extended search
                return (-1);    // it was a false adts synchronization, it could be adif or raw
            }
        }
        else
        {
            iPosSyncAdtsFound = index - 1; // allow overlapping on extended search
            return (-1);    // not enough data to validate synchronization
        }
    }
    else
    {
        iPosSyncAdtsFound = i - 1; // allow overlapping on extended search
        return (-1);    // no adts synchronization, it could be adif or raw
    }
}

//! get clip information: file size, format(ADTS or ADIF) and sampling rate index
int32 AACBitstreamObject::getFileInfo(int32& fileSize, TAACFormat& format, uint8& sampleFreqIndex, uint32& bitRate, uint32& HeaderLen, OSCL_wString&/*aClip*/)
{
    uint32 id;
    uint32 bitstreamType;
    uint32 numProgConfigElem;
    int32 i;
    uint32 ADIFHeaderLen;
    uint32 numFrontChanElem;
    uint32 numSideChanElem;
    uint32 numBackChanElem;
    uint32 numLfeChanElem;
    uint32 numAssocDataElem;
    uint32 numValidCCElem;
    uint32 commentFieldBytes;
    uint32 offset;
    uint32 bitIndex;
    uint8 tmp;

    bitRate = 0;
    HeaderLen = 0;
    iChannelConfig = 0;

    format = EAACUnrecognized;
    fileSize = sampleFreqIndex = 0;
    int32 ret_value = AACBitstreamObject::EVERYTHING_OK;

    if (iFileSize == 0)
    {
        ret_value = reset(0);
        if (ret_value == AACBitstreamObject::EVERYTHING_OK)
        {
            //retrieve the ID3 meta-data from the file
            parseID3Header(*ipAACFile);

            if (id3Parser->GetByteOffsetToStartOfAudioFrames() > 0)
            {
                //skip past the ID3 header.
                ret_value = reset(id3Parser->GetByteOffsetToStartOfAudioFrames());
                if (ret_value != AACBitstreamObject::EVERYTHING_OK)
                {
                    return ret_value;
                }
            }

            fileSize = iFileSize;

            uint8 *pBuffer = &iBuffer[iPos];

            // Check for possible adts sync word
            int32 index = find_adts_syncword(pBuffer);
            if (index != -1)
            {
                //ADTS format
                iAACFormat = format = EAACADTS;
                // get sample frequency index
                iSampleFreqIndex = sampleFreqIndex = (uint8)((pBuffer[2 + index] & 0x3c) >> 2);

                // check the crc_check field
                ibCRC_Check = ((pBuffer[1 + index] & 0x01) ? false : true);

                // get ADTS fixed header part to iAACHeaderBuffer
                oscl_memcpy(iAACHeaderBuffer, &pBuffer[index], PACKET_INDICATOR_LENGTH);

            }
            else if (iFileSize >= 4     &&
                     pBuffer[0] == 0x41 &&  // 'A'
                     pBuffer[1] == 0x44 &&  // 'D'
                     pBuffer[2] == 0x49 &&  // 'I'
                     pBuffer[3] == 0x46)    // 'F'
            {
                // ADIF format
                iAACFormat = format = EAACADIF;

                ADIFHeaderLen = 32; // adif_id 4 bytes

                // check copyright_id_present (1 bit)
                id = pBuffer[4] & 0x80;

                if (id != 0)    //copyright ID is presented
                {
                    ADIFHeaderLen += 75;    // copyright_id_present 1 bit
                    // copyright_id 72 bits,
                    // original_copy 1 bit,
                    // home 1 bit,

                    // check bitstream type
                    bitstreamType = pBuffer[13] & 0x10;

                    // get number of program config. element
                    numProgConfigElem = (pBuffer[16] & 0x1E) >> 1;

                    // get bitrate (max for variable rate bitstream)
                    iBitrate = bitRate = ((pBuffer[13] & 0xF0) << 15) |
                                         (pBuffer[14] << 11) |
                                         (pBuffer[15] << 3)  |
                                         ((pBuffer[16] & 0xE0) >> 5);

                    if (iBitrate == 0)   //bitrate is not known
                    {
                        PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
                        return AACBitstreamObject::MISC_ERROR;
                    }

                    ADIFHeaderLen += 28;    // bitstream_type 1 bit,
                    // bitrate 23 bits
                    // num_program_config_elements 4 bits

                    for (i = 0; i < (int32)numProgConfigElem + 1; i++)
                    {
                        if (bitstreamType == 0) //bistream type is constant rate bitstream
                        {
                            ADIFHeaderLen += 20;    //adif_buffer_fullness 20 bits

                            // get audio object type
                            iAudioObjectType = (uint8)(((pBuffer[19] & 0x1) << 1) | ((pBuffer[20] & 0x80) >> 7));

                            // get sampling rate index
                            iSampleFreqIndex = sampleFreqIndex = (uint8)((pBuffer[20] & 0x78) >> 3);

                            // get number of front channel elements
                            numFrontChanElem = (uint32)(((pBuffer[20] & 0x7) << 1) | ((pBuffer[21] & 0x80) >> 7));

                            // get number of side channel elements
                            numSideChanElem = (uint32)((pBuffer[21] & 0x78) >> 3);

                            // get number of back channel elements
                            numBackChanElem = (uint32)(((pBuffer[21] & 0x7) << 1) | ((pBuffer[22] & 0x80) >> 7));

                            // get number of LFE channel elements
                            numLfeChanElem = (uint32)((pBuffer[22] & 0x60) >> 5);

                            // get number of assoc data elements
                            numAssocDataElem = (uint32)((pBuffer[22] & 0x1C) >> 2);

                            // get number of valid CC elements
                            numValidCCElem = (uint32)(((pBuffer[22] & 0x3) << 2) | ((pBuffer[23] & 0xC0) >> 6));

                            ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                            //object_type 2 bits,
                            //sampling_frequency_index 4 bits,
                            //num_front_channel_elements 4 bits,
                            //num_side_channel_elements 4 bits,
                            //num_back_channel_elements 4 bits,
                            //num_lfe_channel_elements 2 bits,
                            //num_assoc_data_elements 3 bits,
                            //num_valid_cc_elements 4 bits

                            // check mono_mixdown_present
                            if ((pBuffer[23] & 0x20) != 0)  //mono mixdown is presented
                            {
                                ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                                //mono_mixdown_element_number 4 bits

                                //check stereo_mixdown_present
                                if ((pBuffer[23] & 0x1) != 0)   //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[24] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[24] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[23] & 0x1) != 0)
                            }
                            else    //mono mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                                //check stereo_mixdown_present
                                if ((pBuffer[23] & 0x10) != 0)  //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[24] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[23] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[23] & 0x10) != 0)

                            }//if ((pBuffer[23] & 0x20) != 0)

                        }
                        else    //bistream type is variable rate bitstream
                        {
                            // get audio object type
                            iAudioObjectType = (uint8)((pBuffer[17] & 0x18) >> 3);

                            // get sampling rate index
                            iSampleFreqIndex = sampleFreqIndex = (uint8)(((pBuffer[17] & 0x7) << 1) | ((pBuffer[18] & 0x80) >> 7));

                            // get number of front channel elements
                            numFrontChanElem = (uint32)((pBuffer[18] & 0x78) >> 3);

                            // get number of side channel elements
                            numSideChanElem = (uint32)(((pBuffer[18] & 0x7) << 1) | ((pBuffer[19] & 0x80) >> 7));

                            // get number of back channel elements
                            numBackChanElem = (uint32)((pBuffer[19] & 0x78) >> 3);

                            // get number of LFE channel elements
                            numLfeChanElem = (uint32)((pBuffer[19] & 0x6) >> 1);

                            // get number of assoc data elements
                            numAssocDataElem = (uint32)(((pBuffer[19] & 0x1) << 2) | ((pBuffer[20] & 0xC0) >> 6));

                            // get number of valid CC elements
                            numValidCCElem = (uint32)((pBuffer[20] & 0x3C) >> 2);

                            ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                            //object_type 2 bits,
                            //sampling_frequency_index 4 bits,
                            //num_front_channel_elements 4 bits,
                            //num_side_channel_elements 4 bits,
                            //num_back_channel_elements 4 bits,
                            //num_lfe_channel_elements 2 bits,
                            //num_assoc_data_elements 3 bits,
                            //num_valid_cc_elements 4 bits

                            // check mono_mixdown_present
                            if ((pBuffer[20] & 0x2) != 0)   //mono mixdown is presented
                            {
                                ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                                //mono_mixdown_element_number 4 bits

                                //check stereo_mixdown_present
                                if ((pBuffer[21] & 0x10) != 0)  //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[22] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[21] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[21] & 0x10) != 0)
                            }
                            else    //mono mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                                //check stereo_mixdown_present
                                if ((pBuffer[20] & 0x1) != 0)   //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[21] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[21] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[20] & 0x1) != 0)

                            }//if ((pBuffer[20] & 0x2) != 0)

                        }// if (bitstreamType == 0)

                        for (i = 0; i < (int32)numFrontChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //front channel element takes 1 bit
                            iChannelConfig += tmp;

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //front_element_is_cpe[i] 1 bit,
                            //front_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numSideChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //side channel element takes 1 bit
                            iChannelConfig += (tmp + 1);

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                            //side_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numBackChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //back channel element takes 1 bit
                            iChannelConfig += (tmp + 1);

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //back_element_is_cpe[i] 1 bit,
                            //back_element_tag_select[i] 4 bits
                        }

                        if (numLfeChanElem != 0)
                        {
                            iChannelConfig++;   //1 front low frequency effects speaker
                        }

                        for (i = 0; i < (int32)numLfeChanElem; i++)
                        {
                            ADIFHeaderLen += 4; //lfe_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numAssocDataElem; i++)
                        {
                            ADIFHeaderLen += 4; //assoc_data_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numValidCCElem; i++)
                        {
                            ADIFHeaderLen += 5; //cc_element_is_ind_sw[i] 1 bit,
                            //valid_cc_element_tag_select[i] 4 bits
                        }

                        // byte_allignment
                        ADIFHeaderLen += 7;
                        ADIFHeaderLen &= 0xF8;

                        // comment_field_bytes (8 bits)
                        offset = ADIFHeaderLen >> 3;
                        bitIndex = ADIFHeaderLen & 0x7;
                        commentFieldBytes = (pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex));

                        ADIFHeaderLen += 8; //comment_field_bytes 8 bits

                        for (i = 0; i < (int32)commentFieldBytes; i++)
                        {
                            ADIFHeaderLen += 8; //comment_field_data 8 bits
                        }

                    }// for (i = 0; i < (int32)numProgConfigElem + 1; i++)

                }
                else    //copyright ID is not presented
                {
                    ADIFHeaderLen += 3;     // copyright_id_present 1 bit
                    // original_copy 1 bit,
                    // home 1 bit,

                    // check bitstream type
                    bitstreamType = pBuffer[4] & 0x10;

                    // get number of program config. element
                    numProgConfigElem = (pBuffer[7] & 0x1E) >> 1;

                    // get bitrate (max for variable rate bitstream)
                    iBitrate  = bitRate = ((pBuffer[4] & 0xF0) << 15) |
                                          (pBuffer[5] << 11) |
                                          (pBuffer[6] << 3)  |
                                          ((pBuffer[7] & 0xE0) >> 5);

                    if (iBitrate == 0)  //bitrate is not known
                    {
                        PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
                        return AACBitstreamObject::MISC_ERROR;
                    }

                    ADIFHeaderLen += 28;    // bitstream_type 1 bit,
                    // bitrate 23 bits
                    // num_program_config_elements 4 bits

                    for (i = 0; i < (int32)numProgConfigElem + 1; i++)
                    {
                        if (bitstreamType == 0) //bistream type is constant rate bitstream
                        {
                            ADIFHeaderLen += 20;    //adif_buffer_fullness 20 bits

                            // get audio object type
                            iAudioObjectType = (uint8)(((pBuffer[10] & 0x1) << 1) | ((pBuffer[11] & 0x80) >> 7));

                            // get sampling rate index
                            iSampleFreqIndex = sampleFreqIndex = (uint8)((pBuffer[11] & 0x78) >> 3);

                            // get number of front channel elements
                            numFrontChanElem = (uint32)(((pBuffer[11] & 0x7) << 1) | ((pBuffer[12] & 0x80) >> 7));

                            // get number of side channel elements
                            numSideChanElem = (uint32)((pBuffer[12] & 0x78) >> 3);

                            // get number of back channel elements
                            numBackChanElem = (uint32)(((pBuffer[12] & 0x7) << 1) | ((pBuffer[13] & 0x80) >> 7));

                            // get number of LFE channel elements
                            numLfeChanElem = (uint32)((pBuffer[13] & 0x60) >> 5);

                            // get number of assoc data elements
                            numAssocDataElem = (uint32)((pBuffer[13] & 0x1C) >> 2);

                            // get number of valid CC elements
                            numValidCCElem = (uint32)(((pBuffer[13] & 0x3) << 2) | ((pBuffer[14] & 0xC0) >> 6));

                            ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                            //object_type 2 bits,
                            //sampling_frequency_index 4 bits,
                            //num_front_channel_elements 4 bits,
                            //num_side_channel_elements 4 bits,
                            //num_back_channel_elements 4 bits,
                            //num_lfe_channel_elements 2 bits,
                            //num_assoc_data_elements 3 bits,
                            //num_valid_cc_elements 4 bits

                            // check mono_mixdown_present
                            if ((pBuffer[14] & 0x20) != 0)  //mono mixdown is presented
                            {
                                ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                                //mono_mixdown_element_number 4 bits

                                //check stereo_mixdown_present
                                if ((pBuffer[14] & 0x1) != 0)   //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[15] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[15] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[14] & 0x1) != 0)
                            }
                            else    //mono mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                                //check stereo_mixdown_present
                                if ((pBuffer[14] & 0x10) != 0)  //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[15] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[14] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[14] & 0x10) != 0)

                            }//if ((pBuffer[14] & 0x20) != 0)

                        }
                        else    //bistream type is variable rate bitstream
                        {
                            // get audio object type
                            iAudioObjectType = (uint8)((pBuffer[8] & 0x18) >> 3);

                            // get sampling rate index
                            iSampleFreqIndex = sampleFreqIndex = (uint8)(((pBuffer[8] & 0x7) << 1) | ((pBuffer[9] & 0x80) >> 7));

                            // get number of front channel elements
                            numFrontChanElem = (uint32)((pBuffer[9] & 0x78) >> 3);

                            // get number of side channel elements
                            numSideChanElem = (uint32)(((pBuffer[9] & 0x7) << 1) | ((pBuffer[10] & 0x80) >> 7));

                            // get number of back channel elements
                            numBackChanElem = (uint32)((pBuffer[10] & 0x78) >> 3);

                            // get number of LFE channel elements
                            numLfeChanElem = (uint32)((pBuffer[10] & 0x6) >> 1);

                            // get number of assoc data elements
                            numAssocDataElem = (uint32)(((pBuffer[10] & 0x1) << 2) | ((pBuffer[11] & 0xC0) >> 6));

                            // get number of valid CC elements
                            numValidCCElem = (uint32)((pBuffer[11] & 0x3C) >> 2);

                            ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                            //object_type 2 bits,
                            //sampling_frequency_index 4 bits,
                            //num_front_channel_elements 4 bits,
                            //num_side_channel_elements 4 bits,
                            //num_back_channel_elements 4 bits,
                            //num_lfe_channel_elements 2 bits,
                            //num_assoc_data_elements 3 bits,
                            //num_valid_cc_elements 4 bits

                            // check mono_mixdown_present
                            if ((pBuffer[11] & 0x2) != 0)   //mono mixdown is presented
                            {
                                ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                                //mono_mixdown_element_number 4 bits

                                //check stereo_mixdown_present
                                if ((pBuffer[12] & 0x10) != 0)  //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[13] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[12] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[12] & 0x10) != 0)
                            }
                            else    //mono mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                                //check stereo_mixdown_present
                                if ((pBuffer[11] & 0x1) != 0)   //stereo mixdown is presented
                                {
                                    ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                                    //stereo_mixdown_element_number 4 bits

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[12] & 0x8) != 0)   //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }
                                else    //stereo mixdown is not presented
                                {
                                    ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                                    //check matrix_mixdown_idx_present
                                    if ((pBuffer[12] & 0x80) != 0)  //matrix mixdown is presented
                                    {
                                        ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                        //matrix_mixdown_idx 2 bits
                                        //pseudo_surround_enable 1 bit
                                    }
                                    else    //matrix mixdown is not presented
                                    {
                                        ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                                    }
                                }//if ((pBuffer[11] & 0x1) != 0)

                            }//if ((pBuffer[11] & 0x2) != 0)

                        }// if (bitstreamType == 0)

                        for (i = 0; i < (int32)numFrontChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //front channel element takes 1 bit
                            iChannelConfig += tmp;

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //front_element_is_cpe[i] 1 bit,
                            //front_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numSideChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //side channel element takes 1 bit
                            iChannelConfig += (tmp + 1);

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                            //side_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numBackChanElem; i++)
                        {
                            //calculate channel configuration
                            offset = ADIFHeaderLen >> 3;
                            bitIndex = ADIFHeaderLen & 0x7;
                            tmp = (uint8)((pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex)));
                            tmp >>= (8 - 1); //back channel element takes 1 bit
                            iChannelConfig += (tmp + 1);

                            //update ADIF variable header length
                            ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                            //side_element_tag_select[i] 4 bits
                        }

                        if (numLfeChanElem != 0)
                        {
                            iChannelConfig++;   //1 front low frequency effects speaker
                        }

                        for (i = 0; i < (int32)numLfeChanElem; i++)
                        {
                            ADIFHeaderLen += 4; //lfe_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numAssocDataElem; i++)
                        {
                            ADIFHeaderLen += 4; //assoc_data_element_tag_select[i] 4 bits
                        }

                        for (i = 0; i < (int32)numValidCCElem; i++)
                        {
                            ADIFHeaderLen += 5; //cc_element_is_ind_sw[i] 1 bit,
                            //valid_cc_element_tag_select[i] 4 bits
                        }

                        // byte_allignment
                        ADIFHeaderLen += 7;
                        ADIFHeaderLen &= 0xF8;

                        // comment_field_bytes (8 bits)
                        offset = ADIFHeaderLen >> 3;
                        bitIndex = ADIFHeaderLen & 0x7;
                        commentFieldBytes = (pBuffer[offset] << bitIndex) | (pBuffer[offset+1] >> (8 - bitIndex));

                        ADIFHeaderLen += 8; //comment_field_bytes 8 bits

                        for (i = 0; i < (int32)commentFieldBytes; i++)
                        {
                            ADIFHeaderLen += 8; //comment_field_data 8 bits
                        }

                    }// for (i = 0; i < (int32)numProgConfigElem + 1; i++)

                } // if(id!=0)

                // ADIF header length in bits
                iADIFHeaderLen = HeaderLen = ADIFHeaderLen;

                // get ADIF id header part to iAACHeaderBuffer (4 bytes)
                oscl_memcpy(iAACHeaderBuffer, pBuffer, PACKET_INDICATOR_LENGTH);
            }
            else // Check if it is AAC raw bitstream file
            {

                int32 config_header_size = iActual_size;
                int32 status = GetActualAacConfig(pBuffer,
                                                  &iAudioObjectType, &config_header_size, &sampleFreqIndex, &iChannelConfig);
                if (status != SUCCESS) return AACBitstreamObject::MISC_ERROR;

                // Retrieve the audio object type
                if (iAudioObjectType != 2 &&
                        iAudioObjectType != 4 &&
                        iAudioObjectType != 5 &&
                        iAudioObjectType != 29)
                {
                    // Unsupported object type
                    PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
                    return AACBitstreamObject::MISC_ERROR;
                }

                iSampleFreqIndex = sampleFreqIndex;
                oscl_memcpy(iAACHeaderBuffer, pBuffer, config_header_size);
                iRawAACHeaderLen = (uint32)config_header_size;
                HeaderLen = (uint32)config_header_size << 3;

                // Raw AAC format
                iAACFormat = format = EAACRaw;
            }
        }
    }
    else
    {
        fileSize = iFileSize;
        format = iAACFormat;
        sampleFreqIndex = iSampleFreqIndex;
    }
    return ret_value;
}


//! extended search to get clip information only for adts format
int32 AACBitstreamObject::extendedAdtsSearchForFileInfo(TAACFormat& format,
        uint8& sampleFreqIndex)
{
    sampleFreqIndex = 0;
    int32 ret_value = AACBitstreamObject::EVERYTHING_OK;

    iPos = iPosSyncAdtsFound;   /* overlap only needed data */
    ret_value = refill();
    if (ret_value == AACBitstreamObject::EVERYTHING_OK)
    {

        uint8 *pBuffer = &iBuffer[iPos];

        // Check for possible adts sync word
        int32 index = find_adts_syncword(pBuffer);
        if (index != -1)
        {
            //ADTS format
            iAACFormat = format = EAACADTS;
            // get sample frequency index
            iSampleFreqIndex = sampleFreqIndex = (uint8)((pBuffer[2 + index] & 0x3c) >> 2);

            // check the crc_check field
            ibCRC_Check = ((pBuffer[1 + index] & 0x01) ? false : true);

            // get ADTS fixed header part to iAACHeaderBuffer
            oscl_memcpy(iAACHeaderBuffer, &pBuffer[index], PACKET_INDICATOR_LENGTH);

        }
        else
        {
            if (iBytesRead > ADTS_SYNC_SEARCH_LENGTH(iFileSize, 5))    // search over 3.13 % of the file
            {
                return AACBitstreamObject::MISC_ERROR;  // break the loop
            }
            //otherwise keep looking
        }

    }
    return ret_value;
}

int32 AACBitstreamObject::getDecoderConfigHeader(uint8* headerBuffer)
{
    if (iAACFormat == EAACADTS) //ADTS
    {
        if (!headerBuffer ||
                !(iAACHeaderBuffer[0] == 0xff &&
                  (iAACHeaderBuffer[1] & 0xf0) == 0xf0))
        {
            PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
            return AACBitstreamObject::MISC_ERROR;
        }

        // parse the ADTS header of the first frame: iAACHeaderReferenceBuffer[] (got from the bitstream in InitAACFile()
        headerBuffer[0] = headerBuffer[1] = 0;

        // 1. construct auditoObjectType(bit1-5, 5 bits, MPEG-4) from profile(bit17-18, 2 bits, ADTS)
        uint8 profile = (uint8)(iAACHeaderBuffer[2] >> 6); // 2 bits long
        headerBuffer[0] |= ++profile << 3;   // put it into the highest 5 bits

        // 2. construct samplingFrequencyIndex(bit6-9, 4 bits, MPEG-4) from sampling_frequency_index(bit19-22, 4 bits, ADTS)
        uint8 sampling_frequency_index = (uint8)((iAACHeaderBuffer[2] >> 2) & 0x0f); // 4 bits long
        headerBuffer[0] |= (sampling_frequency_index >> 1);      // put 3 bits
        headerBuffer[1] |= ((sampling_frequency_index & 0x1) << 7); // put 1 bit

        // 3. construct channelConfiguration(bit10-13, 4 bits, MPEG-4) from channel_configuration(bit22-24, 3 bits, ADTS)
        uint8 channel_config = (uint8)(((iAACHeaderBuffer[2] & 0x01) << 2) |
                                       (iAACHeaderBuffer[3] >> 6));    // 3 bits long in ADTS => 4 valid bits in MPEG-4
        headerBuffer[1] |= (channel_config << 3);
    }
    else if (iAACFormat == EAACADIF) //ADIF
    {
        if (!headerBuffer ||
                !((iAACHeaderBuffer[0] == 0x41) &&  // 'A'
                  (iAACHeaderBuffer[1] == 0x44) &&  // 'D'
                  (iAACHeaderBuffer[2] == 0x49) &&  // 'I'
                  (iAACHeaderBuffer[3] == 0x46)))   // 'F'
        {
            PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
            return AACBitstreamObject::MISC_ERROR;
        }

        // parse the ADIF header
        headerBuffer[0] = headerBuffer[1] = 0;

        // 1. construct audio object type (2 bits)
        headerBuffer[0] |= ((iAudioObjectType + 1) << 3);

        // 2. construct sampling frequency index (4 bits)
        headerBuffer[0] |= (iSampleFreqIndex >> 1); //put 3 bits in headerBuffer[0]
        headerBuffer[1] |= ((iSampleFreqIndex & 0x1) << 7); //put 1 bit in headerBuffer[1]

        // 3. construct channel configuration (4 bits)
        headerBuffer[1] |= ((iChannelConfig + 1) << 3);
    }
    else if (iAACFormat == EAACRaw)
    {
        // Just copy the audio specific config
        oscl_memcpy(headerBuffer, iAACHeaderBuffer, iRawAACHeaderLen);
    }

    return AACBitstreamObject::EVERYTHING_OK;
}

template <class PtrType>
class AutoPtrArrayContainer
{
    private:
        bool _Ownership;
        PtrType *_Ptr;

    public:
        // default constructors
        explicit AutoPtrArrayContainer(PtrType *inPtr = 0) :
                _Ownership(inPtr != 0), _Ptr(inPtr)   {};

        ~AutoPtrArrayContainer()
        {
            if (_Ownership && _Ptr)
            {
                OSCL_ARRAY_DELETE(_Ptr);
                _Ptr = NULL;
            }
            _Ownership = false;
        }
};


//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::CAACFileParser
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  Constructor for CAACFileParser class
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF CAACFileParser::CAACFileParser(void)
{
    iAACDuration = 0;
    iAACFormat   = EAACUnrecognized;
    iLogger = PVLogger::GetLoggerObject("pvaacparser");
    iDiagnosticLogger = PVLogger::GetLoggerObject("playerdiagnostics.pvaac_parser");

    iTotalNumFramesRead = 0;
    iEndOfFileReached = false;
    ipBSO = NULL;
    iFirstTime = false;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::~CAACFileParser
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  Destructor for CAACFileParser class
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF CAACFileParser::~CAACFileParser(void)
{

    if (iAACFile.IsOpen())
    {
        iAACFile.Close();
    }
    if (ipBSO != NULL)
    {
        PV_AAC_FF_DELETE(NULL, AACBitstreamObject, ipBSO);
        ipBSO = NULL;

    }

}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::InitAACFile
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aClip = pointer to the AAC file name to be played of type TPtrC
//
//  Outputs:
//    None
//
//  Returns:
//    returnValue = true if the init succeeds, else false.
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function opens the AAC file, checks for AAC format type, calculates
//  the track duration, and sets the AAC bitrate value.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF bool CAACFileParser::InitAACFile(OSCL_wString& aClip,  bool aInitParsingEnable, Oscl_FileServer* iFileSession, PVMFCPMPluginAccessInterfaceFactory* aCPMAccess, OsclFileHandle*aHandle)
{
    uint32 bitRateValue;
    uint32 HeaderLenValue;

    iAACFile.SetCPM(aCPMAccess);
    iAACFile.SetFileHandle(aHandle);

    //For aac overwritte pvfile default settings in order to prevent
    //audio artifacts when playing files from MMC
    //use native cache (if supported by the platform) size 32KB.
    PVFileCacheParams cacheParams;
    cacheParams.iCacheSize = 32768; //32K
    cacheParams.iNativeAccessMode = 1;

    iAACFile.SetFileCacheParams(cacheParams);

    // Open the file (aClip)
    if (iAACFile.Open(aClip.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *iFileSession) != 0)
    {
        return false;
    }

    // create ipBSO
    PV_AAC_FF_NEW(NULL, AACBitstreamObject, (&iAACFile), ipBSO);

    if (!ipBSO)
    {
        return false;
    }
    if (ipBSO->get())
    {
        return false;
    }

    // get file info: file size, format, sampling rate
    uint8 sampleFreqTableValue;
    TAACFormat format;
    if (ipBSO->getFileInfo(iAACFileSize, format, sampleFreqTableValue, bitRateValue, HeaderLenValue, aClip))
    {
        return false;
    }

    if (format == EAACADTS)
    {
        // ADTS format
        iAACFormat = EAACADTS;
        if (sampleFreqTableValue >= 16)
            return false;
        iAACSampleFrequency = ADTSSampleFreqTable[(uint16)sampleFreqTableValue];
        if (iAACSampleFrequency == -1)
            return false;
    }
    else if (format == EAACADIF)
    {
        // ADIF format
        iAACFormat = EAACADIF;
        //ADIF and ADTS uses the same sampling frequency lookup table
        if (sampleFreqTableValue >= 16)
            return false;
        iAACSampleFrequency = ADTSSampleFreqTable[(uint16)sampleFreqTableValue];
        if (iAACSampleFrequency == -1)
            return false;
        //bitrate and variable header length for ADIF only
        iAACBitRate = (int32)bitRateValue;
        iAACHeaderLen = (int32)HeaderLenValue;
        iFirstTime = true;
    }
    else if (format == EAACRaw)
    {
        // AAC raw format
        iAACFormat = EAACRaw;
        // Use the same sampling frequency lookup table
        if (sampleFreqTableValue >= 16)
            return false;
        iAACSampleFrequency = ADTSSampleFreqTable[(uint16)sampleFreqTableValue];
        if (iAACSampleFrequency == -1)
            return false;
        //variable header length for raw only
        iAACHeaderLen = (int32)HeaderLenValue;
        iFirstTime = true;
    }
    else
    {
        iAACFormat = EAACUnrecognized;
        return false;
    }

    // Determine file duration and set up random positioning table if needed
    if (aInitParsingEnable)
    {
        if (iAACFormat == EAACADTS)
        {
            // fully go through each frame to calculate duration
            int32 status = AACBitstreamObject::EVERYTHING_OK;
            int32 frame_size = 0;
            int32 numBlocks;


            uint8 *maxAACFrameBuffer = OSCL_ARRAY_NEW(uint8, MAX_AAC_FRAME_SIZE); // MAX_AAC_FRAME_SIZE=8192
            if (!maxAACFrameBuffer)
            {
                return false;
            }
            AutoPtrArrayContainer<uint8> autoPtr(maxAACFrameBuffer);

            iAACDuration = 0;
            int32 filePos = 0;
            iRPTable.push_back(filePos);

            while (status == AACBitstreamObject::EVERYTHING_OK)
            {
                // get the next frame
                status = ipBSO->getNextFrameInfo(frame_size, numBlocks);
                if (status == AACBitstreamObject::END_OF_FILE)
                {
                    break;
                }
                else if (status == AACBitstreamObject::EVERYTHING_OK && frame_size > 0)
                {
                    int32 hdrSize = 0;
                    status = ipBSO->getNextFrame(maxAACFrameBuffer, frame_size, hdrSize);

                    if (status == AACBitstreamObject::EVERYTHING_OK)
                    {
                        // calulate the number of frames
                        iAACDuration ++;

                        // set up the table for randow positioning
                        int32 frame_length = frame_size + ADTS_HEADER_LENGTH + (ipBSO->isCRCEnabled() ? 2 : 0);
                        filePos += frame_length;
                        iRPTable.push_back(filePos);
                    }
                    else if (status == AACBitstreamObject::END_OF_FILE)
                    {
                        break;
                    }
                    else
                    {
                        // error happens!
                        PVMF_AACPARSER_LOGERROR((0, "Error InitAacfile"));
                        return false;

                    }
                }
                else
                {
                    // error happens!
                    PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::InitAACFile:Error in getting next frame "));
                    return false;
                }
            }

            // get the duration in millisec
            MediaClockConverter mcc;
            mcc.set_timescale(iAACSampleFrequency);
            mcc.set_clock(iAACDuration*1024, 0);
            iAACDuration = mcc.get_converted_ts(1000);

            if (ipBSO->reset(0))
                return false;
        }
        else if (iAACFormat == EAACADIF)
        {
            int32 raw_data_bits;

            iAACDuration = 0;
            iRPTable.push_back(iAACDuration);

            raw_data_bits = iAACFileSize * 8 - iAACHeaderLen;

            // rough duration calculation based on max bitrate for variable rate bitstream
            MediaClockConverter mcc;
            mcc.set_timescale(iAACBitRate);
            mcc.set_clock(raw_data_bits, 0);
            iAACDuration = mcc.get_converted_ts(1000);

            if (ipBSO->reset(ipBSO->GetByteOffsetToStartOfAudioFrames() + (iAACHeaderLen >> 3)))
                return false;
        }
        else if (iAACFormat == EAACRaw)
        {
            iAACDuration = 0;
            iRPTable.push_back(iAACDuration);

            // Duration cannot be calculated
            int32 hdrsize = iAACHeaderLen >> 3;
            if (iAACHeaderLen&0x7)
            {
                ++hdrsize;
            }
            if (ipBSO->reset(ipBSO->GetByteOffsetToStartOfAudioFrames() + hdrsize))
            {
                PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::InitAACFile:Error Bitstream object failed to reset"));
                return false;
            }
        }

    } // end of: if(aInitParsingEnable)


    return true;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::RetrieveFileInfo
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//
//  Outputs:
//    None
//
//  Returns:
//    false if an error happens, else true
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function opens the AAC file, checks for AAC format type, calculates
//  the track duration, and sets the AAC bitrate value.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF bool CAACFileParser::RetrieveFileInfo(TPVAacFileInfo& aInfo)
{
    if (iAACFormat == EAACUnrecognized)
    {
        // File is not open and parsed
        return false;
    }

    aInfo.iTimescale = 1000;
    aInfo.iDuration = iAACDuration;
    aInfo.iSampleFrequency = iAACSampleFrequency;
    aInfo.iBitrate = iAACBitRate;
    aInfo.iFormat = iAACFormat;
    aInfo.iFileSize = iAACFileSize;
    PVMF_AACPARSER_LOGDIAGNOSTICS((0, "CAACFileParser::RetrieveFileInfo- duration = %d, bitrate = %d, filesize = %d", iAACDuration, iAACBitRate, iAACFileSize));

    return true;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::RetrieveID3Info
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//		aID3MetaData: pointer to data structure for the output.
//
//  Outputs:
//    None
//
//  Returns:
//    false if an error happens, else true
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function retrieves the ID3 data if any was found in the file.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF bool CAACFileParser::RetrieveID3Info(PvmiKvpSharedPtrVector& aID3MetaData)
{
    if (ipBSO)
    {
        ipBSO->ID3MetaData(aID3MetaData);
        if (aID3MetaData.size() > 0)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

OSCL_EXPORT_REF bool CAACFileParser::IsID3Frame(const OSCL_String &frameType)
{
    return ipBSO->IsID3Frame(frameType);
}

OSCL_EXPORT_REF void CAACFileParser::GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame)
{
    ipBSO->GetID3Frame(aFrameType, aFrame);
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::ResetPlayback
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aStartTime = integer value as where to move the playback positioning to.
//
//  Outputs:
//    None
//
//  Returns:
//    0 if success, -1 if failure
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function sets the file pointer to the location that aStartTime would
//  point to in the file.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------

OSCL_EXPORT_REF int32 CAACFileParser::ResetPlayback(uint32 aStartTime, uint32& aActualStartTime)
{
    // Check if the file is opened
    int32 result;
    if (!iAACFile.IsOpen())
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::ResetPlayback- Misc error-"));
        return AACBitstreamObject::MISC_ERROR;
    }

    // initialize "iTotalNumFramesRead"
    // +1 means we choose the next frame(ts>=aStartTime)
    //iTotalNumFramesRead = aStartTime/(1024000/iAACSampleFrequency);
    iTotalNumFramesRead = (int32)((OsclFloat)aStartTime * iAACSampleFrequency / 1024000.0) + (aStartTime > 0) * 1;
    PVMF_AACPARSER_LOGDIAGNOSTICS((0, "CAACFileParser::resetplayback - TotalNumFramesRead=%d", iTotalNumFramesRead));

    if (iAACFormat == EAACADTS)
    {
        // set new file position
        int32 newPosition = 0;
        if (iAACDuration != 0 && iRPTable.size() <= 0)
        {
            newPosition = (iAACFileSize * aStartTime) / iAACDuration;
            PVMF_AACPARSER_LOGDIAGNOSTICS((0, "CAACFileParser::resetplayback - newPosition=%d", newPosition));
            if (newPosition < 0)
            {
                newPosition = 0; // if we have no duration information, reset the file position at 0.
            }
        }
        else if (iRPTable.size() > 0)
        {
            // use the randow positioning table to determine the file position
            if (iTotalNumFramesRead > (int32)iRPTable.size())
            {
                iTotalNumFramesRead = ((int32)iRPTable.size()) - 2;
            }
            newPosition = iRPTable[iTotalNumFramesRead];
        }
        result = ipBSO->reset(ipBSO->GetByteOffsetToStartOfAudioFrames() + newPosition);
        if (newPosition >= 0 && result)
        {
            PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::ResetPlayback- Misc error-"));
            return result;
        }
        iEndOfFileReached = false;
    }
    else if (iAACFormat == EAACADIF)
    {
        int32 newPosition = (iAACHeaderLen >> 3);

        if (newPosition >= 0 && ipBSO->reset(ipBSO->GetByteOffsetToStartOfAudioFrames() + newPosition))
        {
            PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::ResetPlayback- Misc error-"));
            return AACBitstreamObject::MISC_ERROR;
        }
        iEndOfFileReached = false;
        iFirstTime = true;
        iTotalNumFramesRead = 0; // For ADIF reposition to time 0 always
    }
    else if (iAACFormat == EAACRaw)
    {
        int32 newPosition = (iAACHeaderLen >> 3);
        if (iAACHeaderLen&0x7)
        {
            ++newPosition;
        }

        if (newPosition >= 0 && ipBSO->reset(ipBSO->GetByteOffsetToStartOfAudioFrames() + newPosition))
        {
            PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::ResetPlayback- Misc error-"));
            return AACBitstreamObject::MISC_ERROR;
        }
        iEndOfFileReached = false;
        iFirstTime = true;
        iTotalNumFramesRead = 0; // For AAC raw bitstream reposition to time 0 always
    }

    MediaClockConverter mcc;
    mcc.set_timescale(iAACSampleFrequency);
    mcc.set_clock(iTotalNumFramesRead*1024, 0);
    aActualStartTime = mcc.get_converted_ts(1000);

    PVMF_AACPARSER_LOGDIAGNOSTICS((0, "CAACFileParser::resetplayback - aActualStartTime=%d", aActualStartTime));
    return AACBitstreamObject::EVERYTHING_OK;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::SeekPointFromTimestamp
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aStartTime = integer value as for the specified start time
//
//  Outputs:
//    None
//
//  Returns:
//    Timestamp in milliseconds of the actual position
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function returns the timestamp for an actual position corresponding
//  to the specified start time
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF uint32 CAACFileParser::SeekPointFromTimestamp(uint32 aStartTime)
{
    // Check if the file is opened
    if (!iAACFile.IsOpen())
    {
        return 0;
    }

    // Determine the frame number corresponding to timestamp
    // note: +1 means we choose the next frame(ts>=aStartTime)
    uint32 startframenum = (int32)((OsclFloat)aStartTime * iAACSampleFrequency / 1024000.0) + (aStartTime > 0) * 1;

    // Correct the frame number if necessary
    if (iAACFormat == EAACADTS)
    {
        // set new file position
        if (iAACDuration != 0 && iRPTable.size() <= 0)
        {
            // Duration not known and reposition table not available so go to first frame
            startframenum = 0;
        }
        else if (iRPTable.size() > 0)
        {
            if (startframenum >= iRPTable.size())
            {
                // Requesting past the end of table so set to (end of table-1)
                // to be at the last sample
                startframenum = ((int32)iRPTable.size()) - 2;
            }
        }
    }
    else if (iAACFormat == EAACADIF || iAACFormat == EAACRaw)
    {
        // Not enough info in file to reposition so go to time 0
        return 0;
    }

    return (int32)((OsclFloat)startframenum*1024000.0 / iAACSampleFrequency);
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::GetNextBundledAccessUnits
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aNumSamples = requested number of frames to be read from file
//    aGau        = frame information structure of type GAU
//
//  Outputs:
//    None
//
//  Returns:
//    0 if success, -1 if failure
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function attempts to read in the number of AAC frames specified by
//  aNumSamples. It formats the read data to WMF bit order and stores it in
//  the GAU structure.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF int32
CAACFileParser::GetNextBundledAccessUnits(uint32 *aNumSamples,
        GAU *aGau,
        bool bADTSHeaderIncluded)
{
    // AMR format has already been identified in InitAMRFile function.
    // Check if AMR format is valid as the safeguard
    if (iAACFormat == EAACUnrecognized)
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::GetNextBundledAccessUnits- Misc error-"));
        return AACBitstreamObject::MISC_ERROR;
    }

    int32 returnValue = AACBitstreamObject::EVERYTHING_OK;

    if (iAACFormat == EAACADTS)
    {
        if (iEndOfFileReached)
        {
            *aNumSamples = 0;
            return AACBitstreamObject::END_OF_FILE;
        }

        uint8* pTempGau = (uint8 *)aGau->buf.fragments[0].ptr;
        uint32 gauBufferSize = aGau->buf.fragments[0].len;
        uint32 i, bytesReadInGau = 0, numSamplesRead = 0;
        int32 frame_size = 0, numBlocks;
        int32 prev_iTotalNumFramesRead = iTotalNumFramesRead; // BX

        for (i = 0; i < *aNumSamples && !iEndOfFileReached; i++)
        {
            // get the frame size and number of data blocks
            returnValue = ipBSO->getNextFrameInfo(frame_size, numBlocks);
            PVMF_AACPARSER_LOGDIAGNOSTICS((0, "CAACFileParser::GetNextBundledAccessUnits - frame_size=%d", frame_size));
            if (returnValue == AACBitstreamObject::END_OF_FILE)
            {
                iEndOfFileReached = true;
                break;
            }

            else if (returnValue == AACBitstreamObject::EVERYTHING_OK && frame_size > 0)
            {
                // Check whether the gau buffer will be overflow or the requested number
                // of samples will be met
                if (bytesReadInGau + frame_size >= gauBufferSize ||
                        numSamplesRead + numBlocks > *aNumSamples)
                    break;

                // At this point, we can get the next frame
                int32 hdrSize = 0;
                returnValue = ipBSO->getNextFrame(pTempGau, frame_size, hdrSize, bADTSHeaderIncluded);
                if (returnValue == AACBitstreamObject::END_OF_FILE)
                {
                    iEndOfFileReached = true;
                    break;
                }
                else if (returnValue == AACBitstreamObject::EVERYTHING_OK)
                {
                    // update infomation
                    pTempGau += frame_size;
                    bytesReadInGau += frame_size;
                    aGau->info[i].len = frame_size + hdrSize;
                    aGau->info[i].ts  = (int32)(((OsclFloat)(iTotalNumFramesRead++) * 1024000.0) / (OsclFloat)iAACSampleFrequency); // BX
                    numSamplesRead += numBlocks;
                }
                else
                { // error happens!!
                    PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::GetNextBundledAccessUnits- Read error-"));
                    *aNumSamples = 0;
                    return AACBitstreamObject::READ_ERROR;
                }
            }
            else
            {
                // error happens!!
                PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::GetNextBundledAccessUnits- Read error-"));
                *aNumSamples = 0;
                return AACBitstreamObject::READ_ERROR;
            }
        } // end of: for(i = 0; i < *aNumSamples && !iEndOfFileReached; i++)

        aGau->info[0].ts = (int32)(((OsclFloat)prev_iTotalNumFramesRead * 1024000.0) / (OsclFloat)iAACSampleFrequency); // BX
        *aNumSamples = numSamplesRead;
    }

    else if (iAACFormat == EAACADIF || iAACFormat == EAACRaw)
    {
        *aNumSamples = 0;

        uint8* pTempGau = (uint8 *)aGau->buf.fragments[0].ptr;  //point to the current data location in StreamingBuffer
        uint32 gauBufferSize = aGau->buf.fragments[0].len;
        uint32 bytesReadInGau = 0;
        uint32 bytestoread = 0;
        uint32 i;

        if (iFirstTime == true)
        {
            // read from file to get two guranteed AAC decoder frames in streaming buffer
            for (i = 0; i < 2; i++)  //AAC Streaming Buffer Max Data Size is 15359 (9*1536 + 1535)
            {
                // At this point, we can get the next frame
                if (bytesReadInGau >= gauBufferSize)
                {
                    // No more buffer space to read
                    break;
                }
                bytestoread = gauBufferSize - bytesReadInGau;
                if (bytestoread > AAC_DECODER_INPUT_BUFF_SIZE)
                {
                    bytestoread = AAC_DECODER_INPUT_BUFF_SIZE;
                }
                int32 hdrSize = 0;
                returnValue = ipBSO->getNextFrame(pTempGau, (int32 &)bytestoread, hdrSize, bADTSHeaderIncluded);

                if (returnValue == AACBitstreamObject::END_OF_FILE)
                {
                    iEndOfFileReached = true;
                    break;
                }
                else if (returnValue == AACBitstreamObject::EVERYTHING_OK)
                {
                    // update infomation
                    pTempGau += bytestoread;
                    aGau->info[i].len = bytestoread;
                    aGau->info[i].ts = 0; // First block of AAC data read has timestamp 0
                    bytesReadInGau += bytestoread;
                    *aNumSamples += 1;
                }
                else
                {
                    // error happens!!
                    PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::GetNextBundledAccessUnits- Read error-"));
                    return AACBitstreamObject::READ_ERROR;
                }
            }
            iFirstTime = false;
        }
        else    // read from file to get one guranteed AAC decoder frame in streaming buffer
        {
            // At this point, we can get the next frame
            bytestoread = gauBufferSize;
            if (bytestoread > AAC_DECODER_INPUT_BUFF_SIZE)
            {
                bytestoread = AAC_DECODER_INPUT_BUFF_SIZE;
            }

            int32 hdrSize = 0;
            returnValue = ipBSO->getNextFrame(pTempGau, (int32 &)bytestoread, hdrSize, bADTSHeaderIncluded);

            if (returnValue == AACBitstreamObject::END_OF_FILE)
            {
                iEndOfFileReached = true;
            }
            else if (returnValue == AACBitstreamObject::EVERYTHING_OK)
            {
                // update infomation
                aGau->info[0].len = bytestoread;
                aGau->info[0].ts = 0xFFFFFFFF; // Unknown timestamp
//				aGau->info[0].ts=(int32)(((OsclFloat)(iTotalNumFramesRead++) * 1024000.0) / (OsclFloat)iAACSampleFrequency); // BX
                bytesReadInGau += bytestoread;
                *aNumSamples += 1;
            }
            else
            {
                // error happens!!
                PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::GetNextBundledAccessUnits- Read error-"));
                return AACBitstreamObject::READ_ERROR;
            }
        }
    }

    //We may have reached EOF but also found some samples.
    //don't return EOF until there are no samples left.
    if (returnValue == AACBitstreamObject::END_OF_FILE
            && *aNumSamples > 0)
    {
        return AACBitstreamObject::EVERYTHING_OK;
    }

    return returnValue;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::PeekNextTimestamp
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    return = status EVERYTHING_OK.
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function returns the next timestamp without processing any data.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF int32 CAACFileParser::PeekNextTimestamp(uint32& ts)
{
    MediaClockConverter mcc;
    mcc.set_timescale(iAACSampleFrequency);
    mcc.set_clock((iTotalNumFramesRead + 1)*1024, 0);
    ts = mcc.get_converted_ts(1000);

    return AACBitstreamObject::EVERYTHING_OK;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::GetTrackDecoderSpecificInfoSize
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    return = AAC_DECODER_SPECIFIC_INFO_SIZE
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function returns the value defined by AAC_DECODER_SPECIFIC_INFO_SIZE.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF int32 CAACFileParser::GetTrackDecoderSpecificInfoSize(void)
{
    if (iAACFormat == EAACRaw)
    {
        int32 infosize = iAACHeaderLen >> 3;
        if (iAACHeaderLen&0x7)
        {
            ++infosize;
        }
        return infosize;
    }
    else
    {
        return AAC_DECODER_SPECIFIC_INFO_SIZE;
    }
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::GetTrackDecoderSpecificInfoContent
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    return = address to location of iAACFrameBuffer[PACKET_INDICATOR_LENGTH]
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function populates iAACFrameBuffer with the two byte of decoder
//  specific info and returns the address to the values.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF uint8* CAACFileParser::GetTrackDecoderSpecificInfoContent(void)
{
    iAACFrameBuffer[PACKET_INDICATOR_LENGTH]     = 0x12;
    iAACFrameBuffer[PACKET_INDICATOR_LENGTH + 1] = 0x10;

    ipBSO->getDecoderConfigHeader(&iAACFrameBuffer[PACKET_INDICATOR_LENGTH]);
    return &iAACFrameBuffer[PACKET_INDICATOR_LENGTH];
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAACFileParser::GetAACFormat
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    return = iAACFormat
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function returns iAACFormat value.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF TAACFormat CAACFileParser::GetAACFormat(void)
{
    return iAACFormat;
}

OSCL_EXPORT_REF  PVID3Version CAACFileParser::GetID3Version() const
{
    return ipBSO->GetID3Version();
}

OSCL_EXPORT_REF ParserErrorCode CAACFileParser::getAACHeaderLen(OSCL_wString& aClip,
        bool aInitParsingEnable,
        Oscl_FileServer* iFileSession,
        PVMFCPMPluginAccessInterfaceFactory* aCPMAccess,
        OsclFileHandle* aHandle,
        uint32* HeaderLenValue)
{
    PVFile iAACFileTemp;
    OSCL_UNUSED_ARG(aInitParsingEnable);

    iAACFileTemp.SetCPM(aCPMAccess);
    iAACFileTemp.SetFileHandle(aHandle);

    // Open the file (aClip)
    if (iAACFileTemp.Open(aClip.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *iFileSession) != 0)
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::getAACHeaderLen- File open error-"));
        return FILE_OPEN_ERROR;
    }
    AACBitstreamObject *ipBSOTemp;
    PV_AAC_FF_NEW(NULL, AACBitstreamObject, (&iAACFileTemp), ipBSOTemp);


    if (!ipBSOTemp || ipBSOTemp->get())
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::getAACHeaderLen- Memory allocation failed-"));
        // release temp storage, file control block..
        iAACFileTemp.Close();
        PV_AAC_FF_DELETE(NULL, AACBitstreamObject, ipBSOTemp);
        return MEMORY_ERROR;
    }

    uint32 bitRateValueTemp;
    uint32 tempHeaderLenValue = 0;

    int32 iAACFileSizeTemp;
    uint8 sampleFreqTableValueTemp;
    TAACFormat formatTemp;
    if (ipBSOTemp->getFileInfo(iAACFileSizeTemp, formatTemp, sampleFreqTableValueTemp, bitRateValueTemp, tempHeaderLenValue, aClip))
    {
        return FILE_OPEN_ERROR;
    }
    if (formatTemp == EAACADTS)
    {
        tempHeaderLenValue = ADTS_HEADER_LENGTH;
    }
    iAACFileTemp.Close();
    PV_AAC_FF_DELETE(NULL, AACBitstreamObject, ipBSOTemp);
    ipBSOTemp = NULL;
    *HeaderLenValue = tempHeaderLenValue;
    return OK;
}

OSCL_EXPORT_REF ParserErrorCode CAACFileParser::IsAACFile(OSCL_wString& aClip, Oscl_FileServer* aFileSession, PVMFCPMPluginAccessInterfaceFactory* aCPMAccess, OsclFileHandle* aHandle)
{
    PVFile iAACFileTemp;
    iAACFileTemp.SetCPM(aCPMAccess);
    iAACFileTemp.SetFileHandle(aHandle);

    // Open the file (aClip)
    if (iAACFileTemp.Open(aClip.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *aFileSession) != 0)
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::IsAACFile- File open error-"));
        return FILE_OPEN_ERROR;
    }

    AACBitstreamObject *ipBSOTemp;
    PV_AAC_FF_NEW(NULL, AACBitstreamObject, (&iAACFileTemp), ipBSOTemp);

    if (!ipBSOTemp || ipBSOTemp->get())
    {
        PVMF_AACPARSER_LOGERROR((0, "CAACFileParser::IsAACFile- Memory allocation failed-"));
        // release temp storage, file control block..
        iAACFileTemp.Close();
        PV_AAC_FF_DELETE(NULL, AACBitstreamObject, ipBSOTemp);
        return MEMORY_ERROR;
    }

    int32 result = ipBSOTemp->isAACFile();

    // release temp storage, file control block and the file
    iAACFileTemp.Close();
    PV_AAC_FF_DELETE(NULL, AACBitstreamObject, ipBSOTemp);
    ipBSOTemp = NULL;

    if (AACBitstreamObject::EVERYTHING_OK == result)
    {
        return OK;
    }
    else if (AACBitstreamObject::INSUFFICIENT_DATA == result)
    {
        return INSUFFICIENT_DATA;
    }
    return GENERIC_ERROR;
}

