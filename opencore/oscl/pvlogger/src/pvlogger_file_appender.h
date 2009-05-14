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
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#define PVLOGGER_FILE_APPENDER_H_INCLUDED

#ifndef OSCL_MEM_AUTO_PTR_H_INCLUDED
#include "oscl_mem_auto_ptr.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVLOGGERACCESSORIES_H_INCLUDED
#include "pvlogger_accessories.h"
#endif

template < class Layout, int32 LayoutBufferSize, class Lock = OsclNullLock >
class TextFileAppender : public PVLoggerAppender
{
    public:
        typedef PVLoggerAppender::message_id_type message_id_type;

        static TextFileAppender<Layout, LayoutBufferSize, Lock>* CreateAppender(const OSCL_TCHAR * filename, uint32 cacheSize = 0)
        {

            TextFileAppender<Layout, LayoutBufferSize, Lock> * appender = new TextFileAppender<Layout, LayoutBufferSize, Lock>();
            if (NULL == appender) return NULL;

#ifdef T_ARM
// Seems like ADS 1.2 compiler crashes if template argument is used as part of another template argument so explicitly declare it.
            OSCLMemAutoPtr<TextFileAppender<Layout, LayoutBufferSize, Lock>, Oscl_TAlloc<TextFileAppender<Layout, LayoutBufferSize, Lock>, OsclMemAllocator> > holdAppender(appender);
#else
            OSCLMemAutoPtr<TextFileAppender<Layout, LayoutBufferSize, Lock> > holdAppender(appender);
#endif

            if (0 != appender->_fs.Connect()) return NULL;

            //set log file object options
            //this has its own cache so there's no reason to use pv cache.
            appender->_logFile.SetPVCacheSize(0);
            //make sure there's no logging on this file or we get infinite loop!
            appender->_logFile.SetLoggingEnable(false);
            appender->_logFile.SetSummaryStatsLoggingEnable(false);
            //end of log file object options.

            if (0 != appender->_logFile.Open(filename,
                                             Oscl_File::MODE_READWRITE | Oscl_File::MODE_TEXT,
                                             appender->_fs))
            {
                return NULL;
            }

            if (cacheSize)
            {
                appender->_cache.ptr = OSCL_DEFAULT_MALLOC(cacheSize);
                appender->_cache.len = 0;
            }
            appender->_cacheSize = cacheSize;

            return holdAppender.release();
        }

        virtual ~TextFileAppender()
        {
            if (_cache.ptr)
            {
                _logFile.Write(_cache.ptr, sizeof(char), _cache.len);
                OSCL_DEFAULT_FREE(_cache.ptr);
            }
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

            if (_cache.ptr)
            {
                if (_cache.len + size + 2 < _cacheSize)
                {
                    oscl_memcpy(OSCL_STATIC_CAST(uint8*, _cache.ptr) + _cache.len, stringbuf, size);
                    _cache.len += size;
                    oscl_memcpy(OSCL_STATIC_CAST(uint8*, _cache.ptr) + _cache.len, newline, 2);
                    _cache.len += 2;
                }
                else
                {
                    _logFile.Write(_cache.ptr, sizeof(char), _cache.len);
                    _logFile.Write(stringbuf, sizeof(char), size);
                    _logFile.Write(newline, sizeof(char), 2);
                    _logFile.Flush();
                    _cache.len = 0;
                }
            }
            else
            {
                _logFile.Write(stringbuf, sizeof(char), size);
                _logFile.Write(newline, sizeof(char), 2);
                _logFile.Flush();
            }

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

        TextFileAppender()
        {
            _cache.len = 0;
            _cache.ptr = 0;
            stringbuf = NULL;
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

    private:
        Lock _lock;
        char* stringbuf;


};

class BinaryFileAppender : public PVLoggerAppender
{
    public:
        typedef PVLoggerAppender::message_id_type message_id_type;

        static BinaryFileAppender* CreateAppender(const char* filename, uint32 cacheSize = 0)
        {

            BinaryFileAppender * appender = OSCL_NEW(BinaryFileAppender, ());
            if (NULL == appender) return NULL;

#ifdef T_ARM
            //ADS 1.2 can't compile the auto ptr-- so don't use it.
#else
            OSCLMemAutoPtr<BinaryFileAppender> holdAppender(appender);
#endif

            if (0 != appender->_fs.Connect()) return NULL;

            //set log file object options
            //this has its own cache so there's no reason to use pv cache.
            appender->_logFile.SetPVCacheSize(0);
            //make sure there's no logging on this file or we get infinite loop!
            appender->_logFile.SetLoggingEnable(false);
            appender->_logFile.SetSummaryStatsLoggingEnable(false);
            //end of log file object options.

            if (0 != appender->_logFile.Open(filename,
                                             Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY,
                                             appender->_fs))
            {
                return NULL;
            }

            if (cacheSize)
            {
                appender->_cache.ptr = OSCL_MALLOC(cacheSize);
                appender->_cache.len = 0;
            }
            appender->_cacheSize = cacheSize;

#ifdef T_ARM
            return appender;
#else
            return holdAppender.release();
#endif
        }

        static BinaryFileAppender* CreateAppender(const OSCL_TCHAR * filename, uint32 cacheSize = 0)
        {

            BinaryFileAppender * appender = OSCL_NEW(BinaryFileAppender, ());
            if (NULL == appender) return NULL;

#ifdef T_ARM
            //ADS 1.2 can't compile the auto ptr-- so don't use it.
#else
            OSCLMemAutoPtr<BinaryFileAppender> holdAppender(appender);
#endif

            if (0 != appender->_fs.Connect()) return NULL;

            //set log file object options
            //this has its own cache so there's no reason to use pv cache.
            appender->_logFile.SetPVCacheSize(0);
            //make sure there's no logging on this file or we get infinite loop!
            appender->_logFile.SetLoggingEnable(false);
            appender->_logFile.SetSummaryStatsLoggingEnable(false);
            //end of log file object options.

            if (0 != appender->_logFile.Open(filename,
                                             Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY,
                                             appender->_fs))
            {
                return NULL;
            }

            if (cacheSize)
            {
                appender->_cache.ptr = OSCL_MALLOC(cacheSize);
                appender->_cache.len = 0;
            }
            appender->_cacheSize = cacheSize;

#ifdef T_ARM
            return appender;
#else
            return holdAppender.release();
#endif
        }

        virtual ~BinaryFileAppender()
        {
            if (_cache.ptr)
            {
                _logFile.Write(_cache.ptr, sizeof(char), _cache.len);
                OSCL_FREE(_cache.ptr);
            }
            _logFile.Close();
            _fs.Close();
        }

        void AppendString(message_id_type msgID, const char *fmt, va_list va)
        {
            OSCL_UNUSED_ARG(msgID);
            OSCL_UNUSED_ARG(fmt);
            OSCL_UNUSED_ARG(va);
        }

        void AppendBuffers(message_id_type msgID, int32 numPairs, va_list va)
        {
            OSCL_UNUSED_ARG(msgID);

            for (int32 i = 0; i < numPairs; i++)
            {
                int32 length = va_arg(va, int32);
                void* buffer = va_arg(va, void*);

                if (_cache.ptr)
                {
                    if (_cache.len + length < _cacheSize)
                    {
                        oscl_memcpy(OSCL_STATIC_CAST(uint8*, _cache.ptr) + _cache.len, buffer, length);
                        _cache.len += length;
                    }
                    else
                    {
                        _logFile.Write(_cache.ptr, sizeof(uint8), _cache.len);
                        _logFile.Write(buffer, sizeof(uint8), length);
                        _logFile.Flush();
                        _cache.len = 0;
                    }
                }
                else
                {
                    _logFile.Write(buffer, sizeof(uint8), length);
                    _logFile.Flush();
                }
            }
            va_end(va);
        }

        void AppendBuffers(message_id_type msgID, int32 numPairs, ...)
        {
            va_list args;
            va_start(args, numPairs);
            AppendBuffers(msgID, numPairs, args);
        }

    private:
        BinaryFileAppender()
        {
            _cache.len = 0;
            _cache.ptr = 0;
        }

        Oscl_FileServer _fs;
        Oscl_File _logFile;
        OsclMemoryFragment _cache;
        uint32 _cacheSize;
};




#endif // PVLOGGER_FILE_APPENDER_H_INCLUDED


