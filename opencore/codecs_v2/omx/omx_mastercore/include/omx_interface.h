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
#ifndef OMX_INTERFACE_H_INCLUDED
#define OMX_INTERFACE_H_INCLUDED



// get definitions of omx core methods
#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OSCL_SHARED_LIB_INTERFACE_H_INCLUDED
#include "oscl_shared_lib_interface.h"
#endif


// ID used to identify OMX shared library interface
#define OMX_INTERFACE_ID OsclUuid(0xa054369c,0x22c5,0x412e,0x19,0x17,0x87,0x4c,0x1a,0x19,0xd4,0x5f)


// ptrs to all omx core methods
// will be set in the derived classes
typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_Init)(void);

typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_Deinit)(void);

typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_ComponentNameEnum)(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex);


typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_GetHandle)(
    OMX_OUT OMX_HANDLETYPE* pHandle,
    OMX_IN  OMX_STRING cComponentName,
    OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks);


typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_FreeHandle)(
    OMX_IN  OMX_HANDLETYPE hComponent);

typedef OMX_ERRORTYPE(*tpOMX_GetComponentsOfRole)(
    OMX_IN      OMX_STRING role,
    OMX_INOUT   OMX_U32 *pNumComps,
    OMX_INOUT   OMX_U8  **compNames);


typedef OMX_ERRORTYPE(*tpOMX_GetRolesOfComponent)(
    OMX_IN      OMX_STRING compName,
    OMX_INOUT   OMX_U32 *pNumRoles,
    OMX_OUT     OMX_U8 **roles);


typedef OMX_ERRORTYPE OMX_APIENTRY(*tpOMX_SetupTunnel)(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput);


typedef OMX_ERRORTYPE(*tpOMX_GetContentPipe)(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI);

typedef OMX_BOOL(*tpOMXConfigParser)(
    OMX_PTR aInputParameters,
    OMX_PTR aOutputParameters);


class OMXInterface : public OsclSharedLibraryInterface
{
    public:
        OMXInterface()
        {
            pOMX_Init = NULL;
            pOMX_Deinit = NULL;
            pOMX_ComponentNameEnum = NULL;
            pOMX_GetHandle = NULL;
            pOMX_FreeHandle = NULL;
            pOMX_GetComponentsOfRole = NULL;
            pOMX_GetRolesOfComponent = NULL;
            pOMX_SetupTunnel = NULL;
            pOMX_GetContentPipe = NULL;
            pOMXConfigParser = NULL;
        };

        /* return true when the core library can be safely unloaded */
        /* and reloaded at runtime                                  */
        virtual bool UnloadWhenNotUsed(void) = 0;

        tpOMX_Init GetpOMX_Init()
        {
            return pOMX_Init;
        };

        tpOMX_Deinit GetpOMX_Deinit()
        {
            return  pOMX_Deinit;
        };

        tpOMX_ComponentNameEnum GetpOMX_ComponentNameEnum()
        {
            return pOMX_ComponentNameEnum;
        };

        tpOMX_GetHandle GetpOMX_GetHandle()
        {
            return pOMX_GetHandle;
        };

        tpOMX_FreeHandle GetpOMX_FreeHandle()
        {
            return pOMX_FreeHandle;
        };

        tpOMX_GetComponentsOfRole GetpOMX_GetComponentsOfRole()
        {
            return pOMX_GetComponentsOfRole;
        };

        tpOMX_GetRolesOfComponent GetpOMX_GetRolesOfComponent()
        {
            return pOMX_GetRolesOfComponent;
        };

        tpOMX_SetupTunnel GetpOMX_SetupTunnel()
        {
            return pOMX_SetupTunnel;
        };

        tpOMX_GetContentPipe GetpOMX_GetContentPipe()
        {
            return pOMX_GetContentPipe;
        };

        tpOMXConfigParser GetpOMXConfigParser()
        {
            return pOMXConfigParser;
        };

        // ptrs to all omx core methods
        // will be set in the derived classes
        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_Init)(void);

        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_Deinit)(void);

        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_ComponentNameEnum)(
            OMX_OUT OMX_STRING cComponentName,
            OMX_IN  OMX_U32 nNameLength,
            OMX_IN  OMX_U32 nIndex);


        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_GetHandle)(
            OMX_OUT OMX_HANDLETYPE* pHandle,
            OMX_IN  OMX_STRING cComponentName,
            OMX_IN  OMX_PTR pAppData,
            OMX_IN  OMX_CALLBACKTYPE* pCallBacks);


        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_FreeHandle)(
            OMX_IN  OMX_HANDLETYPE hComponent);

        OMX_ERRORTYPE(*pOMX_GetComponentsOfRole)(
            OMX_IN      OMX_STRING role,
            OMX_INOUT   OMX_U32 *pNumComps,
            OMX_INOUT   OMX_U8  **compNames);


        OMX_ERRORTYPE(*pOMX_GetRolesOfComponent)(
            OMX_IN      OMX_STRING compName,
            OMX_INOUT   OMX_U32 *pNumRoles,
            OMX_OUT     OMX_U8 **roles);


        OMX_ERRORTYPE OMX_APIENTRY(*pOMX_SetupTunnel)(
            OMX_IN  OMX_HANDLETYPE hOutput,
            OMX_IN  OMX_U32 nPortOutput,
            OMX_IN  OMX_HANDLETYPE hInput,
            OMX_IN  OMX_U32 nPortInput);


        OMX_ERRORTYPE(*pOMX_GetContentPipe)(
            OMX_OUT OMX_HANDLETYPE *hPipe,
            OMX_IN OMX_STRING szURI);

        OMX_BOOL(*pOMXConfigParser)(
            OMX_PTR aInputParameters,
            OMX_PTR aOutputParameters);
};
#endif

