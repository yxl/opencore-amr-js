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

/*! \file pvlogger.h
    \brief This file contains basic logger interfaces for common use across platforms.

    This is the main entry point header file for the logger library.  It should be
    the only one users directly include.
*/

#ifndef PVLOGGER_H_INCLUDED
#define PVLOGGER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_SHARED_PTR_H
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_BASE_ALLOC_H_INCLUDED
#include "oscl_base_alloc.h"
#endif


const int32 PVLOGGER_LEVEL_UNINTIALIZED = -1;



////////////////////////////////////////////
// The instrumentation layer allows groups
// of messages to be compiled in or out of
// the code based on a build-time parameter.
/////////////////////////////////////////////

/**
 * Release Layer
 *
 * The release layer should only be used for messages that
 * should remain in the final release. In certain cases all
 * messaging may be disabled depending on customer requirements.
 * However, when allowed the release layer should contain
 * information that will be useful diagnosing problems in a
 * released product (perhaps after entering a diagnostic mode),
 * but with absolutely minimal performance impact when disabled
 * at runtime.
 */
#define PVLOGMSG_INST_REL   0
/**
 * Profile Layer
 *
 * The profile layer is used for messages and information
 * related to messuring and reporting performance-related
 * information.
 */
#define PVLOGMSG_INST_PROF  1
/**
 * High Level Debug Layer
 *
 * This layer should contain messages that have very minimal
 * impact on performance, but are at lower level (i.e., provide
 * more information) than would be appropriate in a shipping
 * product. The messages are probably used to gather information
 * and validate proper functionality at a high level as might
 * be appropriate for IOT, stress testing, or QA testing.
 */
#define PVLOGMSG_INST_HLDBG 2
/**
 * Mid Level Debug Layer
 *
 * This layer should contain messages that are useful in the
 * middle stages of the development cycle where major components
 * are being integrated. The components themselves should
 * already be well-tested so the emphasis is on interfaces
 * between these components and integration testing. Messages
 * at this layer may have some performance impact.
 */
#define PVLOGMSG_INST_MLDBG 3
/**
 * Low Level Debug Layer
 *
 * This layer should contain messages for early functional
 * testing. The messages are typically at a very low-level
 * and allow testing the functionality of individual modules
 * and components. Messages at this layer will typically have
 * a performance impact (sometimes significant) due to the
 * fact that they are at such a low level.
 */
#define PVLOGMSG_INST_LLDBG 4

/*
** Default logger instrumentation level.  To override this
** setting, define PVLOGGER_INST_LEVEL in the osclconfig.h file.
** Possible values and the resulting intrumentation:
**
**  PVLOGGER_INST_LEVEL 0 : No logging.  All logging statements compiled out.
**  PVLOGGER_INST_LEVEL 1 : Release level only.
**  PVLOGGER_INST_LEVEL 2 : Release level + profile level
**  PVLOGGER_INST_LEVEL 3 : Release level + profile level + high-level debug
**  PVLOGGER_INST_LEVEL 4 : Release level + profile level + high-level debug + mid-level debug
**  PVLOGGER_INST_LEVEL 5 & above : Release level + profile level + high-level debug
**                            + mid-level debug + low-level debug
*/
#ifndef PVLOGGER_INST_LEVEL
#if defined(NDEBUG)
/* Release mode-- No logging */
#define PVLOGGER_INST_LEVEL 0
#else
/* Debug mode-- Complete logging */
#define PVLOGGER_INST_LEVEL 5
#endif
#endif


#if (PVLOGGER_INST_LEVEL<1)

/*
** Disable and compile-out all logging
*/
#define PVLOGGER_LOGMSG(IL, LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V(IL, LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN(IL, LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V(IL, LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOG_USE_ONLY(x)

#else //PVLOGGER_INST_LEVEL

/*
** Internal use macros that make the logging calls to PVLogger.
*/
#define _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)\
{\
    if (LOGGER)\
    {\
        if (LOGGER->IsActive(LEVEL))\
        {\
            LOGGER->LogMsgString MESSAGE;\
        }\
    }\
}

#define _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)\
{\
    if (LOGGER)\
    {\
        if (LOGGER->IsActive(LEVEL))\
        {\
            LOGGER->LogMsgStringV MESSAGE;\
        }\
    }\
}

#define _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE) \
{\
    if (LOGGER)\
    {\
    	if (LOGGER->IsActive(LEVEL))\
    	{\
    		LOGGER->LogMsgBuffers MESSAGE;\
    	}\
    }\
}

#define _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE) \
{\
    if (LOGGER)\
    {\
    	if (LOGGER->IsActive(LEVEL))\
    	{\
    		LOGGER->LogMsgBuffersV MESSAGE;\
    	}\
    }\
}

/*
** In case some compilers cannot support the instrumentation-level macros,
** they can be disabled by defining PVLOGGER_INST_LEVEL_SUPPORT to 0
** in their osclconfig.h.  If instrumentation level is not supported, then
** all instrumentation levels will be compiled in.
**
** If PVLOGGER_INST_LEVEL_SUPPORT is not defined, the default is set here to allow
** compile-time instrumentation level support.
*/
#ifndef PVLOGGER_INST_LEVEL_SUPPORT
#define PVLOGGER_INST_LEVEL_SUPPORT 1
#endif

#if !(PVLOGGER_INST_LEVEL_SUPPORT)

/*
** A set of logging macros that ignore the instrumentation level.
** All instrumentation levels will be compiled in.
*/
#define PVLOGGER_LOGMSG(IL, LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V(IL, LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN(IL, LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V(IL, LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)

#else //PVLOGGER_INST_LEVEL_SUPPORT

/*
** This set of macros compiles the logging statements in or out based on the instrumtation
** level.
*/

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_REL)
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)
#else
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_REL(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#endif

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_PROF)
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)
#else
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_PROF(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#endif

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_HLDBG)
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)
#else
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_HLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#endif

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_MLDBG)
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V_PVLOGMSG_V_INST_MLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)
#else
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_MLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V_PVLOGMSG_V_INST_MLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#endif

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGMSG_V(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN(LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) _PVLOGGER_LOGBIN_V(LOGGER, LEVEL, MESSAGE)
#else
#define PVLOGGER_LOGMSG_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGMSG_V_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#define PVLOGGER_LOGBIN_V_PVLOGMSG_INST_LLDBG(LOGGER, LEVEL, MESSAGE) OSCL_UNUSED_ARG(LOGGER);
#endif


/**
 * This is the text based API to log messages
 *
 * @param IL		Instrumentation level.
 * @param LOGGER	Pointer to the logger object, that acts as the logging
 *					control/interface point
 * @param LEVEL		Log level of the message
 * @param MESSAGE	Log Message which includes the message id, and any kind
 *					of formatting information
 *
 * Example Usage:
 * PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger_1, PVLOGMSG_WARNING, (13, "Test Messsage to Node 1\n"));
 *	-This message of log level PVLOGMSG_WARNING, and has a message id of 13
 */
#define PVLOGGER_LOGMSG(IL, LOGGER, LEVEL, MESSAGE) PVLOGGER_LOGMSG_ ## IL (LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGMSG_V(IL, LOGGER, LEVEL, MESSAGE) PVLOGGER_LOGMSG_V_ ## IL (LOGGER, LEVEL, MESSAGE)

/**
 * This is a binary API to log messages
 *
 * @param IL		Instrumentation level.
 * @param LOGGER	Pointer to the logger object, that acts as the logging
 *					control/interface point
 * @param LEVEL		Log level of the message
 * @param MESSAGE	Log Message which includes the message id, and message
 *                  buffers that need to be logged.
 *
 * Example Usage:
 * PVLOGGER_LOGBIN (PVLOGMSG_INST_LLDBG, logger_1, PVLOGMSG_WARNING, (10, 3, msgBuf1Size, msgBuf1,
 *                                                            msgBuf2Size, msgBuf2,
 *											                  msgBuf3Size, msgBuf3));
 *
 *	-This message contains THREE (ptr_len, ptr) pairs.
 *   Log level of this msg is PVLOGMSG_WARNING,
 *   message id is 10.
 */
#define PVLOGGER_LOGBIN(IL, LOGGER, LEVEL, MESSAGE) PVLOGGER_LOGBIN_ ## IL (LOGGER, LEVEL, MESSAGE)
#define PVLOGGER_LOGBIN_V(IL, LOGGER, LEVEL, MESSAGE) PVLOGGER_LOGBIN_V_ ## IL (LOGGER, LEVEL, MESSAGE)

#endif //PVLOGGER_INST_LEVEL_SUPPORT

/**
 * Used to compile in/out lines of code that are used only
 * for PVLogger macros.
 *
 * This code will be removed at compile time when PVLogger
 * is disabled, i.e. Release mode.  So do not put in any
 * code that is necessary for correct functionality of the module
 */
#define PVLOGGER_LOG_USE_ONLY(x) x

#endif // PVLOGGER_INST_LEVEL

/**
 * In case logging is compiled out, there is no need to compile
 * the logger runtime code either.
 */
#ifndef PVLOGGER_ENABLE
#if (PVLOGGER_INST_LEVEL<1)
#define PVLOGGER_ENABLE 0
#else
#define PVLOGGER_ENABLE 1
#endif
#endif

/**
 * Class: PVLogger
 *
 * Each logger instance is associated with a unique tag. PVLoggerRegistry
 * class, maintains a repository of all the loggers, along with their
 * associated tags. Each logger has an associated list of appenders. Appenders
 * are entities that act as a sink for all the incoming messages. They could be
 * file appenders, serial port appenders, buffer appenders etc. Each logger
 * also has a list of message filters. These filters remove any unwanted messages
 * from the output.
 */
class PVLoggerAppender;
class PVLoggerFilter;
class PVLogger
{
    public:

        typedef int32 log_level_type;
        typedef int32 message_id_type;
        typedef int32 filter_status_type;
        typedef _OsclBasicAllocator alloc_type;

        /**
         * PVLogger needs to be initialized once per thread.  This
         * creates the PVLogger singleton that is used throughout
         * the duration of the thread.  Initialization must occur
         * before the first message is logged.
         *
         * @exception leaves if out of memory
         */
        OSCL_IMPORT_REF static void Init();


        /**
         * Frees the PVLogger singleton used by the current thread.
         * This must be called before thread exit.  No messages
         * can be logged after cleanup.
         *
         * @return
         */
        OSCL_IMPORT_REF static void Cleanup();

        /**
         * This is a factory method to create a log control point, with a
         * certain input tag. There is a central registry of all the loggers,
         * with their corresponding tags, called PV Logger Registry. In case
         * the logger with the specified tag exists in the global registry, it
         * is returned, else a new one is created and a pointer to the same is
         * returend.
         *
         * @param inputTag   logger tag, viz. "x.y.z"
         * @param level      log level associated with the logging control point
         *                   (All messages with log levels less than equal to the
         *                    log level of the control point would be logged)
         *
         * @param oAppenderInheritance
         *
         * @return PVLogger* Pointer to the logging control point
         *
         * @exception leaves if out of memory
         */

        OSCL_IMPORT_REF static PVLogger *GetLoggerObject(const char* inputTag);

        /**
         * This method is used to set the log level of a control point.
         *
         * @param level      log level associated with the logging control point
         *
         * @return           NONE
         */
        void SetLogLevel(log_level_type level)
        {
#if(PVLOGGER_ENABLE)
            _level = level;
#else
            OSCL_UNUSED_ARG(level);
#endif
        }

        /**
         * This method is used to set the log level of a control point, as well as
         * to propagate the level to all the descendants of this control point.
         *
         * @param level      log level associated with the logging control point
         *
         * @return           NONE
         */
        OSCL_IMPORT_REF void SetLogLevelAndPropagate(log_level_type level);

        /**
         * This method returns the log level of a control point. This could either
         * have been set explicitly by the user (at the time of creation or later)
         * or could have been inherited from one of its ancestors.
         *
         * @return           log level associated with the logging control point
         */
        log_level_type GetLogLevel()
        {
#if(PVLOGGER_ENABLE)
            return(_level);
#else
            return 0;
#endif
        }

        /**
         * This method disables appender inheritance for the logging control point
         *
         */
        void DisableAppenderInheritance()
        {
#if(PVLOGGER_ENABLE)
            _oAppenderInheritance = false;
#endif
        }

        /**
         * This method adds an appender to the logging control point. Each logger
         * maintains a list of appenders. Any msg to a logger if deemed active is
         * logged to all the appenders.
         *
         * @param appender   pointer to the appender to add
         *
         * @return           NONE
         *
         * @exception        leaves if out of memory
         */
        void AddAppender(OsclSharedPtr<PVLoggerAppender> &appender)
        {
#if(PVLOGGER_ENABLE)
            _pOwnAppenderVec.push_back(appender);
#else
            OSCL_UNUSED_ARG(appender);
#endif
        }

        /**
         * This method removes an appender from the logging control point. Each logger
         * maintains a list of appenders. Any msg to a logger if deemed active is
         * logged to all the appenders.
         *
         * @param appender   pointer to the appender to delete
         *
         * @return           NONE
         */
        void RemoveAppender(OsclSharedPtr<PVLoggerAppender> &appender)
        {
#if(PVLOGGER_ENABLE)
            for (Oscl_Vector<OsclSharedPtr<PVLoggerAppender>, alloc_type>::iterator it = _pOwnAppenderVec.begin();
                    it != _pOwnAppenderVec.end();
                    it++)
            {
                if ((*it).GetRep() == appender.GetRep())
                {
                    _pOwnAppenderVec.erase(it);
                    break;
                }
            }
#else
            OSCL_UNUSED_ARG(appender);
#endif
        }

        /**
         * This method adds a message filter to the logging control point. Each logger
         * maintains a list of filters. Any msg to a logger if deemed active is
         * passed through the msg filters prior to logging.
         *
         * @param msgFilter   pointer to the filter to add
         *
         * @return           NONE
         *
         * @exception        leaves if out of memory
         */
        void AddFilter(OsclSharedPtr<PVLoggerFilter> &filter)
        {
#if(PVLOGGER_ENABLE)
            _pMsgFilterVec.push_back(filter);
#else
            OSCL_UNUSED_ARG(filter);
#endif
        };

        /**
         * This method returns the number of appenders attached to the logging control point.
         */
        uint32 GetNumAppenders()
        {
#if(PVLOGGER_ENABLE)
            return(_pOwnAppenderVec.size());
#else
            return 0;
#endif
        }

        /**
         * This method determines if a msg passed to the logging control point is active
         * or not. Only messages that are deemed active are logged. Messages are considered
         * not active if any of the following criteria are met:
         *		- All logging is disabled at this logging control point
         *      - If all the log levels, leading upto the root log point are uninitialized
         *      - If the log level of the incoming message is LESS THAN that of the active
         *        log level of the logging control point.
         *
         * @return           BOOL
         */
        OSCL_IMPORT_REF bool IsActive(log_level_type level);

        /**
         * This method logs formatted text msg to all the appenders, after running thrrough
         * the message filters. After logging the message to the appenders attached to the
         * current control point, the message is passed up to the parent node, only if
         * appender inheritance is enabled.
         *
         *
         * @param msgID      Message ID, that is unique to a message
         * @param fmt        format string, similar to one taken by printf
         * @param arguments  Variable list of arguments
         *
         * @return           NONE
         */
        OSCL_IMPORT_REF void LogMsgStringV(message_id_type msgID, const char * fmt, va_list arguments);

        /**
         * This method logs opaque data buffers to all the appenders, after running thrrough
         * the message filters. After logging the message to the appenders attached to the
         * current control point, the message is passed up to the parent node, only if
         * appender inheritance is enabled.
         *
         *
         * @param msgID      Message ID, that is unique to a message
         * @param numPairs   Number of (ptr_len, ptr) pairs
         * @param arguments  Variable list of arguments
         *
         * @return           NONE
         */
        OSCL_IMPORT_REF void LogMsgBuffersV(message_id_type msgID, int32 numPairs, va_list arguments);

        /**
         * This method logs formatted text msg to all the appenders, after running thrrough
         * the message filters. After logging the message to the appenders attached to the
         * current control point, the message is passed up to the parent node, only if
         * appender inheritance is enabled.
         *
         *
         * @param msgID      Message ID, that is unique to a message
         * @param fmt        format string, similar to one taken by printf
         * @param arguments  Variable list of arguments
         *
         * @return           NONE
         */
        OSCL_IMPORT_REF void LogMsgString(message_id_type msgID, const char * fmt, ...);

        /**
         * This method logs opaque data buffers to all the appenders, after running thrrough
         * the message filters. After logging the message to the appenders attached to the
         * current control point, the message is passed up to the parent node, only if
         * appender inheritance is enabled.
         *
         *
         * @param msgID      Message ID, that is unique to a message
         * @param numPairs   Number of (ptr_len, ptr) pairs
         * @param arguments  Variable list of arguments
         *
         * @return           NONE
         */
        OSCL_IMPORT_REF void LogMsgBuffers(message_id_type msgID, int32 numPairs, ...);

        /**
         * Logger Constructor
         *
         *
         * @param tag                     Logger tag, unique to a logging control point
         * @param level                   Active Log level of the logger
         * @param oAppenderInheritance
         *
         * @return                        NONE
         */
        OSCL_IMPORT_REF PVLogger(const char* inputTag, log_level_type level, bool oAppenderInheritance);

        virtual ~PVLogger()
        {
#if(PVLOGGER_ENABLE)
            _tagAllocator.deallocate(_tag);
#endif
        }

#if(PVLOGGER_ENABLE)
    protected:
        friend class PVLoggerRegistry;
        void SetParent(PVLogger *parentLogger)
        {
            _parentLogger = parentLogger;
        }
        PVLogger *GetParent()
        {
            return(_parentLogger);
        }

    private:
        filter_status_type FilterMsg(message_id_type msgID);
        void LogMsg(message_id_type msgID, const char *fmt, va_list arguments);
        void LogMsg(message_id_type msgID, int32 numPairs, va_list arguments);

        char* _tag;
        log_level_type _level;
        log_level_type _lastMsgLevel;
        bool _oAppenderInheritance;

        PVLogger *_parentLogger;
        Oscl_TAlloc<char, alloc_type> _tagAllocator;

        Oscl_Vector<OsclSharedPtr<PVLoggerFilter>, alloc_type> _pMsgFilterVec;
        Oscl_Vector<OsclSharedPtr<PVLoggerAppender>, alloc_type>  _pOwnAppenderVec;
#endif //PVLOGGER_ENABLE
};

//////////////////////////////////////
// log message levels
//////////////////////////////////////

/**
 * system is unusable
 */
const PVLogger::log_level_type PVLOGMSG_EMERG   = 0;
/**
 * action must be taken immediately
 */
const PVLogger::log_level_type PVLOGMSG_ALERT   = 1;
/**
 * critical conditions
 */
const PVLogger::log_level_type PVLOGMSG_CRIT    = 2;
/**
 * error conditions
 */
const PVLogger::log_level_type PVLOGMSG_ERR     = 3;
/**
 * warning conditions
 */
const PVLogger::log_level_type PVLOGMSG_WARNING = 4;
/**
 * normal but significant condition
 */
const PVLogger::log_level_type PVLOGMSG_NOTICE  = 5;
/**
 * informational
 */
const PVLogger::log_level_type PVLOGMSG_INFO    = 6;
/**
 * function enter and exit
 */
const PVLogger::log_level_type PVLOGMSG_STACK_TRACE = 7;
/**
 * debug-level messages
 */
const PVLogger::log_level_type PVLOGMSG_DEBUG   = 8;

///////////////////////////////////
// do not use these levels
// for backward compatibility only
///////////////////////////////////
const PVLogger::log_level_type PVLOGMSG_FATAL_ERROR    = PVLOGMSG_EMERG;
const PVLogger::log_level_type PVLOGMSG_NONFATAL_ERROR = PVLOGMSG_ERR;
const PVLogger::log_level_type PVLOGMSG_STATISTIC      = PVLOGMSG_INFO;
const PVLogger::log_level_type PVLOGMSG_VERBOSE        = PVLOGMSG_DEBUG;

#endif // PVLOGGER_H_INCLUDED
