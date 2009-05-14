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
#ifndef PV_OMXCORE_H_INCLUDED
#define PV_OMXCORE_H_INCLUDED

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef PV_OMX_QUEUE_H_INCLUDED
#include "pv_omx_queue.h"
#endif

#ifndef OMX_Types_h
#include "OMX_Types.h"
#endif

#ifndef OSCL_BASE_INCLUDED_H
#include "oscl_base.h"
#endif

#ifndef OSCL_UUID_H_INCLUDED
#include "oscl_uuid.h"
#endif


#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#if PROXY_INTERFACE
#ifndef OMX_PROXY_INTERFACE_H_INCLUDED
#include "omx_proxy_interface.h"
#endif
#endif

#ifndef USE_CML2_CONFIG


#ifdef ANDROID

// NOTE: if at least one component uses dynamic loading,
// USE_DYNAMIC_LOAD_OMX_COMPONENT needs to be 1
#define USE_DYNAMIC_LOAD_OMX_COMPONENTS 0

#define DYNAMIC_LOAD_OMX_AVC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_M4V_COMPONENT 0
#define DYNAMIC_LOAD_OMX_H263_COMPONENT 0
#define DYNAMIC_LOAD_OMX_WMV_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AAC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AMR_COMPONENT 0
#define DYNAMIC_LOAD_OMX_MP3_COMPONENT 0
#define DYNAMIC_LOAD_OMX_WMA_COMPONENT 0

#define DYNAMIC_LOAD_OMX_AMRENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_M4VENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_H263ENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AVCENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AACENC_COMPONENT 0

#else

#define USE_DYNAMIC_LOAD_OMX_COMPONENTS 0

#define DYNAMIC_LOAD_OMX_AVC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_M4V_COMPONENT 0
#define DYNAMIC_LOAD_OMX_H263_COMPONENT 0
#define DYNAMIC_LOAD_OMX_WMV_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AAC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AMR_COMPONENT 0
#define DYNAMIC_LOAD_OMX_MP3_COMPONENT 0
#define DYNAMIC_LOAD_OMX_WMA_COMPONENT 0

#define DYNAMIC_LOAD_OMX_AMRENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_M4VENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_H263ENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AVCENC_COMPONENT 0
#define DYNAMIC_LOAD_OMX_AACENC_COMPONENT 0

#endif
#endif

#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
#ifndef OSCL_SHARED_LIBRARY_H_INCLUDED
#include "oscl_shared_library.h"
#endif
#endif

#define MAX_ROLES_SUPPORTED	3

#ifdef __cplusplus
extern "C"
{
#endif

    OSCL_IMPORT_REF OMX_ERRORTYPE OMX_GetComponentsOfRole(
        OMX_IN		OMX_STRING role,
        OMX_INOUT	OMX_U32	*pNumComps,
        OMX_INOUT	OMX_U8	**compNames);

    OSCL_IMPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
        OMX_OUT OMX_STRING cComponentName,
        OMX_IN  OMX_U32 nNameLength,
        OMX_IN  OMX_U32 nIndex);

    OSCL_IMPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent);

    OSCL_IMPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle,
            OMX_IN  OMX_STRING cComponentName,
            OMX_IN  OMX_PTR pAppData,
            OMX_IN  OMX_CALLBACKTYPE* pCallBacks);

    OSCL_IMPORT_REF OMX_ERRORTYPE OMX_GetRolesOfComponent(
        OMX_IN      OMX_STRING compName,
        OMX_INOUT   OMX_U32* pNumRoles,
        OMX_OUT     OMX_U8** roles);

    OSCL_IMPORT_REF  OMX_ERRORTYPE OMX_SetupTunnel(
        OMX_IN  OMX_HANDLETYPE hOutput,
        OMX_IN  OMX_U32 nPortOutput,
        OMX_IN  OMX_HANDLETYPE hInput,
        OMX_IN  OMX_U32 nPortInput);

    OSCL_IMPORT_REF OMX_ERRORTYPE   OMX_GetContentPipe(
        OMX_OUT  OMX_HANDLETYPE *hPipe,
        OMX_IN   OMX_STRING szURI);

    OSCL_IMPORT_REF OMX_BOOL OMXConfigParser(
        OMX_PTR aInputParameters,
        OMX_PTR aOutputParameters);


#ifdef __cplusplus
}
#endif



#if USE_DYNAMIC_LOAD_OMX_COMPONENTS
//Dynamic loading interface definitions
#define PV_OMX_SHARED_INTERFACE OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x67)
#define PV_OMX_CREATE_INTERFACE OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x68)
#define PV_OMX_DESTROY_INTERFACE OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x69)
#define PV_OMX_AVCDEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6a)
#define PV_OMX_M4VDEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6b)
#define PV_OMX_H263DEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6c)
#define PV_OMX_WMVDEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6d)
#define PV_OMX_AACDEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6e)
#define PV_OMX_AMRDEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x6f)
#define PV_OMX_MP3DEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x70)
#define PV_OMX_WMADEC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x71)
#define PV_OMX_AVCENC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x72)
#define PV_OMX_M4VENC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x73)
#define PV_OMX_H263ENC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x74)
#define PV_OMX_AMRENC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x75)
#define PV_OMX_AACENC_UUID OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x76)

#define OMX_MAX_LIB_PATH 256

class OmxSharedLibraryInterface
{
    public:
        virtual OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId) = 0;
};
#endif // USE_DYNAMIC_LOAD_OMX_COMPONENTS

// PV additions to OMX_EXTRADATATYPE enum
#define OMX_ExtraDataNALSizeArray 0x7F123321 // random value above 0x7F000000 (start of the unused range for vendors)

class ComponentRegistrationType
{
    public:
        // name of the component used as identifier
        OMX_STRING		ComponentName;
        OMX_STRING		RoleString[MAX_ROLES_SUPPORTED];
        OMX_U32			NumberOfRolesSupported;
        // pointer to factory function to be called when component needs to be instantiated
        OMX_ERRORTYPE(*FunctionPtrCreateComponent)(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData,
                OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
        // pointer to function that destroys the component and its AO
        OMX_ERRORTYPE(*FunctionPtrDestroyComponent)(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);
        //This function will return the role string
        void GetRolesOfComponent(OMX_STRING* aRole_string)
        {
            for (OMX_U32 ii = 0; ii < NumberOfRolesSupported; ii++)
            {
                aRole_string[ii] = RoleString[ii];
            }
        }

        // for dynamic loading
        OMX_STRING             SharedLibraryName;
        OMX_PTR                SharedLibraryPtr;
        OMX_PTR                SharedLibraryOsclUuid;
        OMX_U32                SharedLibraryRefCounter;

};

typedef struct CoreDescriptorType
{
    QueueType* pMessageQueue; // The output queue for the messages to be send to the components
} CoreDescriptorType;


/** This structure contains all the fields of a message handled by the core */
struct CoreMessage
{
    OMX_COMPONENTTYPE* pComponent; /// A reference to the main structure that defines a component. It represents the target of the message
    OMX_S32 MessageType; /// the flag that specifies if the message is a command, a warning or an error
    OMX_S32 MessageParam1; /// the first field of the message. Its use is the same as specified for the command in OpenMAX spec
    OMX_S32 MessageParam2; /// the second field of the message. Its use is the same as specified for the command in OpenMAX spec
    OMX_PTR pCmdData; /// This pointer could contain some proprietary data not covered by the standard
};

typedef struct PV_OMXComponentCapabilityFlagsType
{
    ////////////////// OMX COMPONENT CAPABILITY RELATED MEMBERS
    OMX_BOOL iIsOMXComponentMultiThreaded;
    OMX_BOOL iOMXComponentSupportsExternalOutputBufferAlloc;
    OMX_BOOL iOMXComponentSupportsExternalInputBufferAlloc;
    OMX_BOOL iOMXComponentSupportsMovableInputBuffers;
    OMX_BOOL iOMXComponentSupportsPartialFrames;
    OMX_BOOL iOMXComponentUsesNALStartCodes;
    OMX_BOOL iOMXComponentCanHandleIncompleteFrames;
    OMX_BOOL iOMXComponentUsesFullAVCFrames;

} PV_OMXComponentCapabilityFlagsType;

class OMXGlobalData
{
    public:
        OMXGlobalData()
                : iInstanceCount(1),
                iOsclInit(false),
                iNumBaseInstance(0),
                iComponentIndex(0)
        {
            for (OMX_S32 ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
            {
                ipInstantiatedComponentReg[ii] = NULL;
            }
        }

        uint32 iInstanceCount;

        bool iOsclInit; //did we do OsclInit in OMX_Init?  if so we must cleanup in OMX_Deinit.


        //Number of base instances
        OMX_U32 iNumBaseInstance;

        // Array to store component handles for future recognition of components etc.
        OMX_HANDLETYPE iComponentHandle[MAX_INSTANTIATED_COMPONENTS];
        OMX_U32 iComponentIndex;

        // Array of supported component types (e.g. MP4, AVC, AAC, etc.)
        // they need to be registered
        // For each OMX Component type (e.g. Mp3, AVC, AAC) there is one entry in this table that contains info
        // such as component type, factory, destructor functions, library name for dynamic loading etc.
        // when the omx component is registered (at OMX_Init)
        ComponentRegistrationType* ipRegTemplateList[MAX_SUPPORTED_COMPONENTS];


        // Array of pointers - For each OMX component that gets instantiated - the pointer to its registry structure
        // is saved here. This information is needed when the component is to be destroyed

        ComponentRegistrationType* ipInstantiatedComponentReg[MAX_INSTANTIATED_COMPONENTS];
        // array of function pointers. For each component, a destructor function is assigned
        //OMX_ERRORTYPE(*ComponentDestructor[MAX_INSTANTIATED_COMPONENTS])(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount);

#if PROXY_INTERFACE
        ProxyApplication_OMX* ipProxyTerm[MAX_INSTANTIATED_COMPONENTS];
#endif

};


#endif
