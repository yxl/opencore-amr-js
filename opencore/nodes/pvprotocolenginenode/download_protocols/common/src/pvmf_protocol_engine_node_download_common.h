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

#ifndef PVMF_PROTOCOLENGINE_NODE_DOWNLOAD_COMMON_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_DOWNLOAD_COMMON_H_INCLUDED

#ifndef PVMF_PROTOCOLENGINE_NODE_COMMON_H_INCLUDED
#include "pvmf_protocol_engine_node_common.h"
#endif

#ifndef PVMF_FORMAT_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#include "pvmf_format_progdownload_support_extension.h"
#endif

#ifndef PVDL_CONFIG_FILE_H_INCLUDED
#include "pvdl_config_file.h"
#endif


////////////////////////////////////////////////////////////////////////////////////
//////	DownloadContainer
////////////////////////////////////////////////////////////////////////////////////
class DownloadContainer : public ProtocolContainer
{
    public:
        // constructor
        OSCL_IMPORT_REF DownloadContainer(PVMFProtocolEngineNode *aNode = NULL);
        virtual ~DownloadContainer()
        {
            ;
        }

        OSCL_IMPORT_REF virtual void deleteProtocolObjects();
        OSCL_IMPORT_REF virtual int32 doPreStart();
        OSCL_IMPORT_REF virtual bool doPause();
        OSCL_IMPORT_REF virtual PVMFStatus doStop();
        OSCL_IMPORT_REF virtual void doClear(const bool aNeedDelete = false);
        OSCL_IMPORT_REF virtual void doCancelClear();
        OSCL_IMPORT_REF virtual bool doInfoUpdate(const uint32 downloadStatus);
        OSCL_IMPORT_REF virtual bool addSourceData(OsclAny* aSourceData);
        OSCL_IMPORT_REF virtual bool createCfgFile(OSCL_String& aUri);
        OSCL_IMPORT_REF virtual bool getProxy(OSCL_String& aProxyName, uint32 &aProxyPort);
        OSCL_IMPORT_REF virtual void setHttpVersion(const uint32 aHttpVersion);
        OSCL_IMPORT_REF virtual void setHttpExtensionHeaderField(OSCL_String &aFieldKey,
                OSCL_String &aFieldValue,
                const HttpMethod aMethod,
                const bool aPurgeOnRedirect);

        OSCL_IMPORT_REF virtual bool handleContentRangeUnmatch();
        OSCL_IMPORT_REF virtual bool downloadUpdateForHttpHeaderAvailable();
        OSCL_IMPORT_REF virtual bool isStreamingPlayback();
        OSCL_IMPORT_REF virtual bool handleProtocolStateComplete(PVProtocolEngineNodeInternalEvent &aEvent, PVProtocolEngineNodeInternalEventHandler *aEventHandler);
        OSCL_IMPORT_REF virtual void checkSendResumeNotification();

    protected:
        OSCL_IMPORT_REF virtual int32 initNodeOutput();
        OSCL_IMPORT_REF virtual bool initProtocol_SetConfigInfo();
        OSCL_IMPORT_REF virtual void initDownloadControl();
        OSCL_IMPORT_REF virtual void updateDownloadControl(const bool isDownloadComplete = false);
        virtual bool isDownloadComplete(const uint32 downloadStatus) const
        {
            return (downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT);
        }
        OSCL_IMPORT_REF virtual bool ignoreThisTimeout(const int32 timerID);
        virtual bool needToCheckResumeNotificationMaually()
        {
            return iNeedCheckResumeNotificationManually;
        }
        OSCL_IMPORT_REF void setEventReporterSupportObjects();

    protected:
        bool iForceSocketReconnect;
        bool iNeedCheckResumeNotificationManually;
};

////////////////////////////////////////////////////////////////////////////////////
//////	pvHttpDownloadOutput
////////////////////////////////////////////////////////////////////////////////////

// This derived class adds data stream output
struct DownloadOutputConfig
{
    bool isResumeDownload;
    bool isRangeSupport;
    bool isNeedOpenDataStream;

    // constructor
    DownloadOutputConfig() : isResumeDownload(false),
            isRangeSupport(true),
            isNeedOpenDataStream(true)
    {
        ;
    }
};

class pvHttpDownloadOutput : public PVMFProtocolEngineNodeOutput
{
    public:
        OSCL_IMPORT_REF void setOutputObject(OsclAny* aOutputObject, const uint32 aObjectType = NodeOutputType_InputPortForData);
        OSCL_IMPORT_REF int32 initialize(OsclAny* aInitInfo = NULL);
        OSCL_IMPORT_REF virtual int32 flushData(const uint32 aOutputType = NodeOutputType_InputPortForData);
        OSCL_IMPORT_REF virtual void discardData(const bool aNeedReopen = false);
        OSCL_IMPORT_REF uint32 getAvailableOutputSize();
        OSCL_IMPORT_REF uint32 getMaxAvailableOutputSize();

        // constructor and destructor
        OSCL_IMPORT_REF pvHttpDownloadOutput(PVMFProtocolEngineNodeOutputObserver *aObserver = NULL);
        OSCL_IMPORT_REF virtual ~pvHttpDownloadOutput();

    protected:
        // write data to data stream object
        // return~0=0xffffffff for error.
        OSCL_IMPORT_REF uint32 writeToDataStream(OUTPUT_DATA_QUEUE &aOutputQueue);
        OSCL_IMPORT_REF bool writeToDataStream(uint8 *aBuffer, uint32 aBufferLen);
        OSCL_IMPORT_REF virtual int32 openDataStream(OsclAny* aInitInfo);
        // reset
        OSCL_IMPORT_REF virtual void reset();

    protected:
        PVMFDataStreamFactory *iDataStreamFactory;
        PVMIDataStreamSyncInterface *iDataStream;
        PvmiDataStreamSession iSessionID; // PvmiDataStreamSession = int32
        bool isOpenDataStream;
        uint32 iCounter; // for debugging purpose
};

////////////////////////////////////////////////////////////////////////////////////
//////	pvDownloadControl
////////////////////////////////////////////////////////////////////////////////////

// This class does auto-resume control and download progress update for event report
class DownloadProgressInterface;
class pvDownloadControl : public DownloadControlInterface
{
    public:
        // constructor, may leave for creating download clock
        OSCL_IMPORT_REF pvDownloadControl();
        virtual ~pvDownloadControl()
        {
            clearBody();
        }


        // set download control supporting objects:
        //		PVMFFormatProgDownloadSupportInterface object,
        //		PVMFDownloadProgressInterface object,
        //		engine playback clock object,
        //		protocol engine object,
        //		DownloadProgressInterface object,	(to get the clip duraton)
        //		PVMFProtocolEngineNodeOutput object
        OSCL_IMPORT_REF void setSupportObject(OsclAny *aDLSupportObject, DownloadControlSupportObjectType aType);

        // From PVMFDownloadProgressInterface API pass down
        OSCL_IMPORT_REF virtual void requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete, bool& aNeedSendUnderflowEvent);
        OSCL_IMPORT_REF void cancelResumeNotification();

        // check whether to make resume notification; if needed, then make resume notification
        // Return value: 1 means making resume notification normally (underflow->auto resume),
        //				 2 means making resume notification for download complete
        //				 0 means anything else
        OSCL_IMPORT_REF virtual int32 checkResumeNotification(const bool aDownloadComplete = true);

        // From PVMFDownloadProgressInterface API
        virtual void getDownloadClock(OsclSharedPtr<PVMFMediaClock> &aClock)
        {
            OSCL_UNUSED_ARG(aClock);
        }
        // From PVMFDownloadProgressInterface API
        void setClipDuration(const uint32 aClipDurationMsec)
        {
            iClipDurationMsec = aClipDurationMsec;
        }

        void setPrevDownloadSize(uint32 aPrevDownloadSize = 0)
        {
            iPrevDownloadSize = aPrevDownloadSize;
        }

        OSCL_IMPORT_REF void clear();

        // clear several fields for progressive playback repositioning
        virtual void clearPerRequest()
        {
            ;
        }
        // checks if download has completed
        bool isDownloadCompletedPerRequest()
        {
            return iDownloadComplete;
        }

    protected:

        // simple routine to focus on sending resume notification only
        OSCL_IMPORT_REF virtual void sendResumeNotification(const bool aDownloadComplete);
        OSCL_IMPORT_REF void sendDownloadCompleteNotification();

        // auto-resume playback decision
        OSCL_IMPORT_REF bool isResumePlayback(const uint32 aDownloadRate, const uint32 aCurrDownloadSize, const uint32 aFileSize);

        // create iDlProgressClock, will leave when memory allocation fails
        OSCL_IMPORT_REF void createDownloadClock();
        virtual bool updateDownloadClock() = 0;

        // ready means, download clock has been created, and all the objects have passed down
        bool isInfoReady()
        {
            return !(iDlProgressClock.GetRep() == NULL ||
                     iProtocol == NULL				 ||
                     iDownloadProgress == NULL		 ||
                     iNodeOutput == NULL);
        };

        // called by isResumePlayback()
        OSCL_IMPORT_REF virtual bool isDlAlgoPreConditionMet(const uint32 aDownloadRate,
                const uint32 aDurationMsec,
                const uint32 aCurrDownloadSize,
                const uint32 aFileSize);

        // update duration by new playback rate, called by checkAutoResumeAlgoWithConstraint
        virtual bool checkNewDuration(const uint32 aCurrDurationMsec, uint32 &aNewDurationMsec)
        {
            aNewDurationMsec = aCurrDurationMsec;
            return true;
        }

        // called by checkAutoResumeAlgoWithConstraint()
        virtual bool approveAutoResumeDecisionShortCut(const uint32 aCurrDownloadSize,
                const uint32 aDurationMsec,
                const uint32 aPlaybackTimeMsec,
                uint32 &aPlaybackRemainingTimeMsec)
        {
            OSCL_UNUSED_ARG(aCurrDownloadSize);
            OSCL_UNUSED_ARG(aDurationMsec);
            OSCL_UNUSED_ARG(aPlaybackTimeMsec);
            OSCL_UNUSED_ARG(aPlaybackRemainingTimeMsec);
            return false;
        }

        // No constraint: for file size/clip duration/clip bitrate(i.e. playback rate), one of them must be unavailable, except
        // file size and clip duration are available, but clip bitrate is unavailable. This only applies on PDL
        virtual bool checkAutoResumeAlgoNoConstraint(const uint32 aCurrDownloadSize,
                const uint32 aFileSize,
                uint32 &aDurationMsec)
        {
            OSCL_UNUSED_ARG(aCurrDownloadSize);
            OSCL_UNUSED_ARG(aFileSize);
            OSCL_UNUSED_ARG(aDurationMsec);
            return false;
        }

        // adding buffer constraint for the algo, i.e. if buffer constraint meets (or buffer overflows), auto-resume should kick off.
        virtual bool isOutputBufferOverflow()
        {
            return false;
        }

        // handle overflow issue: // result = x*1000/y
        OSCL_IMPORT_REF uint32 divisionInMilliSec(const uint32 x, const uint32 y);

    protected:
        // called by checkResumeNotification()
        OSCL_IMPORT_REF bool checkSendingNotification(const bool aDownloadComplete = false);

        // called by isResumePlayback()
        // with contraint: file size and clip duration are both available
        OSCL_IMPORT_REF bool checkAutoResumeAlgoWithConstraint(const uint32 aDownloadRate,
                const uint32 aRemainingDownloadSize,
                const uint32 aDurationMsec,
                const uint32 aFileSize);

        // use fixed-point calculation to replace the float-point calculation: aRemainingDLSize<0.0009*aDownloadRate*aRemainingPlaybackTime
        OSCL_IMPORT_REF bool approveAutoResumeDecision(const uint32 aRemainingDLSize,
                const uint32 aDownloadRate,
                const uint32 aRemainingPlaybackTime);

        // old algorithm
        OSCL_IMPORT_REF bool isResumePlaybackWithOldAlg(const uint32 aDownloadRate,
                const uint32 aRemainingDownloadSize);

        // set file size to parser node for the new API, setFileSize()
        OSCL_IMPORT_REF void setFileSize(const uint32 aFileSize);
        OSCL_IMPORT_REF bool getPlaybackTimeFromEngineClock(uint32 &aPlaybackTime);
        virtual void setProtocolInfo()
        {
            ;
        }

    private:
        void updateFileSize();
        OSCL_IMPORT_REF void clearBody();


    protected:
        // download control
        PVMFTimebase_Tickcount iEstimatedServerClockTimeBase;
        OsclSharedPtr<PVMFMediaClock> iDlProgressClock;
        PVMFMediaClock* iCurrentPlaybackClock;
        PVMFFormatProgDownloadSupportInterface *iProgDownloadSI;
        HttpBasedProtocol *iProtocol;
        DownloadProgressInterface *iDownloadProgress;
        PVMFProtocolEngineNodeOutput *iNodeOutput;
        PVDlCfgFileContainer *iCfgFileContainer;

        bool iPlaybackUnderflow;
        bool iDownloadComplete;
        bool iRequestResumeNotification;
        bool iFirstResumeNotificationSent;
        uint32 iCurrentNPTReadPosition;
        uint32 iClipDurationMsec;
        uint32 iPlaybackByteRate;
        uint32 iPrevDownloadSize;
        uint32 iFileSize;

        bool iDlAlgoPreConditionMet;
        bool iSetFileSize;
        bool iSendDownloadCompleteNotification;
        uint32 iClipByterate;

        PVLogger* iDataPathLogger;
};

////////////////////////////////////////////////////////////////////////////////////
//////	DownloadProgress
////////////////////////////////////////////////////////////////////////////////////
class DownloadProgress : public DownloadProgressInterface
{
    public:

        // cosntructor and destructor
        OSCL_IMPORT_REF DownloadProgress();
        virtual ~DownloadProgress()
        {
            reset();
        }

        // set download progress supporting objects:
        //		PVMFFormatProgDownloadSupportInterface object,
        //		protocol engine object,
        //		config file object,			(for progressive download only)
        //		track selction container	(for fastrack download only)
        //		PVMFProtocolEngineNodeOutput object (for fasttrack download only)
        OSCL_IMPORT_REF virtual void setSupportObject(OsclAny *aDLSupportObject, DownloadControlSupportObjectType aType);

        // updata download progress
        OSCL_IMPORT_REF bool update(const bool aDownloadComplete = false);

        // return true for the new download progress
        OSCL_IMPORT_REF bool getNewProgressPercent(uint32 &aProgressPercent);

        // return duration regardless of the difference between progressive download and fasttrack download
        void setClipDuration(const uint32 aClipDurationMsec)
        {
            iDurationMsec = aClipDurationMsec;
        }

        virtual	void setDownloadProgressMode(DownloadProgressMode aMode = DownloadProgressMode_TimeBased)
        {
            OSCL_UNUSED_ARG(aMode);
        }


    protected:
        OSCL_IMPORT_REF virtual uint32 getClipDuration();
        OSCL_IMPORT_REF virtual bool updateDownloadClock(const bool aDownloadComplete) = 0;
        OSCL_IMPORT_REF virtual bool calculateDownloadPercent(uint32 &aDownloadProgressPercent);
        OSCL_IMPORT_REF virtual void reset();

    protected:
        HttpBasedProtocol *iProtocol;
        PVMFFormatProgDownloadSupportInterface *iProgDownloadSI;
        PVMFProtocolEngineNodeOutput *iNodeOutput;

        //for progress reports
        uint32 iCurrProgressPercent;
        uint32 iPrevProgressPercent;
        uint32 iDownloadNPTTime;
        uint32 iDurationMsec;
};



////////////////////////////////////////////////////////////////////////////////////
//////	PVMFDownloadDataSourceContainer
////////////////////////////////////////////////////////////////////////////////////

// This container class wraps the data from all the download source data classes, i.e.,
// PVMFDownloadDataSourceHTTP, PVMFDownloadDataSourcePVX, PVMFSourceContextDataDownloadHTTP and PVMFSourceContextDataDownloadPVX

class CPVXInfo;
class PVMFDownloadDataSourceContainer
{
    public:
        bool iHasDataSource;									// true means the constainer is already filled in the data source
        bool iIsNewSession;										// true if the downloading a new file, false if keep downloading a partial downloading file
        uint32 iMaxFileSize;									// the max size of the file.
        uint32 iPlaybackControl;								// correspond to PVMFDownloadDataSourceHTTP::TPVPlaybackControl, PVMFSourceContextDataDownloadHTTP::TPVPlaybackControl
        OSCL_wHeapString<OsclMemAllocator> iConfigFileName;		// download config file
        OSCL_wHeapString<OsclMemAllocator> iDownloadFileName;	// local file name of the downloaded clip
        OSCL_HeapString<OsclMemAllocator>  iProxyName;			// HTTP proxy name, either ip or dns
        uint32 iProxyPort;										// HTTP proxy port
        OSCL_HeapString<OsclMemAllocator> iUserID;				// UserID string used for HTTP basic/digest authentication
        OSCL_HeapString<OsclMemAllocator> iUserPasswd;			// password string used for HTTP basic/digest authentication


        CPVXInfo *iPvxInfo;										// Fasttrack only, contains all the info in the .pvx file except the URL

    public:
        // default constructor
        PVMFDownloadDataSourceContainer()
        {
            clear();
        }

        bool isEmpty()
        {
            return !iHasDataSource;
        }

        // major copy constructor to do type conversion
        PVMFDownloadDataSourceContainer(OsclAny* aSourceData);

        // add source data
        bool addSource(OsclAny* aSourceData);

        void clear()
        {
            iHasDataSource	 = false;
            iIsNewSession	 = true;
            iMaxFileSize	 = 0;
            iPlaybackControl = 0;
            iProxyPort		 = 0;
            iPvxInfo		 = NULL;
        }

    private:
        // type conversion routine for each download source data class
        void copy(const PVMFDownloadDataSourceHTTP& aSourceData);
        void copy(const PVMFDownloadDataSourcePVX& aSourceData);
        void copy(const PVMFSourceContextDataDownloadHTTP& aSourceData);
        void copy(const PVMFSourceContextDataDownloadPVX& aSourceData);
        PVMFSourceContextDataDownloadHTTP::TPVPlaybackControl convert(const PVMFDownloadDataSourceHTTP::TPVPlaybackControl aPlaybackControl);
};



////////////////////////////////////////////////////////////////////////////////////
//////	PVDlCfgFileContainer and its derived class definition
////////////////////////////////////////////////////////////////////////////////////
class PVDlCfgFileContainer
{
    public:
        virtual ~PVDlCfgFileContainer() {}

        PVDlCfgFileContainer(PVMFDownloadDataSourceContainer *aDataSource) :
                iPlaybackMode(PVMFDownloadDataSourceHTTP::EAsap),
                iDataSource(aDataSource)
        {
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.protocolenginenode");
        }

        OSCL_IMPORT_REF virtual PVMFStatus createCfgFile(OSCL_String &aUrl);
        void setDataSource(PVMFDownloadDataSourceContainer *aDataSource)
        {
            iDataSource = aDataSource;
        }

        // get API
        OsclSharedPtr<PVDlCfgFile> &getCfgFile()
        {
            return iCfgFileObj;
        }
        PVMFDownloadDataSourceHTTP::TPVPlaybackControl getPlaybackMode()
        {
            return iPlaybackMode;
        }
        bool isEmpty()
        {
            return (iCfgFileObj.GetRep() == NULL);
        }
        virtual void saveConfig()
        {
            if (!isEmpty()) iCfgFileObj->SaveConfig();
        }

    protected:
        OSCL_IMPORT_REF virtual PVMFStatus configCfgFile(OSCL_String &aUrl);
        OSCL_IMPORT_REF PVMFStatus loadOldConfig(); // utility function for configCfgFile()

    protected:
        OsclSharedPtr<PVDlCfgFile> iCfgFileObj;
        PVMFDownloadDataSourceHTTP::TPVPlaybackControl iPlaybackMode;
        PVMFDownloadDataSourceContainer *iDataSource;
        PVLogger* iDataPathLogger;
};

////////////////////////////////////////////////////////////////////////////////////
//////	downloadEventReporter
////////////////////////////////////////////////////////////////////////////////////

class downloadEventReporter : public EventReporter
{
    public:
        // constructor
        OSCL_IMPORT_REF downloadEventReporter(EventReporterObserver *aObserver);

        OSCL_IMPORT_REF virtual void setSupportObject(OsclAny *aSupportObject, EventReporterSupportObjectType aType);
        OSCL_IMPORT_REF virtual bool checkReportEvent(const uint32 downloadStatus);
        OSCL_IMPORT_REF virtual void clear();
        OSCL_IMPORT_REF bool checkContentInfoEvent(const uint32 downloadStatus);

        // enable some specific events
        OSCL_IMPORT_REF void sendDataReadyEvent();
        OSCL_IMPORT_REF void enableBufferingCompleteEvent();
        OSCL_IMPORT_REF void sendBufferStatusEvent();

    protected:
        virtual bool needToCheckContentInfoEvent()
        {
            return true;
        }
        OSCL_IMPORT_REF virtual void checkUnexpectedDataAndServerDisconnectEvent(const uint32 downloadStatus);

        // supporting function for checkReportEvent()
        OSCL_IMPORT_REF bool checkBufferInfoEvent(const uint32 downloadStatus);
        // check and send buffer complete, data ready and unexpected data events
        OSCL_IMPORT_REF void checkBufferCompleteEvent(const uint32 downloadStatus);
        OSCL_IMPORT_REF void checkUnexpectedDataEvent(const uint32 downloadStatus);
        OSCL_IMPORT_REF virtual void checkServerDisconnectEvent(const uint32 downloadStatus);
        // for checkContentInfoEvent()
        OSCL_IMPORT_REF bool checkContentLengthOrTooLarge();
        OSCL_IMPORT_REF bool checkContentTruncated(const uint32 downloadStatus);
        OSCL_IMPORT_REF int32 isDownloadFileTruncated(const uint32 downloadStatus);
        bool isDownloadComplete(const uint32 downloadStatus) const
        {
            return (downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA ||
                    downloadStatus == PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT);
        }
        // will be overriden in case of PS
        // called by sendBufferStatusEventBody()
        OSCL_IMPORT_REF virtual void reportBufferStatusEvent(const uint32 aDownloadPercent);

    protected:
        bool iSendBufferStartInfoEvent;
        bool iSendBufferCompleteInfoEvent;
        bool iSendMovieAtomCompleteInfoEvent;
        bool iSendInitialDataReadyEvent;
        bool iSendContentLengthEvent;
        bool iSendContentTruncateEvent;
        bool iSendContentTypeEvent;
        bool iSendUnexpectedDataEvent;
        bool iSendServerDisconnectEvent;

        // supporting objects
        DownloadProgressInterface *iDownloadProgress;
        HttpBasedProtocol *iProtocol;
        PVDlCfgFileContainer *iCfgFileContainer;
        PVMFProtocolEngineNodeTimer *iNodeTimer;
        InterfacingObjectContainer *iInterfacingObjectContainer;
        PVMFProtocolEngineNodeOutput *iNodeOutput;

    private:
        void sendBufferStatusEventBody(const bool aForceToSend = false);

    private:
        uint32 iPrevDownloadProgress;
};

#endif

