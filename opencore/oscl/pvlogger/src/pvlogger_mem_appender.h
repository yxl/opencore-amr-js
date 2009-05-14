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
#ifndef PVLOGGER_MEM_APPENDER_H_INCLUDED
#define PVLOGGER_MEM_APPENDER_H_INCLUDED

#ifndef OSCL_MEM_AUTO_PTR_H_INCLUDED
#include "oscl_mem_auto_ptr.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVLOGGERACCESSORIES_H_INCLUDED
#include "pvlogger_accessories.h"
#endif

#define DEFAULT_MEM_SIZE 0x500000 // 2MB

template < class Layout, int32 LayoutBufferSize, class Lock = OsclNullLock >
class MemAppender : public PVLoggerAppender
{
    public:
        typedef PVLoggerAppender::message_id_type message_id_type;

        static MemAppender<Layout, LayoutBufferSize, Lock>* CreateAppender(OSCL_TCHAR * filename, uint32 cacheSize = 0)
        {

            MemAppender<Layout, LayoutBufferSize, Lock> * appender = new MemAppender<Layout, LayoutBufferSize, Lock>();
            if (NULL == appender) return NULL;

#ifdef T_ARM
// Seems like ADS 1.2 compiler crashes if template argument is used as part of another template argument so explicitly declare it.
            OSCLMemAutoPtr<MemAppender<Layout, LayoutBufferSize, Lock>, Oscl_TAlloc<MemAppender<Layout, LayoutBufferSize, Lock>, OsclMemAllocator> > holdAppender(appender);
#else
            OSCLMemAutoPtr<MemAppender<Layout, LayoutBufferSize, Lock> > holdAppender(appender);
#endif

            if (0 != appender->_fs.Connect()) return NULL;

            if (0 != appender->_logFile.Open(filename,
                                             Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT,
                                             appender->_fs))
            {
                return NULL;
            }

            if (!cacheSize)
            {
                cacheSize = DEFAULT_MEM_SIZE;
            }

            appender->_base.ptr = OSCL_DEFAULT_MALLOC(cacheSize);
            appender->_cache.len = 0;
            appender->_cacheSize = cacheSize;
            appender->_cache.ptr = OSCL_STATIC_CAST(uint8*, appender->_base.ptr);
            appender->_memOverFlow = 0;

            return holdAppender.release();
        }

        virtual ~MemAppender()
        {

            uint32 length;
            length = OSCL_STATIC_CAST(uint32, (OSCL_STATIC_CAST(uint8*, _cache.ptr) - OSCL_STATIC_CAST(uint8*, _base.ptr)));

            if (_memOverFlow)
            {
                _logFile.Write(_cache.ptr, sizeof(char), (_cacheSize - length));
            }
            _logFile.Write(_base.ptr, sizeof(char), length);

            OSCL_DEFAULT_FREE(_base.ptr);

            _logFile.Close();
            _fs.Close();
            if (stringbuf)
                OSCL_DEFAULT_FREE((OsclAny*)stringbuf);
        }

        void AppendString(message_id_type msgID, const char *fmt, va_list va)
        {
            _lock.Lock();

            if (!stringbuf)
                stringbuf = (char*)OSCL_DEFAULT_MALLOC(LayoutBufferSize);
            if (!stringbuf)
                return;//out of memory!
            int32 size;
            char newline[2];
            newline[0] = 0x0D;
            newline[1] = 0x0A;

            size = _layout.FormatString(stringbuf, LayoutBufferSize, msgID, fmt, va);
            if (_cache.len + size + 2 >= _cacheSize)
            {
                _cache.ptr = _base.ptr;
                _cache.len = 0;
                _memOverFlow = 1;
            }
            uint32 temp = OSCL_STATIC_CAST(uint32, _cache.ptr);

            oscl_memcpy(OSCL_STATIC_CAST(uint8*, _cache.ptr), stringbuf, size);
            temp += size;

            oscl_memcpy(OSCL_STATIC_CAST(uint8*, _cache.ptr) + size, newline, 2);
            temp += 2;

            _cache.ptr = OSCL_STATIC_CAST(uint8*, temp);
            _cache.len += (size + 2);

            _lock.Unlock();
        }

        void AppendBuffers(message_id_type msgID, int32 numPairs, va_list va)
        {
            OSCL_UNUSED_ARG(msgID);

            for (int32 i = 0; i < numPairs; i++)
            {
                int32 length = va_arg(va, int32);
                uint8* buffer = va_arg(va, uint8*);

                int32 jj;
                for (jj = 10; jj < length; jj += 10)
                {
                    AppendStringA(0, "  %x %x %x %x %x %x %x %x %x %x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
                    buffer += 10;
                }

                uint8 remainderbuf[10];
                uint32 remainder = length - (jj - 10);
                if (remainder > 0 && remainder <= 10)
                {
                    oscl_memcpy(remainderbuf, buffer, remainder);
                    oscl_memset(remainderbuf + remainder, 0, 10 - remainder);
                    buffer = remainderbuf;
                    AppendStringA(0, "  %x %x %x %x %x %x %x %x %x %x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
                }
            }
            va_end(va);
        }

        MemAppender()
        {
            _cache.len = 0;
            _cache.ptr = 0;
            stringbuf = NULL;
            _memOverFlow = 0;
        }

    private:
        void AppendStringA(message_id_type msgID, const char *fmt, ...)
        {
            va_list arguments;
            va_start(arguments, fmt);
            AppendString(msgID, fmt, arguments);
            va_end(arguments);
        }

        Layout _layout;

#ifdef T_ARM
//ADS 1.2 compiler doesn't interpret this correctly.
    public:
#else
    private:
#endif
        Oscl_FileServer _fs;
        Oscl_File _logFile;
        OsclMemoryFragment _cache;
        uint32 _cacheSize;
        bool _memOverFlow;
        OsclMemoryFragment _base;

    private:
        Lock _lock;
        char* stringbuf;


};

#endif // PVLOGGER_MEM_APPENDER_H_INCLUDED


