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
#include "oscl_base.h"
#include "pv_omxdefs.h"
#include "omx_mp3_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#define OMX_HALFRANGE_THRESHOLD 0x7FFFFFFF

OSCL_DLL_ENTRY_POINT_DEFAULT()

// This function is called by OMX_GetHandle and it creates an instance of the mp3 component AO
OSCL_EXPORT_REF OMX_ERRORTYPE Mp3OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    OpenmaxMp3AO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitMp3OmxComponentFields content to actual constructor

    pOpenmaxAOType = (OpenmaxMp3AO*) OSCL_NEW(OpenmaxMp3AO, ());

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
OSCL_EXPORT_REF OMX_ERRORTYPE Mp3OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OpenmaxMp3AO* pOpenmaxAOType = (OpenmaxMp3AO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}
#if DYNAMIC_LOAD_OMX_MP3_COMPONENT
class Mp3OmxSharedLibraryInterface: public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface
{
    public:
        static Mp3OmxSharedLibraryInterface *Instance()
        {
            static Mp3OmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_MP3DEC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mp3OmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mp3OmxComponentDestructor));
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
        Mp3OmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return Mp3OmxSharedLibraryInterface::Instance();
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

OMX_ERRORTYPE OpenmaxMp3AO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType* pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data

    ipComponentProxy = pProxy;


#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OpenmaxMp3AO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OpenmaxMp3AO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OpenmaxMp3AO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OpenmaxMp3AO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OpenmaxMp3AO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxMp3AO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxMp3AO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OpenmaxMp3AO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxMp3AO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxMp3AO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxMp3AO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxMp3AO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OpenmaxMp3AO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OpenmaxMp3AO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OpenmaxMp3AO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OpenmaxMp3AO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OpenmaxMp3AO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxMp3AO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxMp3AO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OpenmaxMp3AO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxMp3AO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxMp3AO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxMp3AO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxMp3AO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OpenmaxMp3AO::BaseComponentSetCallbacks;
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

    /** Domain specific section for the ports. */
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainAudio;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.cMIMEType = (OMX_STRING)"audio/mpeg";
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.pNativeRender = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingMP3;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_MP3;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_MP3;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainAudio;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.cMIMEType = (OMX_STRING)"raw";
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.pNativeRender = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDir = OMX_DirOutput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_MP3;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_MP3 * 6;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;

    //Default values for Mp3 audio param port
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.nChannels = 2;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.nBitRate = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.nSampleRate = 44100;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.nAudioBandWidth = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.eChannelMode = OMX_AUDIO_ChannelModeStereo;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param.eFormat = OMX_AUDIO_MP3StreamFormatMP1Layer3;

    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioEqualizerType.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioEqualizerType.sBandIndex.nMin = 0;
    //Taken these value from from mp3 decoder component
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioEqualizerType.sBandIndex.nValue = (e_equalization) flat;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioEqualizerType.sBandIndex.nMax = (e_equalization) flat_;


    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels = 2;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.eNumData = OMX_NumericalDataSigned;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.bInterleaved = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nBitPerSample = 16;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nSamplingRate = 44100;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    iPortTypesParam.nPorts = 2;
    iPortTypesParam.nStartPortNumber = 0;

    pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    pOutPort = (ComponentPortType*) ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    SetHeader(&pInPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pInPort->AudioParam.nPortIndex = 0;
    pInPort->AudioParam.nIndex = 0;
    pInPort->AudioParam.eEncoding = OMX_AUDIO_CodingMP3;

    SetHeader(&pOutPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pOutPort->AudioParam.nPortIndex = 1;
    pOutPort->AudioParam.nIndex = 0;
    pOutPort->AudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    iOutputFrameLength = OUTPUT_BUFFER_SIZE_MP3;

    if (ipMp3Dec)
    {
        OSCL_DELETE(ipMp3Dec);
        ipMp3Dec = NULL;
    }

    ipMp3Dec = OSCL_NEW(Mp3Decoder, ());
    if (ipMp3Dec == NULL)
    {
        return OMX_ErrorInsufficientResources;
    }

    oscl_memset(ipMp3Dec, 0, sizeof(Mp3Decoder));

    iSamplesPerFrame = DEFAULT_SAMPLES_PER_FRAME_MP3;
    iOutputMilliSecPerFrame = iCurrentFrameTS.GetFrameDuration();

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
	*/

OMX_ERRORTYPE OpenmaxMp3AO::DestroyComponent()
{
    if (iIsInit != OMX_FALSE)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipMp3Dec)
    {
        OSCL_DELETE(ipMp3Dec);
        ipMp3Dec = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}


void OpenmaxMp3AO::SyncWithInputTimestamp()
{
    //Do not check for silence insertion if the clip is repositioned
    if (OMX_FALSE == iRepositionFlag)
    {
        CheckForSilenceInsertion();
    }

    /* Set the current timestamp equal to input buffer timestamp in case of
     * a) All input frames
     * b) First frame after repositioning */
    if (OMX_FALSE == iSilenceInsertionInProgress || OMX_TRUE == iRepositionFlag)
    {
        // Set the current timestamp equal to input buffer timestamp
        iCurrentFrameTS.SetFromInputTimestamp(iFrameTimestamp);

        //Reset the flag back to false, once timestamp has been updated from input frame
        if (OMX_TRUE == iRepositionFlag)
        {
            iRepositionFlag = OMX_FALSE;
        }
    }
}

void OpenmaxMp3AO::ResetComponent()
{
    // reset decoder
    if (ipMp3Dec)
    {
        ipMp3Dec->ResetDecoder();
        //Set this length to zero for flushing the current input buffer
        ipMp3Dec->iInputUsedLength = 0;
        //ipMp3Dec->iInitFlag = 0;
    }
}


void OpenmaxMp3AO::ProcessInBufferFlag()
{
    iIsInputBufferEnded = OMX_FALSE;
}


void OpenmaxMp3AO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData IN"));

    QueueType* pInputQueue  = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType* pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;

    OMX_U8*	pOutBuffer;
    OMX_U32	OutputLength;
    OMX_S32 DecodeReturn;
    OMX_BOOL ResizeNeeded = OMX_FALSE;

    OMX_U32 TempInputBufferSize = (2 * sizeof(uint8) * (ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize));

    if ((!iIsInputBufferEnded) || iEndofStream)
    {
        if (OMX_TRUE == iSilenceInsertionInProgress)
        {
            DoSilenceInsertion();
            //If the flag is still true, come back to this routine again
            if (OMX_TRUE == iSilenceInsertionInProgress)
            {
                return;
            }
        }

        //Check whether prev output bufer has been released or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData Error, Output Buffer Dequeue returned NULL, OUT"));
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentFrameTS.GetConvertedTs();

            // Copy the output buffer that was stored locally before dynamic port reconfiguration
            // in the new omx buffer received.
            if (OMX_TRUE == iSendOutBufferAfterPortReconfigFlag)
            {
                if ((ipTempOutBufferForPortReconfig)
                        && (iSizeOutBufferForPortReconfig <= ipOutputBuffer->nAllocLen))
                {
                    oscl_memcpy(ipOutputBuffer->pBuffer, ipTempOutBufferForPortReconfig, iSizeOutBufferForPortReconfig);
                    ipOutputBuffer->nFilledLen = iSizeOutBufferForPortReconfig;
                    ipOutputBuffer->nTimeStamp = iTimestampOutBufferForPortReconfig;
                }

                iSendOutBufferAfterPortReconfigFlag = OMX_FALSE;

                //Send the output buffer back only when it has become full
                if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < iOutputFrameLength)
                {
                    ReturnOutputBuffer(ipOutputBuffer, pOutPort);
                }

                //Free the temp output buffer
                if (ipTempOutBufferForPortReconfig)
                {
                    oscl_free(ipTempOutBufferForPortReconfig);
                    ipTempOutBufferForPortReconfig = NULL;
                    iSizeOutBufferForPortReconfig = 0;
                }

                //Dequeue new output buffer if required to continue decoding the next frame
                if (OMX_TRUE == iNewOutBufRequired)
                {
                    if (0 == (GetQueueNumElem(pOutputQueue)))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData OUT, output buffer unavailable"));
                        return;
                    }

                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
                    if (NULL == ipOutputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData Error, Output Buffer Dequeue returned NULL, OUT"));
                        return;
                    }

                    ipOutputBuffer->nFilledLen = 0;
                    iNewOutBufRequired = OMX_FALSE;

                    ipOutputBuffer->nTimeStamp = iCurrentFrameTS.GetConvertedTs();
                }
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

        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        OutputLength = 0;

        /* Copy the left-over data from last input buffer that is stored in temporary
         * buffer to the next incoming buffer.
         */
        if (iTempInputBufferLength > 0 &&
                ((iInputCurrLength + iTempInputBufferLength) < TempInputBufferSize))
        {
            oscl_memcpy(&ipTempInputBuffer[iTempInputBufferLength], ipFrameDecodeBuffer, iInputCurrLength);
            iInputCurrLength += iTempInputBufferLength;
            iTempInputBufferLength = 0;
            ipFrameDecodeBuffer = ipTempInputBuffer;
        }

        //Output buffer is passed as a short pointer
        DecodeReturn = ipMp3Dec->Mp3DecodeAudio((OMX_S16*) pOutBuffer,
                                                (OMX_U32*) & OutputLength,
                                                &(ipFrameDecodeBuffer),
                                                &iInputCurrLength,
                                                &iFrameCount,
                                                &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode),
                                                &(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioMp3Param),
                                                iEndOfFrameFlag,
                                                &ResizeNeeded);

        if (ResizeNeeded == OMX_TRUE)
        {
            if (0 != OutputLength)
            {
                iOutputFrameLength = OutputLength * 2;

                //Update the timestamp
                iSamplesPerFrame = OutputLength / ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels;

                iCurrentFrameTS.SetParameters(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nSamplingRate, iSamplesPerFrame);
                iOutputMilliSecPerFrame = iCurrentFrameTS.GetFrameDuration();

            }

            // set the flag to disable further processing until Client reacts to this
            //	by doing dynamic port reconfiguration
            iResizePending = OMX_TRUE;

            /* Do not return the output buffer generated yet, store it locally
             * and wait for the dynamic port reconfig to complete */
            if ((NULL == ipTempOutBufferForPortReconfig))
            {
                ipTempOutBufferForPortReconfig = (OMX_U8*) oscl_malloc(sizeof(uint8) * OutputLength * 2);
                if (NULL == ipTempOutBufferForPortReconfig)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData error, insufficient resources"));
                    return;
                }
            }

            //Copy the omx output buffer to the temporary internal buffer
            oscl_memcpy(ipTempOutBufferForPortReconfig, pOutBuffer, OutputLength * 2);
            iSizeOutBufferForPortReconfig = OutputLength * 2;

            //Set the current timestamp to the output buffer timestamp for the first output frame
            //Later it will be done at the time of dequeue
            iTimestampOutBufferForPortReconfig = iCurrentFrameTS.GetConvertedTs();

            iCurrentFrameTS.UpdateTimestamp(iSamplesPerFrame);
            //Make this length 0 so that no output buffer is returned by the component
            OutputLength = 0;

            // send port settings changed event
            OMX_COMPONENTTYPE* pHandle = (OMX_COMPONENTTYPE*) ipAppPriv->CompHandle;

            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventPortSettingsChanged, //The command was completed
             OMX_PORT_OUTPUTPORT_INDEX,
             0,
             NULL);
        }

        ipOutputBuffer->nFilledLen += OutputLength * 2;
        //offset not required in our case, set it to zero
        ipOutputBuffer->nOffset = 0;

        if (OutputLength > 0)
        {
            iCurrentFrameTS.UpdateTimestamp(iSamplesPerFrame);
        }

        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            /* Changed the condition here. If EOS has come and decoder can't decode
             * the buffer, do not wait for buffer length to become zero and return
             * the callback as the current buffer may contain partial frame and
             * no other data is going to come now
             */
            if (MP3DEC_SUCCESS != DecodeReturn)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData EOS callback send"));

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
                ipOutputBuffer = NULL;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData OUT"));

                return;
            }
        }


        if (MP3DEC_SUCCESS == DecodeReturn)
        {
            ipInputBuffer->nFilledLen = iInputCurrLength;
        }
        else if (MP3DEC_INCOMPLETE_FRAME == DecodeReturn)
        {
            /* If decoder returns MP4AUDEC_INCOMPLETE_FRAME,
             * this indicates the input buffer contains less than a frame data
             * Copy it to a temp buffer to be used in next decode call
             */
            oscl_memcpy(ipTempInputBuffer, ipFrameDecodeBuffer, iInputCurrLength);
            iTempInputBufferLength = iInputCurrLength;
            ipInputBuffer->nFilledLen = 0;
            iInputCurrLength = 0;
        }
        else
        {
            //bitstream error, discard the current data as it can't be decoded further
            ipInputBuffer->nFilledLen = 0;
            iInputCurrLength = 0;

            //Report it to the client via a callback
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData ErrorStreamCorrupt callback send"));

            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventError,
             OMX_ErrorStreamCorrupt,
             0,
             NULL);

        }

        //Return the input buffer if it has been consumed fully by the decoder
        if (0 == ipInputBuffer->nFilledLen)
        {
            ReturnInputBuffer(ipInputBuffer, pInPort);
            ipInputBuffer = NULL;
            iIsInputBufferEnded = OMX_TRUE;
            iInputCurrLength = 0;
        }

        //Send the output buffer back when it has become full
        if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < (iOutputFrameLength))
        {
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
        }

        /* If there is some more processing left with current buffers, re-schedule the AO
         * Do not go for more than one round of processing at a time.
         * This may block the AO longer than required.
         */
        if (((iInputCurrLength != 0 || GetQueueNumElem(pInputQueue) > 0)
                && (GetQueueNumElem(pOutputQueue) > 0) && (ResizeNeeded == OMX_FALSE))
                || (OMX_TRUE == iEndofStream))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ProcessData OUT"));
    return;
}


void OpenmaxMp3AO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"audio_decoder.mp3";
}


//Active object constructor
OpenmaxMp3AO::OpenmaxMp3AO()
{
    ipMp3Dec = NULL;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : constructed"));
}


//Active object destructor
OpenmaxMp3AO::~OpenmaxMp3AO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : destructed"));
}


/** The Initialization function
 */
OMX_ERRORTYPE OpenmaxMp3AO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ComponentInit IN"));

    OMX_BOOL Status = OMX_FALSE;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;

    //Mp3 lib init
    if (!iCodecReady)
    {
        Status = ipMp3Dec->Mp3DecInit(&(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioEqualizerType));
        iCodecReady = OMX_TRUE;
    }

    iInputCurrLength = 0;
    //Used in dynamic port reconfiguration
    iFrameCount = 0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ComponentInit OUT"));

    if (OMX_TRUE == Status)
    {
        return OMX_ErrorNone;
    }
    else
    {
        return OMX_ErrorInvalidComponent;
    }
}

/** This function is called upon a transition to the idle or invalid state.
 *  Also it is called by the Mp3ComponentDestructor() function
 */
OMX_ERRORTYPE OpenmaxMp3AO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ComponentDeInit IN"));

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        ipMp3Dec->Mp3DecDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : ComponentDeInit OUT"));

    return OMX_ErrorNone;

}


//Check whether silence insertion is required here or not
void OpenmaxMp3AO::CheckForSilenceInsertion()
{

    OMX_TICKS CurrTimestamp, TimestampGap;
    //Set the flag to false by default
    iSilenceInsertionInProgress = OMX_FALSE;

    CurrTimestamp = iCurrentFrameTS.GetCurrentTimestamp();
    TimestampGap = iFrameTimestamp - CurrTimestamp;

    if ((TimestampGap > OMX_HALFRANGE_THRESHOLD) || (TimestampGap < iOutputMilliSecPerFrame && iFrameCount > 0))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : CheckForSilenceInsertion OUT - No need to insert silence"));
        return;
    }

    //Silence insertion needed, mark the flag to true
    if (iFrameCount > 0)
    {
        iSilenceInsertionInProgress = OMX_TRUE;
        //Determine the number of silence frames to insert
        if (0 != iOutputMilliSecPerFrame)
        {
            iSilenceFramesNeeded = TimestampGap / iOutputMilliSecPerFrame;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : CheckForSilenceInsertion OUT - Silence Insertion required here"));
    }

    return;
}

//Currently we are doing zero frame insertion in this routine
void OpenmaxMp3AO::DoSilenceInsertion()
{
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_U8*	pOutBuffer = NULL;


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : DoSilenceInsertion IN"));

    while (iSilenceFramesNeeded > 0)
    {
        //Check whether prev output bufer has been consumed or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                //Resume Silence insertion next time when component will be called
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : DoSilenceInsertion OUT output buffer unavailable"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : DoSilenceInsertion Error, Output Buffer Dequeue returned NULL, OUT"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentFrameTS.GetConvertedTs();
        }


        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        oscl_memset(pOutBuffer, 0, iOutputFrameLength);

        ipOutputBuffer->nFilledLen += iOutputFrameLength;
        ipOutputBuffer->nOffset = 0;
        iCurrentFrameTS.UpdateTimestamp(iSamplesPerFrame);

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : DoSilenceInsertion - One frame of zeros inserted"));

        //Send the output buffer back when it has become full
        if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < iOutputFrameLength)
        {
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
        }

        // Decrement the silence frame counter
        --iSilenceFramesNeeded;
    }

    /* Completed Silence insertion successfully, now consider the input buffer already dequeued
     * for decoding & update the timestamps */

    iSilenceInsertionInProgress = OMX_FALSE;
    iCurrentFrameTS.SetFromInputTimestamp(iFrameTimestamp);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMp3AO : DoSilenceInsertion OUT - Done successfully"));

    return;
}


OMX_ERRORTYPE OpenmaxMp3AO::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(nIndex);
    OSCL_UNUSED_ARG(pComponentConfigStructure);
    return OMX_ErrorNotImplemented;
}
