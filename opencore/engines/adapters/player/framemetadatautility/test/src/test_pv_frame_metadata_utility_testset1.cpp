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
#include "test_pv_frame_metadata_utility_testset1.h"

#include "oscl_error_codes.h"

#include "oscl_tickcount.h"

#include "pvmi_kvp.h"

#include "pvmi_kvp_util.h"

#include "pvmf_errorinfomessage_extension.h"

//
// pvframemetadata_async_test_newdelete section
//
void pvframemetadata_async_test_newdelete::StartTest()
{
    AddToScheduler();
    RunIfNotReady();
}


void pvframemetadata_async_test_newdelete::Run()
{
    PVFrameAndMetadataInterface *temp = NULL;
    int error = 0;

    OSCL_TRY(error, temp = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
    if (error)
    {
        PVFMUATB_TEST_IS_TRUE(false);
    }
    else
    {
        PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(temp));
        temp = NULL;

        PVFMUATB_TEST_IS_TRUE(true);
    }

    iObserver->TestCompleted(*iTestCase);
}


void pvframemetadata_async_test_newdelete::CommandCompleted(const PVCmdResponse& /*aResponse*/)
{
    // No callbacks in this test.
}


void pvframemetadata_async_test_newdelete::HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/)
{
    // No callbacks in this test
}


void pvframemetadata_async_test_newdelete::HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/)
{
    // No callbacks in this test
}

//
// pvframemetadata_async_test_getmetadata section
//
void pvframemetadata_async_test_getmetadata::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;

    // Retrieve the logger object
    iLogger = PVLogger::GetLoggerObject("PVFrameAndMetadataUtilityTest");
    iPerfLogger = PVLogger::GetLoggerObject("fmutestdiagnostics");

    RunIfNotReady();
}


void pvframemetadata_async_test_getmetadata::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::Create Called Tick=%d", OsclTickCount::TickCount()));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                if (iMode == 1)
                {
                    uint32 mode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY;
                    iFrameMetadataUtil->SetMode(mode);
                }
                else if (iMode == 2)
                {
                    uint32 mode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL;
                    iFrameMetadataUtil->SetMode(mode);
                }
                iState = STATE_ADDDATASOURCE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            if ((iMode == 1) || (iMode == 2))
            {
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getsourcemetadata_");
            }
            else
            {
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getallmetadata_");
            }
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            if (iMode == 2)
            {
                outputfilename = OUTPUTNAME_PREPEND_WSTRING;
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getmetadata_");
                outputfilename += inputfilename;
                outputfilename += _STRLIT_WCHAR("_frame_");
                outputfilename += iOutputFrameTypeWString;
                outputfilename += _STRLIT_WCHAR(".dat");

                if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
                {
                    PVFMUATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                    break;
                }
            }

            if (iBestThumbNailMode)
            {
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                //set the intent to thumbnails
                iSourceContextData->CommonData()->iIntent = BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS;
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }

            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::AddDataSource Issued Tick=%d", OsclTickCount::TickCount()));

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS1:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::GetMetadataKeys Issued Tick=%d", OsclTickCount::TickCount()));

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::GetMetadataValues Issued Tick=%d", OsclTickCount::TickCount()));

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::GetFrame Issued Tick=%d", OsclTickCount::TickCount()));

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::RemoveDataSource Issued Tick=%d", OsclTickCount::TickCount()));

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                            (0, "PVFrameAndMetadataUtilityTest::CleanUpAndComplete Called Tick=%d", OsclTickCount::TickCount()));

            // Close the output file handles
            iMetadataFile.Close();
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_getmetadata::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::AddDataSource completed sucessfully Tick=%d", OsclTickCount::TickCount()));

                iState = STATE_GETMETADATAKEYS1;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::AddDataSource failed Tick=%d", OsclTickCount::TickCount()));

                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetMetaDataKeys completed sucessfully Tick=%d", OsclTickCount::TickCount()));
                iState = STATE_GETMETADATAVALUES1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetMetadataKeys failed Tick=%d", OsclTickCount::TickCount()));
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetMetaDataValues completed sucessfully Tick=%d", OsclTickCount::TickCount()));
                oscl_snprintf(iTextOutputBuf, 512, "After AddDataSource():\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                if (iMode == 2)
                {
                    iState = STATE_GETFRAME;
                }
                else
                {
                    iState = STATE_REMOVEDATASOURCE;
                }
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetMetadataValues failed Tick=%d", OsclTickCount::TickCount()));
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetFrame completed sucessfully Tick=%d", OsclTickCount::TickCount()));
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::GetFrame failed Tick=%d", OsclTickCount::TickCount()));
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::RemoveDataSource completed sucessfully Tick=%d", OsclTickCount::TickCount()));
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iPerfLogger, PVLOGMSG_NOTICE,
                                (0, "PVFrameAndMetadataUtilityTest::RemoveDataSource failed Tick=%d", OsclTickCount::TickCount()));
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_getmetadata::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getmetadata::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getmetadata::SaveMetadataInfo()
{
    uint32 i = 0;

    oscl_snprintf(iTextOutputBuf, 512, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    }

    oscl_snprintf(iTextOutputBuf, 512, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();

    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Value %d:\n   Key string: %s\n", (i + 1), iMetadataValueList[i].key);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = NULL;
                oscl_snprintf(iTextOutputBuf, 512, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_INT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_UINT8:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:true(1)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                }
                else
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:false(0)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

                }
                break;

            default:
                oscl_snprintf(iTextOutputBuf, 512, "   Value: UNKNOWN VALUE TYPE\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;
        }

        oscl_snprintf(iTextOutputBuf, 512, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
        iMetadataFile.Flush();
    }

    oscl_snprintf(iTextOutputBuf, 512, "\n\n");
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();
}

void pvframemetadata_async_test_getmetadata::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}

//
// pvframemetadata_async_test_getfirstframemetadata section
//
void pvframemetadata_async_test_getfirstframemetadata::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_getfirstframemetadata::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_getfirstframemetadata_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_getfirstframemetadata_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS1:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYS2:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES2:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iMetadataFile.Close();
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_getfirstframemetadata::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETMETADATAKEYS1;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "After AddDataSource():\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_GETFRAME;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_GETMETADATAKEYS2;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES2;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "After GetFrame():\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_getfirstframemetadata::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getfirstframemetadata::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getfirstframemetadata::SaveMetadataInfo()
{
    uint32 i = 0;

    oscl_snprintf(iTextOutputBuf, 512, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    }

    oscl_snprintf(iTextOutputBuf, 512, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();

    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Value %d:\n   Key string: %s\n", (i + 1), iMetadataValueList[i].key);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = NULL;
                oscl_snprintf(iTextOutputBuf, 512, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_INT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_UINT8:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:true(1)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                }
                else
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:false(0)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

                }
                break;

            default:
                oscl_snprintf(iTextOutputBuf, 512, "   Value: UNKNOWN VALUE TYPE\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;
        }

        oscl_snprintf(iTextOutputBuf, 512, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
        iMetadataFile.Flush();
    }

    oscl_snprintf(iTextOutputBuf, 512, "\n\n");
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();
}


void pvframemetadata_async_test_getfirstframemetadata::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_getfirstframeutilitybuffer section
//
void pvframemetadata_async_test_getfirstframeutilitybuffer::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_getfirstframeutilitybuffer::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_getfirstframeutilitybuffer_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETFRAME:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBuffer = NULL;
            iFrameBufferSize = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, &iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_RETURNBUFFER:
        {
            // Return the frame buffer to the utility
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->ReturnBuffer(iFrameBuffer, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_getfirstframeutilitybuffer::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETFRAME;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess &&
                    iFrameBuffer != NULL && iFrameBufferSize > 0)
            {
                SaveVideoFrame();
                iState = STATE_RETURNBUFFER;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_RETURNBUFFER:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // ReturnBuffer failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_getfirstframeutilitybuffer::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getfirstframeutilitybuffer::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_getfirstframeutilitybuffer::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_get30thframe section
//
void pvframemetadata_async_test_get30thframe::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_get30thframe::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_get30thframe_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETFRAME:
        {
            // Retrieve the 30th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 30;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_get30thframe::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETFRAME;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_get30thframe::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_get30thframe::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_get30thframe::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_get10secframe section
//
void pvframemetadata_async_test_get10secframe::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_get10secframe::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_get10secframe_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETFRAME:
        {
            // Retrieve the frame at 10sec
            iFrameSelector.iSelectionMethod = PVFrameSelector::TIMESTAMP;
            iFrameSelector.iFrameInfo.iTimeOffsetMilliSec = 10000;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_get10secframe::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETFRAME;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_get10secframe::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_get10secframe::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_get10secframe::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_cancelcommand section
//
void pvframemetadata_async_test_cancelcommand::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_cancelcommand::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE_QUEUED_CMDS;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE_QUEUED_CMDS:
        {
            iPendingCmds = 0;

            // Add a data source
            iDataSource = new PVPlayerDataSourceURL;
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            ++iPendingCmds;

            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            ++iPendingCmds;

            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = 10;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            ++iPendingCmds;
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_cancelcommand::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetContext() != NULL)
    {
        if (aResponse.GetContext() == (OsclAny*)&iContextObject)
        {
            if (iContextObject != iContextObjectRefValue)
            {
                // Context data value was corrupted
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE_QUEUED_CMDS:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                --iPendingCmds;
                // Cancel remaining pending commands
                int32 error = 0;
                OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->CancelAllCommands((OsclAny*) & iContextObject));
                OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); break;);
                iState = STATE_CANCELALLCOMMANDS;
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_CANCELALLCOMMANDS:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(iPendingCmds == 0);
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
            {
                // Queued command was cancelled
                --iPendingCmds;
            }
            else
            {
                // CancelAllCommands failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_cancelcommand::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_cancelcommand::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


//
// pvframemetadata_async_test_multigetfirstframemetadata section
//
void pvframemetadata_async_test_multigetfirstframemetadata::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_multigetfirstframemetadata::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                iState = STATE_ADDDATASOURCE1;
                RunIfNotReady();
            }
        }
        break;

        case STATE_ADDDATASOURCE1:
        {
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_multigetfirstframemetadata_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_multigetfirstframemetadata_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS1:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME1:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE1:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE2:
        {
            // Add a different data source
            OSCL_wHeapString<OsclMemAllocator> secondclip(_STRLIT_WCHAR("test.mp4"));
            iDataSource->SetDataSourceURL(secondclip);
            iDataSource->SetDataSourceFormatType(PVMF_MIME_MPEG4FF);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS2:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES2:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME2:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE2:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE3:
        {
            // Add the first data source again
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYS3:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES3:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME3:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE3:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iMetadataFile.Close();
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_multigetfirstframemetadata::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            return;
        }
    }

    switch (iState)
    {
        case STATE_ADDDATASOURCE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS1;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "METADATA FOR CLIP 1\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_GETFRAME1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE1;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_ADDDATASOURCE2;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS2;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES2;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "METADATA FOR CLIP 2\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_GETFRAME2;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE2;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_ADDDATASOURCE3;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAKEYS3;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES3;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "METADATA FOR CLIP 3\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_GETFRAME3;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE3;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_multigetfirstframemetadata::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_multigetfirstframemetadata::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_multigetfirstframemetadata::SaveMetadataInfo()
{
    uint32 i = 0;

    oscl_snprintf(iTextOutputBuf, 512, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    }

    oscl_snprintf(iTextOutputBuf, 512, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();

    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Value %d:\n   Key string: %s\n", (i + 1), iMetadataValueList[i].key);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = NULL;
                oscl_snprintf(iTextOutputBuf, 512, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_INT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_UINT8:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:true(1)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                }
                else
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:false(0)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

                }
                break;

            default:
                oscl_snprintf(iTextOutputBuf, 512, "   Value: UNKNOWN VALUE TYPE\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;
        }

        oscl_snprintf(iTextOutputBuf, 512, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
        iMetadataFile.Flush();
    }

    oscl_snprintf(iTextOutputBuf, 512, "\n\n");
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();
}


void pvframemetadata_async_test_multigetfirstframemetadata::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_multigetframe section
//
void pvframemetadata_async_test_multigetframe::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_multigetframe::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_multigetframe_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_frame_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".dat");

            if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETFRAME1:
        {
            // Retrieve the 0th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME2:
        {
            // Retrieve the 50th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 50;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME3:
        {
            // Retrieve the 30th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 30;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME4:
        {
            // Retrieve the 60th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 60;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME5:
        {
            // Retrieve the 0th frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_multigetframe::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETFRAME1;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_GETFRAME2;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_GETFRAME3;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME3:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_GETFRAME4;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME4:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_GETFRAME5;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME5:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_multigetframe::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_multigetframe::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_multigetframe::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_invalidsourcefile section
//
void pvframemetadata_async_test_invalidsourcefile::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_invalidsourcefile::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            wFileName = _STRLIT_WCHAR("a1b2c3d4.mp4");
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_invalidsourcefile::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                // AddDataSource() should fail since source file is invalid
                PVFMUATB_TEST_IS_TRUE(false);
            }

            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_invalidsourcefile::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_invalidsourcefile::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


//
// pvframemetadata_async_test_nogetframe section
//
void pvframemetadata_async_test_nogetframe::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_nogetframe::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_nogetframe_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iMetadataFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_nogetframe::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETMETADATAKEYS;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                SaveMetadataInfo();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_nogetframe::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_nogetframe::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_nogetframe::SaveMetadataInfo()
{
    uint32 i = 0;

    oscl_snprintf(iTextOutputBuf, 512, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    }

    oscl_snprintf(iTextOutputBuf, 512, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();

    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Value %d:\n   Key string: %s\n", (i + 1), iMetadataValueList[i].key);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = NULL;
                oscl_snprintf(iTextOutputBuf, 512, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_INT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_UINT8:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:true(1)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                }
                else
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:false(0)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

                }
                break;

            default:
                oscl_snprintf(iTextOutputBuf, 512, "   Value: UNKNOWN VALUE TYPE\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;
        }

        oscl_snprintf(iTextOutputBuf, 512, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
        iMetadataFile.Flush();
    }

    oscl_snprintf(iTextOutputBuf, 512, "\n\n");
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();
}


//
// pvframemetadata_async_test_novideotrack section
//
void pvframemetadata_async_test_novideotrack::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_novideotrack::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
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
            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            outputfilename += _STRLIT_WCHAR("test_framemetadata_novideotrack_");
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;
            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_GETMETADATAKEYS1:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES1:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETFRAME:
        {
            // Retrieve the first frame
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 0;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAKEYS2:
        {
            // Retrieve all the available metadata keys
            iMetadataKeyList.clear();
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataKeys(iMetadataKeyList, 0, 50, NULL, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_GETMETADATAVALUES2:
        {
            // Retrieve the values
            iMetadataValueList.clear();
            iNumValues = 0;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetMetadataValues(iMetadataKeyList, 0, 50, iNumValues, iMetadataValueList, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iMetadataFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_novideotrack::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                iState = STATE_GETMETADATAKEYS1;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAKEYS1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES1;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES1:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "After AddDataSource():\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_GETFRAME;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(false);
            }

            iState = STATE_GETMETADATAKEYS2;
            RunIfNotReady();
            break;

        case STATE_GETMETADATAKEYS2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_GETMETADATAVALUES2;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataKeys failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETMETADATAVALUES2:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                oscl_snprintf(iTextOutputBuf, 512, "After GetFrame():\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                SaveMetadataInfo();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetMetadataValues failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_novideotrack::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_novideotrack::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_novideotrack::SaveMetadataInfo()
{
    uint32 i = 0;

    oscl_snprintf(iTextOutputBuf, 512, "Metadata key list (count=%d):\n", iMetadataKeyList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

    for (i = 0; i < iMetadataKeyList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Key %d: %s\n", (i + 1), iMetadataKeyList[i].get_cstr());
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    }

    oscl_snprintf(iTextOutputBuf, 512, "\nMetadata value list (count=%d):\n", iMetadataValueList.size());
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();

    for (i = 0; i < iMetadataValueList.size(); ++i)
    {
        oscl_snprintf(iTextOutputBuf, 512, "Value %d:\n   Key string: %s\n", (i + 1), iMetadataValueList[i].key);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

        switch (GetValTypeFromKeyString(iMetadataValueList[i].key))
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%s\n", iMetadataValueList[i].value.pChar_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_WCHARPTR:
            {
                // Assume string is in UCS-2 encoding so convert to UTF-8
                char tmpstr[65];
                oscl_UnicodeToUTF8(iMetadataValueList[i].value.pWChar_value,
                                   oscl_strlen(iMetadataValueList[i].value.pWChar_value), tmpstr, 65);
                tmpstr[64] = NULL;
                oscl_snprintf(iTextOutputBuf, 512, "   Value(in UTF-8, first 64 chars):%s\n", tmpstr);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
            }
            break;

            case PVMI_KVPVALTYPE_UINT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_INT32:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.int32_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_UINT8:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%d\n", iMetadataValueList[i].value.uint8_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_FLOAT:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.float_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_DOUBLE:
                oscl_snprintf(iTextOutputBuf, 512, "   Value:%f\n", iMetadataValueList[i].value.double_value);
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;

            case PVMI_KVPVALTYPE_BOOL:
                if (iMetadataValueList[i].value.bool_value)
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:true(1)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                }
                else
                {
                    oscl_snprintf(iTextOutputBuf, 512, "   Value:false(0)\n");
                    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));

                }
                break;

            default:
                oscl_snprintf(iTextOutputBuf, 512, "   Value: UNKNOWN VALUE TYPE\n");
                iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
                break;
        }

        oscl_snprintf(iTextOutputBuf, 512, "   Length:%d  Capacity:%d\n", iMetadataValueList[i].length, iMetadataValueList[i].capacity);
        iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
        iMetadataFile.Flush();
    }

    oscl_snprintf(iTextOutputBuf, 512, "\n\n");
    iMetadataFile.Write(iTextOutputBuf, sizeof(char), oscl_strlen(iTextOutputBuf));
    iMetadataFile.Flush();
}



//
// pvframemetadata_async_test_settimeout_getframe section
//
void pvframemetadata_async_test_settimeout_getframe::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_settimeout_getframe::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                if (iMode == 1)
                {
                    uint32 mode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY;
                    iFrameMetadataUtil->SetMode(mode);
                }
                else if (iMode == 2)
                {
                    uint32 mode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL;
                    iFrameMetadataUtil->SetMode(mode);
                }
                iState = STATE_QUERYINTERFACE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_QUERYINTERFACE:
        {
            PVUuid capconfigifuuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->QueryInterface(capconfigifuuid, (PVInterface*&)iFMUCapConfigIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;

            OSCL_wHeapString<OsclMemAllocator> outputfilename;
            OSCL_wHeapString<OsclMemAllocator> inputfilename;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));
            RetrieveFilename(wFileName.get_str(), inputfilename);

            // Open file to log the metadata and to save the video frame
            iFS.Connect();

            outputfilename = OUTPUTNAME_PREPEND_WSTRING;
            if ((iMode == 1) || (iMode == 2))
            {
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getsourcemetadata_");
            }
            else
            {
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getallmetadata_");
            }
            outputfilename += inputfilename;
            outputfilename += _STRLIT_WCHAR("_metadata_");
            outputfilename += iOutputFrameTypeWString;
            outputfilename += _STRLIT_WCHAR(".txt");

            if (iMetadataFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT, iFS))
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                break;
            }

            if (iMode == 2)
            {
                outputfilename = OUTPUTNAME_PREPEND_WSTRING;
                outputfilename += _STRLIT_WCHAR("test_framemetadata_getmetadata_");
                outputfilename += inputfilename;
                outputfilename += _STRLIT_WCHAR("_frame_");
                outputfilename += iOutputFrameTypeWString;
                outputfilename += _STRLIT_WCHAR(".dat");

                if (iFrameFile.Open(outputfilename.get_str(), Oscl_File::MODE_READWRITE, iFS))
                {
                    PVFMUATB_TEST_IS_TRUE(false);
                    iState = STATE_CLEANUPANDCOMPLETE;
                    RunIfNotReady();
                    break;
                }
            }

            if (iBestThumbNailMode)
            {
                iSourceContextData = new PVMFSourceContextData();
                iSourceContextData->EnableCommonSourceContext();
                //set the intent to thumbnails
                iSourceContextData->CommonData()->iIntent = BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS;
                iDataSource->SetDataSourceContextData((OsclAny*)iSourceContextData);
            }

            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_SETFRAMERETRIEVALTIMEOUT:
        {
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/fmu/timeout-frameretrieval-in-seconds;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            //Set a timeout value of 1 sec
            iKVPSetAsync.value.uint32_value = 1;
            iErrorKVP = NULL;
            OSCL_TRY(error, iFMUCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iState = STATE_GETFRAME;
            RunIfNotReady();
        }
        break;

        case STATE_GETFRAME:
        {
            // Try to retrieve a far-off frame to trigger timeout
            iFrameSelector.iSelectionMethod = PVFrameSelector::SPECIFIC_FRAME;
            iFrameSelector.iFrameInfo.iFrameIndex = 30000;
            iFrameBufferSize = MAX_VIDEO_FRAME_SIZE;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->GetFrame(iFrameSelector, iFrameBuffer, iFrameBufferSize, iFrameBufferProp, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iSourceContextData;
            iSourceContextData = NULL;

            delete iDataSource;
            iDataSource = NULL;

            // Close the output file handles
            iMetadataFile.Close();
            iFrameFile.Close();
            iFS.Close();

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_settimeout_getframe::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETFRAMERETRIEVALTIMEOUT;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_GETFRAME:
            // Expected result is PVMFErrTimeout
            if (aResponse.GetCmdStatus() == PVMFErrTimeout)
            {
                SaveVideoFrame();
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else if (aResponse.GetCmdStatus() == PVMFErrMaxReached)
            {
                // Test could not be excercised fully since we recvd EOS before timeout error
                fprintf(iTestMsgOutputFile, "\nTest could not be excercised fully since we recvd EOS before timeout error\n\n");
                iState = STATE_REMOVEDATASOURCE;
                RunIfNotReady();
            }
            else
            {
                // GetFrame failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_settimeout_getframe::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_settimeout_getframe::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_settimeout_getframe::SaveMetadataInfo()
{

}

void pvframemetadata_async_test_settimeout_getframe::SaveVideoFrame()
{
    // Write the retrieved video frame data to file
    iFrameFile.Write(iFrameBuffer, 1, iFrameBufferSize);
    iFrameFile.Flush();
}


//
// pvframemetadata_async_test_set_player_key section
//
void pvframemetadata_async_test_set_player_key::StartTest()
{
    AddToScheduler();
    iState = STATE_CREATE;
    RunIfNotReady();
}


void pvframemetadata_async_test_set_player_key::Run()
{
    int error = 0;

    switch (iState)
    {
        case STATE_CREATE:
        {
            iFrameMetadataUtil = NULL;

            OSCL_TRY(error, iFrameMetadataUtil = PVFrameAndMetadataFactory::CreateFrameAndMetadataUtility(iOutputFrameTypeString.get_str(), this, this, this));
            if (error)
            {
                PVFMUATB_TEST_IS_TRUE(false);
                iObserver->TestCompleted(*iTestCase);
            }
            else
            {
                uint32 mode = PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY;
                iFrameMetadataUtil->SetMode(mode);

                iState = STATE_QUERYINTERFACE;
                RunIfNotReady();
            }
        }
        break;

        case STATE_QUERYINTERFACE:
        {
            PVUuid capconfigifuuid = PVMI_CAPABILITY_AND_CONFIG_PVUUID;
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->QueryInterface(capconfigifuuid, (PVInterface*&)iFMUCapConfigIF, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_ADDDATASOURCE:
        {
            // Create a player data source and add it
            iDataSource = new PVPlayerDataSourceURL;

            // Convert the source file name to UCS2 and extract the filename part
            oscl_UTF8ToUnicode(iFileName, oscl_strlen(iFileName), iTempWCharBuf, 512);
            wFileName.set(iTempWCharBuf, oscl_strlen(iTempWCharBuf));

            iDataSource->SetDataSourceURL(wFileName);
            iDataSource->SetDataSourceFormatType(iFileType);

            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->AddDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }

        break;

        case STATE_SETPLAYERKEY:
        {
            // Any key understood by the player
            iKeyStringSetAsync = _STRLIT_CHAR("x-pvmf/player/nodecmd_timeout;valtype=uint32");
            iKVPSetAsync.key = iKeyStringSetAsync.get_str();
            iKVPSetAsync.value.uint32_value = 4421;
            iErrorKVP = NULL;
            OSCL_TRY(error, iFMUCapConfigIF->setParametersSync(NULL, &iKVPSetAsync, 1, iErrorKVP));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady(); return);

            iState = STATE_REMOVEDATASOURCE;
            RunIfNotReady();
        }
        break;

        case STATE_REMOVEDATASOURCE:
        {
            OSCL_TRY(error, iCurrentCmdId = iFrameMetadataUtil->RemoveDataSource(*iDataSource, (OsclAny*) & iContextObject));
            OSCL_FIRST_CATCH_ANY(error, PVFMUATB_TEST_IS_TRUE(false); iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
        }
        break;

        case STATE_CLEANUPANDCOMPLETE:
        {
            PVFMUATB_TEST_IS_TRUE(PVFrameAndMetadataFactory::DeleteFrameAndMetadataUtility(iFrameMetadataUtil));
            iFrameMetadataUtil = NULL;

            delete iDataSource;
            iDataSource = NULL;

            iObserver->TestCompleted(*iTestCase);
        }
        break;

        default:
            break;

    }
}


void pvframemetadata_async_test_set_player_key::CommandCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdId() != iCurrentCmdId)
    {
        // Wrong command ID.
        PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
                return;
            }
        }
        else
        {
            // Context data pointer was corrupted
            PVFMUATB_TEST_IS_TRUE(false);
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
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_ADDDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = STATE_SETPLAYERKEY;
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        case STATE_REMOVEDATASOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVFMUATB_TEST_IS_TRUE(true);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else
            {
                // RemoveDataSource failed
                PVFMUATB_TEST_IS_TRUE(false);
                iState = STATE_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            break;

        default:
        {
            // Testing error if this is reached
            PVFMUATB_TEST_IS_TRUE(false);
            iState = STATE_CLEANUPANDCOMPLETE;
            RunIfNotReady();
        }
        break;
    }
}


void pvframemetadata_async_test_set_player_key::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}


void pvframemetadata_async_test_set_player_key::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}








