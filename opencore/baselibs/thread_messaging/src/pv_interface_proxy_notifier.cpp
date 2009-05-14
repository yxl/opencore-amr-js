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

#ifndef PV_INTERFACE_PROXY_H_INCLUDED
#include "pv_interface_proxy.h"
#endif

#ifndef PV_INTERFACE_PROXY_NOTIFIER_H_INCLUDED
#include "pv_interface_proxy_notifier.h"
#endif

void CPVInterfaceProxyNotifier::Run()
{
    if (Status() != OSCL_REQUEST_ERR_NONE)
        return;

    CPVProxyMsg notice(0, 0, NULL);

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
        CPVProxyInterface *proxy = iProxy->FindInterface(notice.iProxyId);
        if (proxy)
            proxy->iClient->HandleNotification(notice.iMsgId, notice.iMsg);
        else
            OSCL_ASSERT(0);//debug error.
    }

}
