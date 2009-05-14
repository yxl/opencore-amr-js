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
#ifndef PVMF_CPMPLUGIN_CONTENTMANAGEMENT_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_CONTENTMANAGEMENT_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define PVMF_CPMPLUGIN_CONTENTMANAGEMENT_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/contentmanagement_interface"
#define PVMFCPMPluginContentManagementInterfaceUuid PVUuid(0x8cf5b9d3, 0x14f9, 0x4ae0, 0xba, 0x46, 0x5c, 0xd7, 0xab, 0x4a, 0xfb, 0x48)

#define PVMF_CONTENT_ENTRY_ATTRIB_UNDEFINED      0xFFFFFFFF
#define PVMF_CONTENT_ENTRY_ATTRIB_NORMAL         0x00
#define PVMF_CONTENT_ENTRY_ATTRIB_RDONLY         0x01
#define PVMF_CONTENT_ENTRY_ATTRIB_HIDDEN         0x02
#define PVMF_CONTENT_ENTRY_ATTRIB_SYSTEM         0x04
#define PVMF_CONTENT_ENTRY_ATTRIB_DIRENTRY       0x10
#define PVMF_CONTENT_ENTRY_ATTRIB_ARCHIVE        0x20

class PVMFContentEntryProperties
{
    public:
        PVMFContentEntryProperties()
        {
            iIsWChar         = false;
            iEntrySizeOnDisk = 0;
            iEntryAttributes = PVMF_CONTENT_ENTRY_ATTRIB_UNDEFINED;
        };

        PVMFContentEntryProperties(const PVMFContentEntryProperties& a)
        {
            iShortEntryName       = a.iShortEntryName;
            iLongEntryName        = a.iLongEntryName;
            iModificationDateTime = a.iModificationDateTime;
            iEntrySizeOnDisk      = a.iEntrySizeOnDisk;
            iEntryAttributes      = a.iEntryAttributes;
            iIsWChar              = a.iIsWChar;
        };

        virtual ~PVMFContentEntryProperties()
        {
        };

        PVMFContentEntryProperties& operator=(const PVMFContentEntryProperties& a)
        {
            if (&a != this)
            {
                iShortEntryName       = a.iShortEntryName;
                iLongEntryName        = a.iLongEntryName;
                iModificationDateTime = a.iModificationDateTime;
                iEntrySizeOnDisk      = a.iEntrySizeOnDisk;
                iEntryAttributes      = a.iEntryAttributes;
                iIsWChar              = a.iIsWChar;
            }
            return *this;
        };

        void SetWideCharMode()
        {
            iIsWChar = true;
        }

        bool IsWideChar()
        {
            return iIsWChar;
        }

        /* Indicates if wide char or reg char strings are in use */
        bool iIsWChar;
        /* Just the entry name with extension */
        OSCL_wHeapString<OsclMemAllocator> iShortEntryNameW;
        OSCL_HeapString<OsclMemAllocator>  iShortEntryName;
        /* Complete path of the entry from the root drive */
        OSCL_wHeapString<OsclMemAllocator> iLongEntryNameW;
        OSCL_HeapString<OsclMemAllocator>  iLongEntryName;
        /* Entry Modification date and time in ISO 8601 format */
        OSCL_wHeapString<OsclMemAllocator> iModificationDateTimeW;
        OSCL_HeapString<OsclMemAllocator>  iModificationDateTime;
        /* Entry size on disk - if entry is a file it would be the file size */
        uint32 iEntrySizeOnDisk;
        /* Entry attributes - viz. read only, directory etc */
        uint32 iEntryAttributes;
};

/**
 * Content Management interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginContentManagementInterface : public PVInterface
{
    public:
        /**
         * Method to mount a partition
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the drive letter
         *             say, X:\\
         * @param [in] Null terminated unicode string containing the partition id
         *             - not required for public partitions
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus MountPartition(PVMFSessionId aSessionId,
                                          OSCL_wString& aDriveLetter,
                                          OSCL_wString& aPartitionID) = 0;

        /**
         * Method to mount a partition
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the drive letter
         *             say, X:\\
         * @param [in] Null terminated string containing the partition id
         *             - not required for public partitions
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus MountPartition(PVMFSessionId aSessionId,
                                          OSCL_String& aDriveLetter,
                                          OSCL_String& aPartitionID) = 0;

        /**
         * Method to unmount a partition
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the drive letter
         *             say, X:\\
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus UnMountPartition(PVMFSessionId aSessionId,
                                            OSCL_wString& aDriveLetter) = 0;

        /**
         * Method to unmount a partition
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the drive letter
         *             say, X:\\
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus UnMountPartition(PVMFSessionId aSessionId,
                                            OSCL_String& aDriveLetter) = 0;

        /**
         * Method to get the current directory in a drive
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the drive letter
         *             say, X:\\
         * @param [out] Path name of the current working directory, say
         *              X:\\My Content
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetCurrentDirectory(PVMFSessionId aSessionId,
                                               OSCL_wString& aDriveLetter,
                                               OSCL_wString& aPath) = 0;

        /**
         * Method to get the current directory in a drive
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the drive letter
         *             say, X:\\
         * @param [out] Path name of the current working directory, say
         *              X:\\My Content
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetCurrentDirectory(PVMFSessionId aSessionId,
                                               OSCL_String&  aDriveLetter,
                                               OSCL_String&  aPath) = 0;

        /**
         * Method to set current directory
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the path
         *             including the drive letter say, X:\\My Audios\\Album1
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus SetCurrentDirectory(PVMFSessionId aSessionId,
                                               OSCL_wString& aPath) = 0;

        /**
         * Method to set current directory
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the path
         *             including the drive letter say, X:\\My Audios\\Album1
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus SetCurrentDirectory(PVMFSessionId aSessionId,
                                               OSCL_String&  aPath) = 0;

        /**
         * Method to get a listing of all the entries that match a pattern
         * within a path
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath[in] Unicode string containing the path, say
         *              X:\\My Videos
         * @param aPatternList[in] List containing search pattern for entries, say *.3gp, *.wma etc
         * @param aEntryList[out] List containing entry properties
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetEntriesInPath(PVMFSessionId aSessionId,
                                            OSCL_wString& aPath,
                                            Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> aPatternList,
                                            Oscl_Vector<PVMFContentEntryProperties, OsclMemAllocator>& aEntryList) = 0;

        /**
         * Method to get a listing of all the entries that match a pattern
         * within a path
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath[in] string containing the path, say
         *              X:\\My Videos
         * @param aPatternList[in] List containing search pattern for entries, say *.3gp, *.wma etc
         * @param aEntryList[out] List containing entry properties
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetEntriesInPath(PVMFSessionId aSessionId,
                                            OSCL_String&  aPath,
                                            Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> aPatternList,
                                            Oscl_Vector<PVMFContentEntryProperties, OsclMemAllocator>& aEntryList) = 0;

        /**
         * Method to rename a directory or file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aOldPath Unicode string with current complete path
         * @param aNewPath Unicode string with new complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus RenamePath(PVMFSessionId aSessionId,
                                      OSCL_wString& aOldPath,
                                      OSCL_wString& aNewPath) = 0;

        /**
         * Method to rename a directory or file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aOldPath string with current complete path
         * @param aNewPath string with new complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus RenamePath(PVMFSessionId aSessionId,
                                      OSCL_String&  aOldPath,
                                      OSCL_String&  aNewPath) = 0;

        /**
         * Method to create a directory
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath Unicode string with the complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus CreateDirectory(PVMFSessionId aSessionId,
                                           OSCL_wString& aPath) = 0;

        /**
         * Method to create a directory
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath string with the complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus CreateDirectory(PVMFSessionId aSessionId,
                                           OSCL_String&  aPath) = 0;

        /**
         * Method to remove a directory
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath Unicode string with the complete path
         * @param aRecursive For recursive removal
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus DeleteDirectory(PVMFSessionId aSessionId,
                                           OSCL_wString& aPath,
                                           bool          aRecursive) = 0;

        /**
         * Method to remove a directory
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aPath string with the complete path
         * @param aRecursive For recursive removal
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus DeleteDirectory(PVMFSessionId aSessionId,
                                           OSCL_String&  aPath,
                                           bool          aRecursive) = 0;

        /**
         * Method to remove a file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aFileName Unicode string with the complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus DeleteFile(PVMFSessionId aSessionId,
                                      OSCL_wString& aFileName) = 0;

        /**
         * Method to remove a file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aFileName string with the complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus DeleteFile(PVMFSessionId aSessionId,
                                      OSCL_String& aFileName) = 0;


        /**
         * Method to copy a file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aSourceFileName unicode string with source file name, with complete path
         * @param aTargetFileName unicode string with target file name, with complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus CopyFile(PVMFSessionId aSessionId,
                                    OSCL_wString& aSourceFileName,
                                    OSCL_wString& aTargetFileName) = 0;

        /**
         * Method to copy a file
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aSourceFileName string with source file name, with complete path
         * @param aTargetFileName string with target file name, with complete path
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus CopyFile(PVMFSessionId aSessionId,
                                    OSCL_String&  aSourceFileName,
                                    OSCL_String&  aTargetFileName) = 0;

        virtual PVMFStatus IsFileProtected(PVMFSessionId aSessionId,
                                           OSCL_wString& aFileName,
                                           bool& aProtected) = 0;

        virtual PVMFStatus IsFileProtected(PVMFSessionId aSessionId,
                                           OSCL_String& aFileName,
                                           bool& aProtected) = 0;

        virtual PVMFStatus GetNumExportsLeft(PVMFSessionId aSessionId,
                                             OSCL_wString& aFileName,
                                             uint32& aNumExports) = 0;

        virtual PVMFStatus GetNumExportsLeft(PVMFSessionId aSessionId,
                                             OSCL_String& aFileName,
                                             uint32& aNumExports) = 0;
};


#endif //PVMF_CPMPLUGIN_CONTENTMANAGEMENT_INTERFACE_H_INCLUDED

