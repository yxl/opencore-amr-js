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
#ifndef PVMF_PROTOCOLENGINE_PROGRESSIVE_DOWNLOAD_H_INCLUDED
#define PVMF_PROTOCOLENGINE_PROGRESSIVE_DOWNLOAD_H_INCLUDED

#ifndef PVMF_PROTOCOLENGINE_DOWNLOAD_COMMON_H_INCLUDED
#include "pvmf_protocol_engine_download_common.h"
#endif

#define DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST 1 // for progressive streaming or shoutcast


class ProgressiveDownloadState_HEAD : public DownloadState
{
    public:
        bool isCurrentStateOptional()
        {
            return true;
        }

    private:
        // To compose a request, the only difference is HEAD method
        OSCL_IMPORT_REF void setRequestBasics();
        OSCL_IMPORT_REF bool setHeaderFields();
        OSCL_IMPORT_REF int32 checkParsingStatus(int32 parsingStatus);
        int32 updateDownloadStatistics()
        {
            return 0;
        }

        // From HttpParsingBasicObjectObserver
        OSCL_IMPORT_REF int32 OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader);
};

class ProgressiveDownloadState_GET : public DownloadState
{
    public:
        ProgressiveDownloadState_GET() : iSendEndOfMessageTruncate(false), iRangeHeaderSupported(true)
        {
            ;
        }
        // virtual destructor, let internal objects destruct automatically
        virtual ~ProgressiveDownloadState_GET()
        {
            iOutputDataQueue.clear();
        }

        void stop(const bool isAfterEOS = false)
        {
            OSCL_UNUSED_ARG(isAfterEOS);
            reset();
            sendRequest();
        }

        void reset()
        {
            iSendEndOfMessageTruncate = false;
            DownloadState::reset();
        }

    protected:
        OSCL_IMPORT_REF int32 processMicroStateGetResponsePreCheck();
        OSCL_IMPORT_REF virtual bool setHeaderFields();
        OSCL_IMPORT_REF bool setRangeHeaderFields();
        // From HttpParsingBasicObjectObserver
        OSCL_IMPORT_REF int32 OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader);
        OSCL_IMPORT_REF void updateOutputDataQueue(OUTPUT_DATA_QUEUE *aOutputQueue);
        bool isDownloadStreamingDoneState()
        {
            return true;
        }
        bool isLastState()
        {
            return true;
        }
        OSCL_IMPORT_REF void getStartFragmentInNewDownloadData(OUTPUT_DATA_QUEUE &aOutputQueue,
                bool &aUseAllNewDownloadData,
                uint32 &aStartFragsNo,
                uint32 &aStartFragOffset);
        OSCL_IMPORT_REF void getEndFragmentInNewDownloadData(OUTPUT_DATA_QUEUE &aOutputQueue,
                uint32 &aEndFragNo,
                uint32 &aEndFragValidLen);
        OSCL_IMPORT_REF int32 checkContentInfoMatchingForResumeDownload();
        OSCL_IMPORT_REF virtual int32 checkParsingStatus(int32 parsingStatus);
        OSCL_IMPORT_REF int32 updateDownloadStatistics();

    protected:
        bool iSendEndOfMessageTruncate;
        bool iRangeHeaderSupported;
};

// Progressive Streaming is a special form of progressive download
class ProgressiveStreamingState_GET : public ProgressiveDownloadState_GET
{
    public:
        virtual void seek(const uint32 aSeekPosition)
        {
            iCfgFile->SetCurrentFileSize(aSeekPosition);
            iNeedGetResponsePreCheck = true; // reset the parser
        }

    protected:
        OSCL_IMPORT_REF int32 checkParsingStatus(int32 parsingStatus);
        void saveConfig()
        {
            ;
        }
};

// Http progressive download, do state transition
class ProgressiveDownload : public HttpBasedProtocol
{
    public:
        // create states of state machine
        ProgressiveDownload()
        {
            iState[0] = OSCL_NEW(ProgressiveDownloadState_HEAD, ());
            iState[1] = OSCL_NEW(ProgressiveDownloadState_GET, ());
            iInitStateNum = 0;
            iCurrStateNum = iInitStateNum;
            iCurrState = iState[iCurrStateNum];

            // create iComposer and iParser
            iComposer = HTTPComposer::create();
            iParser   = HttpParsingBasicObject::create();
        }

        // destructor
        virtual ~ProgressiveDownload()
        {
            uint32 i = 0;
            for (i = 0; i < NUM_PROGRESSIVE_DOWNLOAD_STATE; i++)
            {
                if (iState[i]) OSCL_DELETE(iState[i]);
                iState[i] = NULL;
            }

            OSCL_DELETE(iComposer);
            OSCL_DELETE(iParser);
        }

        void setConfigInfo(OsclAny* aConfigInfo)
        {
            iCfgFile = *((OsclSharedPtr<PVDlCfgFile> *)aConfigInfo);
            HttpBasedProtocol::setConfigInfo(aConfigInfo);
        }


        void initialize()
        {
            // pass iComposer and iParser down to each state
            uint32 i = 0;
            for (i = 0; i < NUM_PROGRESSIVE_DOWNLOAD_STATE; i++)
            {
                iState[i]->setObserver(this);
                iState[i]->setComposer(iComposer);
                iState[i]->setParser(iParser);
            }
            if (iCfgFile.GetRep())
            {
                if (iCfgFile->getHttpHeadRequestDisabled())
                {
                    iInitStateNum = 1;
                    iCurrStateNum = iInitStateNum;
                    iCurrState = iState[iCurrStateNum];
                }
            }
        }

        void reset()
        {
            HttpBasedProtocol::reset();
            iCurrStateNum = 0;
            iCurrState = iState[iCurrStateNum];
        }

    protected:
        // state transition
        ProtocolState* getNextState()
        {
            // release the memory for the current state
            if (++iCurrStateNum >= NUM_PROGRESSIVE_DOWNLOAD_STATE) iCurrStateNum = iInitStateNum;
            return iState[iCurrStateNum];
        }

    protected:
        DownloadState *iState[NUM_PROGRESSIVE_DOWNLOAD_STATE];
        uint32 iCurrStateNum;
        OsclSharedPtr<PVDlCfgFile> iCfgFile;
        uint32 iInitStateNum;
};

class ProgressiveStreaming : public ProgressiveDownload
{
    public:
        ProgressiveStreaming(): ProgressiveDownload()
        {

            OSCL_DELETE(iState[DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST]);
            iState[DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST] = OSCL_NEW(ProgressiveStreamingState_GET, ());
            if (iParser) iParser->setNumRetry(MAX_NUM_EOS_MESSAGES_FOR_SAME_REQUEST + 1); // +1 for testing purpose (i.e., insert an EOS)
        }

        virtual void seek(const uint32 aSeekPosition)
        {
            HttpBasedProtocol::seek(aSeekPosition);

            // reset EOS related variables inside the parser
            if (iParser) iParser->resetForBadConnectionDetection();
        }
};

#endif // PVMF_PROTOCOLENGINE_PROGRESSIVE_DOWNLOAD_H_INCLUDED

