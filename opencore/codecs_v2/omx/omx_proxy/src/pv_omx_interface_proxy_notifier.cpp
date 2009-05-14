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
#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif


#ifndef PV_OMX_INTERFACE_PROXY_H_INCLUDED
#include "pv_omx_interface_proxy.h"
#endif

#ifndef PV_OMX_INTERFACE_PROXY_NOTIFIER_H_INCLUDED
#include "pv_omx_interface_proxy_notifier.h"
#endif

#if PROXY_INTERFACE
void CPVInterfaceProxyNotifier_OMX::Run()
{
    if (Status() != OSCL_REQUEST_ERR_NONE)
        return;

    CPVProxyMsg_OMX notice(0, 0, 0, NULL);

    //Enter critical section...
    iProxy->iNotifierQueueCrit.Lock();

    //check notice queue...
    if (iProxy->iNotificationQueue.size() > 0)
    {
        notice = iProxy->iNotificationQueue[0];
        iProxy->iNotificationQueue.erase(&iProxy->iNotificationQueue[0]);
    }

    PendForExec();

    //if there's another message queued,
    //go ahead and signal request complete.
    if (iProxy->iNotificationQueue.size() > 0)
        PendComplete(OSCL_REQUEST_ERR_NONE);

    //Leave critical section...
    iProxy->iNotifierQueueCrit.Unlock();

    //Process the notice
    if (notice.iMsg)
    {
        //call proxy client message handler.
        CPVProxyInterface_OMX *proxy = iProxy->FindInterface(notice.iProxyId);
        if (proxy)
            proxy->iClient->HandleNotification(notice.iMsgId, notice.iMsg);
        else
            OSCL_ASSERT(NULL != proxy);//debug error.
    }

}
#endif
