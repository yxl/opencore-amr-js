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

#include "omx_amrenc_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif



// This function is called by OMX_GetHandle and it creates an instance of the amr component AO
OMX_ERRORTYPE AmrEncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    OmxComponentAmrEncoderAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitAmrOmxComponentFields content to actual constructor

    pOpenmaxAOType = (OmxComponentAmrEncoderAO*) OSCL_NEW(OmxComponentAmrEncoderAO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
    ///////////////////////////////////////////////////////////////////////////////////////
}

// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OMX_ERRORTYPE AmrEncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OmxComponentAmrEncoderAO* pOpenmaxAOType = (OmxComponentAmrEncoderAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up encoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if DYNAMIC_LOAD_OMX_AMRENC_COMPONENT
class AmrEncOmxSharedLibraryInterface:  public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static AmrEncOmxSharedLibraryInterface *Instance()
        {
            static AmrEncOmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_AMRENC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AmrEncOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AmrEncOmxComponentDestructor));
                }
            }
            return NULL;
        };

        OsclAny *SharedLibraryLookup(const OsclUuid& aInterfaceId)
        {
            if (aInterfaceId == PV_OMX_SHARED_INTERFACE)
            {
                return OSCL_STATIC_CAST(OmxSharedLibraryInterface*, this);
            }
            return NULL;
        };

    private:
        AmrEncOmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return AmrEncOmxSharedLibraryInterface::Instance();
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

OMX_ERRORTYPE OmxComponentAmrEncoderAO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType* pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iCompressedFormatPortNum = OMX_PORT_OUTPUTPORT_INDEX;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    ipComponentProxy = pProxy;
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data


#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OmxComponentAmrEncoderAO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OmxComponentAmrEncoderAO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OmxComponentAmrEncoderAO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OmxComponentAmrEncoderAO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OmxComponentAmrEncoderAO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentAmrEncoderAO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentAmrEncoderAO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OmxComponentAmrEncoderAO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentAmrEncoderAO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentAmrEncoderAO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentAmrEncoderAO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentAmrEncoderAO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OmxComponentAmrEncoderAO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OmxComponentAmrEncoderAO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OmxComponentAmrEncoderAO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OmxComponentAmrEncoderAO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OmxComponentAmrEncoderAO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentAmrEncoderAO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentAmrEncoderAO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OmxComponentAmrEncoderAO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentAmrEncoderAO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentAmrEncoderAO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentAmrEncoderAO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentAmrEncoderAO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OmxComponentAmrEncoderAO::BaseComponentSetCallbacks;
    iOmxComponent.nVersion.s.nVersionMajor = SPECVERSIONMAJOR;
    iOmxComponent.nVersion.s.nVersionMinor = SPECVERSIONMINOR;
    iOmxComponent.nVersion.s.nRevision = SPECREVISION;
    iOmxComponent.nVersion.s.nStep = SPECSTEP;

    // PV capability
    iPVCapabilityFlags.iOMXComponentSupportsExternalInputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsExternalOutputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsMovableInputBuffers = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsPartialFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesNALStartCodes = OMX_FALSE;
    iPVCapabilityFlags.iOMXComponentCanHandleIncompleteFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames = OMX_FALSE;

    if (ipAppPriv)
    {
        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    ipAppPriv = (ComponentPrivateType*) oscl_malloc(sizeof(ComponentPrivateType));
    if (NULL == ipAppPriv)
    {
        return OMX_ErrorInsufficientResources;
    }

    //Construct base class now
    Status = ConstructBaseComponent(pAppData);

    if (OMX_ErrorNone != Status)
    {
        return Status;
    }

    /** Domain specific section for the ports */
    /* Input port is raw/pcm for AMR encoder */
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainAudio;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.cMIMEType = (OMX_STRING)"raw";
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.pNativeRender = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_AMR_ENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_AMR_ENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    /* Output port is amr format for AMR encoder */
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainAudio;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.cMIMEType = (OMX_STRING)"audio/mpeg";
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.pNativeRender = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingAMR;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDir = OMX_DirOutput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_AMR_ENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_AMR_ENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;

    //Default values for PCM input audio param port
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.nChannels = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.eNumData = OMX_NumericalDataSigned;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.bInterleaved = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.nBitPerSample = 16;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.nSamplingRate = 8000;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    //Default values for AMR output audio param port
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.nChannels = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.nBitRate = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.eAMRBandMode = OMX_AUDIO_AMRBandModeNB7;	//AMRNB Mode 7 = 12200 bps
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.eAMRDTXMode = OMX_AUDIO_AMRDTXModeOnVAD1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;	//PVMF_AMR_IETF

    iPortTypesParam.nPorts = 2;
    iPortTypesParam.nStartPortNumber = 0;

    pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    pOutPort = (ComponentPortType*) ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    SetHeader(&pInPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pInPort->AudioParam.nPortIndex = 0;
    pInPort->AudioParam.nIndex = 0;
    pInPort->AudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    SetHeader(&pOutPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pOutPort->AudioParam.nPortIndex = 1;
    pOutPort->AudioParam.nIndex = 0;
    pOutPort->AudioParam.eEncoding = OMX_AUDIO_CodingAMR;

    iInputBufferRemainingBytes = 0;

    if (ipAmrEnc)
    {
        OSCL_DELETE(ipAmrEnc);
        ipAmrEnc = NULL;
    }

    ipAmrEnc = OSCL_NEW(OmxAmrEncoder, ());
    if (NULL == ipAmrEnc)
    {
        return OMX_ErrorInsufficientResources;
    }


#if PROXY_INTERFACE

    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentSendCommand = BaseComponentSendCommand;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentGetParameter = BaseComponentGetParameter;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentSetParameter = BaseComponentSetParameter;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentGetConfig = BaseComponentGetConfig;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentSetConfig = BaseComponentSetConfig;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentGetExtensionIndex = BaseComponentGetExtensionIndex;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentGetState = BaseComponentGetState;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentUseBuffer = BaseComponentUseBuffer;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentAllocateBuffer = BaseComponentAllocateBuffer;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentFreeBuffer = BaseComponentFreeBuffer;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentEmptyThisBuffer = BaseComponentEmptyThisBuffer;
    ((ProxyApplication_OMX*)ipComponentProxy)->ComponentFillThisBuffer = BaseComponentFillThisBuffer;

#endif
    return OMX_ErrorNone;
}


/** This function is called by the omx core when the component
	* is disposed by the IL client with a call to FreeHandle().
	* \param Component, the component to be disposed
	*/

OMX_ERRORTYPE OmxComponentAmrEncoderAO::DestroyComponent()
{
    if (iIsInit != OMX_FALSE)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipAmrEnc)
    {
        OSCL_DELETE(ipAmrEnc);
        ipAmrEnc = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}



/* This routine will extract the input timestamp from the input buffer */
void OmxComponentAmrEncoderAO::SyncWithInputTimestamp()
{
    iCurrentTimestamp = iFrameTimestamp;
}


void OmxComponentAmrEncoderAO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData IN"));

    QueueType* pInputQueue  = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType* pInPort  = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_COMPONENTTYPE* pHandle  = &iOmxComponent;

    OMX_U8*	 pOutBuffer;
    OMX_U32	 OutputLength;
    OMX_S32  EncodeReturn;
    OMX_U32  RemainderInputBytes = 0;
    OMX_TICKS OutputTimeStamp;

    OMX_U32 TempInputBufferSize = (2 * sizeof(uint8) * (ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize));

    OMX_U32 AllocNumberOutputFrames;
    OMX_U32 ExtraInputBytes = 0;

    if ((!iIsInputBufferEnded) || iEndofStream)
    {
        //Check whether prev output bufer has been released or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

            OSCL_ASSERT(NULL != ipOutputBuffer);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentAmrEncoderAO : ProcessData ERROR - OUT buffer cannot be dequeued"));

                return;
            }

            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //At the first frame count, detrermine the output parameters
            if (0 == iFrameCount)
            {
                //How many frames can be accomodated in the output buffer
                AllocNumberOutputFrames = ipOutputBuffer->nAllocLen / MAX_AMR_FRAME_SIZE;
                iActualNumberOutputFrames = omx_min(AllocNumberOutputFrames, iMaxNumberOutputFrames);

                /* Keep the minimum of the two:
                	-frames accomodated and
                	-maximum frames defined by component */
                iOutputFrameLength = iActualNumberOutputFrames * MAX_AMR_FRAME_SIZE;
            }
        }

        /* Code for the marking buffer. Takes care of the OMX_CommandMarkBuffer
         * command and hMarkTargetComponent as given by the specifications
         */
        if (ipMark != NULL)
        {
            ipOutputBuffer->hMarkTargetComponent = ipMark->hMarkTargetComponent;
            ipOutputBuffer->pMarkData = ipMark->pMarkData;
            ipMark = NULL;
        }

        if (ipTargetComponent != NULL)
        {
            ipOutputBuffer->hMarkTargetComponent = ipTargetComponent;
            ipOutputBuffer->pMarkData = iTargetMarkData;
            ipTargetComponent = NULL;

        }
        //Mark buffer code ends here


        if ((iTempInputBufferLength > 0) &&
                ((iInputCurrLength + iTempInputBufferLength) <= TempInputBufferSize))
        {
            oscl_memcpy(&ipTempInputBuffer[iTempInputBufferLength], ipFrameDecodeBuffer, iInputCurrLength);
            iInputCurrLength += iTempInputBufferLength;
            iTempInputBufferLength = 0;
            ipFrameDecodeBuffer = ipTempInputBuffer;
        }


        //If the number of output buffers to be produced from the current iInputCurrLength
        //are more than our requirement, send only the required data for encoding
        if (iInputCurrLength / iInputFrameLength > iActualNumberOutputFrames)
        {
            ExtraInputBytes = iInputCurrLength - (iInputFrameLength * iActualNumberOutputFrames);
            iInputCurrLength -= ExtraInputBytes;
        }
        else
        {
            /* Before sending the input buffer to the encoder, ensure that the data is multiple
             * of one amr input frame length*/
            RemainderInputBytes = iInputCurrLength % iInputFrameLength;
            iInputCurrLength -= RemainderInputBytes;
        }


        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        OutputLength = 0;

        EncodeReturn = ipAmrEnc->AmrEncodeFrame(pOutBuffer,
                                                &OutputLength,
                                                ipFrameDecodeBuffer,
                                                iInputCurrLength,
                                                iCurrentTimestamp,
                                                &OutputTimeStamp);


        //Attach the timestamp to the output buffer only when we have fetched the new output buffer
        //If we are reusing the same output buffer again, no need to modify the previous timestamp, as it should be of the first frame in that buffer

        if (0 == ipOutputBuffer->nFilledLen)
        {
            ipOutputBuffer->nTimeStamp = OutputTimeStamp;
        }


        ipOutputBuffer->nFilledLen += OutputLength;
        //offset not required in our case, set it to zero
        ipOutputBuffer->nOffset = 0;

        //It has to be incremented atleast one time, so that 'equality with zero' checks added above hold true only once
        iFrameCount++;


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if ((0 == iInputCurrLength) || (OMX_TRUE != EncodeReturn))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData EOS callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventBufferFlag,
                 1,
                 OMX_BUFFERFLAG_EOS,
                 NULL);

                iEndofStream = OMX_FALSE;

                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;

                ReturnOutputBuffer(ipOutputBuffer, pOutPort);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData OUT"));

                return;
            }
        }


        if (OMX_TRUE == EncodeReturn)
        {
            //Do not return the input buffer in case it has more than one frame data to encode
            if (ExtraInputBytes > 0)
            {
                ipFrameDecodeBuffer += iInputCurrLength;
                iInputCurrLength = ExtraInputBytes;
            }
            else
            {

                /* If there are some remainder bytes out of the last buffer, copy into a temp buffer
                 * to be used in next decode cycle and return the existing input buffer*/
                if (RemainderInputBytes > 0)
                {
                    oscl_memmove(ipTempInputBuffer, &ipFrameDecodeBuffer[iInputCurrLength], RemainderInputBytes);
                    iTempInputBufferLength = RemainderInputBytes;
                }

                //Input bytes consumed now, return the buffer
                ipInputBuffer->nFilledLen = 0;
                ReturnInputBuffer(ipInputBuffer, pInPort);
                iIsInputBufferEnded = OMX_TRUE;
                iInputCurrLength = 0;

            }
        }
        //In case of error, discard the bitstream and report data corruption error via callback
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData ErrorStreamCorrupt callback send"));

            ipInputBuffer->nFilledLen = 0;
            ReturnInputBuffer(ipInputBuffer, pInPort);
            iIsInputBufferEnded = OMX_TRUE;
            iInputCurrLength = 0;


            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventError,
             OMX_ErrorStreamCorrupt,
             0,
             NULL);
        }


        /* Send the output buffer back when it has the following conditions as true:
        1) Output buffer contain desired number of frames, calculated above
        2) Output buffer can no longer hold the desired number of frames that
           we will encode in next encode call
        */
        if ((ipOutputBuffer->nFilledLen >= iOutputFrameLength)
                || (ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < iOutputFrameLength)
        {
            //Attach the end of frame flag while sending out the output buffer
            ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
        }


        /* If there is some more processing left with current buffers, re-schedule the AO
         * Do not go for more than one round of processing at a time.
         * This may block the AO longer than required.
         */
        if (((ipInputBuffer->nFilledLen != 0) || (GetQueueNumElem(pInputQueue) > 0))
                && ((GetQueueNumElem(pOutputQueue) > 0) || (OMX_FALSE == iNewOutBufRequired)))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ProcessData OUT"));
    return;
}


//Not implemented & supported in case of base profile components

void OmxComponentAmrEncoderAO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"audio_encoder.amr";
}


//Component constructor
OmxComponentAmrEncoderAO::OmxComponentAmrEncoderAO()
{
    ipAmrEnc = NULL;
    iInputFrameLength = 0;
    iOutputFrameLength = 0;
    iActualNumberOutputFrames = 0;
    iMaxNumberOutputFrames = 0;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : constructed"));
}


//Active object destructor
OmxComponentAmrEncoderAO::~OmxComponentAmrEncoderAO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : destructed"));
}


/** The Initialization function
 */
OMX_ERRORTYPE OmxComponentAmrEncoderAO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ComponentInit IN"));

    OMX_BOOL Status = OMX_TRUE;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;

    //amr encoder lib init
    if (!iCodecReady)
    {
        Status = ipAmrEnc->AmrEncInit(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioPcmMode,
                                      ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioAmrParam,
                                      &iInputFrameLength, &iMaxNumberOutputFrames);

        iCodecReady = OMX_TRUE;
    }

    iInputCurrLength = 0;

    //Used in dynamic port reconfiguration
    iFrameCount = 0;

    if (OMX_TRUE == Status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ComponentInit OUT"));
        return OMX_ErrorNone;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : Error ComponentInit, OUT"));
        return OMX_ErrorInvalidComponent;
    }
}



/** This function is called upon a transition to the idle or invalid state.
 *  Also it is called by the ComponentDestructor() function
 */
OMX_ERRORTYPE OmxComponentAmrEncoderAO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ComponentDeInit IN"));

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        ipAmrEnc->AmrEncDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAmrEncoderAO : ComponentDeInit OUT"));

    return OMX_ErrorNone;

}


/* A component specific routine called from BufferMgmtWithoutMarker */
void OmxComponentAmrEncoderAO::ProcessInBufferFlag()
{
    iIsInputBufferEnded = OMX_FALSE;
}
