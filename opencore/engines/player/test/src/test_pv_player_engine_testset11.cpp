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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET11_H_INCLUDED
#include "test_pv_player_engine_testset11.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef PV_PLAYER_DATASOURCEURL_H_INCLUDED
#include "pv_player_datasourceurl.h"
#endif

#ifndef PV_PLAYER_DATASINKFILENAME_H_INCLUDED
#include "pv_player_datasinkfilename.h"
#endif

#ifndef PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED
#include "pv_player_datasinkpvmfnode.h"
#endif

#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

#ifndef PV_MEDIA_OUTPUT_NODE_FACTORY_H_INCLUDED
#include "pv_media_output_node_factory.h"
#endif

#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif

#ifndef PVMF_SOCKET_NODE_EVENTS_H_INCLUDED
#include "pvmf_socket_node_events.h"
#endif

#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED
#include "pvmf_protocol_engine_node_events.h"
#endif
//Default Fast-track download file
#define DEFAULT_FASTTRACK_DL_FILE "test.pvx"
#define USE_NEW_PVMF_COMMONSOURCE_CONTEXT_DATA	1

//
// pvplayer_async_test_genericcancelall section
//
void pvplayer_async_test_genericcancelall::CreateDownloadDataSource()
{
#if RUN_FASTTRACK_TESTCASES
    if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
    {
        //fasttrack download using PVX.
        //read the pvx file into a memory fragment.
        OsclMemoryFragment pvxmemfrag;
        {
            Oscl_FileServer fs;
            fs.Connect();
            Oscl_File file;
            OSCL_StackString<64> filename;

            if (oscl_strstr(iFileName, DEFAULTSOURCEFILENAME) != NULL)
            {
                filename = SOURCENAME_PREPEND_STRING;
                filename += DEFAULT_FASTTRACK_DL_FILE;
            }
            else
            {
                filename = iFileName;
            }

            if (file.Open(filename.get_str(), Oscl_File::MODE_READ | Oscl_File::MODE_TEXT, fs))
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
                return;
            }

            int32 size = file.Read(iPVXFileBuf, 1, 4096);
            pvxmemfrag.len = size;
            pvxmemfrag.ptr = iPVXFileBuf;
            file.Close();
            fs.Close();
        }

        //Parse, extracting iDownloadPvxInfo and url8.
        OSCL_HeapString<OsclMemAllocator> url8;
        {
            CPVXParser* parser = NULL;
            parser = new CPVXParser;
            if (parser == NULL)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
                return;
            }
            CPVXParser::CPVXParserStatus status = parser->ParsePVX(pvxmemfrag, url8, iDownloadPvxInfo);
            delete parser;
            if (status != CPVXParser::CPVXParser_Success)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
                return;
            }
            //set the playback mode in the test case base class to match
            //the PVX setting.
            switch (iDownloadPvxInfo.iPlaybackControl)
            {
                case CPVXInfo::ENoPlayback:
                    iDownloadOnly = true;
                    break;
                case CPVXInfo::EAfterDownload:
                    iDownloadThenPlay = true;
                    break;
                case CPVXInfo::EAsap:
                    break;
                default:
                    break;
            }
        }

        //convert the url8 to unicode iDownloadURL
        {
            oscl_wchar* wtemp = new oscl_wchar[url8.get_size()+1];
            if (wtemp == NULL)
            {
                // Memory allocation failure
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
                return;
            }
            int32 wtemplen = oscl_UTF8ToUnicode(url8.get_cstr(), url8.get_size(), wtemp, url8.get_size() + 1);
            //iDownloadURL.set(wtemp, wtemplen);
            wFileName.set(wtemp, wtemplen);
            delete [] wtemp;
        }
    }
#endif

    //create the opaque data
    iDownloadProxy = _STRLIT_CHAR("");
    int32 iDownloadProxyPort = 0;

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");

    iDownloadMaxfilesize = 0x7FFFFFFF;

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.dl");
    iContentTooLarge = false;
    bool aIsNewSession = true;

#if RUN_FASTTRACK_TESTCASES
    if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
    {
        iDownloadContextDataPVX = new PVMFDownloadDataSourcePVX(aIsNewSession, iDownloadConfigFilename, iDownloadFilename, \
                iDownloadMaxfilesize,
                iDownloadProxy,
                iDownloadProxyPort,
                iDownloadPvxInfo);
    }
    else
#endif
    {
        iDownloadContextDataHTTP = new PVMFDownloadDataSourceHTTP(aIsNewSession, iDownloadConfigFilename, iDownloadFilename,
                iDownloadMaxfilesize,
                iDownloadProxy,
                iDownloadProxyPort,
                PVMFDownloadDataSourceHTTP::EAsap);
    }
}

void pvplayer_async_test_genericcancelall::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_genericcancelall::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;
            fprintf(iTestMsgOutputFile, "***Creating Player Engine...\n");
            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_QUERYINTERFACE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_QUERYINTERFACE:
        {
            fprintf(iTestMsgOutputFile, "***Query Interface...\n");
            PVUuid capconfigifuuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->QueryInterface(capconfigifuuid, (PVInterface*&)iPlayerCapConfigIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            fprintf(iTestMsgOutputFile, "***Adding Data Source...\n");
            PVMFFormatType fileType = iFileType;

            iDataSource = new PVPlayerDataSourceURL;
            OSCL_HeapString<OsclMemAllocator> filename;

            filename = iFileName;
            oscl_UTF8ToUnicode(filename.get_str(), filename.get_size(), output, 256);
            wFileName.set(output, oscl_strlen(output));

            if (bcloaking)
            {
                fileType = PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL;
            }

            else if (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL)
            {
                fileType = PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL;
                //We will change the iFileType if we need to do ProtocolRollover
                //iFileType = fileType;
                if (iProtocolRollOverWithUnknownURLType == false)
                {
#ifdef USE_NEW_PVMF_COMMONSOURCE_CONTEXT_DATA
                    iSourceContextData = new PVMFSourceContextData();
                    iSourceContextData->EnableStreamingSourceContext();
                    iSourceContextData->EnableCommonSourceContext();
                    PVInterface* sourceContextStream = NULL;

                    PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                    if (iSourceContextData->queryInterface(streamingContextUuid, sourceContextStream))
                    {
                        PVMFSourceContextDataStreaming* streamingContext =
                            OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContextStream);
                        streamingContext->iStreamStatsLoggingURL = wFileName;

                        if (iProxyEnabled)
                        {
                            streamingContext->iProxyName = _STRLIT_WCHAR("");
                            streamingContext->iProxyPort = 7070;
                        }
                    }
                    PVInterface* sourceContextDownload = NULL;
                    iSourceContextData->EnableDownloadHTTPSourceContext();
                    PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                    if (iSourceContextData->queryInterface(downloadContextUuid, sourceContextDownload))
                    {
                        //create the opaque data
                        iDownloadProxy = _STRLIT_CHAR("");
                        int32 iDownloadProxyPort = 0;
                        if (iProxyEnabled)
                        {
                            iDownloadProxy = _STRLIT_CHAR("");
                            iDownloadProxyPort = 7070;
                        }

                        iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
                        iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");
                        iDownloadMaxfilesize = 0x7FFFFFFF;
                        iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
                        iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.dl");
                        bool aIsNewSession = true;

                        iSourceContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
                        iSourceContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
                        iSourceContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
                        iSourceContextData->DownloadHTTPData()->iMaxFileSize = iDownloadMaxfilesize;
                        iSourceContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;
                        iSourceContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
                        iSourceContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
                    }
                    iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
#else
                    //set logging url
                    iStreamDataSource = new PVMFStreamingDataSource();
                    iStreamDataSource->iStreamStatsLoggingURL = wFileName;

                    if (iProxyEnabled)
                    {
                        iStreamDataSource->iProxyName = _STRLIT_WCHAR("");
                        iStreamDataSource->iProxyPort = 8080;
                    }
                    iDataSource->SetDataSourceContextData((OsclAny*)iStreamDataSource);
#endif
                }
            }
            else if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
            {
                CreateDownloadDataSource();
                fileType = PVMF_MIME_DATA_SOURCE_PVX_FILE;
                iDataSource->SetDataSourceContextData((OsclAny*)iDownloadContextDataPVX);
            }
            else if ((iFileType == PVMF_MIME_DATA_SOURCE_RTSP_URL)
                     || (iFileType == PVMF_MIME_DATA_SOURCE_SDP_FILE))
            {
#ifdef USE_NEW_PVMF_COMMONSOURCE_CONTEXT_DATA
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                iSourceContextData->EnableStreamingSourceContext();

                PVInterface* sourceContext = NULL;
                PVUuid streamingContextUuid(PVMF_SOURCE_CONTEXT_DATA_STREAMING_UUID);
                if (iSourceContextData->queryInterface(streamingContextUuid, sourceContext))
                {
                    PVMFSourceContextDataStreaming* streamingContext =
                        OSCL_STATIC_CAST(PVMFSourceContextDataStreaming*, sourceContext);
                    streamingContext->iStreamStatsLoggingURL = wFileName;
                }
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
#endif
            }

            if (iProtocolRollOver)
            {
                if (fileType == PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL)
                {
                    fileType = PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL;
                    iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
                }
                else if (fileType == PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL)
                {
                    fileType = PVMF_MIME_DATA_SOURCE_RTSP_URL;
#if RUN_RTSP_CLOAKING_TESTCASES
                    iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL);
#endif
                }
                else if (fileType == PVMF_MIME_DATA_SOURCE_RTSP_URL)
                {
                    fileType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
                    iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_RTSP_URL);
#ifdef USE_NEW_PVMF_COMMONSOURCE_CONTEXT_DATA
                    iSourceContextData->EnableDownloadHTTPSourceContext();
                    PVInterface* sourceContext = NULL;
                    PVUuid downloadContextUuid(PVMF_SOURCE_CONTEXT_DATA_DOWNLOAD_HTTP_UUID);
                    if (iSourceContextData->queryInterface(downloadContextUuid, sourceContext))
                    {
                        PVMFSourceContextDataDownloadHTTP* downloadContext =
                            OSCL_STATIC_CAST(PVMFSourceContextDataDownloadHTTP*, sourceContext);
                        if (iProxyEnabled)
                        {
                            downloadContext->iProxyName = _STRLIT_CHAR("");
                            downloadContext->iProxyPort = 7070;
                        }
                        downloadContext->iDownloadFileName += _STRLIT_WCHAR("test_ftdownload.loc");
                        downloadContext->iConfigFileName += _STRLIT_WCHAR("mydlconfig");
                        downloadContext->iUserID = _STRLIT_CHAR("abc");
                        downloadContext->iUserPasswd = _STRLIT_CHAR("xyz");
                        downloadContext->bIsNewSession = true;
                        downloadContext->iMaxFileSize = 0x7FFFFFFF;
                    }
#endif
                }
            }
            else if (iProtocolRollOverWithUnknownURLType)
            {
                fileType = PVMF_MIME_DATA_SOURCE_UNKNOWN_URL;
            }

            iDataSource->SetDataSourceURL(wFileName);

            iDataSource->SetDataSourceFormatType(fileType);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iNumPlay++;
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_CONFIGPARAMS:
        {
            fprintf(iTestMsgOutputFile, "***Configuring Params...\n");
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/user-agent;valtype=wchar*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_wHeapString<OsclMemAllocator> userAgent;
            if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
            {
                userAgent = _STRLIT_WCHAR("PVPLAYER 18.07.00.02");
            }
            else
            {
                userAgent = _STRLIT_WCHAR("NSPlayer/9.0 PVPlayerCoreEngineTestUserAgent");
            }
            iKVPSetAsync.value.pWChar_value = userAgent.get_str();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeader(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=Test;method=GET,POST"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeader.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeader.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=streaming;purge-on-redirect");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=GetHeader;method=GET"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderGet.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderGet.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;purge-on-redirect");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet1(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=GetHeader1;method=GET"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderGet1.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderGet1.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // should not display
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=download");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet2(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=GetHeader2;method=GET"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderGet2.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderGet2.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);



            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderPost(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=PostHeader;method=POST"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderPost.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderPost.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // Unnecessary Header should not display
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderHead(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=HeadHeader;method=HEAD"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderHead.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderHead.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);



            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/num-redirect-attempts;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 2;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-timeout;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 20;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);


            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/keep-alive-interval;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 4; //0xffffffff;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);
            iState = STATE_INIT;
            RunIfNotReady();
        }
        break;

        case STATE_INIT:
        {
            fprintf(iTestMsgOutputFile, "***Initializing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 20, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 20, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***Adding Video Data Sink...\n");
            OSCL_wHeapString<OsclMemAllocator> videoSinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            if (iFileType == PVMF_MIME_DATA_SOURCE_RTSP_URL)
            {
                videoSinkFileName += _STRLIT_WCHAR("test_player_rtspurl_video.dat");
            }
            else if ((iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL) || (iFileType == PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL))
            {
                videoSinkFileName += _STRLIT_WCHAR("test_player_httpurl_video.dat");
            }
            else if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
            {
                videoSinkFileName += _STRLIT_WCHAR("test_player_pvx_video.dat");
            }
            else
            {
                videoSinkFileName += _STRLIT_WCHAR("test_genericreset_");
                OSCL_wHeapString<OsclMemAllocator> inputfilename;
                RetrieveFilename(wFileName.get_str(), inputfilename);
                videoSinkFileName += inputfilename;
                videoSinkFileName += _STRLIT_WCHAR("_video.dat");
            }

            iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & videoSinkFileName, MEDIATYPE_VIDEO, iCompressedVideo);
            iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
            iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASINK_AUDIO:
        {
            fprintf(iTestMsgOutputFile, "***Adding Audio Data Sink...\n");
            OSCL_wHeapString<OsclMemAllocator> audioSinkFileName = OUTPUTNAME_PREPEND_WSTRING;
            if (iFileType == PVMF_MIME_DATA_SOURCE_RTSP_URL)
            {
                audioSinkFileName += _STRLIT_WCHAR("test_player_rtspurl_audio.dat");
            }
            else if ((iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL) || (iFileType == PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL))
            {
                audioSinkFileName += _STRLIT_WCHAR("test_player_httpurl_audio.dat");
            }
            else if (iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE)
            {
                audioSinkFileName += _STRLIT_WCHAR("test_player_pvx_audio.dat");
            }
            else
            {
                audioSinkFileName += _STRLIT_WCHAR("test_genericreset_");
                OSCL_wHeapString<OsclMemAllocator> inputfilename;
                RetrieveFilename(wFileName.get_str(), inputfilename);
                audioSinkFileName += inputfilename;
                audioSinkFileName += _STRLIT_WCHAR("_audio.dat");
            }

            iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & audioSinkFileName, MEDIATYPE_AUDIO, iCompressedAudio);
            iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
            iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
            ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);

            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_PREPARE:
        {
            fprintf(iTestMsgOutputFile, "***Preparing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_START:
        {
            fprintf(iTestMsgOutputFile, "***Starting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                fprintf(iTestMsgOutputFile, "***CancelAllCommands Player Engine...\n");
                OSCL_TRY(error, iCancelAllCmdId = iPlayer->CancelAllCommands((OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

                iState = STATE_WAIT_FOR_CANCELALL;
                RunIfNotReady(15000000);
            }
        }
        break;
        case STATE_WAIT_FOR_DATAREADY:
        {
            if (iNumDataReady > 0)
            {
                //playback cases wait on data ready, then start engine.
                iState = STATE_START;
                RunIfNotReady();
            }
            else
            {
                //timed out while waiting.
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
        }
        break;
        case STATE_WAIT_FOR_BUFFCOMPLETE:
            if (iNumBufferingComplete > 0)
            {
                //Download is complete!
                if (iDownloadOnly)
                {
                    //download-only cases can finish now.
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    //playback cases play 10 more seconds then stop.
                    iState = STATE_STOP;
                    RunIfNotReady(10*1000*1000);
                }
            }
            else
            {
                //timed out while waiting.
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE:
        {
            if (oLiveSession == false)
            {
                fprintf(iTestMsgOutputFile, "***Repositioning...\n");

                {
                    PVPPlaybackPosition start, end;
                    start.iIndeterminate = false;
                    start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                    start.iMode = PVPPBPOS_MODE_NOW;
                    if (iSessionDuration)
                        start.iPosValue.millisec_value = iSessionDuration / 4;
                    else
                        start.iPosValue.millisec_value = 4000;

                    end.iIndeterminate = true;
                    OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
                }
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                if ((iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
            }
            else
            {
                fprintf(iTestMsgOutputFile, "***Setting PlayBack Range Not Supported for Live Sessions...\n");
            }
        }
        break;

        case STATE_PAUSE:
        {
            fprintf(iTestMsgOutputFile, "***Pausing...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_RESUME:
        {
            fprintf(iTestMsgOutputFile, "***Resuming...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_EOSNOTREACHED:
        {
            // EOS event not received so initiate stop
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_STOP;
            RunIfNotReady();
        }
        break;

        case STATE_STOP:
        {
            fprintf(iTestMsgOutputFile, "***Stopping...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            fprintf(iTestMsgOutputFile, "***Removing Video Data Sink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            fprintf(iTestMsgOutputFile, "***Removing Audio Data Sink...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            fprintf(iTestMsgOutputFile, "***Resetting...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            if ((iCancelAllWhileProc) && (iEndState == iState))
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            fprintf(iTestMsgOutputFile, "***Removing Data Source...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            fprintf(iTestMsgOutputFile, "***Deleting Player Engine...\n");
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iStreamDataSource;
            iStreamDataSource = NULL;

            delete iDownloadContextDataPVX;
            iDownloadContextDataPVX = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDataSinkVideo;
            iDataSinkVideo = NULL;

            delete iDataSinkAudio;
            iDataSinkAudio = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
            iIONodeVideo = NULL;

            PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
            iIONodeAudio = NULL;

            iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
            iMIOFileOutVideo = NULL;

            iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
            iMIOFileOutAudio = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        case STATE_CANCELALL:
        {
            fprintf(iTestMsgOutputFile, "***CancelAllCommands Player Engine...\n");
            OSCL_TRY(error, iCancelAllCmdId = iPlayer->CancelAllCommands((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

            iState = STATE_WAIT_FOR_CANCELALL;
            RunIfNotReady(15000000);
        }
        break;

        case STATE_WAIT_FOR_CANCELALL:
        {
            // Cancel did not complete in time
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        default:
            break;

    }
}

void pvplayer_async_test_genericcancelall::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }


    switch (iState)
    {
        case STATE_QUERYINTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // QueryInterface failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                }
                else
                {
                    iState = STATE_CONFIGPARAMS;
                }
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_CONFIGPARAMS:
            iState = STATE_INIT;
            RunIfNotReady();
            break;

        case STATE_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    if (iDownloadOnly)
                    {
                        //For download-only, just wait on the buffering complete event
                        //and then reset engine.
                        if (iNumBufferingComplete > 0)
                        {
                            //If the clip is really short it's possible it may
                            //already be downloaded by now.
                            iState = STATE_RESET;
                            RunIfNotReady();
                        }
                        else
                        {
                            //wait on download to complete.
                            iState = STATE_WAIT_FOR_BUFFCOMPLETE;
                            //5 minute error timeout
                            RunIfNotReady(5*60*1000*1000);
                        }
                    }
                    else
                    {
                        iState = STATE_ADDDATASINK_VIDEO;
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                if ((iContentTooLarge) &&
                        (aResponse.GetCmdStatus() == PVMFErrContentTooLarge))
                {
                    // Test success in this case
                    fprintf(iTestMsgOutputFile, "   INIT returned PVMFErrContentTooLarge\n");
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_REMOVEDATASOURCE;
                    RunIfNotReady();
                }
                else
                {
                    // Init failed
                    PVPATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                }
            }
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUELIST;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUELIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PrintMetadataInfo();
                if ((iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE) || (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL))
                {
                    //wait for data ready, unless we already got it.
                    if (iNumDataReady > 0)
                    {
                        iState = STATE_START;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = STATE_WAIT_FOR_DATAREADY;
                        //5 minute error timeout
                        RunIfNotReady(5*60*1000*1000);
                    }
                }
                else
                {
                    iState = STATE_START;
                    RunIfNotReady();
                }

            }
            else
            {
                // GetMetadataValue failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                }
                else
                {
                    iState = STATE_ADDDATASINK_AUDIO;
                }
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                }
                else
                {
                    iState = STATE_PREPARE;
                }
                RunIfNotReady();
            }
            else
            {
                // AddDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                }
                else
                {
                    iState = STATE_GETMETADATAKEYLIST;
                }
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    iPlayStarted = true;
                    if (oLiveSession == false)
                    {
                        if (oSeekEnable == true)
                        {
                            if (iNumBufferingComplete || (iFileType == PVMF_MIME_ASFFF))
                            {
                                if (!iSeekDone)
                                {
                                    iState = STATE_SETPLAYBACKRANGE;
                                    RunIfNotReady(5*1000*1000);
                                }
                            }

                        }
                        else if (oPauseResumeEnable == true)
                        {
                            iState = STATE_PAUSE;
                            RunIfNotReady(20*1000*1000);
                        }
                        else
                        {
                            if (oWaitForEOS == false)
                            {
                                iState = STATE_STOP;
                                RunIfNotReady(1*30*1000*1000);
                            }
                        }
                    }
                    else
                    {
                        //run for 1 mins and stop
                        iState = STATE_STOP;
                        RunIfNotReady(1*60*1000*1000);
                    }
                }
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;
        case STATE_WAIT_FOR_DATAREADY:
        case STATE_WAIT_FOR_BUFFCOMPLETE:
            //shouldn't get here-- there are no engine commands active in this state.
            PVPATB_TEST_IS_TRUE(false);
            break;


        case STATE_SETPLAYBACKRANGE:
            if ((aResponse.GetCmdStatus() == PVMFSuccess) ||
                    (aResponse.GetCmdStatus() == PVMFErrNotSupported))
            {
                iSeekDone = true;

                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    if (iPlayListURL == true)
                    {
                        if (iFileType != PVMF_MIME_DATA_SOURCE_HTTP_URL)
                        {
                            int32 error;
                            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/delay;valtype=uint32");
                            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
                            iKVPSetAsync.value.uint32_value = 4000;
                            iErrorKVP = NULL;
                            fprintf(iTestMsgOutputFile, "***Resetting Jitter Buffer Duration to = %d ms\n", iKVPSetAsync.value.uint32_value);
                            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
                            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);
                        }
                    }

                    if (oWaitForEOS == false)
                    {
                        iState = STATE_EOSNOTREACHED;
                        RunIfNotReady(180000000);
                    }
                }
            }
            else
            {
                // SetPlaybackRange failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_RESUME;
                    /* Stay paused for 10 seconds */
                    RunIfNotReady(10*1000*1000);
                }
            }
            else
            {
                // Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                    RunIfNotReady();
                }
                else
                {
                    if (oWaitForEOS == false)
                    {
                        iState = STATE_EOSNOTREACHED;
                        RunIfNotReady(180000000);
                    }
                }
            }
            else
            {
                // Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((!iCancelAllWhileProc) && (iEndState == iState))
                {
                    iState = STATE_CANCELALL;
                }
                else
                {
                    iState = STATE_REMOVEDATASINK_AUDIO;
                }
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSink failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
                if (iNumPlay == iTargetNumPlay)
                {
                    iState = STATE_CLEANUPANDCOMPLETE;
                }
                else
                {

                    delete iSourceContextData;
                    iSourceContextData = NULL;

                    delete iStreamDataSource;
                    iStreamDataSource = NULL;

                    delete iDataSource;
                    iDataSource = NULL;

                    delete iDataSinkVideo;
                    iDataSinkVideo = NULL;

                    delete iDataSinkAudio;
                    iDataSinkAudio = NULL;

                    PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeVideo);
                    iIONodeVideo = NULL;

                    PVMediaOutputNodeFactory::DeleteMediaOutputNode(iIONodeAudio);
                    iIONodeAudio = NULL;

                    iMioFactory->DestroyVideoOutput(iMIOFileOutVideo);
                    iMIOFileOutVideo = NULL;

                    iMioFactory->DestroyAudioOutput(iMIOFileOutAudio);
                    iMIOFileOutAudio = NULL;

                    iState = STATE_ADDDATASOURCE;
                }
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_CANCELALL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "CancelAllCommand successful\n");
                // Prepare should not complete
                //PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // Prepare failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_WAIT_FOR_CANCELALL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Cancel succeeded so test passed
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                if (IsBusy())
                {
                    Cancel();
                }
                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
            {
                // Prepare being cancelled. Ignore.
                fprintf(iTestMsgOutputFile, "Command %d Cancelled...\n", iCurrentCmdId);
            }
            else
            {
                // Cancel failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            //PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVInterface* iface = (PVInterface*)(aResponse.GetEventExtensionInterface());
        if (iface != NULL)
        {
            PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
            PVMFErrorInfoMessageInterface* infomsgiface = NULL;
            if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
            {
                PVUuid engerruuid;
                int32 errCodeEng;
                infomsgiface->GetCodeUUID(errCodeEng, engerruuid);

                if (engerruuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeErrorIF =
                        infomsgiface->GetNextMessage();

                    if (sourceNodeErrorIF != NULL)
                    {
                        PVUuid eventuuid;
                        int32 srcErrCode;
                        sourceNodeErrorIF->GetCodeUUID(srcErrCode, eventuuid);
                        if (eventuuid == PVMFSocketNodeEventTypeUUID)
                        {
                            HandleSocketNodeErrors(srcErrCode);
                        }
                        else if (eventuuid == PVMFRTSPClientEngineNodeEventTypeUUID)
                        {
                            HandleRTSPNodeErrors(srcErrCode);
                        }
                        else if (eventuuid == PVMFStreamingManagerNodeEventTypeUUID)
                        {
                            HandleStreamingManagerNodeErrors(srcErrCode);
                        }
                        else if (eventuuid == PVMFJitterBufferNodeEventTypeUUID)
                        {
                            HandleJitterBufferNodeErrors(srcErrCode);
                        }
                        else if (eventuuid == PVMFMediaLayerNodeEventTypeUUID)
                        {
                            HandleMediaLayerNodeErrors(srcErrCode);
                        }
                        else if (eventuuid == PVProtocolEngineNodeErrorEventTypesUUID)
                        {
                            HandleProtocolEngineNodeErrors(srcErrCode);
                        }

                    }
                }
            }
        }
    }
}


void pvplayer_async_test_genericcancelall::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
    if (iface != NULL)
    {
        PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
        PVMFErrorInfoMessageInterface* infomsgiface = NULL;
        if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
        {
            PVUuid engerruuid;
            int32 errCodeEng;
            infomsgiface->GetCodeUUID(errCodeEng, engerruuid);

            if (engerruuid == PVPlayerErrorInfoEventTypesUUID)
            {
                PVMFErrorInfoMessageInterface* sourceNodeErrorIF =
                    infomsgiface->GetNextMessage();

                if (sourceNodeErrorIF != NULL)
                {
                    PVUuid eventuuid;
                    int32 srcErrCode;
                    sourceNodeErrorIF->GetCodeUUID(srcErrCode, eventuuid);
                    if (eventuuid == PVMFSocketNodeEventTypeUUID)
                    {
                        HandleSocketNodeErrors(srcErrCode);
                    }
                    else if (eventuuid == PVMFRTSPClientEngineNodeEventTypeUUID)
                    {
                        HandleRTSPNodeErrors(srcErrCode);
                    }
                    else if (eventuuid == PVMFStreamingManagerNodeEventTypeUUID)
                    {
                        HandleStreamingManagerNodeErrors(srcErrCode);
                    }
                    else if (eventuuid == PVMFJitterBufferNodeEventTypeUUID)
                    {
                        HandleJitterBufferNodeErrors(srcErrCode);
                    }
                    else if (eventuuid == PVMFMediaLayerNodeEventTypeUUID)
                    {
                        HandleMediaLayerNodeErrors(srcErrCode);
                    }
                    else if (eventuuid == PVProtocolEngineNodeErrorEventTypesUUID)
                    {
                        HandleProtocolEngineNodeErrors(srcErrCode);
                    }
                }
            }
        }
    }

    // Wait for engine to handle the error
    Cancel();
}

void pvplayer_async_test_genericcancelall::HandleSocketNodeErrors(int32 aErr)
{
    if (aErr == PVMFSocketNodeErrorSocketServerCreateError)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorSocketServerCreateError\n");
    }
    else if (aErr == PVMFSocketNodeErrorSocketServConnectError)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorSocketServConnectError\n");
    }
    else if (aErr == PVMFSocketNodeErrorUDPSocketRecvError)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorUDPSocketRecvError\n");
    }
    else if (aErr == PVMFSocketNodeErrorUDPSocketSendError)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorUDPSocketSendError\n");
    }
    else if (aErr == PVMFSocketNodeErrorSocketFailure)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorSocketFailure\n");
    }
    else if (aErr == PVMFSocketNodeErrorSocketTimeOut)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorSocketTimeOut\n");
    }
    else if (aErr == PVMFSocketNodeErrorInvalidPortTag)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorInvalidPortTag\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNode- Unknown Error Code\n");
    }

}

void pvplayer_async_test_genericcancelall::HandleRTSPNodeErrors(int32 aErr)
{
    if (aErr == PVMFRTSPClientEngineNodeErrorSocketServerError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorSocketServerError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorDNSLookUpError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorDNSLookUpError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPSocketCreateError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPSocketCreateError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPSocketConnectError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorSocketSendError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorSocketSendError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorSocketRecvError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorSocketRecvError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorSocketError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorSocketError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorOutOfMemory)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorOutOfMemory\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPRequestTooBig)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPRequestTooBig\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPParserError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPParserError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorUnknownRTSPMessage)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorUnknownRTSPMessage\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorIncorrectRTSPMessageType)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorIncorrectRTSPMessageType\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorMalformedRTSPMessage\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorMissingSeqNumInServerResponse)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorMissingSeqNumInServerResponse\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPRequestResponseMismatch)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPRequestResponseMismatch\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPComposeDescribeRequestError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPComposeDescribeRequestError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPComposeSetupRequestError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPComposeSetupRequestError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPComposePlayRequestError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPComposePlayRequestError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPComposePauseRequestError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPComposePauseRequestError\n");
    }
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPComposeStopRequestError)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPComposeStopRequestError\n");
    }
    //"400"      ; Bad Request
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode400)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode400\n");
    }
    //"401"      ; Unauthorized
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode401)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode401\n");
    }
    //"402"      ; Payment Required
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode402)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode402\n");
    }
    //"403"      ; Forbidden
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode403)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode403\n");
    }
    //"404"      ; Not Found
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode404)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode404\n");
    }
    //"405"      ; Method Not Allowed
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode405)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode405\n");
    }
    //"406"      ; Not Acceptable
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode406)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode406\n");
    }
    //"407"      ; Proxy Authentication Required
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode407)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode407\n");
    }
    //"408"      ; Request Time-out
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode408)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode408\n");
    }
    //"410"      ; Gone
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode410)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode410\n");
    }
    //"411"      ; Length Required
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode411)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode411\n");
    }
    //"412"      ; Precondition Failed
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode412)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode412\n");
    }
    //"413"      ; Request Entity Too Large
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode413)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode413\n");
    }
    //"414"      ; Request-URI Too Large
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode414)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode414\n");
    }
    //"415"      ; Unsupported Media Type
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode415)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode415\n");
    }
    //"451"      ; Parameter Not Understood
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode451)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode451\n");
    }
    //"452"      ; Conference Not Found
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode452)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode452\n");
    }
    //"453"      ; Not Enough Bandwidth
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode453)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode453\n");
    }
    //"454"      ; Session Not Found
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode454)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode454\n");
    }
    //"455"      ; Method Not Valid in This State
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode455)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode455\n");
    }
    //"456"      ; Header Field Not Valid for Resource
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode456)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode456\n");
    }
    //"457"      ; Invalid Range
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode457)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode457\n");
    }
    //"458"      ; Parameter Is Read-Only
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode458)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode458\n");
    }
    //"459"      ; Aggregate operation not allowed
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode459)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode459\n");
    }
    //"460"      ; Only aggregate operation allowed
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode460)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode460\n");
    }
    //"461"      ; Unsupported transport
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode461)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode461\n");
    }
    //"462"      ; Destination unreachable
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode462)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode462\n");
    }
    //"500"      ; Internal Server Error
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode500)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode500\n");
    }
    //"501"      ; Not Implemented
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode501)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode501\n");
    }
    //"502"      ; Bad Gateway
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode502)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode502\n");
    }
    //"503"      ; Service Unavailable
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode503)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode503\n");
    }
    //"504"      ; Gateway Time-out
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode504)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode504\n");
    }
    //"505"      ; RTSP Version not supported
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode505)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode505\n");
    }
    //"551"      ; Option not supported
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPErrorCode551)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPErrorCode551\n");
    }
    // Unknown
    else if (aErr == PVMFRTSPClientEngineNodeErrorRTSPCodeUnknown)
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNodeErrorRTSPCodeUnknown\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFRTSPClientEngineNode - Unknown Error Code\n");
    }
}
void pvplayer_async_test_genericcancelall::HandleStreamingManagerNodeErrors(int32 aErr)
{
    if (aErr == PVMFStreamingManagerNodeErrorInvalidRequestPortTag)
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNodeErrorInvalidRequestPortTag\n");
    }
    else if (aErr == PVMFStreamingManagerNodeErrorInvalidPort)
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNodeErrorInvalidPort\n");
    }
    else if (aErr == PVMFStreamingManagerNodeErrorParseSDPFailed)
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNodeErrorParseSDPFailed\n");
    }
    else if (aErr == PVMFStreamingManagerNodeGraphConnectFailed)
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNodeGraphConnectFailed\n");
    }
    else if (aErr == PVMFStreamingManagerNodeGraphDisconnectFailed)
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNodeGraphDisconnectFailed\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFStreamingManagerNode - Unknown Error Code\n");
    }
}

void pvplayer_async_test_genericcancelall::HandleJitterBufferNodeErrors(int32 aErr)
{
    if (aErr == PVMFJitterBufferNodeRemoteInactivityTimerExpired)
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNodeRemoteInactivityTimerExpired\n");
    }
    else if (aErr == PVMFJitterBufferNodeRTCPRRGenerationFailed)
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNodeRTCPRRGenerationFailed\n");
    }
    else if (aErr == PVMFJitterBufferNodeUnableToRegisterIncomingPacket)
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNodeUnableToRegisterIncomingPacket\n");
    }
    else if (aErr == PVMFJitterBufferNodeInputDataPacketHeaderParserError)
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNodeInputDataPacketHeaderParserError\n");
    }
    else if (aErr == PVMFJitterBufferNodeRTCPSRProcFailed)
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNodeRTCPSRProcFailed\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFJitterBufferNode - Unknown Error Code\n");
    }
}

void pvplayer_async_test_genericcancelall::HandleMediaLayerNodeErrors(int32 aErr)
{
    if (aErr == PVMFMediaLayerNodePayloadParserError)
    {
        fprintf(iTestMsgOutputFile, "PVMFMediaLayerNodePayloadParserError\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFMediaLayerNode - Unknown Error Code\n");
    }

}

void pvplayer_async_test_genericcancelall::HandleProtocolEngineNodeErrors(int32 aErr)
{
    // The HTTP status errors are propagated up from the Protocol Engine node and
    // are enumerated in pvmf_protocol_engine_node_events.h. The raw value of the error
    // code can be calculated as an offset from aErr - PVProtocolEngineNodeErrorEventStart.
    if ((aErr >= PVProtocolEngineNodeErrorHTTPErrorCode400) &&
            (aErr <= PVProtocolEngineNodeErrorHTTPCode4xxUnknown))
    {
        fprintf(iTestMsgOutputFile, "PVProtocolEngineNodeError4xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVProtocolEngineNodeError General Error %d\n", aErr);
    }
}

void pvplayer_async_test_genericcancelall::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFInfoErrorHandlingStart:
        {
            fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingStart...\n");
        }
        break;

        case PVMFInfoErrorHandlingComplete:
        {
            fprintf(iTestMsgOutputFile, "PVMFInfoErrorHandlingComplete...\n");
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;

        case PVMFInfoSourceFormatNotSupported:
        {
            // source rollover...
            fprintf(iTestMsgOutputFile, "###PVMFInfoSourceFormatNotSupported...\n");
            PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
            if (iface != NULL)
            {
                PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
                PVMFErrorInfoMessageInterface* infomsgiface = NULL;
                if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
                {
                    int32 infocode;
                    PVUuid infouuid;
                    infomsgiface->GetCodeUUID(infocode, infouuid);
                    if ((infouuid == PVPlayerErrorInfoEventTypesUUID) &&
                            (infocode == PVPlayerInfoAttemptingSourceRollOver))
                    {
                        uint8* localBuf = aEvent.GetLocalBuffer();
                        if (localBuf != NULL)
                        {
                            PVMFFormatType srcFormat = PVMF_MIME_FORMAT_UNKNOWN;
                            oscl_memcpy(&srcFormat, &localBuf[4], sizeof(uint32));
                            fprintf(iTestMsgOutputFile, "###Attempting Source RollOver - NewSrcFormat=%s\n", srcFormat.getMIMEStrPtr());
                            //The clip type needs a rollover, store the actual clip type
                            iFileType = srcFormat;
                        }
                    }
                }
            }
        }
        break;

        case PVMFInfoBufferingStart:
        {
            // Preroll buffer has started filling.
            fprintf(iTestMsgOutputFile, "###PVMFInfoBufferingStart...\n");
            if (oCancelDuringPrepare == true)
            {
                iState = STATE_CANCELALL;
                RunIfNotReady();
            }
        }
        break;

        case PVMFInfoBufferingComplete:
            fprintf(iTestMsgOutputFile, "   PVMFInfoBufferingComplete\n");
            iNumBufferingComplete++;
            // Only one buffering complete should be received
            if (iNumBufferingComplete == 1)
            {
                if (iState == STATE_WAIT_FOR_BUFFCOMPLETE)
                {
                    Cancel();
                    RunIfNotReady();
                }
                if ((!iSeekDone) && iPlayStarted)
                {
                    iState = STATE_SETPLAYBACKRANGE;
                    RunIfNotReady(5*1000*1000);
                }


            }
            break;
            // Check for stop time reached event
        case PVMFInfoEndOfData:
        {
            fprintf(iTestMsgOutputFile, "   PVMFInfoEndOfData\n");
            PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
            if (iface == NULL)
            {
                return;
            }
            PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
            PVMFErrorInfoMessageInterface* infomsgiface = NULL;
            if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
            {
                int32 infocode;
                PVUuid infouuid;
                infomsgiface->GetCodeUUID(infocode, infouuid);
                if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
                {
                    iState = STATE_STOP;
                    Cancel();
                    RunIfNotReady();
                }
            }
        }
        break;

        case PVMFInfoBufferingStatus:
        {
            if ((iFileType == PVMF_MIME_DATA_SOURCE_PVX_FILE) || (iFileType == PVMF_MIME_DATA_SOURCE_HTTP_URL))
            {
                if (aEvent.GetLocalBuffer()[0] == 0
                        || aEvent.GetLocalBuffer()[0] == 100)
                {
                    fprintf(iTestMsgOutputFile, "   PVMFInfoBufferingStatus %d\n", aEvent.GetLocalBuffer()[0]);
                }
                else
                {
                    fprintf(iTestMsgOutputFile, ".%d.", aEvent.GetLocalBuffer()[0]);
                }
            }
            else
            {
                uint8* localBuf = aEvent.GetLocalBuffer();
                if (localBuf != NULL)
                {
                    uint32 bufPercent = 0;
                    oscl_memcpy(&bufPercent, &localBuf[4], sizeof(uint32));
                    fprintf(iTestMsgOutputFile, "###PVMFInfoBufferingStatus - BufferedPercent=%d\n", bufPercent);
                }
            }
        }
        break;

        case PVMFInfoPositionStatus:
        {
            PVInterface* iface = (PVInterface*)(aEvent.GetEventExtensionInterface());
            if (iface == NULL)
            {
                return;
            }
            PVUuid infomsguuid = PVMFErrorInfoMessageInterfaceUUID;
            PVMFErrorInfoMessageInterface* infomsgiface = NULL;
            if (iface->queryInterface(infomsguuid, (PVInterface*&)infomsgiface) == true)
            {
                int32 infocode;
                PVUuid infouuid;
                infomsgiface->GetCodeUUID(infocode, infouuid);
                if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoPlaybackPositionStatus))
                {
                    PVPPlaybackPosition aPos1;
                    uint32 aPos2;

                    aPos1.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                    iPlayer->GetCurrentPositionSync(aPos1);

                    uint8* localbuf = aEvent.GetLocalBuffer();
                    if (aEvent.GetLocalBufferSize() == 8 && localbuf[0] == 1)
                    {
                        oscl_memcpy(&aPos2, &localbuf[4], sizeof(uint32));
                    }
                    else
                    {
                        aPos2 = 0;
                    }
                    fprintf(iTestMsgOutputFile, "Postion:%d\n", aPos2);
                }
            }
        }
        break;
        case PVMFInfoPlayListClipTransition:
        {
            PVExclusivePtr aPtr;
            aEvent.GetEventData(aPtr);
            PVMFRTSPClientEngineNodePlaylistInfoType *myType = (PVMFRTSPClientEngineNodePlaylistInfoType*)(aPtr);
            fprintf(iTestMsgOutputFile, "###PVMFInfoPlayListClipTransition:\n<%s, %d, %d.%03d>;npt=%d.%03d\n", myType->iPlaylistUrlPtr
                    , myType->iPlaylistIndex
                    , myType->iPlaylistOffsetSec
                    , myType->iPlaylistOffsetMillsec
                    , myType->iPlaylistNPTSec
                    , myType->iPlaylistNPTMillsec);
        }
        break;
        case PVMFInfoDataReady:
            fprintf(iTestMsgOutputFile, "   PVMFInfoDataReady\n");

            iNumDataReady++;
            //special handling for very first data ready event.
            if (iNumDataReady == 1)
            {
                if (iState == STATE_WAIT_FOR_DATAREADY)
                {
                    Cancel();
                    RunIfNotReady();
                }
                //for download-then-play test, we should not get data ready
                //until download is complete
                if (iDownloadThenPlay)
                {
                    if (iNumBufferingComplete == 0)
                        PVPATB_TEST_IS_TRUE(false);
                }
                //for download-only test, we should not get data ready at all.
                if (iDownloadOnly)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            break;
        case PVMFInfoUnderflow:
            fprintf(iTestMsgOutputFile, "   PVMFInfoUnderflow\n");
            break;
        case PVMFInfoContentLength:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(iTestMsgOutputFile, "   PVMFInfoContentLength = %d\n", contentSize);
        }
        break;
        case PVMFInfoContentTruncated:
        {
            fprintf(iTestMsgOutputFile, "   PVMFInfoContentTruncated!\n");
        }
        break;

        case PVMFInfoContentType:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            char *constentType = (char *)(eventData);
            fprintf(iTestMsgOutputFile, "   PVMFInfoContentType   = %s\n", constentType);
        }
        break;
    }
}

void pvplayer_async_test_genericcancelall::PrintMetadataInfo()
{
    uint32 i = 0;
    fprintf(iTestMsgOutputFile, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        fprintf(iTestMsgOutputFile, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
    }

    fprintf(iTestMsgOutputFile, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        fprintf(iTestMsgOutputFile, "Value %d:\n", (i + 1));
        fprintf(iTestMsgOutputFile, "   Key string: %s\n", iMetadataValueList[i].key);

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                fprintf(iTestMsgOutputFile, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = '\0';
                fprintf(iTestMsgOutputFile, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                break;

            case PVMI_KVPVALTYPE_INT32:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                break;

            case PVMI_KVPVALTYPE_UINT8:
                fprintf(iTestMsgOutputFile, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                fprintf(iTestMsgOutputFile, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                fprintf(iTestMsgOutputFile, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    fprintf(iTestMsgOutputFile, "   Value:true(1)\n");
                }
                else
                {
                    fprintf(iTestMsgOutputFile, "   Value:false(0)\n");
                }
                break;

            default:
                fprintf(iTestMsgOutputFile, "   Value: UNKNOWN VALUE TYPE\n");
                break;
        }

        fprintf(iTestMsgOutputFile, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);

        if ((oscl_strstr(iMetadataValueList[i].key, "duration")) && iSessionDuration == 0)
        {
            iSessionDuration = iMetadataValueList[i].value.uint32_value;

            // Check the timescale. If not available, assume millisecond (1000)
            const char *retTSstr;
            retTSstr = oscl_strstr(iMetadataValueList[i].key, "timescale=");
            uint32 retTSstrLen = 0;
            uint32 tsstrlen = oscl_strlen(_STRLIT_CHAR("timescale="));
            if (retTSstr != NULL)
            {
                retTSstrLen = oscl_strlen(retTSstr);
                if (retTSstrLen > tsstrlen)
                {
                    uint32 timescale = 0;
                    PV_atoi((char*)(retTSstr + tsstrlen), 'd', (retTSstrLen - tsstrlen), timescale);
                    if (timescale > 0 && timescale != 1000)
                    {
                        // Convert to milliseconds
                        MediaClockConverter mcc(timescale);
                        mcc.update_clock(iSessionDuration);
                        iSessionDuration = mcc.get_converted_ts(1000);
                    }
                }
            }
        }
        if (oscl_strcmp((char*)(iMetadataValueList[i].key),
                        _STRLIT_CHAR("duration;valtype=char*")) == 0)
        {
            if (iPlayListURL == false)
            {
                oLiveSession = true;
            }
        }
    }

    fprintf(iTestMsgOutputFile, "\n\n");
}
