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
#ifndef PVAUTHORENGINEFACTORY_H_INCLUDED
#define PVAUTHORENGINEFACTORY_H_INCLUDED

// Forward declaration
class PVAuthorEngineInterface;
class PVCommandStatusObserver;
class PVInformationalEventObserver;
class PVErrorEventObserver;


/**
 * PVAuthorEngineFactory Class
 *
 * PVAuthorEngineFactory class is a singleton class which instantiates and provides
 * access to pvAuthor engine. It returns an PVAuthorEngineInterface
 * reference, the interface class of the pvAuthor SDK.
 *
 * The application is expected to contain and maintain a pointer to the
 * PVAuthorEngineInterface instance at all time that pvAuthor engine is active.
 **/
class PVAuthorEngineFactory
{
    public:
        /**
         * Creates an instance of a pvAuthor engine. If the creation fails, this function will leave.
         *
         * @param aCmdStatusObserver     The observer for command status
         * @param aErrorEventObserver    The observer for unsolicited error events
         * @param aInfoEventObserver     The observer for unsolicited informational events
         * @returns A pointer to an author or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVAuthorEngineInterface* CreateAuthor(PVCommandStatusObserver* aCmdStatusObserver,
                PVErrorEventObserver *aErrorEventObserver, PVInformationalEventObserver *aInfoEventObserver);

        /**
         * This function allows the application to delete an instance of a pvAuthor
         * and reclaim all allocated resources.  An author can be deleted only in
         * the idle state. An attempt to delete an author in any other state will
         * fail and return false.
         *
         * @param aAuthor The author to be deleted.
         * @returns A status code indicating success or failure.
         **/
        OSCL_IMPORT_REF static bool DeleteAuthor(PVAuthorEngineInterface* aAuthor);
};

#endif // PVAUTHORENGINEFACTORY_H_INCLUDED
