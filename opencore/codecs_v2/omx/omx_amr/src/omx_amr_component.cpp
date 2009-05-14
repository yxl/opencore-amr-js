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
#include "omx_amr_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#define OMX_HALFRANGE_THRESHOLD 0x7FFFFFFF
/**** The duration of one output AMR frame (in ms) is fixed and equal to 20ms - needed for timestamp updates ****/
/**** Note that AMR sampling rate is always 8khz, so a frame of 20 ms always corresponds to 160 (16-bit) samples = 320 bytes */
#define OMX_AMR_DEC_FRAME_INTERVAL 20

OSCL_DLL_ENTRY_POINT_DEFAULT()

// This function is called by OMX_GetHandle and it creates an instance of the amr component AO
OSCL_EXPORT_REF OMX_ERRORTYPE AmrOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    OpenmaxAmrAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitAmrOmxComponentFields content to actual constructor

    pOpenmaxAOType = (OpenmaxAmrAO*) OSCL_NEW(OpenmaxAmrAO, ());

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
OSCL_EXPORT_REF OMX_ERRORTYPE AmrOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OpenmaxAmrAO* pOpenmaxAOType = (OpenmaxAmrAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if DYNAMIC_LOAD_OMX_AMR_COMPONENT
class AmrOmxSharedLibraryInterface: public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static AmrOmxSharedLibraryInterface *Instance()
        {
            static AmrOmxSharedLibraryInterface amromxinterface;
            return &amromxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_AMRDEC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AmrOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AmrOmxComponentDestructor));
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
        AmrOmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return AmrOmxSharedLibraryInterface::Instance();
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

OMX_ERRORTYPE OpenmaxAmrAO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType* pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    ipComponentProxy = pProxy;
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data


#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OpenmaxAmrAO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OpenmaxAmrAO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OpenmaxAmrAO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OpenmaxAmrAO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OpenmaxAmrAO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxAmrAO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxAmrAO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OpenmaxAmrAO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxAmrAO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxAmrAO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxAmrAO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxAmrAO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OpenmaxAmrAO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OpenmaxAmrAO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OpenmaxAmrAO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OpenmaxAmrAO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OpenmaxAmrAO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxAmrAO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxAmrAO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OpenmaxAmrAO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxAmrAO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxAmrAO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxAmrAO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxAmrAO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OpenmaxAmrAO::BaseComponentSetCallbacks;
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
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingAMR;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_AMR;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_AMR;
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
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_AMR;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_AMR * 6;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;

    //Default values for AMR audio param port
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.nChannels = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.nBitRate = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRBandMode = OMX_AUDIO_AMRBandModeNB0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRDTXMode = OMX_AUDIO_AMRDTXModeOnVAD1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatConformance;

    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.eNumData = OMX_NumericalDataSigned;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.bInterleaved = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nBitPerSample = 16;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nSamplingRate = 8000;
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
    pInPort->AudioParam.eEncoding = OMX_AUDIO_CodingAMR;

    SetHeader(&pOutPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pOutPort->AudioParam.nPortIndex = 1;
    pOutPort->AudioParam.nIndex = 0;
    pOutPort->AudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    iInputBufferRemainingBytes = 0;

    if (ipAmrDec)
    {
        OSCL_DELETE(ipAmrDec);
        ipAmrDec = NULL;
    }

    ipAmrDec = OSCL_NEW(OmxAmrDecoder, ());
    if (NULL == ipAmrDec)
    {
        return OMX_ErrorInsufficientResources;
    }

    //Commented memset here as some default values are set in the constructor
    //oscl_memset(ipAmrDec, 0, sizeof (OmxAmrDecoder));

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

OMX_ERRORTYPE OpenmaxAmrAO::DestroyComponent()
{
    if (iIsInit != OMX_FALSE)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipAmrDec)
    {
        OSCL_DELETE(ipAmrDec);
        ipAmrDec = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}


/* This routine will extract the input timestamp, verify whether silence insertion
 * is required or not and also handle the various tasks associated with repositioning */
void OpenmaxAmrAO::SyncWithInputTimestamp()
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
        iCurrentTimestamp = iFrameTimestamp;

        //Reset the flag back to false, once timestamp has been updated from input frame
        if (OMX_TRUE == iRepositionFlag)
        {
            iRepositionFlag = OMX_FALSE;
        }
    }
}


/* Reset the decoder library in case of repositioning or flush command or
 * state transition (Executing ->Idle) */
void OpenmaxAmrAO::ResetComponent()
{
    // reset decoder
    if (ipAmrDec)
    {
        ipAmrDec->ResetDecoder();
    }
}

/* A component specific routine called from BufferMgmtWithoutMarker */
void OpenmaxAmrAO::ProcessInBufferFlag()
{
    /* Used in timestamp calculation
     * Since we copy one buffer in advance, so let the first buffer finish up
     * before applying 2nd buffers timestamp into the output timestamp */
    if (iInputBufferRemainingBytes <= 0)
    {
        if (0 == iFrameCount)
        {
            iPreviousFrameLength = ipInputBuffer->nFilledLen;
            iCurrentTimestamp = iFrameTimestamp;
        }

        iInputBufferRemainingBytes += iPreviousFrameLength;
    }

    iPreviousFrameLength = ipInputBuffer->nFilledLen;
}


/* This function will be called in case of buffer management without marker bit present
 * The purpose is to copy the current input buffer into a big temporary buffer, so that
 * an incomplete/partial frame is never passed to the decoder library for decode
 */
void OpenmaxAmrAO::ComponentBufferMgmtWithoutMarker()
{
    //This common routine has been written in the base class
    TempInputBufferMgmtWithoutMarker();
}


void OpenmaxAmrAO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ProcessData IN"));
    if (!iEndOfFrameFlag)
    {
        DecodeWithoutMarker();
    }
    else
    {
        DecodeWithMarker();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ProcessData OUT"));
}


void OpenmaxAmrAO::DecodeWithoutMarker()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_COMPONENTTYPE  *pHandle = &iOmxComponent;

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    OMX_U8*					pTempInBuffer;
    OMX_U32					TempInLength;
    OMX_BOOL				ResizeNeeded = OMX_FALSE;
    OMX_BOOL				DecodeReturn = OMX_FALSE;

    OMX_U32 TempInputBufferSize = (2 * sizeof(uint8) * (ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize));


    if ((!iIsInputBufferEnded) || iEndofStream)
    {
        //Check whether prev output bufer has been released or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker OUT output buffer unavailable"));
                //Store the mark data for output buffer, as it will be overwritten next time
                if (NULL != ipTargetComponent)
                {
                    ipTempTargetComponent = ipTargetComponent;
                    iTempTargetMarkData = iTargetMarkData;
                    iMarkPropagate = OMX_TRUE;
                }
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker Error, Output Buffer Dequeue returned NULL, OUT"));
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentTimestamp;

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
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker OUT output buffer unavailable"));
                        return;
                    }

                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
                    if (NULL == ipOutputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker Error, Output Buffer Dequeue returned NULL, OUT"));
                        return;
                    }

                    ipOutputBuffer->nFilledLen = 0;
                    iNewOutBufRequired = OMX_FALSE;

                    ipOutputBuffer->nTimeStamp = iCurrentTimestamp;
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

        if ((OMX_TRUE == iMarkPropagate) && (ipTempTargetComponent != ipTargetComponent))
        {
            ipOutputBuffer->hMarkTargetComponent = ipTempTargetComponent;
            ipOutputBuffer->pMarkData = iTempTargetMarkData;
            ipTempTargetComponent = NULL;
            iMarkPropagate = OMX_FALSE;
        }
        else if (ipTargetComponent != NULL)
        {
            ipOutputBuffer->hMarkTargetComponent = ipTargetComponent;
            ipOutputBuffer->pMarkData = iTargetMarkData;
            ipTargetComponent = NULL;
            iMarkPropagate = OMX_FALSE;

        }
        //Mark buffer code ends here

        if (iTempInputBufferLength > 0)
        {
            pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
            OutputLength = 0;

            pTempInBuffer = ipTempInputBuffer + iTempConsumedLength;
            TempInLength = iTempInputBufferLength;

            //Output buffer is passed as a short pointer
            DecodeReturn = ipAmrDec->AmrDecodeFrame((OMX_S16*) pOutBuffer,
                                                    (OMX_U32*) & OutputLength,
                                                    &(pTempInBuffer),
                                                    &TempInLength,
                                                    &iFrameCount,
                                                    &ResizeNeeded);


            //If decoder returned error, report it to the client via a callback
            if (OMX_FALSE == DecodeReturn && OMX_FALSE == iEndofStream)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker ErrorStreamCorrupt callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventError,
                 OMX_ErrorStreamCorrupt,
                 0,
                 NULL);
            }

            if (ResizeNeeded == OMX_TRUE)
            {
                if (0 != OutputLength)
                {
                    iOutputFrameLength = OutputLength;
                }

                iResizePending = OMX_TRUE;

                /* Do not return the output buffer generated yet, store it locally
                 * and wait for the dynamic port reconfig to complete */
                if ((NULL == ipTempOutBufferForPortReconfig))
                {
                    ipTempOutBufferForPortReconfig = (OMX_U8*) oscl_malloc(sizeof(uint8) * OutputLength);
                    if (NULL == ipTempOutBufferForPortReconfig)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker error, insufficient resources"));
                        return;
                    }
                }

                //Copy the omx output buffer to the temporary internal buffer
                oscl_memcpy(ipTempOutBufferForPortReconfig, pOutBuffer, OutputLength);
                iSizeOutBufferForPortReconfig = OutputLength;

                iTimestampOutBufferForPortReconfig = iCurrentTimestamp;

                iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;
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

            ipOutputBuffer->nFilledLen += OutputLength;

            //offset not required in our case, set it to zero
            ipOutputBuffer->nOffset = 0;

            if (OutputLength > 0)
            {
                iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;
            }

            iTempConsumedLength += (iTempInputBufferLength - TempInLength);
            iInputBufferRemainingBytes -= (iTempInputBufferLength - TempInLength);

            iTempInputBufferLength = TempInLength;

            if (iInputBufferRemainingBytes <= 0)
            {
                iCurrentTimestamp = iFrameTimestamp;
            }

            //Do not decode if big buffer is less than half the size
            if (TempInLength < (TempInputBufferSize >> 1))
            {
                iIsInputBufferEnded = OMX_TRUE;
                iNewInBufferRequired = OMX_TRUE;
            }
        }


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if ((0 == iTempInputBufferLength) || (OMX_FALSE == DecodeReturn))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker EOS callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventBufferFlag,
                 1,
                 OMX_BUFFERFLAG_EOS,
                 NULL);

                iNewInBufferRequired = OMX_TRUE;
                iEndofStream = OMX_FALSE;

                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                ReturnOutputBuffer(ipOutputBuffer, pOutPort);
                ipOutputBuffer = NULL;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker OUT"));

                return;
            }
        }

        //Send the output buffer back after decode
        if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < (iOutputFrameLength))
        {
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
        }

        /* If there is some more processing left with current buffers, re-schedule the AO
         * Do not go for more than one round of processing at a time.
         * This may block the AO longer than required.
         */
        if ((iTempInputBufferLength != 0 || GetQueueNumElem(pInputQueue) > 0)
                && (GetQueueNumElem(pOutputQueue) > 0) && (ResizeNeeded == OMX_FALSE))

        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithoutMarker OUT"));
    return;
}


void OpenmaxAmrAO::DecodeWithMarker()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType*	pInPort = ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    OMX_BOOL				DecodeReturn = OMX_FALSE;
    OMX_COMPONENTTYPE*		pHandle = &iOmxComponent;
    OMX_BOOL				ResizeNeeded = OMX_FALSE;

    if ((!iIsInputBufferEnded) || (iEndofStream))
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker Error, Output Buffer Dequeue returned NULL, OUT"));
                return;
            }

            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentTimestamp;

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
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO :DecodeWithMarker OUT output buffer unavailable"));
                        return;
                    }

                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
                    if (NULL == ipOutputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker Error, Output Buffer Dequeue returned NULL, OUT"));
                        return;
                    }

                    ipOutputBuffer->nFilledLen = 0;
                    iNewOutBufRequired = OMX_FALSE;

                    ipOutputBuffer->nTimeStamp = iCurrentTimestamp;
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

        if (iInputCurrLength > 0)
        {
            pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
            OutputLength = 0;

            //Output buffer is passed as a short pointer
            DecodeReturn = ipAmrDec->AmrDecodeFrame((OMX_S16*)pOutBuffer,
                                                    (OMX_U32*) & OutputLength,
                                                    &(ipFrameDecodeBuffer),
                                                    &(iInputCurrLength),
                                                    &iFrameCount,
                                                    &ResizeNeeded);

            //If decoder returned error, report it to the client via a callback
            if ((OMX_FALSE == DecodeReturn) && (OMX_FALSE == iEndofStream))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker ErrorStreamCorrupt callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventError,
                 OMX_ErrorStreamCorrupt,
                 0,
                 NULL);
            }

            if (ResizeNeeded == OMX_TRUE)
            {
                if (0 != OutputLength)
                {
                    iOutputFrameLength = OutputLength;

                }

                iResizePending = OMX_TRUE;

                /* Do not return the output buffer generated yet, store it locally
                 * and wait for the dynamic port reconfig to complete */
                if ((NULL == ipTempOutBufferForPortReconfig))
                {
                    ipTempOutBufferForPortReconfig = (OMX_U8*) oscl_malloc(sizeof(uint8) * OutputLength);
                    if (NULL == ipTempOutBufferForPortReconfig)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker error, insufficient resources"));
                        return;
                    }
                }

                //Copy the omx output buffer to the temporary internal buffer
                oscl_memcpy(ipTempOutBufferForPortReconfig, pOutBuffer, OutputLength);
                iSizeOutBufferForPortReconfig = OutputLength;

                iTimestampOutBufferForPortReconfig = iCurrentTimestamp;

                iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;
                //Make this length 0 so that no output buffer is returned by the component
                OutputLength = 0;


                // send port settings changed event
                OMX_COMPONENTTYPE* pHandle = (OMX_COMPONENTTYPE*) ipAppPriv->CompHandle;

                // set the flag to disable further processing until Client reacts to this
                //	by doing dynamic port reconfiguration

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventPortSettingsChanged, //The command was completed
                 OMX_PORT_OUTPUTPORT_INDEX,
                 0,
                 NULL);

            }

            ipOutputBuffer->nFilledLen += OutputLength;
            if (OutputLength > 0)
            {
                iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;
            }
            //offset not required in our case, set it to zero
            ipOutputBuffer->nOffset = 0;


            /* Return the input buffer it has been consumed fully or decoder returned error*/
            if ((iInputCurrLength == 0) || (OMX_FALSE == DecodeReturn))
            {
                ipInputBuffer->nFilledLen = 0;
                ReturnInputBuffer(ipInputBuffer, pInPort);
                iNewInBufferRequired = OMX_TRUE;
                iIsInputBufferEnded = OMX_TRUE;
                iInputCurrLength = 0;
                ipInputBuffer = NULL;
            }
            else
            {
                iNewInBufferRequired = OMX_FALSE;
                iIsInputBufferEnded = OMX_FALSE;
            }
        }


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if ((0 == iInputCurrLength) || (OMX_FALSE == DecodeReturn))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker EOS callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventBufferFlag,
                 1,
                 OMX_BUFFERFLAG_EOS,
                 NULL);

                iNewInBufferRequired = OMX_TRUE;
                //Mark this flag false once the callback has been send back
                iEndofStream = OMX_FALSE;

                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                ReturnOutputBuffer(ipOutputBuffer, pOutPort);
                ipOutputBuffer = NULL;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker OUT"));
                return;
            }

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
        if ((iInputCurrLength != 0 || GetQueueNumElem(pInputQueue) > 0)
                && (GetQueueNumElem(pOutputQueue) > 0) && (ResizeNeeded == OMX_FALSE))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DecodeWithMarker OUT"));
    return;
}



//Not implemented & supported in case of base profile components

void OpenmaxAmrAO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"audio_decoder.amr";
}


//Component constructor
OpenmaxAmrAO::OpenmaxAmrAO()
{
    ipAmrDec = NULL;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : constructed"));
}


//Active object destructor
OpenmaxAmrAO::~OpenmaxAmrAO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : destructed"));
}


/** The Initialization function
 */
OMX_ERRORTYPE OpenmaxAmrAO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ComponentInit IN"));

    OMX_BOOL Status = OMX_TRUE;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;


    // Added an extra check based on whether client has set nb or wb as Role in
    // SetParameter() for index OMX_IndexParamStandardComponentRole
    OMX_AUDIO_AMRBANDMODETYPE AmrBandMode = ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRBandMode;
    if ((OMX_TRUE == iComponentRoleFlag) && (0 == oscl_strcmp((OMX_STRING)iComponentRole, (OMX_STRING)"audio_decoder.amrnb")))
    {
        if ((AmrBandMode < OMX_AUDIO_AMRBandModeNB0) || (AmrBandMode > OMX_AUDIO_AMRBandModeNB7))
        {
            //Narrow band component does not support these band modes
            return OMX_ErrorInvalidComponent;
        }
    }
    else if ((OMX_TRUE == iComponentRoleFlag) && (0 == oscl_strcmp((OMX_STRING)iComponentRole, (OMX_STRING)"audio_decoder.amrwb")))
    {
        if ((AmrBandMode < OMX_AUDIO_AMRBandModeWB0) || (AmrBandMode > OMX_AUDIO_AMRBandModeWB8))
        {
            //Wide band component does not support these band modes
            return OMX_ErrorInvalidComponent;
        }
    }

    //amr lib init
    if (!iCodecReady)
    {
        Status = ipAmrDec->AmrDecInit(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRFrameFormat, ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAmrParam.eAMRBandMode);
        iCodecReady = OMX_TRUE;
    }

    //ipAmrDec->iAmrInitFlag = 0;
    iInputCurrLength = 0;
    //Used in dynamic port reconfiguration
    iFrameCount = 0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ComponentInit OUT"));

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
 *  Also it is called by the ComponentDestructor() function
 */
OMX_ERRORTYPE OpenmaxAmrAO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ComponentDeInit IN"));

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        ipAmrDec->AmrDecDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : ComponentDeInit OUT"));

    return OMX_ErrorNone;

}


//Check whether silence insertion is required here or not
void OpenmaxAmrAO::CheckForSilenceInsertion()
{
    OMX_TICKS TimestampGap;
    //Set the flag to false by default
    iSilenceInsertionInProgress = OMX_FALSE;

    TimestampGap = iFrameTimestamp - iCurrentTimestamp;

    if ((TimestampGap > OMX_HALFRANGE_THRESHOLD) || (TimestampGap < OMX_AMR_DEC_FRAME_INTERVAL && iFrameCount > 0))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : CheckForSilenceInsertion OUT - No need to insert silence"));
        return;
    }

    //Silence insertion needed, mark the flag to true
    if (iFrameCount > 0)
    {
        iSilenceInsertionInProgress = OMX_TRUE;
        //Determine the number of silence frames to insert
        iSilenceFramesNeeded = TimestampGap / OMX_AMR_DEC_FRAME_INTERVAL;
        iZeroFramesNeeded = 0;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : CheckForSilenceInsertion OUT - Silence Insertion required here"));
    }

    return;
}

//Perform the silence insertion
void OpenmaxAmrAO::DoSilenceInsertion()
{
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    OMX_U8*	pOutBuffer = NULL;
    OMX_U32	OutputLength;
    //OMX_BOOL ResizeNeeded = OMX_FALSE;
    OMX_BOOL DecodeReturn;


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion IN"));

    while (iSilenceFramesNeeded > 0)
    {
        //Check whether prev output bufer has been consumed or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                //Resume Silence insertion next time when component will be called
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion OUT output buffer unavailable"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion Error, Output Buffer Dequeue returned NULL, OUT"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentTimestamp;
        }

        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        OutputLength = 0;

        //Decode the silence frame
        DecodeReturn = ipAmrDec->AmrDecodeSilenceFrame((OMX_S16*) pOutBuffer,
                       (OMX_U32*) & OutputLength);


        if (OMX_FALSE == DecodeReturn)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion - Decode error of silence generation, Insert zero frames instead"));
            iZeroFramesNeeded = iSilenceFramesNeeded;
            iSilenceFramesNeeded = 0;
            break;
        }

        //Output length for a buffer of OMX_U8* will be double as that of OMX_S16*
        ipOutputBuffer->nFilledLen += OutputLength;
        //offset not required in our case, set it to zero
        ipOutputBuffer->nOffset = 0;

        if (OutputLength > 0)
        {
            iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion - silence frame decoded"));

        //Send the output buffer back when it has become full
        if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < iOutputFrameLength)
        {
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
        }

        // Decrement the silence frame counter
        --iSilenceFramesNeeded;
    }

    // THE ZERO FRAME INSERTION IS PERFORMED ONLY IF SILENCE INSERTION FAILS
    while (iZeroFramesNeeded > 0)
    {
        //Check whether prev output bufer has been consumed or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                //Resume Silence insertion next time when component will be called
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion OUT output buffer unavailable"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion Error, Output Buffer Dequeue returned NULL, OUT"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentTimestamp;
        }

        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        oscl_memset(pOutBuffer, 0, iOutputFrameLength);

        ipOutputBuffer->nFilledLen += iOutputFrameLength;
        ipOutputBuffer->nOffset = 0;
        iCurrentTimestamp += OMX_AMR_DEC_FRAME_INTERVAL;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion - One frame of zeros inserted"));

        //Send the output buffer back when it has become full
        if ((ipOutputBuffer->nAllocLen - ipOutputBuffer->nFilledLen) < iOutputFrameLength)
        {
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
        }

        // Decrement the silence frame counter
        --iZeroFramesNeeded;
    }

    /* Completed Silence insertion successfully, now consider the input buffer already dequeued
     * for decoding & update the timestamps */

    iSilenceInsertionInProgress = OMX_FALSE;
    iCurrentTimestamp = iFrameTimestamp;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAmrAO : DoSilenceInsertion OUT - Done successfully"));

    return;
}

OMX_ERRORTYPE OpenmaxAmrAO::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(nIndex);
    OSCL_UNUSED_ARG(pComponentConfigStructure);
    return OMX_ErrorNotImplemented;
}
