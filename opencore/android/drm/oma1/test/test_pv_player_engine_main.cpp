/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_pv_player_engine_main.h"
#include "test_pv_drm_plugin.h"
#include "test_pv_drm_plugin_ut.h"
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "oscl_error_panic.h"
#include "oscl_scheduler.h"
#include "pvlogger.h"
#include "pvlogger_file_appender.h"
#include "unit_test_args.h"
#include "pvmf_recognizer_registry.h"
#include "pvoma1_kmj_recognizer.h"
#include "pvmi_datastreamsyncinterface_ref_factory.h"
#include "pvmi_data_stream_interface.h"
#include "oscl_utf8conv.h"
#include "oscl_string_utils.h"
#include "unit_test_args.h"

#include "OMX_Core.h"

void FindSourceFile( cmd_line* command_line,
                     OSCL_HeapString<OsclMemAllocator> &aFileNameInfo,
                     PVMFFormatType &aInputFileFormatType)
{
    aFileNameInfo = SOURCENAME_PREPEND_STRING;
    aFileNameInfo += DEFAULTSOURCEFILENAME;
    aInputFileFormatType = PVMF_FORMAT_UNKNOWN;

    int iFileArgument = 0;
    bool iFileFound = false;
    bool cmdline_iswchar = command_line->is_wchar();

    int count = command_line->get_count();

    // Search for the "-source" argument
    // Go through each argument
    for(int iFileSearch = 0; iFileSearch < count; iFileSearch++)
    {
        char argstr[128] = {0};

        // Convert to UTF8 if necessary
        if(cmdline_iswchar)
        {
            oscl_wchar* argwstr = NULL;
            command_line->get_arg(iFileSearch, argwstr);
            oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
            argstr[127] = 0;
        }
        else
        {
            char* tmpstr = NULL;
            command_line->get_arg(iFileSearch, tmpstr);
            int32 tmpstrlen = oscl_strlen(tmpstr)+1;
            if (tmpstrlen > 128)
            {
                tmpstrlen = 128;
            }

            oscl_strncpy(argstr, tmpstr, tmpstrlen);
            argstr[tmpstrlen-1] = 0;
        }

        // Do the string compare
        if(oscl_strcmp(argstr, "-help") == 0)
        {
            printf("Source specification option. Default is 'test.mp4':\n");
            printf("  -source sourcename\n");
            printf("   Specify the source filename or URL to use for test cases which\n");
            printf("   allow user-specified source name. The unit test determines the\n");
            printf("   source format type using extension or URL header.\n\n");
        }
        else if(oscl_strcmp(argstr, "-source") == 0)
        {
            iFileFound = true;
            iFileArgument = ++iFileSearch;
            break;
        }
    }

    if(iFileFound)
    {
        // Convert to UTF8 if necessary
        if (cmdline_iswchar)
        {
            oscl_wchar* cmd;
            command_line->get_arg(iFileArgument, cmd);

            char tmpstr[256];
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), tmpstr, 256);

            tmpstr[255] = 0;
            aFileNameInfo = tmpstr;
        }
        else
        {
            char* cmdlinefilename = NULL;
            command_line->get_arg(iFileArgument, cmdlinefilename);
            aFileNameInfo = cmdlinefilename;
        }
    }
}

void FindDrmTestCaseParam(cmd_line* command_line, int32_t& iUseMethod)
{
    bool cmdline_iswchar = command_line->is_wchar();
    int count = command_line->get_count();

    char *iSourceFind = NULL;

    if (cmdline_iswchar)
    {
        iSourceFind = new char[256];
    }

    /* Go through each argument */
    for (int iDrmSearch = 0; iDrmSearch < count; iDrmSearch++)
    {
        /* Convert to UTF8 if necessary */
        if (cmdline_iswchar)
        {
            OSCL_TCHAR* cmd = NULL;
            command_line->get_arg(iDrmSearch, cmd);
            oscl_UnicodeToUTF8(cmd, oscl_strlen(cmd), iSourceFind, 256);
        }
        else
        {
            iSourceFind = NULL;
            command_line->get_arg(iDrmSearch, iSourceFind);
        }

        /* Do the string compare */
        if (oscl_strcmp(iSourceFind, "-help") == 0)
        {
            printf("Test cases to run drm cases option. Default is ALL:\n");
            printf("  play\n");
            printf("   Specify to play a DRM protected object.\n");
            printf("  download\n");
            printf("   Specify to downlaod rights for a SD DRM 1.0 media.\n");
            printf("  new_cd\n");
            printf("   indicates that this is a new CD DRM 1.0 media.\n");
        }
    else /* set the usemethod according the input command */
    {
          if (oscl_strcmp(iSourceFind, "play") == 0)
          {
              iUseMethod = DRM_PERMISSION_PLAY;
          }
          else if (oscl_strcmp(iSourceFind, "download") == 0)
          {
              iUseMethod = DRM_DOWNLOAD_RIGHTS_AND_PLAY;
          }
          else if (oscl_strcmp(iSourceFind, "new_cd") == 0)
          {
              iUseMethod = DRM_NEW_CD;
          }
      else if (oscl_strcmp(iSourceFind, "unit_test") == 0)
          {
              iUseMethod = DRM_UNIT_TEST;
          }
          else
          {
              iUseMethod = -1; /* unknown use method */
          }
    }
    }

    if (cmdline_iswchar)
    {
        delete[] iSourceFind;
        iSourceFind = NULL;
    }
}

int _local_main(cmd_line* command_line );


// Entry point for the unit test program
int local_main(cmd_line* command_line )
{
    //Init Oscl
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();

    OMX_Init();

    //Run the test under a trap
    int result = 0;
    int32 err = 0;
    TPVErrorPanic panic;

    OSCL_PANIC_TRAP(err,panic,result=_local_main(command_line););

    //Show any exception.
    if (err!=0)
    {
        printf("Error!  Leave %d\n", err);
    }
    if (panic.iReason!=0)
    {
        printf("Error!  Panic %s %d\n", panic.iCategory.Str(), panic.iReason);
    }

    //Cleanup
    OMX_Deinit();

    OsclMem::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    return result;
}

int main(int argc, char **argv)
{
    int n = 1;

    cmd_line *command_line_ptr;
    cmd_line_linux command_line;
    command_line_ptr = &command_line;

    command_line.setup(argc-n,&argv[n]);
    return local_main(command_line_ptr);
}

int _local_main(cmd_line *command_line)
{
    // Print out the extension for help if no argument
    if (command_line->get_count()==0)
    {
        printf("  Specify '-help' first to get help information on options\n\n");
    }

    OSCL_HeapString<OsclMemAllocator> filenameinfo;
    PVMFFormatType inputformattype ;
    FindSourceFile(command_line, filenameinfo, inputformattype);

    int32_t useMethod;
    FindDrmTestCaseParam(command_line, useMethod);

    pvplayer_engine_test_suite *engine_tests = NULL;
    engine_tests = new pvplayer_engine_test_suite( filenameinfo.get_str(),
                                                   inputformattype,
                                                   useMethod);
    if (engine_tests)
    {
        // Run the engine test
        engine_tests->run_test();

        // Print out the results
        text_test_interpreter interp;
        _STRING rs = interp.interpretation(engine_tests->last_result());
        printf(rs.c_str());

        const test_result the_result = engine_tests->last_result();
        delete engine_tests;
        engine_tests = NULL;

        return (the_result.success_count() != the_result.total_test_count());
    }
    else
    {
        printf("ERROR! pvplayer_engine_test_suite could not be instantiated.\n");
        return 1;
    }
}

pvplayer_engine_test_suite::pvplayer_engine_test_suite( char *aFileName,
                                                        PVMFFormatType aFileType,
                                                        int32_t aUseMethod)
                                                        : test_case()
{
    adopt_test_case(new pvplayer_engine_test( aFileName,
                                              aFileType,
                                              aUseMethod));
}

pvplayer_engine_test::pvplayer_engine_test( char *aFileName,
                                            PVMFFormatType aFileType,
                                            int32_t aUseMethod)
{
    iFileName = aFileName;
    iFileType = aFileType;
    iUseMethod = aUseMethod;
    iCurrentTest = NULL;
}

pvplayer_engine_test::~pvplayer_engine_test()
{
}

void pvplayer_engine_test::TestCompleted(test_case &tc)
{
    // Stop the scheduler
    OsclExecScheduler *sched=OsclExecScheduler::Current();
    if (sched)
    {
        sched->StopScheduler();
    }
}

void pvplayer_engine_test::test()
{
    if (iCurrentTest)
    {
        delete iCurrentTest;
        iCurrentTest = NULL;

        // Shutdown PVLogger and scheduler before checking mem stats
        OsclScheduler::Cleanup();
        PVLogger::Cleanup();
    }

    bool setupLoggerScheduler = true;
    printf("\nStarting DRM 1.0 Plugin Test\r\n");
    SetupLoggerScheduler();

    // Setup the standard test case parameters based on current unit test settings
    PVPlayerAsyncTestParam testparam;
    testparam.iObserver = this;
    testparam.iTestCase = this;
    testparam.iFileName = iFileName;
    testparam.iFileType = iFileType;
    testparam.iUseMethod = iUseMethod;

    switch(iUseMethod)
    {
        case DRM_DOWNLOAD_RIGHTS_AND_PLAY:
        case DRM_PERMISSION_PLAY:
        case DRM_NEW_CD:
            iCurrentTest = new pvplayer_async_test_drmcpmplugin(testparam);
            break;
        case DRM_UNIT_TEST:
            iCurrentTest = new pvplayer_test_drmcpmplugin_ut(testparam);
            break;
        default:
            break;
    }

    if (iCurrentTest)
    {
        OsclExecScheduler *sched = OsclExecScheduler::Current();

        if (sched)
        {
            // Start the test
            iCurrentTest->StartTest();

            // Start the scheduler so the test case would run
            #if USE_NATIVE_SCHEDULER
                // Have PV scheduler use the scheduler native to the system
                sched->StartNativeScheduler();
            #else
                // Have PV scheduler use its own implementation of the scheduler
                sched->StartScheduler();
            #endif
        }
        else
        {
            printf("ERROR! Scheduler is not available. Test could not run.");
        }
    }
    else //no test to run, skip to next test.
    {
        if(setupLoggerScheduler)
        {
            // Shutdown PVLogger and scheduler before continuing on
            OsclScheduler::Cleanup();
            PVLogger::Cleanup();
            setupLoggerScheduler = false;
        }
    }
}

void pvplayer_engine_test::SetupLoggerScheduler()
{
    // Enable the following code for logging (on Symbian, RDebug)
    PVLogger::Init();

    // Construct and install the active scheduler
    OsclScheduler::Init("PVPlayerEngineTestScheduler");
}


