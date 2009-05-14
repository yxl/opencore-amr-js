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

#include "pv_omx_queue.h"

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF OMX_ERRORTYPE QueueInit(QueueType* aQueue)
{
    OMX_S32 ii;
    QueueElement* pTempQElement = NULL;
    aQueue->pFirst = NULL;

    aQueue->pFirst = (QueueElement*) oscl_malloc(sizeof(QueueElement));
    if (NULL == aQueue->pFirst)
    {
        return OMX_ErrorInsufficientResources;
    }
    oscl_memset(aQueue->pFirst, 0, sizeof(QueueElement));

    aQueue->pLast = aQueue->pFirst;
    aQueue->NumElem = 0;
    aQueue->NumElemAdded = 0;

    for (ii = 0; ii < MAX_QUEUE_ELEMENTS - 1; ii++)
    {
        pTempQElement = (QueueElement*) oscl_malloc(sizeof(QueueElement));
        if (NULL == pTempQElement)
        {
            return OMX_ErrorInsufficientResources;
        }

        oscl_memset(pTempQElement, 0, sizeof(QueueElement));

        aQueue->pLast->pQueueNext = pTempQElement;
        aQueue->pLast = pTempQElement;
        pTempQElement = NULL;
    }

    aQueue->pLast->pQueueNext = aQueue->pFirst;
    aQueue->pLast = aQueue->pFirst;

    return OMX_ErrorNone;

}

OSCL_EXPORT_REF OMX_BOOL AddQueueElem(QueueType* aQueue)
{
    QueueElement* pTempQElement = NULL;

    pTempQElement = (QueueElement*) oscl_malloc(sizeof(QueueElement));
    if (NULL == pTempQElement)
    {
        return OMX_FALSE;
    }

    oscl_memset(pTempQElement, 0, sizeof(QueueElement));
    aQueue->pLast->pQueueNext = pTempQElement;
    pTempQElement->pQueueNext = aQueue->pFirst;

    aQueue->NumElemAdded++;

    return OMX_TRUE;

}

OSCL_EXPORT_REF void QueueDeinit(QueueType* aQueue)
{
    OMX_S32 ii;
    QueueElement* pTempQElement;

    OMX_S32 QueueElemCount = aQueue->NumElemAdded + MAX_QUEUE_ELEMENTS;

    pTempQElement = aQueue->pFirst;

    for (ii = 0; ii < QueueElemCount	; ii++)
    {
        if (pTempQElement != NULL)
        {
            pTempQElement = pTempQElement->pQueueNext;
            oscl_free(aQueue->pFirst);
            aQueue->pFirst = pTempQElement;
        }
    }
}

OSCL_EXPORT_REF OMX_ERRORTYPE Queue(QueueType* aQueue, void* aData)
{
    if (aQueue->NumElem == (MAX_QUEUE_ELEMENTS + aQueue->NumElemAdded))
    {
        if (OMX_FALSE == AddQueueElem(aQueue))
        {
            return OMX_ErrorInsufficientResources;
        }
    }

    if (aQueue->NumElem != 0)
    {
        aQueue->pLast = aQueue->pLast->pQueueNext;
    }

    aQueue->pLast->pData = aData;
    aQueue->NumElem++;

    return OMX_ErrorNone;
}

OSCL_EXPORT_REF void* DeQueue(QueueType* aQueue)
{
    void* pData;
    if (NULL == aQueue->pFirst->pData)
    {
        return NULL;
    }

    pData = aQueue->pFirst->pData;
    aQueue->pFirst->pData = NULL;
    aQueue->pFirst = aQueue->pFirst->pQueueNext;
    aQueue->NumElem--;

    if (0 == aQueue->NumElem)
    {
        aQueue->pLast = aQueue->pFirst;
    }

    return pData;
}

OSCL_EXPORT_REF OMX_S32 GetQueueNumElem(QueueType* aQueue)
{
    return aQueue->NumElem;
}

