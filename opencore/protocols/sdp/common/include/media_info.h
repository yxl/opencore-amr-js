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

#ifndef MEDIAINFO_H
#define MEDIAINFO_H
#include "sdp_memory.h"
#include "oscl_string_containers.h"
#include "rtsp_time_formats.h"
#include "rtsp_range_utils.h"
#include "sdp_error.h"
#include "sdp_memory.h"
#include "common_info.h"
#include "oscl_vector.h"
#include "oscl_shared_ptr.h"
#include "oscl_refcounter.h"
#include "aac_payload_info.h"
#include "amr_payload_info.h"
#include "pcmu_payload_info.h"
#include "pcma_payload_info.h"
#include "h263_payload_info.h"
#include "h264_payload_info.h"
#include "m4v_payload_info.h"
#include "rfc3640_payload_info.h"


struct SRTPConfigAttrType
{
    OSCL_HeapString<SDPParserAlloc> intg_nonce;
    OSCL_HeapString<SDPParserAlloc> srtp_key_salt;
    uint32 auth_tag_len ;
    OSCL_HeapString<SDPParserAlloc> srtp_param_ext;

};

struct rtcp_fbType
{
    enum { ACK, NACK, TRR_INT, FB_ID };
    OSCL_HeapString<SDPParserAlloc> pt;
    OSCL_HeapString<SDPParserAlloc> val;
    int val_type;

    OSCL_HeapString<SDPParserAlloc> ack_param;
    OSCL_HeapString<SDPParserAlloc> nack_param;
    OSCL_HeapString<SDPParserAlloc> param;
    uint32 trr_int;

};

class mediaInfo
{
    protected:
        int mediaTrackId;
        Oscl_Vector<int, SDPParserAlloc> alternateTrackId;
        Oscl_Vector<int, SDPParserAlloc> dependentTrackId;
        OSCL_HeapString<SDPParserAlloc> type;

        int suggestedPort;
        int numOfPorts;
        OSCL_HeapString<SDPParserAlloc> transportProfile;

        Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfoVector;

        OSCL_HeapString<SDPParserAlloc> MIMEType; // code support multiple rtpmap with same MIMEType.

        OSCL_HeapString<SDPParserAlloc> controlURL;

        int controlURLTrackID;
        uint32 mediaInfoID;
        int bitrate;
        int rtcpBWSender;
        int rtcpBWReceiver;
        int BWtias;
        RtspRangeType range;


        int parameter_size;
        OSCL_HeapString<SDPParserAlloc> dependsonURL;

        int	DependsOnTrackID;	// ID of track for DependsOn field.
        bool allowRecord;		// Allows recording of media type.

        bool select;			//Used for track selection
        int ssrc;

        bool ssrcIsSet;

        bool randomAccessDenied; // Added for random positioning on 3rd party content - 01/08/02

        QoEMetricsType qoeMetrics;
        uint32 predecbufsize;
        uint32 initpredecbufperiod;
        uint32 initpostdecbufperiod;
        uint32 decbyterate;

        uint32 reportFrequency;
        AssetInfoType assetInfo;

        connectionInfo connectInfo;
        bool c_field_found ;
        SRTPConfigAttrType SRTPConfigAttr;

        rtcp_fbType rtcp_fb;
        OsclFloat maxprate;

        int totalStringLength;
        int segmentNumber;
        int payloadPreference;
        bool matched;
        bool segmentActive;

    public:
        mediaInfo()
        {
            suggestedPort = 0;
            numOfPorts = 0;
            bitrate = 0;
            rtcpBWSender = -1;
            rtcpBWReceiver = -1;
            BWtias = 0;
            allowRecord = false;
            select = false;
            parameter_size = 0;
            controlURLTrackID = 0;
            mediaInfoID = 0;
            totalStringLength = 0;
            DependsOnTrackID = -1;
            ssrc = 0;

            range.format = RtspRangeType::INVALID_RANGE;
            range.start_is_set = false;
            range.end_is_set = false;
            ssrcIsSet = false;
            randomAccessDenied = false;

            oscl_memset(qoeMetrics.name, 0, MAX_METRICS_NAME);
            qoeMetrics.range.format = RtspRangeType::INVALID_RANGE;
            qoeMetrics.range.start_is_set = false;
            qoeMetrics.range.end_is_set = false;
            predecbufsize = 0;
            initpredecbufperiod = 0;
            initpostdecbufperiod = 0;
            decbyterate = 0;

            reportFrequency = 0;
            assetInfo.URL = NULL;
            for (int ii = 0; ii < ASSET_NAME_SIZE;ii++)
                assetInfo.Box[ii] = NULL;

            connectInfo.connectionNetworkType = "IP";
            connectInfo.connectionAddress = "0.0.0.0";
            connectInfo.connectionAddressType = "IP4";
            c_field_found = false;

            SRTPConfigAttr.intg_nonce = NULL;
            SRTPConfigAttr.srtp_key_salt = NULL;
            SRTPConfigAttr.auth_tag_len = 0;
            SRTPConfigAttr.srtp_param_ext = NULL;
            mediaTrackId = 0;
            rtcp_fb.pt = NULL;
            rtcp_fb.val = NULL;
            rtcp_fb.ack_param = NULL;
            rtcp_fb.nack_param = NULL;
            rtcp_fb.param = NULL;
            rtcp_fb.trr_int = 0;
            oscl_memset(qoeMetrics.name, 0, 7);
            qoeMetrics.rateFmt = QoEMetricsType::VAL;
            qoeMetrics.rateVal = 0;
            qoeMetrics.paramFmt = QoEMetricsType::IDIGIT;
            qoeMetrics.paramExtIdigit = 0;
            maxprate = 0;
            segmentNumber = 0;
            payloadPreference = 0;
            matched = false;
            segmentActive = true;
        };

        virtual ~mediaInfo()
        {
            int size = payloadSpecificInfoVector.size();
            for (int ii = 0; ii < size ; ii++)
            {
                payloadSpecificInfoVector[ii]->~PayloadSpecificInfoTypeBase();
                dealloc(payloadSpecificInfoVector[ii]);
            }
            assetInfo.CleanUp();
        };

        void *alloc(const int size)
        {
            void *mem = oscl_malloc(size * sizeof(char));
            if (mem != NULL)
            {
                payloadSpecificInfoVector.push_back((PayloadSpecificInfoTypeBase*)mem);
            }
            return mem;
        };

        inline void dealloc(void *ptr)
        {
            oscl_free(ptr);
        };

        inline PayloadSpecificInfoTypeBase* getPayloadSpecificInfoTypePtr(uint32 number)
        {
            PayloadSpecificInfoTypeBase* retVal = NULL;
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getPayloadNumber() == number)
                {
                    retVal = payloadSpecificInfoVector[ii];
                    break;
                }
            }
            return retVal;
        };

        inline bool lookupPayloadNumber(uint32 number, int &pPosition)
        {
            for (int ii = 0; ii < (int)payloadSpecificInfoVector.size();ii++)
            {
                if (payloadSpecificInfoVector[ii]->getPayloadNumber() == number)
                {
                    pPosition = ii;
                    return true;
                }
            }

            return false;
        };

        inline void resetAlternateTrackId()
        {
            int size = alternateTrackId.size();
            for (int ii = 0; ii < size; ii++)
                alternateTrackId.pop_back();
        }

        inline void resetDependentTrackId()
        {
            int size = dependentTrackId.size();
            for (int ii = 0; ii < size ; ii++)
                dependentTrackId.pop_back();
        }

        inline void setSSRC(int ss)
        {
            ssrc = ss;
            ssrcIsSet = true;
        }
        inline void setType(char* typ)
        {
            type = typ;
        };
        inline void setType(const OSCL_HeapString<SDPParserAlloc>& typ)
        {
            type = typ;
        };

        inline void setType(const OsclMemoryFragment memFrag)
        {
            type.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setSuggestedPort(int port)
        {
            suggestedPort = port;
        };
        inline void setNumOfPorts(int ports)
        {
            numOfPorts = ports;
        };

        inline void setTransportProfile(char* tProfile)
        {
            transportProfile = tProfile;
        };
        inline void setTransportProfile(const OSCL_HeapString<SDPParserAlloc>& tProfile)
        {
            transportProfile = tProfile;
        };
        inline void setTransportProfile(const OsclMemoryFragment memFrag)
        {
            transportProfile.set((const char*)(memFrag.ptr), memFrag.len);
        };

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
        inline void setParameterSize(int pSize)
        {
            parameter_size = pSize;
        };
        inline void setControlURL(char* CURL)
        {
            controlURL = CURL;
        };
        inline void setControlURL(const OSCL_HeapString<SDPParserAlloc>& CURL)
        {
            controlURL = CURL;
        };
        inline void setControlURL(const OsclMemoryFragment memFrag)
        {
            controlURL.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setDependsonURL(char* DURL)
        {
            dependsonURL = DURL;
        };
        inline void setDependsonURL(const OSCL_HeapString<SDPParserAlloc>& DURL)
        {
            dependsonURL = DURL;
        };
        inline void setDependsonURL(const OsclMemoryFragment memFrag)
        {
            dependsonURL.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setpayloadSpacificInfoVector(const Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc>& pInfo)
        {
            payloadSpecificInfoVector = pInfo;
        };

        inline void setMIMEType(const char* MType)
        {
            MIMEType = MType;
        };
        inline void setMIMEType(const OSCL_HeapString<SDPParserAlloc>& MType)
        {
            MIMEType = MType;
        };
        inline void setMIMEType(const OsclMemoryFragment memFrag)
        {
            MIMEType.set((const char*)(memFrag.ptr), memFrag.len);
        };

        inline void setControlTrackID(int tID)
        {
            controlURLTrackID = tID;
        };
        inline void setMediaInfoID(uint32 id)
        {
            mediaInfoID = id;
        };

        inline void setAllowRecord(bool allrec)
        {
            allowRecord = allrec;
        };

        inline void setRange(const RtspRangeType& in_range)
        {
            range = in_range;
        };
        inline void setSelect()
        {
            select = true;
        };
        inline void resetSelect()
        {
            select = false;
        };
        inline void setDependsOnTrackID(int trackID)
        {
            DependsOnTrackID = trackID;
            totalStringLength += oscl_strlen("a=depends-on:") + SDP_INT_STRING_LENGTH;
        };


        inline virtual void setAudioSpecificConfig(unsigned char* /*ASCPtr*/, int /*ASCLen*/) {};
        inline void setRandomAccessDenied(bool accessdenied)
        {
            randomAccessDenied = accessdenied;
        };
        inline void setQoEMetrics(const QoEMetricsType& qMetrics)
        {
            qoeMetrics = qMetrics;
        }

        inline void setPreDecBuffSize(uint32 size)
        {
            predecbufsize = size;
        }
        inline void setInitPreDecBuffPeriod(uint32 period)
        {
            initpredecbufperiod = period;
        }
        inline void setInitPostDecBuffPeriod(uint32 period)
        {
            initpostdecbufperiod = period;
        }
        inline void setDecByteRate(uint32 rate)
        {
            decbyterate = rate;
        }

        inline void setReportFrequency(uint32 freq)
        {
            reportFrequency = freq;
        }
        inline void setAssetInfo(const AssetInfoType &ainfo)
        {
            assetInfo = ainfo;
        }
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

        inline void setCFieldStatus(bool status)
        {
            c_field_found = status;
        }

        inline void setSRTPintg_nonce(char *intg)
        {
            SRTPConfigAttr.intg_nonce = intg;
        }
        inline void setSRTPintg_nonce(const OSCL_HeapString<SDPParserAlloc>& intg)
        {
            SRTPConfigAttr.intg_nonce = intg;
        }
        inline void setSRTPintg_nonce(const OsclMemoryFragment memFrag)
        {
            SRTPConfigAttr.intg_nonce.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setSRTPkey_salt(char *key)
        {
            SRTPConfigAttr.srtp_key_salt = key;
        }
        inline void setSRTPkey_salt(const OSCL_HeapString<SDPParserAlloc>& key)
        {
            SRTPConfigAttr.srtp_key_salt = key;
        }
        inline void setSRTPkey_salt(const OsclMemoryFragment memFrag)
        {
            SRTPConfigAttr.srtp_key_salt.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setSRTPauth_tag_len(int len)
        {
            SRTPConfigAttr.auth_tag_len = len;
        }

        inline void setSRTPparam_ext(char *ext)
        {
            SRTPConfigAttr.srtp_param_ext = ext;
        }
        inline void setSRTPparam_ext(const OSCL_HeapString<SDPParserAlloc>& ext)
        {
            SRTPConfigAttr.srtp_param_ext = ext;
        }
        inline void setSRTPparam_ext(const OsclMemoryFragment memFrag)
        {
            SRTPConfigAttr.srtp_param_ext.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setmediaTrackId(int id)
        {
            mediaTrackId = id;
        }
        inline void setdependentTrackId(int id)
        {
            dependentTrackId.push_back(id);
        }
        inline void setalternateTrackId(int id)
        {
            alternateTrackId.push_back(id);
        }

        inline void setrtcp_fb(rtcp_fbType fb)
        {
            rtcp_fb = fb;
        }
        inline void setrtcp_fb_pt(const OsclMemoryFragment memFrag)
        {
            rtcp_fb.pt.set((const char*)(memFrag.ptr), memFrag.len);
        }

        inline void setrtcp_fb_val(const OsclMemoryFragment memFrag)
        {
            rtcp_fb.val.set((const char*)(memFrag.ptr), memFrag.len);
            if (!oscl_strncmp(rtcp_fb.val.get_str(), "ack", rtcp_fb.val.get_size()))
                rtcp_fb.val_type = rtcp_fbType::ACK;
            else if (!oscl_strncmp(rtcp_fb.val.get_str(), "nack", rtcp_fb.val.get_size()))
                rtcp_fb.val_type = rtcp_fbType::NACK;
            else if (!oscl_strncmp(rtcp_fb.val.get_str(), "trr-int", rtcp_fb.val.get_size()))
                rtcp_fb.val_type = rtcp_fbType::TRR_INT;
            else
                rtcp_fb.val_type = rtcp_fbType::FB_ID;
        }

        inline void setrtcp_fb_trr_val(uint32 trr)
        {
            rtcp_fb.trr_int = trr;
        }
        inline void setrtcp_fb_val_param(const OsclMemoryFragment memFrag)
        {
            if (rtcp_fb.val_type == (rtcp_fbType::ACK))
                rtcp_fb.ack_param.set((const char*)(memFrag.ptr), memFrag.len);
            else if (rtcp_fb.val_type == rtcp_fbType::NACK)
                rtcp_fb.nack_param.set((const char*)(memFrag.ptr), memFrag.len);
            else if (rtcp_fb.val_type == rtcp_fbType::FB_ID)
                rtcp_fb.param.set((const char*)(memFrag.ptr), memFrag.len);

        }

        inline void setMaxprate(OsclFloat rate)
        {
            maxprate = rate;
        }
        inline void setSegmentNumber(int segment)
        {
            segmentNumber = segment;
        };
        inline void setPayloadPreference(int pref)
        {
            payloadPreference = pref;
        };
        inline void setMatched(bool match)
        {
            matched = match;
        };
        inline void setSegmentActive(bool status)
        {
            segmentActive = status;
        };

        inline bool getSelect()
        {
            return select;
        };
        inline const char *getType()
        {
            return type.get_cstr();
        };
        inline int getSuggestedPort()
        {
            return suggestedPort;
        };
        inline int getNumOfPorts()
        {
            return numOfPorts;
        };
        inline const char *getTransportProfile()
        {
            return transportProfile.get_cstr();
        };
        inline int getBitrate()
        {
            return bitrate;
        };
        inline int getParameterSize()
        {
            return parameter_size;
        };
        inline const char *getControlURL()
        {
            return controlURL.get_cstr();
        };
        inline const char *getDependsonTrackID()
        {
            return dependsonURL.get_cstr();
        };
        inline const char *getDependsonURL()
        {
            return dependsonURL.get_cstr();
        };
        inline Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> getPayloadSpecificInfoVector()
        {
            return (payloadSpecificInfoVector);
        }
        inline int getMediaPayloadNumberCount()
        {
            return payloadSpecificInfoVector.size();
        }
        inline uint32 getMediaPayloadNumber(int pos)
        {
            return payloadSpecificInfoVector[pos]->getPayloadNumber();
        }
        inline const char *getMIMEType()
        {
            return MIMEType.get_cstr();
        };
        inline virtual const unsigned char *getAudioSpecificConfig(int*size)
        {
            *size = 0;
            return NULL;
        };
        inline virtual const void *getStreamMuxConfig()
        {
            return NULL;
        };
        inline uint32 getMediaInfoID()
        {
            return mediaInfoID;
        }
        inline int getControlTrackID()
        {
            return controlURLTrackID;
        };
        inline int getDependsOnTrackID()
        {
            return DependsOnTrackID;
        };
        inline int getSSRC()
        {
            return ssrc;
        }
        inline bool getSSRCFlag()
        {
            return ssrcIsSet;
        }
        inline virtual int getProfileLevelID()
        {
            return 0;
        };
        inline virtual unsigned char *getVOLHeader()
        {
            return NULL;
        };


        inline virtual double getFrameRate()
        {
            return 0.0;
        }
        inline virtual int getIFrameInterval()
        {
            return 0;
        }
        inline virtual char *getScalability()
        {
            return 0;
        }
        inline virtual bool getDRC()
        {
            return false;
        };
        inline virtual int getCodecProfile()
        {
            return 0;
        };
        inline virtual int getCodecLevel()
        {
            return 0;
        };

        inline virtual int getCodecModeList()
        {
            return 0;
        };
        inline virtual int getModeChangePeriod()
        {
            return 0;
        };
        inline virtual bool getModeChangeNeighbor()
        {
            return false;
        };
        inline virtual int getMaximumFrames()
        {
            return 0;
        };
        inline virtual bool getRobustSorting()
        {
            return false;
        };

        inline virtual bool getOctetAlign()
        {
            return false;
        };
        inline virtual bool getCRC()
        {
            return false;
        };
        inline virtual int getInterLeaving()
        {
            return 0;
        };
        inline virtual const char *getLang()
        {
            return NULL;
        };

        inline virtual int getMaximumBundle()
        {
            return 0;
        };
        inline virtual int getPacketTime()
        {
            return 0;
        };

        inline virtual bool getAllowRecord()
        {
            return allowRecord;
        };

        inline bool getRandomAccessDenied()
        {
            return randomAccessDenied;
        };

        inline void IncrementTotalStringLength(int inc)
        {
            if (inc > 0) totalStringLength += inc;
        };
        inline RtspRangeType *getRtspRange()
        {
            return &range;
        };

        inline uint32 getPreDecBuffSize()
        {
            return predecbufsize;
        }
        inline uint32 getInitPreDecBuffPeriod()
        {
            return initpredecbufperiod;
        }
        inline uint32 getInitPostDecBuffPeriod()
        {
            return initpostdecbufperiod;
        }
        inline uint32 getDecByteRate()
        {
            return decbyterate ;
        }

        inline QoEMetricsType& getQoEMetrics()
        {
            return qoeMetrics;
        }
        inline uint32 getReportFrequency()
        {
            return reportFrequency;
        }
        inline AssetInfoType &getAssetInfo()
        {
            return assetInfo;
        }


        inline void getConnectionInformation(connectionInfo* ct)
        {
            ct->connectionNetworkType = connectInfo.connectionNetworkType;
            ct->connectionAddress = connectInfo.connectionAddress;
            ct->connectionAddressType = connectInfo.connectionAddressType;
        }

        inline bool getCFieldStatus()
        {
            return c_field_found;
        }

        inline int getRTCPReceiverBitRate()
        {
            return rtcpBWReceiver;
        }
        inline int getRTCPSenderBitRate()
        {
            return rtcpBWSender;
        }
        inline int getBWtias()
        {
            return BWtias;
        }
        inline int getmediaTrackId()
        {
            return mediaTrackId;
        }

        inline Oscl_Vector<int , SDPParserAlloc> getdependentTrackId()
        {
            return dependentTrackId;
        }
        inline Oscl_Vector<int, SDPParserAlloc> getalternateTrackId()
        {
            return alternateTrackId;
        }
        inline rtcp_fbType getrtcp_fb()
        {
            return rtcp_fb;
        }

        inline SRTPConfigAttrType *getSRTPConfigAttr()
        {
            return &SRTPConfigAttr ;
        }

        inline OsclFloat getMaxprate()
        {
            return maxprate;
        }
        inline uint32 getSegmentNumber()
        {
            return segmentNumber;
        };
        inline uint32 getPayloadPreference()
        {
            return payloadPreference;
        };
        inline bool isMatched()
        {
            return matched;
        };
        inline bool isSegmentActive()
        {
            return segmentActive;
        }


        mediaInfo(const mediaInfo &pSource)
        {
            setSSRC(pSource.ssrc);
            setType(pSource.type);
            setSuggestedPort(pSource.suggestedPort);
            setNumOfPorts(pSource.numOfPorts);
            setTransportProfile(pSource.transportProfile);
            setBitrate(pSource.bitrate);
            setRTCPSenderBitRate(pSource.rtcpBWSender);
            setRTCPReceiverBitRate(pSource.rtcpBWReceiver);
            setBWtias(pSource.BWtias);
            setParameterSize(pSource.parameter_size);
            setControlURL(pSource.controlURL);
            setDependsonURL(pSource.dependsonURL);
            setMIMEType(pSource.MIMEType);

//		setpayloadSpacificInfoVector(pSource.payloadSpecificInfoVector);
            setSampleRateForPayloads(pSource.payloadSpecificInfoVector);
            setNumOfChannelsForPayloads(pSource.payloadSpecificInfoVector);

            setControlTrackID(pSource.controlURLTrackID);
            setMediaInfoID(pSource.mediaInfoID);
            setAllowRecord(pSource.allowRecord);
            setRange(pSource.range);

            if (pSource.select == true)
            {
                setSelect();
            }
            else
            {
                resetSelect();
            }
            setDependsOnTrackID(pSource.DependsOnTrackID);

            setRandomAccessDenied(pSource.randomAccessDenied);

            setQoEMetrics(pSource.qoeMetrics);
            setPreDecBuffSize(pSource.predecbufsize);
            setInitPreDecBuffPeriod(pSource.initpredecbufperiod);
            setInitPostDecBuffPeriod(pSource.initpostdecbufperiod);
            setDecByteRate(pSource.decbyterate);

            setReportFrequency(pSource.reportFrequency);
            setAssetInfo((pSource.assetInfo));

            connectionInfo connInf = pSource.connectInfo;
            setCNetworkType(connInf.connectionNetworkType);
            setCAddressType(connInf.connectionAddressType);
            setCAddress(connInf.connectionAddress);
            setCFieldStatus(pSource.c_field_found);

            SRTPConfigAttrType srtp_config = pSource.SRTPConfigAttr;
            setSRTPintg_nonce(srtp_config.intg_nonce);
            setSRTPkey_salt(srtp_config.srtp_key_salt);
            setSRTPauth_tag_len(srtp_config.auth_tag_len);
            setSRTPparam_ext(srtp_config.srtp_param_ext);
            mediaTrackId = pSource.mediaTrackId;
            alternateTrackId = pSource.alternateTrackId;
            dependentTrackId = pSource.dependentTrackId;
            rtcp_fb = pSource.rtcp_fb;
            setMaxprate(pSource.maxprate);
            setSegmentNumber(pSource.segmentNumber);
            setPayloadPreference(pSource.payloadPreference);
            setMatched(pSource.matched);
            setSegmentActive(pSource.segmentActive);

        }

        const mediaInfo & operator=(const mediaInfo &pSource)
        {
            if (&pSource != this)
            {
                setSSRC(pSource.ssrc);
                setType(pSource.type);
                setSuggestedPort(pSource.suggestedPort);
                setNumOfPorts(pSource.numOfPorts);
                setTransportProfile(pSource.transportProfile);
                setBitrate(pSource.bitrate);
                setRTCPSenderBitRate(pSource.rtcpBWSender);
                setRTCPReceiverBitRate(pSource.rtcpBWReceiver);
                setBWtias(pSource.BWtias);
                setParameterSize(pSource.parameter_size);
                setControlURL(pSource.controlURL);
                setDependsonURL(pSource.dependsonURL);

                setMIMEType(pSource.MIMEType);
//			setpayloadSpacificInfoVector(pSource.payloadSpecificInfoVector);
                setSampleRateForPayloads(pSource.payloadSpecificInfoVector);
                setNumOfChannelsForPayloads(pSource.payloadSpecificInfoVector);

                setControlTrackID(pSource.controlURLTrackID);
                setAllowRecord(pSource.allowRecord);
                setRange(pSource.range);
                if (pSource.select == true)
                {
                    setSelect();
                }
                else
                {
                    resetSelect();
                }
                setDependsOnTrackID(pSource.DependsOnTrackID);

                setRandomAccessDenied(pSource.randomAccessDenied);

                setQoEMetrics(pSource.qoeMetrics);
                setPreDecBuffSize(pSource.predecbufsize);
                setInitPreDecBuffPeriod(pSource.initpredecbufperiod);
                setInitPostDecBuffPeriod(pSource.initpostdecbufperiod);
                setDecByteRate(pSource.decbyterate);

                setReportFrequency(pSource.reportFrequency);
                setAssetInfo((pSource.assetInfo));

                connectionInfo connInf = pSource.connectInfo;
                setCNetworkType(connInf.connectionNetworkType);
                setCAddressType(connInf.connectionAddressType);
                setCAddress(connInf.connectionAddress);

                setCFieldStatus(pSource.c_field_found);

                SRTPConfigAttrType srtp_config = pSource.SRTPConfigAttr;
                setSRTPintg_nonce(srtp_config.intg_nonce);
                setSRTPkey_salt(srtp_config.srtp_key_salt);
                setSRTPauth_tag_len(srtp_config.auth_tag_len);
                setSRTPparam_ext(srtp_config.srtp_param_ext);
                mediaTrackId = pSource.mediaTrackId;
                alternateTrackId = pSource.alternateTrackId;
                dependentTrackId = pSource.dependentTrackId;
                rtcp_fb = pSource.rtcp_fb;
                setMaxprate(pSource.maxprate);
                setSegmentNumber(pSource.segmentNumber);
                setPayloadPreference(pSource.payloadPreference);
                setMatched(pSource.matched);
                setSegmentActive(pSource.segmentActive);
            }
            return *this;
        }

        bool setSampleRateForPayloads(Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc> payloadSpecificInfo)
        {
            if (payloadSpecificInfoVector.size() == payloadSpecificInfo.size())
            {
                for (uint32 ii = 0; ii < payloadSpecificInfo.size(); ii++)
                {
                    payloadSpecificInfoVector[ii]->setSampleRate(payloadSpecificInfo[ii]->getSampleRate());
                }
                return true;
            }
            else
                return false;
        }

        bool setNumOfChannelsForPayloads(Oscl_Vector<PayloadSpecificInfoTypeBase*, SDPParserAlloc>payloadSpecificInfo)
        {
            if (payloadSpecificInfoVector.size() == payloadSpecificInfo.size())
            {
                for (uint32 ii = 0; ii < payloadSpecificInfo.size(); ii++)
                {
                    payloadSpecificInfoVector[ii]->setNoOfChannels(payloadSpecificInfo[ii]->getNumberOfChannels());
                }
                return true;
            }
            else
                return false;
        }


};
#endif
