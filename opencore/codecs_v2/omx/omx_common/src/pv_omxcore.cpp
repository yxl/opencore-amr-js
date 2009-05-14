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
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef PV_OMXCORE_H_INCLUDED
#include "pv_omxcore.h"
#endif

#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif

#ifndef OSCL_STDSTRING_H_INCLUDED
#include "oscl_stdstring.h"
#endif

#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif

#ifndef OSCL_INIT_H_INCLUDED
#include "oscl_init.h"
#endif

#ifndef PV_OMX_CONFIG_PARSER_H
#include "pv_omx_config_parser.h"
#endif

// pv_omxregistry.h is only needed if NOT using CML2
#ifndef USE_CML2_CONFIG
#include "pv_omxregistry.h"
#endif

#include "oscl_init.h"

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
// until dynamic registry - register all components
// unconditionally - may error out at load time

OMX_ERRORTYPE Mpeg4Register();
OMX_ERRORTYPE H263Register();
OMX_ERRORTYPE AvcRegister();
OMX_ERRORTYPE WmvRegister();
OMX_ERRORTYPE AacRegister();
OMX_ERRORTYPE AmrRegister();
OMX_ERRORTYPE Mp3Register();
OMX_ERRORTYPE WmaRegister();

OMX_ERRORTYPE AmrEncRegister();
OMX_ERRORTYPE Mpeg4EncRegister();
OMX_ERRORTYPE H263EncRegister();
OMX_ERRORTYPE AvcEncRegister();
OMX_ERRORTYPE AacEncRegister();


#else

#if REGISTER_OMX_M4V_COMPONENT
OMX_ERRORTYPE Mpeg4Register();
#endif

#if REGISTER_OMX_H263_COMPONENT
OMX_ERRORTYPE H263Register();
#endif

#if REGISTER_OMX_AVC_COMPONENT
OMX_ERRORTYPE AvcRegister();
#endif

#if REGISTER_OMX_WMV_COMPONENT
OMX_ERRORTYPE WmvRegister();
#endif

#if REGISTER_OMX_AAC_COMPONENT
OMX_ERRORTYPE AacRegister();
#endif

#if REGISTER_OMX_AMR_COMPONENT
OMX_ERRORTYPE AmrRegister();
#endif

#if REGISTER_OMX_MP3_COMPONENT
OMX_ERRORTYPE Mp3Register();
#endif

#if REGISTER_OMX_WMA_COMPONENT
OMX_ERRORTYPE WmaRegister();
#endif

#if REGISTER_OMX_AMRENC_COMPONENT
OMX_ERRORTYPE AmrEncRegister();
#endif

#if REGISTER_OMX_M4VENC_COMPONENT
OMX_ERRORTYPE Mpeg4EncRegister();
#endif

#if REGISTER_OMX_H263ENC_COMPONENT
OMX_ERRORTYPE H263EncRegister();
#endif

#if REGISTER_OMX_AVCENC_COMPONENT
OMX_ERRORTYPE AvcEncRegister();
#endif

#if REGISTER_OMX_AACENC_COMPONENT
OMX_ERRORTYPE AacEncRegister();
#endif
#endif
OSCL_DLL_ENTRY_POINT_DEFAULT()

/* Initializes the component */
static OMX_ERRORTYPE _OMX_Init()
{
    OMX_ERRORTYPE Status = OMX_ErrorNone;
    int32 error;
    //get global data structure
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (error) // can't access registry
    {
        return OMX_ErrorInsufficientResources;
    }
    else if (!data) // singleton object has been destroyed
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        return OMX_ErrorInsufficientResources;
    }

#if PROXY_INTERFACE
    ProxyApplication_OMX** pProxyTerm = data->ipProxyTerm;
#endif
    OMX_U32 ii;

    /* Initialize template list to NULL at the beginning */
    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii++)
    {
        data->ipRegTemplateList[ii] = NULL;
    }

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        data->iComponentHandle[ii] = NULL;
        data->ipInstantiatedComponentReg[ii] = NULL;
#if PROXY_INTERFACE
        pProxyTerm[ii] = NULL;
#endif
    }

    //Release the singleton.
    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
    if (error)
    {
        //registry error
        Status = OMX_ErrorUndefined;
        return Status;
    }

#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
//unconditional registration
    // MPEG4
    Status = Mpeg4Register();
    if (Status != OMX_ErrorNone)
        return Status;


    //H263
    Status = H263Register();
    if (Status != OMX_ErrorNone)
        return Status;

    // AVC
    Status = AvcRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    // WMV
    Status = WmvRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    // AAC
    Status = AacRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    // AMR
    Status = AmrRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    // MP3
    Status = Mp3Register();
    if (Status != OMX_ErrorNone)
        return Status;

    // WMA
    Status = WmaRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    //AMR ENCODER
    Status = AmrEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    //MPEG4 Encoder
    Status = Mpeg4EncRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    //H263 Encoder
    Status = H263EncRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    //H264/AVC Encoder
    Status = AvcEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;

    //AAC Encoder
    Status = AacEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;


#else
    // REGISTER COMPONENT TYPES (ONE BY ONE)
#if REGISTER_OMX_M4V_COMPONENT
    // MPEG4
    Status = Mpeg4Register();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_H263_COMPONENT
    //H263
    Status = H263Register();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_AVC_COMPONENT
    // AVC
    Status = AvcRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_WMV_COMPONENT
    // WMV
    Status = WmvRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_AAC_COMPONENT
    // AAC
    Status = AacRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_AMR_COMPONENT
    // AMR
    Status = AmrRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_MP3_COMPONENT
    // MP3
    Status = Mp3Register();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_WMA_COMPONENT
    // WMA
    Status = WmaRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_AMRENC_COMPONENT
    //AMR ENCODER
    Status = AmrEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_M4VENC_COMPONENT
    //MPEG4 Encoder
    Status = Mpeg4EncRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_H263ENC_COMPONENT
    //H263 Encoder
    Status = H263EncRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif
#if REGISTER_OMX_AVCENC_COMPONENT
    //H264/AVC Encoder
    Status = AvcEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif

#if REGISTER_OMX_AACENC_COMPONENT
    //AAC Encoder
    Status = AacEncRegister();
    if (Status != OMX_ErrorNone)
        return Status;
#endif
#endif
    return OMX_ErrorNone;
}

//this routine is needed to avoid a longjmp clobber warning
static void _Try_OMX_Init(int32& aError, OMX_ERRORTYPE& aStatus)
{
    OSCL_TRY(aError, aStatus = _OMX_Init(););
}
//this routine is needed to avoid a longjmp clobber warning
static void _Try_OMX_Create(int32& aError, OMXGlobalData*& aData)
{
    OSCL_TRY(aError, aData = OSCL_NEW(OMXGlobalData, ()););
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_Init()
{
    OMX_ERRORTYPE status = OMX_ErrorNone;

    //Check the global instance counter and only init OMX on the first call.
    bool osclInit = false;
    int32 error;
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);

    //Check for whether Oscl is initialized in this thread.  If not, we will
    //initialize it here.
    if (error == EPVErrorBaseNotInstalled)
    {
        //init all Oscl layers except Oscl scheduler.
        OsclSelect select;
        select.iOsclScheduler = false;
        OsclInit::Init(error, &select);
        if (error)
        {
            status = OMX_ErrorUndefined;//can't init Oscl
            return status;
        }
        else
        {
            osclInit = true;
        }
    }

    if (data)
    {
        //Just update the instance counter.
        data->iInstanceCount++;

        //Release the singleton.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        if (error)
        {
            status = OMX_ErrorUndefined;
            return status;
        }
    }
    else
    {
        //First call--
        //create the OMX singleton
        _Try_OMX_Create(error, data);
        if (error != OsclErrNone)
        {
            status = OMX_ErrorInsufficientResources;//some leave happened.
        }

        //Release the singleton.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        if (error)
        {
            //registry error
            status = OMX_ErrorUndefined;
            return status;
        }

        //If create succeeded, then init the OMX globals.
        if (status == OMX_ErrorNone)
        {
            _Try_OMX_Init(error, status);
            if (error != OsclErrNone)
            {
                status = OMX_ErrorUndefined;//probably no memory.
            }
            else
            {
                //keep track of whether we did Oscl init internally,
                //so we can cleanup later.
                data->iOsclInit = osclInit;
            }
        }
    }

    if (error && status == OMX_ErrorNone)
        status = OMX_ErrorUndefined;//registry error

    return status;
}



/* De-initializes the component*/
static OMX_ERRORTYPE _OMX_Deinit(OMXGlobalData* data)
{
    OMX_S32 ii;
#if PROXY_INTERFACE
    ProxyApplication_OMX** pProxyTerm = data->ipProxyTerm;
#endif
    OMX_HANDLETYPE* componentHandle = data->iComponentHandle;


    // go through all component instnaces and delete leftovers if necessary
    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {

#if PROXY_INTERFACE
        if (pProxyTerm[ii])
        {
            // delete leftover components
            // call the OMX_FreeHandle through the proxy
            if ((componentHandle[ii] != NULL) && (data->ipInstantiatedComponentReg[ii] != NULL))
                pProxyTerm[ii]->ProxyFreeHandle(componentHandle[ii]);

            // exit thread
            pProxyTerm[ii]->Exit();
            delete pProxyTerm[ii];
            // delete array entries associated with pProxyTerm and Component handle
            pProxyTerm[ii] = NULL;
            componentHandle[ii] = NULL;
            data->ipInstantiatedComponentReg[ii] = NULL;
        }
#else
        if ((componentHandle[ii] != NULL) && (data->ipInstantiatedComponentReg[ii] != NULL))
        {

            // call destructor with the corresponding handle as argument
            OMX_PTR &aOmxLib = data->ipInstantiatedComponentReg[ii]->SharedLibraryPtr;
            OMX_PTR aOsclUuid = data->ipInstantiatedComponentReg[ii]->SharedLibraryOsclUuid;
            OMX_U32 &aRefCount = data->ipInstantiatedComponentReg[ii]->SharedLibraryRefCounter;
            (data->ipInstantiatedComponentReg[ii]->FunctionPtrDestroyComponent)(componentHandle[ii], aOmxLib, aOsclUuid, aRefCount);
        }

        componentHandle[ii] = NULL;
        data->ipInstantiatedComponentReg[ii] = NULL;
#endif
    }

    //Finally de-register all the components
    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii++)
    {

        if (data->ipRegTemplateList[ii])
        {
            if (data->ipRegTemplateList[ii]->SharedLibraryOsclUuid)
            {
                oscl_free(data->ipRegTemplateList[ii]->SharedLibraryOsclUuid);
                data->ipRegTemplateList[ii]->SharedLibraryOsclUuid = NULL;
            }
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
            if (data->ipRegTemplateList[ii]->SharedLibraryPtr)
            {
                OsclSharedLibrary *lib = (OsclSharedLibrary *)(data->ipRegTemplateList[ii]->SharedLibraryPtr);
                lib->Close();
                OSCL_DELETE(lib);
                data->ipRegTemplateList[ii]->SharedLibraryPtr = NULL;
            }
#endif
            oscl_free(data->ipRegTemplateList[ii]);
            data->ipRegTemplateList[ii] = NULL;
        }
        else
        {
            break;
        }
    }

    return OMX_ErrorNone;
}

//this routine is needed to avoid a longjmp clobber warning.
static void _Try_OMX_Deinit(int32 &aError, OMX_ERRORTYPE& aStatus, OMXGlobalData* data)
{
    OSCL_TRY(aError, aStatus = _OMX_Deinit(data););
}

//this routine is needed to avoid a longjmp clobber warning.
static void _Try_Data_Cleanup(int32 &aError, OMXGlobalData* aData)
{
    OSCL_TRY(aError, OSCL_DELETE(aData););
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_Deinit()
{
    OMX_ERRORTYPE status = OMX_ErrorNone;

    //Check the global instance counter and only cleanup OMX on the last call.
    bool osclInit = false;
    int32 error;
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (data)
    {
        data->iInstanceCount--;
        if (data->iInstanceCount == 0)
        {
            //save the "OsclInit" flag to decide whether to cleanup Oscl later.
            osclInit = data->iOsclInit;

            //Cleanup the OMX globals.
            _Try_OMX_Deinit(error, status, data);
            if (error != OsclErrNone)
                status = OMX_ErrorUndefined;//some leave happened.

            //Regardless of the cleanup result, cleanup the OMX singleton.
            _Try_Data_Cleanup(error, data);
            data = NULL;
            if (error != OsclErrNone)
                status = OMX_ErrorUndefined;//some leave happened.

        }
    }

    //Release the singleton.
    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);

    //If this is the last call and we initialized Oscl in OMX_Init,
    //then clean it up here.
    if (osclInit)
    {
        //cleanup all layers except Oscl scheduler.
        OsclSelect select;
        select.iOsclScheduler = false;
        OsclInit::Cleanup(error, &select);
        //ignore errors here.
    }

    return status;
}

#if PROXY_INTERFACE
static void _Cleanup_Component(ProxyApplication_OMX* aProxyTerm, OMX_OUT OMX_HANDLETYPE aHandle,
                               OMX_STRING cComponentName)
{
    if (aProxyTerm)
    {
        aProxyTerm->Exit();
        delete aProxyTerm;
    }

    if (!aHandle)
    {
        return;
    }

    //find the component destructor and call it
    OMX_S32 ii;
    int32 error;

    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);

    if (error || !data)
    {
        return;
    }

    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii ++)
    {
        // go through the list of supported components and find the component based on its name (identifier)
        if (data->ipRegTemplateList[ii] != NULL)
        {
            if (!oscl_strcmp((data->ipRegTemplateList[ii])->ComponentName, cComponentName))
            {
                // found a matching name. Use this destructor
                OMX_PTR &aOmxLib = data->ipRegTemplateList[ii]->SharedLibraryPtr;
                OMX_PTR aOsclUuid = data->ipRegTemplateList[ii]->SharedLibraryOsclUuid;
                OMX_U32 &aRefCount = data->ipRegTemplateList[ii]->SharedLibraryRefCounter;
                (data->ipRegTemplateList[ii]->FunctionPtrDestroyComponent)(aHandle, aOmxLib, aOsclUuid, aRefCount);
                break;
            }
        }
        else
        {
            break;
        }
    }

    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
}
#endif

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle,
        OMX_IN  OMX_STRING cComponentName,
        OMX_IN  OMX_PTR pAppData,
        OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
{
    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    int32 error;

#if PROXY_INTERFACE
    ProxyApplication_OMX* pTempProxyTerm = new ProxyApplication_OMX;

    if (pTempProxyTerm->GetMemPoolPtr() == NULL)
    {
        _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);
        ErrorType = OMX_ErrorInsufficientResources;
        return ErrorType;
    }
    pTempProxyTerm->Start();

    *pHandle = NULL;

    ErrorType = pTempProxyTerm->ProxyGetHandle(pHandle, cComponentName, pAppData, pCallBacks);

    //Get registry to store values
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (error) // can't access registry
    {
        _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);
        return OMX_ErrorInvalidState;
    }
    else if (!data) // singleton object has been destroyed
    {
        // Unlock registry before calling cleanup otherwise it'll lead to deadlock.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);
        return OMX_ErrorInvalidState;
    }

    if ((NULL != *pHandle) && (OMX_ErrorNone == ErrorType))
    {
        OMX_U32* componentIndex = &(data->iComponentIndex);
        OMX_HANDLETYPE* componentHandle = data->iComponentHandle;

        // First, find an empty slot in the proxy/component handle array to store the component/proxy handle
        OMX_U32 jj;
        for (jj = 0; jj < MAX_INSTANTIATED_COMPONENTS; jj++)
        {
            if (componentHandle[jj] == NULL)
                break;
        }
        // can't find a free slot
        if (jj == MAX_INSTANTIATED_COMPONENTS)
        {
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
            _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);
            ErrorType = OMX_ErrorInsufficientResources;
            return ErrorType;
        }
        else
        {
            *componentIndex = jj;
        }

        ProxyApplication_OMX** pProxyTerm = data->ipProxyTerm;

        pProxyTerm[*componentIndex] = pTempProxyTerm;

        // now that we got the component handle, store the handle in the componentHandle array
        componentHandle[*componentIndex] = *pHandle;

        // record the component destructor function ptr;
        OMX_S32 ii;
        OMX_U8 componentFoundflag = false;

        for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii ++)
        {
            // go through the list of supported components and find the component based on its name (identifier)
            if (data->ipRegTemplateList[ii] != NULL)
            {
                if (!oscl_strcmp((data->ipRegTemplateList[ii])->ComponentName, cComponentName))
                {
                    // found a matching name
                    componentFoundflag = true;
                    // save the Registry Ptr into the array of instantiated components
                    data->ipInstantiatedComponentReg[*componentIndex] = data->ipRegTemplateList[ii];
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (!componentFoundflag)
        {
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
            _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);

            pProxyTerm[*componentIndex] = NULL;
            componentHandle[*componentIndex] = NULL;

            ErrorType = OMX_ErrorComponentNotFound;
            return ErrorType;
        }

        data->iNumBaseInstance++;
        if (data->iNumBaseInstance > MAX_INSTANTIATED_COMPONENTS)
        {
            //Cleanup and unlock registry
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
            _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);

            pProxyTerm[*componentIndex] = NULL;
            componentHandle[*componentIndex] = NULL;
            data->ipInstantiatedComponentReg[*componentIndex] = NULL;

            ErrorType = OMX_ErrorInsufficientResources;
            return ErrorType;
        }
    }
    else
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        _Cleanup_Component(pTempProxyTerm, *pHandle, cComponentName);

        ErrorType = OMX_ErrorUndefined;
        return ErrorType;
    }

#else
    //Get registry to store values
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (error) // can't access registry
    {
        return OMX_ErrorInvalidState;
    }
    else if (!data) // singleton object has been destroyed
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        return OMX_ErrorInvalidState;
    }

    OMX_U32* componentIndex = &(data->iComponentIndex);
    OMX_HANDLETYPE* componentHandle = data->iComponentHandle;

    // First, find an empty slot in the proxy/component handle array to store the component/proxy handle
    OMX_U32 jj;
    for (jj = 0; jj < MAX_INSTANTIATED_COMPONENTS; jj++)
    {
        if (componentHandle[jj] == NULL)
            break;
    }
    // can't find a free slot
    if (jj == MAX_INSTANTIATED_COMPONENTS)
    {
        //Release the singleton.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        ErrorType = OMX_ErrorInsufficientResources;
        return ErrorType;
    }
    else
    {
        *componentIndex = jj;
    }

    OMX_S32 ii;

    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii ++)
    {
        // go through the list of supported components and find the component based on its name (identifier)
        if (data->ipRegTemplateList[ii] != NULL)
        {
            if (!oscl_strcmp((data->ipRegTemplateList[ii])->ComponentName, cComponentName))
            {
                // found a matching name
                // call the factory for the component
                OMX_STRING aOmxLibName = data->ipRegTemplateList[ii]->SharedLibraryName;
                OMX_PTR &aOmxLib = data->ipRegTemplateList[ii]->SharedLibraryPtr;
                OMX_PTR aOsclUuid = data->ipRegTemplateList[ii]->SharedLibraryOsclUuid;
                OMX_U32 &aRefCount = data->ipRegTemplateList[ii]->SharedLibraryRefCounter;
                if ((data->ipRegTemplateList[ii]->FunctionPtrCreateComponent)(pHandle, pAppData, NULL, aOmxLibName, aOmxLib, aOsclUuid, aRefCount) == OMX_ErrorNone)
                {
                    // now that we got the component handle, store the handle in the componentHandle array
                    componentHandle[*componentIndex] = *pHandle;

                    // also, record the component registration info to be able to destroy it
                    data->ipInstantiatedComponentReg[*componentIndex] = (data->ipRegTemplateList[ii]);

                    data->iNumBaseInstance++;

                    if (data->iNumBaseInstance > MAX_INSTANTIATED_COMPONENTS)
                    {
                        //cleanup
                        ((data->ipRegTemplateList[ii])->FunctionPtrDestroyComponent)(componentHandle[*componentIndex], aOmxLib, aOsclUuid, aRefCount);
                        componentHandle[*componentIndex] = NULL;
                        data->ipInstantiatedComponentReg[*componentIndex] = NULL;

                        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
                        ErrorType = OMX_ErrorInsufficientResources;
                        return ErrorType;
                    }

                    ((OMX_COMPONENTTYPE*)*pHandle)->SetCallbacks(*pHandle, pCallBacks, pAppData);
                }
                else
                {
                    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
                    ErrorType = OMX_ErrorInsufficientResources;
                    return ErrorType;
                }
            }

        }
        else
        {
            break;
        }

    }
    // can't find the component after going through all of them
    if (componentHandle[*componentIndex] == NULL)
    {
        ErrorType =  OMX_ErrorComponentNotFound;
    }

#endif

    //Release the singleton
    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
    if (error)
    {
        //registry error
        return OMX_ErrorInvalidState;
    }

    return ErrorType;
}


OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    int32 error;
    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (error) // can't access registry
    {
        return OMX_ErrorInvalidState;
    }
    else if (!data) // singleton object has been destroyed
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        return OMX_ErrorInvalidState;
    }

    OMX_S32 ii, ComponentNumber = 0;

    // Find the component index in the array of handles
    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS ; ii++)
    {
        if (hComponent == data->iComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    // cannot find the component handle
    if (ii == MAX_INSTANTIATED_COMPONENTS)
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        ErrorType = OMX_ErrorInvalidComponent;
        return ErrorType;
    }

#if PROXY_INTERFACE
    ProxyApplication_OMX* pTempProxyTerm = data->ipProxyTerm[ComponentNumber];

    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
    if (error)
    {
        ErrorType = OMX_ErrorUndefined;
        return ErrorType;
    }
    // call the OMX_FreeHandle through the proxy
    ErrorType = pTempProxyTerm->ProxyFreeHandle(hComponent);

    // exit thread
    pTempProxyTerm->Exit();

    data = (OMXGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMX, error);
    if (error) // can't access registry
    {
        return OMX_ErrorInvalidState;
    }
    else if (!data) // singleton object has been destroyed
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
        return OMX_ErrorInvalidState;
    }

    ProxyApplication_OMX** pProxyTerm = data->ipProxyTerm;

    delete pProxyTerm[ComponentNumber];
    // delete array entries associated with pProxyTerm and Component handle
    pProxyTerm[ComponentNumber] = NULL;
    data->iComponentHandle[ComponentNumber] = NULL;
    data->ipInstantiatedComponentReg[ComponentNumber] = NULL;

#else

    // call the component AO destructor through the function pointer
    OMX_PTR &aOmxLib = data->ipInstantiatedComponentReg[ii]->SharedLibraryPtr;
    OMX_PTR aOsclUuid = data->ipInstantiatedComponentReg[ii]->SharedLibraryOsclUuid;
    OMX_U32 &aRefCount = data->ipInstantiatedComponentReg[ii]->SharedLibraryRefCounter;
    ErrorType = (data->ipInstantiatedComponentReg[ComponentNumber]->FunctionPtrDestroyComponent)(hComponent, aOmxLib, aOsclUuid, aRefCount);

    data->iComponentHandle[ComponentNumber] = NULL;
    data->ipInstantiatedComponentReg[ComponentNumber] = NULL;

    data->iNumBaseInstance--;

#endif

    //Release the singleton.
    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
    if (error)
    {
        ErrorType = OMX_ErrorUndefined;
        return ErrorType;
    }
    return ErrorType;

}

//This is a method to be called directly under testapp thread
OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{
    int32 error;

    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMX, error);
    if (!data)
    {
        return OMX_ErrorUndefined;
    }
    OMX_U32 Index = 0;

    while (data->ipRegTemplateList[Index] != NULL)
    {
        if (Index == nIndex)
        {
            break;
        }
        Index++;
    }

    if (data->ipRegTemplateList[Index] != NULL)
    {
        oscl_strncpy(cComponentName, (data->ipRegTemplateList[Index])->ComponentName, nNameLength);
    }
    else
    {
        return OMX_ErrorNoMore;
    }

    return OMX_ErrorNone;

}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_SetupTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput)
{
    OSCL_UNUSED_ARG(hOutput);
    OSCL_UNUSED_ARG(nPortOutput);
    OSCL_UNUSED_ARG(hInput);
    OSCL_UNUSED_ARG(nPortInput);
    return OMX_ErrorNotImplemented;
}



OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI)
{
    OSCL_UNUSED_ARG(hPipe);
    OSCL_UNUSED_ARG(szURI);
    return OMX_ErrorNotImplemented;
}


/////////////////////////////////////////////////////
/////////////// Given a compName, find the component and then return its role(s)
///////////////// It's the caller's responsibility to provide enough space for the role(s)
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetRolesOfComponent(
    OMX_IN      OMX_STRING compName,
    OMX_INOUT   OMX_U32* pNumRoles,
    OMX_OUT     OMX_U8** roles)
{
    int32 error;

    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMX, error);
    if (!data)
    {
        return OMX_ErrorUndefined;
    }

    OMX_STRING RoleString[MAX_ROLES_SUPPORTED];
    OMX_U32 ii;

    // first check if there is a component with the correct name
    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii ++)
    {
        if (data->ipRegTemplateList[ii])
        {
            if (!oscl_strcmp(data->ipRegTemplateList[ii]->ComponentName, compName))
            {
                (data->ipRegTemplateList[ii])->GetRolesOfComponent(RoleString);
                break;
            }
        }
    }

    if (ii == MAX_SUPPORTED_COMPONENTS)
    {
        // component not found
        *pNumRoles = 0;
        return OMX_ErrorInvalidComponent;
    }


    // Return the number of roles supported by the component.
    *pNumRoles = (data->ipRegTemplateList[ii])->NumberOfRolesSupported;
    if (roles != NULL)
    {
        for (ii = 0; ii < *pNumRoles; ii++)
        {
            oscl_strncpy((OMX_STRING) roles[ii], (OMX_STRING)RoleString[ii], oscl_strlen((OMX_STRING)RoleString[ii]) + 1);
        }
    }

    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////
////////// Given a role (say "video_decoder.avc") give the number (and a list) of
///////////components that support the role
/////////// It is the callers responsibility to provide enough space for component names,
//////////// so it may need to make the call twice. Once to find number of components, and 2nd time
//////////// to find their actual names
//////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetComponentsOfRole(
    OMX_IN		OMX_STRING role,
    OMX_INOUT	OMX_U32	*pNumComps,
    OMX_INOUT	OMX_U8	**compNames)
{
    int32 error;

    OMXGlobalData* data = (OMXGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMX, error);
    if (!data)
    {
        return OMX_ErrorUndefined;
    }

    OMX_U32 ii, jj;
    OMX_STRING RoleString[MAX_ROLES_SUPPORTED];
    // initialize
    *pNumComps = 0;

    // go through all components and check if they support the given role
    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii ++)
    {
        if (data->ipRegTemplateList[ii])
        {
            // get the component role
            (data->ipRegTemplateList[ii])->GetRolesOfComponent(RoleString);

            for (jj = 0; jj < (data->ipRegTemplateList[ii])->NumberOfRolesSupported; jj++)
            {
                // if the role matches, increment the counter and record the comp. name
                if (!oscl_strcmp(RoleString[jj], role))
                {
                    // if a placeholder for compNames is provided, copy the component name into it
                    if (compNames != NULL)
                    {
                        oscl_strncpy((OMX_STRING) compNames[*pNumComps], (data->ipRegTemplateList[ii])->ComponentName,
                                     oscl_strlen((data->ipRegTemplateList[ii])->ComponentName) + 1);
                    }
                    // increment the counter
                    *pNumComps = (*pNumComps + 1);
                }

            }
        }
    }

    return OMX_ErrorNone;

}

