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
#ifndef PVLOGGER_ACCESSORIES_H_INCLUDED
#define PVLOGGER_ACCESSORIES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

/**
 * Base class for all message formatters. This class defines the interface to
 * the message formatter. There are two kinds of msg formatting APIs, one to
 * format text messages, and other to format opaque message buffers.
 */
class PVLoggerLayout
{
    public:
        typedef PVLogger::message_id_type message_id_type;

        virtual ~PVLoggerLayout() {}

        /**
         * Formats the string and copies it to the given buffer.
         *
         * @return The length of the string not including the trailing '\0'
         */
        virtual int32 FormatString(char* formatBuf, int32 formatBufSize,
                                   message_id_type msgID, const char * fmt,
                                   va_list va) = 0;

        /**
         * Formats the data and copies it to the given buffer.
         *
         * @return The length of the buffer used.
         */
        virtual int32 FormatOpaqueMessage(char* formatBuf, int32 formatBufSize,
                                          message_id_type msgID, int32 numPairs,
                                          va_list va) = 0;
};

/**
 * Base class for all message filters. This class defines the interface to
 * the message filters. There are two kinds of msg filtering APIs, one to
 * filter text messages, and other to filter opaque message buffers.
 */
class PVLoggerFilter
{
    public:
        virtual ~PVLoggerFilter() {}

        typedef PVLogger::message_id_type message_id_type;
        typedef PVLogger::log_level_type log_level_type;
        typedef PVLogger::filter_status_type filter_status_type;

        virtual filter_status_type FilterString(char* tag, message_id_type msgID, log_level_type level) = 0;
        virtual filter_status_type FilterOpaqueMessge(char* tag, message_id_type msgID, log_level_type level) = 0;
};

const PVLoggerFilter::filter_status_type PVLOGGER_FILTER_ACCEPT = 1;
const PVLoggerFilter::filter_status_type PVLOGGER_FILTER_REJECT = 2;
const PVLoggerFilter::filter_status_type PVLOGGER_FILTER_NEUTRAL = 3;

/**
 * Example filter that allows all messages to be logged.
 */
class AllPassFilter : public PVLoggerFilter
{
    public:
        typedef PVLoggerFilter::message_id_type message_id_type;
        typedef PVLoggerFilter::log_level_type log_level_type;
        typedef PVLoggerFilter::filter_status_type filter_status_type;

        AllPassFilter() {};
        virtual ~AllPassFilter() {};

        filter_status_type FilterString(char* tag, message_id_type msgID, log_level_type level)
        {
            OSCL_UNUSED_ARG(tag);
            OSCL_UNUSED_ARG(msgID);
            OSCL_UNUSED_ARG(level);
            return (PVLOGGER_FILTER_ACCEPT);
        };
        filter_status_type FilterOpaqueMessge(char* tag, message_id_type msgID, log_level_type level)
        {
            OSCL_UNUSED_ARG(tag);
            OSCL_UNUSED_ARG(msgID);
            OSCL_UNUSED_ARG(level);
            return (PVLOGGER_FILTER_ACCEPT);
        };
};

/**
 * Base class for all message appenders. This class defines the interface to
 * the message appenders. There are two kinds of msg appender APIs, one to
 * append text messages, and other to append opaque message buffers.
 */
class PVLoggerAppender
{
    public:
        typedef PVLogger::message_id_type message_id_type;

        virtual ~PVLoggerAppender() {}

        virtual void AppendString(message_id_type msgID, const char *fmt, va_list va) = 0;
        virtual void AppendBuffers(message_id_type msgID, int32 numPairs, va_list va) = 0;
};


#endif

