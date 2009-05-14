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
#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#define PVLOGGER_STDERR_APPENDER_H_INCLUDED

#ifndef PVLOGGERACCESSORIES_H_INCLUDED
#include "pvlogger_accessories.h"
#endif

#ifndef OSCLCONFIG_UTIL_H_INCLUDED
#include "osclconfig_util.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

/**
 * Class: StdErrAppender
 *
 */
template < class Layout, int32 LayoutBufferSize, class Lock = OsclNullLock >
class StdErrAppender : public PVLoggerAppender
{
    public:
        typedef PVLoggerAppender::message_id_type message_id_type;

        StdErrAppender()
        {
            stringbuf = NULL;
            wstringbuf = NULL;
        }
        virtual ~StdErrAppender()
        {
            if (stringbuf)
                OSCL_DEFAULT_FREE(stringbuf);
            if (wstringbuf)
                OSCL_DEFAULT_FREE((OsclAny*)wstringbuf);
        }

        void AppendString(message_id_type msgID, const char *fmt, va_list va)
        {
            _lock.Lock();

            int32 size;

            if (!stringbuf)
            {
                stringbuf = (char*)OSCL_DEFAULT_MALLOC(LayoutBufferSize);
                if (!stringbuf)
                    return;//allocation failed-- just exit gracefully.
            }

            size = _layout.FormatString(stringbuf, LayoutBufferSize, msgID, fmt, va);

            {
                fprintf(stderr, "%s", stringbuf);
                fprintf(stderr, "\n");
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

    private:
        void AppendStringA(message_id_type msgID, const char *fmt, ...)
        {
            va_list arguments;
            va_start(arguments, fmt);
            AppendString(msgID, fmt, arguments);
            va_end(arguments);
        }

        Layout _layout;
        Lock _lock;
        char* stringbuf;
        oscl_wchar* wstringbuf;

};

#endif // PVLOGGER_STDERR_APPENDER_H_INCLUDED

