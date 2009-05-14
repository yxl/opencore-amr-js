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

#ifndef OMX_PROXY_INTERFACE_H_INCLUDED
#define OMX_PROXY_INTERFACE_H_INCLUDED

#ifndef OMX_Types_h
#include "OMX_Types.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef OMX_Audio_h
#include "OMX_Audio.h"
#endif

#ifndef OMX_Video_h
#include "OMX_Video.h"
#endif

#ifndef OMX_Image_h
#include "OMX_Image.h"
#endif

#ifndef OMX_Other_h
#include "OMX_Other.h"
#endif

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef PV_OMX_INTERFACE_PROXY_H_INCLUDED
#include "pv_omx_interface_proxy.h"
#endif

#ifndef PV_OMX_PROXIED_INTERFACE_H_INCLUDED
#include "pv_omx_proxied_interface.h"
#endif

#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif

class ThreadSafeMemPoolFixedChunkAllocator;

#if PROXY_INTERFACE

#define TERM_PROXY_ID 773

#define GET_PARAMS 0
#define SET_PARAMS 1
#define GET_CONFIG 2
#define SET_CONFIG 3
#define GET_EXT	   4
#define GET_ST	   5
#define USE_BUF	   6
#define ALLOC_BUF  7
#define FREE_BUF   8
#define SET_CALL   9
#define SEND_COMM  10
#define EMPTY_BUF  11
#define FILL_BUF   12
#define GET_HANDLE  13
#define FREE_HANDLE 14



void CreateAppenders();

class ProxyApplication_OMX
            : public PVProxiedEngine_OMX
            , public PVProxiedInterfaceServer_OMX
            , public PVProxiedInterfaceClient_OMX
{
    public:
        OSCL_IMPORT_REF ProxyApplication_OMX();
        OSCL_IMPORT_REF virtual ~ProxyApplication_OMX();
        OSCL_IMPORT_REF bool Start();
        OSCL_IMPORT_REF void Exit();
        void TermCmd();

        void CreateLoggerAppenders()
        {
            iNumCreateAppenders++;
            CreateAppenders();
        }
        //from PVProxiedApplication
        void PVThreadLogon(PVMainProxy_OMX&);
        void PVThreadLogoff(PVMainProxy_OMX&);

        //from PVProxiedInterfaceServer
        void HandleCommand(TPVProxyMsgId , TPVCommandId, OsclAny* aData);
        void CleanupNotification(TPVProxyMsgId , OsclAny* aData);

        //from PVProxiedInterfaceClient
        void HandleNotification(TPVProxyMsgId , OsclAny* aData);
        void CleanupCommand(TPVProxyMsgId , OsclAny* aData);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyGetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxySetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyGetExtensionIndex(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType) ;

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyGetState(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STATETYPE* pState);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyGetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxySetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyUseBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyAllocateBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyFreeBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxySetCallbacks(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxySendCommand(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_COMMANDTYPE Cmd,
            OMX_IN  OMX_U32 nParam,
            OMX_IN  OMX_PTR pCmdData);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyEmptyThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyFillThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyGetHandle(
            OMX_OUT OMX_HANDLETYPE* pHandle,
            OMX_IN  OMX_STRING cComponentName,
            OMX_IN  OMX_PTR pAppData,
            OMX_IN  OMX_CALLBACKTYPE* pCallBacks);

        OSCL_IMPORT_REF OMX_ERRORTYPE ProxyFreeHandle(
            OMX_IN OMX_HANDLETYPE hComponent);

        /************************
         * Function pointers that will point to corresponding component's
         * OpenmaxAO class functions to be called later from ProcessMessage()
         * of ProxyApplication Class
         ************************/

        OMX_ERRORTYPE(*ComponentGetConfig)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        OMX_ERRORTYPE(*ComponentSetConfig)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

        OMX_ERRORTYPE(*ComponentGetExtensionIndex)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType) ;

        OMX_ERRORTYPE(*ComponentGetState)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STATETYPE* pState);

        OMX_ERRORTYPE(*ComponentGetParameter)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        OMX_ERRORTYPE(*ComponentSetParameter)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);

        OMX_ERRORTYPE(*ComponentUseBuffer)(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer);

        OMX_ERRORTYPE(*ComponentAllocateBuffer)(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes);

        OMX_ERRORTYPE(*ComponentFreeBuffer)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OMX_ERRORTYPE(*ComponentSetCallbacks)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData);

        OMX_ERRORTYPE(*ComponentSendCommand)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_COMMANDTYPE Cmd,
            OMX_IN  OMX_U32 nParam,
            OMX_IN  OMX_PTR pCmdData);

        OMX_ERRORTYPE(*ComponentEmptyThisBuffer)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OMX_ERRORTYPE(*ComponentFillThisBuffer)(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OsclAny *GetMemPoolPtr()
        {
            return (OsclAny*) iMemoryPool;
        };


    private:
        TPVProxyId iProxyId;
        CPVInterfaceProxy_OMX* ipProxy;
        int32 iNumMessage;
        int32 iNumNotice;
        int32 iMemCmd, iMemResp;
        int32 iNumClientMsg;
        int32 iNumCreate, iNumCreateAppenders, iNumDelete;
        void ProcessMessage(TPVCommandId, OsclAny*);
        void CleanupMessage(OsclAny*);
        OsclSemaphore iInitSemOmx;
        OMX_ERRORTYPE ReturnValueOmxApi;
        ThreadSafeMemPoolFixedChunkAllocator *iMemoryPool;
};

/*********************************
Command/Message classes for each openmax API
**********************************/

class GetParameterMsg
{
    public:
        GetParameterMsg(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nParams, OMX_PTR CompStr)
                : hComponent(hComp), nParamIndex(nParams), ComponentParameterStructure(CompStr)
        {}
        ~GetParameterMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_INDEXTYPE nParamIndex;
        OMX_PTR ComponentParameterStructure;
};

class SetParameterMsg
{
    public:
        SetParameterMsg(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nParams, OMX_PTR CompStr)
                : hComponent(hComp), nParamIndex(nParams), ComponentParameterStructure(CompStr)
        {}
        ~SetParameterMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_INDEXTYPE nParamIndex;
        OMX_PTR ComponentParameterStructure;
};

class GetConfigMsg
{
    public:
        GetConfigMsg(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nInd, OMX_PTR pCompConfig)
                : hComponent(hComp), nIndex(nInd), pComponentConfigStructure(pCompConfig)
        {}
        ~GetConfigMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_INDEXTYPE nIndex;
        OMX_PTR pComponentConfigStructure;
};

class SetConfigMsg
{
    public:
        SetConfigMsg(OMX_HANDLETYPE hComp, OMX_INDEXTYPE nInd, OMX_PTR pCompConfig)
                : hComponent(hComp), nIndex(nInd), pComponentConfigStructure(pCompConfig)
        {}
        ~SetConfigMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_INDEXTYPE nIndex;
        OMX_PTR pComponentConfigStructure;
};

class GetExtMsg
{
    public:
        GetExtMsg(OMX_HANDLETYPE hComp, OMX_STRING cParam, OMX_INDEXTYPE* pInd)
                : hComponent(hComp), cParameterName(cParam), pIndexType(pInd)
        {}
        ~GetExtMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_STRING cParameterName;
        OMX_INDEXTYPE* pIndexType;
};

class GetStateMsg
{
    public:
        GetStateMsg(OMX_HANDLETYPE hComp, OMX_STATETYPE* pSt)
                : hComponent(hComp), pState(pSt)
        {}
        ~GetStateMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_STATETYPE* pState;
};

class UseBufMsg
{
    public:
        UseBufMsg(OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE** ppBuf, OMX_U32 nPtIndex, OMX_PTR pApPrv, OMX_U32 nSz, OMX_U8* pBuf)
                : hComponent(hComp), ppBufferHdr(ppBuf), nPortIndex(nPtIndex), pAppPrivate(pApPrv), nSizeBytes(nSz), pBuffer(pBuf)
        {}
        ~UseBufMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_BUFFERHEADERTYPE** ppBufferHdr;
        OMX_U32 nPortIndex;
        OMX_PTR pAppPrivate;
        OMX_U32 nSizeBytes;
        OMX_U8* pBuffer;

};

class AllocBufMsg
{
    public:
        AllocBufMsg(OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE** ppBuf, OMX_U32 nPtIndex, OMX_PTR pApPrv, OMX_U32 nSz)
                : hComponent(hComp), pBuffer(ppBuf), nPortIndex(nPtIndex), pAppPrivate(pApPrv), nSizeBytes(nSz)
        {}
        ~AllocBufMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_BUFFERHEADERTYPE** pBuffer;
        OMX_U32 nPortIndex;
        OMX_PTR pAppPrivate;
        OMX_U32 nSizeBytes;

};

class FreeBufMsg
{
    public:
        FreeBufMsg(OMX_HANDLETYPE hComp, OMX_U32 nPtInd, OMX_BUFFERHEADERTYPE* pBuf)
                : hComponent(hComp), nPortIndex(nPtInd), pBuffer(pBuf)
        {}
        ~FreeBufMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_U32 nPortIndex;
        OMX_BUFFERHEADERTYPE* pBuffer;

};

class SetCallMsg
{
    public:
        SetCallMsg(OMX_HANDLETYPE hComp, OMX_CALLBACKTYPE* pCallb, OMX_PTR pAppD)
                : hComponent(hComp), pCallbacks(pCallb), pAppData(pAppD)
        {}
        ~SetCallMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_CALLBACKTYPE* pCallbacks;
        OMX_PTR pAppData;

};

class SetCommMsg
{
    public:
        SetCommMsg(OMX_HANDLETYPE hComp, OMX_COMMANDTYPE cmdtype, OMX_U32 nPar, OMX_PTR pCmdD)
                : hComponent(hComp), Cmd(cmdtype), nParam(nPar), pCmdData(pCmdD)
        {}
        ~SetCommMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_COMMANDTYPE Cmd;
        OMX_U32 nParam;
        OMX_PTR pCmdData;
};

class EmptyBufMsg
{
    public:
        EmptyBufMsg(OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE* pBuf)
                : hComponent(hComp), pBuffer(pBuf)
        {}
        ~EmptyBufMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_BUFFERHEADERTYPE* pBuffer;

};

class FillBufMsg
{
    public:
        FillBufMsg(OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE* pBuf)
                : hComponent(hComp), pBuffer(pBuf)
        {}
        ~FillBufMsg()
        {}

        OMX_HANDLETYPE hComponent;
        OMX_BUFFERHEADERTYPE* pBuffer;

};

class GetHandleMsg
{
    public:
        GetHandleMsg(OMX_HANDLETYPE* pHan, OMX_STRING cCompName,
                     OMX_PTR pAppD, OMX_CALLBACKTYPE* pCallB)
                : pHandle(pHan), cComponentName(cCompName), pAppData(pAppD), pCallBacks(pCallB)
        {}
        ~GetHandleMsg()
        {}

        OMX_HANDLETYPE* pHandle;
        OMX_STRING cComponentName;
        OMX_PTR pAppData;
        OMX_CALLBACKTYPE* pCallBacks;

};

class FreeHandleMsg
{
    public:
        FreeHandleMsg(OMX_HANDLETYPE hComp)
                : hComponent(hComp)
        {}
        ~FreeHandleMsg()
        {}

        OMX_HANDLETYPE hComponent;

};


/******************************************/

//Taken from pv's thread messaging folder
#define COMMANDDATA 999
class proxyApplicationCommand
{
    public:
        proxyApplicationCommand()
        {
            data = 999;
        }
        bool IsValid()
        {
            return data == COMMANDDATA;
        }
        int32 data;
};

#define RESPONSEDATA 99
class proxyApplicationResponse
{
    public:
        proxyApplicationResponse()
        {
            data = RESPONSEDATA;
        }
        bool IsValid()
        {
            return data == RESPONSEDATA;
        }
        int32 data;
};

class BasicDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            delete(PVLoggerAppender*)ptr;
        }
};


/*Component Handle Functions*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY GlobalProxyComponentGetHandle(
    OMX_OUT OMX_HANDLETYPE* pHandle,
    OMX_IN  OMX_STRING cComponentName, OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks,
    OMX_IN	OMX_PTR pProxy);

OMX_API OMX_ERRORTYPE OMX_APIENTRY GlobalProxyComponentFreeHandle(
    OMX_IN OMX_HANDLETYPE hComponent);

#endif // PROXY_INTERFACE

#endif //OMX_PROXY_INTERFACE_H_INCLUDED

