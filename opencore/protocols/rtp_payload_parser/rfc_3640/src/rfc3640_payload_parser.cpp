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
///////////////////////////////////////////////////////////////////////////////
//
// RFC3640_payload_parser.cpp
//
// Implementation of payload parser for RFC3640 RTP format.
//
///////////////////////////////////////////////////////////////////////////////

#include "oscl_mem.h"
#include "rfc3640_payload_parser.h"
#include "bitstreamparser.h"
#include "oscl_byte_order.h"
#include "rfc3640_media_info.h"

//Until the fileformat node and decoder are re-worked to handle multiple-fragments
//per media message, we'll need to use this switch to control the output of the
//payload parser
#define RFC3640_ONE_FRAGMENT_PER_MEDIA_MSG 1

//Default Values
#define AAC_HBR_SIZELENGTH_DEFAULT_VALUE        13
#define AAC_HBR_INDEXLENGTH_DEFAULT_VALUE       3
#define AAC_HBR_INDEXDELTALENGTH_DEFAULT_VALUE  3
#define AAC_HBR_CTSDELTALENGTH_DEFAULT_VALUE    0
#define AAC_HBR_DTSDELTALENGTH_DEFAULT_VALUE    0
#define AAC_HBR_HEADERSLENGTH_DEFAULT_VALUE     16
#define AAC_HBR_AUXDATASIZELENGTH_DEFAULT_VALUE 0

///////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF RFC3640PayloadParser::RFC3640PayloadParser()
{
    //Default to no headers at all.
    //This is just in case Init() is never called.
    headersPresent = false;
    headersLength = 0;
    sizeLength = 0;
    indexLength = 0;
    indexDeltaLength = 0;
    CTSDeltaLength = 0;
    DTSDeltaLength = 0;
    randomAccessIndication = false;
    auxDataSizeLength = 0;
}

OSCL_EXPORT_REF RFC3640PayloadParser::~RFC3640PayloadParser()
{
}

///////////////////////////////////////////////////////////////////////////////
//
// Initialization
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF bool RFC3640PayloadParser::Init(mediaInfo* config)
{
    Oscl_Vector<PayloadSpecificInfoTypeBase*, OsclMemAllocator> payloadInfo;
    payloadInfo = config->getPayloadSpecificInfoVector();
    PayloadSpecificInfoTypeBase* payloadInfoBase = payloadInfo[0]; // is this "0" assumption okay???
    RFC3640PayloadSpecificInfoType* r3640PayloadInfo =
        OSCL_STATIC_CAST(RFC3640PayloadSpecificInfoType*, payloadInfoBase);
    rfc3640_mediaInfo *r3640m = OSCL_STATIC_CAST(rfc3640_mediaInfo *, config);
    bool retVal = false;

    //TODO: Implement other modes (such as CELP-cbr, CELP-vbr, AAC-lbr, etc...).
    sizeLength             = r3640PayloadInfo->getSizeLength();
    indexLength            = r3640PayloadInfo->getIndexLength();
    indexDeltaLength       = r3640PayloadInfo->getIndexDeltaLength();
    CTSDeltaLength         = r3640PayloadInfo->getCTSDeltaLength();
    DTSDeltaLength         = r3640PayloadInfo->getDTSDeltaLength();
    randomAccessIndication = false;

    //We support AAC-hbr mode only.
    if (!oscl_strncmp(r3640m->getMode(), "AAC-hbr", oscl_strlen("AAC-hbr")))
    {
        headersPresent         = true;
        headersLength          = AAC_HBR_HEADERSLENGTH_DEFAULT_VALUE;
        auxDataSizeLength      = AAC_HBR_AUXDATASIZELENGTH_DEFAULT_VALUE;

        //do sanity check on values. Only supporting AAC_HBR default fixed values.
        if ((sizeLength != AAC_HBR_SIZELENGTH_DEFAULT_VALUE) ||
                (indexLength != AAC_HBR_INDEXLENGTH_DEFAULT_VALUE) ||
                (indexDeltaLength != AAC_HBR_INDEXDELTALENGTH_DEFAULT_VALUE))
        {
            retVal = true;
        }

    }
    else
    {
        retVal = true;
    }


    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
//
// Payload parsing
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PayloadParserStatus
RFC3640PayloadParser::Parse(const Payload& inputPacket,
                            Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    // Many of OsclRefCounterMemFrag's member functions
    //should be const functions so this casting away constness is not necessary.
    Payload& input = const_cast<Payload&>(inputPacket);

    //@TODO: Implement AU de-interleaving

    Payload out;

    out.stream       = inputPacket.stream;
    out.marker       = inputPacket.marker;
    out.randAccessPt = inputPacket.randAccessPt;
    out.sequence     = inputPacket.sequence + 1;
    out.timestamp    = inputPacket.timestamp;
    //Creating a boolean for checking whether RFC3640_ONE_FRAGMENT_PER_MEDIA_MSG is defined or not
    bool rfc3640_one_fragement_per_media	 = false;

#ifndef RFC3640_ONE_FRAGMENT_PER_MEDIA_MSG
    rfc3640_one_fragement_per_media = true;
#endif

    // Many functions calls inside this for loop may Leave because of an
    // overflow.
    int32 err;
    OSCL_TRY(err,
             //Loop through all of the packets.
             for (uint32 fragmentNumber = 0; fragmentNumber < inputPacket.vfragments.size(); fragmentNumber++)
{
    //
    // Strip RFC 3640 header section and auxiliary section.
    //

    //Establish a pointer to the payload fragment for iterating past the
    //header and aux section.
    BitStreamParser fragment((uint8*)input.vfragments[fragmentNumber].getMemFragPtr(),
                             input.vfragments[fragmentNumber].getMemFragSize());

        //In some cases, such as with fixed-length AUs, the header is not present.
        //Only process the header if it is present.
        uint16 headersLength = 0;
        if (headersPresent)
        {
            //Read the header length from the first 2 octets.
            //Include the size of this headersLength field in the total length.
            headersLength = fragment.ReadUInt16() + BITS_PER_UINT16;
        }

        uint8  accessUnits  = 0; //controls index field interpretation (index or delta)
        uint32 size         = 0; //size of the AU
        uint32 index        = 0; //used for interleaving
        uint32 indexDelta   = 0; //used for interleaving
        int    CTSFlag      = 0; //composition time stamp
        uint32 CTSDelta     = 0; //composition time stamp
        int    DTSFlag      = 0; //decoding time stamp
        uint32 DTSDelta     = 0; //decoding time stamp
        int    RAPFlag      = 0; //random access point - used to mark a key frame
        uint32 auxDataSize  = 0;

        //Loop through all of the access units in the packet.
        while (fragment.BitsRead() < headersLength)
        {
            //If the header is present, parse it.
            if (headersLength)
            {
                if (0 != sizeLength)
                {
                    //Read the AU Size field.
                    size = fragment.ReadBits(sizeLength);
                }

                //If the index field is present...
                if (0 != indexLength)
                {
                    //If this is the first access unit header...
                    if (0 == accessUnits)
                    {
                        //The AU index only occurs in the first header.
                        index = fragment.ReadBits(indexLength);
                    }
                    else
                    {
                        indexDelta = fragment.ReadBits(indexDeltaLength);
                    }
                }

                //From RFC3640: "The CTS-flag field MUST be present in each AU-header
                //               if the length of the CTS-delta field is signaled to
                //               be larger than zero."
                if (0 != CTSDeltaLength)
                {
                    CTSFlag = fragment.ReadBits(1);
                    if (CTSFlag)
                    {
                        CTSDelta = fragment.ReadBits(CTSDeltaLength);
                    }
                }

                if (0 != DTSDeltaLength)
                {
                    DTSFlag = fragment.ReadBits(1);
                    if (DTSFlag)
                    {
                        DTSDelta = fragment.ReadBits(DTSDeltaLength);
                    }
                }

                if (randomAccessIndication)
                {
                    RAPFlag = fragment.ReadBits(1);
                }
            }


            if (rfc3640_one_fragement_per_media == true)
            {
                // At this time the decoder cannot handle multiple fragments.

                OsclMemoryFragment memfrag;
                //memfrag.ptr = NULL; //Unknown at this time.
                memfrag.len = size;
                input.vfragments[fragmentNumber].getRefCounter()->addRef();
                OsclRefCounterMemFrag refCntMemFrag(memfrag,
                                                    input.vfragments[fragmentNumber].getRefCounter(),
                                                    memfrag.len);
                out.vfragments.push_back(refCntMemFrag);
            }
            else
            {
                // Instead of creating multiple fragments, point to the first fragment, but
                // increment the size field to span all access units. The decoder is still
                // getting multiple frames in memory, but the data structure makes it appear as just one.
                // This only works for non-interleaved access units & can only be a temporary solution
                if (accessUnits == 0)
                {
                    OsclMemoryFragment memfrag;
                    memfrag.ptr = (uint8*)(input.vfragments[fragmentNumber].getMemFragPtr()) + (headersLength / 8);
                    memfrag.len = input.vfragments[fragmentNumber].getMemFragSize() - (headersLength / 8);
                    input.vfragments[fragmentNumber].getRefCounter()->addRef();
                    OsclRefCounterMemFrag refCntMemFrag(memfrag,
                                                        input.vfragments[fragmentNumber].getRefCounter(),
                                                        memfrag.len);
                    out.vfragments.push_back(refCntMemFrag);
                }
            }

            accessUnits++;
        }

        //Processed the header.  Skip past any padding.
        if (fragment.GetBitPos() != MOST_SIG_BIT)
        {
            fragment.NextBits(fragment.GetBitPos() + 1);
        }

        //Now skip over the aux data region.
        if (auxDataSizeLength)
        {
            auxDataSize = fragment.ReadBits(auxDataSizeLength);
            if (auxDataSize)
            {
                //Skip over the aux data region.
                fragment.NextBits(auxDataSize);
                //Skip past any padding.
                if (fragment.GetBitPos() != MOST_SIG_BIT)
                {
                    fragment.NextBits(fragment.GetBitPos() + 1);
                }
            }
        }

        if (rfc3640_one_fragement_per_media == true)
        {
            //Update the fragment pointers with real values now that we have
            //parsed the headers.
            //The output vector may contain fragments from previous runs, so
            //start with the last fragments that we pushed on the back of the vector.
            for (uint32 i = (out.vfragments.size() - accessUnits); i < out.vfragments.size(); i++)
            {
                out.vfragments[i].getMemFrag().ptr = fragment.GetBytePos();
                fragment.NextBits(out.vfragments[i].getMemFrag().len * BITS_PER_BYTE);
            }
        }

    }
            ); // End of OSCL_TRY

    if (err != OsclErrNone)
    {
        return PayloadParserStatus_Failure;
    }
    vParsedPayloads.push_back(out);

    return PayloadParserStatus_Success;
}

///////////////////////////////////////////////////////////////////////////////
//
// Repositioning related
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF void RFC3640PayloadParser::Reposition(const bool adjustSequence, const uint32 stream, const uint32 seqnum)
{
    OSCL_UNUSED_ARG(adjustSequence);
    OSCL_UNUSED_ARG(stream);
    OSCL_UNUSED_ARG(seqnum);
}

OSCL_EXPORT_REF uint32 RFC3640PayloadParser::GetMinCurrTimestamp()
{
    return 0;
}
