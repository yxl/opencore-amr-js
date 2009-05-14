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
#include "pvdl_config_file.h"
#include "pvmf_protocolengine_node_tunables.h"

#define PVDLCONFIGFILE_VECTOR_RESERVE_NUMBER	4
#define PVDLCONFIGFILE_TEMPORARY_BUFFER_SIZE	4096
#define PVDLCONFIGFILE_FIXED_HEADER_SIZE		100 // 96+4
#define PVDLCONFIGFILE_FILE_CACHE_BUFFER_SIZE	1024


OSCL_EXPORT_REF PVDlCfgFile::PVDlCfgFile()
        : iTmpBuf(NULL)
        , iFile(NULL)
        , iProxyPort(0)
        , iMaxAllowedFileSize(0)
        , iOverallFileSize(0)
        , iCurrentFileSize(0)
        , iHasContentLength(1)
        , iConnectTimeout(0)
        , iSendTimeout(0)
        , iRecvTimeout(0)
        , iRangeStartTime(0)
        , iMagic32(0x4a6a446c)
        , iVersion(1)
        , iFlag(0)
        , iTotalFixedHeaderSize(PVDLCONFIGFILE_FIXED_HEADER_SIZE)
        , PVDL_CFG_FILE_CACHE_BUF(PVDLCONFIGFILE_FILE_CACHE_BUFFER_SIZE)
        , iLogger(NULL)
        , bIsNewSession(true)
        , iHttpVersionNum(PDL_HTTP_VERSION_NUMBER) // assume 0 => Http 1.0 ; 1 => Http 1.1
        , iDisableHeadRequest(false)
{
    int32 err;
    OSCL_TRY(err,
             iFileServer.Connect();
             iTmpBuf = OSCL_ARRAY_NEW(uint8, iTotalFixedHeaderSize);
             iLogger = PVLogger::GetLoggerObject("PVDlCfgFile");
             iExtensionHeaderKeys.reserve(PVDLCONFIGFILE_VECTOR_RESERVE_NUMBER);
             iExtensionHeaderValues.reserve(PVDLCONFIGFILE_VECTOR_RESERVE_NUMBER);
             iMethodMaskForExtensionHeaders.reserve(PVDLCONFIGFILE_VECTOR_RESERVE_NUMBER);
             iExtensionHeadersPurgeOnRedirect.reserve(PVDLCONFIGFILE_VECTOR_RESERVE_NUMBER);
            );

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
}

OSCL_EXPORT_REF PVDlCfgFile::~PVDlCfgFile()
{
    //SaveConfig();

    if (iTmpBuf)
    {
        //OSCL_ARRAY_DELETE(iTmpBuf);
        OSCL_ARRAY_DELETE(iTmpBuf);
        iTmpBuf = NULL;
    }

    if (iFile)
    {
        iFile->Flush();
        iFile->Close();
        OSCL_DELETE(iFile);
        iFile = NULL;
    }

    iFileServer.Close();
    iExtensionHeaderKeys.clear();
    iExtensionHeaderValues.clear();
    iExtensionHeadersPurgeOnRedirect.clear();
}

OSCL_EXPORT_REF void PVDlCfgFile::SetDownloadType(bool aIsFastTrack)
{
    if (aIsFastTrack)
    {
        iFlag	|= 0x1;
    }
    else
    {
        iFlag	&= (~0x1);
    }
}

OSCL_EXPORT_REF bool PVDlCfgFile::IsFastTrack(void)
{
    return (iFlag & 0x1);
}

OSCL_EXPORT_REF void PVDlCfgFile::SetDonwloadComplete(void)
{
    iFlag	|=	0x2;
}

OSCL_EXPORT_REF void PVDlCfgFile::SetPlaybackMode(TPVDLPlaybackMode aPlaybackMode)
{
    iFlag	&=	(~0xC);	//clear
    uint32 playbackModeBit = OSCL_STATIC_CAST(uint32, aPlaybackMode);
    iFlag	|=	(playbackModeBit << 2);
}

OSCL_EXPORT_REF PVDlCfgFile::TPVDLPlaybackMode PVDlCfgFile::GetPlaybackMode(void)
{
    return OSCL_STATIC_CAST(TPVDLPlaybackMode, ((iFlag & 0xC) >> 2));
}

OSCL_EXPORT_REF bool PVDlCfgFile::SaveConfig(void)
{
    if (iConfigFileName.get_size() <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
        return false;
    }

    if (iFile)
    {
        iFile->Close();
    }
    else
    {
        int32 err;
        OSCL_TRY(err, iFile = OSCL_NEW(Oscl_File, (PVDL_CFG_FILE_CACHE_BUF)););
        if ((err != OsclErrNone) || (iFile == NULL))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVDlCfgFile::SaveConfig() OSCL_NEW ERROR. line %d ", __LINE__));
            return false;
        }
    }

    int32 retval = iFile->Open(iConfigFileName.get_cstr(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, iFileServer);
    if (retval)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
        return false;
    }

    //fixed len part
    composeFixedHeader(iTmpBuf);

    int32 tmpRet = iFile->Write(iTmpBuf, 1, iTotalFixedHeaderSize);
    if (tmpRet == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
        return false;
    }

    //var len part
    uint8 *tmpBuf = iTmpBuf;
    if (iSelectedTrackIDs.size())
    {
        for (int32 i = iSelectedTrackIDs.size() - 1; i >= 0; i--)
        {//Track ID should only use one byte.
            *tmpBuf++ = OSCL_STATIC_CAST(uint8, iSelectedTrackIDs[i]);
        }
        tmpRet = iFile->Write(iTmpBuf, 1, iSelectedTrackIDs.size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }


    if (iUrl.get_size())
    {
        tmpRet = iFile->Write(iUrl.get_str(), 1, iUrl.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iProxyName.get_size())
    {
        tmpRet = iFile->Write(iProxyName.get_str(), 1, iProxyName.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iPlayerVersion.get_size())
    {
        tmpRet = iFile->Write(iPlayerVersion.get_str(), 1, iPlayerVersion.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iUserAgent.get_size())
    {
        tmpRet = iFile->Write(iUserAgent.get_str(), 1, iUserAgent.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iUserNetwork.get_size())
    {
        tmpRet = iFile->Write(iUserNetwork.get_str(), 1, iUserNetwork.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iDeviceInfo.get_size())
    {
        tmpRet = iFile->Write(iDeviceInfo.get_str(), 1, iDeviceInfo.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iUserId.get_size())
    {
        tmpRet = iFile->Write(iUserId.get_str(), 1, iUserId.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iUserAuth.get_size())
    {
        tmpRet = iFile->Write(iUserAuth.get_str(), 1, iUserAuth.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iExpiration.get_size())
    {
        tmpRet = iFile->Write(iExpiration.get_str(), 1, iExpiration.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iAppString.get_size())
    {
        tmpRet = iFile->Write(iAppString.get_str(), 1, iAppString.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iFiller.get_size())
    {
        tmpRet = iFile->Write(iFiller.get_str(), 1, iFiller.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    if (iSign.get_size())
    {
        tmpRet = iFile->Write(iSign.get_str(), 1, iSign.get_size());
        if (tmpRet == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::SaveConfig() ERROR. line %d ", __LINE__));
            return false;
        }
    }

    iFile->Flush();
    return true;
}

OSCL_EXPORT_REF int32 PVDlCfgFile::LoadConfig(void)
{
    bIsNewSession = false;

    if (iConfigFileName.get_size() <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
        return LoadConfigStatus_CriticalError;
    }

    if (iFile)
    {
        iFile->Close();
    }
    else
    {
        int32 err;
        OSCL_TRY(err, iFile = OSCL_NEW(Oscl_File, (PVDL_CFG_FILE_CACHE_BUF)););
        if ((err != OsclErrNone) || (iFile == NULL))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVDlCfgFile::LoadConfig() OSCL_NEW ERROR. line %d ", __LINE__));
            return LoadConfigStatus_CriticalError;
        }
    }

    int32 retval = iFile->Open(iConfigFileName.get_cstr(), Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFileServer);
    if (retval)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
        return LoadConfigStatus_NonCriticalError; // file might not exist yet
    }

    if (iTotalFixedHeaderSize != iFile->Read(iTmpBuf, 1, iTotalFixedHeaderSize))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
        return LoadConfigStatus_NonCriticalError;
    };

    {
        uint32 *tmpPtr = OSCL_STATIC_CAST(uint32*, iTmpBuf);
        if (iMagic32 != *tmpPtr++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
            return LoadConfigStatus_NonCriticalError;
        }
        if (iVersion != *tmpPtr++)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
            return LoadConfigStatus_NonCriticalError;
        }

        //flag for download type 3gpp/fasttrack, download complete, and playback modes
        iFlag = *tmpPtr++;
        uint32 aUrlLen = *tmpPtr++;

        uint32 aHostNameLen = *tmpPtr++;	//if proxy is in use, it is the proxy address len
        iProxyPort = *tmpPtr++;

        //client only downloads the clip which is smaller than this size
        uint32 aMaxAllowedFileSize = *tmpPtr++;
        if (/*iMaxAllowedFileSize==0 && */aMaxAllowedFileSize > 0) iMaxAllowedFileSize = aMaxAllowedFileSize;

        //the file size after it is completly downloaded.
        iOverallFileSize = *tmpPtr++;
        //for FastTrack, this would be the accumulated bytes downloaded
        iCurrentFileSize = *tmpPtr++;
        if (iOverallFileSize == 0 ||
                iMaxAllowedFileSize < iOverallFileSize ||
                iOverallFileSize < iCurrentFileSize)
        {
            return LoadConfigStatus_NonCriticalError;
        }
        // for content-length flag
        iHasContentLength = *tmpPtr++;

        iConnectTimeout = *tmpPtr++;
        iSendTimeout = *tmpPtr++;
        iRecvTimeout = *tmpPtr++;

        //FastTrack only
        iRangeStartTime = *tmpPtr++; //in ms
        uint32 aSelectedTrackIDsSize = *tmpPtr++;

        uint32 aPlayerVersionLen = *tmpPtr++;
        uint32 aUserAgentLen = *tmpPtr++;
        uint32 aUserNetworkLen = *tmpPtr++;
        uint32 aDeviceInfoLen = *tmpPtr++;
        uint32 aUserIdLen = *tmpPtr++;
        uint32 aUserAuthLen = *tmpPtr++;
        uint32 aExpirationLen = *tmpPtr++;
        uint32 aAppStringLen = *tmpPtr++;
        uint32 aFillerLen = *tmpPtr++;
        uint32 aSignLen = *tmpPtr++;

        iSelectedTrackIDs.clear();
        if (aSelectedTrackIDsSize)
        {
            if (aSelectedTrackIDsSize != iFile->Read(iTmpBuf, 1, aSelectedTrackIDsSize))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                return LoadConfigStatus_NonCriticalError;
            };
            uint8 *tmpBuf = iTmpBuf;
            for (int32 i = aSelectedTrackIDsSize - 1; i >= 0; i--)
            {//Track ID should only use one byte.
                iSelectedTrackIDs.push_back(*tmpBuf++);
            }
        }

        // allocate memory for reading the following string
        PVDlCfgFileAllocator alloc;
        uint8 *aTmpBuf = (uint8 *)alloc.allocate(PVDLCONFIGFILE_TEMPORARY_BUFFER_SIZE);
        if (!aTmpBuf) return LoadConfigStatus_CriticalError;

        if (aUrlLen)
        {
            if (aUrlLen != iFile->Read(aTmpBuf, 1, aUrlLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            aTmpBuf[aUrlLen] = 0;
            if (iUrl.get_size() == 0)
                iUrl.set(OSCL_STATIC_CAST(char*, aTmpBuf), aUrlLen);
            else
            {
                // new url exists, and need to compare it with the url in config file
                if (iUrl.get_size() != aUrlLen)
                {
                    alloc.deallocate(aTmpBuf);
                    return LoadConfigStatus_NonCriticalError;
                }
                if (oscl_strcmp(iUrl.get_cstr(), OSCL_STATIC_CAST(char*, aTmpBuf)) != 0)
                {
                    alloc.deallocate(aTmpBuf);
                    return LoadConfigStatus_NonCriticalError;
                }
            }
        }
        if (aHostNameLen)
        {
            if (aHostNameLen != iFile->Read(aTmpBuf, 1, aHostNameLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iProxyName.set(OSCL_STATIC_CAST(char*, aTmpBuf), aHostNameLen);
        }
        if (aPlayerVersionLen)
        {
            if (aPlayerVersionLen != iFile->Read(aTmpBuf, 1, aPlayerVersionLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iPlayerVersion.set(OSCL_STATIC_CAST(char*, aTmpBuf), aPlayerVersionLen);
        }
        if (aUserAgentLen)
        {
            if (aUserAgentLen != iFile->Read(aTmpBuf, 1, aUserAgentLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iUserAgent.set(OSCL_STATIC_CAST(char*, aTmpBuf), aUserAgentLen);
        }
        if (aUserNetworkLen)
        {
            if (aUserNetworkLen != iFile->Read(aTmpBuf, 1, aUserNetworkLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iUserNetwork.set(OSCL_STATIC_CAST(char*, aTmpBuf), aUserNetworkLen);
        }
        if (aDeviceInfoLen)
        {
            if (aDeviceInfoLen != iFile->Read(aTmpBuf, 1, aDeviceInfoLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iDeviceInfo.set(OSCL_STATIC_CAST(char*, aTmpBuf), aDeviceInfoLen);
        }

        if (aUserIdLen)
        {
            if (aUserIdLen != iFile->Read(aTmpBuf, 1, aUserIdLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iUserId.set(OSCL_STATIC_CAST(char*, aTmpBuf), aUserIdLen);
        }
        if (aUserAuthLen)
        {
            if (aUserAuthLen != iFile->Read(aTmpBuf, 1, aUserAuthLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iUserAuth.set(OSCL_STATIC_CAST(char*, aTmpBuf), aUserAuthLen);
        }
        if (aExpirationLen)
        {
            if (aExpirationLen != iFile->Read(aTmpBuf, 1, aExpirationLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iExpiration.set(OSCL_STATIC_CAST(char*, aTmpBuf), aExpirationLen);
        }
        if (aAppStringLen)
        {
            if (aAppStringLen != iFile->Read(aTmpBuf, 1, aAppStringLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iAppString.set(OSCL_STATIC_CAST(char*, aTmpBuf), aAppStringLen);
        }
        if (aFillerLen)
        {
            if (aFillerLen != iFile->Read(aTmpBuf, 1, aFillerLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iFiller.set(OSCL_STATIC_CAST(char*, aTmpBuf), aFillerLen);
        }
        if (aSignLen)
        {
            if (aSignLen != iFile->Read(aTmpBuf, 1, aSignLen))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVDlCfgFile::LoadConfig() ERROR. line %d ", __LINE__));
                alloc.deallocate(aTmpBuf);
                return LoadConfigStatus_NonCriticalError;
            };
            iSign.set(OSCL_STATIC_CAST(char*, aTmpBuf), aSignLen);
        }
        alloc.deallocate(aTmpBuf);
    }

    return LoadConfigStatus_NoError;
}

void PVDlCfgFile::composeFixedHeader(uint8 *aBuf)
{
    uint32 *tmpPtr = OSCL_STATIC_CAST(uint32*, aBuf);
    *tmpPtr++ = 	iMagic32;
    *tmpPtr++ = 	iVersion;

    //flag for download type 3gpp/fasttrack, download complete, and playback modes
    *tmpPtr++ = 	iFlag;
    *tmpPtr++ = 	iUrl.get_size();
    *tmpPtr++ = 	iProxyName.get_size();	//if proxy is in use, it is the proxy address len
    *tmpPtr++ = 	iProxyPort;

    //client only downloads the clip which is smaller than this size
    *tmpPtr++ = 	iMaxAllowedFileSize;
    //the file size after it is completly downloaded.
    *tmpPtr++ = 	iOverallFileSize;
    //for FastTrack, this would be the accumulated bytes downloaded
    *tmpPtr++ = 	iCurrentFileSize;
    // flag of whether to have content length for the previous download
    *tmpPtr++ =		iHasContentLength;

    *tmpPtr++ = 	iConnectTimeout;
    *tmpPtr++ = 	iSendTimeout;
    *tmpPtr++ = 	iRecvTimeout;

    //FastTrack only
    *tmpPtr++ = 	iRangeStartTime; //in ms
    *tmpPtr++ = 	iSelectedTrackIDs.size();

    *tmpPtr++ = 	iPlayerVersion.get_size();
    *tmpPtr++ = 	iUserAgent.get_size();
    *tmpPtr++ = 	iUserNetwork.get_size();
    *tmpPtr++ = 	iDeviceInfo.get_size();
    *tmpPtr++ = 	iUserId.get_size();
    *tmpPtr++ = 	iUserAuth.get_size();
    *tmpPtr++ = 	iExpiration.get_size();
    *tmpPtr++ = 	iAppString.get_size();
    *tmpPtr++ = 	iFiller.get_size();
    *tmpPtr++ = 	iSign.get_size();
}


