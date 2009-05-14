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
 * This PVA_FF_AtomUtils Class contains sime useful methods for operating on Atoms
 */

#ifndef __AtomUtils_H__
#define __AtomUtils_H__

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

typedef const OSCL_String& PVA_FF_UTF8_STRING_PARAM;
typedef OSCL_HeapString<OsclMemAllocator> PVA_FF_UTF8_HEAP_STRING;

typedef const OSCL_wString& PVA_FF_UNICODE_STRING_PARAM;
typedef OSCL_wHeapString<OsclMemAllocator> PVA_FF_UNICODE_HEAP_STRING;

typedef Oscl_File* MP4_AUTHOR_FF_FILE_HANDLE;

#define PV_MP4_FF_NEW(auditCB,T,params,ptr)\
{\
ptr = OSCL_NEW(T,params);\
}


#define PV_MP4_FF_DELETE(auditCB,T,ptr)\
{\
OSCL_DELETE(ptr);\
}

#define PV_MP4_FF_TEMPLATED_DELETE(auditCB,T,Tsimple,ptr)\
{\
OSCL_DELETE(ptr);\
}


#define PV_MP4_FF_ARRAY_MALLOC(auditCB,T,count,ptr)\
{\
	ptr = (T*)OSCL_MALLOC(count);\
}


#define PV_MP4_ARRAY_FREE(auditCB,ptr)\
{\
	OSCL_FREE(ptr);\
}

#define PV_MP4_FF_ARRAY_NEW(auditCB, T, count, ptr)\
{\
	ptr = OSCL_ARRAY_NEW(T, count);\
}

#define PV_MP4_ARRAY_DELETE(auditCB, ptr)\
{\
	OSCL_ARRAY_DELETE(ptr);\
}

typedef struct
{
    MP4_AUTHOR_FF_FILE_HANDLE _filePtr;
    Oscl_FileServer* _osclFileServerSession;

} MP4_AUTHOR_FF_FILE_IO_WRAP;

class PVA_FF_AtomUtils
{

    public:
        // Methods for rendering data to a file stream
        static bool render64(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint64 data);
        static bool render32(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 data);
        static bool render24(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 data);
        static bool render16(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint16 data);
        static bool render8(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint8 data);
        static bool renderString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UTF8_STRING_PARAM data);
        static bool renderUnicodeString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UNICODE_STRING_PARAM data);
        static bool renderNullTerminatedString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UTF8_STRING_PARAM data);
        static bool renderNullTerminatedUnicodeString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UNICODE_STRING_PARAM data);
        static bool renderByteData(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 length, uint8 *data);

        // Method to set time value in seconds since 1904
        static void setTime(uint32& ulTime);

        // This method is used to calculate the number of bytes needed to store the
        // overall size of the class - the value contentSize is the size of the class
        // NOT including the actual _sizeOfClass field (since this is a variable-length
        // field).  This is used when determining the actual _sizeOfClass value for
        // all the Descriptor and Command classes.
        static uint32 getNumberOfBytesUsedToStoreSizeOfClass(uint32 contentSize);
        static uint32 getNumberOfBytesUsedToStoreContent(uint32 sizeOfClass);

        static void   seekFromCurrPos(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n);
        static void   seekFromStart(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n);
        static void   seekToEnd(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        static void   rewindFilePointerByN(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n);
        static int32  getCurrentFilePosition(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        static bool openFile(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UNICODE_STRING_PARAM fileName, int32 mode, uint32 aCacheSize = 0);
        static void closeFile(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        static void closeFileSession(Oscl_FileServer* fs);
        static bool readByteData(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 length, uint8 *data);
};

#endif
