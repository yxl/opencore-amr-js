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
#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#define PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define PVMF_SOURCE_CONTEXT_DATA_UUID PVUuid(0xf218cdfc,0x797d,0x453e,0x9c,0x94,0x8f,0x0e,0x83,0x8a,0xa1,0x99)

#define PVMF_SOURCE_CONTEXT_DATA_COMMON_UUID PVUuid(0xa6b5f829,0x8ead,0x4c21,0xad,0x40,0x8c,0xc5,0x10,0x05,0xb5,0xfa)
#define PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID PVUuid(0x0b8a0087,0xd539,0x4ee0,0x88,0x8d,0x0c,0x1c,0x70,0xf0,0x33,0x59)
#define PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID PVUuid(0x731e4269,0x849b,0x4123,0x92,0x6f,0xaf,0x27,0xc9,0x8a,0x2e,0xab)
#define PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_PVX_UUID PVUuid(0x3dbb1b51,0x49ea,0x4933,0xa6,0xec,0x26,0x97,0x43,0x12,0xb6,0xd6)
#define PVMF_SOURCE_CONTEXT_DATA_PVR_UUID PVUuid(0x442059a2,0x76b6,0x11dc,0x83,0x14,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define PVMF_SOURCE_CONTEXT_DATA_PACKETSOURCE_UUID PVUuid(0x7f064f8c,0xa90d,0x11dc,0x83,0x14,0x08,0x00,0x20,0x0c,0x9a,0x66)
//3f9c615e-31e2-474c-9add-29c61dd07ead

class OsclFileHandle;
class PVMFCPMPluginAccessInterfaceFactory;

#define BITMASK_PVMF_SOURCE_INTENT_PLAY          0x00000001
#define BITMASK_PVMF_SOURCE_INTENT_GETMETADATA   0x00000002
#define BITMASK_PVMF_SOURCE_INTENT_PREVIEW       0x00000004
#define BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS    0x00000008

class PVMFSourceContextDataCommon : public PVInterface
{
    public:
        PVMFSourceContextDataCommon()
        {
            iRefCounter = 0;
            iFileHandle = NULL;
            iPreviewMode = false;
            iIntent = BITMASK_PVMF_SOURCE_INTENT_PLAY;
            iContentAccessFactory = NULL;
            iRecognizerDataStreamFactory = NULL;
        };

        PVMFSourceContextDataCommon(const PVMFSourceContextDataCommon& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataCommon& operator=(const PVMFSourceContextDataCommon& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_COMMON_UUID))
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

        //Optional file handle.
        OsclFileHandle* iFileHandle;

        //Optional field to indicate if the source that is being
        //passed in will be played back in a preview mode.
        bool iPreviewMode;

        //Optional field to indicate if the source that is being
        //passed in will be used for play back or just for metadata retrieval
        uint32 iIntent;

        //HTTP proxy name, either ip or dns, for DRM License Acquisition purposes
        OSCL_wHeapString<OsclMemAllocator> iDRMLicenseProxyName;

        //HTTP proxy port, for DRM for DRM License Acquisition purposes
        int32 iDRMLicenseProxyPort;

        //external datastream for usecases where the input file is controlled by the app
        PVMFCPMPluginAccessInterfaceFactory* iContentAccessFactory;

        //external datastream for usecases where input file to be recognized is controlled by the app
        PVMFCPMPluginAccessInterfaceFactory* iRecognizerDataStreamFactory;

    private:
        void MyCopy(const PVMFSourceContextDataCommon& aSrc)
        {
            iFileHandle				= aSrc.iFileHandle;
            iPreviewMode			= aSrc.iPreviewMode;
            iIntent					= aSrc.iIntent;
            iDRMLicenseProxyName	= aSrc.iDRMLicenseProxyName;
            iDRMLicenseProxyPort	= aSrc.iDRMLicenseProxyPort;
            iContentAccessFactory	= aSrc.iContentAccessFactory;
            iRecognizerDataStreamFactory = aSrc.iRecognizerDataStreamFactory;
        };
};

class PVMFSourceContextDataStreaming : public PVInterface
{
    public:
        //default constructor
        PVMFSourceContextDataStreaming()
        {
            iRefCounter = 0;
            iProxyPort = 0;
        };

        PVMFSourceContextDataStreaming(const PVMFSourceContextDataStreaming& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataStreaming& operator=(const PVMFSourceContextDataStreaming& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID))
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

        //Optional logging url.
        //When present, streaming stats will be sent to this URL.
        //Typically applies to MS HTTP Streaming sessions
        OSCL_wHeapString<OsclMemAllocator> iStreamStatsLoggingURL;

        //HTTP proxy name, either ip or dns
        OSCL_wHeapString<OsclMemAllocator> iProxyName;

        //HTTP proxy port
        int32 iProxyPort;

        // Optional UserID and UserAuthentication (e.g. password) string
        // used for HTTP basic/digest authentication
        OSCL_HeapString<OsclMemAllocator> iUserID;
        OSCL_HeapString<OsclMemAllocator> iUserPasswd;

    private:
        void MyCopy(const PVMFSourceContextDataStreaming& aSrc)
        {
            iStreamStatsLoggingURL	= aSrc.iStreamStatsLoggingURL;
            iProxyName				= aSrc.iProxyName;
            iProxyPort				= aSrc.iProxyPort;
            iUserID					= aSrc.iUserID;
            iUserPasswd				= aSrc.iUserPasswd;
        };
};

//Source data for HTTP Progressive download (format type PVMF_DATA_SOURCE_HTTP_URL)
class PVMFSourceContextDataDownloadHTTP : public PVInterface
{
    public:
        PVMFSourceContextDataDownloadHTTP()
        {
            iRefCounter = 0;
            bIsNewSession = true;
            iMaxFileSize = 0;
        };

        PVMFSourceContextDataDownloadHTTP(const PVMFSourceContextDataDownloadHTTP& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataDownloadHTTP& operator=(const PVMFSourceContextDataDownloadHTTP& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID))
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

        bool	bIsNewSession;				//true if the downloading a new file
        //false if keep downloading a partial downloading file
        OSCL_wHeapString<OsclMemAllocator> iConfigFileName;		//download config file
        OSCL_wHeapString<OsclMemAllocator> iDownloadFileName;	//local file name of the downloaded clip
        uint32	iMaxFileSize;				//the max size of the file.
        OSCL_HeapString<OsclMemAllocator> iProxyName;			//HTTP proxy name, either ip or dns
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

        // Optional UserID and UserAuthentication (e.g. password) string
        // used for HTTP basic/digest authentication
        OSCL_HeapString<OsclMemAllocator> iUserID;
        OSCL_HeapString<OsclMemAllocator> iUserPasswd;

    private:
        void MyCopy(const PVMFSourceContextDataDownloadHTTP& aSrc)
        {
            bIsNewSession		= aSrc.bIsNewSession;
            iConfigFileName		= aSrc.iConfigFileName;
            iDownloadFileName	= aSrc.iDownloadFileName;
            iMaxFileSize		= aSrc.iMaxFileSize;
            iProxyName			= aSrc.iProxyName;
            iProxyPort			= aSrc.iProxyPort;
            iUserID				= aSrc.iUserID;
            iUserPasswd			= aSrc.iUserPasswd;
        };
};

class CPVXInfo;

//Source data for Fasttrack download (format type PVMF_DATA_SOURCE_PVX_FILE)
class PVMFSourceContextDataDownloadPVX : public PVInterface
{
    public:
        PVMFSourceContextDataDownloadPVX()
        {
            iRefCounter = 0;
            bIsNewSession = true;
            iMaxFileSize = 0;
        }

        PVMFSourceContextDataDownloadPVX(const PVMFSourceContextDataDownloadPVX& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataDownloadPVX& operator=(const PVMFSourceContextDataDownloadPVX& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_PVX_UUID))
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

        bool	bIsNewSession;				//true if the downloading a new file
        //false if keep downloading a partial downloading file
        OSCL_wHeapString<OsclMemAllocator> iConfigFileName;		//download config file
        OSCL_wHeapString<OsclMemAllocator> iDownloadFileName;	//local file name of the downloaded clip
        uint32	iMaxFileSize;				//the max size of the file.
        OSCL_HeapString<OsclMemAllocator> iProxyName;			//HTTP proxy name, either ip or dns
        int32	iProxyPort;					//HTTP proxy port

        CPVXInfo *iPvxInfo;					//class which contains all the info in the .pvx file except the URL

    private:
        void MyCopy(const PVMFSourceContextDataDownloadPVX& aSrc)
        {
            bIsNewSession = aSrc.bIsNewSession;
            iConfigFileName = aSrc.iConfigFileName;
            iDownloadFileName = aSrc.iDownloadFileName;
            iMaxFileSize = aSrc.iMaxFileSize;
            iProxyName = aSrc.iProxyName;
            iProxyPort = aSrc.iProxyPort;
            iPvxInfo = aSrc.iPvxInfo;
        };
};


//Source Context Data for PVR
class PVMFPVRControl;

class PVMFSourceContextDataPVR : public PVInterface
{
    public:
        //default constructor
        PVMFSourceContextDataPVR()
        {
            iRefCounter = 0;
            iPVMFPVRControl = NULL;
            iLiveBufferSizeInSeconds = 0;
        };

        PVMFSourceContextDataPVR(const PVMFSourceContextDataPVR& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            iPVMFPVRControl = NULL;
            iLiveBufferSizeInSeconds = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataPVR& operator=(const PVMFSourceContextDataPVR& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

        void SetPVRControl(PVMFPVRControl* aPVMFPVRControl)
        {
            iPVMFPVRControl = aPVMFPVRControl;
        }

        PVMFPVRControl* GetPVRControl()
        {
            return iPVMFPVRControl;
        }


//	void SetLiveBufferSizeInSeconds(uint32 aLiveBufferSizeInSeconds)
//	{
//		iLiveBufferSizeInSeconds = aLiveBufferSizeInSeconds;
//	}

//	uint32 LiveBufferSizeInSeconds()
//	{
//		return iLiveBufferSizeInSeconds;
//	}

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_PVR_UUID))
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

    private:
        void MyCopy(const PVMFSourceContextDataPVR& aSrc)
        {
            // \todo: check this copy constructor
            iPVMFPVRControl = aSrc.iPVMFPVRControl;
            iLiveBufferSizeInSeconds = aSrc.iLiveBufferSizeInSeconds;
        };

        // reference counter
        int32 iRefCounter;


        // This class doesn't own PVMFPVRControl
        PVMFPVRControl* iPVMFPVRControl;

        // temp buffer size in seconds
        uint32 iLiveBufferSizeInSeconds;
};


class PVMFPacketSource;

class PVMFSourceContextDataPacketSource : public PVInterface
{
    public:
        //default constructor
        PVMFSourceContextDataPacketSource()
        {
            iRefCounter = 0;
            iPacketSourcePlugin = NULL;
        };

        PVMFSourceContextDataPacketSource(const PVMFSourceContextDataPacketSource& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextDataPacketSource& operator=(const PVMFSourceContextDataPacketSource& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

        void SetPacketSourcePlugin(PVMFPacketSource* aPacketSourcePlugin)
        {
            iPacketSourcePlugin = aPacketSourcePlugin;
        }

        PVMFPacketSource* GetPacketSourcePlugin()
        {
            return iPacketSourcePlugin;
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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_PACKETSOURCE_UUID))
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

    private:
        void MyCopy(const PVMFSourceContextDataPacketSource& aSrc)
        {
            // \todo: check this copy constructor
            iPacketSourcePlugin = aSrc.iPacketSourcePlugin;
        };

        // reference counter
        int32 iRefCounter;

        // This class doesn't own PVMFPVRControl
        PVMFPacketSource* iPacketSourcePlugin;
};

class PVMFSourceContextData : public PVInterface
{
    public:
        PVMFSourceContextData()
        {
            iRefCounter = 0;
            iCommonDataContextValid = false;
            iStreamingDataContextValid = false;
            iDownloadHTTPDataContextValid = false;
            iDownloadPVXDataContextValid = false;
            iPVRDataContextValid = false;
            iPacketSourceDataContextValid = false;
        };

        PVMFSourceContextData(const PVMFSourceContextData& aSrc) : PVInterface(aSrc)
        {
            iRefCounter = 0;
            MyCopy(aSrc);
        };

        PVMFSourceContextData& operator=(const PVMFSourceContextData& aSrc)
        {
            if (&aSrc != this)
            {
                MyCopy(aSrc);
            }
            return *this;
        };

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
            if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_UUID))
            {
                iface = this;
                return true;
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_COMMON_UUID))
            {
                if (iCommonDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataCommon;
                    return true;
                }
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID))
            {
                if (iStreamingDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataStreaming;
                    return true;
                }
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID))
            {
                if (iDownloadHTTPDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataDownloadHTTP;
                    return true;
                }
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_PVX_UUID))
            {
                if (iDownloadPVXDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataDownloadPVX;
                    return true;
                }
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_PVR_UUID))
            {
                if (iPVRDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataPVR;
                    return true;
                }
            }
            else if (uuid == PVUuid(PVMF_SOURCE_CONTEXT_DATA_PACKETSOURCE_UUID))
            {
                if (iPacketSourceDataContextValid == true)
                {
                    iface = &iPVMFSourceContextDataPacketSource;
                    return true;
                }
            }

            iface = NULL;
            return false;
        }

        void EnableCommonSourceContext()
        {
            iCommonDataContextValid = true;
        }
        void EnableStreamingSourceContext()
        {
            iStreamingDataContextValid = true;
        }
        void EnableDownloadHTTPSourceContext()
        {
            iDownloadHTTPDataContextValid = true;
        }
        void EnableDownloadPVXSourceContext()
        {
            iDownloadPVXDataContextValid = true;
        }
        void EnablePVRSourceContext()
        {
            iPVRDataContextValid = true;
        }
        void EnablePacketSourceSourceContext()
        {
            iPacketSourceDataContextValid = true;
        }
        void DisableCommonSourceContext()
        {
            iCommonDataContextValid = false;
        }
        void DisableStreamingSourceContext()
        {
            iStreamingDataContextValid = false;
        }
        void DisableeDownloadHTTPSourceContext()
        {
            iDownloadHTTPDataContextValid = false;
        }
        void DisableDownloadPVXSourceContext()
        {
            iDownloadPVXDataContextValid = false;
        }
        void DisablePVRSourceContext()
        {
            iPVRDataContextValid = false;
        }
        void DisablePacketSourceSourceContext()
        {
            iPacketSourceDataContextValid = false;
        }
        PVMFSourceContextDataCommon* CommonData()
        {
            return iCommonDataContextValid ? &iPVMFSourceContextDataCommon : NULL;
        }
        PVMFSourceContextDataStreaming* StreamingData()
        {
            return iStreamingDataContextValid ? &iPVMFSourceContextDataStreaming : NULL;
        }
        PVMFSourceContextDataDownloadHTTP* DownloadHTTPData()
        {
            return iDownloadHTTPDataContextValid ? &iPVMFSourceContextDataDownloadHTTP : NULL;
        }
        PVMFSourceContextDataDownloadPVX* DownloadPVXData()
        {
            return iDownloadPVXDataContextValid ? &iPVMFSourceContextDataDownloadPVX : NULL;
        }
        PVMFSourceContextDataPVR* PVRData()
        {
            return iPVRDataContextValid ? &iPVMFSourceContextDataPVR : NULL;
        }
        PVMFSourceContextDataPacketSource* PacketSourceData()
        {
            return iPacketSourceDataContextValid ? &iPVMFSourceContextDataPacketSource : NULL;
        }

    private:
        int32 iRefCounter;
        bool iCommonDataContextValid;
        bool iStreamingDataContextValid;
        bool iDownloadHTTPDataContextValid;
        bool iDownloadPVXDataContextValid;
        bool iPVRDataContextValid;
        bool iPacketSourceDataContextValid;

        PVMFSourceContextDataCommon iPVMFSourceContextDataCommon;
        PVMFSourceContextDataStreaming iPVMFSourceContextDataStreaming;
        PVMFSourceContextDataDownloadHTTP iPVMFSourceContextDataDownloadHTTP;
        PVMFSourceContextDataDownloadPVX iPVMFSourceContextDataDownloadPVX;
        PVMFSourceContextDataPVR iPVMFSourceContextDataPVR;
        PVMFSourceContextDataPacketSource iPVMFSourceContextDataPacketSource;

        void MyCopy(const PVMFSourceContextData& aSrc)
        {
            iCommonDataContextValid = aSrc.iCommonDataContextValid;
            iStreamingDataContextValid = aSrc.iStreamingDataContextValid;
            iDownloadHTTPDataContextValid = aSrc.iDownloadHTTPDataContextValid;
            iDownloadPVXDataContextValid = aSrc.iDownloadPVXDataContextValid;
            iPVRDataContextValid = aSrc.iPVRDataContextValid;
            iPacketSourceDataContextValid = aSrc.iPacketSourceDataContextValid;

            iPVMFSourceContextDataCommon = aSrc.iPVMFSourceContextDataCommon;
            iPVMFSourceContextDataStreaming = aSrc.iPVMFSourceContextDataStreaming;
            iPVMFSourceContextDataDownloadHTTP = aSrc.iPVMFSourceContextDataDownloadHTTP;
            iPVMFSourceContextDataDownloadPVX = aSrc.iPVMFSourceContextDataDownloadPVX;
            iPVMFSourceContextDataPVR = aSrc.iPVMFSourceContextDataPVR;
            iPVMFSourceContextDataPacketSource = aSrc.iPVMFSourceContextDataPacketSource;
        };
};

#endif //PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED

