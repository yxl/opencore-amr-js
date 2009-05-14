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
#ifndef PVDL_CONFIG_FILE_H
#define PVDL_CONFIG_FILE_H

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

class PVDlCfgFile
{
    public:

        enum LoadConfigStatus
        {
            LoadConfigStatus_NoError = 0,
            LoadConfigStatus_CriticalError = -1, // object creation failure, open file failure
            LoadConfigStatus_NonCriticalError = -2 // read failure, url mismatch, headers unavailable
        };

        OSCL_IMPORT_REF PVDlCfgFile();
        OSCL_IMPORT_REF virtual ~PVDlCfgFile();

        void SetConfigFileName(OSCL_wString &aFileName)
        {
            iConfigFileName = aFileName;
        };
        OSCL_IMPORT_REF int32 LoadConfig(void);
        OSCL_IMPORT_REF bool SaveConfig(void);

        OSCL_IMPORT_REF void SetDownloadType(bool aIsFastTrack);
        OSCL_IMPORT_REF bool IsFastTrack(void);

        OSCL_IMPORT_REF void SetDonwloadComplete(void);
        OSCL_IMPORT_REF bool IsDownloadComplete(void);

        enum TPVDLPlaybackMode
        {
            EPVDL_ASAP = 0,
            EPVDL_PLAYBACK_AFTER_DOWNLOAD,
            EPVDL_DOWNLOAD_ONLY
        };
        OSCL_IMPORT_REF void SetPlaybackMode(TPVDLPlaybackMode aPlaybackMode);
        OSCL_IMPORT_REF TPVDLPlaybackMode GetPlaybackMode(void);

        void SetUrl(OSCL_String &aUrl)
        {
            iUrl = aUrl;
        };
        OSCL_String& GetUrl(void)
        {
            return iUrl;
        };

        void SetProxyName(OSCL_String &aProxyName)
        {
            iProxyName = aProxyName;
        };
        OSCL_String& GetProxyName(void)
        {
            return iProxyName;
        };

        void SetProxyPort(uint32 aProxyPort)
        {
            iProxyPort = aProxyPort;
        };
        uint32 GetProxyPort(void)
        {
            return iProxyPort;
        };

        void SetMaxAllowedFileSize(uint32 aFileSize)
        {
            iMaxAllowedFileSize = aFileSize;
        };
        uint32 GetMaxAllowedFileSize(void)
        {
            return iMaxAllowedFileSize;
        };

        void SetOverallFileSize(uint32 aFileSize)
        {
            iOverallFileSize = aFileSize;
        };
        uint32 GetOverallFileSize(void)
        {
            return iOverallFileSize;
        };

        void SetCurrentFileSize(uint32 aFileSize)
        {
            iCurrentFileSize = aFileSize;
        };
        uint32 GetCurrentFileSize(void)
        {
            return iCurrentFileSize;
        };

        void SetNetworkTimeouts(int32 aConnectTimeout, int32 aSendTimeout, int32 aRecvTimeout)
        {
            iConnectTimeout = aConnectTimeout;
            iSendTimeout = aSendTimeout;
            iRecvTimeout = aRecvTimeout;
        };
        void GetNetworkTimeouts(int32 &aConnectTimeout, int32 &aSendTimeout, int32 &aRecvTimeout)
        {
            aConnectTimeout = iConnectTimeout;
            aSendTimeout = iSendTimeout;
            aRecvTimeout = iRecvTimeout;
        };

        void SetRangeStartTime(uint32 aTimeInMillisec)
        {
            iRangeStartTime = aTimeInMillisec;
        };
        uint32 GetRangeStartTime(void)
        {
            return iRangeStartTime;
        };

        void SetSelectedFastTracks(Oscl_Vector<int32, OsclMemAllocator>& aSelectedTrackIDs)
        {
            iSelectedTrackIDs = aSelectedTrackIDs;
        }
        Oscl_Vector<int32, OsclMemAllocator>& GetSelectedFastTracks(void)
        {
            return iSelectedTrackIDs;
        }

        void SetPlayerVersion(OSCL_String &aPlayerVersion)
        {
            iPlayerVersion = aPlayerVersion;
        };

        void SetUserAgent(OSCL_String &aUserAgent)
        {
            iUserAgent = aUserAgent;
        };
        OSCL_String& GetUserAgent(void)
        {
            return iUserAgent;
        };

        void SetUserNetwork(OSCL_String &aUserNetwork)
        {
            iUserNetwork = aUserNetwork;
        };
        OSCL_String& GetUserNetwork(void)
        {
            return iUserNetwork;
        };

        void SetDeviceInfo(OSCL_String &aDeviceInfo)
        {
            iDeviceInfo = aDeviceInfo;
        };
        OSCL_String& GetDeviceInfo(void)
        {
            return iDeviceInfo;
        };

        void SetUserId(OSCL_String &aUserId)
        {
            iUserId = aUserId;
        };
        OSCL_String& GetUserId(void)
        {
            return iUserId;
        };

        void SetUserAuth(OSCL_String &aUserAuth)
        {
            iUserAuth = aUserAuth;
        };
        OSCL_String& GetUserAuth(void)
        {
            return iUserAuth;
        };

        void SetExpiration(OSCL_String &aExpiration)
        {
            iExpiration = aExpiration;
        };
        OSCL_String& GetExpiration(void)
        {
            return iExpiration;
        };

        void SetAppString(OSCL_String &aAppString)
        {
            iAppString = aAppString;
        };
        OSCL_String& GetAppString(void)
        {
            return iAppString;
        };

        void SetFiller(OSCL_String &aFiller)
        {
            iFiller = aFiller;
        };
        OSCL_String& GetFiller(void)
        {
            return iFiller;
        };

        void SetSign(OSCL_String &aSign)
        {
            iSign = aSign;
        };
        OSCL_String& GetSign(void)
        {
            return iSign;
        };

        void SetUnmodifiedDateStart(OSCL_String &aDate)
        {
            iUnmodifiedDateStart = aDate;
        };
        OSCL_String& GetUnmodifiedDateStart(void)
        {
            return iUnmodifiedDateStart;
        };

        void setHttpVersion(uint32 aVersion)
        {
            iHttpVersionNum = aVersion;
        }
        uint32 getHttpVersion()
        {
            return iHttpVersionNum;
        }

        //for integratity check and also argument passing
        void SetDownloadFileName(OSCL_wString &aFileName)
        {
            iDownloadFileName = aFileName;
        };
        OSCL_wString& GetDownloadFileName(void)
        {
            return iDownloadFileName;
        };
        OSCL_IMPORT_REF bool Validate();
        bool IsNewSession(void)
        {
            return bIsNewSession;
        };
        void SetNewSession(const bool aNewSession = true)
        {
            bIsNewSession = aNewSession;
        }

        bool HasContentLength() const
        {
            return (iHasContentLength > 0);
        }
        void setHasContentLengthFlag(const bool aHasContentLength = true)
        {
            iHasContentLength = (uint32)aHasContentLength;
        }

        void SetExtensionHeaderKey(OSCL_String &aKey)
        {
            OSCL_HeapString<OsclMemAllocator> fieldKey(aKey);
            int32 err = 0;
            OSCL_TRY(err, iExtensionHeaderKeys.push_back(fieldKey););
        }
        void SetExtensionHeaderValue(OSCL_String &aValue)
        {
            OSCL_HeapString<OsclMemAllocator> fieldValue(aValue);
            int32 err = 0;
            OSCL_TRY(err, iExtensionHeaderValues.push_back(fieldValue););
        }
        void SetHTTPMethodMaskForExtensionHeader(const uint32 bitMask)
        {
            int32 err = 0;
            OSCL_TRY(err, iMethodMaskForExtensionHeaders.push_back(bitMask););
        }
        void SetExtensionHeaderPurgeOnRediect(const bool aPurgeOnRedirect = true)
        {
            int32 err = 0;
            OSCL_TRY(err, iExtensionHeadersPurgeOnRedirect.push_back(aPurgeOnRedirect););
        }
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &getExtensionHeaderKeys()
        {
            return iExtensionHeaderKeys;
        }
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &getExtensionHeaderValues()
        {
            return iExtensionHeaderValues;
        }
        Oscl_Vector<uint32, OsclMemAllocator> &getHTTPMethodMasksForExtensionHeader()
        {
            return iMethodMaskForExtensionHeaders;
        }
        Oscl_Vector<bool, OsclMemAllocator> &getExtensionHeadersPurgeOnRedirect()
        {
            return iExtensionHeadersPurgeOnRedirect;
        }

        // flag of disabling HTTP HEAD request
        void setHttpHeadRequestDisabled(const bool aDisableHeadRequest = true)
        {
            iDisableHeadRequest = aDisableHeadRequest;
        }
        bool getHttpHeadRequestDisabled() const
        {
            return iDisableHeadRequest;
        }


    private:
        void composeFixedHeader(uint8 *aBuf);

    private:
        uint8 *iTmpBuf;

        Oscl_FileServer iFileServer;
        Oscl_File	*iFile;

        //memory allocator type for this node.
        typedef OsclMemAllocator PVDlCfgFileAllocator;

        OSCL_wHeapString<PVDlCfgFileAllocator> iConfigFileName;
        OSCL_wHeapString<PVDlCfgFileAllocator> iDownloadFileName;

        OSCL_HeapString<PVDlCfgFileAllocator> iUrl;
        //if proxy is in use, it is the proxy address
        OSCL_HeapString<PVDlCfgFileAllocator> iProxyName;
        //if proxy is in use, it is the proxy address len
        uint32	iProxyPort;

        //client only downloads the clip which is smaller than this size
        uint32 iMaxAllowedFileSize;
        //the file size after it is completly downloaded.
        uint32	iOverallFileSize;
        //for FastTrack, this would be the accumulated bytes downloaded
        uint32	iCurrentFileSize;
        //flag of whether to have content length for the previous download
        // boolean variable, but intentionally choose uint32 instead of bool, for consistency with other variables
        uint32 iHasContentLength;

        int32 iConnectTimeout;
        int32 iSendTimeout;
        int32 iRecvTimeout;

        uint32	iRangeStartTime; //in ms

        OSCL_HeapString<PVDlCfgFileAllocator> iPlayerVersion;
        OSCL_HeapString<PVDlCfgFileAllocator> iUserAgent;
        OSCL_HeapString<PVDlCfgFileAllocator> iUserNetwork;
        OSCL_HeapString<PVDlCfgFileAllocator> iDeviceInfo;
        OSCL_HeapString<PVDlCfgFileAllocator> iUserId;
        OSCL_HeapString<PVDlCfgFileAllocator> iUserAuth;
        OSCL_HeapString<PVDlCfgFileAllocator> iExpiration;
        OSCL_HeapString<PVDlCfgFileAllocator> iAppString;
        OSCL_HeapString<PVDlCfgFileAllocator> iFiller;
        OSCL_HeapString<PVDlCfgFileAllocator> iSign;
        OSCL_HeapString<PVDlCfgFileAllocator> iUnmodifiedDateStart;

        Oscl_Vector<int32, PVDlCfgFileAllocator> iSelectedTrackIDs;

        const uint32 iMagic32;
        const uint32 iVersion;
        uint32	iFlag;
        const uint32 iTotalFixedHeaderSize; //bytes from iMagic32 to iSignLen in bytes
        const uint32 PVDL_CFG_FILE_CACHE_BUF;
        PVLogger* iLogger;
        bool bIsNewSession;
        uint32 iHttpVersionNum;

        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iExtensionHeaderKeys;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iExtensionHeaderValues;
        Oscl_Vector<uint32, OsclMemAllocator> iMethodMaskForExtensionHeaders; // bit 0 = 1 => HTTP GET method
        // bit 1 = 1 => HTTP POST method
        Oscl_Vector<bool, OsclMemAllocator> iExtensionHeadersPurgeOnRedirect;
        bool iDisableHeadRequest;
};

#endif //#ifndef PVDL_CONFIG_FILE_H

