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


//                  OSCL_S E M A P H O R E (S E M A P H O R E  I M P L E M E N T A T I O N)


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \file oscl_semaphore.cpp .This file provides SEMAPHORE implementation that can be ported
to three OS LINUX, SYMBIAN, WIN32
*/

// Implementation file for OSCL Semaphores
#include "oscl_semaphore.h"
#include "oscl_assert.h"



// Class contructor
OSCL_EXPORT_REF OsclSemaphore::OsclSemaphore()
{
    bCreated = false;
}

OSCL_EXPORT_REF OsclSemaphore::~OsclSemaphore()
{
    //make sure it was closed
    OSCL_ASSERT(!bCreated);
}


/**
 * Creates the Semaphore
 *
 * @param Intialcount & Maximum count
 *
 * @return Returns the Error whether it is success or failure.
 * Incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::Create(uint32 InitialCount)
{
    if (bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    iCount = InitialCount;
    if (pthread_cond_init(&ObjCondition, NULL) == 0
            && pthread_mutex_init(&ObjMutex, NULL) == 0)
    {
        bCreated = true;
        return OsclProcStatus::SUCCESS_ERROR;
    }
    return OsclProcStatus::OTHER_ERROR;

}


/**
 * Closes the Semaphore
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure.
 * Incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::Close(void)
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;

    bCreated = false;
    return OsclProcStatus::SUCCESS_ERROR;

}



/**
 * Waits for access to the  Semaphore
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 * Incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::Wait()
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int res = pthread_mutex_lock(&ObjMutex);
    if (res != 0)
        return OsclProcStatus::OTHER_ERROR;
    while (iCount == 0 && res == 0)
    {   // wait till the semaphore is signaled
        // or an error has occurred
        res = pthread_cond_wait(&ObjCondition, &ObjMutex);
    }
    if (res == 0)
    {   // signaled
        iCount--;
    }
    pthread_mutex_unlock(&ObjMutex);
    if (res != 0)
    {   // error occurred
        // invalid condition or mutex
        return OsclProcStatus::OTHER_ERROR;
    }
    return OsclProcStatus::SUCCESS_ERROR;

}

#include "osclconfig_time.h"
static void getAbsTime(struct timespec &abs, uint32 timeout_msec)
{
    //convert timeout msec to sec + usec.
    uint32 timeout_sec = timeout_msec / 1000;
    uint32 timeout_usec = (timeout_msec % 1000) * 1000;

    //compute target time = time now plus timeout.

    struct timeval timenow;
    gettimeofday(&timenow, NULL);

    uint32 targ_sec = timenow.tv_sec + timeout_sec;
    uint32 targ_usec = timenow.tv_usec + timeout_usec;
    while (targ_usec >= 1000000)
    {
        targ_sec++;
        targ_usec -= 1000000;
    }

    //convert target time to timespec.
    abs.tv_sec = targ_sec;
    abs.tv_nsec = targ_usec * 1000;
}

/**
 * Makes the thread to wait on the Semaphore, with a timeout.
 *
 * @param timeout in milliseconds.
 *
 * @return Returns WAIT_TIMEOUT_ERROR if the timeout expires,
 * SUCCESS_ERROR if the semaphore is obtained, or in
 * case of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::Wait(uint32 timeout_msec)
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int res = pthread_mutex_lock(&ObjMutex);
    if (res != 0)
        return OsclProcStatus::OTHER_ERROR;
    if (iCount > 0)
    {
        iCount--;
        pthread_mutex_unlock(&ObjMutex);
        return OsclProcStatus::SUCCESS_ERROR;
    }

    struct timespec abs;
    getAbsTime(abs, timeout_msec);

    while ((iCount == 0) && (res == 0))
    {   // wait till semaphore is signaled
        // or time runs out
        res = pthread_cond_timedwait(&ObjCondition, &ObjMutex, &abs);
    }
    if (res == 0)
    {   // signalied
        iCount--;
    }
    pthread_mutex_unlock(&ObjMutex);
    if (res == ETIMEDOUT)
    {   // timeour occurred
        return OsclProcStatus::WAIT_TIMEOUT_ERROR;
    }
    else if (res != 0)
    {   // error occurred
        return OsclProcStatus::OTHER_ERROR;
    }
    return OsclProcStatus::SUCCESS_ERROR;

}

/**
 * Try to acquire semaphore, if the semaphore is already acquired by another thread,
 *  calling thread immediately returns with out blocking
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 * Incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::TryWait()
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int res = pthread_mutex_lock(&ObjMutex);
    if (res != 0)
        return OsclProcStatus::OTHER_ERROR;
    if (iCount > 0)
    {//take it
        iCount--;
        pthread_mutex_unlock(&ObjMutex);
        return OsclProcStatus::SUCCESS_ERROR;
    }
    else
    {//can't take it now so don't block.
        pthread_mutex_unlock(&ObjMutex);
        return OsclProcStatus::SEM_NOT_SIGNALED_ERROR;
    }

}


/**
 * Signals that the thread is finished with the Semaphore
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 * Incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclSemaphore::Signal()
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int res = pthread_mutex_lock(&ObjMutex);
    if (res != 0)
        return OsclProcStatus::OTHER_ERROR;
    iCount++;
    res = pthread_cond_signal(&ObjCondition);
    pthread_mutex_unlock(&ObjMutex);
    if (res != 0)
        return OsclProcStatus::OTHER_ERROR;
    return OsclProcStatus::SUCCESS_ERROR;


}

