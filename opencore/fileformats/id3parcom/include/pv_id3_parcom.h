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
 * @file pv_id3_parcom.h
 * @brief ID3 tag parser-composer utility
 */

#ifndef PV_ID3_PARCOM_H_INCLUDED
#define PV_ID3_PARCOM_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif
#ifndef PVMF_META_DATA_H_INCLUDED
#include "pvmf_meta_data_types.h"
#endif
#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PV_ID3_PARCOM_TYPES_H_INCLUDED
#include "pv_id3_parcom_types.h"
#endif
#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef PV_ID3_PARCOM_CONSTANTS_H_INCLUDED
#include "pv_id3_parcom_constants.h"
#endif

/** Forward declaration */
class PVLogger;

/**
 * PVID3ParCom is an ID3 Tag parser-composer utility. This utiltiy allows user to parse an ID3 tag
 * to retrieve tag information, modify an existing ID3 tag and compose a new ID3 tag.
 */
class PVID3ParCom
{
    public:
        /** Default constructor */
        OSCL_IMPORT_REF PVID3ParCom();

        /** Destructor */
        OSCL_IMPORT_REF ~PVID3ParCom();

        /**
         * @brief Checks the file for the presence of ID3V2 Tags.
         *
         * This function checks the file for the presence of ID3V2 Tag
         * This tag is  denoted by the string "ID3" at the begining of file.
         * This method also reads the Id3 tag size from the file and returns
         * to the caller.
         *
         * @returns True if Id3V2 tag present otherwise False
         * @ aTagSize: size of the ID3 tag, if present, else 0
         */
        OSCL_IMPORT_REF bool IsID3V2Present(PVFile* aFile, uint32& aTagSize);

        /**
         * Gets size of ID3 tag in the specified file without parsing the entire tag for its data.
         *
         * @param aFile File handle of the file containing the ID3 tag
         * @return Size of ID3 tag. Returns 0 if ID3 tag is not found.
         */
        OSCL_IMPORT_REF uint32 GetID3TagSize(PVFile* aFile);

        /**
         * Gets size of ID3 tag in a memory fragment without parsing the entire tag for its data.
         *
         * @param aTag Memory fragment containing the ID3 tag
         * @return Size of ID3 tag. Returns 0 if ID3 tag is not found.
         */
        OSCL_IMPORT_REF uint32 GetID3TagSize(const OsclMemoryFragment& aTag);
        /**
         * Gets the size of a parsed ID3 tag.
         *
         * @return Size of ID3 tag.  Returns 0 if called before an ID3 tag is parsed by ParseID3Tag() call.
         */
        OSCL_IMPORT_REF uint32 GetID3TagSize();

        /**
         * Sets the maximum size of a composed ID3 tag. It is an option to pad the composed ID3 tag
         * to the specified maximum size.  If the composed ID3 tag exceeds the maximum size, one
         * or more ID3 frames data will be truncated.  This API is applicable only for ID3V2 tags.
         *
         * @param aUseMaxSize When set to true, the composed ID3 tag will have a maximum size of
         *                    aMaxSize.  When set to false, the tag will not have a maximum size.
         *                    By default, maximum tag size is disabled by this library.
         * @param aMaxSize Maximum size of the composed ID3 tag in bytes
         * @param aUsePadding When set to true, the composed ID3 tag will be padded to the
         *                    specified maximum size.
         * @return Completion status. Returns PVMFErrNotSupported if ID3V1 is used.
         */
        OSCL_IMPORT_REF PVMFStatus SetMaxID3TagSize(bool aUseMaxSize, uint32 aMaxSize, bool aUsePadding);

        /**
         * Parse an ID3 tag from the specified file.
         *
         * @param aFile File handle of the file containing the ID3 tag to be parsed
         * @return Completion status. If no ID3 tag is found in the file, PVMFFailure is returned.
         */
        OSCL_IMPORT_REF PVMFStatus ParseID3Tag(PVFile* aFile, uint32 buffsize = 100);

        /**
         * Parse an ID3 tag from the specified memory fragment.
         *
         * @param aTag Memory fragment containing the ID3 tag to be parsed
         * @return Completion status. If no ID3 tag is found in the memory fragment, PVMFFailure is returned.
         */
        OSCL_IMPORT_REF PVMFStatus ParseID3Tag(const OsclMemoryFragment& aTag);

        /**
         * Query the version of ID3 used for this tag.
         *
         * @return ID3 version of the tag.
         */
        OSCL_IMPORT_REF PVID3Version GetID3Version() const;

        /**
         * Query if id3v1 present in the clip.
         *
         * @return true if present
         */
        OSCL_IMPORT_REF bool IsID3V1Present() const;

        /**
         * Query if id3v2 present in the clip.
         *
         * @return true if present
         */
        OSCL_IMPORT_REF bool IsID3V2Present() const;

        /**
         * Retrieve a vector of key-value pairs containing data of all ID3 frames in the ID3 tag.
         *
         * @param aFrames Output parameter to store key-value pairs containing data of ID3 frames from the tag.
         * @return Status of completion.
         */
        OSCL_IMPORT_REF PVMFStatus GetID3Frames(PvmiKvpSharedPtrVector& aFrames);

        /**
         * Retrieve an ID3 frame of the specified type in the ID3 tag.
         *
         * @param aFrameType Type of ID3 frame to be retrieved
         * @param aFrame Output parameter to store key-value pair containing data of requested ID3 frame.
         * @return Status of completion. Returns PVMFFailure if a frame of the specified
         *         type is not found.
         */
        OSCL_IMPORT_REF PVMFStatus GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame);

        /**
        *	Check if frame of aFrameType is available
        * @param aFrameType Type of ID3 frame
        * @return true if frame is available otherwise false
        */
        OSCL_IMPORT_REF bool IsID3FrameAvailable(const OSCL_String& aFrameType);

        /**
         * Sets ID3 version to be used.
         *
         * When composing a new ID3 tag, the default ID3 version is v2.4. When modifying
         * an existing ID3 tag, if it is ID3v2.x and is changed to ID3v1.x, ID3 frames
         * that are not supported by ID3v1.x will be removed from the tag and data strings
         * will be trimmed to the maximum lengths specified in ID3v1.x.
         *
         * @param aVersion ID3 version.
         * @return Status of completion.
         */
        OSCL_IMPORT_REF PVMFStatus SetID3Version(PVID3Version aVersion);

        /**
         * Sets an ID3 frame in the ID3 tag.
         *
         * If the frame does not already exist, a new frame will be created and added
         * to the tag.  If a frame of the same type already exists, the frame data will
         * be updated and saved to the tag.
         *
         * @param aFrame ID3 frame to be set in the tag.
         * @return Status of completion.
         */
        OSCL_IMPORT_REF PVMFStatus SetID3Frame(const PvmiKvp& aFrame);

        /**
         * Removes an ID3 frame of the specified type in the ID3 tag.
         *
         * @param aFrameType Type of ID3 frame to be removed.
         * @return Completion status. Returns PVMFFailure if ID3 frame of the specified type
         *         is not found in the tag.
         */
        OSCL_IMPORT_REF PVMFStatus RemoveID3Frame(const OSCL_String& aFrameType);

        /**
         * Compose an ID3 tag and return a shared pointer to the formatted tag to user.
         *
         * ID3 tag is composed using ID3 frames that are set for the tag.  Data strings in ID3 frames are
         * converted from Unicode to the character set specified in the frame when composing the tag.
         * The tag is formatted to the specified ID3 version and written to the memory fragment provided
         * by user.  This method will erase all data from the memory fragment before writing the ID3 tag.
         * If the memory fragment does not contain allocated memory or the capacity of fragment is less
         * than the size of ID3 tag to be written, this method would allocate memory before writing the ID3
         * tag. There must be at least one valid ID3 frame assoicated to the tag to compose successfully.
         *
         * When composing a new ID3 tag (i.e. not editing an existing tag), if no ID3 version is specified
         * through SetID3Version(), it is defaulted to ID3 v2.4.
         *
         * @param aTag Output parameter of memory fragment to which the formatted ID3 tag data will be written.
         * @return Completion status.  Returns PVMFFailure if no ID3 frame is associated with this tag.
         */
        OSCL_IMPORT_REF PVMFStatus ComposeID3Tag(OsclRefCounterMemFrag& aTag);

        /**
         * Reset the parser-composer and remove all ID3 frames.
         *
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus Reset();

        /**
         * This function returns byte offset to the end of id3 tag. Audio frame follows ID3 tag.
         *
         * @return byte offset to end of id3 tag.
         */

        OSCL_IMPORT_REF uint32 GetByteOffsetToStartOfAudioFrames()
        {
            return iByteOffsetToStartOfAudioFrames;
        }

        /**
         * This function looks for ID3 V2.4 tag at the end of file.
         * It will look for ID3 footer starting from the offset till buff_sz. If footer is found, it will get the
         * id3 tag size from the footer header and parse ID3 tag.
         * @param buff_sz size of the buffer to look for footer
         * @param offset in file from the end of file. e.g An offset of 0 means end of file
         * @return Comletion status.
         */

        OSCL_IMPORT_REF PVMFStatus LookForV2_4Footer(uint32 buff_sz, uint32 offset);
        /**
         * This function will look for ID3 footer starting from the offset till buff_sz.
         * @param buff_sz size of the buffer to look for footer
         * @param offset in file from the end of file. e.g An offset of 0 means end of file
         * @return footer location in file otherwise 0 if no footer is found
         */

        uint32 SearchTagV2_4(uint32 buff_sz, uint32 offset);


    public:
        /** Enumerated list of supported character sets */
        typedef enum
        {
            PV_ID3_CHARSET_INVALID = -1,
            PV_ID3_CHARSET_ISO88591 = 0,
            PV_ID3_CHARSET_UTF16    = 1,
            PV_ID3_CHARSET_UTF16BE  = 2,
            PV_ID3_CHARSET_UTF8     = 3,
            PV_ID3_CHARSET_END		= 4,
        } PVID3CharacterSet;

    private:
        /** Enumerated list of ID3 Frame Types */
        typedef enum
        {
            PV_ID3_FRAME_INVALID = -3,
            PV_ID3_FRAME_CANDIDATE,		/**< Frame ID doesn't match with known frame IDs.
										 The frame ID made out of the characters capital A-Z and 0-9.*/
            PV_ID3_FRAME_UNRECOGNIZED,	/** Valid frame ID, but ID3 parcom doesn't support. */
            PV_ID3_FRAME_TITLE,         /**< Title. Supported by all ID3 versions. Maximum 30 characters for
										 ID3v1.x. There is no maximum length limit for ID3v2.x. */
            PV_ID3_FRAME_ARTIST,        /**< Artist. Supported by all ID3 versions. Maximum 30 characters for
										 ID3v1.x. There is no maximum length limit for ID3v2.x*/
            PV_ID3_FRAME_ALBUM,         /**< Album. Supported by all ID3 versions. Maximum 30 characters for
										 ID3v1.x. There is no maximum length limit for ID3v2.x */
            PV_ID3_FRAME_YEAR,          /**< 4-character numeric string with the year of the recording.
										 Supported by all ID3 version. */
            PV_ID3_FRAME_COMMENT,       /**< Comment. Supported by all ID3 versions. Maximum 30 characters for
										 ID3v1.x. There is no maximum length limit for ID3v2.x. */
            PV_ID3_FRAME_TRACK_NUMBER,  /**< Numeric string of track number on Album. Supported by ID3v1.1 and later. */
            PV_ID3_FRAME_GENRE,         /**< Genre of the music. For ID3v1.x, numeric string of genre type value
										 defined at http://www.id3.org/id3v2.3.0.html#secA should be used.
										 ID3v2.x should use the genre name, or refer to ID3v1.x genre type
										 using the format "(genre_number)" where genre_number is a numeric
										 string of ID3v1.x genre type value. */
            PV_ID3_FRAME_COPYRIGHT,      /**< Copyright message. It must begin with a 4-character numeric string of
										 the year of copyright and a space character. Supported by ID3v2.x or later. */
            PV_ID3_FRAME_TRACK_LENGTH,	 /**< Track length. The duration in milliseconds.*/
            PV_ID3_FRAME_DATE,
            PV_ID3_FRAME_PADDED,		 /**< Padding. A frame padded with zero.*/

            PV_ID3_FRAME_PIC,			 /**< PIC. */
            PV_ID3_FRAME_APIC,			 /**< APIC.*/
            PV_ID3_FRAME_LYRICS,			 /**< Unsynchronized Lyrics.*/
            PV_ID3_FRAME_SEEK,			 /* For SEEK frame */
            PV_ID3_FRAME_RECORDING_TIME,  /* RECORDING TIME */
            PV_ID3_FRAME_AUTHOR,		  /* author or writer*/
            PV_ID3_FRAME_COMPOSER,		  /* music composer */
            PV_ID3_FRAME_DESCRIPTION,	  /* brief description of the content */
            PV_ID3_FRAME_VERSION,		  /* Software version of the authoring software */
            PV_ID3_FRAME_PART_OF_SET,     /* Which part of a set this belongs to */

            PV_ID3_FRAME_EEND			 /**< End. Marks end of frames.*/
        } PVID3FrameType;

        /**
         *  @brief The TID3TagInfo class acts as temporary storage for information
         *  extracted from the different types of ID3 tags to a given ID3tag file. This
         *  class is created to keep track of certain qualities of an ID3 tag that
         *  the caller of the ID3 library would not need to know.
         */
        struct TID3TagInfo
        {
            uint32      iPositionInFile;
            PVID3CharacterSet iTextType;

            uint32    iID3V2FrameSize;
            uint32	  iID3V2SeekOffset;
            uint8     iID3V2TagFlagsV2;
            uint32    iID3V2ExtendedHeaderSize;
            uint32    iID3V2TagSize;
            uint8     iID3V2FrameID[5];
            uint8     iID3V2FrameFlag[2];
            bool	  iFooterPresent;
            uint8	  iID3V2LanguageID[4];

        };

        /**
         * @brief Checks the ID3 Tag v2.4 frame size whether it is syncsafe or not.
         *
         * This function validates next ID3 frame to check whether frame size is
         * syncsafe.
         *
         * @returns Validated frame size
         */
        uint32 ValidateFrameLengthV2_4(uint32 aFrameSize);

        /**
         * @brief Checks the ID3 Tag v2.4 frame is valid or not.
         *
         * This function validates ID3 frame type and then, frame size and flags
         *
         * @params
         * [IN] bUseSyncSafeFrameSize - validate syncsafe/non-syncsafe frame size
         * [OUT] frameType - frame type for frame currently validated
         *
         * @returns validation status
         */
        bool ValidateFrameV2_4(PVID3FrameType& frameType, bool bUseSyncSafeFrameSize = true);

        /**
         * @brief Checks the file for the presence of ID3V1 Tag.
         *
         * This function checks the file for the presence of ID3V1 Tag. This tag is
         * denoted by the string "TAG" starting 128 bytes in from the end-of-file.
         *
         * @returns True if Id3V1 tag present otherwise False
         */
        bool CheckForTagID3V1();

        /**
         * @brief Checks the file for the presence of ID3V2 Tag attached to
         * the beginning of the ID3tag file.
         *
         * This function checks the file for the presence of ID3V2 Tag attached to
         * the beginning of the ID3 file. This tag is  denoted by the string "ID3"
         * at the very beginning of the ID3 file.
         *
         * @returns True if Id3V2 tag present otherwise False
         */
        bool CheckForTagID3V2();

        /**
         * @brief Parses the file and populates the frame vector when an ID3V1
         * tag is found and no ID3V2 tag was found.
         *
         * This function parses the file and populates the frame vector when
         * an ID3V1 tag is found to be attached to the ID3 file and no ID3V2 tag was
         * found. The data in the ID3V1 tag can only be ASCII, so the ConvertToUnicode
         * must be called for all fields read from the ID3V1 tag.
         *
         * @param None
         * @returns None
         */
        void ReadID3V1Tag();

        /**
         * @brief reads id3 V2 tag header.
         * @param aReadTags, read the id3 frames when true, else just reads the header
         * @@returns true if tags were read successfully
         */
        bool ReadHeaderID3V2(bool aReadTags = true);

        /**
         * @brief Parses the ID3V2 tag frames from the file and populates the
         * frame vector when an ID3V2 tag is found
         * @param version ID3 V2 sub version number (v2.2, v2.3 or v2.4)
         * * @returns number of tags successfully parsed
         */
        int ReadTagID3V2(PVID3Version version);

        /**
         * @brief Parses the frame and populates the iID3TagInfo structure
         * when an ID3V2 tag is found.
         *
         * This function parses the ID3 tag frames from the file and populates the
         * frame vector when an ID3V2 tag is found attached to the start
         * of a ID3 file.
         *
         * @param version ID3 V2 sub version number (v2.2, v2.3 or v2.4)
         * @returns None
         */
        void ReadFrameHeaderID3V2(PVID3Version version);

        /**
        * @brief Parses the frame and populates the iID3TagInfo structure
        * when an ID3V2_2 tag is found.
        *
        * This function parses the ID3 tag frames from the file and populates the
        * frame vector when an ID3V2_2 tag is found attached to the start
        * of a ID3 file.
        *
        * @param None
        * @returns None
        */
        void ReadFrameHeaderID3V2_2();

        /**
         * @brief Receives a ID3V2 frame and handles it accordingly. All of the frames
         * sent to this function have ASCII strings in them.
         *
         * This function receives a ID3V2 frame and handles it accordingly. Each
         * frame header is composed of 10 bytes - 4 bytes for FrameID, 4 bytes for
         * FrameSize, and 2 bytes for FrameFlags, respectively. Once we have a
         * frame's unique FrameID, we can handle the frame accordingly. (eg. The
         * frame with FrameID = "TIT2" contains data describing the title of an ID3
         * file. Clearly, the data in that frame should be copied to it respective
         * location in frame vector. The location for the data with FrameID = "TIT2"
         * goes in the Title frame.
         *
         * @param aFrameType Enumerated TFrameTypeID3V2 type; used to determine if
         * Frame is supported
         * @param aPos Position in file where frame data begins
         * @param aSize Size of the frame data
         * @returns None
         */
        void HandleID3V2FrameDataASCII(PVID3FrameType aframeType,
                                       uint32 aPos,
                                       uint32 aSize);

        /**
         * @brief Receives a ID3V2 frame and handles it accordingly. All of the frames
         * sent to this function have unicode strings in them.
         *
         * This function receives a ID3V2 frame and handles it accordingly. Each
         * frame header is composed of 10 bytes - 4 bytes for FrameID, 4 bytes for
         * FrameSize, and 2 bytes for FrameFlags, respectively. Once we have a
         * frame's unique FrameID, we can handle the frame accordingly. (eg. The
         * frame with FrameID = "TIT2" contains data describing the title of an ID3
         * file. Clearly, the data in that frame should be copied to it respective
         * location in Frame vector. The location for the data with FrameID = "TIT2"
         * goes in the Title frame.iTitle buffer. All of the frames
         * sent to this function have unicode strings in them that will need to be
         * read different than the ASCII strings.
         *
         * @param aFrameType Enumerated TFrameTypeID3V2 type; used to determine if
         * Frame is supported
         * @param aPos Position in file where frame data begins
         * @param aSize Size of the frame data
         * @param aEndianType Flag indicator regarding the text endian type
         * @returns None
         */
        void HandleID3V2FrameDataUnicode16(PVID3FrameType aframeType,
                                           uint32 aPos,
                                           uint32 aSize,
                                           uint32 aEndienType);

        /**
         * @brief Receives a ID3V2 frame and handles it accordingly. All of the frames
         * sent to this function have UTF8 strings in them.
         *
         * This function receives a ID3V2 frame and handles it accordingly. Each
         * frame header is composed of 10 bytes - 4 bytes for FrameID, 4 bytes for
         * FrameSize, and 2 bytes for FrameFlags, respectively. Once we have a
         * frame's unique FrameID, we can handle the frame accordingly. (eg. The
         * frame with FrameID = "TIT2" contains data describing the title of an ID3
         * file. Clearly, the data in that frame should be copied to it respective
         * location in Frame vector. The location for the data with FrameID = "TIT2"
         * goes in the Title frame. All of the frames sent to this function have UTF8
         * strings in them that will need to be read different than the ASCII strings.
         *
         * @param aFrameType Enumerated TFrameTypeID3V2 type; used to determine if
         * Frame is supported
         * @param aPos Position in file where frame data begins
         * @param aSize Size of the frame data
         * @returns None
         */
        void HandleID3V2FrameDataUTF8(PVID3FrameType aframeType,
                                      uint32 aPos,
                                      uint32 aSize);


        /**
         * @brief This function handls id3 frames that are not currently supported by
         * the id3parcom library. It stores the frame alongwith the header as it is read from the file.
         * @param aFrameType Enumerated TFrameTypeID3V2 type; It will be PV_ID3_FRAME_UNSUPPORTED
         * @param aPos Position in file where frame data begins
         * @param aSize Size of the frame data
         * @returns None
         */

        void HandleID3V2FrameUnsupported(PVID3FrameType aframeType,
                                         uint32 aPos,
                                         uint32 aSize);

        /**
         * @brief Detects the ID3V2FrameType and returns the enum value that
         * corresponds to the current frame.
         *
         * @param None
         * @returns Value that describes the current frame of type PVID3FrameType
         */
        PVID3FrameType FrameSupportedID3V2(PVID3Version version, uint8* aframeid = NULL);

        /**
         * @brief Detects the ID3V2FrameType and fine whether it us unsupported or
         * invalid frame type.
         *
         * @param None
         * @returns Value that describes the current frame of type PVID3FrameType
         */
        PVID3FrameType FrameValidatedID3V2_4(uint8* aFrameID);

        /**
         * @brief Detects the ID3V 2.2 FrameType and returns the enum value that
         * corresponds to the current frame.
         *
         * @param None
         * @returns Value that describes the current frame of type PVID3FrameType
         */
        PVID3FrameType FrameSupportedID3V2_2(void);

        /**
         * @brief Converts the data in the TBuf8 buffer and puts the data into the
         * TBuf16 buffer
         *
         * @param aPtrFrameData8 Pointer to intput string format of either big
         * or little endian.
         * @param aSize Number of character elements in aPtrFrameData8
         * @param aEndianType Indicates if the encoded Unicode text in the
         * aPtrFrameData8 buffer is in big or little endian.
         * @param aPtrFrameData16 Pointer to the output string in unicode format.
         * @returns None
         */
        uint32 EightBitToWideCharBufferTransfer(const uint8* aPtrFrameData,
                                                uint32 aSize,
                                                uint32 aEndianType,
                                                oscl_wchar* aPtrFrameData16);


        //convert an interger to Synchsafe integer
        uint32 ConvertToSyncSafeInt(uint32 src);
        uint32 SafeSynchIntToInt32(uint32 SafeSynchInteger);

        /**
         * @brief Read in byte data and take most significant byte first
         *
         * @param aInputFile File handle to input file
         * @param data Reference to output data buffer where the data read from file will be stored
         * @return true if the read is successful
         */
        bool readByteData(PVFile* aInputFile, uint32 length, uint8 *data);

        /**
         * @brief Read in the 32 bits byte by byte and take most significant byte first
         *
         * @param aInputFile File handle to input file
         * @param data Reference to output data buffer where the data read from file will be stored
         * @return true if the read is successful
         */
        bool read32(PVFile* aInputFile, uint32 &data);

        /**
         * @brief Read in the 24 bits byte by byte and take most significant byte first
         *
         * @param aInputFile File handle to input file
         * @param data Reference to output data buffer where the data read from file will be stored
         * @return true if the read is successful
         */
        bool read24(PVFile* aInputFile, uint32 &data);

        /**
         * @brief Read in 8 bytes from specified input file
         *
         * @param aInputFile File handle to input file
         * @param data Reference to output data buffer where the data read from file will be stored
         * @return true if the read is successful
         */
        bool read8(PVFile* aInputFile, uint8 &data);

        /**
         * @brief Read in 16 bytes from specified input file
         *
         * @param aInputFile File handle to input file
         * @param data Reference to output data buffer where the data read from file will be stored
         * @return true if the read is successful
         */

        bool read16(PVFile* aInputFile, uint16 &data);

        /**
         * @breif reads information stored in id3 frame
         * @param unicode tells if the data is unicode
         * @param frameType type of frame
         * @param pos position in file
         * @param currFrameLength length of the frame
         */
        bool  ReadFrameData(uint8 unicode, PVID3FrameType frameType, uint32 pos, uint32 currFrameLength);
        /**
         * @brief reads extended header if present in id3 tag header
         * @param none
         * @return true if success.
         */
        bool ReadExtendedHeader();

        /**
         * @brief allocate memeory for key value pair structure.
         * @param aKey key string
         * @param aValueType type of key value
         * @param aValueSize size of key value
         * @param truncate bool type indicates if the frame was truncated.
         * @return shared pointer to key value pair structure
         */
        PvmiKvpSharedPtr AllocateKvp(OSCL_String& aKey, PvmiKvpValueType aValueType, uint32 aValueSize, bool &truncate);

        /**
         * @brief forms key string for id3 frames
         * @param aKey key string
         * @param aType frame type
         * @param aCharSet text encoding type (unicode, ascii)
         * @return Completion status
         */
        PVMFStatus ConstructKvpKey(OSCL_String& aKey, PVID3FrameType aType, PVID3CharacterSet aCharSet);

        /**
         * @brief reads string data in frames
         * @param aFrameType frame type
         * @param aCharSet text encoding
         * @param aValueSize size of the frame data
         * @return Completion status
         */
        PVMFStatus ReadStringValueFrame(PVID3FrameType aFrameType, PVID3CharacterSet aCharSet, uint32 aValueSize);

        /**
         * @brief reads frame of type track length
         * @param aValueSize size of the frame
         * @param aCharSet text encoding
         * @return completion status
         */
        PVMFStatus ReadTrackLengthFrame(uint32 aValueSize, PVID3CharacterSet aCharSet);

        /**
         * @brief reads ID3 frames
         * @param aFrameType frame type
         * @param aValueSize size of the frame data
         * @return Completion status
         */

        PVMFStatus ReadFrame(PVID3FrameType aFrameType, uint32 aValueSize);


        /**
         * @brief This function is costructing the key-value pair for AlbumArt(apic/pic) frame. It
         * first converts the AlbumArt frame in to the PvmfApicStruct and then set the
         * key_specific_value of the Pvmikvp struct.
         * @param frame type
         * @param unicode
         * @param frame size
         * @return Success if read successfull.
         */
        PVMFStatus ReadAlbumArtFrame(PVID3FrameType aFrameType, uint8 unicode, uint32 aFrameSize);

        /**
         * @brief This function extracts the albumart information, for ascii data, based on frametype (apic/pic).
         * @param frame type
         * @param frame size
         * @param Image format
         * @param description
         * @param picture type
         * @param data length
         * @return Success if conversion is successfull.
         	*/
        PVMFStatus GetAlbumArtInfo(PVID3FrameType aFrameType, uint32 aFrameSize, OSCL_HeapString<OsclMemAllocator> &ImageFormat,
                                   uint8 &PicType, OSCL_HeapString<OsclMemAllocator> &Description, 	uint32 &DataLen);
        /**
         * @brief This function extracts the albumart information, for unicode data, based on frametype (apic/pic).
         * @param frame type
         * @param frame size
         * @param Image format
         * @param description
         * @param picture type
         * @param data length
         * @return Success if conversion is successfull.
         */
        PVMFStatus GetAlbumArtInfo(PVID3FrameType aFrameType, uint32 aFrameSize, OSCL_HeapString<OsclMemAllocator> &ImageFormat,
                                   uint8 &PicType, OSCL_wHeapString<OsclMemAllocator> &aDescription, uint32 &DataLen);



        /**
         * @brief This function does the actual conversion from the raw ascii data from the
         * GetAlbumArtInfo function to the PvmfApicStruct.
         * @param MimeType
         * @param Description
         * @param GraphicType
         * @param GraphicDataLen
         * @param pointer to memory allocated to store kvp information
         * @return Success if conversion is successfull.
         */
        PVMFStatus ConvertUnicodeDataToApic(char* aMimeTypeID3, oscl_wchar* aDescriptionID3,
                                            uint8 aGraphicType, uint32 aGraphicDataLen,
                                            void *key_specific_value, uint32 total_size);

        /**
         * @brief This function does the actual conversion from the raw unicode data from the
         * ConvertPictureToAPIC function to the PvmfApicStruct.
         * @param MimeType
         * @param Description
         * @param GraphicType
         * @param GraphicOffset
         * @param GraphicDataLen
         * @param pointer to memory allocated to store kvp information
         * @return Success if conversion is successfull.
         */
        PVMFStatus ConvertAsciiDataToApic(char* aMimeTypeID3, char* aDescriptionID3,
                                          uint8 aGraphicType, uint32 aGraphicDataLen,
                                          void *key_specific_value, uint32 total_size);

        /**
         * @brief Read the information from the USLT/ULT frames.
         * @param NULL
         * @return Success if successfully read.
         */
        PVMFStatus ReadLyricsCommFrame(uint8 unicodeCheck, uint32 aFramesize, PVID3FrameType aFrameType);


        /**
         * @brief Reads Null terminated ascii strings from file
         * @param aInputFile file pointer
         * @parm data heap string to store string
         * @return true on success otherwise false
         */
        bool readNullTerminatedAsciiString(PVFile* aInputFile, OSCL_HeapString<OsclMemAllocator> &data);

        /**
         * @brief Reads Null terminated unicode strings from file
         * @param aInputFile file pointer
         * @parm data heap string to store string
         * @return completion status
         */

        PVMFStatus readNullTerminatedUnicodeString(PVFile* aInputFile, OSCL_wHeapString<OsclMemAllocator> &aData, uint32 &bomSz);

        /**
         * @brief reads frame of type comment
         * @param aValueSize size of the frame
         * @return completion status
         */

        PVMFStatus ReadCommentFrame(uint32 aValueSize);

        /**
         * @brief Gets value type and char set stored in kvp
         * @param aKvp shared pointer to kvp
         * @param aValueType value type
         * @param aCharSet char set
         * @return Completion status.
         */
        PVMFStatus GetKvpValueType(PvmiKvpSharedPtr aKvp, PvmiKvpValueType& aValueType, PVID3CharacterSet& aCharSet);

        /**
         * @brief Gets frame ID and frame type from kvp
         * @param aKvp  shared pointer to key value pair structure
         * @param aFrameID frame ID
         * @param aFrameType frame type
         * @return Completion status.
         */
        PVMFStatus GetFrameTypeFromKvp(PvmiKvpSharedPtr aKvp, OSCL_String& aFrameID, PVID3FrameType& aFrameType);

        /**
         * @brief Gets frame ID and frame type from kvp
         * @param aKvp  key value pair structure
         * @param aFrameID frame ID
         * @param aFrameType frame type
         * @return Completion status.
         */
        PVMFStatus GetFrameTypeFromKvp(const PvmiKvp& aKvp, OSCL_String& aFrameID, PVID3FrameType& aFrameType);

        /**
         * @brief Get Size of ID3 frame stored in kvp.
         * @param akvp shared pointer to kvp
         * @param aValueType type of value stored in kvp
         * @param aCharSet character set of value
         * @param aSize size of the frame
         * @return Completion Status
         */
        PVMFStatus GetID3v2FrameDataSize(PvmiKvpSharedPtr aKvp, PvmiKvpValueType aValueType,
                                         PVID3CharacterSet aCharSet, uint32& aSize);

        /**
         * @brief compose ID3 V2 tag
         * @param aTag reference counter to memory fragment
         * @return Completion Status
         */
        PVMFStatus ComposeID3v2Tag(OsclRefCounterMemFrag& aTag);

        /**
         * @brief push frame to supplied frame vector
         * @param aFrame Frame to be pushed
         * @param aFrameVector vector in which frame is to be pushed
         * @return Completion Status
         */
        PVMFStatus PushFrameToFrameVector(PvmiKvpSharedPtr& aFrame, PvmiKvpSharedPtrVector& aFrameVector);
        /**
         * @brief Allocate array of specified size and type of elements
         * @param aValueType, type of array elements
         * @param aNumElements, size of required array
         * @return aLeaveCode, err code
         * @return buffer, pointer to allocated buffer
         */
        OsclAny* AllocateValueArray(int32& aLeaveCode, PvmiKvpValueType aValueType, int32 aNumElements, OsclMemAllocator* aMemAllocator = NULL);

        /**
         * @brief Allocate kvp and handle leave
         * @param aValueType, type of array elements
         * @param aValueSize, value size of required kvp
         * @return aStatus, err code
         * @return truncate, returns false if specified memory is not allocated, else true
         */
        PvmiKvpSharedPtr HandleErrorForKVPAllocation(OSCL_String& aKey, PvmiKvpValueType aValueType, uint32 aValueSize, bool &truncate, PVMFStatus &aStatus);

    private:

        // Variables for parsing
        PVFile* iInputFile;
        TID3TagInfo iID3TagInfo;
        bool iTitleFoundFlag;
        bool iArtistFoundFlag;
        bool iAlbumFoundFlag;
        bool iYearFoundFlag;
        bool iCommentFoundFlag;
        bool iTrackNumberFoundFlag;
        bool iGenereFoundFlag;
        int32 iFileSizeInBytes;
        uint32 iByteOffsetToStartOfAudioFrames;

        bool iID3V1Present;
        bool iID3V2Present;
        // Variables for composing
        PVID3Version iVersion;
        PvmiKvpSharedPtrVector iFrames;
        bool iUseMaxTagSize;
        uint32 iMaxTagSize;
        bool iUsePadding;
        bool iTagAtBof;
        bool iSeekFrameFound;

        OsclMemAllocator iAlloc;
        PVLogger* iLogger;
};

#endif // PV_ID3_PARCOM_H_INCLUDED


