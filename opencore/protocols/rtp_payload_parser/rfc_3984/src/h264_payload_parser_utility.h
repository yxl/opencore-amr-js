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
#ifndef H264_PAYLOAD_PARSER_UTILITY_H
#define H264_PAYLOAD_PARSER_UTILITY_H

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

#ifndef H264_PAYLOAD_PARSER_MACROS_H_INCLUDED
#include "h264_payload_parser_macros.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// The following data structures are designed to remove conditional(e.g.long switch or   //
// if-else statments) using polymorphism, or remove type code with class				 //
///////////////////////////////////////////////////////////////////////////////////////////
class H264PayloadParser;

////// Base class  //////////////////
class H264PayloadParserUtility
{
    public:
        virtual ~H264PayloadParserUtility() { };
        /**
         * Note: for the following APIs, setMarkerInfo() must be called at first, because generateMemFrag()
         * could ovewrite something that needs to be saved in setMarkerInfo(), and will be used in
         * setMediaDataTimestamp(). Adding a flag to check this is not a good option.
         * But setSeqNum() is independent, and can be called any time
        **/


        /**
        * Generate the output memory fragments for the given input memory fragment and output media data impl object
        * The function is designed to be non-pure virtual because we want to put in the default implementation to be
        * shared with the derived classes
        *
        * @param aMediaDataOut, output media data impl object
        * @param nal_type,		 input nal type to avoid uncessary parsing operation
        * @param aMemFragIn,	 input memory fragment
        * @param rtp_payload_ptr_offset, the offset of rtp payload pointer for the input memory fragment, which can be modified
        * @return RTP_PAYLOAD_PARSER_RET_CODE return code
        **/
        virtual PayloadParserStatus generateMemFrag(const IPayloadParser::Payload& aIn,
                IPayloadParser::Payload& aOut,
                const uint8 nal_type,
                uint32 &rtp_payload_ptr_offset);

        /**
         * Set the marker info for the output media data impl object
         * The function is designed to be non-pure virtual and the base classe provides the default implementation.
         *
         * @param aMediaDataOut, output media data impl object
          * @param nal_type,		 input nal type to avoid uncessary parsing operation
         * @param aMemFragIn,	 input memory fragment
         **/
        virtual void setMarkerInfo(IPayloadParser::Payload& aIn, IPayloadParser::Payload& aOut, const uint8 nal_type);

        /**
         * Set the timestamp for the output media data object
         * The function is designed to be non-pure virtual and the base class provides the default implementation.
         *
         * @param accessUnit,	 output media data object
          * @param nal_type,		 input nal type to avoid uncessary parsing operation
         * @param rtp_timestamp, input RTP timestamp
         **/
        virtual void setMediaDataTimestamp(IPayloadParser::Payload& aOut, const uint8 nal_type, const uint32 rtp_timestamp);

        /**
         * Set the sequence number for the output media data object
         * The function is designed to be non-pure virtual and the base class provides the default implementation.
         *
         * @param accessUnit,	 output media data object
          * @param nal_type,		 input nal type to avoid uncessary parsing operation
         * @param rtp_timestamp, input RTP timestamp
         **/
        virtual void setSeqNum(IPayloadParser::Payload& aOut, const uint8 nal_type, const uint32 seq_number)
        {
            OSCL_UNUSED_ARG(nal_type); // no use at this time
            aOut.sequence = seq_number;
        }

        // constructor
        H264PayloadParserUtility(H264PayloadParser *aParser, InterleaveModeProcessing *aIMP = NULL) :
                iParser(aParser), iIMP(aIMP)
        {
            ;
        }

        void setIMPObject(InterleaveModeProcessing *aIMP)
        {
            iIMP = aIMP;
        }

    protected:

        /**
         * Get the pointer and length of the output memory fragment for the given input memory fragment
         * The function is designed to be pure virtual so that each derived classes must implement this function
         *
         * @param aMemFragIn,	 input memory fragment
         * @param nal_type,		 input nal type to avoid uncessary parsing operation
         * @param aMemFragPtr,	 output memory fragment pointer
         * @param aMemFragLen,	 output memory fragment length
         * @param rtp_payload_ptr_offset, the offset of rtp payload pointer for the input memory fragment, which can be modified
         * @return RTP_PAYLOAD_PARSER_RET_CODE return code
         **/
        virtual	PayloadParserStatus getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn, const uint8 nal_type,
                uint8* &aMemFragPtr, uint32 &aMemFragLen,
                uint32 &rtp_payload_ptr_offset) = 0;

    protected:
        H264PayloadParser *iParser;
        InterleaveModeProcessing *iIMP;
};


////// Derived classes  //////////////////

// NALU type based parser utility, NAL type = 0-23, 30, 31
class H264PayloadParserUtilityForNALU : public H264PayloadParserUtility
{
    public:

        // constructor
        H264PayloadParserUtilityForNALU(H264PayloadParser *aParser, InterleaveModeProcessing *aIMP = NULL) :
                H264PayloadParserUtility(aParser, aIMP)
        {
            ;
        }

    private:
        // derived APIs
        PayloadParserStatus getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn, const uint8 nal_type,
                                             uint8* &aMemFragPtr, uint32 &aMemFragLen,
                                             uint32 &rtp_payload_ptr_offset);

};

// FU type based parser utility, NAL type = 28 and 29, FU-A and FU-B
// For H264PayloadParserUtilityForFU, setMarkerInfo() and setMediaDataTimestamp() are special due to marker bit(S or E bit)
class H264PayloadParserUtilityForFU : public H264PayloadParserUtility
{
    public:

        // derived APIs
        void setMarkerInfo(IPayloadParser::Payload& aIn, IPayloadParser::Payload& aOut, const uint8 nal_type);
        void setMediaDataTimestamp(IPayloadParser::Payload& aOut, const uint8 nal_type, const uint32 rtp_timestamp);

        // constructor
        H264PayloadParserUtilityForFU(H264PayloadParser *aParser, InterleaveModeProcessing *aIMP = NULL) :
                H264PayloadParserUtility(aParser, aIMP)
        {
            ;
        }

    private:
        // derived API
        PayloadParserStatus getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn, const uint8 nal_type,
                                             uint8* &aMemFragPtr, uint32 &aMemFragLen,
                                             uint32 &rtp_payload_ptr_offset);

    private:

        bool validateFU(uint8* rtp_payload_ptr, uint8 nal_type)
        {
            if (nal_type == H264_RTP_PAYLOAD_FU_B &&
                    (rtp_payload_ptr[1] & FU_S_BIT_MASK) == 0)
            {
                return false; // FU-B must be the starting FU
            }
            return true;
        }

};

// MTAP type based parser utility, NAL type = 26 and 27, MTAP16 and MTAP24
// For H264PayloadParserUtilityForMTAP, setMediaDataTimestamp() is special due to 16/24-bit TS offset
class H264PayloadParserUtilityForMTAP : public H264PayloadParserUtility
{
    public:

        // derived API
        void setMediaDataTimestamp(IPayloadParser::Payload& aOut, const uint8 nal_type, const uint32 rtp_timestamp);

        // constructor
        H264PayloadParserUtilityForMTAP(H264PayloadParser *aParser, InterleaveModeProcessing *aIMP = NULL) :
                H264PayloadParserUtility(aParser, aIMP)
        {
            ;
        }

    private:
        // derived API
        PayloadParserStatus getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn, const uint8 nal_type,
                                             uint8* &aMemFragPtr, uint32 &aMemFragLen,
                                             uint32 &rtp_payload_ptr_offset);
};

// STAP type based parser utility, NAL type = 24 and 25, STAP-A and STAP-B
// For H264PayloadParserUtilityForSTAP, API generateMemFrag() is kinda different from others, because each input RTP packet needs to generate
// mulitple memory fragments for one output media message. Instead, other types of RTP packet only generate one
// memory fragment per media message.
class H264PayloadParserUtilityForSTAP : public H264PayloadParserUtility
{
    public:

        // derived APIs
        PayloadParserStatus generateMemFrag(const IPayloadParser::Payload& aIn,
                                            IPayloadParser::Payload& aOut,
                                            const uint8 nal_type,
                                            uint32 &rtp_payload_ptr_offset);

        // constructor
        H264PayloadParserUtilityForSTAP(H264PayloadParser *aParser, InterleaveModeProcessing *aIMP = NULL) :
                H264PayloadParserUtility(aParser, aIMP),
                iMemFragmentAlloc(NULL)
        {
            ;
        }

        // destructor
        virtual ~H264PayloadParserUtilityForSTAP()
        {
            OSCL_DELETE(iMemFragmentAlloc);
            iMemFragmentAlloc = NULL;
        }

    private:
        // derived API
        PayloadParserStatus getMemFragPtrLen(OsclRefCounterMemFrag &aMemFragIn, const uint8 nal_type,
                                             uint8* &aMemFragPtr, uint32 &aMemFragLen,
                                             uint32 &rtp_payload_ptr_offset)
        {
            OSCL_UNUSED_ARG(aMemFragIn);
            OSCL_UNUSED_ARG(nal_type);
            OSCL_UNUSED_ARG(aMemFragPtr);
            OSCL_UNUSED_ARG(aMemFragLen);
            OSCL_UNUSED_ARG(rtp_payload_ptr_offset);
            return PayloadParserStatus_Success;
        }

        // Create iMemFragmentAlloc
        PayloadParserStatus CreateMemFragmentAlloc();

    private:
        // Fragment pool for OsclRefCounterMemFrag
        PVMFBufferPoolAllocator *iMemFragmentAlloc;
};

#endif // H264_PAYLOAD_PARSER_UTILITY_H

