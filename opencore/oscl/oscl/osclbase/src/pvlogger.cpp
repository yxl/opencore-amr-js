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
#include "pvlogger.h"
#include "pvlogger_registry.h"
#include "pvlogger_accessories.h"
#include "oscl_dll.h"
#include "oscl_stdstring.h"


#ifndef OSCL_COMBINED_DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()
#endif

#if(PVLOGGER_ENABLE)
const char rootTag[] = "";

//use the TLS registry, or the singleton registry if no TLS.
//Note: singleton registry only works for single-threaded scenarios,
//since this implementation assumes a per-thread registry.
#include "oscl_base.h"
#include "oscl_tls.h"
#define PVLOGGER_REGISTRY OsclTLSRegistry
#define PVLOGGER_REGISTRY_ID OSCL_TLS_ID_PVLOGGER
#define PVLOGGER_REGISTRY_WRAPPER OsclTLS

#endif //PVLOGGER_ENABLE

OSCL_EXPORT_REF void PVLogger::Init()
{
#if(PVLOGGER_ENABLE)
    alloc_type alloc;
    OsclAny* ptr = alloc.allocate(sizeof(PVLoggerRegistry));
    if (ptr)
    {
        PVLoggerRegistry *pvlogreg = new(ptr) PVLoggerRegistry();
        int32 err;
        PVLOGGER_REGISTRY::registerInstance(pvlogreg, PVLOGGER_REGISTRY_ID, err);
    }
#endif
}

OSCL_EXPORT_REF void PVLogger::Cleanup()
{
#if(PVLOGGER_ENABLE)
    int32 err;
    PVLoggerRegistry *pvlogreg = OSCL_STATIC_CAST(PVLoggerRegistry*, PVLOGGER_REGISTRY::getInstance(PVLOGGER_REGISTRY_ID, err));
    if (pvlogreg)
    {
        pvlogreg->~PVLoggerRegistry();
        alloc_type alloc;
        alloc.deallocate(pvlogreg);
        PVLOGGER_REGISTRY::registerInstance(NULL, PVLOGGER_REGISTRY_ID, err);
    }
#endif
}

OSCL_EXPORT_REF PVLogger* PVLogger::GetLoggerObject(const char* inputTag)
{
#if(PVLOGGER_ENABLE)
    PVLoggerRegistry* registry = PVLoggerRegistry::GetPVLoggerRegistry();
    return (registry) ? registry->GetPVLoggerObject(inputTag) : NULL;
#else
    OSCL_UNUSED_ARG(inputTag);
    return NULL;
#endif
}

OSCL_EXPORT_REF void PVLogger::SetLogLevelAndPropagate(log_level_type level)
{
#if(PVLOGGER_ENABLE)
    _level = level;
    PVLoggerRegistry::GetPVLoggerRegistry()->SetNodeLogLevelExplicit(_tag, level);
#else
    OSCL_UNUSED_ARG(level);
#endif
}

OSCL_EXPORT_REF bool PVLogger::IsActive(log_level_type level)
{
#if(PVLOGGER_ENABLE)
    _lastMsgLevel = level;

    if (_level == PVLOGGER_LEVEL_UNINTIALIZED)
    {
        if (_parentLogger != NULL)
        {
            return(_parentLogger->IsActive(level));
        }
        else
        {
            /*
             * We are the root node, as every node other
             * than root MUST have a parent. If the root's
             * log level is uninitialized, then we do not
             * log anything
             */
            return false;
        }
    }
    if (level <= _level)
    {
        return true;
    }
#else
    OSCL_UNUSED_ARG(level);
#endif
    return false;
}

OSCL_EXPORT_REF void PVLogger::LogMsgStringV(message_id_type msgID, const char * fmt, va_list arguments)
{
#if(PVLOGGER_ENABLE)
    filter_status_type msgStatus = FilterMsg(msgID);

    if (msgStatus == PVLOGGER_FILTER_ACCEPT)
    {
        //Log msg to the current node
        LogMsg(msgID, fmt, arguments);
    }

    if ((_parentLogger != NULL) && (_oAppenderInheritance))
    {
        //Pass the msg to the parent
        _parentLogger->LogMsgStringV(msgID, fmt, arguments);
    }
#else
    OSCL_UNUSED_ARG(msgID);
    OSCL_UNUSED_ARG(fmt);
    OSCL_UNUSED_ARG(arguments);
#endif
    return;
}

OSCL_EXPORT_REF void PVLogger::LogMsgBuffersV(message_id_type msgID, int32 numPairs, va_list arguments)
{
#if(PVLOGGER_ENABLE)
    filter_status_type msgStatus = FilterMsg(msgID);

    if (msgStatus == PVLOGGER_FILTER_ACCEPT)
    {
        //Log msg to the current node
        LogMsg(msgID, numPairs, arguments);
    }

    if ((_parentLogger != NULL) && (_oAppenderInheritance))
    {
        //Pass the msg to the parent
        _parentLogger->LogMsgBuffersV(msgID, numPairs, arguments);

    }
#else
    OSCL_UNUSED_ARG(msgID);
    OSCL_UNUSED_ARG(numPairs);
    OSCL_UNUSED_ARG(arguments);
#endif
    return;
}

OSCL_EXPORT_REF void PVLogger::LogMsgString(message_id_type msgID, const char * fmt, ...)
{
#if(PVLOGGER_ENABLE)
    va_list arguments;
    va_start(arguments, fmt);

    filter_status_type msgStatus = FilterMsg(msgID);

    if (msgStatus == PVLOGGER_FILTER_ACCEPT)
    {
        LogMsg(msgID, fmt, arguments);
    }

    if ((_parentLogger != NULL) && (_oAppenderInheritance))
    {
        //Pass the msg to the parent
        _parentLogger->LogMsgStringV(msgID, fmt, arguments);
    }
#else
    OSCL_UNUSED_ARG(msgID);
    OSCL_UNUSED_ARG(fmt);
#endif
    return;
}

OSCL_EXPORT_REF void PVLogger::LogMsgBuffers(message_id_type msgID, int32 numPairs, ...)
{
#if(PVLOGGER_ENABLE)
    va_list arguments;
    va_start(arguments, numPairs);

    filter_status_type msgStatus = FilterMsg(msgID);

    if (msgStatus == PVLOGGER_FILTER_ACCEPT)
    {
        LogMsg(msgID, numPairs, arguments);
    }

    if ((_parentLogger != NULL) && (_oAppenderInheritance))
    {
        //Pass the msg to the parent
        _parentLogger->LogMsgBuffersV(msgID, numPairs, arguments);
    }
#else
    OSCL_UNUSED_ARG(msgID);
    OSCL_UNUSED_ARG(numPairs);
#endif
    return;
}

OSCL_EXPORT_REF PVLogger::PVLogger(const char* inputTag, log_level_type level, bool oAppenderInheritance)
{
#if(PVLOGGER_ENABLE)
    _tag = _tagAllocator.ALLOCATE(oscl_strlen(inputTag) + 1);

    oscl_strncpy(_tag, inputTag, (oscl_strlen(inputTag) + 1));

    _parentLogger = NULL;
    _oAppenderInheritance = oAppenderInheritance;
    _level = level;
    _lastMsgLevel = PVLOGGER_LEVEL_UNINTIALIZED;
#else
    OSCL_UNUSED_ARG(inputTag);
    OSCL_UNUSED_ARG(level);
    OSCL_UNUSED_ARG(oAppenderInheritance);
#endif
    return;
}

#if(PVLOGGER_ENABLE)
PVLogger::filter_status_type PVLogger::FilterMsg(message_id_type msgID)
{
    uint32 j;

    if (_pMsgFilterVec.size() > 0)
    {
        for (j = 0; j < _pMsgFilterVec.size(); j++)
        {
            PVLoggerFilter *msgFilter = _pMsgFilterVec[j];

            filter_status_type msgStatus = msgFilter->FilterString(_tag, msgID, _level);

            if (msgStatus != PVLOGGER_FILTER_NEUTRAL)
            {
                return msgStatus;
            }
        }
    }
    /*
     * Either All filters returned neutral => Accept msg
     * or No msg filters => All msgs accepted by default
     */
    return(PVLOGGER_FILTER_ACCEPT);
}

void PVLogger::LogMsg(message_id_type msgID, const char *fmt, va_list arguments)
{
    uint32 i;

    for (i = 0; i < _pOwnAppenderVec.size(); i++)
    {
        PVLoggerAppender *appender = _pOwnAppenderVec[i];
        appender->AppendString(msgID, fmt, arguments);
    }
    return;
}

void PVLogger::LogMsg(message_id_type msgID, int32 numPairs, va_list arguments)
{
    uint32 i;

    for (i = 0; i < _pOwnAppenderVec.size(); i++)
    {
        PVLoggerAppender *appender = _pOwnAppenderVec[i];
        appender->AppendBuffers(msgID, numPairs, arguments);
    }
    return;
}
#endif //PVLOGGER_ENABLE

OSCL_EXPORT_REF PVLoggerRegistry* PVLoggerRegistry::GetPVLoggerRegistry()
{
#if(PVLOGGER_ENABLE)
    PVLOGGER_REGISTRY_WRAPPER< PVLoggerRegistry, PVLOGGER_REGISTRY_ID > pvLogRegSng;
    return &(*pvLogRegSng);
#else
    return NULL;
#endif
}


OSCL_EXPORT_REF PVLoggerRegistry::PVLoggerRegistry()
{
#if(PVLOGGER_ENABLE)
    /*
     * Create the root logger node, by default turn off logging
     * for the root node
     */
    OsclAny* ptr = _pvloggerAlloc.allocate(sizeof(PVLogger));
    if (ptr)
    {
        PVLogger *logger = new(ptr) PVLogger(rootTag, PVLOGGER_LEVEL_UNINTIALIZED, true);

        // add logger to the tag tree
        _loggerTree[OSCL_CONST_CAST(char*,(rootTag))] = logger;
    }
#endif
};

OSCL_EXPORT_REF PVLoggerRegistry::~PVLoggerRegistry()
{
#if(PVLOGGER_ENABLE)
    Oscl_TagTree<PVLogger*, alloc_type>::iterator iter;

    for (iter = _loggerTree.begin();
            iter != _loggerTree.end(); iter++)
    {
        PVLogger* logger = iter->value;
        logger->~PVLogger();
        _pvloggerAlloc.deallocate(logger);
    }
#endif
}

OSCL_EXPORT_REF PVLogger *PVLoggerRegistry::GetPVLoggerObject(const char* tagIn)
{
#if(PVLOGGER_ENABLE)
    Oscl_TagTree<PVLogger*, alloc_type>::iterator iter =
        _loggerTree.find(OSCL_CONST_CAST(char*, (tagIn)));

    if (iter != _loggerTree.end())
    {
        PVLogger* logger = iter->value;
        return(logger);
    }
    else
    {
        /* creates a new logger object */
        PVLogger *logger = NULL;
        logger = CreatePVLogger(tagIn, PVLOGGER_LEVEL_UNINTIALIZED, true);
        return(logger);
    }
#else
    OSCL_UNUSED_ARG(tagIn);
    return NULL;
#endif
}

OSCL_EXPORT_REF PVLogger *PVLoggerRegistry::CreatePVLogger(const char* tagIn, log_level_type level, bool oAppenderInheritance)
{
#if(PVLOGGER_ENABLE)
    Oscl_TagTree<PVLogger*, alloc_type>::iterator iter;

    /* If the input tag already exists in the tagtree, it should have a pointer value of NULL */
    OSCL_ASSERT((_loggerTree.find(OSCL_CONST_CAST(char* const&, (tagIn))) == _loggerTree.end()) ||
                (_loggerTree.find(OSCL_CONST_CAST(char* const&, (tagIn))))->value == 0);

    OsclAny* ptr = _pvloggerAlloc.allocate(sizeof(PVLogger));
    if (!ptr)
        return NULL;//fail gracefully

    PVLogger *logger = new(ptr) PVLogger(tagIn, level, oAppenderInheritance);

    // add logger to the tag tree
    _loggerTree[OSCL_CONST_CAST(char*,(tagIn))] = logger;

    // how many levels deep is the node we just inserted?
    iter = _loggerTree.find(OSCL_CONST_CAST(char*, (tagIn)));
    uint32 depth = iter->depth();

    // the tag tree will automatically create the parent, grandparent, etc.
    // make sure each ancestor's stats node is initialized, i.e. initialize each ancestor
    // until you reach one that is already initialized.
    Oscl_TagTree<PVLogger*, alloc_type>::node_ptr parent = iter->parent;

    uint32 ii = 0;

    for (ii = 0; ii < depth; ii++)
    {
        OSCL_ASSERT(parent != 0);

        // if initialized then we're done
        PVLogger* tmpPVLoggerNode = parent->value;

        if (tmpPVLoggerNode != NULL)
        {
            break;
        }

        // create new Logger node, for tag use the tag created
        // by the tag tree, for level use PV_LOG_LEVEL_UNINTIALIZED
        // for oAppenderInheritance use true
        OsclAny* ptr = _pvloggerAlloc.allocate(sizeof(PVLogger));
        if (ptr)
        {
            tmpPVLoggerNode = new(ptr) PVLogger(parent->tag.tag, PVLOGGER_LEVEL_UNINTIALIZED, true);

            // Add logger to tag tree
            parent->value = tmpPVLoggerNode;

            parent = parent->parent;
        }
    }

    //Inherit the log level for the newly created node.
    logger->SetLogLevel(level);

    parent = iter->parent;
    OSCL_ASSERT(parent != NULL);
    //Set the parent of the newly created node
    logger->SetParent(parent->value);

    //Set the parent of the other nodes up the tree
    for (ii = 0; ii < depth - 1; ii++)
    {

        PVLogger* tmpPVLoggerNode = parent->value;

        parent = parent->parent;
        OSCL_ASSERT(parent != NULL);

        //Set the parent for the node
        tmpPVLoggerNode->SetParent(parent->value);

    }

    return(logger);
#else
    OSCL_UNUSED_ARG(tagIn);
    OSCL_UNUSED_ARG(level);
    OSCL_UNUSED_ARG(oAppenderInheritance);
    return NULL;
#endif
};

OSCL_EXPORT_REF bool PVLoggerRegistry::SetNodeLogLevelExplicit(char* tagIn,
        log_level_type level)
{
#if(PVLOGGER_ENABLE)
    Oscl_TagTree<PVLogger*, alloc_type>::iterator iter;

    iter = _loggerTree.find(tagIn);

    if (iter != _loggerTree.end())
    {
        Oscl_TagTree<PVLogger*, alloc_type>::node_type* currNode = &(*iter);

        SetNodeLogLevelExplicit(currNode, level);

        return true;
    }
#else
    OSCL_UNUSED_ARG(tagIn);
    OSCL_UNUSED_ARG(level);
#endif
    return false;

}

OSCL_EXPORT_REF void PVLoggerRegistry::SetNodeLogLevelExplicit(Oscl_TagTree<PVLogger*, alloc_type>::node_type* node,
        log_level_type level)
{
#if(PVLOGGER_ENABLE)
    uint32 num_children = node->children.size();

    for (uint32 i = 0; i < num_children; i++)
    {
        Oscl_TagTree<PVLogger*, alloc_type>::node_ptr child = (node->children)[i];

        PVLogger* tmpPVLoggerNode = child->value;
        tmpPVLoggerNode->SetLogLevel(level);
        SetNodeLogLevelExplicit(child, level);
    }
#else
    OSCL_UNUSED_ARG(node);
    OSCL_UNUSED_ARG(level);
#endif
};
