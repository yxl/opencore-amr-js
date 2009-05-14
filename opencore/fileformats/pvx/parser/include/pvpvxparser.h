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

#ifndef PVPVXPARSER_H_INCLUDED
#define PVPVXPARSER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#define MAX_PVXUSERID_LEN			256
#define MAX_PVXAPPSPECIFICSTRING_LEN		256
#define MAX_PVXSESSIONID_LEN			36
#define MAX_PVXEXPIRYTIMESTAMP_LEN		20
#define MAX_PVXRANDOMFILLER_LEN			16
#define MAX_PVXDIGITALSIGNATURE_LEN		40

class CPVXInfo
{
    public:
        OSCL_IMPORT_REF CPVXInfo(void);
        OSCL_IMPORT_REF ~CPVXInfo();
        void Reset(void);
        bool iSecurity;
        OSCL_StackString<MAX_PVXUSERID_LEN>iUserID;
        OSCL_StackString<MAX_PVXAPPSPECIFICSTRING_LEN>iAppSpecificString;
        OSCL_StackString<MAX_PVXSESSIONID_LEN>iSessionID;
        OSCL_StackString<MAX_PVXEXPIRYTIMESTAMP_LEN>iExpiryTimeStamp;
        OSCL_StackString<MAX_PVXRANDOMFILLER_LEN>iRandomFiller;
        OSCL_StackString<MAX_PVXDIGITALSIGNATURE_LEN>iDigitalSignature;

        typedef enum
        {
            ENoAction			= 0,
            ECloseApp,
            ENextUrl
        } TPVEndOfClipAction;

        typedef enum 		// For Download only
        {
            ENoPlayback			= 0,
            EAfterDownload,
            EAsap,
            EReserve
        } TPVPlaybackControl;

        // Imperial specific data
        int16 iPVXVersion;
        int32 iStartTime;
        int32 iEndTime;
        int32 iFlags;
        bool iDownload;
        bool iPurge;
        bool iRandomPos;
        TPVPlaybackControl	iPlaybackControl;
        bool iUserPlayback;
        TPVEndOfClipAction	iEndOfClip;
        OSCL_wHeapStringA iLaunchURL;
};


class CPVXParser
{
    public:

        typedef enum
        {
            CPVXParser_Success,
            CPVXParser_InvalidSize,
            CPVXParser_URLCorrupted
        } CPVXParserStatus;
        /**
         * This routine parses the .pvx file.
         * @param aPVX: (input param) .pvx file.
         * @param aClipURL: (output param) the target URL
         * @param aPVXInfo: (output param) additional information in the .pvx file
         * @return Error code
         */
        OSCL_IMPORT_REF CPVXParserStatus ParsePVX(OsclMemoryFragment &aPVX, OSCL_HeapString<OsclMemAllocator>& aClipURL, CPVXInfo& aPVXInfo);
        OSCL_IMPORT_REF CPVXParser(void);

    private:
        int16 BufToShortInt(uint8* data);
        int32 BufToLongInt(uint8* data);
        PVLogger* iLogger;
};

# endif

