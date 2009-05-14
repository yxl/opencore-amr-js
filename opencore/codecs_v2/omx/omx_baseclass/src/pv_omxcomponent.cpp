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
#include "pv_omxcomponent.h"
#include "pv_omxdefs.h"
#include "oscl_types.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif


OmxComponentBase::OmxComponentBase() :
        OsclActiveObject(OsclActiveObject::EPriorityNominal, "OMXComponent")
{
    iLogger = PVLogger::GetLoggerObject("OmxComponentBase");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : constructed"));

    //Flag to call BufferMgmtFunction in the Run() when the component state is executing
    iBufferExecuteFlag = OMX_FALSE;
    ipAppPriv = NULL;

    ipCallbacks = NULL;
    iCallbackData = NULL;
    iState = OMX_StateLoaded;

    ipCoreDescriptor = NULL;
    iNumInputBuffer = 0;

    ipFrameDecodeBuffer = NULL;
    iPartialFrameAssembly = OMX_FALSE;
    iIsInputBufferEnded = OMX_TRUE;
    iEndofStream = OMX_FALSE;
    ipTempInputBuffer = NULL;
    iTempInputBufferLength = 0;

    ipTargetComponent = NULL;
    iTargetMarkData = NULL;
    iNewInBufferRequired = OMX_TRUE;
    iNewOutBufRequired = OMX_TRUE;

    iTempConsumedLength = 0;
    iOutBufferCount = 0;
    iCodecReady = OMX_FALSE;
    ipInputCurrBuffer = NULL;
    iInputCurrLength = 0;
    iFrameCount = 0;
    iStateTransitionFlag = OMX_FALSE;
    iEndOfFrameFlag = OMX_FALSE;
    ipInputBuffer = NULL;
    ipOutputBuffer = NULL;
    iInputCurrBufferSize = 0;

    iEosProcessing = OMX_FALSE;
    iFirstFragment = OMX_FALSE;
    iResizePending = OMX_FALSE;
    iFrameTimestamp = 0;
    iIsFirstOutputFrame = OMX_TRUE;
    iSilenceInsertionInProgress = OMX_FALSE;

    iNumPorts = 0;
    iCompressedFormatPortNum = OMX_PORT_INPUTPORT_INDEX;
    ipPorts = NULL;

    //Indicate whether component has been already initialized */
    iIsInit = OMX_FALSE;

    iGroupPriority = 0;
    iGroupID = 0;

    ipTempOutBufferForPortReconfig = NULL;
    iSendOutBufferAfterPortReconfigFlag = OMX_FALSE;
    iSizeOutBufferForPortReconfig = 0;

    iComponentRoleFlag = OMX_FALSE;

    ipMark = NULL;

}


OMX_ERRORTYPE OmxComponentBase::ConstructBaseComponent(OMX_PTR pAppData)
{
    OSCL_UNUSED_ARG(pAppData);
    OMX_U32 ii, jj;

    if (iNumPorts)
    {
        if (ipPorts)
        {
            oscl_free(ipPorts);
            ipPorts = NULL;
        }

        ipPorts = (ComponentPortType**) oscl_calloc(iNumPorts, sizeof(ComponentPortType*));
        if (!ipPorts)
        {
            return OMX_ErrorInsufficientResources;
        }

        for (ii = 0; ii < iNumPorts; ii++)
        {
            ipPorts[ii] = (ComponentPortType*) oscl_calloc(1, sizeof(ComponentPortType));
            if (!ipPorts[ii])
            {
                return OMX_ErrorInsufficientResources;
            }

            ipPorts[ii]->TransientState = OMX_StateMax;
            SetHeader(&ipPorts[ii]->PortParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            ipPorts[ii]->PortParam.nPortIndex = ii;

            /** Allocate and initialize buffer Queue */
            ipPorts[ii]->pBufferQueue = (QueueType*) oscl_malloc(sizeof(QueueType));

            if (NULL == ipPorts[ii]->pBufferQueue)
            {
                return OMX_ErrorInsufficientResources;
            }

            if (OMX_ErrorNone != QueueInit(ipPorts[ii]->pBufferQueue))
            {
                return OMX_ErrorInsufficientResources;
            }

            for (jj = 0; jj < OMX_PORT_NUMBER_FORMATS_SUPPORTED; jj++)
            {
                oscl_memset(&ipPorts[ii]->VideoParam[jj], 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            }
        }

        SetPortFlushFlag(iNumPorts, -1, OMX_FALSE);
        SetNumBufferFlush(iNumPorts, -1, OMX_FALSE);
    }


    iCodecReady = OMX_FALSE;
    ipCallbacks = NULL;
    iCallbackData = NULL;
    iState = OMX_StateLoaded;
    ipTempInputBuffer = NULL;
    iTempInputBufferLength = 0;
    iNumInputBuffer = 0;
    iPartialFrameAssembly = OMX_FALSE;
    iEndofStream = OMX_FALSE;
    iIsInputBufferEnded = OMX_TRUE;
    iNewOutBufRequired = OMX_TRUE;
    iEosProcessing = OMX_FALSE;
    iRepositionFlag = OMX_FALSE;
    iIsFirstOutputFrame = OMX_TRUE;
    iMarkPropagate = OMX_FALSE;
    ipTempOutBufferForPortReconfig = NULL;
    iSendOutBufferAfterPortReconfigFlag = OMX_FALSE;
    iSizeOutBufferForPortReconfig = 0;
    iComponentRoleFlag = OMX_FALSE;


    /* Initialize the asynchronous command Queue */
    if (ipCoreDescriptor)
    {
        oscl_free(ipCoreDescriptor);
        ipCoreDescriptor = NULL;
    }

    ipCoreDescriptor = (CoreDescriptorType*) oscl_malloc(sizeof(CoreDescriptorType));
    if (NULL == ipCoreDescriptor)
    {
        return OMX_ErrorInsufficientResources;
    }

    ipCoreDescriptor->pMessageQueue = NULL;
    ipCoreDescriptor->pMessageQueue = (QueueType*) oscl_malloc(sizeof(QueueType));
    if (NULL == ipCoreDescriptor->pMessageQueue)
    {
        return OMX_ErrorInsufficientResources;
    }

    if (OMX_ErrorNone != QueueInit(ipCoreDescriptor->pMessageQueue))
    {
        return OMX_ErrorInsufficientResources;
    }

    /** Default parameters setting */
    iIsInit = OMX_FALSE;
    iGroupPriority = 0;
    iGroupID = 0;
    ipMark = NULL;

    SetHeader(&iPortTypesParam, sizeof(OMX_PORT_PARAM_TYPE));

    iOutBufferCount = 0;
    iStateTransitionFlag = OMX_FALSE;
    iEndOfFrameFlag = OMX_FALSE;
    iFirstFragment = OMX_FALSE;

    //Will be used in case of partial frame assembly
    ipInputCurrBuffer = NULL;
    iInputCurrBufferSize = 0;
    ipAppPriv->CompHandle = &iOmxComponent;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxComponentBase::DestroyBaseComponent()
{
    OMX_U32 ii;

    /*Deinitialize and free ports semaphores and Queue*/
    for (ii = 0; ii < iNumPorts; ii++)
    {
        if (NULL != ipPorts[ii]->pBufferQueue)
        {
            QueueDeinit(ipPorts[ii]->pBufferQueue);
            oscl_free(ipPorts[ii]->pBufferQueue);
            ipPorts[ii]->pBufferQueue = NULL;
        }
        /*Free port*/
        if (NULL != ipPorts[ii])
        {
            oscl_free(ipPorts[ii]);
            ipPorts[ii] = NULL;
        }
    }

    if (ipPorts)
    {
        oscl_free(ipPorts);
        ipPorts = NULL;
    }

    iState = OMX_StateLoaded;

    //Free the temp output buffer
    if (ipTempOutBufferForPortReconfig)
    {
        oscl_free(ipTempOutBufferForPortReconfig);
        ipTempOutBufferForPortReconfig = NULL;
        iSizeOutBufferForPortReconfig = 0;
    }

    if (ipInputCurrBuffer)
    {
        oscl_free(ipInputCurrBuffer);
        ipInputCurrBuffer = NULL;
        iInputCurrBufferSize = 0;
    }

    if (ipTempInputBuffer)
    {
        oscl_free(ipTempInputBuffer);
        ipTempInputBuffer = NULL;
    }

    if (NULL != ipCoreDescriptor)
    {

        if (NULL != ipCoreDescriptor->pMessageQueue)
        {
            /* De-initialize the asynchronous command queue */
            QueueDeinit(ipCoreDescriptor->pMessageQueue);
            oscl_free(ipCoreDescriptor->pMessageQueue);
            ipCoreDescriptor->pMessageQueue = NULL;
        }

        oscl_free(ipCoreDescriptor);
        ipCoreDescriptor = NULL;
    }

    return OMX_ErrorNone;
}


/*********************
 *
 * Component verfication routines
 *
 **********************/

void OmxComponentBase::SetHeader(OMX_PTR aHeader, OMX_U32 aSize)
{
    OMX_VERSIONTYPE* pVersion = (OMX_VERSIONTYPE*)((OMX_STRING) aHeader + sizeof(OMX_U32));
    *((OMX_U32*) aHeader) = aSize;

    pVersion->s.nVersionMajor = SPECVERSIONMAJOR;
    pVersion->s.nVersionMinor = SPECVERSIONMINOR;
    pVersion->s.nRevision = SPECREVISION;
    pVersion->s.nStep = SPECSTEP;
}


OMX_ERRORTYPE OmxComponentBase::CheckHeader(OMX_PTR aHeader, OMX_U32 aSize)
{
    OMX_VERSIONTYPE* pVersion = (OMX_VERSIONTYPE*)((OMX_STRING) aHeader + sizeof(OMX_U32));

    if (NULL == aHeader)
    {
        return OMX_ErrorBadParameter;
    }

    if (*((OMX_U32*) aHeader) != aSize)
    {
        return OMX_ErrorBadParameter;
    }

    if (pVersion->s.nVersionMajor != SPECVERSIONMAJOR ||
            pVersion->s.nVersionMinor != SPECVERSIONMINOR ||
            pVersion->s.nRevision != SPECREVISION ||
            pVersion->s.nStep != SPECSTEP)
    {
        return OMX_ErrorVersionMismatch;
    }

    return OMX_ErrorNone;
}


/**
 * This function verify component state and structure header
 */
OMX_ERRORTYPE OmxComponentBase::ParameterSanityCheck(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_PTR pStructure,
    OMX_IN  size_t size)
{
    OSCL_UNUSED_ARG(hComponent);
    if (iState != OMX_StateLoaded &&
            iState != OMX_StateWaitForResources)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (nPortIndex >= iNumPorts)
    {
        return OMX_ErrorBadPortIndex;
    }

    return CheckHeader(pStructure, size);
}

/**
 * Set/Reset Port Flush Flag
 */
void OmxComponentBase::SetPortFlushFlag(OMX_S32 NumPorts, OMX_S32 index, OMX_BOOL value)
{
    OMX_S32 ii;

    if (-1 == index)
    {
        for (ii = 0; ii < NumPorts; ii++)
        {
            ipPorts[ii]->IsPortFlushed = value;
        }
    }
    else
    {
        ipPorts[index]->IsPortFlushed = value;
    }

}

/**
 * Set Number of Buffer Flushed with the value Specified
 */
void OmxComponentBase::SetNumBufferFlush(OMX_S32 NumPorts, OMX_S32 index, OMX_S32 value)
{
    OMX_S32 ii;

    if (-1 == index)
    {
        // For all ComponentPort
        for (ii = 0; ii < NumPorts; ii++)
        {
            ipPorts[ii]->NumBufferFlushed = value;
        }
    }
    else
    {
        ipPorts[index]->NumBufferFlushed = value;
    }
}


OMX_BOOL OmxComponentBase::ParseFullAVCFramesIntoNALs(OMX_BUFFERHEADERTYPE* aInputBuffer)
{
    OSCL_UNUSED_ARG(aInputBuffer);

    // we should never arrive here if this is not an AVC component, since iOMXComponentUsesFullAVCFrames (which is tested before calling this function)
    // should only be set for an AVC component

    OSCL_ASSERT(OMX_FALSE);

    return OMX_FALSE;
}

/** This function assembles multiple input buffers into
	* one frame with the marker flag OMX_BUFFERFLAG_ENDOFFRAME set
	*/
OMX_BOOL OmxComponentBase::AssemblePartialFrames(OMX_BUFFERHEADERTYPE* aInputBuffer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType* pInPort = ipPorts[OMX_PORT_INPUTPORT_INDEX];
    OMX_U32 BytesToCopy = 0;

    ipInputBuffer = aInputBuffer;

    if (!iPartialFrameAssembly)
    {
        if (iNumInputBuffer > 0)
        {
            if (ipInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME)
            {
                iInputCurrLength = ipInputBuffer->nFilledLen;

                //Only applicable for H.264 component
#ifdef INSERT_NAL_START_CODE
                // this is the case of 1 full NAL in 1 buffer
                // if start codes are inserted, skip the start code
                iInputCurrLength = ipInputBuffer->nFilledLen - 4;
                ipInputBuffer->nOffset += 4;
#endif
                ipFrameDecodeBuffer = ipInputBuffer->pBuffer + ipInputBuffer->nOffset;
                //capture the timestamp to be send to the corresponding output buffer
                iFrameTimestamp = ipInputBuffer->nTimeStamp;
            }
            else
            {
                iInputCurrLength = 0;
                iPartialFrameAssembly = OMX_TRUE;
                iFirstFragment = OMX_TRUE;
                iFrameTimestamp = ipInputBuffer->nTimeStamp;
                ipFrameDecodeBuffer = ipInputCurrBuffer;
            }

        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames ERROR"));
            return OMX_FALSE;
        }

    }

    //Assembling of partial frame will be done based on OMX_BUFFERFLAG_ENDOFFRAME flag marked
    if (iPartialFrameAssembly)
    {
        while (iNumInputBuffer > 0)
        {
            if (OMX_FALSE == iFirstFragment)
            {
                /* If the timestamp of curr fragment doesn't match with previous,
                 * discard the previous fragments & start reconstructing from new
                 */
                if (iFrameTimestamp != ipInputBuffer->nTimeStamp)
                {
                    iInputCurrLength = 0;
                    iPartialFrameAssembly = OMX_TRUE;
                    iFirstFragment = OMX_TRUE;
                    iFrameTimestamp = ipInputBuffer->nTimeStamp;
                    ipFrameDecodeBuffer = ipInputCurrBuffer;

                    //Send a stream corrupt callback
                    OMX_COMPONENTTYPE  *pHandle = &iOmxComponent;
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventError,
                     OMX_ErrorStreamCorrupt,
                     0,
                     NULL);
                }
            }

#ifdef INSERT_NAL_START_CODE
            else
            {
                // this is the case of a partial NAL in 1 buffer
                // this is the first fragment of a nal

                // if start codes are inserted, skip the start code
                ipInputBuffer->nFilledLen -= 4;
                ipInputBuffer->nOffset += 4;
            }
#endif

            if ((ipInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME) != 0)
            {
                break;
            }

            // check if the buffer size can take the new piece, or it needs to expand
            BytesToCopy = ipInputBuffer->nFilledLen;

            if (iInputCurrBufferSize < (iInputCurrLength + BytesToCopy))
            {
                // allocate new partial frame buffer
                OMX_U8* pTempNewBuffer = NULL;
                pTempNewBuffer = (OMX_U8*) oscl_malloc(sizeof(OMX_U8) * (iInputCurrLength + BytesToCopy));

                // in the event that new buffer cannot be allocated
                if (NULL == pTempNewBuffer)
                {
                    // copy into what space is available, and let the decoder complain
                    BytesToCopy = iInputCurrLength - iInputCurrBufferSize;
                }
                else
                {

                    // copy contents of the old buffer into the new one
                    oscl_memcpy(pTempNewBuffer, ipInputCurrBuffer, iInputCurrBufferSize);
                    // free the old buffer
                    if (ipInputCurrBuffer)
                    {
                        oscl_free(ipInputCurrBuffer);
                    }
                    // assign new one
                    ipInputCurrBuffer = pTempNewBuffer;
                    iInputCurrBufferSize = (iInputCurrLength + BytesToCopy);
                    ipFrameDecodeBuffer = ipInputCurrBuffer + iInputCurrLength;
                }
            }

            iInputCurrLength += BytesToCopy;
            oscl_memcpy(ipFrameDecodeBuffer, (ipInputBuffer->pBuffer + ipInputBuffer->nOffset), BytesToCopy); // copy buffer data
            ipFrameDecodeBuffer += BytesToCopy; // move the ptr

            ipInputBuffer->nFilledLen = 0;

            ReturnInputBuffer(ipInputBuffer, pInPort);
            ipInputBuffer = NULL;

            iFirstFragment = OMX_FALSE;

            if (iNumInputBuffer > 0)
            {
                ipInputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pInputQueue);
                if (NULL == ipInputBuffer)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames ERROR DeQueue() returned NULL"));
                    return OMX_FALSE;
                }

                if (ipInputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames EndOfStream arrived"));
                    iEndofStream = OMX_TRUE;
                }
            }
        }

        // if we broke out of the while loop because of lack of buffers, then return and wait for more input buffers
        if (0 == iNumInputBuffer)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames OUT"));
            return OMX_FALSE;
        }
        else
        {
            // we have found the buffer that is the last piece of the frame.
            // Copy the buffer, but do not release it yet (this will be done after decoding for consistency)

            BytesToCopy = ipInputBuffer->nFilledLen;
            // check if the buffer size can take the new piece, or it needs to expand
            if (iInputCurrBufferSize < (iInputCurrLength + BytesToCopy))
            {
                // allocate new partial frame buffer
                OMX_U8* pTempNewBuffer = NULL;
                pTempNewBuffer = (OMX_U8*) oscl_malloc(sizeof(OMX_U8) * (iInputCurrLength + BytesToCopy));

                // if you cannot allocate new buffer, just copy what data you can
                if (NULL == pTempNewBuffer)
                {
                    BytesToCopy = iInputCurrBufferSize - iInputCurrLength;
                }
                else
                {

                    // copy contents of the old one into new one
                    oscl_memcpy(pTempNewBuffer, ipInputCurrBuffer, iInputCurrBufferSize);
                    // free the old buffer
                    if (ipInputCurrBuffer)
                    {
                        oscl_free(ipInputCurrBuffer);
                    }
                    // assign new one
                    ipInputCurrBuffer = pTempNewBuffer;
                    iInputCurrBufferSize = (iInputCurrLength + BytesToCopy);
                    ipFrameDecodeBuffer = ipInputCurrBuffer + iInputCurrLength;
                }
            }

            iInputCurrLength += BytesToCopy;
            oscl_memcpy(ipFrameDecodeBuffer, (ipInputBuffer->pBuffer + ipInputBuffer->nOffset), BytesToCopy); // copy buffer data
            ipFrameDecodeBuffer += BytesToCopy; // move the ptr

            ipFrameDecodeBuffer = ipInputCurrBuffer; // reset the pointer back to beginning of assembly buffer
            iPartialFrameAssembly = OMX_FALSE; // we have finished with assembling the frame, so this is not needed any more
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AssemblePartialFrames OUT"));
    return OMX_TRUE;
}


void OmxComponentBase::ReturnInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuffer, ComponentPortType* pPort)
{
    OSCL_UNUSED_ARG(pPort);
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;

    if (iNumInputBuffer)
    {
        iNumInputBuffer--;
    }

    //Callback for releasing the input buffer
    (*(ipCallbacks->EmptyBufferDone))
    (pHandle, iCallbackData, pInputBuffer);

}

/**
 * Returns Output Buffer back to the IL client
 */
void OmxComponentBase::ReturnOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuffer,
        ComponentPortType *pPort)
{
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;

    //Callback for sending back the output buffer
    (*(ipCallbacks->FillBufferDone))
    (pHandle, iCallbackData, pOutputBuffer);

    if (iOutBufferCount)
    {
        iOutBufferCount--;
    }

    pPort->NumBufferFlushed++;
    iNewOutBufRequired = OMX_TRUE;
}


/** Flushes all the buffers under processing by the given port.
	* This function is called due to a state change of the component, typically
	* @param PortIndex the ID of the port to be flushed
	*/

OMX_ERRORTYPE OmxComponentBase::FlushPort(OMX_S32 PortIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FlushPort IN"));

    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;


    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    OMX_BUFFERHEADERTYPE* pOutputBuff;
    OMX_BUFFERHEADERTYPE* pInputBuff;

    if (OMX_PORT_INPUTPORT_INDEX == PortIndex || OMX_PORT_ALLPORT_INDEX == PortIndex)
    {
        iPartialFrameAssembly = OMX_FALSE;

        //Release all the input buffers in queue
        while ((GetQueueNumElem(pInputQueue) > 0))
        {
            pInputBuff = (OMX_BUFFERHEADERTYPE*) DeQueue(pInputQueue);
            if (NULL == pInputBuff)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FlushPort ERROR DeQueue() returned NULL"));
                return OMX_ErrorUndefined;
            }
            (*(ipCallbacks->EmptyBufferDone))
            (pHandle, iCallbackData, pInputBuff);
            iNumInputBuffer--;
        }

        //Release the current buffer that is being processed by the component.
        if (iNumInputBuffer > 0 && ipInputBuffer && (OMX_FALSE == iIsInputBufferEnded))
        {
            (*(ipCallbacks->EmptyBufferDone))
            (pHandle, iCallbackData, ipInputBuffer);
            iNumInputBuffer--;

            iIsInputBufferEnded = OMX_TRUE;
            iInputCurrLength = 0;
        }
    }

    if (OMX_PORT_OUTPUTPORT_INDEX == PortIndex || OMX_PORT_ALLPORT_INDEX == PortIndex)
    {
        //Release the current output buffer if present that is being processed by the component.
        if ((OMX_FALSE == iNewOutBufRequired) && (iOutBufferCount > 0))
        {
            if (ipOutputBuffer)
            {
                (*(ipCallbacks->FillBufferDone))
                (pHandle, iCallbackData, ipOutputBuffer);
                iOutBufferCount--;
                iNewOutBufRequired = OMX_TRUE;
            }
        }

        //Release all other output buffers in queue
        while ((GetQueueNumElem(pOutputQueue) > 0))
        {
            pOutputBuff = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == pOutputBuff)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FlushPort ERROR DeQueue() returned NULL"));
                return OMX_ErrorUndefined;
            }

            pOutputBuff->nFilledLen = 0;
            (*(ipCallbacks->FillBufferDone))
            (pHandle, iCallbackData, pOutputBuff);
            iOutBufferCount--;
        }

    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FlushPort OUT"));
    return OMX_ErrorNone;
}

/**
 * Disable Single Port
 */
void OmxComponentBase::DisableSinglePort(OMX_U32 PortIndex)
{
    ipPorts[PortIndex]->PortParam.bEnabled = OMX_FALSE;

    if (PORT_IS_POPULATED(ipPorts[PortIndex]) && OMX_TRUE == iIsInit)
    {
        iStateTransitionFlag = OMX_TRUE;
        return;
    }

    ipPorts[PortIndex]->NumBufferFlushed = 0;
}


/** Disables the specified port. This function is called due to a request by the IL client
	* @param PortIndex the ID of the port to be disabled
	*/
OMX_ERRORTYPE OmxComponentBase::DisablePort(OMX_S32 PortIndex)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DisablePort IN"));
    OMX_U32 ii;

    if (-1 == PortIndex)
    {
        for (ii = 0; ii < iNumPorts; ii++)
        {
            ipPorts[ii]->IsPortFlushed = OMX_TRUE;
        }

        /*Flush all ports*/
        FlushPort(PortIndex);

        for (ii = 0; ii < iNumPorts; ii++)
        {
            ipPorts[ii]->IsPortFlushed = OMX_FALSE;
        }
    }
    else
    {
        /*Flush the port specified*/
        ipPorts[PortIndex]->IsPortFlushed = OMX_TRUE;
        FlushPort(PortIndex);
        ipPorts[PortIndex]->IsPortFlushed = OMX_FALSE;
    }

    /*Disable ports*/
    if (PortIndex != -1)
    {
        DisableSinglePort(PortIndex);
    }
    else
    {
        for (ii = 0; ii < iNumPorts; ii++)
        {
            DisableSinglePort(ii);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DisablePort OUT"));

    return OMX_ErrorNone;
}

/**
 * Enable Single Port
 */
void OmxComponentBase::EnableSinglePort(OMX_U32 PortIndex)
{
    ipPorts[PortIndex]->PortParam.bEnabled = OMX_TRUE;

    if (!PORT_IS_POPULATED(ipPorts[PortIndex]) && OMX_TRUE == iIsInit)
    {
        iStateTransitionFlag = OMX_TRUE;
        return;
    }
}

/** Enables the specified port. This function is called due to a request by the IL client
	* @param PortIndex the ID of the port to be enabled
	*/
OMX_ERRORTYPE OmxComponentBase::EnablePort(OMX_S32 PortIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EnablePort IN"));

    OMX_U32 ii;

    /*Enable port/s*/
    if (PortIndex != -1)
    {
        EnableSinglePort(PortIndex);
    }
    else
    {
        for (ii = 0; ii < iNumPorts; ii++)
        {
            EnableSinglePort(ii);
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EnablePort OUT"));
    return OMX_ErrorNone;
}

//Not implemented & supported in case of base profile components

OMX_ERRORTYPE OmxComponentBase::TunnelRequest(
    OMX_IN  OMX_HANDLETYPE hComp,
    OMX_IN  OMX_U32 nPort,
    OMX_IN  OMX_HANDLETYPE hTunneledComp,
    OMX_IN  OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OSCL_UNUSED_ARG(hComp);
    OSCL_UNUSED_ARG(nPort);
    OSCL_UNUSED_ARG(hTunneledComp);
    OSCL_UNUSED_ARG(nTunneledPort);
    OSCL_UNUSED_ARG(pTunnelSetup);

    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentGetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->GetConfig(hComponent, nIndex, pComponentConfigStructure);
    return Status;
}



OMX_ERRORTYPE OmxComponentBase::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig IN"));

    OMX_U32 PortIndex;
    OMX_CONFIG_INTRAREFRESHVOPTYPE* pVideoIFrame;
    OMX_CONFIG_FRAMERATETYPE* pFrameRateType;
    OMX_VIDEO_CONFIG_BITRATETYPE* pConfigBitRateType;

    if (NULL == pComponentConfigStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nIndex)
    {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            pVideoIFrame = (OMX_CONFIG_INTRAREFRESHVOPTYPE*) pComponentConfigStructure;
            if (pVideoIFrame->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig error bad port index for OMX_IndexConfigVideoIntraVOPRefresh"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoIFrame->nPortIndex;
            oscl_memcpy(pVideoIFrame, &ipPorts[PortIndex]->VideoIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
            SetHeader(pVideoIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
        }
        break;

        case OMX_IndexConfigVideoFramerate:
        {
            pFrameRateType = (OMX_CONFIG_FRAMERATETYPE*) pComponentConfigStructure;
            if (pFrameRateType->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig error bad port index for OMX_IndexConfigVideoFramerate"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pFrameRateType->nPortIndex;
            oscl_memcpy(pFrameRateType, &ipPorts[PortIndex]->VideoConfigFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
            SetHeader(pFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
        }
        break;

        case OMX_IndexConfigVideoBitrate:
        {
            pConfigBitRateType = (OMX_VIDEO_CONFIG_BITRATETYPE*) pComponentConfigStructure;
            if (pConfigBitRateType->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig error bad port index for OMX_IndexConfigVideoBitrate"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pConfigBitRateType->nPortIndex;
            oscl_memcpy(pConfigBitRateType, &ipPorts[PortIndex]->VideoConfigBitRateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
            SetHeader(pConfigBitRateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
        }
        break;
        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetParameter error Unsupported Index"));
            return OMX_ErrorUnsupportedIndex;
        }

    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : GetConfig OUT"));

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentSetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->SetConfig(hComponent, nIndex, pComponentConfigStructure);
    return Status;
}


OMX_ERRORTYPE OmxComponentBase::SetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(nIndex);
    OSCL_UNUSED_ARG(pComponentConfigStructure);

    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentGetExtensionIndex(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(cParameterName);
    OSCL_UNUSED_ARG(pIndexType);

    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentGetState(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE* pState)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;

    pOpenmaxAOType->GetState(pState);

    return OMX_ErrorNone;
}


void OmxComponentBase::GetState(OMX_OUT OMX_STATETYPE* pState)
{
    *pState = iState;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentGetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    return Status;

}

OMX_ERRORTYPE OmxComponentBase::BaseComponentSetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->SetParameter(hComponent, nParamIndex, ComponentParameterStructure);

    return Status;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentUseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);

    return Status;
}


OMX_ERRORTYPE OmxComponentBase::UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OSCL_UNUSED_ARG(hComponent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer IN"));
    ComponentPortType* pBaseComponentPort;
    OMX_U32 ii;

    if (nPortIndex >= iNumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer error bad port index"));
        return OMX_ErrorBadPortIndex;
    }

    pBaseComponentPort = ipPorts[nPortIndex];

    if (pBaseComponentPort->TransientState != OMX_StateIdle)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer error incorrect state"));
        return OMX_ErrorIncorrectStateTransition;
    }

    if (NULL == pBaseComponentPort->pBuffer)
    {
        pBaseComponentPort->pBuffer = (OMX_BUFFERHEADERTYPE**) oscl_calloc(pBaseComponentPort->PortParam.nBufferCountActual, sizeof(OMX_BUFFERHEADERTYPE*));
        if (NULL == pBaseComponentPort->pBuffer)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer error insufficient resources"));
            return OMX_ErrorInsufficientResources;
        }

        pBaseComponentPort->BufferState = (OMX_U32*) oscl_calloc(pBaseComponentPort->PortParam.nBufferCountActual, sizeof(OMX_U32));
        if (NULL == pBaseComponentPort->BufferState)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer error insufficient resources"));
            return OMX_ErrorInsufficientResources;
        }
    }

    for (ii = 0; ii < pBaseComponentPort->PortParam.nBufferCountActual; ii++)
    {
        if (!(pBaseComponentPort->BufferState[ii] & BUFFER_ALLOCATED) &&
                !(pBaseComponentPort->BufferState[ii] & BUFFER_ASSIGNED))
        {
            pBaseComponentPort->pBuffer[ii] = (OMX_BUFFERHEADERTYPE*) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (NULL == pBaseComponentPort->pBuffer[ii])
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }
            SetHeader(pBaseComponentPort->pBuffer[ii], sizeof(OMX_BUFFERHEADERTYPE));
            pBaseComponentPort->pBuffer[ii]->pBuffer = pBuffer;
            pBaseComponentPort->pBuffer[ii]->nAllocLen = nSizeBytes;
            pBaseComponentPort->pBuffer[ii]->nFilledLen = 0;
            pBaseComponentPort->pBuffer[ii]->nOffset = 0;
            pBaseComponentPort->pBuffer[ii]->nFlags = 0;
            pBaseComponentPort->pBuffer[ii]->pPlatformPrivate = pBaseComponentPort;
            pBaseComponentPort->pBuffer[ii]->pAppPrivate = pAppPrivate;
            pBaseComponentPort->pBuffer[ii]->nTickCount = 0;
            pBaseComponentPort->pBuffer[ii]->nTimeStamp = 0;
            *ppBufferHdr = pBaseComponentPort->pBuffer[ii];
            if (OMX_DirInput == pBaseComponentPort->PortParam.eDir)
            {
                pBaseComponentPort->pBuffer[ii]->nInputPortIndex = nPortIndex;
                pBaseComponentPort->pBuffer[ii]->nOutputPortIndex = iNumPorts; // here is assigned a non-valid port index
            }
            else
            {
                pBaseComponentPort->pBuffer[ii]->nOutputPortIndex = nPortIndex;
                pBaseComponentPort->pBuffer[ii]->nInputPortIndex = iNumPorts; // here is assigned a non-valid port index
            }
            pBaseComponentPort->BufferState[ii] |= BUFFER_ASSIGNED;
            pBaseComponentPort->BufferState[ii] |= HEADER_ALLOCATED;
            pBaseComponentPort->NumAssignedBuffers++;
            if (pBaseComponentPort->PortParam.nBufferCountActual == pBaseComponentPort->NumAssignedBuffers)
            {
                pBaseComponentPort->PortParam.bPopulated = OMX_TRUE;

                if (OMX_TRUE == iStateTransitionFlag)
                {
                    //Reschedule the AO for a state change (Loaded->Idle) if its pending on buffer allocation
                    RunIfNotReady();
                    iStateTransitionFlag = OMX_FALSE;
                }
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer OUT"));
            return OMX_ErrorNone;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : UseBuffer OUT"));
    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentAllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->AllocateBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);

    return Status;
}


OMX_ERRORTYPE OmxComponentBase::AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OSCL_UNUSED_ARG(hComponent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer IN"));

    ComponentPortType* pBaseComponentPort;
    OMX_U32 ii;

    if (nPortIndex >= iNumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error bad port index"));
        return OMX_ErrorBadPortIndex;
    }

    pBaseComponentPort = ipPorts[nPortIndex];

    if (pBaseComponentPort->TransientState != OMX_StateIdle)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error incorrect state"));
        return OMX_ErrorIncorrectStateTransition;
    }

    if (NULL == pBaseComponentPort->pBuffer)
    {
        pBaseComponentPort->pBuffer = (OMX_BUFFERHEADERTYPE**) oscl_calloc(pBaseComponentPort->PortParam.nBufferCountActual, sizeof(OMX_BUFFERHEADERTYPE*));
        if (NULL == pBaseComponentPort->pBuffer)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error insufficient resources"));
            return OMX_ErrorInsufficientResources;
        }

        pBaseComponentPort->BufferState = (OMX_U32*) oscl_calloc(pBaseComponentPort->PortParam.nBufferCountActual, sizeof(OMX_U32));
        if (NULL == pBaseComponentPort->BufferState)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error insufficient resources"));
            return OMX_ErrorInsufficientResources;
        }
    }

    for (ii = 0; ii < pBaseComponentPort->PortParam.nBufferCountActual; ii++)
    {
        if (!(pBaseComponentPort->BufferState[ii] & BUFFER_ALLOCATED) &&
                !(pBaseComponentPort->BufferState[ii] & BUFFER_ASSIGNED))
        {
            pBaseComponentPort->pBuffer[ii] = (OMX_BUFFERHEADERTYPE*) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (NULL == pBaseComponentPort->pBuffer[ii])
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }
            SetHeader(pBaseComponentPort->pBuffer[ii], sizeof(OMX_BUFFERHEADERTYPE));
            /* allocate the buffer */
            pBaseComponentPort->pBuffer[ii]->pBuffer = (OMX_BYTE) oscl_malloc(nSizeBytes);
            if (NULL == pBaseComponentPort->pBuffer[ii]->pBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }
            pBaseComponentPort->pBuffer[ii]->nAllocLen = nSizeBytes;
            pBaseComponentPort->pBuffer[ii]->nFlags = 0;
            pBaseComponentPort->pBuffer[ii]->pPlatformPrivate = pBaseComponentPort;
            pBaseComponentPort->pBuffer[ii]->pAppPrivate = pAppPrivate;
            *pBuffer = pBaseComponentPort->pBuffer[ii];
            pBaseComponentPort->BufferState[ii] |= BUFFER_ALLOCATED;
            pBaseComponentPort->BufferState[ii] |= HEADER_ALLOCATED;

            if (OMX_DirInput == pBaseComponentPort->PortParam.eDir)
            {
                pBaseComponentPort->pBuffer[ii]->nInputPortIndex = nPortIndex;
                // here is assigned a non-valid port index
                pBaseComponentPort->pBuffer[ii]->nOutputPortIndex = iNumPorts;
            }
            else
            {
                // here is assigned a non-valid port index
                pBaseComponentPort->pBuffer[ii]->nInputPortIndex = iNumPorts;
                pBaseComponentPort->pBuffer[ii]->nOutputPortIndex = nPortIndex;
            }

            pBaseComponentPort->NumAssignedBuffers++;

            if (pBaseComponentPort->PortParam.nBufferCountActual == pBaseComponentPort->NumAssignedBuffers)
            {
                pBaseComponentPort->PortParam.bPopulated = OMX_TRUE;

                if (OMX_TRUE == iStateTransitionFlag)
                {
                    //Reschedule the AO for a state change (Loaded->Idle) if its pending on buffer allocation
                    RunIfNotReady();
                    iStateTransitionFlag = OMX_FALSE;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer OUT"));
            return OMX_ErrorNone;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : AllocateBuffer OUT"));
    return OMX_ErrorInsufficientResources;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentFreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->FreeBuffer(hComponent, nPortIndex, pBuffer);

    return Status;
}


OMX_ERRORTYPE OmxComponentBase::FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FreeBuffer IN"));

    ComponentPortType* pBaseComponentPort;

    OMX_U32 ii;
    OMX_BOOL FoundBuffer;

    if (nPortIndex >= iNumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FreeBuffer error bad port index"));
        return OMX_ErrorBadPortIndex;
    }

    pBaseComponentPort = ipPorts[nPortIndex];

    if (pBaseComponentPort->TransientState != OMX_StateLoaded
            && pBaseComponentPort->TransientState != OMX_StateInvalid)
    {

        (*(ipCallbacks->EventHandler))
        (hComponent,
         iCallbackData,
         OMX_EventError, /* The command was completed */
         OMX_ErrorPortUnpopulated, /* The commands was a OMX_CommandStateSet */
         nPortIndex, /* The State has been changed in message->MessageParam2 */
         NULL);
    }

    for (ii = 0; ii < pBaseComponentPort->PortParam.nBufferCountActual; ii++)
    {
        if ((pBaseComponentPort->BufferState[ii] & BUFFER_ALLOCATED) &&
                (pBaseComponentPort->pBuffer[ii]->pBuffer == pBuffer->pBuffer))
        {

            pBaseComponentPort->NumAssignedBuffers--;
            oscl_free(pBuffer->pBuffer);
            pBuffer->pBuffer = NULL;

            if (pBaseComponentPort->BufferState[ii] & HEADER_ALLOCATED)
            {
                oscl_free(pBuffer);
                pBuffer = NULL;
            }
            pBaseComponentPort->BufferState[ii] = BUFFER_FREE;
            break;
        }
        else if ((pBaseComponentPort->BufferState[ii] & BUFFER_ASSIGNED) &&
                 (pBaseComponentPort->pBuffer[ii] == pBuffer))
        {

            pBaseComponentPort->NumAssignedBuffers--;

            if (pBaseComponentPort->BufferState[ii] & HEADER_ALLOCATED)
            {
                oscl_free(pBuffer);
                pBuffer = NULL;
            }

            pBaseComponentPort->BufferState[ii] = BUFFER_FREE;
            break;
        }
    }

    FoundBuffer = OMX_FALSE;

    for (ii = 0; ii < pBaseComponentPort->PortParam.nBufferCountActual; ii++)
    {
        if (pBaseComponentPort->BufferState[ii] != BUFFER_FREE)
        {
            FoundBuffer = OMX_TRUE;
            break;
        }
    }
    if (!FoundBuffer)
    {
        pBaseComponentPort->PortParam.bPopulated = OMX_FALSE;

        if (OMX_TRUE == iStateTransitionFlag)
        {
            //Reschedule the AO for a state change (Idle->Loaded) if its pending on buffer de-allocation
            RunIfNotReady();
            iStateTransitionFlag = OMX_FALSE;

            //Reset the decoding flags while freeing buffers
            if (OMX_PORT_INPUTPORT_INDEX == nPortIndex)
            {
                iIsInputBufferEnded = OMX_TRUE;
                iTempInputBufferLength = 0;
                iTempConsumedLength = 0;
                iNewInBufferRequired = OMX_TRUE;
            }
            else if (OMX_PORT_OUTPUTPORT_INDEX == nPortIndex)
            {
                iNewOutBufRequired = OMX_TRUE;
            }
        }

        if (NULL != pBaseComponentPort->pBuffer)
        {
            oscl_free(pBaseComponentPort->pBuffer);
            pBaseComponentPort->pBuffer = NULL;
            oscl_free(pBaseComponentPort->BufferState);
            pBaseComponentPort->BufferState = NULL;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FreeBuffer OUT"));
    return OMX_ErrorNone;
}


/** Set Callbacks. It stores in the component private structure the pointers to the user application callbacs
	* @param hComponent the handle of the component
	* @param ipCallbacks the OpenMAX standard structure that holds the callback pointers
	* @param pAppData a pointer to a private structure, not covered by OpenMAX standard, in needed
    */

OMX_ERRORTYPE OmxComponentBase::BaseComponentSetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->SetCallbacks(hComponent, pCallbacks, pAppData);

    return Status;
}


OMX_ERRORTYPE OmxComponentBase::SetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    OSCL_UNUSED_ARG(hComponent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SetCallbacks"));
    ipCallbacks = pCallbacks;
    iCallbackData = pAppData;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentSendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam,
    OMX_IN  OMX_PTR pCmdData)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->SendCommand(hComponent, Cmd, nParam, pCmdData);

    return Status;
}

OMX_ERRORTYPE OmxComponentBase::SendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_S32 nParam,
    OMX_IN  OMX_PTR pCmdData)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand IN"));

    OMX_U32 ii;
    OMX_ERRORTYPE ErrMsgHandler = OMX_ErrorNone;
    QueueType* pMessageQueue;
    CoreMessage* Message = NULL;
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;

    pMessageQueue = ipCoreDescriptor->pMessageQueue;

    if (OMX_StateInvalid == iState)
    {
        ErrMsgHandler = OMX_ErrorInvalidState;
    }

    switch (Cmd)
    {
        case OMX_CommandStateSet:
        {
            Message = (CoreMessage*) oscl_malloc(sizeof(CoreMessage));

            if (NULL == Message)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }

            Message->pComponent = (OMX_COMPONENTTYPE *) hComponent;
            Message->MessageType = SENDCOMMAND_MSG_TYPE;
            Message->MessageParam1 = OMX_CommandStateSet;
            Message->MessageParam2 = nParam;
            Message->pCmdData = pCmdData;

            if ((OMX_StateIdle == nParam) && (OMX_StateLoaded == iState))
            {
                ErrMsgHandler = pOpenmaxAOType->ComponentInit();

                if (OMX_ErrorNone != ErrMsgHandler)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error component init"));
                    return OMX_ErrorInsufficientResources;
                }
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    ipPorts[ii]->TransientState = OMX_StateIdle;
                }
            }
            else if ((OMX_StateLoaded == nParam) && (OMX_StateIdle == iState))
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (PORT_IS_ENABLED(ipPorts[ii]))
                    {
                        ipPorts[ii]->TransientState = OMX_StateLoaded;
                    }
                }
            }
            else if (OMX_StateInvalid == nParam)
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (PORT_IS_ENABLED(ipPorts[ii]))
                    {
                        ipPorts[ii]->TransientState = OMX_StateInvalid;
                    }
                }
            }
            else if (((OMX_StateIdle == nParam) || (OMX_StatePause == nParam))
                     && (OMX_StateExecuting == iState))
            {
                iBufferExecuteFlag = OMX_FALSE;
            }

        }
        break;

        case OMX_CommandFlush:
        {
            Message = (CoreMessage*) oscl_malloc(sizeof(CoreMessage));

            if (NULL == Message)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }

            Message->pComponent = (OMX_COMPONENTTYPE *) hComponent;
            Message->MessageType = SENDCOMMAND_MSG_TYPE;
            Message->MessageParam1 = OMX_CommandFlush;
            Message->MessageParam2 = nParam;
            Message->pCmdData = pCmdData;

            if ((iState != OMX_StateExecuting) && (iState != OMX_StatePause))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                break;

            }
            if ((nParam != -1) && ((OMX_U32) nParam >= iNumPorts))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            SetPortFlushFlag(iNumPorts, nParam, OMX_TRUE);
            SetNumBufferFlush(iNumPorts, -1, 0);
        }
        break;

        case OMX_CommandPortDisable:
        {
            if ((nParam != -1) && ((OMX_U32) nParam >= iNumPorts))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error bad port index"));
                return OMX_ErrorBadPortIndex;
            }

            iResizePending = OMX_FALSE; // reset the flag to enable processing
            iSendOutBufferAfterPortReconfigFlag = OMX_TRUE;


            if (-1 == nParam)
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (!PORT_IS_ENABLED(ipPorts[ii]))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                        ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                        break;
                    }
                    else
                    {
                        ipPorts[ii]->TransientState = OMX_StateLoaded;
                    }
                }
            }
            else
            {
                if (!PORT_IS_ENABLED(ipPorts[nParam]))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                    ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                    break;
                }
                else
                {
                    ipPorts[nParam]->TransientState = OMX_StateLoaded;
                }
            }

            Message = (CoreMessage*) oscl_malloc(sizeof(CoreMessage));
            if (NULL == Message)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }

            Message->pComponent = (OMX_COMPONENTTYPE *) hComponent;
            if (OMX_ErrorNone == ErrMsgHandler)
            {
                Message->MessageType = SENDCOMMAND_MSG_TYPE;
                Message->MessageParam2 = nParam;
            }
            else
            {
                Message->MessageType = ERROR_MSG_TYPE;
                Message->MessageParam2 = ErrMsgHandler;
            }
            Message->MessageParam1 = OMX_CommandPortDisable;
            Message->pCmdData = pCmdData;
        }
        break;


        case OMX_CommandPortEnable:
        {
            if ((nParam != -1) && ((OMX_U32) nParam >= iNumPorts))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error bad port index"));
                return OMX_ErrorBadPortIndex;
            }

            if (-1 == nParam)
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (PORT_IS_ENABLED(ipPorts[ii]))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                        ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                        break;
                    }
                    else
                    {
                        ipPorts[ii]->TransientState = OMX_StateIdle;
                    }
                }
            }
            else
            {
                if (PORT_IS_ENABLED(ipPorts[nParam]))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                    ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                    break;
                }
                else
                {
                    ipPorts[nParam]->TransientState = OMX_StateIdle;
                }
            }

            Message = (CoreMessage*) oscl_malloc(sizeof(CoreMessage));
            if (NULL == Message)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }

            Message->pComponent = (OMX_COMPONENTTYPE *) hComponent;
            if (OMX_ErrorNone == ErrMsgHandler)
            {
                Message->MessageType = SENDCOMMAND_MSG_TYPE;
            }
            else
            {
                Message->MessageType = ERROR_MSG_TYPE;
            }

            Message->MessageParam1 = OMX_CommandPortEnable;
            Message->MessageParam2 = nParam;
            Message->pCmdData = pCmdData;
        }
        break;


        case OMX_CommandMarkBuffer:
        {
            if ((iState != OMX_StateExecuting) && (iState != OMX_StatePause))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error incorrect state"));
                ErrMsgHandler = OMX_ErrorIncorrectStateOperation;
                break;
            }

            if ((nParam != -1) && ((OMX_U32) nParam >= iNumPorts))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error bad port index"));
                return OMX_ErrorBadPortIndex;
            }

            Message = (CoreMessage*) oscl_malloc(sizeof(CoreMessage));
            if (NULL == Message)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error insufficient resources"));
                return OMX_ErrorInsufficientResources;
            }
            Message->pComponent = (OMX_COMPONENTTYPE *) hComponent;
            Message->MessageType = SENDCOMMAND_MSG_TYPE;
            Message->MessageParam1 = OMX_CommandMarkBuffer;
            Message->MessageParam2 = nParam;
            Message->pCmdData = pCmdData;
        }
        break;


        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error unsupported index"));
            ErrMsgHandler = OMX_ErrorUnsupportedIndex;
        }
        break;
    }

    if (OMX_ErrorNone != Queue(pMessageQueue, Message))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand error, Queuing command failed"));
        return OMX_ErrorInsufficientResources;
    }

    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : SendCommand OUT"));
    return ErrMsgHandler;
}



/* This routine will reset all the buffers and flag associated with decoding
 * when receiving a flush command on the respective port*/
void OmxComponentBase::ResetAfterFlush(OMX_S32 PortIndex)
{

    if (OMX_PORT_INPUTPORT_INDEX == PortIndex || OMX_PORT_ALLPORT_INDEX == PortIndex)
    {
        iIsInputBufferEnded = OMX_TRUE;
        iEndofStream = OMX_FALSE;
        iNewInBufferRequired = OMX_TRUE;
        iPartialFrameAssembly = OMX_FALSE;
        iTempInputBufferLength = 0;
        iTempConsumedLength = 0;
        iInputBufferRemainingBytes = 0;
        iInputCurrLength = 0;

        //Assume for this state transition that reposition command has come
        iRepositionFlag = OMX_TRUE;
        //Reset the silence insertion logic also
        iSilenceInsertionInProgress = OMX_FALSE;
    }

    if (OMX_PORT_OUTPUTPORT_INDEX == PortIndex || OMX_PORT_ALLPORT_INDEX == PortIndex)
    {
        iNewOutBufRequired = OMX_TRUE;
    }

    return;
}


/** This is called by the OMX core in its message processing
 * thread context upon a component request. A request is made
 * by the component when some asynchronous services are needed:
 * 1) A SendCommand() is to be processed
 * 2) An error needs to be notified
 * \param Message, the message that has been passed to core
 */

OMX_ERRORTYPE OmxComponentBase::MessageHandler(CoreMessage* Message)
{

    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*) iOmxComponent.pComponentPrivate;
    OMX_U32 ii;
    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;


    /** Dealing with a SendCommand call.
     * -MessageParam1 contains the command to execute
     * -MessageParam2 contains the parameter of the command
     *  (destination state in case of a state change command).
     */

    OMX_STATETYPE orig_state = iState;
    if (SENDCOMMAND_MSG_TYPE == Message->MessageType)
    {
        switch (Message->MessageParam1)
        {
            case OMX_CommandStateSet:
            {
                /* Do the actual state change */
                ErrorType = DoStateSet(Message->MessageParam2);

                if (OMX_TRUE == iStateTransitionFlag)
                {
                    return OMX_ErrorNone;
                }

                //Do not send the callback now till the State gets changed
                if (ErrorType != OMX_ErrorNone)
                {
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventError, /* The command was completed */
                     ErrorType, /* The commands was a OMX_CommandStateSet */
                     0, /* The iState has been changed in Message->MessageParam2 */
                     NULL);
                }
                else
                {
                    /* And run the callback */
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventCmdComplete, /* The command was completed */
                     OMX_CommandStateSet, /* The commands was a OMX_CommandStateSet */
                     Message->MessageParam2, /* The iState has been changed in Message->MessageParam2 */
                     NULL);
                }
            }
            break;

            case OMX_CommandFlush:
            {
                /*Flush ports*/
                ErrorType = FlushPort(Message->MessageParam2);

                SetNumBufferFlush(iNumPorts, -1, 0);

                ResetAfterFlush(Message->MessageParam2);

                //If Flush Command has come at the input port, reset the individual component as well
                if (OMX_PORT_INPUTPORT_INDEX == Message->MessageParam2
                        || OMX_PORT_ALLPORT_INDEX == Message->MessageParam2)
                {
                    /* Component specific flush routine for input buffer where individual components
                     * may set/reset some flags/buffer lengths if required.*/

                    pOpenmaxAOType->ResetComponent();
                }

                if (ErrorType != OMX_ErrorNone)
                {
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventError, /* The command was completed */
                     ErrorType, /* The commands was a OMX_CommandStateSet */
                     0, /* The iState has been changed in Message->MessageParam2 */
                     NULL);
                }
                else
                {
                    if (-1 == Message->MessageParam2)
                    {
                        /*Flush all port*/
                        for (ii = 0; ii < iNumPorts; ii++)
                        {
                            (*(ipCallbacks->EventHandler))
                            (pHandle,
                             iCallbackData,
                             OMX_EventCmdComplete, /* The command was completed */
                             OMX_CommandFlush, /* The commands was a OMX_CommandStateSet */
                             ii, /* The iState has been changed in Message->MessageParam2 */
                             NULL);
                        }
                    }
                    else
                    {
                        /*Flush input/output port*/
                        (*(ipCallbacks->EventHandler))
                        (pHandle,
                         iCallbackData,
                         OMX_EventCmdComplete, /* The command was completed */
                         OMX_CommandFlush, /* The commands was a OMX_CommandStateSet */
                         Message->MessageParam2, /* The iState has been changed in Message->MessageParam2 */
                         NULL);
                    }
                }
                SetPortFlushFlag(iNumPorts, -1, OMX_FALSE);
            }
            break;

            case OMX_CommandPortDisable:
            {
                /** This condition is added to pass the tests, it is not significant for the environment */
                ErrorType = DisablePort(Message->MessageParam2);
                if (OMX_TRUE == iStateTransitionFlag)
                {
                    return OMX_ErrorNone;
                }

                if (ErrorType != OMX_ErrorNone)
                {
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventError, /* The command was completed */
                     ErrorType, /* The commands was a OMX_CommandStateSet */
                     0, /* The iState has been changed in Message->MessageParam2 */
                     NULL);
                }
                else
                {
                    if (-1 == Message->MessageParam2)
                    {
                        /*Disable all ports*/
                        for (ii = 0; ii < iNumPorts; ii++)
                        {
                            (*(ipCallbacks->EventHandler))
                            (pHandle,
                             iCallbackData,
                             OMX_EventCmdComplete, /* The command was completed */
                             OMX_CommandPortDisable, /* The commands was a OMX_CommandStateSet */
                             ii, /* The iState has been changed in Message->MessageParam2 */
                             NULL);
                        }
                    }
                    else
                    {
                        (*(ipCallbacks->EventHandler))
                        (pHandle,
                         iCallbackData,
                         OMX_EventCmdComplete, /* The command was completed */
                         OMX_CommandPortDisable, /* The commands was a OMX_CommandStateSet */
                         Message->MessageParam2, /* The iState has been changed in Message->MessageParam2 */
                         NULL);
                    }
                }
            }
            break;

            case OMX_CommandPortEnable:
            {
                ErrorType = EnablePort(Message->MessageParam2);
                if (OMX_TRUE == iStateTransitionFlag)
                {
                    return OMX_ErrorNone;
                }

                if (ErrorType != OMX_ErrorNone)
                {
                    (*(ipCallbacks->EventHandler))
                    (pHandle,
                     iCallbackData,
                     OMX_EventError, /* The command was completed */
                     ErrorType, /* The commands was a OMX_CommandStateSet */
                     0, /* The State has been changed in Message->MessageParam2 */
                     NULL);
                }
                else
                {
                    if (Message->MessageParam2 != -1)
                    {
                        (*(ipCallbacks->EventHandler))
                        (pHandle,
                         iCallbackData,
                         OMX_EventCmdComplete, /* The command was completed */
                         OMX_CommandPortEnable, /* The commands was a OMX_CommandStateSet */
                         Message->MessageParam2, /* The State has been changed in Message->MessageParam2 */
                         NULL);
                    }
                    else
                    {
                        for (ii = 0; ii < iNumPorts; ii++)
                        {
                            (*(ipCallbacks->EventHandler))
                            (pHandle,
                             iCallbackData,
                             OMX_EventCmdComplete, /* The command was completed */
                             OMX_CommandPortEnable, /* The commands was a OMX_CommandStateSet */
                             ii, /* The State has been changed in Message->MessageParam2 */
                             NULL);
                        }
                    }
                }
            }
            break;

            case OMX_CommandMarkBuffer:
            {
                ipMark = (OMX_MARKTYPE *)Message->pCmdData;
            }
            break;

            default:
            {

            }
            break;
        }
        /* Dealing with an asynchronous error condition
         */
    }

    if (orig_state != OMX_StateInvalid)
    {
        ErrorType = OMX_ErrorNone;
    }

    return ErrorType;
}

/** Changes the state of a component taking proper actions depending on
 * the transiotion requested
 * \param aDestinationState the requested target state.
 */

OMX_ERRORTYPE OmxComponentBase::DoStateSet(OMX_U32 aDestinationState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "OmxComponentBase : DoStateSet IN : iState (%i) aDestinationState (%i)", iState, aDestinationState));

    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    OMX_U32 ii;

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*) iOmxComponent.pComponentPrivate;

    if (OMX_StateLoaded == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            case OMX_StateWaitForResources:
            {
                iState = OMX_StateLoaded;
            }
            break;

            case OMX_StateLoaded:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error same state"));
                return OMX_ErrorSameState;
            }

            case OMX_StateIdle:
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (PORT_IS_ENABLED(ipPorts[ii]) && PORT_IS_POPULATED(ipPorts[ii]))
                    {
                        iStateTransitionFlag = OMX_TRUE;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet Waiting port to be de-populated"));
                        return OMX_ErrorNone;
                    }
                }

                iState = OMX_StateLoaded;

                iNumInputBuffer = 0;
                iOutBufferCount = 0;
                iPartialFrameAssembly = OMX_FALSE;
                iEndofStream = OMX_FALSE;
                iIsInputBufferEnded = OMX_TRUE;
                iNewOutBufRequired = OMX_TRUE;
                iNewInBufferRequired = OMX_TRUE;
                iFirstFragment = OMX_FALSE;

                pOpenmaxAOType->ComponentDeInit();
            }
            break;

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error incorrect state"));
                return OMX_ErrorIncorrectStateTransition;
            }
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
        return OMX_ErrorNone;
    }

    if (OMX_StateWaitForResources == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            case OMX_StateLoaded:
            {
                iState = OMX_StateWaitForResources;
            }
            break;

            case OMX_StateWaitForResources:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error same state"));
                return OMX_ErrorSameState;
            }

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error incorrect state"));
                return OMX_ErrorIncorrectStateTransition;
            }
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
        return OMX_ErrorNone;
    }

    if (OMX_StateIdle == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            case OMX_StateWaitForResources:
            {
                iState = OMX_StateIdle;
            }
            break;

            case OMX_StateLoaded:
            {
                for (ii = 0; ii < iNumPorts; ii++)
                {
                    if (PORT_IS_ENABLED(ipPorts[ii]) && !PORT_IS_POPULATED(ipPorts[ii]))
                    {
                        iStateTransitionFlag = OMX_TRUE;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet Waiting port to be populated"));
                        return OMX_ErrorNone;
                    }
                }

                iState = OMX_StateIdle;

                //Used in case of partial frame assembly
                if (!ipInputCurrBuffer)
                {
                    //Keep the size of temp buffer double to be on safer side
                    iInputCurrBufferSize = 2 * sizeof(uint8) * (ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize);

                    ipInputCurrBuffer = (OMX_U8*) oscl_malloc(iInputCurrBufferSize);
                    if (NULL == ipInputCurrBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error insufficient resources"));
                        return OMX_ErrorInsufficientResources;
                    }

                }

                //Used when the buffers are not marked with EndOfFrame flag
                if (!ipTempInputBuffer)
                {
                    ipTempInputBuffer = (OMX_U8*) oscl_malloc(iInputCurrBufferSize);
                    if (NULL == ipTempInputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error insufficient resources"));
                        return OMX_ErrorInsufficientResources;
                    }
                }

                iTempInputBufferLength = 0;
                iTempConsumedLength = 0;
                iInputBufferRemainingBytes = 0;
            }
            break;

            case OMX_StateIdle:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error same state"));
                return OMX_ErrorSameState;
            }

            //Both the below cases have same body
            case OMX_StateExecuting:
            case OMX_StatePause:
            {
                SetNumBufferFlush(iNumPorts, -1, 0);
                SetPortFlushFlag(iNumPorts, -1, OMX_TRUE);

                ComponentPortType* pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];

                //Return all the buffers if still occupied
                QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;

                while ((iNumInputBuffer > 0) && (GetQueueNumElem(pInputQueue) > 0))
                {
                    FlushPort(OMX_PORT_INPUTPORT_INDEX);
                }

                // if a buffer was previously dequeued, it wasnt freed in above loop. return it now
                if (iNumInputBuffer > 0)
                {
                    ipInputBuffer->nFilledLen = 0;
                    ReturnInputBuffer(ipInputBuffer, pInPort);
                    iNewInBufferRequired = OMX_TRUE;
                    iIsInputBufferEnded = OMX_TRUE;
                    iInputCurrLength = 0;
                    ipInputBuffer = NULL;
                }

                //Return all the buffers if still occupied
                while ((iNumInputBuffer > 0))
                {
                    FlushPort(OMX_PORT_INPUTPORT_INDEX);
                }

                //Return all the output buffers if still occupied
                while (iOutBufferCount > 0)
                {
                    FlushPort(OMX_PORT_OUTPUTPORT_INDEX);
                }

                //Call the reset funstion here to reset the flags and buffer length variables
                ResetAfterFlush(OMX_PORT_ALLPORT_INDEX);


                SetPortFlushFlag(iNumPorts, -1, OMX_FALSE);
                SetNumBufferFlush(iNumPorts, -1, 0);

                pOpenmaxAOType->ResetComponent();

                iState = OMX_StateIdle;
            }
            break;

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error incorrect state"));
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
        return ErrorType;
    }

    if (OMX_StatePause == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            case OMX_StatePause:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error same state"));
                return OMX_ErrorSameState;
            }

            //Falling through to the next case
            case OMX_StateExecuting:
            case OMX_StateIdle:
            {
                iState = OMX_StatePause;
            }
            break;

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error incorrect state"));
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
        return OMX_ErrorNone;
    }

    if (OMX_StateExecuting == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            case OMX_StateIdle:
            {
                iState = OMX_StateExecuting;
            }
            break;

            case OMX_StatePause:
            {
                iState = OMX_StateExecuting;
                /* A trigger to start the processing of buffers when component
                 * transitions to executing from pause, as it is already
                 * holding the required buffers
                 */
                RunIfNotReady();
            }
            break;

            case OMX_StateExecuting:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error same state"));
                return OMX_ErrorSameState;
            }

            default:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error incorrect state"));
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
        return OMX_ErrorNone;
    }

    if (OMX_StateInvalid == aDestinationState)
    {
        switch (iState)
        {
            case OMX_StateInvalid:
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
                return OMX_ErrorInvalidState;
            }

            default:
            {
                iState = OMX_StateInvalid;
                if (iIsInit != OMX_FALSE)
                {
                    pOpenmaxAOType->ComponentDeInit();
                }
            }
            break;
        }

        if (iIsInit != OMX_FALSE)
        {
            pOpenmaxAOType->ComponentDeInit();
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error invalid state"));
        return OMX_ErrorInvalidState;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet OUT"));
    return OMX_ErrorNone;
}



OMX_ERRORTYPE OmxComponentBase::BaseComponentEmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE *)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->EmptyThisBuffer(hComponent, pBuffer);

    return Status;

}


OMX_ERRORTYPE OmxComponentBase::EmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)

{
    OSCL_UNUSED_ARG(hComponent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer IN"));
    //Do not queue buffers if component is in invalid state
    if (OMX_StateInvalid == iState)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error invalid state"));
        return OMX_ErrorInvalidState;
    }

    if ((OMX_StateIdle == iState) || (OMX_StatePause == iState) || (OMX_StateExecuting == iState))
    {
        OMX_U32 PortIndex;
        QueueType* pInputQueue;
        OMX_ERRORTYPE ErrorType = OMX_ErrorNone;

        PortIndex = pBuffer->nInputPortIndex;

        //Validate the port index & Queue the buffers available only at the input port
        if (PortIndex >= iNumPorts ||
                ipPorts[PortIndex]->PortParam.eDir != OMX_DirInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error bad port index"));
            return OMX_ErrorBadPortIndex;
        }

        //Port should be in enabled state before accepting buffers
        if (!PORT_IS_ENABLED(ipPorts[PortIndex]))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error incorrect state"));
            return OMX_ErrorIncorrectStateOperation;
        }

        /* The number of buffers the component can queue at a time
         * depends upon the number of buffers allocated/assigned on the input port
         */
        if (iNumInputBuffer == (ipPorts[PortIndex]->NumAssignedBuffers))
        {
            RunIfNotReady();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error incorrect state"));
            return OMX_ErrorIncorrectStateOperation;
        }

        //Finally after passing all the conditions, queue the buffer in Input queue
        pInputQueue = ipPorts[PortIndex]->pBufferQueue;

        if ((ErrorType = CheckHeader(pBuffer, sizeof(OMX_BUFFERHEADERTYPE))) != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error check header failed"));
            return ErrorType;
        }

        if (OMX_ErrorNone != Queue(pInputQueue, pBuffer))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error, Queuing buffer failed"));
            return OMX_ErrorInsufficientResources;
        }
        iNumInputBuffer++;

        //Signal the AO about the incoming buffer
        RunIfNotReady();
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer error incorrect state"));
        //This macro is not accepted in any other state except the three mentioned above
        return OMX_ErrorIncorrectStateOperation;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : EmptyThisBuffer OUT"));

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentFillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{

    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    OMX_ERRORTYPE Status;

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    Status = pOpenmaxAOType->FillThisBuffer(hComponent, pBuffer);

    return Status;
}

OMX_ERRORTYPE OmxComponentBase::FillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)

{
    OSCL_UNUSED_ARG(hComponent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer IN"));

    OMX_U32 PortIndex;

    QueueType* pOutputQueue;
    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;

    PortIndex = pBuffer->nOutputPortIndex;
    //Validate the port index & Queue the buffers available only at the output port
    if (PortIndex >= iNumPorts ||
            ipPorts[PortIndex]->PortParam.eDir != OMX_DirOutput)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer error bad port index"));
        return OMX_ErrorBadPortIndex;
    }

    pOutputQueue = ipPorts[PortIndex]->pBufferQueue;
    if (iState != OMX_StateExecuting &&
            iState != OMX_StatePause &&
            iState != OMX_StateIdle)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer error invalid state"));
        return OMX_ErrorInvalidState;
    }

    //Port should be in enabled state before accepting buffers
    if (!PORT_IS_ENABLED(ipPorts[PortIndex]))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer error incorrect state"));
        return OMX_ErrorIncorrectStateOperation;
    }

    if ((ErrorType = CheckHeader(pBuffer, sizeof(OMX_BUFFERHEADERTYPE))) != OMX_ErrorNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer error check header failed"));
        return ErrorType;
    }

    //Queue the buffer in output queue
    if (OMX_ErrorNone != Queue(pOutputQueue, pBuffer))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer error, queuing buffer failed"));
        return OMX_ErrorInsufficientResources;
    }

    iOutBufferCount++;

    //Signal the AO about the incoming buffer
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : FillThisBuffer OUT"));

    return OMX_ErrorNone;
}


/** This is the central function for buffers processing and decoding.
	* It is called through the Run() of active object when the component is in executing state
	* and is signalled each time a new buffer is available on the given ports
	* This function will process the input buffers & return output buffers
	*/

void OmxComponentBase::BufferMgmtFunction()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction IN"));

    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*) iOmxComponent.pComponentPrivate;

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType* pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];

    OMX_BOOL PartialFrameReturn, Status;

    /* Don't dequeue any further buffer after endofstream buffer has been dequeued
     * till we send the callback and reset the flag back to false
     */
    if (OMX_FALSE == iEndofStream)
    {
        //More than one frame can't be dequeued in case of outbut blocked
        if ((OMX_TRUE == iIsInputBufferEnded) && (GetQueueNumElem(pInputQueue) > 0))
        {
            ipInputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pInputQueue);
            if (NULL == ipInputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction ERROR DeQueue() returned NULL"));
                return;
            }

            if (ipInputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction EndOfStream arrived"));
                iEndofStream = OMX_TRUE;
            }

            // To do: test this second condition newly added
            if ((ipInputBuffer->nFilledLen != 0) ||
                    ((OMX_TRUE == iEndofStream) && (OMX_TRUE == iPartialFrameAssembly)))
            {
                // if we already started assembling frames, it means
                // we didn't get marker bit yet, but may be getting it
                // when the first frame assembly is over
                // If so, we'll set iEndOfFrameFlag to TRUE in BufferMgmtWithoutMarker assembly
                if (0 == iFrameCount && iPartialFrameAssembly == OMX_FALSE)
                {
                    //Set the marker flag (iEndOfFrameFlag) if first frame has the EndOfFrame flag marked.
                    if ((ipInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME) != 0)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction EndOfFrame flag present"));
                        iEndOfFrameFlag = OMX_TRUE;
                    }

                    /* This routine will allocate the internal input buffers that are required to
                     * assemble partial frames in both with and without marker mode for H.263 decoder component.
                     * The size of partial frame assembly buffer will be in proportion to the W & H of the clip.
                     * We require maximum of first 12 bytes of data (this will be checked */


                    if (OMX_ErrorNone != pOpenmaxAOType->ReAllocatePartialAssemblyBuffers(ipInputBuffer))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction Error Memory Re-allocation of partial frame assembly buffer failed, OUT"));
                        return;
                    }

                }

                /* This condition will be true if OMX_BUFFERFLAG_ENDOFFRAME flag is
                 *  not marked in all the input buffers
                 */
                if (!iEndOfFrameFlag)
                {
                    Status = pOpenmaxAOType->BufferMgmtWithoutMarker();
                    if (OMX_FALSE == Status)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction OUT"));
                        return;
                    }

                }
                //If OMX_BUFFERFLAG_ENDOFFRAME flag is marked, come here
                else
                {
                    if (iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames && (OMX_PORT_INPUTPORT_INDEX == iCompressedFormatPortNum))
                    {
                        // since full frames are sent, there will never be partial frame assembly,
                        // but we do need to parse the frames into NALs to send to the decoder
                        PartialFrameReturn = ParseFullAVCFramesIntoNALs(ipInputBuffer);
                    }
                    else
                    {
                        PartialFrameReturn = AssemblePartialFrames(ipInputBuffer);
                    }
                    if (OMX_FALSE == PartialFrameReturn)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction OUT"));
                        return;
                    }
                    iIsInputBufferEnded = OMX_FALSE;

                    ipTargetComponent = (OMX_COMPONENTTYPE*) ipInputBuffer->hMarkTargetComponent;

                    iTargetMarkData = ipInputBuffer->pMarkData;
                    if (ipTargetComponent == (OMX_COMPONENTTYPE*) pHandle)
                    {
                        (*(ipCallbacks->EventHandler))
                        (pHandle,
                         iCallbackData,
                         OMX_EventMark,
                         1,
                         0,
                         ipInputBuffer->pMarkData);
                    }
                }

                /* This routine will take care of any audio component specific tasks like
                   -> Reading the input buffer timestamp
                   -> Checking for Silence insertion
                   -> Repostioning implementation etc
                */
                if (OMX_TRUE == iIsAudioComponent)
                {
                    pOpenmaxAOType->SyncWithInputTimestamp();
                }


            }	//end braces for if (ipInputBuffer->nFilledLen != 0)
            else
            {
                //Reschedule the AO if there are more buffers in queue
                if ((GetQueueNumElem(pInputQueue) > 0) &&
                        ((GetQueueNumElem(pOutputQueue) > 0) || (OMX_FALSE == iNewOutBufRequired)))
                {
                    RunIfNotReady();
                }

                ReturnInputBuffer(ipInputBuffer, pInPort);
                ipInputBuffer = NULL;
            }

        }	//end braces for if ((OMX_TRUE == iIsInputBufferEnded) && (GetQueueNumElem(pInputQueue) > 0))
    }	//if (OMX_FALSE == iEndofStream)


    //Component specific Encode/Decode routine
    pOpenmaxAOType->ProcessData();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtFunction OUT"));
    return;
}


OMX_BOOL OmxComponentBase::BufferMgmtWithoutMarker()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker IN"));

    ComponentPortType*	pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*) iOmxComponent.pComponentPrivate;


    /* If the buffer has enough data or EndofStream is true, and
     * partial frame assembly is not turned on, process the buffer independently */

    if (((ipInputBuffer->nFilledLen >= (iInputCurrBufferSize >> 1)) || (iEndofStream == OMX_TRUE))
            && (OMX_FALSE == iPartialFrameAssembly))
    {
        //This is a new piece of buffer, process it independently
        if (iNumInputBuffer > 0)
        {
            iInputCurrLength = ipInputBuffer->nFilledLen;
            ipFrameDecodeBuffer = ipInputBuffer->pBuffer + ipInputBuffer->nOffset;
            iFrameTimestamp = ipInputBuffer->nTimeStamp;

            /* Components not implementing ComponentBufferMgmtWithoutMarker(), will reset
             * iIsInputBufferEnded flag in the below routine */
            pOpenmaxAOType->ProcessInBufferFlag();
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker OUT"));
            return OMX_FALSE; // nothing to decode
        }
    }
    else
    {
        if (!iPartialFrameAssembly)
        {
            iInputCurrLength = 0;
            ipFrameDecodeBuffer = ipInputCurrBuffer;
        }

        while (iNumInputBuffer > 0)
        {
            int32 BytesToCopy = ipInputBuffer->nFilledLen;

            if ((iInputCurrLength + BytesToCopy) > iInputCurrBufferSize)
            {
                // allocate new partial frame buffers
                OMX_U8* pTempNewBuffer = NULL;
                pTempNewBuffer = (OMX_U8*) oscl_malloc(sizeof(OMX_U8) * (iInputCurrLength + BytesToCopy));

                if (NULL != pTempNewBuffer)
                {
                    // copy contents of the old buffer into the new one
                    oscl_memcpy(pTempNewBuffer, ipTempInputBuffer, iTempInputBufferLength);
                    // free the old buffer
                    if (ipTempInputBuffer)
                    {
                        oscl_free(ipTempInputBuffer);
                    }
                    // assign new one
                    ipTempInputBuffer = pTempNewBuffer;

                    pTempNewBuffer = NULL;
                    pTempNewBuffer = (OMX_U8*) oscl_malloc(sizeof(OMX_U8) * (iInputCurrLength + BytesToCopy));

                    // in the event that new buffer cannot be allocated
                    if (NULL == pTempNewBuffer)
                    {
                        // copy into what space is available, and let the decoder complain
                        BytesToCopy = iInputCurrLength - iInputCurrBufferSize;
                    }
                    else
                    {
                        // copy contents of the old buffer into the new one
                        oscl_memcpy(pTempNewBuffer, ipInputCurrBuffer, iInputCurrBufferSize);
                        // free the old buffer
                        if (ipInputCurrBuffer)
                        {
                            oscl_free(ipInputCurrBuffer);
                        }
                        // assign new memory location
                        ipInputCurrBuffer = pTempNewBuffer;
                        iInputCurrBufferSize = (iInputCurrLength + BytesToCopy);
                        ipFrameDecodeBuffer = ipInputCurrBuffer + iInputCurrLength;
                    }
                }
                //No memory to allocate ipTempInputBuffer
                else
                {
                    // copy into what space is available, and let the decoder complain
                    BytesToCopy = iInputCurrLength - iInputCurrBufferSize;
                }
            }

            oscl_memcpy(ipFrameDecodeBuffer, (ipInputBuffer->pBuffer + ipInputBuffer->nOffset), BytesToCopy);
            ipFrameDecodeBuffer += ipInputBuffer->nFilledLen; // move the ptr
            iInputCurrLength += BytesToCopy;

            iFrameTimestamp = ipInputBuffer->nTimeStamp;

            // check if we've encountered end of frame flag while trying to assemble the very first frame
            if ((0 == iFrameCount) && ((ipInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME) != 0))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker EndOfFrameFlag finally arrived"));
                iEndOfFrameFlag = OMX_TRUE;
            }

            if ((iInputCurrLength >= (iInputCurrBufferSize >> 1))
                    || (OMX_TRUE == iEndofStream) || (OMX_TRUE == iEndOfFrameFlag))
            {
                break;
            }

            //Set the filled len to zero to indicate buffer is fully consumed
            ipInputBuffer->nFilledLen = 0;
            ReturnInputBuffer(ipInputBuffer, pInPort);
            ipInputBuffer = NULL;

            if (iNumInputBuffer > 0)
            {
                ipInputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pInputQueue);
                if (NULL == ipInputBuffer)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker Error, Input buffer Dequeue returned NULL"));
                    return OMX_FALSE;
                }

                if (ipInputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker EndOfStream arrived"));
                    iEndofStream = OMX_TRUE;
                }
            }
        }

        if (iEndOfFrameFlag)
        {
            // if we have encountered end of frame, 1st frame has been assembled
            // and we can switch to "end of frame flag" mode
            iIsInputBufferEnded = OMX_FALSE;
            iNewInBufferRequired = OMX_FALSE;
            ipFrameDecodeBuffer = ipInputCurrBuffer; // rewind buffer ptr to beginning of inputcurrbuffer
            iPartialFrameAssembly = OMX_FALSE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker Found end of frame flag - OUT"));
            return OMX_TRUE;

        }

        if ((((iInputCurrLength + iTempInputBufferLength) < (iInputCurrBufferSize >> 1)))
                && (OMX_TRUE != iEndofStream))
        {
            iPartialFrameAssembly = OMX_TRUE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker OUT"));
            return OMX_FALSE;
        }
        else
        {
            ipFrameDecodeBuffer = ipInputCurrBuffer;
            iPartialFrameAssembly = OMX_FALSE;

            /* Components not implementing ComponentBufferMgmtWithoutMarker(), will reset
             * iIsInputBufferEnded flag in the below routine */
            pOpenmaxAOType->ProcessInBufferFlag();
        }

    }


    //Different components may choose to do some extra processing here.
    //e.g. video components and also amr component copies the collected data into temp input buffers here.
    pOpenmaxAOType->ComponentBufferMgmtWithoutMarker();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : BufferMgmtWithoutMarker OUT"));
    return OMX_TRUE;

}



//Propagate here the buffer mark through output port in case of BufferMgmtWithoutMarker
void OmxComponentBase::ComponentBufferMgmtWithoutMarker()
{
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;

    ipTargetComponent = (OMX_COMPONENTTYPE*) ipInputBuffer->hMarkTargetComponent;

    iTargetMarkData = ipInputBuffer->pMarkData;
    if (ipTargetComponent == (OMX_COMPONENTTYPE*) pHandle)
    {
        (*(ipCallbacks->EventHandler))
        (pHandle,
         iCallbackData,
         OMX_EventMark,
         1,
         0,
         ipInputBuffer->pMarkData);
    }
}


/* A part of buffer management without marker routine, this function will
 * copy the current input buffer into a big temporary buffer, so that
 * an incomplete/partial frame is never passed to the decoder library for decode
*/
void OmxComponentBase::TempInputBufferMgmtWithoutMarker()
{
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;
    ComponentPortType*	pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    OMX_U32 TempInputBufferSize = iInputCurrBufferSize;

    if (iTempInputBufferLength < (TempInputBufferSize >> 1))
    {
        oscl_memmove(ipTempInputBuffer, &ipTempInputBuffer[iTempConsumedLength], iTempInputBufferLength);
        iIsInputBufferEnded = OMX_TRUE;
        iTempConsumedLength = 0;
    }

    if ((iTempInputBufferLength + iTempConsumedLength + iInputCurrLength)
            <= TempInputBufferSize)
    {
        oscl_memcpy(&ipTempInputBuffer[iTempInputBufferLength + iTempConsumedLength], ipFrameDecodeBuffer, iInputCurrLength);
        iTempInputBufferLength += iInputCurrLength;

        if (iTempInputBufferLength + (TempInputBufferSize >> 1) <= TempInputBufferSize)
        {
            iNewInBufferRequired = OMX_TRUE;
        }
        else
        {
            iNewInBufferRequired = OMX_FALSE;
        }

        ipTargetComponent = (OMX_COMPONENTTYPE*) ipInputBuffer->hMarkTargetComponent;

        iTargetMarkData = ipInputBuffer->pMarkData;
        if (ipTargetComponent == (OMX_COMPONENTTYPE*) pHandle)
        {
            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventMark,
             1,
             0,
             ipInputBuffer->pMarkData);
        }
        ipInputBuffer->nFilledLen = 0;
        ReturnInputBuffer(ipInputBuffer, pInPort);
        ipInputBuffer = NULL;

    }

    if (iTempInputBufferLength >= (TempInputBufferSize >> 1))
    {
        iIsInputBufferEnded = OMX_FALSE;
    }
}



void OmxComponentBase::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : Run IN"));

    CoreMessage* pCoreMessage;
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*) iOmxComponent.pComponentPrivate;

    //Execute the commands from the message handler queue
    if ((GetQueueNumElem(ipCoreDescriptor->pMessageQueue) > 0))
    {
        pCoreMessage = (CoreMessage*) DeQueue(ipCoreDescriptor->pMessageQueue);

        if (OMX_CommandStateSet == pCoreMessage->MessageParam1)
        {
            if (OMX_StateExecuting == pCoreMessage->MessageParam2)
            {
                iBufferExecuteFlag = OMX_TRUE;
            }
            else
            {
                iBufferExecuteFlag = OMX_FALSE;
            }
        }

        MessageHandler(pCoreMessage);

        /* If some allocations/deallocations are required before the state transition
         * then queue the command again to be executed later on
         */
        if (OMX_TRUE == iStateTransitionFlag)
        {
            if (OMX_ErrorNone != Queue(ipCoreDescriptor->pMessageQueue, pCoreMessage))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : Error, Queue command failed, Run OUT"));
                return;
            }

            // Don't reschedule. Wait for arriving buffers to do it
            //RunIfNotReady();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : Run OUT"));
            return;
        }

        else
        {
            oscl_free(pCoreMessage);
            pCoreMessage = NULL;
        }
    }

    /* If the component is in executing state, call the Buffer management function.
     * Stop calling this function as soon as state transition request is received.
     */
    if ((OMX_TRUE == iBufferExecuteFlag) && (OMX_TRUE != iResizePending))
    {
        pOpenmaxAOType->BufferMgmtFunction();
    }

    //Check for any more commands in the message handler queue & schedule them for later
    if ((GetQueueNumElem(ipCoreDescriptor->pMessageQueue) > 0))
    {
        RunIfNotReady();
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : Run OUT"));

    return;
}


/**************************
 AUDIO BASE CLASS ROUTINES
 **************************/
OmxComponentAudio::OmxComponentAudio()
{
    iIsAudioComponent = OMX_TRUE;

}


OMX_ERRORTYPE OmxComponentAudio::GetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OSCL_UNUSED_ARG(hComponent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter IN"));

    OMX_PRIORITYMGMTTYPE* pPrioMgmt;
    OMX_PARAM_BUFFERSUPPLIERTYPE* pBufSupply;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_PORT_PARAM_TYPE* pPortDomains;
    OMX_U32 PortIndex;

    OMX_AUDIO_PARAM_PORTFORMATTYPE* pAudioPortFormat;
    OMX_AUDIO_PARAM_PCMMODETYPE* pAudioPcmMode;
    OMX_AUDIO_PARAM_WMATYPE* pAudioWma;
    OMX_AUDIO_PARAM_MP3TYPE* pAudioMp3;
    OMX_AUDIO_CONFIG_EQUALIZERTYPE* pAudioEqualizer;
    OMX_AUDIO_PARAM_AACPROFILETYPE* pAudioAac;
    OMX_AUDIO_PARAM_AMRTYPE* pAudioAmr;

    ComponentPortType* pComponentPort;

    if (NULL == ComponentParameterStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPriorityMgmt:
        {
            pPrioMgmt = (OMX_PRIORITYMGMTTYPE*) ComponentParameterStructure;
            SetHeader(pPrioMgmt, sizeof(OMX_PRIORITYMGMTTYPE));
            pPrioMgmt->nGroupPriority = iGroupPriority;
            pPrioMgmt->nGroupID = iGroupID;
        }
        break;

        case OMX_IndexParamAudioInit:
        {
            SetHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
            oscl_memcpy(ComponentParameterStructure, &iPortTypesParam, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;


        //Following 3 cases have a single common piece of code to be executed
        case OMX_IndexParamVideoInit:
        case OMX_IndexParamImageInit:
        case OMX_IndexParamOtherInit:
        {
            pPortDomains = (OMX_PORT_PARAM_TYPE*) ComponentParameterStructure;
            SetHeader(pPortDomains, sizeof(OMX_PORT_PARAM_TYPE));
            pPortDomains->nPorts = 0;
            pPortDomains->nStartPortNumber = 0;
        }
        break;

        case OMX_IndexParamAudioPortFormat:
        {
            pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*) ComponentParameterStructure;
            //Added to pass parameter test
            if (pAudioPortFormat->nIndex > ipPorts[pAudioPortFormat->nPortIndex]->AudioParam.nIndex)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error index out of range"));
                return OMX_ErrorNoMore;
            }
            SetHeader(pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            if (pAudioPortFormat->nPortIndex <= 1)
            {
                pComponentPort = (ComponentPortType*) ipPorts[pAudioPortFormat->nPortIndex];
                oscl_memcpy(pAudioPortFormat, &pComponentPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
        }
        break;

        case OMX_IndexParamAudioPcm:
        {
            pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*) ComponentParameterStructure;
            if (pAudioPcmMode->nPortIndex > 1)
            {
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioPcmMode->nPortIndex;
            oscl_memcpy(pAudioPcmMode, &ipPorts[PortIndex]->AudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            SetHeader(pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        }
        break;

        case OMX_IndexParamAudioMp3:
        {
            pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE*) ComponentParameterStructure;
            if (pAudioMp3->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioMp3->nPortIndex;
            oscl_memcpy(pAudioMp3, &ipPorts[PortIndex]->AudioMp3Param, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
            SetHeader(pAudioMp3, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
        }
        break;

        case OMX_IndexParamAudioWma:
        {
            pAudioWma = (OMX_AUDIO_PARAM_WMATYPE*) ComponentParameterStructure;
            if (pAudioWma->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioWma->nPortIndex;
            oscl_memcpy(pAudioWma, &ipPorts[PortIndex]->AudioWmaParam, sizeof(OMX_AUDIO_PARAM_WMATYPE));
            SetHeader(pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));
        }
        break;

        case OMX_IndexConfigAudioEqualizer:
        {
            pAudioEqualizer = (OMX_AUDIO_CONFIG_EQUALIZERTYPE*) ComponentParameterStructure;
            if (pAudioEqualizer->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioEqualizer->nPortIndex;
            oscl_memcpy(pAudioEqualizer, &ipPorts[PortIndex]->AudioEqualizerType, sizeof(OMX_AUDIO_CONFIG_EQUALIZERTYPE));
            SetHeader(pAudioEqualizer, sizeof(OMX_AUDIO_CONFIG_EQUALIZERTYPE));
        }
        break;

        case OMX_IndexParamAudioAac:
        {
            pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE*) ComponentParameterStructure;
            if (pAudioAac->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioAac->nPortIndex;
            oscl_memcpy(pAudioAac, &ipPorts[PortIndex]->AudioAacParam, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
            SetHeader(pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
        }
        break;

        case OMX_IndexParamAudioAmr:
        {
            pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE*) ComponentParameterStructure;
            if (pAudioAmr->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pAudioAmr->nPortIndex;
            oscl_memcpy(pAudioAmr, &ipPorts[PortIndex]->AudioAmrParam, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            SetHeader(pAudioAmr, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
        }
        break;

        case OMX_IndexParamPortDefinition:
        {
            pPortDef  = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
            PortIndex = pPortDef->nPortIndex;
            if (PortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            oscl_memcpy(pPortDef, &ipPorts[PortIndex]->PortParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        }
        break;

        case OMX_IndexParamCompBufferSupplier:
        {
            pBufSupply = (OMX_PARAM_BUFFERSUPPLIERTYPE*) ComponentParameterStructure;
            PortIndex = pBufSupply->nPortIndex;
            if (PortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            SetHeader(pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));

            if (OMX_DirInput == ipPorts[PortIndex]->PortParam.eDir)
            {
                pBufSupply->eBufferSupplier = OMX_BufferSupplyUnspecified;
            }
            else
            {
                SetHeader(pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
                pBufSupply->eBufferSupplier = OMX_BufferSupplyUnspecified;
            }
        }
        break;

        case(OMX_INDEXTYPE) PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX:
        {
            PV_OMXComponentCapabilityFlagsType *pCap_flags = (PV_OMXComponentCapabilityFlagsType *) ComponentParameterStructure;
            if (NULL == pCap_flags)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error pCap_flags NULL"));
                return OMX_ErrorBadParameter;
            }
            oscl_memcpy(pCap_flags, &iPVCapabilityFlags, sizeof(iPVCapabilityFlags));

        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter error Unsupported Index"));
            return OMX_ErrorUnsupportedIndex;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : GetParameter OUT"));

    return OMX_ErrorNone;

}


OMX_ERRORTYPE OmxComponentAudio::SetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter IN"));

    OMX_PRIORITYMGMTTYPE* pPrioMgmt;
    OMX_AUDIO_PARAM_PORTFORMATTYPE* pAudioPortFormat;
    OMX_AUDIO_PARAM_PCMMODETYPE* pAudioPcmMode;
    OMX_PARAM_BUFFERSUPPLIERTYPE* pBufSupply;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef ;
    OMX_PARAM_COMPONENTROLETYPE* pCompRole;
    ComponentPortType* pComponentPort;

    OMX_AUDIO_PARAM_WMATYPE* pAudioWma;
    OMX_AUDIO_PARAM_MP3TYPE* pAudioMp3;
    OMX_AUDIO_CONFIG_EQUALIZERTYPE* pAudioEqualizer;
    OMX_AUDIO_PARAM_AACPROFILETYPE* pAudioAac;
    OMX_AUDIO_PARAM_AMRTYPE* pAudioAmr;

    OMX_U32 PortIndex;
    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    OmxComponentAudio* pOpenmaxAOType = (OmxComponentAudio*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;


    if (NULL == ComponentParameterStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamAudioInit:
        {
            /*Check Structure Header*/
            CheckHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error audio init failed"));
                return ErrorType;
            }
            oscl_memcpy(&iPortTypesParam, ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;

        case OMX_IndexParamAudioPortFormat:
        {
            pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*) ComponentParameterStructure;
            PortIndex = pAudioPortFormat->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            if (PortIndex <= 1)
            {
                pComponentPort = (ComponentPortType*) ipPorts[PortIndex];
                oscl_memcpy(&pComponentPort->AudioParam, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
        }
        break;

        case OMX_IndexParamAudioPcm:
        {
            pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*) ComponentParameterStructure;
            PortIndex = pAudioPcmMode->nPortIndex;
            /*Check Structure Header and verify component State*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            oscl_memcpy(&ipPorts[PortIndex]->AudioPcmMode, pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        }
        break;

        case OMX_IndexParamAudioMp3:
        {
            pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE*) ComponentParameterStructure;
            PortIndex = pAudioMp3->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioMp3, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->AudioMp3Param, pAudioMp3, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
        }
        break;

        case OMX_IndexParamAudioWma:
        {


            pAudioWma = (OMX_AUDIO_PARAM_WMATYPE*) ComponentParameterStructure;
            PortIndex = pAudioWma->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->AudioWmaParam, pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));
        }
        break;

        case OMX_IndexConfigAudioEqualizer:
        {
            pAudioEqualizer = (OMX_AUDIO_CONFIG_EQUALIZERTYPE*) ComponentParameterStructure;
            PortIndex = pAudioEqualizer->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioEqualizer, sizeof(OMX_AUDIO_CONFIG_EQUALIZERTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->AudioEqualizerType, pAudioEqualizer, sizeof(OMX_AUDIO_CONFIG_EQUALIZERTYPE));
        }
        break;

        case OMX_IndexParamAudioAac:
        {
            OMX_BOOL AacPlusFlag = OMX_TRUE;

            pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE*) ComponentParameterStructure;
            PortIndex = pAudioAac->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->AudioAacParam, pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));

            if ((ipPorts[PortIndex]->AudioAacParam.eAACProfile == OMX_AUDIO_AACObjectHE)
                    || (ipPorts[PortIndex]->AudioAacParam.eAACProfile == OMX_AUDIO_AACObjectHE_PS))
            {
                AacPlusFlag = OMX_TRUE;
            }
            else
            {
                AacPlusFlag = OMX_FALSE;
            }

            pOpenmaxAOType->UpdateAACPlusFlag(AacPlusFlag);
        }
        break;

        case OMX_IndexParamAudioAmr:
        {
            pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE*) ComponentParameterStructure;
            PortIndex = pAudioAmr->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pAudioAmr, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->AudioAmrParam, pAudioAmr, sizeof(OMX_AUDIO_PARAM_AMRTYPE));

            //If the band mode turns out to be WB, set the sampling freq to 16KHz
            if ((pAudioAmr->eAMRBandMode >= OMX_AUDIO_AMRBandModeWB0) &&
                    (pAudioAmr->eAMRBandMode <= OMX_AUDIO_AMRBandModeWB8))
            {
                ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nSamplingRate = 16000;
            }
        }
        break;

        case OMX_IndexParamPriorityMgmt:
        {
            if (iState != OMX_StateLoaded && iState != OMX_StateWaitForResources)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error incorrect state error"));
                return OMX_ErrorIncorrectStateOperation;
            }
            pPrioMgmt = (OMX_PRIORITYMGMTTYPE*) ComponentParameterStructure;
            if ((ErrorType = CheckHeader(pPrioMgmt, sizeof(OMX_PRIORITYMGMTTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            iGroupPriority = pPrioMgmt->nGroupPriority;
            iGroupID = pPrioMgmt->nGroupID;
        }
        break;

        case OMX_IndexParamPortDefinition:
        {
            pPortDef  = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
            PortIndex = pPortDef->nPortIndex;

            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }

            ipPorts[PortIndex]->PortParam.nBufferCountActual = pPortDef->nBufferCountActual;
            ipPorts[PortIndex]->PortParam.nBufferSize = pPortDef->nBufferSize;

            oscl_memcpy(&ipPorts[PortIndex]->PortParam.format.audio, &pPortDef->format.audio, sizeof(OMX_AUDIO_PORTDEFINITIONTYPE));
        }
        break;

        case OMX_IndexParamCompBufferSupplier:
        {
            pBufSupply = (OMX_PARAM_BUFFERSUPPLIERTYPE*) ComponentParameterStructure;
            PortIndex = pBufSupply->nPortIndex;

            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
            if (OMX_ErrorIncorrectStateOperation == ErrorType)
            {
                if (PORT_IS_ENABLED(ipPorts[pBufSupply->nPortIndex]))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error incorrect state error"));
                    return OMX_ErrorIncorrectStateOperation;
                }
            }
            else if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error parameter sanity check error"));
                return ErrorType;
            }

            if (pBufSupply->eBufferSupplier == OMX_BufferSupplyUnspecified)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter OUT"));
                return OMX_ErrorNone;
            }

            ErrorType = OMX_ErrorNone;
        }
        break;

        case OMX_IndexParamStandardComponentRole:
        {
            pCompRole = (OMX_PARAM_COMPONENTROLETYPE*) ComponentParameterStructure;
            if ((ErrorType = CheckHeader(pCompRole, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
            {
                break;
            }
            oscl_strncpy((OMX_STRING)iComponentRole, (OMX_STRING)pCompRole->cRole, OMX_MAX_STRINGNAME_SIZE);
            iComponentRoleFlag = OMX_TRUE;
        }
        break;


        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter error Unsupported index"));
            return OMX_ErrorUnsupportedIndex;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAudio : SetParameter OUT"));
    return ErrorType;

}


/*************************
 VIDEO BASE CLASS ROUTINES
 *************************/
OmxComponentVideo::OmxComponentVideo()
{
    iIsAudioComponent = OMX_FALSE;

}


OMX_ERRORTYPE OmxComponentVideo::GetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OSCL_UNUSED_ARG(hComponent);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter IN"));

    OMX_PRIORITYMGMTTYPE* pPrioMgmt;
    OMX_PARAM_BUFFERSUPPLIERTYPE* pBufSupply;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_PORT_PARAM_TYPE* pPortDomains;
    OMX_U32 PortIndex;

    OMX_VIDEO_PARAM_PORTFORMATTYPE* pVideoPortFormat;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE* pProfileLevel;

    OMX_VIDEO_PARAM_MPEG4TYPE* pVideoMpeg4;
    OMX_VIDEO_PARAM_H263TYPE* pVideoH263;
    OMX_VIDEO_PARAM_AVCTYPE* pVideoAvc;
    OMX_VIDEO_PARAM_WMVTYPE* pVideoWmv;

    //Video encoder configuration parameters
    OMX_CONFIG_ROTATIONTYPE*			 pVideoRotation;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE* pVideoErrCorr;
    OMX_VIDEO_PARAM_BITRATETYPE*		 pVideoRateControl;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE*	 pVideoQuant;
    OMX_VIDEO_PARAM_VBSMCTYPE*			 pVideoBlock;
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE*	 pVideoMotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE*	 pVideoIntraRefresh;

    if (NULL == ComponentParameterStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPriorityMgmt:
        {
            pPrioMgmt = (OMX_PRIORITYMGMTTYPE*) ComponentParameterStructure;
            SetHeader(pPrioMgmt, sizeof(OMX_PRIORITYMGMTTYPE));
            pPrioMgmt->nGroupPriority = iGroupPriority;
            pPrioMgmt->nGroupID = iGroupID;
        }
        break;

        case OMX_IndexParamVideoInit:
        {
            SetHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
            oscl_memcpy(ComponentParameterStructure, &iPortTypesParam, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;


        //Following 3 cases have a single common piece of code to be executed
        case OMX_IndexParamAudioInit:
        case OMX_IndexParamImageInit:
        case OMX_IndexParamOtherInit:
        {
            pPortDomains = (OMX_PORT_PARAM_TYPE*) ComponentParameterStructure;
            SetHeader(pPortDomains, sizeof(OMX_PORT_PARAM_TYPE));
            pPortDomains->nPorts = 0;
            pPortDomains->nStartPortNumber = 0;
        }
        break;

        case OMX_IndexParamVideoPortFormat:
        {
            pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE*) ComponentParameterStructure;

            //Check for valid port index
            PortIndex = pVideoPortFormat->nPortIndex;
            if (PortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }

            OMX_U32 QueriedIndex = pVideoPortFormat->nIndex;
            if (QueriedIndex >= ipPorts[PortIndex]->ActualNumPortFormatsSupported)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error index out of range"));
                return OMX_ErrorNoMore;
            }
            else
            {
                SetHeader(pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                oscl_memcpy(pVideoPortFormat, &ipPorts[PortIndex]->VideoParam[QueriedIndex], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            }
        }

        break;

        case OMX_IndexParamVideoMpeg4:
        {
            pVideoMpeg4 = (OMX_VIDEO_PARAM_MPEG4TYPE*) ComponentParameterStructure;
            if (pVideoMpeg4->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoMpeg4->nPortIndex;
            oscl_memcpy(pVideoMpeg4, &ipPorts[PortIndex]->VideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            SetHeader(pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
        }
        break;

        case OMX_IndexParamVideoH263:
        {
            pVideoH263 = (OMX_VIDEO_PARAM_H263TYPE*) ComponentParameterStructure;
            if (pVideoH263->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoH263->nPortIndex;
            oscl_memcpy(pVideoH263, &ipPorts[PortIndex]->VideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            SetHeader(pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
        }
        break;

        case OMX_IndexParamVideoAvc:
        {
            pVideoAvc = (OMX_VIDEO_PARAM_AVCTYPE*) ComponentParameterStructure;
            if (pVideoAvc->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoAvc->nPortIndex;
            oscl_memcpy(pVideoAvc, &ipPorts[PortIndex]->VideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            SetHeader(pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
        }
        break;

        case OMX_IndexParamVideoWmv:
        {
            pVideoWmv = (OMX_VIDEO_PARAM_WMVTYPE*)ComponentParameterStructure;
            if (pVideoWmv->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoWmv->nPortIndex;
            oscl_memcpy(pVideoWmv, &ipPorts[PortIndex]->VideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
            SetHeader(pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
        }
        break;

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            pProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*) ComponentParameterStructure;

            PortIndex = pProfileLevel->nPortIndex;
            if (pProfileLevel->nProfileIndex > ipPorts[PortIndex]->ProfileLevel.nProfileIndex)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error profile not supported"));
                return OMX_ErrorNoMore;
            }

            oscl_memcpy(pProfileLevel, &ipPorts[PortIndex]->ProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
            SetHeader(pProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
        }
        break;

        case OMX_IndexParamVideoProfileLevelCurrent:
        {
            pProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*) ComponentParameterStructure;

            PortIndex = pProfileLevel->nPortIndex;

            oscl_memcpy(pProfileLevel, &ipPorts[PortIndex]->ProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
            SetHeader(pProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
        }
        break;

        case OMX_IndexParamPortDefinition:
        {
            pPortDef  = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
            PortIndex = pPortDef->nPortIndex;
            if (PortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            oscl_memcpy(pPortDef, &ipPorts[PortIndex]->PortParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        }
        break;

        case OMX_IndexParamCompBufferSupplier:
        {
            pBufSupply = (OMX_PARAM_BUFFERSUPPLIERTYPE*) ComponentParameterStructure;
            PortIndex = pBufSupply->nPortIndex;
            if (PortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            SetHeader(pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));

            if (OMX_DirInput == ipPorts[PortIndex]->PortParam.eDir)
            {
                pBufSupply->eBufferSupplier = OMX_BufferSupplyUnspecified;
            }
            else
            {
                SetHeader(pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
                pBufSupply->eBufferSupplier = OMX_BufferSupplyUnspecified;
            }
        }
        break;

        case(OMX_INDEXTYPE) PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX:
        {
            PV_OMXComponentCapabilityFlagsType *pCap_flags = (PV_OMXComponentCapabilityFlagsType *) ComponentParameterStructure;
            if (NULL == pCap_flags)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error pCap_flags NULL"));
                return OMX_ErrorBadParameter;
            }
            oscl_memcpy(pCap_flags, &iPVCapabilityFlags, sizeof(iPVCapabilityFlags));

        }
        break;

        case OMX_IndexConfigCommonRotate:
        {
            pVideoRotation = (OMX_CONFIG_ROTATIONTYPE*) ComponentParameterStructure;
            if (pVideoRotation->nPortIndex >= iNumPorts)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoRotation->nPortIndex;
            oscl_memcpy(pVideoRotation, &ipPorts[PortIndex]->VideoOrientationType, sizeof(OMX_CONFIG_ROTATIONTYPE));
            SetHeader(pVideoRotation, sizeof(OMX_CONFIG_ROTATIONTYPE));
        }
        break;

        case OMX_IndexParamVideoErrorCorrection:
        {
            pVideoErrCorr = (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE*) ComponentParameterStructure;
            if (pVideoErrCorr->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoErrCorr->nPortIndex;
            oscl_memcpy(pVideoErrCorr, &ipPorts[PortIndex]->VideoErrorCorrection, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            SetHeader(pVideoErrCorr, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
        }
        break;

        case OMX_IndexParamVideoBitrate:
        {
            pVideoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE*) ComponentParameterStructure;
            if (pVideoRateControl->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoRateControl->nPortIndex;
            oscl_memcpy(pVideoRateControl, &ipPorts[PortIndex]->VideoRateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            SetHeader(pVideoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
        }
        break;

        case OMX_IndexParamVideoQuantization:
        {
            pVideoQuant = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*) ComponentParameterStructure;
            if (pVideoQuant->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoQuant->nPortIndex;
            oscl_memcpy(pVideoQuant, &ipPorts[PortIndex]->VideoQuantType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            SetHeader(pVideoQuant, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
        }
        break;

        case OMX_IndexParamVideoVBSMC:
        {
            pVideoBlock = (OMX_VIDEO_PARAM_VBSMCTYPE*) ComponentParameterStructure;
            if (pVideoBlock->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoBlock->nPortIndex;
            oscl_memcpy(pVideoBlock, &ipPorts[PortIndex]->VideoBlockMotionSize, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
            SetHeader(pVideoBlock, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
        }
        break;

        case OMX_IndexParamVideoMotionVector:
        {
            pVideoMotionVector = (OMX_VIDEO_PARAM_MOTIONVECTORTYPE*) ComponentParameterStructure;
            if (pVideoMotionVector->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoMotionVector->nPortIndex;
            oscl_memcpy(pVideoMotionVector, &ipPorts[PortIndex]->VideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
            SetHeader(pVideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
        }
        break;

        case OMX_IndexParamVideoIntraRefresh:
        {
            pVideoIntraRefresh = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE*) ComponentParameterStructure;
            if (pVideoIntraRefresh->nPortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
            PortIndex = pVideoIntraRefresh->nPortIndex;
            oscl_memcpy(pVideoIntraRefresh, &ipPorts[PortIndex]->VideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            SetHeader(pVideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter error Unsupported Index"));
            return OMX_ErrorUnsupportedIndex;
        }
        // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : GetParameter OUT"));

    return OMX_ErrorNone;

}


OMX_ERRORTYPE OmxComponentVideo::SetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter IN"));

    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    OMX_PRIORITYMGMTTYPE* pPrioMgmt;
    OMX_VIDEO_PARAM_PORTFORMATTYPE* pVideoPortFormat;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE* pProfileLevel;
    OMX_PARAM_BUFFERSUPPLIERTYPE* pBufSupply;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef ;
    OMX_U32 PortIndex;

    OMX_PARAM_COMPONENTROLETYPE* pCompRole;
    OMX_VIDEO_PARAM_MPEG4TYPE*   pVideoMpeg4;
    OMX_VIDEO_PARAM_H263TYPE*    pVideoH263;
    OMX_VIDEO_PARAM_WMVTYPE*     pVideoWmv;
    OMX_VIDEO_PARAM_AVCTYPE*     pVideoAvc;

    //Video encoder configuration parameters
    OMX_CONFIG_ROTATIONTYPE*			 pVideoRotation;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE* pVideoErrCorr;
    OMX_VIDEO_PARAM_BITRATETYPE*		 pVideoRateControl;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE*	 pVideoQuant;
    OMX_VIDEO_PARAM_VBSMCTYPE*			 pVideoBlock;
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE*	 pVideoMotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE*	 pVideoIntraRefresh;


    ComponentPortType* pComponentPort;

    if (NULL == ComponentParameterStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            /*Check Structure Header*/
            CheckHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error video init check header failed"));
                return ErrorType;
            }
            oscl_memcpy(&iPortTypesParam, ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE));
        }
        break;

        case OMX_IndexParamVideoPortFormat:
        {
            pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE*) ComponentParameterStructure;
            PortIndex = pVideoPortFormat->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            if (PortIndex <= 1)
            {
                pComponentPort = (ComponentPortType*) ipPorts[PortIndex];
                oscl_memcpy(&pComponentPort->VideoParam, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error bad port index"));
                return OMX_ErrorBadPortIndex;
            }
        }
        break;

        case OMX_IndexParamVideoMpeg4:
        {
            pVideoMpeg4 = (OMX_VIDEO_PARAM_MPEG4TYPE*) ComponentParameterStructure;
            PortIndex = pVideoMpeg4->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoMpeg4, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
        }
        break;

        case OMX_IndexParamVideoH263:
        {
            pVideoH263 = (OMX_VIDEO_PARAM_H263TYPE*) ComponentParameterStructure;
            PortIndex = pVideoH263->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoH263, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
        }
        break;

        case OMX_IndexParamVideoAvc:
        {
            pVideoAvc = (OMX_VIDEO_PARAM_AVCTYPE*) ComponentParameterStructure;
            PortIndex = pVideoAvc->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoAvc, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
        }
        break;

        case OMX_IndexParamVideoWmv:
        {
            pVideoWmv = (OMX_VIDEO_PARAM_WMVTYPE*)ComponentParameterStructure;
            PortIndex = pVideoWmv->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoWmv, pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
        }
        break;


        case OMX_IndexParamVideoProfileLevelCurrent:
        {
            pProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*) ComponentParameterStructure;
            PortIndex = pProfileLevel->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
            if (OMX_ErrorNone != ErrorType)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error parameter sanity check error"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->ProfileLevel, pProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
        }
        break;

        case OMX_IndexParamPriorityMgmt:
        {
            if (iState != OMX_StateLoaded && iState != OMX_StateWaitForResources)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error incorrect state error"));
                return OMX_ErrorIncorrectStateOperation;
            }
            pPrioMgmt = (OMX_PRIORITYMGMTTYPE*) ComponentParameterStructure;
            if ((ErrorType = CheckHeader(pPrioMgmt, sizeof(OMX_PRIORITYMGMTTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            iGroupPriority = pPrioMgmt->nGroupPriority;
            iGroupID = pPrioMgmt->nGroupID;
        }
        break;

        case OMX_IndexParamPortDefinition:
        {
            pPortDef  = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
            PortIndex = pPortDef->nPortIndex;

            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error parameter sanity check error"));
                return ErrorType;
            }

            ipPorts[PortIndex]->PortParam.nBufferCountActual = pPortDef->nBufferCountActual;
            ipPorts[PortIndex]->PortParam.nBufferSize = pPortDef->nBufferSize;

            oscl_memcpy(&ipPorts[PortIndex]->PortParam.format.video, &pPortDef->format.video, sizeof(OMX_VIDEO_PORTDEFINITIONTYPE));

        }
        break;

        case OMX_IndexParamCompBufferSupplier:
        {
            pBufSupply = (OMX_PARAM_BUFFERSUPPLIERTYPE*) ComponentParameterStructure;
            PortIndex = pBufSupply->nPortIndex;

            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pBufSupply, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
            if (OMX_ErrorIncorrectStateOperation == ErrorType)
            {
                if (PORT_IS_ENABLED(ipPorts[pBufSupply->nPortIndex]))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error incorrect state error"));
                    return OMX_ErrorIncorrectStateOperation;
                }
            }
            else if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error parameter sanity check error"));
                return ErrorType;
            }

            if (pBufSupply->eBufferSupplier == OMX_BufferSupplyUnspecified)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter OUT"));
                return OMX_ErrorNone;
            }

            ErrorType = OMX_ErrorNone;
        }
        break;

        case OMX_IndexParamStandardComponentRole:
        {
            pCompRole = (OMX_PARAM_COMPONENTROLETYPE*) ComponentParameterStructure;
            if ((ErrorType = CheckHeader(pCompRole, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
            {
                break;
            }
            oscl_strncpy((OMX_STRING)iComponentRole, (OMX_STRING)pCompRole->cRole, OMX_MAX_STRINGNAME_SIZE);
            iComponentRoleFlag = OMX_TRUE;
        }
        break;


        case OMX_IndexConfigCommonRotate:
        {
            pVideoRotation = (OMX_CONFIG_ROTATIONTYPE*) ComponentParameterStructure;
            PortIndex = pVideoRotation->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoRotation, sizeof(OMX_CONFIG_ROTATIONTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoOrientationType, pVideoRotation, sizeof(OMX_CONFIG_ROTATIONTYPE));
        }
        break;

        case OMX_IndexParamVideoErrorCorrection:
        {
            pVideoErrCorr = (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE*) ComponentParameterStructure;
            PortIndex = pVideoErrCorr->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoErrCorr, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoErrorCorrection, pVideoErrCorr, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
        }
        break;

        case OMX_IndexParamVideoBitrate:
        {
            pVideoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE*) ComponentParameterStructure;
            PortIndex = pVideoRateControl->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoRateType, pVideoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
        }
        break;

        case OMX_IndexParamVideoQuantization:
        {
            pVideoQuant = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*) ComponentParameterStructure;
            PortIndex = pVideoQuant->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoQuant, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoQuantType, pVideoQuant, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
        }
        break;

        case OMX_IndexParamVideoVBSMC:
        {
            pVideoBlock = (OMX_VIDEO_PARAM_VBSMCTYPE*) ComponentParameterStructure;
            PortIndex = pVideoBlock->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoBlock, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoBlockMotionSize, pVideoBlock, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
        }
        break;

        case OMX_IndexParamVideoMotionVector:
        {
            pVideoMotionVector = (OMX_VIDEO_PARAM_MOTIONVECTORTYPE*) ComponentParameterStructure;
            PortIndex = pVideoMotionVector->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoMotionVector, pVideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
        }
        break;

        case OMX_IndexParamVideoIntraRefresh:
        {
            pVideoIntraRefresh = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE*) ComponentParameterStructure;
            PortIndex = pVideoIntraRefresh->nPortIndex;
            /*Check Structure Header and verify component state*/
            ErrorType = ParameterSanityCheck(hComponent, PortIndex, pVideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error param check failed"));
                return ErrorType;
            }
            oscl_memcpy(&ipPorts[PortIndex]->VideoIntraRefresh, pVideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
        }
        break;


        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter error Unsupported index"));
            return OMX_ErrorUnsupportedIndex;
        }
        // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentVideo : SetParameter OUT"));
    return ErrorType;

}
#if PROXY_INTERFACE

/** Component entry points declarations with proxy interface*/
OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyGetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyGetConfig(hComponent, nIndex, pComponentConfigStructure);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxySetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxySetConfig(hComponent, nIndex, pComponentConfigStructure);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyGetExtensionIndex(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyGetExtensionIndex(hComponent, cParameterName, pIndexType);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyGetState(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE* pState)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyGetState(hComponent, pState);
    return ReturnValue;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyGetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue = OMX_ErrorNone;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyGetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    return ReturnValue;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentProxySetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxySetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    return ReturnValue;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyUseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyUseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyAllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyAllocateBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyFreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyFreeBuffer(hComponent, nPortIndex, pBuffer);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxySetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxySetCallbacks(hComponent, pCallbacks, pAppData);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxySendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam,
    OMX_IN  OMX_PTR pCmdData)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxySendCommand(hComponent, Cmd, nParam, pCmdData);
    return ReturnValue;
}

OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyEmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyEmptyThisBuffer(hComponent, pBuffer);
    return ReturnValue;
}


OMX_ERRORTYPE OmxComponentBase::BaseComponentProxyFillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxComponentBase* pOpenmaxAOType = (OmxComponentBase*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE ReturnValue;

    ReturnValue = ((ProxyApplication_OMX*)(pOpenmaxAOType->ipComponentProxy))->ProxyFillThisBuffer(hComponent, pBuffer);
    return ReturnValue;
}

#endif // PROXY_INTERFACE
