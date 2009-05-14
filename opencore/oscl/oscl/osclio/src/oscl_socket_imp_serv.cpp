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
#include "oscl_socket_tuneables.h"

#if(PV_SOCKET_SERVER)

#include "oscl_socket_imp.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_scheduler_ao.h"
#include "oscl_socket_method.h"
#include "oscl_error.h"

/** This file contains all the routines called by
 *  the PV Socket server ProcessSocketRequests routine to process active
 *  requests in the select loop.
 */

//Logger macro for socket server logging.
#if(PV_OSCL_SOCKET_SERVER_LOGGER_OUTPUT)
#include "pvlogger.h"
#define LOGSERV(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iSocketServ->iLogger,PVLOGMSG_DEBUG,m);
#else
#define LOGSERV(m)
#endif
/*
//disable this if it's too much
#undef LOGSERV
#define LOGSERV
*/

//Stats macros for use with OsclSocketI
#if(PV_OSCL_SOCKET_STATS_LOGGING)
#define ADD_STATS(x,y) iStats.Add(x,y)
#define ADD_STATSP(x,y,z) iStats.Add(x,y,z)
#define CONSTRUCT_STATS(x,y) iStats.Construct(x,y)
#else
#define ADD_STATS(x,y)
#define ADD_STATSP(x,y,z)
#define CONSTRUCT_STATS(x,y)
#endif

/**
 * Server calls this to process an active Connect request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessConnect(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int sockerr = 0;
    int32 complete = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (aElem->iSelect)
    {
        //check for completion by examining
        //writeset and exceptset
        bool ok, success, fail;
#if (PV_SOCKET_SERVER_SELECT)
        OsclConnectComplete(iSocket,
                            iSocketServ->iWriteset,
                            iSocketServ->iExceptset,
                            success,
                            fail,
                            ok,
                            sockerr);
#else
        //since there was no select call we have to do it now.
        fd_set readset, writeset, exceptset;
        FD_ZERO(&writeset);
        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        FD_SET(iSocket, &writeset);
        FD_SET(iSocket, &exceptset);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        int nfds = iSocket + 1;
        int serverr;
        int nhandles;
        OsclSocketSelect(nfds, readset, writeset, exceptset, timeout, ok, serverr, nhandles);
        if (!ok)
        {
            //select failed.
            sockerr = serverr;
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
        }
        else if (!nhandles)
        {
            //keep waiting.
            success = fail = false;
        }
        else
        {
            //examine results
            OsclConnectComplete(iSocket,
                                writeset,
                                exceptset,
                                success,
                                fail,
                                ok,
                                sockerr);
        }
#endif

        if (success)
        {
            //done, no error.
            ADD_STATS(req->iParam->iFxn, EOsclSocket_Writable);

            iSocketConnected = true;
            complete = OSCL_REQUEST_ERR_NONE;
            iscomplete = true;
        }
        else if (fail)
        {
            //done, with error.
            ADD_STATS(req->iParam->iFxn, EOsclSocket_Except);

            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
        }
        else
        {
            //keep waiting
            ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
        }
    }
    else
    {
        //Start connecting
        ConnectParam *param = (ConnectParam*)req->iParam;
        TOsclSockAddr addr;
        MakeAddr(param->iAddr, addr);
        bool ok, wouldblock;
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclConnect(iSocket, addr, ok, sockerr, wouldblock);
        if (!ok)
        {
            if (wouldblock)
            {
                //we expect a non-blocking socket to return an error...
                //start monitoring the writeset and exceptset.
                aElem->iSelect = (OSCL_WRITESET_FLAG | OSCL_EXCEPTSET_FLAG);
            }
            else
            {
                //connect error.
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
        }
        else
        {
            //Finished.
            iSocketConnected = true;
            complete = OSCL_REQUEST_ERR_NONE;
            iscomplete = true;
        }
    }

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessConnect (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);//should have bits set.
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessConnect (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

/**
 * Server calls this to process an active Shutdown request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessShutdown(OsclSocketServRequestQElem* aElem)
{
    int err = 0;
    int32 complete = 0;

    OsclSocketRequest *req = aElem->iSocketRequest;
    {
        ShutdownParam* param = (ShutdownParam*)req->iParam;
        int how = GetShutdown(param->iHow);
        bool ok;
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclShutdown(iSocket, how, ok, err);
        if (!ok)
        {
            //shutdown error
            complete = OSCL_REQUEST_ERR_GENERAL;
        }
        else
        {
            complete = OSCL_REQUEST_ERR_NONE;
        }
    }
    LOGSERV((0, "OsclSocketI::ProcessShutdown (0x%x) request complete %d sockerr %d", this, complete, err));
    req->Complete(aElem, complete, err);
}

/**
 * Server calls this to process an active Accept request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessAccept(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int sockerr = 0;
    int32 complete = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (!aElem->iSelect)
    {
        //start monitoring.
        AcceptParam* param = (AcceptParam*)req->iParam;
        if (!param->iBlankSocket)
        {
            //bad request.
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            sockerr = PVSOCK_ERR_BAD_PARAM;//unexpected
        }
        else
        {
            aElem->iSelect = (OSCL_READSET_FLAG | OSCL_EXCEPTSET_FLAG);
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else if (FD_ISSET(iSocket, &iSocketServ->iExceptset))
    {
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Except);

        bool ok;
        OsclGetAsyncSockErr(iSocket, ok, sockerr);
        complete = OSCL_REQUEST_ERR_GENERAL;
        iscomplete = true;
    }
    else if (FD_ISSET(iSocket, &iSocketServ->iReadset))
    {
        //socket is readable, we can do an accept call now.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Readable);
#else
    {
        //try the accept.
#endif
        TOsclSocket acceptsock;
        bool ok, wouldblock;
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclAccept(iSocket, acceptsock, ok, sockerr, wouldblock);
        if (!ok)
        {
            if (wouldblock)
            {
#if (PV_SOCKET_SERVER_SELECT)
                //we don't expect wouldblock when socket is readable.
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
#else
                //keep waiting for socket to be readable.
                ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
#endif
            }
            else
            {
                //accept error
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
        }
        else
        {
            //Set accept socket to non-blocking mode.
            OsclSetNonBlocking(acceptsock, ok, sockerr);
            if (!ok)
            {
                OsclCloseSocket(acceptsock, ok, sockerr);
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
            else
            {
                //Save the accept socket in the blank socket that was provided
                //in the Accept command.
                AcceptParam* param = (AcceptParam*)req->iParam;
                param->iBlankSocket->iSocket = acceptsock;
                //the blank socket is now valid and connected.
                param->iBlankSocket->InitSocket(true);
                param->iBlankSocket->iSocketConnected = true;
                complete = OSCL_REQUEST_ERR_NONE;
                iscomplete = true;
            }
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else
    {
        //keep waiting for socket to be readable.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
    }
#endif

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessAccept (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessAccept (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

/**
 * Server calls this to process an active Send request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessSend(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int32 complete = 0;
    int sockerr = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (!aElem->iSelect)
    {
        //start monitoring.
        //don't monitor exceptset b/c windows reports some false errors.
        SendParam *param = (SendParam*)req->iParam;
        if (!param->iBufSend.iPtr)
        {
            //bad request
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            sockerr = PVSOCK_ERR_BAD_PARAM;
        }
        else
        {
            aElem->iSelect = (OSCL_WRITESET_FLAG);
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else if (FD_ISSET(iSocket, &iSocketServ->iWriteset))
    {
        //socket is writable, send data
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Writable);
#else
    {
        //go ahead and try to send
#endif
        SendParam *param = (SendParam*)req->iParam;
        int nbytes;
        bool ok, wouldblock;
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclSend(iSocket,
                 param->iBufSend.iPtr + param->iXferLen,
                 param->iBufSend.iLen - param->iXferLen,
                 ok,
                 sockerr,
                 nbytes,
                 wouldblock);

        //check send result...
        if (!ok)
        {
            if (wouldblock)
            {
                //non-blocking sockets return this when there's no receiver.
                //just keep waiting.
                ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
            }
            else
            {
                //send error
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
        }
        else
        {
            //sent data.
            ADD_STATSP(req->iParam->iFxn, EOsclSocket_DataSent, nbytes);

            param->iXferLen += nbytes;

            //Check for completion
            if (param->iXferLen == param->iBufSend.iLen)
            {
                //all data sent.
                complete = OSCL_REQUEST_ERR_NONE;
                iscomplete = true;
            }
            else if (nbytes == 0)
            {
                //connection closed
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
                //(sockerr will be zero in this case.)
            }
            else
            {
                //keep sending data.
                ;
            }
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else
    {
        //keep waiting for socket to be writable.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
    }
#endif

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessSend (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessSend (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

/**
 * Server calls this to process an active SendTo request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessSendTo(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int32 complete = 0;
    int sockerr = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (!aElem->iSelect)
    {
        //start monitoring.  don't monitor exceptset b/c windows reports some false errors.
        SendToParam *param = (SendToParam*)req->iParam;
        if (!param->iBufSend.iPtr)
        {
            //bad request
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            sockerr = PVSOCK_ERR_BAD_PARAM;//unexpected
        }
        else
        {
            aElem->iSelect = (OSCL_WRITESET_FLAG);
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else if (FD_ISSET(iSocket, &iSocketServ->iWriteset))
    {
        //socket is writable, send data
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Writable);
#else
    {
        //go ahead and try to send
#endif

        SendToParam *param = (SendToParam*)req->iParam;
        int nbytes;
        bool wouldblock, ok;
        TOsclSockAddr toaddr;
        MakeAddr(param->iAddr, toaddr);
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclSendTo(iSocket,
                   param->iBufSend.iPtr + param->iXferLen,
                   param->iBufSend.iLen - param->iXferLen,
                   toaddr,
                   ok,
                   sockerr,
                   nbytes,
                   wouldblock);

        if (!ok)
        {
            if (wouldblock)
            {
                //nonblocking socket returns this error
                //just keep waiting
                ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
            }
            else
            {
                //sendto error
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
        }
        else
        {
            //sent data.
            ADD_STATSP(req->iParam->iFxn, EOsclSocket_DataSent, nbytes);

            param->iXferLen += nbytes;

            //Check for completion
            if (param->iXferLen == param->iBufSend.iLen)
            {
                //all data sent.
                complete = OSCL_REQUEST_ERR_NONE;
                iscomplete = true;
            }
            else if (nbytes == 0)
            {
                //connection closed
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
                //(sockerr will be zero in this case)
            }
            else
            {
                //keep sending data.
                ;
            }
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else
    {
        //keep waiting for socket to be writable.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
    }
#endif

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessSendTo (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessSendTo (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

/**
 * Server calls this to process an active Recv request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessRecv(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int sockerr = 0;
    int32 complete = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (!aElem->iSelect)
    {
        //start monitoring.  don't monitor exceptset b/c windows reports some false errors.
        RecvParam* param = (RecvParam*)req->iParam;
        if (!param->iBufRecv.iPtr)
        {
            //bad request.
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            sockerr = PVSOCK_ERR_BAD_PARAM;//unexpected
        }
        else
        {
            aElem->iSelect = (OSCL_READSET_FLAG);
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else if (FD_ISSET(iSocket, &iSocketServ->iReadset))
    {
        //socket is readable, get data.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Readable);
#else
    {
        //try to read
#endif
        RecvParam* param = (RecvParam*)req->iParam;
        int nbytes;
        bool ok, wouldblock;
        ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
        OsclRecv(iSocket,
                 param->iBufRecv.iPtr + param->iBufRecv.iLen,
                 param->iBufRecv.iMaxLen - param->iBufRecv.iLen,
                 ok,
                 sockerr,
                 nbytes,
                 wouldblock);

        //Check for completion or error.
        if (!ok)
        {
            if (wouldblock)
            {
                //nonblocking sockets return this when there's no
                //data.
                //keep waiting for data.
                ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
            }
            else
            {
                //recv error
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
            }
        }
        else if (nbytes > 0)
        {
            //got some data
            ADD_STATSP(req->iParam->iFxn, EOsclSocket_DataRecv, nbytes);

            param->iBufRecv.iLen += nbytes;

            complete = OSCL_REQUEST_ERR_NONE;
            iscomplete = true;
        }
        else
        {
            //this usually means connection was closed.
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            //(sockerr will be zero in this case)
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else
    {
        //keep waiting for socket to be readable.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
    }
#endif

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessRecv (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessRecv (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

/**
 * Server calls this to process an active RecvFrom request.
 *
 * This is called under the server thread or server AO during
 * the select loop.
 *
 * This routine may complete the request, or may set flags in the
 * read/write/except set to be monitored during the next select call.
 *
 */
void OsclSocketI::ProcessRecvFrom(OsclSocketServRequestQElem* aElem)
{
    bool iscomplete = false;
    int sockerr = 0;
    int32 complete = 0;
    OsclSocketRequest *req = aElem->iSocketRequest;
    if (!aElem->iSelect)
    {
        //start monitoring.  don't monitor exceptset b/c windows reports some false errors.
        RecvFromParam* param = (RecvFromParam*)req->iParam;
        if (!param->iBufRecv.iPtr)
        {
            //bad request.
            complete = OSCL_REQUEST_ERR_GENERAL;
            iscomplete = true;
            sockerr = PVSOCK_ERR_BAD_PARAM;//unexpected
        }
        else
        {
            aElem->iSelect = (OSCL_READSET_FLAG);
        }
    }
#if (PV_SOCKET_SERVER_SELECT)
    else if (FD_ISSET(iSocket, &iSocketServ->iReadset))
    {
        //socket is readable, get data.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_Readable);
#else
    {
        //try the read
#endif

        //we loop through multiple "recvfrom" calls and stop when
        //either a byte limit is reached or else no more data is available
        //without waiting.
        bool loop;
        uint32 loopcount;
        for (loop = true, loopcount = 0;loop;loopcount++)
        {
            loop = false;

            RecvFromParam* param = (RecvFromParam*)req->iParam;
            int nbytes;
            bool ok, wouldblock;
            TOsclSockAddr sourceaddr;
            TOsclSockAddrLen sourceaddrlen = sizeof(sourceaddr);
            ADD_STATS(req->iParam->iFxn, EOsclSocket_OS);
            OsclRecvFrom(iSocket,
                         param->iBufRecv.iPtr + param->iBufRecv.iLen,
                         param->iBufRecv.iMaxLen - param->iBufRecv.iLen,
                         &sourceaddr,
                         &sourceaddrlen,
                         ok,
                         sockerr,
                         nbytes,
                         wouldblock);

            //Check for completion or error.
            if (!ok)
            {
                if (wouldblock)
                {
                    //nonblocking sockets will return an error when
                    //there's no data.
                    if (loopcount == 0)
                    {
                        //keep waiting for data.
                        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
                    }
                    else
                    {
                        //if we already got some data, don't wait for more.
                        complete = OSCL_REQUEST_ERR_NONE;
                        iscomplete = true;
                    }
                }
                else
                {
                    //recvfrom error
                    complete = OSCL_REQUEST_ERR_GENERAL;
                    iscomplete = true;
                }
            }
            else if (nbytes > 0)
            {
                //got some data.
                ADD_STATSP(req->iParam->iFxn, EOsclSocket_DataRecv, nbytes);

                param->iBufRecv.iLen += nbytes;
                if (param->iPacketLen)
                    param->iPacketLen->push_back(nbytes);

                if (sourceaddrlen > 0)
                {
                    //convert the source address.
                    MakeAddr(sourceaddr, param->iAddr);
                    if (param->iPacketSource)
                        param->iPacketSource->push_back(param->iAddr);
                }

                //see whether to try and recv another packet
                //when multi-packet recv is enabled, keep receiving
                //as long as the free space is >= the multi recv limit.
                if (param->iMultiMaxLen > 0
                        && (param->iBufRecv.iMaxLen - param->iBufRecv.iLen) >= param->iMultiMaxLen)
                {
                    loop = true;
                }
                else
                {
                    complete = OSCL_REQUEST_ERR_NONE;
                    iscomplete = true;
                }
            }
            else
            {
                //this usually means connection was closed.
                complete = OSCL_REQUEST_ERR_GENERAL;
                iscomplete = true;
                //(sockerr will be zero in this case)
            }
        }//for loop
    }
#if (PV_SOCKET_SERVER_SELECT)
    else
    {
        //keep waiting for socket to be readable.
        ADD_STATS(req->iParam->iFxn, EOsclSocket_ServPoll);
    }
#endif

    //either keep monitoring activity, or complete the request.
    if (!iscomplete)
    {
        LOGSERV((0, "OsclSocketI::ProcessRecvFrom (0x%x) select=true", this));
        OSCL_ASSERT(aElem->iSelect);
    }
    else
    {
        LOGSERV((0, "OsclSocketI::ProcessRecvFrom (0x%x) request complete %d sockerr %d", this, complete, sockerr));
        req->Complete(aElem, complete, sockerr);
    }
}

#endif //PV_SOCKET_SERVER





