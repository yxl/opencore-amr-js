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
#ifndef	_h324utils_h
#define _h324utils_h
#include "pvt_params.h"
#include "h245def.h"
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#include "pvt_common.h"

#define MAX_NUM_SIMULTANEOUS_CAPABILITIES 31
#define MAX_CONFIG_INFO_SIZE 128
#define MAX_H264_FORMAT_SPECIFIC_INFO_LEN 128
#define DEF_AL3_SEND_BUFFER_SIZE 1024

/* Generic logger macros for using across the protocol stack node */
#define PVMF_INCOMING_VIDEO_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iIncomingVideoLogger,PVLOGMSG_INFO,m);
#define PVMF_INCOMING_AUDIO_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iIncomingAudioLogger,PVLOGMSG_INFO,m);
#define PVMF_OUTGOING_AUDIO_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iOutgoingAudioLogger,PVLOGMSG_INFO,m);
#define PVMF_OUTGOING_VIDEO_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iOutgoingVideoLogger,PVLOGMSG_INFO,m);


#define PV2WAY_FILLER_FSI_LEN 9
#define PV2WAY_FILLER_FSI {1,2,3,4,5,6,7,8,9}


/* Allocator for TSC */
class PVMFTscAlloc : public Oscl_DefAlloc
{
    public:
        void* allocate(const uint32 size)
        {
            void* tmp = (void*)oscl_malloc(size);
            return tmp;
        }

        void deallocate(void* p)
        {
            oscl_free(p);
        }
};

/* Removed C Linkage - Glk - Due to overloaded fxns
#ifdef __cplusplus
extern "C" {
#endif

  */
struct MultiplexCapabilityInfo
{
    bool iAllowAl1Video;
    bool iAllowAl2Video;
    bool iAllowAl3Video;
    bool iAllowAl1Audio;
    bool iAllowAl2Audio;
    bool iAllowAl3Audio;
    unsigned iMaximumAl2SDUSize;
    unsigned iMaximumAl3SDUSize;
};

typedef struct S_CapabilityIdentifierStandard
{
    uint8 reserved1;
    uint8 itu; /* 0 = t */
    uint8 spec_type; /* 0==recommendation */
    uint8 series_letter; /* h = 8 */
    uint32 spec_number; /* 245,241 etc */
    uint8 data[4];
}* PS_CapabilityIdentifierStandard;


PVAudTypeSimple_t GetSimpleAudioType(PVAudType_t);

PVCodecType_t GetCodecType(PS_DataType pDataType);
PV2WayMediaType GetMediaType(PS_DataType pDataType);
PVCodecType_t GetVidCodecTypeFromVideoCapability(PS_VideoCapability capability);
PVCodecType_t GetVidCodecTypeFromCapabilityIdentifier(S_CapabilityIdentifierStandard& identifier);
PVCodecType_t GetAudCodecTypeFrom245Index(int32 index);
PVCodecType_t GetAudCodecType(PS_GenericCapability audio_capability);
PVCodecType_t GetAudCodecType(PS_AudioCapability audio_capability);
PVCodecType_t GetUiCodecTypeFrom245Index(int32 index);
void GetCodecInfo(PS_Capability capability, CodecCapabilityInfo& info);
ErrorProtectionLevel_t GetEpl(uint16 al_index);
void printBuffer(PVLogger* logger, const uint8* buffer, uint16 len);
unsigned GetFormatSpecificInfo(PS_DataType pDataType, uint8*& fsi);
PS_Capability LookupCapability(PS_TerminalCapabilitySet pTcs, uint16 cap_entry_num);
bool IsTransmitOnlyAltCapSet(PS_TerminalCapabilitySet pTcs, PS_AlternativeCapabilitySet pAltCapSet);
PVMFStatus VerifyCodecs(PS_TerminalCapabilitySet pTcs,
                        Oscl_Vector<OlcFormatInfo, OsclMemAllocator> codecs,
                        PVLogger *logger);
PVMFStatus GetCodecCapInfo(PVCodecType_t codec,
                           PS_TerminalCapabilitySet pTcs,
                           CodecCapabilityInfo& codec_info,
                           PVLogger *logger);
uint32 CheckAltCapSet(PS_AlternativeCapabilitySet pAltCapSet, uint32 entry);
PS_TerminalCapabilitySet GenerateTcs(MultiplexCapabilityInfo& mux_cap_info,
                                     Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& outgoing_codecs,
                                     Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& incoming_codecs);
void FillCapabilityTableEntry(PS_CapabilityTableEntry, CodecCapabilityInfo& codec_info, uint32 entry_num);
void FillCapability(CodecCapabilityInfo& codec_info, PS_Capability capability);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_Capability capability);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_GenericCapability capability);
void FillVideoCapability(VideoCodecCapabilityInfo& codec_info, PS_VideoCapability video_capability);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_VideoCapability capability);
void FillAudioCapability(CodecCapabilityInfo& codec_info, PS_AudioCapability audio_capability);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_AudioCapability capability);
void FillH263Capability(VideoCodecCapabilityInfo& video_codec_info, PS_H263VideoCapability h263caps);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_H263VideoCapability capability);
void FillG723Capability(PS_G7231 g723caps);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_G7231 capability);
void FillAmrCapability(PS_GenericCapability amrCaps);
CodecCapabilityInfo* GetCodecCapabilityInfoAmr(PS_GenericCapability capability);
CodecCapabilityInfo* GetCodecCapabilityInfoAvc(PS_GenericCapability capability);
void FillM4vCapability(VideoCodecCapabilityInfo& video_codec_info, PS_GenericCapability m4vcaps);
CodecCapabilityInfo* GetCodecCapabilityInfoMpeg4(PS_GenericCapability capability);
void FillUserInputCapability(CodecCapabilityInfo& codec_info, PS_UserInputCapability userinputCaps);
CodecCapabilityInfo* GetCodecCapabilityInfo(PS_UserInputCapability capability);
void ParseCapabilityIdentifierStandard(uint8* octet_string, uint16 octet_string_len, S_CapabilityIdentifierStandard& capabilityIdentifier);
unsigned GetMaxFrameRate(PS_DataType pDataType);
unsigned GetMaxFrameRate_H263(PS_H263VideoCapability h263caps);
unsigned GetMaxFrameRate_M4V(PS_GenericCapability m4vcaps);
unsigned GetMaxFrameRate_AVC(PS_GenericCapability avcCaps);
unsigned GetMaxBitrate(PS_DataType pDataType);
unsigned GetVideoFrameSize(PS_DataType pDataType, bool width);
unsigned GetVideoFrameSize_H263(PS_H263VideoCapability h263caps, bool width);
unsigned GetVideoFrameSize_M4V(PS_GenericCapability m4vcaps , bool width);

bool FindCodecForMediaType(PV2WayMediaType media, Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>&list, int* index);
bool IsSupported(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& list, PV2WayMediaType media, CodecCapabilityInfo& codec_info, int*index);
CodecCapabilityInfo* IsSupported(CodecCapabilityInfo* codecInfo,
                                 Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& list);
bool IsResolutionSupported(const PVMFVideoResolution& resolution, const Oscl_Vector<PVMFVideoResolutionRange, OsclMemAllocator>& resolutions);
unsigned GetSampleInterval(PS_DataType pDataType);
bool PVCheckSH(uint8 *ptr, int32 size);

PS_H223LogicalChannelParameters
GetH223LogicalChannelParameters(uint8 al_index,
                                bool segmentable,
                                uint32 al_specific = 0);
/** Returns 1 if only one media type is present in the ACS.
   Returns 2 if there are more than one media type
   **/
uint16 GetCodecCapabilityInfo(PS_TerminalCapabilitySet pTcs,
                              PS_AlternativeCapabilitySet pAcs,
                              Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& in_codecs_acs);
void Deallocate(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& cci);
PVMFStatus SetFormatSpecificInfo(PS_DataType pDataType, uint8* fsi, uint32 fsi_len);
uint32 SetFillerFsi(uint8* dest, uint32 dest_len);
bool IsFillerFsi(uint8* fsi, uint32 fsi_len);

/*
#ifdef __cplusplus
}
#endif*/

#define LOG_STATISTICS

#if defined(LOG_STATISTICS)
#define PV_STAT_SET(a, b)\
a  = b;
#else
#define PV_STAT_SET(a, b)
#endif

#if defined(LOG_STATISTICS)
#define PV_STAT_INCR(a, b)\
a  += b;
#else
#define PV_STAT_INCR(a, b)
#endif

#if defined(LOG_STATISTICS)
#define PV_STAT_INCR_COND(a, b, c)\
if(c){\
	a  += b;\
}
#else
#define PV_STAT_INCR_COND(a, b, c)
#endif

#if defined(LOG_STATISTICS)
#define PV_STAT_SET_TIME(a, cond)\
{\
if(cond == 0){\
	TimeValue timenow;\
	a = timenow;\
}\
}
#else
#define PV_STAT_SET_TIME(a, cond)
#endif

#endif	/* _h324utils_h */
