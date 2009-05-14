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

#include "omx_aac_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#define OMX_HALFRANGE_THRESHOLD 0x7FFFFFFF

#define AAC_MONO_SILENCE_FRAME_SIZE 10
#define AAC_STEREO_SILENCE_FRAME_SIZE 11

static const OMX_U8 AAC_MONO_SILENCE_FRAME[]   = {0x01, 0x40, 0x20, 0x06, 0x4F, 0xDE, 0x02, 0x70, 0x0C, 0x1C};      // 10 bytes
static const OMX_U8 AAC_STEREO_SILENCE_FRAME[] = {0x21, 0x10, 0x05, 0x00, 0xA0, 0x19, 0x33, 0x87, 0xC0, 0x00, 0x7E}; // 11 bytes)

OSCL_DLL_ENTRY_POINT_DEFAULT()


// This function is called by OMX_GetHandle and it creates an instance of the aac component AO
OSCL_EXPORT_REF OMX_ERRORTYPE AacOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    OpenmaxAacAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitAacOmxComponentFields content to actual constructor

    pOpenmaxAOType = (OpenmaxAacAO*) OSCL_NEW(OpenmaxAacAO, ());

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
OSCL_EXPORT_REF OMX_ERRORTYPE AacOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OpenmaxAacAO* pOpenmaxAOType = (OpenmaxAacAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if DYNAMIC_LOAD_OMX_AAC_COMPONENT
class AacOmxSharedLibraryInterface: public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static AacOmxSharedLibraryInterface *Instance()
        {
            static AacOmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_AACDEC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AacOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AacOmxComponentDestructor));
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
        AacOmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return AacOmxSharedLibraryInterface::Instance();
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

OMX_ERRORTYPE OpenmaxAacAO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
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

    iOmxComponent.SendCommand = OpenmaxAacAO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OpenmaxAacAO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OpenmaxAacAO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OpenmaxAacAO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OpenmaxAacAO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxAacAO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxAacAO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OpenmaxAacAO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxAacAO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxAacAO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxAacAO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxAacAO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OpenmaxAacAO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OpenmaxAacAO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OpenmaxAacAO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OpenmaxAacAO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OpenmaxAacAO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxAacAO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxAacAO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OpenmaxAacAO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxAacAO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxAacAO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxAacAO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxAacAO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OpenmaxAacAO::BaseComponentSetCallbacks;
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
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.audio.eEncoding = OMX_AUDIO_CodingAAC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_AAC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_AAC;
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
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_AAC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_AAC * 6;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;

    //Default values for AAC audio param port
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nChannels = 2;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nBitRate = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nSampleRate = 44100;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nAudioBandWidth = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.nFrameLength = 2048; // use HE_PS frame size as default
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.eChannelMode = OMX_AUDIO_ChannelModeStereo;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.eAACProfile = OMX_AUDIO_AACObjectHE_PS;    //OMX_AUDIO_AACObjectLC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP2ADTS;

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
    pInPort->AudioParam.eEncoding = OMX_AUDIO_CodingAAC;

    SetHeader(&pOutPort->AudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    pOutPort->AudioParam.nPortIndex = 1;
    pOutPort->AudioParam.nIndex = 0;
    pOutPort->AudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    iOutputFrameLength = OUTPUT_BUFFER_SIZE_AAC;

    if (ipAacDec)
    {
        OSCL_DELETE(ipAacDec);
        ipAacDec = NULL;
    }

    ipAacDec = OSCL_NEW(OmxAacDecoder, ());
    if (NULL == ipAacDec)
    {
        return OMX_ErrorInsufficientResources;
    }

    oscl_memset(ipAacDec, 0, sizeof(OmxAacDecoder));

    iSamplesPerFrame = AACDEC_PCM_FRAME_SAMPLE_SIZE;
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

OMX_ERRORTYPE OpenmaxAacAO::DestroyComponent()
{
    if (iIsInit != OMX_FALSE)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipAacDec)
    {
        OSCL_DELETE(ipAacDec);
        ipAacDec = NULL;
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
void OpenmaxAacAO::SyncWithInputTimestamp()
{
    OMX_AUDIO_AACSTREAMFORMATTYPE InFormat = ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.eAACStreamFormat;

    //Do not check for silence insertion if the clip is repositioned
    if (OMX_FALSE == iRepositionFlag)
    {
        CheckForSilenceInsertion();
    }


    /* Set the current timestamp equal to input buffer timestamp in case of
     * a) first frame for ADIF format
     * b) All input frames for other formats
     * c) First frame after repositioning */
    if (OMX_FALSE == iSilenceInsertionInProgress)
    {
        if ((0 == iFrameCount) || (iFrameCount > 0 && OMX_AUDIO_AACStreamFormatADIF != InFormat)
                || (OMX_TRUE == iRepositionFlag))
        {
            iCurrentFrameTS.SetFromInputTimestamp(iFrameTimestamp);

            //Reset the flag back to false, once timestamp has been updated from input frame
            if (OMX_TRUE == iRepositionFlag)
            {
                iRepositionFlag = OMX_FALSE;
            }
        }
    }
}

/* A component specific routine called from BufferMgmtWithoutMarker */
void OpenmaxAacAO::ProcessInBufferFlag()
{
    iIsInputBufferEnded = OMX_FALSE;
}


void OpenmaxAacAO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData IN"));

    QueueType* pInputQueue  = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType* pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_COMPONENTTYPE* pHandle = &iOmxComponent;

    OMX_U8* pOutBuffer;
    OMX_U32 OutputLength;
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData Error, output buffer dequeue returned NULL, OUT"));
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

                //Dequeue new output buffer to continue decoding the next frame

                if (OMX_TRUE == iNewOutBufRequired)
                {
                    if (0 == (GetQueueNumElem(pOutputQueue)))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData OUT, output buffer unavailable"));
                        return;
                    }

                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
                    if (NULL == ipOutputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData Error, output buffer dequeue returned NULL, OUT"));
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
        DecodeReturn = ipAacDec->AacDecodeFrames((OMX_S16*) pOutBuffer,
                       (OMX_U32*) & OutputLength,
                       &(ipFrameDecodeBuffer),
                       &iInputCurrLength,
                       &iFrameCount,
                       &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode),
                       &(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam),
                       &iSamplesPerFrame,
                       &ResizeNeeded);

        if (ResizeNeeded == OMX_TRUE)
        {
            if (0 != OutputLength)
            {
                iOutputFrameLength = OutputLength * 2;
                //In case of mono files keep the frame boundary more because decoder tries to write something beyond that level also
                if (1 == ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels)
                {
                    iOutputFrameLength *= 2;
                }

                iCurrentFrameTS.SetParameters(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nSamplingRate, iSamplesPerFrame);
                iOutputMilliSecPerFrame = iCurrentFrameTS.GetFrameDuration();

            }

            // set the flag to disable further processing until Client reacts to this
            //  by doing dynamic port reconfiguration
            iResizePending = OMX_TRUE;

            /* Do not return the output buffer generated yet, store it locally
             * and wait for the dynamic port reconfig to complete */

            if ((NULL == ipTempOutBufferForPortReconfig))
            {
                ipTempOutBufferForPortReconfig = (OMX_U8*) oscl_malloc(sizeof(uint8) * OutputLength * 2);
                if (NULL == ipTempOutBufferForPortReconfig)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData error, insufficient resources"));
                    return;
                }
            }

            //Copy the omx output buffer to the temporary internal buffer
            oscl_memcpy(ipTempOutBufferForPortReconfig, pOutBuffer, OutputLength * 2);
            iSizeOutBufferForPortReconfig = OutputLength * 2;

            //Set the current timestamp to the output buffer timestamp for the first output frame, Later it will be done at the time of dequeue
            //(reason here is that at the time of dequeue, output buffer timestamp was equal to timestamp of config input buffer, not the first input buffer)
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


        //Output length for a buffer of OMX_U8* will be double as that of OMX_S16*
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
            if ((0 == iInputCurrLength) || (MP4AUDEC_SUCCESS != DecodeReturn))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData EOS callback send"));

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

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData OUT"));

                return;
            }
        }


        if (MP4AUDEC_SUCCESS == DecodeReturn)
        {
            ipInputBuffer->nFilledLen = iInputCurrLength;
        }
        else if (MP4AUDEC_INCOMPLETE_FRAME == DecodeReturn)
        {
            /* If decoder returns MP4AUDEC_INCOMPLETE_FRAME,
             * this indicates the input buffer contains less than a frame data that
             * can't be processed by the decoder.
             * Copy it to a temp buffer to be used in next decode call
             * buffers can be overlapping, use memmove() instead of memcpy()
             */
            oscl_memmove(ipTempInputBuffer, ipFrameDecodeBuffer, iInputCurrLength);
            iTempInputBufferLength = iInputCurrLength;
            ipInputBuffer->nFilledLen = 0;
            iInputCurrLength = 0;
        }
        else
        {
            OMX_U32 error = OMX_ErrorStreamCorrupt;
            if (ipAacDec->iAacInitFlag == 0)
            {
                // If first frame, it means that the config has an issue.
                error = OMX_ErrorBadParameter;
            }
            //bitstream error, discard the current data as it can't be decoded further
            ipInputBuffer->nFilledLen = 0;
            iInputCurrLength = 0;

            //Report it to the client via a callback
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData ErrorStreamCorrupt callback send"));

            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventError,
             error,
             0,
             NULL);

        }

        //Return the input buffer if it has been consumed fully by the decoder
        if (0 == ipInputBuffer->nFilledLen)
        {
            ReturnInputBuffer(ipInputBuffer, pInPort);
            iIsInputBufferEnded = OMX_TRUE;
            iInputCurrLength = 0;
            ipInputBuffer = NULL;
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
        if (((iInputCurrLength != 0) || (GetQueueNumElem(pInputQueue) > 0))
                && ((GetQueueNumElem(pOutputQueue) > 0) || (OMX_FALSE == iNewOutBufRequired))
                && (ResizeNeeded == OMX_FALSE))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ProcessData OUT"));
    return;
}


/* This routine will reset the decoder library and some of the associated flags*/
void OpenmaxAacAO::ResetComponent()
{
    // reset decoder
    if (ipAacDec)
    {
        ipAacDec->ResetDecoder();
        ipAacDec->iInputUsedLength = 0;
    }

}



/* Routine to call the respective function of the decoder library for updating the
 * AAC+ flag, A separate routine is written here because the direct call can't be
 * done from GetParameter of common audio base class */

void OpenmaxAacAO::UpdateAACPlusFlag(OMX_BOOL aAacPlusFlag)
{
    ipAacDec->UpdateAACPlusEnabled(aAacPlusFlag);
}


void OpenmaxAacAO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"audio_decoder.aac";
}


//Active object constructor
OpenmaxAacAO::OpenmaxAacAO()
{
    ipAacDec = NULL;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : constructed"));
}


//Active object destructor
OpenmaxAacAO::~OpenmaxAacAO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : destructed"));
}


/** The Initialization function
 */
OMX_ERRORTYPE OpenmaxAacAO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ComponentInit IN"));

    OMX_BOOL Status = OMX_TRUE;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;

    //aac lib init
    if (!iCodecReady)
    {
        Status = ipAacDec->AacDecInit(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels);
        iCodecReady = OMX_TRUE;
    }

    //ipAacDec->iAacInitFlag = 0;
    iInputCurrLength = 0;
    //Used in dynamic port reconfiguration
    iFrameCount = 0;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ComponentInit OUT"));

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
OMX_ERRORTYPE OpenmaxAacAO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ComponentDeInit IN"));

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        ipAacDec->AacDecDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : ComponentDeInit OUT"));

    return OMX_ErrorNone;

}

//Check whether silence insertion is required here or not
void OpenmaxAacAO::CheckForSilenceInsertion()
{
    OMX_AUDIO_AACSTREAMFORMATTYPE InFormat = ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam.eAACStreamFormat;
    OMX_TICKS CurrTimestamp, TimestampGap;
    //Set the flag to false by default
    iSilenceInsertionInProgress = OMX_FALSE;

    if (OMX_AUDIO_AACStreamFormatADIF == InFormat)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : CheckForSilenceInsertion OUT - Don't do it for ADIF since there is no timestamp to sync"));
        return;
    }

    CurrTimestamp = iCurrentFrameTS.GetCurrentTimestamp();
    TimestampGap = iFrameTimestamp - CurrTimestamp;

    if ((TimestampGap > OMX_HALFRANGE_THRESHOLD) || (TimestampGap < iOutputMilliSecPerFrame && iFrameCount > 0))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : CheckForSilenceInsertion OUT - No need to insert silence"));
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
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : CheckForSilenceInsertion OUT - Silence Insertion required here"));
    }

    return;
}

//Perform the silence insertion
void OpenmaxAacAO::DoSilenceInsertion()
{
    OMX_S32 NumOfChannels = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode.nChannels;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType* pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    OMX_U8* pOutBuffer = NULL;
    OMX_U8* pSilenceInputBuffer = NULL;
    OMX_U32 OutputLength, SilenceFrameLength;
    OMX_S32 DecodeReturn;
    OMX_BOOL ResizeNeeded = OMX_FALSE;


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion IN"));

    while (iSilenceFramesNeeded > 0)
    {
        //Check whether prev output bufer has been consumed or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                //Resume Silence insertion next time when component will be called
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion OUT output buffer unavailable"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
            if (NULL == ipOutputBuffer)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion Error, output buffer dequeue returned NULL, OUT"));
                iSilenceInsertionInProgress = OMX_TRUE;
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            //Set the current timestamp to the output buffer timestamp
            ipOutputBuffer->nTimeStamp = iCurrentFrameTS.GetConvertedTs();
        }

        // Setup the input side for silence frame
        if (NumOfChannels > 1)
        {
            // Stereo silence frame
            pSilenceInputBuffer = (OMX_U8*) AAC_STEREO_SILENCE_FRAME;
            SilenceFrameLength = AAC_STEREO_SILENCE_FRAME_SIZE;
        }
        else
        {
            // Mono silence frame
            pSilenceInputBuffer = (OMX_U8*) AAC_MONO_SILENCE_FRAME;
            SilenceFrameLength = AAC_MONO_SILENCE_FRAME_SIZE;
        }


        pOutBuffer = &ipOutputBuffer->pBuffer[ipOutputBuffer->nFilledLen];
        OutputLength = 0;

        //Decode the silence frame
        DecodeReturn = ipAacDec->AacDecodeFrames((OMX_S16*) pOutBuffer,
                       (OMX_U32*) & OutputLength,
                       &(pSilenceInputBuffer),
                       &SilenceFrameLength,
                       &iFrameCount,
                       &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AudioPcmMode),
                       &(ipPorts[OMX_PORT_INPUTPORT_INDEX]->AudioAacParam),
                       &iSamplesPerFrame,
                       &ResizeNeeded);



        //Output length for a buffer of OMX_U8* will be double as that of OMX_S16*
        ipOutputBuffer->nFilledLen += OutputLength * 2;
        //offset not required in our case, set it to zero
        ipOutputBuffer->nOffset = 0;

        if (OutputLength > 0)
        {
            iCurrentFrameTS.UpdateTimestamp(iSamplesPerFrame);
        }

        if (MP4AUDEC_SUCCESS != DecodeReturn)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion - Silence frame decoding error. Skip inserting silence frame and move the timestamp forward"));
            iCurrentFrameTS.SetFromInputTimestamp(iFrameTimestamp);
            iSilenceInsertionInProgress = OMX_FALSE;
            return;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion - silence frame decoded"));

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
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxAacAO : DoSilenceInsertion OUT - Done successfully"));

    return;
}

OMX_ERRORTYPE OpenmaxAacAO::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(nIndex);
    OSCL_UNUSED_ARG(pComponentConfigStructure);

    return OMX_ErrorNotImplemented;
}
