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

#ifndef PVMF_PROTOCOLENGINE_NODE_PROGRESSIVE_STREAMING_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_PROGRESSIVE_STREAMING_H_INCLUDED

#ifndef PVMF_PROTOCOLENGINE_NODE_PROGRESSIVE_DOWNLOAD_H_INCLUDED
#include "pvmf_protocol_engine_node_progressive_download.h"
#endif


////////////////////////////////////////////////////////////////////////////////////
//////	ProgressiveStreamingContainer
////////////////////////////////////////////////////////////////////////////////////
class ProgressiveStreamingContainer : public ProgressiveDownloadContainer
{
    public:
        OSCL_IMPORT_REF virtual bool createProtocolObjects();
        OSCL_IMPORT_REF PVMFStatus doStop();
        OSCL_IMPORT_REF PVMFStatus doSeek(PVMFProtocolEngineNodeCommand& aCmd);
        OSCL_IMPORT_REF bool completeRepositionRequest();
        OSCL_IMPORT_REF bool doInfoUpdate(const uint32 downloadStatus);
        void enableInfoUpdate(const bool aEnabled = true)
        {
            iEnableInfoUpdate = aEnabled;
        }

        // constructor
        OSCL_IMPORT_REF ProgressiveStreamingContainer(PVMFProtocolEngineNode *aNode = NULL);

    protected:
        // called by DoSeek()
        OSCL_IMPORT_REF uint32 getSeekOffset(PVMFProtocolEngineNodeCommand& aCmd);
        OSCL_IMPORT_REF PVMFStatus doSeekBody(uint32 aNewOffset);
        OSCL_IMPORT_REF void updateDownloadControl(const bool isDownloadComplete = false);
        OSCL_IMPORT_REF bool needToCheckResumeNotificationMaually();

    protected:
        bool iEnableInfoUpdate;

    private:
        void moveToStartedState();
};


////////////////////////////////////////////////////////////////////////////////////
//////	pvProgressiveStreamingOutput
////////////////////////////////////////////////////////////////////////////////////
class pvProgressiveStreamingOutput : public pvHttpDownloadOutput
{
    public:
        OSCL_IMPORT_REF int32 flushData(const uint32 aOutputType = NodeOutputType_InputPortForData);
        void discardData(const bool aNeedReopen = false)
        {
            OSCL_UNUSED_ARG(aNeedReopen);
            return;
        }
        OSCL_IMPORT_REF bool releaseMemFrag(OsclRefCounterMemFrag* aFrag);
        // for new data stream APIs
        OSCL_IMPORT_REF void setContentLength(uint32 aLength);
        OSCL_IMPORT_REF void dataStreamCommandCompleted(const PVMFCmdResp& aResponse);
        void setDataStreamSourceRequestObserver(PvmiDataStreamRequestObserver* aObserver)
        {
            iSourceRequestObserver = aObserver;
        }
        OSCL_IMPORT_REF void flushDataStream();
        OSCL_IMPORT_REF bool seekDataStream(const uint32 aSeekOffset);

        // constructor and destructor
        OSCL_IMPORT_REF pvProgressiveStreamingOutput(PVMFProtocolEngineNodeOutputObserver *aObserver = NULL);
        virtual ~pvProgressiveStreamingOutput()
        {
            flushDataStream();
        }

    private:
        OSCL_IMPORT_REF int32 openDataStream(OsclAny* aInitInfo);
        // write data to data stream object
        // return~0=0xffffffff for error.
        uint32 writeToDataStream(OUTPUT_DATA_QUEUE &aOutputQueue, PENDING_OUTPUT_DATA_QUEUE &aPendingOutputQueue);

    private:
        PvmiDataStreamRequestObserver* iSourceRequestObserver;
};


////////////////////////////////////////////////////////////////////////////////////
//////	progressiveStreamingControl
////////////////////////////////////////////////////////////////////////////////////
class progressiveStreamingControl : public progressiveDownloadControl
{
    public:
        OSCL_IMPORT_REF void requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete, bool& aNeedSendUnderflowEvent);

        // clear several fields for progressive playback repositioning
        OSCL_IMPORT_REF void clearPerRequest();

        // constructor
        OSCL_IMPORT_REF progressiveStreamingControl();
};

////////////////////////////////////////////////////////////////////////////////////
//////	ProgressiveStreamingProgress
////////////////////////////////////////////////////////////////////////////////////
class ProgressiveStreamingProgress : public ProgressiveDownloadProgress
{
    public:
        // constructor
        ProgressiveStreamingProgress() : ProgressiveDownloadProgress(), iContentLength(0)
        {
            ;
        }

    private:
        OSCL_IMPORT_REF bool calculateDownloadPercent(uint32 &aDownloadProgressPercent);

    private:
        uint32 iContentLength;
};


////////////////////////////////////////////////////////////////////////////////////
//////	progressiveStreamingEventReporter
////////////////////////////////////////////////////////////////////////////////////
class progressiveStreamingEventReporter : public downloadEventReporter
{
    public:
        // constructor
        progressiveStreamingEventReporter(EventReporterObserver *aObserver) : downloadEventReporter(aObserver)
        {
            ;
        }

    private:
        // in case of progressive streaming, currently do not send PVMFInfoSessionDisconnect event
        void checkServerDisconnectEvent(const uint32 downloadStatus)
        {
            OSCL_UNUSED_ARG(downloadStatus);
        }
        // in case of progressive streaming, add buffer fullness information into buffer status report
        OSCL_IMPORT_REF void reportBufferStatusEvent(const uint32 aDownloadPercent);
        // called by reportBufferStatusEvent
        uint32 getBufferFullness();
};

////////////////////////////////////////////////////////////////////////////////////
//////	PVProgressiveStreamingCfgFileContainer
////////////////////////////////////////////////////////////////////////////////////
class PVProgressiveStreamingCfgFileContainer : public PVProgressiveDownloadCfgFileContainer
{
    public:
        PVProgressiveStreamingCfgFileContainer(PVMFDownloadDataSourceContainer *aDataSource) : PVProgressiveDownloadCfgFileContainer(aDataSource)
        {
            ;
        }

    private:
        // no need to save data to config file
        void saveConfig()
        {
            ;
        }
};


#endif

