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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                  OSCL_T H R E A D (T H R E A D  I M P L E M E N T A T I O N)

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \file oscl_thread.cpp .This file provides THREAD implementation that can be ported
to three OS LINUX, SYMBIAN, WIN32
*/



// Implementation file for OSCL threads
#include "oscl_thread.h"



/*
 * Class contructor
 */
OSCL_EXPORT_REF OsclThread::OsclThread()
{
    bCreated = false;

}



/*
 * Class destructor
 */
OSCL_EXPORT_REF OsclThread::~OsclThread()
{

    ;

}


/**
 * OSCL Proc layer function
 * Input Argument:
 * function_name  =  Name of the thread Function
 * stack_size  =  Size of the thread stack
 * argument = Argument to be passed to thread function
 * Thread_State = Enumeration which specifies the state of the thread on creation
 * 		  with values Running and Suspend
 * Return value : eOsclProcError
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::Create(TOsclThreadFuncPtr function_name,
        int32 stack_size,
        TOsclThreadFuncArg argument,
        OsclThread_State state)
{
    if (stack_size < 0)
        return OsclProcStatus::INVALID_PARAM_ERROR;

    if (function_name == NULL)
        return OsclProcStatus::INVALID_PARAM_ERROR;

    //Reset thread creation state, since the thread may
    //have exited.
    if (bCreated)
        bCreated = false;


    switch (state)
    {
        case Start_on_creation:
            break;
        case Suspend_on_creation:
            //can't create suspended pthread.
            return OsclProcStatus::INVALID_PARAM_ERROR;
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size != 0)
        pthread_attr_setstacksize(&attr, stack_size);

    // Default detachstate attribute to PTHREAD_CREATE_DETACHED state
    int detach_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    switch (detach_ret)
    {
        case 0:	// successful, continue thread creation
            break;
        case EINVAL:
        default:
            return OsclProcStatus::PSHARED_ATTRIBUTE_SETTING_ERROR;
    }

    int result = pthread_create(
                     (pthread_t*) & ObjThread,
                     &attr,
                     function_name,
                     (void*)argument);

    switch (result)
    {
        case 0:
            bCreated = true;
            return OsclProcStatus::SUCCESS_ERROR;
        case EAGAIN:
            return OsclProcStatus::NOT_ENOUGH_RESOURCES_ERROR;
        default:
            return OsclProcStatus::OTHER_ERROR;
    }

}


/**
 * OSCL Proc layer function
 * ExitThread is a static function which is used to end the current thread. When called it
 * just ends the execution of the current thread.
 * Input Argument:
 * exitcode  =  Exitcode of the thread. This can be used by other threads to know the
 *				exit status of this thread.
 * Return value : None
*/
OSCL_EXPORT_REF void OsclThread::Exit(OsclAny* exitcode)
{
    //intentionally not implemented.
    OSCL_UNUSED_ARG(exitcode);

}

OSCL_EXPORT_REF void OsclThread::EnableKill()
{
    ; //nothing needed

}

/**
 * OSCL Proc layer function
 * Suspend current thread execution for specified time.
 * @param msec, t2: sleep time in milliseconds.
 */
OSCL_EXPORT_REF void OsclThread::SleepMillisec(const int32 msec)
{
    struct timespec reqt, remt;
    reqt.tv_sec = msec / 1000;
    reqt.tv_nsec = 1000000 * (msec % 1000);
    nanosleep(&reqt, &remt) ;

}


/**

 * OSCL Proc layer function

 * This is used to terminate a thread other than the current thread.

 * Input Argument:

 * exitcode  =  Exitcode of the thread. This can be used by other threads to know the

 * 				exit status of this thread.

 * Return value : Error code

 */

OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::Terminate(OsclAny* oscl_ExitCode)
{


    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;

    {
        //intentionally not implemented.

        OSCL_UNUSED_ARG(oscl_ExitCode);

        bCreated = false;

        return OsclProcStatus::NOT_IMPLEMENTED;
    }
}







/**

 * OSCL Proc layer function

 * This API suspends the thread being referred. The thread can later be brought into execution

 * by calling OSCL_ResumeThread() on it.

 * Input Argument: None

 * Return value : Error code

 */

OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::Suspend()

{

    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;




    {
        //pthread doesn't support suspend/resume
        return OsclProcStatus::NOT_IMPLEMENTED;

    }


}







/**

 * OSCL Proc layer function

 * ResumeThread resumes the suspended thread and brings it into execution.

 * Input Argument: None

 * Return value : Error code

 */

OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::Resume()

{

    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;




    {

        //pthread doesn't support suspend/resume
        return OsclProcStatus::NOT_IMPLEMENTED;

    }


}





/**

 * OSCL Proc layer function

 * GetThreadPriority gets the priority of the thread. It takes reference of the input argument

 * and assigns priority to it from one of the already defined priorities.

 * Input Argument:

 * int16& refThreadPriority : Output Priority value

 * Return value : Error code

 */

OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::GetPriority(OsclThreadPriority& refThreadPriority)

{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;

    struct sched_param schedparam;
    int sched_policy = 0;
    int pri;
    int result;
    result = pthread_getschedparam(ObjThread, &sched_policy, &schedparam);
    if (result != 0)
    {
        switch (result)
        {
            case ESRCH:
                return OsclProcStatus::INVALID_THREAD_ERROR;
            case EFAULT:
                return OsclProcStatus::INVALID_PARAM_ERROR;
            default:
                return OsclProcStatus::OTHER_ERROR;
        }
    }

    pri = schedparam.sched_priority;
    switch (sched_policy)
    {
        case SCHED_RR:
        case SCHED_FIFO:
        {
            //static priority is 1..99
            if (pri < 1)
                return OsclProcStatus::OTHER_ERROR;
            else if (pri == 1)
                refThreadPriority = ThreadPriorityLowest;
            else if (pri <= 20)
                refThreadPriority = ThreadPriorityLow;
            else if (pri <= 40)
                refThreadPriority = ThreadPriorityBelowNormal;
            else if (pri <= 55)
                refThreadPriority = ThreadPriorityNormal;
            else if (pri <= 75)
                refThreadPriority = ThreadPriorityAboveNormal;
            else if (pri <= 98)
                refThreadPriority = ThreadPriorityHighest;
            else if (pri == 99)
                refThreadPriority = ThreadPriorityTimeCritical;
            else if (pri > 99)
                return OsclProcStatus::OTHER_ERROR;
            return OsclProcStatus::SUCCESS_ERROR;
        }
        break;

        case SCHED_OTHER:
        {
            //static priority is always zero.
            if (pri != 0)
                return OsclProcStatus::OTHER_ERROR;
            refThreadPriority = ThreadPriorityNormal;
            return OsclProcStatus::SUCCESS_ERROR;
        }
        break;

        default:
            return OsclProcStatus::OTHER_ERROR;
    }
}



/**
 * OSCL Proc layer function
 * SetThreadPriority sets the priority of the thread. It takes priority as the input argument
 * and assigns it to the thread referred.
 * Input Argument:
 * ePriorityLevel : Input Priority value
 * Return value : Error code
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::SetPriority(OsclThreadPriority ePriority)
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;

    //not supported.
    OSCL_UNUSED_ARG(ePriority);
    return OsclProcStatus::NOT_IMPLEMENTED;

}



/**

 * OSCL Proc layer function

 * This is used to compare whether the thread ID's are equal.

 * @param Thread ID passed by the application

 * @return Error code

 */

OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclThread::GetId(TOsclThreadId& refThreadId)

{




    refThreadId = pthread_self();

    return OsclProcStatus::SUCCESS_ERROR;


}


OSCL_EXPORT_REF bool OsclThread::CompareId(TOsclThreadId &t1, TOsclThreadId &t2)
//static routine
{
    return (pthread_equal(t1, t2) != 0);
}






/**

 * Helper Function

 * Map the Operating system errors to OSCL defined erros

 * Input Argument:

 * error : Input error as one of the OS errors

 * Return value : Error code ( User defined )

 */






