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
#ifndef RM_MEDIAINFO_H
#define RM_MEDIAINFO_H
#include "media_info.h"

class rm_mediaInfo : public mediaInfo
{
    private:
        /* RM Related */
        NptTimeFormat rm_length;
        OSCL_HeapString<SDPParserAlloc> rm_mimetype;
        int rm_StreamId;
        int rm_AvgBitRate;
        int rm_AvgPacketSize;
        int rm_EndOneRuleEndAll;
        int rm_MaxBitRate;
        int rm_MaxPacketSize;
        int rm_Preroll;
        int rm_ActualPreroll;
        int rm_EndTime;
        OSCL_HeapString<SDPParserAlloc> rm_Flags;
        OSCL_HeapString<SDPParserAlloc> rm_ASMRuleBook;
        OSCL_HeapString<SDPParserAlloc> rm_intrinsicDurationType;
        OSCL_HeapString<SDPParserAlloc> rm_StreamName;
        //In real media SDP, decoder specific info is present as a separate SDP field called OpaqueData.
        //It is not stored as a part of config field of a=fmtp.
        OsclSharedPtr<uint8> rm_decoderSpecificdata;
        uint32 rm_decoderSpecificdataSize;

    public:
        rm_mediaInfo()
        {
            rm_StreamId = 0;
            rm_AvgBitRate = 0;
            rm_AvgPacketSize = 0;
            rm_EndOneRuleEndAll = 0;
            rm_MaxBitRate = 0;
            rm_MaxPacketSize = 0;
            rm_Preroll = 0;
            rm_ActualPreroll = 0;
            rm_EndTime = 0;
            rm_decoderSpecificdataSize = 0;
        };

        rm_mediaInfo(const rm_mediaInfo &pSource) : mediaInfo(pSource)
        {
            setRMLength(pSource.rm_length);
        }

        const rm_mediaInfo & operator=(const rm_mediaInfo &pSource)
        {
            if (&pSource != this)
            {
                setRMLength(pSource.rm_length);
            }

            return pSource;
        }

        ~rm_mediaInfo() {};

        inline void setRMMimeType(const char* MType)
        {
            rm_mimetype = MType;
        };

        inline void setRMMimeType(const OSCL_HeapString<SDPParserAlloc>& MType)
        {
            rm_mimetype = MType;
        };

        inline void setRMMimeType(const OsclMemoryFragment memFrag)
        {
            rm_mimetype.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setRMLength(const NptTimeFormat& in_range)
        {
            rm_length = in_range;
        };

        inline OSCL_HeapString<SDPParserAlloc> getRMMimeType()
        {
            return rm_mimetype;
        };

        inline void setRealStreamId(int streamid)
        {
            rm_StreamId = streamid;
        };

        inline int getRealStreamId()
        {
            return rm_StreamId;
        };

        inline void setAvgBitRate(int min_bit_rate)
        {
            rm_AvgBitRate = min_bit_rate;
        };

        inline int getAvgBitRate()
        {
            return	rm_AvgBitRate;
        };

        inline void setAvgPacketSize(int pack_sz)
        {
            rm_AvgPacketSize = pack_sz;
        };
        inline int getAvgPacketSize()
        {
            return	rm_AvgPacketSize;
        };
        inline void setMaxBitRate(int max_bit_rate)
        {
            rm_MaxBitRate = max_bit_rate;
        };
        inline int getMaxBitRate()
        {
            return	rm_MaxBitRate;
        };

        inline void setMaxPacketSize(int max_pack_sz)
        {
            rm_MaxPacketSize = max_pack_sz;
        };
        inline int getMaxPacketSize()
        {
            return	rm_MaxPacketSize;
        };

        inline void setPreroll(int preroll)
        {
            rm_Preroll = preroll;
        };
        inline int getPreroll()
        {
            return	rm_Preroll;
        };
        inline void setActualPreroll(int acpreroll)
        {
            rm_ActualPreroll = acpreroll;
        };

        inline int getActualPreroll()
        {
            return	rm_ActualPreroll;
        };

        inline void setEndTime(int endtime)
        {
            rm_EndTime = endtime;
        };

        inline int getEndTime()
        {
            return	rm_EndTime;
        };
        inline void setEndOneRuleEndAll(int endrule)
        {
            rm_EndOneRuleEndAll = endrule;
        };

        inline int getEndOneRuleEndAll()
        {
            return	rm_EndOneRuleEndAll;
        };

        inline void setRMFlags(const char* flag)
        {
            rm_Flags = flag;
        };

        inline void setRMFlags(const OSCL_HeapString<SDPParserAlloc>& flag)
        {
            rm_Flags = flag;
        };

        inline void setRMFlags(const OsclMemoryFragment memFrag)
        {
            rm_Flags.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline OSCL_HeapString<SDPParserAlloc> getRMFlags()
        {
            return rm_Flags;
        };
        inline void setASMRuleBook(const char* asmRule)
        {
            rm_ASMRuleBook = asmRule;
        };

        inline void setASMRuleBook(const OSCL_HeapString<SDPParserAlloc>& asmRule)
        {
            rm_ASMRuleBook = asmRule;
        };

        inline void setASMRuleBook(const OsclMemoryFragment memFrag)
        {
            rm_ASMRuleBook.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline OSCL_HeapString<SDPParserAlloc> getASMRuleBook()
        {
            return rm_ASMRuleBook;
        };
        inline void setintrinsicDurationType(const char* intrinsicDuration)
        {
            rm_intrinsicDurationType = intrinsicDuration;
        };

        inline void setintrinsicDurationType(const OSCL_HeapString<SDPParserAlloc>& intrinsicDuration)
        {
            rm_intrinsicDurationType = intrinsicDuration;
        };

        inline void setintrinsicDurationType(const OsclMemoryFragment memFrag)
        {
            rm_intrinsicDurationType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline OSCL_HeapString<SDPParserAlloc> getintrinsicDurationType()
        {
            return rm_intrinsicDurationType;
        };

        inline void setStreamName(const char* stream)
        {
            rm_StreamName = stream;
        };

        inline void setStreamName(const OSCL_HeapString<SDPParserAlloc>& stream)
        {
            rm_StreamName = stream;
        };

        inline void setStreamName(const OsclMemoryFragment memFrag)
        {
            rm_StreamName.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline OSCL_HeapString<SDPParserAlloc> getStreamName()
        {
            return rm_StreamName;
        };

        inline void setRMDecoderSpecificInfo(OsclSharedPtr<uint8> Info)
        {
            rm_decoderSpecificdata = Info;
        }

        inline OsclSharedPtr<uint8> getRMDecoderSpecificInfo()
        {
            return rm_decoderSpecificdata;
        }

        inline void setRMDecoderSpecificInfoSize(uint32 size)
        {
            rm_decoderSpecificdataSize = size;
        }

        inline uint32 getRMDecoderSpecificInfoSize()
        {
            return rm_decoderSpecificdataSize;
        }

        inline NptTimeFormat *getRMLength()
        {
            return &rm_length;
        };


};

#endif


