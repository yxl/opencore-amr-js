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
#ifndef CPM_PLUGIN_REGISTRY_H
#define CPM_PLUGIN_REGISTRY_H

/*
 * CPMPluginContainer is a simple container class for a plugin implementation
 * plus user authentication data.  This class is used by the plugin registry.
 */

class PVMFCPMPluginInterface;

class CPMPluginContainer
{
    public:
        CPMPluginContainer(PVMFCPMPluginInterface& aPlugIn,
                           OsclAny* aPlugInData)
                : iPlugIn(aPlugIn)
                , iPlugInUserAuthenticationData(aPlugInData)
        {
        }

        PVMFCPMPluginInterface& PlugIn()
        {
            return iPlugIn;
        }
        OsclAny* PlugInUserAuthenticationData()
        {
            return iPlugInUserAuthenticationData;
        }

    private:
        PVMFCPMPluginInterface& iPlugIn;
        OsclAny*                iPlugInUserAuthenticationData;
};


/*
 * CPMPluginRegistry is an abstract interface class
 * defining the plugin registry.
 */
class OsclSharedLibraryList;
class CPMPluginRegistry
{
    public:

        /*
         * Adds a plug-in instance to the registry.
         * @param aMimeType: a unique MIME string for the plug-in.
         * @param aPlugInContainer: pointer to plug-in instance.
         * @return: true if success, false if failure.
         */
        virtual bool addPluginToRegistry(OSCL_String& aMimeType,
                                         CPMPluginContainer& aPlugInContainer) = 0;
        virtual void removePluginFromRegistry(OSCL_String& aMimeType) = 0;

        /*
         * Retrieves the plug-in for a particular MIME string.
         * @param MIME string.
         * @return Plug-in instance pointer.
         */
        virtual CPMPluginContainer* lookupPlugin(OSCL_String& aMimeType) = 0;

        /*
         * Returns the number of plug-ins in the registry.
         * @return Number of plug-ins.
         */
        virtual uint32 GetNumPlugIns() = 0;

        /*
         * Returns the MIME strings for the plugins.
         * @param aIndex: zero-based index for the selected plugin.
         * @param aMimeType: on return, will contain the mime type for the selected
         *    plugin.
         * @return true if success, false if invalid index.
         */
        virtual bool GetPluginMimeType(uint32 aIndex, OSCL_String& aMimeType) = 0;
        virtual ~CPMPluginRegistry() {}

        virtual OsclSharedLibraryList*& AccessSharedLibraryList() = 0;
};

/**
 * CPMPluginRegistryFactory is a class to create/destroy a CPMPluginRegistry
 * implementation.
 *
 */
class CPMPluginRegistryFactory
{
    public:
        /**
         * Creates an instance of a CPMPluginRegistry.
         * If the creation fails, this function will leave.
         *
         * @param None
         * with compiled in plugins would be used.
         *
         * @returns A pointer to an instance of CPMPluginRegistry
         * leaves if instantiation fails
         **/
        static CPMPluginRegistry* CreateCPMPluginRegistry();
        /**
         * Deletes an instance of CPMPluginRegistry
         * and reclaims all allocated resources.
         *
         * @param aNode The CPMPluginRegistry instance to be deleted
         * @returns None
         **/
        static void DestroyCPMPluginRegistry(CPMPluginRegistry*);
};

#endif //CPM_PLUGIN_REGISTRY_H



