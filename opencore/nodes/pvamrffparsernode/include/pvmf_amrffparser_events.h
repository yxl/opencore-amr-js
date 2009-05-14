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
#ifndef PVMF_AMRFFPARSER_EVENTS_H_INCLUDED
#define PVMF_AMRFFPARSER_EVENTS_H_INCLUDED

/**
 UUID for PV AMR FF parser node error and information event type codes
 **/
#define PVMFAMRParserNodeEventTypesUUID PVUuid(0x13f4c2f2,0x2c66,0x46ac,0x95,0x1d,0x7f,0x86,0xb7,0x58,0x14,0x67)

/**
 * An enumeration of error types from PV AMR FF parser node
 **/
typedef enum
{
    /**
     When AMR FF reports error READ_BITRATE_MUTUAL_EXCLUSION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrBitRateMutualExclusionObjectFailed = 1024,

    /**
     When AMR FF reports error READ_BITRATE_RECORD_FAILED
    **/
    PVMFAMRFFParserErrBitRateRecordReadFailed,

    /**
     When AMR FF reports error READ_CODEC_ENTRY_FAILED
    **/
    PVMFAMRFFParserErrCodecEntryReadFailed,

    /**
     When AMR FF reports error READ_CODEC_LIST_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrCodecListObjectReadFailed,

    /**
     When AMR FF reports error READ_CONTENT_DESCRIPTION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrContentDescriptionObjectReadFailed,

    /**
     When AMR FF reports error READ_CONTENT_DESCRIPTOR_FAILED
    **/
    PVMFAMRFFParserErrContentDescriptorReadFailed,

    /**
     When AMR FF reports error READ_DATA_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrDataObjectReadFailed,

    /**
     When AMR FF reports error READ_DATA_PACKET_FAILED
    **/
    PVMFAMRFFParserErrDataPacketReadFailed,

    /**
     When AMR FF reports error INCORRECT_ERROR_CORRECTION_DATA_TYPE
    **/
    PVMFAMRFFParserErrIncorrectErrorCorrectionDataType,

    /**
     When AMR FF reports error OPAQUE_DATA_NOT_SUPPORTED
    **/
    PVMFAMRFFParserErrOpaqueDataNotSupported,

    /**
     When AMR FF reports error READ_DATA_PACKET_PAYLOAD_FAILED
    **/
    PVMFAMRFFParserErrDataPacketPayloadReadFailed,

    /**
     When AMR FF reports error ZERO_OR_NEGATIVE_SIZE
    **/
    PVMFAMRFFParserErrZeroOrNegativeSize,

    /**
     When AMR FF reports error READ_ERROR_CORRECTION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrErrorCorrectionObjectReadFailed,

    /**
     When AMR FF reports error READ_EXTENDED_CONTENT_DESCRIPTION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrExtendedContentDescriptionObjectReadFailed,

    /**
     When AMR FF reports error READ_FILE_PROPERTIES_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrFilePropertiesObjectReadFailed,

    /**
     When AMR FF reports error INVALID_FILE_PROPERTIES_OBJECT_SIZE
    **/
    PVMFAMRFFParserErrInvalidFilePropertiesObjectSize,

    /**
     When AMR FF reports error INVALID_DATA_PACKET_COUNT
    **/
    PVMFAMRFFParserErrInvalidDataPacketCount,

    /**
     When AMR FF reports error INVALID_PACKET_SIZE
    **/
    PVMFAMRFFParserErrInvalidDataPacketSize,

    /**
     When AMR FF reports error READ_HEADER_EXTENSION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrHeaderExtensionObjectReadFailed,

    /**
     When AMR FF reports error RES_VAL_IN_HEADER_EXTENSION_OBJ_INCORRECT
    **/
    PVMFAMRFFParserErrReservedValueInHeaderExtensionObjectIncorrect,

    /**
     When AMR FF reports error READ_HEADER_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrHeaderObjectReadFailed,

    /**
     When AMR FF reports error MANDATORY_HEADER_OBJECTS_MISSING
    **/
    PVMFAMRFFParserErrMandatoryHeaderObjectsMissing,

    /**
     When AMR FF reports error NO_STREAM_OBJECTS_IN_FILE
    **/
    PVMFAMRFFParserErrNoStreamObjectsInFile,

    /**
     When AMR FF reports error RES_VALUE_IN_HDR_OBJECT_INCORRECT
    **/
    PVMFAMRFFParserErrReservedValueInHeaderObjectIncorrect,

    /**
     When AMR FF reports error DUPLICATE_OBJECTS
    **/
    PVMFAMRFFParserErrDuplicateObjects,

    /**
     When AMR FF reports error ZERO_OR_NEGATIVE_OBJECT_SIZE
    **/
    PVMFAMRFFParserErrZeroOrNegativeObjectSize,

    /**
     When AMR FF reports error READ_SCRIPT_COMMAND_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrScriptCommandObjectReadFailed,

    /**
     When AMR FF reports error READ_PADDING_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrPaddingObjectReadFailed,

    /**
     When AMR FF reports error READ_MARKER_FAILED
    **/
    PVMFAMRFFParserErrMarkerReadFailed,

    /**
     When AMR FF reports error READ_MARKER_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrMarkerObjectReadFailed,

    /**
     When AMR FF reports error READ_STREAM_BITRATE_PROPERTIES_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrStreamBitRatePropertiesObjectReadFailed,

    /**
     When AMR FF reports error READ_STREAM_PROPERTIES_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrStreamPropertiesObjectReadFailed,

    /**
     When AMR FF reports error INVALID_STREAM_PROPERTIES_OBJECT_SIZE
    **/
    PVMFAMRFFParserErrInvalidStreamPropertiesObjectSize,

    /**
     When AMR FF reports error INVALID_STREAM_NUMBER
    **/
    PVMFAMRFFParserErrInvalidStreamNumber,

    /**
     When AMR FF reports error READ_SIMPLE_INDEX_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrSimpleIndexObjectReadFailed,

    /**
     When AMR FF reports error READ_INDEX_ENTRY_FAILED
    **/
    PVMFAMRFFParserErrIndexEntryReadFailed,

    /**
     When AMR FF reports error NO_MEDIA_STREAMS
    **/
    PVMFAMRFFParserErrNoMediaStreams,

    /**
     When AMR FF reports error READ_UNKNOWN_OBJECT
    **/
    PVMFAMRFFParserErrReadUnknownObject,

    /**
     When AMR FF reports error ASF_FILE_OPEN_FAILED
    **/
    PVMFAMRFFParserErrFileOpenFailed,

    /**
     When AMR FF reports error ASF_SAMPLE_INCOMPLETE
    **/
    PVMFAMRFFParserErrIncompleteASFSample,

    /**
     When AMR FF reports error PARSE_TYPE_SPECIFIC_DATA_FAILED
    **/
    PVMFAMRFFParserErrParseTypeSpecificDataFailed,

    /**
     When AMR FF reports error END_OF_MEDIA_PACKETS
    **/
    PVMFAMRFFParserErrEndOfMediaPackets,

    /**
     When AMR FF reports error READ_CONTENT_ENCRYPTION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrContentEncryptionObjectReadFailed,

    /**
     When AMR FF reports error READ_EXTENDED_CONTENT_ENCRYPTION_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrExtendedContentEncryptionObjectReadFailed,

    /**
     When AMR FF reports error READ_INDEX_SPECIFIER_FAILED
    **/
    PVMFAMRFFParserErrIndexSpecifierReadFailed,

    /**
     When AMR FF reports error READ_INDEX_BLOCK_FAILED
    **/
    PVMFAMRFFParserErrIndexBlockReadFailed,

    /**
     When AMR FF reports error READ_INDEX_OBJECT_FAILED
    **/
    PVMFAMRFFParserErrIndexObjectReadFailed,

    PVMFAMRFFParserErrUnableToOpenFile,
    PVMFAMRFFParserErrUnableToRecognizeFile,
    PVMFAMRFFParserErrUnableToCreateASFFileClass,
    PVMFAMRFFParserErrTrackMediaMsgAllocatorCreationFailed,
    PVMFAMRFFParserErrUnableToPopulateTrackInfoList,
    PVMFAMRFFParserErrInitMetaDataFailed,

    /**
     Placeholder for the last PV AMR FF parser error event
     **/
    PVMFAMRFFParserErrLast = 8191
} PVMFAMRFFParserErrorEventType;

/**
 * An enumeration of informational event types from PV ASF FF parser node
 **/
typedef enum
{
    /**
     Placeholder for the last PV ASF FF parser informational event
     **/
    PVMFAMRFFParserInfoLast = 10000

} PVMFAMRFFParserInformationalEventType;

#endif // PVMF_ASFFFPARSER_EVENTS_H_INCLUDED


