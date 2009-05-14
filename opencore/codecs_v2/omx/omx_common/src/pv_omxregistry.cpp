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

#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif

#include "OMX_Component.h"
#include "pv_omxcore.h"

// pv_omxregistry.h is only needed if NOT using CML2
#ifndef USE_CML2_CONFIG
#include "pv_omxregistry.h"
#endif


// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif


#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
OMX_ERRORTYPE OmxComponentFactoryDynamicCreate(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
OMX_ERRORTYPE OmxComponentFactoryDynamicDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif


OMX_ERRORTYPE ComponentRegister(ComponentRegistrationType *pCRT)
{
    int32 error;
    OMX_S32 ii;

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

    for (ii = 0; ii < MAX_SUPPORTED_COMPONENTS; ii++)
    {
        if (NULL == data->ipRegTemplateList[ii])
        {
            data->ipRegTemplateList[ii] = pCRT;
            break;
        }
    }

    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMX, error);
    if (error)
    {
        //registry error
        return OMX_ErrorUndefined;
    }

    if (MAX_SUPPORTED_COMPONENTS == ii)
    {
        return OMX_ErrorInsufficientResources;
    }

    return OMX_ErrorNone;
}

#if REGISTER_OMX_M4V_COMPONENT
#if (DYNAMIC_LOAD_OMX_M4V_COMPONENT == 0)
// external factory functions needed for creation of each component (or stubs for testing)
extern OMX_ERRORTYPE Mpeg4OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE Mpeg4OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif

#if (REGISTER_OMX_M4V_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Mpeg4Register()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.mpeg4dec";
        pCRT->RoleString[0] = (OMX_STRING)"video_decoder.mpeg4";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_m4vdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_M4VDEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_M4V_COMPONENT
#if (DYNAMIC_LOAD_OMX_M4V_COMPONENT == 0)
        pCRT->FunctionPtrCreateComponent = &Mpeg4OmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &Mpeg4OmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif

    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}


#endif
//////////////////////////////////////////////////////////////////////////////

#if REGISTER_OMX_H263_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_H263_COMPONENT == 0)
extern OMX_ERRORTYPE H263OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE H263OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_H263_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE H263Register()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.h263dec";
        pCRT->RoleString[0] = (OMX_STRING)"video_decoder.h263";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_m4vdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_H263DEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_H263_COMPONENT
#if (DYNAMIC_LOAD_OMX_H263_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &H263OmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &H263OmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif
////////////////////////////////////////////////////////////////////////////////////

#if REGISTER_OMX_AVC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AVC_COMPONENT == 0)
extern OMX_ERRORTYPE AvcOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AvcOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AVC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AvcRegister()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.avcdec";
        pCRT->RoleString[0] = (OMX_STRING)"video_decoder.avc";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_avcdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AVCDEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_AVC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AVC_COMPONENT == 0)
        pCRT->FunctionPtrCreateComponent = &AvcOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AvcOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif
////////////////////////////////////////////////////////////////////////////////////

#if REGISTER_OMX_WMV_COMPONENT
#if (DYNAMIC_LOAD_OMX_WMV_COMPONENT == 0)
extern OMX_ERRORTYPE WmvOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE WmvOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_WMV_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE WmvRegister()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.wmvdec";
        pCRT->RoleString[0] = (OMX_STRING)"video_decoder.wmv";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_wmvdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_WMVDEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_WMV_COMPONENT
#if (DYNAMIC_LOAD_OMX_WMV_COMPONENT == 0)
        pCRT->FunctionPtrCreateComponent = &WmvOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &WmvOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_AAC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_AAC_COMPONENT == 0)
extern OMX_ERRORTYPE AacOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AacOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AAC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AacRegister()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.aacdec";
        pCRT->RoleString[0] = (OMX_STRING)"audio_decoder.aac";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_aacdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AACDEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_AAC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AAC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &AacOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AacOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_AMR_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_AMR_COMPONENT == 0)
extern OMX_ERRORTYPE AmrOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AmrOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AMR_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AmrRegister()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.amrdec";
        pCRT->RoleString[0] = (OMX_STRING)"audio_decoder.amr";
        pCRT->RoleString[1] = (OMX_STRING)"audio_decoder.amrnb";
        pCRT->RoleString[2] = (OMX_STRING)"audio_decoder.amrwb";
        pCRT->NumberOfRolesSupported = 3;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_amrdec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AMRDEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;

#endif
#if REGISTER_OMX_AMR_COMPONENT
#if (DYNAMIC_LOAD_OMX_AMR_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &AmrOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AmrOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_MP3_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_MP3_COMPONENT == 0)
extern OMX_ERRORTYPE Mp3OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE Mp3OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_MP3_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Mp3Register()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.mp3dec";
        pCRT->RoleString[0] = (OMX_STRING)"audio_decoder.mp3";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_mp3dec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_MP3DEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_MP3_COMPONENT
#if (DYNAMIC_LOAD_OMX_MP3_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &Mp3OmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &Mp3OmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_WMA_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_WMA_COMPONENT == 0)
extern OMX_ERRORTYPE WmaOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE WmaOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_WMA_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE WmaRegister()
{
    ComponentRegistrationType *pCRT = (ComponentRegistrationType *) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.wmadec";
        pCRT->RoleString[0] = (OMX_STRING)"audio_decoder.wma";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_wmadec_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_WMADEC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_WMA_COMPONENT
#if (DYNAMIC_LOAD_OMX_WMA_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &WmaOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &WmaOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_AMRENC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_AMRENC_COMPONENT == 0)
extern OMX_ERRORTYPE AmrEncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AmrEncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AMRENC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AmrEncRegister()
{
    ComponentRegistrationType* pCRT = (ComponentRegistrationType*) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.amrencnb";
        pCRT->RoleString[0] = (OMX_STRING)"audio_encoder.amrnb";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_amrenc_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AMRENC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#if REGISTER_OMX_AMRENC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AMRENC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &AmrEncOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AmrEncOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_M4VENC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_M4VENC_COMPONENT == 0)
extern OMX_ERRORTYPE Mpeg4EncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE Mpeg4EncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_M4VENC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Mpeg4EncRegister()
{
    ComponentRegistrationType* pCRT = (ComponentRegistrationType*) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.mpeg4enc";
        pCRT->RoleString[0] = (OMX_STRING)"video_encoder.mpeg4";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_m4venc_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_M4VENC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;

#endif
#if REGISTER_OMX_M4VENC_COMPONENT
#if (DYNAMIC_LOAD_OMX_M4VENC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &Mpeg4EncOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &Mpeg4EncOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

#if REGISTER_OMX_H263ENC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_H263ENC_COMPONENT == 0)
extern OMX_ERRORTYPE H263EncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE H263EncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_H263ENC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE H263EncRegister()
{
    ComponentRegistrationType* pCRT = (ComponentRegistrationType*) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.h263enc";
        pCRT->RoleString[0] = (OMX_STRING)"video_encoder.h263";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_m4venc_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_H263ENC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;

#endif
#if REGISTER_OMX_H263ENC_COMPONENT
#if (DYNAMIC_LOAD_OMX_H263ENC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &H263EncOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &H263EncOmxComponentDestructor;
        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_AVCENC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_AVCENC_COMPONENT == 0)
extern OMX_ERRORTYPE AvcEncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AvcEncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AVCENC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AvcEncRegister()
{
    ComponentRegistrationType* pCRT = (ComponentRegistrationType*) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.avcenc";
        pCRT->RoleString[0] = (OMX_STRING)"video_encoder.avc";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_avcenc_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AVCENC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;

#endif
#if REGISTER_OMX_AVCENC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AVCENC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &AvcEncOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AvcEncOmxComponentDestructor;

        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#if REGISTER_OMX_AACENC_COMPONENT
// external factory functions needed for creation of each component (or stubs for testing)
#if (DYNAMIC_LOAD_OMX_AACENC_COMPONENT == 0)
extern OMX_ERRORTYPE AacEncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
extern OMX_ERRORTYPE AacEncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
#endif
#endif
#if (REGISTER_OMX_AACENC_COMPONENT) || (USE_DYNAMIC_LOAD_OMX_COMPONENTS)
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE AacEncRegister()
{
    ComponentRegistrationType* pCRT = (ComponentRegistrationType*) oscl_malloc(sizeof(ComponentRegistrationType));

    if (pCRT)
    {
        pCRT->ComponentName = (OMX_STRING)"OMX.PV.aacenc";
        pCRT->RoleString[0] = (OMX_STRING)"audio_encoder.aac";
        pCRT->NumberOfRolesSupported = 1;
        pCRT->SharedLibraryOsclUuid = NULL;
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
        pCRT->FunctionPtrCreateComponent = &OmxComponentFactoryDynamicCreate;
        pCRT->FunctionPtrDestroyComponent = &OmxComponentFactoryDynamicDestructor;
        pCRT->SharedLibraryName = (OMX_STRING)"libomx_aacenc_sharedlibrary.so";
        pCRT->SharedLibraryPtr = NULL;

        OsclUuid *temp = (OsclUuid *) oscl_malloc(sizeof(OsclUuid));
        if (temp == NULL)
        {
            oscl_free(pCRT); // free allocated memory
            return OMX_ErrorInsufficientResources;
        }
        OSCL_PLACEMENT_NEW(temp, PV_OMX_AACENC_UUID);

        pCRT->SharedLibraryOsclUuid = (OMX_PTR) temp;
        pCRT->SharedLibraryRefCounter = 0;

#endif
#if REGISTER_OMX_AACENC_COMPONENT
#if (DYNAMIC_LOAD_OMX_AACENC_COMPONENT == 0)

        pCRT->FunctionPtrCreateComponent = &AacEncOmxComponentFactory;
        pCRT->FunctionPtrDestroyComponent = &AacEncOmxComponentDestructor;

        pCRT->SharedLibraryName = NULL;
        pCRT->SharedLibraryPtr = NULL;

        if (pCRT->SharedLibraryOsclUuid)
            oscl_free(pCRT->SharedLibraryOsclUuid);

        pCRT->SharedLibraryOsclUuid = NULL;
        pCRT->SharedLibraryRefCounter = 0;
#endif
#endif
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return  ComponentRegister(pCRT);
}
#endif


// In case of dynamic loading of individual omx components,
// this function is called by OMX_GetHandle (either through proxy or directly).
// The method dynamically loads the library and creates an instance of the omx component AO
// NOTE: This method is called when singleton is locked. Access & modification of various
// variables should be (and is) thread-safe
#if USE_DYNAMIC_LOAD_OMX_COMPONENTS

OMX_ERRORTYPE OmxComponentFactoryDynamicCreate(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OMX_ERRORTYPE returnStatus = OMX_ErrorUndefined;

    //OSCL_StackString<M4V_MAX_LIB_PATH> omxLibName(OMX_M4V_LIB_NAME);

    OsclSharedLibrary* lib = NULL;

    // If aOmxLib is NULL, this is the first time this method has been called
    if (NULL == aOmxLib)
    {
        OSCL_StackString<OMX_MAX_LIB_PATH> Libname(aOmxLibName);
        lib = OSCL_NEW(OsclSharedLibrary, (Libname));
    }
    else
    {
        lib = (OsclSharedLibrary *) aOmxLib;
    }

    // Keep track of the number of times OmxLib is accessed
    aRefCount++;

    // Load the associated library. If successful, call the corresponding
    // create function located inside the loaded library

    if (OsclLibSuccess == lib->LoadLib())
    {
        // look for the interface

        OsclAny* interfacePtr = NULL;
        if (OsclLibSuccess == lib->QueryInterface(PV_OMX_SHARED_INTERFACE, (OsclAny*&)interfacePtr))
        {


            // the interface ptr should be ok, but check just in case
            if (interfacePtr != NULL)
            {
                OmxSharedLibraryInterface* omxIntPtr =
                    OSCL_DYNAMIC_CAST(OmxSharedLibraryInterface*, interfacePtr);


                OsclUuid *temp = (OsclUuid*) aOsclUuid;
                OsclAny* createCompTemp =
                    omxIntPtr->QueryOmxComponentInterface(*temp, PV_OMX_CREATE_INTERFACE);

                // check if the component contains the correct ptr
                if (createCompTemp != NULL)
                {

                    // createComp is the function pointer to store the creation function
                    // for the omx component located inside the loaded library
                    OMX_ERRORTYPE(*createComp)(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);

                    createComp = OSCL_DYNAMIC_CAST(OMX_ERRORTYPE(*)(OMX_OUT OMX_HANDLETYPE * pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR , OMX_STRING, OMX_PTR &, OMX_PTR , OMX_U32 &), createCompTemp);

                    // call the component AO factory inside the loaded library
                    returnStatus = (*createComp)(pHandle, pAppData, pProxy, aOmxLibName, aOmxLib, aOsclUuid, aRefCount);

                    // Store the shared library so it can be closed later
                    aOmxLib = (OMX_PTR) lib;
                }
            }
        }
    }

    // if everything is OK, the AO factory should have returned OMX_ErrorNone
    if (returnStatus != OMX_ErrorNone)
    {
        lib->Close();

        // If this is the last time to close the library, delete the
        // OsclSharedLibrary object and be sure to set aOmxLib back to NULL
        aRefCount--;
        if (0 == aRefCount)
        {
            OSCL_DELETE(lib);
            aOmxLib = NULL;
        }
    }
    return returnStatus;
}

OMX_ERRORTYPE OmxComponentFactoryDynamicDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OsclSharedLibrary* lib = (OsclSharedLibrary *) aOmxLib;
    OMX_ERRORTYPE returnStatus = OMX_ErrorUndefined;

    // lib must not be NULL at this point. If the omx component has been
    // created, lib is necessary to destroy the component correctly and
    // cleanup used memory.
    OSCL_ASSERT(NULL != lib);
    if (lib == NULL)
    {
        return returnStatus;
    }

    // Need to get the function pointer for the destroy function through the
    // shared library interface.

    // first, try to get the interface
    OsclAny* interfacePtr = NULL;
    if (OsclLibSuccess == lib->QueryInterface(PV_OMX_SHARED_INTERFACE, (OsclAny*&)interfacePtr))
    {

        if (interfacePtr != NULL)
        {
            OmxSharedLibraryInterface* omxIntPtr =
                OSCL_DYNAMIC_CAST(OmxSharedLibraryInterface*, interfacePtr);

            // try to get the function ptr to the omx component AO destructor inside the loaded library
            OsclUuid *temp = (OsclUuid*) aOsclUuid;
            OsclAny* destroyCompTemp =
                omxIntPtr->QueryOmxComponentInterface(*temp, PV_OMX_DESTROY_INTERFACE);

            if (destroyCompTemp != NULL)
            {
                // destroyComp is the function pointer to store the function for
                // destroying the omx component AO
                OMX_ERRORTYPE(*destroyComp)(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
                destroyComp = OSCL_DYNAMIC_CAST(OMX_ERRORTYPE(*)(OMX_IN OMX_HANDLETYPE, OMX_PTR &, OMX_PTR, OMX_U32 &), destroyCompTemp);

                // call the destructor through the function ptr
                returnStatus = (*destroyComp)(pHandle, aOmxLib, aOsclUuid, aRefCount);
            }
        }
    }

    //Whatever the outcome of the interface queries, this needs to be done
    // Finish memory cleanup by closing the shared library and deleting
    lib->Close();

    // If this is the last time to close the library, delete the
    // OsclSharedLibrary object and be sure to set iOmxLib back to NULL
    aRefCount--;
    if (0 == aRefCount)
    {
        OSCL_DELETE(lib);
        aOmxLib = NULL;
    }

    return returnStatus;
}

#endif // USE_DYNAMIC_LOAD_OMX_COMPONENTS

