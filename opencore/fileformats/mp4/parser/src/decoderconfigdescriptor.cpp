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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                     MPEG-4 DecoderConfigDescriptor Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/



#define IMPLEMENT_DecoderConfigDescriptor

#include "decoderconfigdescriptor.h"
#include "atomutils.h"
#include "atomdefs.h"

#include "h263decoderspecificinfo.h"
#include "amrdecoderspecificinfo.h"

// Constructor
DecoderConfigDescriptor::DecoderConfigDescriptor(MP4_FF_FILE *fp)
        : BaseDescriptor(fp)
{
    _pdecSpecificInfo = NULL;

    if (_success)
    {
        _pparent = NULL;

        if (!AtomUtils::read8(fp, _objectTypeIndication))
            _success = false;

        // Unpack stream type, upstream, and reserved
        uint8 data;
        if (!AtomUtils::read8(fp, data))
            _success = false;

        _streamType = (uint8)((data >> 2) & (uint8) 0x3f);   // 6 bits
        _upStream   = (data >> 1) & 0x1; // 1 bit
        _reserved   = (uint8)(data & (uint8) 0x1);  // 1 bit

        // Read in the other members
        if (!AtomUtils::read24(fp, _bufferSizeDB))
            _success = false;
        if (!AtomUtils::read32(fp, _maxBitrate))
            _success = false;
        if (!AtomUtils::read32(fp, _avgBitrate))
            _success = false;

        uint32 base = 1 + _sizeOfSizeField;
        uint32 bytesRead = 13 + base;


        if (_success)
        {
            if (bytesRead < _sizeOfClass)
            {
                uint8 descr_tag = AtomUtils::peekNextByte(fp);

                if (descr_tag == DECODER_SPECIFIC_INFO_TAG)
                {
                    // Assuming only one DecoderSpecificInfo object for now
                    if (_objectTypeIndication == H263_VIDEO)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, H263DecoderSpecificInfo, (fp), _pdecSpecificInfo);
                    }
                    else if (_objectTypeIndication == AMR_AUDIO)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, AMRDecoderSpecificInfo, (fp), _pdecSpecificInfo);
                    }
                    else if (_objectTypeIndication == QCELP_MP4)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, DecoderSpecificInfo, (fp), _pdecSpecificInfo);
                    }
                    else if (_objectTypeIndication == MPEG4_VIDEO)
                    {
                        //FOR MPEG4

                        PV_MP4_FF_NEW(fp->auditCB, DecoderSpecificInfo, (fp), _pdecSpecificInfo);

                        uint32 infoSize = _pdecSpecificInfo->getInfoSize();
                        uint8  *info = _pdecSpecificInfo->getInfo();

                        uint32 offSet = infoSize - 4;

                        // 00 00 01 (00-1F)
                        if (info != NULL)
                        {
                            if (info[offSet] == 0x00)
                            {
                                if (info[offSet+1] == 0x00)
                                {
                                    if (info[offSet+2] == 0x01)
                                    {
                                        if (info[offSet+3] < 0x20)
                                        {
                                            // Remove existing decoderspecific info
                                            PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, _pdecSpecificInfo);

                                            PV_MP4_FF_NEW(fp->auditCB, H263DecoderSpecificInfo, (fp, true), _pdecSpecificInfo);

                                            H263DecoderSpecificInfo *_pH263decSpecificInfo =
                                                (H263DecoderSpecificInfo *)(_pdecSpecificInfo);

                                            _pH263decSpecificInfo->_VendorCode =
                                                0;

                                            _pH263decSpecificInfo->_codec_version =
                                                0;

                                            _pH263decSpecificInfo->_codec_level =
                                                10;

                                            _pH263decSpecificInfo->_codec_profile =
                                                0;

                                            _pH263decSpecificInfo->_max_width =
                                                0;

                                            _pH263decSpecificInfo->_max_height =
                                                0;

                                            _objectTypeIndication = H263_VIDEO;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (_objectTypeIndication == MPEG4_AUDIO)
                    {
                        //FOR AAC
                        PV_MP4_FF_NEW(fp->auditCB, DecoderSpecificInfo, (fp), _pdecSpecificInfo);
                    }
                    else
                    {
                        PV_MP4_FF_NEW(fp->auditCB, DecoderSpecificInfo, (fp), _pdecSpecificInfo);
                    }
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_DECODER_CONFIG_DESCRIPTOR_FAILED;
                }
            }
            else
            {
                _pdecSpecificInfo = NULL;
            }

            if ((_streamType == STREAM_TYPE_AUDIO) ||
                    (_streamType == STREAM_TYPE_VISUAL))
            {
                // FOR AUDIO/VISUAL TRACKS, DEC SPECIFIC INFO IS MANDATORY
                if (_pdecSpecificInfo == NULL)
                {
                    _success = false;
                    _mp4ErrorCode = READ_DECODER_CONFIG_DESCRIPTOR_FAILED;
                }
            }
        }
        else
        {
            _mp4ErrorCode = READ_DECODER_CONFIG_DESCRIPTOR_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_DECODER_CONFIG_DESCRIPTOR_FAILED;
    }
}

// Destructor
DecoderConfigDescriptor::~DecoderConfigDescriptor()
{
    if (_pdecSpecificInfo != NULL)
        PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, _pdecSpecificInfo);
}




