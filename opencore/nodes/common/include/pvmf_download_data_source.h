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
#ifndef PVMF_DOWNLOAD_DATA_SOURCE_H_INCLUDED
#define PVMF_DOWNLOAD_DATA_SOURCE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define PVMF_DOWNLOAD_DATASOURCE_HTTP_UUID PVUuid(0xdea36265,0x6a59,0x4d8b,0xb3,0xea,0xdf,0x68,0x4d,0x7e,0x0e,0x08)

//Source data for HTTP Progressive download (format type PVMF_DATA_SOURCE_HTTP_URL)
class PVMFDownloadDataSourceHTTP : public PVInterface
{
    public:
        bool	bIsNewSession;				//true if the downloading a new file
        //false if keep downloading a partial downloading file
        OSCL_wString &iConfigFileName;		//download config file
        OSCL_wString &iDownloadFileName;	//local file name of the downloaded clip
        uint32	iMaxFileSize;				//the max size of the file.
        OSCL_String &iProxyName;			//HTTP proxy name, either ip or dns
        int32	iProxyPort;					//HTTP proxy port

        typedef enum 		// For Download only
        {
            ENoPlayback			= 0,
            EAfterDownload,
            EAsap,
            ENoSaveToFile,
            EReserve
        } TPVPlaybackControl;
        TPVPlaybackControl	iPlaybackControl;

        PVMFDownloadDataSourceHTTP(bool aIsNewSession
                                   , OSCL_wString &aConfigFile
                                   , OSCL_wString &aDownloadFileName
                                   , uint32 aMaxSize
                                   , OSCL_String &aProxyName
                                   , int32 aProxyPort
                                   , TPVPlaybackControl aPlaybackControl)
                : bIsNewSession(aIsNewSession)
                , iConfigFileName(aConfigFile)
                , iDownloadFileName(aDownloadFileName)
                , iMaxFileSize(aMaxSize)
                , iProxyName(aProxyName)
                , iProxyPort(aProxyPort)
                , iPlaybackControl(aPlaybackControl)
        {
        }

        /* From PVInterface */
        void addRef()
        {
            iRefCounter++;
        }
        void removeRef()
        {
            iRefCounter--;
        }
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            if (uuid == PVUuid(PVMF_DOWNLOAD_DATASOURCE_HTTP_UUID))
            {
                iface = this;
                return true;
            }
            else
            {
                iface = NULL;
                return false;
            }
        }
        int32 iRefCounter;
};

#define PVMF_DOWNLOAD_DATASOURCE_PVX_UUID PVUuid(0xc3873d74,0x5759,0x42da,0xaa,0x27,0xfa,0x63,0xb1,0xa0,0xef,0x4e)

class CPVXInfo;

//Source data for Fasttrack download (format type PVMF_DATA_SOURCE_PVX_FILE)
class PVMFDownloadDataSourcePVX : public PVInterface
{
    public:
        bool	bIsNewSession;				//true if the downloading a new file
        //false if keep downloading a partial downloading file
        OSCL_wString &iConfigFileName;		//download config file
        OSCL_wString &iDownloadFileName;	//local file name of the downloaded clip
        uint32	iMaxFileSize;				//the max size of the file.
        OSCL_String &iProxyName;			//HTTP proxy name, either ip or dns
        int32	iProxyPort;					//HTTP proxy port

        CPVXInfo &iPvxInfo;					//class which contains all the info in the .pvx file except the URL

        PVMFDownloadDataSourcePVX(bool aIsNewSession, OSCL_wString &aConfigFile, OSCL_wString &aDownloadFileName, uint32 aMaxSize, OSCL_String &aProxyName, int32 aProxyPort, CPVXInfo &p)
                : bIsNewSession(aIsNewSession)
                , iConfigFileName(aConfigFile)
                , iDownloadFileName(aDownloadFileName)
                , iMaxFileSize(aMaxSize)
                , iProxyName(aProxyName)
                , iProxyPort(aProxyPort)
                , iPvxInfo(p)
        {}

        /* From PVInterface */
        void addRef()
        {
            iRefCounter++;
        }
        void removeRef()
        {
            iRefCounter--;
        }
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            if (uuid == PVUuid(PVMF_DOWNLOAD_DATASOURCE_PVX_UUID))
            {
                iface = this;
                return true;
            }
            else
            {
                iface = NULL;
                return false;
            }
        }
        int32 iRefCounter;
};

#endif //PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED

