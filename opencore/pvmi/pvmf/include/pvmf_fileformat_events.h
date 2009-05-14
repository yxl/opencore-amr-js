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
#ifndef PVMF_FILEFORMAT_EVENTS_H_INCLUDED
#define PVMF_FILEFORMAT_EVENTS_H_INCLUDED

/**
 UUID for file format error and information event type codes
 **/
#define PVMFFileFormatEventTypesUUID PVUuid(0x1ccf06b0,0xb6ea,0x4c29,0x9d,0xbe,0xca,0x0a,0x8c,0xc5,0x80,0x1a)

/**
 * An enumeration of error types from file format libraries and nodes
 **/
typedef enum
{
    /**
     File open error
    **/
    PVMFFFErrFileOpen = 1024,

    /**
     File read error
    **/
    PVMFFFErrFileRead,

    /**
     File seek error
    **/
    PVMFFFErrFileSeek,

    /**
     Parsed data is invalid
    **/
    PVMFFFErrInvalidData,

    /**
     Insufficient buffer to read in the data
    **/
    PVMFFFErrInsufficientBuffer,

    /**
     Insufficient data available for the specified type
    **/
    PVMFFFErrInsufficientData,

    /**
     Specified type of data is not supported
    **/
    PVMFFFErrNotSupported,

    /**
     Unknown data type
    **/
    PVMFFFErrUnknownData,

    /**
     Miscellaneous error
    **/
    PVMFFFErrMisc,

    /**
     Placeholder for the last PVMF FF error event
     **/
    PVMFFFErrLast = 8191
} PVMFFileFormatErrorEventType;

/**
 * An enumeration of informational event types from file format libraries and nodes
 **/
typedef enum
{
    /**
     End of file detected
     **/
    PVMFFFInfoEndOfFile	= 8192,

    /**
     Not supported so skipped
     **/
    PVMFFFInfoNotSupported,

    /**
     Placeholder for the last PVMF FF informational event
     **/
    PVMFFFInfoLast = 10000

} PVMFFileFormatInformationalEventType;

#endif // PVMF_FILEFORMAT_EVENTS_H_INCLUDED


