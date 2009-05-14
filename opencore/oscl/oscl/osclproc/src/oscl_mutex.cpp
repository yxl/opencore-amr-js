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

//                  OSCL_MUTEX (M U T E X  I M P L E M E N T A T I O N)

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/*! \file oscl_mutex.cpp .This file provides MUTEX implementation that can be ported
to three OS LINUX, SYMBIAN & WIN32
*/

#include "oscl_mutex.h"


#include "oscl_assert.h"

OSCL_EXPORT_REF OsclMutex::OsclMutex()
{
    bCreated = false;
}

OSCL_EXPORT_REF OsclMutex::~OsclMutex()
{
    //make sure it's closed
    if (bCreated)
        Close();
}

/*
 * Creates the Mutex
 *
 * @param MutexName    NULL terminated string.
 *
 * @return Returns the Error whether it is success or failure
 *incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclMutex::Create()
{
    if (bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int result = pthread_mutex_init(&ObjMutex, NULL);
    if (result == 0)
    {
        bCreated = true;
        return OsclProcStatus::SUCCESS_ERROR;
    }
    else
        return ErrorMapping(result);


}


/**
 * Locks the Mutex
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 *incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF void OsclMutex::Lock()
{
    //verify the mutex is created.
    OSCL_ASSERT(bCreated);


    pthread_mutex_lock(&ObjMutex);

}


/**
 * Try to lock the mutex,if the Mutex is already locked calling thread
 * immediately returns with out blocking
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 *incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclMutex::TryLock()
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int result = pthread_mutex_trylock(&ObjMutex);
    switch (result)
    {
        case 0:
            return OsclProcStatus::SUCCESS_ERROR;
        case EBUSY:
            return OsclProcStatus::MUTEX_LOCKED_ERROR;
        default:
            return ErrorMapping(result);
    }

}

/**
 * Releases the Mutex
 *
 * @param It wont take any parameters
 *
 * @return Returns the Error whether it is success or failure
 *incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF void OsclMutex::Unlock()
{
    //verify the mutex is created.
    OSCL_ASSERT(bCreated);


    pthread_mutex_unlock(&ObjMutex);

}



/**
 * Closes the Mutex
 *
 * @param It wont take any prameters
 *
 * @return Returns the Error whether it is success or failure
 *incase of failure it will return what is the specific error
 */
OSCL_EXPORT_REF OsclProcStatus::eOsclProcError OsclMutex::Close()
{
    if (!bCreated)
        return OsclProcStatus::INVALID_OPERATION_ERROR;


    int result = pthread_mutex_destroy(&ObjMutex);
    if (result == 0)
    {
        bCreated = false;
        return OsclProcStatus::SUCCESS_ERROR;
    }
    else
        return ErrorMapping(result);

}



/**
 * Error Mapping
 *
 * @param It will take error returned by OS specific API
 *
 * @return Returns specific error
 */
OsclProcStatus::eOsclProcError OsclMutex::ErrorMapping(int32 Error)
{


    switch (Error)
    {
        case 0:
            return OsclProcStatus::SUCCESS_ERROR;
        case EAGAIN :
            return  OsclProcStatus::SYSTEM_RESOURCES_UNAVAILABLE_ERROR;
        case EINVAL :
            return  OsclProcStatus::INVALID_ARGUMENT_ERROR;
        case ENOMEM :
            return  OsclProcStatus::NOT_ENOUGH_MEMORY_ERROR;
        case EFAULT  :
            return  OsclProcStatus::INVALID_POINTER_ERROR;
        case EBUSY:
            return 	OsclProcStatus::MUTEX_LOCKED_ERROR;
        default:
            return OsclProcStatus::OTHER_ERROR;
    }

}

OSCL_EXPORT_REF OsclThreadLock::OsclThreadLock()
{
    iMutex.Create();
}
OSCL_EXPORT_REF OsclThreadLock::~OsclThreadLock()
{
    iMutex.Close();
}
OSCL_EXPORT_REF void OsclThreadLock::Lock()
{
    iMutex.Lock();
}
OSCL_EXPORT_REF void OsclThreadLock::Unlock()
{
    iMutex.Unlock();
}

