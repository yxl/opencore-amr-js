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
/**
 * @file pvmp4ffcn_clipconfig.h
 * @brief Clip level configuration of PVMp4FFComposerNode
 */

#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#define PVMP4FFCN_CLIPCONFIG_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PV_MP4FFCOMPOSER_CONFIG_H_INCLUDED
#include "pv_mp4ffcomposer_config.h"
#endif


#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif
#define KPVMp4FFCNClipConfigUuid PVUuid(0x2e3b479f,0x2c46,0x465c,0xba,0x41,0xb8,0x91,0x11,0xa9,0xdf,0x3a)

typedef enum
{

    /**
     * This mode authors 3GPP Progressive Downloadable output files:
     * Meta Data is upfront.
     * Media Data is interleaved. Temp files are used.
     * Temporary files are written to the same directory as the output file.
     */
    PVMP4FFCN_3GPP_PROGRESSIVE_DOWNLOAD_MODE = 0x00000003,

    /**
     * This mode authors 3GPP Downloadable output files:
     * Meta Data is towards the end of the clip.
     * Media Data is interleaved.
     * No temp files are used.
     */
    PVMP4FFCN_3GPP_DOWNLOAD_MODE = 0x00000009,


    /**
     * This mode authors I-Motion compliant output files:
     * Meta Data is upfront.
     * Media Data is interleaved. Temp files are used.
     * Imotion DRM and Udta atoms are authored
     * Temporary files are written to the same directory as the output file.
     */
    PVMP4FFCN_IMOTION_PSEUDO_STREAMING_MODE = 0x00000007,

    /**
     * This mode authors I-Motion compliant output files:
     * Meta Data is towards the end of the file.
     * Media Data is interleaved.
     * Imotion DRM and Udta atoms are authored
     * NoTemp files are used.
     */
    PVMP4FFCN_IMOTION_DOWNLOAD_MODE = 0x0000000D,

    /**
     * This mode authors movie fragment files:
     * Meta Data is towards the end of the clip in MOOV and MOOF.
     * Media Data is interleaved.
     * No temp files are used.
     */
    PVMP4FFCN_MOVIE_FRAGMENT_MODE = 0x00000021


} PVMp4FFCN_AuthoringMode;

/**
 * PVMp4FFCNClipConfigInterface allows a client to control properties of PVMp4FFComposerNode
 */
class PVMp4FFCNClipConfigInterface : public PVInterface
{
    public:
        /**
         * Register a reference to this interface.
         */
        virtual void addRef() = 0;

        /**
         * Remove a reference to this interface.
         */
        virtual void removeRef() = 0;

        /**
         * Query for an instance of a particular interface.
         *
         * @param uuid Uuid of the requested interface
         * @param iface Output parameter where pointer to an instance of the
         * requested interface will be stored if it is supported by this object
         * @return true if the requested interface is supported, else false
         */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * This method sets the output file name. This method must be called before
         * Start() is called.
         *
         * @param aFileName Output file name
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetOutputFileName(const OSCL_wString& aFileName) = 0;

        /**
         * This method sets the output file handle. This method must be called before
         * Start() is called.
         *
         * @param aFileHandle Output file descriptor
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetOutputFileDescriptor(const OsclFileHandle* aFileHandle) = 0;

        /**
         * This method sets the authoring mode. This method must be called before
         * Start() is called.  Default authoring mode is non 3GPPProgressive
         * Downloadable file using no temp files durating authoring.
         *
         * @param aAuthoringMode Authoring mode.
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetAuthoringMode(PVMp4FFCN_AuthoringMode aAuthoringMode = PVMP4FFCN_3GPP_DOWNLOAD_MODE) = 0;

        /**
         * Method to set the sample rate (i.e. timescale) for the overall Mpeg-4 presentation.
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aTimescale Timescale of MPEG4 presentation
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetPresentationTimescale(uint32 aTimescale) = 0;

        /**
         * This method populates the version string. Version string contains
         * information about the version of the author SDK/app that is authoring
         * the clip. Currently only wide char strings are supported.
         */
        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aVersion   version string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetVersion(const OSCL_wString& aVersion, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the title string. Title string contains
         * the title of the authored clip.Currently only wide char strings are supported.
         */


        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aTitle   title string.
         * @param aLangCode OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetTitle(const OSCL_wString& aTitle, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the author string. Title string information about
         * the the author of the clip (actual user - like authored by grandma martha).
         * Currently only wide char strings are supported.
         */
        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aAuthor   author string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetAuthor(const OSCL_wString& aAuthor, const OSCL_String& aLangCode) = 0;


        /**
         * This method populates the copyright string. We do not support the
         * authoring of ISO defined copyright atom yet. This info is used to populate
         * the PV proprietary user data atom.
         */

        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aCopyright   Copyright string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetCopyright(const OSCL_wString& aCopyright, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the description string. Description string contains
         * some brief description of the clip (viz. surfing on the beach). Currently only
         * wide char strings are supported.
         */

        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aDescription   Description string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetDescription(const OSCL_wString& aDescription, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the rating string. Rating string contains
         * some information about the clip rating (viz.PG-13). Currently only
         * wide char strings are supported.
         */
        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aRating   Rating string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetRating(const OSCL_wString& aRating, const OSCL_String& aLangCode) = 0;

        /**
         * This method ests the creation date in ISO 8601 format
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aCreationDate Creation date in ISO 8601 format.
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetCreationDate(const OSCL_wString& aCreationDate) = 0;

        /**
         * This method sets the real time authoring mode where incoming timestamps
         * are adjusted when authoring starts and checks are done to ensure that
         * incremental timstamps do not have 0 or negative deltas. By default, this
         * authoring mode is disabled.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aRealTime  Use real time authoring or not.
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetRealTimeAuthoring(const bool aRealTime) = 0;
        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aAlbum_Title   Album string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetAlbumInfo(const OSCL_wString& aAlbum_Title, const OSCL_String& aLangCode) = 0;
        /* This is an optional configuration API that should be called before Start() is called.
         *
         * @param aRecordingYear   Recoding year.
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetRecordingYear(uint16 aRecordingYear) = 0;
        /**
         * This method populates the performer string. Performer string contains information about
         * the the performer of the clip.
         * Currently only wide char strings are supported.
         * Language code is currently used only in case of IMOTION authoring mode,
         * to populate the IMOTION author info atom.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aPerformer  performer string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetPerformer(const OSCL_wString& aPerformer, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the genre string. Genre string contains information about
         * the the genre of the clip.
         * Currently only wide char strings are supported.
         * Language code is currently used only in case of IMOTION authoring mode,
         * to populate the IMOTION author info atom.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aGenre  genre string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetGenre(const OSCL_wString& aGenre, const OSCL_String& aLangCode) = 0;

        /**
         * This method populates the Classification string. classification string contains information about
         * the the classification of the clip .
         * Currently only wide char strings are supported.
         * Language code is currently used only in case of IMOTION authoring mode,
         * to populate the IMOTION author info atom.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aClassificationInfo   classification information string.
         * @param aClassificationEntity classification Entity string.
         * @param aClassificationTable  classification Table string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetClassification(const OSCL_wString& aClassificationInfo, uint32 aClassificationEntity, uint16 aClassificationTable, const OSCL_String& aLangCode) = 0;
        /**
         * This method populates the keyword string. keyword string contains information about
         * the the keyword of the clip.
         * Currently only wide char strings are supported.
         * Language code is currently used only in case of IMOTION authoring mode,
         * to populate the IMOTION author info atom.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aKeyWordInfo  Keyword information string.
         * @param aLangCode  OSCL_HeapString ISO-639-2/T Language code
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetKeyWord(const OSCL_wString& aKeyWordInfo, const OSCL_String& aLangCode) = 0;
        /**
         * This method populates the LocationInfo string. LocationInfo string contains information about
         * the the Location of the clip.
         *
         * This is an optional configuration API that should be called before Start() is called.
         *
         * @param aLocation_info  Instance of class PvmfAssetInfo3GPPLocationStruct
         * @return Completion status of this method.
         */
        virtual PVMFStatus SetLocationInfo(PvmfAssetInfo3GPPLocationStruct& aLocation_info) = 0;
        /**
        * This method pouplates the aLang string. aLang contains the language code as a heap string and
        * this function is used to convert the lang code into uint16 format and stores the lang code into
        * the authored media file.
        * @param aLang Language code in Heap String format,
        * @return lang_code in a uint16 format.
        */
        virtual uint16 ConvertLangCode(const OSCL_String& aLang) = 0;
};

#endif // PVMP4FFCN_CLIPCONFIG_H_INCLUDED


