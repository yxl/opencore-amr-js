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

#ifndef SESSION_INFO_H
#define SESSION_INFO_H

#define FRESH_TOKEN_SIZE	30
#define OMADRM_ENC_KEY_DATA_SIZE	50
#define AUTH_TAG_SIZE	30


#include "oscl_types.h"
#include "oscl_string_containers.h"
#include "oscl_time.h"
#include "oscl_vector.h"
#include "rtsp_time_formats.h"
#include "rtsp_range_utils.h"
#include "sdp_error.h"
#include "sdp_memory.h"
#include "common_info.h"
#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif

typedef struct _originatorInfo
{
    OSCL_HeapString<SDPParserAlloc> originatorUsername;
    uint32 originatorSessionID;
    uint32 originatorVersion;
    OSCL_HeapString<SDPParserAlloc> originatorNetworkType;
    OSCL_HeapString<SDPParserAlloc> originatorAddressType;
    OSCL_HeapString<SDPParserAlloc> originatorAddress;



}originatorInfo;



typedef struct _sessionTime
{
    uint64 sessionStartTime;
    uint64 sessionStopTime;
}sessionTime;
enum EndOfClipAction { DEFAULT, CLOSE_PLAYER, LAUNCH_URL, DO_NOTHING };
enum VideoOnlyStatus { ALLOWED, NOT_ALLOWED, NOT_PRESENT };

enum SDPAltGroupType
{
    SDP_ALT_GROUP_NONE,
    SDP_ALT_GROUP_BANDWIDTH,
    SDP_ALT_GROUP_LANGUAGE
};

class sessionDescription
{
        SDPAltGroupType iSDPAltGroupType;

        OSCL_HeapString<SDPParserAlloc> type;

        int protocolVersion;

        originatorInfo origInfo;


        OSCL_HeapString<SDPParserAlloc> sessionName;

        OSCL_HeapString<SDPParserAlloc> sessionInformation;
        OSCL_HeapString<SDPParserAlloc> sessionURI;

        connectionInfo connectInfo;

        bool c_field_found;
        OSCL_HeapString<SDPParserAlloc> version;

        OSCL_HeapString<SDPParserAlloc> author;
        OSCL_HeapString<SDPParserAlloc> copyright;
        OSCL_HeapString<SDPParserAlloc> rating;


        uint32 repeatInterval;  // in seconds
        uint32 activeDuration;  // in seconds

        Oscl_Vector<uint32, SDPParserAlloc> listOfOffsetsFromStartTime;

        RtspRangeType session_range;

        OSCL_HeapString<SDPParserAlloc> wmfVersion;

        bool allowRecord;
        bool randomAccessDenied;
        VideoOnlyStatus videoOnlyAllowed;
        OSCL_HeapString<SDPParserAlloc> controlURL;

        OSCL_HeapString<SDPParserAlloc> creationDate;

        OSCL_HeapString<SDPParserAlloc> eTag;
        bool pauseDenied;
        //  char securityFlag;
        OSCL_HeapString<SDPParserAlloc> randomFiller;
        OSCL_HeapString<SDPParserAlloc> digitalSignature;
        OSCL_HeapString<SDPParserAlloc> launchURL;
        EndOfClipAction endOfClip;
        OSCL_HeapString<SDPParserAlloc> appSpecificString;
        OSCL_HeapString<SDPParserAlloc> expiryDate;
        OSCL_HeapString<SDPParserAlloc> email;
        OSCL_HeapString<SDPParserAlloc> phoneNumber;
        OSCL_HeapString<SDPParserAlloc> encryptionKey;


        // For ASF
        unsigned char *asfHeaderPtr;
        int asfHeaderSize;
        bool asfTrue;

        int bitrate;
        int rtcpBWSender;
        int rtcpBWReceiver;
        int BWtias;

        QoEMetricsType qoeMetrics;
        OSCL_HeapString<SDPParserAlloc> intKeyFreshToken;
        OSCL_HeapString<SDPParserAlloc> intKeyMethod;
        OSCL_HeapString<SDPParserAlloc> intKeyData;
        OSCL_HeapString<SDPParserAlloc> intKeyContentIdURI ;
        OSCL_HeapString<SDPParserAlloc> intKeyRightIssuerURI ;
        OSCL_HeapString<SDPParserAlloc> sdpAuthTag;

        AssetInfoType assetInfo;

        OSCL_HeapString<SDPParserAlloc> altGroupBW;
        OSCL_HeapString<SDPParserAlloc> altGroupLANG;
        OsclFloat maxprate;

        NTPTime sessionStartTime;
        NTPTime sessionStopTime;
        //End additions

        // added for RM
        uint32							SdpplinVersion;
        uint32							Flags;
        uint32							IsRealDataType;
        uint32							StreamCount;
        OSCL_HeapString<SDPParserAlloc> ASMRuleBook;
        OSCL_HeapString<SDPParserAlloc> Title;
        OSCL_HeapString<SDPParserAlloc> Keywords;
        OSCL_HeapString<SDPParserAlloc> Abstract;

        // packet source additions
        OSCL_HeapString<SDPParserAlloc> SdpFilename;
    public:
        sessionDescription()
        {
            resetSessionDescription();

        };
        ~sessionDescription()
        {
            OSCL_DELETE(asfHeaderPtr);
            asfHeaderPtr = NULL;
            assetInfo.CleanUp();
        };

        // added for RM

        inline void setSdpplinVersion(int version)
        {
            SdpplinVersion = version;
        };

        inline void setFlags(int flags)
        {
            Flags = flags;
        };

        inline void setStreamCount(int count)
        {
            StreamCount = count;
        };

        inline void setIsRealDataType(int isreal)
        {
            IsRealDataType = isreal;
        }

        inline void setKeywords(const OSCL_HeapString<SDPParserAlloc>& keywords)
        {
            Keywords = keywords;
        }

        inline void setKeywords(const OsclMemoryFragment memFrag)
        {
            Keywords.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setAbstract(const OSCL_HeapString<SDPParserAlloc>& abstract)
        {
            Abstract = abstract;
        }

        inline void setAbstract(const OsclMemoryFragment memFrag)
        {
            Abstract.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setASMRuleBook(const OSCL_HeapString<SDPParserAlloc>& rule)
        {
            ASMRuleBook = rule;
        }

        inline void setASMRuleBook(const OsclMemoryFragment memFrag)
        {
            ASMRuleBook.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setTitle(const OSCL_HeapString<SDPParserAlloc>& title)
        {
            Title = title;
        }

        inline void setTitle(const OsclMemoryFragment memFrag)
        {
            Title.set((const char*)(memFrag.ptr), memFrag.len);
        };

        // end RM

        inline void setProtocolVersion(int protocol)
        {
            protocolVersion = protocol;
        };
        inline void setSessionName(char* sName)
        {
            sessionName = sName;
        };
        inline void setSessionName(const OSCL_HeapString<SDPParserAlloc>& sName)
        {
            sessionName = sName;
        };
        inline void setSessionName(const OsclMemoryFragment memFrag)
        {
            sessionName.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setSessionInformation(char* sInfo)
        {
            sessionInformation = sInfo;
        };
        inline void setSessionInformation(const OSCL_HeapString<SDPParserAlloc>& sInfo)
        {
            sessionInformation = sInfo;
        };
        inline void setSessionInformation(const OsclMemoryFragment memFrag)
        {
            sessionInformation.set((const char*)(memFrag.ptr), memFrag.len);
        };
        inline void setSessionURI(const OsclMemoryFragment memFrag)
        {
            sessionURI.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setVersion(char *pvVersion)
        {
            version = pvVersion;
        };
        inline void setVersion(const OSCL_HeapString<SDPParserAlloc>& pvVersion)
        {
            version = pvVersion;
        };
        inline void setVersion(const OsclMemoryFragment memFrag)
        {
            version.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setAuthor(char* auth)
        {
            author = auth;
        };
        inline void setAuthor(const OSCL_HeapString<SDPParserAlloc>& auth)
        {
            author = auth;
        };
        inline void setAuthor(const OsclMemoryFragment memFrag)
        {
            author.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setCopyright(char* cpright)
        {
            copyright = cpright;
        };
        inline void setCopyright(const OSCL_HeapString<SDPParserAlloc>& cpright)
        {
            copyright = cpright;
        };
        inline void setCopyright(const OsclMemoryFragment memFrag)
        {
            copyright.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setRating(char* srating)
        {
            rating = srating;
        };
        inline void setRating(const OSCL_HeapString<SDPParserAlloc>& srating)
        {
            rating = srating;
        };
        inline void setRating(const OsclMemoryFragment memFrag)
        {
            rating.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setSessionStartTime(uint64 start)
        {
            sessionStartTime = start;
        };
        inline void setSessionStopTime(uint64 stop)
        {
            sessionStopTime = stop;
        };

        inline void setRepeatInterval(uint32 interval)
        {
            repeatInterval = interval;
        };
        inline void setActiveDuration(uint32 duration)
        {
            activeDuration = duration;
        };
        inline void addOffset(uint32 offset)
        {
            listOfOffsetsFromStartTime.push_back(offset);
        };
        inline void setOffsetVector(Oscl_Vector<uint32, SDPParserAlloc> offsetVec)
        {
            listOfOffsetsFromStartTime = offsetVec;
        };

        inline void setRange(const RtspRangeType& in_range)
        {
            session_range = in_range;
        }
        inline void setWmfVersion(char* wmfver)
        {
            wmfVersion = wmfver;
        };
        inline void setWmfVersion(const OSCL_HeapString<SDPParserAlloc>& wmfver)
        {
            wmfVersion = wmfver;
        };
        inline void setWmfVersion(const OsclMemoryFragment memFrag)
        {
            wmfVersion.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setAllowRecord(bool arecord)
        {
            allowRecord = arecord;
        };
        inline void setRandomAccessDenied(bool accessdenied)
        {
            randomAccessDenied = accessdenied;
        };
        inline void setVideoOnlyAllowed(VideoOnlyStatus videoallowed)
        {
            videoOnlyAllowed = videoallowed;
        };
        inline void setControlURL(char* ctrl)
        {
            controlURL = ctrl;
        };
        inline void setControlURL(const OSCL_HeapString<SDPParserAlloc>& ctrl)
        {
            controlURL = ctrl;
        };
        inline void setControlURL(const OsclMemoryFragment memFrag)
        {
            controlURL.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setOriginator() {};
        inline void setOUsername(char *username)
        {
            origInfo.originatorUsername = username;
        }
        inline void setOUsername(const OSCL_HeapString<SDPParserAlloc>& username)
        {
            origInfo.originatorUsername = username;
        }
        inline void setOUsername(const OsclMemoryFragment memFrag)
        {
            origInfo.originatorUsername.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setOSessionID(int OSID)
        {
            origInfo.originatorSessionID = OSID;
        };

        inline void setOVersion(int OSV)
        {
            origInfo.originatorVersion = OSV;
        };

        inline void setONetworkType(char* nType)
        {
            origInfo.originatorNetworkType = nType;
        };
        inline void setONetworkType(const OSCL_HeapString<SDPParserAlloc>& nType)
        {
            origInfo.originatorNetworkType = nType;
        };
        inline void setONetworkType(const OsclMemoryFragment memFrag)
        {
            origInfo.originatorNetworkType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setOAddressType(char* aType)
        {
            origInfo.originatorAddressType = aType;
        };
        inline void setOAddressType(const OSCL_HeapString<SDPParserAlloc>& aType)
        {
            origInfo.originatorAddressType = aType;
        };
        inline void setOAddressType(const OsclMemoryFragment memFrag)
        {
            origInfo.originatorAddressType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setOAddress(char* address)
        {
            origInfo.originatorAddress = address;
        };
        inline void setOAddress(const OSCL_HeapString<SDPParserAlloc>& address)
        {
            origInfo.originatorAddress = address;
        };
        inline void setOAddress(const OsclMemoryFragment memFrag)
        {
            origInfo.originatorAddress.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setCNetworkType(char *nType)
        {
            connectInfo.connectionNetworkType = nType;
        };
        inline void setCNetworkType(const OSCL_HeapString<SDPParserAlloc>& nType)
        {
            connectInfo.connectionNetworkType = nType;
        };
        inline void setCNetworkType(const OsclMemoryFragment memFrag)
        {
            connectInfo.connectionNetworkType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setCAddressType(char* aType)
        {
            connectInfo.connectionAddressType = aType;
        };
        inline void setCAddressType(const OSCL_HeapString<SDPParserAlloc>& aType)
        {
            connectInfo.connectionAddressType = aType;
        };
        inline void setCAddressType(const OsclMemoryFragment memFrag)
        {
            connectInfo.connectionAddressType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setCAddress(char* address)
        {
            connectInfo.connectionAddress = address;
        };
        inline void setCAddress(const OSCL_HeapString<SDPParserAlloc>& address)
        {
            connectInfo.connectionAddress = address;
        };
        inline void setCAddress(const OsclMemoryFragment memFrag)
        {
            connectInfo.connectionAddress.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setCreationDate(char *cDate)
        {
            creationDate = cDate;
        };
        inline void setCreationDate(const OSCL_HeapString<SDPParserAlloc>& cDate)
        {
            creationDate = cDate;
        };
        inline void setCreationDate(const OsclMemoryFragment memFrag)
        {
            creationDate.set((const char*)(memFrag.ptr), memFrag.len);
        };


        inline void setETag(char *et)
        {
            eTag = et;
        }
        inline void setETag(const OSCL_HeapString<SDPParserAlloc>& et)
        {
            eTag = et;
        }
        inline void setETag(const OsclMemoryFragment memFrag)
        {
            eTag.set((const char*)(memFrag.ptr), memFrag.len);
        }
        inline void setRandomFiller(char *rf)
        {
            randomFiller = rf;
        }
        inline void setRandomFiller(const OSCL_HeapString<SDPParserAlloc>& rf)
        {
            randomFiller = rf;
        }
        inline void setRandomFiller(const OsclMemoryFragment memFrag)
        {
            randomFiller.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setDigitalSignature(char *ds)
        {
            digitalSignature = ds;
        }
        inline void setDigitalSignature(const OSCL_HeapString<SDPParserAlloc>& ds)
        {
            digitalSignature = ds;
        }
        inline void setDigitalSignature(const OsclMemoryFragment memFrag)
        {
            digitalSignature.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setLaunchURL(char *lu)
        {
            launchURL = lu;
        }
        inline void setLaunchURL(const OSCL_HeapString<SDPParserAlloc>& lu)
        {
            launchURL = lu;
        }
        inline void setLaunchURL(const OsclMemoryFragment memFrag)
        {
            launchURL.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setEndOfClipAction(EndOfClipAction eoc)
        {
            endOfClip = eoc;
        }

        inline void setAppSpecificString(char *app_spec_str)
        {
            appSpecificString = app_spec_str;
        }
        inline void setAppSpecificString(const OSCL_HeapString<SDPParserAlloc>& app_spec_str)
        {
            appSpecificString = app_spec_str;
        }
        inline void setAppSpecificString(const OsclMemoryFragment memFrag)
        {
            appSpecificString.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setExpiryDate(char *exp_date)
        {
            expiryDate = exp_date;
        }
        inline void setExpiryDate(const OSCL_HeapString<SDPParserAlloc>& exp_date)
        {
            expiryDate = exp_date;
        }
        inline void setExpiryDate(const OsclMemoryFragment memFrag)
        {
            expiryDate.set((const char*)(memFrag.ptr), memFrag.len);
        }
        inline void setPauseDenied(bool pDenied)
        {
            pauseDenied = pDenied;
        }
        inline void setEmail(char *em)
        {
            email = em;
        }
        inline void setEmail(const OSCL_HeapString<SDPParserAlloc>& em)
        {
            email = em;
        }
        inline void setEmail(const OsclMemoryFragment memFrag)
        {
            email.set((const char*)(memFrag.ptr), memFrag.len);
        }
        inline void setPhoneNumber(char *number)
        {
            phoneNumber = number;
        }
        inline void setPhoneNumber(const OSCL_HeapString<SDPParserAlloc>& number)
        {
            phoneNumber = number;
        }
        inline void setPhoneNumber(const OsclMemoryFragment memFrag)
        {
            phoneNumber.set((const char*)(memFrag.ptr), memFrag.len);
        }
        inline void setEncryptionKey(char *key)
        {
            encryptionKey = key;
        }
        inline void setEncryptionKey(const OSCL_HeapString<SDPParserAlloc>& key)
        {
            encryptionKey = key;
        }
        inline void setEncryptionKey(const OsclMemoryFragment memFrag)
        {
            encryptionKey.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setBitrate(int bRate)
        {
            bitrate = bRate;
        };
        inline void setRTCPSenderBitRate(int bRate)
        {
            rtcpBWSender = bRate;
        };
        inline void setRTCPReceiverBitRate(int bRate)
        {
            rtcpBWReceiver = bRate;
        };

        inline void setBWtias(int bRate)
        {
            BWtias = bRate;
        }

        inline void setQoEMetrics(const QoEMetricsType &qMetrics)
        {
            qoeMetrics = qMetrics;
        };

        inline void setKeyMethod(const char *keymethod, int size)
        {
            intKeyMethod.set(keymethod, size);
        }

        inline void setKeyMethod(const OSCL_HeapString<SDPParserAlloc> &keymethod)
        {
            intKeyMethod = keymethod;
        }


        inline void setKeyData(const char *keydata, int size)
        {
            intKeyData.set(keydata, size);

        }

        inline void setKeyData(const OSCL_HeapString<SDPParserAlloc> &keydata)
        {
            intKeyData = keydata;

        }

        inline void setKeyContentIdURI(const char* contURI, int size)
        {
            intKeyContentIdURI.set(contURI, size);
        }

        inline void setKeyContentIdURI(const OSCL_HeapString<SDPParserAlloc> &contURI)
        {
            intKeyContentIdURI = contURI;
        }

        inline void setKeyRightIssuerURI(const char* issuerURI, int size)
        {
            intKeyRightIssuerURI.set(issuerURI, size);
        }
        inline void setKeyRightIssuerURI(const OSCL_HeapString<SDPParserAlloc> &issuerURI)
        {
            intKeyRightIssuerURI = issuerURI;
        }


        inline void setKeyFreshToken(const char *token, int size)
        {
            intKeyFreshToken.set(token, size);
        }
        inline void setKeyFreshToken(const OSCL_HeapString<SDPParserAlloc> &key)
        {
            intKeyFreshToken = key;
        }

        inline void setSdpAuthTag(const OsclMemoryFragment memFrag)
        {
            sdpAuthTag.set((const char *)memFrag.ptr, memFrag.len);
        }

        inline void setSdpAuthTag(const char *tag)
        {
            sdpAuthTag = tag;
        }

        inline void setSdpAuthTag(const OSCL_HeapString<SDPParserAlloc>& tag)
        {
            sdpAuthTag = tag;
        }

        inline void setAssetInfo(const AssetInfoType &ainfo)
        {
            assetInfo = ainfo;
        };

        inline SDPAltGroupType getSDPAltGroupType()
        {
            return iSDPAltGroupType;
        }
        inline void setSDPAltGroupType(SDPAltGroupType aSDPAltGroupType)
        {
            iSDPAltGroupType = aSDPAltGroupType;
        };
        inline void setAltGroupBW(const char *line, int size)
        {
            altGroupBW.set(line, size);
            setSDPAltGroupType(SDP_ALT_GROUP_BANDWIDTH);
        }
        inline void setAltGroupBW(const OSCL_HeapString<SDPParserAlloc>& line)
        {
            altGroupBW = line;
            setSDPAltGroupType(SDP_ALT_GROUP_BANDWIDTH);
        }
        inline void setAltGroupLANG(const char *line, int size)
        {
            altGroupLANG.set(line, size);
            setSDPAltGroupType(SDP_ALT_GROUP_LANGUAGE);
        }
        inline void setAltGroupLANG(const OSCL_HeapString<SDPParserAlloc>& line)
        {
            altGroupLANG = line;
            setSDPAltGroupType(SDP_ALT_GROUP_LANGUAGE);
        }

        inline void setCFieldStatus(bool status)
        {
            c_field_found = status;
        }

        inline void setMaxprate(OsclFloat rate)
        {
            maxprate = rate;
        }
        //End additions

        inline char *getType()
        {
            return (char *)type.get_cstr();
        };
        inline int getProtocolVersion()
        {
            return protocolVersion;
        };
        inline void getOriginator(originatorInfo* orig)
        {
            orig->originatorUsername = origInfo.originatorUsername;
            orig->originatorSessionID = origInfo.originatorSessionID;
            orig->originatorVersion = origInfo.originatorVersion;
            orig->originatorNetworkType = origInfo.originatorNetworkType;
            orig->originatorAddressType = origInfo.originatorAddressType;
            orig->originatorAddress = origInfo.originatorAddress;
        }

        inline const char *getOUserName()
        {
            return origInfo.originatorUsername.get_cstr();
        };
        inline uint32 getOSessionID()
        {
            return origInfo.originatorSessionID;
        }

        //End additions
        inline char *getSessionName()
        {
            return (char *)sessionName.get_cstr();
        };
        inline void getConnectionInformation(connectionInfo* ct)
        {
            ct->connectionNetworkType = connectInfo.connectionNetworkType;
            ct->connectionAddress = connectInfo.connectionAddress;
            ct->connectionAddressType = connectInfo.connectionAddressType;
        }
        inline const char *getSessionInformation()
        {
            return sessionInformation.get_cstr();
        };
        inline const char *getVersion() // Modify this to return char
        {
            return version.get_cstr();
        };
        inline const char *getAuthor()
        {
            return author.get_cstr();
        };
        inline const char *getCopyright()
        {
            return copyright.get_cstr();
        };
        inline const char *getRating()
        {
            return rating.get_cstr();
        };
        inline void getStartStopTimes(sessionTime* st)
        {
            st->sessionStartTime = sessionStartTime.get_value();
            st->sessionStopTime = sessionStopTime.get_value();
        }
        inline NTPTime getStartTime()
        {
            return sessionStartTime;
        }
        inline NTPTime getStopTime()
        {
            return sessionStopTime;
        }
        inline void getStartStopTimes(uint64 *start, uint64 *stop)
        {
            *start = sessionStartTime.get_value();
            *stop = sessionStopTime.get_value();
        };

        // added for RM
        inline const char *getASMRuleBool()
        {
            return ASMRuleBook.get_cstr();
        }

        inline const char *getTitle()
        {
            return Title.get_cstr();
        }

        inline const char *getKeywords()
        {
            return Keywords.get_cstr();
        }

        inline const char *getAbstract()
        {
            return Abstract.get_cstr();
        }

        inline uint32 getSdpplinVersion()
        {
            return SdpplinVersion;
        }

        inline uint32 getStreamCount()
        {
            return StreamCount;
        }

        inline uint32 getFlags()
        {
            return Flags;
        }

        inline uint32 getIsRealDataType()
        {
            return IsRealDataType;
        }

        // end
        inline uint32 convertNTPValueToMilliSeconds(uint64 ntp_value)
        {
            uint32 ntp_value_upper =
                Oscl_Int64_Utils::get_uint64_upper32(ntp_value);

            uint32 ntp_value_lower =
                Oscl_Int64_Utils::get_uint64_lower32(ntp_value);

            MediaClockConverter mediaClockConverter;
            /*
             * Convert Time stamp to milliseconds
             */
            mediaClockConverter.update_clock(ntp_value_upper);
            uint32 ntp_value_upper_ms =
                mediaClockConverter.get_converted_ts(1000);

            /*
             * lower 32 bits are actually in a time scale of 2^32
             * limit precision of lower_32 to just 10 bits
             */
            ntp_value_lower >>= 22;
            mediaClockConverter.set_timescale(1024);
            mediaClockConverter.update_clock(ntp_value_lower);
            uint32 ntp_value_lower_ms =
                mediaClockConverter.get_converted_ts(1000);

            return (ntp_value_upper_ms + ntp_value_lower_ms);
        }

        inline uint32 getSessionDurationInMilliSeconds()
        {
            NTPTime startTimeNTP = getStartTime();
            NTPTime stopTimeNTP  = getStopTime();

            uint64 duration_64 = (stopTimeNTP.get_value() -
                                  startTimeNTP.get_value());

            return (convertNTPValueToMilliSeconds(duration_64));
        }

        inline uint32 getSessionStartTimeInMilliSeconds()
        {
            NTPTime startTimeNTP = getStartTime();
            uint64 session_start = startTimeNTP.get_value();
            return (convertNTPValueToMilliSeconds(session_start));
        }

        inline uint32 getSessionStopTimeInMilliSeconds()
        {
            NTPTime stopTimeNTP  = getStopTime();
            uint64 session_stop = stopTimeNTP.get_value();
            return (convertNTPValueToMilliSeconds(session_stop));
        }

        inline uint32 getSessionRepeatInterval()
        {
            return (repeatInterval);
        };
        inline uint32 getSessionActiveDuration()
        {
            return (activeDuration);
        };
        inline Oscl_Vector<uint32, SDPParserAlloc> getRepeatTimeOffsetVectors()
        {
            return (listOfOffsetsFromStartTime);
        };

        inline const RtspRangeType* getRange()
        {
            return &session_range;
        }
        inline const char *getWmfVersion()
        {
            return wmfVersion.get_cstr();
        };
        inline bool getAllowRecord()
        {
            return allowRecord;
        };
        inline bool getRandomAccessDenied()
        {
            return randomAccessDenied;
        };
        inline VideoOnlyStatus getVideoOnlyAllowed()
        {
            return videoOnlyAllowed;
        };
        inline const char *getControlURL()
        {
            return controlURL.get_cstr();
        };
        inline const char *getCreationDate()
        {
            return creationDate.get_cstr();
        }



        inline const char *getETag()
        {
            return eTag.get_cstr();
        }

        inline const char *getRandomFiller()
        {
            return randomFiller.get_cstr();
        }

        inline const char *getDigitalSignature()
        {
            return digitalSignature.get_cstr();
        }

        inline const char *getLaunchURL()
        {
            return launchURL.get_cstr();
        }

        inline EndOfClipAction getEndOfClipAction()
        {
            return endOfClip;
        }

        inline const char *getAppSpecificString()
        {
            return appSpecificString.get_cstr();
        }

        inline const char *getExpiryDate()
        {
            return expiryDate.get_cstr();
        }

        inline bool getPauseDenied()
        {
            return pauseDenied;
        }
        inline const char *getEmail()
        {
            return email.get_cstr();
        }
        inline const char *getPhoneNumber()
        {
            return phoneNumber.get_cstr();
        }
        inline const char* getEncryptionKey()
        {
            return encryptionKey.get_cstr();
        }

        inline int getBitRate()
        {
            return (bitrate);
        }
        inline int getRTCPBWSender()
        {
            return (rtcpBWSender);
        }
        inline int getRTCPBWReceiver()
        {
            return (rtcpBWReceiver);
        }
        inline int getBWtias()
        {
            return BWtias;
        }

        inline QoEMetricsType &getQoEMetrics()
        {
            return qoeMetrics ;
        }

        inline AssetInfoType &getAssetInfo()
        {
            return assetInfo;
        };

        inline const char* getAltGroupBW()
        {
            return altGroupBW.get_cstr();
        }
        inline const char* getAltGroupLANG()
        {
            return altGroupLANG.get_cstr();
        }
        inline int getAltGroupBWLength()
        {
            return altGroupBW.get_size();
        }
        inline int getAltGroupLANGLength()
        {
            return altGroupLANG.get_size();
        }
        inline const char* getSdpAuthTag()
        {
            return sdpAuthTag.get_cstr();
        }

        inline bool getCFieldStatus()
        {
            return c_field_found;
        }
        inline const char* getSessionURI()
        {
            return sessionURI.get_cstr();
        }
        inline const char* getintKeyMethod()
        {
            return intKeyMethod.get_cstr();
        }
        inline const char* getintKeyFreshToken()
        {
            return intKeyFreshToken.get_cstr();
        }
        inline const char* getintKeyData()
        {
            return intKeyData.get_cstr();
        }
        inline const char* getintKeyRightIssuerURI()
        {
            return intKeyRightIssuerURI.get_cstr();
        }
        inline const char* getintKeyContentIdURI()
        {
            return intKeyContentIdURI.get_cstr();
        }
        inline OsclFloat getMaxprate()
        {
            return maxprate;
        }
        //End additions

        //Additions for asf streaming
        inline void setAsfHeader(unsigned char* ASCPtr, int ASCLen)
        {
            asfHeaderPtr = ASCPtr;
            asfHeaderSize = ASCLen;
        };
        inline unsigned char *getAsfHeader()
        {
            return asfHeaderPtr;
        }
        inline int getAsfHeaderSize()
        {
            return asfHeaderSize;
        }
        inline void setAsfTrue(bool asfFlag)
        {
            asfTrue = asfFlag;
        }
        inline bool getAsfTrue()
        {
            return asfTrue;
        }

        inline void setSDPFilename(const OSCL_HeapString<SDPParserAlloc>& filename)
        {
            SdpFilename = filename;
        }

        inline void setSDPFilename(const OsclMemoryFragment memFrag)
        {
            SdpFilename.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void resetSessionDescription()
        {
            // NTPTime time; // used to initialize the originatorSessionID

            protocolVersion = 0;
            rating = "Not Rated";

            sessionStartTime = (uint64)0;
            sessionStopTime = (uint64)0;

            sessionName = "N/A";
            sessionInformation = "N/A";
            version = "N/A";
            author = "N/A";
            copyright = "N/A";

            wmfVersion = "1.2";

            allowRecord = false;
            randomAccessDenied = false;
            videoOnlyAllowed = NOT_PRESENT;
            controlURL = "*";

            session_range.format = RtspRangeType::INVALID_RANGE;
            session_range.start_is_set = session_range.end_is_set = false;

            origInfo.originatorUsername = "-";
            //origInfo.originatorSessionID =  time.get_upper32();
            //origInfo.originatorVersion = time.get_lower32();
            origInfo.originatorSessionID = (uint32)0;
            origInfo.originatorVersion = (uint32) 0;
            origInfo.originatorNetworkType = "IN";
            origInfo.originatorAddressType = "IP4";
            origInfo.originatorAddress = "0.0.0.0";

            connectInfo.connectionNetworkType = "IP";
            connectInfo.connectionAddress = "0.0.0.0";
            connectInfo.connectionAddressType = "IP4";

            //    securityFlag = 0;
            pauseDenied = false;
            eTag = NULL;
            randomFiller = NULL;
            digitalSignature = NULL;
            launchURL = NULL;
            //Added new enum state.
            endOfClip = DO_NOTHING;
            appSpecificString = NULL;
            email = "support@localhost";
            //End additions

            // ASF
            asfHeaderSize = 0;
            asfHeaderPtr = NULL;
            asfTrue = false;

            bitrate = 0;
            rtcpBWSender = -1;
            rtcpBWReceiver = -1;
            BWtias = 0;
            repeatInterval = 0;
            activeDuration = 0;

            intKeyFreshToken = NULL;
            intKeyMethod = NULL;
            intKeyData = NULL;
            intKeyContentIdURI = NULL;
            intKeyRightIssuerURI = NULL;
            sdpAuthTag = NULL;

            assetInfo.ResetAssetInfoType();

            iSDPAltGroupType = SDP_ALT_GROUP_NONE;
            altGroupBW = NULL;
            altGroupLANG = NULL;
            c_field_found = false;
            oscl_memset(qoeMetrics.name, 0, 7);
            qoeMetrics.rateFmt = QoEMetricsType::VAL;
            qoeMetrics.rateVal = 0;
            qoeMetrics.paramFmt = QoEMetricsType::IDIGIT;
            qoeMetrics.paramExtIdigit = 0;

        }

        sessionDescription(const sessionDescription & sourceSessionDescription)
        {
            setProtocolVersion(((sessionDescription)sourceSessionDescription).getProtocolVersion());
            setSessionName(((sessionDescription)sourceSessionDescription).getSessionName());
            setSessionInformation(((sessionDescription)sourceSessionDescription).getSessionInformation());
            setVersion(((sessionDescription)sourceSessionDescription).getVersion());
            setAuthor(((sessionDescription)sourceSessionDescription).getAuthor());
            setCopyright(((sessionDescription)sourceSessionDescription).getCopyright());
            setRating(((sessionDescription)sourceSessionDescription).getRating());
            uint64 start, stop;
            ((sessionDescription)sourceSessionDescription).getStartStopTimes(&start, &stop);
            setSessionStartTime(start);
            setSessionStopTime(stop);

            setRepeatInterval(((sessionDescription)sourceSessionDescription).getSessionRepeatInterval());
            setActiveDuration(((sessionDescription)sourceSessionDescription).getSessionActiveDuration());
            setOffsetVector(((sessionDescription)sourceSessionDescription).getRepeatTimeOffsetVectors());

            setRange(*((sessionDescription)sourceSessionDescription).getRange());
            setWmfVersion(((sessionDescription)sourceSessionDescription).getWmfVersion());
            setAllowRecord(((sessionDescription)sourceSessionDescription).getAllowRecord());
            setVideoOnlyAllowed(((sessionDescription)sourceSessionDescription).getVideoOnlyAllowed());
            setControlURL(((sessionDescription)sourceSessionDescription).getControlURL());

            originatorInfo originator;
            ((sessionDescription)sourceSessionDescription).getOriginator(&originator);
            setOUsername(originator.originatorUsername);
            //Check the following 2 fields for validity
            setOSessionID(originator.originatorSessionID);
            setOVersion(originator.originatorVersion);
            setONetworkType(originator.originatorNetworkType);
            setOAddressType(originator.originatorAddressType);
            setOAddress(originator.originatorAddress);

            connectionInfo connection;
            ((sessionDescription)sourceSessionDescription).getConnectionInformation(&connection);
            setCNetworkType(connection.connectionNetworkType);
            setCAddressType(connection.connectionAddressType);
            setCAddress(connection.connectionAddress);

            setCreationDate(((sessionDescription)sourceSessionDescription).getCreationDate());
            setRandomAccessDenied(((sessionDescription)sourceSessionDescription).getRandomAccessDenied());
            setETag(((sessionDescription)sourceSessionDescription).getETag());
            setRandomFiller(((sessionDescription)sourceSessionDescription).getRandomFiller());
            setDigitalSignature(((sessionDescription)sourceSessionDescription).getDigitalSignature());
            setLaunchURL(((sessionDescription)sourceSessionDescription).getLaunchURL());
            setEndOfClipAction(((sessionDescription)sourceSessionDescription).getEndOfClipAction());
            setAppSpecificString(((sessionDescription)sourceSessionDescription).getAppSpecificString());
            setExpiryDate(((sessionDescription)sourceSessionDescription).getExpiryDate());
            setPauseDenied(((sessionDescription)sourceSessionDescription).getPauseDenied());
            setEmail(((sessionDescription)sourceSessionDescription).getEmail());
            setPhoneNumber(((sessionDescription)sourceSessionDescription).getPhoneNumber());
            setEncryptionKey(((sessionDescription)sourceSessionDescription).getEncryptionKey());

            setBitrate(((sessionDescription)sourceSessionDescription).getBitRate());
            setRTCPSenderBitRate(((sessionDescription)sourceSessionDescription).getRTCPBWSender());
            setRTCPReceiverBitRate(((sessionDescription)sourceSessionDescription).getRTCPBWReceiver());
            setBWtias(((sessionDescription)sourceSessionDescription).getBWtias());
            //Additions for asf streaming
            setAsfHeader(((sessionDescription)sourceSessionDescription).getAsfHeader(), ((sessionDescription)sourceSessionDescription).getAsfHeaderSize()) ;
            setAsfTrue(((sessionDescription)sourceSessionDescription).getAsfTrue());

            setQoEMetrics(sourceSessionDescription.qoeMetrics);

            setKeyMethod(sourceSessionDescription.intKeyMethod);

            setKeyData(sourceSessionDescription.intKeyData);

            setKeyContentIdURI(sourceSessionDescription.intKeyContentIdURI);
            setKeyRightIssuerURI(sourceSessionDescription.intKeyRightIssuerURI);

            setAssetInfo(((sessionDescription)sourceSessionDescription).getAssetInfo());

            setSdpAuthTag(((sessionDescription)sourceSessionDescription).getSdpAuthTag());

            setSDPAltGroupType(((sessionDescription)sourceSessionDescription).getSDPAltGroupType());
            setAltGroupBW(((sessionDescription)sourceSessionDescription).getAltGroupBW());
            setAltGroupLANG(((sessionDescription)sourceSessionDescription).getAltGroupLANG());

            setCFieldStatus(((sessionDescription)sourceSessionDescription).getCFieldStatus());
            setMaxprate(((sessionDescription)sourceSessionDescription).getMaxprate());

        }

        const sessionDescription &operator=(const sessionDescription & sourceSessionDescription)
        {
            protocolVersion = sourceSessionDescription.protocolVersion;
            sessionName = sourceSessionDescription.sessionName;
            sessionInformation = sourceSessionDescription.sessionInformation;
            version = sourceSessionDescription.version;
            author = sourceSessionDescription.author;
            copyright = sourceSessionDescription.copyright;
            rating = sourceSessionDescription.rating;
            sessionStartTime = sourceSessionDescription.sessionStartTime.get_value();
            sessionStopTime = sourceSessionDescription.sessionStopTime.get_value();

            repeatInterval = sourceSessionDescription.repeatInterval;
            activeDuration = sourceSessionDescription.activeDuration;
            listOfOffsetsFromStartTime = sourceSessionDescription.listOfOffsetsFromStartTime;

            session_range = sourceSessionDescription.session_range;
            wmfVersion = sourceSessionDescription.wmfVersion;
            allowRecord = sourceSessionDescription.allowRecord;
            videoOnlyAllowed = sourceSessionDescription.videoOnlyAllowed;
            controlURL = sourceSessionDescription.controlURL;
            origInfo.originatorUsername = sourceSessionDescription.origInfo.originatorUsername;
            origInfo.originatorSessionID = sourceSessionDescription.origInfo.originatorSessionID;
            origInfo.originatorVersion = sourceSessionDescription.origInfo.originatorVersion;
            origInfo.originatorNetworkType = sourceSessionDescription.origInfo.originatorNetworkType;
            origInfo.originatorAddressType = sourceSessionDescription.origInfo.originatorAddressType;
            origInfo.originatorAddress = sourceSessionDescription.origInfo.originatorAddress;
            connectInfo.connectionNetworkType = sourceSessionDescription.connectInfo.connectionNetworkType;
            connectInfo.connectionAddressType = sourceSessionDescription.connectInfo.connectionAddressType;
            connectInfo.connectionAddress = sourceSessionDescription.connectInfo.connectionAddress;
            creationDate = sourceSessionDescription.creationDate;
            randomAccessDenied = sourceSessionDescription.randomAccessDenied;
            eTag = 	sourceSessionDescription.eTag;
            randomFiller = sourceSessionDescription.randomFiller;
            digitalSignature = sourceSessionDescription.digitalSignature;
            launchURL = sourceSessionDescription.launchURL;
            endOfClip = sourceSessionDescription.endOfClip;
            appSpecificString = sourceSessionDescription.appSpecificString;
            expiryDate = sourceSessionDescription.expiryDate;
            pauseDenied = sourceSessionDescription.pauseDenied;
            email = sourceSessionDescription.email;
            phoneNumber = sourceSessionDescription.phoneNumber;
            encryptionKey = sourceSessionDescription.encryptionKey;
            bitrate = sourceSessionDescription.bitrate;
            rtcpBWSender = 	sourceSessionDescription.rtcpBWSender;
            rtcpBWReceiver = sourceSessionDescription.rtcpBWReceiver;
            BWtias = sourceSessionDescription.BWtias;
            asfHeaderPtr = sourceSessionDescription.asfHeaderPtr;
            asfHeaderSize = sourceSessionDescription.asfHeaderSize;
            asfTrue = sourceSessionDescription.asfTrue;
            qoeMetrics = sourceSessionDescription.qoeMetrics;
            Keywords = 	sourceSessionDescription.Keywords;
            intKeyData = sourceSessionDescription.intKeyData;
            intKeyContentIdURI = sourceSessionDescription.intKeyContentIdURI;
            intKeyRightIssuerURI = sourceSessionDescription.intKeyRightIssuerURI;
            assetInfo = sourceSessionDescription.assetInfo;
            sdpAuthTag = sourceSessionDescription.sdpAuthTag;
            iSDPAltGroupType = sourceSessionDescription.iSDPAltGroupType;
            c_field_found = sourceSessionDescription.c_field_found;
            maxprate = sourceSessionDescription.maxprate;
            altGroupBW = sourceSessionDescription.altGroupBW;
            altGroupLANG = sourceSessionDescription.altGroupLANG;
            iSDPAltGroupType = sourceSessionDescription.iSDPAltGroupType;
            return *this;
        }

};

#endif
