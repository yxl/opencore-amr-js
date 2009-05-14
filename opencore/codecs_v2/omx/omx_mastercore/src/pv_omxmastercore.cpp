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
#include "pv_omxdefs.h"
#include "OMX_Component.h"
#include "OMX_Core.h"
#include "omx_interface.h"

#include "pvlogger.h"
#include "oscl_base.h"
#include "oscl_file_find.h"
#include "oscl_library_list.h"
#include "oscl_shared_library.h"
#include "oscl_configfile_list.h"
#include "osclconfig_lib.h"
#include "oscl_string.h"
#include "oscl_mem.h"
#include "oscl_stdstring.h"
#include "pv_omx_config_parser.h"

#define MAX_PATHNAME_LENGTH 512

#ifndef USE_CML2_CONFIG
#define MAX_NUMBER_OF_OMX_CORES 10
#define MAX_NUMBER_OF_OMX_COMPONENTS 50
#endif /* USE_CML2_CONFIG */


typedef class OMXMasterCoreGlobalData
{
    public:
        OMXMasterCoreGlobalData()
                : iNumMasterOMXInstances(1),
                iInterface(NULL),
                iOMXLibrary(NULL),
                iMasterRegistry(NULL),
                iOMXCompHandles(NULL),
                iTotalNumOMXComponents(0),
                iNumOMXCores(0)
        {

        };
        //Counter of number of instances - so that init and deinit is done only once
        OMX_U32 iNumMasterOMXInstances;

        // Array of interfaces
        void *iInterface;

        // ptr to array of ptrs to libraries
        void *iOMXLibrary;

        // ptr to registry of all omx components from various cores
        void *iMasterRegistry;

        // ptr to array of component handles (when a component is instantiated - needed to be able to free the component)
        void *iOMXCompHandles;

        // total number of all componenents from various cores
        OMX_U32 iTotalNumOMXComponents;

        // number of omx cores from different vendors
        OMX_U32 iNumOMXCores;

} OMXMasterCoreGlobalData;

typedef struct PVOMXMasterRegistryStruct
{
    OMX_U8 CompName[PV_OMX_MAX_COMPONENT_NAME_LENGTH];
    OMX_U8 CompRole[PV_OMX_MAX_COMPONENT_NAME_LENGTH];
    OMX_U32 OMXCoreIndex;
    OMX_U32 CompIndex;
} PVOMXMasterRegistryStruct;

typedef struct PVOMXCompHandles
{
    OMX_HANDLETYPE handle;
    OMX_U32 OMXCoreIndex;
} PVOMXCompHandles;


// helper function to obtain the registry index for a given handle
// NOTE: 2 or more instances of the player/author are using this same OmxCompHandles array
// However,the component handles recorded into the array or deleted from it by one instance/thread don't
// affect other handles in other instances. In other words, for most purposes - it is safe to search through the array without
// actually locking it - even if another
// instance of the player/author modifies this array (by adding or deleting its own component handles) in the meanwhile.
OMX_ERRORTYPE GetRegIndexForHandle(OMX_HANDLETYPE hComponent, OMX_U32 &index, OMXMasterCoreGlobalData *data)
{
    // we need to first find the handle among instantiated components
    // then we retrieve the core based on component handle

    // locking and unlocking of global data takes place outside of this method if necessary
    PVOMXCompHandles *pOMXCompHandles = (PVOMXCompHandles*)(data->iOMXCompHandles);
    if (pOMXCompHandles == NULL)
    {
        return OMX_ErrorComponentNotFound;
    }

    for (index = 0; index < MAX_NUMBER_OF_OMX_COMPONENTS; index++)
    {
        // go through the list of supported components and find the component handle
        if (pOMXCompHandles[index].handle == hComponent)
        {
            // found a matching handle
            break;
        }
    }

    if (index == MAX_NUMBER_OF_OMX_COMPONENTS)
    {
        // could not find a component with the given name
        return OMX_ErrorComponentNotFound;
    }

    return OMX_ErrorNone;
}

// ALL the standard OpenMAX IL core functions are implemented below
static OMX_ERRORTYPE _OMX_Init(OMXMasterCoreGlobalData *data)
{
    OMX_ERRORTYPE Status = OMX_ErrorNone;
    OMX_U32 jj;
    OMX_U32 index;
    OMX_U32 master_index = 0;
    OMX_U32 component_index = 0;

    /*
    ** Step 1. Populate all the config files present in the specified path.
    ** Step 2. Populate all the libraries from the .cfg files that claim to support the OMX_INTERFACE_ID.
    ** Step 3. For these libraries, validate whether they really support the ID by doing a QueryInterface().
    */

    // Step 1
    OsclConfigFileList aCfgList;
    OSCL_HeapString<OsclMemAllocator> configFilePath = PV_DYNAMIC_LOADING_CONFIG_FILE_PATH;
    aCfgList.Populate(configFilePath, OsclConfigFileList::ESortByName);

    // array of ptrs to various cores, one for every valid configuration
    OMXInterface** pInterface = (OMXInterface**)OSCL_MALLOC(aCfgList.Size() * sizeof(OMXInterface *));
    if (pInterface == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }
    // set the global ptr to this array
    data->iInterface = (void*)pInterface;

    // array of ptrs to the omx shared libraries, one for every valid configuration
    OsclSharedLibrary** pLibrary = (OsclSharedLibrary**)OSCL_MALLOC(aCfgList.Size() * sizeof(OsclSharedLibrary *));
    if (pLibrary == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }
    // set the global ptr to this array
    //
    data->iOMXLibrary = (void*)pLibrary;

    for (uint ii = 0; ii < aCfgList.Size(); ii++)
    {
        // Step 2
        OsclLibraryList libList;
        libList.Populate(OMX_INTERFACE_ID, aCfgList.GetConfigfileAt(ii));

        for (uint jj = 0; (jj < libList.Size()) && ((data->iNumOMXCores) < aCfgList.Size()); jj++)
        {
            OsclSharedLibrary* lib = OSCL_NEW(OsclSharedLibrary, ());
            if (lib->LoadLib(libList.GetLibraryPathAt(jj)) == OsclLibSuccess)
            {
                OsclAny* interfacePtr = NULL;
                // Step 3
                OsclLibStatus result = lib->QueryInterface(OMX_INTERFACE_ID, (OsclAny*&)interfacePtr);
                if (result == OsclLibSuccess && interfacePtr != NULL)
                {
                    pLibrary[(data->iNumOMXCores)] = lib;
                    OMXInterface* coreIntPtr = OSCL_DYNAMIC_CAST(OMXInterface*, interfacePtr);
                    pInterface[(data->iNumOMXCores)] = coreIntPtr;
                    (data->iNumOMXCores)++;
                    continue;
                }
            }
            lib->Close();
            OSCL_DELETE(lib);
        }
    }

    // allocate space for the OMX component registry
    PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)OSCL_MALLOC(MAX_NUMBER_OF_OMX_COMPONENTS * sizeof(PVOMXMasterRegistryStruct));
    if (pOMXMasterRegistry == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }
    data->iMasterRegistry = (void*)pOMXMasterRegistry;

    PVOMXCompHandles* pOMXCompHandles = (PVOMXCompHandles*)OSCL_MALLOC(MAX_NUMBER_OF_OMX_COMPONENTS * sizeof(PVOMXCompHandles));
    if (pOMXCompHandles == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }
    data->iOMXCompHandles = (void*)pOMXCompHandles;

    // init the array
    memset(pOMXCompHandles, 0, MAX_NUMBER_OF_OMX_COMPONENTS*sizeof(PVOMXCompHandles));

    // loop over all cores
    master_index = 0;
    OMX_STRING ComponentName = (OMX_STRING)OSCL_MALLOC(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

    for (jj = 0; jj < data->iNumOMXCores; jj++)
    {
        // first call OMX_Init
        Status = (*(pInterface[jj]->GetpOMX_Init()))();

        if (Status == OMX_ErrorNone)
        {
            // enumerate components to get their number etc.
            OMX_ERRORTYPE stat = OMX_ErrorNone;

            index = 0;
            while (stat != OMX_ErrorNoMore)
            {
                // clear, then get next component name
                memset(ComponentName, 0, PV_OMX_MAX_COMPONENT_NAME_LENGTH*sizeof(OMX_U8));

                stat = (*(pInterface[jj]->GetpOMX_ComponentNameEnum()))(
                           ComponentName,
                           PV_OMX_MAX_COMPONENT_NAME_LENGTH,
                           index);

                if (stat == OMX_ErrorNoMore)
                    break;

                // check number roles of the component
                OMX_U32 numRoles;
                numRoles = 0;
                stat = (*(pInterface[jj]->GetpOMX_GetRolesOfComponent()))(
                           ComponentName,
                           &numRoles,
                           NULL);

                if ((numRoles > 0) && (stat == OMX_ErrorNone))
                {
                    // allocate space for roles of the component
                    OMX_U32 role;
                    OMX_U8 **ComponentRoles = (OMX_U8**)OSCL_MALLOC(numRoles * sizeof(OMX_U8 *));

                    for (role = 0; role < numRoles; role++)
                        ComponentRoles[role] = (OMX_U8*)OSCL_MALLOC(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));


                    // get the array of strings with component roles
                    stat = (*(pInterface[jj]->GetpOMX_GetRolesOfComponent()))(
                               ComponentName,
                               &numRoles,
                               ComponentRoles);

                    //register all components separately in master registry
                    if (stat == OMX_ErrorNone)
                    {
                        for (role = 0; (role < numRoles) && (master_index < MAX_NUMBER_OF_OMX_COMPONENTS); role++)
                        {
                            strncpy((OMX_STRING)pOMXMasterRegistry[master_index].CompName, ComponentName, PV_OMX_MAX_COMPONENT_NAME_LENGTH);
                            strncpy((OMX_STRING)pOMXMasterRegistry[master_index].CompRole, (OMX_STRING)ComponentRoles[role], PV_OMX_MAX_COMPONENT_NAME_LENGTH);
                            pOMXMasterRegistry[master_index].OMXCoreIndex = jj;
                            pOMXMasterRegistry[master_index].CompIndex = component_index;
                            master_index++;

                        }
                    }

                    // dealloc space for component roles
                    for (role = 0; role < numRoles; role++)
                        OSCL_FREE(ComponentRoles[role]);

                    OSCL_FREE(ComponentRoles);


                } // done with roles of component represented by index

                index++; // get next component from the jj-th core
                component_index++; // increase the global component index

            } // done with all components from jj-th core
        } // end of if(Status==OMX_ErrorNone)

    }//end of for loop
    // at this point, all cores were init, and all components registered

    OSCL_FREE(ComponentName);
    data->iTotalNumOMXComponents = master_index;


    return OMX_ErrorNone;
}

//this routine is needed to avoid a longjmp clobber warning
static void _Try_OMX_Init(int32& aError, OMX_ERRORTYPE& aStatus, OMXMasterCoreGlobalData *data)
{
    OSCL_TRY(aError, aStatus = _OMX_Init(data););
}
//this routine is needed to avoid a longjmp clobber warning
static void _Try_OMX_Create(int32& aError, OMXMasterCoreGlobalData*& aData)
{
    OSCL_TRY(aError, aData = OSCL_NEW(OMXMasterCoreGlobalData, ()););
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_Init()
{
    OMX_ERRORTYPE status = OMX_ErrorNone;

    //Check the global instance counter and only init OMX on the first call
    int32 error;
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);

    if (data)
    {
        // If singleton was already created
        //Just update the instance counter.
        data->iNumMasterOMXInstances++;

        //Release the singleton.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        if (error)
        {
            status = OMX_ErrorUndefined;
            return status;
        }
    }
    else
    {
        //First call
        //create the OMX Master Core singleton data
        _Try_OMX_Create(error, data);
        if (error != OsclErrNone)
        {
            status = OMX_ErrorInsufficientResources;//some leave happened.
        }

        //If create succeeded, then init the OMX globals.
        if (status == OMX_ErrorNone)
        {
            _Try_OMX_Init(error, status, data);
            if (error != OsclErrNone)
            {
                status = OMX_ErrorUndefined;//probably no memory.
            }
        }

        //Release the singleton.
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        if (error)
        {
            //registry error
            status = OMX_ErrorUndefined;
            return status;
        }


    }

    if (error && status == OMX_ErrorNone)
        status = OMX_ErrorUndefined;//registry error

    return status;
}


static OMX_ERRORTYPE _OMX_Deinit(OMXMasterCoreGlobalData *data)
{
    OMX_U32 jj;
    OMX_ERRORTYPE Status = OMX_ErrorNone;

    //free master registry
    data->iTotalNumOMXComponents = 0;
    PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);

    if (pOMXMasterRegistry)
        OSCL_FREE(pOMXMasterRegistry);

    data->iMasterRegistry = NULL;

    PVOMXCompHandles* pOMXCompHandles = (PVOMXCompHandles*)(data->iOMXCompHandles);
    if (pOMXCompHandles)
        OSCL_FREE(pOMXCompHandles);
    data->iOMXCompHandles = NULL;


    OMXInterface** pInterface = (OMXInterface**)(data->iInterface);
    //call Deinit for each core
    if (pInterface)
    {
        for (jj = 0; jj < data->iNumOMXCores; jj++)
        {
            Status = (*(pInterface[jj]->GetpOMX_Deinit()))();
            OSCL_DELETE(pInterface[jj]);
        }

        OSCL_FREE(pInterface);
        data->iInterface = NULL;
    }


    OsclSharedLibrary** pLibrary = (OsclSharedLibrary**)(data->iOMXLibrary);
    if (pLibrary)
    {
        for (jj = 0; jj < data->iNumOMXCores; jj++)
        {
            if (pLibrary[jj])
            {
                pLibrary[jj]->Close();
                OSCL_DELETE(pLibrary[jj]);
            }
        }
        OSCL_FREE(pLibrary);
        data->iOMXLibrary = NULL;
    }
    data->iNumOMXCores = 0;

    return OMX_ErrorNone;
}

//this routine is needed to avoid a longjmp clobber warning.
static void _Try_OMX_Deinit(int32 &aError, OMX_ERRORTYPE& aStatus, OMXMasterCoreGlobalData* data)
{
    OSCL_TRY(aError, aStatus = _OMX_Deinit(data););
}

//this routine is needed to avoid a longjmp clobber warning.
static void _Try_Data_Cleanup(int32 &aError, OMXMasterCoreGlobalData* aData)
{
    OSCL_TRY(aError, OSCL_DELETE(aData););
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_Deinit()
{
    OMX_ERRORTYPE status = OMX_ErrorNone;

    //Check the global instance counter and only cleanup OMX on the last call.
    int32 error;
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);

    if (data)
    {
        data->iNumMasterOMXInstances--;
        if (data->iNumMasterOMXInstances == 0)
        {

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
    OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);

    return status;
}


OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{
    OSCL_UNUSED_ARG(nNameLength);
    OMX_U32 ii;

    OSCL_UNUSED_ARG(nNameLength);
    int32 error;
    // this method just reads from the master registry (which does not change) - so no lock is needed
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);
        if (NULL == pOMXMasterRegistry)
        {
            return OMX_ErrorNoMore;
        }

        // go through alll the components and find the one with the correct index
        for (ii = 0; ii < data->iTotalNumOMXComponents; ii++)
        {
            if (pOMXMasterRegistry[ii].CompIndex == nIndex)
            {
                oscl_strncpy((OMX_STRING)cComponentName,
                             (OMX_STRING)pOMXMasterRegistry[ii].CompName,
                             PV_OMX_MAX_COMPONENT_NAME_LENGTH);
                break;
            }
        }

        if (ii == data->iTotalNumOMXComponents)
        {
            return OMX_ErrorNoMore;
        }

        return OMX_ErrorNone;
    }
    else
    {
        return OMX_ErrorNoMore;
    }
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY   OMX_GetHandle(
    OMX_OUT OMX_HANDLETYPE* pHandle,
    OMX_IN  OMX_STRING cComponentName,
    OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
{
    OMX_ERRORTYPE Status = OMX_ErrorNone;
    OMX_U32 ii, kk;

    int32 error;
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);

    if (data)
    {
        PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);
        OMXInterface** pInterface = (OMXInterface**)(data->iInterface);
        PVOMXCompHandles* pOMXCompHandles = (PVOMXCompHandles*)(data->iOMXCompHandles);

        if ((pOMXMasterRegistry == NULL) || (pInterface == NULL) || (pOMXCompHandles == NULL))
        {
            // unlock singleton before returning
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
            if (error)
            {
                //registry error
                Status = OMX_ErrorUndefined;
                return Status;
            }
            return OMX_ErrorInsufficientResources;
        }

        for (ii = 0; ii < (data->iTotalNumOMXComponents); ii++)
        {
            // go through the list of supported components and find the component based on its name (identifier)
            if (!oscl_strcmp((OMX_STRING)pOMXMasterRegistry[ii].CompName, cComponentName))
            {
                // found a matching name
                break;
            }
        }
        if (ii == (data->iTotalNumOMXComponents))
        {
            // could not find a component with the given name
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
            if (error)
            {
                //registry error
                Status = OMX_ErrorUndefined;
                return Status;
            }
            return OMX_ErrorComponentNotFound;
        }

        // call the appropriate GetHandle for the component

        // save component handle with the OMX core index, so it can be retrieved
        // later when freehandle is called
        // find an empty slot to write the pair handle/index:
        for (kk = 0; kk < MAX_NUMBER_OF_OMX_COMPONENTS; kk++)
        {
            if (pOMXCompHandles[kk].handle == NULL)
            {
                break;
            }
        }
        if (kk == MAX_NUMBER_OF_OMX_COMPONENTS)
        {
            // no empty slot was found
            return OMX_ErrorComponentNotFound;
        }

        OMX_U32 index = pOMXMasterRegistry[ii].OMXCoreIndex;

        Status = (*(pInterface[index]->GetpOMX_GetHandle()))(pHandle, cComponentName, pAppData, pCallBacks);
        if (Status == OMX_ErrorNone)
        {
            // write the pair handle/index
            pOMXCompHandles[kk].handle       = *pHandle;
            pOMXCompHandles[kk].OMXCoreIndex = index;
        }
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        if (error)
        {
            //registry error
            Status = OMX_ErrorUndefined;

        }
        return Status;
    }
    else
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        if (error)
        {
            //registry error
            Status = OMX_ErrorUndefined;
            return Status;
        }
        return OMX_ErrorInsufficientResources;
    }

}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE Status = OMX_ErrorNone;
    OMX_U32 RegIndex;

    int32 error;
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::lockAndGetInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);

    if (data)
    {
        PVOMXCompHandles* pOMXCompHandles = (PVOMXCompHandles*)(data->iOMXCompHandles);
        OMXInterface** pInterface = (OMXInterface**)(data->iInterface);

        if ((pOMXCompHandles == NULL) || (pInterface == NULL))
        {
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
            if (error)
            {
                //registry error
                Status = OMX_ErrorUndefined;
                return Status;
            }
            return OMX_ErrorInsufficientResources;
        }
        // get the core index for the handle
        Status = GetRegIndexForHandle(hComponent, RegIndex, data);

        if (OMX_ErrorNone != Status)
        {
            OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
            return Status;
        }

        // call the appropriate GetHandle for the component

        OMX_U32 index = pOMXCompHandles[RegIndex].OMXCoreIndex;
        Status = (*(pInterface[index]->GetpOMX_FreeHandle()))(hComponent);
        //we're done with this, so get rid of the component handle
        pOMXCompHandles[RegIndex].handle = NULL;
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        if (error)
        {
            //registry error
            Status = OMX_ErrorUndefined;

        }
        return Status;
    }
    else
    {
        OsclSingletonRegistry::registerInstanceAndUnlock(data, OSCL_SINGLETON_ID_OMXMASTERCORE, error);
        return OMX_ErrorInsufficientResources;
    }
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_SetupTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput)
{
    OMX_ERRORTYPE Status;
    OMX_U32 RegIndex1;
    OMX_U32 RegIndex2;

    int32 error;
    // no need to lock - a different instance/thread does not affect the global array searches in this thread
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        Status = GetRegIndexForHandle(hOutput, RegIndex1, data);
        if (OMX_ErrorNone != Status)
        {
            return Status;
        }

        Status = GetRegIndexForHandle(hInput, RegIndex2, data);
        if (OMX_ErrorNone != Status)
        {
            return Status;
        }

        PVOMXCompHandles* pOMXCompHandles = (PVOMXCompHandles*)(data->iOMXCompHandles);
        OMXInterface** pInterface = (OMXInterface**)(data->iInterface);

        if ((pOMXCompHandles == NULL) || (pInterface == NULL))
        {
            return OMX_ErrorInsufficientResources;
        }

        //if (pOMXCompHandles[RegIndex1].OMXCoreIndex != pOMXCompHandles[RegIndex2].OMXCoreIndex)
        //{
        // the components are from different omx cores
        // it is not clear if setting up a tunnel between them is supported
        //  return OMX_ErrorNotImplemented;
        //}



        OMX_U32 index = pOMXCompHandles[RegIndex1].OMXCoreIndex;
        Status = (*(pInterface[index]->GetpOMX_SetupTunnel()))(
                     hOutput,
                     nPortOutput,
                     hInput,
                     nPortInput);

        return Status;


    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI)
{
    OMX_ERRORTYPE Status = OMX_ErrorNotImplemented;
    OMX_U32 ii;

    int32 error;
    // no need to lock
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        // try to get the content pipe from any of the omx cores
        // return the first valid one
        OMXInterface** pInterface = (OMXInterface**)(data->iInterface);
        if (pInterface)
        {
            for (ii = 0; ii < data->iNumOMXCores; ii++)
            {
                Status = (*(pInterface[ii]->GetpOMX_GetContentPipe()))(hPipe, szURI);
                if (OMX_ErrorNone == Status)
                {
                    break;
                }
            }
        }
        else
        {
            return OMX_ErrorInsufficientResources;
        }
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

    return Status;
}

OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetComponentsOfRole(
    OMX_IN      OMX_STRING role,
    OMX_INOUT   OMX_U32 *pNumComps,
    OMX_INOUT   OMX_U8  **compNames)
{
    OMX_U32 ii;

    // initialize the number of components found to 0
    *pNumComps = 0;
    int32 error;
    // no need to lock - just reading from master registry that does not change
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);
        if (pOMXMasterRegistry == NULL)
        {
            return OMX_ErrorNone;
        }

        // go through all components and check if they support the given role
        for (ii = 0; ii < data->iTotalNumOMXComponents; ii++)
        {
            // if the role matches, increment the counter and record the comp. name
            if (!oscl_strcmp((OMX_STRING)pOMXMasterRegistry[ii].CompRole, role))
            {
                // if a placeholder for compNames is provided, copy the component name into it
                if (NULL != compNames)
                {
                    oscl_strncpy((OMX_STRING)compNames[*pNumComps],
                                 (OMX_STRING)pOMXMasterRegistry[ii].CompName,
                                 PV_OMX_MAX_COMPONENT_NAME_LENGTH);
                }
                // increment the counter
                *pNumComps = (*pNumComps + 1);
            }
        }
    }

    return OMX_ErrorNone;
}


OSCL_EXPORT_REF OMX_ERRORTYPE OMX_GetRolesOfComponent(
    OMX_IN      OMX_STRING compName,
    OMX_INOUT   OMX_U32* pNumRoles,
    OMX_OUT     OMX_U8** roles)
{
    OMX_U32 ii;

    *pNumRoles = 0;

    int32 error;
    // no need to lock - just reading from master registry that does not change
    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);
        if (pOMXMasterRegistry == NULL)
        {
            return OMX_ErrorNone;
        }

        // go through all components
        for (ii = 0; ii < data->iTotalNumOMXComponents; ii++)
        {
            // if the name  matches, increment the counter and record the comp. role
            if (!oscl_strcmp((OMX_STRING)pOMXMasterRegistry[ii].CompName, compName))
            {
                // if a placeholder for roles is provided, copy the component role into it
                if (NULL != roles)
                {
                    oscl_strncpy((OMX_STRING)roles[*pNumRoles],
                                 (OMX_STRING)pOMXMasterRegistry[ii].CompRole,
                                 PV_OMX_MAX_COMPONENT_NAME_LENGTH);
                }
                // increment the counter
                *pNumRoles = (*pNumRoles + 1);
            }
        }
    }

    return OMX_ErrorNone;
}

OMX_BOOL PV_OMXConfigParser(
    OMX_PTR aInputParameters,
    OMX_PTR aOutputParameters)

{
    OMXConfigParserInputs* pInputs;

    pInputs = (OMXConfigParserInputs*) aInputParameters;


    if (NULL != pInputs->cComponentRole)
    {
        if (0 == oscl_strncmp(pInputs->cComponentRole, (OMX_STRING)"audio_decoder", oscl_strlen("audio_decoder")))
        {
            OMX_S32 Status;
            pvAudioConfigParserInputs aInputs;

            aInputs.inPtr = pInputs->inPtr;
            aInputs.inBytes = pInputs->inBytes;

            if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"audio_decoder.wma"))
            {
                aInputs.iMimeType = PVMF_MIME_WMA;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"audio_decoder.aac"))
            {
                aInputs.iMimeType = PVMF_MIME_AAC_SIZEHDR;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"audio_decoder.amr"))
            {
                aInputs.iMimeType = PVMF_MIME_AMR;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"audio_decoder.mp3"))
            {
                aInputs.iMimeType = PVMF_MIME_MP3;

            }
            else
            {
                return OMX_FALSE;
            }


            Status = pv_audio_config_parser(&aInputs, (pvAudioConfigParserOutputs *)aOutputParameters);
            if (0 == Status)
            {
                return OMX_FALSE;
            }
        }
        else if (0 == oscl_strncmp(pInputs->cComponentRole, (OMX_STRING)"video_decoder", oscl_strlen("video_decoder")))
        {

            OMX_S32 Status;
            pvVideoConfigParserInputs aInputs;

            aInputs.inPtr = pInputs->inPtr;
            aInputs.inBytes = pInputs->inBytes;

            if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"video_decoder.wmv"))
            {
                aInputs.iMimeType = PVMF_MIME_WMV;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"video_decoder.avc"))
            {
                aInputs.iMimeType = PVMF_MIME_H264_VIDEO;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"video_decoder.mpeg4"))
            {
                aInputs.iMimeType = PVMF_MIME_M4V;

            }
            else if (0 == oscl_strcmp(pInputs->cComponentRole, (OMX_STRING)"video_decoder.h263"))
            {
                aInputs.iMimeType = PVMF_MIME_H2632000;

            }
            else
            {
                return OMX_FALSE;
            }

            Status = pv_video_config_parser(&aInputs, (pvVideoConfigParserOutputs *)aOutputParameters);
            if (0 != Status)
            {
                return OMX_FALSE;
            }
        }
        else
        {
            return OMX_FALSE;
        }

    }
    else
    {
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

OSCL_EXPORT_REF OMX_BOOL OMXConfigParser(
    OMX_PTR aInputParameters,
    OMX_PTR aOutputParameters)

{
    OMX_BOOL Status = OMX_FALSE;
    OMX_U32 ii;
    int32 error;

    OMXMasterCoreGlobalData* data = (OMXMasterCoreGlobalData*)OsclSingletonRegistry::getInstance(OSCL_SINGLETON_ID_OMXMASTERCORE, error);
    if (data)
    {
        // try to get the omxconfigparser from omx cores
        // return the first valid one
        OMXInterface** pInterface = (OMXInterface**)(data->iInterface);
        PVOMXMasterRegistryStruct* pOMXMasterRegistry = (PVOMXMasterRegistryStruct*)(data->iMasterRegistry);

        if (pOMXMasterRegistry == NULL)
        {
            return Status;
        }
        if (pInterface)
        {
            for (ii = 0; ii < (data->iTotalNumOMXComponents); ii++)
            {
                if (!oscl_strcmp((OMX_STRING)pOMXMasterRegistry[ii].CompRole, ((OMXConfigParserInputs*)aInputParameters)->cComponentRole))
                {
                    // go through the list of supported components and find the component based on its name (identifier)
                    if (!oscl_strcmp((OMX_STRING)pOMXMasterRegistry[ii].CompName, ((OMXConfigParserInputs*)aInputParameters)->cComponentName))
                    {
                        // found a matching name
                        break;
                    }
                }
            }

            if (ii == (data->iTotalNumOMXComponents))
            {
                return Status;
            }

            OMX_U32 index = pOMXMasterRegistry[ii].OMXCoreIndex;
            if (pInterface[index]->GetpOMXConfigParser() == NULL)
            {
                //The OMX core does not have config parser - use PV config parser
                Status = PV_OMXConfigParser(aInputParameters, aOutputParameters);
            }
            else
            {
                Status = (*(pInterface[index]->GetpOMXConfigParser()))(aInputParameters, aOutputParameters);
            }
        }
        else
        {
            return Status;
        }
    }

    return Status;
}






