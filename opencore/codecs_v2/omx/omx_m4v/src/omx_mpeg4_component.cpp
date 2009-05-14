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
#include "omx_mpeg4_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif

// Use default DLL entry point
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

OSCL_DLL_ENTRY_POINT_DEFAULT()

static const uint32 mask[33] =
{
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};

// This function is called by OMX_GetHandle and it creates an instance of the mpeg4 component AO
OSCL_EXPORT_REF OMX_ERRORTYPE Mpeg4OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy , OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);


    OpenmaxMpeg4AO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitMpeg4OmxComponentFields content to actual constructor

    pOpenmaxAOType = (OpenmaxMpeg4AO*) OSCL_NEW(OpenmaxMpeg4AO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    // set decoding mode to H263
    pOpenmaxAOType->SetDecoderMode(MODE_MPEG4);

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
    ///////////////////////////////////////////////////////////////////////////////////////
}

// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OSCL_EXPORT_REF OMX_ERRORTYPE Mpeg4OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OpenmaxMpeg4AO* pOpenmaxAOType = (OpenmaxMpeg4AO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

// This function is called by OMX_GetHandle and it creates an instance of the h263 component AO
OSCL_EXPORT_REF OMX_ERRORTYPE H263OmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_IN OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    OpenmaxMpeg4AO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitMpeg4OmxComponentFields content to actual constructor

    pOpenmaxAOType = (OpenmaxMpeg4AO*) OSCL_NEW(OpenmaxMpeg4AO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    // set decoding mode to H263
    pOpenmaxAOType->SetDecoderMode(MODE_H263);

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
    ///////////////////////////////////////////////////////////////////////////////////////
}

// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OSCL_EXPORT_REF OMX_ERRORTYPE H263OmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OpenmaxMpeg4AO* pOpenmaxAOType = (OpenmaxMpeg4AO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if (DYNAMIC_LOAD_OMX_M4V_COMPONENT || DYNAMIC_LOAD_OMX_H263_COMPONENT)
class Mpeg4H263OmxSharedLibraryInterface: public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static Mpeg4H263OmxSharedLibraryInterface *Instance()
        {
            static Mpeg4H263OmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_M4VDEC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mpeg4OmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mpeg4OmxComponentDestructor));
                }
            }
            else if (PV_OMX_H263DEC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&H263OmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&H263OmxComponentDestructor));
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
        Mpeg4H263OmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return Mpeg4H263OmxSharedLibraryInterface::Instance();
    }
}

#endif

void OpenmaxMpeg4AO::SetDecoderMode(int mode)
{
    iDecMode = mode;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////

OMX_ERRORTYPE OpenmaxMpeg4AO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType *pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    ipComponentProxy = pProxy;
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data


#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OpenmaxMpeg4AO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OpenmaxMpeg4AO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OpenmaxMpeg4AO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OpenmaxMpeg4AO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OpenmaxMpeg4AO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxMpeg4AO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxMpeg4AO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OpenmaxMpeg4AO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxMpeg4AO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxMpeg4AO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxMpeg4AO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxMpeg4AO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OpenmaxMpeg4AO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OpenmaxMpeg4AO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OpenmaxMpeg4AO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OpenmaxMpeg4AO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OpenmaxMpeg4AO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OpenmaxMpeg4AO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OpenmaxMpeg4AO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OpenmaxMpeg4AO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OpenmaxMpeg4AO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OpenmaxMpeg4AO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OpenmaxMpeg4AO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OpenmaxMpeg4AO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OpenmaxMpeg4AO::BaseComponentSetCallbacks;
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
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainVideo;
    if (iDecMode == MODE_MPEG4)
    {
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"video/mpeg4";
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    }
    else if (iDecMode == MODE_H263)
    {
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"video/h263";
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;

    }

    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.pNativeRender = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.bFlagErrorConcealment = OMX_FALSE;

    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nFrameWidth = 176;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nFrameHeight = 144;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nBitrate = 64000;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.xFramerate = (15 << 16);
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_MP4;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_MP4;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainVideo;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"raw";
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.pNativeRender = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameWidth = 176; //320; //176;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameHeight = 144; //240; //144;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nBitrate = 64000;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.xFramerate = (15 << 16);
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDir = OMX_DirOutput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_MP4;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_MP4;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;

    if (iDecMode == MODE_MPEG4)
    {
        //Default values for mpeg4 video param port
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->VideoMpeg4.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->VideoMpeg4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->VideoMpeg4.eLevel = OMX_VIDEO_MPEG4Level1;

        ipPorts[OMX_PORT_INPUTPORT_INDEX]->ProfileLevel.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->ProfileLevel.nProfileIndex = 0;
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->ProfileLevel.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
        ipPorts[OMX_PORT_INPUTPORT_INDEX]->ProfileLevel.eLevel = OMX_VIDEO_MPEG4Level1;
    }
    else if (iDecMode == MODE_H263)
    {

    }

    iPortTypesParam.nPorts = 2;
    iPortTypesParam.nStartPortNumber = 0;

    pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    pOutPort = (ComponentPortType*) ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    pInPort->ActualNumPortFormatsSupported = 1;

    //OMX_VIDEO_PARAM_PORTFORMATTYPE INPUT PORT SETTINGS
    //On input port for index 0
    SetHeader(&pInPort->VideoParam[0], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pInPort->VideoParam[0].nPortIndex = 0;
    pInPort->VideoParam[0].nIndex = 0;

    if (iDecMode == MODE_MPEG4)
    {
        pInPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    }
    else if (iDecMode == MODE_H263)
    {
        pInPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingH263;
    }

    pInPort->VideoParam[0].eColorFormat = OMX_COLOR_FormatUnused;

    pOutPort->ActualNumPortFormatsSupported = 1;

    //OMX_VIDEO_PARAM_PORTFORMATTYPE OUTPUT PORT SETTINGS
    //On output port for index 0
    SetHeader(&pOutPort->VideoParam[0], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pOutPort->VideoParam[0].nPortIndex = 1;
    pOutPort->VideoParam[0].nIndex = 0;
    pOutPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingUnused;
    pOutPort->VideoParam[0].eColorFormat = OMX_COLOR_FormatYUV420Planar;


    iUseExtTimestamp = OMX_TRUE;


    if (ipMpegDecoderObject)
    {
        OSCL_DELETE(ipMpegDecoderObject);
        ipMpegDecoderObject = NULL;
    }
    ipMpegDecoderObject = OSCL_NEW(Mpeg4Decoder_OMX, ());
    oscl_memset(ipMpegDecoderObject, 0, sizeof(Mpeg4Decoder_OMX));

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

OMX_ERRORTYPE OpenmaxMpeg4AO::DestroyComponent()
{
    if (OMX_FALSE != iIsInit)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipMpegDecoderObject)
    {
        OSCL_DELETE(ipMpegDecoderObject);
        ipMpegDecoderObject = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}



/* This function will be called in case of buffer management without marker bit present
 * The purpose is to copy the current input buffer into a big temporary buffer, so that
 * an incomplete/partial frame is never passed to the decoder library for decode
 */
void OpenmaxMpeg4AO::ComponentBufferMgmtWithoutMarker()
{
    //This common routine has been written in the base class
    TempInputBufferMgmtWithoutMarker();
}


void OpenmaxMpeg4AO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ProcessData IN"));
    if (!iEndOfFrameFlag)
    {
        DecodeWithoutMarker();
    }
    else
    {
        DecodeWithMarker();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ProcessData OUT"));
}


void OpenmaxMpeg4AO::DecodeWithoutMarker()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    OMX_COMPONENTTYPE  *pHandle = &iOmxComponent;

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    OMX_U8*					pTempInBuffer;
    OMX_U32					TempInLength;
    OMX_BOOL				DecodeReturn;
    OMX_BOOL				MarkerFlag = OMX_FALSE;
    OMX_BOOL				ResizeNeeded = OMX_FALSE;

    OMX_U32 TempInputBufferSize = (2 * sizeof(uint8) * (ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize));

    OMX_U32 CurrWidth =  ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameWidth;
    OMX_U32 CurrHeight = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameHeight;

    if ((!iIsInputBufferEnded) || iEndofStream)
    {
        //Check whether a new output buffer is available or not
        if (0 == (GetQueueNumElem(pOutputQueue)))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker OUT output buffer unavailable"));
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
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker Error, output buffer dequeue returned NULL, OUT"));
            return;
        }

        //Do not proceed if the output buffer can't fit the YUV data
        if (ipOutputBuffer->nAllocLen < (OMX_U32)((((CurrWidth + 15) >> 4) << 4) * (((CurrHeight + 15) >> 4) << 4) * 3 / 2))
        {
            ipOutputBuffer->nFilledLen = 0;
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
            return;
        }

        ipOutputBuffer->nFilledLen = 0;


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

        pOutBuffer = ipOutputBuffer->pBuffer;
        OutputLength = 0;

        pTempInBuffer = ipTempInputBuffer + iTempConsumedLength;
        TempInLength = iTempInputBufferLength;

        //Output buffer is passed as a short pointer
        DecodeReturn = ipMpegDecoderObject->Mp4DecodeVideo(pOutBuffer, (OMX_U32*) & OutputLength,
                       &(pTempInBuffer),
                       &TempInLength,
                       &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam),
                       &iFrameCount,
                       MarkerFlag,
                       &ResizeNeeded);

        ipOutputBuffer->nFilledLen = OutputLength;

        //offset not required in our case, set it to zero
        ipOutputBuffer->nOffset = 0;

        //If decoder returned error, report it to the client via a callback
        if (!DecodeReturn && OMX_FALSE == iEndofStream)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker ErrorStreamCorrupt callback send"));

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
            // send port settings changed event
            OMX_COMPONENTTYPE* pHandle = (OMX_COMPONENTTYPE*) ipAppPriv->CompHandle;

            // set the flag to disable further processing until Client reacts to this
            //	by doing dynamic port reconfiguration
            iResizePending = OMX_TRUE;

            (*(ipCallbacks->EventHandler))
            (pHandle,
             iCallbackData,
             OMX_EventPortSettingsChanged, //The command was completed
             OMX_PORT_OUTPUTPORT_INDEX,
             0,
             NULL);

        }
        //Set the timestamp equal to the input buffer timestamp
        ipOutputBuffer->nTimeStamp = iFrameTimestamp;

        iTempConsumedLength += (iTempInputBufferLength - TempInLength);
        iTempInputBufferLength = TempInLength;

        //Do not decode if big buffer is less than half the size
        if (TempInLength < (TempInputBufferSize >> 1))
        {
            iIsInputBufferEnded = OMX_TRUE;
            iNewInBufferRequired = OMX_TRUE;
        }


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if ((0 == iTempInputBufferLength) || (!DecodeReturn))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker EOS callback send"));

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

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker OUT"));

                return;
            }
        }

        //Send the output buffer back after decode
        ReturnOutputBuffer(ipOutputBuffer, pOutPort);
        ipOutputBuffer = NULL;

        /* If there is some more processing left with current buffers, re-schedule the AO
         * Do not go for more than one round of processing at a time.
         * This may block the AO longer than required.
         */
        if ((TempInLength != 0 || GetQueueNumElem(pInputQueue) > 0)	&& (GetQueueNumElem(pOutputQueue) > 0) && (ResizeNeeded == OMX_FALSE))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithoutMarker OUT"));
    return;
}


void OpenmaxMpeg4AO::DecodeWithMarker()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType*	pInPort = ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    OMX_BOOL				DecodeReturn = OMX_FALSE;
    OMX_BOOL				MarkerFlag = OMX_TRUE;
    OMX_COMPONENTTYPE *		pHandle = &iOmxComponent;
    OMX_BOOL				ResizeNeeded = OMX_FALSE;

    OMX_U32 CurrWidth =  ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameWidth;
    OMX_U32 CurrHeight = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameHeight;

    if ((!iIsInputBufferEnded) || (iEndofStream))
    {
        //Check whether a new output buffer is available or not
        if (0 == (GetQueueNumElem(pOutputQueue)))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker OUT output buffer unavailable"));
            iNewInBufferRequired = OMX_FALSE;
            return;
        }

        ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);
        if (NULL == ipOutputBuffer)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker Error, output buffer dequeue returned NULL, OUT"));
            iNewInBufferRequired = OMX_FALSE;
            return;
        }

        //Do not proceed if the output buffer can't fit the YUV data
        if (ipOutputBuffer->nAllocLen < (OMX_U32)((((CurrWidth + 15) >> 4) << 4) * (((CurrHeight + 15) >> 4) << 4) * 3 / 2))
        {
            ipOutputBuffer->nFilledLen = 0;
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
            ipOutputBuffer = NULL;
            return;
        }
        ipOutputBuffer->nFilledLen = 0;

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
            pOutBuffer = ipOutputBuffer->pBuffer;
            OutputLength = 0;

            //Output buffer is passed as a short pointer
            DecodeReturn = ipMpegDecoderObject->Mp4DecodeVideo(pOutBuffer, (OMX_U32*) & OutputLength,
                           &(ipFrameDecodeBuffer),
                           &(iInputCurrLength),
                           &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam),
                           &iFrameCount,
                           MarkerFlag,
                           &ResizeNeeded);

            ipOutputBuffer->nFilledLen = OutputLength;
            //offset not required in our case, set it to zero
            ipOutputBuffer->nOffset = 0;

            //If decoder returned error, report it to the client via a callback
            if (!DecodeReturn && OMX_FALSE == iEndofStream)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker ErrorStreamCorrupt callback send"));

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
                // send port settings changed event
                OMX_COMPONENTTYPE* pHandle = (OMX_COMPONENTTYPE*) ipAppPriv->CompHandle;

                iResizePending = OMX_TRUE;
                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventPortSettingsChanged, //The command was completed
                 OMX_PORT_OUTPUTPORT_INDEX,
                 0,
                 NULL);

            }
            //Set the timestamp equal to the input buffer timestamp
            if (OMX_TRUE == iUseExtTimestamp)
            {
                ipOutputBuffer->nTimeStamp = iFrameTimestamp;
            }

            /* Discard the input frame if it is with the marker bit & decoder fails*/
            if (iInputCurrLength == 0 || !DecodeReturn)
            {
                ipInputBuffer->nFilledLen = 0;
                ReturnInputBuffer(ipInputBuffer, pInPort);
                ipInputBuffer = NULL;
                iNewInBufferRequired = OMX_TRUE;
                iIsInputBufferEnded = OMX_TRUE;
                iUseExtTimestamp = OMX_TRUE;
                iInputCurrLength = 0;
            }
            else
            {
                iNewInBufferRequired = OMX_FALSE;
                iIsInputBufferEnded = OMX_FALSE;
                iUseExtTimestamp = OMX_FALSE;
            }
        }
        else if (iEndofStream == OMX_FALSE)
        {
            // it's possible that after partial frame assembly, the input buffer still remains empty (due to
            // client erroneously sending such buffers). This code adds robustness in the sense that it returns such buffer to the client

            ipInputBuffer->nFilledLen = 0;
            ReturnInputBuffer(ipInputBuffer, pInPort);
            ipInputBuffer = NULL;
            iNewInBufferRequired = OMX_TRUE;
            iIsInputBufferEnded = OMX_TRUE;
            iUseExtTimestamp = OMX_TRUE;
        }




        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if (!DecodeReturn)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker EOS callback send"));

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

                if ((iNumInputBuffer != 0) && (NULL != ipInputBuffer))
                {
                    ReturnInputBuffer(ipInputBuffer, pInPort);
                    ipInputBuffer = NULL;
                    iIsInputBufferEnded = OMX_TRUE;
                    iInputCurrLength = 0;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker OUT"));
                return;
            }

        }

        //Send the output buffer back after decode
        ReturnOutputBuffer(ipOutputBuffer, pOutPort);
        ipOutputBuffer = NULL;


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

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : DecodeWithMarker OUT"));
    return;
}


//Not implemented & supported in case of base profile components

void OpenmaxMpeg4AO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"video_decoder.mpeg4";
}


//Component constructor
OpenmaxMpeg4AO::OpenmaxMpeg4AO()
{
    iUseExtTimestamp = OMX_TRUE;
    ipMpegDecoderObject = NULL;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : constructed"));
}


//Active object destructor
OpenmaxMpeg4AO::~OpenmaxMpeg4AO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : destructed"));
}


/** The Initialization function
 */
OMX_ERRORTYPE OpenmaxMpeg4AO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ComponentInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;


    if (!iCodecReady)
    {
        //Call the init routine here in case of H263 mode, without waiting for buffers

        if (iDecMode == MODE_H263)
        {
            OMX_S32 Width, Height, Size = 0;
            OMX_U8* Buff = NULL;

            //Pass dummy pointers during initializations
            if (OMX_TRUE != ipMpegDecoderObject->InitializeVideoDecode(&Width, &Height, &Buff, &Size, iDecMode))
            {
                Status = OMX_ErrorInsufficientResources;
            }

            ipMpegDecoderObject->Mpeg4InitFlag = 1;
        }
        else
        {
            //mp4 lib init
            Status = ipMpegDecoderObject->Mp4DecInit();
        }

        iCodecReady = OMX_TRUE;
    }

    iUseExtTimestamp = OMX_TRUE;
    iInputCurrLength = 0;

    //Used in dynamic port reconfiguration
    iFrameCount = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ComponentInit OUT"));

    return Status;

}

/** This function is called upon a transition to the idle or invalid state.
 *  Also it is called by the ComponentDestructor() function
 */
OMX_ERRORTYPE OpenmaxMpeg4AO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ComponentDeInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        Status = ipMpegDecoderObject->Mp4DecDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OpenmaxMpeg4AO : ComponentDeInit OUT"));

    return Status;

}

OMX_ERRORTYPE OpenmaxMpeg4AO::GetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    OSCL_UNUSED_ARG(nIndex);
    OSCL_UNUSED_ARG(pComponentConfigStructure);
    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE OpenmaxMpeg4AO::ReAllocatePartialAssemblyBuffers(OMX_BUFFERHEADERTYPE* aInputBufferHdr)
{

    // check if there is enough data in the buffer to read the information that we need
    if (aInputBufferHdr->nFilledLen >= MINIMUM_H263_SHORT_HEADER_SIZE)
    {
        OMX_U8 *pInputBuffer = (aInputBufferHdr->pBuffer + aInputBufferHdr->nOffset);

        if (MODE_H263 == iDecMode)
        {
            OMX_BOOL Status = OMX_TRUE;

            Status = DecodeH263Header(pInputBuffer, &iInputCurrBufferSize);

            // Re-allocate the partial frame buffer in case the stream is not corrupted,
            // otherwise leave the buffer size as it is
            if (OMX_TRUE == Status)
            {
                if (NULL != ipInputCurrBuffer)
                {
                    ipInputCurrBuffer = (OMX_U8*) oscl_realloc(ipInputCurrBuffer, iInputCurrBufferSize);
                    if (NULL == ipInputCurrBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error insufficient resources"));
                        return OMX_ErrorInsufficientResources;
                    }
                }

                //Used when the buffers are not marked with EndOfFrame flag
                if (NULL != ipTempInputBuffer)
                {
                    ipTempInputBuffer = (OMX_U8*) oscl_realloc(ipTempInputBuffer, iInputCurrBufferSize);
                    if (NULL == ipTempInputBuffer)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentBase : DoStateSet error insufficient resources"));
                        return OMX_ErrorInsufficientResources;
                    }
                }
            }
        }

        return OMX_ErrorNone;
    }
    else
    {
        return OMX_ErrorInsufficientResources;
    }

}

OMX_BOOL OpenmaxMpeg4AO::DecodeH263Header(OMX_U8* aInputBuffer,
        OMX_U32* aBufferSize)
{
    uint32 codeword;
    int32	extended_PTYPE = 0;
    int32 UFEP = 0;
    int32 custom_PFMT = 0;

    //Reset the data bit position to the start of the stream
    iH263DataBitPos = 0;
    iH263BitPos = 0;
    //BitBuf contains the first 4 bytes of the aInputBuffer
    iH263BitBuf = (aInputBuffer[0] << 24) | (aInputBuffer[1] << 16) | (aInputBuffer[2] << 8) | aInputBuffer[3];

    ReadBits(aInputBuffer, 22, &codeword);
    if (codeword !=  0x20)
    {
        return OMX_FALSE;
    }

    ReadBits(aInputBuffer, 8, &codeword);

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword == 0) return OMX_FALSE;

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword == 1) return OMX_FALSE;

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword == 1) return OMX_FALSE;

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword == 1) return OMX_FALSE;

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword == 1) return OMX_FALSE;

    /* source format */
    ReadBits(aInputBuffer, 3, &codeword);
    switch (codeword)
    {
        case 1:
            *aBufferSize = 32000;
            break;

        case 2:
            *aBufferSize = 32000;
            break;

        case 3:
            *aBufferSize = 128000;
            break;

        case 4:
            *aBufferSize = 256000;
            break;

        case 5:
            *aBufferSize = 512000;
            break;

        case 7:
            extended_PTYPE = 1;
            break;
        default:
            /* Msg("H.263 source format not legal\n"); */
            return OMX_FALSE;
    }

    if (extended_PTYPE == 0)
    {
        return OMX_TRUE;
    }

    /* source format */
    ReadBits(aInputBuffer, 3, &codeword);
    UFEP = codeword;
    if (UFEP == 1)
    {
        ReadBits(aInputBuffer, 3, &codeword);
        switch (codeword)
        {
            case 1:
                *aBufferSize = 32000;
                break;

            case 2:
                *aBufferSize = 32000;
                break;

            case 3:
                *aBufferSize = 128000;
                break;

            case 4:
                *aBufferSize = 256000;
                break;

            case 5:
                *aBufferSize = 512000;
                break;

            case 6:
                custom_PFMT = 1;
                break;
            default:
                /* Msg("H.263 source format not legal\n"); */
                return OMX_FALSE;
        }
        if (custom_PFMT == 0)
        {
            return OMX_TRUE;
        }


        ReadBits(aInputBuffer, 1, &codeword);
        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword) return OMX_FALSE;
        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword) return OMX_FALSE;
        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword) return OMX_FALSE;
        ReadBits(aInputBuffer, 3, &codeword);
        ReadBits(aInputBuffer, 3, &codeword);
        if (codeword) return OMX_FALSE; 			/* RPS, ISD, AIV */
        ReadBits(aInputBuffer, 1, &codeword);
        ReadBits(aInputBuffer, 4, &codeword);
        if (codeword != 8) return OMX_FALSE;
    }

    if (UFEP == 0 || UFEP == 1)
    {
        ReadBits(aInputBuffer, 3, &codeword);
        if (codeword > 1) return OMX_FALSE;
        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword) return OMX_FALSE;
        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword) return OMX_FALSE;
        ReadBits(aInputBuffer, 1, &codeword);
        ReadBits(aInputBuffer, 3, &codeword);
        if (codeword != 1) return OMX_FALSE;
    }
    else
    {
        return OMX_FALSE;
    }

    ReadBits(aInputBuffer, 1, &codeword);
    if (codeword) return OMX_FALSE; /* CPM */
    if (custom_PFMT == 1 && UFEP == 1)
    {
        OMX_U32 DisplayWidth, Width, DisplayHeight, Height, Resolution;

        ReadBits(aInputBuffer, 4, &codeword);
        if (codeword == 0) return OMX_FALSE;
        if (codeword == 0xf)
        {
            ReadBits(aInputBuffer, 8, &codeword);
            ReadBits(aInputBuffer, 8, &codeword);
        }
        ReadBits(aInputBuffer, 9, &codeword);
        DisplayWidth = (codeword + 1) << 2;
        Width = (DisplayWidth + 15) & -16;

        ReadBits(aInputBuffer, 1, &codeword);
        if (codeword != 1) return OMX_FALSE;
        ReadBits(aInputBuffer, 9, &codeword);
        if (codeword == 0) return OMX_FALSE;
        DisplayHeight = codeword << 2;
        Height = (DisplayHeight + 15) & -16;

        Resolution = Width * Height;

        if (Resolution <= 25344)		//25344 = 176x144 (QCIF)
        {
            *aBufferSize = 32000;
        }
        else if (Resolution <= 101376)	//101376 = 352x288 (CIF)
        {
            *aBufferSize = 128000;
        }
        else if (Resolution <= 405504)	//405504 = 704*576 (4CIF)
        {
            *aBufferSize = 256000;
        }
        else							//1408x1152 (16CIF)
        {
            //This is the max buffer size that we want to allocate
            *aBufferSize = 512000;
        }
    }

    return OMX_TRUE;
}


void OpenmaxMpeg4AO::ReadBits(OMX_U8* aStream,           /* Input Stream */
                              uint8 aNumBits,                     /* nr of bits to read */
                              uint32* aOutData                 /* output target */
                             )
{
    uint8 *bits;
    uint32 dataBitPos = iH263DataBitPos;
    uint32 bitPos = iH263BitPos;
    uint32 dataBytePos;

    if (aNumBits > (32 - bitPos))    /* not enough bits */
    {
        dataBytePos = dataBitPos >> 3;    /* Byte Aligned Position */
        bitPos = dataBitPos & 7; /* update bit position */
        bits = &aStream[dataBytePos];
        iH263BitBuf = (bits[0] << 24) | (bits[1] << 16) | (bits[2] << 8) | bits[3];
    }

    iH263DataBitPos += aNumBits;
    iH263BitPos      = (unsigned char)(bitPos + aNumBits);

    *aOutData = (iH263BitBuf >> (32 - iH263BitPos)) & mask[(uint16)aNumBits];

    return;
}

