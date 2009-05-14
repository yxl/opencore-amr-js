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
// h264_payload_parser.cpp
//
// H.264 payload parser implementation.
//
///////////////////////////////////////////////////////////////////////////////

#include "h264_payload_parser.h"
#include "h264_payload_parser_interleave_mode.h"
#include "h264_payload_parser_utility.h"
#include "h264_payload_info.h"

///////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
///////////////////////////////////////////////////////////////////////////////

// constructor
OSCL_EXPORT_REF H264PayloadParser::H264PayloadParser():
        iUtilityTable(NULL),
        iUtility(NULL),
        iInterleaveDepth(0),
        iIMP(NULL),
        iTimestampForFU(1),
        iIsFragmentedBitMask(0)
{
    ;
}

// destructor
OSCL_EXPORT_REF H264PayloadParser::~H264PayloadParser()
{
    // delete the table
    deleteParserUtilityTable();

    // delete iIMP
    OSCL_DELETE(iIMP);
    iIMP = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// Initialization
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF bool H264PayloadParser::Init(mediaInfo* config)
{
    Oscl_Vector<PayloadSpecificInfoTypeBase*, OsclMemAllocator> payloadInfo;
    payloadInfo = config->getPayloadSpecificInfoVector();
    PayloadSpecificInfoTypeBase* payloadInfoBase = payloadInfo[0]; // is this "0" assumption okay???
    H264PayloadSpecificInfoType* h264PayloadInfo =
        OSCL_STATIC_CAST(H264PayloadSpecificInfoType*, payloadInfoBase);
    uint32 interLeaveDepth = (uint32)(h264PayloadInfo->getSpropInterleavingDepth());
    uint32 packetizationMode = (uint32)(h264PayloadInfo->getPacketizationMode());

    // sanity check for the packetization mode and interleaving depth :
    // when packetization mode = 0 or 1, interleave depth shall not be unzero.
    // But when packetization mode = 2, interleave depth can be zero (Actually according to
    // the spec RFC 3984, 8.1, sprop-interleaving-depth, "this parameter MUST be present when
    // the value of packetization-mode is equal to 2", the interleave depth shouldn't be zero)
    if (packetizationMode < 2 && interLeaveDepth > 0) return false;

    iInterleaveDepth = interLeaveDepth;
    if (packetizationMode == 2 && iInterleaveDepth == 0)
    {
        iInterleaveDepth = H264_RTP_PAYLOAD_DEFAULT_INTERLEAVE_DEPTH;
    }

    // create and initialize the priority queue
    if (iInterleaveDepth > 0 && !iIMP)
    {
        if (!createIMPObject())
        {
            return false;
        }
    }

    // create iUtilityTable
    if (!iUtilityTable)
    {
        if (!createParserUtilityTable())
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Payload parsing
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PayloadParserStatus
H264PayloadParser::Parse(const Payload& inputPacket,
                         Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{

#ifdef PASS_THRU_MODE
    return parseRTPPayload_PassthruMode(inputPacket, accessUnit);
#endif

    // check the input and see the internal queue needs to flush in the interleave mode
    if (isInterleaveMode() && isFlushNeeded(inputPacket))
    {
        Payload out;
        PayloadParserStatus result = flush(out);
        if (result != PayloadParserStatus_Success)
        {
            return result;
        }
        vParsedPayloads.push_back(out);
        return PayloadParserStatus_Success;
    }

    // create iUtilityTable
    if (!iUtilityTable)
    {
        if (!createParserUtilityTable()) return  PayloadParserStatus_MemorAllocFail;
    }

    // Get the nal type
    uint8 nal_type = 0;
    if (!getNALType(inputPacket, nal_type)) return PayloadParserStatus_Failure;

    // Process the RTP packet based on non-interleaved mode(single nal unit mode belongs to non-interleaved mode) and interleaved mode
    if (isRequiredTypeForInterleaveMode(nal_type))
    {
        // nal_type: STAP-B, MATP16, MTAP24, FU-B/FU-A(FU-A must follow FU-B)
        return parseRTPPayload_For_InterleavedMode(inputPacket, nal_type, vParsedPayloads);
    }
    else
    {
        // for nal_type = 0,30,31, undefined type, let decoder make the decision
        if (isInterleaveMode() && !isExceptionTypeForInterleaveMode(nal_type)) return PayloadParserStatus_Failure;
        return parseRTPPayload_For_Non_InterleavedMode(inputPacket, nal_type, vParsedPayloads);
    }

    return PayloadParserStatus_Success;
}

///////////////////////////////////////////////////////////////////////////////
//
// Repositioning related
//
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF void H264PayloadParser::Reposition(const bool adjustSequence, const uint32 stream, const uint32 seqnum)
{
    OSCL_UNUSED_ARG(adjustSequence);
    OSCL_UNUSED_ARG(stream);
    OSCL_UNUSED_ARG(seqnum);
}

OSCL_EXPORT_REF uint32 H264PayloadParser::GetMinCurrTimestamp()
{
    return 0;
}

/***************************************************************************************
*******************************	PRIVATE SECTION ****************************************
****************************************************************************************/

PayloadParserStatus
H264PayloadParser::parseRTPPayload_For_Non_InterleavedMode(const Payload& inputPacket,
        const uint8 nal_type,
        Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    uint32 rtp_payload_ptr_offset = 0;
    return parseRTPPayload_For_SingleMessageOutput(inputPacket, nal_type, vParsedPayloads, rtp_payload_ptr_offset);
}


PayloadParserStatus
H264PayloadParser::parseRTPPayload_For_InterleavedMode(const Payload& inputPacket, uint8 nal_type,
        Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads)
{
    // NAL type:for the interleave mode :STAP-B(25), MTAP16(26), MTAP24(27), FU-B(29)/FU-A(28)
    //1. First-time, create and initialize the priority queue
    if (!iIMP)
    {
        if (!createIMPObject())
            return PayloadParserStatus_MemorAllocFail;
    }

    // check if the input rtp packet needs to be ignored in two cases:
    // (1) same input rtp packet as the previous one for multiple media messages output
    // (2) current rtp packet is FU-A packet, but the first FU-B packet is lost
    if (!isCurrRTPPacketIgnored(inputPacket, nal_type))
    {
        //2. generate media message(one or multiple) and push into the prority queue
        uint32 rtp_payload_ptr_offset = 0;
        do
        {
            PayloadParserStatus ret_val =
                parseRTPPayload_For_SingleMessageOutput(inputPacket, nal_type, vParsedPayloads, rtp_payload_ptr_offset);

            if (ret_val == PayloadParserStatus_DataNotReady) break;		// processing is complete
            if (ret_val != PayloadParserStatus_Success) return ret_val;	// error happens

            // push into the prority queue
            if (!iIMP->queue(inputPacket, nal_type, iIsFragmentedBitMask))
            {
                return PayloadParserStatus_MemorAllocFail;
            }

        }
        while (isMTAPType(nal_type)); // For MTAPs, multiple media messages per RTP packet

    } // end of: if(!isCurrRTPPacketIgnored(aRtpPacket, nal_type))

    //3. check the priority queue is ready for sending out media message
    if (iIMP->iNALUnitsCounter < iInterleaveDepth + 1) return PayloadParserStatus_DataNotReady;

    //4. the priority queue is ready for output
    Payload output;
    iIMP->dequeue(output);
    vParsedPayloads.push_back(output);

    // the prority queue still has something to send out, set RTP_PAYLOAD_PARSER_INPUT_NOT_EXHAUSTED
    if (iIMP->iNALUnitsCounter >= iInterleaveDepth + 1) return PayloadParserStatus_InputNotExhausted;

    return PayloadParserStatus_Success;
}

PayloadParserStatus
H264PayloadParser::parseRTPPayload_For_SingleMessageOutput(const Payload& inputPacket, const uint8 nal_type,
        Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads,
        uint32 &rtp_payload_ptr_offset)
{
    // NAL type: NAL unit(1-23) or STAP-A(24) or FU-A(28), or undefined types(0,30,31)

    // get the input setup
    if (!getInputSetup(inputPacket, nal_type, rtp_payload_ptr_offset))
        return PayloadParserStatus_DataNotReady; // rtp_payload_ptr_offset reaches the end of memFragIn


    Payload output;

    // set random access point
    output.randAccessPt = inputPacket.randAccessPt;

    // set marker info
    iUtility->setMarkerInfo(const_cast<IPayloadParser::Payload&>(inputPacket), output, nal_type);

    // memory fragment
    PayloadParserStatus ret_code = iUtility->generateMemFrag(inputPacket, output,
                                   nal_type, rtp_payload_ptr_offset);
    if (ret_code != PayloadParserStatus_Success) return ret_code;

    // set timestamp
    iUtility->setMediaDataTimestamp(output, nal_type, inputPacket.timestamp);

    // set sequence number
    iUtility->setSeqNum(output, nal_type, inputPacket.sequence + 1);

    vParsedPayloads.push_back(output);

    return PayloadParserStatus_Success;
}

inline bool H264PayloadParser::isFlushNeeded(const Payload& rtpPayload)
{
    return ((rtpPayload.vfragments.size() == 0) && iIMP && !iIMP->isQueueEmpty()); // empty input pointer and internal data queue is not empty
}

PayloadParserStatus H264PayloadParser::flush(Payload& accessUnit)
{
    if (iIMP->isQueueEmpty())
        return PayloadParserStatus_EmptyQueue;

    iIMP->dequeue(accessUnit);

    if (iIMP->isQueueEmpty())
        return PayloadParserStatus_EmptyQueue;

    return PayloadParserStatus_Success;
}

inline bool H264PayloadParser::getNALType(const Payload& inputPacket, uint8 &nal_type)
{
    if ((inputPacket.vfragments.size() == 0) && (!iIMP || iIMP->isQueueEmpty()))
    {
        // empty input pointer and internal queue is empty => failure
        return PayloadParserStatus_Failure;
    }

    // Retrieve the nal type(5 bits)
    uint8* ptr = (uint8*)const_cast<OsclRefCounterMemFrag&>(inputPacket.vfragments[0]).getMemFragPtr();
    nal_type = *ptr & NAL_TYPE_BIT_MASK; // take the lowest 5 bits

    return PayloadParserStatus_Success;
}


inline bool H264PayloadParser::isCurrRTPPacketIgnored(const Payload& inputPacket, const uint8 nal_type)
{
    //1. check FU-A packet case
    if ((nal_type == H264_RTP_PAYLOAD_FU_A) && (((iIMP->iPrevNALType >> 2) & 0x01) == 0))
        return true; // forget FU-As if previous FU type is not FU-B

    //2. check if the current input rtp packet is the same as the previous one
    if (iIMP->iRtpPayloadPtr == NULL) return false; // not ignored

    uint8 *memFragPtr = (uint8*)const_cast<Payload&>(inputPacket).vfragments[0].getMemFragPtr();

    if (iIMP->iRtpPayloadPtr == memFragPtr) return true;

    return false;
}

inline bool H264PayloadParser::getInputSetup(const Payload& inputPacket, const uint8 nal_type,
        uint32 rtp_payload_ptr_offset)
{
    if (rtp_payload_ptr_offset > 0 &&
            rtp_payload_ptr_offset + 5 >= const_cast<Payload&>(inputPacket).vfragments[0].getMemFragSize()) return false;

    // Get the proper utility from the utility table based on the nal type
    int32 index = ((int32)nal_type - H264_RTP_PAYLOAD_STAP_A) >> 1;
    if (index < 0) index = 3;
    iUtility = iUtilityTable[index];

    // set IMPObject
    if (iIMP)
    {
        iUtility->setIMPObject(iIMP);
        // save rtp payload ptr
        iIMP->iRtpPayloadPtr = (uint8*)const_cast<Payload&>(inputPacket).vfragments[0].getMemFragPtr();
    }

    return true;
}

bool H264PayloadParser::createParserUtilityTable()
{
    iUtilityTable = OSCL_ARRAY_NEW(H264PayloadParserUtility *, H264_RTP_PAYLOAD_MIN_TYPE_NUM);
    if (!iUtilityTable) return false;
    int32 i;
    for (i = 0; i < H264_RTP_PAYLOAD_MIN_TYPE_NUM; i++) iUtilityTable[i] = NULL;

    // All the H264PayloadParserUtility objects are generated upfront to remove the
    // run-time overhead for the dynamic object creation
    // The mapping in the table(0=>STAP, 1=>MTAP, 2=>FU, 3=>NALU) is important,
    // shouldn't change because the searching logic rely on this mapping
    iUtilityTable[0] = OSCL_NEW(H264PayloadParserUtilityForSTAP, (this)); // iIMP is not created at this point
    iUtilityTable[1] = OSCL_NEW(H264PayloadParserUtilityForMTAP, (this));
    iUtilityTable[2] = OSCL_NEW(H264PayloadParserUtilityForFU, (this));
    iUtilityTable[3] = OSCL_NEW(H264PayloadParserUtilityForNALU, (this));
    if (!iUtilityTable[0] || !iUtilityTable[1] || !iUtilityTable[2] || !iUtilityTable[3])
    {
        deleteParserUtilityTable();
        return false;
    }

    return true;
}

void H264PayloadParser::deleteParserUtilityTable()
{
    if (iUtilityTable)
    {
        int32 i;
        for (i = 0; i < H264_RTP_PAYLOAD_MIN_TYPE_NUM; i++)
        {
            OSCL_DELETE(iUtilityTable[i]);
            iUtilityTable[i] = NULL;
        }
        OSCL_ARRAY_DELETE(iUtilityTable);
        iUtilityTable = NULL;

    }
}

bool H264PayloadParser::createIMPObject()
{
    if (iInterleaveDepth == 0) return false;
    iIMP = new InterleaveModeProcessing();
    if (!iIMP) return false;
    if (!iIMP->initialize(iInterleaveDepth)) return false;

    return true;
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////// H264PayloadParserUtility base implementation  ////////////////////////
/////////////////////////////////////////////////////////////////////////////////
PayloadParserStatus
H264PayloadParserUtility::generateMemFrag(const IPayloadParser::Payload& aIn,
        IPayloadParser::Payload& aOut,
        const uint8 nal_type,
        uint32 &rtp_payload_ptr_offset)
{
    // construct output memory fragment
    OsclRefCounterMemFrag memFragOut(aIn.vfragments[0]);
    uint8* memfrag = NULL;

    // Get the actual memory frag pointer and length
    PayloadParserStatus ret_val =
        getMemFragPtrLen(const_cast<IPayloadParser::Payload&>(aIn).vfragments[0],
                         nal_type, memfrag,
                         memFragOut.getMemFrag().len, rtp_payload_ptr_offset);
    memFragOut.getMemFrag().ptr = (OsclAny*)memfrag;
    if (ret_val != PayloadParserStatus_Success) return ret_val;

    // add the memory fragment into media data imp object
    aOut.vfragments.push_back(memFragOut);

    return PayloadParserStatus_Success;
}

void H264PayloadParserUtility::setMarkerInfo(IPayloadParser::Payload& aIn,
        IPayloadParser::Payload& aOut,
        const uint8 nal_type)
{
    OSCL_UNUSED_ARG(nal_type);
    OSCL_UNUSED_ARG(aIn);
    OSCL_UNUSED_ARG(aOut);

    aOut.marker = aIn.marker;
    aOut.endOfNAL = true;
    iParser->setFragmentedBitMask(0);
}
void H264PayloadParserUtility::setMediaDataTimestamp(IPayloadParser::Payload& aOut,
        const uint8 nal_type,
        const uint32 rtp_timestamp)
{
    OSCL_UNUSED_ARG(nal_type);

    // default implementation: complete NAL, use RTP timestamp directly
    aOut.timestamp = rtp_timestamp;

    // complete nal, set timestamp = 0
    iParser->setTimestampForFU(0);

}


/////////////////////////////////////////////////////////////////////////////////
/////////////////// H264PayloadParserUtilityForNALU implementation  /////////////////////
/////////////////////////////////////////////////////////////////////////////////
PayloadParserStatus
H264PayloadParserUtilityForNALU::getMemFragPtrLen(
    OsclRefCounterMemFrag &aMemFragIn,
    const uint8 nal_type,
    uint8* &aMemFragPtr, uint32 &aMemFragLen,
    uint32 &rtp_payload_ptr_offset)
{
    OSCL_UNUSED_ARG(nal_type);
    OSCL_UNUSED_ARG(rtp_payload_ptr_offset);

    // NAL unit or undefined NAL type
    aMemFragPtr = (uint8*)(aMemFragIn.getMemFragPtr());
    aMemFragLen = aMemFragIn.getMemFragSize();
    return PayloadParserStatus_Success;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////// H264PayloadParserUtilityForFU implementation  ///////////////////////
/////////////////////////////////////////////////////////////////////////////////
PayloadParserStatus
H264PayloadParserUtilityForFU::getMemFragPtrLen(
    OsclRefCounterMemFrag &aMemFragIn,
    const uint8 nal_type,
    uint8* &aMemFragPtr, uint32 &aMemFragLen,
    uint32 &rtp_payload_ptr_offset)
{
    OSCL_UNUSED_ARG(rtp_payload_ptr_offset);

    // figure out ptr and len of the output memory fragment
    uint8* rtp_payload_ptr = (uint8*)(aMemFragIn.getMemFragPtr());

    // check the current FU is valid
    if (!validateFU(rtp_payload_ptr, nal_type)) return PayloadParserStatus_Failure;

    // get 16-bit DON for FU-B
    if (nal_type == H264_RTP_PAYLOAD_FU_B)
    {
        iIMP->iDon = (rtp_payload_ptr[2] << 8) | rtp_payload_ptr[3];
    }

    uint32 fu_type = iParser->getFragmentedBitMask() & 0x03;

    // RTP payload offset:  1 starting FU-A		(fu_type == 1 && nal_type == H264_RTP_PAYLOAD_FU_A)
    //						2 non-starting FU-A	(fu_type != 1 && nal_type == H264_RTP_PAYLOAD_FU_A)
    //						3 starting FU-B		(fu_type == 1 && nal_type == H264_RTP_PAYLOAD_FU_B)
    //						4 non-starting FU-B (fu_type != 1 && nal_type == H264_RTP_PAYLOAD_FU_B)
    uint32 offset = 1 + (uint32)(fu_type != 1) + ((uint32)(nal_type == H264_RTP_PAYLOAD_FU_B) << 1);


    // insert NAL unit type octet for starting FU
    if (fu_type == 1)
    {
        // Construct NAL unit type octet
        octet nal_unit_type = (rtp_payload_ptr[0] & NAL_F_NRI_BIT_MASK) |
                              (rtp_payload_ptr[1] & NAL_TYPE_BIT_MASK);
        rtp_payload_ptr[offset] = nal_unit_type;
    }

    // set aMemFragPtr and aMemFragLen
    aMemFragPtr = rtp_payload_ptr + offset;
    aMemFragLen = aMemFragIn.getMemFragSize() - offset;

    return PayloadParserStatus_Success;

}

void H264PayloadParserUtilityForFU::setMarkerInfo(IPayloadParser::Payload& aIn,
        IPayloadParser::Payload& aOut,
        const uint8 nal_type)
{
    aOut.marker = aIn.marker;
    aOut.endOfNAL = false;

    uint32 aFragmentedBitMask = 0;
    uint8* rtp_payload_ptr = (uint8*)(aIn.vfragments[0].getMemFragPtr());
    if (rtp_payload_ptr[1] & FU_S_BIT_MASK)  // check S bit of FU header
    {
        aFragmentedBitMask = 1;	// starting FU
    }
    else if (rtp_payload_ptr[1] & FU_E_BIT_MASK)  // check E bit of FU header
    {
        aOut.endOfNAL = true;
        aFragmentedBitMask = 3; // ending FU
    }
    else
    {
        aFragmentedBitMask = 2; // intermediate FU
    }

    // bit 2: 0 = FU-A 1 = FU-B
    aFragmentedBitMask |= ((uint32)(nal_type == H264_RTP_PAYLOAD_FU_B) << 2);

    // copy FU header (8bits) to iIsFragmentedBitMask
    aFragmentedBitMask |= ((uint32)rtp_payload_ptr[1] << 3); // Major reason to save this FU header is because this FU header byte could
    // be overwritten by the constructed NAL unit type octet from FU indicator and FU header
    // counter intermediate FU if there is
    if ((aFragmentedBitMask&0x03) == 2)
    {
        uint32 inter_fu_counter = iParser->getFragmentedBitMask() >> 11;
        aFragmentedBitMask |= (++inter_fu_counter << BIT_SHIFT_FOR_FU_COUNTER);
    }

    iParser->setFragmentedBitMask(aFragmentedBitMask);

}

void H264PayloadParserUtilityForFU::setMediaDataTimestamp(IPayloadParser::Payload& aOut,
        const uint8 nal_type,
        const uint32 rtp_timestamp)
{
    OSCL_UNUSED_ARG(nal_type);

    aOut.timestamp = rtp_timestamp;

    uint32 FUType = iParser->getFragmentedBitMask() & 0x03;
    if (FUType == 1 || iParser->getTimestampForFU() == 0)
    {
        // reset point: starting FU or if S-bit is not set for some reason,
        // set the current one, and make the following FUs use this timestamp.
        // Note that H264PayloadParser::iTimestampForFU is initialized to 1
        iParser->setTimestampForFU(rtp_timestamp);
    }

    if (iParser->getTimestampForFU() > 1)
    {
        // use the timestamp of the first fragmented NAL.
        // If H264PayloadParser::iTimestampForFU is not set for some reason, use the latest one
        aOut.timestamp = iParser->getTimestampForFU();
    }

    if (FUType == 3)  // ending FU, set timestamp = 0 for the case where the next packet is still a FU,
    {
        // but for a different nal.
        iParser->setTimestampForFU(0);
    }
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////// H264PayloadParserUtilityForMTAP implementation  /////////////////////
/////////////////////////////////////////////////////////////////////////////////
PayloadParserStatus
H264PayloadParserUtilityForMTAP::getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn,
        const uint8 nal_type,
        uint8* &aMemFragPtr, uint32 &aMemFragLen,
        uint32 &rtp_payload_ptr_offset)
{
    uint8* rtp_payload_ptr = (uint8*)(aMemFragIn.getMemFragPtr());
    int32 memFragLen = aMemFragIn.getMemFragSize() - rtp_payload_ptr_offset;
    if (rtp_payload_ptr_offset == 0 && memFragLen <= 7) return PayloadParserStatus_Failure;
    if (memFragLen <= 5) return PayloadParserStatus_DataNotReady;

    // Get 16-bit DONB at the beginning
    if (rtp_payload_ptr_offset == 0)
    {
        iIMP->iDonBase = (rtp_payload_ptr[1] << 8) | rtp_payload_ptr[2];
        rtp_payload_ptr_offset = 3;
    }

    // get 16-bit NAL size => aMemFragLen
    aMemFragLen  = (uint32)(rtp_payload_ptr[rtp_payload_ptr_offset++] << 8);
    aMemFragLen |= (uint32)rtp_payload_ptr[rtp_payload_ptr_offset++];

    // get 8-bit DOND and calculate the actual DON = DONB+DOND % 65536
    iIMP->iDon = (iIMP->iDonBase + (uint32)rtp_payload_ptr[rtp_payload_ptr_offset++]) % 65536;

    // get 16-bit or 24-bit TS offset
    iIMP->iTimestampOffset = rtp_payload_ptr[rtp_payload_ptr_offset++];
    iIMP->iTimestampOffset = (iIMP->iTimestampOffset << 8) | rtp_payload_ptr[rtp_payload_ptr_offset++];
    if (nal_type == H264_RTP_PAYLOAD_MTAP24)
    {
        iIMP->iTimestampOffset = (iIMP->iTimestampOffset << 8) | rtp_payload_ptr[rtp_payload_ptr_offset++];
    }

    aMemFragPtr = rtp_payload_ptr + rtp_payload_ptr_offset;
    rtp_payload_ptr_offset += aMemFragLen;
    return PayloadParserStatus_Success;
}

void H264PayloadParserUtilityForMTAP::setMediaDataTimestamp(IPayloadParser::Payload& aOut,
        const uint8 nal_type,
        const uint32 rtp_timestamp)
{
    OSCL_UNUSED_ARG(nal_type);

    // Question: section 5.7.2, the definition of timestamp offset is not clear to get the NALU-time
    uint32 ts = rtp_timestamp + iIMP->iTimestampOffset;
    aOut.timestamp = ts;

    // complete nal, set timestamp = 0
    iParser->setTimestampForFU(0);
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////// H264PayloadParserUtilityForSTAP implementation  /////////////////////
/////////////////////////////////////////////////////////////////////////////////
PayloadParserStatus
H264PayloadParserUtilityForSTAP::generateMemFrag(
    const IPayloadParser::Payload& aIn,
    IPayloadParser::Payload& aOut,
    const uint8 nal_type,
    uint32 &rtp_payload_ptr_offset)
{
    OSCL_UNUSED_ARG(rtp_payload_ptr_offset);

    // create iMemFragmentAlloc at the very first time
    if (!iMemFragmentAlloc)
    {
        PayloadParserStatus status = CreateMemFragmentAlloc();
        if (status != PayloadParserStatus_Success) return status;
    }

    uint8* rtp_payload_ptr  = (uint8*)(const_cast<IPayloadParser::Payload&>(aIn).vfragments[0].getMemFragPtr());
    uint32 rtp_payload_size = const_cast<IPayloadParser::Payload&>(aIn).vfragments[0].getMemFragSize();
    uint32 bytesParsed = 1; // STAP-A/B header

    // for STAP-B, get 16-bit DON
    if (nal_type == H264_RTP_PAYLOAD_STAP_B)
    {
        iIMP->iDon  = rtp_payload_ptr[bytesParsed++] << 8;
        iIMP->iDon |= rtp_payload_ptr[bytesParsed++];
    }

    bool isReallyParsing = false;
    while (bytesParsed + 2 < rtp_payload_size)   // 2-byte nal size
    {
        OsclRefCounterMemFrag memFragOut(aIn.vfragments[0]);

        uint16 nal_size = (uint16)(rtp_payload_ptr[bytesParsed++] << 8);
        nal_size |= (uint16)rtp_payload_ptr[bytesParsed++];
        if (nal_size == 0) break;

        memFragOut.getMemFrag().ptr = rtp_payload_ptr + bytesParsed;
        memFragOut.getMemFrag().len = (uint32)nal_size;
        aOut.vfragments.push_back(memFragOut);

        bytesParsed += (uint32)nal_size;
        isReallyParsing = true;
    }

    if (!isReallyParsing)  // no any parsing
    {
        return PayloadParserStatus_Failure;
    }

    return PayloadParserStatus_Success;
}

PayloadParserStatus
H264PayloadParserUtilityForSTAP::CreateMemFragmentAlloc()
{
    iMemFragmentAlloc = OSCL_NEW(PVMFBufferPoolAllocator, ());
    if (!iMemFragmentAlloc) return PayloadParserStatus_MemorAllocFail;

    int32 err = 0;
    OSCL_TRY(err, iMemFragmentAlloc->size(H264_RTP_PAYLOAD_MEMFRAG_POOLNUM, sizeof(OsclMemoryFragment)));
    if (err != OsclErrNone) return PayloadParserStatus_MemorAllocFail;

    return PayloadParserStatus_Success;
}

