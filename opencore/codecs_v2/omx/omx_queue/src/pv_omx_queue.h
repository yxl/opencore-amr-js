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

/*******************
 * Implementation of a FIFO structure for queueing OMX buffers and commands.
********************/


#ifndef PV_OMX_QUEUE_H_INCLUDED
#define PV_OMX_QUEUE_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Types_h
#include "OMX_Types.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

/* Queue Depth i.e maximum number of elements */
#define MAX_QUEUE_ELEMENTS 12


/* Queue Element Type */
struct QueueElement
{
    QueueElement* pQueueNext;
    void* pData;
};

typedef struct QueueType
{
    QueueElement* pFirst;	/** Queue Front */
    QueueElement* pLast;	/** Queue Rear (last filled element of queue) */
    OMX_S32 NumElem;		/** Number of elements currently in the queue */
    OMX_S32 NumElemAdded;	/** Number of elements added extra at run time*/
}QueueType;



//QUEUE OPERATIONS

/* Queue initialization routine */
OSCL_IMPORT_REF OMX_ERRORTYPE QueueInit(QueueType* aQueue);

/* Queue deinitialization routine */
OSCL_IMPORT_REF void QueueDeinit(QueueType* aQueue);

/* Function to queue an element in the given queue*/
OSCL_IMPORT_REF OMX_ERRORTYPE Queue(QueueType* aQueue, void* aData);

/* Function to dequeue an element from the given queue
 * Returns NULL if the queue is empty */
OSCL_IMPORT_REF void* DeQueue(QueueType* aQueue);

/* Function to get number of elements currently in the queue */
OSCL_IMPORT_REF OMX_S32 GetQueueNumElem(QueueType* aQueue);

/* Add new element in the queue if required */
OSCL_IMPORT_REF OMX_BOOL AddQueueElem(QueueType* aQueue);

#endif		//#ifndef PV_OMX_QUEUE_H_INCLUDED
