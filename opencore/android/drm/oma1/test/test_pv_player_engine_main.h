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

#ifndef TEST_PV_PLAYER_ENGINE_MAIN_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_MAIN_H_INCLUDED

#include "test_case.h"
#include "text_test_interpreter.h"
#include "oscl_scheduler_ao.h"
#include "oscl_exception.h"
#include "pv_player_factory.h"
#include "pv_player_interface.h"
#include "pv_engine_observer.h"
#include "pvlogger.h"
#include "pvlogger_stderr_appender.h"
#include "pvlogger_time_and_id_layout.h"
#include "pvmi_media_io_fileoutput.h"

/* added for DRM integration */
#include "objmng/svc_drm.h"

#define DRM_DOWNLOAD_RIGHTS_AND_PLAY 0x20
#define DRM_NEW_CD  0x21
#define DRM_UNIT_TEST 0x22

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
public:
    virtual void destruct_and_dealloc(OsclAny *ptr)
    {
        delete((DestructClass*)ptr);
    }
};

class pvplayer_engine_test_suite : public test_case
{
public:
    pvplayer_engine_test_suite( char *aFilename,
                                PVMFFormatType aFiletype,                                
                                int32_t aUseMethod);
};

// Observer class for pvPlayer async test to notify completion of test
class pvplayer_async_test_observer
{
public:
    // Signals completion of test. Test instance can be deleted after this callback completes.
    virtual void TestCompleted(test_case &)=0;
};

typedef struct
{
    pvplayer_async_test_observer* iObserver;
    test_case* iTestCase;
    FILE* iTestMsgOutputFile;
    char* iFileName;
    PVMFFormatType iFileType;   
    int32_t iUseMethod;
} PVPlayerAsyncTestParam;

#define PVPATB_TEST_IS_TRUE(condition) (iTestCase->test_is_true_stub( (condition),(#condition),__FILE__,__LINE__ ))

typedef enum
{
    STATE_CREATE,
    STATE_QUERYINTERFACE,
    STATE_ADDDATASOURCE,
    STATE_CONFIGPARAMS,
    STATE_INIT,
    STATE_GETMETADATAKEYLIST,
    STATE_GETMETADATAVALUELIST,
    STATE_ADDDATASINK_VIDEO,
    STATE_ADDDATASINK_AUDIO,
    STATE_PREPARE,
    STATE_WAIT_FOR_DATAREADY,
    STATE_WAIT_FOR_BUFFCOMPLETE,
    STATE_CANCELALL,
    STATE_WAIT_FOR_CANCELALL,
    STATE_START,
    STATE_SETPLAYBACKRANGE,
    STATE_PAUSE,
    STATE_RESUME,
    STATE_EOSNOTREACHED,
    STATE_STOP,
    STATE_REMOVEDATASINK_VIDEO,
    STATE_REMOVEDATASINK_AUDIO,
    STATE_RESET,
    STATE_REMOVEDATASOURCE,
    STATE_WAIT_FOR_ERROR_HANDLING,
    STATE_CLEANUPANDCOMPLETE
}PVTestState;

// The base class for all pvplayer engine asynchronous tests
class pvplayer_async_test_base : public OsclTimerObject,
                                 public PVCommandStatusObserver,
                                 public PVInformationalEventObserver,
                                 public PVErrorEventObserver
{
public:
    pvplayer_async_test_base(PVPlayerAsyncTestParam aTestParam) :
      OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerEngineAsyncTestBase")
    {
        OSCL_ASSERT(aTestParam.iObserver!=NULL);
        OSCL_ASSERT(aTestParam.iTestCase!=NULL);
        iObserver=aTestParam.iObserver;
        iTestCase=aTestParam.iTestCase;
        iTestMsgOutputFile=aTestParam.iTestMsgOutputFile;
        iFileName=aTestParam.iFileName;
        iFileType=aTestParam.iFileType;
        iTestCaseName=_STRLIT_CHAR(" ");        
        iUseMethod = aTestParam.iUseMethod;

        // Initialize the variables to use for context data testing
        iContextObjectRefValue=0x5C7A; // some random number
        iContextObject=iContextObjectRefValue;
    }

    virtual ~pvplayer_async_test_base(){}
    virtual void StartTest()=0;

    virtual void CommandCompleted(const PVCmdResponse& /*aResponse*/){}
    virtual void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/){}
    virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/){}

    // Utility function to retrieve the filename from string and replace ',' with '_'
    void RetrieveFilename( const oscl_wchar* aSource,
                           OSCL_wHeapString<OsclMemAllocator>& aFilename)
    {
        if (aSource==NULL)
        {
            return;
        }

        // Find the last '\' or '/' in the string
        oscl_wchar* lastslash=(oscl_wchar*)aSource;
        bool foundlastslash=false;
        while (!foundlastslash)
        {
            oscl_wchar* tmp1=oscl_strstr(lastslash, _STRLIT_WCHAR("\\"));
            oscl_wchar* tmp2=oscl_strstr(lastslash, _STRLIT_WCHAR("/"));
            if (tmp1!=NULL)
            {
                lastslash=tmp1+1;
            }
            else if (tmp2!=NULL)
            {
                lastslash=tmp2+1;
            }
            else
            {
                foundlastslash=true;
            }
        }

        // Now copy the filename
        if (lastslash)
        {
            aFilename=lastslash;
        }

        // Replace each '.' in filename with '_'
        bool finishedreplace=false;
        while (!finishedreplace)
        {
            oscl_wchar* tmp=oscl_strstr(aFilename.get_cstr(), _STRLIT_WCHAR("."));
            if (tmp!=NULL)
            {
                oscl_strncpy(tmp, _STRLIT_WCHAR("_"), 1);
            }
            else
            {
                finishedreplace=true;
            }
        }
    }

    pvplayer_async_test_observer* iObserver;
    test_case* iTestCase;
    FILE* iTestMsgOutputFile;
    char *iFileName;
    PVMFFormatType iFileType;
    int32_t iUseMethod;
    
    OSCL_HeapString<OsclMemAllocator> iTestCaseName;

    uint32 iContextObject;
    uint32 iContextObjectRefValue;    
};

// test_base-based class which will run async tests on pvPlayer engine
class pvplayer_engine_test : public test_case,
                             public pvplayer_async_test_observer
{
public:
    pvplayer_engine_test( char *aFileName,
                          PVMFFormatType aFileType,                         
                          int32_t aUseMethod);
    ~pvplayer_engine_test();

    // From test_case
    virtual void test();

    // From pvplayer_async_test_observer
    void TestCompleted(test_case&);

    void SetupLoggerScheduler();

private:
    char *iFileName;
    PVMFFormatType iFileType;
    int32_t iUseMethod;
    
    pvplayer_async_test_base* iCurrentTest;    
};

/*!
** PVPlayerTestMioFactory: MIO Factory functions
*/
class PvmiMIOControl;
class PVPlayerTestMioFactory
{
public:
    static PvmiMIOControl* CreateAudioOutput(OsclAny* aParam);
    static PvmiMIOControl* CreateAudioOutput( OsclAny* aParam,
                                              PVRefFileOutputTestObserver* aObserver,
                                              bool aActiveTiming,
                                              uint32 aQueueLimit,
                                              bool aSimFlowControl,
                                              bool logStrings=true);
    static void DestroyAudioOutput(PvmiMIOControl* aMio);
    static PvmiMIOControl* CreateVideoOutput(OsclAny* aParam);
    static PvmiMIOControl* CreateVideoOutput( OsclAny* aParam,
                                              PVRefFileOutputTestObserver* aObserver,
                                              bool aActiveTiming,
                                              uint32 aQueueLimit,
                                              bool aSimFlowControl,
                                              bool logStrings=true);
    static void DestroyVideoOutput(PvmiMIOControl* aMio);
    static PvmiMIOControl* CreateTextOutput(OsclAny* aParam);
    static void DestroyTextOutput(PvmiMIOControl* aMio);
};

#endif
