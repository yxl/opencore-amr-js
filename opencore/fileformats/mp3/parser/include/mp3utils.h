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
#ifndef MP3_UTILS_H_INCLUDED
#define MP3_UTILS_H_INCLUDED

//#include "oscl_string.h"

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif

#ifndef IMP3FF_H_INCLUDED
#include "imp3ff.h"
#endif


typedef Oscl_File* MP3_FF_FILE_REFERENCE;

class MP3_FF_FILE
{
    public:
        MP3_FF_FILE()
        {
            _Ptr = NULL;
            _Size = 0;
            auditCB = NULL;
            _streamID = 0;
            int32 ret = -1;
            ret = _fileServSession.Connect();
            if (ret == 0)
                _fileServSessionConnected = true;	// connect success
            else
                _fileServSessionConnected = false;

        }

        MP3_FF_FILE(const MP3_FF_FILE& a)
        {
            _Ptr = a._Ptr;
            _Size = a._Size;
            auditCB = a.auditCB;
            _pvfile = a._pvfile;
            _streamID = a._streamID;
        }

        ~MP3_FF_FILE()
        {
            _Ptr = NULL;
            _Size = 0;
            auditCB = NULL;
            _streamID = 0;
            if (_fileServSessionConnected == true) 		// Still connected
            {
                _fileServSession.Close();				// return from Close is always 0.
                _fileServSessionConnected = false;
            }
        }
        /**
         * The assignment operator
         */
        MP3_FF_FILE& operator=(const MP3_FF_FILE& a)
        {
            if (&a != this)
            {
                _Ptr = a._Ptr;
                _Size = a._Size;
                auditCB = a.auditCB;
                _pvfile.Copy(a._pvfile);
                _streamID = a._streamID;
            }
            return *this;
        }

        MP3_FF_FILE_REFERENCE _Ptr;
        int32                 _Size;
        OsclAuditCB*          auditCB;
        Oscl_FileServer       _fileServSession;
        PVFile                _pvfile;
        uint8                 _streamID;
        bool                  _fileServSessionConnected;
};


class MP3Utils
{

    public:

        static int32  getCurrentFilePosition(PVFile *fp);
        static int32  OpenFile(OSCL_wHeapString<OsclMemAllocator> filename,
                               uint32 mode,
                               MP3_FF_FILE *fp);
        static int32  CloseFile(PVFile *fp);
        static int32  Flush(PVFile *fp);
        static bool   getCurrentFileSize(PVFile *fp, uint32& aCurrentSize);
        static MP3ErrorType SeektoOffset(PVFile *fp, int32 offset, Oscl_File::seek_type aSeekType);
};

#endif // MP3_UTILS_H_INCLUDED
