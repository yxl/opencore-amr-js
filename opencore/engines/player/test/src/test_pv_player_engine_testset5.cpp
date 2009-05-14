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
#include "oscl_base.h"

#ifndef TEST_PV_PLAYER_ENGINE_TESTSET5_H_INCLUDED
#include "test_pv_player_engine_testset5.h"
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

#ifndef PV_PLAYER_DATASINKPVMFNODE_H_INCLUDED
#include "pv_player_datasinkpvmfnode.h"
#endif

#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

#ifndef PV_MEDIA_OUTPUT_NODE_FACTORY_H_INCLUDED
#include "pv_media_output_node_factory.h"
#endif

#ifndef PVMF_DOWNLOAD_DATA_SOURCE_H_INCLUDED
#include "pvmf_download_data_source.h"
#endif
#include "pvmf_source_context_data.h"

#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1_factory.h"
#endif

#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_TYPES_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1_types.h"
#endif

#ifndef PVMF_DURATIONINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_durationinfomessage_extension.h"
#endif

#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif

#ifndef PVMF_MP4FFPARSER_EVENTS_H_INCLUDED
#include "pvmf_mp4ffparser_events.h"
#endif

#ifndef PVMF_FILEFORMAT_EVENTS_H_INCLUDED
#include "pvmf_fileformat_events.h"
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

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

#ifndef PVMF_PROTOCOL_ENGINE_NODE_EVENTS_H_INCLUDED
#include "pvmf_protocol_engine_node_events.h"
#endif

#ifndef PVMF_SOCKET_NODE_EVENTS_H_INCLUDED
#include "pvmf_socket_node_events.h"
#endif

#ifndef PVMF_METADTA_INFOMESSAGE_H
#include "pvmf_metadata_infomessage.h"
#endif

#if(RUN_CPMJANUS_TESTCASES)

#ifndef PVMF_JANUS_TYPES_H_INCLUDED
#include "pvmf_janus_types.h"
#endif

#if !(JANUS_IS_LOADABLE_MODULE)
#ifndef PVMF_JANUS_PLUGIN_FACTORY_H_INCLUDED
#include "pvmf_janus_plugin_factory.h"
#endif
#endif

#ifndef PVMI_DRM_KVP_H_INCLUDED
#include "pvmi_drm_kvp.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_TESTSET_CPMJANUS_TYPES_H_INCLUDED
#include "test_pv_player_engine_testset_cpmjanus_types.h"
#endif
#endif//RUN_CPMJANUS_TESTCASES


//Default Fast-track download file
#define DEFAULT_FASTTRACK_DL_FILE "test.pvx"

//Default 3gpp download URL, in both single and wide-char versions.
#if(RUN_MP4FILE_TESTCASES)
#undef DEFAULT_3GPPDL_URL
#define DEFAULT_3GPPDL_URL "http://pvwmsoha.pv.com:7070/MediaDownloadContent/MP4/prog_dl/mpeg4+aac_metadata_qt.mp4"
//#define DEFAULT_3GPPDL_URL "http://pvwmsoha.pv.com:7070/MediaDownloadContent/MP4/prog_dl/ec36-av-clon-100-10-h176x144p-uc-g7a24;16s-rvdqmt.mp4"
#elif(RUN_ASFFILE_TESTCASES)
#undef DEFAULT_3GPPDL_URL
#define DEFAULT_3GPPDL_URL "http://pvwmsoha.pv.com:7070/MediaDownloadContent/UserUploads/av5.wmv"
#endif

#if(RUN_ASFFILE_TESTCASES)
//For slow download use this one
#define DEFAULT_3GPPDL_URL_SLOW "http://reallinux.pv.com:1415/av5.wmv?bitrate=100000"
#elif (RUN_MP4FILE_TESTCASES)
#define DEFAULT_3GPPDL_URL_SLOW "http://reallinux.pv.com:1415/at_vh264qc128_1_amr_12.2_of.3gp?bitrate=100000"
#endif

//Default Bad URL for the error-case test
#define DEFAULT_BADURL "http://reallinux.pv.com:1415/xxxxx_320x240_15fps_200kbps_wmv7.wmv"

#define DEFAULT_TRUNCATED_TEST_URL "http://reallinux.pv.com:1415/av5.wmv?no_content_length=true"

#define DEFAULT_CANCEL_DURING_INIT_TEST_URL "http://reallinux.pv.com:1415/av5.wmv?bitrate=1000"

#define DEFAULT_CANCEL_DURING_INIT_DELAY_TEST_URL "http://reallinux.pv.com:1415/blz4-110302-aac-h263.mp4?bitrate=1000000"

#define DEFAULT_3GPP_PPB_URL "http://wms.pv.com:7070/MediaDownloadContent/UserUploads/youtube_singer.3gp"

#define DEFAULT_SHOUTCAST_URL "http://scfire-dtc-aa05.stream.aol.com/stream/1018"

extern FILE* file;



//#define USING_PROXY

//
//pvplayer_async_test_downloadbase section
//
pvplayer_async_test_downloadbase::~pvplayer_async_test_downloadbase()
{
    delete iDownloadContextData;
}

void pvplayer_async_test_downloadbase::PrintMetadata()
{
    for (uint32 i = 0;i < iMetadataValueList.size();i++)
    {
        if (!iMetadataValueList[i].key)
        {
            fprintf(file, "  Metadata Key Missing!, value ?\n");
            PVPATB_TEST_IS_TRUE(false);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=char*"))
        {
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, iMetadataValueList[i].value.pChar_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=wchar*"))
        {
            OSCL_HeapString<OsclMemAllocator> ostr;
            char buf[2];
            buf[1] = '\0';
            for (uint32 j = 0;;j++)
            {
                if (iMetadataValueList[i].value.pWChar_value[j] == '\0')
                    break;
                buf[0] = iMetadataValueList[i].value.pWChar_value[j];
                ostr += buf;
            }
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, ostr.get_str());
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint32"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.uint32_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=bool"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.bool_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint8*"))
        {
            fprintf(file, "  Metadata Key '%s', len %d\n", iMetadataValueList[i].key, iMetadataValueList[i].length);
        }
        else
        {
            fprintf(file, "  Metadata Key '%s', value ?\n", iMetadataValueList[i].key);
        }
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
    }
    fprintf(file, "\n\n");
}

void pvplayer_async_test_downloadbase::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_downloadbase::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

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
            CreateDataSource();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            break;

        case STATE_CONFIGPARAMS:
        {
            fprintf(iTestMsgOutputFile, "***Configuring Params...\n");

            //set user-agent, make sure to set as "PVPLAYER VersionNumber" to satisfy Fast Track PV server for Fast Track test

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/user-agent;valtype=wchar*;mode=download");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();

//			OSCL_wHeapString<OsclMemAllocator> userAgent(_STRLIT_WCHAR("PVPLAYER 18akljfaljfa"));
            OSCL_wHeapString<OsclMemAllocator> userAgent(_STRLIT_WCHAR("PVPLAYER 18.07.00.02"));
            iKVPSetAsync.value.pWChar_value = userAgent.get_str();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set http version number
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-version;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 0;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set http timeout
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-timeout;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 20;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set number of redirects
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/num-redirect-attempts;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 4;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set extension header
            // set arbitrary extension header one by one
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeader(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=Test;method=GET,HEAD"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeader.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeader.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=GetHeader;method=GET"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderGet.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderGet.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderHead(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=HeadHeader;method=HEAD"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderHead.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderHead.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // Unnecessary Header should not display
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderPost(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=PostHeader;method=POST"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderPost.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderPost.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);


            // set extension header
            // set arbitrary extension headers all together
            PvmiKvp kvpheader[3];
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=download"));
            OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=download;purge-on-redirect"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;purge-on-redirect"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=streaming"));


            kvpheader[0].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeader1(_STRLIT_CHAR("key=X1;value=Y1;method=GET,HEAD"));
            kvpheader[0].value.pChar_value = protocolExtensionHeader1.get_str();
            kvpheader[0].capacity = protocolExtensionHeader1.get_size();

            kvpheader[1].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet1(_STRLIT_CHAR("key=X2;value=Y2;method=GET"));
            kvpheader[1].value.pChar_value = protocolExtensionHeaderGet1.get_str();
            kvpheader[1].capacity = protocolExtensionHeaderGet1.get_size();

            kvpheader[2].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderHead1(_STRLIT_CHAR("key=X3;value=Y3;method=HEAD"));
            kvpheader[2].value.pChar_value = protocolExtensionHeaderHead1.get_str();
            kvpheader[2].capacity = protocolExtensionHeaderHead1.get_size();

            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, kvpheader, 3, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // enable or disable HEAD request
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-header-request-disabled;valtype=bool");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.bool_value = true;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/max-tcp-recv-buffer-size-download;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 64000;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            //set number of TCP recv buffers for progressive playback.
            if (iDownloadContextData
                    && iDownloadContextData->DownloadHTTPData()
                    && iDownloadContextData->DownloadHTTPData()->iPlaybackControl == PVMFSourceContextDataDownloadHTTP::ENoSaveToFile)
            {
                iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/max-tcp-recv-buffer-count-download;valtype=uint32");
                iKVPSetAsync.key = iKeyStringSetAsync.get_str();
                iKVPSetAsync.value.uint32_value = 8;
                iErrorKVP = NULL;
                OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);
            }

            /////////////////////////////////////////////////////////
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;purge-on-redirect");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderCookie(_STRLIT_CHAR("key=Cookie; value=key1=value1;  key2=value2"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderCookie.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderCookie.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);
            /////////////////////////////////////////////////////////


            iState = STATE_INIT;
            RunIfNotReady();
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_WAIT_FOR_BUFFCOMPLETE:
            if (iNumBufferingComplete > 0)
            {
                //Download is complete!
                if (iDownloadOnly)
                {
                    //download-only cases can finish now.
                    iState = STATE_RESET;
                    RunIfNotReady();
                }
                else
                {
                    if (iPauseAfterDownloadComplete == true)
                    {
                        iState = STATE_PAUSE;
                        RunIfNotReady(5*1000*1000);
                    }
                    else if (iRepositionAfterDownloadComplete == true)
                    {
                        iState = STATE_SETPLAYBACKRANGE;
                        RunIfNotReady();
                    }
                    else
                    {
                        //playback cases play 10 more seconds then stop.
                        iState = STATE_STOP;
                        if (iPlayUntilEOS == false)
                        {
                            RunIfNotReady(10*1000*1000);
                        }
                        else
                        {
                            //wait for EOS
                        }
                        if (iPauseResumeAfterUnderflow == true)
                        {
                            fprintf(file, "\n   Buffering Complete before Underflow event, pass the test case but Scenario not tested\n\n");
                            // pass the test case but scenario is not tested, print a message for the user,
                            // now either play for 5 seconds or wait till the EOS.
                        }
                    }
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

        case STATE_ADDDATASINK_VIDEO:
        {
            CreateDataSinkVideo();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_ADDDATASINK_AUDIO:
        {
            CreateDataSinkAudio();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        case STATE_PREPARE2:
        {
            fprintf(iTestMsgOutputFile, "***Preparing\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 100, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 100, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
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

        case STATE_START:
        case STATE_START2:
        {
            fprintf(iTestMsgOutputFile, "***Starting\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        {
            fprintf(iTestMsgOutputFile, "***Pausing\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        {
            fprintf(iTestMsgOutputFile, "***Resuming...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            if (iSessionDuration > 0)
            {
                fprintf(iTestMsgOutputFile, "***Repositioning to %d ms\n", (iSessionDuration / 4));
                PVPPlaybackPosition start, end;
                start.iIndeterminate = false;
                start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                start.iMode = PVPPBPOS_MODE_NOW;
                start.iPosValue.millisec_value = iSessionDuration / 4;
                end.iIndeterminate = true;
                OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            else
            {
                fprintf(iTestMsgOutputFile, "***Set PlayBack Range Not Supported for a session with unknown duration...\n");
                //just play for 10 seconds then stop
                iState = STATE_STOP;
                RunIfNotReady(10*1000*1000);
            }
        }
        break;

        case STATE_STOP:
        case STATE_STOP2:
        {
            fprintf(iTestMsgOutputFile, "***Stopping\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            fprintf(iTestMsgOutputFile, "***Resetting\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            fprintf(file, "Events:\n");
            fprintf(file, "  Num BuffStart %d\n", iNumBufferingStart);
            fprintf(file, "  Num BuffComplete %d\n", iNumBufferingComplete);
            fprintf(file, "  Num Data Ready %d\n", iNumDataReady);
            fprintf(file, "  Num Underflow %d\n", iNumUnderflow);

            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

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

            //call the test-case specific cleanup.
            CleanupData();

            iObserver->TestCompleted(*iTestCase);
        }
        break;
        case STATE_CANCELALL:	//Added for the derived class pvplayer_async_test_cnclall_prtcl_rllovr
        {
            fprintf(iTestMsgOutputFile, "***CancelAllCommands...\n");
            OSCL_TRY(error, iCancelAllCmdId = iPlayer->CancelAllCommands((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            iState = STATE_WAIT_FOR_CANCELALL;
            RunIfNotReady(1000 * 1000 * 10); //if cancell all doesnt get completed in 10 sec, assume failure
        }
        break;
        case STATE_WAIT_FOR_CANCELALL:
        {
            fprintf(iTestMsgOutputFile, "***Cancel did not complete in time...\n");
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


void pvplayer_async_test_downloadbase::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

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

//#if(RUN_CPMJANUS_TESTCASES)

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
                iState = STATE_CONFIGPARAMS;
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
                        fprintf(file, "***Wait for BufferingComplete...\n");
                        iState = STATE_WAIT_FOR_BUFFCOMPLETE;
                        if (iPlayUntilEOS == false)
                        {
                            //5 minute error timeout
                            RunIfNotReady(5*60*1000*1000);
                        }
                    }
                }
                else
                {
                    //normal playback case, continue to add data sinks.
                    iState = STATE_ADDDATASINK_VIDEO;
                    RunIfNotReady();
                }
            }
            else
            {
                if ((iContentTooLarge) &&
                        (aResponse.GetCmdStatus() == PVMFErrContentTooLarge))
                {
                    // Test success in this case
                    fprintf(file, "   INIT returned PVMFErrContentTooLarge\n");
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

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
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

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
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
                iState = STATE_GETMETADATAKEYLIST;
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

        case STATE_PREPARE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START2;
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

        case STATE_WAIT_FOR_DATAREADY:
        case STATE_WAIT_FOR_BUFFCOMPLETE:
            //shouldn't get here-- there are no engine commands active in this state.
            PVPATB_TEST_IS_TRUE(false);
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
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
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                //Display the resulting metadata.
                PrintMetadata();

                //wait for data ready, unless we already got it.
                if (iNumDataReady > 0)
                {
                    iState = STATE_START;
                    RunIfNotReady();
                }
                else
                {
                    fprintf(file, "***Wait for DataReady...\n");
                    iState = STATE_WAIT_FOR_DATAREADY;
                    if (iPlayUntilEOS == false)
                    {
                        //5 minute error timeout
                        RunIfNotReady(5*60*1000*1000);
                    }
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

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPlayStopPlay)
                {
                    //play for 10 seconds then stop
                    iState = STATE_STOP;
                    RunIfNotReady(10*1000*1000);
                    break;
                }
                if (iProgPlayback)
                {
                    //play for 10 seconds or until EOS.
                    iState = STATE_STOP;
                    if (!iPlayUntilEOS)
                        RunIfNotReady(10*1000*1000);
                    break;
                }
                //most other cases wait for buff complete.
                if (iNumBufferingComplete)
                {
                    if (iDownloadThenPlay)
                    {
                        if (iPauseAfterDownloadComplete)
                        {
                            //play for 5 sec then pause
                            iState = STATE_PAUSE;
                            RunIfNotReady(5*1000*1000);
                            break;
                        }
                        else if (iRepositionAfterDownloadComplete)
                        {
                            //play for 5 sec then repos
                            iState = STATE_SETPLAYBACKRANGE;
                            RunIfNotReady(5*1000*1000);
                            break;
                        }
                    }
                    //play for 10 sec or until EOS.
                    iState = STATE_STOP;
                    if (!iPlayUntilEOS)
                        RunIfNotReady(10*1000*1000);
                    break;
                }
                else
                {
                    //wait for buff complete
                    iState = STATE_WAIT_FOR_BUFFCOMPLETE;
                    if (!iPlayUntilEOS)
                    {
                        //5 minute error timeout to avoid test case hang.
                        RunIfNotReady(5*60*1000*1000);
                        break;
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

        case STATE_START2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                //play for 10 seconds then stop
                iState = STATE_STOP2;
                RunIfNotReady(10*1000*1000);
                break;
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                if (iPauseResumeAfterUnderflow == true)
                    RunIfNotReady();
                else
                    /* Stay paused for 10 seconds */
                    RunIfNotReady(5*1000*1000);
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
                if (iPauseResumeAfterUnderflow == false)
                {
                    iState = STATE_STOP;
                    if (iPlayUntilEOS == false)
                    {
                        //just play for 10 seconds then stop
                        RunIfNotReady(10*1000*1000);
                    }
                    else
                    {
                        // wait for EOS
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

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***Repositioning Success...\n");
                //just play for 10 seconds then stop
                iState = STATE_STOP;
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                fprintf(iTestMsgOutputFile, "***Repositioning Failed...\n");
                // Repos failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPlayStopPlay)
                {
                    iState = STATE_PREPARE2;
                    RunIfNotReady();
                    break;
                }
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
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

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
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
                iState = STATE_CLEANUPANDCOMPLETE;
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

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
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
                        else if (eventuuid == PVProtocolEngineNodeErrorEventTypesUUID)
                        {
                            HandleProtocolEngineNodeErrors(srcErrCode, aResponse.GetEventData());
                        }

                    }
                }
            }
        }
    }
}

void pvplayer_async_test_downloadbase::HandleSocketNodeErrors(int32 aErr)
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
    else if (aErr == PVMFSocketNodeError_TCPSocketConnect)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeError_TCPSocketConnect\n");
    }
    else if (aErr == PVMFSocketNodeError_DNSLookup)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeError_DNSLookup\n");
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
    else if (aErr == PVMFSocketNodeErrorInvalidPortConfig)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorInvalidPortConfig\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNode- Unknown Error Code\n");
    }
}


void pvplayer_async_test_downloadbase::HandleProtocolEngineNodeErrors(int32 aErr, OsclAny* aEventData)
{
    // The HTTP status errors are propagated up from the Protocol Engine node and
    // are enumerated in pvmf_protocol_engine_node_events.h. The raw value of the error
    // code can be calculated as an offset from aErr - PVProtocolEngineNodeErrorEventStart.
    if ((aErr >= PVProtocolEngineNodeErrorHTTPErrorCode400) &&
            (aErr <= PVProtocolEngineNodeErrorHTTPCode4xxUnknown))
    {
        if (aErr == PVProtocolEngineNodeErrorHTTPErrorCode401 && aEventData)
        {
            char *realmString = (char *)aEventData;
            fprintf(iTestMsgOutputFile, "   PVMFInfoMFFailure (HTTP Status code = 401), realm = %s\n", realmString);
        }
        else
        {
            fprintf(iTestMsgOutputFile, "PVProtocolEngineNodeError4xx HTTP Status Code %d\n",
                    aErr - PVProtocolEngineNodeErrorEventStart);
        }
    }
    else if ((aErr >= PVProtocolEngineNodeErrorHTTPErrorCode500) &&
             (aErr < PVProtocolEngineNodeErrorHTTPCode5xxUnknownStart))
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError5xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else if ((aErr >= PVProtocolEngineNodeErrorHTTPRedirectCodeStart) &&
             (aErr <= PVProtocolEngineNodeErrorHTTPRedirectCodeEnd))
    {
        fprintf(file, "   ERROR IN REDIRECT: PVProtocolEngineNodeError3xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else if (aErr < PVProtocolEngineNodeErrorNotHTTPErrorStart || aErr > PVProtocolEngineNodeErrorNotHTTPErrorEnd)
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError HTTP Unknown Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError General Error %d\n", aErr);
    }
}

void pvplayer_async_test_downloadbase::PrintJanusError(const PVCmdResponse& aResp)
{
#if(RUN_CPMJANUS_TESTCASES)
//#if 0
    //Get the extended error info.
    if (aResp.GetCmdStatus() != PVMFSuccess)
    {
        PVInterface* iface = (PVInterface*)(aResp.GetEventExtensionInterface());
        if (iface)
        {
            PVUuid errUuid(PVMFErrorInfoMessageInterfaceUUID);
            PVMFErrorInfoMessageInterface* errMsg = NULL;
            if (iface->queryInterface(errUuid, (PVInterface*&)errMsg))
            {
                //search for a janus error in the error list.
                PVUuid janusUuid(PVMFJanusPluginErrorMessageUuid);
                PVMFJanusPluginErrorMessage* janusErr = NULL;
                PVMFErrorInfoMessageInterface* nextErr = errMsg->GetNextMessage();
                while (nextErr)
                {
                    if (nextErr->queryInterface(janusUuid, (PVInterface*&)janusErr))
                    {
                        uint32 drmErr = janusErr->DrmResult();
                        fprintf(file, "  Janus DRM Error! 0x%x\n", drmErr);
                        break;
                    }
                    nextErr = nextErr->GetNextMessage();
                }
            }
        }
    }
#else
    OSCL_UNUSED_ARG(aResp);
#endif
}

void pvplayer_async_test_downloadbase::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrResourceConfiguration error event\n");
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrResource error event\n");
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrCorrupt error event\n");
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrProcessing error event\n");
            break;

        case PVMFErrTimeout:

            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrTimeout error event");
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_downloadbase::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(file, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(file, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }

    // Check for EOS event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
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
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
            {
                fprintf(file, "   GOT PVPlayerInfoEndOfClipReached EVENT\n");
                iNumEOS++;
                if (iState == STATE_STOP)
                {
                    Cancel();
                    RunIfNotReady();
                }
            }
        }
    }

    //Print some events
    switch (aEvent.GetEventType())
    {
        case PVMFInfoBufferingStatus:
        {
            int32 *percent = (int32*)aEvent.GetLocalBuffer();
            if (iPlayStopPlay)
            {
                //download percent shouldn't go backward after the re-start,
                //since that would suggest we're re-starting the download.
                //(don't do this test for PPB case since download gets repositioned
                //along with playback.)
                if (*percent < iLastBufferingStatusVal && !iProgPlayback)
                {
                    PVPATB_TEST_IS_TRUE(false);
                }
            }
            iLastBufferingStatusVal = *percent;
            if (iLastBufferingStatusVal == 0 || iLastBufferingStatusVal == 100)
            {
                fprintf(file, "   PVMFInfoBufferingStatus %d\n", iLastBufferingStatusVal);
            }
            else
            {
                fprintf(file, ".%d.", iLastBufferingStatusVal);
            }
        }
        break;
        case PVMFInfoBufferingStart:
            fprintf(file, "   PVMFInfoBufferingStart\n");
            iNumBufferingStart++;
            if (iPlayStopPlay)
            {
                //we should only get 1 or 2 of these.
                if (iNumBufferingStart == 3)
                    PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                //we should only get one of these.
                if (iNumBufferingStart == 2)
                    PVPATB_TEST_IS_TRUE(false);
            }
            break;

        case PVMFInfoBufferingComplete:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(file, "   PVMFInfoBufferingComplete (contentSize=%d)\n", contentSize);
            iNumBufferingComplete++;
            if (iPlayStopPlay)
            {
                //we should only get 1 or 2 of these.
                if (iNumBufferingComplete == 3)
                    PVPATB_TEST_IS_TRUE(false);
            }
            else
            {
                //we should only get one of these.
                if (iNumBufferingComplete == 2)
                    PVPATB_TEST_IS_TRUE(false);
            }
            if (iNumBufferingComplete == 1)
            {
                if (iState == STATE_WAIT_FOR_BUFFCOMPLETE)
                {
                    Cancel();
                    RunIfNotReady();
                }
            }
        }
        break;

        case PVMFInfoDataReady:
            iNumDataReady++;
            fprintf(file, "\n   PVMFInfoDataReady\n");
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
            iNumUnderflow++;
            fprintf(file, "\n   PVMFInfoUnderflow\n");
            if (iPauseResumeAfterUnderflow == true)
            {
                iState = STATE_PAUSE;
                RunIfNotReady();
            }
            //we should not get underflow before data ready
            if (iNumUnderflow == 1
                    && iNumDataReady == 0)
                PVPATB_TEST_IS_TRUE(false);
            break;
        case PVMFInfoContentLength:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(file, "   PVMFInfoContentLength = %d\n", contentSize);
        }
        break;

        case PVMFInfoContentTruncated:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 downloadSize = (uint32)(eventData);
            fprintf(file, "\n   PVMFInfoContentTruncated! downloadSize = %d\n", downloadSize);

            // check extension info code
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface() &&
                    aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF = msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);

                        if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID &&
                                srcInfoCode == PVMFPROTOCOLENGINENODEInfo_TruncatedContentByServerDisconnect)
                        {
                            fprintf(file, "   PVMFInfoContentTruncated! TruncatedContentByServerDisconnect!\n");
                        }
                    }
                }
            }

        }
        break;

        case PVMFInfoContentType:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            char *constentType = (char *)(eventData);
            fprintf(file, "   PVMFInfoContentType   = %s\n", constentType);
        }
        break;

        case PVMFInfoUnexpectedData:
            fprintf(file, "   PVMFInfoUnexpectedData! Downloaded more data than content-length\n");
            if (iNumBufferingComplete == 0) PVPATB_TEST_IS_TRUE(false); // we should get this info event after buffer complete event
            break;

        case PVMFInfoSessionDisconnect:
            fprintf(file, "   PVMFInfoSessionDisconnect! Got server disconnect after download is complete\n");
            if (iNumBufferingComplete == 0) PVPATB_TEST_IS_TRUE(false); // we should get this info event after buffer complete event
            break;

        case PVMFInfoRemoteSourceNotification:
        {
            //Examine the extended info message to see if this is the
            //"not progressive downloadable" event.  If so, notify the
            //UI.  They may choose to abort the download at this point.
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface()
                    && aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF =
                        msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);
                        if (infoUUID == PVMFFileFormatEventTypesUUID
                                && srcInfoCode == PVMFMP4FFParserInfoNotPseudostreamableFile)
                        {
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (not progressive-downloadable)\n");
                        }
                        else if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID
                                 && (srcInfoCode >= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode300 &&
                                     srcInfoCode <= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode307))
                        {
                            PVExclusivePtr eventData;
                            aEvent.GetEventData(eventData);
                            char *redirectUrl = (char *)(eventData);
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (HTTP Status code = %d), redirectUrl = %s\n",
                                    srcInfoCode - PVMFPROTOCOLENGINENODEInfo_Redirect, redirectUrl);
                        }

                    }
                }
            }
        }
        break;
        case PVMFInfoTrackDisable:
        {
            PVExclusivePtr aPtr;
            aEvent.GetEventData(aPtr);
            int32 iDisableTrack = (int32)aPtr;
            if (!iDisableTrack)
            {
                fprintf(iTestMsgOutputFile, "###PVMFInfoTrackDisable - DisableTrack=%d\n", iDisableTrack);
            }
        }
        break;
        default:
            break;
    }
}

#if RUN_FASTTRACK_TESTCASES
//
// pvplayer_async_test_ftdlnormal section
//
pvplayer_async_test_ftdlnormal::~pvplayer_async_test_ftdlnormal()
{
}

void pvplayer_async_test_ftdlnormal::CreateDataSource()
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
        iDownloadURL.set(wtemp, wtemplen);
        delete [] wtemp;
    }

    //create the opaque data
    iDownloadProxy = _STRLIT_CHAR("");
    int32 iDownloadProxyPort = 0;

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("mydlconfig");

    iDownloadMaxfilesize = 0x7FFFFFFF;

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_ftdownload.loc");

    iContentTooLarge = false;

    //bool aIsNewSession = false;
    bool aIsNewSession = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadPVXSourceContext();
    iDownloadContextData->DownloadPVXData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadPVXData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadPVXData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadPVXData()->iMaxFileSize = iDownloadMaxfilesize;
    iDownloadContextData->DownloadPVXData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadPVXData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadPVXData()->iPvxInfo = &iDownloadPvxInfo;

    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);
    iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_PVX_FILE);
    iDataSource->SetDataSourceContextData(iDownloadContextData);

}
void pvplayer_async_test_ftdlnormal::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_ftdlnormal_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_ftdlnormal::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_ftdlnormal_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}
#endif

//
// pvplayer_async_test_3gppdlnormal section
//
pvplayer_async_test_3gppdlnormal::~pvplayer_async_test_3gppdlnormal()
{
}

void pvplayer_async_test_3gppdlnormal::CreateDataSource()
{

    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        if (iPlayStopPlay)
        {//use slow download simulator so we test stop during download.
            fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL_SLOW);
            url = DEFAULT_3GPPDL_URL_SLOW;
        }
        else
        {
            fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
            url = DEFAULT_3GPPDL_URL;
        }
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);

    if (iProtocolRollOver == true)
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
        iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }
    else
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppdownload.loc");

    int32 iDownloadProxyPort;
#if PVPLAYER_TEST_ENABLE_PROXY
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 7070;
#else
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 0;
#endif

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl.cfg");

    iContentTooLarge = false;
    uint32 iMaxFileSize = 0x7FFFFFFF;
    bool aIsNewSession = true;

#if(RUN_CPMJANUS_TESTCASES) && !(JANUS_IS_LOADABLE_MODULE)
    //Select the device info configuration.
    bool ok = GetJanusFactories(iTestNumber
                                , iDrmDeviceInfoFactory
                                , iDrmSystemClockFactory);
    if (!ok)
        PVPATB_TEST_IS_TRUE(false);

    //Define the janus configuration.
    PVMFJanusPluginConfiguration config;
    config.iDeviceInfoFactory = iDrmDeviceInfoFactory;
    config.iSystemClockFactory = iDrmSystemClockFactory;
    if (!RegisterJanusPlugin(config))
    {
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }
#endif

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iUserID = _STRLIT_CHAR("abc");
    iDownloadContextData->DownloadHTTPData()->iUserPasswd = _STRLIT_CHAR("xyz");
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

#if !(JANUS_IS_LOADABLE_MODULE)
#if RUN_CPMJANUS_TESTCASES
bool pvplayer_async_test_3gppdlnormal::RegisterJanusPlugin(PVMFJanusPluginConfiguration& aConfig)
{
#if(RUN_CPMJANUS_TESTCASES)
//#if 0
    //Connect to plugin registry
    PVMFStatus status;
    status = iPluginRegistryClient.Connect();
    if (status != PVMFSuccess)
    {
        PVPATB_TEST_IS_TRUE(false);
        return false;
    }
    //Create & the plugin factory.
    iPluginFactory = new PVMFJanusPluginFactory(aConfig);
    if (!iPluginFactory)
    {
        PVPATB_TEST_IS_TRUE(false);
        return false;
    }
    //Register the plugin factory.
    iPluginMimeType = PVMF_CPM_MIME_JANUS_PLUGIN;
    if (iPluginRegistryClient.RegisterPlugin(iPluginMimeType, *iPluginFactory) != PVMFSuccess)
    {
        PVPATB_TEST_IS_TRUE(false);
        return false;
    }
    return true;
#else
    //OSCL_UNUSED_ARG(aConfig);
    return false;
#endif
}
#endif
#endif

void pvplayer_async_test_3gppdlnormal::CleanupData()
{
#if(RUN_CPMJANUS_TESTCASES) && !(JANUS_IS_LOADABLE_MODULE)
    //close the plugin registry client session.
    iPluginRegistryClient.Close();

    //delete the plugin factory.
    if (iPluginFactory)
    {
        delete iPluginFactory;
        iPluginFactory = NULL;
    }
    CleanupJanusFactories(iTestNumber,
                          iDrmDeviceInfoFactory,
                          iDrmSystemClockFactory);
#endif
}

void pvplayer_async_test_3gppdlnormal::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_3gppdlnormal::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}

//
// pvplayer_async_test_ppbnormal section
//
pvplayer_async_test_ppbnormal::~pvplayer_async_test_ppbnormal()
{
}

void pvplayer_async_test_ppbnormal::CreateDataSource()
{

    OSCL_HeapString<OsclMemAllocator> url(iFileName);

    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
        url = DEFAULT_3GPPDL_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);

    if (iProtocolRollOver == true)
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
        iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }
    else
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }

    iDownloadFilename = NULL;

    int32 iDownloadProxyPort;
#if PVPLAYER_TEST_ENABLE_PROXY
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 7070;
#else
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 0;
#endif

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("ppbnormal.cfg");

    iContentTooLarge = false;
    uint32 iMaxFileSize = 0x7FFFFFFF;
    bool aIsNewSession = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iUserID = _STRLIT_CHAR("abc");
    iDownloadContextData->DownloadHTTPData()->iUserPasswd = _STRLIT_CHAR("xyz");
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::ENoSaveToFile;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_ppbnormal::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_ppbnormal::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}

// pvplayer_async_test_3gppdlnormal_dlthenplay section
//
pvplayer_async_test_3gppdlnormal_dlthenplay::~pvplayer_async_test_3gppdlnormal_dlthenplay()
{
}

void pvplayer_async_test_3gppdlnormal_dlthenplay::CreateDataSource()
{
    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
        url = DEFAULT_3GPPDL_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);
    iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppdownload_dlthenplay.loc");

    int32 iDownloadProxyPort;
#if PVPLAYER_TEST_ENABLE_PROXY
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 7070;
#else
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 0;
#endif

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl_dlthenplay.cfg");

    iContentTooLarge = false;
    uint32 iMaxFileSize = 0x7FFFFFFF;
    bool aIsNewSession = true;

    iDownloadThenPlay = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAfterDownload;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_3gppdlnormal_dlthenplay::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_dlthenplay_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_3gppdlnormal_dlthenplay::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_dlthenplay_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}
//
// pvplayer_async_test_3gppdlnormal_dlonly section
//
pvplayer_async_test_3gppdlnormal_dlonly::~pvplayer_async_test_3gppdlnormal_dlonly()
{
}

void pvplayer_async_test_3gppdlnormal_dlonly::CreateDataSource()
{

    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
        url = DEFAULT_3GPPDL_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);
    iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppdownload_dlonly.loc");

    int32 iDownloadProxyPort;
#if PVPLAYER_TEST_ENABLE_PROXY
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 7070;
#else
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 0;
#endif

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl_dlonly.cfg");

    iContentTooLarge = false;
    uint32 iMaxFileSize = 0x7FFFFFFF;
    bool aIsNewSession = true;

    iDownloadOnly = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::ENoPlayback;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_3gppdlnormal_dlonly::CreateDataSinkVideo()
{
    //won't be called
    PVPATB_TEST_IS_TRUE(false);
}

void pvplayer_async_test_3gppdlnormal_dlonly::CreateDataSinkAudio()
{
    //won't be called
    PVPATB_TEST_IS_TRUE(false);
}

//
// pvplayer_async_test_3gppdlcancelduringinit section
//
void pvplayer_async_test_3gppdlcancelduringinit::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_3gppdlcancelduringinit::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            OSCL_HeapString<OsclMemAllocator> url(iFileName);
            OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
            default_source += DEFAULTSOURCEFILENAME;
            if (url == default_source)
            {
                fprintf(file, "Setting source to %s\n", DEFAULT_CANCEL_DURING_INIT_TEST_URL);
                url = DEFAULT_CANCEL_DURING_INIT_TEST_URL;
            }
            oscl_wchar wbuf[2];
            wbuf[1] = '\0';
            for (uint32 i = 0;i < url.get_size();i++)
            {
                wbuf[0] = (oscl_wchar)url.get_cstr()[i];
                iDownloadURL += wbuf;
            }

            iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
            iDownloadFilename += _STRLIT_WCHAR("test_3gppdlcancelduringinit.loc");

            iDownloadProxy = _STRLIT_CHAR("");
            int32 iDownloadProxyPort = 0;

            iConfigFileName = OUTPUTNAME_PREPEND_WSTRING;
            iConfigFileName += _STRLIT_WCHAR("my3gppdlcancel.cfg");

            uint32 iMaxFileSize = 0x7FFFFFFF;
            bool aIsNewSession = true;
            PVMFDownloadDataSourceHTTP::TPVPlaybackControl iPlaybackMode = PVMFDownloadDataSourceHTTP::EAsap;
            iDownloadHttpContextData = new PVMFDownloadDataSourceHTTP(aIsNewSession,
                    iConfigFileName,
                    iDownloadFilename,
                    iMaxFileSize,
                    iDownloadProxy,
                    iDownloadProxyPort,
                    iPlaybackMode);

            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(iDownloadURL);
            iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
            iDataSource->SetDataSourceContextData(iDownloadHttpContextData);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

            iState = STATE_CANCELALL;
            //if this is commmented out. The BufferingStart(change below) could do RunIfInactiv()
            //RunIfNotReady(10*1000*1000);
        }
        break;

        case STATE_CANCELALL:
        {
            if (iCancelCommandExecuted)
            {
                fprintf(file, "\nError - Cancel command being executed multiple times, please verify ...\n");
            }
            fprintf(file, "\n Cancel all ...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->CancelAllCommands((OsclAny*) & iContextObject));
            iCancelCommandExecuted = true;
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

            iState = STATE_WAIT_FOR_CANCELALL;
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

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDownloadHttpContextData;
            iDownloadHttpContextData = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_3gppdlcancelduringinit::CommandCompleted(const PVCmdResponse& aResponse)
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

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
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

        case STATE_CANCELALL:
            if (!iCancelCommandExecuted)
            {
                fprintf(file, "\n Error - Cancel command not executed, Pass the test case but scenario not tested...\n");
            }
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Init should not complete
                iState = STATE_RESET;
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

        case STATE_WAIT_FOR_CANCELALL:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aResponse.GetCmdId() != iCurrentCmdId)
                {
                    // Init success. Ignore.
                    fprintf(file, "\n Init Completed ...\n");
                }
                else
                {
                    fprintf(file, "\n Cancel all complete ...\n");
                    // Cancel succeeded so test passed
                    PVPATB_TEST_IS_TRUE(true);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    if (IsBusy())
                    {
                        Cancel();
                    }
                    RunIfNotReady();
                }
            }
            else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
            {
                PVPATB_TEST_IS_TRUE(true);
                // Init being cancelled. Ignore.
            }
            else
            {
                // Cancel failed
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
                iState = STATE_CLEANUPANDCOMPLETE;
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

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_3gppdlcancelduringinit::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_3gppdlcancelduringinit::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(file, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(file, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }

    if (iState == STATE_CANCELALL && aEvent.GetEventType() == PVMFInfoBufferingStart)
    {
        RunIfNotReady(10*1000);
    }

    switch (aEvent.GetEventType())
    {
        case PVMFInfoBufferingStatus:
        {
            int32 *percent = (int32*)aEvent.GetLocalBuffer();
            if (*percent == 0 || *percent == 100)
            {
                fprintf(file, "   PVMFInfoBufferingStatus %d\n", *percent);
            }
            else
            {
                fprintf(file, ".%d.", *percent);
            }
        }
        break;
        case PVMFInfoBufferingStart:
            fprintf(file, "   PVMFInfoBufferingStart\n");
            break;
        case PVMFInfoBufferingComplete:
            fprintf(file, "   PVMFInfoBufferingComplete\n");
            break;
        case PVMFInfoContentLength:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(file, "   PVMFInfoContentLength = %d\n", contentSize);
        }
        break;
        case PVMFInfoContentTruncated:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 downloadSize = (uint32)(eventData);
            fprintf(file, "\n   PVMFInfoContentTruncated! downloadSize = %d\n", downloadSize);
        }
        break;

        case PVMFInfoContentType:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            char *constentType = (char *)(eventData);
            fprintf(file, "   PVMFInfoContentType   = %s\n", constentType);
        }
        break;

        case PVMFInfoRemoteSourceNotification:
        {
            //Examine the extended info message to see if this is the
            //"not progressive downloadable" event.  If so, notify the
            //UI.  They may choose to abort the download at this point.
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface()
                    && aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF =
                        msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);
                        if (infoUUID == PVMFFileFormatEventTypesUUID
                                && srcInfoCode == PVMFMP4FFParserInfoNotPseudostreamableFile)
                        {
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (not progressive-downloadable)\n");
                        }
                        else if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID
                                 && (srcInfoCode >= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode300 &&
                                     srcInfoCode <= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode307))
                        {
                            PVExclusivePtr eventData;
                            aEvent.GetEventData(eventData);
                            char *redirectUrl = (char *)(eventData);
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (HTTP Status code = %d), redirectUrl = %s\n",
                                    srcInfoCode - PVMFPROTOCOLENGINENODEInfo_Redirect, redirectUrl);
                        }

                    }
                }
            }
        }
        break;
        default:
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////////////
//
// pvplayer_async_test_3gppdlcancelduringinitdelay section
// This test is the same as 105 above, except we delay the cancel call
// It is only made once we receive download status events thus ensuring that Init has
// been received by the parser. This method will fail for small files or high bandwidth!
void pvplayer_async_test_3gppdlcancelduringinitdelay::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_3gppdlcancelduringinitdelay::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

            OSCL_TRY(error, iPlayer = PVPlayerFactory::CreatePlayer(this, this, this));
            if (error)
            {
                PVPATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            OSCL_HeapString<OsclMemAllocator> url(iFileName);
            OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
            default_source += DEFAULTSOURCEFILENAME;
            if (url == default_source)
            {
                fprintf(file, "Setting source to %s\n", DEFAULT_CANCEL_DURING_INIT_DELAY_TEST_URL);
                url = DEFAULT_CANCEL_DURING_INIT_DELAY_TEST_URL;
            }
            oscl_wchar wbuf[2];
            wbuf[1] = '\0';
            for (uint32 i = 0;i < url.get_size();i++)
            {
                wbuf[0] = (oscl_wchar)url.get_cstr()[i];
                iDownloadURL += wbuf;
            }

            iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
            iDownloadFilename += _STRLIT_WCHAR("test_3gppdlcancelduringinit.loc");

            iDownloadProxy = _STRLIT_CHAR("");
            int32 iDownloadProxyPort = 0;

            iConfigFileName = OUTPUTNAME_PREPEND_WSTRING;
            iConfigFileName += _STRLIT_WCHAR("my3gppdlcancel.cfg");

            uint32 iMaxFileSize = 0x7FFFFFFF;
            bool aIsNewSession = true;
            PVMFDownloadDataSourceHTTP::TPVPlaybackControl iPlaybackMode = PVMFDownloadDataSourceHTTP::EAsap;
            iDownloadHttpContextData = new PVMFDownloadDataSourceHTTP(aIsNewSession,
                    iConfigFileName,
                    iDownloadFilename,
                    iMaxFileSize,
                    iDownloadProxy,
                    iDownloadProxyPort,
                    iPlaybackMode);

            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(iDownloadURL);
            iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
            iDataSource->SetDataSourceContextData(iDownloadHttpContextData);
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iInitCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

            //change state but don't call runifinactive yet.
            // wait for first Buffering Status event first because by then most nodes will
            // actually have started processing Init. This will fail for short clips though
            // where Init returns immediately.
            iState = STATE_CANCELALL;
        }
        break;

        case STATE_CANCELALL:
        {
            if (iCancelCommandExecuted)
            {
                fprintf(file, "\nError - Cancel command being executed multiple times, please verify ...\n");
            }
            fprintf(file, "\n Cancel all waiting 15 seconds to complete or cancel download ...\n");
            OSCL_TRY(error, iCurrentCmdId = iPlayer->CancelAllCommands((OsclAny*) & iContextObject));
            iCancelCommandExecuted = true;
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());

            iState = STATE_WAIT_FOR_CANCELALL;
            // 15 second watchdog timer. if Init and cancelAll do not complete until then-> error
            RunIfNotReady(15*1000*1000);
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

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDownloadHttpContextData;
            iDownloadHttpContextData = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_3gppdlcancelduringinitdelay::CommandCompleted(const PVCmdResponse& aResponse)
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

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_INIT;
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

        case STATE_CANCELALL:
            if (!iCancelCommandExecuted)
            {
                fprintf(file, "\n Init complete before CancelAll was issued. Test case passed but scenario not tested \n");
            }
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                // Init completed early, pass the test case but scenario not tested.
                iState = STATE_RESET;
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

        case STATE_WAIT_FOR_CANCELALL:
            if (aResponse.GetCmdId() == iCurrentCmdId && // cancel command returns
                    iInitCmdId == -1 && // init has been completed
                    aResponse.GetCmdStatus() == PVMFSuccess) // cancel was successful
            {
                fprintf(file, "\n Cancel all complete ...\n");

                // Cancel succeeded so test passed
                PVPATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                if (IsBusy())
                {
                    Cancel();
                }
                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus() == PVMFErrCancelled &&  // init was cancelled
                     aResponse.GetCmdId() == iInitCmdId)
            {
                PVPATB_TEST_IS_TRUE(true);
                // Init has been cancelled - mark as done
                iInitCmdId = -1;
            }
            else
            {
                // Cancel failed
                fprintf(file, "\n Previous command was not cancelled or cancel failed ...\n");

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
                iState = STATE_CLEANUPANDCOMPLETE;
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

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvplayer_async_test_3gppdlcancelduringinitdelay::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
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

    // Wait for engine to handle the error
    Cancel();
}


void pvplayer_async_test_3gppdlcancelduringinitdelay::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(file, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(file, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }

    switch (aEvent.GetEventType())
    {
        case PVMFInfoBufferingStatus:
        {
            int32 *percent = (int32*)aEvent.GetLocalBuffer();
            if (*percent == 0 || *percent == 100)
            {
                fprintf(file, "   PVMFInfoBufferingStatus %d\n", *percent);
            }
            else
            {
                fprintf(file, ".%d.", *percent);
            }
            if (iState == STATE_CANCELALL && !iCancelCommandExecuted)
            {
                // now is the time to call CancelAll since processing of download has started
                RunIfNotReady();
            }
        }
        break;
        case PVMFInfoBufferingStart:
            fprintf(file, "   PVMFInfoBufferingStart\n");
            break;
        case PVMFInfoBufferingComplete:
            fprintf(file, "   PVMFInfoBufferingComplete\n");
            break;
        case PVMFInfoContentLength:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(file, "   PVMFInfoContentLength = %d\n", contentSize);
        }
        break;
        case PVMFInfoContentTruncated:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 downloadSize = (uint32)(eventData);
            fprintf(file, "\n   PVMFInfoContentTruncated! downloadSize = %d\n", downloadSize);
        }
        break;

        case PVMFInfoContentType:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            char *constentType = (char *)(eventData);
            fprintf(file, "   PVMFInfoContentType   = %s\n", constentType);
        }
        break;

        case PVMFInfoRemoteSourceNotification:
        {
            //Examine the extended info message to see if this is the
            //"not progressive downloadable" event.  If so, notify the
            //UI.  They may choose to abort the download at this point.
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface()
                    && aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF =
                        msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);
                        if (infoUUID == PVMFFileFormatEventTypesUUID
                                && srcInfoCode == PVMFMP4FFParserInfoNotPseudostreamableFile)
                        {
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (not progressive-downloadable)\n");
                        }
                        else if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID
                                 && (srcInfoCode >= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode300 &&
                                     srcInfoCode <= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode307))
                        {
                            PVExclusivePtr eventData;
                            aEvent.GetEventData(eventData);
                            char *redirectUrl = (char *)(eventData);
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (HTTP Status code = %d), redirectUrl = %s\n",
                                    srcInfoCode - PVMFPROTOCOLENGINENODEInfo_Redirect, redirectUrl);
                        }

                    }
                }
            }
        }
        break;
        default:
            break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////

//
// pvplayer_async_test_3gppdlcontenttoolarge section
//
pvplayer_async_test_3gppdlcontenttoolarge::~pvplayer_async_test_3gppdlcontenttoolarge()
{
}


void pvplayer_async_test_3gppdlcontenttoolarge::CreateDataSource()
{
    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
        url = DEFAULT_3GPPDL_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);
    iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppcontenttoolarge.loc");

    iDownloadProxy = _STRLIT_CHAR("");
    int32 iDownloadProxyPort = 0;

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl_contenttoolarge.cfg");

    // Set the max file size to a small number
    uint32 iMaxFileSize = 0x7FFF;
    fprintf(file, "   Setting iMaxFileSize to %d\n", iMaxFileSize);

    // Set the iContentTooLarge to TRUE.  This will return success
    //   for INIT failure of PVMFErrContentTooLarge
    iContentTooLarge = true;

    bool aIsNewSession = true;

    iDownloadOnly = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::ENoPlayback;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_3gppdlcontenttoolarge::CreateDataSinkVideo()
{
    //won't be called
    PVPATB_TEST_IS_TRUE(false);
}

void pvplayer_async_test_3gppdlcontenttoolarge::CreateDataSinkAudio()
{
    //won't be called
    PVPATB_TEST_IS_TRUE(false);
}


//
// pvplayer_async_test_3gppdlContentTruncated section
//
pvplayer_async_test_3gppdlContentTruncated::~pvplayer_async_test_3gppdlContentTruncated()
{
}

void pvplayer_async_test_3gppdlContentTruncated::CreateDataSource()
{
    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_TRUNCATED_TEST_URL);
        url = DEFAULT_TRUNCATED_TEST_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);
    iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppdownload_truncated.loc");

    iDownloadProxy = _STRLIT_CHAR("");
    int32 iDownloadProxyPort = 0;

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl_truncated.cfg");

    iContentTooLarge = false;

    // Set the iMaxFileSize smaller than the downloading file
    uint32 iMaxFileSize = 3000; //0x7FFFF;

    bool aIsNewSession = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_3gppdlContentTruncated::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdl_truncated_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_3gppdlContentTruncated::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdl_truncated_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}


//
//pvplayer_async_test_ppb_base section
//

void pvplayer_async_test_ppb_base::PrintMetadata()
{
    for (uint32 i = 0;i < iMetadataValueList.size();i++)
    {
        if (!iMetadataValueList[i].key)
        {
            fprintf(file, "  Metadata Key Missing!, value ?\n");
            PVPATB_TEST_IS_TRUE(false);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=char*"))
        {
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, iMetadataValueList[i].value.pChar_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=wchar*"))
        {
            OSCL_HeapString<OsclMemAllocator> ostr;
            char buf[2];
            buf[1] = '\0';
            for (uint32 j = 0;;j++)
            {
                if (iMetadataValueList[i].value.pWChar_value[j] == '\0')
                    break;
                buf[0] = iMetadataValueList[i].value.pWChar_value[j];
                ostr += buf;
            }
            fprintf(file, "  Metadata Key '%s', value '%s'\n", iMetadataValueList[i].key, ostr.get_str());
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint32"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.uint32_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=bool"))
        {
            fprintf(file, "  Metadata Key '%s', value %d\n", iMetadataValueList[i].key, iMetadataValueList[i].value.bool_value);
        }
        else if (oscl_strstr(iMetadataValueList[i].key, "valtype=uint8*"))
        {
            fprintf(file, "  Metadata Key '%s', len %d\n", iMetadataValueList[i].key, iMetadataValueList[i].length);
        }
        else
        {
            fprintf(file, "  Metadata Key '%s', value ?\n", iMetadataValueList[i].key);
        }
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
    }
    fprintf(file, "\n\n");
}

void pvplayer_async_test_ppb_base::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvplayer_async_test_ppb_base::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iPlayer = NULL;

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
            CreateDataSource();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            break;

        case STATE_CONFIGPARAMS:
        {
            fprintf(iTestMsgOutputFile, "***Configuring Params...\n");

            //set user-agent, make sure to set as "PVPLAYER VersionNumber" to satisfy Fast Track PV server for Fast Track test

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/user-agent;valtype=wchar*;mode=download");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();

//                      OSCL_wHeapString<OsclMemAllocator> userAgent(_STRLIT_WCHAR("PVPLAYER 18akljfaljfa"));
            OSCL_wHeapString<OsclMemAllocator> userAgent(_STRLIT_WCHAR("PVPLAYER 18.07.00.02"));
            iKVPSetAsync.value.pWChar_value = userAgent.get_str();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set http version number
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-version;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 0;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set http timeout
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-timeout;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 20;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set number of redirects
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/num-redirect-attempts;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 4;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // set extension header
            // set arbitrary extension header one by one
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeader(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=Test;method=GET,HEAD"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeader.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeader.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=GetHeader;method=GET"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderGet.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderGet.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderHead(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=HeadHeader;method=HEAD"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderHead.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderHead.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // Unnecessary Header should not display
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderPost(_STRLIT_CHAR("key=PVPlayerCoreEngineTest;value=PostHeader;method=POST"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderPost.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderPost.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);


            // set extension header
            // set arbitrary extension headers all together
            PvmiKvp kvpheader[3];
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=download"));
            OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=download;purge-on-redirect"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;purge-on-redirect"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*"));
            //OSCL_StackString<128> paramkey(_STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;mode=streaming"));


            kvpheader[0].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeader1(_STRLIT_CHAR("key=X1;value=Y1;method=GET,HEAD"));
            kvpheader[0].value.pChar_value = protocolExtensionHeader1.get_str();
            kvpheader[0].capacity = protocolExtensionHeader1.get_size();

            kvpheader[1].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderGet1(_STRLIT_CHAR("key=X2;value=Y2;method=GET"));
            kvpheader[1].value.pChar_value = protocolExtensionHeaderGet1.get_str();
            kvpheader[1].capacity = protocolExtensionHeaderGet1.get_size();

            kvpheader[2].key = paramkey.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderHead1(_STRLIT_CHAR("key=X3;value=Y3;method=HEAD"));
            kvpheader[2].value.pChar_value = protocolExtensionHeaderHead1.get_str();
            kvpheader[2].capacity = protocolExtensionHeaderHead1.get_size();

            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, kvpheader, 3, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            // enable or disable HEAD request
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/http-header-request-disabled;valtype=bool");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.bool_value = true;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/max-tcp-recv-buffer-size-download;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 64000;
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            /////////////////////////////////////////////////////////
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/net/protocol-extension-header;valtype=char*;purge-on-redirect");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            OSCL_HeapString<OsclMemAllocator> protocolExtensionHeaderCookie(_STRLIT_CHAR("key=Cookie; value=key1=value1;  key2=value2"));
            iKVPSetAsync.value.pChar_value = protocolExtensionHeaderCookie.get_str();
            iKVPSetAsync.capacity = protocolExtensionHeaderCookie.get_size();
            iErrorKVP = NULL;
            OSCL_TRY(error, iPlayerCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);
            /////////////////////////////////////////////////////////


            iState = STATE_INIT;
            RunIfNotReady();
        }
        break;

        case STATE_INIT:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Init((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_WAIT_FOR_BUFFCOMPLETE:
            if (iNumBufferingComplete > 0)
            {
                if (iRepositionAfterDownloadComplete == false)
                {
                    //playback cases play 10 more seconds then stop.
                    iState = STATE_STOP;
                    if (iPlayUntilEOS == false)
                    {
                        RunIfNotReady(10*1000*1000);
                    }
                    else
                    {
                        //wait for EOS
                    }
                }
                else
                {
                    iState = STATE_SETPLAYBACKRANGE;
                    RunIfNotReady();
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

        case STATE_ADDDATASINK_VIDEO:
        {
            CreateDataSinkVideo();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;


        case STATE_ADDDATASINK_AUDIO:
        {
            CreateDataSinkAudio();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->AddDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PREPARE:
        case STATE_PREPARE_AFTERSTOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Prepare((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYLIST:
        {
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataKeys(iMetadataKeyList, 0, 100, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUELIST:
        {
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->GetMetadataValues(iMetadataKeyList, 0, 100, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
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

        case STATE_START:
        case STATE_START_AFTERSTOP:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Start((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE:
        {
            if (iSessionDuration > 0)
            {
                fprintf(iTestMsgOutputFile, "***Repositioning to %d ms\n", (iSessionDuration / 4));
                PVPPlaybackPosition start, end;
                start.iIndeterminate = false;
                start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                start.iMode = PVPPBPOS_MODE_NOW;
                start.iPosValue.millisec_value = iSessionDuration / 4;
                end.iIndeterminate = true;
                OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            else
            {
                fprintf(iTestMsgOutputFile, "***Set PlayBack Range Not Supported for a session with unknown duration...\n");
            }
        }
        break;

        case STATE_SETPLAYBACKRANGE_AFTERSTART:
        {
            // default clip duration is ~206000ms
            // seek to almost the end of duration (forward)
            // so to trigger a download complete condition
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            if (iForwardStep)
            {
                fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms\n", iForwardSeekTime);
                start.iPosValue.millisec_value = iForwardSeekTime;
                if (iForwardSeekTime < iEndTime)
                {
                    iForwardSeekTime += iForwardSeekStep;
                }
                else
                {
                    iForwardSeekTime = iBeginTime;
                }
            }
            else
            {
                fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms\n", 180000);
                start.iPosValue.millisec_value = 180000;
            }
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE_BEFORESTART:
        {
            // default clip duration is ~206000ms
            // seek to 3/4 of duration
            fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms\n", 120000);
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            start.iPosValue.millisec_value = 120000;
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE_AFTERSTART_TWICE:
        {
            // default clip duration is ~206000ms
            // seek to 1/2 duration (backward)
            PVPPlaybackPosition start, end;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            if (iForwardStep)
            {
                fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms\n", iForwardSeekTime);
                start.iPosValue.millisec_value = iForwardSeekTime;
                if (iForwardSeekTime < iEndTime)
                {
                    iForwardSeekTime += iForwardSeekStep;
                }
                else
                {
                    iForwardSeekTime = iBeginTime;
                }
            }
            else
            {
                fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms\n", 100000);
                start.iPosValue.millisec_value = 100000;
            }
            end.iIndeterminate = true;
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_SETPLAYBACKRANGE_AFTERDOWNLOADCOMPLETE:
        {
            PVPPlaybackPosition start, end, current;
            start.iIndeterminate = false;
            start.iPosUnit = PVPPBPOSUNIT_MILLISEC;
            start.iMode = PVPPBPOS_MODE_NOW;
            end.iIndeterminate = true;
            if (iSeekToBOC)
            {
                // download has completed, rewind back to 0 sec
                fprintf(iTestMsgOutputFile, "\n***Repositioning to 0 ms\n");
                start.iPosValue.millisec_value = 0;
            }
            else if (iSeekInCache)
            {
                current.iPosUnit = PVPPBPOSUNIT_MILLISEC;
                iPlayer->GetCurrentPositionSync(current);
                // download has completed, seek to a position in cache (the rest of the clip should be there)
                fprintf(iTestMsgOutputFile, "\n***Repositioning to %d ms", current.iPosValue.millisec_value + 5000);
                start.iPosValue.millisec_value = current.iPosValue.millisec_value + 5000;
            }
            OSCL_TRY(error, iCurrentCmdId = iPlayer->SetPlaybackRange(start, end, false, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_PAUSE:
        case STATE_PAUSE_TWICE:
        case STATE_PAUSE_AFTERDOWNLOADCOMPLETE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Pause((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESUME:
        case STATE_RESUME_TWICE:
        case STATE_RESUME_AFTERDOWNLOADCOMPLETE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Resume((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_STOP:
        case STATE_STOP_TWICE:
        {
            if (iState == STATE_STOP)
            {
                if (iSessionDuration == 0)
                    fprintf(file, "   DURATION Not Received\n");
                else
                    fprintf(file, "   GOT Duration %d \n\n", iSessionDuration);
            }
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Stop((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_VIDEO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkVideo, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASINK_AUDIO:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSink(*iDataSinkAudio, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RESET:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->Reset((OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iPlayer->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVPATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            fprintf(file, "Events:\n");
            fprintf(file, "  Num BuffStart %d\n", iNumBufferingStart);
            fprintf(file, "  Num BuffComplete %d\n", iNumBufferingComplete);
            fprintf(file, "  Num Data Ready %d\n", iNumDataReady);
            fprintf(file, "  Num Underflow %d\n", iNumUnderflow);

            PVPATB_TEST_IS_TRUE(PVPlayerFactory::DeletePlayer(iPlayer));
            iPlayer = NULL;

            delete iDataSource;
            iDataSource = NULL;

            delete iDownloadContextData;
            iDownloadContextData = NULL;

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

            //call the test-case specific cleanup.
            CleanupData();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvplayer_async_test_ppb_base::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVPATB_TEST_IS_TRUE(false);
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
        return;
    }

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

//#if(RUN_CPMJANUS_TESTCASES)

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
                iState = STATE_CONFIGPARAMS;
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
                //normal playback case, continue to add data sinks.
                iState = STATE_ADDDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                if ((iContentTooLarge) &&
                        (aResponse.GetCmdStatus() == PVMFErrContentTooLarge))
                {
                    // Test success in this case
                    fprintf(file, "   INIT returned PVMFErrContentTooLarge\n");
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

        case STATE_ADDDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_ADDDATASINK_AUDIO;
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

        case STATE_ADDDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iSeekBeforeStart)
                {
                    iState = STATE_SETPLAYBACKRANGE_BEFORESTART;
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

        case STATE_SETPLAYBACKRANGE_BEFORESTART:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_PREPARE;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }

            break;


        case STATE_PREPARE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYLIST;
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

        case STATE_PREPARE_AFTERSTOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_START_AFTERSTOP;
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

        case STATE_WAIT_FOR_DATAREADY:
        case STATE_WAIT_FOR_BUFFCOMPLETE:
            //shouldn't get here-- there are no engine commands active in this state.
            PVPATB_TEST_IS_TRUE(false);
            break;

        case STATE_GETMETADATAKEYLIST:
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
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
            if (aResponse.GetCmdStatus() == PVMFSuccess || aResponse.GetCmdStatus() == PVMFErrArgument)
            {
                //Display the resulting metadata.
                PrintMetadata();

                //wait for data ready, unless we already got it.
                if (iNumDataReady > 0)
                {
                    iState = STATE_START;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_WAIT_FOR_DATAREADY;
                    if (iPlayUntilEOS == false)
                    {
                        //5 minute error timeout
                        RunIfNotReady(5*60*1000*1000);
                    }
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

        case STATE_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iNumDataReady)
                {
                    if (iLongPauseResume || iShortPauseResume)
                    {
                        iState = STATE_PAUSE;

                        if (iShoutcastSession)
                        {
                            //just play for 1 minutes then stop
                            RunIfNotReady(iSCListenTime);
                        }
                        else
                        {
                            //just play for 20 seconds then stop
                            RunIfNotReady(20*1000*1000);
                        }
                    }
                    else if (iSeekAfterDownloadComplete && 0 == iNumBufferingComplete)
                    {
                        iState = STATE_PAUSE_AFTERDOWNLOADCOMPLETE;
                    }
                    else if (!iPlayUntilEOS)
                    {
                        iState = STATE_STOP;

                        if (iShoutcastSession)
                        {
                            //just play for 5 minutes then stop
                            // or 1 minute when it is play stop play
                            RunIfNotReady(iSCListenTime);
                        }
                        else
                        {
                            //just play for 20 second then stop
                            RunIfNotReady(20*1000*1000);
                        }
                    }
                    else
                    {
                        // wait for EOS
                        iState = STATE_STOP;
                    }
                }
                else
                {
                    //wait for data ready, then we'll play 20 seconds more,
                    //or until EOS.
                    iState = STATE_WAIT_FOR_DATAREADY;
                    if (iPlayUntilEOS == false)
                    {
                        //5 minute error timeout.
                        RunIfNotReady(5*60*1000*1000);
                    }
                    else
                    {
                        // wait for EOS
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

        case STATE_START_AFTERSTOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iNumDataReady)
                {
                    if (iEOSStopPlay)
                    {
                        iState = STATE_STOP_TWICE;
                        RunIfNotReady(3*1000*1000);	// Play for 3 seconds
                    }
                    else if (iShoutcastPlayStopPlay)
                    {
                        iState = STATE_STOP_TWICE;
                        RunIfNotReady(iSCListenTime);	// Play for another minute
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

        case STATE_SETPLAYBACKRANGE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iTestMsgOutputFile, "***Repositioning Success...\n");
                //just play for 10 seconds then stop
                iState = STATE_STOP;
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                fprintf(iTestMsgOutputFile, "***Repositioning Failed...\n");
                // Repos failed
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;


        case STATE_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iSeekAfterStart)
                {
                    iState = STATE_SETPLAYBACKRANGE_AFTERSTART;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_RESUME;
                    if (iShortPauseResume)
                    {
                        // wait for 20 second
                        // and resume
                        RunIfNotReady(20*1000*1000);
                    }
                    else if (iLongPauseResume)
                    {
                        // wait for 90 seconds
                        // and resume
                        RunIfNotReady(90*1000*1000);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE_AFTERDOWNLOADCOMPLETE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYBACKRANGE_AFTERDOWNLOADCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE_AFTERSTART:
        case STATE_SETPLAYBACKRANGE_AFTERDOWNLOADCOMPLETE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }

            break;

        case STATE_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (((iLoop > 0) || iTwice) && (iLongPauseResume || iShortPauseResume))
                {
                    iState = STATE_PAUSE_TWICE;
                    if (iStepInterval != 0)
                    {
                        RunIfNotReady(iStepInterval * 1000);
                    }
                    else
                    {
                        // play for 20 more sec
                        RunIfNotReady(20*1000*1000);
                    }
                }
                else
                {
                    iState = STATE_STOP;
                    if (iPlayUntilEOS)
                    {
                        // play until EOS
                        ;
                    }
                    else
                    {

                        if (iShoutcastSession)
                        {
                            //just play for 1 minutes then stop
                            RunIfNotReady(iSCListenTime);
                        }
                        else
                        {
                            // play for another 20 seconds and stop
                            RunIfNotReady(20*1000*1000);
                        }
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_PAUSE_TWICE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iSeekAfterStart)
                {
                    iState = STATE_SETPLAYBACKRANGE_AFTERSTART_TWICE;
                    RunIfNotReady();
                }
                else
                {
                    iState = STATE_RESUME_TWICE;
                    if (iShortPauseResume)
                    {
                        // wait for 20 second
                        // and resume
                        RunIfNotReady(20*1000*1000);
                    }
                    else if (iLongPauseResume)
                    {
                        // wait for 90 seconds
                        // and resume
                        RunIfNotReady(90*1000*1000);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_SETPLAYBACKRANGE_AFTERSTART_TWICE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESUME_TWICE;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }

            break;

        case STATE_RESUME_TWICE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(file, "\n    End of Loop %d\n", iLoopReq - iLoop);
                if (--iLoop > 0)
                {
                    iState = STATE_PAUSE;
                    if (iStepInterval != 0)
                    {
                        RunIfNotReady(iStepInterval * 1000);
                    }
                    else
                    {
                        RunIfNotReady(20*1000*1000);
                    }
                }
                else
                {
                    iState = STATE_STOP;
                    if (iPlayUntilEOS)
                    {
                        // play until EOS
                        ;
                    }
                    else
                    {
                        // play for another 20 seconds and stop
                        RunIfNotReady(20*1000*1000);
                    }
                }
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iEOSStopPlay || iShoutcastPlayStopPlay)
                {
                    iState = STATE_PREPARE_AFTERSTOP;
                }
                else
                {
                    iState = STATE_REMOVEDATASINK_VIDEO;
                }
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_STOP_TWICE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_VIDEO;
                RunIfNotReady();
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASINK_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASINK_AUDIO;
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

        case STATE_REMOVEDATASINK_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_RESET;
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
                iState = STATE_CLEANUPANDCOMPLETE;
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

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
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

void pvplayer_async_test_ppb_base::HandleSocketNodeErrors(int32 aErr)
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
    else if (aErr == PVMFSocketNodeError_TCPSocketConnect)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeError_TCPSocketConnect\n");
    }
    else if (aErr == PVMFSocketNodeError_DNSLookup)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeError_DNSLookup\n");
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
    else if (aErr == PVMFSocketNodeErrorInvalidPortConfig)
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNodeErrorInvalidPortConfig\n");
    }
    else
    {
        fprintf(iTestMsgOutputFile, "PVMFSocketNode- Unknown Error Code\n");
    }
}

void pvplayer_async_test_ppb_base::HandleProtocolEngineNodeErrors(int32 aErr)
{
    // The HTTP status errors are propagated up from the Protocol Engine node and
    // are enumerated in pvmf_protocol_engine_node_events.h. The raw value of the error
    // code can be calculated as an offset from aErr - PVProtocolEngineNodeErrorEventStart.
    if ((aErr >= PVProtocolEngineNodeErrorHTTPErrorCode400) &&
            (aErr <= PVProtocolEngineNodeErrorHTTPCode4xxUnknown))
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError4xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else if ((aErr >= PVProtocolEngineNodeErrorHTTPErrorCode500) &&
             (aErr < PVProtocolEngineNodeErrorHTTPCode5xxUnknownStart))
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError5xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else if ((aErr >= PVProtocolEngineNodeErrorHTTPRedirectCodeStart) &&
             (aErr <= PVProtocolEngineNodeErrorHTTPRedirectCodeEnd))
    {
        fprintf(file, "   ERROR IN REDIRECT: PVProtocolEngineNodeError3xx HTTP Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else if (aErr < PVProtocolEngineNodeErrorNotHTTPErrorStart || aErr > PVProtocolEngineNodeErrorNotHTTPErrorEnd)
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError HTTP Unknown Status Code %d\n",
                aErr - PVProtocolEngineNodeErrorEventStart);
    }
    else
    {
        fprintf(file, "   ERROR: PVProtocolEngineNodeError General Error %d\n", aErr);
    }
}
void pvplayer_async_test_ppb_base::PrintJanusError(const PVCmdResponse& aResp)
{
#if(RUN_CPMJANUS_TESTCASES)
//#if 0
    //Get the extended error info.
    if (aResp.GetCmdStatus() != PVMFSuccess)
    {
        PVInterface* iface = (PVInterface*)(aResp.GetEventExtensionInterface());
        if (iface)
        {
            PVUuid errUuid(PVMFErrorInfoMessageInterfaceUUID);
            PVMFErrorInfoMessageInterface* errMsg = NULL;
            if (iface->queryInterface(errUuid, (PVInterface*&)errMsg))
            {
                //search for a janus error in the error list.
                PVUuid janusUuid(PVMFJanusPluginErrorMessageUuid);
                PVMFJanusPluginErrorMessage* janusErr = NULL;
                PVMFErrorInfoMessageInterface* nextErr = errMsg->GetNextMessage();
                while (nextErr)
                {
                    if (nextErr->queryInterface(janusUuid, (PVInterface*&)janusErr))
                    {
                        uint32 drmErr = janusErr->DrmResult();
                        fprintf(file, "  Janus DRM Error! 0x%x\n", drmErr);
                        break;
                    }
                    nextErr = nextErr->GetNextMessage();
                }
            }
        }
    }
#else
    OSCL_UNUSED_ARG(aResp);
#endif
}

void pvplayer_async_test_ppb_base::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    switch (aEvent.GetEventType())
    {
        case PVMFErrResourceConfiguration:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrResourceConfiguration error event\n");
            break;

        case PVMFErrResource:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrResource error event\n");
            break;

        case PVMFErrCorrupt:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrCorrupt error event\n");
            break;

        case PVMFErrProcessing:
            // Just log the error
            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrProcessing error event\n");
            break;

        case PVMFErrTimeout:

            PVPATB_TEST_IS_TRUE(false);
            fprintf(file, "   GOT PVMFErrTimeout error event");
            break;

        default:
            // Unknown error and just log the error
            PVPATB_TEST_IS_TRUE(false);
            break;
    }

    // Wait for engine to handle the error
    Cancel();
}

void pvplayer_async_test_ppb_base::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingStart)
    {
        fprintf(file, "PVMFInfoErrorHandlingStart...\n");
    }
    if (aEvent.GetEventType() == PVMFInfoErrorHandlingComplete)
    {
        fprintf(file, "PVMFInfoErrorHandlingComplete...\n");
        iState = STATE_CLEANUPANDCOMPLETE;
        RunIfNotReady();
    }
    // Check for EOS event
    if (aEvent.GetEventType() == PVMFInfoEndOfData)
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
            if ((infouuid == PVPlayerErrorInfoEventTypesUUID) && (infocode == PVPlayerInfoEndOfClipReached))
            {
                fprintf(file, "   GOT PVPlayerInfoEndOfClipReached EVENT\n");
                iNumEOS++;
                if (iState == STATE_STOP)
                {
                    Cancel();
                    RunIfNotReady();
                }
                else if ((iState == STATE_PAUSE) && iSeekAfterStart)
                {
                    iState = STATE_SETPLAYBACKRANGE_AFTERSTART;
                    RunIfNotReady();
                }
                else if ((iState == STATE_PAUSE_TWICE) && iSeekAfterStart)
                {
                    // end of clip received before Pause() is called
                    // change the state
                    iState = STATE_SETPLAYBACKRANGE_AFTERSTART_TWICE;
                    RunIfNotReady();
                }
            }
        }
    }

    //Print some events
    switch (aEvent.GetEventType())
    {
        case PVMFInfoBufferingStatus:
        {
            int32 *percent = (int32*)aEvent.GetLocalBuffer();
            if (*percent == 0 || *percent == 100)
            {
                fprintf(file, "   PVMFInfoBufferingStatus %d\n", *percent);
            }
            else
            {
                fprintf(file, ".%d.", *percent);
            }

            // get buffer fullness
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 aBufferFullness = (uint32)(eventData);
            fprintf(file, ".(%d%%).", aBufferFullness);

        }
        break;
        case PVMFInfoBufferingStart:
            fprintf(file, "   PVMFInfoBufferingStart\n");
            iNumBufferingStart++;
            //we should only get one of these.
            if (iNumBufferingStart == 2)
            {
                if (!iEOSStopPlay && !iShoutcastPlayStopPlay)
                    PVPATB_TEST_IS_TRUE(false);
            }
            break;

        case PVMFInfoBufferingComplete:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            iNumBufferingComplete++;
            fprintf(file, "   PVMFInfoBufferingComplete (contentSize=%d), iNumBufferingComplete=%d\n", contentSize, iNumBufferingComplete);
            //we should only get one of these.
            if (iNumBufferingComplete == 2 && !iEOSStopPlay && iLoopReq == 0)
                PVPATB_TEST_IS_TRUE(false);

            if (iNumBufferingComplete == 1)
            {
                // if seek after download complete or waiting for download complete
                if ((iState == STATE_PAUSE_AFTERDOWNLOADCOMPLETE) || (iState == STATE_WAIT_FOR_BUFFCOMPLETE))
                {
                    Cancel();
                    RunIfNotReady();
                }
            }
        }
        break;

        case PVMFInfoDataReady:
            iNumDataReady++;
            fprintf(file, "\n   PVMFInfoDataReady\n");
            //special handling for very first data ready event.
            if (iNumDataReady == 1)
            {
                if (iState == STATE_WAIT_FOR_DATAREADY)
                {
                    Cancel();
                    RunIfNotReady();
                }
            }
            break;
        case PVMFInfoUnderflow:
            iNumUnderflow++;
            fprintf(file, "\n   PVMFInfoUnderflow\n");
            //we should not get underflow before data ready
            if (iNumUnderflow == 1
                    && iNumDataReady == 0)
                PVPATB_TEST_IS_TRUE(false);
            break;
        case PVMFInfoContentLength:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 contentSize = (uint32)(eventData);
            fprintf(file, "   PVMFInfoContentLength = %d\n", contentSize);
        }
        break;

        case PVMFInfoContentTruncated:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            uint32 downloadSize = (uint32)(eventData);
            fprintf(file, "\n   PVMFInfoContentTruncated! downloadSize = %d\n", downloadSize);

            // check extension info code
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface() &&
                    aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF = msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);

                        if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID &&
                                srcInfoCode == PVMFPROTOCOLENGINENODEInfo_TruncatedContentByServerDisconnect)
                        {
                            fprintf(file, "   PVMFInfoContentTruncated! TruncatedContentByServerDisconnect!\n");
                        }
                    }
                }
            }

        }
        break;

        case PVMFInfoContentType:
        {
            PVExclusivePtr eventData;
            aEvent.GetEventData(eventData);
            char *constentType = (char *)(eventData);
            fprintf(file, "   PVMFInfoContentType   = %s\n", constentType);
        }
        break;

        case PVMFInfoUnexpectedData:
            fprintf(file, "   PVMFInfoUnexpectedData! Downloaded more data than content-length\n");
            if (iNumBufferingComplete == 0) PVPATB_TEST_IS_TRUE(false); // we should get this info event after buffer complete event
            break;

        case PVMFInfoSessionDisconnect:
            fprintf(file, "   PVMFInfoSessionDisconnect! Got server disconnect after download is complete\n");
            if (iNumBufferingComplete == 0) PVPATB_TEST_IS_TRUE(false); // we should get this info event after buffer complete event
            break;

        case PVMFInfoRemoteSourceNotification:
        {
            //Examine the extended info message to see if this is the
            //"not progressive downloadable" event.  If so, notify the
            //UI.  They may choose to abort the download at this point.
            PVMFErrorInfoMessageInterface *msg = NULL;
            if (aEvent.GetEventExtensionInterface()
                    && aEvent.GetEventExtensionInterface()->queryInterface(PVMFErrorInfoMessageInterfaceUUID, (PVInterface*&)msg))
            {
                //extract the event code and event UUID.
                int32 eventcode;
                PVUuid eventUuid;
                msg->GetCodeUUID(eventcode, eventUuid);
                if (eventUuid == PVPlayerErrorInfoEventTypesUUID)
                {
                    PVMFErrorInfoMessageInterface* sourceNodeInfoIF =
                        msg->GetNextMessage();

                    if (sourceNodeInfoIF != NULL)
                    {
                        PVUuid infoUUID;
                        int32 srcInfoCode;
                        sourceNodeInfoIF->GetCodeUUID(srcInfoCode, infoUUID);
                        if (infoUUID == PVMFFileFormatEventTypesUUID
                                && srcInfoCode == PVMFMP4FFParserInfoNotPseudostreamableFile)
                        {
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (not progressive-downloadable)\n");
                        }
                        else if (infoUUID == PVMFPROTOCOLENGINENODEInfoEventTypesUUID
                                 && (srcInfoCode >= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode300 &&
                                     srcInfoCode <= PVMFPROTOCOLENGINENODEInfo_HTTPRedirectCode307))
                        {
                            PVExclusivePtr eventData;
                            aEvent.GetEventData(eventData);
                            char *redirectUrl = (char *)(eventData);
                            fprintf(file, "   PVMFInfoRemoteSourceNotification (HTTP Status code = %d), redirectUrl = %s\n",
                                    srcInfoCode - PVMFPROTOCOLENGINENODEInfo_Redirect, redirectUrl);
                        }

                    }
                }
            }
        }
        break;
        case PVMFInfoDurationAvailable:
        {
            PVMFDurationInfoMessageInterface* eventMsg = NULL;
            PVInterface* infoExtInterface = aEvent.GetEventExtensionInterface();

            if (infoExtInterface &&
                    infoExtInterface->queryInterface(PVMFDurationInfoMessageInterfaceUUID, (PVInterface*&)eventMsg))
            {
                PVUuid eventuuid;
                int32 infoCode;
                eventMsg->GetCodeUUID(infoCode, eventuuid);
                if (eventuuid == PVMFDurationInfoMessageInterfaceUUID)
                {
                    iSessionDuration = eventMsg->GetDuration();
                }
            }
        }
        break;
        case PVMFInfoMetadataAvailable:
        {
            PVUuid infomsguuid = PVMFMetadataInfoMessageInterfaceUUID;
            PVMFMetadataInfoMessageInterface* eventMsg = NULL;
            PVInterface* infoExtInterface = aEvent.GetEventExtensionInterface();
            if (infoExtInterface &&
                    infoExtInterface->queryInterface(infomsguuid, (PVInterface*&)eventMsg))
            {
                PVUuid eventuuid;
                int32 infoCode;
                eventMsg->GetCodeUUID(infoCode, eventuuid);
                if (eventuuid == infomsguuid)
                {
                    Oscl_Vector<PvmiKvp, OsclMemAllocator> kvpVector = eventMsg->GetMetadataVector();
                    for (uint32 i = 0;i < kvpVector.size();i++)
                    {
                        //arora
                        if (oscl_strstr(kvpVector[i].key, "valtype=char*"))
                        {
                            fprintf(file, "\n*********************************");
                            fprintf(file, "\nMetadata Key '%s', value '%s'\n", kvpVector[i].key, kvpVector[i].value.pChar_value);
                            fprintf(file, "\n*********************************");
                        }
                        else if (oscl_strstr(kvpVector[i].key, "valtype=wchar*"))
                        {
                            OSCL_HeapString<OsclMemAllocator> ostr;
                            char buf[2];
                            buf[1] = '\0';
                            for (uint32 j = 0;;j++)
                            {
                                if (kvpVector[i].value.pWChar_value[j] == '\0')
                                    break;
                                buf[0] = kvpVector[i].value.pWChar_value[j];
                                ostr += buf;
                            }
                            fprintf(file, "\n*********************************");
                            fprintf(file, "  Metadata Key '%s', value '%s'\n", kvpVector[i].key, ostr.get_str());
                            fprintf(file, "\n*********************************");
                        }
                    }
                }
            }
        }

        default:
            break;
    }
}


//
// pvplayer_async_test_ppb_normal
//
pvplayer_async_test_ppb_normal::~pvplayer_async_test_ppb_normal()
{
    iDownloadContextData = NULL;
}

void pvplayer_async_test_ppb_normal::CreateDataSource()
{
    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;

    if (iFileType == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
    {
        iShoutcastSession = true;
    }

    if (url == default_source)
    {
        if (iFileType == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
        {
            fprintf(file, "Setting source to %s\n", DEFAULT_SHOUTCAST_URL);
            url = DEFAULT_SHOUTCAST_URL;
        }
        else
        {
            fprintf(file, "Setting source to %s\n", DEFAULT_3GPP_PPB_URL);
            url = DEFAULT_3GPP_PPB_URL;
        }
    }

    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0; i < url.get_size(); i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);

    if (iFileType == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL);
    }
    else
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }

    iDownloadFilename = NULL;

    iDownloadProxy = _STRLIT_CHAR("");
    int32 iDownloadProxyPort = 0;

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gp_ppb_normal.cfg");

    iContentTooLarge = false;

    uint32 iMaxFileSize = 0x7FFFFFFF;

    bool aIsNewSession = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::ENoSaveToFile;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}

void pvplayer_async_test_ppb_normal::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gp_ppb_");

    if (iShortPauseResume)
    {
        sinkfile += _STRLIT_WCHAR("sht_ps_rsm_");
    }
    else if (iLongPauseResume)
    {
        sinkfile += _STRLIT_WCHAR("lng_ps_rsm_");
    }

    if (iSeekAfterStart)
    {
        sinkfile += _STRLIT_WCHAR("sk_aft_strt_");
    }
    else if (iSeekBeforeStart)
    {
        sinkfile += _STRLIT_WCHAR("sk_b4_strt_");
    }

    if (iForwardStep)
    {
        sinkfile += _STRLIT_WCHAR("fstp_");
    }

    if (iLoop)
    {
        sinkfile += _STRLIT_WCHAR("lp_");
    }

    if (iPlayUntilEOS)
    {
        sinkfile += _STRLIT_WCHAR("utl_eos_video.dat");
    }
    else
    {
        sinkfile += _STRLIT_WCHAR("nml_video.dat");
    }

    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}

void pvplayer_async_test_ppb_normal::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    if (iShoutcastSession)
    {
        sinkfile += _STRLIT_WCHAR("test_player_shout_");
    }
    else
    {
        sinkfile += _STRLIT_WCHAR("test_player_3gp_ppb_");
    }
    if (iShortPauseResume)
    {
        sinkfile += _STRLIT_WCHAR("sht_ps_rsm_");
    }
    else if (iLongPauseResume)
    {
        sinkfile += _STRLIT_WCHAR("lng_ps_rsm_");
    }

    if (iSeekAfterStart)
    {
        sinkfile += _STRLIT_WCHAR("sk_aft_strt_");
    }
    else if (iSeekBeforeStart)
    {
        sinkfile += _STRLIT_WCHAR("sk_b4_strt_");
    }

    if (iForwardStep)
    {
        sinkfile += _STRLIT_WCHAR("fstp_");
    }

    if (iLoop)
    {
        sinkfile += _STRLIT_WCHAR("lp_");
    }

    if (iShoutcastPlayStopPlay)
    {
        sinkfile += _STRLIT_WCHAR("ply_stp_ply_");
    }

    if (iPlayUntilEOS)
    {
        sinkfile += _STRLIT_WCHAR("utl_eos_audio.dat");
    }
    else
    {
        sinkfile += _STRLIT_WCHAR("nml_audio.dat");
    }

    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}


//
// pvplayer_async_test_PDLPauseResumeAfterUnderFlow section
//
pvplayer_async_test_PDLPauseResumeAfterUnderFlow::~pvplayer_async_test_PDLPauseResumeAfterUnderFlow()
{
}

void pvplayer_async_test_PDLPauseResumeAfterUnderFlow::CreateDataSource()
{

    OSCL_HeapString<OsclMemAllocator> url(iFileName);
    OSCL_HeapString<OsclMemAllocator> default_source(SOURCENAME_PREPEND_STRING);
    default_source += DEFAULTSOURCEFILENAME;
    if (url == default_source)
    {
        fprintf(file, "Setting source to %s\n", DEFAULT_3GPPDL_URL);
        url = DEFAULT_3GPPDL_URL;
    }
    oscl_wchar wbuf[2];
    wbuf[1] = '\0';
    for (uint32 i = 0;i < url.get_size();i++)
    {
        wbuf[0] = (oscl_wchar)url.get_cstr()[i];
        iDownloadURL += wbuf;
    }
    iDataSource = new PVPlayerDataSourceURL;
    iDataSource->SetDataSourceURL(iDownloadURL);

    if (iProtocolRollOver == true)
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
        iDataSource->SetAlternateSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }
    else
    {
        iDataSource->SetDataSourceFormatType(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    }

    iDownloadFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadFilename += _STRLIT_WCHAR("test_3gppdownload.loc");

    int32 iDownloadProxyPort;
#if PVPLAYER_TEST_ENABLE_PROXY
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 7070;
#else
    iDownloadProxy = _STRLIT_CHAR("");
    iDownloadProxyPort = 0;
#endif

    iDownloadConfigFilename = OUTPUTNAME_PREPEND_WSTRING;
    iDownloadConfigFilename += _STRLIT_WCHAR("my3gppdl.cfg");

    iContentTooLarge = false;
    uint32 iMaxFileSize = 0x7FFFFFFF;
    bool aIsNewSession = true;
    iPauseResumeAfterUnderflow = true;

    iDownloadContextData = new PVMFSourceContextData();
    iDownloadContextData->EnableCommonSourceContext();
    iDownloadContextData->EnableDownloadHTTPSourceContext();
    iDownloadContextData->DownloadHTTPData()->bIsNewSession = aIsNewSession;
    iDownloadContextData->DownloadHTTPData()->iConfigFileName = iDownloadConfigFilename;
    iDownloadContextData->DownloadHTTPData()->iDownloadFileName = iDownloadFilename;
    iDownloadContextData->DownloadHTTPData()->iMaxFileSize = iMaxFileSize;
    iDownloadContextData->DownloadHTTPData()->iProxyName = iDownloadProxy;
    iDownloadContextData->DownloadHTTPData()->iProxyPort = iDownloadProxyPort;
    iDownloadContextData->DownloadHTTPData()->iUserID = _STRLIT_CHAR("abc");
    iDownloadContextData->DownloadHTTPData()->iUserPasswd = _STRLIT_CHAR("xyz");
    iDownloadContextData->DownloadHTTPData()->iPlaybackControl = PVMFSourceContextDataDownloadHTTP::EAsap;

    iDataSource->SetDataSourceContextData(iDownloadContextData);
}
void pvplayer_async_test_PDLPauseResumeAfterUnderFlow::CreateDataSinkVideo()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_video.dat");
    iMIOFileOutVideo = iMioFactory->CreateVideoOutput((OsclAny*) & sinkfile, MEDIATYPE_VIDEO, iCompressedVideo);
    iIONodeVideo = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutVideo);
    iDataSinkVideo = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkVideo)->SetDataSinkNode(iIONodeVideo);
}
void pvplayer_async_test_PDLPauseResumeAfterUnderFlow::CreateDataSinkAudio()
{
    OSCL_wHeapString<OsclMemAllocator> sinkfile = OUTPUTNAME_PREPEND_WSTRING;
    sinkfile += _STRLIT_WCHAR("test_player_3gppdlnormal_audio.dat");
    iMIOFileOutAudio = iMioFactory->CreateAudioOutput((OsclAny*) & sinkfile, MEDIATYPE_AUDIO, iCompressedAudio);
    iIONodeAudio = PVMediaOutputNodeFactory::CreateMediaOutputNode(iMIOFileOutAudio);
    iDataSinkAudio = new PVPlayerDataSinkPVMFNode;
    ((PVPlayerDataSinkPVMFNode*)iDataSinkAudio)->SetDataSinkNode(iIONodeAudio);
}
void pvplayer_async_test_PDLPauseResumeAfterUnderFlow::CleanupData()
{
}
