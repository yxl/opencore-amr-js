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

#include "pvpvxparser.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


///////////////////////////////////////////////////
OSCL_EXPORT_REF CPVXInfo::CPVXInfo(void)
{
    Reset();
}

////////////////////////////////////////////////////
OSCL_EXPORT_REF CPVXInfo::~CPVXInfo()
{
}

////////////////////////////////////////////////////
// Clear the PVX info
void CPVXInfo::Reset(void)
{
    iSecurity = false;
    iPVXVersion = 0;
    iStartTime = 0;
    iEndTime = -1;
    iFlags = 0;
    iDownload = false;
    iPurge = false;
    iRandomPos = false;
    iPlaybackControl = ENoPlayback;
    iUserPlayback = false;
    iEndOfClip = ENoAction;
    iLaunchURL = OSCL_wHeapStringA();
}


/////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF CPVXParser::CPVXParser(void)
{
    iLogger = PVLogger::GetLoggerObject("");
}

/////////////////////////////////////////////////////////////////////////////
// Parses the PVX file content contained in aBuffer (2.0 or Imperial) and puts
// the parse content in pPVXInfo
OSCL_EXPORT_REF CPVXParser::CPVXParserStatus CPVXParser::ParsePVX(OsclMemoryFragment &aPVX, OSCL_HeapString<OsclMemAllocator>& aClipURL, CPVXInfo& aPVXInfo)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVXParser::ParsePVX() called"));

    uint32 aBufferLen = aPVX.len;
    uint8* aBuffer = (uint8*)aPVX.ptr;
    // Check if the PVX info structure is valid

    // Get the PVX file size
    uint32 FileLoc = 0;
    if (FileLoc + 2 >= aBufferLen)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error  1 FileLoc+2 >= aBufferLen"));
        return CPVXParser_InvalidSize;
    }

    FileLoc += 2;	// Skip the file size

    // Get video and audio IDs (2.0) OR PVX file version (Imperial) (2 bytes)
    if (FileLoc + 2 >= aBufferLen)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 2 FileLoc+2 >= aBufferLen"));
        return CPVXParser_InvalidSize;
    }

    aPVXInfo.iPVXVersion = BufToShortInt(&aBuffer[FileLoc]);
    FileLoc += 2;

    // Get the subscriber size info (2 bytes)
    if (FileLoc + 2 >= aBufferLen)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 3 FileLoc+2 >= aBufferLen"));
        return CPVXParser_InvalidSize;
    }

    int16 dSubSize = BufToShortInt(&aBuffer[FileLoc]);
    FileLoc += 2;

    // Get subscriber info if present
    if (dSubSize > 0)
    {
        // Get the user ID size (2 bytes)
        if (FileLoc + 2 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 4 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        int16 dUserIDSize = BufToShortInt(&aBuffer[FileLoc]);
        FileLoc += 2;

        // Get User ID if present (variable length specified by dUserIDSize)
        if (dUserIDSize > 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "CPVXParser::ParsePVX() called FileLoc=%d,dUserIDSize=%d, \
			aBufferLen=%d,MAX_PVXUSERID_LEN=%d", FileLoc, dUserIDSize, aBufferLen, MAX_PVXUSERID_LEN));
            if (FileLoc + dUserIDSize >= aBufferLen || dUserIDSize > MAX_PVXUSERID_LEN - 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser:: \
				ParsePVX Error 5 FileLoc+2 >= aBufferLen"));
                return CPVXParser_InvalidSize;
            }
            aPVXInfo.iUserID.write(0, dUserIDSize, (char *)&aBuffer[FileLoc]);
            FileLoc += dUserIDSize;
        }

        // Get the application specific string size (2 bytes)
        if (FileLoc + 2 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 6 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }

        int16 dAppSpecificSize = BufToShortInt(&aBuffer[FileLoc]);
        FileLoc += 2;

        // Get application specific string if present (variable length specified by dAppSpecificSize)
        if (dAppSpecificSize > 0)
        {
            if (FileLoc + dAppSpecificSize >= aBufferLen || dAppSpecificSize > MAX_PVXAPPSPECIFICSTRING_LEN - 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "CPVXParser::ParsePVX Error 7 FileLoc+2 >= aBufferLen"));
                return CPVXParser_InvalidSize;
            }

            aPVXInfo.iAppSpecificString.write(0, dAppSpecificSize, (char *)&aBuffer[FileLoc]);
            FileLoc += dAppSpecificSize;
        }

        // Get the security flag (1 byte)
        if (FileLoc + 1 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error FileLoc+1 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }

        if (aBuffer[FileLoc] > 0)
        {
            aPVXInfo.iSecurity = true;
        }
        else
        {
            aPVXInfo.iSecurity = false;
        }
        FileLoc += 1;

        // Get authentication session ID (36 bytes)
        if (FileLoc + 36 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 8 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }

        aPVXInfo.iSessionID.write(0, MAX_PVXSESSIONID_LEN, (char *)&aBuffer[FileLoc]);
        FileLoc += 36;

        // Get expiry timestamp (20 bytes)
        if (FileLoc + 20 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 9 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iExpiryTimeStamp.write(0, MAX_PVXEXPIRYTIMESTAMP_LEN, (char *)&aBuffer[FileLoc]);
        FileLoc += 20;

        // Get random filler field (16 bytes)
        if (FileLoc + 16 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 10 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iRandomFiller.write(0, MAX_PVXRANDOMFILLER_LEN, (char *)&aBuffer[FileLoc]);
        FileLoc += 16;

        // Get digital signature (40 bytes)
        if (FileLoc + 40 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 11 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iDigitalSignature.write(0, MAX_PVXDIGITALSIGNATURE_LEN, (char *)&aBuffer[FileLoc]);
        FileLoc += 40;
    }

    //	For proper implementation, URL size should be an offset from subscriber auth size

    //	But following PVPlayer 2.0 PC/CE implementation

    // Get the URL size (2 bytes)
    if (FileLoc + 2 >= aBufferLen)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "CPVXParser::ParsePVX Error 12 FileLoc+2 >= aBufferLen"));
        return CPVXParser_InvalidSize;
    }

    int16 dURLSize = BufToShortInt(&aBuffer[FileLoc]);
    FileLoc += 2;

    // Get the URL (variable length specified by dURLSize)
    if (dURLSize > 0)
    {
        if (FileLoc + dURLSize > aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPVXParser::ParsePVX Error 13 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }

        // Only copy dURLSize of bytes
        aClipURL.set((char*)&aBuffer[FileLoc], dURLSize);
    }
    else
    {
        // URL is not present. Bad PVX file
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error NO URL...BAD PVX FILE"));
        return CPVXParser_URLCorrupted;
    }
    FileLoc += dURLSize;

    // If iPVXVersion is 768(0x0300), Imperial format
    if (aPVXInfo.iPVXVersion == 0x0300)
    {
        // Get playback start time (4 bytes)
        if (FileLoc + 4 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 15 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iStartTime = BufToLongInt(&aBuffer[FileLoc]);
        FileLoc += 4;

        // Get playback end time (4 bytes)
        if (FileLoc + 4 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 16 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iEndTime = BufToLongInt(&aBuffer[FileLoc]);
        FileLoc += 4;

        // Get flags (4 bytes)
        if (FileLoc + 4 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 17 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        aPVXInfo.iFlags = BufToLongInt(&aBuffer[FileLoc]);
        FileLoc += 4;

        // Check the flags
        aPVXInfo.iRandomPos = (aPVXInfo.iFlags & 0x01 ? false : true);
        aPVXInfo.iDownload = (aPVXInfo.iFlags & 0x02 ? true : false);
        aPVXInfo.iPurge = (aPVXInfo.iFlags & 0x04 ? true : false);
        aPVXInfo.iUserPlayback = (aPVXInfo.iFlags & 0x20 ? false : true);
        switch (aPVXInfo.iFlags & 0x18)
        {
            case 0x08:	// Automatic after download complete
                aPVXInfo.iPlaybackControl = CPVXInfo::EAfterDownload;
                break;
            case 0x10:	// Play as soon as possible
                aPVXInfo.iPlaybackControl = CPVXInfo::EAsap;
                break;
            case 0x18:	// Reserved
                aPVXInfo.iPlaybackControl = CPVXInfo::EReserve;
                break;
            case 0x00:	// Not automatic
            default:
                aPVXInfo.iPlaybackControl = CPVXInfo::ENoPlayback;
                break;
        }

        // Get end of clip behavior (2 bytes)
        if (FileLoc + 2 >= aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 18 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        switch (BufToShortInt(&aBuffer[FileLoc]))
        {
            case 1:
                aPVXInfo.iEndOfClip = CPVXInfo::ECloseApp;
                break;

            case 2:
                aPVXInfo.iEndOfClip = CPVXInfo::ENextUrl;
                break;

            case 0:
            default:
                aPVXInfo.iEndOfClip = CPVXInfo::ENoAction;
                break;
        }
        FileLoc += 2;

        // Get launch URL size (2 bytes)
        if (FileLoc + 2 > aBufferLen)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 19 FileLoc+2 >= aBufferLen"));
            return CPVXParser_InvalidSize;
        }
        int16 dLaunchURLSize = BufToShortInt(&aBuffer[FileLoc]);
        FileLoc += 2;

        // Get launch URL (variable length specified by dLaunchURLSize)
        if (dLaunchURLSize > 0)
        {
            if (FileLoc + dLaunchURLSize > aBufferLen)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "CPVXParser::ParsePVX Error 20 FileLoc+2 >= aBufferLen"));
                return CPVXParser_InvalidSize;
            }
            OSCL_HeapStringA launchurl8((const char *)&aBuffer[FileLoc], dLaunchURLSize);

            OSCL_wString::chartype* launchurl16 = OSCL_ARRAY_NEW(OSCL_wString::chartype, dLaunchURLSize + 1);
            if (launchurl16 == NULL)
            {
                // Memory allocation failure
                return CPVXParser_InvalidSize;
            }
            oscl_UTF8ToUnicode((const OSCL_String::chartype*)launchurl8.get_cstr(), launchurl8.get_size(), \
                               launchurl16, dLaunchURLSize + 1);

            OSCL_wHeapStringA tmp16(launchurl16, dLaunchURLSize);

            aPVXInfo.iLaunchURL = tmp16;
            OSCL_ARRAY_DELETE(launchurl16);
        }
        FileLoc += dLaunchURLSize;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVXParser::ParsePVX() End"));
    return CPVXParser_Success;
}


///////////////////////////////////////////////////////////////////////////////
// Converts little endian data to short int
int16 CPVXParser::BufToShortInt(uint8* data)
{

    // Little endian
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVXParser::BufToShortInt() called"));
    return (int16)(((((int16)data[1]) << 8)&0xFF00) | (((int16)data[0])&0x00FF));
}


///////////////////////////////////////////////////////////////////////////////
// Converts little endian data to long int
int32 CPVXParser::BufToLongInt(uint8* data)
{
    // Little endian
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVXParser::BufToLongInt() called"));
    return (int32)(((((int32)data[3]) << 24)&0xFF000000) | ((((int32)data[2]) << 16)&0x00FF0000) |
                   ((((int32)data[1]) << 8)&0x0000FF00) | (((int32)data[0])&0x000000FF));
}
////////////////////////////////////////////////////////////////////////////////





